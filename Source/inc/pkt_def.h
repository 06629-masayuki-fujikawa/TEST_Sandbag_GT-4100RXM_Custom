// MH810100(S) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
//[]----------------------------------------------------------------------[]
///	@brief			パケット通信関連データ定義
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pkt_def.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	__PKT_DEF_H__
#define	__PKT_DEF_H__

#include "pkt_com.h"

/**
 *	LCDモジュールとの電文のコマンドＩＤとサブコマンドＩＤから４バイトのＩＤコードを計算する
 */
#define	lcdbm_command_id( id, subid )	( (( (unsigned long)id & 0x0000ffff ) << 16 ) | ( (unsigned long)subid & 0x0000ffff) )

/**
 *	16ビットカラー値を作成する
 */
#define	lcdbm_color_RGB( r, g, b )	((unsigned short)( (((r)&0x1f) << 11) | (((g)&0x3f) << 5) | (((b)&0x1f) << 0) ))

//--------------------------------------------------
//		通信不良要因最大値定義
//--------------------------------------------------
// 通信不良関連エラー全解除の為に要因の最大値(+1)を定義
#define	PKT_COMM_FAIL_MAX	ERR_TKLSLCD_KPALV_RTY_OV+1


//--------------------------------------------------
//		コマンド、サブコマンド定義
//		（ メイン基板 ==> LCDモジュール ）
//--------------------------------------------------
/**
 *	ステータス要求
 */
enum {
	LCDBM_CMD_ID_STATUS =				0x0001,		///< 「ステータス要求」のコマンドＩＤ

	LCDBM_CMD_SUBID_CONFIG =			0x0001,		///< 「基本設定要求」のサブコマンドＩＤ

	// コマンド＋サブコマンド
	LCDBM_CMD_NOTIFY_CONFIG =	lcdbm_command_id( LCDBM_CMD_ID_STATUS, LCDBM_CMD_SUBID_CONFIG ),	// 基本設定要求
};

/**
 *	機器設定要求
 */
enum {
	LCDBM_CMD_ID_SETUP =				0x0004,		///< 「機器設定要求」のコマンドＩＤ

	LCDBM_CMD_SUBID_CLOCK =				0x0101,		///< 「時計設定」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_BRIGHTNESS =		0x0201,		///< 「LCD輝度」のサブコマンドＩＤ
};

/**
 *	機器設定要求(時計通知)
 */
enum {
	LCDBM_CMD_ID_SETUP_CLOCK =			0x0104,		///< 「機器設定要求(時計通知)」のコマンドＩＤ

	LCDBM_CMD_SUBID_SETUP_CLOCK =		0x0101,		///< 「時計通知」のサブコマンドＩＤ
};

/**
 *	テキスト表示
 */
enum {
	LCDBM_CMD_ID_TEXT =					0x0013,		///< 「テキスト表示」のコマンドＩＤ

	LCDBM_CMD_SUBID_TEXT =				0x0001,		///< 「テキスト表示」のサブコマンドＩＤ

	/* コントロールコード */
	LCDBM_CMD_TEXT_CONTROL =			0x1b,		///< コントロールコード
	LCDBM_CMD_TEXT_FONT =				0x25,		///< 文字フォント指定
	LCDBM_CMD_TEXT_COLOR =				0x27,		///< 文字色指定
	LCDBM_CMD_TEXT_BLINK =				0x28,		///< 文字列ブリンク指定
	LCDBM_CMD_TEXT_CLEAR =				0x2b,		///< クリア指定
	LCDBM_CMD_TEXT_MENU =				0x2c,		///< メニュー表示指定
	LCDBM_CMD_TEXT_DIRECT =				0x2d,		///< 文字列指定
	LCDBM_CMD_TEXT_BACKGROUND =			0x2e,		///< 背景色指定
};

#define	LCDBM_CMD_TEXT_CLMN_MAX			30			///< 文字列最大サイズ

/**
 *	アナウンス
 */
enum {
	LCDBM_CMD_ID_AUDIO =				0x0021,		///< 「アナウンス要求」のコマンドＩＤ

	LCDBM_CMD_SUBID_AUDIO_VOLUME = 		0x000A,		///< 「音量調整」のサブコマンドＩＤ
};

/**
 *	ブザー
 */
enum {
	// コマンドＩＤ
	LCDBM_CMD_ID_BEEP =					0x0022,		///< 「ブザー」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_CMD_SUBID_BEEP_VOLUME =		0x000A,		///< 「音量設定」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_BEEP_START =		0x000B,		///< 「ブザー鳴動要求」のサブコマンドＩＤ
// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
	LCDBM_CMD_SUBID_BEEP_ALARM = 		0x000C,		///< 「警報鳴動」のサブコマンドＩＤ
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
};

// MH810100(S) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない
// ブザー種類
#define BEEP_KIND_ALARM		0x00					// 警報
#define BEEP_KIND_CHIME		0x01					// ドアノブチャイム

// ブザー種別
#define BEEP_TYPE_START		0x00					// 開始
#define BEEP_TYPE_STOP		0x01					// 停止
// MH810100(E) S.Takahashi 2020/02/21 #3898 防犯警報アラームが鳴らない

/**
 *	精算機状態通知操作
 */
enum {
	// コマンドＩＤ
	LCDBM_CMD_ID_STATUS_NOTICE =		0x0151,		///< 「精算機状態通知操作」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_CMD_SUBID_NOTICE_OPE =		0x0001,		///< 「操作通知」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_NOTICE_OPN =		0x0002,		///< 「営休業通知」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_NOTICE_PAY =		0x0003,		///< 「精算状態通知」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_NOTICE_DSP =		0x0004,		///< 「ポップアップ表示要求」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_NOTICE_ALM =		0x0005,		///< 「警告通知」のサブコマンドＩＤ
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	LCDBM_CMD_SUBID_NOTICE_DEL =		0x0006,		///< 「ポップアップ削除要求」のサブコマンドＩＤ
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
};

/**
 *	精算・割引情報通知
 */
enum {
	// コマンドＩＤ
	LCDBM_CMD_ID_PAY_INFO_NOTICE =		0x0160,		///< 「精算・割引情報通知」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_CMD_SUBID_PAY_REM_CHG =		0x0002,		///< 「精算残高変化通知」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_PAY_DATA =			0x0003,		///< 「精算データ」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_QR_CONF_CAN =		0x0006,		///< 「QR確定・取消データ」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_QR_DATA_RES =		0x0009,		///< 「QRデータ応答」のサブコマンドＩＤ
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
	LCDBM_CMD_SUBID_REMOTE_TIME_START =	0x000A,		///< 「入庫時刻指定遠隔精算開始」のサブコマンドＩＤ
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	LCDBM_CMD_SUBID_LANE =				0x000B,		///< 「レーンモニタデータ」のサブコマンドＩＤ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	LCDBM_CMD_SUBID_RECEIPT_DATA =		0x000D,		///< 「領収証データ」のサブコマンドＩＤ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
};

/**
 *	メンテナンスデータ
 */
enum {
	// コマンドＩＤ
	LCDBM_CMD_ID_MNT_DATA =				0x0080,		///< 「メンテナンスデータ」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_CMD_SUBID_MNT_QR_CTRL_REQ =	0x0001,		//「QRﾘｰﾀﾞ制御要求」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_MNT_RT_CON_REQ =	0x0010,		//「リアルタイム通信疎通要求」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_MNT_DC_CON_REQ =	0x0011,		//「DC-NET通信疎通要求」のサブコマンドＩＤ
};

//--------------------------------------------------
//		コマンド、サブコマンド定義
//		（ LCDモジュール ==> メイン基板 ）
//--------------------------------------------------
/**
 *	ステータス通知
 */
enum {
	// コマンドＩＤ
	LCDBM_RSP_ID_NOTIFY =				0x0101,		///< 「ステータス通知」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_RSP_SUBID_CONFIG =			0x0001,		///< 「基本設定応答」のサブコマンドＩＤ

	// コマンド＋サブコマンド
	LCDBM_RSP_NOTIFY_CONFIG =	lcdbm_command_id( LCDBM_RSP_ID_NOTIFY, LCDBM_RSP_SUBID_CONFIG ),	// 基本設定応答
};

/**
 *	エラー通知
 */
enum {
	// コマンドＩＤ
	LCDBM_RSP_ID_ERROR =				0x0102,		///< 「エラー通知」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_RSP_SUBID_ERROR =				0x0000,		///< 「エラー通知」のサブコマンドＩＤ

	// コマンド＋サブコマンド
	LCDBM_RSP_ERROR =	lcdbm_command_id( LCDBM_RSP_ID_ERROR, LCDBM_RSP_SUBID_ERROR ),	// エラー通知
};

/**
 *	テンキー制御
 */
enum {
	// コマンドＩＤ
	LCDBM_RSP_ID_TENKEY =				0x0152,		///< 「テンキー応答」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_RSP_SUBID_TK_KEYDOWN =		0x0001,		///< 「テンキー押下情報」のサブコマンドＩＤ

	// コマンド＋サブコマンド
	LCDBM_RSP_TENKEY_KEYDOWN =	lcdbm_command_id( LCDBM_RSP_ID_TENKEY, LCDBM_RSP_SUBID_TK_KEYDOWN ),	// テンキー押下情報
};

/**
 *	精算機状態通知操作
 *	→ LCD⇔精算機両方向コマンドの為、コマンドＩＤ/サブコマンドＩＤは「メイン基板 ==> LCDモジュール」側定義参照
 */
enum {
	// コマンド＋サブコマンド
	LCDBM_RSP_NOTICE_OPE =	lcdbm_command_id( LCDBM_CMD_ID_STATUS_NOTICE, LCDBM_CMD_SUBID_NOTICE_OPE ),	// 操作通知
};

/**
 *	精算・割引情報通知
 */
enum {
	 LCDBM_RSP_ID_PAY_INFO_NOTICE =		0x0160,		///< 「精算・割引情報通知」のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_RSP_SUBID_IN_INFO =			0x0001,		///< 「入庫情報」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_PAY_DATA_RES =		0x0005,		///< 「精算応答データ」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_QR_CONF_CAN_RES =	0x0007,		///< 「QR確定・取消データ応答」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_QR_DATA =			0x0008,		///< 「QRデータ」のサブコマンドＩＤ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	LCDBM_RSP_SUBID_LANE_RES =			0x000C,		///< 「レーンモニタデータ応答」のサブコマンドＩＤ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	LCDBM_RSP_SUBID_RECEIPT_DATA_RES =	0x000E,		///< 「領収証データ応答」のサブコマンドＩＤ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	// コマンド＋サブコマンド
	LCDBM_RSP_IN_INFO =			lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_IN_INFO ),			// 入庫情報
	LCDBM_RSP_PAY_DATA_RES =	lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_PAY_DATA_RES ),		// 精算応答データ
	LCDBM_RSP_QR_CONF_CAN_RES =	lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_QR_CONF_CAN_RES ),	// QR確定・取消データ応答
	LCDBM_RSP_QR_DATA =			lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_QR_DATA ),			// QRデータ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	LCDBM_RSP_LANE_RES =		lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_LANE_RES ),			// レーンモニタデータ応答
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	LCDBM_RSP_RECEIPT_DATA_RES = lcdbm_command_id(LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_RECEIPT_DATA_RES),	// 領収証データ応答
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
};

/**
 *	メンテナンスデータ
 */
enum {
	// コマンドＩＤ
	LCDBM_RSP_ID_MNT_DATA_LCD =			0x0180,		///< 「メンテナンスデータ」(LCD→精算機方向)のコマンドＩＤ

	// サブコマンドＩＤ
	LCDBM_RSP_SUBID_MNT_QR_CTRL_RES =	0x0001,		///< 「QRリーダ制御応答」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_MNT_QR_RD_RSLT =	0x0002,		///< 「QR読取結果」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_MNT_RT_CON_RSLT =	0x0010,		///< 「リアルタイム通信疎通結果」のサブコマンドＩＤ
	LCDBM_RSP_SUBID_MNT_DC_CON_RSLT =	0x0011,		///< 「DC-NET通信疎通結果」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_AUDIO_START =		0x000B,		///< 「開始要求」のサブコマンドＩＤ
	LCDBM_CMD_SUBID_AUDIO_END =			0x000C,		///< 「終了要求」のサブコマンドＩＤ

	// コマンド＋サブコマンド
	LCDBM_RSP_MNT_QR_CTRL_RES =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_QR_CTRL_RES ),	// QRリーダ制御応答
	LCDBM_RSP_MNT_QR_RD_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_QR_RD_RSLT ),	// QR読取結果
	LCDBM_RSP_MNT_RT_CON_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_RT_CON_RSLT ),	// リアルタイム通信疎通結果
	LCDBM_RSP_MNT_DC_CON_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_DC_CON_RSLT ),	// DC-NET通信疎通結果
};
// MH810100(S) Y.Yamauchi 2019/11/28 車番チケットレス(メンテナンス)
enum{
	LCDBM_QR_CTRL_VER = 0,					// ﾊﾞｰｼﾞｮﾝ要求
	LCDBM_QR_CTRL_RD_STA ,					// 読取開始
	LCDBM_QR_CTRL_RD_STP,					// 読取停止
	LCDBM_QR_CTRL_INIT_REQ,					// 初期化要求
};
// MH810100(E) Y.Yamauchi 2019/11/28 車番チケットレス(メンテナンス)

//--------------------------------------------------
//		操作コード定義
//--------------------------------------------------
typedef enum {
	LCDBM_OPCD_PAY_UNDEFINED	= 0,	//  0:(未定義)
	LCDBM_OPCD_PAY_STP,					//  1:精算中止
	LCDBM_OPCD_PAY_STP_RES,				//  2:精算中止応答
	LCDBM_OPCD_PAY_STA,					//  3:精算開始
	LCDBM_OPCD_PAY_STA_RES,				//  4:精算開始応答
	LCDBM_OPCD_MAN_DTC,					//  5:人体検知
	LCDBM_OPCD_DOOR_STATUS,				//  6:ドア状態
	LCDBM_OPCD_RCT_ISU,					//  7:領収証発行
	LCDBM_OPCD_RMT_PAY_STA,				//  8:遠隔精算開始
	LCDBM_OPCD_RMT_PAY_STA_RES,			//  9:遠隔精算開始応答
	LCDBM_OPCD_Dummy0_Send,				// 10:
	LCDBM_OPCD_STA_NOT,					// 11:起動通知
	LCDBM_OPCD_STA_CMP_NOT,				// 12:起動完了通知
	LCDBM_OPCD_PRM_UPL_NOT,				// 13:設定アップロード通知
	LCDBM_OPCD_PRM_UPL_REQ,				// 14:設定アップロード要求
	LCDBM_OPCD_RST_REQ,					// 15:フェーズリセット要求
	LCDBM_OPCD_Dummy1_Recv,				// 16:
	LCDBM_OPCD_Dummy1_Send,				// 17:
	LCDBM_OPCD_MNT_STA_REQ,				// 18:メンテナンス開始要求
	LCDBM_OPCD_MNT_STA_RES,				// 19:メンテナンス開始応答
	LCDBM_OPCD_MNT_END_REQ,				// 20:メンテナンス終了要求
	LCDBM_OPCD_MNT_END_RES,				// 21:メンテナンス終了応答
	LCDBM_OPCD_PAY_CMP_NOT,				// 22:精算完了通知
	LCDBM_OPCD_QR_STP_REQ,				// 23:QR停止要求
	LCDBM_OPCD_QR_STP_RES,				// 24:QR停止応答
	LCDBM_OPCD_QR_STA_REQ,				// 25:QR開始要求
	LCDBM_OPCD_QR_STA_RES,				// 26:QR開始応答
	LCDBM_OPCD_PAY_GUIDE_END_NOT,		// 27:精算案内終了通知
	LCDBM_OPCD_PAY_GUIDE_END_RES,		// 28:精算案内終了応答
	LCDBM_OPCD_RESET_NOT,				// 29:リセット通知
	LCDBM_OPCD_RESTORE_REQUEST,			// 30:復旧データ依頼(RXM←LCD)
	LCDBM_OPCD_RESTORE_RESULT,			// 31:復旧データ応答(RXM→LCD)
	LCDBM_OPCD_RESTORE_NOTIFY,			// 32:復旧データ    (RXM→LCD) ※自発的データ
	LCDBM_OPCD_CHG_IDLE_DISP_NOT,		// 33:待機画面通知  (RXM←LCD)
	LCDBM_OPCD_REJECT_LEVER,			// 34:リジェクト通知(RXM→LCD)
// MH810100(S) K.Onodera 2020/03/31 車番チケットレス(#3941 領収証自動発行後、ボタンが押下できてしまう)
	LCDBM_OPCD_RCT_AUTO_ISSUE,			// 35:領収証発行済み(RXM→LCD)
// MH810100(E) K.Onodera 2020/03/31 車番チケットレス(#3941 領収証自動発行後、ボタンが押下できてしまう)
// MH810100(S) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
	LCDBM_OPCD_IN_INFO_NG,				// 36:入庫情報NG応答
// MH810100(E) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
// MH810100(S) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
	LCDBM_OPCD_QR_NO_RESPONSE,			// 37:QR応答後の入庫情報は来ない通知
// MH810100(E) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
	LCDBM_OPCD_QR_DISCOUNT_OK,			// 38:QR割引処理OK通知
// MH810100(S) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
	LCDBM_OPCD_CARD_PAY_STATE,			// 39:カード決済処理状態通知
// MH810100(E) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
// MH810100(S) S.Nishimoto 2020/08/25 車番チケットレス(#4754 決済NG後のとりけし時にカード抜き取り待ちを行わない)
	LCDBM_OPCD_CARD_EJECTED,			// 40:クレジットカード抜去通知
// MH810100(E) S.Nishimoto 2020/08/25 車番チケットレス(#4754 決済NG後のとりけし時にカード抜き取り待ちを行わない)
// MH810100(S) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
	LCDBM_OPCD_CLEAR_COUNT,				// 41:動作カウントクリア
// MH810100(E) S.Nishimoto 2020/09/16 車番チケットレス(#4867 動作カウント(LCD起動時間)のクリア内容が反映されない)
// MH810103(s) 電子マネー対応 電子マネーブランド選択時の案内放送
	LCDBM_OPCD_BRAND_SELECTED,			// 42:ブランド選択通知
	LCDBM_OPCD_BRAND_SELECT_CANCEL,		// 43:ブランド選択キャンセル
// MH810103(e) 電子マネー対応 電子マネーブランド選択時の案内放送
// MH810103(s) 電子マネー対応 待機画面での残高照会可否
	LCDBM_OPCD_INQUIRY_PAY_AFTER,		// 44:残高照会可否通知
// MH810103(e) 電子マネー対応 待機画面での残高照会可否
// MH810103(s) 電子マネー対応 #5555【WAONシングル対応】精算開始で「音が鳴るまでしっかりタッチして下さい」を案内放送する
	LCDBM_OPCD_EMONEY_SINGLE,			// 45:電子マネーシングル通知
// MH810103(e) 電子マネー対応 #5555【WAONシングル対応】精算開始で「音が鳴るまでしっかりタッチして下さい」を案内放送する
// GG129000(S) H.Fujinaga 2022/12/09 ゲート式車番チケットレスシステム対応（#6411 電子マネー設定なしの設定でサービス券(QR)で精算完了時に領収証発行ボタンがタッチ可能になっている）
	LCDBM_OPCD_EJA_USE,					// 46:電子ジャーナル接続状態通知
// GG129000(E) H.Fujinaga 2022/12/09 ゲート式車番チケットレスシステム対応（#6411 電子マネー設定なしの設定でサービス券(QR)で精算完了時に領収証発行ボタンがタッチ可能になっている）
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	LCDBM_OPCD_SEARCH_TYPE,				// 47:検索タイプ通知(0:車番検索,1:時刻検索,2:QR検索)
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
// GG129000(S) T.Nagai 2023/09/28 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	LCDBM_OPCD_RTM_REMOTE_PAY_REQ,		// 48:遠隔精算（リアルタイム）開始要求
	LCDBM_OPCD_RTM_REMOTE_PAY_RES,		// 49:遠隔精算（リアルタイム）開始応答
	LCDBM_OPCD_RTM_REMOTE_PAY_CAN,		// 50:遠隔精算（リアルタイム）受付中止
// GG129000(E) T.Nagai 2023/09/28 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	LCDBM_OPCD_POP_PAYQR = 90,			// 90:QR駐車券発行ダイアログ(領収証ボタンなし)
	LCDBM_OPCD_POP_PAYQRBUTTON,			// 91:QR駐車券発行ダイアログ(領収証ボタンあり)
	LCDBM_OPCD_POP_PAYQRNONE,			// 92:QR駐車券発行ダイアログ表示なし
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	LCDBM_OPCD_POP_PAYQRNORMAL,			// 94:QR駐車券発行ダイアログ(正常)
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（QR駐車券対応）
} eOPE_NOTIF_CODE;

//--------------------------------------------------
//		休業理由コード定義
//--------------------------------------------------
enum {
	LCDBM_OPNCLS_RSN_UNDEFINED			= 0,	// (未定義)
	LCDBM_OPNCLS_RSN_FORCED,					// 強制営休業
	LCDBM_OPNCLS_RSN_OUT_OF_HOURS,				// 営業時間外
	LCDBM_OPNCLS_RSN_BANKNOTE_FULL,				// 紙幣金庫満杯
	LCDBM_OPNCLS_RSN_COIN_SAFE_FULL,			// コイン金庫満杯
	LCDBM_OPNCLS_RSN_BANKNOTE_DROP,				// 紙幣リーダー脱落
	LCDBM_OPNCLS_RSN_NO_CHANGE,					// 釣銭切れ
	LCDBM_OPNCLS_RSN_COINMECH_COMM_FAIL,		// コインメック通信不良
	LCDBM_OPNCLS_RSN_SEND_BUFF_FULL,			// 送信バッファフル
	LCDBM_OPNCLS_RSN_RESERVE,					// (予約)
	LCDBM_OPNCLS_RSN_SIGNAL,					// 信号による休業
	LCDBM_OPNCLS_RSN_COMM,						// 通信による休業
};

//--------------------------------------------------
//		結果コード定義
//--------------------------------------------------
enum{
	LCDBM_RESUTL_OK						= 0,	// 正常
	LCDBM_RESUTL_NG,							// 異常
	LCDBM_RESUTL_TIMEOUT,						// タイムアウト
};

//--------------------------------------------------
//		ポップアップ表示要求種類
//--------------------------------------------------
typedef enum{
	POP_AZUKARI_PAY = 1,// 1:預り証発行（精算完了時）
	POP_AZUKARI_RET,	// 2:預り証発行（払い戻し時）
	POP_FAIL_REDUCTION,	// 3:減額失敗
	POP_OVER_LIMIT,		// 4:限度超え
	POP_INQUIRY_NG,		// 5:照会NG
// MH810103(s) 電子マネー対応 エラーポップアップ変更
//	POP_CREDIT_SV_NG,	// 6:ｸﾚｼﾞｯﾄｻｰﾊﾞ異常
	POP_EMONEY_INQUIRY_NG,		// 6:照会NG(クレジット以外)
// MH810103(e) 電子マネー対応 エラーポップアップ変更
	POP_SHORT_BALANCE,	// 7:残高不足
	POP_UNABLE_PAY,		// 8:精算不可
	POP_UNABLE_QR,		// 9:QR利用不可
// MH810100(S) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
	POP_UNABLE_DISCOUNT,// 10:割引不可
// MH810100(E) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
// MH810105(S) MH364301 インボイス対応
	POP_RECIPT_OUT_ERROR,	// 11:領収証発行失敗
// MH810105(E) MH364301 インボイス対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	POP_QR_RECIPT,		// 12:QR領収証
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
}ePOP_DISP_KIND;

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
//--------------------------------------------------
//		ポップアップ削除要求種類
//--------------------------------------------------
typedef enum{
	POPDEL_RECIPT_OUT = 1,		// 1:領収証発行POP
}ePOP_DISP_DEL_KIND;
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

#pragma pack
//-------------------------------------------------------------------------------------------------------------------------------------------------------
//ｶｰﾄﾞ情報		// ★ = GT4100で使用する
//-------------------------------------------------------------------------------------------------------------------------------------------------------

// カード番号種別
#define CARD_TYPE_PARKING_TKT	101
#define CARD_TYPE_PASS			102
// GG129000(S) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）
#define CARD_TYPE_INOUT_ID		103
// GG129000(E) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ車番対応)
#define CARD_TYPE_CAR_NUM		203
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ車番対応)

#define ONL_MAX_CARDNUM			6
#define ONL_MAX_SETTLEMENT		10
#define ONL_MAX_DISC_NUM		25
#define ONL_DISC_MAX			25
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
#define ONL_MAX_MEDIA_NUM		32
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)

typedef struct 
{
	ushort 	shYear;			// 年	2000～2099(処理日時)
	uchar 	byMonth;		// 月	1～12(処理日時)
	uchar 	byDay;			// 日	1～31(処理日時)
	uchar 	byHours;		// 時	0～23(処理日時)
	uchar 	byMinute;		// 分	0～59(処理日時)
	uchar 	bySecond;		// 秒	0～59(処理日時)
} stDatetTimeYtoSec_t;		// 年月日時分秒

typedef struct _stShisetuInfo_t
{
	ulong		code;			// 施設コード
	ushort		count;			// 認証確定レシート枚数
	ulong		amount;			// 累計金額
} stShisetuInfo_t;

typedef struct 
{
	ushort 	RevNoInfo;			// ｶｰﾄﾞ情報Rev.No.					10～65535
	uchar 	MasterOnOff;		// ｶｰﾄﾞ情報 ﾏｽﾀｰ情報  有無			0:無､1:有	<- 有
	uchar 	ReserveOnOff;		// ｶｰﾄﾞ情報 予約情報  有無			0:無､1:有	<- 有
	uchar 	PassOnOff;			// ｶｰﾄﾞ情報 定期･会員情報  有無		0:無､1:有	<- 有
	uchar 	ZaishaOnOff;		// ｶｰﾄﾞ情報 在車情報  有無			0:無､1:有	<- 有

	ushort 	MasterSize;			// ｶｰﾄﾞ情報ｻｲｽﾞ  ﾏｽﾀｰ情報			0～65535	<- sizeof(stMasterInfo_t)
	ushort 	ReserveSize;		// ｶｰﾄﾞ情報ｻｲｽﾞ  予約情報			0～65535	<- sizeof(stReserveInfo_t)
	ushort 	PassSize;			// ｶｰﾄﾞ情報ｻｲｽﾞ  定期･会員情報		0～65535	<- sizeof(stPassInfo_t)
	ushort 	ZaishaSize;			// ｶｰﾄﾞ情報ｻｲｽﾞ  在車情報			0～65535	<- sizeof(stZaishaInfo_t)
} stCardInformation_t;			// ｶｰﾄﾞ情報

typedef struct 
{
	uchar	PayResultInfo;		// 要求結果  料金計算				0:無	1:有
	uchar	TotalPayInfo;		// 要求結果  累計金額問合せ			0:無	1:有
	uchar 	MasterOnOff;		// 要求結果 ﾏｽﾀｰ情報  有無			0:無	1:有
	uchar 	ReserveOnOff;		// 要求結果 予約情報  有無			0:無	1:有
	uchar 	PassOnOff;			// 要求結果 定期･会員情報  有無		0:無	1:有
	uchar 	ZaishaOnOff;		// 要求結果 在車情報  有無			0:無	1:有
// MH810100(S) 2020/05/29 車番チケットレス(#4196)
	uchar	CalcResult;			// 要求結果  料金計算排他			0=要求なし、1=排他成功、2=排他中（他の端末で精算中）
// MH810104(S) R.Endo 2021/09/21 車番チケットレス フェーズ2.3 #6001 【連動試験】リアルタイム通信 - センター問合せ結果データを受信しても精算開始できない
// 	uchar	Reserve;			// 予備
// MH810104(E) R.Endo 2021/09/21 車番チケットレス フェーズ2.3 #6001 【連動試験】リアルタイム通信 - センター問合せ結果データを受信しても精算開始できない
// MH810100(E) 2020/05/29 車番チケットレス(#4196)
// GG124100(S) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	uchar 	FeeCalcOnOff;		// 要求結果 料金計算結果  有無		0:無	1:有
// GG124100(E) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

	ushort 	PayResultSize;		// 要求結果ｻｲｽﾞ  料金計算			0～65535
	ushort 	TotalPaySize;		// 要求結果ｻｲｽﾞ  累計金額問合せ		0～65535
	ushort 	MasterSize;			// 要求結果ｻｲｽﾞ  ﾏｽﾀｰ情報			0～65535
	ushort 	ReserveSize;		// 要求結果ｻｲｽﾞ  予約情報			0～65535
	ushort 	PassSize;			// 要求結果ｻｲｽﾞ  定期･会員情報		0～65535
	ushort 	ZaishaSize;			// 要求結果ｻｲｽﾞ  在車情報			0～65535
// GG124100(S) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	ushort 	FeeCalcSize;		// 要求結果ｻｲｽﾞ  料金計算結果		0～65535
// GG124100(E) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
} stRecvCardInformation_t;		// ｶｰﾄﾞ情報_要求結果

typedef struct 
{
	ulong 		ParkingLotNo;	// 駐車場№(0～999999)
	ushort 		CardType;		// 番号種別(0～65000)
	uchar 		byCardNo[32];	// 番号(32桁 ※ASCII(左詰,残りはNull固定))
} stParkKindNum_t;

typedef struct {
	uchar byDispCardNo[32];		// 表示ｶｰﾄﾞ番号(ASCII(左詰,残りはNull固定)一律端末側でｾｯﾄする)
} stDispCardData_t;

typedef struct 
{
	ushort 	shYear;				// 年(2000～2099)
	uchar 	byMonth;			// 月(1～12)
	uchar 	byDay;				// 日(1～31)
} stDate_YYMD_t;

typedef struct 
{
	ulong 	upper;				// 予約情報管理番号 上位6桁	0～999999
	ulong 	lower;				// 予約情報管理番号 下位7桁	0～9999999
} stReserveManageNo_t;

typedef struct 
{
	uchar	RsvFeeType;			// 予約料金徴収種別		0:徴収なし､1:ｾﾝﾀｰ徴収 ､2:端末徴収
	uchar	RsvFeeStatus;		// 予約料金徴収ｽﾃｰﾀｽ	0:未徴収 1:徴収済み
	ulong	RsvFee;				// 予約料金金額			0～999999
}	stReserveFeeInfo_t;			// 予約料金(予約料金徴収種別, 予約料金徴収ｽﾃｰﾀｽ, 予約料金金額)

typedef struct 
{
	uchar	PenaType;			// ﾍﾟﾅﾙﾃｨ料金徴収種別	0:徴収なし､1:減泊及び延泊､2:減泊のみ､3:延泊のみ
	uchar	PenaStatus;			// ﾍﾟﾅﾙﾃｨ料金徴収ｽﾃｰﾀｽ	0:未徴収 1:減泊により徴収済み 2:延泊により徴収済み
	ulong	PenaUnit;			// ﾍﾟﾅﾙﾃｨ料(単価)		0～999999 1日あたりのﾍﾟﾅﾙﾃｨ料の単価
	ulong	PenaFee;			// ﾍﾟﾅﾙﾃｨ料金徴収金額	0～999999端末が徴収した金額｡
	ushort	PenaFinishDates;	// ﾍﾟﾅﾙﾃｨ料金徴収済み日数		0～999
	ulong	PenaTotalDates;		// ﾍﾟﾅﾙﾃｨ料金徴収済み合計金額	0～999999
}	stPenaltyFeeInfo_t;			// ﾍﾟﾅﾙﾃｨ料金(ﾍﾟﾅﾙﾃｨ料金徴収種別, ﾍﾟﾅﾙﾃｨ料金徴収ｽﾃｰﾀｽ, ﾍﾟﾅﾙﾃｨ料(単価), ﾍﾟﾅﾙﾃｨ料金徴収金額, ﾍﾟﾅﾙﾃｨ料金徴収済み日数, ﾍﾟﾅﾙﾃｨ料金徴収済み合計金額)

typedef struct
{
	ushort 	shYear;				// 年	2000～2099
	uchar 	byMonth;			// 月	1～12
	uchar 	byDay;				// 日	1～31
	uchar 	byHours;			// 時	0～23
	uchar 	byMinute;			// 分	0～59
} stDate_YYMDHM_t;				// 年月日時分

typedef struct 
{
	ulong 				ParkingNo;		// 駐車場№(0～999999)
	ushort 				MachineKind;	// 機種ｺｰﾄﾞ(0～999)
	ushort 				MachineNo;		// 機械№(0～999)
	stDatetTimeYtoSec_t	dtTimeYtoSec;	// 年月日時分秒
	uchar				Reserve1;		// 予約		0固定
} stDateParkTime_t;

typedef struct 
{
	ushort 	CargeType;			// 決算区分		0=なし,1=ｸﾚｼﾞｯﾄ,2=交通系電子ﾏﾈｰ(Suica,PASMO,ICOCA等),
								//				3=Edy,4=PiTaPa,5=WAON,6=nanaco,7=SAPICA,8 / 9 =(予約),
								//				10=ETC,11= ETC-EC ※90以降=個別対応
	ulong 	CargeFee;			// 決算金額(0～999999)
} stSettlement_t;

typedef struct {
	ushort 	MachineNo;			// 暗証番号 発券機番号(0～99)
	ulong 	Password;			// 暗証番号 番号(0～99999999)
} stPasswordNo_t;

typedef struct 
{
	ulong	DiscParkNo;				// 駐車場№(0～999999)
	ushort 	DiscSyu;				// 種別(0～9999)
	ushort 	DiscCardNo;				// ｶｰﾄﾞ区分(0～65000)
	uchar 	DiscNo;					// 区分(0～9)
	uchar 	DiscCount;				// 枚数(0～99)
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// 	ulong 	Discount;				// 金額/時間(0～999999)
	ulong 	Discount;				// 金額(0～999999)
	ulong 	DiscTime;				// 時間(0～999999)
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
	ushort 	DiscInfo;				// 割引情報(0～65000)
	ushort 	DiscCorrType;			// 対応ｶｰﾄﾞ種別(0～65000)
	uchar 	DiscStatus;				// ｽﾃｰﾀｽ(0～9)
	uchar 	DiscFlg;				// 割引状況(0～9)
} stDiscount_t;

typedef struct 
{
	ulong	DiscParkNo;				// 駐車場№(0～999999)
	ushort 	DiscSyu;				// 種別(0～9999)
	ushort 	DiscCardNo;				// ｶｰﾄﾞ区分(0～65000)
	uchar 	DiscNo;					// 区分(0～9)
	uchar 	DiscCount;				// 枚数(0～99)
// MH810100(S) 2020/05/28 車番チケットレス(#4196)
//	ulong 	DiscountM;				// 金額(0～999999)
//	ulong 	DiscountT;				// 時間(0～999999)
	ulong 	Discount;				// 金額/時間(0～999999)
	ulong	UsedDisc;				// 買物割引利用時の使用済み割引（金額/時間）(0～999999)
// MH810100(E) 2020/05/28 車番チケットレス(#4196)
	ushort 	DiscInfo;				// 割引情報(0～65000)
	ushort 	DiscCorrType;			// 対応ｶｰﾄﾞ種別(0～65000)
	uchar 	DiscStatus;				// ｽﾃｰﾀｽ(0～9)
	uchar 	DiscFlg;				// 割引状況(0～9)
} stDiscount2_t;

typedef struct 
{
	ulong	DiscParkNo;				// 駐車場№(0～999999)
	ushort 	DiscSyu;				// 種別(0～9999)
	ushort 	DiscCardNo;				// ｶｰﾄﾞ区分(0～65000)
	ushort 	DiscNo;					// 区分(0～9)
	ushort 	DiscInfo;				// 割引情報(0～65000)
} stDiscount3_t;

// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
typedef struct 
{
	ulong	DiscParkNo;				// 割引 駐車場№
	ushort	DiscSyu;				// 割引 種別
	ushort	DiscCardNo;				// 割引 カード区分
	uchar	DiscNo;					// 割引 区分
	uchar	DiscCount;				// 割引 枚数
	ulong	PrevUsageDiscAmount;	// 割引 前回精算までの使用済み割引金額(実割)
	ulong	PrevUsageDiscTime;		// 割引 前回精算までの使用済み割引時間(実割)
	ulong	DiscAmount;				// 割引 今回の割引金額(実割)
	ulong	DiscTime;				// 割引 今回の割引時間(実割)
	ulong	DiscAmountSetting;		// 割引 設定金額(割引金額の設定値×枚数)
	ulong	DiscTimeSetting;		// 割引 設定時間(割引時間の設定値×枚数)
	uchar	DiscPercentSetting;		// 割引 設定%(割引%の設定値)
	uchar	FeeKindSwitchSetting;	// 割引 設定種別切換先車種
	ushort	DiscInfo;				// 割引 割引情報
	ushort	DiscCorrType;			// 割引 対応カード種別
	uchar	DiscStatus;				// 割引 ステータス
	uchar	DiscFlg;				// 割引 割引状況
} stDiscount4_t;
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

typedef struct 
{
	stDiscount3_t		Discount;	// 割引基本情報
	DATE_YMDHMS			Date;		// 認証確定年月日時分秒
	uchar				Reserve;	// 予備
} stWaribikiInfo_t;

typedef struct 
{
	uchar 	PassKind;				// 定期券種別		0=なし､1～15
	ulong 	PassNum;				// 定期券№			0=なし､1～999999
	uchar 	PassStatus;				// 定期券ｶｰﾄﾞｽﾃｰﾀｽ	0=なし､1～99 (1=有効､2=無効)
} stPassKindNumSts_t;				// 定期券種別, 定期券№, 定期券ｶｰﾄﾞｽﾃｰﾀｽ

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 料金計算結果情報(684 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢要求 料金計算｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする
//====================================================================================================
typedef struct _stPayResultInfo_t
{
	ushort 					shPayResult;						// 料金計算結果		0=処理成功､1以上=失敗
	stParkKindNum_t			dtPayMedia;							// 精算媒体(駐車場№, 種別, 番号)
	uchar					FeeType;							// 料金種別(0～12)
	stPassKindNumSts_t		dtPassKindNumSts;					// 定期券種別, 定期券№, 定期券ｶｰﾄﾞｽﾃｰﾀｽ
	uchar					Reserve1;							// 予備(ｻｲｽﾞ調整用)	1	0固定
	stDate_YYMD_t			dtEffectStart;						// 有効開始(年, 月, 日)
	stDate_YYMD_t			dtEffectStop;						// 有効終了(年, 月, 日)
	ulong					ParkingTime;						// 駐車時間(0～999999(分単位))
	ulong					ParkingFee;							// 駐車料金(0～999990(10円単位))
	ulong					Seikyuugaku;						// 請求額(0～999990(10円単位))
	stDatetTimeYtoSec_t		dtNyuukoYtoSec;						// 入庫年月日時分秒
	uchar					Reserve2;							// 予備(ｻｲｽﾞ調整用)	1	0固定
	stDatetTimeYtoSec_t		dtPrivSeisanYtoSec;					// 前回精算年月日時分秒
	uchar					Reserve3;							// 予備(ｻｲｽﾞ調整用)	1	0固定
	stDiscount2_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// 割引01～25
} stPayResultInfo_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 累計金額情報(600 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢情報要求 累計金額問合せ｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする
//====================================================================================================
typedef struct _stTotalPayInfo_t
{
	stShisetuInfo_t		stShisetuInfo[10];					// 施設(施設ｺｰﾄﾞ, 認証確定ﾚｼｰﾄ枚数, 累計金額)
	stWaribikiInfo_t	stWaribikiInfo[ONL_MAX_DISC_NUM];	// 割引01-25
} stTotalPayInfo_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// ﾏｽﾀｰ情報(434 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢情報要求 予約検索｣または
// ｢情報要求 会員･定期検索｣｢情報要求 在車検索｣のいずれかが｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする
//====================================================================================================
typedef struct _stMasterInfo
{
	stParkKindNum_t		stCardDataInfo[ONL_MAX_CARDNUM];		// ｶｰﾄﾞ1～6(駐車場№, 種別, 番号)	★ｶｰﾄﾞ1のみ
	stDispCardData_t	stDispCardDataInfo[ONL_MAX_CARDNUM];	// 表示用ｶｰﾄﾞﾃﾞｰﾀ1～6				★ｶｰﾄﾞ1のみ
	stDate_YYMD_t		StartDate;								// 有効開始年月日
	stDate_YYMD_t		EndDate;								// 有効終了年月日
	uchar				SeasonKind;								// 定期種別(0～15)
	uchar				MemberKind;								// 会員種別(0～255)
	uchar				InOutStatus;							// 入出庫ｽﾃｰﾀｽ	0～99(0=初期状態(入出庫可),1=出庫中(入庫可),2=入庫中(出庫可))
	uchar				PayMethod;								// 支払い手段	ETCｶｰﾄﾞ利用時の決済手段をｾﾝﾀｰから端末へ指定する｡
																//				0:現金など(※),1:ETC決済,2:EC決済
																//				※現金,ｸﾚｼﾞｯﾄ,電子ﾏﾈｰ,ﾌﾟﾘﾍﾟｲﾄﾞなど
	uchar				EntryConf;								// 入場確認ﾌﾗｸﾞ	ｾﾝﾀｰから端末へ指示するﾌﾗｸﾞ｡
																//				0:確認しない 1:確認する(ｹﾞｰﾄを開けず係員による確認を行う)
	uchar				Reserve1;								// 予備			0固定
} stMasterInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 予約情報(84 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢情報要求 予約検索｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする｡
//====================================================================================================
typedef struct _stReserveInfo
{
	stReserveManageNo_t		dtRsvManageNo;		// 予約情報管理番号
	stDatetTimeYtoSec_t		dtRsvTimeYtoSec;	// 予約登録年月日時分秒
	uchar					Reserve1;			// 予備			0固定
	ushort					RsvParkFeeKindNo;	// 予約駐車料金種別設定番号		0～99
	ulong					ParkSpaceMngNo;		// 駐車ｽﾍﾟｰｽ管理番号	0～999999  例)1:予約一般､2:予約身障者､3:優先予約1､4:優先予約2 など
	stDate_YYMDHM_t			dtRsvStartEnter;	// 予約開始(入場予定)年月日時分
	stDate_YYMDHM_t			dtRsvStartExit;		// 予約終了(出場予定)年月日時分
	stReserveFeeInfo_t		dtRsvFeeInfo;		// 予約料金(予約料金徴収種別, 予約料金徴収ｽﾃｰﾀｽ, 予約料金金額)
	stPenaltyFeeInfo_t		dtPenaltyFeeInfo;	// ﾍﾟﾅﾙﾃｨ料金(ﾍﾟﾅﾙﾃｨ料金徴収種別, ﾍﾟﾅﾙﾃｨ料金徴収ｽﾃｰﾀｽ, ﾍﾟﾅﾙﾃｨ料(単価), ﾍﾟﾅﾙﾃｨ料金徴収金額, ﾍﾟﾅﾙﾃｨ料金徴収済み日数, ﾍﾟﾅﾙﾃｨ料金徴収済み合計金額)
	uchar					Under1day;			// 1日未満駐車料金徴収ｽﾃｰﾀｽ		0:精算日時まで､1:1日分の最大料金
	uchar					Reserve2;			// 予備							0固定
	stDate_YYMDHM_t			dtEntryStart;		// 入場可能開始年月日時分
	stDate_YYMDHM_t			dtEntryEnd;			// 入場可能終了年月日時分
	stDate_YYMDHM_t			dtExitStart;		// 出場可能開始年月日時分
	stDate_YYMDHM_t			dtExitEnd;			// 出場可能終了年月日時分
	ushort					TransitTime;		// 通過可能延長時間
												// 予約専用入口から予約棟入口を通過できる時間制限に用いる｡
												// 分単位(0=制限なし､1～1440)
} stReserveInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 定期･会員情報(16 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢情報要求 定期･会員検索｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする｡
//====================================================================================================
typedef struct _stPassInfo
{
	ulong					ContractNo;			// 契約管理番号		1～16,000,000
	uchar					SaleStatus;			// 販売ｽﾃｰﾀｽ		1=販売済み/9=販売前
	uchar					CardStatus;			// ｶｰﾄﾞｽﾃｰﾀｽ		0～99 (1=有効､2=無効)
	uchar					Classification;		// 定期区分			0～99
	uchar					DivisionNo;			// 契約区画番号		0～26
	ushort					CasingNo;			// 契約車室番号		1～9999
	uchar					RackInfo;			// 契約ﾗｯｸ情報		※2
	uchar					UpdatePermission;	// 4月跨ぎ更新許可	0～1(0=許可しない/1=許可する)
	uchar					Option1;			// ｵﾌﾟｼｮﾝ代金ﾌﾗｸﾞ①	各ﾋﾞｯﾄ対応
	uchar					Option2;			// ｵﾌﾟｼｮﾝ代金ﾌﾗｸﾞ②	各ﾋﾞｯﾄ対応
	uchar					ReWriteCount;		// 再発行回数		0～255
	uchar					Reserve1;			// 予備				0固定
} stPassInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 在車情報(668 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢情報要求 在車検索｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする｡
//====================================================================================================
typedef struct _stZaishaInfo
{
	ulong					ResParkingLotNo;					// 在車中の駐車場№(0～999999)在車なしの場合は｢0｣
	uchar					PaymentType;						// 精算処理区分	0=未精算/1=精算/2=再精算/3=精算中止/4=再精算中止
	uchar					ExitPayment;						// 精算出庫		0=通常精算/1=強制出庫/(2=精算なし出庫)/3=不正出庫/
																//				9=突破出庫10=ｹﾞｰﾄ開放/97=ﾛｯｸ開･ﾌﾗｯﾌﾟ上昇前未精算出庫/
																//				98=ﾗｸﾞﾀｲﾑ内出庫/99=ｻｰﾋﾞｽﾀｲﾑ内出庫
	ulong					GenkinFee;							// 現金売上額(0～999999)
	ushort					shFeeType;							// 料金種別(0～99)
	stDateParkTime_t		dtEntryDateTime;					// 入庫			Parking情報 + 年月日時分秒
	stDateParkTime_t		dtPaymentDateTime;					// 精算			Parking情報 + 年月日時分秒
	stDateParkTime_t		dtExitDateTime;						// 出庫			Parking情報 + 年月日時分秒
	stDateParkTime_t		dtEntranceDateTime;					// 入口通過		Parking情報 + 年月日時分秒
	stDateParkTime_t		dtOutleteDateTime;					// 出口通過		Parking情報 + 年月日時分秒
	ulong					ParkingFee;							// 駐車料金(0～999990(10円単位))
	ulong					ParkingTime;						// 駐車時間(0～999999(分単位))
	uchar					NormalFlag;							// 一時利用ﾌﾗｸﾞ(0=定期利用,1=一時利用)
// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	uchar					Reserve3;							// 予備
	uchar					InCarMode;							// 入庫モード
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
	stSettlement_t			stSettlement[ONL_MAX_SETTLEMENT];	// 決算情報01～10
	stPasswordNo_t			stPassword;						// 暗証番号(発券機番号, 番号)
	stDiscount_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// 割引01～25
} stZaishaInfo_t;
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// 料金計算結果(1113 Byte)
// 遠隔ﾘｱﾙﾀｲﾑ通信の｢ｾﾝﾀｰ問合せﾃﾞｰﾀ(ID:XX)｣にて｢要求 料金計算｣が｢1=要求あり｣時にｾﾝﾀｰがｾｯﾄする｡
//====================================================================================================
typedef struct _stFeeCalcInfo
{
	ushort					FeeCalcRevNo;						// 料金計算Rev.No.
	ushort					CalcResult;							// 割引情報取得結果
	ushort 					shPayResult;						// 料金計算結果
	uchar					DateSwitchHours;					// 日付切替時刻 時
	uchar					DateSwitchMinute;					// 日付切替時刻 分
	stParkKindNum_t			dtPayMedia;							// 精算媒体(駐車場№、種別、番号)
	uchar					FeeType;							// 料金種別
	stPassKindNumSts_t		dtPassKindNumSts;					// 定期券情報
	uchar					PassAlarm;							// 定期券アラーム情報
	stDate_YYMD_t			dtEffectStart;						// 有効開始 年月日
	stDate_YYMD_t			dtEffectStop;						// 有効終了 年月日
	ulong					ParkingTime;						// 駐車時間
	ulong					ParkingFee;							// 駐車料金
	ulong					Seikyuugaku;						// 請求額
	stDatetTimeYtoSec_t		dtNyuukoYtoSec;						// 入庫 年月日時分秒
	uchar					Reserve2;							// 予備(サイズ調整用)
	stDatetTimeYtoSec_t		dtPrivSeisanYtoSec;					// 前回精算 年月日時分秒
	uchar					Reserve3;							// 予備(サイズ調整用)
	stDatetTimeYtoSec_t		dtSeisanYtoSec;						// 精算 年月日時分秒
	uchar					Reserve4;							// 予備(サイズ調整用)
// GG132000(S) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
	ushort					MaxFeeApplyFlg;						// 最大料金適用フラグ
	ushort					MaxFeeApplyCnt;						// 最大料金適用回数
	uchar					MaxFeeSettingFlg;					// 最大料金設定有無
// GG132000(E) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
	uchar					PaymentTimeType;					// 精算時刻区分
	stDiscount4_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// 割引01～25
} stFeeCalcInfo_t;
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
//-------------------------------------------------------------------------------------------------------------------------------------------------------
/****************************************/
/*		入庫情報	ｶｰﾄﾞ情報(受信)		*/
/*		Rev.No.10						*/
/****************************************/
// ※ﾘｱﾙﾀｲﾑｻｰﾊﾞから受信するｾﾝﾀｰ問合せ結果と同等
// ※異なる点は､結果ｻｲｽﾞをLCDにて必ず入れる
//   各それぞれの情報もない場合は､空で詰める(固定長にする)

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// typedef struct _stRecvCardInfo_Rev10
// {
// 	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.			0～65535
// 	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
// 	uchar					Reserve1;			// 予備						0固定
// //	stCardInformation_t		dtCardInfo;			// ｶｰﾄﾞ情報
// 
// 	ushort					RsltReason;			// 結果理由					0 =正常(ｾﾝﾀｰがｾｯﾄ)/1 =【通信】ｾﾝﾀｰ?MLC間通信障害中(MLCがｾｯﾄ)/10=【ｾﾝﾀｰ】DB参照ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)/11=【ｾﾝﾀｰ】通信ﾃﾞｰﾀ解析ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)
// 	ulong					AskMediaParkNo;		// 問合せ媒体 駐車場№		0～999999
// 	ushort					AskMediaRevNo;		// 問合せ媒体 情報Rev.No.	0～65535
// 	ushort					AskMediaKind;		// 問合せ媒体 種別			0～65000
// 	uchar					byAskMediaNo[32];	// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）
// 	uchar					ValidCHK_ETC;		// 有効性ﾁｪｯｸ ETCﾈｶﾞﾁｪｯｸ	0=ﾁｪｯｸなし､1=ETCﾈｶﾞOK､2=ETCﾈｶﾞNG
// 	uchar					ValidCHK_Dup;		// 有効性ﾁｪｯｸ 割引重複利用ﾁｪｯｸ	0=ﾁｪｯｸなし､1=割引重複なし､2=割引重複あり､3=ﾃﾞｰﾀ不正､99=その他異常
// // MH810104(S) R.Endo 2021/09/15 車番チケットレス フェーズ2.3 #5931 未清算ありのメッセージ表示
// 	uchar					byUnpaidHabitual;	// 未精算 常習者 有無
// 	uchar					byUnpaidCount;		// 未精算 件数
// // MH810104(E) R.Endo 2021/09/15 車番チケットレス フェーズ2.3 #5931 未清算ありのメッセージ表示
// 	stRecvCardInformation_t	dtReqRslt;			// ｶｰﾄﾞ情報_要求結果
// 
// 	stPayResultInfo_t		dtCalcInfo;			// 料金計算結果情報	Max = 684 Byte
// 	stTotalPayInfo_t		dtTotalPayInfo;		// 累計金額情報		Max = 600 Byte
// 	stMasterInfo_t			dtMasterInfo;		// ﾏｽﾀｰ情報			Max = 434 Byte
// 	stReserveInfo_t			dtReserveInfo;		// 予約情報			Max = 84 Byte
// 	stPassInfo_t			dtPassInfo;			// 定期･会員情報	Max = 16 Byte
// 	stZaishaInfo_t			dtZaishaInfo;		// 在車情報			Max = 668 Byte
// 
// } stRecvCardInfo_Rev10;

// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// Rev.No.11
//typedef struct _stRecvCardInfo_Rev11
//{
//	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.			0～65535
//	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
//	uchar					Reserve1;			// 予備						0固定
//
//	ushort					RsltReason;			// 結果理由					0 =正常(ｾﾝﾀｰがｾｯﾄ)/1 =【通信】ｾﾝﾀｰ?MLC間通信障害中(MLCがｾｯﾄ)/10=【ｾﾝﾀｰ】DB参照ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)/11=【ｾﾝﾀｰ】通信ﾃﾞｰﾀ解析ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)
//	ulong					AskMediaParkNo;		// 問合せ媒体 駐車場№		0～999999
//	ushort					AskMediaRevNo;		// 問合せ媒体 情報Rev.No.	0～65535
//	ushort					AskMediaKind;		// 問合せ媒体 種別			0～65000
//	uchar					byAskMediaNo[32];	// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）
//	uchar					ValidCHK_ETC;		// 有効性ﾁｪｯｸ ETCﾈｶﾞﾁｪｯｸ	0=ﾁｪｯｸなし､1=ETCﾈｶﾞOK､2=ETCﾈｶﾞNG
//	uchar					ValidCHK_Dup;		// 有効性ﾁｪｯｸ 割引重複利用ﾁｪｯｸ	0=ﾁｪｯｸなし､1=割引重複なし､2=割引重複あり､3=ﾃﾞｰﾀ不正､99=その他異常
//	uchar					byUnpaidHabitual;	// 未精算 常習者 有無
//	uchar					byUnpaidCount;		// 未精算 件数
//	stRecvCardInformation_t	dtReqRslt;			// ｶｰﾄﾞ情報_要求結果
//
//	stPayResultInfo_t		dtCalcInfo;			// 割引情報	 Max = 684 Byte
//	stTotalPayInfo_t		dtTotalPayInfo;		// 累計金額問合せ Max = 600 Byte
//	stMasterInfo_t			dtMasterInfo;		// マスター情報 Max = 434 Byte
//	stReserveInfo_t			dtReserveInfo;		// 予約情報 Max = 84 Byte
//	stPassInfo_t			dtPassInfo;			// 定期・会員情報 Max = 16 Byte
//	stZaishaInfo_t			dtZaishaInfo;		// 在車情報 Max = 668 Byte
//	stFeeCalcInfo_t			dtFeeCalcInfo;		// 料金計算結果 Max = 1113 Byte
//} stRecvCardInfo_Rev11;
typedef struct _stRecvCardInfo_RevXX
{
	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.			0～65535
	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
	uchar					Reserve1;			// 予備						0固定

	ushort					RsltReason;			// 結果理由					0 =正常(ｾﾝﾀｰがｾｯﾄ)/1 =【通信】ｾﾝﾀｰ?MLC間通信障害中(MLCがｾｯﾄ)/10=【ｾﾝﾀｰ】DB参照ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)/11=【ｾﾝﾀｰ】通信ﾃﾞｰﾀ解析ｴﾗｰ(ｾﾝﾀｰがｾｯﾄ)
	ulong					AskMediaParkNo;		// 問合せ媒体 駐車場№		0～999999
	ushort					AskMediaRevNo;		// 問合せ媒体 情報Rev.No.	0～65535
	ushort					AskMediaKind;		// 問合せ媒体 種別			0～65000
	uchar					byAskMediaNo[32];	// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）
	uchar					ValidCHK_ETC;		// 有効性ﾁｪｯｸ ETCﾈｶﾞﾁｪｯｸ	0=ﾁｪｯｸなし､1=ETCﾈｶﾞOK､2=ETCﾈｶﾞNG
	uchar					ValidCHK_Dup;		// 有効性ﾁｪｯｸ 割引重複利用ﾁｪｯｸ	0=ﾁｪｯｸなし､1=割引重複なし､2=割引重複あり､3=ﾃﾞｰﾀ不正､99=その他異常
	uchar					byUnpaidHabitual;	// 未精算 常習者 有無
	uchar					byUnpaidCount;		// 未精算 件数
	stRecvCardInformation_t	dtReqRslt;			// ｶｰﾄﾞ情報_要求結果

	stPayResultInfo_t		dtCalcInfo;			// 割引情報	 Max = 684 Byte
	stTotalPayInfo_t		dtTotalPayInfo;		// 累計金額問合せ Max = 600 Byte
	stMasterInfo_t			dtMasterInfo;		// マスター情報 Max = 434 Byte
	stReserveInfo_t			dtReserveInfo;		// 予約情報 Max = 84 Byte
	stPassInfo_t			dtPassInfo;			// 定期・会員情報 Max = 16 Byte
	stZaishaInfo_t			dtZaishaInfo;		// 在車情報 Max = 668 Byte
	stFeeCalcInfo_t			dtFeeCalcInfo;		// 料金計算結果 Max = 1113 Byte
} stRecvCardInfo_RevXX;
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

//-------------------------------------------------------------------------------------------------------------------------------------------------------
/****************************************/
/*		精算ﾃﾞｰﾀ	ｶｰﾄﾞ情報(送信)		*/
/*		Rev.No.10						*/
/****************************************/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// typedef struct _stSendCardInfo_Rev10
// {
// 	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.		0～65535
// 	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
// 	uchar					Reserve1;			// 予備					0固定
// 	stCardInformation_t		dtCardInfo;			// ｶｰﾄﾞ情報
// 			// 以下の情報は有として固定長でlog登録 & RXM -> LCD送信する
// 			//		uchar 	MasterOnOff;		// ｶｰﾄﾞ情報 ﾏｽﾀｰ情報  有無			0:無､1:有
// 			//		uchar 	ReserveOnOff;		// ｶｰﾄﾞ情報 予約情報  有無			0:無､1:有
// 			//		uchar 	PassOnOff;			// ｶｰﾄﾞ情報 定期･会員情報  有無		0:無､1:有
// 			//		uchar 	ZaishaOnOff;		// ｶｰﾄﾞ情報 在車情報  有無			0:無､1:有
// 
// 	stMasterInfo_t			dtMasterInfo;		// ★ﾏｽﾀｰ情報			Max = 434 Byte
// 	stReserveInfo_t			dtReserveInfo;		// ★予約情報			Max = 84 Byte
// 	stPassInfo_t			dtPassInfo;			// ★定期･会員情報		Max = 16 Byte
// 	stZaishaInfo_t			dtZaishaInfo;		// ★在車情報			Max = 668 Byte
// 
// } stSendCardInfo_Rev10;

// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// Rev.No.11
//typedef struct _stSendCardInfo_Rev11
//{
//	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.		0～65535
//	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
//	uchar					Reserve1;			// 予備					0固定
//	stCardInformation_t		dtCardInfo;			// ｶｰﾄﾞ情報
//			// 以下の情報は有として固定長でlog登録 & RXM -> LCD送信する
//			//		uchar 	MasterOnOff;		// ｶｰﾄﾞ情報 ﾏｽﾀｰ情報  有無			0:無､1:有
//			//		uchar 	ReserveOnOff;		// ｶｰﾄﾞ情報 予約情報  有無			0:無､1:有
//			//		uchar 	PassOnOff;			// ｶｰﾄﾞ情報 定期･会員情報  有無		0:無､1:有
//			//		uchar 	ZaishaOnOff;		// ｶｰﾄﾞ情報 在車情報  有無			0:無､1:有
//
//	stMasterInfo_t			dtMasterInfo;		// ★ﾏｽﾀｰ情報			Max = 434 Byte
//	stReserveInfo_t			dtReserveInfo;		// ★予約情報			Max = 84 Byte
//	stPassInfo_t			dtPassInfo;			// ★定期･会員情報		Max = 16 Byte
//	stZaishaInfo_t			dtZaishaInfo;		// ★在車情報			Max = 668 Byte
//} stSendCardInfo_Rev11;
typedef struct _stSendCardInfo_RevXX
{
	ushort					FormatNo;			// ﾌｫｰﾏｯﾄRev.No.		0～65535
	stDatetTimeYtoSec_t		dtTimeYtoSec;		// 年月日時分秒
	uchar					Reserve1;			// 予備					0固定
	stCardInformation_t		dtCardInfo;			// ｶｰﾄﾞ情報
			// 以下の情報は有として固定長でlog登録 & RXM -> LCD送信する
			//		uchar 	MasterOnOff;		// ｶｰﾄﾞ情報 ﾏｽﾀｰ情報  有無			0:無､1:有
			//		uchar 	ReserveOnOff;		// ｶｰﾄﾞ情報 予約情報  有無			0:無､1:有
			//		uchar 	PassOnOff;			// ｶｰﾄﾞ情報 定期･会員情報  有無		0:無､1:有
			//		uchar 	ZaishaOnOff;		// ｶｰﾄﾞ情報 在車情報  有無			0:無､1:有

	stMasterInfo_t			dtMasterInfo;		// ★ﾏｽﾀｰ情報			Max = 434 Byte
	stReserveInfo_t			dtReserveInfo;		// ★予約情報			Max = 84 Byte
	stPassInfo_t			dtPassInfo;			// ★定期･会員情報		Max = 16 Byte
	stZaishaInfo_t			dtZaishaInfo;		// ★在車情報			Max = 668 Byte
} stSendCardInfo_RevXX;
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
/****************************************/
/*		領収証データ 領収証情報			*/
/****************************************/
// RT領収証データ用割引
typedef struct _stReceiptDiscount {
	uchar	ParkingLotNo[3];		// 割引 駐車場№
	ushort	Kind;					// 割引 種別
	ushort	CardType;				// 割引 区分
	ushort	UsageCount;				// 割引 使用回数
	ushort	PrevUsageCount;			// 割引 前回使用回数
	ulong	DisAmount;				// 割引 割引額
	ulong	PrevUsageDisAmount;		// 割引 前回割引額
	ushort	FeeKindSwitchSetting;	// 割引 種別切換情報
	ulong	DisType;				// 割引 割引情報1
	ulong	DisType2;				// 割引 割引情報2
	uchar	EligibleInvoiceTaxable;	// 割引 適格請求書 課税対象
} stReceiptDiscount;

// 領収証情報
typedef	struct _stReceiptInfo {
	ushort					FormatNo;							// フォーマットRev.No.
	stDatetTimeYtoSec_t		dtTimeYtoSec;						// 処理 年月日時分秒
	uchar					Reserve1;							// 予備
	ushort					ModelCode;							// 精算機 機種コード
	ulong					MachineNo;							// 精算機 機械№
	ulong					IndividualNo;						// 精算№ 個別追番
	ulong					ConsecutiveNo;						// 精算№ 通し追番
	stDate_YYMDHM_t			dtEntry;							// 入庫年月日時分
	stDate_YYMDHM_t			dtOldPayment;						// 前回精算年月日時分
	stDate_YYMDHM_t			dtPayment;							// 精算年月日時分
	stDate_YYMDHM_t			dtExitDateTime;						// 出庫年月日時分
	uchar					SettlementModeMode;					// 精算モード
	uchar					FeeKind;							// 料金種別
	ulong					ParkingTime;						// 駐車時間
	ulong					ParkingFee;							// 駐車料金
	ulong					SeasonParkingLotNo;					// 定期券 駐車場№
	ushort					SeasonKind;							// 定期券 種別
	ulong					SeasonId;							// 定期券 ID
	stDate_YYMD_t			stSeasonValidStart;					// 定期券 有効開始年月日
	stDate_YYMD_t			stSeasonValidEnd;					// 定期券 有効終了年月日
	uchar					VehicleNumberSerialNo[12];			// 車番 一連指定番号
	uchar					UniqueID[6];						// 車番チケットレス ユニークID（入出庫ID）
	ulong					PaidFeeAmount;						// 支払済み料金
	ulong					Parking_Bil;						// 合計金額（請求額）
	ushort					CardPaymentCategory;				// カード決済 区分
	uchar					CardPaymentTerminalCategory;		// カード決済 端末区分
	uchar					CardPaymentTransactionStatus;		// カード決済 取引ステータス
	stDatetTimeYtoSec_t		dtCardPayment;						// カード決済 年月日時分秒
	uchar					Reserve2;							// 予備
	union {
		struct {
			ulong			amount;								// 利用金額
			uchar			card_no[20];						// 会員番号
			uchar			cct_num[16];						// 端末識別番号
			uchar			kid_code[6];						// KIDコード
			ulong			app_no;								// 承認番号
			ulong			center_oiban;						// センター処理通番
			uchar			ShopAccountNo[20];					// 加盟店取引番号
			ulong			slip_no;							// 伝票番号
			uchar			reserve[2];							// 予備
		} credit;
		struct {
			ulong			amount;								// 利用金額
			uchar			card_id[30];						// カード番号
			ulong			card_zangaku;						// カード残額
			uchar			inquiry_num[16];					// 問い合わせ番号
			ulong			approbal_no;						// 承認番号
			uchar			terminal_id[13];					// 物販端末ID
			uchar			reserve[9];							// 予備
		} emoney;
		struct {
			ulong			amount;								// 利用金額
			uchar			MchTradeNo[32];						// Mch取引番号
			uchar			PayTerminalNo[16];					// 支払端末ID
			uchar			DealNo[16];							// 取引番号
			uchar			PayKind;							// 決済ブランド
			uchar			reserve[11];						// 予備
		} qr;
	} card;
	ushort					EligibleInvoiceHasInfo;				// 適格請求書 情報有無
	uchar					EligibleInvoiceRegistrationNo[14];	// 適格請求書 登録番号
	ulong					EligibleInvoiceTaxableAmount;		// 適格請求書 課税対象額(税込み)
	ushort					EligibleInvoiceTaxRate;				// 適格請求書 適用税率
	ulong					EntryMachineNo;						// 発券機番号 [Rev.No.1追加]
	ulong					ParkingTicketNo;					// 駐車券番号 [Rev.No.1追加]
	ulong					CashReceiptAmount;					// 現金領収額 [Rev.No.1追加]
	ulong					WInPrice;							// 預り金額 [Rev.No.1追加]
	ulong					WChgPrice;							// 釣り銭額 [Rev.No.1追加]
	ulong					WFusoku;							// 払戻不足金額 [Rev.No.1追加]
	stReceiptDiscount		arrstDiscount[25];					// 割引01-25
	uchar					Reserve3[2];						// 暗号化 バイト調整 [Rev.No.1変更]
} stReceiptInfo;

// QR領収証
#define QRCODE_RECEIPT_HEADER_SIZE 128
#define QRCODE_RECEIPT_ENCRYPT_SIZE 80
#define QRCODE_RECEIPT_SIZE (QRCODE_RECEIPT_HEADER_SIZE + (QRCODE_RECEIPT_ENCRYPT_SIZE * 2))
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

//-------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------
//		コマンド構造体定義
//		（ メイン基板 ==> LCDモジュール ）
//--------------------------------------------------

//--------------------------------------------------
//		コマンド共通領域
//--------------------------------------------------
/**
 *	LCDモジュール宛コマンドの共通部分 構造体
 */
typedef	struct {
	unsigned short	length;		///< コマンド長
	unsigned short	id;			///< コマンドＩＤ
	unsigned short	subid;		///< サブコマンドＩＤ
} lcdbm_cmd_base_t;

//--------------------------------------------------
//		ステータス要求
//--------------------------------------------------
/**
 *	基本設定要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	// コマンド共通情報
	unsigned char		rsw;		// RSW状態
// MH810100(S) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
//	unsigned char		ver[10];	// プログラムバージョン
	unsigned char		ver[8];		// プログラムバージョン
	unsigned char		opemod;		// OpeのMode
	unsigned char		reserved;	// 
// MH810100(E) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
} lcdbm_cmd_config_t;

//--------------------------------------------------
//		機器設定
//--------------------------------------------------
/**
 *	時計設定
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned short		year;		///< 西暦年
	unsigned char		month;		///< 月
	unsigned char		day;		///< 日
	unsigned char		hour;		///< 時
	unsigned char		minute;		///< 分
	unsigned char		second;		///< 秒
	unsigned char		reserved1;	///< 予約
	unsigned char		reserved2;	///< 予約
	unsigned char		reserved3;	///< 予約
} lcdbm_cmd_clock_t;

/**
 *	LCD輝度
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned short		brightness;	///< 輝度値(0～15)
} lcdbm_cmd_brightness_t;

//--------------------------------------------------
//		テキスト表示
//--------------------------------------------------
// テキスト表示コマンド編集バッファのサイズ(1パケット送信データ最大サイズ-電文長(2Byte)-コマンドヘッダサイズ(6Byte))
#define	TEXT_CMD_BUF_SIZE	(PKT_CMD_MAX-2-6)

/**
 *	テキスト表示コマンド構造体
 *	@attention	テキストデータ作成を考慮して lcdbm_cmd_work_t構造体には含まない。
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		buf[ TEXT_CMD_BUF_SIZE ];	///< テキスト表示コマンドを作るためのバッファ領域
} lcdbm_cmd_text_t;

//--------------------------------------------------
//		アナウンス要求
//--------------------------------------------------
/** WAVE再生で１コマンド内に指定できる文節数 */
#define	LCDBM_CMD_AUDIO_PHRASES		10

///** 音声合成で１コマンド内に指定できる文節数 */
//#define	LCDBM_CMD_SPEECH_PHRASES	10
//
///** 音声合成で使うチャネル番号 */
//#define	LCDBM_CMD_SPEECH_CHANNEL	0

/**
 *	音量設定
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		volume;		///< 音量(0～100(0=無音))
	unsigned char		reserved1;	///< 予約
} lcdbm_cmd_audio_volume_t;

/**
 *	開始要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		times;		///< 放送回数(0，1～9回，FFH(0=放送なし，FFH=次の放送開始まで繰り返す))
	unsigned char		condition;	///< 放送開始条件
									///<   0=現在の放送を中断して放送を開始する。
									///<   1=予約
									///<   2=待ち状態の放送を含めすべての放送終了後に開始する
	unsigned char		interval;	///< メッセージ間隔(0～15秒)
	unsigned char		channel;	///< 放送チャネル(0固定)
	unsigned char		reserved1;	///< 予備
	unsigned char		reserved2;	///< 予備
	unsigned char		language;	///< 言語指定(0～99=言語番号)
	unsigned char		AnaKind;	///< 種別(0=メッセージ№/1=文節№)
	unsigned short		AnaNo;		///< メッセージ№/文節№
} lcdbm_cmd_audio_start_t;

/**
 *	終了要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		channel;	///< 放送終了チャネル(0固定)
	unsigned char		abort;		///< 中断方法(0固定)
	unsigned char		reserved1;	///< 予備
	unsigned char		reserved2;	///< 予備
} lcdbm_cmd_audio_end_t;

//--------------------------------------------------
//		ブザー要求
//--------------------------------------------------
/**
 *	音量設定
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		volume;		///< 音量(0～3(0=無音，3=最大音量))
	unsigned char		reserved1;	///< 予備
} lcdbm_cmd_beep_volume_t;

/**
 *	ブザー鳴動要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		beep;		///< 種類(0=ピッ♪,1=ピピィ♪,2=ピピピィ♪)
	unsigned char		reserved1;	///< 予備
} lcdbm_cmd_beep_start_t;

/**
 *	警報鳴動要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		kind;		///< 種類(0=固定)
	unsigned char		type;		///< 種別(0=開始,1=停止)
} lcdbm_cmd_alarm_start_t;

//--------------------------------------------------
//		精算機状態通知操作
//--------------------------------------------------
/**
 *	操作通知
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		ope_code;	///< 操作コード(下記参照)
	unsigned short		status;		///< 状態(下記参照)
									///<   操作コード|データの種類			|状態
									///<   ==========+======================+=============================================
									///<   		    1|精算中止				|0固定
									///<   		    2|精算中止応答			|0=OK/1=NG
									///<   		    3|精算開始				|0固定
									///<   		    4|精算開始応答			|0=OK/1=NG
									///<   		    5|人体検知				|0=OFF/1=ON
									///<   		    6|ドア状態				|0=ドア閉/1=ドア開
									///<   		    7|領収証発行			|0固定
									///<   		    8|遠隔精算開始			|0固定
									///<   		    9|遠隔精算開始応答		|0=OK/1=NG
									///<   		   10|営休業				|0=営業/1=休業
									///<   		   11|起動通知				|0=通常起動 1=設定ｱｯﾌﾟﾛｰﾄﾞ起動
									///<   		   12|起動完了通知			|0固定
									///<   		   13|設定アップロード通知	|0=OK/1=NG
									///<   		   14|設定アップロード要求	|0固定
									///<   		   15|リセット要求			|0固定
									///<   		   16|リザーブ1_受信
									///<   		   17|リザーブ1_送信
									///<   		   18|メンテナンス開始要求	|0固定
									///<   		   19|メンテナンス開始応答	|0=OK/1=NG
									///<   		   20|メンテナンス終了要求	|0x00=待機/それ以外は警告通知のbit
									///<   		   21|メンテナンス終了応答	|0=OK/1=NG
									///<   		   22|精算完了通知			|0=0円表示/1=ラグタイム延長画面/2=精算済み案内
									///<   		   23|QR停止要求			|0固定
									///<   		   24|QR停止応答			|0=OK/1=NG
									///<   		   25|QR開始要求			|0固定
									///<   		   26|QR開始応答			|0=OK/1=NG
									///<   		   27|精算案内終了通知		|0固定
									///<   		   28|精算案内終了応答		|0固定
									///<   		   29|リセット通知			|0固定
} lcdbm_cmd_notice_ope_t;

/**
 *	営休業通知
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		opn_cls;	///< 営休業(0=営業,1=休業)
	unsigned char		reason;		///< 休業理由(下記参照)
									///<    0:
									///<    1:強制営休業
									///<    2:営業時間外
									///<    3:紙幣金庫満杯
									///<    4:コイン金庫満杯
									///<    5:紙幣リーダー脱落
									///<    6:釣銭切れ
									///<    7:コインメック通信不良
									///<    8:送信バッファフル
									///<    9:(予約)
									///<   10:信号による休業
									///<   11:通信による休業
} lcdbm_cmd_notice_opn_t;

/**
 *	精算状態通知
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned long		event;		///< 事象(Bit単位:0=解除,1=発生)
									///<   b0:釣銭不足
									///<   b1:レシート用紙不足
									///<   b2:ジャーナル用紙不足
									///<   b3:Suica利用不可
									///<   b4:クレジット利用不可
									///<   b5:Edy利用不可
									///<   b6 ～31:予備
} lcdbm_cmd_notice_pay_t;

/**
 *	ポップアップ表示要求
 */
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
#define LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE 484
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
typedef	struct {
	lcdbm_cmd_base_t	command;	// コマンド共通情報
	unsigned char		text_code;	// コード(下記参照)
									//   1:預り証発行(精算完了時)
									//   2:預り証発行(払い戻し時)
									//   3:減額失敗
									//   4:限度超え
									//   5:クレジット照会NG
									//   6:クレジットサーバ異常
									//   7:残高不足
	unsigned char		status;		// 状態(下記参照)
									// テキストコード=1/2の場合
									//   0:釣銭なし
									//   1:釣銭あり
									//   2:釣銭なし(領収証)
									//   3:釣銭あり(領収証)
									// テキストコード=5の場合
									//   1:取扱不可
									//   2:無効カード
									//   3:使用不可
									//   4:期限切れ
									//   5:読み取りNG
									//   6:残高不足
	unsigned long		add_info;	// 付加情報
									//   コード=5、状態=6の場合…カード残高
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	unsigned char		add_str[LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE];	// 追加文字列
									//   コード=12の場合… QR領収証の文字列
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
} lcdbm_cmd_notice_dsp_t;

/**
 *	警告通知
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned short		event;		///< 事象(Bit単位:0=解除,1=発生)
									///<   b0:コイン金庫未セット
									///<   b1:コインカセット未セット
									///<   b2:コインメック未施錠
									///<   b3:紙幣リーダー蓋開
									///<   b4:紙幣リーダー未施錠
									///<   b5:レシートプリンタ蓋開
									///<   b6:ジャーナルプリンタ蓋開
									///<   b7:RSWが0以外
									///<   b8～15:予備
} lcdbm_cmd_notice_alm_t;

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
/**
 *	ポップアップ削除要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< コマンド共通情報
	unsigned char		kind;		///< 種類(下記参照)
									///<   1:領収証発行POP
	unsigned char		status;		///< 状態(下記参照)
									///<   1:印字完了
} lcdbm_cmd_notice_del_t;
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

//--------------------------------------------------
//		精算・割引情報通知
//--------------------------------------------------
/**
 *	精算残高変化通知
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			// コマンド共通情報
	unsigned long		id;					// ID(入庫から精算完了までを管理するID)
	unsigned char		pay_sts;			// 精算ステータス(0:精算中,1:精算完了,2:精算済,3:精算不可)
	DATE_YMDHMS			pay_time;			// 精算年月日時分秒
	unsigned long		prk_fee;			// 駐車料金
	unsigned long		pay_rem;			// 精算残額
	unsigned long		cash_in;			// 現金投入額
	unsigned long		chg_amt;			// 釣銭額
	unsigned long		disc_amt;			// 割引額
// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
	unsigned char		season_chk_result;	// 定期チェック結果
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
// MH810103(s) 電子マネー対応
    unsigned char		method;         	// 支払い方法
    unsigned char		brand;          	// ブランド
    unsigned long		emoney_balance; 	// 電子マネー残高
// MH810103(e) 電子マネー対応
// MH810103(s) 電子マネー対応 #5465 再精算で電子マネーを使用した精算完了時、精算機画面上の引き去り金額に初回精算分も含んでしまう
    unsigned long		emoney_pay; 		// 電子マネー決済金額
// MH810103(e) 電子マネー対応 #5465 再精算で電子マネーを使用した精算完了時、精算機画面上の引き去り金額に初回精算分も含んでしまう
// GG129000(S) R.Endo 2024/01/12 #7217 お買物合計表示対応
	unsigned long		Kaimono;			// 買物金額合計
// GG129000(E) R.Endo 2024/01/12 #7217 お買物合計表示対応
// GG129004(S) M.Fujikawa 2024/10/22 買物金額割引情報対応
	unsigned long		Kaimono_info;		// 買物金額割引情報
// GG129004(E) M.Fujikawa 2024/10/22 買物金額割引情報対応
} lcdbm_cmd_pay_rem_chg_t;

/**
 *	精算データ
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ｺﾏﾝﾄﾞ共通情報
	unsigned long			id;					///< ID(入庫から精算完了までを管理するID)
	unsigned long			CenterOiban;		///< センター追番
	unsigned short			CenterOibanFusei;	///< センター追番不正ﾌﾗｸﾞ
	unsigned char			shubetsu;			///< 種別(0=事前精算／1＝未精算出庫精算)
	unsigned short			crd_info_rev_no;	///< ｶｰﾄﾞ情報Rev.No.
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// 	stSendCardInfo_Rev10	crd_info;			///< ｶｰﾄﾞ情報(精算ﾃﾞｰﾀ)
// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	stSendCardInfo_Rev11	crd_info;			///< ｶｰﾄﾞ情報(精算ﾃﾞｰﾀ)
	stSendCardInfo_RevXX	crd_info;			///< ｶｰﾄﾞ情報(精算ﾃﾞｰﾀ)
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
} lcdbm_cmd_pay_data_t;

#define PADSIZE	16-(2+sizeof(lcdbm_cmd_pay_data_t))%16	// 暗号化時の0パディングサイズ(16Byte単位とする)

typedef	struct {
	unsigned short			data_len;		///< 電文長
	lcdbm_cmd_pay_data_t	pay_dat;		///< RT精算データ
	unsigned char			pad[PADSIZE];	///< 16Byte単位とする為の0パディング用エリア
} lcdbm_cmd_pydt_0pd_t;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
/**
 *	領収証データ
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ｺﾏﾝﾄﾞ共通情報
	unsigned long			id;					///< ID(入庫から精算完了までを管理するID)
	unsigned long			CenterOiban;		///< センター追番
	unsigned short			CenterOibanFusei;	///< センター追番不正フラグ
	stReceiptInfo			receipt_info;		///< 領収証情報
} lcdbm_cmd_receipt_data_t;

#define RECEIPT_PADSIZE	(16-(2+sizeof(lcdbm_cmd_receipt_data_t))%16)	// 暗号化時の0パディングサイズ(16Byte単位とする)

typedef	struct {
	unsigned short				data_len;				///< 電文長
	lcdbm_cmd_receipt_data_t	receipt_dat;			///< RT領収証データ
	unsigned char				pad[RECEIPT_PADSIZE];	///< 16Byte単位とする為の0パディング用エリア
} lcdbm_cmd_receiptdt_0pd_t;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

/*--------------------------------------------------------------------------*/
/* QR確定・取消データ														*/
/*--------------------------------------------------------------------------*/
//  割引認証情報 (１つ分)
typedef struct{
	ulong			ParkingNo;				// 駐車場№
	ushort			Kind;					// 種別
	ushort			CardType;				// カード区分
	uchar			Type;					// 区分
	uchar			reserve;				// 予備(サイズ調整用)
	ushort			DiscInfo;				// 割引情報
} QR_Discount;

// QRレシート情報
typedef struct{
	ulong			FacilityCode;			// 施設コード
	ulong			PosNo;					// 端末番号
	DATE_YMDHMS		IssueDate;				// レシート発行年月日時分秒
	uchar			reserve;				// 予備(サイズ調整用)
	ulong			Amount;					// 買上金額
	ushort			HouseKind;				// ハウスカード種別
	QR_Discount		Disc;					// 施設利用割引
} QR_Receipt;

// QR割引券情報
typedef struct{
	ulong			IssueNo;				// 割引券発行追番
	DATE_YMDHMS		IssueDate;				// 割引券発行年月日時分秒
	uchar			reserve;				// 予備(サイズ調整用)
	QR_Discount		Disc;					// 割引券割引
} QR_DiscTicket;

// QRハウスカード情報
typedef struct{
	ushort			Kind;					// カード種別
	uchar			CardNo[32];				// カード番号
} QR_HouceCard;

// ------------------------------ //
// 割引認証登録・取消要求データ
// ------------------------------ //
#define		BAR_DATASIZE	512			// QR生データサイズ
#define		BAR_INFO_SIZE	128			// パース後のデータサイズ

typedef struct{
	ushort			FormRev;				// フォーマットRev.№
	DATE_YMDHMS		CertDate;				// 認証確定/中止年月日時分秒
	uchar			reserve1;				// 予備(サイズ調整用)
	ushort			ProcessKind;			// 処理区分
	struct{
		ulong		ParkingNo;				// 駐車場№
		ushort		Kind;					// 種別
		uchar		CardNo[32];				// カード番号
// MH810100(S) K.Onodera  2020/02/27 #3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう
//		uchar		CardInfo[16];			// カード情報
// MH810100(E) K.Onodera  2020/02/27 #3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう
	} in_media;
// MH810100(S) K.Onodera  2020/02/27 #3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう
	uchar			Passkind;				// 定期種別
	uchar			MemberKind;				// 会員種別
	ulong			InParkNo;				// 入庫駐車場№
// MH810100(E) K.Onodera  2020/02/27 #3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう
	DATE_YMDHMS		InDate;					// 入庫年月日時分秒
	uchar			reserve2;				// 予備(サイズ調整用)
	ushort			ReciptCnt;				// レシート枚数(0～99=認証問合せOKとなったレシート枚数をセット)
	uchar			Method;					// 割引方法(0=オンラインデータ、1=オフラインデータ、2=AR-100／150 )
	uchar			Lastflg;				// 最終フラグ(0=認証開始データ、1=認証途中データ、2=認証最終データ)
	uchar			CertCnt;				// 割引認証数(0～25)
	uchar			reserve3;				// 予備(サイズ調整用)
	ushort			DiscountInfoSize;		// 割引認証情報データサイズ
	QR_Discount		DiscountInfo[25];		// 割引認証情報
	ushort			QR_ID;					// QRコードID
	ushort			QR_FormRev;				// QRコードフォーマットRev.№
	uchar			QR_type;				// QR種別
	ushort			QR_data_size;			// QR情報データサイズ(パースデータ)
	ushort			QR_row_size;			// QRコードデータサイズ(生データ)
	uchar			QR_data[BAR_INFO_SIZE];
	uchar			QR_row_data[BAR_DATASIZE];
} DC_QR_Info;

typedef	struct {
	ulong			ID;						// 入庫から精算完了までを管理するID
	ulong			CenterOiban;			// センター追番
	ushort			CenterOibanFusei;		// センター追番不正ﾌﾗｸﾞ
	ushort			QR_Info_Rev;			// QR情報Rev.№
	DC_QR_Info		QR_Info;				// QR情報

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	// 本構造体を変更する場合は必ず DC_QR_log_date も修正する
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
} DC_QR_log;
extern	DC_QR_log		DC_QR_work;			// QR確定・取消データ登録用ワークバッファ

// LOGレコード日付用
typedef	struct {
	unsigned long		ID;					// 入庫から精算完了までを管理するID
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	ulong				CenterOiban;		// センター追番
	ushort				CenterOibanFusei;	// センター追番不正ﾌﾗｸﾞ
	ushort				QR_Info_Rev;		// QR情報Rev.№
	ushort				FormRev;			// フォーマットRev.№
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// 年月日時分秒
} DC_QR_log_date;

/**
 *	QR確定・取消データ
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ｺﾏﾝﾄﾞ共通情報
	unsigned long			id;					///< ID(入庫から精算完了までを管理するID)
	unsigned long			CenterOiban;		///< センター追番
	unsigned short			CenterOibanFusei;	///< センター追番不正ﾌﾗｸﾞ
	unsigned short			crd_info_rev_no;	///< ｶｰﾄﾞ情報Rev.No.
	DC_QR_Info				QR_code_info;		///< QRコード情報
} lcdbm_cmd_QR_conf_can_t;

#define QR_PADSIZE	16-(2+sizeof(lcdbm_cmd_QR_conf_can_t))%16	// 暗号化時の0パディングサイズ(16Byte単位とする)

typedef	struct {
	unsigned short			data_len;			// 電文長
	lcdbm_cmd_QR_conf_can_t	qr_dat;				// QRデータ
	unsigned char			pad[QR_PADSIZE];	// 16Byte単位とする為の0パディング用エリア
} lcdbm_cmd_qrdt_0pd_t;

/**
 *	QRデータ応答
 */
typedef	struct {
	lcdbm_cmd_base_t		command;		///< ｺﾏﾝﾄﾞ共通情報
	unsigned long			id;				///< ID(入庫から精算完了までを管理するID)
	unsigned char			result;			///< 結果(0:OK,1:NG(排他),2:NG(枚数上限))
	unsigned char			reserve;		///< 予備
} lcdbm_cmd_QR_data_res_t;

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 入庫時刻指定遠隔精算開始
typedef	struct {
	lcdbm_cmd_base_t	command;	// 共通情報
	unsigned long		ulPno;		// 駐車場№
	unsigned short		RyoSyu;		// 料金種別
	DATE_YMDHMS			InTime;		// 入庫年月日時分秒
	unsigned char		reserve;	// 予備(サイズ調整用)
} lcdbm_cmd_remote_time_start_t;
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
/**
 *	レーンモニタデータ(レーンモニタ情報部分)
 */
typedef struct{
	ushort			FormRev;				// フォーマットRev.№
	DATE_YMDHMS		ProcessDate;			// 発生年月日時分秒
	uchar			reserve1;				// 予備(サイズ調整用)
	ulong			StsSeqNo;				// 状態一連番号
	ulong			MediaParkNo;			// 媒体 駐車場№
	ushort			Mediasyu;				// 媒体 種別
	uchar			MediaNo[32];			// 媒体 番号
	ushort			PaySyu;					// 料金種別
	uchar			StsSyu;					// 状態種別
	uchar			StsCode;				// 状態コード
	ulong			FreeNum;				// フリー数値
	uchar			FreeStr[60];			// フリー文字
	uchar			StsName[30];			// 状態名
	uchar			StsMese[150];			// 状態メッセージ
	uchar			AES_Reserve[4];
} DC_LANE_Info;

/**
 *	レーンモニタデータ（レーンモニタデータ登録用）
 */
typedef	struct {
	ulong			ID;						// 入庫から精算完了までを管理するID
	ulong			CenterOiban;			// センター追番
	ushort			CenterOibanFusei;		// センター追番不正ﾌﾗｸﾞ
	DC_LANE_Info	LANE_Info;				// レーンモニタ情報

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	// 本構造体を変更する場合は必ず DC_LANE_log_date も修正する
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
} DC_LANE_log;
extern	DC_LANE_log		DC_LANE_work;		// レーンモニタデータ登録用ワークバッファ

/**
 *	レーンモニタデータ(レーンモニタ情報作成用)
 */
struct stLaneDataInfo {
	ulong			MediaParkNo;			// 媒体 駐車場№
	ushort			Mediasyu;				// 媒体 種別
	uchar			MediaNo[32];			// 媒体 番号
	ushort			PaySyu;					// 料金種別
	ulong			FreeNum;				// フリー数値
	uchar			FreeStr[60];			// フリー文字
};
extern struct stLaneDataInfo m_stLaneWork;

// LOGレコード日付用
typedef	struct {
	unsigned long		ID;					// 入庫から精算完了までを管理するID
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	ulong				CenterOiban;		// センター追番
	ushort				CenterOibanFusei;	// センター追番不正ﾌﾗｸﾞ
	ushort				FormRev;			// フォーマットRev.№
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// 年月日時分秒
} DC_LANE_log_date;

/**
 *	レーンモニタデータ(RXM-LCD間)
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< ｺﾏﾝﾄﾞ共通情報
	unsigned long		id;					///< ID(入庫から精算完了までを管理するID)
	unsigned long		CenterOiban;		///< センター追番
	unsigned short		CenterOibanFusei;	///< センター追番不正ﾌﾗｸﾞ
	DC_LANE_Info		LANE_Info;			///< レーンモニタ情報
} lcdbm_cmd_lane_t;

#define LANE_PADSIZE	16-(2+sizeof(lcdbm_cmd_lane_t))%16	// 暗号化時の0パディングサイズ(16Byte単位とする)

typedef	struct {
	unsigned short			data_len;			// 電文長
	lcdbm_cmd_lane_t		lane_dat;			// レーンモニタデータ
	unsigned char			pad[LANE_PADSIZE];	// 16Byte単位とする為の0パディング用エリア
} lcdbm_cmd_lanedt_0pd_t;

// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

//--------------------------------------------------
//		メンテナンスデータ
//--------------------------------------------------
/**
 *	QRリーダ制御要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			// コマンド共通情報
	unsigned char		ctrl_cd;			// 制御コード(0:バージョン要求,1:読取開始,2:読取停止)
	unsigned char		reserve;			// 予備
} lcdbm_cmd_QR_ctrl_req_t;

/**
 *	リアルタイム通信疎通要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< コマンド共通情報
} lcdbm_cmd_rt_con_req_t;


/**
 *	DC-NET通信疎通要求
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< コマンド共通情報
} lcdbm_cmd_dc_con_req_t;
//--------------------------------------------------
//		コマンド構造体定義
//		（ LCDモジュール ==> メイン基板 ）
//--------------------------------------------------

//--------------------------------------------------
//		コマンド共通領域
//--------------------------------------------------
/**
 *	LCDモジュールからの応答データの共通部分 構造体
 */
typedef	struct {
	unsigned short	id;			///< コマンドＩＤ
	unsigned short	subid;		///< サブコマンドＩＤ
} lcdbm_rsp_base_t;

//--------------------------------------------------
//		QR関連データ共通領域
//--------------------------------------------------
#define		BAR_ID_AMOUNT	(10000)		// QR買上券
#define		BAR_ID_DISCOUNT	(10001)		// QR割引券
// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
#define		BAR_ID_TICKET	(10002)		// QR駐車券
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
#define QR_FORMAT_STANDARD 	(1)		// QRコード 標準フォーマット
#define QR_FORMAT_CUSTOM 	(2)		// QRコード 個別共通フォーマット
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）

/**
 *	LCDモジュールからのQR関連データの共通部分 構造体
 */
// バーコード割引データ
typedef struct{
	uchar			type;		// バーコードタイプ１
	uchar			reserve;	// 予備
	ulong			data;		// データ1
} QR_DiscountData;
// ---------------------- //
// QRデータ情報 QR買上券
// ---------------------- //
typedef struct{
	ulong			FacilityCode;	// 施設コード
	ulong			ShopCode;		// 店舗コード
	ulong			PosNo;			// 店舗内端末番号
	ulong			IssueNo;		// レシート発行追番
	DATE_YMDHMS		IssueDate;		// QR発行年月日時分秒
	uchar			DataCount;		// データ数
	QR_DiscountData	BarData[3];		// バーコードデータ
} QR_AmountInfo;

// ---------------------- //
// QRデータ情報 QR割引券
// ---------------------- //
typedef struct{
// MH810100(S) 2020/06/10 仕様変更 #4206 【連動評価指摘事項】システム：QRコードがユニークにならない場合がある
	ulong			FacilityCode;	// 施設コード
	ulong			ShopCode;		// 店舗コード
	ulong			PosNo;			// 店舗内端末番号
	ulong			IssueNo;		// レシート発行追番
// MH810100(E) 2020/06/10 仕様変更 #4206 【連動評価指摘事項】システム：QRコードがユニークにならない場合がある
	DATE_YMDHMS		IssueDate;		// QR発行年月日時分秒
	uchar			reserve;		// 予備（サイズ調整用）
	ulong			ParkingNo;		// 駐車場番号
	ushort			DiscKind;		// 割引種別
	ushort			DiscClass;		// 割引区分
	ushort			ShopNp;			// 店番号
	DATE_YMD		StartDate;		// 有効開始年月日
	DATE_YMD		EndDate;		// 有効終了年月日
} QR_DiscountInfo;

// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
// ---------------------- //
// QRデータ情報 QR駐車券
// ---------------------- //
typedef struct{
	ulong			ParkingNo;			// 駐車場番号
	ushort			EntryMachineNo;		// 発券機番号
	ulong			ParkingTicketNo;	// 駐車券番号
	uchar			TicketLessMode;		// 非連動（車番チケットレス連動有無）
	DATE_YMDHMS		IssueDate;			// QR発行年月日時分秒
} QR_TicketInfo;
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）

typedef union{
	uchar				data[BAR_INFO_SIZE];	// サイズ固定用
	QR_AmountInfo		AmountType;				// QRデータ情報 QR買上券
	QR_DiscountInfo		DiscountType;			// QRデータ情報 QR割引券
// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
	QR_TicketInfo		TicketType;				// QRデータ情報 QR駐車券
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
} lcdbm_rsp_QR_com_u;

// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
// 期限変更用構造体
typedef struct{
	DATE_YMDHMS		IssueDate;		// QR発行年月日時分秒
	DATE_YMD		StartDate;		// 有効開始年月日
	DATE_YMD		EndDate;		// 有効終了年月日
} QR_YMDData;
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）

// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
//--------------------------------------------------
//		車番情報
//--------------------------------------------------
typedef struct {
	uchar	LandTransOfficeName[12];	// 車番　陸運支局名
	uchar	ClassNum[9];				// 車番　分類番号
	uchar	Reserve1;					// 予備
	uchar	UsageCharacter[3];			// 車番　用途文字
	uchar	Reserve2;					// 予備
	uchar	SeqDesignNumber[12];		// 車番　一連指定番号
	uchar	Reserve3[10];				// 予備
} SHABAN_INFO;
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット

//--------------------------------------------------
//		ステータス通知
//--------------------------------------------------
#define	PKT_VER_LEN	10

/**
 *	基本設定応答
 */
typedef struct {
	lcdbm_rsp_base_t	command;							///< コマンド共通情報
	unsigned char		prgm_ver[PKT_VER_LEN];				///< プログラムバージョン(LCD制御モジュールソフトウェアのバージョン情報(ASCI))
	unsigned char		audio_ver[PKT_VER_LEN];				///< 音声データバージョン(音声データのバージョン情報(ASCI))
	unsigned long		lcd_startup_time;					///< LCDモジュール起動時間
	unsigned short		lcd_brightness;						///< LCD輝度(0～255)
	unsigned char		reserved[10];						///< 予備
} lcdbm_rsp_config_t;

/**
 *	エラー通知
 */
typedef struct {
	lcdbm_rsp_base_t	command;	///< コマンド共通情報
	unsigned short		kind;		///< エラー種別
	unsigned short		code;		///< エラーコード
	unsigned char		occur;		///< 発生/解除(1:発生,2:解除,3:発生解除同時)
	unsigned short		year;		///< 西暦年(2000～2099)
	unsigned char		month;		///< 月(01～12)
	unsigned char		day;		///< 日(01～31)
	unsigned char		hour;		///< 時(00～23)
	unsigned char		minute;		///< 分(00～59)
	unsigned char		second;		///< 秒(00～59)
	unsigned char		info[10];	///< エラー情報
} lcdbm_rsp_error_t;

//--------------------------------------------------
//		テンキー制御
//--------------------------------------------------
/**
 *	テンキー押下情報
 */
typedef	struct {
	lcdbm_rsp_base_t	command;	///< コマンド共通情報
	///< ボタン状態(Bit単位:0=通常状態(離された),1=押下状態(押された))
	unsigned char 		bit15:1;  	///< テンキーC
	unsigned char 		bit14:1;  	///< テンキーF5
	unsigned char 		bit13:1;  	///< テンキーF4
	unsigned char		bit12:1;  	///< テンキーF3
	unsigned char 		bit11:1;  	///< テンキーF2
	unsigned char 		bit10:1;  	///< テンキーF1
	unsigned char 		bit9:1;  	///< テンキー9
	unsigned char 		bit8:1; 	///< テンキー8
	unsigned char 		bit7:1;  	///< テンキー7
	unsigned char 		bit6:1;  	///< テンキー6
	unsigned char 		bit5:1;  	///< テンキー5
	unsigned char		bit4:1;  	///< テンキー4
	unsigned char 		bit3:1;  	///< テンキー3
	unsigned char 		bit2:1;  	///< テンキー2
	unsigned char 		bit1:1;  	///< テンキー1
	unsigned char 		bit0:1; 	///< テンキー0
} lcdbm_rsp_tenkey_info_t;

//--------------------------------------------------
//		精算機状態通知操作
//--------------------------------------------------
/**
 *	操作通知
 */
typedef	struct {
	lcdbm_rsp_base_t	command;	///< コマンド共通情報
	unsigned char		ope_code;	///< 操作コード(lcdbm_cmd_notice_ope_t参照)
	unsigned short		status;		///< 状態(lcdbm_cmd_notice_ope_t参照)
} lcdbm_rsp_notice_ope_t;

//--------------------------------------------------
//		精算・割引情報通知
//--------------------------------------------------
/**
*	入庫情報(割引後情報)
 */
typedef	struct {
	lcdbm_rsp_base_t		command;			///< ｺﾏﾝﾄﾞ共通情報
	unsigned long			id;					///< ID(入庫から精算完了までを管理するID)

// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応/コメント）
//	unsigned char			kind;				///< 種別(0=車番検索/1=日時検索)
	unsigned char			kind;				///< 種別(0=車番検索/1=日時検索/2=QR検索)
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応/コメント）
	union {
//		unsigned char BYTE;
		struct {
			uchar	shaban[4];
			uchar	reserve[2];
		} shabanSearch;							// kind == 車番検索の場合
		struct {
			unsigned short	Year;
			unsigned char	Month;
			unsigned char	Day;
			unsigned char	Hours;
			unsigned char	Minute;
		} nichijiSearch;						// kind == 日時検索の場合
// GG129000(S) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		struct {
			uchar	state;						// 遠隔精算(精算中変更)時に使用 1:初回,2:2回目以降(QR割引)
			uchar	reserve[5];
		} PayInfoChange;						// kind == 遠隔精算の場合
// GG129000(E) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	} data;
	uchar	shaban[4];							// ｾﾝﾀｰ問合せ時の車の車番
	uchar	shubetsu;							// 種別(0=事前精算／1＝未精算出庫精算)
// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
	SHABAN_INFO				ShabanInfo;			// 車番情報
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
	DATE_YMDHMS				PayDateTime;		// 精算開始日時
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

	unsigned short			crd_info_rev_no;	///< ｶｰﾄﾞ情報Rev.No.
// GG124100(S) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 	stRecvCardInfo_Rev10	crd_info;			///< ｶｰﾄﾞ情報(入庫情報)
// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	stRecvCardInfo_Rev11	crd_info;			///< ｶｰﾄﾞ情報(入庫情報)
	stRecvCardInfo_RevXX	crd_info;			///< ｶｰﾄﾞ情報(入庫情報)
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
} lcdbm_rsp_in_car_info_t;

/**
 *	QRデータ
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				// コマンド共通情報
	ushort				id;						// QRコードID
	ushort				rev;					// QRコードフォーマットRev.
	ushort				data_size;				// QRコードデータ(生データ)サイズ
	ushort				enc_type;				// QRコードエンコードタイプ
	uchar				data[BAR_DATASIZE];		// QRコードデータ(生データ)
	ushort				info_size;				// QRデータ情報(パースデータ)サイズ
	lcdbm_rsp_QR_com_u	QR_data;				// 買物金額～予備
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ushort				qr_type;				// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
} lcdbm_rsp_QR_data_t;

/**
 *	精算応答データ
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< コマンド共通情報
	unsigned long		id;						///< ID(入庫から精算完了までを管理するID)
	unsigned char		result;					///< 結果(0:OK,1:NG)
	unsigned char		reserve;				///< 予備
} lcdbm_rsp_pay_data_res_t;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
/**
 *	領収証データ応答
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< コマンド共通情報
	unsigned long		id;						///< ID(入庫から精算完了までを管理するID)
	unsigned char		result;					///< 結果(0:OK,1:NG)
	unsigned char		reserve;				///< 予備
} lcdbm_rsp_receipt_data_res_t;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

/**
 *	QR確定・取消データ応答
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< コマンド共通情報
	unsigned long		id;						///< ID(入庫から精算完了までを管理するID)
	unsigned char		result;					///< 結果(0:OK,1:NG)
	unsigned char		reserve;				///< 予備
} lcdbm_rsp_QR_conf_can_res_t;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
/**
 *	レーンモニタデータ応答
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< コマンド共通情報
	unsigned long		id;						///< ID(入庫から精算完了までを管理するID)
	unsigned short		result;					///< 結果(0:OK,1:NG)
	unsigned char		reserve;				///< 予備
} lcdbm_rsp_LANE_res_t;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

//--------------------------------------------------
//		メンテナンスデータ
//--------------------------------------------------
/**
 *	QRリーダ制御応答
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< コマンド共通情報
	unsigned char		ctrl_cd;			///< 制御コード(要求した制御コード)
	unsigned char		result;				///< 結果(00H:正常,01H:異常)
	unsigned char		part_no[32];		///< 品番
	unsigned char		serial_no[32];		///< シリアルナンバー
	unsigned char		version[32];		///< バージョン
} lcdbm_rsp_QR_ctrl_res_t;

/**
 *	QR読取結果
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< コマンド共通情報
	unsigned char		result;				///< 結果(00H:正常,01H:異常)
	ushort				id;						// QRコードID
	ushort				rev;					// QRコードフォーマットRev.
	ushort				data_size;				// QRコードデータ(生データ)サイズ
	ushort				enc_type;				// QRコードエンコードタイプ
	uchar				data[BAR_DATASIZE];		// QRコードデータ(生データ)
	ushort				info_size;				// QRデータ情報(パースデータ)サイズ
	lcdbm_rsp_QR_com_u	QR_data;				// 買物金額～予備
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ushort				qr_type;				// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
} lcdbm_rsp_QR_rd_rslt_t;

/**
 *	リアルタイム通信疎通結果
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< コマンド共通情報
	unsigned char		result;				///< 結果(00H:正常,01H:通信異常(TCP接続不可),02H:タイムアウト)
} lcdbm_rsp_rt_con_rslt_t;


/**
 *	DC-NET通信疎通結果
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			// コマンド共通情報
	unsigned char		result;				// 結果(00H:正常,01H:通信異常(TCP接続不可),02H:タイムアウト)
} lcdbm_rsp_dc_con_rslt_t;
//--------------------------------------------------
//		受信電文の復号化用
//--------------------------------------------------
/**
 *	暗号化された受信電文の復号化用バッファ
 */
#define DECRYPT_COM	4096	///< １電文の最大長は余裕を持って4096byteとする

//--------------------------------------------------
//		コマンド共用体
//--------------------------------------------------
/**
 *	送信コマンド（ メイン基板 ==> LCDモジュール ）共用体
 */
typedef	union {

	lcdbm_cmd_base_t			command;		///< コマンド共通情報

	/** ステータス要求 */
	lcdbm_cmd_config_t			config;			///< 基本設定要求

	/** 機器設定要求 */
	lcdbm_cmd_clock_t			clock;			///< 時計設定
	lcdbm_cmd_brightness_t		brightness;		///< LCD輝度

	/** テキスト */
	/*
	 *	テキストは文字列データの作成が１箇所で行えない（離れたタイミングで行う）可能性を考えて
	 *	他のコマンドとは別ワークに作成する。
	 */

	/** アナウンス要求 */
	lcdbm_cmd_audio_volume_t	au_volume;		///< 音量設定
	lcdbm_cmd_audio_start_t		au_start;		///< 開始要求
	lcdbm_cmd_audio_end_t		au_end;			///< 終了要求

	/** ブザー要求 */
	lcdbm_cmd_beep_volume_t		bp_volume;		///< 音量設定
	lcdbm_cmd_beep_start_t		bp_start;		///< ブザー鳴動要求

	/** 精算機状態通知操作 */
	lcdbm_cmd_notice_ope_t		operate;		///< 操作通知
	lcdbm_cmd_notice_opn_t		opn_cls;		///< 営休業通知
	lcdbm_cmd_notice_pay_t		pay_state;		///< 精算状態通知
	lcdbm_cmd_notice_dsp_t		display;		///< 表示要求
	lcdbm_cmd_notice_alm_t		alarm;			///< 警告通知
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	lcdbm_cmd_notice_del_t		display_del;	///< 削除要求
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

	/** 精算・割引情報通知 */
	lcdbm_cmd_pay_rem_chg_t		pay_rem_chg;	///< 精算残高変化通知
	lcdbm_cmd_pay_data_t		pay_data;		///< 精算データ
	lcdbm_cmd_QR_conf_can_t		QR_conf_can;	///< QR確定・取消データ
	lcdbm_cmd_QR_data_res_t		QR_data_res;	///<QRデータ応答
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
	lcdbm_cmd_remote_time_start_t	remote_time_start;	// 入庫時刻指定遠隔精算開始
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	lcdbm_cmd_lane_t			lane_data;		///< レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	lcdbm_cmd_receipt_data_t	receipt_data;	///< 領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	/** メンテナンスデータ */
	lcdbm_cmd_QR_ctrl_req_t		QR_ctrl_req;	///< QRリーダ制御要求
	lcdbm_cmd_rt_con_req_t		rt_con_req;		///< リアルタイム通信疎通要求
	lcdbm_cmd_dc_con_req_t		dc_con_req;		// DC-NET通信疎通要求

} lcdbm_cmd_work_t;

/**
 *	受信データ（ LCDモジュール ==> メイン基板 ）共用体
 */
typedef	union {

	lcdbm_rsp_base_t			command;		///< コマンド共通情報

	/** ステータス通知 */
	lcdbm_rsp_config_t			config;			///< 基本設定応答

	/** エラー通知 */
	lcdbm_rsp_error_t			error;			///< エラー通知

	/** テンキー制御 */
	lcdbm_rsp_tenkey_info_t		tenkey;			///< テンキー押下情報

	/** 精算機状態通知操作 */
	lcdbm_rsp_notice_ope_t		operate;		///< 操作通知

	/** 精算・割引情報通知 */
	lcdbm_rsp_in_car_info_t		pay_rem_chg;	///< 入庫情報
	lcdbm_rsp_QR_data_t			QR_data;		///< QRデータ
	lcdbm_rsp_pay_data_res_t	pay_data_res;	///< 精算応答データ
	lcdbm_rsp_QR_conf_can_res_t	QR_conf_can_res;///< QR確定・取消データ応答
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	lcdbm_rsp_LANE_res_t		lane_data_res;	///< レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	lcdbm_rsp_receipt_data_res_t receipt_data_res;	///< 領収証データ応答
// GG129004(E) R.Endo 2024/11/19 電子領収証対応


} lcdbm_rsp_work_t;
#pragma unpack

#endif	// __PKT_DEF_H__
// MH810100(E) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)