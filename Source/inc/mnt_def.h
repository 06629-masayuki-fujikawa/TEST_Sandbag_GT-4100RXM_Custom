/*[]----------------------------------------------------------------------[]*/
/*| Maintenance control header file                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2XXX-XX-XX                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
#ifndef _MNT_DEF_H_
#define _MNT_DEF_H_

#include	"mem_def.h"

/*--------------------------------------------------------------------------*/
/*	Externs																	*/
/*--------------------------------------------------------------------------*/

/* Const table */
extern	const	unsigned short	USER_TBL[][4];
extern	const	unsigned short	SYS_TBL[][4];
extern	const	unsigned short	PARA_SET_TBL[][4];
extern	const	unsigned short	BPARA_SET_TBL[][4];
extern	const	unsigned short	CPARA_SET_TBL[][4];
extern	const	unsigned short	RPARA_SET_TBL[][4];
extern	const	unsigned short	FUN_CHK_TBL[][4];
extern	const	unsigned short	FLCD_CHK_TBL[][4];
extern	const	unsigned short	FSIG_CHK_TBL[][4];
extern	const	unsigned short	FPRN_CHK_TBL1[][4];
extern	const	unsigned short	FPRN_CHK_TBL2[][4];
extern	const	unsigned short	FPRN_CHK_TBL3[][4];
extern	const	unsigned short	FPRN_CHK_TBL4[][4];
// MH810104 GG119201(S) 電子ジャーナル対応
extern	const	unsigned short	FPRN_CHK_TBL5[][4];
// MH810104 GG119201(E) 電子ジャーナル対応
extern	const	unsigned short	FRDR_CHK_TBL[][4];
extern	const	unsigned short	FCMC_CHK_TBL[][4];
extern	const	unsigned short	FBNA_CHK_TBL[][4];
extern	const	unsigned short	LOG_FILE_TBL[][4];
extern	const	unsigned short	USM_MNC_TBL1[][4];
extern	const	unsigned short	USM_MNC_TBL2[][4];
extern	const	unsigned short	USM_MNS_TBL[][4];
extern	const	unsigned short	MNT_SEL_TBL[][4];
extern	const	unsigned short	CARD_ISU_TBL[][4];
extern	const	unsigned short	FUNC_CNT_TBL1[][4];
extern	const	unsigned short	FUNC_CNT_TBL2[][4];
extern	const	unsigned short	UPDN_TBL[][4];
extern	const	unsigned short	UPLORD_TBL[][4];
extern	const	unsigned short	DWLORD_TBL[][4];
extern	const	unsigned short	BKRS_TBL[][4];
extern	const	unsigned short	USM_EXT_TBL[][4];
extern	const	unsigned short	USM_EXTCRE_TBL[][4];
extern	const	unsigned short	MIFARE_CHK_TBL[][4];
extern	const	unsigned short	USM_LBSET_TBL[][4];
extern	const	unsigned short	USM_LOCK_TBL[][4];
extern	const	unsigned short	USM_KIND3_TBL[][4];
extern	const	unsigned short	USM_KIND_TBL[][4];

extern	const	unsigned short	CCOM_CHK_TBL[][4];
extern	const	unsigned short	CCOM_CHK2_TBL[][4];


// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	const	unsigned short	USM_EXTEDY_TBL[][4];
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
extern	const	unsigned short	PIP_CHK_TBL[][4];
extern	const	unsigned short	USM_EXTSCA_TBL[][4];
extern	const	unsigned short	FSUICA_CHK_TBL[][4];
extern	const	unsigned short	FSUICA_CHK_TBL2[][4];
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	const	unsigned short	FEDY_CHK_TBL[][4];
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
extern	const	unsigned short	FILE_TRF_TBL[][4];
extern	const	unsigned short	SYS_SW_TBL[][4];
extern	const 	unsigned short	UPLOAD_LOG_TBL[][4];
extern	const	unsigned short	USM_RT_TBL[][4];
extern	const	unsigned short	CRE_CHK_TBL[][4];
// MH810100(S) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
extern	const	unsigned short	QR_CHK_TBL[][4];
extern	const	unsigned short	REAL_CHK_TBL[][4];
extern	const	unsigned short	DCNET_CHK_TBL[][4];
// MH810100(E) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
// NTNETチェックメニュー
extern	const	unsigned short	FNTNET_CHK_TBL[][4];

extern	const	unsigned short	USM_LOG_TBL[][4];
extern	const	unsigned short	USM_EXTCRE_CAPPI_TBL[][4];

extern	const	unsigned short	FUNC_FLP_TBL[][4];
extern	const	unsigned short	FUNC_CRR_TBL[][4];

extern	const	unsigned short	FUNC_MAFCHK_TBL[][4];
extern	const	unsigned short	FUNC_CAPPICHK_TBL[][4];

extern	const	unsigned short	FUNC_FTPCONNECTCHK_TBL[][4];
extern	const	unsigned short	MNT_STRCHK_TBL[][4];

extern	const	unsigned short	USM_CERTI_TBL[][4];
extern	const	char			USM_CERTI_TBL_MAX;

// MH321800(S) D.Inaba ICクレジット対応 (決済リーダチェック追加)
extern	const	unsigned short	FUN_CHK_TBL2[][4];
extern	const	unsigned short	FECR_CHK_TBL[][4];
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダチェック追加)

/* Groval variables */

extern	char	DP_CP[2];				/* 0:Cursor Possion ( 0～2 )				*/
										/* 1:Item № displayed in the first line	*/
extern	unsigned char	*fck_chk_adr;		// ﾁｪｯｸﾒﾓﾘｰｱﾄﾞﾚｽ（内部ＲＡＭﾒﾓﾘｰﾁｪｯｸ用）
extern	unsigned char	fck_chk_err;		// ﾁｪｯｸ結果		（内部ＲＡＭﾒﾓﾘｰﾁｪｯｸ用）
extern	unsigned char	fck_sav_data;		// ﾃﾞｰﾀ退避ｴﾘｱ	（内部ＲＡＭﾒﾓﾘｰﾁｪｯｸ用）
extern	unsigned char	fck_chk_data1;		// ﾁｪｯｸﾃﾞｰﾀ１	（内部ＲＡＭﾒﾓﾘｰﾁｪｯｸ用）
extern	unsigned char	fck_chk_data2;		// ﾁｪｯｸﾃﾞｰﾀ２	（内部ＲＡＭﾒﾓﾘｰﾁｪｯｸ用）
enum{
// MH810100(S) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
//	Multi_Total = 1,
//	Credit,
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////	Edy,
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Suica,
//	iD,
//	Hojin,
//	DLock,
	DLock = 1,
// MH810100(E) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
	MTotal,		// ＭＴ集計
// MH321800(S) D.Inaba ICクレジット対応（決済リーダ処理追加）
	EcReader,	// 決済リーダ
// MH321800(E) D.Inaba ICクレジット対応（決済リーダ処理追加）
// MH810104 GG119201(S) 電子ジャーナル対応
	EJournal,	// 電子ジャーナル
// MH810104 GG119201(E) 電子ジャーナル対応
	USM_EXT_MAX
};
extern	unsigned char	Ext_Menu_Max;										/* メニュー表示項目数（設定により可変）*/
extern	unsigned char	USM_ExtendMENU[USM_EXT_MAX][31];					/* 拡張機能 Menu （構築用）*/
extern	unsigned short	USM_ExtendMENU_TBL[USM_EXT_MAX][4];					/* 拡張機能 Menu TBL（構築用）*/

extern short	gCurSrtPara;	// 車室分類（駐車/駐輪）
extern short	DispAdjuster;	// 車室パラメータ表示用調整値

extern uchar	MovCntClr;		// 動作ｶｳﾝﾄｸﾘｱ実行有無（OFF=実行無／ON=有）

/* Arcnet NID & Version No. */
struct	st_arc_com	{
	char			id;
	char			nid;
	unsigned char	ver[8];
	char			result;
};

//---------------------------------
// 領収証再発行表示用
//---------------------------------
typedef struct {
	ushort			nextIndex;			// 次のログインデックス
	ushort			nextCount;			// 次の検索対象ログ件数
} t_OpeLog_FindNextLog;

// Function (mntdata.c)
extern	unsigned short	StoF( unsigned short keyid, char mode );
extern	unsigned short	Menu_Slt( const unsigned char (*d_str)[31], const unsigned short (*d_tbl)[4], char cnt, char mod );
extern	char			pag_ctl( unsigned short ind, unsigned short pos, unsigned short *top );

// Function (parametersec.c)
extern	unsigned short	ParSetMain( void );
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(Edy)
//extern	unsigned short	SysParWrite( char * f_DataChange, char * edy_DataChange );
extern	unsigned short	SysParWrite( char * f_DataChange );
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(Edy)
extern	unsigned short	CarParWrite( char * f_DataChange );
extern	unsigned short	RckParWrite( char * f_DataChange );
// MH810100(S) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
extern  unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)

// Function (fncchk.c)
extern	unsigned short	FncChkMain( void );
extern	unsigned short	FunChk_Lcd( void );
extern	void			IBKCtrl_SetRcvData_manu(uchar id, uchar rslt);
extern	uchar	bcdadd( uchar data1, uchar data2 );

extern	void CcomApnGet( uchar * );
extern	void CcomApnSet( const uchar *,unsigned short );
extern	void CcomIpGet( uchar *, short, short);
// MH810100(S) Y.Yamauchi 2019/12/05 車番チケットレス(メンテナンス)
//extern	void VoiceGuideTest( char );
extern	BOOL PKTcmd_mnt_qr_ctrl_req( unsigned char ctrl_cd );
extern	BOOL PKTcmd_mnt_rt_con_req( void );
extern	BOOL PKTcmd_mnt_dc_con_req( void );
// MH810100(E) Y.Yamauchi 2019/12/05 車番チケットレス(メンテナンス)
// Function (lcdchk.c)
extern	unsigned short	FColorLCDChk_PatternDSP( void );
extern	unsigned short	FColorLCDChk_Bk( void );
extern	unsigned short	FColorLCDChk_Lumine_change( void );

extern	int				UsMnt_mnyctl_chk( void );
extern	uchar			UserMnt_SysParaUpdateCheck( ushort );
extern	unsigned char	Is_ExtendMenuMake( void );
extern	void			Is_ExtendMenuMakeTable(uchar, uchar, uchar);
extern	short			Is_CarMenuMake( uchar );
extern	ushort 			Is_MenuStrMake( uchar );
extern	unsigned char	GetCarInfoParam( void );
// メニュー数を設定にて変化させるメニュー管理用のenumです
enum{
	EXTEND_MENU=0,
	CAR_3_MENU,
	CAR_2_MENU,
	MV_CNT_MENU,
	LOCK_CTRL_MENU,
};

extern	const	ushort	Car_Start_Index[3];
extern	void	Ope_clk_set( struct clk_rec *p_clk_data, ushort OpeMonCode );

// Function (usermnt2.c)
extern	unsigned short	UsMnt_AreaSelect(short kind);
extern	unsigned short	UsMnt_StaySts(void);
extern	unsigned short	UsMnt_ParkCnt(void);
extern	unsigned short	UsMnt_FullCtrl(void);
extern	unsigned short	UsMnt_Receipt(void);
									// 個別精算ログ件数・1件取り出し用define
#define	PAY_LOG_ALL				0	// 全個別精算ログを対象とする
#define	PAY_LOG_CMP				1	// 精算完了(不正・強制出庫を除く)の個別精算ログを対象とする
#define	PAY_LOG_STP				2	// 精算中止・再精算中止の個別精算ログを対象とする
#define	PAY_LOG_CMP_STP			3	// 精算完了(不正・強制出庫を除く)・精算中止・再精算中止の個別精算ログを対象とする

extern	ushort			Ope_SaleLog_TotalCountGet( uchar );
extern	uchar			Ope_SaleLog_1DataGet( ushort, uchar, ushort, Receipt_data * );
extern	uchar			Ope_SaleLog_First1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog );
extern	uchar			Ope_SaleLog_Next1DataGet( ushort Index, uchar Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog );
extern	unsigned short	UsMnt_PassInv(void);
extern	unsigned short	UsMnt_PassEnter(void);
extern	unsigned short	UsMnt_PassCheck(void);
extern	unsigned short	UsMnt_SrvTime(void);
extern	unsigned short	UsMnt_LockTimer(uchar);
extern	unsigned short	UsMnt_LockTimerEx(void);
extern	unsigned short	UsMnt_SplDay(void);
extern	unsigned short	UsMnt_PassWord(void);
extern	unsigned short	UsMnt_BackLignt(void);
extern	unsigned short	UsMnt_KeyVolume(void);
extern	unsigned short	UsMnt_TickValid(void);
extern	unsigned short	UsMnt_BusyTime(void);
extern	unsigned short	UsMnt_Mnysetmenu(void);
extern	void			TKI_Delete(short);
extern	unsigned short	UsMnt_Extendmenu(void);
extern	unsigned short	UsMnt_FTotal(void);
extern	unsigned short	UsMnt_PassStop(void);
extern	const struct	TKI_CYUSI	*TKI_Get(struct	TKI_CYUSI *data, short ofs);
extern	void			TKI_DeleteAll(void);
extern	unsigned short	UsMnt_PassExTime(void);
extern	unsigned short	UsMnt_LabelPrint(void);
extern	unsigned short	UsMnt_LabelSet(void);
extern	unsigned short	UsMnt_PreAreaSelect(short);
extern	unsigned short	UsMnt_PwdKyo(void);
extern	unsigned short	UsMnt_VolSwTime(void);
extern	char clk_test( short data, char pos );
extern	unsigned short	UsMnt_StatusView(void);
extern	unsigned short	UsMnt_CreditMnu(void);
extern	unsigned short	UsMnt_CreUseLog(void);
extern	unsigned short	UsMnt_CreUseLog_GetLogCnt(ushort Sdate, ushort Edate);
extern	unsigned short	UsMnt_VoiceGuideTime(void);

#define	_TKI_Idx2Ofs(idx)	_offset(tki_cyusi.wtp, TKI_CYUSI_MAX-tki_cyusi.count+idx, TKI_CYUSI_MAX)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	unsigned short	UsMnt_EdyMnu(void);
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

extern	unsigned short	UsMnt_SuicaMnu(void);
extern	unsigned short	UsMnt_DiditalCasheSyuukei(ushort);
extern	unsigned short	UsMnt_DiditalCasheUseLog(ushort);
extern	unsigned short	SysMnt_FTPClient(void);
extern	unsigned short	SysMnt_FTPServer(void);
extern  unsigned short	SysMnt_SystemSwitch(void);
extern	unsigned short	UsMnt_AttendantValidData(void);
extern	unsigned short	UsMnt_Logprintmenu(void);
extern	unsigned short	UsMnt_NgLog(void);
extern	unsigned short	UsMnt_IoLog(ushort);
// MH810100(S)  K.Onodera   2019/12/26 車番チケットレス(QR確定・取消データ対応)
//extern	ushort UsMnt_SearchIOLog(ushort, ushort, ulong, ulong, IoLog_Data*);
// MH810100(E)  K.Onodera   2019/12/26 車番チケットレス(QR確定・取消データ対応)
extern	void	LockInfoDataCheck(void);
extern	ushort	UsColorLCDLumine(void);
extern unsigned short	UsMnt_ParkingCertificate(void);
extern  unsigned short	SysMnt_RyoTest(void);

// 時刻補正処理
extern int		sntpGetTime(date_time_rec2 *time, ushort * net_msec);
extern void		TimeAdjustInit(void);
extern int		TimeAdjustCtrl(ushort req);
extern void		RegistReviseLog(ushort exec);
extern void		ReqServerTime(ushort msg);
extern int		CheckTimeMargin(date_time_rec2 *dt, ushort net_msec, ulong margin[]);
extern ushort	AddTimeMargin(ulong margin[], short in_d[]);
extern void		TimeAdjustMain(ushort msg);
extern void		clk_auto_set(void);
extern	void	UsMnt_datecheck( uchar );

// Function (usermnt.c)
extern	unsigned short	UsMnt_Total( short );
extern	void	LogDateDsp4( ushort *Date );
extern	void	LogDateDsp5( ushort *Date, uchar pos );
extern	void	LogCntDsp( ushort LogCnt );
extern	uchar	LogDatePosUp( uchar *pos, uchar req );

// MH321800(S) D.Inaba ICクレジット対応
// 拡張機能/決済リーダ処理
extern	const	unsigned short	USM_EXTEC_TBL[][4];
extern unsigned short GetEcShuukeiCnt(ushort s_date, ushort e_date);
// システムメンテナンス/決済リーダチェック/ブランド確認状態
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
//extern unsigned char  get_brand_index( ushort no );
// MH810103 GG119202(E) ブランド名はブランド情報から取得する
extern uchar          check_printer( uchar *pri_kind );
extern unsigned short UsMnt_ECVoiceGuide( void );
// MH321800(E) D.Inaba ICクレジット対応

// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
extern int lcdbm_setting_upload_FTP( void );
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
extern uchar Lcd_QR_ErrDisp( ushort result );
extern void UsMnt_QR_ErrDisp( int err );
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//extern void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page);
//extern void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page);
extern void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page, ushort qrtype);
extern void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page, ushort qrtype);
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
extern ushort UsMnt_QR_GetUpdownPageMax(ushort id, QR_AmountInfo *pAmntQR);
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
extern void UsMnt_QR_DispTicQR(QR_TicketInfo *pTicQR, ushort updown_page);
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）

/*--------------------------------------------------------------------------*/
/*	Difines																	*/
/*--------------------------------------------------------------------------*/

/* Macro */
#define	KEY_TEN0to9(msg)	(KEY_TEN0<=msg)&&(msg<=KEY_TEN9) ? KEY_TEN:msg

#define		PGCNT(cnt)		( cnt % 5 ? cnt / 5 : (cnt / 5) - 1 )

/* タイマー */
#define	OPE_LCD_BACK_CNTL	1					/* LCD Back Timer					*/	//＠

/* メニューテーブルの項目数 */
/* Menu items max */
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// #define		SYS_MENU_MAX		9				/* Engeneer						*/
#define		SYS_MENU_MAX		8				/* Engeneer						*/
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
#define		FL_TRF_MAX			2				/* ファイル転送 Menu　　　　　　*/
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス(メンテナンス)
//#define		SYS_SW_MAX			3				/* システム切替 Menu　　　　　　*/
#define		SYS_SW_MAX			2				/* システム切替 Menu　　　　　　*/
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス(メンテナンス)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//#define		FNTNET_CHK_MAX		3				/* NTNETチェックメニュー		*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間の追番クリアメニューを削除）
//#define		FNTNET_CHK_MAX		2				/* NTNETチェックメニュー		*/
#define		FNTNET_CHK_MAX		1				/* NTNETチェックメニュー		*/
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間の追番クリアメニューを削除）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

#define		PARA_SET_MAX		3				/* 								*/
#define		BPARA_SET_MAX		3				/* 								*/
#define		CPARA_SET_MAX		3				/* 								*/
#define		RPARA_SET_MAX		3				/* 								*/
enum {
	__swt_check = 0,	// スイッチチェック
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
//	 __kbd_check,		// キー入力チェック
//	__lcd_check,		// ＬＣＤチェック
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
	__led_check,		// ＬＥＤチェック
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// __sht_check,		// シャッターチェック
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
	__sig_check,		// 入出力信号チェック
	__cmc_check,		// コインメックチェック
	__bna_check,		// 紙幣リーダーチェック
	__prt_check,		// プリンタチェック
// MH810100(S) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
	__qr_check,		//ＱＲリーダーチェック　
// MH810100(E) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// __rdr_check,		// 磁気リーダーチェック
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
	__ann_check,		// アナウンスチェック
	__mem_check,		// メモリーチェック
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// __lck_check,		// ロックIF盤チェック
// __flp_check,		// フラップ装置チェック
// __crr_check,		// フラップ制御基板チェック
//	__sca_check,		// 交通系ICカードチェック
//	__pip_check,		// Park i Proチェック
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
	__lan_connect_chk,	// ＬＡＮ接続チェック
	__funope_check,		// ＦＡＮ動作チェック
	__centercom_check,	//	センター通信チェック
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// __cre_check,			// クレジットチェック
	__realtime_check,	// リアルタイム通信チェック
	__dc_net_check,		// DC-NET通信チェック
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	__ntn_check,		// ＮＴ－ＮＥＴチェック
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	__ver_check,	//バージョンチェック
	FUN_CHK_MAX
};
// MH810100(S) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)
enum {
	__swt_check2 = 0,	// スイッチチェック
	__led_check2,		// ＬＥＤチェック
	__sig_check2,		// 入出力信号チェック
	__cmc_check2,		// コインメックチェック
	__bna_check2,		// 紙幣リーダーチェック
	__prt_check2,		// プリンタチェック
	__qr_check2,		//ＱＲリーダーチェック　
	__ann_check2,		// アナウンスチェック
	__mem_check2,		// メモリーチェック
	__sca_check2,		// : 決済リーダチェック
	__lan_connect_chk2,	// ＬＡＮ接続チェック
	__funope_check2,		// ＦＡＮ動作チェック
	__centercom_check2,	//	センター通信チェック
	__realtime_check2,	// リアルタイム通信チェック
	__dc_net_check2,		// DC-NET通信チェック
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	__ntn_check2,		// ＮＴ－ＮＥＴチェック
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	__ver_check2,	//バージョンチェック
	FUN_CHK_MAX2
};
// MH810100(E) K.Onodera 2019/10/16 車番チケットレス(メンテナンス)

enum{
	__MNT_ERARM = 0,	// .エラー・アラーム確認
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)
// __MNT_FLCTL,		// .ロック装置開閉
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_MNCNT,		// .釣銭管理
	__MNT_TTOTL,		// .T集計
	__MNT_GTOTL,		// .GT集計
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_STAYD,		// .停留車情報
// __MNT_LOGPRN,		// .履歴情報プリント
// __MNT_FLCNT,		// .駐車台数
// __MNT_FLCTRL,		// .満車コントロール
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_STSVIEW,		// .状態確認
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_FLSTS,		// .車室情報
// __MNT_CARFAIL,		// .車室故障
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_REPRT,		// .領収書再発行
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_UKERP,		// .駐車証明書再発行
// __MNT_INVLD,		// .定期有効/無効
// __MNT_ENTRD,		// .定期入庫/出庫
// __MNT_PSTOP,		// .定期券精算中止データ
// __MNT_PASCK,		// .定期券チェック
// __MNT_TKTDT,		// .券データ確認
	__MNT_QRCHECK,		// .QRデータ確認
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_CLOCK,		// .時計合せ
	__MNT_SERVS,		// .サービスタイム
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_FLTIM,		// .ロック装置閉タイマー
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_SPCAL,		// .特別日/特別期間
	__MNT_OPCLS,		// .営休業切替
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_SHTER,		// .シャッター開放
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_PWMOD,		// .係員パスワード
	__MNT_KEYVL,		// .キー音量調整
	__MNT_CNTRS,		// .輝度調整
	__MNT_VLSW,			// .音声案内時間
	__MNT_OPNHR,		// .営業開始時刻
// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_TKEXP,		// .券期限
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_MNYSET,		// .料金設定

// MH810100(S) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
// __MNT_ATTENDDATA,	// .係員有効データ
// __MNT_PWDKY,		// .解除番号強制出庫
// MH810100(E) Y.Yamauchi 20191003 車番チケットレス(メンテナンス)	
	__MNT_EXTEND,		// .拡張機能
	USER_MENU_MAX,
};

#define		MNT_SEL_MAX			2				/*								*/
#define		FLCD_CHK_MAX1		2				/*								*/
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
// #define		FLCD_CHK_MAX2		3				/*								*/
#define		FLCD_CHK_MAX2		1				/*								*/
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
// MH810104 GG119201(S) 電子ジャーナル対応
//#define		FPRN_CHK_MAX1		2				/*								*/
#define		FPRN_CHK_MAX1		3				/*								*/
// MH810104 GG119201(E) 電子ジャーナル対応
#define		FPRN_CHK_MAX2		3				/*								*/
#define		FPRN_CHK_MAX3		2				/*								*/
// MH810104 GG119201(S) 電子ジャーナル対応
#define		FPRN_CHK_MAX5		4				/*								*/
// MH810104 GG119201(E) 電子ジャーナル対応
#define		FRDR_CHK_MAX		5				/*								*/
#define		FCMC_CHK_MAX		4				/*								*/
#define		USM_MNC_MAX1		3				/*								*/
#define		USM_MNC_MAX2		5				/*								*/
#define		FBNA_CHK_MAX		3				/*								*/
// MH810100(S) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
//　MH810101 Takei(S) フェーズ2　2021/02/16 定期対応　定期無効曜日を出す
#define		USM_MNS_MAX			8				/* 料金設定内の項目数			*/
// #define		USM_MNS_MAX			7				/* 料金設定内の項目数			*/
//　MH810101 Takei(E) フェーズ2　2021/02/16 定期対応　定期無効曜日を出す
// MH810100(E) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
#define		CARD_ISU_MAX		2				/* 								*/
// MH810100(S) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
// #define		LOG_MENU_MAX		16				/*								*/
#define		LOG_MENU_MAX		15				/*								*/
// MH810100(E) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
#define		FCNT_MENU1_MAX		2				/* 								*/
#define		FCNT_MENU2_MAX		3				/* 								*/
#define		USM_EXTCRE_MAX		4				/* 								*/
#define		MIFARE_CHK_MAX		4				/* 								*/
#define		LBSET_MENU_MAX		3				/* 								*/
#define		USM_EXTCRE_CAPPI_MAX	2			/* クレジット処理(Cappi)項目数　*/
#define		USM_LOG_MAX			3				/* 	履歴情報プリントﾒﾆｭｰ項目数	*/
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//#define		CCOM_CHK_MAX		7
// MH810100(S) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
// #define		CCOM_CHK_MAX		8
// #define		CCOM_CHK_MAX		5
#define		CCOM_CHK_MAX		4
// MH810100(E) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
#define		CCOM_CHK2_MAX		3
#define		PIP_CHK_MAX			1
#define		CRE_CHK_MAX			2
// MH810100(S) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
#define		QR_CHK_MAX			2
#define		REAL_CHK_MAX		3
#define		DC_NET_CHK_TBL		3
// MH810100(E) Y.Yamauchi 20191008 車番チケットレス(メンテナンス)
#define		LOCK_MENU_MAX1		2		/* ロック装置開閉 Menu1 */
#define		LOCK_MENU_MAX2		4		/* ロック装置開閉 Menu2 */
#define		KIND_MENU_MAX		3		/* 車種選択 Menu */
// MH810100(S) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
// #define		BKRS_MAX			3
#define		BKRS_MAX			2
// MH810100(E) Y.Yamauchi 2019/10/03 車番チケットレス(メンテナンス)
#define		UPDN_MAX			2
extern	const	char		UPLD_MAX;
extern	const	char		DNLD_MAX;

#define		UPLD_LOG_MAX		4				/* ログデータ Menu　　　　　　　*/

#define		FSIG_CHK_MAX		2
#define		FSUICA_CHK_MAX		5				/*								*/
#define		FSUICA_CHK_MAX2		2				/*								*/
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		FEDY_CHK_MAX		7				/*								*/
//
//#define		USM_EXTEDY_MAX		5				/* Ｅｄｙメニュー項目数			*/
//#define		USM_BUN_MAX			2				/* Ｅｄｙメニュー項目数(未使用)	*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		USM_EXTSCA_KND		5				/* 電子マネー種類数(未使用)		*/
#define		USM_EXTSCA_MAX		2				/* 電子マネーメニュー項目数		*/

#define		FUNC_FLP_MAX		2				/* フラップ装置チェックメニュー項目数*/
#define		FUNC_CRR_MAX		2				/* フラップ制御基板チェックメニュー項目数*/
#define		FUNC_MAF_MAX		3				/* 遠隔通信メニュー項目数		*/
#define		FUNC_CAPPI_MAX		2				/* CAPPIチェックメニュー項目数		*/

#define		FUNC_FTPCONNECT_MAX		2			/* FTP接続確認メニュー項目数		*/
#define		MNT_STRCHK_MAX		1				/* デバッグ用メニュー項目数*/
#define		MNT_FEETEST_MAX		2				/* 料金計算テスト項目数*/
#define		ANT_CNT_DEF			100				/* 電波受信状態取得回数デフォルト(100回)	*/
#define		ANT_INT_DEF			5				/* 電波受信状態取得間隔デフォルト(500ms)	*/

// MH321800(S) D.Inaba ICクレジット対応
#define		FECR_CHK_MAX		5				/* 決済リーダチェック メニュー項目数 */
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//#define		USM_EXTEC_MAX		3				/* 決済リーダ処理 メニュー項目数*/
#define		USM_EXTEC_MAX		4				/* 決済リーダ処理 メニュー項目数*/
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH321800(E) D.Inaba ICクレジット対応

// MH810104 GG119201(S) 電子ジャーナル対応
#define		USM_EXTEJ_MAX		1				/* 電子ジャーナル メニュー項目数*/
// MH810104 GG119201(E) 電子ジャーナル対応

/* Maintenance operation level */
#define		ATTEND			0					/* 								*/
#define		MANAGE			1					/* 								*/
#define		ENGENE			2					/* 								*/

/* Operation ID */
/* Phase 0 */
#define	USERMNTMAIN	1	/* User Maintenance...Attendant or Manager */
#define	SYSMNTMAIN	2	/* System Maintenance...Engeneer */

/* Phase 1 */
/* User Maintenance 0-69 */
enum{
	// 以下の番号は、同一メニュー内で一意の数値であればいいので、メニュー番号とリンクする必要はない
	// 今後の追加は一意の番号と被らないように設定すること。
	// 現時点ではMNT_IVTRY=50なので、原則そこに被らないようにすること
	MNT_NONE	= 0,				// 割当なし
	MNT_ERARM	= 1,                // .エラー・アラーム確認
	MNT_FLCTL	,                   // .ロック装置開閉
	MNT_MNCNT	,                   // .釣銭管理
	MNT_TTOTL	,					// .T集計
	MNT_GTOTL	,                   // .GT集計
	MNT_STAYD	,                   // .停留車情報
	MNT_LOGPRN	,                   // .履歴情報プリント	
	MNT_FLCNT	,                   // .駐車台数
	MNT_FLCTRL	,                   // .満車コントロール
	MNT_STSVIEW	,                   // .状態確認
	MNT_FLSTS	,                   // .車室情報
	MNT_CARFAIL	,                   // .車室故障
	MNT_REPRT	,                   // .領収書再発行
	MNT_UKERP	,                   // .駐車証明書再発行
	MNT_INVLD	,                   // .定期有効/無効
	MNT_ENTRD	,                   // .定期入庫/出庫
	MNT_PSTOP	,                   // .定期券精算中止データ
	MNT_PASCK	,                   // .定期券チェック
	MNT_TKTDT	,                   // .券データ確認
// MH810100(S) Y.Yamauchi 2019/11/07 車番チケットレス(メンテナンス)
	MNT_QRCHECK,					// .QRデータ確認
// MH810100(E) Y.Yamauchi 2019/11/07 車番チケットレス(メンテナンス)
	MNT_CLOCK	,                   // .時計合せ
	MNT_SERVS	,                   // .サービスタイム
	MNT_FLTIM	,                   // .ロック装置閉タイマー
	MNT_SPCAL	,                   // .特別日/特別期間
	MNT_OPCLS	,                   // .営休業切替
	MNT_SHTER	,                   // .シャッター開放
	MNT_PWMOD	,                   // .係員パスワード
	MNT_KEYVL	,                   // .キー音量調整
	MNT_CNTRS	,                   // .輝度調整
	MNT_VLSW	,                   // .音量案内時間
	MNT_OPNHR	,                   // .営業開始時刻
	MNT_TKEXP	,                   // .券期限
	MNT_MNYSET	,                   // .料金設定
	MNT_ATTENDDATA	,               // .係員有効データ
	MNT_EXTEND	,                   // .拡張機能
};

#define	MNT_IVTRY	50			// インベントリ
#define	MNT_BKLIT	51			// バックライト点灯方法
#define	MNT_MNYBOX	52			// 金庫確認
#define	MNT_MNYCHG	53			// 釣銭枚数
#define	MNT_FTOTL	54			// 複数台数集計
#define	MNT_LBSET	55			// ラベルプリンタ設定
#define	MNT_LBPAPER	56			// 用紙残量設定
#define	MNT_LBPAY	57			// ラベル発行不可時の設定
#define	MNT_LBTESTP	58			// ラベルテスト印字
#define	MNT_PWDKY	59			// 暗証番号強制出庫
#define MNT_MNYLIM 	60			// Limit money setting
#define MNT_MNYTIC	61			// Service ticket setting
#define MNT_MNYSHP	62			// Shop setting
#define MNT_MNYTAX	63			// Tax setting
#define MNT_MNYKID	64			// A-L kind setting
#define MNT_MNYCHA	65			// Charge setting
#define MNT_MNYTWK	66			// Ticket disable week setting

/* Phase 2 */       
/* System Maintenance 70-89 */
#define	MNT_PRSET	70
#define	MNT_FNCHK	71
#define	MNT_FNCNT	72
#define	MNT_LKCNT	73
#define	MNT_LOGFL	74
#define	MNT_BAKRS	75
#define	MNT_INITI	76
#define	MNT_CDISS	77
#define	MNT_OPLOG	78
#define	MNT_SYSMN	79
#define	MNT_TBSET	80
#define	MNT_CRSET	81
#define	MNT_FORMT	82
#define	MNT_UPDWN	83
#define	MNT_MNYPRI	84

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define	MNT_EDY_MEISAI	85	// Ｅｄｙ利用明細
//#define	MNT_EDY_SHUUKEI	86	// Ｅｄｙ集計
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define	MNT_SCA			87	// Ｓｕｉｃａ処理
#define	MNT_SCA_MEISAI	88	// Ｓｕｉｃａ利用明細
#define	MNT_SCA_SHUUKEI	89	// Ｓｕｉｃａ集計

/* Function Check 90-110 */
#define	SWT_CHK		90
#define	LCD_CHK		91
#define	PRT_CHK		92
#define	CMC_CHK		93
#define	BNA_CHK		94
#define	SIG_CHK		95
#define	MEM_CHK		96
#define	ANN_CHK		97
#define	ARC_CHK		99
#define	LED_CHK		100
#define	KBD_CHK		101
#define	SHT_CHK		102
#define	RDR_CHK		103
#define	NTNET_CHK	104
#define	VER_CHK		105
#define	MIF_CHK		106
#define	LCK_CHK		107
#define	FLP_CHK		108
#define	SUICA_CHK	109
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define	EDY_CHK		110
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define	CCM_CHK		111
#define	PIP_CHK		112
#define	FUNOPE_CHK	113
#define MNT_FLTRF	114
#define MNT_SYSSW	115
#define CRE_CHK		116
#define	CRR_CHK		119

/* Function Check 121-125 */
#define	FLCD1_CHK	121
#define	FLCD2_CHK	122
#define	FLCD3_CHK	123
#define	FLCD4_CHK	124

/* Function Check 127-128 */
#define	FSIG1_CHK	127
#define	FSIG2_CHK	128

/* Function Check 131-136 */
#define	FPRNR_CHK	131		// ﾚｼｰﾄﾌﾟﾘﾝﾀﾁｪｯｸ
#define	FPRNJ_CHK	132		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾁｪｯｸ

#define	FPRN1_CHK	133		// ﾚｼｰﾄﾌﾟﾘﾝﾀ	：ﾃｽﾄ印字
#define	FPRN2_CHK	134		// ﾚｼｰﾄﾌﾟﾘﾝﾀ	：状態確認
#define	FPRN3_CHK	135		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ	：ﾃｽﾄ印字
#define	FPRN4_CHK	136		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ	：状態確認
#define	FPRN5_CHK	137		// ﾚｼｰﾄﾌﾟﾘﾝﾀ	：ロゴ印字データ登録
#define	FPRN7_CHK	139		// ﾚｼｰﾄﾃｽﾄｲﾝｻﾂ
#define	FPRN8_CHK	140		// ﾚｼｰﾄﾌﾟﾘﾝﾀ	：領収書印刷

/* Function Check 141-145 */
#define	FRD1_CHK	141
#define	FRD2_CHK	142
#define	FRD3_CHK	143
#define	FRD4_CHK	144
#define	FRD5_CHK	145

/* Function Check 151-154 */
#define	FCM1_CHK	151
#define	FCM2_CHK	152
#define	FCM3_CHK	153
#define	FCM4_CHK	154

/* Function Check 161- 163*/
#define	FBN1_CHK	161
#define	FBN2_CHK	162
#define	FBN3_CHK	163

/* Function Check 167-169 */
#define	FNTNET1_CHK	167
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//#define	FNTNET2_CHK	168
//#define	FNTNET3_CHK	169
#define	CCOM_SEQCLR	168
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/* Log Files 171-181 */
#define	ERR_LOG		171		// エラー情報
#define	ARM_LOG		172		// アラーム情報
#define	TGOU_LOG	173		// Ｔ合計情報
#define	COIN_LOG	174		// コイン金庫情報
#define	NOTE_LOG	175		// 紙幣金庫情報
#define	TURI_LOG	176		// 釣銭管理情報
#define	SEISAN_LOG	177		// 個別精算情報
#define	FUSKYO_LOG	178		// 不正・強制出庫情報
#define	OPE_LOG		180		// 操作情報
#define	TEIFUK_LOG	181		// 停復電情報
#define	CLEAR_LOG	182		// ログファイルクリア
#define	MONI_LOG	183		// モニタ情報
#define	ADDPRM_LOG	184		// アラーム情報
#define	RTPAY_LOG	185		// 料金テスト情報
#define	GTGOU_LOG	186		// ＧＴ合計情報
#define	RMON_LOG	187		// 遠隔監視情報

/* Backup & Restore XX-XX(25?) */

/* Card Issue 190-192 */
#define	CARD_PASS	190		/* 定期 */
#define	CARD_KKRI	191		/* 係員カード */
#define	CARD_MIFA	192		/* Mifare定期 */

/* Exit ID */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
#define MOD_CUT		0xFFFD
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
#define	MOD_EXT		0xFFFE
#define	MOD_CHG		0xFFFF

/* Maintenance Select 300-301 */
#define	USR_MNT		300
#define	SYS_MNT		301

/* Parameter Setting1 200-202(3) */
#define	BASC_PARA	200		/* Basic Parameter */
#define	CELL_PARA	201		/* Cell Parameter */
#define	ROCK_PARA	202		/* Rock Parameter */

/* Basic Parameter Setting 210-213 */
#define	CMON_PARA	210		/* Common Parameter */
#define	INDV_PARA	211		/* Individual Parameter */
#define	PRNT_PARA	212		/* Print All Parameter */
#define	DEFA_PARA	213		/* Default All Parameter */

/* Cell Parameter Setting 220-222 */
#define	CARP_PARA	220		/* Car Parameter */
#define	CARP_PRNT	221		/* Print All Parameter */
#define	CARP_DEFA	222		/* Default All Parameter */

/* Rock Parameter Setting 230-232 */
#define	RCKP_PARA	230		/* Car Parameter */
#define	RCKP_PRNT	231		/* Print All Parameter */
#define	RCKP_DEFA	232		/* Default All Parameter */

/* Function count 240-241 */
#define	FCNT_FUNC	240		/* 動作カウント */
#define	FCNT_ROCK	241		/* ロック装置動作カウント */
#define	FCNT_FLAP	242		/* フラップ装置動作カウント */

// Proc number of money management
#define	MNY_CHG_BEFORE			100
#define	MNY_CHG_AFTER			101
#define	MNY_CTL_BEFORE			102
#define	MNY_CTL_AFTER			103
#define MNY_INCOIN				104
#define MNY_CTL_AUTO			105
#define MNY_CHG_10YEN			106
#define MNY_CHG_50YEN			107
#define MNY_CHG_100YEN			108
#define MNY_CHG_500YEN			109
#define MNY_CHG_10SUBYEN		110
#define MNY_CHG_100SUBYEN		111
#define MNY_CTL_AUTOSTART		112
#define MNY_CTL_AUTOCOMPLETE	113
#define MNY_COIN_CASETTE		114
#define MNY_COIN_INVSTART		115
#define MNY_COIN_INVCOMPLETE	116
#define MNY_COIN_INVBUTTON		117
#define MNY_CHG_50SUBYEN		118

/* backup or restore 250-252 */
#define	BKRS_BK					250
#define	BKRS_RS					251
#define	BKRS_FLSTS				252

/* upload or download 260-261 */
#define	UPDN_UP					260
#define	UPDN_DN					261

/* upload(download) data 270-275 */
#define	LD_LOGO					270
#define	LD_HDR					271
#define	LD_FTR					272
#define	LD_PARA					273
#define	LD_SLOG					274
#define	LD_TLOG					275
#define	LD_LOCKPARA				276
#define	LD_ACCEPTFTR			277
#define	LD_SYOM					278
#define	LD_KAME					279

/* Mifare checkt 280-283 */
#define	CHK_COMTST				280		/* 通信テスト */
#define	CHK_REDLITTST			281		/* リード・ライトテスト  */
#define	CHK_CADPECRNUMRED		282		/* カード固有番号リード  */
#define	CHK_VERCHK				283		/* バージョンチェック  */

/* Lock-machine 290-291 */
#define	MNT_FLAPUD				290		/* フラップ上昇下降 */
#define	MNT_BIKLCK				291		/* 駐輪ロック開閉 */
#define	MNT_FLAPUD_ALL			292		/* フラップ上昇下降（全て）		*/
#define	MNT_BIKLCK_ALL			293		/* 駐輪ロック開閉（全て）	 	*/
#define	MNT_INT_CAR				319		/* 駐車(内蔵) */
#define	MNT_CAR					320		/* 駐車 */
#define	MNT_BIK					321		/* 駐輪 */

/* User Maintenance 200-299 */
#define	MNT_CREDIT				310	// クレジット処理（拡張機能）
#define	MNT_CREUSE				311	// クレジット利用明細（拡張機能）
#define	MNT_CRECONECT			312	// クレジット接続確認（拡張機能）
#define	MNT_CREUNSEND			313	// クレジット未送信売上依頼データ（拡張機能）
#define	MNT_CRESALENG			314	// クレジット売上拒否データ（拡張機能）


/* upload(download) data (2) : 320-329 */
enum {
	LD_EDYAT = 320,
	LD_USERDEF,
	LD_LOG_DATA,
	LD_ERR,
	LD_ARM,
	LD_MONI,
	LD_OPE_MONI,
	LD_PAY_CAL,				// 料金計算用のデバッグ処理
	LD_CAR_INFO,			// 車室情報
	LD_TCARD_FTR,			// Tカードフッター
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
	LD_CREKBR_FTR,			// クレジット過払いフッター
	LD_EPAYKBR_FTR,			// 電子マネー過払いフッター
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
	LD_FUTURE_FTR,			// 後日支払額フッター
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
	LD_EMG,					// 障害連絡票フッター
// MH810105(E) MH364301 QRコード決済対応
	LD_AZU_FTR = 500,		// 預り証フッター
	LD_REMOTE_RES,			// 遠隔ダウンロード結果
	LD_PROG,
	LD_PARA_S,
	LD_VOICE_A,
	LD_MAIN_S,
	LD_VOICE_S,
	LD_AU_SCR,
	LD_CHKMODE_RES,
	LD_REMOTEDL,
	LD_PARAM_UP,
	LD_CONNECT,
// MH810100(S) K.Onodera 2019/11/20 車番チケットレス（→LCD パラメータ)
	LD_PARAM_UP_LCD,
// MH810100(E) K.Onodera 2019/11/20 車番チケットレス（→LCD パラメータ)

	__ldfile_max
};

#define	CCOM_DTCLR				330		// 未送信データクリア
#define	CCOM_FLSH				331		// 未送信データ送信
#define	CCOM_TEST				332		// センター通信テスト
#define	CCOM_APN				333		// 接続先ＡＰＮ確認
#define	CCOM_APN_AMS			334		// 接続先選択（工場検査）ＡＭＳ
#define	CCOM_APN_AMN			335		// 接続先選択（工場検査）ＡＭＡＮＯ
#define	CCOM_APN_PWEB			336		// 接続先選択（工場検査）駐車場センター（将来用）
#define	CCOM_ATN				337		// 電波受信状態確認
#define	CCOM_OIBANCLR			338		// センター追番クリア
#define	CCOM_REMOTE_DL			339		// 遠隔ダウンロード確認

#define	PIP_TEST				340		// 基板接続通信テスト
#define	PIP_DTCLR				341		// データクリア

#define	CREDIT_UNSEND			343		// 未送信売上データ
#define	CREDIT_SALENG			344		// 売上拒否データ

/* new add No.380～ */
// Menu_Slt()のmenu_tblへの対応
enum {
	MNT_INV_MONEY = 380,	// No.380
	MNT_INV_KIND,			// No.381
	MNT_INV_ALL,			// No.382
	FISMF1_CHK,				// No.383
	FISMF2_CHK,				// No.384
	FISMF3_CHK,				// No.385
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	FEDY1_CHK,				// No.386
//	FEDY2_CHK,				// No.387
//	FEDY3_CHK,				// No.388
//	FEDY4_CHK,				// No.389
//	FEDY5_CHK,				// No.390
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	FPIP1_CHK,				// No.391
	FPIP2_CHK,				// No.392
	FAPS1_CHK,				// No.393
	FAPS2_CHK,				// No.394
	// ユーザーメンテナンスのメニュー数が増えすぎて設計当初の番号管理では
	// 対応できなくなってきている。本来ならばユーザーメンテは0～69の箇所に
	// 宣言したいが、すでに一杯になっているので、ここに定義する。
	// 今後、見直す必要がある。
	MNT_CREUPDATE,			// カード情報データ要求（拡張機能）
	MNT_NGLOG,				// 不正券
	MNT_IOLOG,				// 入出庫
	MNT_PRISET,				// 料金設定メニュー プリント
	MNT_CRR_VER,			// CRRバージョンチェック
	MNT_CRR_TST,			// CRR折り返しテスト
	MNT_MAFCOMCHK,			// 基板接続通信テスト
	MNT_MAFDATACLR,			// データクリア
	MNT_MAF_RISM_COMCHK,	// Rismサーバー接続確認
	MNT_CAPPI_LOOPBACK,		// 通信折り返しテスト
	MNT_CAPPI_RESET,		// 追番リセット
	MNT_CAPPI_COMCHK,	    // Cappiサーバー接続確認
	MNT_WCARD_INF,
	MNT_HOJIN,
	CAR_FUNC_SELECT,		// Car Function Select
	MNT_PARAM_LOG,
	FTP_CONNECT_CHK,		// FTP接続チェック
	FTP_PARAM_UPL,			// 共通パラメータアップロード
	FTP_CONNECT_FUNC,		// 接続確認
	LAN_CONNECT_CHK,		// LAN接続チェック
	CHK_PRINT,
	MNT_FLP_LOOP_DATA_CHK,	// ループデータ確認
	MNT_FLP_SENSOR_CTRL,	// 車両検知センサー操作
	MNTLOG1_CHK,			// 
	MNT_DLOCK	,			// 電磁ロック対応
	MNT_SUICALOG_INJI,		// Suica通信ログ印字(全体ログ)
	MNT_SUICALOG_INJI2,		// Suica通信ログ印字(直近ログ)
	MNT_CERTI_SET,
	MNT_CERTI_ISSU,
	MNT_RYOTS,				// 料金テスト
	MNT_MTOTL,				// ＭＴ集計
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
	CCOM_LONG_PARK_ALLREL,	// 長期駐車全解除
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH321800(S) D.Inaba ICクレジット対応
	MNT_ECR,				// 決済リーダ処理
	MNT_ECR_SHUUKEI,		// 日毎集計プリント
	MNT_ECR_MEISAI,			// 利用明細プリント
	ECR_BRAND_COND,			// ブランド状態確認
	MNT_ECR_ALARM_LOG,		// 処理未了取引記録
	ECR_CHK,				// 決済リーダチェック
	ECR_MNT,				// リーダメンテナンス
// MH321800(E) D.Inaba ICクレジット対応
// MH810100(S) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
	QR_CHK,					// ＱＲリーダーチェック
	QR_READ,				// 読取テスト　　　　　　　　
	QR_VER,					// バージョン確認　　　　　　
	REAL_CHK,				// リアルタイム通信チェック
	UNSEND_REAL_CLEAR,			// 未送信データクリア 
	CENTER_REAL_TEST,			// センター通信テスト
	REAL_OIBANCLR,				// センター追番クリア
	DC_CHK,					// DC-NET通信チェック
	DC_UNSEND_CLEAR,			// 未送信データクリア
	DC_CONNECT_TEST,			// センター通信テスト
	DC_OIBANCLR,				// センター追番クリア
// MH810100(E) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	MNT_ECR_MINASHI,		// みなし決済プリント
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH810104 GG119201(S) 電子ジャーナル対応
	FPRNEJ_CHK,				// 電子ジャーナルチェック
	FPRN_RW_CHK,			// リードライトテスト
	FPRN_SD_CHK,			// ＳＤカード情報
	FPRN_VER_CHK,			// バージョン確認
	MNT_EJ,					// 電子ジャーナル
	MNT_EJ_SD_INF,			// ＳＤカード情報
// MH810104 GG119201(E) 電子ジャーナル対応
};

/* User Maintenance Edy 350-360 */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define	MNT_EDY					350	// Ｅｄｙ処理
//#define	MNT_EDY_ARM				351	// アラーム取引情報
//#define	MNT_EDY_CENT			352	// センター通信開始
//#define	MNT_EDY_SHIME			353	// Ｅｄｙ締め記録情報
//#define	MNT_EDY_TEST			354	// TEST用
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

#define	FILE_SERI				360		// シリアル接続
#define	FILE_FTP_CLI			361		// FTP接続(クライアント)
#define	FILE_FTP_SEV			362		// FTP接続(サーバー)

#define	SYS_MAIN_PRO			365		// メインプログラム
#define	SYS_SUB_PRO				366		// サブプログラム
#define	SYS_COM_PRM				367		// 共通パラメーター

// 係員有効データ登録数
#define	MNT_ATTEND_REGSTER_COUNT	10	// 画面で登録可能なデータ数
#define MNT_MIN(a,b)	((a) < (b) ? (a) : (b))

#define	PRM_LOKNO_MIN	100	// 装置内連番の有効範囲の下限
#define	PRM_LOKNO_MAX	315	// 装置内連番の有効範囲の上限

//---------------------------------
// 券ﾃﾞｰﾀ表示用
//---------------------------------
struct CrServiceRec {
	ushort	ParkNo;				// 駐車場No.
	ushort	Kind;
	ushort	ShopNo;
	uchar	StartDate[3];
	uchar	EndDate[3];
	ushort	Status;
};

struct CrPrepaidRec {
	ushort	ParkNo;				// 駐車場No.
	ushort	Mno;
	uchar	IssueDate[3];
	ulong	SaleAmount;
	ulong	RemainAmount;
	ushort	LimitAmount;
	ulong	CardNo;
};

struct CrPassRec {
	ushort	ParkNo;
	ushort	Code;
	ushort	Kind;
	uchar	StartDate[3];
	uchar	EndDate[3];
	ushort	Status;
	uchar	WriteDate[4];
};
struct CrKaitikRec {
	ushort	ParkNo;				// 駐車場No.
	ushort	TanRyo;				// 単位金額
	ushort	LimDosu;			// 制限度数
	uchar	StartDate[3];		// 有効開始日
	uchar	EndDate[3];			// 有効終了日
	ushort	Kaisu;				// 回数（残り）
	uchar	WriteDate[4];		// 処理日時
};

extern union CrDspRec {

	struct	CrServiceRec service;
	struct	CrPrepaidRec prepaid;
	struct	CrPassRec pass;
	struct	CrKaitikRec	kaiticket;

} cr_dsp;

typedef struct{
	short	Section;							// 共通パラメータのセクション番号
	short	Address;							// 共通パラメータのアドレス番号
	char	Assign;								// 比較する位置
	long	Value;								// 比較する値
}CeMenuPara;

union ioLogNumberInfo{
	ulong		n;
	struct{
		ulong	sdat:13;		// 最古日時情報保存領域
		ulong	edat:13;		// 最新日時情報保存領域
		ulong	evnt: 6;		// 最新事象発生日時保存位置
	}log;
};

//---------------------------------
// 操作ログ登録用
//---------------------------------
extern ushort	OpelogNo;

extern ushort	OpelogNo2;


/* error information */
typedef struct {
	long	errmsg;			/* error message to display */
	ulong	address;		/* detail of error */
}t_SysMnt_ErrInfo;

/* error message number */
#define	_SYSMNT_ERR_NONE			27
#define	_SYSMNT_ERR_COM				29
#define	_SYSMNT_ERR_NO_DATA			30
#define	_SYSMNT_ERR_INVALID_DATA	31
#define	_SYSMNT_ERR_DATA_LOCKED		32
#define	_SYSMNT_ERR_WRITE			33
#define	_SYSMNT_ERR_ERASE			34
#define	_SYSMNT_ERR_VERIFY			36

extern	void	sysmnt_Backup(t_SysMnt_ErrInfo *errinfo);
extern	void	sysmnt_Restore(t_SysMnt_ErrInfo *errinfo);
extern	ushort	parameter_upload_chk(void);

//---------------------------------
// 設定FLASH書き換え用
//---------------------------------
extern ushort	SetChange;
#define		SysMnt_Work_Size		65536

typedef struct {
	long	from;	// マスク開始アドレス
	long	to;		// マスク終了アドレス
} PRM_MASK;

extern	const	PRM_MASK param_mask[];		// 設定全体
extern	const	PRM_MASK param_mask31_0[];	// 昼夜帯
extern	const	PRM_MASK param_mask31_1[];	// 逓減帯

extern	const	short	prm_mask_max;	// マスクデータ件数
extern	const	short	prm_mask31_0_max;	// マスクデータ件数：昼夜帯
extern	const	short	prm_mask31_1_max;	// マスクデータ件数：逓減帯

// MH810100(S) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
//---------------------------------
// QRバージョン表示用
//---------------------------------
enum{
	QR_HINBAN,
	QR_SERIAL,
	QR_VERSION,
	QR_VER_MAX,
};
#define QR_VER_RESULT_MAX				QR_VER_MAX

// GG120600(S) // Phase9 遠隔メンテナンス用
//typedef enum{
//	FTP_REQ_NONE,			// 0:要求なし
//	FTP_REQ_NORMAL,			// 1:FTP要求あり
//	FTP_REQ_WITH_POWEROFF,	// 2:FTP要求あり(電断要)
//} eFTP_REQ_TYPE;
// GG120600(E) // Phase9 遠隔メンテナンス用

//---------------------------------
// QRデータ確認用
//---------------------------------
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
////割引券
// enum{
//	QR_DISCOUNT_MAKE_DATE,	// QR発行年/月/日
//	QR_DISCOUNT_MAKE_TIME,	// QR発行時/分/秒
//	PARKING_LOT,	// 駐車場Lot番号
//	DISCOUNT_KIND,	// 割引種別
//	DISCOUNT_CLASS,	// 割引区分
//	SHOP_NUMBER,	// 店番号
//	CARD_START_DATE,		// 有効開始年/月/日
//	CARD_START_HOUR,		// 有効開始時/分/秒
//// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//	FACILITY_CODE,	// 施設コード
//	SHOP_CODE,		// 店舗コード
//	POS_NUMBER,		// 店舗内端末番号
//	ISSUE_NUMBER,	// レシート発行追い番
//// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//
//	QR_DISCOUNT_DATA_MAX
// };
////買上券
// enum{
//	QR_AMOUNT_MAKE_DATE,	// QR発行年/月/日
//	QR_AMOUNT_MAKE_TIME,	// QR発行時/分/秒
//	FACILITY_NUMBER,// 施設番号
//	QR_AMOUNT_SHOP_NUMBER,	// 店番号
//	MACHINE_NUMBER,	// 店舗内端末番号
//	RECIETE_NUMBER,	// レシート発行追い番
//	QR_AMOUNT_HEAD = RECIETE_NUMBER,
//	BARCODE_TEYPE_1, // バーコードタイプ１
//	DATA_1,			// データ１
//	BARCODE_TEYPE_2, // バーコードタイプ２
//	DATA_2,			// データ２
//	BARCODE_TEYPE_3, // バーコードタイプ３
//	DATA_3,			// データ３
//	QR_AMOUNT_DATA_MAX
// };
// #define	BAR_DATA_OFFSET		(DATA_1 - BARCODE_TEYPE_1)
#define	BAR_DATA_OFFSET	(2)
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

// GG120600(S) // Phase9 遠隔メンテナンス用
//extern void mnt_SetFtpFlag( eFTP_REQ_TYPE req );
// GG120600(E) // Phase9 遠隔メンテナンス用
extern eFTP_REQ_TYPE mnt_GetFtpFlag( void );

typedef struct {
	ushort				err;		// エラー
	ushort				id;			// QRコードID
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ushort				type;		// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	lcdbm_rsp_QR_com_u	QR_Data;	// QR情報(パース済み)
} _backdata;

typedef struct {
	int			cnt;
	_backdata	backdata[5];	// 過去データ
} _ticketdata;
extern _ticketdata	ticketdata;

// MH810100(E) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)
#endif	// _MNT_DEF_H_
