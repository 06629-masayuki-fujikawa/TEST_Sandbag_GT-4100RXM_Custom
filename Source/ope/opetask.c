/*[]----------------------------------------------------------------------[]*/
/*| operation task control                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<machine.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"irq1.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"mnt_def.h"
#include	"AppServ.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"LKmain.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"tbl_rkn.h"
#include	"ifm_ctrl.h"
#include	"ope_ifm.h"
#include	"ntnetauto.h"

#include	"remote_dl.h"
#include	"can_api.h"
#include	"can_def.h"
#include	"rtc_readwrite.h"
#include	"I2c_driver.h"
#include	"bluetooth.h"
#include	"updateope.h"
#include	"strdef.h"
#include	"pip_def.h"
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"ntcom.h"
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"FlashSerial.h"
#include	"ftpctrl.h"
#include	"ifm_ctrl.h"
#include	"oiban.h"
#include	"rau.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応

static void start_dsp( void );

static short DateTimeCheck( void );
static void	start_up_LD1( void );

extern	unsigned short	UsMnt_mnyctl( void );
extern	unsigned short	UsMnt_mnyctl2( void );
extern	void 	Ether_DTinit(void);
// MH810100(S) K.Onodera  2019/12/25 車番チケットレス(起動通知受信をOPEタスク初期化完了まで待つ)
extern void PktStartUpStsChg( uchar faze );		// 起動完了状態更新
// MH810100(E) K.Onodera  2019/12/25 車番チケットレス(起動通知受信をOPEタスク初期化完了まで待つ)
// MH810100(S) K.Onodera  2020/01/14 車番チケットレス(LCDリセット通知対応)
extern void PktResetReqFlgSet( uchar rst_req );	// リセット要求フラグセット
// MH810100(E) K.Onodera  2020/01/14 車番チケットレス(LCDリセット通知対応)

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:5;				// Bit 3-7 = 予備
		unsigned char	START_STATUS:1;		// Bit 2 = 起動時のﾄﾞｱの開閉状態
		unsigned char	TIMER_END:1;		// Bit 1 = マスクタイマータイムアウト
		unsigned char	MASK:1;				// Bit 0 = 振動センサーマスク中
	} BIT;
} t_mask_VIB_SNS;

t_mask_VIB_SNS	mask_VIB_SNS;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//static	char	auto_cntcom_phase_check( uchar* );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

void	ck_syuukei_ptr_zai( ushort LogKind );
extern	uchar	LogDataClr( ushort LogSyu );
#define	_RTC_INT_ENB()	{\
	P_INTC.ISCR.BIT.IRQ2SC = 2;				/* IRQ2 立ち上がりedge */		\
	P_INTC.IER.BIT.IRQ2E = 1;				/* IRQ2 割込み許可 */			\
	wait2us( 5L );							/* wait 10us */					\
	if( P_INTC.ISR.BIT.IRQ2F ){				\
		P_INTC.ISR.BIT.IRQ2F = 0; 			/* IRQ2 Inservice flg clear */	\
	}										\
}
ushort	DoorLockTimer;
ulong	attend_no;
static uchar	StartupDoorArmDisable = 0;

static uchar Printer_Status_Get( uchar Type, uchar *NEW, uchar *OLD );
static void I2C_Err_Regist( unsigned short msg, unsigned char ErrData );
static	t_OpeKeyRepeat	OpeKeyRepeat;			// 初期値OFF

void	Ope_KeyRepeat();
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//static	void	SoundInsProgram( void );
static	void		lcdbm_LcdIpPort_DefaultSetting( void );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// MH810100(S) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
//static	void	SoundPlayProgram( void );
// MH810100(E) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
uchar	DisableDoorKnobChime = 0;		// ドアノブ戻し忘れチャイム抑止フラグ
static uchar chk_fan_timectrl( void );
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
uchar Is_in_lagtim( void );		// 精算時間 + ﾗｸﾞﾀｲﾑ時間 >= 現在時間のﾁｪｯｸ
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
static	void	kinko_syuukei(void);
unsigned char	Pri_Open_Status_R;			// プリンタカバー状態
unsigned char	Pri_Open_Status_J;			// プリンタカバー状態
// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
//static	void	log_clear_all(void);
static	void	log_clear_all(uchar factor);
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// アイドルサブタスクへ移動
//// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
//static void LongTermParkingCheck( void );
//static short LongTermParking_stay_hour_check(flp_com *p, short hours);
//void	LongTermParkingCheck_Resend( void );
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
extern uchar	KSG_RauBkupAPNName[32];// APNの設定値の比較用バックアップ FOMAモジュールに設定済みのデータか比較するためのもの
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
// MH810103 GG119202(S) 起動シーケンス不具合修正
int		ModuleStartFlag;
// MH810103 GG119202(E) 起動シーケンス不具合修正

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝﾀｽｸﾒｲﾝ                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opetask( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void opetask( void )
{
	unsigned char	f_ErrAlm=0;										// b0:1=E0057(時計読み込み失敗)発生
	char	LOCKCLR = 0;											// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
																	// b1:1=E0058(機器情報読み込み失敗)発生
	char	f_CPrmDefaultSet=0;										// 1=共通ﾊﾟﾗﾒｰﾀ defaultｾｯﾄ起動
	uchar i;
	uchar log_ver_temp = 0;
	ushort interval;
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	uchar chg_info_ver_temp = 0;
// GG120600(e) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
// MH810105(s) 2022/11/07 K.A (レシートプリンタ通信ができない対策)
	ushort	delay;
// MH810105(e) 2022/11/07 K.A (レシートプリンタ通信ができない対策)

	uchar	remote_timer_set;
	uchar	f_parm_datchk;
	uchar	f_log_init = 0;// 1=ログのみイニシャル実施,2=スーパーイニシャル実施(操作モニタ登録判定用)
// MH810103 GG119202(S) 起動シーケンス不具合修正
	ModuleStartFlag = 1;
// MH810103 GG119202(E) 起動シーケンス不具合修正
	dog_init();														// Watch dog timer start

	dog_init();

	/*** ﾒｯｾｰｼﾞｸﾘｱ, 外部入力初期値読み取り, 外部出力初期化 ***/
	Ptout_Init();													// 出力ﾎﾟｰﾄ初期化
	ExIOSignalwt(EXPORT_JP_RES, 1);									// ジャーナルプリンタリセット
	ExIOSignalwt(EXPORT_URES, 1);									// ジャーナルプリンタリセット	

	ExIOSignalwt(EXPORT_M_LD0, 1 );									// 通常：H（デジタルアンプゲイン：0）設定

	Mcb_Init();														// Message buffer initial
	Tcb_Init();
	f_port_scan_OK = 0;												// 入力ポートの割り込みスキャン禁止

	wait2us(50000L);												// wait 100ms
	CP_CAN_RES = 0;													// CAN Reset 解除
	CP_FB_RES = 1;													// FB-7000 Reset 解除
	wait2us(50000L);												// wait 100ms
	dog_init();
	CAN1_init();													// 機器内通信ポート初期化
	/*** LED, LCD OFF ***/
	LedInit();														// LED initial(All OFF)
	dispinit();														// LCD initial
// MH810105(s) 2022/11/07 K.A (レシートプリンタ通信ができない対策)
	delay = 30;														// ３秒
	while(delay != 0) {
		wait2us(50000L);											// wait 100ms
		dog_init();
		delay--;
	}
// MH810105(e) 2022/11/07 K.A (レシートプリンタ通信ができない対策)

	/*** 立上り表示 ***/
	start_dsp();

	/*** ﾒﾓﾘｸﾘｱ ***/
	prm_init( COM_PRM );											// 共通ﾊﾟﾗﾒｰﾀの格納ｱﾄﾞﾚｽを取得
	prm_init( PEC_PRM );											// 個別ﾊﾟﾗﾒｰﾀの格納ｱﾄﾞﾚｽを取得

	StartingFunctionKey = 0;
	I2C1_init();												
	Rspi_from_Init();
	Flash2Init();
// 起動直後に音声データの有無を判定し、LED点滅にて知らせたいために音声データ運用面更新処理は先に行なう
	if( read_rotsw() == 4 ){										// ----- スーパーイニシャライズ(RSW=4)? -----
		// 運用面のイニシャライズ処理は先にやっておく
		update_sw_area_init();										// 運用面情報初期化処理
	}
	get_from_swdata();												// FROMの運用面情報を取得しRAMに展開する
	wave_data_swupdate();											// 音声データ運用面更新処理
	f_wave_datchk = wave_data_swchk();								// 音声データ運用面チェック処理
	start_up_LD1();													// 起動時のプログラム、音声有無のLED点滅処理

	log_ver_temp = FLT_read_log_version();
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	chg_info_ver_temp = FLT_read_chg_info_version();
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	
// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
//	if( (LOG_VERSION != log_ver_temp) ||// ログバージョン不一致
//	    (memcmp( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM )) != 0) ){// パスワードNG
//		// FROMに格納されているログバージョンをSRAMに格納する
//		bk_log_ver = log_ver_temp;
//		// プログラムのログバージョンをFROMに格納する
//		FLT_write_log_version(LOG_VERSION);
//		// ログパスワードセット
//		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );
//
//		// バックアップデータの構造体のみサイズ変更した場合、
//		// case文にログバージョンを追加し、ログ初期化を行わないようにすること
//		switch (bk_log_ver) {
////		case XX:
////			break;
//		default:
//			log_clear_all();
//			f_log_init = 1;											// ログイニシャル実施
//			break;
//		}
	if( memcmp( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM )) != 0 ){// パスワードNG
		// ログの管理エリアが壊れているので無条件に初期化する
		// FROMに格納されているログバージョンをSRAMに格納する
		bk_log_ver = log_ver_temp;
		// プログラムのログバージョンをFROMに格納する
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//		FLT_write_log_version(LOG_VERSION);
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
		// ログパスワードセット
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );
		log_clear_all(1);	// パスワードNGによるクリア
		f_log_init = 1;											// ログイニシャル実施
	}else if( LOG_VERSION != log_ver_temp ){// ログバージョン不一致
		// FROMに格納されているログバージョンをSRAMに格納する
		bk_log_ver = log_ver_temp;
		// プログラムのログバージョンをFROMに格納する
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//		FLT_write_log_version(LOG_VERSION);
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
		// ログパスワードセット
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );

		// バックアップデータの構造体のみサイズ変更した場合、
		// case文にログバージョンを追加し、ログ初期化を行わないようにすること
		switch (bk_log_ver) {
		default:
			log_clear_all(0);	// ログバージョン不一致によるクリア
			f_log_init = 1;											// ログイニシャル実施
			break;
		}
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
// MH810103(s) バージョンアップ後１回目の復電時にしかリストアできない不具合修正
//	}else if( bk_log_ver < 5 && log_ver_temp < 5){
//		// ログバージョンが5未満では、bk_log_verは0のはずだが、FROMバージョンとLOG_VERSIONが一致しているのに、5未満の場合はセットし直す。
//		// ログバージョンが5以上は、bk_log_verに正しい値が入っているはずである。
//		bk_log_ver = log_ver_temp;
// MH810103(e) バージョンアップ後１回目の復電時にしかリストアできない不具合修正
	} 
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	if( CHG_INFO_VERSION != chg_info_ver_temp ){// chg_infoバージョン不一致)
		switch (chg_info_ver_temp) {
		case 0xFF:// GG120600より前が0xFF
			remotedl_chg_info_log0_to1();
			break;
		default:
			break;
		}
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
	}
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	TENKEY_F1 = 0;
	TENKEY_F2 = 0;
	TENKEY_F3 = 0;
	TENKEY_F4 = 0;
	TENKEY_F5 = 0;
	switch (read_rotsw()) {
	case 1:		// メモリーイニシャル
		TENKEY_F1 = 1;
		TENKEY_F3 = 1;
		break;
	case 2:		// パラメータデフォルトセット
		TENKEY_F2 = 1;
		TENKEY_F4 = 1;
		break;
	case 3:		// メモリーイニシャル＋パラメータデフォルトセット
		TENKEY_F3 = 1;
		TENKEY_F5 = 1;
		break;
// MH810100(S) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
//	case 7:
//		OPECTL.Mnt_mod = 5;
//		break;
//	case 8:		// ﾛｯｸ装置状態ﾘｽﾄｱ要求(F1&F2&F3 ON)?   AppServ_FukudenProc()で必要
//		TENKEY_F1 = 1;
//		TENKEY_F2 = 1;
//		TENKEY_F3 = 1;
//		break;
// MH810100(E) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
	default:
		break;
	}
	if( remotedl_update_chk() || 									// ----- プログラム更新後？ -----
		(( TENKEY_F1 == 1 )&&( TENKEY_F3 == 1 )) ){					// ----- RAMｸﾘｱ要求１(F1&F3 ON)? -----
		Log_Write_Pon();// ログ管理復電処理(全体)
		FLT_WriteLog_Pon();// ログ管理復電処理(FROMの書き込み部分のみ)
		StartingFunctionKey = 2;
		RAMCLR = memclr( 1 );										// SRAM初期化（無条件ｸﾘｱ）
		prm_clr( PEC_PRM, 1, 0 );									// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		LOCKCLR = 1;												// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
	}
	else if( ( TENKEY_F2 == 1 )&&( TENKEY_F4 == 1 ) ){				// ----- RAMｸﾘｱ要求２(F2&F4 ON)? -----
		Log_Write_Pon();// ログ管理復電処理(全体)
		FLT_WriteLog_Pon();// ログ管理復電処理(FROMの書き込み部分のみ)
		StartingFunctionKey = 3;
		NtNet_FukudenParam.DataKind = 0;
		prm_clr( COM_PRM, 1, 0 );									// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		RAMCLR = memclr( 0 );										// SRAM初期化（条件付きｸﾘｱ）
		lockinfo_clr(1);											// 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		LOCKCLR = 1;												// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ﾊﾟﾗﾒｰﾀｰﾃﾞﾌｫﾙﾄｾｯﾄ       "表示
		f_CPrmDefaultSet = 1;										// 1=共通ﾊﾟﾗﾒｰﾀ defaultｾｯﾄ
		f_ParaUpdate.BYTE = 0;										// ﾃﾞﾌｫﾙﾄｾｯﾄするので、フラグをクリア
	}
	else if( ( TENKEY_F3 == 1 )&&( TENKEY_F5 == 1 ) ){				// ----- RAMｸﾘｱ要求３(F3&F5 ON)? -----
		Log_Write_Pon();// ログ管理復電処理(全体)
		FLT_WriteLog_Pon();// ログ管理復電処理(FROMの書き込み部分のみ)
		StartingFunctionKey = 1;
		NtNet_FukudenParam.DataKind = 0;
		log_init();
		prm_clr( COM_PRM, 1, 0 );									// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		prm_clr( PEC_PRM, 1, 0 );									// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		RAMCLR = memclr( 1 );										// SRAM初期化（無条件ｸﾘｱ）
		lockinfo_clr(1);											// 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		LOCKCLR = 1;												// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ﾊﾟﾗﾒｰﾀｰﾃﾞﾌｫﾙﾄｾｯﾄ       "表示
		f_CPrmDefaultSet = 1;										// 1=共通ﾊﾟﾗﾒｰﾀ defaultｾｯﾄ
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// デフォルトセットログ登録処理
		f_ParaUpdate.BYTE = 0;										// ﾃﾞﾌｫﾙﾄｾｯﾄするので、フラグをクリア
	}
	else if( read_rotsw() == 4 ){				// ----- スーパーイニシャライズ(RSW=4)? -----
		Flash2ChipErase();											// FROM全領域消去
		for( i = 0; i < eLOG_MAX; i++ ){
			memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
		}
		Log_clear_log_bakupflag();// LOGデータ登録復電処理用のフラグ、退避データクリア処理
		for( i = 0; i < eLOG_MAX; i++ ){
			BR_LOG_SIZE[i] = LogDatMax[i][0];
		}
		FLT_req_tbl_clear();// ログサイズ情報書き込みの前に要求をクリアする必要がある
		FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ログサイズ情報書き込み
		// FLT_WriteBRLOGSZ()で_flt_EnableTaskChg()がコールされ、タスクチェンジの発生を抑止する
		_flt_DisableTaskChg();
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//		FLT_write_log_version(LOG_VERSION);							// プログラムのログバージョンをFROMに格納
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
		f_log_init = 2;												// スーパーイニシャル実施(操作モニタ登録判定用)
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );	// ログパスワードセット
		StartingFunctionKey = 1;
		NtNet_FukudenParam.DataKind = 0;

		log_init();
		prm_clr( COM_PRM, 1, 0 );									// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		prm_clr( PEC_PRM, 1, 0 );									// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		RAMCLR = memclr( 1 );										// SRAM初期化（無条件ｸﾘｱ）
		lockinfo_clr(1);											// 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		LOCKCLR = 1;												// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ﾊﾟﾗﾒｰﾀｰﾃﾞﾌｫﾙﾄｾｯﾄ       "表示
		f_CPrmDefaultSet = 1;										// 1=共通ﾊﾟﾗﾒｰﾀ defaultｾｯﾄ
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// デフォルトセットログ登録処理
		f_ParaUpdate.BYTE = 0;										// ﾃﾞﾌｫﾙﾄｾｯﾄするので、フラグをクリア
	}
	else if( read_rotsw() == 5 ){
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// rotsw=5でLCD通信設定のﾃﾞｨﾌｫﾙﾄ設定に関連する処理を追加
//		SoundInsProgram();

		// ログ管理復電処理(全体)
		Log_Write_Pon();

		// ログ管理復電処理(FROMの書き込み部分のみ)
		FLT_WriteLog_Pon();

		// SRAM初期化（条件付きｸﾘｱ）
		RAMCLR = memclr( 0 );

		// LCD通信設定のﾃﾞｨﾌｫﾙﾄ(RomTable = cprm_rec)対応
		lcdbm_LcdIpPort_DefaultSetting();

// MH810100(S) K.Onodera 2020/03/10 車番チケットレス (#4014 RSW=5が設定更新情報印字に記録されない)
		SetSetDiff( SETDIFFLOG_SYU_IP_DEF );	// IPデフォルトセットログ登録処理
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス (#4014 RSW=5が設定更新情報印字に記録されない)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	}
// MH810100(S) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
//	else if( read_rotsw() == 6 ){
//		SoundPlayProgram();
//	}
// MH810100(E) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
	else if( read_rotsw() == 9 ){
		Log_Write_Pon();// ログ管理復電処理(全体)
		FLT_WriteLog_Pon();// ログ管理復電処理(FROMの書き込み部分のみ)
		for(i = 0 ;i < FAN_EXE_BUF ;i++){							// FAN駆動ログの初期化
			fan_exe_time.fan_exe[i].year = 0;
			fan_exe_time.fan_exe[i].mont = 0;
			fan_exe_time.fan_exe[i].date = 0;
			fan_exe_time.fan_exe[i].hour = 0;
			fan_exe_time.fan_exe[i].minu = 0;
			fan_exe_time.fan_exe[i].f_exe = 0;
			fan_exe_time.index = 0;
		}
	}
	else{															// ----- 通常立上げ時 -----
		Log_Write_Pon();// ログ管理復電処理(全体)
		FLT_WriteLog_Pon();// ログ管理復電処理(FROMの書き込み部分のみ)
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		NTNET_UpdateParam(&NtNet_FukudenParam);
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		RAMCLR = memclr( 0 );										// SRAM初期化（条件付きｸﾘｱ）
	}
	clr_req = 0;
	if (FLAGPT.memflg != _MEMFLG_VAL || RAMCLR) {
		FLAGPT.memflg = _MEMFLG_VAL;
		clr_req = _CLR_ALLMEM;
	}
	parm_data_swupdate();											// 共通パラメータ運用面更新処理
	f_parm_datchk = parm_data_swchk();								// 共通パラメータ運用面チェック処理

	if( RAMCLR ){
		grachr( 6, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[47] );	// "          ﾒﾓﾘｰｲﾆｼｬﾙ           "表示
	}

	dog_init();
	Ether_DTinit();
	CLK_REC_msec = 0;		// 時計処理の前にクリア
	_ei();															// enable interrupts

	AppServ_FukudenProc( RAMCLR, f_CPrmDefaultSet );				// FlashRom制御系 復電処理
	IFM_Init((int)RAMCLR);
// Note:ログの取得中にタスク切り換えが行われると、ログが登録される可能性がある
// 		タスク切り替えをせずにFlashにアクセスするため、flataskの_flt_DisableTaskChg()を使用
	_flt_DisableTaskChg();					// タスク切り替え 禁止
	IFM_RcdBufClrAll();						// データテーブルの初期化
	IFM_Accumulate_Payment_Init();			// 精算データ初期作成
	IFM_Accumulate_Error_Init();			// エラーデータ初期作成
	_flt_EnableTaskChg();					// タスク切り替え 解除

	NTNET_Init((uchar)RAMCLR);										// NT-NETｴﾘｱｲﾆｼｬﾙ
	ntautoInit((uchar)RAMCLR);
	ope_imf_Init();
// MH322914(S) K.Onodera 2016/09/12 AI-V対応：振替精算
	PiP_FurikaeInit();
// MH322914(E) K.Onodera 2016/09/12 AI-V対応：振替精算
	if (prm_get(COM_PRM, S_SYS, 80, 1, 6) != 0) {
		OPECTL.Seisan_Chk_mod = ON;									// 精算ﾁｪｯｸﾓｰﾄﾞON
	}
	else{
		OPECTL.Seisan_Chk_mod = OFF;								// 精算ﾁｪｯｸﾓｰﾄﾞOFF
	}

	if (NTNetID100ResetFlag == 1) {
		NTNET_Snd_Data101_2();
		wopelg(OPLOG_SET_CTRL_RESET, 0, 0);							// 操作履歴登録
	}
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
	if(RAMCLR != 0){// メモリイニシャル実行
		LongTermParkingCheck_r10_defset();// 長期駐車検出時間の設定を記憶
	}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
	if (OPECTL.Seisan_Chk_mod == ON) {			// 精算ﾁｪｯｸﾓｰﾄﾞON
		// SRAM初期化（無条件ｸﾘｱ）
		log_init();
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
// note : GT-4100はセンター通信が必要なので、メモリクリアは行わない
//		memclr(1);
//
//		// 車室ﾊﾟﾗﾒｰﾀ詰め直し
//		memset(LockInfo, 0, sizeof(LockInfo));
//
//		for (i = 0; i < INT_CAR_LOCK_MAX; i++) {
//			LockInfo[lockinfo_recAP1[i].adr] = lockinfo_recAP1[i].dat;
//			LockInfo[lockinfo_recAP2[i].adr] = lockinfo_recAP2[i].dat;
//		}
//		
//		// 通信設定無効化
//		CPrmSS[S_TYP][41]  = 0;		// ﾛｯｸ装置用I/F盤親機接続台数
//		CPrmSS[S_TYP][101] = 0;		// ﾌﾗｯﾌﾟ用I/F盤ﾀｰﾐﾅﾙ数
//		CPrmSS[S_TYP][41]  = 0;			// ﾛｯｸ装置用I/F盤親機接続台数（0=直接制御）
//		CPrmSS[S_TYP][42]  = 200000;	// ﾛｯｸ装置用I/F盤親機接続台数（20台）
//		CPrmSS[S_TYP][100]  = 10;		// 
//		CPrmSS[S_TYP][47]  = 1001;		// 
//		CPrmSS[S_TYP][118]  = 3;		// 
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算
		CPrmSS[S_SYS][80]  = 100000;		// 
		// 各動作用データ初期化
		memset(&AutoPayment, 0, sizeof(t_AutoPayment));
	}
#endif

	if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){			// FLASH⇒RAMへのパラメータデータ転送
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[61] );	// "      ﾀﾞｳﾝﾛｰﾄﾞﾊﾟﾗﾒｰﾀｰｾｯﾄ      "表示
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// デフォルトセットログ登録処理
	}
	dsp_background_color(COLOR_WHITE);
	SetDiffLogReset(2);												// 共通パラメータミラーリング

	if(	LOCKCLR == 1 ){												// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		lockmaker_clr(1);											// ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ(共通ﾊﾟﾗﾒｰﾀ確定後行う)
	}
	NTNET_ClrSetup();

	remote_timer_set = remotedl_restore();

	/*** 停復電ﾛｸﾞ登録処理 ***/
	Log_regist( LOG_POWERON );										// 停復電ﾛｸﾞ登録
	if(f_log_init == 1){											// ログイニシャル実施
		wopelg( OPLOG_DATA_INITIAL, 0, 0 );							// 操作履歴登録 データイニシャル起動
	}else if(f_log_init == 2){										// スーパーイニシャル実施
		wopelg( OPLOG_ALL_INITIAL, 0, 0 );							// 操作履歴登録 全イニシャライズ起動
	}

// MH810100(S) K.Onodera  2020/01/17 車番チケットレス(LCDリセット通知対応)
	PktResetReqFlgSet( SystemResetFlg );	// LCDリセット通知要求要否セット
	SystemResetFlg = 0;						// システムリセット実行フラグOFF
// MH810100(E) K.Onodera  2020/01/17 車番チケットレス(LCDリセット通知対応)
	FLAGPT.nmi_flg = 0;												// NMIﾌﾗｸﾞｸﾘｱ
	
	if( FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC ){		// 起動時の設定更新履歴は時計確定後に登録する
		SetSetDiff(SETDIFFLOG_SYU_INVALID);					// 設定更新履歴ログ登録	
	}
	
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_INSTRUCTION ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_INSTRUCTION, 2, 0, 0 );	// 未定義命令例外割り込み発生
	}
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_SUPERVISOR_INST ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_SUPERVISOR_INST, 2, 0, 0 );	// 特権命令例外発生
	}
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_FLOATINGPOINT ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_FLOATINGPOINT, 2, 0, 0 );	// 浮動小数点例外発生
	}
	if( FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_UNKOWNVECT, 2, 0, 0 );			// 未登録ﾍﾞｸﾀｱｸｾｽ発生
	}
	
	FLAGPT.event_CtrlBitData.BYTE = 0;								// ﾌﾗｸﾞのｸﾘｱ
	inc_dct( MOV_PDWN_CNT, 1 );										// 復電動作回数ｶｳﾝﾄ
	/*** 復電処理 ***/

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
//	// この処理は重いうえ,毎回呼ぶ必要が無いので電源ON時と設定変更時のみCallする事とする。
//	mc10();															// 料金計算用設定ｾｯﾄ
	cal_cloud_init();												// クラウド料金計算 初期化処理
	if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
		// 毎回呼ぶ必要の無い重い処理なので電源ON時と設定変更時のみCallする
		mc10();
	}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

	if( RAMCLR != 0 ){
		Prn_LogoHead_Init();
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		LogDataClr(LOG_EDYSYUUKEI);
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		LogDataClr(LOG_SCASYUUKEI);
// MH321800(S) hosoda ICクレジット対応
		LogDataClr(LOG_ECSYUUKEI);
// MH321800(E) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		memset( &Edy_Recv_Buf, 0, sizeof( Edy_Recv_Buf ));
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	}
	else{
		ope_Furikae_fukuden();
		turikan_fuk();												// 釣銭管理復電処理
		fukuden();													// 集計等、復電処理
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		Edy_Fukuden();
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		Mifare_WrtNgDataUpdate_Exec();								// Mifare書込み失敗ｶｰﾄﾞ情報復電処理
		LockInfoDataCheck();
	}

	Ope_InSigSts_Initial();											// 入力信号情報(OPE用)初期化
	
	TimeAdjustInit();												// 時刻自動補正初期処理

	/*** ﾀｲﾏｰｽﾀｰﾄ ***/
	Lag10msInit();
	Lag20msInit();
	Lag500msInit();
	Btcom_Init();
	TPU1_init();													// TPU1 Initial
	TPU2_init();													// TPU2 Initial
	TPU3_init();													// TPU3 Initial
	CMTU0_CMT0_init();												// JVMA SYN信号用1shotタイマ初期化
	Sci2Init();														// JVMAシリアルポート初期化
	Sci3Init();														// 磁気リーダー通信ポート初期化
	Sci4Init();														// Flap制御通信ポート初期化
	Sci7Init();														// serverFOMA_Rism通信ポート初期化
	Sci9Init();														// NT-NET通信ポート初期化
	Sci10Init();													// 非接触IC通信ポート初期化
	Sci11Init();													// クレジットFOMA_Cappi通信ポート初期化
// MH810103 GG119202(S) DT10のSPIドライバ対応
#ifdef	DT10_SPI
	Rspi_DT10_Init();
#endif	// DT10_SPI
// MH810103 GG119202(E) DT10のSPIドライバ対応
	WACDOG;															// Watch dog timer reset

	if( OPECTL.Mnt_mod != 5 ){
	RAU_Credit_Enabale = (uchar)prm_get( COM_PRM,S_PAY,24,1,2 );
	if(RAU_Credit_Enabale != 2 && RAU_Credit_Enabale != 3){
		RAU_Credit_Enabale = 0;
	}
	/*** ﾓｼﾞｭｰﾙ機器接続待ち ***/
	door_int_read();												// ドア状態をリードする
	if( OPE_SIG_DOOR_Is_OPEN ){
		OPECTL.f_DoorSts = 1;										// ﾄﾞｱ開状態とする
	}
	mojule_wait();													// 接続機器起動待ち( ARCNET機器含む )	
	key_read();														// key scan data read
	prn_int();														// ﾌﾟﾘﾝﾀ初期化
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	Prn_GetModel_R();												// レシートプリンタモデル取得
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

	CountFree(ALL_COUNT);		// 復電処理対象外（印字途中）の通し追い番を返却


	/*** ※ｴﾗｰ登録はこれ以降で行う（ mojule_wait()内で全ｸﾘｱ登録しているため ）***/

	PortAlm_Init();													// 警報情報Reset（情報収集開始）

	/*** 共通ﾊﾟﾗﾒｰﾀ ｻﾑｴﾗｰLOG登録 ***/								// mojule_wait() で全ｴﾗｰｸﾘｱ登録するので その後に登録する
	if( 0 != (PowonSts_Param & 0x0002) ){							// SRAM上共通ﾊﾟﾗﾒｰﾀ ｻﾑｴﾗｰ
		err_chk( ERRMDL_MAIN, ERR_MAIN_SETTEIRAM, 2, 0, 0 );		// RAM上 設定ｴﾘｱ ｻﾑ異常 ｴﾗｰ登録
	}
	if( 0 != (PowonSts_Param & 0x0020) ){							// FlashROM上共通ﾊﾟﾗﾒｰﾀ ｻﾑｴﾗｰ
		err_chk( ERRMDL_MAIN, ERR_MAIN_SETTEIFLASH, 2, 0, 0 );		// FlashROM上 設定ｴﾘｱ ｻﾑ異常 ｴﾗｰ登録
	}
	if( 0 != (f_ErrAlm & 0x01) ){									// b0:1=E0057(時計読み込み失敗)発生
		err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKREADFAIL, 1, 0, 0 );	// 時計読み込み失敗
	}
	if( 0 != (f_ErrAlm & 0x02) ){									// b1:1=E0058(機器情報読み込み失敗)発生
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_MACHINEREADFAIL, 2, 0 );	// ｴﾗｰﾛｸﾞ登録(機器情報読み込み失敗)
	}

	if( RTC_Err.BIT.RTC_VLF ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);				/* VLF(発振回路電圧低下	*/
	}
	if( RTC_Err.BIT.RTC_VDET ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);			/* VDET(温度補償回路電圧低下) */
	}
	RTC_Err.BYTE = 0;
	if(f_parm_datchk == PARM_AREA_ERR) {
		err_chk( ERRMDL_MAIN, ERR_MAIN_BOOTINFO_PARM, 1, 0, 0 );	// 共通パラメータ運用面異常(発生のみ、チェックの時点で修復はしている)
	}
	if((f_wave_datchk == WAVE_AREA_ERR_SETAREA1) || (f_wave_datchk == WAVE_AREA_ERR_SETAREA2)) {
		err_chk( ERRMDL_MAIN, ERR_MAIN_BOOTINFO_WAVE, 1, 0, 0 );	// 音声データ運用面異常(発生のみ、チェックの時点で修復はしている)
	}

// MH810100(S) K.Onodera 2020/03/11 車番チケットレス(不要処理削除)
//	AVM_Sodiac_Init();
//
//	red_int();														// ﾘｰﾀﾞｰ初期化
// MH810100(E) K.Onodera 2020/03/11 車番チケットレス(不要処理削除)

	}

// MH810103 GG119202(S) 起動シーケンス不具合修正
	ModuleStartFlag = 0;
// MH810103 GG119202(E) 起動シーケンス不具合修正
// GM760201(S) 設定アップロード中の電源OFF/ONでAMANO画面から進まないタイミングがあるため、修正
// GG129000(S) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
// 	remotedl_info_clr();
// GG129000(E) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
// GM760201(E) 設定アップロード中の電源OFF/ONでAMANO画面から進まないタイミングがあるため、修正
	/*** ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ確定 ***/
	if(OPECTL.Mnt_mod != 5){
		if(stricmp((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2) != 0){
			memset(&Chk_info.Chk_str1, 0, 30);		// 固定文字列1・2をクリアする
		}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// PowerOn時に必ずop_init00()を起動するための処理に変更
//		if( CP_MODECHG == 0 ){											// 設定ﾓｰﾄﾞｽｲｯﾁ(開錠ｽｲｯﾁ)ON?
//			OPECTL.Mnt_mod = 1;											// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
//			OPECTL.Mnt_lev = (char)-1;									// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//			OPECTL.Ope_mod = 0;											// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
//			OPECTL.Pay_mod = 0;											// 精算の種類(通常精算)
//			OPECTL.PasswordLevel = (char)-1;
//		}else{
//			OPECTL.Mnt_mod = 0;											// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
//			OPECTL.Mnt_lev = (char)-1;									// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//			OPECTL.Ope_mod = 0;											// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
//			OPECTL.Pay_mod = 0;											// 精算の種類(通常精算)
//			OPECTL.PasswordLevel = (char)-1;
//		}
//		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//		OPECTL.Kakari_Num = 0;											// ﾒﾝﾃﾅﾝｽ係員番号初期化

		// PowerOn時に必ずop_init00()を起動するための処理
		OPECTL.init_sts = 0;										// 未初期化
		OPECTL.Mnt_mod = 0;											// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
		OPECTL.Mnt_lev = (char)-1;									// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
		OPECTL.Ope_mod = 255;										// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ変更
		OPECTL.Pay_mod = 0;											// 精算の種類(通常精算)
		OPECTL.PasswordLevel = (char)-1;
		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
		OPECTL.Kakari_Num = 0;											// ﾒﾝﾃﾅﾝｽ係員番号初期化
		OPECTL.sended_opcls = (char)-1;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	}
	// イニシャル後の電源立ち上げ時に初期設定を行わせるため。
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	ck_syuukei_ptr_zai(LOG_SCASYUUKEI);
// MH321800(S) hosoda ICクレジット対応
	ck_syuukei_ptr_zai(LOG_ECSYUUKEI);
// MH321800(E) hosoda ICクレジット対応
	if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// 起動時にﾄﾞｱの開閉状態をﾁｪｯｸ
		mask_VIB_SNS.BIT.START_STATUS = 1;							// 起動時にﾄﾞｱ閉の場合はﾌﾗｸﾞON
	}

	if(OPE_SIG_DOOR_and_NOBU_are_CLOSE && OPE_IS_EBABLE_MAG_LOCK){	// 前面扉が閉且つﾄﾞｱﾉﾌﾞも閉状態かつ電磁ロックを行う設定
		OPE_SIG_OUT_DOORNOBU_LOCK(1);								// ﾄﾞｱﾉﾌﾞのﾛｯｸを行う
		wmonlg( OPMON_LOCK_CLOSE, 0, 0 );							// ﾓﾆﾀ登録
		wopelg2( OPLOG_DOORNOBU_LOCK, 0L, 0L );						// 操作履歴登録(ﾄﾞｱﾉﾌﾞﾛｯｸ)
		attend_no = 0;					// ドアロック係員№をクリア
	}

	if( GT_Settei_flg == 1 && PPrmSS[S_P01][1] != 0 ){				// 磁気ﾘｰﾀﾞｰﾀｲﾌﾟ異常の場合でｼｬｯﾀｰ開放設定時
		read_sht_cls();												// ｼｬｯﾀｰｸﾛｰｽﾞ
	}

	if(SFV_SNDSTS.BIT.note != 0){
		err_chk( ERRMDL_NOTE, ERR_NOTE_SAFE, 2, 0, 0 );
		IoLog_write(IOLOG_EVNT_ERR_NOTESAFE, (ushort)(( LockInfo[ryo_buf.pkiti - 1].area * 10000L ) + LockInfo[ryo_buf.pkiti - 1].posi ), 0, 0);
		SFV_SNDSTS.BIT.note = 0;
	}
	if(SFV_SNDSTS.BIT.coin != 0){
		err_chk( ERRMDL_COIM, ERR_COIN_SAFE, 2, 0, 0 );
		IoLog_write(IOLOG_EVNT_ERR_COINSAFE, (ushort)(( LockInfo[ryo_buf.pkiti - 1].area * 10000L ) + LockInfo[ryo_buf.pkiti - 1].posi ), 0, 0);
		SFV_SNDSTS.BIT.coin = 0;
	}
	if( remote_timer_set == 1 ){									// 結果情報のアップロードが必要な場合
		// 接続リトライ間隔(秒)
		interval = prm_get(COM_PRM, S_RDL, 5, 3, 3);
		if (!interval) {
			interval = 1;
		}
		LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
	}
	else {
		remotedl_pow_flg_set(FALSE);
	}

	creCheckRejectSaleData();										// 拒否データがあればエラー出力

	LcdBackLightCtrl( ON );											// back light ON ※start_dsp()修正による追加
	// 起動時にセンター用端末情報データを送信する
	NTNET_Snd_Data65(REMOTE_PC_TERMINAL_NO);
// MH810100(S) K.Onodera  2019/12/25 車番チケットレス(起動通知受信をOPEタスク初期化完了まで待つ)
	PktStartUpStsChg( 1 );
// MH810100(E) K.Onodera  2019/12/25 車番チケットレス(起動通知受信をOPEタスク初期化完了まで待つ)
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// アイドルサブタスクに移動
//// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
//	LongTermParkingCheck();// 長期駐車チェック
//// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

	/*** ｵﾍﾟﾚｰｼｮﾝﾀｽｸ ***/
	for( ; ; ) {
		switch( OPECTL.Mnt_mod ){									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ
			case 0:													// 通常ﾓｰﾄﾞ
				OpeMain();
				NTNET_ClrSetup();
				break;
			case 1:													// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択
				MntMain();
				break;
			case 2:													// 係員,管理者ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
				UserMntMain();
				break;
			case 3: 												// 技術員ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
				SysMntMain();
				break;
			case 4: 												// 補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
				dsp_background_color(COLOR_WHITE);
				if( CPrmSS[S_KAN][1] ){
					UsMnt_mnyctl();									// 釣銭補充（金銭管理ありの場合）
				}else{
					UsMnt_mnyctl2();								// 釣銭補充（金銭管理なしの場合）
				}
				OPECTL.Mnt_mod = 0;									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ ← 通常
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;
			case 5:													// 工場検査モード
				ChekModeMain();
				break;
			default:
				OPECTL.Mnt_mod = 0;									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;							// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				OPECTL.Ope_mod = 0;									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
				OPECTL.Pay_mod = 0;									// 精算の種類(通常精算)
				break;
		}
		rd_shutter();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
		LedReq( CN_TRAYLED, LED_OFF );								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 振動センサータイムアウト発生待ち処理 コールバック関数                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : vib_sns_timeout( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void			                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : y.katoh                                                 |*/
/*| Date         : 2007-09-25                                              |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void vib_sns_timeout( void )
{
	if( OPE_SIG_DOOR_Is_OPEN == 0 && FNT_VIB_SNS != 0 ){	// ドア閉かつ振動センサーONの場合
		alm_chk( ALMMDL_MAIN, ALARM_VIBSNS, 1 );	// アラーム登録
	}												// それ以外の場合はアラーム登録しない
	
	mask_VIB_SNS.BIT.MASK = 0;						// 振動センサーマスクフラグOFF
	mask_VIB_SNS.BIT.TIMER_END = 1;					// 振動センサーマスクタイマータイムアウト
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ待ち処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMessage( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ﾒｯｾｰｼﾞのｺﾏﾝﾄﾞ部                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

unsigned short GetMessage( void )
{
	MsgBuf		*msb;
	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;
	ushort PrintKind;
	T_FrmIrekaeTest FrmIrekaeTest;
	T_FrmEnd *FrmEnd;
	short	VIB_SNS_timer;
	static char	on_off;

	for( ; ; ) {
		taskchg( IDLETSKNO );										// Change task to idletask
		if( (msb = GetMsg( OPETCBNO )) == NULL ){
			continue;
		}
		kinko_syuukei();

		sc.comd = msb->msg.command;
		OPECTL.Comd_knd = sc.cc[0];									// Command kind
		OPECTL.Comd_cod = sc.cc[1];									// Command code

		switch( OPECTL.Comd_knd ){

			case MSGGETHIGH(KEY_TEN):								// KEY event

				OPECTL.on_off = msb->msg.data[0];
				switch( sc.comd ){

					case KEY_MODECHG:								// Mode change key
						BUZPI();
						break;

					case KEY_DOOR1:									// Door1 key event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							if( OPECTL.on_off == 1 ){				// OFF(Door Open)
								wopelg( OPLOG_DOOROPEN, 0, 0 );		// 操作履歴登録
								wmonlg( OPMON_DOOROPEN, 0, 0 );		// ﾓﾆﾀ登録
								mask_VIB_SNS.BIT.MASK = 0;			// ドア開でマスクフラグを解除
								mask_VIB_SNS.BIT.TIMER_END = 0;		// ドア開でマスクタイマータイムアウト検知初期化
								LagCan500ms(LAG500_VIB_SNS_TIMER);	//  タイマーキャンセル
								if(OPE_SIG_DOORNOBU_Is_OPEN){				// ﾄﾞｱﾉﾌﾞが開(横)状態
									StartupDoorArmDisable = 1;
								}
								else {
									if( StartupDoorArmDisable == 1 ){				// 起動時のドア警報を抑止
										on_off = !(OPECTL.on_off & 1);
										queset(OPETCBNO, KEY_DOORARM, 1, &on_off);	// ドア警報
									}
								}
								if(DoorLockTimer){								// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｾｯﾄ済み
									DoorLockTimer = 0;							// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰﾌﾗｸﾞｸﾘｱ
									Lagcan( OPETCBNO, 16 );					// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰﾘｾｯﾄ
								}
								Lagcan(OPETCBNO, 5);				// ドアノブ戻し忘れ防止タイマー停止
								opa_chime(OFF);						// チャイム停止
								if(OPE_SIG_DOORNOBU_Is_OPEN){				// ﾄﾞｱﾉﾌﾞを開状態にした場合
									OPECTL.f_DoorSts = 1;
								}
							}else{
								StartupDoorArmDisable = 1;
								if ( mask_VIB_SNS.BIT.MASK == 0 )			// ドア開閉によるタイマー再起動保護
								{
									VIB_SNS_timer = (short)prm_get(COM_PRM, S_PAY, 32, 2, 1);		// 振動センサーマスク時間取得
									
									// タイムアウト値が０で無い場合は、マスクタイマースタート
									if ( VIB_SNS_timer != 0 )
									{
										mask_VIB_SNS.BIT.TIMER_END = 0;		// マスクタイマータイムアウト検知初期化
										mask_VIB_SNS.BIT.MASK = 1;			// 振動センサーマスク中
										LagTim500ms( LAG500_VIB_SNS_TIMER, (short)((VIB_SNS_timer * 2) + 1), vib_sns_timeout );
									}
								}								
								wopelg( OPLOG_DOORCLOSE, 0, 0 );	// 操作履歴登録
								wmonlg( OPMON_DOORCLOSE, 0, 0 );	// ﾓﾆﾀ登録
								if( !OPE_SIG_DOORNOBU_Is_OPEN ){			// ﾄﾞｱﾉﾌﾞが縦(閉)の状態の場合
									if(prm_get(COM_PRM, S_PAY, 17, 3, 4)){	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ち時間設定あり
										Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ち時間ﾘｾｯﾄ
									}
								}
								else{
									Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ち時間ﾘｾｯﾄ
								}
								Ope_StartDoorKnobTimer();
								Lagcan( OPETCBNO, 33 );
							}
						}
						OPECTL.coin_syukei = 0;						// ｺｲﾝ金庫集計未出力とする
						OPECTL.note_syukei = 0;						// 紙幣金庫集計未出力とする
						mask_VIB_SNS.BIT.START_STATUS = 0;				// ﾄﾞｱ開閉で起動時状態のﾌﾗｸﾞﾘｾｯﾄ
						break;

					case KEY_COINSF:								// Coin safe event
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// チャイム停止
						}
						if( OPECTL.on_off == 1 ){					// ON
							if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
								if( OPECTL.coin_syukei == 0 ){		// ｺｲﾝ金庫集計未出力?
									coin_kinko_evt = 1;				// コイン金庫引き抜き
								}
								wopelg( OPLOG_COINKINKO, 0, 0 );	// 操作履歴ﾛｸﾞ登録
								wmonlg( OPMON_COINKINKO_UNSET, 0, 0 );	// ﾓﾆﾀ登録
							}
						}
						else{										// OFF (ｺｲﾝ金庫着)
							if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
								wmonlg( OPMON_COINKINKO_SET, 0, 0 );	// ﾓﾆﾀ登録
							}
						}
						sc.comd = 0;
						break;

					case KEY_DOORARM:								// Door alarm event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							#if (DEBUG_MODE_01 == DEBUG_MODE)		// ﾏｽｸ設定（評価用）
								OPECTL.on_off = 1;					// 常にOFF（警報無し）
							#endif
							if( OPECTL.on_off == 0 ){				// High
								Lagtim( OPETCBNO, 33, 1*50 );		// 1sタイマセット(警報ブザー発報ディレイタイマ(ドア警報))
							}else{									// Low
								alm_chk( ALMMDL_MAIN, ALARM_DOORALARM, 0 );
								Lagcan( OPETCBNO, 33);
							}
						}
						sc.comd = 0;
						break;

					case KEY_VIBSNS:								// 振動ｾﾝｻ event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							VIB_SNS_timer = (short)prm_get(COM_PRM, S_PAY, 32, 2, 1);		// 振動センサーマスク時間取得
							if( OPECTL.on_off == 1 && 
								(( mask_VIB_SNS.BIT.MASK == 0 && !VIB_SNS_timer ) || 				// 振動センサーONかつマスク時間が０の場合か
								   mask_VIB_SNS.BIT.TIMER_END == 1 || mask_VIB_SNS.BIT.START_STATUS == 1 )){	// マスクタイマーがタイムアウト済みか起動時にﾄﾞｱ閉状態での振動検知は即時アラーム登録
								Lagtim( OPETCBNO, 34, 1*50 );		// 1sタイマセット(警報ブザー発報ディレイタイマ(振動センサー))
							}
							else if( OPECTL.on_off == 0 ){ 
								Lagcan( OPETCBNO, 34 );
							}
						}
						sc.comd = 0;
						break;

					case KEY_NTFALL:								// 紙幣ﾘｰﾀﾞｰ脱落検知ｽｲｯﾁ
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							#if (DEBUG_MODE_01 == DEBUG_MODE)		// ﾏｽｸ設定（評価用）
								OPECTL.on_off = 1;					// 常にOFF（警報無し）
							#endif
							if( OPECTL.on_off == 0 ){				// High
								Lagtim( OPETCBNO, 35, 1*50 );		// 1sタイマセット(警報ブザー発報ディレイタイマ(紙幣脱落))
							}else{									// Low
								Lagcan( OPETCBNO, 35 );
							}
						}
						sc.comd = 0;
						break;

					case KEY_CNDRSW:								// ｺｲﾝﾒｯｸ引き抜きｽｲｯﾁ
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// チャイム停止
						}
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							if( OPECTL.on_off == 0 ){				// High
								wopelg( OPLOG_COINMECHSWCLOSE, 0, 0 );	// 操作履歴登録
								wmonlg( OPMON_COINMECHSWCLOSE, 0, 0 );	// ﾓﾆﾀ登録
							}else{
								wopelg( OPLOG_COINMECHSWOPEN, 0, 0 );	// 操作履歴登録
								wmonlg( OPMON_COINMECHSWOPEN, 0, 0 );	// ﾓﾆﾀ登録
							}
						}
						break;

					case KEY_NOTESF:								// 紙幣金庫ｶﾊﾞｰｽｲｯﾁ
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// チャイム停止
						}
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							if( OPECTL.on_off == 0 ){				// High
								wopelg( OPLOG_NOTERDSWCLOSE, 0, 0 );	// 操作履歴登録
								wmonlg( OPMON_NOTERDSWCLOSE, 0, 0 );	// ﾓﾆﾀ登録
							}else{
								wopelg( OPLOG_NOTERDSWOPEN, 0, 0 );		// 操作履歴登録
								wmonlg( OPMON_NOTERDSWOPEN, 0, 0 );		// ﾓﾆﾀ登録
							}
						}
						break;
					case KEY_DOORNOBU:
						if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// SW動作ﾁｪｯｸ中?
							if(OPE_SIG_DOORNOBU_Is_OPEN){				// ﾄﾞｱﾉﾌﾞを開状態にした場合
								Ope_StartDoorKnobTimer();
								on_off = !(OPECTL.on_off & 1);
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//// MH810100(S) S.Nishimoto 2020/08/27 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
//								// 印字処理中はドアノブ開のイベントも通知しない
//								if (OPECTL.Mnt_mod == 0 ||
//										(rct_proc_data.Printing == 0 && jnl_proc_data.Printing == 0)) {
//// MH810100(E) S.Nishimoto 2020/08/27 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
								// 印字処理中/パラメータアップロード中はドアノブ開のイベントも通知しない
								if (OPECTL.Mnt_mod == 0 ||
										(rct_proc_data.Printing == 0 && jnl_proc_data.Printing == 0 &&
										OPECTL.lcd_prm_update == 0)) {
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
// MH810100(S) S.Nishimoto 2020/08/27 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
								}
// MH810100(E) S.Nishimoto 2020/08/27 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
								queset(OPETCBNO, KEY_DOORARM, 1, &on_off);	// ドア警報解除
								Ope_CenterDoorTimer( 0 );				// タイマー停止
							}else{
								Lagcan( OPETCBNO, 16 );					// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸﾀｲﾏｰﾘｾｯﾄ
								Lagcan( OPETCBNO, 17 );					// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ちﾀｲﾏｰﾘｾｯﾄ
								if( !OPE_SIG_DOOR_Is_OPEN ){					// ﾄﾞｱ閉
									if( prm_get(COM_PRM, S_PAY, 17, 3, 4) ){	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ち時間設定あり
										Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50) );	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
									}
								}
								Lagcan(OPETCBNO, 5);					// ドアノブ戻し忘れ防止タイマー停止
								opa_chime(OFF);							// チャイム停止
								on_off = !(OPECTL.on_off & 1);
								if( OPECTL.Mnt_mod != 0 ){				// メンテナンス中
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//									if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){	// 印字処理中ではない場合のみ
									if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) && 	// 印字処理中ではない場合と
											 OPECTL.lcd_prm_update == 0){									// パウラメータアップロード中でない場合
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
										queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
									}
								}
								else{
									queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
								}
								if( OPECTL.f_DoorSts && (!OPE_SIG_DOOR_Is_OPEN) ){
									// ドアノブ通常、ドア閉状態でタイマースタートさせる。ドア開ではスタートさせない。
									Ope_CenterDoorTimer( 1 );			// タイマー起動
								}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
								// ドア閉時にRSWが0以外？
								if( read_rotsw() != 0 ){
									lcdbm_notice_alm( lcdbm_alarm_check() );	// 警告通知
								}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
							}
						}
// MH810100(S) Yamauchi 2020/02/18 車番チケットレス_ドア状態の通知は無条件で行う
						// ドア状態
						lcdbm_notice_ope( LCDBM_OPCD_DOOR_STATUS, (ushort)on_off );
// MH810100(E) Yamauchi 2020/02/18 車番チケットレス
						break;						
					case KEY_FANST:									// FAN駆動開始温度検出
						if( 1 == chk_fan_timectrl() ){				// FAN強制停止時刻
							if(OPECTL.on_off == 1) {				// FAN駆動開始温度到達
								fan_exe_state = 1;					// 通常制御時の状態をバックアップ
							}else{
								fan_exe_state = 0;					// 通常制御時の状態をバックアップ
							}
						}else{
							if(OPECTL.on_off == 1) {				// FAN駆動開始温度到達
								CP_FAN_CTRL_SW = 1;					// FAN駆動
								fan_exe_state = 1;					// 通常制御時の状態をバックアップ
								// FANを駆動した時間を記録
								fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
								fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
								fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
								fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
								fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
								fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 1;//ON
								fan_exe_time.index++;
								if(fan_exe_time.index >= FAN_EXE_BUF) {
									fan_exe_time.index = 0;
								}
							} else {// OPECTL.on_off == 0
								CP_FAN_CTRL_SW = 0;					// FAN停止
								fan_exe_state = 0;					// 通常制御時の状態をバックアップ
								// FANを停止した時間を記録
								fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
								fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
								fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
								fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
								fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
								fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 2;//OFF
								fan_exe_time.index++;
								if(fan_exe_time.index >= FAN_EXE_BUF) {
									fan_exe_time.index = 0;
								}
							}
						}
						break;
					case KEY_TEN_F1:
					case KEY_TEN_F2:
						if( OPECTL.on_off == 1) {					// ON
							if(OpeKeyRepeat.enable == 1) {			// キーリピート有効
								if(OpeKeyRepeat.key == sc.comd) {	// キーON2回目以降(リピート)
									LagTim20ms(LAG20_KEY_REPEAT, 11, Ope_KeyRepeat);	// 200ms
								}
								else {								// キーON初回
									OpeKeyRepeat.key = sc.comd;		// リピート対象キーコマンド退避
									LagTim20ms(LAG20_KEY_REPEAT, 26, Ope_KeyRepeat);	// 500ms
								}
							}
						}
						else {										// OFF
							OpeKeyRepeat.key = 0;
							LagCan20ms(LAG20_KEY_REPEAT);			// リピートタイマ削除
							sc.comd = 0;							// OFF時はﾒｯｾｰｼﾞ無しとする
						}
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							if( Ope_IsEnableDoorKnobChime() ){
								// ドアノブ戻し忘れチャイム有効の場合のみタイマを再設定する
								Ope_StartDoorKnobTimer();
							}
							opa_chime(OFF);								// チャイム停止
						}
						break;
					case KEY_TEN_F5:
						if( OPECTL.Mnt_mod != 0 && OPECTL.Mnt_mod != 5 ){					// メンテナンス中
							if( !OPE_SIG_DOORNOBU_Is_OPEN ){		// ﾄﾞｱﾉﾌﾞ閉状態
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
							}
						}
					default:
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							if( Ope_IsEnableDoorKnobChime() && sc.comd != KEY_MANDET ){	// ドアノブ戻し忘れチャイム有効
								Ope_StartDoorKnobTimer();
							}
							if( sc.comd != KEY_MANDET ) {			// 人体検知以外はアラーム停止する
								opa_chime(OFF);						// チャイム停止
							}
						}
						if( sc.comd == KEY_RXIIN )
						{	// RYB入力信号
							if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// 入出力信号ﾁｪｯｸ中でない
								InSignalCtrl(INSIG_OPOPEN);			//強制営業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
								InSignalCtrl(INSIG_OPCLOSE);		//強制休業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
								if( OPECTL.on_off == 0 ){			// OFF?(Y)
									sc.comd = 0;					// OFF時はﾒｯｾｰｼﾞ無しとする
								}
							}
						}else{
							if( OPECTL.on_off == 0 ){				// OFF?(Y)
								sc.comd = 0;						// OFF時はﾒｯｾｰｼﾞ無しとする
							}
						}
						break;
				}
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
			case MSGGETHIGH(LCD_BASE):				// lcdbm_rsp
				switch( sc.comd ) {
					// エラー通知
					case LCD_ERROR_NOTICE:
						memcpy( &LcdRecv.lcdbm_rsp_error, &msb->msg.data[0], sizeof( lcdbm_rsp_error_t ));
						// 受信エラーの登録処理
						lcdbm_ErrorReceiveProc();
						break;

					// 決済状態ﾃﾞｰﾀ
					case LCD_ICC_SETTLEMENT_STS:
						memcpy( &lcdbm_ICC_Settlement_Status, &msb->msg.data[0], sizeof( unsigned char ));
						break;

					// 決済結果情報
					case LCD_ICC_SETTLEMENT_RSLT:
						memcpy( &lcdbm_ICC_Settlement_Result, &msb->msg.data[0], sizeof( unsigned char ));
						break;

					// リアルタイム通信疎通結果(メンテナンス)
					case LCD_MNT_REALTIME_RESULT:
						memcpy( &LcdRecv.lcdbm_rsp_rt_con_rslt, &msb->msg.data[0], sizeof( lcdbm_rsp_rt_con_rslt_t ));
						break;

					// DC-NET通信疎通結果(メンテナンス)
					case LCD_MNT_DCNET_RESULT:
						memcpy( &LcdRecv.lcdbm_rsp_dc_con_rslt, &msb->msg.data[0], sizeof( lcdbm_rsp_dc_con_rslt_t ));
						break;

					// 操作通知
					case LCD_OPERATION_NOTICE:
						memcpy( &LcdRecv.lcdbm_rsp_notice_ope, &msb->msg.data[0], sizeof( lcdbm_rsp_notice_ope_t ));
						if (LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_RESET_NOT) {
							on_off = 1;				// ﾘｾｯﾄ通知
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ送信処理
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCDBM_OPCD_RESET_NOT受信時のope処理は無処理
						}
// MH810100(S) 2020/07/29 #4560 画面がフリーズしてしまう場合がある
						// 起動通知(フェーズがずれたのでいったん切断)
						if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_STA_NOT && LcdRecv.lcdbm_rsp_notice_ope.status != 0){
							on_off = 1;				// ﾘｾｯﾄ通知
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ送信処理
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCDBM_OPCD_RESET_NOT受信時のope処理は無処理

						}
// MH810100(E) 2020/07/29 #4560 画面がフリーズしてしまう場合がある
						if (LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PRM_UPL_REQ) {
 							if(0 == lcdbm_setting_upload_FTP()){						// LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞを行う
								// 成功
								lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, 0 );	// 設定アップロード通知送信（ OK(再起動要) ）
							}else{
								// 失敗
								lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, 1 );	// 設定アップロード通知送信（ 1=NG )
							}
						}
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
						if(LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PAY_STP) {
							OPECTL.f_req_paystop = (uchar)LcdRecv.lcdbm_rsp_notice_ope.status;
						}
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
						break;

					// 切断検知(PKTtaskでLCDとの通信が不通時に発行される)
					case LCD_COMMUNICATION_CUTTING:
						if( OPECTL.Ope_mod != 255 ){
							on_off = 2;				// 切断検知
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ送信処理
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCD_COMMUNICATION_CUTTING受信時のope処理は無処理
						}
						break;

					// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ送信処理
					case LCD_DISCONNECT:
						memcpy( &LcdRecv.lcdbm_lcd_disconnect, &msb->msg.data[0], sizeof( lcdbm_lcd_disconnect_t ));
						break;
				}
				break;		// return == sc.comd
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

			case MSGGETHIGH(CLOCK_CHG):								// Clock change event
				ClkrecUpdate( &msb->msg.data[0] );
				TimeAdjustMain(sc.comd);							// 時刻補正

				AutoDL_UpdateTimeCheck();

				if( 1 == f_NTNET_RCV_MNTTIMCHG ){					// ﾒﾝﾃﾅﾝｽ現在時刻変更
					NTNET_Snd_Data229();							// NT-NET時計ﾃﾞｰﾀ作成
					f_NTNET_RCV_MNTTIMCHG = 0;
				}
// MH810104 GG119201(S) 電子ジャーナル対応
				if (OPECTL.EJAClkSetReq != 0) {
					MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);			// 時刻設定
					OPECTL.EJAClkSetReq = 0;
				}
// MH810104 GG119201(E) 電子ジャーナル対応

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_TimeSyncSend_Chk();
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//				//LCDﾊﾞｯｸﾗｲﾄ動作ｶｳﾝﾄ更新
//				if(SD_LCD && backlight != 0){
//					inc_dct(LCD_LIGTH_CNT,1);
//				}
				// QRリーダー動作時間
				if( QR_READER_USE ){
					inc_dct(QR_READER_CNT,1);
				}
				SUBCPU_MONIT = 0;									// ｻﾌﾞCPU監視ﾌﾗｸﾞｸﾘｱ
				err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 0, 0, 0 ); 		// ｴﾗｰﾛｸﾞ登録
				if( DateTimeCheck() == 0 ){
					// 日時範囲ﾁｪｯｸOK
					err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKFAIL, 0, 0, 0 ); 	// ｴﾗｰﾛｸﾞ登録
				}else{
					err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKFAIL, 1, 0, 0 ); 	// ｴﾗｰﾛｸﾞ登録
				}
				auto_syuukei();										// 自動集計処理

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//				if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {		// Edy利用可能状態か？
//					auto_centercomm( 3 );
//				}
//				if(	Edy_Rec.edy_status.BIT.CENTER_COM_START ){
//					CneterComLimitTime++;
//					Edy_CentComm_Recv( 1 );
//				}
//				Edy_SndData18();									// 閉塞ｺﾏﾝﾄﾞ送信
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

				creOneMinutesCheckProc();
				ntautoTimeAdjust();

				// 共通パラメータで有効になっている方の集計情報を更新する(02-0024③④)
// MH321800(S) hosoda ICクレジット対応
//				if(prm_get(COM_PRM, S_PAY, 24, 1, 3) != 0)				/* ④Suica有効 */
				if ( isSX10_USE() )
// MH321800(E) hosoda ICクレジット対応
				{
					ck_syuukei_ptr_zai(LOG_SCASYUUKEI);
				}
// MH321800(S) hosoda ICクレジット対応
				else if ( isEC_USE() ) {
				// 決済リーダ
					ck_syuukei_ptr_zai(LOG_ECSYUUKEI);
				}
// MH321800(E) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				else if(prm_get(COM_PRM, S_PAY, 24, 1, 4) != 0)			/* ③Edy有効 */
//				{
//					ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);
//				}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// アイドルサブタスクに移動
//// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
//					LongTermParkingCheck();// 長期駐車チェック
//// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322917(S) A.Iiizumi 2018/12/05 リアルタイム情報 フォーマットRev11対応
				// 運用中にリアルタイム情報用の日付切替基準時刻の設定変更が行われたときの設定更新処理
				date_uriage_prmcng_judge();
// MH322917(E) A.Iiizumi 2018/12/05 リアルタイム情報 フォーマットRev11対応
				if( 1 == chk_fan_timectrl() ){// FAN強制停止時刻
					if(f_fan_timectrl == 0){// 前回は強制停止でない
						// FANを停止した時間を記録
						fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
						fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
						fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
						fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
						fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
						fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 2;//OFF
						fan_exe_time.index++;
						if(fan_exe_time.index >= FAN_EXE_BUF) {
							fan_exe_time.index = 0;
						}
					}
					f_fan_timectrl = 1;// FAN強制停止中フラグセット
					CP_FAN_CTRL_SW =0;// FAN動作停止
				}else{
					if(f_fan_timectrl != 0){
						f_fan_timectrl = 0;// FAN強制停止中フラグクリア
						CP_FAN_CTRL_SW = fan_exe_state;// 通常制御時の状態に戻す
						if(fan_exe_state == 1){// 通常制御時はONである
							// FANを駆動した時間を記録
							fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
							fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
							fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
							fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
							fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
							fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 1;//ON
							fan_exe_time.index++;
							if(fan_exe_time.index >= FAN_EXE_BUF) {
								fan_exe_time.index = 0;
							}
						}
					}
				}
// MH810104 GG119201(S) 電子ジャーナル対応
				EJA_TimeAdjust();
// MH810104 GG119201(E) 電子ジャーナル対応
				break;
			case MSGGETHIGH(TIMEOUT):								// Timme out
				if(OPECTL.Comd_cod == 0x03){
					if(CardStackStatus){
						StackCardEject(4);
					}
				}
				else
				if(OPECTL.Comd_cod == 0x05){	// ドアノブ戻し忘れ防止タイマー(5sec)タイムアウト
					if ((OPE_SIG_DOOR_Is_OPEN == 0) && (OPE_SIG_DOORNOBU_Is_OPEN)) {	// ドア閉＆ドアノブ開
						if( Ope_IsEnableDoorKnobChime() ){							// ドアノブ戻し忘れチャイム有効
							opa_chime(ON);
							Ope_StopDoorKnobTimer();
						}
					}
				}
				else if(OPECTL.Comd_cod == 0x06) {			// メンテナンス処理（印字等）タイムアウト
					if(!Ope_IsEnableDoorKnobChime()){
						// ドアノブ開チャイムが無効状態で印字等がタイムアウトした場合は有効にする
						Ope_EnableDoorKnobChime();			// ドアノブ開監視タイマ開始
					}
				}
				else if(OPECTL.Comd_cod == 29) {			// ドアノブ戻し忘れ防止アラーム警告音出力時間タイムアウト
					opa_chime(OFF);
				}
				else
				if(OPECTL.Comd_cod == 0x10){
					if( DoorLockTimer && OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5 ){					// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰがONかつ動作チェックの入出力信号チェック中以外
						Lagcan( OPETCBNO, 16 );										// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰﾘｾｯﾄ
						DoorLockTimer=0;												// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｸﾘｱ
						if(!prm_get(COM_PRM, S_PAY, 17, 3, 4) && !OPE_SIG_DOORNOBU_Is_OPEN){	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ち時間が０且つﾄﾞｱﾉﾌﾞが縦(閉)の状態の場合
						}else{														// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰﾘｾｯﾄ
							Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ちﾀｲﾏｰｽﾀｰﾄ
						}
					}
				}else if(OPECTL.Comd_cod == 0x11){
					if( !OPE_SIG_DOOR_Is_OPEN && OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5 	// 前面扉閉かつ動作チェックの入出力信号チェック中以外
						&& OPE_IS_EBABLE_MAG_LOCK){													// かつ電磁ロックを行う
						OPE_SIG_OUT_DOORNOBU_LOCK(1);								// ﾄﾞｱﾉﾌﾞﾛｯｸ
						wmonlg( OPMON_LOCK_CLOSE, 0, 0 );							// ﾓﾆﾀ登録
						wopelg2( OPLOG_DOORNOBU_LOCK, 0L, 0L );						// 操作履歴登録(ﾄﾞｱﾉﾌﾞﾛｯｸ)
						DoorLockTimer = 0;												// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｸﾘｱ
						Lagcan( OPETCBNO, 17 );										// ﾀｲﾏｰ11ﾘｾｯﾄ
						attend_no = 0;					// ドアロック係員№をクリア
					}
				}
				else if( OPECTL.Comd_cod == 0x09 ){	// Time out No.9
					creCtrl( CRE_EVT_TIME_UP );  	//ﾀｲﾑｱｳﾄ処理
				}
				else if(OPECTL.Comd_cod == 0x16){
					AVM_Sodiac_Play_Wait_Tim(0);	// 0ch音声再生要求送信
				}
				else if(OPECTL.Comd_cod == 0x17){
					AVM_Sodiac_Play_Wait_Tim(1);	// 1ch音声再生要求送信
				}
				else if(OPECTL.Comd_cod == 0x18){
					AVM_Sodiac_TimeOut();			// 音声終了通知なしﾀｲﾏｰﾀｲﾑｱｳﾄ
				}
				else if(OPECTL.Comd_cod == 30){
					Ope_CenterDoorResend();			// センターにエラー・アラームを送る処理
				}
				else if(OPECTL.Comd_cod == 33){
					alm_chk( ALMMDL_MAIN, ALARM_DOORALARM, 1 );			// ドア警報検知
				}
				else if(OPECTL.Comd_cod == 34){
					alm_chk( ALMMDL_MAIN, ALARM_VIBSNS, 1 );			// 振動センサー検知
				}
				else if(OPECTL.Comd_cod == 35){
					alm_chk( ALMMDL_MAIN, ALARM_NOTERE_REMOVE, 1 );		// 紙幣脱落検知
				}
// MH810103 GG119202(S) 起動シーケンス不具合修正
				else if(OPECTL.Comd_cod == TIMERNO_EC_BRAND_NEGO_WAIT) {
					if (isEC_USE()) {
						if ((ECCTL.phase == EC_PHASE_BRAND) &&
							(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3)) {
							// フェーズはそのままで待機状態に戻ったタイミングで再開する
						}
						else {
							// フェーズを初期化中に戻してスタンバイコマンドからやり直し
							ECCTL.phase = EC_PHASE_INIT;				// 初期化中
							ECCTL.step = 0;
						}
					}
				}
// MH810103 GG119202(E) 起動シーケンス不具合修正
				TimeAdjustMain(sc.comd);
				break;

			case MSGGETHIGH(COIN_EVT):								// Coin Mech event
				if (OPECTL.Mnt_mod) {						// メンテナンス中
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);							// チャイム停止
				}
				OPECTL.CN_QSIG = OPECTL.Comd_cod;
				switch( OPECTL.CN_QSIG ){
					case MSGGETLOW(COIN_ES_TIM):					// ｴｽｸﾛ待ちﾀｲﾑｱｳﾄ?
						safecl( 7 );								// 金庫枚数算出
						sc.comd = COIN_EVT;
						break;
					case MSGGETLOW(COIN_CASSET):					// ｶｾｯﾄ着?
						wopelg( OPLOG_TURICASETTE, 0, 0 );
						break;
					case MSGGETLOW(COIN_INVENTRY):					// ﾒｯｸﾎﾞﾀﾝによるｲﾝﾍﾞﾝﾄﾘ終了
						wopelg( OPLOG_INVBUTTON, 0, 0 );
						break;
					case MSGGETLOW(COIN_RJ_EVT):					// 精算中止
						sc.comd = COIN_RJ_EVT;
						break;
					default:
						sc.comd = COIN_EVT;
						break;
				}
				break;

			case MSGGETHIGH(NOTE_EVT):								// Note Reader event
				if (OPECTL.Mnt_mod) {								// メンテナンス中
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);									// チャイム停止
				}
				OPECTL.NT_QSIG = OPECTL.Comd_cod;
				if( sc.comd == NOTE_SO_EVT ){						// Note safe event
					if( msb->msg.data[0] == 1 ){					// ｽﾀｯｶ開
						inc_dct( MOV_NOTE_BOX, 1 );					// 紙幣金庫抜取 動作ｶｳﾝﾄup
						if( OPECTL.Ope_Mnt_flg != 4 && OPECTL.Mnt_mod != 5){  			// 動作ﾁｪｯｸ中?
							if( OPECTL.note_syukei == 0 ){			// 紙幣金庫集計未出力?
								note_kinko_evt = 1;					// 紙幣金庫引き抜き
							}
						}
						wopelg( OPLOG_NOTEKINKO, 0, 0 );			// 操作履歴ﾛｸﾞ登録
						wmonlg( OPMON_NOTEKINKO_UNSET, 0, 0 );		// ﾓﾆﾀ登録
						sc.comd = 0;
					}
					else{											// ｽﾀｯｶ閉
						wmonlg( OPMON_NOTEKINKO_SET, 0, 0 );		// ﾓﾆﾀ登録
						sc.comd = 0;
					}
				}else{
					sc.comd = NOTE_EVT;
				}
				break;

			case MSGGETHIGH(PRNFINISH_EVT):							// Print End event
				if (OPECTL.Mnt_mod) {								// メンテナンス中
					if(!Ope_IsEnableDoorKnobChime()) {
						// ドアノブ開チャイムが無効状態で印字が終了した場合は有効にする
						Ope_EnableDoorKnobChime();
					}
				}
				FrmEnd = (T_FrmEnd*)msb->msg.data;
				OPECTL.Pri_Result	= FrmEnd->BMode;				// 印字結果をﾜｰｸへ格納
				OPECTL.Pri_Kind		= FrmEnd->BPrikind;				// ﾌﾟﾘﾝﾀ種別をﾜｰｸへ格納
				OPECTL.PriEndMsg[(OPECTL.Pri_Kind == R_PRI)? 0 : 1] = *FrmEnd;

				PrintKind = sc.comd & (~INNJI_ENDMASK);				// 印字種別取得
				if (PrintKind == PREQ_COKINKO_G ||					// コイン金庫合計
					PrintKind == PREQ_SIKINKO_G) {					// 喜平金庫合計
					// 金庫集計後は異常でも集計完了とする
					if( OPECTL.Pri_Kind == R_PRI ){					// ﾌﾟﾘﾝﾀ種別？
						// ﾚｼｰﾄ印字終了
						rct_goukei_pri = OFF;
					}
					else{
						// ｼﾞｬｰﾅﾙ印字終了
						jnl_goukei_pri = OFF;
					}
					if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){

						// 合計記録の印字が全て終了した場合

						switch( PrintKind ){
							case PREQ_COKINKO_G:					// コイン金庫集計完了
								// NOTE: ﾚｼｰﾄﾌﾟﾘﾝﾀ、ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀどちらかが異常時は、正常に印字できた方のｲﾍﾞﾝﾄしか返らない仕組みになっているが、
								//       ﾚｼｰﾄﾌﾟﾘﾝﾀ、ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀともに異常時は、それぞれの印字異常終了のｲﾍﾞﾝﾄが返る仕組みになっている。
								if(OPECTL.coin_syukei != (char)-1){ // ｺｲﾝ金庫集計未集計の場合に実施する。
									OPECTL.coin_syukei = (char)-1;	// ｺｲﾝ金庫集計出力済みとする
									kinko_clr( 0 );					// コイン金庫集計ｴﾘｱ更新処理
								}
								break;
							case PREQ_SIKINKO_G:					// 紙幣金庫集計完了
								// NOTE: ﾚｼｰﾄﾌﾟﾘﾝﾀ、ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀどちらかが異常時は、正常に印字できた方のｲﾍﾞﾝﾄしか返らない仕組みになっているが、
								//       ﾚｼｰﾄﾌﾟﾘﾝﾀ、ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀともに異常時は、それぞれの印字異常終了のｲﾍﾞﾝﾄが返る仕組みになっている。
								if(OPECTL.note_syukei != (char)-1){     // 紙幣金庫集計未集計の場合に実施する。
									OPECTL.note_syukei = (char)-1;		// 紙幣金庫集計出力済みとする
									kinko_clr( 1 );						// 紙幣金庫集計ｴﾘｱ更新処理
								}
								break;
							default:
								break;
						}
					}
				}
				else if( PrnGoukeiChk( PrintKind ) == YES ){		// T合計, GT合計記録の印字終了？
					// 合計記録の印字終了
					if( FrmEnd->BMode == PRI_NML_END ){				// 正常終了?
						// 正常終了
						if( OPECTL.Pri_Kind == R_PRI ){				// ﾌﾟﾘﾝﾀ種別？
							// ﾚｼｰﾄ印字終了
							rct_goukei_pri = OFF;
						}
						else{
							// ｼﾞｬｰﾅﾙ印字終了
							jnl_goukei_pri = OFF;
						}
						if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){

							// 合計記録の印字が全て終了した場合（印字ﾌﾟﾘﾝﾀは設定に従う）

							switch( PrintKind ){
								case PREQ_TGOUKEI:					// Ｔ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									if( Check_syuukei_clr( 0 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									ac_flg.syusyu = 11;				// 11:Ｔ合計印字完了
									Make_Log_TGOUKEI();				// 車室毎集計を含めたT合計ﾛｸﾞ作成
									if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){	// Ｔ合計送信する
										NTNET_Snd_TGOUKEI();
									}
									syuukei_clr( 0 );				// Ｔ集計ｴﾘｱ更新処理
									if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// バッチ送信設定有
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
								case PREQ_GTGOUKEI:					// ＧＴ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									if( Check_syuukei_clr( 1 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									syuukei_clr( 1 );				// ＧＴ集計ｴﾘｱ更新処理
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
								case PREQ_MTGOUKEI:					// ＭＴ合計印字完了
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									if( Check_syuukei_clr( 2 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									ac_flg.syusyu = 102;			// 102:ＭＴ合計印字完了
									syuukei_clr( 2 );				// ＭＴ集計ｴﾘｱ更新処理
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
									break;
								default:
									break;
							}
						}
					}
					else {
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;
					// 集計できなければ追番返却
						switch( PrintKind ){
						case PREQ_TGOUKEI:
							CountFree(T_TOTAL_COUNT);
							break;
						case PREQ_GTGOUKEI:
							CountFree(GT_TOTAL_COUNT);
							break;
						default:
							break;
						}
					}
				}
				else if( PrintKind == PREQ_AT_SYUUKEI ){			// 自動集計の印字終了？
					// 自動集計の印字終了
					if( OPECTL.Pri_Kind == R_PRI ){					// ﾌﾟﾘﾝﾀ種別？
						// ﾚｼｰﾄ印字終了
						rct_atsyuk_pri = OFF;
					}
					else{
						// ｼﾞｬｰﾅﾙ印字終了
						jnl_atsyuk_pri = OFF;
					}
					if( rct_atsyuk_pri == OFF && jnl_atsyuk_pri == OFF ){
						// 自動集計の印字が全て終了した場合（印字ﾌﾟﾘﾝﾀは設定に従う）	※自動集計の場合、異常終了でも集計完了とする
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						if( Check_syuukei_clr( 0 )){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						ac_flg.syusyu = 11;							// 11:Ｔ合計印字完了
						Make_Log_TGOUKEI();							// 車室毎集計を含めたT合計ﾛｸﾞ作成
						if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){	// Ｔ合計送信する
							NTNET_Snd_TGOUKEI();
						}
						syuukei_clr( 0 );							// Ｔ集計ｴﾘｱ更新処理
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// バッチ送信設定有
			 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
						auto_syu_prn = 3;							// 集計状態 ← 自動集計完了
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//						// Edy設定があるか								締めﾀｲﾐﾝｸﾞがT合計後か
//						if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && prm_get(COM_PRM, S_SCA, 57, 1, 6) == 0 ) {	
//							auto_centercomm( 2 );						// 自動集計（Ｔ合計ﾌﾟﾘﾝﾄ）後の自動センター通信（締め）処理実施
//						}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
					}
				}
				else if( PrintKind == PREQ_TURIKAN ){					// 釣銭管理印字完了

					uchar terget_pri = PrnGoukeiPri(PREQ_TURIKAN);	// 印字先取得
					uchar clr = 0;

					switch( terget_pri ){
						case 0: // なし
						case R_PRI:
							if( OPECTL.Pri_Kind == R_PRI ){
								clr = 1;
							}
							break;
						case J_PRI:
							if( OPECTL.Pri_Kind == J_PRI ){
								clr = 1;
							}
							break;
						case RJ_PRI:
							if( OPECTL.Pri_Kind == R_PRI ){
								turikan_pri_status |= 0x01;
							}
							if( OPECTL.Pri_Kind == J_PRI ){
								turikan_pri_status |= 0x02;
							}
							if( turikan_pri_status == 0x03 ){
								clr = 1;
							}
							break;
					}
					if( clr ){
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						if( Check_turikan_clr()){
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						turikan_clr();
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
						}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
					}
				}
// MH321800(S) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				else if (PrintKind == PREQ_EDY_SYU_LOG
//						|| PrintKind == PREQ_SCA_SYU_LOG
				else if (PrintKind == PREQ_SCA_SYU_LOG
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						|| PrintKind == PREQ_EC_SYU_LOG) {
					// 処理なし
					// ジャーナル印字中に紙切れが発生した場合、この直後の条件によりメッセージ無しに書き換えられてしまい
					// メンテナンス画面までイベントが到達しない。
					// そのため、日毎集計ではこのパスを通しメッセージの書き換えをさせない
				}
// MH321800(E) hosoda ICクレジット対応
				else if(OPECTL.Mnt_mod != 0){
					if((OPECTL.Pri_Result == PRI_ERR_END) && (OPECTL.Pri_Kind == J_PRI)){
						if( OPECTL.Mnt_mod != 0 && OPECTL.Mnt_mod != 5 ){				// メンテナンス中
							if( !OPE_SIG_DOORNOBU_Is_OPEN ){							// ﾄﾞｱﾉﾌﾞ閉状態
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
							}
						}
						if (OPECTL.Mnt_mod) {						// メンテナンス中
							if( Ope_IsEnableDoorKnobChime() && sc.comd != KEY_MANDET ){	// ドアノブ戻し忘れチャイム有効
								Ope_StartDoorKnobTimer();
							}
							if( sc.comd != KEY_MANDET ) {			// 人体検知以外はアラーム停止する
								opa_chime(OFF);						// チャイム停止
							}
						}
						if( sc.comd == KEY_RXIIN )
						{	// RYB入力信号
							if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// 入出力信号ﾁｪｯｸ中でない
								InSignalCtrl(INSIG_OPOPEN);			//強制営業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
								InSignalCtrl(INSIG_OPCLOSE);		//強制休業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
								if( OPECTL.on_off == 0 ){			// OFF?(Y)
									sc.comd = 0;					// OFF時はﾒｯｾｰｼﾞ無しとする
								}
							}
						}else{
							if( OPECTL.on_off == 0 ){				// OFF?(Y)
								sc.comd = 0;						// OFF時はﾒｯｾｰｼﾞ無しとする
							}
						}
					}
				}
				break;

			case MSGGETHIGH(PRINTER_EVT):							// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信
				if (OPECTL.Mnt_mod) {								// メンテナンス中
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);									// チャイム停止
				}
// MH810104 GG119201(S) 電子ジャーナル対応
//				if( sc.comd == PREQ_JUSHIN_ERR1 ){					// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信
				if( sc.comd == PREQ_JUSHIN_ERR1 ||					// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信
					sc.comd == PREQ_JUSHIN_INFO ){
// MH810104 GG119201(E) 電子ジャーナル対応
					unsigned char	NEW,OLD;
					unsigned char	max;
					
					do{
						max = Printer_Status_Get( msb->msg.data[0], &NEW, &OLD );
						if( max == 0xff ) {
							// 処理対象のプリンタイベントなしの場合は処理終了
							break;
						}
						
						
						if( msb->msg.data[0] == R_PRI ){
							// ﾚｼｰﾄﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信
							
							err_pr_chk( NEW, OLD ); // ﾚｼｰﾄﾌﾟﾘﾝﾀｴﾗｰﾁｪｯｸ
							if( ( ( OLD & 0x02 ) != 0 )&&
								( ( NEW & 0x02 ) == 0 ) ){  /* 蓋閉じた? */

								if( OPE_SIG_DOOR_Is_OPEN ){					// ﾄﾞｱ開?
									FrmIrekaeTest.prn_kind = R_PRI;
									queset( PRNTCBNO, PREQ_IREKAE_TEST, sizeof(T_FrmIrekaeTest), &FrmIrekaeTest );
																					// 用紙入れ替え時テスト印字
								}
							}
						}
						else{
// MH810104 GG119201(S) 電子ジャーナル対応
							if (sc.comd == PREQ_JUSHIN_INFO) {
								err_eja_chk(NEW, OLD);	// エラーチェック
// MH810104 GG119201(S) 電子ジャーナル対応 #5944 SDカード利用不可時の精算機の警告画面が違う
								lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810104 GG119201(E) 電子ジャーナル対応 #5944 SDカード利用不可時の精算機の警告画面が違う
							}
							else {
// MH810104 GG119201(E) 電子ジャーナル対応
							// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信
							err_pr_chk2( NEW, OLD ); // ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｴﾗｰﾁｪｯｸ
// MH810104 GG119201(S) 電子ジャーナル対応
							}
// MH810104 GG119201(E) 電子ジャーナル対応
						}
					}while( max );
				}
				break;

			case MSGGETHIGH(ARC_CR_EVT):							// Reader
				opr_ctl( sc.comd );									// message分析処理
				break;

			case MSGGETHIGH(IBK_EVT):								// IBK event
				if(( sc.comd >= IBK_MIF_EVT_ERR )&&
				   ( sc.comd <= IBK_MIF_AF_NG_EVT )){
					/* Mifare event */
					memcpy( &MIF_ENDSTS.sts1, &msb->msg.data[0], sizeof( t_MIF_ENDSTS ));	// 終了ｽﾃｰﾀｽｾｯﾄ
					OpMif_ctl( sc.comd );							// message分析処理
				}
				else if(( sc.comd >= IBK_NTNET_ERR_REC )&&
					    ( sc.comd <= IBK_NTNET_BUFSTATE_CHG )){
					/* Nt-net event */
					NTNET_GetRevData( sc.comd, &msb->msg.data[0] );	// message分析(受信ﾃﾞｰﾀ解析)処理
				}
				else if( sc.comd == IBK_NTNET_CHKPASS ){
					/* 定期券問合せ結果ﾃﾞｰﾀ受信 */
					memcpy( &PassChk, &msb->msg.data[0], 22 );		// 定期券問合せ結果保存
				}
				else if(( sc.comd >= IBK_LPR_SRLT_REC )&&
					    ( sc.comd <= IBK_LPR_B1_REC )){
					/* Label printer event */
					sc.comd = OpLpr_ctl(sc.comd, msb->msg.data);	// message分析処理
				}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//				else if((sc.comd & 0xfff0) == (IBK_EDY_RCV & 0xfff0)) {
//					Edy_Ope_Recv_Ctrl( sc.comd, msb->msg.data );
//				}
//
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				break;
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//			case MSGGETHIGH(CRE_EVT):								// credit event
//				if( sc.comd == CRE_EVT_RECV ){
//					creCtrl( CRE_EVT_RECV_ANSWER );					// クレジット応答データ受信
//				}
//				else if ( sc.comd == CRE_EVT_CONN_PPP) {			// PPP接続完了
//					if( cre_ctl.Initial_Connect_Done == 0 ) {		// 初回接続(開局)未完了
//						if( CREDIT_ENABLED() ){
//							cre_ctl.OpenKind = CRE_KIND_STARTUP;	// 開局発生要因に起動時を設定
//							creCtrl( CRE_EVT_SEND_OPEN );			// 開局コマンド(01)送信.
//						}
//					}
//				}
//				break;
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)

			case MSGGETHIGH(SUICA_EVT):								// 交通系ICカードイベント
				if( SUICA_EVT == sc.comd ){							// SX-10からメッセージ受信
					Ope_Suica_Event( sc.comd, OPECTL.Ope_mod );
				}
				else if( SUICA_INITIAL_END == sc.comd ){
					if( Suica_Rec.Data.BIT.FUKUDEN_SET ){			/* 精算データ送信要求あり 		*/
// MH321800(S) hosoda ICクレジット対応
//-						Suica_Data_Snd( S_PAY_DATA,&CLK_REC);		/* SX-10締め処理実行			*/
//-						Suica_Rec.Data.BIT.FUKUDEN_SET = 0;			/* 精算データ送信要求クリア		*/
						if ( isSX10_USE() ) {
							Suica_Data_Snd( S_PAY_DATA,&CLK_REC);	/* SX-10締め処理実行			*/
							Suica_Rec.Data.BIT.FUKUDEN_SET = 0;		/* 精算データ送信要求クリア		*/
						}
// MH321800(E) hosoda ICクレジット対応
					}
					// イニシャルが完了時点で、待機で受付不可状態(受付可も送信していない)の場合は、受付可を送信する
					if( (( !Suica_Rec.Data.BIT.CTRL_MIRYO && !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ) &&
						 ( OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 ))){
// MH321800(S) T.Nagai ICクレジット対応
						if (isEC_USE()) {
							EcSendCtrlEnableData();
						}
						else {
// MH321800(E) T.Nagai ICクレジット対応
						Suica_Ctrl( S_CNTL_DATA, 0x01 );						// Suica利用を可にする
// MH321800(S) T.Nagai ICクレジット対応
						}
// MH321800(E) T.Nagai ICクレジット対応
					}
				}
				else if( SUICA_PAY_DATA_ERR == sc.comd ){
					PayDataErrDisp();
				}
				break;
			case MSGGETHIGH(REMOTE_DL_EVT):							// 遠隔ダウンロードイベント
				remote_evt_recv( sc.comd, msb->msg.data );
				break;
			case MSGGETHIGH(I2C_EVT):								// I2Cイベント
				switch( sc.comd ){
					case	I2C_RP_ERR_EVT:
					case	I2C_JP_ERR_EVT:
						I2C_Err_Regist( sc.comd, msb->msg.data[0]);
						break;
				}
				break;

			case MSGGETHIGH(SODIAC_EVT):							// SODIACイベント
				switch( sc.comd ){
					case	SODIAC_NEXT_REQ:
						AVM_Sodiac_Play_WaitReq( msb->msg.data[0], msb->msg.data[1]);
						break;
				}
				break;
			case MSGGETHIGH(SNTPMSG_EVT):
				TimeAdjustMain(sc.comd);
				break;
			case MSGGETHIGH(OPE_EVT):
				switch (sc.comd) {
				case OPE_REQ_CALC_FEE:
					// 車室情報をチェック
					if (CheckReqCalcData()) {
						sc.comd = 0;
					}
					break;
				default:
					break;
				}
				break;
// MH810103 GG119202(S) JVMAリセット処理変更
			case MSGGETHIGH(EC_EVT):
				switch (sc.comd) {
				case EC_EVT_COMFAIL:				// 通信不良発生
				case EC_EVT_NOT_AVAILABLE_ERR:		// 決済リーダ切り離しエラー発生
					if (1 <= OPECTL.Ope_mod && OPECTL.Ope_mod <= 3) {
						// 待機状態以外はJVMAリセットしない
						break;
					}
					// JVMAリセットを要求してコインメックタスクの初期化完了を待つ
					Ec_check_PendingJvmaReset();
					break;
// MH810103 GG119202(S) みなし決済扱い時の動作
				case EC_EVT_DEEMED_SETTLE_FUKUDEN:
					// みなし決済扱い（復電）の処理
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//					EcRegistSettlementAbort();
					// 決済処理失敗(休業)
// MH810103 MHUT40XX(S) E3247印字時の理由コード間違いを修正
//					EcRegistDeemedSettlementError(1);
					EcRegistDeemedSettlementError(2);
// MH810103 MHUT40XX(E) E3247印字時の理由コード間違いを修正
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
					sc.comd = 0;
					break;
// MH810103 GG119202(E) みなし決済扱い時の動作
				default:
					break;
				}
				break;
// MH810103 GG119202(E) JVMAリセット処理変更
			default:
				break;
		}
		break;
	}

	FreeBuf( msb );
	return( sc.comd );
}

//[]----------------------------------------------------------------------[]
///	@brief			金庫集計
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	void	kinko_syuukei(void)
{
	if (coin_kinko_evt == 0 && note_kinko_evt == 0 ){
		return;		// 保留イベントなし
	}
	if (OPECTL.Mnt_mod == 0 &&
		(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3)){
		return;		// 精算中はしない
	}
	if (turi_dat.turi_in != 0){
		return;		// 釣銭管理中
	}
	if (CountCheck() == 0){
		return;		// 他の合計印字中
	}
	if ( coin_kinko_evt == 1 ){
		coin_kinko_evt = 0;
		if(OPECTL.coin_syukei == 0){		// ｺｲﾝ金庫集計未出力?
			kinko_syu( 0, 1 );				// ｺｲﾝ金庫集計&印字
			OPECTL.coin_syukei = 1;			// ｺｲﾝ金庫集計出力中とする
			return;
		}
	}
	if ( note_kinko_evt == 1 ){
		note_kinko_evt = 0;
		if( OPECTL.note_syukei == 0){		// 紙幣金庫集計未出力?
			kinko_syu( 1, 1 );				// 紙幣金庫集計&印字
			OPECTL.note_syukei = 1;			// 紙幣金庫集計出力中とする
			return;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾓｼﾞｭｰﾙ起動待ち                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mojule_wait                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mojule_wait( void )
{
	unsigned char	CNMStartFlg;
	unsigned long	CNMStartTime;
	ulong req;
	long param;
// MH810103 GG119202(S) 決済リーダ起動完了待ち時間設定削除
//// MH321800(S) D.Inaba ICクレジット対応
//	unsigned long	ec_WakeUp_Timer = 0;
//// MH321800(E) D.Inaba ICクレジット対応
// MH321800 GG119202(E) 決済リーダ起動完了待ち時間設定削除
// MH321800(S) E3210登録タイミング変更
	short	timer;
// MH321800(E) E3210登録タイミング変更

	read_sht_opn();													// ﾘｰﾀﾞｰｼｬｯﾀｰ開,LED点滅

	LedReq( CN_TRAYLED, LED_ONOFF );								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点滅

	Ope_ErrAlmClear();												// NT-NET HOSTｴﾗｰ/ｱﾗｰﾑ解除ﾃﾞｰﾀ送信
	Suica_fukuden_que_check();										// 複電時の受信バッファチェック


	/*--------------------------------------------------------------*/
	/*	ﾌﾟﾘﾝﾀﾘｾｯﾄ解除 解除後のｳｪｲﾄは 最大5秒必要					*/
	/*	ﾌﾟﾘﾝﾀ復帰は正常時3秒,ｵｰﾄｶｯﾀｰが通常位置にない場合5秒必要		*/
	/*	ﾒｯｸはﾘｾｯﾄ解除するまでｳｪｲﾄ2秒必要							*/
	/*	ARCNETは起動までに最大ARC_INI_TOUT秒必要(通信異常時等)		*/
	/*--------------------------------------------------------------*/
	ExIOSignalwt(EXPORT_JP_RES, 0);									// ジャーナルプリンタリセット

//	timeout = 0;
	CNMStartFlg = 1;
// MH321800(S) Y.Tanizaki ICクレジット対応
	if( (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 6) != 0 ){		// 決済リーダ接続あり？
		CP_CN_SYN = 0;
		// タイマ時間をｾｯﾄ 10ms * (X * 100) = Xsec Wait
// MH810103 GG119202(S) 決済リーダ起動完了待ち時間設定削除
//		ec_WakeUp_Timer = (ulong)prm_get(COM_PRM, S_ECR, 20, 3, 1);
//// MH321800(S) E3210登録タイミング変更
////		if(ec_WakeUp_Timer == 0) {
////			ec_WakeUp_Timer = 300;
////		} else if(ec_WakeUp_Timer < 10){
////			ec_WakeUp_Timer = 10;
////		}
//		if (ec_WakeUp_Timer < 10) {
//			// 最小値を10秒とする
//			ec_WakeUp_Timer = 10;
//		}
//// MH321800(E) E3210登録タイミング変更
//		ec_WakeUp_Timer *= 100;
// MH810103 GG119202(E) 決済リーダ起動完了待ち時間設定削除
	}
// MH321800(E) Y.Tanizaki ICクレジット対応
	CNMStartTime = LifeTimGet();
	for( ; ; ){

		taskchg( IDLETSKNO );										// task change

		Ope_MsgBoxClear();

// MH321800(S) T.Nagai ICクレジット対応
		if( (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 6) == 0 ){		// 決済リーダ接続なし？
// MH321800(E) T.Nagai ICクレジット対応
		if( CNMStartFlg ){
			if( 200 <= LifePastTimGet( CNMStartTime ) ){			// 10ms * 200 = 2sec Wait
				CNMTSK_START = 1;									// ｺｲﾝﾒｯｸﾀｽｸ起動
				CP_CN_SYN = 1;										// ｺｲﾝﾒｯｸﾘｾｯﾄ解除
				CNMStartFlg = 0;
				// 起動までの待ち時間の改善、コインメックスタンバイOKで待ち終了とする
				break;
			}
		}
// MH321800(S) T.Nagai ICクレジット対応
		}
		else {														// 決済リーダ接続あり？
			if( CNMStartFlg == 1 ){
				// SYN信号を1秒間Lレベルにする
				if( 100 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 100 = 1sec Wait
					CP_CN_SYN = 1;									// ｺｲﾝﾒｯｸﾘｾｯﾄ解除
					CNMStartFlg = 2;
					CNMStartTime = LifeTimGet();
				}
			}
			else if( CNMStartFlg == 2 ){
				// SYN信号をHレベルにしてから4秒後にコインメックタスクを起動する
				// CN_reset()に合わせて4秒後とした
				if( 400 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 400 = 4sec Wait
					CNMTSK_START = 1;								// ｺｲﾝﾒｯｸﾀｽｸ起動
					CNMStartFlg = 0;
					CNMStartTime = LifeTimGet();
// MH321800(S) E3210登録タイミング変更
// MH810103 GG119202(S) E3210登録処理修正
//					// JVMA通信開始から3分(固定)経過しても決済リーダの初期化シーケンスが完了しない場合、
//					// E3210を登録する
					// JVMA通信開始から初期化シーケンス完了待ち時間経過しても
					// 決済リーダの初期化シーケンスが完了しない場合、E3210を登録する
// MH810103 GG119202(E) E3210登録処理修正
					timer = OPE_EC_WAIT_BOOT_TIME * 2;
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
					if (EcReaderVerUpFlg != 0) {
						timer = OPE_EC_WAIT_BOOT_TIME_VERUP * 2;
					}
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応
					LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH321800(E) E3210登録タイミング変更
// MH810103 GG119202(S) 起動初期化調整
//// GG119202(S) 決済リーダ起動完了待ち時間設定削除
//					// 決済リーダの起動完了を待たずに待機画面へ遷移する
//					break;
//// GG119202(E) 決済リーダ起動完了待ち時間設定削除
					CNMStartFlg = 3;
					CNMStartTime = LifeTimGet();
// MH810103 GG119202(E) 起動初期化調整
				}
			}
// MH810103 GG119202(S) 決済リーダ起動完了待ち時間設定削除
//			else {
//				// 決済リーダの起動を待つ
//				if( Suica_Rec.Data.BIT.INITIALIZE == 1 ){				// 決済リーダ初期化完了？
//					break;
//				}
//				if( Suica_Rec.Data.BIT.INITIALIZE != 1 && 
//					ec_WakeUp_Timer <= LifePastTimGet( CNMStartTime ) ){
//// MH321800(S) E3210登録タイミング変更
////					// E3210発生
////					err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 1, 0, 0 );
//// MH321800(E) E3210登録タイミング変更
//					break;
//				}
//			}
// MH810103 GG119202(E) 決済リーダ起動完了待ち時間設定削除
// MH810103 GG119202(S) 起動初期化調整
			else if( CNMStartFlg == 3 ){
				if( 50 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 50 = 0.5sec Wait
				// コインメックタスク起動により接続されるリーダーが決定されるため、待ち合わせる。
					break;
				}
			}
// MH810103 GG119202(E) 起動初期化調整
		}
// MH321800(E) T.Nagai ICクレジット対応
	}

	/*** ﾓｼﾞｭｰﾙ初期化 ***/
	/*--------------------------------------------------------------*/
	/*	磁気ﾘｰﾀﾞｰの初期化は正常時はﾘｰﾀﾞｰｲﾆｼｬﾙ動作完了受信にて完了。	*/
	/*	異常時はﾀｲﾑｱｳﾄを15秒にしているため最大15秒必要。			*/
	/*--------------------------------------------------------------*/

	req = 0;

	if (StartingFunctionKey == 1) {
		// F3F5が押されたときは、無条件に時計データと制御データを要求する。
		req |= NTNET_MANDATA_CLOCK;									// 時計ﾃﾞｰﾀ要求
		req |= NTNET_MANDATA_CTRL;									// 制御ﾃﾞｰﾀ要求
	} else {
		param = prm_get(0, S_NTN, 43, 1, 1);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 時計データ送信チェック
			req |= NTNET_MANDATA_CLOCK;
		}
		param = prm_get(0, S_NTN, 43, 1, 2);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 制御データ送信チェック
			req |= NTNET_MANDATA_CTRL;
		}
		param = prm_get(0, S_NTN, 43, 1, 3);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 共通設定データ送信チェック
			req |= NTNET_MANDATA_COMMON;
		}
		param = prm_get(0, S_NTN, 43, 1, 4);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 定期券ステータスデータ送信チェック
			req |= NTNET_MANDATA_PASSSTS;
		}
		param = prm_get(0, S_NTN, 44, 1, 4);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 定期券更新データ送信チェック
			req |= NTNET_MANDATA_PASSCHANGE;
		}
		param = prm_get(0, S_NTN, 43, 1, 6);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 定期券出庫時刻データ送信チェック
			req |= NTNET_MANDATA_PASSEXIT;
		}
		param = prm_get(0, S_NTN, 44, 1, 1);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 車室パラメータデータ送信チェック
			req |= NTNET_MANDATA_LOCKINFO;
		}
		param = prm_get(0, S_NTN, 44, 1, 2);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ロック装置データ送信チェック
			req |= NTNET_MANDATA_LOCKMARKER;
		}
		param = prm_get(0, S_NTN, 43, 1, 5);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 定期券中止データ送信チェック
			req |= NTNET_MANDATA_PASSSTOP;
		}
		param = prm_get(0, S_NTN, 44, 1, 6);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// 特別日設定データ送信チェック
			req |= NTNET_MANDATA_SPECIALDAY;
		}
	}
	NTNET_Snd_Data104(req);											// NT-NET管理ﾃﾞｰﾀ要求作成

	NTNET_Snd_Data190();
	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御

	LedReq( CN_TRAYLED, LED_OFF );									// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 非初期化ｴﾘｱｸﾘｱ                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : memclr                                                  |*/
/*| PARAMETER    : flg = 1:無条件でｸﾘｱ                                     |*/
/*| RETURN VALUE : 0:saved memory  1:memory all clear                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	memclr( char flg )
{
	char	ret;
	ushort	i;

	if (memcmp(PASSPT.pas_word, PASSDF.pas_word, sizeof(PASSPT.pas_word)) != 0) {
		f_ParaUpdate.BYTE = 0;										// メモリ破損のため、更新フラグをクリア
	}

	if( flg != 0 )													// 無条件ｸﾘｱ?
	{
		ret = 1;
		memset( &FLAGPT, 0, sizeof( flg_rec ) );
		memset( &FLAPDT, 0, sizeof( flp_rec ) );
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			FLAPDT.flp_data[i].timer = -1;							// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ、ﾗｸﾞﾀｲﾑﾀｲﾏｰｸﾘｱ
			FLPCTL.Flp_mv_tm[i] = -1;								// ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰｸﾘｱ
			FLPCTL.Flp_uperr_tm[i] = -1;							// 上昇ﾛｯｸ(閉異常)ｴﾗｰ判定ﾀｲﾏｰｸﾘｱ
			FLPCTL.Flp_dwerr_tm[i] = -1;							// 下降ﾛｯｸ(開異常)ｴﾗｰ判定ﾀｲﾏｰｸﾘｱ
			FLAPDT.flp_data[i].in_chk_cnt = -1;						// 入庫判定ﾀｲﾏｰｸﾘｱ
		}
		memset( FLAPDT_SUB, 0, sizeof( flp_com_sub )*10 );			// ﾌﾗｯﾌﾟｻﾌﾞｴﾘｱｸﾘｱ
		memset( Mov_cnt_dat, 0, sizeof( Mov_cnt_dat ) );
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//		memset( pas_tbl, 0, sizeof( PAS_TBL )*PAS_MAX );			// 定期券ﾃｰﾌﾞﾙｸﾘｱ
//		memset( pas_renewal, 0, sizeof( PAS_TBL )*(PAS_MAX/4) );	// 定期券更新ﾃｰﾌﾞﾙｸﾘｱ
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
		memset( syusei, 0, sizeof( struct SYUSEI )*LOCK_MAX );
		memset( &tki_cyusi, 0, sizeof( t_TKI_CYUSI ) );				// 定期券中止ﾊﾞｯﾌｧｸﾘｱ
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//		memset( &pas_extimtbl, 0, sizeof( PASS_EXTIMTBL ) );		// 定期券出庫時刻ﾃｰﾌﾞﾙｸﾘｱ
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
		memset( NTNetDataCont, 0, sizeof( NTNetDataCont ) );		// 各ﾃﾞｰﾀ追い番
		memset( UketukeNoBackup, 0, sizeof( UketukeNoBackup ) );	// 受付券発行追番ﾊﾞｯｸｱｯﾌﾟｸﾘｱ
		memset( &CarCount, 0, sizeof( CAR_COUNT ) );				// 入出庫台数ｶｳﾝﾄｴﾘｱｸﾘｱ
		memset( &CarCount_W, 0, sizeof( CAR_COUNT ) );				// 入出庫台数ｶｳﾝﾄﾜｰｸｴﾘｱｸﾘｱ
		syu_init();
		cnm_mem( 0 );
		memset( &ac_flg, 0, sizeof(struct AC_FLG) );				// 0:待機状態
		memset( &Attend_Invalid_table, 0, sizeof( Attend_Invalid_table ) );				// 係員無効ﾃｰﾌﾞﾙ
// MH810100(S) Y.Yamauchi  2019/12/26 車番チケットレス(メンテナンス)
		memset( &ticketdata, 0, sizeof( ticketdata ) );				// 係員無効ﾃｰﾌﾞﾙ
// MH810100(E) Y.Yamauchi  2019/12/26 車番チケットレス(メンテナンス)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		LaneStsSeqNo = 0;											// 状態一連番号クリア
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

		LKcom_PassBreak();

		memcpy( PASSPT.pas_word, PASSDF.pas_word, sizeof( PASSPT.pas_word ) );	// ﾊﾟｽﾜｰﾄﾞ[AMANO]ｾｯﾄ
		auto_syu_prn	= 0;	// 自動集計状態		初期化
		auto_syu_ndat	= 0;	// 自動集計開始日付	初期化
		coin_kinko_evt = 0;		// コイン金庫イベント
		note_kinko_evt = 0;		// 紙幣金庫イベント
		Mifare_WrtNgDataUpdate( 1, (t_MIF_CardData*)0L );			// Mifare書込み失敗ｶｰﾄﾞ情報ｸﾘｱ
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//		NTBUF_AllClr();												//NT-NETバッファ内のデータを全て削除
		NTBUF_AllClr_startup();										//NT-NETバッファ内のデータを全て削除
		NTCom_ClearData(0);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		auto_cnt_prn	= 0;	// 自動センター通信状態		初期化
//		auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);		// 最終センター通信実施日時に現在時間ｾｯﾄ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//		memset( &cre_saleng, 0, sizeof( cre_saleng ) );				// クレジット売上拒否データ
//		memset( &cre_uriage, 0, sizeof( cre_uriage ) );		// クレジット売上収集チェックエリア	
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

// MH321800(S) G.So ICクレジット対応 不要機能削除(CCT)
//		memset( &ClkLastSend, 0, sizeof( ClkLastSend ) );	// カード情報要求最終送信時刻
// MH321800(E) G.So ICクレジット対応 不要機能削除(CCT)
		creCtrlInit( flg );

// MH321800(S) hosoda ICクレジット対応
		Product_Select_Data = 0;							// 送信した選択商品データ(決済時の比較用)
// MH321800(E) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		Edy_OpeDealNo = 0;					// Main管理用Edy取引通番
//		Edy_ShimeDataClear();								// EM締めﾃﾞｰﾀ一時保存ｴﾘｱｸﾘｱ
//#endif
//		memset( &PayData_save, 0, sizeof(PayData_save) );	// 停電前PayData保存域ｸﾘｱ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		memset( &ETC_cache, 0xFF, sizeof(ETC_cache));
		memset( &bk_LockInfo, 0, sizeof(bk_LockInfo));
		memset( &CAL_WORK, 0, sizeof( t_cal_bak_rec ));							/* 遠隔用ﾃﾞｰﾀ格納ｴﾘｱ	  byte	*/

		ntnet_nmax_flg = 0;							/*　最大料金ﾌﾗｸﾞｸﾘｱ	*/

		memset( LongTermParkingPrevTime, 0, sizeof( LongTermParkingPrevTime ));
// GM760201(S) 設定アップロード中の電源OFF/ONでAMANO画面から進まないタイミングがあるため、修正
//		remotedl_info_clr();
// GM760201(E) 設定アップロード中の電源OFF/ONでAMANO画面から進まないタイミングがあるため、修正
// GG129000(S) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
		remotedl_info_clr();
// GG129000(E) R.Endo 2023/01/11 車番チケットレス4.0 #6774 プログラムダウンロード処理中の電源断で再起動後にリトライせず [共通改善項目 No1537]
		memset(&Chk_info, 0, sizeof(Chk_info));
		for(i=0;i<RT_PAY_LOG_CNT;i++){
			memset(&RT_PAY_LOG_DAT.RT_pay_log_dat[i], 0 ,sizeof(Receipt_data));
		}
		RT_PAY_LOG_DAT.RT_pay_count = 0;
		RT_PAY_LOG_DAT.RT_pay_wtp = 0;
		memset( Encryption_Key[0], 0, sizeof(Encryption_Key[0]) * ENCKEY_NUM );
		// センター追番初期化
		memset(Rau_SeqNo, 0, sizeof(Rau_SeqNo));
		for(i = 0; i < RAU_SEQNO_TYPE_COUNT; ++i) {
			Rau_SeqNo[i] = 1;
		}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
		// DC-NET通信用センター追番初期化
		memset( DC_SeqNo, 0, sizeof(DC_SeqNo) );
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
//		for( i = 0; i < DC_SEQNO_TYPE_COUNT; ++i ){
//			DC_SeqNo[i] = 1;
//		}
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
		memcpy( &DC_SeqNo_wk, &DC_SeqNo, sizeof( DC_SeqNo_wk ) );
		// リアルタイム通信用センター追番初期化
		memset( REAL_SeqNo, 0, sizeof(REAL_SeqNo) );
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
//		for( i = 0; i < REAL_SEQNO_TYPE_COUNT; ++i ){
//			REAL_SeqNo[i] = 1;
//		}
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//日付切替基準の小計(リアルタイムデータ用)
		Date_Syoukei.Date_Time.Hour = 0xFF;
		Date_Syoukei.Date_Time.Min  = 0xFF;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応

// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));	// Edy上位端末ID
		EcAlarmLog_Clear();
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
		memset(RecvBrandTbl, 0, sizeof(RecvBrandTbl));
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810104 GG119201(S) 電子ジャーナル対応
		memset(&eja_prn_buff, 0, sizeof(eja_prn_buff));
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
		EcReaderVerUpFlg = 0;
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		prn_job_id = 0;
		memset(&prn_dat_que, 0, sizeof(prn_dat_que));
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		memset(&pri_rcv_buff_r, 0, sizeof(PRN_RCV_DATA_BUFF_R));
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	}else{
		ret = 0;
		if( memcmp( PASSPT.pas_word, PASSDF.pas_word, sizeof( PASSPT.pas_word )) == 0 ){
			// ﾊﾟｽﾜｰﾄﾞﾁｪｯｸOK
			nmitrap();
		}
		cnm_mem( 1 );
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			FLPCTL.Flp_mv_tm[i] = -1;								// ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰｸﾘｱ
		}
		creCtrlInit( flg );
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		memcpy( &PayData_save, &PayData, sizeof(PayData_save) );	// 停電前PayData保存
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		if(	2 == auto_syu_prn ){									// 「2:集計印字中」
			auto_syu_prn = 0;										// 0:空き（集計時刻待ち）
		}
// MH810100(S) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		Remote_Cal_Data_Restor();
// MH810100(E) K.Onodera  2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
// MH810100(S) K.Onodera  2020/04/02 #4092 車番チケットレス(実行してもバックアップした内容に戻らない)
//		for( i = 0; i < DC_SEQNO_TYPE_COUNT; ++i ){
//			if( DC_SeqNo[i] != DC_SeqNo_wk[i] ){
//				DC_SeqNo[i] = DC_SeqNo_wk[i];
//			}
//		}
// MH810100(E) K.Onodera  2020/04/02 #4092 車番チケットレス(実行してもバックアップした内容に戻らない)
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
	}
	for( i = 0; i < BIKE_START_INDEX; i++ ){									/* フラップ */
		if(LockInfo[i].lok_syu == 0){
			if( FLAPDT.flp_data[i].nstat.bits.b04 == 1 )						/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
			{																	/*								*/
				FLAPDT.flp_data[i].nstat.bits.b04 = 0;							/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
			}																	/*								*/
			if( FLAPDT.flp_data[i].nstat.bits.b05 == 1 )						/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
			{																	/*								*/
				FLAPDT.flp_data[i].nstat.bits.b05 = 0;							/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
			}
			FLAPDT.flp_data[i].nstat.bits.b12 = 0;		// 上昇異常回復
			FLAPDT.flp_data[i].nstat.bits.b13 = 0;		// 下降異常回復
		}
	}
	for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){								/* ロック装置 */
		WACDOG;																	// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if(LockInfo[i].lok_syu == 0){
		if( FLAPDT.flp_data[i].nstat.bits.b04 == 1 )							/* 上昇ﾛｯｸ中(閉ﾛｯｸ中)			*/
		{																		/*								*/
			FLAPDT.flp_data[i].nstat.bits.b04 = 0;								/* 上昇ﾛｯｸ解除(閉ﾛｯｸ解除)		*/
		}																		/*								*/
		if( FLAPDT.flp_data[i].nstat.bits.b05 == 1 )							/* 下降ﾛｯｸ中(開ﾛｯｸ中)			*/
		{																		/*								*/
			FLAPDT.flp_data[i].nstat.bits.b05 = 0;								/* 下降ﾛｯｸ解除(開ﾛｯｸ解除)		*/
		}
		FLAPDT.flp_data[i].nstat.bits.b12 = 0;		// 上昇異常回復
		FLAPDT.flp_data[i].nstat.bits.b13 = 0;		// 下降異常回復
		}
	}

	SetTaskID( 0xFF );				// TaskIDの初期化

	memset( &CN_RDAT.r_dat09[0], 0x01, 4 ); /* 釣り切れ状態回避の為 */
	flp_DownLock_Initial();														/* 下降ﾛｯｸﾀｲﾏｰｴﾘｱ初期化		*/
	SetDiffLogReset((uchar)flg);												// 設定変更履歴情報初期化処理
	memset( &TempFus, 0, sizeof( TempFus ));
	CountInit((uchar)flg);			// 追い番初期化
	turi_kan_f_defset_wait = TURIKAN_DEFAULT_NOWAIT;	//デフォルトセット待ちフラグクリア
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
	memset( LongParkingFlag, 0, sizeof( LongParkingFlag ) );//長期駐車状態管理フラグクリア
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
	memset( KSG_RauBkupAPNName, 0xFF, sizeof( KSG_RauBkupAPNName ) );// APNの設定値の比較用バックアップ クリア（設定アドレスのデータは0の可能性があるのでFFで初期化する）
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 立上り時画面表示                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : start_dsp                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : ﾌｫﾝﾄ変更に伴う修正 2005-08-31 Hashimoto                 |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	start_dsp( void )
{
	unsigned char	buf[16];

	blightLevel = 0xFF;
	blightMode = 0xFF;

	// 起動時の輝度値は必ずデフォルト値をセットするように統一する。
	// 起動画面後の次の画面へ移行するタイミングで
	// 本来の設定値をセットし直す。
	// (02-0027が異常値だった場合の値の修正は行わない)
	backlight = LCD_LUMINE_DEF;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	lcd_contrast( backlight );										// バックライト輝度設定
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）

	lcd_backlight( ON );											// back light ON
	dsp_background_color(COLOR_BLACK);
	as1chg( VERSNO.ver_part, buf, 8 );								// ｿﾌﾄﾊﾞｰｼﾞｮﾝ(ｱｽｷｰ)を表示文字ｺｰﾄﾞへ変換する
	grachr( 1, 7, 16, 0, COLOR_WHITE, LCD_BLINK_OFF, (uchar*)buf );	// ｿﾌﾄﾊﾞｰｼﾞｮﾝ表示

	grachr( 5, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[7] );	// "     しばらくお待ち下さい     "
}

/*[]----------------------------------------------------------------------[]*/
/*| 時計ﾃﾞｰﾀﾁｪｯｸ(CLK_RECのﾁｪｯｸ)                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : ClkrecUpdate( void )                                     |*/
/*| PARAMETER   : void  :                                                  |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimo                                                |*/
/*| Date        : 2005-07-12                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	DateTimeCheck( void )
{
	if( (CLK_REC.year < 1990)||(CLK_REC.year > 2050) ){
		return( -1 );
	}
	if( (CLK_REC.mont < 1)||(CLK_REC.mont > 12) ){
		return( -1 );
	}
	if( (CLK_REC.date < 1)||(CLK_REC.date > 31) ){
		return( -1 );
	}
	if( (CLK_REC.hour > 23) ){
		return( -1 );
	}
	if( (CLK_REC.minu > 59) ){
		return( -1 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| 自動集計処理　　　　　　　　　　　　　　　　　　　　                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : auto_syuukei                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  K.Motohashi                                            |*/
/*| Date         :  2005-09-01                                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	auto_syuukei( void )
{
	uchar			syu_time_h10;	// 自動集計時刻（時：１０位）
	uchar			syu_time_h01;	// 自動集計時刻（時：１位）
	uchar			syu_time_m10;	// 自動集計時刻（分：１０位）
	uchar			syu_time_m01;	// 自動集計時刻（分：１位）
	ushort			syu_time;		// 自動集計時刻（変換ﾃﾞｰﾀ）
	T_FrmSyuukei	FrmSyuukei;		// 集計印字要求ﾒｯｾｰｼﾞ作成ｴﾘｱ

	if(( auto_syu_prn == 3 ) &&	( auto_syu_ndat != CLK_REC.ndat )){

		// 自動集計処理完了し日付が替わった場合
		auto_syu_prn = 0;														// 集計状態 ← 空き（集計時刻待ち）
	}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( auto_cnt_prn == 2 )
//		return;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	if( ( prm_get( COM_PRM, S_TOT, 8, 1, 1 ) != 0 ) && ( auto_syu_prn == 0 ) ){

		// 自動集計＝する　且つ　集計時刻待ち　の場合

		syu_time_h10 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 4 );				// 自動集計時刻（時：１０位）設定ﾃﾞｰﾀ取得
		syu_time_h01 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 3 );				// 自動集計時刻（時：１位）　設定ﾃﾞｰﾀ取得
		syu_time_m10 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 2 );				// 自動集計時刻（分：１０位）設定ﾃﾞｰﾀ取得
		syu_time_m01 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 1 );				// 自動集計時刻（分：１位）　設定ﾃﾞｰﾀ取得

		syu_time =	(syu_time_h10*600) +										// 自動集計時刻ﾃﾞｰﾀを現在時刻との比較用に変換
					(syu_time_h01*60)  +
					(syu_time_m10*10)  +
					syu_time_m01;

		if( syu_time <= CLK_REC.nmin ){											// 時刻一致もしくはすでに実行時刻が経過していた場合
			// 自動集計時刻になった場合
			auto_syu_prn = 1;													// 集計状態 ← 集計開始要求
			auto_syu_ndat = CLK_REC.ndat;										// 集計日付ｾｯﾄ
		}
	}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( ( auto_syu_prn == 1 ) && ( OPECTL.Mnt_mod == 0 ) && ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ) && 
//		(auto_cnt_prn != 2) ){	// 自動ｾﾝﾀｰ通信実施中でない
	if( ( auto_syu_prn == 1 ) && ( OPECTL.Mnt_mod == 0 ) && ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ) ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		if (AppServ_IsLogFlashWriting(eLOG_TTOTAL) != 0)
			return;																// 書込み中なら自動T集計を実施しない

		if (CountCheck() == 0){
			return;				// 金庫集計中なら待つ
		}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		auto_cnt_prn = 0;		// 自動ｾﾝﾀｰ通信を行わない（集計実施後に行うため）
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		// 集計開始要求　且つ　ﾒﾝﾃﾅﾝｽ操作中／精算操作中でない場合（自動集計を行う）

		if( prm_get( COM_PRM, S_TOT, 8, 1, 2 ) == 1 ){							// 金庫集計する／しない？

			// 自動集計で金庫集計をする場合
			kinko_syu( 0, 0 );													// ｺｲﾝ金庫集計（印字なし）
			kinko_syu( 1, 0 );													// 紙幣金庫集計（印字なし）
		}
		FrmSyuukei.prn_data = (void*)syuukei_prn(PREQ_AT_SYUUKEI, &sky.tsyuk);
		memcpy( &sky.tsyuk.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// 現在時刻
		FrmSyuukei.prn_kind = R_PRI;											// ﾌﾟﾘﾝﾀ種別
// MH810105 GG119202(S) T合計連動印字対応
		FrmSyuukei.print_flag = 0;
		if (isEC_USE()) {
			// 決済リーダ接続ありでT合計印字の場合は設定を参照して連動印字を行う
			ec_linked_total_print(PREQ_AT_SYUUKEI, &FrmSyuukei);
		}
		else {
// MH810105 GG119202(E) T合計連動印字対応
		queset( PRNTCBNO, PREQ_AT_SYUUKEI, sizeof(T_FrmSyuukei), &FrmSyuukei );	// 自動集計印字要求
// MH810105 GG119202(S) T合計連動印字対応
		}
// MH810105 GG119202(E) T合計連動印字対応
		wopelg( OPLOG_T_GOKEI, 0, 0 );											// 操作履歴登録（T合計）

		auto_syu_prn = 2;														// 集計状態 ← 集計印字中

	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰ/ｱﾗｰﾑ解除登録                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  okuda                                                  |*/
/*| Date         :  2005-12-13                                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_ErrAlmClear( void )
{
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	/* 親機へｸﾘｱ電文送信 */
//	NTNET_Snd_Data120_CL();											// NT-NET HOSTｴﾗｰ解除ﾃﾞｰﾀ送信
//	NTNET_Snd_Data121_CL();											// NT-NET HOSTｱﾗｰﾑ解除ﾃﾞｰﾀ送信
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

	/* 送信したことをLOGに残す */

	/* ｴﾗｰLOG登録 */
	memcpy( &Err_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
	Err_work.Errsyu = 255;											// ｴﾗｰ種別
	Err_work.Errcod = 255;											// ｴﾗｰｺｰﾄﾞ
// MH322915(S) K.Onodera 2017/05/22 全解除エラーステータス
//	Err_work.Errdtc = 2;											// ｴﾗｰ発生/解除
	Err_work.Errdtc = 0;											// ｴﾗｰ解除
// MH322915(E) K.Onodera 2017/05/22 全解除エラーステータス
	Err_work.Errlev = 5;											// ｴﾗｰﾚﾍﾞﾙ
	// 解除ｲﾍﾞﾝﾄはﾄﾞｱ閉にして送る
	Err_work.ErrDoor = 0;											// ﾄﾞｱ閉状態とする
	Err_work.Errinf = 0;											// ｴﾗｰ情報有無
	Log_regist( LOG_ERROR );										// ｴﾗｰﾛｸﾞ登録

	/* ｱﾗｰﾑLOG登録 */
	memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
	Arm_work.Armsyu = 255;											// ｱﾗｰﾑ種別
	Arm_work.Armcod = 255;											// ｱﾗｰﾑｺｰﾄﾞ
// MH322915(S) K.Onodera 2017/05/22 全解除エラーステータス
//	Arm_work.Armdtc = 2;											// ｱﾗｰﾑ発生/解除
	Arm_work.Armdtc = 0;											// ｱﾗｰﾑ解除
// MH322915(E) K.Onodera 2017/05/22 全解除エラーステータス
	Arm_work.Armlev = 5;											// ｱﾗｰﾑﾚﾍﾞﾙ
	// 解除ｲﾍﾞﾝﾄはﾄﾞｱ閉にして送る
	Arm_work.ArmDoor = 0;											// ﾄﾞｱ閉状態とする
	Arm_work.Arminf = 0;											// ｱﾗｰﾑ情報有無
	Log_regist( LOG_ALARM );										// ｱﾗｰﾑﾛｸﾞ登録

// GM849100(S) 名鉄協商コールセンター対応　アラームデータをエラー９６ｘｘで送信
	// AFFFFはalm_chk()で登録していないので、ここでE9699(全アラーム解除を)登録する
	err_chk2(ERRMDL_ALARM, (char)99, 2, 0, 0, NULL);
// GM849100(E) 名鉄協商コールセンター対応　アラームデータをエラー９６ｘｘで送信

	IFM_Snd_ErrorClear();											// IFMへ全エラー解除通知
}

/*[]-----------------------------------------------------------------------[]*/
/*| Opetask受信メールをクリア（廃棄）する                                   |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	RYB信号入力情報だけは最新の状態を反映する。								|*/
/*|	（現状PowerON後のOpetask起動前のみCallされる）							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-09-27                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_MsgBoxClear( void )
{
	MsgBuf	*msb;
	ushort	wks;

	if( (msb = GetMsg( OPETCBNO )) != NULL ){					// ﾒｯｾｰｼﾞあり?

		wks = msb->msg.command;									// Mail-ID get
		if ((wks == IBK_NTNET_DAT_REC ) ||						// NT-NETﾒｯｾｰｼﾞ受信
				  (wks == IFM_RECV ) ||							// IFMﾒｯｾｰｼﾞ受信
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				  (wks == IBK_EDY_RCV ) ||						// Edyﾒｯｾｰｼﾞ受信
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				  (wks == SUICA_INITIAL_END ) ||				// Suica初期化完了ｲﾍﾞﾝﾄ
				  (wks == SUICA_EVT ) ||						// Suica初期化完了ｲﾍﾞﾝﾄ
// MH810103 GG119202(S) みなし決済扱い時の動作
				  (wks == EC_EVT_DEEMED_SETTLE_FUKUDEN) ||		// みなし決済扱い（復電）
// MH810103 GG119202(E) みなし決済扱い時の動作
				  (wks == IBK_NTNET_FREE_REC ) ||				// NTNET FREEﾊﾟｹｯﾄﾃﾞｰﾀ受信
				  (wks == IBK_NTNET_BUFSTATE_CHG ) ||			// NTNET ﾊﾞｯﾌｧ状態変化
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				  (wks == CRE_EVT_CONN_PPP ) ||					// クレジットPPP接続イベント
//				  (wks == IBK_EDY_ERR_REC )) {					// Edyｴﾗｰﾒｯｾｰｼﾞ受信
				  (wks == CRE_EVT_CONN_PPP )) {					// クレジットPPP接続イベント
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			PutMsg( OPETCBNO, msb );							// 受信ﾒｯｾｰｼﾞ再登録
			return;												// ﾒｯｾｰｼﾞ解放しない
		}
		FreeBuf( msb ); 										// ﾒｯｾｰｼﾞ解放(読み捨てる)
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| RYB入力情報初期化                                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PowerON後 ｷ-入力初期値（現在値）を読んでから定期ｾﾝｽを始める前に			|*/
/*|	key_dat[] から情報を作成する。											|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-09-27                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_InSigSts_Initial( void )
{

	//汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄがONの場合
	if( RXM1_IN == 1 || RXI1_IN	== 1)
	{
		InSignalCtrl(INSIG_OPOPEN);		//強制営業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
		InSignalCtrl(INSIG_OPCLOSE);	//強制休業が共通パラメータの汎用入力ﾎﾟｰﾄもしくは予備入力ﾎﾟｰﾄに登録されている場合はメインにイベントを通知する。
	}
}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*| Ｅｄｙ自動センター通信機能							                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : auto_centercomm                                         |*/
///*| PARAMETER    : void                                                    |*/
///*| PARAMETER    : uchar execkind  1：手動T集計 2：自動T集計 3:自動ｾﾝﾀｰ通信|*/
///*| RETURN VALUE : 1=自動ｾﾝﾀｰ通信を実行した、0=してない                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :  suzuki		                                           |*/
///*| Date         :  2007-02-26                                             |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//uchar	auto_centercomm( uchar execkind )
//{
//	uchar			w_time_h10;				// 自動通信時刻	（時：１０位）
//	uchar			w_time_h01;				// 				（時：１位）
//	uchar			w_time_m10;				// 				（分：１０位）
//	uchar			w_time_m01;				// 				（分：１位）
//	ushort			w_time;					// 				（変換ﾃﾞｰﾀ）
//	uchar			w_exec_flg;				// work（実施の要:1/否:0）
//	uchar 			w_shime_kind=0;			// 0：ｾﾝﾀｰ通信のみ 1：ｾﾝﾀｰ通信＆締め処理
//
//	if( auto_cnt_prn == 2 )
//		return	0;
//
//	// 自動センター通信「する」設定で場合はセンター通信開始の要否を判別する
//		if( (auto_syu_prn == 1) || (auto_syu_prn == 2) ){						// 自動集計実施待ちor実施中
//			auto_cnt_prn = 0;		// 自動ｾﾝﾀｰ通信を行わない（集計実施後に行うため）
//			return	0;
//		}
//
//		if( Edy_Rec.edy_status.BIT.INITIALIZE == 0 )							// Edyの初期設定が完了していない場合は通信しない
//			return 0; 										// 起動時のｾﾝﾀｰ通信は、次の１分歩進時に判定する
//
//		w_exec_flg = autocnt_execchk( execkind );								// 自動センター通信実施条件ﾁｪｯｸ (要:1/否:0）
//		if( w_exec_flg == 2 && execkind != 3 )									// 指定時間設定かつ１分歩進時の通信要求でない場合
//			return 0;
//
//		if( auto_cnt_prn == 0 ){												// 実行予約の無い時
//																				// 自動通信センター開始時刻 設定ﾃﾞｰﾀ取得
//			w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 4 );				// （時：１０位）
//			w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 3 );				// （時：１位）
//			w_time_m10 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 2 );				// （分：１０位）
//			w_time_m01 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 1 );				// （分：１位）
//			w_time =	(w_time_h10*600) +											// 自動集計時刻ﾃﾞｰﾀを現在時刻との比較用に変換
//						(w_time_h01*60)  +
//						(w_time_m10*10)  +
//						w_time_m01;
//
//			if( execkind != 3 || ((w_time == CLK_REC.nmin) ||						// 手動・自動T合計後の要求か指定時間か
//				( ( prm_get( COM_PRM, S_SCA, 58, 1, 5 ) == 0 ) && (fg_autocnt_fstchk == 0) && (w_time < CLK_REC.nmin)) )) {			// 立ち上がり時に指定時間を経過していた場合
//
//				fg_autocnt_fstchk = 1;												// 立上げ時のEdy自動センター通信要否確認フラグ（確認済）
//
//				if(w_exec_flg == 1 || ( execkind == 3 && w_exec_flg == 2 )) {		// センター通信実施条件成立？
//					auto_cnt_prn = 1;												// 自動センター通信実行待ち
//				}
//			}
//		}
//
//	// 自動センター通信開始要求　且つ　ﾒﾝﾃﾅﾝｽ操作中／精算操作中でない場合（自動センター通信を行う）
//		if( auto_cnt_prn == 1 ){ 													// ｾﾝﾀｰ通信実施要求あり
//			if( auto_cntcom_phase_check( &execkind ) ){
//				switch( execkind ){
//					case 1:																// 手動T集計後のｾﾝﾀｰ通信処理
//						wmonlg( OPMON_EDY_MT_CEN_START, 0, 0 );							// ﾓﾆﾀLOG登録（センター通信開始）
//						w_shime_kind = 1;												// ｾﾝﾀｰ通信＆締め処理実行
//						break;
//					case 2:																// 自動T集計後のｾﾝﾀｰ通信処理
//						wmonlg( OPMON_EDY_AT_CEN_START, 0, 0 );							// ﾓﾆﾀLOG登録（センター通信開始）
//						w_shime_kind = 1;												// ｾﾝﾀｰ通信＆締め処理実行
//						break;
//					case 3:																// 指定時間でのｾﾝﾀｰ通信
//						wmonlg( OPMON_EDY_A_CEN_START, 0, 0 );							// ﾓﾆﾀLOG登録（センター通信開始）
//						execkind = 2;
//						if( w_exec_flg == 2 ){											// 指定時間で締め実行条件？
//							w_shime_kind = 1;											// ｾﾝﾀｰ通信＆締め処理実行
//						}else{
//							w_shime_kind = 0;											// ｾﾝﾀｰ通信のみ
//						}
//						break;
//				}
//				Edy_CentComm_sub(execkind, w_shime_kind );								// センター通信処理実施（締め処理＆画面表示なし指定）
//				if( edy_cnt_ctrl.BIT.exec_status ){										// 通信開始
//					auto_cnt_prn = 2;													// 通信中フラグをセット
//				}else{																	// 通信未実施（エラー発生等）
//					auto_cnt_prn = 0;													// 通信状態フラグをクリア
//				}
//				return	1;
//			}
//		}
//	return	0;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| Ｅｄｙ自動センター実施要否判定処理					                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : autocnt_execchk	                                       |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : void                                                    |*/
///*| RETURN VALUE : uchar w_exec_flg						  				   |*/
///*|					  0：通信しない						  				   |*/
///*|                   1：通信実行(締めを実行するかどうかは呼び元で判断)	   |*/
///*|					  2：ｾﾝﾀｰ通信＆締め実行(指定時間設定時専用)			   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :  suzuki		                                           |*/
///*| Date         :  2007-02-26                                             |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//uchar	autocnt_execchk( uchar type )
//{
//	uchar			w_time_h10;				// 自動通信時刻	（時：１０位）
//	uchar			w_time_h01;				// 				（時：１位）
//	uchar			w_time_m10;				// 				（分：１０位）
//	uchar			w_time_m01;				// 				（分：１位）
//	ushort			wk_s1, wk_s2;			// work（未送信取引ログ件数）
//	t_NrmYMDHM		wl_Time1, wl_Time2;		// work（積算時刻）
//	uchar			w_exec_flg = 0;			// work（実施の要:1/否:0）
//
//	if( prm_get( COM_PRM, S_SCA, 57, 1, 6 ) == 1 ){						// Edy締め実行ﾀｲﾐﾝｸﾞが指定時間の場合
//		w_exec_flg = 2;													// 自動センター通信実施 (要:1）
//	}else{																// Edy締め実行ﾀｲﾐﾝｸﾞがT合計時の場合
//		if( type != 3 ){													// ｾﾝﾀｰ通信実行種別が自動実行でない場合
//			w_exec_flg = 1;													// ｾﾝﾀｰ通信&締めを実行
//			return w_exec_flg;
//		}
//		// 実施条件が「未送信LOG件数オーバー」の場合
//		if( prm_get( COM_PRM, S_SCA, 57, 1, 5 ) == 1 ){
//			if(Edy_Rec.edy_status.BIT.INITIALIZE == 1) {					// Edyﾓｼﾞｭｰﾙの初期化が完了しているか？
//				wk_s1 = (ushort)bcdbin2(OpeEmStatusInfo.NotSnd_Log_Num);	// 現在未送信取引ログ件数
//				w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 4 );	// 限界未送信取引ログ件数設定値（1000位）
//				w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 3 );	// （100位）
//				w_time_m10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 2 );	// （10位）
//				w_time_m01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 1 );	// （1位）
//				wk_s2 =	(w_time_h10*1000) +	(w_time_h01*100) + (w_time_m10*10) + w_time_m01;
//				if(wk_s1 >= wk_s2) {
//					w_exec_flg = 1;											// 自動センター通信実施 (要:1）
//				}
//			}
//		}
//
//		// 実施条件が「前回通信からの経過時間オーバー」の場合
//		else if( prm_get( COM_PRM, S_SCA, 57, 1, 5 ) == 2 ){
//			wl_Time1.ul = Nrm_YMDHM((date_time_rec *)&CLK_REC);				// wl_Time1 = 現在時刻
//			w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 2 );		// 経過期限設定日数値（10位）
//			w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 1 );		// 					 （ 1位）
//			if(	auto_cnt_ndat > wl_Time1.ul){ 								// 精算機時刻更新等による矛盾時
//				w_exec_flg = 1;												// 自動センター通信実施 (要:1）
//			}
//			else{
//				wl_Time2.ul = auto_cnt_ndat;								// 前回センター通信日時get
//				wl_Time2.us[0] += ((w_time_h10*10) + w_time_h01);			// wl_Time2 = 次回通信開始予定時間
//
//				if( wl_Time1.ul >= wl_Time2.ul ){							// 予定時間が過ぎている
//					w_exec_flg = 1;											// 自動センター通信実施 (要:1）
//				}
//			}
//		}
//	}
//	return(w_exec_flg);
//}
//#endif
////[]----------------------------------------------------------------------[]
/////	@brief			Edyセンター通信実施可能チェック
/////	@param[in]		execkind	1：手動T集計<br>
/////								2：自動T集計<br>
/////								3：自動ｾﾝﾀｰ通信
/////	@return			0：実行不可<br>
/////					1：実行可<br>
/////	@author			Namioka
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2008/12/03<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//static char auto_cntcom_phase_check(uchar *execkind )
//{
//	char ret = 0;
//	if( *execkind == 1 || 															// 手動T合計後の通信要求又は
//	  ( ( *execkind == 2 || *execkind == 3 ) && 									// 自動T合計後の通信要求又は指定時間での通信要求で
//	  	(OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100)))){	// 待機または休業中の場合
//	  	ret = 1;																	// 実行可
//	  	if( *execkind != 1 && edy_cnt_ctrl.BIT.wait_kind ){											// 前回実行フェーズ外で分歩進待ちしている場合
//	 		*execkind = edy_cnt_ctrl.BIT.wait_kind;									// 実行種別の更新
//	 	}else{
//	 		edy_cnt_ctrl.BIT.wait_kind = 0;											// 実行種別のクリア
//		}	 		
//	}else{																			// 実行フェーズ外なので、次回分歩進時に再チェック
//		if( !edy_cnt_ctrl.BIT.wait_kind ){											// すでに実行予約がある場合は、上書きしない
//			edy_cnt_ctrl.BIT.wait_kind = *execkind;									// 実行フェーズの保存
//		}
//	}
//	 return ret;
//
//}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//[]----------------------------------------------------------------------[]
///	@brief			日毎集計処理
///	@param[in]		LogKind		LOG_SCASYUUKEI(22):Suica利用集計
///								LOG_EDYSYUUKEI(21):Edy利用集計
///	@return			void
///	@author			Ise
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void ck_syuukei_ptr_zai( ushort LogKind )
{
	char	chk = 0;				/* 初回立ち上げ処理判断用フラグ */
	uchar	hour;					/* 共通パラメータの日毎集計設定時刻取得用(時) */
	uchar	minu;					/* 共通パラメータの日毎集計設定時刻取得用(分) */
	uchar	ptr_cnt[2];				/* 停復電時のデータ保障変数(0:ライトポインタ/1:集計件数) */
	ulong	old_date;				/* 前回通知日時情報 */
	ulong	now_date;				/* 今回通知日時情報 */
	ulong	sy1_date;				/* 前回通知時の集計日時情報その１ */
	ulong	sy2_date;				/* 前回通知時の集計日時情報その２ */
	ulong	sy3_date;				/* 今回通知時の集計日時情報 */
	syuukei_info	*p_syu_info;	/* 処理する集計情報構造体のアドレス */
// MH321800(S) hosoda ICクレジット対応
	syuukei_info2	*p_syu_info2 = 0;	/* 処理する集計情報構造体のアドレス */
// MH321800(E) hosoda ICクレジット対応
	date_time_rec	wrk_date;		/* 次の集計時間特定用構造体 */

	// 集計を取っている年月日と現年月日が違う場合は時刻が日付切替になっていなくても
	// 特注集計の格納ポインタを変更する。

	/* 有効な集計対象機能を判断して以下の処理に反映する */
	if(LogKind == LOG_SCASYUUKEI)							/* Suica */
	{
		hour = (uchar)prm_get(COM_PRM, S_SCA, 7, 2, 3);		/* 共通パラメータから集計時間設定情報『時』取得 */
		minu = (uchar)prm_get(COM_PRM, S_SCA, 7, 2, 1);		/* 共通パラメータから集計時間設定情報『分』取得 */
		p_syu_info = &Syuukei_sp.sca_inf;					/* 集計する構造体アドレスを取得 */
	}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	else if(LogKind == LOG_EDYSYUUKEI)						/* Edy */
//	{
//		hour = (uchar)prm_get(COM_PRM, S_SCA, 59, 2, 3);	/* 共通パラメータから集計時間設定情報『時』取得 */
//		minu = (uchar)prm_get(COM_PRM, S_SCA, 59, 2, 1);	/* 共通パラメータから集計時間設定情報『分』取得 */
//		p_syu_info = &Syuukei_sp.edy_inf;					/* 集計する構造体アドレスを取得 */
//	}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) hosoda ICクレジット対応
	else if(LogKind == LOG_ECSYUUKEI)						/* 決済リーダ */
	{
		hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);		/* 共通パラメータから集計時間設定情報『時』取得 */
		minu = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);		/* 共通パラメータから集計時間設定情報『分』取得 */
		p_syu_info = 0;										/* 集計する構造体アドレスなし */
		p_syu_info2 = &Syuukei_sp.ec_inf;					/* 集計する構造体アドレスを取得 */
	}
// MH321800(E) hosoda ICクレジット対応
	else													/* 上記以外(どの集計対象機能も未動作) */
	{
		p_syu_info = 0;										/* 集計する構造体アドレスなし */
	}

	if(p_syu_info)														/* 上記判断で正常にアドレスを取得 */
	{
		// 今回の通知時刻を反映
		memcpy(&p_syu_info->NowTime, &CLK_REC, sizeof(date_time_rec));
		now_date = Nrm_YMDHM(&p_syu_info->NowTime);						/* 今回通知日時情報をノーマライズで取得 */
		old_date = Nrm_YMDHM(&p_syu_info->OldTime);						/* 前回通知日時情報をノーマライズで取得 */
		/* 集計の日付に古い(前回通知日時情報)方を取得 */
		memcpy(&wrk_date, &p_syu_info->OldTime, sizeof(date_time_rec));
		wrk_date.Hour = hour;											/* 集計時を適用 */
		wrk_date.Min = minu;											/* 集計分を適用 */
		sy1_date = Nrm_YMDHM(&wrk_date);								/* 前回通知日時情報をノーマライズで取得 */
		sy2_date = sy1_date + 0x10000;									/* 取得した日時情報を１日進める */
		sy3_date = sy1_date - 0x10000;									/* 取得した日時情報を１日戻す */

		//初回？（電源立ち上げ時 or データクリア時）
		if(p_syu_info->cnt == 0)
		{
			ptr_cnt[0] = 0;												// 集計件数を初期化
			ptr_cnt[1] = 1;												// 格納ポインタを初期化
			chk = 1;													// 起動orクリアフラグ有効
			/* クリア要求のモジュールが無効になっている */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( ((LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//				((LogKind == LOG_EDYSYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			if( (LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			{
				// 更新した情報を管理構造体へ反映
				nmisave(&p_syu_info->ptr, ptr_cnt, 2);
				// 現集計（当日）エリアのクリア処理
				memset( &p_syu_info->now, 0, sizeof(syu_dat) );
				memset( &p_syu_info->bun[p_syu_info->ptr], 0, sizeof(syu_dat) );
				// 今回の通知時刻を保持
				memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
				// 処理終了
				return;
			}
		}
		// 初回以外？（日時変化通知）
		else
		{
			ptr_cnt[0] = p_syu_info->ptr;								// 現在の集計件数を取得
			ptr_cnt[1] = p_syu_info->cnt;								// 現在集計の格納ポインタを取得
			chk = 0;													// 起動orクリアフラグ無効
			/* 指定のモジュールが有効になっている */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( ((LogKind == LOG_SCASYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//				((LogKind == LOG_EDYSYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			if( (LogKind == LOG_SCASYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			{
				/* 現在の日時情報がクリアされている(モジュール有効最初の時刻通知) */
				if((!p_syu_info->now.SyuTime.Mon) && (!p_syu_info->now.SyuTime.Day))
				{
					/* 前回通知された時間を集計開始時間として適用 */
					memcpy( &p_syu_info->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
					// 今回の通知時刻を保持
					memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
					// 処理終了
					return;
				}
			}
			/* 指定のモジュールが無効になっている */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			else if(((LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//					((LogKind == LOG_EDYSYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			else if( (LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			{
				// 今回の通知時刻を保持
				memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
				// 処理終了
				return;
			}
		}
		/* 前回⇔今回通知間に集計時間をまたいでいる　または　電源起動orクリアフラグ有効 */
		if( ((now_date >= sy1_date) && (sy1_date > old_date)) ||		// 今回通知日時　≧　前回通知時の集計日時　＞　前回通知日時
			((now_date >= sy2_date) && (sy2_date > old_date)) ||		// 今回通知日時　≧　今回通知時の集計日時　＞　前回通知日時
			((now_date <= sy1_date) && (sy1_date < old_date)) ||		// 今回通知日時　≦　前回通知時の集計日時　＜　前回通知日時
			((now_date <= sy3_date) && (sy3_date < old_date)) ||		// 今回通知日時　≦　今回通知時の集計日時＋１日　＜　前回通知日時
			(chk))														// 初回起動orクリアフラグ有効
		{
			//初回（電源立ち上げ時 or データクリア時）以外で日付更新あり？
			if( chk == 0 ) {
				// 現時刻までの集計情報を保存
				memcpy( &p_syu_info->bun[ptr_cnt[0]], &p_syu_info->now, sizeof(syu_dat) );
				//ポインタの更新
				if( ptr_cnt[0] < (SYUUKEI_DAY-1) ) {					// SYUUKEI_DAY = 101
					ptr_cnt[0]++;
				} else {
					ptr_cnt[0] = 0;										// テーブル終端で0(最初)に戻る
				}
				//カウントの更新
				if( ptr_cnt[1] < SYUUKEI_DAY ) {						// SYUUKEI_DAY = 101
					ptr_cnt[1]++;
				}
			}
			// 更新した情報を管理構造体へ反映
			nmisave(&p_syu_info->ptr, ptr_cnt, 2);
			//現集計（当日）エリアのクリア処理
			memset( &p_syu_info->now, 0, sizeof(syu_dat) );
			memset( &p_syu_info->bun[p_syu_info->ptr], 0, sizeof(syu_dat) );
			// 次に集計する年月日をセット
			memcpy( &p_syu_info->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
		}
		// 今回の通知時刻を保持
		memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
	}
// MH321800(S) hosoda ICクレジット対応
	if(p_syu_info2)														/* 上記判断で正常にアドレスを取得 */
	{
		// 今回の通知時刻を反映
		memcpy(&p_syu_info2->NowTime, &CLK_REC, sizeof(date_time_rec));
		now_date = Nrm_YMDHM(&p_syu_info2->NowTime);					/* 今回通知日時情報をノーマライズで取得 */
		old_date = Nrm_YMDHM(&p_syu_info2->OldTime);					/* 前回通知日時情報をノーマライズで取得 */
		/* 集計の日付に古い(前回通知日時情報)方を取得 */
		memcpy(&wrk_date, &p_syu_info2->OldTime, sizeof(date_time_rec));
		wrk_date.Hour = hour;											/* 集計時を適用 */
		wrk_date.Min = minu;											/* 集計分を適用 */
		sy1_date = Nrm_YMDHM(&wrk_date);								/* 前回通知日時情報をノーマライズで取得 */
		sy2_date = sy1_date + 0x10000;									/* 取得した日時情報を１日進める */
		sy3_date = sy1_date - 0x10000;									/* 取得した日時情報を１日戻す */

		//初回？（電源立ち上げ時 or データクリア時）
		if(p_syu_info2->cnt == 0)
		{
			ptr_cnt[0] = 0;												// 格納ポインタを初期化
			ptr_cnt[1] = 1;												// 集計件数を初期化
			chk = 1;													// 起動orクリアフラグ有効
			/* クリア要求のモジュールが無効になっている */
			if((LogKind == LOG_ECSYUUKEI) && (!isEC_USE()))
			{
				// 更新した情報を管理構造体へ反映
				nmisave(&p_syu_info2->ptr, ptr_cnt, 2);
				// 現集計（当日）エリアのクリア処理
				memset( &p_syu_info2->next, 0, sizeof(syu_dat2) );
				memset( &p_syu_info2->now, 0, sizeof(syu_dat2) );
				memset( &p_syu_info2->bun[p_syu_info2->ptr], 0, sizeof(syu_dat2) );
				// 今回の通知時刻を保持
				memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
				// 処理終了
				return;
			}
		}
		// 初回以外？（日時変化通知）
		else
		{
			ptr_cnt[0] = p_syu_info2->ptr;								// 現在集計の格納ポインタを取得
			ptr_cnt[1] = p_syu_info2->cnt;								// 現在の集計件数を取得
			chk = 0;													// 起動orクリアフラグ無効
			/* 指定のモジュールが有効になっている */
			if((LogKind == LOG_ECSYUUKEI) && (isEC_USE()))
			{
				/* 現在の日時情報がクリアされている(モジュール有効最初の時刻通知) */
				if((!p_syu_info2->now.SyuTime.Mon) && (!p_syu_info2->now.SyuTime.Day))
				{
					/* 前回通知された時間を集計開始時間として適用 */
					memcpy( &p_syu_info2->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
					// 今回の通知時刻を保持
					memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
					// 処理終了
					return;
				}
			}
			/* 指定のモジュールが無効になっている */
			else if((LogKind == LOG_ECSYUUKEI) && (!isEC_USE()))
			{
				// 今回の通知時刻を保持
				memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
				// 処理終了
				return;
			}
		}
		/* 前回⇔今回通知間に集計時間をまたいでいる　または　電源起動orクリアフラグ有効 */
		if( ((now_date >= sy1_date) && (sy1_date > old_date)) ||		// 今回通知日時　≧　前回通知時の集計日時　＞　前回通知日時
			((now_date >= sy2_date) && (sy2_date > old_date)) ||		// 今回通知日時　≧　今回通知時の集計日時　＞　前回通知日時
			((now_date <= sy1_date) && (sy1_date < old_date)) ||		// 今回通知日時　≦　前回通知時の集計日時　＜　前回通知日時
			((now_date <= sy3_date) && (sy3_date < old_date)) ||		// 今回通知日時　≦　今回通知時の集計日時＋１日　＜　前回通知日時
			(chk))														// 初回起動orクリアフラグ有効
		{
			//初回（電源立ち上げ時 or データクリア時）以外で日付更新あり？
			if( chk == 0 ) {
				// 現時刻までの集計情報を保存
				memcpy( &p_syu_info2->bun[ptr_cnt[0]], &p_syu_info2->now, sizeof(syu_dat2) );
				//ポインタの更新
				if( ptr_cnt[0] < (SYUUKEI_DAY_EC-1) ) {
					ptr_cnt[0]++;
				} else {
					ptr_cnt[0] = 0;										// テーブル終端で0(最初)に戻る
				}
				//カウントの更新
				if( ptr_cnt[1] < SYUUKEI_DAY_EC ) {
					ptr_cnt[1]++;
				}
			}
			// 更新した情報を管理構造体へ反映
			nmisave(&p_syu_info2->ptr, ptr_cnt, 2);
			//現集計（当日）エリアのクリア処理
			memcpy( &p_syu_info2->now, &p_syu_info2->next, sizeof(syu_dat2) );		// 翌日分として集計されたデータを当日に移動
			memset( &p_syu_info2->next, 0, sizeof(syu_dat2) );						// 翌日分として集計されたエリアをクリア
			memset( &p_syu_info2->bun[p_syu_info2->ptr], 0, sizeof(syu_dat2) );
			// 次に集計する年月日をセット
			memcpy( &p_syu_info2->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
		}
		// 今回の通知時刻を保持
		memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
	}
// MH321800(E) hosoda ICクレジット対応

	return;
}

/*[]----------------------------------------------------------------------[]*
 *| MAF通信用共通パラメータ更新											  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : MAFParamUpdate										  |
 *| PARAMETER    : ｾｸﾞﾒﾝﾄ：seg	ｱﾄﾞﾚｽ：add　ﾃﾞｰﾀ：data          		  |
  *| RETURN VALUE : 正常：1	異常：-1									  |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	MAFParamUpdate(ushort seg, ushort add, long data)
{
	char	ret=(char)-1;

	/* 受信ﾃﾞｰﾀ確認 */
	if ((seg > 0 && seg < C_PRM_SESCNT_MAX) && (add > 0 && add <= CPrmCnt[seg])){	/* ｾｸﾞﾒﾝﾄ(ｾｸｼｮﾝ) && 開始ｱﾄﾞﾚｽ */
		/* ﾃﾞｰﾀ更新 */
		CPrmSS[seg][add] = (long)data;

		DataSumUpdate(OPE_DTNUM_COMPARA);			/* update parameter sum on ram */
												// 個別ﾊﾟﾗﾒｰﾀはｺﾋﾟｰしない（AppServ_PParam_Copy = OFF）
		FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		/* FlashROM update */
		// パスワードデータではmc10は必要ないが、本対策前の運用を方法を考慮し、更新処理は残す
		f_NTNET_RCV_MC10_EXEC = 1;				/* mc10()更新が必要な設定が通信で更新された情報set */
		SetSetDiff(SETDIFFLOG_SYU_REMSET);		// パスワード変更情報登録処理
		ret = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 起動時のLD1点滅処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : start_up_LD1                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2010-03-10                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void	start_up_LD1( void )
{
	ExIOSignalwt(EXPORT_M_LD1, 1 );
	if(f_wave_datchk != WAVE_AREA_NODATA) {// 音声データが存在する場合
		ExIOSignalwt(EXPORT_M_LD2, 1 );
	}
	wait2us( 200000L ); 						// wait 400ms
	ExIOSignalwt(EXPORT_M_LD1, 0 );
	ExIOSignalwt(EXPORT_M_LD2, 0 );
	wait2us( 150000L ); 						// wait 300ms
	if(f_wave_datchk != WAVE_AREA_NODATA) {// 音声データが存在する場合
		ExIOSignalwt(EXPORT_M_LD2, 1 );
	}
	ExIOSignalwt(EXPORT_M_LD1, 1 );
	wait2us( 200000L ); 						// wait 400ms
	ExIOSignalwt(EXPORT_M_LD1, 0 );
	ExIOSignalwt(EXPORT_M_LD2, 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| プリンタのステータス取得処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Printer_Status_Get	                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static uchar Printer_Status_Get( uchar Type, uchar *NEW, uchar *OLD )
{
	PRN_DATA_BUFF	*p;							// ﾌﾟﾘﾝﾀﾃﾞｰﾀﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	
	if( Type == R_PRI ){
		p = &rct_prn_buff;
	}else{
		p = &jnl_prn_buff;
	}

	if( p->PrnStateRcvCnt > 1 ){
		*NEW = p->PrnState[1];
		*OLD = p->PrnState[2];
		p->PrnStateRcvCnt = 1;				// 残りのチェック回数を１回とする
	}
	else if(p->PrnStateRcvCnt == 0) {		// プリンタイベント受信なし
		return 0xff;
	}else{
		*NEW = p->PrnState[0];
		*OLD = p->PrnState[1];
		p->PrnStateRcvCnt--;				// 受信済み分を減算
	}
	return p->PrnStateRcvCnt;

}

/*[]----------------------------------------------------------------------[]*/
/*| プリンタエラー登録処理		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : I2C_Err_Regist                                          |*/
/*| PARAMETER    : msg ：イベントメッセージ ErrData：エラーデータ          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2012-03-6	                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void I2C_Err_Regist( unsigned short msg, unsigned char ErrData )
{
	
	short	err_code;
// MH810104 GG119201(S) 電子ジャーナル対応
	char	err_mdl;
// MH810104 GG119201(E) 電子ジャーナル対応
	
	if( msg == I2C_RP_ERR_EVT ){
// MH810104 GG119201(S) 電子ジャーナル対応
		err_mdl = ERRMDL_PRINTER;
// MH810104 GG119201(E) 電子ジャーナル対応
		err_code = ERR_PRNT_PARITY_ERR_R;
	}else{
// MH810104 GG119201(S) 電子ジャーナル対応
		if (isEJA_USE()) {
			err_mdl = ERRMDL_EJA;
			err_code = ERR_EJA_PARITY_ERR;
		}
		else {
			err_mdl = ERRMDL_PRINTER;
// MH810104 GG119201(E) 電子ジャーナル対応
		err_code = ERR_PRNT_PARITY_ERR_J;
// MH810104 GG119201(S) 電子ジャーナル対応
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	
// MH810104 GG119201(S) 電子ジャーナル対応
//	if( ErrData & BIT_PARITY_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, err_code, 2, 0, 0 );
//	}
//	if( ErrData & BIT_FRAMING_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, (err_code+1), 2, 0, 0 );
//	}
//	if( ErrData & BIT_OVERRUN_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, (err_code+2), 2, 0, 0 );
//	}
	if( ErrData & BIT_PARITY_ERR_LSR ){
		err_chk( err_mdl, err_code, 2, 0, 0 );
	}
	if( ErrData & BIT_FRAMING_ERR_LSR ){
		err_chk( err_mdl, (err_code+1), 2, 0, 0 );
	}
	if( ErrData & BIT_OVERRUN_ERR_LSR ){
		err_chk( err_mdl, (err_code+2), 2, 0, 0 );
	}
// MH810104 GG119201(E) 電子ジャーナル対応
}
/*[]----------------------------------------------------------------------[]*/
/*|	キー長押しのリピートタイマ関数										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi                                             |*/
/*| Date         : 2012/05/14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void Ope_KeyRepeat()
{
	uchar on_off = 1;
	
	if(OpeKeyRepeat.key != 0) {
		queset(OPETCBNO, OpeKeyRepeat.key, 1, &on_off);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| キーリピート処理有効無効制御	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mnt_mode	メンテナンスモード                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi                                             |*/
/*| Date         : 2012/05/14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void Ope_KeyRepeatEnable(char mnt_mode)
{
	if(mnt_mode != 0) {
// MH810100(S) S.Takahashi 2020/02/13 #3841 「▲」「▼」を同時にタッチ、同時に離すとカーソル移動したままとなる
//		OpeKeyRepeat.enable = 1;				// キーリピート有効
// MH810100(E) S.Takahashi 2020/02/13 #3841 「▲」「▼」を同時にタッチ、同時に離すとカーソル移動したままとなる
	}
	else {
		LagCan20ms(LAG20_KEY_REPEAT);			// リピートタイマ削除
		OpeKeyRepeat.enable = 0;				// キーリピート無効
		OpeKeyRepeat.key = 0;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 音声インストールプログラム		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2012/06/05                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
//static	void	SoundInsProgram( void )
void	SoundInsProgram( void )
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
{
	ushort	i;
	uchar	BcdTime[11];
	ushort	chk_sum = 0;
	ushort	cmp_sum = 0;
	ulong	read_Addr = 0;
	ulong	sect_cnt;
	ulong	data_cnt = 0;
	ulong	buf_cnt;
	ulong	data_len= 0;
	
	f_SoundIns_OK = 0;
	// 念の為、メモリークリアは実施する
	WACDOG;														// Watch dog timer reset

	for( i = 0; i < eLOG_MAX; i++ ){
		memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
	}
	log_init();
	prm_clr( COM_PRM, 1, 0 );									// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
	prm_clr( PEC_PRM, 1, 0 );									// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
	RAMCLR = memclr( 1 );										// SRAM初期化（無条件ｸﾘｱ）
	Ether_DTinit();
	_ei();															// enable interrupts
	Flash2Init();
	Flash1Init();
	/* 要求受け付けを初期化 */
	FLT_req_tbl_clear();
	// INIT処理をやっておかないと、エラー登録等を行った場合に不定値によるメモリアクセスが発生するので、
	// 今は使用しないが一応やっておく
	NTNET_Init((uchar)RAMCLR);										// NT-NETｴﾘｱｲﾆｼｬﾙ
	FLAGPT.nmi_flg = 0;												// NMIﾌﾗｸﾞｸﾘｱ

	// タイマー関連の初期化実施
	Lag10msInit();
	Lag20msInit();
	Lag500msInit();
	TPU1_init();													// TPU1 Initial
	TPU2_init();													// TPU2 Initial
	TPU3_init();													// TPU3 Initial
	WACDOG;															// Watch dog timer reset
	
	// メモリージャンパーPINを外して起動するらしいので、時計は擬似的に2012/01/01 0:00をセットする
	BcdTime[0] = 0x20;
	BcdTime[1] = 0x12;
	BcdTime[2] = 0x01;
	BcdTime[3] = 0x01;
	BcdTime[4] = 0x00;
	BcdTime[5] = 0x00;
	BcdTime[6] = 0x00;
	BcdTime[7] = 0x00;
	BcdTime[8] = 0x00;
	BcdTime[9] = 0x00;
	BcdTime[10] = 0x00;
	
	ClkrecUpdate(BcdTime);
	BootInfo.sw.wave = OPERATE_AREA2;							// 音声データのインストールは、面１にする為、一時的に運用面を面２とする
	SysMnt_FTPServer();
	_flt_DisableTaskChg();
	FLT_read_wave_sum_version(0,swdata_write_buf);// 音声データ面1の情報をリード
	// レングスの取得
	data_len = (ulong)(swdata_write_buf[LENGTH_OFFSET]<<24);
	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+1]<<16);
	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+2]<<8);
	data_len += (ulong)swdata_write_buf[LENGTH_OFFSET+3];
	// チェックサムの取得
	cmp_sum = (ushort)(swdata_write_buf[SUM_OFFSET]<<8);
	cmp_sum += (ushort)swdata_write_buf[SUM_OFFSET+1];
	// FROMから面1の音声データを読み出しサムを算出する
	read_Addr = FLT_SOUND0_SECTOR;
	for(sect_cnt = 0; sect_cnt < FLT_SOUND0_SECTORMAX; sect_cnt++,read_Addr += FLT_SOUND0_SECT_SIZE) {
		FlashReadData_direct(read_Addr, &FLASH_WRITE_BUFFER[0], FLT_SOUND0_SECT_SIZE);// 1セクタ分をリード
		for (buf_cnt = 0; buf_cnt < FLT_SOUND0_SECT_SIZE; buf_cnt++) {
			chk_sum += FLASH_WRITE_BUFFER[buf_cnt];
			data_cnt++;
			if(data_len <= data_cnt){
				goto smchk_loop_end;
			}
		}
	}
smchk_loop_end:
	if(cmp_sum != chk_sum){// ダウンロードファイルのチェックサムともう一度比較し、違っていた場合は失敗
		f_SoundIns_OK = 0;
	}
	BootInfo.sw.wave = OPERATE_AREA1;							// 音声データのインストール後は、面１を運用面にする
	FLT_write_wave_swdata(OPERATE_AREA1);						// FROMにも運用面を書き込む
	// 通信が終われば、ﾘｾｯﾄされるまで何もしない
	while(1){
		taskchg( IDLETSKNO );
		ExIOSignalwt(EXPORT_M_LD2, 1 );// LD2点灯で終了(成功時)
		if(f_SoundIns_OK == 0){// 失敗
			wait2us( 150000L ); // wait 300ms
			ExIOSignalwt(EXPORT_M_LD2, 0 );// LD2点滅
			wait2us( 150000L ); // wait 300ms
		}
	}

}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//[]----------------------------------------------------------------------[]
///	@brief		LCDとの通信設定デフォルト
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static	void	lcdbm_LcdIpPort_DefaultSetting( void )
{
	int		i;				// index

	for( i=0; 0xff != cprm_rec[i].ses; i++) {
		// 標準通信モジュール設定
		if( cprm_rec[i].ses == 41 ){
			// 通信規格，ホストIPアドレス，サブネットマスク，デフォルトゲートウェイ
			if( cprm_rec[i].adr >= 1 && cprm_rec[i].adr <= 7 ){
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr] = cprm_rec[i].dat;
			}
		}
		// LCD通信設定
		else if( cprm_rec[i].ses == 48 ){
			// LCDパネルIPアドレス①～④，ポート番号
			if( cprm_rec[i].adr >= 1 && cprm_rec[i].adr <= 7 ){
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr] = cprm_rec[i].dat;
			}
		}
	}
	// update parameter sum on ram
	DataSumUpdate( OPE_DTNUM_COMPARA );
}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
///*[]----------------------------------------------------------------------[]*/
///*| 音声テスト再生プログラム		                                       |*/
///*[]----------------------------------------------------------------------[]*/
///*| PARAMETER    : void							                           |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : T.Namioka	                                           |*/
///*| Date         : 2012/06/05                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
//static	void	SoundPlayProgram( void )
//{
//	ushort	msg = 0;
//	ushort	i;
//	for( i = 0; i < eLOG_MAX; i++ ){
//		memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
//	}
//	log_init();
//	prm_clr( COM_PRM, 1, 0 );									// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
//	prm_clr( PEC_PRM, 1, 0 );									// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
//	RAMCLR = memclr( 1 );										// SRAM初期化（無条件ｸﾘｱ）
//	FLT_req_tbl_clear();										// 要求受け付けを初期化
//// 基板単体で通常デフォルトで鳴らすには大きすぎるので 音量を「3」とする
//	CPrmSS[S_SYS][51] = 30000L;// ch1音声
//	CPrmSS[S_SYS][60] = 3L;// ブザー音量
//	AVM_Sodiac_Init();
//	while( 1 ){
//		if( AVM_Sodiac_Ctrl[0].play_cmp ){
//			if( msg > ANN_MSG_MAX){
//				msg = 0;
//			}
//			avm_test_no[0] = msg++;
//			avm_test_ch = 0;
//			avm_test_cnt = 1;
//			ope_anm( AVM_AN_TEST );
//		}
//		taskchg( IDLETSKNO );
//	}
//}
// MH810100(E) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
/*[]----------------------------------------------------------------------[]*/
/*| ドアノブ戻し忘れタイマ開始                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_StartDoorKnobTimer(void)
{
	short	doorKnobAlarmTimer;

	doorKnobAlarmTimer = OPE_DOOR_KNOB_ALARM_START_TIMER;
	if(doorKnobAlarmTimer) {
		Lagtim(OPETCBNO, 5, doorKnobAlarmTimer);	// ドアノブ戻し忘れ防止タイマー更新
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ドアノブ戻し忘れ警告音出力時間設定                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_StopDoorKnobTimer(void)
{
	short	doorKnobAlarmStopTimer;

	doorKnobAlarmStopTimer = OPE_DOOR_KNOB_ALARM_STOP_TIMER;
	if(doorKnobAlarmStopTimer) {
		Lagtim(OPETCBNO, 29, doorKnobAlarmStopTimer);	// ドアノブ戻し忘れ警告音出力時間タイマー
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ドアノブ戻し忘れチャイム有効                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_EnableDoorKnobChime(void)
{
	DisableDoorKnobChime = 0;				// ドアノブ戻し忘れチャイム抑止フラグOFF(チャイム有効)
	Ope_StartDoorKnobTimer();				// ドアノブ戻し忘れタイマ開始
}
/*[]----------------------------------------------------------------------[]*/
/*| ドアノブ戻し忘れチャイム無効                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_DisableDoorKnobChime(void)
{
	DisableDoorKnobChime = 1;				// ドアノブ戻し忘れチャイム抑止フラグON(チャイム無効)
}
/*[]----------------------------------------------------------------------[]*/
/*| ドアノブ戻し忘れチャイム状態取得                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : 1 : 有効                                                |*/
/*|              : 0 : 無効                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	Ope_IsEnableDoorKnobChime(void)
{
	if(DisableDoorKnobChime == 0) {
		return 1;
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| 時刻によるFAN制御判定処理                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_fan_timectrl                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 1 : 強制停止                                            |*/
/*|              : 0 : 通常制御                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : A.iiizumi                                               |*/
/*| Date         : 2012-07-04                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static uchar chk_fan_timectrl( void )
{
	ulong	st_time;
	ulong	ed_time;
	ulong	now_time;
	uchar	ret = 0;

	//設定時間を完全ノーマライズ
	// 02-0063③④⑤⑥ FAN強制停止開始時刻
	st_time = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
							   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][63]/100), (short)(CPrmSS[S_PAY][63]%100) ) );
	// 02-0064③④⑤⑥ FAN強制停止終了時刻
	ed_time = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
							   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );

	//現在時間を完全ノーマライズ
	now_time = time_nrmlz ( CLK_REC.ndat, CLK_REC.nmin );

	//設定時間大小変換
	if( st_time > ed_time ){ //EX>Start 20:00 , End 6:00
		if( ed_time > now_time ){//now 3:00
			//開始を前日にする	02-0063③④⑤⑥ FAN強制停止開始時刻
			st_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
									 (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][63]/100), (short)(CPrmSS[S_PAY][63]%100) ) );
		}else if(now_time > st_time){//now 20:00
			//終了を次の日にする 02-0064③④⑤⑥ FAN強制停止終了時刻
			ed_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) + 1),
								     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );
		}
	}else if( st_time < ed_time ){ //EX> now 1:00 , Start 2:00 , End 10:00 
		if( st_time > now_time ){
			//終了を前日にする 02-0064③④⑤⑥ FAN強制停止終了時刻
			ed_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
								     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );
		}
	}else if( st_time == ed_time ){
		return(ret); //開始と終了が一致した場合は制御しない
	}

	if(st_time <= now_time){// FAN制御 強制停止開始
		ret = 1;//強制停止
	}else{
		ret = 0;//通常制御(強制停止解除)
	}
	if(ed_time <= now_time){
		ret = 0;//通常制御(強制停止解除)
	}
	return ret;
}
// MH810100(S) 2020/06/16 #4231 【連動評価指摘事項】ラグタイムオーバ後に出庫しているが、未精算出庫の再精算が無料になってしまう(No.02-0027)
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))_for_LCD_IN_CAR_INFO_料金計算
//// if (精算時間 + ﾗｸﾞﾀｲﾑ時間 >= 現在時間) ---> op_mod230(精算済み案内)
//// return		1:	op_mod230(精算済み案内)へ移行
//uchar Is_in_lagtim( void )
//{
//	ushort	prm_minutes = 0;
//	uchar	syu;
//	ulong	st_time;
//	ulong	now_time;
//	uchar	ret = 0;	// 0 = ﾗｸﾞﾀｲﾑｵｰﾊﾞ(op_mod230(精算済み案内)への移行なし)
//	ushort	Year;
//	uchar	Month, Day, Hours, Minute, Second;
//	ulong	wkul;
//	struct clk_rec PaymentTime;
//	struct clk_rec NowTime;
//
//	Year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;		// 在車情報_精算_年	2000～2099(処理日時)
//	Month	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;		// 在車情報_精算_月	1～12(処理日時)
//	Day		= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;		// 在車情報_精算_日	1～31(処理日時)
//	Hours	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;		// 在車情報_精算_時	0～23(処理日時)
//	Minute	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;		// 在車情報_精算_分	0～59(処理日時)
//	Second	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;		// 在車情報_精算_秒	0～59(処理日時)
//
//	//精算時間 + ﾗｸﾞﾀｲﾑ時間を完全ﾉｰﾏﾗｲｽﾞ
//	PaymentTime.year	= Year;		// 在車情報_精算_年	2000～2099(処理日時)
//	PaymentTime.mont	= Month;	// 在車情報_精算_月	1～12(処理日時)
//	PaymentTime.date	= Day;		// 在車情報_精算_日	1～31(処理日時)
//	PaymentTime.hour	= Hours;	// 在車情報_精算_時	0～23(処理日時)
//	PaymentTime.minu	= Minute;	// 在車情報_精算_分	0～59(処理日時)
//	PaymentTime.seco	= Second;	// 在車情報_精算_秒	0～59(処理日時)
//	PaymentTime.week	= 0;		// Day   (0:SUN-6:SAT)
//	PaymentTime.ndat	= 0;		// Normalize Date
//	PaymentTime.nmin	= 0;		// Normalize Minute
//	// 年月日時分秒データから、秒単位のノーマライズ値get
//	st_time = c_Normalize_sec(&PaymentTime);
//
//	NowTime.year	= CLK_REC.year;
//	NowTime.mont	= CLK_REC.mont;
//	NowTime.date	= CLK_REC.date;
//	NowTime.hour	= CLK_REC.hour;
//	NowTime.minu	= CLK_REC.minu;
//	NowTime.seco	= CLK_REC.seco;
//	NowTime.week	= 0;		// Day   (0:SUN-6:SAT)
//	NowTime.ndat	= 0;		// Normalize Date
//	NowTime.nmin	= 0;		// Normalize Minute
//
//	// 年月日時分秒データから、秒単位のノーマライズ値get
//	now_time = c_Normalize_sec(&NowTime);
//
//	// 29-0001	ｻｰﾋﾞｽﾀｲﾑ切換	⑥:ｻｰﾋﾞｽﾀｲﾑ切換    0=なし(全車種共通)/1=あり(車種毎に設定)
//	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// 全車種共通?
//		// 29-0004	全車種共通	ﾗｸﾞﾀｲﾑ	④⑤⑥:時間	001～120(分)/000=設定不可 精算から出庫までの料金未発生時間(000010)
//		prm_minutes = (short)CPrmSS[S_STM][4];
//
//	// 種別毎ｻｰﾋﾞｽﾀｲﾑ
//	}else{
//		// 車種を取得
//		syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
//		if(( 1 <= syu )&&( syu <= 12 )){
//			// 車種毎ﾗｸﾞﾀｲﾑ		29-0007, 29-0010,  29-0013... A車種 - L車種
//			prm_minutes = (short)CPrmSS[S_STM][7 + (3 * (syu - 1))];
//		}
//	}
//
//	wkul = (prm_minutes * 60);		// 単位を秒に
//	st_time += wkul;
//
//	// if (精算時間 + ﾗｸﾞﾀｲﾑ時間 >= 現在時間) ---> op_mod230(精算済み案内)
//	if (st_time >= now_time) {
//		ret = 1;				// 1 = op_mod230(精算済み案内)へ移行する
//	}
//	return ret;
//}
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))_for_LCD_IN_CAR_INFO_料金計算

// 精算時刻  ⇒⇒   出庫時刻				の場合①：精算時刻＋ラグタイム >= 出庫時刻	---> op_mod230(精算済み案内)
// 出庫時刻  ⇒⇒   精算時刻				の場合②：未精算出庫の精算済みのはず		---> op_mod230(精算済み案内)
// 精算時刻  ⇒⇒   現在時刻				の場合③：精算時刻＋ラグタイム >= 現在時間	---> op_mod230(精算済み案内)
uchar Is_in_lagtim( void )
{
	ushort	prm_minutes = 0;
	uchar	syu;
	ulong	Payst_time;
	ulong	Exitst_time;
	ulong	now_time;
	uchar	ret = 0;	// 0 = ﾗｸﾞﾀｲﾑｵｰﾊﾞ(op_mod230(精算済み案内)への移行なし)
	ulong	wkul;
	struct clk_rec PaymentTime;
	struct clk_rec ExitTime;
	struct clk_rec NowTime;

	// 出庫時間
	ExitTime.year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.shYear;		// 在車情報_出庫_年	2000～2099(処理日時)
	ExitTime.mont	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byMonth;		// 在車情報_出庫_月	1～12(処理日時)
	ExitTime.date	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byDay;		// 在車情報_出庫_日	1～31(処理日時)
	ExitTime.hour	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byHours;		// 在車情報_出庫_時	0～23(処理日時)
	ExitTime.minu	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byMinute;	// 在車情報_出庫_分	0～59(処理日時)
	ExitTime.seco	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.bySecond;	// 在車情報_出庫_秒	0～59(処理日時)
	ExitTime.week	= 0;		// Day   (0:SUN-6:SAT)
	ExitTime.ndat	= 0;		// Normalize Date
	ExitTime.nmin	= 0;		// Normalize Minute
	// 年月日時分秒データから、秒単位のノーマライズ値get
	Exitst_time = c_Normalize_sec(&ExitTime);
	
	// 精算時間
	PaymentTime.year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;		// 在車情報_精算_年	2000～2099(処理日時)
	PaymentTime.mont	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;	// 在車情報_精算_月	1～12(処理日時)
	PaymentTime.date		= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;	// 在車情報_精算_日	1～31(処理日時)
	PaymentTime.hour	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;	// 在車情報_精算_時	0～23(処理日時)
	PaymentTime.minu	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;	// 在車情報_精算_分	0～59(処理日時)
	PaymentTime.seco	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;	// 在車情報_精算_秒	0～59(処理日時)
	PaymentTime.week	= 0;		// Day   (0:SUN-6:SAT)
	PaymentTime.ndat	= 0;		// Normalize Date
	PaymentTime.nmin	= 0;		// Normalize Minute
	// 年月日時分秒データから、秒単位のノーマライズ値get
	Payst_time = c_Normalize_sec(&PaymentTime);

	NowTime.year	= CLK_REC.year;
	NowTime.mont	= CLK_REC.mont;
	NowTime.date	= CLK_REC.date;
	NowTime.hour	= CLK_REC.hour;
	NowTime.minu	= CLK_REC.minu;
	NowTime.seco	= CLK_REC.seco;
	NowTime.week	= 0;		// Day   (0:SUN-6:SAT)
	NowTime.ndat	= 0;		// Normalize Date
	NowTime.nmin	= 0;		// Normalize Minute
	// 年月日時分秒データから、秒単位のノーマライズ値get
	now_time = c_Normalize_sec(&NowTime);

	// 29-0001	ｻｰﾋﾞｽﾀｲﾑ切換	⑥:ｻｰﾋﾞｽﾀｲﾑ切換    0=なし(全車種共通)/1=あり(車種毎に設定)
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// 全車種共通?
		// 29-0004	全車種共通	ﾗｸﾞﾀｲﾑ	④⑤⑥:時間	001～120(分)/000=設定不可 精算から出庫までの料金未発生時間(000010)
		prm_minutes = (short)CPrmSS[S_STM][4];

	// 種別毎ｻｰﾋﾞｽﾀｲﾑ
	}else{
		// 車種を取得
		syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
		if(( 1 <= syu )&&( syu <= 12 )){
			// 車種毎ﾗｸﾞﾀｲﾑ		29-0007, 29-0010,  29-0013... A車種 - L車種
			prm_minutes = (short)CPrmSS[S_STM][7 + (3 * (syu - 1))];
		}
	}

	wkul = (prm_minutes * 60);		// 単位を秒に

// 精算時刻  ⇒⇒   出庫時刻				の場合①：精算時刻＋ラグタイム >= 出庫時刻	---> op_mod230(精算済み案内)
// 出庫時刻  ⇒⇒   精算時刻				の場合②：未精算出庫の精算済みのはず		---> op_mod230(精算済み案内)
// 精算時刻  ⇒⇒   現在時刻				の場合③：精算時刻＋ラグタイム >= 現在時間	---> op_mod230(精算済み案内)
	
	// 出庫時刻がある場合
	if(ExitTime.year && ExitTime.mont && ExitTime.date){
		if( Exitst_time >= Payst_time){
			// ①：精算時刻＋ラグタイム >= 出庫時刻
			if( (Payst_time + wkul ) >= Exitst_time){
				ret = 1;				// 1 = op_mod230(精算済み案内)へ移行する
			}
		}else{
			// ②：未精算出庫の精算済みのはず
			ret = 1;				// 1 = op_mod230(精算済み案内)へ移行する
		}
	}else{
		// ③：精算時刻＋ラグタイム >= 現在時間
		if( (Payst_time + wkul) >= now_time){
			ret = 1;				// 1 = op_mod230(精算済み案内)へ移行する
		}
	}
	return ret;
}

// MH810100(E) 2020/06/16 #4231 【連動評価指摘事項】ラグタイムオーバ後に出庫しているが、未精算出庫の再精算が無料になってしまう(No.02-0027)
/*[]----------------------------------------------------------------------[]*/
/*| センターに送るエラー・アラームの送信タイマーを起動                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_CenterDoorTimer                                     |*/
/*| PARAMETER    : start_stop: 0=stop, 1=start                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : t.hashimoto                                             |*/
/*| Date         : 2013-02-22                                              |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void Ope_CenterDoorTimer( char start_stop )
{
	ushort	usT;

	if( !OPE_Is_DOOR_ERRARM ){				// ドア開時のエラー・アラーム制御あり？
		// システムメンテで機能なしに変更された場合の対処
		OPECTL.f_DoorSts = 0;
		return;
	}
	if( prm_get(COM_PRM, S_PAY, 24, 1, 1) == 2 ){				// 遠隔NT-NET
		if( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1 ){			// 接続先は駐車場センター
			if( start_stop != 0 ){
				usT = (ushort)(prm_get(COM_PRM, S_CEN, 96, 2, 1));
				if( usT == 0 ){
					usT = 10;
				}
				Lagtim( OPETCBNO, 30,  usT * 50 );
			}else{
				Lagcan( OPETCBNO, 30 );							// ﾀｲﾏｰﾘｾｯﾄ
			}
		}
	}
}

// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
//static void log_clear_all(void)
static void log_clear_all(uchar factor)
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
{
	uchar i;
// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
	uchar is_need_delete = FLT_check_syukei_log_delete();
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)

	// FROMに格納してあるログバージョンとプログラムコードのログバージョンが異なる場合
	// ソフトバージョンアップでログの構造が変更となったためログを初期化する
	// また、パスワード領域が破壊されていた場合、ログを初期化し不正アクセスしないように保護する
	// (バックアップバッテリーのジャンパがOPENの状態で起動した時、音声データの領域を破壊するケースがあるため)
	for(i = 0;i < eLOG_MAX; i++){
// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
		if( 0 == factor &&								// クリア要因がログバージョン不一致、かつ
			0 == is_need_delete &&						// 集計ログの管理情報が変更されていない
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//			(eLOG_TTOTAL == i || eLOG_LCKTTL == i) ) {	// 集計ログと車室毎集計ログは削除しない
			(eLOG_TTOTAL == i) ) {						// 集計ログは削除しない
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810104(S) 2021/10/27 車番チケットレス ログバージョンが不一致の場合、集計ログもクリアされてしまう不具合対応
			BR_LOG_SIZE[i] = LogDatMax[i][0];			// ここでサイズを保持しておかないと0で書き込まれるため、FLT_initで削除されてしまう
// MH810104(E) 2021/10/27 車番チケットレス ログバージョンが不一致の場合、集計ログもクリアされてしまう不具合対応
			continue;
		}
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
		FLT_LogErase2(i);
		BR_LOG_SIZE[i] = LogDatMax[i][0];
	}
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );// 入出庫履歴のログもここでクリアする必要がある
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
	log_init();
	Log_clear_log_bakupflag();									// LOGデータ登録復電処理用のフラグ、退避データクリア処理
	FLT_req_tbl_clear();										// ログサイズ情報書き込みの前に要求をクリアする必要がある
	FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ログサイズ情報書き込み
	// FLT_WriteBRLOGSZ()で_flt_EnableTaskChg()がコールされ、タスクチェンジの発生を抑止する
	_flt_DisableTaskChg();
}
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// アイドルサブタスクへ移動
//-// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
//-//[]----------------------------------------------------------------------[]
//-///	@brief		長期駐車チェック
//-//[]----------------------------------------------------------------------[]
//-///	@return		
//-///	@author		A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/05/16<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//void	LongTermParkingCheck( void )
//{
//	ushort	wHour;
//	ushort	i;
//	ulong	ulwork;
//
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){					// 長期駐車検出する
///	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
//	if(prm_wk == 0){	// 長期駐車検出(アラームデータによる通知)
//		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車
//		if(wHour == 0){
//			wHour = 48;												// 設定が0の場合は強制的に48時間とする
//		}
//		for( i = 0; i < LOCK_MAX; i++ ){
//			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
//			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// 車室有効
//				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//「出庫処理中」または「空車」以外で有効
//					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報取得
//					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// 指定時間以上停車
//						if(LongParkingFlag[i] == 0){					// 長期駐車状態：長期駐車状態なし
//							LongParkingFlag[i] = 1;						// 長期駐車状態あり
//							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 1, 2, 1, &ulwork);	// A0031登録 長期駐車検出
//						}
//					}else{// 指定時間未満
//						if(LongParkingFlag[i] != 0){					// 長期駐車状態：長期駐車状態あり
//							// 時刻変更により長期駐車状態「あり」→「なし」に変化する場合は解除する
//							LongParkingFlag[i] = 0;						// 長期駐車状態なし
//							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);	// A0031解除 長期駐車検出
//						}
//					}
//				}
//			}
//		}
//	}
//
//}
//-//[]----------------------------------------------------------------------[]
//-///	@brief		長期駐車時間チェック
//-//[]----------------------------------------------------------------------[]
//-///	@param[in]  flp_com *p   : 先頭番号
//-///	@param[in]  short   hours: 駐車時間 
//-///	@return     ret          : 0:指定時間未満 1:指定時間到達
//-///	@author     A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/05/16<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
//short LongTermParking_stay_hour_check(flp_com *p, short hours)
//{
//	short	year, mon, day, hour, min;
//	short	add_day, add_hour;
//
//	year = p->year;		/* 入庫年 */
//	mon = p->mont;		/* 入庫月 */
//	day = p->date;		/* 入庫日 */
//	hour = p->hour;		/* 入庫時 */
//	min = p->minu;		/* 入庫分 */
//
//	// 駐車時間(時)を日、時に換算する
//	add_day = hours / 24;
//	add_hour = hours % 24;
//
//	// 入庫日時からadd_hour時間後の日時を取得する
//	hour += add_hour;
//	if(hour >= 24){
//		day++;//1日加算
//		hour = hour % 24;
//		for ( ; ; ) {
//			if (day <= medget(year, mon)) {// 月末以下
//				break;
//			}
//			// 該当月の日数を引いて月を加算
//			day -= medget(year, mon);
//			if (++mon > 12) {
//				mon = 1;
//				year++;
//			}
//		}
//	}
//	// 入庫日時からadd_day日後の日時を取得する
//	day += add_day;
//	for ( ; ; ) {
//		if (day <= medget(year, mon)) {// 月末以下
//			break;
//		}
//		// 該当月の日数を引いて月を加算
//		day -= medget(year, mon);
//		if (++mon > 12) {
//			mon = 1;
//			year++;
//		}
//	}
//
//	if(CLK_REC.year > year){
//		return 1;	//現在年が長期駐車年を過ぎた(指定時間到達)
//	}
//	if(CLK_REC.year < year){
//		return 0;	//現在年が長期駐車年より過去(指定時間未満)
//	}
//	// 以下 年は同じ
//	if(CLK_REC.mont > mon){
//		return 1;	//現在月が長期駐車月を過ぎた(指定時間到達)
//	}
//	if(CLK_REC.mont < mon){
//		return 0;	//現在月が長期駐車月より過去(指定時間未満)
//	}
//	// 以下 月は同じ
//	if(CLK_REC.date > day){
//		return 1;	//現在日が長期駐車日を過ぎた(指定時間到達)
//	}
//	if(CLK_REC.date < day){
//		return 0;	//現在日が長期駐車日より過去(指定時間未満)
//	}
//	// 以下 日は同じ
//	if(CLK_REC.hour > hour){
//		return 1;	//現在時が長期駐車時を過ぎた(指定時間到達)
//	}
//	if(CLK_REC.hour < hour){
//		return 0;	//現在時が長期駐車時より過去(指定時間未満)
//	}
//	// 以下 時は同じ
//	if(CLK_REC.minu > min){
//		return 1;	//現在分が長期駐車分を過ぎた(指定時間到達)
//	}
//	if(CLK_REC.minu < min){
//		return 0;	//現在分が長期駐車分より過去(指定時間未満)
//	}
//	return 1;// 年月日時分一致(指定時間到達)
//}
//-//[]----------------------------------------------------------------------[]
//-///	@brief		長期駐車チェック(ドア閉時にセンターに対して送信する処理)
//-//[]----------------------------------------------------------------------[]
//-///	@return		
//-///	@author		A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/07/26<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//void	LongTermParkingCheck_Resend( void )
//{
//	ushort	wHour;
//	ushort	i;
//	ulong	ulwork;
//
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){					// 長期駐車検出する
//	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
//	if((prm_wk == 0) || (prm_wk == 2)){
//		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車
//		if(wHour == 0){
//			wHour = 48;												// 設定が0の場合は強制的に48時間とする
//		}
//
//		if( isDefToErrAlmTbl(1, ALMMDL_MAIN, ALARM_LONG_PARKING) == FALSE ){
//			// アラームデータの送信レベルの設定（34-0037③）で送信するレベルでない場合はチェックしない
//			return;
//		}
//		// ドア閉時に長期駐車チェックを行い検出中であればセンターに対して再送するためにログに登録する
//		for( i = 0; i < LOCK_MAX; i++ ){
//			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
//			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// 車室有効
//				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//「出庫処理中」または「空車」以外で有効
//					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報取得
//					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// 指定時間以上停車
//						// A0031登録 長期駐車検出
//						memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
//						Arm_work.Armsyu = ALMMDL_MAIN;										// ｱﾗｰﾑ種別
//						Arm_work.Armcod = ALARM_LONG_PARKING;								// ｱﾗｰﾑｺｰﾄﾞ
//						Arm_work.Armdtc = 1;												// ｱﾗｰﾑ発生/解除
//											
//						Arm_work.Armlev = (uchar)getAlmLevel( ALMMDL_MAIN, ALARM_LONG_PARKING );// ｱﾗｰﾑﾚﾍﾞﾙ
//						Arm_work.ArmDoor = ERR_LOG_RESEND_F;								// 再送状態とする
//						Arm_work.Arminf = 2;												// 付属ﾃﾞｰﾀ(bin)あり
//						Arm_work.ArmBinDat = ulwork;										// bin アラーム情報をｾｯﾄ
//
//						Log_regist( LOG_ALARM );											// ｱﾗｰﾑﾛｸﾞ登録
//					}
//				}
//			}
//		}
//	}
//
//}
//-// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH810104 GG119201(S) 電子ジャーナル対応
//[]----------------------------------------------------------------------[]
///	@brief			時計合わせ（電子ジャーナル用）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EJA_TimeAdjust(void)
{
	ushort	adj_time;

	if (!isEJA_USE()) {
		return;
	}

	adj_time = (ushort)prm_tim(COM_PRM, S_REC, 1);
	if (adj_time == CLK_REC.nmin) {
		// 時刻設定要求
		MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
	}
}
// MH810104 GG119201(E) 電子ジャーナル対応


