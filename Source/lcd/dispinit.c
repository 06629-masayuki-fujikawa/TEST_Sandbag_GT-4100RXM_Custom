/*[]----------------------------------------------------------------------[]*/
/*| Display function                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update      : 新精算機対応 2012-12-15 k.totsuka                        |*/
/*| Date        :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"lcd_def.h"
#include	"iodefine.h"
#include	"strdef.h"
#include	"prm_tbl.h"

#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"tbl_rkn.h"
#include	"can_def.h"
// MH810100(S) 
#include	"pktctrl.h"
// MH810100(E) 

t_blk_lg LCD_Blk_lg[LCD_BLK_LG];			/* Blink area register */
unsigned char	LCD_BLKLG_CNT;				/* Blink area count */
unsigned short	DspSts;						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
unsigned short	DspSts2;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
T_WMSG_CNT		Lcd_Wmsg_Cnt[LCD_CLM];		// ﾜｰﾆﾝｸﾞ表示制御ｴﾘｱ

unsigned char	LCD_BACKUP_MSG6[LCD_CLM];	// 6Line目ﾊﾞｯｸｱｯﾌﾟ表示ﾃﾞｰﾀ
unsigned short	LCD_BACKUP_COLOR6[LCD_CLM];	// 6Line目ﾊﾞｯｸｱｯﾌﾟ表示色
unsigned char	LCD_BACKUP_MSG7[LCD_CLM];	// 7Line目ﾊﾞｯｸｱｯﾌﾟ表示ﾃﾞｰﾀ
unsigned short	LCD_BACKUP_COLOR7[LCD_CLM];	// 7Line目ﾊﾞｯｸｱｯﾌﾟ表示色
unsigned char	LCD_BACKUP_Flag[8];


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispinit                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : M66273AFP(Display Controler) initialization             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : char result = 0:OK/ 1:NG                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	dispinit( void )
{
	unsigned char	i;

	// ﾜｰﾆﾝｸﾞ表示制御ｴﾘｱ初期化
	DspSts = LCD_WMSG_OFF;								// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示状態	：ＯＦＦ
	DspSts2 = LCD_WMSG_OFF;								// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示状態	：ＯＦＦ
	for( i=0; i<LCD_CLM; i++){
		Lcd_Wmsg_Cnt[i].WarMsg = 0x20;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀ	：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].WarColor = 0;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示色	：ｸﾘｱ

		Lcd_Wmsg_Cnt[i].RecMsg = 0x20;					// 通常表示ﾃﾞｰﾀ			：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].RecColor = 0;					// 通常表示色			：ｸﾘｱ

		Lcd_Wmsg_Cnt[i].WarMsg2 = 0x20;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀ	：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].WarColor2 = 0;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示色	：ｸﾘｱ

		Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;					// 通常表示ﾃﾞｰﾀ			：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].RecColor2 = 0;					// 通常表示色			：ｸﾘｱ
	}

	LCD_BLKLG_CNT = 0;

	memset ( &LCD_BACKUP_Flag[0], 0 , sizeof(LCD_BACKUP_Flag));

	Disp_color_font_now.Color	= 0xFFFE;				//現在設定中のカラーとフォント種別を初期化
	Disp_color_font_now.Type 	= 0xFF;

	memset(Disp_blink_now, 0xFF, sizeof(Disp_blink_now));

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispclr                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display All Clear                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dispclr( void )
{
// MH810100(S) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
	t_DispClr DispClr;
	unsigned char	i;

	memset( &DispClr, 0, sizeof(t_DispClr));
	DispClr.Esc = LCD_ESC_CLEAR;							// ｸﾘｱ指定
	DispClr.Row = 0;										// 指定行
	DispClr.Colmun = 0;										// 指定ｶﾗﾑ
	DispClr.Scope = 1;										// 全画面ｸﾘｱ
	can_snd_data4((void *)&DispClr, sizeof(DispClr));

	for( i=0; i<LCD_CLM; i++){
		Lcd_Wmsg_Cnt[i].RecMsg = 0x20;						// 通常表示ﾃﾞｰﾀ	：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].RecColor = 0;						// 通常表示色	：ｸﾘｱ

		Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;						// 通常表示ﾃﾞｰﾀ	：ｸﾘｱ
		Lcd_Wmsg_Cnt[i].RecColor2 = 0;						// 通常表示色	：ｸﾘｱ
	}

	if( DspSts == LCD_WMSG_ON ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中　且つ　表示面が通常表示面（精算表示面）
		Lcd_Wmsg_Disp();									// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
	}
	if( DspSts2 == LCD_WMSG_ON ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中　且つ　表示面が通常表示面（精算表示面）
		Lcd_Wmsg_Disp2();									// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
	}
// MH810100(S) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
#else
	// メンテ中のみ表示を行う
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
	PKTcmd_text_1_clear( 0, 0, 1 );	// 全クリア
// MH810100(S) Y.Yamauchi 2020/03/18 車番チケットレス(#4036 料金設定の店割引を店001から005まで設定すると、画面操作不能になる)
	taskchg( IDLETSKNO );
// MH810100(E) Y.Yamauchi 2020/03/18 車番チケットレス(#4036 料金設定の店割引を店001から005まで設定すると、画面操作不能になる)
#endif
// MH810100(E) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : displclr                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Line Clear                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short line	 = 0-7                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	displclr( unsigned short lin )
{
// MH810100(S) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
	t_DispClr DispClr;
	unsigned char	i;

	memset( &DispClr, 0, sizeof(t_DispClr));
	DispClr.Esc = LCD_ESC_CLEAR;							// ｸﾘｱ指定
	DispClr.Row = (unsigned char)lin;						// 指定行
	DispClr.Colmun = 0;										// 指定ｶﾗﾑ
	DispClr.Scope = 0;										// 1行クリア
	can_snd_data4((void *)&DispClr, sizeof(DispClr));

	if( lin == LCD_WMSG_LINE ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示行　の場合
		for( i=0; i<LCD_CLM; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = 0x20;					// 通常表示ﾃﾞｰﾀ	：ｸﾘｱ
			Lcd_Wmsg_Cnt[i].RecColor = 0;					// 通常表示色	：ｸﾘｱ
		}
	}

	if( DspSts == LCD_WMSG_ON ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中　且つ　表示面が通常表示面（精算表示面）
		Lcd_Wmsg_Disp();									// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
	}

	if( lin == LCD_WMSG_LINE2 ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示行　の場合
		for( i=0; i<LCD_CLM; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;					// 通常表示ﾃﾞｰﾀ	：ｸﾘｱ
			Lcd_Wmsg_Cnt[i].RecColor2 = 0;					// 通常表示色	：ｸﾘｱ
		}
	}

	if( DspSts2 == LCD_WMSG_ON ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中　且つ　表示面が通常表示面（精算表示面）
		Lcd_Wmsg_Disp2();									// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
	}
// MH810100(S) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
#else
	// メンテ中のみ表示を行う
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
	PKTcmd_text_1_clear( lin, 0, 0 );	// 行クリア
#endif
// MH810100(E) K.Onodera 2019/12/17 車番チケットレス（画面表示変更）
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispmlclr                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Multi Line Clear                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : from = start line                                       |*/
/*|                to   = end line                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dispmlclr( unsigned short from, unsigned short to )
{
	while( from <= to ){
		displclr( from++ );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : grachr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Graphic Character Disp (8LineMode)              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7(8line mode) / 0-6(7)    |*/
/*|              : unsigned short col	colmn No. = 0-29th                 |*/
/*|              : unsigned short cnt	character number = 1-30            |*/
/*|              : unsigned short mod	mode = 0(normal) / 1(reverse)      |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned short * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-26                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	grachr( unsigned short lin,
					unsigned short col,
					unsigned short cnt,
					unsigned short mod,
					unsigned short color,
					unsigned short blink,
					const unsigned char *dat )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;
	char i, j, cursor;



	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

// MH810100(S) 
#if GT4100_NO_LCD
// MH810100(E) 
	/* 空白を反転指定された場合は背景色を変えて空白表示する */
	cursor = 0;
	if(mod != 0){					// 反転指定
		for(i=0; i<cnt; i+=2){
			if(dat[i] == 0x81){
				if(dat[i+1] == 0x40){
					continue;
				}
			}
			break;
		}
		if( i >= cnt){					// 全て空白
			dsp_background_color(LCD_CURSOR_COLOR);
			cursor = 1;
		}
	}

	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;						// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_NORMAL_FONT;						// ﾉｰﾏﾙﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	// ブリンク指定
	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
	DispBlink.Row = (unsigned char)lin;					// 行指定
	DispBlink.Colmun = 0;								// 列指定(設定無効)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
	}
	DispBlinkDataSnd(&DispBlink);

	// ﾜｰﾆﾝｸﾞ表示制御
/* 	ﾜｰﾆﾝｸﾞ表示中にﾜｰﾆﾝｸﾞ表示行(6,7行目)に通常表示しようとした場合、現状の処理では
	①以前の通常メッセージリカバリ 
	②ﾜｰﾆﾝｸﾞ再表示	→終了
	つまり表示しようとした通常メッセージは表示されない
	この処理の意図が不明だがとりあえず①の処理は不要と思うので削除
	この場合の処理は、「通常表示しようとしたメッセージをリカバリエリアにセットして表示は更新しない」ように修正する		*/
	// 文字カラー指定
	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
	/* modがnormal指定の場合 */
	if( 0 == mod )
	{
		DispColor.Color = color;							// 色指定
	}
	/* FT4800で反転指定の場合はFT4000では赤表示を行う */
	else
	{
		DispColor.Color = COLOR_RED;					// 赤指定
	}
	DispColorDataSnd( &DispColor );

	// 文字列指定
	DispStr.Esc = LCD_ESC_STR;							// 文字列指定
	DispStr.Row = (unsigned char)lin;					// 行指定
	DispStr.Colmun = (unsigned char)col;				// 列指定
	memcpy( &DispStr.Msg, dat, (size_t)cnt );
	if(lin != LCD_WMSG_LINE && lin != LCD_WMSG_LINE2){
		can_snd_data4((void *)&DispStr, 4 + cnt);
	}

	// ﾜｰﾆﾝｸﾞ表示制御
	if( lin == LCD_WMSG_LINE ){
		// 表示行がﾜｰﾆﾝｸﾞ表示行の場合、通常表示ﾃﾞｰﾀ保存
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor = DispColor.Color;
			j++;
		}

		if(DspSts != LCD_WMSG_ON){
			can_snd_data4((void *)&DispStr, 4 + cnt);
		}
	}
	if( lin == LCD_WMSG_LINE2 ){
		// 表示行がﾜｰﾆﾝｸﾞ表示行の場合、通常表示ﾃﾞｰﾀ保存
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor2 = DispColor.Color;
			j++;
		}

		if(DspSts2 != LCD_WMSG_ON){
			can_snd_data4((void *)&DispStr, 4 + cnt);
		}
	}
	if(cursor){
		dsp_background_color(COLOR_WHITE);
	}
// MH810100(S) 
#else
	// メンテ中のみ表示を行う
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
//	PKTcmd_text_begin();
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	/* 空白を反転指定された場合は背景色を変えて空白表示する */	
	cursor = 0;
	if(mod != 0){					// 反転指定
		for(i=0; i<cnt; i+=2){
			if(dat[i] == 0x81){
				if(dat[i+1] == 0x40){
					continue;
				}
			}
			break;
		}
		if( i >= cnt){					// 全て空白
			PKTcmd_text_1_backgroundcolor(LCD_CURSOR_COLOR);
			cursor = 1;
		}
	}
// MH810100(S) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	PKTcmd_text_begin();
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない

	// 文字フォント指定
//	DispFont.Esc = LCD_ESC_FONTTYPE;						// 文字ﾌｫﾝﾄ指定
//	DispFont.Type = LCD_NORMAL_FONT;						// ﾉｰﾏﾙﾌｫﾝﾄ
//	PKTcmd_text_color(&DispFont);
	PKTcmd_text_font(LCD_NORMAL_FONT);

	// ブリンク指定
//	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
	DispBlink.Row = (unsigned char)lin;					// 行指定
	DispBlink.Colmun = 0;								// 列指定(設定無効)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
	}
//	PKTcmd_text_blink(&DispBlink);
	PKTcmd_text_blink(lin, 0, (uchar)DispBlink.Mode, (uchar)DispBlink.Interval);

	// ﾜｰﾆﾝｸﾞ表示制御
/* 	ﾜｰﾆﾝｸﾞ表示中にﾜｰﾆﾝｸﾞ表示行(6,7行目)に通常表示しようとした場合、現状の処理では
	①以前の通常メッセージリカバリ 
	②ﾜｰﾆﾝｸﾞ再表示	→終了
	つまり表示しようとした通常メッセージは表示されない
	この処理の意図が不明だがとりあえず①の処理は不要と思うので削除
	この場合の処理は、「通常表示しようとしたメッセージをリカバリエリアにセットして表示は更新しない」ように修正する		*/
	// 文字カラー指定
//	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
	/* modがnormal指定の場合 */
	if( 0 == mod )
	{
		DispColor.Color = color;							// 色指定
	}
// MH810100(S) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	else if(cursor == 1) {
		DispColor.Color = LCD_CURSOR_COLOR;				// カーソル
	}
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	/* FT4800で反転指定の場合はFT4000では赤表示を行う */
	else
	{
		DispColor.Color = COLOR_RED;					// 赤指定
	}
//	PKTcmd_text_backgroundcolor( &DispColor );
// MH810100(S) S.Takahashi 2019/12/18
//	PKTcmd_text_backgroundcolor( DispColor.Color );
	PKTcmd_text_color(DispColor.Color);
// MH810100(E) S.Takahashi 2019/12/18

	// 文字列指定
//	DispStr.Esc = LCD_ESC_STR;							// 文字列指定
	DispStr.Row = (unsigned char)lin;					// 行指定
	DispStr.Colmun = (unsigned char)col;				// 列指定
	memcpy( &DispStr.Msg, dat, (size_t)cnt );
	if(lin != LCD_WMSG_LINE && lin != LCD_WMSG_LINE2){
//		PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
		PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
	}

	// ﾜｰﾆﾝｸﾞ表示制御
	if( lin == LCD_WMSG_LINE ){
		// 表示行がﾜｰﾆﾝｸﾞ表示行の場合、通常表示ﾃﾞｰﾀ保存
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor = DispColor.Color;
			j++;
		}

		if(DspSts != LCD_WMSG_ON){
//			PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
			PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
		}
	}
	if( lin == LCD_WMSG_LINE2 ){
		// 表示行がﾜｰﾆﾝｸﾞ表示行の場合、通常表示ﾃﾞｰﾀ保存
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor2 = DispColor.Color;
			j++;
		}

		if(DspSts2 != LCD_WMSG_ON){
//			PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
			PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
		}
	}
	
// MH810100(S) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
	if(cursor){
		PKTcmd_text_1_backgroundcolor(COLOR_WHITE);
	}

// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない
//	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2020/02/07 #3843 カーソルが表示されない

#endif
// MH810100(E) 
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : grawaku                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : 飾り枠表示                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char startLine	開始行 0-6(7)                  |*/
/*|              : unsigned char endLine	終了行 0-6(7)                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-12-15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
	unsigned short CanDispLine[2][15] = {
	{ LCD_GAIJI_LEFT_UP, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_RIGHT_UP },
	{ LCD_GAIJI_LEFT_DW, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_RIGHT_DW }
	};
void	grawaku( unsigned char startLine,  unsigned char endLine )
{
	unsigned short i;
	unsigned short linL, linR;

	if((startLine > 6) || (endLine > 6)){
		return;
	}
	if(startLine >= endLine){
		return;
	}

	grachr( startLine, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[0] );	//上横線
	grachr( endLine, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[1] );	//下横線

	linL = LCD_GAIJI_LEFT_VERT;
	linR = LCD_GAIJI_RIGHT_VERT;
	for(i=startLine+1; i<endLine; i++){
		grachr( i , 0, 2, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&linL);	//左縦線
		grachr( i , 28, 2, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&linR);	//右縦線
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : numchr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BIG FONT ( 24 * 32 ) DISP ( 1 chacter )                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7                         |*/
/*|              : unsigned short col	colmn = 0-27                       |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned char  dat	data = '0'-'9' or ':'              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	numchr( unsigned short lin,
				unsigned short col,
				unsigned short color,
				unsigned short blink,
				unsigned char  dat )
{
	unsigned short	datsav;
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if(('0' <= dat) && (dat <= '9')){
		datsav = (unsigned short)((0x824f)+(dat-0x30));	// ASCII -> CGC Change
	} else {
		if (dat == ':') {
			datsav = 0x8146;
		} else {
			datsav = 0x8140;
		}
	}
// MH810100(S) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）

	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_BIG_FONT;						// ﾋﾞｯｸﾞﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	// 文字カラー指定
	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
	DispColor.Color = color;							// 色指定
	DispColorDataSnd( &DispColor );

	// ブリンク指定
	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
	DispBlink.Row = (unsigned char)lin;					// 行指定
	DispBlink.Colmun = 0;								// 列指定(設定無効)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
	}
	DispBlinkDataSnd(&DispBlink);

	// 文字列指定
	DispStr.Esc = LCD_ESC_STR;							// 文字列指定
	DispStr.Row = (unsigned char)lin;					// 行指定
	DispStr.Colmun = (unsigned char)col;				// 列指定
	memcpy( DispStr.Msg, &datsav, sizeof(short) );	
	can_snd_data4((void *)&DispStr, 4 + sizeof(short));
// MH810100(S) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）	
#else
// MH810100(S) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）	
	memcpy( DispStr.Msg, &datsav, sizeof(short) );
// MH810100(E) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	// 文字フォント指定
	PKTcmd_text_begin();
	PKTcmd_text_font(LCD_BIG_FONT);		// ビッグフォント

	// 文字カラー指定
	PKTcmd_text_color(color);			// 文字カラー指定

	// ブリンク指定
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)0,(unsigned char)0);				// ブリンク指定
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)1,(unsigned char)blink);				// ブリンク指定
	}
	
	// 文字列指定
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
	memcpy( DispStr.Msg, &datsav, sizeof(short) );
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
	PKTcmd_text_direct((unsigned char)lin,(unsigned char)col, (uchar*)&DispStr.Msg);
	PKTcmd_text_end();
#endif
// MH810100(E) Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : bigcr                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BIG FONT ( 24 * 32 ) DISP                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7(8 line mode)            |*/
/*|              : unsigned short col	colmn = 0-27                       |*/
/*|              : unsigned short cnt	character number                   |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned char * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bigcr(  unsigned short lin,
				unsigned short col,
				unsigned short cnt,
				unsigned short color,
				unsigned short blink,
				const unsigned char *dat )
{
	unsigned char	DispMsg[LCD_CLM];
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;

	memset(&DispMsg, 0, LCD_CLM);
	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_BIG_FONT;						// ﾋﾞｯｸﾞﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	// 文字カラー指定
	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
	DispColor.Color = color;							// 色指定
	DispColorDataSnd( &DispColor );

	// ブリンク指定
	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
	DispBlink.Row = (unsigned char)lin;					// 行指定
	DispBlink.Colmun = 0;								// 列指定(設定無効)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
	}
	DispBlinkDataSnd(&DispBlink);

	// 文字列指定
	DispStr.Esc = LCD_ESC_STR;							// 文字列指定
	DispStr.Row = (unsigned char)lin;					// 行指定
	DispStr.Colmun = (unsigned char)col;				// 列指定
	memcpy( &DispStr.Msg[0], dat, (size_t)cnt );
	can_snd_data4((void *)&DispStr, 4 + cnt);

// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#else
	// 文字フォント指定
	PKTcmd_text_begin();
	PKTcmd_text_font(LCD_BIG_FONT);		// ビッグフォント

	// 文字カラー指定
	PKTcmd_text_color(color);			// 文字カラー指定

	// ブリンク指定
	if(blink == 0) {
		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)0,(unsigned char)0);				// ブリンク指定
	} else {
		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)1,(unsigned char)blink);				// ブリンク指定
	}

	// 文字列指定
	PKTcmd_text_direct((unsigned char)lin,(unsigned char)col, (uchar*)&DispStr.Msg);
	PKTcmd_text_end();
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blink_reg                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Blink area registation                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7                         |*/
/*|              : unsigned short col	colmn = 0-29                       |*/
/*|              : unsigned short cnt	character number = 1-30            |*/
/*|              : unsigned short mod	mode = 0(normal)                   |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	blink_reg(  unsigned short lin,
					unsigned short col,
					unsigned short cnt,
					unsigned short mod,
					unsigned short color,
					const unsigned char *dat )
{

	if( LCD_BLKLG_CNT >= 10 ){
		return;
	}
	// ﾌﾞﾘﾝｸ停止用にﾌﾞﾘﾝｸ開始時のﾃﾞｰﾀを退避
	LCD_Blk_lg[LCD_BLKLG_CNT].lin = lin;						// 指定行
	LCD_Blk_lg[LCD_BLKLG_CNT].col = col;						// 指定ｶﾗﾑ
	LCD_Blk_lg[LCD_BLKLG_CNT].cnt = cnt;						// 文字数
	LCD_Blk_lg[LCD_BLKLG_CNT].mod = mod;						// 反転
	LCD_Blk_lg[LCD_BLKLG_CNT].color = color;					// 文字色
	memcpy( &LCD_Blk_lg[LCD_BLKLG_CNT].dat, dat, (size_t)cnt );	// 表示文字

	LCD_BLKLG_CNT++;

	grachr( lin, col, cnt, mod, color, LCD_BLINK_ON, dat );		// ﾌﾞﾘﾝｸ表示開始
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blink_end (未使用関数)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Blinking end                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	blink_end( void )
{
	unsigned short	i;

	for( i = 0; i < LCD_BLKLG_CNT; i++ ){

		// ﾌﾞﾘﾝｸ表示停止
		grachr( LCD_Blk_lg[i].lin,								// 指定行
				LCD_Blk_lg[i].col,								// 指定ｶﾗﾑ
				LCD_Blk_lg[i].cnt,								// 文字数
				LCD_Blk_lg[i].mod,								// 反転指定
				LCD_Blk_lg[i].color,							// 表示文字色
				LCD_BLINK_OFF,									// ﾌﾞﾘﾝｸ指定:なし
				LCD_Blk_lg[i].dat );							// 表示文字列
	}

	LCD_BLKLG_CNT = 0;
	memset( LCD_Blk_lg, 0, sizeof(t_blk_lg) * LCD_BLK_LG );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lcd_backlight                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LCDC back light on/off                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mode = 0:off/ 1:on                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lcd_backlight( char mode )
{
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	unsigned char	CtlCode;
	unsigned char	LEDPattan;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）

	if( (blightMode == mode) && (blightLevel == backlight) ) {
		return;											// 前回と同じ内容であれば送信しない
	}
	blightMode = mode;
	blightLevel = backlight;

// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	if( mode ) {
		// バックライトON
		CtlCode = CAN_CTRL_BKLT | backlight;
		LEDPattan = 0;
		SD_LCD = 1;
	} else {
		// バックライトOFF
		CtlCode = CAN_CTRL_BKLT;
		LEDPattan = 0;
		SD_LCD = 0;
	}
	can_snd_data3(CtlCode, LEDPattan);					// CANコントロール送信
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#else
	// ON？
	if( mode ){
		PKTcmd_brightness( backlight );
	}else{
		PKTcmd_brightness( 0 );
	}
#endif	
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lcd_contrast                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : コントラスト調整                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : value = 1～16:輝度値，0:バックライトOFF                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lcd_contrast( unsigned char value )
{
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	unsigned char	CtlCode;
	unsigned char	LEDPattan;
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）

	if( value >  LCD_LUMINE_MAX) {
		return;
	}

// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
	if( value != 0) {
		// バックライトON
		CtlCode = CAN_CTRL_BKLT | value;
		LEDPattan = 0;
	} else {
		// バックライトOFF
		CtlCode = CAN_CTRL_BKLT;
		LEDPattan = 0;
	}
	can_snd_data3(CtlCode, LEDPattan);					// CANコントロール送信
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
#else
	PKTcmd_brightness( value ) ;
#endif	
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス（画面表示変更）
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Fun_Dsp                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌｧﾝｸｼｮﾝｷｰ枠表示                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short dat	dsp data = FUNMSG[]                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Fun_Dsp( const unsigned char* dat )
{
// MH810100(S)
#if (!GT4100_NO_LCD)
// MH810100(E)
// MH810100(S) S.Takahashi 2019/12/18
//	unsigned short	lin;
//	t_DispMenu		DispMenu;
	unsigned short	lin = 7;
// MH810100(E) S.Takahashi 2019/12/18
	unsigned char	i;

// MH810100(S) S.Takahashi 2019/12/18
//	memset(&DispMenu, 0, sizeof(t_DispMenu));
// MH810100(E) S.Takahashi 2019/12/18

	// メニュー表示指定
// MH810100(S) S.Takahashi 2019/12/18
//	DispMenu.Esc = LCD_ESC_MENU;						// ﾒﾆｭｰ表示指定
//	DispMenu.Row = 7;									// 行指定
//	DispMenu.Colmun = 0;								// 列指定
//	memcpy( &DispMenu.Msg[0], dat, LCD_CLM );
//	can_snd_data4((void *)&DispMenu, 4 + LCD_CLM);
	PKTcmd_text_begin();	
	PKTcmd_text_menu(lin, 0, dat);
// MH810100(E) S.Takahashi 2019/12/18

	// ﾜｰﾆﾝｸﾞ表示制御
// MH810100(S) S.Takahashi 2019/12/18
//	lin = 7;
// MH810100(E) S.Takahashi 2019/12/18
	if( lin == LCD_WMSG_LINE ){

		// ﾜｰﾆﾝｸﾞ表示行がﾌｧﾝｸｼｮﾝｷｰ表示行(=7)の場合
		for( i=0; i<LCD_CLM; i++ ){
// MH810100(S) S.Takahashi 2019/12/18
//			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecMsg = dat[i];
// MH810100(E) S.Takahashi 2019/12/18
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中の場合
			Lcd_Wmsg_Disp();								// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
		}
	}
// MH810100(S) S.Takahashi 2019/12/18
	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2019/12/18
// MH810100(S)
#else
	unsigned short	lin;
	t_DispMenu		DispMenu;
	unsigned char	i;
	memset( &DispMenu, 0, sizeof(t_DispMenu) );

	DispMenu.Esc = LCD_ESC_MENU;						// ﾒﾆｭｰ表示指定
	DispMenu.Row = 7;									// 行指定
	DispMenu.Colmun = 0;								// 列指定
	memcpy( &DispMenu.Msg[0], dat, LCD_CLM );
	can_snd_data4((void *)&DispMenu, 4 + LCD_CLM);

	lin = 7;
	if( lin == LCD_WMSG_LINE ){

		// ﾜｰﾆﾝｸﾞ表示行がﾌｧﾝｸｼｮﾝｷｰ表示行(=7)の場合
		for( i=0; i<LCD_CLM; i++ ){
			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中の場合
			Lcd_Wmsg_Disp();								// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
		}
	}
#endif
// MH810100(E)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Vram_BkupRestr                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LCD_ADD の 6.or.7Line目を Backup / Restore する          */
/*|                将来全ての行が必要なら２次元配列にするのがベスト         */
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short mode 1:Backup 0:Restore				   |*/
/*|                unsigned short line	 = 6,7                             |*/
/*|                short show_sw : 0:Not 1:Show ON						   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okamoto                                                 |*/
/*| Date         : 2006-09-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Vram_BkupRestr( unsigned short mode ,unsigned short lin , short show_sw )
{
// MH810100(S) K.Onodera 2020/03/12 車番チケットレス(不要処理削除)
//
//	t_DispFont		DispFont;
//	t_DispColor		DispColor;
//	t_DispStr		DispStr;
//	unsigned char	i;
//
//	memset(&DispFont, 0, sizeof(t_DispFont));
//	memset(&DispColor, 0, sizeof(t_DispColor));
//	memset(&DispStr, 0, sizeof(t_DispStr));
//
//	if(mode){												// ﾊﾞｯｸｱｯﾌﾟ
//		switch(lin){
//		case	6:
//			for( i=0; i<LCD_CLM; i++ ){
//				LCD_BACKUP_MSG6[i] = Lcd_Wmsg_Cnt[i].RecMsg2;
//				LCD_BACKUP_COLOR6[i] = Lcd_Wmsg_Cnt[i].RecColor2;
//			}
//			break;
//		case	7:
//			for( i=0; i<LCD_CLM; i++ ){
//				LCD_BACKUP_MSG7[i] = Lcd_Wmsg_Cnt[i].RecMsg;
//				LCD_BACKUP_COLOR7[i] = Lcd_Wmsg_Cnt[i].RecColor;
//			}
//			break;
//		}
//	}else{													// ﾘｽﾄｱ
//		if ( LCD_BACKUP_Flag[lin] ){
//			switch(lin){
//			case	6:
//				for( i=0; i<LCD_CLM; i++ ){
//					Lcd_Wmsg_Cnt[i].RecMsg2 = LCD_BACKUP_MSG6[i];
//					Lcd_Wmsg_Cnt[i].RecColor2 = LCD_BACKUP_COLOR6[i];
//				}
//				for( i=0; i<LCD_CLM; i++ ){
//					if( show_sw == 1 ){							// ﾘｽﾄｱ＋表示
//						// 文字フォント指定
//						DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
//						DispFont.Type = LCD_NORMAL_FONT;					// ﾉｰﾏﾙﾌｫﾝﾄ
//						DispFontDataSnd(&DispFont);
//
//						// 文字カラー指定
//						DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
//						DispColor.Color = LCD_BACKUP_COLOR6[i];				// 色指定
//						DispColorDataSnd( &DispColor );
//
//						// 文字列指定
//						DispStr.Esc = LCD_ESC_STR;							// 文字列指定
//						DispStr.Row = (unsigned char)lin;					// 行指定
//						DispStr.Colmun = i;									// 列指定
//						if(SJIS_Size(&LCD_BACKUP_MSG6[i]) == 1){
//							// 半角
//							DispStr.Msg[0] = LCD_BACKUP_MSG6[i];
//							can_snd_data4((void *)&DispStr, 4 + 1);
//						}
//						else{
//							// 全角
//							DispStr.Msg[0] = LCD_BACKUP_MSG6[i];
//							DispStr.Msg[1] = LCD_BACKUP_MSG6[i+1];
//							can_snd_data4((void *)&DispStr, 4 + 2);
//							i++;
//						}
//					}
//				}
//				break;
//			case	7:
//				for( i=0; i<LCD_CLM; i++ ){
//					Lcd_Wmsg_Cnt[i].RecMsg = LCD_BACKUP_MSG7[i];
//					Lcd_Wmsg_Cnt[i].RecColor = LCD_BACKUP_COLOR7[i];
//				}
//				for( i=0; i<LCD_CLM; i++ ){
//					if( show_sw == 1 ){							// ﾘｽﾄｱ＋表示
//						// 文字フォント指定
//						DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
//						DispFont.Type = LCD_NORMAL_FONT;					// ﾉｰﾏﾙﾌｫﾝﾄ
//						DispFontDataSnd(&DispFont);
//
//						// 文字カラー指定
//						DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
//						DispColor.Color = LCD_BACKUP_COLOR7[i];				// 色指定
//						DispColorDataSnd( &DispColor );
//
//						// 文字列指定
//						DispStr.Esc = LCD_ESC_STR;							// 文字列指定
//						DispStr.Row = (unsigned char)lin;					// 行指定
//						DispStr.Colmun = i;									// 列指定
//						if(SJIS_Size(&LCD_BACKUP_MSG7[i]) == 1){
//							// 半角
//							DispStr.Msg[0] = LCD_BACKUP_MSG7[i];
//							can_snd_data4((void *)&DispStr, 4 + 1);
//						}
//						else{
//							// 全角
//							DispStr.Msg[0] = LCD_BACKUP_MSG7[i];
//							DispStr.Msg[1] = LCD_BACKUP_MSG7[i+1];
//							can_snd_data4((void *)&DispStr, 4 + 2);
//							i++;
//						}
//					}
//				}
//				break;
//			}
//		}
//	}
//
//	LCD_BACKUP_Flag[lin] = (unsigned char)mode;
// MH810100(E) K.Onodera 2020/03/12 車番チケットレス(不要処理削除)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Fun_Dsp                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾌｧﾝｸｼｮﾝｷｰ1枠表示                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short dat	dsp data = FUNMSG[]                |*/
/*|              : unsigned char  num  表示する箇所指定(１～５)            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Funckey_one_Dsp( const unsigned char* dat, unsigned char num )
{
	unsigned short	lin;
	t_DispMenu		DispMenu;
	unsigned char	i;

	
	memset(&DispMenu, 0, sizeof(t_DispMenu));

	// メニュー表示指定
	DispMenu.Esc = LCD_ESC_MENU;						// ﾒﾆｭｰ表示指定
	DispMenu.Row = 7;									// 行指定
	DispMenu.Colmun = (num-1)*6;								// 列指定
	memcpy( &DispMenu.Msg[0], &dat[0], 6 );
	can_snd_data4((void *)&DispMenu, 4 + 6);

	// ﾜｰﾆﾝｸﾞ表示制御
	lin = 7;
	if( lin == LCD_WMSG_LINE ){

		// ﾜｰﾆﾝｸﾞ表示行がﾌｧﾝｸｼｮﾝｷｰ表示行(=7)の場合
		for( i=0; i<LCD_CLM; i++ ){
			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中の場合
			Lcd_Wmsg_Disp();								// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
		}
	}
	return;
}
void	grawaku2( unsigned short start_line )
{
	unsigned short lin;

	grachr( start_line , 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[0] );	//上横線
	grachr( start_line+3 , 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[1] );	//下横線

	lin = LCD_GAIJI_LEFT_VERT;
	grachr( start_line+1 , 0, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//左縦線
	grachr( start_line+2 , 0, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//左縦線
	lin = LCD_GAIJI_RIGHT_VERT;
	grachr( start_line+1 , 28, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//右縦線
	grachr( start_line+2 , 28, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//右縦線

	return;
}
