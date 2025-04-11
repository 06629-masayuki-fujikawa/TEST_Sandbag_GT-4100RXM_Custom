/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽ操作                                                         |*/
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
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"AppServ.h"
#include	"sysmnt_def.h"
#include	"fla_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"ntnet.h"
#include	"flp_def.h"
#include	"tbl_rkn.h"
#include	"rkn_fun.h"
#include	"ntnet_def.h"
#include	<stdlib.h>
#include	"remote_dl.h"
#include	"IFM.h"
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)

/* Function define */
/* SysMntMain()でCallする */
unsigned short	SysMnt_Movcnt( void );
unsigned short	SysMnt_LkMovcnt( void );
unsigned short	SysMnt_LkMcntClr( short kind, ushort s_no, char ctype );
unsigned short	SysMnt_CardIssue( void );
unsigned short	SysMnt_MvcntMenu( void );
// MH810100(S) Y.Yamauchi 20191009 車番チケットレス(メンテナンス)
unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 20191009 車番チケットレス(メンテナンス)

ulong	BitShift_Left(ulong ,int );
void	LogDataClr_CheckBufferFullRelease(ushort LogSyu );

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* retry count, when bcc error occur */
#define	_SYSMNT_RETRY				2

#define	_SYSMNT_LOCK_ALL			(0xFFFFFFFF)

/* parameter of SysMntDisplayBackupRestoreTitle() */
enum {
	_SYSMNT_BKRS_START,
	_SYSMNT_BKRS_EXEC,
	_SYSMNT_BKRS_END,
};

/* parameter of sysmnt_GetMessage() */
enum {
	_SYSMNT_WAIT_MSG,
	_SYSMNT_NOT_WAIT_MSG
};

/* 料金計算テスト 画面モード */
enum {
	RT_MODE0 = 0,
	RT_MODE1,
	RT_MODE_MAX
};

/* 料金計算テスト サブ画面モード */
enum {
	RT_SUB_MODE0 = 0,
	RT_SUB_MODE1,
	RT_SUB_MODE2,
	RT_SUB_MODE3,
	RT_SUB_MODE_MAX
};

/* 料金計算テスト 基本画面 表示項目 */
enum {
	RT_I_YEAR = 0,
	RT_I_MONT,
	RT_I_DAY,
	RT_I_HOUR,
	RT_I_MIN,
	RT_O_YEAR,
	RT_O_MONT,
	RT_O_DAY,
	RT_O_HOUR,
	RT_O_MIN,
// MH810100(S) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)
//	RT_FLP_NO,
	RT_KIND,
// MH810100(E) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)
	RT_PASS,
	RT_BASE_MAX
};

/* 料金計算テスト 計算結果画面 表示項目 */
enum {
	RT_RYOUKIN = 0,
	RT_U_CARD,
	RT_RESULT_MAX
};

/* 料金計算テスト 定期券情報入力画面 表示項目 */
enum {
	RT_PASS_KIND = 0,
	RT_S_YEAR,
	RT_S_MONT,
	RT_S_DAY,
	RT_E_YEAR,
	RT_E_MONT,
	RT_E_DAY,
	RT_PASSINFO_MAX
};

/* 料金計算テスト 掛売先情報(掛売券用)入力画面 表示項目 */
enum {
	RT_KAKE_NO = 0,
	RT_KAKEINFO_MAX
};

/* 料金計算テスト サービス券種別入力画面 表示項目 */
enum {
	RT_SERV_KIND = 0,
	RT_SERVKIND_MAX
};

/* 料金計算テスト 使用カード */
enum {
	RT_CRD_TICK = 1,
	RT_CRD_PASS,
	RT_CRD_SERV,
	RT_CRD_KAKE
};

#define DISP_CHENGE(low, col, siz, zer, mod, cnt)	\
	do \
	{ \
		ulong tempdata = disp_Position((ushort)(cnt),&col,&siz);\
		opedpl3(low, col, tempdata, siz, zer, mod, COLOR_BLACK,  LCD_BLINK_OFF);	\
	} while (0)

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* parameter of SysMntUploadLog() */
typedef struct {
	ulong		record_size;								/* size of 1 record */
	const uchar	*filename;									/* filename to upload data of */
	const char	*ram_record;								/* top address of log records on ram */
	ulong		ram_record_num;								/* number of records on ram */
	ulong		(*get_record_num)(long *);					/* api pointer to get number of records on flash */
	ulong		(*get_first_log)(t_FltLogHandle*, char*);	/* api pointer to get oldest record data on flash */
	ulong		(*get_next_log)(t_FltLogHandle*, char*);	/* api pointer to get record data on flash */
}t_UploadLogParam;

struct T_FrmPcRyoukin{										/*								*/
	unsigned char			Flap_no;						/* ﾌﾗｯﾌﾟNo.						*/
	unsigned char			Kaisu;							/* 繰り返し数					*/
	date_time_rec			TInTime;						/*入庫日時(日付構造体参照)		*/
	date_time_rec			TOutTime;						/*出庫日時(日付構造体参照)		*/
	date_time_rec			InAdd;							/* 入庫加算量 					*/
	date_time_rec			OutAdd;							/* 出庫加算量					*/
	unsigned short			Ticket[10];						/* 券種							*/
	unsigned char			PkTicket;						/* '0':精算前駐車券				*/
	unsigned char			yobi1;							/* ﾌﾗｯﾌﾟNo.						*/
	unsigned char			yobi2;							/* ﾌﾗｯﾌﾟNo.						*/
															/*								*/
}; 															/*								*/

// 料金計算テスト表示項目制御テーブル構造
typedef struct {
	ushort	line;		// 行
	ushort	lpos;		// 列
	ushort	keta;		// 桁
	uchar	width;		// 半角(0)／全角(1)
	uchar	min;		// 最小値
	ushort	max1;		// 最大値1
	uchar	max2;		// 最大値2
} t_RT_DispItem;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_sysmnt_NextBlkNo(no)	((uchar)(((no)+1 > 99) ? 1 : (no)+1))

static	ushort	SysMnt_BackupRestore(void);
static	ushort	SysMnt_BackupRestoreData(ushort kind);
static	ushort	SysMnt_FileTransfer(void);
static	void	sysmnt_StartBkRsUpDwn(t_SysMnt_ErrInfo *errinfo);
#define			_sysmnt_StartMemoryInit(errinfo)	sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartBackupRestore(errinfo)	sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartUpload(errinfo)		sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartDownload(errinfo)		sysmnt_StartBkRsUpDwn(errinfo)
static	void	sysmnt_InitErr(t_SysMnt_ErrInfo *errinfo);
static	void	sysmnt_OnFlashErr(t_SysMnt_ErrInfo *errinfo, ulong fltcode);

static	void	SysMntDisplayBackupRestoreTitle(ushort kind, int phase);
static	void	SysMntDisplayResult(t_SysMnt_ErrInfo *errinfo);
static	short	sysmnt_WaitF5(void);
static	short	sysmnt_GetMessage(int option);
static	BOOL	sysmnt_RamLock(ulong lock);
static	void	sysmnt_RamUnlock(ulong unlock);
static uchar	SysMnt_Log_Printer(ushort LogSyu);
uchar	LogDatePosUp2( uchar *pos, uchar req );
void	LogDateGet2( date_time_rec *Old, date_time_rec *New, ushort *Date );
void	LogDateDsp2( ushort *Date, uchar pos );
uchar	LogDateChk2( uchar pos, short data );
uchar	LogDateChk_Range_Minute( uchar pos, short data );
static	void	RT_init_disp_item( void );
static	void	RT_base_disp( long *p_in );
// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
static	void	RT_attention_disp( void );
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
static	void	RT_change_disp(char mode, char sub_mode, char pos, long data, ushort rev);
static	uchar	RT_check_disp_data( long data );
static	uchar	RT_check_ymd( long year, long month, long day );
static	void	RT_get_disp_data( long *p_in );
static	void	RT_move_cursor( ushort msg, long *p_in );
static	uchar	RT_check_KEY_TEN_F3( long *p_in );
static	short	RT_ryo_cal( void );
static	uchar	RT_check_KEY_TEN( long *p_in, ushort msg );
static	uchar	RT_check_disp_change( long *p_in, ushort msg );
static	void	RT_recover_disp(void);
static	void	RT_init_cal_data( void );
static	void	RT_make_mag_data( char *p_data );
static	void	RT_log_regist( void );
static	void	RT_make_pass( m_gtapspas *p_magP );
static	void	RT_make_service( m_gtservic *p_magS );
static	short	RT_day_max( void );
static	short	RT_set_tim( ushort, struct clk_rec * );
static	void	RT_disp_cal_result( void );
static	void	RT_bkp_rst_data( char kind );
static	void	RT_clear_error( void );
static	void	RT_get_valid_no( char *p_data );
static	void	RT_ryo_cal2( char, ushort );
static	void	RT_edit_log_data( T_FrmReceipt *p_pri_data );
static	void	RT_vl_carchg( ushort );

unsigned char bin_to_asc( unsigned char c );

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/* timer for receive */
ushort	SysMnt_RcvTimer;

/* work */
extern	uchar	SysMnt_Work[SysMnt_Work_Size];	/* 64KB */
	char	SysMnt_Work2[4][36];
	char	SysMnt_Work3[6][20];
	char	SysMnt_UP_DW_Text[T_FOOTER_GYO_MAX][36];

	T_SYSMNT_PKT	SysMnt_SndBuf;
	T_SYSMNT_PKT	SysMnt_RcvBuf;

/* filename to upload & download */
	const uchar	SYSMNT_LOGO_FILENAME[]			= {"Logo.bmp"};
	const uchar	SYSMNT_HEADER_FILENAME[]		= {"Header.txt"};
	const uchar	SYSMNT_FOOTER_FILENAME[]		= {"Footer.txt"};
	const uchar	SYSMNT_PARAM_FILENAME[]			= {"Parameter.csv"};
	const uchar	SYSMNT_LOCKPARAM_FILENAME[]		= {"LockInfo.csv"};
	const uchar	SYSMNT_SALELOG_FILENAME[]		= {"SaleLog_%02d%02d%02d%02d%02d%02d.bin"};
	const uchar	SYSMNT_TOTALLOG_FILENAME[]		= {"TotalLog_%02d%02d%02d%02d%02d%02d.bin"};
	const uchar	SYSMNT_ACCEPTFTR_FILENAME[]		= {"uketuke.txt"};
	const uchar	SYSMNT_SYOMEI_FILENAME[]		= {"shomeiran.txt"};
	const uchar	SYSMNT_KAMEI_FILENAME[]			= {"kameiten.txt"};
	const uchar	SYSMNT_EDYAT_FILENAME[]			= {"EdyAtCmd.txt"};
	const uchar	SYSMNT_USERDEF_FILENAME[]		= {"userdefine.ini"};
	const uchar	SYSMNT_ERR_FILENAME[]			= {"ErrLog.csv"};
	const uchar	SYSMNT_ARM_FILENAME[]			= {"ArmLog.csv"};
	const uchar	SYSMNT_NONI_FILENAME[]			= {"MonitorLog.csv"};
	const uchar	SYSMNT_OPE_MONI_FILENAME[]		= {"OpeMonitorLog.csv"};
	const uchar	SYSMNT_CAR_INFO_FILENAME[]		= {"CarStatusInfo.csv"};
	const uchar	SYSMNT_TCARDFTR_FILENAME[]		= {"TCardFooter.txt"};
	const uchar	SYSMNT_AZUFTR_FILENAME[]		= {"AzukariFooter.txt"};
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
	const uchar	SYSMNT_CREKBRFTR_FILENAME[]		= {"CreKabaraiFooter.txt"};
	const uchar	SYSMNT_EPAYKBRFTR_FILENAME[]	= {"EpayKabaraiFooter.txt"};
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
	const uchar	SYSMNT_FUTUREFTR_FILENAME[]		= {"GojitsuFooter.txt"};
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
	const uchar	SYSMNT_EMGFOOTER_FILENAME[]		= {"emgfooter.txt"};
// MH810105(E) MH364301 QRコード決済対応
	const uchar	SYSMNT_REMOTE_DL_RESULT_FILENAME[] = {"RemoteDLResult.txt"};
	const uchar	SYSMNT_REMOTE_DL_PARAM[] 		= {"PARAMETER.TXT"};
	const uchar	SYSMNT_REMOTE_DL_CONNECT[] 		= {"CONNECT.TXT"};
// MH810100(S) K.Onodera 2019/11/20 車番チケットレス（→LCD パラメータ)
	const uchar	SYSMNT_UP_PARAM_LCD[] 			= {"PARAMETER.DAT"};
// MH810100(E) K.Onodera 2019/11/20 車番チケットレス（→LCD パラメータ)

/* flag to check mode change occured */
static	BOOL	SysMntModeChange;
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
static	BOOL	SysMntLcdDisconnect;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
t_RT_Ctrl	rt_ctrl;								// 料金計算テスト制御データ
													// mode=0:基本画面編集中
													// 		1:料金計算結果表示中
													// sub_mode=0:mode=0/1 基本画面編集中
													// 			1:mode=0/1 定期券情報編集中
													// 			2:mode=0/1 掛売先情報編集中
													// 			3:mode=  1 サービス券種別編集中
t_RT_Ctrl	rt_ctrl_bk1;							// 料金計算テスト制御データバックアップ領域(親画面⇔子画面遷移用)
t_RT_Ctrl	rt_ctrl_bk2;							// 料金計算テスト制御データバックアップ領域(初期画面⇔計算結果画面遷移用)
long	base_data[RT_BASE_MAX];						// 基本画面基本データ
long	rslt_data[RT_RESULT_MAX];					// 計算結果画面データ
long	pass_data[RT_PASSINFO_MAX];					// 定期券情報入力画面データ
long	kake_data[RT_KAKEINFO_MAX];					// 掛売先情報(掛売券用)入力画面データ
long	serv_data[RT_SERVKIND_MAX];					// サービス券種別画面データ
long	base_data_bk[RT_BASE_MAX];					// 基本画面基本データバックアップ
long	pass_data_bk[RT_PASSINFO_MAX];				// 定期券情報入力画面データ
long	kake_data_bk[RT_KAKEINFO_MAX];				// 掛売先情報(掛売券用)入力画面データ
long	serv_data_bk[RT_SERVKIND_MAX];				// サービス券種別画面データ

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

ushort	SysMnt_Log( void );
ushort	SysMnt_Log_CntDsp1( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd );
ushort	SysMnt_LogClr( void );
uchar	LogDatePosUp( uchar *pos, uchar req );
uchar	LogDateChk( uchar pos, short data );
void	LogCntDsp( ushort LogCnt );
void	LogDateDsp( ushort *Date, uchar pos );
void	LogDateDsp4( ushort *Date );
void	LogDateDsp5( ushort *Date, uchar pos );
ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req );
void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
uchar	LogDataClr( ushort LogSyu );

static ushort	card_iss_pass(void);
static void		cardiss_pass_prm_dsp(char pos, long prm, ushort rev);
static char		cardiss_pass_prm_chk( long *prm );
static uchar	FRdr_DipSW_GetStatus( void );
static ushort	card_iss_kakari(void);
static void		cardiss_kakari_prm_dsp(char pos, char prm, ushort rev);
static char		cardiss_kakari_prm_chk(char pos, char prm);

static ulong	disp_Position( ushort, ushort*, ushort*);

/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽﾒｲﾝ処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMntMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void SysMntMain( void )
{
	unsigned short	usSysEvent = MNT_NONE;
	char	wk[2];
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	uchar ucReq = 0;	// 0=OK/1=NG/2=OK(再起動不要)
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//	ushort	mode = 0;
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)

	DP_CP[0] = DP_CP[1] = 0;

	memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));	// 料金計算テスト制御データ初期化

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[0] );			// [00]	"＜システムメンテナンス＞　　　"

		if(( rt_ctrl.param.set != ON ) &&
		   ( rt_ctrl.param.no == 0 )){
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
// 			usSysEvent = Menu_Slt( SMMENU, SYS_TBL, (char)SYS_MENU_MAX, (char)1 );
			if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
				usSysEvent = Menu_Slt(SMMENU_CC, SYS_TBL_CC, (char)SYS_MENU_MAX_CC, (char)1);
			} else {					// 通常料金計算モード
				usSysEvent = Menu_Slt(SMMENU, SYS_TBL, (char)SYS_MENU_MAX, (char)1);
			}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usSysEvent ){

		/* パラメーター設定 */
			case MNT_PRSET:
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//				usSysEvent = ParSetMain();				
				usSysEvent = BPara_Set();
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
				break;
		/* 動作チェック */
			case MNT_FNCHK:
				usSysEvent = FncChkMain();
				break;
		/* 動作カウント */
			case MNT_FNCNT:
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//				usSysEvent = SysMnt_MvcntMenu();				
				usSysEvent = SysMnt_Movcnt();
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
				break;
		/* ログファイルプリント */
			case MNT_LOGFL:
				usSysEvent = SysMnt_Log();
				break;
		/* バックアップ／リストア */
			case MNT_BAKRS:
				usSysEvent = SysMnt_BackupRestore();
				break;
//未使用です。(S)
		/* カード発行 */
			case MNT_CDISS:
				if(0 == prm_get(COM_PRM, S_PAY, 21, 1, 3)) {	// 磁気リーダー接続なし(02-0021④=0)
					BUZPIPI();
					break;
				}
				if((prm_get( COM_PRM,S_SYS,12,1,6 ) == 1) &&
					((GTF_PKNO_LOWER > CPrmSS[S_SYS][1]) ||
					(CPrmSS[S_SYS][1] > GTF_PKNO_UPPER)) ){
					BUZPIPI();								//駐車場No設定間違い
					break;
				}else if((prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) &&
					(( APSF_PKNO_LOWER > CPrmSS[S_SYS][1] ) ||
					(CPrmSS[S_SYS][1] > APSF_PKNO_UPPER)) ){
					BUZPIPI();								//駐車場No設定間違い
					break;
				}
				usSysEvent = SysMnt_CardIssue();
				break;
//未使用です。(E)
		/* ファイル転送 */
			case MNT_FLTRF:
				usSysEvent = SysMnt_FileTransfer();
				break;
		/* システム切替 */
			case MNT_SYSSW:
				usSysEvent = SysMnt_SystemSwitch();
				break;
		/* 料金計算テスト */
			case MNT_RYOTS:
				ryo_test_flag = 1;
				usSysEvent = SysMnt_RyoTest();
				ryo_test_flag = 0;
				break;

			//Return Status of User Menu
// MH810100(S) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return;
				break;
// MH810100(E) Y.Yamauchi 20191213 車番チケットレス(メンテナンス)
			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return;
				break;

			default:
				break;
		}
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of User Operation
		if( usSysEvent == MOD_CHG ){
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			return;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
//			mode = op_wait_mnt_close();	// メンテナンス終了処理
//			if( mode == MOD_CHG ){
			{
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
				if( mnt_GetFtpFlag() != FTP_REQ_NONE ){
					ucReq = 1;	// 失敗
					if( 0 == lcdbm_setting_upload_FTP() ){		// LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞを行う
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
// MH810100(S) S.Nishimoto 2020/09/02 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
			}
			else {
				// 料金計算テストからの戻りでメニューを表示できないので、
				// 料金計算データををクリアする
				memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));	// 料金計算テスト制御データ初期化
			}
// MH810100(E) S.Nishimoto 2020/09/02 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4777 メンテナンスでブザー音量変更後にドア閉から精算ボタンを押しても精算開始できない)
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if( usSysEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;							// 初期化状態へ
			OPECTL.init_sts = 0;							// 初期化未完了状態とする
			OPECTL.Pay_mod = 0;								// 通常精算
// MH810100(S) K.Onodera 2020/03/13 車番チケットレス(#3887 通信復旧時の考慮漏れ修正)
			OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
// MH810100(E) K.Onodera 2020/03/13 車番チケットレス(#3887 通信復旧時の考慮漏れ修正)
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

/*[]----------------------------------------------------------------------[]*/
/*| 動作カウントメニュー                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_MvcntMenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//未使用です。（S）
unsigned short	SysMnt_MvcntMenu( void )
{
	unsigned short	usCntEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[1] );			// "＜動作カウント＞　　　　　　　"

		Is_CarMenuMake(MV_CNT_MENU);

		usCntEvent = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usCntEvent ){

		/* 1.動作カウント */
			case FCNT_FUNC:
				wopelg( OPLOG_DOUSACOUNT, 0, 0 );	// 操作履歴登録
				usCntEvent = SysMnt_Movcnt();
				break;

		/* 2.フラップ装置動作カウント */
			case FCNT_FLAP:
				gCurSrtPara = FCNT_FLAP;
				wopelg( OPLOG_FLAPDCNT, 0, 0 );		// フラップ装置動作カウント操作履歴登録
				MovCntClr = OFF;					// 動作ｶｳﾝﾄｸﾘｱ実行状態を初期化（無ｾｯﾄ）
				usCntEvent = SysMnt_LkMovcnt();
				if( MovCntClr == ON ){					// 動作ｶｳﾝﾄｸﾘｱ実行あり？
					wopelg( OPLOG_FLAPDCNTCLR, 0, 0 );	// 操作履歴登録（ﾌﾗｯﾌﾟ装置動作ｶｳﾝﾄｸﾘｱ）
				}
				break;

		/* 3.ロック装置動作カウント */
			case FCNT_ROCK:
				gCurSrtPara = FCNT_ROCK;
				wopelg( OPLOG_LOCKDCNT, 0, 0 );		// ロック装置動作カウント操作履歴登録
				MovCntClr = OFF;					// 動作ｶｳﾝﾄｸﾘｱ実行状態を初期化（無ｾｯﾄ）
				usCntEvent = SysMnt_LkMovcnt();
				if( MovCntClr == ON ){					// 動作ｶｳﾝﾄｸﾘｱ実行あり？
					wopelg( OPLOG_LOCKDCNTCLR, 0, 0 );	// 操作履歴登録（ﾛｯｸ装置動作ｶｳﾝﾄｸﾘｱ）
				}
				break;

			//Return Status of User Menu
			case MOD_EXT:
				return MOD_EXT;

			default:
				break;
		}
		//Return Status of User Operation
		if( usCntEvent == MOD_CHG ){
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if( usCntEvent == MOD_CUT ){
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];	
	}
}
//未使用です。（E）
/*[]----------------------------------------------------------------------[]*/
/*| 動作ｶｳﾝﾄﾁｪｯｸ                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_Movcnt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura 仕様修正に対応                       |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SysMnt_Movcnt( void )
{
	ushort	msg;
	int		top;
	int		pos;
	char	i;
	char	mod;		/* 0:プリント画面 1:クリア画面 2:全クリア画面 */
	char	pgchg;
	T_FrmDousaCnt	DousaCntData;
	ushort	culmn = 17;
	ushort	datasize = 9;
// MH810100(S) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
	int		inji_end = 0;
// MH810100(E) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)


	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[1] );		// [01]	"＜動作カウント＞　　　　　　　"
	Fun_Dsp( FUNMSG[59] );					// [59]	"　▲　　▼　クリア ﾌﾟﾘﾝﾄ 終了 "

	top = 0;
	mod = 0;
	pgchg = 1;
	for( ; ; ){

		if (pgchg) {
			for (i = 0; i < 6 ; i++) {
				if (top+i < MOV_CNT_MAX){
					grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CNTSTR[top+i] );				// 各項目表示
					DISP_CHENGE((ushort)(1+i),culmn,datasize,0,0,top+i);			// ｶｳﾝﾄ表示
				}
				else {
					displclr((ushort)(i+1));									// 未使用の行はｸﾘｱ
				}
			}
			if (mod == 1) {
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);		// ｶｳﾝﾄ表示(反転)
			}
			pgchg = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT) {
			BUZPI();
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
	/*--- プリント画面 ---*/
		if (mod == 0) {
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F5:	// F5:Exit
				BUZPI();
				return( MOD_EXT );
			case KEY_TEN_F1:	// F1:"▲"
			case KEY_TEN_F2:	// F2:"▼"
				BUZPI();
				if (msg == KEY_TEN_F1) {
					top -= 6;
					if (top < 0)
						top = (char)((MOV_CNT_MAX-1)/6*6);
				}
				else {
					top += 6;
					if (top >= MOV_CNT_MAX)
						top = 0;
				}
				pgchg = 1;
				break;
			case KEY_TEN_F3:	// F3:"ｸﾘｱ"
				BUZPI();
				mod = 1;
				pos = top;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ｶｳﾝﾄ表示(反転)
				Fun_Dsp( FUNMSG[62] );		// [62]	"　▲　　▼　クリア 全て  終了 "
				wopelg( OPLOG_DOUSACOUNTCLR, 0, 0 );	// 操作履歴登録
				break;
			case KEY_TEN_F4:	// F4:"ﾌﾟﾘﾝﾄ"
				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
					BUZPIPI();
					break;
				}
				BUZPI();
				/* プリント実行 */
				DousaCntData.prn_kind = R_PRI;
				DousaCntData.Kikai_no = (uchar)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_DOUSACNT, sizeof( T_FrmDousaCnt ), &DousaCntData );	//Move Count Print
				Ope_DisableDoorKnobChime();

				/* プリント終了を待ち合わせる */
				Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					/* プリント終了 */
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
// MH810100(S) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						inji_end = 1;
// MH810100(E) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						break;
					}
					/* タイムアウト(10秒)検出 */
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* モードチェンジ */
					if (msg == KEY_MODECHG) {
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						// ドアノブの状態にかかわらずトグル動作してしまうので、
						// ドアノブ閉かどうかのチェックを実施
						if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
						}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if ( msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					
				}
				break;
			default:
				break;
			}
		}
	/*--- クリア画面 ---*/
		else if (mod == 1) {
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F5:	// F5:Exit
				BUZPI();
				mod = 0;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,0,pos);			// ｶｳﾝﾄ表示(正転)
				Fun_Dsp( FUNMSG[59] );					// [59]	"　▲　　▼　クリア ﾌﾟﾘﾝﾄ 終了 "
				break;
			case KEY_TEN_F3:	// F3:"ｸﾘｱ"
				Mov_cnt_dat[pos] = 0;
// MH810100(S) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
				if (pos == LCD_LIGTH_CNT) {
					lcdbm_notice_ope(LCDBM_OPCD_CLEAR_COUNT, 0);
				}
// MH810100(E) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
				/* break */
			case KEY_TEN_F1:	// F1:"▲"
			case KEY_TEN_F2:	// F2:"▼"
				BUZPI();
				if (msg == KEY_TEN_F1) {
					pos--;
					if (pos < 0) {
						pos = (char)(MOV_CNT_MAX-1);
						top = (char)(pos/6*6);
						pgchg = 1;
					}
					else if (pos < top) {
						top -= 6;
						pgchg = 1;
					}
					else {
						DISP_CHENGE((ushort)(1+pos+1-top),culmn,datasize,0,0,pos+1);		// ｶｳﾝﾄ表示(正転) 前の行
						DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ｶｳﾝﾄ表示(反転) 今の行
					}
				}
				else {
					pos++;
					if (pos >= MOV_CNT_MAX) {
						pos = 0;
						top = 0;
						pgchg = 1;
					}
					else if (pos >= top+6) {
						top += 6;
						pgchg = 1;
					}
					else {
						DISP_CHENGE((ushort)(1+pos-1-top),culmn,datasize,0,0,pos-1);		// ｶｳﾝﾄ表示(正転) 前の行
						DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ｶｳﾝﾄ表示(反転) 今の行
					}
				}
				break;
			case KEY_TEN_F4:	// F4:"全ｸﾘｱ"
				BUZPI();
				mod = 2;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,0,pos);			// ｶｳﾝﾄ表示(正転)
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR2[9]);		/* "　　　全てクリアしますか？　　" */
				Fun_Dsp( FUNMSG[19] );				/* "　　　　　　 はい いいえ　　　" */
				break;
			default:
				break;
			}
		}
	/*--- 全クリア画面 ---*/
		else {	/* if (mod == 2) */
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F3:	// F3:"はい"
				/* 全データクリア */
				memset(Mov_cnt_dat, 0, sizeof(Mov_cnt_dat));
// MH810100(S) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
				lcdbm_notice_ope(LCDBM_OPCD_CLEAR_COUNT, 0);
// MH810100(E) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
			case KEY_TEN_F4:	// F4:"いいえ"
				BUZPI();
				Fun_Dsp( FUNMSG[62] );		// [62]	"　▲　　▼　クリア 全て  終了 "
				pgchg = 1;
				mod = 1;
				break;
			default:
				break;
			}
		}

// MH810100(S) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
		if (inji_end == 1 && CP_MODECHG) {
			// 印字終了後、ドアノブ閉状態であればメンテナンス終了
			return MOD_CHG;
		}
// MH810100(E) S.Nishimoto 2020/09/02 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		動作カウント数表示位置分岐処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	disp_Position( cnt,  culmn, datasize)				|*/
/*|																			|*/
/*|	PARAMETER		:	ushort cnt			表示する項目要素数				|*/
/*|						ushort *culmn		表示位置						|*/
/*|						ushort *datasize	表示サイズ						|*/
/*|																			|*/
/*|	RETURN VALUE	:	ulong	 			表示する動作カウント			|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	fsiabc T.Namioka											|*/
/*|	Date	:	2006-02-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
static ulong	disp_Position( ushort cnt, ushort *culmn, ushort *datasize)
{
	ulong tmpdata=0;
// MH810100(S) Y.Yamauchi 2019/10/26 車番チケットレス(メンテナンス)
//	if(cnt == LCD_LIGTH_CNT){
//		tmpdata = Mov_cnt_dat[cnt]/60;	//分単位を時間単位に変換したデータの設定
//		*culmn = 16;					//カウント数の表示開始位置の設定
//		*datasize = 8;					//データサイズの設定
//	//表示項目がLCDﾊﾞｯｸﾗｲﾄ点灯以外の場合
//	}else{
//		tmpdata = Mov_cnt_dat[cnt];		//データの設定
//		*culmn = 17;					//カウント数の表示開始位置の設定
//		*datasize = 9;					//データサイズの設定
//	}
//	return tmpdata;
//		
//}
	//表示項目がLCDまたはQRﾘｰﾀﾞｰ動作時間だった場合
	if(cnt == QR_READER_CNT){
		tmpdata = Mov_cnt_dat[cnt]/60;	//分単位を時間単位に変換したデータの設定
		*culmn = 16;					//カウント数の表示開始位置の設定
		*datasize = 8;					//データサイズの設定
	}else if(cnt == LCD_LIGTH_CNT){
		tmpdata = Mov_cnt_dat[cnt];	
		*culmn = 16;					//カウント数の表示開始位置の設定
		*datasize = 8;					//データサイズの設定
	}
	//表示項目がLCD,QR以外の場合
	else{
		tmpdata = Mov_cnt_dat[cnt];		//データの設定
		*culmn = 17;					//カウント数の表示開始位置の設定
		*datasize = 9;					//データサイズの設定
	}
// MH810100(E) Y.Yamauchi 2019/10/26 車番チケットレス(メンテナンス)
	
	return tmpdata;
}


unsigned short	SysMnt_LkMovcnt( void )
{
	short	msg = -1;
	ushort	page[2];
	short	inpt = -1;
	char	timeout = 1;
	T_FrmLockDcnt	pri_data;
	char	fk_page = 0;			// ﾌｧﾝｸｼｮﾝｷｰ表示画面（0：動作ｶｳﾝﾄ表示画面／1:ｸﾘｱ操作画面）
	ushort	s_no_wk;				// 装置No.ﾜｰｸ
	ushort	s_no_max;				// 最大装置No.
	ulong	cnt1,cnt2,cnt3;			// 動作ｶｳﾝﾀｰ格納ﾜｰｸ
	uchar	re_dsp = 0;				// 全画面再表示ﾌﾗｸﾞ
	ushort	ret;					// ｸﾘｱ確認画面戻りｽﾃｰﾀｽ
	char	type;
	ushort	tno;					// 端末No.
	ushort	lock_no;				// 端末内通番

	dispclr();

	if( gCurSrtPara == FCNT_FLAP ){
		// フラップ装置動作カウント
		memset( MntFlapDoCount, 0xff, sizeof( MntFlapDoCount ) );	// ﾌﾗｯﾌﾟ装置動作ｶｳﾝﾄﾊﾞｯﾌｧ0xffｸﾘｱ
		type = (char)GetCarInfoParam();
		switch( type&0x06 ){
			case	0x04:
				s_no_max = INT_CAR_LOCK_MAX;						// ﾌﾗｯﾌﾟ装置最大装置No.ｾｯﾄ
				type = _MTYPE_INT_FLAP;
				break;
			case	0x06:
				s_no_max = CAR_LOCK_MAX+INT_CAR_LOCK_MAX;			// ﾌﾗｯﾌﾟ装置最大装置No.ｾｯﾄ
				type = (uchar)(_MTYPE_FLAP + _MTYPE_INT_FLAP);
				break;
			case	0x02:
			default:
				s_no_max = CAR_LOCK_MAX;							// ﾌﾗｯﾌﾟ装置最大装置No.ｾｯﾄ
				type = _MTYPE_FLAP;
				break;
		}
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[75] );							// [75]	"＜フラップ装置動作カウント＞　"
		for(s_no_wk = 0; s_no_wk < s_no_max; ++s_no_wk) {
			lock_no = 1;
			
			if(type == _MTYPE_INT_FLAP) {
				tno = LockInfo[s_no_wk + INT_CAR_START_INDEX].if_oya;
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// 動作回数
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// 手動動作回数
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// 故障回数
			}
			else {
				tno = LockInfo[s_no_wk].if_oya;
				MntFlapDoCount[s_no_wk][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// 動作回数
				MntFlapDoCount[s_no_wk][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// 手動動作回数
				MntFlapDoCount[s_no_wk][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// 故障回数
			}
		}
	}
	else{
		// ロック装置動作カウント
		s_no_max = BIKE_LOCK_MAX;									// ﾛｯｸ装置最大装置No.ｾｯﾄ
		memset( MntLockDoCount, 0xff, sizeof( MntLockDoCount ) );	// ﾛｯｸ装置動作ｶｳﾝﾄﾊﾞｯﾌｧ0xffｸﾘｱ
		type = _MTYPE_LOCK;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[2] );							// [02]	"＜ロック装置動作カウント＞　　"
		for(s_no_wk = 0; s_no_wk < s_no_max; ++s_no_wk) {
			tno = LockInfo[s_no_wk + BIKE_START_INDEX].if_oya;
			lock_no = LockInfo[s_no_wk + BIKE_START_INDEX].lok_no;

			MntLockDoCount[s_no_wk][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// 動作回数
			MntLockDoCount[s_no_wk][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// 手動動作回数
			MntLockDoCount[s_no_wk][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// 故障回数
		}
	}
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"　　 しばらくお待ち下さい 　　"
	page[0] = page[1] = 1;

	if( gCurSrtPara == FCNT_FLAP ){
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"　フラップ装置　　　　　　　　"
	}
	else{
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"　ロック装置　　　　　　　　　"
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );	// [04]	"　動作回数　　　　　　　　回　"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );	// [05]	"　手動動作回数　　　　　　回　"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );	// [06]	"　故障回数　　　　　　　　回　"
	Fun_Dsp( FUNMSG[88] );				// [88]	"  ＋  －／読 ﾌﾟﾘﾝﾄ 次へ  終了 "
	page[1] = page[0] + 1;				// 強制的に画面更新させるためにpage[1]≠page[0]とする
	timeout = 0;
	goto LInit;

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg )){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );		// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );		// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"終了"
				if( timeout )	break;
				BUZPI();
				Lagcan( OPETCBNO, 6 );		// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1
				if( timeout )	break;

				if( fk_page == 0 ){
					// ｶｳﾝﾄ表示画面："＋"
					BUZPI();
					inpt = -1;
					page[0] = (( page[0] >= s_no_max ) ? 1 : page[0] + 1 );
				}
				else{
					// ｸﾘｱ操作画面："ｸﾘｱ"
					if( gCurSrtPara == FCNT_FLAP ){
						// ﾌﾗｯﾌﾟ装置
						if( type == _MTYPE_INT_FLAP )
							s_no_wk = page[0]+INT_CAR_START_INDEX;	// 装置No.ｾｯﾄ（51～100）
						else
						s_no_wk = page[0];					// 装置No.ｾｯﾄ（1～50）
					}
					else{
						// ﾛｯｸ装置
						s_no_wk = page[0]+BIKE_START_INDEX;	// 装置No.ｾｯﾄ（101～150）
					}
					if( LockInfo[s_no_wk-1].lok_syu ){
						// 接続あり
						BUZPI();
						ret = SysMnt_LkMcntClr( gCurSrtPara, s_no_wk, type );	// ｸﾘｱ確認画面処理
					if( ret == MOD_CHG ){
						return( MOD_CHG );
						}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if( ret == MOD_CUT ){
						return( MOD_CUT );
						}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						page[1] = page[0] + 1;		// 強制的に画面更新させるためにpage[1]≠page[0]とする
						re_dsp = 1;					// 全画面再表示
					}
					else{
						// 接続なし
						BUZPIPI();
					}
				}
				break;

			case KEY_TEN_F2:	// F2
				if( timeout )	break;
				BUZPI();

				if( fk_page == 0 ){
					// ｶｳﾝﾄ表示画面："-/読"
					if( inpt == -1 ){
						// 装置No.未入力の場合："-"
						page[0] = (( page[0] <= 1 ) ? s_no_max : page[0] - 1 );
					}
					else{
						// 装置No.入力済みの場合："読出"
						if(( 1 <= inpt )&&( inpt <= s_no_max )){
							BUZPI();
							page[0] = inpt;
							inpt = -1;
						}else if( inpt == -1 ){
							BUZPI();
						}else{
							BUZPIPI();
							opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
							inpt = -1;
						}
					}
				}
				else{
					// ｸﾘｱ操作画面："全ｸﾘｱ"
					ret = SysMnt_LkMcntClr( gCurSrtPara, 0, type );	// ｸﾘｱ確認画面処理
					if( ret == MOD_CHG ){
						return( MOD_CHG );
					}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
					if( ret == MOD_CUT ){
						return( MOD_CUT );
					}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
					page[1] = page[0] + 1;		// 強制的に画面更新させるためにpage[1]≠page[0]とする
					re_dsp = 1;					// 全画面再表示
				}
				break;

			case KEY_TEN_F3:	// F3:"ﾌﾟﾘﾝﾄ"
				if( timeout || fk_page ){		// 動作ｶｳﾝﾄ読出中　または　ｸﾘｱ操作画面ではﾌﾟﾘﾝﾄｷｰ入力不可
					BUZPIPI();
					break;
				}
				if(Ope_isPrinterReady() == 0){	// レシートプリンタが印字不可能な状態
					BUZPIPI();
					break;
				}
				BUZPI();

				/* ﾌﾟﾘﾝﾀ印字要求 */
				pri_data.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
				pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// 機械No.
				pri_data.Req_syu = type;							// 1:ﾛｯｸ 2:IFﾌﾗｯﾌﾟ 3:内蔵ﾌﾗｯﾌﾟ 5:ﾌﾗｯﾌﾟ両方																	
				queset(PRNTCBNO, PREQ_LK_DOUSACNT, sizeof(T_FrmLockDcnt), &pri_data);
				Ope_DisableDoorKnobChime();

				/* プリント終了を待ち合わせる */
				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					/* プリント終了 */
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					/* タイムアウト(10秒)検出 */
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* モードチェンジ */
					if (msg == KEY_MODECHG) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if (msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				}
				break;

			case KEY_TEN_CL:	/* 取消 */
				if( timeout || fk_page )	break;	// 動作ｶｳﾝﾄ読出中　または　ｸﾘｱ操作画面では取消ｷｰ入力不可
				BUZPI();
				if(inpt != -1)	inpt = -1;
				opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;

			case KEY_TEN_F4:	// F4:"次へ"（ﾌｧﾝｸｼｮﾝｷｰ切替）
				if( timeout )	break;
				BUZPI();
				fk_page ^= 1;
				if (fk_page) {
					opedsp( 2, 22, page[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// 装置No.反転表示OFF
					Fun_Dsp(FUNMSG[89]);														// " ｸﾘｱ  全ｸﾘｱ        次へ  終了 "
				} else {
					opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// 装置No.反転表示ON
					Fun_Dsp(FUNMSG[88]);														// "  ＋  －／読 ﾌﾟﾘﾝﾄ 次へ  終了 "
				}
				break;
				
			case TIMEOUT6:		// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				if( gCurSrtPara == FCNT_FLAP ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"　フラップ装置　　　　　　　　"
				}
				else{
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"　ロック装置　　　　　　　　　"
				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );	// [04]	"　動作回数　　　　　　　　回　"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );	// [05]	"　手動動作回数　　　　　　回　"
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );	// [06]	"　故障回数　　　　　　　　回　"
				Fun_Dsp( FUNMSG[88] );				// [88]	"  ＋  －／読 ﾌﾟﾘﾝﾄ 次へ  終了 "
				page[1] = page[0] + 1;				// 強制的に画面更新させるためにpage[1]≠page[0]とする
				timeout = 0;
				break;

			case KEY_TEN:
				if( timeout || fk_page )	break;	// 動作ｶｳﾝﾄ読出中　または　ｸﾘｱ操作画面ではﾃﾝｷｰ入力不可
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 100 ) * 10 + (short)(msg - KEY_TEN0);
				opedsp( 2, 22, (ushort)inpt, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;

			default:
				break;
		}
LInit:
		/*** Page Change ***/
		if( page[0] != page[1] ){

			if( re_dsp ){
				// 全画面再表示（ｸﾘｱ画面からの復帰）
				if( gCurSrtPara == FCNT_FLAP ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"　フラップ装置　　　　　　　　"
				}
				else{
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"　ロック装置　　　　　　　　　"
				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );		// [04]	"　動作回数　　　　　　　　回　"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );		// [05]	"　手動動作回数　　　　　　回　"
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );		// [06]	"　故障回数　　　　　　　　回　"
				if (fk_page) {
					Fun_Dsp(FUNMSG[89]);												// " ｸﾘｱ  全ｸﾘｱ        次へ  終了 "
					opedsp( 2, 22, page[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ロック装置Ｎｏ．表示（通常表示）
				} else {
					Fun_Dsp(FUNMSG[88]);												// "  ＋  －／読 ﾌﾟﾘﾝﾄ 次へ  終了 "
					opedsp( 2, 22, page[0], 3, 1, 1 , COLOR_BLACK, LCD_BLINK_OFF);	// ロック装置Ｎｏ．表示（反転表示）
				}
				re_dsp = 0;																// 全画面再表示ﾌﾗｸﾞﾘｾｯﾄ
			}
			else{
				opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ロック装置Ｎｏ．表示（反転表示）
			}
			if( gCurSrtPara == FCNT_FLAP ){
				// ﾌﾗｯﾌﾟ装置
				if( type == _MTYPE_INT_FLAP )
					s_no_wk = page[0]+INT_CAR_START_INDEX;	// 装置No.ｾｯﾄ（51～100）
				else
				s_no_wk = page[0];						// 装置No.ｾｯﾄ（1～50）
			}
			else{
				// ﾛｯｸ装置
				s_no_wk = page[0]+BIKE_START_INDEX;		// 装置No.ｾｯﾄ（101～150）
			}

			if( LockInfo[s_no_wk-1].lok_syu ){
				/*** ﾛｯｸ装置接続有り ***/
				s_no_wk = page[0]-1;						// 動作ｶｳﾝﾄﾃﾞｰﾀの配列No.ｾｯﾄ
				if( gCurSrtPara == FCNT_FLAP ){
					if( type == _MTYPE_INT_FLAP )
						s_no_wk = s_no_wk+INT_CAR_START_INDEX;	// Indexｾｯﾄ（50～99）
					// ﾌﾗｯﾌﾟ装置
					cnt1 = MntFlapDoCount[s_no_wk][0];		// 動作回数
					cnt2 = MntFlapDoCount[s_no_wk][1];		// 手動動作回数
					cnt3 = MntFlapDoCount[s_no_wk][2];		// 故障回数
				}
				else{
					// ﾛｯｸ装置
					cnt1 = MntLockDoCount[s_no_wk][0];		// 動作回数
					cnt2 = MntLockDoCount[s_no_wk][1];		// 手動動作回数
					cnt3 = MntLockDoCount[s_no_wk][2];		// 故障回数
				}

				if( cnt1 == 0xffffffff ){
					/*** 動作ｶｳﾝﾄﾃﾞｰﾀ未受信 ***/
					grachr( 1, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][10] );	// [07]	"　　　　　　　　　　　未受信　"
					grachr( 3, 20, 6, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"　　　　　　　　　　******　　"
					grachr( 4, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"　　　　　　　　　　　****　　"
					grachr( 5, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"　　　　　　　　　　　****　　"
				}else{
					/*** 動作ｶｳﾝﾄﾃﾞｰﾀ受信済み ***/
					displclr( 1 );							// Line Clear
					opedpl3( 3, 20, cnt1, 6, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 動作回数表示
					opedpl3( 4, 22, cnt2, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 手動動作回数表示
					opedpl3( 5, 22, cnt3, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 故障回数表示
				}
			}else{
				/*** ﾛｯｸ装置接続無し ***/
				grachr( 1, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][0] );		// [07]	"　　　　　　　　　　　未接続　"
				grachr( 3, 20, 6, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"　　　　　　　　　　******　　"
				grachr( 4, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"　　　　　　　　　　　****　　"
				grachr( 5, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"　　　　　　　　　　　****　　"
			}
			page[1] = page[0];
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ/ﾛｯｸ装置動作ｶｳﾝﾄｸﾘｱ確認画面処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_LkMcntClr( kind, s_no  )                         |*/
/*|                                                                        |*/
/*| PARAMETER    : short kind：動作ｶｳﾝﾄ種別                               |*/
/*|                  FCNT_FLAP=ﾌﾗｯﾌﾟ動作ｶｳﾝﾄ                               |*/
/*|                  FCNT_ROCK=ﾛｯｸ装置動作ｶｳﾝﾄ                             |*/
/*|                                                                        |*/
/*|                ushort s_no：装置No.                                    |*/
/*|                  0    =全動作ｶｳﾝﾄｸﾘｱ                                   |*/
/*|                  0以外=個別動作ｶｳﾝﾄｸﾘｱ                                 |*/
/*|                                                                        |*/
/*| RETURN VALUE : ushort 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	SysMnt_LkMcntClr( short kind, ushort s_no, char ctype  )
{
	ushort	ret;
	ushort	line;
	short	msg;
	ushort	s_no_wk;
	char	type;
	short	search_max;

	for( line = 1 ; line < 6 ; line++ ){
		displclr( line );					// 動作ｶｳﾝﾄ表示ｸﾘｱ
	}
	if( s_no != 0 ){
		// 個別動作ｶｳﾝﾄｸﾘｱ
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[76] );	// "　装置　　　の動作カウントを　"

		if( kind == FCNT_ROCK ){
			// ﾛｯｸ装置の場合、表示する装置No.を変換（101～150 → 1～50）
			opedsp( 2, 6, (ushort)(s_no-BIKE_START_INDEX), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// 装置No.表示
			s_no_wk = s_no-BIKE_START_INDEX-1;											// s_no_wk←動作ｶｳﾝﾄﾃﾞｰﾀ配列No.(0～49)
		}
		else{
			if( ctype == _MTYPE_INT_FLAP )
				opedsp( 2, 6, (ushort)(s_no-INT_CAR_START_INDEX), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 装置No.表示
			else
			opedsp( 2, 6, s_no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 装置No.表示
			s_no_wk = s_no-1;												// s_no_wk←動作ｶｳﾝﾄﾃﾞｰﾀ配列No.(0～49)
		}
	}
	else{
		// 全動作ｶｳﾝﾄｸﾘｱ
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[77] );	// "　全装置の動作カウントを　　　"
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[78] );		// "　クリアしますか？　　　　　　"

	Fun_Dsp( FUNMSG[19] );					// "　　　　　　 はい いいえ　　　"

	for( ret = 0 ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// キー入力待ち

		switch( msg ){						// キー入力により各処理へ分岐
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ｷｰ切替
				ret = MOD_CHG;
				break;

			case KEY_TEN_F3:				// F3:"はい"
				BUZPI();
				ret = MOD_EXT;
				MovCntClr = ON;				// 動作ｶｳﾝﾄｸﾘｱ実行状態に有ｾｯﾄ（操作ﾛｸﾞ登録用）

				if( kind == FCNT_FLAP ){
					// 装置種別＝フラップ
					if( s_no != 0 ){
						// 個別動作ｶｳﾝﾄｸﾘｱ
						MntFlapDoCount[s_no_wk][0] = 0;							// 動作回数ｸﾘｱ
						MntFlapDoCount[s_no_wk][1] = 0;							// 手動動作回数ｸﾘｱ
						MntFlapDoCount[s_no_wk][2] = 0;							// 故障回数ｸﾘｱ
						queset( FLPTCBNO, LK_SND_CNT, sizeof(s_no), &s_no );	// 動作ｶｳﾝﾀｰｸﾘｱ（指定）要求送信
					}
					else{
						// 全動作ｶｳﾝﾄｸﾘｱ
						if( ctype & _MTYPE_INT_FLAP ){
							search_max = INT_FLAP_END_NO;
							if( ctype == _MTYPE_INT_FLAP )
								s_no_wk = INT_CAR_START_INDEX;
							else
								s_no_wk = 0;
						}else{
							search_max = CAR_LOCK_MAX;
							s_no_wk = 0;
						}
						for( ; s_no_wk<search_max ; s_no_wk++ ){	// ﾌﾗｯﾌﾟ装置数分ﾙｰﾌﾟ
							if( MntFlapDoCount[s_no_wk][0] != 0xffffffff ){
								// 接続されているﾌﾗｯﾌﾟ装置の動作ｶｳﾝﾄﾊﾞｯﾌｧを0ｸﾘｱする
								MntFlapDoCount[s_no_wk][0] = 0;					// 動作回数ｸﾘｱ
								MntFlapDoCount[s_no_wk][1] = 0;					// 手動動作回数ｸﾘｱ
								MntFlapDoCount[s_no_wk][2] = 0;					// 故障回数ｸﾘｱ
							}
						}
						type = (char)GetCarInfoParam();
						switch( type&0x06 ){
							case	0x04:
								type = _MTYPE_INT_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ﾒﾝﾃﾅﾝｽ情報要求送信
								break;
							
							case	0x06:
								type = _MTYPE_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ﾒﾝﾃﾅﾝｽ情報要求送信
								type = _MTYPE_INT_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ﾒﾝﾃﾅﾝｽ情報要求送信
								break;
							case	0x02:
							default:
								type = _MTYPE_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ﾒﾝﾃﾅﾝｽ情報要求送信
								break;
						}
					}
				}
				else if( kind == FCNT_ROCK ){
					// 装置種別＝ロック装置
					if( s_no != 0 ){
						// 個別動作ｶｳﾝﾄｸﾘｱ
						MntLockDoCount[s_no_wk][0] = 0;							// 動作回数ｸﾘｱ
						MntLockDoCount[s_no_wk][1] = 0;							// 手動動作回数ｸﾘｱ
						MntLockDoCount[s_no_wk][2] = 0;							// 故障回数ｸﾘｱ
						queset( FLPTCBNO, LK_SND_CNT, sizeof(s_no), &s_no );	// 動作ｶｳﾝﾀｰｸﾘｱ（指定）要求送信
					}
					else{
						// 全動作ｶｳﾝﾄｸﾘｱ
						for( s_no_wk=0 ; s_no_wk<BIKE_LOCK_MAX ; s_no_wk++ ){	// ﾛｯｸ装置数分ﾙｰﾌﾟ
							WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
							if( MntLockDoCount[s_no_wk][0] != 0xffffffff ){
								// 接続されているﾛｯｸ装置の動作ｶｳﾝﾄﾊﾞｯﾌｧを0ｸﾘｱする
								MntLockDoCount[s_no_wk][0] = 0;					// 動作回数ｸﾘｱ
								MntLockDoCount[s_no_wk][1] = 0;					// 手動動作回数ｸﾘｱ
								MntLockDoCount[s_no_wk][2] = 0;					// 故障回数ｸﾘｱ
							}
						}
						type = _MTYPE_LOCK;
						queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );		// 動作ｶｳﾝﾀｰｸﾘｱ（全て）要求送信
					}
				}
				break;

			case KEY_TEN_F4:				// F4:"いいえ"
				BUZPI();
				ret = MOD_EXT;
				break;
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		各種ﾛｸﾞ情報（履歴）印字操作処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMnt_Log( void )
{
	ushort	usSysEvent;
	char	wk[2];
	ushort	NewOldDate[6];		// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
	ushort	LogCount;			// LOG登録件数
	date_time_rec	NewestDateTime, OldestDateTime;


	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[48] );			// [48]	"＜ログファイルプリント＞　　　"

// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)
// 		usSysEvent = Menu_Slt( LOGMENU, LOG_FILE_TBL, (char)LOG_MENU_MAX, (char)1 );
		if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
			usSysEvent = Menu_Slt(LOGMENU_CC, LOG_FILE_TBL_CC, (char)LOG_MENU_MAX_CC, (char)1);
		} else {					// 通常料金計算モード
			usSysEvent = Menu_Slt(LOGMENU, LOG_FILE_TBL, (char)LOG_MENU_MAX, (char)1);
		}
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応(メンテナンス非表示)

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usSysEvent ){

			case	ERR_LOG:	// １：エラー情報
				Ope2_Log_NewestOldestDateGet( eLOG_ERROR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_ERROR,
													LogCount,
													LOGSTR1[0],
													PREQ_ERR_LOG,
													&NewOldDate[0]
												);
				break;

			case	ARM_LOG:	// ２：アラーム情報
				Ope2_Log_NewestOldestDateGet( eLOG_ALARM, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_ALARM,
													LogCount,
													LOGSTR1[1],
													PREQ_ARM_LOG,
													&NewOldDate[0]
												);
				break;

			case	TGOU_LOG:	// ３：Ｔ合計情報
				Ope2_Log_NewestOldestDateGet( eLOG_TTOTAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_TTOTAL,
													LogCount,
													LOGSTR1[2],
													PREQ_TGOUKEI_J,
													&NewOldDate[0]
												);
				break;

			case	GTGOU_LOG:	// ４：ＧＴ合計情報
				Ope2_Log_NewestOldestDateGet( eLOG_GTTOTAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_GTTOTAL,
													LogCount,
													LOGSTR1[15],
													PREQ_GTGOUKEI_J,
													&NewOldDate[0]
												);
				break;

			case	COIN_LOG:	// ４：コイン金庫情報

			 	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_COINBOX, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_COINBOX,
													LogCount,
													LOGSTR1[3],
													PREQ_COKINKO_J,
													&NewOldDate[0]
												);
				break;

			case	NOTE_LOG:	// ５：紙幣金庫情報

			 	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_NOTEBOX, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_NOTEBOX,
													LogCount,
													LOGSTR1[4],
													PREQ_SIKINKO_J,
													&NewOldDate[0]
												);
				break;

			case	TURI_LOG:	// ６：釣銭管理情報
			 	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_MNYMNG_SRAM, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_MNYMNG_SRAM,
													LogCount,
													LOGSTR1[5],
													PREQ_TURIKAN_LOG,
													&NewOldDate[0]
												);
				break;

			case	SEISAN_LOG:	// ７：個別精算情報

			 	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_PAYMENT, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_PAYMENT,
													LogCount,
													LOGSTR1[6],
													PREQ_KOBETUSEISAN,
													&NewOldDate[0]
												);
				break;
//未使用です（S）
			case	FUSKYO_LOG:	// ８：不正・強制出庫情報

			 	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_ABNORMAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );


				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_ABNORMAL,
													LogCount,
													LOGSTR1[7],
													PREQ_FUSKYO_LOG,
													&NewOldDate[0]
												);
				break;
//未使用です（E）
			case	OPE_LOG:	// １０：操作情報
				Ope2_Log_NewestOldestDateGet( eLOG_OPERATE, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_OPERATE,
													LogCount,
													LOGSTR1[9],
													PREQ_OPE_LOG,
													&NewOldDate[0]
												);
				break;

			case	TEIFUK_LOG:	// １１：停復電情報
				Ope2_Log_NewestOldestDateGet( eLOG_POWERON, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_POWERON,
													LogCount,
													LOGSTR1[10],
													PREQ_TEIFUK_LOG,
													&NewOldDate[0]
												);
				break;

			case	MONI_LOG:	// １２：モニタ情報
				Ope2_Log_NewestOldestDateGet( eLOG_MONITOR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_MONITOR,
													LogCount,
													LOGSTR1[11],
													PREQ_MON_LOG,
													&NewOldDate[0]
												);
				break;

			case	ADDPRM_LOG:	// １３：設定更新情報
				usSysEvent = SysMnt_Log_CntDsp1	(	//	ログ件数表示画面処理１（対象期間指定なし）
													ADDPRM_LOG,
													(ushort)SetDiffLogBuff.Header.UseInfoCnt,
													LOGSTR1[12],
													SETDIFFLOG_PRINT
												);
				break;
			case	RTPAY_LOG:	// １４：料金テスト情報
				usSysEvent = SysMnt_Log_CntDsp1	(	//	ログ件数表示画面処理１（対象期間指定なし）
													LOG_RTPAYMENT,
													(ushort)RT_PAY_LOG_DAT.RT_pay_count,
													LOGSTR1[14],
													PREQ_RT_PAYMENT_LOG
												);
				break;
			case	RMON_LOG:	// １５：遠隔監視情報
				Ope2_Log_NewestOldestDateGet( eLOG_REMOTE_MONITOR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
													LOG_REMOTE_MONITOR,
													LogCount,
													LOGSTR1[16],
													PREQ_RMON_LOG,
													&NewOldDate[0]
												);
				break;
			case	CLEAR_LOG:	// １６：全ログファイルクリア

				usSysEvent = SysMnt_LogClr();		// ログファイルクリア処理
				break;

			//Return Status of User Menu
			case MOD_EXT:
				//OPECTL.Mnt_mod = 1;

				return(usSysEvent);
				break;

			default:
				break;
		}
		//Return Status of User Operation
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usSysEvent == MOD_CHG ){
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			return(usSysEvent);
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ログ件数表示画面処理１（印字対象期間指定なし）						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp1( LogSyu,LogCnt,*Title,PreqCmd )	|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	:	ログ種別						|*/
/*|																			|*/
/*|						ushort	LogCnt	:	ログ件数						|*/
/*|																			|*/
/*|						ushort	*Title	:	タイトル表示データポインタ		|*/
/*|																			|*/
/*|						ushort	PreqCmd	:	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMnt_Log_CntDsp1( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd )
{
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	ushort			pri_cmd = 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	T_FrmLogPriReq1	FrmLogPriReq1;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	uchar			end_flg;			// クリア操作画面終了ﾌﾗｸﾞ


	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );			// ログ情報プリントタイトル表示
	LogCntDsp( LogCnt );													// ログ件数表示
	Fun_Dsp( FUNMSG[85] );													// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		if( pri_cmd == 0 ){

			// 印字要求前（印字要求前画面）
			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替

					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				case KEY_TEN_F2:							// Ｆ２（クリア）キー押下

					BUZPI();
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);		// "　　ログをクリアしますか？　　"
					Fun_Dsp(FUNMSG[19]);													// "　　　　　　 はい いいえ　　　"

					for( end_flg = OFF ; end_flg == OFF ; ){
						RcvMsg = StoF( GetMessage(), 1 );			// イベント待ち

						switch( RcvMsg ){							// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
							case LCD_DISCONNECT:
								RcvMsg = MOD_CUT;					// 前画面に戻る
								end_flg = ON;

								break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
							case KEY_MODECHG:						// 設定キー切替

								RcvMsg = MOD_CHG;					// 前画面に戻る
								end_flg = ON;

								break;

							case KEY_TEN_F3:						// Ｆ３（はい）キー押下

								BUZPI();
								if( LogDataClr( LogSyu ) == OK ){	// ログデータクリア
									LogCnt = 0;						// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
									LogCntDsp( LogCnt );			// ログ件数表示（０件）
								}
								end_flg = ON;

								break;

							case KEY_TEN_F4:						// Ｆ４（いいえ）キー押下

								BUZPI();
								end_flg = ON;

								break;
						}
						if( end_flg != OFF ){
							displclr( 6 );							// "　　ログをクリアしますか？　　"表示行クリア
							Fun_Dsp( FUNMSG[85] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
						}
					}

					break;

				case KEY_TEN_F4:							// Ｆ４（実行）キー押下
					if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
					FrmLogPriReq1.prn_kind = R_PRI;											// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					FrmLogPriReq1.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// 機械№	：設定ﾃﾞｰﾀ
					FrmLogPriReq1.Kakari_no = OPECTL.Kakari_Num;							// 係員No.
					memcpy( &FrmLogPriReq1.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// 印字時刻	：現在時刻
					queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq1), &FrmLogPriReq1 );

					pri_cmd = PreqCmd;						// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ

					Fun_Dsp( FUNMSG[82] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"

					break;

				case KEY_TEN_F5:							// Ｆ５（終了）キー押下

					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
				RcvMsg = MOD_EXT;							// YES：前画面に戻る
			}

			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					RcvMsg = MOD_CHG;						// 前画面に戻る
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();

					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// 前画面に戻る
		}
	}
	return( RcvMsg );
}

//[]----------------------------------------------------------------------[]
///	@brief		Log種別をeLog種別に変換する
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: Log種別
///	@return		eLogSyu		: eLog種別
//[]----------------------------------------------------------------------[]
///	@author		m.nagashima
///	@date		Create	: 2012/03/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Conv_to_eLogSyu( ushort LogSyu )
{
	ushort	eLogSyu = 0xffff;

	switch( LogSyu ){
		case LOG_PAYMENT:
			eLogSyu = eLOG_PAYMENT;			// 精算
			break;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(RT精算データ対応)
//		case LOG_ENTER:
//			eLogSyu = eLOG_ENTER;			// 入庫
//			break;
		case LOG_RTPAY:
			eLogSyu = eLOG_RTPAY;			// RT精算データ
			break;
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(RT精算データ対応)
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		case LOG_RTRECEIPT:
			eLogSyu = eLOG_RTRECEIPT;		// RT領収証データ
			break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		case LOG_TTOTAL:
			eLogSyu = eLOG_TTOTAL;			// 集計
			break;
		case LOG_ERROR:
			eLogSyu = eLOG_ERROR;			// エラー
			break;
		case LOG_ALARM:
			eLogSyu = eLOG_ALARM;			// アラーム
			break;
		case LOG_OPERATE:
			eLogSyu = eLOG_OPERATE;			// 操作
			break;
		case LOG_MONITOR:
			eLogSyu = eLOG_MONITOR;			// モニタ
			break;
		case LOG_ABNORMAL:
			eLogSyu = eLOG_ABNORMAL;		// 不正強制出庫
			break;
		case LOG_MONEYMANAGE:
			eLogSyu = eLOG_MONEYMANAGE;		// 釣銭管理
			break;
		case LOG_PARKING:
			eLogSyu = eLOG_PARKING;			// 駐車台数データ
			break;
		case LOG_NGLOG:
			eLogSyu = eLOG_NGLOG;			// 不正情報
			break;
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(QR確定・取消データ対応)
//		case LOG_IOLOG:
//			eLogSyu = eLOG_IOLOG;			// 入出庫履歴
		case LOG_DC_QR:
			eLogSyu = eLOG_DC_QR;			// QR確定・取消データ
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(QR確定・取消データ対応)
			break;
		case LOG_CREUSE:
			eLogSyu = eLOG_CREUSE;			// クレジット利用
			break;
		case LOG_HOJIN_USE:
			eLogSyu = eLOG_HOJIN_USE;		// 法人カード利用
			break;
		case LOG_REMOTE_SET:
			eLogSyu = eLOG_REMOTE_SET;		// 遠隔料金設定
			break;
		case LOG_LONGPARK:
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//			eLogSyu = eLOG_LONGPARK;		// 長期駐車
			eLogSyu = eLOG_LONGPARK_PWEB;	// 長期駐車(ParkingWeb用)
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
			break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		case LOG_RISMEVENT:
//			eLogSyu = eLOG_RISMEVENT;		// RISMイベント
		case LOG_DC_LANE:
			eLogSyu = eLOG_DC_LANE;			// レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			break;
		case LOG_GTTOTAL:
			eLogSyu = eLOG_GTTOTAL;			// GT集計
			break;
		case LOG_COINBOX:
			eLogSyu = eLOG_COINBOX;			// コイン金庫集計(ramのみ)
			break;
		case LOG_NOTEBOX:
			eLogSyu = eLOG_NOTEBOX;			// 紙幣金庫集計(ramのみ)
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case LOG_EDYARM:
//			eLogSyu = eLOG_EDYARM;			// Edyアラーム(ramのみ)
//			break;
//		case LOG_EDYSHIME:
//			eLogSyu = eLOG_EDYSHIME;		// Edy締め(ramのみ)
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case LOG_POWERON:
			eLogSyu = eLOG_POWERON;			// 復電(ramのみ)
			break;
		case LOG_MNYMNG_SRAM:
			eLogSyu = eLOG_MNYMNG_SRAM;		// 復電(ramのみ)
			break;
		case LOG_REMOTE_MONITOR:
			eLogSyu = eLOG_REMOTE_MONITOR;	// 遠隔監視
			break;
		default :
			break;
	}

	return eLogSyu;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ログ件数表示画面処理２（印字対象期間指定あり）						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp2( LogCnt, *Title, PreqCmd )		|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu		:	ログ種別					|*/
/*|																			|*/
/*|						ushort	LogCnt		:	ログ件数					|*/
/*|																			|*/
/*|						ushort	*Title		:	タイトル表示データポインタ	|*/
/*|																			|*/
/*|						ushort	PreqCmd		:	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ			|*/
/*|																			|*/
/*|						ushort	*NewOldDate	:	最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ	|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ１（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA1[6][4] = {
	{ 4, 12, 2 ,1 },	// 開始（年）
	{ 4, 18, 2 ,0 },	// 開始（月）
	{ 4, 24, 2, 0 },	// 開始（日）
	{ 5, 12, 2, 1 },	// 終了（年）
	{ 5, 18, 2, 0 }, 	// 終了（月）
	{ 5, 24, 2, 0 }		// 終了（日）
};
const	ushort	POS_DATA1_0[3][4] = {
	{ 5, 12, 2, 1 },	// 印刷（年）
	{ 5, 18, 2, 0 },	// 印刷（月）
	{ 5, 24, 2, 0 },	// 印刷（日）
};

ushort	SysMnt_Log_CntDsp2( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate )
{
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	ushort			pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	T_FrmLogPriReq2	FrmLogPriReq2;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			Date_Now[6];		// 日付ﾃﾞｰﾀ（[0]：開始年、[1]：開始月、[2]：開始日、[3]：終了年、[4]：終了月、[5]：終了日）
	uchar			pos		= 0;		// 期間ﾃﾞｰﾀ入力位置（０：開始年、１：開始月、２：開始日、３：終了年、４：終了月、５：終了日）
	short			inp		= -1;		// 入力ﾃﾞｰﾀ
	uchar			All_Req	= OFF;		// 「全て」指定中ﾌﾗｸﾞ
	uchar			Date_Chk;			// 日付指定ﾃﾞｰﾀﾁｪｯｸﾌﾗｸﾞ
	uchar			end_flg;			// クリア操作画面終了ﾌﾗｸﾞ
	ushort			wks;
	ushort			Date_Now_Prev[6];	// 前回までの年月日
	ushort			eLogSyu;			// ログ種別
	ushort			index;				// ログ検索位置

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );							// ログ情報プリントタイトル表示
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// 最古／最新ログ日付表示
	LogCntDsp( LogCnt );									// ログ件数表示

	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );
	memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

	LogDateDsp4( &Date_Now[0] );							// 最古／最新ログ日付データ表示
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
	LogDateDsp5( &Date_Now[0], pos );						// 印刷日付データ表示

	Fun_Dsp( FUNMSG[83] );									// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "

	eLogSyu = Conv_to_eLogSyu( LogSyu );					// Log種別→eLog種別変換
	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		if( pri_cmd == 0 ){

			// 印字要求前（印字要求前画面）

			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替

					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				case KEY_TEN:								// ０～９

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){					// 「全て」指定中？（指定中は、無視する）

						//	「全て」指定中でない場合

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// 入力ﾃﾞｰﾀ表示
									POS_DATA1_0[pos][0],	// 表示行
									POS_DATA1_0[pos][1],	// 表示ｶﾗﾑ
									(ushort)inp,			// 表示ﾃﾞｰﾀ
									POS_DATA1_0[pos][2],	// ﾃﾞｰﾀ桁数
									POS_DATA1_0[pos][3],	// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// 取消キー

					BUZPI();

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){					// 「全て」指定中？

						//	「全て」指定中でない場合

						opedsp	(							// 入力前のﾃﾞｰﾀを表示させる
									POS_DATA1_0[pos][0],	// 表示行
									POS_DATA1_0[pos][1],	// 表示ｶﾗﾑ
									Date_Now[pos],			// 表示ﾃﾞｰﾀ
									POS_DATA1_0[pos][2],	// ﾃﾞｰﾀ桁数
									POS_DATA1_0[pos][3],	// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// 入力状態初期化
					}
					else{
						//	「全て」指定中の場合

						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
						inp = -1;							// 入力状態初期化
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
						LogDateDsp5( &Date_Now[0], pos );						// 印刷日付データ表示
						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
					}

					break;

				case KEY_TEN_F1:							// Ｆ１（←）キー押下
				case KEY_TEN_F2:							// Ｆ２（→）キー押下	※日付入力中の場合
															// Ｆ２（クリア）押下	※「全て」指定中の場合

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){						// 「全て」指定中？

						//	「全て」指定中でない場合

						if( inp == -1 ){						// 入力あり？

							//	入力なしの場合
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
							}
							else{
								LogDatePosUp( &pos, 1 );		// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
							}
						}
						else{
							//	入力ありの場合
							if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ？

								//	入力ﾃﾞｰﾀＯＫの場合
								BUZPI();

								if( pos == 0 ){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
								}
								else{
									LogDatePosUp( &pos, 1 );	// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
								}
							}
							else{
								//	入力ﾃﾞｰﾀＮＧの場合
								BUZPIPI();
							}
						}
						/* 指定年月日範囲内のﾃﾞｰﾀ数再表示 */
						LogDateDsp5( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
						inp = -1;								// 入力状態初期化
					}
					else if( RcvMsg == KEY_TEN_F2 ){

						// 「全て」指定中のＦ２（クリア）キー

						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "　　ログをクリアしますか？　　"
						Fun_Dsp(FUNMSG[19]);							// "　　　　　　 はい いいえ　　　"

						for( end_flg = OFF ; end_flg == OFF ; ){
							RcvMsg = StoF( GetMessage(), 1 );			// イベント待ち

							switch( RcvMsg ){							// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
								case LCD_DISCONNECT:					// 切断通知
									RcvMsg = MOD_CUT;					// 前画面に戻る
									end_flg = ON;

									break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
								case KEY_MODECHG:						// 設定キー切替

									RcvMsg = MOD_CHG;					// 前画面に戻る
									end_flg = ON;

									break;

								case KEY_TEN_F3:						// Ｆ３（はい）キー押下

									BUZPI();
									if( LogDataClr( LogSyu ) == OK ){	// ログデータクリア
										LogCnt = 0;						// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
										/* データなし時の範囲日付は今日にする */
										NewOldDate[0] = NewOldDate[3] = (ushort)CLK_REC.year;		// 開始、終了（年）←現在日時（年）
										NewOldDate[1] = NewOldDate[4] = (ushort)CLK_REC.mont;		// 開始、終了（月）←現在日時（月）
										NewOldDate[2] = NewOldDate[5] = (ushort)CLK_REC.date;		// 開始、終了（日）←現在日時（日）
										LogCntDsp( LogCnt );			// ログ件数表示（０件）
										switch(LogSyu)					// クリアログ登録処理
										{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//										case LOG_EDYSYUUKEI:			// Ｅｄｙ集計クリア
//											wopelg(OPLOG_EDYSYU_CLR,0,0);
//											break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
										case LOG_SCASYUUKEI:			// Ｓｕｉｃａ集計クリア
// MH321800(S) D.Inaba ICクレジット対応
										case LOG_ECSYUUKEI:			// 決済リーダ集計クリア
// MH321800(E) D.Inaba ICクレジット対応
											wopelg(OPLOG_SCASYU_CLR,0,0);
											break;
										default:						// 上記以外(登録対象ではない)
											break;
										}
									}
									end_flg = ON;

									break;

								case KEY_TEN_F4:						// Ｆ４（いいえ）キー押下

									BUZPI();
									end_flg = ON;

									break;
							}
							if( end_flg != OFF ){
								displclr( 6 );										// "　　ログをクリアしますか？　　"表示行クリア
								memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );	// 日付表示範囲はMaxとする
								LogCntDsp( LogCnt );								// ログ件数表示
								LogDateDsp4( &Date_Now[0] );						// 最古／最新ログ日付データ表示
								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
								LogDateDsp5( &Date_Now[0], pos );					// 印刷日付データ表示
								Fun_Dsp( FUNMSG[83] );								// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
								All_Req = OFF;										// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
							}
						}
					}

					break;

				case KEY_TEN_F3:							// Ｆ３（全て）キー押下

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){						// 「全て」指定中？（指定中は、無視する）

						if( LogCnt != 0 ){

							//	ログデータがある場合
							BUZPI();
							memcpy	(							// 最古＆最新ﾃﾞｰﾀ日付をｺﾋﾟｰ
										&Date_Now[0],
										NewOldDate,
										12
									);
							pos = 0;							// ｶｰｿﾙ位置＝開始（年）
							inp = -1;							// 入力状態初期化
							displclr( 5 );						// 印刷日付表示クリア
							LogCntDsp( LogCnt );				// ログ件数表示
							Fun_Dsp( FUNMSG[85] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
							All_Req = ON;						// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
						}
						else{
							//	ログデータがない場合
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// Ｆ４（実行）キー押下

					displclr( 1 );							// 1行目表示クリア
					Date_Chk = OK;

					if( inp != -1 ){						// 入力あり？

						//	入力ありの場合
						if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ？

							//	入力データＯＫの場合
							if( pos == 0 ){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}
						else{
							//	入力データＮＧの場合
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// 「全て」指定でない場合、年月日ﾁｪｯｸを行う

							//	年月日が存在する日付かチェックする

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// 開始日付ﾁｪｯｸ
								Date_Chk = NG;
							}
						}
					}

					if( Date_Chk == OK ){

						// 開始日付＆終了日付ﾁｪｯｸＯＫの場合
						wks = Ope2_Log_CountGet_inDate( eLogSyu, &Date_Now[0], &index );
						if( wks == 0 ){
							BUZPIPI();
							grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// データなし表示
							inp = -1;											// 入力状態初期化
							break;
						}
						if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
							BUZPIPI();
							break;
						}
						BUZPI();

						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
						FrmLogPriReq2.prn_kind	= SysMnt_Log_Printer(LogSyu);	// 種類毎に対象ﾌﾟﾘﾝﾀを判断
						if(!FrmLogPriReq2.prn_kind)								// 取得したプリンタ種別が0
						{
							RcvMsg = MOD_EXT;									// 前画面に戻る
							break;
						}
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// 検索方法	：全て
						}
						else{
							FrmLogPriReq2.BType	= 1;							// 検索方法	：日付
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						FrmLogPriReq2.LogCount = LogCnt;	// LOG登録件数(個別精算LOG・集計LOGで使用)

						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
						Ope_DisableDoorKnobChime();

						pri_cmd = PreqCmd;					// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
						if( All_Req != ON ){					// 「全て」指定なし
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
							LogDateDsp5( &Date_Now[0], 0xff );	// 印刷日付データ表示
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// 実行中ブリンク表示
						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( LogSyu == LOG_EDYARM )
//							wopelg( OPLOG_EDY_ARM_PRN,0,0 );	// 操作履歴登録（Edy取引ｱﾗｰﾑ情報ﾌﾟﾘﾝﾄ）
//						else if( LogSyu == LOG_EDYSHIME )
//							wopelg( OPLOG_EDY_SHIME_PRN,0,0 );	// 操作履歴登録（Edy締め記録情報ﾌﾟﾘﾝﾄ）
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

					}
					else{
						// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
						BUZPIPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
						LogDateDsp5( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
						Fun_Dsp( FUNMSG[83] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
						All_Req = OFF;							// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
						inp = -1;								// 入力状態初期化
					}

					break;

				case KEY_TEN_F5:							// Ｆ５（終了）キー押下

					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
				RcvMsg = MOD_EXT;							// YES：前画面に戻る
			}

			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:						// 切断通知
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)s
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					RcvMsg = MOD_CHG;						// 前画面に戻る
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();

					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
			break;											// 前画面に戻る
		}
	}
	return( RcvMsg );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ件数表示画面処理（Suica日毎集計印字用）							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp2( LogCnt, *Title, PreqCmd )		|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu		:	ログ種別					|*/
/*|																			|*/
/*|						ushort	LogCnt		:	ログ件数					|*/
/*|																			|*/
/*|						ushort	*Title		:	タイトル表示データポインタ	|*/
/*|																			|*/
/*|						ushort	PreqCmd		:	印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ			|*/
/*|																			|*/
/*|						ushort	*NewOldDate	:	最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ	|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	DiditalCashe_Log_CntDsp( ushort LogSyu, ushort LogCnt, uchar *title, ushort PreqCmd, ushort *NewOldDate )
{
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	ushort			pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	T_FrmLogPriReq2	FrmLogPriReq2;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			Date_Now[6];		// 日付ﾃﾞｰﾀ（[0]：開始年、[1]：開始月、[2]：開始日、[3]：終了年、[4]：終了月、[5]：終了日）
	uchar			pos		= 0;		// 期間ﾃﾞｰﾀ入力位置（０：開始年、１：開始月、２：開始日、３：終了年、４：終了月、５：終了日）
	short			inp		= -1;		// 入力ﾃﾞｰﾀ
	uchar			All_Req	= OFF;		// 「全て」指定中ﾌﾗｸﾞ
	uchar			Date_Chk;			// 日付指定ﾃﾞｰﾀﾁｪｯｸﾌﾗｸﾞ
	ushort			Sdate;				// 開始日付
	ushort			Edate;				// 終了日付
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
	ushort			LogCntNow;			// 現在のログ数
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
	uchar			end_flg;			// クリア操作画面終了ﾌﾗｸﾞ
	ushort			Date_Now_Prev[6];	// 前回までの年月日
	uchar			f_line_chg = 0;		// カーソル移動時の行移動判定


	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,title );							// ログ情報プリントタイトル表示
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "　　保有ログ件数：　　　　件　"
	opedsp( 2, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,LOGSTR2[1] );						// 開始日付表示
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,LOGSTR2[2] );						// 終了日付表示

	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );
	memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
	LogCntNow = LogCnt;
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）

	LogDateDsp( &Date_Now[0], pos );						// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示

	Fun_Dsp( FUNMSG[83] );									// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		if( pri_cmd == 0 ){

			// 印字要求前（印字要求前画面）

			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:						// 切断通知
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替

					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				case KEY_TEN:								// ０～９

					if( All_Req == OFF ){					// 「全て」指定中？（指定中は、無視する）

						//	「全て」指定中でない場合

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// 入力ﾃﾞｰﾀ表示
									POS_DATA1[pos][0],		// 表示行
									POS_DATA1[pos][1],		// 表示ｶﾗﾑ
									(ushort)inp,			// 表示ﾃﾞｰﾀ
									POS_DATA1[pos][2],		// ﾃﾞｰﾀ桁数
									POS_DATA1[pos][3],		// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// 取消キー

					BUZPI();

					if( All_Req == OFF ){					// 「全て」指定中？

						//	「全て」指定中でない場合

						opedsp	(							// 入力前のﾃﾞｰﾀを表示させる
									POS_DATA1[pos][0],		// 表示行
									POS_DATA1[pos][1],		// 表示ｶﾗﾑ
									Date_Now[pos],			// 表示ﾃﾞｰﾀ
									POS_DATA1[pos][2],		// ﾃﾞｰﾀ桁数
									POS_DATA1[pos][3],		// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// 入力状態初期化
					}
					else{
						//	「全て」指定中の場合

						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
						inp = -1;							// 入力状態初期化
						opedsp	(							// ｶｰｿﾙを表示させる
									POS_DATA1[pos][0],		// 表示行
									POS_DATA1[pos][1],		// 表示ｶﾗﾑ
									Date_Now[pos],			// 表示ﾃﾞｰﾀ
									POS_DATA1[pos][2],		// ﾃﾞｰﾀ桁数
									POS_DATA1[pos][3],		// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
					}

					break;

				case KEY_TEN_F1:							// Ｆ１（←）キー押下
				case KEY_TEN_F2:							// Ｆ２（→）キー押下	※日付入力中の場合
															// Ｆ２（クリア）押下	※「全て」指定中の場合

					if( All_Req == OFF ){						// 「全て」指定中？

						//	「全て」指定中でない場合

						if( inp == -1 ){						// 入力あり？

							//	入力なしの場合
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								if( pos != 0 ){
									pos = (uchar)((pos) - 1);
								}
								else{
									pos = 5;
								}
								if((pos == 2) || (pos == 5))
								{
									f_line_chg = 1;				/* 行移動あり */
								}
							}
							else{
								pos = (uchar)((pos) + 1);
								if( pos >= 6 ){
									pos = 0;
								}
								if((pos == 0) || (pos == 3))
								{
									f_line_chg = 1;				/* 行移動なし */
								}
							}
						}
						else{
							//	入力ありの場合
							if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ？

								//	入力ﾃﾞｰﾀＯＫの場合
								BUZPI();

								if( pos == 0 || pos == 3 ){		// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
									Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] = (ushort)inp;
								}

								if( RcvMsg == KEY_TEN_F1 ){
									if( pos != 0 ){
										pos = (uchar)((pos) - 1);
									}
									else{
										pos = 5;
									}
									if((pos == 2) || (pos == 5))
									{
										f_line_chg = 1;				/* 行移動あり */
									}
								}
								else{
									pos = (uchar)((pos) + 1);
									if( pos >= 6 ){
										pos = 0;
									}
									if((pos == 0) || (pos == 3))
									{
										f_line_chg = 1;				/* 行移動なし */
									}
								}
							}
							else{
								//	入力ﾃﾞｰﾀＮＧの場合
								BUZPIPI();
							}
						}
						/* 指定年月日範囲内のﾃﾞｰﾀ数再表示 */
						if( (f_line_chg) &&						// 行移動発生　且つ
							(0 != memcmp( Date_Now_Prev, Date_Now, sizeof(Date_Now) ) ) )
						{										// 今までと年月日値が異なる場合
//							wks = Ope2_Log_CountGet_inDate( LogSyu, &Date_Now[3], &Date_Now[0] );
//							LogCntDsp( wks );		// ログ件数表示
							memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
							switch (LogSyu) {
							case LOG_ECSYUUKEI:
								Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
													(short)Date_Now[0],
													(short)Date_Now[1],
													(short)Date_Now[2]
												);

								Edate = dnrmlzm(							// 終了日付ﾃﾞｰﾀ取得
													(short)Date_Now[3],
													(short)Date_Now[4],
													(short)Date_Now[5]
												);
											
								LogCntNow = GetEcShuukeiCnt(Sdate, Edate);
								grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "　　保有ログ件数：　　　　件　"
								opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
								break;
							default:
								break;
							}
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
						}
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
						f_line_chg = 0;
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
						LogDateDsp( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
						inp = -1;								// 入力状態初期化
					}
					else if( RcvMsg == KEY_TEN_F2 ){

						// 「全て」指定中のＦ２（クリア）キー

						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK,LCD_BLINK_OFF, SMSTR1[60]);				// "　　ログをクリアしますか？　　"
						Fun_Dsp(FUNMSG[19]);							// "　　　　　　 はい いいえ　　　"

						for( end_flg = OFF ; end_flg == OFF ; ){
							RcvMsg = StoF( GetMessage(), 1 );			// イベント待ち

							switch( RcvMsg ){							// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
								case LCD_DISCONNECT:					// 切断通知
									RcvMsg = MOD_CUT;					// 前画面に戻る
									end_flg = ON;

									break;								
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
								case KEY_MODECHG:						// 設定キー切替

									RcvMsg = MOD_CHG;					// 前画面に戻る
									end_flg = ON;

									break;

								case KEY_TEN_F3:						// Ｆ３（はい）キー押下

									BUZPI();
									if( LogDataClr( LogSyu ) == OK ){	// ログデータクリア
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
//										LogCnt = 0;						// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
										LogCnt = LogCntNow = 0;			// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
										/* データなし時の範囲日付は今日にする */
										NewOldDate[0] = NewOldDate[3] = (ushort)CLK_REC.year;		// 開始、終了（年）←現在日時（年）
										NewOldDate[1] = NewOldDate[4] = (ushort)CLK_REC.mont;		// 開始、終了（月）←現在日時（月）
										NewOldDate[2] = NewOldDate[5] = (ushort)CLK_REC.date;		// 開始、終了（日）←現在日時（日）
										grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "　　保有ログ件数：　　　　件　"
										opedsp( 2, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
										switch(LogSyu)					// クリアログ登録処理
										{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//										case LOG_EDYSYUUKEI:			// Ｅｄｙ集計クリア
//											wopelg(OPLOG_EDYSYU_CLR,0,0);
//											break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
										case LOG_SCASYUUKEI:			// Ｓｕｉｃａ集計クリア
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
										case LOG_ECSYUUKEI:				// 決済リーダ集計クリア
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
											wopelg(OPLOG_SCASYU_CLR,0,0);
											break;
										default:						// 上記以外(登録対象ではない)
											break;
										}
									}
									end_flg = ON;

									break;

								case KEY_TEN_F4:						// Ｆ４（いいえ）キー押下

									BUZPI();
									end_flg = ON;

									break;
							}
							if( end_flg != OFF ){
								displclr( 6 );										// "　　ログをクリアしますか？　　"表示行クリア
								memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );	// 日付表示範囲はMaxとする
//								LogCntDsp( LogCnt );								// ログ件数表示
								LogDateDsp( &Date_Now[0], pos );					// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
								Fun_Dsp( FUNMSG[83] );								// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
								All_Req = OFF;										// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
							}
						}
					}

					break;

				case KEY_TEN_F3:							// Ｆ３（全て）キー押下

					if( All_Req == OFF ){						// 「全て」指定中？（指定中は、無視する）

						if( LogCnt != 0 ){

							//	ログデータがある場合
							BUZPI();
							memcpy	(							// 最古＆最新ﾃﾞｰﾀ日付をｺﾋﾟｰ
										&Date_Now[0],
										NewOldDate,
										12
									);
							pos = 0;							// ｶｰｿﾙ位置＝開始（年）
							inp = -1;							// 入力状態初期化
							LogDateDsp( &Date_Now[0], 0xff );	// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示（ｶｰｿﾙ表示なし）
//							LogCntDsp( LogCnt );				// ログ件数表示
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
							switch (LogSyu) {
							case LOG_ECSYUUKEI:
								LogCntNow = LogCnt;
								grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "　　保有ログ件数：　　　　件　"
								opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
								break;
							default:
								break;
							}
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
							Fun_Dsp( FUNMSG[85] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
							All_Req = ON;						// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
						}
						else{
							//	ログデータがない場合
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// Ｆ４（実行）キー押下

					Date_Chk = OK;

					if( inp != -1 ){						// 入力あり？

						//	入力ありの場合
						if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ？

							//	入力データＯＫの場合
							if( pos == 0 || pos == 3 ){		// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
								Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] = (ushort)inp;
							}
						}
						else{
							//	入力データＮＧの場合
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// 「全て」指定でない場合、年月日ﾁｪｯｸを行う

							//	年月日が存在する日付かチェックする

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// 開始日付ﾁｪｯｸ
								Date_Chk = NG;
							}
							if( chkdate( (short)Date_Now[3], (short)Date_Now[4], (short)Date_Now[5] ) != 0 ){	// 終了日付ﾁｪｯｸ
								Date_Chk = NG;
							}
						}
					}
					if( Date_Chk == OK ){

						//	開始日付＜＝終了日付かチェックする

						Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
											(short)Date_Now[0],
											(short)Date_Now[1],
											(short)Date_Now[2]
										);

						Edate = dnrmlzm(							// 終了日付ﾃﾞｰﾀ取得
											(short)Date_Now[3],
											(short)Date_Now[4],
											(short)Date_Now[5]
										);

						if( Sdate > Edate ){						// 開始日付／終了日付ﾁｪｯｸ
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						// 開始日付＆終了日付ﾁｪｯｸＯＫの場合

						// 入力あれば件数再表示
						if( inp != -1 ){						// 入力あり？
							if( 0 != memcmp( Date_Now_Prev, Date_Now, sizeof(Date_Now) ) )
							{									// 今までと年月日値が異なる場合
//								wks = Ope2_Log_CountGet_inDate( LogSyu, &Date_Now[3], &Date_Now[0] );
//								LogCntDsp( wks );		// ログ件数表示
								memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );
// MH321800(S) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
								switch (LogSyu) {
								case LOG_ECSYUUKEI:
									LogCntNow = GetEcShuukeiCnt(Sdate, Edate);
									grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );			// "　　保有ログ件数：　　　　件　"
									opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
									break;
								default:
									break;
								}
// MH321800(E) D.Inaba ICクレジット対応（マルチ電子マネーリーダ対応移植）
							}
						}
						if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
							BUZPIPI();
							break;
						}
						BUZPI();

						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
						FrmLogPriReq2.prn_kind	= SysMnt_Log_Printer(LogSyu);	// 種類毎に対象ﾌﾟﾘﾝﾀを判断
						if(!FrmLogPriReq2.prn_kind)								// 取得したプリンタ種別が0
						{
							RcvMsg = MOD_EXT;									// 前画面に戻る
							break;
						}
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// 検索方法	：全て
						}
						else if( Sdate == Edate ){
							FrmLogPriReq2.BType	= 1;							// 検索方法	：日付
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						else{
							FrmLogPriReq2.BType	= 2;							// 検索方法	：期間
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
							FrmLogPriReq2.TEndTime.Year	= Date_Now[3];
							FrmLogPriReq2.TEndTime.Mon	= (uchar)Date_Now[4];
							FrmLogPriReq2.TEndTime.Day	= (uchar)Date_Now[5];
						}
						FrmLogPriReq2.LogCount = LogCnt;	// LOG登録件数(個別精算LOG・集計LOGで使用)

						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );

						pri_cmd = PreqCmd;					// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
						LogDateDsp( &Date_Now[0], 0xff );	// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示（ｶｰｿﾙ表示を消す）
						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"

					}
					else{
						// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
						BUZPIPI();
						LogDateDsp( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
						inp = -1;								// 入力状態初期化
					}

					break;

				case KEY_TEN_F5:							// Ｆ５（終了）キー押下

					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
				RcvMsg = MOD_EXT;							// YES：前画面に戻る
			}

			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:						// 切断通知
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					RcvMsg = MOD_CHG;						// 前画面に戻る
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();

					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// 前画面に戻る
		}
	}
	return( RcvMsg );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログファイルクリア（全て）操作処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_LogClr( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//#define		CLR_LOG_CNT		13		// ログファイル種別数
#define		CLR_LOG_CNT		14		// ログファイル種別数
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)

const	ushort	LogSyuTbl[CLR_LOG_CNT] = {
	LOG_ERROR,		// エラー情報
	LOG_ALARM,		// アラーム情報
	LOG_OPERATE,	// 操作情報
	LOG_POWERON,	// 停復電情報
	LOG_TTOTAL,		// Ｔ合計情報
	LOG_COINBOX,	// コイン金庫情報
	LOG_NOTEBOX,	// 紙幣金庫情報
	LOG_MNYMNG_SRAM,// 釣銭管理情報(SRAM)
	LOG_PAYMENT,	// 個別精算情報
	LOG_ABNORMAL,	// 不正・強制出庫情報
	LOG_MONITOR,	// モニタ情報
	ADDPRM_LOG,		// 設定更新履歴
	LOG_RTPAYMENT,	// 料金テスト情報
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
	LOG_LONGPARK,	// 長期駐車データ(ParkingWeb用)
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
};

ushort	SysMnt_LogClr( void )
{
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	uchar			i;					// ﾛｸﾞﾌｧｲﾙｸﾘｱﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	uchar			Clr_Ope = OFF;		// ｸﾘｱ操作ﾌﾗｸﾞ

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[59] );						// "＜ログファイルクリア＞　　　　"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[61]);						// "　全てのログをクリアしますか？"
	Fun_Dsp(FUNMSG[19]);									// "　　　　　　 はい いいえ　　　"

	for( ; ; ){
		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		switch( RcvMsg ){									// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:						// 切断通知
				RcvMsg = MOD_CUT;						// 前画面に戻る
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	// 設定キー切替

				RcvMsg = MOD_CHG;							// 前画面に戻る
				break;

			case KEY_TEN_F3:	// Ｆ３（はい）キー押下

				if( Clr_Ope == OFF ){						// クリア操作前？

					BUZPI();

					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[62]);		// "　全てのログをクリア中です。　"
					Fun_Dsp(FUNMSG[0]);						// ﾌｧﾝｸｼｮﾝｷｰ表示クリア
					for(i = 0;i < eLOG_MAX; i++){
						FLT_LogErase2(i);
						Log_CheckBufferFull(FALSE, i, eLOG_TARGET_REMOTE);				// バッファフル解除チェック
					}
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//					memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );// 入出庫履歴のログもここでクリアする必要がある
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
					for( i = 0 ; i < CLR_LOG_CNT ; i++ ){	// ログファイル全クリア
						LogDataClr( LogSyuTbl[i] );
					}
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[63]);		// "　全てのログをクリアしました。"
					Fun_Dsp(FUNMSG[8]);						// "　　　　　　　　　　　　 終了 "

					Clr_Ope = ON;							// クリア操作済みセット
				}
				break;

			case KEY_TEN_F4:	// Ｆ４（いいえ）キー押下

				if( Clr_Ope == OFF ){						// クリア操作前？
					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る
				}
				break;

			case KEY_TEN_F5:	// Ｆ５（終了）キー押下

				if( Clr_Ope == ON ){						// クリア操作後？
					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る
				}
				break;

			default:
				break;
		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// 前画面に戻る
		}
	}
	return( RcvMsg );
}


/*[]-----------------------------------------------------------------------[]*/
/*|		ログ日付位置番号更新処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDatePosUp( uchar *pos, uchar req )				|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*pos= 位置番号ﾃﾞｰﾀﾎﾟｲﾝﾀ						|*/
/*|																			|*/
/*|						uchar	req	= 処理要求								|*/
/*|										０：位置番号－１（左移動）			|*/
/*|										１：位置番号＋１（右移動）			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	LogDatePosUp( uchar *pos, uchar req )
{
	uchar ret = 0;					/* 行移動判定(0:なし/1:あり) */
	if( req == 0 ){

		// 位置番号－１（左移動）
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = 2;
		}
	}
	else{
		// 位置番号＋１（右移動）
		*pos = (uchar)((*pos) + 1);
		if( *pos >= 3 ){
			*pos = 0;
		}
	}
	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ日付データチェック処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateChk( pos, data )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pos	= 日付位置番号ﾃﾞｰﾀ						|*/
/*|																			|*/
/*|						short	data= 日付ﾃﾞｰﾀ								|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret	= ﾁｪｯｸ結果(OK/NG)						|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// 項目毎のﾃﾞｰﾀ範囲ﾁｪｯｸﾃﾞｰﾀﾃｰﾌﾞﾙ（最小値、最大値）
const	short	LOG_DATE_DATA[6][2] = {
	{ 0, 99 },	// 開始（年）
	{ 1, 12 },	// 開始（月）
	{ 1, 31 },	// 開始（日）
	{ 0, 99 },	// 終了（年）
	{ 1, 12 },	// 終了（月）
	{ 1, 31 },	// 終了（日）
};

uchar	LogDateChk( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA[pos][0] && data <= LOG_DATE_DATA[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ件数表示処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp( LogCnt )									|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogCnt	:	ログ件数						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogCntDsp( ushort LogCnt )
{
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "　　保有ログ件数：　　　　件　"

	opedsp( 3, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 件数ﾃﾞｰﾀ表示（４桁、０サプレス）
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ログ件数表示処理２													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp2( LogCnt )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogCnt1	:	対象ログ件数					|*/
/*|					:	ushort	LogCnt2	:	総ログ件数						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	LogCntDsp2( ushort LogCnt1, ushort LogCnt2, ushort type)
{
	ushort	w_len1, w_len2;
	ushort	w_pos1, w_pos2, w_pos3, w_pos4;

	w_pos1 = 0;
	// 対象ログ件数の桁数
	if( LogCnt1 >= 1000 ) {
		w_len1 = 4;
	} else if (LogCnt1 >= 100 ) {
		w_len1 = 3;
	} else if (LogCnt1 >= 10 ) {
		w_len1 = 2;
	} else {
		w_len1 = 1;
	}

	// 総ログ件数の桁数
	if( LogCnt2 >= 1000 ) {
		w_len2 = 4;
	} else if (LogCnt2 >= 100 ) {
		w_len2 = 3;
	} else if (LogCnt2 >= 10 ) {
		w_len2 = 2;
	} else {
		w_len2 = 1;
	}

	// 表示位置を決定
	if( type == 3 ) {			// クレジット利用明細ログ
		w_pos1 = 10;			// 対象ログ件数
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[95] );
	}
	else if( type == 4) {	//入出庫ログ
		w_pos1 = 10;			// 対象ログ件数
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[95] );
	}
	w_pos2 = w_pos1 + w_len1*2;	// 「／」
	w_pos3 = w_pos2 + 2;		// 総ログ件数
	w_pos4 = w_pos3 + w_len2*2; // 「件」

	// 表示
	opedsp( 2, w_pos1, LogCnt1, w_len1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 対象ログ件数表示（０サプレス）
	grachr( 2, w_pos2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"／" );
	opedsp( 2, w_pos3, LogCnt2, w_len2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 総ログ件数表示（０サプレス）
	grachr( 2, w_pos4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"件" );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ日付位置番号更新処理２（不正券情報プリント用）					|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDatePosUp1( uchar *pos, uchar req )				|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*pos= 位置番号ﾃﾞｰﾀﾎﾟｲﾝﾀ	(0-9)				|*/
/*|																			|*/
/*|						uchar	req	= 処理要求								|*/
/*|										０：位置番号－１（左移動）			|*/
/*|										１：位置番号＋１（右移動）			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	行移動（0:なし／1:あり）					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
uchar	LogDatePosUp1( uchar *pos, uchar req )
{
	uchar	flg = 0;
 
	if( req == 0 ){		// 位置番号－１（左移動）
		if( (*pos == 0)||(*pos == 3) ) {
			flg = 1;
		}
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = 5;
		}
	}
	else{				// 位置番号＋１（右移動）		
		if( (*pos == 2)||(*pos == 5) ) {
			flg = 1;
		}
		*pos = (uchar)((*pos) + 1);
		if( *pos >= 6 ){
			*pos = 0;
		}
	}
	return(flg);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ印字開始／終了日付ﾃﾞｰﾀ表示処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	反転表示位置					|*/
/*|											０～５：指定位置反転表示		|*/
/*|											その他：反転表示なし			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ２（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA2[6][4] = {
	{ 4,  8, 4, 1 },	// 開始（年）
	{ 4, 18, 2, 0 },	// 開始（月）
	{ 4, 24, 2, 0 },	// 開始（日）
	{ 5,  8, 4, 1 },	// 終了（年）
	{ 5, 18, 2, 0 }, 	// 終了（月）
	{ 5, 24, 2, 0 }		// 終了（日）
};

void	LogDateDsp( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// 日付ﾃﾞｰﾀ表示位置
	ushort	rev_data;		// 反転表示ﾃﾞｰﾀ


	for( pos = 0 ; pos < 6 ; pos++){		// 日付ﾃﾞｰﾀ表示：開始（年）～終了（日）

		opedsp	(
					POS_DATA2[pos][0],		// 表示行
					POS_DATA2[pos][1],		// 表示ｶﾗﾑ
					*Date,					// 表示ﾃﾞｰﾀ
					POS_DATA2[pos][2],		// ﾃﾞｰﾀ桁数
					POS_DATA2[pos][3],		// ０サプレス有無
					0,						// 反転表示：なし
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;			// 反転表示ﾃﾞｰﾀｾｰﾌﾞ
		}

		Date++;								// 日付ﾃﾞｰﾀﾎﾟｲﾝﾀ更新
	}
	if( Rev_Pos <= 5 ){						// 反転表示指定あり？

		opedsp	(							// 指定位置を反転表示させる
					POS_DATA1[Rev_Pos][0],	// 表示行
					POS_DATA1[Rev_Pos][1],	// 表示ｶﾗﾑ
					rev_data,				// 表示ﾃﾞｰﾀ
					POS_DATA1[Rev_Pos][2],	// ﾃﾞｰﾀ桁数
					POS_DATA1[Rev_Pos][3],	// ０サプレス有無
					1,						// 反転表示：あり
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		最古／最新ログ日付データ表示処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp4( *Date )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	m.nagashima													|*/
/*|	Date	:	2012-03-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ４（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA4[6][4] = {
	{ 2,  0, 4, 1 },	// 最古ログ（年）
	{ 2,  6, 2, 0 },	// 最古ログ（月）
	{ 2, 10, 2, 0 },	// 最古ログ（日）
	{ 2, 16, 4, 1 },	// 最新ログ（年）
	{ 2, 22, 2, 0 }, 	// 最新ログ（月）
	{ 2, 26, 2, 0 }		// 最新ログ（日）
};

void	LogDateDsp4( ushort *Date )
{
	uchar	pos;			// 日付データ表示位置

	for( pos = 0 ; pos < 6 ; pos++){		// 日付ﾃﾞｰﾀ表示：開始（年）～終了（日）

		opedsp3	(
					POS_DATA4[pos][0],		// 表示行
					POS_DATA4[pos][1],		// 表示ｶﾗﾑ
					*Date,					// 表示ﾃﾞｰﾀ
					POS_DATA4[pos][2],		// ﾃﾞｰﾀ桁数
					POS_DATA4[pos][3],		// ０サプレス有無
					0,						// 反転表示：なし
					COLOR_BLACK,
					LCD_BLINK_OFF
				);


		Date++;								// 日付ﾃﾞｰﾀﾎﾟｲﾝﾀ更新
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印刷日付データ表示処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp5( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	反転表示位置					|*/
/*|											０～３：指定位置反転表示		|*/
/*|											その他：反転表示なし			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	m.nagashima													|*/
/*|	Date	:	2012-03-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ５（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA5_0[3][4] = {
	{ 5,  8, 4, 1 },	// 印刷（年）
	{ 5, 18, 2, 0 },	// 印刷（月）
	{ 5, 24, 2, 0 },	// 印刷（日）
};
const	ushort	POS_DATA5_1[3][4] = {
	{ 5, 12, 2, 1 },	// 印刷（年）
	{ 5, 18, 2, 0 },	// 印刷（月）
	{ 5, 24, 2, 0 },	// 印刷（日）
};

void	LogDateDsp5( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// 日付ﾃﾞｰﾀ表示位置
	ushort	rev_data;		// 反転表示ﾃﾞｰﾀ


	for( pos = 0 ; pos < 3 ; pos++){			// 印刷日付データ表示：印刷（年）～印刷（日）

		opedsp	(
					POS_DATA5_0[pos][0],		// 表示行
					POS_DATA5_0[pos][1],		// 表示ｶﾗﾑ
					*Date,						// 表示ﾃﾞｰﾀ
					POS_DATA5_0[pos][2],		// ﾃﾞｰﾀ桁数
					POS_DATA5_0[pos][3],		// ０サプレス有無
					0,							// 反転表示：なし
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;				// 反転表示ﾃﾞｰﾀｾｰﾌﾞ
		}

		Date++;									// 日付ﾃﾞｰﾀﾎﾟｲﾝﾀ更新
	}
	if( Rev_Pos < 3 ){							// 反転表示指定あり？

		opedsp	(								// 指定位置を反転表示させる
					POS_DATA5_1[Rev_Pos][0],	// 表示行
					POS_DATA5_1[Rev_Pos][1],	// 表示ｶﾗﾑ
					rev_data,					// 表示ﾃﾞｰﾀ
					POS_DATA5_1[Rev_Pos][2],	// ﾃﾞｰﾀ桁数
					POS_DATA5_1[Rev_Pos][3],	// ０サプレス有無
					1,							// 反転表示：あり
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータポインタ取得処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet( cnt, wp, max, req )						|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt	= 対象ﾊﾞｯﾌｧの現在格納件数				|*/
/*|																			|*/
/*|						ushort	wp	= 対象ﾊﾞｯﾌｧのﾗｲﾄﾎﾟｲﾝﾀ					|*/
/*|																			|*/
/*|						ushort	max	= 対象ﾊﾞｯﾌｧの最大格納件数				|*/
/*|																			|*/
/*|						uchar	req	= 処理要求								|*/
/*|										０：最古ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|										１：最新ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req )
{
	ushort	ret = 0;

	if( cnt != 0 ){

		if( req == 0 ){

			// 最古ﾃﾞｰﾀ検索
			if( cnt == max ){
				ret = wp;
			}
			else{
				ret = 0;
			}
		}
		else{
			// 最新ﾃﾞｰﾀ検索
			if( wp != 0 ){
				ret = wp-1;
			}
			else{
				ret = max-1;
			}
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータポインタ取得処理３（不正券情報プリント用）				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet3( cnt, wp, max, req )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req	= 処理要求								|*/
/*|										０：最古ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|										１：最新ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
ushort	LogPtrGet3( uchar req )
{
	ushort	posi = 0;
	return(posi);
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータポインタ取得処理２（入出庫情報プリント用）				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet2( no )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	no		= 車室番号							|*/
/*|					:	ulong	*inf	= ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）			|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= 件数									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-20													|*/
/*|	Update	:   2013-01-09 m.onouchi 										|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort LogPtrGet2(ushort no, ulong *inf)
{
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	uchar i;																	// 日時情報参照先用ループ変数
//	ushort pt = 0;																// 条件が一致したログの位置
//	ushort log_cnt = 0;															// 条件に一致したログの件数
//	ulong Wdate;																// 参照中日時情報
//	ulong Sdate = 0, Edate = 0;													// 最新・最古日時情報(仮)
//
//	((union ioLogNumberInfo *)(inf))->n = 0;
//	pt = UsMnt_SearchIOLog(0, no, 0, 0, &Io_log_wk);							// 最初のログ検索
//	if (pt) {																	// 条件に一致したログあり
//		/* 事象管理テーブルの終端を取得 */
//		for(i = 0; ((i+1) < IO_EVE_CNT) && (Io_log_wk.iolog_data[i+1].Event_type); i++);	// 事象件数末尾までシーク
//		Sdate = enc_nmlz_mdhm(													// 参照中イベント発生日時を変換
//			Io_log_wk.iolog_data[0].NowTime.Year,								// 年
//			Io_log_wk.iolog_data[0].NowTime.Mon,								// 月
//			Io_log_wk.iolog_data[0].NowTime.Day,								// 日
//			Io_log_wk.iolog_data[0].NowTime.Hour,								// 時
//			Io_log_wk.iolog_data[0].NowTime.Min									// 分
//		);
//		Edate = enc_nmlz_mdhm(													// 参照中イベント発生日時を変換
//			Io_log_wk.iolog_data[i].NowTime.Year,								// 年
//			Io_log_wk.iolog_data[i].NowTime.Mon,								// 月
//			Io_log_wk.iolog_data[i].NowTime.Day,								// 日
//			Io_log_wk.iolog_data[i].NowTime.Hour,								// 時
//			Io_log_wk.iolog_data[i].NowTime.Min									// 分
//		);
//		((union ioLogNumberInfo *)(inf))->log.sdat = (ulong)(pt - 1);			// 最古ログ領域情報更新
//		((union ioLogNumberInfo *)(inf))->log.edat = (ulong)(pt - 1);			// 最新ログ領域情報更新
//		((union ioLogNumberInfo *)(inf))->log.evnt = (ulong)i;					// 最新事象発生格納位置更新
//		log_cnt++;																// 条件一致件数更新
//		while (0 != (pt = UsMnt_SearchIOLog(pt, no, 0, 0, &Io_log_wk))) {		// 次の条件一致ログを検索
//			for(i = 0; ((i+1) < IO_EVE_CNT) && (Io_log_wk.iolog_data[i+1].Event_type); i++);	// 事象件数末尾までシーク
//			Wdate = enc_nmlz_mdhm(												// 参照中イベント発生日時を変換
//				Io_log_wk.iolog_data[0].NowTime.Year,							// 年
//				Io_log_wk.iolog_data[0].NowTime.Mon,							// 月
//				Io_log_wk.iolog_data[0].NowTime.Day,							// 日
//				Io_log_wk.iolog_data[0].NowTime.Hour,							// 時
//				Io_log_wk.iolog_data[0].NowTime.Min								// 分
//			);
//			if (Edate < Wdate) {												// 保持している時間より後
//				Edate = Wdate;													// 最新時間更新
//				((union ioLogNumberInfo *)(inf))->log.edat = (ulong)(pt - 1);	// 最新ログ領域情報更新
//				((union ioLogNumberInfo *)(inf))->log.evnt = (ulong)i;			// 最新事象発生格納位置更新
//			}
//			Wdate = enc_nmlz_mdhm(												// 参照中イベント発生日時を変換
//				Io_log_wk.iolog_data[i].NowTime.Year,							// 年
//				Io_log_wk.iolog_data[i].NowTime.Mon,							// 月
//				Io_log_wk.iolog_data[i].NowTime.Day,							// 日
//				Io_log_wk.iolog_data[i].NowTime.Hour,							// 時
//				Io_log_wk.iolog_data[i].NowTime.Min								// 分
//			);
//			if (Sdate > Wdate) {												// 保持している時間より前
//				Sdate = Wdate;													// 最古時間更新
//				((union ioLogNumberInfo *)(inf))->log.sdat = (ulong)(pt - 1);	// 最古ログ領域情報更新
//			}
//			log_cnt++;															// 条件一致件数更新
//		}
//	}
//	return log_cnt;															// 条件に一致したログの件数を返す
	return 0;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
}

/*[]-----------------------------------------------------------------------[]*/
/*|		最古＆最新ログデータ日付データ取得処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateGet( *Old, *New, *Date )						|*/
/*|																			|*/
/*|	PARAMETER		:	date_time_rec	*Old	= 最古ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ	|*/
/*|																			|*/
/*|						date_time_rec	*New	= 最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ	|*/
/*|																			|*/
/*|						ushort			*Date	= 日付ﾃﾞｰﾀ格納ﾎﾟｲﾝﾀ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date )
{
	// 最古ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ
	*Date = Old->Year;
	Date++;
	*Date = Old->Mon;
	Date++;
	*Date = Old->Day;
	Date++;
	// 最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ
	*Date = New->Year;
	Date++;
	*Date = New->Mon;
	Date++;
	*Date = New->Day;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータクリア処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDataClr( LogSyu )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	= ログデータ種別					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret = OK/NG									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	LogDataClr( ushort LogSyu )
{
	uchar	ret = OK;	// 戻り値（処理結果）

	switch( LogSyu ){				// ログデータ種別？

		case	LOG_ERROR:			// エラー情報
			FLT_LogErase2( eLOG_ERROR );
			break;

		case	LOG_ALARM:			// アラーム情報
			FLT_LogErase2( eLOG_ALARM );
			break;

		case	LOG_OPERATE:		// 操作情報
			FLT_LogErase2( eLOG_OPERATE );
			break;

		case	LOG_POWERON:		// 停復電情報
			FLT_LogErase2( eLOG_POWERON );
			break;

		case	LOG_TTOTAL:			// Ｔ合計情報
			FLT_LogErase2( eLOG_TTOTAL );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//			FLT_LogErase2( eLOG_LCKTTL );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
			break;

		case	LOG_GTTOTAL:		// ＧＴ合計情報
			FLT_LogErase2( eLOG_GTTOTAL );
			break;

		case	LOG_COINBOX:		// コイン金庫情報
			FLT_LogErase2( eLOG_COINBOX );
			break;

		case	LOG_NOTEBOX:		// 紙幣金庫情報
			FLT_LogErase2( eLOG_NOTEBOX );
			break;

		case	LOG_MONEYMANAGE:	// 釣銭管理情報
			FLT_LogErase2( eLOG_MONEYMANAGE );
			break;

		case	LOG_PAYMENT:		// 個別精算情報
			FLT_LogErase2( eLOG_PAYMENT );
			break;

		case	LOG_ABNORMAL:		// 不正・強制出庫情報
			FLT_LogErase2( eLOG_ABNORMAL );
			break;

		case	LOG_MONITOR:		// モニタ情報
			FLT_LogErase2( eLOG_MONITOR );
			break;

		case	LOG_CREUSE:			// クレジット利用明細ログ
			FLT_LogErase2( eLOG_CREUSE );
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case	LOG_EDYARM:			// Edyアラーム取引ログ
//			FLT_LogErase2( eLOG_EDYARM );
//			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {	// Edy設定があるか
//				wopelg( OPLOG_EDY_ARM_CLR,0,0 );	// 操作履歴登録（Edy取引ｱﾗｰﾑ情報ｸﾘｱ）
//			}
//			break;
//
//		case	LOG_EDYSHIME:			// Edy締め記録ログ
//			FLT_LogErase2( eLOG_EDYSHIME );
//			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {	// Edy設定があるか
//				wopelg( OPLOG_EDY_SHIME_CLR,0,0 );	// 操作履歴登録（Edy締め記録情報ｸﾘｱ）
//			}
//			break;
//#ifdef	FUNCTION_MASK_EDY 
//		case LOG_EDYSYUUKEI:								// Ｅｄｙ集計件数
//			Syuukei_sp.edy_inf.cnt = 0;						// 集計件数初期化
//			ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);				// 集計情報初期化処理
//			memset( &bk_syu_dat, 0, sizeof( bk_syu_dat ));
//			break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		case LOG_SCASYUUKEI:								// Ｓｕｉｃａ集計件数
			Syuukei_sp.sca_inf.cnt = 0;						// 集計件数初期化
			ck_syuukei_ptr_zai(LOG_SCASYUUKEI);				// 集計情報初期化処理
			memset( &bk_syu_dat, 0, sizeof( bk_syu_dat ));
			break;

// MH321800(S) D.Inaba ICクレジット対応
		case LOG_ECSYUUKEI:									// 決済リーダ集計件数
			Syuukei_sp.ec_inf.cnt = 0;						// 集計件数初期化
			ck_syuukei_ptr_zai(LOG_ECSYUUKEI);				// 集計情報初期化処理
			memset( &bk_syu_dat_ec, 0, sizeof( bk_syu_dat_ec ));
			break;
// MH321800(E) D.Inaba ICクレジット対応
		case ADDPRM_LOG:									// 設定更新情報
			SetDiffLogReset(1);								// 設定変更履歴ログクリア
			break;

		case LOG_HOJIN_USE:
			FLT_LogErase2( eLOG_HOJIN_USE );
			break;

		case	LOG_NGLOG:						// 不正ログ
			FLT_LogErase2( eLOG_NGLOG );
			break;

		case	LOG_REMOTE_SET:					// 遠隔料金設定ログ
			FLT_LogErase2( eLOG_REMOTE_SET );
			break;

// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//		case	LOG_ENTER:						// 入庫ログ
//			FLT_LogErase2( eLOG_ENTER );
//			break;
		case	LOG_RTPAY:						// RT精算データ
			FLT_LogErase2( eLOG_RTPAY );
			break;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
		
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		case	LOG_RTRECEIPT:					// RT領収証データ
			FLT_LogErase2( eLOG_RTRECEIPT );
			break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

		case	LOG_PARKING:					// 駐車台数データ
			FLT_LogErase2( eLOG_PARKING );
			break;
		
		case	LOG_LONGPARK:					// 長期駐車データ
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//			FLT_LogErase2( eLOG_LONGPARK );
			FLT_LogErase2( eLOG_LONGPARK_PWEB );
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
			break;
		
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		case	LOG_RISMEVENT:					// RISMイベントデータ
//			FLT_LogErase2( eLOG_RISMEVENT );
		case LOG_DC_LANE:						// レーンモニタデータ
			FLT_LogErase2( eLOG_DC_LANE );
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			break;

// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//		case LOG_IOLOG:
//			memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );
//			FLT_LogErase2( eLOG_IOLOG );
		case LOG_DC_QR:
			FLT_LogErase2( eLOG_DC_QR );
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
			break;

		case	LOG_MNYMNG_SRAM:				// 釣銭管理情報(SRAM)
			FLT_LogErase2( eLOG_MNYMNG_SRAM );
			FLT_LogErase2( eLOG_MONEYMANAGE );
			break;

		case	LOG_RTPAYMENT:					// 料金テスト情報(SRAM)
			RT_PAY_LOG_DAT.RT_pay_count = 0;	// 集計件数初期化
			RT_PAY_LOG_DAT.RT_pay_wtp = 0;		// ライトポインタクリア
			memset( &RT_PAY_LOG_DAT, 0, sizeof( RT_PAY_LOG_DAT ));	// 料金テスト履歴ログクリア
			break;
		case	LOG_REMOTE_MONITOR:				// 遠隔監視データ
			FLT_LogErase2( eLOG_REMOTE_MONITOR );
			break;
		default:								// その他（ログデータ種別エラー）
			ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}
	if(ret == OK) {
		LogDataClr_CheckBufferFullRelease(LogSyu);
	}

	return(ret);
}

/*[]----------------------------------------------------------------------[]*
 *| backup / restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_BackupRestore
 *| PARAMETER    : none
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	SysMnt_BackupRestore(void)
{
	ushort	usSysEvent;
	char	wk[2];
	
	/* initialize cursor position */
	DP_CP[0] = DP_CP[1] = 0;
	
	while (1) {
		/* clear screen */
		dispclr();
		/* display title */
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[39]);			/* [39]	"＜バックアップ／リストア＞" */
		/* display function */
		Fun_Dsp(FUNMSG[25]);						/* [25]	"　▲　　▼　　　　 読出  終了 " */
		/* display items */
		usSysEvent = Menu_Slt(BKRSMN, BKRS_TBL, (char)BKRS_MAX, (char)1);
		
		/* save current cursor position */
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		
		/* dispatch event */
		switch (usSysEvent) {
			case BKRS_BK:		/* no break */
			case BKRS_RS:
				usSysEvent = SysMnt_BackupRestoreData(usSysEvent);
				break;
//未使用です（S）				
			case BKRS_FLSTS:
				if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
					if( Ext_Menu_Max > 1){
						usSysEvent = UsMnt_PreAreaSelect(BKRS_FLSTS);
					}
					else {
						DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
						usSysEvent = UsMnt_AreaSelect(BKRS_FLSTS);
					}
				}else{
					BUZPIPI();
				}
				break;
//未使用です（E）				
			case MOD_EXT:		/* end function selected */
				return MOD_EXT;					/* exit screen! */
			default:
				break;
		}
		
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}

		if (usSysEvent == MOD_CHG) {
		/* mode change occured */
			return MOD_CHG;						/* exit screen! */
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( usSysEvent == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		/* restore cursor position */
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*
 *| backup / restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_BackupRestoreData
 *| PARAMETER    : kind - BKRS_BK or BKRS_RS
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	SysMnt_BackupRestoreData(ushort kind)
{
// MH810100(S) Y.Yamauchi 2020/03/10 車番チケットレス(メンテナンス)
//	short	msg;
	short msg, ret;
// MH810100(E) Y.Yamauchi 2020/03/10 車番チケットレス(メンテナンス)
	t_SysMnt_ErrInfo	errinfo;
	
	/* display foolproof */
	dispclr();
	SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_START);
	Fun_Dsp(FUNMSG[19]);								/* [19]	"　　　　　　 はい いいえ　　　" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);					/* [21]	"　　　 よろしいですか？ 　　　" */
	while (1) {
		/* wait key input */
		msg = sysmnt_GetMessage(_SYSMNT_WAIT_MSG);
		/* dispath event */
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			return MOD_CHG;				/* exit screen! */
		case KEY_TEN_F3:	/* はい */
			BUZPI();
			Ope_DisableDoorKnobChime();
			/* display executing */
			dispclr();
			SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_EXEC);
			Fun_Dsp(FUNMSG[77]);						/* [77]	"　　　　　　　　　　　　　　　" */
			grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);	/* [24]	"　　しばらくお待ちください　　" */
			/* initialization */
			_sysmnt_StartBackupRestore(&errinfo);
			/* execute download */
			switch (kind) {
			case BKRS_BK:
				xPause( 20 );		// 200ms wait（ﾌﾞｻﾞｰOFFするまで待つ）
				wopelg( OPLOG_ALLBACKUP, 0, 0 );			// 操作履歴登録
				sysmnt_Backup(&errinfo);
				break;
			case BKRS_RS:
				xPause( 20 ); 		// 200ms wait（ﾌﾞｻﾞｰOFFするまで待つ）
				sysmnt_Restore(&errinfo);
				wopelg( OPLOG_ALLRESTORE, 0, 0 );			// 操作履歴登録
				break;
			}
			if(!Ope_IsEnableDoorKnobChime()) {
				Ope_EnableDoorKnobChime();
			}
			/* display result */
			dispclr();
			SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_END);
			Fun_Dsp(FUNMSG[8]);							/* [08]	"　　　　　　　　　　　　 終了 " */
			SysMntDisplayResult(&errinfo);
			/* wait for user response */
// MH810100(S) Y.Yamauchi 2020/03/10 車番チケットレス(メンテナンス)
//			if (sysmnt_WaitF5() == KEY_MODECHG) {
//				return MOD_CHG;			/* exit screen! */
//			}
			ret = sysmnt_WaitF5();
			if( ret == LCD_DISCONNECT ){
				return MOD_CUT;
			}else if( ret == KEY_MODECHG ){
				return MOD_CHG;
			}
// MH810100(E) Y.Yamauchi 2020/03/10 車番チケットレス(メンテナンス)
			return MOD_EXT;				/* exit screen! */
		case KEY_TEN_F4:	/* いいえ */
			BUZPI();
			return MOD_EXT;				/* exit screen! */
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| backup
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_Backup
 *| PARAMETER    : errinfo - error information				<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_Backup(t_SysMnt_ErrInfo *errinfo)
{
	ulong ret;
	ulong verup = 0;
	uchar i;
	uchar f_Exec = FLT_EXCLUSIVE;			/* タスク切替せず実行 */

	// バックアップ時のログバージョンを格納
	bk_log_ver = LOG_VERSION;

	//リストア時にサイズが同じかチェックするために格納
	memset(&BR_LOG_SIZE,0,sizeof(BR_LOG_SIZE));
	for( i = 0; i < eLOG_MAX; i++ ){
		BR_LOG_SIZE[i] = LogDatMax[i][0];
	}

	/* lock RAM data */
	if (!sysmnt_RamLock(_SYSMNT_LOCK_ALL)) {
		errinfo->errmsg = _SYSMNT_ERR_DATA_LOCKED;
		remotedl_monitor_info_set(41);
		return;
	}
	
	/* write on flash */
	_di();
	ret = FLT_Backup(verup,f_Exec);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		ret = FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ログサイズ情報書き込み
	}
	_ei();
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		sysmnt_OnFlashErr(errinfo, ret);
		goto _end;
	}
	/* wait flash written */

_end:

	/* unlock RAM data */
	sysmnt_RamUnlock(_SYSMNT_LOCK_ALL);
}

/*[]----------------------------------------------------------------------[]*
 *| restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_Restore
 *| PARAMETER    : errinfo - error information				<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_Restore(t_SysMnt_ErrInfo *errinfo)
{
	ulong ret;
	uchar i;
	uchar f_Exec = FLT_EXCLUSIVE;			/* タスク切替せず実行 */
	uchar f_LogSzChg = 0;
	memset(&BR_LOG_SIZE,0,sizeof(BR_LOG_SIZE));
	if (!sysmnt_RamLock(_SYSMNT_LOCK_ALL)) {
		errinfo->errmsg = _SYSMNT_ERR_DATA_LOCKED;
		return;
	}
	
	/* write on flash */
	_di();
	AppServ_PParam_Copy = ON;				// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無＝ｺﾋﾟｰする
	ret = FLT_Restore( f_Exec );
	AppServ_PParam_Copy = OFF;				// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無＝ｺﾋﾟｰしない
	_ei();
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		sysmnt_OnFlashErr(errinfo, ret);
		goto _end;
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);		/* update parameter sum on ram */
	DataSumUpdate(OPE_DTNUM_LOCKINFO);		/* update parameter sum on ram */
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );		// FTP更新フラグセット（要電断）
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)

_end:
	/* unlock RAM data */
	sysmnt_RamUnlock(_SYSMNT_LOCK_ALL);
	ret = FLT_ReadBRLOGSZ( &BR_LOG_SIZE[0] );
	for(i = 0;i < eLOG_MAX; i++){
		if( BR_LOG_SIZE[i] != LogDatMax[i][0] ){
			FLT_LogErase2(i);
			f_LogSzChg = 1;
		}
	}
	if(f_LogSzChg != 0){
		for( i = 0; i < eLOG_MAX ; i++ ){
			BR_LOG_SIZE[i] = LogDatMax[i][0];							// 新ログサイズset
		}
		FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );			// 新ログサイズ情報書き込み
	}
}

/*[]----------------------------------------------------------------------[]*
 *| display title of screen
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMntDisplayBackupRestoreTitle
 *| PARAMETER    : kind - kind of screen (BKRS_XX)
 *|				   phase - phase of execution
 *|					(_SYSMNT_BKRS_START/_SYSMNT_BKRS_EXEC/_SYSMNT_BKRS_END)
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SysMntDisplayBackupRestoreTitle(ushort kind, int phase)
{
	ushort w1, w2;
	
	switch (kind) {
	case BKRS_BK:
		w1 = 46;
		switch (phase) {
		case _SYSMNT_BKRS_START:
			w2 = 40;
			break;
		case _SYSMNT_BKRS_EXEC:
			w2 = 41;
			break;
		case _SYSMNT_BKRS_END:
			w2 = 42;
			break;
		default:
			return;
		}
		break;
	case BKRS_RS:
		w1 = 47;
		switch (phase) {
		case _SYSMNT_BKRS_START:
			w2 = 43;
			break;
		case _SYSMNT_BKRS_EXEC:
			w2 = 44;
			break;
		case _SYSMNT_BKRS_END:
			w2 = 45;
			break;
		default:
			return;
		}
		break;
	default:
		return;
	}
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[w1]);
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[w2]);
}

/*[]----------------------------------------------------------------------[]*
 *| display result of execution
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMntDisplayResult
 *| PARAMETER    : errinfo - result of execution
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SysMntDisplayResult(t_SysMnt_ErrInfo *errinfo)
{
	if (errinfo->errmsg == _SYSMNT_ERR_NONE) {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* [27]	"　　　　　 正常終了 　　　　　" */
		BUZPI();
	}
	else {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* [28]	"　　　　　 異常終了 　　　　　" */
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[errinfo->errmsg]);	/* error detail */
		if (errinfo->errmsg == _SYSMNT_ERR_WRITE || errinfo->errmsg == _SYSMNT_ERR_ERASE) {
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[35]);	/* [35]	"アドレス：　　　Ｈ　　　　　　" */
			opedpl2(6, 10, errinfo->address, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		/* 		"　　　　　XXXXXX　　　　　　　" */
		}
		else if (errinfo->errmsg == _SYSMNT_ERR_VERIFY) {
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[37]);	/* [37]	"再度実行して下さい。　　　　　" */
		}
		BUZPIPI();
	}
}

/*[]----------------------------------------------------------------------[]*
 *| wait for F5 pushed
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_WaitF5
 *| PARAMETER    : none
 *| RETURN VALUE : event occured(KEY_TEN_F5/KEY_MODECHG)
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	sysmnt_WaitF5(void)
{
	short msg;
	
	/* read event stocked */
	while (1) {
		msg = sysmnt_GetMessage(_SYSMNT_NOT_WAIT_MSG);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (msg == KEY_MODECHG) {
		if (msg == KEY_MODECHG ||  msg == LCD_DISCONNECT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if (!SysMntModeChange) {
				SysMntModeChange = TRUE;
			}
			else {
				SysMntModeChange = FALSE;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			if (!SysMntLcdDisconnect) {
				SysMntLcdDisconnect = TRUE;
			}
			else {
				SysMntLcdDisconnect = FALSE;
			}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		}
		else if (msg == TIMEOUT6) {
			break;
		}
	}
	
	if (SysMntModeChange ) {
		msg =  KEY_MODECHG;
// MH810100(S)
	} else if(SysMntLcdDisconnect){
		msg =  LCD_DISCONNECT;
// MH810100(E)
	}
	else {
		/* wait F5 input */
		while (1) {
			msg = sysmnt_GetMessage(_SYSMNT_WAIT_MSG);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			if (msg == KEY_MODECHG || msg == KEY_TEN_F5) {
			if (msg == KEY_MODECHG || msg == KEY_TEN_F5 || msg == LCD_DISCONNECT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				break;
			}
		}
	}
	
	BUZPI();
	
	return msg;
}



/*[]----------------------------------------------------------------------[]*
 *| initialize backup/restore/download/upload operations
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_StartBkRsUpDwn
 *| PARAMETER    : errinfo - error information to initialize	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_StartBkRsUpDwn(t_SysMnt_ErrInfo *errinfo)
{
	sysmnt_InitErr(errinfo);
	SysMntModeChange = FALSE;
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
	SysMntLcdDisconnect = FALSE;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
}

/*[]----------------------------------------------------------------------[]*
 *| initialize error information
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_InitErr
 *| PARAMETER    : errinfo - error information to initialize	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_InitErr(t_SysMnt_ErrInfo *errinfo)
{
	errinfo->errmsg = _SYSMNT_ERR_NONE;
}

/*[]----------------------------------------------------------------------[]*
 *| regist error result to call flash api, to error information
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_OnFlashErr
 *| PARAMETER    : errinfo - error information to regist to	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_OnFlashErr(t_SysMnt_ErrInfo *errinfo, ulong fltcode)
{
	switch (_FLT_RtnKind(fltcode)) {
	case FLT_ERASE_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_ERASE;
		errinfo->address	= _FLT_RtnDetail(fltcode);
		remotedl_monitor_info_set(42);
		break;
	case FLT_WRITE_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_WRITE;
		errinfo->address	= _FLT_RtnDetail(fltcode);
		remotedl_monitor_info_set(43);
		break;
	case FLT_VERIFY_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_VERIFY;
		remotedl_monitor_info_set(44);
		break;
	case FLT_PARAM_ERR:		/* no break */
		/* this path should not be passed. */
	case FLT_NODATA:
		errinfo->errmsg		= _SYSMNT_ERR_NO_DATA;
		remotedl_monitor_info_set(45);
		break;
	case FLT_BUSY:			/* no break */
	case FLT_NOT_LOCKED:
		errinfo->errmsg		= _SYSMNT_ERR_DATA_LOCKED;
		remotedl_monitor_info_set(46);
		break;
	case FLT_INVALID_SIZE:
		errinfo->errmsg		= _SYSMNT_ERR_INVALID_DATA;
		remotedl_monitor_info_set(47);
		break;
	default:	/* case FLT_NORMAL */
		errinfo->errmsg	= _SYSMNT_ERR_NONE;
		break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| get message with using timeout
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_GetMessage
 *| PARAMETER    : option - _SYSMNT_WAIT_MSG/_SYSMNT_NOT_WAIT_MSG
 *| RETURN VALUE : message
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	sysmnt_GetMessage(int option)
{
	short msg;
	
	if (option == _SYSMNT_NOT_WAIT_MSG) {
	/* start timer to set timeout */
		Lagtim(OPETCBNO, 6, 1);
	}
	/* wait message */
	msg = StoF(GetMessage(), 1);
	if (option == _SYSMNT_NOT_WAIT_MSG) {
	/* stop timer */
		Lagcan(OPETCBNO, 6);
	}
	
	return msg;
}

/*[]----------------------------------------------------------------------[]*
 *| lock ram area
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_RamLock
 *| PARAMETER    : lock - ram area to lock (_SYSMNT_LOCK_XXX)
 *| RETURN VALUE : TRUE as success to lock
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	sysmnt_RamLock(ulong lock)
{
	int i;
	
	/* get semaphore */
	for (i = 0; i < 32; i++) {
		if (lock & BitShift_Left(1L,i)) {
			if (!Log_SemGet((uchar)i)) {
				break;		/* can't get semaphore */
			}
		}
	}
	if (i < 32) {
	/* failed to get */
		/* release semaphore, already have get */
		while (--i >= 0) {
			if (lock & BitShift_Left(1L,i)) {
				Log_SemFre((uchar)i);
			}
		}
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*
 *| unlock ram area
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_RamUnlock
 *| PARAMETER    : unlock - ram area to unlock (_SYSMNT_LOCK_XXX)
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_RamUnlock(ulong unlock)
{
	int i;
	
	for (i = 0; i < 32; i++) {
		if (unlock & BitShift_Left(1L,i)) {
			Log_SemFre((uchar)i);
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ビット左シフト（４バイトまで対応）
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : BitShift_Left
 *| PARAMETER    : 	ulong 	data	：元データ
 *| 				int		bit_cnt	：左シフトしたいビット数
 *| RETURN VALUE : 	ulong output	：結果
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	BitShift_Left(ulong data,int bit_cnt)
{
	int i;
	ulong output;

	output = data;
	for(i=0;i<bit_cnt;i++){
		output = output << 1;
	}
	return(output);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ロゴ印字データプリンタ登録処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMntLogoDataRegDsp( )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar *title = 画面タイトル							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	SysMntLogoDataRegDsp( const uchar *title )
{
	T_FrmLogoRegist	FrmLogoRegist;		// ロゴ印字データ登録要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	uchar			wait_cnt= 0;		// ロゴデータ登録完了待ち回数
	uchar			end_sts	= 0;		// 登録処理終了状態

	// ロゴデータ登録中画面表示
	dispclr();												// 画面クリア
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);								// ﾀﾞｳﾝﾛｰﾄﾞﾀｲﾄﾙ表示
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[54]);						// [54]	"　プリンタにデータ登録中です　"
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);				// [24]	"　　しばらくお待ちください　　"
	Fun_Dsp(FUNMSG[77]);									// [77]	"　　　　　　　　　　　　　　　"

	// ロゴデータ登録要求メッセージ作成
	Logo_Reg_sts_rct = OFF;									// ﾛｺﾞ登録処理状態ﾘｾｯﾄ（ﾚｼｰﾄ）
	Logo_Reg_sts_jnl = OFF;									// ﾛｺﾞ登録処理状態ﾘｾｯﾄ（ｼﾞｬｰﾅﾙ）


	FrmLogoRegist.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	Logo_Reg_sts_jnl = ON;									// ﾛｺﾞ登録処理状態（ｼﾞｬｰﾅﾙ）：登録終了ｾｯﾄ

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ロゴ印字データ登録要求

	Lagtim( OPETCBNO, 6, 1*50 );							// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（１秒）起動

	// ロゴデータ登録終了待ち
	for( ; end_sts == 0 ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		switch( RcvMsg ){									// イベント？

			case TIMEOUT6:									// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（２秒）ﾀｲﾑｱｳﾄ

				if( (Logo_Reg_sts_rct == ON) && (Logo_Reg_sts_jnl == ON) ){
					// 登録処理状態：終了
					end_sts = 1;							// 登録終了待ち画面終了（正常終了）
				}
				else{
					// 登録処理状態：未終了

					if( wait_cnt < 20 ){					// 最大２０秒まで登録終了待ちとする

						if( wait_cnt < 15 ){
							grachr(6, (ushort)(wait_cnt*2), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[38]);	// 完了待ち「＊」表示
						}
						wait_cnt++;							// 登録完了待ち回数＋１
						Lagtim( OPETCBNO, 6, 1*50 );		// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（１秒）起動
					}
					else{
						end_sts = 2;						// 登録終了待ち画面終了（異常終了）
					}
				}
				break;
		}
	}

	dispclr();												// 画面クリア
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);		// ﾀﾞｳﾝﾛｰﾄﾞﾀｲﾄﾙ表示

	if( end_sts == 1 ){										// 登録処理結果？
		// 正常終了
		BUZPI();
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[55]);					// [55]	"　ダウンロードとプリンタへの　"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[56]);					// [56]	"　データ登録を終了しました。　"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);					// [27]	"　　　　　 正常終了 　　　　　"
	}
	else{
		// 異常終了
		BUZPIPI();
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[57]);					// [57]	"　プリンタへのデータ登録に　　"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[58]);					// [58]	"　失敗しました。　　　　　　　"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);					// [28]	"　　　　　 異常終了 　　　　　"
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ロゴ印字データプリンタ登録処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMntLogoDataReg( )								|*/
/*|																			|*/
/*|	PARAMETER		:	*end_sts = 登録処理終了状態							|*/
/*|																			|*/
/*|	RETURN VALUE	:	MOD_EXT												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMntLogoDataReg(void)
{
	T_FrmLogoRegist	FrmLogoRegist;		// ロゴ印字データ登録要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	uchar			wait_cnt= 0;		// ロゴデータ登録完了待ち回数
	uchar			end_sts = 0;

	// ロゴデータ登録要求メッセージ作成
	Logo_Reg_sts_rct = OFF;									// ﾛｺﾞ登録処理状態ﾘｾｯﾄ（ﾚｼｰﾄ）
	Logo_Reg_sts_jnl = OFF;									// ﾛｺﾞ登録処理状態ﾘｾｯﾄ（ｼﾞｬｰﾅﾙ）

	FrmLogoRegist.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	Logo_Reg_sts_jnl = ON;									// ﾛｺﾞ登録処理状態（ｼﾞｬｰﾅﾙ）：登録終了ｾｯﾄ

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ロゴ印字データ登録要求

	Lagtim( OPETCBNO, 6, 1*50 );							// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（１秒）起動

	// ロゴデータ登録終了待ち
	for( ; end_sts == 0 ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		switch( RcvMsg ){									// イベント？

			case TIMEOUT6:									// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（２秒）ﾀｲﾑｱｳﾄ

				if( (Logo_Reg_sts_rct == ON) && (Logo_Reg_sts_jnl == ON) ){
					// 登録処理状態：終了
					end_sts = 1;							// 登録終了待ち画面終了（正常終了）
				}
				else{
					// 登録処理状態：未終了

					if( wait_cnt < 20 ){					// 最大２０秒まで登録終了待ちとする
						wait_cnt++;							// 登録完了待ち回数＋１
						Lagtim( OPETCBNO, 6, 1*50 );		// ﾛｺﾞﾃﾞｰﾀ登録完了待ちﾀｲﾏｰ（１秒）起動
					}
					else{
						// 登録終了待ち画面終了（異常終了）
						end_sts = 2;						// 登録終了待ち画面終了（異常終了）
					}
				}
				break;
		}
	}

	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| システムメンテナンス：カード発行                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_CardIssue( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SysMnt_CardIssue( void )
{
	ushort	usSysEvent;
	char	wk[2];
	ushort	ret = 0;
	ushort	msg;

	DP_CP[0] = DP_CP[1] = 0;

	dispclr();
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR2[24]);	/* "              ｉ              " */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[25]);			/* " プラスチックカード(低保磁)に " */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[26]);			/* " 書き込みするときは必ず       " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[27]);			/* " 磁気リーダーのSW6(保磁力設定)" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[28]);			/* " をOFFにしてください          " */
	Fun_Dsp( FUNMSG2[43] );													/* "　　　　　　 確認 　　　　　　" */
	for ( ret = 0 ; ret == 0 ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F3:		/* 確認(F3) */
			BUZPI();
			ret = MOD_EXT;
			break;
		default:
			break;
		}
	}
	ret = 0;

	for ( ret = 0 ; ret == 0 ; ){

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0] );		/* "＜カード発行＞　　　　　　　　" */

		usSysEvent = Menu_Slt( CARDMENU, CARD_ISU_TBL, (char)CARD_ISU_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usSysEvent) {
		/* 1.定期 */
		case CARD_PASS:
			usSysEvent = card_iss_pass();
			break;
		/* 2.係員カード */
		case CARD_KKRI:
			usSysEvent = card_iss_kakari();
			break;
		/* 3.Mifare定期 */
		case MOD_EXT:
			ret = MOD_EXT;
			break;
		default:
			break;
		}
		if (usSysEvent == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( usSysEvent == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}

	dispclr();
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR2[24]);	/* "              ｉ              " */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[27]);			/* " 磁気リーダーのSW6(保磁力設定)" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[29] );			/* " を変更した場合は             " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[30] );			/* " 必ず元に戻してください       " */
	Fun_Dsp( FUNMSG2[43] );													/* "　　　　　　 確認 　　　　　　" */
	ret = 0;
	for ( ret = 0 ; ret == 0 ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT) {	
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F3:		/* 確認(F3) */
			BUZPI();
			return MOD_EXT;
			break;
		default:
			break;
		}
	}
	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| カード発行：定期                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : card_iss_pass( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
enum {
	CI_KIND = 0,
	CI_NO,
	CI_B_YEAR,
	CI_B_MONT,
	CI_B_DAY,
	CI_E_YEAR,
	CI_E_MONT,
	CI_E_DAY,
	CI_MONT,
	CI_DAY,
	CI_HOUR,
	CI_MIN,
	CI_STS,
	_CI_MAX_
};
static ushort	card_iss_pass(void)
{
	ushort		msg;
	long		prm[_CI_MAX_];		/* 設定パラメータ */
	int			pos;
	char		mode;
	long		input;
	m_apspas	*outp;
	char		err_flag;
	uchar		tbl[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	uchar		str[4];
	uchar		ch[2];
	uchar		move_cmd = OFF;			// ｶｰﾄﾞ取込要求ｺﾏﾝﾄﾞ送信ﾌﾗｸﾞ
	long		prm_wk;					// ﾊﾟﾗﾒｰﾀ保存ﾜｰｸ
	uchar		IdSyuPara;				// 使用ID種設定ﾊﾟﾗﾒｰﾀ
	uchar		sts_bit = 0;			// ディップスイッチチェックビット
	uchar		dipSW_getSts;			// DipSW 設定取得状態

	dipSW_getSts = FRdr_DipSW_GetStatus();		/* 磁気リーダーのDipSW状態を取得 */

	OPE_red = 6;
	opr_snd( 90 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞを無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信

	read_sht_opn();						// ｼｬｯﾀｰ「開」

	prm[CI_KIND] = 1;
	prm[CI_NO] = 1;
	prm[CI_B_YEAR] = prm[CI_E_YEAR] = (long)(CLK_REC.year%100);
	prm[CI_B_MONT] = prm[CI_E_MONT] = prm[CI_MONT] = (long)CLK_REC.mont;
	prm[CI_B_DAY] = prm[CI_E_DAY] = prm[CI_DAY] = (long)CLK_REC.date;
	prm[CI_HOUR] = CLK_REC.hour;
	prm[CI_MIN] = CLK_REC.minu;
	prm[CI_STS] = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0]);			/* "＜カード発行＞　　　　　　　　" */
	grachr(0, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "　　　　　　　　定期　　　　　" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[1]);			/* "種別　　　　 定期No.　　　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[2]);			/* "有効開始　　　年　　月　　日　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[3]);			/* "有効終了　　　年　　月　　日　" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[4]);			/* "処理　　　　月　　日　　：　　" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[5]);			/* "ステータス　　　　　　　　　　" */
	/* パラメータ表示 */
	cardiss_pass_prm_dsp(0, prm[0], 1);
	for (pos = 1; pos < _CI_MAX_; pos++) {
		cardiss_pass_prm_dsp(pos, prm[pos], 0);
	}
	Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */

	mode = 0;
	pos = 0;
	input = -1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if ( msg == KEY_MODECHG ) {
			BUZPI();
			rd_shutter();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
			if( move_cmd == ON ){
				opr_snd( 90 );						// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
			}
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT ) {
			BUZPI();
			rd_shutter();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
			if( move_cmd == ON ){
				opr_snd( 90 );						// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
			}
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
		/* パラメータ設定中 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				rd_shutter();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				if( move_cmd == ON ){
					opr_snd( 90 );					// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				}
				OPE_red = 2;
				if(( RD_mod != 10 )&&( RD_mod != 11 )){
					opr_snd( 3 );					// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
				}
				return MOD_EXT;
			case KEY_TEN_F1:		/* ▲(F1) */
			case KEY_TEN_F2:		/* ▼(F2) */
				if (input != -1) {
					/* 有効範囲チェック */
					prm_wk = prm[pos];				// 入力前のﾊﾟﾗﾒｰﾀをｾｰﾌﾞ
					prm[pos] = input;				// 入力されたﾊﾟﾗﾒｰﾀｾｯﾄ
					if (cardiss_pass_prm_chk( prm )) {
						// ﾊﾟﾗﾒｰﾀﾁｪｯｸＮＧ
						prm[pos] = prm_wk;			// 入力前のﾊﾟﾗﾒｰﾀに戻す
						BUZPIPI();
						cardiss_pass_prm_dsp(pos, prm[pos], 1);
						input = -1;
						break;
					}
				}
				BUZPI();
				/* 表示-正転 */
				cardiss_pass_prm_dsp(pos, prm[pos], 0);
				if (pos == CI_STS) {
					Fun_Dsp( FUNMSG2[16] );			/* "　▲　　▼　　　　 発行  終了 " */
				}
				/* カーソル移動 */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = (char)(_CI_MAX_-1);
				}
				else {
					if (++pos >= _CI_MAX_)
						pos = 0;
				}
				/* 表示-反転 */
				cardiss_pass_prm_dsp(pos, prm[pos], 1);
				if (pos == CI_STS) {
					Fun_Dsp( FUNMSG2[18] );			/* "　▲　　▼　 切替　発行  終了 " */
					input = prm[pos];
				}
				else {
					input = -1;
				}
				break;
			case KEY_TEN_F3:		/* 切替(F3) */
				if (pos == CI_STS) {
					BUZPI();
					if (++input > 3)
						input = 0;
					/* 表示-反転 */
					cardiss_pass_prm_dsp(pos, input, 1);
				}
				break;
			case KEY_TEN_F4:		/* 発行(F4) */
				if (input != -1) {
					/* 有効範囲チェック */
					prm_wk = prm[pos];				// 入力前のﾊﾟﾗﾒｰﾀをｾｰﾌﾞ
					prm[pos] = input;				// 入力されたﾊﾟﾗﾒｰﾀｾｯﾄ
					if (cardiss_pass_prm_chk( prm )) {
						// ﾊﾟﾗﾒｰﾀﾁｪｯｸＮＧ
						prm[pos] = prm_wk;			// 入力前のﾊﾟﾗﾒｰﾀに戻す
						BUZPIPI();
						cardiss_pass_prm_dsp(pos, prm[pos], 1);
						input = -1;
						break;
					}
				}
				BUZPI();
				/* カード取り込み要求 */
				opr_snd(200);
				move_cmd = ON;						// ｶｰﾄﾞ取込要求送信ﾌﾗｸﾞｾｯﾄ
				/* カード発行画面へ遷移 */
				cardiss_pass_prm_dsp(pos, prm[pos], 0);
				grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
				Fun_Dsp( FUNMSG[82] );				/* "　　　　　　 中止             " */
				mode = 1;
				err_flag = 0;
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				if (pos == CI_STS)
					break;
				BUZPI();
				if (input == -1)
					input = 0;
				if (pos == CI_NO)
					input = (input*10 + msg-KEY_TEN0) % 100000;
				else
					input = (input*10 + msg-KEY_TEN0) % 100;
				cardiss_pass_prm_dsp(pos, input, 1);
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				cardiss_pass_prm_dsp(pos, prm[pos], 1);
				if (pos == CI_STS) {
					input = prm[pos];
				}
				else {
					input = -1;
				}
				break;
			default:
				break;
			}
		}
		else if ( mode == 3 ) {
			/************************************************/
			/*		中止ボタン押下後のカード位置判定		*/
			/************************************************/
			switch ( msg ) {
				case ARC_CR_E_EVT:						/* リーダー動作完了イベント */
					Lagcan( OPETCBNO, 6 );
					mode = 0;
					if(dipSW_getSts == 1){				/* DipSW設定状態取得済み */
						sts_bit = RDS_REC.state[1];		// 磁気リーダー(DipSwitch SW2 の値取得
						sts_bit &= 0x0F;				// 下位4bitのみ有効
						switch(sts_bit){
							case 0x03:					// 海外3:ISO_Track2クレジット対応(上下2方向,逆面取付)
							case 0x04:					// 国内:JISⅡクレジット対応(逆面取付)
							case 0x05:					// ACI向けクレジット対応(上下2方向,逆面取付,HR200 127店舗対応)
								if(RED_REC.posi[0] & 0x07){		// 磁気リーダー奥側（センサー1,2,3）にカードが位置している場合(逆面取付)は、返却コマンドを発行する
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* 状態要求ﾀｲﾏｰ(2s)起動 */
									mode = 4;						// カード返却コマンドの終了待ち状態へ
								}
								break;
							default:
								if ( RED_REC.posi[0] & 0x70){	// 磁気リーダー奥側（センサー7,6,5）にカードが位置している場合(正面取付け)は、返却コマンドを発行する
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* 状態要求ﾀｲﾏｰ(2s)起動 */
									mode = 4;						// カード返却コマンドの終了待ち状態へ
								}
								break;
						}
					}
					break;
				case TIMEOUT6:					/* 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ */
					mode = 0;
					break;
				default:
					break;
			}
		}
		else if ( mode == 4 ) {
			/****************************************/
			/*		カード返却コマンド終了待ち		*/
			/****************************************/
			switch ( msg ) {
			case ARC_CR_E_EVT:				/* リーダー動作完了イベント */
				Lagcan( OPETCBNO, 6 );
				mode = 0;					// パラメータ設定状態へ
				break;
			case TIMEOUT6:					/* 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ */
				mode = 0;					// パラメータ設定状態へ
				break;
			default:
				break;
			}
		}
		/* カード発行中 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 中止(F3) */
				BUZPI();
				if (err_flag) {
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
					err_flag = 0;
					/* カード取り込み要求 */
					opr_snd(200);
				}
				else {
					pos = 0;
					cardiss_pass_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
					Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */
					input = -1;
					mode = 3;
					opr_snd( 90 );						// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
					Lagtim( OPETCBNO, 6, 2*50 );		/* 状態要求ﾀｲﾏｰ(2s)起動 */
				}
				break;

			case ARC_CR_EOT_RCMD:	/* カード抜き取り */

				if( err_flag ){
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
					err_flag = 0;
					opr_snd(200);						/* ｶｰﾄﾞ取込要求						*/
				}
				break;

			case ARC_CR_E_EVT:		/* リーダー動作完了イベント */
				/* カード取り込み終了 */
				if (mode == 1) {
					/* 定期データ作成 */
					outp = (m_apspas *)MDP_buf;

					if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// 新カードＩＤ使用する？
						(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APSフォーマットで使用する？

						// 新カードＩＤを使用する場合
						IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,11,1,1 );				// 基本駐車場No.券ID（定期券）設定ﾊﾟﾗﾒｰﾀ取得
						switch( IdSyuPara ){

							case	1:													// ID1
								outp->aps_idc = 0x1A;
								break;
							case	2:													// ID2
								outp->aps_idc = 0x64;
								break;
							case	3:													// ID3
								outp->aps_idc = 0x6A;
								break;
							case	4:													// ID4
								outp->aps_idc = 0x70;
								break;
							case	5:													// ID5
								outp->aps_idc = 0x76;
								break;
							case	6:													// ID1とID2
								outp->aps_idc = 0x1A;
								break;
							case	7:													// ID1とID3
								outp->aps_idc = 0x1A;
								break;
							case	8:													// ID6
								outp->aps_idc = 0x44;
								break;
							case	0:													// 読まない
							default:													// その他（設定値ｴﾗｰ）
								outp->aps_idc = 0x1A;
								break;
						}
					}
					else{
						// 新カードＩＤを使用しない場合
						outp->aps_idc = 0x1A;											/* カード種別(0x1A) */
					}
					outp->aps_pno[0] = (uchar)(CPrmSS[S_SYS][1] & 0x7FL);				/* 駐車場番号 下位7bit(bit6-bit0) */
					outp->aps_pno[1] = (uchar)(((CPrmSS[S_SYS][1] & 0x0080L) >> 1) |	/* bit6:駐車場番号bit7 */
											   ((CPrmSS[S_SYS][1] & 0x0100L) >> 3) |	/* bit5:駐車場番号bit8 */
											   ((CPrmSS[S_SYS][1] & 0x0200L) >> 5) |	/* bit4:駐車場番号bit9 */
											   prm[CI_KIND]);							/* bit3-bit0:定期券種別 */
					outp->aps_pcd[0] = (uchar)(prm[CI_NO] >> 7);						/* 定期券番号 上位7bit */
					outp->aps_pcd[1] = (uchar)(prm[CI_NO] & 0x7FL);						/*            下位7bit */
					outp->aps_sta[0] = (uchar)prm[CI_B_YEAR];							/* 有効開始年 */
					outp->aps_sta[1] = (uchar)prm[CI_B_MONT];							/*         月 */
					outp->aps_sta[2] = (uchar)prm[CI_B_DAY];							/*         日 */
					outp->aps_end[0] = (uchar)prm[CI_E_YEAR];							/* 有効開始年 */
					outp->aps_end[1] = (uchar)prm[CI_E_MONT];							/*         月 */
					outp->aps_end[2] = (uchar)prm[CI_E_DAY];							/*         日 */
					outp->aps_sts = (uchar)prm[CI_STS];									/* ステータス */
					outp->aps_wrt[0] = (uchar)prm[CI_MONT];								/* 処理月 */
					outp->aps_wrt[1] = (uchar)prm[CI_DAY];								/* 　　日 */
					outp->aps_wrt[2] = (uchar)prm[CI_HOUR];								/* 　　時 */
					outp->aps_wrt[3] = (uchar)prm[CI_MIN];								/* 　　分 */
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット						
						outp->aps_pno[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x0400L) >> 3);/* bit7:駐車場番号bit10 */
						outp->aps_pno[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x0800L) >> 4);/* bit7:駐車場番号bit11 */
						outp->aps_pcd[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x1000L) >> 5);/* bit7:駐車場番号bit12 */
						outp->aps_pcd[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x2000L) >> 6);/* bit7:駐車場番号bit13 */
						outp->aps_sta[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x4000L) >> 7);/* bit7:駐車場番号bit14 */
						outp->aps_sta[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x8000L) >> 8);/* bit7:駐車場番号bit15 */
						outp->aps_sta[2] |= (uchar)((CPrmSS[S_SYS][1] & 0x10000L) >> 9);/* bit7:駐車場番号bit16 */
						outp->aps_end[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x20000L) >> 10);/* bit7:駐車場番号bit17 */
						MDP_buf[127] = 1;		//GTフォーマット作成フラグ
					}else{
						MDP_buf[127] = 0;		//GTフォーマット作成フラグ
					}
					MDP_mag = sizeof( m_apspas );
					MDP_siz = sizeof(m_apspas);
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット						
						md_pari2((uchar *)MDP_buf, (ushort)1, 1);								/* パリティ作成 */
					}else{
						md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 0);								/* パリティ作成 */
					}
					/* 定期データ書き込み */
					opr_snd(201);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[8]);		/* "　　　　＝書込中です＝　　　　" */
					mode = 2;
				}
				/* カード書き込み終了 */
				else {
					/* 書込ＮＧ */
					if (RED_REC.ercd) {
						ch[0] = tbl[RED_REC.ercd >> 4 & 0x0F];
						ch[1] = tbl[RED_REC.ercd & 0x0F];
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[7]);		/* "　　　　書込ＮＧ（Ｅ　　）　　" */
						as1chg(ch, str, 2);
						grachr(6, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str);						/* エラーコード */
						err_flag = 1;
					}
					/* 書込ＯＫ */
					else {
						/* 定期No.を１増やす */
						if (++prm[CI_NO] > 12000)
							prm[CI_NO] = 1;
						cardiss_pass_prm_dsp(CI_NO, prm[CI_NO], 0);
						grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
						/* カード取り込み要求 */
						opr_snd(200);
					}
					mode = 1;
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期－パラメータ表示                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_pass_prm_dsp( pos, prm, rev )                   |*/
/*| PARAMETER    : char   pos : 表示項目番号                               |*/
/*|              : long   prm : データ内容                                 |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	cardiss_pass_prm_dsp(char pos, long prm, ushort rev)
{
	ushort	line;
	ushort	lpos;
	ushort	keta;
	const uchar	*pstr;

	if (pos == CI_STS) {
		if (prm == 0)
			pstr = DAT2_6[18];	/* "発行" */
		else if (prm == 1)
			pstr = DAT2_6[15];	/* "入庫" */
		else if (prm == 2)
			pstr = DAT2_6[14];	/* "出庫" */
		else
			pstr = DAT2_6[19];	/* "精算" */
		grachr(5, 12, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, pstr);
	}
	else {
		keta = 2;
		switch (pos) {
		case CI_KIND:
			line = 1;
			lpos = 4;
			break;
		case CI_NO:
			keta = 5;
			line = 1;
			lpos = 20;
			break;
		case CI_B_YEAR:
			line = 2;
			lpos = 10;
			break;
		case CI_B_MONT:
			line = 2;
			lpos = 16;
			break;
		case CI_B_DAY:
			line = 2;
			lpos = 22;
			break;
		case CI_E_YEAR:
			line = 3;
			lpos = 10;
			break;
		case CI_E_MONT:
			line = 3;
			lpos = 16;
			break;
		case CI_E_DAY:
			line = 3;
			lpos = 22;
			break;
		case CI_MONT:
			line = 4;
			lpos = 8;
			break;
		case CI_DAY:
			line = 4;
			lpos = 14;
			break;
		case CI_HOUR:
			line = 4;
			lpos = 20;
			break;
		default:	/* case CI_MIN: */
			line = 4;
			lpos = 26;
			break;
		}
		opedpl(line, lpos, (ulong)prm, keta, 1, rev, COLOR_BLACK,  LCD_BLINK_OFF);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期－パラメータチェック                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_pass_prm_chk( *prm )                            |*/
/*| PARAMETER    : long   *prm : ﾊﾟﾗﾒｰﾀﾎﾟｲﾝﾀ                               |*/
/*|                                                                        |*/
/*| RETURN VALUE : 0:OK                                                    |*/
/*|              : 1:NG                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	cardiss_pass_prm_chk( long *prm )
{
	short	para_data[_CI_MAX_];		// 設定ﾊﾟﾗﾒｰﾀﾜｰｸ
	uchar	i;							// ﾙｰﾌﾟｶｳﾝﾀｰ

	// ﾊﾟﾗﾒｰﾀﾃﾞｰﾀ取得
	for( i=0 ; i<_CI_MAX_ ; i++ ){
		para_data[i] = (short)*prm;
		prm++;
	}
	// 有効開始年ﾃﾞｰﾀ変換（ 80～99=1980～1999 / 00～79=2000～2079 ）
	if( para_data[CI_B_YEAR] < 80 ){
		para_data[CI_B_YEAR] += 2000;
	}
	else{
		para_data[CI_B_YEAR] += 1900;
	}
	// 有効終了年ﾃﾞｰﾀ変換（ 80～99=1980～1999 / 00～79=2000～2079 ）
	if( para_data[CI_E_YEAR] < 80 ){
		para_data[CI_E_YEAR] += 2000;
	}
	else{
		para_data[CI_E_YEAR] += 1900;
	}
	// 種別ﾁｪｯｸ
	if( (para_data[CI_KIND] < 1) || (para_data[CI_KIND] > 15) ){
		return(1);
	}
	// 定期No.ﾁｪｯｸ
	if( (para_data[CI_NO] < 1) || (para_data[CI_NO] > 12000) ){
		return(1);
	}
	// 有効開始日付ﾁｪｯｸ
	if( ( (para_data[CI_B_MONT] == 0) || (para_data[CI_B_DAY] == 0) )
			||
		( chkdate( para_data[CI_B_YEAR], para_data[CI_B_MONT], para_data[CI_B_DAY] ) != 0 ) ){
		return(1);
	}
	// 有効終了日付ﾁｪｯｸ
	if( ( (para_data[CI_E_MONT] == 0) || (para_data[CI_E_DAY] == 0) )
			||
		( chkdate( para_data[CI_E_YEAR], para_data[CI_E_MONT], para_data[CI_E_DAY] ) != 0 ) ){
		return(1);
	}
	// 処理日付ﾁｪｯｸ
	if( ( (para_data[CI_MONT] == 0) || (para_data[CI_DAY] == 0) )
			||
		( chkdate( (short)CLK_REC.year, para_data[CI_MONT], para_data[CI_DAY] ) != 0 ) ){
		return(1);
	}
	// 処理時刻ﾁｪｯｸ
	if( (para_data[CI_HOUR] > 23) || (para_data[CI_MIN] > 59) ){
		return(1);
	}

	// ﾁｪｯｸ終了（ﾊﾟﾗﾒｰﾀ正常）
	return(0);
}

/*[]----------------------------------------------------------------------[]*/
/*| カード発行：係員カード                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : card_iss_kakari( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
enum {
	CIK_KNO = 0,
	CIK_TYPE,
	CIK_ROLE,
	CIK_LEVEL,
	_CIK_MAX_
};
static ushort	card_iss_kakari(void)
{
	ushort		msg;
	char		prm[_CIK_MAX_];		/* 設定パラメータ */
	int			pos;
	char		mode;
	char		input;
	m_kakari	*outp;
	char		err_flag;
	uchar		str[4];
	uchar		ch[2];
	uchar		type_tbl[] = {0x20, 0x49, 0x4b, 0x53, 0x4a};	/* ' ', 'I', 'K', 'S', 'J' */
	uchar		tbl[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	uchar		move_cmd = OFF;			// ｶｰﾄﾞ取込要求ｺﾏﾝﾄﾞ送信ﾌﾗｸﾞ
	uchar		IdSyuPara;				// 使用ID種設定ﾊﾟﾗﾒｰﾀ
	uchar		sts_bit = 0;			// ディップスイッチチェックビット
	uchar		dipSW_getSts;			// DipSW 設定取得状態

	dipSW_getSts = FRdr_DipSW_GetStatus();		/* 磁気リーダーのDipSW状態を取得 */
	
	OPE_red = 6;
	opr_snd( 90 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞを無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信

	read_sht_opn();						// ｼｬｯﾀｰ「開」

	prm[CIK_KNO] = 1;
	prm[CIK_TYPE] = 2;
	prm[CIK_ROLE] = 1;
	prm[CIK_LEVEL] = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0]);			/* "＜カード発行＞　　　　　　　　" */
	grachr(0, 16, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[10]);		/* "　　　　　　　　係員カード　　" */
	grachr(1, 0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[22]);			/* "係員No.　　　　　　 　　　　　" */
	grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[6]);			/* "カードタイプ　　　　　　　　　" */
	grachr(3, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[20]);			/* "役割　　　　　　　　　　　　　" */
	grachr(4, 0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[15]);			/* "レベル　　　　　　　　　　　　" */
	/* パラメータ表示 */
	cardiss_kakari_prm_dsp(0, prm[0], 1);
	for (pos = 1; pos < _CIK_MAX_; pos++) {
		cardiss_kakari_prm_dsp(pos, prm[pos], 0);
	}
	Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */

	mode = 0;
	pos = 0;
	input = (char)-1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			rd_shutter();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
			if( move_cmd == ON ){
				opr_snd( 90 );						// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
			}	
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg ==  LCD_DISCONNECT ) {
			BUZPI();
			rd_shutter();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
			if( move_cmd == ON ){
				opr_snd( 90 );						// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
			}	
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		/* パラメータ設定中 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				rd_shutter();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				if( move_cmd == ON ){
					opr_snd( 90 );					// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				}
				OPE_red = 2;
				if(( RD_mod != 10 )&&( RD_mod != 11 )){
					opr_snd( 3 );					// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
				}
				return MOD_EXT;
			case KEY_TEN_F1:		/* ▲(F1) */
			case KEY_TEN_F2:		/* ▼(F2) */
				if (input != (char)-1) {
					/* 有効範囲チェック */
					if (cardiss_kakari_prm_chk(pos, input)) {
						BUZPIPI();
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						input = (char)-1;
						break;
					}
					/* パラメータ更新 */
					prm[pos] = input;
				}
				BUZPI();
				/* 表示-正転 */
				cardiss_kakari_prm_dsp(pos, prm[pos], 0);
				if (pos == CIK_TYPE) {
					Fun_Dsp( FUNMSG2[16] );			/* "　▲　　▼　　　　 発行  終了 " */
				}
				/* カーソル移動 */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = (char)(_CIK_MAX_-1);
				}
				else {
					if (++pos >= _CIK_MAX_)
						pos = 0;
				}
				/* 表示-反転 */
				cardiss_kakari_prm_dsp(pos, prm[pos], 1);
				if (pos == CIK_TYPE) {
					Fun_Dsp( FUNMSG2[18] );			/* "　▲　　▼　 切替　発行  終了 " */
					input = prm[pos];
				}
				else {
					input = (char)-1;
				}
				break;
			case KEY_TEN_F3:		/* 切替(F3) */
				if (pos == CIK_TYPE) {
					BUZPI();
					if (input == 2) {	// 2:補充カード
						input = 0;		// 0:係員カード
					}
					else{
						input = 2;		// 2:補充カード
					}
					/* 表示-反転 */
					cardiss_kakari_prm_dsp(pos, input, 1);
				}
				break;
			case KEY_TEN_F4:		/* 発行(F4) */
				if (input != (char)-1) {
					/* 有効範囲チェック */
					if (cardiss_kakari_prm_chk(pos, input)) {
						BUZPIPI();
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						input = (char)-1;
						break;
					}
					/* パラメータ更新 */
					prm[pos] = input;
				}
				BUZPI();
				/* カード取り込み要求 */
				opr_snd(200);
				move_cmd = ON;						// ｶｰﾄﾞ取込要求送信ﾌﾗｸﾞｾｯﾄ
				/* カード発行画面へ遷移 */
				cardiss_kakari_prm_dsp(pos, prm[pos], 0);
				grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
				Fun_Dsp( FUNMSG[82] );				/* "　　　　　　 中止             " */
				mode = 1;
				err_flag = 0;
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				if (pos == CIK_TYPE)
					break;
				BUZPI();
				if (input == (char)-1)
					input = 0;
				if (pos == CIK_KNO)
					input = (char)((input*10 + msg-KEY_TEN0) % 100);
				else
					input = (char)(msg - KEY_TEN0);
				cardiss_kakari_prm_dsp(pos, input, 1);
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				cardiss_kakari_prm_dsp(pos, prm[pos], 1);
				if (pos == CIK_TYPE) {
					input = prm[pos];
				}
				else {
					input = (char)-1;
				}
				break;
			default:
				break;
			}
		}
		else if ( mode == 3 ) {
			/************************************************/
			/*		中止ボタン押下後のカード位置判定		*/
			/************************************************/
			switch ( msg ) {
				case ARC_CR_E_EVT:						/* リーダー動作完了イベント */
					Lagcan( OPETCBNO, 6 );
					mode = 0;
					if(dipSW_getSts == 1){				/* DipSW設定状態取得済み */
						sts_bit = RDS_REC.state[1];		// 磁気リーダー(DipSwitch SW2 の値取得
						sts_bit &= 0x0F;				// 下位4bitのみ有効
						switch(sts_bit){
							case 0x03:					// 海外3:ISO_Track2クレジット対応(上下2方向,逆面取付)
							case 0x04:					// 国内:JISⅡクレジット対応(逆面取付)
							case 0x05:					// ACI向けクレジット対応(上下2方向,逆面取付,HR200 127店舗対応)
								if(RED_REC.posi[0] & 0x07){		// 磁気リーダー奥側（センサー1,2,3）にカードが位置している場合(逆面取付)は、返却コマンドを発行する
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* 状態要求ﾀｲﾏｰ(2s)起動 */
									mode = 4;						// カード返却コマンドの終了待ち状態へ
								}
								break;
							default:
								if ( RED_REC.posi[0] & 0x70){	// 磁気リーダー奥側（センサー7,6,5）にカードが位置している場合(正面取付け)は、返却コマンドを発行する
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* 状態要求ﾀｲﾏｰ(2s)起動 */
									mode = 4;						// カード返却コマンドの終了待ち状態へ
								}
								break;
						}
					}
					break;
				case TIMEOUT6:					/* 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ */
					mode = 0;
					break;
				default:
					break;
			}
		}
		else if ( mode == 4 ) {
			/****************************************/
			/*		カード返却コマンド終了待ち		*/
			/****************************************/
			switch ( msg ) {
			case ARC_CR_E_EVT:				/* リーダー動作完了イベント */
				Lagcan( OPETCBNO, 6 );
				mode = 0;					// パラメータ設定状態へ
				break;
			case TIMEOUT6:					/* 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ */
				mode = 0;					// パラメータ設定状態へ
				break;
			default:
				break;
			}
		}
		/* カード発行中 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 中止(F3) */
				BUZPI();
				if (err_flag) {
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "　　カードを挿入して下さい　　" */
					err_flag = 0;
					/* カード取り込み要求 */
					opr_snd(200);
				}
				else {
					pos = 0;
					cardiss_kakari_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);	/* 行クリア */
					Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */
					input = (char)-1;
					mode = 3;
					opr_snd( 90 );							// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
					Lagtim( OPETCBNO, 6, 2*50 );		/* 状態要求ﾀｲﾏｰ(2s)起動 */
				}
				break;

			case ARC_CR_EOT_RCMD:	/* カード抜き取り */

				if( err_flag ){
					pos = 0;
					cardiss_kakari_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
					Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */
					input = (char)-1;
					mode = 0;
					opr_snd( 90 );							// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				}
				break;

			case ARC_CR_E_EVT:		/* リーダー動作完了イベント */
				/* カード取り込み終了 */
				if (mode == 1) {
					/* 係員カードデータ作成 */
					outp = (m_kakari *)MDP_buf;

					if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// 新カードＩＤ使用する？
						(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APSフォーマットで使用する？

						// 新カードＩＤを使用する場合
						IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,10,1,1 );				// 基本駐車場No.券ID（係員ｶｰﾄﾞ）設定ﾊﾟﾗﾒｰﾀ取得
						switch( IdSyuPara ){

							case	1:													// ID1
								outp->kkr_idc = 0x41;
								break;
							case	2:													// ID2
								outp->kkr_idc = 0x69;
								break;
							case	3:													// ID3
								outp->kkr_idc = 0x6F;
								break;
							case	4:													// ID4
								outp->kkr_idc = 0x75;
								break;
							case	5:													// ID5
								outp->kkr_idc = 0x7B;
								break;
							case	6:													// ID1とID2
								outp->kkr_idc = 0x41;
								break;
							case	7:													// ID1とID3
								outp->kkr_idc = 0x41;
								break;
							case	8:													// ID6
								outp->kkr_idc = 0x7E;
								break;
							case	0:													// 読まない
							default:													// その他（設定値ｴﾗｰ）
								outp->kkr_idc = 0x41;
								break;
						}
					}
					else{
						// 新カードＩＤを使用しない場合
						outp->kkr_idc = 0x41;
					}
					outp->kkr_year[0] = (uchar)(CLK_REC.year%100/10 + 0x30);
					outp->kkr_year[1] = (uchar)(CLK_REC.year%10 + 0x30);
					outp->kkr_mon[0] = (uchar)(CLK_REC.mont/10 + 0x30);
					outp->kkr_mon[1] = (uchar)(CLK_REC.mont%10 + 0x30);
					outp->kkr_day[0] = (uchar)(CLK_REC.date/10 + 0x30);
					outp->kkr_day[1] = (uchar)(CLK_REC.date%10 + 0x30);
					outp->kkr_did = 0x53;
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット
						outp->kkr_park[0] = 0x50;	/* 'P' */
						outp->kkr_park[1] = 0x47;	/* 'G' */
						outp->kkr_park[2] = 0x54;	/* 'T' */
					}else{//APSフォーマット
						outp->kkr_park[0] = 0x50;	/* 'P' */
						outp->kkr_park[1] = 0x41;	/* 'A' */
						outp->kkr_park[2] = 0x4B;	/* 'K' */
					}
					outp->kkr_role = (uchar)(prm[CIK_ROLE] + 0x30);
					outp->kkr_lev = (uchar)(prm[CIK_LEVEL] + 0x30);
					outp->kkr_type = type_tbl[prm[CIK_TYPE]];
					outp->kkr_kno[0] = 0x30;
					outp->kkr_kno[1] = 0x30;
					outp->kkr_kno[2] = (uchar)(prm[CIK_KNO]/10 + 0x30);
					outp->kkr_kno[3] = (uchar)(prm[CIK_KNO]%10 + 0x30);
					memset(&outp->kkr_jdg, 0x30, sizeof(outp->kkr_jdg));
					memset(outp->kkr_rsv1, 0x20, sizeof(outp->kkr_rsv1));
					memset(outp->kkr_rsv2, 0x20, sizeof(outp->kkr_rsv2));
					memset(outp->kkr_rsv3, 0x20, sizeof(outp->kkr_rsv3));
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット
						outp->kkr_rsv2[4] = (uchar)(CPrmSS[S_SYS][1]/100000 + 0x30);
						outp->kkr_rsv2[5] = (uchar)(CPrmSS[S_SYS][1]%100000/10000 + 0x30);
						outp->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]%10000/1000 + 0x30);
						outp->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
						outp->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
						outp->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
						MDP_buf[127] = 1;
					}else{//APSフォーマット
						outp->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]/1000 + 0x30);
						outp->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
						outp->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
						outp->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
						MDP_buf[127] = 0;
					}
					MDP_mag = sizeof( m_kakari );
					MDP_siz = sizeof(m_kakari);
					md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 1);								/* パリティ作成(偶数ﾊﾟﾘﾃｨ) */
					/* 係員カード書き込み */
					opr_snd(202);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[8]);		/* "　　　　＝書込中です＝　　　　" */
					mode = 2;
				}
				/* カード書き込み終了 */
				else {
					/* 書込ＮＧ */
					if (RED_REC.ercd) {
						ch[0] = tbl[RED_REC.ercd >> 4 & 0x0F];
						ch[1] = tbl[RED_REC.ercd & 0x0F];
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[7]);		/* "　　　　書込ＮＧ（Ｅ　　）　　" */
						as1chg(ch, str, 2);
						grachr(6, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str);			/* エラーコード */
						err_flag = 1;
						mode = 1;
					}
					/* 書込ＯＫ */
					else {
						pos = 0;
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
						Fun_Dsp( FUNMSG2[16] );					/* "　▲　　▼　　　　 発行  終了 " */
						input = (char)-1;
						mode = 0;
						opr_snd( 90 );							// ｶｰﾄﾞ取込要求を無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 係員カード－パラメータ表示                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_kakari_prm_dsp( pos, prm, rev )                 |*/
/*| PARAMETER    : char   pos : 表示項目番号                               |*/
/*|              : char   prm : データ内容                                 |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	cardiss_kakari_prm_dsp(char pos, char prm, ushort rev)
{
	switch (pos) {
	case CIK_KNO:
		opedsp(1, 7, (ushort)prm, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );		/* 係員No. */
		break;
	case CIK_TYPE:
		if (prm == 0) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[10]);		/* "係員カード" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　" */
		}
		else if (prm == 1) {
			grachr(2, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[5]);		/* "インベントリ" */
			grachr(2, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　" */
		}
		else if (prm == 2) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[11]);		/* "補充カード" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　" */
		}
		else if (prm == 3) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[12]);		/* "小計カード" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　" */
		}
		else {
			grachr(2, 14, 16, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_1[1]);		/* "自動棚卸しカード" */
		}
		break;
	case CIK_ROLE:
		opedsp(3, 8, (ushort)prm, 1, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 役割 */
		break;
	default:	/* case CIK_LEVEL: */
		opedsp(4, 8, (ushort)prm, 1, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* レベル */
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 係員カード－パラメータチェック                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_kakari_prm_chk( pos, prm )                      |*/
/*| PARAMETER    : char   pos : 表示項目番号                               |*/
/*|              : char   prm : データ内容                                 |*/
/*| RETURN VALUE : 0:OK                                                    |*/
/*|              : 1:NG                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	cardiss_kakari_prm_chk(char pos, char prm)
{
	switch (pos) {
	case CIK_KNO:
		if (prm < 1){
			return 1;
		}
		break;
	case CIK_TYPE:
		if (prm > 4){
			return 1;
		}
		break;
	case CIK_ROLE:
		if (prm < 1 || prm > 3){
			return 1;
		}
		break;
	case CIK_LEVEL:
		if (prm > 4){
			return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気リーダーDipSW設定取得処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdr_DipSW_GetStatus()   			                   |*/
/*| PARAMETER    : void 					            	               |*/
/*| RETURN VALUE : uchar	dipSW_getSts                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.takeuchi                                              |*/
/*| Date         : 2012/10/19                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	FRdr_DipSW_GetStatus( void )
{
	
	short		msg;
	uchar dipSWSts;

	dipSWSts = 0;

	if( opr_snd( 95 ) == 0 ){ 				/* 状態要求 */
		Lagtim( OPETCBNO, 6, 2*50 );		/* 状態要求ﾀｲﾏｰ(2s)起動 */
	}
	else{
		return dipSWSts;
	}
	
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );		/* ﾒｯｾｰｼﾞ受信 */

		switch( msg){						/* 受信ﾒｯｾｰｼﾞ？ */
			case ARC_CR_E_EVT:				/* 終了ｺﾏﾝﾄﾞ受信 */
				Lagcan( OPETCBNO, 6 );
				if(RDS_REC.result == 0){
					dipSWSts = 1;			/* DipSW設定状態取得完了 */
				}
			case TIMEOUT6:					/* 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ */
				return dipSWSts;
			default:
				break;
		}
	}
	return dipSWSts;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータポインタ取得処理（クレジット利用明細プリント用）			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet_M( req )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req	= 処理要求								|*/
/*|										０：最古ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|										１：最新ﾃﾞｰﾀﾎﾟｲﾝﾀ取得				|*/
/*|					:	uchar	type = 明細タイプ							|*/
/*|							CREDIT_CARD/CREDIT_iD/...（以下追加）			|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ（配列番号）				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Yanase(COSMO)												|*/
/*|	Date	:	2006-07-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
ushort	LogPtrGet_M( uchar req, uchar type )
{
	ushort	posi = 0;
	return(posi);
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ件数表示画面処理３（印字対象期間指定あり）
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ログ種別
///	@param[in]		LogCnt	: ログ件数
///	@param[in]		title	: タイトル表示データポインタ
///	@param[in]		PreqCmd	: 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
///	@param[in]		NewOldDate	: 最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@return			操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)
///	@note			SysMnt_Log_CntDsp2()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
#define		_POS_MAX	6		// カーソル移動MAX値
#define		_POS_SYEAR	0		// 開始"年"位置
#define		_POS_EYEAR	3		// 終了"年"位置
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ１（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA_TIME1[_POS_MAX][4] = {
	{ 4,  6, 2 ,1 },	// 開始（年）
	{ 4, 12, 2 ,0 },	// 開始（月）
	{ 4, 18, 2, 0 },	// 開始（日）
	{ 5,  6, 2, 1 },	// 終了（年）
	{ 5, 12, 2, 0 }, 	// 終了（月）
	{ 5, 18, 2, 0 },	// 終了（日）
};

//[]----------------------------------------------------------------------[]
///	@brief			ログ件数表示画面処理４（印字対象期間指定あり）
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ログ種別
///	@param[in]		title	: タイトル表示データポインタ
///	@param[in]		PreqCmd	: 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
///	@return			操作ﾓｰﾄﾞ(MOD_CHG/MOD_EXT)
///	@note			SysMnt_Log_CntDsp2_Time()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Ise
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	SysMnt_Log_CntDsp3(ushort LogSyu, const uchar *title, ushort PreqCmd)
{
	uchar			pos		= 0;			// 期間ﾃﾞｰﾀ入力位置（０：開始年、１：開始月、２：開始日、３：開始時、
											//					 ４：終了年、５：終了月、６：終了日、７：終了時）
	uchar			All_Req	= OFF;			// 「全て」指定中ﾌﾗｸﾞ
	uchar			Date_Chk;				// 日付指定ﾃﾞｰﾀﾁｪｯｸﾌﾗｸﾞ
	short			inp		= -1;			// 入力ﾃﾞｰﾀ
	ushort			RcvMsg;					// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
	ushort			pri_cmd	= 0;			// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	ushort			NewOldDate[_POS_MAX];	// 日付ﾃﾞｰﾀ（[0]：開始年、[1]：開始月、[2]：開始日、[3]：開始時、
	ushort			Date_Now[_POS_MAX];		// 			 [4]：終了年、[5]：終了月、[6]：終了日、[7]：終了時）
	ushort			Sdate;					// 開始日付
	ushort			Edate;					// 終了日付
	ushort			wks, wks2;
	ushort			LogMax, LogCnt;		// Log最大件数
	ushort			FstIdx = 0;			// FlashROM検索で最初に一致した番号
	ushort			LstIdx = 0;			// FlashROM検索で最後に一致した番号
	date_time_rec	NewestDateTime;		// 条件に一致した情報の最も新しい精算日時
	date_time_rec	OldestDateTime;		// 条件に一致した情報の最も古い精算日時
	T_FrmLogPriReq4	FrmLogPriReq4;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ(標準用)
	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			FstIdxAll;			// FlashROM検索で最初に一致した番号(全て釦用)
	ushort			LstIdxAll;			// FlashROM検索で最後に一致した番号(全て釦用)
	ushort			i;					// ループ変数
// MH321800(S) D.Inaba ICクレジット対応
	ulong			start_time;
	ulong			past_time;
	
	start_time = LifeTimGet();
// MH321800(E) D.Inaba ICクレジット対応

	
	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
	LogMax = Ope2_Log_NewestOldestDateGet_AttachOffset( LogSyu, &NewestDateTime, &OldestDateTime, &FstIdx, &LstIdx);
// MH321800(S) D.Inaba ICクレジット対応
	// ログ検索中画面を最低1秒間表示
	switch (LogSyu) {
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	case LOG_ECMINASHI:
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
	case LOG_ECMEISAI:
		past_time = LifePastTimGet( start_time );
		if (past_time < 100UL) {
			// ファンクションキーが表示される前のため内容なしで表示しておく
			Fun_Dsp( FUNMSG[0] );											// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　　　　　　　　　　"
			Lagtim(OPETCBNO, 6, (ushort)((1000UL - (past_time*10)) / 20));	// 操作ｶﾞｰﾄﾞﾀｲﾏｰ起動
			for( ;; ){
				RcvMsg = StoF(GetMessage(), 1 );
				if (RcvMsg == TIMEOUT6) {
					// 操作ｶﾞｰﾄﾞﾀｲﾏｰﾀｲﾑｱｳﾄ
					break;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				if (RcvMsg == KEY_MODECHG) {
				if (RcvMsg == KEY_MODECHG || RcvMsg == LCD_DISCONNECT) {	// モードチェンジもしくは切断通知
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return RcvMsg;
				}
			}
		}
		break;
	default:
		break;
	}
// MH321800(E) D.Inaba ICクレジット対応
	
	// 最古＆最新日付ﾃﾞｰﾀ取得(両者の年月日を結合して第三引数に格納)
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
	FstIdxAll = FstIdx;					// 全て釦用変数に最初一致番号を反映
	LstIdxAll = LstIdx;					// 全て釦用変数に最後一致番号を反映


	LogCnt = LogMax;										// Log件数適用
	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );							// ログ情報プリントタイトル表示
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// 最古／最新ログ日付表示
	LogCntDsp( LogCnt );									// ログ件数表示

	LogDateDsp4( &Date_Now[0] );							// 最古／最新ログ日付データ表示
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
	LogDateDsp5( &Date_Now[0], pos );						// 印刷日付データ表示

	Fun_Dsp( FUNMSG[83] );									// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		if( pri_cmd == 0 ){

			// 印字要求前（印字要求前画面）

			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替

					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				case KEY_TEN:								// ０～９

					if( All_Req == OFF ){					// 「全て」指定中？（指定中は、無視する）

						//	「全て」指定中でない場合

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// 入力ﾃﾞｰﾀ表示
									POS_DATA5_1[pos][0],	// 表示行
									POS_DATA5_1[pos][1],	// 表示ｶﾗﾑ
									(ushort)inp,			// 表示ﾃﾞｰﾀ
									POS_DATA5_1[pos][2],	// ﾃﾞｰﾀ桁数
									POS_DATA5_1[pos][3],	// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// 取消キー

					BUZPI();

					if( All_Req == OFF ){					// 「全て」指定中？

						//	「全て」指定中でない場合

						opedsp	(							// 入力前のﾃﾞｰﾀを表示させる
									POS_DATA5_1[pos][0],	// 表示行
									POS_DATA5_1[pos][1],	// 表示ｶﾗﾑ
									Date_Now[pos],			// 表示ﾃﾞｰﾀ
									POS_DATA5_1[pos][2],	// ﾃﾞｰﾀ桁数
									POS_DATA5_1[pos][3],	// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// 入力状態初期化
					}
					else{
						//	「全て」指定中の場合

						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
						inp = -1;							// 入力状態初期化
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
						LogDateDsp5( &Date_Now[0], pos );						// 印刷日付データ表示
						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
					}

					break;

				case KEY_TEN_F1:							// Ｆ１（←）キー押下
				case KEY_TEN_F2:							// Ｆ２（→）キー押下	※日付入力中の場合
															// Ｆ２（クリア）押下	※「全て」指定中の場合

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){						// 「全て」指定中？

						//	「全て」指定中でない場合

						if( inp == -1 ){						// 入力あり？

							//	入力なしの場合
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
							}
							else{
								LogDatePosUp( &pos, 1 );		// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
							}
						}
						else{
							//	入力ありの場合
							if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ？

								//	入力ﾃﾞｰﾀＯＫの場合
								BUZPI();

								if( pos == 0 || pos == 3){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
								}
								else{
									LogDatePosUp( &pos, 1 );	// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
								}
							}
							else{
								//	入力ﾃﾞｰﾀＮＧの場合
								BUZPIPI();
							}
						}
						/* 指定年月日範囲内のﾃﾞｰﾀ数再表示 */
						LogDateDsp5( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
						inp = -1;								// 入力状態初期化
					}

					break;

				case KEY_TEN_F3:							// Ｆ３（全て）キー押下

					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){						// 「全て」指定中？（指定中は、無視する）

						if( LogCnt != 0 ){

							//	ログデータがある場合
							BUZPI();
							memcpy	(							// 最古＆最新ﾃﾞｰﾀ日付をｺﾋﾟｰ
										&Date_Now[0],
										NewOldDate,
										12
									);
							FstIdx = FstIdxAll;					// 最初一致番号を反映
							LstIdx = LstIdxAll;					// 最後一致番号を反映
							pos = 0;							// ｶｰｿﾙ位置＝開始（年）
							inp = -1;							// 入力状態初期化
							displclr( 5 );						// 印刷日付表示クリア
							LogCntDsp( LogCnt );				// ログ件数表示
							Fun_Dsp( FUNMSG[81] );				// "　　　　　　　　　 実行  終了 "
							All_Req = ON;						// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
						}
						else{
							//	ログデータがない場合
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// Ｆ４（実行）キー押下

					displclr( 1 );							// 1行目表示クリア
					Date_Chk = OK;

					if( inp != -1 ){						// 入力あり？

						//	入力ありの場合
						if( OK == LogDateChk2( pos, inp ) ){// 入力ﾃﾞｰﾀＯＫ？

							//	入力データＯＫの場合
							if( pos == 0 || pos == 3){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}
						else{
							//	入力データＮＧの場合
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// 「全て」指定でない場合、年月日ﾁｪｯｸを行う

							//	年月日が存在する日付かチェックする

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// 開始日付ﾁｪｯｸ
								Date_Chk = NG;
							}
						}
					}
					if( ( All_Req == ON ) && ( Date_Chk == OK ) ){

						//	開始日付＜＝終了日付かチェックする

						Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
											(short)Date_Now[_POS_SYEAR],
											(short)Date_Now[_POS_SYEAR+1],
											(short)Date_Now[_POS_SYEAR+2]
										);

						Edate = dnrmlzm(							// 終了日付ﾃﾞｰﾀ取得
											(short)Date_Now[_POS_EYEAR],
											(short)Date_Now[_POS_EYEAR+1],
											(short)Date_Now[_POS_EYEAR+2]
										);

						if( Sdate > Edate ){	// 開始日付／終了日付ﾁｪｯｸ
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
											(short)Date_Now[_POS_SYEAR],
											(short)Date_Now[_POS_SYEAR+1],
											(short)Date_Now[_POS_SYEAR+2]
										);
						wks = 0;
//						index = Ope_Log_TotalCountGet(eLOG_PAYMENT);
						if( All_Req == OFF ){
							wks2 = Ope2_Log_CountGet_inDate( eLOG_PAYMENT, &Date_Now[0], &FstIdx );
							for(i = 0; i < wks2; i++)					/* FlashROMからデータを読み出すループ */
							{
								// 個別精算データ１件読出し
								Ope_Log_1DataGet(eLOG_PAYMENT, (ushort)(FstIdx + i), SysMnt_Work);	

// MH321800(S) D.Inaba ICクレジット対応（バグ修正移植）
//								if(is_match_receipt((Receipt_data*)&SysMnt_Work, Sdate, 0, LOG_SCAMEISAI))
								if(is_match_receipt((Receipt_data*)&SysMnt_Work, Sdate, 0, LogSyu))
// MH321800(E) D.Inaba ICクレジット対応（バグ修正移植）
								{
									wks++;
								}
							}
							if( wks == 0 ){
								BUZPIPI();
								grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// データなし表示
								inp = -1;											// 入力状態初期化
								break;
							}
						}
//						LogCntDsp( wks );		// ログ件数表示

						if (Ope_isPrinterReady() == 0) {		// レシート出力不可
							BUZPIPI();
							break;
						}
						// 開始日付＆終了日付ﾁｪｯｸＯＫの場合
						BUZPI();
						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
						memset( &FrmLogPriReq4,0,sizeof(FrmLogPriReq4) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
						FrmLogPriReq4.prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
						FrmLogPriReq4.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
						FrmLogPriReq4.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.
						if( All_Req == ON ){
							FrmLogPriReq4.BType	= 0;							// 検索方法	：全て
							FrmLogPriReq4.LogCount = LogMax;	// LOG登録件数(個別精算LOG・集計LOGで使用)
							FrmLogPriReq4.Ffst_no		= 0;
						}
						else {
							FrmLogPriReq4.BType	= 1;							// 検索方法	：日付
							FrmLogPriReq4.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq4.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq4.TSttTime.Day	= (uchar)Date_Now[2];
							FrmLogPriReq4.LogCount = wks;	// LOG登録件数(個別精算LOG・集計LOGで使用)
							FrmLogPriReq4.Ffst_no		= FstIdx;
						}
						FrmLogPriReq4.Flst_no		= LstIdx;
						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq4), &FrmLogPriReq4 );
						Ope_DisableDoorKnobChime();
						pri_cmd = PreqCmd;					// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
						if( All_Req != ON ){					// 「全て」指定なし
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
							LogDateDsp5( &Date_Now[_POS_SYEAR], 0xff );					// 印刷日付データ表示
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// 実行中ブリンク表示
						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"

					}
					else{
						// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
						BUZPIPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
						LogDateDsp5( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
						Fun_Dsp( FUNMSG[83] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
						All_Req = OFF;							// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
						inp = -1;								// 入力状態初期化
					}

					break;

				case KEY_TEN_F5:							// Ｆ５（終了）キー押下

					BUZPI();
					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
					RcvMsg = MOD_EXT;						// YES：前画面に戻る
			}

			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					RcvMsg = MOD_CHG;						// 前画面に戻る
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();
					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// 前画面に戻る

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
			break;											// 前画面に戻る
		}
	}
	return( RcvMsg );
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ件数表示画面の印字処理で対象プリンタを判断
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ログ種別
///	@return			プリンタ種別(J_PRI/R_PRI/RJ_PRI/0)
///	@note			SysMnt_Log_CntDspxx()関数共通用<br>
///					各種別で判断対象となる値が異常な場合は0を返す。<br>
//[]----------------------------------------------------------------------[]
///	@author			Ise
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static uchar	SysMnt_Log_Printer(ushort LogSyu)
{
	uchar prn_kind;											// ﾌﾟﾘﾝﾀ種別

	switch(LogSyu)											// 種別毎にプリンタ種別を判断
	{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case LOG_EDYSYUUKEI:									// Ｅｄｙ集計
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case LOG_SCASYUUKEI:									// Ｓｕｉｃａ集計
		switch(prm_get(COM_PRM, S_SCA, 100, 1, 1))			// 35-0100⑥:日毎集計印字出力先
		{
		case 3:												// 印字先がレシートとジャーナル
			if((Ope_isJPrinterReady() == 1) && (Ope_isPrinterReady() == 1))
			{												// レシート・ジャーナルが印字可能な状態
				prn_kind	= RJ_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ&ｼﾞｬｰﾅﾙ
				break;
			}
			if(Ope_isJPrinterReady() == 1)					// ジャーナルプリンタが印字可能な状態
			{
				prn_kind	= J_PRI;						// 対象ﾌﾟﾘﾝﾀ：ｼﾞｬｰﾅﾙ
				break;
			}												// 上記以外時は下のcase文でレシートの可否を判断
		case 2:												// 印字先がレシート
			if(Ope_isPrinterReady() == 1)					// レシートプリンタが印字可能な状態
			{
				prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
			}
			else											// 印字不可(紙切れ)
			{
				prn_kind	= 0;							// 対象ﾌﾟﾘﾝﾀ：なし
			}
			break;
		case 1:												// 印字先がジャーナル
			if(Ope_isJPrinterReady() == 1)					// ジャーナルプリンタが印字可能な状態
			{
				prn_kind	= J_PRI;						// 対象ﾌﾟﾘﾝﾀ：ｼﾞｬｰﾅﾙ
			}
			else
			{
				prn_kind	= 0;							// 対象ﾌﾟﾘﾝﾀ：なし
			}
			break;
		case 0:												// 印字先設定なし
		default:											// 規格外の設定(異常)
			prn_kind	= 0;								// 対象ﾌﾟﾘﾝﾀ：なし
			break;
		}
		break;
// MH321800(S) D.Inaba ICクレジット対応
	case LOG_ECSYUUKEI:										// 決済リーダ集計
		switch(prm_get(COM_PRM, S_ECR, 4, 1, 1))			// 50-004⑥:日毎集計印字出力先
		{
		case 3:												// 印字先がレシートとジャーナル
			if((Ope_isJPrinterReady() == 1) && (Ope_isPrinterReady() == 1))
			{												// レシート・ジャーナルが印字可能な状態
				prn_kind	= RJ_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ&ｼﾞｬｰﾅﾙ
				break;
			}
			if(Ope_isJPrinterReady() == 1)					// ジャーナルプリンタが印字可能な状態
			{
				prn_kind	= J_PRI;						// 対象ﾌﾟﾘﾝﾀ：ｼﾞｬｰﾅﾙ
				break;
			}												// 上記以外時は下のcase文でレシートの可否を判断
		case 2:												// 印字先がレシート
			if(Ope_isPrinterReady() == 1)					// レシートプリンタが印字可能な状態
			{
				prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
			}
			else											// 印字不可(紙切れ)
			{
				prn_kind	= 0;							// 対象ﾌﾟﾘﾝﾀ：なし
			}
			break;
		case 1:												// 印字先がジャーナル
			if(Ope_isJPrinterReady() == 1)					// ジャーナルプリンタが印字可能な状態
			{
				prn_kind	= J_PRI;						// 対象ﾌﾟﾘﾝﾀ：ｼﾞｬｰﾅﾙ
			}
			else
			{
				prn_kind	= 0;							// 対象ﾌﾟﾘﾝﾀ：なし
			}
			break;
		case 0:												// 印字先設定なし
		default:											// 規格外の設定(異常)
			prn_kind	= 0;								// 対象ﾌﾟﾘﾝﾀ：なし
			break;
		}
		break;
// MH321800(E) D.Inaba ICクレジット対応
	default:
		if(Ope_isPrinterReady() == 1)						// レシートプリンタが印字可能な状態
		{
			prn_kind	= R_PRI;							// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
		}
		else												// 印字不可(紙切れ)
		{
			prn_kind	= 0;								// 対象ﾌﾟﾘﾝﾀ：なし
		}
		break;
	}

	return prn_kind;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ日付位置番号更新処理("時間"追加)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	pos		: 位置番号ﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[in]		req		: 処理要求
///								０：位置番号－１（左移動）
///								１：位置番号＋１（右移動）
///	@return			uchar	: 行移動判定(0:なし/1:あり)
///	@note			LogDatePosUp()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			ISE
///	@date			Create	: 20098/02/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	LogDatePosUp2( uchar *pos, uchar req )
{
	uchar ret = 0;					/* 行移動判定(0:なし/1:あり) */

	if( req == 0 ){

		// 位置番号－１（左移動）
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = _POS_MAX-1;
		}
		if((*pos == 3) || (*pos == (_POS_MAX-1)))
		{
			ret = 1;				/* 行移動あり */
		}
	}
	else{
		// 位置番号＋１（右移動）
		*pos = (uchar)((*pos) + 1);
		if( *pos >= _POS_MAX ){
			*pos = 0;
		}
		if((*pos == 0) || (*pos == (_POS_MAX/2)))
		{
			ret = 1;				/* 行移動なし */
		}
	}
	return ret;
}
#define		_POS_MAX_IO	10		// カーソル移動MAX値
//[]----------------------------------------------------------------------[]
///	@brief			ログ日付位置番号更新処理("分"追加)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	pos		: 位置番号ﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[in]		req		: 処理要求
///								０：位置番号－１（左移動）
///								１：位置番号＋１（右移動）
///	@return			uchar	: 行移動判定(0:なし/1:あり)
///	@note			LogDatePosUp2()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			ISE
///	@date			Create	: 2009/06/12<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	LogDatePosUp3( uchar *pos, uchar req )
{
	uchar ret = 0;					/* 行移動判定(0:なし/1:あり) */

	if( req == 0 ){

		// 位置番号－１（左移動）
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = _POS_MAX_IO-1;
		}
		if((*pos == ((_POS_MAX_IO/2)-1)) || (*pos == (_POS_MAX_IO-1)))
		{
			ret = 1;				/* 行移動あり */
		}
	}
	else{
		// 位置番号＋１（右移動）
		*pos = (uchar)((*pos) + 1);
		if( *pos >= _POS_MAX_IO ){
			*pos = 0;
		}
		if((*pos == 0) || (*pos == (_POS_MAX_IO/2)))
		{
			ret = 1;				/* 行移動なし */
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			最古＆最新ログデータ日付データ取得処理("時間"追加)
//[]----------------------------------------------------------------------[]
///	@param[in]		Old		: 最古ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[in]		New		: 最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[out]		Date	: 日付ﾃﾞｰﾀ格納ﾎﾟｲﾝﾀ
///	@return			void
///	@note			LogDateGet()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	LogDateGet2(date_time_rec *Old, date_time_rec *New, ushort *Date)
{
	// 最古ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ
	*Date = Old->Year;
	Date++;
	*Date = Old->Mon;
	Date++;
	*Date = Old->Day;
	Date++;
	*Date = Old->Hour;
	Date++;
	// 最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ
	*Date = New->Year;
	Date++;
	*Date = New->Mon;
	Date++;
	*Date = New->Day;
	Date++;
	*Date = New->Hour;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ印字開始／終了日付ﾃﾞｰﾀ表示処理
//[]----------------------------------------------------------------------[]
///	@param[in]		Date	: 日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
///	@param[in]		Rev_Pos	: 反転表示位置<br>
///								０～５：指定位置反転表示<br>
///								その他：反転表示なし
///	@return			void
///	@note			LogDateDsp()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ２（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA_TIME2[_POS_MAX][4] = {
	{ 4,  6, 2, 1 },	// 開始（年）
	{ 4, 12, 2, 0 },	// 開始（月）
	{ 4, 18, 2, 0 },	// 開始（日）
	{ 5,  6, 2, 1 },	// 終了（年）
	{ 5, 12, 2, 0 }, 	// 終了（月）
	{ 5, 18, 2, 0 },	// 終了（日）
};

void	LogDateDsp2( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// 日付ﾃﾞｰﾀ表示位置
	ushort	rev_data;		// 反転表示ﾃﾞｰﾀ


	for( pos = 0 ; pos < _POS_MAX ; pos++){		// 日付ﾃﾞｰﾀ表示：開始（年）～終了（時）
		opedsp	(
					POS_DATA_TIME2[pos][0],		// 表示行
					POS_DATA_TIME2[pos][1],		// 表示ｶﾗﾑ
					*Date,						// 表示ﾃﾞｰﾀ
					POS_DATA_TIME2[pos][2],		// ﾃﾞｰﾀ桁数
					POS_DATA_TIME2[pos][3],		// ０サプレス有無
					0,							// 反転表示：なし
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;				// 反転表示ﾃﾞｰﾀｾｰﾌﾞ
		}

		Date++;									// 日付ﾃﾞｰﾀﾎﾟｲﾝﾀ更新
	}
	if( Rev_Pos <= _POS_MAX-1 ){				// 反転表示指定あり？

		opedsp	(								// 指定位置を反転表示させる
					POS_DATA_TIME1[Rev_Pos][0],	// 表示行
					POS_DATA_TIME1[Rev_Pos][1],	// 表示ｶﾗﾑ
					rev_data,					// 表示ﾃﾞｰﾀ
					POS_DATA_TIME1[Rev_Pos][2],	// ﾃﾞｰﾀ桁数
					POS_DATA_TIME1[Rev_Pos][3],	// ０サプレス有無
					1,							// 反転表示：あり
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ログ印字開始／終了日付ﾃﾞｰﾀ表示処理３（入出庫情報プリント用）		|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp3( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	日付ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	反転表示位置					|*/
/*|											０～５：指定位置反転表示		|*/
/*|											その他：反転表示なし			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
// 表示項目毎の表示方法ﾃﾞｰﾀﾃｰﾌﾞﾙ３（表示行、表示ｶﾗﾑ、表示桁数、０サプレス有無）
const	ushort	POS_DATA31[10][4] = {
	{ 3, 12, 2, 1 },	// 開始（年）
	{ 3, 18, 2, 0 },	// 開始（月）
	{ 3, 24, 2, 0 },	// 開始（日）
	{ 4, 20, 2, 0 },	// 開始（時）
	{ 4, 26, 2, 1 },	// 開始（分）
	{ 5, 12, 2, 1 },	// 終了（年）
	{ 5, 18, 2, 0 }, 	// 終了（月）
	{ 5, 24, 2, 0 },	// 終了（日）
	{ 6, 20, 2, 0 },	// 終了（時）
	{ 6, 26, 2, 1 }		// 終了（分）
};
const	ushort	POS_DATA32[10][4] = {
	{ 3,  8, 4, 1 },	// 開始（年）
	{ 3, 18, 2, 0 },	// 開始（月）
	{ 3, 24, 2, 0 },	// 開始（日）
	{ 4, 20, 2, 0 },	// 開始（時）
	{ 4, 26, 2, 1 },	// 開始（分）
	{ 5,  8, 4, 1 },	// 終了（年）
	{ 5, 18, 2, 0 }, 	// 終了（月）
	{ 5, 24, 2, 0 },	// 終了（日）
	{ 6, 20, 2, 0 },	// 終了（時）
	{ 6, 26, 2, 1 }		// 終了（分）
};

void	LogDateDsp3( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// 日付ﾃﾞｰﾀ表示位置
	ushort	rev_data;		// 反転表示ﾃﾞｰﾀ


	for( pos = 0 ; pos < 10 ; pos++){		// 日付ﾃﾞｰﾀ表示：開始（年）～終了（分）

		opedsp	(
					POS_DATA32[pos][0],		// 表示行
					POS_DATA32[pos][1],		// 表示ｶﾗﾑ
					*Date,					// 表示ﾃﾞｰﾀ
					POS_DATA32[pos][2],		// ﾃﾞｰﾀ桁数
					POS_DATA32[pos][3],		// ０サプレス有無
					0,						// 反転表示：なし
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;			// 反転表示ﾃﾞｰﾀｾｰﾌﾞ
		}

		Date++;								// 日付ﾃﾞｰﾀﾎﾟｲﾝﾀ更新
	}
	if( Rev_Pos <= 9 ){						// 反転表示指定あり？

		opedsp	(							// 指定位置を反転表示させる
					POS_DATA31[Rev_Pos][0],	// 表示行
					POS_DATA31[Rev_Pos][1],	// 表示ｶﾗﾑ
					rev_data,				// 表示ﾃﾞｰﾀ
					POS_DATA31[Rev_Pos][2],	// ﾃﾞｰﾀ桁数
					POS_DATA31[Rev_Pos][3],	// ０サプレス有無
					1,						// 反転表示：あり
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ日付データチェック処理("時間"追加)
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: 日付位置番号ﾃﾞｰﾀ
///	@param[in]		data	: 日付ﾃﾞｰﾀ
///	@return			ret		: ﾁｪｯｸ結果(OK/NG)
///	@note			LogDateChk()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// 項目毎のﾃﾞｰﾀ範囲ﾁｪｯｸﾃﾞｰﾀﾃｰﾌﾞﾙ（最小値、最大値）
const	short	LOG_DATE_DATA2[_POS_MAX][2] = {
	{ 0, 99 },	// 開始（年）
	{ 1, 12 },	// 開始（月）
	{ 1, 31 },	// 開始（日）
	{ 0, 99 },	// 終了（年）
	{ 1, 12 },	// 終了（月）
	{ 1, 31 },	// 終了（日）
};
uchar	LogDateChk2( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA2[pos][0] && data <= LOG_DATE_DATA2[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ日付データチェック処理("分"追加)
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: 日付位置番号ﾃﾞｰﾀ
///	@param[in]		data	: 日付ﾃﾞｰﾀ
///	@return			ret		: ﾁｪｯｸ結果(OK/NG)
///	@note			LogDateChk()を参照<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2009/10/29<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
// 項目毎のﾃﾞｰﾀ範囲ﾁｪｯｸﾃﾞｰﾀﾃｰﾌﾞﾙ（最小値、最大値）
const	short	LOG_DATE_DATA3[_POS_MAX_IO][2] = {
	{ 0, 99 },	// 開始（年）
	{ 1, 12 },	// 開始（月）
	{ 1, 31 },	// 開始（日）
	{ 0, 23 },	// 開始（時）
	{ 0, 59 },	// 開始（分）
	{ 0, 99 },	// 終了（年）
	{ 1, 12 },	// 終了（月）
	{ 1, 31 },	// 終了（日）
	{ 0, 23 },	// 終了（時）
	{ 0, 59 },	// 終了（分）
};
uchar	LogDateChk_Range_Minute( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA3[pos][0] && data <= LOG_DATE_DATA3[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*
 *| transfer file (serial interface & FTP connection)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_FileTransfer
 *| PARAMETER    : none
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort	SysMnt_FileTransfer(void)
{
	ushort	usSysEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for ( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[4] );		/* "＜接続方式選択＞　　　　　　　" */
		Fun_Dsp(FUNMSG[25]);					/* "　▲　　▼　　　　 読出  終了 " */
		usSysEvent = Menu_Slt(FLTRFMENU, FILE_TRF_TBL, (char)FL_TRF_MAX, (char)1);

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usSysEvent) {
		/* 2.FTP接続(クライアント) */
		case FILE_FTP_CLI:
			BUZPIPI();
			break;
		/* 3.FTP接続(サーバー) */
		case FILE_FTP_SEV:
			Ope_DisableDoorKnobChime();
			usSysEvent = SysMnt_FTPServer();
			break;
		case MOD_EXT:
			return MOD_EXT;
		default:
			break;
		}
		if (usSysEvent == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (usSysEvent == MOD_CUT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

// 表示項目毎のF3キー入力無効/有効制御テーブル
static const uchar RT_KEY_TEN_F3_valid_tbl[RT_MODE_MAX][RT_SUB_MODE_MAX][18] = {
		// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17		<=表示項目No.
// MH810100(S) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)			
//		{{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 4, 4, 4, 4, 4},	// mode=0,	sub_mode=0
		{{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 4, 4, 4, 4, 4},	// mode=0,	sub_mode=0
// MH810100(E) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)	
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},	// 			sub_mode=3
		{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// mode=1,	sub_mode=0
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}	// 			sub_mode=3
	};																// 0 = 無効
																	// 1 = 有効	画面切換(→共通パラメータ設定画面)
																	// 2 = 有効	表示切換(料金種別)
																	// 3 = 有効	画面切換(→定期券情報入力画面)
																	// 4 = 有効	画面切換(→掛売先情報入力画面)
																	// 5 = 有効	表示切換(サービス券種別)

// 表示項目毎のテンキー入力無効/有効制御テーブル
static const uchar RT_KEY_TEN_valid_tbl[RT_MODE_MAX][RT_SUB_MODE_MAX][18] = {
		// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17		<=表示項目No.
// MH810100(S) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)	
//		{{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 0, 3, 1, 3, 1, 3, 1},	// mode=0,	sub_mode=0
		{{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 3, 1, 3, 1, 3, 1},	// mode=0,	sub_mode=0
// MH810100(E) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)			
		 { 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 3, 1, 3, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},	// 			sub_mode=3
		{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// mode=1,	sub_mode=0
		 { 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}	// 			sub_mode=3
	};																// 0 = 無効
																	// 1 = 有効	1桁表示項目
																	// 2 = 有効	2桁表示項目
																	// 3 = 有効	3桁表示項目
//[]----------------------------------------------------------------------[]
///	@brief			システムメンテナンス：料金計算テスト
//[]----------------------------------------------------------------------[]
///	@return			MOD_CHG	: mode change<br>
///					MOD_EXT	: F5 key
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/30<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
unsigned short	SysMnt_RyoTest( void )
{
	ushort	msg;
	long	input;				// 入力データ
	short	result;
	uchar	ret;
	T_FrmReceipt	rec_data;	// 領収証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ/

// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//	f_sousai = 0;
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
	memset(&rt_ctrl_bk1, 0, sizeof(t_RT_Ctrl));
	memset(&rt_ctrl_bk2, 0, sizeof(t_RT_Ctrl));
	if( rt_ctrl.param.set == ON ){							// 共通パラメータ設定からの戻り
		rt_ctrl.param.set = OFF;
	}else{													// 通常起動or共通パラメータ設定からの起動
		RT_init_disp_item();								// 表示項目の初期化
		if( rt_ctrl.param.no == 0 ){						// 共通パラメータ設定からの起動ではない
			rt_ctrl.nrml_strt = ON;							// 通常起動フラグON
		}
	}

// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
// 	RT_base_disp(&input);									// 基本画面表示
	if ( OPECTL.RT_show_attention == 1 ) {
		// 注意画面表示
		RT_attention_disp();
	} else {
		// 基本画面表示
		RT_base_disp(&input);
	}
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		if( msg == KEY_MODECHG ){
			BUZPI();
			memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));			// 料金計算テスト制御データ初期化
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if( msg == LCD_DISCONNECT ){
			BUZPI();
			memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));			// 料金計算テスト制御データ初期化
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
		if ( OPECTL.RT_show_attention == 1 ) {
			switch( KEY_TEN0to9(msg) ) {
			case KEY_TEN_F1:		// 戻る
				BUZPI();

				if ( rt_ctrl.nrml_strt == ON ) {			// 通常終了
					// 料金計算テスト制御データ初期化
					memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
					return MOD_EXT;
				} else {									// 共通パラメータ設定への戻り
					// 料金計算テスト制御データ初期化(param.noを除く)
					memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
					return MNT_PRSET;
				}

				break;
			case KEY_TEN_F5:		// 次へ
				BUZPI();

				// 基本画面表示
				RT_base_disp(&input);

				// 次のログインまで注意画面を表示しない
				OPECTL.RT_show_attention = 0;

				break;
			default:
				break;
			}

			continue;
		}
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加

		// 料金計算前画面表示中
		if( rt_ctrl.mode == 0 ){
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// 終了(F5)
				BUZPI();
				if( rt_ctrl.sub_mode == 0 ){				// 親画面：料金計算テスト終了
					if( rt_ctrl.nrml_strt == ON ){			// 通常終了
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
															// 料金計算テスト制御データ初期化
						return MOD_EXT;
					}else{									// 共通パラメータ設定への戻り
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
															// 料金計算テスト制御データ初期化(param.noを除く)
						return MNT_PRSET;
					}
				}else{										// 子画面：基本画面に戻る
					memcpy(&rt_ctrl, &rt_ctrl_bk1, sizeof(t_RT_Ctrl));
					RT_base_disp(&input);					// 基本画面表示
				}
				break;
			case KEY_TEN_F1:		// ⊂(F1)
			case KEY_TEN_F2:		// ⊃(F2)
				if (input != -1) {
					// 有効範囲チェック
					if( RT_check_disp_data(input) ) {
						// チェックＮＧ(表示値戻しは関数の中で行う)
						BUZPIPI();
						input = -1;
						break;
					}
				}
				// カーソル移動前の表示項目を正転
				RT_get_disp_data(&input);					// 表示値取得
				// 表示項目の更新
				RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 0);
				// カーソル移動後の表示項目を反転＆７行目の表示
				RT_move_cursor(msg, &input);
				break;
			case KEY_TEN_F3:		// 切換・パラメータ(F3)
// MH810100(S) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)
//				if( (rt_ctrl.param.set != ON) && (rt_ctrl.param.no == 0) ){
// MH810100(E) Y.Yamauchi 2019121 車番チケットレス(メンテナンス)
					ret =  RT_check_KEY_TEN_F3(&input);
					if( ret == 1 ){
						return MNT_PRSET ;
					}
// MH810100(S) Y.Yamauchi 2019/12/14 車番チケットレス(メンテナンス)
//				}
// MH810100(E) Y.Yamauchi 2019/12/14 車番チケットレス(メンテナンス)
				break;
			case KEY_TEN_F4:		// 書込・実行(F4)
				if (input != -1){
					// 有効範囲チェック
					if( RT_check_disp_data(input) ) {
						// チェックＮＧ(表示値戻しは関数の中で行う)
						BUZPIPI();
						input = -1;
						break;
					}
				}
				RT_bkp_rst_data(0);							// データバックアップ
				result = RT_ryo_cal();
				if( !result ){								// 料金計算OK
					BUZPI();
					rt_ctrl.mode = 1;						// 料金計算結果画面
					rt_ctrl.sub_mode = 0;					// 親画面：料金計算テスト結果表示中
					rt_ctrl.pos = 0;
					input = -1;
				}else{
					BUZPIPI();
				}
				break;
			case KEY_TEN:			// 数字(テンキー)
				if( input == -1 ){							// その項目での最初の入力
					input = 0;
				}
				if( RT_check_KEY_TEN(&input, msg) ){		// テンキーチェック＆入力値更新
					break;
				}
				BUZPI();
				// 表示項目の更新
				RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
				break;
			case KEY_TEN_CL:		// 取消(テンキー)
				if( RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos] ){
					BUZPI();
					RT_get_disp_data(&input);				// 表示値取得
					RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
					if(( rt_ctrl.sub_mode == 0 ) &&
// MH810100(S) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)
//						( rt_ctrl.pos == RT_FLP_NO )){		// フラップ№
					   ( rt_ctrl.pos == RT_KIND )){			// 料金種別
// MH810100(E) Y.Yamauchi 2019/12/11 車番チケットレス(メンテナンス)
							input = base_data[rt_ctrl.pos];	// 表示値を設定
					}else{
						input = -1;
					}
				}
				break;
			default:
				break;
			}
		// 料金計算結果画面表示中
		}else if( rt_ctrl.mode == 1 ){
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// 終了(F5)
				BUZPI();
				if( rt_ctrl.sub_mode == 0 ){				// 親画面：料金計算テスト結果表示中
					if( rt_ctrl.param.no == 0 ){			// 通常終了
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
															// 料金計算テスト制御データ初期化
					}else{									// 共通パラメータ設定への戻り
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
															// 料金計算テスト制御データ初期化(param.noを除く)
					}
					RT_bkp_rst_data(1);						// データリストア
					RT_base_disp(&input);					// 基本画面表示
				}else{										// 子画面：親画面に戻る
					rt_ctrl.sub_mode = 0;					// 親画面：料金計算テスト結果表示中
					rt_ctrl.pos = 0;
					RT_recover_disp();
					input = -1;
				}
				break;
			case KEY_TEN_F1:		// 定期券・⊂(F1)
			case KEY_TEN_F2:		// サービス券・⊃(F2)
				if( rt_ctrl.sub_mode == 0 ){				// 親画面：料金計算テスト結果表示中
					if( ryo_buf.dsp_ryo ){					// 表示料金≠0円
						RT_check_disp_change(&input, msg);
					}
				}else if( rt_ctrl.sub_mode == 1 ){			// 定期券情報入力画面
					if (input != -1) {
						// 有効範囲チェック
						if( RT_check_disp_data(input) ) {
							// チェックＮＧ(表示値戻しは関数の中で行う)
							BUZPIPI();
							input = -1;
							break;
						}
					}
					// カーソル移動前の表示項目を正転
					RT_get_disp_data(&input);				// 表示値取得
					// 表示項目の更新
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 0);
					// カーソル移動後の表示項目を反転＆７行目の表示
					RT_move_cursor(msg, &input);
				}
				break;
			case KEY_TEN_F3:		// 掛売券・切換(F3)
				if( rt_ctrl.sub_mode == 0 ){				// 親画面：料金計算テスト結果表示中
					if( ryo_buf.dsp_ryo ){					// 表示料金≠0円
						RT_check_disp_change(&input, msg);
					}
				}else{										// 子画面
					RT_check_KEY_TEN_F3(&input);
				}
				break;
			case KEY_TEN_F4:		// 書込
				if( rt_ctrl.sub_mode != 0 ){				// 子画面
					if (input != -1) {
						// 有効範囲チェック
						if( RT_check_disp_data(input) ){
							// チェックＮＧ(表示値戻しは関数の中で行う)
							BUZPIPI();
							input = -1;
							break;
						}
					}
					result = RT_ryo_cal();
					if( !result ){							// 料金計算OK
						BUZPI();
						rt_ctrl.mode = 1;					// 料金計算結果画面
						rt_ctrl.sub_mode = 0;				// 親画面：料金計算テスト結果表示中
						rt_ctrl.pos = 0;
						input = -1;
					}else{
						BUZPIPI();
					}
				}else{
					RT_edit_log_data(&rec_data);			// ログ登録用データ編集
					RT_log_regist();
					BUZPI();
					// 登録後は基本画面へ
					if( rt_ctrl.param.no == 0 ){			// 通常終了
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));// 料金計算テスト制御データ初期化
					}else{									// 共通パラメータ設定への戻り
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));// 料金計算テスト制御データ初期化(param.noを除く)
					}
					RT_bkp_rst_data(1);						// データリストア
					RT_base_disp(&input);					// 基本画面表示
				}
				break;
			case KEY_TEN:			// 数字(テンキー)
				if( rt_ctrl.sub_mode != 0 ){				// 子画面
					if( input == -1 ){						// その項目での最初の入力
						input = 0;
					}
					if( RT_check_KEY_TEN(&input, msg) ){	// テンキーチェック＆入力値更新
						break;
					}
					BUZPI();
					// 表示項目の更新
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
				}
				break;
			case KEY_TEN_CL:		// 取消(テンキー)
				if( RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos] ){
					BUZPI();
					RT_get_disp_data(&input);				// 表示値取得
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//					if( rt_ctrl.sub_mode == 3 ){			// サービス券種別入力画面
//						input = serv_data[RT_SERV_KIND];	// 表示値を設定
//					}else{
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
						input = -1;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//					}
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
				}
				break;
			default:
				break;
			}
		// エラー画面表示中
		}else{
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// 終了(F5)
				if( rt_ctrl_bk1.mode == 1 ){				// 料金計算結果画面(子画面)でエラー発生
					BUZPI();
					rt_ctrl.mode = 1;						// 料金計算結果画面
					rt_ctrl.sub_mode = 0;					// 親画面：料金計算テスト結果表示中
					rt_ctrl.pos = 0;
					OPECTL.CR_ERR_DSP = 0;
					RT_recover_disp();						// 画面復帰
					input = -1;
				}
				break;
			case KEY_TEN_F4:		// エラークリア(F4)
				BUZPI();
				if( rt_ctrl_bk1.mode == 0 ){				// 基本画面でエラー発生
					memcpy(&rt_ctrl, &rt_ctrl_bk1, sizeof(t_RT_Ctrl));
															// rt_ctrlリストア
					RT_base_disp(&input);					// 基本画面表示
				}else{										// 料金計算結果画面(子画面)でエラー発生
					rt_ctrl.mode = 1;						// 料金計算結果の子画面(sub_modeは引き継ぐ)
					RT_clear_error();						// エラークリア
					OPECTL.CR_ERR_DSP = 0;
					if( rt_ctrl.sub_mode != 3 ){			// サービス券種別入力画面以外
						input = -1;
					}
				}
				break;
			default:
				break;
			}
		}
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 表示項目の初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_init_disp_item( void )
{
	// 基本画面(mode=0,sub_mode=0) 表示項目初期化
	base_data[RT_I_YEAR]    = base_data[RT_O_YEAR]   = (long)(CLK_REC.year%100);
															// 入庫年=出庫年=現在年
	base_data[RT_I_MONT]    = base_data[RT_O_MONT]   = (long)CLK_REC.mont;
															// 入庫月=出庫月=現在月
	base_data[RT_I_DAY]     = base_data[RT_O_DAY]    = (long)CLK_REC.date;
															// 入庫日=出庫日=現在日
	base_data[RT_I_HOUR]    = base_data[RT_I_MIN]    = 0;	// 入庫時分=0
	base_data[RT_O_HOUR]    = CLK_REC.hour;					// 出庫時=現在時
	base_data[RT_O_MIN]     = CLK_REC.minu;					// 出庫分=現在分
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	base_data[RT_FLP_NO]    = 1;							// フラップ№=1
	base_data[RT_KIND]      = 0;							// 料金種別=A種
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)

	// 計算結果画面(mode=1,sub_mode=0) 表示項目初期化
	rslt_data[RT_RYOUKIN]   = 0; 							// 計算結果画面データ
	rslt_data[RT_U_CARD]    = 0; 							// 計算結果画面データ

	// 定期券情報入力画面(mode=0/1,sub_mode=1) 表示項目初期化
	base_data[RT_PASS] = 0;									// 定期種別=0
	pass_data[RT_S_YEAR]    = pass_data[RT_E_YEAR]   = (long)(CLK_REC.year%100);
															// 有効開始年=有効終了年=現在年
	pass_data[RT_S_MONT]    = pass_data[RT_E_MONT]   = (long)CLK_REC.mont;
															// 有効開始月=有効終了月=現在月
	pass_data[RT_S_DAY]     = pass_data[RT_E_DAY]    = (long)CLK_REC.date;
															// 有効開始日=有効終了日=現在日
	// 掛売先情報(掛売券用)入力画面(mode=1,sub_mode=2) 表示項目初期化
	kake_data[RT_KAKE_NO]   = 0;							// 店No.=0

	// サービス券種別入力画面(mode=1,sub_mode=3) 表示項目初期化
	serv_data[RT_SERV_KIND] = 0;							// サービス券種別=A種

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 基本画面表示
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: 表示データポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_base_disp( long *p_in )
{
	uchar	base_max;												// 基本画面表示項目最大数
	char	disp_pos;												// 表示位置
	long	disp_data;												// 表示データ
	ushort	rev;													// 反転

	// clear screen
	dispclr();
	// 基本画面生成
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);		// "＜料金計算テスト＞　　　　　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[2]);		// "各項目を入力して下さい　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[3]);		// "入庫時刻：  年  月  日  ：  　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[4]);		// "出庫時刻：  年  月  日  ：  　"
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[5]);		// "料金種別：　種　定期種別：　　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[13]);	// "料金種別：　種　　　　　　　　"
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	base_max = RT_BASE_MAX - 1;										// 項目数
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[7]);		// "　実行：料金計算を実行します　"

	for (disp_pos = 0; disp_pos < base_max; disp_pos++){
		disp_data = base_data[disp_pos];
		if( disp_pos == rt_ctrl.pos ){								// カーソル位置データ
			rev = 1;												// 反転
		}else{
			rev = 0;												// 正転
		}
		RT_change_disp(0, 0, disp_pos, disp_data, rev);				// 表示項目の更新
	}
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	 if( rt_ctrl.param.no != 0){										// システムパラメータからの要求
//	 	Fun_Dsp( FUNMSG2[46] );										// "　⊂　　⊃　　　　 実行  終了 "
	if( rt_ctrl.pos == RT_KIND ){
		Fun_Dsp( FUNMSG2[64] );										// "　⊂　　⊃　 切換  実行  終了 "
		*p_in = base_data[RT_KIND];
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	}else{
// GG124100(S) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
// 		Fun_Dsp( FUNMSG2[52] );										// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
		if ( rt_ctrl.nrml_strt == ON ) {							// 通常起動
			Fun_Dsp( FUNMSG2[52] );									// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
		} else {													// 共通パラメーター設定からの起動
			Fun_Dsp( FUNMSG2[46] );									// "　⊂　　⊃　　　　 実行  終了 "
		}
// GG124100(E) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
		*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)		
	}
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	return;
}

// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 注意画面表示
//[]----------------------------------------------------------------------[]
///	@param[in/out]	void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			R.Endo
///	@date			Create	: 2021/09/29
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void	RT_attention_disp( void )
{
	// clear screen
	dispclr();

	// 注意画面生成
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);		// "＜料金計算テスト＞　　　　　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[21]);	// "本テストの制限事項：　　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[22]);	// "・車種切換をともなう割引は　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[23]);	// "　最初に適用してください。　　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[24]);	// "・多店舗割引は非対応です。　　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[25]);	// "　　　　　　　　　　　　　　　"
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[26]);	// "　　　　　　　　　　　　　　　"
	Fun_Dsp(FUNMSG[114]);											// " 戻る                    次へ "

	return;
}
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加

// 基本画面項目(mode=0,sub_mode=0)
static const t_RT_DispItem disp_item_base[] = {
	{ 2, 10, 2, 0, 0,  99,   0},		// 入庫年
	{ 2, 14, 2, 0, 1,  12,   0},		// 入庫月
	{ 2, 18, 2, 0, 1,  31,   0},		// 入庫日
	{ 2, 22, 2, 0, 0,  23,   0},		// 入庫時
	{ 2, 26, 2, 0, 0,  59,   0},		// 入庫分
	{ 3, 10, 2, 0, 0,  99,   0},		// 出庫年
	{ 3, 14, 2, 0, 1,  12,   0},		// 出庫月
	{ 3, 18, 2, 0, 1,  31,   0},		// 出庫日
	{ 3, 22, 2, 0, 0,  23,   0},		// 出庫時
	{ 3, 26, 2, 0, 0,  59,   0},		// 出庫分
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	{ 4, 10, 4, 0, 1,9900,   0},		// 車室№
	{ 4, 10, 1, 1, 0,  11,   0},		// 料金種別
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	{ 4, 26, 2, 1, 0,  15,   0},		// 定期種別(0:なし/01～15)
	{ 5,  4, 3, 0, 0, 100, 255},		// 店1-No.		(最大値：1店舗=100/多店舗=255)
};

// 定期券情報入力画面項目(mode=0/1,sub_mode=1)
static const t_RT_DispItem disp_item_pass[] = {
	{ 2, 10, 2, 1, 0,  15,   0},		// 定期種別(0:なし/01～15)
	{ 3, 10, 2, 0, 0,  99,   0},		// 有効開始年
	{ 3, 14, 2, 0, 1,  12,   0},		// 有効開始月
	{ 3, 18, 2, 0, 1,  31,   0},		// 有効開始日
	{ 4, 10, 2, 0, 0,  99,   0},		// 有効終了年
	{ 4, 14, 2, 0, 1,  12,   0},		// 有効終了月
	{ 4, 18, 2, 0, 1,  31,   0},		// 有効終了日
};

// 掛売先情報(掛売券用)入力画面項目(mode=1,sub_mode=2)
static const t_RT_DispItem disp_item_kake[] = {
	{ 2,  5, 3, 0, 0, 100,  0},			// 店1-No.
};

// サービス券種別入力画面項目(mode=1,sub_mode=3)
static const t_RT_DispItem disp_item_serv[] = {
	{ 2,  6, 1, 1, 0,  14,   0},		// サービス券種別(0～14:A～O種)
};

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 表示項目の更新
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: 画面編集モード
///	@param[in]		sub_mode: 子画面編集モード
///	@param[in]		pos		: 表示項目番号
///	@param[in]		prm		: データ内容
///	@param[in]		rev		: 0:正転 1:反転 
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_change_disp(char mode, char sub_mode, char pos, long data, ushort rev)
{
	ushort	line;
	ushort	lpos;
	ushort	keta;
	uchar	width;
	uchar	disp_flg = 1;

	if( !mode ){		// mode=0 基本画面
		switch( sub_mode ){
		case  0:		// 基本画面編集中
			line  = disp_item_base[pos].line;
			lpos  = disp_item_base[pos].lpos;
			keta  = disp_item_base[pos].keta;
			width = disp_item_base[pos].width;
			break;
		default:
			disp_flg = 0;
			break;
		}
	}else{				// mode=1 計算結果画面
		switch( sub_mode ){
		case  1:		// 定期券情報入力画面編集中
			line  = disp_item_pass[pos].line;
			lpos  = disp_item_pass[pos].lpos;
			keta  = disp_item_pass[pos].keta;
			width = disp_item_pass[pos].width;
			break;
		case  2:		// 掛売先情報(掛売券用)入力画面編集中
			line  = disp_item_kake[pos].line;
			lpos  = disp_item_kake[pos].lpos;
			keta  = disp_item_kake[pos].keta;
			width = disp_item_kake[pos].width;
			break;
		case  3:		// サービス券種別入力画面編集中
			line  = disp_item_serv[pos].line;
			lpos  = disp_item_serv[pos].lpos;
			keta  = disp_item_serv[pos].keta;
			width = disp_item_serv[pos].width;
			break;
		default:
			disp_flg = 0;
			break;
		}
	}

	if( disp_flg ){
		if( width ){	// 全角
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//			if(( mode == 1 ) && ( sub_mode == 3 ) && ( pos == RT_SERV_KIND )){
			if(( mode == 0 ) && ( sub_mode == 0 ) && ( pos == RT_KIND )){
				grachr(4, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data]);		// 料金種別="Ａ"～"Ｌ"表示
			}else if(( mode == 1 ) && ( sub_mode == 3 ) && ( pos == RT_SERV_KIND )){
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
				grachr(2, 6, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data]);			// サービス券種別="Ａ"～"Ｃ(Ｏ)"表示
			}else if(( mode == 0 ) && ( sub_mode == 0 ) && ( pos == RT_PASS ) && ( data == 0 )){
				grachr(4, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// 定期種別="なし"表示
			}else if(( sub_mode == 1 ) && ( pos == RT_PASS_KIND ) && ( data == 0 )){
				grachr(2, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// 定期種別="なし"表示
			}else{
				opedpl(line, lpos, (ulong)data, keta, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
			}
		}else{			// 半角
			opedpl3(line, lpos, (ulong)data, keta, 1, rev, COLOR_BLACK,  LCD_BLINK_OFF);
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 表示データチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: ﾊﾟﾗﾒｰﾀﾎﾟｲﾝﾀ
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2005/04/12<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
static uchar	RT_check_disp_data( long data )
{
	ushort	max_wk;
	long	data_wk = 0;
	uchar	ret = 1;
	// 入力値をチェックしてＯＫならデータ格納
	// ＮＧなら表示を戻して（反転）リターン

	if( rt_ctrl.mode == 0 ){		// mode=0 基本画面
		switch( rt_ctrl.sub_mode ){
		case  0:		// 基本画面編集中
			data_wk = base_data[rt_ctrl.pos];
			if(( rt_ctrl.pos == RT_I_DAY ) ||				// 入庫日
			  ( rt_ctrl.pos == RT_O_DAY )){					// 出庫日
				max_wk = (uchar)RT_day_max();				// 該当月の最終日算出
			}else{
				max_wk = disp_item_base[rt_ctrl.pos].max1;
			}
			if(( disp_item_base[rt_ctrl.pos].min <= data ) &&
			   ( data <= max_wk )){							// 範囲内
				base_data[rt_ctrl.pos] = data;				// 入力値格納
				if( rt_ctrl.pos <= RT_I_DAY ){
					ret = RT_check_ymd(base_data[RT_I_YEAR], base_data[RT_I_MONT], base_data[RT_I_DAY]);
				}else if(( RT_O_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_O_DAY)){
					ret = RT_check_ymd(base_data[RT_O_YEAR], base_data[RT_O_MONT], base_data[RT_O_DAY]);
				}else{
					ret = 0;
				}
				if( ret ){
					base_data[rt_ctrl.pos] = data_wk;
				}
			}
			break;
		default:
			break;
		}
	}else{						// mode=1 計算結果画面
		switch( rt_ctrl.sub_mode ){
		case  1:		// 定期券情報入力画面編集中
			data_wk = pass_data[rt_ctrl.pos];
			if(( rt_ctrl.pos == RT_S_DAY ) ||				// 有効開始日
			   ( rt_ctrl.pos == RT_E_DAY )){ 				// 有効終了日
				max_wk = (uchar)RT_day_max();				// 該当月の最終日算出
			}else{
				max_wk = disp_item_pass[rt_ctrl.pos].max1;
			}
			if(( disp_item_pass[rt_ctrl.pos].min <= data ) &&
			   ( data <= max_wk )){							// 範囲内
				pass_data[rt_ctrl.pos] = data;				// 入力値格納
				if(( RT_S_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_S_DAY)){
					ret = RT_check_ymd(pass_data[RT_S_YEAR], pass_data[RT_S_MONT], pass_data[RT_S_DAY]);
				}else if(( RT_E_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_E_DAY)){
					ret = RT_check_ymd(pass_data[RT_E_YEAR], pass_data[RT_E_MONT], pass_data[RT_E_DAY]);
				}else{
					ret = 0;
				}
				if( ret ){
					pass_data[rt_ctrl.pos] = data_wk;
				}
			}
			break;
		case  2:		// 掛売先情報(掛売券用)入力画面編集中
			data_wk = kake_data[rt_ctrl.pos];
			if(( disp_item_kake[rt_ctrl.pos].min <= data ) &&
			   ( data <= disp_item_kake[rt_ctrl.pos].max1 )){// 範囲内
				kake_data[rt_ctrl.pos] = data;				// 入力値格納
				ret = 0;
			}
			break;
		case  3:		// サービス券種別入力画面編集中
			data_wk = serv_data[rt_ctrl.pos];
			if(( disp_item_serv[rt_ctrl.pos].min <= data ) &&
			   ( data <= disp_item_serv[rt_ctrl.pos].max1 )){// 範囲内
				serv_data[rt_ctrl.pos] = data;				// 入力値格納
				ret = 0;
			}
			break;
		default:
			break;
		}
	}

	if( ret ){		// チェックNG
		RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, data_wk, 1);
	}														// 入力前のデータを反転表示

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 年月日チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		year	: 年
///	@param[in]		month	: 月
///	@param[in]		day		: 日
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_ymd( long year, long month, long day )
{
	uchar ret = 0;

	if( year < 80 ){
		year += 2000;
	}
	else{
		year += 1900;
	}

	if( DATE_CHK_ERR((short)year, (short)month, (short)day) ){
		ret = 1;
	}

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 表示データ取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: 表示データポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_get_disp_data( long *p_in )
{

	if( rt_ctrl.mode == 0 ){								// mode=0 基本画面
		switch( rt_ctrl.sub_mode ){
		case  0:											// 基本画面編集中
			*p_in = base_data[rt_ctrl.pos];
			break;
		default:
			break;
		}
	}else{													// mode=1 計算結果画面
		switch( rt_ctrl.sub_mode ){
		case  1:											// 定期券情報入力画面編集中
			*p_in = pass_data[rt_ctrl.pos];
			break;
		case  2:											// 掛売先情報(掛売券用)入力画面編集中
			*p_in = kake_data[rt_ctrl.pos];
			break;
		case  3:											// サービス券種別入力画面編集中
			*p_in = serv_data[rt_ctrl.pos];
			break;
		default:
			break;
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト カーソル移動
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: メッセージ
///	@param[in/out]	p_in	: 表示データポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_move_cursor( ushort msg, long *p_in )
{
	long	data;

	if( msg == KEY_TEN_F1 ){								// 前項目に移動
		if( rt_ctrl.mode == 0 ){							// mode=0 基本画面
			switch( rt_ctrl.sub_mode ){
			case  0:										// 基本画面編集中
				BUZPI();
				--rt_ctrl.pos;
				if( rt_ctrl.pos < 0 ){
					rt_ctrl.pos = (char)( RT_BASE_MAX - 2);	// 料金種別位置とする
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				 }
//				 if( rt_ctrl.param.no != 0 ){
//				 	Fun_Dsp( FUNMSG2[46] );					// "　⊂　　⊃　　　　 実行  終了 "
					Fun_Dsp( FUNMSG2[64] );					// "　⊂　　⊃　 切換  実行  終了 "
					*p_in = base_data[rt_ctrl.pos];			// 表示値を返す
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)					
				}else{
// GG124100(S) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
// 					Fun_Dsp( FUNMSG2[52] );					// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
					if ( rt_ctrl.nrml_strt == ON ) {		// 通常起動
						Fun_Dsp( FUNMSG2[52] );				// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
					} else {								// 共通パラメーター設定からの起動
						Fun_Dsp( FUNMSG2[46] );				// "　⊂　　⊃　　　　 実行  終了 "
					}
// GG124100(E) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
				}
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
// MH810100(S) Y.Yamauchi #3909 料金計算テスト?を押した際の出庫・入庫時刻が間違って表示される
				*p_in = -1;
// MH810100(E) Y.Yamauchi #3909 料金計算テスト?を押した際の出庫・入庫時刻が間違って表示される
				break;
			default:
				break;
			}
		}else{												// mode=1 計算結果画面
			switch( rt_ctrl.sub_mode ){
			case  1:										// 定期券情報入力画面編集中
				BUZPI();
				--rt_ctrl.pos;
				if( rt_ctrl.pos < 0 ){
					rt_ctrl.pos = (char)( RT_PASSINFO_MAX - 1);	// 有効終了日位置とする
				}
				Fun_Dsp( FUNMSG2[2] );						// "　⊂　　⊃　　　　 書込  終了 "
				*p_in = -1;
				break;
			case  2:										// 掛売先情報(掛売券用)入力画面編集中
			case  3:										// サービス券種別入力画面編集中
				// 項目移動がないので通らない
			default:
				break;
			}
		}
	}else{													// 後項目に移動
		if( rt_ctrl.mode == 0 ){							// mode=0 基本画面
			switch( rt_ctrl.sub_mode ){
			case  0:										// 基本画面編集中
				BUZPI();
				++rt_ctrl.pos;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				if( rt_ctrl.pos >= (RT_BASE_MAX - 1) ){		// 多店舗割引なし時の位置最大値以上
				if( rt_ctrl.pos >= RT_BASE_MAX - 1 ){			// 位置最大値以上
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
					rt_ctrl.pos = 0;						// 先頭位置とする
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				 }
//				 if( rt_ctrl.param.no != 0){					// システムパラメータからの要求
//				 	Fun_Dsp( FUNMSG2[46] );					// "　⊂　　⊃　　　　 実行  終了 "
//				 }else{
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)					
// GG124100(S) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
// 					Fun_Dsp( FUNMSG2[52] );					// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
					if ( rt_ctrl.nrml_strt == ON ) {		// 通常起動
						Fun_Dsp( FUNMSG2[52] );				// "　⊂　　⊃　ﾊﾟﾗﾒｰﾀ 実行  終了 "
					} else {								// 共通パラメーター設定からの起動
						Fun_Dsp( FUNMSG2[46] );				// "　⊂　　⊃　　　　 実行  終了 "
					}
// GG124100(E) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
				}


// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				*p_in = -1;
				if( rt_ctrl.pos == RT_KIND ){				// 料金種別
					Fun_Dsp( FUNMSG2[64] );					// "　⊂　　⊃　 切換  実行  終了 "
					*p_in = base_data[rt_ctrl.pos];			// 表示値を返す
				}else{
					*p_in = -1;
				}
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
				break;
			default:
				break;
			}
		}else{												// mode=1 計算結果画面
			switch( rt_ctrl.sub_mode ){
			case  1:										// 定期券情報入力画面編集中
				BUZPI();
				++rt_ctrl.pos;
				if( rt_ctrl.pos >= RT_PASSINFO_MAX ){		// 定期券情報入力位置最大値以上
					rt_ctrl.pos = 0;						// 先頭位置とする
				}
				Fun_Dsp( FUNMSG2[2] );						// "　⊂　　⊃　　　　 書込  終了 "
				*p_in = -1;
				break;
			case  2:										// 掛売先情報(掛売券用)入力画面編集中
			case  3:										// サービス券種別入力画面編集中
				// 項目移動がないので通らない
			default:
				break;
			}
		}
	}
	RT_get_disp_data(&data);								// 表示値の取得
	RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, data, 1);
															// 現在カーソル位置反転

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト F3キーチェック＆入力値更新
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: 入力値のポインタ
///	@return			kind	: 0 = 無効	1 = 設定	2～5 = 切換
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_KEY_TEN_F3( long *p_in )
{
	uchar	kind;

	kind = RT_KEY_TEN_F3_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos];
	switch( kind ){
	case  1:		// 共通パラメータ設定への遷移
// GG124100(S) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
		if ( rt_ctrl.nrml_strt == ON ) {					// 通常起動
// GG124100(E) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
			BUZPI();
			rt_ctrl.param.set = ON;							// 共通パラメータ設定要求ON
// GG124100(S) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
		} else {											// 共通パラメーター設定からの起動
			kind = 0;
		}
// GG124100(E) R.Endo 2022/02/17 車番チケットレス3.0 #6138 パラメータ設定画面から料金計算テスト画面遷移時に「パラメータ」ボタンが表示されてしまう
		break;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	case  2:		// 表示切換(料金種別)
		BUZPI();
		if( ++(*p_in) > 11 ){								// input > L種
			*p_in = 0;										// A種に戻す
		}
		RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, *p_in, 1);
															// 反転表示
		break;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	case  5:		// 表示切換(サービス券種別)
		BUZPI();
		if( rt_ctrl.sub_mode != 3 ){
			memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));// rt_ctrlバックアップ
			rt_ctrl.sub_mode = 3;							// サービス券種別入力画面
			rt_ctrl.pos = 0;								// サービス券種別位置
		}
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);					// "＜料金計算テスト＞　　　　　　"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[14]);				// "サービス券種別を入力して下さい"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[15]);				// "種別：　種　　　　　　　　　　"
		if( ++(*p_in) > 14 )								// input > O種
		{
			*p_in = 0;										// A種に戻す
		}
		grachr(2, 6, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[*p_in]);				// サービス券種別="Ａ"～"Ｃ(Ｏ)"表示
		Fun_Dsp( FUNMSG2[54] );								// "　　　　　　 切換  書込  終了 "
	default:
		break;
	}

	return(kind);
}
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//[]----------------------------------------------------------------------[]
///	@brief			駐車券 役割設定チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		m_gtticstp *mag	: 駐車券データ
///					uchar check_type: 0 
///					uchar f_CheckOnly: 0 <br>
///@return			ret		: 13 = 種別規定外ｴﾗｰ 0 = 正常
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/12/01<br>
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]	
//static short RT_al_tikck2( m_gtticstp *mag, uchar check_type, uchar f_CheckOnly )
static short RT_al_tikck2( m_gtticstp *mag )
{
	short	ret = 0;
	/* 駐車券 役割設定チェック */
	if( prm_get( COM_PRM,S_SHA,(short)(1+6*( base_data[RT_KIND] )),2,5 ) == 0L ){	// 車種設定なし（使用する設定でない）
		ret = 13;											// 種別規定外ｴﾗｰ
	}
	return( ret );
}
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 料金計算実行
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_ryo_cal( void )
{
	short	ret = 0;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	ushort	posi;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
	
	memset( MAGred, '\0', sizeof(MAGred) );
	// 磁気データ生成
	RT_make_mag_data(MAGred);
	if( rt_ctrl.mode == 0 ){		// 基本画面からの料金計算実行
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrlバックアップ
		RT_init_cal_data();									// 各種料金計算用データ初期化
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//		if( LKopeGetLockNum( (uchar)0, (ushort)base_data[RT_FLP_NO], &posi ) ){
//			OPECTL.Op_LokNo = (ulong)( base_data[RT_FLP_NO] );		// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
//			OPECTL.Pr_LokNo = posi;									// 内部処理用駐車位置番号(1～324)
//			ret = (short)FlpSetChk(posi-1);
//		}
//
//		if( ret == 0 ){										// チェックNG
//			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[17] );			// " ＜＜ 設定されていません ＞＞ "
//			Fun_Dsp( FUNMSG2[57] );							// "　　　　　　　　　ｴﾗｰｸﾘｱ　　　"
//			rt_ctrl.mode = 2;								// エラー表示画面
//			rt_ctrl.sub_mode = 0;							// 親画面
//			rt_ctrl.pos =0;
//			return(1);
//	}
//
//		ret = RT_set_tim( OPECTL.Pr_LokNo, &CLK_REC );
		ret = RT_al_tikck2( (m_gtticstp*)MAGred );		// 駐車券 役割設定チェック
		if( ret != 0 ){				// チェックNG
			// エラー表示
			CardErrDsp( ret );							//  エラー表示
			Fun_Dsp( FUNMSG2[57] );						// "　　　　　　　　　ｴﾗｰｸﾘｱ　　　"
			rt_ctrl.mode = 2;							// エラー表示画面
			rt_ctrl.sub_mode = 0;						// 親画面
			rt_ctrl.pos =0;
			return(1);
		}
		OPECTL.CalStartTrigger = (uchar)1;				// 料金計算開始要因set
		card_use[USE_TIK] += 1;

		ret = RT_set_tim((ushort)OPECTL.CalStartTrigger, &CLK_REC);
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
		if( ret ){
			// エラー表示
			grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, SMSTR4[20]);				// エラー("　 入出庫時刻判定エラーです 　")
			Fun_Dsp( FUNMSG2[57] );							// "　　　　　　　　　ｴﾗｰｸﾘｱ　　　"
			rt_ctrl.mode = 2;								// エラー表示画面
			rt_ctrl.sub_mode = 0;							// 親画面
			rt_ctrl.pos =0;
			return(1);
		}

		RT_ryo_cal2(0, OPECTL.Pr_LokNo);						// 料金計算実行
		RT_disp_cal_result();								// 料金計算結果表示
	}else{						// 計算結果画面からの料金計算実行
		switch( rt_ctrl.sub_mode ){
		case  1:											// 定期券情報入力画面
			ret = al_pasck( (m_gtapspas *)MAGred );
			if( ret != 0 ){									// チェックNG
				OPECTL.CR_ERR_DSP = ret;
				// エラー表示
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				CardErrDsp();
				CardErrDsp( ret );
// MH810100(E Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
				Fun_Dsp( FUNMSG2[55] );						// "　　　　　　　　　ｴﾗｰｸﾘｱ 終了 "
				rt_ctrl.mode = 2;							// エラー表示画面
				return(1);
			}
			memcpy( &RT_PayData.teiki, &InTeiki_PayData_Tmp, sizeof(teiki_use) );// 定期券情報セット
			PayData.teiki.id = InTeiki_PayData_Tmp.id;							 // 条理チェックで使用するためIDセット
			card_use[USE_PAS] += 1;
			rt_ctrl.last_card.kind = RT_CRD_PASS;			// 最終使用カード種別=定期券
			rt_ctrl.last_card.info = (short)pass_data[RT_PASS_KIND];
															// 最終使用カード情報=定期券種別
			RT_ryo_cal2(1, OPECTL.Pr_LokNo);				// 料金計算実行
			RT_disp_cal_result();							// 料金計算結果表示
			break;
		case  2:											// 掛売先情報入力画面
		case  3:											// サービス券種別入力画面
			ret = al_svsck( (m_gtservic *)MAGred );
			if( ret != 0 ){									// チェックNG
				// エラー表示
				OPECTL.CR_ERR_DSP = ret;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)	
//				CardErrDsp();				
				CardErrDsp( ret );
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)					
				Fun_Dsp( FUNMSG2[55] );						// "　　　　　　　　　ｴﾗｰｸﾘｱ 終了 "
				rt_ctrl.mode = 2;							// エラー表示画面
				return(1);
			}
			if( rt_ctrl.sub_mode == 2 ){					// 掛売先情報入力画面
				rt_ctrl.last_card.kind = RT_CRD_KAKE;		// 最終使用カード種別=掛売券
				rt_ctrl.last_card.info = (short)kake_data[RT_KAKE_NO];
															// 最終使用カード情報=店No.
			}else{
				rt_ctrl.last_card.kind = RT_CRD_SERV;		// 最終使用カード種別=サービス券
				rt_ctrl.last_card.info = (short)serv_data[RT_SERV_KIND];
															// 最終使用カード情報=サービス券種別
			}
			RT_ryo_cal2(3, OPECTL.Pr_LokNo);				// 料金計算実行
			RT_disp_cal_result();							// 料金計算結果表示
			memcpy( PayData.DiscountData, RT_PayData.DiscountData, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 条理チェックのため、精算ﾃﾞｰﾀ割引情報作成セット*/
			break;
		default:
			break;
		}
	}

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト テンキーチェック＆入力値更新
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: 入力値のポインタ
///	@param[in]		msg		: キー内容
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_KEY_TEN( long *p_in, ushort msg )
{
	uchar	keta;

	keta = RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos];
	if( !keta ) {	// テンキー無効
		return(1);
	}

	switch( keta ){
	case  1:		// 1桁
		*p_in = (long)(msg - KEY_TEN0);
		break;
	case  2:		// 2桁
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 100);
		break;
	case  3:		// 3桁
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 1000);
		break;
	case  4:		// 4桁
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 10000);
		break;
	default:
		return(1);
		break;
	}

	return(0);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 表示切換チェック
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: 入力値のポインタ
///	@param[in]		msg		: キー内容
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/13<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_disp_change( long *p_in, ushort msg )
{
	char	disp_pos;										// 表示位置

	switch( msg ){
	case KEY_TEN_F1:		// 画面切換(→定期券情報入力画面)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrlバックアップ
		// rt_ctrl.modeは引継ぐ
		rt_ctrl.sub_mode = 1;										// 定期券情報入力画面
		rt_ctrl.pos = 0;											// 定期券種別位置
		pass_data[RT_PASS_KIND] = base_data[RT_PASS];				// 定期種別値コピー
		dispclr();													// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);	// "＜料金計算テスト＞　　　　　　"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[8]);	// "定期券情報を入力して下さい　　"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[9]);	// "定期種別：　　　　　　　　　　"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[10]);	// "有効開始：  年  月  日　　　　"
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[11]);	// "有効終了：  年  月  日　　　　"
		RT_change_disp(1, 1, 0, pass_data[0], 1);			// 定期券種別を反転表示
		for (disp_pos = 1; disp_pos < RT_PASSINFO_MAX; disp_pos++) {
			RT_change_disp(1, 1, disp_pos, pass_data[disp_pos], 0);	// 各項目表示
		}
		*p_in = -1;
		Fun_Dsp( FUNMSG2[2] );								// "　⊂　　⊃　　　　 書込  終了 "
		break;
	case KEY_TEN_F2:		// 表示切換(サービス券種別)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrlバックアップ
		rt_ctrl.sub_mode = 3;								// サービス券種別入力画面
		rt_ctrl.pos = 0;									// サービス券種別位置
		*p_in = serv_data[RT_SERV_KIND];					// サービス券種別取得
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);					// "＜料金計算テスト＞　　　　　　"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[14]);					// "サービス券種別を入力して下さい"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[15]);					// "種別：　種　　　　　　　　　　"
		if( *p_in > 14 )									// input > O種
		{
			*p_in = 0;										// A種に戻す
		}
		grachr(2, 6, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[*p_in]);					// サービス券種別="Ａ"～"Ｏ"表示
		Fun_Dsp( FUNMSG2[54] );								// "　　　　　　 切換  書込  終了 "
		break;
	case KEY_TEN_F3:		// 画面切換(→掛売先情報入力画面)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrlバックアップ
		// rt_ctrl.modeは引継ぐ
		rt_ctrl.sub_mode = 2;								// 掛売先情報入力画面
		rt_ctrl.pos = 0;									// 店1No.種別位置
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);						// "＜料金計算テスト＞　　　　　　"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[12]);					// "掛売先情報を入力して下さい　　"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[6]);						// "店1：   　　　　　　　　　　　"
		RT_change_disp(1, 2, 0, kake_data[0], 1);			// 店1No.を反転表示
		Fun_Dsp( FUNMSG2[1] );								// "　　　　　　　　　 書込  終了 "
		*p_in = -1;
		break;
	default:
		break;
	}

	return(0);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 画面復帰
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/13<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_recover_disp(void)
{
	uchar	base_max;										// 基本画面表示項目最大数
	char	disp_pos;										// 表示位置
	long	disp_data;										// 表示データ

	dispclr();												// clear screen
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);							// "＜料金計算テスト＞　　　　　　"
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR4[16]);				// "計算結果：　　　　　　　　　円"
	opedpl(1, 16, (ulong)ryo_buf.dsp_ryo,6, 0, 0, COLOR_FIREBRICK, LCD_BLINK_OFF);		// 計算結果表示
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[3]);							// "入庫時刻：  年  月  日  ：  　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[4]);							// "出庫時刻：  年  月  日  ：  　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[13]);						// 	"料金種別：　種　　　　　　　　", 
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)	
//	base_max = RT_BASE_MAX;								// 項目数
	base_max = RT_BASE_MAX - 1 ;						// 項目数
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)		

	for (disp_pos = 0; disp_pos < base_max; disp_pos++) {
		disp_data = base_data[disp_pos];
		RT_change_disp(0, 0, disp_pos, disp_data, 0);
	}
	switch( rt_ctrl.last_card.kind ){
	case RT_CRD_PASS:										// 定期券
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[17]);					// "使用券種：定期券　種別　　　　"
		opedpl(6, 22, (ulong)rt_ctrl.last_card.info,2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case RT_CRD_SERV:										// サービス券
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[18]);					// "使用券種：サービス券　　種　　"
		grachr(6, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF,DAT1_4[rt_ctrl.last_card.info]);
		break;
	case RT_CRD_KAKE:										// 掛売券
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[19]);						// "使用券種：掛売券店　No　　　　"
		opedpl(6, 22, (ulong)rt_ctrl.last_card.info,3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	default			:
		break;
	}
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)			
//	Fun_Dsp( FUNMSG2[53] );									// "定期券 サ券 掛売券　　　 終了 "
	if( !rt_ctrl.pos == RT_KIND ){							// システムパラメータからの要求
		Fun_Dsp( FUNMSG2[64] );								// "　⊂　　⊃　 切換  実行  終了 "
	}else{ 
		Fun_Dsp( FUNMSG2[53] );								// "定期券 サ券 掛売券　　　 終了 "
	}
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)			
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 各種料金計算用データ初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/20<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_init_cal_data( void )
{
	// 料金計算用データをここで初期化する
	CRD_DAT.PAS.cod = 0;									// 定期ｺｰﾄﾞｸﾘｱ
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)			
	OPECTL.CalStartTrigger = 0;								// 料金計算開始要因set
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)				
	CardUseSyu = 0;											// 1精算の割引種類の件数ｸﾘｱ
	PassIdBackup = 0;										// 定期券idﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)ｸﾘｱ
	PassIdBackupTim = 0;									// 定期n分ﾁｪｯｸ解除
	PassPkNoBackup = 0;										// 定期券駐車場番号ﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)ｸﾘｱ

	/** ｴﾘｱｸﾘｱ **/
	cm27();													// ﾌﾗｸﾞｸﾘｱ
	OPECTL.ChkPassPkno = 0L;								// 定期券問合せ中駐車場№ｸﾘｱ
	OPECTL.ChkPassID = 0L;									// 定期券問合せ中定期券IDｸﾘｱ
	memset( card_use , 0 , sizeof( card_use ) ) ;			// 1精算ｶｰﾄﾞ使用種類
	memset( &CRD_DAT.PAS, 0, sizeof(pas_rcc) );
	memset( &CRD_DAT.SVS, 0, sizeof(svs_rec) );
	memset( &CRD_DAT.PRE, 0, sizeof(pre_rec) );
	memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 磁気データ生成
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: 磁気データのポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_mag_data( char *p_data )
{
	m_gtapspas	*p_magP;		// 定期券
	m_gtservic	*p_magS;		// サービス券

	if( rt_ctrl.mode == 0 ){								// 基本画面
		if( !base_data[RT_PASS] ){							// 駐車券
		}else{												// 定期券
			p_magP = (m_gtapspas *)p_data;
			RT_make_pass(p_magP);
		}
	}else{													// 計算結果画面
		if( rt_ctrl.sub_mode == 1 ){						// 定期券
			p_magP = (m_gtapspas *)p_data;
			RT_make_pass(p_magP);
		}else{												// 掛売券・サービス券
			p_magS = (m_gtservic *)p_data;
			RT_make_service(p_magS);
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 定期券磁気データ生成
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_mag	: 磁気データのポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_pass( m_gtapspas *p_magP )
{
	p_magP->apspas.aps_idc = 0x1a;							// 定期券
	RT_get_valid_no((char *)p_magP);						// 有効定期券ID取得
	p_magP->apspas.aps_pno[1] |= (uchar)(pass_data[RT_PASS_KIND] & 0x0f);
															// 定期券種別
	p_magP->apspas.aps_sta[0] = (uchar)(pass_data[RT_S_YEAR] & 0x7f);
	p_magP->apspas.aps_sta[1] = (uchar)(pass_data[RT_S_MONT] & 0x7f);
	p_magP->apspas.aps_sta[2] = (uchar)(pass_data[RT_S_DAY]  & 0x7f);
															// 有効開始年月日
	p_magP->apspas.aps_end[0] = (uchar)(pass_data[RT_E_YEAR] & 0x7f);
	p_magP->apspas.aps_end[1] = (uchar)(pass_data[RT_E_MONT] & 0x7f);
	p_magP->apspas.aps_end[2] = (uchar)(pass_data[RT_E_DAY]  & 0x7f);
															// 有効終了年月日
	p_magP->apspas.aps_sts    = 0x02;						// ステータス=入庫固定
	p_magP->apspas.aps_wrt[0] = (uchar)(base_data[RT_I_MONT] & 0x7f);
	p_magP->apspas.aps_wrt[1] = (uchar)(base_data[RT_I_DAY]  & 0x7f);
	p_magP->apspas.aps_wrt[2] = (uchar)(base_data[RT_I_HOUR] & 0x7f);
	p_magP->apspas.aps_wrt[3] = (uchar)(base_data[RT_I_MIN]  & 0x7f);
															// 処理月日時分=入庫月日時分

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト サービス券磁気データ生成
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_mag	: 磁気データのポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_service( m_gtservic *p_magS )
{
	long	pno;				// 駐車場No.

	p_magS->servic.svc_idc = 0x2d;							// サービス券
	if( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 ){			// 基本サービス券使用可
		pno = CPrmSS[S_SYS][1];								// 基本駐車場No.取得
	}else if( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 ){	// 拡張1サービス券使用可
		pno = CPrmSS[S_SYS][2];								// 拡張1駐車場No.取得
	}else if( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 ){	// 拡張2サービス券使用可
		pno = CPrmSS[S_SYS][3];								// 拡張2駐車場No.取得
	}else{													// その他は拡張3サービス券使用可とする
		pno = CPrmSS[S_SYS][4];								// 拡張3駐車場No.取得
	}
	p_magS->servic.svc_pno[0] = (uchar)(pno & 0x7f);		// P0～P6
	if( 0 != (pno & 0x0080) ){
		p_magS->servic.svc_pno[1] |= 0x40;					// P7
	}
	if( 0 != (pno & 0x0100) ){
		p_magS->servic.svc_pno[1] |= 0x20;					// P8
	}
	if( 0 != (pno & 0x0200) ){
		p_magS->servic.svc_pno[1] |= 0x10;					// P9
	}
	if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){
		p_magS->magformat.type = 1;							// 券種別=GTフォーマット
		p_magS->magformat.ex_pkno = (uchar)((pno & 0x3fc00) >> 10);
															// P10-P17
	}
	if( rt_ctrl.sub_mode == 2 ){							// 掛売券
															// サービス券種別=0固定
		p_magS->servic.svc_sno[0]  = (uchar)((kake_data[RT_KAKE_NO] >> 7) & 0x7f);
		p_magS->servic.svc_sno[1]  = (uchar)(kake_data[RT_KAKE_NO] & 0x7f);
															// 店No.
	}else{													// サービス券
		p_magS->servic.svc_pno[1] |= (uchar)((serv_data[RT_SERV_KIND] + 1) & 0x0f);
															// サービス券種別
		p_magS->servic.svc_sno[1]  = 0x01;					// 店No.(掛売先コード)=1固定
	}
	p_magS->servic.svc_sta[0] = 0;							// 有効開始年=0固定
	p_magS->servic.svc_sta[1] = 0;							// 有効開始月=0固定
	p_magS->servic.svc_sta[2] = 0;							// 有効開始日=0固定
	p_magS->servic.svc_end[0] = 99;							// 有効終了年=99固定
	p_magS->servic.svc_end[1] = 99;							// 有効終了月=99固定
	p_magS->servic.svc_end[2] = 99;							// 有効終了日=99固定
															// 無期限有効とする

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 該当月の最終日算出
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			short	: 最大日
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_day_max( void )
{
	long	year;
	long	mont;
	short	day;

	if( rt_ctrl.sub_mode == 0 ){							// 親画面
		if( rt_ctrl.pos == RT_I_DAY ){
			year = base_data[RT_I_YEAR];
			mont = base_data[RT_I_MONT];
		}else{
			year = base_data[RT_O_YEAR];
			mont = base_data[RT_O_MONT];
		}
	}else{													// 子画面
		if( rt_ctrl.pos == RT_S_DAY ){
			year = pass_data[RT_S_YEAR];
			mont = pass_data[RT_S_MONT];
		}else{
			year = pass_data[RT_E_YEAR];
			mont = pass_data[RT_E_MONT];
		}
	}
	if( year < 80 ){
		year += 2000;
	}else{
		year += 1900;
	}
	day = medget((short)year, (short)mont);

	return(day);
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 入出庫時刻ｾｯﾄ
//[]----------------------------------------------------------------------[]
///	@param[in]		num      : 精算開始ﾄﾘｶﾞｰ（入庫時刻 取得元判断用）<br>
///                 			    1:駐車券挿入（精算前）<br>
///                 			    2:駐車券挿入（掛売券）<br>
///                 			    3:駐車券挿入（中止券）<br>
///                 			    4:紛失精算<br>
///                 			    5:定期券挿入<br>
///	@return			ret      : 入出庫時刻判定(1980～2079年以内か?)0:OK 1:NG
///	@author			
///	@attention		グローバル変数car_in, car_ot, car_in_f, car_ot_fに入出庫時刻をセットする
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	:	2011/04/11<br>
///					Update	:	2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_set_tim( ushort num, struct clk_rec *clk_para )
{
	ushort	in_tim;
	ushort	out_tim;
	ushort	in_year;
	ushort	out_year;

	// set_tim()より必要部分を抜出して変更
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// 入車時刻ｸﾘｱ
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// 出車時刻ｸﾘｱ
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// 入車時刻Fixｸﾘｱ
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// 出車時刻Fixｸﾘｱ
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// 再計算用入車時刻ｸﾘｱ

	if( base_data[RT_I_YEAR] < 80 ){
		in_year = (ushort)(base_data[RT_I_YEAR] + 2000);
	}else{
		in_year = (ushort)(base_data[RT_I_YEAR] + 1900);
	}
	if( base_data[RT_O_YEAR] < 80 ){
		out_year = (ushort)(base_data[RT_O_YEAR] + 2000);
	}else{
		out_year = (ushort)(base_data[RT_O_YEAR] + 1900);
	}

	car_in.year = in_year;											// 入車 年
	car_in.mon  = base_data[RT_I_MONT];								//      月
	car_in.day  = base_data[RT_I_DAY];								//      日
	car_in.hour = base_data[RT_I_HOUR];								//      時
	car_in.min  = base_data[RT_I_MIN];								//      分

	car_ot.year = out_year;											// 出庫 年
	car_ot.mon  = (uchar)base_data[RT_O_MONT];						//      月
	car_ot.day  = (uchar)base_data[RT_O_DAY];						//      日
	car_ot.hour = (uchar)base_data[RT_O_HOUR];						//      時
	car_ot.min  = (uchar)base_data[RT_O_MIN];						//      分

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// 入車時刻Fix
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// 出車時刻Fix

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// 入庫時刻規定外?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// 出庫時刻規定外?
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (入庫日+1年)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// 出庫日normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		return( 1 );
	}

	in_tim = dnrmlzm((short)( car_in.year ),						/* (入庫日)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){
		return( 1 );
	}else if( in_tim == out_tim ){										// 入庫年月日=出庫年月日
		in_tim  = tnrmlz (0, 0, (short)car_in.hour, (short)car_in.min);
		out_tim = tnrmlz (0, 0, (short)car_ot.hour, (short)car_ot.min);
		if( in_tim > out_tim ){										/*								*/
			return( 1 );
		}
	}																/*								*/
	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 入出庫時刻ｾｯﾄ
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/12<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_disp_cal_result( void )
{
	uchar	base_max;										// 基本画面表示項目最大数
	char	disp_pos;										// 表示位置
	long	disp_data;										// 表示データ

	// 料金計算結果の表示
	dispclr();												// clear screen
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[1]);					// "＜料金計算テスト＞　　　　　　"
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF,SMSTR4[16]);			// "計算結果：　　　　　　　　　円"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[3]);					// "入庫時刻：  年  月  日  ：  　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[4]);					// "出庫時刻：  年  月  日  ：  　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[13]);					// "料金種別：　種　　　　　　　　"
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)			
//	base_max = RT_BASE_MAX;									// 項目数
	base_max = RT_BASE_MAX - 1 ;									// 項目数
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス（メンテナンス)	
	switch( rt_ctrl.sub_mode ){
	case  1:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[17]);				// "使用券種：定期券　種別　　　　"
		opedpl(6, 22, (ulong)pass_data[RT_PASS_KIND],2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		base_data[RT_PASS] = pass_data[RT_PASS_KIND];
		break;
	case  2:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[19]);				// "使用券種：掛売券　店　No　　　"
		opedpl(6, 22, (ulong)kake_data[RT_KAKE_NO],3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case  3:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[18]);				// "使用券種：サービス券　　種　　"
		grachr(6, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF,DAT1_4[serv_data[RT_SERV_KIND]]);
		break;
	default:
		break;
	}
	opedpl(1, 16, (ulong)ryo_buf.dsp_ryo,6, 0, 0, COLOR_FIREBRICK, LCD_BLINK_OFF);	// 計算結果(料金)表示
	for (disp_pos = 0; disp_pos < base_max; disp_pos++) {
		disp_data = base_data[disp_pos];
		RT_change_disp(0, 0, disp_pos, disp_data, 0);
	}
	if( !ryo_buf.dsp_ryo ){									// 表示料金=0円
		Fun_Dsp( FUNMSG2[56] );								// "　　　　　　　　　 登録  終了 "
	}else{
		Fun_Dsp( FUNMSG2[53] );								// "定期券 サ券 掛売券　　　 終了 "
	}
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト データバックアップ／リストア
//[]----------------------------------------------------------------------[]
///	@param[in]		char	kind = 0:バックアップ／1:リストア
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/19<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_bkp_rst_data( char kind )
{
	if( !kind ){
		memcpy(&base_data_bk[0], &base_data[0], sizeof(long)*RT_BASE_MAX);
		memcpy(&pass_data_bk[0], &pass_data[0], sizeof(long)*RT_PASSINFO_MAX);
		memcpy(&kake_data_bk[0], &kake_data[0], sizeof(long)*RT_KAKEINFO_MAX);
		memcpy(&serv_data_bk[0], &serv_data[0], sizeof(long)*RT_SERVKIND_MAX);
		memcpy(&rt_ctrl_bk2, &rt_ctrl, sizeof(t_RT_Ctrl));
															// データバックアップ
	}else{
		memcpy(&base_data[0], &base_data_bk[0], sizeof(long)*RT_BASE_MAX);
		memcpy(&pass_data[0], &pass_data_bk[0], sizeof(long)*RT_PASSINFO_MAX);
		memcpy(&kake_data[0], &kake_data_bk[0], sizeof(long)*RT_KAKEINFO_MAX);
		memcpy(&serv_data[0], &serv_data_bk[0], sizeof(long)*RT_SERVKIND_MAX);
		memcpy(&rt_ctrl, &rt_ctrl_bk2, sizeof(t_RT_Ctrl));
															// データリストア
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト エラー表示クリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/22<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_clear_error( void )
{

	if( rt_ctrl.mode == 0 ){
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[2]);						// "各項目を入力して下さい　　　　"
		Fun_Dsp( FUNMSG2[2] );								// "　⊂　　⊃　　　　 書込  終了 "
	}else{
		switch( rt_ctrl.sub_mode ){
		case  1:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[8]);					// "定期券情報を入力して下さい　　"
			Fun_Dsp( FUNMSG2[2] );							// "　⊂　　⊃　　　　 書込  終了 "
			break;
		case  2:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[12]);				// "掛売先情報を入力して下さい　　"
			Fun_Dsp( FUNMSG2[1] );							// "　　　　　　　　　 書込  終了 "
			break;
		case  3:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[14]);				// "サービス券種別を入力して下さい"
			Fun_Dsp( FUNMSG2[54] );							// "　　　　　　 切換  書込  終了 "
			break;
		default:
			break;
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト ログ用データ編集
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_edit_log_data( T_FrmReceipt *p_pri_data )
{
	ushort	num;
	
	// 以下、領収証再発行処理(UsMnt_Receipt())を参考に作成
	RT_PayData.TInTime.Year = car_in_f.year;						// 入庫 年
	RT_PayData.TInTime.Mon  = car_in_f.mon;							//      月
	RT_PayData.TInTime.Day  = car_in_f.day;							//      日
	RT_PayData.TInTime.Hour = car_in_f.hour;						//      時
	RT_PayData.TInTime.Min  = car_in_f.min;							//      分
	RT_PayData.TOutTime.Year = car_ot_f.year;						// 出庫 年
	RT_PayData.TOutTime.Mon  = car_ot_f.mon;						//      月
	RT_PayData.TOutTime.Day  = car_ot_f.day;						//      日
	RT_PayData.TOutTime.Hour = car_ot_f.hour;						//      時
	RT_PayData.TOutTime.Min  = car_ot_f.min;						//      分
	if( ryo_buf.ryo_flg < 2 ){										// 駐車券精算処理
		RT_PayData.WPrice = ryo_buf.tyu_ryo;						// 駐車料金セット
	}else{
		RT_PayData.WPrice = ryo_buf.tei_ryo;						// 定期料金セット
	}

	RT_PayData.Kikai_no    = (uchar)CPrmSS[S_PAY][2];		// 機械No.セット
	RT_PayData.Seisan_kind = 0xFF;							// 精算方式=料金計算テストセット

	RT_PayData.Oiban.w = 999999L;							// 追い番=999999とする
	RT_PayData.Oiban.i = 999999L;							// 追い番=999999とする

	RT_PayData.syu = (char)(ryo_buf.syubet + 1);			// 駐車種別

	num = ryo_buf.pkiti - 1;
	RT_PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// 接客用駐車位置番号

	p_pri_data->prn_data = (Receipt_data *)&RT_PayData;		// 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ
	p_pri_data->prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	p_pri_data->reprint = OFF;								// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）


	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト 有効駐車券No.／定期券ID取得
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: 磁気データのポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/28<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_get_valid_no( char *p_data )
{
	m_gtapspas	*p_magP;		// 定期券
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	long	i;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
	long	pno = 0;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	long	pass_id = 0;
	long	pass_id = 1;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

	if( p_data[MAG_ID_CODE] == 0x1a ){						// 定期券
		p_magP   = (m_gtapspas *)p_data;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//		for( i = 1; i < PAS_MAX+1; i++ ){
//			if( !pas_tbl[i-1].BIT.INV ){					// 有効
//				if(( !CPrmSS[S_SYS][61] ) ||				// 定期テーブル分割①なし
//				   (( CPrmSS[S_SYS][61] ) && ( i < CPrmSS[S_SYS][61] ))){
//															// 定期テーブル分割①より小
//					pno = CPrmSS[S_SYS][1];					// 基本駐車場No.とする
//					pass_id = i;							// 定期券IDはそのまま
//				}else{
//					if(( !CPrmSS[S_SYS][62] ) ||			// 定期テーブル分割②なし
//					   (( CPrmSS[S_SYS][62] ) && ( i < CPrmSS[S_SYS][62] ))){
//															// 定期テーブル分割②より小
//						pno = CPrmSS[S_SYS][2];				// 拡張１駐車場No.とする
//						pass_id = i - CPrmSS[S_SYS][61] + 1;// 定期券ID=i-定期テーブル分割①+1
//					}else{
//						if(( !CPrmSS[S_SYS][63] ) ||		// 定期テーブル分割③なし
//						   (( CPrmSS[S_SYS][63] ) && ( i < CPrmSS[S_SYS][63] ))){
//															// 定期テーブル分割③より小
//							pno = CPrmSS[S_SYS][3];			// 拡張２駐車場No.とする
//							pass_id = i - CPrmSS[S_SYS][62] + 1;
//															// 定期券ID=i-定期テーブル分割②+1
//						}else{
//							pno = CPrmSS[S_SYS][4];			// 拡張３駐車場No.とする
//							pass_id = i - CPrmSS[S_SYS][63] + 1;
//															// 定期券ID=i-定期テーブル分割③+1
//						}
//					}
//				}
//				break;
//			}
//		}
//		if( !pno ){
//			// 有効定期が見つからなかった場合、基本駐車場の定期券ID=1とする
//			pno = CPrmSS[S_SYS][1];							// 基本駐車場No.とする
//			pass_id = 1;									// 定期券ID=1とする
//		}
		pno = CPrmSS[S_SYS][1];							// 基本駐車場No.とする
		pass_id = 1;									// 定期券ID=1とする
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
		p_magP->apspas.aps_pno[0] = (uchar)(pno & 0x7f);	// 駐車場No.P0～P6設定
		if( 0 != (pno & 0x0080) ){
			p_magP->apspas.aps_pno[1] |= 0x40;				// 駐車場No.P7設定
		}
		if( 0 != (pno & 0x0100) ){
			p_magP->apspas.aps_pno[1] |= 0x20;				// 駐車場No.P8設定
		}
		if( 0 != (pno & 0x0200) ){
			p_magP->apspas.aps_pno[1] |= 0x10;				// 駐車場No.P9設定
		}
		if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){
			p_magP->magformat.type = 1;						// 券種別=GTフォーマット
			p_magP->magformat.ex_pkno = (uchar)((pno & 0x3fc00) >> 10);
															// P10-P17
		}
		p_magP->apspas.aps_pcd[0] |= (uchar)((pass_id >> 7) & 0x7f);
		p_magP->apspas.aps_pcd[1] |= (uchar)(pass_id & 0x7f);// 定期券ID設定
	}

	return;
}
//[]-----------------------------------------------------------------------------------------------[]
///	@brief			料金計算テスト 料金計算処理メイン
//[]-----------------------------------------------------------------------------------------------[]
///	PARAMETER	: r_knd	; 料金計算種別															   |*/
///						;   0:駐車券															   |*/
///						;   1:定期券															   |*/
///						;   2:ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ														   |*/
///						;   3:ｻｰﾋﾞｽ券															   |*/
///						;   4:中止券(未使用)													   |*/
///						;   6:掛売駐車券(未使用)												   |*/
///						; 100:修正精算															   |*/
///				  num	; 内部処理用駐車位置番号(1～324)										   |*/
///	 RETURN VALUE: void	;																		   |*/
//[]----------------------------------------------------------------------------------------------[]
///	@author			Y.shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---------------------------[]
static	void	RT_ryo_cal2( char r_knd, ushort num )							/*								*/
{																	/*								*/
	ulong	sot_tax = 0L;											/*								*/
	ulong	dat = 0L;												/*								*/
	struct	REQ_RKN		*rhs_p;										/*								*/
	short	calcreqnum	;											/* 券毎料金計算要求数			*/
	char	cnt;													/*								*/
	char	i;														/*								*/
	uchar	wrcnt = 0;												/*								*/
	ushort	wk_dat;													/*								*/
	ushort	wk_dat2;												/*								*/
	ulong	wk_dat3;												/*								*/
	ulong	wk_pay_ryo;				//再精算用プリペイド金額格納エリア

	struct	CAR_TIM	Adjustment_Beginning;							/*								*/
																	/*								*/
	memcpy( &Adjustment_Beginning, &car_ot, sizeof( Adjustment_Beginning ) );	/*					*/
																	/*								*/
	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
																	/*								*/
	if( r_knd == 0 ){																/*								*/
		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* 現在計算後格納ｴﾘｱｸﾘｱ			*/
		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* 料金ﾊﾞｯﾌｧｸﾘｱｰ				*/
		ryo_buf.ryo_flg = 0;										/* 通常料金ｾｯﾄ					*/
		ryo_buf.pkiti = num;										/* 駐車位置番号ｾｯﾄ				*/
		memset( &RT_PayData, 0, sizeof( Receipt_data ) );			/* 精算情報,領収証印字ﾃﾞｰﾀ		*/
		memset( &PayData, 0, sizeof( Receipt_data ) );				/* 精算情報,領収証印字ﾃﾞｰﾀ		*/
// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
		memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	/* 精算データ保持				*/
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
		memset( &PPC_Data_Detail, 0, sizeof( PPC_Data_Detail ));	/* ﾌﾟﾘﾍﾟ詳細印字ｴﾘｱｸﾘｱ			*/	
		ntnet_nmax_flg = 0;											/*　最大料金ﾌﾗｸﾞｸﾘｱ	*/
		memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );	/* NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ	*/
		memset( RT_PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ	*/
		memset( card_use, 0, sizeof( card_use ) );					/* 1精算ｶｰﾄﾞ毎使用枚数ﾃｰﾌﾞﾙ		*/
		memset( card_use2, 0, sizeof( card_use2 ) );				/* 1精算ｻｰﾋﾞｽ券毎使用枚数ﾃｰﾌﾞﾙ	*/
		CardUseSyu = 0;												/* 1精算の割引種類の件数ｸﾘｱ		*/
		discount_tyushi = 0;										/*　割引額　　　　　　　　　　　*/
		memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));		/*	詳細中止ｴﾘｱｸﾘｱ				*/
		Flap_Sub_Num = 0;											/*	詳細中止ｸﾘｱ					*/
		RT_vl_carchg( num );										/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ryo_buf_n.require = ryo_buf.zankin;								/*								*/
	ryo_buf_n.dis     = ryo_buf.waribik;							/*								*/
	ryo_buf_n.nyu     = ryo_buf.nyukin;								/*								*/
																	/*								*/
	tol_dis = ryo_buf.waribik;										/*								*/
																	/*								*/
	ec09();															/* VL料金ﾊﾟﾗﾒｰﾀｾｯﾄ				*/
	if( CPrmSS[S_STM][1] == 0 ){									/* 全共通ｻｰﾋﾞｽﾀｲﾑ,ｸﾞﾚｰｽﾀｲﾑ		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ｻｰﾋﾞｽﾀｲﾑget					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ｸﾞﾚｰｽﾀｲﾑget					*/
	}else{															/* 種別毎ｻｰﾋﾞｽﾀｲﾑ				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(rysyasu-1))];		/* 種別毎ｻｰﾋﾞｽﾀｲﾑget			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(rysyasu-1))];		/* 種別毎ｸﾞﾚｰｽﾀｲﾑget			*/
	}																/*								*/

	SvsTime_Syu[num-1] = rysyasu;								/* 精算時の料金種別ｾｯﾄ（ﾗｸﾞﾀｲﾑ処理用）*/

	et02();															/* 料金計算						*/
	for( calcreqnum =  0 ;											/* 料金計算要求数				*/
		  calcreqnum < (short)req_crd_cnt	;						/* ex.各券毎に料金計算			*/
		  calcreqnum ++					)							/* 処理数が異なる				*/
	{																/*								*/
		memcpy( &req_rkn, req_crd + calcreqnum,						/* 料金計算要求複写				*/
				sizeof(req_rkn)				) ;						/* ex.2 件目以降→要求域		*/
		et02()	;													/* 料金計算						*/
	}																/*								*/
																	/*								*/
	if( tki_flg != OFF )											/* 時間帯定期後時間割引 		*/
	{																/* ｷｰ処理なら					*/
		if( tki_ken ){												/*								*/
			ryo_buf.dis_fee = 0L;									/* 前回料金割引額ｸﾘｱ			*/
			ryo_buf.dis_tim = 0L;									/* 前回時間割引額ｸﾘｱ			*/
			ryo_buf.fee_amount = 0L;								/* 料金割引額ｸﾘｱ				*/
			ryo_buf.tim_amount = 0L;								/* 割引時間数ｸﾘｱ				*/
			wk_pay_ryo = PayData_Sub.pay_ryo;
			memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
			PayData_Sub.pay_ryo = wk_pay_ryo;
			wrcnt_sub = 0;
			discount_tyushi = 0;									/*　割引額　　　　　　　　　　　　　　　　　　*/
		}															/*								*/
		for( i = 0; i < (short)tki_ken; i++ )						/* 保存して有る料金計算 		*/
		{															/* 分ﾙｰﾌﾟ						*/
			memset( &req_rkn.param,0x00,21);						/* 料金要求ﾃｰﾌﾞﾙｸﾘｱ				*/
			memcpy( (char *)&req_rkn,								/* 保存してた料金計算要 		*/
					(char *)&req_tki[i],10 );						/* 求を再び要求					*/
			if((req_rkn.param == RY_FRE_K)							/* 拡張1回数券割引要求			*/
			 ||(req_rkn.param == RY_FRE)							/* 回数券割引要求				*/
			 ||(req_rkn.param == RY_FRE_K2)							/* 拡張2回数券割引要求			*/
			 ||(req_rkn.param == RY_FRE_K3)							/* 拡張3回数券割引要求			*/
			 ||(req_rkn.param == RY_PCO_K2)							/* 拡張2ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO_K3)							/* 拡張3ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO_K)							/* 拡張1ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO))							/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ					*/
			{														/*								*/
			   req_rkn.param = 0xff;								/*								*/
			}														/*								*/
			et02();													/* 料金計算処理					*/
		}															/*								*/
		tki_ken = 0;												/* 料金計算保存券数ｸﾘｱ			*/
		re_req_flg = OFF;											/* 時間帯定期後時間割引 		*/
	}																/* ﾌﾗｸﾞﾘｾｯﾄ						*/
																	/*								*/
	ryo_buf.zankin  = ryo_buf_n.require ;							/* 駐車料金ｾｯﾄ					*/
	ryo_buf.tax     = ryo_buf_n.tax;								/* 税金ｾｯﾄ						*/
	ryo_buf.waribik = ryo_buf_n.dis;								/* 割引額						*/
	if( OPECTL.Ope_mod != 22 )										/* 定期更新時でないなら			*/
	ryo_buf.syubet  = (char)(rysyasu - 1);							/* 料金種別ｾｯﾄ(種別切換後)		*/
																	/*								*/
	if( r_knd == 1 )												/* 定期券処理?					*/
	{																/*								*/
		ryo_buf.ryo_flg += 2;										/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* 駐車券精算処理				*/
	{																/*								*/
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス（メンテナンス)
//		ryo_buf.tik_syu = rysyasu;									/* 料金種別						*/
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;							/* 料金種別					*/
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)		
	}																/*								*/
	else															/*								*/
	{															/*								*/
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//		ryo_buf.pas_syu = rysyasu;									/* 定期券の料金種別				*/
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
		ryo_buf.tei_ryo = ryo_buf_n.ryo;							/* 定期券料金ｾｯﾄ				*/
	}																/*								*/
																	/*								*/
	/*====================================================*/		/*								*/
	// 定期券処理前の各割引処理を削除する							/*								*/
	/*====================================================*/		/*								*/
	for( cnt = 0; cnt < rhspnt; cnt++ )								/* 料金計算後ﾃﾞｰﾀ数になるまで	*/
	{																/*								*/
		rhs_p = &req_rhs[cnt];										/*								*/
		if(( rhs_p->param == RY_PKC		||							/* 料金計算要求が定期券処理の場合	*/
			 rhs_p->param == RY_PKC_K	||							/*								*/
			 rhs_p->param == RY_PKC_K2	||							/*								*/
			 rhs_p->param == RY_PKC_K3	||				/*								*/
			 ((rhs_p->param == RY_KCH) && (( rhs_p->data[1] >> 16 )== 2))) &&	// 定期券車種切替
			 cnt != 0 )
		{															/*								*/
			for( i = (char)(cnt-1); i > 0; i-- )					/*								*/
			{														/*								*/
				rhs_p = &req_rhs[i];								/*								*/
				if( rhs_p->param == RY_TWR || 						/* 料金割引処理					*/
					rhs_p->param == RY_RWR || 						/* 時間割引処理					*/
					rhs_p->param == RY_SKC || 						/* ｻｰﾋﾞｽ券処理					*/
					rhs_p->param == RY_SKC_K ||						/* 拡張ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_SKC_K2 ||					/* 拡張2ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_SKC_K3 ||					/* 拡張3ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_KAK ||						/* 掛売券処理					*/
					rhs_p->param == RY_KAK_K ||						/* 拡張1掛売券					*/
					rhs_p->param == RY_KAK_K2 ||					/* 拡張2掛売券					*/
					rhs_p->param == RY_KAK_K3 ||					/* 拡張3掛売券					*/
					rhs_p->param == RY_SSS ||						/* 修正							*/
					rhs_p->param == RY_CSK ||						/* 精算中止ｻｰﾋﾞｽ券				*/
					rhs_p->param == RY_SNC			)				/* 店割引処理					*/
				{													/*								*/
					rhs_p->param = 0xff;							/* 要求ﾃﾞｰﾀFFｸﾘｱ				*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	/*==============================================================================================*/
	/* 料金ﾊﾟﾗﾒｰﾀより領収証(個別精算情報含む)、精算ﾃﾞｰﾀの割引部分を全てここで作成する				*/
	/*==============================================================================================*/
	if( r_knd != 0 )												/* 駐車券以外?					*/
	{																/*								*/
		memset( RT_PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ	*/
																	/*								*/
		for( cnt = 0; cnt < rhspnt; cnt++ )							/*								*/
		{															/*								*/
			rhs_p = &req_rhs[cnt];									/*								*/
			switch( rhs_p->param )									/* 料金計算要求ｺｰﾄﾞにより分岐	*/
			{														/*								*/
			  case RY_SKC:											/* 基本ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K:										/* 拡張1ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K2:										/* 拡張2ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K3:										/* 拡張3ｻｰﾋﾞｽ券					*/
				wk_dat = (ushort)((rhs_p->data[0] &					/* 種類ｾｯﾄ						*/
									0xffff0000) >> 16);				/*								*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* 枚数が0なら1ｾｯﾄ				*/
				}													/*								*/
																	/*								*/
				if(( wk_dat >= 1 )&&( wk_dat <= SVS_MAX )){			/* ｻｰﾋﾞｽ券A～O					*/
					// 精算ﾃﾞｰﾀ用									/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//					wk_dat2 = (ushort)CPrmSS[S_SER][1+3*(wk_dat-1)];/*								*/
					wk_dat2 = (ushort)prm_get( COM_PRM, S_SER, (short)(1+3*(wk_dat-1)), 1, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					if( wk_dat2 == 1 ){								/*								*/
						wk_dat2 = NTNET_SVS_T;						/* 割引種別=ｻｰﾋﾞｽ券(時間)		*/
					}else{											/*								*/
						wk_dat2 = NTNET_SVS_M;						/* 割引種別=ｻｰﾋﾞｽ券(料金)		*/
					}												/*								*/
																	/*								*/
					if( rhs_p->param == RY_SKC ){					/* 基本駐車場№					*/
						wk_dat3 = CPrmSS[S_SYS][1];					/* 駐車場№ｾｯﾄ					*/
					}else if( rhs_p->param == RY_SKC_K ){			/* 拡張1駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][2];					/* 駐車場№ｾｯﾄ					*/
					}else if( rhs_p->param == RY_SKC_K2 ){			/* 拡張2駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][3];					/* 駐車場№ｾｯﾄ					*/
					}else{											/* 拡張3駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][4];					/* 駐車場№ｾｯﾄ					*/
					}												/*								*/
																	/*								*/
					for( i=0; i < WTIK_USEMAX; i++ ){				/*								*/
						if(( RT_PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* 駐車場№が等しい?*/
						   ( RT_PayData.DiscountData[i].DiscSyu == wk_dat2 )&&		/* 割引種別が等しい?*/
						   ( RT_PayData.DiscountData[i].DiscNo == wk_dat )&&		/* 割引区分が等しい?*/
						   ( RT_PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x0000ffff) ))	/* 割引情報1(掛売り先)が等しい? */
						{											/*								*/
							RT_PayData.DiscountData[i].DiscCount +=	/* 使用枚数						*/
										(ushort)rhs_p->data[1];		/*								*/
							RT_PayData.DiscountData[i].Discount += rhs_p->data[2];	/* 割引額			*/
							if( wk_dat2 == NTNET_SVS_T ){			/* 時間割引?					*/
								RT_PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*			*/
									prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));	/* 割引情報2(割引時間数：分) */
							}										/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){	/* 割引該当無し					*/
						RT_PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* 駐車場№					*/
						RT_PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* 割引種別					*/
						RT_PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* 割引区分(ｻｰﾋﾞｽ券A～C)	*/
						RT_PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
										(ushort)rhs_p->data[1];		/*								*/
						RT_PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* 割引額			*/
						RT_PayData.DiscountData[wrcnt].DiscInfo1 =		/*								*/
									rhs_p->data[0] & 0x0000ffff;	/* 割引情報1(掛売り先)			*/
						if( wk_dat2 == NTNET_SVS_T ){				/* 時間割引?					*/
							RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*			*/
								prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));/* 割引情報2(割引時間数) */
						}else{										/*								*/
							RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;	/* 割引情報2(未使用) */
						}											/*								*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 						RT_PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用			*/
// 						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算) */
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
						wrcnt++;									/*								*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_KAK:											/* 基本掛売券(店№割引)			*/
			  case RY_KAK_K:										/* 拡張1掛売券(店№割引)		*/
			  case RY_KAK_K2:										/* 拡張2掛売券(店№割引)		*/
			  case RY_KAK_K3:										/* 拡張3掛売券(店№割引)		*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* 枚数が0なら1ｾｯﾄ				*/
				}													/*								*/
				// 精算ﾃﾞｰﾀ用										/*								*/
				wk_dat = (ushort)rhs_p->data[0];					/* 店№							*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				wk_dat2 = (ushort)CPrmSS[S_STO][1+3*(wk_dat-1)];	/*								*/
				wk_dat2 = (ushort)prm_get( COM_PRM, S_STO, (short)(1+3*(wk_dat-1)), 1, 1 );   /*	*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
				if( wk_dat2 == 1 ){									/*								*/
					wk_dat2 = NTNET_KAK_T;							/* 割引種別=店割引(時間)		*/
				}else{												/*								*/
					wk_dat2 = NTNET_KAK_M;							/* 割引種別=店割引(料金)		*/
				}													/*								*/
																	/*								*/
				if( rhs_p->param == RY_KAK ){						/* 基本駐車場№					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_KAK_K ){				/* 拡張1駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_KAK_K2 ){				/* 拡張2駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* 駐車場№ｾｯﾄ					*/
				}else{												/* 拡張3駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* 駐車場№ｾｯﾄ					*/
				}													/*								*/
																	/*								*/
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/
					if(( RT_PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* 駐車場№が等しい?	*/
					   ( RT_PayData.DiscountData[i].DiscSyu == wk_dat2 )&&	/* 割引種別が等しい?		*/
					   ( RT_PayData.DiscountData[i].DiscNo == wk_dat ))	/* 割引区分が等しい?		*/
					{												/*								*/
						RT_PayData.DiscountData[i].DiscCount +=		/* 使用枚数						*/
									(ushort)rhs_p->data[1];			/*								*/
						RT_PayData.DiscountData[i].Discount += rhs_p->data[2];	/* 割引額				*/
						if( wk_dat2 == NTNET_KAK_T ){				/* 時間割引?					*/
							RT_PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*				*/
								prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));	/* 割引情報2(割引時間数：分) */
						}											/*								*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* 割引該当無し					*/
					RT_PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* 駐車場№						*/
					RT_PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* 割引種別						*/
					RT_PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* 割引区分(店№)				*/
					RT_PayData.DiscountData[wrcnt].DiscCount =			/* 使用枚数						*/
										(ushort)rhs_p->data[1];		/*								*/
					RT_PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* 割引額				*/
					RT_PayData.DiscountData[wrcnt].DiscInfo1 = 0L;		/* 割引情報1(未使用)			*/
					if( wk_dat2 == NTNET_KAK_T ){					/* 時間割引?					*/
						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*				*/
							prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));/* 割引情報2(割引時間数)*/
					}else{											/*								*/
						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;/* 割引情報2(未使用) */
					}												/*								*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					RT_PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用				*/
// 					RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算)*/
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		CardUseSyu = wrcnt;											/* 1精算の割引種類の件数ｸﾘｱ		*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg <= 1 )										/* 定期使用無し精算？			*/
	{																/*								*/
		if(	ryo_buf.tyu_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tyu_ryo -	ryo_buf.waribik;	/* 課税対象額＝駐車料金 － 割引額計	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	} else {														/* 定期使用						*/
		if(	ryo_buf.tei_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tei_ryo -	ryo_buf.waribik;	/* 課税対象額＝定期券駐車料金 － 割引額計	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( CPrmSS[S_CAL][19] )											/* 税率設定あり					*/
	{																/*								*/
		if( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )					/* 外税?						*/
		{															/*								*/
			sot_tax = ryo_buf.tax;									/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	switch( ryo_buf.ryo_flg )										/*								*/
	{																/*								*/
		case 0:														/* 通常(定期使用無し)精算		*/
			if(( ryo_buf.tyu_ryo + sot_tax )						/* 駐車料金＋外税 ＜ 入金額＋割引額計?	*/
					< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay ))	/*								*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay )	/*								*/
						- ( ryo_buf.tyu_ryo + sot_tax );			/*								*/
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* 多い入金分を戻す				*/
				}													/*								*/
			}														/*								*/

			if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )	/* 表示料金＝(駐車料金＋外税)	*/
								- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	－割引額計	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* 表示料金＝０					*/
				if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		case 2:														/* 通常(定期使用)精算			*/
																	/* 入金額 = 駐車料金			*/
			if(( ryo_buf.tei_ryo + sot_tax )						/* 定期駐車料金＋外税 ＜ 入金額＋割引額計？*/
			< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*					*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)	/*				*/
					- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* 多い入金分を戻す				*/
				}													/*								*/
			}														/*								*/
			if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*	*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* 表示料金＝(定期駐車料金＋外税)	*/
								- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		－割引額計	*/
			} else {												/*								*/
			ryo_buf.dsp_ryo = 0l;									/* 表示料金＝０					*/
				if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
	}																/*								*/
	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
	memcpy( &car_ot, &Adjustment_Beginning, sizeof( car_ot ) );		/*								*/
	return;															/*								*/
}																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車券ﾃﾞｰﾀ処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: RT_vl_carchg( no )															   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: Y.shiraishi																	   |*/
/*| Date		: 2012-11-08																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RT_vl_carchg( ushort no )
{
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	 if( no >= 1 && no <= LOCK_MAX ){							// 駐車位置番号ﾁｪｯｸ(1～LOCK_MAX)
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)	

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );		// 駐車券ﾃﾞｰﾀ(vl_tik)初期化(0ｸﾘｱ)
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//		 vl_tik.syu = LockInfo[no-1].ryo_syu;					// 種別ｾｯﾄ(駐車位置設定毎)
		vl_tik.syu = base_data[RT_KIND]+1;
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
		cr_dat_n = 0x24;										// 券IDｺｰﾄﾞｾｯﾄ(精算前券)
		vl_now = V_CHM;											// 読込み券種別ｾｯﾄ(精算前券)

		vl_tik.chk = 0;											// 精算前券ｾｯﾄ

		vl_tik.cno = KIHON_PKNO;								// 駐車場№ｾｯﾄ(基本駐車場№)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// 発券機No.
		vl_tik.hno = 0;											// 発券No.
		vl_tik.mno = 0;											// 店№ｾｯﾄ

		vl_tik.tki = car_in_f.mon;								// 入車月ｾｯﾄ
		vl_tik.hii = car_in_f.day;								// 入車日ｾｯﾄ
		vl_tik.jii = car_in_f.hour;								// 入車時ｾｯﾄ
		vl_tik.fun = car_in_f.min;					 			// 入車分ｾｯﾄ

		syashu       = vl_tik.syu	;							// 車種

		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// 入車年
		car_in.mon   = vl_tik.tki;								//     月
		car_in.day   = vl_tik.hii;								//     日
		car_in.hour  = vl_tik.jii;								//     時
		car_in.min   = vl_tik.fun;								//     分
		car_in.week  = (char)youbiget( car_in.year ,			//     曜日
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// 出庫年
		car_ot.mon  = hzuk.m;									//     月
		car_ot.day  = hzuk.d;									//     日
		car_ot.week = hzuk.w;									//     曜日
		car_ot.hour = jikn.t;									//     時
		car_ot.min  = jikn.m;									//     分

		Flap_Sub_Flg = 0;
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//	 }
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
}

//[]----------------------------------------------------------------------[]
///	@brief			料金計算テスト log登録
//[]----------------------------------------------------------------------[]
/*| MODULE NAME  : Log_regist( Lno )                                       |*/
/*| PARAMETER    : ﾛｸﾞ№                                                   |*/
/*| RETURN VALUE : void                                                    |*/
//[]----------------------------------------------------------------------[]
///	@author			Y.shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void RT_log_regist( void )
{
	memcpy( &RT_PAY_LOG_DAT.RT_pay_log_dat[RT_PAY_LOG_DAT.RT_pay_wtp], &RT_PayData, sizeof( Receipt_data ) );
	if( RT_PAY_LOG_DAT.RT_pay_count < RT_PAY_LOG_CNT ){
		RT_PAY_LOG_DAT.RT_pay_count++;	/* RAM登録件数を+1 */
	}

	RT_PAY_LOG_DAT.RT_pay_wtp++;			/* ﾗｲﾄﾎﾟｲﾝﾀ +1 */
	if( RT_PAY_LOG_DAT.RT_pay_wtp >= RT_PAY_LOG_CNT ){
		RT_PAY_LOG_DAT.RT_pay_wtp = 0;
	}

	return;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		ログデータクリア処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDataClr( LogSyu )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	= ログデータ種別					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret = OK/NG									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogDataClr_CheckBufferFullRelease(ushort LogSyu )
{
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
#if 0		// UT4000の処理を使用する
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	switch( LogSyu ){				// ログデータ種別？
		case	LOG_ERROR:			// エラー情報
			Log_CheckBufferFull(FALSE, eLOG_ERROR, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_ALARM:			// アラーム情報
			Log_CheckBufferFull(FALSE, eLOG_ALARM, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_OPERATE:		// 操作情報
			Log_CheckBufferFull(FALSE, eLOG_OPERATE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_POWERON:		// 停復電情報
			Log_CheckBufferFull(FALSE, eLOG_POWERON, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_TTOTAL:			// Ｔ合計情報
			Log_CheckBufferFull(FALSE, eLOG_TTOTAL, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//			Log_CheckBufferFull(FALSE, eLOG_LCKTTL, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
			break;

		case	LOG_GTTOTAL:		// ＧＴ合計情報
			Log_CheckBufferFull(FALSE, eLOG_GTTOTAL, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_COINBOX:		// コイン金庫情報
			Log_CheckBufferFull(FALSE, eLOG_COINBOX, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_NOTEBOX:		// 紙幣金庫情報
			Log_CheckBufferFull(FALSE, eLOG_NOTEBOX, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_MONEYMANAGE:	// 釣銭管理情報
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_PAYMENT:		// 個別精算情報
			Log_CheckBufferFull(FALSE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_ABNORMAL:		// 不正・強制出庫情報
			Log_CheckBufferFull(FALSE, eLOG_ABNORMAL, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_MONITOR:		// モニタ情報
			Log_CheckBufferFull(FALSE, eLOG_MONITOR, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_CREUSE:			// クレジット利用明細ログ
			Log_CheckBufferFull(FALSE, eLOG_CREUSE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case	LOG_EDYARM:			// Edyアラーム取引ログ
//			Log_CheckBufferFull(FALSE, eLOG_EDYARM, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
//			break;
//
//		case	LOG_EDYSHIME:			// Edy締め記録ログ
//			Log_CheckBufferFull(FALSE, eLOG_EDYSHIME, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		case LOG_HOJIN_USE:
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_NGLOG:						// 不正ログ
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		case	LOG_REMOTE_SET:					// 遠隔料金設定ログ
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_SET, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
//		case	LOG_ENTER:						// 入庫ログ
//			Log_CheckBufferFull(FALSE, eLOG_ENTER, eLOG_TARGET_REMOTE);		// バッファフル解除チェック
//			break;
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
		
		case	LOG_PARKING:					// 駐車台数データ
			Log_CheckBufferFull(FALSE, eLOG_PARKING, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;
		
		case	LOG_LONGPARK:					// 長期駐車データ
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//			Log_CheckBufferFull(FALSE, eLOG_LONGPARK, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			Log_CheckBufferFull(FALSE, eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
			break;
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//		case LOG_IOLOG:
//			Log_CheckBufferFull(FALSE, eLOG_IOLOG, eLOG_TARGET_REMOTE);		// バッファフル解除チェック
//			break;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)

		case	LOG_MNYMNG_SRAM:				// 釣銭管理情報(SRAM)
			Log_CheckBufferFull(FALSE, eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);		// バッファフル解除チェック
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);		// バッファフル解除チェック
			break;

		case	LOG_REMOTE_MONITOR:
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;

		default:								// その他（ログデータ種別エラー）
			break;
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
#endif
	short	target;					// チェック対象
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
	short	targetList[NTNET_TARGET_MAX];
	int		targetCount = 0;		// 対象ターゲット数
	int		index = 0;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
//	if(_is_ntnet_normal()) {		// NT-NET
//		target = eLOG_TARGET_NTNET;
//	}
//	else if(_is_ntnet_remote()) {	// 遠隔NT-NET
//		target = eLOG_TARGET_REMOTE;
//	}
//	else {
//		return;
//	}
	memset(targetList, 0, sizeof(targetList));
	if(_is_ntnet_normal()) {		// NT-NET
		targetList[targetCount] = eLOG_TARGET_NTNET;
		++targetCount;
	}

	if(_is_ntnet_remote()) {		// 遠隔NT-NET
		targetList[targetCount] = eLOG_TARGET_REMOTE;
		++targetCount;
	}

	if(targetCount <= 0) {
		// 端末間、遠隔共設定がないなら何もしない
		return;
	}

	for(index = 0; index < targetCount; ++index) {
		target = targetList[index];
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
	switch( LogSyu ){				// ログデータ種別？
		case	LOG_ERROR:			// エラー情報
			Log_CheckBufferFull(FALSE, eLOG_ERROR, target);				// バッファフル解除チェック
			break;

		case	LOG_ALARM:			// アラーム情報
			Log_CheckBufferFull(FALSE, eLOG_ALARM, target);				// バッファフル解除チェック
			break;

		case	LOG_OPERATE:		// 操作情報
			Log_CheckBufferFull(FALSE, eLOG_OPERATE, target);				// バッファフル解除チェック
			break;

		case	LOG_POWERON:		// 停復電情報
			Log_CheckBufferFull(FALSE, eLOG_POWERON, target);				// バッファフル解除チェック
			break;

		case	LOG_TTOTAL:			// Ｔ合計情報
			Log_CheckBufferFull(FALSE, eLOG_TTOTAL, target);				// バッファフル解除チェック
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
//			Log_CheckBufferFull(FALSE, eLOG_LCKTTL, target);				// バッファフル解除チェック
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
			break;

		case	LOG_GTTOTAL:		// ＧＴ合計情報
			Log_CheckBufferFull(FALSE, eLOG_GTTOTAL, target);				// バッファフル解除チェック
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
//// MH364300 GG119A17(S) // MH341110(S) A.Iiizumi 2017/12/20 前回T/GT合計で車室位置毎集計の印字が合わない不具合修正(GT用車室位置毎集計ログ追加) (共通改善№1392)
//			Log_CheckBufferFull(FALSE, eLOG_GT_LCKTTL, target);				// バッファフル解除チェック
//// MH364300 GG119A17(E) // MH341110(E) A.Iiizumi 2017/12/20 前回T/GT合計で車室位置毎集計の印字が合わない不具合修正(GT用車室位置毎集計ログ追加) (共通改善№1392)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
			break;

		case	LOG_COINBOX:		// コイン金庫情報
			Log_CheckBufferFull(FALSE, eLOG_COINBOX, target);				// バッファフル解除チェック
			break;

		case	LOG_NOTEBOX:		// 紙幣金庫情報
			Log_CheckBufferFull(FALSE, eLOG_NOTEBOX, target);				// バッファフル解除チェック
			break;

		case	LOG_MONEYMANAGE:	// 釣銭管理情報
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, target);			// バッファフル解除チェック
			break;

		case	LOG_PAYMENT:		// 個別精算情報
			Log_CheckBufferFull(FALSE, eLOG_PAYMENT, target);				// バッファフル解除チェック
			break;

		case	LOG_ABNORMAL:		// 不正・強制出庫情報
			Log_CheckBufferFull(FALSE, eLOG_ABNORMAL, target);				// バッファフル解除チェック
			break;

		case	LOG_MONITOR:		// モニタ情報
			Log_CheckBufferFull(FALSE, eLOG_MONITOR, target);				// バッファフル解除チェック
			break;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100の処理に戻す）
//// MH321801(S) 未了取引記録件数を増やす
////		case	LOG_CREUSE:			// クレジット利用明細ログ
////			Log_CheckBufferFull(FALSE, eLOG_CREUSE, target);				// バッファフル解除チェック
////			break;
//// MH321801(E) 未了取引記録件数を増やす
		case	LOG_CREUSE:			// クレジット利用明細ログ
			Log_CheckBufferFull(FALSE, eLOG_CREUSE, eLOG_TARGET_REMOTE);	// バッファフル解除チェック
			break;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100の処理に戻す）

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case	LOG_EDYARM:			// Edyアラーム取引ログ
//			Log_CheckBufferFull(FALSE, eLOG_EDYARM, target);				// バッファフル解除チェック
//			break;
//
//		case	LOG_EDYSHIME:			// Edy締め記録ログ
//			Log_CheckBufferFull(FALSE, eLOG_EDYSHIME, target);				// バッファフル解除チェック
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		case LOG_HOJIN_USE:
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, target);				// バッファフル解除チェック
			break;

		case	LOG_NGLOG:						// 不正ログ
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, target);				// バッファフル解除チェック
			break;

		case	LOG_REMOTE_SET:					// 遠隔料金設定ログ
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_SET, target);				// バッファフル解除チェック
			break;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
//		case	LOG_ENTER:						// 入庫ログ
//			Log_CheckBufferFull(FALSE, eLOG_ENTER, target);					// バッファフル解除チェック
//			break;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
		
		case	LOG_PARKING:					// 駐車台数データ
			Log_CheckBufferFull(FALSE, eLOG_PARKING, target);				// バッファフル解除チェック
			break;
		
		case	LOG_LONGPARK:					// 長期駐車データ
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//			Log_CheckBufferFull(FALSE, eLOG_LONGPARK, target);				// バッファフル解除チェック
			Log_CheckBufferFull(FALSE, eLOG_LONGPARK_PWEB, target);			// バッファフル解除チェック
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
			break;
		
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
//		case LOG_IOLOG:
//			Log_CheckBufferFull(FALSE, eLOG_IOLOG, target);					// バッファフル解除チェック
//			break;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）

		case	LOG_MNYMNG_SRAM:				// 釣銭管理情報(SRAM)
			Log_CheckBufferFull(FALSE, eLOG_MNYMNG_SRAM, target);					// バッファフル解除チェック
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, target);					// バッファフル解除チェック
			break;

		case	LOG_REMOTE_MONITOR:
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_MONITOR, target);	// バッファフル解除チェック
			break;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）
//// MH364300 GG119A34(S) 改善連絡表No.83対応
//		case	LOG_LEAVE:						// 出庫ログ
//			Log_CheckBufferFull(FALSE, eLOG_LEAVE, target);					// バッファフル解除チェック
//			break;
//// MH364300 GG119A34(E) 改善連絡表No.83対応
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100不要処理）

		default:								// その他（ログデータ種別エラー）
			break;
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
}
