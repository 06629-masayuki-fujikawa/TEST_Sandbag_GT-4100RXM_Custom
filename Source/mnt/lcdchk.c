/*[]----------------------------------------------------------------------[]*/
/*| LCDﾁｪｯｸ                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.05.05                                               |*/
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
#include	"LKmain.H"
#include	"PRM_TBL.H"
#include	"can_def.h"

#define	LCDBM_WAIT_TIMEOUT	5		// LCDモジュールとの通信タイムアウト時間（"しばらくお待ち下さい"を表示する最大時間）

static void	FColorLCDChk_TestPtnMenu(char pos, ushort rev);
static unsigned short	FColorLCDChk_TestPtnDsp(char ptn);
static unsigned short FColorLCDChk_GetColorPtn(char ptn);

/*[]----------------------------------------------------------------------[]*/
/*| LCDチェックメニュー画面                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Lcd( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	FunChk_Lcd( void ){

	unsigned short	usFlcdEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[0] );					/* "＜ＬＣＤチェック＞　　　　　　　" */
		usFlcdEvent = Menu_Slt( FLCDMENU, FLCD_CHK_TBL, (char)FLCD_CHK_MAX2, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFlcdEvent ){
//未使用です（S）
			case FLCD1_CHK:
				usFlcdEvent = FColorLCDChk_Bk();				// ﾊﾞｯｸﾗｲﾄﾃｽﾄ
				break;
//未使用です（E）
			case FLCD2_CHK:
				usFlcdEvent = FColorLCDChk_Lumine_change();		// 輝度調整
				break;
//未使用です（S）
			case FLCD3_CHK:
				usFlcdEvent = FColorLCDChk_PatternDSP();		// 表示器ﾃｽﾄ
				break;
			case FLCD4_CHK:
				//usFlcdEvent = FLcdChk_T2();
				break;
//未使用です（E）
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFlcdEvent;
				break;
			default:
				break;
		}
		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );							// 操作履歴登録
			OpelogNo = 0;
			SetChange = 1;
			UserMnt_SysParaUpdateCheck( OpelogNo );
			SetChange = 0;
		}
		
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
//		if( usFlcdEvent == MOD_CHG){
		if( usFlcdEvent == MOD_CHG || usFlcdEvent ==  MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFlcdEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			表示器テスト
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_PatternDSP(void)
{
	//変数
	short pos = 0;
	short	msg;					// 受信メッセージ
	unsigned short	ret;			// 戻り値

	//メンテナンス画面表示
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[18]);	// "＜表示器テスト＞　　　　　　　"
	grachr(2,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);			// "   黄   "
	grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);			// "   赤   "
	grachr(3,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);			// "   青   "
	grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);			// "   黒   "
	grachr(4,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);			// "   緑   "
	grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);			// "   白   "
	FColorLCDChk_TestPtnMenu(pos,1);										// 反転
	Fun_Dsp( FUNMSG2[46] );													// "　⊂　　⊃　 　　　表示  終了 "

	//操作
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				dsp_background_color(COLOR_WHITE);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:	// モードチェンジ
				BUZPI();
				dsp_background_color(COLOR_WHITE);
				return MOD_CHG;

			case KEY_TEN_F5:	// 終了
				BUZPI();
				dsp_background_color(COLOR_WHITE);
				return MOD_EXT;

			case KEY_TEN_F1:
			case KEY_TEN_F2:
				//一度背景を変更すると画面を抜けるまで操作させない
				BUZPI();
				FColorLCDChk_TestPtnMenu(pos,0);			// 正転
				if(msg == KEY_TEN_F1){
					pos--;
					if(pos < 0){
						pos = 5;
					}
				}
				else{
					pos++;
					if(pos > 5){
						pos = 0;
					}
				}
				FColorLCDChk_TestPtnMenu(pos,1);			// 反転
				break;
			
			case KEY_TEN_F4:
				BUZPI();
				// カラーLCD表示
				ret = FColorLCDChk_TestPtnDsp(pos);							// テスト表示
				if(ret == MOD_CHG) {
					dsp_background_color(COLOR_WHITE);
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				if( ret == MOD_CUT) {
					dsp_background_color(COLOR_WHITE);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
				else if(ret == MOD_EXT) {
					dsp_background_color(COLOR_WHITE);
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[18]);	// "＜表示器テスト＞　　　　　　　"
					grachr(2,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);			// "   黄   "
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);			// "   赤   "
					grachr(3,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);			// "   青   "
					grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);			// "   黒   "
					grachr(4,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);			// "   緑   "
					grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);			// "   白   "
					FColorLCDChk_TestPtnMenu(pos,1);									// 反転
					Fun_Dsp( FUNMSG2[46] );												// "　⊂　　⊃　 　　　表示  終了 "
				}
				break;
			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			バックライトテスト
//[]----------------------------------------------------------------------[]
///	@return			MOD_CHG	: <br>
///					MOD_EXT	: 
//[]----------------------------------------------------------------------[]
///	@author			ichihara
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_Bk( void )
{

	char	val1 = 1;
	short			msg;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[19] );		/* // [19]	"＜バックライトテスト＞　　　　" */
	grachr( 2,  0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[20] );		/* // [20]	"　バックライト　　　　　　　　" */
	grachr( 2,  16, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_0[val1?0:1] );	/* // [XX]	"　　　　　　　　　ＯＮ　　　　" */
	Fun_Dsp( FUNMSG[50] );														/* // [50]	"　　　　　　　　　 ON/OF 終了 " */


	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( msg ){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case LCD_DISCONNECT:
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
			case KEY_MODECHG:
				BUZPI();
												/* 共通ﾊﾟﾗﾒｰﾀ43-21取得 */
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_CHG );
				break;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
												/* 共通ﾊﾟﾗﾒｰﾀ43-21取得 */
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_EXT );
				break;
			case KEY_TEN_F4:					/* F4:Update */
				BUZPI();
												/* ON/OFF */
				val1 = (0==val1) ? 1 : 0;
				lcd_backlight( (char)val1 );
				break;
			default:
				break;
		}
		grachr( 2,  16, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_0[val1?0:1] );
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			輝度調整
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_Lumine_change(void)
{
	ushort	msg;
	int		lumine_change = 0;
	uchar	side;								// 0:点灯(精算時･夜時間帯)，1:消灯(待機時･昼時間帯)
	unsigned short	lumine_num[2];						// [0]:点灯時の輝度，[1]:消灯時の輝度
	int		mode_change = 0;
	ushort	column;								// 描画位置
	side = 0;
	lumine_num[0] = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027⑤⑥
	lumine_num[1] = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028⑤⑥
	/*	不正な輝度を取得した場合 */
	if( lumine_num[side] > LCD_LUMINE_MAX )
	{
		lumine_num[side] = LCD_LUMINE_MAX;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0]);	// "＜輝度調整＞　　　　　　　　　"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[7]);	// "⊂⊃キーで輝度を設定して下さい"
	displclr(3);
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[8]);	// "輝度設定：　　　　　　　　　　"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[9]);	// "0 　　　　　　　　　　　　　15"
	Fun_Dsp(FUNMSG2[2]);						// "　⊂　　⊃　　　　 書込  終了 "
	lumine_change = 1;

	while (1) {
		msg = StoF(GetMessage(), 1);											// ﾒｯｾｰｼﾞ受信

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			mode_change = 2;	// 「操作モード切替イベント受信済み」とする
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:		// 操作モード切替
			mode_change = 1;	// 「操作モード切替イベント受信済み」とする
			break;

		case KEY_TEN_F4:														// 書込
			BUZPI();
			CPrmSS[S_PAY][27] = (unsigned char)(lumine_num[side]);				// CPrmSS & Flash Data Update
			OpelogNo = OPLOG_CONTRAST;				// 操作履歴登録
			f_ParaUpdate.BIT.other = 1;				// 復電時にRAM上パラメータデータのSUM更新する
// MH810100(S) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
 			mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 車番チケットレス(メンテナンス)
// GG120600(S) // Phase9 設定変更通知対応
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 設定変更通知対応

			break;

		case KEY_TEN_F5:														// 終了
			BUZPI();
			// 設定値をLCDモジュールに反映
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));
			Lagcan( OPETCBNO, 6 );
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
			if (lumine_num[side] < LCD_LUMINE_MAX ) {
				lumine_num[side]++;
				lumine_change = 1;
			}
			break;
		default:
			break;
		}

		if (lumine_change) {
			lcd_contrast((unsigned char)lumine_num[side]);
			backlight = lumine_num[side];
			opedsp(4, 10, (ushort)lumine_num[side], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// 輝度
			if (lumine_num[side] < LCD_LUMINE_MAX) {
				dsp_background_color(COLOR_DARKORANGE);
				column = (ushort)(lumine_num[side]*2);
				if(column) {
					grachr(6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);	// プログレスバー表示
				}
				dsp_background_color(COLOR_WHITE);
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
				dsp_background_color(COLOR_DARKORANGE);
				grachr(6 , 0, 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);			// プログレスバー表示
				dsp_background_color(COLOR_WHITE);
				lumine_num[side] = LCD_LUMINE_MAX;
			}
			lumine_change = 0;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)
//		if ( mode_change != 0 ) {
//			return MOD_CHG;
//		} 
// MH810100(E) Y.Yamauchi 2020/1/6 車番チケットレス(メンテナンス)	
		if ( mode_change == 1 ) {
			return MOD_CHG;
		} else if( mode_change == 2 ) {
			return MOD_CUT;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			表示器テストパターン表示
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: 表示開始位置
///	@param[in]		rev		: 反転指定
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static void	FColorLCDChk_TestPtnMenu(char pos, ushort rev)
{
	switch(pos){
	case 0:
		grachr(2,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);				// "   黄   "
		break;

	case 1:
		grachr(2, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);				// "   赤   "
		break;

	case 2:
		grachr(3,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);				// "   青   "
		break;

	case 3:
		grachr(3, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);				// "   黒   "
		break;

	case 4:
		grachr(4,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);				// "   緑   "
		break;

	case 5:
		grachr(4, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);				// "   白   "
		break;

	case 6:
		grachr(5,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);				// "パターン"
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			表示器テストパターン表示
//[]----------------------------------------------------------------------[]
///	@param[in]		ptn		: テストパターン
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static unsigned short	FColorLCDChk_TestPtnDsp(char ptn)
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned short	color;
	unsigned char	line;
	char			i;
	unsigned char	column;
	ushort	msg;

	if (ptn < 6) {
		// 黄、赤、青、黒、緑、白
		color = FColorLCDChk_GetColorPtn(ptn);
		dsp_background_color( color );
		dispclr();											//画面クリア
	} else {
		// ﾊﾟﾀｰﾝ
		for( line = 0; line < 8; line++ ) {
			// 縦8ﾗｲﾝ分
			i = 0;
			for( column = 0; column < LCD_CLM; column++ ) {
				// 30桁分
				color = FColorLCDChk_GetColorPtn(i);

				// 文字フォント指定
				memset(&DispFont, 0, sizeof(t_DispFont));
				DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
				DispFont.Type = LCD_NORMAL_FONT;					// ﾉｰﾏﾙﾌｫﾝﾄ
				DispFontDataSnd(&DispFont);

				// 文字カラー指定
				memset(&DispColor, 0, sizeof(t_DispColor));
				DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
				DispColor.Color = color;							// 色指定
				DispColorDataSnd( &DispColor );

				// 文字列指定
				memset(&DispStr, 0, sizeof(t_DispStr));
				DispStr.Esc = LCD_ESC_STR;							// 文字列指定
				DispStr.Row = line;									// 指定行
				DispStr.Colmun = 0;									// 指定ｶﾗﾑ
				memcpy( &DispStr.Msg, "  ", 2 );
				can_snd_data4((void *)&DispStr, 4 + 2);

				if( i < 5 ) {
					i++;
				} else {
					i = 0;
				}
			}
		}
		dispclr();											//画面クリア
	}
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 車番チケットレス(メンテナンス)
		case KEY_MODECHG:	// モードチェンジ
			BUZPI();
			return MOD_CHG;
		default:
			if(msg >= KEY_TEN0 && msg <= KEY_TEN_CL) {		// テンキーで前の画面に戻る
				BUZPI();
				return MOD_EXT;
			}
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			色情報(RGB)取得
//[]----------------------------------------------------------------------[]
///	@param[in]		ptn		: テストパターン
///	@return			unsigned short ： 色情報(RGB)
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static unsigned short FColorLCDChk_GetColorPtn(char ptn)
{
	unsigned short	color;

	switch(ptn){
	case 0:			// 黄
		color = COLOR_YELLOW;
		break;

	case 1:			// 赤
		color = COLOR_RED;
		break;

	case 2:			// 青
		color = COLOR_BLUE;
		break;

	case 3:			// 黒
		color = COLOR_BLACK;
		break;

	case 4:			// 緑
		color = COLOR_GREEN;
		break;

	case 5:			// 白
		color = COLOR_WHITE;
		break;

	case 6:			// ﾊﾟﾀｰﾝ
	default:
		return 0;
	}
	return color;
}

