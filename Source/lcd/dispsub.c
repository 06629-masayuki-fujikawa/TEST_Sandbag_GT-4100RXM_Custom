/*[]----------------------------------------------------------------------[]*/
/*| Display function                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2002-03-29                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update      : 新精算機対応 2012-12-15 k.totsuka                        |*/
/*| Date        :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	"system.h"
#include	<string.h>
#include	<stdio.h>
#include	"lcd_def.h"
#include	"iodefine.h"
#include	"can_def.h"

t_DispColorFontNow	Disp_color_font_now;		//現在設定中のカラーとフォント種別を保持しておく
t_DispBlinkNow 		Disp_blink_now[8];			//現在設定中のブリンク情報を保持しておく

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_ON( uchar mod, uchar *msg )                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示開始                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ushort mod  = 表示方法（0:通常 / 1:反転）               |*/
/*|                ushort color= 表示色                                    |*/
/*|                ushort blink= ﾌﾞﾘﾝｸ表示 0(none) - blink time(10ms)      |*/
/*|                uchar  *msg = 表示文字列ﾎﾟｲﾝﾀ（文字列は30文字固定）     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_ON( unsigned short mod, unsigned short color, unsigned short blink, const unsigned char *msg )
{
// MH810100(S) K.Onodera 2020/01/06 車番チケットレス（画面表示対応）
//	t_DispFont		DispFont;
//	t_DispColor		DispColor;
//	t_DispBlink		DispBlink;
//	t_DispStr		DispStr;
//	unsigned char	i;
//
//	memset(&DispFont, 0, sizeof(t_DispFont));
//	memset(&DispColor, 0, sizeof(t_DispColor));
//	memset(&DispBlink, 0, sizeof(t_DispBlink));
//	memset(&DispStr, 0, sizeof(t_DispStr));
//
//	DspSts = LCD_WMSG_ON;								// 表示状態ＯＮ
//
//	// 文字フォント指定
//	DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
//	DispFont.Type = LCD_NORMAL_FONT;					// ﾉｰﾏﾙﾌｫﾝﾄ
//	DispFontDataSnd(&DispFont);
//
//	// 文字カラー指定
//	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
//	DispColor.Color = color;							// 色指定
//	DispColorDataSnd( &DispColor );
//
//	// ブリンク指定
//	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
//	DispBlink.Row = LCD_WMSG_LINE;						// 行指定
//	DispBlink.Colmun = 0;								// 列指定(設定無効)
//	if(blink == 0) {
//		DispBlink.Mode = 0;								// ﾌﾞﾘﾝｸ解除
//		DispBlink.Interval = 0;							// ﾌﾞﾘﾝｸ間隔
//	} else {
//		DispBlink.Mode = 1;								// ﾌﾞﾘﾝｸ開始
//		DispBlink.Interval = (unsigned char)blink;		// ﾌﾞﾘﾝｸ間隔
//	}
//	DispBlinkDataSnd(&DispBlink);
//
//	// 文字列指定
//	DispStr.Esc = LCD_ESC_STR;							// 文字列指定
//	DispStr.Row = LCD_WMSG_LINE;						// 行指定
//	DispStr.Colmun = 0;									// 列指定
//	memcpy( &DispStr.Msg[0], msg, LCD_CLM );
//	can_snd_data4((char *)&DispStr, 4 + LCD_CLM);
//
//	for( i=0; i<LCD_CLM; i++ ){
//		Lcd_Wmsg_Cnt[i].WarMsg = DispStr.Msg[i];
//		Lcd_Wmsg_Cnt[i].WarColor = DispColor.Color;		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示色保存
//	}
// MH810100(E) K.Onodera 2020/01/06 車番チケットレス（画面表示対応）
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_OFF( void )                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示終了                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_OFF( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if( DspSts == LCD_WMSG_ON ){

		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中の場合
		DspSts = LCD_WMSG_OFF;										// 表示状態ＯＦＦ

		// 文字フォント指定
		DispFont.Esc = LCD_ESC_FONTTYPE;							// 文字ﾌｫﾝﾄ指定
		DispFont.Type = LCD_NORMAL_FONT;							// ﾉｰﾏﾙﾌｫﾝﾄ
		DispFontDataSnd(&DispFont);

		for( i=0; i<LCD_CLM; i++ ){
			// 文字カラー指定
			DispColor.Esc = LCD_ESC_FONTCOLR;						// 文字ｶﾗｰ指定
			DispColor.Color = Lcd_Wmsg_Cnt[i].RecColor;				// 色指定
			DispColorDataSnd( &DispColor );

			// 文字列指定
			DispStr.Esc = LCD_ESC_STR;								// 文字列指定
			DispStr.Row = LCD_WMSG_LINE;							// 行指定
			DispStr.Colmun = i;										// 列指定
			if(SJIS_Size(&Lcd_Wmsg_Cnt[i].RecMsg) == 1){
				// 半角
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg;
				can_snd_data4((void *)&DispStr, 4 + 1);
			}
			else{
				// 全角
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg;
				DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].RecMsg;
				can_snd_data4((void *)&DispStr, 4 + 2);
				i++;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_Wmsg_Disp( void )                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_Wmsg_Disp( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;							// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_NORMAL_FONT;							// ﾉｰﾏﾙﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	for( i=0; i<LCD_CLM; i++ ){
		// 文字カラー指定
		DispColor.Esc = LCD_ESC_FONTCOLR;							// 文字ｶﾗｰ指定
		DispColor.Color = Lcd_Wmsg_Cnt[i].WarColor;					// 色指定
		DispColorDataSnd( &DispColor );

		// 文字列指定
		DispStr.Esc = LCD_ESC_STR;									// 文字列指定
		DispStr.Row = LCD_WMSG_LINE;								// 行指定
		DispStr.Colmun = i;											// 列指定
		if(SJIS_Size(&Lcd_Wmsg_Cnt[i].WarMsg) == 1){
			// 半角
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			can_snd_data4((void *)&DispStr, 4 + 1);
		}
		else{
			// 全角
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].WarMsg;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			can_snd_data4((void *)&DispStr, 4 + 2);
			i++;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_ON( uchar mod, uchar *msg )                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示開始                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ushort mod  = 表示方法（0:通常 / 1:反転）               |*/
/*|                ushort color= 表示色                                    |*/
/*|                ushort blink= ﾌﾞﾘﾝｸ表示 0(none) - blink time(10ms)      |*/
/*|                uchar  *msg = 表示文字列ﾎﾟｲﾝﾀ（文字列は30文字固定）     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_ON2( unsigned short mod, unsigned short color, unsigned short blink, const unsigned char *msg )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

	DspSts2 = LCD_WMSG_ON;								// 表示状態ＯＮ

	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;					// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_NORMAL_FONT;					// ﾉｰﾏﾙﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	// 文字カラー指定
	DispColor.Esc = LCD_ESC_FONTCOLR;					// 文字ｶﾗｰ指定
	DispColor.Color = color;							// 色指定
	DispColorDataSnd( &DispColor );

	// ブリンク指定
	DispBlink.Esc = LCD_ESC_BLINK;						// ﾌﾞﾘﾝｸ指定
	DispBlink.Row = LCD_WMSG_LINE;						// 行指定
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
	DispStr.Row = LCD_WMSG_LINE2;						// 行指定
	DispStr.Colmun = 0;									// 列指定
	memcpy( &DispStr.Msg[0], msg, LCD_CLM );
	can_snd_data4((void *)&DispStr, 4 + LCD_CLM);

	for( i=0; i<LCD_CLM; i++ ){
		Lcd_Wmsg_Cnt[i].WarMsg2 = DispStr.Msg[i];		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示ﾃﾞｰﾀ保存
		Lcd_Wmsg_Cnt[i].WarColor2 = DispColor.Color;	// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示色保存
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_OFF( void )                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示終了                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_OFF2( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if( DspSts2 == LCD_WMSG_ON ){
		// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示中の場合
		DspSts2 = LCD_WMSG_OFF;										// 表示状態ＯＦＦ

		// 文字フォント指定
		DispFont.Esc = LCD_ESC_FONTTYPE;							// 文字ﾌｫﾝﾄ指定
		DispFont.Type = LCD_NORMAL_FONT;							// ﾉｰﾏﾙﾌｫﾝﾄ
		DispFontDataSnd(&DispFont);

		for( i=0; i<LCD_CLM; i++ ){
			// 文字カラー指定
			DispColor.Esc = LCD_ESC_FONTCOLR;						// 文字ｶﾗｰ指定
			DispColor.Color = Lcd_Wmsg_Cnt[i].RecColor2;			// 色指定
			DispColorDataSnd( &DispColor );

			// 文字列指定
			DispStr.Esc = LCD_ESC_STR;								// 文字列指定
			DispStr.Row = LCD_WMSG_LINE2;							// 行指定
			DispStr.Colmun = i;										// 列指定
			if(SJIS_Size(&Lcd_Wmsg_Cnt[i].RecMsg2) == 1){
				// 半角
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg2;				// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
				can_snd_data4((void *)&DispStr, 4 + 1);
			}
			else{
				// 全角
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg2;				// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
				DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].RecMsg2;				// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
				can_snd_data4((void *)&DispStr, 4 + 2);
				i++;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_Wmsg_Disp2( void )                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_Wmsg_Disp2( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));


	// 文字フォント指定
	DispFont.Esc = LCD_ESC_FONTTYPE;							// 文字ﾌｫﾝﾄ指定
	DispFont.Type = LCD_NORMAL_FONT;							// ﾉｰﾏﾙﾌｫﾝﾄ
	DispFontDataSnd(&DispFont);

	for( i=0; i<LCD_CLM; i++ ){
		// 文字カラー指定
		DispColor.Esc = LCD_ESC_FONTCOLR;							// 文字ｶﾗｰ指定
		DispColor.Color = Lcd_Wmsg_Cnt[i].WarColor2;				// 色指定
		DispColorDataSnd( &DispColor );

		// 文字列指定
		DispStr.Esc = LCD_ESC_STR;									// 文字列指定
		DispStr.Row = LCD_WMSG_LINE2;								// 行指定
		DispStr.Colmun = i;											// 列指定
		if(SJIS_Size(&Lcd_Wmsg_Cnt[i].WarMsg2) == 1){
			// 半角
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg2;					// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			can_snd_data4((void *)&DispStr, 4 + 1);
		}
		else{
			// 全角
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg2;					// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].WarMsg2;					// 通常表示ﾃﾞｰﾀﾘｶﾊﾞｰ
			can_snd_data4((void *)&DispStr, 4 + 2);
			i++;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispColorDataSnd                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :   カラー情報設定処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispColor*	DispColor			カラー情報             |*/
/*| RETURN VALUE : ０：正常終了：　１：異常終了（前回設定値と同じ）        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                               |*/
/*| Date         : 2012-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispColorDataSnd( t_DispColor*	DispColor )
{
	//前回設定値と異なる場合
	if(Disp_color_font_now.Color !=  DispColor->Color)
	{
		Disp_color_font_now.Color =  DispColor->Color;
		can_snd_data4((void *)DispColor, sizeof(t_DispColor));
		return 0;				//正常終了
	}
	return	1;					//異常終了
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispFontDataSnd                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :  フォント情報設定処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispFont*	DispFont			フォント情報           |*/
/*| RETURN VALUE : ０：正常終了：　１：異常終了（前回設定値と同じ）        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                                |*/
/*| Date         : 2006-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispFontDataSnd( t_DispFont*	DispFont )
{
	//前回設定値と異なる場合
	if( Disp_color_font_now.Type !=  DispFont->Type)
	{
		Disp_color_font_now.Type =  DispFont->Type;
		can_snd_data4((void *)DispFont, sizeof(t_DispFont));
		return 0;				//正常終了
	}
	return	1;					//異常終了
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispBlinkDataSnd                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :  ブリンク情報設定処理                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispBlink*	DispBlink			ブリンク情報           |*/
/*| RETURN VALUE : ０：正常終了：　１：異常終了（前回設定値と同じ）        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                                |*/
/*| Date         : 2006-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispBlinkDataSnd( t_DispBlink*	DispBlink )
{
	//前回設定値と異なる場合
	if( ( Disp_blink_now[DispBlink->Row].Colmun !=  DispBlink->Colmun ) ||
		( Disp_blink_now[DispBlink->Row].Mode !=  DispBlink->Mode ) ||
		( Disp_blink_now[DispBlink->Row].Interval !=  DispBlink->Interval ) )
	{
		Disp_blink_now[DispBlink->Row].Colmun =  DispBlink->Colmun;
		Disp_blink_now[DispBlink->Row].Mode =  DispBlink->Mode;
		Disp_blink_now[DispBlink->Row].Interval =  DispBlink->Interval;
		can_snd_data4((void *)DispBlink, sizeof(t_DispBlink));
		return 0;				//正常終了
	}
	// 前回と同じでもブリンク開始要求であればブリンク指定を送信
	if( Disp_blink_now[DispBlink->Row].Mode == 1 ){
		can_snd_data4((void *)DispBlink, sizeof(t_DispBlink));
		return 0;				//正常終了
	}
	return	1;					//異常終了
}

