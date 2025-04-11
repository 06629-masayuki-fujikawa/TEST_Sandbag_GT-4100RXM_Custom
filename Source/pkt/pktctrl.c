// MH810100(S) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
//[]----------------------------------------------------------------------[]
///	@brief			パケット通信関連制御
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pktctrl.c
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"ope_def.h"
#include	"rtc_readwrite.h"
#include	"pktctrl.h"
#include	"pkt_buffer.h"

//--------------------------------------------------
//		定義
//--------------------------------------------------
/**
 *	テキストコマンド管理
 */
typedef struct {
	BOOL	is_begun;		///< テキスト開始関数を呼び出し済みか（このフラグがTRUEでない場合はコントロールコード処理を行わない）
	ushort	bytes;			///< 文字列データを何バイト格納したか
} packet_text_work_t;

//--------------------------------------------------
//		変数
//--------------------------------------------------
static lcdbm_cmd_work_t		SendCmdDt;		///< コマンド作成用ワーク
static lcdbm_cmd_text_t		text_command;	///< テキスト表示コマンド作成用のワーク
static packet_text_work_t	text_work;		///< テキスト表示コマンド管理用

//--------------------------------------------------
//		外部関数参照
//--------------------------------------------------
extern	void	ClkrecUpdateFromRTC( struct	RTC_rec *Time );
extern	uchar	PktResetReqFlgRead( void );

//--------------------------------------------------
//		ステータス要求コマンド
//--------------------------------------------------
/**
 *	基本設定要求/リセット要求
 *
 *	@retval	FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval	TRUE	パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 */
BOOL PKTcmd_request_config( void )
{
	lcdbm_cmd_config_t	*dst = &SendCmdDt.config;

	if( PktResetReqFlgRead() ){	// リセット要求あり
		return ( PKTcmd_notice_ope( LCDBM_OPCD_RST_REQ, 0 ) );
	}else{
		// 初期化
		memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_config_t) );

		// 共通部
		dst->command.length = sizeof(lcdbm_cmd_config_t) - sizeof(SendCmdDt.command.length);
		dst->command.id = LCDBM_CMD_ID_STATUS;
		dst->command.subid = LCDBM_CMD_SUBID_CONFIG;

		// データ
		dst->rsw = read_rotsw();	// 現在のRSW状態をセットする

		// プログラムバージョン
		memcpy( dst->ver, VERSNO.ver_part, 8 );	// RXMプログラムバージョン格納(左詰め)
// MH810100(S) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
		dst->opemod = OPECTL.Ope_mod;
// MH810100(E) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)

		return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_config_t) ) );
	}
}

//--------------------------------------------------
//		機器設定コマンド
//--------------------------------------------------
/**
 *	時計設定
 *
 *	@retval	FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval	TRUE	パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 */
BOOL PKTcmd_clock( uchar type )			// type = 0 (手動での時計変更),type = 1(SNTPからの時計変更) 
{
	lcdbm_cmd_clock_t	*dst = &SendCmdDt.clock;

	// 最新のRTC時刻を取得してCLK_REC更新
	RTC_ClkDataReadReq( RTC_READ_NON_QUE );	// RTC時刻読み出し(OPEへの更新要求なし)
	ClkrecUpdateFromRTC( &RTC_CLOCK );		// CLK_REC更新

	// 初期化
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_clock_t) );
	dst->command.length = sizeof(lcdbm_cmd_clock_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_SETUP;
	dst->command.subid = LCDBM_CMD_SUBID_CLOCK;

	// データ
	if( type == 1 ){			// SNTPからの時計変更
		dst->year	= 0;	// 年
		dst->month	= 0;	// 月
		dst->day	= 0;	// 日
		dst->hour	= 0;	// 時
		dst->minute	= 0;	// 分
		dst->second	= 0;	// 秒
	}else{						// 手動での時計変更
	// データ
		dst->year	= CLK_REC.year;	// 年
		dst->month	= CLK_REC.mont;	// 月
		dst->day	= CLK_REC.date;	// 日
		dst->hour	= CLK_REC.hour;	// 時
		dst->minute	= CLK_REC.minu;	// 分
		dst->second	= CLK_REC.seco;	// 秒
	}

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_clock_t) ) );
}

/**
 *	LCD輝度
 *
 *	@param[in]	brightness	輝度値（0～15）
 *	@retval		FALSE		パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE		パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 */
BOOL PKTcmd_brightness( ushort brightness )
{
	BOOL ret = TRUE;
	lcdbm_cmd_brightness_t	*dst = &SendCmdDt.brightness;

	// 初期化
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_brightness_t) );
	dst->command.length = sizeof(lcdbm_cmd_brightness_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_SETUP;
	dst->command.subid = LCDBM_CMD_SUBID_BRIGHTNESS;

	// データ
	if ( LCDBM_LCD_BRIGHTNESS_MAX < brightness ) {
		brightness = LCDBM_LCD_BRIGHTNESS_MAX;
	}
	dst->brightness = brightness;

	ret = PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_brightness_t) );

	return ret;
	
}

//--------------------------------------------------
//		テキスト系コマンド
//	（１コマンド内に複数コントロールコードを指定する）
//--------------------------------------------------
#define	LCDBM_CMD_TEXT_FONT_SIZE		3	///< 「文字フォント指定」のデータサイズ
#define	LCDBM_CMD_TEXT_COLOR_SIZE		4	///< 「文字色指定」のデータサイズ
#define	LCDBM_CMD_TEXT_BLINK_SIZE		6	///< 「文字列ブリンク指定」のデータサイズ
#define	LCDBM_CMD_TEXT_CLEAR_SIZE		5	///< 「クリア指定」のデータサイズ
#define	LCDBM_CMD_TEXT_MENU_SIZE		4	///< 「メニュー表示指定」の最小データサイズ（データ数により可変長）
#define	LCDBM_CMD_TEXT_DIRECT_SIZE		4	///< 「文字列指定」の最小データサイズ（データ数により可変長）
#define	LCDBM_CMD_TEXT_BACKGROUND_SIZE	4	///< 「背景色指定」のデータサイズ
/**
 *	テキストコマンド作成開始
 *
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/29(木)
 *	@attention
 *		PKTcmd_text_begin() ～ PKTcmd_text_end() の間に
 *		１コマンド内にコントロールコードを一つのみ指定するテキスト系コマンド関数が呼ばれたりすると
 *		コマンドが正常に生成できなくなるため、PKTcmd_text_begin() ～ PKTcmd_text_end()は見通しのよい箇所で使うこと。
 */
void PKTcmd_text_begin( void )
{
	// テキストコマンド作成開始関数を呼び出し済みとする
	text_work.is_begun = TRUE;

	// 文字列データを格納していない状態とする
	text_work.bytes = 0;

}


/**
 *	テキストコマンド作成終了＆送信予約
 *
 *	@retval	FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval	TRUE	パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@attention
 *		PKTcmd_text_begin() ～ PKTcmd_text_end() の間に
 *		１コマンド内にコントロールコードを一つのみ指定するテキスト系コマンド関数が呼ばれたりすると
 *		コマンドが正常に生成できなくなるため、PKTcmd_text_begin() ～ PKTcmd_text_end()は見通しのよい箇所で使うこと。
 */
BOOL PKTcmd_text_end( void )
{
	lcdbm_cmd_text_t	*dst = &text_command;
	ushort	data_length;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 初期化
	dst->command.length = (unsigned short)( sizeof(text_command.command.id) + sizeof(text_command.command.subid) + text_work.bytes );
	dst->command.id = LCDBM_CMD_ID_TEXT;
	dst->command.subid = LCDBM_CMD_SUBID_TEXT;

	// テキストコマンド作成開始関数を呼び出し前とする
	text_work.is_begun = FALSE;

	data_length = (ushort)( dst->command.length + sizeof(text_command.command.length) );
	return ( PKTbuf_SetSendCommand( (uchar *)&text_command, data_length ) );
}


/**
 *	文字フォント指定
 *
 *	@param[in]	font_type	フォント種別（LCDBM_TEXT_FONT_16～）
 *	@retval		FALSE		テキストコマンド長チェックでＮＧ、または	<br>
 *							フォント種別チェックでＮＧ
 *	@retval		TRUE		正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+'%' （1BH，25H）
 *	-	言語指定、フォント種別の正当性チェックは関数内では行っていない
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_font( const uchar font_type )
{
	unsigned char	*p;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_FONT_SIZE) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_FONT;

	// フォント種別
	*p = font_type;

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_FONT_SIZE;

	return TRUE;
}

/**
 *	文字色指定
 *
 *	@param[in]	color	１６ビットカラーデータ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+''' （1BH，27H）
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_color( const ushort color )
{
	unsigned char	*p;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_COLOR_SIZE) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_COLOR;

	// RGB指定
	*p++ = (uchar)( color >> 8 );
	*p   = (uchar)( color & 0xff);

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_COLOR_SIZE;

	return TRUE;
}


/**
 *	文字ブリンク指定
 *
 *	@param[in]	line		行(0～7)
 *	@param[in]	column		列(設定無効)
 *	@param[in]	blink		開始／解除(OFF(0):解除/ON(1):開始)
 *	@param[in]	interval	点滅間隔（0～255(×10msec)）
 *	@retval		FALSE		テキストコマンド長チェックでＮＧ
 *	@retval		TRUE		正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+'(' （1BH，28H）
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_blink( const uchar line, const uchar column, const uchar blink, const uchar interval )
{
	unsigned char	*p;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_BLINK_SIZE) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_BLINK;

	// 行
	*p++ = line;

	// 列
	*p++ = column;

	// 開始／解除
	*p++ = blink;

	// 点滅間隔
	*p   = interval;

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_BLINK_SIZE;

	return TRUE;
}

/**
 *	クリア指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0固定)
 *	@param[in]	range	クリア範囲(0:1行クリア(LCDBM_CLEAR_1_LINE)/1:全クリア(LCDBM_CLEAR_ALL_LINE))
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 *	-	空白を表示する事によりテキストを消去する
 */
BOOL PKTcmd_text_clear( const uchar line, const uchar column, const uchar range )
{
	unsigned char	*p;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_CLEAR_SIZE) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_CLEAR;

	// 行
	*p++ = line;

	// 列
	*p++ = column;

	// クリア範囲
	*p   = range;

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_CLEAR_SIZE;

	return TRUE;
}

/**
 *	メニュー表示指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0～29列目)
 *	@param[in]	src		文字列データ(SJISコード)格納領域へのポインタ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ、または	<br>
 *						文字列データ長＝０
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+',' （1BH，2CH）
 *	-	文字列データに'\0'が含まれる場合はその直前までを有効な文字列データとする。
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_menu( const uchar line, const uchar column, const uchar *src )
{
	unsigned char	*p;
	unsigned short	length = 0;		// '\0'直前までのバイト数

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 文字列から'\0'を探し、'\0'の前までを有効とする
	for ( length = 0; length < LCDBM_CMD_TEXT_CLMN_MAX; length++ ) {
		if ( src[length] == '\0' ) {
			break;
		}
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_MENU_SIZE + length) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_MENU;

	// 行
	*p++ = line;

	// 列
	*p++ = column;

	// 文字列データのコピー
	memcpy( p, src, (size_t)length );

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_DIRECT_SIZE + length;

	return TRUE;
}

/**
 *	文字列指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0～29列目)
 *	@param[in]	src		文字列データ(SJISコード)格納領域へのポインタ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ、または	<br>
 *						文字列データ長＝０
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+'-' （1BH，2DH）
 *	-	文字列データに'\0'が含まれる場合はその直前までを有効な文字列データとする。
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_direct( const uchar line, const uchar column, const uchar *src )
{
	unsigned char	*p;
	unsigned short	length = 0;		// '\0'直前までのバイト数

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 文字列から'\0'を探し、'\0'の前までを有効とする(文字列最大は30)
	for ( length = 0; length < 30; length++ ) {
		if ( src[length] == '\0' ) {
			break;
		}
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_DIRECT_SIZE + length) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_DIRECT;

	// 行
	*p++ = line;

	// 列
	*p++ = column;

	// 文字列データのコピー
	memcpy( p, src, (size_t)length );

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_DIRECT_SIZE + length;

	return TRUE;
}

/**
 *	背景色指定
 *
 *	@param[in]	color	RGB(0000h～FFFFh (R=5bit，G=6bit，B=5bit))
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+''' （1BH，27H）
 *	-	本関数呼び出し前後で PKTcmd_text_begin(), PKTcmd_text_end() を呼び出す必要あり。
 */
BOOL PKTcmd_text_backgroundcolor( const ushort color )
{
	unsigned char	*p;

	// テキストコマンド作成開始済みでないならリターンする
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// 残りバッファ容量チェック
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_BACKGROUND_SIZE) ) {
		return FALSE;
	}

	// データ
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_BACKGROUND;

	// RGB指定
	*p++ = (uchar)( color >> 8 );
	*p   = (uchar)( color & 0xff);

	// データ格納バイト数更新
	text_work.bytes += LCDBM_CMD_TEXT_BACKGROUND_SIZE;

	return TRUE;
}

//--------------------------------------------------
//		テキスト系コマンド
//	（１コマンド内にコントロールコードを一つのみ指定する）
//--------------------------------------------------
/**
 *	文字フォント指定
 *
 *	@param[in]	font_type	フォント種別（LCDBM_TEXT_FONT_16～）
 *	@retval		FALSE		テキストコマンド長チェックでＮＧ、または	<br>
 *							フォント種別チェックでＮＧ
 *	@retval		TRUE		正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(月)
 *	@see	PKTcmd_text_font()
 *	@note
 *	-	コントロールコード： ESC+'%' （1BH，25H）
 *	-	言語指定、フォント種別の正当性チェックは関数内では行っていない
 */
BOOL PKTcmd_text_1_font( const uchar font_type )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_font( font_type ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	文字色指定
 *
 *	@param[in]	color	１６ビットカラーデータ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(月)
 *	@see	PKTcmd_text_color()
 *	@note
 *	-	コントロールコード： ESC+''' （1BH，27H）
 */
BOOL PKTcmd_text_1_color( const ushort color )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_color( color ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	文字ブリンク指定
 *
 *	@param[in]	line		行(0～7)
 *	@param[in]	column		列(設定無効)
 *	@param[in]	blink		開始／解除(OFF(0):解除/ON(1):開始)
 *	@param[in]	interval	点滅間隔（0～255(×10msec)）
 *	@retval		FALSE		テキストコマンド長チェックでＮＧ
 *	@retval		TRUE		正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(月)
 *	@see	PKTcmd_text_blink()
 *	@note
 *	-	コントロールコード： ESC+'(' （1BH，28H）
 */
BOOL PKTcmd_text_1_blink( const uchar line, const uchar column, const uchar blink, const uchar interval )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	クリア指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0固定)
 *	@param[in]	range	クリア範囲(0:1行クリア(LCDBM_CLEAR_1_LINE)/1:全クリア(LCDBM_CLEAR_ALL_LINE))
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 *	-	空白を表示する事によりテキストを消去する
 */
BOOL PKTcmd_text_1_clear( const uchar line, const uchar column, const uchar range )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_clear( line, column, range ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	メニュー表示指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0～29列目)
 *	@param[in]	src		文字列データ(SJISコード)格納領域へのポインタ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(月)
 *	@note
 *	-	コントロールコード： ESC+',' （1BH，2CH）
 *	-	文字列データに'\0'が含まれる場合はその直前までを有効な文字列データとする。
 */
BOOL PKTcmd_text_1_menu( const uchar line, const uchar column, const uchar *src )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_menu( line, column, src ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	文字列指定
 *
 *	@param[in]	line	行(0～7)
 *	@param[in]	column	列(0～29列目)
 *	@param[in]	src		文字列データ格納領域へのポインタ
 *	@retval		FALSE	テキストコマンド長チェックでＮＧ、または	<br>
 *						文字列データ長＝０
 *	@retval		TRUE	正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(月)
 *	@see	PKTcmd_text_direct()
 *	@note
 *	-	コントロールコード： ESC+'-' （1BH，2DH）
 *	-	文字列データに'\0'が含まれる場合はその直前までを有効な文字列データとする。
 *	-	文字列データは半角と全角が入ると奇数バイトになる可能性あり。
 */
BOOL PKTcmd_text_1_direct( const uchar line, const uchar column, const uchar *src )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_direct(line, column, src ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}
/**
 *	カラーLCDへの背景色表示
 *
 *	@param[in]	color		１６ビットカラーデータ
 *	@date	2019/12/25
 */
// MH810100(S) S.Takahashi 2019/12/25 車番チケットレス(メンテナンス)
BOOL PKTcmd_text_1_backgroundcolor( ushort color )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_backgroundcolor( color ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}
// MH810100(E) S.Takahashi 2019/12/25 車番チケットレス(メンテナンス)
/**
 *	カラーLCDへのテキスト表示
 *
 *	@param[in]	color		１６ビットカラーデータ
 *	@param[in]	blink		開始／解除(OFF(0):解除/ON(1):開始)
 *	@param[in]	interval	点滅間隔（0～255(×10msec)）
 *	@param[in]	line		行(0～7)
 *	@param[in]	column		列(0～29列目)
 *	@param[in]	src			文字列データ(SJISコード)格納領域へのポインタ
 *	@retval		TRUE		正常終了
 *	@retval		FALSE		異常終了
 *	@author	m.onouchi
 *	@date	2009/11/24(火)
 *	@attention	文字列指定の場合は終端文字(NULL)を付加すること。
 *	@note
 *	-	文字列を直接指定する方法とテキストファイルを指定する方法が選べます。
 *	-	テキストファイルは２バイト配列で複数指定が可能です。
 */
BOOL PKTcmd_text_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src )
{

	PKTcmd_text_begin();

	// カラー指定
	if ( LCDBM_COLOR_DEFAULT != color ) {
		if ( PKTcmd_text_color( (ushort)(color&0x0000FFFF) ) == FALSE ) {
			goto _cmderr;				// length error
		}
	}

	// ブリンク指定
	if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
		goto _cmderr;				// length error
	}

	// テキスト指定
	if ( PKTcmd_text_direct( line, column, src ) == FALSE ) {
		goto _cmderr;				// length error
	}

_cmderr:
	return PKTcmd_text_end();
}

/**
 *	カラーLCDへのテキスト表示
 *
 *	@param[in]	color		１６ビットカラーデータ
 *	@param[in]	blink		開始／解除(OFF(0):解除/ON(1):開始)
 *	@param[in]	interval	点滅間隔（0～255(×10msec)）
 *	@param[in]	line		行(0～7)
 *	@param[in]	column		列(0～29列目)
 *	@param[in]	src			文字列データ(SJISコード)格納領域へのポインタ
 *	@param[in]	step		今回のStep
 *	@param[in]	totalstep	全Step数
 *	@retval		TRUE		正常終了
 *	@retval		FALSE		異常終了
 *	@author	m.onouchi
 *	@date	2009/11/24(火)
 *	@attention	文字列指定の場合は終端文字(NULL)を付加すること。
 *	@note
 *	-	文字列を直接指定する方法とテキストファイルを指定する方法が選べます。
 *	-	テキストファイルは２バイト配列で複数指定が可能です。
 */
BOOL PKTcmd_text_1_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src, ushort step, ushort totalstep )
{

	if(step == 1) {
		PKTcmd_text_begin();

		// カラー指定
		if ( LCDBM_COLOR_DEFAULT != color ) {
			if ( PKTcmd_text_color((ushort)(color&0x0000FFFF) ) == FALSE) {
				goto _cmderr;				// length error
			}
		}

		// ブリンク指定
		if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
			goto _cmderr;				// length error
		}

	}

	// テキスト指定
	if ( PKTcmd_text_direct( line, column, src ) == FALSE ) {
		goto _cmderr;				// length error
	}

_cmderr:
	if(step >= totalstep) {
		// 全Step終了で作成終了＆送信予約
		return PKTcmd_text_end();
	}
	else {
		return TRUE;
	}
}



//--------------------------------------------------
//		アナウンス要求コマンド
//--------------------------------------------------
/**
 *	音量設定
 *
 *	@param[in]	volume	音量(0～100(0=無音))
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/21(水)
 *	@note
 */
BOOL PKTcmd_audio_volume( uchar volume )
{
	lcdbm_cmd_audio_volume_t	*dst = &SendCmdDt.au_volume;

	// 初期化
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_volume_t) );
	dst->command.length = sizeof(lcdbm_cmd_audio_volume_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_AUDIO;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_VOLUME;

	// 音量
	dst->volume = volume;

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_volume_t) ) );
}


/**
 *	開始要求
 *
 *	@param[in]	src		LCDへ送信するコマンドが格納された領域へのポインタ
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/15(木)
 *	@note
 *	-	送信コマンドは可変長なので注意。
 *	-	コマンドの各項目のチェックは行わない。
 */
BOOL PKTcmd_audio_start( uchar startType, uchar reqKind, ushort no )
{
	lcdbm_cmd_audio_start_t	*dst = &SendCmdDt.au_start;

	// 初期化
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_start_t) );

	dst->command.length = (unsigned short)( sizeof(lcdbm_cmd_audio_start_t) - sizeof(SendCmdDt.command.length) );
	dst->command.id = LCDBM_RSP_ID_MNT_DATA_LCD;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_START;

	// データ
	dst->times = 1;				// 放送回数(1固定)
	dst->condition = startType;	// 放送開始条件
	dst->interval = 0;				// メッセージ間隔(放送回数1固定のため未使用)
	dst->channel = 0;			// 放送チャネル(0固定)
	dst->language = 0;			// 言語指定(とりあえず0)
	dst->AnaKind = reqKind;		// 種別(0=メッセージ№/1=文節№)
	dst->AnaNo = no;			// メッセージ№/文節№

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_start_t) ) );
}


/**
 *	終了要求
 *
 *	@param[in]	channel			放送終了チャネル(0固定)
 *	@param[in]	intrpt_method	中断方法(0固定)
 *	@retval		FALSE			パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE			パケット送信予約 正常終了
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/15(木)
 */
BOOL PKTcmd_audio_end( uchar channel, uchar intrpt_method )
{
	lcdbm_cmd_audio_end_t	*dst = &SendCmdDt.au_end;

	// 初期化
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_end_t) );
	dst->command.length = sizeof(lcdbm_cmd_audio_end_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_RSP_ID_MNT_DATA_LCD;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_END;

	// データ
	dst->channel = channel;			// 放送チャネル
	dst->abort = intrpt_method;		// 中断方法

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_end_t) ) );
}

//--------------------------------------------------
//		ブザー要求コマンド
//--------------------------------------------------
/**
 *	音量設定
 *
 *	@param[in]	volume	音量(0～3(0=無音，3=最大音量))
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_beep_volume( uchar volume )
{
	lcdbm_cmd_beep_volume_t	*dst = (lcdbm_cmd_beep_volume_t*)&SendCmdDt.bp_volume;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_beep_volume_t));
	dst->command.length = sizeof(lcdbm_cmd_beep_volume_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_VOLUME;

	// データ
	dst->volume = volume;	// 音量

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_beep_volume_t)));
}

/**
 *	ブザー鳴動要求
 *
 *	@param[in]	beep	種類(0=ピッ♪、1=ピピィ♪、2=ピピピィ♪)
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 */
BOOL PKTcmd_beep_start( uchar beep )
{
	lcdbm_cmd_beep_start_t	*dst = (lcdbm_cmd_beep_start_t*)&SendCmdDt.bp_start;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_beep_start_t));
	dst->command.length = sizeof(lcdbm_cmd_beep_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_START;

	// データ
	dst->beep = beep;	// 種類

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_beep_start_t)));
}

/**
 *	警報鳴動要求
 *
 *	@param[in]	kind	種類(0=固定)
 *	@param[in]	type	種別(0=開始、1=停止)
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@date	2019/11/28
 *	@note
 */
BOOL PKTcmd_alarm_start( uchar kind, uchar type )
{
	lcdbm_cmd_alarm_start_t	*dst = (lcdbm_cmd_alarm_start_t*)&SendCmdDt.bp_start;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_alarm_start_t));
	dst->command.length = sizeof(lcdbm_cmd_alarm_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
//	dst->command.subid = LCDBM_CMD_SUBID_BEEP_START;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_ALARM;
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない

	// データ
	dst->kind = kind;	// 種類
	dst->type = type;	// 種別

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_alarm_start_t)));
}

//--------------------------------------------------
//		精算機状態通知操作コマンド
//--------------------------------------------------
/**
 *	操作通知
 *
 *	@param[in]	ope_code	操作コード
 *	@param[in]	status		状態
 *	@retval		FALSE		パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE		パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_notice_ope( uchar ope_code, ushort status )
{
	lcdbm_cmd_notice_ope_t	*dst = (lcdbm_cmd_notice_ope_t*)&SendCmdDt.operate;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_ope_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_ope_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_OPE;

	// データ
	dst->ope_code = ope_code;	// 操作コード
	dst->status = status;		// 状態

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_ope_t)));
}

/**
 *	営休業通知
 *
 *	@param[in]	opn_cls	営休業(0=営業,1=休業)
 *	@param[in]	reason	休業理由
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_notice_opn( uchar opn_cls, uchar reason )
{
	lcdbm_cmd_notice_opn_t	*dst = (lcdbm_cmd_notice_opn_t*)&SendCmdDt.opn_cls;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_opn_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_opn_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_OPN;

	// データ
	dst->opn_cls = opn_cls;	// 営休業
	dst->reason = reason;	// 休業理由

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_opn_t)));
}

/**
 *	精算状態通知
 *
 *	@param[in]	event	事象(Bit単位:0=解除,1=発生)
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_notice_pay( ulong event )
{
	lcdbm_cmd_notice_pay_t	*dst = (lcdbm_cmd_notice_pay_t*)&SendCmdDt.pay_state;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_pay_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_pay_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_PAY;

	// データ
	dst->event = event;	// 事象

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_pay_t)));
}

/**
 *	ポップアップ表示要求
 *
 *	@param[in]	text_code	テキストコード(1:預り証発行,2:残高不足,3:減額失敗(再タッチ要求))
 *	@retval		FALSE		パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE		パケット送信予約 正常終了
 *	@note
 */
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//BOOL PKTcmd_notice_dsp( uchar text_code, uchar status )
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info )
BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info, uchar *str, ulong str_size )
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
{
	lcdbm_cmd_notice_dsp_t	*dst = (lcdbm_cmd_notice_dsp_t*)&SendCmdDt.display;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	ushort length = sizeof(lcdbm_cmd_notice_dsp_t);
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_dsp_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	if ( text_code != POP_QR_RECIPT ) {
		dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
	} else {
		dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
	}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_DSP;

	// データ
	dst->text_code = text_code;	// コード
	dst->status    = status;	// 状態
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
	dst->add_info  = add_info;	// 追加情報
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_dsp_t)));
	if ( text_code == POP_QR_RECIPT ) {
		// 追加文字列を設定する
		if ( str_size > LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE ) {
			memcpy(dst->add_str, str, LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE);
		} else {
			memcpy(dst->add_str, str, str_size);
		}
	} else {
		// 追加文字列を削る
		dst->command.length -= LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE;
		length -= LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE;
	}

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, length));
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
}

/**
 *	警告通知
 *
 *	@param[in]	event	事象(Bit単位:0=解除,1=発生)
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_notice_alm( ushort event )
{
	lcdbm_cmd_notice_alm_t	*dst = (lcdbm_cmd_notice_alm_t*)&SendCmdDt.alarm;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_alm_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_alm_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_ALM;

	// データ
	dst->event = event;	// 事象

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_alm_t)));
}

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
/**
 *	ポップアップ削除要求
 *
 *	@param[in]	NONE
 *	@retval		FALSE		パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE		パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_notice_del(uchar kind, uchar status)
{
	lcdbm_cmd_notice_del_t	*dst = (lcdbm_cmd_notice_del_t*)&SendCmdDt.display_del;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_del_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_del_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_DEL;

	// データ
	dst->kind = kind;		// 種類
	dst->status = status;	// 状態

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_del_t)));
}
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

//--------------------------------------------------
//		精算・割引情報通知コマンド
//--------------------------------------------------
/**
 *	精算残高変化通知
 *
 *	@param[in]	src		LCDへ送信するｺﾏﾝﾄﾞが格納された領域へのﾎﾟｲﾝﾀ
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_pay_rem_chg( lcdbm_cmd_pay_rem_chg_t *src )
{
	lcdbm_cmd_pay_rem_chg_t	*dst = (lcdbm_cmd_pay_rem_chg_t*)&SendCmdDt.pay_rem_chg;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_pay_rem_chg_t));
	dst->command.length = sizeof(lcdbm_cmd_pay_rem_chg_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_PAY_REM_CHG;

	// データコピー
	memcpy(&SendCmdDt.pay_rem_chg.id, &(src->id), sizeof(lcdbm_cmd_pay_rem_chg_t)-sizeof(lcdbm_cmd_base_t));

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_pay_rem_chg_t)));
}

/**
 *	QRデータ応答
 *
 *	@param[in]	id		入庫から精算完了までを管理するID
 *	@param[in]	result	結果(0:OK,1:NG(排他),2:NG(枚数上限))
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_QR_data_res( ulong id, uchar result )
{
	lcdbm_cmd_QR_data_res_t	*dst = (lcdbm_cmd_QR_data_res_t*)&SendCmdDt.QR_data_res;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_QR_data_res_t));
	dst->command.length = sizeof(lcdbm_cmd_QR_data_res_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_QR_DATA_RES;

	// データ
	dst->id = id;			// ID
	dst->result = result;	// 結果

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_QR_data_res_t)));
}

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
//[]----------------------------------------------------------------------[]
///	@brief			入庫時刻指定遠隔精算開始
//[]----------------------------------------------------------------------[]
///	@param[in]		ushort	: 入庫時刻指定遠隔精算開始
///	@return			ret		: パケット送信予約結果<br>
///							  TRUE  = 正常終了<br>
///							  FALSE = 失敗(パケット送信バッファオーバーフロー)<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
BOOL PKTcmd_remote_time_start( lcdbm_cmd_remote_time_start_t *src )
{
	lcdbm_cmd_remote_time_start_t *dst = (lcdbm_cmd_remote_time_start_t*)&SendCmdDt.remote_time_start;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_remote_time_start_t));
	dst->command.length = sizeof(lcdbm_cmd_remote_time_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_REMOTE_TIME_START;

	// データコピー
	memcpy(&(dst->ulPno), &(src->ulPno),
		sizeof(lcdbm_cmd_remote_time_start_t) - sizeof(lcdbm_cmd_base_t));

	return (PKTbuf_SetSendCommand((uchar*)&SendCmdDt, sizeof(lcdbm_cmd_remote_time_start_t)));
}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

//--------------------------------------------------
//		メンテナンスデータコマンド
//--------------------------------------------------
/**
 *	QRリーダ制御要求
 *
 *	@param[in]	ctrl_cd	制御コード(0:バージョン要求,1:読取開始,2:読取停止)
 *	@retval		FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval		TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_mnt_qr_ctrl_req( unsigned char ctrl_cd )
{
	lcdbm_cmd_QR_ctrl_req_t	*dst = (lcdbm_cmd_QR_ctrl_req_t*)&SendCmdDt.QR_ctrl_req;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_QR_ctrl_req_t));
	dst->command.length = sizeof(lcdbm_cmd_QR_ctrl_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_QR_CTRL_REQ;

	// データ
	dst->ctrl_cd = ctrl_cd;	// 制御コード

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_QR_ctrl_req_t)));
}

/**
 *	リアルタイム通信疎通要求
 *
 *	@retval	FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval	TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_mnt_rt_con_req( void )
{
	lcdbm_cmd_rt_con_req_t	*dst = (lcdbm_cmd_rt_con_req_t*)&SendCmdDt.rt_con_req;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_rt_con_req_t));
	dst->command.length = sizeof(lcdbm_cmd_rt_con_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_RT_CON_REQ;

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_rt_con_req_t)));
}

/**
 *	DC-NET通信疎通要求
 *
 *	@retval	FALSE	パケット送信予約 失敗（パケット送信バッファオーバーフロー）
 *	@retval	TRUE	パケット送信予約 正常終了
 *	@note
 */
BOOL PKTcmd_mnt_dc_con_req( void )
{
	lcdbm_cmd_dc_con_req_t	*dst = (lcdbm_cmd_dc_con_req_t*)&SendCmdDt.dc_con_req;

	// 初期化
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_dc_con_req_t));
	dst->command.length = sizeof(lcdbm_cmd_dc_con_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_DC_CON_REQ;

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_dc_con_req_t)));
}

//@debug <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//--------------------------------------------------
//		テストコード
//--------------------------------------------------
#ifdef	CRW_DEBUG

// 文字列データ
uchar	*test_text_message[] = {
	"サービス券Aを先に入れて下さい",	// 全角の途中に半角が入っているパターン
};

// テキストファイルNo.データ
ushort	test_text_fileNo[] = {
	0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987,
};

// ポップアップ画像の表示左上座標
lcdbm_pos_t	test_popup_image_pos = { 55, 89 };

// ポップアップテキストの開始座標
lcdbm_pos_t	test_popup_text_pos = { 11, 13 };


void test_packet_function( void )
{
#if 0
//
//	unsigned char	area = 1;
//
//
//		PKTcmd_image_part( 1, 0, FALSE );	// 表示区画No.=1、グラフィックパターンNo.=0、動画停止=しない
//
//		// テキスト
//		PKTcmd_text_begin();
//
//// 		// 登録済みの文字列データがバッファサイズを超えた場合のテスト
//// 		PKTcmd_text_direct( test_text_message[0], TEXT_CMD_BUF_SIZE-LCDBM_CMD_TEXT_DIRECT_SIZE );
//// 		PKTcmd_text_fileNo( &test_text_fileNo[0], TEXT_CMD_BUF_SIZE-LCDBM_CMD_TEXT_FILENO_SIZE );
//
//		// 各関数のテスト
//		PKTcmd_text_font( LCDBM_LANGUAGE_CHINESE, LCDBM_TEXT_FONT_64 );
//		PKTcmd_text_color( lcdbm_color_RGB(22,45,22) );
//		PKTcmd_text_blink( ON, 4875/*0x130b*/ );
//
//		PKTcmd_text_align( LCDBM_TEXT_ALIGN_CENTER );
//		PKTcmd_text_scroll( float_fix16(33.333333) );
//		PKTcmd_text_clock( LCDBM_TEXT_CLOCK_ENGLISH, ON );
//
//		PKTcmd_text_direct( test_text_message[0], 0 );	// 文字列長を０としてテスト
//		PKTcmd_text_direct( test_text_message[0], 10 );	// 実際の文字列長より短いデータ長を指定してテスト
//		PKTcmd_text_direct( test_text_message[0], 40 );	// 実際の文字列長より長いデータ長を指定してテスト
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 0 );	// データ数を０としてテスト
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 10 );	// 指定可能なデータ数ぴったり
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 15 );	// 指定可能なデータ数よりも多い場合
//
//		PKTcmd_text_end( 254 );
//
//		// ポップアップ
//		PKTcmd_popup_on( test_popup_image_pos, test_popup_text_pos, 123 );
//		PKTcmd_popup_off();
//
//		// 設定要求
//		PKTcmd_request_config();
//		PKTcmd_request_status();
//
//		// 機器設定要求
//		PKTcmd_clock();
//		PKTcmd_backlight( ON );
//		PKTcmd_brightness( 21930 );	// テストのために大きい値を指定してみる
//
//
//		// タスク切り替えをして、いったん電文を送信する
//		taskchg( IDLETSKNO );
//
//		// テキスト関数のテスト
//		PKTcmd_text_1_font(   area, LCDBM_LANGUAGE_CHINESE, LCDBM_TEXT_FONT_64 );
//		PKTcmd_text_1_color(  area, lcdbm_color_RGB(22,45,22) );
//		PKTcmd_text_1_blink(  area, ON, 4875/*0x130b*/ );
//		PKTcmd_text_1_align(  area, LCDBM_TEXT_ALIGN_CENTER );
//		PKTcmd_text_1_scroll( area, float_fix16(33.333333) );
//		PKTcmd_text_1_clock(  area, LCDBM_TEXT_CLOCK_ENGLISH, ON );
//		PKTcmd_text_1_direct( area, test_text_message[0], 40 );
//		PKTcmd_text_1_fileNo( area, &test_text_fileNo[0], 15 );
//
#endif
}

#endif//CRW_DEBUG
//@debug >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// MH810100(E) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
