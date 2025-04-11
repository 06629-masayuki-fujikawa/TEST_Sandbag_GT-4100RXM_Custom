/*[]----------------------------------------------------------------------[]*/
/*| ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽ操作                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"irq1.h"
#include	"flp_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"tbl_rkn.h"
#include	"cnm_def.h"
#include	"mdl_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mif.h"
#include	"sysmnt_def.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include 	"ksg_def.h"
#include	"raudef.h"
#include	"ntnetauto.h"
#include	"oiban.h"
// GG120600(S) // Phase9 設定変更通知対応
#include	"remote_dl.h"
// GG120600(E) // Phase9 設定変更通知対応
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)

/* Function Define */
unsigned short	UsMnt_Clock( void );
unsigned short	UsMnt_Clock2( void );
unsigned short	UsMnt_shtctl( void );
unsigned short	UsMnt_OCsw( void );
unsigned short	UsMnt_mnyctl( void );
unsigned short	UsMnt_mnyctl2( void );
unsigned short	UsMnt_mnychg( void );
unsigned short	UsMnt_invtry( void );
unsigned short	UsMnt_mnybox( void );

unsigned short	UsMnt_Total( short );
unsigned short	UsMnt_Mnctlmenu( void );
unsigned short	UsMnt_ErrAlm( void );
unsigned short	UsMnt_TicketCkk(void);
// MH810100(S) K.Onodera 2019/12/04 車番チケットレス(メンテナンス)
unsigned short	UsMnt_QR_DataCkk(void);
// MH810100(E) K.Onodera 2019/12/04 車番チケットレス(メンテナンス)

static int mnyctl_nuchk( uchar, short, uchar, uchar );
static void mnyctl_dsp( ushort, ushort, ushort, ushort );
static uchar mnyctl_cur( uchar, ushort, ushort );
static void clk_sub( short * );
static void HasseiErrSearch( short );
static void ServiceDsp( void );
static void PassDsp( void );
static void PrepaidDsp( short );
static void KaiDsp( void );
static uchar AmanoCard_Dsp( void );
static int	CheckPrinter(uchar pri_kind);
static int	check_print_result(int no);
static void	syukei_all_dsp(uchar topNo, uchar rev_no, uchar elog );
// MH810105 GG119202(S) T合計連動印字対応
//extern	uchar	SysMnt_Work[];	/* 32KB */
// MH810105 GG119202(E) T合計連動印字対応


/*[]----------------------------------------------------------------------[]*/
/*| 共通パラメータ更新チェック＆サム更新処理                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UserMnt_SysParaUpdateCheck( ushort )                    |*/
/*| PARAMETER    : wkOpelogNo : OpelogNo と同一値                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ユーザーメンテナンス画面終了時点にCallする処理で、操作履歴データ       |*/
/*| （OpelogNo）から共通パラメータ(CPrm[])が変更されているか否かをチェック |*/
/*| し、変更されている場合は共通パラメータのサムを更新する処理。           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar UserMnt_SysParaUpdateCheck( ushort	wkOpelogNo )
{	
	uchar	ret = 0;										// 0:パラメータ更新なし
															// 1:パラメータ更新あり
	int	doSumCheck = 0;

	switch( wkOpelogNo ){
	case	OPLOG_SHOKIMAISUHENKO:							// 釣銭管理－初期枚数変更
	case	OPLOG_TOKUBETUBI:								// 特別日
	case	OPLOG_TOKUBETUKIKAN:							// 特別期間
	case	OPLOG_TOKUBETUHAPPY:							// ハッピーマンデー
	case	OPLOG_TOKUBETUYOBI:								// 特別曜日
	case	OPLOG_SERVICETIME:								// サービスタイム
	case	OPLOG_ROCKTIMER:								// ロック装置閉タイマー
	case	OPLOG_KAKARIINPASS:								// 係員パスワード
	case	OPLOG_BACKLIGHT:								// バックライト点灯
	case	OPLOG_KEYVOLUME:								// ブザー音量調整
	case	OPLOG_KENKIGEN:									// 券期限
	case	OPLOG_EIGYOKAISHI:								// 営業開始時刻
	case	OPLOG_SAIDAIRYOKIN:								// 最大料金
	case	OPLOG_SERVICEYAKUWARI:							// サービス券役割
	case	OPLOG_MISEKUWARI:								// 店役割
	case	OPLOG_SHOHIZEI:									// 消費税の税率
	case	OPLOG_ALYAKUWARI:								// Ａ～Ｌ種役割
	case	OPLOG_TANIJIKANRYO:								// 単位時間料金
	case	OPLOG_TEIKIMUKOYOUBI:							// 定期無効曜日
	case	OPLOG_CONTRAST:									// コントラスト調整
	case	OPLOG_VLSWTIME:									// 音量切替時刻
	case	OPLOG_TOKUBETUWEEKLY:							// 特別曜日

		doSumCheck = -1;
	}

	if( doSumCheck | SetChange ){
			// チェックサムＮＧ
			// 1.共通パラメータのサムを再計算
			DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
			// 2.フラッシュにセーブ
			(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
			// 3.料金計算用設定を更新
			mc10();	//この処理は重いうえ,毎回呼ぶ必要が無いので電源ON時と設定変更時のみCallする事とする。
			// 4.設定更新履歴ログ登録
			SetSetDiff(SETDIFFLOG_SYU_USRMNT);
			f_ParaUpdate.BYTE = 0;							// 復電時にRAM上パラメータデータのSUM更新しない
// GG120600(S) // Phase9 設定変更通知対応
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
		ret = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽﾒｲﾝ処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UserMntMain( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void UserMntMain( void )
{
	unsigned short	usUserEvent;
	char	wk[2];
	unsigned short	parm;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	uchar ucReq = 0;	// 0=OK/1=NG/2=OK(再起動不要)
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//	ushort	mode = 0;
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	uchar	wtype = 0;			// 作業用の一時領域
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[0] );			// [00]	"＜ユーザーメンテナンス＞　　　"

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
// 		usUserEvent = Is_MenuStrMake( 1 );
		if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
			usUserEvent = Is_MenuStrMake(3);
		} else {					// 通常料金計算モード
			usUserEvent = Is_MenuStrMake(1);
		}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		if( OPECTL.Mnt_lev < 2 ){
			// 係員権限では無効とする
			switch( usUserEvent ){
				case MNT_CLOCK:		/* 時計合わせ */
				case MNT_MNCNT:		/* 釣銭管理 */
				case MNT_INVLD:		/* 定期有効／無効(未使用) */
				case MNT_ENTRD:		/* 定期入庫／出庫(未使用) */
				case MNT_SERVS:		/* サービスタイム */
				case MNT_FLTIM:		/* ロック装置開タイマー(未使用) */
				case MNT_SPCAL:		/* 特別日／特別期間 */
				case MNT_PWMOD:		/* 係員パスワード */
				case MNT_BKLIT:		/* バックライト点灯方法 */
				case MNT_TKEXP:		/* 券期限(未使用) */
				case MNT_OPNHR:		/* 営業開始時刻 */
				case MNT_PSTOP:		/* 定期券精算中止データ(未使用) */
				case MNT_PWDKY:		/* 暗証番号強制出庫(未使用) */
				case MNT_VLSW:		/* 音量切替時刻 */
				case MNT_MNYSET:	/* 料金設定 */
					BUZPIPI();
					continue;
			}
		}

		OpelogNo = 0;
		SetChange = 0;
		OpelogNo2 = 0;

		switch( usUserEvent ){
	/* Ｔ集計 */
		case MNT_TTOTL:
			usUserEvent = UsMnt_Total(MNT_TTOTL);
			break;

	/* ロック装置開閉(未使用) */
		case MNT_FLCTL:
			if( Is_CarMenuMake(LOCK_CTRL_MENU) != 0 ){
				usUserEvent = UsMnt_PreAreaSelect(MNT_FLCTL);
			}else{
				BUZPIPI();
				continue;
			}
			break;
	/* 車室情報(未使用) */
		case MNT_FLSTS:
			if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
				if( Ext_Menu_Max > 1){
					usUserEvent = UsMnt_PreAreaSelect(MNT_FLSTS);
				}
				else {
					DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
					usUserEvent = UsMnt_AreaSelect(MNT_FLSTS);
				}
			}else{
				BUZPIPI();
				continue;
			}
			break;

	/* 領収証再発行 */
		case MNT_REPRT:
			usUserEvent = UsMnt_Receipt();
			break;

	/* 受付券再発行(未使用) */
		case MNT_UKERP:
			usUserEvent = UsMnt_ParkingCertificate();
			break;

	/* エラー・アラーム確認 */
		case MNT_ERARM:
			usUserEvent = UsMnt_ErrAlm();		 	// エラー・アラーム確認
			break;

	/* 停留車情報(未使用) */
		case MNT_STAYD:
			usUserEvent = UsMnt_StaySts();
			break;

	/* 履歴情報プリント */
		case MNT_LOGPRN:
			usUserEvent = UsMnt_Logprintmenu();
			break;

	/* 車室故障(未使用) */
		case MNT_CARFAIL:
			if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
				if( Ext_Menu_Max > 1){
					usUserEvent = UsMnt_PreAreaSelect(MNT_CARFAIL);
				}
				else {
					usUserEvent = UsMnt_AreaSelect(MNT_CARFAIL);
				}
			}else{
				BUZPIPI();
				continue;
			}
			break;

	/* 時計合わせ */
		case MNT_CLOCK:
			usUserEvent = UsMnt_Clock();
			break;
	/* 音量切替時刻 */
		case MNT_VLSW:
// MH321800(S) D.Inaba ICクレジット対応
//			usUserEvent = UsMnt_VoiceGuideTime();
			//決済リーダ接続設定あり？
			if( isEC_USE() ){
				usUserEvent = UsMnt_ECVoiceGuide(); 
			}else{
				usUserEvent = UsMnt_VoiceGuideTime();
			}
// MH321800(E) D.Inaba ICクレジット対応

			break;

	/* 駐車台数(未使用) */
		case MNT_FLCNT:
			usUserEvent = UsMnt_ParkCnt();
			break;
	/* 満車コントロール(未使用) */
		case MNT_FLCTRL:
			usUserEvent = UsMnt_FullCtrl();
			break;

	/* 釣銭管理 */
		case MNT_MNCNT:
			usUserEvent = UsMnt_Mnctlmenu();
			break;
	/* ＧＴ集計 */
		case MNT_GTOTL:
			usUserEvent = UsMnt_Total(MNT_GTOTL);
			break;
	/* 営休業切替 */
		case MNT_OPCLS:
			usUserEvent = UsMnt_OCsw();
			break;
	/* シャッター開放(未使用) */
		case MNT_SHTER:
			usUserEvent = UsMnt_shtctl();
			break;
	/* 券データ確認(未使用) */
		case MNT_TKTDT:
			usUserEvent = UsMnt_TicketCkk();
			break;

// MH810100(S) Y.Yamauchi 2019/11/07 車番チケットレス(メンテナンス)
	/* QRデータ確認 */
		case MNT_QRCHECK:
			usUserEvent = UsMnt_QR_DataCkk();
			break;
// MH810100(E) Y.Yamauchi 2019/11/07 車番チケットレス(メンテナンス)

	/* 料金設定 */
		case MNT_MNYSET:
			usUserEvent = UsMnt_Mnysetmenu();
			break;
	/* サービスタイム */
		case MNT_SERVS:
			usUserEvent = UsMnt_SrvTime();
			break;

	/* ロック装置開タイマー(未使用) */
		case MNT_FLTIM:
			if(( parm = (ushort)GetCarInfoParam()) == 0 ){
				BUZPIPI();
				continue;
			}
			if ((parm & 0x01) && ( parm & 0x06 )) {		//駐輪・駐車併設
				usUserEvent = UsMnt_LockTimerEx();
			} else {
				if( parm & 0x01 ){
					wtype = 1;
				}else{
					wtype = 0;
				}
				usUserEvent = UsMnt_LockTimer(wtype);
			}
			break;

	/* 特別日／特別期間 */
		case MNT_SPCAL:
			usUserEvent = UsMnt_SplDay();
			break;
	/* 営業開始時刻 */
		case MNT_OPNHR:
			usUserEvent = UsMnt_BusyTime();
			break;

	/* 券期限(未使用) */
		case MNT_TKEXP:
			usUserEvent = UsMnt_TickValid();
			break;
	/* 定期有効／無効(未使用) */
		case MNT_INVLD:
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//			usUserEvent = UsMnt_PassInv();
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
			break;
	/* 定期入庫／出庫(未使用) */
		case MNT_ENTRD:
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//			usUserEvent = UsMnt_PassEnter();
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
			break;
	/* 定期券チェック(未使用) */
		case MNT_PASCK:
			usUserEvent = UsMnt_PassCheck();
			break;
	/* 定期券精算中止データ(未使用) */
		case MNT_PSTOP:
			usUserEvent = UsMnt_PassStop();
			break;

	/* 係員パスワード */
		case MNT_PWMOD:
			usUserEvent = UsMnt_PassWord();
			break;
	/* 状態確認 */
		case MNT_STSVIEW:
			usUserEvent = UsMnt_StatusView();
			break;
	/* 輝度調整 */
		case MNT_CNTRS:
			usUserEvent = UsColorLCDLumine();
			break;
	/* ブザー音量調整 */
		case MNT_KEYVL:
			usUserEvent = UsMnt_KeyVolume();
			break;

	/* 係員有効データ(未使用) */
		case MNT_ATTENDDATA:
			if(OPECTL.Mnt_lev >= 2 && prm_get(COM_PRM, S_PAY, 21, 1, 3)) {	// 管理者以上かつ磁気リーダあり
				usUserEvent = UsMnt_AttendantValidData();
			}
			else {
				BUZPIPI();
				continue;
			}
			break;
	/* 暗証番号強制出庫(未使用) */
		case MNT_PWDKY:
			usUserEvent = UsMnt_PwdKyo();
			break;

	/* 拡張機能 */
		case MNT_EXTEND:
			if( !Is_ExtendMenuMake() ){				// 拡張機能無効（有効な機能なし）
				BUZPIPI();
				continue;
			}
			usUserEvent = UsMnt_Extendmenu();
			break;
		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return;
		default:
			break;
		}

		if(( OpelogNo )||( OpelogNo2 )||( SetChange )){
			if( OpelogNo ) wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			if( OpelogNo2 ) wopelg( OpelogNo2, 0, 0 );		// 操作履歴登録
			OpelogNo2 = 0;
			if( UserMnt_SysParaUpdateCheck( OpelogNo ) ){
				usUserEvent = parameter_upload_chk();
			}
		}

		if( usUserEvent == MOD_CHG || usUserEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of User Operation
		if( usUserEvent == MOD_CHG ){
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			OPECTL.PasswordLevel = (char)-1;
//			return;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//			mode = op_wait_mnt_close();	// メンテナンス終了処理
//			if( mode == MOD_CHG ){
			{
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
				if( mnt_GetFtpFlag() != FTP_REQ_NONE ){				// FTPフラグを確認
					ucReq = 1;	// 失敗
					if( 0 == lcdbm_setting_upload_FTP() ){			// FTP開始
						// 成功
						switch( mnt_GetFtpFlag() ){
							// 1:FTP要求あり
							case FTP_REQ_NORMAL:
								ucReq = 2;	// OK(再起動不要)
								break;
							// 2:FTP要求あり(電断要)
							case FTP_REQ_WITH_POWEROFF:
								ucReq = 0;	// OK(再起動要)
								break;
							default:
								break;
						}
					}
					lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)ucReq );	// 設定アップロード要求送信
					mnt_SetFtpFlag( FTP_REQ_NONE );								// FTP更新フラグ（更新終了）をセット
				}
// GG124100(S) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
				// 先にLCDへのパラメータアップロードを行ってから遠隔監視データを送信する
				// (LCDへのパラメータアップロード中に遠隔メンテナンス要求が届くのを防止するため)
				if ( mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE ) {
					// parkingWeb接続あり
					if ( _is_ntnet_remote() ) {
						// 端末で設定パラメータ変更
						rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
						// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
						remotedl_cancel_setting();
					}
					mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
				}
// GG124100(E) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//				return;
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
			}
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
			op_wait_mnt_close();	// メンテナンス終了処理
			return;
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
			}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if( usUserEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;							// 初期化状態へ
			OPECTL.init_sts = 0;							// 初期化未完了状態とする
			OPECTL.Pay_mod = 0;								// 通常精算
			OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
			OPECTL.PasswordLevel = (char)-1;
			return;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 時計設定																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_Clock( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*| 			: 2005-07-19 ART:ogura modify													   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

static const unsigned short	CLCK_PNT[5][4] = {
	{ 2, 12 ,1, 2 }, { 2, 18 ,1, 2 }, { 3, 2, 1, 4 }, { 3, 12, 0, 2 }, { 3, 18, 0, 2 } 
};

unsigned short	UsMnt_Clock( void )
{

	ushort			msg = 0;
	short			ndat;
	unsigned short	ret;
	int				ret2;
	short			in_now = -1;
	short			in_d[5];
	short			pos = 0;
	struct	clk_rec	clk_data;
	short			y, m, d;
	ushort			mode = 0;

	TimeAdjustCtrl(1);			// 自動時刻補正停止

	in_d[0] = (short)CLK_REC.hour;
	in_d[1] = (short)CLK_REC.minu;
	in_d[2] = (short)CLK_REC.year;
	in_d[3] = (short) CLK_REC.mont;
	in_d[4] = (short) CLK_REC.date;

	dispclr();

/* Display */
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "＜時計合わせ＞　　　　　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[0] );			// "　　　　　　　　：　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[1] );			// "　　　　　年　　月　　日（　）"
	if( TimeAdjustCtrl(0) == 0 ){	// 同期不可の場合
		Fun_Dsp( FUNMSG[45] );												// "　←　　→　セット       終了 "
	} else {						// 同期可能の場合
		Fun_Dsp( FUNMSG2[58] );												// "　←　　→　セット ｾﾝﾀｰ  終了 "
	}

	clk_sub( in_d );
	opedsp( 2, 12, (unsigned short)CLK_REC.hour, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );

	ret2 = 0;

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );
		if( ret2 ){
			if( msg == TIMEOUT6 ){				// ﾀｲﾑｱｳﾄで抜ける
				ret = MOD_EXT;
				continue;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			if( (msg == KEY_TEN_F5)||(msg == KEY_MODECHG) ){
			if( (msg == KEY_TEN_F5)||(msg == KEY_MODECHG) ||(msg == LCD_DISCONNECT)){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				;
			}else{
				continue;
			}
		}

		switch( KEY_TEN0to9( msg ) )
		{
			case KEY_TEN_F5:
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
				if (mode != 0) {
					break;
				}
				// 時計セット前のみ押下可能
				if(( in_now != -1 ) && ( clk_test( in_now, (char)pos ) != 0 )){ // 入力値範囲ﾁｪｯｸ
					BUZPIPI();
					in_now = -1;
					break;
				}
				if( in_now != -1 ) {
					in_d[pos] = in_now;
					in_now = -1;
					if (pos >= 2) {
						// 年・月・日の位置であれば曜日を更新する
						ndat = dnrmlzm( (short)in_d[2], (short)in_d[3], (short)in_d[4] );
						grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((ndat + 6) % 7)] ); // 曜日表示
					}
				}
				BUZPI();
				// 反転表示を正転表示に戻す
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_d[pos], CLCK_PNT[pos][3], CLCK_PNT[pos][2], 0, COLOR_BLACK, LCD_BLINK_OFF );
				// ｶｰｿﾙ位置を次の位置にｾｯﾄ
				if( msg == KEY_TEN_F2 ){
					pos = ( 4 < pos + 1 ) ? 0 : pos + 1;
				}else{
					pos = ( 0 > pos - 1 ) ? 4 : pos - 1;
				}
				break;
			case KEY_TEN:
				if (mode != 0) {
					break;
				}
				// 時計セット前のみ押下可能
				BUZPI();
				if( in_now == -1 )
				{	// in_now <= 入力値
					in_now = (short)(msg - KEY_TEN0);
				}else{
					if( pos == 2 )
					{	// 年入力であれば4桁の数値にする
						in_now = (short)( in_now % 1000 ) * 10 + (short)(msg - KEY_TEN0);
					}else{
						// 年以外であれば2桁の数値にする
						in_now = (short)( in_now % 10 ) * 10 + (short)(msg - KEY_TEN0);
					}
				}
				// 入力値を表示する（反転表示）
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_now, CLCK_PNT[pos][3], CLCK_PNT[pos][2], 1,
																							COLOR_BLACK, LCD_BLINK_OFF );	/* reverse */
				break;
			case KEY_TEN_CL:
				if (mode != 0) {
					break;
				}
				// 時計セット前のみ押下可能
				BUZPI();
				in_now = -1;
				break;
			case KEY_TEN_F4:
				if (mode != 0) {
					break;
				}
				if( TimeAdjustCtrl(0) == 1 ){	// SNTP機能有効
					// 時計セット前のみ押下可能
					BUZPI();
					ret = (ushort)UsMnt_Clock2();				// 時刻補正機能へ
				}
				break;
			case KEY_TEN_F3:
				if (mode != 0) {
					break;
				}
				// 時計セット前のみ押下可能
				if(( in_now != -1 ) && ( clk_test( in_now, (char)pos ) != 0 )){ // 入力値範囲ﾁｪｯｸ
					BUZPIPI();
					in_now = -1;
					break;
				}
				if (in_now == -1){
					in_now = in_d[pos];
				}
				y = (pos==2) ? in_now : in_d[2];
				m = (pos==3) ? in_now : in_d[3];
				d = (pos==4) ? in_now : in_d[4];
				if( chkdate( y, m, d ) != 0 ){ // 存在する日付かﾁｪｯｸ
					BUZPIPI();
					in_now = -1;
					break;
				}
				in_d[pos] = in_now;
				BUZPI();
				clk_data.year = in_d[2];
				clk_data.mont = (char)in_d[3];
				clk_data.date = (char)in_d[4];
				clk_data.hour = (char)in_d[0];
				clk_data.minu = (char)in_d[1];
				clk_data.seco = 0;
				clk_data.ndat = dnrmlzm( (short)clk_data.year, (short)clk_data.mont, (short)clk_data.date );
				clk_data.nmin = tnrmlz ( (short)0, (short)0, (short)clk_data.hour, (short)clk_data.minu );
				clk_data.week = (unsigned char)((clk_data.ndat + 6) % 7);

				if(prm_get(0, S_NTN, 38, 1, 1) == 1){	//ｼｽﾃﾑ変更送信設定(時計)が１の時
					f_NTNET_RCV_MNTTIMCHG = 1;		// ﾒﾝﾃﾅﾝｽ現在時刻変更
				}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_Rec.edy_status.BIT.MNTTIMCHG = 1;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				Ope_clk_set( &clk_data, OPLOG_TOKEISET2 );			// 時計更新
				in_d[0] = (short)clk_data.hour;
				in_d[1] = (short)clk_data.minu;
				in_d[2] = (short)clk_data.year;
				in_d[3] = (short)clk_data.mont;
				in_d[4] = (short)clk_data.date;
				clk_sub( in_d );
				grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((clk_data.ndat + 6) % 7)] ); // 曜日表示

				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[19] );	// "　　 << セットしました >> 　　"
				Lagtim( OPETCBNO, 6, 50*3 );
				ret2 = -1;
				// 終了キー以外を押下不可能にする
				mode = 1;
				Fun_Dsp( FUNMSG[8] );											// "　　　　　　　　　　　　 終了 "
				break;
		}
		if (mode == 0) {
			// ｶｰｿﾙ位置の数値を反転表示させる
			if(( msg == KEY_TEN_F1 ) || ( msg == KEY_TEN_F2 ) || ( msg == KEY_TEN_F3 ) || ( msg == KEY_TEN_CL )){
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_d[pos], CLCK_PNT[pos][3], CLCK_PNT[pos][2], 1,
																							COLOR_BLACK, LCD_BLINK_OFF );
			}
		}
	}
	Lagcan( OPETCBNO, 6 );
	TimeAdjustCtrl(2);			// 自動時刻補正停止解除
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| NTPサーバーに時刻要求を出し、システム時計を補正する											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_Clock2( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: MATSUSHITA																	   |*/
/*| Date		: 2013-02-13																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
unsigned short	UsMnt_Clock2( void )
{
	ushort	msg;
	struct	clk_rec		clk_data;
	date_time_rec2		nettime;
	short	in_d[6];
	ushort	ret, mode;
	ushort	net_msec;
	ulong	margin[3];		// [0]=day part, [1]=msec part, [2]=sign
	int		ans;


	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "＜時計合わせ＞　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[157] );		// "   センターに接続しています   "
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[114] );		// "　　 しばらくお待ち下さい 　　"
	Fun_Dsp( FUNMSG[0] );

	ReqServerTime(SNTPMSG_RECV_TIME);		// サーバーに時刻要求を出す
	mode = 0;								// SNTPからの応答待ち状態にする
// sntp処理
	for( ret = 0; ret == 0; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// 補正実行フラグを落とす
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// 補正実行フラグを落とす
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:
		// 終了キー
			if (mode != 0) {
				BUZPI();
				ret = MOD_EXT;
			}
			break;
		case SNTPMSG_RECV_TIME:
			if (mode == 0) {
				Lagcan(OPETCBNO, _TIMERNO_SNTP);

			// サーバー時刻取得
				ans = sntpGetTime(&nettime, &net_msec);
				if (ans < 0) {
				// サーバー時間の取得に失敗した場合はタイムアウト終了と同じ処理にする
					Lagtim(OPETCBNO, _TIMERNO_SNTP, 1);
					break;
				}

				if (CLK_REC.nmin == (23*60+59) && CLK_REC.seco >= 50) {
				// システム時計が23:59:50～59の間では時刻補正を行わず、タイムアウト終了と同じ処理にする
					RegistReviseLog(SNTP_GUARD_TIME);		// ログ登録
					Lagtim(OPETCBNO, _TIMERNO_SNTP, 1);
					break;
				}

				err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 0, 0, 0, 0);	// E0083解除
				wmonlg(OPMON_TIME_AUTO_REVISE, 0, 0);					// モニタログ登録

				CheckTimeMargin(&nettime, net_msec, margin);
				if (ans == 1) {
				// 応答に遅延発生
					RegistReviseLog(SNTP_DELAYED_RETRY);	// ログ登録
					ReqServerTime(SNTPMSG_RECV_TIME);
					break;
				}
				net_msec = AddTimeMargin(margin, in_d);

			// 手動時計補正では誤差にかかわらず時刻補正を行う
				BUZPI();
				if (net_msec) {
					// 1ms単位のタイマをセット
					Lagtim_1ms( OPETCBNO, 1, net_msec );
					break;
				}
			}
			// no break
		case TIMEOUT1_1MS:
			if (mode == 0) {
				RegistReviseLog(SNTP_NORMAL_REVISE);

				clk_data.year = in_d[2];
				clk_data.mont = (char)in_d[3];
				clk_data.date = (char)in_d[4];
				clk_data.hour = (char)in_d[0];
				clk_data.minu = (char)in_d[1];
				clk_data.seco = (char)in_d[5];
				clk_data.ndat = dnrmlzm( (short)clk_data.year, (short)clk_data.mont, (short)clk_data.date );
				clk_data.nmin = tnrmlz ( (short)0, (short)0, (short)clk_data.hour, (short)clk_data.minu );
				clk_data.week = (unsigned char)((clk_data.ndat + 6) % 7);
				if(prm_get(0, S_NTN, 38, 1, 1) == 1){		// ｼｽﾃﾑ変更送信設定(時計)が１の時
					f_NTNET_RCV_MNTTIMCHG = 1;				// ﾒﾝﾃﾅﾝｽ現在時刻変更
				}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_Rec.edy_status.BIT.MNTTIMCHG = 1;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

				mode = 1;									// SNTP応答受信後の状態にする
				Lagtim( OPETCBNO, 6, 50*3 );				// 時刻セット後3秒経過で上位メニューに戻るタイマー
				Ope_clk_set( &clk_data, OPLOG_SET_TIME2_SNTP );		// 時計更新

				dispclr();
				grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "＜時計合わせ＞　　　　　　　　"
				grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[0] );			// "　　　　　　　　：　　　　　　"
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[1] );			// "　　　　　年　　月　　日（　）"

				in_d[0] = (short)clk_data.hour;
				in_d[1] = (short)clk_data.minu;
				in_d[2] = (short)clk_data.year;
				in_d[3] = (short)clk_data.mont;
				in_d[4] = (short)clk_data.date;
				in_d[5] = (short)clk_data.seco;
				clk_sub( in_d );						// システム時計の時刻を表示

// MH810100(S) K.Onodera 2020/03/27 車番チケットレス（#3960 SNTPでの時計セット後、即時反映されない）
//				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[19] );		// "　　 << セットしました >> 　　"
				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[45] );		// "　<<数秒経過後に完了します　>>"
// MH810100(E) K.Onodera 2020/03/27 車番チケットレス（#3960 SNTPでの時計セット後、即時反映されない）
				Fun_Dsp( FUNMSG[8] );													// "　　　　　　　　　　　　 終了 "
			}
			break;
		case TIMEOUT_SNTP:
		// SNTPタイムアウト
			if (mode == 0) {
				RegistReviseLog(SNTP_TIMEOUT);			// ログ登録
				RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// 失敗 補正実行フラグを落とす

			// 手動の場合は再要求しない
				BUZPIPI();
				err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 1, 0, 0, 0);		// E0083発生
				dispclr();
				grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "＜時計合わせ＞　　　　　　　　"
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[158] );		// "   センターに接続できません   "
				grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[159] );		// "   時刻の取得に失敗しました   "
				Fun_Dsp( FUNMSG[8] );													// "　　　　　　　　　　　　 終了 "
				mode = 2;
			}
			break;
		case TIMEOUT6:
		// 時刻セット後3秒経過した時のタイマー
			ret = MOD_EXT;
			break;
		default:
			break;
		}
	}
	Lagcan(OPETCBNO, _TIMERNO_SNTP);
	Lagcan_1ms(OPETCBNO, 1);
	if (mode == 1) {
		TimeAdjustCtrl(3);					// 停止解除＆リセット要求
	}
	else {
		TimeAdjustCtrl(2);					// 停止解除要求
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  時計の更新処理                                                        |*/
/*[]----------------------------------------------------------------------[]*/
void	Ope_clk_set( struct clk_rec *p_clk_data, ushort OpeMonCode )
{
	struct	clk_rec wk_CLK_REC_old;			// 時計更新前の時間
	ushort	wk_CLK_REC_msec_old;			//					ミリ秒
	ulong	wkul,wkul2;
	unsigned char clkstr[11];
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;

	c_Now_CLK_REC_ms_Read( &wk_CLK_REC_old, &wk_CLK_REC_msec_old );		// 時計更新前の時間get

	timset( p_clk_data );									// 時計更新

	// 時計更新した旨をOPEタスクへ通知
	// 時計CPUから通知されたのと同じI/Fで通知する
	clkstr[0] = binbcd( (unsigned char)( p_clk_data->year / 100 ));
	clkstr[1] = binbcd( (unsigned char)( p_clk_data->year % 100 ));
	clkstr[2] = binbcd( p_clk_data->mont );
	clkstr[3] = binbcd( p_clk_data->date );
	clkstr[4] = binbcd( p_clk_data->hour );
	clkstr[5] = binbcd( p_clk_data->minu );
	clkstr[6] = binbcd( p_clk_data->seco );

	u_LifeTime.ul = LifeTim2msGet();
	clkstr[7] = u_LifeTime.uc[0];
	clkstr[8] = u_LifeTime.uc[1];
	clkstr[9] = u_LifeTime.uc[2];
	clkstr[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// 日時同期データ送信要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	queset( OPETCBNO, CLOCK_CHG, 11, clkstr ); // Send message to opration task
// MH810104 GG119201(S) 電子ジャーナル対応
	if (isEJA_USE()) {
		// 電子ジャーナルへ時刻設定データを送信する
		OPECTL.EJAClkSetReq = 1;
	}
// MH810104 GG119201(E) 電子ジャーナル対応

	// 時計更新LOG登録
	wkul = c_Normalize_sec( &wk_CLK_REC_old );				// 変更前時間
	wkul2 = c_Normalize_sec( p_clk_data );					// 変更後時間
	wopelg( OpeMonCode, wkul, wkul2 );						// 操作LOG登録(FT-4800)
// MH810100(S) Y.Yamauchi 2020/02/26 車番チケットレス（時計合わせ）
	if( OpeMonCode == OPLOG_SET_TIME2_SNTP ){	// SNTP 自動時計更新機能による時計セット、時計合わせ（センター）
		PKTcmd_clock( 1 );				// LCDに時計送信（センター）
	} else {							// 時計合わせ（手動）、NTNET（時計データ：119）
		PKTcmd_clock( 0 );				// LCDに時計送信(手動)
	}
// MH810100(E) Y.Yamauchi 2020/02/26 車番チケットレス（時計合わせ）
}

/*[]----------------------------------------------------------------------[]*/
/*|  時計データのチェック                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| RETURN VALUE : short	ret;    0 : OK  -1 : NG                        |*/
/*[]----------------------------------------------------------------------[]*/
char clk_test( short data, char pos )
{
	switch( pos ){
		case 0:		//hour
			if( data >= 24 ) {
				return( (char)-1 );
			}
			break;
		case 1:		//minit
			if( data >= 60 ) {
				return( (char)-1 );
			}
			break;
		case 2:		//year
			if(( data < 1990 ) || ( data > 2050 )){
				return( (char)-1 );
			}
			break;
		case 3:		//month
			if(( data > 12 ) || ( data < 1 )){
				return( (char)-1 );
			}
			break;
		case 4:		//date
			if(( data > 31 ) || ( data < 1 )){
				return( (char)-1 );
			}
			break;
		case 5:		//week
			break;
		default:
			return( (char)-1 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*|  時計データの表示                                                      |*/
/*[]----------------------------------------------------------------------[]*/
static void clk_sub( short * dat )
{
	ushort ndat;

	opedsp( 2, 12, (unsigned short)dat[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 18, (unsigned short)dat[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	opedsp( 3, 2, (unsigned short) dat[2], 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	//not 0sup
	opedsp( 3, 12, (unsigned short) dat[3], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 18, (unsigned short) dat[4], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );

	ndat = dnrmlzm( (short)dat[2], (short)dat[3], (short)dat[4] );
	grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((ndat + 6) % 7)] );

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| ｼｬｯﾀｰ開放                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_shtctl( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	SHTMOD_CNT	2

unsigned short	UsMnt_shtctl( void )
{
	ushort	msg;
	long	*pSHT;
	const uchar	*str_tbl[] = {DAT2_2[1], DAT2_2[0]};
	char	pos;

	pSHT = &PPrmSS[S_P01][1];
	pos = (*pSHT == 0) ? 0 : 1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[33] );					/* "＜シャッター開放＞　　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "　現在の状態　　　　　　　　　" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "　　　　　　：　　　　　　　　" */
	grachr( 2, 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "　　　　　　　ＸＸ　　　　　　" */
	grachr( 4, 14,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "　　　　　　　自動　　　　　　" */
	grachr( 5, 14,  4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "　　　　　　　開放　　　　　　" */
	Fun_Dsp( FUNMSG[20] );								/* "　▲　　▼　　　　 書込  終了 " */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				return( MOD_EXT );
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
				BUZPI();
				grachr((ushort)(4+pos), 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				pos ^= 1;
				grachr((ushort)(4+pos), 14,  4, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				break;
			case KEY_TEN_F4:					/* F4:書込 */
				BUZPI();
				*pSHT = (long)pos;
				OpelogNo = OPLOG_SHUTTERKAIHO;		// 操作履歴登録
				grachr( 2, 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 営休業切換                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_OCsw( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	OCMOD_CNT	3

unsigned short	UsMnt_OCsw( void )
{
	ushort	msg;
	long	*pOPCL;
	long	val_tbl[] = {1, 0, 2};
	const uchar	*str_tbl[] = {DAT4_2[12], DAT4_2[13], DAT4_2[14]};
	int		pos;

	pOPCL = &PPrmSS[S_P01][2];
	for (pos = 0; pos < 2; pos++) {
		if (*pOPCL == val_tbl[pos])
			break;
	}
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[11] );					/* "＜営休業切替＞　　　　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "　現在の状態　　　　　　　　　" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "　　　　　　：　　　　　　　　" */
	grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "　　　　　　　ＸＸＸＸ　　　　" */
	grachr( 4, 12,  8, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "　　　　　　　強制営業　　　　" */
	grachr( 5, 12,  8, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "　　　　　　　　自動　　　　　" */
	grachr( 6, 12,  8, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[2] );	/* "　　　　　　　強制休業　　　　" */
	Fun_Dsp( FUNMSG[20] );								/* "　▲　　▼　　　　 書込  終了 " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:					/* F5:Exit */
			BUZPI();
			return( MOD_EXT );
		case KEY_TEN_F1:					/* F1: */
		case KEY_TEN_F2:					/* F2: */
			BUZPI();
			grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 2;
			}
			else {
				if (++pos > 2)
					pos = 0;
			}
			grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			break;
		case KEY_TEN_F4:					/* F4:書込 */
			BUZPI();
			*pOPCL = val_tbl[pos];
			OpelogNo = OPLOG_EIKYUGYOKIRIKAE;		// 操作履歴登録
			grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
// MH810100(S) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Total                                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Total( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : hashimo                                                 |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	DispMax_Syu	6
#define	UsMnt_Total_Wait_To	50*3 //3S
unsigned short	UsMnt_Total( short syu_kind )
{
static	const	ushort	menu_reverse[3] = { 12, 12, 16 };
static	const	ushort	pri_cmd[][3] = {
		{PREQ_TSYOUKEI, PREQ_TGOUKEI, PREQ_TGOUKEI_Z}		// T
	,	{PREQ_GTSYOUKEI, PREQ_GTGOUKEI, PREQ_GTGOUKEI_Z}	// GT
	,	{PREQ_MTSYOUKEI, PREQ_MTGOUKEI, PREQ_MTGOUKEI_Z}	// MT
};
static	const	ushort	ope_log[][3] = {
		{OPLOG_T_SHOKEI, OPLOG_T_GOKEI, OPLOG_T_ZENGOKEI}
	,	{OPLOG_GT_SHOKEI, OPLOG_GT_GOKEI, OPLOG_GT_ZENGOKEI}
	,	{OPLOG_MT_SHOKEI, OPLOG_MT_GOKEI, OPLOG_MT_ZENGOKEI}
};
	short			msg;
	unsigned char	pos[2], menu_su;
	uchar			pri_kind;		// 印字先ﾌﾟﾘﾝﾀ種別

	T_FrmSyuukei	FrmSyuukei;
	T_FrmPrnStop	FrmPrnStop;
	int				inji_end;
	int				can_req;
	uchar			fg_Ttotal;		//Ｔ集計の「合計プリント」終了判定フラグ
	struct SKY		*p_sky;
	SYUKEI			*p_syukei;
	date_time_rec	*p_sky_date;	// 集計日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	ushort			pri_req;
	ushort			oplg_no;
	uchar			mnt_sw = 0;		// 印字中にメンテナンスOFFになったかどうか
	uchar			R_END;			// 印字正常終了フラグ
	uchar			J_END;
	uchar			LogCount;		// ログ登録件数
	uchar			maxPage;		// 最大ページ数
	uchar			nowPage;		// 現在表示ページ
	uchar			nowselect;		// 現在のカーソル位置値
	uchar			syu_elog;		// T/GT集計のeLog値を保持

// initialize ---------------
	pos[0] = 0;
	pos[1] = 0;

TOTALTOP:
// menu loop ---------------
	Lagcan( OPETCBNO, 6 );
	dispclr();
	pri_kind = 0;
	R_END = 0;
	J_END = 0;
	fg_Ttotal = 0;			//Ｔ集計の「合計プリント」終了判定フラグ

	switch(syu_kind) {
	default:
//	case	MNT_TTOTL:
		menu_su = 0;
		break;
	case	MNT_GTOTL:
		menu_su = 1;
		break;
	case	MNT_MTOTL:
		menu_su = 19;
		break;
	}
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[menu_su] );	// タイトル行
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[2] );			// [02]	"　小計プリント　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[3] );			// [03]	"　合計プリント　　　　　　　　"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[4] );			// [04]	"　前回合計プリント　　　　　　"
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[5] );			// [05]	"　実行：プリントを開始します　"
	Fun_Dsp( FUNMSG[68] );													// [XX]	"　▲　　▼　　　　 実行  終了 "

	grachr( (unsigned short)(pos[1]+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[1]+2]);
	grachr( (unsigned short)(pos[0]+2), 2, menu_reverse[pos[0]], 1, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[0]+2]+2);

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return( MOD_CHG );

			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return( MOD_EXT );

			case KEY_TEN_F1:						/* F1: */
				BUZPI();
				pos[0] = (unsigned char)(( 0 == pos[0] ) ? ( 2 ) : ( pos[0] - 1 ));
				break;

			case KEY_TEN_F2:						/* F2: */
				BUZPI();
				pos[0] = (unsigned char)((2 == pos[0] ) ? 0 : ( pos[0] + 1 ));
				break;

			case TIMEOUT6:							// タイムアウト
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[21] );// "　　　　　　　　　　　　　　　"
				break;

			case KEY_TEN_F4:						/* F5: 実行 */
				if( OPECTL.Mnt_lev < 2 ){								// 係員でログイン中？
					if(	(syu_kind == MNT_TTOTL && pos[0] == 1) ||		//  T合計を実施する？
						(syu_kind == MNT_GTOTL && pos[0] == 1) ||		// GT合計を実施する？
						(syu_kind == MNT_MTOTL && pos[0] == 1)){		// MT合計を実施する？
						BUZPIPI();										// エラー音
						break;											// 実行権がないので抜ける
					}
				}
			// decide source data
				if (CheckPrinter(pri_kind) == 0 ||
					auto_syu_prn == 2) {
				// プリンタ異常
				// 自動集計中
					BUZPIPI();
					break;
				}
				p_sky = (pos[0] == 2)? &skybk : & sky;
				switch(syu_kind) {
				default:
			//	case	MNT_TTOTL:
					p_syukei = &p_sky->tsyuk;
					pri_req = pri_cmd[0][pos[0]];
					oplg_no = ope_log[0][pos[0]];
					syu_elog = eLOG_TTOTAL;
					break;
				case	MNT_GTOTL:
					p_syukei = &p_sky->gsyuk;
					pri_req = pri_cmd[1][pos[0]];
					oplg_no = ope_log[1][pos[0]];                           //操作ログ登録｛0=T集計、1=GT集計、２=MT集計｝
					syu_elog = eLOG_GTTOTAL;
					break;
				case	MNT_MTOTL:
					p_syukei = &p_sky->msyuk;
					pri_req = pri_cmd[2][pos[0]];
					oplg_no = ope_log[2][pos[0]];
					break;
				}
			// check condtion
				if (pos[0] == 1) {
				// 合計？
					if (CountCheck() == 0) {
					// 金庫集計中
						grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " 処理中：しばらくお待ち下さい "
						Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
						BUZPIPI();
						break;
					}
					if (pri_req == PREQ_TGOUKEI) {
					// フラッシュ書き込み中はT集計不可とする(ﾊﾞｸﾞNo.157)
						if (AppServ_IsLogFlashWriting(eLOG_TTOTAL) != 0) {	// Flashへの書込み中？
							grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " 処理中：しばらくお待ち下さい "
							Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
							BUZPIPI();					// 書き込み中なのでエラー音
							break;						// 抜ける
						}
					}
					else if (pri_req == PREQ_GTGOUKEI) {
					// フラッシュ書き込み中はGT集計不可とする
						if (AppServ_IsLogFlashWriting(eLOG_GTTOTAL) != 0) {	// Flashへの書込み中？
							grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " 処理中：しばらくお待ち下さい "
							Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
							BUZPIPI();					// 書き込み中なのでエラー音
							break;						// 抜ける
						}
					}
					pri_kind = PrnGoukeiPri(pri_req);
				}
				else {
					if (pos[0] == 2) {
						if( MNT_MTOTL != syu_kind ){	// MT集計の場合は従来動作
							// 前回集計？
							if( 0 == Ope_Log_TotalCountGet(syu_elog)){	// ログ登録件数取得
								// 前回集計？
								p_sky_date = (date_time_rec*)&p_syukei->NowTime;
								if( (p_sky_date->Year == 0) &&
								    (p_sky_date->Mon  == 0) &&
								    (p_sky_date->Day  == 0) &&
								    (p_sky_date->Hour == 0) &&
									(p_sky_date->Min  == 0) ){
									// 前回集計ﾃﾞｰﾀがない場合
									BUZPIPI();
									break;
								}
							} else {
								BUZPI();
								goto SELECTDATE;	// T/GT集計の場合、日付選択画面へ
							}
						} else {
							// 前回集計？
							p_sky_date = (date_time_rec*)&p_syukei->NowTime;
							if( (p_sky_date->Year == 0) &&
							    (p_sky_date->Mon  == 0) &&
							    (p_sky_date->Day  == 0) &&
							    (p_sky_date->Hour == 0) &&
								(p_sky_date->Min  == 0) ){
								// 前回集計ﾃﾞｰﾀがない場合
								BUZPIPI();
								break;
							}
						}
					}
					pri_kind = R_PRI;
				}
				BUZPI();
				wopelg( oplg_no , 0, 0 );
				Lagcan( OPETCBNO, 6 );
				goto TOTALPRN;		// Go Print!!

			default:
				break;
		}

		if(pos[0] != pos[1])
		{
			grachr( (unsigned short)(pos[1]+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[1]+2]);
			grachr( (unsigned short)(pos[0]+2), 2, menu_reverse[pos[0]], 1, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[0]+2]+2);
			pos[1] = pos[0];
		}

	}

SELECTDATE:
// request to print ----------
	Lagcan( OPETCBNO, 6 );

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[20] );	// ＜前回合計プリント＞
	Fun_Dsp(FUNMSG2[51]);								// "　▲　　▼　　⊃　 実行  終了 "

	LogCount = Ope_Log_TotalCountGet(syu_elog);		// ログ登録件数取得
			
	if( 0 != LogCount % DispMax_Syu ){
		maxPage = (LogCount / DispMax_Syu ) + 1;		// 最大ページ数をセット
	} else {
		maxPage = (LogCount / DispMax_Syu );			// 最大ページ数をセット
	}
	nowPage = 1;
	nowselect = 1;
	syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// データ一括表示


	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:				/* F5:終了 */
			BUZPI();
			goto TOTALTOP;
			break;
			return( MOD_EXT );
		case KEY_TEN_F4:				/* F4:発行 */
// MH810105 GG119202(S) T合計連動印字対応
//			p_syukei = (SYUKEI*)SysMnt_Work;
			memset(g_TempUse_Buffer, 0, sizeof(g_TempUse_Buffer));
			p_syukei = (SYUKEI*)g_TempUse_Buffer;
// MH810105 GG119202(E) T合計連動印字対応
			if( 0 == Ope_Log_1DataGet( syu_elog, (ushort)LogCount - (nowPage - 1)*DispMax_Syu - nowselect , p_syukei ) ){	// 最古からn番目のログを取得
				BUZPIPI();	// 正しく取得できなければ抜ける
				break;
			}
			pri_kind = R_PRI;
			BUZPI();
			goto TOTALPRN;		// Go Print!!
			break;
		case KEY_TEN_F3:				/* F3:ラベル */
			BUZPI();
			if( nowPage == maxPage ){
				nowPage = 1;
				nowselect = 1;
			} else {
				nowPage++;
				nowselect = 1;
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// データ一括表示
			break;
		case KEY_TEN_F1:				/* F1:▲ */
			BUZPI();
			if (nowselect > 1) {
				nowselect--;			// 選択位置を1つ↑へ
			}else{
				if( nowPage == 1 ){
					nowPage = maxPage;
					nowselect = LogCount - ( maxPage - 1)*DispMax_Syu;
				} else {
					nowPage--;
					nowselect = DispMax_Syu;
				}
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// データ一括表示
			break;
		case KEY_TEN_F2:				/* F2:▼ */
			BUZPI();
			if( nowPage == maxPage ){
				if ( nowselect < LogCount - ( maxPage - 1)*DispMax_Syu) {
					nowselect++;
				}else{
					nowPage = 1;
					nowselect = 1;
				}				
			} else {
				if ( nowselect < DispMax_Syu) {
					nowselect++;
				}else{
					nowPage++;
					nowselect = 1;
				}
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// データ一括表示
			break;
		default:
			break;
		}
	}

TOTALPRN:
// request to print ----------
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[menu_su] );	// タイトル行
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8] );			// [08]	"　　　 プリント中です 　　　　"
	Fun_Dsp( FUNMSG[82] );													// [51]	"　　　　　　 中止 　　　　　　"

	if (pos[0] != 2) {
	// 小計・合計
		FrmSyuukei.prn_data = (void*)syuukei_prn((int)pri_req, p_syukei);
	}
	else {
		FrmSyuukei.prn_data = (void*)p_syukei;
	}
	if (pos[0] != 1) {													// 合計以外
		// 前回の合計で中止した場合、以下のフラグがONままとなるのでOFFにする
		rct_goukei_pri = OFF;											// 合計記録印字状態（ﾚｼｰﾄ用）ﾘｾｯﾄ
		jnl_goukei_pri = OFF;											// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）ﾘｾｯﾄ
	}
	FrmSyuukei.prn_kind = pri_kind;
	memcpy( &FrmSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// 現在時刻ｾｯﾄ
// MH810105 GG119202(S) T合計連動印字対応
	FrmSyuukei.print_flag = 0;
	if (isEC_USE() &&
		(pri_req == PREQ_TGOUKEI ||
		 pri_req == PREQ_TGOUKEI_Z)) {
		// 決済リーダ接続ありでT合計印字の場合は設定を参照して連動印字を行う
		ec_linked_total_print(pri_req, &FrmSyuukei);
	}
	else {
// MH810105 GG119202(E) T合計連動印字対応
	queset( PRNTCBNO, pri_req, sizeof(T_FrmSyuukei), &FrmSyuukei );
// MH810105 GG119202(S) T合計連動印字対応
	}
// MH810105 GG119202(E) T合計連動印字対応

	for (inji_end = 0, can_req = 0; inji_end < 10; ){

		//----------------------------------------
		// 印字終了待ち
		//----------------------------------------

		msg = StoF( GetMessage(), 1 );

		if( msg == (INNJI_ENDMASK|pri_req) )
		{
			if( rct_goukei_pri != ON){												// レシートプリンタ動作していない
				if( OPECTL.Pri_Kind == 1 &&											// レシート印字完了
					PRI_NML_END == check_print_result(0) &&							// 正常終了
					PrnGoukeiChk( (ushort)(msg & (~INNJI_ENDMASK))) == YES){		// 合計印字
					R_END = 1;
				}
			}
			if( jnl_goukei_pri != ON){												// ジャーナルプリンタ動作していない
				if( OPECTL.Pri_Kind == 2 &&											// ジャーナル印字完了
					PRI_NML_END == check_print_result(1) &&							// 正常終了
					PrnGoukeiChk( (ushort)(msg & (~INNJI_ENDMASK))) == YES){		// 合計印字
					J_END = 1;
				}
			}
			if ( pos[0] == 1 ) {				// T/GT/MT合計の印字中？
			// 両プリンタ（集計）が終了している？
				if (rct_goukei_pri != ON && jnl_goukei_pri != ON) {
					inji_end = 1;				// 印字終了
				}
			}
			else {
			// この場合はレシートプリンタのみ
				inji_end = 1;					// 印字終了
			}

			msg &= (~INNJI_ENDMASK);
		}

		switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			// ドアノブの状態にかかわらずトグル動作してしまうので、
			// ドアノブ閉かどうかのチェックを実施
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			mnt_sw = 1;			// メンテOFF保持
			if (inji_end == 0 || inji_end == 4 ){
				break;			// 印字実行中は無視
			}
			inji_end = 10;		// ループend
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			break;

		case KEY_TEN_F4:
			if(inji_end != 3){
				break;
			}
			inji_end = 1;
			// no break
		case KEY_TEN_F3:						/* F3:Stop */
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// 印字中止要求
					can_req = 1;
					if (pri_kind != 0) {
						FrmPrnStop.prn_kind = pri_kind;
						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					}
				}
				if (pri_req == PREQ_TGOUKEI) {
					CountFree(T_TOTAL_COUNT);			// 12-40
					wopelg( OPLOG_CAN_T_SUM , 0, 0 );
				} else if (pri_req == PREQ_GTGOUKEI) {
					CountFree(GT_TOTAL_COUNT);
					wopelg( OPLOG_CAN_GT_SUM , 0 ,0 );	// 12-41
				} else if (pri_req == PREQ_MTGOUKEI) {
					wopelg( OPLOG_CAN_MT_SUM , 0 ,0 );	// 12-42
				}
				
				break;
			}
			else if (inji_end == 1) {
				BUZPI();
				inji_end = 10;

				if (pri_req == PREQ_TGOUKEI) {	// 用紙無でいいえをした場合、追番を返却
					// T合計
					CountFree(T_TOTAL_COUNT);
					wopelg( OPLOG_CAN_T_SUM , 0 ,0 );	// 12-40
				} else if (pri_req == PREQ_GTGOUKEI) {
					// GT合計
					CountFree(GT_TOTAL_COUNT);
					wopelg( OPLOG_CAN_GT_SUM , 0 ,0 );	// 12-41
				} else if( pri_req == PREQ_MTGOUKEI) {
					// MT合計
					wopelg( OPLOG_CAN_MT_SUM , 0 ,0 );	// 12-42
				}
			}
			else if (inji_end == 3) {								// 印字なしで集計→はい
				BUZPI();
				/*	レシート・ジャーナル両方印字設定で片方紙切れだった場合	*/
				/*	ここで使えるプリンタに印字要求を出す					*/
				if( (pri_kind == 3) && 						// レシート・ジャーナル両方に印字
					(R_END != 1 && J_END != 1)){			// 両方とも印字完了していない
					if(CheckPrinter(RJ_PRI)){				// 両プリンタ使用可
						pri_kind = RJ_PRI;
						goto TOTALPRN;
					}
					else if(CheckPrinter(R_PRI)){			// レシートプリンタ使用可
						pri_kind = R_PRI;
						goto TOTALPRN;
					}
					else if( CheckPrinter(J_PRI) ){			// ジャーナルプリンタ使用可
						pri_kind = J_PRI;
						goto TOTALPRN;
					}
				}
				/*	印字しなかった場合、集計電文送信とｸﾘｱを行う 		*/
				/*	この時点で印字完了フェーズとする					*/
				switch( syu_kind ){
					case MNT_TTOTL:									// Ｔ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						if( Check_syuukei_clr( 0 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						ac_flg.syusyu = 11;
						Make_Log_TGOUKEI();				// 車室毎集計を含めたT合計ﾛｸﾞ作成
						CountGet( T_TOTAL_COUNT, &p_syukei->Oiban );		// T合計追番
						syuukei_clr( 0 );							// Ｔ集計ｴﾘｱ更新処理
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// バッチ送信設定有
							if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//						if (prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1) {
//							// 手動「Ｔ合計プリント」後のセンター通信処理（締め）
//							auto_centercomm( 1 );
//							if( edy_cnt_ctrl.BIT.exec_status ){
//								inji_end = 4;						// Edyセンター通信実施中
//								continue;
//							}
//						}						
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						break;
					case MNT_GTOTL:									// ＧＴ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						if( Check_syuukei_clr( 1 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						ac_flg.syusyu = 22;
						CountGet( GT_TOTAL_COUNT, &p_syukei->Oiban );		// GT合計追番
						syuukei_clr( 1 );							// ＧＴ集計ｴﾘｱ更新処理
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// バッチ送信設定有(T合計と同じ）
							if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						break;
					case MNT_MTOTL:									// ＭＴ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						if( Check_syuukei_clr( 2 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						ac_flg.syusyu = 102;
						syuukei_clr( 2 );							// ＭＴ集計ｴﾘｱ更新処理
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						break;
				}
				inji_end = 10;
			}
			break;
			case KEY_TEN_F5:						/* F5:Exit */
				if( (inji_end != 0) && (inji_end != 3) && (inji_end != 4) ){
					BUZPI();
					goto TOTALTOP;
				}
			break;

		case PREQ_TSYOUKEI:  	// Ｔ小計
		case PREQ_GTSYOUKEI: 	// ＧＴ小計
		case PREQ_TGOUKEI:  	// Ｔ合計
		case PREQ_GTGOUKEI: 	// ＧＴ合計
		case PREQ_TGOUKEI_Z:	// 前回Ｔ
		case PREQ_GTGOUKEI_Z: 	// 前回ＧＴ
		case PREQ_MTSYOUKEI: 	// ＭＴ小計
		case PREQ_MTGOUKEI: 	// ＭＴ合計
		case PREQ_MTGOUKEI_Z: 	// 前回ＭＴ
			if( inji_end == 1 || (inji_end == 0 && OPECTL.Pri_Result == PRI_ERR_END))
			{
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
//				if (mnt_sw == 1) {
//					goto TOTALTOP;
//				}
				// ドアノブ閉状態の場合はメンテナンス終了
				if (mnt_sw == 1 || CP_MODECHG) {
					return MOD_CHG;
				}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				switch( OPECTL.Pri_Result ){	// 印字結果？
					case PRI_NML_END:
					case PRI_CSL_END:
						if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){
							inji_end = 10;
						// Edyユーザメンテ（Ｔ集計「合計プリント」終了後の締め処理実施）
							if((syu_kind == MNT_TTOTL) && (pos[0] == 1) && (fg_Ttotal == 0)) {
							//Ｔ合計プリントが正常終了後１回のみ実施する
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//								if (prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1) {
//								// 手動「Ｔ合計プリント」後のセンター通信処理（締め）
//									auto_centercomm( 1 );
//									if( edy_cnt_ctrl.BIT.exec_status ){
//										inji_end = 4;						// Edyセンター通信実施中
//									}
//								}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
								fg_Ttotal = 1;		// 締め実行済み
							}
						}
						break;
					case PRI_ERR_END:
// MH810100(S) Y.Yamauchi 2020/02/17 #3856 紙切れやエラー発生中に「プリント中です」と表示される
						displclr(3);
// MH810100(E) Y.Yamauchi 2020/02/17 #3856 紙切れやエラー発生中に「プリント中です」と表示される
						if ((ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0) ||	// ﾚｼｰﾄ紙切れ
							(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0)){	// ｼﾞｬｰﾅﾙ紙切れ
							grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[10] );		//"　印字できません（紙切れ）　　"
						}
						else{
							grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[9] );		//"　印字できません（エラー）　　"
						}
						if((msg == PREQ_TGOUKEI) || (msg == PREQ_GTGOUKEI) || (msg == PREQ_MTGOUKEI) ){	// 合計印字
							grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[15] );		//"　印字なしで集計を行いますか？"
							grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[17] );		//"いいえの時は問題を解決した後で"
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[18] );		//"再度集計処理を行って下さい。　"
							Fun_Dsp( FUNMSG[19] );												//"　　　　　　 はい いいえ　　　"
							inji_end = 3;
						}
						else{																	// 合計印字以外
							Fun_Dsp( FUNMSG[8] );												// [08]	"　　　　　　　　　　　　 終了 "
						}
						
					default:
						break;
				}
			}
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case IBK_EDY_RCV:
//			if( inji_end == 4 && !edy_cnt_ctrl.BIT.exec_status ){
//				inji_end = 10;
//			}
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		default:
			break;
		}
	}

	if (mnt_sw == 1) {
	// 印字中にメンテナンスOFFされた(印字終了で待機画面へ)
		return(MOD_CHG);
	}
	goto TOTALTOP;		// メニューへ戻る
}

/*[]-----------------------------------------------------------------------[]*/
/*|  集計印字履歴表示(5件まで)												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : receipt_all_dsp( topNo, rev_no, elog )					|*/
/*| PARAMETER    : uchar topNo		: 先頭LogNo.							|*/
/*|              : uchar rev_no		: 反転番号								|*/
/*|              : uchar elog		: T/GT集計の状態						|*/
/*| RETURN VALUE : void														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]をﾜｰｸｴﾘｱとして使用する。									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	syukei_all_dsp(uchar topNo, uchar rev_no, uchar elog )
{
// MH810105 GG119202(S) T合計連動印字対応
//	SYUKEI	*syukei = (SYUKEI*)SysMnt_Work;
	SYUKEI	*syukei = (SYUKEI*)g_TempUse_Buffer;
// MH810105 GG119202(E) T合計連動印字対応
	uchar	i;
	rev_no = rev_no - 1;	// 補正
	
	for (i = 0; i < DispMax_Syu; i++,topNo--) {
		if( 1 == Ope_Log_1DataGet( elog, (ushort)topNo, syukei ) ){	// 最古からn番目のログを取得
			grachr( 1+i,  1, 18, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF, &LOGSTR2[1][12] );	// "　　年　　月　　日"
			grachr( 1+i, 23,  2, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		// "："
			opedsp( 1+i,  1, (ushort)syukei->NowTime.Year, 2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// 年
			opedsp( 1+i,  7, (ushort)syukei->NowTime.Mon,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// 月
			opedsp( 1+i, 13, (ushort)syukei->NowTime.Day,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// 日
			opedsp( 1+i, 19, (ushort)syukei->NowTime.Hour, 2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// 時
			opedsp( 1+i, 25, (ushort)syukei->NowTime.Min,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// 分
			if( topNo == 0 ){
				i++;
				break;
			}
		} else {
			break;
		}
	}
	for ( ; i < DispMax_Syu; i++) {
		grachr(1+i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Money Control                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Mnctl( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	COINCHK_CNT	4	//予蓄、保留は別画面

unsigned short	UsMnt_Mnctlmenu( void ){

	unsigned short	usUmnyEvent;
	char	wk[2];
	char	org[2];
	T_FrmTuriKan	FrmTuriKan;
	ushort			msg;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[0] );		// [00]	"＜釣銭管理＞　　　　　　　　　" */

		if( CPrmSS[S_KAN][1] == 0 ){
			// 金銭管理ﾓｰﾄﾞ設定＝しない
			usUmnyEvent = Menu_Slt( MNYMENU, USM_MNC_TBL1, (char)USM_MNC_MAX1, (char)1 );
		}
		else{
			// 金銭管理ﾓｰﾄﾞ設定＝する
			usUmnyEvent = Menu_Slt( MNYMENU, USM_MNC_TBL2, (char)USM_MNC_MAX2, (char)1 );
		}
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){

			case MNT_MNCNT:
				if( CPrmSS[S_KAN][1] ){
					usUmnyEvent = UsMnt_mnyctl();
				}else{
					usUmnyEvent = UsMnt_mnyctl2();
				}
				break;
			case MNT_IVTRY:
				usUmnyEvent = UsMnt_invtry();
				break;
			case MNT_MNYBOX:
				usUmnyEvent = UsMnt_mnybox();
				break;
			case MNT_MNYCHG:
				if( CPrmSS[S_KAN][1] ){
					usUmnyEvent = UsMnt_mnychg();
				}else{
					BUZPIPI();
				}
				break;
			case MNT_MNYPRI:	// 釣銭管理小計プリント
				turikan_gen();
				FrmTuriKan.prn_kind = R_PRI;
				FrmTuriKan.prn_data = &turi_kan;
				turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];							// 機械№
				turi_kan.Kakari_no = OPECTL.Kakari_Num;									// 係員番号set
				memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );			// 現在時刻
				turikan_subtube_set();
				queset( PRNTCBNO, PREQ_TURIKAN_S, sizeof(T_FrmTuriKan), &FrmTuriKan  );	// 釣銭管理（小計）印字要求
				OpelogNo = OPLOG_TURIKAN_SHOKEI;										// 操作履歴登録

				// プリント終了を待ち合わせる
				Lagtim( OPETCBNO, 6, 5*50 );	// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ(５秒)起動
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					// プリント終了
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					// タイムアウト検出
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					// モードチェンジ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//					if (msg == KEY_MODECHG) {
					if (msg == KEY_MODECHG || msg == LCD_DISCONNECT) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						// ドアノブの状態にかかわらずトグル動作してしまうので、
						// ドアノブ閉かどうかのチェックを実施
						if (CP_MODECHG && msg == KEY_MODECHG) {
							break;
						}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						BUZPI();
						Lagcan(OPETCBNO, 6);

						OPECTL.Mnt_mod = 0;
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						OPECTL.Mnt_lev = (char)-1;
						OPECTL.PasswordLevel = (char)-1;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//						return(MOD_CHG);
						if( msg == KEY_MODECHG ){
							return(MOD_CHG);
						} else {
							return(MOD_CUT);
						}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					}
					// Ｆ５（終了）キー
					if (msg == KEY_TEN_F5) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						DP_CP[0] = org[0];
						DP_CP[1] = org[1];
						return( MOD_EXT );
					}
				}
				break;
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( MOD_EXT );
				break;

			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
			UserMnt_SysParaUpdateCheck( OpelogNo );
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usUmnyEvent == MOD_CHG ){
		if( usUmnyEvent == MOD_CHG ||usUmnyEvent == MOD_CUT ){	
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			return usUmnyEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			釣銭補充事前チェック
//[]----------------------------------------------------------------------[]
///	@return			1=補充可
//[]----------------------------------------------------------------------[]
///	@author			
///	@date			Create	: 
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		UsMnt_mnyctl_chk( void )
{
	if ((CN_RDAT.r_dat0c[1] & 0x40) != 0						// コインメックカセット脱
		|| turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT 
		|| ((int)err_cnm_chk() < 0)
	) {
		return 0;
	}
	return 1;
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| 釣銭補充
//|   予蓄部としてSUBチューブ、補助チューブが設定可能。
//|   SUBチューブは、10円、100円、が設定可能。
//|   補助チューブは、10円、50円、100円、が設定可能。
//|   SUBと補助を同じ金種に設定することも可能。
//|   予蓄部設定なしも可能。
//|     現在枚数
//|     turi_dat.coin_dsp[0-3]  循環部
//|     turi_dat.coin_dsp[4]    10円予蓄（金種固定、設定ありの場合のみ値が入る）
//|     turi_dat.coin_dsp[5]    50円予蓄（金種固定、設定ありの場合のみ値が入る）
//|     turi_dat.coin_dsp[6]    100円予蓄（金種固定、設定ありの場合のみ値が入る）
//|     設定枚数
//|     initial_cnt[0-3]  循環部
//|     initial_cnt[4]   10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
//|     initial_cnt[5]   10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnyctl( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyctl()	//釣銭補充
{
	short	msg = -1;
	int		i, j;
	int		subtube = 0;
	ushort	initial_cnt[6];
	ushort	sub_dsp[2];
	ushort	ret, ret1, ret2;
	uchar	adr[] = { 27, 30, 33 }, tube[] = { 0, 0 };
	uchar	bit, idx;

	int		mod = 0;					// 0:釣銭補充、1:釣り合わせ

	if (! UsMnt_mnyctl_chk() ) {		// 実行不可？
		BUZPIPI();
		return(MOD_EXT);
	}

	turikan_proc(MNY_CTL_BEFORE);

	bit = 0x01;
	idx = 0;
	sub_dsp[0] = 0;
	sub_dsp[1] = 0;
	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// 各金種に対する予蓄の割当状況を取得する
		switch( ret ){
		case 1:	// 予蓄１
			tube[idx++] |= bit;
			break;
		case 2:	// 予蓄２
			tube[idx++] |= bit;
			break;
		default:
			break;
		}
		if( idx > 1 ){
			break;
		}
		bit <<= 1;
	}
	idx = 0;
	for( i=0; i<2; i++ ){		// 予蓄１・２の現在枚数に格納
		if( tube[i] & 0x01 ){			// \10
			sub_dsp[idx++] = turi_dat.coin_dsp[4];
		} else if( tube[i] & 0x02 ){	// \50
			sub_dsp[idx++] = turi_dat.coin_dsp[5];
		} else if( tube[i] & 0x04 ){	// \100
			sub_dsp[idx++] = turi_dat.coin_dsp[6];
		}
		if( idx > 1 ){
			break;
		}
	}
	if( tube[0] && tube[1] ){	// 判定の簡素化用
		subtube = 3;
	} else {
		if( tube[0] ){
			subtube = 1;
		} else if( tube[1] ){
			subtube = 2;
		}
	}
	initial_cnt[0] = (short)CPrmSS[S_KAN][3];
	initial_cnt[1] = (short)CPrmSS[S_KAN][9];
	initial_cnt[2] = (short)CPrmSS[S_KAN][15];
	initial_cnt[3] = (short)CPrmSS[S_KAN][21];
	initial_cnt[4] = 0;
	idx = 4;
	for( i=0; i<2; i++ ){		// 予蓄１・２の初期設定枚数を取得
		if( tube[i] & 0x01 ){			// \10
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[0], 3, 1 );
		} else if( tube[i] & 0x02 ){	// \50
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[1], 3, 1 );
		} else if( tube[i] & 0x04 ){	// \100
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[2], 3, 1 );
		}
		if( idx > 5 ){
			break;
		}
	}

	ret = ret2 = 0;
	for( ; ret == 0 ; ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "＜釣銭補充＞　　　　　　　　　"

		if( subtube != 3 ){
			grachr( 1, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	/* // "　　　　　　現在枚数　初期枚数" */
		}else{
			grachr( 0, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	/* // "　　　　　　現在枚数　初期枚数" */
		}
		if(( CPrmSS[S_KAN][1] == 2 )&&( OPECTL.Mnt_lev >= 2 )){
			Fun_Dsp( FUNMSG[71] );					// "　　　　　　　　　釣合せ 終了 " */
		}else{
			Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 " */
		}

		// 循環部を表示
		j = 1;
		if( subtube != 3 ) j = 2;
		for( i = 0; i < 4 ; i++ ){
			grachr( (unsigned short)(i + j),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"金種"
			grachr( (unsigned short)(i + j), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"："
			opedsp( (unsigned short)(i + j), 12, (ushort)turi_dat.coin_dsp[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( (unsigned short)(i + j), 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"枚"
			opedsp( (unsigned short)(i + j), 22, initial_cnt[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( (unsigned short)(i + j), 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"枚"
		}

		// 予蓄部を表示
		if( subtube == 3 ){
			// 予蓄１と予蓄２を両方使用
			for( i=0; i<2; i++ ){
				switch( tube[i] ){
				case 0x01:	// \10
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"金種"
					break;
				case 0x02:	// \50
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"金種"
					break;
				case 0x04:	// \100
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"金種"
					break;
				}
			}
			grachr( 5, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
			opedsp( 5, 12, sub_dsp[0], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 5, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
			opedsp( 5, 22, initial_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 5, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"

			grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
			opedsp( 6, 12, sub_dsp[1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
			opedsp( 6, 22, initial_cnt[5], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
		} else if( subtube ){
			// 片方のみ使用
			if( tube[0] == 0x01 || tube[1] == 0x01 ){	// \10
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"金種"
			}
			if( tube[0] == 0x02 || tube[1] == 0x02 ){	// \50
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"金種"
			}
			if( tube[0] == 0x04 || tube[1] == 0x04 ){	// \100
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"金種"
			}
			grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
			opedsp( 6, 12, sub_dsp[0], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
			opedsp( 6, 22, initial_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
		}

		ret1 = 0;
		for( ; (ret1 == 0)&&(ret == 0) ; ){

			msg = StoF( GetMessage(), 1 );

			switch( KEY_TEN0to9( msg ) ){							/* FunctionKey Enter */

				case TIMEOUT6:
					ret = ret2;
					break;

				case COIN_EVT:
					switch( mod ) {
						case 2: // 終了待ち
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_EN_EVT)) )
							{	// 払出可
								ret = ret2;
								break;
							}
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_IH_EVT)) )
							{
								// 入金あり（保有枚数変化）
								turikan_proc( MNY_INCOIN );
							}
							break;
						case 1: // 釣り合わせ中
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_EN_EVT)) )
							{	// 払出可
								dispclr();
								grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "＜釣銭補充＞　　　　　　　　　"
								grachr( 3,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, MNCSTR[8] );	// "　　＜＜自動釣合わせ中＞＞　　"
								Fun_Dsp( FUNMSG[77] );													// "　　　　　　　　　　　　　　　"
								turikan_proc(MNY_CTL_AUTOSTART);
								break;
							}
							if( ( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_OT_EVT)) ) ||
							  ( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_ER_EVT)) ) )
							{	// 払出完了
								turikan_proc(MNY_CTL_AUTOCOMPLETE);
								ret1 = 1;
								mod = 0;
								grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );	// "　　＜＜自動釣合わせ中＞＞　　" を消す
							}
							break;
						case 0: // 補充中
							turikan_proc( MNY_INCOIN );

							j = 1;
							if( subtube != 3 ) j = 2;
							for( i = 0; i < 4; i++ ){
												/* 保有枚数の増加分を表示ｴﾘｱに加算する */
								if( turi_dat.coin_dsp[i] != turi_dat.dsp_ini[i] )
								{
									opedsp( (ushort)(i + j), 12, (ushort)turi_dat.coin_dsp[i], 3, 0, 0,
																		COLOR_BLACK, LCD_BLINK_OFF );
								}
							}
							break;
						default:
							break;
					}
					break;
				case KEY_TEN_F4:						// 釣り合わせ(いいえ)
					if( CPrmSS[S_KAN][1] != 2 ) break;
					if( OPECTL.Mnt_lev < 2 ) break;
					if( mod != 0 ) break;
					if( cn_errst[0] ){					/* コインメック故障中?(Y)			*/
						BUZPIPI();
						break;
					}
					for( i = 0; i < 4; i++ )
					{
						if( turi_dat.coin_dsp[i] > initial_cnt[i] )
						{
							mod = 1;
							break;
						}
					}
					if( mod == 1 )
					{
						BUZPI();
						turikan_proc(MNY_CTL_AUTO);
					}else{
						BUZPIPI();
					}
					break;

				case ARC_CR_R_EVT:						// ｶｰﾄﾞIN
					// メンテナンスでのｶｰﾄﾞ挿入(OPE_red == 2)の場合は、operedで戻すため、ここではもどさない				//
					// FT・UTでは精算中にメンテ画面に移行することがないので、メンテ画面中にOPE_redが2以外になることが	//
					// ないが念のため、OPE_redが2以外の場合は戻すようにする												//
					if( OPE_red != 2 )
					opr_snd( 13 );						// 券排出
					if( NG == hojuu_card() ){			// 補充ｶｰﾄﾞﾁｪｯｸ
						// 補充ｶｰﾄﾞでない場合
						break;
					}									// 補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ終了
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:
				case KEY_TEN_F5:						// F5:Exit
					if( mod != 0 ) break;
					BUZPI();

					cn_stat( 2, 0 );				// 入金不可
					Lagtim( OPETCBNO, 6, 250 ); 	// 5sec
					mod = 2;						// 釣銭補充を終了
												// 払出可能状態（CREM OFF）になるまで待つ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//					ret2 = MOD_CHG;
//					if( msg == KEY_TEN_F5 )
//					{
//						ret2 = MOD_EXT;
//					}	
					if( msg == KEY_TEN_F5 )
					{
						ret2 = MOD_EXT;
					}else if(  msg == LCD_DISCONNECT ){
						ret2 = MOD_CUT;
					}else{
						ret2 = MOD_CHG;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					break;

				default:
					break;
			}
		}
	}
	Lagcan( OPETCBNO, 6 );
	turikan_proc( MNY_CTL_AFTER );
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| 釣銭補充（金銭管理なしの場合）
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnyctl2( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyctl2()	//釣銭補充
{
	short	msg = -1;
	int		i, mod;
	ushort	ret, ret2;
	const uchar	*ARINASHI;

	if (! UsMnt_mnyctl_chk() ) {		// 実行不可？
		BUZPIPI();
		return(MOD_EXT);
	}
	turikan_proc(MNY_CTL_BEFORE);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "＜釣銭補充＞　　　　　　　　　"
	grachr( 1, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[9]+14 );		// "　　　　　　　釣銭状態　　　　"
	Fun_Dsp( FUNMSG[8] );													// "　　　　　　　　　　　　 終了 "

	// 循環部を表示
	for( i = 0; i < 4 ; i++ ){
		grachr( (unsigned short)(i + 2),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"金種"
		grachr( (unsigned short)(i + 2), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"："
		if( CN_RDAT.r_dat09[i] != 0 ){
			ARINASHI = DAT6_0[9];
		}else{
			ARINASHI = DAT6_0[10];
		}
		grachr( (unsigned short)(i + 2), 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, ARINASHI );		// あり／なし
	}

	mod = 0;
	ret = ret2 = 0;
	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){							/* FunctionKey Enter */

			case TIMEOUT6:
				ret = ret2;
				break;

			case COIN_EVT:
				switch( mod ) {

					case 2: // 終了待ち
						if( OPECTL.CN_QSIG == (uchar)(COIN_EN_EVT & 0x00ff ) )
						{	// 払出可
							ret = ret2;
						}
						break;

					case 0: // 補充中

						turikan_proc( MNY_INCOIN );

						for( i = 0; i < 4; i++ ){
											/* 保有枚数の増加分を表示ｴﾘｱに加算する */
							if( CN_RDAT.r_dat09[i] != 0 ){
								ARINASHI = DAT6_0[9];
							}else{
								ARINASHI = DAT6_0[10];
							}
							grachr( (unsigned short)(i + 2), 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, ARINASHI );		// あり／なし
						}
						break;

					default:
						break;
				}
				break;

			case ARC_CR_R_EVT:					// ｶｰﾄﾞIN
				// メンテナンスでのｶｰﾄﾞ挿入(OPE_red == 2)の場合は、operedで戻すため、ここではもどさない				//
				// FT・UTでは精算中にメンテ画面に移行することがないので、メンテ画面中にOPE_redが2以外になることが	//
				// ないが念のため、OPE_redが2以外の場合は戻すようにする												//
				if( OPE_red != 2 )
					opr_snd( 13 );				// 券排出
				if( NG == hojuu_card() ){		// 補充ｶｰﾄﾞﾁｪｯｸ
					// 補充ｶｰﾄﾞでない場合
					break;
				}								// 補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ終了
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
			case KEY_MODECHG:
			case KEY_TEN_F5:					// F5:Exit
				if( mod != 0 ) break;
				BUZPI();

				cn_stat( 2, 0 );				// 入金不可
				Lagtim( OPETCBNO, 6, 250 ); 	// 5sec
				mod = 2;						// 釣銭補充を終了
											// 払出可能状態（CREM OFF）になるまで待つ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				ret2 = MOD_CHG;
//				if( msg == KEY_TEN_F5 )
//				{
//					ret2 = MOD_EXT;
//				}
//				
//				break;
				if( msg == KEY_TEN_F5 )
				{
					ret2 = MOD_EXT;
				}else if( msg == LCD_DISCONNECT ){
					ret2 = MOD_CUT;
				}else{
					ret2 = MOD_CHG;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 6 );
	turikan_proc( MNY_CTL_AFTER );
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| 釣銭枚数変更
//|   予蓄部としてSUBチューブ、補助チューブが設定可能。
//|   SUBチューブは、10円、100円、が設定可能。
//|   補助チューブは、10円、50円、100円、が設定可能。
//|   SUBと補助を同じ金種に設定することも可能。
//|   予蓄部設定なしも可能。
//|     現在枚数
//|     current_cnt[0-3]  循環部
//|     current_cnt[4]    10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
//|     current_cnt[5]    10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
//|     設定枚数
//|     current_cnt[6-9]  循環部
//|     current_cnt[10]   10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
//|     current_cnt[11]   10円または50円または100円予蓄（金種可変、設定ありの場合のみ値が入る）
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnychg( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnychg()
{
	short	msg = -1;
	int		i, j;
	int		genmai_change = 0;
	int		subtube = 0;
	short	inpt = -1;
	uchar	pos[2];
	ushort	current_cnt[12], usWork, ret;
	int		Mnt_lev = OPECTL.Mnt_lev;
	uchar	adr[] = { 27, 30, 33 }, tube[] = { 0, 0 };
	uchar	bit, idx, wkc;


	if (turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT) {		// 釣銭管理合計印字中？
		BUZPIPI();
		return MOD_EXT;
	}
	pos[0] = pos[1] = 0;

	turikan_proc( MNY_CHG_BEFORE );

	for( i = 0; i < 4 ; i++ ){
		current_cnt[i] = turi_dat.coin_dsp[i];
	}
	bit = 0x01;
	idx = 0;
	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// 各金種に対する予蓄の割当状況を取得する
		switch( ret ){
		case 1:	// 予蓄１
			tube[idx++] |= bit;
			break;
		case 2:	// 予蓄２
			tube[idx++] |= bit;
			break;
		default:
			break;
		}
		if( idx > 1 ){
			break;
		}
		bit <<= 1;
	}
	idx = 4;
	for( i=0; i<2; i++ ){		// 予蓄１・２の現在枚数を取得
		if( tube[i] & 0x01 ){			// \10
			current_cnt[idx++] = turi_dat.coin_dsp[4];
		} else if( tube[i] & 0x02 ){	// \50
			current_cnt[idx++] = turi_dat.coin_dsp[5];
		} else if( tube[i] & 0x04 ){	// \100
			current_cnt[idx++] = turi_dat.coin_dsp[6];
		}
		if( idx > 5 ){
			break;
		}
	}
	if( tube[0] && tube[1] ){	// 判定の簡素化用
		subtube = 3;
	} else {
		if( tube[0] ){
			subtube = 1;
		} else if( tube[1] ){
			subtube = 2;
		}
	}

	current_cnt[6] = (short)CPrmSS[S_KAN][3];
	current_cnt[7] = (short)CPrmSS[S_KAN][9];
	current_cnt[8] = (short)CPrmSS[S_KAN][15];
	current_cnt[9] = (short)CPrmSS[S_KAN][21];
	idx = 10;
	for( i=0; i<2; i++ ){		// 予蓄１・２の初期設定枚数を取得
		if( tube[i] & 0x01 ){			// \10
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[0], 3, 1 );
		} else if( tube[i] & 0x02 ){	// \50
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[1], 3, 1 );
		} else if( tube[i] & 0x04 ){	// \100
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[2], 3, 1 );
		}
		if( idx > 11 ){
			break;
		}
	}

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[7] );			// "＜釣銭枚数＞　　　　　　　   " */
	if( subtube != 3 ){
		grachr( 1, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	// "　　　　　　現在枚数　初期枚数" */
	}else{
		grachr( 0, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	// "　　　　　　現在枚数　初期枚数" */
	}

	usWork = 0;
	if( Mnt_lev > 1 ){
		Fun_Dsp( FUNMSG[75] );					// "　▲　　▼　　→　 書込  終了 " */
		usWork = 1; // 最初だけ反転させる
	}else{
		Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 " */
	}

	// 循環部を表示
	j = 1;
	if( subtube != 3 ) j = 2;
	for( i = 0; i < 4 ; i++ ){
		grachr( (unsigned short)(j + i),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"金種"
		grachr( (unsigned short)(j + i), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"："
		opedsp( (unsigned short)(j + i), 12, current_cnt[i], 3, 0, usWork, COLOR_BLACK, LCD_BLINK_OFF );
		usWork = 0;
		grachr( (unsigned short)(j + i), 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"枚"
		opedsp( (unsigned short)(j + i), 22, current_cnt[6+i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( (unsigned short)(j + i), 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"枚"
	}

	// 予蓄部を表示
	if( subtube == 3 ){
		// 予蓄１と予蓄２を両方使用
		for( i=0; i<2; i++ ){
			switch( tube[i] ){
			case 0x01:	// \10
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"金種"
				break;
			case 0x02:	// \50
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );//"金種"
				break;
			case 0x04:	// \100
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"金種"
				break;
			}
		}
		grachr( 5, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
		opedsp( 5, 12, current_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 5, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
		opedsp( 5, 22, current_cnt[10], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 5, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"

		grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
		opedsp( 6, 12, current_cnt[5], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
		opedsp( 6, 22, current_cnt[11], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
	} else if( subtube ){
		// 片方のみ使用
		if( tube[0] == 0x01 || tube[1] == 0x01 ){	// \10
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"金種"
		}
		if( tube[0] == 0x02 || tube[1] == 0x02 ){	// \50
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"金種"
		}
		if( tube[0] == 0x04 || tube[1] == 0x04 ){	// \100
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"金種"
		}
		grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"："
		opedsp( 6, 12, current_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
		opedsp( 6, 22, current_cnt[10], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"枚"
	}

	ret = 0;
	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						/* 終了 */
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN_F4:						/* 書込 */
				if( Mnt_lev < 2 ) break;

				if( inpt != -1 ){
					if( mnyctl_nuchk( pos[0], inpt, tube[0], tube[1] ) != 0 ){
						BUZPIPI();
						inpt = -1;
						break;
					}
				}

				BUZPI();
				pos[1] = pos[0];
				if( inpt != -1 )
				{
					current_cnt[pos[0]] = inpt;

					if( pos[0] > 5 )
					{
						if( pos[0] < 10 )
						{	// 初期枚数書込
							usWork = (ushort)(pos[0]-6);
							CPrmSS[S_KAN][3+usWork*6] = inpt;
						}else{
							if( pos[0] == 10 ){
								wkc = tube[0];
							}else{
								wkc = tube[1];
							}
							switch( wkc ){
							case 0x01:	// \10
								CPrmSS[S_KAN][27] = inpt;
								break;
							case 0x02:	// \50
								CPrmSS[S_KAN][30] = inpt;
								break;
							case 0x04:	// \100
								CPrmSS[S_KAN][33] = inpt;
								break;
							}
						}
						OpelogNo = OPLOG_SHOKIMAISUHENKO;		// 操作履歴登録
						SetChange = 1;
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}else{
						genmai_change = -1;
						switch( pos[0] )
						{
							case 0: turikan_proc(MNY_CHG_10YEN); break;
							case 1: turikan_proc(MNY_CHG_50YEN); break;
							case 2: turikan_proc(MNY_CHG_100YEN); break;
							case 3: turikan_proc(MNY_CHG_500YEN); break;
							case 4:
							case 5:
								switch( tube[(pos[0]-4)] ){
								case 0x01:	// \10
									turikan_proc(MNY_CHG_10SUBYEN);
									break;
								case 0x02:	// \50
									turikan_proc(MNY_CHG_50SUBYEN);
									break;
								case 0x04:	// \100
									turikan_proc(MNY_CHG_100SUBYEN);
									break;
								}
								break;
						}
					}
				}
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F2, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F3:						/* → or ← */
				if( Mnt_lev < 2 ) break;

				BUZPI();
				pos[1] = pos[0];

				if( pos[0] < 6 )
					Fun_Dsp( FUNMSG[76] );						/* // "　▲　　▼　　←　 書込  終了 " */
				else
					Fun_Dsp( FUNMSG[75] );						/* // "　▲　　▼　　→　 書込  終了 " */

				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F3,(ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_CL:						//Clear
				if( Mnt_lev < 2 ) break;
				BUZPI();
				inpt = -1;
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F1:						/* ▲ */
				if( Mnt_lev < 2 ) break;
				BUZPI();
				pos[1] = pos[0];
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F1, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F2:						/* ▼ */
				if( Mnt_lev < 2 ) break;
				BUZPI();
				pos[1] = pos[0];
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F2, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN:
				if( Mnt_lev < 2 ) break;
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 100 ) * 10 + (short)(msg - KEY_TEN0);
				if( inpt != -1 ){						//入力あり
					mnyctl_dsp((ushort)(pos[0]), (ushort)inpt, 1, (ushort)subtube );
				}
				break;

			default:
				break;
		}
	}
	if( genmai_change != 0 )
	{	// 現在枚数を変更した
		for( i = 0; i < 4; i++ ){
			turi_dat.coin_dsp[i] = current_cnt[i];
		}
		for( i=0; i<2; i++ ){
			if( i == 0 ){
				usWork = current_cnt[4];
			} else {
				usWork = current_cnt[5];
			}
			switch( tube[i] ){
			case 0x01:	// \10
				turi_dat.coin_dsp[4] = usWork;
				break;
			case 0x02:	// \50
				turi_dat.coin_dsp[5] = usWork;
				break;
			case 0x04:	// \100
				turi_dat.coin_dsp[6] = usWork;
				break;
			}
		}
	}
	turikan_proc( MNY_CHG_AFTER );	// 釣銭管理ﾌﾟﾘﾝﾄ

	Lagtim( OPETCBNO, 6, 150 );	// 3sec
	for( ; ; )
	{
		msg = GetMessage();
		if(( msg == COIN_EVT)||(msg == TIMEOUT6) )
		{
			break;
		}
	}
	Lagcan( OPETCBNO, 6 );
	return( ret );
}

// 入力値ﾁｪｯｸ
// return = 0:OK -1:NG
// ---------------------------------------------------------------------------------
// NOTE: FT4000既存の枚数判定処理はFT4800の処理からの流用だが、
//       FT4000で使用するコインメックは有効受付枚数等がFT4800のもとのは異なる為、
//       GT7700での処理を参考に修正を行った。
// ---------------------------------------------------------------------------------
static int mnyctl_nuchk( uchar loc, short inpt, uchar sub1, uchar sub2 )
{
	int nResult = 0; // OK
	short nInpChk;
	short nChknu10sub;
	short nChknu100sub;
	short nChknu50sub;
	uchar wkc;
	nInpChk = 0;

	nChknu10sub = 0;
	nChknu100sub = 0;
	nChknu50sub = 0;
	wkc = 0;
	switch( loc ){
	case 4:
	case 5:
		if( loc == 4 ){
			wkc = sub1;
		} else {
			wkc = sub2;
		}
		switch( wkc ){
		case 0x01:		// \10
			nChknu10sub = TUB_CURMAX10SUB + TUB_CURMAX10YO;
			break;
		case 0x02:		// \50
			nChknu50sub = TUB_CURMAX50YO;
			break;
		case 0x04:		// \100
			nChknu100sub = TUB_CURMAX100SUB + TUB_CURMAX100YO;
			break;
		}
		break;
	case 10:
	case 11:
		if( loc == 10 ){
			wkc = sub1;
		} else {
			wkc = sub2;
		}
		switch( wkc ){
		case 0x01:		// \10
			nChknu10sub = TUB_MAX10SUB + TUB_MAX10YO;
			break;
		case 0x02:		// \50
			nChknu50sub = TUB_MAX50YO;
			break;
		case 0x04:		// \100
			nChknu100sub = TUB_MAX100SUB + TUB_MAX100YO;
			break;
		}
		break;
	}

	switch( loc ){
		case 0:
			nInpChk = TUB_CURMAX10;
			break;
		case 1:
			nInpChk = TUB_CURMAX50;
			break;
		case 2:
			nInpChk = TUB_CURMAX100;
			break;
		case 3:
			nInpChk = TUB_CURMAX500;
			break;
		case 6:
			nInpChk = TUB_MAX10;
			break;
		case 7:
			nInpChk = TUB_MAX50;
			break;
		case 8:
			nInpChk = TUB_MAX100;
			break;
		case 9:
			nInpChk = TUB_MAX500;
			break;
		case 4:		// 現在枚数：予蓄１
		case 10:	// 初期枚数：予蓄１
		case 5:		// 現在枚数：予蓄２
		case 11:	// 初期枚数：予蓄２
			if( loc == 4 || loc == 10 ){
				wkc = sub1;
			} else {	// loc == 5 || loc == 11
				wkc = sub2;
			}
			switch( wkc ){
			case 0x01:		// \10
				nInpChk = nChknu10sub;
				break;
			case 0x02:		// \50
				nInpChk = nChknu50sub;
				break;
			case 0x04:		// \100
				nInpChk = nChknu100sub;
				break;
			}
			break;
	}

	if( inpt > nInpChk ){
		nResult = -1;
	}
	return( nResult );
}

// 枚数表示を行う
static const ushort dsp_loc[][2] = {
	{ 1, 12 },	// 10円
	{ 2, 12 },	// 50円
	{ 3, 12 },	// 100円
	{ 4, 12 },	// 500円
	{ 5, 12 },	// SUB 10円 or 100円
	{ 6, 12 },	// SUB 100円

	{ 1, 22 },	// 10円
	{ 2, 22 },	// 50円
	{ 3, 22 },	// 100円
	{ 4, 22 },	// 500円
	{ 5, 22 },	// SUB 10円 or 100円
	{ 6, 22 } };// SUB 100円

static void mnyctl_dsp( ushort loc, ushort dat, ushort rev, ushort sub )
{
	if( sub == 3 ){
		opedsp( dsp_loc[loc][0], dsp_loc[loc][1], dat, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );
	}else{
		opedsp( (ushort)(dsp_loc[loc][0]+1), dsp_loc[loc][1], dat, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );
	}
}

// ｶｰｿﾙ移動制御
static uchar mnyctl_cur( uchar pos, ushort inkey, ushort subtube )
{
	uchar max1, max2;

	if( subtube == 3 )
	{	// 10円、100円ともにあり
		max1 = 5;
		max2 = 11;
	}else if( subtube ){
		max1 = 4;
		max2 = 10;
	}else{
		max1 = 3;
		max2 = 9;
	}
	switch( inkey )
	{
		case KEY_TEN_F1: //▲
			if( pos <= max1 )
			{
				if( pos == 0 ){
					pos = max1;
				}else{
					pos--;
				}
			}else{
				if( pos == 6 ){
					pos = max2;
				}else{
					pos--;
				}
			}
			break;
		case KEY_TEN_F2: // ▼
			if( pos <= max1 )
			{
				if( pos == max1 ){
					pos = 0;
				}else{
					pos++;
				}
			}else{
				if( pos >= max2 ){
					pos = 6;
				}else{
					pos++;
				}
			}
			break;
		case KEY_TEN_F3: //→　←
			if( pos <= max1 ){
				pos = 6;
			}else{
				pos = 0;
			}
			break;
	}
	return pos;
}

unsigned short	UsMnt_invtry()	//インベントリ
{

	short			msg = -1;
	short			inpt = -1;
	unsigned short	cindsp, inv_excute, ret;
	int				dsp;
	int				mod;
	char			mec_wait = 0;

	if (turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT) {		// 釣銭管理合計印字中？
		BUZPIPI();
		return MOD_EXT;
	}
	turikan_proc( MNY_COIN_INVSTART );
	inv_excute = 0;
	cindsp = 0;
	mod = 0;
INVTOP:

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[2] );			/* // [01]	"＜インベントリ＞　　　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[3] );			/* // [01]	"　払出金額：　　　　　　　　　" */
	grachr( 2, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3] );			//"円"
	Fun_Dsp( FUNMSG[70] );						/* // [70]	"　　　　　　　　　 払出  終了 " */


	dsp = 1;
	ret = 0;
	for( ; ret == 0; ){

		if( dsp ){
			cindsp = (unsigned short)( inpt == -1 ? 0 : inpt );
			opedsp( 2, 12, cindsp, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// input
			opedsp( 2, 16, 0, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// 0
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg )){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:						/* Exit */
				if( 1 == mod )
					break;
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN:
				if( 1 == mod )
					break;
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 10 ) * 10 + (short)(msg - KEY_TEN0);
				dsp = 1;
				break;

			case KEY_TEN_CL:						// Clear
				if( 1 == mod )
					break;
				BUZPI();
				inpt = -1;
				dsp = 1;
				break;

			case KEY_TEN_F3:						/* はい */
				if( mod == 1 ){
					mod = 2;
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "　　　　　　　　　　　　　　　"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "　　　　　　　　　　　　　　　"
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "　　　　　　　　　　　　　　　"
					Fun_Dsp( FUNMSG[70] );													// "　　　　　　　　　 払出  終了 "
				}else{
					break;
				}
			case COIN_EVT:					// コインメックイベント
				if( msg == COIN_EVT ){
					if( OPECTL.CN_QSIG == 0x35 ){   	// インベントリ開始時には保有枚数の送信をするので、保有枚数送信完了も条件に追加
						mec_wait = 0x02;
						break;
					}else if( OPECTL.CN_QSIG == 0x07 ){	// ｺｲﾝﾒｯｸから「払出し可状態」受信
						Lagcan( OPETCBNO, 6 );
						mec_wait |= 0x02;
						if( !(mec_wait & 0x01) ){
							break;
						}
					}else{
						break;
					}
				}
			case KEY_TEN_F4:						/* Start(いいえ) */
				if( mod == 1 )						// ﾄﾞｱ開ﾒｯｾｰｼﾞ中
				{
					BUZPI();
					mod = 0;
					goto INVTOP;
				}
				if(( inpt == -1 )||( inpt == 0 ))
				{
					BUZPIPI();
					break;
				}
				BUZPI();
				if( mec_wait <= 1 ){
					mec_wait = 1;
					Lagtim( OPETCBNO, 6, 50*5 );	// 払出可能状態遷移待ち監視 5s
					break;
				}

				mod = 0;
				inpt = -1;
				inv_excute = 123;
				opedsp( 2, 12, cindsp, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// input
				//払出開始
				safecl( 0 ); 						/* 金庫入金前処理 */
				if( refund( (long)( cindsp * 10 ) ) != 0 ){
					//err
					BUZPIPI();
					inv_excute = 0;
					dsp = 1;
					break;
				}else{
					//ok
					;
				}
				Fun_Dsp( FUNMSG[8] );			/* // [08]	"　　　　　　　　　　　　 終了 " */
				Lagtim( OPETCBNO, 6, 50*60*3 );	// 払出監視 3min

				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case LCD_DISCONNECT:
							ret = MOD_CUT;
							break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case KEY_MODECHG:
							BUZPI();
							ret = MOD_CHG;
							break;
						case TIMEOUT6:
							Lagcan( OPETCBNO, 6 );
							cn_stat( 2, 0 );				// CREM OFF
							mec_wait = 0;
							goto INVTOP;
							break;
						case COIN_EVT:
							if( ( OPECTL.CN_QSIG == (uchar)(COIN_OT_EVT & 0x00ff ) ) ||
								( OPECTL.CN_QSIG == (uchar)(COIN_ER_EVT & 0x00ff ) ) )
							{	// 払出完了
								Lagcan( OPETCBNO, 6 );
								cn_stat( 2, 0 );			// CREM OFF
								mec_wait = 0;
								goto INVTOP;
							}
							break;
						default:
							break;
					}
				}
				break;
			case TIMEOUT6:
				Lagcan( OPETCBNO, 6 );
				BUZPIPI();
				mec_wait = 0;
				goto INVTOP;
				break;

			default:
				break;
		}
	}
	if( inv_excute )
	{
		xPause( 40L ); 	// 400ms wait
						// 金庫計算が起動してしまう為、払出ﾃﾞｰﾀが完全に受信するまで待つ。
		turikan_proc( MNY_COIN_INVCOMPLETE );
	}
	turi_dat.turi_in = 0;
	return( ret );
}

unsigned short	UsMnt_mnybox()	//金庫確認
{
	short			msg = -1;
	unsigned char	i;
	unsigned short	cincnt[COINCHK_CNT+1];
	unsigned short	ret = 0;
	unsigned char	pritimer = OFF;
	T_FrmCoSyuukei FrmCoSyuukei;
	T_FrmSiSyuukei FrmSiSyuukei;
	date_time_rec	*p_sky_date;	// 集計日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
	int				inji_end = 0;
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)

	Ope_KeyRepeatEnable(0);			// キーリピート無効
	memset( cincnt, 0, COINCHK_CNT );
	for( i = 0; i < 4; i++ )
	{
		cincnt[i] = (unsigned short)(SFV_DAT.safe_dt[i]);
	}
	cincnt[4] = (unsigned short)(SFV_DAT.nt_safe_dt);

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[4] );				// [01]	"＜金庫確認＞　　　　　　　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[5] );				// [05]	"　　　　　　現在枚数　　　　　"
	Fun_Dsp( FUNMSG[84] );														// [84]	"紙小計コ小計紙前計コ前計 終了 "

	for( i = 0; i < (COINCHK_CNT+1) ; i++ ){
		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );	//"金種"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"："
		opedsp( (unsigned short)(i + 2), 14, cincnt[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	//金庫枚数
		grachr( (unsigned short)(i + 2), 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"枚"
	}


	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
		if ((msg & INNJI_ENDMASK) != 0) {
			msg &= ~INNJI_ENDMASK;
			inji_end = 1; // 印字終了
		}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)

		switch( KEY_TEN0to9( msg )){				// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				// ドアノブの状態にかかわらずトグル動作してしまうので、
				// ドアノブ閉かどうかのチェックを実施
				if (CP_MODECHG && pritimer == ON) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				BUZPI();
				ret = MOD_CHG;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				break;

			case KEY_TEN_F2:						// Ｆ２（コ小計）キー
				if( pritimer == OFF ){
					if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					kinko_syu( 2, 1 );				// ｺｲﾝ金庫小計
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_COIN_SHOKEI, 0, 0 );	// 操作履歴登録

					Lagtim( OPETCBNO, 6, 3*50 );	// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ(３秒)起動（連続して印字要求をさせないようにｶﾞｰﾄﾞする）
					pritimer = ON;					// ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
				}
				break;

			case KEY_TEN_F1:						// Ｆ１（紙小計）キー
				if( pritimer == OFF ){
					if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					kinko_syu( 3, 1 );				// 紙幣金庫小計
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_NOTE_SHOKEI, 0, 0 );	// 操作履歴登録

					Lagtim( OPETCBNO, 6, 3*50 );	// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ(３秒)起動（連続して印字要求をさせないようにｶﾞｰﾄﾞする）
					pritimer = ON;					// ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
				}
				break;
			case KEY_TEN_F3:						// Ｆ３（紙前計）キー
				if( pritimer == OFF ){
					p_sky_date = &nobk_syu.NowTime;		// 前回紙幣金庫合計：今回集計日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
					if( (p_sky_date->Year == 0) &&
					    (p_sky_date->Mon  == 0) &&
					    (p_sky_date->Day  == 0) &&
					    (p_sky_date->Hour == 0) &&
					    (p_sky_date->Min  == 0) ){

						// 前回集計ﾃﾞｰﾀがない場合
						BUZPIPI();
						break;
					}
					if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					memcpy( &FrmSiSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// 現在時刻ｾｯﾄ
					FrmSiSyuukei.prn_kind = R_PRI;
					FrmSiSyuukei.prn_data = &nobk_syu;
					queset( PRNTCBNO, PREQ_SIKINKO_Z, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// 紙幣金庫合計印字
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_NOTE_ZENGOUKEI, 0, 0 );
					Lagtim( OPETCBNO, 6, 3*50 );	// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ(３秒)起動（連続して印字要求をさせないようにｶﾞｰﾄﾞする）
					pritimer = ON;					// ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
				}
				break;
			case KEY_TEN_F4:						// Ｆ４（コ前計）キー
				if( pritimer == OFF ){
					p_sky_date = &cobk_syu.NowTime;		// 前回ｺｲﾝ金庫合計：今回集計日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
					if( (p_sky_date->Year == 0) &&
					    (p_sky_date->Mon  == 0) &&
					    (p_sky_date->Day  == 0) &&
					    (p_sky_date->Hour == 0) &&
					    (p_sky_date->Min  == 0) ){

						// 前回集計ﾃﾞｰﾀがない場合
						BUZPIPI();
						break;
					}
					if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					memcpy( &FrmCoSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// 現在時刻ｾｯﾄ
					FrmCoSyuukei.prn_kind = R_PRI;
					FrmCoSyuukei.prn_data = &cobk_syu;
					queset( PRNTCBNO, PREQ_COKINKO_Z, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ｺｲﾝ金庫合計印字
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_COIN_ZENGOUKEI, 0, 0 );
					Lagtim( OPETCBNO, 6, 3*50 );	// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ(３秒)起動（連続して印字要求をさせないようにｶﾞｰﾄﾞする）
					pritimer = ON;					// ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
				}
				break;

			case KEY_TEN_F5:						// Ｆ５（終了）キー
				BUZPI();
				ret = MOD_EXT;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;

			case TIMEOUT6:							// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰﾀｲﾑｱｳﾄ
				pritimer = OFF;						// ﾀｲﾏｰﾌﾗｸﾞﾘｾｯﾄ
				break;

			default:
				break;
		}
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
		// 印字終了後、ドアノブ閉状態であればメンテナンス終了
		if (inji_end == 1 && CP_MODECHG) {
			ret = MOD_CHG;
			break;
		}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
	}
	if( pritimer == ON ){							// 印字要求ｶﾞｰﾄﾞﾀｲﾏｰ起動中？
		Lagcan( OPETCBNO, 6 );						// ﾀｲﾏｰ解除
	}
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| エラー・アラーム確認																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_ErrAlm( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto																	   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// ALM_CHK[]に対応する文字列のﾃｰﾌﾞﾙ番号を定義する
static const unsigned short	AlmTableNo[ALM_MOD_MAX][ALM_NUM_MAX] = {
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 
{	// Main 00
	0, 2, 0, 0, 0,22,23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
{	// Sub 01
	0,97, 4, 0, 0, 0,13,15, 0, 0, 0, 0, 0, 0, 0, 5, 6, 7, 8, 0, 
	0, 9,10,11,12, 0, 0, 0,96, 3, 0,16,14, 0, 0, 0, 0, 0, 0, 0, 
// MH810104 GG119201(S) 電子ジャーナル対応 #5949 エラーコードと精算機のエラー表示が一致しない
//// MH810104 GG119201(S) 電子ジャーナル対応
////	0, 0, 0, 0, 0, 0,113,114,115,116, 0, 0, 0,75, 0, 0, 0, 0, 0, 0, 
//	0, 0, 0, 0, 0,326,113,114,115,116, 0, 0, 0,75, 0,324,325, 0, 0, 0, 
//// MH810104 GG119201(E) 電子ジャーナル対応
	0, 0, 0, 0, 0,325,113,114,115,116, 0, 0, 0,75, 0,323,324, 0, 0, 0, 
// MH810104 GG119201(E) 電子ジャーナル対応 #5949 エラーコードと精算機のエラー表示が一致しない
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
// MH321800(S) D.Inaba ICクレジット対応
//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
// MH810105 GG119202(S) リーダから直取中の状態データ受信した際の新規アラームを設ける
//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 298, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 327, 0, 0, 0, 0, 0, 0, 0, 0, 298, 0, 0, 0, 0, 0, 0, 
// MH810105 GG119202(E) リーダから直取中の状態データ受信した際の新規アラームを設ける
// MH321800(E) D.Inaba ICクレジット対応
},
{	// 02
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
{	// 05 認証機関連
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
};
// ERR_CHK[]に対応する文字列のﾃｰﾌﾞﾙ番号を定義する
static const unsigned short	ErrTableNo[ERR_MOD_MAX][ERR_NUM_MAX] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 
{	// Main 00
	 0,17,0,0, 0,18,19,24,25, 0,20,21, 0,180,181, 0,26,72,73, 0,
	 0, 0, 0, 0, 0,68,69,70,71, 0, 0,74, 0, 0, 0,131, 0, 0, 0, 0,
	 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,95, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 51, 0, 0, 0, 65, 66, 0, 0, 0, 0, 0, 0, 0,200,201,202,203,
},
{	// Reader 01
	 0,29, 182, 0, 0, 0, 0, 0, 0, 0,30,31, 0,32,33,34,35,36,37,38,
	39, 0, 0, 0,40,41, 0, 0, 0, 0, 0, 0,28, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Printer 02
	 0,42, 0,43,45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	46, 0,47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Coin 03
	 0,48, 0, 0, 0,49,50,52, 0, 0, 0, 0,53,54,55,56, 0, 0,57,58,
	59,60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Note 04
	 0,61, 0, 0, 0,62,63,64,67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// IF-ROCK 05
	 0,76,77,78,79,80,81,82,83,84,85,86,87, 0, 0, 0, 0, 0, 0, 0,
	// 05-20は子機通信異常であるが、ここでは表示番号はｾｯﾄしない。ｴﾗｰの数とは別に数えるため。
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Mifare 06
	 0,98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// NT-NET Child 07
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 227, 0, 0, 0, 0, 228, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// NT-NET IBK 08
	 0,99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// ﾗﾍﾞﾙﾌﾟﾘﾝﾀ	64
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,104,  0,  0,  0,  0,  0,  0,  0,  0,105,106,107,108,109,110,111,  0,  0,  0,
	112,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},
{	// ﾌﾗｯﾌﾟ 10
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,117,118,119,120,121,122,123,124,125,126,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,129,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
//{	// CCT Credit 54
//	 0,144,145,146,147,148,149, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
//},
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
{	// LAN  Mod=72
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,165,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00～19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 20～39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,253,	/* 40～59 */
	242,  0,  0,243,244,245,246,247,248,249,250,251,  0,  0,  0,  0,  0,  0,  0,  0,	/* 60～79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80～99 */
},
{	// Dopa  Mod=73
	0, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
{	// Suica	69
	 0,150, 151, 0,152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0,239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//{	// Edy	62
//	//0  1   2   3   4  5  6  7  8  9  10  11  12 13 14 15 16 17 18 19
//	 0,153,154,155,156, 0, 0, 0, 0, 0,157,  0,158, 0, 0, 0, 0, 0, 0, 0,
//   159,160,  0,161,  0, 0, 0, 0, 0, 0,162,163,164, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//},
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
{	// remote 55
	 0,167, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Dopa  Mod=77
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,184,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00～19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,254,255,266,  0,  0,  0,	/* 20～39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,253,	/* 40～59 */
	242,  0,  0,243,244,245,246,247,248,249,250,251,  0,  0,185,186,187,  0,  0,  0,	/* 60～79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80～99 */
},
{	// NT-NET-DOPA MAF	65
	 0, 0, 225,226, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0,204, 0,205, 0,206, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,226, /* ｺｰﾄﾞ：60～ 79 */
	 207,208, 0, 0, 0, 0, 0, 0, 0, 0,224, 0, 0, 0,209,210,211,0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// NT-NET-DOPA 	67
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// Cappi Credit 74
	 0,188, 0, 0,189,190, 0,191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// iDC 75
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// CRR 15
	  0,212,213,214,215,216,217,218,219,220,221,222,223,  0,  0,  0,  0,  0,  0,  0,/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ｺｰﾄﾞ：80～ 99 */
},
{	// CARD 76
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// 簡易フラップ／ロック装置 Mod=16
	 0,76,77,78,79,80,81,82,83,84,85,86,87,131,132,133,134,135,136,137, /* ｺｰﾄﾞ： 0～ 19 */
   138,139,140,141,142,143,212,213,214,215,216,217,  0,  0,  0,  0,  0,  0,  0,  0, /* ｺｰﾄﾞ：20～ 39 */
	 0,  0,  0,218,219,220,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 234, 0, 0, 0, 0, 					/* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 					/* ｺｰﾄﾞ：80～ 99 */
},
{	// CAN 86
	 0,240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},
{	// SODIAC 25
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：20～ 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：40～ 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：60～ 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ｺｰﾄﾞ：80～ 99 */
},	
{	// I2C_Error
	//	ERR_I2C_BFULL_SEND_RP	10	I2Cﾚｼｰﾄ送信ﾊﾞｯﾌｧﾌﾙ		235
	//	ERR_I2C_BFULL_RECV_RP	11	I2Cﾚｼｰﾄ受信ﾊﾞｯﾌｧﾌﾙ		236
	//	ERR_I2C_BFULL_SEND_JP	12	I2Cｼﾞｬｰﾅﾙ送信ﾊﾞｯﾌｧﾌﾙ	237
	//	ERR_I2C_BFULL_RECV_JP	13	I2Cｼﾞｬｰﾅﾙ受信ﾊﾞｯﾌｧﾌﾙ	238
//	1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 235, 236, 237, 238, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},

// MH321800(S) D.Inaba ICクレジット対応 (決済リーダエラー表示文字対応)
{	// 決済リーダ	32
//  0     1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19 
// MH810103 GG119202(S) 異常データフォーマット変更
//	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ： 0～ 19 */
// MH810103 GG119202(S) 決済リーダからの再起動要求を記録する
//	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274, 288, 300,   0,   0,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ： 0～ 19 */
	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274, 288, 315, 319,   0,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ： 0～ 19 */
// MH810103 GG119202(E) 決済リーダからの再起動要求を記録する
// MH810103 GG119202(E) 異常データフォーマット変更
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
//   275,   0, 276, 277, 278, 279, 280, 281,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ：20～ 39 */
   275,   0, 276, 277, 278, 279, 280, 281,   0,   0,   0,   0,   0,   0, 318,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ：20～ 39 */
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
//     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ｺｰﾄﾞ：40～ 59 */
// MH810103 GG119202(S) みなし決済扱い時の動作
//     0,   0,   0,   0, 299,   0,   0,   0,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ｺｰﾄﾞ：40～ 59 */
// MH810103 GG119202(S) 決済停止エラーの登録
//     0,   0,   0,   0, 299,   0,   0, 301,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ｺｰﾄﾞ：40～ 59 */
// MH810105(S) MH364301 E3249を発生／解除別に変更
//     0,   0,   0,   0, 314,   0, 317, 316,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ｺｰﾄﾞ：40～ 59 */
     0,   0,   0,   0, 314,   0, 317, 316,   0,   332,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ｺｰﾄﾞ：40～ 59 */
// MH810105(E) MH364301 E3249を発生／解除別に変更
// MH810103 GG119202(E) 決済停止エラーの登録
// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//   284, 285, 286, 287, 288, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297,   0,   0,   0, /* ｺｰﾄﾞ：60～ 79 */
// MH810105(S) MH364301 QRコード決済対応／PiTaPa対応
//   284, 285, 286, 287,   0, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297,   0,   0,   0, /* ｺｰﾄﾞ：60～ 79 */
   284, 285, 286, 287,   0, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297, 328, 329, 330, /* ｺｰﾄﾞ：60～ 79 */
// MH810105(E) MH364301 QRコード決済対応／PiTaPa対応
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ｺｰﾄﾞ：80～ 99 */
},
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダエラー表示文字対応)

// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(LCD_IF対応)
{	// 車番チケットレスLCD通信	26
	  0,299,300,301,302,303,304,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,305,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：80～ 99 */
},

// バーコード	30
{
	  0,  0,  0,  0,  0,306,  0,  0,  0,  0,307,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：80～ 99 */
},

// DC-NET通信	28
{
	  0,311,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：80～ 99 */
},

// リアルタイム通信	37
{
	  0,308,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,309,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,310,  0,	/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：80～ 99 */
},

// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(LCD_IF対応)
// MH810100(S) S.Fujii 2020/08/24 #4609 SDカードエラー対応
// SDカード	31
{
	//0                                      10
// GG129000(S) R.Endo 2023/06/21 車番チケットレス4.1 #7063 運用中にSDカードのファイルシステムが破損する(FCR P230154)
// 	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,312,313,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,312,313,  0,333,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ： 0～ 19 */
// GG129000(E) R.Endo 2023/06/21 車番チケットレス4.1 #7063 運用中にSDカードのファイルシステムが破損する(FCR P230154)
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：20～ 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：40～ 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：60～ 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ｺｰﾄﾞ：80～ 99 */
},
// MH810100(E) S.Fujii 2020/08/24 #4609 SDカードエラー対応
// MH810104 GG119201(S) 電子ジャーナル対応
{	// 電子ジャーナル	22
//	  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19
// MH810104 GG119201(S) 電子ジャーナル対応 #5949 エラーコードと精算機のエラー表示が一致しない
	  0, 320, 321, 322, 326,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	//  0～19
// MH810104 GG119201(E) 電子ジャーナル対応 #5949 エラーコードと精算機のエラー表示が一致しない
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 20～39
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 40～59
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 60～79
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 80～99
},
// MH810104 GG119201(E) 電子ジャーナル対応

{	// サブCPU機能	80
	 0,88,89, 0, 229,230,231,232,233, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},

{	// クレジット( エラー77と共通の文字列を使っておく )
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,256,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00～19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,257,258,  0,  0,  0,  0,	/* 20～39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 40～59 */
	263,264,265,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,259,260,261,262,  0,  0,	/* 60～79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80～99 */
},

// 以下 3桁のエラーコード 特殊処理用
{	// LAN2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
{	// Dopa2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
};

static	const	char	error_pri[] = {
	mod_main
	,	mod_SubCPU
	,	mod_read,	mod_prnt,	mod_coin,	mod_note
	,	mod_ifflap,	mod_ifrock, mod_crrflap
	,	mod_ntnet,	mod_ntibk
	,	mod_lprn,	mod_mifr
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	,	mod_cct,	mod_Suica,	mod_Edy
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
	,	mod_Suica
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	,	mod_remote
	,	mod_fomaibk
	,	mod_lanibk,	mod_lanibk2
	,	mod_dopaibk,	mod_dopaibk2
	,	mod_ntmf,	mod_ntdp
	,	mod_cappi
	,	mod_idc
	,	mod_card
	,	mod_flapcrb
	,	mod_can
	,	mod_sodiac
	,	mod_I2c	
	,	mod_Credit
// MH321800(S) D.Inaba ICクレジット対応
	,	mod_ec
// MH321800(E) D.Inaba ICクレジット対応
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(LCD_IF対応)
	,	mod_tklslcd
	,	mod_barcode
	,	mod_dc_net
	,	mod_realtime
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(LCD_IF対応)
// MH810100(S) S.Fujii 2020/08/24 #4609 SDカードエラー対応
	,	mod_sd
// MH810100(E) S.Fujii 2020/08/24 #4609 SDカードエラー対応
// MH810104 GG119201(S) 電子ジャーナル対応
	,	mod_eja
// MH810104 GG119201(E) 電子ジャーナル対応
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
	,	mod_cc
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
};
unsigned short UsMnt_ErrAlm( void )
{
	int i, j;
	short TotalPageNo;
	short CurrentPageNo;
	short msg = -1;
	ushort ret;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[23] );			/* "＜エラー・アラーム確認＞　　　" */

	TotalPageNo = 0;
	// 発生中ｱﾗｰﾑの数を数える
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		for( j = 1; j < ALM_NUM_MAX; j++ ){
			if( ALM_CHK[i][j] && AlmTableNo[i][j] ){
				 TotalPageNo++;
			}
		}
	}

	// 発生中ｴﾗｰの数を数える
	for( i = 0; i < ERR_MOD_MAX; i++ ){
		for( j = 1; j < ERR_NUM_MAX; j++ ){
			if( ERR_CHK[i][j] && ErrTableNo[i][j] ){
				TotalPageNo++;
			}
		}
	}

	// ロック装置とフラップの開閉異常（ロック）の数を数える（簡易フラップ／ロック）
	if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) ){
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 0, _MTYPE_LOCK ) );
		}
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 1, _MTYPE_LOCK ) );
		}
	}
	if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) ){
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 0, _MTYPE_INT_FLAP ) );
		}
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 1, _MTYPE_INT_FLAP ) );
		}
	}

	// 総画面数をｾｯﾄ
	if( TotalPageNo ){
		TotalPageNo--;
		TotalPageNo = (TotalPageNo / 6) + 1;
	}else{
		TotalPageNo = -1;	// なにも発生していない
	}

	if( TotalPageNo == -1 ){
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[0] ); // "  発生中のエラー・アラームは  "
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[1] ); // "        ありません            "
	}

	if( TotalPageNo > 1 ){
		Fun_Dsp( FUNMSG[6] );				// "　▲　　▼　　　　　　　 終了 "
	}else{
		Fun_Dsp( FUNMSG[8] );				// "　　　　　　　　　　　　 終了 "
	}

	CurrentPageNo = 1;
	if( TotalPageNo != -1 ){
		HasseiErrSearch( CurrentPageNo );		// ｴﾗｰを表示する
	}

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg )
		{
			case KEY_TEN_F5:
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
				if( TotalPageNo == -1 ) break;
				if( TotalPageNo == 1 ) break;		// 1ﾍﾟｰｼﾞしかないので無効
				BUZPI();
				if( CurrentPageNo == 1 ){
					CurrentPageNo = TotalPageNo;
				}else{
					CurrentPageNo--;
				}
				HasseiErrSearch( CurrentPageNo );	// ｴﾗｰを表示する
				break;
			case KEY_TEN_F2:
				if( TotalPageNo == -1 ) break;
				if( TotalPageNo == 1 ) break;		// 1ﾍﾟｰｼﾞしかないので無効
				BUZPI();
				if( CurrentPageNo == TotalPageNo ){
					CurrentPageNo = 1;
				}else{
					CurrentPageNo++;
				}
				HasseiErrSearch( CurrentPageNo );	// ｴﾗｰを表示する
				break;
			case KEY_TEN_F3:
			case KEY_TEN_F4:
			case KEY_TEN_CL:
				break;
		}
	}
	return( ret );
}

//----------------------------------------------------------------------------
// 1)発生中のｱﾗｰﾑとｴﾗｰをｻｰﾁする。
// 2)ﾍﾟｰｼﾞ番号により表示するｱｲﾃﾑを配列Dspitemへｾｯﾄする
// 3)配列Dspitemを表示する
//----------------------------------------------------------------------------
static void HasseiErrSearch( short page )
{
	static uchar	dsp[30];
	int		i, j;
	int		k;
	int		TotalSearch = 0;
	int 	StartItemNu;
	int 	ItemOfPage = 0;		// 1画面に表示するｱｲﾃﾑの数

	struct {
		ushort	DspItem[6];
		ushort	SlaveIfErr[6];		// 子機IF盤ｴﾗｰ用
		uchar	LockArea[6];		// 区画
		ushort	LockPosi[6];		// 車室番号
	} dpsb;

	uchar	buf[6];

	uchar	tno = 0;

	memset( &dpsb, 0, sizeof(dpsb) );

	// 画面に表示する最初のｱｲﾃﾑ番号をﾍﾟｰｼﾞ番号からｾｯﾄする
	if( page ) page -= 1;
	StartItemNu = page * 6;

	// まずｱﾗｰﾑからｻｰﾁする
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		for( j = 1; j < ALM_NUM_MAX; j++ ){
			if( ALM_CHK[i][j] && AlmTableNo[i][j] )
			{
				TotalSearch++;
				if( TotalSearch > StartItemNu ){
					dpsb.DspItem[ItemOfPage] = AlmTableNo[i][j];
					ItemOfPage++;
					if( ItemOfPage >= 6 ){
						// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
						j = ALM_NUM_MAX;
						i = ALM_MOD_MAX;
					}
				}
			}
		}
	}
	if( ItemOfPage < 6 ){
		// 次にｴﾗｰをｻｰﾁする
		for( k = 0; k < ERR_MOD_MAX; k++ ){
			i = error_pri[k];
			for( j = 1; j < ERR_NUM_MAX; j++ ){
				if( ERR_CHK[i][j] && ErrTableNo[i][j] )
				{
					TotalSearch++;
					if( TotalSearch > StartItemNu ){
						dpsb.DspItem[ItemOfPage] = ErrTableNo[i][j];
						ItemOfPage++;
						if( ItemOfPage >= 6 ){
							// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
							j = ERR_NUM_MAX;
							k = ERR_MOD_MAX;
						}
					}
				}
			}
		}
	}

	if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) ){
		if( ItemOfPage < 6 ){
			// 内蔵ﾌﾗｯﾌﾟ装置の上昇異常（ﾌﾗｯﾌﾟ）の数を数える
			if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
				for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){
					WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b04 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 93;
								// 車室番号をｾｯﾄ（表示用）
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
									break;
								}
							}
						}
					}
				}
			}
		}

		if( ItemOfPage < 6 ){
			// 内蔵ﾌﾗｯﾌﾟ装置の下降異常（ﾌﾗｯﾌﾟ）の数を数える
			if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
				for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){
					WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b05 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 94;
								// 車室番号をｾｯﾄ（表示用）
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) ){
		if( ItemOfPage < 6 ){
			// 内蔵ﾌﾗｯﾌﾟ装置の上昇異常（ﾌﾗｯﾌﾟ）の数を数える
			if( ERR_CHK[mod_flapcrb][ERR_FLAPCLOSEFAIL] ){
				for( i = INT_CAR_START_INDEX; i < BIKE_START_INDEX; i++ ){
					WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b04 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 127;
								// 車室番号をｾｯﾄ（表示用）
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
									break;
								}
							}
						}
					}
				}
			}
		}

		if( ItemOfPage < 6 ){
			// 内蔵ﾌﾗｯﾌﾟ装置の下降異常（ﾌﾗｯﾌﾟ）の数を数える
			if( ERR_CHK[mod_flapcrb][ERR_FLAPOPENFAIL] ){
				for( i = INT_CAR_START_INDEX; i < BIKE_START_INDEX; i++ ){
					WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b05 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 128;
								// 車室番号をｾｯﾄ（表示用）
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// 表示するｱｲﾃﾑ数が画面一杯なのでｻｰﾁ終了
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	for( i = 0; i < 6; i++ ){
		if( dpsb.DspItem[i] ){
			if(( dpsb.DspItem[i] == 93 )||( dpsb.DspItem[i] == 94 )){
				// ロック装置開閉異常
// MH322914 (s) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
				dsp[18] = '('; // 0x0028;
				if( dpsb.LockArea[i] ){
					dsp[19] = (uchar)( 0x40 + dpsb.LockArea[i] );
					dsp[20] = '-'; // 0x002d;
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[21] = (uchar)(buf[0]);
					dsp[22] = (uchar)(buf[1]);
					dsp[23] = (uchar)(buf[2]);
					dsp[24] = (uchar)(buf[3]);
					dsp[25] = ')'; // 0x0029
				}else{
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[19] = (uchar)(buf[0]);
					dsp[20] = (uchar)(buf[1]);
					dsp[21] = (uchar)(buf[2]);
					dsp[22] = (uchar)(buf[3]);
					dsp[23] = ')';
				}
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // ｴﾗｰ、ｱﾗｰﾑ表示
			}else if(( dpsb.DspItem[i] == 127 )||( dpsb.DspItem[i] == 128 )){
				// ﾌﾗｯﾌﾟ装置開閉異常
// MH322914 (s) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
				dsp[20] = '('; // 0x0028;
				if( dpsb.LockArea[i] ){
					dsp[21] = (uchar)( 0x40 + dpsb.LockArea[i] );
					dsp[22] = '-'; // 0x002d;
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[23] = (uchar)(buf[0]);
					dsp[24] = (uchar)(buf[1]);
					dsp[25] = (uchar)(buf[2]);
					dsp[26] = (uchar)(buf[3]);
					dsp[27] = ')'; // 0x0029
				}else{
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[21] = (uchar)(buf[0]);
					dsp[22] = (uchar)(buf[1]);
					dsp[23] = (uchar)(buf[2]);
					dsp[24] = (uchar)(buf[3]);
					dsp[25] = ')';
				}
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // ｴﾗｰ、ｱﾗｰﾑ表示
			}else if( dpsb.DspItem[i] == 129 ){
				// IBC前回ﾃﾞｰﾀ未処理
// MH322914 (s) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 静的領域のメモリ破壊修正[共通バグNo.1244](MH341106)
				dsp[24] = '('; // 0x0028;
				intoas( buf, (ushort)tno, 2 );
				dsp[25] = (uchar)(buf[0]);
				dsp[26] = (uchar)(buf[1]);
				dsp[27] = ')';
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // ｴﾗｰ、ｱﾗｰﾑ表示
			}else if( (150 <= dpsb.DspItem[i]) && (dpsb.DspItem[i] <= 152) ){			// Suicaリーダーエラー
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[SuicaErrCharChange(dpsb.DspItem[i])] );	// "電子マネーリーダー..."表示
			}else{
// MH810105(S) MH364301 QRコード決済対応
				if (dpsb.DspItem[i] == 330) {		// E32-79
					if (RecvSubBrandTbl.tbl_sts.BIT.MORE_LESS == 2){
						dpsb.DspItem[i] = 331;		// less
					}
				}
// MH810105(E) MH364301 QRコード決済対応
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[dpsb.DspItem[i]] ); // ｴﾗｰ、ｱﾗｰﾑ表示
			}
		}else{
			grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] ); // 行ｸﾘｱ
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：券データ確認                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_TicketCkk( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_TicketCkk(void)
{
	ushort	msg;
	int	dsp, loop;
	short	page, pagemax;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[44]);	/* "＜券データ確認＞　　　　　　　" */

// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)
//	CardErrDsp();		
	CardErrDsp( OPECTL.CR_ERR_DSP );	// エラー表示
// MH810100(E) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)
	for( ; ; ) {

		page = 0;
		if(( MAGred[MAG_ID_CODE] != (char)-1 ) && 		// MAGredの値が-1(0xFF)の場合は読取不可カードなのでそれ以外で、
		   ( OPECTL.LastCardInfo & 0x8000) ){			// 最上位BitのチェックでJIS表示かを判定
			grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[6] );			// "JISカード　　　" 
		}else{
			switch( OPECTL.LastCardInfo ){
				case 0x2d:
					break;
				case 0x1a:
					break;
				case 0x0e:
					PrepaidDsp(0);				// ﾌﾟﾘﾍﾟｲﾄﾞ情報作成
					page = 1;
					pagemax = 2;
					break;
				case 0x2c:						// 回数券
					break;
				case 0x41:						// ＜アマノ係員／補充カード＞
					break;
				default:
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDERRSTR[11] );			// "　　　券データはありません　　"
					break;
			}
		}

		dsp = 1;
		for( loop=1; loop; ) {

			if( dsp ){
				dsp = 0;
				switch( OPECTL.LastCardInfo ){
					case 0x2d:
						ServiceDsp();
						break;
					case 0x1a:
						PassDsp();
						break;
					case 0x0e:
						PrepaidDsp(page);
						break;
					case 0x2c:							// 回数券
						KaiDsp();
						break;
					case 0x41:											// ＜アマノ係員／補充カード＞
						if(AmanoCard_Dsp())								// 画面表示情報作成
						{
							grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDERRSTR[11] );		// "　　　券データはありません　　"
						}
						break;
				}
				if( page == 1 ){
					Fun_Dsp(FUNMSG[11]);				/* "　　　　▼　　　　　　　 終了 " */
				}else if( page == 2 ){
					Fun_Dsp(FUNMSG[12]);				/* "　▲　　　　　　　　　　 終了 " */
				}else{
					Fun_Dsp(FUNMSG[8]);					/* "　　　　　　　　　　　　 終了 " */
				}
			}

			msg = StoF( GetMessage(), 1 );

			switch( msg ){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:
					BUZPI();
					return( MOD_CHG );
				case KEY_TEN_F5:				/* F5:Exit */
					BUZPI();
					return( MOD_EXT );

				case KEY_TEN_F1:
					if( page != 0 ){
						if( page != 1 ){
							page--;
							BUZPI();
							dsp = 1;
						}
					}
					break;
				case KEY_TEN_F2:
					if( page != 0 ){
						if( page < pagemax ){
							BUZPI();
							page++;
							dsp = 1;
						}
					}
					break;

				case ARC_CR_R_EVT:				// Card inserted
					displclr(1);
					displclr(2);
					displclr(3);
					displclr(4);
					displclr(5);
					displclr(6);
					loop = 0;
					break;
				default:
					break;
			}
		}
	}
}


// サービス券表示
static void ServiceDsp( void )
{
	uchar	buf[2];
	m_servic *mag = (m_servic*)&MAGred[MAG_ID_CODE];
	ulong	temp;
	static const char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };
	ushort	wk1, wk2;

	cr_dsp.service.Kind = (ushort)( mag->svc_pno[1] & 0x0f );
	cr_dsp.service.ShopNo = (short)mag->svc_sno[1] + (((short)mag->svc_sno[0])<<7);
	memcpy( cr_dsp.service.StartDate, mag->svc_sta, 6 );
	cr_dsp.service.Status = (ushort)( mag->svc_sts );

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->svc_pno[0];
		wk2 = mag->svc_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17格納
		wk2 = 0;
	}else{
		wk1 = mag->svc_pno[0];
		wk2 = mag->svc_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// 駐車場No.ｾｯﾄ
		wk2 = 0;
	}

	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"

	if( cr_dsp.service.Kind != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[0] );			// "サービス券　　　"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[0] );			// "  種別      分類コード        "
		buf[0] = (uchar)(0x82);	// 'Ａ'
		buf[1] = (uchar)(0x60 + cr_dsp.service.Kind-1);
	    grachr( 3, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &buf[0] );							 // 種別
		opedpl( 3, 22, (ulong)cr_dsp.service.ShopNo, 4, 1, 0 , COLOR_BLACK,  LCD_BLINK_OFF); // 分類コード
		wk2 = 1;
	}
	else if( cr_dsp.service.Status != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[2] );			// "割引券　　　"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[3] );			// "  種別        店№         "
		opedpl( 3, 6, (ulong)cr_dsp.service.Status, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // 種別
		opedpl( 3,18, (ulong)cr_dsp.service.ShopNo, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // 店№
		wk2 = 2;
	}
	else if( cr_dsp.service.ShopNo != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[1] );			// "掛売券　　　"
		grachr( 3, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[3]+14 );		// "  店№         "
		opedpl( 3, 6, (ulong)cr_dsp.service.ShopNo, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // 店№
		wk2 = 2;
	}
	if( memcmp( cr_dsp.service.StartDate, ck_dat, 6 ) != 0 ){
		// 期限あり
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );			// "  有効開始      年    月    日"
		grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );			// "  有効終了      年    月    日"
		opedpl( 4, 12, (ulong)cr_dsp.service.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// 有効開始（年）表示
		opedpl( 4, 18, (ulong)cr_dsp.service.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// 　　　　（月）表示
		opedpl( 4, 24, (ulong)cr_dsp.service.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// 　　　　（日）表示
		opedpl( 5, 12, (ulong)cr_dsp.service.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 有効終了（年）表示
		opedpl( 5, 18, (ulong)cr_dsp.service.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 　　　　（月）表示
		opedpl( 5, 24, (ulong)cr_dsp.service.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 　　　　（日）表示
	}
	if( wk2 != 0 ){
		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.service.ParkNo = wk1;
				grachr( 2, ((wk2==2)?6:10), 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）	表示
				break;
			}
		}
	}
}

static void PrepaidDsp( short page )
{
	char c;
	m_prepid *mag = (m_prepid*)&MAGred[MAG_ID_CODE];
	ushort	wk1;
	ulong	temp;
	struct	clk_rec		clk_wok;

	if( page == 0 ){
		cr_dsp.prepaid.Mno = (ushort)astoin( mag->pre_mno, 2 );
		cr_dsp.prepaid.IssueDate[0] = (uchar)astoin( mag->pre_sta, 2 );
		cr_dsp.prepaid.IssueDate[1] = (uchar)astoin( mag->pre_sta+2, 2 );
		cr_dsp.prepaid.IssueDate[2] = (uchar)astoin( mag->pre_sta+4, 2 );
		c = mag->pre_amo;
		if(( c > 0x30)&&( c < 0x3a )){
			cr_dsp.prepaid.SaleAmount = (ulong)(c-0x30) * 1000L;
		}else if(( c > 0x40)&&( c < 0x5b )){
			cr_dsp.prepaid.SaleAmount = ((ulong)(c-0x40) * 1000L) + 9000L;
		}else{
			cr_dsp.prepaid.SaleAmount = ((ulong)(c-0x60) * 1000L) + 35000L;
		}
		cr_dsp.prepaid.RemainAmount = astoinl( mag->pre_ram, 5 );
		cr_dsp.prepaid.LimitAmount = (ushort)astoinl( mag->pre_plm, 3 );
		cr_dsp.prepaid.CardNo = astoinl( mag->pre_cno, 5 );
	}else if( page == 1 ){
		if(MAGred[MAG_GT_APS_TYPE] == 1){
			temp  =  (long)mag->pre_pno[0] & 0x0000003F;
			temp |= ((long)mag->pre_pno[1] & 0x0000003F) << 6;
			temp |= ((long)mag->pre_pno[2] & 0x0000003F) << 12;
		}else{
			temp = astoin( mag->pre_pno, 2 );
			temp += ( mag->pre_pno[2]&0x0f ) * 100;
		}
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[3] );			// "プリペイドカード"
		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.prepaid.ParkNo = wk1;
				grachr( 2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）	表示
				break;
			}
		}
		grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[4] );			// "  機械№      券№            "
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[5] );			// "  販売金額              円    "
		grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[6] );			// "  カード残額            円    "
		grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[7] );			// "  発行      年    月    日    "
		opedpl( 3, 8, (ulong)cr_dsp.prepaid.Mno, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 3, 18, (ulong)cr_dsp.prepaid.CardNo, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 4, 14, (ulong)cr_dsp.prepaid.SaleAmount, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 5, 14, (ulong)cr_dsp.prepaid.RemainAmount, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 8, (ulong)cr_dsp.prepaid.IssueDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 14, (ulong)cr_dsp.prepaid.IssueDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 20, (ulong)cr_dsp.prepaid.IssueDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
	}else{
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[3] );			// "プリペイドカード"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[25] );		// "  限度額                   円 ",
		opedpl( 3, 18, (ulong)cr_dsp.prepaid.LimitAmount, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		displclr(4);
		if ( prm_get( COM_PRM,S_PRP,11,2,1 ) == 99 ){
			grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[27] );	// " (有効期限           期限なし)",
		}else{
			grachr ( 5, 0, 30, 0, COLOR_BLACK,LCD_BLINK_OFF, CARDSTR2[26] );	// " (有効期限     年    月    日)",
			al_preck_Kigen ( (m_gtprepid*)MAGred, &clk_wok );
			opedpl ( 5, 10, (ulong)(clk_wok.year % 100), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
			opedpl ( 5, 16, (ulong)(clk_wok.mont),       2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
			opedpl ( 5, 22, (ulong)(clk_wok.date),       2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
		}
		displclr(6);
	}
}

static void PassDsp( void )
{
	ushort	wk1, wk2;

	ulong	temp;
	m_apspas *mag = (m_apspas*)&MAGred[MAG_ID_CODE];

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->aps_pno[0];
		wk2 = mag->aps_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17格納
		wk2 = 0;
	}else{
		wk1 = mag->aps_pno[0];
		wk2 = mag->aps_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// 駐車場No.ｾｯﾄ
		wk2 = 0;
	}
	cr_dsp.pass.Kind = mag->aps_pno[1] & 0x0f;
	wk2 = mag->aps_pcd[0];
	wk1 = mag->aps_pcd[1];
	cr_dsp.pass.Code = wk1 | (wk2<<7);
	cr_dsp.pass.StartDate[0] = mag->aps_sta[0];
	cr_dsp.pass.StartDate[1] = mag->aps_sta[1];
	cr_dsp.pass.StartDate[2] = mag->aps_sta[2];
	cr_dsp.pass.EndDate[0] = mag->aps_end[0];
	cr_dsp.pass.EndDate[1] = mag->aps_end[1];
	cr_dsp.pass.EndDate[2] = mag->aps_end[2];
	cr_dsp.pass.Status = mag->aps_sts;
	cr_dsp.pass.WriteDate[0] = mag->aps_wrt[0];
	cr_dsp.pass.WriteDate[1] = mag->aps_wrt[1];
	cr_dsp.pass.WriteDate[2] = mag->aps_wrt[2];
	cr_dsp.pass.WriteDate[3] = mag->aps_wrt[3];

	grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[4] );								// "定期券          "
	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "＜※＞"

		// 定期券ﾃｰﾌﾞﾙ分割がある場合

		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.prepaid.ParkNo = wk1;
				grachr( 2, 6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）	表示
				break;
			}
		}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[9] );								// "  種別        券№            "
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );								// "  有効開始      年    月    日"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );								// "  有効終了      年    月    日"
	grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[10] );							// "            月    日    ：    "

	opedpl( 3, 6, (ulong)cr_dsp.pass.Kind, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );				// 種別
	opedpl( 3, 18, (ulong)cr_dsp.pass.Code, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );				// 券№
	opedpl( 4, 12, (ulong)cr_dsp.pass.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 有効開始（年）
	opedpl( 4, 18, (ulong)cr_dsp.pass.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（月）
	opedpl( 4, 24, (ulong)cr_dsp.pass.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（日）
	opedpl( 5, 12, (ulong)cr_dsp.pass.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 有効終了（年）
	opedpl( 5, 18, (ulong)cr_dsp.pass.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（月）
	opedpl( 5, 24, (ulong)cr_dsp.pass.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（日）

	switch( cr_dsp.pass.Status ){
		case 0:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[0] );						// "初期"
			break;
		case 1:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[2] );						// "出庫"
			break;
		case 2:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[1] );						// "入庫"
			break;
		case 3:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[3] );						// "精算"
			break;
		default:
			if( cr_dsp.pass.Status == (uchar)prm_get( COM_PRM,S_KOU,5,2,1 ) ){
				grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[4] );					// "更新"
			}
			break;
	}
	opedpl( 6, 8, (ulong)cr_dsp.pass.WriteDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 処理日時（月）表示
	opedpl( 6, 14, (ulong)cr_dsp.pass.WriteDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（日）表示
	opedpl( 6, 20, (ulong)cr_dsp.pass.WriteDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（時）表示
	opedpl( 6, 26, (ulong)cr_dsp.pass.WriteDate[3], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 　　　　（分）表示
}

static void KaiDsp( void )
{
	ushort	wk1, wk2;
	ushort	line;
	ulong	temp;
	char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };

	m_kaisuutik *mag = (m_kaisuutik *)&MAGred[MAG_ID_CODE];

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->kaitik_pno[0];
		wk2 = mag->kaitik_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17格納
		wk2 = 0;
	}else{
		wk1 = mag->kaitik_pno[0];
		wk2 = mag->kaitik_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// 駐車場No.ｾｯﾄ
		wk2 = 0;
	}

	cr_dsp.kaiticket.LimDosu		= mag->kaitik_pno[1] & 0x0f;		// 制限度数ｾｯﾄ

	wk2 = mag->kaitik_tan[0];
	wk1 = mag->kaitik_tan[1];
	cr_dsp.kaiticket.TanRyo 		= wk1 | (wk2<<7);					// 単位料金ｾｯﾄ

	cr_dsp.kaiticket.StartDate[0]	= mag->kaitik_sta[0];				// 有効開始（年）ｾｯﾄ
	cr_dsp.kaiticket.StartDate[1]	= mag->kaitik_sta[1];				// 　　　　（月）ｾｯﾄ
	cr_dsp.kaiticket.StartDate[2]	= mag->kaitik_sta[2];				// 　　　　（日）ｾｯﾄ

	cr_dsp.kaiticket.EndDate[0]		= mag->kaitik_end[0];				// 有効終了（年）ｾｯﾄ
	cr_dsp.kaiticket.EndDate[1]		= mag->kaitik_end[1];				// 　　　　（月）ｾｯﾄ
	cr_dsp.kaiticket.EndDate[2]		= mag->kaitik_end[2];				// 　　　　（日）ｾｯﾄ

	cr_dsp.kaiticket.Kaisu			= mag->kaitik_kai;					// 回数（残り）ｾｯﾄ

	cr_dsp.kaiticket.WriteDate[0]	= mag->kaitik_wrt[0];				// 処理日時（月）ｾｯﾄ
	cr_dsp.kaiticket.WriteDate[1]	= mag->kaitik_wrt[1];				// 　　　　（日）ｾｯﾄ
	cr_dsp.kaiticket.WriteDate[2]	= mag->kaitik_wrt[2];				// 　　　　（時）ｾｯﾄ
	cr_dsp.kaiticket.WriteDate[3]	= mag->kaitik_wrt[3];				// 　　　　（分）ｾｯﾄ

	line = 2;															// 表示開始行
	grachr( line, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[5] );								// "回数券          "
	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"
	for( wk1=0 ; wk1<=3 ; wk1++ ){
		if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
			cr_dsp.kaiticket.ParkNo = wk1;
			grachr( 2, 6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）	表示
			break;
		}
	}
	line++;

	if( cr_dsp.kaiticket.TanRyo != 0 ){
		// ≠無料回数券
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[11] );							// "  残／制限度数      回／    回"
		opedpl( line, 16, (ulong)cr_dsp.kaiticket.Kaisu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 回数（残り）	表示
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.LimDosu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 制限度数		表示
		line++;
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[13] );							// "  単位金額          円        "
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.TanRyo, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 単位金額	表示
		line++;
	}
	else{
		// ＝無料回数券
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[12] );							// "  残度数        回            "
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.Kaisu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 回数（残り）	表示
		line++;
	}

	if( memcmp( cr_dsp.kaiticket.StartDate, ck_dat, 6 ) != 0 ){

		// 有効期限あり

		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );							// "  有効開始      年    月    日"
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 年	表示
		opedpl( line, 18, (ulong)cr_dsp.kaiticket.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 月	表示
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 日	表示
		line++;

		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );							// "  有効終了      年    月    日"
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 年	表示
		opedpl( line, 18, (ulong)cr_dsp.kaiticket.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 月	表示
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 日	表示
	}
}
// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)		
//void CardErrDsp( void )	
void CardErrDsp( short err )															// 料金計算テストはerrを使ってエラー表示を分岐させています。
{
//	switch( OPECTL.CR_ERR_DSP )
	switch( err )
// MH810100(E) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)			
	{
		case 0:
			break;
		case 1:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[0] );				// " ＜＜ 駐車場№が違います ＞＞ "
			break;
		case 2:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[12] );			// "   ＜＜ データ異常です ＞＞   "
			break;
		case 3:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[2] );				// " ＜＜　　期限切れです　　＞＞ "
			break;
		case 4:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[4] );				// " ＜＜定期入出庫エラーです＞＞ "
			break;
		case 5:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[3] );				// " ＜＜　無効登録定期です　＞＞ "
			break;
		case 6:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[1] );				// "     ＜＜ 期限前です ＞＞     "
			break;
		case 7:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[6] );				// " ＜＜  残額がありません  ＞＞ "
			break;
		case 8:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[13] );			// "   ＜＜ 使用不可券です ＞＞   "
			break;
		case 9:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[7] );				// "    ＜＜ 二重使用です ＞＞    "
			break;
		case 10:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[8] );				// " ＜＜ 挿入方向が違います ＞＞ "
			break;
		case 13:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[9] );				// "  ＜＜　種別が規定外です　＞＞"
			break;
		case 14:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[10] );			// "  ＜＜限度枚数を超えてます＞＞"
			break;
		case 20:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[14] );			// "   ＜＜ 読取できません ＞＞   "
			break;
		case 25:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[16] );			// "  ＜＜ 他の車種の券です ＞＞  "
			break;
		case 26:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[17] );			// " ＜＜ 設定されていません ＞＞ "
			break;
		case 27:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[18] );			// " ＜＜ 料金種別が違います ＞＞ "
			break;
		case 28:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[19] );			// " ＜＜ 限度枚数が０枚です ＞＞ "
			break;
		case 29:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[20] );			// "   ＜＜ 問合せ異常です ＞＞   "
			break;
		case 30:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[21] );			// " ＜＜ 精算順番が違います ＞＞ "
			break;
		case 40:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[0] );			// 短券エラー
			break;
		case 41:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[1] );			// 長券エラー
			break;
		case 42:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[2] );			// スタートビット符号エラー
			break;
		case 43:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[3] );			// データエラー
			break;
		case 44:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[4] );			// パリティエラー
			break;
		case 45:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[5] );			// 読取りＣＲＣエラー
			break;
		case 46:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ＜＜ 読取できません ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[6] );			// その他のエラー
			break;
		case 47:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[23] );			// "   ＜＜ データ異常です ＞＞   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[7] );			// CRCエラー
			break;
		case 50:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[15] );			// "   ＜＜ 更新期限外です ＞＞   "
			break;
		case 33:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[24] );			// " ＜＜カード設定が違います＞＞ "
			break;
		default:
			break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 拡張機能 メニュー構築                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_ExtendMenuMake( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ﾒﾆｭｰ表示の項目数、0=ﾒﾆｭｰOpenする必要なし                |*/
/*[]----------------------------------------------------------------------[]*/
/*|	ユーザメンテナンス→拡張機能メニューを選択された場合、子メニューを     |*/
/*|	構築する															   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar Is_ExtendMenuMake( void )
{
	uchar	mno;		//メニュー文字列番号
	uchar	ret;		//メニュー項目数

	ret =0;
	mno =0;

	memset(&USM_ExtendMENU[0][0], 0, sizeof(USM_ExtendMENU));			//構築先（メニュー文字列）をクリア
	memset(&USM_ExtendMENU_TBL[0][0], 0, sizeof(USM_ExtendMENU_TBL));	//構築先（制御テーブル）をクリア

// MH810100(S) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
//	// 複数台数集計
//	if( (prm_get(COM_PRM, S_NTN, 26, 1, 2) == 1) &&			// 複数集計問合せ＝する
//		(prm_get(COM_PRM, S_TOT,  2, 1, 1) == 1) &&			// 複数台数集計＝あり
//		( (2 <= OPECTL.Mnt_lev) && (OPECTL.Mnt_lev <= 4) ) )// 権限が管理者以上
//	{
//		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//		ret += 1;
//	}
//
//	// クレジット処理
//	mno += 1;
//	if( (prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) ||	// ｸﾚｼﾞｯﾄｻｰﾊﾞｰ
//		(prm_get(COM_PRM, S_PAY, 24, 1, 2) == 3) )
//	{
//		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//		ret += 1;
//	}
// MH810100(E) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
	
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	// Ｅｄｙ処理
//	mno += 1;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

// MH810100(S) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
//	// Ｓｕｉｃａ処理
//	mno += 1;
//
//// MH321800(S) D.Inaba ICクレジット対応（判定変更）
////	if( prm_get (COM_PRM, S_PAY, 24, 1, 3) !=0 )
//	if( isSX10_USE() )
//// MH321800(E) D.Inaba ICクレジット対応（判定変更）
//	{
//		if(prm_get(COM_PRM, S_SCA, 7, 2, 5) != 0)
//		{
//			Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//			ret += 1;
//		}
//	}
//
//	// 法人カード
//	mno += 1;
// MH810100(E) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)

	// ＭＴ集計
// MH810100(S) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
//	mno += 1;
// MH810100(E) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
	if( (prm_get ( COM_PRM, S_TOT, 1, 1, 1 ) != 0) &&	// MT集計あり
		( (2 <= OPECTL.Mnt_lev) ))						// 権限が管理者以上
	{
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}

	// 電磁ロック解除
	mno += 1;
	if(CPrmSS[S_PAY][17]) {							// 電磁ロックありの設定
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}

// MH321800(S) D.Inaba ICクレジット対応 (拡張機能に決済リーダ処理追加)
	// 決済リーダ処理
	mno += 1;
	if( isEC_USE() ){		// 決済リーダ接続
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}
// MH321800(E) D.Inaba ICクレジット対応 (拡張機能に決済リーダ処理追加)
// MH810104 GG119201(S) 電子ジャーナル対応
	// 電子ジャーナル
	mno += 1;
	if( isEJA_USE() ){		// 電子ジャーナル接続あり
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}
// MH810104 GG119201(E) 電子ジャーナル対応
	Ext_Menu_Max = ret;
	return	ret;
}

typedef struct{
	const uchar	(*num)[3];
	const uchar	(*str)[29];
	const ushort	(*Ctrl_table)[4];
}t_make_table_info;
static	const	t_make_table_info	make_table_info[] = {
	{MENU_NUMBER,	EXTENDMENU_BASE,	USM_EXT_TBL},	// 拡張機能
	{MENU_NUMBER,	KIND_3_MENU,		USM_KIND3_TBL},	// 車室メニュー（３メニュー用）
	{MENU_NUMBER,	KIND_2_MENU,		USM_KIND_TBL},	// 車室メニュー（２メニュー用）
	{MENU_NUMBER,	FCNTMENUCOUNT,		FUNC_CNT_TBL2},	// 動作カウントメニュー
	{MENU_NUMBER,	LOCKMENU,			USM_LOCK_TBL},	// ﾛｯｸ装置開閉メニュー
};

/*[]----------------------------------------------------------------------[]*/
/*| 拡張機能 メニューテーブル構築                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_ExtendMenuMakeTable( void )                          |*/
/*| PARAMETER    : ret=構築先の配列番号                                    |*/
/*|              : mno=構築元の配列番号                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	ユーザメンテナンス→拡張機能メニューを選択された場合、子メニューを     |*/
/*|	構築する															   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Is_ExtendMenuMakeTable(uchar ret, uchar mno, uchar type )
{
	const t_make_table_info *p = &make_table_info[type];
	
	//メニューの項目文字列を作る
	memcpy(&USM_ExtendMENU[ret][0], p->num[ret], sizeof(p->num[0]) );			//番号を入れる
	memcpy(&USM_ExtendMENU[ret][2], p->str[mno], sizeof(p->str[0]) );			//文字列を入れる

	//メニューの制御テーブルを作る
	memcpy(&USM_ExtendMENU_TBL[ret][0], p->Ctrl_table[mno], sizeof(p->Ctrl_table[0]));
	USM_ExtendMENU_TBL[ret][2] = (ushort)ret;

}

/*[]----------------------------------------------------------------------[]*/
/*| 車室選択 メニュー構築				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_CarMenuMake( void )     	     	 		           |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : 文字列ポインタ                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			                                       |*/
/*| Date         : 2009-06-09                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short Is_CarMenuMake( uchar type )
{
	uchar	mno=0;		//メニュー文字列番号
	uchar	ret=0;		//メニュー項目数
	short	menu=0;		//メニュー項目数

	uchar	param;
	
	param = GetCarInfoParam();
	
	memset(&USM_ExtendMENU[0][0], 0, sizeof(USM_ExtendMENU));			//構築先（メニュー文字列）をクリア
	memset(&USM_ExtendMENU_TBL[0][0], 0, sizeof(USM_ExtendMENU_TBL));	//構築先（制御テーブル）をクリア

	switch( type ){
		case	CAR_3_MENU:								// 車室メニュー（３画面用）
			// 駐車
			if( param & 0x04 )
			{
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				menu = MNT_INT_CAR;
			}

			// 駐車（IF-3100/3150）
			mno++;
			if( param & 0x02 )
			{
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				menu = MNT_CAR;
			}
			break;
		case	MV_CNT_MENU:							// 動作カウントメニュー
			Is_ExtendMenuMakeTable(ret,mno,type);
			ret++;
			mno++;
			// no break;
		case	CAR_2_MENU:								// 車室メニュー（２画面用）
			if( param & 0x06 ){
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;

				if(( param & 0x06 ) == 0x04 )
					menu = MNT_INT_CAR;
				else
					menu = MNT_CAR;
			}
			break;
		case	LOCK_CTRL_MENU:							// ロック開閉メニュー
			if( param & 0x06 ){
				Is_ExtendMenuMakeTable(ret,mno,type);	// フラップ上昇下降（個別）
				ret++;
				mno++;

				Is_ExtendMenuMakeTable(ret,mno,type);	// フラップ上昇下降（全て）
				ret++;

				if(( param & 0x06 ) == 0x04 )
					menu = MNT_INT_CAR;
				else
					menu = MNT_CAR;
			}
	}
	// 駐輪
	mno++;
	if( param & 0x01 )
	{
		switch( type ){
			case	MV_CNT_MENU:
				if( ret == 0 ){
					mno++; 
				}
				break;
			case	LOCK_CTRL_MENU:
				if( ret == 0 ){
					mno++; 
				}
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				mno++;
				break;
		}
		Is_ExtendMenuMakeTable(ret,mno,type);
		ret++;
		menu = MNT_BIK;
	}

	Ext_Menu_Max = ret;
	return menu;
}

typedef struct{
	const uchar	(*str)[27];
	const ushort	(*Ctrl_table)[4];
	const ushort	max;
}t_make_menu;
static	const	t_make_menu	make_menu[] = {
	{FCMENU,	FUN_CHK_TBL,	FUN_CHK_MAX},
	{UMMENU,	USER_TBL,		USER_MENU_MAX},
// MH321800(S) D.Inaba ICクレジット対応 (決済リーダ接続時、決済リーダチェック追加)
// MH810100(S)
//	{FCMENU2,	FUN_CHK_TBL2,	FUN_CHK_MAX},
	{FCMENU2,	FUN_CHK_TBL2,	FUN_CHK_MAX2},
// MH810100(E)
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダ接続時、決済リーダチェック追加)
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
	{UMMENU_CC,	USER_TBL_CC,	USER_MENU_MAX_CC},
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
};

/*[]----------------------------------------------------------------------[]*/
/*| メニューテーブル構築 					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_MenuStrMake( void )	     	      		           |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : 文字列ポインタ                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			                                       |*/
/*| Date         : 2009-06-09                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ushort Is_MenuStrMake( uchar type )
{
	const t_make_menu	*p = &make_menu[type];
	// とりあえず、５０メニュー分確保。動作チェックと共有なので、どちらかのサイズが
	// ５０を超えたら要変更。
	static uchar	MENU_STR[50][31];
	char	i;
	ushort	event;
	
	memset( MENU_STR[0], 0, sizeof( MENU_STR ));
	
	//メニューの項目文字列を作る
	for( i=0;i<p->max;i++ ){
		memcpy(&MENU_STR[i][0], MNT_MENU_NUMBER[i], sizeof(MNT_MENU_NUMBER[0]) );	//番号を入れる
		memcpy(&MENU_STR[i][4], p->str[i], sizeof(p->str[0]) );				//文字列を入れる
	}
	
	event = Menu_Slt( (void*)MENU_STR, p->Ctrl_table, (char)p->max, (char)1 );

	return	event;
}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*|  センター通信処理実施ルーチン										   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Edy_CentComm_sub( uchar disp_type )	                   |*/
///*| PARAMETER    : uchar disp_type（画面表示1:あり／0:なし）               |*/
///*| 			 : uchar comm_type（締め処理1:あり／0:なし）               |*/
///*| RETURN VALUE : void						                               |*/
///*| Date         : 2007-02-26                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//void	Edy_CentComm_sub( uchar disp_type, uchar comm_type )
//{
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 &&			// Edy利用可能状態か？
//		Edy_Rec.edy_EM_err.BIT.Comfail == 0 &&				// EM通信エラーが発生していないか？
//		Edy_Rec.edy_status.BIT.INITIALIZE ) {				// Edyﾓｼﾞｭｰﾙの初期化が完了しているか？
//
//		auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);	// 最終センター通信開始日時更新
//		Edy_SndData13(comm_type);							// センター通信開始指示（締め処理の有無）送信
//		if(disp_type ==1) {
//			dispclr();										// 初期画面「センター通信中」表示
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[0]);					/* "＜Ｔ集計＞　　　　　　　　　　" */
//			grachr(2, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);			/* "　　Ｅｄｙセンターと通信中　　" （点滅表示）*/
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[9]);			/* " 通信中は他の操作ができません " */
//			Fun_Dsp(FUNMSG[0]);																/* "　　　　　　　　　　　　　　　" */
//			edy_cnt_ctrl.BIT.blink_status = 1;				// ブリンクステータスを反転に設定
//			LagTim500ms( LAG500_EDY_LED_RESET_TIMER,		// 点滅用Timerスタート(1sec)
//			 3, Edy_CentComm_Blink );						// EdyLED用のタイマーを使用する
//		}
//		else if(disp_type == 2) {							// 自動センター通信時
//			if( OPECTL.Ope_mod == 0 ){							// 待機画面中は1-3行目の表示を消去
//				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "　　					　　　" */
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "　　					　　　" */
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "　　					　　　" */
//			}
//			grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);				/* "　　Ｅｄｙセンターと通信中　　" */
//			grachr(5, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, EDY_DATECHK_STR[13]);	/* "     しばらくお待ち下さい     " */
//		}
//		edy_cnt_ctrl.BIT.comm_type = comm_type;
//		edy_cnt_ctrl.BIT.disp_type = disp_type;	
//		edy_cnt_ctrl.BIT.exec_status = 1;	
//		Edy_Rec.edy_status.BIT.CENTER_COM_START = 1;		// ｾﾝﾀｰ通信開始		
//	} else {
//		wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ﾓﾆﾀLOG登録（センター通信終了:NG）
//	}
//	
//}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)


/*[]----------------------------------------------------------------------[]*/
/*| アマノ係員カード表示                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AmanoCard_Dsp( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static uchar AmanoCard_Dsp( void )
{
	uchar		gt_ap;
	m_kakari	*crd_data;											// 磁気ｶｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	ulong		atend_no;
	ushort		wk;
	ulong		pk_no;

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];
	gt_ap = MAGred[MAG_GT_APS_TYPE];

	if (MAGred[MAG_GT_APS_TYPE] == 0 ){								//GT未対応リーダーでも新係員カードは読める.
		if ( crd_data->kkr_park[0] == 'P' && crd_data->kkr_park[1] == 'G' && crd_data->kkr_park[2] == 'T' ){ // PGT //
			gt_ap = 1;												//GT//
		}
	}

	/* カード種別 */
	if( crd_data->kkr_type == 0x20 ) {
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[17] );						// "係員カード                    "
	} else if( crd_data->kkr_type == 0x4b ) {
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[18] );						// "釣銭補充カード                "
	} else {
		return(1);													// 以外
	}

	if( gt_ap == 1 ){	//GTフォーマット
		pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +	// 駐車場Ｎｏ．取得
					( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
					( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
					( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
					( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
					( (crd_data->kkr_pno[3] & 0x0f) * 1L )
				);
	}else{				//APSフォーマット
		pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// 駐車場Ｎｏ．取得
					( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
					( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
					( (crd_data->kkr_pno[3] & 0x0f) * 1L )
				);
	}

	for( wk=0 ; wk<=3 ; wk++ ){
		if( (long)pk_no == CPrmSS[S_SYS][1+wk] ){
			grachr( 2, ((crd_data->kkr_type == 0x4b)?14:10), 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）	表示
			break;
		}
	}

	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[gt_ap] );						// "<*>"

	/* 係員NO. */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[15] );		// "  係員No. ：                  "
	atend_no = (( (crd_data->kkr_kno[0] & 0x0f) * 1000)  +						//1000のを取得
				( (crd_data->kkr_kno[1] & 0x0f) * 100 )	 +						//100の桁を取得
				( (crd_data->kkr_kno[2] & 0x0f) * 10 )	 +						//10の桁を取得
				( (crd_data->kkr_kno[3] & 0x0f) * 1 ));							//1の桁を取得
	opedpl( 3, 12, atend_no, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 係員NO 表示

	/* 役割 */
	switch( crd_data->kkr_role ) {
		case '0':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[20] );					// "  役割　  ：初期画面操作      "
			break;
		case '1':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[21] );					// "  役割　  ：係員操作          "
			break;
		case '2':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[22] );					// "  役割　  ：管理者操作        "
			break;
		case '3':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[23] );					// "  役割　  ：技術員操作        "
			break;
		case '4':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[24] );					// "  役割　  ：開発者操作        "
			break;
		default:
			break;
	}

	/* 発行日 */
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[16] );	// "  発行日  ：    年    月    日"
	atend_no = (( (crd_data->kkr_year[0] & 0x0f) * 10) +					//10の桁を取得
				( (crd_data->kkr_year[1] & 0x0f) * 1 ));					//1の桁を取得
	opedpl( 5, 12, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 発行（年）表示
	atend_no = (( (crd_data->kkr_mon[0] & 0x0f) * 10)  +					//10の桁を取得
				( (crd_data->kkr_mon[1] & 0x0f) * 1 ));						//1の桁を取得
	opedpl( 5, 18, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 　　（月）表示
	atend_no = (( (crd_data->kkr_day[0] & 0x0f) * 10)  +					//10の桁を取得
				( (crd_data->kkr_day[1] & 0x0f) * 1 ));						//1の桁を取得
	opedpl( 5, 24, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// 　　（日）表示

	return(0);
}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////[]----------------------------------------------------------------------[]
/////	@brief			センター通信受信処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		type 0:IBK(EM)からの受信 1:分歩進からの呼び出し
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 2008/12/08<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void	Edy_CentComm_Recv( uchar type )
//{
//	R_CENTER_TRAFFIC_RESULT_DATA	*p_rcv_data;		// センター通信実施結果データﾎﾟｲﾝﾀ
//	R_CENTER_TRAFFIC_CHANGE_DATA	*p_rcv_center;		// ｾﾝﾀｰ通信状況変化通知データﾎﾟｲﾝﾀ
//	uchar	LimitTime;									// ｾﾝﾀｰ通信終了Limit時間
//	int		wk_OKosNG=-1;								// ｾﾝﾀｰ通信終了(-1:処理中0:OK/1:NG)
//
//	if( !edy_cnt_ctrl.BIT.exec_status )
//		return;
//	
//	if( type ){											// 分歩進からの呼び出し
//		LimitTime = (uchar)prm_get(COM_PRM, S_SCA, 61, 2, 3);
//
//		if( Edy_Rec.edy_status.BIT.CENTER_COM_START && CneterComLimitTime > LimitTime+5 ){
//			if(edy_cnt_ctrl.BIT.exec_status >= 1) {		// 開始指示以降のみ受付
//				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ﾓﾆﾀLOG登録（センター通信終了:NG）
//				wk_OKosNG = 1;							// ｾﾝﾀｰ通信終了(0:OK/1:NG)
//				goto Edy_CentComm_sub_End;
//			}
//		}
//		return;											// 分歩進からのコールの場合は、電文解析処理はしない
//	}
//	switch( Edy_Rec.rcv_kind ){							// 受信ﾃﾞｰﾀ種別
//
//		case	R_FIRST_STATUS:							// 初期状態通知 受信
//			if(edy_cnt_ctrl.BIT.exec_status >= 1) {			// 開始指示以降のみ受付
//				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ﾓﾆﾀLOG登録（センター通信終了:NG）
//				wk_OKosNG = 1;							// ｾﾝﾀｰ通信終了(0:OK/1:NG)
//			}
//			break;
//
//		case	R_CENTER_TRAFFIC_RESULT:				// ｾﾝﾀｰ通信開始結果 受信
//			p_rcv_data = (R_CENTER_TRAFFIC_RESULT_DATA *)&Edy_Rec.rcv_data;	// 受信ﾃﾞｰﾀﾎﾟｲﾝﾀget
//			if(p_rcv_data->Rcv_Status[0] == 0x00) {		// 正常応答時
//				switch(p_rcv_data->Status) {
//
//					case 0x01:							// 開始指示受付 応答
//						if(edy_cnt_ctrl.BIT.exec_status == 1) {					// 開始指示中のみ受付
//							edy_cnt_ctrl.BIT.exec_status = 2;					// 開始受付済
//						}
//						break;
//
//					case 0x02:							// 通信完了
//						if(edy_cnt_ctrl.BIT.exec_status >= 1) {					// 開始指示以降のみ受付
//							edy_cnt_ctrl.BIT.exec_status = 3;					// 通信終了
//							wmonlg( OPMON_EDY_CEN_STOP, 0, 1 );	// ﾓﾆﾀLOG登録（センター通信終了:OK）
//							wk_OKosNG = 0;						// ｾﾝﾀｰ通信終了(0:OK/1:NG)
//						}
//						break;
//
//					default:
//						break;
//				}
//			} else {									// 異常応答時
//				if(edy_cnt_ctrl.BIT.exec_status >= 1) {							// 開始指示以降のみ受付
//					wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ﾓﾆﾀLOG登録（センター通信終了:NG）
//					wk_OKosNG = 1;							// ｾﾝﾀｰ通信終了(0:OK/1:NG)
//				}
//			}
//			break;
//
//		case	R_CENTER_TRAFFIC_CHANGE:				// ｾﾝﾀｰ通信状況変化通知
//			if(edy_cnt_ctrl.BIT.disp_type !=1) break;
//
//			p_rcv_center = (R_CENTER_TRAFFIC_CHANGE_DATA *)&Edy_Rec.rcv_data;	// 受信ﾃﾞｰﾀﾎﾟｲﾝﾀget
//			if(p_rcv_center->Rcv_Status[0] == 0x00) {
//				switch(p_rcv_center->Status_code) {
//					case 0x01:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[19]);		/* "　　　【データ前処理中】　　　"*/
//						break;
//					case 0x02:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[10]);		/* "　　　【コネクション中】　　　"*/
//						break;
//					case 0x03:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[20]);		/* "　　　【端末ＩＤ送出中】　　　"*/
//						break;
//					case 0x04:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[21]);		/* "　　　【端末認証送出中】　　　"*/
//						break;
//					case 0x05:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[22]);		/* "　　　【セッション開始】　　　"*/
//						break;
//					case 0x06:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[23]);		/* "　　　【データ集信中】  　　　"*/
//						break;
//					case 0x07:
//					case 0x10:
//					case 0x11:
//					case 0x12:
//					case 0x13:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[24]);		/* "　　　【データ配信中】  　　　"*/
//						break;
//					case 0x08:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[25]);		/* "　　　【セッション終了】　　　"*/
//						break;
//					case 0x09:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[26]);		/* "　　　【クローズ処理中】　　　"*/
//						break;
//					case 0x0A:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[27]);		/* "　　　【データ後処理中】　　　"*/
//						break;
//					case 0x0B:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[28]);		/* "　　　【ＤＨＣＰ処理中】　　　"*/
//						break;
//					default:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "　　　　              　　　　"*/
//						break;
//				}
//			}
//			break;
//
//		default:
//			break;
//	}
//	
//Edy_CentComm_sub_End:
//	if( wk_OKosNG != -1 ){
//		if( edy_cnt_ctrl.BIT.disp_type == 1 ){
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	// 点滅表示用タイマー開放
//				dispclr();										// 画面表示消去
//		}
//
//		if(edy_cnt_ctrl.BIT.disp_type == 2) {
//			if( OPECTL.Ope_mod == 0 ){						// 待機画面中で自動センター通信を行った場合
//				if( Edy_Rec.edy_status.BIT.ZAN_SW ){		// 残額照会中の場合
//					Edy_StopAndLedOff();					// Edyの停止＆LED_OFF
//					Edy_Rec.edy_status.BIT.ZAN_SW = 0;		// 残高照会ﾎﾞﾀﾝ押下ﾌﾗｸﾞｾｯﾄ
//				}
//				dispclr();									// 画面表示消去 
//				OpeLcd( 1 );								// 待機画面再表示
//				Op_Event_enable();
//
//			}else if( OPECTL.Ope_mod == 100 ){
//				dispclr();									// 画面表示消去 
//				OpeLcd( 17 );								// 休業中画面再表示
//				Op_Event_enable();
//			}else{											// それ以外（休業中）
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);
//			}
//		}
//
//		if(edy_cnt_ctrl.BIT.comm_type == 1) {				// 「Ｅｄｙ締め」ありでセンター通信を実施した
//			if(edy_cnt_ctrl.BIT.disp_type == 2){ 			// 「自動」Ｔ合計時
//				Edy_Shime_Pri(1, (uchar)wk_OKosNG);			// ログ保存＆ﾚｼｰﾄ印字処理
//			} else {										// 「手動」Ｔ合計時
//				Edy_Shime_Pri(0, (uchar)wk_OKosNG);			// ログ保存＆ﾚｼｰﾄ印字処理
//			}
//		}
//
//		Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// ｾﾝﾀｰ通信終了情報ｾｯﾄ
//		CneterComLimitTime = 0;
//		edy_cnt_ctrl.SHORT = 0;								// センター通信フラグクリア
//		
//	}
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			センター通信中のブリンク表示切替
////[]----------------------------------------------------------------------[]
/////	@param[in]		None
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 2008/12/08<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void	Edy_CentComm_Blink( void )
//{
//	if( edy_cnt_ctrl.BIT.disp_type ){
//		if(edy_cnt_ctrl.BIT.exec_status == 2) {				// 通信中は点滅表示を実施する
//			edy_cnt_ctrl.BIT.blink_status ^= 1;
//			grachr(2, 0, 30, (ushort)edy_cnt_ctrl.BIT.blink_status, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);		/* "　　Ｅｄｙセンターと通信中　　" */
//			LagTim500ms( LAG500_EDY_LED_RESET_TIMER,		// 点滅用Timerスタート(1sec)
//			 3, Edy_CentComm_Blink );						// EdyLED用のタイマーを使用する
//		}
//	}
//}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

//[]----------------------------------------------------------------------[]
///	@brief			プリンタチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind : 対象プリンタ
///	@return			1 = OK
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CheckPrinter(uchar pri_kind)
{
	switch(pri_kind) {
	case	R_PRI:
		if ((Ope_isPrinterReady() && Pri_Open_Status_R == 0)
			&& !IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {	// レシートプリンタ通信不良
			return 1;
		}
		break;
	case	RJ_PRI:
		if ((! Ope_isPrinterReady() || Pri_Open_Status_R != 0)
			|| IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {	// レシートプリンタ通信不良
			break;
		}
	// not break
	case	J_PRI:
		if ((Ope_isJPrinterReady() && Pri_Open_Status_J == 0)
// MH810104 GG119201(S) 電子ジャーナル対応
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)				// 電子ジャーナル通信不良
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810104 GG119201(S) 電子ジャーナル対応（E2204発生時にレシートにT合計が印字されない）
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE)
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR)
// MH810104 GG119201(E) 電子ジャーナル対応（E2204発生時にレシートにT合計が印字されない）
			&& !IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM)) {	// ジャーナルプリンタ通信不良
			return 1;
		}
		break;
	default:
//	case	0:
		return 1;		// 印字指定なしは常にOK
		break;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			プリント結果チェック（サブ）
//[]----------------------------------------------------------------------[]
///	@param[in]		no : 対象プリンタ（0/1）
///	@return			PRI_NML_END／PRI_CSL_END／PRI_ERR_END
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	int	check_print_result(int no)
{
	int		ret;
	ret = OPECTL.PriEndMsg[no].BMode;
	if (ret == PRI_ERR_END) {
		if (OPECTL.PriEndMsg[no].BStat == PRI_ERR_STAT) {
//			if ((OPECTL.PriEndMsg[no].BPrinStat & 0x08) == 0) {
//			// ヘッド温度異常以外は紙切れと見なす
			if (OPECTL.PriEndMsg[no].BPrinStat & 0x0e) {
			// ニアエンド以外は紙切れとする
				ret = PRI_NO_PAPER;
			}
		}
	}
	return ret;
}

// MH810100(S) Y.Yamauchi 2019/12/04 車番チケットレス(メンテナンス)
//[]----------------------------------------------------------------------[]
///	@brief			文字の桁数を測る
//[]----------------------------------------------------------------------[]
///	@return			文字の桁数
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/14<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort get_digit( ulong data )
{
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//	ushort length = 1;
//
//	while( data != 0){
//		data = data / 10;
//		++length;
//	}
	ushort length = 0;

	do {
		data = data / 10;
		++length;
	} while ( data != 0 );
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善

	return length;
}
//[]----------------------------------------------------------------------[]
///	@brief			QRデータ更新処理
//[]----------------------------------------------------------------------[]
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/14<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void push_ticket( tMediaDetail *MediaDetail, ushort err )
{
	// 最新データを前回データにする
	ticketdata.backdata[4] = ticketdata.backdata[3];			// 過去から5番目のデータを過去から4番目のデータにコピー
	ticketdata.backdata[3] = ticketdata.backdata[2];			// 過去から4番目のデータを過去から3番目のデータにコピー
	ticketdata.backdata[2] = ticketdata.backdata[1];			// 過去から3番目のデータを過去から2番目のデータにコピー
	ticketdata.backdata[1] = ticketdata.backdata[0];			// 最新データを前回データにコピー

	// 最新データに今読んだQRデータをセット
	memcpy( &ticketdata.backdata[0].QR_Data, (uchar*)&MediaDetail->Barcode.QR_data, sizeof(ticketdata.backdata[0].QR_Data) );

	ticketdata.backdata[0].id = MediaDetail->Barcode.id;		// データIDをセット(10000=QR買上券 / 10001=QR割引券)
	ticketdata.backdata[0].err = err;							// エラーをセット
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ticketdata.backdata[0].type = MediaDetail->Barcode.qr_type;	// QRフォーマットタイプをセット(1=標準 / 2=個別共通)
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）

	if( ticketdata.cnt < 5 ){
		ticketdata.cnt++;	// データ件数更新
	}
}
// MH810100(S) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
//[]----------------------------------------------------------------------[]
///	@brief			QR エラー表示
//[]----------------------------------------------------------------------[]
///	@param[in]		result		: QR読取結果応答
///	@return			ret         :エラーの表示
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// static uchar	Lcd_QR_ErrDisp( ushort result )
uchar Lcd_QR_ErrDisp( ushort result )
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
{
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:533)対応
//	uchar ret;
	uchar ret = RESULT_NO_ERROR;
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:533)対応

	switch ( result )
	{
	case 0x04:
// MH810100(S) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
		ret = RESULT_BAR_FORMAT_ERR;	// フォーマットエラー
// MH810100(E) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
		break;
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	case 0x01:
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	case 0x05:
		ret = RESULT_BAR_ID_ERR;		// 対象外
		break;
	default:
		break;
	}
	return ret;
}

// MH810100(E) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
//[]----------------------------------------------------------------------[]
///	@brief			QR ID取得
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: 何件目のデータか
///	@return			ID情報
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static ushort	pop_ticketID( int no )
{
	return ticketdata.backdata[no].id;
}

//[]----------------------------------------------------------------------[]
///	@brief			QRデータエラー取得
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: 何件目のデータか
///	@return			エラー情報
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort	pop_ticketErr( int no )
{
	return ticketdata.backdata[no].err;
}
//[]----------------------------------------------------------------------[]
///	@brief			QRデータ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: 何件目のデータか
///	@return			QRデータ情報
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void*	pop_ticketData( int no )
{
	return &ticketdata.backdata[no].QR_Data;
}

// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//[]----------------------------------------------------------------------[]
///	@brief			QRフォーマットタイプ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: 何件目のデータか
///	@return			QRフォーマットタイプ
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2023/01/10<br>
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
static ushort	pop_ticketType( int no )
{
	return ticketdata.backdata[no].type;
}
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
///[]----------------------------------------------------------------------[]
/// @brief        QRエラーデータ表示  
//[]----------------------------------------------------------------------[]
/// @param[in]	 :  int page : 何枚目のデータ
///					int err  : エラー内容
//[]----------------------------------------------------------------------[]
/// @return      : void
//[]----------------------------------------------------------------------[]
/// @date    	 Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// static void  UsMnt_QR_ErrDisp( int err )
void UsMnt_QR_ErrDisp( int err )
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
{
	switch ( err )
	{
	case RESULT_BAR_USED:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[1] );	// " 　＜＜使用済バーコード＞＞　 "
		break;

	case RESULT_BAR_READ_MAX:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[2] );	// "　　　 ＜＜枚数上限＞＞ 　　　"
		break;

	case RESULT_BAR_EXPIRED:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[3] );	//"　　＜＜有効期限範囲外＞＞　　"
		break;

// ↓↓QRのフォーマットが正しくないため、中身を表示させない

	case RESULT_BAR_FORMAT_ERR:
// MH810100(S) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//		grachr( 3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[4] );	// " 　　　フォーマット不正　　　 "
		grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[4]);	// " 　＜＜フォーマット不正＞＞　 "
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[19]);			// "フォーマットが正しくないため　"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[21]);			// "内容を表示できません　　　　　"
		dispmlclr(4, 1);
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[22]);			// "読み取ったＱＲコードか設定に　"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);				// "問題が無いか確認して下さい　　"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// MH810100(E) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
		break;

	case RESULT_BAR_ID_ERR:
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//		grachr( 3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[5] );	// " 　　　対象外バーコード　　　 "
		grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[5]);	// " 　＜＜対象外バーコード＞＞　 "
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[20]);			// "対象外のＱＲコードを読んだため"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[21]);			// "内容を表示できません　　　　　"
		dispmlclr(4, 1);
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[22]);			// "読み取ったＱＲコードか設定に　"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);				// "問題が無いか確認して下さい　　"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
		break;
	
	default:
		break;
	}

}
///[]----------------------------------------------------------------------[]
/// @brief          QRデータ表示
//[]----------------------------------------------------------------------[]
///	@param[in]	slide_page		: 横移動
///	@param[in]	updown_page		: 上下移動
///	@param[in]	slide_pagemax	: 横移動最大ページ数
///	@param[in]	updown_pagemax	: 上下移動最大ページ数
//[]----------------------------------------------------------------------[]
/// @return      : MOD_CHG : mode change
///              : MOD_EXT : F5 key<br>
//[]----------------------------------------------------------------------[]
/// @date    	 Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// static void UsMnt_QR_DataDisp( ushort slide_page, ushort updown_page , ushort slide_pagemax)
static void UsMnt_QR_DataDisp( ushort slide_page, ushort updown_page , ushort slide_pagemax, ushort updown_pagemax)
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
{
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//	uchar	i;							// 表示用のカウンタ
//	ushort	max = 0;
//	uchar	top = (updown_page * 5);	// １ページの最大表示数
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
	QR_AmountInfo*	 pAmntQR = NULL;	// QRデータ情報 QR買上券
	QR_DiscountInfo* pDisQR = NULL;		// QRデータ情報 QR割引券
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	QR_TicketInfo*	 pTicQR = NULL;		// QRデータ情報 QR駐車券
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//	uchar	temp[10];					// バイナリからAsciiに変換用
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
	uchar	err;						// エラー表示用
	uchar	size;
	uchar	len;
	uchar	pos;
	ushort	_slide_page = slide_page + 1;
// MH810100(S) 2020/09/17 #4869【QRデータ確認画面で、駐車場ナンバーが表示されている
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//	ushort  wk1 = 0;
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// MH810100(E) 2020/09/17 #4869【QRデータ確認画面で、駐車場ナンバーが表示されている

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	dispmlclr( 1, 6 );
//	// 割引券？
//	if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page)) ){
//		pDisQR = (QR_DiscountInfo*)pop_ticketData( slide_page );
//	}
//	// 買上券？
//	else{
//		pAmntQR = (QR_AmountInfo*)pop_ticketData( slide_page );
//	}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

	// エラー取得
	err = pop_ticketErr( slide_page );
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	if ( err != RESULT_NO_ERROR || err != RESULT_QR_INQUIRYING ){
//		UsMnt_QR_ErrDisp( err );
//	}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_TITLE[0] );	// 	"ＱＲデータ確認　　　　　　　　"	
	size = get_digit(( ulong ) slide_pagemax );
	len = get_digit(( ulong ) _slide_page );
	pos = 30 - (size*2);
	opedsp( 0, pos, slide_pagemax, size, 0, 0, COLOR_BLACK, 0);
	grachr( 0,(pos -2), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "／" );
	opedsp( 0, ((pos -2) -(len*2)), _slide_page, len, 0, 0, COLOR_BLACK, 0);
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	dispmlclr(1, 6);
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//	Fun_Dsp(FUNMSG[124]);													// 	"  ▲    ▼    ⊂    ⊃   終了 ",
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない

	if( err != RESULT_BAR_ID_ERR && err!= RESULT_BAR_FORMAT_ERR ){				// 対象外,フォーマット不正のID以外の場合
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//		for( i = 0; i < 5 ; i++ ){
//			// 割引券
//			if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page))){
//// MH810100(S) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
////				max = QR_DISCOUNT_DATA_MAX - 1;
//				max = QR_DISCOUNT_DATA_MAX;
//// MH810100(E) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
//				if( max > top + i ){
//					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[top+i] );	// 3行目から各項目表示
//				}
//			}
//			// 買上券
//			else{
//				max = QR_AMOUNT_HEAD + 1;
//				max += (pAmntQR->DataCount * (BAR_DATA_OFFSET+1));
//				if( max > top + i ){
//					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[top+i] );		// 3行目から各項目表示
//				}
//			}
//		}
//
//		switch( pop_ticketID(slide_page) ){
//			// QR買上券
//			case BAR_ID_AMOUNT:
//				switch( updown_page ){
//					case 0:
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Year,4);	// 発行年
//						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Mon,2);	// 発行月
//						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Day,2);	// 発行日
//						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Hour,2);	// 発行時
//						grachr(  3, 18, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Min,2);	// 発行分
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Sec,2);	// 発行秒
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						opedpl3( 4, (30-(get_digit( pAmntQR->FacilityCode ))), pAmntQR->FacilityCode, get_digit( pAmntQR->FacilityCode ), 0, 0,COLOR_BLACK,0 );	// 施設コード
//						opedpl3( 5, (30- get_digit( pAmntQR->ShopCode )), pAmntQR->ShopCode, get_digit( pAmntQR->ShopCode ) , 0, 0,COLOR_BLACK,0 ); // 店舗コード
//						opedpl3( 6, (30-(get_digit( pAmntQR->PosNo ))), pAmntQR->PosNo, get_digit( pAmntQR->PosNo ), 0, 0,COLOR_BLACK,0 );	// 店舗内端末番号
//						break;
//					case 1:
//						opedpl3( 2, (30- get_digit( pAmntQR->IssueNo )), pAmntQR->IssueNo, get_digit( pAmntQR->IssueNo ) , 0, 0,COLOR_BLACK,0 );	// レシート発行追番
//						if ( (ulong)pAmntQR->DataCount == 1 ){
//							opedpl3( 3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0,COLOR_BLACK,0 );	// バーコードタイプ１
//							opedpl3( 4, (30-(get_digit( pAmntQR->BarData[0].data ))), (ulong)pAmntQR->BarData[0].data, get_digit( pAmntQR->BarData[0].data ), 0, 0,COLOR_BLACK,0 );	// データ１
//						} else {
//							opedpl3( 3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0,COLOR_BLACK,0 );	// バーコードタイプ１
//							opedpl3( 4, (30-( get_digit( pAmntQR->BarData[0].data ))), (ulong)pAmntQR->BarData[0].data, get_digit( pAmntQR->BarData[0].data ), 0, 0,COLOR_BLACK,0 );	// データ１
//							opedpl3( 5, 28, (ulong)pAmntQR->BarData[1].type, 2, 0, 0,COLOR_BLACK,0 );	// バーコードタイプ２
//							opedpl3( 6, (30-(get_digit( pAmntQR->BarData[1].data))), (ulong)pAmntQR->BarData[1].data, get_digit( pAmntQR->BarData[1].data), 0, 0,COLOR_BLACK,0 );	// データ２
//						}
//						break;
//					case 2:
//						if( (ulong)pAmntQR->DataCount == 3 ){
//							opedpl3( 2, 28, (ulong)pAmntQR->BarData[2].type, 2, 0, 0,COLOR_BLACK,0 );	// バーコードタイプ３
//							opedpl3( 3, (30-(get_digit( pAmntQR->BarData[2].data ))), (ulong)pAmntQR->BarData[2].data, get_digit( pAmntQR->BarData[2].data ), 0, 0,COLOR_BLACK,0 );	// データ３
//						}
//						break;
//					default:
//						break;
//				}
//				break;
//
//			// QR割引券
//			case BAR_ID_DISCOUNT:
//				switch( updown_page ){
//					case 0:
//						// TODO :ベタうちではない方法を探す
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Year,4);		// 発行年
//						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Mon,2);		// 発行月
//						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Day,2);		// 発行日
//						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(S) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
////						opedpl3( 3, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 駐車場番号
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 割引種別
////						opedpl3( 5, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // 割引区分
////						opedpl3( 3, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 駐車場番号
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 割引種別
////						opedpl3( 5, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // 割引区分
////						opedpl3( 6, (30-(get_digit( pDisQR->ShopNp ))), pDisQR->ShopNp, get_digit( pDisQR->ShopNp ), 0, 0,COLOR_BLACK,0 );	// 店番号
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Hour,2);	// 発行時
//						grachr(  3, 18, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Min,2);	// 発行分
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Sec,2);	// 発行秒
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(S) 2020/09/17 #4869【QRデータ確認画面で、駐車場ナンバーが表示されている
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 駐車場番号
//						for( wk1=0 ; wk1<=3 ; wk1++ ){
//							if( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+wk1] ){
//								break;
//							}
//						}
//						grachr( 4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３／  ）	表示
//// MH810100(E) 2020/09/17 #4869【QRデータ確認画面で、駐車場ナンバーが表示されている
//						opedpl3( 5, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// 割引種別
//						opedpl3( 6, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // 割引区分
//// MH810100(E) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
//						break;
//					case 1:
//// MH810100(S) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Year,4);		// 有効開始年
////						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Mon,2);		// 有効開始月
////						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Day,2);		// 有効開始日
////						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Year,4);		// 有効終了年
////						grachr(  3, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Mon,2);		// 有効終了月
////						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Day,2);		// 有効終了日
////						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						opedpl3( 2, (30-(get_digit( pDisQR->ShopNp ))), pDisQR->ShopNp, get_digit( pDisQR->ShopNp ), 0, 0,COLOR_BLACK,0 );	// 店番号
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Year,4);		// 有効開始年
//						grachr(  3, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Mon,2);		// 有効開始月
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Day,2);		// 有効開始日
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Year,4);		// 有効終了年
//						grachr(  4, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Mon,2);		// 有効終了月
//						grachr(  4, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Day,2);		// 有効終了日
//						grachr(  4, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(E) S.Takahashi 2020/02/14 割引券に発行時分秒を追加
//// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//						opedpl3( 5, (30-(get_digit( pDisQR->FacilityCode ))), pDisQR->FacilityCode, get_digit( pDisQR->FacilityCode ), 0, 0,COLOR_BLACK,0 );	// 施設コード
//						opedpl3( 6, (30- get_digit( pDisQR->ShopCode )), pDisQR->ShopCode, get_digit( pDisQR->ShopCode ) , 0, 0,COLOR_BLACK,0 );				// 店舗コード
//// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//						break;
//// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//					case 2:
//						opedpl3( 2, (30-(get_digit( pDisQR->PosNo ))), pDisQR->PosNo, get_digit( pDisQR->PosNo ), 0, 0,COLOR_BLACK,0 );			// 店舗内端末番号
//						opedpl3( 3, (30- get_digit( pDisQR->IssueNo )), pDisQR->IssueNo, get_digit( pDisQR->IssueNo ) , 0, 0,COLOR_BLACK,0 );	// レシート発行追番
//						break;
//// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//					default:
//						break;
//				}
//				break;
//
//			default:
//				break;
//		}
		switch( pop_ticketID(slide_page) ) {
		case BAR_ID_DISCOUNT:	// 割引券
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
			pDisQR = (QR_DiscountInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[1]);	// "：割引券"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//			UsMnt_QR_DispDisQR(pDisQR, updown_page);
			UsMnt_QR_DispDisQR(pDisQR, updown_page, pop_ticketType(slide_page));
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
			break;
		case BAR_ID_AMOUNT:		// 買上券
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
			pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[0]);	// "：買上券"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//			UsMnt_QR_DispAmntQR(pAmntQR, updown_page);
			UsMnt_QR_DispAmntQR(pAmntQR, updown_page, pop_ticketType(slide_page));
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
			break;
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
		case BAR_ID_TICKET:		// 駐車券
			pTicQR = (QR_TicketInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[2]);	// "：駐車券"
			UsMnt_QR_DispTicQR(pTicQR, updown_page);
			break;
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
		default:
			break;
		}
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
	}

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	UsMnt_QR_ErrDisp(err);
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
	if ( (pDisQR != NULL) || (pAmntQR != NULL) ) {	// 割引券、買上券
		if ( updown_page == 0 ) {
			Fun_Dsp(FUNMSG[126]);											// "        ▼    ⊂    ⊃   終了 "
		} else if ( updown_page >= updown_pagemax ) {
			Fun_Dsp(FUNMSG[127]);											// "  ▲          ⊂    ⊃   終了 "
		} else {
			Fun_Dsp(FUNMSG[124]);											// "  ▲    ▼    ⊂    ⊃   終了 "
		}
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	} else if ( pTicQR != NULL ) {	// 駐車券
		// 表示内容は1ページのみ
		Fun_Dsp(FUNMSG[128]);												// "              ⊂    ⊃   終了 "
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	} else {										// エラー
		Fun_Dsp(FUNMSG[128]);												// "              ⊂    ⊃   終了 "
	}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
}
///[]----------------------------------------------------------------------[]
/// @brief	上下移動のページの最大数
///[]----------------------------------------------------------------------[]
///	@param[in]	slide_page		: 横移動
/// @return 	updown_page_max	: 上下移動のページの最大数
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// ushort	UsMnt_QR_CardChk( QR_AmountInfo*  pAmntQR, ushort slide_page)
ushort UsMnt_QR_CardChk(ushort slide_page)
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
{
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//	ushort updown_pagemax;
//
//	if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page))){
//// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
////		updown_pagemax = 1;
//		updown_pagemax = 2;		// ページ数拡張(2->3)
//// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//	} else {
//		pAmntQR = (QR_AmountInfo*)pop_ticketData( slide_page );
//		if( pAmntQR->DataCount == 3 ){
//			updown_pagemax = 2;
//		} else {
//			updown_pagemax = 1;
//		}
//	}
//	return updown_pagemax;
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//	pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
	QR_AmountInfo *pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
	return UsMnt_QR_GetUpdownPageMax(pop_ticketID(slide_page), pAmntQR);
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
}

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//[]----------------------------------------------------------------------[]
/// @brief		数字表示
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: 表示行
///	@param[in]	col			: 表示列
///	@param[in]	dat			: データ
///	@param[in]	siz			: サイズ
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispNum(ushort low, ushort col, ulong dat, ushort siz)
{
	opedpl3(low, col, dat, siz, 1, 0, COLOR_BLACK, 0);
}

//[]----------------------------------------------------------------------[]
/// @brief		数字表示(右寄せ)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: 表示行
///	@param[in]	dat			: データ
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispNumRight(ushort low, ulong dat)
{
	ushort siz = get_digit(dat);
	opedpl3(low, (30-siz), dat, siz, 0, 0, COLOR_BLACK, 0);
}

//[]----------------------------------------------------------------------[]
/// @brief		QR割引券の表示(割引種別)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: 表示行
///	@param[in]	pDisQR		: QRデータ(QR割引券)
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispDisQR_DiscKind(ushort low, QR_DiscountInfo *pDisQR)
{
	switch ( pDisQR->DiscKind ) {
	case 1:		// サービス券(金額)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[0]);
		break;
	case 2:		// 店割引(金額)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[1]);
		break;
	case 3:		// 多店舗割引(金額)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[2]);
		break;
	case 4:		// 割引券(金額)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[3]);
		break;
	case 50:	// 種別割引
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[4]);
		break;
	case 101:	// サービス券(時間)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[5]);
		break;
	case 102:	// 店割引(時間)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[6]);
		break;
	case 103:	// 多店舗割引(時間)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[7]);
		break;
	case 104:	// 割引券(時間)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[8]);
		break;
	case 150:	// 使用禁止
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[9]);
		break;
	default:	// その他
		// 数値をそのまま表示
		UsMnt_QR_DispNumRight(low, pDisQR->DiscKind);
		break;
	}
}

//[]----------------------------------------------------------------------[]
/// @brief		QR割引券の表示(割引区分)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: 表示行
///	@param[in]	pDisQR		: QRデータ(QR割引券)
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispDisQR_DiscClass(ushort low, QR_DiscountInfo *pDisQR)
{
	int is_disp_num = 0;

	switch ( pDisQR->DiscKind ) {
	case 1:		// サービス券(金額)
	case 50:	// 種別割引
	case 101:	// サービス券(時間)
		if ( pDisQR->DiscClass < _countof(DAT2_7) ) {
			grachr(low, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[pDisQR->DiscClass]);
		} else {
			is_disp_num = 1;	// 数値をそのまま表示
		}
		break;
	case 2:		// 店割引(金額)
	case 102:	// 店割引(時間)
		switch ( pDisQR->DiscClass ) {
		case 0:		// 店割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[0]);
			break;
		case 1:		// 使用禁止
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[1]);
			break;
		case 2:		// テナント割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[2]);
			break;
		case 3:		// 所有者割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[3]);
			break;
		case 4:		// 初期無料（一律）
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[4]);
			break;
		case 5:		// 車種割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[5]);
			break;
		case 6:		// 発券位置割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[6]);
			break;
		case 7:		// 認証機割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[7]);
			break;
		case 100:	// システム障害時割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[8]);
			break;
		case 101:	// 出庫延長割引
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[9]);
			break;
		default:	// その他
			is_disp_num = 1;	// 数値をそのまま表示
			break;
		}
		break;
	case 3:		// 多店舗割引(金額)
	case 4:		// 割引券(金額)
	case 103:	// 多店舗割引(時間)
	case 104:	// 割引券(時間)
	case 150:	// 使用禁止
	default:	// その他
		is_disp_num = 1;	// 数値をそのまま表示
		break;
	}
	
	if ( is_disp_num ) {
		// 数値をそのまま表示
		UsMnt_QR_DispNumRight(low, pDisQR->DiscClass);
	}
}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//[]----------------------------------------------------------------------[]
/// @brief		QR割引券の表示
//[]----------------------------------------------------------------------[]
///	@param[in]	pDisQR		: QRデータ(QR割引券)
///	@param[in]	updown_page	: 上下移動のページ
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page)
void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page, ushort qrtype)
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
{
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	ushort i;
//	ushort top;
//	ushort max;
//	uchar temp[10];
//
//	top = updown_page * 5;
//	max = QR_DISCOUNT_DATA_MAX;
//
//	// 3行目から各項目表示
//	for( i = 0; i < 5 ; i++ ) {
//		if ( max > top + i ) {
//			grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,
//				QR_DISCOUNT_DATA_CHECK_STR[top+i]);
//		}
//	}
//
//	switch( updown_page ) {
//	case 0:
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Year, 4);		// 発行年
//		grachr(2, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Mon, 2);			// 発行月
//		grachr(2, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Day, 2);			// 発行日
//		grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Hour, 2);		// 発行時
//		grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Min, 2);			// 発行分
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Sec, 2);			// 発行秒
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		for ( i = 0; i <= 3; i++ ) {
//			if ( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
//				break;
//			}
//		}
//		grachr(4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// 駐車場（基本／拡１／拡２／拡３／　　）
//		opedpl3(5, (30-(get_digit(pDisQR->DiscKind))), pDisQR->DiscKind,
//			get_digit(pDisQR->DiscKind), 0, 0, COLOR_BLACK, 0);			// 割引種別
//		opedpl3(6, (30-(get_digit(pDisQR->DiscClass))), pDisQR->DiscClass,
//			get_digit(pDisQR->DiscClass), 0, 0, COLOR_BLACK, 0);		// 割引区分
//		break;
//	case 1:
//		opedpl3(2, (30-(get_digit(pDisQR->ShopNp))), pDisQR->ShopNp,
//			get_digit(pDisQR->ShopNp), 0, 0, COLOR_BLACK, 0);			// 店番号
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Year, 4);		// 有効開始年
//		grachr(3, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Mon, 2);			// 有効開始月
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Day, 2);			// 有効開始日
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Year, 4);			// 有効終了年
//		grachr(4, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Mon, 2);			// 有効終了月
//		grachr(4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Day, 2);			// 有効終了日
//		grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		opedpl3(5, (30-(get_digit(pDisQR->FacilityCode))), pDisQR->FacilityCode,
//			get_digit(pDisQR->FacilityCode), 0, 0, COLOR_BLACK, 0);		// 施設コード
//		opedpl3(6, (30-get_digit(pDisQR->ShopCode)), pDisQR->ShopCode,
//			get_digit(pDisQR->ShopCode), 0, 0, COLOR_BLACK, 0);			// 店舗コード
//		break;
//	case 2:
//		opedpl3(2, (30-(get_digit(pDisQR->PosNo))), pDisQR->PosNo,
//			get_digit(pDisQR->PosNo), 0, 0, COLOR_BLACK, 0);			// 店舗内端末番号
//		opedpl3(3, (30-get_digit(pDisQR->IssueNo)), pDisQR->IssueNo,
//			get_digit(pDisQR->IssueNo), 0, 0, COLOR_BLACK, 0);			// レシート発行追番
//		break;
//	default:
//		break;
//	}
	ushort i;
	ushort j;
	ushort size = _countof(QR_DISCOUNT_DATA_CHECK_STR);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 7) && (j < size); i++, j++ ) {	// 1行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNum(2, 11, pDisQR->IssueDate.Year, 4);		// 発行年
		UsMnt_QR_DispNum(2, 16, pDisQR->IssueDate.Mon, 2);		// 発行月
		UsMnt_QR_DispNum(2, 19, pDisQR->IssueDate.Day, 2);		// 発行日
		UsMnt_QR_DispNum(2, 22, pDisQR->IssueDate.Hour, 2);		// 発行時
		UsMnt_QR_DispNum(2, 25, pDisQR->IssueDate.Min, 2);		// 発行分
		UsMnt_QR_DispNum(2, 28, pDisQR->IssueDate.Sec, 2);		// 発行秒
		UsMnt_QR_DispNumRight(3, pDisQR->FacilityCode);			// 施設コード
		UsMnt_QR_DispNumRight(4, pDisQR->ShopCode);				// 店舗コード
		UsMnt_QR_DispNumRight(5, pDisQR->PosNo);				// 店舗内端末番号
		UsMnt_QR_DispNumRight(6, pDisQR->IssueNo);				// レシート発行追番
		break;
	case 1:
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		if( qrtype != QR_FORMAT_CUSTOM ) {
		// 標準フォーマット
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		for( i = 2, j = 5; (i < 7) && (j < size); i++, j++ ) {	// 6行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		for ( i = 0; i <= 3; i++ ) {
			if ( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
				break;
			}
		}
		grachr(2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// 駐車場（基本／拡１／拡２／拡３／　　）
		UsMnt_QR_DispDisQR_DiscKind(3, pDisQR);							// 割引種別
		UsMnt_QR_DispDisQR_DiscClass(4, pDisQR);						// 割引区分
		UsMnt_QR_DispNumRight(5, pDisQR->ShopNp);						// 店番号
		UsMnt_QR_DispNum(6, 9, pDisQR->StartDate.Year, 4);				// 有効開始年
		UsMnt_QR_DispNum(6, 14, pDisQR->StartDate.Mon, 2);				// 有効開始月
		UsMnt_QR_DispNum(6, 17, pDisQR->StartDate.Day, 2);				// 有効開始日
		UsMnt_QR_DispNum(6, 20, pDisQR->EndDate.Year, 4);				// 有効終了年
		UsMnt_QR_DispNum(6, 25, pDisQR->EndDate.Mon, 2);				// 有効終了月
		UsMnt_QR_DispNum(6, 28, pDisQR->EndDate.Day, 2);				// 有効終了日
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		} else {
		// 個別共通フォーマット
		for( i = 2, j = 6; (i < 5) && (j < size); i++, j++ ) {	// 6行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispDisQR_DiscKind(2, pDisQR);							// 割引種別
		UsMnt_QR_DispDisQR_DiscClass(3, pDisQR);						// 割引区分
		UsMnt_QR_DispNumRight(4, pDisQR->ShopNp);						// 店番号
		}
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		break;
	default:
		break;
	}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
}

//[]----------------------------------------------------------------------[]
/// @brief		QR買上券の表示
//[]----------------------------------------------------------------------[]
///	@param[in]	pAmntQR		: QRデータ(QR買上券)
///	@param[in]	updown_page	: 上下移動のページ
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page)
void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page, ushort qrtype)
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
{
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	ushort i;
//	ushort top;
//	ushort max;
//	uchar temp[10];
//
//	top = updown_page * 5;
//	max = QR_AMOUNT_HEAD + 1;
//	max += (pAmntQR->DataCount * (BAR_DATA_OFFSET + 1));
//
//	// 3行目から各項目表示
//	for( i = 0; i < 5 ; i++ ) {
//		if ( max > top + i ) {
//			grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[top+i]);
//		}
//	}
//
//	switch( updown_page ) {
//	case 0:
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Year, 4);		// 発行年
//		grachr(2, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Mon, 2);		// 発行月
//		grachr(2, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Day, 2);		// 発行日
//		grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Hour, 2);		// 発行時
//		grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Min, 2);		// 発行分
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Sec, 2);		// 発行秒
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		opedpl3(4, (30-(get_digit(pAmntQR->FacilityCode))), pAmntQR->FacilityCode,
//			get_digit(pAmntQR->FacilityCode), 0, 0, COLOR_BLACK, 0);	// 施設コード
//		opedpl3(5, (30-get_digit(pAmntQR->ShopCode)), pAmntQR->ShopCode,
//			get_digit(pAmntQR->ShopCode), 0, 0, COLOR_BLACK, 0);		// 店舗コード
//		opedpl3(6, (30-(get_digit(pAmntQR->PosNo))), pAmntQR->PosNo,
//			get_digit(pAmntQR->PosNo), 0, 0, COLOR_BLACK, 0);			// 店舗内端末番号
//		break;
//	case 1:
//		opedpl3(2, (30-get_digit(pAmntQR->IssueNo)), pAmntQR->IssueNo,
//			get_digit(pAmntQR->IssueNo), 0, 0, COLOR_BLACK, 0);			// レシート発行追番
//		if ( (ulong)pAmntQR->DataCount == 1 ) {
//			opedpl3(3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0, COLOR_BLACK, 0);	// バーコードタイプ１
//			opedpl3(4, (30-(get_digit(pAmntQR->BarData[0].data))), (ulong)pAmntQR->BarData[0].data,
//				get_digit(pAmntQR->BarData[0].data), 0, 0, COLOR_BLACK, 0);				// データ１
//		} else {
//			opedpl3(3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0, COLOR_BLACK, 0);	// バーコードタイプ１
//			opedpl3(4, (30-(get_digit(pAmntQR->BarData[0].data))), (ulong)pAmntQR->BarData[0].data,
//				get_digit(pAmntQR->BarData[0].data), 0, 0, COLOR_BLACK, 0);				// データ１
//			opedpl3(5, 28, (ulong)pAmntQR->BarData[1].type, 2, 0, 0, COLOR_BLACK, 0);	// バーコードタイプ２
//			opedpl3(6, (30-(get_digit(pAmntQR->BarData[1].data))), (ulong)pAmntQR->BarData[1].data,
//				get_digit(pAmntQR->BarData[1].data), 0, 0, COLOR_BLACK, 0);				// データ２
//		}
//		break;
//	case 2:
//		if ( (ulong)pAmntQR->DataCount == 3 ) {
//			opedpl3(2, 28, (ulong)pAmntQR->BarData[2].type, 2, 0, 0, COLOR_BLACK, 0);	// バーコードタイプ３
//			opedpl3(3, (30-(get_digit(pAmntQR->BarData[2].data))), (ulong)pAmntQR->BarData[2].data,
//				get_digit(pAmntQR->BarData[2].data), 0, 0, COLOR_BLACK, 0);				// データ３
//		}
//		break;
//	default:
//		break;
//	}
	ushort i;
	ushort j;
	ushort size = _countof(QR_AMOUNT_DATA_CHECK_STR) - (BAR_DATA_OFFSET * 3) + (BAR_DATA_OFFSET * pAmntQR->DataCount);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 7) && (j < size); i++, j++ ) {	// 1行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNum(2, 11, pAmntQR->IssueDate.Year, 4);	// 発行年
		UsMnt_QR_DispNum(2, 16, pAmntQR->IssueDate.Mon, 2);		// 発行月
		UsMnt_QR_DispNum(2, 19, pAmntQR->IssueDate.Day, 2);		// 発行日
		UsMnt_QR_DispNum(2, 22, pAmntQR->IssueDate.Hour, 2);	// 発行時
		UsMnt_QR_DispNum(2, 25, pAmntQR->IssueDate.Min, 2);		// 発行分
		UsMnt_QR_DispNum(2, 28, pAmntQR->IssueDate.Sec, 2);		// 発行秒
		UsMnt_QR_DispNumRight(3, pAmntQR->FacilityCode);		// 施設コード
		UsMnt_QR_DispNumRight(4, pAmntQR->ShopCode);			// 店舗コード
		UsMnt_QR_DispNumRight(5, pAmntQR->PosNo);				// 店舗内端末番号
		UsMnt_QR_DispNumRight(6, pAmntQR->IssueNo);				// レシート発行追番
		break;
	case 1:
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		if( qrtype != QR_FORMAT_CUSTOM ) {
		// 標準フォーマット
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		for( i = 2, j = 5; (i < 7) && (j < size); i++, j++ ) {	// 6行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->DataCount);			// データ数
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[0].type);		// バーコードタイプ１
		UsMnt_QR_DispNumRight(4, pAmntQR->BarData[0].data);		// データ１
		if ( pAmntQR->DataCount >= 2 ) {
			UsMnt_QR_DispNumRight(5, pAmntQR->BarData[1].type);	// バーコードタイプ２
			UsMnt_QR_DispNumRight(6, pAmntQR->BarData[1].data);	// データ２
		}
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		} else {
		// 個別共通フォーマット
		for( i = 2, j = 6; (i < 4) && (j < size); i++, j++ ) {	// 6行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->BarData[0].type);		// バーコードタイプ１
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[0].data);		// データ１
		}
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
		break;
	case 2:
		for( i = 2, j = 10; (i < 7) && (j < size); i++, j++ ) {	// 11行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->BarData[2].type);	// バーコードタイプ３
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[2].data);	// データ３
		break;
	default:
		break;
	}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
}

// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
//[]----------------------------------------------------------------------[]
/// @brief		QR駐車券の表示
//[]----------------------------------------------------------------------[]
///	@param[in]	pDisQR		: QRデータ(QR駐車券)
///	@param[in]	updown_page	: 上下移動のページ
/// @return 	無し
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2023/01/05
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void UsMnt_QR_DispTicQR(QR_TicketInfo *pTicQR, ushort updown_page)
{
	ushort i;
	ushort j;
	ushort size = _countof(QR_TICKET_DATA_CHECK_STR);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 6) && (j < size); i++, j++ ) {	// 1行目から
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TICKET_DATA_CHECK_STR[j]);
		}
		for ( i = 0; i <= 3; i++ ) {
			if ( (long)pTicQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
				break;
			}
		}
		UsMnt_QR_DispNum(2, 11, pTicQR->IssueDate.Year, 4);				// 発行年
		UsMnt_QR_DispNum(2, 16, pTicQR->IssueDate.Mon, 2);				// 発行月
		UsMnt_QR_DispNum(2, 19, pTicQR->IssueDate.Day, 2);				// 発行日
		UsMnt_QR_DispNum(2, 22, pTicQR->IssueDate.Hour, 2);				// 発行時
		UsMnt_QR_DispNum(2, 25, pTicQR->IssueDate.Min, 2);				// 発行分
		UsMnt_QR_DispNum(2, 28, pTicQR->IssueDate.Sec, 2);				// 発行秒
		grachr(3, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// 駐車場（基本／拡１／拡２／拡３／　　）
		UsMnt_QR_DispNumRight(4, pTicQR->EntryMachineNo);				// 発券機番号
		UsMnt_QR_DispNumRight(5, pTicQR->ParkingTicketNo);				// 駐車券番号
		break;
	default:
		break;
	}
}
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）

//[]----------------------------------------------------------------------[]
/// @brief		上下移動のページの最大数
//[]----------------------------------------------------------------------[]
///	@param[in]	id				: QRコードID
///	@param[in]	pAmntQR			: QRデータ(QR買上券)
/// @return 	updown_page_max	: 上下移動のページの最大数
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ushort UsMnt_QR_GetUpdownPageMax(ushort id, QR_AmountInfo *pAmntQR)
{
	ushort updown_page_max;

	if( id == BAR_ID_DISCOUNT ) {	// 割引券
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//		updown_page_max = 2;
		updown_page_max = 1;
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	} else if( id == BAR_ID_TICKET ) {	// 駐車券
		updown_page_max = 0;
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	} else {						// 買上券
		if( pAmntQR->DataCount == 3 ){
			updown_page_max = 2;
		} else {
			updown_page_max = 1;
		}
	}

	return updown_page_max;
}
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善

///[]----------------------------------------------------------------------[]
/// @brief		バーコード可能チェック
///[]----------------------------------------------------------------------[]
/// @param		: tMediaDetail *MediaDetail : QRデータ情報
//[]-----------------------------------------------------------------------[]
///	@return		: RESULT_NO_ERROR : 使用可能なバーコード
///				 RESULT_BAR_FORMAT_ERR：フォーマットエラー<br>
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static OPE_RESULT UsMnt_UseBarcodeChk( tMediaDetail *MediaDetail )
{
	// 非対応バーコード
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
//	if( MediaDetail->Barcode.id != BAR_ID_AMOUNT && MediaDetail->Barcode.id != BAR_ID_DISCOUNT ){
	if( MediaDetail->Barcode.id != BAR_ID_AMOUNT && 
		MediaDetail->Barcode.id != BAR_ID_DISCOUNT &&
		MediaDetail->Barcode.id != BAR_ID_TICKET ){
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
		return RESULT_BAR_ID_ERR;	// 対象外
	}

	return RESULT_NO_ERROR;
}
//[]----------------------------------------------------------------------[]
///	@brief			バーコード解析
//[]----------------------------------------------------------------------[]
/// @param		: tMediaDetail *MediaDetail : QRデータ情報
//[]----------------------------------------------------------------------[]
///	@return			TRUE : 解析成功
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL UsMnt_ParseBarcodeData( tMediaDetail *MediaDetail )
{
	ushort	row_size = 0;
	lcdbm_rsp_QR_rd_rslt_t *pQR = &LcdRecv.lcdbm_rsp_QR_rd_rslt;

	// LCDで解析済みのため, 解析データをコピー
	MediaDetail->Barcode.id = pQR->id;					// QRコードID
	MediaDetail->Barcode.rev = pQR->rev;					// QRコードフォーマットRev.
	MediaDetail->Barcode.enc_type = pQR->enc_type;		// QRコードエンコードタイプ
	MediaDetail->Barcode.info_size = pQR->info_size;		// QRデータ情報(パースデータ)サイズ
	memcpy( &MediaDetail->Barcode.QR_data, &pQR->QR_data, sizeof(MediaDetail->Barcode.QR_data) );	// QRパースデータ
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	MediaDetail->Barcode.qr_type = pQR->qr_type;		// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）

	// 生データコピー
	row_size = pQR->data_size;
	if( row_size > BAR_DATASIZE ){
		row_size = BAR_DATASIZE;
	}
	memcpy( MediaDetail->RowData, pQR->data, row_size );
	return TRUE;
}

///[]----------------------------------------------------------------------[]
/// @brief ユーザーメンテナンス：QRデータ確認                                    
///[]----------------------------------------------------------------------[]
/// @param 	     : void
///[]----------------------------------------------------------------------[]
/// @return      : MOD_CHG : mode change
///                MOD_EXT : F5 key 
///                MOD_CUT : LCD_DISCONNECT
///[]----------------------------------------------------------------------[]
/// @date        : 2020.1.13                                               
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort	UsMnt_QR_DataCkk( void )
{
	ushort msg;
	ushort updown_page = 0;	// 上下移動
	ushort	Ret_updown_pagemax;	// 上下移動最大ページ数
	ushort slide_page = 0;	// 横移動
	ushort slide_pagemax = ticketdata.cnt; // 横移動の最大ページ数
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//	QR_AmountInfo*	 pAmntQR;			// QRデータ情報 QR買上券
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
	ushort result;
	int mode = 0;							// 0 = 通常、1 = データなし、

	dispclr();

	// タイトル表示
	if( slide_pagemax < 1){
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_TITLE[0] );	// 	"ＱＲデータ確認　　　　　　　　",
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[0] );	// 	" 　　データはありません　　 ",
		Fun_Dsp(FUNMSG[8]);															// 	"                         終了 ",
		mode = 1;																	// F５以外操作させない。
	}else {
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//		UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
		result = pop_ticketErr(slide_page);
		Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// 上下移動のページの最大数
		UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
		mode = 0;
	}

	// 読取開始要求
// MH810100(S) Y.Yamauchi 2020/03/16 車番チケットレス (#4037 QRデータ確認画面に遷移後、音が鳴ってしまう)
//	if( FALSE == PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) == FALSE ){
//		// 失敗時
//		BUZPIPI();
//	 }
	if( FALSE == PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) ){
		// 失敗時
		BUZPIPI();
	}
// MH810100(E) Y.Yamauchi 2020/03/16 車番チケットレス (#4037 QRデータ確認画面に遷移後、音が鳴ってしまう)

	while( 1 ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){

			// 操作モード切替
			case LCD_DISCONNECT:
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
// MH810100(S) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				return MOD_CUT;
				break;

			case KEY_MODECHG:
				BUZPI();
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
// MH810100(S) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				return MOD_CHG;
				break;

			// 「▲」
			case KEY_TEN_F1:
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//				if(mode == 0){
//					BUZPI();
//					if( slide_pagemax >= 1){
//						Ret_updown_pagemax = UsMnt_QR_CardChk( pAmntQR, slide_page);	// 上下移動のページの最大数
//						if( updown_page == 0 ){
//							updown_page = Ret_updown_pagemax;
//						}else{
//							updown_page--;
//						}
//
//						UsMnt_QR_DataDisp( slide_page, updown_page , slide_pagemax);
//					}
//				}
// GG124100(S) R.Endo 2022/09/08 車番チケットレス3.0 #6577 ユーザーメンテナンスのQRデータ確認で、使用済バーコードの情報が一部しか表示されない [共通改善項目 No1529]
// 				if ( (mode == 0) && (result == RESULT_NO_ERROR) && (updown_page > 0) ) {
				if ( (mode == 0) &&							// データあり
					 (result != RESULT_BAR_FORMAT_ERR) &&	// フォーマットエラー
					 (result != RESULT_BAR_ID_ERR) &&		// 対象外
					 (updown_page > 0) ) {					// 上に移動できない
// GG124100(E) R.Endo 2022/09/08 車番チケットレス3.0 #6577 ユーザーメンテナンスのQRデータ確認で、使用済バーコードの情報が一部しか表示されない [共通改善項目 No1529]
					BUZPI();
					updown_page--;
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
				}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
				break;

			// 「▼」
			case KEY_TEN_F2:
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//				if(mode == 0){
//					BUZPI();
//					if( slide_pagemax >= 1){
//						Ret_updown_pagemax = UsMnt_QR_CardChk( pAmntQR, slide_page);	// 上下移動のページの最大数
//						updown_page++;
//						if( updown_page > Ret_updown_pagemax ){
//							updown_page = 0;
//						}
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
//					}
//				}
// GG124100(S) R.Endo 2022/09/08 車番チケットレス3.0 #6577 ユーザーメンテナンスのQRデータ確認で、使用済バーコードの情報が一部しか表示されない [共通改善項目 No1529]
// 				if ( (mode == 0) && (result == RESULT_NO_ERROR) && (updown_page < Ret_updown_pagemax) ) {
				if ( (mode == 0) &&							// データあり
					 (result != RESULT_BAR_FORMAT_ERR) &&	// フォーマットエラー
					 (result != RESULT_BAR_ID_ERR) &&		// 対象外
					 (updown_page < Ret_updown_pagemax) ) {	// 下に移動できない
// GG124100(E) R.Endo 2022/09/08 車番チケットレス3.0 #6577 ユーザーメンテナンスのQRデータ確認で、使用済バーコードの情報が一部しか表示されない [共通改善項目 No1529]
					BUZPI();
					updown_page++;
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
				}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
				break;

			// 「＜」
			case KEY_TEN_F3:
				if(mode == 0){
					BUZPI();
					if( slide_pagemax > 1){
						if( slide_page == 0 ){
							slide_page = slide_pagemax - 1;
						}else{
							slide_page--;
						}
						updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
						result = pop_ticketErr(slide_page);
						Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// 上下移動のページの最大数
						UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
					}
				}
				break;

			// 「＞」
			case KEY_TEN_F4:
				if(mode == 0){
					BUZPI();
					if( slide_pagemax > 1){
						slide_page++;
						if( slide_page >= slide_pagemax ){
							slide_page = 0;
						}
						updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
						result = pop_ticketErr(slide_page);
						Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// 上下移動のページの最大数
						UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
					}
				}
				break;

			// 「終了」
			case KEY_TEN_F5:
// MH810100(S) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
				BUZPI();
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
				return MOD_EXT;

			// 読取結果
			case LCD_MNT_QR_READ_RESULT:
				// QRデータのサイズが０のものは無視
// MH810100(S) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
//				if( LcdRecv.lcdbm_rsp_QR_rd_rslt.result != 0x09 ) {
				if ( LcdRecv.lcdbm_rsp_QR_rd_rslt.result != 0x01 ) {	// 異常 以外
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// MH810100(E) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
					if( LcdRecv.lcdbm_rsp_QR_rd_rslt.result == 0x00 ){
						// 正常
						if( UsMnt_ParseBarcodeData( &MediaDetail ) ){			// バーコード解析	
							result = UsMnt_UseBarcodeChk( &MediaDetail );		// バーコード可能チェック
// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//							if( result == RESULT_NO_ERROR ){
//								updown_page = 0;								// 正常データの場合、ページ位置を初期化
//							}
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
						}
					}
// MH810100(S) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
					else {	
						// フォーマット不正,対象外
						result = Lcd_QR_ErrDisp( LcdRecv.lcdbm_rsp_QR_rd_rslt.result );
					}
// MH810100(E) Y.Yamauchi 2020/03/25 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
					push_ticket( &MediaDetail, result);			// QRデータ更新処理
					slide_pagemax = ticketdata.cnt;				// ページ枚数更新
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//					BUZPI();
//					UsMnt_QR_DataDisp( 0, 0, slide_pagemax );
					if ( result == RESULT_NO_ERROR ) {
						BUZPI();
					} else {
						BUZPIPI();
					}
					slide_page = 0;
					updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax);
					Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// 上下移動のページの最大数
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
					mode = 0;
// MH810100(S) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
//					PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STA );
					Lagtim(OPETCBNO, 6, 100);		/* 2sec timer start */
// MH810100(E) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
				} else {												// 異常
					// 異常の場合はQRリーダーとの通信不良を想定してディレイを10秒に増やす。
					// 読み取った内容のエラーではないためQRデータ更新処理や表示処理は行わない。
					Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// MH810100(S) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
				}
// MH810100(E) Y.Yamauchi 2020/03/30 車番チケットレス(#4068で対象外QRを読み込んでも画面に遷移しない)
				break;
// MH810100(S) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
			case TIMEOUT6:
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STA );
				break;
// MH810100(E) 2020/07/17 #4546【連動評価指摘事項】【連動評価指摘事項】QRコード確認（メンテナンス）の読み取り間隔が短い
			default:
				break;
		}
	}
}
// MH810100(E) Yamauchi 2019/12/04 車番チケットレス(メンテナンス)