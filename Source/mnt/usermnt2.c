/*[]----------------------------------------------------------------------[]*/
/*| ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽ操作                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
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
#include	"irq1.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"flp_def.h"
#include	"LKmain.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"rkn_fun.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"mdl_def.h"
#include	"LCM.h"
#include	"cre_ctrl.h"

#define	GET_YEAR(d)		((d)/10000)
#define	GET_MONTH(d)	(((d)%10000)/100)
#define	GET_DAY(d)		((d)%100)
#define	GET_HM_WKNO(n,d)	(n&1)?(d%10):((d/1000)%10)
#define	GET_SPY_WKNO(d)		((d%100)/10)
#define	GET_SPY_YOBI(d)		(d%10)

static	const	ushort	LOCKPRN_MENU[][2] = {
	/* x, len */
		4, 16,
		4, 16,
		4, 12
	};
#define	dsp_menu(y, m, s, n, r)	grachr((ushort)(n+y), m[n][0], m[n][1], (ushort)r, COLOR_BLACK, LCD_BLINK_OFF, &s[n][(m[n][0])])
#define	dsp_prnmenu(y, no, rv)		dsp_menu(y, LOCKPRN_MENU, UMSTR6, no, rv)

/*--------------------------------------------------------------------------*/
/*   static function prototype                                              */
/*--------------------------------------------------------------------------*/
/* 共通 */
static void	area_posi_dsp(short top, short no, char area, ulong posi, ushort rev);
/*  5.ロック装置開閉 */
static char	lockctrl_set(char area, short kind);
static void	lockctrl_set_sts_dsp(short top, short no);
static void	lockctrl_set_all_dsp(short top, short max, short rev_no, short *idx);
/*  6.車室情報 */
static char	locksts_set(char area, short kind);
static char	locksts_set_flpdata(short index);
static void	locksts_set_flpdata_dsp(char pos, short data, ushort rev);
static char locksts_set_flpdata_check(char pos, short data);
static void	locksts_set_all_dsp(short top, short max, short rev_no, short *idx);
/*  7.停留車情報 */
static char	staysts_search(short days);
short staysts_check(flp_com *p, short days);
static void	staysts_all_dsp(short top, short max, short rev_no, short *idx);
/*  8.駐車台数 */
/*  9.満車コントロール */
/* 10.領収証再発行 */
static void	receipt_all_dsp(ushort top, ushort max, ushort rev_no, ushort TotalCount);
/* 11.受付券再発行 */
static char	lockuke_set(char area, short kind);
static void	lockuke_set_sts_dsp(short top, short no, flp_com *pflp);
static void	lockuke_set_all_dsp(short top, short max, short rev_no, short *idx);
/* 13.営休業切替 */
/* 14.シャッター開放 */
/* 15.券データ確認 */
/* 16.定期有効／無効 */
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//static char	passinv_set(char block, char mode);
//static char	passinv_allset( char mode, char block );
//static char	passinv_print(char block, char mode);
//static void	passinv_print_dsp(char pos, ushort rev);
//static void	passinv_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind);
///* 17.定期入庫／出庫 */
//
//static char	passent_set(char block, char mode);
//
//ushort	UsMnt_PassEnter_BeforeValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID );
//ushort	UsMnt_PassEnter_AfterValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID );
//
//static char	passent_allset( char mode, char block );
//static char	passent_print(char block, char mode);
//static void	passent_print_dsp(char pos, ushort rev);
//static void	passent_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind);
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
/* 18.定期券チェック */
/* 19.サービスタイム */
/* 20.ロック装置開閉タイマー */
/* 21.特別日／特別期間 */
static void splday_check(void);
static char	splday_day(void);
static char	splday_period(void);
static char	splday_monday(void);
static void	splday_dsp(char pos, ushort rev);
static void	splday_day_dsp(char top, char no, char pos, long data, ushort rev);
static void	splday_period_dsp(char no, char pos, long data, ushort rev);
static void	splday_monday_dsp(char top, char no, char pos, long data, ushort rev);
static char	splday_week(void);
static void	splday_week_dsp(char top, char no, char pos, long data, ushort rev);
/* 22.係員パスワード */
/* 23.バックライト点灯方法 */
static void	backlgt_dsp(long data, ushort rev);
static void	backlgt_time_dsp(char chng, long time, ushort rev);
/* 25.キー音量調節 */
static void	keyvol_dsp(char pos);
/* 26.券期限 */
static void	ticval_kind_dsp(long kind, ushort rev);
static void	ticval_data_dsp(long kind, long data, ushort rev);
static void	ticval_date_dsp(long d1, long d2, long d3, long d4, char rev_pos);
static uchar ticval_date_chk(long date );
/* 27.営業開始時刻 */
static void	bsytime_dsp(char pos, long time, ushort rev);
/* 28.料金設定 */
unsigned short	UsMnt_mnylim(void);
static void	mnylim_dsp(char type, char no, char kind, char rev_pos, char top_pos);
unsigned short	UsMnt_mnytic(void);
static void	mnytic_data_dsp(char kind, long role, long data, ushort rev);
static void	mnytic_chng_dsp(char kind, long chng, ushort rev);
unsigned short	UsMnt_mnyshp(void);
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// static void	mnyshp_data_dsp(char top, char no, long role, long data, ushort rev);
// static void	mnyshp_chng_dsp(char top, char no, long chng, ushort rev);
static void	mnyshp_data_dsp(short top, short no, long role, long data, ushort rev);
static void	mnyshp_chng_dsp(short top, short no, long chng, ushort rev);
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
unsigned short	UsMnt_mnytax(void);
// MH810105(S) MH364301 インボイス対応
//インボイス用に作り直しているので旧プロトは削除
//static void	mnytax_dsp(long tax, ushort rev);
// MH810105(E) MH364301 インボイス対応
unsigned short	UsMnt_mnykid(void);
static void	mnykid_role_dsp(char top, char kind, long data, ushort rev);
static void	mnykid_time_dsp(char top, char kind, long data, ushort rev);
unsigned short	UsMnt_mnycha(void);
static void	mnycha_time_dsp(char mode, long time, ushort rev);
static void	mnycha_money_dsp(char mode, long money, ushort rev);
unsigned short	UsMnt_mnycha_dec(void);
static void	mnycha_time_dsp_dec(long time, ushort rev);
static void	mnycha_money_dsp_dec(long money, ushort rev);
static void	mnycha_coefficient_dsp_dec(long coefficient, ushort rev);
unsigned short	UsMnt_mnytweek(void);
static void	mnytweek_dsp(char no);
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//static ushort	UsMnt_CreConect( void );
//static ushort	UsMnt_CreUnSend( void );
//static ushort	UsMnt_CreSaleNG( void );
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
static void	ftotal_dsp(char pos, ushort rev);
static ushort	ftotal_print(char kind);
static ushort	UsMnt_VoiceGuideTimeAllDay(char *volume, ushort ptn);
static char 	Calc_tmset_sc(ushort tNum, struct GUIDE_TIME * guide);
static ushort 	UsMnt_VoiceGuideTimeExtra(char *volume);
static char		get_volume(char);

/* 定期券精算中止データ */
static t_TKI_CYUSI teiki_cyusi;	// 定期券中止データ編集用ワーク
static void pstop_num_dsp(short numerator, short denominator);
static void pstop_data_dsp(struct TKI_CYUSI *data, long type);
static short pstop_del_dsp(short index, short mode, struct TKI_CYUSI* work_data);
static void pstop_show_data(short index, t_TKI_CYUSI *cyusi, struct TKI_CYUSI *work_data, char fk_page);
static void	time_dsp(ushort low, ushort col, ushort hour, ushort min, ushort rev);
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//static void	time_dsp2(ushort low, ushort col, ushort hour, ushort min, ushort rev);
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
static void	date_dsp(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev);
static short pstop_calcel_dsp(void);

#if	UPDATE_A_PASS
static	unsigned short	UsMnt_LabelPaperSet(void);
static	unsigned short	UsMnt_LabelPaySet(void);
static	unsigned short	UsMnt_LabelTest(void);
#endif	// UPDATE_A_PASS

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//static void		pextim_show_data(short index, PASS_EXTBL *work_data, char fk_page);
//static void		pextim_num_dsp(ushort numerator, ushort denominator);
//static void		pextim_data_dsp(PASS_EXTBL *data);
//static short	pextim_del_dsp(short index, short mode, PASS_EXTBL *work_data);
//static short	pextim_calcel_dsp(void);
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

/* ロック装置情報インデックス格納用ワーク領域 */
static short	lock_idx[LOCK_MAX];

/* ロック装置状態(開・閉操作記憶用) */
static char		lock_sts[LOCK_MAX];

/* 定期券精算中止データ */
static t_TKI_CYUSI teiki_cyusi;	// 定期券中止データ編集用ワーク
static	PARK_NUM_CTRL	ParkWkDt;							// 駐車台数管理ﾃﾞｰﾀ01～03

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//static void cre_unsend_data_dsp(struct DATA_BK *data);	/* クレジット未送信売上依頼データ データ表示 */
//static void cre_saleng_show_data(short index);
//static void cre_saleng_num_dsp(unsigned char numerator, char denominator);
//static void cre_saleng_data_dsp(short index);
//static void	date_dsp3(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev);
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
extern	const	ushort	POS_DATA1_0[6][4];
extern	ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req );
extern	void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
extern	void	LogCntDsp2( ushort LogCnt1, ushort LogCnt2, ushort type);
extern	void	LogDateDsp( ushort *Date, uchar pos );
extern	uchar	LogDateChk( uchar pos, short data );
extern	uchar	LogDataClr( ushort LogSyu );
extern	uchar	LogDatePosUp1( uchar *pos, uchar req );
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//static	ushort	UsMnt_Edy_PayArmPrint( void );		// アラーム取引情報
//static	ushort	UsMnt_Edy_CentCon( void );			// センター通信開始
//static	ushort	UsMnt_Edy_ShimePrint( void );		// Ｅｄｙ締め記録情報
//#if (4 == AUTO_PAYMENT_PROGRAM)						// 試験用にEMへｺﾏﾝﾄﾞ送信（debug処理）
//static	ushort	UsMnt_Edy_Test( void );				// 
//#endif
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

#define USM_MAKE_MENU 10												/* 作成用メニューテーブル最大件数 */
static	unsigned char	UsMnt_DiditalCasheMENU_STR[USM_MAKE_MENU][31];	/* 作成用メニューテーブル(文字) */
static	unsigned short	UsMnt_DiditalCasheMENU_CTL[USM_MAKE_MENU][4];	/* 作成用メニューテーブル(制御) */
/* メニューテーブル作成関数 */
static	unsigned char	UsMnt_DiditalCasheMenuMake31(const CeMenuPara *prm_tbl, const unsigned char str_tbl[][31], const unsigned short ctr_tbl[][4], const unsigned char tbl_cnt);
unsigned short UsMnt_DiditalCasheSyuukei(ushort);
unsigned short UsMnt_DiditalCasheUseLog(ushort);

extern	uchar	SysMnt_Work[];	/* 32KB */
extern	Receipt_data	PayLog_Work;	// 個別精算ログ取出し用エリア

static ushort UsMnt_NgLog_GetLogCnt(ushort Sdate, ushort Edate);
ushort	UsMnt_IoLog(ushort no);
static char	lock_io_set(char area, short kind);
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//static void	lockio_set_all_dsp(short top, short max, short rev_no, short *idx);
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
extern	void	LogDateGet2( date_time_rec *Old, date_time_rec *New, ushort *Date );
extern	void	LogDateDsp3( ushort *Date, uchar pos );
extern	uchar	LogDateChk2( uchar pos, short data );
extern const	ushort	POS_DATA31[10][4];
extern	uchar	LogDatePosUp3( uchar *pos, uchar req );
static 	t_ValidWriteData at_invwork;
static	char	AteVaild_ConfirmDelete(ushort attendNo);
const	uchar*	car_title_table[3][4] = {
		{ UMSTR1[26], UMSTR1[28], UMSTR1[39],UMSTR1[41],},	// 駐車（内蔵）
		{ UMSTR1[26], UMSTR1[28], UMSTR1[39],UMSTR1[41],},	// 駐車（IF）
		{ UMSTR1[27], UMSTR1[29], UMSTR1[40],UMSTR1[42],},	// 駐輪
	};

static void		Carfail_all_dsp(short, short, short, short *);
static ushort	UsMnt_CarFailSet( ushort );
static void		LockInfoDataGet_for_Failer( void );
static char		LockInfoDataSet_for_Failer( char ret_value );
static ushort	LockInfoFailer_ResetMessage( void );
static uchar	LockInfoDataSet_sub( ushort );
#define MAX_VOICE_GUIDE	3
short	GET_TIME_IDX(char n, char b, char m);
short	GET_MONEY_IDX(char n, char k, char b, char m);
char 	mnycha_sub( char set, short seg, char no, char kind, char band, long data );
short 	GET_LIM_IDX(char n, char k);
char 	GET_LIM_TYP( char kind, char* max_set, ushort* param_pos);
void 	mnylim_type_disp( char type , char top_pos);
#if (5 == AUTO_PAYMENT_PROGRAM)
unsigned short UsMnt_mc10print(void);
#endif
ushort	UsColorLCDLumine(void);
static void	Lumine_dsp(long , ushort );
static void	Lumine_time_dsp(char , long , ushort );
static ushort	UsColorLCDLumine_change(char);
#define		STAYSTS_AL_DISP_NO_REVERSE		(short)-1

extern	uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam);

static unsigned short	UsMnt_ParkingCertificateSet(void);
void	UsMnt_datecheck( uchar );
static	void	tickvalid_check( void );

// MH321800(S) D.Inaba ICクレジット対応
extern	const	ushort	POS_DATA1[6][4];
// 拡張機能/決済リーダ処理
extern	const	unsigned short	USM_EXTEC_TBL[][4];
static unsigned short UsMnt_ECReaderMnu( void );
static unsigned short UsMnt_EcReaderAlarmLog( void );
// 音声案内時間/リーダへのコマンド送信関数
static char 	get_volume_EC(char num);
static char	 	VoiceGuide_VolumeChange( void );
static ushort 	ECVoiceGuide_VolumeTest( uchar vol );
static ushort 	ECVoiceGuide_SelectTarget( char *vol_actuary, char *vol_reader, ushort ptn, ushort swit );
static ushort	UsMnt_ECVoiceGuideTimeAllDay(char *volume, ushort ptn, uchar target);
static ushort	UsMnt_ECVoiceGuideTimeExtra(char *vol_actuary, char *vol_reader);
static ushort	ECVoiceGuideTimeExtra_Resultdisplay(uchar result);
#define ADJUS_MAX_VOLUME	15
#define ADJUS_MIN_VOLUME	0
// MH321800(E) D.Inaba ICクレジット対応
// MH810104 GG119201(S) 電子ジャーナル対応
extern	const	unsigned short	USM_EXTEJ_TBL[][4];
static unsigned short UsMnt_EJournalMnu( void );
static ushort	UsMnt_EJournalSDInfo( void );
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(S) MH364301 インボイス対応
static int	get_next_curpos(ushort msg, int pos);
static int	Date_Check( short data, int pos );
static void	mnytax_dsp(ushort val, int pos, ushort rev);
static void	mnytax_dspinit_dsp(ushort* val);
static void	mnytax_editinit_dsp(ushort* val);
static void	before_tax_dsp(ushort tax, ushort rev);
static void	after_tax_dsp(ushort tax, ushort rev);
static void	ChangeBaseDate_dsp(ushort y, ushort m, ushort d);
static void	ChangeBaseDate_edit(uchar kind, ushort val, ushort rev);
// MH810105(E) MH364301 インボイス対応

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾊﾟﾗﾒｰﾀ変換（汎用）																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_get(par1,par2,par3,par4,par5,par6)													   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀ種類 0=共通/1=固有															   |*/
/*|				: par2	: ﾊﾟﾗﾒｰﾀｾｯｼｮﾝNo.																	   |*/
/*| 			: par3	: ｱﾄﾞﾚｽ																				   |*/
/*| 			: par4	: 桁数(何桁取得するか)																   |*/
/*| 			: par5	: 位置(何桁目から取得するか 位置は654321とする)										   |*/
/*| 			: par6	: 適用する値(10進数6桁まで)															   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: Y.Ise(AMANO)																				   |*/
/*| Date		: 2009-09-10																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long prm_set(char kin, short ses, short adr, char len, char pos, long dat)		/*								*/
{																				/*								*/
	long	prm_wk1, prm_wk2;													/* ﾊﾟﾗﾒｰﾀ変換ﾜｰｸ				*/
	long	result	= -1;														/* 戻り値用変数					*/
	long*	WPrmSS;																/* 値を反映させるパラメータ		*/
	const	long arr[] = {0L, 1L, 10L, 100L, 1000L, 10000L, 100000L, 1000000L};	/* 桁管理用テーブル				*/
																				/*								*/
	if((pos > 0) && (len > 0) && ((len + pos) <= 7))							/* 指定桁数が正常				*/
	{																			/*								*/
		if( kin == PEC_PRM )													/* 固有ﾊﾟﾗﾒｰﾀ					*/
		{																		/*								*/
			WPrmSS = &PPrmSS[ses][adr];											/* 固有ﾊﾟﾗﾒｰﾀ					*/
		}																		/*								*/
		else																	/* 上記以外(共通ﾊﾟﾗﾒｰﾀ)			*/
		{																		/*								*/
			WPrmSS = &CPrmSS[ses][adr];											/* 共通ﾊﾟﾗﾒｰﾀ					*/
		}																		/*								*/
		prm_wk2 = (*WPrmSS) % arr[pos];											/* 目標桁より下の桁を一時退避	*/
		prm_wk1 = (*WPrmSS) / arr[pos + len];									/* 目標の桁を最下位桁へ移動		*/
		prm_wk1 = (prm_wk1 * arr[len + 1]) + dat;								/* 目標桁の値を反映				*/
		prm_wk1 = (prm_wk1 * arr[pos]) + prm_wk2;								/* 目標桁以下の値を元に戻す		*/
		*WPrmSS = prm_wk1;														/* 適用した値をパラメータへ反映	*/
		result = *WPrmSS;														/* 反映後の値を戻り値用変数へ	*/
	}																			/*								*/
	return( result );															/* 設定反映後の値を返す			*/
}																				/*								*/

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：共通 区画選択画面                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_AreaSelect( char type )                           |*/
/*| PARAMETER    : char type : MNT_FLCTL:ロック装置開閉                    |*/
/*|                          : MNT_FLSTS:車室情報                          |*/
/*|                          : MNT_UKERP:受付券再発行                      |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_AreaSelect(short kind)
{
	ushort	msg;
	short	count;
	char	area[27];		/* A-Z(1-26) */
	char	max;
	char	top;
	int		no;
	short	i, j;
	uchar	ch;
	uchar	str[2];
	char	ret;
	char	dsp;
	char	(*func_set)(char area, short kind);		/* 設定画面関数ポインタ */
	const uchar	*title;

	/* 設定画面関数の決定 */
	switch (kind) {
	case MNT_FLCTL:
		/* ロック装置開閉 */
		func_set = lockctrl_set;

		switch( gCurSrtPara ){
			case	MNT_FLAPUD:		// フラップ上昇下降（個別）
				title = UMSTR1[24];
				break;
			case	MNT_FLAPUD_ALL:	// フラップ上昇下降（全て）
				title = UMSTR1[25];
				break;
			case	MNT_BIKLCK:		// 駐輪ロック開閉（個別）
				title = UMSTR1[30];
				break;
			case	MNT_BIKLCK_ALL:	// 駐輪ロック開閉（全て）
				title = UMSTR1[31];
				break;
			default:				// その他（通常ありえないが念の為）
				title = UMSTR1[24];
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}

		break;
	case MNT_FLSTS:
		/* 車室情報 */
		func_set = locksts_set;

		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][0];
									/* "＜車室情報（駐車）＞　　　　　" */
									/* "＜車室情報（駐輪）＞　　　　　" */
		break;
	case MNT_UKERP:
		/* 受付券再発行 */
		func_set = lockuke_set;
		title = UMSTR1[7];			/* "＜受付券再発行＞　　　　　　　" */
		break;
	case MNT_IOLOG:
		/* 入出庫 */
		func_set = lock_io_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][3];
									/* "＜入出庫情報プリント(駐車)＞　" */
									/* "＜入出庫情報プリント(駐輪)＞　" */
		break;
	case MNT_CARFAIL:
		/* 車室故障 */
		func_set = locksts_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][2];
		break;
	default:	/* case BKRS_FLSTS: */
		/* 車室情報調整 */
		func_set = locksts_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][1];
									/* "＜車室情報調整(駐車)＞　　　　" */
									/* "＜車室情報調整(駐輪)＞　　　　" */
		break;
	}

	/* ロック情報から区画を検索する */
	memset(area, 0, sizeof(area));
	count = 0;
	for (i = 0, j = 0; i < LOCK_MAX; i++) {
		WACDOG;										// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			area[LockInfo[i].area] = 1;
			count++;
		}
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);

	if( kind != MNT_CARFAIL ){
	/* ロック装置未接続時 */
		if (count == 0) {
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[21] );			/* "　接続中のﾛｯｸ装置はありません " */
			Fun_Dsp( FUNMSG[8] );						/* "　　　　　　　　　　　　 終了 " */
			for( ; ; ){
				msg = StoF( GetMessage(), 1 );
				switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case LCD_DISCONNECT:
						return( MOD_CUT );
						break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case KEY_MODECHG:
						BUZPI();
						return( MOD_CHG );
						break;
					case KEY_TEN_F5:
						BUZPI();
						return( MOD_EXT );
						break;
					default:
						break;
				}
			}
		}
	}

	/* ロック装置接続時ー区画使用なし、又はＡのみ */
	if(CPrmSS[S_TYP][81] < 2){
		ret = func_set(CPrmSS[S_TYP][81]? 1 : 0, kind);
		if( kind == MNT_CARFAIL ){
			ret = LockInfoDataSet_for_Failer( ret );
		}
		if (ret == 1) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		if (ret == 2) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		return MOD_EXT;
	}

	/* ロック装置接続時ー区画使用あり */
	for (i = 1, j = 0; i < 27; i++) {
		if (area[i] == 1) {
			area[j++] = (char)i;
		}
	}
	max = (char)(j-1);

	top = no = 0;
	dsp = 1;
	for( ; ; ){
		if (dsp) {
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );
			for( i = 0; i < 5; i++ ){
				if (top+i > max) {
					break;
				}
				ch = (uchar)( 'A' + area[top+i] - 1);
				as1chg(&ch, str, 1);
				grachr ((ushort)(i+2), 4, 2, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, str);			//alphabet
				grachr ((ushort)(i+2), 6, 4, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[0]);	//区画
			}
			Fun_Dsp( FUNMSG[25] );							/* // [25]	"　▲　　▼　　　　 読出  終了 " */
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:
			BUZPI();
			return( MOD_EXT );
		case KEY_TEN_F1:				/* F1:▲ */
		case KEY_TEN_F2:				/* F2:▼ */
			BUZPI();
			if (msg == KEY_TEN_F1) {
				no--;
				if (no < 0) {
					no = max;
					top = (char)(max/5*5);
				}
				else if (no < top) {
					top -= 5;
				}
			}
			else {
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			/* データ表示 */
			for( i = 0; i < 5; i++ ){
				if (top+i > max) {
					for ( ; i < 5; i++)
						grachr((ushort)(i+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
					break;
				}
				ch = (uchar)( 'A' + area[top+i] - 1);
				as1chg(&ch, str, 1);
				grachr ((ushort)(i+2), 4, 2, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, str);			//alphabet
				grachr ((ushort)(i+2), 6, 4, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[0]);	//区画
			}
			break;
		case KEY_TEN_F4:
			BUZPI();
			ret = func_set(area[no], kind);
			if( kind == MNT_CARFAIL ){
				ret = LockInfoDataSet_for_Failer( ret );
			}
			if (ret == 1) {
				return MOD_CHG;
			}
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  共通－車室番号表示                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : area_posi_dsp( top, no, posi, index, rev )              |*/
/*| PARAMETER    : short  top   : 先頭番号                                 |*/
/*|              : short  no    : 表示番号                                 |*/
/*|              : char   area  : 区画                                     |*/
/*|              : ulong  posi  : 車室番号                                 |*/
/*|              : ushort rev   : 0:正転 1:反転                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	area_posi_dsp(short top, short no, char area, ulong posi, ushort rev)
{
	uchar		ch;
	uchar		str[2];

	/* 区画ありの場合 */
	if (CPrmSS[S_TYP][81] > 0 && area > 0) {
		ch = (uchar)('A' + area - 1);
		as1chg(&ch, str, 1);
		grachr((ushort)(2+no-top), 3, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, str);					/* 区画 */
		opedsp((ushort)(2+no-top), 5, (ushort)posi, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
	}
	/* 区画なし(又はAのみ)の場合 */
	else {
		if( posi == 9999L ){										/* 定期券更新精算? */
			grachr((ushort)(2+no-top), 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[43]);		/* "更新精算"表示 */
		}else{
			opedsp((ushort)(2+no-top), 4, (ushort)posi, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ロック装置開閉－設定画面                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set( area )                                    |*/
/*| PARAMETER    : char area  : 区画                                       |*/
/*|              : short kind : 機能種別(未使用)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lockctrl_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	short	max;
	short	i, j;
	short	flp_no;
	ushort	cmd;
	ulong	ulwork;
	short   start, end;	
	uchar	allActionWatchFlag = 0;							// 全装置動作監視中フラグ
	uchar	actionFlag;									// 動作中ありフラグ
	uchar	updown;										// 上昇下降フラグ
	ushort	index;										// LockInfoインデックス
	ushort	indexCount;									// LockInfoインデックスカウント数
	ushort	allCmdTimeout;								// 全操作コマンドタイムアウト時間
	t_LockMaker flapMaker;								// 装置情報

	ushort	lno;
	uchar	m_mode;			// ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞ状態
	cmd = 0;

	/* ロック装置情報(LockInfo[])からarea区画であるデータを抽出する */
	
	switch( gCurSrtPara ){
		case	MNT_FLAPUD:		// フラップ上昇下降（個別）
		case	MNT_FLAPUD_ALL:	// フラップ上昇下降（全て）
			ulwork = GetCarInfoParam();
			switch( ulwork&0x06 ){
				case	0x04:
					start = INT_CAR_START_INDEX;
					end   = start + INT_CAR_LOCK_MAX;
					break;
				case	0x06:
					start = CAR_START_INDEX;
					end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
					break;
				case	0x02:
				default:
					start = CAR_START_INDEX;
					end   = start + CAR_LOCK_MAX;
					break;
			}
			break;

		case	MNT_BIKLCK:		// 駐輪ロック開閉（個別）
		case	MNT_BIKLCK_ALL:	// 駐輪ロック開閉（全て）
			start = BIKE_START_INDEX;
			end   = start + BIKE_LOCK_MAX;
			break;

		default:				// その他（通常ありえないが念の為）
			start = CAR_START_INDEX;
			end   = start + CAR_LOCK_MAX;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}

	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			if (area == 0) {
				lock_idx[j] = i;
				lock_sts[j] = FLAPDT.flp_data[i].nstat.bits.b01 ? 1 : 0;
				j++;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j] = i;
					lock_sts[j] = FLAPDT.flp_data[i].nstat.bits.b01 ? 1 : 0;
					j++;
				}
			}
		}
	}
	max = j-1;
	indexCount = (ushort)j;

	/* 該当データなし */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "　　　データがありません　　　" */
		Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)				
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:終了 */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	switch( gCurSrtPara ){
		case	MNT_FLAPUD:		// フラップ上昇下降（個別）
			top = no = 0;
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				// "　【車室番号】　　　【状態】　"
			lockctrl_set_all_dsp(top, max, no, lock_idx);									// データ一括表示
			Fun_Dsp( FUNMSG2[24] );															// "  ＋  －／読 下降  上昇  終了 "
			break;

		case	MNT_FLAPUD_ALL:	// フラップ上昇下降（全て）
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );				// "　　全装置の動作を行います　　"
			Fun_Dsp( FUNMSG2[28] );															// "             下降  上昇  終了 "
			break;

		case	MNT_BIKLCK:		// 駐輪ロック開閉（個別）
			top = no = 0;
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				// "　【車室番号】　　　【状態】　"
			lockctrl_set_all_dsp(top, max, no, lock_idx);									// データ一括表示
			Fun_Dsp( FUNMSG[74] );															// "　＋　－／読　開　　閉　 終了 "
			break;

		case	MNT_BIKLCK_ALL:	// 駐輪ロック開閉（全て）
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );				// "　　全装置の動作を行います　　"
			Fun_Dsp( FUNMSG2[29] );															// "              開    閉   終了 "
			break;
	}

	input = -1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
		case KEY_MODECHG:
			Lagcan(OPETCBNO, 14);								// 1sの監視タイマを止める
			Lagcan(OPETCBNO, 19);								// 全装置動作監視タイマを止める
			BUZPI();
			return 1;
		case KEY_TEN_F5:
			Lagcan(OPETCBNO, 14);								// 1sの監視タイマを止める
			Lagcan(OPETCBNO, 19);								// 全装置動作監視タイマを止める
			BUZPI();
			return 0;
		case KEY_TEN_F1:				/* F1:▲ */
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// フラップ上昇下降（全て）　又は　駐輪ロック開閉（全て）の場合”F1”ｷｰ無効
				break;
			}
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* データ一括表示 */
			lockctrl_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:				/* F2:▼／読 */
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// フラップ上昇下降（全て）　又は　駐輪ロック開閉（全て）の場合”F2”ｷｰ無効
				break;
			}
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
						break;
					}
				}
				if (i > max) {
					/* 入力した車室番号が見つからない場合 */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* データ一括表示 */
			lockctrl_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F3:
		case KEY_TEN_F4:
			m_mode = OFF;
			switch( gCurSrtPara ){
				case	MNT_FLAPUD:		// フラップ上昇下降（個別）
				case	MNT_BIKLCK:		// 駐輪ロック開閉（個別）
					m_mode = m_mode_chk_psl( (ushort)(lock_idx[no]+1) );	// 接続されているﾀｰﾐﾅﾙNo.の手動ﾓｰﾄﾞ状態ﾁｪｯｸ
					break;

				case	MNT_FLAPUD_ALL:	// フラップ上昇下降（全て）
					ulwork = GetCarInfoParam();
					switch( ulwork&0x06 ){
						case	0x04:
							m_mode = m_mode_chk_all( LK_KIND_INT_FLAP );	// 内蔵ﾌﾗｯﾌﾟ装置全ﾀｰﾐﾅﾙの手動ﾓｰﾄﾞ状態ﾁｪｯｸ
							break;
						case	0x02:
						default:
							m_mode = m_mode_chk_all( LK_KIND_FLAP );		// ﾌﾗｯﾌﾟ装置全ﾀｰﾐﾅﾙの手動ﾓｰﾄﾞ状態ﾁｪｯｸ
							break;
					}
					break;

				case	MNT_BIKLCK_ALL:	// 駐輪ロック開閉（全て）
					m_mode = m_mode_chk_all( LK_KIND_LOCK );				// ﾛｯｸ装置全ﾀｰﾐﾅﾙの手動ﾓｰﾄﾞ状態ﾁｪｯｸ
					break;
			}
			if( m_mode == ON ){
				// 手動ﾓｰﾄﾞ中の場合、操作不可とする
				BUZPIPI();
				break;
			}
			if(allActionWatchFlag) {
				// 全装置動作監視中は操作不可とする
				BUZPIPI();
				break;
			}

			BUZPI();

			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// フラップ上昇下降（全て）　又は　駐輪ロック開閉（全て）の場合
				if( gCurSrtPara == MNT_FLAPUD_ALL ){
					// フラップ上昇下降（全て）
					ulwork = GetCarInfoParam();
					if (msg == KEY_TEN_F3) {
						// フラップ下降（全て）
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[33] );			// 
						lno = OPLOG_A_FLAP_DOWN;					// 操作履歴：全ﾌﾗｯﾌﾟ下降（ﾒﾝﾃﾅﾝｽ）
						switch( ulwork&0x06 ){
							case	0x04:
								cmd = INT_FLAP_A_DOWN_SND_MNT;		// 全内蔵ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)
								break;
							case	0x02:
							default:
								cmd = FLAP_A_DOWN_SND_MNT;			// 全ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)
								break;
						}
					}
					else {
						// フラップ上昇（全て）
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[34] );			// 
						lno = OPLOG_A_FLAP_UP;						// 操作履歴：全ﾌﾗｯﾌﾟ上昇（ﾒﾝﾃﾅﾝｽ）
						switch( ulwork&0x06 ){
							case	0x04:
								cmd = INT_FLAP_A_UP_SND_MNT;		// 全内蔵ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)
								break;
							
							case	0x02:
							default:
								cmd = FLAP_A_UP_SND_MNT;			// 全ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)
								break;
						}
					}
				}
				else{
					// 駐輪ロック開閉（全て）
					if (msg == KEY_TEN_F3) {
						// 駐輪ロック開（全て）
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[35] );			// 
						cmd = LOCK_A_OPN_SND_MNT;					// 全ﾛｯｸ装置開要求(ﾒﾝﾃﾅﾝｽ)
						lno = OPLOG_A_LOCK_OPEN;					// 操作履歴：全駐輪ﾛｯｸ開（ﾒﾝﾃﾅﾝｽ）
					}
					else {
						// 駐輪ロック閉（全て）
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[36] );			// 
						cmd = LOCK_A_CLS_SND_MNT;					// 全ﾛｯｸ装置閉要求(ﾒﾝﾃﾅﾝｽ)
						lno = OPLOG_A_LOCK_CLOSE;					// 操作履歴：全駐輪ﾛｯｸ閉（ﾒﾝﾃﾅﾝｽ）
					}
				}

				// フラップタスクへコマンド送信前にタイムアウト監視タイマを開始する
				allCmdTimeout = 0;
				if( gCurSrtPara == MNT_FLAPUD_ALL ){								// フラップ
					for(index = 0; index < indexCount; ++index) {
						if((FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0){	// 故障していない
							LCM_GetFlapMakerParam(LockInfo[lock_idx[index]].lok_syu, &flapMaker);	// フラップ装置情報取得
							if(cmd == INT_FLAP_A_DOWN_SND_MNT || cmd == FLAP_A_DOWN_SND_MNT) {	// 全下降
								allCmdTimeout += flapMaker.open_tm;					// 下降時間
							}
							else {													// 全上昇
								allCmdTimeout += flapMaker.clse_tm;					// 上昇時間
							}
						}
					}
				}
				else {																// ロック装置
					for(index = 0; index < indexCount; ++index) {
						if((FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0){			// 故障していない
							if(cmd == LOCK_A_OPN_SND_MNT) {							// 全開
								allCmdTimeout += LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].open_tm / 10;	// 下降時間(100ms単位)
								if(LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].open_tm % 10) {
									++allCmdTimeout;
								}
							}
							else {													// 全閉
								allCmdTimeout += LockMaker[LockInfo[index].lok_syu - 1].clse_tm / 10;	// 上昇時間(100ms単位)
								if(LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].clse_tm % 10) {
									++allCmdTimeout;
								}
							}
						}
					}
				}
				Lagtim(OPETCBNO, 19, allCmdTimeout * 50);							// コマンドタイムアウト監視タイマ
				allActionWatchFlag = 1;
				queset( FLPTCBNO, cmd, 0, NULL );					// ﾛｯｸ装置管理処理（fcmain）へﾒｯｾｰｼﾞ送信
				Ope_DisableDoorKnobChime();
				wopelg( lno, 0, 0 );								// 操作履歴登録
				for( i=0; i<LOCK_MAX; i++ )
				{
					WACDOG;											// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
					Kakari_Numu[i] = OPECTL.Kakari_Num;
				}

				Lagtim(OPETCBNO, 6, 100);							// 操作ｶﾞｰﾄﾞﾀｲﾏｰ起動（２秒）
				/* ２秒待機する */
				for ( ; ; ) {
					msg = StoF(GetMessage(), 1 );
					if (msg == TIMEOUT6) {
						// 操作ｶﾞｰﾄﾞﾀｲﾏｰﾀｲﾑｱｳﾄ
						break;
					}
					if (msg == KEY_MODECHG) {
						// モードチェンジ
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return 1;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					if (msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return 2;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				}
				Lagtim(OPETCBNO, 14, 50);											// 状態監視タイマ(1s)
				break;
			}
			if (msg == KEY_TEN_F3) {
				cmd = FLAP_DOWN_SND;
				lock_sts[no] = 0;
				ulwork = (ulong)(( LockInfo[lock_idx[no]].area * 10000L ) + LockInfo[lock_idx[no]].posi );	// 区画情報Get
				if(FALSE == LCM_CanFlapCommand(lock_idx[no], 0)) {			// 下降コマンド実行不可?
					// 現在の状態が下降済みの場合は、下降コマンドを受け付けない
					BUZPIPI();
					input = -1;
					break;
				}
				
				if( lock_idx[no] < INT_FLAP_END_NO ){
					// 操作対象＝ﾌﾗｯﾌﾟ
					wopelg( OPLOG_FLAP_DOWN, 0, ulwork );	// 操作履歴登録（ﾌﾗｯﾌﾟ下降）
					Kakari_Numu[lock_idx[no]] = OPECTL.Kakari_Num;		//係員No保存
				}
				else{
					// 操作対象＝ﾛｯｸ装置
					wopelg( OPLOG_LOCK_OPEN, 0, ulwork );	// 操作履歴登録（ﾛｯｸ装置開）
					Kakari_Numu[lock_idx[no]] = OPECTL.Kakari_Num;		//係員No保存
				}
			}
			else {
				cmd = FLAP_UP_SND;
				lock_sts[no] = 1;
				ulwork = (ulong)(( LockInfo[lock_idx[no]].area * 10000L ) + LockInfo[lock_idx[no]].posi );	// 区画情報Get
				if(FALSE == LCM_CanFlapCommand(lock_idx[no], 1)) {			// 上昇コマンド実行不可?
					// 現在の状態が上昇済みの場合は、上昇コマンドを受け付けない
					// 操作ログ作成、画面表示変更前に処理を中止する
					BUZPIPI();
					input = -1;
					break;
				}
				if( lock_idx[no] < INT_FLAP_END_NO ){
					// 操作対象＝ﾌﾗｯﾌﾟ
					wopelg( OPLOG_FLAP_UP, 0, ulwork );		// 操作履歴登録（ﾌﾗｯﾌﾟ上昇）
				}
				else{
					// 操作対象＝ﾛｯｸ装置
					wopelg( OPLOG_LOCK_CLOSE, 0, ulwork );	// 操作履歴登録（ﾛｯｸ装置閉）
				}
			}
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[14]);		/* "動作中" */
			/* ロック装置動作要求 */
			flp_no = lock_idx[no] + 1;
			queset(FLPTCBNO, cmd, sizeof(flp_no), &flp_no);
			Lagtim(OPETCBNO, 6, 100);		/* 2sec timer start */
			/* ２秒待機する */
			for ( ; ; ) {
				msg = StoF(GetMessage(), 1 );
				/* タイムアウト(2秒)検出 */
				if (msg == TIMEOUT6) {
					break;
				}
				/* モードチェンジ */
				if (msg == KEY_MODECHG) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if ( msg == LCD_DISCONNECT ) {	
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			}
			Flapdt_sub_clear((ushort)(flp_no - 1));			// 中止詳細ｴﾘｱｸﾘｱ
			lockctrl_set_sts_dsp(top, no);
			input = -1;
			break;
		case KEY_TEN:
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// フラップ上昇下降（全て）　又は　駐輪ロック開閉（全て）の場合”TEN”ｷｰ無効
				break;
			}
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// フラップ上昇下降（全て）　又は　駐輪ロック開閉（全て）の場合”取消”ｷｰ無効
				break;
			}
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		case TIMEOUT14:											// 1s毎の状態監視
			Lagcan(OPETCBNO, 14);
			actionFlag = 0;
			if(gCurSrtPara == MNT_FLAPUD_ALL) {					// フラップ
				if(cmd == INT_FLAP_A_DOWN_SND_MNT || cmd == FLAP_A_DOWN_SND_MNT){
					updown = 0;									// 下降
				}
				else {
					updown = 1;									// 上昇
				}
				for (index = 0; index < indexCount; ++index) {
					if (LockInfo[lock_idx[index]].lok_syu != 0 && LockInfo[lock_idx[index]].ryo_syu != 0 &&	// 装置あり
						(FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0) {				// 故障していない
						if(updown != FLAPDT.flp_data[lock_idx[index]].nstat.bits.b01) {
							actionFlag = 1;
							break;
						}
					}
				}
			}
			else {												// ロック装置
				if(cmd == OPLOG_A_LOCK_OPEN){
					updown = 0;									// 下降
				}
				else {
					updown = 1;									// 上昇
				}
				for (index = 0; index < indexCount; ++index) {
					if (LockInfo[lock_idx[index]].lok_syu != 0 && LockInfo[lock_idx[index]].ryo_syu != 0 &&	// 装置あり
						(FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0) {				// 故障していない
						if(updown != FLAPDT.flp_data[lock_idx[index]].nstat.bits.b01) {
							actionFlag = 1;
							break;
						}
					}
				}
			}
			
			if(actionFlag == 0) {								// 全装置動作完了
				Lagcan(OPETCBNO, 19);
				allActionWatchFlag = 0;
				BUZPI();
				grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );		// "　　全装置の動作を行います　　"
			}
			else {												// 動作中
				Lagtim(OPETCBNO, 14, 50);
			}
			break;
		case TIMEOUT19:											// 全装置動作監視タイマ
			Lagcan(OPETCBNO, 14);								// 1sの監視タイマを止める
			allActionWatchFlag = 0;
			BUZPI();
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );		// "　　全装置の動作を行います　　"
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ロック装置開閉－状態表示                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set_sts_dsp( top, no, *pflp )                  |*/
/*| PARAMETER    : short   top   : 先頭番号                                |*/
/*|              : short   no    : 表示番号                                |*/
/*|              : flp_com *pflp : ロック情報                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockctrl_set_sts_dsp(short top, short no)
{
	grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */

	if (gCurSrtPara == MNT_FLAPUD) {
		// フラップ
		if (lock_sts[no]) {
			grachr((ushort)(2+no-top), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[34]);		/* "上昇" */
			grachr((ushort)(2+no-top), 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);		/* "　" */
		} else {
			grachr((ushort)(2+no-top), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[35]);		/* "下降" */
			grachr((ushort)(2+no-top), 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);		/* "　" */
		}
	}
	else {
		// 駐輪ロック
		if (lock_sts[no]) {
			grachr((ushort)(2+no-top), 23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[15]);		/* "閉" */
		} else {
			grachr((ushort)(2+no-top), 23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[16]);		/* "開" */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ロック装置開閉－全データ表示(5件まで)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set_all_dsp( top, max, rev_no, *idx )          |*/
/*| PARAMETER    : short top    : 先頭番号                                 |*/
/*|              : short max    : 最大番号                                 |*/
/*|              : short rev_no : 反転番号                                 |*/
/*|              : short *idx   : ロック装置情報の配列番号テーブル         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockctrl_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		lockctrl_set_sts_dsp(top, (short)(top+i));
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報－設定画面                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set( area )                                     |*/
/*| PARAMETER    : char area  : 区画                                       |*/
/*|              : short kind : 機能種別                                   |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	locksts_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	char	dsp;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
	short	max;
	short	i, j;
	T_FrmPkjouNow	pri_data;
	const uchar	*title;
	const uchar	*fun;
	short   start, end;	
	short	no2;
	short	max2;
	char	dsp2;
	char	loop;
	void	(*disp_set)(short, short, short, short *);		/* 設定画面関数ポインタ */
	const uchar	*disp;	

	if( kind == MNT_CARFAIL ){
		LockInfoDataGet_for_Failer();
	}

	/* ロック装置情報(LockInfo[])からarea区画であるデータを抽出する */

	if (gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ) {
		i = GetCarInfoParam();
		switch( i&0x06 ){
			case	0x04:
				start = INT_CAR_START_INDEX;
				end   = start + INT_CAR_LOCK_MAX;
				break;
			case	0x06:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
				break;
			case	0x02:
			default:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX;
				break;
		}
	}
	else {
		start = BIKE_START_INDEX;
		end   = start + BIKE_LOCK_MAX;
	}
	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			if (area == 0) {
				lock_idx[j++] = i;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j++] = i;
				}
			}
		}
	}
	max = j-1;

	/* 該当データなし */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "　　　データがありません　　　" */
		Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:終了 */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	switch( kind ){
		case	MNT_FLSTS:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][0];
									/* "＜車室情報（駐車ＩＦ）＞　　　" */
									/* "＜車室情報（駐車）＞　　　　　" */
									/* "＜車室情報（駐輪）＞　　　　　" */
			fun = FUNMSG[73];		/* "  ＋  －／読 　　 ﾌﾟﾘﾝﾄ  終了 " */
			disp_set = locksts_set_all_dsp;
			disp = UMSTR3[46];
			break;
		case	BKRS_FLSTS:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][1];
									/* "＜車室情報調整（駐車ＩＦ）＞　" */
									/* "＜車室情報調整(駐車)＞　　　　" */
									/* "＜車室情報調整(駐輪)＞　　　　" */
			fun = FUNMSG2[19];		/* "  ＋  －／読 変更 ﾌﾟﾘﾝﾄ  終了 " */
			disp_set = locksts_set_all_dsp;
			disp = UMSTR3[46];
			break;
		case	MNT_CARFAIL:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][2];
									/* "＜車室故障（駐車ＩＦ）＞　　　" */
									/* "＜車室故障(駐車)＞　　　　　　" */
									/* "＜車室故障(駐輪)＞　　　　　　" */
			fun = FUNMSG[97];		/* "  ＋  －／読 　　 読出  終了 " */		
			disp_set = Carfail_all_dsp;
			disp = CARFAIL[0];
			break;
	}
	dsp = 1;
	top = no = 0;
	input = -1;
	for ( ; ; ) {
		if (dsp) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);						/* "＜ＸＸＸＸ＞　　　　　　　　　" */
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, disp );					/* "　【車室番号】　　　【XXXX】　" */
			disp_set(top, max, no, lock_idx);				/* データ一括表示 */
			Fun_Dsp(fun);									/* "  ＋  －／読 ＸＸ ﾌﾟﾘﾝﾄ  終了 " */
			dsp = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			if( (kind == BKRS_FLSTS)                &&
				(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//			if(msg == KEY_MODECHG){
//				return 1;
//			}
			if(msg == KEY_MODECHG){
				return 1;			// MOD_CHG
			}else{
				return 2;			// MOD_CUT
			}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		case KEY_TEN_F5:
			BUZPI();
			if( (kind == BKRS_FLSTS)                &&
				(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return 0;
		case KEY_TEN_F1:			/* F1:▲ */
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* データ一括表示 */
			disp_set(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:			/* F2:▼／読 */
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input){
						break;
					}
				}
				if (i > max) {
					/* 入力した車室番号が見つからない場合 */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* データ一括表示 */
			disp_set(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F3:		/* F3:変更(車室情報調整画面のみ) */
			if (kind == BKRS_FLSTS) {
				/* 駐車中 */
				BUZPI();
				/* 入庫時刻変更画面表示 */
				if (locksts_set_flpdata(lock_idx[no]) == 1) {	/* ﾓｰﾄﾞﾁｪﾝｼﾞ? */
					if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
						(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
						NT_pcars_timer = 0;
					}
					return 1;
				}
// MH810100(S)
				if (locksts_set_flpdata(lock_idx[no]) == 2) {	/* LCD切断通知 */
					if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
						(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
						NT_pcars_timer = 0;
					}
					return 2;
				}
// MH810100(E)
				
				dsp = 1;	/* 表示を元に戻す */
			}
			break;
		case KEY_TEN_F4:		/* F4:ﾌﾟﾘﾝﾄ */
			BUZPI();
			if( kind == MNT_CARFAIL ){
				/* 故障状態変更画面表示 */
				if (UsMnt_CarFailSet((ushort)lock_idx[no]) == MOD_CHG) {	/* ﾓｰﾄﾞﾁｪﾝｼﾞ? */
					return 1;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				if (UsMnt_CarFailSet((ushort)lock_idx[no]) == MOD_CUT) {	/*切断通知*/
					return 2;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				dsp = 1;									/* 表示を元に戻す */
				break;
			}
			// プリント項目選択画面へ遷移
			dsp2 = 1;
			loop = 1;
			no2 = 0;
			max2 = 2;
			while (loop) {
				if (dsp2) {
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);
					for (i = 0; i <= max2; i++) {
						if (i == no2) {
							// 選択されている項目のみ反転表示
							j = 1;
						} else {
							j = 0;
						}
						dsp_prnmenu(2, i, j);
					}
					Fun_Dsp(FUNMSG[68]);		/* "　▲　　▼　　　　 実行  終了 " */
					dsp2 = 0;
				}
				msg = StoF( GetMessage(), 1 );
				switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					loop = 0;
					return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:
					BUZPI();
					loop = 0;
					return 1;
					break;
				case KEY_TEN_F5:				/* F5:終了 */
					BUZPI();
					loop = 0;	//現在の画面終了
					dsp = 1;	//前画面を表示
					break;
				case KEY_TEN_F1:				/* F1:▲ */
					BUZPI();
					dsp_prnmenu(2, no2, 0);
					no2--;
					if (no2 < 0) {
						no2 = max2;
					}
					dsp_prnmenu(2, no2, 1);
					break;
				case KEY_TEN_F2:				/* F2:▼ */
					BUZPI();
					dsp_prnmenu(2, no2, 0);
					no2++;
					if (no2 > max2) {
						no2 = 0;
					}
					dsp_prnmenu(2, no2, 1);
					break;
				case KEY_TEN_F4:				/* F4:実行 */
					if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					RecvBackUp.ReceiveFlg = 0;	// 要求未受信(このﾌﾗｸﾞが0にならないとNTNETﾛｯｸ制御ﾃﾞｰﾀの受信が出来ないのでこのﾀｲﾐﾝｸﾞで0に出来るようにしておく)
					/* プリント実行 */
					pri_data.prn_menu = (uchar)no2;		// 印字ﾒﾆｭｰ(0:駐車のみ/1:空車のみ/2:全て)
					pri_data.prn_kind = R_PRI;
					pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
					queset(PRNTCBNO, PREQ_PKJOU_NOW, sizeof(T_FrmPkjouNow), &pri_data);
					Ope_DisableDoorKnobChime();

					/* プリント終了を待ち合わせる */
					Fun_Dsp(FUNMSG[82]);			// Fｷｰ表示　"　　　　　　 中止 　　　　　　"
					for ( priend = 0 ; priend == 0 ; ) {
						msg = StoF( GetMessage(), 1 );
						if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
							msg &= (~INNJI_ENDMASK);
						}
						switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
							BUZPI();
							if( (kind == BKRS_FLSTS)                &&
								(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
								(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
								NT_pcars_timer = 0;
							}
// MH810100(S)
//							return 1;
							if( msg == KEY_MODECHG ){
								return 1;		// MOD_CHG
							}else{
								return 2;		// MOD_CUT
							}
// MH810100(E)
						case PREQ_PKJOU_NOW:	// 印字終了
							priend = 1;
							break;
						case KEY_TEN_F3:		// F3ｷｰ（中止）
							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
							priend = 1;
							break;
						default:
							break;
						}
					}
					loop = 0;	// プリント項目選択画面のループを抜ける
					input = -1;
					dsp = 1;
					break;
				default:
					break;
				}
			}
			break;
		case KEY_TEN:
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報－全データ表示(5件まで)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : 先頭番号                                 |*/
/*|              : short max    : 最大番号                                 |*/
/*|              : short rev_no : 反転番号                                 |*/
/*|              : short *idx   : ロック装置情報の配列番号テーブル         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	locksts_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);

		switch( FLAPDT.flp_data[idx[top+i]].mode ){					// 装置状態(ﾓｰﾄﾞ)？

			case FLAP_CTRL_MODE1:		// 下降済み(車両なし)
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[0]);			// "  空車  " 
				break;

			case FLAP_CTRL_MODE2:		// 下降済み(車両あり､上昇ﾀｲﾏｰｶｳﾝﾄ中)
			case FLAP_CTRL_MODE3:		// 上昇動作中
			case FLAP_CTRL_MODE4:		// 上昇済み(駐車中)
				if( SvsTimChk( (ushort)(idx[top+i]+1) ) == OK ){	// ｻｰﾋﾞｽﾀｲﾑ中ﾁｪｯｸ
					grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[1]);		// "ｻｰﾋﾞｽﾀｲﾑ" 
				}
				else{
					grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[2]);		// "  駐車  " 
				}
				break;

			case FLAP_CTRL_MODE5:		// 下降動作中
			case FLAP_CTRL_MODE6:		// 下降済み(車両あり､ﾗｸﾞﾀｲﾏｰｶｳﾝﾄ中)
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[3]);			// "出庫待ち" 
				break;

			default:	// その他
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[0]);			// "  空車  " 
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報－全データ表示(5件まで)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Carfail_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : 先頭番号                                 |*/
/*|              : short max    : 最大番号                                 |*/
/*|              : short rev_no : 反転番号                                 |*/
/*|              : short *idx   : ロック装置情報の配列番号テーブル         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	Carfail_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);

		switch( FLAPDT.flp_data[idx[top+i]].car_fail & 0x01 ){		// 装置状態(ﾓｰﾄﾞ)？

			case 0:		// 故障なし
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// "なし" 
				break;

			case 1:		// 故障中
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		// "あり" 
				break;

			default:	// その他
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// "なし" 
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報調整－入庫時刻設定画面                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata( index )                            |*/
/*| PARAMETER    : short index : ロック装置情報のインデックス              |*/
/*| RETURN VALUE : 0:MOD_EXT 1:MOD_CHG                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	locksts_set_flpdata(short index)
{
	ushort	msg;
	short	in_d[11];
	short	input;
	char	pos;
	int		next;
	short	y, m, d;
	flp_com	*pflp;

	/* 操作履歴登録(車室情報調整) */
	wopelg(OPLOG_SHASHITUDTSET, 0, 0);

	pflp = &FLAPDT.flp_data[index];

	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[65]);			/* "入庫時刻：　　：　　　　　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[66]);			/* "　　　　　　　年　　月　　日　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[67]);			/* "状態　：　　　　　　　　　　　" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[68]);			/* "駐車　：　　　　ロック装置：　" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[69]);			/* "受付券：　　　　中止情報：　　" */
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[70]);			/* "暗証番号：　　　　　　　　　　" */
	Fun_Dsp(FUNMSG2[12]);						/* "　▲　　▼　 変更　書込　終了 " */

	// 区画は画面の表示上無理なので表示しない
	if( index < INT_CAR_START_INDEX ){
		grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"IF");			/* "状態　：　　　　　　　　　　IF" */	
	}

	opedsp(0, 22, (ushort)LockInfo[index].posi, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	/* 番号 */

	in_d[0] = (short)pflp->hour;
	in_d[1] = (short)pflp->minu;
	in_d[2] = (short)pflp->year;
	in_d[3] = (short)pflp->mont;
	in_d[4] = (short)pflp->date;
	in_d[5] = (short)pflp->mode;
	in_d[6] = (short)pflp->nstat.bits.b00;
	in_d[7] = (short)pflp->nstat.bits.b01;
	in_d[8] = (short)pflp->uketuke;
	in_d[9] = (short)pflp->bk_syu;
	in_d[10] = (short)pflp->passwd;
	locksts_set_flpdata_dsp(0, in_d[0], 1);
	locksts_set_flpdata_dsp(1, in_d[1], 0);
	locksts_set_flpdata_dsp(2, in_d[2], 0);
	locksts_set_flpdata_dsp(3, in_d[3], 0);
	locksts_set_flpdata_dsp(4, in_d[4], 0);
	locksts_set_flpdata_dsp(5, in_d[5], 0);
	locksts_set_flpdata_dsp(6, in_d[6], 0);
	locksts_set_flpdata_dsp(7, in_d[7], 0);
	locksts_set_flpdata_dsp(8, in_d[8], 0);
	locksts_set_flpdata_dsp(9, in_d[9], 0);
	locksts_set_flpdata_dsp(10, in_d[10], 0);

	pos = 0;
	input = -1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {	/* ﾓｰﾄﾞﾁｪﾝｼﾞ */
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {	/* ﾓｰﾄﾞﾁｪﾝｼﾞ */	
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
		if (msg == KEY_TEN_F5) {	/* 終了 */
			BUZPI();
			return 0;
		}
		/* 入庫時刻設定 */
		if (pos <= 4) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	/* ▲ */
			case KEY_TEN_F2:	/* ▼ */
				next = pos;
				/* ｶｰｿﾙ位置を次の位置にｾｯﾄ */
				if (msg == KEY_TEN_F1) {
					if (--next < 0) {
						for (next = 10; next > 7; next--) {
							if (in_d[next]) {	/* "なし"はスキップ */
								break;
							}
						}
					}
				}
				else {
					if (++next == 5) {
						if (in_d[5] == 1) {	/* "サービスタイム中"はスキップ */
							next = 6;
						}
					}
				}
				if (next > 4) {
					/* "▲""▼"により入庫時刻を抜けた場合 */
					input = in_d[next];
					in_d[0] = (short)pflp->hour;
					in_d[1] = (short)pflp->minu;
					in_d[2] = (short)pflp->year;
					in_d[3] = (short)pflp->mont;
					in_d[4] = (short)pflp->date;
					locksts_set_flpdata_dsp(0, in_d[0], 0);
					locksts_set_flpdata_dsp(1, in_d[1], 0);
					locksts_set_flpdata_dsp(2, in_d[2], 0);
					locksts_set_flpdata_dsp(3, in_d[3], 0);
					locksts_set_flpdata_dsp(4, in_d[4], 0);
				}
				else {
					if (input != -1) {
						/* 入力値範囲ﾁｪｯｸ */
						if (locksts_set_flpdata_check(pos, input) != 1) {
							BUZPIPI();
							locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 再表示 */
							input = -1;
							break;
						}
						in_d[pos] = input;
						input = -1;
					}
					
				}
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);		/* 正転表示に戻す */
				locksts_set_flpdata_dsp(next, in_d[next], 1);	/* 反転表示 */
				pos = next;
				break;
			case KEY_TEN_F4:	/* 書込 */
				if(input != -1) {
					/* 入力値範囲ﾁｪｯｸ */
					if (locksts_set_flpdata_check(pos, input) != 1) {
						BUZPIPI();
						locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 再表示 */
						input = -1;
						break;
					}
				}
				else {
					input = in_d[pos];
				}
				y = (pos==2) ? input : in_d[2];
				m = (pos==3) ? input : in_d[3];
				d = (pos==4) ? input : in_d[4];
				if( !y || !m || !d || chkdate( y, m, d ) != 0 ){ // 存在する日付かﾁｪｯｸ
					BUZPIPI();
					locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 再表示 */
					input = -1;
					break;
				}
				BUZPI();
				in_d[pos] = input;
				/* データ更新 */
				pflp->hour = (uchar)in_d[0];
				pflp->minu = (uchar)in_d[1];
				pflp->year = (ushort)in_d[2];
				pflp->mont = (uchar)in_d[3];
				pflp->date = (uchar)in_d[4];
				/* 表示 */
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);	/* 正転表示 */
				if (in_d[5] == 1) {	/* "サービスタイム中"はスキップ */
					pos = 6;
				} else {
					pos = 5;
				}
				input = (short)pflp->mode;
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 反転表示 */
				break;
			case KEY_TEN:		/* テンキー */
				BUZPI();
				if(input == -1) {
					input = (short)(msg - KEY_TEN0);
				}
				else {
					if( pos == 2 ) {
						/* 年入力であれば4桁の数値にする */
						input = (short)(((input * 10) + (msg - KEY_TEN0)) % 10000);
					}
					else{
						/* 年以外であれば2桁の数値にする */
						input = (short)(((input * 10) + (msg - KEY_TEN0)) % 100);
					}
				}
				locksts_set_flpdata_dsp(pos, input, 1);		/* 入力値を表示する（反転表示）*/
				break;
			case KEY_TEN_CL:	/* 取消 */
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 再表示 */
				input = -1;
				break;
			default:
				break;
			}
		}
		/* その他 */
		else {
			switch (msg) {
			case KEY_TEN_F4:	/* 書込 */
				in_d[pos] = input;
				if (pos == 5) {
					pflp->mode = (ushort)in_d[pos];
				}
				else if (pos == 6) {
					pflp->nstat.bits.b00 = (ushort)in_d[pos];
				}
				else if (pos == 7) {
					pflp->nstat.bits.b01 = (ushort)in_d[pos];
					pflp->nstat.bits.b02 = pflp->nstat.bits.b01;
					pflp->nstat.bits.b03 = 0;
					pflp->nstat.bits.b04 = 0;
					pflp->nstat.bits.b05 = 0;
					pflp->nstat.bits.b06 = 0;
					pflp->nstat.bits.b07 = 0;
					pflp->nstat.bits.b08 = 0;
					pflp->nstat.bits.b09 = 0;
				}
				else if (pos == 8) {
					pflp->uketuke = (ushort)in_d[pos];
					if (pflp->uketuke == 0) {
						pflp->u_year = 0;
						pflp->u_mont = 0;
						pflp->u_date = 0;
						pflp->u_hour = 0;
						pflp->u_minu = 0;
					}
				}
				else if (pos == 9) {
					pflp->bk_syu = (ushort)in_d[pos];
					Flapdt_sub_clear((ushort)index);			// 中止詳細ｴﾘｱｸﾘｱ
				}
				else {
					pflp->passwd = (ushort)in_d[pos];
				}
			case KEY_TEN_F1:	/* ▲ */
			case KEY_TEN_F2:	/* ▼ */
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);	/* 正転表示 */
				if (msg == KEY_TEN_F1) {
					pos--;
					if (pos > 7) {
						for ( ; pos > 7; pos--) {
							if (in_d[pos]) {
								break;
							}
						}
					}
					else if (pos == 5) {
						if (in_d[5] == 1) {	/* "サービスタイム中"はスキップ */
							pos = 4;
						}
					}
				}
				else {
					pos++;
					if (pos > 7) {
						for ( ; pos <= 10; pos++) {
							if (in_d[pos]) {
								break;
							}
						}
					}
					if (pos > 10) {
						pos = 0;
					}
				}
				if (pos <= 4) {			/* 入庫時刻設定 */
					input = -1;
				}
				else {
					input = in_d[pos];	/* その他 */
				}
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 反転表示 */
				break;
			case KEY_TEN_F3:	/* 変更 */
				BUZPI();
				if (pos == 5) {
					if (input < 3){
						input = 3;
					} else if (input < 5) {
						input = 5;
					} else {
						input = 0;
					}
				}
				else if (pos == 6 || pos == 7) {
					if (input) {
						input = 0;
					} else {
						input = 1;
					}
				}
				else {		/* if (pos == 8 || pos == 9 || pos == 10) */
					if (input) {
						input = 0;
					} else {
						input = in_d[pos];
					}
				}
				locksts_set_flpdata_dsp(pos, input, 1);	/* 反転表示 */
				break;
			case KEY_TEN_CL:	/* 取消 */
				BUZPI();
				input = in_d[pos];
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* 再表示 */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報調整－項目表示                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata_dsp( pos, data, rev )               |*/
/*| PARAMETER    : char   pos  : 項目番号                                  |*/
/*|              : short  data : データ                                    |*/
/*|              : ushort rev  : 反転/正転                                 |*/
/*| RETURN VALUE : nothing                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	locksts_set_flpdata_dsp(char pos, short data, ushort rev)
{
	switch (pos) {
	case 0:		/* 時 */
		opedsp(1, 10, (ushort)data, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 1:		/* 分 */
		opedsp(1, 16, (ushort)data, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 2:		/* 年 */
		opedsp(2, 6,  (ushort)data, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 3:		/* 月 */
		opedsp(2, 16, (ushort)data, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 4:		/* 日 */
		opedsp(2, 22, (ushort)data, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 5:		/* 状態 */
		switch (data) {
		case 0:
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[22]);		/* "待機" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　　　　" */
			break;
		case 1:
			grachr(3, 8, 14, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[0]);		/* "サービスタイム" */
			grachr(3, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "中" */
			break;
		case 2:
		case 3:
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);		/* "入庫" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　　　　" */
			break;
		default:	/* case 4,5 */
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);		/* "出庫" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　　　　" */
			break;
		}
		break;
	case 6:		/* 駐車 */
		if (data) {
			grachr(4, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[24]);		/* "車有" */
		}
		else {
			grachr(4, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[23]);		/* "車無" */
		}
		break;
	case 7:		/* ロック装置 */
		if (data) {
			grachr(4, 28, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[15]);		/* "閉" */
		}
		else {
			grachr(4, 28, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[16]);		/* "開" */
		}
		break;
	case 8:		/* 受付券 */
		if (data) {
			grachr(5, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "あり" */
		}
		else {
			grachr(5, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "なし" */
		}
		break;
	case 9:		/* 中止情報 */
		if (data) {
			grachr(5, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "あり" */
		}
		else {
			grachr(5, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "なし" */
		}
		break;
	default:	/* 暗証番号 (case 10:) */
		if (data) {
			grachr(6, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "あり" */
		}
		else {
			grachr(6, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "なし" */
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車室情報調整－項目判定                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata_check( pos, data )                  |*/
/*| PARAMETER    : char   pos  : 項目番号                                  |*/
/*|              : short  data : データ                                    |*/
/*| RETURN VALUE : 0:NG 1:OK                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char locksts_set_flpdata_check(char pos, short data)
{
	switch( pos ){
	case 0:		//hour
		if( data >= 24 ){
			return 0;
		}
		break;
	case 1:		//minit
		if( data >= 60 ){
			return 0;
		}
		break;
	case 2:		//year
		if(( data < 1990 ) || ( data > 2050 )){
			return 0;
		}
		break;
	case 3:		//month
		if(( data > 12 ) || ( data < 1 )){
			return 0;
		}
		break;
	case 4:		//date
		if(( data > 31 ) || ( data < 1 )){
			return 0;
		}
		break;
	default:
		break;
	}
	return 1;
}


/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：停留車情報                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_StaySts( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura CPrmSS[3][72]の参照方法を修正        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_StaySts(void)
{
	ushort	msg;
	char	dsp;
	short	days;
	char	ret;
	short	old;
	long	bk_prk;
	
	DP_CP[0] = DP_CP[1] = 0;
	gCurSrtPara = MNT_CAR;
	old = 1;
	days = -1;

	bk_prk = Is_CarMenuMake(CAR_2_MENU);
	do {
		msg = 0;
		dsp = 1;
		
		if (bk_prk) {								// 車室の設定が１種類以上あるか？
			if( Ext_Menu_Max > 1 ){					// 複数メニュー表示の場合
			// 駐車/駐輪選択画面
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[41]);		/* "＜停留車情報＞　　　　　　　　" */
				gCurSrtPara = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
				days = -1;							// 車室選択ﾒﾆｭｰに戻る場合には入力値をｸﾘｱ
			}
			else{									// １メニュー表示の場合
				gCurSrtPara = (short)bk_prk;
				bk_prk = 0;
			}
		}else{
			BUZPIPI();								// 車室設定が無いのでエラーではじく
			gCurSrtPara = MOD_EXT;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if ((gCurSrtPara == (short)MOD_EXT) || (gCurSrtPara == (short)MOD_CHG)){
		if ((gCurSrtPara == (short)MOD_EXT) || (gCurSrtPara == (short)MOD_CHG) || (gCurSrtPara == (short)MOD_CUT)){	
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			msg = gCurSrtPara;
			break;
		}

		while (msg != MOD_EXT) {
		
			if (dsp) {
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[((gCurSrtPara != MNT_BIK) ? 68 : 69)]);
														/* [68]: "＜停留車情報（駐車）＞　　　　" */
														/* [69]: "＜停留車情報（駐輪）＞　　　　" */

				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[42]);		/* "　日数を入力してください。　　" */
				grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[20]);			/* "停留日数" */
				grachr(4, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "：" */
				opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 日数 */
				grachr(4, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[5]);			/* "日" */
				Fun_Dsp(FUNMSG2[15]);					/* "　　　　　　 検索　　　　終了 " */
				dsp = 0;
			}

			msg = StoF( GetMessage(), 1 );
			switch(KEY_TEN0to9(msg)){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
			case KEY_TEN_F5:				/* F5:終了 */
				BUZPI();
				msg = MOD_EXT;
				break;
			case KEY_TEN_F3:				/* F3:検索 */
				if (days == -1) {
					days = old;
				}
				if (days == 0) {
					BUZPIPI();
					opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 日数 */
					days = -1;
					break;
				}
				BUZPI();
				ret = staysts_search(days);
				if (ret == 1) {
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				if (ret == 2) {
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				old = days;
				days = -1;
				dsp = 1;
				break;
			case KEY_TEN:
				BUZPI();
				if (days == -1) {
					days = 0;
				}
				days = (days*10 + msg-KEY_TEN0) % 100;
				opedsp(4, 16, (ushort)days, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 日数 */
				break;
			case KEY_TEN_CL:
				BUZPI();
				opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 日数 */
				days = -1;
				break;
			default:
				break;
			}
		}
	} while (bk_prk);

	return msg;
}

/*[]----------------------------------------------------------------------[]*/
/*|  停留車情報－検索結果画面                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_search( days )                                  |*/
/*| PARAMETER    : short days : 停留日数                                   |*/
/*| RETURN VALUE : 1 : mode change                                         |*/
/*|              : 0 : F5 key                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/20 ART:ogura 駐車中の車室のみ表示するように修正   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	staysts_search(short days)
{
	ushort	msg;
	short	i, j;
	short	max;
	short	top;
	short	tmp;
	T_FrmTeiRyuuJou	pri_data;
	short   index;
	short   start, end;
	
	/* 該当する車両を検索する */
	if( gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ){
		tmp = GetCarInfoParam();
		switch( tmp & 0x06 ){
			case 0x04:
				start = INT_CAR_START_INDEX;
				end   = start + INT_CAR_LOCK_MAX;
				break;
			case 0x06:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
				break;
			case 0x02:
			default:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX;
				break;
		}
		index = 68;
	}else {
		start = BIKE_START_INDEX;
		end   = start + BIKE_LOCK_MAX;
		index = 69;
	}

	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0 && FLAPDT.flp_data[i].nstat.bits.b00) {
			/* 指定日数経過しているかどうか判定 */
			if (staysts_check(&FLAPDT.flp_data[i], days) <= 0) {
				lock_idx[j++] = i;
			}
		}
	}
	max = j-1;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[index]);			/* [68]: "＜停留車情報（駐車）＞　　　　" */
												/* [69]: "＜停留車情報（駐輪）＞　　　　" */

	/* 該当データなし */
	if (max == -1) {
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "　　　データがありません　　　" */
		Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:終了 */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	/* 該当車両あり */
	if (CPrmSS[S_TYP][81] >= 2) {
		/* lock_idx[]を区画順にソートする */
		for (i = 0; i < max; i++) {
			for (j = max; j > i; j--) {
				if (LockInfo[lock_idx[j-1]].area > LockInfo[lock_idx[j]].area) {
					tmp = lock_idx[j];
					lock_idx[j] = lock_idx[j-1];
					lock_idx[j - 1]= tmp;
				}
			}
		}
	}

	top = 0;
	grachr(0, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[14]);			/* "計" */
	opedsp(0, 22, (ushort)(max+1), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	/* 合計台数 */
	grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);			/* "台" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[43]);			/* "　【車室番号】　【入庫日時】　" */
	staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);	/* データ一括表示 */
	Fun_Dsp(FUNMSG2[17]);					/* "　▲　　▼　       ﾌﾟﾘﾝﾄ 終了 " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return 1;
		case KEY_TEN_F5:				/* F5:終了 */
			BUZPI();
			return 0;
		case KEY_TEN_F4:				/* F4:ﾌﾟﾘﾝﾄ */
			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
				BUZPIPI();
				break;
			}
			BUZPI();
			/* プリント実行 */
			pri_data.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// 機械No.
			pri_data.Kakari_no = 0;
			pri_data.Day = days;								// 停留日数
			pri_data.Cnt = (short)(max+1);						// 停留車数
			pri_data.Data = &lock_idx[0];						// 停留車情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
			queset(PRNTCBNO, PREQ_TEIRYUU_JOU, sizeof(T_FrmTeiRyuuJou), &pri_data);
			Ope_DisableDoorKnobChime();							// 印字中のドアノブ開チャイム抑止
			/* プリント終了を待ち合わせる */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
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
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if ( msg == LCD_DISCONNECT) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			}
			break;
		case KEY_TEN_F1:				/* F1:▲ */
			BUZPI();
			top -= 5;
			if(top < 0) {
				top = (short)(max/5*5);
			}
			/* データ一括表示 */
			staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);
			break;
		case KEY_TEN_F2:				/* F2:▼ */
			BUZPI();
			top += 5;
			if(top > max) {
				top = 0;
			}
			/* データ一括表示 */
			staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  停留車情報－停留日数チェック                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_check( *p, days )                               |*/
/*| PARAMETER    : flp_com *p   : 先頭番号                                 |*/
/*|              : short   days : 表示番号                                 |*/
/*| RETURN VALUE : 正:指定日数未満 負or０:指定日数以上                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura CPrmSS[3][72]の参照方法を修正        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short staysts_check(flp_com *p, short days)
{
	short	year, mon, day, hour, min;
	short	ret;

	year = p->year;		/* 入庫年 */
	mon = p->mont;		/* 入庫月 */
	day = p->date;		/* 入庫日 */
	hour = p->hour;		/* 入庫時 */
	min = p->minu;		/* 入庫分 */

	/* 入庫日時からdays日後の日時を取得する */
	day += days;
	for ( ; ; ) {
		if (day <= medget(year, mon)) {
			break;
		}
		day -= medget(year, mon);
		if (++mon > 12) {
			mon = 1;
			year++;
		}
	}

	ret = year - CLK_REC.year;
	if (ret == 0) {
		ret = mon - CLK_REC.mont;
		if (ret == 0) {
			ret = day - CLK_REC.date;
			/* 日数を日付で算出する場合(CPrmSS[S_TYP][72]==1)はここまで */
			if (prm_get( COM_PRM,S_TYP,72,1,1 ) == 0 && ret == 0) {
				ret = hour - CLK_REC.hour;
				if (ret == 0) {
					ret = min - CLK_REC.minu;
				}
			}
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  停留車情報ー全データ表示(5件まで)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_all_dsp( top, max, rev_no, *idx )               |*/
/*| PARAMETER    : short top    : 先頭番号                                 |*/
/*|              : short max    : 最大番号                                 |*/
/*|              : short rev_no : 反転番号                                 |*/
/*|              : short *idx   : 精算履歴の配列番号テーブル               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	staysts_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		/* 車室番号表示 */
		if(rev_no == STAYSTS_AL_DISP_NO_REVERSE) {
			area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, 0);
		}
		else {
			area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		}
		/* 精算日時表示 */
		opedsp3((ushort)(2+i), 17, (ushort)FLAPDT.flp_data[idx[top+i]].mont, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 月 */
		grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);									/* "/" */
		opedsp3((ushort)(2+i), 20, (ushort)FLAPDT.flp_data[idx[top+i]].date, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 日 */
		opedsp3((ushort)(2+i), 23, (ushort)FLAPDT.flp_data[idx[top+i]].hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
		grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);									/* ":" */
		opedsp3((ushort)(2+i), 26, (ushort)FLAPDT.flp_data[idx[top+i]].minu, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：駐車台数                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkCnt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_ParkCnt(void)
{
	ushort	msg;
	long	*pPRM[3];
	char	changing;
	int		pos;		/* 0:満車 1:満車解除 2:現在 */
	long	count;
	char	dsp;
	char	loop;
	char	comm;
	char	redkey = 0;
	char	syskey = 0;
	char	wrikey = 0;

	ushort	disp_max, disp_no;

	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 0 ) {		// 駐車状況表示機能なし
		BUZPIPI();
		return MOD_EXT;
	}
	
	comm = 0;
	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 1 ) {		// 駐車状況 親機で管理
		comm = 1;

_RELOAD:	// 「再読」キー押下時jumpラベル

// 親機と通信しない設定なら起動しない
		if (!_is_ntnet_normal()) {
			BUZPIPI();
			return MOD_EXT;
		}
		// 台数問い合わせ実施
		NTNET_Snd_Data104(NTNET_MANDATA_PARKNUMCTL);	// NT-NET管理ﾃﾞｰﾀ要求作成(駐車台数管理)
		// 画面切替え
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "＜駐車台数＞　　　　　　　　　"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[96]);					// "　駐車台数問い合わせ中です　　"
		dsp = 0;
		grachr(4, 0, 30, (ushort)dsp, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	// "　　しばらくお待ち下さい　　　"
		Fun_Dsp(FUNMSG[0]);						// "　　　　　　　　　　　　　　　"
		Lagtim(OPETCBNO, 6, (ushort)(prm_get(COM_PRM, S_NTN, 31, 2, 5)*50+1));	// ﾀｲﾏｰ6(XXs)起動(NT-NET受信監視)
		Lagtim(OPETCBNO, 7, 25);				// 文字点滅用タイマーセット(500msec) */
		loop = 1;
		while (loop) {
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				// モードチェンジ
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;
			case TIMEOUT6:
				// 通信タイムアウト
				BUZPIPI();
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "＜駐車台数＞　　　　　　　　　"
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[97]);		// "駐車台数の取得に失敗しました　"
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[98]);		// "親機との通信を確認して下さい　"
				Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
				while (1) {
					msg = StoF( GetMessage(), 1 );
					switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case LCD_DISCONNECT:
						return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case KEY_MODECHG:
						// モードチェンジ
						BUZPI();
						return MOD_CHG;
					case KEY_TEN_F5:	/* 終了(F5) */
						BUZPI();
						return MOD_EXT;
					default:
						break;
					}
				}
				break;
			case TIMEOUT7:
				// 表示反転
				if (dsp) {
					dsp = 0;
				} else {
					dsp = 1;
				}
				grachr(4, 0, 30, (ushort)dsp, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);		// "　　しばらくお待ち下さい　　　"
				Lagtim(OPETCBNO, 7, 25);				// 文字点滅用タイマーセット(500msec) */
				break;
			case IBK_NTNET_DAT_REC:		// NTNETデータ受信
				// 受信データの駐車台数設定一致を確認
				if (NTNET_is234StateValid() != 1) {
					// 不一致ならエラー表示
					BUZPIPI();
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "＜駐車台数＞　　　　　　　　　"
					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[99]);		// "親機と本体との設定が不一致です"
					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[100]);		// "設定を確認して下さい　　　　　"
					Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
					while (1) {
						msg = StoF( GetMessage(), 1 );
						switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case LCD_DISCONNECT:
							return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
						case KEY_MODECHG:
							// モードチェンジ
							BUZPI();
							return MOD_CHG;
						case KEY_TEN_F5:	/* 終了(F5) */
							BUZPI();
							return MOD_EXT;
						default:
							break;
						}
					}
					break;
				} else {
					// 一致なら駐車台数表示へ(既存の処理)
					loop = 0;	// 台数表示処理へ移行
				}
				break;
			default:
				break;
			}
		}
	}

	/* 切替画面数の設定 */
	switch (prm_get(COM_PRM,S_SYS,39,1,1)) {
		case 3:  disp_max = 2; disp_no = 1; break;
		case 4:  disp_max = 3; disp_no = 1; break;
		case 0:
		default: disp_max = 0; disp_no = 0; break;	/* 切替なし */
	}
	
	pPRM[0] = &PPrmSS[S_P02][(4*disp_no)+3];
	pPRM[1] = &PPrmSS[S_P02][(4*disp_no)+4];
	pPRM[2] = &PPrmSS[S_P02][(4*disp_no)+2];

	ParkWkDt.CurNum		= *pPRM[2];				// 現在駐車台数
	ParkWkDt.NoFullNum	= *pPRM[1];				// 満車解除台数
	ParkWkDt.FullNum	= *pPRM[0];				// 満車台数

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);				/* "＜駐車台数＞　　　　　　　　　" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[16]);					/* "　　満車台数　　　　　　　　　" */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "　　　　　　　　：　　　　　　" */
	opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "　　　　　　　　　ＸＸＸＸ　　" */
	grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "　　　　　　　　　　　　　台　" */
	grachr(3, 4, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[4]);					/* "　　満車解除台数　　　　　　　" */
	grachr(3, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "　　　　　　　　：　　　　　　" */
	opedsp(3, 18, (ushort)*pPRM[1], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "　　　　　　　　　ＸＸＸＸ　　" */
	grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "　　　　　　　　　　　　　台　" */
	grachr(4, 4, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[3]);					/* "　　現在駐車台数　　　　　　　" */
	grachr(4, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "　　　　　　　　：　　　　　　" */
	opedsp(4, 18, (ushort)*pPRM[2], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "　　　　　　　　　ＸＸＸＸ　　" */
	grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "　　　　　　　　　　　　　台　" */
	grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[15]);					/* "　　空車台数　　　　　　　　　" */
	grachr(5, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "　　　　　　　　：　　　　　　" */
	count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
	opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* "　　　　　　　　　ＸＸＸＸ　　" */
	grachr(5, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "　　　　　　　　　　　　　台　" */

	if (comm != 1) {
		// 本体でカウントの場合
		if (disp_max > 0) {
			grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "［駐車１］" */
			Fun_Dsp(FUNMSG2[25]);					/* " 切替 　　　 変更　　　　終了 " */
		} else  {
			Fun_Dsp(FUNMSG2[0]);					/* "　　　　　　 変更　　　　終了 " */
		}
	} else {
		// 通信でカウントの場合
		if (disp_max > 0) {
			grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "［駐車１］" */
			Fun_Dsp(FUNMSG2[35]);					/* " 切替 再読　 変更　　　　終了 " */
		} else  {
			Fun_Dsp(FUNMSG2[33]);					/* "　　　再読　 変更　　　　終了 " */
		}
		redkey = 1;
	}

	changing = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT) {
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
					(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
					NT_pcars_timer = 0;
				}
				return MOD_EXT;
			case KEY_TEN_F3:	/* 変更(F3) */
				BUZPI();
				changing = 1;
				if (comm == 1) {
					*pPRM[2] = ParkWkDt.CurNum;			// 現在駐車台数
					*pPRM[1] = ParkWkDt.NoFullNum;		// 満車解除台数
					*pPRM[0] = ParkWkDt.FullNum;		// 満車台数
				}
				opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 満車台数 反転 */
				if (comm != 1) {
					if (prm_get(COM_PRM,S_NTN,38,1,3) == 2) {
						Fun_Dsp(FUNMSG2[37]);		/* "　▲　　▼　 ｼｽﾃﾑ　書込　終了 " */
						syskey = 1;
						wrikey = 1;
					} else {
						Fun_Dsp(FUNMSG[20]);		/* "　▲　　▼　　　　 書込  終了 " */
						syskey = 0;
						wrikey = 1;
					}
				} else {
					Fun_Dsp(FUNMSG2[36]);		/* "　▲　　▼　 ｼｽﾃﾑ　　　　終了 " */
					syskey = 1;
					wrikey = 0;
				}
				count = -1;
				pos = 0;
				break;
			case KEY_TEN_F1:	/* 切替(F1) */
				if (disp_max > 0) {
					BUZPI();
					
					/* 画面番号設定 */
					disp_no = (disp_no == disp_max) ? 1 : (disp_no + 1);
					
					pPRM[0] = &PPrmSS[S_P02][(4*disp_no)+3];
					pPRM[1] = &PPrmSS[S_P02][(4*disp_no)+4];
					pPRM[2] = &PPrmSS[S_P02][(4*disp_no)+2];
					
					ParkWkDt.CurNum	   = *pPRM[2];			// 現在駐車台数
					ParkWkDt.NoFullNum = *pPRM[1];			// 満車解除台数
					ParkWkDt.FullNum   = *pPRM[0];			// 満車台数

					/* 数値書換え */
					grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[(disp_no-1)]);					/* "［駐車Ｘ］" */
					opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 満車台数     */
					opedsp(3, 18, (ushort)*pPRM[1], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 満車解除台数 */
					opedsp(4, 18, (ushort)*pPRM[2], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 現在駐車台数 */
					count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
					opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* 空車台数     */
				}
				break;
			case KEY_TEN_F2:	/* 再読(F2) */
				if (redkey) {
					goto _RELOAD;	// NT-NET通信処理へ戻る
				}
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F3:	/* ｼｽﾃﾑ(F3) */
				if (syskey == 1) {
					// 内部保持データ書換え
					if (count != -1) {
						*pPRM[pos] = count;
						if (pos != 1) {
							count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
							opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
						}
					}
					// 制御データ送信(現在台数管理データ)
					ParkWkDt.CurNum    = *pPRM[2];			// 現在駐車台数
					ParkWkDt.NoFullNum = *pPRM[1];			// 満車解除台数
					ParkWkDt.FullNum   = *pPRM[0];			// 満車台数
					changing = 0;
				} else {
					break;
				}
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				if (comm != 1) {
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 台数 正転 */
				}else{
					opedsp((ushort)(2), 18, (ushort)ParkWkDt.FullNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 台数 正転 */
					opedsp((ushort)(3), 18, (ushort)ParkWkDt.NoFullNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 台数 正転 */
					opedsp((ushort)(4), 18, (ushort)ParkWkDt.CurNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 台数 正転 */
					count = (ParkWkDt.FullNum > ParkWkDt.CurNum) ? (ParkWkDt.FullNum-ParkWkDt.CurNum) : 0;
					opedsp((ushort)(5), 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 台数 正転 */
					*pPRM[2] = ParkWkDt.CurNum;			// 現在駐車台数
					*pPRM[1] = ParkWkDt.NoFullNum;		// 満車解除台数
					*pPRM[0] = ParkWkDt.FullNum;		// 満車台数
				}
				if (comm != 1) {
					// 本体でカウントの場合
					if (disp_max > 0) {
						Fun_Dsp(FUNMSG2[25]);			/* " 切替 　　　 変更　　　　終了 " */
					} else {
						Fun_Dsp(FUNMSG2[0]);			/* "　　　　　　 変更　　　　終了 " */
					}
				} else {
					// 通信でカウントの場合
					if (disp_max > 0) {
						Fun_Dsp(FUNMSG2[35]);			/* " 切替 再読　 変更　　　　終了 " */
					} else {
						Fun_Dsp(FUNMSG2[33]);			/* "　　　再読　 変更　　　　終了 " */
					}
				}
				changing = 0;
				break;
			case KEY_TEN_F4:	/* 書込(F4) */
				if (!wrikey) {
					break;
				} else {
				if (count != -1) {
					*pPRM[pos] = count;
					OpelogNo = OPLOG_CHUSHADAISU;		// 操作履歴登録
					if (pos != 1) {
						count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
						opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 空車台数 */
					}
				}
				}
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				if (comm != 1) {
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 台数 正転 */
				}else{
					if (count != -1) {
						*pPRM[pos] = count;
					}
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 台数 正転 */
					if (pos != 1) {
						count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
						opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 空車台数 */
					}
				}
				if (msg == KEY_TEN_F1) {
					if (--pos < 0) {
						pos = 2;
					}
				}
				else {
					if (++pos > 2) {
						pos = 0;
					}
				}
				opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 台数 反転 */
				count = -1;
				break;
			case KEY_TEN:		/* 数字(テンキー) */
				BUZPI();
				if (count == -1) {
					count = 0;
				}
				count = (count*10 + msg-KEY_TEN0) % 10000;
				opedsp((ushort)(2+pos), 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 台数 反転 */
				break;
			case KEY_TEN_CL:	/* 取消(テンキー) */
				BUZPI();
				opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 台数 反転 */
				count = -1;
				break;
			default:
				break;
			}
		}
	}	
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：満車コントロール                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_FullCtrl( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_FullCtrl(void)
{
	ushort	msg;
	long	*pCTRL;
	long	val_tbl[] = {1, 0, 2};
	const uchar	*str_tbl[] = {DAT4_2[18], DAT4_2[13], DAT4_2[17]};
	int		pos;		/* 0:強制満車 1:自動 2:強制空車 */

	ushort	disp_max, disp_no;

	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 0 ) {		// 駐車状況表示機能なし
		BUZPIPI();
		return MOD_EXT;
	}
	
	/* 切替画面数の設定 */
	switch (prm_get(COM_PRM,S_SYS,39,1,1)) {
		case 3:  disp_max = 2; disp_no = 1; break;
		case 4:  disp_max = 3; disp_no = 1; break;
		case 0:
		default: disp_max = 0; disp_no = 0; break;	/* 切替なし */
	}
	
	pCTRL = &PPrmSS[S_P02][(4*disp_no)+1];
	for (pos = 0; pos < 2; pos++) {
		if (*pCTRL == val_tbl[pos]) {
			break;
		}
	}
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[35] );					/* "＜満車コントロール＞　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "　現在の状態　　　　　　　　　" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "　　　　　　：　　　　　　　　" */
	grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "　　　　　　　ＸＸＸＸ　　　　" */
	grachr( 4, 12,  8, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "　　　　　　　強制満車　　　　" */
	grachr( 5, 12,  8, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "　　　　　　　　自動　　　　　" */
	grachr( 6, 12,  8, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[2] );	/* "　　　　　　　強制空車　　　　" */

	if (disp_max > 0) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "［駐車１］" */
		Fun_Dsp( FUNMSG2[27] );							/* "　▲　　▼　 切替  書込  終了 " */
	}
	else {
		Fun_Dsp( FUNMSG[20] );							/* "　▲　　▼　　　　 書込  終了 " */
	}

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_CHG );
		case KEY_TEN_F5:					/* F5:Exit */
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_EXT );
		case KEY_TEN_F3:					/* F3:Switch */
			if (disp_max > 0) {
				BUZPI();

				/* 画面番号設定 */
				disp_no = (disp_no == disp_max) ? 1 : (disp_no + 1);
				grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[(disp_no-1)]);				/* "［駐車Ｘ］" */
				pCTRL = &PPrmSS[S_P02][(4*disp_no)+1];

				grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				for (pos = 0; pos < 2; pos++) {
					if (*pCTRL == val_tbl[pos]) {
						break;
					}
				}
				grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
				grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			}
			break;
		case KEY_TEN_F1:					/* F1: */
		case KEY_TEN_F2:					/* F2: */
			BUZPI();
			grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			if (msg == KEY_TEN_F1) {
				if (--pos < 0) {
					pos = 2;
				}
			}
			else {
				if (++pos > 2) {
					pos = 0;
				}
			}
			grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			break;
		case KEY_TEN_F4:					/* F4:書込 */
			BUZPI();
			*pCTRL = val_tbl[pos];
			OpelogNo = OPLOG_MANSYACONTROL;			// 操作履歴登録
			grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：領収証再発行                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Receipt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]をﾜｰｸｴﾘｱとして使用する。                       [MH544401] |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Receipt(void)
{
#define	RERECEIPT_MAX_COUNT		50
	ushort			msg;
	ushort			max;				/* 最大番号(履歴数-1) */
	ushort			top;				/* 先頭番号 */
	ushort			no;					/* 番号(0～max) */
	T_FrmReceipt	rec_data;			/* 領収証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ */
	ushort			LogTotalCount;
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	ushort			jnl_pri_wait_Timer;	// ジャーナル印字完了待ちタイマー
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	Receipt_data	*pWkRec = (Receipt_data*)&SysMnt_Work[0];
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

	LogTotalCount = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP );					// 総精算完了LOG件数get
	if( RERECEIPT_MAX_COUNT < LogTotalCount ){
		max = RERECEIPT_MAX_COUNT;
	}else{
		max = LogTotalCount;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[45]);			/* "＜領収証再発行＞　　　　　　　" */

	/* 履歴なし */
	if (max == 0) {
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[48]);		/* "　　 精算履歴がありません 　　" */
		Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　　　　　　 終了 " */
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
				return( MOD_EXT );
			default:
				break;
			}
		}
	}

	/* 履歴あり */
	top = no = 0;
// MH810100(S) Y.Yamauchi 20191227 車番チケットレス(メンテナンス)
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[47]);			/* "　【車室番号】　【精算日時】　" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[185]);			/* 	"　【車番】　【精算日時】　", */
// MH810100(E) Y.Yamauchi 20191227 車番チケットレス(メンテナンス)
	receipt_all_dsp(top, max, no, LogTotalCount);		/* データ一括表示 */

	if( ( prm_get(COM_PRM,S_PAY,25,2,1) != 0 )&&( CPrmSS[S_KOU][1] ) ){		// 定期更新あり&ﾗﾍﾞﾙﾌﾟﾘﾝﾀあり
		Fun_Dsp(FUNMSG2[23]);						/* "　▲　　▼　ラベル 発行  終了 " */
	}else{
		Fun_Dsp(FUNMSG2[16]);						/* "　▲　　▼　　　　 発行  終了 " */
	}

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
			return( MOD_EXT );
		case KEY_TEN_F4:				/* F4:発行 */
// MH810105(S) MH364301 インボイス対応
//			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
			if(Ope_isPrinterReady() == 0 ||			// レシートプリンタが印字不可能な状態
				(IS_INVOICE &&
// GG129000(S) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5689：#6819電子ジャーナル使用不可発生中にメンテナンスで領収証再発行するとエラー音が再生されない）
//				 Ope_isJPrinterReady() == 0)){		// インボイス設定ありはジャーナル状態もチェック
// GG129000(S) M.Fujikawa 2023/10/18 ゲート式車番チケットレスシステム対応　不具合#7167
//				(Ope_isJPrinterReady() && (paperchk2() == -1)))) {
				(Ope_isJPrinterReady() == 0 || (paperchk2() == -1)))) {
// GG129000(E) M.Fujikawa 2023/10/18 ゲート式車番チケットレスシステム対応　不具合#7167
// GG129000(E) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5689：#6819電子ジャーナル使用不可発生中にメンテナンスで領収証再発行するとエラー音が再生されない）
// MH810105(E) MH364301 インボイス対応
				BUZPIPI();
				break;
			}
			BUZPI();
			/* 領収証再発行 */
// GG129002(S) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
			memset(&rec_data, 0, sizeof(rec_data));
// GG129002(E) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
			rec_data.prn_kind = R_PRI;									/* ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ */
// MH810105(S) MH364301 インボイス対応
			if (IS_INVOICE) {
				rec_data.prn_kind = J_PRI;									// ジャーナル印字が先
			}
// MH810105(E) MH364301 インボイス対応
			(void)Ope_SaleLog_1DataGet( (ushort)(LogTotalCount - no - 1), PAY_LOG_CMP, LogTotalCount, pWkRec );
			rec_data.prn_data = pWkRec;										/* 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ */
			rec_data.kakari_no = OPECTL.Kakari_Num;							// 係員No.
			rec_data.reprint = ON;											// 再発行ﾌﾗｸﾞｾｯﾄ（再発行）
			memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );	// 再発行日時（現在日時）ｾｯﾄ

			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			Ope_DisableDoorKnobChime();
			OpelogNo = OPLOG_RYOSHUSAIHAKKO;		// 操作履歴登録
			/* プリント終了を待ち合わせる */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
			for ( ; ; ) {
				msg = StoF( GetMessage(), 1 );
				/* プリント終了 */
				if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
// MH810105(S) MH364301 インボイス対応
//					Lagcan(OPETCBNO, 6);
// MH810105(E) MH364301 インボイス対応
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブ閉状態であればメンテナンス終了
					if (CP_MODECHG) {
// MH810105(S) MH364301 インボイス対応
						Lagcan(OPETCBNO, 6);
						Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 インボイス対応
						return MOD_CHG;
					}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
// MH810105(S) MH364301 インボイス対応
//					break;
					if (IS_INVOICE) {
						// インボイス設定あり
						if (OPECTL.Pri_Kind == J_PRI) {
							if (OPECTL.Pri_Result == PRI_NML_END) {
								// 短い領収証（現金精算など）印字で印字中にフタを開けた場合、
								// 印字完了→印字失敗という順で通知される場合がある
								// そのため、印字完了受信後にWaitタイマを動作させて
								// タイマ動作中に印字失敗を受信した場合は印字失敗として扱う
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//								Lagtim(OPETCBNO, 7, 150);	// 3sec timer start
								jnl_pri_wait_Timer = GyouCnt_All_j + GyouCnt_j;	// 送信済み印字ﾃﾞｰﾀ全行数取得
								Lagtim(OPETCBNO, 7, (jnl_pri_wait_Timer * 5));	// 1行/100ms(20ms*5)
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
							}
							else {
								Lagcan(OPETCBNO, 6);
								break;
							}
						}
						else {
							Lagcan(OPETCBNO, 6);
							break;
						}
					}
					else {
						// インボイス設定なし
						Lagcan(OPETCBNO, 6);
						break;
					}
// MH810105(E) MH364301 インボイス対応
				}
				/* タイムアウト(10秒)検出 */
				if (msg == TIMEOUT6) {
					BUZPIPI();
					break;
				}
// MH810105(S) MH364301 インボイス対応
				/* タイムアウト(3秒)検出 */
				if(msg == TIMEOUT7){
					// レシート印字可能
					if (Ope_isPrinterReady() &&
						paperchk() != -1 &&
						Ope_isJPrinterReady() &&
						paperchk2() != -1) {
						rec_data.prn_kind = R_PRI;	// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
						queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
						/* プリント終了を待ち合わせる */
						Lagtim(OPETCBNO, 6, 500);	// 10sec timer start
					}
					// レシート印字不可
					else{
						Lagcan(OPETCBNO, 6);
						break;
					}
				}
// MH810105(E) MH364301 インボイス対応
				/* モードチェンジ */
				if (msg == KEY_MODECHG) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
// MH810105(S) MH364301 インボイス対応
					Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 インボイス対応
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if ( msg == LCD_DISCONNECT ) {	
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			}
// MH810105(S) MH364301 インボイス対応
			// for文を抜ける時念のためlagcan7をしておく
			Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 インボイス対応
			break;
		case KEY_TEN_F3:				/* F3:ラベル */
			break;
		case KEY_TEN_F1:				/* F1:▲ */
			BUZPI();
			if (no > 0) { 
				no--;
			}else{
				no = max - 1;
				top = (max-1)/5*5;
			}
			if (no < top) {
				top -= 5;
			}
			/* データ一括表示 */
			receipt_all_dsp(top, max, no, LogTotalCount);
			break;
		case KEY_TEN_F2:				/* F2:▼ */
			BUZPI();
			if (no < (max-1)) {
				no++;
			}else{
				top = no = 0;
			}
			if (no > top+4) {
				top = no;
			}
			/* データ一括表示 */
			receipt_all_dsp(top, max, no, LogTotalCount);
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  領収証再発行ー全データ表示(5件まで)                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : receipt_all_dsp( top, max, rev_no, TotalCount )         |*/
/*| PARAMETER    : ushort top        : 先頭番号                            |*/
/*|              : ushort max        : 最大番号                            |*/
/*|              : ushort rev_no     : 反転番号                            |*/
/*|              : ushort TotalCount : FLASHも含めた全件数      [MH544401] |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]をﾜｰｸｴﾘｱとして使用する。                       [MH544401] |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	receipt_all_dsp(ushort top, ushort max, ushort rev_no, ushort TotalCount)
{
	char	i;
	Receipt_data	*p = (Receipt_data*)&SysMnt_Work[0];
	t_OpeLog_FindNextLog	NextLogInfo;
// MH810100(S) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)
	ushort mod = 0;
// MH810100(E) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)

	for (i = 0; i < 5; i++) {
		if (top+i >= max) {
			break;
		}
		if(i == 0) {
			(void)Ope_SaleLog_First1DataGet( (ushort)(TotalCount - top - i - 1), PAY_LOG_CMP, TotalCount, p, &NextLogInfo);
		}
		else {
			(void)Ope_SaleLog_Next1DataGet( (ushort)(TotalCount - top - i - 1), (uchar)PAY_LOG_CMP, p, &NextLogInfo);
		}
		/* 車室番号表示 */
// MH810100(S) Y.Yamauchi 20191227 車番チケットレス(メンテナンス)
//		area_posi_dsp((short)top, (short)(top+i), (char)(p->WPlace/10000), (ulong)(p->WPlace%10000), (rev_no==top+i)?1:0);
// MH810100(S) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)
//		if( rev_no == (top+i) ){
//			grachr((ushort)(2+i), 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber));		/*車番*/
//		}else {
//			grachr((ushort)(2+i), 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber));		/*車番*/
//		}
		if( rev_no == (top+i) ){
			mod = 1;
		}else{
			mod = 0;
		}
		if( p->PayMode !=4 ){
// MH810100(S) 2020/07/28 領収証再発行で車番がないとわかりづらい
//			grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber) );	// 車番
			if( p->CarNumber[0] != NULL){
				grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber) );	// 車番
			}else{
				grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, RECEIPT_DAT1[1] );	// 車番
			}
// MH810100(E) 2020/07/28 領収証再発行で車番がないとわかりづらい
		}else{
			grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, RECEIPT_DAT1[0] );				// "遠隔"
		}
// MH810100(E) K.Onodera  2020/03/25 車番チケットレス(#3976 遠隔精算時に領収証再発行の表示が分かりづらい)
// MH810100(E) Y.Yamauchi 20191227 車番チケットレス(メンテナンス)
		
		/* 精算日時表示 */
// MH810100(S) 2020/09/04 #4864 メンテナンスの領収証再発行画面で未精算出庫精算の【精算日時】が出庫時刻で表示されている
//		opedsp3((ushort)(2+i), 17, (ushort)p->TOutTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 月 */
//		grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);						/* "/" */
//		opedsp3((ushort)(2+i), 20, (ushort)p->TOutTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 日 */
//		opedsp3((ushort)(2+i), 23, (ushort)p->TOutTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 時 */
//		grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);						/* ":" */
//		opedsp3((ushort)(2+i), 26, (ushort)p->TOutTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 分 */
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//		// 種別	(0=事前精算／1＝未精算出庫精算)
//		if(p->shubetsu == 0){
		// 種別	(0=事前精算／1＝未精算出庫精算／2=遠隔精算(精算中変更))
		if(p->shubetsu == 0 || p->shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
			opedsp3((ushort)(2+i), 17, (ushort)p->TOutTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 月 */
			grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);						/* "/" */
			opedsp3((ushort)(2+i), 20, (ushort)p->TOutTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 日 */
			opedsp3((ushort)(2+i), 23, (ushort)p->TOutTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 時 */
			grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);						/* ":" */
			opedsp3((ushort)(2+i), 26, (ushort)p->TOutTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 分 */
		}else{
			opedsp3((ushort)(2+i), 17, (ushort)p->TUnpaidPayTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 月 */
			grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							/* "/" */
			opedsp3((ushort)(2+i), 20, (ushort)p->TUnpaidPayTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 日 */
			opedsp3((ushort)(2+i), 23, (ushort)p->TUnpaidPayTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
			grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);							/* ":" */
			opedsp3((ushort)(2+i), 26, (ushort)p->TUnpaidPayTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 分 */
		}
// MH810100(E) 2020/09/04 #4864 メンテナンスの領収証再発行画面で未精算出庫精算の【精算日時】が出庫時刻で表示されている
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  個別精算LOG件数取得													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_SaleLog_TotalCountGet( void )						|*/
/*| PARAMETER    : Kind  = ログ件数カウント指定(bit対応)					|*/
/*|							all"0":全個別精算ログ件数をget					|*/
/*|							bit0=1:精算完了(不正・強制出庫を除く)の個別精算	|*/
/*|							       ログ件数をget							|*/
/*|							bit1=1:精算中止・再精算中止の個別精算ログ件数を	|*/
/*|							       get										|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   精算完了(不正・強制出庫を除く)・精算中止・再	|*/
/*|							   精算中止の個別精算ログ件数をgetする			|*/
/*| RETURN VALUE : cnt_tgt = Kindで指定された個別精算ログの登録件数			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-02-28												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
ushort	Ope_SaleLog_TotalCountGet( uchar Kind )
{
	ushort	i;
	ushort	cnt_ttl;
	ushort	cnt_tgt = 0;
	uchar	cnt_flg = FALSE;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// 個別精算ログ登録総件数get(Flash+SRAM)

	if( Kind ){														// ログ件数取得指定あり?
		for( i = 0; i < cnt_ttl; i++ ){
			WACDOG;													// ログ検索の際はウォッチドックリセット実行
			memset( &PayLog_Work, 0, sizeof( Receipt_data ) );		// ワークエリアクリア
			ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, i, &PayLog_Work );// 個別精算ログ1件取出し
			if( ulRet ){											// 取出しOK
				if(( PayLog_Work.PayClass == 2 ) ||					// 処理区分＝精算中止?
				   ( PayLog_Work.PayClass == 3 )){					// 処理区分＝再精算中止?
					if( Kind & 0x02 ){								// 精算中止／再精算中止の指定あり?
						cnt_flg = TRUE;								// ログ件数に含める
					}
				}else if(( PayLog_Work.OutKind !=  3 ) &&			// 精算出庫≠不正出庫?
						 ( PayLog_Work.OutKind !=  1 ) &&			// 精算出庫≠強制出庫?
						 ( PayLog_Work.OutKind != 11 ) &&			// 精算出庫≠強制出庫(遠隔)?
// MH321800(S) D.Inaba ICクレジット対応
//						 ( PayLog_Work.OutKind != 97 )){			// フラップ上昇、ロック閉タイマ内出庫
						 ( PayLog_Work.OutKind != 97 ) &&			// フラップ上昇、ロック閉タイマ内出庫
						 ( PayLog_Work.Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// 復決済ではない
// MH321800(E) D.Inaba ICクレジット対応
						if( Kind & 0x01 ){							// 精算完了の指定あり?
							cnt_flg = TRUE;							// ログ件数に含める
					}
				}
				if( cnt_flg == TRUE ){								// ログ件数に含める?
					cnt_tgt++;										// ログ件数カウントアップ
				}
				cnt_flg = FALSE;
			}else{													// 取出しNG
				cnt_tgt = 0;
				break;
			}
		}
	}else{															// 全個別精算ログ件数get
		cnt_tgt = cnt_ttl;
	}

	return cnt_tgt;
}

/*[]-----------------------------------------------------------------------[]*/
/*|  個別精算LOG 1件取出し													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	最古ﾃﾞｰﾀを0とした登録番号を元に、指定ﾃﾞｰﾀを1件分読み出す。				|*/
/*|	Call側にはﾃﾞｰﾀがSRAM or FlashROM どちらにあるかを意識させない関数。		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = 目的ﾃﾞｰﾀの番号（0～ ）							|*/
/*|						   0=最古、最大数値=最新							|*/
/*|						  （例：登録件数=1000件時の最新ﾃﾞｰﾀをGetする場合999	|*/
/*|							を渡すこと）									|*/
/*|				   Kind  = 取り出し指定(bit対応)							|*/
/*|							all"0":全個別精算ログを取出し対象とする			|*/
/*|							bit0=1:精算完了(不正・強制出庫を除く)の個別精算	|*/
/*|							       ログを取出し対象とする					|*/
/*|							bit1=1:精算中止・再精算中止の個別精算ログを取出	|*/
/*|							       し対象とする								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   精算完了(不正・強制出庫を除く)・精算中止・再	|*/
/*|							   精算中止の個別精算ログを取出し対象とする		|*/
/*|				   Index_Kind  = 指定された個別精算ログの総件数				|*/
/*|								 (Ope_SaleLog_TotalCountGet()で取得)		|*/
/*|				   pSetBuf = Readﾃﾞｰﾀをｾｯﾄするｴﾘｱへのﾎﾟｲﾝﾀ					|*/
/*| RETURN VALUE : 1=Read成功、0=失敗										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-03-05												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf )
{
	ushort	cnt_ttl;
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// 個別精算ログ登録総件数get(Flash+SRAM)
	index_tgt = cnt_ttl - 1;
	index_knd = Index_Kind - 1;

	while( !flg_end ){
		WACDOG;														// ログ検索の際はウォッチドックリセット実行
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// 個別精算ログ取り出しOK
			if( !Kind ){											// 全個別精算ログ取出し対象?
				flg_tgt = TRUE;										// 指定された個別精算ログ取出しOK
			}else if(( pSetBuf->PayClass == 2 ) ||					// 処理区分＝精算中止?
					 ( pSetBuf->PayClass == 3 )){					// 処理区分＝再精算中止?
				if( Kind & 0x02 ){									// 精算中止／再精算中止の指定あり
					flg_tgt = TRUE;									// 指定された個別精算ログ取出しOK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// 精算出庫≠不正出庫?
					 ( pSetBuf->OutKind !=  1 ) &&					// 精算出庫≠強制出庫?
					 ( pSetBuf->OutKind != 11 ) &&					// 精算出庫≠強制出庫(遠隔)?
// MH321800(S) D.Inaba ICクレジット対応
//					 ( pSetBuf->OutKind != 97 )){					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->OutKind != 97 ) &&					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// 復決済ではない
// MH321800(E) D.Inaba ICクレジット対応
					if( Kind & 0x01 ){								// 精算完了の指定あり?
						flg_tgt = TRUE;								// 指定された個別精算ログ取出しOK
				}
			}
			if( flg_tgt == TRUE ){									// 指定されたログ?
				if( index_knd == Index ){							// 指定された番号?
					flg_end = ON;									// 検索終了(Read成功)
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// 指定された個別精算ログ総件数検索継続?
						index_knd--;								// 次の指定された個別精算ログ検索
					}else{
						flg_end = ON;								// 検索終了(Read失敗)
					}
				}
			}
			if( flg_end == OFF ){									// 検索終了?
				if( index_tgt ){									// 個別精算ログ総件数検索継続?
					index_tgt--;									// 次の個別精算ログ検索
				}else{
					flg_end = ON;									// 検索終了(Read失敗)
				}
			}
		}else{
			flg_end = ON;											// 個別精算ログ取り出しNG
		}
	}

	return flg_tgt;
}
/*[]-----------------------------------------------------------------------[]*/
/*|  個別精算LOG 1件取出し													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	最古ﾃﾞｰﾀを0とした登録番号を元に、指定ﾃﾞｰﾀを1件分読み出す。				|*/
/*|	Call側にはﾃﾞｰﾀがSRAM or FlashROM どちらにあるかを意識させない関数。		|*/
/*| 連続でReadする場合に使用する関数。										|*/
/*| 出力されたpNextLogをそのままOpe_SaleLog_Next1DataGet()に使用する		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = 目的ﾃﾞｰﾀの番号（0～ ）							|*/
/*|						   0=最古、最大数値=最新							|*/
/*|						  （例：登録件数=1000件時の最新ﾃﾞｰﾀをGetする場合999	|*/
/*|							を渡すこと）									|*/
/*|				   Kind  = 取り出し指定(bit対応)							|*/
/*|							all"0":全個別精算ログを取出し対象とする			|*/
/*|							bit0=1:精算完了(不正・強制出庫を除く)の個別精算	|*/
/*|							       ログを取出し対象とする					|*/
/*|							bit1=1:精算中止・再精算中止の個別精算ログを取出	|*/
/*|							       し対象とする								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   精算完了(不正・強制出庫を除く)・精算中止・再	|*/
/*|							   精算中止の個別精算ログを取出し対象とする		|*/
/*|				   Index_Kind  = 指定された個別精算ログの総件数				|*/
/*|								 (Ope_SaleLog_TotalCountGet()で取得)		|*/
/*|				   pSetBuf = Readﾃﾞｰﾀをｾｯﾄするｴﾘｱへのﾎﾟｲﾝﾀ					|*/
/*|				   pNextLog = 次に検索するログ情報格納領域					|*/
/*|				   				Ope_SaleLog_Next1DataGet()で使用する		|*/
/*| RETURN VALUE : 1=Read成功、0=失敗										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-03-05												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_First1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog )
{
	ushort	cnt_ttl;
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// 個別精算ログ登録総件数get(Flash+SRAM)
	index_tgt = cnt_ttl - 1;
	index_knd = Index_Kind - 1;

	while( !flg_end ){
		WACDOG;														// ログ検索の際はウォッチドックリセット実行
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// 個別精算ログ取り出しOK
			if( !Kind ){											// 全個別精算ログ取出し対象?
				flg_tgt = TRUE;										// 指定された個別精算ログ取出しOK
			}else if(( pSetBuf->PayClass == 2 ) ||					// 処理区分＝精算中止?
					 ( pSetBuf->PayClass == 3 )){					// 処理区分＝再精算中止?
				if( Kind & 0x02 ){									// 精算中止／再精算中止の指定あり
					flg_tgt = TRUE;									// 指定された個別精算ログ取出しOK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// 精算出庫≠不正出庫?
					 ( pSetBuf->OutKind !=  1 ) &&					// 精算出庫≠強制出庫?
					 ( pSetBuf->OutKind != 11 ) &&					// 精算出庫≠強制出庫(遠隔)?
// MH321800(S) D.Inaba ICクレジット対応
//					 ( pSetBuf->OutKind != 97 )){					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->OutKind != 97 ) &&					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// 復決済ではない
// MH321800(E) D.Inaba ICクレジット対応
					if( Kind & 0x01 ){								// 精算完了の指定あり?
						flg_tgt = TRUE;								// 指定された個別精算ログ取出しOK
				}
			}
			if( flg_tgt == TRUE ){									// 指定されたログ?
				if( index_knd == Index ){							// 指定された番号?
					flg_end = ON;									// 検索終了(Read成功)
					if(pNextLog) {
						pNextLog->nextIndex = index_tgt;			// 検索したインデックスを格納する
						pNextLog->nextCount = index_knd;			// 次の検索対象件数を格納する
					}
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// 指定された個別精算ログ総件数検索継続?
						index_knd--;								// 次の指定された個別精算ログ検索
					}else{
						flg_end = ON;								// 検索終了(Read失敗)
					}
				}
			}
			if( flg_end == OFF ){									// 検索終了?
				if( index_tgt ){									// 個別精算ログ総件数検索継続?
					index_tgt--;									// 次の個別精算ログ検索
				}else{
					flg_end = ON;									// 検索終了(Read失敗)
				}
			}
		}else{
			flg_end = ON;											// 個別精算ログ取り出しNG
		}
	}

	return flg_tgt;
}
/*[]-----------------------------------------------------------------------[]*/
/*|  個別精算LOG 1件取出し													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	最古ﾃﾞｰﾀを0とした登録番号を元に、指定ﾃﾞｰﾀを1件分読み出す。				|*/
/*|	Call側にはﾃﾞｰﾀがSRAM or FlashROM どちらにあるかを意識させない関数。		|*/
/*| 連続でReadする場合に使用する関数。										|*/
/*|	Ope_SaleLog_First1DataGet()の後にコールすること							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = 目的ﾃﾞｰﾀの番号（0～ ）							|*/
/*|						   0=最古、最大数値=最新							|*/
/*|						  （例：登録件数=1000件時の最新ﾃﾞｰﾀをGetする場合999	|*/
/*|							を渡すこと）									|*/
/*|				   Kind  = 取り出し指定(bit対応)							|*/
/*|							all"0":全個別精算ログを取出し対象とする			|*/
/*|							bit0=1:精算完了(不正・強制出庫を除く)の個別精算	|*/
/*|							       ログを取出し対象とする					|*/
/*|							bit1=1:精算中止・再精算中止の個別精算ログを取出	|*/
/*|							       し対象とする								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   精算完了(不正・強制出庫を除く)・精算中止・再	|*/
/*|							   精算中止の個別精算ログを取出し対象とする		|*/
/*|				   pSetBuf = Readﾃﾞｰﾀをｾｯﾄするｴﾘｱへのﾎﾟｲﾝﾀ					|*/
/*|				   pNextLog = 次に検索するログ情報格納領域					|*/
/*| 							そのまま次のOpe_SaleLog_Next1DataGet()に使用|*/
/*|								する										|*/
/*| RETURN VALUE : 1=Read成功、0=失敗										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : S.Takahashi												|*/
/*| Date		 : 2012-05-11												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_Next1DataGet( ushort Index, uchar Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog )
{
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	index_tgt = pNextLog->nextIndex - 1;							// 検索開始インデックス
	index_knd = pNextLog->nextCount - 1;							// 検索対象のログ件数

	while( !flg_end ){
		WACDOG;														// ログ検索の際はウォッチドックリセット実行
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// 個別精算ログ取り出しOK
			if( !Kind ){											// 全個別精算ログ取出し対象?
				flg_tgt = TRUE;										// 指定された個別精算ログ取出しOK
			}else if(( pSetBuf->PayClass == 2 ) ||					// 処理区分＝精算中止?
					 ( pSetBuf->PayClass == 3 )){					// 処理区分＝再精算中止?
				if( Kind & 0x02 ){									// 精算中止／再精算中止の指定あり
					flg_tgt = TRUE;									// 指定された個別精算ログ取出しOK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// 精算出庫≠不正出庫?
					 ( pSetBuf->OutKind !=  1 ) &&					// 精算出庫≠強制出庫?
					 ( pSetBuf->OutKind != 11 ) &&					// 精算出庫≠強制出庫(遠隔)?
// MH321800(S) D.Inaba ICクレジット対応
//					 ( pSetBuf->OutKind != 97 )){					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->OutKind != 97 ) &&					// フラップ上昇、ロック閉タイマ内出庫
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// 復決済ではない
// MH321800(E) D.Inaba ICクレジット対応
					if( Kind & 0x01 ){								// 精算完了の指定あり?
						flg_tgt = TRUE;								// 指定された個別精算ログ取出しOK
				}
			}
			if( flg_tgt == TRUE ){									// 指定されたログ?
				if( index_knd == Index ){							// 指定された番号?
					flg_end = ON;									// 検索終了(Read成功)
					if(pNextLog) {
						pNextLog->nextIndex = index_tgt;
						pNextLog->nextCount = index_knd;
					}
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// 指定された個別精算ログ総件数検索継続?
						index_knd--;								// 次の指定された個別精算ログ検索
					}else{
						flg_end = ON;								// 検索終了(Read失敗)
					}
				}
			}
			if( flg_end == OFF ){									// 検索終了?
				if( index_tgt ){									// 個別精算ログ総件数検索継続?
					index_tgt--;									// 次の個別精算ログ検索
				}else{
					flg_end = ON;									// 検索終了(Read失敗)
				}
			}
		}else{
			flg_end = ON;											// 個別精算ログ取り出しNG
		}
	}

	return flg_tgt;
}

/*[]----------------------------------------------------------------------[]*/
/*|  受付券再発行－設定画面                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set( area )                                     |*/
/*| PARAMETER    : char area  : 区画                                       |*/
/*|              : short kind : 機能種別(未使用)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lockuke_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	short	max;
	short	i, j;
	uchar	type;

	/* ロック装置情報(LockInfo[])からarea区画であるデータを抽出する */
	for (i = 0, j = 0; i < LOCK_MAX; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0 && 
			(FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4)) {
			if (area == 0) {
				lock_idx[j++] = i;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j++] = i;
				}
			}
		}
	}
	max = j-1;

	/* 該当データなし */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);						/* "　　　データがありません　　　" */
		Fun_Dsp(FUNMSG[8]);																		/* "　　　　　　　　　　　　 終了 " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:終了 */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	top = no = 0;
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				/* "　【車室番号】　　　【状態】　" */
	lockuke_set_all_dsp(top, max, no, lock_idx);									/* データ一括表示 */
	Fun_Dsp( FUNMSG[72] );															/* "  ＋  －／読       発行  終了 " */

	input = -1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
			return 1;
		case KEY_TEN_F5:
			BUZPI();
			LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
			return 0;
		case KEY_TEN_F1:				/* F1:▲ */
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* データ一括表示 */
			lockuke_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:				/* F2:▼／読 */
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
						break;
					}
				}
				if (i > max) {
					/* 入力した車室番号が見つからない場合 */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* データ一括表示 */
			lockuke_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F4:

			if((( FLAPDT.flp_data[lock_idx[no]].uketuke == 0 )&&				/* 発行済みフラグOFF? */
				( prm_get( COM_PRM,S_TYP,62,1,1 ) == 1 )) ||					/* 未発行の発行はなし */
				( prm_get(COM_PRM, S_TYP, 62, 1, 3) == 0 )){ 					/* 発行上限が０枚 */
				// 未発行の車室はメンテナンスでは発行させない。
				BUZPIPI();
				break;
			}

			if( PriRctCheck() != OK ) {											// ﾚｼｰﾄ印字処理可／不可ﾁｪｯｸ
				BUZPIPI();
				break;
			}

			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			/* 受付券発行処理 */
			type = 1;
			if( FLAPDT.flp_data[lock_idx[no]].uketuke == 0 ){					/* 未発行 */
				type = 2;
			}
			else																/* 1枚以上発行済み */
			{
				FLAPDT.flp_data[lock_idx[no]].issue_cnt--;						/* 発行枚数デクリメント(印字前にインクリメントする為) */
			}

			uke_isu((ulong)(LockInfo[lock_idx[no]].area * 10000L + LockInfo[lock_idx[no]].posi), (ushort)(lock_idx[no] + 1), type );	/* 受付券発行要求（再発行） */
			Ope_DisableDoorKnobChime();
			OpelogNo = OPLOG_UKETUKESAIHAKKO;		// 操作履歴登録
			/* 終了を待ち合わせる */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
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
					LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if ( msg == LCD_DISCONNECT ) {
					Lagcan(OPETCBNO, 6);
					LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				
			}
			lockuke_set_sts_dsp(top, no, &FLAPDT.flp_data[lock_idx[no]]);
			input = -1;
			break;
		case KEY_TEN:
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  入出庫印字画面                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lock_io_set( area )                                     |*/
/*| PARAMETER    : char area  : 区画                                       |*/
/*|              : short kind : 機能種別(未使用)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lock_io_set(char area, short kind)
{
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
//	short	top;
//	short	no;
//	short	max;
//	short	i, j;
//	short	start, end;
//	ushort	msg;
//	long	input;
//	char	clr_flg = 0;
//	/* ロック装置情報(LockInfo[])からarea区画であるデータを抽出する */
//	if (gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ) {
//		no = (char)GetCarInfoParam();
//		switch( no&0x06 ){
//			case	0x04:
//				start = INT_CAR_START_INDEX;
//				end   = start + INT_CAR_LOCK_MAX;
//				break;
//			case	0x06:
//				start = CAR_START_INDEX;
//				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
//				break;
//			case	0x02:
//			default:
//				start = CAR_START_INDEX;
//				end   = start + CAR_LOCK_MAX;
//				break;
//		}
//	}
//	else {
//		start = BIKE_START_INDEX;
//		end   = start + BIKE_LOCK_MAX;
//	}
//	
//	for (i = start, j = 0; i < end; i++) {
//		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
//		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
//			if (area == 0) {
//				lock_idx[j++] = i;
//			}
//			else {
//				if (LockInfo[i].area == area) {
//					lock_idx[j++] = i;
//				}
//			}
//		}
//	}
//	max = j-1;
//
//	/* 該当データなし */
//	if (max == -1) {
//		for (i = 0; i < 6; i++) {
//			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
//		}
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);						/* "　　　データがありません　　　" */
//		Fun_Dsp(FUNMSG[8]);																		/* "　　　　　　　　　　　　 終了 " */
//		for( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){					/* FunctionKey Enter */
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			case KEY_MODECHG:
//				BUZPI();
//				return 1;
//			case KEY_TEN_F5:				/* F5:終了 */
//				BUZPI();
//				return 0;
//			default:
//				break;
//			}
//		}
//	}
//
//	top = no = 0;
//	input = -1;
//	Fun_Dsp( FUNMSG[105] );															/* "　＋　－／読クリア 読出  終了 " */
//	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "　【車室番号】　　　　　　　　" */
//	lockio_set_all_dsp(top, max, no, lock_idx);										/* データ一括表示 */
//
//	for( ; ; ){
//		msg = StoF( GetMessage(), 1 );
//		switch( KEY_TEN0to9( msg ) ){
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		case KEY_MODECHG:
//			BUZPI();
//			LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
//			return 1;
//		case KEY_TEN_F5:
//			BUZPI();
//			LedReq(CN_TRAYLED, LED_OFF);	/* ｺｲﾝ取出し口ｶﾞｲﾄﾞLED OFF */
//			return 0;
//		case KEY_TEN_F1:				/* F1:▲ */
//			BUZPI();
//			no--;
//			if (no < 0) {
//				no = max;
//				top = (short)(max/5*5);
//			}
//			else if (no < top) {
//				top -= 5;
//			}
//			/* データ一括表示 */
//			lockio_set_all_dsp(top, max, no, lock_idx);
//			input = -1;
//			break;
//		case KEY_TEN_F2:				/* F2:▼ */
//			if (input == -1) {
//				BUZPI();
//				no++;
//				if (no > max) {
//					top = no = 0;
//				}
//				else if (no > top+4) {
//					top = no;
//				}
//			}
//			else {
//				for (i = 0; i <= max; i++) {
//					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
//						break;
//					}
//				}
//				if (i > max) {
//					/* 入力した車室番号が見つからない場合 */
//					BUZPIPI();
//					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
//					input = -1;
//					break;
//				}
//				BUZPI();
//				no = i;
//				top = (short)(no/5*5);
//			}
//			/* データ一括表示 */
//			lockio_set_all_dsp(top, max, no, lock_idx);
//			input = -1;
//			break;
//		case KEY_TEN_F4:
//			BUZPI();
//			msg = UsMnt_IoLog((ushort)LockInfo[lock_idx[no]].posi);
//			dispmlclr(1,7);																	/* 画面表示初期化 */
//			Fun_Dsp( FUNMSG[105] );															/* "　＋　－／読クリア 読出  終了 " */
//			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "　【車室番号】　　　　　　　　" */
//			lockio_set_all_dsp(top, max, no, lock_idx);										/* データ一括表示 */
//			break;
//		case KEY_TEN:
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 10000;
//			area_posi_dsp(top, no, area, (ulong)input, 1);
//			break;
//		case KEY_TEN_CL:
//			BUZPI();
//			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
//			input = -1;
//			break;
//		case KEY_TEN_F3:
//			BUZPI();
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[61]);				// "　全てのログをクリアしますか？", // [61]
//			Fun_Dsp(FUNMSG[19]);							// "　　　　　　 はい いいえ　　　"
//
//			for( clr_flg = 0;  clr_flg == 0; ){
//				msg = StoF( GetMessage(), 1 );			// イベント待ち
//
//				switch( msg ){							// イベント？
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//					case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//					case KEY_MODECHG:						// 設定キー切替
//						clr_flg = 1;
//						break;
//
//					case KEY_TEN_F3:						// Ｆ３（はい）キー押下
//						LogDataClr(LOG_IOLOG);
//						// no break
//					case KEY_TEN_F4:						// Ｆ４（いいえ）キー押下
//						BUZPI();
//						clr_flg = 1;
//						break;
//					default:
//						break;
//				}
//				if(clr_flg == 1){
//					dispmlclr(1,7);																	/* 画面表示初期化 */
//					Fun_Dsp( FUNMSG[105] );															/* "　＋　－／読　　　 読出  終了 " */
//					grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "　【車室番号】　　　　　　　　" */
//					lockio_set_all_dsp(top, max, no, lock_idx);										/* データ一括表示 */
//				}
//			}
//			break;
//		default:
//			break;
//		}
//
//		/* モードチェンジ */
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
////		if (msg == KEY_MODECHG) {
//		if (msg == KEY_MODECHG || msg == LCD_DISCONNECT) {	
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			return 1;
//		}
//	}
	return 0;
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
}

// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
///*[]----------------------------------------------------------------------[]*/
///*|  入出庫情報画面表示－全データ表示(5件まで)                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : lockio_set_all_dsp( top, max, rev_no, *idx )           |*/
///*| PARAMETER    : short top    : 先頭番号                                 |*/
///*|              : short max    : 最大番号                                 |*/
///*|              : short rev_no : 反転番号                                 |*/
///*|              : short *idx   : ロック装置情報の配列番号テーブル         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// static void	lockio_set_all_dsp(short top, short max, short rev_no, short *idx)
// {
// 	char	i;
//
// 	for (i = 0; i < 5; i++) {
// 		if (top+i > max) {
// 			break;
// 		}
// 		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
// 	}
// 	for ( ; i < 5; i++) {
// 		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
// 	}
// }
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)

/*[]----------------------------------------------------------------------[]*/
/*|  受付券再発行－状態表示                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set_sts_dsp( top, no, bit )                     |*/
/*| PARAMETER    : short top   : 先頭番号                                  |*/
/*|              : short no    : 表示番号                                  |*/
/*|              : BITS  bit   : ロック情報                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockuke_set_sts_dsp(short top, short no, flp_com *pflp)
{
	if (pflp->uketuke) {
		grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_1[1]);		/* "発行済" */
	} else {
		grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_1[0]);		/* "未発行" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  受付券再発行－全データ表示(5件まで)                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : 先頭番号                                 |*/
/*|              : short max    : 最大番号                                 |*/
/*|              : short rev_no : 反転番号                                 |*/
/*|              : short *idx   : ロック装置情報の配列番号テーブル         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockuke_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		lockuke_set_sts_dsp(top, (short)(top+i), &FLAPDT.flp_data[idx[top+i]]);
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* 行クリア */
	}
}

#define PARK_MAX	4

typedef struct {
	char no;		// 駐車場番号インデックス
	char mno;		// マスター駐車番号インデックス
} s_ParkNo;

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//unsigned short	UsMnt_PassInv(void)
//{
//	s_ParkNo	index[PARK_MAX];
//	ushort		msg;
//	char		ret;
//	char		max_block;
//	char		i, j;
//	const uchar	*str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	char		dsp;
//	int			pos;
//	
//	/* 有効駐車場番号を取得 */
//	max_block = 0;
//	memset(index, 0, (PARK_MAX * 2));
//	for (i = 0; i < PARK_MAX; i++) {
//		pos = (i == 0) ? 6 : i;
//		if (prm_get( COM_PRM,S_SYS,70,1,pos ) && (CPrmSS[S_SYS][i+1] != 0) ) {
//			for (j = 0; j < PARK_MAX; j++) {
//				if (CPrmSS[S_SYS][i+1] == CPrmSS[S_SYS][65+j]) {
//					index[max_block].no  = i;
//					index[max_block].mno = j;
//					max_block++;
//					break;
//				}
//			}
//		}
//	}
//
//	if (max_block == 0) {
//		dispclr();
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);	/* "＜定期有効／無効＞　　　　　　" */
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	/* "　　　データはありません　　　" */
//		Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//		
//		for ( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:終了 */
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//	else {
//		pos = 0;
//		dsp = 1;
//		for( ; ; ) {
//			if (dsp) {
//				/* 初期画面表示 */
//				dispclr();
//				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);	/* "＜定期有効／無効＞　　　　　　" */
//				for (i = 0; i < max_block; i++) {
//					grachr((ushort)(2+i), 4, 6, ((pos==i)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[i].no]);
//				}
//				if(max_block >= 2){
//					Fun_Dsp( FUNMSG2[13] );				/* "　▲　　▼　ﾌﾟﾘﾝﾄ  読出  終了 " */				
//				}
//				else{
//					Fun_Dsp( FUNMSG2[49] );				/* "　　　　　　ﾌﾟﾘﾝﾄ  読出  終了 " */				
//				}
//				dsp = 0;
//			}
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:終了 */
//				BUZPI();
//				return MOD_EXT;
//			case KEY_TEN_F1:			/* F1:▲ */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (--pos < 0) {
//						pos = max_block - 1;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F2:			/* F2:▼ */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (++pos >= max_block) {
//						pos = 0;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F3:			/* F3:ﾌﾟﾘﾝﾄ */
//				BUZPI();
//				/* プリント画面へ */
//				ret = passinv_print(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			case KEY_TEN_F4:			/* F4:読出 */
//				BUZPI();
//				/* 設定画面へ */
//				ret = passinv_set(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期有効／無効ー設定画面                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_set( block )                                    |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//#define	PASINV(b, o)	(pas_tbl[((b)+(o)-1)].BIT.INV)
//static char	passinv_set(char block, char mode)
//{
//	ushort	msg;
//	long	no;
//	long	max;
//	long	top;
//	char	ret;
//	long	begin;
//	long	end;
//	char	i;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	const uchar	*str_tbl[] = {DAT2_6[11], DAT2_6[12]};
//	long	input;
//	char	dsp;
//	PAS_TBL pass_data;
//
//	if (block == 0) {
//		begin = 1;
//		end = (CPrmSS[S_SYS][61] == 0) ? 12000 : CPrmSS[S_SYS][61]-1;
//	}
//	else if (block == 1) {
//		begin = CPrmSS[S_SYS][61];
//		end = (CPrmSS[S_SYS][62] == 0) ? 12000 : CPrmSS[S_SYS][62]-1;
//	}
//	else if (block == 2) {
//		begin = CPrmSS[S_SYS][62];
//		end = (CPrmSS[S_SYS][63] == 0) ? 12000 : CPrmSS[S_SYS][63]-1;
//	}
//	else {	/* if (block == 3) */
//		begin = CPrmSS[S_SYS][63];
//		end = 12000;
//	}
//	max = end-begin;
//
//	no = top = 0;
//	input = -1;
//	dsp = 1;
//	for( ; ; ) {
//		if (dsp) {
//			/* 初期画面表示 */
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);			/* "＜定期有効／無効＞　　　　　　" */
//			grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);	/* "　　　　　　　　　　　　ＸＸＸ" */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//				grachr((ushort)(1+i), 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);						/* "－" */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);	/* "有効"or"無効" */
//			}
//			Fun_Dsp(FUNMSG[17]);					/* "  ＋  －／読 変更 全変更 終了 " */
//			dsp = 0;
//			pass_data = pas_tbl[((begin)+(no)-1)];
//		}
//
//		msg = StoF( GetMessage(), 1 );
//		switch (KEY_TEN0to9(msg)) {
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:終了 */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:＋ */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			no--;
//			if (no < 0) {
//				if (max > 5) {
//					top = max-5;
//				} else {
//					top = 0;
//				}
//				no = max;
//			}
//			else if (no < top) {
//				top--;
//			}
//			/* 表示 */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);	/* "有効"or"無効" */
//			}
//			pass_data = pas_tbl[((begin)+(no)-1)];
//			input = -1;
//			break;
//		case KEY_TEN_F2:		/* F2:－／読 */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			if (input == -1) {
//				BUZPI();
//				no++;
//				if (no > max) {
//					no = top = 0;
//				} else if (no > top+5) {
//					top++;
//				}
//			}
//			else {
//				if (input == 0 || input > max+1) {
//					BUZPIPI();
//				}
//				else {
//					BUZPI();
//					top = no = input-1;
//					if (top + 5 > max) {
//						if (max > 5) {
//							top = max-5;
//						} else {
//							top = 0;
//						}
//					}
//				}
//			}
//			/* 表示 */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);		/* "有効"or"無効" */
//			}
//			pass_data = pas_tbl[((begin)+(no)-1)];
//			input = -1;
//			break;
//		case KEY_TEN_F3:		/* F3:変更 */
//			BUZPI();
//			PASINV(begin, no) ^= 1;
//			OpelogNo = OPLOG_TEIKIYUKOMUKO;		// 操作履歴登録
//			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);				/* 番号 */
//			grachr((ushort)(1+no-top), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin, no)]);	/* "有効"or"無効" */
//			input = -1;
//			break;
//		case KEY_TEN_F4:		/* F4:全変更 */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			ret = passinv_allset( mode, block );
//			if (ret == 1) {
//				return 1;
//			}
//			dsp = 1;
//			input = -1;
//			break;
//		case KEY_TEN:			/* 数字(テンキー) */
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 100000L;
//			opedpl((ushort)(1+no-top), 2, (ulong)input, 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//			break;
//		case KEY_TEN_CL:		/* 取消(テンキー) */
//			BUZPI();
//			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//			input = -1;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期有効／無効ー全設定画面                                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_allset( block )                                 |*/
///*| PARAMETER    : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*|                char block : block number                               |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passinv_allset( char mode, char block )
//{
//	ushort	msg;
//	char	pos;	/* 0:有効 1:無効 */
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);		/* "＜定期有効／無効＞　　　　　　" */
//	grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);/* "　　　　　　　　　　　　ＸＸＸ" */
//	grachr(2, 12, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);		/* "　　　　　　有効　　　　　　　" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[38]);		/* "　全契約Noを　　にしますか？　" */
//	grachr(4, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);		/* "　　　　　　無効　　　　　　　" */
//	Fun_Dsp(FUNMSG[18]);													/* "　▲　　▼　 はい いいえ　　　" */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F1:		/* F1:▲ */
//		case KEY_TEN_F2:		/* F2:▼ */
//			BUZPI();
//			pos ^= 1;
//			grachr(2, 12, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);	/* "　　　　　　有効　　　　　　　" */
//			grachr(4, 12, 4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);	/* "　　　　　　無効　　　　　　　" */
//			break;
//		case KEY_TEN_F3:		/* F3:はい */
//			BUZPI();
//			/* 全設定変更 */
//			if( pos == 0 ){
//				wopelg( OPLOG_TEIKIZENYUKO, 0, 0 );		// 操作履歴登録
//				// 全有効
//				FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0, 0xFFFFFFFF, 0, 0 );
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//									, 0
//									, (ulong)(CPrmSS[S_SYS][65 + block])
//									, 0
//									, NTNET_PASSUPDATE_ALL_INITIAL
//									, 0
//									, NULL );
//			}else{
//				wopelg( OPLOG_TEIKIZENMUKO, 0, 0 );		// 操作履歴登録
//				// 全無効
//				FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0xFFFF, 0xFFFFFFFF, 1, 0 );
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//									, 0
//									, (ulong)(CPrmSS[S_SYS][65 + block])
//									, 0
//									, NTNET_PASSUPDATE_ALL_INVALID
//									, 0
//									, NULL );
//			}
//			return 0;
//		case KEY_TEN_F4:		/* F4:いいえ */
//			BUZPI();
//			return 0;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期有効／無効ープリント画面                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_print( block )                                  |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passinv_print(char block, char mode)
//{
//	ushort			msg;
//	int				pos;		/* 0:有効のみ 1:無効のみ 2:全て */
//	T_FrmTeikiData1	pri_data;
//	uchar			priend;
//	char			ret;
//	T_FrmPrnStop	FrmPrnStop;
//
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37] );		/* "＜定期有効／無効＞　　　　　　" */
//	passinv_print_dsp(0, 1);												/* "　　有効のみプリント　　　　　" */
//	passinv_print_dsp(1, 0);												/* "　　無効のみプリント　　　　　" */
//	passinv_print_dsp(2, 0);												/* "　　全てプリント　　　　　　　" */
//	Fun_Dsp(FUNMSG[68]);													/* "　▲　　▼　　　　 実行  終了 " */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:終了 */
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:▲ */
//		case KEY_TEN_F2:		/* F2:▼ */
//			BUZPI();
//			passinv_print_dsp(pos, 0);
//			if (msg == KEY_TEN_F1) {
//				if (--pos < 0) {
//					pos = 2;
//				}
//			}
//			else {
//				if (++pos > 2) {
//					pos = 0;
//				}
//			}
//			passinv_print_dsp(pos, 1);
//			break;
//		case KEY_TEN_F4:		/* F4:実行 */
//			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
//				BUZPIPI();
//				break;
//			}
//			BUZPI();
//			/* プリント実行 */
//			pri_data.prn_kind = R_PRI;
//			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			pri_data.Kakari_no = OPECTL.Kakari_Num;
//			pri_data.Pkno_syu = (uchar)mode;
//			if (pos == 0) {			/* 有効のみ */
//				pri_data.Req_syu = 1;
//			} else if (pos == 1) {	/* 無効のみ */
//				pri_data.Req_syu = 2;
//			} else {				/* 全て */
//				pri_data.Req_syu = 0;
//			}
//			queset(PRNTCBNO, PREQ_TEIKI_DATA1, sizeof(T_FrmTeikiData1), &pri_data);
//			Ope_DisableDoorKnobChime();
//
//			/* プリント終了を待ち合わせる */
//
//			Fun_Dsp(FUNMSG[82]);				// Fｷｰ表示　"　　　　　　 中止 　　　　　　"
//
//			for ( priend = 0 ; priend == 0 ; ) {
//
//				msg = StoF( GetMessage(), 1 );
//
//				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
//					msg &= (~INNJI_ENDMASK);
//				}
//				switch( msg ){
//
//					case	KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
//						BUZPI();
//						ret = 1;
//						priend = 1;
//						break;
//
//					case	PREQ_TEIKI_DATA1:	// 印字終了
//						ret = 0;
//						priend = 1;
//						break;
//
//					case	KEY_TEN_F3:			// F3ｷｰ（中止）
//
//						BUZPI();
//						FrmPrnStop.prn_kind = R_PRI;
//						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
//						ret = 0;
//						priend = 1;
//						break;
//				}
//			}
//			return(ret);
//
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期有効／無効ープリント文字列表示                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_print( block )                                  |*/
///*| PARAMETER    : char pos   : 0:有効 1:無効 2:全て                       |*/
///*|              : ushort rev : 0:正転 1:反転                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passinv_print_dsp(char pos, ushort rev)
//{
//	if (pos == 0) {
//		grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);		/* "　　有効　　　　　　　　　　　" */
//		grachr(2, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "　　　　のみ　　　　　　　　　" */
//		grachr(2, 12, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　　　プリント　　　　　" */
//	}
//	else if (pos == 1) {
//		grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);		/* "　　無効　　　　　　　　　　　" */
//		grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "　　　　のみ　　　　　　　　　" */
//		grachr(3, 12, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　　　プリント　　　　　" */
//	}
//	else {
//		grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);		/* "　　全て　　　　　　　　　　　" */
//		grachr(4, 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　プリント　　　　　　　" */
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期有効／無効　データチェック                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_check( block )                                  |*/
///*| PARAMETER    : PAS_TBL before 前回値                                   |*/
///*| PARAMETER    : PAS_TBL after  今回値                                   |*/
///*| PARAMETER    : ushort index   データインデックス                       |*/
///*| PARAMETER    : ushort parking_kind   定期区分                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void passinv_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind)
//{
//	if (before.BIT.INV != after.BIT.INV) {
//		if (after.BIT.INV) {
//			// 無効
//			WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, 0xFFFF, 0xFFFFFFFF, 1 );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//							, (ulong)index
//							, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//							, after.BIT.PKN
//							, NTNET_PASSUPDATE_INVALID
//							, 0
//							, NULL );
//		} else {
//			// 有効
//			WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, 0, 0xFFFFFFFF, 0 );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//							, (ulong)index
//							, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//							, after.BIT.PKN
//							, NTNET_PASSUPDATE_INITIAL
//							, 0
//							, NULL );
//		}
//	}
//}
//
//unsigned short	UsMnt_PassEnter(void)
//{
//	s_ParkNo	index[PARK_MAX];
//	ushort		msg;
//	char		ret;
//	char		max_block;
//	char		i, j;
//	const uchar	*str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	char		dsp;
//	int			pos;
//	
//	/* 有効駐車場番号を取得 */
//	max_block = 0;
//	memset(index, 0, (PARK_MAX * 2));
//	for (i = 0; i < PARK_MAX; i++) {
//		pos = (char)((i == 0) ? 6 : i);
//		if (prm_get( COM_PRM,S_SYS,70,1,pos ) && (CPrmSS[S_SYS][i+1] != 0) ) {
//			for (j = 0; j < PARK_MAX; j++) {
//				if (CPrmSS[S_SYS][i+1] == CPrmSS[S_SYS][65+j]) {
//					index[max_block].no  = i;
//					index[max_block].mno = j;
//					max_block++;
//					break;
//				}
//			}
//		}
//	}
//
//	if (max_block == 0) {
//		dispclr();
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);	/* "＜定期入庫／出庫＞　　　　　　" */
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	/* "　　　データはありません　　　" */
//		Fun_Dsp(FUNMSG[8]);					/* "　　　　　　　　　　　　 終了 " */
//		
//		for ( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:終了 */
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//	else {
//		pos = 0;
//		dsp = 1;
//		for( ; ; ) {
//			if (dsp) {
//				/* 初期画面表示 */
//				dispclr();
//				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);	/* "＜定期入庫／出庫＞　　　　　　" */
//				for (i = 0; i < max_block; i++) {
//					grachr((ushort)(2+i), 4, 6, ((pos==i)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[i].no]);
//				}
//				if(max_block >= 2){
//					Fun_Dsp( FUNMSG2[13] );				/* "　▲　　▼　ﾌﾟﾘﾝﾄ  読出  終了 " */				
//				}
//				else{
//					Fun_Dsp( FUNMSG2[49] );				/* "　　　　　　ﾌﾟﾘﾝﾄ  読出  終了 " */				
//				}
//				dsp = 0;
//			}
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:終了 */
//				BUZPI();
//				return MOD_EXT;
//			case KEY_TEN_F1:			/* F1:▲ */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (--pos < 0) {
//						pos = (char)(max_block - 1);
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F2:			/* F2:▼ */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (++pos >= max_block) {
//						pos = 0;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F3:			/* F3:ﾌﾟﾘﾝﾄ */
//				BUZPI();
//				/* プリント画面へ */
//				ret = passent_print(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			case KEY_TEN_F4:			/* F4:読出 */
//				BUZPI();
//				/* 設定画面へ */
//				ret = passent_set(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫ー設定画面                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_set( block )                                    |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//#define	PASSTS(b, o)	(pas_tbl[((b)+(o)-1)].BIT.STS)
//static char	passent_set(char block, char mode)
//{
//#define	DSP_COUNT_MAX	6			// 最大表示件数
//
//	ushort	msg;
//	long	max;
//	char	ret;
//	long	begin;
//	long	end;
//	char	i;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	const uchar	*str_tbl[] = {DAT2_6[13], DAT2_6[15], DAT2_6[14]};
//	long	input;
//	char	dsp;					// 1=再表示（IDは変わらすステータス表示のみ更新）又は最初の表示
//									// 2=次画面表示
//									// 3=前画面表示
//									// 4=先頭IDから表示
//									// 5=末尾IDを最後に表示
//	PAS_TBL pass_data;
//	char	DspCount;				// 表示件数（0～6）
//	ushort	DspID[DSP_COUNT_MAX];	// 表示ID（1～12000）
//	ushort	DspTop;					// 先頭に表示する定期ID（1～12000：無効判断前）
//	char	CslPos;					// カーソル表示行(0～DSP_COUNT_MAX-1)
//	ushort	WkID, WkPrevID;			// work
//	WkID = WkPrevID = 0;
//
//	if (block == 0) {
//		begin = 1;
//		end = (CPrmSS[S_SYS][61] == 0) ? 12000 : CPrmSS[S_SYS][61]-1;
//	}
//	else if (block == 1) {
//		begin = CPrmSS[S_SYS][61];
//		end = (CPrmSS[S_SYS][62] == 0) ? 12000 : CPrmSS[S_SYS][62]-1;
//	}
//	else if (block == 2) {
//		begin = CPrmSS[S_SYS][62];
//		end = (CPrmSS[S_SYS][63] == 0) ? 12000 : CPrmSS[S_SYS][63]-1;
//	}
//	else {	/* if (block == 3) */
//		begin = CPrmSS[S_SYS][63];
//		end = 12000;
//	}
//	max = end-begin;
//
//	input = -1;
//	dsp = 4;			// 先頭から表示
//
//	for( ; ; ) {
//		if (dsp) {
//			/* 初期画面表示 */
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);			/* "＜定期入庫／出庫＞　　　　　　" */
//			grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);	/* "　　　　　　　　　　　　ＸＸＸ" */
//
//			// 表示要求（次頁，前頁，再表示）に従い次に先頭行に表示するID(DspTop)を求める
//			// ついでにカーソル位置(CslPos)もセットする
//
//passent_set_10:
//
//			if( 1 == dsp ){								// 再表示
//				;										// 再表示の場合は　DspTop値のまま
//				;										// カーソル位置も変えない
//			}
//			else if( 2 == dsp ){						// 次画面表示（１データ分下にずらした表示）
//				if( DSP_COUNT_MAX == DspCount ){		// 現在最大件数を表示している時のみ次ページのID検索を行う
//					WkID = UsMnt_PassEnter_AfterValidIdSerach( (ushort)begin, (ushort)end, (ushort)(DspID[DSP_COUNT_MAX-1] + 1) );
//														// +方向に有効IDを探す
//					if( 0 == WkID ){					// 有効なID無し
//						dsp = 4;						// 先頭から表示する
//						goto passent_set_10;
//					}
//					else{
//						DspTop = DspID[1];				// 2行目に表示しているIDを次の先頭とする
//						;								// カーソルは末尾行のまま
//					}
//				}
//				else{									// 最大件数表示で無い場合は
//					dsp = 4;							// 先頭から表示する
//					goto passent_set_10;
//				}
//			}
//			else if( 3 == dsp ){						// 前画面表示
//				WkID = UsMnt_PassEnter_BeforeValidIdSerach( (ushort)begin, (ushort)end, (ushort)(DspID[0] - 1) );
//														// 現在先頭に表示しているIDの1つ前の有効IDを求める
//				if( 0 == WkID ){						// 前は無い（既に有効な先頭ID）
//					dsp = 5;							// 末尾を最後に表示
//					goto passent_set_10;
//				}
//				else{									// 1つ前の有効IDあり
//					DspTop = WkID;						// 次先頭表示IDセット
//				}
//				CslPos = 0;								// カーソルは先頭行
//			}
//			else if( 4 == dsp ){						// 先頭から表示。（最初と最終IDから次キー押下時）
//				DspTop = (ushort)begin;
//				CslPos = 0;								// カーソルは先頭行
//			}
//			else if( 5 == dsp ){						// 末尾を最後に表示。（先頭IDから前キー押下時）
//				WkID = (ushort)end;
//				for( i=0; i<DSP_COUNT_MAX; ++i ){		// 末尾IDから表示MAX件数分の有効IDを探す
//					WkID = UsMnt_PassEnter_BeforeValidIdSerach( (ushort)begin, (ushort)end, WkID );
//					if( 0 == WkID ){					// 前に有効なID無し
//						break;
//					}
//					WkPrevID = WkID;					// 今のところ有効なID（若番で）
//					--WkID;
//				}
//				DspTop = WkPrevID;
//				CslPos = (char)(i - 1);					// カーソルは末尾行
//			}
//			dsp = 0;
//
//			// 表示先頭IDから表示するID（最大 DSP_COUNT_MAX分）を DspID[] にセットする
//			WkID = DspTop;
//			for( i=0; i<DSP_COUNT_MAX; ++i ){			// 表示MAX件数分
//				WkID = UsMnt_PassEnter_AfterValidIdSerach( (ushort)begin, (ushort)end, WkID );
//														// +方向に有効IDを探す
//				if( 0 == WkID ){						// 有効なID無し
//					break;
//				}
//				else{									// 有効IDあり
//					DspID[i] = WkID;					// 表示テーブルにセット
//					++WkID;								// 次検索開始IDセット
//				}
//			}
//			DspCount = i;								// 表示件数セット
//
//			// 表示
//			for (i = 0; i < DspCount; i++) {
//				opedpl((ushort)(1+i), 2, (ulong)(DspID[i]-begin+1), 5, 1, (i==CslPos)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//				grachr((ushort)(1+i), 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);										/* "－" */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[ pas_tbl[ DspID[i]-1 ].BIT.STS ]);		/* "初期"or"出庫"or"入庫" */
//			}
//
//			Fun_Dsp(FUNMSG[17]);						/* "  ＋  －／読 変更 全変更 終了 " */
//			dsp = 0;
//			if( 0 == DspCount ){						// 1件も表示していない
//				pass_data.BYTE = 0;
//			}
//			else{
//				pass_data = pas_tbl[ DspID[CslPos]-1 ];
//			}
//		}
//
//		msg = StoF( GetMessage(), 1 );
//		switch (KEY_TEN0to9(msg)) {
//// MH810100(S)
//		case LCD_DISCONNECT:
//			return 2;
//// MH810100(E)
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:終了 */
//			if( 0 != DspCount ){						// 表示データあり
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:＋ */
//			if( 0 != DspCount ){						// 表示データあり
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//			BUZPI();
//			if( 0 == CslPos ){							// 最上行にカーソルあり
//				dsp = 3;								// 前頁表示指示
//														// 末尾への移動判定も上に任せる
//			}
//			else{
//				// 番号表示のカーソルのみ移動
//				opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号カーソル消し */
//				--CslPos;
//				opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号カーソル点け */
//
//				input = -1;								// Jump登録消し
//				pass_data = pas_tbl[ DspID[CslPos]-1 ];	// 変更前ステータス保存
//			}
//// MH322914 (s) kasiyama 2016/07/07 最下段で数値入力->「+」キー後「-/読」で数値が残っているためｼﾞｬﾝﾌﾟしてしまう(共通改善No.993)(MH341106)
//			input = -1;
//// MH322914 (e) kasiyama 2016/07/07 最下段で数値入力->「+」キー後「-/読」で数値が残っているためｼﾞｬﾝﾌﾟしてしまう(共通改善No.993)(MH341106)
//			break;
//		case KEY_TEN_F2:		/* F2:－／読 */
//			if( 0 != DspCount ){						// 表示データあり
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//
//			if (input == -1) {
//				BUZPI();
//				if( (DspCount-1) <= CslPos ){				// 最下行にカーソルあり
//						dsp = 2;							// 次ページ表示指示
//															// 先頭ページからの表示判定も上に任せる
//				}
//				else{
//					// 番号表示のカーソルのみ移動
//					opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	/* 番号カーソル消し */
//					++CslPos;
//					opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 番号カーソル点け */
//
//					input = -1;								// Jump登録消し
//					pass_data = pas_tbl[ DspID[CslPos]-1 ];	// 変更前ステータス保存
//				}
//			}
//			else {
//				if (input == 0 || input > max+1) {
//					BUZPIPI();
//				}
//				else if( pas_tbl[ begin+input-2 ].BIT.INV ){	// 無効定期
//					BUZPIPI();
//				}
//				else {
//					BUZPI();
//					DspTop = (ushort)(begin+input-1);		// 表示先頭ID更新
//					dsp = 1;								// 再表示指示
//					CslPos = 0;								// カーソルは先頭
//				}
//			}
//			input = -1;
//			break;
//		case KEY_TEN_F3:		/* F3:変更 */
//			if( 0 == DspCount ){						// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//			BUZPI();
//			if( pas_tbl[ DspID[CslPos]-1 ].BIT.STS >= 2 ){
//				pas_tbl[ DspID[CslPos]-1 ].BIT.STS = 0;
//			}
//			else{
//				++(pas_tbl[ DspID[CslPos]-1 ].BIT.STS);
//			}
//			grachr((ushort)(1+CslPos), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[ pas_tbl[ DspID[CslPos]-1 ].BIT.STS ]);	/* "初期"or"出庫"or"入庫" */
//
//			OpelogNo = OPLOG_TEIKIINOUT;		// 操作履歴登録
//			input = -1;
//			break;
//		case KEY_TEN_F4:		/* F4:全変更 */
//			if( 0 != DspCount ){						// 表示データあり
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//			BUZPI();
//			ret = passent_allset( mode, block );
//			if (ret == 1) {
//				return 1;
//			}
//			dsp = 1;
//			input = -1;
//			break;
//		case KEY_TEN:			/* 数字(テンキー) */
//			if( 0 == DspCount ){						// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 100000L;
//			opedpl((ushort)(1+CslPos), 2, (ulong)input, 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//			break;
//		case KEY_TEN_CL:		/* 取消(テンキー) */
//			if( 0 == DspCount ){						// 表示データなし
//				break;									// キークリック音も鳴らさない
//			}
//			BUZPI();
//			opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
//			input = -1;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫－次有効定期ID取得                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassEnter_BeforeValidIdSerach()                   |*/
///*| PARAMETER    : TopID    = 駐車場内の先頭ID (1～12000)                  |*/
///*|				   BottomID = 駐車場内の最終ID (1～12000)                  |*/
///*|				   TargetID = 基準ID (1～12000)							   |*/
///*|							  基準IDより前方向で次に有効なIDを探す。	   |*/
///*|							　（基準IDも検索対象です）					   |*/
///*| RETURN VALUE : 0 = データ無し                                          |*/
///*|              : 1～12000 = 次の有効ID                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
//ushort	UsMnt_PassEnter_BeforeValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID )
//{
//	ushort ret=0;
//
//	if( (TargetID < TopID) || (BottomID < TargetID) ){	// 駐車場内定期の範囲外
//		return	ret;									// データ無し
//	}
//
//	for( ; TopID <= TargetID; --TargetID ){				// 駐車場内の使用定期ID分(1～12000)
//		if( pas_tbl[ TargetID-1 ].BIT.INV ){			// 無効定期
//			continue;
//		}
//		else{											// 有効定期
//			ret = TargetID;
//			break;
//		}
//	}
//	return	ret;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫－次有効定期ID取得                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassEnter_AfterValidIdSerach()                    |*/
///*| PARAMETER    : TopID    = 駐車場内の先頭ID (1～12000)                  |*/
///*|				   BottomID = 駐車場内の最終ID (1～12000)                  |*/
///*|				   TargetID = 基準ID (0～12000)							   |*/
///*|							  基準IDより後方向で次に有効なIDを探す		   |*/
///*|							　（基準IDも検索対象です）					   |*/
///*| RETURN VALUE : 0 = データ無し                                          |*/
///*|              : 1～12000 = 次の有効ID                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
//ushort	UsMnt_PassEnter_AfterValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID )
//{
//	ushort ret=0;
//
//	if( (TargetID < TopID) || (BottomID < TargetID) ){	// 駐車場内定期の範囲外
//		return	ret;									// データ無し
//	}
//
//	for( ; TargetID <= BottomID; ++TargetID ){			// 駐車場内の使用定期ID分(1～12000)
//		if( pas_tbl[ TargetID-1 ].BIT.INV ){			// 無効定期
//			continue;
//		}
//		else{											// 有効定期
//			ret = TargetID;
//			break;
//		}
//	}
//	return	ret;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫ー全設定画面                                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_allset( mode, block )                           |*/
///*| PARAMETER    : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*|                char block : block number                               |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passent_allset( char mode, char block )
//{
//	ushort	msg;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);		/* "＜定期入庫／出庫＞　　　　　　" */
//	grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);/* "　　　　　　　　　　　　ＸＸＸ" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[39]);		/* "全契約Noを初期状態にしますか？" */
//	Fun_Dsp(FUNMSG[19]);													/* "　　　　　　 はい いいえ　　　" */
//
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F3:		/* F3:はい */
//			BUZPI();
//			/* 全設定変更 */
//			FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0, 0xFFFFFFFF, 0, PASS_DATA_FILL_VALID );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//								, 0
//								, (ulong)(CPrmSS[S_SYS][65 + block])
//								, 0
//								, NTNET_PASSUPDATE_ALL_INITIAL2
//								, 0
//								, NULL );
//			wopelg( OPLOG_TEIKIZENSHOKI, 0, 0 );		// 操作履歴登録
//			return 0;
//		case KEY_TEN_F4:		/* F4:いいえ */
//			BUZPI();
//			return 0;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫ープリント画面                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_print( block )                                  |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:基本 1:拡張1 2:拡張2 3:拡張3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passent_print(char block, char mode)
//{
//	ushort			msg;
//	int				pos;		/* 0:入庫のみ 1:出庫のみ 2:全て */
//	T_FrmTeikiData2	pri_data;
//	uchar			priend;
//	char			ret;
//	T_FrmPrnStop	FrmPrnStop;
//
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36] );		/* "＜定期入庫／出庫＞　　　　　　" */
//	passent_print_dsp(0, 1);						/* "　　入庫中のみプリント　　　　" */
//	passent_print_dsp(1, 0);				/* "　　出庫中のみプリント　　　　" */
//	passent_print_dsp(2, 0);				/* "　　全てプリント　　　　　　　" */
//	Fun_Dsp(FUNMSG[68]);					/* "　▲　　▼　　　　 実行  終了 " */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:終了 */
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:▲ */
//		case KEY_TEN_F2:		/* F2:▼ */
//			BUZPI();
//			passent_print_dsp(pos, 0);
//			if (msg == KEY_TEN_F1) {
//				if (--pos < 0)
//					pos = 2;
//			}
//			else {
//				if (++pos > 2)
//					pos = 0;
//			}
//			passent_print_dsp(pos, 1);
//			break;
//		case KEY_TEN_F4:		/* F4:実行 */
//			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
//				BUZPIPI();
//				break;
//			}
//			BUZPI();
//			/* プリント実行 */
//			pri_data.prn_kind = R_PRI;
//			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			pri_data.Kakari_no = OPECTL.Kakari_Num;
//			pri_data.Pkno_syu = (uchar)mode;
//			if (pos == 0)		/* 入庫のみ */
//				pri_data.Req_syu = 1;
//			else if (pos == 1)	/* 出庫のみ */
//				pri_data.Req_syu = 2;
//			else				/* 全て */
//				pri_data.Req_syu = 0;
//			queset(PRNTCBNO, PREQ_TEIKI_DATA2, sizeof(T_FrmTeikiData2), &pri_data);
//			Ope_DisableDoorKnobChime();
//
//			/* プリント終了を待ち合わせる */
//			Fun_Dsp(FUNMSG[82]);				// Fｷｰ表示　"　　　　　　 中止 　　　　　　"
//
//			for ( priend = 0 ; priend == 0 ; ) {
//
//				msg = StoF( GetMessage(), 1 );
//
//				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
//					msg &= (~INNJI_ENDMASK);
//				}
//				switch( msg ){
//
//					case	KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
//						BUZPI();
//						ret = 1;
//						priend = 1;
//						break;
//
//					case	PREQ_TEIKI_DATA2:	// 印字終了
//						ret = 0;
//						priend = 1;
//						break;
//
//					case	KEY_TEN_F3:			// F3ｷｰ（中止）
//
//						BUZPI();
//						FrmPrnStop.prn_kind = R_PRI;
//						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
//						ret = 0;
//						priend = 1;
//						break;
//				}
//			}
//			return(ret);
//
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫ープリント文字列表示                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_print( block )                                  |*/
///*| PARAMETER    : char pos   : 0:入庫 1:出庫 2:全て                       |*/
///*|              : ushort rev : 0:正転 1:反転                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passent_print_dsp(char pos, ushort rev)
//{
//	if (pos == 0) {
//		grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);		/* "　　入庫　　　　　　　　　　　" */
//		grachr(2, 8, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "　　　　中　　　　　　　　　　" */
//		grachr(2, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "　　　　　のみ　　　　　　　　" */
//		grachr(2, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　　　　プリント　　　　" */
//	}
//	else if (pos == 1) {
//		grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);		/* "　　出庫　　　　　　　　　　　" */
//		grachr(3, 8, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "　　　　中　　　　　　　　　　" */
//		grachr(3, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "　　　　　のみ　　　　　　　　" */
//		grachr(3, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　　　　プリント　　　　" */
//	}
//	else {
//		grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);		/* "　　全て　　　　　　　　　　　" */
//		grachr(4, 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "　　　　プリント　　　　　　　" */
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期入庫／出庫　データチェック                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_check( block )                                  |*/
///*| PARAMETER    : PAS_TBL before 前回値                                   |*/
///*| PARAMETER    : PAS_TBL after  今回値                                   |*/
///*| PARAMETER    : ushort index   データインデックス                       |*/
///*| PARAMETER    : ushort parking_kind   定期区分                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passent_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind)
//{
//	if (before.BIT.STS != after.BIT.STS) {
//		WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, (ushort)after.BIT.STS, (ulong)CPrmSS[S_SYS][1], 0 );
//		NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//						, (ulong)index
//						, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//						, after.BIT.PKN
//						, after.BIT.STS
//						, 0
//						, NULL );
//	}
//}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：定期券チェック                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassCheck( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassCheck(void)
{
	ushort	msg;
	char	pos;

	pos = (char)(PPrmSS[S_P01][3] & 1);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[40] );					/* "＜定期券チェック＞　　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[12] );					/* "　現在の設定　　　　　　　　　" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "　　　　　　：　　　　　　　　" */
	if (pos == 0)
		grachr( 2, 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );
	else
		grachr( 2, 16,  6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );
	grachr( 4, 15,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );	/* "　　　　　　　 する 　　　　　" */
	grachr( 5, 14,  6, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );		/* "　　　　　　　しない　　　　　" */
	if(prm_get(0, S_NTN, 38, 1, 2) == 2){
		Fun_Dsp( FUNMSG[87] );								/* "　▲　　▼　 ｼｽﾃﾑ  書込  終了 " */
	}else{
		Fun_Dsp( FUNMSG[20] );								/* "　▲　　▼　　　　 書込  終了 " */
	}

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
				pos ^= 1;
				grachr( 4, 15,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );	/* "　　　　　　　 する 　　　　　" */
				grachr( 5, 14,  6, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );		/* "　　　　　　　しない　　　　　" */
				break;
			case KEY_TEN_F3:					/* F3:ｼｽﾃﾑ */
				if(prm_get(0, S_NTN, 38, 1, 2) != 2){
					break;
				}
			case KEY_TEN_F4:					/* F4:書込 */
				BUZPI();
				PPrmSS[S_P01][3] = (long)pos;
				if((prm_get(0, S_NTN, 38, 1, 2) == 1) || (msg == KEY_TEN_F3)){
					NTNET_Snd_Data100();			//制御データ送信(ｱﾝﾁﾊﾟｽ)
				}
				if( pos == 0 ){
					wopelg( OPLOG_TEIKICHKSURU, 0, 0 );		// 操作履歴登録
				}else{
					wopelg( OPLOG_TEIKICHKSHINAI, 0, 0 );		// 操作履歴登録
				}
				if (pos == 0) {
					grachr( 2, 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );
					grachr( 2, 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]  );
				}
				else
					grachr( 2, 16,  6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：特別日／特別期間                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_SplDay( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_SplDay(void)
{
	ushort			msg;
	int				pos;
	T_FrmSplDay		SplDay;
	char			ret;
	char			dsp;

	pos = 0;
	dsp = 1;
	for( ; ; ) {
		if (dsp) {
			/* 初期画面表示 */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[0]);		/* "＜特別日＞　　　　　　　　　　" */
			splday_dsp(0, ((pos==0)?1:0));		/* "　特別日　　　　　　　　　　　" */
			splday_dsp(1, ((pos==1)?1:0));		/* "　特別期間　　　　　　　　　　" */
			splday_dsp(2, ((pos==2)?1:0));		/* "　ハッピーマンデー　　　　　　" */
												/* OR                               */
												/* "　特別曜日　　　　　　　　　　" */
			Fun_Dsp(FUNMSG2[13]);				/* "　▲　　▼　ﾌﾟﾘﾝﾄ  読出  終了 " */
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:		/* ▲(F1) */
		case KEY_TEN_F2:		/* ▼(F2) */
			BUZPI();
			splday_dsp(pos, 0);		/* 前回選択項目-正転 */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0){
					pos = 2;
				}
			}
			else {
				if (++pos > 2){
					pos = 0;
				}
			}
			splday_dsp(pos, 1);		/* 今回選択項目-反転 */
			break;
		case KEY_TEN_F3:		/* ﾌﾟﾘﾝﾄ(F3) */
			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
				BUZPIPI();
				break;
			}
			BUZPI();
			SplDay.Kikai_no = (uchar)CPrmSS[S_PAY][2];							// 機械№
			SplDay.Kakari_no = OPECTL.Kakari_Num;								// 係員No.
			SplDay.prn_kind = R_PRI;											// ﾌﾟﾘﾝﾀ種別
			queset(PRNTCBNO, PREQ_SPLDAY, sizeof(T_FrmSplDay), &SplDay);		// 特別日印字要求
			Ope_DisableDoorKnobChime();

			/* プリント終了を待ち合わせる */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
			for ( ; ; ) {
				msg = StoF( GetMessage(), 1 );
				/* プリント終了 */
				if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
					Lagcan(OPETCBNO, 6);
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブ閉状態であればメンテナンス終了
					if (CP_MODECHG) {
						return MOD_CHG;
					}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
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
				if ( msg == LCD_DISCONNECT ) {
					Lagcan(OPETCBNO, 6);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			}
			break;
		case KEY_TEN_F4:		/* 読出(F4) */
			BUZPI();
			OpelogNo = 0;
			splday_check();

			switch(pos){
				case 0:
					ret = splday_day();			/* "　特別日　　　　　　　　　　　" */
					break;
				case 1:
					ret = splday_period();		/* "　特別期間　　　　　　　　　　" */
					break;				
				default:
					if((prm_get( 0,S_TOK,(short)(41),1,1 )) == 1 ){
						ret = splday_week();		// 特別曜日
					}
					else{
						ret = splday_monday();		// ハッピーマンデー
					}
					break;
			}

			splday_check();

			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
				OpelogNo = 0;
				SetChange = 1;			// FLASHｾｰﾌﾞ指示
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	//FTPフラグセット
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
			}

			if (ret == 1){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			if (ret == 2){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

//--------------------------------------------------------------------------
// 入力日付のチェックを行い、月のみの入力などは削除する。
//--------------------------------------------------------------------------
static void splday_check( void )
{
	int	i;

	ulong	lData;
	ulong	sData1, sData2;
	ulong	sData3;

	// 特別日チェック
	for( i = 0; i < SP_DAY_MAX; i++ ){
		lData = CPrmSS[S_TOK][9+i];
		sData1 = lData % 100;
		sData2 = (lData % 10000) / 100;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][9+i] = 0;
		}
	}

	// 特別期間チェック
	for( i = 0; i < SP_RANGE_MAX*2; i++ ){
		lData = CPrmSS[S_TOK][1+i] % 10000;
		sData1 = lData % 100;
		sData2 = (lData % 10000) / 100;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][1+i] = 0;
		}
	}
	// 開始日・終了日が対で設定されているかチェック
	for( i = 0; i < SP_RANGE_MAX; i++ ){
		if (CPrmSS[S_TOK][i*2+1] && !CPrmSS[S_TOK][i*2+2]) {
			CPrmSS[S_TOK][i*2+1] = 0;
		}
		if (!CPrmSS[S_TOK][i*2+1] && CPrmSS[S_TOK][i*2+2]) {
			CPrmSS[S_TOK][i*2+2] = 0;
		}
	}

	for( i = 0; i < (SP_HAPPY_MAX/2); i++ ){
		lData = CPrmSS[S_TOK][42+i] / 1000;
		sData1 = lData / 10;
		sData2 = lData % 10;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][42+i] %= 1000;
		}

		lData = CPrmSS[S_TOK][42+i] % 1000;
		sData1 = lData / 10;
		sData2 = lData % 10;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][42+i] = CPrmSS[S_TOK][42+i] / 1000 * 1000;
		}
	}

	// 特別曜日チェック
	for( i = 0; i < SP_WEEK_MAX; i++ ){
		lData = CPrmSS[S_TOK][56+i];	// 04-0056～
		sData1 = ((lData % 10000 ) /100);	// ③④：月
		sData2 = ((lData % 100) / 10);		// 　⑤：週
											// 　⑥：曜日
		sData3 = lData % 10;				// 　⑥：曜日

		if (!sData1 || !sData2 || !sData3) {
			CPrmSS[S_TOK][56+i] = 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日／特別期間表示                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_dsp( pos, rev )                                  |*/
/*| PARAMETER    : char pos   : 0:特別日 1:特別期間 2:ハッピーマンデー     |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_dsp(char pos, ushort rev)
{
	switch(pos){
		case 0:
			grachr(2, 4, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);		/* "　特別日　　　　　　　　　　　" */
			break;
		case 1:
			grachr(3, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[1]);		/* "　特別期間　　　　　　　　　　" */
			break;
		default:
			if((prm_get( 0,S_TOK,(short)(41),1,1 )) == 1 ){
				grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[3]);		/* "　特別曜日　　　　　　　　　　" */
			}else{
				grachr(4, 4, 16, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[2]);		/* "　ハッピーマンデー　　　　　　" */
			}
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日／特別期間ー特別日設定                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_day( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_day(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-30 */
	long	*pSPD;
	char	dsp;
	char	mode;	/* 0:通常 1:取消 */
	char	pos;	/* 0:月 1:日 2:切替 */
	long	input;

	pSPD = &CPrmSS[S_TOK][9];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* 画面表示 */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[0]);								/* "＜特別日＞　　　　　　　　　　" */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[10]);			/* "　特別日　　：　　月　　日　　" */
				opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
				splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);									/* 月 */
				splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);									/* 日 */
				splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);									/* 振替 */
			}
			if (mode == 0) {
				grachr(1, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[17]);	/* "　　　　　　　　　　　　　振替" */
				splday_day_dsp(top, no, 0, pSPD[no], 1);							/* 月 反転 */
				Fun_Dsp( FUNMSG[39] );												/* "　＋　　－　 取消  書込  終了 " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);		/* "　取消す番号を選択して下さい　" */
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);			/* "特別日" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 反転 */
				Fun_Dsp( FUNMSG[41] );												/* "　＋　　－　 取消 全取消 終了 " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス（メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
	
		/* 通常画面 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* ＋(F1) */
				/* 振替 */
				if (pos == 2) {
					BUZPI();
					input = 1;
					splday_day_dsp(top, no, pos, input*100000L, 1);
				}
				/* 月・日 */
				else {
					if (pos == 1) {
						Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
					}
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// 日が未確定で次の行に遷移：現在行のデータをクリア
					}
					BUZPI();
					no--;
					if (no < 0) {
						top = 26;
						no = 30;
					}
					else if (no < top)
						top--;
					input = -1;
					/* 表示 */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* 月 */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* 日 */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* 振替 */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* 月 反転 */
				}
				break;
			case KEY_TEN_F2:		/* －(F2) */
				/* 振替 */
				if (pos == 2) {
					BUZPI();
					input = 0;
					splday_day_dsp(top, no, pos, input*100000L, 1);
				}
				/* 月・日 */
				else {
					if (pos == 1) {
						pos = 0;
						Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
					}
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// 日が未確定で前の行に遷移：現在行のデータをクリア
					}
					BUZPI();
					no++;
					if (no > 30)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* 表示 */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* 月 */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* 日 */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* 振替 */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* 月 反転 */
				}
				break;
			case KEY_TEN_F3:		/* 取消(F3) */
				BUZPI();
				if (pos != 0) {		// BACKｷｰ：前の項目に戻る
					if (pos == 1) {
						Fun_Dsp( FUNMSG[39] );				// "　＋　　－　 取消  書込  終了 "
					}
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos--;
					input = -1;
					splday_day_dsp(top, no, pos, pSPD[no], 1);
				}
				else {				// 取消ｷｰ：取消ﾓｰﾄﾞに遷移
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// 日が未確定で次の行に遷移：現在行のデータをクリア
					}
					pos = 0;
					mode = 1;
					dsp = 1;
					OpelogNo = OPLOG_TOKUBETUBI;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				}
				break;
			case KEY_TEN_F4:		/* 書込(F4) */
				if (pos == 0) {
					if (input == -1)
						input = GET_MONTH(pSPD[no]);
					if (input < 1 || input > 12) {
						BUZPIPI();
						input = -1;
						splday_day_dsp(top, no, pos, pSPD[no], 1);
						break;
					}
					BUZPI();
					// 月日の正当性チェック
					if (GET_DAY(pSPD[no]) > medget(2004, (short)input))
						pSPD[no] = pSPD[no]/100000L*100000L + input*100;
					else
						pSPD[no] = pSPD[no]/100000L*100000L + input*100 + pSPD[no]%100;
					OpelogNo = OPLOG_TOKUBETUBI;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos = 1;
					input = -1;
					splday_day_dsp(top, no, pos, pSPD[no], 1);
					Fun_Dsp( FUNMSG[30] );				/* "  ＋    －    ⊂   書込  終了 " */
				}
				else if (pos == 1) {
					if (input == -1)
						input = GET_DAY(pSPD[no]);
					if (input < 1 || input > medget(2004, (short)GET_MONTH(pSPD[no]))) {	/* 2004は閏年 */
						BUZPIPI();
						input = -1;
						splday_day_dsp(top, no, pos, pSPD[no], 1);
						break;
					}
					BUZPI();
					pSPD[no] = pSPD[no]/100*100 + input;
					OpelogNo = OPLOG_TOKUBETUBI;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos = 2;
					input = (short)(pSPD[no]/100000L);
					splday_day_dsp(top, no, pos, pSPD[no], 1);
				}
				else {	/* if (pos == 2) */
					BUZPI();
					pSPD[no] = input*100000L + pSPD[no]%100000L;
					OpelogNo = OPLOG_TOKUBETUBI;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					no++;
					if (no > 30)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* 表示 */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* 月 */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* 日 */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* 振替 */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* 月 反転 */
					Fun_Dsp( FUNMSG[39] );				/* "　＋　　－　 取消  書込  終了 " */
				}
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				BUZPI();
				if (pos == 2)
					break;
				if (input == -1)
					input = 0;
				input = (input*10 + msg-KEY_TEN0) % 100;
				if (pos == 0)
					splday_day_dsp(top, no, pos, input*100, 1);
				else	/* if (pos == 1) */
					splday_day_dsp(top, no, pos, input, 1);
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				if (pos == 2)
					input = (short)(pSPD[no]/100000L);
				else
					input = -1;
				splday_day_dsp(top, no, pos, pSPD[no], 1);
				break;
			default:
				break;
			}
		}
		/* 取消画面 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 取消(F3) */
				pSPD[no] = 0;
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* ＋(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = 26;
					no = 30;
				}
				else if (no < top)
					top--;
				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "特別日" */
					opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
					splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);										/* 月 */
					splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);										/* 日 */
					splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);										/* 振替 */
				}
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "特別日" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 反転 */
				break;
			case KEY_TEN_F2:		/* －(F2) */
				BUZPI();
				no++;
				if (no > 30)
					top = no = 0;
				else if (no > top+4)
					top++;
				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "特別日" */
					opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
					splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);										/* 月 */
					splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);										/* 日 */
					splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);										/* 振替 */
				}
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "特別日" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 反転 */
				break;
			case KEY_TEN_F4:		/* 全取消(F4) */
				BUZPI();
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[7]);		/* "　全ての特別日を取消しますか？" */
				grachr((ushort)(2+no-top), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "特別日" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
				Fun_Dsp( FUNMSG[19] );												/* "　　　　　　 はい いいえ　　　" */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					if (msg == KEY_MODECHG) {
						BUZPI();
						return 1;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス（メンテナンス)
					if (msg == LCD_DISCONNECT) {
						return 2;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
	
					if (msg == KEY_TEN_F3) {		/* はい(F3) */
						BUZPI();
						for (i = 0; i < SP_DAY_MAX; i++) {
							pSPD[i] = 0;
						}
						break;
					}
					else if (msg == KEY_TEN_F4) {	/* いいえ(F4) */
						BUZPI();
						break;
					}
				}
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日表示                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_day_dsp( top, no, pos, data, rev )               |*/
/*| PARAMETER    : char top   : 先頭番号                                   |*/
/*|              : char no    : 番号                                       |*/
/*|              : char pos   : 位置(0:月 1:日 2:振替)                     |*/
/*|              : long data  : データ                                     |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_day_dsp(char top, char no, char pos, long data, ushort rev)
{
	if (pos == 0) {
		if (GET_MONTH(data))
			opedsp((ushort)(2+no-top), 14, (ushort)GET_MONTH(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );	/* 月 */
		else
			grachr((ushort)(2+no-top), 14, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);						/* "　　" */
	}
	else if (pos == 1) {
		if (GET_DAY(data))
			opedsp((ushort)(2+no-top), 20, (ushort)GET_DAY(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 日 */
		else
			grachr((ushort)(2+no-top), 20, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "　　" */
	}
	else {		/* if (pos == 2) */
		if (data/100000L)	/* シフトする */
			grachr( (ushort)(2+no-top), 27,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[0] );				/* ○ */
		else		/* シフトしない */
			grachr( (ushort)(2+no-top), 27,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[2] );				/* × */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日／特別期間ー特別期間設定                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_period( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_period(void)
{
	ushort	msg;
	char	i;
	long	input;
	int		no;
	char	pos;
	char	mode;
	long	*pSPP;

	pSPP = &CPrmSS[S_TOK][1];

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[1] );		/* "＜特別期間＞　　　　　　　　　" */
	for (i = 0; i < 3; i++) {
		grachr((ushort)(1+i*2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[11]);			/* "　期間　開始日：　　月　　日　" */
		opedsp((ushort)(1+i*2), 6, (ushort)(i+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 期間 */
		splday_period_dsp((char)(i*2), 0, pSPP[i*2], 0);		/* 月 */
		splday_period_dsp((char)(i*2), 1, pSPP[i*2], 0);		/* 日 */

		grachr((ushort)(2+i*2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[12]);			/* "　　　　終了日：　　月　　日　" */
		splday_period_dsp((char)(i*2+1), 0, pSPP[i*2+1], 0);	/* 月 */
		splday_period_dsp((char)(i*2+1), 1, pSPP[i*2+1], 0);	/* 日 */
	}
	splday_period_dsp(0, 0, pSPP[0], 1);						/* 月 反転 */
	Fun_Dsp( FUNMSG[39] );										/* "　＋　　－　 取消  書込  終了 " */

	pos = 0;
	no = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		/* 通常画面 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* ▲(F1) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
				}
				if (!GET_DAY(pSPP[no])) {
					pSPP[no] = 0;			// 日に有効データがない場合現在行のデータをクリア
					splday_period_dsp(no, 0, 0, 0);
				}
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 0);
				if (--no < 0)
					no = 5;
				pos = 0;
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			case KEY_TEN_F2:		/* ▼(F2) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "					
				}
				if (!GET_DAY(pSPP[no])) {
					pSPP[no] = 0;		// 日に有効データがない場合現在行のデータをクリア
					splday_period_dsp(no, 0, 0, 0);
				}
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 0);
				if (++no > 5)
					no = 0;
				pos = 0;
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			case KEY_TEN_F3:		/* 取消(F3) */
				BUZPI();
				if (pos == 0) {		// 取消ｷｰ：取消ﾓｰﾄﾞに遷移
					if (!GET_DAY(pSPP[no])) {
						pSPP[no] = 0;			// 日に有効データがない場合現在行のデータをクリア
					}
					splday_period_dsp(no, pos, pSPP[no], 0);
					mode = 1;
					no = 0;
					grachr((ushort)(1+no*2), 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "期間" */
					opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 期間 */
					Fun_Dsp(FUNMSG2[14]);											/* "　＋　　－　 取消        終了 " */
					OpelogNo = OPLOG_TOKUBETUKIKAN;									// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				}
				else {				// BACKｷｰ：月に戻る
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos--;
					input = -1;
					splday_period_dsp(no, pos, pSPP[no], 1);
					Fun_Dsp( FUNMSG[39] );				// "　＋　　－　 取消  書込  終了 "
				}
				break;
			case KEY_TEN_F4:		/* 書込(F4) */
				if (pos == 0) {
					if (input == -1)
						input = GET_MONTH(pSPP[no]);
					if (input < 1 || input > 12) {
						BUZPIPI();
						splday_period_dsp(no, pos, pSPP[no], 1);
						input = -1;
						break;
					}
					BUZPI();
					// 月日の正当性チェック
					if (GET_DAY(pSPP[no]) > medget(2004, (short)input))
						pSPP[no] = input*100;
					else
						pSPP[no] = input*100 + pSPP[no]%100;
					OpelogNo = OPLOG_TOKUBETUKIKAN;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos = 1;
					splday_period_dsp(no, pos, pSPP[no], 1);
					input = -1;
					Fun_Dsp( FUNMSG[30] );				/* "  ＋    －    ⊂   書込  終了 " */
				}
				else {	/* if (pos == 1) */
					if (input == -1)
						input = GET_DAY(pSPP[no]);
					if (input < 1 || input > medget(2004, (short)GET_MONTH(pSPP[no]))) {	/* 2004は閏年 */
						BUZPIPI();
						splday_period_dsp(no, pos, pSPP[no], 1);
						input = -1;
						break;
					}
					BUZPI();
					pSPP[no] = pSPP[no]/100*100 + input;
					OpelogNo = OPLOG_TOKUBETUKIKAN;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos = 0;
					if (++no > 5)
						no = 0;
					splday_period_dsp(no, pos, pSPP[no], 1);
					input = -1;
					Fun_Dsp( FUNMSG[39] );				/* "　＋　　－　 取消  書込  終了 " */
				}
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				BUZPI();
				if (input == -1)
					input = 0;
				input = (input*10 + msg-KEY_TEN0) % 100;
				if (pos == 0)
					splday_period_dsp(no, pos, (long)(input*100), 1);
				else
					splday_period_dsp(no, pos, (long)input, 1);
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			default:
				break;
			}
		}
		/* 取消画面 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 取消(F3) */
				pSPP[no*2] = 0;
				pSPP[no*2+1] = 0;
				splday_period_dsp((char)(no*2), 0, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2), 1, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2), 2, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2+1), 0, pSPP[no*2+1], 0);
				splday_period_dsp((char)(no*2+1), 1, pSPP[no*2+1], 0);
				splday_period_dsp((char)(no*2+1), 2, pSPP[no*2+1], 0);
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				grachr((ushort)(1+no*2), 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "期間" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 期間 */
				no = 0;
				pos = 0;
				mode = 0;
				input = -1;
				splday_period_dsp(no, pos, pSPP[no], 1);
				Fun_Dsp( FUNMSG[39] );		/* "　＋　　－　 取消  書込  終了 " */
				break;
			case KEY_TEN_F1:		/* ▲(F1) */
			case KEY_TEN_F2:		/* ▼(F2) */
				BUZPI();
				grachr((ushort)(1+no*2), 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "期間" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 期間 */
				if (msg == KEY_TEN_F1) {
					if (--no < 0)
						no = 2;
				}
				else {
					if (++no > 2)
						no = 0;
				}
				grachr((ushort)(1+no*2), 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "期間" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 期間 */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別期間表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_period_dsp( no, pos, data, rev )                 |*/
/*| PARAMETER    : char no    : 番号                                       |*/
/*|              : char pos   : 位置(0:月 1:日)                            |*/
/*|              : long data  : データ                                     |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_period_dsp(char no, char pos, long data, ushort rev)
{
	if (pos == 0) {
		if (GET_MONTH(data))
			opedsp((ushort)(1+no), 16, (ushort)GET_MONTH(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 月 */
		else
			grachr((ushort)(1+no), 16, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "　　" */
	}
	else {	/* pos == 1 */
		if (GET_DAY(data))
			opedsp((ushort)(1+no), 22, (ushort)GET_DAY(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 日 */
		else
			grachr((ushort)(1+no), 22, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "　　" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日／特別期間ーハッピーマンデー設定                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_monday( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_monday(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-11 */
	long	*pSPM;
	char	dsp;
	char	mode;	/* 0:通常 1:取消 */
	char	pos;	/* 0:月 1:月曜 */
	long	input;

	pSPM = &CPrmSS[S_TOK][42];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* 画面表示 */
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[2] );			/* "＜ハッピーマンデー＞　　　　　" */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
				opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
				splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* 月 */
				splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* 週 */
			}
			if (mode == 0) {
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* 月 反転 */
				Fun_Dsp( FUNMSG[39] );				/* "　＋　　－　 取消  書込  終了 " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);		/* "　取消す番号を選択して下さい　" */
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 反転 */
				Fun_Dsp(FUNMSG2[14]);				/* "　＋　　－　 取消        終了 " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		/* 通常画面 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* ＋(F1) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
				}
				// 週が未設定の場合現在行のデータをクリア
				input = GET_HM_WKNO(no, pSPM[no/2]);
				if (!input) {
					pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
				}
				BUZPI();
				no--;
				if (no < 0) {
					top = SP_HAPPY_MAX-5;
					no = SP_HAPPY_MAX-1;
				}
				else if (no < top)
					top--;
				pos = 0;
				input = -1;
				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* 月 */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* 週 */
				}
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* 月 反転 */
				break;
			case KEY_TEN_F2:		/* －(F2) */
				if (pos == 1) {
					pos = 0;
					Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
				}
				// 週が未設定の場合現在行のデータをクリア
				input = GET_HM_WKNO(no, pSPM[no/2]);
				if (!input) {
					pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
				}
				BUZPI();
				no++;
				if (no > SP_HAPPY_MAX-1)
					top = no = 0;
				else if (no > top+4)
					top++;
				input = -1;
				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* 月 */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* 週 */
				}
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* 月 反転 */
				break;
			case KEY_TEN_F3:		/* 取消(F3) */
				BUZPI();
				if (pos == 0) {		// 取消ｷｰ：取消ﾓｰﾄﾞに遷移
					// 週が未設定の場合現在行のデータをクリア
					input = GET_HM_WKNO(no, pSPM[no/2]);
					if (!input) {
						pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
					}
					mode = 1;
					dsp = 1;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				}
				else {				// BACKｷｰ：前の項目に戻る
					splday_monday_dsp(top, no, pos, pSPM[no/2], 0);
					pos--;
					input = -1;
					splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
					Fun_Dsp( FUNMSG[39] );		// "　＋　　－　 取消  書込  終了 "
				}
				break;
			case KEY_TEN_F4:		/* 書込(F4) */
				if (pos == 0) {
					if (input == -1)
						input = (no & 1) ? pSPM[no/2]%1000/10 : pSPM[no/2]/10000;
					if (input < 1 || input > 12) {
						BUZPIPI();
						input = -1;
						splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
						break;
					}
					BUZPI();
					if (no & 1)
						pSPM[no/2] = pSPM[no/2]/1000*1000 + input*10 + pSPM[no/2]%10;
					else
						pSPM[no/2] = input*10000 + pSPM[no/2]%10000;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_monday_dsp(top, no, pos, pSPM[no/2], 0);
					pos = 1;
					input = -1;
					splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
					Fun_Dsp( FUNMSG[30] );				/* "  ＋    －    ⊂   書込  終了 " */
				}
				else {	/* if (pos == 1) */
					if (input == -1)
						input = (no & 1) ? pSPM[no/2]%10 : pSPM[no/2]/1000%10;
					if (input < 1 || input > 5) {
						BUZPIPI();
						input = -1;
						splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
						break;
					}
					BUZPI();
					if (no & 1)
						pSPM[no/2] = pSPM[no/2]/10*10 + input;
					else
						pSPM[no/2] = pSPM[no/2]/10000*10000 + input*1000 + pSPM[no/2]%1000;
					no++;
					if (no > SP_HAPPY_MAX-1)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// 操作履歴登録
					f_ParaUpdate.BIT.splday = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					/* 表示 */
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
						splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* 月 */
						splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* 週 */
					}
					splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* 月 反転 */
					Fun_Dsp( FUNMSG[39] );				/* "　＋　　－　 取消  書込  終了 " */
				}
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				BUZPI();
				if (input == -1)
					input = 0;
				if (pos == 0) {
					input = (input*10 + msg-KEY_TEN0) % 100;
					if (no & 1)
						splday_monday_dsp(top, no, pos, input*10, 1);
					else
						splday_monday_dsp(top, no, pos, input*10000, 1);
				}
				else {	/* if (pos == 1) */
					input = msg-KEY_TEN0;
					if (no & 1)
						splday_monday_dsp(top, no, pos, input, 1);
					else
						splday_monday_dsp(top, no, pos, input*1000, 1);
				}
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
				input = -1;
				break;
			default:
				break;
			}
		}
		/* 取消画面 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 取消(F3) */
				if (no & 1)
					pSPM[no/2] = pSPM[no/2]/1000*1000;
				else
					pSPM[no/2] = pSPM[no/2]%1000;
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* ＋(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = SP_HAPPY_MAX-5;
					no = SP_HAPPY_MAX-1;
				}
				else if (no < top)
					top--;

				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* 月 */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* 週 */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 反転 */
				break;
			case KEY_TEN_F2:		/* －(F2) */
				BUZPI();
				no++;
				if (no > SP_HAPPY_MAX-1)
					top = no = 0;
				else if (no > top+4)
					top++;

				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "　　　：　　月　第　月曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* 月 */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* 週 */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 反転 */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ハッピーマンデー表示                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_monday_dsp( top, no, pos, data, rev )            |*/
/*| PARAMETER    : char top   : 先頭番号                                   |*/
/*|              : char no    : 番号                                       |*/
/*|              : char pos   : 位置(0:月 1:週)                            |*/
/*|              : long data  : データ                                     |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_monday_dsp(char top, char no, char pos, long data, ushort rev)
{
	ushort	tmp;

	if (pos == 0) {
		tmp = (no & 1) ? (ushort)(data%1000/10) : (ushort)(data/10000);
		if (tmp)
			opedsp((ushort)(2+no-top), 8, tmp, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* 月 */
		else
			grachr((ushort)(2+no-top), 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);		/* "　　" */
	}
	else {	/* if (pos == 1) */
		tmp = (no & 1) ? (ushort)(data%10) : (ushort)(data/1000%10);
		if (tmp)
			opedsp((ushort)(2+no-top), 18, tmp, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* 週 */
		else
			grachr((ushort)(2+no-top), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);		/* "　" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別日／特別期間--特別曜日指定　　　　                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_week( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_week(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-11 */
	long	*pSPM;
	char	dsp;
	char	mode;	/* 0:通常 1:取消 */
	char	pos;	/* 0:月 1:週　2:曜日 */
	long	input,weekcnt=0;

	pSPM = &CPrmSS[S_TOK][56];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* 画面表示 */
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[18] );									/* "＜特別曜日＞ " */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "　　　：　　月　第　曜日　　" */
				opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
				splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* 月 */
				splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* 週 */
				splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* 曜日 */
			}
			if (mode == 0) {
				splday_week_dsp(top, no, 0, pSPM[no], 1);														/* 月 反転 */
				Fun_Dsp( FUNMSG[39] );																			/* "　＋　　－　 取消  書込  終了 " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);									/* "　取消す番号を選択して下さい　" */
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 反転 */
				Fun_Dsp(FUNMSG2[14]);																			/* "　＋　　－　 取消        終了 " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( msg == LCD_DISCONNECT ) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		/* 通常画面 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* ＋(F1) */
				if (pos == 2) {
					BUZPI();
					input = 1;
					weekcnt--;
					if(weekcnt < 1)
						weekcnt = 7;
					splday_week_dsp(top, no, pos, weekcnt, 1);
				}else{
				BUZPI();
				no--;
				if (no < 0) {
					top = 7;
					no = 11;
				}
				else if (no < top)
					top--;
				input = -1;
				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "　　　：　　月　第　曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* 月 */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* 週 */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* 曜日 */
				}
				splday_week_dsp(top, no, pos, pSPM[no], 1);							/* 月 反転 */
				}
				break;
			case KEY_TEN_F2:		/* －(F2) */
				if (pos == 1) {
					BUZPIPI();
					break;
				}else if(pos == 2){
					BUZPI();
					input = 0;
					weekcnt++;
					if(weekcnt > 7)
						weekcnt = 1;
					splday_week_dsp(top, no, pos, weekcnt, 1);

				}else{
					BUZPI();
					no++;
					if (no > 11)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* 表示 */
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "　　　：　　月　第　曜日　　" */
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* 月 */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* 週 */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* 曜日 */
					}
					splday_week_dsp(top, no, 0, pSPM[no], 1);							/* 月 反転 */
				}
				break;
			case KEY_TEN_F3:		/* 取消(F3) */
				BUZPI();
				mode = 1;
				dsp = 1;
				OpelogNo = OPLOG_TOKUBETUWEEKLY;		// 操作履歴登録
				f_ParaUpdate.BIT.splday = 1;			// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				break;
			case KEY_TEN_F4:		/* 書込(F4) */
				if (pos == 0) {												/* 月入力状態？ */
					if (input == -1)										/* 値の変更なし状態で書き込み押下 */
						input = ((pSPM[no] % 10000 ) /100);					/* 表示されている値をセット */
					if (input < 1 || input > 12) {							/* 入力値設定範囲チェック */
						BUZPIPI();
						input = -1;
						splday_week_dsp(top, no, pos, pSPM[no], 1);			
						break;
					}
					BUZPI();
					pSPM[no] = (input*100) + (pSPM[no] % 100);				/* 入力された値をバッファに設定 */
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* 操作履歴登録 */
					f_ParaUpdate.BIT.splday = 1;							// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					splday_week_dsp(top, no, pos, pSPM[no], 0);
					pos = 1;												/* 次の入力へ */
					input = -1;												/* 入力状態フラグを初期値に戻す */
					splday_week_dsp(top, no, pos, pSPM[no], 1);
				}
				else if(pos == 1){											/* 週入力状態？ */
					if (input == -1)										/* 値の変更なし状態で書き込み押下 */
						input = ((pSPM[no] % 100) / 10);					/* 表示されている値をセット */
					if (input < 1 || input > 5) {							/* 入力値設定範囲チェック */
						BUZPIPI();
						input = -1;
						splday_week_dsp(top, no, pos, pSPM[no], 1);
						break;
					}
					BUZPI();
					pSPM[no] = (pSPM[no]/100)*100 + input*10 + (pSPM[no]%10);	/* 入力された値をバッファに設定 */
					if (no > 11)			
						top = no = 0;									
					else if (no > top+4)
						top++;
					pos = 2;												/* 次の入力へ */
					input = -1;												/* 入力状態フラグを初期値に戻す */
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* 操作履歴登録 */
					f_ParaUpdate.BIT.splday = 1;							// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

					/* 表示 */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);		/* 月 */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);		/* 週 */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);		/* 曜日 */
					}
					splday_week_dsp(top, no, 2, pSPM[no], 1);							/* 曜日 反転 */
					weekcnt = (ushort)((pSPM[no]%10) ? (ushort)(pSPM[no]%10):1);

				}
				else {	/* if (pos == 2) */
					if (input == -1){										/* 値の変更なし状態で書き込み押下 */
						weekcnt = (pSPM[no]%10);							/* 表示されている値をセット */
					}
					if (weekcnt < 1 || weekcnt > 7 ) {						/* 入力値設定範囲チェック */
						if(weekcnt != 0){
							BUZPIPI();	
							input = -1;										/* 入力状態フラグを初期値に戻す */
							weekcnt = 1;									/* 曜日カウントを初期値（日）に設定 */
							splday_week_dsp(top, no, pos, pSPM[no], 1);
							break;
						}else{
							weekcnt = 1;									/* 曜日カウントを初期値（日）に設定 */
						}
					}

					BUZPI();
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* 操作履歴登録 */
					f_ParaUpdate.BIT.splday = 1;							// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					pSPM[no] = (pSPM[no]/100)*100 + ((pSPM[no]%100)/10)*10 + weekcnt;	/* 入力された値をバッファに設定 */
					no++;													/* 入力行をインクリメント */
					if (no > 11)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;												/* 次の入力を月に設定 */
					input = -1;												/* 入力状態フラグを初期値に戻す */
					/* 表示 */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);		/* 月 */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);		/* 週 */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);		/* 曜日 */
					}
					splday_week_dsp(top, no, pos, pSPM[no], 1);							/* 月 反転 */
				}
				break;
			case KEY_TEN:			/* 数字(テンキー) */
				BUZPI();
				if (input == -1)
					input = 0;												/* 入力状態フラグの値を更新 */
				if (pos == 0) {												/* 月入力状態？ */
					input = (input*10 + msg-KEY_TEN0) % 100;				/* 入力値を取得（２桁） */
					splday_week_dsp(top, no, pos, input*100, 1);
				}
				else if(pos == 1){											/* 週入力状態？ */
					input = msg-KEY_TEN0;									/* 入力値の取得 */
					splday_week_dsp(top, no, pos, input*10, 1);
				}else{	/* if (pos == 2) */									/* 曜日入力状態？ */
					input = msg-KEY_TEN0;									/* 入力値の取得 */
					if(input > 7 || input == 0){							/* 入力値の範囲チェック */
						BUZPIPI();
						break;
					}
					splday_week_dsp(top, no, pos, input, 1);
					weekcnt = input;										/* ＋-キーとの整合性を保つため曜日カウントを入力値で更新 */
				}
				break;
			case KEY_TEN_CL:		/* 取消(テンキー) */
				BUZPI();
				splday_week_dsp(top, no, pos, pSPM[no], 1);
				input = -1;													/* 入力状態フラグを初期値に戻す */
				break;
			default:
				break;
			}
		}
		/* 取消画面 */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* 取消(F3) */
				pSPM[no] = 0;
			case KEY_TEN_F5:		/* 終了(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* ＋(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = 7;
					no = 11;
				}
				else if (no < top)
					top--;

				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "　　　：　　月　第　曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* 月 */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* 週 */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* 曜日 */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 番号 反転 */
				break;
			case KEY_TEN_F2:		/* －(F2) */
				BUZPI();
				no++;
				if (no > 11)
					top = no = 0;
				else if (no > top+4)
					top++;

				/* 表示 */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "　　　：　　月　第　曜日　　" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 番号 */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* 月 */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* 週 */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* 曜日 */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 番号 反転 */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  特別曜日表示　　　　                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_week_dsp( top, no, pos, data, rev )   　         |*/
/*| PARAMETER    : char top   : 先頭番号                                   |*/
/*|              : char no    : 番号                                       |*/
/*|              : char pos   : 位置(0:月 1:週 2:曜日)                     |*/
/*|              : long data  : データ                                     |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static void	splday_week_dsp(char top, char no, char pos, long data, ushort rev)
{
	ushort	tmp=0;

	if (pos == 0) {
		tmp = (ushort)((data % 10000) / 100);							/* 引数より月算出 */
		if (tmp)
			opedsp((ushort)(2+no-top), 8, tmp, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* 月 */
		else
			grachr((ushort)(2+no-top), 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* 初期表示用 */
	}
	else if(pos == 1){
		tmp = (ushort)((data % 100) / 10);								/* 引数より週算出 */
		if (tmp)
			opedsp((ushort)(2+no-top), 18, tmp, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* 週 */
		else
			grachr((ushort)(2+no-top), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* 初期表示用 */
	}else{
		tmp = (ushort)(data % 10);										/* 引数より曜日算出 */	
		if(!tmp && rev == 1){											/* データが０且つ反転表示させる場合(初期表示用) */
			grachr((ushort)(2+no-top), 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, WEEKLYFFNT[1]);		/* "日"表示 */
		}else{			
			grachr( (ushort)(2+no-top), 20,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, WEEKLYFFNT[tmp] );	/* "日～土"表示 */
		}
	}
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
			if ((OPECTL.PriEndMsg[no].BPrinStat & 0x08) == 0) {
			// ヘッド温度異常以外は紙切れと見なす
				ret = PRI_NO_PAPER;
			}
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			プリント結果チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind : 対象プリンタ
///	@param[in]		disp : 結果表示する<br>
//							1:左詰表示（中止を除外）<br>
//							2:左詰表示<br>
//							3:中央表示（中止を除外）<br>
//							4:中央表示
///	@param[in]		line : 結果表示行
///	@return			PRI_NML_END／PRI_CSL_END／PRI_ERR_END
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CheckPrintResult(uchar pri_kind, int disp, int line)
{
	static	const	char	msgtbl[][4] = {
		{  0,  9, 10,  0},		{ 0,  9, 10, 11},
		{  0, 12, 13,  0},		{ 0, 12, 13, 14}
	};
	int		ret = PRI_ERR_END;
	int		no;

	switch(pri_kind) {
	default:
		ret = check_print_result(0);
		break;
	case	RJ_PRI:
		ret = check_print_result(0);
		if (ret != PRI_NML_END)
			break;
	// not break
	case	J_PRI:
		ret = check_print_result(1);
		break;
	}
	if (disp) {
		switch(ret) {
		default:
			no = 0;
			break;
		case	PRI_ERR_END:
			no = 1;
			break;
		case	PRI_NO_PAPER:
			no = 2;
			break;
		case	PRI_CSL_END:
			no = 3;
			break;
		}
		no = msgtbl[disp-1][no];
		if (no != 0) {
			grachr( (ushort)line,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[no] );
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			料金設定：プリント
//[]----------------------------------------------------------------------[]
///	@return			
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/27
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
unsigned short	UsMnt_mnyprint(void)
{
	ushort msg;
	int		inji_end;
	int		can_req;
	T_FrmChargeSetup	FrmChargeSetup;
	T_FrmPrnStop		FrmPrnStop;

	dispclr();

	FrmChargeSetup.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// 機械№
	FrmChargeSetup.Oiban = CountRead_Individual(SET_PRI_COUNT);						// 追番読出し
	memcpy( &FrmChargeSetup.NowTime, &CLK_REC, sizeof( date_time_rec ) );			// 現在時刻
	FrmChargeSetup.prn_kind = R_PRI;
	queset( PRNTCBNO, PREQ_CHARGESETUP, sizeof(T_FrmChargeSetup), &FrmChargeSetup );// 料金設定印字
	Ope_DisableDoorKnobChime();
	wopelg( OPLOG_CHARGE_PRINT, 0, 0 );
	inji_end = 0;
	can_req = 0;
	Cal_Parameter_Flg = 0;					// 設定値異常フラグクリア

	grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0]);		// "＜料金設定＞　　　　　　　　　"
	grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "　　　 プリント中です 　　　　"
	Fun_Dsp(FUNMSG[82]);					// "　　　　　　 中止 　　　　　　"
	// 追い番カウントアップ
	CountUp_Individual(SET_PRI_COUNT);

	while (1) {
		msg = StoF(GetMessage(), 1);
		if (msg == (INNJI_ENDMASK|PREQ_CHARGESETUP)) {
			inji_end = 1;
			msg &= (~INNJI_ENDMASK);
		}
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			// ドアノブの状態にかかわらずトグル動作してしまうので、
			// ドアノブ閉かどうかのチェックを実施
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			BUZPI();
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			break;
		case KEY_TEN_F3:
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// 印字中止要求
					can_req = 1;
					FrmPrnStop.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					wopelg( OPLOG_CAN_SET_PRI, 0, 0);			// プリント中止操作モニタ登録
				}
			}
			else {
				BUZPI();
				return MOD_EXT;
			}
			break;
		case PREQ_CHARGESETUP:
			if (inji_end == 1) {
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：サービスタイム                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_SrvTime( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define INDEX_MAX	12		//車種A～Lの個数
unsigned short	UsMnt_SrvTime(void)
{
	ushort	msg;
	long	*pSTM;
	int		pos;		/* 0:サービスタイム 1:グレースタイム 2:ラグタイム */
	char	changing;
	long	min;
	long	max_tbl[] = {720, 120, 120};

	ushort	sel;		//サービスタイム切換選択
	ushort	index;		//車種へのインデックス

	sel = (ushort)prm_get( COM_PRM,S_STM,1,1,1 );
	if (sel == 0) {
		index = 0;
	} else {
		index = 1;
	}

	pSTM = &CPrmSS[S_STM][2];

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[23]);			/* "＜サービスタイム＞　　　　　　" */
	if (index) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR5[0]);			/* "［Ａ車種］" */
	}
	grachr(2, 4, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[0]);					/* "サービスタイム" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "：" */
	opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
	grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);				/* "分" */
	grachr(3, 4, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[1]);					/* "グレースタイム" */
	grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "：" */
	opedsp(3, 20, (ushort)pSTM[index*3+1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);/* 分 */
	grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);				/* "分" */
	grachr(4, 4, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[8]);					/* "ラグタイム" */
	grachr(4, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "：" */
	opedsp(4, 20, (ushort)pSTM[index*3+2], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);/* 分 */
	grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);			/* "分" */
	if (index) {
		Fun_Dsp(FUNMSG2[9]);						/* "　▲　　▼　 変更　　　　終了 " */
	} else {
		Fun_Dsp(FUNMSG2[0]);						/* "　　　　　　 変更　　　　終了 " */
	}

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* 変更(F3) */
				BUZPI();
				opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
				Fun_Dsp(FUNMSG[20]);		/* "　▲　　▼　　　　 書込  終了 " */
				min = -1;
				pos = 0;
				changing = 1;
				break;
			case KEY_TEN_F1:	/* ▲ */
			case KEY_TEN_F2:	/* ▼ */
				if (index == 0) {	//車種切替なし
					break;
				}
				//車種インデックスの取得
				if (msg == KEY_TEN_F1) {
					if (index == 1) {
						index = INDEX_MAX;
					} else {
						index--;
					}
				} else {
					if (index == INDEX_MAX) {
						index = 1;
					} else {
						index++;
					}
				}
				BUZPI();
				//画面表示変更
				grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR5[index-1]);				/* "［A～L車種］" */
				opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* サービスタイム 分 */
				opedsp(3, 20, (ushort)pSTM[index*3+1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* グレースタイム 分 */
				opedsp(4, 20, (ushort)pSTM[index*3+2], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* ラグタイム 分 */
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				if (index) {
					Fun_Dsp(FUNMSG2[9]);			/* "　▲　　▼　 変更　　　　終了 " */
				} else {
					Fun_Dsp(FUNMSG2[0]);			/* "　　　　　　 変更　　　　終了 " */
				}
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 正転 */
				changing = 0;
				break;
			case KEY_TEN_F4:	/* 書込(F4) */
				if (min != -1) {
					if (min > max_tbl[pos]) {
						BUZPIPI();
						opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
						min = -1;
						break;
					}
					pSTM[index*3+pos] = min;
					OpelogNo = OPLOG_SERVICETIME;		// 操作履歴登録
					f_ParaUpdate.BIT.other = 1;			// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
					mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTPフラグセット
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				}
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 正転 */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = 2;
				}
				else {
					if (++pos > 2)
						pos = 0;
				}
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 反転 */
				min = -1;
				break;
			case KEY_TEN:
				BUZPI();
				if (min == -1)
					min = 0;
				min = (min*10 + msg-KEY_TEN0) % 1000;
				opedsp((ushort)(2+pos), 20, (ushort)min, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);							/* 分 反転 */
				break;
			case KEY_TEN_CL:
				BUZPI();
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 反転 */
				min = -1;
				break;
			default:
				break;
			}
			
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ロック装置閉タイマー                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LockTimer( void )                                 |*/
/*| PARAMETER    : type 0：フラップ 1：ロック                              |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	GET_MIN(t)		(((t)%100000L)/100)
#define	GET_SEC(t)		((t)%100)
enum{
	IDLE = 0,
	KIND,
	MIN,
	SEC
};
unsigned short	UsMnt_LockTimer( uchar type )
{
	ushort	msg;
	long	*ptime;
	char	set;
	long	data;
	long	tmp;

	char	syu = 1;
	uchar	t_type;

	if( !type ){
		if( !Get_Pram_Syubet(FLAP_UP_TIMER) ){
			ptime = &CPrmSS[S_TYP][118];	// フラップ上昇タイマー
			t_type = 0;
		}else{
			ptime = &CPrmSS[S_LTM][31];		// フラップ上昇タイマー(Ａ種)
			t_type = 1;
		}
	}else{
		if( !Get_Pram_Syubet(ROCK_CLOSE_TIMER) ){
			ptime = &CPrmSS[S_TYP][69];		// ロック閉タイマー
			t_type = 0;
		}else{
			ptime = &CPrmSS[S_LTM][11];		// ロック閉タイマー(Ａ種)
			t_type = 1;
		}
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[24]);					/* "＜ロック装置閉タイマー＞　　　" */
		grachr(3,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[(!type?27:28)]);		/* "フラップ　" or "駐輪ロック" */
		grachr(3, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "：" */
		if( t_type ){
			grachr(3, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);				/* 種別 */
		}
		opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "分" */
		opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
		grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "秒" */
	Fun_Dsp(FUNMSG2[0]);						/* "　　　　　　 変更　　　　終了 " */

	set = IDLE;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
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
		if (set == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* 変更(F3) */
				BUZPI();
				if( t_type ){
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* 種別 */
					Fun_Dsp(FUNMSG[20]);				/* "　▲　　▼　　　　 書込  終了 " */
					set = KIND;
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
					Fun_Dsp(FUNMSG2[1]);				/* "　　　　　　 　　　書込　終了 " */
					set = MIN;
				}
				data = -1;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:
				BUZPI();
				if( t_type ){
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* 種別 */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
				}
				Fun_Dsp(FUNMSG2[0]);				/* "　　　　　　 変更　　　　終了 " */
				set = IDLE;
				break;
			case KEY_TEN_F4:
				if (set == KIND) {
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);								/* 種別 正転 */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
					set = MIN;
					data = -1;
					Fun_Dsp(FUNMSG2[1]);											/* "　　　　　　 　　　書込　終了 " */
				}else if (set == MIN) {
					if (data != -1) {
						tmp = *ptime/100000L*100000L + data*100 + *ptime%100;
						if ((tmp%100000L) > 72000) {
							BUZPIPI();
							opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
							data = -1;
							break;
						}
						*ptime = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 正転 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
					data = -1;
					set = SEC;
				}
				else {
					if (data != -1) {
						tmp = *ptime/100*100 + data;
						if ((tmp%100000L) > 72000 || data > 59) {
							BUZPIPI();
							opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
							data = -1;
							break;
						}
						*ptime = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
					if( t_type ){
						grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);							/* 種別 正転*/
						Fun_Dsp(FUNMSG[20]);				/* "　▲　　▼　　　　 書込  終了 " */
					}else{
						opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 反転 */
					}
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 秒 正転 */
					data = -1;
					set = (t_type ? KIND:MIN);
				}
				BUZPI();
				break;
			case KEY_TEN:
				if (data == -1)
					data = 0;
				if (set == MIN) {
					data = (data*10 + msg-KEY_TEN0) % 1000;
					opedsp(3, 16, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
				}
				else if(set == SEC){
					data = (data*10 + msg-KEY_TEN0) % 100;
					opedsp(3, 24, (ushort)data, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
				}else{	// set == KIND
					break;
				}
				BUZPI();
				break;
			case KEY_TEN_CL:
				if (set == MIN){
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
				}else if(set == SEC){
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
				}else{// set == KIND
					break;
				}
				BUZPI();
				data = -1;
				break;
			case KEY_TEN_F1:
				if( set == KIND ){
					BUZPI();
					syu--;
					ptime--;
					if( syu < 1 ){
						syu = 12;
						ptime += 12;
					}
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* Ａ種 */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 正転 */
				}
				break;
			case KEY_TEN_F2:
				if( set == KIND ){
					BUZPI();
					syu++;
					ptime++;
					if( syu > 12 ){
						syu = 1;
						ptime -= 12;
					}
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* Ａ種 */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 正転 */
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：係員パスワード                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassWord( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/20 ART:ogura 表示修正                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassWord(void)
{
	const uchar	*role_tbl[] = {DAT3_3[1], DAT3_3[3], DAT3_3[4], DAT3_3[7]};
	ushort		msg;
	short		role;		// 役割(0-3)
	short		lv;			// レベル(0-5)
	long		pswd;
	char		no;			/* 係員番号(0-9) */
	char		set;		/* 0:役割設定 1:パスワード設定 */
	char		i;
	char		top;

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[25]);				/* "＜係員パスワード＞　　　　　　" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[26]);				/* "           役割  ﾚﾍﾞﾙ  ﾊﾟｽﾜｰﾄﾞ" */

	for (i=0; i < 5; i++) {
		pswd = (ushort)prm_get(COM_PRM, S_PSW, (short)(i*2+1), 4, 1);		// 係員№1～5(パスワード)
		role = (short)prm_get(COM_PRM, S_PSW, (short)(i*2+2), 1, 1);		// 係員№1～5(役割)
		lv = (short)prm_get(COM_PRM, S_PSW, (short)(i*2+2), 1, 2);			// 係員№1～5(レベル)
		grachr((ushort)(2+i), 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, &DAT3_3[3][1]);				/* "係員" */
		opedsp((ushort)(2+i), 4, (ushort)(i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 係員番号 */
		grachr((ushort)(2+i), 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "：" */
		if (i == 0) {
			grachr((ushort)(2+i), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 反転 */
		} else {
			grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 */
		}
		opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* レベル */
		if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
			grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);			/* "＊＊＊＊" */
		} else {
			opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* パスワード */
		}
	}
	Fun_Dsp( FUNMSG2[12] );							/* "　▲　　▼　 変更　書込　終了 " */

	no = 0;
	set = 0;
	pswd = prm_get(COM_PRM, S_PSW, 1, 4, 1);				// 係員№１ パスワード
	role = (short)prm_get(COM_PRM, S_PSW, 2, 1, 1);			// 係員№１ 役割(27-0002⑥)
	lv = (short)prm_get(COM_PRM, S_PSW, 2, 1, 2);			// 係員№１ レベル(27-0002⑤)
	top = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == KEY_TEN_F5) {	/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		}
		if (set == 0) {
		/* 役割設定中 */
			switch (msg) {
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				if (msg == KEY_TEN_F1) {
					if (no == 0) {
						top = 5;
						no = 9;
					}
					else if (no == 5) {
						top = 0;
						no = 4;
					}
					else
						no--;
				}
				else {
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;
				}

				for (i=0; i < 5; i++) {
					pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// 係員№1～5(パスワード)
					role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// 係員№1～5(役割)
					lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// 係員№1～5(レベル)
					opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 係員番号 */
					grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 */
					opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* レベル */
					if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
						grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "＊＊＊＊" */
					} else {
						opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* パスワード */
					}
				}
				pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
				role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
				lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
				grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 反転 */
				break;
			case KEY_TEN_F3:	/* 変更(F3) */
				/* 技術員操作の場合 */
				if (OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4) {
					if (++role > 3)
						role = 0;
				}
				/* 技術員操作以外 */
				else {
					/* "技術員"の場合は変更不可 */
					if (role == 3) {
						BUZPIPI();
						break;
					}
					if (++role > 2)
						role = 0;
				}
				BUZPI();
				grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 反転 */
				break;
			case KEY_TEN_F4:	/* 書込(F4) */
				if (OPECTL.Mnt_lev < 3 && role == 3) {
					BUZPIPI();
					break;
				}
				BUZPI();
				prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 1, (long)role);
				if ((role == 0) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {	// 次の係員へ
					if (role == 0) {	/* 役割なし-レベルとパスワードを０にして次の係員へ */
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 2, 0);
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+1), 4, 1, 0);
					}
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;

					for (i=0; i < 5; i++) {
						pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// 係員№1～5(パスワード)
						role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// 係員№1～5(役割)
						lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// 係員№1～5(レベル)
						opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 係員番号 */
						grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 */
						opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* レベル */
						if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
							grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "＊＊＊＊" */
						} else {
							opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* パスワード */
						}
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 反転 */
				}
				else {
					Fun_Dsp(FUNMSG2[1]);					/* "　　　　　　 　　　書込　終了 " */
					grachr((ushort)(2+no-top), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);			/* 役割 正転 */
					if ((OPECTL.Mnt_lev >= 3) || (OPECTL.PasswordLevel >= lv)) {	// レベル設定へ
						opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* レベル 反転 */
						lv = -1;
						set = 1;
					} else {														// パスワード設定へ
						opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* パスワード 反転 */
						pswd = -1;
						set = 2;
					}
				}
				OpelogNo = OPLOG_KAKARIINPASS;		// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;			// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP更新フラグセット
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				break;
			default:
				break;
			}
		}
		else {
		/* レベルまたはパスワード設定中 */
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F4:	/* 書込(F4) */
				if (set == 1) {				// レベル設定中
					if ((lv > 5) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
						BUZPIPI();
						lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
						opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* レベル 反転 */
						break;
					}
					BUZPI();
					if (lv != -1) {
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 2, (long)lv);
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* レベル 正転 */
					opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* パスワード 反転 */
					pswd = -1;
					set = 2;
				} else {					// パスワード設定中
					BUZPI();
					if (pswd != -1) {
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+1), 4, 1, pswd);
					}
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;

					for (i=0; i < 5; i++) {
						pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// 係員№1～5(パスワード)
						role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// 係員№1～5(役割)
						lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// 係員№1～5(レベル)
						opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 係員番号 */
						grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 */
						opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* レベル */
						if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
							grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "＊＊＊＊" */
						} else {
							opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* パスワード */
						}
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);									// 係員№1～5(パスワード)
					role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* 役割 反転 */
					Fun_Dsp(FUNMSG2[12]);							/* "　▲　　▼　 変更　書込　終了 " */
					set = 0;
				}
				OpelogNo = OPLOG_KAKARIINPASS;		// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;			// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				break;
			case KEY_TEN:		/* 数字(テンキー) */
				BUZPI();
				if (set == 1) {				// レベル設定中
					if (lv == -1) {
						lv = 0;
					}
					lv = (lv*10 + msg-KEY_TEN0) % 10;
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* レベル 反転 */
				} else {					// パスワード設定中
					if (pswd == -1) {
						pswd = 0;
					}
					pswd = (pswd*10 + msg-KEY_TEN0) % 10000;
					opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* パスワード 反転 */
				}
				break;
			case KEY_TEN_CL:	/* 取消(テンキー) */
				BUZPI();
				if (set == 1) {				// レベル設定中
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* レベル 反転 */
					lv = -1;
				} else {					// パスワード設定中
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					opedsp((ushort)(2+no-top), 22, pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* パスワード 反転 */
					pswd = -1;
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：バックライト点灯方法                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_BackLignt( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_BackLignt(void)
{
	ushort	msg;
	long	blgt;
	char	changing;
	long	data;

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[32]);		/* "＜バックライト点灯方法＞　　　" */
	grachr(2, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);			/* "現在の状態 */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "：" */

	blgt = CPrmSS[S_PAY][29] % 10;
	if (blgt < 1)
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "常時点灯" */
	else if (blgt < 2)
		grachr(2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
	else if (blgt < 3)
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "外部信号" */
	else
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */
	grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "常時点灯" */
	grachr(4, 14, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
	grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "外部信号" */
	grachr(5, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */
	backlgt_time_dsp(1, CPrmSS[S_PAY][30], 0);
	grachr(6, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[8]);			/* "～" */
	backlgt_time_dsp(2, CPrmSS[S_PAY][31], 0);
	backlgt_dsp(blgt, 1);
	Fun_Dsp( FUNMSG[13] );					/* "　←　　→　　　　 書込  終了 " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ←(F1) */
		case KEY_TEN_F2:	/* →(F2) */
			if (changing)
				break;
			BUZPI();
			backlgt_dsp(blgt, 0);
			if (msg == KEY_TEN_F1) {
				blgt -= 1;
				if (blgt < 0)
					blgt += 4;
			}
			else {
				blgt += 1;
				if (blgt >= 4)
					blgt -= 4;
			}
			backlgt_dsp(blgt, 1);
			break;
		case KEY_TEN_F4:	/* 書込(F4) */
			if (changing == 0) {
				BUZPI();
				CPrmSS[S_PAY][29] = CPrmSS[S_PAY][29]/10*10 + blgt;
				grachr(2, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* "　　" */
				if (blgt < 1)
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "常時点灯" */
				else if (blgt < 2)
					grachr(2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
				else if (blgt < 3)
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "外部信号" */
				else {
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */
					backlgt_dsp(blgt, 0);
					backlgt_time_dsp(1, CPrmSS[S_PAY][30], 1);
					Fun_Dsp(FUNMSG2[1]);		/* "　　　　　　　　　 書込  終了 " */
					changing = 1;
					data = -1;
				}
				OpelogNo = OPLOG_BACKLIGHT;		// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
 				mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			}
			else {
				if (data != -1) {
					if (data%100 > 59 || data > 2359) {
						BUZPIPI();
						backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
						data = -1;
						break;
					}
					CPrmSS[S_PAY][29+changing] = data;
				}
				BUZPI();
				backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 0);
				if (changing == 1) {
					changing = 2;
					backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
					data = -1;
				}
				else {
					backlgt_dsp(blgt, 1);
					Fun_Dsp( FUNMSG[13] );					/* "　←　　→　　　　 書込  終了 " */
					changing = 0;
				}
			}
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			if (data == -1)
				data = 0;
			data = (data*10 + msg-KEY_TEN0) % 10000;
			backlgt_time_dsp(changing, data, 1);
			break;
		case KEY_TEN_CL:	/* 取消(テンキー) */
			BUZPI();
			if (changing == 0) {
				backlgt_dsp(blgt, 0);
				blgt = CPrmSS[S_PAY][29] % 10;
				backlgt_dsp(blgt, 1);
			}
			else {
				backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
				data = -1;
			}
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  バックライト点灯方法表示                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( data, rev )                                 |*/
/*| PARAMETER    : long   data : 点灯状態                                  |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	backlgt_dsp(long data, ushort rev)
{
	data = data%10;
	if (data < 1) {
		grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);		/* "常時点灯" */
	}
	else if (data < 2) {
		grachr(4, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
	}
	else if (data < 3) {
		grachr(5, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);		/* "外部信号" */
	}
	else {
		grachr(5, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  バックライト点灯方法表示                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : backlgt_time_dsp( chng, time, rev )                     |*/
/*| PARAMETER    : char   chng : 1:開始時刻 2:終了時刻                     |*/
/*|              : long   time : 時刻                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	backlgt_time_dsp(char chng, long time, ushort rev)
{
	chng -= 1;
	opedsp(6, (ushort)(4+(chng*12)), (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
	grachr(6, (ushort)(8+(chng*12)), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
	opedsp(6, (ushort)(10+(chng*12)), (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
}


// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//[]----------------------------------------------------------------------[]
//	@brief			アナウンスの音声をLCDに送信するための値に変換
//[]----------------------------------------------------------------------[]
//	@param[in]		uchar volume:RXMの音量、	uchar kind:アナウンス == 0,キー音 == 1
//	@return			なし
//	@author			Y.Yamauchi
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2020/02/28<br> 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar Anounce_volume ( uchar volume )
{
	// LCDへの音量送信テーブル
	uchar	announce_vol[ADJUS_MAX_VOLUME+1] = {0,7,14,21,28,35,42,50,56,63,69,75,81,87,93,100}; // LCD用アナウンス音量
	
	return announce_vol[volume];		// アナウンス
}

//[]----------------------------------------------------------------------[]
//	@brief			キーの音声をLCDに送信するための値に変換
//[]----------------------------------------------------------------------[]
//	@param[in]		uchar volume:RXMの音量
//					kind: 0 = ポジション（キー音量設定から）
//						  1 = RXMのパラメータから
//	@return			なし
//	@author			Y.Yamauchi
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2020/02/28<br> 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar Key_volume ( uchar volume, uchar kind)
{
	// LCDへの音量送信テーブル
	uchar	kye_vol[4] = {0, 1, 2, 3};	// LCD用キー音量 0:無音 1:小 2:中 3:大

	if( kind == 1 ){
		switch( volume ){
			case 0:
				return 0;		// 無音
			case 1:
				return 3;		// 大
			case 2:
				return 2;		// 中
			case 3:
				return 1;		// 小
			default:
				break;
		}
	}
	return ( kye_vol[volume] );
}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ブザー音量調整                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_KeyVolume( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_KeyVolume(void)
{
	ushort	msg;
	long	vol;
	int		pos;	/* 0:無音 1:小 3:中 4:大 */
	long	vol_tbl[5] = {0, 30, 20, 10};	// RXMパラメータ用 0:無音 30:小 20:中 10:大
// MH810100(S) 2020/02/21 Y.Yamauchi 車番チケットレス(メンテナンス)
	uchar	volume;	// 音量
// MH810100(E) 2020/02/21 Y.Yamauchi 車番チケットレス(メンテナンス)

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[27]);		/* "＜ブザー音量調整＞　　　　　　" */
	grachr(2, 4, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);			/* "現在の状態 */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "：" */

	vol = CPrmSS[S_PAY][29] % 100;
	if (vol < 10) {
		grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);		/* "無音" */
		pos = 0;
	}
	else if (vol < 20) {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);		/* "大" */
		pos = 3;
	}
	else if (vol < 30) {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "中" */
		pos = 2;
	}
	else {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);		/* "小" */
		pos = 1;
	}
	keyvol_dsp(pos);						/* "無音　小　中　大" */
	Fun_Dsp( FUNMSG[13] );					/* "　←　　→　　　　 書込  終了 " */

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了(F5) */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			volume = Key_volume( ((uchar)prm_get(COM_PRM, S_PAY, 29, 1, 2)), 1);	// RXMのキー音量
			PKTcmd_beep_volume( volume );		// ブザー要求（設定）
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ←(F1) */
		case KEY_TEN_F2:	/* →(F2) */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 3;
			}
			else {
				if (++pos > 3)
					pos = 0;
			}
			keyvol_dsp(pos);			/* "無音　小　中　大" */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			vol = CPrmSS[S_PAY][29];
//			CPrmSS[S_PAY][29] = vol_tbl[pos] + (vol%10);
//			BUZPI();
//			CPrmSS[S_PAY][29] = vol;
			volume = Key_volume( pos, 0);		// キー音量
			PKTcmd_beep_volume( volume );		// ブザー要求（設定）
			BUZPI();
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			break;
		case KEY_TEN_F4:	/* 書込(F4) */
			CPrmSS[S_PAY][29] = vol_tbl[pos] + (CPrmSS[S_PAY][29]%10);	// パラメータに反映
			BUZPI();
			grachr(2, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);				/* "　" */
			if (pos == 0)
				grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);			/* "無音" */
			else if (pos == 3)
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);			/* "大" */
			else if (pos == 2)
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);			/* "中" */
			else
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);			/* "小" */
			OpelogNo = OPLOG_KEYVOLUME;		// 操作履歴登録
			f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP更新フラグセット
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			break;
		case KEY_TEN_CL:	/* 取消(テンキー) */
			BUZPI();
			vol = CPrmSS[S_PAY][29] % 100;
			if (vol < 10)
				pos = 0;
			else if (vol < 20)
				pos = 3;
			else if (vol < 30)
				pos = 2;
			else
				pos = 1;
			keyvol_dsp(pos);			/* "無音　小　中　大" */
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  音量表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( pos )                                       |*/
/*| PARAMETER    : char pos : 反転位置                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	keyvol_dsp(char pos)
{
	grachr(4,  8, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);			/* "無音" */
	grachr(4, 14, 2, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);			/* "小" */
	grachr(4, 18, 2, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);			/* "中" */
	grachr(4, 22, 2, ((pos==3)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);			/* "大" */
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：券期限                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_TickValid( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_TickValid(void)
{
	TIC_PRM	*wktic;
	TIC_PRM	*ptic;
	TIC_PRM	tic;
	ushort	msg;
	char	changing;
	char	no;				/* 期限番号(1-3) */
	char	pos;
	ushort	rev;
	long	*p;
	char	kind;
	long	data;
	long	tmp;

	wktic = &tick_valid_data.tic_prm;
	ptic = (TIC_PRM *)&CPrmSS[S_DIS][8];

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "＜券期限＞　　　　　　　　　" */
	grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "期限" 反転 */
	opedsp(2, 8, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* "１"   反転 */
	grachr(3, 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "期限" */
	opedsp(3, 8, 2, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* "２" */
	grachr(4, 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "期限" */
	opedsp(4, 8, 3, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* "３" */
	Fun_Dsp( FUNMSG[25] );					/* "　▲　　▼　　　　 読出  終了 " */

	changing = 0;
	no = 1;
	tick_valid_data.no = no;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
		if (changing == 0) {
		/* ---期限番号選択中--- */
			switch (msg) {
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				grachr((ushort)(1+no), 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
				opedsp((ushort)(1+no), 8, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
				if (msg == KEY_TEN_F1) {
					if (--no < 1)
						no = 3;
				}
				else {
					if (++no > 3)
						no = 1;
				}
				tick_valid_data.no = no;
				grachr((ushort)(1+no), 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" 反転 */
				opedsp((ushort)(1+no), 8, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号   反転 */
				break;
			case KEY_TEN_F4:	/* 読出(F4) */
				BUZPI();
				tic = ptic[no-1];
				memcpy( wktic, &tic, sizeof(TIC_PRM) );
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);				/* "＜券期限＞　　　　　　　　　　" */
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[29]);				/* "　　　　種別　　　　　内容　　" */
				grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);					/* "期限" */
				opedsp(2, 4, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 番号 */
				ticval_kind_dsp(tic.kind, 1);					/* 種別 反転 */
				ticval_data_dsp(tic.kind, tic.data, 0);			/* 内容 */
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "　変更　　　　　年　　月　　日" */
				grachr(3, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[9]);					/* "前" */
				grachr(3, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);					/* "開始" */
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "　変更　　　　　年　　月　　日" */
				grachr(4, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[9]);					/* "前" */
				grachr(4, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);					/* "終了" */
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "　変更　　　　　年　　月　　日" */
				grachr(5, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[10]);					/* "後" */
				grachr(5, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);					/* "開始" */
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "　変更　　　　　年　　月　　日" */
				grachr(6, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[10]);					/* "後" */
				grachr(6, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);					/* "終了" */
				p = tic.date;
				ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* 日付 */
				Fun_Dsp( FUNMSG2[10] );					/* "　　　　　　 変更　書込　終了 " */
				kind = (char)tic.kind;
				pos = 0;
				changing = 1;
				kind = (char)tic.kind;
				break;
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
		else {
		/* ---各項目設定中--- */
			if (msg == KEY_TEN_F5) {	/* 終了(F5) */
				if( NG == ticval_date_chk( tic.date[0] ) ||		// 変更前開始日付ﾁｪｯｸ
					NG == ticval_date_chk( tic.date[1] ) ||		// 変更前終了日付ﾁｪｯｸ
					NG == ticval_date_chk( tic.date[2] ) ||		// 変更後開始日付ﾁｪｯｸ
					NG == ticval_date_chk( tic.date[3] ) ){		// 変更後終了日付ﾁｪｯｸ

					// 日付ﾃﾞｰﾀが不正な場合
					BUZPIPI();
					continue;
				}
				BUZPI();
				ptic[no-1] = tic;	/* パラメータ更新 */
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "＜券期限＞　　　　　　　　　" */
				rev = (no == 1) ? 1 : 0;
				grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
				opedsp(2, 8, 1, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "１" */
				rev = (no == 2) ? 1 : 0;
				grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
				opedsp(3, 8, 2, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "２" */
				rev = (no == 3) ? 1 : 0;
				grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
				opedsp(4, 8, 3, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "３" */
				Fun_Dsp( FUNMSG[25] );					/* "　▲　　▼　　　　 読出  終了 " */
				changing = 0;
				if( OpelogNo ){
					wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
					OpelogNo = 0;
					SetChange = 1;			// FLASHｾｰﾌﾞ指示
					UserMnt_SysParaUpdateCheck( OpelogNo );
					SetChange = 0;
				}
			}
			if (pos == 0) {
			/* ---種別設定中--- */
				switch (msg) {
				case KEY_TEN_F3:	/* 変更 */
					BUZPI();
					if (++kind > 4)
						kind = 0;
					if (kind == 3)
						kind = 4;	// 割引券項目マスク
					ticval_kind_dsp((long)kind, 1);					/* 種別 反転 */
					break;
				case KEY_TEN_F4:	/* 書込 */
					BUZPI();
					OpelogNo = OPLOG_KENKIGEN;			// 操作履歴登録
					f_ParaUpdate.BIT.tickvalid = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					if (kind == 0) {			/* なし */
						/* 期限番号選択へ */
						tic.kind = kind;
						tic.data = tic.date[0] = tic.date[1] = tic.date[2] = tic.date[3] = 0;
						ptic[no-1] = tic;			/* パラメータ更新 */
						dispclr();
						grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "＜券期限＞　　　　　　　　　" */
						rev = (no == 1) ? 1 : 0;
						grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
						opedsp(2, 8, 1, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "１" */
						rev = (no == 2) ? 1 : 0;
						grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
						opedsp(3, 8, 2, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "２" */
						rev = (no == 3) ? 1 : 0;
						grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "期限" */
						opedsp(4, 8, 3, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "３" */
						Fun_Dsp( FUNMSG[25] );						/* "　▲　　▼　　　　 読出  終了 " */
						changing = 0;
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						if( OpelogNo ){
							wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
							OpelogNo = 0;
							SetChange = 1;			// FLASHｾｰﾌﾞ指示
							UserMnt_SysParaUpdateCheck( OpelogNo );
							SetChange = 0;
						}
					}
					else if (kind == 4) {	/* 全て */
						/* 日付設定へ */
						pos = 2;
						tic.kind = kind;
						ticval_kind_dsp((long)kind, 0);						/* 種別 */
						ticval_data_dsp(tic.kind, tic.data, 0);			/* 内容 */
						p = tic.date;
						ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
						Fun_Dsp(FUNMSG2[2]);			/* "　←　　→　　　　 書込  終了 " */
						data = -1;
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
					}
					else {
						/* 内容設定へ */
						pos = 1;
						if (tic.kind != kind) {
							tic.kind = kind;
							tic.data = 0;
							memcpy( wktic, &tic, sizeof(TIC_PRM) );
						}
						ticval_kind_dsp((long)kind, 0);						/* 種別 */
						ticval_data_dsp(tic.kind, tic.data, 1);			/* 内容 反転 */
						if (kind == 1) {
							Fun_Dsp(FUNMSG2[4]);		/* "　←　　→　 変更　書込  終了 " */
							data = tic.data;
						}
						else {
							Fun_Dsp(FUNMSG2[11]);	/* "　←　　→　 全て　書込  終了 " */
							data = -1;
						}
					}
					break;
				case KEY_TEN_CL:
					BUZPI();
					kind = (char)tic.kind;
					ticval_kind_dsp((long)kind, 1);					/* 種別 反転 */
					break;
				default:
					break;
				}
			}
			else if (pos == 1) {
			/* ---内容設定中--- */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ← */
					BUZPI();
					ticval_kind_dsp(tic.kind, 1);					/* 種別 反転 */
					ticval_data_dsp(tic.kind, tic.data, 0);			/* 内容 */
					Fun_Dsp( FUNMSG2[10] );			/* "　　　　　　 変更　書込　終了 " */
					pos = 0;
					break;
				case KEY_TEN_F4:	/* 書込 */
					if (data != -1) {
						if (tic.kind == 2) {
							if (data > 100) {
								BUZPIPI();
								ticval_data_dsp(tic.kind, tic.data, 1);			/* 内容 反転 */
								data = -1;
								break;
							}
						}
						tic.data = data;
						OpelogNo = OPLOG_KENKIGEN;			// 操作履歴登録
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						f_ParaUpdate.BIT.tickvalid = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* → */
					BUZPI();
					pos = 2;
					ticval_data_dsp(tic.kind, tic.data, 0);			/* 内容 */
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
					Fun_Dsp(FUNMSG2[2]);			/* "　←　　→　　　　 書込  終了 " */
					data = -1;
					break;
				case KEY_TEN_F3:	/* 変更or全て */
					BUZPI();
					if (tic.kind == 1) {
						if (++data > 15)
							data = 0;
					}
					else {
						data = 0;
					}
					ticval_data_dsp(tic.kind, data, 1);			/* 内容 反転 */
					break;
				case KEY_TEN:
					if (tic.kind != 1) {
						BUZPI();
						if (data == -1)
							data = 0;
						if (tic.kind == 2) {
							data = (data*10 + msg-KEY_TEN0) % 1000;
							opedsp(2, 20, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 内容 反転 */
						}
						else {
							data = (data*10 + msg-KEY_TEN0) % 10000;
							opedsp(2, 20, (ushort)data, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 内容 反転 */
						}
					}
					break;
				case KEY_TEN_CL:
					BUZPI();
					ticval_data_dsp(tic.kind, tic.data, 1);			/* 内容 反転 */
					if (tic.kind == 1) {
						data = tic.data;
					}
					else {
						data = -1;
					}
					break;
				default:
					break;
				}
			}
			else {
			/* ---日付設定中--- */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ← */
					if( (pos%3) == 2 ){
						// ｶｰｿﾙ位置が年入力の場合（入力中の日付から別の入力項目へ移動する場合）
						if( NG == ticval_date_chk( tic.date[(pos-2)/3] ) ){	// 日付ﾃﾞｰﾀﾁｪｯｸ
							BUZPIPI();										// ＮＧ→ｶｰｿﾙ移動しない
							break;
						}
					}
					BUZPI();
					if (--pos < 2) {
						if (tic.kind == 4) {
							/* 種別設定へ */
							pos = 0;
							ticval_kind_dsp(tic.kind, 1);					/* 種別 反転 */
							p = tic.date;
							ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* 日付 */
							Fun_Dsp( FUNMSG2[10] );					/* "　　　　　　 変更　書込　終了 " */
							kind = (char)tic.kind;
						}
						else {
							/* 内容設定へ */
							p = tic.date;
							ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* 日付 */
							ticval_data_dsp(tic.kind, tic.data, 1);			/* 内容 反転 */
							if (tic.kind == 1) {
								Fun_Dsp(FUNMSG2[4]);		/* "　←　　→　 変更　書込  終了 " */
								data = tic.data;
							}
							else {
								Fun_Dsp(FUNMSG2[11]);		/* "　←　　→　 全て　書込  終了 " */
								data = -1;
							}
						}
					}
					else {
						p = tic.date;
						ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
						data = -1;
					}
					break;
				case KEY_TEN_F4:	/* 書込 */
					if (data != -1) {
						if ((pos%3) == 2) {
							tic.date[(pos-2)/3] = data*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + GET_DAY(tic.date[(pos-2)/3]);
						}
						else if ((pos%3) == 0) {
							if ( (data > 12) && (data != 99) ) {
								BUZPIPI();
								p = tic.date;
								ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
								data = -1;
								break;
							}
							tic.date[(pos-2)/3] = GET_YEAR(tic.date[(pos-2)/3])*10000 + data*100 + GET_DAY(tic.date[(pos-2)/3]);
						}
						else {
							if ( (data > 31) && (data != 99) ) {
								BUZPIPI();
								p = tic.date;
								ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
								data = -1;
								break;
							}
							tic.date[(pos-2)/3] = GET_YEAR(tic.date[(pos-2)/3])*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + data;
						}
						OpelogNo = OPLOG_KENKIGEN;			// 操作履歴登録
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						f_ParaUpdate.BIT.tickvalid = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* → */
					if( (pos%3) == 1 ){
						// ｶｰｿﾙ位置が日入力の場合（入力中の日付から別の入力項目へ移動する場合）
						if( NG == ticval_date_chk( tic.date[(pos-2)/3] ) ){	// 日付ﾃﾞｰﾀﾁｪｯｸ
							BUZPIPI();										// ＮＧ→ｶｰｿﾙ移動しない
							break;
						}
					}
					BUZPI();
					if (++pos > 13)
						pos = 2;
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
					data = -1;
					break;
				case KEY_TEN:
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 100;
					if ((pos%3) == 2)
						tmp = data*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + GET_DAY(tic.date[(pos-2)/3]);
					else if ((pos%3) == 0)
						tmp = GET_YEAR(tic.date[(pos-2)/3])*10000 + data*100 + GET_DAY(tic.date[(pos-2)/3]);
					else
						tmp = GET_YEAR(tic.date[(pos-2)/3])*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + data;
					p = tic.date;
					if (pos < 5)
						ticval_date_dsp(tmp, p[1], p[2], p[3], pos);	/* 日付 */
					else if (pos < 8)
						ticval_date_dsp(p[0], tmp, p[2], p[3], pos);	/* 日付 */
					else if (pos < 11)
						ticval_date_dsp(p[0], p[1], tmp, p[3], pos);	/* 日付 */
					else
						ticval_date_dsp(p[0], p[1], p[2], tmp, pos);	/* 日付 */
					break;
				case KEY_TEN_CL:
					BUZPI();
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* 日付 */
					data = -1;
					break;
				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  種別表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_kind_dsp( kind, rev )                            |*/
/*| PARAMETER    : long   kind : 種別                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_kind_dsp(long kind, ushort rev)
{
	const uchar	*p;
	ushort	len;

	grachr(2, 8, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);
	switch (kind) {
	case 0:
		p = DAT2_6[9];
		len = 4;
		break;
	case 1:
		p = DAT5_3[4];
		len = 10;
		break;
	case 2:
		p = DAT3_3[5];
		len = 6;
		break;
	case 3:
		p = DAT3_3[6];
		len = 6;
		break;
	default:
		p = DAT2_6[8];
		len = 4;
		break;
	}
	grachr(2, 8, len, rev, COLOR_BLACK, LCD_BLINK_OFF, p);
}

/*[]----------------------------------------------------------------------[]*/
/*|  内容表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_data_dsp( kind, data, rev )                      |*/
/*| PARAMETER    : long   kind : 種別                                      |*/
/*|              : long   data : 内容                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_data_dsp(long kind, long data, ushort rev)
{
	grachr(2, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);
	if (kind != 0 && kind != 4) {
		if (data == 0)
			grachr(2, 22, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);	/* "全て" */
		else {
			if (kind == 1)
				grachr(2, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data-1]);
			else if (kind == 2)
				opedsp(2, 20, (ushort)data, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
			else
				opedsp(2, 20, (ushort)data, 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  日付表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_date_dsp( d1, d2, d3, d4, rev_pos)               |*/
/*| PARAMETER    : long d1      : 変更前開始日時                           |*/
/*|              : long d2      : 変更前終了日時                           |*/
/*|              : long d3      : 変更後開始日時                           |*/
/*|              : long d4      : 変更後終了日時                           |*/
/*|              : char rev_pos : 反転位置                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_date_dsp(long d1, long d2, long d3, long d4, char rev_pos)
{

	opedsp(3, 12, (ushort)GET_YEAR(d1), 2, 1, ((rev_pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(3, 18, (ushort)GET_MONTH(d1), 2, 0, ((rev_pos==3)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(3, 24, (ushort)GET_DAY(d1), 2, 0, ((rev_pos==4)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(4, 12, (ushort)GET_YEAR(d2), 2, 1, ((rev_pos==5)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(4, 18, (ushort)GET_MONTH(d2), 2, 0, ((rev_pos==6)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(4, 24, (ushort)GET_DAY(d2), 2, 0, ((rev_pos==7)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(5, 12, (ushort)GET_YEAR(d3), 2, 1, ((rev_pos==8)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(5, 18, (ushort)GET_MONTH(d3), 2, 0, ((rev_pos==9)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(5, 24, (ushort)GET_DAY(d3), 2, 0, ((rev_pos==10)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(6, 12, (ushort)GET_YEAR(d4), 2, 1, ((rev_pos==11)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(6, 18, (ushort)GET_MONTH(d4), 2, 0, ((rev_pos==12)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(6, 24, (ushort)GET_DAY(d4), 2, 0, ((rev_pos==13)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
}

/*[]----------------------------------------------------------------------[]*/
/*|  日付チェック                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_date_chk( date )                                 |*/
/*| PARAMETER    : long date     : 年月日(yymmdd)                          |*/
/*|                                                                        |*/
/*| RETURN VALUE : OK/NG                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static uchar ticval_date_chk( long date )
{
	short	year;			// 年
	short	mon;			// 月
	short	day;			// 日

	if( (date == 0) || (date == 999999) ){
		return( OK );						// 年月日全て０の場合ＯＫ
	}

	year	= (short)GET_YEAR(date);		// 年ﾃﾞｰﾀ取得
	mon		= (short)GET_MONTH(date);		// 月ﾃﾞｰﾀ取得
	day		= (short)GET_DAY(date);			// 日ﾃﾞｰﾀ取得

	if( mon == 0 || day == 0 ){
		return( NG );						// 月または日が０の場合ＮＧ

	}
	if( year >= 80 ){						// 年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
		year += 1900;
	}else{
		year += 2000;
	}

	if( 0 != chkdate( year, mon, day ) ){	// 存在する日かﾁｪｯｸ
		return( NG );
	}
	return( OK );
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：営業開始時刻                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_BusyTime( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_BusyTime(void)
{
	ushort	msg;
	char	changing;	/* 1:時刻設定中 */
	char	pos;		/* 0:開始時刻 1:終了時刻 */
	long	input;
	long	*ptim;

	ptim = &CPrmSS[S_SYS][40];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[31]);		/* "＜営業開始時刻＞　　　　　　　" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);			/* "開始時刻 */
	bsytime_dsp(0, ptim[0], 0);				/* 時刻 正転 */
	grachr(3, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);			/* "終了時刻" */
	bsytime_dsp(1, ptim[1], 0);				/* 時刻 正転 */
	Fun_Dsp( FUNMSG2[0] );					/* "　　　　　　 変更　　　　終了 " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了(F5) */
			BUZPI();
			if (changing == 0)
				return MOD_EXT;
			bsytime_dsp(pos, ptim[pos], 0);		/* 時刻 正転 */
			Fun_Dsp( FUNMSG2[0] );				/* "　　　　　　 変更　　　　終了 " */
			changing = 0;
			break;
		case KEY_TEN_F3:	/* 変更(F3) */
			if (changing == 1)
				break;
			BUZPI();
			bsytime_dsp(0, ptim[0], 1);		/* 時刻 反転 */
			Fun_Dsp( FUNMSG[20] );			/* "　▲　　▼　　　　 書込  終了 " */
			input = -1;
			changing = 1;
			pos = 0;
			break;
		case KEY_TEN_F4:	/* 書込(F4) */
			if (changing == 0)
				break;
			if (input != -1) {
				if (input%100 > 59 || input > 2359) {
					BUZPIPI();
					bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
					input = -1;
					break;
				}
				ptim[pos] = input;
				OpelogNo = OPLOG_EIGYOKAISHI;			// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTP更新フラグ
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			}
		case KEY_TEN_F1:	/* ▲(F1) */
		case KEY_TEN_F2:	/* ▼(F2) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 0);		/* 時刻 正転 */
			pos ^= 1;
			bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
			input = -1;
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			if (input == -1)
				input = 0;
			input = (input*10 + msg-KEY_TEN0) % 10000;
			bsytime_dsp(pos, input, 1);			/* 時刻 反転 */
			break;
		case KEY_TEN_CL:	/* 取消(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  営業時間表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( pos )                                       |*/
/*| PARAMETER    : char   pos  : 表示位置                                  |*/
/*|              : long   time : 時間                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	bsytime_dsp(char pos, long time, ushort rev)
{
	opedsp((ushort)(2+pos), 14, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
	grachr((ushort)(2+pos), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
	opedsp((ushort)(2+pos), 20, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：料金設定                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Mnysetmenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Mnysetmenu(void)
{
	unsigned short	usUmnyEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0] );		/* [00]	"＜料金設定＞　　　　　　　　　" */

		usUmnyEvent = Menu_Slt( MNYSETMENU, USM_MNS_TBL, (char)USM_MNS_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){

			/* 最大料金 */
			case MNT_MNYLIM:
				usUmnyEvent = UsMnt_mnylim();
				break;
			/* サービス券役割 */
			case MNT_MNYTIC:
				usUmnyEvent = UsMnt_mnytic();
				break;
			/* 店役割 */
			case MNT_MNYSHP:
				usUmnyEvent = UsMnt_mnyshp();
				break;
			/* 消費税の税率 */
			case MNT_MNYTAX:
				usUmnyEvent = UsMnt_mnytax();
				break;
			/* Ａ～Ｌ種役割 */
			case MNT_MNYKID:
				usUmnyEvent = UsMnt_mnykid();
				break;
			/* 単位時間／料金 */
			case MNT_MNYCHA:
				if (CPrmSS[S_CAL][1] != 1) {
					usUmnyEvent = UsMnt_mnycha();
				} else {
					usUmnyEvent = UsMnt_mnycha_dec();
				}
				break;
			/* 定期無効曜日 */
			case MNT_MNYTWK:
				usUmnyEvent = UsMnt_mnytweek();
				break;
			/* プリント */
			case MNT_PRISET:
				if (Ope_isPrinterReady() == 0) {	// レシート出力不可
					BUZPIPI();
					break;
				}
#if (5 == AUTO_PAYMENT_PROGRAM)
				if(TENKEY_CL == 1){
					usUmnyEvent = UsMnt_mc10print();
					break;
				}
#endif
				usUmnyEvent = UsMnt_mnyprint();
				break;

			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return MOD_EXT;
			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
			SetChange = 1;			// FLASHｾｰﾌﾞ指示
			UserMnt_SysParaUpdateCheck( OpelogNo );
			SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTP更新フラグ
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
		}

		if( usUmnyEvent == MOD_CHG || usUmnyEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}
		if (usUmnyEvent == MOD_CHG){
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (usUmnyEvent == MOD_CUT) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：最大料金回数設定アドレス取得                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_mnylim_kaisuu_adr(char, short*, char*, char*)       |*/
/*| PARAMETER    : char : 最大タイプの設定値                               |*/
/*|                short* : 共通パラメータのアドレス                       |*/
/*|                char* : 設定値の桁数                                    |*/
/*|                char* : 設定値の最下位桁位置                            |*/
/*| RETURN VALUE : void                                                    |*/
/*| 補足         : 28-0002の設定値を取得し、それぞれのアドレスを返す       |*/
/*| 							  10：新最大時間・回数のアドレス           |*/
/*| 						上記以外：従来最大時間・回数のアドレス         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void get_mnylim_kaisuu_adr(char type, char kind, short* adr, char* len, char* pos)
{
	if(!RYO_CAL_EXTEND){
		switch(type)		/* 28-0002から取得した値 */
		{
		default:			/* 標準 */
		case SP_MX_N_HOUR:	/* ｎ時間ｍ回最大(旧) */
			*adr = 14;		/* アドレス */
			*len = 6;		/* 桁数 */
			*pos = 1;		/* 最下位桁位置 */
			break;
		case SP_MX_N_MH_NEW:/* ｎ時間ｍ回最大(新) */
			*adr = 38;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 1;		/* 最下位桁位置 */
			break;
		}
	}
	else{
		switch(type)		/* 56-0020から取得した値 */
		{
		default:			/* 標準 */
		case SP_MX_N_HOUR:		/* ｎ時間ｍ回最大(旧) */
		case SP_MX_N_MH_NEW:	/* ｎ時間ｍ回最大(新) */
			*adr = 32+kind;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 1;		/* 最下位桁位置 */
			break;
		}
	}
		
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：最大料金時間設定アドレス取得                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_mnylim_zikann_adr(char, short*, char*, char*)       |*/
/*| PARAMETER    : char : 最大タイプの設定値                               |*/
/*|                short* : 共通パラメータのアドレス                       |*/
/*|                char* : 設定値の桁数                                    |*/
/*|                char* : 設定値の最下位桁位置                            |*/
/*| RETURN VALUE : void                                                    |*/
/*| 補足         : 28-0002の設定値を取得し、それぞれのアドレスを返す       |*/
/*| 							  10：新最大時間・回数のアドレス           |*/
/*| 						上記以外：従来最大時間・回数のアドレス         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void get_mnylim_zikann_adr(char type, char kind, short* adr, char* len, char* pos)
{
	if(!RYO_CAL_EXTEND){
		switch(type)		/* 28-0002から取得した値 */
		{
		default:			/* 標準 */
		case SP_MX_N_HOUR:
			*adr = 13;		/* アドレス */
			*len = 6;		/* 桁数 */
			*pos = 1;		/* 最下位桁位置 */
			break;
		case SP_MX_N_MH_NEW:
			*adr = 38;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 3;		/* 最下位桁位置 */
			break;
		case SP_MX_N_MHOUR_W:
			*adr = 41;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 3;		/* 最下位桁位置 */
			break;
		}
	}
	else{
		switch(type)		/* 56-0020から取得した値 */
		{
		default:				/* 標準 */
		case SP_MX_N_HOUR:		/* ｎ時間ｍ回最大(旧) */
		case SP_MX_N_MH_NEW:	/* ｎ時間ｍ回最大(新) */
			*adr = 32+kind;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 3;		/* 最下位桁位置 */
			break;
		case SP_MX_N_MHOUR_W:
			*adr = 20+kind;		/* アドレス */
			*len = 2;		/* 桁数 */
			*pos = 3;		/* 最下位桁位置 */
			break;
		}
	}
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*|  種別毎の最大料金タイプ取得                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_LIM_TYP( )  									       |*/
/*| PARAMETER    : kind    :種別										   |*/
/*|              : max_set :最下行の位置                                   |*/
/*|              : param_pos: パラメータ位置							   |*/
/*| RETURN VALUE : type    :料金種別                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char GET_LIM_TYP( char kind, char* max_set, ushort* param_pos ){
	
	char type;
	if(RYO_CAL_EXTEND){					// 料金計算拡張有り
		type = (char)prm_get(COM_PRM, S_CLX, (short)(2+(kind/3)), 2, (char)(6-((kind%3+1)*2)+1) );		// 種別毎の最大料金タイプ取得
	}
	else {
		type = (char)CPrmSS[S_CAL][2];						// 最大料金タイプ取得
	}

	switch( type ){
		case 2:		// 時刻指定日毎最大
		case 3:		// ２種時刻指定日毎最大
		case 5:		// 時間帯最大
		case 10:	// ｎ時間ｍ回最大
		case 20:	// Ｗ上限
			break;

		default:
			type = 0;	// 上記以外は最大なしで動作する。
			break;
	}

	switch(type){
	case SP_MX_NON:					// 日毎最大料金無し
	default:
		*max_set = 2;
		*param_pos = 1;
		break;
	case SP_MX_INTIME:				// 入庫時刻最大
	case SP_MX_TIME:				// 時刻指定最大
		*max_set = 3;
		*param_pos = 1;
		break;
	case SP_MX_WTIME:				// ２種類の時刻指定最大
	case SP_MX_N_HOUR:				// ｎ時間ｍ回最大(旧設定)
	case SP_MX_N_MH_NEW:			// ｎ時間ｍ回最大
		*max_set = 4;
		*param_pos = 1;
		break;
	case SP_MX_N_MHOUR_W:			// ２種類のｎ時間ｍ回最大
		*max_set = 5;
		*param_pos = 1;
		break;
	case SP_MX_BAND:				// 時間帯最大
		*max_set = 8;
		*param_pos = 15;
		break;
	}
	return type;
}
/*[]----------------------------------------------------------------------[]*/
/*|  最大料金タイプ毎の画面表示                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnylim_type_disp( )  							       |*/
/*| PARAMETER    : type    :最大料金タイプ								   |*/
/*| PARAMETER    : top_pos :料金帯の最上表示番号						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void mnylim_type_disp( char type , char top_pos )
{

	if (type == SP_MX_INTIME) {				// 入庫時刻最大(6/12/24)
		if( CPrmSS[S_CAL][13] == 6 ){
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[67] );	/* "  ６時間最大：　　　　　　０円" */
		}else if( CPrmSS[S_CAL][13] == 12 ){
			grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[50] );	/* "１２時間最大：　　　　　　０円" */
		}else{
			grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[9] );	/* "２４時間最大：　　　　　　０円" */
		}
	}
	else if (type == SP_MX_TIME) {			// 時刻指定日毎最大
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[8] );		/* "日毎最大料金：　　　　　　０円" */
	}
	else if (type == SP_MX_WTIME){			// ２種類の時刻指定日毎最大
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[10] );	/* "日毎最大料金１：　　　　　０円" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[11] );	/* "日毎最大料金２：　　　　　０円" */
	}
	else if (type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW){				// ｎ時間ｍ回最大
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[150] );	/* "　　時間　　：　　　　　　０円" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[153] );	/* "繰り返し回数：　　　　　　　回" */
	}
	else if (type == SP_MX_BAND){			// 時間帯最大
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[140 + top_pos ] );	/* "　第１料金帯：　　　　	　０円" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[141 + top_pos ] );	/* "　第２料金帯：　　	　　　０円" */
		grachr(5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[142 + top_pos ] );	/* "　第３料金帯：	　　　　　０円" */
		grachr(6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[143 + top_pos ] );	/* "　第４料金帯：	　　　　　０円" */
	}
	else if(type == SP_MX_N_MHOUR_W){		// ２種類のｎ時間ｍ回最大
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[151] );	/* "　　時間(１)：　　　　　　０円" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[152] );	/* "　　時間(２)：　　　　　　０円" */
		grachr(5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[153] );	/* "繰り返し回数：　　　　　　　回" */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  パラメータ上の絶対最大料金のINDEXを取得する                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_LIM_IDX(char n, char k)                             |*/
/*| PARAMETER    : n  料金体系                                             |*/
/*|              : k  種別                                                 |*/
/*| RETURN VALUE : index  絶対最大料金のINDEX                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short GET_LIM_IDX(char n, char k){
	short	index;
	short	wk;
	
	wk = (n%3 == 0) ? 3 : n%3;
	/* (体系*300 + 61(先頭アドレス))  +  (種別*20) */
	index = (RYO_TAIKEI_SETCNT*((wk)-1)+61)+(k*RYO_SYUBET_SETCNT);
	
	return index;
	
}
/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：最大料金                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnylim( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*| 補足         : 最大料金タイプ  0：日毎最大料金なし                     |*/
/*| 							   1：入出庫時間最大(6/12/24)              |*/
/*| 							   2：時刻指定日毎最大		               |*/
/*| 							   3：2種類の時刻指定日毎最大              |*/
/*| 							   4：ｎ時間ｍ回最大		               |*/
/*| 							   5：時間帯最大			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
///* パラメータ上の絶対最大料金のINDEXを取得するマクロ */
unsigned short	UsMnt_mnylim(void)
{
	ushort	msg;
	char	set;		/* 0:体系設定中 1:種別設定中 2-6:金額設定中 7:時間回設定中*/
	char	max_set;
	char	no;			/* 体系(1-3) */
	int		kind;		/* 種別(A-L) */
	int		type;		/* 日毎最大料金タイプ */
	long	money;
	char	calc;		/* 料金計算方式(0:昼夜帯 1:逓減帯) */
	ushort	line_pos;				/* 表示位置 */
	ushort	param_pos;				/* 共通パラメータ位置 */
	long	time_count;				/* 時間・回 */
	short	adr_k, adr_z;			/* ｱﾄﾞﾚｽNo.						*/
	char	pos_k, pos_z;			/* 桁位置						*/
	long	*pRAT;					/* 共通パラメータへのポインタ */
	char	seg;					/* セクションNo.() */
	short	lim_sec;				/* セクションNo.(Ｗ上限)*/
	short	adr_z2;					/* ｱﾄﾞﾚｽNo.(Ｗ上限)*/
	char	pos_z2;
	char	len_z, len_z2, len_k;
	short	para_idx;
	char	top_m_pos = 0;			/* 料金帯の最上表示番号(時間帯最大のみ使用) 0:第1料金帯 */

	/* 初期は第１料金体系 */
	pRAT = CPrmSS[S_RAT];
	seg = S_RAT;
	no = 1;
	kind = 0;
	type = GET_LIM_TYP(kind, &max_set, &param_pos);			// 最大料金タイプとmax_set取得
	calc = (char)CPrmSS[S_CAL][1];
	if (calc) {
		/* 逓減帯の時は"２種の日毎最大"は認めていない("時刻指定日毎最大"とする) */
		if (type < 0 || type  > 2)
			type = 2;
	}
	if(!RYO_CAL_EXTEND){				// 料金計算拡張あり？
		lim_sec = S_CAL;									// 最大料金の時間・回数の設定位置
	}
	else {
		lim_sec = S_CLX;
	}
	if(type == SP_MX_N_MHOUR_W){
		get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);			// 繰り返し回数の設定取得
		get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);			// 最大１の設定取得
		get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);		// 最大２の設定取得
	}
	else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){
		get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);			// 繰り返し回数の設定取得
		get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);		// 最大時間の設定取得
	}
	time_count = -1;							// 時間/回数(入力値)

	dispclr();

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[1] );			/* "＜最大料金＞　　　　　　　　　" */
	if (calc == 0) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[14]);		/* "（昼夜帯）" */
	}
	else {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[15]);		/* "（逓減帯）" */
	}

	grachr( 1, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );		/* "第　料金体系" 反転 */
	opedsp( 1, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 体系(1-3)      反転 */
	grachr( 1, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0] );		/* "－" */
	grachr( 1, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L) */
	grachr( 1, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );		/* "種" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[7] );		/* "絶対最大料金：　　　　　　０円" */
	mnylim_type_disp( type ,top_m_pos);		/* 最大料金タイプ毎の表示 */
	line_pos = 1;							/* 画面表示開始位置 */

	mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
	Fun_Dsp( FUNMSG[25] );						/* "　▲　　▼　　　　 読出  終了 " */

	set = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( msg ) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F3:					/* 時間回(F3) */
			/* ﾀｲﾌﾟが4かつ、最大料金編集中か、時間回を編集中の場合 */						
			if(((type == SP_MX_N_MHOUR_W) && (set == 2 || set == 3 || set == 4 || set == 5)) ||		// 料金・回数設定中
				(type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW )&& (set == 2 || set == 3 || set == 4)){
				if(type == SP_MX_N_MHOUR_W){							// ２種類のｎ時間ｍ回最大
					Fun_Dsp( FUNMSG2[45] );				/* "　⊂　　⊃　最大２ 書込  終了 " */
				}
				else {
					Fun_Dsp( FUNMSG2[2] );				/* "　⊂　　⊃　       書込  終了 " */
				}
				time_count = -1;			/* 時間回設定領域を初期化 */						
				set = 7;					/* 状態を時間回編集ﾓｰﾄﾞに設定 */
				mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
				BUZPI();
				break;
			}
		default:
			/* 料金体系 設定 */
			if (set == 0) {
				switch ( msg ) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					no--;
					switch(no){
						case 0:
							no = 9;			// 第１→第９料金体系
							seg = S_CLB;		// セクション59
							break;
						case 3:				// 第４→第３料金体系
							seg = S_RAT;		// セクション31
							break;
						case 6:				// 第７→第６料金体系
							seg = S_CLA;		// セクション58
							break;
						default:
							break;
					}
					pRAT = CPrmSS[seg];		// 設定位置更新
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* 体系(1-3) 反転 */
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
					break;
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					no++;
					switch(no){
					case 4:				// 第３→第４料金体系
						seg = S_CLA;		// セクション58
						break;
					case 7:				// 第６→第７料金体系
						seg = S_CLB;		// セクション59
						break;
					case 10:			
						no = 1;			// 第９→第１料金体系
						seg = S_RAT;		// セクション31
						break;
					default:
						break;
					}
					pRAT = CPrmSS[seg];	// 設定位置更新
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 体系(1-3) 反転 */
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr( line_pos, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );	/* "第　料金体系" 正転 */
					opedsp( line_pos, 2, (ushort)no, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 体系(1-3)      正転 */
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      反転 */
					grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "種"           反転 */
					Fun_Dsp( FUNMSG[28] );						/* "  ▲    ▼    ←   読出  終了 " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* 種別(A-L) 設定 */
			else if (set == 1) {
				switch ( msg ) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					if (--kind < 0)
						kind = 11;
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      反転 */
					if(RYO_CAL_EXTEND){								// 料金計算拡張あり
						type = GET_LIM_TYP( kind ,&max_set, &param_pos);				// 種別毎の最大料金タイプ取得
						if(type == SP_MX_N_MHOUR_W){													// ２種類のｎ時間ｍ回最大
							get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);		// 最大１の設定取得
							get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);		// 回数の設定取得
							get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);	// 最大２の設定取得
						}
						else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){						// ｎ時間ｍ回最大
							get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);					// 回数の設定取得
							get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);				// 最大時間の設定取得
						}
					}
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
					break;
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (++kind > 11)
						kind = 0;
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      反転 */
					if(RYO_CAL_EXTEND){								// 料金計算拡張あり
						type = GET_LIM_TYP( kind ,&max_set, &param_pos);				// 種別毎の最大料金タイプ取得
						if(type == SP_MX_N_MHOUR_W){													// ２種類のｎ時間ｍ回最大
							get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);		// 最大時間１の設定取得
							get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);		// 回数の設定取得
							get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);	// 最大時間２の設定取得
						}
						else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){						// ｎ時間ｍ回最大
							get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);					// 回数の設定取得
							get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);				// 最大時間の設定取得
						}
					}
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
					break;
				case KEY_TEN_F3:	/* ←(F3) */
					BUZPI();
					grachr( line_pos, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );	/* "第　料金体系" 反転 */
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 体系(1-3)      反転 */
					grachr( line_pos, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      正転 */
					grachr( line_pos, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "種"           正転 */
					Fun_Dsp( FUNMSG[25] );						/* "　▲　　▼　　　　 読出  終了 " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr( line_pos, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      正転 */
					grachr( line_pos, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "種"           正転 */
					if( type == SP_MX_BAND ){		// 時間帯上限
						Fun_Dsp( FUNMSG[20] );					/* "　▲　　▼　　　　 書込  終了 " */
					} else if(type != SP_MX_N_HOUR && type != SP_MX_N_MH_NEW && type != SP_MX_N_MHOUR_W){	/* ﾀｲﾌﾟが４・１０・２０以外だった場合 */
						Fun_Dsp( FUNMSG2[2] );					/* "　←　　→　　　　 書込  終了 " */
					}else{
						Fun_Dsp( FUNMSG2[40] );					/* "　←　　→　時間　 書込  終了 " */
					}
					mnylim_dsp(type, no, kind, 0 ,top_m_pos);				/* 金額表示 */
					money = -1;
					set = 2;
					break;
				default:
					break;
				}
			}
			else if(set == 7){									/* 時間回編集モード */
				if( type == SP_MX_BAND ){	// 時間帯上限
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F1:	/* ←(F1) */
						--set;
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
						money = -1;
						break;
					case KEY_TEN_F4:	/* 書込(F4) */
						para_idx = 0;
						if (money != -1) {
							para_idx = GET_LIM_IDX(no, kind)+(param_pos+4);	/* 時間帯最大第５料金帯 */
							pRAT[para_idx] = money*10;
							OpelogNo = OPLOG_SAIDAIRYOKIN;			// 操作履歴登録
							f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
							mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
						}
					case KEY_TEN_F2:	/* →(F2) */
						++set;
						if( 1 == top_m_pos && 8 == set ){	// 最上表示が第2料金帯でsetが8になったとき、最上表示は第3料金帯にする
							top_m_pos++;
						}
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
						money = -1;
						break;
					case KEY_TEN:		/* 数字(テンキー) */
						BUZPI();
						if (money == -1)
							money = 0;

						money = (money*10 + (msg-KEY_TEN0)) % 100000L;
						opedpl( (ushort)((line_pos*2-2 - top_m_pos ) + set), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 料金 反転 */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2 - top_m_pos ) + set), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
						break;
					case KEY_TEN_CL:	/* 取消(テンキー) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* 金額表示 */
						money = -1;
						time_count = -1;
						break;
					default:
						break;
					}
				} else {
					switch ( KEY_TEN0to9(msg) ) {

					case KEY_TEN_F4:								/* 書込(F4) */
						OpelogNo = OPLOG_SAIDAIRYOKIN;				// 操作履歴登録
					case KEY_TEN_F1:
					case KEY_TEN_F2:
					case KEY_TEN_F3:
						if(msg == KEY_TEN_F3 && type != SP_MX_N_MHOUR_W){
							break;
						}
						if( time_count != -1){				/* ﾃﾝｷｰでの入力がある場合 */	
							if( time_count == 0 || time_count > 24 ||					/* 入力値が０、２４以上の場合 */
								(type == SP_MX_N_MHOUR_W && msg == KEY_TEN_F4 && time_count >= prm_get(COM_PRM, lim_sec, adr_z, len_z, pos_z)) ){
								/* Ｗ上限では最大１＜最大２となるように設定する */
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
								time_count = -1;										/* 時間回入力領域を初期化 */
								break;
							}
							if( msg == KEY_TEN_F4 ){		
								prm_set(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2, time_count);
								f_ParaUpdate.BIT.other = 1;								// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
								mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
							}
						}else{																					// 入力がない場合
							if( msg != KEY_TEN_F3 && prm_get(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2) == 0){	// 最大２の設定が０の時はエラー(Ｆ３のみカーソル移動可)
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
								time_count = -1;										/* 時間回入力領域を初期化 */
								BUZPIPI();					
								break;
							}
						}	
						BUZPI();
						if(msg == KEY_TEN_F3 && type == SP_MX_N_MHOUR_W){				/* Ｗ上限 */
							set = 8;										/* 最大２の設定へ */
							Fun_Dsp( FUNMSG2[44] );							// "　⊂　　⊃　上限１ 書込  終了 "
						}
						else{
							set = 3;										/* 料金設定へ */
							Fun_Dsp( FUNMSG2[40] );							// "　⊂　　⊃　 時間  書込  終了 "
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
						time_count = -1;										/* 時間回入力領域を初期化 */
						break;
					case KEY_TEN:		/* 数字(テンキー) */
						BUZPI();
						if (time_count == -1){				/* ﾃﾝｷｰでの入力がない場合 */
							time_count = 0;					/* 入力領域の初期化 */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* ﾃﾝｷｰ入力値の取得 */
						if( time_count == 0 ){				/* 入力値が0の場合 */
							grachr( (ushort)(line_pos*2+1), 0, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
						opedpl( (ushort)(line_pos*2+1), 0, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 入力値 反転 */
						break;
					case KEY_TEN_CL:	/* 取消(テンキー) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* 金額表示 */
						time_count = -1;
						money = -1;
						break;
					default:
						break;
					}
				}
			}
			else if(set == 8){
				if( type == SP_MX_BAND ){	// 時間帯上限
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F1:	/* ←(F1) */
						--set;
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
						money = -1;
						break;
					case KEY_TEN_F4:	/* 書込(F4) */
						para_idx = 0;
						if (money != -1) {
							BUZPI();
							para_idx = GET_LIM_IDX(no, kind)+(param_pos-13);	/* 時間帯最大第５料金帯 */
							pRAT[para_idx] = money*10;
							OpelogNo = OPLOG_SAIDAIRYOKIN;			// 操作履歴登録
							f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
							mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
						}
						break;
					case KEY_TEN_F2:	/* →(F2) */
						BUZPIPI();	// 第6料金帯までしか遷移させない
						break;
					case KEY_TEN:		/* 数字(テンキー) */
						BUZPI();
						if (money == -1)
							money = 0;

						money = (money*10 + (msg-KEY_TEN0)) % 100000L;
						opedpl( (ushort)((line_pos*2-2) + set - top_m_pos ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 料金 反転 */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set - top_m_pos ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
						break;
					case KEY_TEN_CL:	/* 取消(テンキー) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* 金額表示 */
						money = -1;
						time_count = -1;
						break;
					default:
						break;
					}
				} else {
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F4:								/* 書込(F4) */
						OpelogNo = OPLOG_SAIDAIRYOKIN;				// 操作履歴登録
					case KEY_TEN_F1:								/* ←(F1) */
					case KEY_TEN_F2:								/* ←(F2) */
					case KEY_TEN_F3:								/* ←(F3) */
						if(time_count != -1){
							if( time_count == 0 || time_count > 24 ||		 				/* 入力値が0 */
								(msg == KEY_TEN_F4 && time_count <= prm_get(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2)) ){
								/* Ｗ上限では最大１＜最大２となるように設定する */
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
								time_count = -1;										/* 時間回入力領域を初期化 */
								break;
							}
							if( msg == KEY_TEN_F4 ){		
								prm_set(COM_PRM, lim_sec, adr_z, len_z, pos_z, time_count);
								f_ParaUpdate.BIT.other = 1;								// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
								mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
							}
						}else{	/* 時間の初期状態が0かつ時間設定中の場合 */
							if( msg != KEY_TEN_F3  && (prm_get(COM_PRM, lim_sec, adr_z, len_z, pos_z) == 0)){
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
								time_count = -1;										/* 時間回入力領域を初期化 */
								break;
							}	
						}
						BUZPI();
						if(msg == KEY_TEN_F3){							
							set = 7;										// 最大１の設定へ
							Fun_Dsp( FUNMSG2[45] );							// "　⊂　　⊃　上限２ 書込  終了 "
						}
						else{
							set = 4;										// 料金設定へ
							Fun_Dsp( FUNMSG2[40] );							// "　⊂　　⊃　 時間  書込  終了 "
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 画面表示変更 */
						time_count = -1;										/* 時間回入力領域を初期化 */
						money = -1;												/* 時間回料金入力領域を初期化 */
						break;
					case KEY_TEN:		/* 数字(テンキー) */
						BUZPI();
						if (time_count == -1){				/* ﾃﾝｷｰでの入力がない場合 */
							time_count = 0;					/* 入力領域の初期化 */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* ﾃﾝｷｰ入力値の取得 */
						if( time_count == 0 ){				/* 入力値が0の場合 */
							grachr( (ushort)(line_pos*2+2), 0, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
						opedpl( (ushort)(line_pos*2+2), 0, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 入力値 反転 */
						break;
					case KEY_TEN_CL:	/* 取消(テンキー) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* 金額表示 */
						time_count = -1;
						money = -1;
						break;
					default:
						break;
					}
				}
			}
			/* 最大料金 設定 */
			else {
				switch ( KEY_TEN0to9(msg) ) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					if (--set < 2) {
						/* 種別(A-L) 設定へ */
						grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      反転 */
						grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "種"           反転 */
						mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
						Fun_Dsp( FUNMSG[28] );						/* "  ▲    ▼    ←   読出  終了 " */
					}
					else {
						if( type == SP_MX_BAND ){	// 時間帯上限
							// 最上表示が第2料金帯でsetが3になったとき、最上表示は第1料金帯にする
							// 最上表示が第3料金帯でsetが4になったとき、最上表示は第2料金帯にする
							if( ( 1 == top_m_pos && 3 == set ) || ( 2 == top_m_pos && 4 == set ) ){
								top_m_pos--;
							}
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
						money = -1;
					}
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					para_idx = 0;
					if(type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW || type == SP_MX_N_MHOUR_W){			// ｎ時間ｍ回設定
						switch(set){
						case 2:										/* 絶対最大料金 */
							if(money != -1){						// 入力がない場合
								para_idx = GET_LIM_IDX(no, kind);
							}
							break;
						case 3:
							if(money != -1){					// 入力がある場合
								if(type == SP_MX_N_MHOUR_W && money >= pRAT[GET_LIM_IDX(no, kind)+param_pos]/10){
									/* Ｗ上限では最大１＜最大２となるように設定する */
									BUZPIPI();
									mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
									money = -1;
									para_idx = -1;
									break;
								}
								else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){			// ｎ時間ｍ回最大の最大料金
									para_idx = GET_LIM_IDX(no, kind)+param_pos;
								}
								else{
									para_idx = GET_LIM_IDX(no, kind)+param_pos+1;
								}
							}
							break;
						case 4:	
							if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR ){				// ｎ時間ｍ回最大のときは回数の設定
								if( time_count != -1 ){		// 回数の入力がある場合
									prm_set(COM_PRM, lim_sec, adr_k, len_k, pos_k, time_count);
								}
								break;
							}
							else{						// Ｗ上限では最大２最大料金の設定
								if(money != -1){		// 入力がある場合
									if(money <= pRAT[GET_LIM_IDX(no, kind)+param_pos+1]/10){
										/* Ｗ上限では最大１＜最大２となるように設定する */
										BUZPIPI();
										mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
										money = -1;
										para_idx = -1;
										break;
									}
									else{
										para_idx = GET_LIM_IDX(no, kind)+(param_pos);
									}
								}
							}
							break;
						case 5:			// Ｗ上限、回数の設定
							if( time_count != -1 ){		// 回数の入力がある場合
								prm_set(COM_PRM, lim_sec, adr_k, len_k, pos_k, time_count);
							}
							break;
						}
						if(para_idx == -1){			// 入力値エラー
							break;
						}
						else if(para_idx != 0){				// 金額の入力がある場合
							pRAT[para_idx] = money*10;
						}
						OpelogNo = OPLOG_SAIDAIRYOKIN;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
						time_count = -1;										/* 時間回入力領域を初期化 */
					}
					else
					if (money != -1) {

						switch(set){
							case 2:							/* 絶対最大料金 */
								para_idx = GET_LIM_IDX(no, kind);
								break;
							case 3:							/* 日毎・24時間最大料金・日毎最大料金２・時間帯最大第１料金帯 */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos);
								break;
							case 4:							/* 日毎・24時間最大料金・ｎ時間ｍ回最大料金・時間帯最大第２料金帯 */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+1);
								break;
							case 5:							/* 時間帯最大第３料金帯・ｎ時間ｍ回最大料金２ */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+2);
								break;
							case 6:							/* 時間帯最大第４料金帯 */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+3);
								break;
						}
						pRAT[para_idx] = money*10;
						OpelogNo = OPLOG_SAIDAIRYOKIN;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					if( type == SP_MX_BAND ){	// 時間帯上限
						if (++set > max_set) {
							BUZPIPI();	// 第6料金帯までしか遷移させない
						} else {
							if( 0 == top_m_pos && 7 == set ){	// 最上表示が第1料金帯でsetが7になったとき、最上表示は第2料金帯にする
								top_m_pos++;
							}
							BUZPI();
							mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
							money = -1;
						}
					} else {					// 時間帯上限以外
						BUZPI();
						if (++set > max_set) {
							/* 種別(A-L) 設定へ */
							set = 1;
							grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* 種別(A-L)      反転 */
							grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "種"           反転 */
							mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* 金額表示 */
							Fun_Dsp( FUNMSG[28] );						/* "  ▲    ▼    ←   読出  終了 " */
						}
						else {
							if(type != SP_MX_N_HOUR && type != SP_MX_N_MH_NEW && type != SP_MX_N_MHOUR_W){
								Fun_Dsp( FUNMSG2[2] );					/* "  ▲    ▼         読出  終了 " */
							}else{
								Fun_Dsp( FUNMSG2[40] );					/* "  ▲    ▼  時間   読出  終了 " */
							}
							mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* 金額表示 */
							money = -1;
						}
					}
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if( ((type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW) && set == 4) ||			// 時間回/回数設定
						(type == SP_MX_N_MHOUR_W && set == 5)){
						if (time_count == -1){				/* ﾃﾝｷｰでの入力がない場合 */
							time_count = 0;					/* 入力領域の初期化 */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* ﾃﾝｷｰ入力値の取得 */
						if( time_count == 0 ){				/* 入力値が0の場合 */
							grachr( (ushort)((line_pos*2-2) + set), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
						opedpl( (ushort)((line_pos*2-2) + set), 24, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 入力値 反転 */
						break;
					}

					if (money == -1)
						money = 0;

					money = (money*10 + (msg-KEY_TEN0)) % 100000L;
					if( type == SP_MX_BAND ){	// 時間帯上限
						opedpl( (ushort)((line_pos*2-2) + set - top_m_pos ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 料金 反転 */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set - top_m_pos ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
					} else {
						opedpl( (ushort)((line_pos*2-2) + set ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* 料金 反転 */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　" */
						}
					}
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* 金額表示 */
					money = -1;
					time_count = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  最大料金表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnylim_dsp( type, no, kind, mod1, mod2, mod3 )          |*/
/*| PARAMETER    : char type    : 日毎最大料金タイプ                       |*/
/*|              : char no      : 体系(1-3)                                |*/
/*|              : char kind    : 種別(A-L)                                |*/
/*|              : char rev_pos : 反転位置                                 |*/
/*|              : char top_pos : 料金帯の最上表示番号                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnylim_dsp(char type, char no, char kind, char rev_pos, char top_pos )
{
	long	money;
	ulong	time_count;
	ushort	rev1, rev2, rev3, rev4;
	ushort	line_pos;
	ushort	param_pos;
	short	adr_k, adr_z;			/* ｱﾄﾞﾚｽNo.						*/
	char	pos_k, pos_z;			/* 桁位置						*/
	short	seg;
	long	*pRAT;
	ushort	rev6, rev7;
	char	len_z,len_k;
	uchar	i = 0;		// for文用
	struct {
		short	no;
		ushort	rev;
	} revise[] = {
		{   0, (rev_pos == 1) ? 1 : 0},	// 第1料金帯
		{   1, (rev_pos == 2) ? 1 : 0},	// 第2料金帯
		{   2, (rev_pos == 3) ? 1 : 0},	// 第3料金帯
		{   3, (rev_pos == 4) ? 1 : 0},	// 第4料金帯
		{   4, (rev_pos == 5) ? 1 : 0},	// 第5料金帯
		{ -13, (rev_pos == 6) ? 1 : 0}	// 第6料金帯 共通パラメータが第6のみ別の場所に設定されているため、-13している
	};

	rev1 = (rev_pos == 0) ? 1 : 0;		// 絶対最大料金
	rev2 = (rev_pos == 1) ? 1 : 0;		// 
	rev3 = (rev_pos == 2) ? 1 : 0;		// 
	rev4 = (rev_pos == 3) ? 1 : 0;		// 
	rev6 = (rev_pos == 5) ? 1 : 0;		// 
	rev7 = (rev_pos == 6) ? 1 : 0;		// 
	/* 料金体系によって参照するパラメータのセクションを指定する */
	seg = GET_PARSECT_NO(no);
	pRAT = CPrmSS[seg];
	

	if(type != SP_MX_BAND){			// 時間帯最大以外
		if( type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW || type == SP_MX_N_MHOUR_W){					// ｎ時間最大
			if(!RYO_CAL_EXTEND){		// 料金計算拡張
				seg = S_CAL;								// セクション28
			}
			else {
				seg = S_CLX;								// セクション56
			}
		}
		param_pos = 1;
	}else{
		param_pos = 15;
	}
	line_pos = 2;									// 開始行

	dispmlclr(3, 6);
	mnylim_type_disp( type  ,top_pos);
	money = pRAT[GET_LIM_IDX(no, kind)] / 10;
	opedpl( line_pos, 16, (ulong)money, 5, 0, rev1, COLOR_BLACK,  LCD_BLINK_OFF );		/* 絶対最大料金 */
	if (money == 0)
		grachr( line_pos, 24, 2, rev1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
	if (type != 0) {
		if (type == 4 || type == 10) {
			get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);
			get_mnylim_zikann_adr(type, kind, &adr_z, &len_z, &pos_z);
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos]/10;			/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* 日毎・２４時間最大料金・○○時間○○回料金 */
			if (money == 0){
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( line_pos, 0, time_count, 2, 0, rev6, COLOR_BLACK,  LCD_BLINK_OFF );				/* 時間回/時間 */
			if (time_count == 0){
				/* ０を反転表示(赤色表示)するため、空白の反転は不要 */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}
			time_count = prm_get(COM_PRM, seg, adr_k, len_k, pos_k);	/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( ++line_pos, 24, time_count, 2, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* 時間回/回数 */
			if (time_count == 0){
				grachr( line_pos, 24, 4, rev3, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[36]);				/* "　" */
			}
		}else if(type == SP_MX_N_MHOUR_W){																// ２種類のｎ時間ｍ回最大
			get_mnylim_zikann_adr(type, kind, &adr_z, &len_z, &pos_z);
			
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos+1]/10;			/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* 日毎・２４時間最大料金・○○時間○○回料金 */
			if (money == 0){
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}
			
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( line_pos, 0, time_count, 2, 0, rev6, COLOR_BLACK,  LCD_BLINK_OFF );				/* 最大時間１ */
			if (time_count == 0){
				/* ０を反転表示(赤色表示)するため、空白の反転は不要 */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}
			
			get_mnylim_kaisuu_adr(10, kind, &adr_k, &len_k, &pos_k);
			get_mnylim_zikann_adr(10, kind, &adr_z, &len_z, &pos_z);
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos]/10;			/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* 日毎・２４時間最大料金・○○時間○○回料金 */
			if (money == 0){
				grachr( line_pos, 24, 2, rev3, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}
		
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( line_pos, 0, time_count, 2, 0, rev7, COLOR_BLACK,  LCD_BLINK_OFF );				/* 最大時間２ */
			if (time_count == 0){
				/* ０を反転表示(赤色表示)するため、空白の反転は不要 */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
			}

			time_count = prm_get(COM_PRM, seg, adr_k, len_k, pos_k);	/* 共通ﾊﾟﾗﾒｰﾀ取得 */
			opedpl( ++line_pos, 24, time_count, 2, 0, rev4, COLOR_BLACK,  LCD_BLINK_OFF );			/* 繰り返し回数 */
			if (time_count == 0){
				grachr( line_pos, 24, 4, rev4, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[36]);				/* "　" */
			}
		} else if ( SP_MX_BAND == type ){			// 時間帯上限（料金帯上限）
			for( i = 0; i < 4 ; i++ ){
				money = pRAT[GET_LIM_IDX(no, kind)+param_pos + revise[ top_pos + i ].no] / 10;
				opedpl( ++line_pos, 16, (ulong)money, 5, 0, revise[ top_pos + i ].rev, COLOR_BLACK,  LCD_BLINK_OFF );	/* 第1料金体帯 */
				if (money == 0)
					grachr( line_pos, 24, 2, revise[ top_pos + i ].rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
			}
		}else{
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos] / 10;
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* 日毎・２４時間最大料金・○○時間○○回料金 */
			if (money == 0)
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
			if (type == 3 || type == 5) {
				money = pRAT[GET_LIM_IDX(no, kind)+(param_pos+1)] / 10;
				opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* 日毎最大料金２ */
				if (money == 0)
					grachr( line_pos, 24, 2, rev3, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：サービス券役割                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnytic( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnytic(void)
{
	typedef struct {
		long	role;		/* 役割 */
		long	data;		/* データ */
		long	chng;		/* 切替 */
	} SRV_PRM;
	SRV_PRM	*psrv;
	ushort	msg;
	char	set;			/* ０:割引設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券種別 */
							/* １:割引設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券役割 */
							/* ２:割引設定中／ｶｰｿﾙ位置＝割引内容 	*/
							/* ３:切替設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券種別 */
							/* ４:切替設定中／ｶｰｿﾙ位置＝車種切替 	*/

	int		kind;			/* 種別 0～14=A～O */
	long	role;
	long	data;
	long	chng;
	uchar	i;
	uchar	page;
	ushort	line;
	uchar	svs_syu;

	psrv = (SRV_PRM *)&CPrmSS[S_SER][1];

	dispclr();

	set = 0;
	kind = 0;

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[2]);									/* "＜サービス券役割＞　　　　　　" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[12]);								/* " [種別ー役割] 　　 [割引内容] " */

	for( i=0 ; i<5 ; i++){											/* ｻｰﾋﾞｽA～E表示（A種反転表示）		*/

		line	= 2+i;												/* 表示行ｾｯﾄ 						*/

		if( i==0 ){
			grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);						/* 種別 反転 						*/
			grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "種" 反転 						*/
		}
		else{
			grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);						/* 種別 正転 						*/
			grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "種" 正転 						*/
		}
		grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "ー" 							*/
		grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[i].role]);				/* 役割 							*/
		mnytic_data_dsp( (char)i, psrv[i].role, psrv[i].data, 0 );	/* 割引内容 						*/
	}

	Fun_Dsp( FUNMSG2[3] );											/* "　▲　　▼　　→　種切替 終了 " */

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:											/* モード切替 						*/

			BUZPI();
			return MOD_CHG;

		case KEY_TEN_F5:											/* 終了(F5) 						*/

			BUZPI();
			return MOD_EXT;

		default:													/* その他 							*/
			/* 割引-種別設定 */
			if (set == 0) {

				/* ０:割引設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券種別 */

				switch (msg) {

				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */

					BUZPI();
					page = (uchar)(kind/5);												/* 現在表示中のﾍﾟｰｼﾞ取得（A～E=0,F～J=1,K～O=2）*/
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 14;
					}
					else {
						if (++kind > 14)
							kind = 0;
					}
					if( page != kind/5 ){
						/* ﾍﾟｰｼﾞ変更あり */
						for( i=0 ; i<5 ; i++){											/* ｻｰﾋﾞｽ券５種表示 					*/
							line	= 2+i;												/* 表示行ｾｯﾄ 						*/
							svs_syu	= (uchar)((kind/5)*5+i);							/* ｻｰﾋﾞｽ券種ｾｯﾄ 					*/

							if( i==(uchar)(kind%5) ){
								grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ｶｰｿﾙ位置：種別 反転 				*/
								grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* ｶｰｿﾙ位置："種" 反転 				*/
							}
							else{
								grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* 種別 正転 						*/
								grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "種" 正転 						*/
							}
							grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "ー" 							*/
							grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);			/* 役割 							*/
							mnytic_data_dsp( (char)svs_syu, psrv[svs_syu].role, psrv[svs_syu].data, 0);	/* 割引内容 		*/
						}
					}
					else{														
						/* ﾍﾟｰｼﾞ変更なし */
						line	= (kind+2-((kind/5)*5));
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ｶｰｿﾙ位置：種別 反転 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ｶｰｿﾙ位置："種" 反転 				*/

						if (msg == KEY_TEN_F1) {
							line	= ((kind+1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind+1);
						}
						else {
							line	= ((kind-1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind-1);
						}
						grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);						/* 前ｶｰｿﾙ位置：種別 正転 			*/
						grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* 前ｶｰｿﾙ位置："種" 正転 			*/
					}
					break;

				case KEY_TEN_F3:	/* →(F3) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ｶｰｿﾙ位置：種別 正転 				*/
					grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ｶｰｿﾙ位置："種" 正転 				*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);					/* 役割      反転 					*/
					Fun_Dsp( FUNMSG2[4] );																				/* "　←　　→　 変更　書込  終了 " */
					role = psrv[kind].role;
					set = 1;
					break;

				case KEY_TEN_F4:	/* 種切替(F4) */

					BUZPI();
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[15]);									/* " [種別ー車種切替] 　　　　　　" */

					svs_syu = (uchar)((kind / 5) * 5);
					for( i=0 ; i<5 ; i++ ){
						mnytic_chng_dsp( (char)(svs_syu+i), psrv[svs_syu+i].chng, 0 );	/* 切替 							*/
					}
					Fun_Dsp( FUNMSG2[5] );												/* "　▲　　▼　　→　 割引  終了 " */
					set = 3;
					break;

				default:
					break;
				}
			}
			/* 割引-役割設定 */
			else if (set == 1) {

				/* １:割引設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券役割 */

				switch (msg) {

				case KEY_TEN_F1:	/* ←(F1) */

					BUZPI();

					for( i=0 ; i<5 ; i++){												/* ｻｰﾋﾞｽ券５種表示 					*/

						line	= 2+i;													/* 表示行ｾｯﾄ 						*/
						svs_syu	= (uchar)((kind/5)*5+i);								/* ｻｰﾋﾞｽ券種ｾｯﾄ 					*/

						if( i==(uchar)(kind%5) ){
							grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);					/* ｶｰｿﾙ位置：種別 反転 				*/
							grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* ｶｰｿﾙ位置："種" 反転 				*/
						}
						else{
							grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);					/* 種別 正転 						*/
							grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* "種" 正転 						*/
						}
						grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);								/* "ー" 							*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);				/* 役割 							*/
					}
					Fun_Dsp( FUNMSG2[3] );																				/* "　▲　　▼　　→　種切替 終了 " */
					set = 0;
					break;

				case KEY_TEN_F4:	/* 書込(F4) */
					/* 役割が変更されたら内容を０にする */
					if (psrv[kind].role != role)
						psrv[kind].data = 0;
					psrv[kind].role = role;
					OpelogNo = OPLOG_SERVICEYAKUWARI;									/* 操作履歴登録 					*/
					f_ParaUpdate.BIT.other = 1;											// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

				case KEY_TEN_F2:	/* →(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));									/* 表示行ｾｯﾄ 						*/

					/* 時間割引・料金割引の場合：割引内容設定へ */
					if (psrv[kind].role == 1 || psrv[kind].role == 2) {
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);				/* 役割      正転 					*/
						mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );									/* 内容      反転 					*/
						Fun_Dsp( FUNMSG2[2] );																			/* "　←　　→　 　　　書込  終了 " */
						data = -1;
						set = 2;
					}
					/* 未設定・全額割引の場合：種別設定へ */
					else {
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ｶｰｿﾙ位置：種別 反転 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ｶｰｿﾙ位置："種" 反転 				*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);				/* 役割      正転 					*/
						mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );	/* 内容      正転 					*/
						Fun_Dsp( FUNMSG2[3] );											/* "　▲　　▼　　→　種切替 終了 " */
						set = 0;
					}
					break;

				case KEY_TEN_F3:	/* 変更(F3) */

					BUZPI();
					if (++role > 4)
						role = 0;
					if (role == 3)	/* "％割引"をスキップする */
						role = 4;
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);								/* 役割      反転 					*/
					break;

				case KEY_TEN_CL:	/* 取消(テンキー) */

					BUZPI();
					role = psrv[kind].role;
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);								/* 役割      反転 					*/
					break;

				default:
					break;
				}
			}
			/* 割引-内容設定 */
			else if (set == 2) {

				/* ２:割引設定中／ｶｰｿﾙ位置＝割引内容 	*/

				switch (KEY_TEN0to9(msg)) {

				case KEY_TEN_F1:	/* ←(F1) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);					/* 役割      反転 					*/
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );										/* 内容      正転 					*/
					Fun_Dsp( FUNMSG2[4] );																				/* "　←　　→　 変更　書込  終了 " */
					role = psrv[kind].role;
					set = 1;
					break;

				case KEY_TEN_F4:	/* 書込(F4) */

					if (data != -1) {
						/* 時間割引 */
						if (psrv[kind].role == 1) {
							if ((data%100) > 59) {
								BUZPIPI();
								mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );	/* 内容  反転 				*/
								data = -1;
								break;
							}
						}
						psrv[kind].data = data;
						OpelogNo = OPLOG_SERVICEYAKUWARI;								/* 操作履歴登録 					*/
						f_ParaUpdate.BIT.other = 1;										// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}

				case KEY_TEN_F2:	/* →(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ｶｰｿﾙ位置：種別 反転 				*/
					grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ｶｰｿﾙ位置："種" 反転 				*/
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );										/* 内容      正転 					*/
					Fun_Dsp( FUNMSG2[3] );																				/* "　▲　　▼　　→　種切替 終了 " */
					set = 0;
					break;

				case KEY_TEN:		/* 数字(テンキー) */

					BUZPI();
					/* 時間割引：最大値9:59(959) */
					if (psrv[kind].role == 1) {
						if (data == -1)
							data = 0;
						data = (data*10 + msg-KEY_TEN0) % 10000;
					}
					/* 料金割引：最大値9990円 */
					else {
						if (data == -1)
							data = 0;
						data = (data*10 + (msg-KEY_TEN0)*10) % 10000;
					}
					mnytic_data_dsp( kind, psrv[kind].role, data, 1 );					/* 内容  反転 						*/
					break;

				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );		/* 内容  反転 						*/
					data = -1;
					break;

				default:
					break;
				}
			}
			/* 車種-種別設定 */
			else if (set == 3) {

				/* ３:切替設定中／ｶｰｿﾙ位置＝ｻｰﾋﾞｽ券種別 */

				switch (msg) {

				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */

					BUZPI();
					page = (uchar)(kind/5);												/* 現在表示中のﾍﾟｰｼﾞ取得（A～E=0,F～J=1,K～O=2）*/
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 14;
					}
					else {
						if (++kind > 14)
							kind = 0;
					}
					if( page != kind/5 ){
						/* ﾍﾟｰｼﾞ変更あり */
						for( i=0 ; i<5 ; i++){											/* ｻｰﾋﾞｽ券５種表示 					*/

							line	= 2+i;												/* 表示行ｾｯﾄ 						*/
							svs_syu	= (uchar)((kind/5)*5+i);							/* ｻｰﾋﾞｽ券種ｾｯﾄ 					*/

							if( i==(uchar)(kind%5) ){
								grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ｶｰｿﾙ位置：種別 反転 				*/
								grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* ｶｰｿﾙ位置："種" 反転 				*/
							}
							else{
								grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* 種別 正転 						*/
								grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "種" 正転 						*/
							}
							grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "ー" 							*/
							mnytic_chng_dsp( (char)svs_syu, psrv[svs_syu].chng, 0 );									/* 切替 							*/
						}
					}
					else{														
						/* ﾍﾟｰｼﾞ変更なし */
						line	= (kind+2-((kind/5)*5));
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ｶｰｿﾙ位置：種別 反転 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ｶｰｿﾙ位置："種" 反転 				*/

						if (msg == KEY_TEN_F1) {
							line	= ((kind+1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind+1);
						}
						else {
							line	= ((kind-1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind-1);
						}
						grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);						/* 前ｶｰｿﾙ位置：種別 正転 			*/
						grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* 前ｶｰｿﾙ位置："種" 正転 			*/
					}
					break;

				case KEY_TEN_F3:	/* →(F3) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* 表示行ｾｯﾄ 						*/
					grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* 種別(A-C) 正転 					*/
					grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* "種"      正転 					*/
					mnytic_chng_dsp( kind, psrv[kind].chng, 1 );														/* 切替 反転 						*/
					Fun_Dsp( FUNMSG2[4] );																				/* "　←　　→　 変更　書込  終了 " */
					chng = psrv[kind].chng;
					set = 4;
					break;

				case KEY_TEN_F4:	/* 割引(F4) */

					BUZPI();
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[12] );									/* " [種別ー役割] 　　 [割引内容] " */

					for( i=0 ; i<5 ; i++){												/* ｻｰﾋﾞｽ券５種表示 					*/

						line	= 2+i;													/* 表示行ｾｯﾄ 						*/
						svs_syu	= (uchar)((kind/5)*5+i);								/* ｻｰﾋﾞｽ券種ｾｯﾄ 					*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);				/* 役割 */
						mnytic_data_dsp( (char)svs_syu, psrv[svs_syu].role, psrv[svs_syu].data, 0);	/* 内容 				*/
					}
					Fun_Dsp( FUNMSG2[3] );												/* "　▲　　▼　　→　種切替 終了 " */
					set = 0;
					break;

				default:
					break;
				}
			}
			/* 車種-切替設定 */
			else {	/* if (set == 4) */

				/* ４:切替設定中／ｶｰｿﾙ位置＝車種切替 	*/

				switch (msg) {

				case KEY_TEN_F4:	/* 書込(F4) */

					psrv[kind].chng = chng;
					OpelogNo = OPLOG_SERVICEYAKUWARI;									/* 操作履歴登録 					*/
					f_ParaUpdate.BIT.other = 1;											// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

				case KEY_TEN_F1:	/* ←(F1) */
				case KEY_TEN_F2:	/* →(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));									/* 表示行ｾｯﾄ 						*/
					grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ｶｰｿﾙ位置：種別 反転 				*/
					grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ｶｰｿﾙ位置："種" 反転 				*/
					mnytic_chng_dsp( kind, psrv[kind].chng, 0 );						/* 切替 正転 						*/
					Fun_Dsp( FUNMSG2[5] );												/* "　▲　　▼　　→　 割引  終了 " */
					set = 3;
					break;

				case KEY_TEN_F3:	/* 変更(F3) */

					BUZPI();
					if (++chng > 12) {
						chng = 0;
					}
					mnytic_chng_dsp( kind, chng, 1 );									/* 切替 反転 						*/
					break;

				case KEY_TEN_CL:	/* 取消(テンキー) */

					BUZPI();
					chng = psrv[kind].chng;
					mnytic_chng_dsp( kind, chng, 1 );									/* 切替 反転						*/
					break;

				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  割引内容表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytic_data_dsp( kind, role, data, rev )                |*/
/*| PARAMETER    : char   kind : 種別(0:A 1:B 2:C)                         |*/
/*|              : long   role : 役割(0-4)                                 |*/
/*|              : long   data : 内容                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytic_data_dsp(char kind, long role, long data, ushort rev)
{
	ushort	line;	/* 表示行 */

	line	= (kind+2-((kind/5)*5));						/* 表示行ｾｯﾄ 	*/
	grachr( line, 16, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);					/* "　" 		*/
	/* 時間 */
	if (role == 1) {
		grachr( line, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "ー" 		*/
		// 時間割引は99:59までとする
		opedsp( line, 18, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 			*/
		grachr( line, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" 		*/
		opedsp( line, 24, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 			*/
	}
	/* 料金 */
	else if (role == 2) {
		grachr( line, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "ー" 		*/
		opedsp( line, 18, (ushort)(data/10), 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 料金 		*/
		if (data/10 == 0) {
			grachr( line, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" 		*/
		}
		opedsp( line, 24, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* "０" 		*/
		grachr( line, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);					/* "円" 		*/
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  種別切替表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytic_chng_dsp( kind, chng, rev )                      |*/
/*| PARAMETER    : char   kind : 種別(0:A 1:B 2:C)                         |*/
/*|              : long   chng : 切替                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytic_chng_dsp(char kind, long chng, ushort rev)
{
	ushort	line;	/* 表示行 */

	line	= (kind+2-((kind/5)*5));					/* 表示行ｾｯﾄ 	*/
	grachr( line, 8, 22, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" 		*/
	if (chng == 0) {
		grachr( line, 10, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[1]);			/* "　なし" 	*/
	}
	else {
		grachr( line, 10, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[0]);			/* "　車種" 	*/
		grachr( line, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[chng-1]);		/* A-L 			*/
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：店役割                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnyshp( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyshp(void)
{
	typedef struct {
		long	role;		/* 役割 */
		long	data;		/* データ */
		long	chng;		/* 切替 */
	} SHP_PRM;
	SHP_PRM	*pshp;
	ushort	msg;
	char	set;			/* 0:店設定中 1:役割設定中 2:内容設定中 3:切替設定中 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
//	char	no;				/* 店番号 1-100 */
	short	no;				/* 店番号 1-999 */
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
	long	role;
	long	data;
	long	chng;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 	char	top;
	short	top;			/* 先頭店番号 1-995 */
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
	char	i;
	short	input_no;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
	short	no_max;			/* 店番号最大 */
	short	top_max;		/* 先頭店番号最大 */
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)

	pshp = (SHP_PRM *)&CPrmSS[S_STO][1];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[3]);				/* "＜店役割＞　　　　　　　　　　" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[17]);			/* "　　　　　【役割】【割引内容】" */
	for (i = 0; i < 5; i++) {
		grachr((ushort)(2+i), 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[4]);						/* "店" */
		opedsp((ushort)(2+i), 2, (ushort)(i+1), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 */
		grachr((ushort)(2+i), 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);						/* "ー" */
		grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[i].role]);			/* 役割 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 		mnyshp_data_dsp(1, (char)(i+1), pshp[i].role, pshp[i].data, 0);								/* 内容 */
		mnyshp_data_dsp(1, (i+1), pshp[i].role, pshp[i].data, 0);									/* 内容 */
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
	}
	opedsp(2, 2, 1, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);											/* "００１" 反転 */
	Fun_Dsp( FUNMSG2[6] );						/* "  ＋  －／読  →  種切替 終了 " */

	set = 0;
	no = top = 1;
	input_no = -1;

// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
	if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {	// 多店舗割引あり
		no_max  = T_MISE_NO_CNT;
		top_max = T_MISE_NO_CNT - 4;
	} else {										// 多店舗割引なし
		no_max  = MISE_NO_CNT;
		top_max = MISE_NO_CNT - 4;
	}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* 割引-店番号設定 */
			if (set == 0) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					no--;
					if (no < 1) {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						no = 100;
// 						top = 96;
						no = no_max;
						top = top_max;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					else if (no < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);	/* 役割 */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* 内容 */
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* 役割(未使用) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* 内容(空白)*/
						} else {							// 店割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* 役割 */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* 内容 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 反転 */
					input_no = -1;
					break;
				case KEY_TEN_F2:	/* ▼／読(F2) */
					if (input_no == -1) {
						no++;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						if (no > 100) {
						if ( no > no_max ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
							no = top = 1;
						}
						else if (no > top+4) {
							top++;
						}
					}
					else {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						if (input_no < 1 || input_no > 100) {
						if ( (input_no < 1) || (input_no > no_max) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
							BUZPIPI();
							opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 店番号 反転 */
							input_no = -1;
							break;
						}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						no = top = (char)input_no;
// 						if (top > 96) {
// 							top = 96;
						no = top = input_no;
						if ( top > top_max ) {
							top = top_max;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
						}
					}
					BUZPI();
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);	/* 役割 */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* 内容 */
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* 役割(未使用) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* 内容(空白)*/
						} else {							// 店割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* 役割 */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* 内容 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 反転 */
					input_no = -1;
					break;
				case KEY_TEN_F3:	/* →(F3) */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					if ( no > MISE_NO_CNT ) {
						BUZPIPI();
						break;
					}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 店番号 正転 */
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);		/* 役割 反転 */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);									/* 内容 */
					Fun_Dsp( FUNMSG2[4] );																			/* "　←　　→　 変更　書込  終了 " */
					role = pshp[no-1].role;
					set = 1;
					break;
				case KEY_TEN_F4:	/* 種切替(F4) */
					BUZPI();
					grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[18]);			/* "　　　　　【車種切替】　　　　" */
					for (i = 0; i < 5; i++) {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);				/* 切替 */
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* 切替 */
						} else {							// 店割引
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* 切替 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					Fun_Dsp( FUNMSG2[7] );														/* "  ＋  －／読  →   割引  終了 " */
					input_no = -1;
					set = 3;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (input_no == -1)
						input_no = 0;
					input_no = (input_no*10 + (msg-KEY_TEN0)) % 1000;
					opedsp((ushort)(2+(no-top)), 2, (ushort)input_no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 店番号 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					input_no = -1;
					break;
				default:
					break;
				}
			}
			/* 割引-役割設定 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* 店番号 反転 */
					grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);		/* 役割   正転 */
					Fun_Dsp( FUNMSG2[6] );															/* "  ＋  －／読  →  種切替 終了 " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					/* 役割が変更されたら内容を０にする */
					if (pshp[no-1].role != role)
						pshp[no-1].data = 0;
					pshp[no-1].role = role;
					OpelogNo = OPLOG_MISEKUWARI;			// 操作履歴登録
					f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					/* 時間割引・料金割引の場合：割引内容設定へ */
					if (pshp[no-1].role == 1 || pshp[no-1].role == 2) {
						grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	/* 役割 正転 */
						mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);									/* 内容 反転 */
						Fun_Dsp( FUNMSG2[2] );													/* "　←　　→　 　　　書込  終了 " */
						data = -1;
						set = 2;
					}
					/* 未設定・全額割引の場合：種別設定へ */
					else {
						opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 反転 */
						grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	 /* 役割   正転 */
						mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);	/* 内容   正転 */
						Fun_Dsp( FUNMSG2[6] );														/* "  ＋  －／読  →  種切替 終了 " */
						input_no = -1;
						set = 0;
					}
					break;
				case KEY_TEN_F3:	/* 変更(F3) */
					BUZPI();
					if (++role > 4)
						role = 0;
					if (role == 3)	/* "％割引"をスキップする */
						role = 4;
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);	/* 役割 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					role = pshp[no-1].role;
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);	/* 役割 反転 */
					break;
				default:
					break;
				}
			}
			/* 割引-割引内容設定 */
			else if (set == 2) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	/* 役割 反転 */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);	/* 内容 正転 */
					Fun_Dsp( FUNMSG2[4] );						/* "　←　　→　 変更　書込  終了 " */
					role = pshp[no-1].role;
					set = 1;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						/* 時間割引 */
						if (pshp[no-1].role == 1) {
							if ((data%100) > 59) {
								BUZPIPI();
								mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);	/* 内容 反転 */
								data = -1;
								break;
							}
						}
						pshp[no-1].data = data;
						OpelogNo = OPLOG_MISEKUWARI;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 反転 */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);									/* 内容   正転 */
					Fun_Dsp( FUNMSG2[6] );														/* "  ＋  －／読  →  種切替 終了 " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					/* 時間割引：最大値9:59(959) */
					if (pshp[no-1].role == 1) {
						if (data == -1)
							data = 0;
						data = (data*10 + msg-KEY_TEN0) % 10000;
					}
					/* 料金割引：最大値9990円 */
					else {
						if (data == -1)
							data = 0;
						data = (data*10 + (msg-KEY_TEN0)*10) % 10000;
					}
					mnyshp_data_dsp(top, no, pshp[no-1].role, data, 1);				/* 内容 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);	/* 内容 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 種切替-店番号設定 */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ＋(F1) */
					BUZPI();
					no--;
					if (no < 1) {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						no = 100;
// 						top = 96;
						no = no_max;
						top = top_max;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					else if (no < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);								/* 切替 */
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* 切替 */
						} else {							// 店割引
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* 切替 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					input_no = -1;
					break;
				case KEY_TEN_F2:	/* －／読(F2) */
					if (input_no == -1) {
						no++;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						if (no > 100) {
						if ( no > no_max ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
							no = top = 1;
						}
						else if (no > top+4) {
							top++;
						}
					}
					else {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						if (input_no < 1 || input_no > 100) {
						if ( (input_no < 1) || (input_no > no_max) ) {
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
							BUZPIPI();
							opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 店番号 反転 */
							input_no = -1;
							break;
						}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						no = top = (char)input_no;
// 						if (top > 96) {
// 							top = 96;
						no = top = input_no;
						if ( top > top_max ) {
							top = top_max;
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
						}
					}
					BUZPI();
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);								/* 切替 */
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* 切替 */
						} else {							// 店割引
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* 切替 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					input_no = -1;
					break;
				case KEY_TEN_F3:	/* →(F3) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 店番号 正転 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 					mnyshp_chng_dsp(top, no, pshp[no-1].chng, 1);			/* 切替 反転 */
// 					Fun_Dsp( FUNMSG2[4] );						/* "　←　　→　 変更　書込  終了 " */
// 					chng = pshp[no-1].chng;
					if ( no > MISE_NO_CNT ) {	// 多店舗割引
						chng = prm_get(COM_PRM, S_TAT, (no - 69), 2, 1);
					} else {					// 店割引
						chng = pshp[no-1].chng;
					}
					mnyshp_chng_dsp(top, no, chng, 1);			/* 切替 反転 */
					Fun_Dsp( FUNMSG2[4] );						/* "　←　　→　 変更　書込  終了 " */
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					set = 4;
					break;
				case KEY_TEN_F4:	/* 割引(F4) */
					BUZPI();
					grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[17]);			/* "　　　　　【役割】【割引内容】" */
					for (i = 0; i < 5; i++) {
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* 役割 */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);
						if ( (top + i) > MISE_NO_CNT ) {	// 多店舗割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* 役割(未使用) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* 内容(空白)*/
						} else {							// 店割引
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* 役割 */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* 内容 */
						}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					Fun_Dsp( FUNMSG2[6] );						/* "  ＋  －／読  →  種切替 終了 " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (input_no == -1)
						input_no = 0;
					input_no = (input_no*10 + (msg-KEY_TEN0)) % 1000;
					opedsp((ushort)(2+(no-top)), 2, (ushort)input_no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 店番号 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* 店番号 反転 */
					input_no = -1;
					break;
				default:
					break;
				}
			}
			/* 種切替-種別切替設定 */
			else {	/* if (set == 4) */
				switch (msg) {
				case KEY_TEN_F4:	/* 書込(F4) */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					if ( no > MISE_NO_CNT ) {	// 多店舗割引
						prm_set(COM_PRM, S_TAT, (no - 69), 2, 1, chng);
					} else {					// 店割引
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
						pshp[no-1].chng = chng;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					OpelogNo = OPLOG_MISEKUWARI;			// 操作履歴登録
					f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				case KEY_TEN_F1:	/* ←(F1) */
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 店番号 反転 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					if ( no > MISE_NO_CNT ) {	// 多店舗割引
						mnyshp_chng_dsp(top, no, prm_get(COM_PRM, S_TAT, (no - 69), 2, 1), 0);	/* 切替   正転 */
					} else {					// 店割引
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
						mnyshp_chng_dsp(top, no, pshp[no-1].chng, 0);							/* 切替   正転 */
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					Fun_Dsp( FUNMSG2[7] );						/* "  ＋  －／読  →   割引  終了 " */
					input_no = -1;
					set = 3;
					break;
				case KEY_TEN_F3:	/* 変更(F3) */
					BUZPI();
					if (++chng > 12) {
						chng = 0;
					}
					mnyshp_chng_dsp(top, no, chng, 1);		/* 切替 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					if ( no > MISE_NO_CNT ) {	// 多店舗割引
						chng = prm_get(COM_PRM, S_TAT, (no - 69), 2, 1);
					} else {					// 店割引
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
						chng = pshp[no-1].chng;
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
					mnyshp_chng_dsp(top, no, chng, 1);		/* 切替 反転 */
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  割引内容表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnyshp_data_dsp( top, no, role, data, rev )             |*/
/*| PARAMETER    : short  top  : 先頭店番号                                |*/
/*|              : short  no   : 店番号                                    |*/
/*|              : long   role : 役割                                      |*/
/*|              : long   data : 内容                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// static void	mnyshp_data_dsp(char top, char no, long role, long data, ushort rev)
static void	mnyshp_data_dsp(short top, short no, long role, long data, ushort rev)
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
{
	grachr((ushort)(2+no-top), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);					/* "　" */
	/* 時間 */
	if (role == 1) {
		grachr((ushort)(2+no-top), 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "ー" */
		opedsp((ushort)(2+no-top), 20, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
		grachr((ushort)(2+no-top), 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
		opedsp((ushort)(2+no-top), 26, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
	}
	/* 料金 */
	else if (role == 2) {
		grachr((ushort)(2+no-top), 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "ー" */
		opedsp((ushort)(2+no-top), 20, (ushort)(data/10), 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 料金 */
		if (data/10 == 0) {
			grachr((ushort)(2+no-top), 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
		}
		opedsp((ushort)(2+no-top), 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* "０" */
		grachr((ushort)(2+no-top), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);				/* "円" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車種切替表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnyshp_chng_dsp( top, no, chng, rev )                   |*/
/*| PARAMETER    : short  top  : 先頭店番号                                |*/
/*|              : short  no   : 店番号                                    |*/
/*|              : long   chng : 切替                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
// static void	mnyshp_chng_dsp(char top, char no, long chng, ushort rev)
static void	mnyshp_chng_dsp(short top, short no, long chng, ushort rev)
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店役割画面の変更)
{
	grachr((ushort)(2+no-top), 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
	if (chng == 0) {
		grachr((ushort)(2+no-top), 12, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[1]);			/* "　なし" */
	}
	else {
		grachr((ushort)(2+no-top), 12, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[0]);			/* "　車種" */
		grachr((ushort)(2+no-top), 12, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[chng-1]);		/* A-L */
	}
}

// MH810105(S) MH364301 インボイス対応
///*[]----------------------------------------------------------------------[]*/
///*|  料金設定：消費税の税率                                                |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_mnytax( void )                                    |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//unsigned short	UsMnt_mnytax(void)
//{
//	ushort	msg;
//	long	tax;		/* 税率 */
//	char	changing;	/* 1:税率設定中 */
//	long	*ptax;
//
//	ptax = &CPrmSS[S_CAL][19];
//
//	dispclr();
//
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );			/* "＜消費税の税率＞　　　　　　　" */
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[13] );			/* "　現在の税率：　　　　　　　　" */
//	if (*ptax == 0) {
//		grachr( 2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );		/* "消費税なし　" */
//	}
//	else {
//		mnytax_dsp(*ptax, 0);						/* 税率表示 */
//	}
//	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[14] );			/* "　　入力範囲は０～９９．９９　" */
//	Fun_Dsp( FUNMSG2[0] );														/* "　　　　　　 変更　　　　終了 " */
//
//	changing = 0;
//	for ( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch ( KEY_TEN0to9(msg) ) {
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		case LCD_DISCONNECT:
//			return MOD_CUT;
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		case KEY_MODECHG:
//			BUZPI();
//			return MOD_CHG;
//		case KEY_TEN_F5:		/* 終了(F5) */
//			BUZPI();
//			return MOD_EXT;
//		case KEY_TEN_F3:		/* 変更(F3) */
//			if (changing == 0) {
//				BUZPI();
//				mnytax_dsp(*ptax, 1);		/* 税率表示 反転 */
//				Fun_Dsp( FUNMSG2[1] );		/* "　　　　　　　　　 書込  終了 " */
//				changing = 1;
//				tax = -1;
//			}
//			break;
//		case KEY_TEN_F4:		/* 書込(F4) */
//			if (changing == 1) {
//				BUZPI();
//				if (tax != -1) {
//					*ptax = tax;	/* パラメータ更新 */
//				}
//				/* 表示 */
//				if (*ptax == 0) {
//					grachr( 2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	/* "消費税なし　" */
//				}
//				else {
//					mnytax_dsp(*ptax, 0);				/* 税率表示 */
//				}
//				Fun_Dsp( FUNMSG2[0] );			/* "　　　　　　 変更　　　　終了 " */
//				changing = 0;
//				OpelogNo = OPLOG_SHOHIZEI;			// 操作履歴登録
//				f_ParaUpdate.BIT.other = 1;			// 復電時にRAM上パラメータデータのSUM更新する
//			}
//			break;
//		case KEY_TEN:			/* 数字(テンキー) */
//			if (changing == 1) {
//				BUZPI();
//				if (tax == -1)
//					tax = 0;
//				tax = (tax*10 + (msg-KEY_TEN0)) % 10000;
//				mnytax_dsp(tax, 1);				/* 税率表示 反転 */
//			}
//			break;
//		case KEY_TEN_CL:		/* 取消(テンキー) */
//			if (changing == 1) {
//				BUZPI();
//				mnytax_dsp(*ptax, 1);				/* 税率表示 反転 */
//				tax = -1;
//			}
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  税率表示                                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : mnytax_dsp( tax, rev )                                  |*/
///*| PARAMETER    : long  tax  : 税率                                       |*/
///*|              : ushort rev : 0:正転 1:反転                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	mnytax_dsp(long tax, ushort rev)
//{
//	opedsp( 2, 16, (ushort)(tax/100), 2, 0, rev , COLOR_BLACK, LCD_BLINK_OFF);	/* 整数部分 */
//	grachr( 2, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[3] );				/* "．" */
//	opedsp( 2, 22, (ushort)(tax%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	/* 少数部分 */
//	grachr( 2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );				/* "％" */
//}
// インボイス用に作り直し
//[]----------------------------------------------------------------------[]
///	@brief			料金設定：消費税の税率
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///					        MOD_EXT : F5 key
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
unsigned short	UsMnt_mnytax(void)
{
	ushort	msg;
	int		changing;			// 1:編集中 
	long	*ptax;				// 18-0077 適用税率
	long	*pdate;				// 18-0078 税率変更基準日
	short	in_val=-1;			// 入力値(2桁)、-1は未入力状態
	int		cur_pos=0;			// カーソル位置: 変更前税率    =0
								//               変更後税率    =1
								//               変更基準日(年)=2
								//               変更基準日(月)=3
								//               変更基準日(日)=4
	ushort	cur_val[5];			// カーソル位置の入力値
	ushort	cur_val_tmp;		// F4押下時に変更した日付がエラーの場合に備えて元の入力値を退避するエリア

	ptax  = &CPrmSS[S_PRN][77];	// 18-0077 適用税率 取得
	pdate = &CPrmSS[S_PRN][78];	// 18-0078 税率変更基準日 取得

	dispclr();

	// 初期値を取得
	cur_val[0] = (ushort)prm_get(COM_PRM, S_PRN, 77, 2, 3);	// 変更前税率(18-0077③④)を取り出す
	cur_val[1] = (ushort)prm_get(COM_PRM, S_PRN, 77, 2, 1);	// 変更後税率(18-0077⑤⑥)を取り出す
	cur_val[2] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 5);	// 年(18-0078①②)を取り出す
	cur_val[3] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 3);	// 月(18-0078③④)を取り出す
	cur_val[4] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 1);	// 日(18-0078⑤⑥)を取り出す

	// 消費税の税率 初期表示
	mnytax_dspinit_dsp(cur_val);	// 変更前税率、変更後税率、変更基準日
	Fun_Dsp( FUNMSG2[0] );			// "　　　　　　 変更　　　　終了 "

	changing = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( KEY_TEN0to9(msg) ) {
// MH810105(S) MH364301 インボイス対応 GT-4100
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810105(E) MH364301 インボイス対応 GT-4100
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	// 終了(F5)
			BUZPI();
			if (changing == 1) {
				mnytax_dspinit_dsp(cur_val);								// 変更前税率、変更後税率、変更基準日
				Fun_Dsp( FUNMSG2[0] );										// "　　　　　　 変更　　　　終了 "
				changing = 0;												// 
			} else {
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F1:	// カーソル移動
		case KEY_TEN_F2:
			if (changing == 0) {
				break;
			}
			if((cur_pos >= 2) && (cur_pos <= 4)){							// 変更基準日から移動する場合
				if( in_val != -1 ){											// 数値を入力した場合はチェックする
					if( Date_Check( in_val, cur_pos ) != 0 ) {				// 変更基準日の入力値範囲チェック
						BUZPIPI();
						in_val = -1;
						break;
					}
				}
			}
			BUZPI();
			if( in_val != -1 ){
				cur_val[cur_pos] = in_val;												// 入力した場合は入力値をセット
			}
			mnytax_dsp(cur_val[cur_pos], cur_pos, 0);									// 入力値を表示 正転
			cur_pos = get_next_curpos(msg, cur_pos);									// 次のカーソル位置を取得
			in_val = -1;
			break;
		case KEY_TEN_F3:	// 変更(F3)
			if (changing == 0) {
				BUZPI();
				cur_pos = 0;															// カーソル位置を初期位置にセットする
				mnytax_editinit_dsp(cur_val);											// 変更前税率、変更後税率、変更基準日
				Fun_Dsp( FUNMSG[20] );													// "　▲　　▼　　　　 書込  終了 "
				changing = 1;															// 編集中
				in_val = -1;															// 未入力状態
			}
			break;
		case KEY_TEN_F4:	// 書込(F4)
			if (changing == 1) {
				// 何れかのカーソルで入力済みの状態で書込の場合はその値を反映してから設定処理を行う
				if( in_val != -1 ){
					cur_val_tmp = cur_val[cur_pos]; 									// 確定している入力値を退避する
					cur_val[cur_pos] = in_val;											// 現在の入力値を反映する
				}

				if( date_exist_check((short)(2000 + cur_val[2]), (short)cur_val[3], (short)cur_val[4]) != 0 ){	// 存在する日付かチェック
					BUZPIPI();
					if( in_val != -1 ){													// カーソル
						cur_val[cur_pos] = cur_val_tmp;									// 元に値に戻す
					}
					in_val = -1;
				} else {
					BUZPI();
					mnytax_dspinit_dsp(cur_val);												// 変更前税率、変更後税率、変更基準日
					Fun_Dsp( FUNMSG2[0] );														// "　　　　　　 変更　　　　終了 "
					*ptax  = (cur_val[0] * 100) + cur_val[1];									// 税率パラメータ更新
					*pdate = (((long)cur_val[2]) * 10000) + (cur_val[3] * 100) + cur_val[4];	// 変更基準日パラメータ更新
					changing = 0;																// 編集解除
					OpelogNo = OPLOG_SHOHIZEI;													// 操作履歴登録
					f_ParaUpdate.BIT.other = 1;													// 復電時にRAM上パラメータデータのSUM更新する
				}
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			}
			break;
		case KEY_TEN:	// 数字(テンキー)
			if (changing == 1) {
				BUZPI();
				if (in_val == -1){
					in_val = 0;
				}
				in_val = (short)( in_val % 10 ) * 10 + (short)(msg - KEY_TEN0);			// 2桁入力を数字に変換
			}
			break;
		case KEY_TEN_CL:	// 取消(テンキー)
			if (changing == 1) {
				BUZPI();
				in_val = -1;
			}
			break;
		default:
			break;
		}

		if( (changing == 1) && ((msg == KEY_TEN_F1) || (msg == KEY_TEN_F2) || (msg == KEY_TEN_F3) || (msg == KEY_TEN_F4) || (KEY_TEN0to9(msg) == KEY_TEN) || (msg == KEY_TEN_CL) ) ){
			// カーソル入力値表示
			if( in_val == -1 ){
				mnytax_dsp(cur_val[cur_pos], cur_pos, 1);	// 入力前の設定値を表示 反転
			} else {
				mnytax_dsp((ushort)in_val, cur_pos, 1);		// 入力値を表示 反転
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			日付存在チェック関数
//[]----------------------------------------------------------------------[]
///	@param[in]		y  : 年
///	@param[in]		m  : 月
///	@param[in]		d  : 日
///	@return			0  : 存在する日付   -1 : 存在しない日付
///	@author			
///	@note			None
///	@attention		chkdate(yyyy,mm,dd)だと月、日の入力値が0の場合でも
///					最小値を設定してしまうので、本関数では入力値0をエラー
///					にする。ただしすべて0は許容する。
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short	date_exist_check(short y, short m, short d)
{
	if ( y == 2000 && m == 0 && d == 0 ) {		// 00年00月00日は許容
		return ( 0 );
	}
	if( m == 0 ){
		return (-1);							// 月が0の場合は存在しない日として返す
	}
	if( d == 0 ){
		return (-1);							// 日が0の場合は存在しない日として返す
	}
	return chkdate( y, m, d);					// 存在する日付かどうかチェック
}
//[]----------------------------------------------------------------------[]
///	@brief			次カーソル位置算出
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		:F1/F2キーメッセージ
///	@param[in]		pos		:現在のカーソル位置
///	@return			次のカーソル位置
///	@author			
///	@note			None
///	@attention		カーソル位置: 変更前税率    =1
///					              変更後税率    =2
///					              変更基準日(年)=3
///					              変更基準日(月)=4
///					              変更基準日(日)=5
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static int	get_next_curpos(ushort msg, int pos)
{
	int next_pos;

	next_pos = pos;				// 現在のカーソル位置をセット

	if (msg == KEY_TEN_F1) {
		switch(pos){
		case 1:
		case 2:
		case 3:
		case 4:
			next_pos--;			// カーソル位置をひとつ前に移動する
			break;
		case 0:
			next_pos = 4;		// カーソル位置を変更基準日(日)に移動する
			break;
		default:
			break;
		}
	} else {	// F2
		switch(pos){
		case 0:
		case 1:
		case 2:
		case 3:
			next_pos++;			// カーソル位置をひとつ後に移動する
			break;
		case 4:
			next_pos = 0;		// カーソル位置を変更前税率に移動する
			break;
		default:
			break;
		}
	}

	return next_pos;
}

//[]----------------------------------------------------------------------[]
///	@brief			変更基準日データのチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: 入力値
///	@param[in]		pos		: 年(2) or 月(3) or 日(4)
///	@return			ret		: 0 = OK -1 = NG
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static int Date_Check( short data, int pos )
{
	switch( pos ){
		case 2:		//year
			if( data > 79 ){
				return( -1 );
			}
			break;
		case 3:		//month
			if( data > 12 ){		// 0は許容する
				return( -1 );
			}
			break;
		case 4:		//date
			if( data > 31 ){		// 0は許容する
				return( -1 );
			}
			break;
		default:
			return( -1 );
	}
	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief			消費税の税率表示
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : 表示する値
///	@param[in]		pos  : カーソル位置
///	@param[in]		rev  : 0:正転 1:反転
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	mnytax_dsp(ushort val, int pos, ushort rev)
{
	switch(pos){
	case 0:
		before_tax_dsp(val, rev);					// 変更前税率表示
		break;
	case 1:
		after_tax_dsp(val, rev);					// 変更後税率表示
		break;
	case 2:
		ChangeBaseDate_edit(0, val, rev);			// 変更基準日(年)表示
		break;
	case 3:
		ChangeBaseDate_edit(1, val, rev);			// 変更基準日(月)表示
		break;
	case 4:
		ChangeBaseDate_edit(2, val, rev);			// 変更基準日(日)表示
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			消費税の税率表示(初期表示)
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : 表示する値配列のポインタ
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	mnytax_dspinit_dsp(ushort* val)
{
	// 固定文字列設定
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );		// "＜消費税の税率＞　　　　　　　"
// MH810105(S) MH364301 インボイス対応 GT-4100
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188] );	// "　変更前税率：　　　　　　　　"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "　変更後税率：　　　　　　　　"
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "変更基準日　　　年　　月　　日"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "　変更前税率：　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "　変更後税率：　　　　　　　　"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[191] );	// "変更基準日　　　年　　月　　日"
// MH810105(E) MH364301 インボイス対応 GT-4100

	if(val[0] == 0) {
		grachr( 2, 14, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	// "消費税なし" 
	} else {
		before_tax_dsp(val[0], 0);						// 変更前税率表示 
	}
	if(val[1] == 0) {
		grachr( 3, 14, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	// "消費税なし"
	} else {
		after_tax_dsp(val[1], 0);						// 変更後税率表示
	}
	if((val[2] == 0) && (val[3] == 0) && (val[4] == 0)) {
		// 年月日全て0の場合は"－－"表示
		grachr( 5, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "－－"
		grachr( 5, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "－－"
		grachr( 5, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "－－"
		return;
	}
	ChangeBaseDate_dsp(val[2], val[3], val[4]);	// 変更基準日(年月日)表示
}

//[]----------------------------------------------------------------------[]
///	@brief			消費税の税率表示(変更ボタン押下後初期表示)
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : 表示する値配列のポインタ
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Co8pyright(C) 2022 AMANO Corp.---[]
static void	mnytax_editinit_dsp(ushort* val)
{
	// 固定文字列設定
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );		// "消費税の税率　　　　　　　　　"
// MH810105(S) MH364301 インボイス対応 GT-4100
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188] );	// "　変更前税率：　　　　　　　　"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "　変更後税率：　　　　　　　　"
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "変更基準日　　　年　　月　　日"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "　変更前税率：　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "　変更後税率：　　　　　　　　"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[191] );	// "変更基準日　　　年　　月　　日"
// MH810105(E) MH364301 インボイス対応 GT-4100

	before_tax_dsp(val[0], 0);					// 変更前税率表示 
	after_tax_dsp(val[1], 0);					// 変更後税率表示
	ChangeBaseDate_dsp(val[2], val[3], val[4]);	// 変更基準日(年月日)表示
}

//[]----------------------------------------------------------------------[]
///	@brief			変更前税率表示
//[]----------------------------------------------------------------------[]
///	@param[in]		tax  : 税率
///	@param[in]		rev  : 0:正転 1:反転
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	before_tax_dsp(ushort tax, ushort rev)
{
	opedsp( 2, 14, tax, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );		// 税率
	grachr( 2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );		// "％"
	grachr( 2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"    " );	// "消費税なし"の"なし"が表示されている場合にはここで消す
}

//[]----------------------------------------------------------------------[]
///	@brief			変更後税率表示
//[]----------------------------------------------------------------------[]
///	@param[in]		tax  : 税率
///	@param[in]		rev  : 0:正転 1:反転
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	after_tax_dsp(ushort tax, ushort rev)
{
	opedsp( 3, 14, tax, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );		// 税率
	grachr( 3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );		// "％"
	grachr( 3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"    " );	// "消費税なし"の"なし"が表示されている場合にはここで消す
}

//[]----------------------------------------------------------------------[]
///	@brief			変更基準日表示(初期表示)
//[]----------------------------------------------------------------------[]
///	@param[in]		y  : 年
///	@param[in]		m  : 月
///	@param[in]		d  : 日
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	ChangeBaseDate_dsp(ushort y, ushort m, ushort d)
{
	opedsp( 5, 12, y, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 年
	opedsp( 5, 18, m, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 月
	opedsp( 5, 24, d, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 日
}

//[]----------------------------------------------------------------------[]
///	@brief			変更基準日表示(編集)
//[]----------------------------------------------------------------------[]
///	@param[in]		kind : 変更対象(0:年、1:月、2:日)
///	@param[in]		val  : 設定値
///	@param[in]		rev  : 0:正転 1:反転
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	ChangeBaseDate_edit(uchar kind, ushort val, ushort rev)
{
	switch( kind ) {
	case 0:
		opedsp( 5, 12, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// 年
		break;
	case 1:
		opedsp( 5, 18, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// 月
		break;
	case 2:
		opedsp( 5, 24, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// 日
		break;
	default:
		break;
	}
}
// MH810105(E) MH364301 インボイス対応

/*[]----------------------------------------------------------------------[]*/
/*|  料金設定：Ａ～Ｌ種役割                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnykid( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura 時間入力を３桁までに変更             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* パラメータ上のA-L車種の役割/種別時間割引のINDEXを取得するマクロ */
#define	GET_KID_IDX(k)			(1+6*(k))
unsigned short	UsMnt_mnykid(void)
{
	ushort	msg;
	char	set;		/* 0:種別設定中 1:役割設定中 2:割引時間設定中 */
	int		kind;		/* 種別(0-11(=A-L)) */
	char	top;
	long	data;
	char	i;
	long	*pSHA;

	pSHA = CPrmSS[S_SHA];

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[5] );			/* "＜Ａ～Ｌ種役割＞　　　　　　　" */
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[19]);			/* "【種別－役　割】 【割引時間】 " */
	for (i = 0; i < 5; i++) {
		grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);			/* 種別 */
		grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" */
		grachr((ushort)(2+i), 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);			/* "ー" */
		mnykid_role_dsp(0, i, pSHA[GET_KID_IDX(i)], 0);		/* 役割 */
		mnykid_time_dsp(0, i, pSHA[GET_KID_IDX(i)], 0);		/* 時間 */
	}
	grachr(2, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* "Ａ" 反転 */
	grachr(2, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "種" 反転 */
	Fun_Dsp( FUNMSG2[8] );														/* "　▲　　▼　　→　 　　  終了 " */

	set = 0;
	kind = top = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( KEY_TEN0to9(msg) ) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* 種別設定中 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					kind--;
					if (kind < 0) {
						kind = 11;
						top = 7;
					}
					else if (kind < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[top+i]);						/* 種別 */
						grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* "種" */
						mnykid_role_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);	/* 役割 */
						mnykid_time_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);	/* 時間 */
					}
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 反転 */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 反転 */
					break;
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					kind++;
					if (kind > 11) {
						kind = top = 0;
					}
					else if (kind > top+4) {
						top++;
					}
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[top+i]);			/* 種別 */
						grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "種" */
						mnykid_role_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);						/* 役割 */
						mnykid_time_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);						/* 時間 */
					}
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 反転 */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 反転 */
					break;
				case KEY_TEN_F3:	/* →(F3) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 正転 */
					grachr((ushort)(2+kind-top), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 正転 */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);										/* 役割 反転 */
// MH810100(S) 2020/09/11 #4834 現状未対応の種別割引が共通パラメータにて設定の利用が可能となっている
//					Fun_Dsp( FUNMSG2[4] );												 /* "　←　　→　 変更　書込  終了 " */
					Fun_Dsp( FUNMSG2[65] );												 /* "　←　　 　 変更　書込  終了 " */
// MH810100(E) 2020/09/11 #4834 現状未対応の種別割引が共通パラメータにて設定の利用が可能となっている
					data = pSHA[GET_KID_IDX(kind)];
					set = 1;
					break;
				default:
					break;
				}
			}
			/* 役割設定中 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 反転 */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 反転 */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* 役割 正転 */
					Fun_Dsp( FUNMSG2[8] );														 /* "　▲　　▼　　→　 　　  終了 " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
// GG121802(S) R.Endo 2023/09/08 #7129 メンテナンスの料金設定で「A～L種役割」ページの役割変更時にF2キーが有効になっている
					BUZPI();
// GG121802(E) R.Endo 2023/09/08 #7129 メンテナンスの料金設定で「A～L種役割」ページの役割変更時にF2キーが有効になっている
					pSHA[GET_KID_IDX(kind)] = data;
					OpelogNo = OPLOG_ALYAKUWARI;			// 操作履歴登録
					f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
// MH810100(S) 2020/09/11 #4834 現状未対応の種別割引が共通パラメータにて設定の利用が可能となっている
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 反転 */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 反転 */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* 役割 正転 */
					Fun_Dsp( FUNMSG2[8] );														 /* "　▲　　▼　　→　 　　  終了 " */
					set = 0;
					break;
// MH810100(E) 2020/09/11 #4834 現状未対応の種別割引が共通パラメータにて設定の利用が可能となっている
				case KEY_TEN_F2:	/* →(F2) */
// GG121802(S) R.Endo 2023/09/08 #7129 メンテナンスの料金設定で「A～L種役割」ページの役割変更時にF2キーが有効になっている
// 					BUZPI();
// 					/* 車種役割=使用する */
// 					if (pSHA[GET_KID_IDX(kind)]/10000) {
// 						/* 割引時間設定へ */
// 						mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);		/* 役割 正転 */
// 						mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);		/* 時間 反転 */
// 						Fun_Dsp( FUNMSG2[2] );						/* "　←　　→　　　　 書込  終了 " */
// 						data = -1;
// 						set = 2;
// 					}
// 					/* 車種役割=未使用 */
// 					else {
// 						/* 種別設定へ */
// 						grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 反転 */
// 						grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 反転 */
// 						mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);										/* 役割 正転 */
// 						mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);										/* 時間 正転 */
// 						Fun_Dsp( FUNMSG2[8] );												/* "　▲　　▼　　→　 　　  終了 " */
// 						set = 0;
// 					}
// GG121802(E) R.Endo 2023/09/08 #7129 メンテナンスの料金設定で「A～L種役割」ページの役割変更時にF2キーが有効になっている
					break;
				case KEY_TEN_F3:	/* 変更(F3) */
					BUZPI();
					if (data/10000)
						data %= 10000;
					else
						data += 10000;
					mnykid_role_dsp(top, kind, data, 1);		/* 役割 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* 役割 反転 */
					data = pSHA[GET_KID_IDX(kind)];
					break;
				default:
					break;
				}
			}
			/* 割引時間設定中 */
			else {	/*if (set == 2) */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* 役割 反転 */
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);	/* 時間 正転 */
					Fun_Dsp( FUNMSG2[4] );						/* "　←　　→　 変更　書込  終了 " */
					data = pSHA[GET_KID_IDX(kind)];
					set = 1;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						if (data%100 > 59) {
							BUZPIPI();
							mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* 時間 反転 */
							data = -1;
							break;
						}
						pSHA[GET_KID_IDX(kind)] = 10000+data;
						OpelogNo = OPLOG_ALYAKUWARI;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 反転 */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 反転 */
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* 時間 正転 */
					Fun_Dsp( FUNMSG2[8] );														 /* "　▲　　▼　　→　 　　  終了 " */
					set = 0;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnykid_time_dsp(top, kind, 10000+data, 1);	/* 時間 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* 時間 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  車種役割表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   top  : 先頭種別                                  |*/
/*|              : char   kind : 種別                                      |*/
/*|              : long   data : 時間                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnykid_role_dsp(char top, char kind, long data, ushort rev)
{
	/* 車種役割=使用する */
	if (data/10000) {
		grachr((ushort)(2+kind-top), 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[5]);		/* "料金種別" */
	}
	/* 車種役割=未使用 */
	else {
		grachr((ushort)(2+kind-top), 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);		/* "未使用　" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  割引時間表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   top  : 先頭種別                                  |*/
/*|              : char   kind : 種別                                      |*/
/*|              : long   data : 時間                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura 時間表示を３桁までに変更             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnykid_time_dsp(char top, char kind, long data, ushort rev)
{
	/* 車種役割=使用する */
	if (data/10000) {
		data %= 10000;
		grachr((ushort)(2+kind-top), 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "ー" */
		opedsp((ushort)(2+kind-top), 18, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);									/* 時 */
		grachr((ushort)(2+kind-top), 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
		opedsp((ushort)(2+kind-top), 24, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);									/* 分 */
	}
	/* 車種役割=未使用 */
	else {
		grachr((ushort)(2+kind-top), 16, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "　" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金帯単位時間の設定Indexを取得する                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_TIME_IDX                                            |*/
/*| PARAMETER    : n(料金体系),  b(料金帯),  m(基本/追加)                  |*/
/*| RETURN VALUE : index                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	GET_TIME_IDX(char n, char b, char m){
	
	short	index;
	short	wk;
	wk = (n%3 == 0) ? 3 : n%3;
	index = (RYO_TAIKEI_SETCNT*(wk-1)+6)+(4*((b)-1))+m;
	
	return index;
}
	
/*[]----------------------------------------------------------------------[]*/
/*| 基本/追加料金の設定Indexを取得する                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_MONEY_IDX                                           |*/
/*| PARAMETER    :  n(料金体系),  k(料金種別),b(料金帯),  m(基本/追加)     |*/
/*| RETURN VALUE : index                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	GET_MONEY_IDX(char n, char k, char b, char m){
	
	short	index;
	short	wk;
	wk = (n%3 == 0) ? 3 : n%3;
	index = (RYO_TAIKEI_SETCNT*(wk-1)+64)+((RYO_SYUBET_SETCNT*k)+(2*((b)-1)))+m;
	
	return index;
}
/*[]----------------------------------------------------------------------[]*/
/*| 料金設定：単位時間／料金                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnycha( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnycha(void)
{
	ushort	msg;
	char	set;	/* 0:料金体系設定中 1:種別設定中 2:料金帯設定中 3:基本-時間 4:基本-料金 5:追加-時間 6:追加-料金 */
	char	no;		/* 料金体系(1-3) */
	int		kind;	/* 種別(0-11) */
	char	band;	/* 料金帯(1-6) */
	long	data;
	long	*pRAT;

	pRAT = CPrmSS[S_RAT];
	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[6]);				/* "＜単位時間／料金＞　　　　　　" */
	grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[14]);			/* "（昼夜帯）" */
	grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "第　料金体系" 反転 */
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* "１" 反転 */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "ー" */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* 種別 */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "種" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "ー" */
	grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);			/* "第　料金帯" */
	opedsp(2, 22, 1, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* "１" */

	grachr(4, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);							/* "基本" */
	mnycha_time_dsp(0, pRAT[GET_TIME_IDX(1,1,0)], 0);		/* 時間 */
	grachr(4, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "ー" */
	mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(1,0,1,0)], 0);	/* 料金 */
	opedsp(4, 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "０" */
	grachr(4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "円" */

	grachr(5, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[1]);							/* "追加" */
	mnycha_time_dsp(1, pRAT[GET_TIME_IDX(1,1,1)], 0);		/* 時間 */
	grachr(5, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "ー" */
	mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(1,0,1,1)], 0);	/* 料金 */
	opedsp(5, 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "０" */
	grachr(5, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "円" */

	Fun_Dsp( FUNMSG[25] );					/* "　▲　　▼　　　　 読出  終了 " */

	set = 0;
	no = 1;
	kind = 0;
	band = 1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* 料金体系設定中 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if(--no < 1){
							no = 9;			
						}
					}
					else {
						if(++no > 9){
							no = 1;
						}
					}
					/* 切替えた料金体系の設定位置を指定 */
					pRAT = CPrmSS[GET_PARSECT_NO(no)];			// 設定位置更新
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		/* "第　料金体系" 反転 */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 体系           反転 */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);				/* 基本-時間 */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);		/* 基本-料金 */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);				/* 追加-時間 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);		/* 追加-料金 */
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);									/* "第　料金体系" 正転 */
					opedsp(2, 2, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* 体系           正転 */
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* "種" 反転 */
					Fun_Dsp( FUNMSG[28] );																			/* "  ▲    ▼    ←   読出  終了 " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* 種別設定中 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 11;
					}
					else {
						if (++kind > 11)
							kind = 0;
					}
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);	/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);		/* "種" 反転 */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);				/* 基本-時間 */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);		/* 基本-料金 */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);				/* 追加-時間 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);		/* 追加-料金 */
					break;
				case KEY_TEN_F3:	/* ←(F3) */
					BUZPI();
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		/* "第　料金体系" 反転 */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 体系           反転 */
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);	/* 種別 正転 */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);		/* "種" 正転 */
					Fun_Dsp( FUNMSG[25] );						 /* "　▲　　▼　　　　 読出  終了 " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 正転 */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 正転 */
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "第　料金帯" 反転 */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 帯 　　　　　反転 */
					set = 2;
					break;
				default:
					break;
				}
			}
			/* 料金帯設定中 */
			else if (set == 2) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--band < 1)
							band = 6;
					}
					else {
						if (++band > 6)
							band = 1;
					}
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);			/* "第　料金帯" 反転 */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 帯 　　　　　反転 */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);						/* 基本-時間 */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);				/* 基本-料金 */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);						/* 追加-時間 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);				/* 追加-料金 */
					break;
				case KEY_TEN_F3:	/* ←(F3) */
					BUZPI();
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "種" 反転 */
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "第　料金帯" 正転 */
					opedsp(2, 22, (ushort)band, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 帯 　　　　　正転 */
					set = 1;
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "第　料金帯" 正転 */
					opedsp(2, 22, (ushort)band, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 帯           正転 */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);					/* 基本-時間    反転 */
					Fun_Dsp( FUNMSG2[2] );													/* "　←　　→　　　　 書込  終了 " */
					data = -1;
					set = 3;
					break;
				default:
					break;
				}
			}
			/* 基本-時間 */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "第　料金帯" 反転 */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 帯           反転 */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);					/* 基本-時間    正転 */
					Fun_Dsp( FUNMSG[28] );									 /* "  ▲    ▼    ←   読出  終了 " */
					set = 2;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						if ( (data/100 >23) || (data%100 > 59)) {
							BUZPIPI();
							mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);	/* 基本-時間 反転 */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX(no,band,0)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);			/* 基本-時間 正転 */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* 基本-料金 反転 */
					data = -1;
					set = 4;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp(0, data, 1);		/* 基本-時間 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* 基本-時間 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 基本-料金 */
			else if (set == 4) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* 基本-時間 反転 */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);	/* 基本-料金 正転 */
					data = -1;
					set = 3;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX(no,kind,band,0)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);	/* 基本-料金 正転 */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* 追加-時間 反転 */
					data = -1;
					set = 5;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp(0, data, 1);	/* 基本-料金 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* 基本-料金 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 追加-時間 */
			else if (set == 5) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* 基本-料金 反転 */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);			/* 追加-時間 正転 */
					data = -1;
					set = 4;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						if ( (data/100 >23) || (data%100 > 59)) {
							BUZPIPI();
							mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);	/* 追加-時間 反転 */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX(no,band,1)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);			/* 追加-時間 正転 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 1);	/* 追加-料金 反転 */
					data = -1;
					set = 6;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp(1, data, 1);		/* 追加-時間 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* 追加-時間 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 追加-料金 */
			else {	/* if (set == 6) */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ←(F1) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* 追加-時間 反転 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);	/* 追加-料金 正転 */
					data = -1;
					set = 5;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX(no,kind,band,1)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* →(F2) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* 基本-時間 反転 */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);	/* 追加-料金 正転 */
					data = -1;
					set = 3;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp(1, data, 1);	/* 追加-料金 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 1);	/* 追加-料金 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  単位時間表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   mode : 0:基本 1:追加                             |*/
/*|              : long   time : 時間                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_time_dsp(char mode, long time, ushort rev)
{
	ushort	line;

	line = (mode == 0) ? 4 : 5;
	opedsp(line, 6, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 時 */
	grachr(line, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
	opedsp(line, 12, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
}

/*[]----------------------------------------------------------------------[]*/
/*|  単位料金表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( mode, money, rev )                     |*/
/*| PARAMETER    : char   mode  : 0:基本 1:追加                            |*/
/*|              : long   money : 時間                                     |*/
/*|              : ushort rev   : 0:正転 1:反転                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_money_dsp(char mode, long money, ushort rev)
{
	ushort	line;

	line = (mode == 0) ? 4 : 5;
	opedsp(line, 18, (ushort)(money/10), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 料金 */
	if (money/10 == 0)
		grachr(line, 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金設定：単位時間／料金                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnycha( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* パラメータ上の単位時間のINDEXを取得するマクロ */
#define	GET_TIME_IDX_DEC(n, p)			(300*((n)-1)+11+2*((p)-1))
/* パラメータ上の単位料金のINDEXを取得するマクロ */
#define	GET_MONEY_IDX_DEC(n, k, p)		(300*((n)-1)+63+20*(k)+(p))
/* パラメータ上の単位係数のINDEXを取得するマクロ */
#define	GET_COEFFICIENT_IDX_DEC(n, p)	(300*((n)-1)+11+2*((p)-1)+1)
unsigned short	UsMnt_mnycha_dec(void)
{
	ushort	msg;
	char	set;	/* 0:料金体系設定中 1:種別設定中 2:料金パターン設定中 3:単位時間 4:単位料金 5:単位係数 */
	char	no;		/* 料金体系(1-3) */
	int		kind;	/* 種別(0-11) */
	int		pattern;/* パターン(0-12) ０：深夜帯*/
	long	data;
	long	*pRAT;

	pRAT = CPrmSS[S_RAT];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[6]);				/* "＜単位時間／料金＞　　　　　　" */
	grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[15]);			/* "（逓減帯）" */
	grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "第　料金体系" 反転 */
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* "１" 反転 */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "－" */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* 種別 */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "種" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "－" */
	grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[1]);			/* "第１ﾊﾟﾀｰﾝ " */

	grachr(4, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[27]);				/* "単位時間" */
	mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(1,1)], 0);						/* 時間 */

	grachr(5, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[28]);				/* "単位料金" */
	mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(1,0,1)], 0);					/* 料金 */
	opedsp(5, 22, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "０" */
	grachr(5, 24, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);				/* "円" */

	grachr(6, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[29]);				/* "単位係数" */
	mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(1,1)], 0);			/* 係数 */

	Fun_Dsp( FUNMSG[25] );														/* "　▲　　▼　　　　 読出  終了 " */

	set = 0;
	no = 1;
	kind = 0;
	pattern = 1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* 料金体系設定中 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--no < 1)
							no = 3;
					}
					else {
						if (++no > 3)
							no = 1;
					}
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);					/* "第　料金体系" 反転 */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 体系           反転 */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* 時間 */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);	/* 係数 */
					}
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);				/* 料金 */
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "第　料金体系" 正転 */
					opedsp(2, 2, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* 体系           正転 */
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);			/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "種" 反転 */
					Fun_Dsp( FUNMSG[28] );														/* "  ▲    ▼    ←   読出  終了 " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* 種別設定中 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 11;
					}
					else {
						if (++kind > 11)
							kind = 0;
					}
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 反転 */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* 時間 */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);	/* 係数 */
					}
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);				/* 料金 */
					break;
				case KEY_TEN_F3:	/* ←(F3) */
					BUZPI();
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);					/* "第　料金体系" 反転 */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 体系           反転 */
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 正転 */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 正転 */
					Fun_Dsp( FUNMSG[25] );															/* "　▲　　▼　　　　 読出  終了 " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* 種別 正転 */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "種" 正転 */
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);			/* "第Ｘﾊﾟﾀｰﾝ " 反転 */
					set = 2;
					break;
				default:
					break;
				}
			}
			/* 料金パターン設定中 */
			else if (set == 2) {
				switch (msg) {
				case KEY_TEN_F1:	/* ▲(F1) */
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--pattern < 0)
							pattern = 12;
					}
					else {
						if (++pattern > 12)
							pattern = 0;
					}
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "第Ｘﾊﾟﾀｰﾝ " 反転 */
					if (pattern != 0) {
						grachr(4, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[27]);							/* "単位時間" */
						grachr(5, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[28]);							/* "単位料金" */
						opedsp(5, 22, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "０" */
						grachr(5, 24, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "円" */
						grachr(6, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[29]);							/* "単位係数" */
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);								/* 時間 */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);						/* 料金 */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);				/* 係数 */
					} else {
						grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);						/* "　" */
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);						/* "　" */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);						/* 料金 */
					}

					break;
				case KEY_TEN_F3:	/* ←(F3) */
					BUZPI();
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* 種別 反転 */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* "種" 反転 */
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "第　　ﾊﾟﾀｰﾝ " 正転 */
					set = 1;
					break;
				case KEY_TEN_F4:	/* 読出(F4) */
					BUZPI();
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "第　　ﾊﾟﾀｰﾝ " 正転 */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* 時間    反転 */
						Fun_Dsp( FUNMSG[20] );					/* "　▲　　▼　　　　 書込  終了 " */
						set = 3;
					} else {
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* 料金 反転 */
						Fun_Dsp( FUNMSG[22] );					/* "　▲　　　　　　　 書込  終了 " */
						set = 4;
					}
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 時間 */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);		/* "第　　ﾊﾟﾀｰﾝ " 反転 */
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* 時間         正転 */
					Fun_Dsp( FUNMSG[28] );														/* "  ▲    ▼    ←   読出  終了 " */
					set = 2;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						if (data%100 > 59) {
							// 分が５９以上だと元の表示に戻す。
							BUZPIPI();
							mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);	/* 時間 反転 */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX_DEC(no,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);			/* 時間 正転 */
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* 料金 反転 */
					data = -1;
					set = 4;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp_dec(data, 1);		/* 時間 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* 時間 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 料金 */
			else if (set == 4) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* 時間 反転 */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);	/* 料金 正転 */
						set = 3;
					} else {
						grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);	/* "第　　ﾊﾟﾀｰﾝ " 反転 */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);		/* 料金 正転 */
						Fun_Dsp( FUNMSG[28] );													/* "  ▲    ▼    ←   読出  終了 " */
						set = 2;
					}
					data = -1;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* ▼(F2) */
					if( pattern != 0 || KEY_TEN0to9(msg) != KEY_TEN_F2)
						BUZPI();
					if (pattern != 0) {
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);	/* 料金 正転 */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 1);			/* 係数 反転 */
						set = 5;
					}
					data = -1;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp_dec(data, 1);	/* 料金 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* 料金 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* 係数 */
			else {	/*if (set == 5) {*/
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ▲(F1) */
					BUZPI();
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* 料金 反転 */
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);			/* 係数 正転 */
					data = -1;
					set = 4;
					break;
				case KEY_TEN_F4:	/* 書込(F4) */
					if (data != -1) {
						pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
				case KEY_TEN_F2:	/* ▼(F2) */
					BUZPI();
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);			/* 係数 正転 */
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* 時間    反転 */
					data = -1;
					set = 3;
					break;
				case KEY_TEN:		/* 数字(テンキー) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)) % 10000;
					mnycha_coefficient_dsp_dec(data, 1);	/* 係数 反転 */
					break;
				case KEY_TEN_CL:	/* 取消(テンキー) */
					BUZPI();
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 1);			/* 係数 反転 */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  単位時間表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_time_dsp_dec(time, rev)                          |*/
/*| PARAMETER    : long   time : 時間                                      |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_time_dsp_dec(long time, ushort rev)
{
	opedsp(4, 16, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
	grachr(4, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
	opedsp(4, 22, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
}

/*[]----------------------------------------------------------------------[]*/
/*|  単位料金表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_money_dsp_dec(money, rev)                        |*/
/*| PARAMETER    : long   money : 料金                                     |*/
/*|              : ushort rev   : 0:正転 1:反転                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_money_dsp_dec(long money, ushort rev)
{
	opedsp(5, 14, (ushort)(money/10), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 料金 */
	if (money/10 == 0)
		grachr(5, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
}

/*[]----------------------------------------------------------------------[]*/
/*|  単位係数表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_coefficient_dsp_dec(coefficient, rev)            |*/
/*| PARAMETER    : long   coefficient : 係数                                     |*/
/*|              : ushort rev   : 0:正転 1:反転                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_coefficient_dsp_dec(long coefficient, ushort rev)
{
	opedsp(6, 18, (ushort)(coefficient), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 係数 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金設定：定期無効曜日                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnytweek( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* 各曜日が有効なら０を返すマクロ */
#define	IS_SUN(d)	((((d)%1000000L) < 100000L) ? 0 : 1)
#define	IS_MON(d)	((((d)%100000L) < 10000L) ? 0 : 1)
#define	IS_TUE(d)	((((d)%10000) < 1000) ? 0 : 1)
#define	IS_WED(d)	((((d)%1000) < 100) ? 0 : 1)
#define	IS_THU(d)	((((d)%100) < 10) ? 0 : 1)
#define	IS_FRI(d)	((((d)%10) < 1) ? 0 : 1)
#define	IS_SAT(d)	((((d)%1000000L) < 100000L) ? 0 : 1)
#define	IS_SP0(d)	((((d)%100000L) < 10000L) ? 0 : 1)
#define	IS_SP1(d)	((((d)%10000) < 1000) ? 0 : 1)
#define	IS_SP2(d)	((((d)%1000) < 100) ? 0 : 1)
#define	IS_SP3(d)	((((d)%100) < 10) ? 0 : 1)
unsigned short	UsMnt_mnytweek(void)
{
	ushort	msg;
	char	changing;
	char	pos;		/* 0:曜日設定 1:期間設定 */
	char	no;			/* 定期番号(1-15) */
	long	*pdata;
	long	tbl[] = {100000,10000,1000,100,10,1};

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "＜定期無効曜日＞　　　　　　　" */
	grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "定期" 反転 */
	opedsp(2, 8, 1, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* "０１" 反転 */
	grachr(2, 12, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[2]);		/* "の無効曜日" */
	mnytweek_dsp(1);														/* 無効曜日 */
	Fun_Dsp(FUNMSG2[9]);													/* "　▲　　▼　 変更　　　　終了 " */

	changing = 0;
	no = 1;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {	
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (changing == 0) {
			/* 定期番号-選択中 */
			switch (msg) {
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				if (msg == KEY_TEN_F1) {
					if (--no < 1)
						no = 15;
				}
				else {
					if (++no > 15)
						no = 1;
				}
				opedsp(2, 8, (ushort)no, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 反転 */
				mnytweek_dsp(no);						/* 無効曜日 */
				break;
			case KEY_TEN_F3:	/* 変更(F3) */
				BUZPI();
				pdata = &CPrmSS[S_PAS][6+(no-1)*10];
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "＜定期無効曜日＞　　　　　　　" */
				grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "定期" */
				opedsp(2, 4, (ushort)no, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 */
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[22]);		/* "　　　　１２３４５６７　　８　" */
				grachr(4, 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[5]);			/* "曜日" 反転 */
				grachr(4,  8, 2, IS_MON(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "月" */
				grachr(4, 10, 2, IS_TUE(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "火" */
				grachr(4, 12, 2, IS_WED(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "水" */
				grachr(4, 14, 2, IS_THU(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "木" */
				grachr(4, 16, 2, IS_FRI(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "金" */
				grachr(4, 18, 2, IS_SAT(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "土" */
				grachr(4, 20, 2, IS_SUN(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "日" */
				grachr(4, 24, 6, IS_SP0(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "特別日" */
				grachr(5, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);					/* "期間" 正転 */
				grachr(5,  8, 4, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
				opedsp(5, 12, 1, 1, 0, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "１" */
				grachr(5, 16, 4, IS_SP2(pdata[1]),   COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);/* "期間" */
				opedsp(5, 20, 2, 1, 0, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "２" */
				grachr(5, 24, 4, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
				opedsp(5, 28, 3, 1, 0, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "３" */
				Fun_Dsp(FUNMSG[6]);			/* "　▲　　▼　 　　　　　　終了 " */
				changing = 1;
				pos = 0;
				OpelogNo = OPLOG_TEIKIMUKOYOUBI;		// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				break;
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
		else {
			/* 無効曜日-設定中 */
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
				BUZPI();
				pos ^= 1;
				grachr(4, 2, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[5]);			/* "曜日" */
				grachr(5, 2, 4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);			/* "期間" */
				break;
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "＜定期無効曜日＞　　　　　　　" */
				grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "定期" 反転 */
				opedsp(2, 8, (ushort)no, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 番号 反転 */
				grachr(2, 12, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[2]);		/* "の無効曜日" */
				mnytweek_dsp(no);														/* 無効曜日 */
				Fun_Dsp(FUNMSG2[9]);													/* "　▲　　▼　 変更　　　　終了 " */
				changing = 0;
				break;
			case KEY_TEN:		/* 数字(テンキー) */
				if (pos == 0) {
					switch (msg) {
					case KEY_TEN1:
						BUZPI();
						if (IS_MON(pdata[0]))
							pdata[0] -= tbl[1];
						else
							pdata[0] += tbl[1];
						grachr(4,  8, 2, IS_MON(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "月" */
						break;
					case KEY_TEN2:
						BUZPI();
						if (IS_TUE(pdata[0]))
							pdata[0] -= tbl[2];
						else
							pdata[0] += tbl[2];
						grachr(4, 10, 2, IS_TUE(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "火" */
						break;
					case KEY_TEN3:
						BUZPI();
						if (IS_WED(pdata[0]))
							pdata[0] -= tbl[3];
						else
							pdata[0] += tbl[3];
						grachr(4, 12, 2, IS_WED(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "水" */
						break;
					case KEY_TEN4:
						BUZPI();
						if (IS_THU(pdata[0]))
							pdata[0] -= tbl[4];
						else
							pdata[0] += tbl[4];
						grachr(4, 14, 2, IS_THU(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "木" */
						break;
					case KEY_TEN5:
						BUZPI();
						if (IS_FRI(pdata[0]))
							pdata[0] -= tbl[5];
						else
							pdata[0] += tbl[5];
						grachr(4, 16, 2, IS_FRI(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "金" */
						break;
					case KEY_TEN6:
						BUZPI();
						if (IS_SAT(pdata[1]))
							pdata[1] -= tbl[0];
						else
							pdata[1] += tbl[0];
						grachr(4, 18, 2, IS_SAT(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "土" */
						break;
					case KEY_TEN7:
						BUZPI();
						if (IS_SUN(pdata[0]))
							pdata[0] -= tbl[0];
						else
							pdata[0] += tbl[0];
						grachr(4, 20, 2, IS_SUN(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "日" */
						break;
						break;
					case KEY_TEN8:
						BUZPI();
						if (IS_SP0(pdata[1]))
							pdata[1] -= tbl[1];
						else
							pdata[1] += tbl[1];
						grachr(4, 24, 6, IS_SP0(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "特別日" */
						break;
					default:
						break;
					}
				}
				else {
					switch (msg) {
					case KEY_TEN1:
						BUZPI();
						if (IS_SP1(pdata[1]))
							pdata[1] -= tbl[2];
						else
							pdata[1] += tbl[2];
						grachr(5,  8, 4, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
						opedsp(5, 12, 1, 1, 0, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "１" */
						break;
					case KEY_TEN2:
						BUZPI();
						if (IS_SP2(pdata[1]))
							pdata[1] -= tbl[3];
						else
							pdata[1] += tbl[3];
						grachr(5, 16, 4, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
						opedsp(5, 20, 2, 1, 0, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "２" */
						break;
					case KEY_TEN3:
						BUZPI();
						if (IS_SP3(pdata[1]))
							pdata[1] -= tbl[4];
						else
							pdata[1] += tbl[4];
						grachr(5, 24, 4, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
						opedsp(5, 28, 3, 1, 0, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "３" */
						break;
					default:
						break;
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
/*|  無効曜日表示                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytweek_dsp( no )                                      |*/
/*| PARAMETER    : char no : 定期番号                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytweek_dsp(char no)
{
	long	*p;

	p = &CPrmSS[S_PAS][6+(no-1)*10];
	if (p[0] == 0 && p[1] == 0) {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[21]);		/* "　　なし（全曜日有効）　　　　" */
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "　　　　　　　　　　　　　　　" */
	}
	else {
		grachr(4,  4, 2, IS_MON(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "月" */
		grachr(4,  6, 2, IS_TUE(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "火" */
		grachr(4,  8, 2, IS_WED(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "水" */
		grachr(4, 10, 2, IS_THU(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "木" */
		grachr(4, 12, 2, IS_FRI(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "金" */
		grachr(4, 14, 2, IS_SAT(p[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "土" */
		grachr(4, 16, 2, IS_SUN(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "日" */
		grachr(4, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "　" */
		grachr(4, 20, 6, IS_SP0(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "特別日" */
		grachr(5,  4, 4, IS_SP1(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
		opedsp(5,  8, 1, 1, 0, IS_SP1(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "１" */
		grachr(5, 12, 4, IS_SP2(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
		opedsp(5, 16, 2, 1, 0, IS_SP2(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "２" */
		grachr(5, 20, 4, IS_SP3(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "期間" */
		opedsp(5, 24, 3, 1, 0, IS_SP3(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "３" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：拡張機能                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Extendmenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Extendmenu(void)
{
	unsigned short	usUextEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[51]);		/* "＜拡張機能＞　　　　　　　　　" */

		usUextEvent = Menu_Slt((void*)USM_ExtendMENU , (void*)USM_ExtendMENU_TBL , (char)Ext_Menu_Max, 1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usUextEvent) {

		/* 複数台数集計 */
		case MNT_FTOTL:
			usUextEvent = UsMnt_FTotal();
			break;

		/* クレジット処理 */
		case MNT_CREDIT:
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//			usUextEvent = UsMnt_CreditMnu();
			BUZPIPI();
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		case MNT_EDY:
//			usUextEvent = UsMnt_EdyMnu();
//			break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		/* Ｓｕｉｃａ処理 */
		case MNT_SCA:
			usUextEvent = UsMnt_SuicaMnu();
			break;
		case MNT_HOJIN:					/* 法人カード */
			BUZPIPI();
			break;
		case MNT_DLOCK:					// 電磁ロック解除
			usUextEvent = Cardress_DoorOpen();
			break;
		/* ＭＴ集計 */
		case MNT_MTOTL:
			usUextEvent = UsMnt_Total(MNT_MTOTL);
			break;
// MH321800(S) D.Inaba ICクレジット対応 (決済リーダ処理追加)
		/* 決済リーダ処理 */
		case MNT_ECR:
			usUextEvent =  UsMnt_ECReaderMnu();
			break;
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダ処理追加)
// MH810104 GG119201(S) 電子ジャーナル対応
		/* 電子ジャーナル */
		case MNT_EJ:
			usUextEvent =  UsMnt_EJournalMnu();
			break;
// MH810104 GG119201(E) 電子ジャーナル対応
		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return MOD_EXT;
		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
			SetChange = 1;			// FLASHｾｰﾌﾞ指示
		}
		if (usUextEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (usUextEvent == MOD_CUT) {	
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：拡張機能 - 複数台数集計                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ftotal( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_FTotal(void)
{
	ushort	msg;
	uchar	dsp;
	int		pos;
	ushort	ret;

	if( (prm_get(COM_PRM, S_TOT,  2, 1, 1) == 0) ||		// 複数台数集計なし
		(prm_get(COM_PRM, S_NTN, 26, 1, 2) == 0) ||
		_is_ntnet_remote()	||						// センター接続？
		(OPECTL.Mnt_lev < 2) )
	{
		BUZPIPI();
		return MOD_EXT;	
	}

	pos = 0;
	dsp = 1;
	for ( ; ; ) {
		if (dsp) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[52]);		/* "＜複数台数集計（拡張）＞　　　" */
			ftotal_dsp(0, (pos==0)?1:0);											/* "小計プリント" */
			ftotal_dsp(1, (pos==1)?1:0);											/* "合計プリント" */
			ftotal_dsp(2, (pos==2)?1:0);											/* "前回合計プリント" */
			Fun_Dsp(FUNMSG[68]);													/* "　▲　　▼　　　　 実行  終了 " */
			dsp = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:	/* ﾓｰﾄﾞﾁｪﾝｼﾞ */
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了 */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ▲ */
		case KEY_TEN_F2:	/* ▼ */
			BUZPI();
			ftotal_dsp(pos, 0);			/* 前回項目-正転表示 */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 2;
			}
			else {
				if (++pos > 2)
					pos = 0;
			}
			ftotal_dsp(pos, 1);			/* 今回項目-反転表示 */
			break;
		case KEY_TEN_F4:	/* 実行 */
			BUZPI();
			ret = ftotal_print(pos);	/* プリント実行 */
			if (ret == MOD_CHG)
				return MOD_CHG;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if (ret == LCD_DISCONNECT)
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  複数台数合計表示                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ftotal_dsp(pos, rev)                                    |*/
/*| PARAMETER    : char   pos : 表示項目(0:小計 1:合計 2:前回合計)         |*/
/*|              : ushort rev : 0:正転 1:反転                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ftotal_dsp(char pos, ushort rev)
{
	const uchar	*str;
	ushort	len;

	switch (pos) {
	case 0:
		str = &TGTSTR[2][2];	/* "小計プリント" */
		len = 12;
		break;
	case 1:
		str = &TGTSTR[3][2];	/* "合計プリント" */
		len = 12;
		break;
	default:	/* case 2: */
		str = &TGTSTR[4][2];	/* "前回合計プリント" */
		len = 16;
		break;
	}
	grachr((ushort)(2+pos), 2, len, rev, COLOR_BLACK, LCD_BLINK_OFF, str);
}

/*[]----------------------------------------------------------------------[]*/
/*|  複数台数合計 プリント実行                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ftotal_print(kind)                                      |*/
/*| PARAMETER    : char  kind : 選択種別(0:小計 1:合計 2:前回合計)         |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static ushort	ftotal_print(char kind)
{
	ushort			msg;
	uchar			mode;
	T_FrmSyuukei	FrmSyuukei;
	T_FrmPrnStop	FrmPrnStop;
	ushort			type;
	ushort			log;
	char			inji_end;
	ushort			rev;
	ushort			result;
	ushort			tmout;
	ulong			req;
	char			value;
	uchar			mnt_sw = 0;		// 印字中にメンテナンスOFFになったかどうか

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[52]);		/* "＜複数台数集計（拡張）＞　　　" */
	if (kind == 0) {
		grachr(1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[8]);		/* "複数台数小計 */
		req = NTNET_DATAREQ2_MSYOUKEI;
		value = 0;
	}
	else if (kind == 1) {
		grachr(1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[9]);		/* "複数台数合計 */
		req = NTNET_DATAREQ2_MGOUKEI;
		value = 1;
	}
	else {
		grachr(1, 2, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_1[3]);		/* "前回複数台数合計 */
		req = NTNET_DATAREQ2_MGOUKEI;
		value = 2;
	}
	rev = 0;
	grachr(2, 2, 20, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[53]);	/* "　しばらくお待ち下さい　　　　" */
	Fun_Dsp(FUNMSG[82]);													/* "　　　　　　 中止 　　　　　　" */

	/* NTNETデータ送信(データ要求2) */
	NTNET_Snd_Data109(req, value);
	tmout = (ushort)(prm_get(COM_PRM, S_NTN, 30, 2, 1) * 1000 / 20);	/* 20msec単位 */
	Lagtim(OPETCBNO, 6, tmout);			/* 受信タイムアウト時間セット */
	Lagtim(OPETCBNO, 7, 25);			/* 文字点滅用タイマーセット(500msec) */
	mode = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		/* 受信完了待ち */
		if (mode == 0) {
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	/* ﾓｰﾄﾞﾁｪﾝｼﾞ */
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;
			case KEY_TEN_F3:	/* 中止 */
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_EXT;
			case TIMEOUT7:		/* 文字点滅 */
				rev ^= 1;
				grachr(2, 2, 20, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[53]);	/* "　しばらくお待ち下さい　　　　" */
				Lagtim(OPETCBNO, 7, 25);												/* 文字点滅用タイマーリスタート(500msec) */
				break;
			case IBK_NTNET_DAT_REC:		/* NTNETデータ受信 */
				switch (NTNET_isTotalEndReceived(&result)) {
				/* データ要求2結果NG 受信 */
				case -1:
					Lagcan(OPETCBNO, 6);
					Lagcan(OPETCBNO, 7);
					BUZPIPI();
					if( 3 == result ){														// 失敗ｺｰﾄﾞ3（複数合計/小計ﾃﾞｰﾀなし）
						grachr(2, 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR3[65]);	// "　集計データがありません　　　"
					}
					else{																	// 他の失敗（ほとんどの場合 ｴﾗｰｺｰﾄﾞ3となる）
						grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[66]);	// "　失敗応答受信（コードＸＸ）　"
						opedsp(2, 22, result, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 結果(番号)表示
					}
					Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
					mode = 2;
					break;
				/* 集計終了通知 受信 */
				case 1:
					Lagcan(OPETCBNO, 6);
					Lagcan(OPETCBNO, 7);
					/* プリント実行 */
					if (kind == 0) {
						type = PREQ_F_TSYOUKEI;
						log = OPLOG_F_SHOKEI;
					}
					else if (kind == 1) {
						type = PREQ_F_TGOUKEI;
						log = OPLOG_F_GOKEI;
					}
					else {
						type = PREQ_F_TGOUKEI_Z;
						log = OPLOG_F_ZENGOKEI;
					}
					FrmSyuukei.prn_kind = R_PRI;
					sky.fsyuk.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// 機械№
					sky.fsyuk.Kakari_no = OPECTL.Kakari_Num;							// 係員No.
					FrmSyuukei.prn_data = (void*)&sky.fsyuk;
					memcpy( &FrmSyuukei.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// 印字時刻	：現在時刻
// MH810105(S) MH364301 複数台集計でみなし決済情報を印字してしまう
					FrmSyuukei.print_flag = 0;
// MH810105(E) MH364301 複数台集計でみなし決済情報を印字してしまう
					queset(PRNTCBNO, type, sizeof(T_FrmSyuukei), &FrmSyuukei);
					wopelg(log, 0, 0);		/* 操作履歴登録 */

					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[54]);		/* "　プリントを開始します　　　　" */
					inji_end = 0;
					mode = 1;
					break;
				default:
					break;
				}
				break;
			case TIMEOUT6:		/* 受信タイムアウト */
				Lagcan(OPETCBNO, 7);
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[55]);		/* "　 応答がありません 　　　　　" */
				Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
				mode = 2;
				break;
			default:
				break;
			}
		}
		/* プリント中 */
		else if (mode == 1) {
			if ((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
				inji_end = 1;						// 印字終了
				msg &= (~INNJI_ENDMASK);
			}
			switch (msg){
			case KEY_TEN_F3:		/* 中止 */
				BUZPI();
				if (inji_end == 0) {
					FrmPrnStop.prn_kind = R_PRI;			// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					queset(PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop);	// 印字中止要求
				}
				if (mnt_sw == 1) {
					return MOD_CHG;
				}
				return MOD_EXT;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				if (inji_end) {
					BUZPI();
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:		/* ﾓｰﾄﾞﾁｪﾝｼﾞ */
				if (inji_end) {
					BUZPI();
					return MOD_CHG;
				}
				mnt_sw = 1;			// メンテOFF保持
				break;
			case PREQ_F_TSYOUKEI:  	// 複数小計
			case PREQ_F_TGOUKEI: 	// 複数合計
			case PREQ_F_TGOUKEI_Z:	// 前回複数合計
				if (inji_end) {
					if (mnt_sw == 1) {
						return MOD_CHG;
					}
					switch (OPECTL.Pri_Result) {	// 印字結果？
					case PRI_NML_END:	/* 正常終了 */
					case PRI_CSL_END:	/* ｷｬﾝｾﾙ要求による終了 */
						return MOD_EXT;
					case PRI_ERR_END:	/* 異常終了 */
						grachr(2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[7]);		// "　プリンター異常です。　　　　"
						Fun_Dsp(FUNMSG[8]);														// "　　　　　　　　　　　　 終了 "
						mode = 2;
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
		/* 応答なし or プリンタ異常 */
		else {	/* if (mode == 2) */
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	/* ﾓｰﾄﾞﾁｪﾝｼﾞ */
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:	/* 終了 */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
	}

}


/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：定期券精算中止データ                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassStop( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassStop(void)
{
	ushort	msg;
	char	fk_page;	// ファンクションキー表示ページ
	char	repaint;	// 画面再描画フラグ
	short	show_index;	// 表示中のインデックス
	short ret;
	struct TKI_CYUSI work_data;
	short data_count;
	f_NTNET_RCV_TEIKITHUSI = 0;
	
	dispclr();									// 画面クリア

	fk_page = 0;
	repaint = 1;
	show_index = 0;
	data_count = 0;

	
	// 画面タイトルを表示
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "＜定期券中止データ＞　　　　　"

	Fun_Dsp(FUNMSG2[20]);													// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "

	for ( ; ; ) {
	
		if (repaint) {
			pstop_show_data(show_index, &tki_cyusi, &work_data, fk_page);
			data_count = tki_cyusi.count;
			repaint = 0;
		}
		msg = StoF(GetMessage(), 1);
		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ｷｰが押されたら
			if ( f_NTNET_RCV_TEIKITHUSI == 1 ) {
				f_NTNET_RCV_TEIKITHUSI = 0;
				BUZPIPI();
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//				if( 1 == pstop_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
//														// 変更されていたら「ワーニング画面」に遷移する
//					return MOD_CHG;
//				}
//				else
//					return MOD_EXT;
				if( 1 == pstop_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
														// 変更されていたら「ワーニング画面」に遷移する
					return MOD_CHG;
				}
				else if(3 == pstop_calcel_dsp()){
					return MOD_CUT;
				} else {
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			}
		}

		switch (msg) {
		case KEY_TEN_F1:			/* F1:▲ */
			if (data_count != 0) {	// データ数が０以上の時有効

				BUZPI();
				if (fk_page == 0) {			// ファンクションキー１ページ目
					show_index--;
					if (show_index < 0) {
						show_index = tki_cyusi.count -1 ;
					}
				} else {					// ファンクションキー２ページ目
					// 削除
					ret = pstop_del_dsp(show_index, 0, &work_data);
					if (ret == 1) {
						return( MOD_CHG );
					}else if(ret == 2){
						return( MOD_EXT);
					} else if (ret == -1) {
						
						if (show_index >= tki_cyusi.count) {
							show_index--;
						}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
					}else if(ret == 3){
						return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
					}
					if (fk_page) {
						Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
					} else {
						Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
					}
				}
				repaint = 1;
			}
			break;
		case KEY_TEN_F2:			/* F2:▼ */
			if (data_count != 0) {	// データ数が０以上の時有効
				if (fk_page == 0) {			// ファンクションキー１ページ目
					BUZPI();
					show_index++;
					if (show_index > tki_cyusi.count - 1) {
						show_index = 0;
					}
				} else {					// ファンクションキー２ページ目
					// 全削除
					BUZPI();
					ret = pstop_del_dsp(0, 1, &work_data);
					if (ret == 0) {
						if (fk_page) {
							Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
						} else {
							Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
						}
					} else if (ret == 1) {
						return( MOD_CHG );
					}
					else if (ret == 2) {
						return( MOD_EXT );
					}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
					else if (ret == 3) {
						return( MOD_CUT );
					}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				}
				repaint = 1;
			}
			break;
		case KEY_TEN_F3:			/* F3:プリント */
			break;
		case KEY_TEN_F4:			/* F4:次へ */
			if (data_count != 0) {	// データ数が０以上の時有効
				BUZPI();
				fk_page ^= 1;
				if (fk_page) {
					Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
				} else {
					Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
				}
			}
			break;
		case KEY_TEN_F5:			/* F5:終了 */
			BUZPI();
			return( MOD_EXT );
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:	// モードチェンジ
			BUZPI();
			return( MOD_CHG );
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ 表示                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_show_data                                         |*/
/*| PARAMETER    :                                         |*/
/*| PARAMETER    :                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_show_data(short index, t_TKI_CYUSI *cyusi, struct TKI_CYUSI *work_data, char fk_page)
{

	if (cyusi->count != 0) {		// データ有り
		pstop_num_dsp((short)(index + 1), cyusi->count);

		TKI_Get(work_data, index);

		pstop_data_dsp(work_data, CPrmSS[S_TIK][5]);

		if (fk_page) {
			Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
		} else {
			Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
		}

	} else {					// データ無し
		pstop_num_dsp(0, 0);
		displclr(1);
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);			// "　　　データはありません　　　"
		displclr(3);
		displclr(4);
		displclr(5);
		displclr(6);
		Fun_Dsp(FUNMSG[8]);						// "　　　　　　　　　　　　 終了 "
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ 件数表示                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_num_dsp( short numerator, short denominator )     |*/
/*| PARAMETER    : numerator   分子                                        |*/
/*| PARAMETER    : denominator 分母                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_num_dsp(short numerator, short denominator)
{
	// 枠を表示
	grachr(0, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[31]);		// [  /  ]

	// 分子を表示
	opedsp3(0, 24, (unsigned short)numerator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

	// 分母を表示
	opedsp3(0, 27, (unsigned short)denominator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

}

/*[]----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ データ表示                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_data_dsp                                          |*/
/*| PARAMETER    : index 表示対象                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_data_dsp(struct TKI_CYUSI *data, long type)
{
	// 共通項目を表示
	grachr(1, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[29]);			// ＩＤ
	grachr(1, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			// ：
	opedpl(1, 6, (unsigned long)data->no, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);

	grachr(1, 23, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[19]);		// [　　]
	if( data->pk == CPrmSS[S_SYS][1] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);		// 基本
	}
	else if( data->pk == CPrmSS[S_SYS][2] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[26]);	// 拡１
	}
	else if( data->pk == CPrmSS[S_SYS][3] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[27]);	// 拡２
	}
	else if( data->pk == CPrmSS[S_SYS][4] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[28]);	// 拡３
	}

	grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);			// 入庫
	grachr(2, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			// ：
	date_dsp(2, 6, (ushort)data->year, (ushort)data->mon, (ushort)data->day, 0);	// 日付
	time_dsp(2, 16, (unsigned short)data->hour, (unsigned short)data->min, 0);		// 時間

	grachr(3, 0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[32]);		// 定期種別
	grachr(3, 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		// ：
	opedsp(3, 10, (unsigned short)data->tksy, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

	grachr(3, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[32]);	// 種別
	grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		// ：
	grachr(3, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[data->syubetu]);

}

/*[]----------------------------------------------------------------------[]*/
/*|  時間表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : time_dsp                                                |*/
/*| PARAMETER    : ushort low  : 行                                        |*/
/*| PARAMETER    : ushort col  : 列                                        |*/
/*| PARAMETER    : ushort hour : 時                                        |*/
/*| PARAMETER    : ushort min  : 分                                        |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	time_dsp(ushort low, ushort col, ushort hour, ushort min, ushort rev)
{
	opedsp3(low, col, hour, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* 時 */
	grachr(low, (ushort)(col+2), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);	/* ":" */
	opedsp3(low, (ushort)(col+3), min , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 */
}

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
///*[]----------------------------------------------------------------------[]*/
///*|  時間表示  全角                                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : time_dsp2                                               |*/
///*| PARAMETER    : ushort low  : 行                                        |*/
///*| PARAMETER    : ushort col  : 列                                        |*/
///*| PARAMETER    : ushort hour : 時                                        |*/
///*| PARAMETER    : ushort min  : 分                                        |*/
///*|              : ushort rev  : 0:正転 1:反転                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	time_dsp2(ushort low, ushort col, ushort hour, ushort min, ushort rev)
//{
//	opedsp(low, col, hour, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* 時 */
//	grachr(low, (ushort)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		/* ":" */
//	opedsp(low, (ushort)(col+6), min , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* 分 */
//}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

/*[]----------------------------------------------------------------------[]*/
/*|  日付表示                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : date_dsp                                                |*/
/*| PARAMETER    : ushort low  : 行                                        |*/
/*| PARAMETER    : ushort col  : 列                                        |*/
/*| PARAMETER    : ushort hour : 月                                        |*/
/*| PARAMETER    : ushort min  : 日                                        |*/
/*|              : ushort rev  : 0:正転 1:反転                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	date_dsp(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev)
{
	opedsp3(low, col, year, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* 年 */
	grachr(low, (ushort)(col+2), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);	/* "/" */
	opedsp3(low, (ushort)(col+3), month, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 月 */
	grachr(low, (ushort)(col+5), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);	/* "/" */
	opedsp3(low, (ushort)(col+6), day , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* 日 */
}

/*[]----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ 削除画面表示                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_del_dsp                                           |*/
/*| PARAMETER    : index 削除対象                                          |*/
/*| RETURN VALUE : short 0:normal                                          |*/
/*| RETURN VALUE : short 1:mode change                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short pstop_del_dsp(short index, short mode, struct TKI_CYUSI* work_data)
{

	ushort	msg;
	short del_index;		// 削除対象の配列上のインデックス

	displclr(1);
	if (mode) {
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "＜定期券中止データ＞　　　　　"
		grachr(2, 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR3[61]);		// "　　 全データを削除します 　　"
	} else {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);		// "　　　データを削除します　　　"
	}
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);			// "　　　 よろしいですか？ 　　　"
	displclr(4);
	displclr(5);
	displclr(6);
	Fun_Dsp(FUNMSG[19]);						// "　　　　　　 はい いいえ　　　"

	// 削除対象のインデックスを求める。
	del_index = tki_cyusi.wtp - tki_cyusi.count + index;
	if (del_index < 0) {
		del_index = TKI_CYUSI_MAX + del_index;
	}

	for ( ; ; ) {
	
		msg = StoF(GetMessage(), 1);
		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ｷｰが押されたら
			if ( f_NTNET_RCV_TEIKITHUSI == 1 ) {
				f_NTNET_RCV_TEIKITHUSI = 0;
				BUZPIPI();
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//				if( 1 == pstop_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
//														// 変更されていたら「ワーニング画面」に遷移する
//					return 1;
//				}
//				return 2;
				if( 1 == pstop_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
														// 変更されていたら「ワーニング画面」に遷移する
					return MOD_CHG;
				}
				else if(3 == pstop_calcel_dsp()){
					return MOD_CUT;
				} else {
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			}
		}

		switch (msg) {
		case KEY_TEN_F3:			/* F3:はい */
			BUZPI();
			if (mode) {
				// 全削除
				TKI_DeleteAll();
				wopelg(OPLOG_TEIKICHUSI_ALLDEL, 0, 0);
				NTNET_Snd_Data219(2, NULL);
			} else {
				// １件削除
				if (memcmp(&tki_cyusi.dt[del_index], work_data, sizeof(struct TKI_CYUSI)) == 0) {
					// 削除データ一致
					TKI_Delete(del_index);
					wopelg(OPLOG_TEIKICHUSI_DEL, 0, 0);
					NTNET_Snd_Data219(1, work_data);
				}
			}
			displclr(2);
			displclr(3);
			return( -1 );
		case KEY_TEN_F4:			/* F4:いいえ */
			BUZPI();
			displclr(2);
			displclr(3);
			return( 0 );
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( 3 );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:	// モードチェンジ
			BUZPI();
			return( 1 );
		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ：処理中止画面                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_calcel_dsp( void )                                 |*/
/*| PARAMETER    : void                                                     |*/
/*| RETURN VALUE : short 0:normal                                           |*/
/*| RETURN VALUE : short 1:mode change                                      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
static short pstop_calcel_dsp(void)
{
	ushort	msg;

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "＜定期券中止データ＞　　　　　"
	displclr(1);
	displclr(2);
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[149]);		// "　　データが更新されました　　"

	displclr(4);
	displclr(5);
	displclr(6);
	Fun_Dsp(FUNMSG[8]);						// "　　　　　　　　　　　　 終了 "

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);

		switch (msg) {
		case KEY_TEN_F5:	/* F5:終了 */
			BUZPI();
			displclr(2);
			displclr(3);
			return( 0 );
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( 3 );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:	// モードチェンジ
			BUZPI();
			return( 1 );
		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ  データ取得                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_Get                                                  |*/
/*| PARAMETER    : data	= 取得データ格納領域						<OUT>   |*/
/*|				   ofs	= 取得したいデータのテーブル先頭からのオフセット    |*/
/*| RETURN VALUE : データありの場合	→	コピー元データアドレス              |*/
/*|				   データなしの場合	→	NULL                                |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
const struct TKI_CYUSI *TKI_Get(struct TKI_CYUSI *data, short ofs)
{
	short data_index;

	if (tki_cyusi.count > 0) {
		if (tki_cyusi.count > ofs) {
			data_index = tki_cyusi.wtp - tki_cyusi.count + ofs;

			if (data_index < 0) {
				data_index = TKI_CYUSI_MAX + data_index;
			}
			if (data != NULL) {
				memcpy(data, &tki_cyusi.dt[data_index], sizeof(struct TKI_CYUSI));
			}
			return &tki_cyusi.dt[data_index];
		}
	}

	return NULL;
}

/*[]-----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ  データ削除                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_Delete                                               |*/
/*| PARAMETER    : ofs	= 削除したいデータのテーブル先頭からのオフセット    |*/
/*| RETURN VALUE : none                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| REMARK       : 削除後の先頭データが配列の[0]にくるようデータを構成し直す|*/
/*|				   停電保証に対応                                           |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
void	TKI_Delete(short ofs)
{

	short old_index;		// 削除前の最古データのインデックス
	short i;

	memset(&teiki_cyusi, 0, sizeof(t_TKI_CYUSI));

	teiki_cyusi.count = tki_cyusi.count - 1;
	teiki_cyusi.wtp = teiki_cyusi.count;
	
	// 最古データインデックスを取得
	old_index = tki_cyusi.wtp - tki_cyusi.count;
	if (old_index < 0) {
		old_index = TKI_CYUSI_MAX + old_index;
	}

	for (i = 0; i < teiki_cyusi.count; ) {
		if (old_index != ofs) {	// 削除対象と一致したときはコピーしない
			memcpy(&teiki_cyusi.dt[i], &tki_cyusi.dt[old_index], sizeof(struct TKI_CYUSI));
			i++;
		}
		old_index++;
		if (old_index >= TKI_CYUSI_MAX) {	// 配列の最大サイズを超えていた場合先頭に戻す。
			old_index = 0;
		}
	}
	
	nmisave(&tki_cyusi, &teiki_cyusi, sizeof(t_TKI_CYUSI));

}

/*[]-----------------------------------------------------------------------[]*/
/*|  定期券精算中止データ  データ全削除                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_DeleteAll( void )                                    |*/
/*| PARAMETER    : void                                                     |*/
/*| RETURN VALUE : none                                                     |*/
//*[]----------------------------------------------------------------------[]*/
/*| REMARK       : 停電保証に対応                                           |*/
//*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
void	TKI_DeleteAll(void)
{
	memset(&teiki_cyusi, 0, sizeof(t_TKI_CYUSI));
	nmisave(&tki_cyusi, &teiki_cyusi, sizeof(t_TKI_CYUSI));

}

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：定期券出庫時刻データ                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassExTime( void )                                |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//unsigned short	UsMnt_PassExTime(void)
//{
//	ushort				msg;
//	char				fk_page;	// ファンクションキー表示ページ
//	char				repaint;	// 画面再描画フラグ
//	short				show_index;	// 表示中のインデックス
//	short				ret;
//	ushort 				data_count;
//	PASS_EXTBL			work_data;
//
//	f_NTNET_RCV_TEIKIEXTIM = 0;
//
//	dispclr();									// 画面クリア
//
//	fk_page = 0;
//	repaint = 1;
//	show_index = 0;
//	data_count = 0;
//
//	// 画面タイトルを表示
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);			// "＜定期出庫データ＞　　　　　　"
//	Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
//
//	for ( ; ; ) {
//		if (repaint) {
//			pextim_show_data(show_index, &work_data, fk_page);
//			data_count = pas_extimtbl.Count;
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ｷｰが押されたら
//			if ( f_NTNET_RCV_TEIKIEXTIM == 1 ) {
//				f_NTNET_RCV_TEIKIEXTIM = 0;
//				BUZPIPI();
//				if( 1 == pextim_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
//														// 変更されていたら「ワーニング画面」に遷移する
//					return MOD_CHG;
//				}
//				else
//					return MOD_EXT;
//			}
//		}
//
//		switch (msg) {
//		case KEY_TEN_F1:			/* F1:▲ */
//			if (data_count != 0) {	// データ数が０以上の時有効
//				BUZPI();
//				if (fk_page == 0) {			// ファンクションキー１ページ目
//					show_index--;
//					if (show_index < 0) {
//						show_index = pas_extimtbl.Count -1 ;
//					}
//				} else {					// ファンクションキー２ページ目
//					// 削除
//					ret = pextim_del_dsp(show_index, 0, &work_data);
//					if (ret == 1) {
//						return( MOD_CHG );
//					}else if(ret == 2){
//						return( MOD_EXT);
//					} else if (ret == -1) {
//						
//						if (show_index >= pas_extimtbl.Count) {
//							show_index--;
//						}
//					}
//					if (fk_page) {
//						Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
//					} else {
//						Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
//					}
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F2:			/* F2:▼ */
//			if (data_count != 0) {	// データ数が０以上の時有効
//				if (fk_page == 0) {			// ファンクションキー１ページ目
//					BUZPI();
//					show_index++;
//					if (show_index > pas_extimtbl.Count - 1) {
//						show_index = 0;
//					}
//				} else {					// ファンクションキー２ページ目
//					// 全削除
//					BUZPI();
//					ret = pextim_del_dsp(0, 1, &work_data);
//					if (ret == 0) {
//						if (fk_page) {
//							Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
//						} else {
//							Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
//						}
//					} else if (ret == 1) {
//						return( MOD_CHG );
//					}
//					else if (ret == 2) {
//						return( MOD_EXT );
//					}
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F3:			/* F3:プリント */
//			if (data_count != 0) {	// データ数が０以上の時有効
//				if (fk_page == 0) {			// ファンクションキー１ページ目
//				}
//			}
//			break;
//		case KEY_TEN_F4:			/* F4:次へ */
//			if (data_count != 0) {	// データ数が０以上の時有効
//				BUZPI();
//				fk_page ^= 1;
//				if (fk_page) {
//					Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
//				} else {
//					Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
//				}
//			}
//			break;
//		case KEY_TEN_F5:			/* F5:終了 */
//			BUZPI();
//			return( MOD_EXT );
//		case KEY_MODECHG:	// モードチェンジ
//			BUZPI();
//			return( MOD_CHG );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期券出庫時刻データ 表示                                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_show_data                                        |*/
///*| PARAMETER    :                                         |*/
///*|              :                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_show_data(short index, PASS_EXTBL *work_data, char fk_page)
//{
//
//	if (pas_extimtbl.Count != 0) {		// データ有り
//		pextim_num_dsp((ushort)(index + 1), pas_extimtbl.Count);
//		/* 表示データ記憶 */
//		memcpy(work_data, &pas_extimtbl.PassExTbl[index], sizeof(PASS_EXTBL));
//		pextim_data_dsp(work_data);
//
//		if (fk_page) {
//			Fun_Dsp(FUNMSG2[21]);						// " 削除 全削除　　　 次へ  終了 "
//		} else {
//			Fun_Dsp(FUNMSG2[20]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 次へ  終了 "
//		}
//	} else {					// データ無し
//		pextim_num_dsp(0, 0);
//		displclr(1);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);			// "　　　データはありません　　　"
//		displclr(3);
//		displclr(4);
//		displclr(5);
//		displclr(6);
//		Fun_Dsp(FUNMSG[8]);						// "　　　　　　　　　　　　 終了 "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期券出庫時刻データ 件数表示                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_num_dsp( ushort numerator, ushort denominator )  |*/
///*| PARAMETER    : numerator   分子                                        |*/
///*| PARAMETER    : denominator 分母                                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_num_dsp(ushort numerator, ushort denominator)
//{
//	// 枠を表示
//	grachr(0, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[11]);		// [    /    ]
//	// 分子を表示
//	opedsp3(0, 19, numerator, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	// 分母を表示
//	opedsp3(0, 24, denominator, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期券出庫時刻データ データ表示                                       |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_data_dsp                                         |*/
///*| PARAMETER    : index 表示対象                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_data_dsp(PASS_EXTBL *data)
//{
//	// 共通項目を表示
//	grachr(1, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[29]);								// ＩＤ
//	grachr(1, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// ：
//	opedpl(1, 6, (unsigned long)data->PassId, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);			// 定期ID
//
//	grachr(1, 23, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[19]);							// [　　]
//	if (data->ParkNo == CPrmSS[S_SYS][1]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);							// 基本
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][2]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[26]);						// 拡１
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][3]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[27]);						// 拡２
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][4]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[28]);						// 拡３
//	}
//
//	grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);								// 出庫
//	grachr(2, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// ：
//
//	opedsp3(2, 6, (ushort)(data->PassTime.Year%100), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 年
//	grachr(2, 8, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							// "/"
//	opedsp3(2, 9, (ushort)(data->PassTime.Mon), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 月
//	grachr(2, 11, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							// "/"
//	opedsp3(2, 12, (ushort)(data->PassTime.Day), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 日
//	opedsp3(2, 16, (ushort)(data->PassTime.Hour), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 時
//	grachr(2, 18, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);							// ":"
//	opedsp3(2, 19, (ushort)(data->PassTime.Min), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 分
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  定期券出庫時刻データ 削除画面表示                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_del_dsp                                          |*/
///*| PARAMETER    : index 削除対象                                          |*/
///*| RETURN VALUE : short 0:normal                                          |*/
///*| RETURN VALUE : short 1:mode change                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static short pextim_del_dsp(short index, short mode, PASS_EXTBL *work_data)
//{
//	ushort	msg;
//
//	displclr(1);
//	if (mode) {
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);		// "＜定期出庫データ＞　　　　　"
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[61]);		// "　　 全データを削除します 　　"
//	} else {
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);		// "　　　データを削除します　　　"
//	}
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);			// "　　　 よろしいですか？ 　　　"
//	Fun_Dsp(FUNMSG[19]);									// "　　　　　　 はい いいえ　　　"
//
//	for ( ; ; ) {
//	
//		msg = StoF(GetMessage(), 1);
//		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ｷｰが押されたら
//			if ( f_NTNET_RCV_TEIKIEXTIM == 1 ) {
//				f_NTNET_RCV_TEIKIEXTIM = 0;
//				BUZPIPI();
//				if( 1 == pextim_calcel_dsp() ){			// ﾃﾞｰﾀが通信で更新されているかをﾁｪｯｸ
//														// 変更されていたら「ワーニング画面」に遷移する
//					return 1;
//				}
//				return 2;
//			}
//		}
//		switch (msg) {
//		case KEY_TEN_F3:			/* F3:はい */
//			BUZPI();
//			if (mode) {
//				// 全削除
//				memset(&pas_extimtbl, 0, sizeof(PASS_EXTIMTBL));
//				wopelg(OPLOG_PEXTIM_ALLDEL, 0, 0);		// 操作履歴登録
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_OUTTIME
//								, 0
//								, 0
//								, 0
//								, 0
//								, 0
//								, NULL );
//			}
//			else {
//				// １件削除
//				if (memcmp(&pas_extimtbl.PassExTbl[index], work_data, sizeof(PASS_EXTBL)) == 0) {
//					// 削除データ一致
//					PassExitTimeTblDelete((ulong)work_data->ParkNo, work_data->PassId);
//					wopelg(OPLOG_PEXTIM_DEL, 0, 0);		// 操作履歴登録
//					NTNET_Snd_Data116( NTNET_PASSUPDATE_OUTTIME
//									, (ulong)work_data->PassId
//									, work_data->ParkNo
//									, 0
//									, 0
//									, 0
//									, NULL );
//				}
//			}
//			displclr(2);
//			displclr(3);
//			return( -1 );
//		case KEY_TEN_F4:	/* F4:いいえ */
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:	// モードチェンジ
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  定期券出庫時刻データ：処理中止画面                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_calcel_dsp( void )                                 |*/
///*| PARAMETER    : void                                                     |*/
///*| RETURN VALUE : short 0:normal                                           |*/
///*| RETURN VALUE : short 1:mode change                                      |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
//static short pextim_calcel_dsp(void)
//{
//	ushort	msg;
//
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);		// "＜定期出庫データ＞　　　　　　"
//	displclr(1);
//	displclr(2);
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[149]);		// "　　データが更新されました　　"
//	Fun_Dsp(FUNMSG[8]);						// "　　　　　　　　　　　　 終了 "
//
//	for ( ; ; ) {
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F5:	/* F5:終了 */
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:	// モードチェンジ
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ラベルプリンタ設定                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelSet( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_LabelSet(void)
{
	unsigned short	usUextEvent;
	char	wk[2];

	/* 定期券更新機能なしの場合は画面を抜ける */

	if( ( prm_get(COM_PRM,S_PAY,25,2,1) == 0 )||( !CPrmSS[S_KOU][1] ) ){	// 定期更新なし&ﾗﾍﾞﾙﾌﾟﾘﾝﾀなし
		BUZPIPI();
		return MOD_EXT;
	}

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[64]);		// "＜ラベルプリンタ設定＞　　　　"

		usUextEvent = Menu_Slt( LABELSETMENU, USM_LBSET_TBL, LBSET_MENU_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usUextEvent) {

		case MNT_LBPAPER:		// 用紙残量設定
			usUextEvent = UsMnt_LabelPaperSet();
			break;

		case MNT_LBPAY:			// ラベル発行不可時の設定
			usUextEvent = UsMnt_LabelPaySet();
			break;

		case MNT_LBTESTP:		// ラベルテスト印字
			usUextEvent = UsMnt_LabelTest();
			break;

		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return MOD_EXT;
		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
			SetChange = 1;			// FLASHｾｰﾌﾞ指示
		}
		if (usUextEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = -1;
			OPECTL.PasswordLevel = -1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (usUextEvent == MOD_CUT ) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = -1;
			OPECTL.PasswordLevel = -1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ラベルプリンタ用紙残量設定                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelPaperSet( void )                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelPaperSet(void)
{
	ushort	msg;
	long	*pPRM[2];
	long	count;
	ushort	nearend;
	char	pos;		// 0:現在枚数 1:用紙切れ枚数
	char	changing;

	dispclr();

	pPRM[0] = &PPrmSS[S_P01][10];								// ﾗﾍﾞﾙ現在枚数設定
	pPRM[1] = &CPrmSS[S_KOU][25];								// ﾃﾞﾌｫﾙﾄ枚数、ﾆｱｴﾝﾄﾞ枚数設定
	nearend = (ushort)prm_get( COM_PRM,S_KOU,25,2,1 );			// ﾆｱｴﾝﾄﾞ枚数

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[0]);							// "＜用紙残量設定＞　　　　　　　"

	grachr(2, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);								// "現在の状態
	grachr(2, 15, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// "："
	if( LprnErrChk( 0 ) ){										// ｵﾌﾗｲﾝ以外のｴﾗｰあり?
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[26]);						// 
	}else if( ERR_CHK[mod_lprn][ERR_LPRN_OFFLINE] ){			// ｵﾌﾗｲﾝ状態?
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[25]);						// 
	}else{														// 正常
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[24]);						// 
	}
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[2]);							// "　現在の枚数　 ： 　　　　枚　"
	opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					// "　　　　　　　　　ＸＸＸＸ　　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[3]);							// "　用紙切れ枚数 ： 　　　　枚　"
	opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							// "　　　　　　　　　　　ＸＸ　　"

	if( OPECTL.Mnt_lev >= 2 )
		Fun_Dsp( FUNMSG2[0] );									// "　　　　　　 変更　　　　終了 "
	else
		Fun_Dsp( FUNMSG[8] );									// "　　　　　　　　　　　　 終了 "

	changing = 0;
	for ( ; ; ) {
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
		
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* 変更(F3) */
				if( OPECTL.Mnt_lev < 2 ) break;
				BUZPI();
				changing = 1;
				opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "　　　　　　　　　ＸＸＸＸ　　"現在枚数反転
				Fun_Dsp( FUNMSG2[22] );															// "　▲　　▼　 自動　書込　終了 "
				count = -1;
				pos = 0;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				if( pos == 0 )
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// "　　　　　　　　　ＸＸＸＸ　　"現在枚数正転
				else
					opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// "　　　　　　　　　　　ＸＸ　　"用紙切れ枚数正転

				Fun_Dsp(FUNMSG2[0]);							// "　　　　　　 変更　　　　終了 "
				changing = 0;
				break;
			case KEY_TEN_F4:	/* 書込(F4) */
				if( count != -1 ){
					if( pos == 0 ){
						*pPRM[0] = count;
					}else{
						*pPRM[1] /= 100;
						*pPRM[1] *= 100;						// 下2桁(用紙切れ枚数)削除
						*pPRM[1] += count;						// 下2桁(用紙切れ枚数)更新
						nearend = (ushort)prm_get( COM_PRM,S_KOU,25,2,1 );	// ﾆｱｴﾝﾄﾞ枚数
					}
					OpelogNo = OPLOG_LPRPAPERSET;				// 操作履歴登録
					SetChange = 1;								// FLASHｾｰﾌﾞ指示
				}
				// no break
			case KEY_TEN_F1:	/* ▲(F1) */
			case KEY_TEN_F2:	/* ▼(F2) */
			case KEY_TEN_CL:	/* 取消(F4) */
				BUZPI();
				if( msg == KEY_TEN_F1 || msg == KEY_TEN_F2 || msg == KEY_TEN_F4 ){
					if( pos == 0 ){
						pos = 1;
						Fun_Dsp( FUNMSG[20] );					// "　▲　　▼　　　　 書込  終了 "
					}else{
						pos = 0;
						Fun_Dsp( FUNMSG2[22] );					// "　▲　　▼　 自動　書込　終了 "
					}
				}
				if( pos == 0 ){
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	// "　　　　　　　　　ＸＸＸＸ　　"現在枚数反転
					opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// "　　　　　　　　　　　ＸＸ　　"用紙切れ枚数正転
				}else{
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// "　　　　　　　　　ＸＸＸＸ　　"現在枚数正転
					opedsp(5, 22, nearend, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			// "　　　　　　　　　　　ＸＸ　　"用紙切れ枚数反転
				}
				count = -1;
				break;
			case KEY_TEN:		/* 数字(テンキー) */
				BUZPI();
				if (count == -1)
					count = 0;
				count = (count*10 + msg-KEY_TEN0) % 10000;
				if( pos == 0 ){
					opedsp(4, 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "　　　　　　　　　ＸＸＸＸ　　"現在枚数反転
				}else{
					opedsp(5, 22, (ushort)count, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "　　　　　　　　　　　ＸＸ　　"用紙切れ枚数反転
				}
				break;
			case KEY_TEN_F3:	/* 自動(F3) */
				if( pos == 0 ){
					BUZPI();
					count = prm_get( COM_PRM,S_KOU,25,4,3 );	// ﾃﾞﾌｫﾙﾄ用紙枚数
					opedsp(4, 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "　　　　　　　　　ＸＸＸＸ　　"現在枚数反転
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ラベル発行不可時の設定                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelPaySet( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelPaySet(void)
{
	ushort	msg;
	char	pos;

	pos = (char)(CPrmSS[S_KOU][26] % 10L);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[1] );							// "＜ラベル発行不可時の設定＞　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[12] );								// "　現在の設定　　　　　　　　　"
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );								// "　　　　　　：　　　　　　　　"
	grachr( 2, 14, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2+pos] );							// "更新精算する　"or"更新精算しない"

	grachr( 4,  8, 12, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2] );					// "更新精算する　"
	grachr( 5,  8, 14, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[3] );					// "更新精算しない"
	Fun_Dsp( FUNMSG[20] );											// "　▲　　▼　　　　 書込  終了 "

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
				pos ^= 1;
				grachr( 4,  8, 12, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2] );		// "更新精算する　"
				grachr( 5,  8, 14, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[3] );		// "更新精算しない"
				break;
			case KEY_TEN_F4:					/* F4:書込 */
				BUZPI();
				CPrmSS[S_KOU][26] /= 10;
				CPrmSS[S_KOU][26] *= 10;							// 下1桁(印字精算時の精算)削除
				CPrmSS[S_KOU][26] += (long)pos;						// 下1桁(印字精算時の精算)更新

				if( pos == 0 ){
					OpelogNo = OPLOG_LPRKOSINARI;					// ラベル発行不可時の更新あり操作登録
				}else{
					OpelogNo = OPLOG_LPRKOSINNASI;					// ラベル発行不可時の更新なし操作登録
				}
				grachr( 2, 14, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2+pos] );				// "更新精算する　"or"更新精算しない"
				SetChange = 1;										// FLASHｾｰﾌﾞ指示
				break;
			default:
				break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ラベルテスト印字                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelTest( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelTest(void)
{
	ushort	msg;

	if( LprnErrChk( 1 ) ){
		BUZPIPI();													// ﾗﾍﾞﾙ関連ｴﾗｰ時はﾃｽﾄ印字させない
		return( MOD_EXT );
	}

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[4] );							// "＜テスト印字＞　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );								// "　印字中です　　　　　　　　　"
	Fun_Dsp( FUNMSG[8] );											// "　　　　　　　　　　　　 終了 "

	memset( &LabelPayData, 0, sizeof( Receipt_data ) );				// ﾗﾍﾞﾙ印字用ﾜｰｸﾊﾞｯﾌｧｸﾘｱ

	LabelPayData.TOutTime.Year = CLK_REC.year;						// 発行年
	LabelPayData.TOutTime.Mon  = CLK_REC.mont;						// 発行月
	LabelPayData.TOutTime.Day  = CLK_REC.date;						// 発行日

	OpLpr_snd( &LabelPayData, 1 );									// ﾗﾍﾞﾙ出力(ﾃｽﾄ印刷)

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
				// no break
			case IBK_LPR_B1_REC_OK:				/* ﾌﾟﾘﾝﾄOK */
			case IBK_LPR_B1_REC_NG:				/* ﾌﾟﾘﾝﾄNG */
				return( MOD_EXT );									// ﾌﾟﾘﾝﾀからのﾚｽﾎﾟﾝｽにて画面を戻す
			default:
				break;
		}
	}
}
#endif	// UPDATE_A_PASS

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能 - クレジット処理ﾒﾆｭｰ                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreditMnu( void )                                 |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//unsigned short	UsMnt_CreditMnu(void)
//{
//	unsigned short	usUcreEvent;
//	char	wk[2];
//
//	DP_CP[0] = DP_CP[1] = 0;
//
//	for( ; ; ) {
//
//		dispclr();								// 画面クリア
//		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[74] );		// "＜クレジット処理＞　　　　　　　　　"
//
//		usUcreEvent = Menu_Slt( EXTENDMENU_CRE, USM_EXTCRE_TBL, (char)USM_EXTCRE_MAX, (char)1 );
//		wk[0] = DP_CP[0];
//		wk[1] = DP_CP[1];
//
//		switch( usUcreEvent ){
//			case MNT_CREUSE:
//				usUcreEvent = UsMnt_CreUseLog();					// クレジット利用明細
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CRECONECT:
//				usUcreEvent = UsMnt_CreConect();					// クレジット接続確認
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CREUNSEND:
//				usUcreEvent = UsMnt_CreUnSend();					// クレジット未送信売上依頼データ
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CRESALENG:
//				if (OPECTL.Mnt_lev < 2){							// 係員以下は操作不可
//					BUZPIPI();
//				}else{
//					usUcreEvent = UsMnt_CreSaleNG();				// クレジット売上拒否データ
//					if (usUcreEvent == MOD_CHG){
//						return(MOD_CHG);
//					}
//				}
//				break;
//
//			case MOD_EXT:		// 終了（Ｆ５）
//				return(MOD_EXT);
//
//			case MOD_CHG:		// モードチェンジ
//				return(MOD_CHG);
//
//			default:
//				break;
//		}
//		DP_CP[0] = wk[0];
//		DP_CP[1] = wk[1];
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット接続確認                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreConect(void)                                   |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static ushort	UsMnt_CreConect( void )
//{
//	ushort			msg;
//	uchar			mode;
//	ushort			rev;				//反転表示用
//	short			ret;
//	DATA_KIND_137_02	*Recv137_02;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[75]);			/* "＜クレジット接続確認＞　　　　" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[77]);			/* "　　　接続の確認中です　　　　" */
//	rev = 0;
//	grachr(5, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "　　しばらくお待ち下さい　　　" */
//
//	ret = creOnlineTestCheck();				//接続確認
//	wopelg( OPLOG_CRE_CON_CHECK, 0, 0 );	//操作ログ：接続確認
//
//	switch( ret ){
//	case  0:		//正常受付
//		Lagtim(OPETCBNO, 7, 25);		//文字点滅用タイマーセット(500msec)
//		mode = 0;
//		break;
//	case  1:		//遠隔設定なしの為、バッファ未登録（ここには来ないはず）
//	case -1:		//ｸﾚｼﾞｯﾄ設定なし
//	default:
//		BUZPIPI();
//		displclr(3);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "　　　　　異常終了　　　　　　" */
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[80]);		/* "　　　通信タイムアウトが　　　" */
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[81]);		/* "　　　　発生しました。　　　　" */
//		Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//		mode = 2;
//		break;
//	case -2:		//通信処理中
//		BUZPIPI();
//		displclr(3);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "　　　　　異常終了　　　　　　" */
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "　　現在クレジット処理中。　　" */
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "　ビジーのため実行できません。" */
//		Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//		mode = 2;
//		break;
//	}
//
//	for ( ; ; ) {
//		msg = StoF(GetMessage(), 1);
//		// 受信完了待ち
//		if( mode == 0 ){
//			switch (msg) {
//			case KEY_MODECHG:			// ﾓｰﾄﾞﾁｪﾝｼﾞ
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_CHG;
//			case TIMEOUT7:				// 文字点滅
//				rev ^= 1;
//				grachr(5, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "　　しばらくお待ち下さい　　　" */
//				if( rev ){
//					Lagtim(OPETCBNO, 7, 50);	// 文字点滅用タイマーリスタート(反転=1sec)
//				}
//				else{
//					Lagtim(OPETCBNO, 7, 25);	// 文字点滅用タイマーリスタート(通常=500msec)
//				}
//				break;
//			case CRE_EVT_02_OK:			// 正常
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				displclr(3);
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[78]);			/* "　　　　　正常終了　　　　　　" */
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[82]);			/* "　正常に接続されていることを　" */
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[83]);			/* "　　　確認できました。　　　　" */
//				Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
//				mode = 2;
//				break;
//			case CRE_EVT_02_NG:			// サーバからエラー受信
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				displclr(3);
//				displclr(5);
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);			/* "　　　　　異常終了　　　　　　" */
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[165]);			/* "　　 エラーを受信しました 　　" */
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[166]);			/* "（コード：ＸＸＸＸ－ＸＸＸＸ）" */
//				Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//				opedsp(5, 10, Recv137_02->Result1, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				opedsp(5, 20, Recv137_02->Result2, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
//				mode = 2;
//				break;
//			case TIMEOUT9:				// 受信タイムアウト
//				if( cre_ctl.Status == CRE_STS_IDLE ){	// 応答待ちﾀｲﾑｱｳﾄ
//					//リトライ不可能な場合
//					Lagcan(OPETCBNO, 7);
//					BUZPIPI();
//					displclr(3);
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "　　　　　異常終了　　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[80]);		/* "　　　通信タイムアウトが　　　" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[81]);		/* "　　　　発生しました。　　　　" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;
//				}
//				break;
//			default:
//				break;
//			}
//		}
//		// 結果表示中
//		else {	// if (mode == 2)
//			switch (msg) {
//			case KEY_MODECHG:	// ﾓｰﾄﾞﾁｪﾝｼﾞ
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:	// 終了
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット未送信売上依頼データ         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreUnSend(void)                                   |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	UsMnt_CreUnSend( void )
//{
//	ushort	msg;
//	uchar	mode;
//	ushort	rev = 0;	//反転表示用
//	short	ret;
//	char	repaint;	// 画面再描画フラグ
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//	uchar	key_flg;	// Ｆキー： 0=無効、1=有効
//
//	repaint = 1;
//	mode = 0;
//	if( cre_uriage.UmuFlag == ON ){						// 未送信データあり
//		key_flg = 1;
//	}else{
//		key_flg = 0;
//	}
//
//	for ( ; ; ) {
//
//		if (repaint) {
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[160]);	/* "＜未送信売上データ＞　　　　　" */
//			if( cre_uriage.UmuFlag == ON ){						// 未送信データあり
//
//				Fun_Dsp(FUNMSG2[60]);							/* "　　　　　　 ﾌﾟﾘﾝﾄ 送信  終了 " */
//				if( mode == 0 ){
//					cre_unsend_data_dsp( &cre_uriage.back );	//データ表示
//				}
//			}
//			else {												// 未送信データなし
//				Fun_Dsp(FUNMSG[8]);								// "　　　　　　　　　　　　 終了 "
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "　　　データはありません　　　"
//			}
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		/* 送信前～受信完了 */
//		if( mode < 2 ){
//			switch (msg) {
//			case KEY_MODECHG:			// ﾓｰﾄﾞﾁｪﾝｼﾞ
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_CHG;
//
//			case KEY_TEN_F1:			/* F1:削除 */
//				break;
//
//			case KEY_TEN_F3:			// プリント
//				if( mode == 1 ){
//					break;
//				}
//				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
//					BUZPIPI();
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// 未送信データなし
//					if( key_flg ){
//						/*  */
//						BUZPIPI();
//						repaint = 1;
//						key_flg = 0;
//					}
//					break;
//				}
//				BUZPI();
//
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// 印字時刻	：現在時刻
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// 売上依頼ﾃﾞｰﾀ
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// 印字要求
//
//				wopelg( OPLOG_CRE_CAN_PRN,0,0 );			// 操作履歴登録
//
//				/* プリント終了を待ち合わせる */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* プリント終了 */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* タイムアウト(10秒)検出 */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* モードチェンジ */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//
//				repaint = 1;
//				break;
//
//			case KEY_TEN_F4:			// 送信
//				if( key_flg == 0 ){
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// 未送信データなし
//					BUZPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "　　　　　 正常終了 　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "　　　 送信完了しました 　　　" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;		// 送信済み
//					key_flg = 0;	// 次回からはキー無効
//					break;
//				}
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				rev = 0;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "　　しばらくお待ち下さい　　　" */
//
//				cre_ctl.SalesKind = CRE_KIND_MANUAL;		// 送信要因に手動を設定
//				if(cre_ctl.Initial_Connect_Done == 0) {		// 未開局?
//					cre_ctl.OpenKind = CRE_KIND_AUTO;		// 開局発生要因に自動時を設定
//					ret = creCtrl( CRE_EVT_SEND_OPEN );		// 開局コマンド送信
//				}
//				else {
//					ret = creSendData_SALES();				//売上依頼データ送信
//				}
//
//				if( ret == 0 ){								//正常受付なら
//					Lagtim(OPETCBNO, 7, 25);				//文字点滅用タイマーセット(500msec)
//					wopelg( OPLOG_CRE_UNSEND_SEND, 0, 0 );	//未送信売上再送
//					mode = 1;	// 送信中
//					Fun_Dsp(FUNMSG[0]);								// "　　　　　　　　　　　　　　 "
//				}else if( ret == 1 ){						//受付不可（ここには来ない）
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "　　　 送信失敗しました 　　　" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;	// 送信済み
//				}else if( ret == -2 ){						//通信処理中
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "　　　　　異常終了　　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "　　現在クレジット処理中。　　" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "　ビジーのため実行できません。" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;	// 送信済み
//				}
//				break;
//
//			case KEY_TEN_F5:			// 終了
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_EXT;
//
//			case CRE_EVT_02_NG:			//開局処理エラー
//				// no break
//			case CRE_EVT_06_OK:			//正常
//				// no break
//			case CRE_EVT_06_NG:			//サーバからエラー受信
//				if( mode != 1 ){
//					break;				// 送信中でない場合は無視する
//				}
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				dispmlclr( 2, 6 );
//				if(msg == CRE_EVT_02_NG || 
//					(msg == CRE_EVT_06_NG && ((DATA_KIND_137_06 *)cre_ctl.RcvData)->Result1 == 99)) {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "　　　　　 異常終了 　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "　　　 送信失敗しました 　　　" */
//				}
//				else {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "　　　　　 正常終了 　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "　　　 送信完了しました 　　　" */
//				}
//				Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
//				mode = 2;	// 送信済み
//				break;
//
//			case TIMEOUT7:				// 文字点滅
//				rev ^= 1;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "　　しばらくお待ち下さい　　　" */
//				if( rev ){
//					Lagtim(OPETCBNO, 7, 50);	// 文字点滅用タイマーリスタート(反転=1sec)
//				}
//				else{
//					Lagtim(OPETCBNO, 7, 25);	// 文字点滅用タイマーリスタート(通常=500msec)
//				}
//				break;
//			case TIMEOUT9:				// 受信タイムアウト
//				if( mode != 1 ){
//					break;				// 送信中でない場合は無視する
//				}
//				if( cre_ctl.Status == CRE_STS_IDLE ){		// 応答待ちﾀｲﾑｱｳﾄ
//					Lagcan(OPETCBNO, 7);
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "　　　　　 異常終了 　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "　　　 送信失敗しました 　　　" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;	// 送信済み
//				}
//				break;
//			default:
//				break;
//			}
//		}
//		// 結果表示中
//		else {	// mode >= 2
//			switch (msg) {
//			case KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:		// 終了
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  クレジット未送信売上依頼データ データ表示                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_unsend_data_dsp                                     |*/
///*| PARAMETER    : index 表示対象                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_unsend_data_dsp(struct DATA_BK *data)
//{
//	date_time_rec	*time;
//
//	time = &data->time;
//	displclr(2);
//
//	//精算日
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "精算日　　：　　　　　　　　　"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//精算時刻
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "精算時刻　：　　　　　　　　　"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//精算金額
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "精算金額　：　　　　　　円　　"
//
//	if( data->ryo > 999999L ){							// 999999超えたら"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "　　　　　　＊＊＊＊＊＊"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//取引№
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "取引№　　　　　　　　　　　　"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890　　　　　"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット売上拒否データ               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreSalesNG( void )                                 |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	UsMnt_CreSaleNG( void )
//{
//	ushort	msg;
//	char	repaint;	// 画面再描画フラグ
//	short	show_index;	// 表示中のインデックス
//	char	data_count;	// 更新後のデータ件数
//	T_FrmSaleNGPriReq	saleng_pri;
//
//	dispclr();									// 画面クリア
//
//	repaint = 1;
//	show_index = 0;
//	data_count = 0;
//
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[161]);	/* "＜売上拒否データ＞　" */
//
//	for ( ; ; ) {
//	
//		if (repaint) {
//			cre_saleng_show_data( show_index );	//データ表示
//			data_count = cre_saleng.ken;
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F1:			/* F1:▲ */
//			if (data_count > 1) {	// データ数が2以上の時有効
//				BUZPI();
//				if (--show_index < 0) {
//					show_index = cre_saleng.ken - 1;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F2:			/* F2:▼ */
//			if (data_count > 1) {	// データ数が2以上の時有効
//				BUZPI();
//				if (++show_index > cre_saleng.ken - 1) {
//					show_index = 0;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F3:			/* F3:プリント */
//			if (data_count != 0) {	// データ数が1以上の時有効
//				if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				saleng_pri.prn_kind = R_PRI;
//				saleng_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &saleng_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );						// 印字時刻	：現在時刻
//				saleng_pri.ng_data = &cre_saleng;														// 印字時刻	：売上拒否ﾃﾞｰﾀ
//				queset( PRNTCBNO, PREQ_CREDIT_SALENG, (char)sizeof(T_FrmSaleNGPriReq), &saleng_pri );	// 印字要求
//
//				/* プリント終了を待ち合わせる */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* プリント終了 */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* タイムアウト(10秒)検出 */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* モードチェンジ */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//			}
//			break;
//		case KEY_TEN_F4:			/* F4:削除 */
//			break;
//
//		case KEY_TEN_F5:			/* F5:終了 */
//			BUZPI();
//			return( MOD_EXT );
//		case KEY_MODECHG:	// モードチェンジ
//			BUZPI();
//			return( MOD_CHG );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット売上拒否-ファンクション表示  |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreSaleNG_fnc(char ken)                           |*/
///*| PARAMETER    : char ken : 件数                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void	UsMnt_CreSaleNG_fnc( char ken )
//{
//	if( ken > 1 ){	// 2件以上
//		Fun_Dsp(FUNMSG2[61]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ　　　 終了 "
//	} else {		// 2件未満
//		Fun_Dsp(FUNMSG2[62]);						// "　　　　　　 ﾌﾟﾘﾝﾄ　　　 終了 "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ 表示                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_show_data                                    |*/
///*| PARAMETER    : index     : 表示するテーブル番号                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_show_data( short index )
//{
//	if (cre_saleng.ken != 0) {		// データ有り
//		cre_saleng_num_dsp( (uchar)(index + 1), cre_saleng.ken );	// [ / ]
//		cre_saleng_data_dsp( index );
//		UsMnt_CreSaleNG_fnc( cre_saleng.ken );
//	} else {						// データ無し
//		cre_saleng_num_dsp( 0, 0 );
//		displclr(1);
//		displclr(2);
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "　　　データはありません　　　"
//		displclr(4);
//		displclr(5);
//		displclr(6);
//		Fun_Dsp(FUNMSG[8]);					// "　　　　　　　　　　　　 終了 "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ 件数表示                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_num_dsp( uchar numerator, char denominator ) |*/
///*| PARAMETER    : numerator   分子                                        |*/
///*| PARAMETER    : denominator 分母                                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_saleng_num_dsp( uchar numerator, char denominator )
//{
//	// 枠を表示
//	grachr(0, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[31]);	// [  /  ]
//
//	// 分子を表示
//	opedsp3(0, 24, (unsigned short)numerator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//
//	// 分母を表示
//	opedsp3(0, 27, (unsigned short)denominator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ データ表示                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_data_dsp                                     |*/
///*| PARAMETER    : index 表示対象                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_data_dsp( short index )
//{
//	struct DATA_BK	*data;
//	date_time_rec	*time;
//
//	data = &cre_saleng.back[ index ];
//	time = &data->time;
//	displclr(2);
//
//	//精算日
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "精算日　　：　　　　　　　　　"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//精算時刻
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "精算時刻　：　　　　　　　　　"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//精算金額
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "精算金額　：　　　　　　円　　"
//
//	if( data->ryo > 999999L ){							// 999999超えたら"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "　　　　　　＊＊＊＊＊＊"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//取引№
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "取引№　　　　　　　　　　　　"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890　　　　　"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  日付表示  全角 (ｘｘ年／ｘｘ月／ｘｘ日)                               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : date_dsp3                                               |*/
///*| PARAMETER    : ushort low  : 行                                        |*/
///*| PARAMETER    : ushort col  : 列                                        |*/
///*| PARAMETER    : ushort hour : 月                                        |*/
///*| PARAMETER    : ushort min  : 日                                        |*/
///*|              : ushort rev  : 0:正転 1:反転                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void	date_dsp3(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev)
//{
//
//	opedsp(low, col  , year, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* 年 */
//	grachr(low, (unsigned short)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "／" */
//	opedsp(low, (unsigned short)(col+6)  , month, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 月 */
//	grachr(low, (unsigned short)(col+10), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "／" */
//	opedsp(low, (unsigned short)(col+12), day , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* 日 */
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ユーザーメンテナンス：利用明細ログ                					   |*
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : UsMnt_CreUseLog										   |*
// *| PARAMETER    : none													   |*
// *| RETURN VALUE : MOD_CHG : mode change								   |*
// *|              : MOD_EXT : F5 key										   |*
// *[]----------------------------------------------------------------------[]*
// *| REMARK       : 														   |*
// *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//ushort	UsMnt_CreUseLog(void)
//{
//	date_time_rec	NewTime;
//	date_time_rec	OldTime;
//	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
//	ushort			pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
//	T_FrmLogPriReq3	FrmLogPriReq3;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
//	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
//	ushort			Date_Now[6];		// 日付ﾃﾞｰﾀ（[0]：開始年、[1]：開始月、[2]：開始日、[3]：終了年、[4]：終了月、[5]：終了日）
//	uchar			pos		= 0;		// 期間ﾃﾞｰﾀ入力位置（０：開始年、１：開始月、２：開始日、３：終了年、４：終了月、５：終了日）
//	short			inp		= -1;		// 入力ﾃﾞｰﾀ
//	uchar			All_Req	= OFF;		// 「全て」指定中ﾌﾗｸﾞ
//	uchar			Date_Chk;			// 日付指定ﾃﾞｰﾀﾁｪｯｸﾌﾗｸﾞ
//	ushort			Sdate;				// 開始日付
//	ushort			Edate;				// 終了日付
//	uchar			end_flg;			// クリア操作画面終了ﾌﾗｸﾞ
//	ushort			Date_Old[6];		// 日付ﾃﾞｰﾀ（開始年[0],月[1],日[2],時[3],分[4]、終了[5],月[6],日[7],時[8],分[9]）
//	ushort			LogCnt_total;		// 総ログ件数
//	ushort			LogCnt;				// 指定期間内対象ログ件数
//
//	Ope2_Log_NewestOldestDateGet( eLOG_CREUSE, &NewTime, &OldTime, &LogCnt_total );
//
//	/* 初期画面表示 */
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[88] );	// "＜クレジット利用明細＞　　　　"
//	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// 最古／最新ログ日付表示
//	Date_Now[0] = (ushort)OldTime.Year;								// 開始（年）←最古ログ（年）
//	Date_Now[1] = (ushort)OldTime.Mon;								// 開始（月）←最古ログ（月）
//	Date_Now[2] = (ushort)OldTime.Day;								// 開始（日）←最古ログ（日）
//	
//	Date_Now[3] = (ushort)NewTime.Year;								// 開始（年）←最新ログ（年）
//	Date_Now[4] = (ushort)NewTime.Mon;								// 開始（月）←最新ログ（月）
//	Date_Now[5] = (ushort)NewTime.Day;								// 開始（日）←最新ログ（日）
//	memcpy(Date_Old, Date_Now, sizeof(Date_Old));					// 変更前日付ﾃﾞｰﾀセーブ
//	LogDateDsp4( &Date_Now[0] );									// 最古／最新ログ日付データ表示
//	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
//	LogDateDsp5( &Date_Now[0], pos );								// 印刷日付データ表示
//	
//	// 指定期間内ログ件数取得
//	Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);	// 開始日付ﾃﾞｰﾀ取得
//	Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);	// 終了日付ﾃﾞｰﾀ取得
//	LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//	LogCntDsp( LogCnt );											// ログ件数表示
//	
//	Fun_Dsp( FUNMSG[83] );											// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "
//
//	/* 画面操作受け付け */
//	for( ; ; ){
//		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち
//
//		/*  印字要求前（印字要求前画面）*/
//		if( pri_cmd == 0 ){
//			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？
//
//				/* 設定キー切替 */
//				case KEY_MODECHG:
//					RcvMsg = MOD_CHG;						// 前画面に戻る
//					break;
//
//				/* ０～９ */
//				case KEY_TEN:
//					if( All_Req == OFF ){					// 「全て」指定中でない場合（指定中は、無視する）
//						BUZPI();
//						if( inp == -1 ){
//							inp = (short)(RcvMsg - KEY_TEN0);
//						}else{
//							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
//						}
//						opedsp(								// 入力ﾃﾞｰﾀ表示
//								POS_DATA1_0[pos][0],		// 表示行
//								POS_DATA1_0[pos][1],		// 表示ｶﾗﾑ
//								(ushort)inp,				// 表示ﾃﾞｰﾀ
//								POS_DATA1_0[pos][2],		// ﾃﾞｰﾀ桁数
//								POS_DATA1_0[pos][3],		// ０サプレス有無
//								1,							// 反転表示：あり
//								COLOR_BLACK,				// 文字色：黒
//								LCD_BLINK_OFF				// ブリンク：しない
//						);
//					}
//					break;
//
//				/* 取消キー */
//				case KEY_TEN_CL:
//					BUZPI();
//					displclr( 1 );							// 1行目表示クリア
//					if( All_Req == OFF ){	// 「全て」指定中でない場合
//						opedsp(								// 入力前のﾃﾞｰﾀを表示させる
//								POS_DATA1_0[pos][0],		// 表示行
//								POS_DATA1_0[pos][1],		// 表示ｶﾗﾑ
//								Date_Now[pos],				// 表示ﾃﾞｰﾀ
//								POS_DATA1_0[pos][2],		// ﾃﾞｰﾀ桁数
//								POS_DATA1_0[pos][3],		// ０サプレス有無
//								1,							// 反転表示：あり
//								COLOR_BLACK,				// 文字色：黒
//								LCD_BLINK_OFF				// ブリンク：しない
//						);
//						inp = -1;							// 入力状態初期化
//					}
//					break;
//
//				/* Ｆ１（←）Ｆ２（→）キー押下 */
//				case KEY_TEN_F1:
//				case KEY_TEN_F2:
//					displclr( 1 );							// 1行目表示クリア
//					if( All_Req == OFF ){	// 「全て」指定中でない場合は（　⊂　　⊃　キー：日付入力）
//						if( inp == -1 ){		// 入力なしの場合
//							BUZPI();
//							if( RcvMsg == KEY_TEN_F1 ){
//								LogDatePosUp( &pos, 0 );			// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
//							}else{
//								LogDatePosUp( &pos, 1 );			// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
//							}
//						}
//						else{				//	入力ありの場合
//							if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ
//								BUZPI();
//								if( pos == 0 ){							// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
//									Date_Now[pos] =
//									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//								}else{
//									Date_Now[pos] =
//									Date_Now[pos+3] = (ushort)inp;
//								}
//								if( RcvMsg == KEY_TEN_F1 ){
//									LogDatePosUp( &pos, 0 );			// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
//								}else{
//									LogDatePosUp( &pos, 1 );			// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
//								}
//							}
//							else{								//	入力ﾃﾞｰﾀＮＧ
//								BUZPIPI();
//							}
//						}
//						LogDateDsp5( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
//						inp = -1;								// 入力状態初期化
//					}
//					else if( RcvMsg == KEY_TEN_F2 ){		// 「全て」指定中場合は（Ｆ２：「クリア」キー）
//						BUZPI();
//						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "　　ログをクリアしますか？　　"
//						Fun_Dsp(FUNMSG[19]);							// "　　　　　　 はい いいえ　　　"
//
//						for( end_flg = OFF ; end_flg == OFF ; ){
//							RcvMsg = StoF( GetMessage(), 1 );			// イベント待ち
//
//							switch( RcvMsg ){							// イベント？
//								case KEY_MODECHG:						// 設定キー切替
//									RcvMsg = MOD_CHG;					// 前画面に戻る
//									end_flg = ON;
//									break;
//
//								case KEY_TEN_F3:						// Ｆ３（はい）キー押下
//									BUZPI();
//									wopelg( OPLOG_CRE_USE_DEL,0,0 );		// 操作履歴登録
//									if( LogDataClr( eLOG_CREUSE ) == OK ){	// ログデータクリア
//										LogCnt_total = 0;				// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
//										LogCntDsp( LogCnt_total );		// ログ件数表示（０件）
//										Date_Now[0] = Date_Now[3] = (ushort)CLK_REC.year;	// 開始、終了（年）←現在日時（年）
//										Date_Now[1] = Date_Now[4] = (ushort)CLK_REC.mont;	// 開始、終了（月）←現在日時（月）
//										Date_Now[2] = Date_Now[5] = (ushort)CLK_REC.date;	// 開始、終了（日）←現在日時（日）
//									}
//									end_flg = ON;
//									break;
//
//								case KEY_TEN_F4:						// Ｆ４（いいえ）キー押下
//									BUZPI();
//									end_flg = ON;
//									break;
//							}
//							if( end_flg != OFF ){
//								displclr( 6 );										// "　　ログをクリアしますか？　　"表示行クリア
//								LogDateDsp4( &Date_Now[0] );						// 最古／最新ログ日付データ表示
//								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
//								LogDateDsp5( &Date_Now[0], pos );					// 印刷日付データ表示
//								Fun_Dsp( FUNMSG[83] );								// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
//								All_Req = OFF;										// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
//							}
//						}
//					}
//					break;
//
//				/* Ｆ３（全て）キー押下 */
//				case KEY_TEN_F3:
//					displclr( 1 );							// 1行目表示クリア
//					if( All_Req == OFF ){	// 「全て」指定中？（指定中は、無視する）
//						if( LogCnt_total != 0 ){		//	ログデータがある場合
//							BUZPI();
//							Date_Now[0] = (ushort)OldTime.Year;	// 開始（年）←最古ログ（年）
//							Date_Now[1] = (ushort)OldTime.Mon;	// 開始（月）←最古ログ（月）
//							Date_Now[2] = (ushort)OldTime.Day;	// 開始（日）←最古ログ（日）
//							
//							Date_Now[3] = (ushort)NewTime.Year;	// 開始（年）←最新ログ（年）
//							Date_Now[4] = (ushort)NewTime.Mon;	// 開始（月）←最新ログ（月）
//							Date_Now[5] = (ushort)NewTime.Day;	// 開始（日）←最新ログ（日）
//							pos = 0;								// ｶｰｿﾙ位置＝開始（年）
//							inp = -1;								// 入力状態初期化
//							displclr( 5 );						// 印刷日付表示クリア
//							// 指定期間内対象ログ件数を取得し表示する
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
//							Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// 終了日付ﾃﾞｰﾀ取得
//							LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//							LogCntDsp( LogCnt );					// ログ件数表示
//							Fun_Dsp( FUNMSG[85] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
//							All_Req = ON;							// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
//						}
//						else{					//	ログデータがない場合
//							BUZPIPI();
//						}
//					}
//					break;
//
//				/* Ｆ４（実行）キー押下 */
//				case KEY_TEN_F4:
//					displclr( 1 );							// 1行目表示クリア
//					Date_Chk = OK;
//					if( inp != -1 ){	//	入力ありの場合
//						if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫの場合
//							if( pos == 0 ){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
//								Date_Now[pos] =
//								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//							}else{
//								Date_Now[pos] =
//								Date_Now[pos+3] = (ushort)inp;
//							}
//						}else{								//	入力データＮＧの場合
//							Date_Chk = NG;
//						}
//					}
//					// 指定期間内対象ログ件数を取得し表示する
//					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
//					Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// 終了日付ﾃﾞｰﾀ取得
//					LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//					if( LogCnt == 0 ){
//						BUZPIPI();
//						grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// データなし表示
//						inp = -1;									// 入力状態初期化
//						break;
//					}
//					if( Date_Chk == OK ){
//						if( All_Req != ON ){	// 「全て」指定でない場合、年月日ﾁｪｯｸを行う
//							//	年月日が存在する日付かチェックする
//							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// 開始日付ﾁｪｯｸ
//								Date_Chk = NG;
//							}
//							if( chkdate( (short)Date_Now[3], (short)Date_Now[4], (short)Date_Now[5] ) != 0 ){	// 終了日付ﾁｪｯｸ
//								Date_Chk = NG;
//							}
//						}
//					}
//					if( Date_Chk == OK ){
//						//	開始日付＜＝終了日付かチェックする
//						Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
//											(short)Date_Now[0],
//											(short)Date_Now[1],
//											(short)Date_Now[2]
//										);
//						Edate = dnrmlzm(							// 終了日付ﾃﾞｰﾀ取得
//											(short)Date_Now[3],
//											(short)Date_Now[4],
//											(short)Date_Now[5]
//										);
//						if( Sdate > Edate ){						// 開始日付／終了日付ﾁｪｯｸ
//							Date_Chk = NG;
//						}
//					}
//					if( Date_Chk == OK ){	// 開始日付＆終了日付ﾁｪｯｸＯＫの場合
//						if(Ope_isPrinterReady() == 0){					// レシートプリンタが印字不可能な状態
//							BUZPIPI();
//							break;
//						}
//						BUZPI();
//						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
//						memset( &FrmLogPriReq3,0,sizeof(FrmLogPriReq3) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
//						memcpy( &FrmLogPriReq3.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// 印字時刻	：現在時刻
//						FrmLogPriReq3.prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
//						FrmLogPriReq3.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
//						FrmLogPriReq3.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.
//
//						if( All_Req == ON ){
//							FrmLogPriReq3.BType	= 0;							// 検索方法	：全て
//						}else{
//							FrmLogPriReq3.BType	= 1;							// 検索方法	：日付
//							FrmLogPriReq3.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq3.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq3.TSttTime.Day	= (uchar)Date_Now[2];
//						}
//						FrmLogPriReq3.LogCount = LogCnt_total;	// LOG登録件数(個別精算LOG・集計LOGで使用)
//						queset( PRNTCBNO, PREQ_CREDIT_USE, sizeof(T_FrmLogPriReq3), &FrmLogPriReq3 );
//						Ope_DisableDoorKnobChime();
//						pri_cmd = PREQ_CREDIT_USE;				// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
//						if( All_Req != ON ){					// 「全て」指定なし
//							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
//							LogDateDsp5( &Date_Now[0], 0xff );	// 印刷日付データ表示
//						}
//						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// 実行中ブリンク表示
//						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"
//
//					}else{	// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
//						BUZPIPI();
//						LogDateDsp5( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
//						inp = -1;								// 入力状態初期化
//					}
//					break;
//
//				/* Ｆ５（終了）キー押下 */
//				case KEY_TEN_F5:							// 
//					BUZPI();
//					if( All_Req == OFF ){	// 「全て」指定中でない場合
//						RcvMsg = MOD_EXT;						// 前画面に戻る
//					}else{					//	「全て」指定中の場合
//						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
//						inp = -1;							// 入力状態初期化
//						opedsp(								// ｶｰｿﾙを表示させる
//								POS_DATA1_0[pos][0],		// 表示行
//								POS_DATA1_0[pos][1],		// 表示ｶﾗﾑ
//								Date_Now[pos],				// 表示ﾃﾞｰﾀ
//								POS_DATA1_0[pos][2],		// ﾃﾞｰﾀ桁数
//								POS_DATA1_0[pos][3],		// ０サプレス有無
//								1,							// 反転表示：あり
//								COLOR_BLACK,				// 文字色：黒
//								LCD_BLINK_OFF				// ブリンク：しない
//						);
//						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
//						LogDateDsp5( &Date_Now[0], pos );	// 印刷日付データ表示
//						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "
//						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
//					}
//					break;
//				default:
//					break;
//			}
//		}
//		/* 印字要求後（印字終了待ち画面）*/
//		else{
//			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
//				RcvMsg = MOD_EXT;							// YES：前画面に戻る
//			}
//
//			switch( RcvMsg ){								// イベント？
//
//				case KEY_MODECHG:	// 設定キー切替
//					RcvMsg = MOD_CHG;						// 前画面に戻る
//					break;
//
//				case KEY_TEN_F3:	// Ｆ３（中止）キー押下
//					BUZPI();
//					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
//					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
//					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
//					RcvMsg = MOD_EXT;						// 前画面に戻る
//					break;
//
//				default:
//					break;
//			}
//		}
//
//		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
//			break;											// 前画面に戻る
//		}
//	}
//	return( RcvMsg );
//}
//
//ushort	UsMnt_CreUseLog_GetLogCnt(ushort Sdate, ushort Edate)
//{
//	ushort			log_date;			// ログ格納日付
//	ushort			LOG_Date[3];		// ログ格納日付（[0]：年、[1]：月、[2]：日）
//	ushort			log_cnt;			// 指定期間内ログ件数
//	ushort			cnt;
//	meisai_log 		wk_log_dat;
//	ushort			LogCnt_total;
//	
//	log_cnt = 0;
//	LogCnt_total = Ope_Log_TotalCountGet( eLOG_CREUSE );
//	
//	for(cnt=0; cnt < LogCnt_total; cnt++) {
//		if( 0 == Ope_Log_1DataGet(eLOG_CREUSE, cnt, &wk_log_dat) ){
//			log_cnt = 0;
//			break;
//		}
//		LOG_Date[0] = wk_log_dat.PayTime.Year;
//		LOG_Date[1] = wk_log_dat.PayTime.Mon;
//		LOG_Date[2] = wk_log_dat.PayTime.Day;
//		log_date = dnrmlzm( (short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]);
//		if( (log_date >= Sdate) && (log_date <= Edate) ){
//			log_cnt++;
//		}
//	}
//	return(log_cnt);
//}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能 - Ｅｄｙ処理ﾒﾆｭｰ					   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_EdyMnu( void )                                    |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-11-06 Suzuki                                       |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//unsigned short	UsMnt_EdyMnu(void)
// {
// 	unsigned short	usUcreEvent;
// 	char	wk[2];
// 	unsigned char	MenuValue = 0;					// メニュー項目の表示数
// 	CeMenuPara make_table[] = 						// メニュー表示条件テーブル
// 	{
// 		{0,			0,			0,		0},			// アラーム取引情報
// 		{0,			0,			0,		0},			// センター通信開始
// 		{0,			0,			0,		0},			// 締め記録情報
// 		{S_SCA,		59,			5,		1},			// 日毎集計プリント
// 		{S_SCA,		59,			6,		1},			// 利用明細プリント
// #if (4 == AUTO_PAYMENT_PROGRAM)										// 試験用にEMへｺﾏﾝﾄﾞ送信（debug処理）
// 		{0,			0,			0,		0},			// 電文送信テスト
// #endif
// 	};
//
// 	DP_CP[0] = DP_CP[1] = 0;
//
// 	/* どの印字機能が有効であるかを共通パラメータから取得し、表示可否の判断をする */
// 	MenuValue = UsMnt_DiditalCasheMenuMake31(make_table, EXTENDMENU_EDY, USM_EXTEDY_TBL, (sizeof(make_table) / sizeof(CeMenuPara)));
//
// 	for( ; ; ) {
//
// 		dispclr();								// 画面クリア
// 		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[101] );	// "＜Ｅｄｙ処理＞　　　　　　　　"
//
// 		usUcreEvent = Menu_Slt( (void*)UsMnt_DiditalCasheMENU_STR, (void*)UsMnt_DiditalCasheMENU_CTL, (char)MenuValue, (char)1 );
// 		wk[0] = DP_CP[0];
// 		wk[1] = DP_CP[1];
//
// 		switch( usUcreEvent ){
// 			case MNT_EDY_ARM:										// アラーム取引情報
// 				usUcreEvent = UsMnt_Edy_PayArmPrint();
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_CENT:
// 				usUcreEvent = UsMnt_Edy_CentCon( );					// センター通信開始
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_SHIME:										// Ｅｄｙ締め記録情報
// 				usUcreEvent = UsMnt_Edy_ShimePrint();
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_MEISAI:									// Ｅｄｙ利用明細
// 				usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_EDY_MEISAI);
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_SHUUKEI:									// Ｅｄｙ集計
// 				usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_EDY_SHUUKEI);
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 		#if (4 == AUTO_PAYMENT_PROGRAM)								// 試験用にEMへｺﾏﾝﾄﾞ送信（debug処理）
// 			case MNT_EDY_TEST:
// 				usUcreEvent = UsMnt_Edy_Test( );					// センター通信開始
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
// 		#endif
//
// 			case MOD_EXT:		// 終了（Ｆ５）
// 				return(MOD_EXT);
//
// 			case MOD_CHG:		// モードチェンジ
// 				return(MOD_CHG);
//
// 			default:
// 				break;
// 		}
// 		DP_CP[0] = wk[0];
// 		DP_CP[1] = wk[1];
// 	}
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ユーザーメンテナンス：拡張機能 - Ｅｄｙ処理（１．アラーム取引情報）   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_PayArmPrint( void )                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_PayArmPrint( void )
// {
// 	ushort	usSysEvent;
// 	ushort	NewOldDate[6];				// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
// 	ushort	LogCount;					// LOG登録件数
// 	date_time_rec	NewestDateTime, OldestDateTime;
//
//  	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
// 	Ope2_Log_NewestOldestDateGet( eLOG_EDYARM, &NewestDateTime, &OldestDateTime, &LogCount );
//
// 	// 最古＆最新日付ﾃﾞｰﾀ取得
// 	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
//
// 	//	ログ件数表示画面処理２（対象期間指定あり）
// 	usSysEvent = SysMnt_Log_CntDsp2	(
// 										LOG_EDYARM,			// ログ種別
// 										LogCount,			// ログ件数
// 										UMSTR3[102],		// タイトル表示データポインタ
// 										PREQ_EDY_ARMPAY_LOG,// 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
// 										&NewOldDate[0]		// 最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
// 									);
// 	return usSysEvent;
//
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ユーザーメンテナンス：拡張機能 - Ｅｄｙ処理（２．センター通信開始）   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_CentCon( void )                               |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*| Date         : 2007-02-26                                              |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_CentCon( void )
// {
// 	R_CENTER_TRAFFIC_RESULT_DATA	*p_rcv_data;		// センター通信実施結果データﾎﾟｲﾝﾀ
// 	R_CENTER_TRAFFIC_CHANGE_DATA	*p_rcv_center;		// ｾﾝﾀｰ通信状況変化通知データﾎﾟｲﾝﾀ
// 	ushort	msg;
// 	char	wk_stat;	// 実施状態(0:初期画面/1:開始指示/2:通信開始/3:通信終了/9:異常応答)
// 	uchar	EndKey_stat = 0;							// F5キー有効(1)／無効(0)
// 	ushort	rev;
// 	uchar	LimitTime;									// ｾﾝﾀｰ通信終了Limit時間
//
// 	/* Edy利用可能状態かをチェックする */
// 	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 &&		// Edy利用可能設定か？
// 		Edy_Rec.edy_EM_err.BIT.Comfail == 0 &&			// EM通信エラーが発生していないか？
// 		Edy_Rec.edy_status.BIT.INITIALIZE ) {			// Edyﾓｼﾞｭｰﾙの初期化が完了しているか？
// 		dispclr();										// 初期画面表示
// 		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[103]);				/* "＜センター通信＞          　　" */
// 		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[104]);				/* "　Ｅｄｙセンターと接続します　" */
// 		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[105]);				/* "通信中は他の操作はできません" */
// 		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[106]);				/* "　実行してもよろしいですか？　" */
// 		Fun_Dsp(FUNMSG[19]);							/* "　　　　　　 はい いいえ　　　" */
// 		EndKey_stat = 0;								// F5キー有効(1)／無効(0)
// 	} else {											// Edy利用不可時は本画面に遷移させない（警告音）
// 		BUZPIPI();
// 		return MOD_EXT;										
// 	}
//
// 	wk_stat = 0;											// 実施状態(0:初期画面)
// 	LimitTime = (uchar)prm_get(COM_PRM, S_SCA, 61, 2, 3);
//
// 	CneterComLimitTime = 0;
// 	Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;			// ｾﾝﾀｰ通信ﾌﾗｸﾞ初期化
//
// 	for ( ; ; ) {
//
// 		msg = StoF( GetMessage(), 1 );
//
// 		if( Edy_Rec.edy_status.BIT.CENTER_COM_START && CneterComLimitTime > LimitTime+5 ){
// 			if(wk_stat >= 1) {										// 開始指示以降のみ受付
// 				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* 表示行（3）をクリア */
// 				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "　　Ｅｄｙセンター通信終了　　" */
// 				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "　　　　【異常終了】　　　　　" */
// 				Fun_Dsp(FUNMSG[8]);									/* "　　　　　　　　　　　　 終了 " */
// 				EndKey_stat = 1;									// F5キー有効(1)／無効(0)
// 				wk_stat = 9;										// 異常応答
// 				Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// ｾﾝﾀｰ通信終了
// 				CneterComLimitTime = 0;
// 				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ﾓﾆﾀLOG登録（センター通信終了:NG）
// 			}
// 		}		
// 		switch( msg ){		/* FunctionKey Enter */
//
// 			case KEY_MODECHG:
// 				if(wk_stat == 1 || wk_stat == 2 ) break;	/* 初期画面表示中/通信完了以外は無効 */
// 				return MOD_CHG;
//
// 			case KEY_TEN_F1:								/* "IPaddr" */
// 				break;
//
// 			case KEY_TEN_F3:								/* "はい" */
// 				if(wk_stat != 0) break;						/* 初期画面表示中以外は無効 */
// 				BUZPI();
// 				auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);	// 最終センター通信開始日時更新
// 				Edy_SndData13(0);							// センター通信開始指示（締めなし）	送信
// 				wk_stat = 1;								// センター通信開始指示中
// 				Edy_Rec.edy_status.BIT.CENTER_COM_START = 1;// ｾﾝﾀｰ通信開始
// 															// 「センター通信中」画面表示
// 				grachr(2, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);			/* "　　Ｅｄｙセンターと通信中　　" （点滅表示）*/
// 				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[9]);			/* " 通信中は他の操作ができません " */
// 				Fun_Dsp(FUNMSG[0]);								/* "　　　　　　　　　　　　　　　" */
// 				EndKey_stat = 0;								// F5キー有効(1)／無効(0)
// 				rev = 1;
// 				Lagtim(OPETCBNO, 7, 50);						// 点滅用Timerスタート(1sec)
// 				break;
//
// 			case KEY_TEN_F4:										/* "いいえ" */
// 				if(wk_stat != 0) break;								// 初期画面表示中以外は無効
// 				BUZPI();
// 				return MOD_EXT;
//
// 			case KEY_TEN_F5:										/* "終了" */
// 				if(EndKey_stat == 0) break;							// F5キー無効(0)時は無効
// 				BUZPI();
// 				Lagcan(OPETCBNO, 7);								/* 点滅表示用タイマー開放 */
// 				return MOD_EXT;
//
// 			case IBK_EDY_RCV:										/* EDY 受信ｲﾍﾞﾝﾄ 受信 */
// 				switch( Edy_Rec.rcv_kind ){							// 受信ﾃﾞｰﾀ種別
//
// 					case	R_EMSET_STATUS_READ:					// EM設定/ｽﾃｰﾀｽ読み出し指示の応答
// 						memcpy(&Edy_SetStatus_ReadData, Edy_Rec.rcv_data, sizeof(Edy_SetStatus_ReadData));	// EM設定/ｽﾃｰﾀｽﾃﾞｰﾀ保存
// 						if( (Edy_SetStatus_ReadData.Line_kind == 2) ) {	// 回線種別：LAN
// 							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[108]);
// 							opedsp3(6, 12, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 16, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[1], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 20, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[2], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 24, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[3], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 						}
// 						break;			
//
// 					case	R_FIRST_STATUS:										// 初期状態通知 受信
// 						if(wk_stat >= 1) {										// 開始指示以降のみ受付
// 							grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* 表示行（3）をクリア */
// 							grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "　　Ｅｄｙセンター通信終了　　" */
// 							grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "　　　　【異常終了】　　　　　" */
// 							Fun_Dsp(FUNMSG[8]);									/* "　　　　　　　　　　　　 終了 " */
// 							EndKey_stat = 1;									// F5キー有効(1)／無効(0)
// 							wk_stat = 9;										// 異常応答
// 							wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ﾓﾆﾀLOG登録（センター通信終了:NG）
// 							Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// ｾﾝﾀｰ通信終了
// 							CneterComLimitTime = 0;
// 						}
// 						break;
//
// 					case	R_CENTER_TRAFFIC_RESULT:				// ｾﾝﾀｰ通信開始結果 受信
// 						p_rcv_data = (R_CENTER_TRAFFIC_RESULT_DATA *)&Edy_Rec.rcv_data;	// 受信ﾃﾞｰﾀﾎﾟｲﾝﾀget
// 						if(p_rcv_data->Rcv_Status[0] == 0x00) {		// 正常応答時
// 							switch(p_rcv_data->Status) {
//
// 								case 0x01:							// 開始指示受付 応答
// 									if(wk_stat == 1) {								// 開始指示中のみ受付
// 										wk_stat = 2;								// 開始受付済
// 										wmonlg( OPMON_EDY_M_CEN_START, 0, 0 );		// ﾓﾆﾀLOG登録（センター通信開始）
// 									}
// 									break;
//
// 								case 0x02:							// 通信完了
// 									if(wk_stat == 2) {								// 開始受付済時のみ受付
// 										grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);		/* 表示行（3）をクリア */
// 										grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);	/* "　　Ｅｄｙセンター通信終了　　" */
// 										grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[12]);	/* "　　　　【正常終了】　　　　　" */
// 										Fun_Dsp(FUNMSG[8]);							/* "　　　　　　　　　　　　 終了 " */
// 										EndKey_stat = 1;							// F5キー有効(1)／無効(0)
// 										wk_stat = 3;								// 通信終了
// 										wmonlg( OPMON_EDY_CEN_STOP, 0, 1 );			// ﾓﾆﾀLOG登録（センター通信終了:OK）
// 										Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;// ｾﾝﾀｰ通信終了
// 										CneterComLimitTime = 0;
// 									}
// 									break;
//
// 								default:
// 									break;
// 							}
// 						} else {									// 異常応答時
// 							if(wk_stat >= 1) {										// 開始指示以降のみ受付
// 								grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* 表示行（3）をクリア */
// 								grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "　　Ｅｄｙセンター通信終了　　" */
// 								grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "　　　　【異常終了】　　　　　" */
// 								Fun_Dsp(FUNMSG[8]);									/* "　　　　　　　　　　　　 終了 " */
// 								EndKey_stat = 1;									// F5キー有効(1)／無効(0)
// 								wk_stat = 9;										// 異常応答
// 								wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ﾓﾆﾀLOG登録（センター通信終了:NG）
// 								Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// ｾﾝﾀｰ通信終了
// 								CneterComLimitTime = 0;
// 							}
// 						}
// 						break;
//
// 					case	R_CENTER_TRAFFIC_CHANGE:				// ｾﾝﾀｰ通信状況変化通知
// 						p_rcv_center = (R_CENTER_TRAFFIC_CHANGE_DATA *)&Edy_Rec.rcv_data;	// 受信ﾃﾞｰﾀﾎﾟｲﾝﾀget
// 						if(p_rcv_center->Rcv_Status[0] == 0x00) {
// 							switch(p_rcv_center->Status_code) {
// 								case 0x01:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[19]);		/* "　　　【データ前処理中】　　　"*/
// 									break;
// 								case 0x02:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[10]);		/* "　　　【コネクション中】　　　"*/
// 									break;
// 								case 0x03:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[20]);		/* "　　　【端末ＩＤ送出中】　　　"*/
// 									break;
// 								case 0x04:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[21]);		/* "　　　【端末認証送出中】　　　"*/
// 									break;
// 								case 0x05:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[22]);		/* "　　　【セッション開始】　　　"*/
// 									break;
// 								case 0x06:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[23]);		/* "　　　【データ集信中】  　　　"*/
// 									break;
// 								case 0x07:
// 								case 0x10:
// 								case 0x11:
// 								case 0x12:
// 								case 0x13:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[24]);		/* "　　　【データ配信中】  　　　"*/
// 									break;
// 								case 0x08:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[25]);		/* "　　　【セッション終了】　　　"*/
// 									break;
// 								case 0x09:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[26]);		/* "　　　【クローズ処理中】　　　"*/
// 									break;
// 								case 0x0A:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[27]);		/* "　　　【データ後処理中】　　　"*/
// 									break;
// 								case 0x0B:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[28]);		/* "　　　【ＤＨＣＰ処理中】　　　"*/
// 									break;
// 								default:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "　　　　              　　　　"*/
// 									break;
// 							}
// 						}
// 						break;
//
// 					default:
// 						break;
// 					}
// 					break;
//
// 			case TIMEOUT7:											/* 文字点滅 */
// 				if(wk_stat == 2) {									// 通信中は点滅表示を実施する
// 					rev ^= 1;
// 					grachr(2, 0, 30, rev, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);		/* "　　Ｅｄｙセンターと通信中　　" */
// 					Lagtim(OPETCBNO, 7, 50);						// 文字点滅用タイマーリスタート
// 				}
// 				break;
//
// 			default:
// 				break;
// 		}
// 	}
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ユーザーメンテナンス：拡張機能 - Ｅｄｙ処理（３．Ｅｄｙ締め記録情報） |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_ShimePrint( void )	                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_ShimePrint( void )
// {
// 	ushort	usSysEvent;
// 	ushort	NewOldDate[6];				// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
// 	ushort	LogCount;					// LOG登録件数
// 	date_time_rec	NewestDateTime, OldestDateTime;
//
//  	// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
// 	Ope2_Log_NewestOldestDateGet( eLOG_EDYSHIME, &NewestDateTime, &OldestDateTime, &LogCount );
//
// 	// 最古＆最新日付ﾃﾞｰﾀ取得
// 	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
//
// 	//	ログ件数表示画面処理２（対象期間指定あり）
// 	usSysEvent = SysMnt_Log_CntDsp2	(
// 										LOG_EDYSHIME,		// ログ種別
// 										LogCount,			// ログ件数
// 										UMSTR3[109],		// タイトル表示データポインタ
// 										PREQ_EDY_SHIME_LOG,	// 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
// 										&NewOldDate[0]		// 最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
// 									);
// 	return usSysEvent;
//
// }
// #endif
//
// #if (4 == AUTO_PAYMENT_PROGRAM)						// 試験用にEMへｺﾏﾝﾄﾞ送信（debug処理）
// /*[]----------------------------------------------------------------------[]*/
// /*|  ユーザーメンテナンス：拡張機能 - Ｅｄｙ処理（４．電文送信テスト  ）   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_Test( void )   	                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_Test( void )
// {
// 	ushort	msg;
// 	short	input=0;
// 	ulong	input_l;
// 	ushort	input_work=0;
// 	uchar	wk_stat=0;
// 	uchar	led = 0;
// 	union{
// 		ushort	us;
// 		uchar	uc[2];
// 	}u;
// 	const unsigned char	EDY_TEST_STR[][31] = {
// 		"　　　　              　　　　", //[00]
// 		"＜電文送信テスト＞　　　　　　", //[01]
// 		"　送信電文番号：　　　　　　　", //[02]
// 		"　　　　　送信中！　　　　　　", //[02]
// 		"　　　　　受信完了　　　　　　", //[02]
// 	};
// 	/* Edy利用可能状態かをチェックする */
//
// 	dispclr();								// 画面クリア
// 	if( EDY_USE_ERR ) {						// Edyﾓｼﾞｭｰﾙのｴﾗｰ等が発生していないかつ初期化が完了しているか？
// 		BUZPIPI();
// 		return MOD_EXT;										
// 	}
//
// 	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[1]);	/* "＜電文送信テスト＞　　　　　　" */
// 	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[2]);	/* 　送信電文番号：　　　　　　　 */
// 	opedsp(2, 16, 0, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
// 	Fun_Dsp(FUNMSG2[1]);
//
// 	for ( ; ; ) {
//
// 		msg = StoF( GetMessage(), 1 );
//
//
// 		switch( KEY_TEN0to9(msg) ){			
//
// 			case KEY_MODECHG:
// 				if(wk_stat != 0) break;		/* 電文送信中は無効 */
// 				Edy_StopAndLedOff();
// 				return MOD_CHG;
//
// 			case KEY_TEN:					/* 数字(テンキー) */
// 				BUZPI();
// 				if (input == -1){			/* 送信済みの場合 */
// 					input = 0;				/* 数字(テンキー)クリア */
// 					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[0]);	/* "　　　　              　　　　" */
//
// 				}
// 				input_l = input;
// 				input = (ushort)((input_l*10 + msg-KEY_TEN0) % 10000);
// 				opedsp(2, 16, (ushort)(input), 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 電文番号表示 */
// 				break;
//
// 			case KEY_TEN_F4:								/* "書込み（電文送信）" */
// 				if( input == -1 )							/* 電文送信済み */
// 					input = input_work;						/* 続けて同じ電文を送信する場合、保持していた値をｾｯﾄ */
// 				else
// 					input_work = input;						/* 今回送信する電文を保持 */
//
// 				wk_stat = 1;								/* 送信中ﾌﾗｸﾞON */
// 				grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[3]);		/* "　　　　　送信中！　　　　　　" */
// 				BUZPI();
// 				u.uc[0] = binbcd( (uchar)(input/100) );
// 				u.uc[1] = binbcd( (uchar)(input%100) );
// 				switch( u.us ){								/* 入力値により電文を送信する */
// 					case	S_CARD_DETECTION_START:
// 						Edy_SndData01();
// 						break;
// 					case	S_SUBTRACTION:
// 						Edy_SndData02( (ulong)CPrmSS[1][5] );
// 						break;
// 					case	S_CARD_DETECTION_STOP:
// 						Edy_SndData04();
// 						break;
// 					case	S_LAST_PAY_READ:
// 						Edy_SndData05();
// 						break;
// 					case	S_CARD_STATUS_READ:
// 						Edy_SndData06();
// 						break;
// 					case	S_TAMPER_STATUS_READ:
// 						Edy_SndData07();
// 						break;
// 					case	S_EMSET_STATUS_READ:
// 						Edy_SndData10();
// 						break;
// 					case	S_EM_STATUS_READ:
// 						Edy_SndData11();
// 						break;
// 					case	S_CENTER_TRAFFIC_START:
// 						Edy_SndData13(0);
// 						break;
// 					case	S_FIRST_STATUS:
// 						Edy_SndData14();
// 						break;
// 					case	S_LED_CTRL:
// 						switch( led ){
// 							case	0:
// 								Edy_SndData15(1,0,0);
// 								led++;
// 								break;
// 							case	1:
// 								Edy_SndData15(0,1,0);
// 								led++;
// 								break;
// 							case	2:
// 								Edy_SndData15(0,0,1);
// 								led++;
// 								break;
// 							case	3:
// 								Edy_SndData15(1,1,0);
// 								led++;
// 								break;
// 							case	4:
// 								Edy_SndData15(0,1,1);
// 								led++;
// 								break;
// 							case	5:
// 								Edy_SndData15(1,0,1);
// 								led++;
// 								break;
// 							case	6:
// 								Edy_SndData15(1,1,1);
// 								led++;
// 								break;
// 							case	7:
// 								Edy_SndData15(2,0,0);
// 								led++;
// 								break;
// 							case	8:
// 								Edy_SndData15(0,2,0);
// 								led++;
// 								break;
// 							case	9:
// 								Edy_SndData15(0,0,2);
// 								led++;
// 								break;
// 							case	10:
// 								Edy_SndData15(2,2,2);
// 								led++;
// 								break;
// 							case	11:
// 								Edy_SndData15(0,0,0);
// 								led = 0;
// 								break;
// 						}
// 						break;
// 					case	S_DATE_SYN:
// 						Edy_SndData16();
// 						break;
// 					case	S_EM_CLOSE:
// 						if( prm_get(COM_PRM,S_SCA,56,1,3) == 1){
// 							Edy_Rec.edy_status.BIT.CLOSE_MODE_END = 0;
// 							Edy_Rec.edy_status.BIT.CLOSE_MODE = 1;
// 							Edy_SndData18();
// 						}
// 						break;
// 				}
// 				break;
// 			case KEY_TEN_F5:													/* "終了" */
// 				BUZPI();
// 				Edy_StopAndLedOff();											/* Edyを待機状態に戻す */
// 				return MOD_EXT;
//
// 			case KEY_TEN_CL:													/* 取消(テンキー) */
// 				opedsp(2, 16, 0, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 電文番号表示 */
// 				input = 0;
// 				break;
//
// 			case IBK_EDY_RCV:										/* EDY 受信ｲﾍﾞﾝﾄ 受信 */
// 				switch( Edy_Rec.rcv_kind ){
// 					case	R_CENTER_TRAFFIC_RESULT:
// 						if( ((R_CENTER_TRAFFIC_RESULT_DATA*)(Edy_Rec.rcv_data))->Status == 2 )
// 							break;
// 					case	R_CENTER_TRAFFIC_CHANGE:
// 					case	R_ERR_CHANGE:
// 					continue;
// 				}
// 				grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[4]);				/* "　　　　　受信完了　　　　　　" */
//  				input = -1;											
// 				wk_stat = 0;										/* 送信中ﾌﾗｸﾞOFF */	
// 			default:	
// 				break;
// 		}
// 	}
// }
// #endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)


/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：共通 系統選択画面                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PreAreaSelect( char type )                        |*/
/*| PARAMETER    : char type : MNT_FLCTL:ロック装置開閉                    |*/
/*|                          : MNT_FLSTS:車室情報                          |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PreAreaSelect(short kind)
{
	unsigned short ret;

	DP_CP[0] = DP_CP[1] = 0;

	ret = 0;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//	while (ret != MOD_CHG) {
	while (ret != MOD_CHG && ret != MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		dispclr();

		switch (kind) {
		case MNT_FLCTL:
			/* ロック装置開閉 */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[9]);		/* "＜ロック装置開閉＞　　　　　　" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_FLSTS:
			/* 車室情報 */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[2]);		/* "＜車室情報＞　　　　　　　　　" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_CARFAIL:
			/* 車室故障 */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[38]);		/* "＜車室故障＞　　　　　　　　　" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_IOLOG:
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[127]);		/* "＜入出庫情報プリント＞　　　　　　　　　" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		default:	// BKRS_FLSTS
			/* 車室情報調整 */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[64]);		/* "＜車室情報調整＞　　　　　　　" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		}
		gCurSrtPara = ret;
		
		if (ret == MOD_EXT) {
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		else if (ret != MOD_CHG) {
		else if (ret != MOD_CHG || ret != MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			switch( gCurSrtPara ){
				case	MNT_CAR:		// 駐車
				case	MNT_FLAPUD:		// フラップ上昇下降（個別）
				case	MNT_FLAPUD_ALL:	// フラップ上昇下降（全て）
				case	MNT_INT_CAR:	// 駐車(内蔵)
					ret = (ushort)GetCarInfoParam();
					switch( ret&0x06 ){
						case	0x04:
							DispAdjuster = INT_CAR_START_INDEX;
							break;
						case	0x02:
						case	0x06:
						default:
							DispAdjuster = CAR_START_INDEX;
							break;
					}
					break;

				case	MNT_BIK:		// 駐輪
				case	MNT_BIKLCK:		// 駐輪ロック開閉（個別）
				case	MNT_BIKLCK_ALL:	// 駐輪ロック開閉（全て）
					DispAdjuster = BIKE_START_INDEX;
					break;

				default:
					DispAdjuster = BIKE_START_INDEX;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}
			ret = UsMnt_AreaSelect(kind);
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：暗証番号強制出庫                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PwdKyo( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PwdKyo(void)
{
	ushort	msg;
	ushort	data;
	ushort	start_pos;
	
	data = (ushort)prm_get(COM_PRM,S_PAY,41,1,1);
	if (data == 0) {
		// 暗証番号強制出庫なし
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();
	start_pos = (data == 1) ? 1 : (ushort)(CLK_REC.mont % 10);
														/* １：開始桁１固定／２：現在の月下一桁     */
	if (start_pos == 0) start_pos = 10;					/* ０の場合１０桁目から参照                 */

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[70]);					/* "＜暗証番号強制出庫＞　　　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[71]);					/* "現在の乱数表　開始桁位置は　　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[72]);					/* "　　　　　　　　　桁目　です。" */
	if (start_pos < 10)
		opedsp(3, 16, start_pos, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 開始桁(1桁)                      */
	else
		opedsp(3, 14, start_pos, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 開始桁(2桁)                      */

	Fun_Dsp(FUNMSG[8]);																	/* "　　　　　　　　　　　　 終了 " */

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (msg == KEY_MODECHG) {
		if (msg == KEY_MODECHG || msg == MOD_CUT) {	
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			BUZPI();
			return MOD_CHG;
		}
		else if (msg == KEY_TEN_F5) {
			BUZPI();
			return MOD_EXT;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：音量切替時刻                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_VolSwTime( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_VolSwTime(void)
{
	ushort	msg;
	char	changing;	/* 1:時刻設定中 */
	char	pos;		/* 0:開始時刻 1:終了時刻 */
	long	input;
	long	*ptim;

	ptim = &CPrmSS[S_SYS][42];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);			/* "＜音量切替時刻＞　　　　　　　" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);				/* "開始時刻                        */
	bsytime_dsp(0, ptim[0], 0);					/* 時刻 正転                        */
	grachr(3, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);				/* "終了時刻"                       */
	bsytime_dsp(1, ptim[1], 0);					/* 時刻 正転                        */
	Fun_Dsp( FUNMSG2[0] );						/* "　　　　　　 変更　　　　終了 " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了(F5) */
			BUZPI();
			if (changing == 0)
				return MOD_EXT;
			bsytime_dsp(pos, ptim[pos], 0);		/* 時刻 正転 */
			Fun_Dsp( FUNMSG2[0] );				/* "　　　　　　 変更　　　　終了 " */
			changing = 0;
			break;
		case KEY_TEN_F3:	/* 変更(F3) */
			if (changing == 1)
				break;
			BUZPI();
			bsytime_dsp(0, ptim[0], 1);		/* 時刻 反転 */
			Fun_Dsp( FUNMSG[20] );			/* "　▲　　▼　　　　 書込  終了 " */
			input = -1;
			changing = 1;
			pos = 0;
			break;
		case KEY_TEN_F4:	/* 書込(F4) */
			if (changing == 0)
				break;
			if (input != -1) {
				if (input%100 > 59 || input > 2359) {
					BUZPIPI();
					bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
					input = -1;
					break;
				}
				ptim[pos] = input;
				OpelogNo = OPLOG_VLSWTIME;			// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;			// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			}
		case KEY_TEN_F1:	/* ▲(F1) */
		case KEY_TEN_F2:	/* ▼(F2) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 0);		/* 時刻 正転 */
			pos ^= 1;
			bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
			input = -1;
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			if (input == -1)
				input = 0;
			input = (input*10 + msg-KEY_TEN0) % 10000;
			bsytime_dsp(pos, input, 1);			/* 時刻 反転 */
			break;
		case KEY_TEN_CL:	/* 取消(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 1);		/* 時刻 反転 */
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：状態確認                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_StatusView( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const unsigned char	Det_Sts[][7] = {		// 「DET状態」表示ﾃﾞｰﾀ
	"ＯＮ　",	// [00]
	"ＯＦＦ",	// [01]
};

const unsigned char	Ari_Nasi[][5] = {		// 「エラー」「アラーム」有無表示ﾃﾞｰﾀ
	"あり",		// [00]
	"なし",		// [01]
};

const unsigned char	Eigyou_Mode1[][5] = {	// 「営休業」表示ﾃﾞｰﾀ1
	"営業",	// [00]
	"休業",	// [01]
};

const unsigned char	Eigyou_Mode2[][13] = {	// 「営休業」表示ﾃﾞｰﾀ2
	"（自動）　　",	// [00]
	"（強制）　　",	// [01]
	"（通信）　　",	// [02]
	"（信号）　　",	// [03]
	"（トラブル）",	// [04]
	"（金庫満杯）",	// [05]
	"（釣切れ）　",	// [06]
	"（メモリ）　",	// [07]
	"（手動）　　",	// [08]
// MH810105(S) MH364301 インボイス対応
	"（紙切れ）　",	// [09]
// MH810105(E) MH364301 インボイス対応
};

const unsigned char	Mankusha_Sts1[][5] = {	// 「満空車」表示ﾃﾞｰﾀ1
	"満車",	// [00]
	"空車",	// [01]
};

const unsigned char	Mankusha_Sts2[][9] = {	// 「満空車」表示ﾃﾞｰﾀ2
	"（自動）",	// [00]
	"（強制）",	// [01]
	"（通信）",	// [02]
};

unsigned short	UsMnt_StatusView(void)
{
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//	long i;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
	ushort	msg;
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//	char factor;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)	
	dispclr();												// 画面クリア

	// １行目："＜状態確認＞　　　ｈｈ：ｍｍ　"
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//	grachr( 0, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"＜状態確認＞");			// "＜状態確認＞"	
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:290)対応
//	grachr( 0, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"状態確認　");			// "　状態確認＞"
	grachr( 0, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"状態確認　");			// "　状態確認＞"
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:290)対応
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)	

	// ２行目："　エラー　　：ＸＸ　　　　　"
	grachr( 1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"エラー　　：");
	if( Err_onf == 0 ){																		// エラー発生有無表示
		grachr( 1, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[1] );					// なし表示
	}
	else{
		grachr( 1, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[0] );					// あり表示
	}
	// ３行目："　アラーム　　：ＸＸ　　　　　"
	grachr( 2, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"アラーム　：");
	if( Alm_onf == 0 ){																		// アラーム発生有無表示
		grachr( 2, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[1] );					// なし表示
	}
	else{
		grachr( 2, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[0] );					// あり表示
	}

	// ４行目："　営休業　　　：ＸＸＸＸ　　　"
	grachr( 3, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"営休業　　：");
	// 営業or休業
	if (opncls() == 1) {
		// 営業中
		grachr(3, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode1[0]);	// 営業
		// 営業理由
		if (OPECTL.opncls_eigyo == 1) {
			// 外部信号入力の営業信号あり
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[3]);	// （信号）
		} else if (PPrmSS[S_P01][2] == 1) {
			// ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽによる強制営業
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[1]);	// （強制）
		} else if (OPEN_stat == OPEN_NTNET) {
			// NT-NET制御ﾃﾞｰﾀによる営業指示
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[2]);	// （通信）
		} else {
			// 営業時間中
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[0]);	// （自動）
		}
	} else {
		// 休業中
		grachr(3, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode1[1]);	// 休業
		// 休業理由
		switch (CLOSE_stat) {	// opncls関数をｺｰﾙすることでCLOSE_statが更新される
		case 1:			// ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽによる強制休業
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[1]);	// （強制）
			break;
		case 2:			// 営業時間外
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[0]);	// （自動）
			break;
		case 3:			// 紙幣金庫満杯
		case 4:			// ｺｲﾝ金庫満杯
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[5]);	// （金庫満杯）
			break;
		case 6:			// 釣銭切れ
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[6]);	// （釣切れ）
			break;
		case 7:			// ﾄﾗﾌﾞﾙ
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[4]);	// （トラブル）
			break;
		case 8:			// NT-NET 送信ﾊﾞｯﾌｧFULL
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[7]);	// （メモリ）
			break;
		case 9:			// ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞ
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[8]);	// （手動）
			break;
		case 10:		// 外部信号入力の休業信号
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[3]);	// （信号）
			break;
		case 11:		// NT-NET制御ﾃﾞｰﾀによる休業指示
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[2]);	// （通信）
			break;
// MH810105(S) MH364301 インボイス対応
		case 15:		// ｲﾝﾎﾞｲｽ設定時のﾌﾟﾘﾝﾀｴﾗｰ
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[9]);	// （紙切れ）
			break;
// MH810105(E) MH364301 インボイス対応
		default:		// その他
			break;
		}
	}

// MH810100(S) K.Onodera 2019/10/22 車番チケットレス(メンテナンス)
//	// ５～７行目："　満空車　　：ＸＸＸＸ　　　"
//	//			   "　満空車１　：ＸＸＸＸ　　　"	満空車１～３まで
//		// 駐車1、2、3
//		grachr( 4, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"満空車１　：");
//		grachr( 5, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"満空車２　：");
//		grachr( 6, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"満空車３　：");
//
//	// ﾃﾞｰﾀ表示
//	for (i = 0; i < 3; i++) {
//		// 状態取得
//		factor = getFullFactor((uchar)i);
//		// 満車/空車表示
//		if (factor & 0x10) {
//			grachr( (ushort)(4+i), 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts1[0]);	// 満車
//		} else {
//			grachr( (ushort)(4+i), 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts1[1]);	// 空車
//		}
//		// 理由表示
//		if (factor & 0x01) {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[1]);	// （強制）
//		} else if (factor & 0x02) {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[2]);	// （通信）
//		} else {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[0]);	// （自動）
//		}
//	}
// MH810100(E) K.Onodera 2019/10/22 車番チケットレス(メンテナンス)

	// ファンクションキー表示
	Fun_Dsp( FUNMSG[8] );				// "　　　　　　　　　　　　 終了 "

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		
		else if (msg == KEY_TEN_F5) {
			BUZPI();
			return MOD_EXT;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：ロック装置閉タイマー拡張                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LockTimerEx( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	KIND_CHK( a,b )	(a&(b==2?0x02:0x01))
unsigned short	UsMnt_LockTimerEx(void)
{
	ushort	msg;
	long	*ptime[2];
	char	set;
	long	data;
	long	tmp;

	unsigned short	line = 0;	//1：フラップ選択中、2：駐輪ロック選択中
	char	syu[2];
	uchar	t_type = 0;

	memset( syu, 0, sizeof( syu ));
	if( !Get_Pram_Syubet(FLAP_UP_TIMER) ){
		ptime[0] = &CPrmSS[S_TYP][118];		// フラップ上昇タイマー
	}else{
		ptime[0] = &CPrmSS[S_LTM][31];		// フラップ上昇タイマー(Ａ種)
		t_type = 0x01;
		syu[0] = 1;
	}

	if( !Get_Pram_Syubet(ROCK_CLOSE_TIMER) ){
		ptime[1] = &CPrmSS[S_TYP][69];		// ロック閉タイマー
	}else{
		ptime[1] = &CPrmSS[S_LTM][11];		// ロック閉タイマー(Ａ種)
		t_type |= 0x02;
		syu[1] = 1;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[24]);					/* "＜ロック装置閉タイマー＞　　　" */
	grachr(3,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[27]);					/* "フラップ　" or "駐輪ロック" */
	grachr(3, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "：" */
	if( t_type & 0x01 ){
		grachr(3, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);			/* 種別 */
	}
	opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
	grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "分" */
	opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
	grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "秒" */

	grachr(4,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[28]);					/* "フラップ　" or "駐輪ロック" */
	grachr(4, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "：" */
	if( t_type & 0x02 ){
		grachr(4, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);				/* 種別 */
	}
	opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
	grachr(4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "分" */
	opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
	grachr(4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "秒" */

	Fun_Dsp(FUNMSG2[0]);						/* "　　　　　　 変更　　　　終了 " */

	set = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (set == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* 変更(F3) */
				BUZPI();
				if( t_type & 0x01 ){
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);					/* 種別 反転*/
					Fun_Dsp(FUNMSG[20]);											/* "　▲　　▼　　　　 書込  終了 " */
					set = KIND;
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* 分 反転 */
					Fun_Dsp(FUNMSG[20]);											/* "　▲　　▼　　　　 書込  終了 " */
					set = MIN;
				}
				line = 1;
				data = -1;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	// ▲ F1
			case KEY_TEN_F2:	// ▼ F2
				if( set == KIND ){
					BUZPI();
					if( KEY_TEN0to9(msg) == KEY_TEN_F1 ){
						syu[line-1]--;
						ptime[line-1]--;
						if( syu[line-1] < 1 ){
							syu[line-1] = 12;
							ptime[line-1] += 12;
						}
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);				/* Ａ種 */
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
						opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 正転 */
					}else{
						syu[line-1]++;
						ptime[line-1]++;
						if( syu[line-1] > 12 ){
							syu[line-1] = 1;
							ptime[line-1] -= 12;
						}
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);				/* Ａ種 */
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
						opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 正転 */
					}
				}else{
				// インデントをあわせる為にタブを追加
					BUZPI();
					if( t_type & 0x01 ){
						grachr(3, 12, 2, (line==1?0:1), COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);				/* 種別 */
						opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 上段 分 正転 */
					}else{
						opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, (line==1?0:1), COLOR_BLACK, LCD_BLINK_OFF);	/* 上段 分 正転 */
					}
					opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 上段 秒 正転 */
					
					if( t_type & 0x02 ){
						grachr(4, 12, 2, (line==2?0:1), COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);				/* 種別 */
						opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* 下段 分 反転 */
					}else{
						opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, (line==2?0:1), COLOR_BLACK, LCD_BLINK_OFF);	/* 下段 分 反転 */
					}
					opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* 下段 秒 正転 */
					line = (line == 1?2:1);
					data = -1;		//書込されていない入力値はクリア
					set = (KIND_CHK(t_type,line) ? KIND:MIN);
				}
				break;
			case KEY_TEN_F5:
				BUZPI();
				if( t_type & 0x01 ){
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);	/* 種別 */
				}
				if( t_type & 0x02 ){
					grachr(4, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);	/* 種別 */
				}
				opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
				opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */
				opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
				opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 */

				Fun_Dsp(FUNMSG2[0]);				/* "　　　　　　 変更　　　　終了 " */
				set = IDLE;
				break;
			case KEY_TEN_F4:
				if( set == KIND ){
					grachr((ushort)(line+2), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);		/* 種別 正転 */
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
					set = MIN;
					data = -1;
				}else if( set == MIN ){
					if (data != -1) {
						tmp = *ptime[line-1]/100000L*100000L + data*100 + *ptime[line-1]%100;
						if ((tmp%100000L) > 72000) {
							BUZPIPI();
							opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
							data = -1;
							break;
						}
						*ptime[line-1] = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 正転 */
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
					data = -1;
					set = SEC;
				}
				else {
					if (data != -1) {
						tmp = *ptime[line-1]/100*100 + data;
						if ((tmp%100000L) > 72000 || data > 59) {
							BUZPIPI();
							opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
							data = -1;
							break;
						}
						*ptime[line-1] = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// 操作履歴登録
						f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
					}
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 正転 */
					line = (line == 1?2:1);
					if( KIND_CHK(t_type,line) ){
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);							/* 種別 反転*/
					}else{
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* 分 反転 */
					}
					data = -1;
					set = (KIND_CHK(t_type,line) ? KIND:MIN);
				}
				BUZPI();
				break;
			case KEY_TEN:
				if (data == -1)
					data = 0;
				if (set == MIN) {
					data = (data*10 + msg-KEY_TEN0) % 1000;
					opedsp((ushort)(line+2), 16, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
				}
				else if(set == SEC){
					data = (data*10 + msg-KEY_TEN0) % 100;
					opedsp((ushort)(line+2), 24, (ushort)data, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
				}else{	// set == KIND
					break;
				}
				BUZPI();
				break;
			case KEY_TEN_CL:
				if (set == MIN){
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 反転 */
				}else if(set == SEC){
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* 秒 反転 */
				}else{// set == KIND
					break;
				}
				BUZPI();
				data = -1;
				break;
			default:
				break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：利用明細ログ印字画面への中継処理
//[]----------------------------------------------------------------------[]
///	@param[in]		MsgId			MNT_EDY_MEISAI : Ｅｄｙ利用明細
///									MNT_SCA_MEISAI : Ｓｕｉｃａ利用明細
///									MNT_EC_MEISAI  : 決済リーダ利用明細
///	@return			usSysEvent		MOD_CHG : mode change<br>
/// 								MOD_EXT : F5 key
///	@author			Y.Ise
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_DiditalCasheUseLog(ushort MsgId)
{
	ushort	usSysEvent;
	ushort	PrintNum;										// 印刷要求時のメッセージ番号
	ushort	LogKind;										// 使用するログ種別
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	uchar	titleNum = 110;
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける

	switch(MsgId)											// カード種別毎に表示タイトルとメッセージ番号を決定
	{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	case MNT_EDY_MEISAI:									// Ｅｄｙ
//		LogKind = LOG_EDYMEISAI;							// ログ種別
//		PrintNum = PREQ_EDY_USE_LOG;						// 印刷要求時のメッセージ番号
//		break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case MNT_SCA_MEISAI:									// Ｓｕｉｃａ
	default:												// カード種別なし(異常)
		LogKind = LOG_SCAMEISAI;							// ログ種別
		PrintNum = PREQ_SCA_USE_LOG;						// 印刷要求時のメッセージ番号
		break;

// MH321800(S) D.Inaba ICクレジット対応 (決済リーダ用の明細印字設定)
	case MNT_ECR_MEISAI:									// 決済リーダ処理・明細
		LogKind = LOG_ECMEISAI;								// ログ種別
		PrintNum = PREQ_EC_USE_LOG;							// 印刷要求時のメッセージ番号
		break;
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダ用の明細印字設定)
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	case MNT_ECR_MINASHI:									// みなし決済処理・明細
		LogKind = LOG_ECMINASHI;							// ログ種別
		PrintNum = PREQ_EC_MINASHI_LOG;						// 印刷要求時のメッセージ番号
		titleNum = 186;
		break;
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
	}

	dispclr();
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[110]);						// ログ情報プリントタイトル表示
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[titleNum]);					// ログ情報プリントタイトル表示
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
	grachr(2, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[113]);				/* "　　　 ログの検索中です 　　　" */
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[156]);				/* "最大約３０秒かかる事があります" */
	grachr(5, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_ON, UMSTR3[114]);				/* "　　 しばらくお待ち下さい 　　" */

	//	ログ件数表示画面処理４（対象期間指定あり／第一引数で検索処理をする）
	usSysEvent = SysMnt_Log_CntDsp3	(
										LogKind,			// ログ種別
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//										UMSTR3[110],		// タイトル表示データポインタ
										UMSTR3[titleNum],	// タイトル表示データポインタ
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
										PrintNum			// 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
									);
	return usSysEvent;
}

//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：日毎集計ログ印字画面への中継関数
//[]----------------------------------------------------------------------[]
///	@param[in]		MsgId			MNT_EDY_MEISAI : Ｅｄｙ利用明細
///									MNT_SCA_MEISAI : Ｓｕｉｃａ利用明細
///									MNT_EC_SHUUKEI : 決済リーダ日毎集計
///	@return			usSysEvent		MOD_CHG : mode change<br>
/// 								MOD_EXT : F5 key
///	@author			Y.Ise
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_DiditalCasheSyuukei(ushort MsgId)
{
extern	ushort	DiditalCashe_Log_CntDsp( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate );
	ushort	usSysEvent;
	ushort	NewOldDate[6];									// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
	ushort	LogCount;										// LOG登録件数
	ushort	PrintNum;										// 印刷要求時のメッセージ番号
	ushort	LogKind;										// 使用するログ種別
	date_time_rec	NewestDateTime, OldestDateTime;

	switch(MsgId)											// カード種別毎に表示タイトルとメッセージ番号を決定
	{
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	case MNT_EDY_SHUUKEI:									// Ｅｄｙ
//		LogKind = LOG_EDYSYUUKEI;							// ログ種別
//		PrintNum = PREQ_EDY_SYU_LOG;						// 印刷要求時のメッセージ番号
//		break;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case MNT_SCA_SHUUKEI:									// Ｓｕｉｃａ
	default:												// カード種別なし(異常)
		LogKind = LOG_SCASYUUKEI;							// ログ種別
		PrintNum = PREQ_SCA_SYU_LOG;						// 印刷要求時のメッセージ番号
		break;

// MH321800(S) D.Inaba ICクレジット対応 (決済リーダ用の集計印字設定)
	case MNT_ECR_SHUUKEI:									// 決済リーダ処理・集計
		LogKind = LOG_ECSYUUKEI;							// ログ種別
		PrintNum = PREQ_EC_SYU_LOG;							// 印刷要求時のメッセージ番号
		break;
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダ用の集計印字設定)

	}

	DiditalCashe_NewestOldestDateGet( LogKind, &NewestDateTime, &OldestDateTime, &LogCount );
//	// 最古＆最新日付ﾃﾞｰﾀ取得(両者の年月日を結合して第三引数に格納)
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
	usSysEvent = DiditalCashe_Log_CntDsp(
										LogKind,			// ログ種別
										LogCount,			// ログ件数
										UMSTR3[111],		// タイトル表示データポインタ
										PrintNum,			// 印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
										&NewOldDate[0]		// 最古＆最新ﾃﾞｰﾀ日付ﾃﾞｰﾀﾎﾟｲﾝﾀ
									);
	return usSysEvent;
}

//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：共通パラメータからメニューを作成(31byteテーブル用)
//[]----------------------------------------------------------------------[]
///	@param[in]		const CeMenuPara		*prm_tbl		: メニューの表示条件テーブル
///					const unsigned char		 str_tbl[][31]	: メニューの文字列テーブル(31バイト)
///					const unsigned short	 ctr_tbl[][4]	: メニューの制御情報テーブル
///					const unsigned char		 tbl_cnt		: 扱うメニューの総数
///	@return			unsigned char			ret				: 作成したメニュー項目数<br>
///	@author			Y.Ise
///	@note			共通パラメータから条件と一致した項目のみでメニューを作成する。
///					無条件に表示する項目は第一引数のSection,Address,Assignメンバを0にする。
///	@attention		メニューの作成最大数は10件(USM_MAKE_MENUデファイン依存)まで。
///					本関数を使用するにあたり、メニューの文字列は全角15文字(31byte)固定であること。
///					メニューの文字列は、複写元先頭から2byteを削除し、そこに番号を付加する。
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static unsigned char UsMnt_DiditalCasheMenuMake31(
	const CeMenuPara		*prm_tbl,											/* メニューの表示条件テーブル */
	const unsigned char		 str_tbl[][31],										/* メニューの文字列テーブル */
	const unsigned short	 ctr_tbl[][4],										/* メニューの制御情報テーブル */
	const unsigned char		 tbl_cnt											/* 扱うメニューの総数 */
){
	unsigned char cnt;															/* ループ用変数 */
	unsigned char ret;															/* 作成項目数 */

	memset(UsMnt_DiditalCasheMENU_STR, 0, sizeof(UsMnt_DiditalCasheMENU_STR));	// メニュー項目作成テーブル初期化
	memset(UsMnt_DiditalCasheMENU_CTL, 0, sizeof(UsMnt_DiditalCasheMENU_CTL));	// メニュー制御作成テーブル初期化

	for(cnt = 0, ret = 0; (cnt < tbl_cnt) && (cnt < USM_MAKE_MENU); cnt++)		// メニュー項目数分処理す
	{
		//メニューの項目文字列を作る
		strcpy((char*)&UsMnt_DiditalCasheMENU_STR[cnt][0], (char*)&MENU_NUMBER[ret][0]);
		strcat((char*)&UsMnt_DiditalCasheMENU_STR[cnt][2], (char*)&str_tbl[cnt][2]);
		/* 条件が設定されていない(無条件表示) */
		if((!prm_tbl[cnt].Section) && (!prm_tbl[cnt].Address) && (!prm_tbl[cnt].Assign))
		{
			//メニューの制御テーブルを作る
			memcpy(UsMnt_DiditalCasheMENU_CTL[ret], ctr_tbl[cnt], sizeof(UsMnt_DiditalCasheMENU_CTL[0]));
			ret++;																// メニュー項目数更新
		}
		/* 指定の共通パラメータアドレスが比較値と同じ */
		else if(prm_get(COM_PRM, prm_tbl[cnt].Section, prm_tbl[cnt].Address, 1, prm_tbl[cnt].Assign) == prm_tbl[cnt].Value)
		{
			//メニューの制御テーブルを作る
			memcpy(UsMnt_DiditalCasheMENU_CTL[ret], ctr_tbl[cnt], sizeof(UsMnt_DiditalCasheMENU_CTL[0]));
			ret++;																// メニュー項目数更新
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：拡張機能 - Suica処理ﾒﾆｭｰ
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			Suzuki
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006-11-06
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_SuicaMnu(void)
{
	unsigned char	MenuValue = 0;					// メニュー項目の表示数
	unsigned short	usUcreEvent;
	char	wk[2];
	CeMenuPara make_table[] =						// メニュー表示条件テーブル
	{
		{S_SCA,		7,			5,		1},			// 日毎集計プリント
		{S_SCA,		7,			6,		1},			// 利用明細プリント
	};

	DP_CP[0] = DP_CP[1] = 0;
	/* どの印字機能が有効であるかを共通パラメータから取得し、表示可否の判断をする */
	MenuValue = UsMnt_DiditalCasheMenuMake31(make_table, EXTENDMENU_SCA, USM_EXTSCA_TBL, USM_EXTSCA_MAX);

	for( ; ; ) {

		dispclr();									// 画面クリア
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[112]);			// "＜電子マネー処理＞　　　　　　"
		usUcreEvent = Menu_Slt( (void*)UsMnt_DiditalCasheMENU_STR, (void*)UsMnt_DiditalCasheMENU_CTL, (char)MenuValue, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUcreEvent ){
			case MNT_SCA_MEISAI:					// Ｓｕｉｃａ利用明細
				usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_SCA_MEISAI);
				if (usUcreEvent == MOD_CHG){
					return(MOD_CHG);
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if (usUcreEvent == MOD_CUT ){	
					return(MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				break;

			case MNT_SCA_SHUUKEI:					// Ｓｕｉｃａ集計
				usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_SCA_SHUUKEI);
				if (usUcreEvent == MOD_CHG){
					return(MOD_CHG);
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if (usUcreEvent == MOD_CUT ){
					return(MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				break;

			case MOD_EXT:		// 終了（Ｆ５）
				return(MOD_EXT);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case MOD_CUT:
				return(MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case MOD_CHG:		// モードチェンジ
				return(MOD_CHG);

			default:
				break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ユーザーメンテナンス：不正券ログ                					   |*
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : UsMnt_NgLog											   |*
 *| PARAMETER    : none													   |*
 *| RETURN VALUE : MOD_CHG : mode change								   |*
 *|              : MOD_EXT : F5 key										   |*
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 														   |*
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	UsMnt_NgLog(void)
{
	date_time_rec	NewTime;
	date_time_rec	OldTime;
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
	uchar			end_flg;			// クリア操作画面終了ﾌﾗｸﾞ
	ushort			Date_Old[6];		// 日付ﾃﾞｰﾀ（開始年[0],月[1],日[2],時[3],分[4]、終了[5],月[6],日[7],時[8],分[9]）
	ushort LogSyu;
	ushort LogCnt_total;				// 総ログ件数
	ushort LogCnt;						// 指定期間内対象ログ件数
	ushort PreqCmd;

	LogSyu = LOG_NGLOG;

	Ope2_Log_NewestOldestDateGet(eLOG_NGLOG, &NewTime, &OldTime, &LogCnt_total);
	PreqCmd = PREQ_NG_LOG;


	/* 初期画面表示 */
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[126]);		// "＜使用カード情報プリント＞    "
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );		// 最古／最新ログ日付表示
	Date_Now[0] = (ushort)OldTime.Year;	// 開始（年）←最古ログ（年）
	Date_Now[1] = (ushort)OldTime.Mon;	// 開始（月）←最古ログ（月）
	Date_Now[2] = (ushort)OldTime.Day;	// 開始（日）←最古ログ（日）
	
	Date_Now[3] = (ushort)NewTime.Year;	// 開始（年）←最新ログ（年）
	Date_Now[4] = (ushort)NewTime.Mon;	// 開始（月）←最新ログ（月）
	Date_Now[5] = (ushort)NewTime.Day;	// 開始（日）←最新ログ（日）
	memcpy(Date_Old, Date_Now, sizeof(Date_Old));		// 変更前日付ﾃﾞｰﾀセーブ
	LogDateDsp4( &Date_Now[0] );										// 最古／最新ログ日付データ表示
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );		// 印刷日付表示
	LogDateDsp5( &Date_Now[0], pos );									// 印刷日付データ表示
	
	// 指定期間内ログ件数取得
	Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
	Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// 終了日付ﾃﾞｰﾀ取得
	LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
	LogCntDsp( LogCnt );							// ログ件数表示
	
	Fun_Dsp( FUNMSG[83] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "

	/* 画面操作受け付け */
	for( ; ; ){
		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち

		/*  印字要求前（印字要求前画面）*/
		if( pri_cmd == 0 ){
			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？

				/* 設定キー切替 */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:
					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				/* ０～９ */
				case KEY_TEN:
					if( All_Req == OFF ){					// 「全て」指定中でない場合（指定中は、無視する）
						BUZPI();
						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}else{
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

				/* 取消キー */
				case KEY_TEN_CL:
					BUZPI();
					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){	// 「全て」指定中でない場合
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
					break;

				/* Ｆ１（←）Ｆ２（→）キー押下 */
				case KEY_TEN_F1:
				case KEY_TEN_F2:
					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){	// 「全て」指定中でない場合は（　⊂　　⊃　キー：日付入力）
						if( inp == -1 ){		// 入力なしの場合
							BUZPI();
							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
							}else{
								LogDatePosUp( &pos, 1 );		// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
							}
						}
						else{				//	入力ありの場合
							if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ
								BUZPI();
								if( pos == 0 ){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
								}else{
									LogDatePosUp( &pos, 1 );	// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
								}
							}
							else{								//	入力ﾃﾞｰﾀＮＧ
								BUZPIPI();
							}
						}
						// 指定期間内対象ログ件数を取得し表示する
						LogDateDsp5( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
						inp = -1;								// 入力状態初期化
					}
					else if( RcvMsg == KEY_TEN_F2 ){		// 「全て」指定中場合は（Ｆ２：「クリア」キー）
						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "　　ログをクリアしますか？　　"
						Fun_Dsp(FUNMSG[19]);															// "　　　　　　 はい いいえ　　　"

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
										LogCnt_total = 0;				// ログ件数（入力ﾊﾟﾗﾒｰﾀ）クリア
										LogCntDsp( LogCnt_total );		// ログ件数表示
										Date_Now[0] = Date_Now[3] = (ushort)CLK_REC.year;	// 開始、終了（年）←現在日時（年）
										Date_Now[1] = Date_Now[4] = (ushort)CLK_REC.mont;	// 開始、終了（月）←現在日時（月）
										Date_Now[2] = Date_Now[5] = (ushort)CLK_REC.date;	// 開始、終了（日）←現在日時（日）
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
								LogDateDsp4( &Date_Now[0] );						// 最古／最新ログ日付データ表示
								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
								LogDateDsp5( &Date_Now[0], pos );					// 印刷日付データ表示
								Fun_Dsp( FUNMSG[83] );								// ﾌｧﾝｸｼｮﾝｷｰ表示："　←　　→　 全て  実行  終了 "
								All_Req = OFF;										// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
							}
						}
					}
					break;

				/* Ｆ３（全て）キー押下 */
				case KEY_TEN_F3:
					displclr( 1 );							// 1行目表示クリア
					if( All_Req == OFF ){	// 「全て」指定中？（指定中は、無視する）
						if( LogCnt_total != 0 ){		//	ログデータがある場合
							BUZPI();
							Date_Now[0] = (ushort)OldTime.Year;	// 開始（年）←最古ログ（年）
							Date_Now[1] = (ushort)OldTime.Mon;	// 開始（月）←最古ログ（月）
							Date_Now[2] = (ushort)OldTime.Day;	// 開始（日）←最古ログ（日）
							
							Date_Now[3] = (ushort)NewTime.Year;	// 開始（年）←最新ログ（年）
							Date_Now[4] = (ushort)NewTime.Mon;	// 開始（月）←最新ログ（月）
							Date_Now[5] = (ushort)NewTime.Day;	// 開始（日）←最新ログ（日）
							pos = 0;								// ｶｰｿﾙ位置＝開始（年）
							inp = -1;								// 入力状態初期化
							displclr( 5 );							// 印刷日付表示クリア
							// 指定期間内対象ログ件数を取得し表示する
							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
							Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// 終了日付ﾃﾞｰﾀ取得
							LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
							LogCntDsp( LogCnt );					// ログ件数表示
							Fun_Dsp( FUNMSG[85] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
							All_Req = ON;							// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
						}
						else{					//	ログデータがない場合
							BUZPIPI();
						}
					}
					break;

				/* Ｆ４（実行）キー押下 */
				case KEY_TEN_F4:
					displclr( 1 );							// 1行目表示クリア
					Date_Chk = OK;
					if( inp != -1 ){	//	入力ありの場合
						if( OK == LogDateChk( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫの場合
							if( pos == 0 ){					// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}else{								//	入力データＮＧの場合
							Date_Chk = NG;
						}
					}
					// 指定期間内対象ログ件数を取得し表示する
					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
					Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// 終了日付ﾃﾞｰﾀ取得
					LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
					if( LogCnt == 0 ){
						BUZPIPI();
						grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// データなし表示
						inp = -1;							// 入力状態初期化
						break;
					}
					if( Date_Chk == OK ){
						if( All_Req != ON ){	// 「全て」指定でない場合、年月日ﾁｪｯｸを行う
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
					if( Date_Chk == OK ){	// 開始日付＆終了日付ﾁｪｯｸＯＫの場合
						BUZPI();
						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
						FrmLogPriReq2.prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// 検索方法	：全て
						}else{
							FrmLogPriReq2.BType	= 1;							// 検索方法	：日付
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						FrmLogPriReq2.LogCount = LogCnt_total;	// LOG登録件数(個別精算LOG・集計LOGで使用)
						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
						Ope_DisableDoorKnobChime();
						pri_cmd = PreqCmd;					// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
						if( All_Req != ON ){					// 「全て」指定なし
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
							LogDateDsp5( &Date_Now[0], 0xff );	// 印刷日付データ表示
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// 実行中ブリンク表示
						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"

					}else{	// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
						BUZPIPI();
						LogDateDsp5( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
						inp = -1;								// 入力状態初期化
					}
					break;

				/* Ｆ５（終了）キー押下 */
				case KEY_TEN_F5:							// 
					BUZPI();
					if( All_Req == OFF ){	// 「全て」指定中でない場合
						RcvMsg = MOD_EXT;						// 前画面に戻る
					}else{					//	「全て」指定中の場合
						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
						inp = -1;							// 入力状態初期化
						opedsp	(							// ｶｰｿﾙを表示させる
									POS_DATA1_0[pos][0],	// 表示行
									POS_DATA1_0[pos][1],	// 表示ｶﾗﾑ
									Date_Now[pos],			// 表示ﾃﾞｰﾀ
									POS_DATA1_0[pos][2],	// ﾃﾞｰﾀ桁数
									POS_DATA1_0[pos][3],	// ０サプレス有無
									1,						// 反転表示：あり
									COLOR_BLACK, 
									LCD_BLINK_OFF
								);
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// 印刷日付表示
						LogDateDsp5( &Date_Now[0], pos );	// 印刷日付データ表示
						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "
						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
					}
					break;
				default:
					break;
			}
		}
		/* 印字要求後（印字終了待ち画面）*/
		else{
			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
				RcvMsg = MOD_EXT;							// YES：前画面に戻る
			}

			switch( RcvMsg ){								// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// 前画面に戻る
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:	// 設定キー切替
					RcvMsg = MOD_CHG;						// 前画面に戻る
					break;

				case KEY_TEN_F3:	// Ｆ３（中止）キー押下
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

ushort	UsMnt_NgLog_GetLogCnt(ushort Sdate, ushort Edate)
{
	ushort			log_date;			// ログ格納日付
	ushort			LOG_Date[3];		// ログ格納日付（[0]：年、[1]：月、[2]：日）
	ushort			log_cnt;			// 指定期間内ログ件数
	ushort			cnt;
	ushort			LogCnt_total;

	log_cnt = 0;
	LogCnt_total = Ope_Log_TotalCountGet(eLOG_NGLOG);
	
	for(cnt=0; cnt < LogCnt_total; cnt++) {
		if( 0 == Ope_Log_1DataGet(eLOG_NGLOG, cnt, &nglog_data)){		// 
			log_cnt = 0;
			break;
		}
		LOG_Date[0] = nglog_data.NowTime.Year;
		LOG_Date[1] = nglog_data.NowTime.Mon;
		LOG_Date[2] = nglog_data.NowTime.Day;
		log_date = dnrmlzm(	(short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]) ;
		if( (log_date >= Sdate) && (log_date <= Edate) ) {
			log_cnt++;
		}
	}
	return(log_cnt);
}
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
//static ulong nmlS;															/* 検索開始日時変換用 */
//static ulong nmlE;															/* 検索終了日時変換用 */
//ushort UsMnt_SearchIOLog(ushort offset, ushort no, ulong sdate, ulong edate, IoLog_Data *p)
//{
//	uchar BType;															/* 検索方法(0:無条件/1:指定日時/2:指定期間) */
//	ushort i, j;															/* ログテーブル検索用ループ変数 */
//	ulong date1;															/* 参照中のイベント発生日時変換用 */
//	ulong date2;															/* 参照中ひとつ先のイベント日時変換用 */
//	if(sdate && edate)														/* 検索開始・終了日時がセットされている */
//	{
//		BType = 2;															/* 検索方法を指定期間 */
//	}
//	else if(sdate)															/* 検索開始日時のみセットされている */
//	{
//		BType = 1;															/* 検索方法を指定日時 */
//	}
//	else																	/* 上記以外 */
//	{
//		BType = 0;															/* 検索方法を無条件 */
//	}
//	nmlS = sdate;															/* 検索開始日時情報を一時領域へ */
//	nmlE = edate;															/* 検索終了日時情報を一時領域へ */
//
//	memset(p, 0, sizeof(IoLog_Data));
//	for (i = offset; 0 != Ope_Log_1DataGet(eLOG_IOLOG, i, p); i++) {			// 引数の検索開始位置からログ領域末尾まで
//		/* 参照中ログが指定された車室番号と一致 または 車室番号指定無し 且つ ログ記録が完了している */
//		if ((p->room == no) || (no == 0xFFFF)) {
//			/* 参照中ログのイベントを最後まで検索 */
//			for (j = 0; (j < IO_EVE_CNT) && (p->iolog_data[j].Event_type != 0); j++) {
//				date1 = enc_nmlz_mdhm(								// 参照中イベント発生日時を変換
//					p->iolog_data[j].NowTime.Year,					// 年
//					p->iolog_data[j].NowTime.Mon,					// 月
//					p->iolog_data[j].NowTime.Day,					// 日
//					p->iolog_data[j].NowTime.Hour,					// 時
//					p->iolog_data[j].NowTime.Min					// 分
//					);
//				switch (BType) {									// 検索方法
//				case 0:												// 検索方法:無条件
//					return (ushort)(i + 1);							// 現在のログ参照位置(+1)を返す
//				case 1:												// 検索方法:指定日時
//					if(date1 == nmlS) {								// 指定日時とイベント発生日時が一致
//						return (ushort)(i + 1);						// 現在のログ参照位置(+1)を返す
//					} else if ((j< (IO_EVE_CNT-1)) && (p->iolog_data[j+1].Event_type != 0)) {	// 次の記録情報にイベントがある
//						date2 = enc_nmlz_mdhm(						// 次のイベント発生日時を変換
//							p->iolog_data[j+1].NowTime.Year,		// 年
//							p->iolog_data[j+1].NowTime.Mon,			// 月
//							p->iolog_data[j+1].NowTime.Day,			// 日
//							p->iolog_data[j+1].NowTime.Hour,		// 時
//							p->iolog_data[j+1].NowTime.Min			// 分
//							);
//						if ((date1 <= nmlS) && (nmlS <= date2)) {	// 現在が指定以降 且つ 次が指定以前
//							return (ushort)(i + 1);					// 現在のログ参照位置(+1)を返す
//						}
//					}
//					break;
//				case 2:												// 検索方法:指定期間
//					if((date1 >= nmlS) && (date1 <= nmlE)) {		// 発生が開始以降 且つ 発生が終了以前
//						return (ushort)(i + 1);						// 現在のログ参照位置(+1)を返す
//					} else if ((j< (IO_EVE_CNT-1)) && (p->iolog_data[j+1].Event_type != 0)) {	// 次の記録情報にイベントがある
//						date2 = enc_nmlz_mdhm(						// 次のイベント発生日時を変換
//							p->iolog_data[j+1].NowTime.Year,		// 年
//							p->iolog_data[j+1].NowTime.Mon,			// 月
//							p->iolog_data[j+1].NowTime.Day,			// 日
//							p->iolog_data[j+1].NowTime.Hour,		// 時
//							p->iolog_data[j+1].NowTime.Min			// 分
//							);
//						if ((date1 <= nmlS) && (nmlS <= date2)) {	// 現在が指定以降 且つ 次が指定以前
//							return (ushort)(i + 1);					// 現在のログ参照位置(+1)を返す
//						}
//					}
//					break;
//				}
//			}
//		}
//		memset(p, 0, sizeof(IoLog_Data));
//	}
//	memset(p, 0, sizeof(IoLog_Data));
//	return 0;
//}
//
//ushort	UsMnt_IoLog_GetLogCnt(ushort no, ushort* date)
//{
//	ushort offset;																					/* 条件が一致したログの位置 */
//	ushort log_cnt;																					/* 条件に一致したログの件数 */
//	ulong Sdate, Edate;																				/* 検索開始・終了日時変換用 */
//
//	Sdate = enc_nmlz_mdhm(date[0], (uchar)date[1], (uchar)date[2], (uchar)date[3], (uchar)date[4]);	/* 検索開始日時情報変換 */
//	Edate = enc_nmlz_mdhm(date[5], (uchar)date[6], (uchar)date[7], (uchar)date[8], (uchar)date[9]);	/* 検索終了日時情報変換 */
//	for(offset = 0, log_cnt = 0; 0 != (offset = UsMnt_SearchIOLog(offset, no, Sdate, Edate, &Io_log_wk)); log_cnt++);
//	return log_cnt;																					/* 条件一致ログ件数を返す */
//}
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
/*[]----------------------------------------------------------------------[]*
 *| ユーザーメンテナンス：入出庫ログ                					   |*
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : UsMnt_IoLog											   |*
 *| PARAMETER    : none													   |*
 *| RETURN VALUE : MOD_CHG : mode change								   |*
 *|              : MOD_EXT : F5 key										   |*
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 														   |*
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	UsMnt_IoLog(ushort no)
{
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
//	ushort			RcvMsg;				// 受信ﾒｯｾｰｼﾞ格納ﾜｰｸ
//	ushort			pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
//	T_FrmLogPriReq2	FrmLogPriReq2;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
//	T_FrmPrnStop	FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
//	ushort			Date_Now[10];		// 確定後日付ﾃﾞｰﾀ（開始年[0],月[1],日[2],時[3],分[4]、終了[5],月[6],日[7],時[8],分[9]）
//	ushort			Date_Old[10];		// 確定前日付ﾃﾞｰﾀ（開始年[0],月[1],日[2],時[3],分[4]、終了[5],月[6],日[7],時[8],分[9]）
//	ushort			NewOldDate[10];		// 最古＆最新日付ﾃﾞｰﾀ（開始年[0],月[1],日[2],時[3],分[4]、終了[5],月[6],日[7],時[8],分[9]）
//	uchar			flg;				// カーソル行移動有(1)無(0)
//	uchar			pos		= 0;		// 期間ﾃﾞｰﾀ入力位置（開始年(0),月(1),日(2)時(3)分(4)、終了年(5),月(6),日(7)時(8)分(9)）
//	short			inp		= -1;		// 入力ﾃﾞｰﾀ
//	uchar			All_Req	= OFF;		// 「全て」指定中ﾌﾗｸﾞ
//	uchar			Date_Chk;			// 日付指定ﾃﾞｰﾀﾁｪｯｸﾌﾗｸﾞ
//	ushort			Sdate, Edate;		// 開始日付、終了日付：日数(YYYY+MM+DD)
//	ushort			Stime, Etime;		// 開始時刻、終了時刻：分数(hh+mm)
//	union ioLogNumberInfo num;			// ログ管理番号取得用構造体
//
//	ushort LogCnt_total;				// 総ログ件数
//	ushort LogCnt;						// 指定期間内対象ログ件数
//	extern	uchar	LogDateChk_Range_Minute( uchar pos, short data );
//	memset(&Io_log_wk, 0, sizeof(Io_log_wk));
//	/* 初期画面表示 */
//	dispmlclr(1,7);																	// 画面クリア
//	Fun_Dsp(FUNMSG[77]);															// "　　　　　　　　　　　　　　　"
//	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);			// "　　しばらくお待ちください　　"
//	LogCnt = LogPtrGet2(no, &num.n);												// 最古・最新ログデータのポインター取得
//	displclr(4);
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[128]);					// "車室番号：　　　　　　　　　　"
//	opedsp6(1, 10, no, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);							// 車室番号表示
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[6]);					// "開始日付ＹＹＹＹ年ＭＭ月ＤＤ日"
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[8]);					// "開始日付　　　　　　００：００"
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[7]);					// "終了日付ＹＹＹＹ年ＭＭ月ＤＤ日"
//	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[8]);					// "終了日付　　　　　　００：００"
//	if(num.n) {
//		Ope_Log_1DataGet(eLOG_IOLOG, (ushort)num.log.sdat, &Io_log_wk);
//		Date_Now[0]	= Io_log_wk.iolog_data[0].NowTime.Year;				// 開始（年）←最古ログ（年）
//		Date_Now[1]	= Io_log_wk.iolog_data[0].NowTime.Mon;				// 開始（月）←最古ログ（月）
//		Date_Now[2]	= Io_log_wk.iolog_data[0].NowTime.Day;				// 開始（日）←最古ログ（日）
//		Date_Now[3]	= Io_log_wk.iolog_data[0].NowTime.Hour;				// 開始（時）←最古ログ（時）
//		Date_Now[4]	= Io_log_wk.iolog_data[0].NowTime.Min;				// 開始（分）←最古ログ（分）
//		
//		Ope_Log_1DataGet(eLOG_IOLOG, (ushort)num.log.edat, &Io_log_wk);
//		Date_Now[5]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Year;	// 終了（年）←最新ログ（年）
//		Date_Now[6]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Mon;		// 終了（月）←最新ログ（月）
//		Date_Now[7]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Day;		// 終了（日）←最新ログ（日）
//		Date_Now[8]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Hour;	// 終了（時）←最新ログ（時）
//		Date_Now[9]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Min;		// 終了（分）←最新ログ（分）
//	} else {
//		Date_Now[0] = (ushort)CLK_REC.year;				// 開始（年）←現在日時（年）
//		Date_Now[1] = (ushort)CLK_REC.mont;				// 開始（月）←現在日時（月）
//		Date_Now[2] = (ushort)CLK_REC.date;				// 開始（日）←現在日時（日）
//		Date_Now[3] = (ushort)CLK_REC.hour;				// 開始（時）←現在日時（時）
//		Date_Now[4] = (ushort)CLK_REC.minu;				// 開始（分）←現在日時（分）
//		Date_Now[5] = (ushort)CLK_REC.year;				// 終了（年）←現在日時（年）
//		Date_Now[6] = (ushort)CLK_REC.mont;				// 終了（月）←現在日時（月）
//		Date_Now[7] = (ushort)CLK_REC.date;				// 終了（日）←現在日時（日）
//		Date_Now[8] = (ushort)CLK_REC.hour;				// 終了（時）←現在日時（時）
//		Date_Now[9] = (ushort)CLK_REC.minu;				// 終了（分）←現在日時（分）
//	}
//	memcpy(Date_Old, Date_Now, sizeof(Date_Old));		// 変更前日付ﾃﾞｰﾀセーブ
//	memcpy(NewOldDate, Date_Now, sizeof(NewOldDate));	// 『全て』ボタン用最新・最古日付ﾃﾞｰﾀ
//	LogDateDsp3( &Date_Now[0], pos );					// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
//
//	LogCnt_total = LogCnt;								// ログ件数初期化
//	// ログ件数表示（対象件数／総件数）
//	LogCntDsp2( LogCnt, LogCnt_total, 3 );											// ログ件数表示（０件）
//	Fun_Dsp( FUNMSG[83] );															// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "
//
//	/* 画面操作受け付け */
//	for( ; ; ){
//		RcvMsg = StoF( GetMessage(), 1 );					// イベント待ち
//
//		/*  印字要求前（印字要求前画面）*/
//		if( pri_cmd == 0 ){
//			switch( KEY_TEN0to9( RcvMsg ) ){				// イベント？
//
//				/* 設定キー切替 */
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				case KEY_MODECHG:
//					RcvMsg = MOD_CHG;						// 前画面に戻る
//					break;
//
//				/* ０～９ */
//				case KEY_TEN:
//					if( All_Req == OFF ){					// 「全て」指定中でない場合（指定中は、無視する）
//						BUZPI();
//						if( inp == -1 ){
//							inp = (short)(RcvMsg - KEY_TEN0);
//						}else{
//							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
//						}
//						opedsp	(							// 入力ﾃﾞｰﾀ表示
//									POS_DATA31[pos][0],		// 表示行
//									POS_DATA31[pos][1],		// 表示ｶﾗﾑ
//									(ushort)inp,			// 表示ﾃﾞｰﾀ
//									POS_DATA31[pos][2],		// ﾃﾞｰﾀ桁数
//									POS_DATA31[pos][3],		// ０サプレス有無
//									1,						// 反転表示：あり
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//					}
//					break;
//
//				/* 取消キー */
//				case KEY_TEN_CL:
//					BUZPI();
//					if( All_Req == OFF ){	// 「全て」指定中でない場合
//						opedsp	(							// 入力前のﾃﾞｰﾀを表示させる
//									POS_DATA31[pos][0],		// 表示行
//									POS_DATA31[pos][1],		// 表示ｶﾗﾑ
//									Date_Now[pos],			// 表示ﾃﾞｰﾀ
//									POS_DATA31[pos][2],		// ﾃﾞｰﾀ桁数
//									POS_DATA31[pos][3],		// ０サプレス有無
//									1,						// 反転表示：あり
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//						inp = -1;							// 入力状態初期化
//					}
//					break;
//
//				/* Ｆ１（←）Ｆ２（→）キー押下 */
//				case KEY_TEN_F1:
//				case KEY_TEN_F2:
//					if( All_Req == OFF ){	// 「全て」指定中でない場合は（　⊂　　⊃　キー：日付入力）
//						if( inp == -1 ){		// 入力なしの場合
//							BUZPI();
//
//							if( RcvMsg == KEY_TEN_F1 ){
//								flg = LogDatePosUp3( &pos, 0 );		// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
//							}else{
//								flg = LogDatePosUp3( &pos, 1 );		// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
//							}
//						}
//						else{				//	入力ありの場合
//							if( OK == LogDateChk_Range_Minute( pos, inp ) ){	// 入力ﾃﾞｰﾀＯＫ
//								BUZPI();
//								if( pos == 0 || pos == 5 ){		// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
//									Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//								}else{
//									Date_Now[pos] = (ushort)inp;
//								}
//								if( RcvMsg == KEY_TEN_F1 ){
//									flg = LogDatePosUp3( &pos, 0 );	// 入力位置ﾃﾞｰﾀ更新（位置番号－１）
//								}else{
//									flg = LogDatePosUp3( &pos, 1 );	// 入力位置ﾃﾞｰﾀ更新（位置番号＋１）
//								}
//							}
//							else{								//	入力ﾃﾞｰﾀＮＧ
//								BUZPIPI();
//							}
//						}
//						// 指定期間内対象ログ件数を取得し表示する
//						if( (flg == 1) && (memcmp(Date_Old, Date_Now, sizeof(Date_Old)) != 0) ) {	// 行移動で変化があった場合のみ
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
//							Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// 終了日付ﾃﾞｰﾀ取得
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// 開始時刻ﾃﾞｰﾀ取得
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// 終了時刻ﾃﾞｰﾀ取得
//							LogCnt = 0;
//							if( Sdate < Edate ){
//								Ope_KeyRepeatEnable(0);			// キーリピート無効
//								LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//								Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							}else{
//								if( Sdate == Edate ){
//									if( Stime <= Etime ){
//										Ope_KeyRepeatEnable(0);			// キーリピート無効
//										LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//										Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//									}
//								}
//							}
//							LogCntDsp2( LogCnt, LogCnt_total, 4 );									// ログ件数再表示
//							memcpy(Date_Old, Date_Now, sizeof(Date_Old));							// 変更前日付ﾃﾞｰﾀセーブ
//						}
//
//						LogDateDsp3( &Date_Now[0], pos );		// ｶｰｿﾙ位置移動（入力ＮＧの場合、移動しない）
//						inp = -1;								// 入力状態初期化
//					}
//					break;
//
//				/* Ｆ３（全て）キー押下 */
//				case KEY_TEN_F3:
//					if( All_Req == OFF ){	// 「全て」指定中？（指定中は、無視する）
//						if( LogCnt_total != 0 ){		//	ログデータがある場合
//							BUZPI();
//							memcpy( &Date_Now[0], &NewOldDate[0], sizeof(Date_Now) );	// 最古＆最新ﾃﾞｰﾀ日付をｺﾋﾟｰ
//							pos = 0;								// ｶｰｿﾙ位置＝開始（年）
//							inp = -1;								// 入力状態初期化
//							LogDateDsp3( &Date_Now[0], 0xff );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示（ｶｰｿﾙ表示なし）
//							// 指定期間内対象ログ件数を取得し表示する
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
//							Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// 終了日付ﾃﾞｰﾀ取得
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// 開始時刻ﾃﾞｰﾀ取得
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// 終了時刻ﾃﾞｰﾀ取得
//							Ope_KeyRepeatEnable(0);			// キーリピート無効
//							LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							LogCntDsp2( LogCnt, LogCnt_total, 4 );			// ログ件数再表示
//							Fun_Dsp( FUNMSG[81] );					// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　クリア　　　 実行  終了 "
//							All_Req = ON;							// 「全て」指定中ﾌﾗｸﾞｾｯﾄ
//						}
//						else{					//	ログデータがない場合
//							BUZPIPI();
//						}
//					}
//					break;
//
//				/* Ｆ４（実行）キー押下 */
//				case KEY_TEN_F4:
//					Date_Chk = OK;
//					if( inp != -1 ){	//	入力ありの場合
//						if( OK == LogDateChk_Range_Minute( pos, inp ) ){	// 指定期間ＯＫ（開始<=終了年月日時分）の場合
//							if( pos == 0 || pos == 5 ){		// 入力ﾃﾞｰﾀｾｰﾌﾞ（年ﾃﾞｰﾀの場合、下２桁のみ書きかえる）
//								Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//							}else{
//								Date_Now[pos] = (ushort)inp;
//							}
//						}else{								//	入力データＮＧの場合
//							Date_Chk = NG;
//						}
//					}
//					// 指定期間内対象ログ件数を取得し表示する
//					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// 開始日付ﾃﾞｰﾀ取得
//					Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// 終了日付ﾃﾞｰﾀ取得
//					Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// 開始時刻ﾃﾞｰﾀ取得
//					Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// 終了時刻ﾃﾞｰﾀ取得
//					LogCnt = 0;
//					if( Sdate < Edate ){
//						Ope_KeyRepeatEnable(0);			// キーリピート無効
//						LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					}else{
//						if( Sdate == Edate ){
//							if( Stime <= Etime ){
//								Ope_KeyRepeatEnable(0);			// キーリピート無効
//								LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//								Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							}
//						}
//					}
//					LogCntDsp2( LogCnt, LogCnt_total, 4 );			// ログ件数再表示
//					if( Date_Chk == OK ){
//						if( All_Req != ON ){	// 「全て」指定でない場合、年月日ﾁｪｯｸを行う
//							//	年月日が存在する日付かチェックする
//							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// 開始日付ﾁｪｯｸ
//								Date_Chk = NG;
//							}
//							if( chkdate( (short)Date_Now[5], (short)Date_Now[6], (short)Date_Now[7] ) != 0 ){	// 終了日付ﾁｪｯｸ
//								Date_Chk = NG;
//							}
//						}
//					}
//					if( Date_Chk == OK ){
//						//	開始日付＜＝終了日付かチェックする
//						Sdate = dnrmlzm(							// 開始日付ﾃﾞｰﾀ取得
//											(short)Date_Now[0],
//											(short)Date_Now[1],
//											(short)Date_Now[2]
//										);
//						Edate = dnrmlzm(							// 終了日付ﾃﾞｰﾀ取得
//											(short)Date_Now[5],
//											(short)Date_Now[6],
//											(short)Date_Now[7]
//										);
//						if( Sdate > Edate ){
//							Date_Chk = NG;
//						}
//
//						/*-------------------------------------------------*/
//						// 	開始日付＝終了日付の時は、開始時刻＜＝終了時刻かチェックする
//						if( Sdate == Edate ){
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];		// 開始時刻ﾃﾞｰﾀ取得
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];		// 終了時刻ﾃﾞｰﾀ取得
//							if( Stime > Etime ){
//								Date_Chk = NG;
//							}
//						}
//						/*--------------------------------------------------*/
//
//					}
//					if( Date_Chk == OK ){	// 開始日付時刻＆終了日付時刻ﾁｪｯｸＯＫの場合
//						BUZPI();
//						/*------	印字要求ﾒｯｾｰｼﾞ送信	-----*/
//						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ０ｸﾘｱ
//						FrmLogPriReq2.prn_kind	= R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ｼﾞｬｰﾅﾙ
//						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
//						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// 係員No.
//						FrmLogPriReq2.Room_no	= no;							// 車室番号
//
//						if( All_Req == ON ){
//							FrmLogPriReq2.BType	= 0;							// 検索方法	：全て
//						}else if( (Sdate == Edate) && (Stime == Etime) ){
//							FrmLogPriReq2.BType	= 1;							// 検索方法	：日付
//							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
//							FrmLogPriReq2.TSttTime.Hour	= (uchar)Date_Now[3];
//							FrmLogPriReq2.TSttTime.Min	= (uchar)Date_Now[4];
//						}else{
//							FrmLogPriReq2.BType	= 2;							// 検索方法	：期間
//							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
//							FrmLogPriReq2.TSttTime.Hour	= (uchar)Date_Now[3];
//							FrmLogPriReq2.TSttTime.Min	= (uchar)Date_Now[4];
//							FrmLogPriReq2.TEndTime.Year	= Date_Now[5];
//							FrmLogPriReq2.TEndTime.Mon	= (uchar)Date_Now[6];
//							FrmLogPriReq2.TEndTime.Day	= (uchar)Date_Now[7];
//							FrmLogPriReq2.TEndTime.Hour	= (uchar)Date_Now[8];
//							FrmLogPriReq2.TEndTime.Min	= (uchar)Date_Now[9];
//						}
//						FrmLogPriReq2.LogCount = LogCnt_total;	// LOG登録件数(個別精算LOG・集計LOGで使用)
//						queset( PRNTCBNO, PREQ_IO_LOG, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
//						pri_cmd = PREQ_IO_LOG;					// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
//						LogDateDsp3( &Date_Now[0], 0xff );	// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示（ｶｰｿﾙ表示を消す）
//						Fun_Dsp( FUNMSG[82] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"
//
//					}else{	// 開始日付＆終了日付ﾁｪｯｸＮＧの場合
//						BUZPIPI();
//						LogDateDsp3( &Date_Now[0], pos );		// 開始日付ﾃﾞｰﾀ＆終了日付ﾃﾞｰﾀ　表示
//						inp = -1;								// 入力状態初期化
//					}
//					break;
//
//				/* Ｆ５（終了）キー押下 */
//				case KEY_TEN_F5:							// 
//					BUZPI();
//
//					if( All_Req == OFF ){	// 「全て」指定中でない場合
//						RcvMsg = MOD_EXT;							// YES：前画面に戻る
//					}else{					//	「全て」指定中の場合
//						pos = 0;							// ｶｰｿﾙ位置＝開始（年）
//						inp = -1;							// 入力状態初期化
//						opedsp	(							// ｶｰｿﾙを表示させる
//									POS_DATA31[pos][0],		// 表示行
//									POS_DATA31[pos][1],		// 表示ｶﾗﾑ
//									Date_Now[pos],			// 表示ﾃﾞｰﾀ
//									POS_DATA31[pos][2],		// ﾃﾞｰﾀ桁数
//									POS_DATA31[pos][3],		// ０サプレス有無
//									1,						// 反転表示：あり
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//						Fun_Dsp( FUNMSG[83] );				// ﾌｧﾝｸｼｮﾝｷｰ表示："　⊂　　⊃　 全て  実行  終了 "
//						All_Req = OFF;						// 「全て」指定中ﾌﾗｸﾞﾘｾｯﾄ
//					}
//					break;
//				default:
//					break;
//			}
//		}
//		/* 印字要求後（印字終了待ち画面）*/
//		else{
//			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// 印字終了ﾒｯｾｰｼﾞ受信？
//				RcvMsg = MOD_EXT;							// YES：前画面に戻る
//			}
//
//			switch( RcvMsg ){								// イベント？
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				case KEY_MODECHG:	// 設定キー切替
//					RcvMsg = MOD_CHG;						// 前画面に戻る
//					break;
//
//				case KEY_TEN_F3:	// Ｆ３（中止）キー押下
//					BUZPI();
//					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
//					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
//					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
//					RcvMsg = MOD_EXT;						// 前画面に戻る
//					break;
//
//				default:
//					break;
//			}
//		}
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
////		if(RcvMsg == MOD_CHG){								// モードチェンジ
//		if(RcvMsg == MOD_CHG || RcvMsg == LCD_DISCONNECT){								// モードチェンジ
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			RcvMsg = KEY_MODECHG;							// モードチェンジのデータ形式を変更
//			break;											// 前画面に戻る
//		}else if(RcvMsg == MOD_EXT){						// 終了ボタン
//			break;											// 前画面に戻る
//		}
//	}
//	return( RcvMsg );
	return( MOD_EXT );
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(QR確定・取消データ対応)
}
/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：履歴情報プリント                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Logprintmenu( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	UsMnt_Logprintmenu(void)
{
	unsigned short	usPlogEvent;
	char	wk[2];
	ushort	NewOldDate[6];		// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
	ushort	LogCount;			// LOG登録件数
	date_time_rec	NewestDateTime, OldestDateTime;

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[125] );		/* [125] "＜履歴情報プリント＞　　　　　　　　　" */


		usPlogEvent = Menu_Slt( LOGPRNMENU, USM_LOG_TBL, (char)USM_LOG_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usPlogEvent ){

			/* 不正券 */
			case MNT_NGLOG:
				if (OPECTL.Mnt_lev < 2 )		//管理者レベルか
					BUZPIPI();
				else
					usPlogEvent = UsMnt_NgLog();
				break;

			/* 入出庫 */
			case MNT_IOLOG:
				if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
					if( Ext_Menu_Max > 1){
						usPlogEvent = UsMnt_PreAreaSelect(MNT_IOLOG);
					}
					else {
						usPlogEvent = UsMnt_AreaSelect(MNT_IOLOG);
					}
				}else{
					BUZPIPI();
					continue;
				}
				break;

			/* 不正・強制出庫 */
			case FUSKYO_LOG:
				// LOGﾃﾞｰﾀの最古＆最新日付と登録件数を得る
				Ope2_Log_NewestOldestDateGet( eLOG_ABNORMAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// 最古＆最新日付ﾃﾞｰﾀ取得
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				//	ログ件数表示画面処理２（対象期間指定あり）
				usPlogEvent = SysMnt_Log_CntDsp2(LOG_ABNORMAL, LogCount, LOGSTR1[7], PREQ_FUSKYO_LOG, &NewOldDate[0]);
				break;

			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return MOD_EXT;
			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
			SetChange = 1;			// FLASHｾｰﾌﾞ指示
		}
		if (usPlogEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (usPlogEvent == MOD_CUT ) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：係員有効データ                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_AttendantValidData( void )                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	UsMnt_AttendantValidData(void)
{
	ushort	msg;
	T_FrmLogPriReq1	aid_pri;
	T_FrmPrnStop	FrmPrnStop;
	t_ValidWriteData *work = &at_invwork;		// 係員有効データワークテーブル
	long	no,max,top,input;
	ushort	attendNo;							// 係員No.
	char	i,priend;
	char	dsp;

	dispclr();									// 画面クリア

//	max = MNT_ATTEND_REGSTER_COUNT-1;			// 画面で登録可能な件数
	max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// 画面で登録可能な件数
	if(max == MNT_ATTEND_REGSTER_COUNT) {
		--max;
	}

	no = top = 0;
	input = -1;
	dsp = 1;

	// 画面タイトルを表示
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);			// "＜係員有効データ＞　　　　　"
	grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[8]);				//○○○○件

	Fun_Dsp(FUNMSG2[50]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "

	memcpy(work, &Attend_Invalid_table, sizeof(t_ValidWriteData));
	
	for ( ; ; ) {
	
		opedpl(0, 20, (ulong)Attend_Invalid_table.count, 4, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
		if (dsp) {
			for (i = 0; i < 6; i++) {
				if (top+i > max)
					break;
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "－" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);				/* 番号 */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 削除  終了 "
					}
				}
			}
			dsp = 0;
		}
		msg = StoF(GetMessage(), 1);

		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return (MOD_CHG);
		case KEY_TEN_F5:		/* F5:終了 */
			BUZPI();
			return 0;
		case KEY_TEN_F2:		/* F2:＋ */
			BUZPI();
			no++;
			if (no > max) {
				no = top = 0;
			}
			else if (no > top+5) {
				top++;
			}
			for (i = 0; i < 6; i++) {
				if (top+i > max) {
					break;
				}
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "－" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 削除  終了 "
					}
				}
			}
			input = -1;
			break;
		case KEY_TEN_F1:		/* F1:－／読 */
			BUZPI();
			no--;
			if (no < 0) {
				if (max > 5) {
					top = max-5;
				}
				else {
					top = 0;
				}
				no = max;
			}
			else if (no < top) {
				top--;
			}

			for (i = 0; i < 6; i++) {
				if (top+i > max) {
					break;
				}
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "－" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 削除  終了 "
					}
				}
			}
			input = -1;
			break;
		case KEY_TEN_F3:		/* ﾌﾟﾘﾝﾄ */
			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
				BUZPIPI();
				break;
			}
			BUZPI();
			aid_pri.prn_kind = R_PRI;
			aid_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
			aid_pri.Kakari_no = OPECTL.Kakari_Num;
			queset(PRNTCBNO, PREQ_ATEND_INV_DATA, sizeof(T_FrmLogPriReq1), &aid_pri);		// 印字要求
			Ope_DisableDoorKnobChime();

			/* プリント終了を待ち合わせる */
			Fun_Dsp(FUNMSG[82]);				// Fｷｰ表示　"　　　　　　 中止 　　　　　　"

			priend = 0;
			for ( ; ; ) {
				
				msg = StoF( GetMessage(), 1 );

				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
					msg &= (~INNJI_ENDMASK);
				}
				switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case	LCD_DISCONNECT:
						return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case	KEY_MODECHG:			// ﾓｰﾄﾞﾁｪﾝｼﾞ
						BUZPI();
						return MOD_CHG;
	
					case	PREQ_ATEND_INV_DATA:	// 印字終了
						priend = 1;
						break;

					case	KEY_TEN_F3:				// F3ｷｰ（中止）
						BUZPI();
						FrmPrnStop.prn_kind = R_PRI;
						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// 印字中止要求
						priend = 1;
						break;
					case 	TIMEOUT6:
						BUZPIPI();
						priend = 1;
						break;
				}
				if(priend){
					Fun_Dsp(FUNMSG2[50]);			// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "
					break;
				}
			}

			break;
		case KEY_TEN_F4:		// 書込
			attendNo = Attend_Invalid_table.aino[dispPos(top,no-top)];
			if(attendNo == 0) {					// 係員No.未登録行
				if(input > 0 && input < 99) {
					if((short)-1 == AteVaild_Check((ushort)input)) {	// 入力係員No.チェック
						BUZPI();
						AteVaild_Update((ushort)input, 1);				// 係員No.を登録
						max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// 画面の最大件数更新
						if(max == MNT_ATTEND_REGSTER_COUNT) {
							--max;
						}

						if(no < max) {
							no++;
						}
						if (no > max) {
							no = top = 0;
						}
						else if (no > top+5) {
							top++;
						}
						for (i = 0; i < 6; i++) {
							if (top+i > max) {
								break;
							}
							opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
							grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "－" */
							opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
							if(no==top+i) {
								if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
									Fun_Dsp(FUNMSG2[50]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 書込  終了 "
								}
								else {
									Fun_Dsp(FUNMSG2[30]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 削除  終了 "
								}
							}
						}
					}
					else {
						// 係員No.登録済みはエラー
						BUZPIPI();
						opedpl((ushort)(1+no-top), 16, (ulong)attendNo, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 係員No. */
					}
				}
				else {							// 係員No.範囲外
					BUZPIPI();
					opedpl((ushort)(1+no-top), 16, (ulong)attendNo, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 係員No. */
				}
			}
			else {								// 係員No.登録行
				BUZPI();
				if(1 != AteVaild_ConfirmDelete(attendNo)) {							// 係員No.削除確認画面
					// 画面再描画
					dispclr();														// 画面クリア
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);	// "＜係員有効データ＞　　　　　"
					grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[8]);		//○○○○件
					max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// 画面の最大件数更新
					if(max == MNT_ATTEND_REGSTER_COUNT) {
						--max;
					}
					dsp = 1;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//				else {
//					return (MOD_CHG);
//				}
				if(1 == AteVaild_ConfirmDelete(attendNo)){
					return (MOD_CHG);
				}else if(2 == AteVaild_ConfirmDelete(attendNo)){
					return (MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			}
			input = -1;
			break;
		case KEY_TEN:			/* 数字(テンキー) */
			if(0 == Attend_Invalid_table.aino[dispPos(top,no-top)]) {				// 係員No.未設定行
				BUZPI();
				if (input == -1) {
					input = 0;
				}
				input = (input*10 + msg-KEY_TEN0) % 100L;
				opedpl((ushort)(1+no-top), 16, (ulong)input, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 係員No. */
			}
			else {				// 係員No.設定行は入力不可
				BUZPIPI();
				input = -1;
			}
			break;
		case KEY_TEN_CL:		/* 取消(テンキー) */
			BUZPI();
			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 4, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 番号 */
			opedpl((ushort)(1+no-top), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,no-top)], 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* 係員No. */
			input = -1;
			break;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*
 *| 係員有効データ：登録・削除											  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_Update										  |
 *| PARAMETER    : 係員番号：id	登録or削除：status(0:削除 1：登録)		  |
 *| RETURN VALUE : 正常：1	異常：-1									  |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *| REMARK       : 削除後の先頭データが配列の[0]にくるようデータを構成し直す
 *|				   停電保証に対応										  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	AteVaild_Update(ushort id, ushort status)
{
	short idx;
	t_ValidWriteData *work = &at_invwork;			// 係員有効データワークテーブル
	ushort	tmparea;
	memset(work,0,sizeof(t_ValidWriteData));

	if( id > 10000 || !id )
		return (char)-1;
		
	if( Attend_Invalid_table.count == MNT_ATTEND_REGSTER_COUNT && status == 1 ){	// 登録上限に達している場合の登録処理
		wopelg(OPLOG_SET_ATE_INV_MAX, 0, 0);			// 操作モニタ登録
		return (char)-1;								// 登録処理はしない
	}

	idx = AteVaild_Check(id);
	
	if(status){										// ﾃﾞｰﾀのアップデート
		memcpy(work, &Attend_Invalid_table, sizeof(t_ValidWriteData));

		if (idx >= 0) {
			// 該当データあり → そのデータを上書き
				work->aino[idx] = id;
		}else{

			// 該当データなし → 最後尾に追加
            work->aino[work->wtp] = id;

			if (work->count < ADN_VAILD_MAX) {
				work->count++;
			}
			if (++work->wtp >= ADN_VAILD_MAX) {
				work->wtp = 0;
			}
		}
	}else{														// ﾃﾞｰﾀの削除
		if (idx >= 0) {											// 削除ﾃﾞｰﾀがあった場合
            memcpy(work, &Attend_Invalid_table, 4); 			// 登録ｶｳﾝﾄ・書込み位置のｺﾋﾟｰ
			if(work->count == ADN_VAILD_MAX && work->wtp > 0){	// ﾃﾞｰﾀがﾘﾝｸﾞﾊﾞｯﾌｧ状になっている場合
				if(work->wtp < _AteGetDataPos(idx)){			// 削除ﾃﾞｰﾀの位置が書込位置よりも後ろにあった場合
					/*[]----------------------------------------------------------------------[]
					 *|			   先頭(書込)位置   	  削除データ
					 *|				  ↓			          ↓
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]        [999] 
					 *|		┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐      ┬-─┐
					 *|		│１｜２│３｜４│５│６│７│８│９│10│11│・・・│999│
					 *|		└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘      ┴-─┘
					 *|			  				｜①先頭位置から削除ﾃﾞｰﾀまでをｺﾋﾟｰ
					 *|			  				｜②削除ﾃﾞｰﾀの次のﾃﾞｰﾀから配列の最終ﾃﾞｰﾀまでをｺﾋﾟｰ
					 *|			  				｜③配列の先頭から先頭位置(書込位置)までをｺﾋﾟｰする。
					 *|			  				↓※先頭ﾃﾞｰﾀが配列の先頭に来るように再構成し直す。
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]       [997][998][999] 
					 *|		┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐      ┬─┬─┬──┐
					 *|		│３｜４│５｜６│７│８│10│11│12│13│14│・・・│１│２│空白│
					 *|		└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘      ┴─┴─┴──┘
					 *[]----------------------------------------------------------------------[]*/
					tmparea = (ushort)((work->count-1)-(_AteGetDataPos(idx)));
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[work->wtp], (sizeof(Attend_Invalid_table.aino[0])*(_AteGetDataPos(idx)-Attend_Invalid_table.wtp)));
	            	memcpy(&work->aino[(_AteGetDataPos(idx)-work->wtp)], &Attend_Invalid_table.aino[_AteGetDataPos(idx+1)], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
            		memcpy(&work->aino[tmparea+idx], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0])*work->wtp));
	  			}else{											// 削除ﾃﾞｰﾀの位置が書込位置よりも前もしくは同じだった場合
					/*[]----------------------------------------------------------------------[]
					 *|			  削除データ		  先頭(書込)位置
					 *|		    	  ↓ 			          ↓
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]        [999] 
					 *|		┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐      ┬-─┐
					 *|		│１｜２│３｜４│５│６│７│８│９│10│11│・・・│999│
					 *|		└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘      ┴-─┘
					 *|			  				｜①先頭位置から配列の最終ﾃﾞｰﾀまでをｺﾋﾟｰ
					 *|			  				｜②配列の先頭から削除ﾃﾞｰﾀまでをｺﾋﾟｰ
					 *|			  				｜③削除ﾃﾞｰﾀの次のﾃﾞｰﾀから先頭位置(書込位置)までをｺﾋﾟｰする。
					 *|			  				｜※削除データが最終ﾃﾞｰﾀの場合は③は実施しない
					 *|			  				↓※先頭ﾃﾞｰﾀが配列の先頭に来るように再構成し直す。
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]       [997][998][999] 
					 *|		┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐      ┬─┬─┬──┐
					 *|		│９｜10│11｜12│13│14│15│16│17│18│19│・・・│７│８│空白│
					 *|		└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘      ┴─┴─┴──┘
					 *[]----------------------------------------------------------------------[]*/
					if(!idx){							// 削除位置が先頭(書込)位置だった場合
						work->wtp++;					// ｺﾋﾟｰ開始位置をずらす
					}else{								
						if(idx != ADN_VAILD_MAX-1)		// 削除位置が最終ﾃﾞｰﾀ位置では無い場合
							memcpy(&work->aino[(ushort)(work->count-work->wtp)+_AteGetDataPos(idx)], &Attend_Invalid_table.aino[_AteGetDataPos(idx)+1], (sizeof(Attend_Invalid_table.aino[0])*(Attend_Invalid_table.wtp-(_AteGetDataPos(idx)+1))));
					}
					tmparea = (ushort)(work->count-work->wtp);
	            	memcpy(&work->aino[0], &Attend_Invalid_table.aino[work->wtp], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
	            	memcpy(&work->aino[tmparea], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0])*_AteGetDataPos(idx)));

				}
			}else{
				/*	先頭ﾃﾞｰﾀを削除する場合 */
				if(!idx){
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[1], (sizeof(Attend_Invalid_table.aino[0]) * (ADN_VAILD_MAX-1)));
				}else{
					/*	途中のを削除する場合 */
					tmparea = (ushort)_AteGetDataPos(idx);
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0]) * tmparea));
					tmparea = (ushort)((Attend_Invalid_table.count-1)-(_AteGetDataPos(idx)));
		            memcpy(&work->aino[_AteGetDataPos(idx)], &Attend_Invalid_table.aino[_AteGetDataPos(idx+1)], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
				}
			}	
			work->count--;
			work->wtp = work->count;
		}else{
			return (char)-1;
		}
	}
	// ワークエリアから実領域へ書き込み
	nmisave(&Attend_Invalid_table, work, sizeof(t_ValidWriteData));
	wopelg(OPLOG_SET_ATE_INV, 0, 0);
	return 1;

}
/*[]----------------------------------------------------------------------[]*
 *| 係員有効データ：参照												  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_Check     									  |
 *| PARAMETER    : 係員番号：id										      |
 *| RETURN VALUE : 該当データ有：要素番号	該当データ無：-1              |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *| REMARK       : 														  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	AteVaild_Check( ushort id )
{
	short idx,ofs,ret=-1;

	for (idx = 0; idx < Attend_Invalid_table.count; idx++) {
		ofs = _AteIdx2Ofs(idx);
		if (Attend_Invalid_table.aino[ofs] == id) {
			ret = idx;
			break;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *| 有効係員IDテーブルデータ更新										  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_table_update								  |
 *| PARAMETER    : 係員番号：id										      |
 *| RETURN VALUE : 該当データ有：要素番号	該当データ無：-1              |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-06-22				    						  |
 *| REMARK       : 														  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	AteVaild_table_update( ushort *data, ushort size)
{
	t_ValidWriteData *work = &at_invwork;			// 無効係員データワークテーブル
	short ofs;

	memset(work,0,sizeof(t_ValidWriteData));
	memcpy(&work->aino, data, (size_t)size);

	for (ofs = 0; ofs < ADN_VAILD_MAX; ofs++) {
		if (!work->aino[ofs]) {
			memset( &work->aino[ofs],0,(sizeof(work->aino[ofs])*(ADN_VAILD_MAX-ofs)) );
			break;
		}
	}
	work->count = ofs;
	if( ofs != ADN_VAILD_MAX )
		work->wtp = ofs;
	else
		work->wtp = 0;
	nmisave(&Attend_Invalid_table, work, sizeof(t_ValidWriteData));
	wopelg(OPLOG_SET_ATE_INV, 0, 0);

}
/*[]----------------------------------------------------------------------[]*/
/*|  有効係員データ削除画面　　                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AteVaild_ConfirmDelete(  )                              |*/
/*| PARAMETER    : ushort attendNo : 削除する係員No.                       |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	AteVaild_ConfirmDelete(ushort attendNo)
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);		// "＜係員有効データ＞　　　　　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[155]);		// "　係員№　　を削除しますか？　"
	Fun_Dsp(FUNMSG[19]);												// "　　　　　　 はい いいえ　　　"

	opedpl(3, 8, (ulong)attendNo, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// 係員No.

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return 1;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス（メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return 1;
		case KEY_TEN_F3:		/* F3:はい */
			BUZPI();
			AteVaild_Update(attendNo, 0);								// 係員No.を削除
			return 0;
		case KEY_TEN_F4:		/* F4:いいえ */
			BUZPI();
			return 0;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：車室故障						                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_CarFailSet( ushort Index )  	                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ushort	UsMnt_CarFailSet( ushort Index )
{
	ushort	msg;
	uchar	pos;
	uchar	fail = (uchar)(FLAPDT.flp_data[Index].car_fail & 0x01);
	uchar	disp = 0;
	ulong	posi = LockInfo[Index].posi;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, car_title_table[gCurSrtPara - MNT_INT_CAR][2]);	/* "＜車室故障（XX）＞　　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[1] );										/* "　車室番号：　　　　　　　　　" */
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[2]  );									/* "　現在設定：　　　　　　　　　" */
	opedsp( 2,  12, (ushort)posi, (ushort)(posi/1000?4:((posi/100)?3:(posi/10)?2:1)), 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* 番号 */
	grachr( 3, 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, (fail ? CARFAIL[4]:CARFAIL[3]));					/* 故障状態 */
	
	grachr( 4, 12,  8, ((fail==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[3] );						/* "　　　　　　故障なし　　　　　" */
	grachr( 5, 12,  8, ((fail==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[4] );						/* "　　　　　　故障あり　　　　　" */
	Fun_Dsp( FUNMSG[98] );																					/* "　▲　　▼　 　　  設定  終了 " */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
				if( disp )
					break;
				BUZPI();
				fail ^= 1;
				grachr( 4, 12,  8, ((fail==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[3] );	/* "　　　　　　故障なし　　　　　" */
				grachr( 5, 12,  8, ((fail==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[4] );	/* "　　　　　　故障あり　　　　　" */
				break;
			case KEY_TEN_F3:					/* F3: */
				break;
			case KEY_TEN_F4:					/* F4:設定 */
				if( disp )
					break;
				BUZPI();
				if( (FLAPDT.flp_data[Index].car_fail & 0x01) == fail )
					return MOD_EXT;
				
				grachr( 3, 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, (fail ? CARFAIL[4]:CARFAIL[3]));			/* 故障状態 */
				
				if( FLAPDT.flp_data[Index].car_fail & 0x80 ){					// 一度状態を変化させたが元に戻した
					FLAPDT.flp_data[Index].car_fail = fail;						// 今回変更BITを落とす
				}else{															// 以前の状態から変更
					FLAPDT.flp_data[Index].car_fail = (uchar)(fail|0x80);		// 今回変更BITをセット
				}
				if( fail == 0 ){
					wopelg( OPLOG_CCRFAIL_ON, 0, posi );		// 操作履歴登録
				}else{
					wopelg( OPLOG_CCRFAIL_OFF, 0, posi );		// 操作履歴登録
				}
				pos = (uchar)(prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[Index].ryo_syu-1)*6)),1,1 ) & 0x03);
				grachr(4, 0,  30, 0, COLOR_RED, LCD_BLINK_OFF, CARFAIL[5]);					/* "　＜＜  設定しました  ＞＞　　" */
				if( prm_get(COM_PRM, S_SYS, 39, 1, 2) ){
					switch( prm_get(COM_PRM, S_SYS, 39, 1, 1) ){
						case	0:
						default:
							grachr( 5, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[6] );
							break;
						case	3:
						case	4:
							grachr( 5, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[pos+6] );
							break;
						
					}
					grachr( 6, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[10]  );
				}else{
					dispmlclr(5,6);
				}
				Fun_Dsp( FUNMSG[8] );								/* "　　　　　　　　　　　  終了 " */
				disp = 1;	
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：車室故障時の車室情報取得		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataGet_for_Failer( void )  	                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : 車室故障ﾒﾆｭｰ時には本関数を呼び出して、ｸﾘｱしたLockInfoを |*/
/*| 		     : 再設定して、表示を行えるようにする。					   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	LockInfoDataGet_for_Failer( void )
{
	ushort	i;
	uchar	ret=0;
	
	for( i=0; i<LOCK_MAX; i++ ){
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( FLAPDT.flp_data[i].car_fail & 0x01 ){
			LockInfo[i] = bk_LockInfo[i];
			ret = 1;
		}
	}
	
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);								// RAM上のSUM更新
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);							// FLASH書込み
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：車室故障時の車室情報取得		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataSet_for_Failer( ret_value ) 	               |*/
/*| PARAMETER    : ret_value 0:MOD_EXT 1:MOD_CHG                           |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : 車室故障ﾒﾆｭｰ時を抜ける場合には本関数を呼び出すこと	   |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char	LockInfoDataSet_for_Failer( char ret_value )
{

	ushort	i;
	uchar	ret=0;
	uchar	result = 0;
	char	ret2 = ret_value;
	
	for( i=0; i<LOCK_MAX; i++ ){
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if(( result = LockInfoDataSet_sub(i)) != 0 ){
			ret |= result;							// LockInfoの更新あり
		}

	}
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);			// RAM上のSUM更新
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);		// FLASH書込み
		if(( ret & 0x02 ) == 0x02 ){
			if( LockInfoFailer_ResetMessage() == MOD_EXT ){	// 画面終了が終了キー
				if( ret_value == 1 ){				// 子画面からの戻りがメンテキー
					ret2 = 1;						// メンテキーとして戻る
				}else{
					ret2 = 0;						// 終了キーとして戻る
				}
			}else{
				ret2 = 1;							// メンテキーとして戻る
			}
		}
	}
	return ret2;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：車室故障時の再起動メッセージ表示関数            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoFailer_ResetMessage( void )  	               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2010/01/07                                              |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	LockInfoFailer_ResetMessage( void )
{
	ushort	msg;

	dispmlclr(1,6);
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[11] );		/* "故障を解除するには精算機の　　" */
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[12]  );		/* "再起動が必要です。　　　　　　" */
	
	Fun_Dsp( FUNMSG2[43] );						/* "　　　　　　 確認 　　　　　　" */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F3:					/* F3:Exit */
				BUZPI();
				return MOD_EXT;
			default:
				break;
		}
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：車室故障時の車室情報設定		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataSet_sub( void )  		                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : 車室故障ﾒﾆｭｰ時を抜ける場合には本関数を呼び出すこと	   |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	LockInfoDataSet_sub( ushort index )
{
	uchar	ret=0;
	
	if( FLAPDT.flp_data[index].car_fail & 0x01 ){			// 故障あり
		bk_LockInfo[index] = LockInfo[index];
		memset( &LockInfo[index], 0, sizeof( LockInfo[0]) );
		if(index >= INT_CAR_START_INDEX && index < TOTAL_CAR_LOCK_MAX) {
			// フラップの場合は連番を1に設定しておく
			LockInfo[index].lok_no = 1;
		}
		FLAPDT.flp_data[index].car_fail = 0x01;				// 再セット
		ret = 1;
	}else{													// 故障なし
		if( FLAPDT.flp_data[index].car_fail & 0x80 ){		// 故障あり→なしに変更あり
			LockInfo[index] = bk_LockInfo[index];
			FLAPDT.flp_data[index].car_fail = 0;			// 上位のビットを落とす
			ret = 2;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  復電時：車室故障時の車室情報設定					                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataCheck( void )  	     			           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : 車室故障ﾒﾆｭｰ中に停複電した場合には、起動時にエリアの	   |*/
/*| 		     : 更新を纏めて実施する									   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	LockInfoDataCheck( void )
{

	ushort	i;
	uchar	ret=0;

	for( i=0;i<LOCK_MAX; i++ ){
		WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( FLAPDT.flp_data[i].car_fail & 0x80 ){				// 車室故障設定時に停電
			if( FLAPDT.flp_data[i].car_fail & 0x01 ){			// 故障なし⇒あり
				bk_LockInfo[i] = LockInfo[i];					// 車室設定をバックアップ
				memset( &LockInfo[i], 0, sizeof( LockInfo[0]) );// 車室設定をクリア
				if(i >= INT_CAR_START_INDEX && i < TOTAL_CAR_LOCK_MAX) {
					// フラップの場合は連番を1に設定しておく
					LockInfo[i].lok_no = 1;
				}
				memset( &FLAPDT.flp_data[i], 0, sizeof( FLAPDT.flp_data[0]) );	// 車室情報もクリア
				FLAPDT.flp_data[i].car_fail = 0x01;				// 再セット
			}else{												// 故障あり⇒なし
				LockInfo[i] = bk_LockInfo[i];					// バックアップデータをリストア
				FLAPDT.flp_data[i].car_fail = 0;				// 上位のビットを落とす
			}
			ret = 1;
		}
	}
	
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);								// RAM上のSUM更新
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);							// FLASH書込み
	}
}

// 音声案内時間切替機能
//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：音声案内時間
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change
///							MOD_EXT : F5 key
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
struct GUIDE_TIME {
	short sTime[2];		// 開始時刻
	short eTime[2];		// 終了時刻
	char  volume;		// 音量
};

ushort	UsMnt_VoiceGuideTime(void)
{
	ushort	pos;
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_flag = 1;
	uchar	disp_item_flag = 1;
	char volume;
	char nSet;

// MH810100(S) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 音量が変更されない

	// 音量設定数を取得
	nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (MAX_VOICE_GUIDE < nSet) {
		// 最大設定数は3
		nSet = MAX_VOICE_GUIDE;
	}
	// 音量取得
	volume = get_volume(nSet);
	
	// 設定数が1以下なら終日固定、2以上なら時間帯別にカーソルを合わせる
	pos = (nSet != 0);

	do {
		// ＜音声案内時間＞画面表示
		if ( 1 == disp_flag){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "＜音声案内時間＞　　　　　　　"
			grachr(1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[130]);		// "現在の設定：
			if (nSet == 0) {
				grachr(1, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);	// 終日固定音量
			} else {
				grachr(1, 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);	// 時間帯別音量切替
			}
			grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[131]);		// 現在の音量：
			opedsp(2, 12, (ushort)volume, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF); // 音量
			if (volume > 0) {
				Fun_Dsp( FUNMSG[99] );	//"　▲　　▼　テスト 書込  終了 "
			} else {
				Fun_Dsp( FUNMSG[25] );	//"　▲　　▼　　　　 読出  終了 "
			}
			disp_flag = 0;
		}
		if ( 1 == disp_item_flag){
			grachr(4, 6, 12, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);		// 終日固定音量
			grachr(5, 6, 16, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);		// 時間帯別音量切替
			disp_item_flag = 0;
		}
		
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
			Lagcan(OPETCBNO, 6);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
			return MOD_CHG;
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
			ret = MOD_EXT;
			break;
		case KEY_TEN_F4:							// 書込(F4)
			BUZPI();
			if (pos == 0) {
				// 終日固定音量
				ret = UsMnt_VoiceGuideTimeAllDay(&volume, 0);
				if( ret == 1 ){
					CPrmSS[S_SYS][53] = 0;			// 01-0053
					CPrmSS[S_SYS][54] = 0;			// 01-0054
					CPrmSS[S_SYS][55] = 0;			// 01-0055
					CPrmSS[S_SYS][56] = 0;			// 01-0056
					ret = 0;
					nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053⑥
				}
				Ope_EnableDoorKnobChime();
			} else {
				// 時間帯別音量切替
				ret = UsMnt_VoiceGuideTimeExtra(&volume);
				// 設定数を確認
				nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053⑥
				volume = get_volume(nSet);
			}
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
				OpelogNo = 0;
				SetChange = 1;					// FLASHｾｰﾌﾞ指示
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP更新フラグ
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
			}
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F3:							// (F3)
			// 音声を流す
			if (volume != 0) {		// 0なら何もしない
				Ope_DisableDoorKnobChime();
				// 音声終了イベントが無いので、アラーム監視タイマ間はアラームを抑止する
				Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
//				VoiceGuideTest(volume);	// 「ご利用ありがとうございました。またの～」
				lcd_volume	= Anounce_volume((uchar) volume );
				PKTcmd_audio_volume( lcd_volume );
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
			}
			break;
		case KEY_TEN_F1:							// (F1)
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// 終日・時間帯のカーソル移動
			pos ^= 1;
			disp_item_flag = 1;
			break;
		default:
			break;
		}
	} while (0 == ret);
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：音声案内時間（終日固定音量設定）
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume	音量値格納領域へのポインタ
//	@param[in]		ptn		0: 終日固定音量設定
//							1: パターン１（時間帯別音量切替）
//							2: パターン２（時間帯別音量切替）
//							3: パターン３（時間帯別音量切替）
///	@return			ret		MOD_CHG : mode change
///							0		: 正常終了
///							1		: 正常終了(書き込みあり)
///	@author			Yamada
///	@note
///	-	終日固定音量設定の場合は音量値を確定（書込キー押下）した時点でパターン２,３(設定アドレス01-0051の下４桁)を０クリアする。
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_VoiceGuideTimeAllDay(char *volume, ushort ptn)
{
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_item_flag = 1;
	ushort	vol;
	long	*pvol;
	uchar	each_volume[ MAX_VOICE_GUIDE ];	/*	パターン１～３の各音量
											 *	each_volume[0]:パターン１（時間帯別音量切替）／終日固定音量
											 *	each_volume[1]:パターン２（時間帯別音量切替）
											 *	each_volume[2]:パターン３（時間帯別音量切替）
											 */
	char	f_write = 0;
	ushort	column;							// 描画位置
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 音量が変更されない	
	uchar	lcd_volume;						// 音量
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
	

	pvol = &CPrmSS[S_SYS][51];			// 01-0051
	// 現在設定されている音量を記憶
	memset( each_volume, 0, sizeof(each_volume) );
	each_volume[ 0 ] = (uchar)( ( *pvol / 10000 ) % 100 );	// パターン１の音量を取り出す
	each_volume[ 1 ] = (uchar)( ( *pvol / 100 ) % 100 );	// パターン２の音量を取り出す
	each_volume[ 2 ] = (uchar)( *pvol % 100 );				// パターン３の音量を取り出す

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[136]);	// ⊂⊃キーで音量を設定して下さい
	grachr(4, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[137]);	// 音量設定：　　　　　　　　　　
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[138]);	// 1　　　　　　　　　　　　　 16

	vol = (*volume);
// MH321800(S) T.Nagai ICクレジット対応
//	if (vol > 15) {
//		// 音量最大値は15
//		vol = 15;
//	}
	if (vol > ADJUS_MAX_VOLUME) {
		// 音量最大値は15
		vol = ADJUS_MAX_VOLUME;
	}
// MH321800(E) T.Nagai ICクレジット対応

	do {
		// ＜音声案内時間＞画面表示
		if ( 1 == disp_item_flag ){
			opedsp(4, 10, vol, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// 音量
			if ( vol <= 15 ) {
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor( COLOR_DARKORANGE );
// MH810100(E) S.Takahashi 2019/12/25
				column = (ushort)(vol*2);
				if(column) {
					grachr( 6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);		// プログレスバー表示
				}
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
// MH810100(E) S.Takahashi 2019/12/25
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
			}
			if ( vol > 0) {
				Fun_Dsp( FUNMSG[100] );
			} else {
				Fun_Dsp( FUNMSG[13] );
			}
			disp_item_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//			ope_anm(AVM_STOP);
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			if (msg == KEY_MODECHG) {
//				return MOD_CHG;
//			} else {
//				return ret;
//			}
			if (msg == KEY_MODECHG) {
				return MOD_CHG;
			} else if(msg == LCD_DISCONNECT) {
				return MOD_CUT;
			} else {
				return ret;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_TEN_F4:							// 書込(F4)
			BUZPI();
			// 音量書込み
			f_write = 1;
			*volume = (char)vol;
			switch ( ptn ) {
			case 0:
				*pvol = (long)( (long)*volume * 10000L );
				break;
			case 1:
				*pvol = (long)(( (long)*volume * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)each_volume[ 2 ]);
				break;
			case 2:
				*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)*volume * 100 ) + (long)each_volume[ 2 ]);
				break;
			case 3:
				*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)*volume);
				break;
			default:
				*pvol = 0;	// ptnが範囲外の場合は０にしておく
				break;
			}

			// 操作履歴登録・FLASHにセーブ
			OpelogNo = OPLOG_VLSWTIME;
			f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

			if (f_write == 1 && ptn == 0) {
				CPrmSS[S_SYS][53] = 0;			// 01-0053
				CPrmSS[S_SYS][54] = 0;			// 01-0054
				CPrmSS[S_SYS][55] = 0;			// 01-0055
				CPrmSS[S_SYS][56] = 0;			// 01-0056
			}
			ret = 1;
			break;
		case KEY_TEN_F3:	/* (F3) */
			// 音声を流す
			if (vol != 0) {		// 0なら何もしない
				Ope_DisableDoorKnobChime();
				// 音声終了イベントが無いので、アラーム監視タイマ間はアラームを抑止する
				Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
//				VoiceGuideTest((char)vol);	// 「ご利用ありがとうございました。またの～」
				lcd_volume = Anounce_volume( (uchar)vol );
				PKTcmd_audio_volume( lcd_volume );		// アナウンス音量をLCDに送信
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 音量が変更されない
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// プログレスバー左へ移動
			if (vol > 0) {
				vol--;
				disp_item_flag = 1;
			}
			break;
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// プログレスバー右へ移動
// MH321800(S) T.Nagai ICクレジット対応
//			if (vol < 15) {
			if (vol < ADJUS_MAX_VOLUME) {
// MH321800(E) T.Nagai ICクレジット対応
				vol++;
				disp_item_flag = 1;
			}
			break;
		default:
			break;
		}
	} while (1);

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			時間帯別の計算
//[]----------------------------------------------------------------------[]
///	@return			ret		1	: 正常終了
///							0	: 入力時刻不正
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Calc_tmset_sc(ushort tNum, struct GUIDE_TIME * guide)
{
	ushort time[3];
	ushort wtime;
	ushort num;
	char i;
	char ret = 1;

	for ( i = 0; i < tNum ; ++i ){
		time[i] = guide[i].sTime[0]*60 + guide[i].sTime[1];
		if ( guide[i].sTime[1] >= 60 || time[i] >= 24*60 ){
			return 0;
		}
	}
	num = tNum;
	for ( i = 0; i < (num-1) ; ++i ){
		// 2分以上の開始時間の差がない場合を矛盾とみなす
		if(time[i] + 1 >= time[i+1]){
			num = i+1;
			ret = 0;
			break;
		}
	}
	for ( i = 0; i < tNum; ++i ){
		if(i < num){
			if( i == num - 1 ){
				wtime = time[0];
			}else{
				wtime = time[i+1];
			}
			if ( 0 == wtime ){
				wtime = 24*60-1;
			} else {
				--wtime;
			}
			guide[i].eTime[0] = wtime/60;
			guide[i].eTime[1] = wtime%60;
		}
		else{
			guide[i].eTime[0] = -1;
			guide[i].eTime[1] = -1;
		}
	}
	return ret;
}


//[]----------------------------------------------------------------------[]
///	@brief			ユーザーメンテナンス：音声案内時間（時間帯別音量切替設定）
//[]----------------------------------------------------------------------[]
///	@return			ret		1	: 終日固定画面に移行（未使用,コメント参照）
///							0	: 正常終了
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_VoiceGuideTimeExtra(char *volume)
{
	ushort	msg;
	uchar	disp_flag = 1;
	ushort	pos_l = 0;
	ushort	pos_c = 0;	// 0:時,1:分
	struct	GUIDE_TIME guideTime[3];
	ushort	tNum;
	ushort	num;
	ushort	calc_flag = 0;
	ushort	i;
	char	flag;
	ushort	tmInput;
	long	*ptim;
	const char prm_pos[3] = {5, 3, 1};
	ulong	tmpvol;
	char	cl_flag = 1;
	ushort	ret;
	ptim = CPrmSS[S_SYS];
	
	// 設定数取得
	tNum = (short)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (tNum == 0) {
		tNum = 1;
	} else if (tNum > MAX_VOICE_GUIDE) {
		tNum = MAX_VOICE_GUIDE;
	}
	num = tNum;

	// 初期化
	memset(guideTime, 0, sizeof(struct GUIDE_TIME) * 3);

	for (i = 0; i < num; i++) {
		// 開始時刻 01-0054,01-0055,01-0056
		guideTime[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// 時
		guideTime[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// 分
		// 音量 01-0051①②,③④,⑤⑥
		guideTime[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);	// No  開始時刻   終了時刻	 音量

	tmInput = guideTime[0].sTime[0] * 100 + guideTime[0].sTime[1];
	while (1) {
		if ( 1 == disp_flag ){
			calc_flag = Calc_tmset_sc(num, guideTime);
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);														// No  開始時刻   終了時刻	 音量
			for( i = 0; i < MAX_VOICE_GUIDE ; ++i ){
				if( i < num ){
					if( guideTime[i].eTime[1] == -1 ){
						grachr((ushort)(2 + i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);								// 　 －－：－－ －－：－－　－－
					}
					flag = (i == pos_l) && (0 == pos_c);
					opedsp((ushort)(i + 2), 3,	(ushort)guideTime[i].sTime[0], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // 　 ＊＊：－－ －－：－－　－－
					grachr((ushort)(i + 2), 7,	2, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); // コロンを反転
					opedsp((ushort)(i + 2), 9,	(ushort)guideTime[i].sTime[1], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // 　 －－：＊＊ －－：－－　－－
					if( guideTime[i].eTime[1] != -1 ){
						opedsp((ushort)(i + 2), 14, (ushort)guideTime[i].eTime[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 　 －－：－－ ＊＊：－－　－－
						grachr((ushort)(i + 2), 18,	2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 			// コロンを正転表示
						opedsp((ushort)(i + 2), 20, (ushort)guideTime[i].eTime[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 　 －－：－－ －－：＊＊　－－
					}
					opedsp((ushort)(i + 2), 26, (ushort)guideTime[i].volume,   2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF); 			// 　 －－：－－ －－：－－　＊＊
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//＊＊－－：－－ －－：－－　－－
				}else{
					grachr((ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);									// 　 －－：－－ －－：－－　－－
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//＊＊－－：－－ －－：－－　－－
				}
			}

			
			if( tNum > 1 && (pos_l + 1) == tNum ){
				Fun_Dsp( FUNMSG[101] );					// "　音量　　▼　　 削除 　書込　　終了　"
			}else{
				Fun_Dsp( FUNMSG[102] );					// "　音量　　▼　　　　　　書込　　終了　"
			}
			disp_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
			return 0;
		case KEY_TEN_F4:							// 書込(F4)
			if ( 1 == calc_flag ){
				// 時間を書込み
				ptim[54 + pos_l] = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
				if( tNum < pos_l + 1 ){
					tNum = pos_l + 1;
				}
				ptim[53] = tNum;

				// 音量の継承時の値のまま「書込」した場合には、その音量で確定させる
				switch (pos_l) {
				case 0:
					tmpvol = ptim[51] % 10000;
					tmpvol += (ulong)(guideTime[pos_l].volume) * 10000;
					break;
				case 1:
					tmpvol  = ptim[51] / 10000;
					tmpvol *= 10000;
					tmpvol += ptim[51] % 100;
					tmpvol += (ulong)(guideTime[pos_l].volume) * 100;
					break;
				case 2:
					tmpvol  = ptim[51] / 100;
					tmpvol *= 100;
					tmpvol += guideTime[pos_l].volume;
					break;
				default:
					tmpvol = 0;
					break;
				}
				ptim[51] = tmpvol;
				// 確定したときには一つ前の音量を継承して表示
				if ( pos_l < MAX_VOICE_GUIDE ){
					if( !ptim[54 + pos_l + 1] )								// 次の開始時刻が未設定の場合
					guideTime[pos_l+1].volume = guideTime[pos_l].volume;
				}
				// 操作履歴登録・FLASHにセーブ
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;									// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				goto UsMnt_VoiceGuide_CUR_DWN;	// カーソル移動
			}else{
				// 時刻が不正であれば、PIPI音の後、表示を戻す
				BUZPIPI();
				// 開始時刻 01-0054,01-0055,01-0056
				guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
				guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
				tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
				disp_flag = 1;
				break;
			}
		case KEY_TEN_F2:							// (F2)
			// 最新の音量設定を画面に反映させる
			guideTime[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			if ( pos_l+1 < MAX_VOICE_GUIDE ){
				if( !ptim[54 + pos_l + 1] )									// 次の開始時刻が未設定の場合
					guideTime[pos_l+1].volume = guideTime[pos_l].volume;	// 一つ前の設定を継承
			}
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
UsMnt_VoiceGuide_CUR_DWN:
			BUZPI();
			
			++pos_l;
			if(pos_l < MAX_VOICE_GUIDE) {
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
			}
			disp_flag = 1;
			pos_c = 0;
			if ( pos_l > tNum || pos_l >= 3 ){
				pos_l = 0;
				if(num > 1 && tNum < 3 ){
					num--;
				}
			} else {
				if ( tNum <= pos_l ){
					num++;
				}
			}
			tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
			cl_flag = 1;
			break;		
		case KEY_TEN_F3:	// (F3)
			// 設定削除(開始時刻、音量をクリア、設定数を-1)
			if( tNum > 1 && (pos_l + 1) == tNum ){
				tNum--;
				ptim[53] = tNum;
				ptim[54 + pos_l] = 0;
				switch (pos_l) {
				case 0:
					tmpvol = ptim[51] % 10000;
					break;
				case 1:
					/*
					 *	@note	パターン２を削除した時にパターン３の音量値を残しておかないようにする。（０クリアする）
					 */
					tmpvol = ( ptim[51] / 10000 ) * 10000;
					break;
				case 2:
					tmpvol  = ptim[51] / 100;
					tmpvol *= 100;
					break;
				default:
					tmpvol = 0;
					break;
				}
				ptim[51] = tmpvol;
				// 操作履歴登録・FLASHにセーブ
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				goto UsMnt_VoiceGuide_CUR_DWN;
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// 音量設定画面へ
			ret = UsMnt_VoiceGuideTimeAllDay(&guideTime[pos_l].volume, (ushort)(pos_l + 1));
			*volume = guideTime[pos_l].volume;
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
				OpelogNo = 0;
				SetChange = 1;					// FLASHｾｰﾌﾞ指示
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
			}
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if( ret == MOD_CUT){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			for (i = 1; i < 7; i++) {
				displclr(i);
			}
			disp_flag = 1;
			break;
		case KEY_TEN:								// 数字(テンキー)
			// 時間設定
			BUZPI();
			if( cl_flag ){
				tmInput = 0;
				cl_flag = 0;
			}
			tmInput = (tmInput % 1000) * 10 + msg - KEY_TEN0;
			guideTime[pos_l].sTime[0] = tmInput / 100;
			guideTime[pos_l].sTime[1] = tmInput % 100;
			disp_flag = 1;
			break;
		case KEY_TEN_CL:							// 取消(テンキー)
			BUZPI();
			// 開始時刻 01-0054,01-0055,01-0056
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
			tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
			// 音量 01-0051①②,③④,⑤⑥
			guideTime[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			disp_flag = 1;
			cl_flag = 1;
			break;
		default:
			break;
		}
	}
	
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			時間帯別の音量取得
//[]----------------------------------------------------------------------[]
///	@return			vol		:	音量
///	@param[in]		num		:	設定数
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_volume(char num)
{
	char ptn, i, vol, pos;
	short nowtime, settime1, settime2;
	
	nowtime = (short)CLK_REC.hour * 100 + (short)CLK_REC.minu;	// 現在時刻
	// 開始時間のパターンを取得
	if (num <= 1) {
		ptn = 1;				// 設定数が1以下はパターン1
	} else {
		for (i = 0; i < num - 1; i++) {
			// 現在時刻が設定パターンiとi+1の間にあるか判定する
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054～
			if (settime1 <= nowtime) {
				settime2 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 55), 4, 1);
				if (settime1+1 < settime2){
					if (nowtime < settime2) {
						ptn = (char)(i + 1);
						break;
					}
				}else{
					ptn = (char)(i + 1);
					break;
				}
			}
		}
		if (i == num - 1) {
			ptn = num;
		}
	}
	// 音量取得
	switch (ptn) {
	case 1:	pos = 5;	break;
	case 2:	pos = 3;	break;
	case 3:	pos = 1;	break;
	default:			return 0;
	}
	vol = (char)prm_get(COM_PRM, S_SYS, 51, 2, pos);
	
	return vol;
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室情報の設定を取得する 				       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_Search_Ifno	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2009-07-24                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	GetCarInfoParam( void )
{
	uchar	param[2]={0};
	uchar	ret=0;

	param[0] = (uchar)prm_get(COM_PRM,S_TYP,100,1,1);
	param[1] = (uchar)prm_get(COM_PRM,S_TYP,100,1,2);
	
	if( param[0] )				// 駐輪併設あり？
		ret = 0x01;

	if(param[1]){
		ret |= 0x04;			// 駐車（内蔵）あり？
	}
	return ret;
}
#if (5 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| 料金計算エリア内のデータ印字処理 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mc10print  	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : sugata                                                 |*/
/*| Date         : 2010-08-25                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
unsigned short UsMnt_mc10print(void)
{
	ushort msg,dat;
	int		inji_end;
	int		can_req;
	T_FrmChargeSetup	FrmChargeSetup;
	T_FrmPrnStop		FrmPrnStop;

	dispclr();

	grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0]);		// "＜料金設定＞　　　　　　　　　"
	grachr(2,  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		// ""
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);
	Fun_Dsp( FUNMSG[96] );
	inji_end = 1;
	dat = 1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == (INNJI_ENDMASK|PREQ_CHARGESETUP)) {
			inji_end = 1;
			msg &= (~INNJI_ENDMASK);
		}
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F3:
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// 印字中止要求
					can_req = 1;
					FrmPrnStop.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
				}
			}
			else{
				if(dat == 0){
					BUZPIPI();
					break;
				}
				FrmChargeSetup.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// 機械№
				FrmChargeSetup.NowTime.Mon = 0xff;							// mc10印字フラグとして使用
				FrmChargeSetup.NowTime.Year = dat;							// 料金体系指定
				FrmChargeSetup.prn_kind = R_PRI;
				queset( PRNTCBNO, PREQ_CHARGESETUP, sizeof(T_FrmChargeSetup), &FrmChargeSetup );// 料金設定印字
				inji_end = 0;
				can_req = 0;
				grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "　　　 プリント中です 　　　　"
				Fun_Dsp(FUNMSG[82]);													// "　　　　　　 中止 　　　　　　"
			}
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			if (inji_end != 0) {
				BUZPI();
				dat = msg-KEY_TEN0;
				opedsp(2, 2, dat, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);
			}
			break;
		case PREQ_CHARGESETUP:
			if (inji_end == 1) {
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);;
				Fun_Dsp( FUNMSG[96] );
			}
			break;
		default:
			break;
		}
		
	}

}
#endif
//[]----------------------------------------------------------------------[]
///	@brief	ユーザーメンテナンス：コントラスト調整
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	UsColorLCDLumine(void)
{

	ushort	msg;
	uchar	disp_flag;
	long	blgt;
	char	changing;
	long	data;
	ushort	tim1, tim2;
	ushort	set;
// MH810100(S) S.Takahashi 2020/02/10 #3848 [終了]を2回押下しないと輝度調整画面に遷移しない
	ushort ret = 0;
// MH810100(E) S.Takahashi 2020/02/10 #3848 [終了]を2回押下しないと輝度調整画面に遷移しない
	char	off_flag;

	disp_flag = 1;
	changing = 0;
	for ( ; ; ) {
		if (1 == disp_flag){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0] );		// "＜輝度調整＞　　　　　　　　　"
			grachr(1, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);					// "現在の状態"
			grachr(1, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					// "：" 
			grachr(2, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[10]);		// "現在の輝度："

			blgt = CPrmSS[S_PAY][29] % 10;
			if (blgt == 0) {
				grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);				// "常時点灯"
				off_flag = 0;															// ON
			} else if (blgt == 1) {
				grachr(1, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);			// "テンキー入力"
				off_flag = 0;															// ON
			} else {
				grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);			// "夜時間帯"
				off_flag = 0;															// OFF
				set = (ushort)CPrmSS[S_PAY][30];
				tim1 = ( set / 100 ) * 60 + ( set % 100 );
				set = (ushort)CPrmSS[S_PAY][31];
				tim2 = ( set / 100 ) * 60 + ( set % 100 );
				if( tim1 == tim2 ){														// 夜間時間帯なし
					off_flag = 0;														// 
				}else if( tim1 < tim2 ){
					if(( tim1 <= CLK_REC.nmin )&&( CLK_REC.nmin <= tim2 )){				// 夜間時間帯
						off_flag = 1;													// 
					}
				}else{
					if(( CLK_REC.nmin >= tim1 )||( tim2 >= CLK_REC.nmin )){				// 夜間時間帯
						off_flag = 1;													// 
					}
				}
			}
			opedsp(2, 16, (ushort)prm_get(COM_PRM, S_PAY, (short)(27+off_flag), 2, 1), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// 輝度値(02-0027⑤⑥/02-0028⑤⑥)
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27+off_flag, 2, 1));	// 点灯時の輝度
			grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);					// "常時点灯" */
			grachr(4, 14, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);				// "テンキー入力"
			grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);					// "夜時間帯"
			Lumine_time_dsp(1, CPrmSS[S_PAY][30], 0);
			grachr(6, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[8]);				// "～"
			Lumine_time_dsp(2, CPrmSS[S_PAY][31], 0);
			Lumine_dsp(blgt, 1);
			Fun_Dsp( FUNMSG2[48] );														// "　⊂　　⊃　 調整　書込  終了 "
			disp_flag = 0;
		}
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* 終了(F5) */
			LcdBackLightCtrl( ON );							// back light ON
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ←(F1) */
		case KEY_TEN_F2:	/* →(F2) */
			if (changing)
				break;
			BUZPI();
			Lumine_dsp(blgt, 0);
			if (msg == KEY_TEN_F1) {
				if (blgt == 3) {
					blgt -= 2;
				} else {
					blgt -= 1;
				}
				if (blgt < 0)
					blgt += 4;
			}
			else {
				if (blgt == 1) {
					blgt += 2;
				} else {
					blgt += 1;
				}
				if (blgt >= 4)
					blgt -= 4;
			}
			Lumine_dsp(blgt, 1);
			break;
		case KEY_TEN_F3:	// 調整(F3)
			if (changing){
				break;
			}
			BUZPI();
			// 輝度値変更
// MH810100(S) S.Takahashi 2020/02/10 #3848 [終了]を2回押下しないと輝度調整画面に遷移しない
//			if (UsColorLCDLumine_change(off_flag) == MOD_CHG ) {
//				return MOD_CHG;
//			}
//// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			if ( UsColorLCDLumine_change(off_flag) == LCD_DISCONNECT){
//				return MOD_CUT;
//			}
//// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			ret = UsColorLCDLumine_change(off_flag);
			if (ret == MOD_CHG || ret == MOD_CUT) {
				return ret;
			}
// MH810100(E) S.Takahashi 2020/02/10 #3848 [終了]を2回押下しないと輝度調整画面に遷移しない
			LcdBackLightCtrl( ON );							// back light ON
			disp_flag = 1;
			break;
		case KEY_TEN_F4:	/* 書込(F4) */
			if (changing == 0) {
				BUZPI();
				CPrmSS[S_PAY][29] = CPrmSS[S_PAY][29]/10*10 + blgt;
				grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);				/* "　　" */
				if (blgt == 0)
					grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "常時点灯" */
				else if (blgt == 1)
					grachr(1, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
				else {
					grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */
					Lumine_dsp(blgt, 0);
					Lumine_time_dsp(1, CPrmSS[S_PAY][30], 1);
					Fun_Dsp(FUNMSG2[1]);		/* "　　　　　　　　　 書込  終了 " */
					changing = 1;
					data = -1;
				}
				OpelogNo = OPLOG_BACKLIGHT;		// 操作履歴登録
				f_ParaUpdate.BIT.other = 1;		// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP更新フラグセット
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
			}
			else {
				if (data != -1) {
					if (data%100 > 59 || data > 2359) {
						BUZPIPI();
						Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
						data = -1;
						break;
					}
					CPrmSS[S_PAY][29+changing] = data;
				}
				BUZPI();
				Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 0);
				if (changing == 1) {
					changing = 2;
					Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
					data = -1;
				}
				else {
					Lumine_dsp(blgt, 1);
					Fun_Dsp( FUNMSG2[48] );												// "　⊂　　⊃　 調整　書込  終了 "
					changing = 0;
					disp_flag = 1;
				}
			}
			break;
		case KEY_TEN:		/* 数字(テンキー) */
			if (changing == 0)
				break;
			BUZPI();
			if (data == -1)
				data = 0;
			data = (data*10 + msg-KEY_TEN0) % 10000;
			Lumine_time_dsp(changing, data, 1);
			break;
		case KEY_TEN_CL:	/* 取消(テンキー) */
			BUZPI();
			if (changing == 0) {
				Lumine_dsp(blgt, 0);
				blgt = CPrmSS[S_PAY][29] % 10;
				Lumine_dsp(blgt, 1);
			}
			else {
				Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
				data = -1;
			}
			break;
		default:
			break;
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief	ユーザーメンテナンス：コントラスト調整
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static	ushort	UsColorLCDLumine_change(char off_flag)
{
	ushort	msg;
	int		lumine_change = 0;
	uchar	side;								// 0:点灯(精算時･夜時間帯)，1:消灯(待機時･昼時間帯)
	uchar	mode;
	unsigned short	lumine_num[2];				// [0]:点灯時の輝度，[1]:消灯時の輝度
	int		write_flg = 0;
	int		mode_change = 0;

	side = off_flag;
 	mode = (uchar)prm_get(COM_PRM, S_PAY, 29, 1, 1);			// 02-0029⑥
	lumine_num[0] = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027⑤⑥
	lumine_num[1] = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028⑤⑥

	BUZPI();
	dispclr();
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[7]);		// "⊂⊃キーで輝度を設定して下さい"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[8]);		// "輝度設定：　　　　　　　　　　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[9]);		// "0 　　　　　　　　　　　　　15"
	if (mode > 0) {
		Fun_Dsp(FUNMSG2[48]);						// "　⊂　　⊃　 切替  書込  終了 "
	} else {
		Fun_Dsp(FUNMSG[13]);						// "　⊂　　⊃　　　　 書込  終了 "
	}
	lumine_change = 1;


	while (1) {
		msg = StoF(GetMessage(), 1);			// ﾒｯｾｰｼﾞ受信

		switch (msg) {
		case KEY_TEN_F5:						// 終了
			BUZPI();
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// 操作履歴登録
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// 点灯時の輝度
			return MOD_EXT;
			break;

		case KEY_TEN_F1:						// ⊂
			BUZPI();
			// プログレスバー左へ移動
			if (lumine_num[side] > 0) {
				lumine_num[side]--;
				lumine_change = 1;
			}
			break;

		case KEY_TEN_F2:						// ⊃
			BUZPI();
			// プログレスバー右へ移動
			if (lumine_num[side] < LCD_LUMINE_MAX) {
				lumine_num[side]++;
				lumine_change = 1;
			}
			break;

		case KEY_TEN_F3:						// 切替
			if (mode > 0) {
				BUZPI();
				if (side) {
					side = 0;
				} else {
					side = 1;
				}
				lumine_change = 1;
			}
			break;
		case KEY_TEN_F4:						// 書込
			BUZPI();
			CPrmSS[S_PAY][27 + side] = lumine_num[side];
			write_flg = 1;
			break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			mode_change = 2;	// 「操作モード切替イベント受信済み」とする
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// 操作履歴登録
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// 点灯時の輝度
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			mode_change = 1;	// 「操作モード切替イベント受信済み」とする
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// 操作履歴登録
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// 点灯時の輝度
			break;
		default:
			break;
		}

		if (lumine_change) {
			lcd_contrast((unsigned char)lumine_num[side]);
			backlight = lumine_num[side];
			switch (mode) {
			case 1:		// テンキー入力により点灯
			case 3:		// 夜間時間帯のみ常時点灯(昼間はテンキー入力により点灯)
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[mode+side]);	// "＜輝度調整＞　　　　（ＸＸＸ）"
				break;
			default:	// 常時点灯
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0]);			// "＜輝度調整＞　　　　　　　　　"
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}
			opedsp(4, 10, (ushort)lumine_num[side], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 輝度
			if (lumine_num[side] < LCD_LUMINE_MAX) {
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor(COLOR_DARKORANGE);			// カラーLCDへの背景色表示
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
				grachr( 6 , 0, (ushort)((lumine_num[side]*2)), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);	// プログレスバー表示
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor(COLOR_WHITE);					// カラーLCDへの背景色表示
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
				grachr(6, (ushort)((lumine_num[side]*2)+1), 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
			}else{
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor(COLOR_DARKORANGE);			// カラーLCDへの背景色表示
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
				grachr( 6 , 0, 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);								// プログレスバー表示
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor(COLOR_WHITE);					// カラーLCDへの背景色表示
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
				lumine_num[side] = LCD_LUMINE_MAX;
			}
			lumine_change = 0;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if ( mode_change != 0 ) {
		if ( mode_change == 1 ) {
			if (write_flg) {
				wopelg(OPLOG_CONTRAST,0,0);			// 操作履歴登録
			}
			return MOD_CHG;
		}
		if ( mode_change == 2 ) {
			if (write_flg) {
				wopelg(OPLOG_CONTRAST,0,0);			// 操作履歴登録
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
	}
}

//[]----------------------------------------------------------------------[]
///	@brief	ユーザーメンテナンス：コントラスト調整方法表示 
//[]----------------------------------------------------------------------[]
///	@param[in]		data : 点灯状態
///	@param[in]		rev  : 0:正転 1:反転   
///	@return			void   
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	Lumine_dsp(long data, ushort rev)
{
	data = data%10;
	if (data == 0) {
		grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);		/* "常時点灯" */
	}
	else if (data == 1) {
		grachr(4, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "テンキー入力" */
	}
	else {
		grachr(5, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "夜時間帯" */

	}
}
//[]----------------------------------------------------------------------[]
///	@brief	ユーザーメンテナンス：コントラスト調整方法表示2
//[]----------------------------------------------------------------------[]
///	@param[in]		chng : 1:開始時刻 2:終了時刻
///	@param[in]		time : 時刻
///	@param[in]		rev  : 0:正転 1:反転      
///	@return			void   
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	Lumine_time_dsp(char chng, long time, ushort rev)
{
	chng -= 1;
	opedsp(6, (ushort)(4+(chng*12)), (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 時 */
	grachr(6, (ushort)(8+(chng*12)), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "：" */
	opedsp(6, (ushort)(10+(chng*12)), (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* 分 */
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車証明書設定					 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkingCertificateSet( void )                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static unsigned short	UsMnt_ParkingCertificateSet(void)
{
	char	val;		// 03-0062③
	int		pos;		// same as num
	char	max = 1;
	short	in_now = -1;
	ushort	len;
	ushort	num;
	long	param;
	ushort	msg;
	static	const	uchar	setting[] = {2, 1, 0};

	num = (ushort)prm_get(COM_PRM, S_TYP, 62, 1, 3);		// 03-0062④
	pos = (num == 0)? 0 : 1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[43]);		// "＜駐車証明書発行設定＞"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10]);		/* "　現在の状態　　　　　　　　　" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		/* "　　　　　　：　　　　　　　　" */
	len = (ushort)strlen(P_CERTI_SEL[0]);
	grachr( 4, 14, len, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[0] );
	len = (ushort)strlen(P_CERTI_SEL[1]);
	grachr( 5, 14, len, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[1] );

	len = (ushort)strlen(P_CERTI_SEL[pos]);
	grachr( 2, 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos] );

	if (pos != 0) {
	// 発行可能枚数(現在の状態が"する"設定のみ表示)
		grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[44] );	/* "  発行可能枚数：  枚（１～９）" */
		opedsp( 6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		max = 2;
	}

	Fun_Dsp( FUNMSG[20] );					/* "　▲　　▼　　　　 書込  終了 " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (KEY_TEN0to9(msg)) {
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
			if (pos == 2) {
			// 発行可能枚数
				opedsp(6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
				in_now = -1;
			} else {
				grachr((ushort)(4+pos), 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos]);
			}
			if (msg == KEY_TEN_F1) {
				if (--pos < 0) {
					pos = max;
				}
			}
			else {
				if (++pos > max) {
					pos = 0;
				}
			}
			if (pos == 2) {
			// 発行可能枚数
				opedsp(6, 16, num, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			} else {
				len = (ushort)strlen(P_CERTI_SEL[pos]);
				grachr((ushort)(4+pos), 14, len, 1, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos]);
			}
			break;
		case KEY_TEN_F4:					/* F4:書込 */
			BUZPI();
			if (pos == 2) {
			// 発行可能枚数
				if (in_now != -1) {
				// 入力値あり
					param = (prm_get(COM_PRM, S_TYP, 62, 1, 4) * 1000);		// 03-0062③
					param += (in_now * 100);								// 入力値
					param += (prm_get(COM_PRM, S_TYP, 62, 1, 2) * 10);		// 03-0062⑤
					param += prm_get(COM_PRM, S_TYP, 62, 1, 1);				// 03-0062⑥
					CPrmSS[S_TYP][62] = param;
					num = (ushort)in_now;
					in_now = -1;
				}
			} else {
				if (pos == 1) {
				// "する"→発行可能枚数表示(7行目)
					if (num != 0) {
					// "する"→"する"なら変更なし(ログ登録は行う)
						OpelogNo = OPLOG_SHOUMEISETTEI;
						break;
					}
					num = 2;
					grachr( 2, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6] );						/* "  " */
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[44] );						/* "  発行可能枚数：  枚（１～９）" */
					opedsp( 6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
					val = setting[prm_get(COM_PRM, S_TYP, 62, 1, 4)];	// 03-0062③の設定によって
					CPrmSS[S_ANA][6] = 195153;	// 25-0006「駐車証明書が必要な方は、駐車証明書キーを押して下さい。精算される方は、精算キーを押して下さい。」「訂正は取消キーを押して下さい。」
					CPrmSS[S_ANA][7] = 000000;	// 25-0007「(なし)」
					CPrmSS[S_ANA][8] = 000000;	// 25-0008「(なし)」
					max = 2;
				} else {
				// "しない"→発行可能枚数表示削除(7行目)
					if (num == 0) {
					// "しない"→"しない"なら変更なし(ログ登録は行う)
						OpelogNo = OPLOG_SHOUMEISETTEI;
						break;
					}
					num = 0;
					displclr(6);
					val = pos;
					CPrmSS[S_ANA][6] = 141153;	// 25-0006「車室番号を確認して良ければ精算キーを押して下さい。」「訂正は取消キーを押して下さい。」
					CPrmSS[S_ANA][7] = 000000;	// 25-0007「(なし)」
					CPrmSS[S_ANA][8] = 000000;	// 25-0008「(なし)」
					max = 1;
				}
				grachr( 2, 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos] );
				param = (prm_get(COM_PRM, S_TYP, 62, 1, 4) * 1000);		// 03-0062③
				param += (num * 100);									// 03-0062④
				param += (prm_get(COM_PRM, S_TYP, 62, 1, 2) * 10);		// 03-0062⑤
				param += val;											// 03-0062⑥
				CPrmSS[S_TYP][62] = param;
			}
			OpelogNo = OPLOG_SHOUMEISETTEI;
			SetChange = 1;
			break;
		case KEY_TEN:						/* テンキー: */
			if (pos < 2) {
				break;
			}
			if (msg != KEY_TEN0) {
			// KEY0は受け付けない
				BUZPI();
				in_now = (short)(msg - KEY_TEN0);
				// 入力値を表示する（反転表示）
				opedsp(6, 16, (ushort)in_now, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			}
			break;
		case KEY_TEN_CL:					/* C: */
			if (pos < 2) {
				break;
			}
			BUZPI();
			opedsp(6, 16, num, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			in_now = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車証明書メニュー				 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkingCertificate( void )                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned short	UsMnt_ParkingCertificate(void)
{
	unsigned short	usUmnyEvent;
	char	wk[2];
	char	cnt;

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[37]);		//	"＜駐車証明書＞"

		cnt = (prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 )? 1 : 2;
		usUmnyEvent = Menu_Slt( PARKING_CERTIFICATE_MENU, USM_CERTI_TBL, cnt, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){
		case MNT_CERTI_SET:
			usUmnyEvent = UsMnt_ParkingCertificateSet();
			break;

		case MNT_CERTI_ISSU:
			usUmnyEvent = UsMnt_AreaSelect(MNT_UKERP);
			break;

		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			return MOD_EXT;

		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			OpelogNo = 0;
		}

		if (usUmnyEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CHG;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			入力日付のチェック
//[]----------------------------------------------------------------------[]
///	@param[]		event	: 0:特別日／特別期間 1:券期限
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			matsushima
///	@date			Create	: 2014/11/07<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	UsMnt_datecheck( uchar event )
{

	switch( event ){
	case 0:
		splday_check();							// 特別日／特別期間の入力日付のチェック
		break;

	case 1:
		tickvalid_check();						// 券期限の入力日付のチェック
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------------------
// 入力日付のチェックを行い、NGの場合は削除する。
//--------------------------------------------------------------------------
static void tickvalid_check( void )
{

	TIC_PRM	*ptic;
	TIC_PRM	tic;
	char	no;				/* 期限番号(1-3) */

	ptic = (TIC_PRM *)&CPrmSS[S_DIS][8];
	memcpy( &tic, &tick_valid_data.tic_prm, sizeof(TIC_PRM) );
	no = tick_valid_data.no;

	if(( no < 1)||( no > 3 )){
		return;				// 期限番号 範囲外
	}

	// 券期限の日付チェック
	if( NG == ticval_date_chk( tic.date[0] ) ){		// 変更前開始日付ﾁｪｯｸ
		tic.date[0] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[1] ) ){		// 変更前終了日付ﾁｪｯｸ
		tic.date[1] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[2] ) ){		// 変更後開始日付ﾁｪｯｸ
		tic.date[2] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[3] ) ){		// 変更後終了日付ﾁｪｯｸ
		tic.date[3] = 0;
	}

	// パラメータ更新
	ptic[no-1] = tic;

}

// MH321800(S) D.Inaba ICクレジット対応
/*[]----------------------------------------------------------------------[]*/
/*| 　　　　　　リーダメンテナンスデータ送信（音量テスト）			       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ECVoiceGuide_VolumeTest				                   |*/
/*| PARAMETER    : vol		: 音量					                       |*/
/*| RETURN VALUE : ret  	: 1 (失敗成功どちらとも)                       |*/
/*| 					  	: MOD_CHG			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-06-13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static ushort ECVoiceGuide_VolumeTest( uchar vol ){

	ushort		msg;
	ushort		ret = 1;
	uchar		Receiveflag = 0;	// 結果受信フラグ　0 = 受信中、　1 = 受信OK
	uchar		funflag = 0;		// ファンクションキー表示フラグ
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
	uchar		lcd_flag = 0;		// 切断検知用のフラグ
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
	// 決済リーダ初期化OK？(決済リーダ起動シーケンス起動済でなければコマンドは送信しない)
	if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
		return ret;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[179]);	// " 再生中です。　　　　　　　　 "

	// 初期化
	memset(&MntTbl, 0x0, sizeof(EC_MNT_TBL));

	MntTbl.cmd = 1;						// 音量テストセット
	MntTbl.vol = (uchar)(vol);			// 音量セット
	if(MntTbl.vol > EC_MAX_VOLUME) {
		MntTbl.vol = EC_MAX_VOLUME;
	} 

	// リーダメンテナンスデータ送信（音声テスト）
	Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
	// タイムアウト設定
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	for( ; ; ) {
		// 結果受信待ち中は表示しない
		if(Receiveflag && funflag){
			Fun_Dsp( FUNMSG[8] );					//"                    　 終了  "
			funflag = 0;
		}

		msg = StoF(GetMessage(), 1);
		
		switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:							// 終了(F5)
				// リーダから返答または、タイムアウトするまで終了させない
				if(Receiveflag){
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return ret;
				}
				break;
			case EC_MNT_UPDATE:
				if(!Receiveflag){
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
					Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
					Receiveflag = 1;
					funflag = 1;
					// 音声テストでない時、何もしない。
					if( RecvMntTbl.cmd != 1){
						BUZPIPI();
						grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[178]);	// "  ＜＜再生に失敗しました＞＞  "
						break;
					}
					grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[177]);	// "  ＜＜再生に成功しました＞＞  "
				}
				break;
			case EC_CMDWAIT_TIMEOUT:
				if(!Receiveflag){
					Receiveflag = 1;
					funflag = 1;
					BUZPIPI();
					grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[178]);	// "  ＜＜再生に失敗しました＞＞  "
				}
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				lcd_flag = 1;
				break;
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
		// 切断待ち且つ、受信済み
		if( lcd_flag && Receiveflag ){
			Lagcan( OPETCBNO, 6 );
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 　　　　　　			音量変更データ送信					           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : VoiceGuide_VolumeChange				                   |*/
/*| PARAMETER    : void			     				                       |*/
/*| RETURN VALUE : ret	: 1 = 失敗                                         |*/
/*|						: 0 = 成功　（現状、両者同じ処理）                 |*/
/*|						: 2 = 未初期化　コマンド送信しない                 |*/
/*|						: 9 = 切断通知                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-03-1                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static char VoiceGuide_VolumeChange( void ){

	ushort		msg;
	char		ret = 0;
	uchar		loop = 0;

	// 決済リーダ初期化OK？(決済リーダ起動シーケンス起動済でなければコマンドは送信しない)
	if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
		BUZPI();
		ret = 2;
		return ret;
	}

// MH810103 GG119202(S) 音量変更中の表示修正
	// 音量変更中は8行目をクリアする
	Fun_Dsp( FUNMSG[0] );	// "　　　　　　　　　　　　　　　"
// MH810103 GG119202(E) 音量変更中の表示修正

	// 音量変更データセット
	SetEcVolume();

	// 音量変更データ送信
	Ec_Pri_Data_Snd( S_VOLUME_CHG_DATA, 0 );
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */
	loop = 1;

	// 返答かタイムアウトが来るまで待つ
	while(loop){
		msg = StoF(GetMessage(), 1);

		switch (KEY_TEN0to9(msg)) {
			case EC_VOLUME_UPDATE:
				// 0：変更OK、1：変更NG
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if(!RecvVolumeRes){
				if(!ECCTL.RecvVolumeRes){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					BUZPI();
					ret = 0;
				} else {
					BUZPIPI();
					ret = 1;
				}
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
				loop = 0;
				break;
			case EC_CMDWAIT_TIMEOUT:
				BUZPIPI();
				ret = 1;
				loop = 0;
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = 9;
				loop = 0;
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
			default:
				break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			音声案内時間メニュー
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change
//							MOD_EXT : F5 key
//	@author			Inaba
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]

struct GUIDE_TIME_EC {
	struct GUIDE_TIME gt[3];
	char  volume_reader[3];		// リーダ音量
};

ushort	UsMnt_ECVoiceGuide(void)
{
	ushort	pos;
	ushort	msg;
	char 	ptn;							// 音量設定パターン数
	ushort	ret = 0;
	char	str_vol[15];					// 音量表示変数 精算機/リーダ
	uchar	disp_flag = 1;
	uchar	disp_item_flag = 1;
	char 	volume_Actuary;					// 精算機の音量
	char 	volume_Reader;					// リーダの音量

// MH810103 GG119202(S) 終日固定音量設定状態で時間帯音量切替画面に遷移し変更なしで戻ると終日固定音量が選択されている(#5094)
	// 音量設定数を取得 (音量パターン設定パラメータの位置は変化なし)
	ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (MAX_VOICE_GUIDE < ptn) {
		// 最大設定数は3
		ptn = MAX_VOICE_GUIDE;
	}
	// 設定数が1以下なら終日固定、2以上なら時間帯別にカーソルを合わせる
	pos = (ptn != 0);
// MH810103 GG119202(E) 終日固定音量設定状態で時間帯音量切替画面に遷移し変更なしで戻ると終日固定音量が選択されている(#5094)
	for( ; ; ) {

		// ＜音声案内時間＞画面表示
		if ( 1 == disp_flag){
			// 音量設定数を取得 (音量パターン設定パラメータの位置は変化なし)
			ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
			if (MAX_VOICE_GUIDE < ptn) {
				// 最大設定数は3
				ptn = MAX_VOICE_GUIDE;
			}
			// 精算機の音量取得
			volume_Actuary = get_volume(ptn);
			// リーダの音量取得
			volume_Reader = get_volume_EC(ptn);
			// 設定数が1以下なら終日固定、2以上なら時間帯別にカーソルを合わせる
// MH810103 GG119202(S) 終日固定音量設定状態で時間帯音量切替画面に遷移し変更なしで戻ると終日固定音量が選択されている(#5094)
//			// 設定数が1以下なら終日固定、2以上なら時間帯別にカーソルを合わせる
//			pos = (ptn != 0);
// MH810103 GG119202(E) 終日固定音量設定状態で時間帯音量切替画面に遷移し変更なしで戻ると終日固定音量が選択されている(#5094)

			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "＜音声案内時間＞　　　　　　　"
			grachr(1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[130]);		// "現在の設定：
			if (ptn == 0) {
				grachr(1, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);	// 終日固定音量
			} else {
				grachr(1, 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);	// 時間帯別音量切替
			}
			memset(str_vol, 0, sizeof(str_vol));
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[170]);		// 現在の音量：精算機／リーダ
			sprintf(str_vol, "　%02d　／　%02d　", (int)volume_Actuary, (int)volume_Reader);
			grachr(3, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_vol);

			Fun_Dsp( FUNMSG[99] );	//"　▲　　▼　テスト 書込  終了 "
			disp_flag = 0;
		}
		if ( 1 == disp_item_flag){
			grachr(5, 6, 12, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);		// 終日固定音量
			grachr(6, 6, 16, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);		// 時間帯別音量切替
			disp_item_flag = 0;
		}
		
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
			Lagcan(OPETCBNO, 6);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
			return MOD_CHG;
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
			return MOD_EXT;
		case KEY_TEN_F4:							// 読出(F4)
			BUZPI();
			if (pos == 0) {
				// 終日固定音量
				// 音声変更機種選択画面
				ret = ECVoiceGuide_SelectTarget(&volume_Actuary, &volume_Reader, 0, 0 );
			} else {
				// 時間帯別音量切替
				ret = UsMnt_ECVoiceGuideTimeExtra(&volume_Actuary, &volume_Reader);
				// 設定数を確認
				ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053⑥
				volume_Actuary = get_volume(ptn);
				volume_Reader = get_volume_EC(ptn);
			}
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
				OpelogNo = 0;
				SetChange = 1;					// FLASHｾｰﾌﾞ指示
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
				mnt_SetFtpFlag( FTP_REQ_NORMAL ); // FTP更新フラグ
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
			}
			// メンテナンスOFF
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(メンテナンス)
//			if( ret == MOD_CHG ){
//				return MOD_CHG;
//			}
			if( ret == MOD_CHG || ret == MOD_CUT ){
				return ret;
			}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(メンテナンス)
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F3:							// (F3) テスト
			BUZPI();
			Ope_DisableDoorKnobChime();
			// 音声テスト機種選択画面を表示
			ret = ECVoiceGuide_SelectTarget(&volume_Actuary, &volume_Reader, ptn, 1 );
			// メンテナンスOFF
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F1:							// (F1)
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// 終日・時間帯のカーソル移動
			pos ^= 1;
			disp_item_flag = 1;
			break;
		default:
			break;
		}
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			時間帯別,リーダの音量取得
//[]----------------------------------------------------------------------[]
//	@return			vol		:	音量
//	@param[in]		num		:	設定数
//	@author			Inaba
//	@note			get_volume(char num)と同様
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_volume_EC(char num)
{
	char ptn, i, vol, pos;
	short nowtime, settime1, settime2;
	
	nowtime = (short)CLK_REC.hour * 100 + (short)CLK_REC.minu;	// 現在時刻
	// 開始時間のパターンを取得
	if (num <= 1) {
		ptn = 1;				// 設定数が1以下はパターン1
	} else {
		for (i = 0; i < num - 1; i++) {
			// 現在時刻が設定パターンiとi+1の間にあるか判定する
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054～
			if (settime1 <= nowtime) {
				settime2 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 55), 4, 1);
				if (settime1+1 < settime2){
					if (nowtime < settime2) {
						ptn = (char)(i + 1);
						break;
					}
				}else{
					ptn = (char)(i + 1);
					break;
				}
			}
		}
		if (i == num - 1) {
			ptn = num;
		}
	}
	// 音量取得
	switch (ptn) {
	case 1:	pos = 5;	break;
	case 2:	pos = 3;	break;
	case 3:	pos = 1;	break;
	default:			return 0;
	}
	vol = (char)prm_get(COM_PRM, S_ECR, 27, 2, pos);
	
	return vol;
}

//[]----------------------------------------------------------------------[]
//	@brief			ユーザーメンテナンス：対象選択
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume_Actuary	精算機音量値格納領域へのポインタ
//					volume_Reader	リーダ音量格納領域へのポインタ
//					nset			音量設定パターン数
//					swit			0: 音声変更
//									1: 音声テスト
//	@return			ret		MOD_CHG : mode change
//							0		: 正常終了
//							1		: 正常終了(書き込みあり)
//	@author			Inaba
//	@note
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort ECVoiceGuide_SelectTarget(char *vol_actuary, char *vol_reader, ushort ptn, ushort swit )
{

	char 	volume_Actuary;					// 精算機の音量
	char 	volume_Reader;					// リーダの音量
	uchar	pos = 0;						// カーソル位置
	ushort	msg;
	ushort	disp_item_flag = 1;
	ushort	disp_all = 1;
	ushort	ret = 0;
// MH810100(S) Y.Yamauchi 2020/02/28 車番チケットレス(メンテナンス)
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/28 車番チケットレス(メンテナンス)
	// 精算機の音量取得
	volume_Actuary = *vol_actuary;
	// リーダの音量取得
	volume_Reader = *vol_reader;

	for( ; ; ) {

		if(disp_all){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "＜音声案内時間＞　　　　　　　"
			if(!swit){
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[171]);	// "音量変更の対象を選択して下さい。"
				Fun_Dsp( FUNMSG[25] );	//"　▲　　▼　　　　 読出  終了 "
			}
			else{
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[172]);	// "音声テストを実行します。"
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[173]);	// "対象を選択して下さい。"
				Fun_Dsp( FUNMSG[120] );	//"　▲　　▼　　テスト　　   終了 "
			}
			disp_all = 0;
		}

		if ( 1 == disp_item_flag){
			grachr(4, 12, 6, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[174]);		// 精算機
			grachr(5, 10, 10, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[175]);		// 決済リーダ
			disp_item_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
				Lagcan(OPETCBNO, 6);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
// MH810100(S) Y.Yamauchi 20191220 車番チケットレス(メンテナンス)
//				ope_anm( AVM_STOP );
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191220 車番チケットレス(メンテナンス)
				Lagcan(OPETCBNO, 6);
				return MOD_CHG;
			case KEY_TEN_F5:							// 終了(F5)
				BUZPI();
// MH810100(S) Y.Yamauchi 20191220 車番チケットレス(メンテナンス)
//				ope_anm( AVM_STOP );
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191220 車番チケットレス(メンテナンス)
				Lagcan(OPETCBNO, 6);
				return ret;
			case KEY_TEN_F4:							// 読出(F4)
				// 	音量テストの場合、何もしない
				if(swit == 1){
					continue;
				}
				BUZPI();
				if (pos == 0) {
					//  精算機選択 終日固定音量
					ret = UsMnt_ECVoiceGuideTimeAllDay(&volume_Actuary, ptn, 0);
				} else {
					// リーダ選択 終日固定音量
					ret = UsMnt_ECVoiceGuideTimeAllDay(&volume_Reader, ptn , 1);
				}
				Ope_EnableDoorKnobChime();
				// メンテナンスOFF
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(メンテナンス)
//				if( ret == MOD_CHG ){
//					return MOD_CHG;
//				}
				if( ret == MOD_CHG || ret == MOD_CUT ){
					return ret;
				}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(メンテナンス)
				disp_item_flag = 1;
				disp_all = 1;
				break;
			case KEY_TEN_F3:							// (F3)
				// 	音声変更場合、何もしない。
				if(swit == 0){
					continue;
				}
				if(pos == 0){
					// 精算機選択　音声テスト
					if (volume_Actuary == 0) {	// 0ならピィピィ
						BUZPIPI();
						continue;
					}
					Ope_DisableDoorKnobChime();
					// 音声終了イベントが無いので、アラーム監視タイマ間はアラームを抑止する
					Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);

// MH810100(S) Y.Yamauchi 2020/02/25 車番チケットレス(メンテナンス)
//					VoiceGuideTest((char)volume_Actuary);	 // 「ご利用ありがとうございました。またの～」
					lcd_volume = Anounce_volume( (uchar)volume_Actuary );
					PKTcmd_audio_volume( lcd_volume );		// アナウンス音量をLCDに送信
// MH810100(E) Y.Yamauchi 2020/02/25 車番チケットレス(メンテナンス)
					
				} else {
					// リーダ選択　音声テスト
					BUZPI();
					ret = ECVoiceGuide_VolumeTest((char)volume_Reader);
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
					if( ret == MOD_CUT ){
						return MOD_CUT;
					}
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
					// メンテナンスOFF(扉を閉めていたら)
					if( !OPE_SIG_DOORNOBU_Is_OPEN ){
						return MOD_CHG;
					}
					disp_all = 1;
				}
				disp_item_flag = 1;
				break;
			case KEY_TEN_F1:							// (F1)
			case KEY_TEN_F2:							// (F2)
				BUZPI();
				// 精算機・リーダ　カーソル移動
				pos ^= 1;
				disp_item_flag = 1;
				break;
			default:
				break;
		}
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			終日固定音量設定（ECリーダ接続時）
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume	音量値格納領域へのポインタ
//	@param[in]		ptn		0: 終日固定音量設定
//							1: パターン１（時間帯別音量切替）
//							2: パターン２（時間帯別音量切替）
//							3: パターン３（時間帯別音量切替）
// 					target  0: 精算機
//							1: リーダ
//	@return			ret		MOD_CHG : mode change
//							0		: 正常終了
//							1		: 正常終了(書き込みあり)
//	@author			Inaba
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_ECVoiceGuideTimeAllDay(char *volume, ushort ptn, uchar target)
{
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_item_flag = 1;
	uchar	disp_all = 1;
	uchar	result = 2;  // 0 = OK, 1 = NG, 2 = 何も表示しない(消す)
	uchar	max_vol;
	uchar	min_vol;
	ushort	vol;
	long	*pvol;							// 精算機音量
	uchar	each_volume[ MAX_VOICE_GUIDE ];	/*	パターン１～３の各音量
											 *	each_volume[0]:パターン１（時間帯別音量切替）／終日固定音量
											 *	each_volume[1]:パターン２（時間帯別音量切替）
											 *	each_volume[2]:パターン３（時間帯別音量切替）
											 */
	char	f_write = 0;
	ushort	column;							// 描画位置
// MH810100(S) Y.Yamauchi 2020/02/28 車番チケットレス(メンテナンス)
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/28 車番チケットレス（メンテナンス
	
	if (MAX_VOICE_GUIDE < ptn) {
		// 最大設定数は3
		ptn = MAX_VOICE_GUIDE;
	}
	// 精算機の音量取得
// MH810103 GG119202(S) 現在時間帯の音量が表示されない
	vol = (ushort)*volume;
// MH810103 GG119202(E) 現在時間帯の音量が表示されない
	if(target == 0){
// MH810103 GG119202(S) 現在時間帯の音量が表示されない
//		vol = get_volume(ptn);
// MH810103 GG119202(E) 現在時間帯の音量が表示されない
		pvol = &CPrmSS[S_SYS][51];			// 01-0051
		max_vol = (uchar)ADJUS_MAX_VOLUME;
		min_vol = (uchar)ADJUS_MIN_VOLUME;
	} else {
// MH810103 GG119202(S) 現在時間帯の音量が表示されない
//		vol = get_volume_EC(ptn);
// MH810103 GG119202(E) 現在時間帯の音量が表示されない
		pvol = &CPrmSS[S_ECR][27];			// 50-0027
		max_vol = (uchar)EC_MAX_VOLUME;
		min_vol = (uchar)EC_MIN_VOLUME;
	}

	// 現在設定されている音量を記憶
	memset( each_volume, 0, sizeof(each_volume) );
	each_volume[ 0 ] = (uchar)( ( *pvol / 10000 ) % 100 );	// パターン１の音量を取り出す
	each_volume[ 1 ] = (uchar)( ( *pvol / 100 ) % 100 );	// パターン２の音量を取り出す
	each_volume[ 2 ] = (uchar)( *pvol % 100 );				// パターン３の音量を取り出す

	if (vol > max_vol) {
		// 最大値より大きい値の場合、最大値に変更する
		vol = max_vol;
	}
	if (vol < min_vol) {
		// 最小値より小さい値の場合、最小値に変更する
		vol = min_vol;
	}

	for( ; ; ) {

		if(disp_all){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
			if( target == 0 ){
				// 精算機選択？
				grachr(1, 0, 6, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[174]);	// "精算機"
// MH810103 GG119202(S) 表示文字桁あふれ修正
//				grachr(1, 6, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "　　　の音量を変更します"
				grachr(1, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "　　　の音量を変更します"
// MH810103 GG119202(E) 表示文字桁あふれ修正
			}
			else {
				// 決済リーダ選択？
				grachr(1, 0, 10, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[175]);	// "決済リーダ"
// MH810103 GG119202(S) 表示文字桁あふれ修正
//				grachr(1, 10, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "　　　    の音量を変更します"
				grachr(1, 10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "　　　    の音量を変更します"
// MH810103 GG119202(E) 表示文字桁あふれ修正
			}
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[136]);	// ⊂⊃キーで音量を設定して下さい
			grachr(4, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[137]);	// 音量設定：　　　　　　　　　　
			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[138]);	// 1　　　　　　　　　　　　　 16
			disp_all = 0;
		}

		// ＜音声案内時間＞画面表示
		if ( 1 == disp_item_flag ){
			if(result == 2){
			grachr(3, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, UMSTR3[154]);
		}	
			opedsp(4, 10, vol, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// 音量
			if ( vol <= max_vol ) {
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor( COLOR_DARKORANGE );
// MH810100(E) S.Takahashi 2019/12/25
				column = (ushort)(vol*2);
				if(column) {
					grachr( 6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);		// プログレスバー表示
				}
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
// MH810100(E) S.Takahashi 2019/12/25
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
			}
			if ( vol > 0) {
				Fun_Dsp( FUNMSG[100] );
			} else {
				Fun_Dsp( FUNMSG[13] );
			}
			disp_item_flag = 0;
		}

		if(result != 2){
			// result = 0   書込に成功しました。
			// result = 1   書込に失敗しました。
			grachr(3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[(182 + result)]);
			result = 2;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
			case KEY_TEN_F5:							// 終了(F5)
				BUZPI();
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				ope_anm(AVM_STOP);
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
				Lagcan(OPETCBNO, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				if (msg == KEY_MODECHG) {
//					return MOD_CHG;
//				} else {
//					return ret;
//				}
				if (msg == KEY_MODECHG) {
					return MOD_CHG;
				} else if (msg == LCD_DISCONNECT) {
					return MOD_CUT;
				}else{
					return ret;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			
			case KEY_TEN_F4:							// 書込(F4)
				if(target == 0){
					BUZPI();
				}
				// 音量書込み
				f_write = 1;
				*volume = (char)vol;
				switch ( ptn ) {
					case 0:
						*pvol = (long)( (long)*volume * 10000L );
						break;
					case 1:
						*pvol = (long)(( (long)*volume * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)each_volume[ 2 ]);
						break;
					case 2:
						*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)*volume * 100 ) + (long)each_volume[ 2 ]);
						break;
					case 3:
						*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)*volume);
						break;
					default:
						*pvol = 0;	// ptnが範囲外の場合は０にしておく
						break;
				}
				// 操作履歴登録・FLASHにセーブ
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				if (f_write == 1 && ptn == 0) {
					CPrmSS[S_SYS][53] = 0;			// 01-0053
					CPrmSS[S_SYS][54] = 0;			// 01-0054
					CPrmSS[S_SYS][55] = 0;			// 01-0055
					CPrmSS[S_SYS][56] = 0;			// 01-0056
				}
				if(target == 1){	// リーダ
					result = VoiceGuide_VolumeChange( );
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
					if( result == 9 ){
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
					// メンテナンスOFF(扉を閉めていたら)
					if( !OPE_SIG_DOORNOBU_Is_OPEN ){
						return MOD_CHG;
					}
					disp_all = 1;
					disp_item_flag = 1;
				}
				ret = 1;
// MH810100(S) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
 				mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
				break;
			case KEY_TEN_F3:	/* (F3) */
				// 音声を流す
				if (vol != 0) {		// 0なら何もしない
					if(target == 0){	// 精算機
						Ope_DisableDoorKnobChime();
						// 音声終了イベントが無いので、アラーム監視タイマ間はアラームを抑止する
						Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 車番チケットレス(メンテナンス)
//						VoiceGuideTest((char)vol);	// 「ご利用ありがとうございました。またの～」
						lcd_volume = Anounce_volume( (uchar)vol );
						PKTcmd_audio_volume( lcd_volume );		// アナウンス音量をLCDに送信
// MH810100(E) Y.Yamauchi 2020/02/25 車番チケットレス(メンテナンス)
					} else {	//　リーダ
						Ope_DisableDoorKnobChime();
						BUZPI();
						ret = ECVoiceGuide_VolumeTest((char)vol);
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
						if( ret == MOD_CUT ){
							return MOD_CUT;
						}
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
						// メンテナンスOFF(扉を閉めていたら)
						if( !OPE_SIG_DOORNOBU_Is_OPEN ){
							return MOD_CHG;
						}
						disp_item_flag = 1;
						disp_all = 1;
					}
				}
				break;
			case KEY_TEN_F1:							// (F1)
				BUZPI();
				// プログレスバー左へ移動
				if (vol > min_vol) {
					vol--;
					disp_item_flag = 1;
				}
				break;
			case KEY_TEN_F2:							// (F2)
				BUZPI();
				// プログレスバー右へ移動
				if (vol < max_vol) {
					vol++;
					disp_item_flag = 1;
				}
				break;
			default:
				break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			時間帯別音量切替設定・書込結果画面（ECリーダ）
//[]----------------------------------------------------------------------[]
//	@param[in,out]	result	0	: 成功
//							1	: 失敗
//							2	: 表示しない
//	@return			ret		0	: 遷移元に遷移する。
// 							9	: 切断通知
//	@author			Inaba
//	@note			None
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort ECVoiceGuideTimeExtra_Resultdisplay(uchar result)
{
	ushort	msg;
	ushort	ret = 1;

	if( result == 2 ){
		ret = 0;
		return ret;
	}
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
	else if( result == 9 ){
		return MOD_CUT;
	}
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
	grachr(2, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[182 + result]);
	// result == 0  " ＜＜書込に成功しました。＞＞ "
	// result == 1  " ＜＜書込に失敗しました。＞＞ "
		
	Fun_Dsp( FUNMSG[8] );	//"                    　 終了  "

	while(ret) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/20 車番チケットレス(メンテナンス)
//			ope_anm(AVM_STOP);
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 2019/12/20 車番チケットレス(メンテナンス)
			Lagcan(OPETCBNO, 6);
			ret = 0;
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
			break;
		case LCD_DISCONNECT:						// 切断通知
			PKTcmd_audio_end( 0, 0 );				// アナウンス終了要求
			Lagcan(OPETCBNO, 6);
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
		default:
			break;
		}
	}
	displclr(2);
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			時間帯別音量切替設定（ECリーダ）
//[]----------------------------------------------------------------------[]
//	@param[in,out]	vol_actuary	精算機音量値格納領域へのポインタ
//					vol_reader	リーダ音量値格納領域へのポインタ
//	@return			ret		1	: 終日固定画面に移行（未使用,コメント参照）
//							0	: 正常終了
//	@author			Inaba
//	@note			None
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/25<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort	UsMnt_ECVoiceGuideTimeExtra(char *vol_actuary, char *vol_reader)
{
	ushort	msg;
	uchar	disp_flag = 1;
	uchar	value_ref = 1;
	ushort	pos_l = 0;
	ushort	pos_c = 0;	// 0:時,1:分
	struct	GUIDE_TIME_EC guideTime;
	char	str_vol[6];
	ushort	tNum;
	ushort	num;
	ulong	tmpvol_actuary;
	ulong	tmpvol_reader;
	ushort	calc_flag = 0;
	char	cl_flag = 1;
	ushort	i;
	ushort	ret = 0;
	char	flag;
	ushort	tmInput;
	long	*ptim, *ptim_reader;
	const char prm_pos[3] = {5, 3, 1};
	ptim = CPrmSS[S_SYS];				// 精算機音量 音量設定パターン
	ptim_reader = CPrmSS[S_ECR];		// リーダ音量

	// 設定数取得
	tNum = (short)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (tNum == 0) {
		tNum = 1;
	} else if (tNum > MAX_VOICE_GUIDE) {
		tNum = MAX_VOICE_GUIDE;
	}
	num = tNum;

	// 初期化
	memset(&guideTime, 0, sizeof(struct GUIDE_TIME_EC));
	
	for (i = 0; i < num; i++) {
		// 開始時刻 01-0054,01-0055,01-0056
		guideTime.gt[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// 時
		guideTime.gt[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// 分
		// 音量 01-0051①②,③④,⑤⑥
		guideTime.gt[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
		// 音量 50-0027①②,③④,⑤⑥
		guideTime.volume_reader[i] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[i]);
		value_ref = 0;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "＜音声案内時間＞　　　　　　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);	// No  開始時刻   終了時刻	 音量

	tmInput = guideTime.gt[0].sTime[0] * 100 + guideTime.gt[0].sTime[1];
	while (1) {
		if(value_ref){
			for (i = 0; i < num; i++) {
			// 開始時刻 01-0054,01-0055,01-0056
			guideTime.gt[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// 時
			guideTime.gt[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// 分
			// 音量 01-0051①②,③④,⑤⑥
			guideTime.gt[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
			// 音量 50-0027①②,③④,⑤⑥
			guideTime.volume_reader[i] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[i]);
			value_ref = 0;
			}
		}

		if ( 1 == disp_flag ){
			calc_flag = Calc_tmset_sc(num, guideTime.gt);
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);														// No  開始時刻   終了時刻	 音量
			for( i = 0; i < MAX_VOICE_GUIDE ; ++i ){
				if( i < num ){
					if( guideTime.gt[i].eTime[1] == -1 ){
						grachr((ushort)(2 + i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);								// 　 －－：－－ －－：－－　－－
					}
					flag = (i == pos_l) && (0 == pos_c);
					opedsp((ushort)(i + 2), 3,	(ushort)guideTime.gt[i].sTime[0], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // 　 ＊＊：－－ －－：－－　－－
					grachr((ushort)(i + 2), 7,	2, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 	// コロンを反転
					opedsp((ushort)(i + 2), 9,	(ushort)guideTime.gt[i].sTime[1], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // 　 －－：＊＊ －－：－－　－－
					if( guideTime.gt[i].eTime[1] != -1 ){
						opedsp((ushort)(i + 2), 14, (ushort)guideTime.gt[i].eTime[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 　 －－：－－ ＊＊：－－　－－
						grachr((ushort)(i + 2), 18,	2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 			// コロンを正転表示
						opedsp((ushort)(i + 2), 20, (ushort)guideTime.gt[i].eTime[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// 　 －－：－－ －－：＊＊　－－
					}
					memset(str_vol, 0, sizeof(str_vol));
					sprintf(str_vol, "%02d/%02d", (int)guideTime.gt[i].volume, (int)guideTime.volume_reader[i]);
					grachr((ushort)(i + 2), 25, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_vol);				// 　 －－：－－ －－：－－　＊＊
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//＊＊－－：－－ －－：－－　－－
				}else{
					grachr((ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);									// 　 －－：－－ －－：－－　－－
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//＊＊－－：－－ －－：－－　－－
				}
			}

			if( tNum > 1 && (pos_l + 1) == tNum ){
				Fun_Dsp( FUNMSG[101] );					// "　音量　　▼　　 削除 　書込　　終了　"
			}else{
				Fun_Dsp( FUNMSG[102] );					// "　音量　　▼　　　　　　書込　　終了　"
			}
			disp_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:							// 終了(F5)
			BUZPI();
			return ret;
		case KEY_TEN_F4:							// 書込(F4)
			if ( 1 == calc_flag ){
				// 時間を書込み
				ptim[54 + pos_l] = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				if( tNum < pos_l + 1 ){
					tNum = pos_l + 1;
				}
				ptim[53] = tNum;

				// 音量の継承時の値のまま「書込」した場合には、その音量で確定させる
				switch (pos_l) {
					case 0:
						// 精算機音量保存
						tmpvol_actuary = ptim[51] % 10000;
						tmpvol_actuary += (ulong)(guideTime.gt[pos_l].volume) * 10000;
						// リーダ音量保存
						tmpvol_reader = ptim_reader[27] % 10000;
						tmpvol_reader += (ulong)(guideTime.volume_reader[pos_l]) * 10000;
						break;
					case 1:
						// 精算機音量保存
						tmpvol_actuary  = ptim[51] / 10000;
						tmpvol_actuary *= 10000;
						tmpvol_actuary += ptim[51] % 100;
						tmpvol_actuary += (ulong)(guideTime.gt[pos_l].volume) * 100;
						// リーダ音量保存
						tmpvol_reader  = ptim_reader[27] / 10000;
						tmpvol_reader *= 10000;
						tmpvol_reader += ptim_reader[27] % 100;
						tmpvol_reader += (ulong)(guideTime.volume_reader[pos_l]) * 100;
						break;
					case 2:
						// 精算機音量保存
						tmpvol_actuary  = ptim[51] / 100;
						tmpvol_actuary *= 100;
						tmpvol_actuary += guideTime.gt[pos_l].volume;
						// リーダ音量保存
						tmpvol_reader  = ptim_reader[27] / 100;
						tmpvol_reader *= 100;
						tmpvol_reader += guideTime.volume_reader[pos_l];
						break;
					default:
						tmpvol_actuary = 0;
						tmpvol_reader = 0;
						break;
				}
				ptim[51] = tmpvol_actuary;
				ptim_reader[27] = tmpvol_reader;
				// 確定したときには一つ前の音量を継承して表示
				if ( pos_l < MAX_VOICE_GUIDE ){
					if( !ptim[54 + pos_l + 1] )	{						// 次の開始時刻が未設定の場合
						guideTime.gt[pos_l+1].volume = guideTime.gt[pos_l].volume;
						guideTime.volume_reader[pos_l+1] = guideTime.volume_reader[pos_l];
					}
				}
				// 操作履歴登録・FLASHにセーブ
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;									// 復電時にRAM上パラメータデータのSUM更新する
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				

				ret = VoiceGuide_VolumeChange( );
				ret = ECVoiceGuideTimeExtra_Resultdisplay(ret);
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
				if( ret == MOD_CUT ){
					return ret;
				}
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
				// メンテナンスOFF(扉を閉めていたら)
				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
					return MOD_CHG;
				}

				// カーソル移動
				++pos_l;
				if(pos_l < MAX_VOICE_GUIDE) {
					guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
					guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
				}
				disp_flag = 1;
				pos_c = 0;
				if ( pos_l > tNum || pos_l >= 3 ){
					pos_l = 0;
					if(num > 1 && tNum < 3 ){
						num--;
					}
				} else {
					if ( tNum <= pos_l ){
						num++;
					}
				}
				tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				cl_flag = 1;
				// 音量変更データ送信
			}
			break;
		case KEY_TEN_F2:							// (F2)
			// 最新の音量設定を画面に反映させる
			guideTime.gt[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]); // 精算機の音量
			guideTime.volume_reader[pos_l] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[pos_l]); // リーダの音量
			if ( pos_l+1 < MAX_VOICE_GUIDE ){
				if( !ptim[54 + pos_l + 1] )	{								// 次の開始時刻が未設定の場合
					guideTime.gt[pos_l+1].volume = guideTime.gt[pos_l].volume;	// 一つ前の精算機音量を継承
					guideTime.volume_reader[pos_l+1] = guideTime.volume_reader[pos_l];	// 一つ前のリーダ音量を継承
				}
			}
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
			BUZPI();
			
			++pos_l;
			if(pos_l < MAX_VOICE_GUIDE) {
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
			}
			disp_flag = 1;
			pos_c = 0;
			if ( pos_l > tNum || pos_l >= 3 ){
				pos_l = 0;
				if(num > 1 && tNum < 3 ){
					num--;
				}
			} else {
				if ( tNum <= pos_l ){
					num++;
				}
			}
			tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
			cl_flag = 1;
			break;		
		case KEY_TEN_F3:	// (F3)
			// 設定削除(開始時刻、音量をクリア、設定数を-1)
			if( tNum > 1 && (pos_l + 1) == tNum ){
				tNum--;
				ptim[53] = tNum;
				ptim[54 + pos_l] = 0;
				switch (pos_l) {
				case 0:
					tmpvol_actuary = ptim[51] % 10000;
					tmpvol_reader = ptim_reader[27] % 10000;
					break;
				case 1:
					/*
					 *	@note	パターン２を削除した時にパターン３の音量値を残しておかないようにする。（０クリアする）
					 */
					tmpvol_actuary = ( ptim[51] / 10000 ) * 10000;
					tmpvol_reader= ( ptim_reader[27] / 10000 ) * 10000;
					break;
				case 2:
					tmpvol_actuary  = ptim[51] / 100;
					tmpvol_actuary *= 100;
					tmpvol_reader  = ptim_reader[27] / 100;
					tmpvol_reader *= 100;
					break;
				default:
					tmpvol_actuary = 0;
					tmpvol_reader = 0;
					break;
				}
				ptim[51] = tmpvol_actuary;
				ptim[27] = tmpvol_reader;
				// 操作履歴登録・FLASHにセーブ
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
				
				ret = VoiceGuide_VolumeChange(  );
// MH810100(S) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
				if( ret == 9 ){
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2020/03/11 車番チケットレス(メンテナンス)
				// メンテナンスOFF(扉を閉めていたら)
				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
					return MOD_CHG;
				}

				// カーソル移動
				++pos_l;
				if(pos_l < MAX_VOICE_GUIDE) {
					guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
					guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
				}
				disp_flag = 1;
				pos_c = 0;
				if ( pos_l > tNum || pos_l >= 3 ){
					pos_l = 0;
					if(num > 1 && tNum < 3 ){
						num--;
					}
				} else {
					if ( tNum <= pos_l ){
						num++;
					}
				}
				tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				cl_flag = 1;
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// 音量設定画面へ
			ret = ECVoiceGuide_SelectTarget( &guideTime.gt[pos_l].volume, &guideTime.volume_reader[pos_l], (ushort)(pos_l + 1), 0 );
			if(ret == 1){
				value_ref = 1;
			}
			*vol_actuary = guideTime.gt[pos_l].volume;
			*vol_reader = guideTime.volume_reader[pos_l];

			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
				OpelogNo = 0;
				SetChange = 1;					// FLASHｾｰﾌﾞ指示
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
			}
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if(ret == MOD_CUT ){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			for (i = 1; i < 7; i++) {
				displclr(i);
			}
			disp_flag = 1;
			break;
		case KEY_TEN:								// 数字(テンキー)
			// 時間設定
			BUZPI();
			if( cl_flag ){
				tmInput = 0;
				cl_flag = 0;
			}
			tmInput = (tmInput % 1000) * 10 + msg - KEY_TEN0;
			guideTime.gt[pos_l].sTime[0] = tmInput / 100;
			guideTime.gt[pos_l].sTime[1] = tmInput % 100;
			disp_flag = 1;
			break;
		case KEY_TEN_CL:							// 取消(テンキー)
			BUZPI();
			// 開始時刻 01-0054,01-0055,01-0056
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// 時
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// 分
			tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
			// 音量 01-0051①②,③④,⑤⑥
			guideTime.gt[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			// 音量 50-0027①②,③④,⑤⑥
			guideTime.volume_reader[pos_l] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[pos_l]);
			disp_flag = 1;
			cl_flag = 1;
			break;
		default:
			break;
		}
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
//	@brief			ユーザーメンテナンス：拡張機能 - 決済リーダ処理メニュー
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change<br>
//							MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyMnu()移植
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019-01-29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	UsMnt_ECReaderMnu(void)
{
	ushort	usUcreEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();									// 画面クリア
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[167]);			// "＜決済リーダ処理＞　　　　　　"
		usUcreEvent = Menu_Slt( (void*)EXTENDMENU_EC, USM_EXTEC_TBL, (char)USM_EXTEC_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUcreEvent ){

		case MNT_ECR_SHUUKEI:					// 決済リーダ処理 - 日毎集計プリント
			usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_ECR_SHUUKEI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			break;

		case MNT_ECR_MEISAI:					// 決済リーダ処理 - 利用明細プリント
			usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_ECR_MEISAI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			break;

		case MNT_ECR_ALARM_LOG:					// 決済リーダ処理 - 処理未了取引記録
			usUcreEvent = UsMnt_EcReaderAlarmLog();
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			break;
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
		case MNT_ECR_MINASHI:					// 決済リーダ処理 - みなし決済プリント
			usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_ECR_MINASHI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
			break;
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける

		case MOD_EXT:		// 終了（Ｆ５）
			return(MOD_EXT);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case MOD_CUT:
			return(MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case MOD_CHG:		// モードチェンジ
			return(MOD_CHG);

		default:
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| 決済リーダ日毎集計数取得  		                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetEcShuukeiCnt                                        |*/
/*| PARAMETER    : s_date : ノーマライズした開始日付                       |*/
/*|              : e_date : ノーマライズした終了日付                       |*/
/*| RETURN VALUE : ushort   log_cnt 日毎集計数                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-01-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
ushort	GetEcShuukeiCnt(ushort s_date, ushort e_date)
{
	ushort			log_date;			// ログ格納日付
	ushort			log_cnt;			// 指定期間内ログ件数
	ushort			cnt;
	ushort			total;
	ushort			pos;				// 検索を始める配列の位置
	date_time_rec	*date_work;
	ushort			LOG_Date[3];		// ログ格納日付（[0]：年、[1]：月、[2]：日）


	total = Syuukei_sp.ec_inf.cnt - 1;
	log_cnt = 0;
	for (cnt = 1; cnt <= total; cnt++) {
		/* 次の参照位置へ移動 */
		pos = (Syuukei_sp.ec_inf.ptr + SYUUKEI_DAY_EC - cnt) % SYUUKEI_DAY_EC;
	
		/* Targetﾃﾞｰﾀの登録日時をﾉｰﾏﾗｲｽﾞ値でget */
		date_work = &Syuukei_sp.ec_inf.bun[pos].SyuTime;
		LOG_Date[0] = date_work->Year;
		LOG_Date[1] = date_work->Mon;
		LOG_Date[2] = date_work->Day;
		log_date = dnrmlzm((short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]) ;
		
		// 範囲に該当するかチェック
		if ((log_date >= s_date) && (log_date <= e_date)) {
			log_cnt++;
		}
	}
	return log_cnt;
}

//[]----------------------------------------------------------------------[]
//	@brief			ユーザーメンテナンス：決済リーダ処理 処理未了取引記録
//[]----------------------------------------------------------------------[]
//	@param[in]		None
//	@return			usSysEvent		MOD_CHG : mode change<br>
// 									MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyAlarmLog移植
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static	ushort UsMnt_EcReaderAlarmLog(void)
{
	ushort				msg;
	int					mode;				// 0:ログなし、1:ログ件数表示中、2:全データ削除確認中
											// 3:1件目プリント中、4:プリント中止要求可、5:プリント中止要求中
	int					disp;
	ushort				log_cnt;			// ログ件数
	ushort				pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	uchar				pri;				// 対象プリンタ(0/R_PRI/J_PRI/RJ_PRI)
	T_FrmEcAlarmLog		FrmPrnAlarmLog;		// 処理未了取引記録印字要求メッセージ
	T_FrmPrnStop		FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	int					cancel_key_flag;	// プリント中止キー押下フラグ


	// ログ件数取得
	mode = 0;
	log_cnt = EcAlarmLog_GetCount();
	if (log_cnt != 0) {
		mode = 1;
	}

	// タイトル表示
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[168]);		/* "＜処理未了取引記録＞　　　　　" */
	disp = 1;

	for ( ; ; ) {
		// 画面更新
		if (disp) {
			disp = 0;
			displclr(2);
			displclr(3);
			switch (mode) {
			case 0:		// ログなし
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);		// "　　　データはありません　　　"
				Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 "
				break;
			case 1:		// ログ件数あり				
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[169]);		// "　　　件のデータがあります　　"
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//				opedsp( 3, 4, log_cnt, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 対象ログ件数表示（０サプレス）
				opedsp( 3, 2, log_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 対象ログ件数表示（０サプレス）
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
				Fun_Dsp( FUNMSG2[63] );					// " ｸﾘｱ　　　　 ﾌﾟﾘﾝﾄ 　　　終了 "
				break;
			case 2:		// 全削除確認
				displclr(6);
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[61]);		// "　　 全データを削除します 　　"
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);		// "　　　 よろしいですか？ 　　　"
				Fun_Dsp( FUNMSG[19] );					// "　　　　　　 はい いいえ　　　"
				break;
			case 3:		// プリント中
				displclr(6);
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "　　　 プリント中です 　　　　"
				Fun_Dsp( FUNMSG[82] );					// "　　　　　　 中止 　　　　　　"
				break;
			case 5:		// プリント中止要求中
				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[14]);		// 	"　 　 印字を中止しました 　 　"
				Fun_Dsp( FUNMSG[0] );					//  "　　　　　　　　　 　　  　　 "
				break;
			default:
				break;
			}
		}
		
		// メッセージ処理
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			// ドアノブの状態にかかわらずトグル動作してしまうので、
			// ドアノブ閉かどうかのチェックを実施
// MH810103(s) 電子マネー対応 #5499 処理未了取引画面でドアノブ閉しても待機画面に遷移しない
//			if (mode == 3 && CP_MODECHG) {
			if (mode != 3 && CP_MODECHG) {
// MH810103(e) 電子マネー対応 #5499 処理未了取引画面でドアノブ閉しても待機画面に遷移しない
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			// BUZPI();
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			}
			break;
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)

		case KEY_TEN_F5:	/* 終了(F5) */
			if (mode >= 2) {
			// データなしorデータ件数表示中以外処理しない
				break;
			}

			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F4:	/* いいえ(F4) */
			if (mode != 2) {
			// 全削除確認中以外、処理しない
				break;
			}

			BUZPI();
			// ログクリアしない
			mode = 1;
			disp = 1;			// 画面更新
			break;

		case KEY_TEN_F3:	/* プリント(F3)、中止(F3)、はい(F3) */
			if (mode == 1) {
			// ログ件数表示中
				pri = R_PRI;
				if (check_printer(&pri) != 0) {
				// レシートプリンタ紙切れ or エラー発生中
					BUZPIPI();
					break;
				}
				BUZPI();
				cancel_key_flag = 0;
				/*------	処理未了取引記録印字要求ﾒｯｾｰｼﾞ送信	-----*/
// MH810105 GG119202(S) T合計連動印字対応
				memset(&FrmPrnAlarmLog, 0, sizeof(FrmPrnAlarmLog));
// MH810105 GG119202(E) T合計連動印字対応
				FrmPrnAlarmLog.prn_kind = R_PRI;						// ﾌﾟﾘﾝﾀ種別
				FrmPrnAlarmLog.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// 機械№
				pri_cmd = PREQ_EC_ALARM_LOG;
				queset( PRNTCBNO, pri_cmd, sizeof(T_FrmEcAlarmLog), &FrmPrnAlarmLog );
				mode = 3;
				disp = 1;
			}
			else if (mode == 2) {
			// 全削除確認中
				BUZPI();
				// ログクリア
				EcAlarmLog_Clear();
				log_cnt = 0;
				mode = 0;
				disp = 1;			// 画面更新
			}
			else if (mode == 3) {
			// 1件目プリント中
				if (!cancel_key_flag) {
					BUZPI();
					cancel_key_flag = 1;
				}
			}
			else if (mode == 4) {
			// プリント中止可
				/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
				FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
				queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
				BUZPI();
				mode = 5;
				disp = 1;
			}
			break;

		case KEY_TEN_F1:	/* クリア(F1) */
			if (mode != 1) {
			// ログ件数表示中以外、処理しない
				break;
			}

			BUZPI();
			mode = 2;
			disp = 1;			// 画面更新
			break;

		case OPE_EC_ALARM_LOG_PRINT_1_END:	/* 処理未了取引ログ1件印字完了 */
			if (mode == 3) {
				if (cancel_key_flag) {
					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					mode = 5;
					disp = 1;
				}
				else {
					mode = 4;
				}
			}
			break;

		default:
			// 印字メッセージ処理
			if (pri_cmd != 0) {
				if (msg == ( pri_cmd | INNJI_ENDMASK )) { 			// 印字終了ﾒｯｾｰｼﾞ受信？
					mode = 1;
					disp = 1;
				}
			}
			break;
		}
	} // end of for

	// 後処理
	// なし
}

//[]----------------------------------------------------------------------[]
//	@brief			プリンタエラーチェック
//[]----------------------------------------------------------------------[]
//	@param[in/out]	*pri_kind : 対象プリンタ(0/R_PRI/J_PRI/RJ_PRI)
//	@return			ret		0x00 = エラーなし
//	@return					0x01 = エラーあり（ジャーナル使用設定なしを含む）
//	@return					0x02 = 紙切れ（アラーム）あり
//	@author			Inaba
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29<br> 
//                          :   GM494202_494302のOpe_isPrinterErrotExist()をupdate
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	check_printer(uchar *pri_kind)
{
	uchar	ret;	// レシート
	uchar	ret2;	// ジャーナル
	uchar	back;	// 元の印字先

	back = *pri_kind;

	ret = 0;
	ret2 = 0;
	switch (*pri_kind) {
	case	RJ_PRI:
	case	R_PRI:
	// レシートプリンタチェック
		if( (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) ||		// Printer Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_HEADHEET)) ||		// Head Heet Up Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_CUTTER)) ) {			// Cutter       Error
			ret |= 0x01;
		}

		if( ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0 ){
 		// 紙切れ
			ret |= 0x02;
		}

		if (ret != 0) {
		// 紙切れ or エラー発生中
			*pri_kind &= (~R_PRI);		// 印字先：レシート解除
		}
		if (back == R_PRI) {
		// 印字先＝レシートならここでチェック終了
			break;
		}
	// no break;	// RJ_PRI
	case	J_PRI:
	// ジャーナルプリンタチェック
		if ((IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM)) ||		// Printer Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)) ) {		// Head Heet Up Error
			ret2 |= 0x01;
		}
// MH810104 GG119201(S) 電子ジャーナル対応
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL) ||			// 通信不良
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||	// SDカード使用不可
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||		// 書込み失敗
// MH810104 GG119201(S) 電子ジャーナル対応（E2204の名称変更）
//			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WAKEUP_ERR)) {		// 未接続
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)) {		// 未接続
// MH810104 GG119201(E) 電子ジャーナル対応（E2204の名称変更）
			ret2 |= 0x01;
		}
// MH810104 GG119201(E) 電子ジャーナル対応

		if (ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0) {
 		// 紙切れ
			ret2 |= 0x02;
		}

// MH810104 GG119201(S) 電子ジャーナル対応
		if (ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
			// SDカードエンド
			ret2 |= 0x02;
		}
// MH810104 GG119201(E) 電子ジャーナル対応

		if (ret2 != 0) {
		// 設定なし or 紙切れ or エラー発生中
			*pri_kind &= (~J_PRI);		// 印字先：ジャーナル解除
		}
		break;
	default :
		break;
	}

	return (uchar)(ret | ret2);
}
// MH321800(E) D.Inaba ICクレジット対応
// MH810104 GG119201(S) 電子ジャーナル対応
//[]----------------------------------------------------------------------[]
//	@brief			ユーザーメンテナンス：拡張機能 - 電子ジャーナルメニュー
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change<br>
//							MOD_EXT : F5 key
//	@author			
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2021-01-08
//					Update	:	
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static ushort	UsMnt_EJournalMnu(void)
{
	ushort	usEJournalEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();									// 画面クリア
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[187]);			// "＜電子ジャーナル＞　　　　　　"
		usEJournalEvent = Menu_Slt( (void*)EXTENDMENU_EJ, USM_EXTEJ_TBL, (char)USM_EXTEJ_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usEJournalEvent ){
		case LCD_DISCONNECT:
			return MOD_CUT;

		case MNT_EJ_SD_INF:					// ＳＤカード情報
			usEJournalEvent = UsMnt_EJournalSDInfo();
			if (usEJournalEvent == MOD_CHG){
				return(MOD_CHG);
			}
			break;

		case MOD_EXT:		// 終了（Ｆ５）
			return(MOD_EXT);

		case MOD_CHG:		// モードチェンジ
			return(MOD_CHG);

		default:
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			SDカード情報（電子ジャーナル）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=モード切替, MOD_EXT=終了
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static ushort	UsMnt_EJournalSDInfo(void)
{
	ushort	ret = 0;	// 戻り値
	short	msg = -1;	// 受信ﾒｯｾｰｼﾞ
	ulong	upper, lower, work;

	if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
		IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR)) {
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188]);		// "＜ＳＤカード情報＞　　　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[24]);		// "　全体サイズ：　　　　　ＭＢ　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[25]);		// "　空きサイズ：　　　　　ＭＢ　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[26]);		// "　使用時間　：　　　　　時間　"

	Fun_Dsp( FUNMSG[8] );												// "　　　　　　　　　　　　 終了 "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_INFO);					// SDカード情報要求

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
		case LCD_DISCONNECT:
			return MOD_CUT;
		case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// Ｆ５（終了）
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SD_INFO:						// SDカード情報通知
			memcpy(&upper, &eja_work_buff[0], 4);
			memcpy(&lower, &eja_work_buff[4], 4);
			work = upper * (1 << 12);			// 上位桁をMB単位に変換
			work += (lower / (1024*1024));		// 下位桁をMB単位に変換
			opedpl3(2, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 全体サイズ

			memcpy(&upper, &eja_work_buff[8], 4);
			memcpy(&lower, &eja_work_buff[12], 4);
			work = upper * (1 << 12);			// 上位桁をMB単位に変換
			work += (lower / (1024*1024));		// 下位桁をMB単位に変換
			opedpl3(3, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 空きサイズ

			memcpy(&work, &eja_work_buff[16], 4);
			opedpl3(4, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 使用時間
			break;
		default:
			break;
		}
	}
	return ret;
}
// MH810104 GG119201(E) 電子ジャーナル対応

