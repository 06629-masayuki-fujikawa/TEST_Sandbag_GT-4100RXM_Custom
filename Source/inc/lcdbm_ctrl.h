// MH810100(S) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
/**
 *	@file	lcdbm_ctrl.h
 *	@brief	LCDモジュール制御ヘッダファイル
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@since	2009/10/08 08:33:50
 *
 *	@internal
 *	$Name:$
 *	$Author:$
 *	$Date::                            $
 *	$Revision:$
 *
 *		Copyright (c)  Amano Co.,Ltd. 2009
 *		Licensed Material of Amano Co.,Ltd.
 *
 */
#ifndef _LCDBM_CTRL_H_
#define _LCDBM_CTRL_H_

//--------------------------------------------------
//		INCLUDE
//--------------------------------------------------
#include "system.h"
#include "common.h"
//#include "mdl_def.h"
#include "pkt_def.h"
#include "lcdbm_def.h"
//#include "lcdbm_popup.h"
//#include "lcdbm_anime.h"

// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)
// #define	_countof(a)				(sizeof(a) / sizeof((a)[0]))
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)

//-#define	LCDBM_AUDIO_COMMAND_QUEUE_DEPTH		5	///< LCDモジュールの音声系コマンドの最大保持数
/** LCDモジュールのから受信するバージョン桁数 */
#define	LCDBM_VERSION_LENGTH				10	// バージョン長

//-enum {
//-	LCDBM_EVENT_ID_AUDIO,	///< 音声系イベント
//-	LCDBM_EVENT_ID_IMAGE,	///< 画像系イベント
//-	LCDBM_EVENT_ID_NUM,		///< イベントＩＤ数
//-};

/**
 *	LCDモジュール制御構造体
 */
typedef struct {

//-	/** 言語 */
//-	struct {
//-		unsigned char	no;		///< カレント言語（０＝日本語、１＝英語、２＝韓国語、３＝中国語）
//-		unsigned char	index;	///< 現在の選択言語インデックス（０～LCDBM_LANGUAGE_CONFIG_NUM）
//-
//-	} language;
//-
//-	/** 画面 */
//-	struct {
//-		unsigned short	base_No;	///< 現在表示中の基本画面番号
//-		unsigned char	part_No;	///< 再生終了した区画番号
//-		unsigned short	lcd_no;		// LCDNOを保存する（表示高速化のために同じ画面を何度も表示するのを抑制する）
//-	} display;
//-
//-	/** 音声 */
//-	struct {
//-		unsigned char	volume;			///< LCDモジュールに設定したボリューム値（０～LCDBM_VOLUME_MAX-1）
//-		unsigned long	command_No;		///< 前回のコマンド識別番号（１以上とする）
//-		unsigned long	issued_buf[LCDBM_AUDIO_COMMAND_QUEUE_DEPTH];	///< 終了待ちコマンド識別番号を保持しておくバッファ.
//-
//-	} audio;

	/** LCDモジュールから受信したデータ */
	struct {
		// 基本設定応答
		struct {
			unsigned char	prgm_ver[LCDBM_VERSION_LENGTH];		///< プログラムバージョン(LCD制御モジュールソフトウェアのバージョン情報(ASCI))
			unsigned char	audio_ver[LCDBM_VERSION_LENGTH];	///< 音声データバージョン(音声データのバージョン情報(ASCI))
			unsigned long	lcd_startup_time;					///< LCDモジュール起動時間
			unsigned short	lcd_brightness;						///< LCD輝度(0～255)
			unsigned char	reserved[10];						///< 予備
		} config;
//		// 状態応答
//		struct {
//			unsigned short	progress;		///< LCDモジュール状態
//			unsigned long	SD_writing;		///< SDカード書き込み回数
//		} status;
//		// 通信設定応答
//		struct {
//			unsigned char	result;			///< 処理結果 00H：成功 00H以外：失敗
//			unsigned char	hostip[12];		///< 自局IPアドレス（ASCII）
//			unsigned char	lcdip[12];		///< LCDモジュールIPアドレス（ASCII）
//			unsigned char	hostsubnet[12];	///< 自局モジュールサブネットマスク（ASCII）
//			unsigned char	hostgateway[12];///< 自局モジュールデフォルトゲートウェイ（ASCII）
//			unsigned char	lcdtcpport[5];	///< LCDモジュールTCPポート（ASCII）
//		} tcpip;
//		// SDカードテスト
//		struct {
//			unsigned char		res;		///< 結果（0=OK、1=SD未実装、2=リードオンリー）
//			unsigned long		readtime;	///< SDリードタイム（単位：ミリ秒）
//			unsigned long		writetime;	///< SDライトタイム（単位：ミリ秒）
//		} sdreadwrite;
//		// 容量チェック
//		struct {
//			unsigned char		res;		///< 結果（0=OK）
//			unsigned long		freesize;	///< 空きサイズ（単位：MB）
//			unsigned long		totalsize;	///< 全体サイズ（単位：MB）
//		} sdcapacitycheck;
//		// 性能テスト
//		struct {
//			unsigned char		res;		///< 結果（0=OK、1=NG）
//			unsigned long		speed;		///< 性能*10（単位：KB/S）
//		} sdperformance;
	} receive_data;

//-	/** イベントフラグ */
//-	struct {
//-		unsigned short	bit;	/**< イベント発生待ち合わせフラグ
//-								 *	イベントが発生していない場合には該当ビットが０となっている。
//-								 *	イベント発生待ちに入る前に該当ビットを０クリアしておく。
//-								 *	イベントが発生した際には該当するビットを１にする。（イベントが発生した事を表す）
//-								 *	待っている全てのイベントが発生終了したかどうかは該当するビットパターンと比較する事で判定する。（AND待ち）
//-								 *	待っているどれかのイベントが発生終了したかどうかは該当するビットパターンが０かどうかで判定する。（OR待ち）
//-								 */
//-	} event[LCDBM_EVENT_ID_NUM];

	/** 各種ステータス */
	struct {
//-		BOOL	restart;				///< LCDモジュールの１回目の再起動を検出済みかどうか
//-		BOOL	detail;					///< 料金内訳表示済みかどうか
//-		BOOL	image_base;				///< 基本画面を強制的に書き換えるかどうか
//-		unsigned char	return_card;	/**< 返却カード種類
//-										 *	cardANOTHER =	なし
//-										 *	cardTICKET =	駐車券
//-										 *	cardPASS =		定期券
//-										 */
//-		struct {
//-			unsigned char	WarSts;		///< ワーニング状態
//-			unsigned char	ReciSw;		///< 領収証ボタン押下状況
//-		} Ope2_Sale_StsDisp;
//-		struct {
//-			unsigned short	change;			///< つり切れ表示金種をビットでOR指定する（LCDBM_CHANGE_BIT_10～）
//-			unsigned short	waiting;		///< 待機画面で表示するアラーム（LCDBM_WAITING_ALARM_CHANGE～）
//-			short			count_500ms;	///< 待機画面で表示するアラームの切替え間隔カウンタ
//-		} alarm;
		int		keepalive;
	} status;

} lcdbm_ctrl_t;

/**
 *	LCDモジュール設定情報構造体
 */
typedef struct {
	unsigned char	Version[LCDBM_VERSION_LENGTH];					// ソフトウェアバージョン
	unsigned char	Res1[2];										// 予備
	unsigned char	SDVersion[LCDBM_VERSION_LENGTH];				// SDカードデータバージョン
	unsigned char	Res2;											// 予備
	unsigned char	SD_read_time;									// SDカードリード時間
	unsigned char	Res3[2];										// 予備
	unsigned short	brightness;										// 輝度値
	unsigned char	touch;											// タッチパネル有無
	unsigned char	Res4;											// 予備
	unsigned char	Res5;											// 予備
	unsigned char	tenkey;											// テンキー対応
	unsigned char	ioext1;											// 拡張IO 1
	unsigned char	ioext2;											// 拡張IO 2
	unsigned char	ioext3;											// 拡張IO 3
	unsigned char	ioext4;											// 拡張IO 4
	unsigned char	rotsw;											// ロータリースイッチ
	unsigned char	dipsw;											// ディップスイッチ
} lcdbm_base_info_t;

/**
 *	ARCNET送受信データ構造体
 *	（ブロック数、データ長、データ本文、を構造体化）
 */
//typedef struct red_rec	red_rec_t;
//--------------------------------------------------
//		DEFINE
//	（GT-7000関連の情報を定義する）
//--------------------------------------------------
#define	LCD_HORIZONTAL_CHARS	31		// 白黒LCDの横文字数（半角で30文字＋'\0'の1文字）
#define	AVM_VOLUME_MAX			32		// AVM3の音量設定段階（設定可能範囲は 0～AVM_VOLUME_MAX-1）


//--------------------------------------------------
//		DEFINE
//	（GT-7700関連の情報を定義する）
//--------------------------------------------------
/**
 *	LCDモジュール制御状態
 */
enum {
	LCDBM_PROGRESS_BOOT,			///< 0: 起動開始
	LCDBM_PROGRESS_LOGO,			///< 1: 初期画面（LCDモジュールが静的に保持している会社ロゴ画面）表示中
	LCDBM_PROGRESS_SCREEN1,			///< 2: 初期画面表示終了（メイン側から基本画面表示コマンドが発行された後）
	LCDBM_PROGRESS_CHARGED,			///< 3: 電源供給基板蓄電完了
	LCDBM_PROGRESS_INITIALIZED,		///< 4: 初期化完了
};

/**
 *	フォント種別
 *	@note
 *	-	半角の場合は横サイズが半分になる。（縦サイズは変わらない）
 *	-	サイズは仮（2009/10/29現在）
 */
enum {
	LCDBM_TEXT_FONT_16,			///< 0: 全角16x16
	LCDBM_TEXT_FONT_24,			///< 1: 全角24x24
	LCDBM_TEXT_FONT_30,			///< 2: 全角30x30（使用する文字のみデータを持つ）
	LCDBM_TEXT_FONT_32,			///< 3: 全角32x32（線が細いので使わない可能性大）
	LCDBM_TEXT_FONT_36,			///< 4: 全角36x36
	LCDBM_TEXT_FONT_64,			///< 5: 全角64x64（使用する文字のみデータを持つ）
	LCDBM_TEXT_FONT_TYPES,		///< フォント種別数
};


/**
 *	寄せ指定
 */
enum {
	LCDBM_TEXT_ALIGN_LEFT,		///< 0: 左寄せ
	LCDBM_TEXT_ALIGN_CENTER,	///< 1: 中央寄せ
	LCDBM_TEXT_ALIGN_TYPES,		///< 寄せ指定 種別数
};


/**
 *	時計フォーマット
 */
enum{
	LCDBM_TEXT_CLOCK_JAPANESE =	0,		///< 日本語表記
	LCDBM_TEXT_CLOCK_ENGLISH =	400,	///< 英語表記
	LCDBM_TEXT_CLOCK_TYPES,				///< 時計書式 種別数
};


/**
 *	音量
 */
#define	LCDBM_VOLUME_MAX	128		///< LCDモジュールの音量設定段階（設定可能範囲は ０～AVM_VOLUME_MAX-1）


/**
 *	画面
 */
#define	LCDBM_LCD_WIDTH				800		///< ＬＣＤの横ドット数
#define	LCDBM_LCD_HEIGHT			600		///< ＬＣＤの縦ドット数
#define	LCDBM_LCD_BRIGHTNESS_MAX	15		///< ＬＣＤバックライト輝度設定の最大値
#define	LCDBM_LCD_CONTRAST_MAX		15		///< ＬＣＤコントラスト設定の最大値


/**
 *	払戻口・返却口
 */
enum {
	LCDBM_FPORT_COIN,		// 硬貨
	LCDBM_FPORT_RECEIPT,	// 領収証・預り証
	LCDBM_FPORT_NOTE_BNA,	// 紙幣（入金）
	LCDBM_FPORT_NOTE_BND,	// 紙幣（出金）
	LCDBM_FPORT_CARD,		// 磁気券(券領収証含)
	LCDBM_FPORT_GATE,		// ゲート
	LCDBM_FPORT_ALL,		// すべて
	// ここより上に追加する

	LCDBM_FPORT_DUMMY,		// 払出完了タイマー再セットで使う
};


/**
 *	テキスト表示系(lcdbm_grachr)
 */
#define	LCDBM_STRING		0												// 文字列直接指定
#define	LCDBM_COLOR_DEFAULT	0xFFFF0000										// 文字色:デフォルト
#define	LCDBM_COLOR_BLACK			((ulong)lcdbm_color_RGB( 0,  0,  0))	// 文字色:ブラック
#define	LCDBM_COLOR_RED				((ulong)lcdbm_color_RGB(31,  0,  0))	// 文字色:レッド
#define	LCDBM_COLOR_GREEN			((ulong)lcdbm_color_RGB( 7, 31, 11))	// 文字色:グリーン
#define	LCDBM_COLOR_BLUE			((ulong)lcdbm_color_RGB( 0,  0, 31))	// 文字色:ブルー
#define	LCDBM_COLOR_MIDNIGHTBLUE	((ulong)lcdbm_color_RGB( 2,  4,  8))	// 文字色:ミッドナイトブルー
#define	LCDBM_COLOR_PALETURQUOISE	((ulong)lcdbm_color_RGB(21, 59, 29))	// 文字色:パールターコイズ
#define	LCDBM_COLOR_LIGHTSEAGREEN	((ulong)lcdbm_color_RGB( 4, 44, 21))	// 文字色:ライトシーグリーン
#define	LCDBM_COLOR_DODGERBLUE		((ulong)lcdbm_color_RGB( 3, 18, 31))	// 文字色:ドジャーブルー
#define	LCDBM_COLOR_MEDIUMBLUE		((ulong)lcdbm_color_RGB( 0, 0, 25))		// 文字色:ミディアムブルー
#define	LCDBM_NO_BLINK		0												// ブリンク指定なし
#define	LCDBM_ATTR_NORMAL	0												// 通常属性
#define	LCDBM_ATTR_WARNING	1												// 警告属性


/**
 *	輝度調整系(lcdbm_lumine_change)
 */
#define	LCDBM_LUMINE_MAX	16		// 最大輝度値


/**
 *	LCDワーニング表示制御(カラーLCD)
 */
typedef struct {
	ushort	DspSts;				// LCD_WMSG_ON:ワーニング表示中，LCD_WMSG_OFF:ワーニング非表示中
	uchar	TextArea;			// ワーニング表示中のテキストエリア(中見出し:LCDBM_AREA_TEXT__PAYING_CAPTION_2，ポップアップ:LCDBM_AREA_TEXT__POPUP)
	ushort	excode;				// 例外表示コード(0-300)
} T_LCDBM_WMSG_CNT;

extern	T_LCDBM_WMSG_CNT	lcdbm_Lcd_Wmsg_Cnt;	// ワーニング表示制御エリア


/**
 *	テキストスクロール
 */
#define	LCDBM_TEXT_SCROLL_DEFAULT	float_fix16( 5.0 )		///< テキストスクロール量のデフォルト値


//--------------------------------------------------
//		型定義
//--------------------------------------------------
/**
 *	LCDモジュール １６ビットカラーデータ
 */
typedef union {
	unsigned short	data;
	struct {
		unsigned short	R:5;	///< 赤（0～31）
		unsigned short	G:6;	///< 緑（0～63）
		unsigned short	B:5;	///< 青（0～31）
	} bit;
} lcdbm_color16_t;


/**
 *	固定小数点型
 *
 *	整数部＝上位１２ビット、小数部＝下位４ビット
 */
typedef unsigned short	lcdbm_fix16;


/**
 *	Ｘ、Ｙ座標
 */
typedef struct {
	ushort	x;		///< X座標
	ushort	y;		///< Y座標
} lcdbm_pos_t;


//--------------------------------------------------
//		マクロ定義
//--------------------------------------------------
/**
 *	float値を小数部４ビットの固定小数形式（lcdbm_fix16型）に変換する
 */
#define	float_fix16( x )	( (lcdbm_fix16)(((float)(x) * 16.0f)) )

/**
 *	LCDモジュールとの電文のコマンドＩＤとサブコマンドＩＤから４バイトのＩＤコードを計算する
 */
#define	lcdbm_command_id( id, subid )	( (( (unsigned long)id & 0x0000ffff ) << 16 ) | ( (unsigned long)subid & 0x0000ffff) )

/**
 *	16ビットカラー値を作成する
 */
#define	lcdbm_color_RGB( r, g, b )	((unsigned short)( (((r)&0x1f) << 11) | (((g)&0x3f) << 5) | (((b)&0x1f) << 0) ))

/**
 *	LCDモジュールの輝度値に変換する
 */
#define	lcdbm_lumine(n)	(unsigned short)((n)?(n*LCDBM_LUMINE_MAX-1):0)	// 1～16:輝度値(15～255)，0:BackLight=OFF


//--------------------------------------------------
//		テキスト番号データ管理
//--------------------------------------------------
/**
 *	テキストファイルNo.を指定する場合の最大指定数
 *	本当は pkt_def.h に置くべきだが多重インクルードになってしまうので、ここで定義する。
 */
#define	TEXT_CMD_FILENO_NUM		10

/**
 *	テキスト番号列データ（番号格納配列あり）
 */
typedef struct {
	unsigned short	no[ TEXT_CMD_FILENO_NUM ];	///< テキスト番号格納配列
	unsigned short	number;						///< テキスト番号格納数
} lcdbm_text_array_t;

/**
 *	テキスト番号列データ（番号格納領域へのポインタあり）
 */
typedef struct {
	unsigned short	*p;			///< テキスト番号格納配列へのポインタ
	unsigned short	number;		///< テキスト番号格納数
} lcdbm_text_no_t;

/**
 *	テキスト番号列データの管理
 */
typedef struct {
	lcdbm_text_no_t	*p;			///< テキスト番号格納配列へのポインタ
	unsigned short	number;		///< テキスト番号格納数
} lcdbm_text_no_table_t;

/**
 *	テキスト番号列のデータ定義マクロ
 *	@note
 *	-	lcdbm_text_no_t型の配列を定義する時に使う
 */
#define	lcdbm_text( name )	{ &(name)[0], _countof(name), }


//--------------------------------------------------
//		プロトタイプ宣言
//--------------------------------------------------
// コマンドメッセージチェック
ushort lcdbm_check_message( const ulong data );

// キープアライブリトライ状態チェック
BOOL lcdbm_check_keepalive_status( uchar cnt );

// キープアライブリトライカウンタリセット
void lcdbm_reset_keepalive_status( void);

// 基本設定応答 受信時処理
void lcdbm_receive_config( lcdbm_rsp_config_t *p_rcv );

// 入庫情報 受信時処理
void lcdbm_receive_in_car_info( lcdbm_rsp_in_car_info_t *p_rcv );

// QRデータ 受信時処理
void lcdbm_receive_QR_data( lcdbm_rsp_QR_data_t *p_rcv );

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// レーンモニタデータ応答 受信時処理
void lcdbm_receive_DC_LANE_res( lcdbm_rsp_LANE_res_t *p_rcv, ulong *id, uchar *result );
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

// 精算応答データ 受信時処理
void lcdbm_receive_RTPay_res( lcdbm_rsp_pay_data_res_t *p_rcv, ulong *id, uchar *result );

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 領収証データ応答 受信時処理
void lcdbm_receive_RTReceipt_res( lcdbm_rsp_receipt_data_res_t *p_rcv, ulong *id, uchar *result );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

// QR確定・取消応答データ 受信時処理
void lcdbm_receive_DC_QR_res( lcdbm_rsp_QR_conf_can_res_t *p_rcv, ulong *id, uchar *result );

// QRリーダ制御応答 受信時処理
void lcdbm_receive_QR_ctrl_res( lcdbm_rsp_QR_ctrl_res_t *p_rcv );

// QR読取結果 受信時処理
void lcdbm_receive_QR_rd_rslt( lcdbm_rsp_QR_rd_rslt_t *p_rcv );

// LCD制御モジュールソフトウェアのバージョン情報を返す
uchar lcdbm_get_config_prgm_ver(char *dst, uchar size);

// 音声データのバージョン情報を返す
uchar lcdbm_get_config_audio_ver(char *dst, uchar size);

// LCDモジュール起動時間を返す
ulong lcdbm_get_config_lcd_startup_time(void);

// LCD輝度を返す
ushort lcdbm_get_config_lcd_brightness(void);

// 取得した基本設定応答の先頭ポインタを返す
uchar* lcdbm_get_config_ptr(void);

//-// LCDモジュール イベント処理
//-int lcdbm_receive_event( const unsigned short message_id );
//-
//-// パケット通信タスク関連の受信メールをクリア（廃棄）する
//-//void lcdbm_Ope_MsgBoxClear( const MsgBuf * const msb );

//--------------------------------------------------
//		インクルード
//--------------------------------------------------
//#include "lcdbm_sub.h"		// サブルーチン群
void number_string_to_zenkaku( const unsigned short *dst, const unsigned char *src, const unsigned short count );
void number_to_zenkaku( unsigned short *dst, const unsigned long number, unsigned short count );
void intoasl_zrschr(ushort *buf, const ulong data, uchar count);
void zrschr_al(char *dat, uchar cnt);
void zrschr( unsigned char* dat, unsigned char cnt );
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

BOOL lcdbm_lumine_change(ushort brightness);

#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------
//		テストコード
//--------------------------------------------------
#ifdef	CRW_DEBUG
void test_lcdbm_function( void );
#else
#define	test_lcdbm_function()	;	// リリースビルド時に"No prototype function"ワーニングが出ないようにする
#endif//CRW_DEBUG


#endif//_LCDBM_CTRL_H_
// MH810100(E) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
