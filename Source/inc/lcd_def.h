/*[]----------------------------------------------------------------------[]*/
/*| headder file for LCDcontrol                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2002.08.XX                                               |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _LCD_DEF_H_
#define _LCD_DEF_H_

#define LCD_CLM		0x1E

#define	RGB16(r, g, b)			((unsigned short)((((r)>>3) << 11) | (((g)>>2) << 5) | (((b)>>3) << 0)))
#define	COLOR_BLACK				((unsigned short)RGB16(  0,  0,  0))	// ブラック
#define	COLOR_RED				((unsigned short)RGB16(255,  0,  0))	// レッド
#define	COLOR_GREEN				((unsigned short)RGB16(  0,128,  0))	// グリーン
#define	COLOR_BLUE				((unsigned short)RGB16(  0,  0,255))	// ブルー
#define	COLOR_MIDNIGHTBLUE		((unsigned short)RGB16( 25, 25,112))	// ミッドナイトブルー
#define	COLOR_PALETURQUOISE		((unsigned short)RGB16(175,238,238))	// パールターコイズ
#define	COLOR_LIGHTSEAGREEN		((unsigned short)RGB16( 32,178,170))	// ライトシーグリーン
#define	COLOR_MEDIUMBLUE		((unsigned short)RGB16(  0,  0,205))	// ミディアムブルー
#define	COLOR_DODGERBLUE		((unsigned short)RGB16( 30,144,255))	// ドジャーブルー
#define	COLOR_MEDIUMSEAGREEN	((unsigned short)RGB16( 60,179,113))	// ミディアムシーグリーン
#define	COLOR_DARKGREEN			((unsigned short)RGB16(  0,100,  0))	// ダークグリーン
#define	COLOR_INDIGO			((unsigned short)RGB16( 75,  0,130))	// インディゴ
#define	COLOR_DARKSLATEBLUE		((unsigned short)RGB16( 72, 61,139))	// ダークスレートブルー
#define	COLOR_DARKGRAY			((unsigned short)RGB16(169,169,169))	// ダークグレー
#define	COLOR_DIMGRAY			((unsigned short)RGB16(105,105,105))	// ディムグレー
#define	COLOR_YELLOW			((unsigned short)RGB16(255,255,  0))	// イエロー
#define	COLOR_GOLD				((unsigned short)RGB16(255,215,  0))	// ゴールド
#define	COLOR_DARKORANGE		((unsigned short)RGB16(255,140,  0))	// ダークオレンジ
#define	COLOR_DARKGOLDENROD		((unsigned short)RGB16(184,134, 11))	// ダークゴールデンロッド
#define	COLOR_TOMATO			((unsigned short)RGB16(255, 99, 71))	// トマト
#define	COLOR_SIENNA			((unsigned short)RGB16(160, 82, 45))	// シエンナ
#define	COLOR_LIGHTBLUE			((unsigned short)RGB16(173,216,230))	// ライトブルー
#define	COLOR_KHAKI				((unsigned short)RGB16(240,230,140))	// カーキ
#define	COLOR_BURLYWOOD			((unsigned short)RGB16(222,184,135))	// バーリーウッド
#define	COLOR_FIREBRICK			((unsigned short)RGB16(178, 34, 34))	// ファイヤーブレイク
#define	COLOR_ROSYBROWN			((unsigned short)RGB16(188,143,143))	// ローズィブラウン
#define	COLOR_DARKMAGENTA		((unsigned short)RGB16(139,  0,139))	// ダークマゼンタ
#define	COLOR_PALEVIOLETRED		((unsigned short)RGB16(219,112,147))	// ペールバイオレットレッド
#define	COLOR_CORNSILK			((unsigned short)RGB16(255,248,220))	// コーンシルク
#define	COLOR_THISTLE			((unsigned short)RGB16(216,191,216))	// シスル
#define	COLOR_CRIMSON			((unsigned short)RGB16(220, 20, 60))	// クリムソン
#define	COLOR_HOTPINK			((unsigned short)RGB16(255,105,180))	// ホットピンク
#define	COLOR_WHITE				((unsigned short)RGB16(255,255,255))	// ホワイト
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
//#define	COLOR_F1BLUE			((unsigned short)RGB16( 36,173,226))	// F1(DIC-2177)
#define	COLOR_F1BLUE			((unsigned short)RGB16( 85,255,255))	// F1
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
#define	COLOR_F2GREEN			((unsigned short)RGB16(138,206, 54))	// F2(DIC-2544)
#define	COLOR_F3YELLOW			((unsigned short)RGB16(255,186,  0))	// F3(DIC-86)
#define	COLOR_F4PINK			((unsigned short)RGB16(245,100,143))	// F4(DIC-50)
#define	COLOR_F5PURPLE			((unsigned short)RGB16(188,183,217))	// F5(DIC-45)

/* 反転指定 */
#define LCD_REVERS_OFF		0					/* 通常表示					*/
#define LCD_REVERS_ON		1					/* 反転表示					*/

#define LCD_CURSOR_COLOR	COLOR_PALEVIOLETRED	/* カーソル色(空白部分反転用) */

/* 点滅間隔(10msec単位) */
#define LCD_BLINK_OFF		0x00				/* 点滅なし					*/
#define LCD_BLINK_ON		0x64				/* 1sec						*/

/* ﾌｫﾝﾄ指定 */
#define LCD_NORMAL_FONT		0					/* ﾉｰﾏﾙﾌｫﾝﾄ					*/
#define LCD_BIG_FONT		1					/* ﾋﾞｯｸﾞﾌｫﾝﾄ				*/

/* ﾃﾞｨｽﾌﾟﾚｲ用ｴｽｹｰﾌﾟｼｰｹﾝｽ */
#define LCD_ESC_FONTTYPE	0x1b25				/* 文字ﾌｫﾝﾄ指定				*/
#define LCD_ESC_FONTCOLR	0x1b27				/* 文字色指定				*/
#define LCD_ESC_BLINK		0x1b28				/* 文字列の点滅指定			*/
#define LCD_ESC_CLEAR		0x1b2b				/* ｸﾘｱ指定					*/
#define LCD_ESC_MENU		0x1b2c				/* ﾒﾆｭｰ表示指定				*/
#define LCD_ESC_STR			0x1b2d				/* 文字列指定				*/
#define LCD_ESC_BKCOLOR		0x1b2e				/* 背景色指定				*/

// 外字定義
#define LCD_GAIJI_LEFT_VERT		0x853f				/* 飾り枠(左縦線)			*/
#define LCD_GAIJI_LEFT_UP		0x8540				/* 飾り枠(左上線)			*/
#define LCD_GAIJI_RIGHT_UP		0x8541				/* 飾り枠(右上線)			*/
#define LCD_GAIJI_HORIZON		0x8542				/* 飾り枠(水平線)			*/
#define LCD_GAIJI_LEFT_DW		0x8543				/* 飾り枠(左下線)			*/
#define LCD_GAIJI_RIGHT_DW		0x8544				/* 飾り枠(右下線)			*/
#define LCD_GAIJI_RIGHT_VERT	0x8545				/* 飾り枠(右縦線)			*/
#define LCD_GAIJI_ARROW			0x8546				/* 右矢印					*/
#define LCD_GAIJI_F1_LH			0x8547				/* F1(左半分)				*/
#define LCD_GAIJI_F1_RH			0x8548				/* F1(右半分)				*/
#define LCD_GAIJI_F2A_LH		0x8549				/* F2A(左半分)				*/
#define LCD_GAIJI_F2A_RH		0x854A				/* F2A(右半分)				*/
#define LCD_GAIJI_F2R_LH		0x854B				/* F2R(左半分)				*/
#define LCD_GAIJI_F2R_RH		0x854C				/* F2R(右半分)				*/
#define LCD_GAIJI_F3_LH			0x854D				/* F3(左半分)				*/
#define LCD_GAIJI_F3_RH			0x854E				/* F3(右半分)				*/
#define LCD_GAIJI_F4_LH			0x854F				/* F4(左半分)				*/
#define LCD_GAIJI_F4_RH			0x8550				/* F4(右半分)				*/
#define LCD_GAIJI_F5_LH			0x8551				/* F5(左半分)				*/
#define LCD_GAIJI_F5_RH			0x8552				/* F5(右半分)				*/
#define LCD_GAIJI_NONE			0x0000

#define	LCD_LUMINE_DEF		11					/* 輝度デフォルト値			*/
#define	LCD_LUMINE_MAX		15					/* 最大輝度値				*/

// コントロールコード
#define CAN_CTRL_NONE	0x00		// 未定義
#define CAN_CTRL_PI1	0x10		// ブザー音（ピッ♪）
#define CAN_CTRL_PI2	0x20		// ブザー音（ピピッ♪）
#define CAN_CTRL_PI3	0x40		// ブザー音（ピピピッ♪）
#define CAN_CTRL_BKLT	0x80		// バックライト

// コントロール出力
#define CAN_LED_TRAY	0x80		// 釣銭取出口ガイドLED
#define CAN_LED_TKEY	0x40		// テンキーガイドLED(予約)
#define CAN_LED_BTN1	0x20		// 照光式押しボタン1(予約)
#define CAN_LED_BTN2	0x10		// 照光式押しボタン2(予約)

#pragma	pack
// コントロール制御用構造体
typedef struct {
	unsigned char	CtlCode;		// コントロールコード
	unsigned char	LEDPattan;		// LED出力信号
} t_CanCtl;


// 文字フォント指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned char	Type;			// フォント種別(0=ノーマル 1=ビッグ)
} t_DispFont;

// 文字フォントカラー指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned short	Color;			// RGB 0000h～FFFFh(R=5bit G=6bit B=5bit)
} t_DispColor;

// 文字列ブリンク指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned char	Row;			// 行(0-7)
	unsigned char	Colmun;			// 列(設定無効)
	unsigned char	Mode;			// 開始／解除(0=解除 1=開始)
	unsigned char	Interval;		// 点滅間隔(0～255×10msec)
} t_DispBlink;

// クリア指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned char	Row;			// 行(0-7)
	unsigned char	Colmun;			// 列(設定無効)
	unsigned char	Scope;			// クリア範囲(0=1行クリア 1=全画面クリア)
} t_DispClr;

// メニュー表示指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned char	Row;			// 行(0-7)
	unsigned char	Colmun;			// 列(0-29)
	unsigned char	Msg[LCD_CLM];	// 文字列
} t_DispMenu;

// 文字列指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned char	Row;			// 行(0-7)
	unsigned char	Colmun;			// 列(0-29)
	unsigned char	Msg[LCD_CLM];	// 文字列
} t_DispStr;

// カラーパレット指定用構造体
typedef struct {
	unsigned short	Esc;			// エスケープシーケンス
	unsigned short	Rgb;			// カラーパレット(0=デフォルト)
} t_BackColor;
// 時計表示指定用構造体
#pragma unpack


extern	uchar	backlight;			// LCDﾊﾞｯｸﾗｲﾄ輝度ﾃﾞｰﾀ
extern	uchar	blightLevel;		// 前回のLCDﾊﾞｯｸﾗｲﾄ輝度
extern	uchar	blightMode;			// 前回のLCDﾊﾞｯｸﾗｲﾄON/OFF情報


/*** Timer request area ***/
typedef	struct {
	unsigned long	ReqValue;				/* Timer request value (x10ms) : ex) 100=1s */
	unsigned long	StartTime;				/* Timer start time (LifeTime) */
} t_LCD_Timer;

#define	LCD_TIM1_VALUE	(unsigned long)80L	/* 800 msec */

/*** Blink data          ***/
typedef struct {
	unsigned short	lin;
	unsigned short	col;
	unsigned short	cnt;
	unsigned short	mod;
	unsigned short	color;
	unsigned char	dat[LCD_CLM];
} t_blk_lg;

#define LCD_BLK_LG				10


/*** Retry counter         ***/
#define LCD_NGSTA_RTYMAX	3

/*** Status check mode         ***/
#define LCD_STA01_CMD	0
#define LCD_STA2_ADR	2
#define LCD_STA3_ADW	3
#define LCD_STA5_ABL	5

typedef struct {
	unsigned short	Color;			// RGB 0000h～FFFFh(R=5bit G=6bit B=5bit)
	unsigned char	Type;			// フォント種別(0=ノーマル 1=ビッグ)
}t_DispColorFontNow;
typedef struct {
	unsigned char	Colmun;			// 列(設定無効)
	unsigned char	Mode;			// 開始／解除(0=解除 1=開始)
	unsigned char	Interval;		// 点滅間隔(0～255×10msec)
}t_DispBlinkNow;

extern t_DispColorFontNow	Disp_color_font_now;		//現在設定中のカラーとフォント種別を保持しておく
extern t_DispBlinkNow 		Disp_blink_now[8];			//現在設定中のブリンク情報を保持しておく

/*** dispinit.c          ***/
extern	char			dispinit( void );
extern	void			dispmod( unsigned short mod );
extern	void			dispclr( void );
extern	void			displclr( unsigned short line );
extern	void			dispmlclr( unsigned short from, unsigned short to );
extern	void			grachr( unsigned short lin,unsigned short col,unsigned short cnt,unsigned short mod, unsigned short color, unsigned short blink, const unsigned char * data );
extern	void			grawaku( unsigned char startLine,  unsigned char endLine );
extern	void			numchr( unsigned short lin, unsigned short col, unsigned short color, unsigned short blink, unsigned char  dat );
extern	void			bigcr( unsigned short lin, unsigned short col, unsigned short cnt, unsigned short color, unsigned short blink, const unsigned char *dat );
extern	void			blink_reg( unsigned short lin, unsigned short col, unsigned short cnt, unsigned short mod, unsigned short color, const unsigned char *dat );
extern	void			blink_end( void );
extern	void			Fun_Dsp( const unsigned char *dat );
extern	void			lcd_backlight( char mode );
extern	void			lcd_contrast( unsigned char value );
extern	void			Vram_BkupRestr( unsigned short mode ,unsigned short lin, short show_sw );


/*** dispsub.c          ***/
extern	void			Lcd_WmsgDisp_ON( unsigned short mod, unsigned short color, unsigned short blink, const uchar *msg );
																	// ﾜｰﾆﾝｸﾞ表示開始
extern	void			Lcd_WmsgDisp_OFF( void );					// ﾜｰﾆﾝｸﾞ表示終了
extern	void			Lcd_Wmsg_Disp( void );						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示

extern	void			Lcd_WmsgDisp_ON2( unsigned short mod, unsigned short color, unsigned short blink, const uchar *msg );
																	// ﾜｰﾆﾝｸﾞ表示開始
extern	void			Lcd_WmsgDisp_OFF2( void );					// ﾜｰﾆﾝｸﾞ表示終了
extern	void			Lcd_Wmsg_Disp2( void );						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示
extern	unsigned char	DispColorDataSnd( t_DispColor*	DispColor ); 	// カラー情報設定処理
extern	unsigned char	DispFontDataSnd( t_DispFont*	DispFont );		// フォント情報設定処理
extern	unsigned char	DispBlinkDataSnd( t_DispBlink*	DispBlink );	// ブリンク情報設定処理
extern	void	Funckey_one_Dsp( const unsigned char* dat, unsigned char num );
extern	void	grawaku2( unsigned short start_line );

/*** lcdctrl.c          ***/
extern	void	dspclk(ushort lin, ushort color);

//==============================================
// LCDﾜｰﾆﾝｸﾞ表示制御
//==============================================
typedef struct
{
//	unsigned short	DspSts;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
	unsigned char	WarMsg;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目表示ﾃﾞｰﾀ					(表示文字列ﾃﾞｰﾀ)
	unsigned short	WarColor;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目表示ﾃﾞｰﾀ					(表示色)
	unsigned char	RecMsg;					// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目OFF時にﾘｶﾊﾞｰする表示ﾃﾞｰﾀ	(表示文字列ﾃﾞｰﾀ)
	unsigned short	RecColor;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目OFF時にﾘｶﾊﾞｰする表示ﾃﾞｰﾀ	(表示色)
//	unsigned short	DspSts2;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
	unsigned char	WarMsg2;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目表示ﾃﾞｰﾀ					(表示文字列ﾃﾞｰﾀ)
	unsigned short	WarColor2;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目表示ﾃﾞｰﾀ					(表示色)
	unsigned char	RecMsg2;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目OFF時にﾘｶﾊﾞｰする表示ﾃﾞｰﾀ	(表示文字列ﾃﾞｰﾀ)
	unsigned short	RecColor2;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目OFF時にﾘｶﾊﾞｰする表示ﾃﾞｰﾀ	(表示色)
} T_WMSG_CNT;

extern	unsigned short	DspSts;				// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ7行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
extern	unsigned short	DspSts2;			// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ6行目表示状態	( LCD_WMSG_ON=表示中 / LCD_WMSG_OFF=未表示 )
extern	T_WMSG_CNT	Lcd_Wmsg_Cnt[LCD_CLM];	// ﾜｰﾆﾝｸﾞ表示制御ｴﾘｱ

#define		LCD_WMSG_ON		1		// ﾜｰﾆﾝｸﾞ表示あり
#define		LCD_WMSG_OFF	0		// ﾜｰﾆﾝｸﾞ表示なし
#define		LCD_WMSG_LINE	7		// ﾜｰﾆﾝｸﾞ表示行（7行目）
#define		LCD_WMSG_LINE2	6		// ﾜｰﾆﾝｸﾞ表示行（6行目）

#endif	// _LCD_DEF_H_
