/*[]----------------------------------------------------------------------[]*/
/*| 検査モード処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
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
#include	"mdl_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"tbl_rkn.h"
#include	"Lkcom.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"fla_def.h"
#include	"mnt_def.h"

/* チェックメニュー番号(仮) */
enum{
	MAG_READER = 0, 		// 磁気リーダー
	NOTE_READER,		// 紙幣リーダー
	COINMEC,			// コインメック
	R_PRINTER,			// レシートプリンタ
	J_PRINTER,			// ジャーナルプリンタ
	KEY_DISP,			// スイッチキー/表示器
	LED_SHUTTER,		// LED/シャッター
	ANNOUNCE,			// 案内放送
	SWITCH,				// スイッチ
	INOUTSIG,			// 入出力信号
	
	CHK_MNU_MAX,
};

// TODO:ここで設定しているテストパターンは暫定で実際にはバーコード情報から検査実行する項目を振り分けること！
char CHK_MODE_NUM[12][CHK_MNU_MAX] = 
{	
	{1,1,1,1,1,1,1,1,1,1},		// モード1(全パターン)
	{1,1,1,1,0,1,1,1,1,1},		// モード2(ジャーナルプリンタなし)
	{1,0,0,0,0,0,0,0,0,0},		// モード3(磁気リーダーのみ)
	{0,1,0,0,0,0,0,0,0,0},		// モード4(紙幣リーダーのみ)
	{0,0,1,0,0,0,0,0,0,0},		// モード5(コインメックのみ)
	{0,0,0,1,0,0,0,0,0,0},		// モード6(レシートプリンタのみ)
	{0,0,0,0,1,0,0,0,0,0},		// モード7(ジャーナルプリンタのみ)
	{0,0,0,0,0,1,0,0,0,0},		// モード8(スイッチキー/表示器のみ)
	{0,0,0,0,0,0,1,0,0,0},		// モード9(シャッターのみ)
	{0,0,0,0,0,0,0,1,0,0},		// モード10(音声案内のみ)
	{0,0,0,0,0,0,0,0,1,0},		// モード11(スイッチのみ)
	{0,0,0,0,0,0,0,0,0,1},		// モード12(入出力信号のみ)
};

enum {
	CIK_KNO = 0,
	CIK_TYPE,
	CIK_ROLE,
	CIK_LEVEL,
	_CIK_MAX_
};

static short mag_Chk( void );
static short Note_Chk( void );
static short Coin_Chk( void );
static short RJ_Print_Chk( void );
static short Key_DispChk( void );
static short Led_Shut_Chk( void );
static short Ann_Chk( void );
static short Inout_Chk( void );
static short Sig_Chk( void );

static unsigned char Init_Magread( void );
static unsigned char Init_NoteReader( void );
static unsigned char Init_R_Printer( void );
static unsigned char Init_J_Printer( void );
static unsigned char Init_Sound( void );

static short indexToAnn( char index );

typedef short (*FUNC)(void);
typedef uchar (*INIT)(void);

typedef struct{
	unsigned char menu;
	FUNC Func;
	INIT Init;
}CHK_FUNC;

CHK_FUNC Chkfunc_tbl[] = {
	{ MAG_READER,		mag_Chk, 		Init_Magread	},
	{ NOTE_READER,		Note_Chk, 		Init_NoteReader	},
	{ COINMEC,			Coin_Chk, 		NULL	},
	{ R_PRINTER,		RJ_Print_Chk, 	Init_R_Printer	},
	{ J_PRINTER,		RJ_Print_Chk, 	Init_J_Printer	},
	{ KEY_DISP,			Key_DispChk,  	NULL	},
	{ LED_SHUTTER,		Led_Shut_Chk, 	NULL	},
	{ ANNOUNCE,			Ann_Chk,  		Init_Sound		},
	{ SWITCH,			Inout_Chk, 		NULL	},
	{ INOUTSIG,			Sig_Chk,		NULL	},
};

/* 機器情報照合のチェックテーブル */
static	struct {
	char	sec;		// 機器の設定(セクション)
	char	adr;		// 機器の設定(アドレス)
	char	pos;		// 機器の設定(桁位置)
	char	err_m;		// チェックするエラー(モジュール)
	char	err_c;		// チェックするエラー(コード)
} Chk_List[] = {
	{	S_PAY,	21,	3,	ERRMDL_READER,	01},		// MAG_READER,
	{	NULL,	 0,	0,	ERRMDL_NOTE,	01},		// NOTE_READER,
	{	NULL,	 0,	0,	ERRMDL_COIM,	01},		// COINMEC,
	{	NULL,	 0,	0,	ERRMDL_PRINTER,	01},		// R_PRINTER,
	{	S_PAY,	21,	1,	ERRMDL_PRINTER,	20},		// J_PRINTER,
	{	NULL,	 0,	0,	NULL,			00},		// KEY_DISP,
	{	NULL,	 0,	0,	NULL,			00},		// LED_SHUTTER,
	{	NULL,	 0,	0,	NULL,			00},		// ANNOUNCE,
	{	NULL,	 0,	0,	NULL,			00},		// SWITCH,	
	{	NULL,	 0,	0,	NULL,			00},		// INOUTSIG,
	{0,0,0,0,0},
};

// MH810100(S)
//const unsigned char	SYSMNT_CHKRESULT_FILENAME[] = {"FT4000_%s_%02d%02d%02d%02d%02d.csv"};		// 結果情報ファイル名のベース
const unsigned char	SYSMNT_CHKRESULT_FILENAME[] = {"GT4100_%s_%02d%02d%02d%02d%02d.csv"};		// 結果情報ファイル名のベース
// MH810100(E)
const unsigned char	SYSMNT_CHKRESULT_DIRECT[] = {"/SYS/"};			// 結果情報ファイルのディレクトリ
void ChekModeMain(void);
short Input_Moj_Rev(void);
void Test_Param_Set(ulong chk_no);
short Test_Chk_Init(short *tbl, ushort no);
unsigned char Chk_Memory(void);
void NG_key_chek( void );
void Disp_Chk_NG(const unsigned char (*dat)[31], unsigned char cnt, unsigned char *result);
void Chk_Exit(void);
unsigned char Chkmod_Sw_rd(char);
unsigned char Init_Chkmod(void);
char Chk_mod;

extern long prm_set(char kin, short ses, short adr, char len, char pos, long dat);

/*[]----------------------------------------------------------------------[]*/
/*| 検査モードメイン処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void ChekModeMain(void){
	
	/* チェックモード選択画面表示 */
	ushort msg;
	short	Chk_Tbl[CHK_MNU_MAX];
	char	faze,i,j,fuku;
	char	chk_cnt = 0;
	ushort	inp, kakari_wk, lin;
	ulong	chk_no = 0;

	OPE_SIG_OUT_DOORNOBU_LOCK(0);									// 電磁ロック解除
	key_read();														// key scan data read
	ExIOSignalwt(EXPORT_JP_RES, 0);									// ジャーナルプリンタリセット
	Ope_KeyRepeatEnable(0);
	inp = 0;
	faze = 0;
	fuku = 0;
	
	fuku = Init_Chkmod();
	if(fuku == 1){		// 検査途中の複電
		memset(&Chk_result.set, 0xff, sizeof(Chk_result.set));		/* 各モジュールの初期化を行うため機器構成情報はクリア */
		goto _CHK_START;
	}
	else if(fuku == 2){
		goto _CHK_END;
	}
	
	for( ; ; )
	{
		if(faze == 0){						// 画面表示
			dispclr();
			chk_cnt = 0;
			bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[1]);	// "
			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
			grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[6]);				// "
			grawaku2( 4 );
			teninb(inp, 4, 5, 10, COLOR_FIREBRICK);	// 駐車位置番号表示
			faze = 1;
		}
		msg = GetMessage();
		switch (KEY_TEN0to9(msg)) {
		/* mode change */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)

			case KEY_MODECHG:			// メンテナンスキーは無視する
				break;
		/* number key */
			case KEY_TEN:
				BUZPI();
				if(faze == 1){					// 社員番号の入力
					lin = 5;
					inp = ((inp*10 + (msg - KEY_TEN))%10000);
					teninb(inp, 4, lin, 10, COLOR_FIREBRICK);	// 番号表示
				}
				else if(faze == 2){				// 検査パターンの入力
					lin = 3;
					chk_no = ((chk_no*10 + (msg - KEY_TEN))%1000000L);
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);	// 番号表示
				}
				else{
					break;
				}
				break;
			case KEY_TEN_CL:				// クリアキー
				BUZPI();
				if(faze == 1){
					lin = 5;
					inp = 0;
					teninb(inp, 4, lin, 10, COLOR_FIREBRICK);	// 番号表示
				}
				else if(faze == 2){
					lin = 3;
					chk_no = 0;
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);	// 番号表示
				}
				break;
			case KEY_TEN_F1:			// 戻る
				if(faze == 2){
					faze = 0;
					chk_no = 0;
				}
				break;
			case KEY_TEN_F2:
			case KEY_TEN_F4:
				break;
			case KEY_TEN_F3:			// FTP
				BUZPI();
				SysMnt_FTPServer();
				faze = 0;
				break;
			case KEY_TEN_F5:			// 次へ
				if(faze == 1){			// 社員番号
					if(inp == 0){
						BUZPIPI();
						break;
					}
					kakari_wk = inp;
					faze = 2;			// 検査パターン入力へ
					inp = 0;
					dispclr();
					grachr(0, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[7]);
					grachr(6, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[2]);
					/* 横棒表示はいまのところできないみたいなので０表示 */
					teninb2(chk_no, 6, 3, 6, COLOR_FIREBRICK);
					Fun_Dsp( FUNMSG[114] );
					break;
				}
				if(chk_no == 0 || chk_no >= 12){				// TODO:暫定処理
					if( faze == 2 ){
						lin = 3;
					}
					BUZPIPI();
					chk_no = 0;
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);
					break;
				}
				BUZPI();
				/* チェックモード決定 */
				/* チェックする項目と精算機の構成を比較する */
				/* 機器構成チェック＆初期化 */
				Chk_info.pt_no = chk_no;
				memset(&Chk_result, 0xff, sizeof(Chk_result));		// 検査結果格納バッファ初期化
				Chk_result.Kakari_no = kakari_wk;
				Chk_result.Chk_no = chk_no;
				Chk_result.Card_no = 0;
				memcpy( &Chk_result.Chk_date, &CLK_REC, sizeof(date_time_rec));
				memset(&Chk_res_ftp, 0, sizeof(t_Chk_res_ftp));

				Test_Param_Set( chk_no );							// 共通パラメータセット
				return;												// 共通パラメータセット後、再起動するのでここに来ることはない。
_CHK_START:
				strcpy((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1);			// 固定文字列①セット
				chk_cnt = Test_Chk_Init(Chk_Tbl, Chk_info.pt_no-1);			// メモリチェック＆モジュール初期化処理
_CHK_MODULE:
				/* モジュール基板 */
				if(Input_Moj_Rev() == -1){			// 戻る
					inp = 0;
					chk_no = 0;
					dispclr();
					grachr(0, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[7]);
					grachr(6, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[2]);
					/* 横棒表示はいまのところできないみたいなので０表示 */
					teninb2(chk_no, 6, 3, 6, COLOR_FIREBRICK);
					Fun_Dsp( FUNMSG[111] );
					break;
				}
				for(i = 0; i < chk_cnt; i++){
_CHK_REVERSE:
					for(j=0;i<CHK_MNU_MAX;j++){
						if(Chkfunc_tbl[j].menu == Chk_Tbl[i]){
							if(fuku){				// 停複電後の検査再開
								if(Chk_info.Chk_mod != 0){
									if(Chk_info.Chk_mod != Chk_Tbl[i]){		// 停電時に実施していた検査までスキップ
										break;
									}
								}
								fuku = 0;
							}
							Chk_info.Chk_mod = Chkfunc_tbl[j].menu;
							if(Chkfunc_tbl[j].Func() == -1){			// 各検査実施
								// 戻る
								if(i == 0){
									// モジュール基板
									goto _CHK_MODULE;
								}
								else{
									i--;
									goto _CHK_REVERSE;
								}
								break;
							}
							else{
								// 次へ
								break;
							}
						}
					}
				}
				strcpy((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2);			// 固定文字列②セット
_CHK_END:
				if(fuku == 2){
					// 復電からの検査終了画面時は印字できるようにモジュール初期化を行う
					Test_Chk_Init(Chk_Tbl, Chk_info.pt_no-1);					// メモリチェック＆モジュール初期化処理
				}
				Chk_Exit();						// 検査終了画面
				faze = 0;						// 検査終了
				inp = 0;
				chk_no = 0;
				fuku = 0;
				break;
			
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| パラメータセット処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Test_Param_Set( ulong chk_no )                          |*/
/*| PARAMETER    : chk_no	:検査パターン番号                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Test_Param_Set( ulong chk_no )
{
	ushort	msg;
	uchar	i;
	long	val;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[9]);
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[10]);
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[11]);
	Fun_Dsp( FUNMSG[0] );

	Lagtim( OPETCBNO, 8, (50*10));			// 自動再起動までの10秒タイマセット

	strcpy((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1);			// 固定文字列①セット
	Chk_info.Chk_mod = 0;

	prm_clr( COM_PRM, 1, 0 );				// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ

	// ドアノブ戻し忘れアラーム無効
	CPrmSS[S_PAY][39] = 0;


	// TODO:検査パターンが確定していないので暫定
	for( i=0; i<CHK_MNU_MAX; i++){
		switch(i){
		case 0:		// 磁気リーダー
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 1:		// 紙幣リーダー
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 2:		// コインメック
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 3:		// レシートプリンタ
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 4:		// ジャーナルプリンタ
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
				val = prm_set(COM_PRM, S_PAY, 21, 1, 1, 1);
				CPrmSS[S_PAY][21] = val;
			}
			else{
				// 検査しない
				val = prm_set(COM_PRM, S_PAY, 21, 1, 1, 0);
				CPrmSS[S_PAY][21] = val;
			}
			break;
		case 5:		// スイッチキー/表示器
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 6:		// LED/シャッター
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 7:		// 案内放送
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		case 8:		// スイッチ
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
		case 9:		// 入出力信号
			if(CHK_MODE_NUM[chk_no-1][i]){
				// 検査する
			}
			else{
				// 検査しない
			}
			break;
		default:
			break;
		}
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);

	while( 1 ){
		msg = GetMessage();					// ﾒｯｾｰｼﾞ受信
		switch(msg){						// 受信ﾒｯｾｰｼﾞ？
			case TIMEOUT8:					// 再起動ﾀｲﾏｰﾀｲﾑｱｳﾄ
				Lagcan( OPETCBNO, 6 );
				// システム再起動
				System_reset();				// Main CPU reset (It will not retrun from this function)
				break;
			default:
				break;
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 各モジュール初期化処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Test_Chk_Init(short *tbl, ushort no)                    |*/
/*| PARAMETER    : *tbl	:チェックメニューテーブルへのポインタ              |*/
/*|                no	:チェックパターンナンバー                          |*/
/*| RETURN VALUE : unsigned short: チェックする項目数                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Test_Chk_Init(short *tbl, ushort no)
{
	short result,cnt,i;
	cnt = 0;
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[3]);
	bigcr(3, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[2]);
	Fun_Dsp( FUNMSG[0] );
	/* メモリチェック */
	Chk_Memory();

	/* 各モジュール初期化 */
	for(i = 0; i < CHK_MNU_MAX; i++){
		if( CHK_MODE_NUM[no][i] == 1){	// 検査実施？
			if( Chk_List[i].sec != NULL ){
				if( 0 == prm_get(COM_PRM, Chk_List[i].sec, Chk_List[i].adr, 1, Chk_List[i].pos) ){
					/* 設定なし */
					continue;
				}
			}
			
			if(Chkfunc_tbl[i].Init != NULL){
				Chk_mod = i;
				result = Chkfunc_tbl[i].Init();		// 初期化処理実施
			}
			else{
				result = 0;
			}
			if(result == 0){				// 初期化完了
				tbl[cnt] = i;				// チェック項目番号格納
				cnt++;						// 件数+1
			}
		}
	}
	return cnt;
}
/*[]----------------------------------------------------------------------[]*/
/*| モジュールバーコード読み取り処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : short 0:次へ -1:戻る                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	BLUETOOTH_EVENT	100			// バーコードスキャン通知のイベント(仮)
#define	MOJU_ANN_ST	200				// モジュールのアナウンス番号開始
#define	MOJU_MAX	16				// モジュールの最大個数
short Input_Moj_Rev(void)
{
	ushort msg;
	char	i, lin, index;
	
	dispclr();
	
	memset(&Chk_result.moj, 0, 16);
	for(i=0;i<7;i++){
		grachr( i,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[i] );
	}
	Fun_Dsp( FUNMSG[111] );
	avm_test_cnt = 2;
	lin = 1;					// 反転表示する行数
	index = 1;					// 反転表示する文字列番号
	avm_test_no[0] = 200;		// これから指示するブロックのバーコードをスキャンしてください
	avm_test_no[1] = indexToAnn( index );		// RXM基板
	
	ope_anm( AVM_AN_TEST );		// ありがとうございました。
	grachr( 1,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[1] );		// 反転表示
	for(;;){
			
		msg = GetMessage();
		switch(msg){
			case BLUETOOTH_EVENT:
				/* 受信データを結果エリアへ */
			case KEY_TEN_F1:		// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F2:		// ▲
				if(index == 1){
					break;
				}
				grachr( lin,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// 通常表示
				index--;
				lin--;
				if(lin == 0){
					dispmlclr(1,6);
					for(i=0;i<6;i++){
						grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[(index-5)+i] );
					}
					lin = 6;
				}
				grachr( lin,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// 反転表示
				avm_test_no[0] = indexToAnn( index );		// 前のアナウンス番号
				avm_test_cnt = 1;
				ope_anm( AVM_AN_TEST );		// ありがとうございました。
				break;
			case KEY_TEN_F3:		// ▼
				if(index >= MOJU_MAX){
					return 0;
				}
				grachr( lin,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// 通常表示
				index++;
				lin++;
				if(lin == 7){
					dispmlclr(1,6);
					for(i=0;i<6;i++){
						grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index+i] );
						if((index+i)>=MOJU_MAX){
							break;
						}
					}
					lin = 1;
				}
				grachr( lin,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// 反転表示
				avm_test_no[0] = indexToAnn( index );		// 次のアナウンス番号
				avm_test_cnt = 1;
				ope_anm( AVM_AN_TEST );		// ありがとうございました。
				break;
			case KEY_TEN_F5:		// 次へ
				BUZPI();
				return 0;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| アナウンス番号取得                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : indexToAnn( char index )                                |*/
/*| PARAMETER    : char index : モジュール機番                             |*/
/*| RETURN VALUE : short      : アナウンス番号                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short indexToAnn( char index )
{
	short ann = 0;

	switch(index){
	case 1:			// RXM基板
		ann = 201;
		break;
	case 2:			// RXF基板
		ann = 202;
		break;
	case 3:			// RXI基板
		ann = 203;
		break;
	case 4:			// コインメック
		ann = 205;
		break;
	case 5:			// 紙幣リーダー
		ann = 206;
		break;
	case 6:			// 電子決済リーダー
		ann = 207;
		break;
	case 7:			// ジャーナルプリンター
		ann = 208;
		break;
	case 8:			// レシートプリンター
		ann = 209;
		break;
	case 9:			// 左側のFOMAモジュール
		ann = 210;
		break;
	case 10:		// 右側のFOMAモジュール
		ann = 211;
		break;
	case 11:		// 磁気リーダー
		ann = 213;
		break;
	case 12:		// Vプリンター
		ann = 214;
		break;
	case 13:		// カラーLCDモジュール
		ann = 215;
		break;
	case 14:		// 電源ユニット
		ann = 218;
		break;
	case 15:		// 拡張電源ユニット
		ann = 219;
		break;
	case 16:		// テンキーモジュール
		ann = 220;
		break;
	default:
		break;
	}
	return ann;
}

/*[]----------------------------------------------------------------------[]*/
/*| キー入力/表示器確認                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Key_DispChk( void )
{
	struct {
		unsigned short str;		// 文字列の配列番号
		unsigned short lin;		// 表示行
		unsigned short col;		// 表示位置
		unsigned short back;	// 変化させる背景色、輝度
	} KEY_pos[16] = {
	//  {表示行, 表示位置, 背景}
		{6,	5,	11,	COLOR_BLACK},		// ０
		{3,	2,	 5,	COLOR_WHITE},		// １
		{3,	2,	11,	COLOR_HOTPINK},		// ２
		{3,	2,	17,	COLOR_BLUE},		// ３
		{4,	3,	 5,	COLOR_GREEN},		// ４
		{4,	3,	11,	COLOR_GOLD},		// ５
		{4,	3,	17,	COLOR_SIENNA},		// ６
		{5,	4,	 5,	COLOR_RED},			// ７
		{5,	4,	11,	COLOR_PALEVIOLETRED},// ８
		{5,	4,	17,	0},					// ９
		{3,	2,	23,	0},					// F1
		{4,	3,	23,	1},					// F2
		{5,	4,	23,	6},					// F3
		{6,	5,	23,	11},				// F4
		{6,	5,	17,	LCD_LUMINE_MAX},	// F5
		{6,	5,	 5,	COLOR_PALETURQUOISE},// Ｃ
	};
	
	char	KEY_flg[16] = {0};
	unsigned short	color;
	ushort msg;
	char	i,Chk_end,key_comp,disp_reset;
	
	Chk_end = 0;
	key_comp = 0;
	disp_reset = 1;
	memset(&Chk_result.key_disp, 0, 2);
	for( ;Chk_end == 0; )
	{
		if(disp_reset){
			/* 画面描画しなおす */
			dispclr();													// Display All Clear
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, KEY_DISP_STR[0]);
			for(i = 0; i < 16; i++){
				if(KEY_flg[i]){
					color = COLOR_GOLD;					// 押し済みのキーの色
				}
				else{
					color = COLOR_SIENNA;				// 未押下のキーの色
				}
				grachr( KEY_pos[i].lin,  KEY_pos[i].col, 2, 0, 
						color, LCD_BLINK_OFF, &KEY_DISP_STR[KEY_pos[i].str][KEY_pos[i].col] );	// 押されたキーを色を切り替えて表示
			}
			if((key_comp) || (Chk_result.key_disp[0] == 1)){
				Fun_Dsp( FUNMSG[109] );				
			}else{
				Funckey_one_Dsp(&FUNMSG[109][18], 4);
			}
			disp_reset = 0;
		}
		msg = GetMessage();
		if(msg >= KEY_TEN0 && msg <= KEY_TEN_CL){
			if(msg == KEY_TEN_F1 && ((key_comp) || (Chk_result.key_disp[0] == 1))){
				dsp_background_color(COLOR_WHITE);
				dispclr();													// Display All Clear
				lcd_backlight(1);
				return -1;
			}
			/* 全て押し済みの状態でF5キー押したら終了 */
			if(msg == KEY_TEN_F5 && ((key_comp) || (Chk_result.key_disp[0] == 1))){
				dsp_background_color(COLOR_WHITE);
				dispclr();													// Display All Clear
				lcd_backlight(1);
				return 0;
			}
			/* 押されたキーの反転表示 */
			KEY_flg[msg-KEY_TEN] = 1;
			color = COLOR_GOLD;
			grachr( KEY_pos[msg-KEY_TEN].lin,  KEY_pos[msg-KEY_TEN].col, 2, 0, 
					color, LCD_BLINK_OFF, &KEY_DISP_STR[KEY_pos[msg-KEY_TEN].str][KEY_pos[msg-KEY_TEN].col] );	// 押されたキーを色を切り替えて表示
			for(i = 0; i < 16; i++){
				if(KEY_flg[i] == 0){
					break;
				}
			}
			if(i >= 15){
				key_comp = 1;
				Fun_Dsp( FUNMSG[109] );
			}
		}
			
		switch (msg) {
		/* mode change */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				break;
		/* number key */
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN_CL:
				BUZPI();
				dsp_background_color(KEY_pos[msg-KEY_TEN].back);			// 背景色変更
				disp_reset = 1;												// 画面再描画
				break;
			case KEY_TEN9:
				BUZPI();
				break;
			case KEY_TEN_F1:		// 
			case KEY_TEN_F2:		// 
			case KEY_TEN_F3:		// 
			case KEY_TEN_F4:		// 
			case KEY_TEN_F5:		// 
				BUZPI();
				lcd_contrast((uchar)KEY_pos[msg-KEY_TEN].back);				// バックライト（コントラスト）
				break;
			case TIMEOUT8:
				dsp_background_color(COLOR_WHITE);
				lcd_backlight(1);
				Disp_Chk_NG(KEY_DISP_NGSTR, 2, &Chk_result.key_disp[0]);
				disp_reset = 1;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| 磁気リーダーチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short mag_Chk( void )
{
	short		msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	Chk_end = 0;	// チェック終了フラグ
	char		faze,disp;
	uchar		prn;
	m_kakari	*crd_data;			// 磁気ｶｰﾄﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
	ulong		atend_no = 0;
	char		write_mode;			// ライトモード状態 0:書込以外 1:書込中
	uchar		IdSyuPara;			// 使用ID種設定ﾊﾟﾗﾒｰﾀ
	char		prm[_CIK_MAX_];		// 設定パラメータ
	uchar		type_tbl[] = {0x20, 0x49, 0x4b, 0x53, 0x4a};	/* ' ', 'I', 'K', 'S', 'J' */

	dispclr();

	prn = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// 磁気ﾘｰﾀﾞｰ種別取得
	if( prn == 1 || prn == 2 ){						// プリンタあり？
		prn = 1;
	}
	else{
		prn = 0;
	}

	prm[CIK_KNO] = 1;
	prm[CIK_TYPE] = 2;
	prm[CIK_ROLE] = 1;
	prm[CIK_LEVEL] = 0;

	faze = 0;
	disp = 1;
	OPE_red = 0;
	for( ; Chk_end == 0 ; ){
		if(disp != 0){
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[disp] );	//
			disp = 0;
		}

		if( faze == 0 ){
			read_sht_opn();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「開」
			opr_snd(200);						// カード取り込み要求
			write_mode = 1;						// 書込み中
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[0] );	//
			displclr(2);
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[5] );	// " 検査員カードを挿入して下さい "
			bigcr(4, 10, 10, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[3]);
			faze = 1;
			Fun_Dsp( FUNMSG[109] );
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case TIMEOUT13:
				displclr(6);
				Lagcan( OPETCBNO, 13 );
				if(faze == 3){
					if( opr_snd( 93 ) != 0 ){		// 印字ﾃｽﾄ要求
						Chk_end = 1;
					}
				}
				break;
			case ARC_CR_R_EVT:
				if(MAGred[MAG_ID_CODE] != 0x41){		// 係員カード以外
					grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "このカードは使えません"
					Lagtim( OPETCBNO, 13, (50*6));
					ope_anm( AVM_CARD_ERR1 );
					opr_snd(13);
					break;
				}
				if(faze == 2){				// リードテスト
					crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];
					atend_no = (( (crd_data->kkr_kno[0] & 0x0f) * 1000)  +						//1000のを取得
								( (crd_data->kkr_kno[1] & 0x0f) * 100 )	 +						//100の桁を取得
								( (crd_data->kkr_kno[2] & 0x0f) * 10 )	 +						//10の桁を取得
								( (crd_data->kkr_kno[3] & 0x0f) * 1 ));							//1の桁を取得
					teninb((ushort)atend_no, 2, 4, 13, COLOR_FIREBRICK);	// 検査員番号表示
					Chk_result.Card_no = atend_no;
					displclr(6);
					if(prn){						// プリンタあり？
						opr_snd(2);					// 戻し
					}
					else{							// プリンタなしの場合は廃券して戻す
						if( opr_snd( 102 ) == 0 ){				// 廃券
							faze = 4;							// 消磁気テスト
							disp = 4;
						}
					}
				}
				else if(faze == 4){					// 印字テスト後の券挿入→消磁気テスト
					if(opr_snd(102) != 0){
						Chk_end = 1;
					}
				}
				break;
			case ARC_CR_E_EVT:
				switch(faze){
				case 1:
					if (write_mode == 1) {
						/* 係員カードデータ作成 */
						crd_data = (m_kakari *)MDP_buf;
						if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// 新カードＩＤ使用する？
							(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APSフォーマットで使用する？
							// 新カードＩＤを使用する場合
							IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,10,1,1 );				// 基本駐車場No.券ID（係員ｶｰﾄﾞ）設定ﾊﾟﾗﾒｰﾀ取得
							switch( IdSyuPara ){
								case	1:													// ID1
									crd_data->kkr_idc = 0x41;
									break;
								case	2:													// ID2
									crd_data->kkr_idc = 0x69;
									break;
								case	3:													// ID3
									crd_data->kkr_idc = 0x6F;
									break;
								case	4:													// ID4
									crd_data->kkr_idc = 0x75;
									break;
								case	5:													// ID5
									crd_data->kkr_idc = 0x7B;
									break;
								case	6:													// ID1とID2
									crd_data->kkr_idc = 0x41;
									break;
								case	7:													// ID1とID3
									crd_data->kkr_idc = 0x41;
									break;
								case	8:													// ID6
									crd_data->kkr_idc = 0x7E;
									break;
								case	0:													// 読まない
								default:													// その他（設定値ｴﾗｰ）
									crd_data->kkr_idc = 0x41;
									break;
							}
						}
						else{
							// 新カードＩＤを使用しない場合
							crd_data->kkr_idc = 0x41;
						}
						crd_data->kkr_year[0] = (uchar)(CLK_REC.year%100/10 + 0x30);
						crd_data->kkr_year[1] = (uchar)(CLK_REC.year%10 + 0x30);
						crd_data->kkr_mon[0] = (uchar)(CLK_REC.mont/10 + 0x30);
						crd_data->kkr_mon[1] = (uchar)(CLK_REC.mont%10 + 0x30);
						crd_data->kkr_day[0] = (uchar)(CLK_REC.date/10 + 0x30);
						crd_data->kkr_day[1] = (uchar)(CLK_REC.date%10 + 0x30);
						crd_data->kkr_did = 0x53;
						if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット
							crd_data->kkr_park[0] = 0x50;	/* 'P' */
							crd_data->kkr_park[1] = 0x47;	/* 'G' */
							crd_data->kkr_park[2] = 0x54;	/* 'T' */
						}else{//APSフォーマット
							crd_data->kkr_park[0] = 0x50;	/* 'P' */
							crd_data->kkr_park[1] = 0x41;	/* 'A' */
							crd_data->kkr_park[2] = 0x4B;	/* 'K' */
						}
						crd_data->kkr_role = (uchar)(prm[CIK_ROLE] + 0x30);
						crd_data->kkr_lev = (uchar)(prm[CIK_LEVEL] + 0x30);
						crd_data->kkr_type = type_tbl[prm[CIK_TYPE]];
						crd_data->kkr_kno[0] = 0x30;
						crd_data->kkr_kno[1] = 0x30;
						crd_data->kkr_kno[2] = (uchar)(prm[CIK_KNO]/10 + 0x30);
						crd_data->kkr_kno[3] = (uchar)(prm[CIK_KNO]%10 + 0x30);
						memset(&crd_data->kkr_jdg, 0x30, sizeof(crd_data->kkr_jdg));
						memset(crd_data->kkr_rsv1, 0x20, sizeof(crd_data->kkr_rsv1));
						memset(crd_data->kkr_rsv2, 0x20, sizeof(crd_data->kkr_rsv2));
						memset(crd_data->kkr_rsv3, 0x20, sizeof(crd_data->kkr_rsv3));
						if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GTフォーマット
							crd_data->kkr_rsv2[4] = (uchar)(CPrmSS[S_SYS][1]/100000 + 0x30);
							crd_data->kkr_rsv2[5] = (uchar)(CPrmSS[S_SYS][1]%100000/10000 + 0x30);
							crd_data->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]%10000/1000 + 0x30);
							crd_data->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
							crd_data->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
							crd_data->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
							MDP_buf[127] = 1;
						}else{//APSフォーマット
							crd_data->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]/1000 + 0x30);
							crd_data->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
							crd_data->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
							crd_data->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
							MDP_buf[127] = 0;
						}
						MDP_mag = sizeof( m_kakari );
						MDP_siz = sizeof(m_kakari);
						md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 1);								/* パリティ作成(偶数ﾊﾟﾘﾃｨ) */
						/* 係員カード書き込み */
						opr_snd(202);
						write_mode = 2;
					}
					/* カード書き込み終了 */
					else {
						/* 書込ＮＧ */
						if (RED_REC.ercd) {
							// TODO:ここに書込みＮＧのエラー表示処理を入れる
							grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[9] );	// "このカードは書けません"
							Lagtim( OPETCBNO, 13, (50*6));
							ope_anm( AVM_CARD_ERR1 );
							
							faze = 0;
							disp = 1;
							OPE_red = 0;
						}
						/* 書込ＯＫ */
						else {
							write_mode = 0;
							faze = 2;
							disp = 2;
							opr_snd( 3 );
							Chk_result.mag[0] = 0;		// 書込みテストOK
						}
					}
					break;
				case 2:									// リードテスト
					if( RED_REC.ercd == 0x21){			// 読取不可で消磁テスト正常終了
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[8] );	// "このカードは読めません"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR2 );
						opr_snd( 3 );
					}
					break;
				case 3:									// 印字テスト終了
					if( RED_REC.ercd == 0){
						faze = 4;
						disp = 4;
						opr_snd( 3 );
						Chk_result.mag[2] = 0;			// 印字テストOK
					}
					else{
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "このカードは使えません"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR1 );
					}
					break;
				case 4:									// 消磁テスト終了
					if( RED_REC.ercd == 0x21){
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "このカードは使えません"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR1 );
						opr_snd( 3 );
					}
					break;
				case 5:
					if( RED_REC.ercd == 0x21){			// 読取不可で消磁テスト正常終了
						read_sht_cls();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
						OPE_red = 2;
						Chk_end = 1;
						Chk_result.mag[3] = 0;			// 消磁テストOK
						break;
					}
					break;
				default:
					opr_snd( 3 );
					break;
				}
				break;
			
			case ARC_CR_EOT_EVT:									// 券抜き取り
				switch(faze){
				case 1:
					// ライトテスト
					break;
				case 2:
					if(atend_no != 0){								// 係員カード抜き取り
						if(prn){
							if( opr_snd( 93 ) == 0 ){				// 印字ﾃｽﾄ要求
								faze = 3;
								disp = 3;
								Chk_result.mag[1] = 0;				// リードテストOK
							}
						}
						else{
							displclr(6);
							/* 係員カード以外の券抜き取り */
						}
					}
					else{					// 係員カード以外の券
						opr_snd( 3 );		// リード要求
					}
					break;
				case 4:				// 廃券書き込み
					faze = 5;		// 廃券挿入待ち
					opr_snd( 3 );
					break;
				default:
					faze = 0;
					break;
				}
				break;
				
			case KEY_TEN_F1:						// 戻る
				BUZPI();
				read_sht_cls();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
				OPE_red = 6;
				opr_snd( 90 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞを無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				Chk_end = 1;
				return -1;
			case KEY_TEN_F5:						// 次へ
				BUZPI();
				read_sht_cls();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
				OPE_red = 6;
				opr_snd( 90 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞを無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				Chk_end = 1;
				break;
			case TIMEOUT8:
				read_sht_cls();						// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ「閉」
				OPE_red = 6;
				opr_snd( 90 );						// ﾘｰﾄﾞｺﾏﾝﾄﾞを無効とする為、ﾃｽﾄｺﾏﾝﾄﾞを送信
				Disp_Chk_NG(MAG_CHK_NGSTR, 4, &Chk_result.mag[0]);
				OPE_red = 0;
				faze = 0;
				disp = 1;
				atend_no = 0;
				displclr(4);
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 紙幣リーダーチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Note_Chk( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	unsigned short	rd_cnt	= 0;		// 紙幣読込枚数
	unsigned char	disp_reset = 1;
	unsigned char	rd_sts	= 0;		// 紙幣ﾘｰﾀﾞｰ状態
										//  0:空き
										//  1:紙幣保留中
										//  2:紙幣戻し完了待ち
										//  3:紙幣取込完了待ち
	unsigned short	First_flag = 0;		// ４枚目の入金がされたかどうか
	
	cn_stat( 1, 1 );						// 入金許可
	memset( Chk_result.note, 0, 3);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[0] );	//
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[1] );		// "１０００円札"
			opedsp( 3, 14,  rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// "０"
			opedsp( 3, 22,  4, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// "４"
			Fun_Dsp( FUNMSG[109] );					// "　　　　　　 戻し  取込  終了 "
			disp_reset = 0;
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:			// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// 次へ
				BUZPI();
				cn_stat( 2, 1 );	// 入金不可
				Chk_end = 1;
				break;
			case TIMEOUT8:
				if(rd_sts < 2){
					cn_stat( 2, 1 );	// 入金不可
					Disp_Chk_NG(NOTE_CHK_NGSTR, 3, &Chk_result.note[0]);
					cn_stat( 1, 1 );						// 入金許可
					disp_reset = 1;
				}
				break;
			case NOTE_EVT:					// 紙幣リーダーイベント
				switch( OPECTL.NT_QSIG ){
					case	1:				// 紙幣ﾘｰﾀﾞｰから「入金あり」受信
						rd_cnt++;																// 読込枚数更新（＋１）
						opedsp( 3, 14, rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// 読込枚数表示
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[2] );		// "　紙幣を保留中です　　　　　　"
						rd_sts = 1;								// 状態：紙幣保留中
						if(rd_cnt >= 4 ){				// ４枚目以降?
							if( !First_flag ){			// 初回なら戻す
								cn_stat( 2, 1 ); 		// 入金不可
								rd_sts = 2;				// 状態：紙幣戻し完了待ち
								First_flag++;
							}else{						// 再挿入なら取り込む
								First_flag++;
								nt_com = 0x80 + 4;		/* ｴｽｸﾛ紙幣取り込み */
							}
						}
						break;
					case	2:				// 紙幣ﾘｰﾀﾞｰから「払出し完了」受信
						if( rd_sts == 2 ){		// 紙幣戻し完了待ち？
							rd_cnt--;															// 読込枚数更新（－１）
							opedsp( 3, 14, rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// 読込枚数表示
							displclr( 6 );														// "　紙幣を保留中です　　　　　　"表示オフ
							rd_sts = 0;															// 状態：空き
							cn_stat( 1, 1);														// 入金許可
						}
						break;
					case	3:				// 紙幣ﾘｰﾀﾞｰから「収金完了」受信
						if( rd_sts == 3 ){		// 紙幣取込完了待ち？
							displclr( 6 );						// "　紙幣を保留中です　　　　　　"表示オフ
							rd_sts = 0;							// 状態：空き
						}else if( First_flag == 2 ){
							cn_stat( 2, 1 );	// 入金不可
							Chk_end = 1;
						}
						break;
					case 	5:									// 後続入金あり
						cn_stat( 1, 1 );						// 紙幣収金
						rd_sts = 3;								// 状態：紙幣取込完了待ち
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
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| コインメックチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define COINSYU_CNT 4
short Coin_Chk( void )
{
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	short	msg = -1;
	int		i;
	int		subtube = 0;
	ushort	current_cnt[10];		// 枚数データ(BCD)
	uchar	wk1,wk2,cnt_total = 0;
	unsigned char	disp_reset = 1;
	long ref_val = 0;
	long total_val = 0;
	
	memset(&ref_coinsyu, 0, sizeof(ref_coinsyu));
	if( CN_SUB_SET[1] != 0 ){	// 100円SUBあり
		subtube = 0x02;
	} else {					// 10円SUBあり
		subtube = 0x01;
	}
	memset(current_cnt, 0, sizeof(current_cnt));
	current_cnt[4] = 0x05;			// 予備部には予め5枚の硬貨が補充されているとみなす
	if( subtube & 0x01 ){
		current_cnt[5] = 0x05;		// 10円規定値
	}
	else{
		current_cnt[5] = 0x05;
	}

	current_cnt[6] = 0x05;			// 50円規定値

	if( subtube & 0x02 ){
		current_cnt[7] = 0x05;		// 10円規定値
	}
	else{
		current_cnt[7] = 0x05;		// 100円規定値
	}
	current_cnt[8] = 0x05;			// 500円規定値
	current_cnt[9] = 0x05;			// 予備
	cn_stat( 33, 0 );				// 入金許可(CREM:ON)

	memset( Chk_result.coin, 0, 3);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[0] );	// "＜コインメック確認＞"
			for( i = 0; i < 4 ; i++ ){
				grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[i+1] );	// 
				opedsp( (unsigned short)(i+1), 14, (unsigned short)bcdbin(current_cnt[i]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
				opedsp( (unsigned short)(i+1), 22, (unsigned short)bcdbin(current_cnt[5+i]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			}

			// 予蓄部を表示
			if( subtube ){
				if( subtube & 0x01 ){
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[5] );	//
				}else{
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[6] );	//
				}
				opedsp( 5, 14, (unsigned short)bcdbin(current_cnt[4]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
				opedsp( 5, 22, (unsigned short)bcdbin(current_cnt[9]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );

			}
			Fun_Dsp( FUNMSG[109] );
			disp_reset = 0;
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:			// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// 次へ
				BUZPI();
				cn_stat( 2, 0 );
				Chk_end = 1;
				break;
			case TIMEOUT8:				// F4キー長押し
					cn_stat( 2, 0 );
					Disp_Chk_NG(COIN_CHK_NGSTR, 3, &Chk_result.coin[0]);
					cn_stat( 33, 0 );
					disp_reset = 1;
				break;
			case COIN_EVT:					// コインメックイベント
				switch( OPECTL.CN_QSIG ){
					case	1:				// ｺｲﾝﾒｯｸから「入金あり」受信
					case	5:				// ｺｲﾝﾒｯｸから「入金あり（max達成）」受信
						for( i = 0; i < COINSYU_CNT ; i++ ){		// ｺｲﾝ入金枚数表示
							cnt_total = bcdadd( current_cnt[i], CN_RDAT.r_dat08[i] );
							opedsp( (unsigned short)(i + 1), 14, (unsigned short)bcdbin(cnt_total), 2, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
							if(cnt_total >= current_cnt[5+i]){
								ref_coinsyu[i] = bcdbin(current_cnt[5+i]);					// 金種毎の払い出し枚数にセット
								if( (i==0 && (subtube & 0x01)) || (i==2 && (subtube & 0x02)) ){
									ref_coinsyu[i] += 5;			/* サブ＆予備分を加算 */
								}
							}
						}
							for( i = 0; i < COINSYU_CNT ; i++ ){	// ｺｲﾝ入金枚数保存ﾜｰｸに受信ﾃﾞｰﾀ（枚数）を加算
								current_cnt[i] = bcdadd( current_cnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// 入金許可(CREM:ON,投入金ｸﾘｱ)
						ref_val = 0;
						for(i=0;i<COINSYU_CNT;i++){
							if(ref_coinsyu[i] == 0){
								break;
							}
							ref_val += (long)(ref_coinsyu[i]*coin_vl[i]);
						}
						if(i>=COINSYU_CNT){
							cn_stat( 2, 0 );													// 入金不可
						}
						break;
					case	2:				// ｺｲﾝﾒｯｸから「払出し完了」受信
						ref_val = 0;															// 払出金額ｸﾘｱ
						for( i = 0; i < COINSYU_CNT ; i++ ){		// ｺｲﾝ入金枚数表示
							cnt_total = bcdbin(CN_RDAT.r_dat09[i]) * coin_vl[i];
							wk1 = bcdbin( CN_RDAT.r_dat0a[i]);
							wk2 = bcdbin( cnt_total );
							if(wk1>wk2){
								wk2 = 0;
							}
							else{
								wk2 -= wk1;
							}
							cnt_total = binbcd(wk2);
							opedsp( (unsigned short)(i + 1), 14, (unsigned short)bcdbin(cnt_total), 2, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
							current_cnt[i] = cnt_total;
						}
						opedsp( 5, 14, 0, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
						cn_stat( 33, 0 );					// 入金許可(CREM:ON)
						break;

					case	7:				// ｺｲﾝﾒｯｸから「払出し可状態」受信
						if(ref_val){
							total_val = 0;
							for(i=0;i<COINSYU_CNT;i++){
								total_val += bcdbin(CN_RDAT.r_dat09[i]) * coin_vl[i];
							}
							if( subtube & 0x01 )
							{	// 10円SUBあり
								total_val += current_cnt[4] * coin_vl[0];
							}
							if( subtube & 0x02 )
							{	// 100円SUBあり
								total_val += current_cnt[4] * coin_vl[2];
							}
							refund(total_val);				// 払出開始(全ての硬貨を払い出す)
						}
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| プリンタチェック(レシート、ジャーナル共通)                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		TSTPRI_END	(PREQ_PRINT_TEST | INNJI_ENDMASK)
const	unsigned char	PriSts[4] =	// ﾌﾟﾘﾝﾀ状態ﾁｪｯｸﾌﾗｸﾞ
{
	0x01,	// b0:ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ
	0x02,	// b1:ﾌﾟﾗﾃﾝｵｰﾌﾟﾝ
	0x04,	// b2:ﾍﾟｰﾊﾟｰｴﾝﾄﾞ
	0x08	// b3:ﾍｯﾄﾞ温度異常
};
short RJ_Print_Chk( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	T_FrmPrintTest	FrmPrintTest;		// 印字要求ﾒｯｾｰｼﾞ作成ｴﾘｱ
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	unsigned char	priend, sts, pState, sts_bk, com, com_bk;
	unsigned char	line, i;
	
	
	dispclr();
	if(Chk_info.Chk_mod == R_PRINTER){
		FrmPrintTest.prn_kind = R_PRI;				// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[0] );	//レシート
		memset( Chk_result.r_print, 0, 6);
	}
	else if(Chk_info.Chk_mod == J_PRINTER){
		FrmPrintTest.prn_kind = J_PRI;				// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[1] );	//ジャーナル
		memset( Chk_result.j_print, 0, 5);
	}
	
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[10] );	//
	Fun_Dsp( FUNMSG[0] );
	
	queset( PRNTCBNO, PREQ_PRINT_TEST, sizeof( T_FrmPrintTest ), &FrmPrintTest ); 			// テスト印字要求

	Lagtim( OPETCBNO, 6, 3*50 );					// 印字中画面表示ﾀｲﾏｰ6(3s)起動（ﾃｽﾄ印字中の画面表示を３秒間保持する）
	priend		= OFF;								// 印字終了ﾌﾗｸﾞﾘｾｯﾄ
	
	for( ; priend == OFF; ){

		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信

		switch( msg){			// 受信ﾒｯｾｰｼﾞ？
			case TSTPRI_END:	// テスト印字終了

				priend = ON;						// NO　→印字終了ﾌﾗｸﾞｾｯﾄ

				break;

			case TIMEOUT6:		// 印字中画面終了ﾀｲﾏｰﾀｲﾑｱｳﾄ

				priend = ON;							// 印字中画面表示ﾀｲﾏｰﾀｲﾑｱｳﾄ

				break;
		}
	}
	Lagcan( OPETCBNO, 6 );				
	
	sts = 0;
	pState = 0;
	sts_bk = 0xff;
	com = OFF;
	com_bk = 0xff;
	
	Fun_Dsp( FUNMSG[109] );
	for( ; Chk_end == 0; ){
		if( Chk_info.Chk_mod == R_PRINTER ){
			//	レシートプリンタ
			if(ERR_CHK[ERRMDL_PRINTER][01] != 0){
				com = ON;										// 通信不良
			}
			pState = (uchar)(rct_prn_buff.PrnStateMnt & 0x0f);	// 状態ﾘｰﾄﾞ
		}
		else{
			//	ジャーナルプリンタ
			if(ERR_CHK[ERRMDL_PRINTER][20] != 0){
				com = ON;										// 通信不良
			}
			pState = (uchar)(jnl_prn_buff.PrnStateMnt & 0x0f);	// 状態ﾘｰﾄﾞ
		}
		sts |= pState;											// 一度発生したエラーは解除しない
		if( com == OFF ){

			// 通信状態＝正常
			if(sts != sts_bk || com != com_bk){					// ステータス変化あり
				sts_bk = sts;
				com_bk = com;
				dispmlclr(3, 6);
				line	= 2;
				for( i = 0 ; i < 3 ; i++ ){						// 状態をﾁｪｯｸし発生しているｴﾗｰ情報を全て表示する
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[i+5] );	// ｴﾗｰ情報表示
					if( sts & PriSts[i] ){
						grachr( line, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[i+5] );	// ｴﾗｰ情報表示
					}
					line++;
				}
			}
		}
		else{
			if(com_bk == OFF){
				// 通信状態＝異常
				dispmlclr(3, 6);
				grachr( 3,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[9] );					// "　通信不良　　　　　　　　　　"
				com_bk =com;
			}
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		
		switch(msg){
			case KEY_TEN_F1:			// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// 次へ
				BUZPI();
				Chk_end = 1;
				break;
			case TIMEOUT8:
				if(Chk_info.Chk_mod == R_PRINTER){
					Disp_Chk_NG(PRI_CHK_NGSTR, 6, &Chk_result.r_print[0]);
					grachr( 0,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[0] );	//レシート
				}
				else{
					Disp_Chk_NG(PRI_CHK_NGSTR, 5, &Chk_result.j_print[0]);
					grachr( 0,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[1] );	//ジャーナル
				}
				displclr(1);
				Fun_Dsp( FUNMSG[109] );
				sts_bk = 0xff;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| シャッター/LEDチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Led_Shut_Chk( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	unsigned char	shutflg,startflg;
	startflg = 1;
	memset( Chk_result.led_shut, 0, 2);
	for( ; Chk_end == 0; ){
		if( startflg ){													// 画面表示
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[0] );	//
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[1] );	//
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[2] );	//
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[3] );	//
			Fun_Dsp(FUNMSG[109]);
			read_sht_opn();								// シャッター開
			Lagtim(OPETCBNO, 6, 50*2);					// 2秒
			shutflg = 1;
			startflg = 0;
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:					// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F5:					// 次へ
				BUZPI();
				Lagcan(OPETCBNO, 6);					// タイマーキャンセル
				read_sht_cls();
				Chk_end = 1;
				break;
			case TIMEOUT6:
				if(shutflg){
					read_sht_cls();
					shutflg = 0;
				}
				else{
					read_sht_opn();	
					shutflg = 1;
				}
				Lagtim(OPETCBNO, 6, 50*2);					// 2秒
				break;
			case TIMEOUT8:
				Lagcan(OPETCBNO, 6);					// タイマーキャンセル
				read_sht_cls();
				Disp_Chk_NG(SHUT_CHK_NGSTR, 2, &Chk_result.led_shut[0]);
				startflg = 1;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| アナウンスチェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define BIG_VOL		16
#define MID_VOL		8
#define SMALL_VOL	1
short Ann_Chk( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	unsigned char	disp_reset, ann;
	unsigned char	Ann_prm[3] =  { BIG_VOL, MID_VOL, SMALL_VOL };
	disp_reset = 1;
	key_num = 1;
	announceFee = 100;
	memset( Chk_result.ann, 0, 2);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[0] );	// "案内放送確認　　　　　　　　",
			grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[1] );	// "大「車室番号を入力して下さい」"
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[2] );	// "中「料金は１００円です」　　　"
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[3] );	// "小「このカードは使えません」　"
			Fun_Dsp(FUNMSG[109]);
			disp_reset = 0;
			ann = 0;
			pre_volume[0] = Ann_prm[0];
			avm_test_no[0] = 132;		// 車室番号を入力して下さい
			avm_test_cnt = 1;
			
			ope_anm(AVM_AN_TEST);		// アナウンス
		}
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:			// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// 次へ
				BUZPI();
				ope_anm( AVM_STOP );
				Chk_end = 1;
				break;
			case SODIAC_PLAY_CMP:
				grachr( ann+2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[ann+1] );	//
				ann++;
				if(ann >= 3){
					ann = 0;
				}
SOUND_REPLAY:
				if(ann < 3){
					pre_volume[0] = Ann_prm[ann];
				}
				switch(ann){
				case 0:
					avm_test_no[0] = 132;				// 車室番号を入力して下さい
					avm_test_cnt = 1;
					ope_anm(AVM_AN_TEST);				// アナウンス
					break;
				case 1:
					ope_anm(AVM_RYOUKIN);				// 料金読み上げ;
					break;
				case 2:
					avm_test_no[0] = 163;				// 車室番号を入力して下さい
					avm_test_cnt = 1;
					ope_anm(AVM_AN_TEST);				// アナウンス
					break;
				case 3:
					Lagtim( OPETCBNO, 13, (50*3));
				default:
					break;
				}
				if(ann < 3){
					grachr( ann+2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[ann+1] );	// 該当行を反転表示
				}
				break;
			case TIMEOUT13:
				ann = 0;
				goto	SOUND_REPLAY;
				break;
			case TIMEOUT8:
				ope_anm( AVM_STOP );
				Disp_Chk_NG(ANN_CHK_NGSTR, 2, &Chk_result.ann[0]);
				ann = 0;
				disp_reset = 1;
				break;
			default:
				break;
		}
		NG_key_chek();						// F4キーの監視
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| 入出力チェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Inout_Chk( void )
{

	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	Chk_end = 0;			// チェック終了フラグ
	unsigned char mod,i,pos,sts[5];
	struct {
		char sw;				// Chkmod_Sw_rd()に指定する引数
		unsigned short lin;		// 表示行
		unsigned short col;		// 表示位置
		unsigned short ann;		// アナウンス番号
	} sw_disp[5] = {
		{1, 1, 0, 223 },		// コインメック鍵
		{2, 2, 0, 225 },		// コイン金庫着脱
		{5, 3, 0, 229 },		// ドア
		{4, 4, 0, 230 },		// ドアノブ
		{3, 5, 0, 224 },		// 紙幣リーダー鍵
	};
	mod = 0;
	dispclr();
	grachr( 0,  0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[0] );	//
	for(i=0;i<5;i++){
		sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
		if(sts[i]){
			grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
		}
		else{
			grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
		}
	}
	memset( Chk_result.sw, 0, 5);
	Fun_Dsp(FUNMSG[109]);
	Lagcan( OPETCBNO, 8 );
	for( ; Chk_end == 0; ){
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		if(mod == 0){
			for(i=0; i<5; i++){
				if(sts[i] != Chkmod_Sw_rd(sw_disp[i].sw)){
					sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
					if(sts[i]){
						grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						avm_test_ch = 0;
						avm_test_cnt = 1;
						avm_test_no[0] = sw_disp[i].ann;
						ope_anm(AVM_AN_TEST);
					}
				}
			}
			switch(msg){
				case KEY_TEN_F1:			// 戻る
					BUZPI();
					return -1;
				case KEY_TEN_F5:			// 次へ
					BUZPI();
					Chk_end = 1;
					break;
				case TIMEOUT8:
					mod = 1;
					pos = 0;
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
					grachr( sw_disp[0].lin,  sw_disp[0].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[1] );
					for(i=1;i<5;i++){
						if(Chk_result.sw[i] == 0){
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
						else{
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
					}
					Fun_Dsp(FUNMSG[110]);
					break;
				default:
					break;
			}
			NG_key_chek();
		}
		else if(mod == 1){					// NG項目登録
			switch(msg){
				case KEY_TEN_F1:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					else{
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					if(pos > 0){
						pos-=1;
					} else {
						pos = 4;
					}
					grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					break;
				case KEY_TEN_F2:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					else{
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					if(pos < 4){
						pos++;
					}
					else{
						pos = 0;
					}
					grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					break;
				case KEY_TEN_F4:
					BUZPI();
					mod = 0;
					grachr( 0,  0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[0] );	//
					for(i=0;i<5;i++){
						sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
						if(sts[i]){
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
						else{
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
					}
					Fun_Dsp(FUNMSG[109]);
					break;
				case KEY_TEN_F5:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						Chk_result.sw[pos] = 1;
					}
					else{
						Chk_result.sw[pos] = 0;
					}
					break;
				default:
					break;
			}
		}
	}
	return 0;
}
// スイッチリード(funcchk.cより流用、一部改造)
unsigned char	Chkmod_Sw_rd(  char idx  )
{
	unsigned char ret;

	switch( idx ){		// スイッチ種別？

		case	0:		// 設定ＳＷ
			ret = ( CP_MODECHG ) ? 0 : 1;
			break;

		case	1:		// コインメックセットＳＷ
			ret = ( FNT_CN_DRSW ) ? 1 : 0;
			break;

		case	2:		// コイン金庫ＳＷ
			ret = ( FNT_CN_BOX_SW ) ? 1 : 0;
			break;

		case	3:		// 紙幣金庫ＳＷ
			ret = ( FNT_NT_BOX_SW ) ? 1 : 0;
			break;

		case	4:		// ドアノブ
			ret = ( OPE_SIG_DOORNOBU_Is_OPEN ) ? 0 : 1;
			break;

		case	5:		// ドア開
			ret = ( OPE_SIG_DOOR_Is_OPEN ) ? 0 : 1;
			break;

		case	6:		// 振動センサー
			ret = FNT_VIB_SNS;
			break;

		case	7:		// 紙幣リーダー脱落センサー
			ret = ( FNT_NT_FALL_SW ) ? 0: 1;
			break;

		case	15:		// 人体検知
			ret = ( FNT_MAN_DET ) ? 1: 0;
			break;

		default:		// その他
			ret = 0;
			break;

	}
	return(ret);
}


/*[]----------------------------------------------------------------------[]*/
/*| 信号チェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Sig_Chk( void )
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned short	sig_sts,Chk_end = 0;			// チェック終了フラグ
	unsigned char	onoff, sig_no,i,ng_mod = 0;
	unsigned char	res[8];
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[0] );	//
	for(i = 0; i < 7; i++){
		grachr( 3,  (7+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[3] );	//
	}
	Fun_Dsp(FUNMSG[0]);
	memset( Chk_result.sig, 0, 1);
	sig_no = 8;
	onoff = 1;
	ExIOSignalwt( sig_no , onoff);	
	Lagtim( OPETCBNO, 6, 10 );								// ﾃｽﾄﾀｲﾏｰ6(200ms)起動
	Lagcan( OPETCBNO, 8 );
	for( ; Chk_end == 0; ){
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:			// 戻る
				BUZPI();
				return -1;
			case KEY_TEN_F4:			// NG
				if(ng_mod){
					dispclr();
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[0] );	//
					for(i = 0; i < 7; i++){
						grachr( 3,  (7+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[res[i]] );	//
					}
					Fun_Dsp(FUNMSG[0]);
					ng_mod = 0;
				}
				break;
			case KEY_TEN_F5:			// 次へ
				if(sig_no >= 15){
					if(ng_mod){
						Chk_result.sig[0] = 1;		// 信号チェックNG
					}
					BUZPI();
					for(i = 0; i < 7; i++){
						ExIOSignalwt( i+7 , 0);
					}
					Chk_end = 1;
				}
				break;
			case TIMEOUT6:									// ﾃｽﾄﾀｲﾏｰﾀｲﾑｱｳﾄ
				sig_sts = ExIOSignalrd(INSIG_RXIIN);
				if(onoff){
					if(sig_sts != onoff){
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[1] );	// "×"
						res[sig_no-8] = 1;		// NG
						sig_no++;
					}
					else{
						onoff = 0;
					}
				}
				else{
					if(sig_sts != onoff){
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[1] );	// "×"
						res[sig_no-8] = 1;		// NG
					}
					else{
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[0] );	// "○"
						res[sig_no-8] = 0;		// OK
					}
					sig_no++;
					onoff = 1;
				}
				if(sig_no < 15){
					ExIOSignalwt( sig_no , onoff);	
					Lagtim( OPETCBNO, 6, 10 );	
				}
				else{
					for(i=0; i<7; i++){
						if(res[i]){
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[2] );	// "異常終了"
							break;
						}
					}
					if(i>=8){
						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[1] );	// "正常終了"
					}
					Fun_Dsp(FUNMSG[109]);
				}
				break;
			case TIMEOUT8:
				if(sig_no >= 15){
					ng_mod = 1;
					dispclr();
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_NGSTR[0] );	//
					Fun_Dsp( FUNMSG[110] );
					Chk_result.sig[0] = 1;
				}
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| NGキー(F4)長押し判定処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void NG_key_chek( void )
{
	if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F4)) ){
		if( OPECTL.on_off == 0 ){
			Lagcan( OPETCBNO, 8 );
		}
		else{
			Lagtim( OPETCBNO, 8, 3*50 );
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| メモリチェック(funccheck.cから流用)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned long	memory_adr[3] =
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
unsigned char Chk_Memory(void)
{
	uchar	ret	= 0;		// 戻り値
	ulong	i;				// ﾙｰﾌﾟ処理ｶｳﾝﾀｰ
	ushort	chk_cnt;		// 処理経過
	ushort	ram_no;			// ﾁｪｯｸ対象RAM No.
	uchar	*chk_adr;		// ﾁｪｯｸﾒﾓﾘｰｱﾄﾞﾚｽ
	uchar	chk_err;		// ﾁｪｯｸ結果
	uchar	sav_data;		// ﾃﾞｰﾀ退避ｴﾘｱ
	uchar	chk_data1;		// ﾁｪｯｸﾃﾞｰﾀ１
	uchar	chk_data2;		// ﾁｪｯｸﾃﾞｰﾀ２
	uchar	chk_no = 0;
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
		fck_chk_adr++;										// ﾁｪｯｸｱﾄﾞﾚｽ更新
		_ei();											// 割込み禁止解除
		if (fck_chk_err != 0) {
			// ＲＷﾁｪｯｸＮＧ
			break;
		}
	}
	Chk_result.set[chk_no] = fck_chk_err;
	chk_no++;
	xPause( 10 );

	//ＲＡＭ１メモリーチェック
	chk_err = 0;
	for (ram_no = 0 ; ram_no <= 1 ; ram_no++) {			// RAM毎のﾒﾓﾘｰﾁｪｯｸ処理ﾙｰﾌﾟ
		chk_err = 0;
		chk_adr = (uchar *)memory_adr[ram_no];
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
	Chk_result.set[chk_no] = fck_chk_err;
	chk_no++;
	xPause( 10 );

	//ＲＡＭ２メモリーチェック
	chk_err = 0;
	chk_adr = (uchar *)memory_adr[2];
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
	Chk_result.set[chk_no] = fck_chk_err;
	xPause( 10 );

	return(ret);
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気リーダー初期化                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 結果 0:正常                               |*/
/*|              :                    1:異常                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		DIPSWITCH_MAX		4
unsigned char Init_Magread( void )
{
	unsigned short	msg = 0;
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	int		loop;
	short	rd_snd;
	char	ret, i,j;
	uchar	result = 0;
	uchar	sts,sts_bit,dsw;				// ディップスイッチの状態(0:OFF, 1:ON)
	
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = TIMEOUT6;
	Target_WaitMsgID.Command[1] = ARC_CR_SND_EVT;
	Target_WaitMsgID.Command[2] = ARC_CR_E_EVT;

	OPE_red = 0;													// ﾘｰﾀﾞｰ自動排出としない
	Lagtim( OPETCBNO, 6, 15*50 );								// ﾀｲﾏｰ1(15s)起動

	rd_snd = i = 0;
	RD_mod = 0;
	for( ret = 0; ret == 0; ){
		if( i == 0 ){
			switch( RD_mod ){
				case 0:
					opr_snd( rd_snd = 0 );							// 初期化コマンド
					i = 1;
					break;
				case 6:
				case 8:
				case 20:
					opr_snd( rd_snd = 95 );							// 状態要求
					i = 1;
					break;
				case 10:
				case 11:
					Lagcan( OPETCBNO, 6 );							// Timer Cancel
					return 0;
				default:
					i = 1;
					break;
			}
		}

		for( loop=1; loop ; ){
			taskchg( IDLETSKNO );									// Change task to idletask
			msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID );		// 期待するﾒｰﾙだけ受信（他は溜めたまま）
			if( NULL == msb ){										// 期待するﾒｰﾙなし（未受信）
				continue;
			}

			msg = msb->msg.command;
			switch( msg ){
				case TIMEOUT6:
					loop = 0;
					break;
				case ARC_CR_SND_EVT:								// 送信完了
				case ARC_CR_E_EVT:									// 終了ｺﾏﾝﾄﾞ受信
					opr_ctl( msg );									// message分析処理
					loop = 0;
					break;
				default:
					break;
			}
			if( !loop ){
				FreeBuf( msb );										// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
			}
		}

		switch( msg ){
			case TIMEOUT6:											// ﾀｲﾏｰ3ﾀｲﾑｱｳﾄ
				/*** ﾘｰﾀﾞ正常ではない ***/
				opr_snd( 0 );
				ret = 1;
				result = 1;
				break;
			case ARC_CR_SND_EVT:									// 送信完了
			case ARC_CR_E_EVT:										// 終了ｺﾏﾝﾄﾞ受信
				if(( rd_snd == 95 )&&( msg == ARC_CR_E_EVT )){		// 状態データ受信
					ret = 1;
					for(i = 0;i<3;i++){						// ディップスイッチ状態を結果情報へ格納
						if(i == 2){
							dsw = 3;
						}
						else{
							dsw = i;
						}
						sts = RDS_REC.state[dsw];			// 下位4bit取得
						sts_bit = (0x01 << (DIPSWITCH_MAX-1));
						for (j = 0; j < 4; j++, sts_bit >>= 1) {
							if (sts & sts_bit) {
								Chk_res_ftp.Chk_Res03.Dip_sw[i][j] = 1;
								
							} else {
								Chk_res_ftp.Chk_Res03.Dip_sw[i][j] = 0;
							}
						}
					}
				}else{
					i = 0;											// 次の処理へ
				}
				break;
			default:
				break;
		}
	}
	Chk_result.set[3] = result;		// 初期化結果
	w_stat2 = 0;													// Rｺﾏﾝﾄﾞ受信 New Status
	Lagcan( OPETCBNO, 6 );											// Timer Cancel

	return result;
	
}

/*[]----------------------------------------------------------------------[]*/
/*| 紙幣リーダー初期化                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 結果 0:正常                               |*/
/*|              :                    1:異常                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_NoteReader( void )
{
	uchar	ret = 0;
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	uchar	Syn_flg, result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	Target_WaitMsgID.Count = 1;
	Target_WaitMsgID.Command[0] = TIMEOUT6;
	
	CP_CN_SYN = 0;			/* SYM Enable	*/
	Cnm_Force_Reset = 1;
	Lagtim( OPETCBNO, 6, 50 );	//1秒
	Syn_flg = 0;
	for(;ret == 0;){
		taskchg( IDLETSKNO );															// ｱｲﾄﾞﾙﾀｽｸ切替
		if(Cnm_Force_Reset == 0xff){
			ret = 1;
		}
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if( NULL == msb ){																// 期待するﾒｰﾙなし（未受信）
			continue;
		}
		//	受信ﾒｯｾｰｼﾞあり
		memcpy( &msg , msb , sizeof(MsgBuf) );											// 受信ﾒｯｾｰｼﾞ格納
		FreeBuf( msb );																	// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		switch( msg.msg.command ){														// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐
			case TIMEOUT6:
				if(Syn_flg == 0){
					CNMTSK_START = 1;													// ｺｲﾝﾒｯｸﾀｽｸ起動
					CP_CN_SYN = 1;
					Syn_flg = 1;
					Lagtim( OPETCBNO, 6, 50*10 );	//10秒
				}
				else{
					result = 1;
					ret = 1;
				}
				break;
			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 6 );																// Timer Cancel

	if( result ){
		Chk_result.set[4] = 1;															// 初期化結果
		Chk_result.set[5] = 1;															// 初期化結果
	}else{
		Chk_result.set[4] = cn_errst[1];												// 初期化結果
		Chk_result.set[5] = cn_errst[0];												// 初期化結果
	}
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| レシートプリンタ初期化                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 結果 0:正常                               |*/
/*|              :                    1:異常                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_R_Printer( void )
{
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	uchar	result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_R;
	Target_WaitMsgID.Command[1] = TIMEOUT6;
	Target_WaitMsgID.Command[2] = PREQ_SND_COMP_RP;
	Lagtim( OPETCBNO, 6, 50*10 );												// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰｽﾀｰﾄ（３秒）
	rct_init_sts = 0;
	PrnInit_R();
	if( PrnJnlCheck() != ON ){				// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？
		//	ｼﾞｬｰﾅﾙ接続なし
		jnl_init_sts	= 3;				// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（未接続）
	}
	for( ; ; ){
	
		taskchg( IDLETSKNO );															// ｱｲﾄﾞﾙﾀｽｸ切替
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if( NULL == msb ){																// 期待するﾒｰﾙなし（未受信）
			continue;
		}
		//	受信ﾒｯｾｰｼﾞあり
		memcpy( &msg , msb , sizeof(MsgBuf) );											// 受信ﾒｯｾｰｼﾞ格納
		FreeBuf( msb );																	// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		switch( msg.msg.command ){														// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐
			case TIMEOUT6:
				result = 1;
				break;
			case PREQ_INIT_END_R:														// ＜ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化終了通知＞

				rct_init_sts = 1;														// ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化完了）
				break;

			case	PREQ_SND_COMP_RP:
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
				
			default:
				break;
		}
		if(msg.msg.command == TIMEOUT6 || msg.msg.command == PREQ_INIT_END_R){
			break;
		}
	}
	Chk_result.set[6] = result;		// 初期化結果
	Lagcan( OPETCBNO, 6 );														// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰ解除
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| ジャーナルトプリンタ初期化                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 結果 0:正常                               |*/
/*|              :                    1:異常                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_J_Printer( void )
{
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	uchar	result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_J;
	Target_WaitMsgID.Command[1] = TIMEOUT6;
	Target_WaitMsgID.Command[2] = PREQ_SND_COMP_JP;
	Lagtim( OPETCBNO, 6, 50*10 );												// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰｽﾀｰﾄ（３秒）
	jnl_init_sts = 0;
	PrnInit_J();
	for( ; ; ){
	
		taskchg( IDLETSKNO );															// ｱｲﾄﾞﾙﾀｽｸ切替
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if( NULL == msb ){																// 期待するﾒｰﾙなし（未受信）
			continue;
		}
		//	受信ﾒｯｾｰｼﾞあり
		memcpy( &msg , msb , sizeof(MsgBuf) );											// 受信ﾒｯｾｰｼﾞ格納
		FreeBuf( msb );																	// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		switch( msg.msg.command ){														// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐
			case TIMEOUT6:
				result = 1;
				break;

			case PREQ_INIT_END_J:														// ＜ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化終了通知＞

				jnl_init_sts = 1;														// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化完了）
				break;

			case	PREQ_SND_COMP_JP:
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
			default:
				break;
		}
		if(msg.msg.command == TIMEOUT6 || msg.msg.command == PREQ_INIT_END_J){
			break;
		}
	}
	Chk_result.set[7] = result;		// 初期化結果
	Lagcan( OPETCBNO, 6 );														// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰ解除
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| 音声モジュール初期化                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Init_Sound( void )      	                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 結果 0:正常                               |*/
/*|              :                    1:異常                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned char Init_Sound( void )
{
	AVM_Sodiac_Init();
	return (uchar)( Avm_Sodiac_Err_flg != SODIAC_ERR_NONE );
}
/*[]----------------------------------------------------------------------[]*/
/*| NG項目登録処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : unsigned char (*dat)[31]: 表示する文字列                |*/
/*|              : unsigned char cnt       : 項目数                        |*/
/*|              : unsigned char *result   : 結果情報領域のポインタ        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Disp_Chk_NG(const unsigned char (*dat)[31], unsigned char cnt, unsigned char *result)
{
	short			msg = -1;			// 受信ﾒｯｾｰｼﾞ
	unsigned char	i, end = 0;			// チェック終了フラグ
	char pos,pre;
	char ng_tbl[6] = {0};
	
	memcpy(ng_tbl, result, cnt);
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
	for(i=0;i<cnt;i++){
		if((ng_tbl[i] == 0) || (ng_tbl[i] == 0xff)){							// 正常または未実施
			grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[i] );	//
		}
		else{
			grachr( i+1,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[i] );	//
		}
	}
	pos = pre = 1;
	grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF,dat[pos-1] );	//反転表示
	Fun_Dsp( FUNMSG[110] );
	for( ; end == 0; ){
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F1:
				BUZPI();
				pos--;
				if(pos <= 0){
					pos = cnt;
				}
				if((ng_tbl[pre-1] == 0) || (ng_tbl[pre-1] == 0xff)){
					grachr( pre,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[pre-1] );	//通常表示
				}
				else{
					grachr( pre,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[pre-1] );	//異常表示
				}
				grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, dat[pos-1] );	//反転表示
				pre = pos;
				break;
			case KEY_TEN_F2:
				BUZPI();
				pos++;
				if(pos > cnt){
					pos = 1;
				}
				if((ng_tbl[pre-1] == 0) || (ng_tbl[pre-1] == 0xff)){
					grachr( pre,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[pre-1] );	//通常表示
				}
				else{
					grachr( pre,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[pre-1] );	//異常表示
				}
				grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, dat[pos-1] );	//反転表示
				pre = pos;
				break;
			case KEY_TEN_F5:						// 登録
				BUZPI();
				if(ng_tbl[pos-1] == 1){
					ng_tbl[pos-1] = 0;				// 正常
				}
				else{
					ng_tbl[pos-1] = 1;				// NG
				}
				break;
			case KEY_TEN_F4:
				BUZPI();
				memcpy(result, ng_tbl, cnt);		// 結果情報に反映
				end = 1;
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 検査終了処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Chk_Exit(void)
{
	ushort msg;
	uchar	i;
	T_FrmChk_result	Frm_result;
	dispclr();
	bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[4]);	// "検査完了
	grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
	grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[5]);				// "
	// debug ファイル名作成
	/* 結果情報ファイル名の作成 */
	sprintf((char *)&Chk_info.dir_chkresult, (const char*)SYSMNT_CHKRESULT_DIRECT);
	sprintf((char *)&Chk_info.fn_chkresult, (const char*)SYSMNT_CHKRESULT_FILENAME, 
			"123456"							/*パターン番号を入れる予定*/,
			Chk_result.Chk_date.Year%100, 		/*検査日時*/
			Chk_result.Chk_date.Mon, 			/**/
			Chk_result.Chk_date.Day, 			/**/
			Chk_result.Chk_date.Hour, 			/**/
			Chk_result.Chk_date.Min			/**/
	);
	// 結果情報格納処理(仮)
	memcpy( &Chk_res_ftp.Chk_Res01.Chk_date, &Chk_result.Chk_date, 6);
	Chk_res_ftp.Chk_Res01.Kakari_no = Chk_result.Kakari_no;
	strcpy((char *)Chk_res_ftp.Chk_Res01.Machine_No, "123456");
	strcpy((char *)Chk_res_ftp.Chk_Res01.Model, "FT4000FX");
	Chk_res_ftp.Chk_Res01.System = 10;
	Chk_res_ftp.Chk_Res01.Sub_Money = 100;
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[0], "MH123456");
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[1], "MH012345");
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[2], "MH111111");
	for(i=0;i<27;i++){
		strcpy( (char *)Chk_res_ftp.Chk_Res05.Mojule[i], "MH111111" );
	}
	Fun_Dsp( FUNMSG[113] );

	for( ; ; )
	{
		msg = GetMessage();				// ﾒｯｾｰｼﾞ受信
		switch(msg){
			case KEY_TEN_F5:			// 再検査
				Chk_info.pt_no = 0;
				Chk_info.Chk_mod = 0;
				memset(&Chk_info.Chk_str1, 0, 30);		// 固定文字列1・2をクリアする
				return;
			case KEY_TEN_F3:			// FTP
				BUZPI();
				SysMnt_FTPServer();
				dispclr();
				bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[4]);	// "検査完了
				grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
				grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[5]);				// "]
				Fun_Dsp( FUNMSG[113] );
				break;
			case KEY_TEN_F4:		// 印字
				Frm_result.prn_kind = R_PRI;
				Frm_result.Serial_no = 999999;			// TODO:ここはバーコードスキャンした製品号機を設定するので保留
				memcpy(&Frm_result.ChkTime, &Chk_result.Chk_date, sizeof(date_time_rec));
				Frm_result.Kakari_no = Chk_result.Kakari_no;
				Frm_result.Chk_no = Chk_result.Chk_no;
				Frm_result.Card_no = Chk_result.Card_no;
				queset( PRNTCBNO, PREQ_CHKMODE_RESULT, sizeof(T_FrmChk_result), &Frm_result );
				break;;
			case KEY_TEN_F1:			// 再検査
				Chk_info.pt_no = 0;
				Chk_info.Chk_mod = 0;
				System_reset();				// 再起動
				break;
			default:
				break;
		}
	}
}
	
/*[]----------------------------------------------------------------------[]*/
/*| 検査モード前処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 0:初回                                    |*/
/*|                              1:検査中                                  |*/
/*|                              2:検査終了                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_Chkmod(void)
{
	
	if (stricmp((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1) != 0){			// 固定文字列①不一致
		/*イニシャライズして最初から実施*/
		log_init();
		memclr(1);
		memset(&Chk_info, 0, sizeof(t_Chk_info));		// 検査情報エリアクリア
		return 0;
	}
	else if(stricmp((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2) != 0){		// 固定文字列①のみ一致
		/* 検査途中での復電 */
		return 1;
	}
	else{				// 両方一致
		/* 検査終了後の復電 */
		return 2;
	}
}

