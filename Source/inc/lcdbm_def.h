// MH810100(S) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
/**
 *	@file	lcdbm_def.h
 *	@brief	LCDモジュールで扱う管理番号の定義
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@since	2009/11/10 10:31:26
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
#ifndef _LCDBM_DEF_H_
#define _LCDBM_DEF_H_

//--------------------------------------------------
//		個別ソフトで使用する番号
//--------------------------------------------------
/**
 *	@note	SDカード内に保存する各種データの番号管理
 *
 *	＜ 基本画面パターンファイル ＞
 *	ファイル名（scr?????.ini）に指定する10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ グラフィックパターンファイル ＞
 *	GrphPtn.iniファイル内のパターン番号（[Image?????]）で指定する10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ 画像・動画ファイル ＞
 *	ファイル名（img?????.bmp, img?????.jpg, img?????.3g2）に指定する10進5桁の数値
 *	*.bmp：ビットマップ形式ファイル
 *	*.jpg：JPEGファイル
 *	*.3g2：動画ファイル
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ テキストパターンファイル ＞
 *	TextPtn.iniファイル内のパターン番号（[TextPtn?????]）で指定する10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ テキストデータ＞
 *	TextMsg.txtファイル内でテキスト番号を指定する行頭の10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ ポップアップパターンファイル ＞
 *	ファイル名（pop?????.ini）に指定する10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ 音声データファイル ＞
 *	ファイル名（phr?????.wav）に指定する10進5桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | 開発テスト用（製品版には実装しない）             |
 *	+---------------+--------------------------------------------------+
 *
 *	＜ 文節番号 ＞
 *	AnnTbl.ini内で分節番号を指定する10進3桁の数値
 *	+---------------+--------------------------------------------------+
 *	|   範囲        |    割り当て                                      |
 *	+===============+==================================================+
 *	|   001 -   799 | 標準ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *	|   800 -   999 | 個別ソフトで使用する                             |
 *	+---------------+--------------------------------------------------+
 *
 */


//--------------------------------------------------
//		基本画面番号
//--------------------------------------------------
/**
 *	基本画面番号
 */
enum {
	LCDBM_IMAGE_BASE__WAITING_CPS =	20000,						// 事前精算機用 待機画面
	LCDBM_IMAGE_BASE__WAITING_EPS =	20100,						// 出口精算機用 待機画面
	LCDBM_IMAGE_BASE__PAYING =		20200,						// 料金案内用画面
	LCDBM_IMAGE_BASE__PAY_SATRT =	LCDBM_IMAGE_BASE__PAYING,	// 駐車券挿入案内
	LCDBM_IMAGE_BASE__PAID =		LCDBM_IMAGE_BASE__PAYING,	// 精算完了
	LCDBM_IMAGE_BASE__PAY_CANCEL =	20270,						// とりけし画面
	LCDBM_IMAGE_BASE__EXIT =		20400,						// 出庫中ベース画面
	LCDBM_IMAGE_BASE__BREAK =		20500,						// 休業中ベース画面
	LCDBM_IMAGE_BASE__WARNING =		20600,						// 警告ベース画面
	LCDBM_IMAGE_BASE__MAINTENANCE =	20700,						// メンテナンス画面
	LCDBM_IMAGE_BASE__REFILL =		20800,						// 釣銭補充ベース画面
	LCDBM_IMAGE_BASE__MANUAL =		LCDBM_IMAGE_BASE__WARNING,	// 手動精算画面
};


//--------------------------------------------------
//		メンテナンス 画像番号
//--------------------------------------------------
enum {
	LCDBM_IMAGE_MAINTENANCE__BASE =		LCDBM_IMAGE_BASE__BREAK,		// 休業中と共用
	LCDBM_IMAGE_MAINTENANCE__LUMINE =	LCDBM_IMAGE_BASE__MAINTENANCE,	// 輝度調整(カラーパターン)
// LH068005 sekiguchi(S) 2010/03/25 LCDモジュール対応 動作チェック
	LCDBM_IMAGE_MAINTENANCE__YELLOW,									// 黄色
	LCDBM_IMAGE_MAINTENANCE__RED,										// 赤色
	LCDBM_IMAGE_MAINTENANCE__BLUE,										// 青色
	LCDBM_IMAGE_MAINTENANCE__BLACK,										// 黒色
	LCDBM_IMAGE_MAINTENANCE__GREEN,										// 緑色
	LCDBM_IMAGE_MAINTENANCE__WHITE,										// 白色
	LCDBM_IMAGE_MAINTENANCE__PATTERN,									// パターン
// LH068005 sekiguchi(E)
};


//--------------------------------------------------
//		釣銭補充 画像番号
//--------------------------------------------------
/**
 *	釣銭補充の部品番号
 *	@note
 *	-	釣銭補充中に表示する画像の定義
 */
enum {
	// 機種別背景
	LCDBM_IMAGE_REFILL__BASE_TYPE1 =	LCDBM_IMAGE_BASE__REFILL,	// 紙幣払出機なし(金銭管理あり)
	LCDBM_IMAGE_REFILL__BASE_TYPE2,									// 紙幣払出機あり(金銭管理あり)
	LCDBM_IMAGE_REFILL__BASE_TYPE3,									// 紙幣循環機あり(金銭管理あり)
	LCDBM_IMAGE_REFILL__BASE_TYPE4,									// 紙幣払出機なし(金銭管理なし)
	LCDBM_IMAGE_REFILL__BASE_TYPE5,									// 紙幣払出機あり(金銭管理なし)
	LCDBM_IMAGE_REFILL__BASE_TYPE6,									// 紙幣循環機あり(金銭管理なし)

	// コインメック
	LCDBM_IMAGE_REFILL__COINMEC_500,			// コインメック500円筒
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_100,			// コインメック100円筒
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_10,				// コインメック10円筒
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_50,				// コインメック50円筒
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_S_INVALID,		// コインメックSUB筒(未使用)
	LCDBM_IMAGE_REFILL__COINMEC_S10,			// コインメックSUB筒(10円)
	LCDBM_IMAGE_REFILL__COINMEC_S10_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_S100,			// コインメックSUB筒(100円)
	LCDBM_IMAGE_REFILL__COINMEC_S100_PTN1,

	// コインホッパー
	LCDBM_IMAGE_REFILL__HOPPER1_INVALID,		// コインホッパー1(未使用)
	LCDBM_IMAGE_REFILL__HOPPER1_10,				// コインホッパー1(10円)
	LCDBM_IMAGE_REFILL__HOPPER1_10_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_50,				// コインホッパー1(50円)
	LCDBM_IMAGE_REFILL__HOPPER1_50_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_100,			// コインホッパー1(100円)
	LCDBM_IMAGE_REFILL__HOPPER1_100_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_500,			// コインホッパー1(500円)
	LCDBM_IMAGE_REFILL__HOPPER1_500_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_INVALID,		// コインホッパー2(未使用)
	LCDBM_IMAGE_REFILL__HOPPER2_10,				// コインホッパー2(10円)
	LCDBM_IMAGE_REFILL__HOPPER2_10_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_50,				// コインホッパー2(50円)
	LCDBM_IMAGE_REFILL__HOPPER2_50_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_100,			// コインホッパー2(100円)
	LCDBM_IMAGE_REFILL__HOPPER2_100_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_500,			// コインホッパー2(500円)
	LCDBM_IMAGE_REFILL__HOPPER2_500_PTN1,

	// 紙幣払出機
	LCDBM_IMAGE_REFILL__CASSETTE1_INVALID,		// 紙幣払出カセット1(未使用)
	LCDBM_IMAGE_REFILL__CASSETTE1_1000,			// 紙幣払出カセット1(1000円)
	LCDBM_IMAGE_REFILL__CASSETTE1_1000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE1_2000,			// 紙幣払出カセット1(2000円)
	LCDBM_IMAGE_REFILL__CASSETTE1_2000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE1_5000,			// 紙幣払出カセット1(5000円)
	LCDBM_IMAGE_REFILL__CASSETTE1_5000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_INVALID,		// 紙幣払出カセット2(未使用)
	LCDBM_IMAGE_REFILL__CASSETTE2_1000,			// 紙幣払出カセット2(1000円)
	LCDBM_IMAGE_REFILL__CASSETTE2_1000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_2000,			// 紙幣払出カセット2(2000円)
	LCDBM_IMAGE_REFILL__CASSETTE2_2000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_5000,			// 紙幣払出カセット2(5000円)
	LCDBM_IMAGE_REFILL__CASSETTE2_5000_PTN1,

	// 紙幣循環機
	LCDBM_IMAGE_REFILL__NTCYCL_1000,			// 紙幣循環機1000円収納部
	LCDBM_IMAGE_REFILL__NTCYCL_1000_PTN1,

	// 硬貨投入口
	LCDBM_IMAGE_REFILL__COIN_ANIME_01,			// コイン投入アニメーション
	LCDBM_IMAGE_REFILL__COIN_ANIME_02,
	LCDBM_IMAGE_REFILL__COIN_ANIME_03,
	LCDBM_IMAGE_REFILL__COIN_ANIME_04,
	LCDBM_IMAGE_REFILL__COIN_ANIME_05,
	LCDBM_IMAGE_REFILL__COIN_ANIME_06,
	LCDBM_IMAGE_REFILL__COIN_ANIME_07,
	LCDBM_IMAGE_REFILL__COIN_ANIME_08,
	LCDBM_IMAGE_REFILL__COIN_ANIME_09,
	LCDBM_IMAGE_REFILL__COIN_ANIME_10,
	LCDBM_IMAGE_REFILL__COIN_ANIME_11,
	LCDBM_IMAGE_REFILL__COIN_ANIME_12,
	LCDBM_IMAGE_REFILL__COIN_ANIME_13,
	LCDBM_IMAGE_REFILL__COIN_ANIME_14,
	LCDBM_IMAGE_REFILL__COIN_ANIME_15,
	LCDBM_IMAGE_REFILL__COIN_ANIME_16,

	LCDBM_IMAGE_REFILL__DUMMY_77,
	LCDBM_IMAGE_REFILL__DUMMY_78,
	LCDBM_IMAGE_REFILL__DUMMY_79,
	LCDBM_IMAGE_REFILL__DUMMY_80,

	// 紙幣投入口
	LCDBM_IMAGE_REFILL__NOTE_ANIME_01,			// 紙幣挿入アニメーション
	LCDBM_IMAGE_REFILL__NOTE_ANIME_02,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_03,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_04,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_05,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_06,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_07,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_08,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_09,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_10,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_11,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_12,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_13,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_14,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_15,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_16,
};


/**
 *	釣銭金庫の状態パターン
 */
enum {
	LCDBM_IMAGE_REFILL_NO__NORMAL,			//  0: 補充可能＆規定枚数内
	LCDBM_IMAGE_REFILL_NO__EXCEED,			//  1: 補充可能＆枚数超過(自動釣り合わせの対象)
	LCDBM_IMAGE_REFILL_NO__STOP,			//  2: 補充不可
};


//--------------------------------------------------
//		区画番号
//--------------------------------------------------
/**
 *	待機画面（CPS、EPS） のテキスト区画番号
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		+-----------------------+-------+-----------------------+
 *		|                       |(Text5)|      時計(Text6)      |  <-- つり切れ表示（Text5+Text6の領域）
 *		+-----------------------+-------+-----------------------+
 *	@endcode
 */
// 事前精算機（CPS）
enum {
	LCDBM_AREA_TEXT__WAITING_CPS_00,		//  0: 未使用
	LCDBM_AREA_TEXT__WAITING_CPS_01,		//  1: 未使用
	LCDBM_AREA_TEXT__WAITING_CPS_02,		//  2: 未使用
	LCDBM_AREA_TEXT__WAITING_CPS_03,		//  3: 未使用
	LCDBM_AREA_TEXT__WAITING_CPS_04,		//  4: 未使用
	LCDBM_AREA_TEXT__WAITING_CPS_ALARM,		//  5: [24x24]	つり切れ等のアラーム
	LCDBM_AREA_TEXT__WAITING_CPS_CLOCK,		//  6: [24x24]	時計
};
// 出口精算機（EPS）
enum {
	LCDBM_AREA_TEXT__WAITING_EPS_00,		//  0: 未使用
	LCDBM_AREA_TEXT__WAITING_EPS_01,		//  1: 未使用
	LCDBM_AREA_TEXT__WAITING_EPS_02,		//  2: 未使用
	LCDBM_AREA_TEXT__WAITING_EPS_03,		//  3: 未使用
	LCDBM_AREA_TEXT__WAITING_EPS_04,		//  4: 未使用
	LCDBM_AREA_TEXT__WAITING_EPS_ALARM,		//  5: [24x24]	つり切れ等のアラーム
	LCDBM_AREA_TEXT__WAITING_EPS_CLOCK,		//  6: [24x24]	時計
};


/**
 *	待機画面（CPS、EPS） のグラフィック区画番号
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                 アニメーション領域                    |
 *		|                      (Graphic8)                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		+-------------------------------------------------------+
 *		|                 静止画領域(Graphic2)                  |
 *		+-------------------------------------------------------+
 *	@endcode
 */
// 事前精算機（CPS）
enum {
	LCDBM_AREA_GRAPHIC__WAITING_CPS_00,			// 0: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_01,			// 1: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_CAPTION_3,	// 2: 最下段＋時計
	LCDBM_AREA_GRAPHIC__WAITING_CPS_03,			// 3: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_04,			// 4: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_05,			// 5: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_06,			// 6: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_07,			// 7: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_CPS_ANIME_1,	// 8: アニメーション領域
};
// 出口精算機（EPS）
enum {
	LCDBM_AREA_GRAPHIC__WAITING_EPS_00,			// 0: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_01,			// 1: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_CAPTION_3,	// 2: 最下段＋時計
	LCDBM_AREA_GRAPHIC__WAITING_EPS_03,			// 3: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_04,			// 4: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_05,			// 5: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_06,			// 6: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_07,			// 7: 未使用
	LCDBM_AREA_GRAPHIC__WAITING_EPS_ANIME_1,	// 8: アニメーション領域
};


/**
 *	駐車券挿入案内、料金案内用ベース画面 のテキスト区画番号
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                  大見出し領域(Text1)                  |	<-- 金額表示領域(Text2)
 *		|                                                       |	<-- 32x32の大見出し(Text8)
 *		+-------------------------------------------------------+
 *		|                    中見出し(Text3)                    |
 *		+-----------------------+-------------------------------+
 *		|                       |     メッセージ領域(Text10)    |  <-- 明細1（Text14）
 *		|                       +-------------------------------+  <-- 明細2（Text15）
 *		|                       |     メッセージ領域(Text11)    |  <-- 明細3（Text16）
 *		|                       +-------------------------------+  <-- 明細4（Text17）
 *		|                       |     メッセージ領域(Text12)    |  <-- 明細5（Text18）
 *		|                       +-------------------------------+  <-- 明細6（Text19）
 *		|                       |     メッセージ領域(Text13)    |  <-- 明細7（Text20）
 *		|                       +-------------------------------+  <-- 明細8（Text21）
 *		|                       |                               |  <-- 明細9（Text22）
 *		|                       |                               |
 *		|                       |                               |
 *		|                       |                               |
 *		|                       |                               |
 *		+-----------------------+-------+-----------------------+
 *		|   駐車時間 (Text7)    |(Text9)|      時計(Text6)      |  <-- つり切れ表示（Text5=Text9+Text6の領域）
 *		+-----------------------+-------+-----------------------+  <-- Text9の位置は仮（2010/01/14）
 *		|                     最下段(Text4)                     |
 *		+-------------------------------------------------------+
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__PAYING_00,			//  0: 未使用
	LCDBM_AREA_TEXT__PAYING_CAPTION_1,	//  1: [64x64]	大見出し
	LCDBM_AREA_TEXT__PAYING_FEE,		//  2: [64x64]	料金表示
	LCDBM_AREA_TEXT__PAYING_CAPTION_2,	//  3: [32x32]	中見出し
	LCDBM_AREA_TEXT__PAYING_CAPTION_3,	//  4: [24x24]	小見出し
	LCDBM_AREA_TEXT__PAYING_ALARM,		//  5: [24x24]	つり切れ
	LCDBM_AREA_TEXT__PAYING_CLOCK,		//  6: [24x24]	時計
	LCDBM_AREA_TEXT__PAYING_TIME,		//  7: [24x24]	入庫時刻／駐車時間
	LCDBM_AREA_TEXT__PAYING_CAPTION_4,	//  8: [36x36]	大見出し
	LCDBM_AREA_TEXT__PAYING_09,			//  9: 未使用

	// モジュールアニメ
	LCDBM_AREA_TEXT__ANIME_COMMENT_1,	// 10: [24x24]	メッセージ（１行目）
	LCDBM_AREA_TEXT__ANIME_COMMENT_2,	// 11: [24x24]	メッセージ（２行目）
	LCDBM_AREA_TEXT__ANIME_COMMENT_3,	// 12: [24x24]	メッセージ（３行目）
	LCDBM_AREA_TEXT__ANIME_COMMENT_4,	// 13: [24x24]	メッセージ（４行目）

	// 料金内訳
	LCDBM_AREA_TEXT__PAYING_DETAILS_1,	// 14: [24x24]	明細1（明細1-9は連番の必要あり）
	LCDBM_AREA_TEXT__PAYING_DETAILS_2,	// 15: [24x24]	明細2
	LCDBM_AREA_TEXT__PAYING_DETAILS_3,	// 16: [24x24]	明細3
	LCDBM_AREA_TEXT__PAYING_DETAILS_4,	// 17: [24x24]	明細4
	LCDBM_AREA_TEXT__PAYING_DETAILS_5,	// 18: [24x24]	明細5
	LCDBM_AREA_TEXT__PAYING_DETAILS_6,	// 19: [24x24]	明細6
	LCDBM_AREA_TEXT__PAYING_DETAILS_7,	// 20: [24x24]	明細7
	LCDBM_AREA_TEXT__PAYING_DETAILS_8,	// 21: [24x24]	明細8
	LCDBM_AREA_TEXT__PAYING_DETAILS_9,	// 22: [24x24]	明細9

	// 明細表示用の行数
	LCDBM_AREA_TEXT__PAYING_DETAILS_NUM =	9,
};


/**
 *	駐車券挿入案内、料金案内用ベース画面 のグラフィック区画番号
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                 静止画領域(Graphic1)                  |
 *		|                                                       |
 *		+-----------------------+-------------------------------+
 *		|                       |                               |	<-- 料金内訳領域(Graphic6) ※Graphic4とGraphic5を合わせた領域
 *		|                       |     静止画領域(Graphic4)      |
 *		|                       |                               |
 *		|  アニメーション領域１ +-------------------------------+
 *		|      (Graphic3)       |                               |
 *		|                       |     アニメーション領域２      |
 *		|                       |         (Graphic5)            |
 *		|                       |                               |
 *		|                       |                               |
 *		+-----------------------+-------------------------------+
 *		|                 静止画領域(Graphic2)                  |
 *		|                                                       |
 *		+-------------------------------------------------------+
 *	@endcode
 */
enum {
	LCDBM_AREA_GRAPHIC__PAYING_00,				// 0: 未使用
	LCDBM_AREA_GRAPHIC__PAYING_CAPTION_1,		// 1: 大見出し＋中見出し
	LCDBM_AREA_GRAPHIC__PAYING_CAPTION_3,		// 2: 最下段＋時計
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_1,			// 3: アニメーション領域１
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_COMMENT,	// 4: アニメーション領域２に連動したメッセージ
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_2,			// 5: アニメーション領域２
	LCDBM_AREA_GRAPHIC__PAYING_DETAIL,			// 6: 料金内訳
	LCDBM_AREA_GRAPHIC__PAYING_GOODBYE,			// 7: 精算完了後の出庫督促アニメーション
};


/**
 *	精算完了画面のテキスト区画番号
 *	（精算中画面と統合したため不要だが、分ける必要が生じた場合の備えて残しておく）
 */
enum {
	LCDBM_AREA_TEXT__PAID_00 =			LCDBM_AREA_TEXT__PAYING_00,			//  0: 未使用
	LCDBM_AREA_TEXT__PAID_CAPTION_1 =	LCDBM_AREA_TEXT__PAYING_CAPTION_1,	//  1: [64x64]	大見出し
	LCDBM_AREA_TEXT__PAID_FEE =			LCDBM_AREA_TEXT__PAYING_FEE,		//  2: [64x64]	料金表示
	LCDBM_AREA_TEXT__PAID_CAPTION_2 =	LCDBM_AREA_TEXT__PAYING_CAPTION_2,	//  3: [32x32]	中見出し
	LCDBM_AREA_TEXT__PAID_CAPTION_3 =	LCDBM_AREA_TEXT__PAYING_CAPTION_3,	//  4: [24x24]	小見出し
	LCDBM_AREA_TEXT__PAID_ALARM =		LCDBM_AREA_TEXT__PAYING_ALARM,		//  5: [24x24]	つり切れ
	LCDBM_AREA_TEXT__PAID_CLOCK =		LCDBM_AREA_TEXT__PAYING_CLOCK,		//  6: [24x24]	時計
	LCDBM_AREA_TEXT__PAID_TIME =		LCDBM_AREA_TEXT__PAYING_TIME,		//  7: [24x24]	入庫時刻／駐車時間
	LCDBM_AREA_TEXT__PAID_CAPTION_4 =	LCDBM_AREA_TEXT__PAYING_CAPTION_4,	//  8: [36x36]	大見出し
	LCDBM_AREA_TEXT__PAID_09 =			LCDBM_AREA_TEXT__PAYING_09,			//  9: 未使用
	LCDBM_AREA_TEXT__PAID_10 =			10,									// 10: 未使用
	LCDBM_AREA_TEXT__PAID_11 =			11,									// 11: 未使用
	LCDBM_AREA_TEXT__PAID_12 =			12,									// 12: 未使用
	LCDBM_AREA_TEXT__PAID_13 =			13,									// 13: 未使用

	// 料金内訳
	LCDBM_AREA_TEXT__PAID_DETAILS_1 =	LCDBM_AREA_TEXT__PAYING_DETAILS_1,	// 14: [24x24]	明細1（明細1-9は連番の必要あり）
	LCDBM_AREA_TEXT__PAID_DETAILS_2 =	LCDBM_AREA_TEXT__PAYING_DETAILS_2,	// 15: [24x24]	明細2
	LCDBM_AREA_TEXT__PAID_DETAILS_3 =	LCDBM_AREA_TEXT__PAYING_DETAILS_3,	// 16: [24x24]	明細3
	LCDBM_AREA_TEXT__PAID_DETAILS_4 =	LCDBM_AREA_TEXT__PAYING_DETAILS_4,	// 17: [24x24]	明細4
	LCDBM_AREA_TEXT__PAID_DETAILS_5 =	LCDBM_AREA_TEXT__PAYING_DETAILS_5,	// 18: [24x24]	明細5
	LCDBM_AREA_TEXT__PAID_DETAILS_6 =	LCDBM_AREA_TEXT__PAYING_DETAILS_6,	// 19: [24x24]	明細6
	LCDBM_AREA_TEXT__PAID_DETAILS_7 =	LCDBM_AREA_TEXT__PAYING_DETAILS_7,	// 20: [24x24]	明細7
	LCDBM_AREA_TEXT__PAID_DETAILS_8 =	LCDBM_AREA_TEXT__PAYING_DETAILS_8,	// 21: [24x24]	明細8
	LCDBM_AREA_TEXT__PAID_DETAILS_9 =	LCDBM_AREA_TEXT__PAYING_DETAILS_9,	// 22: [24x24]	明細9
};


/**
 *	精算完了画面のグラフィック区画番号
 *	（精算中画面と統合したため不要だが、分ける必要が生じた場合の備えて残しておく）
 */
enum {
	LCDBM_AREA_GRAPHIC__PAID_00 =				LCDBM_AREA_GRAPHIC__PAYING_00,				// 0: 未使用
	LCDBM_AREA_GRAPHIC__PAID_CAPTION_1 =		LCDBM_AREA_GRAPHIC__PAYING_CAPTION_1,		// 1: 大見出し＋中見出し
	LCDBM_AREA_GRAPHIC__PAID_CAPTION_3 =		LCDBM_AREA_GRAPHIC__PAYING_CAPTION_3,		// 2: 最下段＋時計
	LCDBM_AREA_GRAPHIC__PAID_ANIME_1 =			LCDBM_AREA_GRAPHIC__PAYING_ANIME_1,			// 3: 全面パネルアニメーション
	LCDBM_AREA_GRAPHIC__PAID_ANIME_COMMENT =	LCDBM_AREA_GRAPHIC__PAYING_ANIME_COMMENT,	// 4: 未使用
	LCDBM_AREA_GRAPHIC__PAID_ANIME_2 =			LCDBM_AREA_GRAPHIC__PAYING_ANIME_2,			// 5: 未使用
	LCDBM_AREA_GRAPHIC__PAID_DETAIL =			LCDBM_AREA_GRAPHIC__PAYING_DETAIL,			// 6: 利用明細
	LCDBM_AREA_GRAPHIC__PAID_GOODBYE =			LCDBM_AREA_GRAPHIC__PAYING_GOODBYE,			// 7: 精算完了後の出庫督促アニメーション
};


/**
 *	休業中画面のテキスト区画番号
 *
 *	@code
 *	┏━━━━━━━━━━━━━━━━━━┓
 *	┃        [  TextArea01  ][TextArea02]┃
 *	┃                                    ┃
 *	┃[            TextArea03            ]┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃[      TextArea05      ][TextArea06]┃
 *	┗━━━━━━━━━━━━━━━━━━┛
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__BREAK_00,	// 00: 未使用
	LCDBM_AREA_TEXT__BREAK_01,	// 01: 大見出し			[64x64]
	LCDBM_AREA_TEXT__BREAK_02,	// 02: 休業理由
	LCDBM_AREA_TEXT__BREAK_03,	// 03: 中見出し			[32x32]
	LCDBM_AREA_TEXT__BREAK_04,	// 04: 未使用
	LCDBM_AREA_TEXT__BREAK_05,	// 05: インジケーター	[24x24]
	LCDBM_AREA_TEXT__BREAK_06,	// 06: 時計				[24x24]
};


/**
 *	休業中画面のグラフィック区画番号
 *
 *	@code
 *	┏━━━━━━━━━━━━━━━━━━┓
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃           GraphicArea01            ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┗━━━━━━━━━━━━━━━━━━┛
 *	@endcode
 */
enum {
	/*
	 *	末尾に"_NOT_UPDATE"が付いている定義名はコマンドによる書き換えを行わない領域である。
	 */
	LCDBM_AREA_GRAPHIC__BREAK_00_NOT_UPDATE,	// 00: 未使用
	LCDBM_AREA_GRAPHIC__BREAK_01_NOT_UPDATE,	// 01: 背景(全画面)
};


/**
 *	警告画面のテキスト区画番号
 *
 *	@code
 *	┏━━━━━━━━━━━━━━━━━━┓
 *	┃          [  TextArea01  ]          ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃[            TextArea10            ]┃
 *	┃[            TextArea11            ]┃
 *	┃[            TextArea12            ]┃
 *	┃[            TextArea13            ]┃
 *	┃[            TextArea14            ]┃
 *	┃[            TextArea15            ]┃
 *	┃[            TextArea16            ]┃
 *	┃[            TextArea17            ]┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃[      TextArea05      ][TextArea06]┃
 *	┗━━━━━━━━━━━━━━━━━━┛
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__WARNING_00,	// 00: 未使用
	LCDBM_AREA_TEXT__WARNING_01,	// 01: 大見出し			[64x64]
	LCDBM_AREA_TEXT__WARNING_02,	// 02: 未使用
	LCDBM_AREA_TEXT__WARNING_03,	// 03: 未使用
	LCDBM_AREA_TEXT__WARNING_04,	// 04: 未使用
	LCDBM_AREA_TEXT__WARNING_05,	// 05: インジケーター	[24x24]
	LCDBM_AREA_TEXT__WARNING_06,	// 06: 時計				[24x24]
	LCDBM_AREA_TEXT__WARNING_07,	// 07: 未使用
	LCDBM_AREA_TEXT__WARNING_08,	// 08: 未使用
	LCDBM_AREA_TEXT__WARNING_09,	// 09: 未使用
	LCDBM_AREA_TEXT__WARNING_10,	// 10: 警告内容①
	LCDBM_AREA_TEXT__WARNING_11,	// 11: 警告内容②
	LCDBM_AREA_TEXT__WARNING_12,	// 12: 警告内容③
	LCDBM_AREA_TEXT__WARNING_13,	// 13: 警告内容④
	LCDBM_AREA_TEXT__WARNING_14,	// 14: 警告内容⑤
	LCDBM_AREA_TEXT__WARNING_15,	// 15: 警告内容⑥
	LCDBM_AREA_TEXT__WARNING_16,	// 16: 警告内容⑦
	LCDBM_AREA_TEXT__WARNING_17,	// 17: 警告内容⑧
};


/**
 *	警告画面のグラフィック区画番号
 *
 *	@code
 *	┏━━━━━━━━━━━━━━━━━━┓
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃           GraphicArea01            ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┃                                    ┃
 *	┗━━━━━━━━━━━━━━━━━━┛
 *	@endcode
 */
enum {
	/*
	 *	末尾に"_NOT_UPDATE"が付いている定義名はコマンドによる書き換えを行わない領域である。
	 */
	LCDBM_AREA_GRAPHIC__WARNING_00_NOT_UPDATE,	// 00: 未使用
	LCDBM_AREA_GRAPHIC__WARNING_01_NOT_UPDATE,	// 01: 背景(全画面)
};


/**
 *	釣銭補充画面のテキスト区画番号
 */
enum {
	LCDBM_AREA_TEXT__REFILL_00,					// 00: 未使用
	LCDBM_AREA_TEXT__REFILL_CAPTION_1,			// 01: [64x64]	大見出し
	LCDBM_AREA_TEXT__REFILL_CAPTION_2,			// 02: [36x36]	中見出し
	LCDBM_AREA_TEXT__REFILL_CHANGE,				// 03: [24x24]	釣銭金庫項目(紙幣投入口，硬貨投入口，コインホッパー)
	LCDBM_AREA_TEXT__REFILL_SAFE_TYPE1,			// 04: [24x24]	金庫項目(紙幣金庫，コイン金庫)
	LCDBM_AREA_TEXT__REFILL_SAFE_TYPE2,			// 05: [24x24]	金庫項目(紙幣金庫)
	LCDBM_AREA_TEXT__REFILL_COINMEC,			// 06: [36x36]	コインメック現在枚数
	LCDBM_AREA_TEXT__REFILL_IN_COIN,			// 07: [24x24]	コインメック入金枚数
	LCDBM_AREA_TEXT__REFILL_HOPPER,				// 08: [36x36]	コインホッパー現在枚数
	LCDBM_AREA_TEXT__REFILL_CSAFE_10,			// 09: [36x36]	コイン金庫10円現在枚数
	LCDBM_AREA_TEXT__REFILL_CSAFE_50,			// 10: [36x36]	コイン金庫50円現在枚数
	LCDBM_AREA_TEXT__REFILL_CSAFE_100,			// 11: [36x36]	コイン金庫100円現在枚数
	LCDBM_AREA_TEXT__REFILL_CSAFE_500,			// 12: [36x36]	コイン金庫500円現在枚数
	LCDBM_AREA_TEXT__REFILL_CASSETTE1,			// 13: [36x36]	紙幣払出カセット1現在枚数
	LCDBM_AREA_TEXT__REFILL_CASSETTE2,			// 14: [36x36]	紙幣払出カセット2現在枚数
	LCDBM_AREA_TEXT__REFILL_NTCYCL,				// 15: [36x36]	紙幣循環機1000円現在枚数
	LCDBM_AREA_TEXT__REFILL_IN_NOTE,			// 16: [36x36]	紙幣循環機1000円入金枚数
	LCDBM_AREA_TEXT__REFILL_NSAFE_1000,			// 17: [36x36]	紙幣金庫1000円現在枚数
	LCDBM_AREA_TEXT__REFILL_NSAFE_2000,			// 18: [36x36]	紙幣金庫2000円現在枚数
	LCDBM_AREA_TEXT__REFILL_NSAFE_5000,			// 19: [36x36]	紙幣金庫5000円現在枚数
	LCDBM_AREA_TEXT__REFILL_NSAFE_10000,		// 20: [36x36]	紙幣金庫10000円現在枚数
};


/**
 *	釣銭補充画面のグラフィック区画番号
 */
enum {
	LCDBM_AREA_GRAPHIC__REFILL_00,				// 00: 未使用
	LCDBM_AREA_GRAPHIC__REFILL_BACKGROUND,		// 01: 背景
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_500,		// 02: コインメック500円筒
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_100,		// 03: コインメック100円筒
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_10,		// 04: コインメック10円筒
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_50,		// 05: コインメック50円筒
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_SUB,		// 06: コインメックSUB筒
	LCDBM_AREA_GRAPHIC__REFILL_HOPPER1,			// 07: コインホッパー1
	LCDBM_AREA_GRAPHIC__REFILL_HOPPER2,			// 08: コインホッパー2
	LCDBM_AREA_GRAPHIC__REFILL_CASSETTE1,		// 09: 紙幣払出機カセット1
	LCDBM_AREA_GRAPHIC__REFILL_CASSETTE2,		// 10: 紙幣払出機カセット2
	LCDBM_AREA_GRAPHIC__REFILL_NTCYCL,			// 11: 紙幣循環機1000円収納部
	LCDBM_AREA_GRAPHIC__REFILL_COIN_ANIME,		// 12: コイン投入アニメーション
	LCDBM_AREA_GRAPHIC__REFILL_NOTE_ANIME,		// 13: 紙幣挿入アニメーション
};


/**
 *	メンテナンス画面
 *	（背景画像、画面レイアウトは休業中の使いまわし）
 */
enum {
	LCDBM_AREA_TEXT__MAINTENANCE_00,		// 00: 未使用
	LCDBM_AREA_TEXT__MAINTENANCE_CAPTION_1,	// 01: [64x64]	大見出し
	LCDBM_AREA_TEXT__MAINTENANCE_02,		// 02: 未使用
	LCDBM_AREA_TEXT__MAINTENANCE_CAPTION_2,	// 03: [36x36]	中見出し
	LCDBM_AREA_TEXT__MAINTENANCE_04,		// 04: 未使用
	LCDBM_AREA_TEXT__MAINTENANCE_05,		// 05: 未使用
	LCDBM_AREA_TEXT__MAINTENANCE_CLOCK,		// 06: [24x24]	時計
};
enum {
	LCDBM_AREA_GRAPHIC__MAINTENANCE_00,		// 00: 未使用
	LCDBM_AREA_GRAPHIC__MAINTENANCE_01,		// 01: 背景
};


//--------------------------------------------------
//		精算機パネル 画像番号
//--------------------------------------------------
#define	PANEL_PATTERN_NUM	50									// パネル絵のパターン数
#define	PANEL_INCREMENT	(PANEL_PATTERN_NUM + PANEL_PATTERN_NUM)	// パネル絵のベース番号増分（LCD消灯グラフィック枚数＋LED点灯グラフィック枚数）

/**
 *	精算機パネルのオプション別 オフセット番号
 */
enum {
	LCDBM_IMAGE_PANEL__OFFSET_MIFARE =											PANEL_INCREMENT,	// Mifare
	LCDBM_IMAGE_PANEL__OFFSET_SUICA =		LCDBM_IMAGE_PANEL__OFFSET_MIFARE +	PANEL_INCREMENT,	// Suica
	LCDBM_IMAGE_PANEL__OFFSET_EDY =			LCDBM_IMAGE_PANEL__OFFSET_SUICA +	PANEL_INCREMENT,	// Edy
	LCDBM_IMAGE_PANEL__OFFSET_WAON =		LCDBM_IMAGE_PANEL__OFFSET_EDY +		PANEL_INCREMENT,	// WAON
	LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE =	LCDBM_IMAGE_PANEL__OFFSET_WAON +	PANEL_INCREMENT,	// 乗車履歴
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE1 =	LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE +PANEL_INCREMENT,	// 予備
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE2 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE1 +PANEL_INCREMENT,	// 予備
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE3 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE2 +PANEL_INCREMENT,	// 予備
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE4 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE3 +PANEL_INCREMENT,	// 予備
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE5 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE4 +PANEL_INCREMENT,	// 予備
};


/**
 *	精算機前面パネル番号
 *	@note
 *	-	精算中の画面左側に表示する
 *	-	CPS、EPSは共通で同じ絵を使う
 */
enum {
	// 精算機パネル絵 ベース番号
	LCDBM_IMAGE_PANEL__BASE =	1000,

	// GT-7700 紙幣払出機なし
	LCDBM_IMAGE_PANEL__GT7700_NORMAL =			LCDBM_IMAGE_PANEL__BASE,													//
	LCDBM_IMAGE_PANEL__GT7700_MIFARE =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,		// Mifare
	LCDBM_IMAGE_PANEL__GT7700_SUICA =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,			// Suica
	LCDBM_IMAGE_PANEL__GT7700_EDY =				LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,			// Edy
	LCDBM_IMAGE_PANEL__GT7700_WAON =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,			// WAON
	LCDBM_IMAGE_PANEL__GT7700_PARKRIDE =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,		// 乗車履歴
	LCDBM_IMAGE_PANEL__GT7700_RESERVE1 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,		// 予備
	LCDBM_IMAGE_PANEL__GT7700_RESERVE2 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,		// 予備
	LCDBM_IMAGE_PANEL__GT7700_RESERVE3 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,		// 予備
	LCDBM_IMAGE_PANEL__GT7700_RESERVE4 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,		// 予備
	LCDBM_IMAGE_PANEL__GT7700_RESERVE5 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,		// 予備

	// GT-7700 紙幣払出機あり
	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL =		LCDBM_IMAGE_PANEL__GT7700_RESERVE5 + PANEL_INCREMENT,						//
	LCDBM_IMAGE_PANEL__GT7700_BDU_MIFARE =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,	// Mifare
	LCDBM_IMAGE_PANEL__GT7700_BDU_SUICA =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,		// Suica
	LCDBM_IMAGE_PANEL__GT7700_BDU_EDY =			LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,		// Edy
	LCDBM_IMAGE_PANEL__GT7700_BDU_WAON =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,		// WAON
	LCDBM_IMAGE_PANEL__GT7700_BDU_PARKRIDE =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,	// 乗車履歴
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE1 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,	// 予備
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE2 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,	// 予備
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE3 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,	// 予備
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE4 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,	// 予備
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE5 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,	// 予備

	// GT-7750 紙幣循環機あり
	LCDBM_IMAGE_PANEL__GT7750_NORMAL =			LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE5 + PANEL_INCREMENT,					//
	LCDBM_IMAGE_PANEL__GT7750_MIFARE =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,		// Mifare
	LCDBM_IMAGE_PANEL__GT7750_SUICA =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,			// Suica
	LCDBM_IMAGE_PANEL__GT7750_EDY =				LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,			// Edy
	LCDBM_IMAGE_PANEL__GT7750_WAON =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,			// WAON
	LCDBM_IMAGE_PANEL__GT7750_PARKRIDE =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,		// 乗車履歴
	LCDBM_IMAGE_PANEL__GT7750_RESERVE1 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,		// 予備
	LCDBM_IMAGE_PANEL__GT7750_RESERVE2 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,		// 予備
	LCDBM_IMAGE_PANEL__GT7750_RESERVE3 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,		// 予備
	LCDBM_IMAGE_PANEL__GT7750_RESERVE4 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,		// 予備
	LCDBM_IMAGE_PANEL__GT7750_RESERVE5 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,		// 予備
};


/**
 *	精算機前面パネルの○印パターン
 *	@note
 *	-	LCDBM_IMAGE_PANEL_* で定義される値に加算してグラフィック番号を算出する
 *	-	LED消灯グラフィックとLED点灯グラフィックを組み合わせてLED点滅アニメーションを構成する
 */
enum {
	// グラフィックパターンファイル番号を指定する場合にはパネル絵のベース番号にこの値を加算する
	LCDBM_IMAGE_PANEL_NO__NOTHING,			//  0: ○印なし
	LCDBM_IMAGE_PANEL_NO__TICKET_IN,		//  1: 駐車券投入口
	LCDBM_IMAGE_PANEL_NO__COIN_IN,			//  2: 硬貨投入口
	LCDBM_IMAGE_PANEL_NO__NOTE_IN,			//  3: 紙幣投入口
	LCDBM_IMAGE_PANEL_NO__TICKET_OUT,		//  4: 駐車券返却口
	LCDBM_IMAGE_PANEL_NO__COIN_OUT,			//  5: 硬貨・領収証返却口
	LCDBM_IMAGE_PANEL_NO__NOTE_OUT,			//  6: 紙幣返却口
	LCDBM_IMAGE_PANEL_NO__ICCARD1,			//  7: ICカードリーダー（残額照会）
	LCDBM_IMAGE_PANEL_NO__ICCARD2,			//  8: ICカードリーダー（決済）
	LCDBM_IMAGE_PANEL_NO__ICCARD3,			//  9: 乗車履歴リーダー
};


//--------------------------------------------------
//		モジュールアニメ 画像番号
//--------------------------------------------------
/**
 *	モジュールアニメ
 *	@note
 *	-	精算中の画面右側に表示する
 */
enum {
	LCDBM_IMAGE_MODULE__ANIME_COMMENT =	15000,	// モジュールアニメ解説テキスト背景
	LCDBM_IMAGE_MODULE__START =	10000,			// モジュールアニメのベース画像番号

	/*
	 *	磁気リーダ
	 *
	 *	10000:磁気リーダ 使用不可
	 *	10001:磁気カード 挿入
	 *	10002:磁気カード 返却
	 */
	LCDBM_IMAGE_MODULE__MAG_READER_NG =		LCDBM_IMAGE_MODULE__START,
	LCDBM_IMAGE_MODULE__MAG_READER_IN,
	LCDBM_IMAGE_MODULE__MAG_READER_OUT,

	/*
	 *	サイバネリーダ
	 *
	 *	10010:サイバネリーダ 使用不可
	 *	10011:サイバネカード 挿入
	 *	10012:サイバネカード 返却
	 */
	LCDBM_IMAGE_MODULE__CYBERNE_READER_NG =	LCDBM_IMAGE_MODULE__START + 10,
	LCDBM_IMAGE_MODULE__CYBERNE_READER_IN,
	LCDBM_IMAGE_MODULE__CYBERNE_READER_OUT,

	/*
	 *	硬貨 入金
	 *
	 *	10020:硬貨 入金不可
	 *	10021:硬貨 入金
	 *	10022:硬貨 入金不可（まとめ入れ）
	 *	10023:硬貨 入金（まとめ入れ）
	 */
	LCDBM_IMAGE_MODULE__COIN_READER_NG =	LCDBM_IMAGE_MODULE__START + 20,
	LCDBM_IMAGE_MODULE__COIN_READER_IN,
// LH068007 機能アップ (S) 2011/04/15(金) <Yoshiaki_Hamada@amano.co.jp> コインまとめ入れグラフィック追加
	LCDBM_IMAGE_MODULE__COIN_READER_2_NG,
	LCDBM_IMAGE_MODULE__COIN_READER_2_IN,
// LH068007 機能アップ (E) 2011/04/15(金) <Yoshiaki_Hamada@amano.co.jp> コインまとめ入れグラフィック追加

	/*
	 *	紙幣リーダー 入金
	 *
	 *	10030:紙幣リーダー 入金不可
	 *	10031:紙幣リーダー 入金
	 *	10032:紙幣リーダー 返却不可
	 *	10033:紙幣リーダー 返却
	 */
	LCDBM_IMAGE_MODULE__NOTE_READER_NG =	LCDBM_IMAGE_MODULE__START + 30,
	LCDBM_IMAGE_MODULE__NOTE_READER_IN,
	LCDBM_IMAGE_MODULE__NOTE_READER_NG2,
	LCDBM_IMAGE_MODULE__NOTE_READER_OUT,

	/*
	 *	紙幣払い出し機 出金
	 *
	 *	10040:紙幣払い出し機 出金不可
	 *	10041:紙幣払い出し機 出金
	 */
	LCDBM_IMAGE_MODULE__NOTE_PAYOUT_NG =	LCDBM_IMAGE_MODULE__START + 40,
	LCDBM_IMAGE_MODULE__NOTE_PAYOUT_OUT,

	/*
	 *	紙幣循環機 入金
	 *
	 *	10050:紙幣循環機 入金不可
	 *	10051:紙幣循環機 入金
	 */
	LCDBM_IMAGE_MODULE__BRU_READER_NG =		LCDBM_IMAGE_MODULE__START + 50,
	LCDBM_IMAGE_MODULE__BRU_READER_IN,

	/*
	 *	紙幣循環機 出金
	 *
	 *	10060:紙幣循環機 出金不可
	 *	10061:紙幣循環機 出金
	 */
	LCDBM_IMAGE_MODULE__BRU_PAYOUT_NG =		LCDBM_IMAGE_MODULE__START + 60,
	LCDBM_IMAGE_MODULE__BRU_PAYOUT_OUT,

	/*
	 *	Mifare
	 *
	 *	10070:Mifareリーダ 利用不可
	 *	10071:Mifareリーダかざす
	 */
	LCDBM_IMAGE_MODULE__MIFARE_NG =			LCDBM_IMAGE_MODULE__START + 70,
	LCDBM_IMAGE_MODULE__MIFARE_TOUCH,

	/*
	 *	Suica（PASMOも同じ絵を使う）
	 *
	 *	10080:Suicaリーダ 利用不可
	 *	10081:Suicaリーダかざす（決済）
	 *	10082:Suicaリーダかざす（残額照会）
	 */
	LCDBM_IMAGE_MODULE__SUICA_NG =			LCDBM_IMAGE_MODULE__START + 80,
	LCDBM_IMAGE_MODULE__SUICA_PAY,
	LCDBM_IMAGE_MODULE__SUICA_BALANCE,

	/*
	 *	PASMOはSuicaと同じ絵を使うので欠番
	 *	LCDBM_IMAGE_MODULE__PASMO =		LCDBM_IMAGE_MODULE__START + 90,
	 */

	/*
	 *	Edy
	 *
	 *	10100:Edyリーダ 利用不可
	 *	10101:Edyリーダかざす（決済）
	 *	10102:Edyリーダかざす（残額照会）
	 */
	LCDBM_IMAGE_MODULE__EDY_NG =			LCDBM_IMAGE_MODULE__START + 100,
	LCDBM_IMAGE_MODULE__EDY_PAY,
	LCDBM_IMAGE_MODULE__EDY_BALANCE,

	/*
	 *	WAON
	 *
	 *	10110:WAONリーダ 利用不可
	 *	10111:WAONリーダかざす（決済）
	 *	10112:WAONリーダかざす（残額照会）
	 */
	LCDBM_IMAGE_MODULE__WAON_NG =			LCDBM_IMAGE_MODULE__START + 110,
	LCDBM_IMAGE_MODULE__WAON_PAY,
	LCDBM_IMAGE_MODULE__WAON_BALANCE,

	/*
	 *	乗車履歴リーダー（ボタンつき）
	 *
	 *	10120:乗車履歴リーダー 利用不可
	 *	10121:乗車履歴リーダーにかざす
	 */
	LCDBM_IMAGE_MODULE__PARKRIDE1_NG =		LCDBM_IMAGE_MODULE__START + 120,
	LCDBM_IMAGE_MODULE__PARKRIDE1_TOUCH,

	/*
	 *	(乗車履歴)Suicaリーダー（ボタンつき）
	 *
	 *	10130:乗車履歴ボタンを押す
	 *	10131:電子マネーボタンを押す
	 */
	LCDBM_IMAGE_MODULE__PARKRIDE2_NG =		LCDBM_IMAGE_MODULE__START + 130,
	LCDBM_IMAGE_MODULE__PARKRIDE2_TOUCH,

	/*
	 *	コイン返金
	 *
	 *	10140:コイン返金不可（実際にはあり得ないので番号予約とする）
	 *	10141:コイン返金
	 */
	LCDBM_IMAGE_MODULE__COIN_PAYOUT_NG =	LCDBM_IMAGE_MODULE__START + 140,
	LCDBM_IMAGE_MODULE__COIN_PAYOUT_OUT,

	/*
	 *	領収証・預り証
	 *
	 *	10150:領収証・預り証 印刷不可（不要と考えるので番号予約とする）
	 *	10151:領収証・預り証 印刷（領収証取出し口）
	 */
	LCDBM_IMAGE_MODULE__RECEIPT_NG =		LCDBM_IMAGE_MODULE__START + 150,
	LCDBM_IMAGE_MODULE__RECEIPT_OUT,
};


//--------------------------------------------------
//		グラフィックパターン
//--------------------------------------------------
/**
 *	グラフィック表示切替コマンドで指定するグラフィックパターン番号
 */
enum {
	LCDBM_IMAGE_PATTERN__PAID_GOODBYE = 20440,	// 精算完了後の出庫督促アニメーション
};


//--------------------------------------------------
//		テキスト
//--------------------------------------------------
/** モジュールアニメのメッセージの行数 */
#define	LCDBM_AREA_TEXT__ANIME_COMMENT_NUM	( LCDBM_AREA_TEXT__ANIME_COMMENT_4 - LCDBM_AREA_TEXT__ANIME_COMMENT_1 + 1 )


/**
 *	テキスト番号
 */
enum {
	//--------------------------------------------------
	//		特殊
	//--------------------------------------------------
	LCDBM_FILE_TEXT__NOTHING =	0,					//	0: 何も表示しない（表示しているテキストを消去する時に使う）


	LCDBM_FILE_TEXT__CARD_NAME_2 =			100,	/*	券名称2（駐車券の名称）
													 *	+0:	"駐車券"
													 *	+1:	"駐車カード"
													 *	+2:	"駐車整理券"
													 *	+3:	"整理券"
													 *	+4:	"当日利用券"
													 *	+5:	"一時利用券"
													 *	+6:	"一日利用券"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_3 =			200,	/*	券名称3（定期券の名称）
													 *	+0:	"定期券"
													 *	+1:	"定期利用券"
													 *	+2:	"パスカード"
													 *	+3:	"契約カード"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_4 =			300,	/*	券名称4（サービス券／掛売券の名称）
													 *	+0:	"サービス券"
													 *	+1:	"割引券"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5 =			400,	/*	券名称5（精算で決済前に入れるように促す券種）
													 *	+0:	"プリペイドカード"
													 *	+1:	"回数券"
													 *	+2:	"クレジットカード"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_ICCARD =		500,	/*	電子カード系
													 *	+0:	"Ｅｄｙ"
													 */


	//--------------------------------------------------
	//		各文字列テーブルに対応する番号
	//	（プログラム内部に固定データとして持っている文字列に対応）
	//--------------------------------------------------
	LCDBM_FILE_TEXT__OPE_CHR =				10000,	// OPE_CHR[0]
	LCDBM_FILE_TEXT__OPE_CHR_G =			10200,	// OPE_CHR_G[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_MSG	=	10400,	// OPE_CHR_G_SALE_MSG[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_MSG2 =	10600,	// OPE_CHR_G_SALE_MSG2[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_28 =	10800,	// OPE_CHR_G_SALE_28[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_8 =		11000,	// OPE_CHR_G_SALE_8[0]
	LCDBM_FILE_TEXT__CLSMSG =				11200,	// CLSMSG[0]
	LCDBM_FILE_TEXT__DRCLS_ALM =			11400,	// DRCLS_ALM[0]

	// 利用可能媒体名称（24-0031～24-0034）
	LCDBM_FILE_TEXT__OPE_CHR_CYCLIC_MSG =	11600,	// OPE_CHR_CYCLIC_MSG[0]

	// 料金内訳
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_0 =		11800,	// [0]  "駐車料金　　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_1,				// [1]  "投入金額　　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_2,				// [2]  "割引金額　　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_3,				// [3]  "プリペイドカード"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_4,				// [4]  "回数券　　　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_5,				// [5]  "電子マネー　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_6,				// [6]  "前回支払額　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_7,				// [7]  "クレジットカード"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_8,				// [8]  "おつり　　　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_9,				// [9]  "　　　　　（残額"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_10,				// [10] "　　　　（残度数"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_11,				// [11] "円  "
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_12,				// [12] "円）"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_13,				// [13] "回）"
// GW-861011 カラーLCDのEdy対応 (S) 2010/07/29(木) <Yoshiaki_Hamada@amano.co.jp> Edy対応
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_14,				// [14] "Ｅｄｙ支払　　　"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_15,				// [15] "Ｅｄｙ残高　　　"
// GW-861011 カラーLCDのEdy対応 (E) 2010/07/29(木) <Yoshiaki_Hamada@amano.co.jp> Edy対応


	//--------------------------------------------------
	//		ポップアップ
	//--------------------------------------------------
	LCDBM_FILE_TEXT__POPUP =	30000,				// ポップアップに表示するテキスト

	LCDBM_FILE_TEXT__POPUP_LINEFEED =	30000,		/*	改行用テキスト
													 *	+1:	１行改行したい場合
													 *	+2:	２行改行したい場合
													 *	+2:	３行改行したい場合
													 */
	LCDBM_FILE_TEXT__POPUP_EXCEPTION =	30010,		// ポップアップ例外表示に使うテキスト

	LCDBM_FILE_TEXT__POPUP_PASS_CPS =	30211,		/*	CPSのために定期券での精算が出来ずにエラーとなった場合の案内(43-0064)
													 *	+0:	"は出口にてご利用ください"
													 *	+1:	"出口精算機で精算してください"
													 *	+2:	"受付においでください"
													 */


	//--------------------------------------------------
	//		その他の標準ソフト用データ
	//--------------------------------------------------
	LCDBM_FILE_TEXT__UNIVERSAL_HA =		40000,		// 40000: "は"
	LCDBM_FILE_TEXT__UNIVERSAL_TADAIMA,				// 40001: "只今、"
	LCDBM_FILE_TEXT__UNIVERSAL_KONO,				// 40002: "この"
	LCDBM_FILE_TEXT__UNIVERSAL_HAVE,				// 40003: "をお持ちの方は"
	LCDBM_FILE_TEXT__UNIVERSAL_TOUCH,				// 40004: "タッチしてください"
	LCDBM_FILE_TEXT__UNIVERSAL_NOT_USE,				// 40005: "ご利用できません"
	LCDBM_FILE_TEXT__UNIVERSAL_MIDDLE_DOT,			// 40006: "・"
	LCDBM_FILE_TEXT__UNIVERSAL_MON,					// 40007: "月"
	LCDBM_FILE_TEXT__UNIVERSAL_TOUTEN,				// 40008: "、"

	LCDBM_FILE_TEXT__CANCEL_CAPTION_1 =	40100,		// 大見出し "と　り　け　し"
	LCDBM_FILE_TEXT__TICKET_TIME =		40101,		/*	精算中の磁気カード情報を表示する時間データの前に付加する
													 *	+0: "入庫時刻 "
													 *	+1: "前回精算 "
													 *	+2: "駐車時間 "
													 */
	LCDBM_FILE_TEXT__NOTICE_CHANGE =	40104,		// "取り忘れにご注意ください"
	LCDBM_FILE_TEXT__YOU_CAN_PAY =		40105,		// "でお支払いできます"
	LCDBM_FILE_TEXT__GET_PLEASE =		40106,		// "をお取りください"

	LCDBM_FILE_TEXT__PAY_MANUAL =		40107,		// "手動精算"

// GW861010 m.onouchi(S) 2010/07/26 LCDモジュール対応(車番認識)
	LCDBM_FILE_TEXT__MATCHING =			40108,		// "ありがとうございました"
// GW861010 m.onouchi(E) 2010/07/26 LCDモジュール対応(車番認識)

	LCDBM_FILE_TEXT__HEADLINE_L_2 =		40110,		/*	大見出し２（36x36）
													 *	+0: "料金は　　　　　　円です"
													 *	+1: "あと　　　　　　　円です"
													 *	+2: "おつりは　　　　　円です"
													 */
// GW861010 m.onouchi(S) 2010/07/26 LCDモジュール対応(車番認識)
	LCDBM_FILE_TEXT__VISITOR =			40113,		// "お客様の"(券名称2)／(券名称3)
	LCDBM_FILE_TEXT__CARD_NUM,						// "番号は"ｘｘｘｘｘ
	LCDBM_FILE_TEXT__DESU,							// "です"
// GW861010 m.onouchi(E) 2010/07/26 LCDモジュール対応(車番認識)
// LH068007(S) Hamada Yoshiaki 2011/05/11 CPSで「駐車券を入れて下さい」メッセージを設定可能とする
	LCDBM_FILE_TEXT__CPS_TICKET_INSERT =	40117,	/*	CPSの駐車券挿入待ちメッセージ
													 *	+0: "お帰りの方は、"
													 *	+1: "この精算機"
													 *	+2: "で精算してください"
													 *	+3: "お帰りの際に、"
													 */
// LH068007(E) Hamada Yoshiaki 2011/05/11 CPSで「駐車券を入れて下さい」メッセージを設定可能とする

	/*-----  設定で文言を変更するもの  -----*/
	LCDBM_FILE_TEXT__STAFF_NAME =			40500,	/*	係員名称（係員の表示名称）
													 *	+0: "係員"
													 *	+1: "従業員"
													 *	+2: "技術員"
													 *	+3: "管理者"
													 *	+4: "管理人"
													 *	+5:	"責任者"
													 */
	LCDBM_FILE_TEXT__OFFICE_NAME =			40510,	/*	管理室名称（管理室の表示名称）
													 *	+0: "管理室"
													 *	+1: "駐車場管理室"
													 *	+2: "管理ブース"
													 *	+3: "管理会社"
													 *	+4: "サービスカウンター"
													 *	+5:	"インフォメーションセンター"
													 */
	LCDBM_FILE_TEXT__ERROR_CARD_NAME =		40520,	/*	エラー時の券名称
													 *	+0: "券"
													 *	+1: "カード"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_1 =			40525,	/*	券名称2と券名称3を連結する接続詞
													 *	"又は"
													 */
	LCDBM_FILE_TEXT__PAYING_CAPTION_1 =		40550,	/*	駐車料金精算時の大見出しの文字列
													 *	+0:	"を入れてください"
													 *	+1:	"を1に入れてください"
													 *	+2:	"を矢印の方向に入れてください"
													 */
	LCDBM_FILE_TEXT__PAY_START_CAPTION_2 =	40560,	/*	駐車料金精算時の中見出しの文字列
													 *	+0:	表示なし
													 *	+1:	"をお持ちの方は"
													 *	+2:	"を入れてください"
													 *	+3:	"を紛失された方は"
													 *	+4:	"紛失ボタンを押してください"
													 *	+5:	"を更新される方は"
													 *	+6:	"更新ボタンを押してください"
													 */
	LCDBM_FILE_TEXT__PAYING_CAPTION_2 =		40590,	/*	駐車料金精算時の中見出しの文字列（EPS）
													 *	+0:	表示なし
													 *	+1:	"は先に入れてください"
													 *	+2:	"ポイントカードは現金投入前に入れてください"
													 *	+3:	"ポイントカードは"
													 *	+4:	"使用後に入れてください"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_PASS =		40600,	/*	券名称5（の定期券名称）
													 *	+0:	"定期券、"
													 *	+1:	"定期利用券、"
													 *	+2:	"パスカード、"
													 *	+3:	"契約カード、"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_SERVICE =	40610,	/*	券名称5（のサービス券名称）
													 *	+0:	"サービス券、"
													 *	+1:	"割引券、"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_OTHER =	40620,	/*	券名称5
													 *	+0:	"プリペイドカード、"
													 *	+1:	"回数券、"
													 *	+2:	"クレジットカード、"
													 */
	LCDBM_FILE_TEXT__PAID_CAPTION_2 =	40640,		/*	精算完了時のメッセージ表示(CPS/EPS)
													 *	+0:	表示なし
													 *	+1:	"ゲートが開きます　前へお進みください"
													 *	+2:	"ご利用ありがとうございました"
													 *	+3:	"気をつけてお帰りください"
													 *	+4:	"またのご利用お待ちしております"
													 *	+5:	"戻った"(券名称2)／(券名称3)
													 *	+6:	"は出口で必要です"
													 */
	LCDBM_FILE_TEXT__RECEIPT_NAME =	40650,			/* 領収証名称
													 * +0:"領収証"
													 * +1:"領収書"
													 * +2:"［領収証］"
													 * +3:"［領収書］"
													 */
	LCDBM_FILE_TEXT__RECEIPT_GUIDANCE =	40660,		/* 領収証案内
													 * +0:"の必要な方は"
													 * +1:"ボタンを押してください"
													 * +2:"精算完了時に"
													 * +3:"を発行します"
													 * +4:"または利用明細書を発行します"
													 * +5:"の発行は出来ません"
													 * +6:"をお取りください"
													 * +7:"利用明細書をお取りください"
													 */

	// つり切れ
	LCDBM_FILE_TEXT__ALARM_TITLE =	41000,			// "つり切れ "
	LCDBM_FILE_TEXT__ALARM_10,						// "10円 "
	LCDBM_FILE_TEXT__ALARM_50,						// "50円 "
	LCDBM_FILE_TEXT__ALARM_100,						// "100円 "
	LCDBM_FILE_TEXT__ALARM_500,						// "500円 "
	LCDBM_FILE_TEXT__ALARM_1000,					// "1000円 "
	LCDBM_FILE_TEXT__ALARM_2000,					// "2000円 "
	LCDBM_FILE_TEXT__ALARM_5000,					// "5000円"

	// 待機画面アラーム
	LCDBM_FILE_TEXT__ALARM_CHANGE =	41010,			// "釣銭不足"
	LCDBM_FILE_TEXT__ALARM_RECEIPT,					// "レシート用紙不足"
	LCDBM_FILE_TEXT__ALARM_JOURNAL,					// "ジャーナル用紙不足"
	LCDBM_FILE_TEXT__ALARM_NOTE,					// "紙幣利用不可"

	// 休業中
	LCDBM_FILE_TEXT__CLOSURE_CAPTION_1 =	41100,	// "休　業　中"
	LCDBM_FILE_TEXT__CLOSURE_INFO,					/*	休業時の表示(43-0062)
													 *	+0:	"ほかの精算所で精算してください"
													 *	+1:	"出口においでください"
													 *	+2:	"恐れ入りますが、受付においでください"
													 *	+3:	"恐れ入りますが、"
													 *	+4:	"においでください"
													 *	+5:	"インターホンで連絡してください"
													 *	+6:	"に連絡してください"
													 *	+7:	"呼び出しボタンを押してください"
													 *	+8:	"がまいります。しばらくお待ちください"
													 */

	// 釣銭補充と金庫枚数
	LCDBM_FILE_TEXT__REFILL	= 41300,				// LCDBM_FILE_TEXT__REFILL+((LCDBM_AREA_TEXT__REFILL_xxxx -1)*3)+OFFSET

	// 警告
	LCDBM_FILE_TEXT__WARNING =	41400,				// 警告画面の大見出し

// GW-861011 カラーLCDのEdy対応 (S) 2010/07/28(水) <Yoshiaki_Hamada@amano.co.jp> Edy対応
	// Ｅｄｙ関連
	LCDBM_FILE_TEXT__EDY =	41500,							// Ｅｄｙ関連
	LCDBM_FILE_TEXT__EDY_TESTMODE =	LCDBM_FILE_TEXT__EDY,	// Ｅｄｙテストモードの大見出し
// GW-861011 カラーLCDのEdy対応 (E) 2010/07/28(水) <Yoshiaki_Hamada@amano.co.jp> Edy対応


	//--------------------------------------------------
	//		個別ソフト用（ 50000 - 64999 ）
	//--------------------------------------------------
	LCDBM_FILE_TEXT__50000 =	50000,

};


#endif//_LCDBM_DEF_H_
// MH810100(E) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
