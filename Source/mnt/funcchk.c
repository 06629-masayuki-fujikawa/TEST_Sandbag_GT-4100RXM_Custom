/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽ動作ﾁｪｯｸ処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	<trsocket.h>
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
#include	"mdl_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"tbl_rkn.h"
#include	"Lkcom.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ifm_ctrl.h"
#include	"appserv.h"
#include	"fla_def.h"
#include	"ksg_def.h"
#include	"rauconstant.h"
#include	"raudef.h"
#include	"pip_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"ntcom.h"
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"FlashSerial.h"
#include	"updateope.h"

/*	CRWでサブCPUをサポートする上で以下の相違点があります。
	①メモリチェック
	　サブCPUメモリチェックに対応
	②NT-NETチェック
	　IBK／サブCPUの２経路に対応
	③バージョンチェック
	　メイン、サブバージョンの表示
	④サブCPU汎用通信ラインチェック
	CRW処理は原則GT-7000から(一部不要な過去履歴は削除して)そのまま移植しました。
*/
#include	"lkmain.h"
#include	"remote_dl.h"
#include	"IFM.h"
#include	"flp_def.h"
#include	"toSdata.h"

/*----------------------------------------------------------------------*/
/*		関数ﾌﾟﾛﾄﾀｲﾌﾟ宣言												*/
/*----------------------------------------------------------------------*/
ushort	FunChk_Sig( void );
static void	inSigStsDsp( uchar req, uchar ionsts );				// 入力信号状態表示
static void	outSigStsDsp( uchar *ionsts );						// 出力信号状態表示
ushort	FunChk_Swt( void );
ushort	FunChk_Kbd( void );
ushort	FunChk_Sht( void );
ushort	FunChk_Led( void );
ushort	FunChk_Prt( void );
ushort	FunChk_JRPrt( ushort event );
ushort	FPrnChk_tst( ushort event );
ushort	FPrnChk_ryo_tst( void );						//領収証印字ﾃｽﾄ
ushort	FPrnChk_tst_rct( void );						//ﾚｼｰﾄﾃｽﾄ印刷(ﾒﾆｭ-)
void FPrnChk_Testdata_mk( char sw,Receipt_data *dat );	//ﾃｽﾄ印字用擬似ﾃﾞｰﾀ作成
ushort	FPrnChk_sts( ushort event );
ushort	FPrnChk_logo( ushort event );
// MH810104 GG119201(S) 電子ジャーナル対応
ushort	FPrnChk_RW(void);
ushort	FPrnChk_SD(void);
ushort	FPrnChk_Ver(void);
// MH810104 GG119201(E) 電子ジャーナル対応
ushort	FunChk_Rdr( void );
ushort	FRdrChk_RW( void );
ushort	FRdrChk_Pr( void );
ushort	RdrChk_ErrDsp( uchar ercd, ulong errbit );
void	Rdr_err_dsp( uchar pos, uchar *data );
ushort	FRdrChk_Sts( void );
void	Rdr_sts_dsply( uchar pos1, uchar pos2, uchar *sta );
ushort	FRdrChk_Ver( void );
ushort	FunChk_Cmc( void );
ushort	CmcInchk( void );
void	InchkDsp( void );
ushort	CmcInchk2( uchar* );
uchar	bcdadd( uchar data1, uchar data2 );
ushort	CmcOutchk( void );
ushort	CmcStschk( void );
void	Cmc_err_dsp( uchar pos, uchar *data );
ushort	CmcVerchk( void );
ushort	FunChk_Ann( void );
ushort	Ann_chk2(void);
ushort	FunChk_BNA( void );
ushort	BnaInchk( void );
ushort	BnaStschk( void );
ushort	BnaVerchk( void );
ushort	FunChk_Mck( void );
ushort	FunChk_Ifb( uchar );
ushort	FunChk_CRRVer( void );
ushort	FunChk_CrrComChk( void );
ushort	FunChk_Version( void );
ushort FncChk_Suica( void );
ushort Com_Chk_Suica( void );
ushort Log_Print_Suica( uchar print_kind );
ushort Log_Print_Suica_menu( void );
ushort Log_Print_Suica2( void );
ushort	FncChk_CCom( void );
ushort	FncChk_bufctrl( int type );
ushort	dsp_unflushed_count(uchar type, uchar page);
void	clr_dsp_count(uchar page);
uchar	dsp_ccomchk_cursor(int index, ushort mode, uchar page);
ushort FncChk_ccomtst( void );
ushort FncChk_CcomApnDisp( void );
ushort FncChk_CcomAntDisp( void );
static void antsetup_dsp( short * dat, short pos );
ushort FncChk_cOibanClr( void );
void cOibanClrDsp( void );
void cOibanCursorDsp( uchar index, ushort mode );
ushort FncChk_CcomApnSelect( void );
ushort FncChk_CcomApnSet( ushort );
ushort FncChk_CComRemoteDLChk(void);
static unsigned short	FRdrChk_DipSwitch( void );
void	FRdrChk_Wait( void );
ushort Ryokin_Setlog_Print( void );
unsigned short	FucChk_DebugMenu(void);
unsigned short	Fchk_TimeCpuDnmDisp( void );
void	FunChk_TimeCpuEepromDumpDisp( void );
unsigned short	Fchk_Stack_StsPrint( void );
void	Fchk_Stak_chek( void );
ushort	FncChk_PiP(void);
ushort	FncChk_pipclr(void);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
ushort	FunChk_Ntnet( void );
ushort	check_nt_dtclr( void );
void	DataCntDsp( ushort dataCnt, ushort type, ushort	line );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//ushort FncChk_Cre( void );
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
// MH810100(S) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
ushort  FncChk_QRchk( void );
// MH810100(E) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
// MH810100(S) Y.Yamauchi 20191010 車番チケットレス(メンテナンス)	
ushort	FncChk_QRread( void );
ushort	FncChk_QRinit(void);
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// static ushort	FncChk_QRread_result(uchar result);
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// static ushort FncChk_QRread_result(ushort updown_page);
static ushort FncChk_QRread_result(ushort updown_page, ushort updown_page_max);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
ushort	FncChk_QRverChk(void);
static ushort	FncChk_QRverchk_result(uchar result);
ushort	FncChk_QRinitChk(void);
ushort	FncChk_RealChk(void);
ushort	RealT_FncChk_Unsend(int type);
ushort FncChk_CenterChk(void);
static ushort FncChk_realt_OibanClear(void);
ushort real_dsp_unflushed_count(uchar type, uchar page);
void   realt_clr_dsp_count(uchar page);
uchar realt_dsp_ccomchk_cursor(int index, ushort mode, uchar page);
static ushort FncChk_DC_Chk(void);
static ushort FncChk_DC_UnsendClear(void);
static ushort FncChk_DC_ConnectCheck( void );
static ushort FncChk_DC_OibanClear( void );
// MH810100(E) Y.Yamauchi 20191010 車番チケットレス(メンテナンス)
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//static ushort	FncChk_CreUnSend( void );
//static void cre_unsend_data_dsp( struct DATA_BK * );
//static short cre_unsend_del_dsp( char );
//static ushort	FncChk_CreSaleNG( void );
//void	FncChk_CreSaleNG_fnc( char );
//static void cre_saleng_show_data( short );
//static void cre_saleng_num_dsp( uchar, char );
//static void	time_dsp2( ushort, ushort, ushort, ushort, ushort );
//static void cre_saleng_data_dsp( short );
//static void	date_dsp3( ushort, ushort, ushort, ushort, ushort, ushort );
//static short cre_saleng_del_dsp( short, char );
//static void	CRE_SaleNG_Delete( void );
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
static ushort	FncChk_CComLongParkAllRel( void );
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

// MH321800(S) D.Inaba ICクレジット対応
static ushort FncChk_EcReader( void );
static unsigned short EcReaderBrandCondition( void );
// MH810105(S) MH364301 QRコード決済対応
//static void 	dsp_brand_condition( int index );
static ushort	dsp_brand_condition(int index);
static ushort	EcReaderSubBrandCondition(uchar sub_idx);
static void		dsp_sub_brand_condition(uchar sub_idx, uchar current_page, uchar max_page);
// MH810105(E) MH364301 QRコード決済対応
static ushort	EcReader_Mnt(void);
static uchar	EcReader_Mnt_CommResult( uchar mode );
static void 	EcReader_Mnt_ResetDisp(void);
// MH321800(E) D.Inaba ICクレジット対応

enum {
// MH810100(S) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)	
//	CCOM_DATA_INCAR,			// 入庫
// MH810100(E) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)
	CCOM_DATA_TOTAL,			// 集計
	CCOM_DATA_SALE,				// 精算
	CCOM_DATA_COIN,				// コ金庫計
	CCOM_DATA_ERR,				// ｴﾗｰ
	CCOM_DATA_NOTE,				// 紙金庫計
	CCOM_DATA_ALARM,			// ｱﾗｰﾑ
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	CCOM_DATA_CAR_CNT,			// 駐車台数
//	CCOM_DATA_MONITOR,			// ﾓﾆﾀｰ
//	CCOM_DATA_MONEY, 			// 金銭管理
//	CCOM_DATA_OPE,				// 操作
//	CCOM_DATA_TURI, 			// 釣銭管理
//	CCOM_DATA_RMON,				// 遠隔監視
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	CCOM_DATA_LPARK,			// 長期駐車
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	CCOM_DATA_MONEY, 			// 金銭管理
	CCOM_DATA_MONITOR,			// ﾓﾆﾀｰ
	CCOM_DATA_TURI, 			// 釣銭管理
	CCOM_DATA_OPE,				// 操作
	CCOM_DATA_RMON,				// 遠隔監視
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	CCOM_DATA_MAX
};
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//#define CCOM_EXIST_DATA_MAX		(CCOM_DATA_OPE+1)
#define CCOM_EXIST_DATA_MAX		CCOM_DATA_OPE	// Phase1には釣銭管理がないため「+1」が不要
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)	
enum{
	RTCOM_DATA_PAY,				// RT精算データ
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	RTCOM_DATA_RECEIPT,			// RT領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	RT_DATA_MAX
};
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// #define RT_EXIST_DATA_MAX		(RTCOM_DATA_PAY+1)
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
enum{
	DC_DATA_QR,					// QR確定・取消データ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	DC_DATA_LANE,				// レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	DC_DATA_MAX
};
// MH810100(E) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)	
CRE_SALENG	cre_saleng_work;				// 売上拒否ﾃﾞｰﾀ１０件分編集用ワーク

t_NtBufCount IBK_BufCount;					// IBKテーブルデータ件数
uchar	IBK_ComChkResult;					// センター通信テスト結果コード

static ulong PreCount[CCOM_DATA_MAX];		// IBKテーブルデータ件数更新用
// MH810100(S) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)	
static ulong RT_PreCount[RT_DATA_MAX];		// リアルタイム通信テーブルデータ件数更新用
static ulong DC_PreCount[DC_DATA_MAX];		// リアルタイム通信テーブルデータ件数更新用
// MH810100(E) Y.Yamauchi 2019/10/29 車番チケットレス(メンテナンス)	

static ushort	MemoryCheck(void);
static void		FunChk_Draw_SubCPU_Version(void);

static uchar	wlcd_buf[31];
static Receipt_data	TestData;				// 擬似精算完了データ作成ｴﾘｱ
static uchar	APN_str[33];
static uchar	IP_str[20];
const unsigned char APN_TBL[][33]={
	"parking.ams-amano.co.jp         ",		//ＡＭＳ用ＡＰＮ 
	"sf.amano.co.jp                  ",		//アマノ用ＡＰＮ
	"parkingweb.jp                   ",		//駐車場センター用ＡＰＮ
								};
ushort FncChk_Cappi( void );
ushort	check_cappi_seqclr( void );


static ushort FunChk_CrrBoardCHK( void );
static ushort FunChk_FlpCHK( void );

static ushort	CarFuncSelect( void );
ushort	Ftp_ConnectChk( void );
ushort	ParameterUpload( uchar );
static ushort	Lan_Connect_Chk( void );
static int		pingStart (short * dat);
void pingOpenCB (int socketDescriptor);
static void		Lan_Connect_dsp( void );
static void		ping_dsp( short * dat, short pos );
static ushort	FunChk_FlapSensorControl( void );
static ushort	FunChk_FlapLoopData( void );
static ushort	FunChk_FUNOPECHK( void );
unsigned short avm_test_no[2];
char	avm_test_ch;
short	avm_test_cnt;
// MH810100(S) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)
static ushort	GetFromCheckSum( void );
#define	FROM_IN_SECTOR_SIZE		4096			// CPUプログラムフラッシュセクタサイズ(byte)
#define FROM_IN_ADDRESS_START	0xFFE00000L		// 内部FROM プログラム領域先頭アドレス
#define FROM_IN_ADDRESS_END		0xFFFFFFFFL		// 内部FROM プログラム領域最終アドレス
#define	FROM_PROG_SIZE			FROM_IN_ADDRESS_END - FROM_IN_ADDRESS_START + 1	// プログラム領域サイズ
static uchar ProgramRomBuffer[FROM_IN_SECTOR_SIZE];
// MH810100(E) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)

/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽ初期処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkInit( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FncChkInit( void )
{
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ｼｽﾃﾑﾒﾝﾃﾅﾝｽﾒｲﾝ処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short FncChkMain( void )
{
	unsigned short	usFncEvent;
	char	wk[2];
	char	org[2];
	uchar	i;
	uchar	crr_check;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	FncChkInit();
	// 有効なCRR基板があるのかチェック
	crr_check = 0;
	for (i=0; i < IFS_CRR_MAX; i++) {
		if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){
			crr_check = 1;	// 有効なCRR基板あり
			break;
		}
	}

	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[00] );			// [00]	"＜動作チェック＞　　　　　　　"

// MH321800(S) D.Inaba ICクレジット対応 (決済リーダチェック追加)
//		usFncEvent = Is_MenuStrMake( 0 );
		if( isEC_USE() ){
			// 決済リーダ接続あり
			usFncEvent = Is_MenuStrMake( 2 );
		} else {
			// 従来
			usFncEvent = Is_MenuStrMake( 0 );
		}
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダチェック追加)
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		OpelogNo = 0;
		SetChange = 0;

		switch( usFncEvent ){

			case SWT_CHK:						// 　１．スイッチチェック
				wopelg( OPLOG_SWCHK, 0, 0 );			// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 6;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Swt();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(S)
			case KBD_CHK:						// 　キー入力チェック
				wopelg( OPLOG_KEYCHK, 0, 0 );			// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 1;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Kbd();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
			case LCD_CHK:						// 　ＬＣＤチェック
				wopelg( OPLOG_LCDCHK, 0, 0 );			// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 2;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Lcd();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(E)
			case LED_CHK:						// 　ＬＥＤチェック
				wopelg( OPLOG_LEDCHK, 0, 0 );			// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 3;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Led();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(S)
			case SHT_CHK:						// 　シャッターチェック
				wopelg( OPLOG_SHUTTERCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 7;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Sht();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(E)
			case SIG_CHK:						// 　入出力信号チェック
				wopelg( OPLOG_SIGNALCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 8;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Sig();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

			case CMC_CHK:						// 　コインメックチェック
				wopelg( OPLOG_COINMECHCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 5;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Cmc();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

			case BNA_CHK:						// 　紙幣リーダーチェック
				wopelg( OPLOG_NOTEREADERCHK, 0, 0 );	// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 4;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_BNA();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

			case PRT_CHK:						// 　プリンタチェック
				wopelg( OPLOG_PRINTERCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 9;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Prt();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

// MH810100(S) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
			case QR_CHK:						// 　ＱＲリーダーチェック
				wopelg( OPLOG_QRCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 16;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_QRchk();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
// MH810100(E) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)


//未使用です。(S)
			case RDR_CHK:						//	磁気リーダーチェック
				wopelg( OPLOG_READERCHK, 0, 0 );		// 操作履歴登録
				if( prm_get( COM_PRM,S_PAY, 21, 1, 3 ) != 0 ){		// 磁気ﾘｰﾀﾞｰ接続有？

					// 接続あり
					OPECTL.Ope_Mnt_flg	= 10;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
					usFncEvent = FunChk_Rdr();
					OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				}
				else{
					// 接続なし
					BUZPIPI();					// 接続なしの場合、磁気ﾘｰﾀﾞｰﾁｪｯｸ不可とする
				}
				break;
//未使用です。(E)
			case ANN_CHK:						// 	アナウンスチェック
				wopelg( OPLOG_ANNAUNCECHK, 0, 0 );	// 操作履歴登録

					// 接続あり
					OPECTL.Ope_Mnt_flg	= 11;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
					usFncEvent = FunChk_Ann();
					OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

			case MEM_CHK:						// メモリーチェック
				wopelg( OPLOG_MEMORYCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 12;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Mck();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(S)
			case LCK_CHK:						// ロック装置チェック
				wopelg( OPLOG_IFBOARDCHK, 0, 0 );		// 操作履歴登録
				if( GetCarInfoParam() & 0x01 ){		// 駐輪場併設あり
					OPECTL.Ope_Mnt_flg	= 13;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
					usFncEvent = FunChk_Ifb( 0 );
					OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				}else{
					// 接続なし
					BUZPIPI();					// 接続なしの場合、ﾛｯｸ装置ﾁｪｯｸ不可とする
				}
				break;
//未使用です。(E)
			case CCM_CHK:							// センター通信チェック
				wopelg( OPLOG_CCOMCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 21;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_CCom();
				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
// MH810100(S) Y.Yamauchi 2019/10/18 車番チケットレス(メンテナンス)
			case REAL_CHK:							// リアルタイム通信チェック
				wopelg( OPLOG_REALCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 27;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_RealChk();
				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;

			case DC_CHK:							// DC-NET通信チェック
				wopelg( OPLOG_DCLCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 29;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_DC_Chk();
				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
// MH810100(E) Y.Yamauchi 2019/10/18 車番チケットレス(メンテナンス)
			case VER_CHK:						// バージョンチェック
				wopelg( OPLOG_VERSIONCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 20;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Version();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
				break;

//未使用です。(S)
			case FLP_CHK:							//フラップ装置チェック
				if ( GetCarInfoParam() & 0x04 ){	// フラップ接続設定有り？
					wopelg( OPLOG_IFBOARDCHK, 0, 0 );		// 操作履歴登録
					OPECTL.Ope_Mnt_flg	= 14;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
					usFncEvent = FunChk_FlpCHK();
					OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				}else{
					BUZPIPI();					
				}
				break;
//			case CRR_CHK:						//ＣＲＲチェック
//				if ( GetCarInfoParam() & 0x04 ){// CRR接続有り？ */
//					OPECTL.Ope_Mnt_flg	= 25;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
//					usFncEvent = FunChk_CrrBoardCHK();
//					OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
//				}else{
//					BUZPIPI();					
//				}
//				break;
			case CRR_CHK:						// フラップ制御基板チェック
				if ( 1 == crr_check ){			// CRR接続有り？ */
					OPECTL.Ope_Mnt_flg	= 25;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
					usFncEvent = FunChk_CrrBoardCHK();
					OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				}else{
					BUZPIPI();					
				}
				break;
//未使用です。(E)

			case CAR_FUNC_SELECT:
				OPECTL.Ope_Mnt_flg	= 26;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = CarFuncSelect();
				OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
			case LAN_CONNECT_CHK:
				OPECTL.Ope_Mnt_flg	= 28;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = Lan_Connect_Chk();
				OPECTL.Ope_Mnt_flg	= 0;	// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
			case FUNOPE_CHK:					// ＦＡＮ動作チェック
				wopelg( OPLOG_FANOPE_CHK, 0, 0 );	// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 50;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_FUNOPECHK();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
//未使用です。(S)				
			case SUICA_CHK:						//交通系ＩＣリーダーチェック
				wopelg( OPLOG_SUICACHK, 0, 0 );	// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 30;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_Suica();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
			case PIP_CHK:							// Park i Proチェック
				wopelg( OPLOG_PIPCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 19;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_PiP();
				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
			case CRE_CHK:							// クレジットチェック
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//				OPECTL.Ope_Mnt_flg	= 22;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
//				usFncEvent = FncChk_Cre();
//				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				BUZPIPI();
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
				break;
// MH321800(S) D.Inaba ICクレジット対応 (決済リーダチェック追加)
			case ECR_CHK:							// 決済リーダチェック
				wopelg( OPLOG_CHK_EC, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 31;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FncChk_EcReader();
				OPECTL.Ope_Mnt_flg	= 0;			// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
// MH321800(E) D.Inaba ICクレジット対応 (決済リーダチェック追加)				
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
			case NTNET_CHK:						// １６．ＮＴ－ＮＥＴチェック
				wopelg( OPLOG_NTNETCHK, 0, 0 );		// 操作履歴登録
				OPECTL.Ope_Mnt_flg	= 15;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞｾｯﾄ
				usFncEvent = FunChk_Ntnet();
				OPECTL.Ope_Mnt_flg	= 0;		// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞﾘｾｯﾄ
				break;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFncEvent;
				break;
//未使用です。(E)

			default:
				break;
		}
		if(( OpelogNo )||( SetChange )){
			if( OpelogNo ) wopelg( OpelogNo, 0, 0 );		// 操作履歴登録
			if( UserMnt_SysParaUpdateCheck( OpelogNo ) ){
				usFncEvent = parameter_upload_chk();
			}
		}

		if( usFncEvent == MOD_CHG || usFncEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of Funcchk Operation
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFncEvent == MOD_CHG ){
		if(  usFncEvent == MOD_CHG || usFncEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFncEvent;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| IN/OUT Sygnal Check                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Sig( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned char	SIG_IStsCol[] = { 14 };						// 入力信号表示ｶﾗﾑ
// MH810100(S) S.Takahashi 2019/12/18
//const	unsigned char	SIG_OStsCol[] = { 5,8,11,14,17,20,23 };		// 出力信号表示ｶﾗﾑ
const	unsigned char	SIG_OStsCol[] = { 2,6,10,14,18,22,26 };		// 出力信号表示ｶﾗﾑ
// MH810100(E) S.Takahashi 2019/12/18

ushort	FunChk_Sig( void ){

	unsigned short	ret;			// 戻り値
	short			msg = -1;		// 受信ﾒｯｾｰｼﾞ
	char			i;				// ﾙｰﾌﾟｶｳﾝﾀｰ
	unsigned char	page;			// 表示画面（０：入力信号画面、１：出力信号画面）
	unsigned char	pos;			// 出力信号画面カーソル位置（0～6：出力信号1～7）
	unsigned char	outsig_req;		// ＯＮ／ＯＦＦ要求パラメータ
	unsigned char	insts;			// 入力信号状態(RXI_IN)
	unsigned char	outsts[7];		// 出力信号状態(出力信号1～7)
	unsigned short  sd_exioport;	// 出力ﾎﾟｰﾄの初期値を保持
	unsigned char	col;
	
	sd_exioport	=	SD_EXIOPORT;

	insts = ExIOSignalrd(15);						// RXI-1入力信号状態リード
	for( i = 0; i < 7 ; i++ )
	{
		outsts[i] = 0x01 & (SD_EXIOPORT>>(14-i));	// 出力信号1～7状態リード
	}

	page = 0;										// 画面１（入力）
	pos  = 0;										// カーソル位置初期化
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[11] );				// [11]	"＜入出力信号チェック＞　　　　"
	inSigStsDsp( 0, insts );													// 信号状態表示（入力）
	Fun_Dsp( FUNMSG[36] );														// [36]	"入／出                   終了 "
	Lagtim ( OPETCBNO, 21, 25 );												// 入力信号ﾁｪｯｸﾀｲﾏｰ起動

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );
		ret = 0;

		switch( msg){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;														// 入出力信号チェック処理終了
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

			case KEY_MODECHG:

				BUZPI();
				ret = MOD_CHG;														// 入出力信号チェック処理終了

				break;


			case KEY_TEN_F1:	// Ｆ１:"入／出"（表示画面切替）
				BUZPI();
				pos  = 0;															// カーソル位置初期化
				if( page == 0 ){
					// 信号状態表示（出力）
					page = 1;
					outSigStsDsp( &outsts[0] );
// MH810100(S) S.Takahashi 2020/02/16 #3839 【出力】に遷移時、「1」が赤字にならない
//					grachr( 3,  5, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][5] );
					grachr( 3,  SIG_OStsCol[0], 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][SIG_OStsCol[0]] );
// MH810100(E) S.Takahashi 2020/02/06 #3839 【出力】に遷移時、「1」が赤字にならない
					Fun_Dsp( FUNMSG[37] );											// [37]	"入／出  ←    →  ON/OFF 終了 "
				}
				else{
					// 信号状態表示（入力）
					page = 0;
					inSigStsDsp( 0, insts );
					Fun_Dsp( FUNMSG[36] );											// [36]	"入／出                   終了 "
					Lagtim ( OPETCBNO, 21, 25 );								// 入力信号ﾁｪｯｸﾀｲﾏｰ起動
				}
				break;

			case KEY_TEN_F2:	// Ｆ２:"←"（ｶｰｿﾙ移動：左）
				// 信号状態表示（出力)の場合
				if(page == 1)
				{
					BUZPI();
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
					if( pos == 0 )												// カーソル位置更新
					{
						pos = 6;
					}
					else{
						pos--;
					}
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
				}
				break;

			case KEY_TEN_F3:	// Ｆ３:"→"（ｶｰｿﾙ移動：右）
				// 信号状態表示（出力)の場合
				if(page == 1)
				{
					BUZPI();
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
					if( pos == 6 ){													// カーソル位置更新
						pos = 0;
					}
					else{
						pos++;
					}
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
				}
				break;

			case KEY_TEN_F4:	// Ｆ４:"ON/OFF"
				// [出力]画面表示中
				if( page == 1 )
				{
					BUZPI();
					if( outsts[pos] == 0 ){
						// 現在状態＝ＯＦＦ
						outsig_req = outsts[pos] = 1;								// ＯＮ要求
					}
					else{
						// 現在状態＝ＯＮ
						outsig_req = outsts[pos] = 0;								// ＯＦＦ要求
					}
// MH810100(S) S.Takahashi 2019/12/18
//					grachr( 4, (ushort)SIG_OStsCol[pos], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[outsig_req] );	// ON/OFF表示切替
					grachr( 4, (ushort)SIG_OStsCol[pos], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[outsig_req] );	// ON/OFF表示切替
// MH810100(E) S.Takahashi 2019/12/18
					ExIOSignalwt( 14-pos , outsig_req);									// 拡張IOﾎﾟｰﾄ信号出力
				}
				else{
					// [入力]画面表示中
					;																	//なにもしない
				}
				break;

			case KEY_TEN_F5:	// Ｆ５:終了
				BUZPI();
				ret = MOD_EXT;															// 入出力信号チェック処理終了

				break;
			case TIMEOUT21:																/* 入力信号チェック */
				insts = ExIOSignalrd(15);												// RXI-1入力信号状態リード
				inSigStsDsp( 1, insts );												// 信号状態表示（入力）
				Lagtim ( OPETCBNO, 21, 25 );											// 入力信号ﾁｪｯｸﾀｲﾏｰ起動
				break;
			default:

				break;
		}
		if( ret != 0 ){
			break;
		}
	}
	EXIO_CS4_EXIO = SD_EXIOPORT = sd_exioport;										// 出力ﾎﾟｰﾄ値を元に戻す

	return( ret );
}

static	uchar	insts_now;		// 入力信号状態リードバッファ

static void	inSigStsDsp( uchar req, uchar ionsts )				// 入力信号状態表示
{
	uchar	i;					// ﾙｰﾌﾟｶｳﾝﾀｰ

	if( req == 0 ){
		// 表示要求画面＝入力
		for( i = 1 ; i < 6 ; i++ ){							// ライン１～５クリア
			displclr( (ushort)i );
		}

		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[0] );
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[2] );

		if( ionsts == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
			grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		else{
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
			grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
		}
	}
	else{
		// 表示要求種別＝変化した入出力信号状態のみ
		if( insts_now != ionsts ){				// 現在状態≠表示状態？
			insts_now = ionsts;

			if( ionsts == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//				grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
				grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
			}
			else{
// MH810100(S) S.Takahashi 2019/12/18
//				grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
				grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
			}
		}
	}
}
static void	outSigStsDsp( uchar *ionsts )						// 出力信号状態表示
{
	uchar	i;					// ﾙｰﾌﾟｶｳﾝﾀｰ

	// 表示要求画面＝出力
	for( i = 1 ; i < 6 ; i++ ){							// ライン１～５クリア
		displclr( (ushort)i );
	}

	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[1] );
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[3] );

	for( i = 0 ; i < 7 ; i++ ){							// 出力信号状態表示

		if( (*ionsts) == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_OStsCol[i], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
			grachr( 4, (ushort)SIG_OStsCol[i], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		else{
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_OStsCol[i], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
			grachr( 4, (ushort)SIG_OStsCol[i], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		ionsts++;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Switch Check                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Swt( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#define	FUNCHK_SWMAX	10

unsigned short	FunChk_Swt( void ){

	unsigned char	Sw_rd( char idx );

	short			msg = -1;
	char			i;
	int				page;
	unsigned char	sts[FUNCHK_SWMAX];
	unsigned short	lin,col,cnt;
	unsigned char	page_chg;
	const unsigned char Stsknd[] = { 12,2,2,2,8,6,4,10 };				//DAT4_1[n]
	char			strbuf[10];
	unsigned long	dipswHex;

	page = 0;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[3] );			// [03]	"＜スイッチチェック＞　　　　　" */
	Fun_Dsp( FUNMSG[06] );						// [06]	"　▲　　▼　　　　　　　 終了 "

	for( i = 0; i < 6; i++ ){

		// 表示画面１（本体ﾊﾝﾄﾞﾙ～ドア開）
		sts[i] = Sw_rd( i );
		grachr( (unsigned short)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
		grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );
		page_chg = OFF;

		switch( msg){							// FunctionKey Enter

// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				break;

			case KEY_TEN_F5:					// F5:終了

				BUZPI();
				if( CP_MODECHG == 0 ){
					// ﾒﾝﾃ鍵状態：ＯＮ
					return( MOD_EXT );
				}
				else{
					// ﾒﾝﾃ鍵状態：ＯＦＦ
					return( MOD_CHG );
				}

				break;

			case KEY_TEN_F1:					// F1:"▲"

				BUZPI();

				page_chg = ON;					// 表示画面切替
				page--;
				if( page < 0 ){
					page = 1;
				}

				break;

			case KEY_TEN_F2:					// F2:"▼"

				BUZPI();

				page_chg = ON;					// 表示画面切替
				page++;
				if( page > 1 ){
					page = 0;
				}

				break;

			default:
				break;
		}
		if( page == 0 ){

			// 表示画面１（本体ﾊﾝﾄﾞﾙ～ドア開）

			if( page_chg == ON ){

				// 表示画面切替
				for( i = 0; i < 6; i++ ){		// 本体ﾊﾝﾄﾞﾙ～ドア開までの状態表示を行う
					sts[i] = Sw_rd( i );
					grachr( (unsigned short)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
					grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
				}
			}
			else{
				// ＳＷ状態変化
				for( i = 0; i < 6 ; i++ ){		// 本体ﾊﾝﾄﾞﾙ～ドア開までの状態表示を行う（変化したSWのみ状態表示を更新）
					if( sts[i] != Sw_rd( i ) ){
						sts[i] = Sw_rd( i );
						grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
					}
				}
			}
		}
		else{
			// 表示画面２（振動ｾﾝｻｰ～DIPSW4）

			if( page_chg == ON ){

				// 表示画面切替
				for( i = 6; i < FUNCHK_SWMAX; i++ ){		// 振動ｾﾝｻｰ～DIPSW4までの状態表示を行う

					if( i <= 9 ){
						// 振動ｾﾝｻｰ,紙幣ﾘｰﾀﾞｰ脱落,人体検知,ロータリースイッチ,ディップスイッチ
						lin = (i-5);
						col = 22;
						cnt	= 8;
						displclr(lin);
						grachr( lin, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
					}
					sts[i] = Sw_rd( i );
					if( i <= 7 ) {
						grachr( lin, col, cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
					}
					else if(i == 8){	// ロータリスイッチ
						memset(strbuf, 0, sizeof(strbuf));
						intoas( (unsigned char *)&strbuf, sts[i], 1 );
						grachr( 3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
					}
					else if(i == 9){	// ディップスイッチ
						memset(strbuf, 0, sizeof(strbuf));
						dipswHex = (unsigned long)sts[i];
						hextoas( (unsigned char *)&strbuf, dipswHex, 2 , 0);
						grachr( 4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
					}
				}
				displclr(5);
				displclr(6);
			}
			else{
				// ＳＷ状態変化
				for( i = 6; i < FUNCHK_SWMAX ; i++ ){		// 振動ｾﾝｻｰ～DIPSW4,人体検知までの状態表示を行う（変化したSWのみ状態表示を更新）
					if( sts[i] != Sw_rd( i ) ){
						sts[i] = Sw_rd( i );

						if( i <= 7 ){
							// 振動ｾﾝｻｰ～ｽｲｯﾁ3
							lin = (i-5);
							col = 22;
							cnt	= 8;
							grachr( lin, col, cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
						}
						else if(i == 8){	// ロータリスイッチ
							memset(strbuf, 0, sizeof(strbuf));
							intoas( (unsigned char *)&strbuf, sts[i], 1 );
							grachr( 3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
						}
						else if(i == 9){	// ディップスイッチ
							memset(strbuf, 0, sizeof(strbuf));
							dipswHex = (unsigned long)sts[i];
							hextoas( (unsigned char *)&strbuf, dipswHex, 2 , 0);
							grachr( 4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
						}
					}
				}
			}
		}
	}
}

// スイッチリード処理
unsigned char	Sw_rd(  char idx  )
{
	unsigned char ret;

	switch( idx ){		// スイッチ種別？

		case	0:		// 設定ＳＷ
			ret = ( CP_MODECHG ) ? 0 : 1;
			break;

		case	1:		// コインメックセットＳＷ
			ret = ( FNT_CN_DRSW ) ? 0 : 1;
			break;

		case	2:		// コイン金庫ＳＷ
			ret = ( FNT_CN_BOX_SW ) ? 0 : 1;
			break;

		case	3:		// 紙幣金庫ＳＷ
			ret = ( FNT_NT_BOX_SW ) ? 0 : 1;
			break;

		case	4:		// ドア開
			ret = ( OPE_SIG_DOOR_Is_OPEN ) ? 0 : 1;
			break;

		case	5:		// 振動センサー
			ret = FNT_VIB_SNS;
			break;

		case	6:		// 紙幣リーダー脱落センサー
			ret = ( FNT_NT_FALL_SW ) ? 0: 1;
			break;
		case	7:		// 人体検知
			ret = ( FNT_MAN_DET ) ? 1: 0;
			break;
		case	8:		// ロータリースイッチ
			ret = read_rotsw();
			break;
		case	9:		// ディップスイッチ
			ret = 0;
			ret = (key_dat[0].BYTE & 0x80) ? 0x10 : 0;	// RXFディップスイッチ1
			ret += (key_dat[0].BYTE & 0x40) ? 0x20 : 0;	// RXFディップスイッチ2
			ret += (key_dat[0].BYTE & 0x20) ? 0x40 : 0;	// RXFディップスイッチ3
			ret += (key_dat[0].BYTE & 0x10) ? 0x80 : 0;	// RXFディップスイッチ4
			ret += ExIOSignalrd(INSIG_DPSW0);			// RXMディップスイッチ1
			ret += ExIOSignalrd(INSIG_DPSW1) << 1;		// RXMディップスイッチ2
			ret += ExIOSignalrd(INSIG_DPSW2) << 2;		// RXMディップスイッチ3
			ret += ExIOSignalrd(INSIG_DPSW3) << 3;		// RXMディップスイッチ4
			break;
		default:		// その他
			ret = 0;
			break;
	}
	return(ret);
}


/*[]----------------------------------------------------------------------[]*/
/*| Key Board Check                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Kbd( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

unsigned short	FunChk_Kbd( void ){

	short			msg = -1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[4] );		/* // [04]	"＜キー入力チェック＞　　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[5] );		/* // [05]	"　入力キー：　　　　　　　　　" */
	grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[21] );		/* // [00]	"　　　　　　”  ”　　　　　　" */
	Fun_Dsp( FUNMSG[8] );					/* // [08]	"　　　　　　　　　　　　 終了 " */

	for( ; ; ){
		msg = GetMessage();					/*								*/
		switch( msg){						/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN9:
			case KEY_TEN_CL:
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
			case KEY_TEN_F3:					/* F3: */
			case KEY_TEN_F4:					/* F4: */
				BUZPI();
				grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[msg - 0x0130] );
				break;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[msg - 0x0130] );
				return( MOD_EXT );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Sht                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｼｬｯﾀｰﾁｪｯｸ処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	SHTCHK_CNT	1	// ﾁｪｯｸ対象ｼｬｯﾀｰ項目数
#define	SHTSTS_CLM	27	// ｼｬｯﾀｰ状態（開/閉）表示ｶﾗﾑ

unsigned short	FunChk_Sht( void ){

	unsigned short	ret = 0;				// 戻り値
	short			msg = -1;				// 受信ﾒｯｾｰｼﾞ
	unsigned char	sht_ope = OFF;			// ｼｬｯﾀｰ開閉操作規制ﾌﾗｸﾞ
	unsigned char	sw = 0;					// 巡回ﾃｽﾄ中ﾌﾗｸﾞ
	unsigned char	pos = 0;				// ｶｰｿﾙ表示位置
	unsigned char	sts[SHTCHK_CNT];		// ｼｬｯﾀｰ状態保存ﾜｰｸ（ﾃｽﾄ開始前の状態）	※１＝「閉」、２＝「開」
	unsigned char	sts_now[SHTCHK_CNT];	// ｼｬｯﾀｰ状態ﾜｰｸ（ﾃｽﾄ中の状態）			※０＝「閉」、１＝「開」
	char			counter = 0;
	#define AUTO_SHTCHK_MAX	5				// 自動チェックの最大実行回数

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHTSTR[0] );							// "＜シャッターチェック＞　　　　"
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );							// "　テスト：巡回ﾃｽﾄを開始します "
	Fun_Dsp( FUNMSG[46] );																		// "　▲　　▼　テスト 開/閉 終了 "

																								// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ現在状態をｾｰﾌﾞ後、「閉」とする
	sts[0]		= READ_SHT_flg;																	// 現在状態をｾｰﾌﾞ
	sts_now[0]	= 0;																			// ｼｬｯﾀｰ状態ｾｯﾄ「閉」
	read_sht_cls();																				// ｼｬｯﾀｰ「閉」
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHTSTR[1] );							// "　磁気ﾘｰﾀﾞｰｼｬｯﾀｰ 　　　→　　 "
	grachr( 2, SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[0] );					// ｼｬｯﾀｰ状態表示「閉」（反転表示）

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );							// ﾒｯｾｰｼﾞ受信

		switch( msg){											// 受信ﾒｯｾｰｼﾞ？

// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:									// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:									// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;


			case KEY_TEN_F3:									// Ｆ３

				BUZPI();
				if( sw != 0 ){

					// 巡回ﾃｽﾄ中の場合：Ｆ３＝停止

					sw = 0;
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "　テスト：巡回ﾃｽﾄを開始します "
					Fun_Dsp( FUNMSG[46] );														// "　▲　　▼　テスト 開/閉 終了 "
					Lagcan( OPETCBNO, 6 );														// 巡回ﾃｽﾄﾀｲﾏｰﾘｾｯﾄ
					Ope_EnableDoorKnobChime();
				}
				else{
					// 巡回ﾃｽﾄ中でない場合：Ｆ３＝ﾃｽﾄ

					sw = 1;

																								// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰを「閉」とする
					sts_now[0]	= 0;															// ｼｬｯﾀｰ状態ｾｯﾄ「閉」
					read_sht_cls();																// ｼｬｯﾀｰ「閉」
					grachr( 2, SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[0] );	// ｼｬｯﾀｰ状態表示「閉」

																								// ｺｲﾝ投入口ｼｬｯﾀｰを「閉」とする

					pos = SHTCHK_CNT-1;															// ｶｰｿﾙ位置初期化（初回ﾃｽﾄでｶｰｿﾙが先頭になるように最後位置をｾｯﾄする）

					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[12] );			// "　停止：巡回ﾃｽﾄを停止します 　"
					Fun_Dsp( FUNMSG[48] );														// "　　　　　　 停止 　　　 終了 "
					Lagtim( OPETCBNO, 6, 1*50 );												// 巡回ﾃｽﾄﾀｲﾏｰ6(1s)起動
					counter = 0;								// 巡回カウントクリア
					Ope_DisableDoorKnobChime();
				}

				break;

			case KEY_TEN_F4:									// Ｆ４（ON/OFF）

				if( sw == 0 && sht_ope == OFF ){				// 巡回ﾃｽﾄ中、または　ｼｬｯﾀｰ開閉操作規制中の場合、無効

					BUZPI();

					if( pos == 0 ){
						// ｶｰｿﾙ位置＝磁気ﾘｰﾀﾞｰｼｬｯﾀｰ
						if( sts_now[pos] == 0 ){				// 現在状態？
							sts_now[pos] = 1;					// 状態更新
							read_sht_opn();						// ｼｬｯﾀｰ「開」
						}
						else{
							sts_now[pos] = 0;					// 状態更新
							read_sht_cls();						// ｼｬｯﾀｰ「閉」
						}
					}
					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// ｼｬｯﾀｰ状態表示（反転表示）

					Lagtim( OPETCBNO, 7, 1*50 );				// ｼｬｯﾀｰ開閉操作規制ﾀｲﾏｰ7(1s)起動
					sht_ope = ON;								// ｼｬｯﾀｰ開閉操作規制ﾌﾗｸﾞｾｯﾄ（規制開始）
				}
				break;

			case TIMEOUT6:										// 巡回ﾃｽﾄﾀｲﾏｰﾀｲﾑｱｳﾄ

				if( sw != 0 ){									// 巡回ﾃｽﾄ中？

					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// 現在のｶｰｿﾙ位置：反転表示OFF

					if( pos == 0 ){
						// ｶｰｿﾙ位置＝磁気ﾘｰﾀﾞｰｼｬｯﾀｰ
						if( sts_now[pos] == 0 ){				// 現在状態？
							sts_now[pos] = 1;					// 状態更新
							read_sht_opn();						// ｼｬｯﾀｰ「開」
						}
						else{
							sts_now[pos] = 0;					// 状態更新
							read_sht_cls();						// ｼｬｯﾀｰ「閉」
						}
					}
					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// ｼｬｯﾀｰ状態表示（反転表示）

					if( pos && !sts_now[pos] ){
						if( ++counter >= AUTO_SHTCHK_MAX ){
							sw = 0;
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );	// "　テスト：巡回ﾃｽﾄを開始します "
							Fun_Dsp( FUNMSG[46] );				// "　▲　　▼　テスト 開/閉 終了 "
							Lagcan( OPETCBNO, 6 );				// 巡回ﾃｽﾄﾀｲﾏｰﾘｾｯﾄ
							break;
						}
					}
					Lagtim( OPETCBNO, 6, 1*50 );				// 巡回ﾃｽﾄﾀｲﾏｰ6(1s)起動
				}

				break;

			case TIMEOUT7:										// ｼｬｯﾀｰ開閉操作規制ﾀｲﾏｰﾀｲﾑｱｳﾄ

				sht_ope = OFF;									// ｼｬｯﾀｰ開閉操作規制ﾌﾗｸﾞﾘｾｯﾄ（規制解除）
				break;

			default:
				break;
		}

		if( ret != 0 ){											// ｼｬｯﾀｰﾁｪｯｸ終了？

			if( sw != 0 ){										// 巡回ﾃｽﾄ中？
				Lagcan( OPETCBNO, 6 );							// 巡回ﾃｽﾄﾀｲﾏｰ解除
			}

			if( sht_ope == ON ){								// ｼｬｯﾀｰ開閉操作規制中？
				Lagcan( OPETCBNO, 7 );							// ｼｬｯﾀｰ開閉操作規制ﾀｲﾏｰ解除
			}

			// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰをﾃｽﾄ開始前の状態に戻す
			if( sts[0] == 1 ){
				read_sht_cls();									// ｼｬｯﾀｰ「閉」
			}
			else{
				read_sht_opn();									// ｼｬｯﾀｰ「開」
			}
			// ｺｲﾝ投入口ｼｬｯﾀｰをﾃｽﾄ開始前の状態に戻す
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Led                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LEDﾁｪｯｸ処理                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	LEDSTS_CLM	27	// LED状態（ON/OFF）表示ｶﾗﾑ

unsigned short	FunChk_Led( void ){

	unsigned short	ret = 0;				// 戻り値
	short			msg = -1;				// 受信ﾒｯｾｰｼﾞ
	unsigned char	i;						// ﾙｰﾌﾟｶｳﾝﾀｰ
	unsigned char	sw = 0;					// 巡回ﾃｽﾄ中ﾌﾗｸﾞ
	unsigned char	pos = 0;				// ｶｰｿﾙ表示位置
	unsigned char	sts[LEDCT_MAX];			// LED状態保存ﾜｰｸ（ﾃｽﾄ開始前の状態）
	unsigned char	sts_now[LEDCT_MAX];		// LED状態ﾜｰｸ（ﾃｽﾄ中の状態）


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[6] );			// "＜ＬＥＤチェック＞　　　　　　"

// MH810100(S) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
//	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "　テスト：巡回ﾃｽﾄを開始します "
//	Fun_Dsp( FUNMSG[47] );														// "　▲　　▼　テストON/OFF 終了 "
	if(LEDCT_MAX > 1){
		grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "　テスト：巡回ﾃｽﾄを開始します "
		Fun_Dsp( FUNMSG[120] );														// 	"　　　　　　テストON/OFF 終了 ",
		}
		else{
			Fun_Dsp( FUNMSG[50] );														// 	"　　　　　　　　　ON/OFF 終了 ",
		}												
// MH810100(E) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)

	for( i = 0; i < LEDCT_MAX ; i++ ){			// 全LEDに対し現在の状態をｾｰﾌﾞ後、OFFとする
		sts[i] = IsLedReq( i );										// 現在状態ｾｰﾌﾞ
		sts_now[i] = 0;												// LED状態ｾｯﾄ（OFF）
		LedReq( i, 0 );												// LED制御（OFF）
		grachr( (ushort)(i + 1),  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LEDSTR[i] );			// LED名称表示
// MH810100(S) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
//		grachr( (ushort)(i + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED状態表示（OFF）
		grachr( (ushort)(i + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED状態表示（OFF）
// MH810100(E) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
	}

// MH810100(S) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
//	grachr( 1, LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );						// 初期ｶｰｿﾙ位置を反転表示
	grachr( 1, LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );						// 初期ｶｰｿﾙ位置を反転表示
// MH810100(E) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg){								// 受信ﾒｯｾｰｼﾞ？

// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

			case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:						// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:						// Ｆ１（▲）
// MH810100(S) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)
//				if( sw == 0 ){						// 巡回ﾃｽﾄ中の場合、無効
//
//					BUZPI();
//
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 現在のｶｰｿﾙ位置：反転表示OFF
//
//					if( pos == 0 ){					// ｶｰｿﾙ位置更新（上移動）
//						pos = LEDCT_MAX-1;
//					}
//					else{
//						pos--;
//					}
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 移動ｶｰｿﾙ位置：反転表示
//				}
//
// MH810100(S) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)
				break;

			case KEY_TEN_F2:						// Ｆ２（▼）
// MH810100(S) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)
//				if( sw == 0 ){						// 巡回ﾃｽﾄ中の場合、無効
//
//					BUZPI();
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 現在のｶｰｿﾙ位置：反転表示OFF
//
//					if( pos >= (LEDCT_MAX-1) ){		// ｶｰｿﾙ位置更新（下移動）
//						pos = 0;
//					}
//					else{
//						pos++;
//					}
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 移動ｶｰｿﾙ位置：反転表示
//				}
// MH810100(E) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)
				break;

			case KEY_TEN_F3:						// Ｆ３
// MH810100(S) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
				if(LEDCT_MAX > 1){
// MH810100(E) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
					BUZPI();
					if( sw != 0 ){

						// 巡回ﾃｽﾄ中の場合：Ｆ３＝停止

						sw = 0;
						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "　テスト：巡回ﾃｽﾄを開始します "
						Fun_Dsp( FUNMSG[47] );														// "　▲　　▼　テストON/OFF 終了 "
						Lagcan( OPETCBNO, 6 );														// 巡回ﾃｽﾄﾀｲﾏｰﾘｾｯﾄ
						Ope_EnableDoorKnobChime();
					}
					else{
						// 巡回ﾃｽﾄ中でない場合：Ｆ３＝ﾃｽﾄ

						sw = 1;

						for( i = 0; i < LEDCT_MAX ; i++ ){																// 全LEDをOFFとする
							sts_now[i] = 0;																				// LED状態ｾｯﾄ（OFF）
							LedReq( i, 0 );																				// LED制御（OFF）
// MH810100(S) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
//							grachr( (ushort)(i + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED状態表示
							grachr( (ushort)(i + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED状態表示
// MH810100(E) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
						}
						pos = LEDCT_MAX-1;							// ｶｰｿﾙ位置初期化（初回ﾃｽﾄでｶｰｿﾙが先頭になるように最後位置をｾｯﾄする）

						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[12] );			// "　停止：巡回ﾃｽﾄを停止します 　"
						Fun_Dsp( FUNMSG[48] );														// "　　　　　　 停止 　　　 終了 "
						Lagtim( OPETCBNO, 6, 1*50 );												// 巡回ﾃｽﾄﾀｲﾏｰ6(1s)起動
						Ope_DisableDoorKnobChime();
					}
// MH810100(S) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
				}
// MH810100(E) Y.Yamauchi 2019/10/15 車番チケットレス(メンテナンス)
				break;

			case KEY_TEN_F4:						// Ｆ４（ON/OFF）

				if( sw == 0 ){						// 巡回ﾃｽﾄ中の場合、無効

					BUZPI();
					if( sts_now[pos] == 0 ){		// 状態更新
						sts_now[pos] = 1;
					}
					else{
						sts_now[pos] = 0;
					}
					LedReq( pos, sts_now[pos] );	// LED制御
// MH810100(S) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED状態表示（反転表示）
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED状態表示（反転表示）
// MH810100(E) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
				}
				break;

			case TIMEOUT6:							// 巡回ﾃｽﾄﾀｲﾏｰﾀｲﾑｱｳﾄ

				if( sw != 0 ){						// 巡回ﾃｽﾄ中？

// MH810100(S) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 現在のｶｰｿﾙ位置：反転表示OFF
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// 現在のｶｰｿﾙ位置：反転表示OFF
// MH810100(E) S.Takahashi 2019/12/18 車番チケットレス(メンテナンス)
// MH810100(S) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
//					if( pos >= (LEDCT_MAX-1) ){		// ｶｰｿﾙ位置更新（下移動）
//						pos = 0;
//					}
//					else{
//						pos++;
//					}
#if (LEDCT_MAX > 1)
					if( pos >= (LEDCT_MAX-1) ){		// ｶｰｿﾙ位置更新（下移動）
						pos = 0;
					}
					else{
						pos++;
					}
#else
					pos = 0;
#endif
// MH810100(E) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
					if( sts_now[pos] == 0 ){		// 状態更新
						sts_now[pos] = 1;
					}
					else{
						sts_now[pos] = 0;
					}
					LedReq( pos, sts_now[pos] );	// LED制御
// MH810100(S) S.Takahashi 2019/12/18
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED状態表示（反転表示）
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED状態表示（反転表示）
// MH810100(E) S.Takahashi 2019/12/18

					Lagtim( OPETCBNO, 6, 1*50 );	// 巡回ﾃｽﾄﾀｲﾏｰ6(1s)起動
				}

				break;

			default:
				break;
		}

		if( ret != 0 ){								// LEDﾁｪｯｸ終了？

			if( sw != 0 ){							// 巡回ﾃｽﾄ中？
				Lagcan( OPETCBNO, 6 );				// 巡回ﾃｽﾄﾀｲﾏｰ解除
			}

			for( i = 0; i < LEDCT_MAX ; i++ ){		// 全LEDをﾃｽﾄ開始前の状態に戻す
				LedReq( i, sts[i] );				// LED制御
			}

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Journal & Receipt printer check                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Prt( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Prt( void )
{
	unsigned short	usFprnEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[0] );		/* // [00]	"＜プリンタチェック＞　　　　　" */

		usFprnEvent = Menu_Slt( FPRNMENU1, FPRN_CHK_TBL1, (char)FPRN_CHK_MAX1, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnEvent ){

			case FPRNJ_CHK:	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ
// MH810104 GG119201(S) 電子ジャーナル対応
			case FPRNEJ_CHK:// 電子ジャーナル
// MH810104 GG119201(E) 電子ジャーナル対応
				if( OFF == PrnJnlCheck() ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続？
					BUZPIPI();					// 接続なしの場合、ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾁｪｯｸ不可とする
					break;
				}
// MH810104 GG119201(S) 電子ジャーナル対応
				if (usFprnEvent == FPRNJ_CHK && isEJA_USE()) {
					// 電子ジャーナル接続時はジャーナルプリンタチェック不可
					BUZPIPI();
					break;
				}
				else if (usFprnEvent == FPRNEJ_CHK && !isEJA_USE()) {
					// 電子ジャーナル接続なしは電子ジャーナルチェック不可
					BUZPIPI();
					break;
				}
// MH810104 GG119201(E) 電子ジャーナル対応
				// FunChk_JRPrtで処理する（break不要）

			case FPRNR_CHK:	// ﾚｼｰﾄﾌﾟﾘﾝﾀ
				usFprnEvent = FunChk_JRPrt( usFprnEvent );
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnEvent;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				return( MOD_EXT );
				break;

			default:
				break;
		}
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス（メンテナンス)
//		if( usFprnEvent == MOD_CHG){
		if( usFprnEvent == MOD_CHG || usFprnEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス（メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

unsigned short	FunChk_JRPrt( unsigned short event ){

	unsigned short	usFprnEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		if( FPRNR_CHK == event ){
			// ﾚｼｰﾄﾌﾟﾘﾝﾀﾁｪｯｸ
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );		/* // "＜レシートプリンタチェック＞　" */
// MH810104 GG119201(S) 電子ジャーナル対応
		}else if (FPRNEJ_CHK == event) {
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );		// "＜電子ジャーナルチェック＞　　"
// MH810104 GG119201(E) 電子ジャーナル対応
		}else{
			// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾁｪｯｸ
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );		/* // "＜ジャーナルプリンタチェック＞" */
		}

		if( FPRNR_CHK == event ){
			// ﾚｼｰﾄﾌﾟﾘﾝﾀﾁｪｯｸ
			usFprnEvent = Menu_Slt( FPRNMENU2, FPRN_CHK_TBL2, (char)FPRN_CHK_MAX2, (char)1 );
// MH810104 GG119201(S) 電子ジャーナル対応
		}else if (FPRNEJ_CHK == event) {
			usFprnEvent = Menu_Slt( FPRNMENU5, FPRN_CHK_TBL5, (char)FPRN_CHK_MAX5, (char)1 );
// MH810104 GG119201(E) 電子ジャーナル対応
		}else{
			// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾁｪｯｸ
			usFprnEvent = Menu_Slt( FPRNMENU2, FPRN_CHK_TBL3, (char)(FPRN_CHK_MAX2-1), (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnEvent ){

			case FPRN7_CHK:	// ﾚｼｰﾄﾃｽﾄｲﾝｻﾂ
				usFprnEvent = FPrnChk_tst_rct( );
				break;
			case FPRN3_CHK:	// ｼﾞｬｰﾅﾙ印字ﾃｽﾄ
				usFprnEvent = FPrnChk_tst( usFprnEvent );
				break;
			case FPRN2_CHK:	// ﾚｼｰﾄ状態確認
			case FPRN4_CHK:	// ｼﾞｬｰﾅﾙ状態確認
				usFprnEvent = FPrnChk_sts( usFprnEvent );
				break;
			case FPRN5_CHK:	// ロゴ印字データ登録（ﾚｼｰﾄ）
				usFprnEvent = FPrnChk_logo( usFprnEvent );
				break;
// MH810104 GG119201(S) 電子ジャーナル対応
			case FPRN_RW_CHK:	// リード・ライトテスト
				usFprnEvent = FPrnChk_RW();
				break;
			case FPRN_SD_CHK:	// ＳＤカード情報
				usFprnEvent = FPrnChk_SD();
				break;
			case FPRN_VER_CHK:	// バージョン確認
				usFprnEvent = FPrnChk_Ver();
				break;
// MH810104 GG119201(E) 電子ジャーナル対応
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFprnEvent == MOD_CHG ){
		if( usFprnEvent == MOD_CHG || usFprnEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_tst                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌﾟﾘﾝﾀﾃｽﾄ印字処理                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN1_CHK : ﾚｼｰﾄ印字ﾃｽﾄ                               |*/
/*|                  FPRN3_CHK : ｼﾞｬｰﾅﾙ印字ﾃｽﾄ                             |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/06/30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		TSTPRI_END	(PREQ_PRINT_TEST | INNJI_ENDMASK)

unsigned short	FPrnChk_tst( unsigned short event )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	T_FrmPrintTest	FrmPrintTest;		// 印字要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	unsigned short	ret = 0;			// 戻り値
	unsigned char	priend;				// 印字終了ﾌﾗｸﾞ				※OFF:印字中、ON:印字終了
	unsigned char	dsptimer;			// 印字中画面表示ﾀｲﾏｰﾌﾗｸﾞ	※OFF:ﾀｲﾑｱｳﾄ済み	ON:起動中

	dispclr();

	if( event == FPRN1_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );			// "＜レシートテスト印刷＞　　　　" 
	}
	else{
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );			// "＜ジャーナルプリンタチェック＞" 
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[3] );				// "【テストパタン印刷】　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );				// "　印刷中です　　　　　　　　　" 
// MH810100(S) 2020/08/03 #4566【【検証課指摘事項】システムメンテナンス　ドアノブによる印字中止が行われない
// // MH810100(S) Y.Yamauchi 2020/03/19 車番チケットレス(#3963 テスト印刷開始し印刷が途中で停止するとテスト印刷画面に留まってしまう。)
// 	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );				// "強制停止したい場合は　　　　 "
// 	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[19] );				// "メンテナンス終了してください"
// // MH810100(E) Y.Yamauchi 2020/03/19 車番チケットレス(#3963 テスト印刷開始し印刷が途中で停止するとテスト印刷画面に留まってしまう。)
// MH810100(E) 2020/08/03 #4566【【検証課指摘事項】システムメンテナンス　ドアノブによる印字中止が行われない
	Fun_Dsp(FUNMSG[0]);															/* [0]	"　　　　　　　　　　　　　　　" */
	
	if( event == FPRN1_CHK ){
		FrmPrintTest.prn_kind = R_PRI;				// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	}
	else{
		FrmPrintTest.prn_kind = J_PRI;				// ﾌﾟﾘﾝﾀ種別：ｼﾞｬｰﾅﾙ
	}

	queset( PRNTCBNO, PREQ_PRINT_TEST, sizeof( T_FrmPrintTest ), &FrmPrintTest ); 			// テスト印字要求
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 3*50 );					// 印字中画面表示ﾀｲﾏｰ6(3s)起動（ﾃｽﾄ印字中の画面表示を３秒間保持する）
	dsptimer	= ON;								// 印字中画面表示ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
	priend		= OFF;								// 印字終了ﾌﾗｸﾞﾘｾｯﾄ

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg ){			// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	// 操作ﾓｰﾄﾞ切替
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				// ドアノブの状態にかかわらずトグル動作してしまうので、
				// ドアノブ閉かどうかのチェックを実施
				if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				BUZPI();
				ret = MOD_CHG;
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				break;
			case TSTPRI_END:	// テスト印字終了
				if( dsptimer == OFF ){					// 印字中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}
				else{
					priend = ON;						// NO　→印字終了ﾌﾗｸﾞｾｯﾄ
				}
				break;
			case TIMEOUT6:		// 印字中画面終了ﾀｲﾏｰﾀｲﾑｱｳﾄ
				dsptimer = OFF;							// 印字中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ
				if( priend == ON ){						// 印字終了済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}
				break;
			default:			// その他
				break;
		}

		if( ret != 0 ){									// 印字中画面終了？
			if( dsptimer == ON ){						// 印字中画面表示ﾀｲﾏｰ起動中？
				Lagcan( OPETCBNO, 6 );					// 印字中画面表示ﾀｲﾏｰ解除
			}
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_sts                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌﾟﾘﾝﾀ状態確認処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN2_CHK : ﾚｼｰﾄ状態確認                              |*/
/*|                  FPRN4_CHK : ｼﾞｬｰﾅﾙ状態確認                            |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/06/30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned char	PriErrSts[4] =	// ﾌﾟﾘﾝﾀ状態ﾁｪｯｸﾌﾗｸﾞ
	{
		0x01,	// b0:ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ
		0x02,	// b1:ﾌﾟﾗﾃﾝｵｰﾌﾟﾝ
		0x04,	// b2:ﾍﾟｰﾊﾟｰｴﾝﾄﾞ
		0x08	// b3:ﾍｯﾄﾞ温度異常
	};

unsigned short	FPrnChk_sts( unsigned short event )	// 状態確認
{
	short			msg = -1;
	unsigned short	ret = 0;
	unsigned char	sts;
	unsigned char	com = OFF;
	unsigned short	line;
	unsigned char	i;

	dispclr();

	if( event == FPRN2_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );				// "＜レシートプリンタチェック＞　"
	}
	else{
// MH810104 GG119201(S) 電子ジャーナル対応
		if (isEJA_USE()) {
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );			// "＜電子ジャーナルチェック＞　　"
		}
		else {
// MH810104 GG119201(E) 電子ジャーナル対応
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );				// "＜ジャーナルプリンタチェック＞"
// MH810104 GG119201(S) 電子ジャーナル対応
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[6] );					// "【状態確認】　　　　　　　　　"
	Fun_Dsp( FUNMSG[8] );																// "　　　　　　　　　　　　 終了 "

	//状態確認
	if( event == FPRN2_CHK ){
		//	レシートプリンタ
		if(ERR_CHK[ERRMDL_PRINTER][01] != 0){
			com = ON;									// 通信不良
		}
		sts = (uchar)(rct_prn_buff.PrnStateMnt & 0x0f);	// 状態ﾘｰﾄﾞ
	}
	else{
		//	ジャーナルプリンタ
		if(ERR_CHK[ERRMDL_PRINTER][20] != 0){
			com = ON;									// 通信不良
		}
// MH810104 GG119201(S) 電子ジャーナル対応
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
			com = ON;									// 通信不良
		}
// MH810104 GG119201(E) 電子ジャーナル対応
		sts = (uchar)(jnl_prn_buff.PrnStateMnt & 0x0f);	// 状態ﾘｰﾄﾞ
// MH810104 GG119201(S) 電子ジャーナル対応（SDカード使用期限超過ステータスが表示されない）
		if (isEJA_USE()) {
			sts = (uchar)(jnl_prn_buff.PrnStateMnt & 0x1f);	// 状態ﾘｰﾄﾞ
		}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカード使用期限超過ステータスが表示されない）
	}
	if( com == OFF ){

		// 通信状態＝正常

		if( sts == 0 ){
			// 状態＝異常なし
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[7] );						// "　正常です　　　　　　　　　　"
		}
		else{
			// 状態＝異常あり
			line	= 2;
			for( i = 0 ; i < 4 ; i++ ){						// 状態をﾁｪｯｸし発生しているｴﾗｰ情報を全て表示する
				if( sts & PriErrSts[i] ){
// MH810104 GG119201(S) 電子ジャーナル対応
// MH810104 GG119201(S) 電子ジャーナル対応（レシートのニアエンドでSDニアエンドが表示される）
//					if (isEJA_USE()) {
					if (isEJA_USE() && event == FPRN4_CHK) {
// MH810104 GG119201(E) 電子ジャーナル対応（レシートのニアエンドでSDニアエンドが表示される）
						grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[i+19] );	// ｴﾗｰ情報表示
					}
					else {
// MH810104 GG119201(E) 電子ジャーナル対応
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[i+8] );	// ｴﾗｰ情報表示
// MH810104 GG119201(S) 電子ジャーナル対応
					}
// MH810104 GG119201(E) 電子ジャーナル対応
					line++;
				}
			}
// MH810104 GG119201(S) 電子ジャーナル対応（SDカード使用期限超過ステータスが表示されない）
			if (isEJA_USE()) {
				if (sts & 0x10) {
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[31] );	// ｴﾗｰ情報表示
					line++;
				}
			}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカード使用期限超過ステータスが表示されない）
		}
	}
	else{
		// 通信状態＝異常
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[12] );					// "　通信不良　　　　　　　　　　"
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );					// ﾒｯｾｰｼﾞ受信

		switch( msg ){									// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:							// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			default:
				break;
		}
		if( ret != 0 ){
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_logo                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ロゴ印字データ登録処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN5_CHK : ロゴ印字データ登録（ﾚｼｰﾄ）                |*/
/*|                  FPRN6_CHK : ロゴ印字データ登録（ｼﾞｬｰﾅﾙ）              |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/08/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		LOGOREG_END	(PREQ_LOGO_REGIST | INNJI_ENDMASK)

unsigned short	FPrnChk_logo( unsigned short event )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	T_FrmLogoRegist	FrmLogoRegist;		// ロゴ印字データ登録要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	unsigned short	ret = 0;			// 戻り値
	unsigned char	regend;				// 登録終了ﾌﾗｸﾞ				※OFF:登録中、ON:登録終了
	unsigned char	dsptimer;			// 登録中画面表示ﾀｲﾏｰﾌﾗｸﾞ	※OFF:ﾀｲﾑｱｳﾄ済み	ON:起動中

	dispclr();

	if( event == FPRN5_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );			// "＜レシートプリンタチェック＞　" 
	}
	else{
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );			// "＜ジャーナルプリンタチェック＞" 
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[13] );				// "【ロゴ印字データ登録】　　　　"
	grachr( 3,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, PRTSTR[14] );		// "　登録中(しばらくお待ち下さい)" 
	Fun_Dsp( FUNMSG[0] );														// "　　　　　　　　　　　　　　　" 

	if( event == FPRN5_CHK ){
		FrmLogoRegist.prn_kind = R_PRI;				// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	}
	else{
		FrmLogoRegist.prn_kind = J_PRI;				// ﾌﾟﾘﾝﾀ種別：ｼﾞｬｰﾅﾙ
	}

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ロゴ印字データ登録要求
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 5*50 );					// 登録中画面表示ﾀｲﾏｰ6(5s)起動（登録中の画面表示を５秒間保持する）
	dsptimer	= ON;								// 登録中画面表示ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
	regend		= OFF;								// 登録終了ﾌﾗｸﾞﾘｾｯﾄ

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg){			// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case LOGOREG_END:	// 登録終了

				if( dsptimer == OFF ){					// 登録中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}
				else{
					regend = ON;						// NO　→　登録終了ﾌﾗｸﾞｾｯﾄ
				}

				break;

			case TIMEOUT6:		// 登録中画面終了ﾀｲﾏｰﾀｲﾑｱｳﾄ

				dsptimer = OFF;							// 登録中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ

				if( regend == ON ){						// 登録終了済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}

				break;

			default:			// その他
				break;
		}

		if( ret != 0 ){									// 登録中画面終了？

			if( dsptimer == ON ){						// 登録中画面表示ﾀｲﾏｰ起動中？
				Lagcan( OPETCBNO, 6 );					// 登録中画面表示ﾀｲﾏｰ解除
			}

			break;
		}
	}
	return( ret );
}

// MH810104 GG119201(S) 電子ジャーナル対応
//[]----------------------------------------------------------------------[]
///	@brief			リード・ライトテスト（電子ジャーナル）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=モード切替, MOD_EXT=終了
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_RW(void)
{
	ushort	ret = 0;	// 戻り値
	short	msg = -1;	// 受信ﾒｯｾｰｼﾞ
	ulong	time;

	if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
		IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||
		ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "＜電子ジャーナルチェック＞　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[1]);			// "【リード・ライトテスト】　　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[24]);		// "　　しばらくお待ちください　　"

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_TEST_RW);				// SDカードテスト（リードライト）

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
		case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// Ｆ５（終了）
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SD_TEST_RW:					// SDカードテスト（リードライト）結果通知
// MH810104 GG119201(S) 電子ジャーナル対応（テスト成功メッセージ表示）
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[27]);// "　リード時間：　　　　　ｍｓ　"
//			memcpy(&time, &eja_work_buff[0], 4);
//			opedpl3(3, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// リード時間
//
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[28]);// "　ライト時間：　　　　　ｍｓ　"
//			memcpy(&time, &eja_work_buff[4], 4);
//			opedpl3(4, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ライト時間
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[32]);// "テストに成功しました　　　　　"
			displclr(3);
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[27]);// "　リード時間：　　　　　ｍｓ　"
			memcpy(&time, &eja_work_buff[0], 4);
			opedpl3(4, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// リード時間

			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[28]);// "　ライト時間：　　　　　ｍｓ　"
			memcpy(&time, &eja_work_buff[4], 4);
			opedpl3(5, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ライト時間
// MH810104 GG119201(E) 電子ジャーナル対応（テスト成功メッセージ表示）

			Fun_Dsp( FUNMSG[8] );										// "　　　　　　　　　　　　 終了 "
			break;
		case PREQ_ERR_END:
			displclr(3);
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[29]);// "　　 テストに失敗しました 　　"
			Fun_Dsp( FUNMSG[8] );										// "　　　　　　　　　　　　 終了 "
			break;
		default:
			break;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			SDカード情報（電子ジャーナル）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=モード切替, MOD_EXT=終了
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_SD(void)
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

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "＜電子ジャーナルチェック＞　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[23]);		// "【ＳＤカード情報】　　　　　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[24]);		// "　全体サイズ：　　　　　ＭＢ　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[25]);		// "　空きサイズ：　　　　　ＭＢ　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[26]);		// "　使用時間　：　　　　　時間　"

	Fun_Dsp( FUNMSG[8] );												// "　　　　　　　　　　　　 終了 "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_INFO);					// SDカード情報要求

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
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
			opedpl3(3, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 全体サイズ

			memcpy(&upper, &eja_work_buff[8], 4);
			memcpy(&lower, &eja_work_buff[12], 4);
			work = upper * (1 << 12);			// 上位桁をMB単位に変換
			work += (lower / (1024*1024));		// 下位桁をMB単位に変換
			opedpl3(4, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 空きサイズ

			memcpy(&work, &eja_work_buff[16], 4);
			opedpl3(5, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// 使用時間
			break;
		default:
			break;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			バージョン確認（電子ジャーナル）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=モード切替, MOD_EXT=終了
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_Ver(void)
{
	ushort	ret = 0;	// 戻り値
	short	msg = -1;	// 受信ﾒｯｾｰｼﾞ
	uchar	version[8];
	uchar	sd_version[8];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "＜電子ジャーナルチェック＞　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[4]);			// "【バージョン確認】　　　　　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[6]);			// "　バージョン → 　　　　　　　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[30]);		// "　SDｶｰﾄﾞﾃﾞｰﾀ → 　　　　　　　"

	Fun_Dsp( FUNMSG[8] );												// "　　　　　　　　　　　　 終了 "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_VER_INFO);					// ソフトバージョン要求

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
		case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// Ｆ５（終了）
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SW_VER_INFO:					// ソフトバージョン通知
			memcpy(version, eja_work_buff, 8);
			grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, version);	// バージョン表示
			// ソフトバージョン通知の受信後にSDカードバージョン要求を送信する
			MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_VER_INFO);				// SDカードバージョン要求
			break;
		case PREQ_SD_VER_INFO:					// SDカードバージョン通知
			memcpy(sd_version, eja_work_buff, 8);
			grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, sd_version);	// SDカードバージョン表示
			break;
		default:
			break;
		}
	}
	return ret;
}
// MH810104 GG119201(E) 電子ジャーナル対応

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Rdr                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾁｪｯｸ処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Rdr( void )
{
	unsigned short	usFrdrEvent;
	char			wk[2];
	char			org[2];
	short			msg;
	int				mode = -1;
	unsigned char set;

	org[0]		= DP_CP[0];
	org[1]		= DP_CP[1];
	DP_CP[0]	= DP_CP[1] = 0;

	if( opr_snd( 95 ) == 0 ){ 		// 状態要求
		Lagtim( OPETCBNO, 6, 2*50 );// 状態要求ﾀｲﾏｰ(2s)起動
	}
	while (mode < 0) {
		msg = StoF(GetMessage(), 1);		// ﾒｯｾｰｼﾞ受信
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替
			BUZPI();
			return(MOD_CHG);
			break;
		case ARC_CR_E_EVT:					// 状態受信
			mode = (RED_REC.ercd == 0) ? 1: 0;
			break;
		case TIMEOUT6:						// 状態要求ﾀｲﾏｰﾀｲﾑｱｳﾄ
			mode = 0;						// ディップスイッチチェックに遷移させない
			break;
		default :
			break;
		}
	}

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "＜磁気リーダーチェック＞　　　"
		Fun_Dsp( FUNMSG[25] );						// "　▲　　▼　　　　 読出  終了 "

		usFrdrEvent = Menu_Slt( FRDMENU, FRDR_CHK_TBL, (char)FRDR_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFrdrEvent ){

			case FRD1_CHK:							// リード／ライトテスト
				usFrdrEvent = FRdrChk_RW();
				break;

			case FRD2_CHK:							// 印字テスト
				set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
				if(set == 1 || set == 2) {
					usFrdrEvent = FRdrChk_Pr();
				}
				else {
					BUZPIPI();
				}
				break;

			case FRD3_CHK:							// 状態確認
				usFrdrEvent = FRdrChk_Sts();
				break;

			case FRD4_CHK:							// バージョン確認
				usFrdrEvent = FRdrChk_Ver();
				break;
			case FRD5_CHK:							// ディップスイッチ確認
				if (mode > 0) {
					usFrdrEvent = FRdrChk_DipSwitch();
				}
				else {
					BUZPIPI();
				}
				break;

			case MOD_EXT:							// 終了
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFrdrEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFrdrEvent == MOD_CHG ){
		if( usFrdrEvent == MOD_CHG || usFrdrEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFrdrEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_RW                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾘｰﾄﾞ・ﾗｲﾄﾃｽﾄ処理                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		RDRSTSCHK1	0xffff0083l		// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀ（一方向用）
#define		RDRSTSCHK2	0xffff0383l		// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀ（四方向用）

unsigned short	FRdrChk_RW( void )
{
	ushort		ret = 0;		// 戻り値
	short		msg = -1;		// 受信ﾒｯｾｰｼﾞ
	uchar		req = 0;		// ﾃｽﾄ要求状態
	uchar		prm;			// 磁気ﾘｰﾀﾞｰ種別
	ulong		m;				// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀ
	ulong		l;				// ﾘｰﾀﾞｰｽﾃｰﾀｽ


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );				// "＜磁気リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[1] );				// "【リード・ライトテスト】　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );				// "　磁気カードを入れてください　"

	Fun_Dsp( FUNMSG[8] );							// "　　　　　　　　　　　　 終了 "


	prm = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// 磁気ﾘｰﾀﾞｰ種別取得

	if( prm == 1 || prm == 3 ){						// 磁気ﾘｰﾀﾞｰ種別？
		// 一方向
		m = RDRSTSCHK1;								// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀｾｯﾄ（一方向用）
	}
	else{
		// 四方向
		m = RDRSTSCHK2;								// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀｾｯﾄ（四方向用）
	}

	for( ; ret == 0 ; ){

		if( req == 0 ){
			if( opr_snd( 92 ) == 0 ){				// ﾘｰﾄﾞﾗｲﾄﾃｽﾄ要求
				// ｺﾏﾝﾄﾞ送信ＯＫ
				read_sht_opn();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「開」
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );	// "　磁気カードを入れてください　"
				req = 1;
			}else{
				// ｺﾏﾝﾄﾞ送信ＮＧ
				return( MOD_EXT );
			}
		}

		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg){								// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:						// 終了ｺﾏﾝﾄﾞ受信

				if( req != 2 ){

					memcpy( &l, RED_stat, 4 );							// ﾘｰﾀﾞｰｽﾃｰﾀｽ取得

					if(( RED_REC.ercd == 0 ) && (( l & m ) == 0l )){	// ﾃｽﾄ結果？
						// 正常終了
						grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[10] );				// "　正常です　　　　　　　　　　"
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_WR, 1 );
//						inc_dct( READ_YO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
					}
					else{
						// 異常終了
					ret = RdrChk_ErrDsp( RED_REC.ercd, ( l & m ));		// ｴﾗｰ表示画面処理
					if(RED_REC.ercd == E_VERIFY || RED_REC.ercd == E_VERIFY_30){
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_VN, 1 );
//						inc_dct( READ_WR, 1 );
//						inc_dct( READ_YO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
					}else{
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

					}
					}
					req = 2;
				}
				else
				{
					req = 0;
				}

				break;

			default:
				break;
		}
	}
	read_sht_cls();														// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
	opr_snd( 90 );														// 状態要求
	FRdrChk_Wait();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Pr                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰ印字ﾃｽﾄ処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Pr( void )
{
	ushort		ret = 0;		// 戻り値
	short		msg = -1;		// 受信ﾒｯｾｰｼﾞ
	uchar		req = 0;		// ﾃｽﾄ要求状態
	uchar		prm;			// 磁気ﾘｰﾀﾞｰ種別
	ulong		m;				// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀ
	ulong		l;				// ﾘｰﾀﾞｰｽﾃｰﾀｽ


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );				// "＜磁気リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[2] );				// "【印字テスト】　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );				// "　磁気カードを入れてください　"

	Fun_Dsp( FUNMSG[8] );							// "　　　　　　　　　　　　 終了 "


	prm = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// 磁気ﾘｰﾀﾞｰ種別取得

	if( prm == 1 || prm == 3 ){						// 磁気ﾘｰﾀﾞｰ種別？
		// 一方向
		m = RDRSTSCHK1;								// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀｾｯﾄ（一方向用）
	}
	else{
		// 四方向
		m = RDRSTSCHK2;								// ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸﾃﾞｰﾀｾｯﾄ（四方向用）
	}

	for( ; ret == 0 ; ){

		if( req == 0 ){
			if( opr_snd( 93 ) == 0 ){				// 印字ﾃｽﾄ要求
				// ｺﾏﾝﾄﾞ送信ＯＫ
				read_sht_opn();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「開」
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );	// "　磁気カードを入れてください　"
				req = 1;
			}else{
				// ｺﾏﾝﾄﾞ送信ＮＧ
				return( MOD_EXT );
			}
		}

		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg){								// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:						// 終了ｺﾏﾝﾄﾞ受信

				if( req != 2 ){

					memcpy( &l, RED_stat, 4 );							// ﾘｰﾀﾞｰｽﾃｰﾀｽ取得

					if(( RED_REC.ercd == 0 ) && (( l & m ) == 0l )){	// ﾃｽﾄ結果？
						// 正常終了
						grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[10] );				// "　正常です　　　　　　　　　　"
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_YO, 1 );
//						inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)			
					}
					else{
						// 異常終了
					ret = RdrChk_ErrDsp( RED_REC.ercd, ( l & m ));		// ｴﾗｰ表示画面処理
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//					inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
					}
					req = 2;
				}

				break;

			default:
				break;
		}
	}
	read_sht_cls();														// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
	opr_snd( 90 );														// 状態要求
	FRdrChk_Wait();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RdrChk_ErrDsp                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾃｽﾄｴﾗｰ画面処理                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char ercd                                      |*/
/*|                unsigned long errbit                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	RDR_ERR_MAX		10
// 磁気ﾘｰﾀﾞｰｽﾃｰﾀｽﾁｪｯｸ用ﾃﾞｰﾀ（ﾁｪｯｸbit＆ｴﾗｰﾒｯｾｰｼﾞNO.）
const	unsigned long	rdr_sts_chk[32][2] =
	{
// ｽﾃｰﾀｽ情報１
		0x01000000l,	1,	//	Bit24:S1ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ1）
		0x02000000l,	2,	//	Bit25:S2ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ2）
		0x04000000l,	3,	//	Bit26:S3ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ3）
		0x08000000l,	4,	//	Bit27:S4ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ4）
		0x10000000l,	5,	//	Bit28:S5ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ5）
		0x20000000l,	6,	//	Bit29:S6ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ6）
		0x40000000l,	7,	//	Bit30:S7ｱﾗｰﾑ（ﾘｰﾀﾞｰ位置ｾﾝｻｰ7）
		0x80000000l,	8,	//	Bit31:SPｱﾗｰﾑ（ﾌﾟﾘﾝﾀ位置ｾﾝｻｰ）

// ｽﾃｰﾀｽ情報２
		0x00010000l,	9,	//	Bit16:S1不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ1）
		0x00020000l,	10,	//	Bit17:S2不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ2）
		0x00040000l,	11,	//	Bit18:S3不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ3）
		0x00080000l,	12,	//	Bit19:S4不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ4）
		0x00100000l,	13,	//	Bit20:S5不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ5）
		0x00200000l,	14,	//	Bit21:S6不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ6）
		0x00400000l,	15,	//	Bit22:S7不良（ﾘｰﾀﾞｰ位置ｾﾝｻｰ7）
		0x00800000l,	16,	//	Bit23:SP不良（ﾌﾟﾘﾝﾀ位置ｾﾝｻｰ）

// ｽﾃｰﾀｽ情報３
		0x00000100l,	17,	//	Bit08:磁気ﾍｯﾄﾞHPｾﾝｻｰ1不良
		0x00000200l,	18,	//	Bit09:磁気ﾍｯﾄﾞHPｾﾝｻｰ2不良
		0,				0,	//	Bit10:予備					※ｴﾗｰ表示なし
		0,				0,	//	Bit11:予備					※ｴﾗｰ表示なし
		0,				0,	//	Bit12:挿入口券				※ｴﾗｰ表示なし
		0,				0,	//	Bit13:予備					※ｴﾗｰ表示なし
		0,				0,	//	Bit14:予備					※ｴﾗｰ表示なし
		0,				0,	//	Bit15:外字登録				※ｴﾗｰ表示なし

// ｽﾃｰﾀｽ情報４
		0x00000001l,	19,	//	Bit00:ｶｰﾄﾞ詰まり（ﾘｰﾀﾞｰ）
		0x00000002l,	20,	//	Bit01:ｶｰﾄﾞ詰まり（ﾌﾟﾘﾝﾀ）
		0x00000004l,	21,	//	Bit02:ﾓｰﾀｰ故障（ﾘｰﾀﾞｰ）		※未使用（ｽﾃｰﾀｽ情報に反映されない）
		0x00000008l,	22,	//	Bit03:ﾓｰﾀｰ故障（ﾌﾟﾘﾝﾀ）		※未使用（ｽﾃｰﾀｽ情報に反映されない）
		0,				0,	//	Bit04:予備					※ｴﾗｰ表示なし
		0,				0,	//	Bit05:ｶｰﾄﾞ挿入待ち			※ｴﾗｰ表示なし
		0,				0,	//	Bit06:ｺﾏﾝﾄﾞ受信待ち			※ｴﾗｰ表示なし
		0x00000080l,	23	//	Bit07:RAM不良

	};

// 磁気ﾘｰﾀﾞｰｴﾗｰｺｰﾄﾞﾁｪｯｸ用ﾃﾞｰﾀ（ｴﾗｰｺｰﾄﾞ＆ｴﾗｰﾒｯｾｰｼﾞNO.）
const	unsigned char	rdr_erc_chk[RDR_ERR_MAX][2] =
	{
		0x11,	24,			// 短券ｴﾗｰ
		0x12,	25,			// 長券ｴﾗｰ
		0x21,	26,			// ｽﾀｰﾄ符号ｴﾗｰ
		0x22,	27,			// ﾃﾞｰﾀｴﾗｰ
		0x23,	28,			// ﾊﾟﾘﾃｨｰｴﾗｰ
		0x26,	29,			// ﾘﾄﾗｲﾘｰﾄﾞｴﾗｰ
		0x27,	30,			// ﾍﾞﾘﾌｧｲｴﾗｰ
		0x31,	31,			// 不条理ｺﾏﾝﾄﾞ受信
		0x24,	32,			// 書込みＣＲＣエラー
		0x25,	33			// 読取りＣＲＣエラー
	};

unsigned short	RdrChk_ErrDsp( uchar ercd, ulong errbit )
{
	unsigned short	ret			= 0;	// 戻り値
	unsigned char	e_page_cnt	= 1;	// 異常ｽﾃｰﾀｽ表示ﾍﾟｰｼﾞ数
	unsigned char	e_page_pos	= 1;	// 異常ｽﾃｰﾀｽ表示ﾍﾟｰｼﾞ位置
	unsigned char	e_cnt		= 0;	// 異常ｽﾃｰﾀｽ数
	unsigned char	i;					// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned char	err_no[33];			// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱ
	unsigned char	set_pos		= 0;	// 異常ｽﾃｰﾀｽNO.格納ﾎﾟｲﾝﾀ
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ


	memset( err_no, 0, 33 );							// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱｸﾘｱ

	for( i = 0 ; i < 32 ; i++ ){						// 異常ｽﾃｰﾀｽﾃﾞｰﾀ検索
		if( errbit & rdr_sts_chk[i][0] ){
			// 異常あり
			e_cnt++;									// 異常ｽﾃｰﾀｽ数更新（+1）
			err_no[set_pos] = (uchar)rdr_sts_chk[i][1];	// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱへ異常ｽﾃｰﾀｽNO.を格納
			set_pos++;									// 格納ﾎﾟｲﾝﾀ更新（+1）
		}
	}

	if( ercd != 0 ){									// ｴﾗｰ発生？

		for( i = 0 ; i < RDR_ERR_MAX ; i++ ){						// ｴﾗｰｺｰﾄﾞ検索
			if( ercd == rdr_erc_chk[i][0] ){
				// ｴﾗｰｺｰﾄﾞ一致
				e_cnt++;								// 異常ｽﾃｰﾀｽ数更新（+1）
				err_no[set_pos] = rdr_erc_chk[i][1];	// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱへ異常ｽﾃｰﾀｽNO.を格納
				set_pos++;								// 格納ﾎﾟｲﾝﾀ更新（+1）
				break;
			}
		}
	}

	if( e_cnt ){										// 異常あり？

		// 異常がある場合

		e_page_cnt = (uchar)(e_cnt / 5);				// 異常ｽﾃｰﾀｽ数から表示ﾍﾟｰｼﾞ数をｾｯﾄ(１ﾍﾟｰｼﾞに最大５ｽﾃｰﾀｽまで表示)
		if( e_cnt % 5 ){
			e_page_cnt++;
		}
	}
	Rdr_err_dsp( e_page_pos, &err_no[0] );				// 異常ｽﾃｰﾀｽ表示

	if( e_page_cnt == 1 ){								// ﾌｧﾝｸｼｮﾝｷｰ表示
		// 他に表示ページがない場合
		Fun_Dsp( FUNMSG[8] );							// "　　　　　　　　　　　　 終了 "
	}
	else{
		// 他に表示ページがある場合
		Fun_Dsp( FUNMSG[6] );							// "　▼　　▲　　　　　　　 終了 "
	}

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );					// ﾒｯｾｰｼﾞ受信

		switch( msg){									// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:							// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:							// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:							// Ｆ１（▲：前ページ切替）

				if( e_page_cnt != 1 ){

					BUZPI();

					if( e_page_pos == 1 ){
						e_page_pos = e_page_cnt;
					}
					else{
						e_page_pos--;
					}
					Rdr_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			case KEY_TEN_F2:							// Ｆ２（▼：次ページ切替）

				if( e_page_cnt != 1 ){

					BUZPI();

					e_page_pos++;
					if( e_page_pos > e_page_cnt ){
						e_page_pos = 1;
					}
					Rdr_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			default:									// その他
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Rdr_err_dsp                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾁｪｯｸ異常ｽﾃｰﾀｽ表示処理                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char pos（表示ﾍﾟｰｼﾞ位置）                      |*/
/*|              : unsigned char *data（異常ｽﾃｰﾀﾃﾞｰﾀｽﾎﾟｲﾝﾀ）               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Rdr_err_dsp( unsigned char pos, unsigned char *data )
{
	unsigned char	max;
	unsigned char	ofs;
	unsigned char	no;
	unsigned char	i;

	switch( pos ){		// ﾍﾟｰｼﾞ位置？

		case	1:		// １ページ目
			max = 5;
			ofs = 0;
			break;

		case	2:		// ２ページ目
			max = 5;
			ofs = 5;
			break;

		case	3:		// ３ページ目
			max = 5;
			ofs = 10;
			break;

		case	4:		// ４ページ目
			max = 5;
			ofs = 15;
			break;

		case	5:		// ５ページ目
			max = 5;
			ofs = 20;
			break;

		case	6:		// ６ページ目
			max = 5;
			ofs = 25;
			break;

		case	7:		// ７ページ目
			max = 3;
			ofs = 30;
			break;

		default:		// その他（ページエラー）
			return;
	}
	for( i = 2 ; i <= 6 ; i++ ){	// ﾗｲﾝ２～６ｸﾘｱ
		displclr( (unsigned short)i );
	}

	for( i = 0 ; i < max ; i++ ){	// 異常ｽﾃｰﾀｽ表示（指定ページの最大行数、または表示ﾃﾞｰﾀ終了まで）

		no = data[ofs+i];											// 表示する異常ｽﾃｰﾀｽNO.取得
		if( no == 0 ){
			break;													// 表示ﾃﾞｰﾀ終了
		}
		grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDESTR[no] );	// 異常ｽﾃｰﾀｽ表示
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Sts                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰ状態確認処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Sts( void )
{
	ushort		ret = 0;		// 戻り値
	ushort		req = OFF;		// 状態要求状態
	ushort		reqtimer = OFF;	// 状態要求ﾀｲﾏｰ状態
	short		msg = -1;		// 受信ﾒｯｾｰｼﾞ


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "＜磁気リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[3] );			// "【状態確認】　　　　　　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[7] );			// "記号　S1S2S3S4S5S6S7　SP　　HP"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[8] );			// "良否　　　　　　　　　　　　　"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[9] );			// "検知　　　　　　　　　　　　　"

	Fun_Dsp( FUNMSG[8] );						// "　　　　　　　　　　　　 終了 "

	for( ; ; ){

		if( req == OFF ) {
			if( opr_snd( 90 ) == 0 ){			// 状態要求
				req = ON;
			}
		} else {
			if( RD_mod < 6 ) {
				req = OFF;
			}
		}

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信

		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:					// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:					// 状態受信

				Rdr_sts_dsply( RED_REC.posi[0], RED_REC.posi[1], RED_stat );	// 状態表示
				Lagtim( OPETCBNO, 6, 2*50 );									// 状態要求ﾀｲﾏｰ(2s)起動（状態確認画面中、２秒毎に状態要求を行う）
				reqtimer = ON;

				break;

			case TIMEOUT6:						// 状態要求ﾀｲﾏｰﾀｲﾑｱｳﾄ

				req = OFF;
				reqtimer = OFF;
				break;

			default:
				break;
		}
		if( ret != 0 ){		// 状態確認終了？

			if( reqtimer == ON ){				// 状態要求ﾀｲﾏｰ起動中？
				Lagcan( OPETCBNO, 6 );			// 状態要求ﾀｲﾏｰ解除
			}

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Rdr_sts_dsply                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰ状態確認処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : uchar pos1:ｾﾝｻｰ状態                                     |*/
/*|              : uchar pos2:磁気ﾍｯﾄﾞ位置                                 |*/
/*|              : uchar *sta:ﾘｰﾀﾞｰ状態                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Rdr_sts_dsply( uchar pos1, uchar pos2, uchar *sta )
{
	uchar	i,d;
	uchar	printerUse ;

	printerUse = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
	for( i = 0; i < 8; i++ ){
		d = 0;
		if((( sta[1] >> i ) & 0x01 ) == 1 ) {
			d = 1;
		} else {
			if((( sta[0] >> i ) & 0x01 ) == 1 ) {
				d = 2;
			}
		}
		if(i < 7) {
		grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[d] );						// 良否（S1～SP）

		grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[(( pos1 >> i ) & 0x01 )] );	// 検知（S1～SP）
		}
		else {
			// プリンタ未使用の設定ではSPの良否、検知は'－'を表示する
			if(printerUse == 1 || printerUse == 2) {	// プリンタあり
				grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[d] );						// 良否（SP）
				grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[(( pos1 >> i ) & 0x01 )] );	// 検知（SP）
			}
			else {										// プリンタなし
				grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3] );						// 良否（SP）
				grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3] );						// 検知（SP）
			}
		}
	}
	grachr( 4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_6[ (pos2 & 0x03) ] );													// HP（ﾍｯﾄﾞ位置）
}

//[]----------------------------------------------------------------------[]
///	@brief			ディップスイッチ確認処理 
//[]----------------------------------------------------------------------[]
///	@return			ret		: MOD_CHG or MOD_EXT
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/02/29<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
#define		DIPSWITCH_MAX		4
static unsigned short	FRdrChk_DipSwitch( void )
{
	char		i;					// ループ
	int			pos;				// SW1～4
	uchar		display;			// 表示の有無
	uchar		sts;				// ディップスイッチの状態(0:OFF, 1:ON)
	uchar		sts_bit;			// ディップスイッチチェックビット
	int			req;				// 状態要求状態
	ushort		ret;				// 戻り値
	short		msg;				// 受信ﾒｯｾｰｼﾞ

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0]);				// "＜磁気リーダーチェック＞　　　"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[11]);			// "【ディップスイッチ確認】　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[12]);			// "　　　　　１２３４　　　　　　"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[13]);			// "　　　ON　　　　　　　　　　　"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[14]);			// "　　　OFF 　　　　　　　　　　"
	Fun_Dsp(FUNMSG2[42]);						// "ＳＷ１ ＳＷ２　　 ＳＷ４ 終了 "

	display = OFF;
	pos = 0;
	req = 0;
	msg = -1;
	ret = 0;
	do {
		switch ( msg ) {						// 受信ﾒｯｾｰｼﾞ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:					// Ｆ５（終了）
			BUZPI();
			ret = MOD_EXT;
			break;
		case KEY_TEN_F1:					// Ｆ１
		case KEY_TEN_F2:					// Ｆ２
		case KEY_TEN_F4:					// Ｆ４
			BUZPI();
			pos = (int)(msg - KEY_TEN_F1);
		case -1:
			grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[(pos == 3)? 17 : 15+pos] );
							// "ＳＷ１：単体テスト・調整　　　"
							// "ＳＷ２：プログラム機能種別　　"
							// "ＳＷ４：通信仕様・通信アドレス"
			req = 0;
			break;
		case ARC_CR_E_EVT:					// 状態受信
			if (req == 1) {
				if (RED_REC.ercd == 0) {
				// no error
					display  = ON;
					req = 2;
				}
			}
			break;
		case TIMEOUT6:						// 状態要求ﾀｲﾏｰﾀｲﾑｱｳﾄ
			if (req == 1) {
				req = 0;
				BUZPIPI();					// 情報が取得出来ないときは終了
				ret = MOD_EXT;
			}
			break;
		default:							// その他
			break;
		}

		if( req == 0 ) {
			if( opr_snd( 95 ) == 0 ){ 		// 状態要求
				Lagtim( OPETCBNO, 6, 2*50 );// 状態要求ﾀｲﾏｰ(2s)起動
				req = 1;
			}
		}
		if (display == ON) {
			displclr(3);
			displclr(4);
			sts = RDS_REC.state[pos];			// 下位4bit取得
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[13]);			// "　　　ON　　　　　　　　　　　"
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[14]);			// "　　　OFF 　　　　　　　　　　"
			sts_bit = (0x01 << (DIPSWITCH_MAX-1));
			for (i = 0; i < DIPSWITCH_MAX; i++, sts_bit >>= 1) {
				if (sts & sts_bit) {
					grachr(3, (ushort)(10+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[9]);	// ONに"■"
				} else {
					grachr(4, (ushort)(10+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[9]);	// OFFに"■"
				}
			}
			display = OFF;
		}
		msg = StoF(GetMessage(), 1);		// ﾒｯｾｰｼﾞ受信
	} while(ret == 0);
	Lagcan( OPETCBNO, 6 );		// 状態要求ﾀｲﾏｰ解除
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Ver                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾊﾞｰｼﾞｮﾝ確認処理                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Ver( void )
{
	ushort		ret = 0;	// 戻り値
	short		msg = -1;	// 受信ﾒｯｾｰｼﾞ
	uchar		version[10];

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "＜磁気リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[4] );			// "【バージョン確認】　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[6] );			// "　バージョン → 　　　　　　　"

	Fun_Dsp( FUNMSG[8] );						// "　　　　　　　　　　　　 終了 "

	// 8桁のバージョンを表示する
	opr_snd( 94 );								// ﾊﾞｰｼﾞｮﾝ要求

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信

		switch( msg ){							// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:					// Ｆ５（終了）
				BUZPI();
				ret = MOD_EXT;
				break;
			case ARC_CR_VER_EVT:				// ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ受信
				memset(version, 0, sizeof(version));
				memcpy(version, RVD_REC.vers, sizeof(RVD_REC.vers));
				grachr( 3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, version );		// バージョンＮｏ．表示
				break;
			default:							// その他
				break;
		}
	}
	return( ret );
}


/*[]----------------------------------------------------------------------[]*/
/*| コインメックチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Cmc( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	COINCHK_CNT	4	//予蓄、保留は別画面

unsigned short	FunChk_Cmc( void ){

	unsigned short	usFcmcEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		/* // [00]	"＜コインメックチェック＞　　　" */

		usFcmcEvent = Menu_Slt( FCMCMENU, FCMC_CHK_TBL, (char)FCMC_CHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFcmcEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;
			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				return( MOD_EXT );
				break;

			case FCM1_CHK:
				wopelg( OPLOG_NYUKINCHK, 0, 0 );	// 操作履歴登録
				usFcmcEvent = CmcInchk();
				break;
			case FCM2_CHK:
				wopelg( OPLOG_HARAIDASHICHK, 0, 0 );	// 操作履歴登録
				usFcmcEvent = CmcOutchk();
				break;
			case FCM3_CHK:
				usFcmcEvent = CmcStschk();
				break;
			case FCM4_CHK:
				usFcmcEvent = CmcVerchk();
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFcmcEvent;
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFcmcEvent == MOD_CHG ){
		if( usFcmcEvent == MOD_CHG || usFcmcEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFcmcEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcInchk                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸ入金ﾃｽﾄ処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcInchk( void )
{
	unsigned short	ret=0;					// 戻り値
	short			msg = -1;				// 受信ﾒｯｾｰｼﾞ
	unsigned char	i;						// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned char	cincnt[COINCHK_CNT];	// ｺｲﾝ入金枚数保存ﾜｰｸ
	unsigned char	cnt_total;				// ｺｲﾝ入金枚数合計
	memset( cincnt, 0, COINCHK_CNT );		// ｺｲﾝ入金枚数保存ﾜｰｸｸﾘｱ
	cn_stat( 33, 0 );						// 入金許可(CREM:ON)

	InchkDsp();								// 入金テスト画面初期状態表示
	for( i = 0; i < COINCHK_CNT ; i++ ){			// ｺｲﾝ入金枚数表示
		opedsp( (unsigned short)(i + 2), 14,  0, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );			// ０枚表示
	}
	xPause( 50L );							// 500ms Pause

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( msg ){						// 受信ﾒｯｾｰｼﾞ？

// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:				// Ｆ１（画面切替）

				BUZPI();

				ret = CmcInchk2( cincnt );	// 釣銭枚数表示画面処理

				if( ret == 0 ){				// 入金テスト継続？
					InchkDsp();										// 入金テスト画面初期状態表示
					for( i = 0; i < COINCHK_CNT ; i++ ){			// ｺｲﾝ入金枚数表示
						cnt_total = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
						opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(cnt_total), 3, 0, 0,
																		COLOR_BLACK, LCD_BLINK_OFF );
					}
				}

				break;

			case KEY_TEN_F3:				// Ｆ３（クリア）

				BUZPI();

				memset( cincnt, 0, COINCHK_CNT );					// ｺｲﾝ入金枚数保存ﾜｰｸｸﾘｱ

				for( i = 0; i < COINCHK_CNT ; i++ ){				// ｺｲﾝ入金枚数表示（全て０枚）
					opedsp( (unsigned short)(i + 2), 14, 0, 3, 0, 0,
									COLOR_BLACK, LCD_BLINK_OFF  );
				}
				cn_stat( 8, 0 );									// 入金不可(CREM:OFF,投入金ｸﾘｱ)
				Lagtim( OPETCBNO, 6, 5*50 );						// ﾀｲﾏｰ6(5s)起動(ﾒﾝﾃﾅﾝｽ制御用)
				for( ; ; ){
					msg = GetMessage();								// ﾒｯｾｰｼﾞ受信
					if( msg == COIN_EVT )							// コインメックイベント
						break;
					// コインメックライムアウト監視
					if( msg == TIMEOUT6 ){		// タイマー６タイムアウト監視
						break;
					}
				}
				Lagcan( OPETCBNO, 6 );
				cn_stat( 5, 0 );									// 入金許可(CREM:ON,投入金ｸﾘｱ)

				break;

			case COIN_EVT:					// コインメックイベント

				switch( OPECTL.CN_QSIG ){

					case	1:				// ｺｲﾝﾒｯｸから「入金あり」受信
					case	5:				// ｺｲﾝﾒｯｸから「入金あり（max達成）」受信

						for( i = 0; i < COINCHK_CNT ; i++ ){		// ｺｲﾝ入金枚数表示
							cnt_total = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(cnt_total), 3, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
						}
						if( OPECTL.CN_QSIG == 5 ){

							for( i = 0; i < COINCHK_CNT ; i++ ){	// ｺｲﾝ入金枚数保存ﾜｰｸに受信ﾃﾞｰﾀ（枚数）を加算
								cincnt[i] = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// 入金許可(CREM:ON,投入金ｸﾘｱ)
						}

						break;

					default:
						break;
				}

				break;

			default:						// その他
				break;
		}
		if( ret != 0 ){		// 入金テスト終了？

			cn_stat( 8, 0 );				// 入金不可(CREM:OFF,投入金ｸﾘｱ)

			Lagtim( OPETCBNO, 6, 5*50 );	// ﾀｲﾏｰ6(5s)起動(ﾒﾝﾃﾅﾝｽ制御用)
			for( ; ; ){
				msg = GetMessage();			// ﾒｯｾｰｼﾞ受信
				if( msg == COIN_EVT )		// コインメックイベント
					break;
				// コインメックライムアウト監視
				if( msg == TIMEOUT6 ){		// タイマー６タイムアウト監視
					break;
				}

			}
			Lagcan( OPETCBNO, 6 );
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InchkDsp                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸ入金ﾃｽﾄ画面初期状態表示                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	InchkDsp( void )
{
	unsigned char	i;						// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "＜コインメックチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[1] );		// "【入金テスト】　　　　　　　　"
	Fun_Dsp( FUNMSG[53] );					// "　▼　　　　クリア　　　 終了 "

	for( i = 0; i < COINCHK_CNT ; i++ ){	// ｺｲﾝ入金枚数表示（全て０枚）
		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );	// "金種"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	// "："
		grachr( (unsigned short)(i + 2), 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "枚"
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcInchk2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸ入金ﾃｽﾄ処理２（釣銭枚数表示画面処理）             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcInchk2( uchar *cincnt )
{
	unsigned short	ret		= 0;			// 戻り値
	unsigned short	dspend	= OFF;			// 釣銭表示画面終了ﾌﾗｸﾞ
	short			msg = -1;				// 受信ﾒｯｾｰｼﾞ
	unsigned char	i;						// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned short	yosub;					// 予/SUB枚数


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "＜コインメックチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[8] );		// "　　　　　　　保有枚数　予/SUB"
																			// "　　　１０円：　　　枚　　　枚"
																			// "　　　５０円：　　　枚　　　枚"
																			// "　　１００円：　　　枚　　　枚"
																			// "　　５００円：　　　枚　　　枚"
	Fun_Dsp( FUNMSG[12] );													// "　▲　　　　　　　　　　 終了 "

	for( i = 0; i < COINCHK_CNT ; i++ ){									// 釣銭枚数、予/SUB枚数　表示

		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );						// "金種"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );						// "："
		opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(CN_RDAT.r_dat07[i]), 3, 0, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );					// 保有枚数
		grachr( (unsigned short)(i + 2), 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );							// "枚"

		if( i == 0 ){												// 10円？
			yosub = (												// 10円枚数ﾊﾞｲﾅﾘー変換
						( (CN_RDAT.r_dat07[5] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[4] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[4] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0 , COLOR_BLACK, LCD_BLINK_OFF );									// 予/SUB 10円　枚数表示
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "枚"
		}
		if( i == 1 ){												// 50円？
			yosub = (												// 50円枚数ﾊﾞｲﾅﾘー変換
						( (CN_RDAT.r_dat07[7] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[6] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[6] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0 , COLOR_BLACK, LCD_BLINK_OFF );									// 予/SUB 50円　枚数表示
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "枚"
		}
		if( i == 2 ){												// 100円？
			yosub = (												// 100円枚数ﾊﾞｲﾅﾘー変換
						( (CN_RDAT.r_dat07[9] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[8] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[8] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );									// 予/SUB 100円　枚数表示
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "枚"
		}
	}

	for( ; dspend == OFF ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( msg){						// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				dspend = ON;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;
				dspend = ON;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;
				dspend = ON;

				break;

			case KEY_TEN_F1:				// Ｆ１（画面切替）

				BUZPI();
				dspend = ON;

				break;

			case COIN_EVT:					// コインメックイベント

				switch( OPECTL.CN_QSIG ){

					case	1:				// ｺｲﾝﾒｯｸから「入金あり」受信
					case	5:				// ｺｲﾝﾒｯｸから「入金あり（max達成）」受信

						for( i = 0; i < COINCHK_CNT ; i++ ){		// 保有枚数表示
							opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(CN_RDAT.r_dat07[i]), 3, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
						}

						if( OPECTL.CN_QSIG == 5 ){
							for( i = 0; i < COINCHK_CNT ; i++ ){	// ｺｲﾝ入金枚数保存ﾜｰｸに受信ﾃﾞｰﾀ（枚数）を加算
								cincnt[i] = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// 入金許可(CREM:ON,投入金ｸﾘｱ)
						}

						break;

					default:
						break;
				}

				break;

			default:
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : bcdadd                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BCDﾃﾞｰﾀ加算処理（1ﾊﾞｲﾄBCD+1ﾊﾞｲﾄBCD→1ﾊﾞｲﾄBCD）          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char data1                                     |*/
/*|              : unsigned char data2                                     |*/
/*| RETURN VALUE : unsigned char                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	bcdadd( uchar data1, uchar data2 )
{
	ushort	wrk1,wrk2;
	uchar	ret;

	wrk1 = (data1 >> 4)*10;		// data1ﾊﾞｲﾅﾘ変換
	wrk1 += (data1 & 0x0f);

	wrk2 = (data2 >> 4)*10;		// data2ﾊﾞｲﾅﾘ変換
	wrk2 += (data2 & 0x0f);

	wrk1 += wrk2;				// data1+data2
	if( wrk1 > 99 ){
		wrk1 = 99;
	}
	ret = (uchar)( ( (wrk1/10) << 4 ) + ( wrk1%10 ) );	// 計算結果をBCDに変換

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcOutchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸ払出ﾃｽﾄ処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcOutchk( void )
{
	unsigned short	ret		= 0;			// 戻り値
	unsigned short	re_vl	= 0;			// 払戻金額
	unsigned short	cn_st	= OFF;			// ｺｲﾝﾒｯｸ状態
	short			msg 	= -1;			// 受信ﾒｯｾｰｼﾞ

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "＜コインメックチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[2] );		// "【払出しテスト】　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[3] );		// "　払出金額：　　　０円　　　　"
	grachr( 3, 12,  6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// 金額入力部分３桁を反転表示（ｽﾍﾟｰｽ）
	Fun_Dsp( FUNMSG[52] );					// "　　　　　　クリア 払出  終了 "


	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( KEY_TEN0to9( msg ) ){		// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F3:				// Ｆ３（クリア）

				if( cn_st != ON ){			// 払出中？（払出中は無効とする）

					BUZPI();
					re_vl = 0;								// 払出金額ｸﾘｱ
					grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );		// 払出金額を反転表示（ｽﾍﾟｰｽ）
				}

				break;


			case KEY_TEN_F4:				// Ｆ４（払出）

				if( cn_st != ON ){			// 払出中？（払出中は無効とする）


					if( re_vl != 0 ) {														// 払出金額≠０？
						BUZPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[9] );	// "　硬貨を払出中です　　　　　　"
						Fun_Dsp( FUNMSG[8] );												// "　　　　　　　　　　　　 終了 "
						cn_stat( 2, 0 );													// 入金不可
						cn_st = ON;															// 払出中ｽﾃｰﾀｽｾｯﾄ
					}
					else{
						BUZPIPI();
					}
				}

				break;

			case KEY_TEN:					// ＴＥＮキー（０～９）

				if( cn_st != ON ){			// 払出中？（払出中は無効とする）

					BUZPI();

					re_vl = ( (re_vl % 100) * 10 ) + ( msg - KEY_TEN0 );					// 払出金額ｾｯﾄ

					if( re_vl == 0 ) {														// 払出金額＝０？
						grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// 払出金額を反転表示（ｽﾍﾟｰｽ）
					}
					else{
						opedsp( 3,12, re_vl, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 払出金額表示
					}
				}

				break;

			case COIN_EVT:					// コインメックイベント

				switch( OPECTL.CN_QSIG ){

					case	2:				// ｺｲﾝﾒｯｸから「払出し完了」受信

						if( cn_st == ON ){														// 払出中？

							cn_st = OFF;														// 払出中ｽﾃｰﾀｽﾘｾｯﾄ
							re_vl = 0;															// 払出金額ｸﾘｱ
							grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// 払出金額を反転表示（ｽﾍﾟｰｽ）
							displclr( 5 );						// ”払出中”表示ＯＦＦ
							Fun_Dsp( FUNMSG[52] );				// "　　　　　　クリア 払出  終了 "
							LedReq( CN_TRAYLED, LED_OFF );		// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
						}

						break;

					case	7:				// ｺｲﾝﾒｯｸから「払出し可状態」受信

						if( cn_st == ON ){						// 払出中？
							LedReq( CN_TRAYLED, LED_ON );		// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点灯

							safecl( 0 ); 						/* 金庫入金前処理 */
							refund( (long)(re_vl*10) );			// 払出開始
						}

						break;

					default:				// その他
						break;
				}

				break;

			default:						// その他
				break;
		}
		if( ret != 0 ){			// 払出テスト終了？
			LedReq( CN_TRAYLED, LED_OFF );		// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
			cn_stat( 8, 0 );					// 入金不可(保有枚数送信)
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcStschk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸ状態確認処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// ｺｲﾝﾒｯｸ異常ｽﾃｰﾀｽﾁｪｯｸ用ﾃﾞｰﾀ（r_dat0cのｵﾌｾｯﾄ＆ﾁｪｯｸbit）
const	unsigned char	cmc_err_chk[10][2] =
	{
		0,	0,			// ﾀﾞﾐｰ
		0,	0x08,		// ｱｸｾﾌﾟﾀ異常
		0,	0x10,		//  10円ｴﾝﾌﾟﾃｨｽｲｯﾁ
		0,	0x20,		//  50円ｴﾝﾌﾟﾃｨｽｲｯﾁ
		0,	0x40,		// 100円ｴﾝﾌﾟﾃｨｽｲｯﾁ
		0,	0x80,		// 500円ｴﾝﾌﾟﾃｨｽｲｯﾁ
		1,	0x10,		// 返金ｽｲｯﾁ
		1,	0x20,		// 硬貨払出し
		1,	0x40,		// ｾｰﾌﾃｨｽｲｯﾁ（釣銭ｶｾｯﾄ）
		1,	0x80,		// ﾊﾟﾙｽｽｲｯﾁ
	};

unsigned short	CmcStschk( void )
{
	unsigned short	ret			= 0;	// 戻り値
	unsigned char	e_page_cnt	= 1;	// 異常ｽﾃｰﾀｽ表示ﾍﾟｰｼﾞ数
	unsigned char	e_page_pos	= 1;	// 異常ｽﾃｰﾀｽ表示ﾍﾟｰｼﾞ位置
	unsigned char	e_cnt		= 0;	// 異常ｽﾃｰﾀｽ数
	unsigned char	i;					// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned char	err_no[12];			// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱ
	unsigned char	set_pos;			// 異常ｽﾃｰﾀｽNO.格納ﾎﾟｲﾝﾀ
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ


	memset( err_no, 0, 12 );					// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱｸﾘｱ

	dispclr();									// 画面ｸﾘｱ

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );			// "＜コインメックチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[4] );			// "【状態確認】　　　　　　　　　"

	if( cn_errst[0] & 0xc0 ){					// 通信エラー発生中？

		// 通信エラー発生中の場合

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[10] );		// "　コインメック通信不良　　　　"
	}
	else{
		// 通信エラー発生中でない場合

		set_pos = 0;
		for( i = 1 ; i < 10 ; i++ ){			// 異常ｽﾃｰﾀｽﾃﾞｰﾀ検索
			if( CN_RDAT.r_dat0c[cmc_err_chk[i][0]] & cmc_err_chk[i][1] ){
				// 異常あり
				e_cnt++;						// 異常ｽﾃｰﾀｽ数更新（+1）
				err_no[set_pos] = i;			// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱへ異常ｽﾃｰﾀｽNO.を格納
				set_pos++;						// 格納ﾎﾟｲﾝﾀ更新（+1）
			}
		}

		if( e_cnt ){							// 異常あり？

			// ｺｲﾝﾒｯｸ異常がある場合

			if( e_cnt <= 5 ){					// 異常ｽﾃｰﾀｽ数から表示ﾍﾟｰｼﾞ数をｾｯﾄ
				e_page_cnt = 1;
			}
			else if( e_cnt <= 10 ){
				e_page_cnt = 2;
			}
			else{
				e_page_cnt = 3;
			}

			Cmc_err_dsp( e_page_pos, &err_no[0] );	// 異常ｽﾃｰﾀｽ表示

		}else{
			// ｺｲﾝﾒｯｸ異常がない場合

			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[5] );	// "　正常です　　　　　　　　　　"
		}
	}

	if( e_page_cnt == 1 ){						// ﾌｧﾝｸｼｮﾝｷｰ表示
		// 他に表示ページがない場合
		Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 "
	}
	else{
		// 他に表示ページがある場合
		Fun_Dsp( FUNMSG[6] );					// "　▼　　▲　　　　　　　 終了 "
	}

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信

		switch( msg){							// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:					// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:					// Ｆ１（▲：前ページ切替）

				if( e_page_cnt != 1 ){

					BUZPI();

					if( e_page_pos == 1 ){
						e_page_pos = e_page_cnt;
					}
					else{
						e_page_pos--;
					}
					Cmc_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			case KEY_TEN_F2:					// Ｆ２（▼：次ページ切替）

				if( e_page_cnt != 1 ){

					BUZPI();

					e_page_pos++;
					if( e_page_pos > e_page_cnt ){
						e_page_pos = 1;
					}
					Cmc_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			default:							// その他
				break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cmc_err_dsp                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸﾁｪｯｸ異常ｽﾃｰﾀｽ表示処理                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char pos（表示ﾍﾟｰｼﾞ位置）                      |*/
/*|              : unsigned char *data（異常ｽﾃｰﾀﾃﾞｰﾀｽﾎﾟｲﾝﾀ）               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Cmc_err_dsp( unsigned char pos, unsigned char *data )
{
	unsigned char	max;
	unsigned char	ofs;
	unsigned char	no;
	unsigned char	i;

	switch( pos ){		// ﾍﾟｰｼﾞ位置？

		case	1:		// １ページ目
			max = 5;
			ofs = 0;
			break;

		case	2:		// ２ページ目
			max = 5;
			ofs = 5;
			break;

		case	3:		// ３ページ目
			max = 2;
			ofs = 10;
			break;

		default:		// その他（ページエラー）
			return;
	}
	for( i = 2 ; i <= 6 ; i++ ){	// ﾗｲﾝ２～６ｸﾘｱ
		displclr( (unsigned short)i );
	}

	for( i = 0 ; i < max ; i++ ){	// 異常ｽﾃｰﾀｽ表示（指定ページの最大行数、または表示ﾃﾞｰﾀ終了まで）

		no = data[ofs+i];																			// 表示する異常ｽﾃｰﾀｽNO.取得
		if( no == 0 ){
			break;																					// 表示ﾃﾞｰﾀ終了
		}
		grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMESTR[no-1] );	// 異常ｽﾃｰﾀｽ表示
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcVerchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｺｲﾝﾒｯｸﾊﾞｰｼﾞｮﾝ確認処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcVerchk( void )
{
	unsigned short	ret=0;			// 戻り値
	short			msg = -1;		// 受信ﾒｯｾｰｼﾞ

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "＜コインメックチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[6] );		// "【バージョン確認】　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[7] );		// "　バージョン → 　　－　　　　"

	opedsp( 3, 16, (unsigned short)( bcdbin( CN_RDAT.r_dat0d[0] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 22, (unsigned short)( bcdbin( CN_RDAT.r_dat0d[1] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( msg){						// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			default:						// その他
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ann                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ｱﾅｳﾝｽﾁｪｯｸ処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810100(S)
//unsigned short	FunChk_Ann( void )
//{
//
//	unsigned short	ret		= 0;			// 戻り値
//	short			msg		= -1;			// 受信ﾒｯｾｰｼﾞ
//	unsigned short	req_msg	= 0;			// 確認要求ﾒｯｾｰｼﾞ番号
//	unsigned char	msg_snd	= 0;			// ﾒｯｾｰｼﾞ送信済みﾌﾗｸﾞ
//	char	ch	= 0;			// チャンネル指定
//	unsigned char	pos		= 0;			// 文節No(4) or 登録No(5)
//	unsigned short	entry	= 0;			// 登録No(1-50)
//	char soundVersion[20];
//	unsigned short cnt;
//	unsigned short cnt2 = 0;
//	ulong	data_cnt = 0;
//	ulong	buf_cnt;
//	ulong	data_len= 0;
//	ulong	read_Addr = 0;
//	ulong	sect_cnt;
//	ushort	chk_sum = 0;
//	ushort	cmp_sum = 0;
//	
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "＜アナウンスチェック＞　　　　"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[10] );							// "　　　　　チェック中　　　　　"
//
//	if(BootInfo.sw.wave == OPERATE_AREA1) {
//		FLT_read_wave_sum_version(0,swdata_write_buf);// 音声データ面1の情報をリード
//		read_Addr = FLT_SOUND0_SECTOR;
//	}
//	else {
//		FLT_read_wave_sum_version(1,swdata_write_buf);// 音声データ面2の情報をリード
//		read_Addr = FLT_SOUND1_SECTOR;
//	}
//	// レングスの取得
//	data_len = (ulong)(swdata_write_buf[LENGTH_OFFSET]<<24);
//	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+1]<<16);
//	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+2]<<8);
//	data_len += (ulong)swdata_write_buf[LENGTH_OFFSET+3];
//	// チェックサムの取得
//	cmp_sum = (ushort)(swdata_write_buf[SUM_OFFSET]<<8);
//	cmp_sum += (ushort)swdata_write_buf[SUM_OFFSET+1];
//	// FROMから音声データを読み出しサムを算出する
//	for(sect_cnt = 0; sect_cnt < FLT_SOUND0_SECTORMAX; sect_cnt++,read_Addr += FLT_SOUND0_SECT_SIZE) {
//		taskchg( IDLETSKNO );
//		FlashReadData_direct(read_Addr, &FLASH_WRITE_BUFFER[0], FLT_SOUND0_SECT_SIZE);// 1セクタ分をリード
//		for (buf_cnt = 0; buf_cnt < FLT_SOUND0_SECT_SIZE; buf_cnt++) {
//			chk_sum += FLASH_WRITE_BUFFER[buf_cnt];
//			data_cnt++;
//			if(data_len <= data_cnt){
//				goto smchk_loop_end;
//			}
//		}
//	}
//smchk_loop_end:
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "＜アナウンスチェック＞　　　　"
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[0] );							// "バージョン                    "
//	memset(&soundVersion, 0, sizeof(soundVersion));
//	for(cnt=0; cnt<sizeof(SOUND_VERSION); cnt++){
//		if(((SOUND_VERSION[cnt] >= 0x30) && (SOUND_VERSION[cnt] <= 0x39)) || 
//			((SOUND_VERSION[cnt] >= 0x41) && (SOUND_VERSION[cnt] <= 0x5a))){				// 0～9 または A～Z
//			soundVersion[cnt2] = 0x82;
//			soundVersion[cnt2+1] = 0x1f + SOUND_VERSION[cnt];
//			cnt2 += 2;
//		}
//		else{
//			if(SOUND_VERSION[cnt] == 0x20){
//				soundVersion[cnt2] = 0x81;
//				soundVersion[cnt2+1] = 0x40;
//				cnt2 += 2;
//			}
//			else{
//				break;
//			}
//		}
//	}
//	grachr( 2, 11, (unsigned short)strlen(soundVersion), 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// 音声ROMバージョン
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[3] );							// "　　　 （　　　　 　　）　　　"
//	opedsp5(3, 9, (ulong)chk_sum, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	if(cmp_sum == chk_sum) {				// SUM値OK
//		grachr(3, 18, 4, 0 ,COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[0]);
//	}
//	else {									// SUM値NG
//		grachr(3, 18, 4, 0 ,COLOR_RED, LCD_BLINK_OFF, DAT2_3[1]);
//		BUZPIPI();
//	}
//	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[1] );							// "　メッセージNo.：　　　　　　 "
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[2] );							// "　登録No.      ：　　　　　　 "
//	opedsp( 4, 17, req_msg, 3, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
//	opedsp( 5, 19, entry, 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
//	Fun_Dsp( FUNMSG[115] );																	// "　▲　　▼　 切替  実行  終了 "
//	
//	memset(avm_test_no, 0, sizeof(avm_test_no));
//	for( ; ; ){
//
//		msg = StoF( GetMessage(), 1 );
//
//		switch( KEY_TEN0to9( msg ) ){		// 受信ﾒｯｾｰｼﾞ？
//
//			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替
//
//				BUZPI();
//				ret = MOD_CHG;
//
//				break;
//			case KEY_TEN_F5:				// Ｆ５（終了）
//
//				BUZPI();
//				ret = MOD_EXT;
//
//				break;
//
//			case KEY_TEN_F4:				// Ｆ４（実行）
//				ope_anm( AVM_STOP );			// 案内メッセージ停止要求
//				if (pos == 0) {
//					if(req_msg > ANN_MSG_MAX){
//						BUZPIPI();
//					} else {
//						switch(req_msg){
//// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
////						case AVM_Edy_OK:
////						case AVM_Edy_NG:
//// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
//
//						case AVM_BOO:
//						case AVM_BGM_MSG:
//							ch = 1;
//							break;
//						default:
//							ch = 0;
//							break;
//						}
//						avm_test_no[0] = req_msg;
//						avm_test_ch = ch;
//						avm_test_cnt = 1;
//						ope_anm(AVM_AN_TEST);
//					}
//				} else {
//					if((entry == 0) || (entry > 50)){
//						BUZPIPI();
//					}
//					else{
//						ope_anm(entry+1);
//					}
//				}
//				msg_snd = 1;				// ﾒｯｾｰｼﾞ送信済み
//				break;
//			case KEY_TEN_F3:				// Ｆ３（切替）
//				BUZPI();
//				if (pos == 0) {		// 文節Noクリア
//					req_msg = (req_msg >= 999) ? 0 : (req_msg + 1);
//					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
//				} else {			// 登録Noクリア
//					entry = (entry >= 50) ? 0 : (entry + 1);
//					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
//				}
//				break;
//			case KEY_TEN_F2:				// Ｆ２（▼）
//			case KEY_TEN_F1:				// Ｆ１（▲）
//				BUZPI();
//				pos ^= 1;
//				if (pos == 0) {
//					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// 文節№番号表示（反転表示）
//					opedsp(5, 19, entry, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 登録№番号表示（正転表示）
//				} else {
//					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 登録№番号表示（反転表示）
//					opedsp(4, 17, req_msg, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 文節№番号表示（正転表示）
//				}
//				break;
//			case KEY_TEN:					// ＴＥＮキー（０～９）
//
//				BUZPI();
//
//				if( msg_snd == 1 ){			// ﾒｯｾｰｼﾞ送信済み?
//					msg_snd = 0;			// ﾒｯｾｰｼﾞ未送信
//					if (pos == 0) {
//						req_msg = 0;		// ﾒｯｾｰｼﾞ番号ｸﾘｱ
//					} else {
//						entry = 0;			// 登録番号ｸﾘｱ
//					}
//				}
//				if (pos == 0) {
//					req_msg = ( (req_msg % 100) * 10 ) + ( msg - KEY_TEN0 );			// ﾒｯｾｰｼﾞ番号ｾｯﾄ
//					opedsp( 4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ﾒｯｾｰｼﾞ番号表示（反転表示）
//				} else {
//					entry = ( (entry % 10) * 10 ) + ( msg - KEY_TEN0 );				// 登録番号ｾｯﾄ
//					opedsp( 5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 登録番号表示（反転表示）
//				}
//				break;
//			default:						// その他
//				break;
//		}
//		if( ret != 0 ){		// アナウンスチェック終了？
//
//			ope_anm( AVM_STOP );			// 案内メッセージ停止要求
//			break;
//		}
//	}
//	return( ret );
//}
//
unsigned short	FunChk_Ann( void )
{

	unsigned short	ret		= 0;			// 戻り値
	short			msg		= -1;			// 受信ﾒｯｾｰｼﾞ
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//	unsigned short	req_msg	= 0;			// 確認要求ﾒｯｾｰｼﾞ番号
	unsigned short	req_msg	= 1;			// 確認要求ﾒｯｾｰｼﾞ番号
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
	unsigned char	msg_snd	= 0;			// ﾒｯｾｰｼﾞ送信済みﾌﾗｸﾞ
	unsigned char	pos		= 0;			// 文節No(4) or 登録No(5)
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//	unsigned short	entry	= 0;			// 登録No(1-50)
	unsigned short	entry	= 1;			// 登録No(1-50)
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
	char soundVersion[16];
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:547)対応
	uchar soundVersionLen;
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:547)対応
	uchar	startType = 0;					// 放送開始条件 ( 0 = 現在の放送を中断して放送を開始する)	
											// ( 1 = 予約)
											// ( 2 = 待ち状態の放送を含めすべての放送終了後に開始する)	

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "＜アナウンスチェック＞　　　　"

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "＜アナウンスチェック＞　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[0] );							// "バージョン                    "
	memset(&soundVersion, 0, sizeof(soundVersion));
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:547)対応
//	lcdbm_get_config_audio_ver( soundVersion, sizeof(soundVersion) );
//	grachr( 2, 16, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// 音声ROMバージョン
	soundVersionLen = lcdbm_get_config_audio_ver( soundVersion, sizeof(soundVersion) );
	grachr( 2, 16, (unsigned short)soundVersionLen, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// 音声ROMバージョン
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:547)対応
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[1] );							// "　メッセージNo.：　　　　　　 "
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[2] );							// "　登録No.      ：　　　　　　 "
	opedsp( 4, 17, req_msg, 3, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
	opedsp( 5, 19, entry, 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
	Fun_Dsp( FUNMSG[115] );																	// "　▲　　▼　 切替  実行  終了 "
	
	memset(avm_test_no, 0, sizeof(avm_test_no));
	for( ; ret == 0; ){
		msg = StoF( GetMessage(), 1 );
		switch( msg ){		// 受信ﾒｯｾｰｼﾞ
			case LCD_DISCONNECT:
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
				ret = MOD_CUT;
				break;
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:				// Ｆ５（終了）
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
				BUZPI();
				ret = MOD_EXT;
				break;
			case KEY_TEN_F4:				// Ｆ４（実行）
				if (pos == 0 ){
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//					 if(req_msg > ANN_MSG_MAX){
					if (req_msg == 0 || req_msg > ANN_MSG_MAX) {	// 0のメッセージNoは存在しないのでエラー扱い
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
						BUZPIPI();
					 } else {
						PKTcmd_audio_start(startType, ( uchar )pos, ( ushort )req_msg);	// アナウンス開始要求
					}
				}else{
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//					if((entry == 0) || (entry > 50)){
					if ((entry == 0) || (entry > 99)) {	// 登録Noは99まで
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
						BUZPIPI();
					}
					else{
				 		PKTcmd_audio_start(startType, ( uchar )pos, ( ushort )entry);	// アナウンス開始要求
					}
				}
				msg_snd = 1;				// ﾒｯｾｰｼﾞ送信済み
				break;
			case KEY_TEN_F3:				// Ｆ３（切替）
				BUZPI();
				if (pos == 0) {		// 文節Noクリア
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//					req_msg = (req_msg >= ANN_MSG_MAX) ? 0 : (req_msg + 1);
					req_msg = (req_msg >= ANN_MSG_MAX) ? 1 : (req_msg + 1);
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
				} else {			// 登録Noクリア
// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
//					entry = (entry >= 50) ? 0 : (entry + 1);
					entry = (entry >= 99) ? 1 : (entry + 1);
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4832 メンテナンスのアナウンスチェックで「メッセージNo」と「登録No」で再生される音声が逆になっている)
					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
				}
				break;
			case KEY_TEN_F2:				// Ｆ２（▼）
			case KEY_TEN_F1:				// Ｆ１（▲）
				BUZPI();
				pos ^= 1;
				if (pos == 0) {
					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// 文節№番号表示（反転表示）
					opedsp(5, 19, entry, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 登録№番号表示（正転表示）
				} else {
					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 登録№番号表示（反転表示）
					opedsp(4, 17, req_msg, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 文節№番号表示（正転表示）
				}
				break;
			case KEY_TEN0:					// ＴＥＮキー（０～９）
			case KEY_TEN1:					// ＴＥＮキー（０～９）
			case KEY_TEN2:					// ＴＥＮキー（０～９）
			case KEY_TEN3:					// ＴＥＮキー（０～９）
			case KEY_TEN4:					// ＴＥＮキー（０～９）
			case KEY_TEN5:					// ＴＥＮキー（０～９）
			case KEY_TEN6:					// ＴＥＮキー（０～９）
			case KEY_TEN7:					// ＴＥＮキー（０～９）
			case KEY_TEN8:					// ＴＥＮキー（０～９）
			case KEY_TEN9:					// ＴＥＮキー（０～９）
				BUZPI();

				if( msg_snd == 1 ){			// ﾒｯｾｰｼﾞ送信済み?
					msg_snd = 0;			// ﾒｯｾｰｼﾞ未送信
					if (pos == 0) {
						req_msg = 0;		// ﾒｯｾｰｼﾞ番号ｸﾘｱ
					} else {
						entry = 0;			// 登録番号ｸﾘｱ
					}
				}
				if (pos == 0) {
					req_msg = ( (req_msg % 100) * 10 ) + ( msg - KEY_TEN0 );			// ﾒｯｾｰｼﾞ番号ｾｯﾄ
					opedsp( 4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ﾒｯｾｰｼﾞ番号表示（反転表示）
				} else {
					entry = ( (entry % 10) * 10 ) + ( msg - KEY_TEN0 );				// 登録番号ｾｯﾄ
					opedsp( 5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 登録番号表示（反転表示）
				}
				break;
			default:						// その他
				break;
		}
		if( ret != 0 ){		// アナウンスチェック終了？
			PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
		}
	}
	return( ret );
}
// MH810100(E)

unsigned short	Ann_chk2( void )
{

	unsigned short	ret		= 0;			// 戻り値
	short			msg		= -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned long	req_ryo	= 0;			// 確認要求ﾒｯｾｰｼﾞ番号
	unsigned short	req_sya = 0;
	unsigned char	msg_snd	= 0;			// ﾒｯｾｰｼﾞ送信済みﾌﾗｸﾞ
	unsigned char	mod = 0;;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "＜アナウンスチェック＞　　　　"
	grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "料金読み上げチェック" );				// "　バージョン → 　　　　　　　"
	opedpl( 5, 10, req_ryo, 6, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
	Fun_Dsp( FUNMSG[81] );																	// "　　　　　　　　　 実行  終了 "
	
	memset(avm_test_no, 0, sizeof(avm_test_no));
	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){		// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;
			case KEY_TEN_F1:				// Ｆ５（終了）
				if(mod){
					mod = 0;
					grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "料金読み上げチェック" );			
					displclr(5);
					opedpl( 5, 10, req_ryo, 6, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
					req_ryo = 0;
				}
				else{
					mod = 1;
					grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "車室読み上げチェック" );			
					displclr(5);
					opedsp( 5, 10, req_sya, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
					req_sya = 0;
				}
				break;
			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F4:				// Ｆ４（実行）
				BUZPI();
				if(mod == 0){
					announceFee = req_ryo;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
//					ope_anm(AVM_RYOUKIN );
					PKTcmd_audio_start( 0, ( uchar ) 0, ( ushort )1/*TODO：放送内容*/);	// アナウンス開始要求
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
				}
				else{
					key_num = req_sya;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
//					ope_anm(AVM_SHASHITU);
					PKTcmd_audio_start( 0, ( uchar ) 0, ( ushort )1/*TODO：放送内容*/);	// アナウンス開始要求
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
				}
				msg_snd = 1;
				break;


			case KEY_TEN:					// ＴＥＮキー（０～９）

				BUZPI();
				if(msg_snd){
					req_ryo = 0;
					req_sya = 0;
					msg_snd = 0;
				}
				if(mod == 0){
					req_ryo = ( (req_ryo % 1000000) * 10 ) + ( msg - KEY_TEN0 );					// 確認要求ﾒｯｾｰｼﾞ番号ｾｯﾄ
					opedpl( 5, 10, req_ryo, 6, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
				}
				else{
					req_sya = ( (req_sya % 1000) * 10 ) + ( msg - KEY_TEN0 );					// 確認要求ﾒｯｾｰｼﾞ番号ｾｯﾄ
					opedsp( 5, 10, req_sya, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 確認要求ﾒｯｾｰｼﾞ番号表示（反転表示）
					
				}
				break;

			default:						// その他

				break;
		}
		if( ret != 0 ){		// アナウンスチェック終了？
// MH810100(S) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
//				ope_anm( AVM_STOP );		// 案内メッセージ停止要求
				PKTcmd_audio_end( 0, 0 );	// アナウンス終了要求
// MH810100(E) Y.Yamauchi 20191209 車番チケットレス(メンテナンス)
			announceFee = 0;
			key_num = 0;
			break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| BNA Check                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_BNA( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_BNA( void )
{
	unsigned short	usFbnaEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		/* // [00]	"＜紙幣リーダーチェック＞　　　" */

		usFbnaEvent = Menu_Slt( FBNAMENU, FBNA_CHK_TBL, (char)FBNA_CHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFbnaEvent ){
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

			case FBN1_CHK:
				wopelg( OPLOG_NOTENYUCHK, 0, 0 );		// 操作履歴登録
				usFbnaEvent = BnaInchk();
				break;
			case FBN2_CHK:
				usFbnaEvent = BnaStschk();
				break;
			case FBN3_CHK:
				usFbnaEvent = BnaVerchk();
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFbnaEvent;
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFbnaEvent == MOD_CUT ){
		if( usFbnaEvent == MOD_CUT || usFbnaEvent == MOD_CHG ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFbnaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaInchk                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 紙幣ﾘｰﾀﾞｰ入金ﾃｽﾄ処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	BnaInchk( void )
{

	unsigned short	ret	= 0;			// 戻り値
	short			msg	= -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	rd_cnt	= 0;		// 紙幣読込枚数
	unsigned char	rd_sts	= 0;		// 紙幣ﾘｰﾀﾞｰ状態
										//  0:空き
										//  1:紙幣保留中
										//  2:紙幣戻し完了待ち
										//  3:紙幣取込完了待ち


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		// "＜紙幣リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[1] );		// "【入金テスト】　　　　　　　　"
	Fun_Dsp( FUNMSG[8] );													// "　　　　　　 　　　　　　終了 "
												
	grachr( 3,  0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_2[0] );		// "１０００円札"
	grachr( 3, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		// "："
	opedsp( 3, 16,  0, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );											// "０"
	grachr( 3, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		// "枚"

	cn_stat( 1, 1 );						// 入金許可

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( msg ){						// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F3:				// Ｆ３（戻し）

				if( rd_sts == 1 ){			// 紙幣保留中？

					BUZPI();
					cn_stat( 2, 1 ); 		// 入金不可
					rd_sts = 2;				// 状態：紙幣戻し完了待ち
					Fun_Dsp( FUNMSG[8] );	// "　　　　　　　　　　　　 終了 "
				}

				break;

			case KEY_TEN_F4:				// Ｆ４（取込）

				if( rd_sts == 1 ){			// 紙幣保留中？

					BUZPI();
					cn_stat( 1, 1 ); 		// 入金許可
					rd_sts = 3;				// 状態：紙幣取込完了待ち
					Fun_Dsp( FUNMSG[8] );	// "　　　　　　　　　　　　 終了 "
				}

				break;

			case NOTE_EVT:					// 紙幣リーダーイベント

				switch( OPECTL.NT_QSIG ){

					case	1:				// 紙幣ﾘｰﾀﾞｰから「入金あり」受信

						rd_cnt++;																// 読込枚数更新（＋１）
						opedsp( 3, 16, rd_cnt, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 読込枚数表示
						grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[6] );		// "　紙幣を保留中です　　　　　　"
						rd_sts = 1;								// 状態：紙幣保留中
						Fun_Dsp( FUNMSG[55] );					// "　　　　　　 戻し  取込  終了 "

						break;

					case	2:				// 紙幣ﾘｰﾀﾞｰから「払出し完了」受信

						if( rd_sts == 2 ){		// 紙幣戻し完了待ち？

							rd_cnt--;															// 読込枚数更新（－１）
							opedsp( 3, 16, rd_cnt, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 読込枚数表示
							displclr( 5 );														// "　紙幣を保留中です　　　　　　"表示オフ
							rd_sts = 0;															// 状態：空き
							cn_stat( 1, 1);														// 入金許可
						}

						break;

					case	3:				// 紙幣ﾘｰﾀﾞｰから「収金完了」受信

						if( rd_sts == 3 ){		// 紙幣取込完了待ち？

							displclr( 5 );						// "　紙幣を保留中です　　　　　　"表示オフ
							rd_sts = 0;							// 状態：空き
						}

						break;

					case	7:				// 紙幣ﾘｰﾀﾞｰから「払出し可状態」受信

						if( rd_sts == 2 ){		// 紙幣戻し完了待ち？

							cn_stat( 10, 1);
						}

						break;

					default:				// その他
						break;
				}

				break;

			default:						// その他
				break;
		}
		if( ret != 0 ){			// 入金テスト終了？

			cn_stat( 2, 1 );	// 入金不可

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaStschk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 紙幣ﾘｰﾀﾞｰ状態確認処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// 紙幣ﾘｰﾀﾞｰ異常ｽﾃｰﾀｽﾁｪｯｸ用ﾃﾞｰﾀ（r_dat1cのｵﾌｾｯﾄ＆ﾁｪｯｸbit）
const	unsigned char	bna_err_chk[6][2] =
	{
		0,	0,			// ﾀﾞﾐｰ
		0,	0x04,		// 識別部異常
		0,	0x08,		// スタッカー異常
		0,	0x10,		// 紙幣詰まり
		0,	0x20,		// 紙幣払出し異常
		0,	0x80		// 紙幣金庫満杯
	};


unsigned short	BnaStschk( void )
{
	unsigned short	ret			= 0;	// 戻り値
	unsigned char	e_cnt		= 0;	// 異常ｽﾃｰﾀｽ数
	unsigned char	i;					// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned char	err_no[5];			// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱ
	unsigned char	set_pos;			// 異常ｽﾃｰﾀｽNO.格納ﾎﾟｲﾝﾀ
	unsigned char	no;					// 異常ｽﾃｰﾀｽNO.
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ


	memset( err_no, 0, 5 );						// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱｸﾘｱ

	dispclr();									// 画面ｸﾘｱ

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );			// "＜紙幣リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[2] );			// "【状態確認】　　　　　　　　　"

	if( cn_errst[1] & 0xc0 ){					// 通信エラー発生中？

		// 通信エラー発生中の場合

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[7] );		// "　紙幣リーダー通信不良　　　　"
	}
	else{
		// 通信エラー発生中でない場合

		set_pos = 0;
		for( i = 1 ; i <= 5 ; i++ ){			// 異常ｽﾃｰﾀｽﾃﾞｰﾀ検索
			if( NT_RDAT.r_dat1c[bna_err_chk[i][0]] & bna_err_chk[i][1] ){
				// 異常あり
				e_cnt++;						// 異常ｽﾃｰﾀｽ数更新（+1）
				err_no[set_pos] = i;			// 異常ｽﾃｰﾀｽNO.格納ｴﾘｱへ異常ｽﾃｰﾀｽNO.を格納
				set_pos++;						// 格納ﾎﾟｲﾝﾀ更新（+1）
			}
		}

		if( e_cnt ){							// 異常あり？

			// 紙幣ﾘｰﾀﾞｰ異常がある場合

			for( i = 0 ; i < 5 ; i++ ){			// 異常ｽﾃｰﾀｽ（最大５つ）を全て表示する

				no = err_no[i];												// 表示する異常ｽﾃｰﾀｽNO.取得
				if( no == 0 ){
					break;													// 表示ﾃﾞｰﾀ終了
				}
				grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNESTR[no-1] );	// 異常ｽﾃｰﾀｽ表示
			}

		}else{
			// 紙幣ﾘｰﾀﾞｰ異常がない場合

			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[3] );	// "　正常です　　　　　　　　　　"
		}
	}

	Fun_Dsp( FUNMSG[8] );						// "　　　　　　　　　　　　 終了 "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信

		switch( msg){							// 受信ﾒｯｾｰｼﾞ

// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:					// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:					// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			default:							// その他
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaVerchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 紙幣ﾘｰﾀﾞｰﾊﾞｰｼﾞｮﾝ確認処理                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	BnaVerchk( void )
{
	unsigned short	ret=0;			// 戻り値
	short			msg = -1;		// 受信ﾒｯｾｰｼﾞ

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		// "＜紙幣リーダーチェック＞　　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[4] );		// "【バージョン確認】　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[5] );		// "　バージョン → 　　－　　　　"

	opedsp( 3, 16, (unsigned short)( bcdbin( NT_RDAT.r_dat1d[0] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 22, (unsigned short)( bcdbin( NT_RDAT.r_dat1d[1] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	Fun_Dsp( FUNMSG[8] );					// "　　　　　　　　　　　　 終了 "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// ﾒｯｾｰｼﾞ受信

		switch( msg){						// 受信ﾒｯｾｰｼﾞ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// Ｆ５（終了）

				BUZPI();
				ret = MOD_EXT;

				break;

			default:						// その他
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Mck                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾒﾓﾘｰﾁｪｯｸ処理                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned long	mem_chk_adr[3] =
	{
		// ＲＡＭ１は物理的には一つであるがアドレスデコードで2つの空間に分けられる
		0x7000000,		// ＲＡＭ１－１：CS1:アドレス空間0x07000000-0x070FFFFF(1MB)
		0x7400000,		// ＲＡＭ１－２：CS1:アドレス空間0x07400000-0x074FFFFF(1MB)
		0x6000000,		// ＲＡＭ２    ：CS2:アドレス空間0x06000000-0x060FFFFF(1MB)
	};
#define		FCK_INSRAM_ADR	0x00000000	// 内部ＳＲＡＭ開始アドレス：0x00000000～0x00020000（0x20000Byte）
#define		FCK_INSRAM_SIZ	0x20000		// 内部ＳＲＡＭサイズ
#define		FCK_EXSRAM_SIZ	0x100000	// 外部ＳＲＡＭサイズ
#define		FCK_EXSRAM_CHKSIZ	1024*10	// 外部ＳＲＡＭ単位当たりのチェックレングス

// ﾒﾓﾘｰＲＷﾃｽﾄﾃﾞｰﾀ
#define		FMCK_DATA1	0xA5
#define		FMCK_DATA2	0x5A

#define 	SEC			50
unsigned short	FunChk_Mck( void )
{
	short	msg;			// 受信ﾒｯｾｰｼﾞ
	ushort	ret = 0;		// 戻り値
	ushort	i;				// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	uchar	f_flag = 0;		// 0:first/1:not first

	while (1) {
		if (f_flag == 0) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[0]);						// line:0
			for (i = 2; i < 6; i++) {															// line:2～5
				grachr((ushort)i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[i+4]);
			}
			Fun_Dsp(FUNMSG2[47]);					 			// "　　　　　　 実行  　　　終了 "
			f_flag = 1;
		}

		msg = StoF(GetMessage(), 1);					// ﾒｯｾｰｼﾞ受信
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			ret = MOD_CUT;
			return(ret);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:								// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			return(ret);
		case KEY_TEN_F5:								// Ｆ５（終了）
			BUZPI();
			ret = MOD_EXT;
			return(ret);
		case KEY_TEN_F3:								// Ｆ３（実行）
			BUZPI();
			Ope_DisableDoorKnobChime();
			ret = MemoryCheck();
			f_flag = 0;
			break;
		default:										// その他
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (ret == MOD_CHG) {
		if (ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			return(ret);
		}
	}
	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			MemoryCheck(memory check main function)
//[]----------------------------------------------------------------------[]
///	@param[in]		none	: 
///	@return			ret		: MOD_CHG or MOD_EXT
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/12<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static ushort	MemoryCheck(void)
{
	short	msg;			// 受信ﾒｯｾｰｼﾞ
	ushort	ret	= 0;		// 戻り値
	ulong	i;				// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	ushort	chk_cnt;		// 処理経過
	ushort	ram_no;			// ﾁｪｯｸ対象RAM No.
	uchar	*chk_adr;		// ﾁｪｯｸﾒﾓﾘｰｱﾄﾞﾚｽ
	uchar	chk_err;		// ﾁｪｯｸ結果
	uchar	sav_data;		// ﾃﾞｰﾀ退避ｴﾘｱ
	uchar	chk_data1;		// ﾁｪｯｸﾃﾞｰﾀ１
	uchar	chk_data2;		// ﾁｪｯｸﾃﾞｰﾀ２

	dispclr();
	for (i = 0; i < 4; i++) {																// line:0～3
		grachr((ushort)i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[i]);
	}
	grachr(6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[10]);						// "チェック中"反転表示(line:6)
	Fun_Dsp(FUNMSG[0]);
	xPause( 10 );

	//内部ＲＡＭﾁｪｯｸ
	fck_chk_err = 0;
	fck_chk_adr = (uchar *)FCK_INSRAM_ADR;
	for (i = 0 ; i < FCK_INSRAM_SIZ ; i++) {						// ﾁｪｯｸﾚﾝｸﾞｽ分ﾒﾓﾘﾁｪｯｸを行う
		WACDOG;											// WATCHDOGﾀｲﾏｰﾘｾｯﾄ
		_di();											// 割込み禁止
		fck_sav_data 	= *fck_chk_adr;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをｾｰﾌﾞ
		*fck_chk_adr	= FMCK_DATA1;					// ﾁｪｯｸﾃﾞｰﾀ１ﾗｲﾄ
		fck_chk_data1	= *fck_chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ１ﾘｰﾄﾞ
		*fck_chk_adr	= FMCK_DATA2;					// ﾁｪｯｸﾃﾞｰﾀ２ﾗｲﾄ
		fck_chk_data2	= *fck_chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ２ﾘｰﾄﾞ
		if ((fck_chk_data1 != FMCK_DATA1) || (fck_chk_data2 != FMCK_DATA2)) {
			// ＲＷﾁｪｯｸＮＧ
			fck_chk_err = 1;
		}
		*fck_chk_adr = fck_sav_data;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをﾘｶﾊﾞｰ
		fck_chk_adr++;									// ﾁｪｯｸｱﾄﾞﾚｽ更新
		_ei();											// 割込み禁止解除
		if (fck_chk_err != 0) {
			// ＲＷﾁｪｯｸＮＧ
			break;
		}
	}
	grachr(1, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[fck_chk_err]);			// 内部ＲＡＭﾁｪｯｸ結果表示(line:1)
	xPause( 10 );

	//ＲＡＭ１メモリーチェック
	chk_err = 0;
	for (ram_no = 0 ; ram_no <= 1 ; ram_no++) {			// RAM毎のﾒﾓﾘｰﾁｪｯｸ処理ﾙｰﾌﾟ
		chk_err = 0;
		chk_adr = (uchar *)mem_chk_adr[ram_no];
		for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM（1Mbyte）を10Kづつﾁｪｯｸ
			WACDOG;										// WATCHDOGﾀｲﾏｰﾘｾｯﾄ
			for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// ﾁｪｯｸﾚﾝｸﾞｽ分ﾒﾓﾘﾁｪｯｸを行う
				_di();									// 割込み禁止
				sav_data 	= *chk_adr;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをｾｰﾌﾞ
				*chk_adr	= FMCK_DATA1;				// ﾁｪｯｸﾃﾞｰﾀ１ﾗｲﾄ
				chk_data1	= *chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ１ﾘｰﾄﾞ
				*chk_adr	= FMCK_DATA2;				// ﾁｪｯｸﾃﾞｰﾀ２ﾗｲﾄ
				chk_data2	= *chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ２ﾘｰﾄﾞ
				if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
					// ＲＷﾁｪｯｸＮＧ
					chk_err = 1;
				}
				*chk_adr = sav_data;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをﾘｶﾊﾞｰ
				chk_adr++;								// ﾁｪｯｸｱﾄﾞﾚｽ更新
				_ei();									// 割込み禁止解除
				if (chk_err != 0) {
					// ＲＷﾁｪｯｸＮＧ
					break;
				}
			}
			if (chk_err != 0) {
				break;
			}
		}
	}
	grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[chk_err]);				// ＲＡＭﾁｪｯｸ結果表示(line:2)
	xPause( 10 );

	//ＲＡＭ２メモリーチェック
	chk_err = 0;
	chk_adr = (uchar *)mem_chk_adr[2];
	for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM（1Mbyte）を10Kづつﾁｪｯｸ
		WACDOG;										// WATCHDOGﾀｲﾏｰﾘｾｯﾄ
		for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// ﾁｪｯｸﾚﾝｸﾞｽ分ﾒﾓﾘﾁｪｯｸを行う
			_di();									// 割込み禁止
			sav_data 	= *chk_adr;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをｾｰﾌﾞ
			*chk_adr	= FMCK_DATA1;				// ﾁｪｯｸﾃﾞｰﾀ１ﾗｲﾄ
			chk_data1	= *chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ１ﾘｰﾄﾞ
			*chk_adr	= FMCK_DATA2;				// ﾁｪｯｸﾃﾞｰﾀ２ﾗｲﾄ
			chk_data2	= *chk_adr;					// ﾁｪｯｸﾃﾞｰﾀ２ﾘｰﾄﾞ
			if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
				// ＲＷﾁｪｯｸＮＧ
				chk_err = 1;
			}
			*chk_adr = sav_data;					// ﾁｪｯｸﾒﾓﾘｰﾃﾞｰﾀをﾘｶﾊﾞｰ
			chk_adr++;								// ﾁｪｯｸｱﾄﾞﾚｽ更新
			_ei();									// 割込み禁止解除
			if (chk_err != 0) {
				// ＲＷﾁｪｯｸＮＧ
				break;
			}
		}
		if (chk_err != 0) {
			break;
		}
	}
	grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[chk_err]);				// ＲＡＭﾁｪｯｸ結果表示(line:3)
	displclr(6);											// "チェック中"表示クリア
	xPause( 10 );

	Ope_EnableDoorKnobChime();
	Fun_Dsp(FUNMSG[8]);										// "　　　　　　　　　　　　 終了 "
	while (ret == 0) {
		msg = StoF(GetMessage(), 1);						// ﾒｯｾｰｼﾞ受信
		switch (msg) {										// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:								// 操作ﾓｰﾄﾞ切替
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:								// Ｆ５（終了）
				BUZPI();
				ret = MOD_EXT;
				break;
			default:										// その他
				break;
		}
	}
	return(ret);
}
/*[]----------------------------------------------------------------------[]*/
/*| IF盤ﾁｪｯｸ                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ifb( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Ifb( uchar	Car_type )
{
	short	msg = -1;
	short	tno[2], btno[2];
	ushort	top, btop;
	ushort	CNTMAX;
	char	pgchg = 0;
	short	i, j;
	char	mtype;
	char	mod = 0;			// 0:親機及びﾀｰﾐﾅﾙNo表示 1:子機表示
	char	timeout = 1;
	short	tnoIndex[LOCK_IF_MAX];
	char	work;
	ushort	B_CNTMAX;
	const uchar	*title;
	short	terminalCount;

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ﾀｲﾏｰ6(12s)起動(ﾒﾝﾃﾅﾝｽ制御用)

	if( !Car_type ){
		mtype = _MTYPE_LOCK;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[13] );			// [12]	"＜ＩＦ盤チェック＞　　　　　　"
		title = IFCSTR[1];
	}else{
		mtype = _MTYPE_INT_FLAP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[52] );			// [52]	"＜ＣＲＲ基板チェック＞　　　　"	
		title = IFCSTR[4];
	}
	queset( FLPTCBNO, LK_SND_VER, 1, &mtype );	// ﾛｯｸ装置ﾊﾞｰｼﾞｮﾝ要求送信
	Ope_DisableDoorKnobChime();
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);		// [10]	"　　 しばらくお待ち下さい 　　"
	Fun_Dsp(FUNMSG[0]);														/* [77]	"　　　　　　　　　　　　　　　" */

	top =0;
	tno[0] = tno[1] = 0;						// ﾀｰﾐﾅﾙ№ｸﾘｱ
	memset( tnoIndex, 0, sizeof( tnoIndex ));
	
	terminalCount = (short)LKcom_GetAccessTarminalCount();
	for (i = 1,CNTMAX=0; i <= terminalCount; i++) {
		work = LKcom_Search_Ifno( (uchar)i );
		if( !Car_type ){
			if( !work ){
				tnoIndex[CNTMAX] = i;
				CNTMAX++;
			}
		}else{
			if( work ){
				tnoIndex[CNTMAX] = i;
				CNTMAX++;
			}
		}
	}
	B_CNTMAX = CNTMAX;

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"終了"
				BUZPI();
				if( mod == 1 ){							// 子機のﾊﾞｰｼﾞｮﾝ表示?
					BUZPI();
					mod = 0;							// 親機のﾊﾞｰｼﾞｮﾝ表示
					CNTMAX = B_CNTMAX;					// 親機の最大接続数
					top = btop;							// 画面の先頭の№を戻す
					tno[0] = btno[0];					// ﾀｰﾐﾅﾙ№を戻す
					tno[1] = btno[1];					// ﾀｰﾐﾅﾙ№を戻す
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [00]	"　XXXX№　　　　バージョン　　"
					Fun_Dsp( FUNMSG[6] );				// [06]	"　▲　　▼　　　　       終了 "
					pgchg = 1;
				}else{
					Lagcan( OPETCBNO, 6 );				// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
					return( MOD_EXT );
				}
				break;

			case KEY_TEN_F1:	// F1:"▲"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] <= 0 ){
					tno[0] = CNTMAX - 1;
				}else{
					tno[0]--;
				}
				pgchg = pag_ctl( CNTMAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[1]]):(tno[1] + 1)), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					if( top + tno[0] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[0]]):(tno[0] + 1)), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"▼"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] >= CNTMAX - 1 ){
					tno[0] = 0;
				}else{
					tno[0]++;
				}
				pgchg = pag_ctl( CNTMAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[1]]):(tno[1] + 1)), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					if( top + tno[0] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[0]]):(tno[0] + 1)), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}
				}
				break;

			case KEY_TEN_F4:	// F4:"読出"
				break;
			case TIMEOUT6:		// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [00]	"　親機№　　　　バージョン　　"
					Fun_Dsp( FUNMSG[6] );				// [25]	"　▲　　▼　　　　       終了 "
				pgchg = 1;
				timeout = 0;
				break;
			default:
				break;
		}
		if( pgchg ){
			dispmlclr( 2, 6 );											// これから編集する行をクリア
			for( i = 0, j = 0; j < 5 && (top + i) < CNTMAX; i++ ){
				work = LKcom_Search_Ifno( (uchar)(top + i + 1) );
				if( top + j < CNTMAX ){
					grachr( (ushort)(j + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"　№　　　－　　　　　　　　　"
					if( j == tno[0] % 5 ){
						opedsp( (ushort)(j + 2), 4, (ushort)(!mod?(tnoIndex[top + j]):(top + i + 1)), 2, 1, 1,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}else{
						opedsp( (ushort)(j + 2), 4, (ushort)(!mod?(tnoIndex[top + j]):(top + i + 1)), 2, 1, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					memcpy( wlcd_buf, &OPE_CHR[0], sizeof( wlcd_buf ) );	// ｽﾍﾟｰｽ(0x20)ｸﾘｱ
					if( IFM_LockTable.sSlave[tnoIndex[top + j] - 1].cVersion[0] == 0x00 ){
						if( !Car_type )
							memcpy( wlcd_buf, &IFCSTR[3][12], 18 );		// [未受信]表示用
						else
							memcpy( wlcd_buf, &IFCSTR[5][12], 18 );		// [未受信]表示用							
					}else{
						as1chg((uchar*)IFM_LockTable.sSlave[tnoIndex[top + j] - 1].cVersion, wlcd_buf, (uchar)8 );	// 受信したﾊﾞｰｼﾞｮﾝ(ｱｽｷｰ)を表示文字ｺｰﾄﾞへ変換する
					}
					grachr( (ushort)(j + 2), 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// ﾊﾞｰｼﾞｮﾝ表示
					j++;
				}
				pgchg = 0;
			}
			
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| CRR板バージョンチェック                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_CRRVer( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_CRRVer( void )
{
	short	msg = -1;
	ushort	top;
	ushort	CNTMAX;
	char	pgchg = 0;
	short	i, j;
	char	mtype;
	short	tnoIndex[LOCK_IF_MAX];
	const uchar	*title;

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ﾀｲﾏｰ6(12s)起動(ﾒﾝﾃﾅﾝｽ制御用)

	mtype = _MTYPE_INT_FLAP;
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[55] );			// [55]	"＜ＣＲＲバージョンチェック＞  "	
	title = IFCSTR[6];

	queset( FLPTCBNO, LK_SND_VER, 1, &mtype );	// ﾛｯｸ装置ﾊﾞｰｼﾞｮﾝ要求送信
	Ope_DisableDoorKnobChime();
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"　　 しばらくお待ち下さい 　　"
	Fun_Dsp(FUNMSG[0]);														/* [77]	"　　　　　　　　　　　　　　　" */

	top =0;
	memset( tnoIndex, 0, sizeof( tnoIndex ));
	
	for (i = 0,CNTMAX=0; i < IFS_CRR_MAX; i++) {
		if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){	// CRR基板の設定がされているかを確認
			tnoIndex[CNTMAX] = i+1;
			CNTMAX++;
		}
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"終了"
				if( pgchg == 0 ){	// 実行中は終了させない
					break;
				}
				BUZPI();
				Lagcan( OPETCBNO, 6 );				// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1:"▲"
				break;

			case KEY_TEN_F2:	// F2:"▼"
				break;

			case KEY_TEN_F4:	// F4:"読出"
				break;
			case TIMEOUT6:		// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [06]	"　基板№　　　　バージョン　　"
//					Fun_Dsp( FUNMSG[6] );				// [06]	"　▲　　▼　　　　       終了 "
					Fun_Dsp( FUNMSG[8] );				// [08]	"　　　　　　　　　　　　 終了 "
				pgchg = 1;
				break;
			default:
				break;
		}
		if( pgchg ){
			dispmlclr( 2, 6 );											// これから編集する行をクリア
			for( i = 0, j = 0; j < 5 && (top + i) < CNTMAX; i++ ){
				if( top + j < CNTMAX ){
					grachr( (ushort)(j + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"　№　　　－　　　　　　　　　"
					opedsp( (ushort)(j + 2), 4, (ushort)tnoIndex[top + j], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					memcpy( wlcd_buf, &OPE_CHR[0], sizeof( wlcd_buf ) );	// ｽﾍﾟｰｽ(0x20)ｸﾘｱ
					if( IFM_LockTable.sSlave_CRR[tnoIndex[top + j] - 1].cVersion[0] == 0x00 ){
						memcpy( wlcd_buf, &IFCSTR[5][12], 18 );		// [未受信]表示用							
					}else{
						as1chg((uchar*)IFM_LockTable.sSlave_CRR[tnoIndex[top + j] - 1].cVersion, wlcd_buf, (uchar)8 );	// 受信したﾊﾞｰｼﾞｮﾝ(ｱｽｷｰ)を表示文字ｺｰﾄﾞへ変換する
					}
					grachr( (ushort)(j + 2), 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// ﾊﾞｰｼﾞｮﾝ表示
					j++;
				}
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			CRR基板チェックメニュー表示
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort FunChk_CrrComChk( void )
{
	
	uchar	i,dsp=1;
	ushort	msg;
	short	tnoIndex[IFS_CRR_MAX]={0};
	char	index=0;
	ushort	CNTMAX;
	char	tmp[6]={0};
	
	for ( ; ; ) {

		if( dsp == 1 ){
			/* 初期画面表示 */
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[8] );		/* "＜ＣＲＲ折り返し工場テスト＞　" */
			grachr( 1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[6]);			/* "　CRR基板№　　 バージョン　　" */
			Fun_Dsp( FUNMSG[68] );						// "　▲　　▼　　　　実行　 終了 "

			for (i = 0,CNTMAX=0; i < IFS_CRR_MAX; i++) {
				if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){	// CRR基板の設定がされているかを確認
					grachr( (ushort)(CNTMAX + 2), 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"　№"
					opedsp( (ushort)(CNTMAX + 2), 4, (ushort)i+1, 2, 1, (index==CNTMAX?1:0) ,COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					tnoIndex[CNTMAX] = i+1;
					CNTMAX++;
				}
			}
			if( !CNTMAX ){								// 表示対象が１件も無ければ戻る
				BUZPIPI();
				return MOD_EXT;
			}
			dsp++;
		}
		
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// ポーリングタイマー再開
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// ポーリングタイマー再開
			return MOD_CHG;

		case KEY_TEN_F5:	/* "終了" */
			if( dsp == 3 ){	// 実行中は終了させない
				break;
			}
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// ポーリングタイマー再開
			if( dsp == 4 ){
				dsp = 1;
				break;
			}
			return MOD_EXT;

		case KEY_TEN_F1:	// F1:"▲"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			if( CNTMAX == 1 ){
				break;
			}
			if( index <= 0 ){
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転解除）
				index = (char)(CNTMAX - 1);
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転）
			}else{
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転解除）
				index--;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転）
			}
			break;

		case KEY_TEN_F2:	// F2:"▼"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			if( CNTMAX == 1 ){
				break;
			}
			if( index >= CNTMAX - 1 ){
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転解除）
				index = 0;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転）
			}else{
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転解除）
				index++;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転）
			}
			break;

		case KEY_TEN_F4:	// F4:"読出"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			for( i = 1; i < 8; i++ ){
				displclr( (ushort)i );		// 行をクリア
			}
			grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"　　 しばらくお待ち下さい 　　"
			Fun_Dsp(FUNMSG[0]);														/* [77]	"　　　　　　　　　　　　　　　" */
			
			queset( FLPTCBNO, LK_SND_P_CHK, sizeof(tnoIndex[0]), &tnoIndex[index] );
			// 車室設定をデフォルトから１０車室設定した場合、検査冶具がついていると、フラップの検査を始めるまでに時間がかかる（約１００秒）
			// 場合があるのでタイムアウトまでの時間を１２０秒（２分）とする。
			Lagtim( OPETCBNO, 6, 120*50 );							// ﾀｲﾏｰ6(120s)起動(ﾒﾝﾃﾅﾝｽ制御用)
			dsp++;
			break;

		case TIMEOUT6:
		case CTRL_PORT_CHK_RECV:	/* 制御ポート検査応答受信 */
			if( dsp < 3 ){												// ﾀｲﾏｰ6ｷｬﾝｾﾙ
				break;
			}
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[9] );						/* 装置No 結果    装置No 結果 */
			displclr( 3 );												// しばらくお待ち下さいを消去
			Fun_Dsp( FUNMSG[8] );										// "　　　　　　　　　　　　 終了 "
			
			for( i=0; i<10; i++ ){
				grachr( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?0:15), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &IFCSTR[2][2] );			// [02]	"№"
				opedsp( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?2:17), (ushort)(i+1), 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ﾀｰﾐﾅﾙ№表示（反転解除）
				switch( MntLockTest[i] ){
					case 0x00:
						sprintf( tmp, "%s    ", "－");
						break;
					case 0x01:
						sprintf( tmp, "%s    ", "○");
						break;
					default:
						sprintf( tmp, "%s(%2X)","×",MntLockTest[i]);
						break;
					
				}
				grachr( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?7:22), 6, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)tmp );		// 結果表示
			}
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// ポーリングタイマー再開
			dsp++;
			break;

		default:
			break;
		}
	}
}

// MH810100(S) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)
//[]----------------------------------------------------------------------[]
///	@brief		FROMのチェックサムを計算
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static ushort GetFromCheckSum(void)
{
	ushort nReturn = 0;
	ulong	offset = 0;
	ulong	index = 0;
	
	// 4KB毎にバッファに書き込む
	for(offset = 0; offset < FROM_PROG_SIZE; offset += FROM_IN_SECTOR_SIZE) {
		memcpy(ProgramRomBuffer, (void*)(FROM_IN_ADDRESS_START + offset), FROM_IN_SECTOR_SIZE);
		
		// sum値計算
		for(index = 0; index < FROM_IN_SECTOR_SIZE; ++index) {
			nReturn += ProgramRomBuffer[index];
		}

		taskchg( IDLETSKNO );
	}

	return nReturn;
}
// MH810100(E) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)

/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾞｰｼﾞｮﾝﾁｪｯｸ                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Version( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FunChk_Version( void )
{
	ushort	msg;
	uchar	f_DspFace;		// 0:バージョン画面表示中，1:時計ＣＰＵメモリダンプ表示中，2:バージョン表示へ
	unsigned short	usFncEvent;
// MH810100(S) Y.Yamauchi 2019/12/12 車番チケットレス(遠隔ダウンロード)	
	char	Lcd_ver[16];			// Lcdﾊﾞｰｼﾞｮﾝ（ｱﾌﾟﾘｹｰｼｮﾝ）
	char	sound_ver[16];			// 音声ﾊﾞｰｼﾞｮﾝ（ｱﾌﾟﾘｹｰｼｮﾝ）
// MH810100(S) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
	uchar keyIndex = 0;
// MH810100(E) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
// MH810100(S) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)
	char checksum[16];
	ushort nCheckSum;
// MH810100(E) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)

	memset(Lcd_ver,0,sizeof(Lcd_ver));
	memset(sound_ver,0,sizeof(sound_ver));
// MH810100(E) Y.Yamauchi 2019/12/12 車番チケットレス(遠隔ダウンロード)	
	for ( ; ; ) {
		f_DspFace = 0;

		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[15]);		/* "＜バージョンチェック＞　　　　" */
		Fun_Dsp(FUNMSG[0]);						/* "　　　　　　　 　　　　  　　 " */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[16]);		/* "メインプログラム：    　　　　" */
		grachr(2, 20,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, VERSNO.ver_part);	// ｿﾌﾄﾊﾞｰｼﾞｮﾝ表示

		Fun_Dsp(FUNMSG[8]);						/* "　　　　　　　 　　　　  終了 " */

// MH810100(S) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[51]);		// "フロントプログラム：    　　　　"
//
//		grachr(3, 20,10, 0, COLOR_BLACK, LCD_BLINK_OFF, RXF_VERSION );		// ソフトバージョン表示
//		
//// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(遠隔ダウンロード)	
////		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[53]);		// "音声データ　　　：    　　　　"
////		grachr(4, 20, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, SOUND_VERSION);		// ソフトバージョン表示
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[59]);		// 	"LCDモジュール    :　　　　　 "
//		lcdbm_get_config_prgm_ver( Lcd_ver, sizeof(Lcd_ver) );
//		grachr(4, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar *)Lcd_ver);		// 	"LCDモジュール    :　　　　　 "
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[60]);		// "コンテンツデータ  ：    　　　",
//		lcdbm_get_config_audio_ver( sound_ver, sizeof(sound_ver) );
//		grachr(5, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)sound_ver);		// ソフトバージョン表示
//// MH810100(E) Y.Yamauchi 20191212 車番チケットレス(遠隔ダウンロード)	
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[51]);		// "フロントプログラム：    　　　　"
		grachr(4, 20,10, 0, COLOR_BLACK, LCD_BLINK_OFF, RXF_VERSION );		// ソフトバージョン表示
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[59]);		// 	"LCDモジュール    :　　　　　 "
		lcdbm_get_config_prgm_ver( Lcd_ver, sizeof(Lcd_ver) );
		grachr(5, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar *)Lcd_ver);		// 	"LCDモジュール    :　　　　　 "
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[60]);		// "コンテンツデータ  ：    　　　",
		lcdbm_get_config_audio_ver( sound_ver, sizeof(sound_ver) );
		grachr(6, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)sound_ver);		// ソフトバージョン表示
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[62]);		// "チェックサム      ：          ",
		memset(checksum, 0, sizeof(checksum));
		nCheckSum = GetFromCheckSum();
		sprintf(checksum, "0x%04x",nCheckSum);
		grachr(3, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)checksum);		// チェックサム表示
// MH810100(E) S.Fujii 2020/07/15 車番チケットレス(チェックサム表示)
		do {
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:	/* "終了" */
				BUZPI();
				return MOD_EXT;
// MH810100(S) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
//			// 隠し機能として、時計CPUのEEPROM内容Dump機能を持つ
//			// LCD下のF4キーを押しながら、LCD下のF1キーを押した場合のみ起動する
//			case KEY_TEN_F1:
//				if( (TENKEY_F1 == 1) && (TENKEY_F4 == 1) && (f_DspFace == 0) ){	// 通常Ver表示からF4+F1押下
			// 隠し機能としてスタック使用量の印字機能を持つ
			// F4->F1->F1->F4の連続操作を行った場合のみ起動する
			case KEY_TEN_F1:
				if(keyIndex == 1 || keyIndex == 2) {
					++keyIndex;
				}
				else {
					keyIndex = 0;
				}
				break;
			case KEY_TEN_F4:
				if(keyIndex == 3) {
					keyIndex = 0;
// MH810100(E) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
					BUZPI();
					f_DspFace = 1;
					usFncEvent = FucChk_DebugMenu();
					switch(usFncEvent){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					case LCD_DISCONNECT:
						return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス（メンテナンス)
					case MOD_CHG:
						return( MOD_CHG );
					default:
						f_DspFace = 2;
						break;
					}
				}
// MH810100(S) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
				else if(keyIndex == 0) {
					++keyIndex;
				}
				else {
					keyIndex = 0;
				}
// MH810100(E) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
					break;
// MH810100(S) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN9:
			case KEY_TEN_CL:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
				keyIndex = 0;
				break;
// MH810100(E) S.Takahashi 2020/02/20 #3904 バージョンチェックのデバッグメニューを表示できない
			case HIF_RECV_VER:
				/*
				 *	サブＣＰＵのバージョン取得イベントより先にタイマ６のタイムアウトイベントが入ってきた場合
				 *	バージョン情報が空白のままとなるため、サブＣＰＵのバージョン取得イベントをキャッチした場合は
				 *	ここでバージョン情報の（再）表示を行う。
				 */
				FunChk_Draw_SubCPU_Version();
				break;
			default:
				break;
			}
		} while( f_DspFace != 2 );
	 }
}
/**
 *	サブＣＰＵのバージョンを描画する
 *
 *	@param[in]	version_string	バージョン文字列へのポインタ
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2008/09/20(土)
 */
static void FunChk_Draw_SubCPU_Version(void)
{
	unsigned short	length;
	char	sub_ver[16];

	memset(sub_ver, 0, sizeof(sub_ver));

	displclr(4);
	length = (ushort)strlen(sub_ver);
	if ( 0 < length ) {
		// 文字列長が１以上ならバージョンを表示する
		grachr(3, 18, length, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)sub_ver);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suicaﾁｪｯｸ                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_Suica( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FncChk_Suica( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

	/* 設定ﾊﾟﾗﾒｰﾀ-NTNET接続参照 */
// MH810103 GG119202(S) 設定参照変更
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) {
	if (! isSX10_USE()) {
// MH810103 GG119202(E) 設定参照変更
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[3]);			/* "＜交通系ICリーダーチェック＞　" */
		msg = Menu_Slt(SUICA_MENU, FSUICA_CHK_TBL, (char)FSUICA_CHK_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case MOD_CHG:
			return(MOD_CHG);
			break;
		case MOD_EXT:		/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
			break;
		case FISMF1_CHK:	/* 通信テスト 				*/
			ret = Com_Chk_Suica();
			break;
		case FISMF2_CHK:	/* 通信ログプリント 		*/
			ret = Log_Print_Suica_menu();
			break;
		case FISMF3_CHK:	/* 通信ログプリント（異常） */
			ret = Log_Print_Suica(1);
			break;
		case MNT_SCA_MEISAI:					// Ｓｕｉｃａ利用明細
			ret = UsMnt_DiditalCasheUseLog(MNT_SCA_MEISAI);
			break;
		case MNT_SCA_SHUUKEI:					// Ｓｕｉｃａ集計
			ret = UsMnt_DiditalCasheSyuukei(MNT_SCA_SHUUKEI);
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if ( ret == LCD_DISCONNECT ) {
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica通信チェック処理  		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Com_Chk_Suica( void )         　                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Date         : 2007-02-26                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort Com_Chk_Suica( void )
{
	ushort	msg;

// MH321801(S) 通信テストで「正常」と表示される
	// 起動シーケンス完了前は通信テスト不可とする
	if (isEC_USE() &&
		Suica_Rec.Data.BIT.INITIALIZE == 0) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH321801(E) 通信テストで「正常」と表示される

	dispclr();
// MH321800(S) D.Inaba ICクレジット対応（表示文字修正）
	if( isEC_USE() ){
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[17]);		/* "＜通信テスト＞                " */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[4]);		/* "　通信状態　　   ：           " */
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[18]);		/* "　端末状態　　   ：           " */
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[6]);		/* "　バージョン     ：           " */
	}
	else {
// MH321800(E) D.Inaba ICクレジット対応（表示文字修正）
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[3]);			/* "＜ＳＵＩＣＡチェック＞　　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[4]);			/* "　通信状態　　   ：           " */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[5]);			/* "　端末状態　　   ：           " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[6]);			/* "　バージョン     ：           " */
// MH321800(S) D.Inaba ICクレジット対応（表示文字修正）
	}
// MH321800(E) D.Inaba ICクレジット対応（表示文字修正）


	if ( Suica_Rec.suica_err_event.BIT.COMFAIL ) {
		grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[2]);		/* "異常" */
		grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3]);		/* "－－" */

	}else{
		grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[1]);		/* "正常" */
		
		if ( Suica_Rec.suica_err_event.BIT.ERR_RECEIVE )
			grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[2]);	/* "異常" */
		else
			grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[1]);	/* "正常" */
		
	}

	if( !Suica_Rec.suica_err_event.BIT.COMFAIL && !Suica_Rec.suica_err_event.BIT.ERR_RECEIVE ){
		opedsp5(4, 20, (ushort)fix_data[0], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* "　　　　　　　　　　　ｘｘ　　" */
		Fun_Dsp(FUNMSG[8]);															/* "　　　　　　　　　 　　  終了 " */
	}

	else {
		grachr(4, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3]);		/* "－－" */
		Fun_Dsp(FUNMSG[8]);								/* "　　　　　　　　　 　　  終了 " */
	}

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
			case KEY_MODECHG:	/* モードチェンジ */
				return MOD_CHG;
				break;

			case KEY_TEN_F5:	/* 受信タイムアウト(通常ありえない) */
				BUZPI();
				return MOD_EXT;
				break;

			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suicaログプリント処理  		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_Print_Suica( void )       　                        |*/
/*| PARAMETER    : uchar print_kind 0:通常LOG 1:異常LOG                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Date         : 2007-02-26                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort Log_Print_Suica( uchar print_kind )
{
	ushort	msg;
	T_FrmLogPriReq1	SuicaLogPriReq;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	SuicaPrnStop;		// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			pri_cmd = 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	ushort			log_count = 0;
	uchar			status=0;


	dispclr();
	if( print_kind )
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[14]);		/* "＜通信ログプリント（異常）＞　" */
	else		
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* "＜通信ログプリント＞　　　　　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[9]);			/* " 通信ログが        件あります " */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[10]);			/* "　　　プリントしますか？　　　" */

	log_count = Log_Count_search( print_kind );	// ﾛｸﾞから実際の登録件数を検索してｾｯﾄ

	opedsp(3, 11, log_count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// 件数表示

	Fun_Dsp(FUNMSG2[38]);										// ﾌｧﾝｸｼｮﾝｷｰ表示

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)				
				case KEY_MODECHG:								/* モードチェンジ */
					return MOD_CHG;
					break;

				case KEY_TEN_F1:								

					if( !status ){
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[11]);		/* "　　　クリアしますか？　　　" */
						Fun_Dsp(FUNMSG[19]);					// ﾌｧﾝｸｼｮﾝｷｰ表示
						status = 1;								// 画面ｽﾃｰﾀｽの変更
						BUZPI();
					}
					break;

				case KEY_TEN_F3:
// MH321800(S) D.Inaba ICクレジット対応（レシート印字不可でログクリアができない修正）
					if (status != 0) {
					// 画面がｸﾘｱ画面の場合
						BUZPI();
						if (print_kind != 0) {													// 異常LOG印字の場合
							memset(&SUICA_LOG_REC_FOR_ERR, 0, sizeof(struct suica_log_rec));	// 異常LOGﾃﾞｰﾀの初期化
						} else {
							memset(&SUICA_LOG_REC, 0, sizeof(struct suica_log_rec));			// ﾃﾞｰﾀの初期化
						}
						return MOD_EXT;
					}
// MH321800(E) D.Inaba ICクレジット対応（レシート印字不可でログクリアができない修正）
					if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					if( !status ){																// 画面がﾌﾟﾘﾝﾄ画面の場合
						SuicaLogPriReq.prn_kind = R_PRI;										// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
						SuicaLogPriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// 機械№	：設定ﾃﾞｰﾀ
						SuicaLogPriReq.Kakari_no = OPECTL.Kakari_Num;							// 係員No.
						memcpy( &SuicaLogPriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// 印字時刻	：現在時刻
						if( print_kind ){														// 異常印字処理の場合
							// 異常印字要求かどうかの判断をプリンタタスクで行う為に、機械Noの先頭Bitを使用して判断する。
							// 現状、機械NoはNo.1～20までしか設定しない為、先頭Bitは使用しても問題ないと判断する
							SuicaLogPriReq.Kikai_no |= 0x8000;									// 機械No格納ｴﾘｱの先頭bitを使用してﾌﾗｸﾞを立てる
						}
						queset( PRNTCBNO, PREQ_SUICA_LOG, sizeof(T_FrmLogPriReq1), &SuicaLogPriReq );
						Ope_DisableDoorKnobChime();
						pri_cmd = PREQ_SUICA_LOG;												// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ

						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[12]);										/* "　　　プリント中 　　　" */
						Fun_Dsp( FUNMSG[82] );													// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"
// MH321800(S) D.Inaba ICクレジット対応（レシート印字不可でログクリアができない修正）					
// 					}else{																		// 画面がｸﾘｱ画面の場合
// 						if( print_kind ){														// 異常LOG印字の場合
// 							memset( &SUICA_LOG_REC_FOR_ERR,0,sizeof( struct	suica_log_rec ));	// 異常LOGﾃﾞｰﾀの初期化
// 						}else
// 						memset( &SUICA_LOG_REC,0,sizeof( struct	suica_log_rec ));				// ﾃﾞｰﾀの初期化
// 						return MOD_EXT;
// MH321800(E) D.Inaba ICクレジット対応（レシート印字不可でログクリアができない修正）					
					}
					break;

				case KEY_TEN_F4:	
					BUZPI();
					if( status ){
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[10]);										/* "　　　プリントしますか？　　　" */
						Fun_Dsp(FUNMSG2[38]);													// ﾌｧﾝｸｼｮﾝｷｰ表示："
						status = 0;																// 画面ｽﾃｰﾀｽの変更
					}else return MOD_EXT;

					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){		// 印字終了ﾒｯｾｰｼﾞ受信？
				msg = MOD_EXT;								// YES：前画面に戻る
			}

			switch( msg ){									// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();

					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					SuicaPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &SuicaPrnStop );

					return MOD_EXT;

					break;

				default:
					break;
			}

		}
		if( (msg == MOD_EXT) || (msg == MOD_CHG) ){
			break;											// 前画面に戻る
		}		
	}
	return MOD_EXT;
}
//[]----------------------------------------------------------------------[]
///	@brief			Suicaログプリントメニュー処理 
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort Log_Print_Suica_menu( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* ＜通信ログプリント＞ */
		msg = Menu_Slt(SUICA_MENU2, FSUICA_CHK_TBL2, (char)FSUICA_CHK_MAX2, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)		
		case MOD_CHG:
			return(MOD_CHG);
			break;
		case MOD_EXT:		/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
			break;
		case MNT_SUICALOG_INJI:		// 通信ログプリント（全ログ）
			ret = Log_Print_Suica(0);
			break;
		case MNT_SUICALOG_INJI2:	// 通信ログプリント（直近ログ）
			ret = Log_Print_Suica2();
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		if (ret == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}
//[]----------------------------------------------------------------------[]
///	@brief			Suicaログプリント処理(直近ログ)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort Log_Print_Suica2( void )
{


	ushort	msg;
	T_FrmLogPriReq1	SuicaLogPriReq;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	SuicaPrnStop;		// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			pri_cmd = 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* ＜通信ログプリント＞ */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[15]);			/* " 直近ログをプリントしますか？ " */

	Fun_Dsp(FUNMSG[19]);										// ﾌｧﾝｸｼｮﾝｷｰ表示 "　　　　　　 はい いいえ　　　"

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:								/* モードチェンジ */
					return MOD_CHG;
					break;

				case KEY_TEN_F3:	
					if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
						BUZPIPI();
						break;
					}
					BUZPI();
					SuicaLogPriReq.prn_kind = R_PRI;										// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					SuicaLogPriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// 機械№	：設定ﾃﾞｰﾀ
					SuicaLogPriReq.Kakari_no = OPECTL.Kakari_Num;							// 係員No.
					memcpy( &SuicaLogPriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// 印字時刻	：現在時刻
					queset( PRNTCBNO, PREQ_SUICA_LOG2, sizeof(T_FrmLogPriReq1), &SuicaLogPriReq );
					Ope_DisableDoorKnobChime();
					pri_cmd = PREQ_SUICA_LOG2;												// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[12]);										/* "　　　プリント中 　　　" */
					Fun_Dsp( FUNMSG[82] );													// ﾌｧﾝｸｼｮﾝｷｰ表示："　　　　　　 中止 　　　　　　"
					break;

				case KEY_TEN_F4:	
					BUZPI();
					return MOD_EXT;
					break;

				default:
					break;
			}
		}
		else{
			// 印字要求後（印字終了待ち画面）

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){		// 印字終了ﾒｯｾｰｼﾞ受信？
				msg = MOD_EXT;								// YES：前画面に戻る
			}

			switch( msg ){									// イベント？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:						//切断通知
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:							// 設定キー切替
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					// ドアノブの状態にかかわらずトグル動作してしまうので、
					// ドアノブ閉かどうかのチェックを実施
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					}
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
					break;

				case KEY_TEN_F3:							// Ｆ３（中止）キー押下

					BUZPI();

					/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
					SuicaPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &SuicaPrnStop );

					return MOD_EXT;

					break;

				default:
					break;
			}

		}
		if( (msg == MOD_EXT) || (msg == MOD_CHG) ){
			break;											// 前画面に戻る
		}		
	}
	return MOD_EXT;


}
/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ装置ﾁｪｯｸ                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Flp( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Flp( void )
{
	short	msg = -1;
	short	tno[2];
	ushort	top;
	char	pgchg = 0;
	char	i;
	char	timeout = 1;
	char	data[8];
	char	rcvSts;
	memset(data,0,sizeof(data));
	rcvSts = 0;

	if( !(GetCarInfoParam() & 0x02) ){
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ﾀｲﾏｰ6(12s)起動(ﾒﾝﾃﾅﾝｽ制御用)

	// まず保持しているすべてのﾊﾞｰｼﾞｮﾝ情報をｸﾘｱする
	i = _MTYPE_FLAP;
	queset( FLPTCBNO, LK_SND_VER, 1, &i );	// ﾛｯｸ装置ﾊﾞｰｼﾞｮﾝ要求送信

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[44]);			// [44]	"＜フラップIF盤チェック＞　　　"
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"　　 しばらくお待ち下さい 　　"
	
	top = 0;
	tno[0] = tno[1] = 0;						// ﾀｰﾐﾅﾙ№ｸﾘｱ

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:						// 切断通知
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"終了"
				BUZPI();
				Lagcan( OPETCBNO, 6 );				// ﾀｲﾏｰ6ﾘｾｯﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1:"▲"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] <= 0 ){
					tno[0] = TERM_NO_MAX - 1;
				}else{
					tno[0]--;
				}
				pgchg = pag_ctl( TERM_NO_MAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(tno[1] + 1), 2, 1, 0, 
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					if( top + tno[0] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(tno[0] + 1), 2, 1, 1, 
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"▼"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] >= TERM_NO_MAX - 1 ){
					tno[0] = 0;
				}else{
					tno[0]++;
				}
				pgchg = pag_ctl( TERM_NO_MAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(tno[1] + 1), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					if( top + tno[0] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(tno[0] + 1), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}
				}
				break;

			case TIMEOUT6:		// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ(ﾒﾝﾃﾅﾝｽ制御用)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[4] );		// [04]	"　ﾀｰﾐﾅﾙ№ 　　　バージョン　　"
				Fun_Dsp( FUNMSG[06] );					// [25]	"　▲　　▼　　　　 読出  終了 "
				pgchg = 1;
				timeout = 0;
				break;

			default:
				break;
		}
		if( pgchg ){
			for( i = 0; i < 5; i++ ){
				if( top + i < TERM_NO_MAX ){
					grachr( (ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"　№　　　－　　　　　　　　　"
					if( i == tno[0] % 5 ){
						opedsp( (ushort)(i + 2), 4, (ushort)(top + i + 1), 2, 1, 1, 
																	COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}else{
						opedsp( (ushort)(i + 2), 4, (ushort)(top + i + 1), 2, 1, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示
					}
					if (rcvSts == 0) {
						// 接続ﾀｰﾐﾅﾙなし
						memcpy( wlcd_buf, &IFCSTR[5][12], 32 );		// [ｎｏ ｓｌａｖｅ]表示用
					} else if (rcvSts == 2) {
						// ROMﾊﾞｰｼﾞｮﾝ未受信
						memcpy( wlcd_buf, &IFCSTR[3][14], 32 );		// [未受信]表示用
					} else {
						as1chg( (uchar*)data, wlcd_buf, 8 );	// 受信したﾊﾞｰｼﾞｮﾝ(ｱｽｷｰ)を表示文字ｺｰﾄﾞへ変換する
					}
					grachr( (ushort)(i + 2), 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// ﾊﾞｰｼﾞｮﾝ表示
				}else{
					displclr( (ushort)(i + 2) );						// 未使用の行はｸﾘｱ
				}
				pgchg = 0;
			}
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_tst_rct                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : レシートプリンタ印字テスト                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	FPrnChk_tst_rct( void ){

	unsigned short	usFprnRctEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );		/* // "＜レシートテスト印刷＞　　　　" */

		usFprnRctEvent = Menu_Slt( FPRNMENU3, FPRN_CHK_TBL4, (char)FPRN_CHK_MAX3, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnRctEvent ){
			case FPRN1_CHK:	// ﾚｼｰﾄ印字ﾃｽﾄ
				usFprnRctEvent = FPrnChk_tst( usFprnRctEvent );
				break;
			case FPRN8_CHK:	// ﾚｼｰﾄ:領収証印字ﾃｽﾄ
				usFprnRctEvent = FPrnChk_ryo_tst( );
				break;
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnRctEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFprnRctEvent == MOD_CHG ){
		if( usFprnRctEvent == MOD_CHG || usFprnRctEvent == MOD_CUT ){		// モードチェンジもしくは切断通知
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnRctEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_ryo_tst                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌﾟﾘﾝﾀ領収証ﾃｽﾄ印字処理                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#define		TSTRYOPRI_END	(PREQ_RYOUSYUU | INNJI_ENDMASK)

unsigned short	FPrnChk_ryo_tst( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	T_FrmReceipt	FrmPrintRctTest;	// 印字要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	unsigned short	ret = 0;			// 戻り値
	unsigned char	priend;				// 印字終了ﾌﾗｸﾞ				※OFF:印字中、ON:印字終了
	unsigned char	dsptimer;			// 印字中画面表示ﾀｲﾏｰﾌﾗｸﾞ	※OFF:ﾀｲﾑｱｳﾄ済み	ON:起動中

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );				// "＜レシートテスト印刷＞　　　　" 
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[16] );				// "【領収証印刷】　　　　　　　　"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );				// "　印刷中です　　　　　　　　　" 
	Fun_Dsp(FUNMSG[77]);														/* [77]	"　　　　　　　　　　　　　　　" */

	memset(&TestData,0,sizeof(TestData));
	FPrnChk_Testdata_mk( 0,&TestData );												// 擬似精算完了データセット
// GG129002(S) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
	memset(&FrmPrintRctTest, 0, sizeof(FrmPrintRctTest));
// GG129002(E) ゲート式車番チケットレスシステム対応（新プリンタ印字処理不具合修正）
	FrmPrintRctTest.prn_kind = R_PRI;												// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	FrmPrintRctTest.reprint = OFF;													// 再発行ﾌﾗｸﾞ
	FrmPrintRctTest.prn_data = &TestData;
	memcpy( &FrmPrintRctTest.PriTime, &CLK_REC, sizeof(date_time_rec) );			// 再発行日時をテスト印字日付として使う

	queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof( T_FrmReceipt ), &FrmPrintRctTest ); 	// 領収証印字要求
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 3*50 );					// 印字中画面表示ﾀｲﾏｰ6(3s)起動（ﾃｽﾄ印字中の画面表示を３秒間保持する）
	dsptimer	= ON;								// 印字中画面表示ﾀｲﾏｰﾌﾗｸﾞｾｯﾄ
	priend		= OFF;								// 印字終了ﾌﾗｸﾞﾘｾｯﾄ

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信
		switch( msg){			// 受信ﾒｯｾｰｼﾞ？
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:	// 切断通知
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	// 操作ﾓｰﾄﾞ切替
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				// ドアノブの状態にかかわらずトグル動作してしまうので、
				// ドアノブ閉かどうかのチェックを実施
				if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				BUZPI();
				ret = MOD_CHG;
// MH810100(S) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				}
// MH810100(E) S.Nishimoto 2020/09/01 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
				break;
			case TSTRYOPRI_END:	// 領収証印字終了
				if( dsptimer == OFF ){					// 印字中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}
				else{
					priend = ON;						// NO　→印字終了ﾌﾗｸﾞｾｯﾄ
				}
				break;
			case TIMEOUT6:		// 印字中画面終了ﾀｲﾏｰﾀｲﾑｱｳﾄ
				dsptimer = OFF;							// 印字中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ
				if( priend == ON ){						// 印字終了済み？
					ret = MOD_EXT;						// YES→前画面に戻る
				}
				break;
			default:			// その他
				break;
		}
		if( ret != 0 ){									// 印字中画面終了？
			if( dsptimer == ON ){						// 印字中画面表示ﾀｲﾏｰ起動中？
				Lagcan( OPETCBNO, 6 );					// 印字中画面表示ﾀｲﾏｰ解除
			}
			break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_Testdata_mk                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌﾟﾘﾝﾀ印字ﾃｽﾄ用擬似精算完了データ作成                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : char sw  0=領収証                                       |*/
/*|                Receipt_data *                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void FPrnChk_Testdata_mk( char sw, Receipt_data *dat )
{
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	int i, cnt;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)

	//擬似精算完了データ作成
	dat->Kikai_no		= 1;					// 機械Ｎｏ（自番号0,1-20）
	dat->Oiban.i		= 1L;					// 追番
	dat->Oiban.w		= 1L;					// 追番
	dat->WPlace			= 1L;					// 車室番号
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	dat->CarSearchFlg	= 0;					// 0=車番検索
	memset( dat->CarNumber, 0, sizeof(dat->CarNumber) );
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:552)対応
//	strcpy( (char*)dat->CarNumber, "1234" );
	strncpy( (char*)dat->CarNumber, "1234", sizeof(dat->CarNumber) );
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:552)対応
	memset( dat->CarDataID, 0, sizeof(dat->CarDataID) );
	for( i=0, cnt=0; i<sizeof(dat->CarDataID); i++ ){
		dat->CarDataID[i] = (0x30 + cnt);
		if( ++cnt > 9 ){
			cnt = 0;
		}
	}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	dat->TInTime.Year	= 2000;					// 入庫日時	（年）
	dat->TInTime.Mon	= 1;					// 			（月）
	dat->TInTime.Day	= 1;					// 			（日）
	dat->TInTime.Hour	= 0;					// 			（時）
	dat->TInTime.Min	= 0;					// 			（分）
	dat->TOutTime.Year	= 2000;					// 出庫日時	（年）
	dat->TOutTime.Mon	= 1;					// 			（月）
	dat->TOutTime.Day	= 1;					// 			（日）
	dat->TOutTime.Hour	= 1;					// 			（時）
	dat->TOutTime.Min	= 0;					// 			（分）
	dat->WPrice			= 500L;					// 駐車料金
	dat->syu			= 1;					// 料金種別（１～１２：Ａ～Ｌ）
	dat->testflag		= ON;					// テスト印字フラグ
	dat->WInPrice		= 1000L;				// 入金額
	dat->Seisan_kind	= 0;					// 精算種別（0=自動、1=半自動、2=手動、3=紛失)
	dat->WTotalPrice	= 500L;					// 領収金額
	dat->Wtax			= 0L;					// 税金
	dat->WChgPrice		= 500L;					// 領収証の場合		：釣銭金額
// GG129001(S) インボイス対応（追加修正）
	dat->WTaxRate		= Disp_Tax_Value( (date_time_rec*)&CLK_REC );	// 適用税率（現在時刻）
// GG129001(E) インボイス対応（追加修正）
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	dat->WBillAmount = Billingcalculation(dat);		// 請求額
	dat->WTaxPrice = TaxAmountcalculation(dat, 0);	// 課税対象額
// GG129001(E) データ保管サービス対応（課税対象額をセットする）

}

enum {
	CCOMCHK_TYPE_CLR = 0,
	CCOMCHK_TYPE_SND,
};

#define	_getid(t)	((t == CCOMCHK_TYPE_CLR)? 62 : 61)

void IBKCtrl_SetRcvData_manu(uchar id, uchar rslt)
{
	switch (id) {

	case 61:
	case 62:
		if (rslt) 	
			queset( OPETCBNO, IBK_CTRL_NG, 0, NULL );
		else
			queset( OPETCBNO, IBK_CTRL_OK, 0, NULL );
		break;

	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CCom( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_CCom( void )
{
	ushort	fncChkCcomEvent;
	ushort	ret;
	char	wk[2];

	// センター通信有効？
	if (!_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {		//	Ntnet_Remote_Comm
		BUZPIPI();
		return MOD_EXT;
	}

	if(_is_ntnet_remote()) {
	if( (prm_get( COM_PRM, S_CEN, 65, 3, 4 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 65, 3, 1 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 66, 3, 4 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 66, 3, 1 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 51, 1, 3 ) == 0) ) {
		BUZPIPI();
		return MOD_EXT;
	}
	}

	// 自動送信一時停止
	ntautoStopReq();
	
	DP_CP[0] = DP_CP[1] = 0;
	fncChkCcomEvent = 0;
	ret             = 0;

	while ((fncChkCcomEvent != MOD_EXT) && (ret != MOD_CHG)) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[45] );		/* "＜センター通信チェック＞" */
		fncChkCcomEvent = Menu_Slt( CCOMMENU, CCOM_CHK_TBL, CCOM_CHK_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( fncChkCcomEvent ){

		case CCOM_DTCLR:	// 未送信データクリア
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_bufctrl(CCOMCHK_TYPE_CLR);
			break;

		case CCOM_FLSH:		// 未送信データ送信
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_bufctrl(CCOMCHK_TYPE_SND);
			break;

//未使用です。（S）			
		case CCOM_TEST:		// ＦＯＭＡ通信テスト
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 &&				// LAN以外
				prm_get( COM_PRM,S_NTN,121,1,1) != 0) {				// 駐車場センター形式
				ret = FncChk_ccomtst();								// 通信テスト
			}
			else {													// LANの場合は通信テスト不可
				BUZPIPI();
			}
			break;
		case CCOM_APN:		// 接続先ＡＰＮ確認
			if(prm_get( COM_PRM,S_CEN,51,1,3 ) == 1){
				BUZPIPI();
				break;
			}
			if( (prm_get( COM_PRM,S_CEN,78,1,1 ) == 1) ||
				(prm_get( COM_PRM,S_CEN,78,1,1 ) == 2) ) {			/* FOMA対応?(Y)				*/
				ret = FncChk_CcomApnDisp();
			}else{
				BUZPIPI();
			}
			break;
		case CCOM_ATN:		// 電波受信状態確認
			if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1) {				// LAN以外
				ret = FncChk_CcomAntDisp();
			}else{
				BUZPIPI();
			}
			break;
//未使用です。（E）			

		case CCOM_OIBANCLR:	// センター追い番クリア
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				ret = FncChk_cOibanClr();
			}else{
				BUZPIPI();
			}
			break;
		case CCOM_REMOTE_DL:
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_CComRemoteDLChk();
			break;
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
		case CCOM_LONG_PARK_ALLREL:
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if(prm_get(COM_PRM, S_TYP, 135, 1, 5) != 2){	// 長期駐車検出(長期駐車データによる通知)以外
				BUZPIPI();
				break;
			}
			ret = FncChk_CComLongParkAllRel();
			break;
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

		case MOD_EXT:
			break;

		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( fncChkCcomEvent == MOD_CHG ){
		if( fncChkCcomEvent == MOD_CHG || fncChkCcomEvent == MOD_CUT ){		//  モードチェンジもしくは切断通知
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if ( fncChkCcomEvent == MOD_CHG ) {
				ret = MOD_CHG;
			} else {
				ret = MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	
	// 自動送信再開
	ntautoStartReq();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　バッファ制御                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_bufctrl( int type )                              |*/
/*| PARAMETER    : type : 0: クリア                                        |*/
/*|                       1: 送信                                          |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
static const ulong	req_flg[CCOM_DATA_MAX] = {
// MH810100(S) Y.Yamauchi 20191028 車番チケットレス(メンテナンス)
//	NTNET_BUFCTRL_REQ_INCAR,			// 入庫
// MH810100(E) Y.Yamauchi 20191028 車番チケットレス(メンテナンス)
	NTNET_BUFCTRL_REQ_TTOTAL,			// 集計
	NTNET_BUFCTRL_REQ_SALE,				// 精算
	NTNET_BUFCTRL_REQ_COIN,				// コ金庫計
	NTNET_BUFCTRL_REQ_ERROR,			// ｴﾗｰ
	NTNET_BUFCTRL_REQ_NOTE,				// 紙金庫計
	NTNET_BUFCTRL_REQ_ALARM,			// ｱﾗｰﾑ
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	NTNET_BUFCTRL_REQ_CAR_COUNT,		// 駐車台数
//	NTNET_BUFCTRL_REQ_MONITOR,			// ﾓﾆﾀｰ
//	NTNET_BUFCTRL_REQ_MONEY, 			// 金銭管理
//	NTNET_BUFCTRL_REQ_OPE_MONITOR,		// 操作
//	NTNET_BUFCTRL_REQ_TURI, 			// 釣銭管理
//	NTNET_BUFCTRL_REQ_RMON,				// 遠隔監視
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	NTNET_BUFCTRL_REQ_LONG_PARK,		// 長期駐車
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	NTNET_BUFCTRL_REQ_MONEY, 			// 金銭管理
	NTNET_BUFCTRL_REQ_MONITOR,			// ﾓﾆﾀｰ
	NTNET_BUFCTRL_REQ_TURI, 			// 釣銭管理
	NTNET_BUFCTRL_REQ_OPE_MONITOR,		// 操作
	NTNET_BUFCTRL_REQ_RMON,				// 遠隔監視
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
};
static const ulong	MntTransData[CCOM_DATA_MAX][2] = {
// MH810100(S) Y.Yamauchi 20191028 車番チケットレス(メンテナンス)
//	{61, 6},		// 入庫
// MH810100(E) Y.Yamauchi 20191028 車番チケットレス(メンテナンス)
	{61, 4},		// 集計
	{61, 5},		// 精算
	{62, 5},		// ｺｲﾝ金庫集計	(ramのみ)
	{61, 3},		// ｴﾗｰ
	{62, 4},		// 紙幣金庫集計	(ramのみ)
	{61, 2},		// ｱﾗｰﾑ
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	{62, 3},		// 駐車台数ﾃﾞｰﾀ
//	{61, 1},		// ﾓﾆﾀ
//	{62, 1},		// 金銭管理
//	{62, 6},		// 操作ﾓﾆﾀ
//	{62, 2},		// 釣銭管理
//	{61, 3},		// 遠隔監視(参照するパラメータがないのでエラーデータのパラメータと同じ動作とする)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	{61, 3},		// 長期駐車(参照するパラメータがないのでエラーデータのパラメータを仮の参照位置とする)
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	{62, 1},		// 金銭管理
	{61, 1},		// ﾓﾆﾀ
	{62, 2},		// 釣銭管理
	{62, 6},		// 操作ﾓﾆﾀ
	{61, 3},		// 遠隔監視(参照するパラメータがないのでエラーデータのパラメータと同じ動作とする)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
};

ushort FncChk_bufctrl( int type )
{
	ushort	msg;
	uchar	mode;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
	uchar	dsp;		// 0:描画なし/ 1:件数更新/ 2:画面更新/ 3:件数表示
	int		item_pos;	// カーソル位置
	ushort	pos;
	uchar	countreq = 0;
	uchar	ibkdown = 0;
	uchar	fkey_enb = 0;
	uchar	page = 0;
	uchar	save_page;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[type] );	// "＜センター未送信データクリア＞"
																		// "＜センター未送信データ送信＞　"
	dsp      = 2;
	mode     = 0;
	item_pos = 0;

	for ( ; ; ) {
		if (dsp) {
		// 件数表示更新要求あり
			if (mode == 0) {
				if (dsp == 3) {
				// 件数表示
					dsp_unflushed_count(ibkdown, page);		// 件数を表示
					dsp_ccomchk_cursor(item_pos, 1, page);	// 反転表示
					Fun_Dsp(FUNMSG[(92+type)]);			// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
														// " 送信 全送信　⊃　 　　  終了 "
					fkey_enb = 1;						// 以後F1&F2許可
					Lagtim( OPETCBNO, 6, 2*50 );		// 画面更新ﾀｲﾏｰ6(2s)起動
				}
				else {
				// 件数要求
					if (dsp == 2) {
						clr_dsp_count(page);			// 表示クリア
						fkey_enb = 0;
					}
					Lagtim( OPETCBNO, 2, 200/20 );		// 件数表示ﾀｲﾏｰ2(200ms)起動 = TIMEOUT2
					countreq = 1;						// 件数要求中
				}
			}
			dsp = 0;
		}
		
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			Lagcan(OPETCBNO, 6);
			Lagcan(OPETCBNO, 7);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)	
		case KEY_MODECHG:
			Lagcan(OPETCBNO, 6);
			Lagcan(OPETCBNO, 7);
			return MOD_CHG;

		case KEY_TEN_F5:			// 「終了」
			if (mode == 0) {		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_EXT;
			}
			else if (mode == 3) {	// エラー画面
				BUZPI();
				dsp  = 2;
				mode = 0;
			}
			break;

		case KEY_TEN_F1:		// クリア/送信
			if (mode == 0) {
				if (fkey_enb == 0){
					break;
				}
				BUZPI();
				
				dispmlclr(1, 6);
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//				pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
				pos = ((item_pos % 2) || (item_pos == CCOM_DATA_TOTAL) || (item_pos == CCOM_DATA_RMON)) ? 4 : 8;
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[item_pos] );				// "選択項目"
				grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[type] );		// "データを消去します"
																										//          送信
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
				Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
				mode = 1;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			}
			break;
		case KEY_TEN_F2:		// 全クリア/全送信
			if (mode == 0) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				if (fkey_enb == 0){
					break;
				}
				BUZPI();
				
				dispmlclr(1, 6);
				grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );		// "全"
				grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[type] );				// "データを消去します"
																								//          送信
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "　　　　よろしいですか？　　　"
				Fun_Dsp(FUNMSG[19]);								// "　　　　　　 はい いいえ　　　"
				mode = 2;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			}
			break;

		case KEY_TEN_F3:
			if (ibkdown) {
			// IBKダウン中ならばIBKに遠隔指示を出さない
				if (mode == 1 || mode == 2) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				// 未送信データクリア要求→IBK_CTRL_OK処理へ（本体データをクリア）
				// 未送信データ送信要求　→IBK_CTRL_NG処理へ（エラーとする）
					BUZPI();
					queset(OPETCBNO, (type == CCOMCHK_TYPE_CLR)? IBK_CTRL_OK : IBK_CTRL_NG, 0, NULL);
					break;
				}
			}
			switch (mode) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			case 0:			// カーソル移動
				BUZPI();
				dsp_ccomchk_cursor(item_pos, 0, page);	// 正転表示
				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//					if (--item_pos < CCOM_DATA_INCAR) {
					if (--item_pos < CCOM_DATA_TOTAL) {
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
						item_pos = CCOM_DATA_MAX - 1;
					}
				}
				else {
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//					if (--item_pos < CCOM_DATA_INCAR) {
					if (--item_pos < CCOM_DATA_TOTAL) {
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
						item_pos = CCOM_EXIST_DATA_MAX - 1;
					}
				}
				save_page = page;
				page = dsp_ccomchk_cursor(item_pos, 1, save_page);	// 反転表示
				if (page != save_page) {
					fkey_enb = 0;
					Lagtim( OPETCBNO, 2, 200/20 );		// 件数表示ﾀｲﾏｰ2(200ms)起動 = TIMEOUT2
					countreq = 1;						// 件数要求中
				}
				break;
			case 1:			// クリア・送信実行
				BUZPI();
				NTNET_Snd_DataTblCtrl(_getid(type), req_flg[item_pos], 0);
				Lagtim( OPETCBNO, 7, 5*50 );			// ﾀｲﾑｱｳﾄ用ﾀｲﾏｰ7(5s)起動
				break;
			case 2:			// 全クリア・全送信実行
				BUZPI();
 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					NTNET_Snd_DataTblCtrl(_getid(type), NTNET_BUFCTRL_REQ_ALL, 0);
				}
				else {
					NTNET_Snd_DataTblCtrl(_getid(type), NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
				}
				Lagtim( OPETCBNO, 7, 5*50 );			// ﾀｲﾑｱｳﾄ用ﾀｲﾏｰ7(5s)起動
				break;
			case 3:
			default:
				break;
			}
			break;

		case KEY_TEN_F4:						// 「いいえ」
			if ((mode == 1) || (mode == 2)) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				BUZPI();
				dsp  = 2;	// 画面再描画
				mode = 0;
			}
			else if (mode == 0) {
				BUZPI();
				dsp_ccomchk_cursor(item_pos, 0, page);	// 正転表示
				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					if (++item_pos >= CCOM_DATA_MAX) {
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//						item_pos = CCOM_DATA_INCAR;
						item_pos = CCOM_DATA_TOTAL;
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
					}
				}
				else {
					if (++item_pos >= CCOM_EXIST_DATA_MAX) {
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//						item_pos = CCOM_DATA_INCAR;
						item_pos = CCOM_DATA_TOTAL;
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
					}
				}
				save_page = page;
				page = dsp_ccomchk_cursor(item_pos, 1, save_page);	// 反転表示
				if (page != save_page) {
					fkey_enb = 0;
					Lagtim( OPETCBNO, 2, 200/20 );		// 件数表示ﾀｲﾏｰ2(200ms)起動 = TIMEOUT2
					countreq = 1;						// 件数要求中
				}
			}
			break;

		case IBK_CTRL_NG:	// 指示送信NG
			Lagcan(OPETCBNO, 7);	// ﾀｲﾑｱｳﾄ監視ｷｬﾝｾﾙ
		case TIMEOUT7:		// 応答ﾀｲﾑｱｳﾄ
			if (mode) {
				BUZPIPI();
				displclr(4);
				grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[3] );		// "　　　現在実行できません　　　"
				Fun_Dsp(FUNMSG[8]);													// "　　　　　　　　　　　　 終了 "
				mode = 3;
			}
			break;
		case IBK_CTRL_OK:	// 指示送信OK
			if (mode) {		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				Lagcan(OPETCBNO, 7);	// ﾀｲﾑｱｳﾄ監視ｷｬﾝｾﾙ
				dsp  = 2;				// 画面再描画
				mode = 0;
			}
			break;

		case TIMEOUT6:							// 画面更新ﾀｲﾏｰﾀｲﾑｱｳﾄ
			if (countreq == 0) {
				dsp = 1;						// 件数再取得
			}
			else {
				countreq = 0;
				ibkdown = 1;
				dsp = 3;						// 件数再描画
			}
			break;

		case TIMEOUT2:							// 件数表示ﾀｲﾏｰ2(200ms)起動
		case IBK_COUNT_UPDATE:
			if (countreq) {
				countreq = 0;
				Lagcan(OPETCBNO, 6);			// ﾀｲﾑｱｳﾄ監視ｷｬﾝｾﾙ
			}
			ibkdown = 0;
			dsp = 3;							// 件数再描画
			break;
			
		default:
			break;
		}
	}
}

#define DATA_LINE_MAX	6
#define PAGE_ITEM_MAX	(DATA_LINE_MAX * 2)
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//static const uchar st_offset[CCOM_DATA_MAX] = { 14,2,1,3,5,4,6,16,8,7,9,18,19 };
//NOTE:st_offset中の数字はt_NtBufCountのインデックスです
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static const uchar st_offset[CCOM_DATA_MAX] = { 14,2,1,3,5,4,6,16,8,7,9,18,19,20 };
static const uchar st_offset[CCOM_DATA_MAX] = {  2, 1, 3, 5, 4, 6, 7, 8,18, 9,19 };
static const uchar st_num_dg[CCOM_DATA_MAX] = {  4, 4, 8, 3, 8, 4, 8, 4, 8, 4, 4 };	// st_offset[]に対応する表示文字桁数(半角)を定義する
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

// 件数を初期化
void	clr_dsp_count(uchar page)
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset(PreCount, 0xFF, sizeof(ulong)*CCOM_DATA_MAX);
	dispmlclr(1, 6);
	for (i=1, j=offset; (i<=DATA_LINE_MAX) && (j<CCOM_DATA_MAX); i++) {
// MH810100(S) Y.Yamauchi 2019/10/31 車番チケットレス(メンテナンス)	
//		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
//		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "件"
		grachr( i,  0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
		grachr( i, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "件"
// MH810100(E) Y.Yamauchi 2019/10/31 車番チケットレス(メンテナンス)	
		
		if ((i <= DATA_LINE_MAX) && (j < CCOM_DATA_MAX)) {
			if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
			}
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
			grachr( i, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "件"
		}
	}
	Fun_Dsp(FUNMSG[8]);		// "　　　　　　　　　　　　 終了 "
}

// 未送信データ数を取得して表示する
ushort dsp_unflushed_count(uchar type, uchar page)
{
	ushort i, j;
	ushort size;
	ulong  num;
	ulong  *pt1, *pt2;
	t_NtBufCount buf;
	char  numStr[10];
	ushort offset = page * PAGE_ITEM_MAX;
	ushort k;
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	ushort col, cnt;
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	NTBUF_GetBufCount(&buf);
	NTBUF_GetBufCount(&buf, TRUE);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	pt1 = (ulong*)&buf;
	if (type == 0) {
	// 合計表示
		pt2 = (ulong*)&IBK_BufCount;
		size = sizeof(t_NtBufCount)/sizeof(ulong);
		for (i = 0; i < size; i++) {
			*pt1++ += *pt2++;
		}
		pt1 -= size;
	}

	// 件数表示
	for (i = offset, k = 0; (i < CCOM_DATA_MAX) && (k < PAGE_ITEM_MAX); i++, k++) {
		if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
			if( i == CCOM_EXIST_DATA_MAX ) {
				break;
			}
		}
		if(0 == prm_get(COM_PRM, S_NTN, MntTransData[i][0], 1, MntTransData[i][1])) {
// MH322917(S) A.Iiizumi 2018/09/21 遠隔動作チェック改善
//			if(i == 1 && 0 == prm_get(COM_PRM, S_NTN, 26, 1, 3)) {	// 集計で34-0026④=0(送信しない）なら０件とする
			if((i == NTNET_BUFCTRL_REQ_TTOTAL) && (0 == prm_get(COM_PRM, S_NTN, 26, 1, 3))) {	// 集計で34-0026④=0(送信しない）なら０件とする
// MH322917(E) A.Iiizumi 2018/09/21 遠隔動作チェック改善
				num = 0;
			}
			else {
				num = *(pt1+st_offset[i]);
			}
		}
		else {
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)+遠隔動作チェック改善
//			num = 0;
			// 遠隔監視と長期駐車は送信マスクはしないので常に有効
			if((i == NTNET_BUFCTRL_REQ_RMON)||(i == NTNET_BUFCTRL_REQ_LONG_PARK)){
				num = *(pt1+st_offset[i]);
			}else{
				num = 0;
			}
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)+遠隔動作チェック改善
		}
		
		if (num != PreCount[i]) {
			memset(numStr, 0x00, sizeof(numStr));
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			sprintf(numStr, "%4d", num);
//			j = (ushort)((i/2)%DATA_LINE_MAX+1);
//			if (!(i%2)) {
//				// １列目(３桁)
//// MH810100(S) Y.Yamauchi 2019/10/31 車番チケットレス(メンテナンス)	
////				grachr( j, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//				grachr( j, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//// MH810100(E) Y.Yamauchi 2019/10/31 車番チケットレス(メンテナンス)	
//			}
//			else {
//				// ２列目(２桁)
//				grachr( j, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//			}
			j = (ushort)((i/2)%DATA_LINE_MAX+1);
			if( st_num_dg[i] <= 6 ){	// 半角6文字以下
				col = 8;	// 8カラム目
				cnt = 4;	// 4桁
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:570)対応
//				sprintf(numStr, "%4d", num);
				sprintf(numStr, "%4ld", num);
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:570)対応
			}else{
				col = 10;	// 10カラム目
				cnt = 2;	// 2桁
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:571)対応
//				sprintf(numStr, "%2d", num);
				sprintf(numStr, "%2ld", num);
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:571)対応
			}
			grachr( j, (ushort)((i%2)*16+col), cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			PreCount[i] = num;
		}
	}

	return 0;
}

// カーソル表示
uchar dsp_ccomchk_cursor(int index, ushort mode, uchar page)
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
// MH322917(S) A.Iiizumi 2018/09/21 遠隔動作チェック改善
//	ushort size = (col && (row>1)) ? 8 : 4;
	ushort size = 8;
// MH322917(E) A.Iiizumi 2018/09/21 遠隔動作チェック改善
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
		clr_dsp_count(new_page);
	}
// MH810100(S) K.Onodera 2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	grachr( row, ((col)?17:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[index] );
	grachr( row, ((col)?16:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[index] );
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	return new_page;
}

static uchar ChkNo;		// 通信チェック要求データ電文作成用の通信ﾁｪｯｸNo.
/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　センター通信テスト                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_ccomtst( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_ccomtst( void )
{
	ushort	msg;
	char	mode;	// 0:初期/ 1:通信中/ 2:結果

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[2] );		// "＜センター通信テスト＞　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[4] );		// " 回線を接続しテストを行います "
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );		// "　　　　よろしいですか？　　　"
	Fun_Dsp(FUNMSG[19]);					// "　　　　　　 はい いいえ　　　"
	
	mode = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			return MOD_CHG;
		case KEY_TEN_F5:
			if (mode == 2) {		// 「終了」
				BUZPI();
				return MOD_EXT; 
			}
			break;
		case KEY_TEN_F4:
			if (mode == 0) {		// 「いいえ」
				BUZPI();
				return MOD_EXT; 
			}
			break;
		case KEY_TEN_F3:
			if (mode == 0) {		// 「はい」
				BUZPI();

				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[10] );			// "　　 しばらくお待ち下さい 　　"
				displclr(4);
				Fun_Dsp(FUNMSG[00]);													// "　　　　　　 　　 　　　　　　"
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );			// "　　　　 通信テスト中 　　　　"
				if (++ChkNo > 99){
					ChkNo = 1;
				}
				NTNET_Snd_Data100_R(0, ChkNo);				// 通信チェック要求
				mode = 1;
			}
			else if (mode == 1) {	// 「中止」
			}
			break;
		case IBK_COMCHK_END:		// 通信テスト終了
			if (mode == 1) {
				BUZPIPI();
				
				blink_end();										// 点滅終了
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );					// "　　　　通信テスト終了　　　　"
				displclr(3);
				
				if (IBK_ComChkResult == 0) {
					grachr ( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[6] );				// "　センターとの通信は正常です　"
				}
				else {
					grachr ( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[7] );				// "センターとの通信に失敗しました"
					
					switch (IBK_ComChkResult) {
					case 2:											// "　　（02：上り回線使用中）　　"
					case 3:											// "　　 （03：回線接続失敗） 　　"
					case 5:											// "（05：ＴＣＰコネクション失敗）"
					case 4:											// "（04：ＴＣＰコネクション失敗）"
					case 6:											// "　　（06：サーバー無応答）　　"
					case 7:											// "　　（07：下り回線使用中）　　"
					case 11:										// "（11：FOMAモジュール接続不良）"
					case 12:										// "　 （12：ダイアル発信失敗） 　"
					case 13:										// "　　　（13：PPP接続失敗） 　　"
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[IBK_ComChkResult-1] );
						break;
					case 99:
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[14] );		// "　 （99：受信データ不一致） 　"
						break;
					default:
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[15] );		// "　　（失敗理由コード：　）　　"
						opedpl2( 5, 22, (ulong)IBK_ComChkResult, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF  );
						break;
					}
				}
				Fun_Dsp(FUNMSG[8]);									// "　　　　　　　　　　　　 終了 "

				mode = 2;
			}
			break;
			
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　接続先ＡＰＮ確認		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnDisp( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnDisp( void )
{
	ushort	fncChkCcomApnEvent;

	ushort	msg;
	ushort	F3Cnt;									// F3 3回押下により，APN変更（隠し）モードへ遷移

	for ( ; ; ) {
		fncChkCcomApnEvent=0;
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[0] );		// "＜接続先ＡＰＮ確認＞　　　　　"
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[1] );		// "APN 　　　　　　　　　　　　　"
		if(prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {	// クレジットなし
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[2] );	// "ｾﾝﾀｰ局IP　　　　　　　　　　　"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "自局IP　　　　　　　　　　　　"
		}
		else if(_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) {	// センター&クレジット
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[2] );	// "ｾﾝﾀｰ局IP　　　　　　　　　　　"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[8] );	// "ｸﾚｼﾞｯﾄIP　　　　　　　　　　　"
			grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "自局IP　　　　　　　　　　　　"
		}
		else {											// クレジットのみ
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[8] );	// "ｸﾚｼﾞｯﾄIP　　　　　　　　　　　"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "自局IP　　　　　　　　　　　　"
		}

		memset( APN_str, 0x20, sizeof(APN_str));	// Clear

		CcomApnGet( APN_str );						/*	APN Get						*/
		grachr( 2,  10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, APN_str);				// APN表示
		grachr( 3,  10, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, &APN_str[20]);		// APN表示

		if(prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {	// クレジットなし
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 62);				// ｾﾝﾀｰ局IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ｾﾝﾀｰ局IP表示

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// 自局IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// 自局IP表示
		}
		else if(_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) {	// センター&クレジット
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 62);				// ｾﾝﾀｰ局IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ｾﾝﾀｰ局IP表示

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CRE, 21);				// 自局IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ｸﾚｼﾞｯﾄP表示

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// 自局IP
			grachr( 6,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// 自局IP表示
		}
		else {										// クレジットのみ
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CRE, 21);				// 自局IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ｸﾚｼﾞｯﾄIP表示

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// 自局IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// 自局IP表示
		}

		Fun_Dsp(FUNMSG[8]);							/* "　　　　　　　　　　　　 終了 " */

		F3Cnt=0;
		for ( ; ; ) {
	
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:			// 「終了」
				BUZPI();
				return MOD_EXT;

			case KEY_TEN_F3:		//変更
				if( F3Cnt < 2 ){
					F3Cnt++;
				}else{
					F3Cnt=0;
					BUZPI();
					fncChkCcomApnEvent = FncChk_CcomApnSelect();
				}
				break;
			case KEY_TEN_F1:		//
			case KEY_TEN_F2:		//
			case KEY_TEN_F4:		//
					F3Cnt=0;
				break;
			default:
				break;
			}
			if (fncChkCcomApnEvent == MOD_CHG){
				OPECTL.Mnt_mod = 0;
				OPECTL.Mnt_lev = (char)-1;
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			else if( fncChkCcomApnEvent == MOD_CUT ) {
				OPECTL.Mnt_mod = 0;
				OPECTL.Mnt_lev = (char)-1;
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			}else if (fncChkCcomApnEvent == MOD_EXT) {
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　電波受信状態確認		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomAntDisp( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  k.totsuka                                      		   |*/
/*| Date        :  2012-09-10                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomAntDisp( void )
{
	ushort	msg;
	ushort	maxCount = 0;
	ushort	nowCount = 1;
	ushort	antAve = 0,antAve2 = 0,antMax = 0,antMin = 0;
	ushort	recAve = 0,recMax = 0,recMin = 0;
	long	antAve_work = 0,recAve_work = 0;
	char	prog[16];
	ushort	modemOff = 0;
	ushort	pos = 0;
	short	in_dat = -1;
	short	dat[2];
	ushort	measure = 0;
	ushort	tmRetryCnt;
	uchar	mnt_chg = 0;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
	uchar	discon = 0;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

	// HOSTと送受信中ならアンテナレベル取得開始不可
	if( RAUhost_GetSndSeqFlag() != RAU_SND_SEQ_FLG_NORMAL ||
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//		Credit_GetSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ||
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
		RAUhost_GetRcvSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "＜電波受信状態確認＞　　　　　"
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[11] );				// "　　通信中は実行できません　　"

		Fun_Dsp(FUNMSG[8]);																// "　　　　　　　　　　　　 終了 "

		for ( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				return MOD_CHG;
			case KEY_TEN_F5:						// 「終了」
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
			case KEY_TEN_F4:						// 「読出」
				BUZPI();
				break;
			default:
				break;
			}
		}
	}

	// 以下、測定条件（送信間隔、送信回数）入力
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "＜電波受信状態確認＞　　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[10] );				// "　 測定条件を入力して下さい 　"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[8] );				// "　送信間隔：　　　　　　　　　"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[9] );				// "　送信回数：　　　　　　　　　"
	dat[0] = ANT_INT_DEF;						// 送信間隔
	dat[1] = ANT_CNT_DEF;						// 送信回数
	antsetup_dsp( dat, pos );

	Fun_Dsp(FUNMSG[68]);						// "　▲　　▼　　　　 実行  終了 "

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:						// 「終了」
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F1:						// 「▲」
		case KEY_TEN_F2:						// 「▼」
//			if(( in_dat < 1 ) || ( in_dat > 999 )){ 					// 入力値範囲ﾁｪｯｸ
//				BUZPIPI();
//				in_dat = -1;
//				break;
//			}
			if( in_dat != -1 ) {
				dat[pos] = in_dat;
				in_dat = -1;
			}

			BUZPI();
			if(pos == 0){
				pos = 1;
			}
			else{
				pos = 0;
			}
			antsetup_dsp( dat, pos );
			break;
		case KEY_TEN_F3:
			break;
		case KEY_TEN_F4:						// 「読出」
			if( in_dat != -1 ){
				dat[pos] = in_dat;
				in_dat = -1;
			}
			if((dat[0] < 1) || (dat[0] > 999) || (dat[1] < 1) || (dat[1] > 999)){
				BUZPIPI();
				in_dat = -1;
				break;
			}
			BUZPI();
			measure = 1;
			break;
		case KEY_TEN:
			BUZPI();
			if( in_dat == -1 )
			{	// in_dat <= 入力値
				in_dat = (short)(msg - KEY_TEN0);
			}else{
				in_dat = (short)( in_dat % 100 ) * 10 + (short)(msg - KEY_TEN0);
			}
			// 入力値を表示する（反転表示）
			opedsp( 4+pos, 14, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
			break;
		case KEY_TEN_CL:
			BUZPI();
			in_dat = 0;
			// 入力値を表示する（反転表示）
			opedsp( 4+pos, 14, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
			break;
		default:
			break;
		}
		if(measure){								// 測定処理へ
			break;
		}
	}

	// 以下、測定処理
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "＜電波受信状態確認＞　　　　　"
	grachr( 2,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_ON, CCOMSTR4[3] );			// "　　 しばらくお待ち下さい 　　"
	Fun_Dsp(FUNMSG[117]);															// "             中止        終了 "

	if( KSG_RauGetAntLevel( 0, dat[0] ) < 0 ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "＜電波受信状態確認＞　　　　　"
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[11] );				// "　　通信中は実行できません　　"

		Fun_Dsp(FUNMSG[8]);																// "　　　　　　　　　　　　 終了 "

		for ( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				return MOD_CHG;
			case KEY_TEN_F5:						// 「終了」
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
			case KEY_TEN_F4:						// 「読出」
				BUZPI();
				break;
			default:
				break;
			}
		}
	}
	
	Lagtim( OPETCBNO, 6, 60*50 );					// 応答受信ﾀｲﾏｰ6(60s)起動
	tmRetryCnt = 0;
	maxCount = dat[1];

	// 計測処理
	for ( ; ; ) {

		for ( ; ; ) {

			msg = StoF( GetMessage(), 1 );

			switch( msg ){							// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				if(nowCount <= maxCount){
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:576)対応
//					discon == 1;
					discon = 1;
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:576)対応
				}
				else{
					Lagcan(OPETCBNO, 6);
					if(modemOff == 0){
						KSG_RauGetAntLevel( 1, 0 );		// アンテナレベル、受信電力指数の取得終了
					}
					return MOD_CUT;
				}
				break;				
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				if(nowCount <= maxCount){
					mnt_chg ^= 1;
				}
				else{
					Lagcan(OPETCBNO, 6);
					if(modemOff == 0){
						KSG_RauGetAntLevel( 1, 0 );		// アンテナレベル、受信電力指数の取得終了
					}
					return MOD_CHG;
				}
				break;

			case KEY_TEN_F5:						// 「終了」
				BUZPI();
				Lagcan(OPETCBNO, 6);
				if(modemOff == 0){
					KSG_RauGetAntLevel( 1, 0 );		// アンテナレベル、受信電力指数の取得終了
				}

				if(mnt_chg){
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//				else{
//					return MOD_EXT;
//				}
				else if(discon){
					return MOD_CUT;
				}else{
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F4:
				break;
			case KEY_TEN_F3:						// 「中止」
				if(modemOff == 0){
					if(nowCount <= maxCount){
						BUZPI();
						Lagcan(OPETCBNO, 6);
						dispmlclr(1, 3);
						KSG_RauGetAntLevel( 1, 0 );		// アンテナレベル、受信電力指数の取得終了
						modemOff = 1;
						grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[7] );			// "　　　処理を中止しました　　　"
						Fun_Dsp(FUNMSG[8]);				// "　　　　　　　　　　　　 終了 "
					}
				}
				break;
			case KSG_ANT_EVT:						// アンテナレベル受信イベント
				Lagtim( OPETCBNO, 6, 60*50 );			// 応答受信ﾀｲﾏｰ6(60s)起動
				if(nowCount <= maxCount){
					sprintf(prog, "(%03d/%03d)", nowCount, maxCount);
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[5] );		// "　　測定中　　　　　　　　　　"
					grachr( 3, 12, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)prog );
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[1] );		// "アンテナレベル：　　　　　　　"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[2] );		// "受信電力指数　：　　　　　　　"
					if(KSG_gAntLevel != -1){
						opedpl3( 4, 18, KSG_gAntLevel, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						grachr( 4, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, "－" );
						break;
					}
					if(KSG_gReceptionLevel != -1){
						opedpl3( 5, 17, KSG_gReceptionLevel, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						grachr( 5, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, "－" );
						break;
					}
					antAve_work += KSG_gAntLevel;
					recAve_work += KSG_gReceptionLevel;
					if(antMax < KSG_gAntLevel){
						antMax = KSG_gAntLevel;
					}
					if((antMin == 0) || (antMin > KSG_gAntLevel)){
						antMin = KSG_gAntLevel;
					}
					if(recMax < KSG_gReceptionLevel){
						recMax = KSG_gReceptionLevel;
					}
					if((recMin == 0) || (recMin > KSG_gReceptionLevel)){
						recMin = KSG_gReceptionLevel;
					}
					nowCount++;
				}
				else{
					Lagcan(OPETCBNO, 6);
					BUZPI();
					antAve = (antAve_work*10) / (nowCount - 1);
					antAve2 = ((antAve_work*100) / (nowCount - 1)) % 10;
					recAve = recAve_work / (nowCount - 1);
					dispmlclr(1, 5);
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[6] );		// "　　　測定結果　平均 最大 最小"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[1] );		// "アンテナレベル：　　　　　　　"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[2] );		// "受信電力指数　：　　　　　　　"
					if(antAve2 < 5){					// 小数点2位を四捨五入し小数点1位に反映
						opedpl3( 4, 16, antAve/10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						opedpl3( 4, 18, antAve%10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						if((antAve%10) == 9){
							opedpl3( 4, 16, (antAve/10)+1, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						}
						else{
							opedpl3( 4, 16, antAve/10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						}
						opedpl3( 4, 18, (antAve%10)+1, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					grachr( 4,  17, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, "." );
					opedpl3( 4, 23, antMax, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 4, 28, antMin, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 17, recAve, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 22, recMax, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 27, recMin, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					KSG_RauGetAntLevel( 1, 0 );			// アンテナレベル、受信電力指数の取得終了
					modemOff = 1;
					Fun_Dsp(FUNMSG[8]);					// "　　　　　　　　　　　　 終了 "
				}
				break;
			case TIMEOUT6:								// 応答受信ﾀｲﾏｰﾀｲﾑｱｳﾄ
				if(tmRetryCnt == 0){
					KSG_RauGetAntLevel( 0, dat[0] );	// アンテナレベル、受信電力指数の取得開始
					Lagtim( OPETCBNO, 6, 60*50 );		// 応答受信ﾀｲﾏｰ6(60s)起動
					tmRetryCnt++;
				}
				else{
					dispmlclr(1, 5);
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[4] );
					KSG_RauGetAntLevel( 1, 0 );				// アンテナレベル、受信電力指数の取得終了
					modemOff = 1;
					Fun_Dsp(FUNMSG[8]);					// "　　　　　　　　　　　　 終了 "
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| アンテナレベル取得の送信間隔、送信回数表示		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ping_dsp( short * dat, short pos )                      |*/
/*| PARAMETER    : short * dat                                             |*/
/*|              : short pos                                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void antsetup_dsp( short * dat, short pos )
{
	ushort mod[2];												// 0(normal) / 1(reverse)

	memset(mod, 0x00, sizeof(mod));
	mod[pos] = 1;
	opedsp( 4, 14, (unsigned short)dat[0], 3, 0, mod[0], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 5, 14, (unsigned short)dat[1], 3, 0, mod[1], COLOR_BLACK, LCD_BLINK_OFF );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　センター追番クリア                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_cOibanClr( void )                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_cOibanClr( void )
{
	ushort	msg;
	uchar	mode;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
	uchar	dsp;		// 0:描画なし/ 1:画面表示
	uchar	item_pos;	// カーソル位置
	ushort	pos;
	uchar	i;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "＜センター追番クリア＞　　　　"

	dsp      = 1;
	mode     = 0;
	item_pos = 0;

	for ( ; ; ) {
		if (dsp) {
		// 件数表示更新要求あり
			if (mode == 0) {
				if (dsp == 1) {
					cOibanClrDsp();						// クリア画面表示
					cOibanCursorDsp(item_pos, 1);		// 反転表示
					Fun_Dsp(FUNMSG[119]);				// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:			// 「終了」
			if (mode == 0) {		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				BUZPI();
				return MOD_EXT;
			}
			else if (mode == 3) {	// エラー画面
				BUZPI();
				dsp  = 1;
				mode = 0;
			}
			break;

		case KEY_TEN_F1:		// クリア
			if (mode == 0) {
				BUZPI();
				dispmlclr(1, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
				pos = 4;
				// 釣銭管理
				if( item_pos == 2 ){
					pos = 8;	// 4文字なので8Byte
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[item_pos] );				// "選択項目"
				grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "追番をクリアします"
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
				Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
				mode = 1;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			}
			break;
		case KEY_TEN_F2:		// 全クリア
			if (mode == 0) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				BUZPI();
				
				dispmlclr(1, 6);
				grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );				// "全"
				grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "追番をクリアします"
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
				Fun_Dsp(FUNMSG[19]);								// "　　　　　　 はい いいえ　　　"
				mode = 2;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			}
			break;

		case KEY_TEN_F3:
			switch (mode) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
			case 0:			// カーソル移動
				BUZPI();
				cOibanCursorDsp(item_pos, 0);			// 正転表示
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				if (++item_pos >= 4) {
				if (++item_pos >= 3) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					item_pos = 0;
				}
				cOibanCursorDsp(item_pos, 1);			// 反転表示
				break;
			case 1:			// 個別クリア実行
				BUZPI();
				switch(item_pos) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//				case 0:									// センター追番（入庫）
//					Rau_SeqNo[RAU_SEQNO_ENTER] = 1;
//					break;
//				case 1:									// センター追番（集計）
//					Rau_SeqNo[RAU_SEQNO_TOTAL] = 1;
//					break;
//				case 2:									// センター追番（精算）
//					Rau_SeqNo[RAU_SEQNO_PAY] = 1;
//					break;
//				case 3:									// センター追番（釣銭管理集計）
//					Rau_SeqNo[RAU_SEQNO_CHANGE] = 1;
//					break;
				case 0:									// センター追番（集計）
					Rau_SeqNo[RAU_SEQNO_TOTAL] = 1;
					break;
				case 1:									// センター追番（精算）
					Rau_SeqNo[RAU_SEQNO_PAY] = 1;
					break;
				case 2:									// センター追番（釣銭管理集計）
					Rau_SeqNo[RAU_SEQNO_CHANGE] = 1;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				}
				dsp  = 1;	// 画面再描画
				mode = 0;
				break;
			case 2:			// 全クリア実行
				BUZPI();
				for(i = 0; i < RAU_SEQNO_TYPE_COUNT; ++i) {
					Rau_SeqNo[i] = 1;
				}
				dsp  = 1;	// 画面再描画
				mode = 0;
				break;
			case 3:
			default:
				break;
			}
			break;

		case KEY_TEN_F4:						// 「いいえ」
			if ((mode == 1) || (mode == 2)) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				BUZPI();
				dsp  = 1;	// 画面再描画
				mode = 0;
			}
			break;

		default:
			break;
		}
	}
}

// センター追番画面表示
void cOibanClrDsp(void)
{
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//	ushort i, j;
	ushort i = 1, j = 0;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

	memset(PreCount, 0xFF, sizeof(ulong)*CCOM_DATA_MAX);
	dispmlclr(1, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//	for (i=1, j=0; i<=2; i++) {
//		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );
//		grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );
//	}
	// １行目
	grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "集計　　"
	grachr( i, 15, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "精算　　"

	// ２行目
	i++;
	grachr( i,  0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "釣銭管理"
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
	Fun_Dsp(FUNMSG[8]);		// "　　　　　　　　　　　　 終了 "
}

// カーソル表示
void cOibanCursorDsp(uchar index, ushort mode)
{
	ushort row  = (ushort)(index/2+1);
	ushort col  = (ushort)(index%2);
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//	ushort size = (col && (row>1)) ? 8 : 4;
	ushort size = 4;

	// 釣銭管理
	if(index == 2 ){
		size = 8;
	}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[index] );
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　接続先ＡＰＮ選択（工場検査）	               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnSelect( void ) 	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnSelect( void )
{
	unsigned short	fncChkCcomApnEvent;
//	ushort	msg;
	char	wk[2];
//	uchar	mode;		// 0:APN変更待ち画面/ 1:APN変更終了画面
//	ushort	i;

	for ( ; ; ) {
		DP_CP[0] = DP_CP[1] = 0;
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[4] );		// "＜接続先選択（工場検査）＞　　"

		fncChkCcomApnEvent = Menu_Slt( CCOMMENU2, CCOM_CHK2_TBL, CCOM_CHK2_MAX, 1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (fncChkCcomApnEvent) {

			case CCOM_APN_AMS:		// ＡＭＳ用ＡＰＮ
			case CCOM_APN_AMN:		// アマノ用ＡＰＮ
			case CCOM_APN_PWEB:		// 駐車場センター用ＡＰＮ
				fncChkCcomApnEvent = FncChk_CcomApnSet(fncChkCcomApnEvent);
			break;

			case MOD_EXT:
				return MOD_EXT;

			case KEY_TEN_F5:			// 「終了」
				BUZPI();
				return MOD_EXT;

			default:
				break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];

		if (fncChkCcomApnEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		else if( fncChkCcomApnEvent == MOD_CUT) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　接続先設定（工場検査）		               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnSet( void ) 	                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnSet( ushort	fncChkCcomApnEvent )
{
	ushort	msg;
//	char	wk[2];
	uchar	mode;		// 0:APN変更待ち画面/ 1:APN変更終了画面
	ushort	i;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[4] );		// "＜接続先選択（工場検査）＞　　"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[5] );		// "接続先は、　　　　　　　　　　"

	if( fncChkCcomApnEvent == CCOM_APN_AMS){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[0] );		// "１．ＡＭＳ用ＡＰＮ　　　　　　"
	}else if( fncChkCcomApnEvent == CCOM_APN_AMN){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[1] );		// "２．アマノ用ＡＰＮ　　　　　　"
	}else if( fncChkCcomApnEvent == CCOM_APN_PWEB){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[2] );		// "３．駐車場センター用ＡＰＮ　　"
	}else{
															// ""
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS]);				// APN表示
	grachr( 4, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS][30]);			// APN表示
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );			// "　　　　よろしいですか？　　　"
	Fun_Dsp(FUNMSG[19]);						// "　　　　　　 はい いいえ　　　"

	mode = 0;
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:			// 「終了」
				if( mode == 1){ 
					BUZPI();
					return MOD_EXT;
				}
				break;
			case KEY_TEN_F3:		// 「はい」
				if( mode == 0){ 
					BUZPI();
					for(i=0;i<32;i++){
						if(APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS][i] == 0x20){
							break;
						}
					}
					CcomApnSet( APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS],i );

					DataSumUpdate(OPE_DTNUM_COMPARA);			/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);	// FlashROM update
					wopelg( OPLOG_CCOMCHK_APN, 0, 0 );			// 操作履歴登録  ｾﾝﾀｰ通信ﾁｪｯｸ APN変更操作（システムメンテナンス）

					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[6] );		// "ＡＰＮを変更しました。　　　　"
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[7] );		// "再起動後に更新されます。　　　"
					Fun_Dsp(FUNMSG[8]);							// "　　　　　　　　　　　　 終了 "
					mode = 1;	
				}else if( mode == 1){ 
				}
				break;
			case KEY_TEN_F4:		// 「いいえ」
				if( mode == 0){ 
					BUZPI();
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　APNを共通パラーメータから配列に取得する      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomApnGet( void ) 			                           |*/
/*| PARAMETER    : uchar *apn_str:apn文字列                                |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : APN文字列を設定から読み出し、APN文字数分（設定）を	   |*/
/*| 			 : 配列にセットする　									   |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomApnGet( uchar *apn_str )
{
	unsigned short	apn_cnt;
	unsigned char	prm;
	short			i;
	char 			pos;

	apn_cnt=(unsigned short)(prm_get(COM_PRM,S_CEN,79,2,1));	/* APN文字数				*/
	prm=(unsigned char)(prm_get(COM_PRM,S_CEN,80,3,4));			/* APN 1文字目				*/
																/*							*/
	if((apn_cnt != 0)&&(apn_cnt <= 32)&&(prm != 0)){			/* APN文字数指定あり，32文字以下，最初のデータが０以外	?(Y)*/
		for( i=0; i<apn_cnt; i++){								/*							*/
			pos = (char)(4-(i%2)*3);							/* 654または321				*/
			prm = (unsigned char)prm_get(COM_PRM,S_CEN,(short)(80+i/2), (char)3, (char)pos) ;/* APN	*/
			*apn_str = (uchar)(prm);							/* APN Get					*/
			apn_str++;											/*							*/
		}														/*							*/
	}															/*							*/
																/*							*/
}																/*							*/

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　IPを共通パラーメータから配列(文字)に取得する |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomIpGet( void ) 			                           |*/
/*| PARAMETER    : uchar *ipstr:文字列格納先                               |*/
/*| 			 : ses ｾｯｼｮﾝNo.(共通パラメータ)					           |*/
/*| 			 : adr ｱﾄﾞﾚｽNo.(共通パラメータ)					           |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : IPを設定から読み出し、配列にセットする    		       |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomIpGet( uchar *ipstr, short	ses, short	adr)
{

	unsigned char	prm;
	char			i;
	char			j;
	short			k;
	short			spc_set;

	k=0;																			/*							*/
	for( j=0; j<4;j++){																/*	4ｵｸﾃｯﾄ					*/
		if( j > 0 ){																/* 初回はピリオドつけない	*/
			ipstr[k++]= '.';														/*							*/
		}																			/*							*/
		if( j == 2 ){																/* 次のアドレス				*/
			adr++;																	/*							*/
		}																			/*							*/
																					/*							*/
		spc_set = 0;																/*							*/
		for( i=0 ;i<3; i++){ 														/* 3文字分取得				*/
			prm =(unsigned char)(prm_get(COM_PRM,ses,adr,1,(char)(6-i-(j % 2)*3)));	/* IP 1-3文字目	/4-6文字目	*/
			prm +='0';	/* IP 1-3文字目	/4-6文字目	*/								/*							*/
			if(( prm == '0')&&(spc_set == 0)){										/*							*/
				prm = 0x20;															/* 先頭の0はスペースに置き換え	*/
			}else{																	/* 0以外の文字検出?(Y)		*/
				spc_set = 1;														/*							*/
			}																		/*							*/
			ipstr[k++]= prm;														/*							*/
		}																			/*							*/
																					/*							*/
	}																				/*							*/
																					/*							*/
}																					/*							*/

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　APNを共通パラーメータにセットする			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomApnSet( void ) 			                           |*/
/*| PARAMETER    : uchar *apn_str:apn文字列                                |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : APN文字列を設定に書き込む						       |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomApnSet( const uchar *apn_str,unsigned short apn_cnt )
{
	short			i;											/*							*/
																/*							*/
	if((apn_cnt != 0 )&&(apn_cnt <= 32 )){						/*							*/
		CPrmSS[S_CEN][79] = apn_cnt;							/*							*/
		for( i=0;i<32;i++){										/*							*/
			if( i < apn_cnt	){									/*	APN文字あり?(Y)			*/
																/*							*/
				if( (i % 2) == 0 ){								/*							*/
					CPrmSS[S_CEN][80+i/2] = (long)apn_str[i]*1000L;	/*	設定上書き			*/
				}else{											/*							*/
					CPrmSS[S_CEN][80+i/2] += (long)apn_str[i];	/*	設定加算				*/
				}												/*							*/
			}else{												/*	APN文字部分終了			*/
				if( (i % 2) == 0 ){								/*							*/
					CPrmSS[S_CEN][80+i/2] = 0L;					/*	設定上書き				*/
				}else{											/*							*/
					CPrmSS[S_CEN][80+i/2] += 0L;				/*	設定加算				*/
				}												/*							*/
			}													/*							*/
		}														/*							*/
	}															/*							*/
																/*							*/
}																/*							*/

// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
////[]----------------------------------------------------------------------[]
/////	@brief			音声案内時間画面でのアナウンステスト
////[]----------------------------------------------------------------------[]
/////	@param[in]		volume	: volume
/////	@attention		None
/////	@author			Yamada
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2008/02/05<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void VoiceGuideTest(char volume)
//{
//	pre_volume[0] = volume;
//	memset(avm_test_no, 0, sizeof(avm_test_no));
//	avm_test_no[0] = 145;			// リパーク駐車場をご利用いただきありがとうございました
//	avm_test_no[1] = 136;			// フラップ版が下がったことを確認して出庫して下さい
//	avm_test_cnt = 2;
//	avm_test_ch = 0;;
//
//	ope_anm( AVM_AN_TEST );		// ありがとうございました。
//}
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)

//[]----------------------------------------------------------------------[]
//	@brief			フラップ装置チェックメニュー表示
//[]----------------------------------------------------------------------[]
//	@param[in]		volume	: volume
//	@attention		None
//	@author			Yamada
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2008/02/05<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort		FunChk_FlpCHK( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];
	uchar	flp_check;
	uchar	i;

	/* CRRの接続設定がされていない時は画面遷移しない（警告音）*/
	if(!( GetCarInfoParam() & 0x04 )) {
		BUZPIPI();
		return MOD_EXT;
	}
	flp_check = 0;
	for( i=INT_CAR_START_INDEX ; i<TOTAL_CAR_LOCK_MAX; i++ ){
		if( (LockInfo[i].lok_syu == LK_TYPE_SANICA_FLAP) ){
			flp_check = 1;	// サニカ設定のフラップがある
			break;
		}
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[54]);									/* "＜フラップ装置チェック＞　　　" */
		msg = Menu_Slt(FLPBOARDMENU, FUNC_FLP_TBL, FUNC_FLP_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)		
		case MOD_CHG:
			return(MOD_CHG);
		case MOD_EXT:		/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
		case MNT_FLP_LOOP_DATA_CHK:					// ループデータ確認
			if( 1 == flp_check ){
				wopelg( OPLOG_FLA_LOOP_DATA, 0, 0 );	// 操作履歴登録
				ret = FunChk_FlapLoopData();
			} else {				// 通信タイプのフラップが設定されていない場合、遷移させない
				BUZPIPI();
			}
			break;
		case MNT_FLP_SENSOR_CTRL:					// 車両検知センサー操作
			wopelg( OPLOG_FLA_SENSOR_CTRL, 0, 0 );	// 操作履歴登録
			ret = FunChk_FlapSensorControl();
			break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if (ret == MOD_CHG) {
		if (ret == MOD_CHG ||  ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			フラップ制御基板チェックメニュー表示
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	FunChk_CrrBoardCHK( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[52]);		/* "＜フラップ制御基板チェック＞　　" */
		msg = Menu_Slt(CRRBOARDMENU, FUNC_CRR_TBL, FUNC_CRR_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)		
		case MOD_CHG:
			return(MOD_CHG);
		case MOD_EXT:		/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
		case MNT_CRR_VER:	/* バージョンチェック	*/
			wopelg( OPLOG_CCRVERCHK, 0, 0 );		// 操作履歴登録
			ret = FunChk_CRRVer();
			break;
		case MNT_CRR_TST:	/* 折り返しテスト 		*/
			wopelg( OPLOG_CCRCOMCHK, 0, 0 );		// 操作履歴登録
			ret = FunChk_CrrComChk();
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (ret == MOD_CHG) {
		if ( ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)			
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

const	ulong	CarParamDef[3][10]={
	{2,		0,	0,	0,	0,20,	0,110000,	0},		// [0]駐車（CRR）のみ
	{0,		0,	0,	0,	0,10,	1,	   0,	0},		// [1]駐車（IF）のみ
	{41,42,43,44,45,100,101,123,124},				// [2]設定アドレス用
};

/*[]----------------------------------------------------------------------[]*/
/*|  車室機能設定：						                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CarFuncSelect( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	CarFuncSelect( void )
{
	ushort	msg;
	char	pos=0;		/* 0:駐車 1:駐車（IF）*/
	char	changing=0;
	char	i;
	struct	LOCKINFO_REC	CarInfo;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[0]);					/* "＜車室機能選択＞　　　　　　　" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[1]);					/* "　機能選択：　　　　　　　　　" */
	grachr(2, 12, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, &KIND_3_MENU[0][2]);			/* "駐車/駐車（IF）" */
	Fun_Dsp(FUNMSG[98]);																/* "  ▲    ▼         設定  終了 " */
	
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (changing != 2) {
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	/* メンテナンスキー */
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:	/* 終了(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:	/* ▲ */
			case KEY_TEN_F2:	/* ▼ */
				pos ^= 1;
				BUZPI();
				//画面表示変更
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[1]);								/* "　機能選択：　　　　　　　　　" */
				grachr(2, 12, (ushort)(pos==1?10:4), 1, COLOR_BLACK, LCD_BLINK_OFF, &KIND_3_MENU[pos][2]);	/* "駐車/駐車（IF）" */
				break;
			case KEY_TEN_F4:	/* 設定(F4) */
				BUZPI();
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[2]);				/* "　　　設定を変更しました　　　" */
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[3]);				/* "　 精算機の再起動が必要です 　" */
				Fun_Dsp(FUNMSG2[43]);															/* "　　　　　　 確認 　　　　　　" */
				changing = 2;
				for( i=0;i<TBL_CNT(CarParamDef[0]);i++ ){
					CPrmSS[S_TYP][CarParamDef[2][i]] = CarParamDef[pos][i];
				}

				memset(LockInfo, 0, sizeof(LockInfo));	/* ゼロクリア */
				memset(&CarInfo, 0, sizeof(CarInfo));	/* ゼロクリア */
				
				if( !pos ){
					CarInfo.adr = 50;
				}else{
					CarInfo.adr = 0;
				}
				CarInfo.dat.lok_syu = 11;
				CarInfo.dat.ryo_syu = 1;
				CarInfo.dat.area = 0;
				CarInfo.dat.posi = 1;
				CarInfo.dat.if_oya = 1;
				CarInfo.dat.lok_no = 1;
						
				LockInfo[CarInfo.adr] = CarInfo.dat;

				OpelogNo = OPLOG_CAR_FUNC_SELECT;
				SetChange = 1;
				f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
				memcpy( &bk_LockInfo, &LockInfo, sizeof( LockInfo ));
// GG120600(S) // Phase9 設定変更通知対応
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応				
				// 車室パラメータはここで上書きを行う。 共通パラメータは上位の共通箇所で書き換え
				DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM上のSUM更新
				FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH書込み
				if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
					NTNET_Snd_Data225(0);						// NTNETへデータ送信
				}
				break;
			default:
				break;
			}
		}
		else {
			switch (msg) {
			case KEY_TEN_F3:
				BUZPI();
				if( CP_MODECHG == 0 ){
					return MOD_CHG;
				}else{
					return MOD_EXT;
				}
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Wait                   	                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 磁気ﾘｰﾀﾞｰﾁｪｯｸからの戻り判定処理                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FRdrChk_Wait( void )
{
	
	short		msg;		// 受信ﾒｯｾｰｼﾞ
	
	Lagtim( OPETCBNO, 6, 10*50 );					// 応答待ちﾀｲﾏｰ(10s)起動
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );				// ﾒｯｾｰｼﾞ受信

		switch( msg){								// 受信ﾒｯｾｰｼﾞ？
			case ARC_CR_E_EVT:						// 終了ｺﾏﾝﾄﾞ受信
			case TIMEOUT6:							// 応答待ちﾀｲﾏｰﾀｲﾑｱｳﾄ
				Lagcan( OPETCBNO, 6 );
				return;
			default:
				break;
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金設定通信ログ印字                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ryokin_Setlog_Print( void )                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#define	RYOSET_INJI_END	(PREQ_RYOSETTEI_LOG | INNJI_ENDMASK)
extern	void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
ushort Ryokin_Setlog_Print( void )
{
	ushort	NewOldDate[6];		// 最古＆最新ﾛｸﾞﾃﾞｰﾀ日付ﾃﾞｰﾀ格納ｴﾘｱ
	ushort	LogCount;			// LOG登録件数
	date_time_rec	NewestDateTime, OldestDateTime;
	if (Ope_isPrinterReady() == 0) {	// レシート出力不可
		BUZPIPI();
		return MOD_EXT;
	}
	BUZPI();
	Ope2_Log_NewestOldestDateGet( eLOG_REMOTE_SET, &NewestDateTime, &OldestDateTime, &LogCount );
	// 最古＆最新日付ﾃﾞｰﾀ取得
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );

	return SysMnt_Log_CntDsp2	(	//	ログ件数表示画面処理２（対象期間指定あり）
										LOG_REMOTE_SET,
										LogCount,
										LOGSTR1[13],
										PREQ_RYOSETTEI_LOG,
										&NewOldDate[0]
									);
}	

//[]----------------------------------------------------------------------[]
///	@brief			FTP接続確認チェックメニュー
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ftp_Connect_Chk( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[0]);									/* "＜ＦＴＰ接続チェック＞　　　　" */
		msg = Menu_Slt(FTPCONNECTMENU, FUNC_FTPCONNECTCHK_TBL, FUNC_FTPCONNECT_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {			/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case MOD_CHG:
				ret = MOD_CHG;
				break;
			
		case MOD_EXT:			/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return MOD_EXT;
			
		case FTP_CONNECT_FUNC:	/* 接続確認 */
			ret = Ftp_ConnectChk();
			break;

		case FTP_PARAM_UPL:		/* パラメータアップロード	*/
			ret = ParameterUpload(0);
			break;
		}
		if( ret == MOD_CHG || ret == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//ドアノブ開チャイムが無効状態のままメンテナンスモード終了または画面を終了した場合は有効にする
				Ope_EnableDoorKnobChime();
			}
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if ( ret == MOD_CHG ) {
		if ( ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| パラメーターアップロード                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ParameterUpload( void )                                 |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : 設定ﾓｰﾄﾞｽｲｯﾁOFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	ParameterUpload( uchar type )
{
	
	ushort	msg, usSysEvent;
	uchar	loop = 0;
	
	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[12] );						// "＜パラメータアップロード＞　　"
	grachr( 3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10] );						// "　　 しばらくお待ち下さい 　　"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[16] );						// "通信終了後、再度操作を行う際は"
	grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[17] );						// "時間を空けて実行して下さい。　"
	Ope_DisableDoorKnobChime();

	wopelg( OPLOG_FTP_PARAM_UPLD, 0, 0 );							// 操作履歴登録
	remotedl_chg_info_bkup();
	remotedl_connect_req( OPLOG_REMOTE_DISCONNECT, CTRL_PARAM_UPLOAD );
	
	while ( !loop ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
		case REMOTE_CONNECT_EVT:									// FTP通信によるRism接続切断通知
			loop = 1;
			break;
		default:
			break;
		}
	}
	
	remotedl_script_typeset( PARAMTER_SCRIPT );						// スクリプトファイル種別セット
	Param_Upload_type_set( type );
	if( type ){														// ユーザーメンテ及び共通パラメータからのコール
		usSysEvent = Param_Upload_ShortCut();						// ショートカット処理へ移行
	}else{
		usSysEvent = SysMnt_FTPClient();							// 共通パラメータアップロード実施
	}
	
	remotedl_connect_req( OPLOG_REMOTE_CONNECT, CTRL_PARAM_UPLOAD );
	remotedl_chg_info_restore();
	
	return usSysEvent;
}

/*[]----------------------------------------------------------------------[]*/
/*| FTP接続確認				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ftp_ConnectChk( void )       	                       |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : 設定ﾓｰﾄﾞｽｲｯﾁOFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	Ftp_ConnectChk( void )
{
	
	ushort	msg, ret, end_flag;
	ushort	result = 0;
	uchar	mnt_chg = 0;
	ret		 = 0;
	end_flag = 0;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[1]);		// "＜接続確認＞　　　　　　　　　"
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// "　　 しばらくお待ち下さい 　　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[16]);		// "通信終了後、再度操作を行う際は"
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[17]);		// "時間を空けて実行して下さい。　"
	Fun_Dsp(FUNMSG[0]);														/* [0]	"　　　　　　　　　　　　　　　" */

	
	wopelg( OPLOG_FTP_CONNECT_CHK, 0, 0 );							// 操作履歴登録

	remotedl_chg_info_bkup();										// 現時点のステータスをバックアップする
	remotedl_status_set( R_DL_IDLE );
	remotedl_connect_req( OPLOG_REMOTE_DISCONNECT, CTRL_CONNECT_CHK );
	Ope_DisableDoorKnobChime();
	
	while ( 1 ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				if( end_flag == 1 ){
					return MOD_CUT;
				}else{
					mnt_chg ^= 2;
					continue;
				}
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	/* メンテナンスキー */
				if( end_flag == 1 ){
					return MOD_CHG;
				}else{
					mnt_chg ^= 1;
					continue;
				}
				break;
								
			case KEY_TEN_F5:	/* 終了(F5) */
				if( end_flag == 1 ){
					BUZPI();
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//					if( mnt_chg ){
//						return MOD_CHG;
//					}else{
//						return MOD_EXT;
//					}
					if( mnt_chg == 1){
						return MOD_CHG;
					}else if( mnt_chg == 2 ){
						return MOD_CUT;
					}else{
						return MOD_EXT;
					}
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
				}
				break;
			case REMOTE_CONNECTCHK_END:								// FTP通信によるRism接続切断通知
				dispmlclr(3,6);										// 画面表示ｸﾘｱ
				end_flag = 1;
				if(!Ope_IsEnableDoorKnobChime()) {
					Ope_EnableDoorKnobChime();
				}
// GG120600(S) // Phase9 結果を区別する
//				result = remotedl_result_get(2);
				result = remotedl_result_get(RES_COMM);
// GG120600(E) // Phase9 結果を区別する
				if( result == PROG_DL_COMP ){
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[2] );		// "　　　　　 正常終了 　　　　　"
				}else{
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[3] );		// "　　　　　 異常終了 　　　　　"
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[4] );		// "　　（エラーコード：　　）　　"
					opedsp( 4, 20, result, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// エラーコード表示
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[5] );		// "接続に失敗しています。設定及び"
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[6] );		// "接続環境の確認が必要です。　　"
				}
#ifdef	CRW_DEBUG
				Lcd_WmsgDisp_OFF();	
#endif
				Fun_Dsp(FUNMSG[8]);									/* "　　　　　　　　　　　　 終了 " */
				remotedl_connect_req( OPLOG_REMOTE_CONNECT, CTRL_CONNECT_CHK );
				remotedl_chg_info_restore();						// 接続確認後は、ステータスを元に戻す
				break;
				
			default:
				break;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 共通パラメータアップロード確認画面                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : parameter_upload_chk( void )                            |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : 設定ﾓｰﾄﾞｽｲｯﾁOFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
#define	PARAM_UPLOAD_INVALID	0

ushort	parameter_upload_chk( void )
{
	
	ushort	msg,ret;

	if(!PARAM_UPLOAD_INVALID) {
		ret = MOD_EXT;
		if( CP_MODECHG ){
			ret = MOD_CHG;
		}
		return ret;
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[7] );								// "＜アップロード確認＞　　　　　"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[8] );								// "パラメータが変更されました。　"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[9] );								// "共通パラメータのアップロードを"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[10] );								// "行います。　　　　　　　　　　"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[11] );								// "よろしいですか？　　　　　　　"
	Fun_Dsp(FUNMSG[19]);													/* "　　　　　　 はい いいえ　　　" */

	while ( 1 ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	/* メンテナンスキー */
				BUZPI();
				return MOD_CHG;
								
			case KEY_TEN_F3:	/* はい(F3) */
				BUZPI();
				ret = ParameterUpload(1);
				if( CP_MODECHG == 0 ){
					ret = MOD_CHG;
				}
				return ret;

			case KEY_TEN_F4:	/* いいえ(F4) */
				BUZPI();
				ret = MOD_EXT;
				if( CP_MODECHG == 0 ){
					ret = MOD_CHG;
				}
				return ret;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| LAN接続確認				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lan_Connect_Chk( void )       	                       |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : 設定ﾓｰﾄﾞｽｲｯﾁOFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	Lan_Connect_Chk( void )
{
	ushort	msg, ret;
	int		ret2;
	short	in_dat = -1;
	short	in_ip[4] = {192, 168, 1, 1};
	short	pos = 0;
	short	retryCnt = 0;
	int		socketDescriptor = -1;

	wopelg( OPLOG_LAN_CONNECT_CHK, 0, 0 );									// 操作履歴登録

	Lan_Connect_dsp();
	ping_dsp( in_ip, pos );

	ret2 = 0;

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );
		if( ret2 ){
			if( msg == TIMEOUT6 ){											// 5秒タイムアウトで最大4回リトライ
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				if(retryCnt >= 4){
					dispmlclr(4,4);											// 画面表示ｸﾘｱ
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[4] );		// "　　　　　 異常終了 　　　　　"
					Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
				}
				else{
					retryCnt++;
					// ping開始
					socketDescriptor = pingStart(in_ip);
					Ope_DisableDoorKnobChime();
				}
				continue;
			}
			if( msg == KEY_TEN_F5 ){
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret2 = 0;
				Lan_Connect_dsp();
				ping_dsp( in_ip, pos );
				in_dat = -1;
				continue;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//			else if( msg == KEY_MODECHG ){
			else if( msg == KEY_MODECHG || msg == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					;
			}
			else{
				continue;
			}
		}

		switch( KEY_TEN0to9( msg ) )
		{
			case KEY_TEN_F5:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_CUT;
				break;			
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
				if(( in_dat != -1 ) && ( in_dat > 255 )){ 					// 入力値範囲ﾁｪｯｸ
					BUZPIPI();
					in_dat = -1;
					break;
				}
				if( in_dat != -1 ) {
					in_ip[pos] = in_dat;
					in_dat = -1;
				}
				BUZPI();
				// ｶｰｿﾙ位置を次の位置にｾｯﾄ
				if( msg == KEY_TEN_F2 ){
					pos = ( 3 < pos + 1 ) ? 0 : pos + 1;
				}
				else{
					pos = ( 0 > pos - 1 ) ? 3 : pos - 1;
				}
				break;
			case KEY_TEN:
				BUZPI();
				if( in_dat == -1 )
				{	// in_dat <= 入力値
					in_dat = (short)(msg - KEY_TEN0);
				}else{
					in_dat = (short)( in_dat % 100 ) * 10 + (short)(msg - KEY_TEN0);
				}
				// 入力値を表示する（反転表示）
				opedsp( 2, (pos*7)+1, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;
			case KEY_TEN_CL:
			case KEY_TEN_F3:
				BUZPI();
				in_dat = -1;
				break;
			case KEY_TEN_F4:
				if(( in_dat != -1 ) && ( in_dat > 255 )){ 					// 入力値範囲ﾁｪｯｸ
					BUZPIPI();
					in_dat = -1;
					break;
				}
				if( in_dat != -1 ){
					in_ip[pos] = in_dat;
				}
				// オールブロードキャストは禁止とする
				if((in_ip[0] == 255) && (in_ip[1] == 255) && (in_ip[2] == 255) && (in_ip[3] == 255)){
					BUZPIPI();
					in_dat = -1;
					break;
				}
				BUZPI();

				// ping開始
				dispmlclr(4,4);												// 画面表示ｸﾘｱ
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[2] );		// "　　　　 通信テスト中 　　　　"
				Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
				retryCnt = 0;
				socketDescriptor = pingStart(in_ip);
				Ope_DisableDoorKnobChime();
				ret2 = -1;
				break;
		}
		// ｶｰｿﾙ位置の数値を反転表示させる
		if(( msg == KEY_TEN_F1 ) || ( msg == KEY_TEN_F2 ) || ( msg == KEY_TEN_F3 ) || ( msg == KEY_TEN_CL )){
			ping_dsp( in_ip, pos );
		}
	}
	Lagcan( OPETCBNO, 6 );

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ping実行				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pingStart( short *dat )                                 |*/
/*| PARAMETER    : short *dat                                              |*/
/*| RETURN VALUE : socketDescriptor                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
int pingStart(short *dat)
{
	char	myIP[16];
	char	IPAddress[16];
	int		socketDescriptor;


	memset(myIP, 0x00, sizeof(myIP));
	sprintf(myIP, "%0d.%0d.%0d.%0d",
				prm_get(COM_PRM,S_MDL,2,3,4),
				prm_get(COM_PRM,S_MDL,2,3,1),
				prm_get(COM_PRM,S_MDL,3,3,4),
				prm_get(COM_PRM,S_MDL,3,3,1));

	memset(IPAddress, 0x00, sizeof(IPAddress));
	sprintf(IPAddress, "%0d.%0d.%0d.%0d",
			dat[0],
			dat[1],
			dat[2],
			dat[3]);
	if( (strcmp(IPAddress, "127.0.0.1") == 0) ||
		(strcmp(myIP, IPAddress) == 0) ){
		// ループバック
		socketDescriptor = tfPingOpenStart (IPAddress,				// IPアドレス
											0,						// PINGエコー要求の間隔（秒単位）
											32,						// PINGエコー要求のユーザデータ長
											(ttPingCBFuncPtr)0);	// PINGエコー応答
		if(socketDescriptor != -1){
			Lagcan( OPETCBNO, 6 );
			dispmlclr(4,4);											// 画面表示ｸﾘｱ
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[3] );		// "　　　　　 正常終了 　　　　　"
			Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
			tfPingClose(socketDescriptor);
		}
		else{
			Lagtim( OPETCBNO, 6, 50*5 );							// 5秒タイマーセット
		}
	}
	else{
		socketDescriptor = tfPingOpenStart (IPAddress,				// IPアドレス
											5,						// PINGエコー要求の間隔（秒単位）
											32,						// PINGエコー要求のユーザデータ長
											pingOpenCB);			// PINGエコー応答
		Lagtim( OPETCBNO, 6, 50*5 );								// 5秒タイマーセット
	}
	return socketDescriptor;
}

/*[]----------------------------------------------------------------------[]*/
/*| pingエコーコールバック				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pingOpenCB(int socketDescriptor)                        |*/
/*| PARAMETER    : int socketDescriptor                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void pingOpenCB(int socketDescriptor)
{
	int ret;
	ttPingInfo pingInfo;

	ret = tfPingGetStatistics(socketDescriptor, &pingInfo);
	if(ret == 0){
		Lagcan( OPETCBNO, 6 );
		dispmlclr(4,4);										// 画面表示ｸﾘｱ
		if( pingInfo.pgiTransmitted == pingInfo.pgiReceived ) {
			// 要求パケット数と応答パケット数が同じならば正常終了
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[3] );	// "　　　　　 正常終了 　　　　　"
		}
		else {
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[4] );	// "　　　　　 異常終了 　　　　　"
		}
		Fun_Dsp(FUNMSG[8]);														/* "　　　　　　　　　　　　 終了 " */
	}
	if(socketDescriptor != -1){
		tfPingClose(socketDescriptor);
	}
	if(!Ope_IsEnableDoorKnobChime()) {
		Ope_EnableDoorKnobChime();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| pingテスト画面表示					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lan_Connect_dsp( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void Lan_Connect_dsp( void )
{
	unsigned long	prm_41_0001_3;
	unsigned long	prm_41_0001_456;

	dispclr();

/* Display */
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[0] );		/* "＜ＬＡＮ接続チェック＞　　　　" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[1] );		/* "　　　．　　　．　　　．　　　" */
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[5] );		/* "通信種別：　　　　　　　　　　" */
	prm_41_0001_3 	= (unsigned long)prm_get(COM_PRM, S_MDL, 1, 1, 4);		//	41-0001-③
	prm_41_0001_456 = (unsigned long)prm_get(COM_PRM, S_MDL, 1, 3, 1);		//	41-0001-④⑤⑥
	if ((prm_41_0001_456 == 0) || (prm_41_0001_3 == 0)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[0] );		//	ＡＵＴＯ
	} else if ((prm_41_0001_456 == 10) && (prm_41_0001_3 == 1)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[1] );		//	10BASE-T(全二重)
	} else if ((prm_41_0001_456 == 10) && (prm_41_0001_3 == 2)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[2] );		//	10BASE-T(半二重)
	} else if ((prm_41_0001_456 == 100) && (prm_41_0001_3 == 1)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[3] );		//	100BASE-T(全二重)
	} else if ((prm_41_0001_456 == 100) && (prm_41_0001_3 == 2)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[4] );		//	100BASE-T(半二重)
	} else {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[2] );		//	10BASE-T(半二重)
	}

	Fun_Dsp(FUNMSG[9]);														/* "　⊂　　⊃　　　　 読出  終了 " */

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| pingテストIP表示					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ping_dsp( short * dat, short pos )                      |*/
/*| PARAMETER    : short * dat                                             |*/
/*|              : short pos                                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void ping_dsp( short * dat, short pos )
{
	ushort mod[4];												// 0(normal) / 1(reverse)

	memset(mod, 0x00, sizeof(mod));
	mod[pos] = 1;
	opedsp( 2, 1, (unsigned short)dat[0], 3, 0, mod[0], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 8, (unsigned short)dat[1], 3, 0, mod[1], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 15, (unsigned short)dat[2], 3, 0, mod[2], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 22, (unsigned short)dat[3], 3, 0, mod[3], COLOR_BLACK, LCD_BLINK_OFF );

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			フラップセンサー操作表示
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort FunChk_FlapSensorControl( void )
{
	ulong	ulwork;
	ushort	i,dsp=1;
	ushort	msg, command;
	short	tnoIndex[LOCK_IF_MAX]={0};
	short	roomNo;
	ushort	index=0;
	ushort	pageIndex;
	ushort	page, oldPage;
	ushort	drawCount;
	ushort	CNTMAX;
	
	for ( ; ; ) {

		if( dsp == 1 ){
			/* 初期画面表示 */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[1]);		/* "＜車両検知センサー操作＞　　　" */
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[2]);		/* "　車室№              状態　　" */
			Fun_Dsp( FUNMSG[103] );						// "  ＋  －／読  OFF   ON   終了 "

			CNTMAX = 0;
			for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
				WACDOG;										// 装置ループの際はWDGリセット実行
				if( (LockInfo[i].if_oya != 0) && (LockInfo[i].lok_syu != 0) ){	// ターミナルNo一致かつ接続あり？
					if(1 == LKcom_Search_Ifno(LockInfo[i].if_oya)) {
						tnoIndex[CNTMAX] = i;
						CNTMAX++;
					}
				}
			}
			
			if( !CNTMAX ){								// 表示対象が１件も無ければ戻る
				BUZPIPI();
				return MOD_EXT;
			}
			dsp++;
			page = 0;
			oldPage = 1;								// 描画させるため1を設定

		}
		else {
		
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
				return MOD_CHG;

			case KEY_TEN_F5:	/* "終了" */
				BUZPI();
				Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
				if( dsp == 4 ){
					dsp = 1;
					break;
				}
				return MOD_EXT;

			case KEY_TEN_F1:	// F1:"▲"
				if( dsp > 2 ) {
					break;
				}
				BUZPI();
				if( CNTMAX == 1 ) {
					break;
				}
				if( index <= 0 ){
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転解除）
					index = (char)(CNTMAX - 1);
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転）
					}
				}else{
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転解除）
					--index;
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5)+ 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転）
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"▼"
				if( dsp > 2 ) {
					break;
				}
				BUZPI();
				if( CNTMAX == 1 ) {
					break;
				}
				if( index >= CNTMAX - 1 ){
					opedsp( (ushort)((index % 5)+ 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転解除）
					index = 0;
					page = 0;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転）
					}
				}else{
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転解除）
					++index;
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示（反転）
					}
				}
				break;

			case KEY_TEN_F3:	// F3:"OFF"
			case KEY_TEN_F4:	// F4:"ON"
				if( dsp > 2 ) {
					break;
				}

				// 内蔵ループセンサが未接続は強制OFF/ON不可しない
				if(IFM_FlapSensor[LockInfo[tnoIndex[index]].if_oya - 1].c_LoopSensor == 0) {
					BUZPIPI();
					break;
				}
				i = (ushort)LockInfo[tnoIndex[index]].if_oya - 1;
				if( IsErrorOccuerd( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_COMFAIL + i ) ){	// 対象端末通信正常
					BUZPIPI();
					break;
				}
				if( LK_TYPE_CONTACT_FLAP == LockInfo[tnoIndex[index]].lok_syu ){	// 接点フラップの場合、強制ON/OFFはさせない
					BUZPIPI();
					break;
				}
				BUZPI();
			
				ulwork = (ulong)(( LockInfo[tnoIndex[index]].area * 10000L ) + LockInfo[tnoIndex[index]].posi );
				// 表示変更
				if(msg == KEY_TEN_F3) {
					command = LK_SND_FORCE_LOOP_OFF;
					grachr( (ushort)(index%5+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[48] );			// [48] 強制OFF
					wopelg( OPLOG_FLAP_SENSOR_OFF, 0, ulwork );	// 操作履歴登録
				}
				else {
					command = LK_SND_FORCE_LOOP_ON;
					grachr( (ushort)(index%5+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[49] );			// [49] 強制ON
					wopelg( OPLOG_FLAP_SENSOR_ON, 0, ulwork );	// 操作履歴登録
				}

				// コマンドを送信
				roomNo = tnoIndex[index] + 1;
				queset( FLPTCBNO, command, sizeof(tnoIndex[0]), &roomNo );
			
				break;

			default:
				break;
			}
		}
		
		if(page != oldPage) {
			oldPage = page;
			pageIndex = page * 5; 
			for(drawCount = 0; drawCount < 5; ++drawCount) {
				if(pageIndex + drawCount < CNTMAX) {
					if(index != (pageIndex + drawCount)) {
						opedsp( (ushort)(drawCount + 2), 2,
								(ushort)LockInfo[tnoIndex[pageIndex + drawCount]].posi, 4, 1, 0,
								COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転解除)
					}
					else {
						opedsp( (ushort)(drawCount + 2), 2,
								(ushort)LockInfo[tnoIndex[pageIndex + drawCount]].posi, 4, 1, 1,
								COLOR_BLACK, LCD_BLINK_OFF );	// ﾀｰﾐﾅﾙ№表示(反転)
					}
					i = (ushort)LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1;
					if(	!IsErrorOccuerd( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_COMFAIL + i ) ){	// 対象端末通信正常
						switch(IFM_FlapSensor[LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1].c_LoopSensor) {
						case '1':			// OFF状態
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[45] );		// [45] "ＯＦＦ　"
							break;
						case '2':			// ON状態
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[46] );		// [46] "ＯＮ　　"
							break;
						case '3':			// 異常
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[47] );		// [47] "異常　　"
							break;
						case '4':			// 強制OFF状態
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[48] );		// [48] "強制OFF "
							break;
						case '5':			// 強制ON状態
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[49] );		// [49] "強制ON　"
							break;
						case '6':			// 不明状態
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[50] );		// [50] "不明　　"
							break;
						default:
							switch(IFM_FlapSensor[LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1].c_ElectroSensor) {
							case '1':			// OFF状態
							case '2':			// ON状態
								grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[51] );		// [51] "　－　　"
								break;
							default:
								grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[52] );		// [52] "未接続　"
								break;
							}
						}
					}
					else {
						// 通信異常中は「未接続」を表示する
						grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[52] );		// [52] "未接続　"
					}
				}
				else {
					displclr((ushort)(drawCount+2));				// 装置が無い場合は行をクリアする
				}
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			フラップループデータ表示
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	FunChk_FlapLoopData( void )
{
	T_FrmAnyData	FrmChkData;
	ulong	ulData;
	ushort	i;
	ushort	msg;
	short	tnoIndex[LOCK_IF_MAX]={0};
	uchar	type;
	ushort	index = 0;
	ushort	count = 0;
	uchar	timeout = 1;
	uchar	priend = 0;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[0]);		// [00] "＜ループデータ＞　　　　　　　"
	grachr(3, 0, 30, 0 ,COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"　　 しばらくお待ち下さい 　　"
	Fun_Dsp(FUNMSG[0]);														/* [0]	"　　　　　　　　　　　　　　　" */
	
	for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
		WACDOG;											// 装置ループの際はWDGリセット実行
		if( (LockInfo[i].if_oya != 0) && (LockInfo[i].lok_syu != 0) ){	// ターミナルNo一致かつ接続あり？
			tnoIndex[count] = i;
			++count;
		}
	}

	if( !(GetCarInfoParam() & 0x04) || count == 0 ){	// フラップ設定なし
		BUZPIPI();
		return MOD_EXT;
	}
	
	type = _MTYPE_INT_FLAP;
	queset( FLPTCBNO, LK_SND_A_LOOP_DATA, 1, &type );	// ループデータ要求送信
	Ope_DisableDoorKnobChime();
	
	Lagtim( OPETCBNO, 6, 6*50 );						// ﾀｲﾏｰ6(6s)起動(ﾒﾝﾃﾅﾝｽ制御用)
	
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			return MOD_CHG;

		case KEY_TEN_F5:	/* "終了" */
			if(timeout == 1){
				break;
			}
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
			return MOD_EXT;


		case KEY_TEN_F3:	// F3:"ﾌﾟﾘﾝﾄ"
			if(timeout == 1){
				break;
			}
			if(Ope_isPrinterReady() == 0){			// レシートプリンタが印字不可能な状態
				BUZPIPI();
				break;
			}
			BUZPI();
			FrmChkData.Kikai_no = (uchar)CPrmSS[S_PAY][2];
			FrmChkData.prn_kind = R_PRI;
			queset( PRNTCBNO, PREQ_LOOP_DATA, sizeof(FrmChkData), &FrmChkData);
			Ope_DisableDoorKnobChime();
			Fun_Dsp(FUNMSG[82]);		// Fｷｰ表示　"　　　　　　 中止 　　　　　　"
			
			for ( priend = 0 ; priend == 0 ; ) {
				msg = StoF( GetMessage(), 1 );
				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
					msg &= (~INNJI_ENDMASK);
				}
				switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case LCD_DISCONNECT:
					Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
					return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case KEY_MODECHG:		// ﾓｰﾄﾞﾁｪﾝｼﾞ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					Lagcan( OPETCBNO, 6 );										// ﾀｲﾏｰ6ｷｬﾝｾﾙ
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
					BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//					return 1;
					return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				case PREQ_LOOP_DATA:	// 印字終了
					priend = 1;
					break;
				case KEY_TEN_F3:		// F3ｷｰ（中止）
					BUZPI();
					FrmChkData.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(FrmChkData), &FrmChkData );	// 印字中止要求
					priend = 1;
					break;
				default:
					break;
				}
			}
			Fun_Dsp( FUNMSG[104] );						// [104]"  ＋  　－　 ﾌﾟﾘﾝﾄ 　　  終了 "
			break;
			
		case KEY_TEN_F4:	// F4:
			break;

		case KEY_TEN_F1:	// F1:"＋"
		case KEY_TEN_F2:	// F2:"－"
			if(timeout == 1){
				break;
			}
		case TIMEOUT6:		// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ(ﾒﾝﾃﾅﾝｽ制御用)
			if(msg == KEY_TEN_F2) {
				BUZPI();
				if(index < count - 1) {
					++index;
				}
				else {
					index = 0;			// 先頭に戻る
				}
			}
			else if(msg == KEY_TEN_F1) {
				BUZPI();
				if(index != 0) {
					--index;
				}
				else {
					index = count - 1;	// 末尾に戻る
				}
			}
			else if(msg == TIMEOUT6) { 
				timeout = 0;
				index = 0;
				Fun_Dsp( FUNMSG[104] );						// [104]"  ＋  　－　 ﾌﾟﾘﾝﾄ 　　  終了 "
			}

			grachr( 1, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[1] );			// [01]	"　車室番号：　　　　　　　　　"
			opedsp( 1, 10, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
												COLOR_BLACK, LCD_BLINK_OFF );		// 車室番号表示

			// 各データのタイトル部分を表示
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[3]);
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[4]);
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[5]);
			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[6]);
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[7]);

			// 各データの値を表示
			// '0'～'9' 'A'～'F'以外があれば"＊＊＊＊"を表示する
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucLoopCount, 4, &ulData)) {
				opedsp5(2, 22, (ushort)ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// ループカウンタ表示
			}
			else {
				grachr(2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "＊＊＊＊"
			}

			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOffBaseCount, 4, &ulData)) {
				opedsp5(3, 22, (ushort)ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// オフベース表示
			}
			else {
				grachr(3, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "＊＊＊＊"
			}
				
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOffLevelCount, 4, &ulData)) {
				opedsp5(4, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// オフレベル表示
			}
			else {
				grachr(4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "＊＊＊＊"
			}
			
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOnBaseCount, 4, &ulData)) {
				opedsp5(5, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// オンベース表示
			}
			else {
				grachr(5, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "＊＊＊＊"
			}
			
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOnLevelCount, 4, &ulData)) {
				opedsp5(6, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// オンレベル表示
			}
			else {
				grachr(6, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		// "＊＊＊＊"
			}

			break;

		default:
			break;
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			デバッグ用メニュー
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FucChk_DebugMenu(void)
{
	unsigned short	usFbruEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for ( ; ; ) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNT_STRCHK_STR[1] );		/* 	"＜デバッグメニュー＞　　　　　" */

		usFbruEvent = Menu_Slt( MNT_STRCHK_STR, MNT_STRCHK_TBL, (char)MNT_STRCHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usFbruEvent) {
		case MNTLOG1_CHK:						/* 設定＆ステータスプリント */
			if (Ope_isPrinterReady() == 0) {
				BUZPIPI();
				break;
			}
			usFbruEvent = Fchk_Stack_StsPrint();
			break;
		case MOD_EXT:
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return usFbruEvent;
			break;
		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (usFbruEvent == MOD_CHG) {
		if (usFbruEvent == MOD_CHG || usFbruEvent == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			return usFbruEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			スタック使用量プリント
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	Fchk_Stack_StsPrint( void )
{
	T_FrmLogPriReq1	MntStak_PriReq;		// 印字要求ﾒｯｾｰｼﾞﾜｰｸ
	T_FrmPrnStop	MntStak_PrnStop;	// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
	ushort			pri_cmd = 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	ushort	msg;

	dispclr();									// 初期画面（印字中）表示
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[2]);	/* "＜設定＆ステータスプリント＞  " */
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)のための定義移動
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,EDY_STSPRINT_STR[2]);	/* "　　　　 印字中です         　" */
//	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,EDY_STSPRINT_STR[3]);	/* "　　中止：印字を中止します　　" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[3]);	/* "　　　　 印字中です         　" */
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[4]);	/* "　　中止：印字を中止します　　" */
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)のための定義移動
	Fun_Dsp(FUNMSG[82]);						/* "　　　　　　 中止 　　　　　　" */
	
	Fchk_Stak_chek();
	
	MntStak_PriReq.prn_kind = R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
	MntStak_PriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// 機械№	：設定ﾃﾞｰﾀ
	MntStak_PriReq.Kakari_no = OPECTL.Kakari_Num;			// 係員No.
	memcpy( &MntStak_PriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );// 印字時刻	：現在時刻
	
	/* 印字要求*/
	queset( PRNTCBNO, PREQ_MNT_STACK, sizeof(T_FrmLogPriReq1), &MntStak_PriReq );
	Ope_DisableDoorKnobChime();

	pri_cmd = PREQ_MNT_STACK;							// 送信ｺﾏﾝﾄﾞｾｰﾌﾞ
	
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){		// 印字開始前
			switch( msg ){		/* FunctionKey Enter */

				case KEY_TEN_F3:	/* "中止" */
					BUZPI();
					MntStak_PrnStop.prn_kind = R_PRI;			// 印字中止ﾒｯｾｰｼﾞ送信
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &MntStak_PrnStop );
					return MOD_EXT;
					
				default:
					break;
			}
		}
		else{		// 印字開始後（印字終了待ち画面）
			switch( msg ){		/* FunctionKey Enter */
				case KEY_TEN_F3:	/* "中止" */
					BUZPI();
					MntStak_PrnStop.prn_kind = R_PRI;				// 印字中止ﾒｯｾｰｼﾞ送信
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &MntStak_PrnStop );
					break;
			}

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){			// 印字終了ﾒｯｾｰｼﾞ受信？
				return MOD_EXT;									// YES：前画面に戻る
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			スタック使用量算出
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Fchk_Stak_chek( void )
{
	
	unsigned short	i,j;				// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	unsigned char	*chk_adr;			// ﾁｪｯｸﾒﾓﾘｰｱﾄﾞﾚｽ
	unsigned char	*chk_adr_s;			// ﾁｪｯｸﾒﾓﾘｰｽﾀｰﾄｱﾄﾞﾚｽ
	unsigned char	chk_stop = 0;		// ﾁｪｯｸ結果
	unsigned char	chk_data1;			// ﾁｪｯｸﾃﾞｰﾀ１

	uchar			stak_not = 0xED;	// 比較用データ

	chk_adr_s = (unsigned char *)xSTK_RANGE_BTM;
	
	// STACKチェック
	for(j = 0;j<TSKMAX;j++){						// タスクサイズ分ループ
		chk_adr_s -= xSTACK_TBL[j];					// スタートアドレスセット
		chk_adr = chk_adr_s;
		for(i = 0 ; i < xSTACK_TBL[j] ; i++){		// スタックサイズ分ループ
			WACDOG;									// WATCHDOGﾀｲﾏｰﾘｾｯﾄ
			
			_di();									// 割込み禁止
			
			chk_data1 = *chk_adr;					// データセット
			
			if(chk_data1 != stak_not){				// !0xED
				Pri_Tasck[j][0] = (ushort)(xSTACK_TBL[j]-i);				// スタック使用量の計算
				Pri_Tasck[j][1] = (ushort)(Pri_Tasck[j][0]/(xSTACK_TBL[j]/100));			// スタック使用量の計算（％）
				chk_stop = 1;
			}
			chk_adr++;
			
			_ei();									// 割込み禁止解除
			
			if(chk_stop == 1){						// スタック使用量の計算終了の為、次のスタック計算へ
				chk_stop = 0;
				break;
			}
		}
	}
	// 精算、集計データエリアサイズ
	Pri_Pay_Syu[0][0] = sizeof(Receipt_data);		// 精算データサイズ（1件）
	Pri_Pay_Syu[1][0] = sizeof(Syu_log);			// 集計データサイズ（1件）
	Pri_Pay_Syu[0][1] = ( (LogDatMax[eLOG_PAYMENT][0]*LOG_SECORNUM(eLOG_PAYMENT)) +					// 精算ログサイズ(RAM + FLASH(2MByte))
						  (((2000*1024)/LogDatMax[eLOG_PAYMENT][0])*LogDatMax[eLOG_PAYMENT][0]) ); 
	Pri_Pay_Syu[1][1] = ( (LogDatMax[eLOG_TTOTAL][0]*LOG_SECORNUM(eLOG_TTOTAL)) +					// 集計ログサイズ(RAM + FLASH(384KByte)) 
						  (((384*1024)/LogDatMax[eLOG_TTOTAL][0])*LogDatMax[eLOG_TTOTAL][0]) );		
	Pri_program = AUTO_PAYMENT_PROGRAM;				// デバッグ用プログラムモード
	
}
//[]----------------------------------------------------------------------[]
///	@brief			FAN動作チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/05/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	FunChk_FUNOPECHK( void )
{
	ushort	msg;
	ushort	ret			= 0;											// 戻り値
	uchar state = 0; //0:OFF 1:ON
	dispclr();															// 画面ｸﾘｱ
	
	CP_FAN_CTRL_SW = state;

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNOPECHKSTR[0] );	// "＜ＦＡＮ動作チェック＞　　　　"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNOPECHKSTR[1] );	// "  現在の状態 →               "
	grachr( 2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[0] );// OFF

	Fun_Dsp( FUNMSG[50] );												// "　　　　　　　　　ON/OFF 終了 ", // [50]

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );									// ﾒｯｾｰｼﾞ受信

		switch( msg ){													// 受信ﾒｯｾｰｼﾞ
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:											// 操作ﾓｰﾄﾞ切替
				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:											// Ｆ５（終了）
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN_F4:											// Ｆ４
				BUZPI();
				if(state == 0){											// OFF
					state = 1;// ONへ
				}else{													// ON
					state = 0;// OFFFへ
				}
				grachr( 2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[state] );// OFF
				CP_FAN_CTRL_SW = state;
				break;

			default:													// その他
				break;
		}
	}
	if(f_fan_timectrl == 1){											// FAN強制停止中
		CP_FAN_CTRL_SW = 0;												// 元の状態は停止
	}else{
		CP_FAN_CTRL_SW = fan_exe_state;									// 通常制御時の状態に戻す
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Park i PROチェック処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CCom( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_PiP( void )
{
	unsigned short	fncChkPiPEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	// Park i PRO有効チェック？
	if (_is_not_pip()) {
		BUZPIPI();
		return MOD_EXT;
	}

	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  FCSTR1[47] );		/* "＜Park i PROチェック＞　　　　" */
		fncChkPiPEvent = Menu_Slt( PIPMENU, PIP_CHK_TBL, PIP_CHK_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (fncChkPiPEvent) {

		case PIP_DTCLR:		// データクリア
			fncChkPiPEvent = FncChk_pipclr();
			break;

		case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return fncChkPiPEvent;

		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if (fncChkPiPEvent == MOD_CHG) {
		if ( fncChkPiPEvent == MOD_CHG || fncChkPiPEvent == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return fncChkPiPEvent;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Park i PROチェック処理　データクリア		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_pipclr( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort	FncChk_pipclr(void)
{
	ushort msg;
	dispmlclr(1, 2);
	grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );	// "全"
	grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );			// "データを消去します"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );			// "　　　　よろしいですか？　　　"
	Fun_Dsp(FUNMSG[19]);						// "　　　　　　 はい いいえ　　　"

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F3:		// 「はい」
			IFM_RcdBufClrAll();		// データテーブルの初期化
			/* not break; */
		case KEY_TEN_F4:		// 「いいえ」
			BUZPI();
			return MOD_EXT;
			
		default:
			break;
		}
	}
}
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
///*[]----------------------------------------------------------------------[]*/
///*| クレジットチェック処理                                                 |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_Cre( void )                                      |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : unsigned short                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//ushort FncChk_Cre( void )
//{
//	unsigned short	fncChkCreEvent;
//	char	wk[2];
//	char	org[2];
//
//	org[0] = DP_CP[0];
//	org[1] = DP_CP[1];
//
//	// クレジット有効？
//	if( ! CREDIT_ENABLED() ){
//		BUZPIPI();
//		return MOD_EXT;
//	}
//
//	DP_CP[0] = DP_CP[1] = 0;
//	for ( ; ; ) {
//
//		dispclr();
//		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[56] );		// "＜クレジットチェック＞　　　　"
//
//		fncChkCreEvent = Menu_Slt( CREMENU, CRE_CHK_TBL, (char)CRE_CHK_MAX, (char)1 );
//		wk[0] = DP_CP[0];
//		wk[1] = DP_CP[1];
//
//		switch (fncChkCreEvent) {
//		case CREDIT_UNSEND:
//			fncChkCreEvent = FncChk_CreUnSend();	// 未送信売上データ
//			break;
//
//		case CREDIT_SALENG:
//			fncChkCreEvent = FncChk_CreSaleNG();	// 売上拒否データ
//			break;
//
//		case MOD_EXT:								// 終了（Ｆ５）
//			DP_CP[0] = org[0];
//			DP_CP[1] = org[1];
//			return fncChkCreEvent;
//
//		default:
//			break;
//		}
//
//		if (fncChkCreEvent == MOD_CHG){				// モードチェンジ
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			OPECTL.PasswordLevel = (char)-1;
//			return fncChkCreEvent;
//		}
//		DP_CP[0] = wk[0];
//		DP_CP[1] = wk[1];
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット未送信売上依頼データ         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_CreUnSend(void)                                  |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	FncChk_CreUnSend( void )
//{
//	ushort	msg;
//	uchar	mode;
//	ushort	rev = 0;	//反転表示用
//	short	ret;
//	char	repaint;	// 画面再描画フラグ
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//	char	print_flg = 0;
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
//				Fun_Dsp(FUNMSG2[59]);							/* " 削除 　　　 ﾌﾟﾘﾝﾄ 再送  終了 " */
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
//				if( key_flg == 0 ){
//					break;
//				}
//				if( mode == 1 ){		// 画面からの送信中
//					break;
//				}
//				
//				if(cre_ctl.Status != CRE_STS_IDLE) {	// 再送タイマにより送信中
//					BUZPIPI();
//					displclr(3);
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "　　　　　異常終了　　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "　　現在クレジット処理中。　　" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "　ビジーのため実行できません。" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;
//					break;
//				}
//
//				// 削除
//				BUZPI();
//				ret = cre_unsend_del_dsp(print_flg);
//				switch( ret ){
//				case  1:	// モードチェンジ
//					Lagcan(OPETCBNO, 7);
//					return( MOD_CHG );
//				case -1:	// はい
//					Lagcan(OPETCBNO, 7);
//					return MOD_EXT;
//				case  0:	// いいえ
//				default:
//					break;
//				}
//				repaint = 1;
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
//						BUZPIPI();
//						repaint = 1;
//						key_flg = 0;
//					}
//					break;
//				}
//				BUZPI();
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// 印字時刻	：現在時刻
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// 売上依頼ﾃﾞｰﾀ
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// 印字要求
//
//				print_flg = 1;							// 印字フラグON(印字できなかった場合でも印字したことにする)
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
//				repaint = 1;
//				break;
//
//			case KEY_TEN_F4:			// 送信
//				if( key_flg == 0 ){
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// 未送信データなし
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "　　　　　 正常終了 　　　　　" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "　　　 送信完了しました 　　　" */
//					Fun_Dsp(FUNMSG[8]);													/* "　　　　　　　　　　　　 終了 " */
//					mode = 2;	// 送信済み
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
///*|  クレジット未送信売上依頼データ 削除画面表示                           |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_unsend_del_dsp                                      |*/
///*| RETURN VALUE : short  0:normal                                         |*/
///*| RETURN VALUE : short  1:mode change                                    |*/
///*| RETURN VALUE : short -1:delete ok                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static short cre_unsend_del_dsp( char pri )
//{
//	ushort	msg;
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[160]);	/* "＜未送信売上データ＞　　　　　" */
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);	// "　　　データを削除します　　　"
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);	// "　　　　よろしいですか？　　　"
//	Fun_Dsp(FUNMSG[19]);											// "　　　　　　 はい いいえ　　　"
//
//	for ( ; ; ) {
//
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F3:			// F3:はい
//			BUZPI();
//
//			if( cre_uriage.UmuFlag == OFF ) {		// 未送信データなし
//				return -1;
//			}
//			
//			// 削除前に印字する
//			if(pri == 0 && Ope_isPrinterReady()) {				// レシートに印字可能
//				memset(&unsend_pri, 0, sizeof(unsend_pri));
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// 印字時刻	：現在時刻
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// 売上依頼ﾃﾞｰﾀ
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// 印字要求
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
//			
//			// 削除
//			cre_uriage.UmuFlag = OFF;						// 売上依頼データ(05)送信中ﾌﾗｸﾞOFF
//			memset( &cre_uriage, 0, sizeof(cre_uriage));	// 売上依頼データエリアをクリア
//			creSales_Reset();								// タイマカウント、リトライ情報を削除する
//
//			if( creErrorCheck() == 0 ){
//				cre_ctl.Credit_Stop = 0;	// クレジット停止要因がなくなったら解除する
//			}
//
//			wopelg( OPLOG_CRE_UNSEND_DEL, 0, 0 );			// 未送信売上削除
//			return( -1 );
//		case KEY_TEN_F4:			// F4:いいえ
//			BUZPI();
//			if( cre_uriage.UmuFlag == OFF ) {		// 未送信データなし
//				return( -1 );
//			}
//			return( 0 );
//		case KEY_MODECHG:			// モードチェンジ
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ユーザーメンテナンス：拡張機能-クレジット売上拒否データ               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_CreSalesNG( void )                               |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	FncChk_CreSaleNG( void )
//{
//	ushort	msg;
//	char	repaint;	// 画面再描画フラグ
//	short	show_index;	// 表示中のインデックス
//	char	data_count;	// 更新後のデータ件数
//	short	ret;
//	T_FrmSaleNGPriReq	saleng_pri;
//	char	print_flg = 0;
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
//				print_flg = 1;							// 印字フラグON(印字できなかった場合でも印字したことにする)
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
//			if (data_count != 0) {	// データ数が1以上の時有効
//				// 削除
//				BUZPI();
//				ret = cre_saleng_del_dsp( show_index , print_flg);
//				if (ret == 1) {
//					return( MOD_CHG );
//				} else if (ret == -1) {
//					if (show_index >= cre_saleng.ken) {
//						--show_index;
//					}
//				}
//				repaint = 1;
//			}
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
///*| MODULE NAME  : FncChk_CreSaleNG_fnc(char ken)                          |*/
///*| PARAMETER    : char ken : 件数                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void	FncChk_CreSaleNG_fnc( char ken )
//{
//	if( ken > 1 ){	// 2件以上
//		Fun_Dsp(FUNMSG2[30]);						// "　▲　　▼　 ﾌﾟﾘﾝﾄ 削除  終了 "
//	} else {		// 2件未満
//		Fun_Dsp(FUNMSG2[31]);						// "　　　　　　 ﾌﾟﾘﾝﾄ 削除  終了 "
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
//		FncChk_CreSaleNG_fnc( cre_saleng.ken );
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
///*[]----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ 削除画面表示                                 |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_del_dsp                                      |*/
///*| PARAMETER    : index 削除対象                                          |*/
///*| RETURN VALUE : short  0:normal                                         |*/
///*| RETURN VALUE : short  1:mode change                                    |*/
///*| RETURN VALUE : short -1:delete ok                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static short cre_saleng_del_dsp( short index, char pri)
//{
//	ushort	msg;
//	T_FrmSaleNGPriReq	saleng_pri;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[161]);	// "＜クレジット売上拒否データ＞　"
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);	// "　　　データを削除します　　　"
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);	// "　　　　よろしいですか？　　　"
//	Fun_Dsp(FUNMSG[19]);											// "　　　　　　 はい いいえ　　　"
//
//	for ( ; ; ) {
//
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F3:			// F3:はい
//			BUZPI();
//
//			// 削除時に印字する
//			if(pri == 0 && Ope_isPrinterReady()) {	// レシートに印字可能
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
//
//			// 全件削除
//			CRE_SaleNG_Delete();
//			if( cre_saleng.ken == 0 ){	// 削除後 0件 ならエラー解除
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x00, 0, 0, NULL );	// エラー(解除)
//			}
//			cre_ctl.Credit_Stop = 0;	// クレジット停止要因がなくなったら解除する
//			wopelg( OPLOG_CRE_SALE_NG_DEL, 0, 0 );		// 操作履歴登録
//			displclr(2);
//			displclr(3);
//			return( -1 );
//		case KEY_TEN_F4:			// F4:いいえ
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:			// モードチェンジ
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ  データ削除                                   |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : CRE_SaleNG_Delete                                        |*/
///*| PARAMETER    : none                                                     |*/
///*| RETURN VALUE : none                                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                               |*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2013 AMANO Corp.---[]*/
//static void	CRE_SaleNG_Delete( void )
//{
//	memset(&cre_saleng_work, 0, sizeof(CRE_SALENG));
//
//	nmisave(&cre_saleng, &cre_saleng_work, sizeof(CRE_SALENG));
//}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

// GG120600(S)
/*[]----------------------------------------------------------------------[]*/
/*|  ユーザーメンテナンス：遠隔監視データ画面表示                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  :dsp_time_info( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
// GG120600(E)
static void dsp_time_info(uchar type)
{
	ushort col = 2;
	t_remote_time_info *pTimeInfo;
	uchar  tmpStr[12];

	if (type >= TIME_INFO_MAX) {
		return;
	}
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[9+type] );	// タイトル

	// 時刻情報を取得
	pTimeInfo = remotedl_time_info_get(type);

	switch (type) {
	case PROG_DL_TIME:
	case PARAM_DL_TIME:
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	case PARAM_DL_DIF_TIME:		// 差分パラメータダウンロード
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)	
// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
//		if (!remote_date_chk(&pTimeInfo->start_time) || !remote_date_chk(&pTimeInfo->sw_time)) {
		if (!Check_date_time_rec(&pTimeInfo->start_time) || !Check_date_time_rec(&pTimeInfo->sw_time)) {
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "予定はありません　　　　　　　"
			return;
		}

		// 開始時刻
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->start_time.Year%100, pTimeInfo->start_time.Mon, pTimeInfo->start_time.Day,
			pTimeInfo->start_time.Hour, pTimeInfo->start_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[0] );		// "開始時刻：　年　月　日　：　　"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// 年
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// 月
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// 日
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// 時
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// 分
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_START]] );	// 処理ステータス
		col++;

		// 更新時刻
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->sw_time.Year%100, pTimeInfo->sw_time.Mon, pTimeInfo->sw_time.Day,
			pTimeInfo->sw_time.Hour, pTimeInfo->sw_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[1] );		// "更新時刻：　年　月　日　：　　"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// 年
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// 月
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// 日
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// 時
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// 分
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_SW]] );	// 処理ステータス
		col++;
		break;
	case PARAM_UP_TIME:
	case RESET_TIME:
// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
//		if (!remote_date_chk(&pTimeInfo->start_time)) {
		if (!Check_date_time_rec(&pTimeInfo->start_time)) {
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "予定はありません　　　　　　　"
			return;
		}

		// 開始時刻
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->start_time.Year%100, pTimeInfo->start_time.Mon, pTimeInfo->start_time.Day,
			pTimeInfo->start_time.Hour, pTimeInfo->start_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[0] );		// "開始時刻：　年　月　日　：　　"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// 年
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// 月
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// 日
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// 時
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// 分
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_START]] );	// 処理ステータス
		col++;

		break;
	case PROG_ONLY_TIME:
// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
//		if (!remote_date_chk(&pTimeInfo->sw_time)) {
		if (!Check_date_time_rec(&pTimeInfo->sw_time)) {
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "予定はありません　　　　　　　"
			return;
		}

		// 更新時刻
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->sw_time.Year%100, pTimeInfo->sw_time.Mon, pTimeInfo->sw_time.Day,
			pTimeInfo->sw_time.Hour, pTimeInfo->sw_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[1] );		// "更新時刻：　年　月　日　：　　"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// 年
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// 月
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// 日
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// 時
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// 分
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_SW]] );	// 処理ステータス
		col++;

		break;
	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| センター通信チェック処理　遠隔ダウンロード確認                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CComRemoteDLChk(void)                            |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_CComRemoteDLChk(void)
{
	ushort	msg;
	uchar	mode;		// 0:時刻確認画面/ 1:クリア確認画面
	uchar	dsp;		// 0:描画なし/ 1:画面表示
	int		page;		// ページ番号
// GG120600(S) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
	u_inst_no*		pinstTemp = NULL;
	u_inst_no		instTemp;
// GG120600(E) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)

	dispclr();

	dsp		= 1;
	mode	= 0;
	page	= PROG_DL_TIME;

	for ( ; ; ) {
		if (dsp) {
			if (mode == 0) {
				if (dsp == 1) {
					dsp_time_info(page);
					Fun_Dsp(FUNMSG[118]);				// "　⊂　　⊃　クリア       終了 "
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)		
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:		// 「終了」
			if (mode == 0) { 
				BUZPI();
				return MOD_EXT;
			}
			break;

		case KEY_TEN_F1:		// ページ移動（←）
			if (mode == 0) {	// 0:時刻確認画面/ 1:クリア確認画面
				BUZPI();

				if (--page < 0) {
					page = TIME_INFO_MAX - 1;
				}
				dispclr();
				dsp = 1;
			}
			break;
		case KEY_TEN_F2:		// ページ移動（→）
			if (mode == 0) {	// 0:時刻確認画面/ 1:クリア確認画面
				BUZPI();

				if (++page >= TIME_INFO_MAX) {
					page = PROG_DL_TIME;
				}
				dispclr();
				dsp = 1;
			}
			break;

		case KEY_TEN_F3:		// クリア
			switch (mode) {
			case 0:				// 時刻確認画面
				BUZPI();
				dispmlclr(1, 6);
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[2] );							// "　　時刻情報をクリアします　　"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
				Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
				mode = 1;		// 0:時刻確認画面/ 1:クリア確認画面
				break;
			case 1:				// クリア確認画面
				BUZPI();

// GG120600(S) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
//// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
////				remotedl_time_info_clear(page);
//				remotedl_time_info_clear(page,remotedl_instNo_get(),remotedl_instFrom_get());
//// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる
				// 途中でクリアされるため、この時点でのを保持して渡す
				pinstTemp = remotedl_instNo_get_with_Time_Type(page);
				memcpy(&instTemp,pinstTemp,sizeof(u_inst_no));
				remotedl_time_info_clear(page,&instTemp,remotedl_instFrom_get_with_Time_Type(page));
// GG120600(E) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
				dispmlclr(1, 6);
				Fun_Dsp(FUNMSG[118]);																	// "　⊂　　⊃　クリア       終了 "
				dsp  = 1;		// 画面再描画
				mode = 0;		// 0:時刻確認画面/ 1:クリア確認画面
				break;
			default:
				break;
			}
			break;

		case KEY_TEN_F4:
			switch (mode) {		// 0:時刻確認画面/ 1:クリア確認画面
			case 1:				// クリア確認画面
				BUZPI();
				dispmlclr(1, 6);
				dsp  = 1;		// 画面再描画
				mode = 0;		// 0:時刻確認画面/ 1:クリア確認画面
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェック(長期駐車データによる検出)
//[]----------------------------------------------------------------------[]
///	@return     ret    MOD_CHG/MOD_EXT
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
ushort	FncChk_CComLongParkAllRel( void )
{
	ushort	msg,i;

	dispclr();
// MH321800(S) S.Takahashi 2020/05/12 遠隔ダウンロード
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[14] );	// "＜長期駐車全解除送信＞　　　　"
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[15] );	// "＜長期駐車全解除送信＞　　　　"
// MH321800(E) S.Takahashi 2020/05/12 遠隔ダウンロード
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[9] );	// "　長期駐車全解除を送信します　"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );		// "　　　　よろしいですか？　　　"
	Fun_Dsp(FUNMSG[19]);												// "　　　　　　 はい いいえ　　　"

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
			break;
		case KEY_TEN_F3:
			BUZPI();
			for(i = 0; i < LOCK_MAX ; i++){
				// 長期駐車の全車室の状態を解除
				FLAPDT.flp_data[i].flp_state.BIT.b00 = 0;
				FLAPDT.flp_data[i].flp_state.BIT.b01 = 0;
			}
			// 全解除のログ登録
			Make_Log_LongParking_Pweb( 0, 0, LONGPARK_LOG_ALL_RESET, LONGPARK_LOG_NON);// 全解除 長期駐車データログ生成
			Log_regist( LOG_LONGPARK );// 長期駐車データログ登録
			return MOD_EXT;
			break;
		case KEY_TEN_F4:						// 「いいえ」
			BUZPI();
			return MOD_EXT;
			break;
		default:
			break;
		}
	}

}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

// MH321800(S) D.Inaba ICクレジット対応(決済リーダチェック追加)
/*[]----------------------------------------------------------------------[]*/
/*| 決済リーダチェック                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_EcReader( void )                　               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static ushort	FncChk_EcReader( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

// MH810103 GG119202(S) 設定参照変更
	if (! isEC_USE()) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH810103 GG119202(E) 設定参照変更
	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[16]);			/* "＜決済リーダチェック＞　" */
		msg = Menu_Slt(ECR_MENU, FECR_CHK_TBL, (char)FECR_CHK_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return( MOD_CUT );
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case MOD_CHG:
			return( MOD_CHG );
			break;
		case MOD_EXT:		/* "終了" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return( MOD_EXT );
			break;
		case FISMF1_CHK:	/* 通信テスト 				*/
			ret = Com_Chk_Suica();
			break;
		case FISMF2_CHK:	/* 通信ログプリント 		*/
			ret = Log_Print_Suica_menu();
			break;
		case FISMF3_CHK:	/* 通信ログプリント（異常）*/
			ret = Log_Print_Suica( 1 );
			break;
		case ECR_BRAND_COND:	/*   ブランド状態確認   */
			ret = EcReaderBrandCondition();
			if (ret == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			if (ret == LCD_DISCONNECT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			break;
		case ECR_MNT:		/* 決済リーダ処理 -  リーダメンテナンス	*/
			// 決済リーダ初期化OK？(決済リーダ起動シーケンス起動済でなければ入れない)
			if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
				BUZPIPI();
				break;
			}
			ret = EcReader_Mnt();
			if (ret == MOD_CHG){
				return(MOD_CHG);
			}
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}

//[]----------------------------------------------------------------------[]
//	@brief			決済リーダチェック/ブランド状態確認
//[]----------------------------------------------------------------------[]
//	@param[in]		None
//	@return			usSysEvent		MOD_CHG : mode change<br>
// 									MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyBrandCondition移植
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static	ushort EcReaderBrandCondition(void)
{
	ushort				msg;
	int					mode = 0;			// 0:実行前、1:プリント中
	int					disp;
	int					index;
	ushort				pri_cmd	= 0;		// 印字要求ｺﾏﾝﾄﾞ格納ﾜｰｸ
	uchar				pri;				// 対象プリンタ(0/R_PRI/J_PRI/RJ_PRI)
	T_FrmEcBrandReq		FrmPrnBrand;		// ブランド状態印字要求メッセージ
	T_FrmPrnStop		FrmPrnStop;			// 印字中止要求ﾒｯｾｰｼﾞﾜｰｸ
// MH810105(S) MH364301 QRコード決済対応
//// MH810103(s) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
//	uchar				single = 0;			// 1ブランドのみ
//// MH810103(e) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
	int					idx;
	ushort				brand_no;
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
	ushort	ret;
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。


	// 有効なテーブルがない場合は終了
	if (RecvBrandTbl[0].num == 0) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH810105(S) MH364301 QRコード決済対応
//// MH810103(s) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
//	else if( RecvBrandTbl[0].num == 1){
//		// 1ブランドのみ
//		single = 1;
//	}			
//// MH810103(e) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
// MH810105(E) MH364301 QRコード決済対応

	// 初期表示
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[10]);					/* "＜ブランド状態確認＞　　　　　" */
// MH810105(S) MH364301 QRコード決済対応
//// MH810103(s) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
////	Fun_Dsp( FUNMSG2[17] );															/* "　▲　　▼　       ﾌﾟﾘﾝﾄ 終了 " */
//	if( single ){
//		Fun_Dsp( FUNMSG2[66] );															/* "　 　　 　       ﾌﾟﾘﾝﾄ 終了 " */
//		
//	}else{
//		Fun_Dsp( FUNMSG2[17] );															/* "　▲　　▼　       ﾌﾟﾘﾝﾄ 終了 " */
//	}
//// MH810103(e) 電子マネー対応 #5582 【検証課指摘事項】1ページしかない画面にページ切り替えボタンが表示されている
// MH810105(E) MH364301 QRコード決済対応

	index = 0;
	disp = 1;
	for ( ; ; ) {
		// 画面更新
		if (disp) {
			disp = 0;
// MH810105(S) MH364301 QRコード決済対応
//			dsp_brand_condition(index);
			brand_no = dsp_brand_condition(index);
// MH810105(E) MH364301 QRコード決済対応
		}
		
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
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)
			}
			break;
// MH810100(E) S.Nishimoto 2020/08/31 車番チケットレス(#4758 テストパタン印字中にドアノブ閉→開すると初期画面に遷移し、精算開始が行えない状態となる)

		case KEY_TEN_F5:	/* 終了(F5) */
			if (mode != 0) {
			// プリント中は処理しない
				break;
			}
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F3:	/* 中止(F3) */
			if (mode != 1) {
// MH810105(S) MH364301 QRコード決済対応
				idx = EcSubBrandCheck(brand_no);
				if (idx >= 0) {						// サブブランドを持つブランド番号
					BUZPI();
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
//					EcReaderSubBrandCondition((uchar)idx);	// サブブランドの一覧を表示
					ret = EcReaderSubBrandCondition((uchar)idx);	// サブブランドの一覧を表示
					if (ret == MOD_CHG){
						return(MOD_CHG);
					}
					if (ret == LCD_DISCONNECT){
						return(MOD_CUT);
					}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
					disp = 1;
				}
// MH810105(E) MH364301 QRコード決済対応
			// プリント中以外、処理しない
				break;
			}
			/*------	印字中止ﾒｯｾｰｼﾞ送信	-----*/
			FrmPrnStop.prn_kind = R_PRI;			// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
			pri_cmd = PREQ_INNJI_TYUUSHI;
			queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F4:	/* プリント(F4) */
			if (mode == 0) {
			// 実行前のみ有効
				pri = R_PRI;
				if (check_printer(&pri) != 0) {
				// レシートプリンタ紙切れ or エラー発生中
					BUZPIPI();
					break;
				}
				BUZPI();
				/*------	ブランド状態印字要求ﾒｯｾｰｼﾞ送信	-----*/
				FrmPrnBrand.prn_kind = R_PRI;							// ﾌﾟﾘﾝﾀ種別
				FrmPrnBrand.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// 機械№
				FrmPrnBrand.pridata = &RecvBrandTbl[0];
				FrmPrnBrand.pridata2 = &RecvBrandTbl[1];
				pri_cmd = PREQ_EC_BRAND_COND;
				queset( PRNTCBNO, PREQ_EC_BRAND_COND, sizeof(T_FrmEcBrandReq), &FrmPrnBrand );
				mode = 1;
				Fun_Dsp( FUNMSG[82] );												/* "　　　　　　 中止 　　　　　　" */
			}
			break;

		case KEY_TEN_F1:	/* ▲(F1) */
			if (mode != 0) {
			// 実行前以外、処理しない
				break;
			}
			if (RecvBrandTbl[0].num == 1) {
			// 有効なテーブルが一つしかない場合は処理しない
				break;
			}
			BUZPI();
			if (index == 0) {
				index = RecvBrandTbl[0].num + RecvBrandTbl[1].num - 1;
			} else {
				index--;
			}
			// 画面更新
			disp = 1;
			break;

		case KEY_TEN_F2:	/* ▼(F2) */
			if (mode != 0) {
			// 実行前以外、処理しない
				break;
			}
			if (RecvBrandTbl[0].num == 1) {
			// 有効なテーブルが一つしかない場合は処理しない
				break;
			}
			BUZPI();
			if (index >= (RecvBrandTbl[0].num + RecvBrandTbl[1].num - 1)) {
				index = 0;
			} else {
				index++;
			}
			// 画面更新
			disp = 1;
			break;

		case EC_BRAND_UPDATE:
			disp = 1;
			break;

		default:
			if (mode != 0) {
			// 実行前以外
				// 印字メッセージ処理
				if (pri_cmd != 0) {
					if (msg == ( pri_cmd | INNJI_ENDMASK )) { 				// 印字終了ﾒｯｾｰｼﾞ受信？
						// 実行完了
						return MOD_EXT;
					}
				}
			}
			break;
		}
		
	}
}

// MH810105(S) MH364301 QRコード決済対応
#define	SUB_BRAND_VIEW_LIMIT	8	// 1ページのサブブランド最大表示数
//[]----------------------------------------------------------------------[]
///	@brief			サブブランド一覧表示
//[]----------------------------------------------------------------------[]
///	@param[in]		sub_idx（0:サブブランドなし）
///	@return			MOD_XX
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022-03-22
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static ushort EcReaderSubBrandCondition(uchar sub_idx)
{
	ushort	msg;
	uchar	disp;
	uchar	sub_num;
	uchar	max_page;
	uchar	current_page;

	if (sub_idx > 0) {
		// サブブランドあり

		// サブブランドの数からページ数を算出(1ページ8項目)
		sub_num = RecvSubBrandTbl.sub_brand_num[sub_idx-1];
		if (sub_num > 0) {
			max_page = (uchar)((sub_num-1) / SUB_BRAND_VIEW_LIMIT);
		}
	}
	else {
		max_page = 0;
	}
	current_page = 0;							// 1ページ目
	disp = 1;

	while (1) {
		if (disp) {
			disp = 0;
			if (max_page == 0) {
				Fun_Dsp( FUNMSG[8] );			// "　　　　　　　　　　　 終了 "
			} else {
				Fun_Dsp( FUNMSG[6] );			// "　▲　　▼　　　　　　　 終了 "
			}
			dsp_sub_brand_condition(sub_idx, current_page, max_page);
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
//		case KEY_MODECHG:
//			return MOD_CHG;
		case LCD_DISCONNECT:
			return MOD_CUT;
		case KEY_MODECHG:
			// ドアノブ閉かどうかのチェックを実施
			if (CP_MODECHG) {
				return MOD_CHG;
			}
			break;
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。

		case KEY_TEN_F1:	/* ▲(F1) */
			if (max_page == 0) {
				break;
			}

			BUZPI();
			if (current_page == 0) {
				current_page = max_page;
			}
			else {
				current_page--;
			}
			disp = 1;
			break;

		case KEY_TEN_F2:	/* ▼(F2) */
			if (max_page == 0) {
				break;
			}

			BUZPI();
			if (current_page >= max_page) {
				current_page = 0;
			}
			else {
				current_page++;
			}
			disp = 1;
			break;

		case KEY_TEN_F5:	/* 終了(F5) */
			BUZPI();
			return MOD_EXT;
		}
	}
}

uchar	initial_message[2][19] = {
	{"決済ブランド未受信"},
	{"決済ブランドなし"},
};
//[]----------------------------------------------------------------------[]
///	@brief			サブブランド一覧表示(詳細)
//[]----------------------------------------------------------------------[]
///	@param[in]		sub_idx			: サブブランドインデックス（0:サブブランドなし）
///	@param[in]		current_page	: 現在ページ
///	@param[in]		max_page		: 最大ページ
///	@return			None
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022-03-22
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void dsp_sub_brand_condition(uchar sub_idx, uchar current_page, uchar max_page)
{
	uchar	i, max;
	uchar	cnt;
	uchar	line_max = SUB_BRAND_VIEW_LIMIT/2;
	ushort	len;
	char	str_page[8];
	char	brand_name[15];

	// 画面クリア
	for(i = 1; i <= 6; i++) {
		displclr((ushort)i);
	}

	// ページを表示
	memset(str_page, 0, sizeof(str_page));
	sprintf(str_page, "[%02d/%02d]", current_page+1, max_page+1);
	grachr(0, 22, 7, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)str_page);

	if (RecvSubBrandTbl.tbl_sts.BIT.RECV == 0) {
		// "決済ブランド未受信"
		grachr(3, 2, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, initial_message[0]);
		return;
	}
	else if (sub_idx == 0) {
		// "決済ブランドなし"
		grachr(3, 2, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, initial_message[1]);
		return;
	}

	i = (uchar)(current_page * SUB_BRAND_VIEW_LIMIT);
	max = (uchar)((current_page+1) * SUB_BRAND_VIEW_LIMIT);
	for (; i < max; i++) {
		if (i >= RecvSubBrandTbl.sub_brand_num[sub_idx-1]) {
			// サブブランド数に到達
			break;
		}

		// サブブランド名取得
		len = (ushort)EcGetSubBrandName(RecvSubBrandTbl.brand_no[sub_idx-1], 0, i, (uchar*)brand_name);
		if (len == 0) {
			continue;
		}

		cnt = (uchar)(i % SUB_BRAND_VIEW_LIMIT);
		if (cnt < line_max) {		// 左に表示するか、右に表示するか
			// サブブランドを左側に表示
			grachr((ushort)(2+cnt), 2, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)brand_name);
		}
		else {
			// サブブランドを右側に表示
			grachr((ushort)(2+cnt-line_max), 16, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)brand_name);
		}
	}
}
// MH810105(E) MH364301 QRコード決済対応

//[]----------------------------------------------------------------------[]
///	@brief			ブランド状態表示
//[]----------------------------------------------------------------------[]
///	@param[in]		index	:ブランドテーブルインデックス
///	@return			None
///	@author			Inaba
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/01/29
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810105(S) MH364301 QRコード決済対応
//static	void dsp_brand_condition(int index)
static	ushort dsp_brand_condition(int index)
// MH810105(E) MH364301 QRコード決済対応
{
	char		str_page[8];
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
//	int			brand;
// MH810103 GG119202(E) ブランド名はブランド情報から取得する
	int			brand_index;
	int			check;
	int			Array_Num  = 0;
	ushort		i;
	int			page_index = index;
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
	uchar		wname[21];
// MH810103 GG119202(E) ブランド名はブランド情報から取得する
// MH810105(S) MH364301 QRコード決済対応
	ushort		brand_no;
// MH810105(E) MH364301 QRコード決済対応

	// ブランド数が10より大きい時、RecvBrandTbl[1]を参照する。
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	if(index >= EC_BRAND_MAX){
//		Array_Num = 1;
//		index = index - EC_BRAND_MAX;
//	}
	if(index >= RecvBrandTbl[0].num){
		Array_Num = 1;
		index = index - RecvBrandTbl[0].num;
	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	
// MH810105(S) MH364301 QRコード決済対応
	brand_no = RecvBrandTbl[Array_Num].ctrl[index].no;
	if (RecvBrandTbl[0].num == 1) {
		if (EcSubBrandCheck(brand_no) >= 0) {
			Fun_Dsp( FUNMSG2[68] );							// "　　　　　　 ｼｮｳｻｲ ﾌﾟﾘﾝﾄ 終了 "
		}
		else {
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
//			Fun_Dsp( FUNMSG2[64] );							// "　 　　 　       ﾌﾟﾘﾝﾄ 終了 "
			Fun_Dsp( FUNMSG2[66] );							// "　 　　 　       ﾌﾟﾘﾝﾄ 終了 "
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6438 ブランド状態確認でQRのサブブランド確認時にドアノブを閉めると、接客画面に遷移しない。
		}
	}
	else{
		if (EcSubBrandCheck(brand_no) >= 0) {
			Fun_Dsp( FUNMSG2[67] );							// "　▲　　▼　 ｼｮｳｻｲ ﾌﾟﾘﾝﾄ 終了 "
		}
		else {
			Fun_Dsp( FUNMSG2[17] );							// "　▲　　▼　       ﾌﾟﾘﾝﾄ 終了 "
		}
	}
// MH810105(E) MH364301 QRコード決済対応

	// 画面クリア
	for(i = 1; i <= 5; i++) {
		displclr((ushort)i);
	}

	// ページ
	memset(str_page, 0x0, sizeof(str_page));
	sprintf(str_page, "[%02d/%02d]", page_index + 1, (int)(RecvBrandTbl[0].num + RecvBrandTbl[1].num));
	grachr(0, 22, 7, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_page);

	// ブランド名
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
//	brand = get_brand_index(RecvBrandTbl[Array_Num].ctrl[index].no);
//	if( brand == ( EC_UNKNOWN_USED - EC_USED ) ){
//		brand_index = 3;	// 不明
//	} else {
//		brand_index = brand + 2;
//	}
//	grachr(1, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_BRAND_STR[brand_index]);
//	grachr(1, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "：");
	brand_index = getBrandName(&RecvBrandTbl[Array_Num].ctrl[index], wname);
	grachr(1, 2, (ushort)brand_index, 0, COLOR_BLACK, LCD_BLINK_OFF, wname);
// MH810103 GG119202(E) ブランド名はブランド情報から取得する

	// ラベル
	for (i = 0; i < 3; i++) {
		grachr(i+2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_BRAND_STR[i]);				// "    開局／閉局  "
		grachr(i+2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "：");						// "    有効／無効  "
	}																					// "    有効／無効  "
																						// "    サービス状態"
	// 状態
// MH810103 GG119202(S) ブランド状態判定処理変更
//	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x0f) == 0x01) ? 1 : 0;		// 開局 & 有効 & 空き & 動作中
// MH810103 GG119202(S) サービス停止中ブランドはブランド設定データにセットしない
//	check = (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[Array_Num].ctrl[index].status)) ? 1 : 0;	// 開局 & 有効
	check = (isEC_BRAND_STS_ENABLE(RecvBrandTbl[Array_Num].ctrl[index].status)) ? 1 : 0;	// 開局 & 有効 & サービス停止中
// MH810103 GG119202(E) サービス停止中ブランドはブランド設定データにセットしない
// MH810103 GG119202(E) ブランド状態判定処理変更
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
//	grachr(1, 18, (check ? 6 : 8), 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[check]);		// "利用不可/利用可"
	grachr(5, 11, 8, 0, (check ? COLOR_GREEN : COLOR_RED), LCD_BLINK_OFF, EC_STR[check]);	// "利用不可/利用可"
// MH810103 GG119202(E) ブランド名はブランド情報から取得する
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x01) == 0x01) ? 1 : 0;
	grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[2 + check]);					// "閉局/開局"
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x02) == 0x02) ? 1 : 0;
	grachr(3, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[4 + check]);					// "有効/無効"
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x08) == 0x08) ? 1 : 0;
	grachr(4, 18, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[6 + check]);					// "動作中/停止中"

// MH810105(S) MH364301 QRコード決済対応
//	return;
	return brand_no;
// MH810105(E) MH364301 QRコード決済対応
}

// MH810103 GG119202(S) ブランド名はブランド情報から取得する
////[]----------------------------------------------------------------------[]
////	@brief		ブランドインデックス取得
////[]----------------------------------------------------------------------[]
////	@param[in]	no	:	ブランド番号
////	@return		ブランドインデックス
////[]----------------------------------------------------------------------[]
////	@author		emura
////	@date		Create	: 14/03/28<br>
////				Update	: 19/01/29  決済リーダ対応
////	@file		usermnt2.c
////[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
//uchar	get_brand_index(ushort no)
//{
//// GG119200(S) ブランド選択シーケンス変更（T／法人対応から移植）
////	uchar	index = EC_UNKNOWN_USED - EC_USED;
////
////	switch (no) {
////	case	BRANDNO_KOUTSUU:
////		index = EC_KOUTSUU_USED - EC_USED;
////		break;
////	case	BRANDNO_EDY:
////		index = EC_EDY_USED - EC_USED;
////		break;
////	case	BRANDNO_NANACO:
////		index = EC_NANACO_USED - EC_USED;
////		break;
////	case	BRANDNO_WAON:
////		index = EC_WAON_USED - EC_USED;
////		break;
////	case	BRANDNO_SAPICA:
////		index = EC_SAPICA_USED - EC_USED;
////		break;
////	case	BRANDNO_ID:
////		index = EC_ID_USED - EC_USED;
////		break;
////	case	BRANDNO_QUIC_PAY:
////		index = EC_QUIC_PAY_USED - EC_USED;
////		break;
////	case	BRANDNO_CREDIT:
////		index = EC_CREDIT_USED - EC_USED;
////		break;
////	case	BRANDNO_HOUJIN:
////		index = EC_HOUJIN_USED - EC_USED;
////		break;
////	default	:
////		break;
////	}
////	return index;
//	return (uchar)(convert_brandno(no) - EC_USED);
//// GG119200(E) ブランド選択シーケンス変更（T／法人対応から移植）
//}
// MH810103 GG119202(E) ブランド名はブランド情報から取得する

/*[]----------------------------------------------------------------------[]*/
/*| 　　　　　　　　リーダメンテナンス処理 /　送信画面（受信画面）　　　   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_EcReaderMnt(void)       　                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_EXT(F5)   MOD_CHG(メンテナンス終了) 	   			   |*/
/*| Date         : 2019-01-28                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static ushort EcReader_Mnt( void )
{
	ushort	msg;
	uchar	mode_now = 0; 	// 現在のリーダモード
							// 0 = 通常モード
							// 1 = メンテナンスモード
							// 2 = 結果受信画面にてメンテナンススイッチOFF
							// 3 = 結果受信画面にて通信不良発生
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
							// 4 = 結果受信画面にてLCD通信不良発生
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
	uchar	disp = 1;		// 1 = 画面更新
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//	uchar	mnt_exit = 0;	// メンテナンスモード終了確認画面フラグ： 0 = 「メンテナンス中」表示 , 1 = 「通常モードに～」表示
	uchar	ret = 0;		// 0=成功, 1=失敗, 2=メンテOFF, 3=通信不良
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更

	// 送信データ初期化
	memset(&MntTbl, 0x0, sizeof(EC_MNT_TBL));

	for( ; ; ) {
		if( disp ){
			if( mode_now == 0 ){
				if(!Ope_IsEnableDoorKnobChime()){											// ドアノブ戻し忘れチャイム無効？
					Ope_EnableDoorKnobChime();												// ドアノブ戻し忘れチャイム有効にする(ﾀｲﾏｰｽﾀｰﾄ)
				}
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "＜リーダメンテナンス＞　  　　" */
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[1] );				/* " リーダをメンテナンスモードに " */
//				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );				/* " 移行します。よろしいですか？ "*/
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[1] );				/* "　　　　 決済リーダを 　　　　" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );				/* "　メンテナンスモードにします　" */
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );				/* "　　　 よろしいですか？ 　　　" */
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
				Fun_Dsp( FUNMSG[19] );														/* "　　　　　はい　いいえ　　　" */
				MntTbl.mode = 1;		// 現状のモードが通常なら送るモードはメンテナンスモード
				disp = 0;
			} else {
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "＜リーダメンテナンス＞　　  　" */
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//				if( mnt_exit == 0 ){
//					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[5] );			/* "  リーダメンテナンス中です。  " */
//					Fun_Dsp( FUNMSG[8] );													/* "　　　　　  　　　  終了 " */
//				} else {
//					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[14] );			/* " 　　リーダを通常モードに 　　" */
//					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );			/* " 移行します。よろしいですか？ "*/
//					Fun_Dsp( FUNMSG[19] );													/* "　　　　　はい　いいえ　　　" */
//				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[5] );				/* " 　リーダメンテナンス中です 　" */
				Fun_Dsp( FUNMSG[8] );														/* "　　　　　　　　　　　　 終了 " */
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
				MntTbl.mode = 0;		// 現状のモードがメンテナンスなら送るモードは通常モード
				disp = 0;
			}
		}

		if( mode_now == 1 ){
			Ope_DisableDoorKnobChime();														// 決済リーダメンテナンス中はドアノブ戻し忘れチャイム無効
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg ){						/* FunctionKey Enter */
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
		case LCD_DISCONNECT:
			// リーダを通常モードに移行して終了する
			if( mode_now == 1 ){
				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
// MH810103 GG119202(S) メンテナンスを抜けるときも応答待ちする
				EcReader_Mnt_CommResult( 0 );
// MH810103 GG119202(E) メンテナンスを抜けるときも応答待ちする
			}
			return MOD_CUT;
			break;
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)

		case KEY_MODECHG:
			// リーダを通常モードに移行して終了する
			if( mode_now == 1 ){
				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
// MH810103 GG119202(S) メンテナンスを抜けるときも応答待ちする
				EcReader_Mnt_CommResult( 0 );
// MH810103 GG119202(E) メンテナンスを抜けるときも応答待ちする
			}
			return MOD_CHG;
			break;

		case KEY_TEN_F3:					/* F3: はい*/
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//			if( mode_now == 0 ) {	// リーダのモードが通常
//				BUZPI();
//				// リーダをメンテナンスモードに移行
//				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//				mode_now = EcReader_Mnt_CommResult( MntTbl.mode );
//				disp = 1;
//			} else {				// リーダのモードがメンテナンス
//				if( mnt_exit ){		// 「通常モードに～」表示中以外は受け付けない
//					BUZPI();
//					// リーダを通常モードに移行
//					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//					mode_now = EcReader_Mnt_CommResult( MntTbl.mode );
//					mnt_exit = 0;
//					disp = 1;
//				}
//			}
//			// 結果受信画面にて物理的にメンテナンススイッチをOFFにした
//			if( mode_now == 2 ){
//				return MOD_CHG;
//			}
//			// 結果受信画面にて通信不良が発生した
//			else if( mode_now == 3 ){
//				return MOD_EXT;
//			}
//			// mode_now = 1 もしくは mode_now = 0
//			else{
//				// メンテナンスOFF(扉を閉めていたら)
//				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
//					if( mode_now == 1 ){
//						// メンテナンスモード中であれば通常モードに移行して終了する
//						MntTbl.mode = 0;
//						Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//// GG119200(S) メンテナンスを抜けるときも応答待ちする
//						EcReader_Mnt_CommResult( 0 );
//// GG119200(E) メンテナンスを抜けるときも応答待ちする
//					}
//					return MOD_CHG;
//				}
//			}
			if (mode_now != 0) {
				break;
			}

			BUZPI();
			// リーダをメンテナンスモードに移行
			Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
			ret = EcReader_Mnt_CommResult( MntTbl.mode );
			switch (ret) {
			case 0:				// 成功
				// メンテナンスOFF(扉を閉めていたら)
				if (!OPE_SIG_DOORNOBU_Is_OPEN) {
					// リーダを通常モードに移行
					MntTbl.mode = 0;
					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
					EcReader_Mnt_CommResult( MntTbl.mode );
					return MOD_CHG;
				}
				// メンテナンスモード
				mode_now = 1;
				disp = 1;
				break;
			case 1:				// 失敗
			case 3:				// 通信不良
			default:
				return MOD_EXT;
			case 2:				// メンテOFF
				return MOD_CHG;
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
			case 4:
				return MOD_CUT;
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
			}
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
			break;
		
		case KEY_TEN_F4:					/* F4: いいえ*/
			if( mode_now == 0 ) {	// リーダのモードが通常	
				BUZPI();
				return MOD_EXT;
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//			} else {				// リーダのモードがメンテナンス
//				if( mnt_exit ){		// 「通常モードに～」表示中以外は受け付けない
//					BUZPI();
//					mnt_exit = 0;
//					disp = 1;
//				}
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
			}
			break;
		
		case KEY_TEN_F5:					/* F5:終了 */
			// 「メンテナンス中」表示中以外は受け付けない
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//			if( mode_now == 1 && mnt_exit == 0 ) {
//				BUZPI();
//				mnt_exit = 1;
//				disp = 1;
//			}
			if (mode_now != 1) {
				break;
			}
			BUZPI();

			// リーダを通常モードに移行
			Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
			ret = EcReader_Mnt_CommResult( MntTbl.mode );
			switch (ret) {
			case 0:				// 成功
				// 通常モード
				mode_now = 1;
				// no break
			case 1:				// 失敗
			case 3:				// 通信不良
			default:
				return MOD_EXT;
			case 2:				// メンテOFF
				return MOD_CHG;
			}
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
			break;

		case EC_MNT_UPDATE:			// 決済リーダが自らモード移行通知データ送信してきた
			// 音声再生だった場合は読み捨て
			if( RecvMntTbl.cmd == 1){
				break;
			} else if(RecvMntTbl.cmd == 2) {
				// 決済リーダが再起動したため精算機の再起動を促す
				EcReader_Mnt_ResetDisp();
			}
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//			mode_now = RecvMntTbl.mode;
//			mnt_exit = 0;
//			disp = 1;				// 画面更新
			if (RecvMntTbl.mode == 0) {
				// 通常モード
				return MOD_EXT;
			}
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
			break;

		case EC_EVT_COMFAIL:		// 通信不良発生
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );			/* "＜リーダメンテナンス＞　　　  " */
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[15] );			/* "　　通信不良が発生しました　　" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[16] );			/* "リーダメンテナンスを終了します" */
				Fun_Dsp( FUNMSG[8] );													/* "　　　　　  　　　  終了 " */
				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg ){								/* FunctionKey Enter */
						case KEY_TEN_F5:						/* F5:終了 */
							BUZPI();
							return MOD_EXT;
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
						case LCD_DISCONNECT:
							return MOD_CUT;
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
						case KEY_MODECHG:
							BUZPI();
							return MOD_CHG;
						// 何もしない
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

/*[]----------------------------------------------------------------------[]*/
/*| リーダメンテナンス処理 /受信画面（失敗画面）                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcReader_Mnt_CommResult                                 |*/
/*| PARAMETER    : uchar 決済リーダへ送信したモード                        |*/
/*| RETURN VALUE : uchar 0 : 成功                                          |*/
/*|                      1 : 失敗                                          |*/
/*|                      2 : メンテナンススイッチOFF                       |*/
/*|                      3 : 通信不良発生                                  |*/
/*| Date         : 2019-01-28                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static uchar EcReader_Mnt_CommResult( uchar mode )
{
	ushort	msg;
	uchar	result = 0; 	// 0 = 成功、　1 = 失敗
	uchar	permission = 0;	// 0 = 許可しない、1 = 許可する
// MH810103 GG119202(S) ドア閉しても待機画面に遷移しない
//	uchar	modechg = 1;	// 0 = メンテナンススイッチOFF、1 = メンテナンススイッチON
	uchar	mnt_sw = 0;		// メンテナンスOFFになったかどうか
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
	uchar	disconnect = 1;	// 0 = LCD通信不良発生、1 = LCD通信正常
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)

	if (!OPE_SIG_DOORNOBU_Is_OPEN) {
		mnt_sw = 1;			// メンテOFF保持
	}
// MH810103 GG119202(E) ドア閉しても待機画面に遷移しない

	// 送信したメンテナンスモードに応じて表示を変える mode： 1 = メンテナンスモード送信　0 = 通常モード送信
	dispclr();
	if( mode ){
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "＜リーダメンテナンス＞　  　　" */
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[3] );				/* "メンテナンスモードに移行中です" */
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "　しばらくお待ちください。　" */
	} else {
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "＜リーダメンテナンス＞　  　　" */
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[6] );				/* "　通常モードに移行中です。  　" */
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "　しばらくお待ちください。  　" */
	}
// MH810105(S) MH364301 ファンクションキーを表示させる
	Fun_Dsp( FUNMSG[0] );															/* "　　　　　　　　　　　　　　　" */
// MH810105(E) MH364301 ファンクションキーを表示させる

	// タイムアウト設定
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	for( ; ; ) {

		// 失敗の時表示する
		if( result ){
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );			/* "＜リーダメンテナンス＞　　　  " */
				if( mode ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[7] );	/* " メンテナンスモードへの移行に " */
				} else {
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[8] );	/* " 	  通常モードへの移行に	  "*/
				}
			grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[9] );			/* "  　　　失敗しました。　  　　" */
			Fun_Dsp( FUNMSG[8] );													/* "　　　　　  　　　      終了  " */
			result = 0;
			permission = 1;
// MH810103 GG119202(S) ドア閉しても待機画面に遷移しない
			// メンテナンスOFF(扉を閉めていたら)
			if (mnt_sw) {
				return 2;
			}
// MH810103 GG119202(E) ドア閉しても待機画面に遷移しない
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg ){						/* FunctionKey Enter */
		case EC_EVT_COMFAIL:		// 通信不良発生
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );		/* "＜リーダメンテナンス＞　　　  " */
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[15] );		/* "　　通信不良が発生しました　　" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[16] );		/* "リーダメンテナンスを終了します" */
				Fun_Dsp( FUNMSG[8] );												/* "　　　　　  　　　  終了 " */
				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg ){													/* FunctionKey Enter */
						case KEY_TEN_F5:											/* F5:終了 */
							BUZPI();
							result = 3;
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
							Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
							return result;
						case KEY_MODECHG:
							BUZPI();
							result = 2;
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
							Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
							return result;
// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
						case LCD_DISCONNECT:
							result = 4;
							return result;
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
						// 何もしない
						default:
							break;
					}
				}
			break;

// MH810100(S) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)
		case LCD_DISCONNECT:
			// LCD通信不良発生
			disconnect = 0;
			break;
// MH810100(E) S.Fujii 2020/06/30 車番チケットレス(電子決済端末対応)

		case KEY_MODECHG:
// MH810103 GG119202(S) ドア閉しても待機画面に遷移しない
//				// メンテナンススイッチをOFFにした
//				modechg = 0;
			if (permission) {
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
				return 2;
			}
			else {
				mnt_sw = 1;
			}
// MH810103 GG119202(E) ドア閉しても待機画面に遷移しない
			break;

		case KEY_TEN_F5:					/* F5:終了 */
			// 失敗画面以外は処理させない
			if( permission ) {
				BUZPI();
// MH810103 GG119202(S) ドア閉しても待機画面に遷移しない
//				// メンテナンススイッチをOFFにした
//				if( !modechg ){
//					// リーダを通常モードに移行して終了する
//					if( mode == 0 ){
//						Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//					}
//					mode = 2;
//				} else {
				{
// MH810103 GG119202(E) ドア閉しても待機画面に遷移しない
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//					// 失敗時は必ず逆を返す
//					mode = mode? 0 : 1;
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
				}
// MH810103(s) 電子マネー対応
				if( !disconnect ){
					// リーダを通常モードに移行して終了する
					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
				}
// MH810103(e) 電子マネー対応
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//				return mode;
				return 1;	// 失敗
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更
			}
			break;

		case EC_MNT_UPDATE:			// モード移行通知データ受信
			// 音声再生だった場合は読み捨て
			if( RecvMntTbl.cmd == 1){
				break;
			} else if(RecvMntTbl.cmd == 2) {
				// 決済リーダが再起動したため精算機の再起動を促す
				EcReader_Mnt_ResetDisp();
			}
			// 受信したモードと要求したモードが不一致の場合は
			// モード移行失敗とする
			if( RecvMntTbl.mode != mode){
				result = 1;
				break;
			}
// MH810103 GG119202(S) 問い合わせタイマキャンセル処理追加
			Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) 問い合わせタイマキャンセル処理追加
// MH810103 GG119202(S) リーダーメンテナンス画面仕様変更
//			return mode;
//			break;
			return 0;	// 成功
// MH810103 GG119202(E) リーダーメンテナンス画面仕様変更

		case EC_CMDWAIT_TIMEOUT: // タイムアウト
			result = 1;
			break;

		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 　　　　　　リーダメンテナンス処理 /　再起動画面　　　　　　　　　　 　|*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcReader_Mnt_ResetDisp        　                        |*/
/*| PARAMETER    : uchar                                                   |*/
/*| RETURN VALUE : MOD_EXT    			                                   |*/
/*| Date         : 2019-09-11                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static void 	EcReader_Mnt_ResetDisp(void)
{
	ushort	msg = 0;

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "＜リーダメンテナンス＞　  　　" */
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[11] );				/* "　　決済リーダを更新中です　　" */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "　　しばらくお待ちください　　" */
	Fun_Dsp( FUNMSG[0] );														/* "　　　　　　　　　　　　　　　" */

	// タイムアウト設定
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	// タイムアウトを待つ
	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch( msg ){
		case EC_CMDWAIT_TIMEOUT: // タイムアウト
			grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[12] );		/* "決済リーダの更新が完了しました" */
			grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[13] );		/* " 　電源をOFF/ONしてください　 " */
			break;
		// 何もしない
		default:
			break;
		}
	}
}
// MH321800(E) D.Inaba ICクレジット対応(決済リーダチェック追加)

// MH810100(S) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)
//[]----------------------------------------------------------------------[]
/// @brief ＱＲリーダーチェックメニュー
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort  FncChk_QRchk( void ) 
{
	ushort msg;
	char	wk[2];
	char	org[2];

	// QR有効？
	if( !QR_READER_USE ){
		BUZPIPI();
		return MOD_EXT;
	}
	
	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;


	for ( ; ; ) {

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[57] );		// "＜ＱＲリーダーチェック＞　　"

		msg = Menu_Slt( QRMENU, QR_CHK_TBL, (char)QR_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (msg) {
			case QR_READ:
				msg = FncChk_QRread();		// 読取テスト
				break;

			case QR_VER:
				msg = FncChk_QRverChk();	// バージョン確認
				break;

			case MOD_EXT:								// 終了（Ｆ５）
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
		if (msg == MOD_CHG || msg == MOD_CUT){				// モードチェンジ
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
/// @brief		QR読み取りテスト
//[]----------------------------------------------------------------------[]
///	@param[in]	無し
/// @return 	ret		: Exit ID<br>
///					MOD_CUT=切断通知<br>
///					MOD_CHG=モードチェンジ<br>
///					MOD_EXT=終了
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2019/10/08<br>
///				Update	: 2021/03/26
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// ushort	 FncChk_QRread( void )
// {
// 	short		msg;				// 受信ﾒｯｾｰｼﾞ
// 	uchar		req = 0;		// ﾃｽﾄ要求状態
// 	ushort		ret = 0;		// 戻り値
// 
// 	dispclr();
// 	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[00] );	//"＜ＱＲリーダーチェック＞　　　"
// 	grachr( 2, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[00] );		// ＱＲコードの
// 	grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[01] );		// "リードテストを行います　　　　
// 	grachr( 5, 0, 28, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[02] );		// ＱＲコードをかざして下さい　　
// 	Fun_Dsp(FUNMSG[82]);												// "　　　　　　 中止 　　　　　　", 
// 
// 	if( PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) ){		// 読取開始
// 		req = 1;										// QRリーダ制御応答待ち
// 	}else{
// 		// ｺﾏﾝﾄﾞ送信ＮＧ
// 		BUZPIPI();
// 		return( MOD_EXT );
// 	}
// 
//	while( req == 1 || ret == 0 ){
//		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
//
//		switch (msg) {							// 受信ﾒｯｾｰｼﾞ
//		case LCD_DISCONNECT:					// 切断通知
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
//			req = 0 ;
//			return MOD_CUT;
//			break;
//		case KEY_MODECHG:						// 操作ﾓｰﾄﾞ切替
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
//			req = 0 ;
//			return MOD_CHG;
//			break;
//
//		case KEY_TEN_F3:						// 中止 (終了)
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
//			req = 0 ;
//			return MOD_EXT;
//			break;
//
//		case LCD_MNT_QR_READ_RESULT:			// 読取結果
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QRリーダ停止要求
//			ret = FncChk_QRread_result( LcdRecv.lcdbm_rsp_QR_rd_rslt.result );	//結果表示
//			req = 0 ;
//			break;
//		default:
//			break;
//		}
//	}
//
//	return( 1 );
// }
ushort FncChk_QRread(void)
{
	lcdbm_rsp_QR_rd_rslt_t*	pQR		= &LcdRecv.lcdbm_rsp_QR_rd_rslt;	// QR読取結果
	QR_AmountInfo*			pAmntQR	= &pQR->QR_data.AmountType;			// QRデータ情報 QR買上券
	short					msg;				// 受信メッセージ
	ushort					state	= 1;		// 案内画面
	ushort					ret		= MOD_EXT;	// 終了
	ushort					updown_page;		// 上下移動のページ
	ushort					updown_page_max;	// 上下移動のページの最大数

	dispclr();
	grachr(0, 0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);	// "ＱＲリーダーチェック　　　　　"
	grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);		// "ＱＲコードの　　　　　　　　　"
	grachr(3, 0, 22, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[1]);		// "リードテストを行います　　　　"
	grachr(5, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[2]);		// "ＱＲコードをかざして下さい　　"
	Fun_Dsp(FUNMSG[82]);											// "             中止             "

	Lagtim(OPETCBNO, 6, 1);	// 読取開始タイマースタート(即時)

	while( state ) {
		msg = StoF( GetMessage(), 1 );	// メッセージ受信

		switch ( msg ) {
		case LCD_DISCONNECT:			// 切断通知
			BUZPI();
			ret = MOD_CUT;
			state = 0;	// 終了
			break;
		case KEY_MODECHG:				// 操作ﾓｰﾄﾞ切替
			BUZPI();
			ret = MOD_CHG;
			state = 0;	// 終了
			break;
		case KEY_TEN_F1:				// ▲
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			if ( state == 2 ) {						// 結果画面 のみ
//				if ( updown_page > 0 ) {
//					updown_page--;
//				} else {
//					updown_page = updown_page_max;
//				}
//				state = FncChk_QRread_result(updown_page);
//			}
			if ( (state == 2) || (state == 5) ) {	// 結果画面 or 結果画面(下) のみ
				updown_page--;
				state = FncChk_QRread_result(updown_page, updown_page_max);
			}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			break;
		case KEY_TEN_F2:				// ▼
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			if ( state == 2 ) {						// 結果画面 のみ
//				if ( updown_page < updown_page_max ) {
//					updown_page++;
//				} else {
//					updown_page = 0;
//				}
//				state = FncChk_QRread_result(updown_page);
//			}
			if ( (state == 2) || (state == 4) ) {	// 結果画面 or 結果画面(上) のみ
				updown_page++;
				state = FncChk_QRread_result(updown_page, updown_page_max);
			}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			break;
		case KEY_TEN_F3:				// 中止
			if ( state == 1 ) {						// 初回読取 のみ
				BUZPI();
				state = 0;	// 終了
			}
			break;
		case KEY_TEN_F5:				// 終了
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			if ( (state == 2) || (state == 3) ) {	// 結果画面 or 失敗画面 のみ
			if ( state != 1 ) {						// 案内画面 以外
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
				BUZPI();
				state = 0;	// 終了
			}
			break;
		case LCD_MNT_QR_READ_RESULT:	// QR読取結果(ﾒﾝﾃﾅﾝｽ)
			updown_page = 0;
			updown_page_max = UsMnt_QR_GetUpdownPageMax(pQR->id, pAmntQR);
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			state = FncChk_QRread_result(updown_page);
			state = FncChk_QRread_result(updown_page, updown_page_max);
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			if ( pQR->result != 0x01 ) {			// 異常 以外
				Lagtim(OPETCBNO, 6, 100);	// 読取開始タイマースタート 2秒(20ms x 100)
			} else {								// 異常
				Lagtim(OPETCBNO, 6, 500);	// 読取開始タイマースタート 10秒(20ms x 500)
			}
			break;
		case TIMEOUT6:					// 読取開始タイマー
			if ( !(PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA)) ) {	// 読取開始
				BUZPIPI();
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}

	Lagcan(OPETCBNO, 6);	// 読取開始タイマーストップ
	PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STP);	// 読取停止

	return ret;
}
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善

//[]----------------------------------------------------------------------[]
/// @brief		QR読み取りテスト(読取結果)
//[]----------------------------------------------------------------------[]
///	@param		updown_page		: 上下移動のページ
///	@param		updown_page_max	: 上下移動のページの最大数
/// @return 	state			: 状態<br>
///					0=終了<br>
///					1=案内画面<br>
///					2=結果画面<br>
///					3=失敗画面<br>
///					4=結果画面(上)<br>
///					5=結果画面(下)
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2019/10/08<br>
///				Update	: 2021/03/26
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善
// static ushort	FncChk_QRread_result(uchar result)
// {
// 	ushort		msg;				// 受信ﾒｯｾｰｼﾞ
//
// 	dispclr();
// 	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0] );	// ＜ＱＲリーダーチェック＞　　　",
// 	grachr( 2, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[00] );	//ＱＲコードの
//
// 	if( result == LCDBM_RESUTL_OK ){
// 		grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[03] );	//リード正常終了　　　　　　　　", 
// 		BUZPIPI();
// 	}else{
// 		grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[04] );	//"読取に失敗しました　　　　　　",  
// 		grachr( 5, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5] );	//		"リーダー又は　　　　　　　　　",  
// 		grachr( 6, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6] );	//	"ＱＲコードを確認して下さい　　", 
// 		BUZPIPI();
// 	}
// 	Fun_Dsp(FUNMSG[8]);						//	"　　　　　　　　　　　　 終了 ", 
// 	while(1){
// 		msg = StoF( GetMessage(), 1 );			// ﾒｯｾｰｼﾞ受信
// 		switch( msg ){		// FunctionKey Enter
// 		case LCD_DISCONNECT:
// 			return MOD_CUT;
// 		case KEY_MODECHG:
// 			BUZPI();
// 			return MOD_CHG;
// 			break;
// 		case KEY_TEN_F5:		// 「終了」
// 			BUZPI();
// 			return MOD_EXT;
// 			break;				
// 		default:
// 			break;
// 		}
// 	}
// }
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
// static ushort FncChk_QRread_result(ushort updown_page)
static ushort FncChk_QRread_result(ushort updown_page, ushort updown_page_max)
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
{
	lcdbm_rsp_QR_rd_rslt_t*	pQR		= &LcdRecv.lcdbm_rsp_QR_rd_rslt;	// QR読取結果
	QR_DiscountInfo*		pDisQR	= &pQR->QR_data.DiscountType;		// QRデータ情報 QR割引券
	QR_AmountInfo*			pAmntQR	= &pQR->QR_data.AmountType;			// QRデータ情報 QR買上券
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
	QR_TicketInfo*			pTicQR  = &pQR->QR_data.TicketType;			// QRデータ情報 QR駐車券
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//	ushort					state	= 3;								// 失敗画面
	ushort					state	= 2;								// 結果画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	grachr(0, 0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);	// "ＱＲリーダーチェック　　　　　"
	dispmlclr(1, 6);
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
	if ( updown_page == 0 ) {
		state = 4;	// 結果画面(上)
	} else if ( updown_page >= updown_page_max ) {
		state = 5;	// 結果画面(下)
	}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない

	switch( pQR->result ) {
	case 0x00:	// 正常
		switch( pQR->id ) {
		case BAR_ID_DISCOUNT:	// 割引券
			BUZPI();
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//			dispmlclr(2, 6);
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[1]);	// "：割引券"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//			UsMnt_QR_DispDisQR(pDisQR, updown_page);
			UsMnt_QR_DispDisQR(pDisQR, updown_page, pQR->qr_type);
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			Fun_Dsp(FUNMSG[6]);										// "  ▲    ▼               終了 "
//			state = 2;	// 結果画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			break;
		case BAR_ID_AMOUNT:		// 買上券
			BUZPI();
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//			dispmlclr(2, 6);
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[0]);	// "：買上券"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
//			UsMnt_QR_DispAmntQR(pAmntQR, updown_page);
			UsMnt_QR_DispAmntQR(pAmntQR, updown_page, pQR->qr_type);
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//			Fun_Dsp(FUNMSG[6]);										// "  ▲    ▼               終了 "
//			state = 2;	// 結果画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			break;
// GG129000(S) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
		case BAR_ID_TICKET:		// 駐車券
			BUZPI();
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[2]);	// "：駐車券"
			UsMnt_QR_DispTicQR(pTicQR, updown_page);
			state = 3;	// QR駐車券は1ページのみ
			break;
// GG129000(E) H.Fujinaga 2023/01/05 ゲート式車番チケットレスシステム対応（QR読取画面対応/QRデータ表示）
		default:
			BUZPIPI();
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			state = 3;	// 失敗画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
			break;
		}

		break;
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//	case 0x01:	// 異常
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	case 0x04:	// フォーマット不正
	case 0x05:	// 対象外
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
		BUZPIPI();
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
		state = 3;	// 失敗画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
		break;
	case 0x01:	// 異常
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	default:
		BUZPIPI();
// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);	// "ＱＲコードの　　　　　　　　　"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[4]);	// "リードテストに失敗しました　　"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5]);	// "ＱＲコードリーダーに　　　　　"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);	// "問題が無いか確認して下さい　　"
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
		state = 3;	// 失敗画面
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
		break;
	}

// MH810102(S) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない
//	if ( state == 3 ) {	// 失敗画面
//// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
////		dispmlclr(2, 6);
////		grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);	// "ＱＲコードの　　　　　　　　　"
////		grachr(3, 0, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[4]);	// "読取に失敗しました　　　　　　"
////		grachr(5, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5]);	// "リーダー又は　　　　　　　　　"
////		grachr(6, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);	// "ＱＲコードを確認して下さい　　"
//// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
//		Fun_Dsp(FUNMSG[8]);											// "　　　　　　　　　　　　 終了 "
//	}
	switch ( state ) {
	case 3:	// 失敗画面
		Fun_Dsp(FUNMSG[8]);											// "　　　　　　　　　　　　 終了 "
		break;
	case 4:	// 結果画面(上)
		Fun_Dsp(FUNMSG[11]);										// "        ▼               終了 "
		break;
	case 5:	// 結果画面(下)
		Fun_Dsp(FUNMSG[12]);										// "  ▲                     終了 "
		break;
	case 2:	// 結果画面
	default:
		Fun_Dsp(FUNMSG[6]);											// "  ▲    ▼               終了 "
		break;
	}
// MH810102(E) R.Endo 2021/04/09 車番チケットレス フェーズ2.5 #5473 【CR事業部要求】メンテナンスのQR読取結果表示でデータ表示切替を環状にしない

// MH810102(S) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善
	UsMnt_QR_ErrDisp(Lcd_QR_ErrDisp(pQR->result));
// MH810102(E) R.Endo 2021/04/01 車番チケットレス フェーズ2.5 #5454 【PK事業部要求】メンテナンスのQR読取結果表示内容改善

	return state;
}
// MH810102(E) R.Endo 2021/03/26 車番チケットレス フェーズ2.5 #5360 【PK事業部要求】システムメンテナンスのQRリーダーチェックの改善

//[]----------------------------------------------------------------------[]
/// @brief QRバージョン確認
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort	FncChk_QRverChk(void)
{
	ushort	msg;
	ushort	req	= 0;		// ﾃｽﾄ要求状態
	uchar	ret = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);		// "＜ＱＲリーダーチェック＞　　　"	
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[1]);		// "【バージョン確認】　　　　　　", 
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9]);			// "　　しばらくお待ち下さい　　　",
	Fun_Dsp(FUNMSG[00]);												// "　　　　　　　　　　　　　　　",
	
	if(PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_VER )){					// ﾊﾞｰｼﾞｮﾝ要求
		Lagtim( OPETCBNO, 6, 10*50 );									// QRﾊﾞｰｼﾞｮﾝ確認ﾀｲﾏ- 6(10s)起動		
		req = 1;													// QRリーダ制御応答待ち
	}else{
		BUZPIPI();
		return( MOD_EXT ); // ｺﾏﾝﾄﾞ送信ＮＧ
	}
	
	for ( ;ret == 0; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );													// QRﾊﾞｰｼﾞｮﾝ確認ﾀｲﾏ-ﾘｾｯﾄ
				return MOD_CUT;
				break;
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );													// QRﾊﾞｰｼﾞｮﾝ確認ﾀｲﾏ-ﾘｾｯﾄ
				return MOD_CHG;
				break;
			case LCD_MNT_QR_CTRL_RESP:													// QRﾘｰﾀﾞ制御応答(ﾒﾝﾃﾅﾝｽ)
				if(LcdRecv.lcdbm_rsp_QR_ctrl_res.ctrl_cd == LCDBM_QR_CTRL_VER ){			// QRﾘｰﾀﾞ制御応答（バージョン確認）
					if(req){
						Lagcan( OPETCBNO, 6 );
					 	ret = FncChk_QRverchk_result( LcdRecv.lcdbm_rsp_QR_ctrl_res.result );	// 結果表示
						BUZPI();
						req = 0;
					}
				}
				break;	
			case TIMEOUT6:
				if(req){
					ret = FncChk_QRverchk_result( LCDBM_RESUTL_NG );
				}
				req = 0;
				break;
			case KEY_TEN_F5:															// 終了
				if(!req){
					BUZPI();
					Lagcan( OPETCBNO, 6 );												// QRﾊﾞｰｼﾞｮﾝ確認ﾀｲﾏｰﾘｾｯﾄ
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
	return( 1 );
}

//[]----------------------------------------------------------------------[]
/// @brief QRバージョン確認結果表示
//[]----------------------------------------------------------------------[]
/// @param		: Nones
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/11/29<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	FncChk_QRverchk_result_info(uchar page)
{
	#define MAX_ONE_COL	28
	ushort	cnt = 0;
	uchar	tmpBuff[35];
	
	memset( tmpBuff, 0, sizeof(tmpBuff) );
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
	dispmlclr( 3, 5 );
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)	

	switch (page) {
		case QR_HINBAN:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[16]);	// 品番
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.part_no, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.part_no) );
			break;

		case QR_SERIAL:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[17]);	// シリアル番号
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.serial_no, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.serial_no) );
			break;

		case QR_VERSION:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[13]);	// バージョン
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.version, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.version) );
			break;
		default:
			break;
	}
	cnt = strlen( ( const char* )tmpBuff );
	if( cnt > MAX_ONE_COL ){
		cnt -= MAX_ONE_COL;
		grachr( 4, 2, MAX_ONE_COL,	0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[0] );
		grachr( 5, 2, cnt, 			0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[MAX_ONE_COL] );
	}else{
		grachr( 4, 2, cnt,			0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[0] );
	}
	return(1);
}
//[]----------------------------------------------------------------------[]
/// @brief QRバージョン確認結果
//[]----------------------------------------------------------------------[]
/// @param		: result 応答結果
/// @return 	: MOD_CHG / MOD_EXT / MOD_CUT
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/11/29<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	FncChk_QRverchk_result(uchar result){
	ushort		msg;		// 受信ﾒｯｾｰｼﾞ
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
	uchar		dsp = 0;	// 0:描画なし/ 1:画面表示
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:579,580,581)対応
//	int			page;		// ページ番号
	int			page = QR_HINBAN;		// ページ番号
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:579,580,581)対応
	
	dispclr();
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);		// "＜ＱＲリーダーチェック＞　　　"	
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[1]);		// "【バージョン確認】　　　　　　",   

	// 成功
	if( result == LCDBM_RESUTL_OK ){
		Fun_Dsp(FUNMSG[123]);											//"　⊂　　⊃　　　　　　　 終了 ",
		dsp = 1;
	}
	// 失敗
	else{
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[14]);	// "バージョンの取得に",  
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[18]);	// "　失敗しました　　　　　　　　",
		Fun_Dsp(FUNMSG[8]);												// "　　　　　　　　　　　　 終了 ", 
		BUZPIPI();
	}
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
	while(1){
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
		if (dsp == 1) {
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)			
			dsp = 0;
			// バージョン詳細画面表示
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
			FncChk_QRverchk_result_info( page );
			BUZPI();
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス (#4003 QRバージョンチェックで[品番]と[シリアル番号]と[バージョン]の情報が取得できない。)
		}
		// メッセージ受信
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
			case LCD_DISCONNECT:
				return MOD_CUT;
				break;

			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
				break;

			case KEY_TEN_F1:	// ページ移動（→）
				if( result == LCDBM_RESUTL_OK ){
					if (--page < 0) {
						page = QR_VER_RESULT_MAX - 1;
					}
					dsp = 1;
				}
				break;

			case KEY_TEN_F2:	// ページ移動（←）
				if( result == LCDBM_RESUTL_OK ){
					if (++page >= QR_VER_RESULT_MAX) {
						page = QR_HINBAN ;
					}
					dsp = 1;
				}
				break;

			case KEY_TEN_F5:		// 「終了」
				BUZPI();
				return MOD_EXT;
				break;

			default:
				break;
		}
	}
}
//[]----------------------------------------------------------------------[]
/// @brief リアルタイム通信チェックメニュー	
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
enum {
	UNSENDDATA_REAL_CLEAR = 0,
	// UNSENDDATA_REAL_SENT,
	CENTER_REAL_COM_TEST,
};
ushort 	FncChk_RealChk( void ) 
{
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;
	while (1) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[58] );		// "＜リアルタイム通信チェック＞　", 
		msg = Menu_Slt( REALTI_MENU, REAL_CHK_TBL, (char)REAL_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (msg) {

			// 未送信データクリア
			case UNSEND_REAL_CLEAR:
				msg = RealT_FncChk_Unsend(UNSENDDATA_REAL_CLEAR);
				break;

			// 未送信データ送信は不要となったが、念の為、処理はコメントアウトで残しておく
			// case UNSEND_REAL_SENT:
			// 	msg = RealT_FncChk_Unsend(UNSENDDATA_REAL_SENT);	// 未送信データ送信
			// 	break;

			// センター通信テスト
			case CENTER_REAL_TEST:
				msg = FncChk_CenterChk();
				break;

			// センター追番クリア
			case REAL_OIBANCLR:
				msg = FncChk_realt_OibanClear();
				break;

			case MOD_EXT:								// 終了（Ｆ５）
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
		if (msg == MOD_CHG || msg == MOD_CUT ){				// モードチェンジもしくは切断通知
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
/// @brief リアルタイム通信未送信データクリア	
//[]----------------------------------------------------------------------[]
/// @param		: type	0:未送信データクリア/1:未送信データ送信(未実装)
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort RealT_FncChk_Unsend( int type )
{
	ushort	msg;
	uchar	mode;		// 0:件数表示画面
						// 1:確認画面

	uchar	dsp;		// 0:描画なし
						// 1:件数更新
						// 2:画面更新
						// 3:件数表示

	int		item_pos;	// カーソル位置
	ushort	pos;
	uchar	fkey_enb = 0;
	uchar	page = 0;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	uchar RT_POS_TBL[RT_DATA_MAX] =
	{
		4,	// 精算
		6,	// 領収証
	};
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	dispclr();
	// タイトル表示
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[type] );	// "＜センター未送信データクリア＞"
	dsp      = 2;	// 画面更新(件数含む)
	mode     = 0;	// 件数表示画面
	item_pos = 0;	// 位置

	while(1){
		// 件数表示更新要求あり
		if( dsp ){
			// 件数表示画面？
			if( mode == 0 ){
				// 件数表示
				if( dsp == 2 || dsp == 3 ){
					realt_clr_dsp_count(page);						// 表示クリア("○○   件"だけにする)
					real_dsp_unflushed_count(0, page);				// 件数を表示
					realt_dsp_ccomchk_cursor(item_pos, 1, page);	// 反転表示
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//					Fun_Dsp(FUNMSG[(121+type)]);					// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
					Fun_Dsp(FUNMSG[(92)]);							// " ｸﾘｱ  全ｸﾘｱ   ⊂    ⊃   終了 "
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810103(s) 電子マネー対応 動作チェック（未送信クリアボタン修正）
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:577)対応
////					fkey_enb == 1;
//					fkey_enb = 1;
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:577)対応
// MH810103(e) 電子マネー対応 動作チェック（未送信クリアボタン修正）
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;

			case KEY_TEN_F5:			// 「終了」
				if( mode == 0 ){		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					BUZPI();
					Lagcan( OPETCBNO, 7 );
					return MOD_EXT;
				}
				else if( mode == 3 ){	// エラー画面
					BUZPI();
					dsp  = 2;			// 2:画面更新
					mode = 0;			// 0:件数表示画面
				}
				break;

			case KEY_TEN_F1:		// クリア
				// 件数表示画面？
				if( mode == 0 ){
					// ファンクションキー無効？
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// １～６行目までクリア
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//					pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
//					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[0] );				// "選択項目"
					pos = RT_POS_TBL[item_pos];
					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[item_pos] );		// "選択項目"
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:582)対応
//					grachr( 2, (ushort)(pos+2), 31, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_STR[type] );	// "データを消去します"
					grachr( 2, (ushort)(pos+2), 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_STR[type] );	// "データを消去します"
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:582)対応
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
					mode = 1;																				// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					}
				break;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//			case KEY_TEN_F3:					// 「はい」
//				if( mode == 1 ){	// 確認画面
//					BUZPI();
//					switch( type ){
//						case UNSENDDATA_REAL_CLEAR:	// 未送信データクリア
//							Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 未送信RT精算データを送信済みとする
//							break;
//						// 未送信データ送信は不要となったが、念の為、処理はコメントアウトで残しておく
//						// case UNSENDDATA_REAL_SENT:	// 未送信データ送信
//						// 	break;
//						default:
//							break;
//					}
//					dsp  = 2;			// 画面再描画
//					mode = 0;			// 0:件数表示画面
//				}
//				break;
			case KEY_TEN_F2:		// 全クリア
				// 件数表示画面？
				if ( mode == 0 ) {
					// ファンクションキー無効？
					if ( fkey_enb != 0 ) {
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// １～６行目までクリア

					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX] );	// "全"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );					// "データを消去します"
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);															// "　　　　　　 はい いいえ　　　"
					mode = 2;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				}
				break;

			case KEY_TEN_F3:
				switch ( mode ) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					case 0:			// カーソル移動
						BUZPI();
						realt_dsp_ccomchk_cursor( item_pos, 0, page );		// 正転表示
						if ( ++item_pos >= RT_DATA_MAX ) {
							item_pos = 0;
						}
						realt_dsp_ccomchk_cursor( item_pos, 1, page );		// 反転表示
						break;
					case 1:			// クリア
						BUZPI();
						switch ( item_pos ) {
							case 0:		// QR確定
								Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );
								break;
							case 1:		// ﾚｰﾝﾓﾆﾀ
								Ope_Log_UnreadToRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );
								break;
							default:
								break;
						}
						dsp  = 2;		// 画面再描画
						mode = 0;
						break;
					case 2:			// 全クリア
						BUZPI();
						Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );
						Ope_Log_UnreadToRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );
						dsp  = 2;		// 画面再描画
						mode = 0;
						break;
					case 3:
					default:
						break;
				}
				break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

			case KEY_TEN_F4:						// 「いいえ」
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//				if( mode == 1){	// 確認画面
				if ( (mode == 1) || (mode == 2) ) {
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					BUZPI();
					dsp  = 2;	// 画面再描画
					mode = 0;	// 0:件数表示画面
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				else if ( mode == 0 ) {
					// カーソル移動
					BUZPI();
					realt_dsp_ccomchk_cursor( item_pos, 0, page );		// 正転表示
					if ( --item_pos < 0 ) {
						item_pos = RT_DATA_MAX - 1;
					}
					realt_dsp_ccomchk_cursor( item_pos, 1, page );		// 反転表示
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				break;

			default:
				break;
		}
	}
}

#define DATA_LINE_MAX	6
#define PAGE_ITEM_MAX	(DATA_LINE_MAX * 2)
// 件数を初期化
void	realt_clr_dsp_count(uchar page)
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset(RT_PreCount, 0xFF, sizeof(ulong)*RT_DATA_MAX);
	dispmlclr(1, 6);
	for (i=1, j=offset; (i<=DATA_LINE_MAX) && (j<RT_DATA_MAX); i++) {
		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[j++] );
		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX] );	// "件"
		
		if ((i <= DATA_LINE_MAX) && (j < RT_DATA_MAX)) {
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
		}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		if( RT_DATA_MAX > 6 ){																	//左側の項目が6個以上であれば右側の項目を表示
		if((i*2) <= RT_DATA_MAX){
			// 右側表示
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[j++] );			//入庫
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX]  );	// "件"
		}
	}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	Fun_Dsp(FUNMSG[8]);		// "　　　　　　　　　　　　 終了 "
	Fun_Dsp(FUNMSG[128]);	// "            ⊂    ⊃   終了 "
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
}

// 未送信データ数を取得して表示する
ushort real_dsp_unflushed_count( uchar type, uchar page )
{
	ushort count = 0;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	ushort count_receipt = 0;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	char  numStr[10];

	// RT精算データ個数取得
	count = Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	count_receipt = Ope_Log_UnreadCountGet(eLOG_RTRECEIPT, eLOG_TARGET_LCD);
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	// 件数表示
	if( count != RT_PreCount[RTCOM_DATA_PAY] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count );
		// ３桁
		grachr( 1, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		RT_PreCount[RTCOM_DATA_PAY] = count;
	}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	// 件数表示(領収証データ)
	if ( count_receipt != RT_PreCount[RTCOM_DATA_RECEIPT] ) {
		memset(numStr, 0x00, sizeof(numStr));
		sprintf(numStr, "%4d", count_receipt);
		// ３桁
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr);
		RT_PreCount[RTCOM_DATA_RECEIPT] = count_receipt;
	}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	return 0;
}
// カーソル表示
uchar realt_dsp_ccomchk_cursor(int index, ushort mode, uchar page)
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
	ushort size = 8;
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
		realt_clr_dsp_count(new_page);
	}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF,REAL_UNSEND_TITLE[new_page]);
	grachr(row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[index]);
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	return new_page;
}

static uchar ChkNo;		// 通信チェック要求データ電文作成用の通信ﾁｪｯｸNo.

//[]----------------------------------------------------------------------[]
/// @brief リアルタイム通信センター通信テスト
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort FncChk_CenterChk( void )
{
	ushort	msg;
	ushort	req	= 0;		// 1:結果要求状態、2:結果表示状態

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[1] );	// "＜センター通信テスト＞　　　　"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[4] );		// "センター通信テストを行います　"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[5] );		// "よろしいですか？　　　　　　　"
	Fun_Dsp( FUNMSG[19] );												// "　　　　　　 はい いいえ　　　"


	while(1) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );												// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ-ﾘｾｯﾄ
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );												// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ-ﾘｾｯﾄ
			return MOD_CHG;
			break;

		case KEY_TEN_F4:														//いいえ
			if( !req ){	
				BUZPI();
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F3:														//はい
			if( !req ){
				BUZPI();
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );		// "　　　　 通信テスト中 　　　　"
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9] );		// "　　しばらくお待ち下さい　　　"
				Fun_Dsp( FUNMSG[00]);												// "　　　　　　　　　　　　　　　"
				// リアルタイム通信コマンドセット
				if( PKTcmd_mnt_rt_con_req() ){
					Lagtim( OPETCBNO, 6, 10*50 );									// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ- 6(10s)起動
					req = 1;														// ﾘｱﾙﾀｲﾑ通信制御応答待ち
				}
				else{
					return( MOD_EXT ); // ｺﾏﾝﾄﾞ送信ＮＧ
				}
			}
			break;

		// ﾘｱﾙﾀｲﾑ通信疎通結果
		case LCD_MNT_REALTIME_RESULT:
			if( req == 1 ){
				Lagcan( OPETCBNO, 6 );												// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ- 6(10s)停止
				BUZPI();
				Fun_Dsp(FUNMSG[8]);													// "　　　　　　　　　　　　 終了 ",
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "　　　　通信テスト終了　　　　"

				displclr(3);
				if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:正常,01H:通信異常(TCP接続不可),02H:タイムアウト)
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[00]);	// "センターとの通信は正常です　　",
				}
				else if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "センターとの通信に失敗しました",
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);	// "(02：タイムアウト)　　　　　　",
				}
				else{
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "センターとの通信に失敗しました",
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[2] );	// "(01：通信異常(TCP接続不可))　",
				}
			}
			req = 2;
			break;	

		case TIMEOUT6:
			if(req == 1){
				BUZPI();
				displclr(3);
				Lagcan( OPETCBNO, 6 );												// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ- 6(10s)停止
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "　　　　通信テスト終了　　　　"
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1]);		// "センターとの通信に失敗しました"
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);		// "(02：タイムアウト)　　　　　　"
				Fun_Dsp(FUNMSG[8]);													// "　　　　　　　　　　　　 終了 "
			}
			req = 2;
			break;

		case KEY_TEN_F5:	// 終了
			if(req == 2){
				BUZPI();
				Lagcan( OPETCBNO, 6 );												// ﾘｱﾙﾀｲﾑ通信確認ﾀｲﾏ- 6(10s)停止
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief リアルタイム通信センター追番クリア＿種別選択画面
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void realt_OibanClrDsp( void )
{
	dispmlclr(1, 6);

	// １行目
	grachr( 1,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_PAY] );		// "精算　　"
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
	// 1行目
	grachr( 1, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_PAY_AFTER] );		// "後日精算"
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	grachr(2, 0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_RECEIPT]);	// "領収証　"
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
}

//[]----------------------------------------------------------------------[]
/// @briefリアルタイム通信センター追番クリア＿反転処理
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void realt_OibanCursorDsp( uchar index, ushort mode )
{
	ushort row;
	ushort col;
	ushort size = 4;

	if( index >= REAL_SEQNO_TYPE_COUNT ){
		return;
	}

	row = (ushort)(index/2+1);
	col = (ushort)(index%2);
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
	if(index == 1 ){
		size = 8;
	}
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	if ( index == 2 ) {
		size = 6;
	}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[index] );
}

//[]----------------------------------------------------------------------[]
/// @brief リアルタイム通信センター追番クリア
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort FncChk_realt_OibanClear( void )
{
	ushort	msg;
	uchar	mode = 0;		// 0:種別選択画面/ 1:クリア確認画面
	uchar	dsp = 1;		// 0:描画なし/ 1:画面表示
	uchar	item_pos = 0;	// カーソル位置
	ushort	pos;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
	ushort  i = 0;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応

	// タイトル表示
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "＜センター追番クリア＞　　　　"

	for ( ; ; ) {
		// 件数表示更新要求あり
		if( dsp ){
			if( mode == 0 ){
				realt_OibanClrDsp();					// 種別選択画面表示
				realt_OibanCursorDsp( item_pos, 1 );	// 反転表示
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
//				Fun_Dsp(FUNMSG[121]);				// " ｸﾘｱ                     終了 "
				Fun_Dsp(FUNMSG[119]);				// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter

			case LCD_DISCONNECT:
				return MOD_CUT;

			case KEY_MODECHG:
				return MOD_CHG;

			// 「終了」
			case KEY_TEN_F5:
				// 種別選択画面？
				if( mode == 0 ){
					BUZPI();
					return MOD_EXT;
				}
				break;

			case KEY_TEN_F1:		// クリア
				// 種別選択画面？
				if( mode == 0 ){
					BUZPI();
					dispmlclr( 1, 6 );
					pos = 4;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					if( item_pos == 1 ){
						pos = 8;	// 4文字なので8Byte
					}
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					if ( item_pos == 2 ) {
						pos = 6;
					}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[item_pos] );				// "選択項目"
					grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "追番をクリアします"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
					mode = 1;		// 0:種別選択画面/ 1:クリア確認画面
				}
				break;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
			case KEY_TEN_F2:		// 全クリア
				if (mode == 0) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					BUZPI();
					
					dispmlclr(1, 6);
					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );				// "全"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "追番をクリアします"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);								// "　　　　　　 はい いいえ　　　"
					mode = 2;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				}
				break;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応

			case KEY_TEN_F3:
				switch( mode ){
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					case 0:			// カーソル移動
						BUZPI();
						realt_OibanCursorDsp(item_pos, 0);			// 正転表示
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//						if (++item_pos >= 2) {
						if ( ++item_pos >= REAL_SEQNO_TYPE_COUNT ) {
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
							item_pos = 0;
						}
						realt_OibanCursorDsp(item_pos, 1);			// 反転表示
						break;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					// クリア確認画面？
					case 1:
						BUZPI();
						switch( item_pos ){
							case 0:									// センター追番（精算）
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
//								REAL_SeqNo[REAL_SEQNO_PAY] = 1;
								REAL_SeqNo[REAL_SEQNO_PAY] = 0;
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
								break;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
							case 1:
								REAL_SeqNo[REAL_SEQNO_PAY_AFTER] = 0;
								break;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
							case 2:
								REAL_SeqNo[REAL_SEQNO_RECEIPT] = 0;
								break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
						}
						dsp  = 1;	// 画面再描画
						mode = 0;
						break;
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					case 2:			// 全クリア実行
						BUZPI();
						for(i = 0; i < REAL_SEQNO_TYPE_COUNT; ++i) {
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
//							REAL_SeqNo[i] = 1;
							REAL_SeqNo[i] = 0;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
						}
						dsp  = 1;	// 画面再描画
						mode = 0;
						break;
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					default:
						break;
				}
				break;

			// 「いいえ」
			case KEY_TEN_F4:
				// クリア確認画面？
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
//				if( mode == 1 ){
				if( mode == 1 || mode == 2){
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
					BUZPI();
					dsp  = 1;	// 画面再描画
					mode = 0;
				}
				break;

			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信チェックメニュー
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/27<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort FncChk_DC_Chk( void )
{
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;

	while (1) {

		// タイトル表示
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[61] );		// DC-NET通信チェック

		msg = Menu_Slt( DC_NET_MENU, DCNET_CHK_TBL, (char)DC_NET_CHK_TBL, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( msg ){

			// 未送信データクリア
			case DC_UNSEND_CLEAR:
				msg = FncChk_DC_UnsendClear();
				break;

			// センター通信テスト
			case DC_CONNECT_TEST:
				msg = FncChk_DC_ConnectCheck();
				break;

			// センター追番クリア
			case DC_OIBANCLR:
				msg = FncChk_DC_OibanClear();
				break;

			// 終了(F5)
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if( msg == MOD_CHG ){
		if( msg == MOD_CHG || msg == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信未送信データクリア（件数をクリアする処理）
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// イメージ↓↓↓
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//// ------------------------------- //
//// QR登録       件
////
////
////
////
////
//// 　　　　　　　　　　　　 終了 
//// ------------------------------- //
// ------------------------------- //
// QR登録       件　ﾚｰﾝﾓﾆﾀ       件
//
//
//
//
//
// ｸﾘｱ　　全ｸﾘｱ　　←　　　→　　　終了 
// ------------------------------- //
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
void dc_clr_dsp_count( uchar page )
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset( DC_PreCount, 0xFF, sizeof(ulong)*DC_DATA_MAX );

	dispmlclr( 1, 6 );	// 1～6行目クリア

	for( i=1, j=offset; (i<=DATA_LINE_MAX) && (j<DC_DATA_MAX); i++ ){
		grachr( i,  0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[j++] );
		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[DC_DATA_MAX] );	// "件"

		if( (i <= DATA_LINE_MAX) && (j < DC_DATA_MAX) ){
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
		}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		if( DC_DATA_MAX > 6 ){																	//左側の項目が6個以上であれば右側の項目を表示
		if((i*2) <= DC_DATA_MAX){
			// 右側表示
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[j++] );			//入庫
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[DC_DATA_MAX]  );	// "件"
		}
	}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	Fun_Dsp(FUNMSG[8]);		// "　　　　　　　　　　　　 終了 "
	Fun_Dsp(FUNMSG[128]);	// "            ⊂    ⊃   終了 "
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信未送信データクリア（未送信データ数を取得して表示する）
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort dc_dsp_unflushed_count( void )
{
	ushort count = 0;
	char  numStr[10];
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	ushort count_lane = 0;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	// QR確定・取消データ個数取得
	count = Ope_Log_UnreadCountGet( eLOG_DC_QR, eLOG_TARGET_LCD );
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	count_lane = Ope_Log_UnreadCountGet( eLOG_DC_LANE, eLOG_TARGET_LCD );
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	// 件数表示
	if( count != DC_PreCount[DC_DATA_QR] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count );
		// ３桁
		grachr( 1, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		DC_PreCount[DC_DATA_QR] = count;
	}

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	// 件数表示(レーンモニタデータ)
	if( count_lane != DC_PreCount[DC_DATA_LANE] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count_lane );
		// ３桁
		grachr( 1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		DC_PreCount[DC_DATA_QR] = count_lane;
	}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	return 0;
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信未送信データクリア（カーソル表示）
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar dc_dsp_ccomchk_cursor( int index, ushort mode, uchar page )
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
	ushort size = 8;
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		realt_clr_dsp_count(new_page);
		dc_clr_dsp_count(new_page);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[new_page]);
	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[index]);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	return new_page;
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信未送信データクリア（メイン処理）
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/17<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort FncChk_DC_UnsendClear( void )
{
	ushort	msg;
	uchar	mode;		// 0:件数表示画面
						// 1:確認画面

	uchar	dsp;		// 0:描画なし
						// 1:件数更新
						// 2:画面更新
						// 3:件数表示

	int		item_pos;	// カーソル位置
	ushort	pos;
	uchar	fkey_enb = 0;
	uchar	page = 0;

	uchar DC_POS_TBL[DC_DATA_MAX] =
	{
		6,		// QR確定
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		6,		// レーンモニタ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	};

	// タイトル表示
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[0] );	// "＜センター未送信データクリア＞"

	dsp      = 2;	// 画面更新(件数含む)
	mode     = 0;	// 件数表示画面
	item_pos = 0;	// 位置

	while(1){
		// 件数表示更新要求あり
		if( dsp ){
			// 件数表示画面？
			if( mode == 0 ){
				// 件数表示
				if( dsp == 2 || dsp == 3 ){
					dc_clr_dsp_count(page);							// 表示クリア("○○   件"だけにする)
					dc_dsp_unflushed_count();						// 件数を表示
					dc_dsp_ccomchk_cursor( item_pos, 1, page );		// 反転表示
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//					Fun_Dsp( FUNMSG[(121)] );						// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
					Fun_Dsp( FUNMSG[(92)] );						// " ｸﾘｱ  全ｸﾘｱ   ⊂    ⊃   終了 "
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// MH810103(s) 電子マネー対応 動作チェック（未送信クリアボタン修正）
//// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:577)対応
////					fkey_enb == 1;
//					fkey_enb = 1;
//// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:577)対応
// MH810103(e) 電子マネー対応 動作チェック（未送信クリアボタン修正）
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){

			// 切断検知
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)

			// ドア閉
			case KEY_MODECHG:
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;

			// 「終了」
			case KEY_TEN_F5:
				if( mode == 0 ){		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					BUZPI();
					Lagcan( OPETCBNO, 7 );
					return MOD_EXT;
				}
				else if( mode == 3 ){	// エラー画面
					BUZPI();
					dsp  = 2;
					mode = 0;
				}
				break;

			// 「クリア」
			case KEY_TEN_F1:
				// 件数表示画面？
				if( mode == 0 ){
					// ファンクションキー無効？
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// １～６行目までクリア

					pos = DC_POS_TBL[item_pos];
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[0] );				// "選択項目"
					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[item_pos] );			// "選択項目"
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:583)対応
//					grachr( 2, (ushort)(pos+2), 31, 0, COLOR_BLACK, LCD_BLINK_OFF, DCNET_UNSEND_STR[0] );	// "データを消去します"
					grachr( 2, (ushort)(pos+2), 30, 0, COLOR_BLACK, LCD_BLINK_OFF, DCNET_UNSEND_STR[0] );	// "データを消去します"
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:583)対応
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp( FUNMSG[19] );																	// "　　　　　　 はい いいえ　　　"
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応/コメント）
//					mode = 1;	// 確認画面中モードへ
					mode = 1;	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応/コメント）
				}
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			case KEY_TEN_F2:		// 全クリア
				// 件数表示画面？
				if( mode == 0 ){
					// ファンクションキー無効？
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// １～６行目までクリア

					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_TYPE_COUNT] );	// "全"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );					// "データを消去します"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);															// "　　　　　　 はい いいえ　　　"
					mode = 2;		// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//			// 「はい」
//			case KEY_TEN_F3:
//				// 確認画面中？
//				if( mode == 1 ){
//					BUZPI();
//					Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
//					dsp  = 2;		// 画面再描画
//					mode = 0;
//				}
//				break;
			case KEY_TEN_F3:
				switch (mode) {	// 0:件数表示画面/ 1:確認画面（指定ﾃﾞｰﾀ）/ 2:確認画面（全ﾃﾞｰﾀ）/ 3:エラー画面
					case 0:			// カーソル移動
						BUZPI();
						dc_dsp_ccomchk_cursor( item_pos, 0, page );		// 正転表示
						if (++item_pos >= 2) {
							item_pos = 0;
						}
						dc_dsp_ccomchk_cursor( item_pos, 1, page );		// 反転表示
						break;
					case 1:			// クリア
						BUZPI();
						switch (item_pos) {
							case 0:		// QR確定
								Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
								break;
							case 1:		// ﾚｰﾝﾓﾆﾀ
								Ope_Log_UnreadToRead( eLOG_DC_LANE, eLOG_TARGET_LCD );
								break;
							default:
								break;
						}
						dsp  = 2;		// 画面再描画
						mode = 0;
						break;
					case 2:			// 全クリア
						BUZPI();
						Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
						Ope_Log_UnreadToRead( eLOG_DC_LANE, eLOG_TARGET_LCD );
						dsp  = 2;		// 画面再描画
						mode = 0;
						break;
					case 3:
					default:
						break;
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			// 「いいえ」
			case KEY_TEN_F4:
				// 確認画面中？
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//				if( mode == 1){
				if ((mode == 1) || (mode == 2)) {
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					BUZPI();
					dsp  = 2;	// 画面再描画
					mode = 0;
				}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				else if (mode == 0) {
					// カーソル移動
					BUZPI();
					dc_dsp_ccomchk_cursor( item_pos, 0, page );		// 正転表示
					if (--item_pos < 0) {
						item_pos = DC_SEQNO_TYPE_COUNT - 1;
					}
					dc_dsp_ccomchk_cursor( item_pos, 1, page );		// 反転表示
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				break;

			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信センター通信テスト
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort FncChk_DC_ConnectCheck( void )
{
	ushort	msg;
	ushort	req	= 0;		// 0:結果表示状態、1:結果要求状態

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[1] );	// "＜センター通信テスト＞　　　　"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[4] );		// "センター通信テストを行います　"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[5] );		// "よろしいですか？　　　　　　　"
	Fun_Dsp( FUNMSG[19] );												// "　　　　　　 はい いいえ　　　"


	while(1) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			// LCD切断検知
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
			// ドア開
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return MOD_CHG;
				break;

			// 「いいえ」
			case KEY_TEN_F4:
				if( !req ){	
					BUZPI();
					return MOD_EXT;
				}
				break;

			// 「はい」
			case KEY_TEN_F3:
				if( !req ){
					BUZPI();
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );		// "　　　　 通信テスト中 　　　　"
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9] );		// "　　しばらくお待ち下さい　　　"
					Fun_Dsp( FUNMSG[00]);												// "　　　　　　　　　　　　　　　"
					// リアルタイム通信コマンドセット
					if( PKTcmd_mnt_dc_con_req() ){
						Lagtim( OPETCBNO, 6, 10*50 );
						req = 1;														// DC-NET通信制御応答待ち
					}
					else{
						return( MOD_EXT ); // ｺﾏﾝﾄﾞ送信ＮＧ
					}
				}
				break;

			// DC-NET通信疎通結果
			case LCD_MNT_DCNET_RESULT:
				if( req == 1 ){
					Lagcan( OPETCBNO, 6 );
					BUZPI();
					Fun_Dsp(FUNMSG[8]);													// "　　　　　　　　　　　　 終了 ",
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "　　　　通信テスト終了　　　　"

					displclr(3);
// MH810100(S) 2020/10/08 #4914  【検証課指摘事項】LANケーブルが刺さっていない状態でも動作チェックのセンター通信テストが正常になる(No.95)
//					if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:正常,01H:通信異常(TCP接続不可),02H:タイムアウト)
					if( LcdRecv.lcdbm_rsp_dc_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:正常,01H:通信異常(TCP接続不可),02H:タイムアウト)
// MH810100(E) 2020/10/08 #4914  【検証課指摘事項】LANケーブルが刺さっていない状態でも動作チェックのセンター通信テストが正常になる(No.95)
						grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[00]);	// "センターとの通信は正常です　　",
					}
// MH810100(S) 2020/10/08 #4914  【検証課指摘事項】LANケーブルが刺さっていない状態でも動作チェックのセンター通信テストが正常になる(No.95)
//					else if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
					else if( LcdRecv.lcdbm_rsp_dc_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
// MH810100(E) 2020/10/08 #4914  【検証課指摘事項】LANケーブルが刺さっていない状態でも動作チェックのセンター通信テストが正常になる(No.95)
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "センターとの通信に失敗しました",
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);	// "(02：タイムアウト)　　　　　　",
					}
					else{
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "センターとの通信に失敗しました",
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[2] );	// "(01：通信異常(TCP接続不可))　",
					}
				}
				req = 2;
				break;	

			case TIMEOUT6:
				if(req == 1){
					BUZPI();
					displclr(3);
					Lagcan( OPETCBNO, 6 );
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "　　　　通信テスト終了　　　　"
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1]);		// "センターとの通信に失敗しました"
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);		// "(02：タイムアウト)　　　　　　"
					Fun_Dsp(FUNMSG[8]);													// "　　　　　　　　　　　　 終了 "
				}
				req = 2;
				break;

			case KEY_TEN_F5:
				if( req == 2 ){
					BUZPI();
					Lagcan( OPETCBNO, 6 );
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
}
// MH810100(E) Y.Yamauchi 2019/10/08 車番チケットレス(メンテナンス)

// MH810100(S) Y.Yamauchi 2020/01/07 車番チケットレス(メンテナンス)
//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信センター追番クリア＿種別選択画面
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_OibanClrDsp( void )
{
	dispmlclr(1, 6);

	// １行目
	grachr( 1,  0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_QR] );		// "QR登録　"
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	grachr( 1, 15, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_LANE] );	// "ﾚｰﾝﾓﾆﾀ　 "
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信センター追番クリア＿反転処理
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_OibanCursorDsp( uchar index, ushort mode )
{
	ushort row;
	ushort col;
	ushort size = 6;

	if( index >= DC_SEQNO_TYPE_COUNT ){
		return;
	}

	row = (ushort)(index/2+1);
	col = (ushort)(index%2);

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[index] );
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET通信センター追番クリア
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort FncChk_DC_OibanClear( void )
{
	ushort	msg;
	uchar	mode = 0;		// 0:種別選択画面/ 1:クリア確認画面
	uchar	dsp = 1;		// 0:描画なし/ 1:画面表示
	uchar	item_pos = 0;	// カーソル位置
	ushort	pos;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	uchar	i;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	// タイトル表示
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "＜センター追番クリア＞　　　　"

	for ( ; ; ) {
		// 件数表示更新要求あり
		if( dsp ){
			if( mode == 0 ){
				DC_OibanClrDsp();					// 種別選択画面表示
				DC_OibanCursorDsp( item_pos, 1 );	// 反転表示
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//				Fun_Dsp(FUNMSG[121]);				// " ｸﾘｱ                     終了 "
				Fun_Dsp(FUNMSG[119]);				// " ｸﾘｱ　全ｸﾘｱ 　⊃　       終了 "
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter

			case LCD_DISCONNECT:
				return MOD_CUT;

			case KEY_MODECHG:
				return MOD_CHG;

			// 「終了」
			case KEY_TEN_F5:
				// 種別選択画面？
				if( mode == 0 ){
					BUZPI();
					return MOD_EXT;
				}
				break;

			case KEY_TEN_F1:		// クリア
				// 種別選択画面？
				if( mode == 0 ){
					BUZPI();
					dispmlclr( 1, 6 );
					pos = 6;
					grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[item_pos] );				// "選択項目"
					grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "追番をクリアします"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
					mode = 1;		// 0:種別選択画面/ 1:クリア確認画面
				}
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			case KEY_TEN_F2:		// 全クリア
				if (mode == 0) {
					BUZPI();
					dispmlclr(1, 6);
					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_DATA_MAX] );				// "全"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "追番をクリアします"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "　　　　よろしいですか？　　　"
					Fun_Dsp(FUNMSG[19]);																	// "　　　　　　 はい いいえ　　　"
					mode = 2;		// 0:件数表示画面/ 1:クリア確認画面/ 2:全クリア確認画面
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			case KEY_TEN_F3:
				switch( mode ){
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// カーソル移動？
					case 0:
						BUZPI();
						DC_OibanCursorDsp(item_pos, 0);			// 正転表示
						if (++item_pos >= 2) {
							item_pos = 0;
						}
						DC_OibanCursorDsp(item_pos, 1);			// 反転表示
						break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// クリア確認画面？
					case 1:
						BUZPI();
						switch( item_pos ){
							case 0:									// センター追番（QR確定・取消データ）
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
//								DC_SeqNo[DC_SEQNO_QR] = 1;
								DC_SeqNo[DC_SEQNO_QR] = 0;
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4021 センター追番クリア後、不正フラグが立たない)
								break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							case 1:									// センター追番（レーンモニタデータ）
								DC_SeqNo[DC_SEQNO_LANE] = 0;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
						}
						dsp  = 1;	// 画面再描画
						mode = 0;
						break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// 全クリア実行？
					case 2:
						BUZPI();
						for(i = 0; i < DC_SEQNO_TYPE_COUNT; ++i) {
							DC_SeqNo[i] = 0;
						}
						dsp  = 1;	// 画面再描画
						mode = 0;
						break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					default:
						break;
			}
			break;

			// 「いいえ」
			case KEY_TEN_F4:
				// クリア確認画面？
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//				if( mode == 1 ){
				if ((mode == 1) || (mode == 2)) {	// 0:件数表示画面/ 1:クリア確認画面/ 2:全クリア確認画面
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					BUZPI();
					dsp  = 1;	// 画面再描画
					mode = 0;
				}
				break;

			default:
				break;
		}
	}
}
// MH810100(E) Y.Yamauchi 2020/01/07 車番チケットレス(メンテナンス)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
/*[]----------------------------------------------------------------------[]*/
/*| NTNETﾁｪｯｸ                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ntnet( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Ntnet( void )
{
	ushort	usFntnetEvent;
	ushort	ret = 0;
	char	wk[2];
	char	org[2];

	/* 設定ﾊﾟﾗﾒｰﾀ-NTNET接続参照 */
	if (! (_is_ntnet()) ) {			// LOCAL NTNETでは無い
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[0]);								/* "＜ＮＴ－ＮＥＴ＞　　　　　　　" */
		usFntnetEvent = Menu_Slt(FNTNETMENU, FNTNET_CHK_TBL, (char)FNTNET_CHK_MAX, (char)1);

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch(usFntnetEvent){		/* FunctionKey Enter */
		case FNTNET1_CHK:	// １．データクリア
			usFntnetEvent = check_nt_dtclr();
			break;
// MH341107(S) K.Onodera 2016/11/11 AI-V対応(端末間)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間の追番クリアメニューを削除）
//		case CCOM_SEQCLR:	// ２．センター追番クリア
//			if( prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0 ){
//				usFntnetEvent = FncChk_cOibanClr();
//			}else{
//				BUZPIPI();
//			}
//			break;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間の追番クリアメニューを削除）
// MH341107(E) K.Onodera 2016/11/11 AI-V対応(端末間)
		case MOD_EXT:
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			ret = MOD_EXT;
			break;
		default:
			break;
		}
		if (ret == MOD_EXT) {				// KEY_TEN_F5
			break;
		}
		if (usFntnetEvent == MOD_CHG) {		// KEY_MODECHG
			ret = MOD_CHG;
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| NTNETﾁｪｯｸ - ﾃﾞｰﾀｸﾘｱ                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_nt_dtclr( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
ushort	check_nt_dtclr( void )
{
	ushort	msg;
	ushort	w_Count;
	t_NtBufCount	buf;
// MH364300 GG119A34(S) 改善連絡表No.83対応
	uchar	mode = 0;		// 画面モード(0：初期状態、1：F2キー押下後)
// MH364300 GG119A34(E) 改善連絡表No.83対応

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[0]);	// "＜ＮＴ－ＮＥＴチェック＞　　　"
	memset(&buf,0x00,sizeof(buf));
	NTBUF_GetBufCount(&buf, FALSE);									// データ件数を取得する

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[0]);	// "精算　　9999件　モニタ  9999件" ,
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[5]);	// "精算　　9999件　集計　  9999件" ,
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
	if( buf.sndbuf_sale > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_sale;
	}
	DataCntDsp( w_Count, 0, 1 );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	if( buf.sndbuf_monitor > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_monitor;
//	}
//	DataCntDsp( w_Count, 1, 1 );
//
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[1]);	// "集計　　9999件　操作　　9999件" ,
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
	if( buf.sndbuf_ttotal > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_ttotal;
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	DataCntDsp( w_Count, 0, 2 );
//	if( buf.sndbuf_ope_monitor > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_ope_monitor;
//	}
//	DataCntDsp( w_Count, 1, 2 );
	w_Count = 0;													// フェーズ1では送信しないため常に0件とする
	DataCntDsp( w_Count, 1, 1 );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[2]);	// "エラー　9999件　金銭管理9999件" ,
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[6]);	// "エラー　9999件　金銭管理9999件" ,
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
	if( buf.sndbuf_error > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort) buf.sndbuf_error;
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	DataCntDsp( w_Count, 0, 3 );
	DataCntDsp( w_Count, 0, 2 );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
	if( buf.sndbuf_money > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_money;
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	DataCntDsp( w_Count, 1, 3 );
	w_Count = 0;													// フェーズ1では送信しないため常に0件とする
	DataCntDsp( w_Count, 1, 2 );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//// MH341107(S) K.Onodera 2016/11/11 AI-V対応(端末間)
//	if( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0 ){			// 既存形式の場合
//// MH341107(E) K.Onodera 2016/11/11 AI-V対応(端末間)
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[3]);	// "アラーム9999件　その他　9999件" ,
//	if( buf.sndbuf_alarm > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_alarm;
//	}
//	DataCntDsp( w_Count, 0, 4 );
//	w_Count = (ushort)(	buf.sndbuf_coin + 			// ｺｲﾝ金庫集計（現在未使用）
//					   	buf.sndbuf_note +			// 紙幣金庫集計（現在未使用）
//					   	buf.sndbuf_prior +			// 優先ﾊﾞｯﾌｧ
//						buf.sndbuf_normal );		// 通常ﾊﾞｯﾌｧ
//    if( w_Count > 9999 ) {
//		w_Count = 9999;
//    }
//	DataCntDsp( w_Count, 1, 4 );
//// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
////	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[4]);	// "入庫　　9999件　　　　　　　　" ,
////	if( buf.sndbuf_incar > 9999 ) {
////		w_Count = 9999;
////	} else {
////		w_Count = (ushort) buf.sndbuf_incar;
////	}
////	DataCntDsp( w_Count, 0, 5 );
//// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
//// MH341107(S) K.Onodera 2016/11/11 AI-V対応(端末間)
//	} else {
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[5]);	// "アラーム9999件　釣銭管理9999件" ,
//		if( buf.sndbuf_alarm > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort)buf.sndbuf_alarm;
//		}
//		DataCntDsp( w_Count, 0, 4 );
//		w_Count = (ushort)buf.sndbuf_turi;	 			// 釣銭管理
//		if( w_Count > 9999 ){
//			w_Count = 9999;
//		}
//		DataCntDsp( w_Count, 1, 4 );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[6]);	// "入庫　　9999件　その他　9999件" ,
//		if( buf.sndbuf_incar > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort) buf.sndbuf_incar;
//		}
//		DataCntDsp( w_Count, 0, 5 );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "その他　9999件　　　　　　　　" ,
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "その他　9999件　　　　　　　　" ,
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//		w_Count = (ushort)(	buf.sndbuf_coin + 			// ｺｲﾝ金庫集計（現在未使用）
//						   	buf.sndbuf_note +			// 紙幣金庫集計（現在未使用）
		w_Count = (ushort)(	
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
						   	buf.sndbuf_prior +			// 優先ﾊﾞｯﾌｧ
							buf.sndbuf_normal );		// 通常ﾊﾞｯﾌｧ
		if( w_Count > 9999 ){
			w_Count = 9999;
		}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
//		DataCntDsp( w_Count, 1, 5 );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//		DataCntDsp( w_Count, 0, 5 );
		DataCntDsp( w_Count, 0, 3 );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（入庫））
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
//	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（NT-NETチェック画面に送信対象のみ表示する）
// MH341107(E) K.Onodera 2016/11/11 AI-V対応(端末間)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（出庫））
//// MH364300 GG119A34(S) 改善連絡表No.83対応
//	if (IS_SEND_OUTCAR_DATA) {
//		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "出庫　　9999件　　　　　　　　" ,
//		if( buf.sndbuf_outcar > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort)buf.sndbuf_outcar;
//		}
//		DataCntDsp( w_Count, 0, 6 );
//	}
//// MH364300 GG119A34(E) 改善連絡表No.83対応
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-4100未使用ログ（出庫））

// MH364300 GG119A34(S) 改善連絡表No.83対応
//	grachr(6, 1, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[1]);	// データをクリアしますか？ 
//	Fun_Dsp(FUNMSG[19]);				/* "　　　　　　 はい いいえ　　　" */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	Fun_Dsp(FUNMSG[121]);				/* "    　全ｸﾘｱ 　　　       終了 " */
	Fun_Dsp(FUNMSG[129]);				/* "    　全ｸﾘｱ 　　　       終了 " */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
// MH364300 GG119A34(E) 改善連絡表No.83対応

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
		case KEY_MODECHG:
			return MOD_CHG;

// MH364300 GG119A34(S) 改善連絡表No.83対応
		case KEY_TEN_F2:	/* "全ｸﾘｱ" */
			if (mode == 0) {
				BUZPI();
				displclr(6);
				grachr(6, 1, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[1]);	// データをクリアしますか？
				Fun_Dsp(FUNMSG[19]);											/* "　　　　　　 はい いいえ　　　" */
				mode = 1;
			}
			break;
// MH364300 GG119A34(E) 改善連絡表No.83対応

		case KEY_TEN_F3:	/* "はい" */
// MH364300 GG119A34(S) 改善連絡表No.83対応
			if (mode == 1) {
// MH364300 GG119A34(E) 改善連絡表No.83対応
			BUZPI();
			NTBUF_AllClr();	/* NTNETデータ全クリア */

			// NTCOMタスクを再起動
			NTCom_ClearData(1);
			return MOD_EXT;
// MH364300 GG119A34(S) 改善連絡表No.83対応
			}
			break;
// MH364300 GG119A34(E) 改善連絡表No.83対応

		case KEY_TEN_F4:	/* "いいえ" */
// MH364300 GG119A34(S) 改善連絡表No.83対応
			if (mode == 1) {
// MH364300 GG119A34(E) 改善連絡表No.83対応
			BUZPI();
			return MOD_EXT;
// MH364300 GG119A34(S) 改善連絡表No.83対応
			}
			break;
// MH364300 GG119A34(E) 改善連絡表No.83対応

// MH364300 GG119A34(S) 改善連絡表No.83対応
		case KEY_TEN_F5:	/* "終了" */
			if (mode == 0) {
				BUZPI();
				return MOD_EXT;
			}
			break;
// MH364300 GG119A34(E) 改善連絡表No.83対応

		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ＮＴ－ＮＥＴデータ件数表示処理２									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp2( LogCnt )								|*/
/*|	PARAMETER		:	ushort	dataCnt	:	データ件数						|*/
/*|					:	ushort	type:		0:左／1:右						|*/
/*|					:	ushort	line:		表示行位置						|*/
/*|	RETURN VALUE	:	void												|*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
void	DataCntDsp( ushort dataCnt, ushort type, ushort	line )
{
	ushort	w_len, w_pos, w_pos2;

	// 実件数を最大件数(9999)以内に補正する
	if( dataCnt > 9999 ) {
		dataCnt = 9999;
	}
	
	// 件数の桁数をもとめる
	if( dataCnt >= 1000 ) {			
		w_len = 4;
	} else if (dataCnt >= 100 ) {
		w_len = 3;
	} else if (dataCnt >= 10 ) {
		w_len = 2;
	} else {
		w_len = 1;
	}

	// 表示位置を決定する
	if( type == 0 ) {
		w_pos = 8 - (w_len - 4);			
		w_pos2 = 12;			
	} else {
		w_pos = 24 - (w_len - 4);			
		w_pos2 = 28;			
	}
	
	// 件数を表示する（０サプレス）
	opedsp3( line, w_pos, dataCnt, w_len, 0, 0 ,COLOR_BLACK, LCD_BLINK_OFF );
	grachr( line, w_pos2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"件" );
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
