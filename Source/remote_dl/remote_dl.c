//[]----------------------------------------------------------------------[]
///	@mainpage		FTP遠隔ダウンロード対応
///	remote download function <br><br>
///	<b>Copyright(C) 2010 AMANO Corp.</b>
///	CREATE			2010/10/20 Namioka<br>
///	UPDATE			
///	@file			remote_dl.c
///	@date			2010/10/20
///	@version		MH759500

//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
#include	<string.h>
#include	<stddef.h>
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mem_def.h"										/*						*/
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"remote_dl.h"
#include	"mnt_def.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mdl_def.h"
#include	"flp_def.h"
#include	"ntnetauto.h"
#include	"raudef.h"
#include	"LKcom.h"
#include	"updateope.h"

// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
// /*------------------------------------------------------------------------------*/
// #pragma	section	_CHGINFO		/* "B":Uninitialized data area in external RAM2 */
// /*------------------------------------------------------------------------------*/
// // このセクションにデータを追加する場合，t_prog_chg_info内に追加すること！！
// static	t_prog_chg_info	chg_info;
// static	uchar	work_update;
// static	t_prog_chg_info	bk_chg_info;
// static	t_remote_dl_info	remote_dl_info;
// static	t_remote_dl_info	bk_remote_dl_info;
// ushort	dummy_short;
// #pragma	section

// V0 のMAPでの並び
//
//SECTION=B_CHGINFO
//  _chg_info
//  _bk_chg_info
//  _remote_dl_info
//  _bk_remote_dl_info

// V1 のMAPでの並び
// SECTION=B_CHGINFO
//   _chg_info
//   _remote_dl_info
// SECTION=B_CHGINFO_BKUP
//   _bk_chg_info
//   _bk_remote_dl_info


/*------------------------------------------------------------------------------*/
#pragma	section	_CHGINFO		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// このセクションにデータを追加する場合，t_prog_chg_info内に追加すること！！
static	uchar	work_update;
ushort	dummy_short;
static	t_prog_chg_info	chg_info;
static	t_remote_dl_info	remote_dl_info;
#pragma	section
/*------------------------------------------------------------------------------*/
#pragma	section	_CHGINFO_BKUP		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// このセクションにデータを追加する場合，t_prog_chg_info内に追加すること！！
static	t_prog_chg_info	bk_chg_info;
static	t_remote_dl_info	bk_remote_dl_info;
#pragma	section
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

static	uchar	task_status;
static	t_FtpInfo	g_bk_FtpInfo;
t_NtBufCount		g_bufcnt;
static const uchar st_offset[2][6] = {
	{ 8, 6, 5, 2, 1,14 },
	{ 7,18,16, 4, 3, 9 },
};

const uchar ReqAcceptTbl[REMOTE_REQ_MAX] = {
// GG124100(S) R.Endo 2022/09/08 車番チケットレス3.0 #6586 起動時、NT-NET電文　センター用端末情報データ(ID：065)の項目「受付可能要求」に対応していない機能を「受付可」にして送信する [共通改善項目 No1531]
// 	0,
// 	1,			// バージョンアップ要求受付
// 	0,			// 部番変更要求
// // MH810100(S)
// //	0,			// 設定変更要求受付
// //	0,			// 設定要求受付
// 	1,			// 設定変更要求受付
// 	1,			// 設定要求受付
// // MH810100(E)
// 	1,			// リセット要求受付
// 	0,			// プログラム切換要求受付
// 	1,			// FTP設定変更要求受付
// 	0,			// 遠隔料金設定受付
// // MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
// 	0,			// 予備
// 	0,			// FTP接続テスト要求
// 	0,			// 予備
// 	0,			// 予備
// 	1,			// 設定変更要求
// // MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	0,
// GG129000(S) R.Endo 2022/11/15 車番チケットレス4.0 #6690 phase9（プログラムアップデート等）対応
// 	0,			// バージョンアップ要求
// 	0,			// 部番変更要求
// 	0,			// 設定変更要求
// 	1,			// 設定要求(アップロード)
// 	0,			// リセット要求
// 	0,			// プログラム切替要求
// 	0,			// FTP設定変更要求
// 	0,			// 遠隔料金設定要求
// 	0,			// 予備
// 	0,			// FTP接続テスト要求
// 	0,			// 予備
// 	0,			// 予備
// 	0,			// 差分設定変更要求
	1,			// バージョンアップ要求
	0,			// 部番変更要求
	1,			// 設定変更要求
	1,			// 設定要求(アップロード)
	1,			// リセット要求
	0,			// プログラム切替要求
	1,			// FTP設定変更要求
	0,			// 遠隔料金設定要求
	0,			// 予備
	1,			// FTP接続テスト要求
	0,			// 予備
	0,			// 予備
	1,			// 差分設定変更要求
// GG129000(E) R.Endo 2022/11/15 車番チケットレス4.0 #6690 phase9（プログラムアップデート等）対応
// GG124100(E) R.Endo 2022/09/08 車番チケットレス3.0 #6586 起動時、NT-NET電文　センター用端末情報データ(ID：065)の項目「受付可能要求」に対応していない機能を「受付可」にして送信する [共通改善項目 No1531]
};


#define TEST_CONNECT_FILE_SIZE		1024

#define FILENAME_PARAM_UP			0
#define FILENAME_PARAM_DOWN			1
#define FILENAME_TEST				2
static void MakeRemoteFileNameForIP_MODEL_NUM(ushort UpDown,char *remote);
// MH810103(s) 電子マネー対応 部番チェックを外す
//// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
//static eVER_TYPE GetVersionPrefix( uchar *buf, ushort size );
//// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
// MH810103(e) 電子マネー対応 部番チェックを外す
// GG120600(S) // Phase9 設定変更通知対応
static uchar remotedl_request_kind_to_connect_type(int request);
// GG120600(E) // Phase9 設定変更通知対応
// GG120600(S) // Phase9 遠隔監視データ変更
static ushort	g_usRmonSeq = 0;
// GG120600(E) // Phase9 遠隔監視データ変更
// GG120600(S) // Phase9 リトライの種別を分ける
static uchar connect_type_to_dl_time_kind(uchar connect_type);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
static long remotedl_nrm_time_sub(ulong from, ulong to);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードタスク
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_task( void )								
{															
	while(1){												
		taskchg( IDLETSKNO );								
		if( TASK_START_FOR_DL() ){							// Task起動要求あり
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
			// LCDへのパラメータアップロード中を排他
			if( OPECTL.lcd_prm_update ){
				continue;
			}
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
			ftp_remote_auto_update();						// FTP通信処理へ
		}
	}
}															
															
//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードタスク メッセージ取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			comd	:イベント
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
ushort	GetMessageRemote( void )							
{															
															
	MsgBuf	*msb;
	ushort 	comd;

	for( ; ; ) {
		taskchg( IDLETSKNO );								
		if( (msb = GetMsg( REMOTEDLTCBNO )) == NULL ){
			continue;
		}
		comd = msb->msg.command;
		break;
	}
	FreeBuf( msb );
	return( comd );

}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード状態更新
//[]----------------------------------------------------------------------[]
///	@param[in]		sts		:FTP通信状態	
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_status_set( uchar sts )					
{															
	chg_info.status = sts;
	task_status = sts;
}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード状態参照
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:FTP通信状態
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_status_get( void )								
{															
	return	(chg_info.status);
}															

//[]----------------------------------------------------------------------[]
///	@brief			タスク起動用遠隔ダウンロード状態参照
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:FTP通信状態(タスク切替時用の初期化エリアを使用)
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_task_startup_status_get( void )
{															
	return	(task_status);
}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード接続種別設定
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:接続/切断種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_connect_type_set( uchar type )	
{												
	chg_info.connect_type = type;
}												

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード接続種別取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			connect_type	:接続/切断種別
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_connect_type_get( void )
{										
	return	(chg_info.connect_type);
}										

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード情報取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			t_prog_chg_info	:ダウンロード情報ポインタ
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
t_prog_chg_info*	remotedl_info_get( void )				
{															
	return (&chg_info);
}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード情報クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_info_clr( void )							
{															
	memset( &chg_info, 0, sizeof(chg_info));
	remotedl_result_clr();
	memset(&remote_dl_info, 0, sizeof(remote_dl_info));
}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード結果更新
//[]----------------------------------------------------------------------[]
///	@param[in]		result	:結果種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_result_set( uchar result )
{
	uchar	index = 0;
	if( !DOWNLOADING() ){
		return;
	}
	switch (remotedl_connect_type_get()) {
// GG120600(S) // Phase9 結果を区別する
//	case CTRL_PROG_DL:
//	case CTRL_PARAM_DL:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//	case CTRL_PARAM_DIF_DL:			//差分設定アップロード
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//	case CTRL_PROG_SW:
//		if ((remotedl_status_get() == R_SW_WAIT) ||
//			(remotedl_status_get() == R_SW_START)) {
//			index = RES_SW;
//		}
//		else {
//			index = RES_DL;
//		}
//		break;
	case CTRL_PROG_DL:
	case CTRL_PROG_SW:
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PROG;
		}
		else {
			index = RES_DL_PROG;
		}
		break;
	case CTRL_PARAM_DL:
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PARAM;
		}
		else {
			index = RES_DL_PARAM;
		}
		break;
	case CTRL_PARAM_DIF_DL:			//差分設定アップロード
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PARAM_DIF;
		}
		else {
			index = RES_DL_PARAM_DIF;
		}
		break;
// GG120600(E) // Phase9 結果を区別する
	case CTRL_PARAM_UPLOAD:
		index = RES_UP;
		break;
	case CTRL_CONNECT_CHK:
		index = RES_COMM;
		break;
// GG120600(S) // Phase9 LCD用
	case CTRL_PARAM_UPLOAD_LCD:
		index = RES_UP_LCD;
		break;
// GG120600(E) // Phase9 LCD用
	default:
		return;
	}
	chg_info.result[index] = result;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード結果取得
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:取得する結果種別
///	@return			result	:対象の結果
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_result_get( uchar type )
{
	return (chg_info.result[type]);
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード結果状態初期化
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			none
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_result_clr( void )
{
	memset( chg_info.result, EXCLUDED, sizeof(chg_info.result));	// 結果コードは0xffで初期化
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラム更新結果設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_update_set( void )	
{
	chg_info.update = 1;				// プログラム更新完了フラグをセット
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード/結果情報リトライ情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar retry_info_set(uchar kind)
{
	uchar type = remotedl_status_get();
	uchar ret = 0;
	ulong wtime,interval,overtime;

	if( chg_info.retry[kind].status == RETRY_OVER ){	// リトライ回数をオーバーしている場合は、リトライしない
		return 1;
	}

// GG120600(S) // Phase9 リトライの種別を分ける
//	if (kind == RETRY_KIND_CONNECT) {
	if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 リトライの種別を分ける
		interval = prm_get(COM_PRM, S_RDL, 5, 3, 3);
		memset( &chg_info.retry[kind].time, 0, sizeof( date_time_rec ));
	}
	else {
		// リトライ間隔（分）
		switch (kind) {
// GG120600(S) // Phase9 結果を区別する
//		case RETRY_KIND_DL:
//		case RETRY_KIND_UL:
		case RETRY_KIND_DL_PRG:
		case RETRY_KIND_DL_PARAM:
		case RETRY_KIND_DL_PARAM_DIFF:
		case RETRY_KIND_UL:
// GG120600(E) // Phase9 結果を区別する
			interval = prm_get(COM_PRM, S_RDL, 1, 3, 3);
			break;
		default:
			interval = 0;
			break;
		}

		memcpy( &chg_info.retry[kind].time, &CLK_REC, sizeof( date_time_rec ));
		wtime = Nrm_YMDHM( &chg_info.retry[kind].time );

		overtime = ( wtime&0x0000ffff ) + (interval%1440);
		if( overtime > 1439 ){							// インターバル後の時間が日跨ぎする場合
			wtime += 0x10000;							// ＋１日する
			wtime = ((wtime&0xffff0000) + (overtime - 1440));	// 越えた後の時間(分)を設定する
		}else{											// 日跨ぎしない場合
			wtime += ((interval%1440));
		}

		wtime += ((interval/1440) << 16);				// 日数間隔を＋する
		UnNrm_YMDHM( &chg_info.retry[kind].time, wtime );
	}

	// リトライ回数
	if (chg_info.retry[kind].count == 0) {
// GG120600(S) // Phase9 リトライの種別を分ける
//		if (kind == RETRY_KIND_CONNECT) {
		if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 リトライの種別を分ける
			chg_info.retry[kind].count = prm_get(COM_PRM, S_RDL, 5, 2, 1);
		}
		else {
			switch (kind) {
// GG120600(S) // Phase9 結果を区別する
//			case RETRY_KIND_DL:
			case RETRY_KIND_DL_PRG:
			case RETRY_KIND_DL_PARAM:
			case RETRY_KIND_DL_PARAM_DIFF:
// GG120600(E) // Phase9 結果を区別する
			case RETRY_KIND_UL:
				chg_info.retry[kind].count = prm_get(COM_PRM, S_RDL, 1, 2, 1);
				break;
			default:
				chg_info.retry[kind].count = 0;
				break;
			}
		}
	}

	// リトライ判定
	if( !chg_info.retry[kind].status ){					// 初回の場合
		if( chg_info.retry[kind].count ){				// 回数が０以外ならば、
			chg_info.retry[kind].status = type;			// リトライ予約セット
		}else{											// 回数が０回の場合は
			chg_info.retry[kind].status = RETRY_OVER;	// リトライ終了→リトライ済みをセット
			ret = 1;									// リトライオーバーとする
		}
	}else{												// リトライ中
		if( --chg_info.retry[kind].count == 0 ){		// リトライ回数残りなし
			chg_info.retry[kind].status = RETRY_OVER;	// リトライ終了→リトライ済みをセット
			ret = 1;									// リトライオーバー
		}else{
			chg_info.retry[kind].status &= 0x0f;		// リトライ中フラグOFF
		}
	}

	// リトライオーバーではない？
	if (ret != 1) {
		if( !chg_info.pow_flg ) {						// 復電フラグがOFF
			if( !interval ){							// インターバル間隔が０分の場合は即時判定
// GG120600(S) // Phase9 リトライの種別を分ける
//				if (kind == RETRY_KIND_CONNECT) {
				if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 リトライの種別を分ける
					memcpy( &chg_info.retry[kind].time, &CLK_REC, sizeof( date_time_rec ));
				}
				queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
			}
// GG120600(S) // Phase9 リトライの種別を分ける
//			else if (kind == RETRY_KIND_CONNECT) {		// 接続リトライの場合、タイマを動作させる
//				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
			else if (kind ==  RETRY_KIND_CONNECT_PRG) {		// 接続リトライの場合、タイマを動作させる
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_UP) {		// 接続リトライの場合、タイマを動作させる
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer2);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_DL) {		// 接続リトライの場合、タイマを動作させる
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer3);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_DL_DIFF) {		// 接続リトライの場合、タイマを動作させる
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer4);
// GG120600(E) // Phase9 リトライの種別を分ける
			}
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			接続リトライ開始
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_info_connect_timer(void)
{
	// 復電フラグOFF
	remotedl_pow_flg_set(FALSE);
	// 接続開始時刻セット
// GG120600(S) // Phase9 リトライの種別を分ける
//	memcpy( &chg_info.retry[RETRY_KIND_CONNECT].time, &CLK_REC, sizeof( date_time_rec ));
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PRG].time, &CLK_REC, sizeof( date_time_rec ));
// GG120600(E) // Phase9 リトライの種別を分ける
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
// GG120600(S) // Phase9 リトライの種別を分ける
void retry_info_connect_timer2(void)
{
	// 復電フラグOFF
	remotedl_pow_flg_set(FALSE);
	// 接続開始時刻セット
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_UP].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
void retry_info_connect_timer3(void)
{
	// 復電フラグOFF
	remotedl_pow_flg_set(FALSE);
	// 接続開始時刻セット
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_DL].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
void retry_info_connect_timer4(void)
{
	// 復電フラグOFF
	remotedl_pow_flg_set(FALSE);
	// 接続開始時刻セット
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
// GG120600(E) // Phase9 リトライの種別を分ける

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードリトライ情報をクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void retry_info_clr(uchar kind)
{
	if (kind == RETRY_KIND_MAX) {
		memset(&chg_info.retry, 0, sizeof(chg_info.retry));
	}
	else {
		memset(&chg_info.retry[kind], 0, sizeof(t_retry_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードリトライカウントをクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_count_clr(uchar kind)
{
	int i;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			chg_info.retry[i].count = 0;
		}
	}
	else {
		chg_info.retry[kind].count = 0;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードリトライ時間をクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_time_clr(uchar kind)
{
	int i;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			memset(&chg_info.retry[i].time, 0, sizeof(date_time_rec));
		}
	}
	else {
		memset(&chg_info.retry[kind].time, 0, sizeof(date_time_rec));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			更新処理結果取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			update	:0：プログラム更新未処理 1：プログラム更新済み
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_update_chk( void )	
{
	uchar ret = 0;

	if( ( TENKEY_F1 == 1 )&&( TENKEY_F3 == 1 ) || 
		( TENKEY_F3 == 1 )&&( TENKEY_F5 == 1 )){
		work_update = 0;
	}else{
		// 遠隔メンテナンス情報をバックアップ
		memcpy(&bk_remote_dl_info, &remote_dl_info, sizeof(remote_dl_info));
		if( chg_info.update == 1 ){
			work_update = chg_info.update;
			// プログラム更新、設定更新時はフラグをクリアする
			f_ParaUpdate.BYTE = 0;
			if (remotedl_connect_type_get() == CTRL_PROG_DL ||
				remotedl_connect_type_get() == CTRL_PROG_SW) {
				ret = 1;
			}
		}else{
			work_update = 0;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			更新処理結果取得（MAF初期設定用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			update	:0：プログラム更新未処理 1：プログラム更新済み
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_first_comm_get( void )	
{
	uchar ret = work_update;

	work_update = 0;

	return 	(ret);
}

// GG120600(S) // Phase9 設定変更通知対応
uchar	remotedl_work_update_get( void )	
{
	uchar ret = work_update;

	return 	(ret);
}
// GG120600(E) // Phase9 設定変更通知対応

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードリストア処理実行
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_restore( void )							
{															
	t_SysMnt_ErrInfo	errinfo;
	uchar	status,i;
	ulong 	ret;
	uchar	timerset = 0;
	uchar	result;
	uchar	connect = remotedl_connect_type_get();
	uchar	comp = EXEC_STS_NONE;
	PRG_HDR	header;
// GG120600(S) // Phase9 リトライの種別を分ける
	uchar	kind;
// GG120600(E) // Phase9 リトライの種別を分ける

	if( work_update ){
		remotedl_status_set( R_SW_START );
		
		// プログラム切換時はリストアを行う
		remotedl_chg_info_restore();
		memcpy(&remote_dl_info, &bk_remote_dl_info, sizeof(bk_remote_dl_info));
		connect = remotedl_connect_type_get();
		if (connect == CTRL_PROG_DL ||
			connect == CTRL_PROG_SW) {
			for( i=0; i<RESTORE_RETRY_COUNT; i++){
				errinfo.errmsg = _SYSMNT_ERR_NONE;
				sysmnt_Restore(&errinfo);
				if( errinfo.errmsg == _SYSMNT_ERR_NONE ){
					break;
				}
				WACDOG;
			}
			for( i=0; i<RESTORE_RETRY_COUNT; i++){
				ret = FLT_Restore_FLAPDT();
				if( _FLT_RtnKind(ret) == FLT_NORMAL ){
					dsp_background_color(COLOR_BLACK);
					grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[66] );	// "       ﾛｯｸ装置状態ﾘｽﾄｱ        ",表示
					dsp_background_color(COLOR_WHITE);
					break;
				}
				WACDOG;
			}
			wopelg( OPLOG_ALLRESTORE, 0, 0 );			// 操作履歴登録
			if ((errinfo.errmsg == _SYSMNT_ERR_NONE) && (_FLT_RtnKind(ret) == FLT_NORMAL)) {
				remotedl_result_set(PROG_SW_COMP);		// プログラム更新完了
			}
			else {
				remotedl_result_set(RESTORE_ERR);		// リストア失敗
			}
		}
		else {
			remotedl_result_set(PROG_SW_COMP);			// 更新完了
		}
// GG120600(S) // Phase9 結果を区別する
//		ret = remotedl_result_get(RES_SW);
		switch (connect) {
		case CTRL_PROG_DL:
		case CTRL_PROG_SW:
			ret = remotedl_result_get(RES_SW_PROG);
			break;
		case CTRL_PARAM_DL:
			ret = remotedl_result_get(RES_SW_PARAM);
			break;
		case CTRL_PARAM_DIF_DL:							// 差分設定アップロード
			ret = remotedl_result_get(RES_SW_PARAM_DIF);
			break;
		}
// GG120600(E) // Phase9 結果を区別する
		moitor_regist(OPLOG_REMOTE_SW_END, ret);		// 更新完了の操作モニタを登録
// GG120600(S) // Phase9 種別毎をチェック
//		remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
		switch (connect) {
		case CTRL_PROG_DL:
		case CTRL_PROG_SW:
			switch (ret) {
			case PROG_SW_COMP:
				rmon_regist(RMON_PRG_SW_END_OK);
				comp = EXEC_STS_COMP;
				break;
			case RESTORE_ERR:
				rmon_regist(RMON_PRG_SW_END_RESTORE_NG);
				break;
			}

			if (connect == CTRL_PROG_DL) {
				remotedl_complete_request(REQ_KIND_VER_UP);
				remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, comp);
			}
			else {
				remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
				remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, comp);
			}
			// 再起動
			if (ret == PROG_SW_COMP) {
				System_reset();						// Main CPU reset (It will not retrun from this function)
			}
			break;
		case CTRL_PARAM_DL:
			rmon_regist(RMON_PRM_SW_END_OK);

			remotedl_complete_request(REQ_KIND_PARAM_CHG);
			remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_COMP);
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		case CTRL_PARAM_DIF_DL:							// 差分設定アップロード
			rmon_regist(RMON_PRM_SW_DIF_END_OK);		// 遠隔監視データ登録

			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);		// 遠隔メンテナンス要求完了
			remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_COMP);		// 遠隔メンテナンス処理ステータス設定
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		default:
			break;
		}
		return timerset;
	}
	
	status = remotedl_status_get();
	switch( status ){
		case	R_DL_START:
		case	R_DL_EXEC:
		case	R_SW_START:
			result = (uchar)(remotedl_exec_info_get()+PROG_DL_RESET);
			remotedl_result_set( result );						// 結果情報をセット（リセットによる処理中止）
			moitor_regist(( status == R_SW_START?OPLOG_REMOTE_SW_END:OPLOG_REMOTE_DL_END ), result );	// ダウンロード/更新完了（異常）の操作モニタを登録
			switch (status) {
			case R_DL_START:		// 遠隔ダウンロード実行開始（予約）
			case R_DL_EXEC:			// 遠隔ダウンロード実行（Task起動）
			case R_UP_START:		// アップロード実行開始
				// 復電フラグON
				remotedl_pow_flg_set(TRUE);
				// write情報をクリア
				remotedl_write_info_clear();

				// リトライカウントをクリア
				if (connect == CTRL_PARAM_UPLOAD) {
					retry_info_clr(RETRY_KIND_UL);
				}
				else {
// GG120600(S) // Phase9 結果を区別する
//					retry_info_clr(RETRY_KIND_DL);
					switch (connect) {
					case CTRL_PARAM_DL:
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
						break;
					case CTRL_PARAM_DIF_DL:
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
						break;
					}
// GG120600(E) // Phase9 結果を区別する
				}
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_count_clr(RETRY_KIND_CONNECT);
				switch (connect) {
				case CTRL_PROG_DL:
					kind = RETRY_KIND_CONNECT_PRG;
					break;
				case CTRL_PARAM_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL;
					break;
				case CTRL_PARAM_DIF_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
					break;
				case CTRL_PARAM_UPLOAD:
					kind = RETRY_KIND_CONNECT_PARAM_UP;
					break;
				default:
					break;
				}
				retry_count_clr(kind);
// GG120600(E) // Phase9 リトライの種別を分ける

				// 接続リトライ
// GG120600(S) // Phase9 リトライの種別を分ける
//				if (retry_info_set(RETRY_KIND_CONNECT)) {
				if (retry_info_set(kind)) {
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//					// リトライ情報クリア
//					retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 リトライの種別を分ける
					// ここのルートはリトライなししかありえない
					// リトライオーバーなので受け付けた要求をクリアする
					switch (connect) {
					case CTRL_PROG_DL:
// GG120600(S) // Phase9 リトライの種別を分ける
						retry_info_clr(RETRY_KIND_CONNECT_PRG);
						retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 リトライの種別を分ける
						rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						break;
					case CTRL_PARAM_DL:
// GG120600(S) // Phase9 リトライの種別を分ける
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
						retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 リトライの種別を分ける
						rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 リトライの種別を分ける
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
						retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 リトライの種別を分ける
						rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);		// 遠隔監視データ登録
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// 遠隔メンテナンス要求完了
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 リトライの種別を分ける
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
						retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 リトライの種別を分ける
						rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						remotedl_comp_set(INFO_KIND_START, REQ_KIND_PARAM_UL, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
						break;
					default:
						break;
					}
				}
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
				// FROMのヘッダ情報を削除しておく
				if (connect == CTRL_PROG_DL) {
					memset(&header, 0xFF, sizeof(header));
					FLT_write_program_version( (uchar*)&header );
				}
// GG129000(S) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
				remotedl_arrange_next_request();
// GG129000(E) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
				// Lagtim初期化後にタイマセットする
				timerset = 1;
				break;
			case R_SW_START:		// プログラム更新開始
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
				switch (connect) {
				case CTRL_PROG_DL:
				case CTRL_PROG_SW:
					rmon_regist(RMON_PRG_SW_END_RESET_NG);			// 更新中にリセット
					if (connect == CTRL_PROG_DL) {
						remotedl_complete_request(REQ_KIND_VER_UP);
					}
					else {
						remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
					}
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_SW_END_RESET_NG);			// 更新中にリセット
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_SW_DIF_END_RESET_NG);			// 更新中にリセット
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// 遠隔メンテナンス要求完了
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				default:
					break;
				}
				break;
			default:
				break;
			}
			break;


		case	R_DL_IDLE:
				// 復電フラグON
				remotedl_pow_flg_set(TRUE);
				// リトライ中であればカウントクリア
				if (now_retry_active_chk(RETRY_KIND_MAX)) {
					// リトライカウントをクリア
// GG120600(S) // Phase9 リトライの種別を分ける
//					retry_info_clr(RETRY_KIND_DL);
//					retry_info_clr(RETRY_KIND_UL);
//					retry_count_clr(RETRY_KIND_CONNECT);
					// すべてクリア
					retry_count_clr(RETRY_KIND_MAX);
					switch (connect) {
					case CTRL_PROG_DL:
						kind = RETRY_KIND_CONNECT_PRG;
						break;
					case CTRL_PARAM_DL:
						kind = RETRY_KIND_CONNECT_PARAM_DL;
						break;
					case CTRL_PARAM_DIF_DL:
						kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
						break;
					case CTRL_PARAM_UPLOAD:
						kind = RETRY_KIND_CONNECT_PARAM_UP;
						break;
					default:
						break;
					}
// GG120600(E) // Phase9 リトライの種別を分ける
					// 接続リトライ
// GG120600(S) // Phase9 リトライの種別を分ける
//					if (retry_info_set(RETRY_KIND_CONNECT)) {
					if (retry_info_set(kind)) {
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//						// リトライ情報クリア
//						retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 リトライの種別を分ける
						// ここのルートはリトライなししかありえない
						// リトライオーバーなので受け付けた要求をクリアする
						switch (remotedl_connect_type_get()) {
						case CTRL_PROG_DL:
// GG120600(S) // Phase9 リトライの種別を分ける
							retry_info_clr(RETRY_KIND_CONNECT_PRG);
							retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 リトライの種別を分ける
							rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							break;
						case CTRL_PARAM_DL:
// GG120600(S) // Phase9 リトライの種別を分ける
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
							retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 リトライの種別を分ける
							rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
						case CTRL_PARAM_DIF_DL:		//差分設定アップロード
// GG120600(S) // Phase9 リトライの種別を分ける
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
							retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 リトライの種別を分ける
							rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);		// 遠隔監視データ登録
							remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// 遠隔メンテナンス要求完了
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
						case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 リトライの種別を分ける
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
							retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 リトライの種別を分ける
							rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5820:FTPリトライ中に接続リトライ回数を0に変更後、電源OFF/ONでリトライオーバーしない)
							break;
						default:
							break;
						}
					}
					else {
						// Lagtim初期化後にタイマセットする
						timerset = 1;
					}
				}
				else {
					retry_info_clr(RETRY_KIND_MAX);
				}
				// write情報をクリア
				remotedl_write_info_clear();
				break;
		case	R_RESET_START:
				rmon_regist(RMON_RESET_END_OK);
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
				remotedl_complete_request(REQ_KIND_RESET);
				remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_COMP);
				break;
		case	R_TEST_CONNECT:
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
				remotedl_complete_request(REQ_KIND_TEST);
				break;
		case	R_DL_REQ_RCV:
		case	R_UP_WAIT:
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
				// 開始判定リトライ
				if (remotedl_dl_start_retry_check() < 0) {
					// ここのルートはリトライなししかありえない
					// リトライオーバー
// GG120600(S) // Phase9 種別毎をチェック
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_DIF_DL:		// 差分設定アップロード
						rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);		// 遠隔監視データ登録
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// 遠隔メンテナンス要求完了
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);		// 遠隔メンテナンス処理ステータス設定
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_UPLOAD:
						rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_UL);
						remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
						break;
					default:
						break;
					}
				}
				break;
		case	R_SW_WAIT:
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
				if (remotedl_sw_start_retry_check() < 0) {
					// ここのルートはリトライなししかありえない
					// リトライオーバー
// GG120600(S) // Phase9 種別毎をチェック
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_DIF_DL:
						rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
						remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PROG_SW:
						rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
						remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
						break;
					default:
						break;
					}
				}
				break;
		case	R_RESET_WAIT:
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				if (remotedl_reset_start_retry_check() < 0) {
					// ここのルートはリトライなししかありえない
					// リトライオーバー
// GG120600(S) // Phase9 種別毎をチェック
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
					rmon_regist(RMON_RESET_START_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_RESET);
					remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
				}
				break;
		default:
			break;
	}
	
	return timerset;

}															

//[]----------------------------------------------------------------------[]
///	@brief			全バックアップ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		:0：バックアップ失敗 1：バックアップ成功
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_BackUP( void )							
{															
	t_SysMnt_ErrInfo	errinfo;
	uchar i, ret;
	
	ret = 0;
	// 設定切換はバックアップを行わない
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//	if (remotedl_connect_type_get() == CTRL_PARAM_DL) {
	if (remotedl_connect_type_get() == CTRL_PARAM_DL || remotedl_connect_type_get() == CTRL_PARAM_DIF_DL ) {
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		return 1;
	}
	wopelg( OPLOG_ALLBACKUP, 0, 0 );			// 操作履歴登録
	for( i=0; i<BACKUP_RETRY_COUNT; i++){
		errinfo.errmsg = _SYSMNT_ERR_NONE;
		sysmnt_Backup(&errinfo);
		if( errinfo.errmsg == _SYSMNT_ERR_NONE ){
			ret = 1;
			break;
		}
		WACDOG;
	}
	return ret;
}															

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードRism切断完了処理
//[]----------------------------------------------------------------------[]
///	@param[in]		evt		:イベント種別
///	@param[in]		data	:要求種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_evt_recv( ushort evt, uchar *data )
{

	uchar status = remotedl_status_get();
	
	switch( evt ){
		case	REMOTE_DL_REQ:
			remote_dl_check( data );				// 要求チェック
			break;

		case	REMOTE_CONNECT_EVT:
			switch( status ){
				case	R_DL_START:
				case 	R_UP_START:
				case 	R_TEST_CONNECT:
					remotedl_status_set( R_DL_EXEC );			// remotedl_task起動
					break;


				case	R_SW_START:
					ftp_remote_auto_switch();									// 切替処理実行
					// 更新失敗でリセット
					System_reset();								// Main CPU reset (It will not retrun from this function)
					break;
					
				default:
					break;
			}
			break;
		case	REMOTE_DL_END:
		case	REMOTE_RETRY_SND:
				AutoDL_UpdateTimeCheck();
				break;

		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラム更新時刻判定/実行
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	AutoDL_UpdateTimeCheck( void )
{
	ulong	GetDate,NowDate;
	uchar	i,res,status;
	t_prog_chg_info *dl = remotedl_info_get();
	ushort	sec_cnt;
	ulong	code, count;
	MsgBuf	*msb, msg;
	t_TARGET_MSGGET_PRM	WaitMsgID;
	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;
	
	status = remotedl_status_get();
	
	switch( status ){
		case	R_DL_IDLE:
			// リトライチェック
			switch (remotedl_connect_type_get()) {
// GG120600(S) // Phase9 リトライの種別を分ける
//			case CTRL_PROG_DL:
//			case CTRL_PARAM_DL:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//			case CTRL_PARAM_DIF_DL:
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)			
//				// 接続リトライ？
//				if (now_retry_active_chk(RETRY_KIND_CONNECT) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT].time, R_DL_START)) {
//					dl->retry[RETRY_KIND_CONNECT].status |= RETRY_EXEC;
//				}
//				// ダウンロードリトライ？
//				else if (now_retry_active_chk(RETRY_KIND_DL) && dl_start_chk(&dl->retry[RETRY_KIND_DL].time, R_DL_START)) {
//					dl->retry[RETRY_KIND_DL].status |= RETRY_EXEC;
//				}
//				break;
//			case CTRL_PARAM_UPLOAD:
//				// 接続リトライ？
//				if (now_retry_active_chk(RETRY_KIND_CONNECT) && up_wait_chk(&dl->retry[RETRY_KIND_CONNECT].time, R_UP_START)) {
//					dl->retry[RETRY_KIND_CONNECT].status |= RETRY_EXEC;
//				}
//				// アップロードリトライ？
//				else if (now_retry_active_chk(RETRY_KIND_UL) && up_wait_chk(&dl->retry[RETRY_KIND_UL].time, R_UP_START)) {
//					dl->retry[RETRY_KIND_UL].status |= RETRY_EXEC;
//				}
//				break;
			case CTRL_PROG_DL:
				// 接続リトライ？
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PRG].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PRG].status |= RETRY_EXEC;
				}
				// ダウンロードリトライ？
				else if (now_retry_active_chk(RETRY_KIND_DL_PRG) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PRG].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PRG].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_DL:
				// 接続リトライ？
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_DL].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_DL].status |= RETRY_EXEC;
				}
				// ダウンロードリトライ？
				else if (now_retry_active_chk(RETRY_KIND_DL_PARAM) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PARAM].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PARAM].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_DIF_DL:
				// 接続リトライ？
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].status |= RETRY_EXEC;
				}
				// ダウンロードリトライ？
				else if (now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PARAM_DIFF].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PARAM_DIFF].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_UPLOAD:
				// 接続リトライ？
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) && up_wait_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_UP].time, R_UP_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_UP].status |= RETRY_EXEC;
				}
				// アップロードリトライ？
				else if (now_retry_active_chk(RETRY_KIND_UL) && up_wait_chk(&dl->retry[RETRY_KIND_UL].time, R_UP_START)) {
					dl->retry[RETRY_KIND_UL].status |= RETRY_EXEC;
				}
				break;
// GG120600(E) // Phase9 リトライの種別を分ける
			default:
// GG120600(S) // Phase9 設定変更通知対応
//				return;
				// 念のため次に実行される要求をセット
				remotedl_arrange_next_request();
				break;
// GG120600(E) // Phase9 設定変更通知対応
			}
			break;

		case	R_DL_REQ_RCV:
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				i = PROG_DL_TIME;
				break;
			case CTRL_PARAM_DL:
				i = PARAM_DL_TIME;
				break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
			case CTRL_PARAM_DIF_DL:
				i = PARAM_DL_DIF_TIME;
				break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
			default:
				return;
			}
			dl_start_chk( &dl->dl_info[i].start_time, R_DL_START );
			break;

		case	R_SW_WAIT:
			res = 0;
		
			for( i=0; i<SW_MAX; i++ ){
				switch (i) {
				case SW_PROG:
				case SW_PROG_EX:
					code = RMON_PRG_SW_START_OK;
					break;
// GG120600(S) // Phase9 パラメータ切替を分ける
//				case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////					code = RMON_PRM_SW_START_OK;
//					if ( remotedl_connect_type_get() ==  CTRL_PARAM_DL ){
//						code = RMON_PRM_SW_START_OK;
//					} else {
//						code = RMON_PRM_SW_DIF_START_OK;
//					}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
				case SW_PARAM:
					code = RMON_PRM_SW_START_OK;
					break;
				case SW_PARAM_DIFF:
					code = RMON_PRM_SW_DIF_START_OK;
// GG120600(E) // Phase9 パラメータ切替を分ける
					break;
				}
				if( dl->sw_info[i].exec ){
					GetDate = Nrm_YMDHM( &dl->sw_info[i].sw_time );			// 実行時間ノーマライズ
					NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// 現在時刻ノーマライズ
					if( GetDate <= NowDate ){ 								// 一致もしくは切替時刻を経過している
// GG129000(S) R.Endo 2023/01/26 車番チケットレス4.1 #6816 遠隔設定ダウンロードの即時実行で失敗(一定時間経過)後に再起動しAMANO画面から進まず [共通改善項目 No 1548]
						// 開始時刻が一定時間経過していないか？
						if ( remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME ) {
							remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
							switch ( remotedl_connect_type_get() ) {
							case CTRL_PROG_DL:
								rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_VER_UP);
								remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PARAM_DL:
								rmon_regist(RMON_PRM_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PARAM_CHG);
								remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PARAM_DIF_DL:
								rmon_regist(RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
								remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PROG_SW:
								rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
								remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
								break;
							default:
								break;
							}
							continue;
						}
// GG129000(E) R.Endo 2023/01/26 車番チケットレス4.1 #6816 遠隔設定ダウンロードの即時実行で失敗(一定時間経過)後に再起動しAMANO画面から進まず [共通改善項目 No 1548]
						do {
							// 待機か休業以外の場合
							if (!(OPECTL.Mnt_mod == 0 &&
								(OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100))) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_STATUS_NG;
									break;
// GG120600(S) // Phase9 パラメータ切替を分ける
//								case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////									code = RMON_PRM_SW_START_STATUS_NG;
//									if( remotedl_connect_type_get() == CTRL_PARAM_DL ){		// 遠隔ダウンロード接続種別取得
//										code = RMON_PRM_SW_START_STATUS_NG;			// 状態NG
//									} else {
//										code = RMON_PRM_SW_DIF_START_STATUS_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
								case SW_PARAM:
									code = RMON_PRM_SW_START_STATUS_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_STATUS_NG;
// GG120600(E) // Phase9 パラメータ切替を分ける
									break;
								}
								break;
							}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//							// Edy自動センター通信中、自動T合計実施中
//							if ((auto_cnt_prn == 2) ||
//								(auto_syu_prn == 1) || (auto_syu_prn == 2)) {
							// 自動T合計実施中
							if ((auto_syu_prn == 1) || (auto_syu_prn == 2)) {
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_COMM_NG;
									break;
// GG120600(S) // Phase9 パラメータ切替を分ける
//								case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////									code = RMON_PRM_SW_START_COMM_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_COMM_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_COMM_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
								case SW_PARAM:
									code = RMON_PRM_SW_START_COMM_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_COMM_NG;
// GG120600(E) // Phase9 パラメータ切替を分ける
									break;
								}
								break;
							}
							// フラップ動作中
							if (flap_move_chk()) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_FLAP_NG;
									break;
								case SW_PARAM:
// GG120600(S) // Phase9 パラメータ切替を分ける
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////									code = RMON_PRM_SW_START_FLAP_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_FLAP_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_FLAP_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
									code = RMON_PRM_SW_START_FLAP_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_FLAP_NG;
// GG120600(E) // Phase9 パラメータ切替を分ける
									break;
								}
								break;
							}
							// ドアが開いているか、ソレノイドロックが解除状態
							if (DOOR_OPEN_OR_SOLENOID_UNROCK()) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_DOOR_OPEN_NG;
									break;
								case SW_PARAM:
// GG120600(S) // Phase9 パラメータ切替を分ける
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////									code = RMON_PRM_SW_START_DOOR_OPEN_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_DOOR_OPEN_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_DOOR_OPEN_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
									code = RMON_PRM_SW_START_DOOR_OPEN_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_DOOR_OPEN_NG;
// GG120600(E) // Phase9 パラメータ切替を分ける
									break;
								}
								break;
							}
// GG129000(S) R.Endo 2023/01/26 車番チケットレス4.1 #6816 遠隔設定ダウンロードの即時実行で失敗(一定時間経過)後に再起動しAMANO画面から進まず [共通改善項目 No 1548]
// 							// 開始時刻が一定時間経過していないか？
// // GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
// //							if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
// 							if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// // GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
// // GG120600(S) // Phase9 種別毎をチェック
// //								remotedl_status_set(R_DL_IDLE);
// // GG120600(E) // Phase9 種別毎をチェック
// // GG120600(S) // Phase9 リトライの種別を分ける
// //								remotedl_start_retry_clear();
// 								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// // GG120600(E) // Phase9 リトライの種別を分ける
// 								switch (remotedl_connect_type_get()) {
// 								case CTRL_PROG_DL:
// // GG120600(S) // Phase9 (一定時間経過)を追加
// //									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (一定時間経過)を追加
// 									remotedl_complete_request(REQ_KIND_VER_UP);
// 									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
// 									break;
// 								case CTRL_PARAM_DL:
// // GG120600(S) // Phase9 (一定時間経過)を追加
// //									rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRM_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (一定時間経過)を追加
// 									remotedl_complete_request(REQ_KIND_PARAM_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
// 									break;
// // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
// 								case CTRL_PARAM_DIF_DL:
// // GG120600(S) // Phase9 (一定時間経過)を追加
// //									rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
// 									rmon_regist(RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (一定時間経過)を追加
// 									remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// 									break;
// // MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
// 								case CTRL_PROG_SW:
// // GG120600(S) // Phase9 (一定時間経過)を追加
// //									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (一定時間経過)を追加
// 									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
// 									break;
// 								default:
// 									break;
// 								}
// 								continue;
// 							}
// GG129000(E) R.Endo 2023/01/26 車番チケットレス4.1 #6816 遠隔設定ダウンロードの即時実行で失敗(一定時間経過)後に再起動しAMANO画面から進まず [共通改善項目 No 1548]
						} while (0);

						// 更新開始NG
// MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
//						if ((code != RMON_PRG_SW_START_OK) &&
//							(code != RMON_PRM_SW_START_OK)) {
						if ((code != RMON_PRG_SW_START_OK) &&
							(code != RMON_PRM_SW_START_OK) && (code != RMON_PRM_SW_DIF_START_OK)) {
// MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
							// エラーの監視データを登録
							rmon_regist(code);
							if (remotedl_sw_start_retry_check() < 0) {
// GG120600(S) // Phase9 種別毎をチェック
//								remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//								remotedl_start_retry_clear();
								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
								switch (remotedl_connect_type_get()) {
								case CTRL_PROG_DL:
									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_VER_UP);
									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
									break;
								case CTRL_PARAM_DL:
									rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PARAM_CHG);
									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
									break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
								case CTRL_PARAM_DIF_DL:
									rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
									break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
								case CTRL_PROG_SW:
									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
									break;
								default:
									break;
								}
							}
							continue;
						}
						// 更新開始OK
						else {
// GG120600(S) // Phase9 リトライの種別を分ける
//							remotedl_start_retry_clear();
							remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける

							switch (i) {
							case SW_PROG:
							case SW_PROG_EX:
								rmon_regist(RMON_PRG_SW_START_OK);
								break;
// GG120600(S) // Phase9 パラメータ切替を分ける
//							case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
////								rmon_regist(RMON_PRM_SW_START_OK);
//								if( remotedl_connect_type_get() == (CTRL_PARAM_DL)){
//									rmon_regist(RMON_PRM_SW_START_OK);
//								} else {
//									rmon_regist(RMON_PRM_SW_DIF_START_OK);
//								}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)
							case SW_PARAM:
								rmon_regist(RMON_PRM_SW_START_OK);
								break;
							case SW_PARAM_DIFF:
								rmon_regist(RMON_PRM_SW_DIF_START_OK);
// GG120600(E) // Phase9 パラメータ切替を分ける
								break;
							}
						}
						dl->sw_info[i].exec = 2;							// 切替実行フラグセット
						res = 1;											// 切替あり
// GG120600(S) // Phase9 #5031 バージョンアップ要求と設定変更要求が同じ開始時刻、更新時刻(開始≠更新)で予約されている場合、バージョンアップ要求の処理完了まで設定変更要求の処理を実行しない
						// 1つみつけたらbreak
						break;
// GG120600(E) // Phase9 #5031 バージョンアップ要求と設定変更要求が同じ開始時刻、更新時刻(開始≠更新)で予約されている場合、バージョンアップ要求の処理完了まで設定変更要求の処理を実行しない
					}
// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
					// まだ時間になっていない？
					else {
						// 5分前？
						if( UNSENT_TIME_BEFORE_SW == (GetDate - NowDate) ){
							// パラメータの切換えじゃない？
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//							if( remotedl_connect_type_get() != CTRL_PARAM_DL ){
							if( remotedl_connect_type_get() != CTRL_PARAM_DL || remotedl_connect_type_get() != CTRL_PARAM_DIF_DL){
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
								// データ再送待ちタイマーキャンセル
								RAU_Cancel_RetryTimer();
								// 未送信データ送信
				 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
									NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL, 0);
								}
								else {
									NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
								}
							}
						}
					}
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正
				}
			}
			
			if( res ){														// 切替があり
				remotedl_exec_info_set( FLASH_WRITE_BEFORE );				// 詳細情報をリセット（リセットによる処理中止）
				remotedl_status_set( R_SW_START );							// プログラム更新開始
				
				dispclr();		

				if( opncls() == 1 ){	// 営業中
					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[6]);	/* "　　　メンテナンス中です　　　" */
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[7]);	/* "　　　只今精算できません　　　" */
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[7]);	/* "     しばらくお待ち下さい     " */
				} else {				// 休業中
					grachr(2, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR3[6]);	/* "　　　メンテナンス中です　　　" */
					grachr(4, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR3[7]);	/* "　　　只今精算できません　　　" */
					grachr(5, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[7]);	/* "     しばらくお待ち下さい     " */
				}
				LcdBackLightCtrl( ON );										// back light ON

				// 設定切換はここで抜ける
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//				if (remotedl_connect_type_get() == CTRL_PARAM_DL) {
				if (remotedl_connect_type_get() == CTRL_PARAM_DL || remotedl_connect_type_get() == CTRL_PARAM_DIF_DL) {
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
					break;
				}

				// 下り回線停止
				RAU_StopDownline();
				// lkcomtaskを止める
				LKcom_f_TaskStop = 1;

				// 自動送信一時停止
				ntautoStopReq();
// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード修正
				// データ再送待ちタイマーキャンセル
				RAU_Cancel_RetryTimer();
// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード修正

				// 未送信データ送信
 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL, 0);
				}
				else {
					NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
				}
				// 初期値セット
				sec_cnt = 0;
				res = 0;

				// 未送信データ送信完了を待つため、ここでループを回す。
				// 必要なイベントだけを抜き出すようにする為、他のイベントは保持されたままとする
				memset( &WaitMsgID, 0, sizeof( WaitMsgID ));
				WaitMsgID.Count = 3;
				WaitMsgID.Command[0] = IBK_EVT;
				WaitMsgID.Command[1] = CLOCK_CHG;
				WaitMsgID.Command[2] = TIMEOUT;

				while( 1 ){
					taskchg( IDLETSKNO );									// Change task to idletask
					msb = Target_MsgGet_Range( OPETCBNO, &WaitMsgID );		// 期待するﾒｰﾙだけ受信（他は溜めたまま）
					if( NULL == msb ){										// 期待するﾒｰﾙなし（未受信）
						continue;
					}
					//	受信ﾒｯｾｰｼﾞあり
					memcpy( &msg , msb , sizeof(MsgBuf) );					// 受信ﾒｯｾｰｼﾞ格納
					FreeBuf( msb );											// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
					sc.comd = msg.msg.command;

					switch( sc.cc[0] ){
					case MSGGETHIGH(CLOCK_CHG):								// 分歩進イベント
						// ここでのCLOCK_CHGイベント処理は最低限の処理しかしない。
						// SUBCPU_MONITはここでクリアしないと、E0008が発生するのでクリアする
						ClkrecUpdate( &msg.msg.data[0] );
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						//LCDﾊﾞｯｸﾗｲﾄ動作ｶｳﾝﾄ更新
//						if(SD_LCD){
//							inc_dct(LCD_LIGTH_CNT,1);
//						}
						// QRリーダー動作時間
						if( QR_READER_USE ){
							inc_dct(QR_READER_CNT,1);
						}
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
						SUBCPU_MONIT = 0;									// ｻﾌﾞCPU監視ﾌﾗｸﾞｸﾘｱ
						err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 0, 0, 0 );	// ｴﾗｰ解除
						break;
					case MSGGETHIGH(IBK_EVT):								// IBK event
						if(( sc.comd == IBK_CTRL_OK ) || ( sc.comd == IBK_CTRL_NG )){
							// 1sタイマ起動
							Lagtim( OPETCBNO, 31, 50 );
						}
						break;
					case MSGGETHIGH(TIMEOUT):
						if (sc.comd == TIMEOUT31) {
							// 1s毎に未送信データの件数をチェックする
							count = remotedl_unsent_data_count_get();
							// 未送信データが0になるまで更新処理を行わない
							if (!count) {
								queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
								res = 1;
							}
							// 上り回線の送信シーケンスキャンセルタイマ(300s)経過しても
							// 未送信データが0件にならない場合は更新処理失敗とする
							else if (++sec_cnt >= UNSENT_DATA_RETRY_COUNT) {
								rmon_regist(RMON_PRG_SW_END_UNSENT_DATA_NG);
// GG120600(S) // Phase9 種別毎をチェック
//								remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//								remotedl_start_retry_clear();
								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
								switch (remotedl_connect_type_get()) {
								case CTRL_PROG_DL:
									remotedl_complete_request(REQ_KIND_VER_UP);
									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
									break;
								case CTRL_PROG_SW:
									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
									break;
								default:
									break;
								}
								// 未送信データの送信失敗のためリセット
								System_reset();
							}
							else {
								// 1sタイマ起動
				 				Lagtim( OPETCBNO, 31, 50 );
							}
						}
						break;
					default:
						break;
					}
					if( res ){
						break;
					}
				}
			}
			break;
		case R_UP_WAIT:					// アップロード待ち
			up_wait_chk( &dl->dl_info[PARAM_UP_TIME].start_time, R_UP_START );
			break;
		case R_RESET_WAIT:				// リセット待ち
			reset_start_chk( &dl->dl_info[RESET_TIME].start_time, R_RESET_START );
			break;
		default:
			break;
	}
// GG120600(S) // Phase9 設定変更通知対応
	// メンテで変更フラグがのこっているか一応チェック（設定変更後にメンテをぬけずに電源OFFON）
	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE && OPECTL.Mnt_mod == 0){
		// parkingWeb接続あり
		if(_is_ntnet_remote()) {
			// 端末で設定パラメータ変更
			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
			// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
			remotedl_cancel_setting();
		}
		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
	}
// GG120600(E) // Phase9 設定変更通知対応
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード実行判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		rcvdata	:要求種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_dl_check( uchar	*rcvdata )
{
	uchar	err = REQ_ACCEPT;
	t_ProgDlReq *p;
	BOOL bRT = FALSE;

	p = (t_ProgDlReq *)rcvdata;
	// 受付不可の場合、読み捨てる
	if (remotedl_check_request(p->ReqKind) == 0) {
		// 監視データを登録
// GG120600(S) // Phase9 遠隔監視データ変更
//		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		return;
	}

	// 処理種別
	switch (p->ProcKind) {
	case PROC_KIND_EXEC_RT:					// 即時実行
		bRT = TRUE;
		// no break
	case PROC_KIND_EXEC:					// 予約登録
		err = remotedl_proc_resv_exec(p, bRT);
		if (err == REQ_ACCEPT) {
			// スクリプトファイルなしでダウンロード・アップロードを実施する
			remotedl_script_typeset( REMOTE_NO_SCRIPT );
		}
		break;
	case PROC_KIND_CANCEL:					// 取消
// GG120600(S) // Phase9 遠隔監視データ変更
//		err = remotedl_proc_cancel(p->ReqKind, &p->InstNo1, FALSE);
		err = remotedl_proc_cancel(p->ReqKind, &p->InstNo1, FALSE,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
//	case PROC_KIND_CHG_TIME:				// 時刻変更
//		err = remotedl_proc_chg_time(p);
//		break;
	case PROC_KIND_RESV_INFO:				// 要求確認
		err = remotedl_proc_resv_info(p);
		break;
	default:
		err = REQ_NOT_ACCEPT;
		// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	}

	if (err == REQ_ACCEPT && p->ReqKind == FTP_TEST_REQ) {
		// 接続テストは、即時実行
		queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
//		memcpy(&g_bk_ProgDlReq, p, sizeof(t_ProgDlReq));
//		// 応答はテスト接続後に返信
		return;
	}

//	p->ReqResult = err;
//	NTNET_Snd_Data118_DL(p);

	// 即時実行の場合、イベント送信して開始判定を行う
	if (p->ProcKind == PROC_KIND_EXEC_RT) {
		queset( OPETCBNO, REMOTE_DL_END, 0, NULL );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス予約登録
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: 遠隔メンテナンス要求電文
///	@param[in]		bRT	: 即時実行フラグ
///	@return			REQ_NOT_ACCEPT = 失敗
///					REQ_ACCEPT = 成功
///					REQ_PROGNO_ERR = プログラム部番異常
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_resv_exec(t_ProgDlReq *p, BOOL bRT)
{
	uchar err = REQ_ACCEPT;
	t_FtpInfo *pFtpInfo;
	int i,nType,len;
	date_time_rec	timeTemp;
	uchar			scriptTemp[12];
	u_ipaddr		ftpaddrTemp;
	ulong			offset = 0;
// MH810103(s) 電子マネー対応 部番チェックを外す
//// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
//	eVER_TYPE		now_type, req_type;
//// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
// MH810103(e) 電子マネー対応 部番チェックを外す
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
	u_inst_no*		pinstTemp = remotedl_instNo_get();
	u_inst_no		instTemp;
	uchar			instfrom = remotedl_instFrom_get();

	// この時点で保持しておく
	memcpy(&instTemp,pinstTemp,sizeof(u_inst_no));
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

	// 即時実行であればoffsetをセット
	if (bRT) {
		offset = RMON_OFFSET_REAL_TIME;
	}

	// 要求種別
	switch( p->ReqKind ){
	case VER_UP_REQ:																	// バージョンアップ要求
	case PROGNO_CHG_REQ:																// 部番変更要求
		if( NOT_DOWNLOADING() ){
			
			do {
				// ***  要求チェック開始 **** //
				// ダウンロード開始時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}
				// 更新時刻
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0年0月は現在日時として扱う
						err = REQ_NOT_ACCEPT;							// 異常値なので受付不可で返す
						break;
					}
				}
				// プログラム部番
				memset(scriptTemp, 0, sizeof(scriptTemp));
				memcpy(scriptTemp, p->Prog_No, sizeof(p->Prog_No));
				len = strlen((char *)scriptTemp);
				memcpy(&scriptTemp[len], p->Prog_Ver, sizeof(p->Prog_Ver));

// MH810103(s) 電子マネー対応 部番チェックを外す
//				if (p->ReqKind == VER_UP_REQ) {
//// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
////					// 「プログラム部番」のチェック
////					if( strncmp((char*)scriptTemp, (char*)VERSNO.ver_part, len) != 0 ){
////						err = REQ_PROGNO_ERR;											// 「要求メインプログラム部番異常」
////						break;
////					}
////					// 「プログラムバージョン」のチェック
////					if( strncmp((char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
////						err = REQ_PROGNO_ERR;
////						break;
////					}
//					req_type = GetVersionPrefix( scriptTemp, sizeof(scriptTemp) );						// 要求部番プレフィックス
//					now_type = GetVersionPrefix( (uchar*)VERSNO.ver_part, sizeof(VERSNO.ver_part) );	// 現在部番プレフィックス
//					if( req_type == now_type ){
//						if( req_type == VER_TYPE_GW || req_type == VER_TYPE_GG ){
//							// 「プログラム部番」のチェック(部番同じ？) = GGなどの場合、部番が違うのは実行
//							if( strncmp( (char*)scriptTemp, (char*)VERSNO.ver_part, len) == 0 ){
//								// 「プログラムバージョン」のチェック(古い？)
//								if( strncmp( (char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
//									err = REQ_PROGNO_ERR;
//									break;
//								}
//							}
//						}else{
//							// 「プログラム部番」のチェック
//							if( strncmp( (char*)scriptTemp, (char*)VERSNO.ver_part, len) != 0 ){
//								err = REQ_PROGNO_ERR;											// 「要求メインプログラム部番異常」
//								break;
//							}
//							// 「プログラムバージョン」のチェック
//							if( strncmp( (char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
//								err = REQ_PROGNO_ERR;
//								break;
//							}
//						}
//					}else{
//						// 現状、個別部番⇒標準、標準から個別等は禁止とする
//						if( now_type == VER_TYPE_GM || req_type == VER_TYPE_GM ){
//							err = REQ_PROGNO_ERR;											// 「要求メインプログラム部番異常」
//						}
//						// 想定外の部番には変更できないようにしておく
//						if( req_type == VER_TYPE_MAX ){
//							err = REQ_PROGNO_ERR;											// 「要求メインプログラム部番異常」
//						}
//					}
//// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
//				}
// MH810103(e) 電子マネー対応 部番チェックを外す
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //

				remotedl_recv_info_clear(PROG_DL_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PROG_DL_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(PROG_DL_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[PROG_DL_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[PROG_DL_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// ダウンロード開始時刻を設定
				if (bRT) {
					// 即時実行は現在時刻をセット
					memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PROG_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// 更新時刻を設定
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0年0月は現在時刻として扱う
					memcpy( &chg_info.sw_info[SW_PROG].sw_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.sw_info[SW_PROG].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PROG].sw_time.Mon, &p->SW_Mon, 4 );
				}

				// プログラム部番
				memcpy(&chg_info.script[PROGNO_KIND_DL], scriptTemp, sizeof(scriptTemp));

				// IPアドレス
				chg_info.dl_info[PROG_DL_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTPポート番号
				chg_info.dl_info[PROG_DL_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ユーザ
				strncpy((char *)chg_info.dl_info[PROG_DL_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// パスワード
				strncpy((char *)chg_info.dl_info[PROG_DL_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PRG);
				retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PROG_DL_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				remotedl_accept_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_connect_type_set( CTRL_PROG_DL );
//				remotedl_status_set( R_DL_REQ_RCV );									// 開始予約状態としてダウンロード開始時刻になるまでまつ
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_START, PROG_DL_TIME, &chg_info.dl_info[PROG_DL_TIME].start_time);
				remotedl_time_set(INFO_KIND_SW, PROG_DL_TIME, &chg_info.sw_info[SW_PROG].sw_time);

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRG_DL_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRG_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
				if (err == REQ_NOT_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//					rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				}
				else {
// GG120600(S) // Phase9 遠隔監視データ変更
//					rmon_regist_ex(RMON_PRG_DL_REQ_PRGNO_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_DL_REQ_PRGNO_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				}
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case PARAM_CHG_REQ:																	// 設定変更要求
		if( NOT_DOWNLOADING() ){
			do {
				// ***  要求チェック開始 **** //
				// ダウンロード開始時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}
				// 更新時刻
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0年0月は現在日時として扱う
						err = REQ_NOT_ACCEPT;							// 異常値なので受付不可で返す
						break;
					}
				}
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //
				
				remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_DL_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(PARAM_DL_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[PARAM_DL_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[PARAM_DL_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// ダウンロード開始時刻を設定
				if (bRT) {
					// 即時実行は現在時刻をセット
					memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PARAM_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// 更新時刻を設定
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0年0月は現在時刻として扱う
					memcpy( &chg_info.sw_info[SW_PARAM].sw_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
				}

				// IPアドレス
				chg_info.dl_info[PARAM_DL_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTPポート番号
				chg_info.dl_info[PARAM_DL_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ユーザ
				strncpy((char *)chg_info.dl_info[PARAM_DL_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// パスワード
				strncpy((char *)chg_info.dl_info[PARAM_DL_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
				retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//				remotedl_accept_request(REQ_KIND_PARAM_CHG);
//				remotedl_connect_type_set( CTRL_PARAM_DL );
				if( p->ReqKind == PARAM_CHG_REQ ){
// GG120600(S) // Phase9 設定変更通知対応
//					remotedl_connect_type_set( CTRL_PARAM_DL );		// 種別セット
// GG120600(E) // Phase9 設定変更通知対応
					remotedl_accept_request(REQ_KIND_PARAM_CHG);	// 設定変更要求
				}else{
// GG120600(S) // Phase9 設定変更通知対応
//					remotedl_connect_type_set( CTRL_PARAM_DIF_DL );
// GG120600(E) // Phase9 設定変更通知対応
					remotedl_accept_request(REQ_KIND_PARAM_DIF_CHG);	// 差分設定変更要求
				}
// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_status_set( R_DL_REQ_RCV );									// 開始予約状態としてダウンロード開始時刻になるまでまつ
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_START, PARAM_DL_TIME, &chg_info.dl_info[PARAM_DL_TIME].start_time);
				remotedl_time_set(INFO_KIND_SW, PARAM_DL_TIME, &chg_info.sw_info[SW_PARAM].sw_time);

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // Phase9 設定変更通知対応 #4960
////				rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // Phase9 設定変更通知対応 #4960
				rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // Phase9 設定変更通知対応 #4960
////			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // Phase9 設定変更通知対応 #4960
			rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case PARAM_DIFF_CHG_REQ:																	// 設定変更要求
		if( NOT_DOWNLOADING() ){
			do {
				// ***  要求チェック開始 **** //
				// ダウンロード開始時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}
				// 更新時刻
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0年0月は現在日時として扱う
						err = REQ_NOT_ACCEPT;							// 異常値なので受付不可で返す
						break;
					}
				}
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //
				
				remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_DL_DIF_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(PARAM_DL_DIF_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[PARAM_DL_DIF_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[PARAM_DL_DIF_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// ダウンロード開始時刻を設定
				if (bRT) {
					// 即時実行は現在時刻をセット
					memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// 更新時刻を設定
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0年0月は現在時刻として扱う
// GG120600(S) // Phase9 パラメータ切替を分ける
//					memcpy( &chg_info.sw_info[SW_PARAM].sw_time, &CLK_REC, sizeof(date_time_rec) );
					memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time, &CLK_REC, sizeof(date_time_rec) );
// GG120600(E) // Phase9 パラメータ切替を分ける
				}
				else {
// GG120600(S) // Phase9 パラメータ切替を分ける
//					chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
//					memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
					chg_info.sw_info[SW_PARAM_DIFF].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time.Mon, &p->SW_Mon, 4 );
// GG120600(E) // Phase9 パラメータ切替を分ける
				}

				// IPアドレス
				chg_info.dl_info[PARAM_DL_DIF_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTPポート番号
				chg_info.dl_info[PARAM_DL_DIF_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ユーザ
				strncpy((char *)chg_info.dl_info[PARAM_DL_DIF_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// パスワード
				strncpy((char *)chg_info.dl_info[PARAM_DL_DIF_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
				retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				if( p->ReqKind == PARAM_CHG_REQ ){
// GG120600(S) // Phase9 設定変更通知対応
//					remotedl_connect_type_set( CTRL_PARAM_DL );
// GG120600(E) // Phase9 設定変更通知対応
					remotedl_accept_request(REQ_KIND_PARAM_CHG);
				}else{
// GG120600(S) // Phase9 設定変更通知対応
//					remotedl_connect_type_set( CTRL_PARAM_DIF_DL );
// GG120600(E) // Phase9 設定変更通知対応
					remotedl_accept_request(REQ_KIND_PARAM_DIF_CHG);
				}
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_status_set( R_DL_REQ_RCV );									// 開始予約状態としてダウンロード開始時刻になるまでまつ
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_START, PARAM_DL_DIF_TIME, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time);
// GG120600(S) // Phase9 パラメータ切替を分ける
//				remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
				remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM_DIFF].sw_time);
// GG120600(E) // Phase9 パラメータ切替を分ける

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_OK+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////				rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
			rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
// GG120600(E) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)

	case PARAM_UPLOAD_REQ:																// 設定要求
// GG124100(S) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
// 		if( NOT_DOWNLOADING() ){														// 遠隔ダウンロード未実行
		if ( NOT_DOWNLOADING() && (OPECTL.lcd_prm_update == 0) ) {	// 遠隔ダウンロード未実行 ＆ LCDへのパラメータアップロード中ではない
// GG124100(E) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
			do {
				// ***  要求チェック開始 **** //
				// アップロード開始時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //

				remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_UP_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(PARAM_UP_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[PARAM_UP_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[PARAM_UP_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// アップロード開始時刻を設定
				if (bRT) {
					// 即時実行は現在時刻をセット
					memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time, &timeTemp, sizeof(timeTemp) );
				}

				// IPアドレス
				chg_info.dl_info[PARAM_UP_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTPポート番号
				chg_info.dl_info[PARAM_UP_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ユーザ
				strncpy((char *)chg_info.dl_info[PARAM_UP_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// パスワード
				strncpy((char *)chg_info.dl_info[PARAM_UP_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
				retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				remotedl_accept_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_connect_type_set( CTRL_PARAM_UPLOAD );
//				remotedl_status_set( R_UP_WAIT );
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_START, PARAM_UP_TIME, &chg_info.dl_info[PARAM_UP_TIME].start_time);

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRM_UP_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_UP_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case RESET_REQ:
		if( NOT_DOWNLOADING() ){														// 遠隔ダウンロード未実行
			do{
				// ***  要求チェック開始 **** //
				// リセット開始時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}
				// ***  要求チェック終了 **** //

				remotedl_recv_info_clear(RESET_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(RESET_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(RESET_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[RESET_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[RESET_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// リセット開始時刻を設定
				if (bRT) {
					memcpy( &chg_info.dl_info[RESET_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					memcpy( &chg_info.dl_info[RESET_TIME].start_time, &timeTemp, sizeof(timeTemp) );
				}

			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				remotedl_accept_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_connect_type_set( CTRL_RESET );
//				remotedl_status_set( R_RESET_WAIT );										// リセット待ち
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_START, RESET_TIME, &chg_info.dl_info[RESET_TIME].start_time);

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_RESET_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_RESET_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1);
				rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
		else{
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case FTP_TEST_REQ:
		// メンテナンス中か？ダウンロード中か？
		if (OPECTL.Mnt_mod != 0 || DOWNLOADING() ) {
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1);
			rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}else{
			do {
				// ***  要求チェック開始 **** //
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //
				remotedl_recv_info_clear(TEST_TIME);

				// 命令番号
				memcpy(&chg_info.dl_info[TEST_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[TEST_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// IPアドレス
				chg_info.dl_info[TEST_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTPポート番号
				chg_info.dl_info[TEST_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ユーザ
				strncpy((char *)chg_info.dl_info[TEST_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// パスワード
				strncpy((char *)chg_info.dl_info[TEST_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);
			if (err == REQ_ACCEPT) {
				remotedl_chg_info_bkup();														// バックアップ
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 リトライの種別を分ける
				remotedl_accept_request(REQ_KIND_TEST);
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_connect_type_set( CTRL_CONNECT_CHK );
//				remotedl_status_set( R_TEST_CONNECT );
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
		break;
	case PROG_ONLY_CHG_REQ:					// プログラム切替要求
		if( NOT_DOWNLOADING() ){														// 遠隔ダウンロード未実行
			do {
				// ***  要求チェック開始 **** //
				// 更新時刻
				// 即時実行時はチェックしない
				if (!bRT) {
					timeTemp.Year = (ushort)(p->SW_Year+2000);
					memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// 年月日時分のチェック
						err = REQ_NOT_ACCEPT;								// 異常値なので受付不可で返す
						break;
					}
				}

				// プログラムデータチェック
				if (!IsMainProgExist()) {
					err = REQ_SW_PROG_NONE;								// プログラムデータがない
					break;
				}

				// 部番チェック
				memset(scriptTemp, 0, sizeof(scriptTemp));
				memcpy(scriptTemp, p->Prog_No, sizeof(p->Prog_No));
				len = strlen((char *)scriptTemp);
				memcpy(&scriptTemp[len], p->Prog_Ver, sizeof(p->Prog_Ver));

				if( strncmp((char*)scriptTemp, (char*)ver_datwk_prog, len+sizeof(p->Prog_Ver)) != 0 ){
					err = REQ_SW_PROGNO_ERR;							// 部番不一致
					break;
				}
				// ***  要求チェック終了 **** //
				remotedl_recv_info_clear(PROG_ONLY_TIME);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 設定変更通知対応
////				// @todo phase2で削除すること!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PROG_ONLY_TIME);
//// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_info_clear(PROG_ONLY_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

				// 命令番号
				memcpy(&chg_info.dl_info[PROG_ONLY_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 遠隔監視データ変更
				// 要求元
				chg_info.dl_info[PROG_ONLY_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 遠隔監視データ変更

				// 更新時刻
				if (bRT) {
					memcpy(&chg_info.sw_info[SW_PROG_EX].sw_time, &CLK_REC, sizeof(date_time_rec));
				}
				else {
					memcpy(&chg_info.sw_info[SW_PROG_EX].sw_time, &timeTemp, sizeof(timeTemp));
				}

				// プログラム部番
				memcpy(&chg_info.script[PROGNO_KIND_SW], scriptTemp, sizeof(scriptTemp));

			} while (0);
			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PROG_ONLY_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
				remotedl_accept_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//				remotedl_connect_type_set( CTRL_PROG_SW );
//				remotedl_status_set( R_SW_WAIT );
// GG120600(E) // Phase9 設定変更通知対応
				remotedl_time_set(INFO_KIND_SW, PROG_ONLY_TIME, &chg_info.sw_info[SW_PROG_EX].sw_time);
				// プログラム切換フラグをON
				chg_info.sw_status[SW_PROG] = 1;
				chg_info.sw_info[SW_PROG_EX].exec = 1;

				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRG_SW_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRG_SW_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
				remotedl_monitor_info_set(0);
			}
			else {
				// 監視データ登録
				switch (err) {
				case REQ_NOT_ACCEPT:
// GG120600(S) // Phase9 遠隔監視データ変更
//					rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
					break;
				case REQ_SW_PROG_NONE:
// GG120600(S) // Phase9 遠隔監視データ変更
//					rmon_regist_ex(RMON_PRG_SW_REQ_PRG_NONE_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_PRG_NONE_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
					break;
				case REQ_SW_PROGNO_ERR:
// GG120600(S) // Phase9 遠隔監視データ変更
//					rmon_regist_ex(RMON_PRG_SW_REQ_PRGNO_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_PRGNO_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
					break;
				}
			}
		}else{
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case FTP_CHG_REQ:					// FTP設定変更要求
		if( NOT_DOWNLOADING() ){														// 遠隔ダウンロード未実行
			do {
				// ***  要求チェック開始 **** //
				// IPアドレス
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ユーザ
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// パスワード
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  要求チェック終了 **** //

				for( nType = 0; nType  < TIME_MAX; nType++) {
					// もともといずれかが空であれば、セットしない
					if( chg_info.dl_info[nType].ftpuser[0] == '\0' || chg_info.dl_info[nType].ftppasswd[0] == '\0'){
						continue;
					}
					// IPアドレス
					chg_info.dl_info[nType].ftpaddr.uladdr = ftpaddrTemp.uladdr;

					// FTPポート番号
					chg_info.dl_info[nType].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

					// ユーザ
					strncpy((char *)chg_info.dl_info[nType].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

					// パスワード
					strncpy((char *)chg_info.dl_info[nType].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
				}
			} while (0);

			if (err == REQ_ACCEPT) {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_FTP_CHG_OK, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_CHG_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
			else {
				// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}else{
			err = REQ_NOT_ACCEPT;
			// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1);
			rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	default:
		err = REQ_NOT_ACCEPT;
		// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス予約中止
//[]----------------------------------------------------------------------[]
///	@param[in]		ReqKind		: 要求種別
///	@param[in]		*pInstNo	: 命令番号
///	@return			REQ_NOT_ACCEPT = 失敗
///					REQ_ACCEPT = 成功
///					REQ_RCV_NONE = 未受信
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 遠隔監視データ変更
//uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt)
uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt,uchar from)
// GG120600(E) // Phase9 遠隔監視データ変更
{
	uchar err = REQ_RCV_NONE;
// GG120600(S) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
//	uchar connect = remotedl_connect_type_get();
// GG129000(S) R.Endo 2023/01/18 車番チケットレス4.0 #6804 差分設定ダウンロード中の予約取消で取消が成功する [共通改善項目 No1545]
// 	uchar connect = remotedl_request_kind_to_connect_type(remotedl_connect_type_get());
	uchar connect = remotedl_connect_type_get();
// GG129000(E) R.Endo 2023/01/18 車番チケットレス4.0 #6804 差分設定ダウンロード中の予約取消で取消が成功する [共通改善項目 No1545]
// GG120600(E) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
// GG120600(S) // Phase9 結果を区別する
//	uchar ret = remotedl_result_get(RES_DL);
	uchar ret = EXCLUDED;
	switch(connect){
	case CTRL_PROG_DL:					// プログラムダウンロード
		ret = remotedl_result_get(RES_DL_PROG);
		break;
	case CTRL_PARAM_DL:					// 設定ダウンロード
		ret = remotedl_result_get(RES_DL_PARAM);
		break;
	case CTRL_PARAM_UPLOAD:				// 設定アップロード
		ret = remotedl_result_get(RES_UP);
		break;
	case CTRL_PROG_SW:					// プログラム切換
		ret = remotedl_result_get(RES_SW_PROG);
		break;
	case CTRL_PARAM_DIF_DL:				// 差分設定ダウンロード
		ret = remotedl_result_get(RES_DL_PARAM_DIF);
		break;
	case CTRL_NONE:
	case CTRL_RESET:					// リセット
	case CTRL_CONNECT_CHK:				// 接続テスト
		break;
	}
// GG120600(E) // Phase9 結果を区別する

	// 要求種別
	switch (ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			// バージョンアップ要求処理中
			if (connect == CTRL_PROG_DL) {
				// ダウンロード実行中？
				// プログラム切換中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				// 設定変更要求処理中の場合は結果を再セット
// GG120600(S) // Phase9 結果を区別する
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
////				if (connect == CTRL_PARAM_DL) {
//				if (connect == CTRL_PARAM_DL || connect == CTRL_PARAM_DIF_DL) {
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
//					chg_info.result[RES_DL] = ret;
//				}
				if (connect == CTRL_PARAM_DL) {
					chg_info.result[RES_DL_PARAM] = ret;
				}else if(connect == CTRL_PARAM_DIF_DL ){
					chg_info.result[RES_DL_PARAM_DIF] = ret;
				}
// GG120600(E) // Phase9 結果を区別する
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_DL_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRG_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PROG_DL_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRG_DL_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRG_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PROG_DL_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
	case PARAM_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			// 設定変更要求処理中
			if (connect == CTRL_PARAM_DL) {
				// ダウンロード実行中？
				// 設定切換中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				// バージョンアップ要求処理中の場合は結果を再セット
				if (connect == CTRL_PROG_DL) {
// GG120600(S) // Phase9 結果を区別する
//					chg_info.result[RES_DL] = ret;
					chg_info.result[RES_DL_PROG] = ret;
// GG120600(E) // Phase9 結果を区別する
				}
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
// MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	case PARAM_DIFF_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {	// 要求受付チェック
			// 設定変更要求処理中
			if ( connect == CTRL_PARAM_DIF_DL ) {
				// ダウンロード実行中？
				// 設定切換中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				// バージョンアップ要求処理中の場合は結果を再セット
				if (connect == CTRL_PROG_DL) {
// GG120600(S) // Phase9 結果を区別する
//					chg_info.result[RES_DL] = ret;
					chg_info.result[RES_DL_PROG] = ret;
// GG120600(E) // Phase9 結果を区別する
				}
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//// MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo);
//			rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_OK, pInstNo);
//// MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
			rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo);
//				rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_NG, pInstNo);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
				rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
// MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	case PARAM_UPLOAD_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			// 設定要求(アップロード)処理中
			if (connect == CTRL_PARAM_UPLOAD) {
				// アップロード実行中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRM_UP_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRM_UP_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRM_UP_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRM_UP_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
	case RESET_REQ:
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			// リセット要求処理中
			if (connect == CTRL_RESET) {
				// リセット実行中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_RESET_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_RESET_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[RESET_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_RESET_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_RESET_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[RESET_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
	case PROG_ONLY_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PROG_ONLY_CHG)) {
			// プログラム切換要求処理中
			if (connect == CTRL_PROG_SW) {
				// プログラム切換中？
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// 要求取消
					remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//					// 待機中に遷移
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 設定変更通知対応
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// 要求取消
				remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 設定変更通知対応
//				// 種別を再セット
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 設定変更通知対応
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// 監視データ登録
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_SW_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRG_SW_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PROG_ONLY_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// メンテナンス時はNGの監視データを登録しない
			if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//				rmon_regist_ex(RMON_PRG_SW_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRG_SW_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
			}
		}
// GG120600(S) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		else if( err == REQ_RCV_NONE){
			// remotedl_is_acceptから削除されているため
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PROG_ONLY_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 遠隔メンテナンス確認画面で実行済み及びエラーのステータスのデータがクリアされない
		break;
	default:
		err = REQ_CANCEL_NOT_ACCEPT;
		// メンテナンス時はNGの監視データを登録しない
		if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	}
	if (err == REQ_RCV_NONE) {
		// メンテナンス時はNGの監視データを登録しない
		if (!bMnt) {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo,from);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス時刻変更
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: 遠隔メンテナンス要求電文
///	@return			REQ_NOT_ACCEPT = 失敗
///					REQ_ACCEPT = 成功
///					REQ_RCV_NONE = 未受信
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_chg_time(t_ProgDlReq *p)
{
	uchar err = REQ_RCV_NONE;

	// 要求種別
	switch (p->ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			// ダウンロード開始時刻を設定
			chg_info.dl_info[PROG_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// 更新時刻を設定
			chg_info.sw_info[SW_PROG].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PROG].sw_time.Mon, &p->SW_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PROG_DL_TIME, &chg_info.dl_info[PROG_DL_TIME].start_time);
			remotedl_time_set(INFO_KIND_SW, PROG_DL_TIME, &chg_info.sw_info[SW_PROG].sw_time);
		}
		break;
	case PARAM_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			// ダウンロード開始時刻を設定
			chg_info.dl_info[PARAM_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// 更新時刻を設定
			chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PARAM_DL_TIME, &chg_info.dl_info[PARAM_DL_TIME].start_time);
			remotedl_time_set(INFO_KIND_SW, PARAM_DL_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
		}
		break;
// MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	case PARAM_DIFF_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {
			// ダウンロード開始時刻を設定
			chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// 更新時刻を設定
// GG120600(S) // Phase9 パラメータ切替を分ける
//			chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
//			memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
			chg_info.sw_info[SW_PARAM_DIFF].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time.Mon, &p->SW_Mon, 4 );
// GG120600(E) // Phase9 パラメータ切替を分ける

			remotedl_time_set(INFO_KIND_START, PARAM_DL_DIF_TIME, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time);
// GG120600(S) // Phase9 パラメータ切替を分ける
//			remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
			remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM_DIFF].sw_time);
// GG120600(E) // Phase9 パラメータ切替を分ける
		}
		break;
// MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)

	case PARAM_UPLOAD_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			// 開始時刻を設定
			chg_info.dl_info[PARAM_UP_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time.Mon, &p->DL_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PARAM_UP_TIME, &chg_info.dl_info[PARAM_UP_TIME].start_time);
		}
		break;
	case RESET_REQ:
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			// 開始時刻を設定
			chg_info.dl_info[RESET_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[RESET_TIME].start_time.Mon, &p->DL_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, RESET_TIME, &chg_info.dl_info[RESET_TIME].start_time);
		}
		break;
	default:
		err = REQ_NOT_ACCEPT;
		break;
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス予約確認
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: 遠隔メンテナンス要求電文
///	@return			REQ_NOT_ACCEPT = 失敗
///					REQ_ACCEPT = 成功
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_resv_info(t_ProgDlReq *p)
{
	uchar err = REQ_NOT_ACCEPT;

	// 要求種別
	switch (p->ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_DL_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case PARAM_CHG_REQ:
		// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRM_DL_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	case PARAM_DIFF_CHG_REQ:
			// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////			rmon_regist_ex(REQ_KIND_PARAM_DIF_CHG, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DIF_DL_CHECK_OK, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
			rmon_regist_ex(RMON_PRM_DIF_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
////			rmon_regist_ex(REQ_KIND_PARAM_DIF_CHG, &p->InstNo1);
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
// GG120600(E) // MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	case PARAM_UPLOAD_REQ:
		// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRM_UP_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_UP_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case RESET_REQ:
		// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_RESET_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_RESET_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	case PROG_ONLY_CHG_REQ:
		// 要求受信済み？
		if (remotedl_is_accept(REQ_KIND_PROG_ONLY_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_PRG_SW_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_SW_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		else {
// GG120600(S) // Phase9 遠隔監視データ変更
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		}
		break;
	default:
		// 監視データ登録
// GG120600(S) // Phase9 遠隔監視データ変更
//		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	}
	return err;
}
//[]----------------------------------------------------------------------[]
///	@brief			プログラム更新時刻のチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		time	: チェック対象時刻
///	@return			ret		: 0：範囲外(NG) 1：範囲内(OK)
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar remote_date_chk( date_time_rec *pTime )
{
	uchar	ret = 0;
	ushort	ndat;

	// NULLチェック
	if (pTime == NULL) {
		return ret;
	}

	// 年月日チェック
	if( chkdate2( (short)pTime->Year,(short)pTime->Mon,(short)pTime->Day ) == 0 ){
		// 時間チェック
		if( pTime->Hour <= 23 ){
			if( pTime->Min <= 59 ){
				// 現在年月日より未来の年月日を受け付ける
				ndat = dnrmlzm((short)pTime->Year, (short)pTime->Mon, (short)pTime->Day);
				if (CLK_REC.ndat <= ndat) {
					ret = 1;
				}
			}
		}
	}
	return ret;

}
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5796:前日以前の予定を表示しない)
uchar remote_date_chk_mnt( date_time_rec *pTime )
{
	uchar	ret = 0;

	// NULLチェック
	if (pTime == NULL) {
		return ret;
	}

	// 年月日チェック
	if( chkdate2( (short)pTime->Year,(short)pTime->Mon,(short)pTime->Day ) == 0 ) {
		// 時間チェック
		if( pTime->Hour <= 23 ) {
			if( pTime->Min <= 59 ) {
				ret = 1;
			}
		}
	}
	return ret;

}
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5796:前日以前の予定を表示しない)

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード/結果情報リトライ条件判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		time	:FTP開始時間（現在時刻との比較データ）
///	@param[in]		status	:ステータス情報（更新したいステータスを指定）
///	@return			ret		: 0：チェックNG 1：チェックOK（FTP通信開始）
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	dl_start_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;
	
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
//	if (!remote_date_chk(time)) {							// ダウンロード時刻が不正の場合チェックしない
	if (!remote_date_chk_mnt(time)) {
		// 接続リトライタイマー待ちの状態で分歩進イベントにより処理されないようにガードする
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
		return ret;
	}

	GetDate = Nrm_YMDHM( time );							// ダウンロード時刻ノーマライズ
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// 現在時刻ノーマライズ
	if( GetDate <= NowDate ){	 							// 一致または予定時刻を経過している
		do {
			// メンテナンス中か？
			if (OPECTL.Mnt_mod != 0) {
				// エラーの監視データを登録
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					rmon_regist(RMON_PRG_DL_START_MNT_NG);
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_DL_START_MNT_NG);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_DL_DIF_START_MNT_NG);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				default:
					break;
				}
				// 開始判定リトライ
				if (remotedl_dl_start_retry_check() < 0) {
					// リトライオーバー
// GG120600(S) // Phase9 種別毎をチェック
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					case CTRL_PARAM_DIF_DL:
						rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
					default:
						break;
					}
				}
				break;
			}
			// 開始時刻が一定時間経過していないか？
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
//					rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
// GG120600(S) // Phase9 (一定時間経過)を追加
					rmon_regist(RMON_PRG_DL_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (一定時間経過)を追加
					remotedl_complete_request(REQ_KIND_VER_UP);
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
					break;
				case CTRL_PARAM_DL:
//					rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
// GG120600(S) // Phase9 (一定時間経過)を追加
					rmon_regist(RMON_PRM_DL_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (一定時間経過)を追加
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 (一定時間経過)を追加
//					rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);
					rmon_regist(RMON_PRM_DL_DIF_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (一定時間経過)を追加
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				default:
					break;
				}
				break;
			}

			remotedl_status_set( status );					// FTP通信状態を更新
			remotedl_exec_info_set( FLASH_WRITE_BEFORE );	// ダウンロード状態を初期化(FLASH書込み前とする)
// GG120600(S) // Phase9 リトライの種別を分ける
//			remotedl_start_retry_clear();					// 開始判定リトライ回数クリア
			remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 リトライの種別を分ける

			// 監視データ登録
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				rmon_regist(RMON_PRG_DL_START_OK);
				break;
			case CTRL_PARAM_DL:
				rmon_regist(RMON_PRM_DL_START_OK);
				break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
			case CTRL_PARAM_DIF_DL:
				rmon_regist(RMON_PRM_DL_DIF_START_OK);
				break;				
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
			default:
				break;
			}
			queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
			ret = 1;
		} while (0);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード処理終了処理
//[]----------------------------------------------------------------------[]
///	@param[in]		no		:操作モニタ種別
///	@param[in]		result	:結果情報
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	moitor_regist( ushort no, uchar result )
{
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード中のメンテナンス処理抑止画面
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		:MOD_CHG : mode change<br>
///							:MOD_EXT : F5 key<br>
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
ushort	remotedl_disp( void )
{
	ushort	msg = 0;

	dsp_background_color(COLOR_WHITE);
	dispclr();														// Display All Clear
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[8]);								/* "　サーバーとデータ通信中です　" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[9]);								/* " メンテナンス操作はできません " */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[7]);								/* "     しばらくお待ち下さい     " */
	
	
	for( ;; ){
		msg = GetMessage();
		switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// 初期化状態へ
				OPECTL.init_sts = 0;							// 初期化未完了状態とする
				OPECTL.Pay_mod = 0;								// 通常精算
				OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
// MH810100(S) K.Onodera 2020/03/26 #4101 車番チケットレス（遠隔DL中のドア開閉でDL終了後に操作不能となる不具合修正）
//					OPECTL.Pay_mod = 0;								// 通常精算
//					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)-1;							// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//					OPECTL.PasswordLevel = (char)-1;
					op_wait_mnt_close();							// メンテナンス終了処理
// MH810100(E) K.Onodera 2020/03/26 #4101 車番チケットレス（遠隔DL中のドア開閉でDL終了後に操作不能となる不具合修正）
					return MOD_CHG;
				}
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				Lagtim( OPETCBNO, 4, 5*50 );
				ope_anm( AVM_CARD_ERR5 );						// 只今、お取り扱いができません
				// 自動排出なので、ここでは、吐き出しの処理はしない。
				break;

			case TIMEOUT4:
			case ARC_CR_EOT_EVT:
				rd_shutter();										// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				break;

			case REMOTE_DL_END:										// 遠隔ダウンロード処理終了
				return MOD_EXT;										// 抜ける

			default:
				break;
		}
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードのファイル情報設定処理
//[]----------------------------------------------------------------------[]
///	@param[in/out]	remote	:サーバー向けファイル名バッファポインタ
///	@param[in/out]	local	:内部仮想ファイル名バッファポインタ
///	@param[in]		type	:スクリプトファイル種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
const char *PATH_DATA[] = {
	"/"								// ルートディレクトリ
,	"/TMP/"							// 仮想ディレクトリ
,	"SCRIPT/"						// スクリプトファイル用ディレクトリ
,	"CUSTOM/"						// 手動更新用ディレクトリ
};

const char *AU_SCRIPT_FILE[] = {
	"FT-4000FX.TXT"					// FT4000FX標準バージョンアップ用スクリプト
,	(const char *)chg_info.script	// 手動更新用（Rismからの指定パスを使用）
,	"update10.txt"					// 既存更新用スクリプトファイル（既存ファイル名なので、小文字ファイル名とする）
,	"PARAMETER_UPLOAD.TXT"			// 共通パラメータアップロード用スクリプトファイル
,	"PARAMETER.TXT"					// 共通パラメータアップロードファイル
,	"CONNECT.TXT"					// 接続テスト
// MH810100(S) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
,	"PARAMETER.DAT"					// LCD向けパラメータアップロード
// MH810100(E) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
};

#define SCRIPT_FILE_INDEX_TEST			5

char *RESULT_FILE_PATH[] = {
	"/LOG/RemoteDLResult.txt"											// 内部用ファイルパス（共有）
,	"/RESULT/%s%06ld-%02ld_DL_RESULT_%04d%02d%02d%02d%02d.TXT"			// ダウンロード結果ファイルパス＋ファイル名
,	"/RESULT/%s%06ld-%02ld_SW_RESULT_%04d%02d%02d%02d%02d.TXT"			// 更新結果ファイルパス＋ファイル名
,	"/RESULT/%s%06ld-%02ld_CN_RESULT_%04d%02d%02d%02d%02d.TXT"			// 接続確認用ファイルパス＋ファイル名
};

void	MakeRemoteFileName( uchar *remote, uchar *local, char type )
{
	uchar	index = 0;
	switch( type ){
		case 0:																// スクリプトファイル情報作成
			strcpy((char*)local, PATH_DATA[1]);								// ローカル(仮想)のディレクトリをセット
			strcat((char*)local, AU_SCRIPT_FILE[chg_info.script_type]);		// ローカル(仮想)のファイル名をセット
			
			strcpy((char*)remote, PATH_DATA[0]);							// ファイルパス用に先頭に"/"をセット
			index++;														// インデックスをずらす
			if( chg_info.script_type != MANUAL_SCRIPT ){					// 既存のFTP通信以外
				strcpy((char*)&remote[index], PATH_DATA[2]);				// スクリプトの格納ディレクトリをセット
				index += 7;													// インデックスをずらす
			}

			if( chg_info.script_type == REMOTE_MANUAL_SCRIPT ){				// 手動更新時(個別⇔標準)
				strcpy((char*)&remote[index], PATH_DATA[3]);				// 手動用のスクリプトの格納ディレクトリをセット
				index += 7;													// インデックスをずらす
			}
			strcat((char*)&remote[index], AU_SCRIPT_FILE[chg_info.script_type]);	// ダウンロードするスクリプトファイル名をセット
			
			break;
		case 1:																// 遠隔ダウンロード結果ファイル情報作成
		case 2:																// 更新結果ファイル情報作成
		case 3:																// 接続確認情報作成
			memcpy( (char*)local, RESULT_FILE_PATH[0], 23);					// ローカル（仮想）パスをセット
			sprintf( (char*)remote, RESULT_FILE_PATH[type],					// アップロードするファイル名をセット
							(remotedl_result_get((uchar)(type-1))==0?"":"E"),
// TODO:デバイスIDは標準用の共通パラメータを取得すること
							0,
							prm_get( COM_PRM,S_PAY,2,2,1 ),					// 機械Noセット
							CLK_REC.year,									// アップロード年
							CLK_REC.mont,									// アップロード月
							CLK_REC.date,									// アップロード日
							CLK_REC.hour,									// アップロード時
							CLK_REC.minu									// アップロード分
							);
			break;
		case MAKE_FILENAME_SW:			// DLファイルパス(CWD)
			local[0] = '\0';

			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
// GG120600(S) // Phase9 サーバフォルダ構成変更対応
//// MH810100(S)
////				sprintf((char *)remote, "/BIN/FT4000/%s", chg_info.script);
//				sprintf((char *)remote, "/BIN/GT4100/%s", chg_info.script);
//// MH810100(E)
				sprintf((char *)remote, "/BIN/%03d/%s",NTNET_MODEL_CODE, chg_info.script[PROGNO_KIND_DL]);
// GG120600(E) // Phase9 サーバフォルダ構成変更対応
				break;
			case CTRL_PARAM_DL:
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
			case CTRL_PARAM_DIF_DL:
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_DOWN,(char*)remote);
				break;
			default:
				break;
			}
			break;
		case MAKE_FILENAME_PARAM_MKD:	// フォルダ名
			local[0] = '\0';
			if (remotedl_connect_type_get() == CTRL_PARAM_UPLOAD) {
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_UP,(char*)remote);
			}
			break;
		case MAKE_FILENAME_PARAM_UP:	// パラメータアップ名
			if (remotedl_connect_type_get() == CTRL_PARAM_UPLOAD) {
				sprintf((char*)local, "/%s", PARAM_FILE_NAME);
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_UP,(char*)remote);
				strcat((char*)remote, (char*)local);

				strcpy((char*)local, PATH_DATA[1]);								// ローカル(仮想)のディレクトリをセット
				strcat((char*)local, AU_SCRIPT_FILE[chg_info.script_type]);		// ローカル(仮想)のファイル名をセット
			}
			break;
		case MAKE_FILENAME_TEST_UP:		// 接続テスト用ファイル名
			if (remotedl_connect_type_get() == CTRL_CONNECT_CHK) {
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_TEST,(char*)remote);
				strcat((char*)remote, ".tst");

				strcpy((char*)local, PATH_DATA[1]);								// ローカル(仮想)のディレクトリをセット
				strcat((char*)local, AU_SCRIPT_FILE[SCRIPT_FILE_INDEX_TEST]);	// ローカル(仮想)のファイル名をセット
			}
			break;

// MH810100(S) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
		case MAKE_FILENAME_PARAM_UP_FOR_LCD:
			sprintf( (char*)remote, "%s", PARAM_BIN_FILE_NAME );	// 送信先ディレクトリ
			strcpy( (char*)local, PATH_DATA[1] );					// ローカル(仮想)のディレクトリをセット
			strcat( (char*)local, AU_SCRIPT_FILE[LCD_NO_SCRIPT] );	// ローカル(仮想)のファイル名をセット
			break;
// MH810100(E) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
		default:
			break;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード情報取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			char*	:スクリプトファイル名バッファポインタ
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
const char*	remotedl_script_get( void )
{
	return (AU_SCRIPT_FILE[chg_info.script_type]);
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードスクリプト種別設定
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: スクリプト種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_script_typeset( uchar type )
{
	chg_info.script_type = type;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードスクリプト種別取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			uchar	:スクリプト取得種別
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-04-04<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	remotedl_script_typeget( void )
{
	return	chg_info.script_type;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロードフラップ動作中チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		: 0：動作中のフラップなし 1：あり
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	flap_move_chk( void )
{
	short i;
	uchar ret = 0;
	
	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( FLPCTL.Flp_mv_tm[i] != 0 && FLPCTL.Flp_mv_tm[i] != -1 ){
			ret = 1;
			break;
		}
	}
	return ret;
}															

//[]----------------------------------------------------------------------[]
///	@brief			現在リトライ動作中の機能種別判定
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			status	: リトライステータス 0：リトライ中ではない ≠0 リトライ中
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar now_retry_active_chk(uchar kind)
{
	int i;
	uchar ret = 0;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			if (chg_info.retry[i].status&0x7f) {
				ret = 1;
				break;
			}
		}
	}
	else {
		if (chg_info.retry[kind].status&0x7f) {
			ret = 1;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ダウンロード前のエラー発生時の処理
//[]----------------------------------------------------------------------[]
///	@param[in]		Err		:エラー種別
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Before_DL_Err_Function( uchar Err )
{
	uchar status = remotedl_status_get();
// GG120600(S) // Phase9 リトライの種別を分ける
	uchar kind;
// GG120600(E) // Phase9 リトライの種別を分ける

	remotedl_result_set( Err );

	switch( status ){
		case	R_DL_EXEC:
			moitor_regist(OPLOG_REMOTE_DL_END, Err);
			if( remotedl_connect_type_get() == CTRL_CONNECT_CHK){
//				g_bk_ProgDlReq.ReqResult = REQ_CONN_NG;
//				NTNET_Snd_Data118_DL(&g_bk_ProgDlReq);
//				memset(&g_bk_ProgDlReq,0, sizeof(g_bk_ProgDlReq));
				rmon_regist(RMON_FTP_TEST_RES_NG);					// FTP接続テスト失敗
				remotedl_status_set(R_DL_IDLE);
				remotedl_chg_info_restore();						// 情報リストア
				break;
			}
			// エラーの監視データを登録
			rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);
			// リトライ
// GG120600(S) // Phase9 リトライの種別を分ける
//			if (retry_info_set(RETRY_KIND_CONNECT)) {
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				kind = RETRY_KIND_CONNECT_PRG;
				break;
			case CTRL_PARAM_DL:
				kind = RETRY_KIND_CONNECT_PARAM_DL;
				break;
			case CTRL_PARAM_DIF_DL:
				kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
				break;
			case CTRL_PARAM_UPLOAD:
				kind = RETRY_KIND_CONNECT_PARAM_UP;
				break;
			default:
				return;
				break;
			}
			if (retry_info_set(kind)) {
// GG120600(E) // Phase9 リトライの種別を分ける
				// リトライ情報クリア
// GG120600(S) // Phase9 リトライの種別を分ける
//				retry_info_clr(RETRY_KIND_MAX);
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PRG);
					retry_info_clr(RETRY_KIND_DL_PRG);
					break;
				case CTRL_PARAM_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
					retry_info_clr(RETRY_KIND_DL_PARAM);
					break;
				case CTRL_PARAM_DIF_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
					retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
					break;
				case CTRL_PARAM_UPLOAD:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
					retry_info_clr(RETRY_KIND_UL);
					break;
				}
// GG120600(E) // Phase9 リトライの種別を分ける
				// 受け付けた要求をクリアする
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_VER_UP);
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
				case CTRL_PARAM_UPLOAD:
					rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_UL);
					remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
					break;
				default:
					break;
				}
			}
// GG120600(S) // Phase9 種別毎をチェック
//			remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
			break;


		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			再起動時の結果情報送信待ち処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		SUBCPUは起動から一定時間間隔を空けてからでなければFTP<br>
///					通信が安定しない為、起動から90秒後に接続しにいく
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_result_snd_timer( void )
{
}

//[]----------------------------------------------------------------------[]
///	@brief			Rism接続切断待ちﾀｲﾑｱｳﾄ
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	connect_timeout( void )
{
	queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
}

//[]----------------------------------------------------------------------[]
///	@brief			Rism接続切断要求処理
//[]----------------------------------------------------------------------[]
///	@param[in]		logno	: 操作モニタ種別（切断or接続）
///	@param[in]		type	: 接続/切断種別(接続/切断を行う要因)
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_connect_req( ushort logno, uchar type )
{
	remotedl_connect_type_set( type );
	connect_timeout();
}

/*[]----------------------------------------------------------------------[]*/
///	@brief			ﾒｰﾙｷｭｰから狙ったﾒｰﾙを取得する（拡張）
//[]----------------------------------------------------------------------[]
///	@param[in]		id		: 取得ﾀｽｸﾅﾝﾊﾞｰ
///	@param[in]		pReq	: 検索対象ﾒｯｾｰｼﾞID（ﾒｯｾｰｼﾞ郡の代表番号を指定）
///	@return			MsgBuf	: 取得ﾒｯｾｰｼﾞﾎﾟｲﾝﾀ,取得ﾒｯｾｰｼﾞがなければNULL
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
MsgBuf	*Target_MsgGet_Range( uchar id, t_TARGET_MSGGET_PRM *pReq )
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
			if(( pReq->Command[j] & 0x000f) == 0 ){			// 要求コードの下１桁が０（代表）の場合
				if( pReq->Command[j] == (MailCommand&0xff00) ){	// 下2桁をマスクした値が、代表(グループ)一致
					goto Target_MsgGet_10;						// 見つけたﾒｰﾙをｷｭｰから抜く
				}
			}else{											// 要求コードが枝番の場合
				if( pReq->Command[j] == MailCommand ){		// 枝番まで一致しているかを判定
					goto Target_MsgGet_10;					// 見つけたﾒｰﾙをｷｭｰから抜く
				}
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

//[]----------------------------------------------------------------------[]
///	@brief			切替情報のバックアップ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		dat		: 切替情報ポインタ
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_sw_update_bk( uchar *dat )
{
	memcpy(chg_info.sw_status, dat, 3);
}

//[]----------------------------------------------------------------------[]
///	@brief			切替情報の取得（停復電及び結果情報リトライ時に取得）
//[]----------------------------------------------------------------------[]
///	@param[in]		dat		: 切替情報ポインタ
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_sw_update_get( uchar *dat )
{
	memcpy(dat, chg_info.sw_status, 3);
}

//[]----------------------------------------------------------------------[]
///	@brief			ダウンロード情報のバックアップ
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_chg_info_bkup( void )
{
	memcpy(&bk_chg_info, &chg_info, sizeof( t_prog_chg_info ));
}

//[]----------------------------------------------------------------------[]
///	@brief			ダウンロード情報のリストア
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_chg_info_restore( void )
{
	memcpy(&chg_info, &bk_chg_info, sizeof( t_prog_chg_info ));	// ダウンロード情報のリストア
	memset(&bk_chg_info, 0, sizeof( t_prog_chg_info ));		// リストア成功時にはバックアップエリアを消去
}

//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータアップロード種別設定
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:画面種別 0：ショートカット 1：マニュアル操作
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Param_Upload_type_set( uchar type )
{
	chg_info.param_up = type;
}

//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータアップロード種別取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:画面種別 0：マニュアル操作 1：ショートカット
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	Param_Upload_type_get( void )
{
	return	(chg_info.param_up);
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード実行状況更新
//[]----------------------------------------------------------------------[]
///	@param[in]		sts		:ダウンロード実行状況
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2011-09-26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	remotedl_exec_info_set( uchar exec )
{
	chg_info.exec_info = exec;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔ダウンロード実行状況参照
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:ダウンロード実行状況
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2011-09-26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
uchar	remotedl_exec_info_get( void )
{
	return	(chg_info.exec_info);
}

//[]----------------------------------------------------------------------[]
///	@brief			設定アップロード開始判定
//[]----------------------------------------------------------------------[]
///	@param[in]		*time	: 設定アップロード開始時刻
///	@param[in]		status	: 更新するステータス
///	@return			ret		: 0=開始失敗, 1=開始成功
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar	up_wait_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;

// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
//	if (!remote_date_chk(time)) {							// アップロード時刻が不正の場合チェックしない
	if (!remote_date_chk_mnt(time)) {
		// 接続リトライタイマー待ちの状態で分歩進イベントにより処理されないようにガードする
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
		return ret;
	}

	GetDate = Nrm_YMDHM( time );							// アップロード時刻ノーマライズ
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// 現在時刻ノーマライズ

	if( GetDate <= NowDate ){	 							// 一致または予定時刻を経過している
		do {
			// メンテナンス中か？
// GM760201(S) LCDに設定アップロード中は、センターアップロードしない
//			if (OPECTL.Mnt_mod != 0) {
			if ((OPECTL.Mnt_mod != 0) || (OPECTL.lcd_prm_update != 0)){
// GM760201(E) LCDに設定アップロード中は、センターアップロードしない
				rmon_regist(RMON_PRM_UP_START_MNT_NG);
				if (remotedl_dl_start_retry_check() < 0) {
// GG120600(S) // Phase9 種別毎をチェック
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
					rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_UL);
					remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
				}
				break;
			}
			// 開始時刻が一定時間経過していないか？
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
//				rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
// GG120600(S) // Phase9 (一定時間経過)を追加
				rmon_regist(RMON_PRM_UP_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (一定時間経過)を追加
				remotedl_complete_request(REQ_KIND_PARAM_UL);
				remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
				break;
			}

			remotedl_status_set( status );					// FTP通信状態を更新
// GG120600(S) // Phase9 リトライの種別を分ける
//			remotedl_start_retry_clear();					// 開始判定リトライ回数クリア
			remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける

			rmon_regist(RMON_PRM_UP_START_OK);

			// アップロード開始
			queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
			ret = 1;
		} while (0);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			リセット開始判定
//[]----------------------------------------------------------------------[]
///	@param[in]		*time	: リセット開始時刻
///	@param[in]		status	: 更新するステータス
///	@return			ret		: 0=開始失敗, 1=開始成功
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar	reset_start_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;
	ulong	code;

	GetDate = Nrm_YMDHM( time );							// リセット時刻ノーマライズ
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// 現在の時刻ノーマライズ
	if( GetDate <= NowDate ){	 							// 一致または予定時刻を経過している
		do {
			// メンテナンス中か待機か休業以外の場合
			if (!(OPECTL.Mnt_mod == 0 &&
				(OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100))) {
				code = RMON_RESET_START_STATUS_NG;
				break;
			}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			// Edy自動センター通信中、自動T合計実施中
//			if ((auto_cnt_prn == 2) ||
//				(auto_syu_prn == 1) || (auto_syu_prn == 2)) {
			// 自動T合計実施中
			if ((auto_syu_prn == 1) || (auto_syu_prn == 2)) {
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				code = RMON_RESET_START_COMM_NG;
				break;
			}
			// フラップ動作中
			if (flap_move_chk()) {
				code = RMON_RESET_START_FLAP_NG;
				break;
			}
			// ドアが開いているか、ソレノイドロックが解除状態
			if (DOOR_OPEN_OR_SOLENOID_UNROCK()) {
				code = RMON_RESET_START_DOOR_OPEN_NG;
				break;
			}
			// 開始時刻が一定時間経過していないか？
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
// GG120600(S) // Phase9 種別毎をチェック
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
//				rmon_regist(RMON_RESET_START_RETRY_OVER);
// GG120600(S) // Phase9 (一定時間経過)を追加
				rmon_regist(RMON_RESET_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (一定時間経過)を追加
				remotedl_complete_request(REQ_KIND_RESET);
				remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
				return 0;
			}

			remotedl_status_set( status );					// FTP通信状態を更新
// GG120600(S) // Phase9 リトライの種別を分ける
//			remotedl_start_retry_clear();					// 開始判定リトライ回数クリア
			remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける

			rmon_regist(RMON_RESET_START_OK);

			// リセット開始
			System_reset();
			ret = 1;
		} while (0);

		// エラーの監視データを登録
		rmon_regist(code);

		// リセット開始NG
		if (remotedl_reset_start_retry_check() < 0) {
// GG120600(S) // Phase9 種別毎をチェック
//			remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 種別毎をチェック
// GG120600(S) // Phase9 リトライの種別を分ける
//			remotedl_start_retry_clear();
			remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
			rmon_regist(RMON_RESET_START_RETRY_OVER);
			remotedl_complete_request(REQ_KIND_RESET);
			remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			名前作成（IP+機種+機械№)
//[]----------------------------------------------------------------------[]
///	@param[in]		status	:ステータス情報（更新したいステータスを指定）
///	@return			ret		: 0：チェックNG 1：チェックOK（FTP通信開始）
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static void MakeRemoteFileNameForIP_MODEL_NUM(ushort updown,char *remote)
{
// GG120600(S) // Phase9 サーバフォルダ構成変更対応
//	if( updown == FILENAME_PARAM_UP ){
//		if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN以外
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/UPLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/UPLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_CEN][65],
//				CPrmSS[S_CEN][66],
//				NTNET_MODEL_CODE,						// 機種
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
//				);
//		}else{
//			// LANの場合（暫定・・・192.168等ではぶつかる可能性あり）
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/UPLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/UPLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_MDL][2],
//				CPrmSS[S_MDL][3],
//				NTNET_MODEL_CODE,						// 機種
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
//				);
//		}
//	}else if(updown == FILENAME_PARAM_DOWN){
//		if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN以外
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/DOWNLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/DOWNLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_CEN][65],
//				CPrmSS[S_CEN][66],
//				NTNET_MODEL_CODE,						// 機種
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
//				);
//		}else{
//			// LANの場合（暫定・・・192.168等ではぶつかる可能性あり）
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/DOWNLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/DOWNLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_MDL][2],
//				CPrmSS[S_MDL][3],
//				NTNET_MODEL_CODE,						// 機種
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
//				);
//		}
//	}else if(updown == FILENAME_TEST){
//// MH810100(S)
////		sprintf((char *)remote, "/TEST/FT4000/%06ld%06ld%03d%02ld",
//		sprintf((char *)remote, "/TEST/GT4100/%06ld%06ld%03d%02ld",
//// MH810100(E)
//			CPrmSS[S_CEN][65],
//			CPrmSS[S_CEN][66],
//			NTNET_MODEL_CODE,						// 機種
//			prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
//			);
//	}

	char	pathname[256];
	uchar	ipaddr_tmp[4];
	int 	i;

	memset(pathname,0,sizeof(pathname));
	if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN以外
		sprintf((char *)pathname, "%06ld%06ld%03d%02ld",
				CPrmSS[S_CEN][65],
				CPrmSS[S_CEN][66],
				NTNET_MODEL_CODE,						// 機種
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
				);
	}else{
		// FTP用サーバーフォルダIPがあるかどうかチェック
		for (i = 0; i < 4; i++) {
			ipaddr_tmp[i] = (uchar)prm_get( COM_PRM,S_RDL,7 + i,3,1 );
		}
		// なかったらLANを使用する
		if (*((ulong *)ipaddr_tmp) == 0) {
			sprintf((char *)pathname, "%06ld%06ld%03d%02ld",
				CPrmSS[S_MDL][2],
				CPrmSS[S_MDL][3],
				NTNET_MODEL_CODE,						// 機種
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
				);
		}else{
			sprintf((char *)pathname, "%03d%03d%03d%03d%03d%02ld",
				ipaddr_tmp[0],ipaddr_tmp[1],ipaddr_tmp[2],ipaddr_tmp[3],
				NTNET_MODEL_CODE,						// 機種
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// 機械№
				);
		}

	}
	if( updown == FILENAME_PARAM_UP ){
		sprintf((char *)remote, "/PARAM/%03d/UPLOAD/%s",NTNET_MODEL_CODE,pathname);
	}else if(updown == FILENAME_PARAM_DOWN){
		sprintf((char *)remote, "/PARAM/%03d/DOWNLOAD/%s",NTNET_MODEL_CODE,pathname);
	}else if(updown == FILENAME_TEST){
		sprintf((char *)remote, "/TEST/%03d/%s",NTNET_MODEL_CODE,pathname);
	}

// GG120600(E) // Phase9 サーバフォルダ構成変更対応
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP接続テストファイル内容作成
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pBuff	: ファイルバッファ
///	@param[in]		nSize	: ファイルバッファサイズ
///	@return			ret		: ファイルサイズ
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort AppServ_MakeRemoteDl_TestConnect(char* pBuff,int nSize)
{
	ushort nFileSize;
	if( nSize < TEST_CONNECT_FILE_SIZE){
		nFileSize = nSize;
	}else{
		nFileSize = TEST_CONNECT_FILE_SIZE;
	}
	memset( pBuff, 0x31,nFileSize);
	
	return nFileSize;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTPサーバアドレス取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTPサーバアドレス(数値)
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ulong remotedl_ftp_ipaddr_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_DIF_DL:
		type = PARAM_DL_DIF_TIME;
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].ftpaddr.uladdr;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTPポート番号取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTPポート番号
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort remotedl_ftp_port_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_DIF_DL:
		type = PARAM_DL_DIF_TIME;
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].ftpport;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTPユーザ取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*user	: FTPユーザ文字列
///	@param[in]		size	: FTPユーザ文字列サイズ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_user_get(char *user, int size)
{
	uchar type;

	if (user) {
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
			type = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			type = PARAM_DL_TIME;
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		case CTRL_PARAM_DIF_DL:
			type = PARAM_DL_DIF_TIME;
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		case CTRL_PARAM_UPLOAD:
			type = PARAM_UP_TIME;
			break;
		case CTRL_CONNECT_CHK:
			type = TEST_TIME;
		break;
		default:
			user[0] = '\0';
			return;
		}
		strncpy(user, (char *)chg_info.dl_info[type].ftpuser, size);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTPパスワード取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*passwd	: FTPパスワード文字列
///	@param[in]		size	: FTPパスワード文字列サイズ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_passwd_get(char *passwd, int size)
{
	uchar type;

	if (passwd) {
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
			type = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			type = PARAM_DL_TIME;
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		case CTRL_PARAM_DIF_DL:
			type = PARAM_DL_DIF_TIME;
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		case CTRL_PARAM_UPLOAD:
			type = PARAM_UP_TIME;
			break;
		case CTRL_CONNECT_CHK:
			type = TEST_TIME;
		break;
		default:
			passwd[0] = '\0';
			return;
		}
		strncpy(passwd, (char *)chg_info.dl_info[type].ftppasswd, size);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			DL・UL開始判定リトライ回数チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=リトライ中, -1=リトライオーバー
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_dl_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// ダウンロード・アップロード開始判定リトライ回数
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 3, 3, 1);
// GG120600(S) // Phase9 リトライの種別を分ける
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 リトライの種別を分ける
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			更新開始判定リトライ回数チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=リトライ中, -1=リトライオーバー
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_sw_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// プログラム切換開始判定リトライ回数
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 4, 3, 1);
// GG120600(S) // Phase9 リトライの種別を分ける
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 リトライの種別を分ける
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			リセット開始判定リトライ回数チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=リトライ中, -1=リトライオーバー
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_reset_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// リセット開始判定リトライ回数
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 6, 3, 1);
// GG120600(S) // Phase9 リトライの種別を分ける
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 リトライの種別を分ける
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			開始判定リトライ回数クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 リトライの種別を分ける
//void remotedl_start_retry_clear(void)
void remotedl_start_retry_clear(uchar dl_kind)
// GG120600(E) // Phase9 リトライの種別を分ける
{
// GG120600(S) // Phase9 リトライの種別を分ける
//	chg_info.retry_dl_cnt = 0;
	if (dl_kind < TIME_INFO_MAX) {
		chg_info.retry_dl_cnt[dl_kind] = 0;
	}
// GG120600(E) // Phase9 リトライの種別を分ける
}

//[]----------------------------------------------------------------------[]
///	@brief			フェールセーフタイマクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_failsafe_timer_clear(void)
{
	chg_info.failsafe_timer = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			フェールセーフタイマチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=タイマ動作中, -1=タイマ満了
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_failsafe_timer_check(void)
{
	int ret = 0;
	ushort failsafe;

	failsafe = (ushort)prm_get(COM_PRM, S_RDL, 2, 3, 1);
	if (!failsafe) {
		failsafe = 30;
	}
	if (++chg_info.failsafe_timer > failsafe) {
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス処理状態取得(要求応答の値に変換)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 遠隔メンテナンス処理状態
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_convert_dl_status(void)
{
	int ret = REQ_NONE;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:					// *** プログラムダウンロード ***
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_DL_REQ_RCV:
				ret = REQ_PROG_DL_WAIT;
				break;
			case R_DL_START:
			case R_DL_EXEC:
				ret = REQ_PROG_DL_EXEC;
				break;
			case R_SW_WAIT:
				ret = REQ_PROG_SW_WAIT;
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_PARAM_DL:					// *** 設定ダウンロード ***
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_DIF_DL:
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_DL_REQ_RCV:
				ret = REQ_PARAM_DL_WAIT;
				break;
			case R_DL_START:
			case R_DL_EXEC:
				ret = REQ_PARAM_DL_EXEC;
				break;
			case R_SW_WAIT:
				ret = REQ_PARAM_SW_WAIT;
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_RESET:					// *** リセット ***
		{
			ret = REQ_RESET_WAIT;
		}
		break;
	case CTRL_PARAM_UPLOAD:				// **** 設定アップロード ***
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_UP_WAIT:
				ret = REQ_PARAM_UL_WAIT;	// 設定アップロード待ち
				break;
			case R_UP_START:
			case R_DL_EXEC:
				ret = REQ_PARAM_UL_EXEC;	// 設定アップロード中
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_CONNECT_CHK:				// *** 接続テスト ***
		{
			ret = REQ_TEST_NOW;			// 接続テスト中
		}
		break;
	case CTRL_PROG_SW:					// *** プログラム切換 ***
		{
			ret = REQ_PROG_SW_WAIT;		// プログラム更新開始待ち
		}
		break;
	case CTRL_NONE:
	default:
		ret = REQ_RCV_NONE;
		break;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス要求受付
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: 要求種別
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_accept_request(int request)
{
	chg_info.req_accept |= request;

// GG120600(S) // Phase9 設定変更通知対応
	// 次に実行される要求をセット
	remotedl_arrange_next_request();
// GG120600(E) // Phase9 設定変更通知対応
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス要求クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: 要求種別
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_clear_request(int request)
{
	chg_info.req_accept &= ~request;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス要求受付チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: 要求種別
///	@return			ret		: FALSE=受付あり, TRUE=受付あり
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_is_accept(int request)
{
	uchar ret = FALSE;

	if ((chg_info.req_accept & request) == request) {
		ret = TRUE;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス要求完了
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: 要求種別
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_complete_request(int request)
{
	remotedl_connect_type_set(CTRL_NONE);
	remotedl_clear_request(request);
// GG120600(S) // Phase9 リトライの種別を分ける
//	remotedl_start_retry_clear();
//	retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 リトライの種別を分ける
	remotedl_monitor_info_set(0);

	// バージョンアップ要求
	if (request & REQ_KIND_VER_UP) {
// GG120600(S) // Phase9 結果を区別する
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PROG] = EXCLUDED;
		chg_info.result[RES_SW_PROG] = EXCLUDED;
// GG120600(E) // Phase9 結果を区別する
		remotedl_recv_info_clear(PROG_DL_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		retry_info_clr(RETRY_KIND_CONNECT_PRG);
		retry_info_clr(RETRY_KIND_DL_PRG);
		remotedl_start_retry_clear(PROG_DL_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
		chg_info.update = 0;
	}

	// 設定変更要求
	if (request & REQ_KIND_PARAM_CHG) {
// GG120600(S) // Phase9 結果を区別する
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PARAM] = EXCLUDED;
		chg_info.result[RES_SW_PARAM] = EXCLUDED;
// GG120600(E) // Phase9 結果を区別する
		remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
		retry_info_clr(RETRY_KIND_DL_PARAM);
		remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
		chg_info.update = 0;
	}

// MH810100(S)
	if (request & REQ_KIND_PARAM_DIF_CHG) {
// GG120600(S) // Phase9 結果を区別する
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PARAM_DIF] = EXCLUDED;
		chg_info.result[RES_SW_PARAM_DIF] = EXCLUDED;
// GG120600(E) // Phase9 結果を区別する
		remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
		retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
		remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
		chg_info.update = 0;
	}
// MH810100(E)

	// 設定要求
	if (request & REQ_KIND_PARAM_UL) {
		chg_info.result[RES_UP] = EXCLUDED;
		remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
		retry_info_clr(RETRY_KIND_UL);
		remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
	}

	// リセット要求
	if (request & REQ_KIND_RESET) {
		remotedl_recv_info_clear(RESET_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
	}

	// プログラム切換要求
	if (request & REQ_KIND_PROG_ONLY_CHG) {
// GG120600(S) // Phase9 結果を区別する
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_SW_PROG] = EXCLUDED;
// GG120600(E) // Phase9 結果を区別する
		remotedl_recv_info_clear(PROG_ONLY_TIME);
// GG120600(S) // Phase9 リトライの種別を分ける
		remotedl_start_retry_clear(PROG_ONLY_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
		chg_info.update = 0;
	}

	// 接続テスト
	if (request & REQ_KIND_TEST) {
		chg_info.result[RES_COMM] = EXCLUDED;
		remotedl_recv_info_clear(TEST_TIME);
	}
// GG120600(S) // Phase9 設定変更通知対応
	// 次に実行される要求をセット
	remotedl_arrange_next_request();
// GG120600(E) // Phase9 設定変更通知対応
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス要求受信情報クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: 要求種別
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_recv_info_clear(uchar type)
{
	if (type < TIME_MAX) {
		// 対象種別の保持情報をクリア
		memset(&chg_info.dl_info[type], 0, sizeof(t_dl_info));
		switch (type) {
		case PROG_DL_TIME:
			memset(&chg_info.sw_info[SW_PROG], 0, sizeof(t_sw_info));
			chg_info.script[PROGNO_KIND_DL][0] = '\0';
			remotedl_write_info_clear();
			break;
		case PARAM_DL_TIME:
			memset(&chg_info.sw_info[SW_PARAM], 0, sizeof(t_sw_info));
			break;
// MH810100(S) Y.Yamauchi 20191224 車番チケットレス(遠隔ダウンロード)
		case PARAM_DL_DIF_TIME:
// GG120600(S) // Phase9 パラメータ切替を分ける
//			memset(&chg_info.sw_info[SW_PARAM], 0, sizeof(t_sw_info));
			memset(&chg_info.sw_info[SW_PARAM_DIFF], 0, sizeof(t_sw_info));
// GG120600(E) // Phase9 パラメータ切替を分ける
			break;
// MH810100(E) Y.Yamauchi 20191224 車番チケットレス(遠隔ダウンロード)
		case PROG_ONLY_TIME:
			memset(&chg_info.sw_info[SW_PROG_EX], 0, sizeof(t_sw_info));
			chg_info.script[PROGNO_KIND_SW][0] = '\0';
			break;
		default:
			break;
		}
	}
	else {
		// すべてクリア
		memset(chg_info.dl_info, 0, sizeof(t_prog_chg_info)-offsetof(t_prog_chg_info, dl_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			命令番号取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 命令番号
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
u_inst_no* remotedl_instNo_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 差分ダウンロード時に命令番号が空になる
//		type = PARAM_DL_TIME;
		type = PARAM_DL_DIF_TIME;
// GG120600(E) // Phase9 差分ダウンロード時に命令番号が空になる
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_RESET:
		type = RESET_TIME;
		break;
	case CTRL_PROG_SW:
		type = PROG_ONLY_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return NULL;
	}
	return &chg_info.dl_info[type].u;
}

// GG120600(S) // Phase9 遠隔監視データ変更
uchar remotedl_instFrom_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
	case CTRL_PARAM_DIF_DL:
// GG129000(S) R.Endo 2023/01/26 車番チケットレス4.1 #6817 差分設定要求に設定した命令要求元が遠隔監視データに設定されない場合がある(机上で検出) [共通改善項目 No 1546]
// 		type = PARAM_DL_TIME;
		type = PARAM_DL_DIF_TIME;
// GG129000(E) R.Endo 2023/01/26 車番チケットレス4.1 #6817 差分設定要求に設定した命令要求元が遠隔監視データに設定されない場合がある(机上で検出) [共通改善項目 No 1546]
		break;
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_RESET:
		type = RESET_TIME;
		break;
	case CTRL_PROG_SW:
		type = PROG_ONLY_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].From;
}
// GG120600(E) // Phase9 遠隔監視データ変更
// GG120600(S) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
u_inst_no* remotedl_instNo_get_with_Time_Type(uchar timeType)
{
	if( timeType < TIME_MAX){
		return &chg_info.dl_info[timeType].u;
	}
	// NULLで返すのも微妙なため、TESTの内容を返却する
	return &chg_info.dl_info[TEST_TIME].u;
}

uchar remotedl_instFrom_get_with_Time_Type(uchar timeType)
{
	if( timeType < TIME_MAX){
		return chg_info.dl_info[timeType].From;
	}
	return 0;
}
// GG120600(E) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)

//[]----------------------------------------------------------------------[]
///	@brief			遠隔監視データ登録
//[]----------------------------------------------------------------------[]
///	@param[in]		Code	: コード
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void rmon_regist(ulong Code)
{
	u_inst_no *pInstNo;
// GG120600(S) // Phase9 リトライの種別を分ける
	uchar connect_type;
// GG120600(E) // Phase9 リトライの種別を分ける

	if (Code == 0) {
		return;
	}
// GG120600(S) // Phase9 リトライの種別を分ける
//	// リトライ中は監視データを登録しない
//	if (now_retry_active_chk(RETRY_KIND_MAX) || chg_info.retry_dl_cnt) {
//		return;
//	}
	connect_type = remotedl_connect_type_get();
	// CTRL_CONNECT_CHKは、retry_dl_cntがないので除外
	if(connect_type != CTRL_CONNECT_CHK){
		if(chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(connect_type)]){
			return;
		}
	}
	switch (connect_type) {
	case CTRL_PROG_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) || now_retry_active_chk(RETRY_KIND_DL_PRG)){
			return;
		}
		break;
	case CTRL_PARAM_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) || now_retry_active_chk(RETRY_KIND_DL_PARAM)){
			return;
		}
		break;
	case CTRL_PARAM_DIF_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) || now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF)){
			return;
		}
		break;
	case CTRL_PARAM_UPLOAD:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) || now_retry_active_chk(RETRY_KIND_UL)){
			return;
		}
		break;
	case CTRL_RESET:
	case CTRL_PROG_SW:
	case CTRL_CONNECT_CHK:
	default:
		break;
	}
// GG120600(E) // Phase9 リトライの種別を分ける
	if ((pInstNo = remotedl_instNo_get()) != NULL) {
// GG120600(S) // Phase9 遠隔監視データ変更
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, remotedl_monitor_info_get(), pInstNo, NULL, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, remotedl_monitor_info_get(), pInstNo, NULL, NULL, NULL,remotedl_instFrom_get(),0);
// GG120600(E) // Phase9 遠隔監視データ変更
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔監視データ登録(命令番号あり)
//[]----------------------------------------------------------------------[]
///	@param[in]		Code		: コード
///	@param[in]		*pInstNo	: 命令番号
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 遠隔監視データ変更
//void rmon_regist_ex(ulong Code, ulong *pInstNo)
void rmon_regist_ex(ulong Code, ulong *pInstNo,uchar from)
// GG120600(E) // Phase9 遠隔監視データ変更
{
	switch (Code) {
	case RMON_PRG_DL_CHECK_OK:			// [バージョンアップ 予約確認結果]-[受付成功]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PROG_DL_TIME].start_time,
// GG120600(S) // Phase9 遠隔監視データ変更
//				&chg_info.sw_info[SW_PROG].sw_time, &chg_info.script[PROGNO_KIND_DL][0]);
				&chg_info.sw_info[SW_PROG].sw_time, &chg_info.script[PROGNO_KIND_DL][0],from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	case RMON_PRM_DL_CHECK_OK:			// [設定変更(設定ダウンロード) 予約確認結果]-[受付成功]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_DL_TIME].start_time,
// GG120600(S) // Phase9 遠隔監視データ変更
//				&chg_info.sw_info[SW_PARAM].sw_time, NULL);
				&chg_info.sw_info[SW_PARAM].sw_time, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	case RMON_PRM_UP_CHECK_OK:			// [設定転送(設定アップロード) 予約確認結果]-[受付成功]
// GG120600(S) // Phase9 遠隔監視データ変更
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_UP_TIME].start_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_UP_TIME].start_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	case RMON_RESET_CHECK_OK:			// [再起動(リセット) 予約確認結果]-[受付成功]
// GG120600(S) // Phase9 遠隔監視データ変更
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[RESET_TIME].start_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[RESET_TIME].start_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	case RMON_PRG_SW_CHECK_OK:			// [プログラム切換 予約確認結果]-[受付成功]
// GG120600(S) // Phase9 遠隔監視データ変更
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.sw_info[SW_PROG_EX].sw_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.sw_info[SW_PROG_EX].sw_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
// MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
	case RMON_PRM_DIF_DL_CHECK_OK:		// [差分設定変更(設定ダウンロード) 予約確認結果]-[受付成功]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time,
// GG120600(S) // Phase9 遠隔監視データ変更
//				&chg_info.sw_info[SW_PARAM].sw_time, NULL);
				&chg_info.sw_info[SW_PARAM_DIFF].sw_time, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
// MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
	default:
// GG120600(S) // Phase9 遠隔監視データ変更
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, NULL, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, NULL, NULL, NULL,from,0);
// GG120600(E) // Phase9 遠隔監視データ変更
		break;
	}
}

// GG120600(S) // Phase9 設定変更通知対応
// 端末側起点の遠隔監視データ
void rmon_regist_ex_FromTerminal(ulong Code)
{
// GG120600(S) // Phase9 遠隔監視データ変更
//	u_inst_no inst;
//	inst.ullinstNo = (ulonglong)-1;
//	rmon_regist_ex(Code,&inst.ulinstNo[0]);
	u_inst_no inst;
	inst.ulinstNo[0] = CLK_REC.year%100*100 + CLK_REC.mont%100;
	inst.ulinstNo[0] = inst.ulinstNo[0]*10000 + CLK_REC.date%100*100 + CLK_REC.hour%100;
	inst.ulinstNo[1] = CLK_REC.minu%100;
	inst.ulinstNo[1] = inst.ulinstNo[1]*100000 + NTNET_MODEL_CODE*100 + CPrmSS[S_PAY][2]%100;
	inst.ulinstNo[1] = inst.ulinstNo[1]*100 + g_usRmonSeq%100;
	g_usRmonSeq++;
	if(g_usRmonSeq >= 99){
		g_usRmonSeq = 0;
	}

	wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)&inst.ulinstNo[0], NULL, NULL, NULL,1,1);
// GG120600(E) // Phase9 遠隔監視データ変更
}

// GG120600(S) // Phase9 リトライの種別を分ける
void rmon_regist_with_ConectType(ulong Code,uchar connect_type)
{
	u_inst_no *pInstNo;

	if (Code == 0) {
		return;
	}
	if(chg_info.retry_dl_cnt[connect_type]){
		return;
	}
	switch (connect_type) {
	case CTRL_PROG_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) || now_retry_active_chk(RETRY_KIND_DL_PRG)){
			return;
		}
		break;
	case CTRL_PARAM_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) || now_retry_active_chk(RETRY_KIND_DL_PARAM)){
			return;
		}
		break;
	case CTRL_PARAM_DIF_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) || now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF)){
			return;
		}
		break;
	case CTRL_PARAM_UPLOAD:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) || now_retry_active_chk(RETRY_KIND_UL)){
			return;
		}
		break;
	case CTRL_RESET:
	case CTRL_PROG_SW:
	case CTRL_CONNECT_CHK:
	default:
		break;
	}
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
//	if ((pInstNo = &chg_info.dl_info[connect_type].u) != NULL) {
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, chg_info.monitor_info, pInstNo, NULL, NULL, NULL,chg_info.dl_info[connect_type].From,0);
//	}
	if ((pInstNo = &chg_info.dl_info[connect_type_to_dl_time_kind(connect_type)].u) != NULL) {
		wrmonlg(RMON_FUNC_REMOTEDL, Code, chg_info.monitor_info, pInstNo, NULL, NULL, NULL,chg_info.dl_info[connect_type_to_dl_time_kind(connect_type)].From,0);
	}
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正

}
// GG120600(E) // Phase9 リトライの種別を分ける

// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
void remotedl_cancel_setting(void)
{

	int check_request;
	uchar connect_type;
	ulong i;
// GG120600(S) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
	uchar result;
// GG120600(E) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている

	for (i = 0; i < 32; i++) { // 32bit
		check_request = 1 << i;
		if (!remotedl_is_accept(check_request)) {
			continue;
		}

		if ((connect_type = remotedl_request_kind_to_connect_type(check_request)) == CTRL_NONE) {
			// 不正なビットがONになっている
			continue;
		}

		switch (connect_type) {
		case CTRL_PARAM_DL:
// GG120600(S) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
// GG120600(S) // Phase9 リトライの種別を分ける
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
// 			retry_info_clr(RETRY_KIND_DL_PARAM);
// 			remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更)
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// //			rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更)
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			// 受け付けた要求をクリアする
// 			remotedl_complete_request(REQ_KIND_PARAM_CHG);
// 			// 遠隔メンテナンス情報をクリア
// 			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));

			// クリアしてしまうのでここで結果取得（クリアしてから遠隔監視データを呼ばないと、リトライで送信されない・・・・）
			result = remotedl_result_get(RES_DL_PARAM);
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
			retry_info_clr(RETRY_KIND_DL_PARAM);
			remotedl_start_retry_clear(PARAM_DL_TIME);
			if( result == PROG_DL_COMP){
				// DL済みなので更新開始キャンセル
				rmon_regist_with_ConectType(RMON_PRM_SW_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更)
			}else{
				rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更)

			}
			remotedl_recv_info_clear(PARAM_DL_TIME);
			// 受け付けた要求をクリアする
			remotedl_complete_request(REQ_KIND_PARAM_CHG);
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));

// GG120600(E) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
			break;
		case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
// GG120600(S) // Phase9 リトライの種別を分ける
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
// 			retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// 			remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [キャンセル]（端末で設定パラメータ変更
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// //			rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [キャンセル]（端末で設定パラメータ変更
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			// 受け付けた要求をクリアする
// 			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// 			// 遠隔メンテナンス情報をクリア
// 			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));

			// クリアしてしまうのでここで結果取得（クリアしてから遠隔監視データを呼ばないと、リトライで送信されない・・・・）
			result = remotedl_result_get(RES_DL_PARAM_DIF);
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
			retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
			remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
			if( result == PROG_DL_COMP){
				// DL済みなので更新開始キャンセル
				rmon_regist_with_ConectType(RMON_PRM_SW_DIF_START_CANCEL,connect_type);		// [キャンセル]（端末で設定パラメータ変更
			}else{
				rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [キャンセル]（端末で設定パラメータ変更
			}
			remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
			// 受け付けた要求をクリアする
			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
// GG120600(E) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
			break;
		case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
// GG120600(S) // Phase9 リトライの種別を分ける
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
// 			retry_info_clr(RETRY_KIND_UL);
// 			remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			rmon_regist_with_ConectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 リトライの種別を分ける
// GG120600(S) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// //			rmon_regist_with_CooectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更
// GG120600(E) // Phase9 #5069 [キャンセル]（端末で設定パラメータ変更)の命令番号が不正
// 			// 受け付けた要求をクリアする
// 			remotedl_complete_request(REQ_KIND_PARAM_UL);
// 			// 遠隔メンテナンス情報をクリア
// 			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));

			// UPは更新がないのでわけない
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
			retry_info_clr(RETRY_KIND_UL);
			remotedl_start_retry_clear(PARAM_UP_TIME);
			rmon_regist_with_ConectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [キャンセル]（端末で設定パラメータ変更
			remotedl_recv_info_clear(PARAM_UP_TIME);
			// 受け付けた要求をクリアする
			remotedl_complete_request(REQ_KIND_PARAM_UL);
			// 遠隔メンテナンス情報をクリア
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));

// GG120600(E) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
			break;
		}
	}
	return;

}
// GG120600(E) // Phase9 設定変更通知対応


//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス時刻情報取得
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: 要求種別
///	@return			ret		: 時刻情報ポインタ
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
t_remote_time_info* remotedl_time_info_get(uchar type)
{
	t_remote_time_info *pTime = NULL;

	if (type < TIME_INFO_MAX) {
		pTime = &remote_dl_info.time_info[type];
	}

	return pTime;
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス時刻情報クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 要求種別
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//void remotedl_time_info_clear(uchar type)
void remotedl_time_info_clear(uchar type,u_inst_no *pInstNo,uchar from)
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる
{
	uchar ReqKind;

	switch (type) {
	case PROG_DL_TIME:
		ReqKind = VER_UP_REQ;
		break;
	case PARAM_DL_TIME:
		ReqKind = PARAM_CHG_REQ;
		break;
	case PARAM_UP_TIME:
		ReqKind = PARAM_UPLOAD_REQ;
		break;
	case RESET_TIME:
		ReqKind = RESET_REQ;
		break;
	case PROG_ONLY_TIME:
		ReqKind = PROG_ONLY_CHG_REQ;
		break;
// MH810100(S) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)		
	case PARAM_DL_DIF_TIME:
		ReqKind = PARAM_DIFF_CHG_REQ;
		break;
// MH810100(E) Y.Yamauchi 2019/12/23 車番チケットレス(遠隔ダウンロード)		
	default:
		return;
	}

	// 要求取消
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//// GG120600(S) // Phase9 遠隔監視データ変更
////	remotedl_proc_cancel(ReqKind, &chg_info.dl_info[type].u.ulinstNo[0], TRUE);
//	remotedl_proc_cancel(ReqKind, &chg_info.dl_info[type].u.ulinstNo[0], TRUE,chg_info.dl_info[type].From);
//// GG120600(E) // Phase9 遠隔監視データ変更
	remotedl_proc_cancel(ReqKind, &pInstNo->ulinstNo[0], TRUE,from);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる
}

// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
////[]----------------------------------------------------------------------[]
/////	@brief			遠隔メンテナンス時刻情報全クリア
////[]----------------------------------------------------------------------[]
/////	@param[in]		void
/////	@return			void
/////	@attention		None
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2015-04-08<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
//void remotedl_time_info_clear_all(void)
//{
//	uchar type;
//
//	for (type = PROG_DL_TIME; type < TIME_MAX; type++) {
//		remotedl_time_info_clear(type);
//	}
//}
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス時刻情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 時刻種別
///	@param[in]		type	: 要求種別
///	@param[in]		*pTime	: 時刻情報ポインタ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_time_set(uchar kind, uchar type, date_time_rec *pTime)
{
	switch (kind) {
	case INFO_KIND_START:
		if (type < TIME_INFO_MAX) {
			// 開始時刻をセット
			memcpy(&remote_dl_info.time_info[type].start_time, pTime, sizeof(*pTime));
			// 処理ステータスをクリア
			remote_dl_info.time_info[type].status[INFO_KIND_START] = EXEC_STS_NONE;
		}
		break;
	case INFO_KIND_SW:
		if (type < TIME_INFO_MAX) {
			// 更新時刻をセット
			memcpy(&remote_dl_info.time_info[type].sw_time, pTime, sizeof(*pTime));
			// 処理ステータスをクリア
			remote_dl_info.time_info[type].status[INFO_KIND_SW] = EXEC_STS_NONE;
		}
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			遠隔メンテナンス処理ステータス設定
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 時刻種別
///	@param[in]		type	: 要求種別
///	@param[in]		status	: 処理ステータス
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_comp_set(uchar kind, uchar type, uchar status)
{
	if ((kind < INFO_KIND_MAX) && (type < TIME_INFO_MAX)) {
		// ステータスをセット
		remote_dl_info.time_info[type].status[kind] = status;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		*pFtpInfo	: FTP情報ポインタ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_info_set(t_FtpInfo *pFtpInfo)
{
	memcpy(&g_bk_FtpInfo, pFtpInfo, sizeof(*pFtpInfo));
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP情報取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTP情報ポインタ
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
t_FtpInfo* remotedl_ftp_info_get(void)
{
	return &g_bk_FtpInfo;
}

//[]----------------------------------------------------------------------[]
///	@brief			未送信データ件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 未送信データ件数
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ulong remotedl_unsent_data_count_get(void)
{
	int i,j;
	ulong data_count = 0;
	ulong *pBufCnt;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	NTBUF_GetBufCount(&g_bufcnt);
	NTBUF_GetBufCount(&g_bufcnt, TRUE);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	pBufCnt = (ulong *)&g_bufcnt;

	for (i = 0; i <= 1; i++) {
		for (j = 6; j > 0; j--) {
			// 送信マスク設定チェック
			if (prm_get(COM_PRM, S_NTN, 61+i, 1, j) == 0) {
				// 既存形式の場合
				if (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
					// 釣銭管理集計データはカウントしない
					if ((i == 1) && (j == 2)) {
						continue;
					}
				}
				// 集計データの場合、サーバタイプ時のデータ送信パラメータをチェック
				if ((i == 0) && (j == 4) && (0 == prm_get(COM_PRM, S_NTN, 26, 1, 3))) {
					continue;
				}
				data_count += *(pBufCnt + st_offset[i][j-1]);
			}
		}
	}
	// 現状、遠隔監視データの送信マスク設定がないのでセンター形式であればカウントする
	if (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
		data_count += g_bufcnt.sndbuf_rmon;
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
		data_count += g_bufcnt.sndbuf_lpark;// 長期駐車も設定マスクはないのでセンター形式では加算対象
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	}
	return data_count;
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラム部番+バージョン取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pProgNo	: 文字列バッファ
///	@param[in]		size		: 文字列バッファサイズ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_prog_no_get(char *pProgNo, int size)
{
	if (pProgNo && (size >= sizeof(chg_info.script))) {
		strncpy(pProgNo, (char *)chg_info.script, sizeof(chg_info.script));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラム書込み情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		*pInfo	: プログラム書込み情報ポインタ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_set(t_write_info *pInfo)
{
	if (pInfo) {
		memcpy(&chg_info.write_info, pInfo, sizeof(chg_info.write_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラム書込み情報取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pInfo	: プログラム書込み情報ポインタ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_get(t_write_info *pInfo)
{
	if (pInfo) {
		memcpy(pInfo, &chg_info.write_info, sizeof(chg_info.write_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			プログラムファイル書込み情報クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_clear(void)
{
	memset(&chg_info.write_info, 0, sizeof(chg_info.write_info));
}

//[]----------------------------------------------------------------------[]
///	@brief			要求受付チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		ReqKind	: 要求種別
///	@return			ret		: 0=受付不可, 1=受付, -1=対象外
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_check_request(uchar ReqKind)
{
	int ret = -1;

	if (0 < ReqKind && ReqKind < REMOTE_REQ_MAX) {
		// 要求チェック
		if (ReqAcceptTbl[ReqKind]) {
			ret = 1;
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			端末監視情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		info	: 端末監視情報
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_monitor_info_set(ushort info)
{
	chg_info.monitor_info = info;
}

//[]----------------------------------------------------------------------[]
///	@brief			端末監視情報取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 端末監視情報
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort remotedl_monitor_info_get(void)
{
	return chg_info.monitor_info;
}

//[]----------------------------------------------------------------------[]
///	@brief			復電フラグセット
//[]----------------------------------------------------------------------[]
///	@param[in]		flg 	: 復電フラグ
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-04-14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_pow_flg_set(BOOL bFlg)
{
	chg_info.pow_flg = bFlg;
}

// MH810103(s) 電子マネー対応 部番チェックを外す
//// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
////[]----------------------------------------------------------------------[]
//// 		部番プレフィックス取得
////[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//static eVER_TYPE GetVersionPrefix( uchar *buf, ushort size )
//{
//	const char pref[VER_TYPE_MAX][5] = { "MH", "GM", "GW", "GG" };
//	eVER_TYPE type = VER_TYPE_MAX;
//	ushort i;
//
//	if( buf == NULL || size < 2 ){
//		return type;
//	}
//	for( i=0; i<VER_TYPE_MAX; i++ ){
//		if( strncmp( (char*)buf, pref[i], 2 ) == 0 ){
//			type = (eVER_TYPE)i;
//			break;
//		}
//	}
//	return type;
//}
//// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
// MH810103(e) 電子マネー対応 部番チェックを外す

// GG120600(S) // Phase9 設定変更通知対応
//[]----------------------------------------------------------------------[]
///	@brief			要求種別→処理種別変換
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: 要求種別
///	@return			処理種別
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar remotedl_request_kind_to_connect_type(int request)
{
	uchar connect_type;

	switch (request) {
		case REQ_KIND_VER_UP:
			connect_type = CTRL_PROG_DL;
			break;
		case REQ_KIND_PARAM_CHG:
			connect_type = CTRL_PARAM_DL;
			break;
		case REQ_KIND_PARAM_UL:
			connect_type = CTRL_PARAM_UPLOAD;
			break;
		case REQ_KIND_RESET:
			connect_type = CTRL_RESET;
			break;
		case REQ_KIND_PROG_ONLY_CHG:
			connect_type = CTRL_PROG_SW;
			break;
		case REQ_KIND_FTP_CHG:
			connect_type = CTRL_NONE;
			break;
		case REQ_KIND_TEST:
			connect_type = CTRL_CONNECT_CHK;
			break;
		case REQ_KIND_PARAM_DIF_CHG:
			connect_type = CTRL_PARAM_DIF_DL;
			break;
		case REQ_KIND_PROGNO_CHG:
		default:
			connect_type = CTRL_NONE;
			break;
	}

	return connect_type;
}

//[]----------------------------------------------------------------------[]
///	@brief			次の処理種別を選択
//[]----------------------------------------------------------------------[]
///	@param[in]		遠隔メンテナンス情報
///	@return			最短実行日時
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ulong remotedl_connect_type_set_next(t_prog_chg_info *pg_info)
{
	ulong next_time = 0xFFFFFFFF, check_time;
	int check_request;
	uchar connect_type, next_connect_type = CTRL_NONE;
	uchar status, next_status = R_DL_IDLE;
	ulong i;
	date_time_rec *target_time;
// GG120600(S) // Phase9 結果を区別する
	uchar result;
// GG120600(E) // Phase9 結果を区別する
// GG120600(S) // Phase9 リトライの種別を分ける
	uchar kind_connect;
	uchar kind_updown;
// GG120600(E) // Phase9 リトライの種別を分ける

	if (pg_info->req_accept != 0) {
		for (i = 0; i < 32; i++) { // 32bit
			if (next_connect_type == CTRL_CONNECT_CHK) {
				// FTP接続テストは最優先
				break;
			}

			check_request = 1 << i;
			if (!remotedl_is_accept(check_request)) {
				continue;
			}

			if ((connect_type = remotedl_request_kind_to_connect_type(check_request)) == CTRL_NONE) {
				// 不正なビットがONになっている
				continue;
			}
// GG120600(S) // Phase9 結果を区別する
			switch (connect_type) {
				case CTRL_PROG_DL:
					result = pg_info->result[RES_DL_PROG];
					kind_connect = RETRY_KIND_CONNECT_PRG;
					kind_updown = RETRY_KIND_DL_PRG;
					break;
				case CTRL_PARAM_DL:
					result = pg_info->result[RES_DL_PARAM];
					kind_connect = RETRY_KIND_CONNECT_PARAM_DL;
					kind_updown = RETRY_KIND_DL_PARAM;
					break;
				case CTRL_PARAM_DIF_DL:
					result = pg_info->result[RES_DL_PARAM_DIF];
					kind_connect = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
					kind_updown = RETRY_KIND_DL_PARAM_DIFF;
					break;
				case CTRL_PARAM_UPLOAD:
					kind_connect = RETRY_KIND_CONNECT_PARAM_UP;
					kind_updown = RETRY_KIND_UL;
					break;
			}
// GG120600(E) // Phase9 結果を区別する

			switch (connect_type) {
				case CTRL_PROG_DL:
				case CTRL_PARAM_DL:
				case CTRL_PARAM_DIF_DL:
					status = R_DL_REQ_RCV;
// GG120600(S) // Phase9 結果を区別する
//					if (pg_info->result[RES_DL] != EXCLUDED) {
					if (result == PROG_DL_COMP) {
// GG120600(E) // Phase9 結果を区別する
//						status = R_SW_WAIT;
						switch (connect_type) {
							case CTRL_PROG_DL:
								if (pg_info->sw_info[SW_PROG].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PROG].sw_time;
								}
								else {
									// 端末との転送中 or 端末での更新中はステータスを更新しない
									continue;
								}
								break;
// GG120600(S) // Phase9 パラメータ切替を分ける
//							case CTRL_PARAM_DL:
//							case CTRL_PARAM_DIF_DL:
//								if (pg_info->sw_info[SW_PARAM].exec != 0) {
//									status = R_SW_WAIT;
//									target_time = &pg_info->sw_info[SW_PARAM].sw_time;
//								}
//								else {
//									// 端末への転送中 or 端末での更新中はステータスを更新しない
//									continue;
//								}
//								break;
							case CTRL_PARAM_DL:
								if (pg_info->sw_info[SW_PARAM].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PARAM].sw_time;
								}
								else {
									// 端末への転送中 or 端末での更新中はステータスを更新しない
									continue;
								}
								break;
							case CTRL_PARAM_DIF_DL:
								if (pg_info->sw_info[SW_PARAM_DIFF].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PARAM_DIFF].sw_time;
								}
								else {
									// 端末への転送中 or 端末での更新中はステータスを更新しない
									continue;
								}
								break;
// GG120600(E) // Phase9 パラメータ切替を分ける
						}
						check_time = Nrm_YMDHM(target_time);
					}
// GG120600(S) // Phase9 結果を区別する
//					else if (now_retry_active_chk( RETRY_KIND_DL)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_DL].time);
					else if (now_retry_active_chk( kind_updown)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_updown].time);
// GG120600(E) // Phase9 結果を区別する
						status = R_DL_IDLE;
					}
// GG120600(S) // Phase9 結果を区別する
//					else if (now_retry_active_chk( RETRY_KIND_CONNECT)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_CONNECT].time);
					else if (now_retry_active_chk( kind_connect)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_connect].time);
// GG120600(E) // Phase9 結果を区別する
						status = R_DL_IDLE;
					}
					else {
						switch (connect_type) {
							case CTRL_PROG_DL:
								target_time = &pg_info->dl_info[PROG_DL_TIME].start_time;
								break;
							case CTRL_PARAM_DL:
								target_time = &pg_info->dl_info[PARAM_DL_TIME].start_time;
								break;
							case CTRL_PARAM_DIF_DL:
								target_time = &pg_info->dl_info[PARAM_DL_DIF_TIME].start_time;
								break;
						}
						check_time = Nrm_YMDHM(target_time);
					}
					break;
				case CTRL_PARAM_UPLOAD:
					status = R_UP_WAIT;
					if (now_retry_active_chk( RETRY_KIND_UL)) {
						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_UL].time);
						status = R_DL_IDLE;
					}
// GG120600(S) // Phase9 結果を区別する
//					else if (now_retry_active_chk( RETRY_KIND_CONNECT)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_CONNECT].time);
					else if (now_retry_active_chk( kind_connect)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_connect].time);
// GG120600(E) // Phase9 結果を区別する
						status = R_DL_IDLE;
					}
					else {
						check_time = Nrm_YMDHM(&pg_info->dl_info[PARAM_UP_TIME].start_time);
					}
					break;
				case CTRL_RESET:
					status = R_RESET_WAIT;
					check_time = Nrm_YMDHM(&pg_info->dl_info[RESET_TIME].start_time);
					break;
				case CTRL_PROG_SW:
					status = R_SW_WAIT;
					check_time = Nrm_YMDHM(&pg_info->sw_info[SW_PROG_EX].sw_time);
					break;
				case CTRL_CONNECT_CHK:
					status = R_TEST_CONNECT;
					check_time = 0;
					break;
			}

			if (check_time < next_time) {
				next_status = status;
				next_time = check_time;
				next_connect_type = connect_type;
			}
		}
	}

	remotedl_connect_type_set(next_connect_type);
	remotedl_status_set(next_status);
	return next_time;
}
//[]----------------------------------------------------------------------[]
///	@brief			次の処理種別を選択
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void remotedl_arrange_next_request(void)
{
	// ulong next_time = 0xFFFFFFFF, check_time;
	// ulong i;
	// uchar next_status = R_DL_IDLE;

	// for (i = 0; i < sizeof(chg_info) / sizeof(chg_info[0]); i++) {
	// 	check_time = remotedl_connect_type_set_next(&chg_info[i]);
	// 	if (check_time < next_time) {
	// 		next_status = remotedl_status_get();
	// 	}
	// }
	// remotedl_status_set(next_status);
	uchar next_status = R_DL_IDLE;

	remotedl_connect_type_set_next(&chg_info);
	next_status = remotedl_status_get();
	remotedl_status_set(next_status);
}
// GG120600(E) // Phase9 設定変更通知対応
// GG120600(S) // Phase9 CRCチェック
void	remotedl_prog_crc_set( uchar uc1,uchar uc2)
{															
	chg_info.CRC16.crc[0] = uc1;
	chg_info.CRC16.crc[1] = uc2;
}															
ushort	remotedl_prog_crc_get( void )			
{															
	return	(chg_info.CRC16.uscrc);
}															
// GG120600(E) // Phase9 CRCチェック

// GG120600(S) // Phase9 リトライの種別を分ける
//[]----------------------------------------------------------------------[]
///	@brief			処理種別→ダウンロード時間種別変換
//[]----------------------------------------------------------------------[]
///	@param[in]		connect_type	:	処理種別
///	@return			ダウンロード時間種別
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar connect_type_to_dl_time_kind(uchar connect_type)
{
	uchar time_kind = TIME_MAX;

	switch (connect_type) {
		case CTRL_PROG_DL:
			time_kind = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			time_kind = PARAM_DL_TIME;
			break;
		case CTRL_PARAM_UPLOAD:
			time_kind = PARAM_UP_TIME;
			break;
		case CTRL_RESET:
			time_kind = RESET_TIME;
			break;
		case CTRL_PROG_SW:
			time_kind = PROG_ONLY_TIME;
			break;
		case CTRL_CONNECT_CHK:
			time_kind = TEST_TIME;
			break;
		case CTRL_PARAM_DIF_DL:
			time_kind = PARAM_DL_DIF_TIME;
			break;
	}

	return time_kind;
}
// GG120600(E) // Phase9 リトライの種別を分ける

// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

// chg_infoはバックアップへ(remotedl_restoreでバックアップからリストアされる（バックアップは再起動前に実施されている)
// remote_dl_infoは、バックアップでないほうへ（起動処理のremotedl_update_chkでバックアップされる）

void remotedl_chg_info_log0_to1(void)
{
	int i;
	t_prog_chg_info_V0 chg_info_v0;
	t_remote_dl_info_V0 dl_info_v0;
	uchar* base = (uchar*)&chg_info;

	// V0時の位置を算出
	t_prog_chg_info_V0* pbk = (t_prog_chg_info_V0*)(base + sizeof(t_prog_chg_info_V0));
	t_remote_dl_info_V0* pinfo = (t_remote_dl_info_V0*)(base + sizeof(t_prog_chg_info_V0) + sizeof(t_prog_chg_info_V0));
	// 一旦コピー
	memcpy(&chg_info_v0, pbk,sizeof(t_prog_chg_info_V0));
	memcpy(&dl_info_v0, pinfo,sizeof(t_remote_dl_info_V0));
	// 一旦クリア
	memset(&bk_chg_info,0,sizeof(t_prog_chg_info));
	memset(&remote_dl_info,0,sizeof(t_remote_dl_info));
	//////////////////////////////////////
	// chg_infoはバックアップへ
	//////////////////////////////////////
	bk_chg_info.req_accept = chg_info_v0.req_accept;
	bk_chg_info.connect_type = chg_info_v0.connect_type;
	bk_chg_info.status = chg_info_v0.status;
	bk_chg_info.script_type = chg_info_v0.script_type;
	bk_chg_info.update = chg_info_v0.update;
	memcpy(bk_chg_info.result,chg_info_v0.result,sizeof(uchar)*RES_MAX_V0);

	memset(bk_chg_info.retry_dl_cnt,0, sizeof(ushort)*TIME_INFO_MAX);	// // 再起動なので0固定

	bk_chg_info.failsafe_timer = chg_info_v0.failsafe_timer;
	bk_chg_info.monitor_info = chg_info_v0.monitor_info;
	bk_chg_info.param_up = chg_info_v0.param_up;
	bk_chg_info.exec_info = chg_info_v0.exec_info;
	memset(bk_chg_info.retry,0, sizeof(t_retry_info)*RETRY_KIND_MAX_V0);	// // 再起動なので0固定

	// diff分は不要なのでそのまま
	memcpy(bk_chg_info.sw_status,chg_info_v0.sw_status,sizeof(uchar)*(SW_MAX_V0-1));

	bk_chg_info.pow_flg = chg_info_v0.pow_flg;
// -----------------------------------↑ 実行中情報
// -----------------------------------↓ 受信情報
	// 接続テスト前までCOPY
	for(i = 0; i < TIME_MAX_V0 - 1;i++){
		// 要求元を飛ばしてコピーしていく
		memcpy(&bk_chg_info.dl_info[i].u ,&chg_info_v0.dl_info[i].u,sizeof(u_inst_no));
		memcpy(&bk_chg_info.dl_info[i].start_time ,&chg_info_v0.dl_info[i].start_time,sizeof(date_time_rec));
		memcpy(&bk_chg_info.dl_info[i].ftpaddr ,&chg_info_v0.dl_info[i].ftpaddr,sizeof(u_ipaddr));
		bk_chg_info.dl_info[i].ftpport = chg_info_v0.dl_info[i].ftpport;
		memcpy(bk_chg_info.dl_info[i].ftpuser ,chg_info_v0.dl_info[i].ftpuser,sizeof(chg_info_v0.dl_info[i].ftpuser));
		memcpy(bk_chg_info.dl_info[i].ftppasswd ,chg_info_v0.dl_info[i].ftppasswd,sizeof(chg_info_v0.dl_info[i].ftppasswd));
	}
	// 差分前までをCOPY
	memcpy(bk_chg_info.sw_info,chg_info_v0.sw_info,sizeof(t_sw_info)*SW_MAX_V0);

	memcpy(bk_chg_info.script,chg_info_v0.script,sizeof(chg_info_v0.script));
	memcpy(&bk_chg_info.write_info,&chg_info_v0.write_info,sizeof(t_write_info));

	//////////////////////////////////////
	// remote_dl_infoは、バックアップでないほうへ
	//////////////////////////////////////
	memcpy(&remote_dl_info,&dl_info_v0, sizeof(t_remote_time_info)*TIME_INFO_MAX_V0);

}
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
static long remotedl_nrm_time_sub(ulong from, ulong to)
{
	t_NrmYMDHM _from, _to;
	ulong tmp1, tmp2;
	long tmp3, tmp4;
	long diff, min_diff;
	BOOL borrow = FALSE, positive = from <= to;

	_from.ul = from;
	_to.ul = to;

	if (positive) {
		tmp1 = (ulong)_from.us[1];
		tmp2 = (ulong)_to.us[1];
	}
	else {
		tmp1 = (ulong)_to.us[1];
		tmp2 = (ulong)_from.us[1];
	}
	tmp3 = (long)tmp1;
	tmp4 = (long)tmp2;
	if (tmp3 > tmp4) {
		borrow = TRUE;
		tmp4 += 1440;
	}
	min_diff = tmp4 - tmp3;

	if (positive) {
		tmp1 = (ulong)_from.us[0];
		tmp2 = (ulong)_to.us[0];
	}
	else {
		tmp1 = (ulong)_to.us[0];
		tmp2 = (ulong)_from.us[0];
	}
	tmp3 = (long)tmp1;
	tmp4 = (long)tmp2;

	diff = (tmp4 - tmp3) * 1440;
	if (borrow) {
		diff -= 1440;
	}
	diff += min_diff;

	if (!positive) {
		diff *= -1;
	}

	return diff;
}
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(フェーズ9) (#5821:日を跨ぐと遠隔メンテナンス要求が実行されなくなる)
