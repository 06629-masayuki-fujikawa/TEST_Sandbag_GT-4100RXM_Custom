//[]----------------------------------------------------------------------[]
///	@brief			決済リーダR/W
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 19/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
#ifndef _EC_DEF_H_
#define _EC_DEF_H_

#include	"system.h"
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
#include	"common.h"
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
#include	"mem_def.h"
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

typedef	struct {
	uchar	type;		// R/W type
	uchar	mdl;		// module code
	void	(*init)(void);
	void	(*event)(void);
	short	(*recv)(uchar*, short);
	void	(*log_regist)(uchar*, ushort, uchar);
} t_JVMA_SETUP;

// MH810103 MHUT40XX(S) Edy・WAON対応
//// GG119202(S) 精算機の未了確定応答待ちタイマを変更する
////#define	OPE_EC_MIRYO_TIME		65									// 未了監視タイムアウト:65s
//#define	OPE_EC_MIRYO_TIME		80									// 未了監視タイムアウト:80s
//// GG119202(E) 精算機の未了確定応答待ちタイマを変更する
// MH810103 MHUT40XX(E) Edy・WAON対応

#define	OPE_EC_RECEPT_SEND_TIME	10									// 次の受付許可送信:200ms(200ms/20ms=10)
// MH810103 GG119202(S) E3210登録処理修正
//// MH321800(S) E3210登録タイミング変更
//#define	OPE_EC_WAIT_BOOT_TIME	180									// 初期化シーケンス完了待ち時間
//// MH321800(E) E3210登録タイミング変更
#define	OPE_EC_WAIT_BOOT_TIME	(7*60)								// 初期化シーケンス完了待ち時間
// MH810103 GG119202(E) E3210登録処理修正
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
#define	OPE_EC_WAIT_BOOT_TIME_VERUP	(40*60)							// 初期化シーケンス完了待ち時間（決済リーダバージョンアップ中）
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応

// 決済リーダ固定値(※SX-10では設定参照)
#define	EC_STATUS_TIME			0									// 状態監視タイマー(固定：分)
#define	EC_INIT_RETRY_CNT		0									// 再初期化リトライ回数(固定)
#define	EC_ENABLE_WAIT_TIME		10									// 受付可受信待ちタイマー(固定：秒)
#define	EC_ENABLE_RETRY_CNT		0									// タイムアウト時リトライ回数
#define	EC_DISABLE_WAIT_TIME	5									// 受付不可受信待ちタイマー(固定：秒)
#define	EC_DISABLE_RETRY_CNT	1									// タイムアウト時リトライ回数
#define	EC_MIN_VOLUME			1									// 最小音量
#define	EC_MAX_VOLUME			15									// 最大音量
#define	EC_CMDWAIT_TIME			1000								// メンテナンス結果データ受信待ちﾀｲﾏｰ：20s(20000ms/20ms=1000)
// GG116202(S) JVMAリセット回数制限対応
#define	EC_JVMA_RESET_CNT		3									// JVMAリセットリトライ回数
// GG116202(E) JVMAリセット回数制限対応
// MH810103 GG119202(S) 起動シーケンス不具合修正
#define	EC_BRAND_NEGO_WAIT_TIME	250									// ブランドネゴシエーション中の応答待ちタイマー:5s(5000ms/20ms=250)
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG118807_GG118907(S) 交通系ICの限度額を10万円とする
#define	EC_PAY_LIMIT_KOUTSUU	100000								// 交通系ICの精算限度額（10万円）
// MH810103 GG118807_GG118907(E) 交通系ICの限度額を10万円とする

// フェーズ
enum {
	EC_PHASE_INIT = 0,				// 初期化中
// MH810103 GG119202(S) 起動シーケンス不具合修正
//	EC_PHASE_PAY,					// 精算中(ノーマル)
// MH810103 GG119202(E) 起動シーケンス不具合修正
	EC_PHASE_BRAND,					// ブランド合わせ中
// MH810103 GG119202(S) 起動シーケンス不具合修正
	EC_PHASE_VOL_CHG,				// 音量変更中
	EC_PHASE_PAY,					// 精算中(ノーマル)
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) JVMAリセット処理不具合
	EC_PHASE_WAIT_RESET,			// JVMAリセット待ち
// MH810103 GG119202(E) JVMAリセット処理不具合
};

// 決済リーダ制御
struct	ECCTL_rec {
	uchar				phase;					// 決済リーダフェーズ
	uchar				step;					// フェーズ内ステップ
	uchar				pon_err_alm_regist;		// 1=受付可アラーム、ブランドエラー登録する
	uchar				not_ready_timer;		// 1=リーダー受付許可制御失敗タイマー監視中
	uchar				brand_num;				// リーダ側ブランド数
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//	uchar				dmy[3];
	uchar				prohibit_snd_bsy;		// 制御データ（受付禁止）応答監視状態
												// 0x00:応答監視なし
												// 0x01:受付禁止応答監視中
												//(0x02:再起動可は応答監視対象外)
												// 0x08:金額変更応答監視中
												// 0x80:取引終了応答監視中
	uchar				transact_end_snd;		// 取引終了送信要求状態
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	uchar				dmy;
	uchar				RecvVolumeRes;			// 受信した音量変更結果データ
	uchar				ec_Disp_No;				// 決済結果詳細エラーコードの案内表示パターン
												//  0	:表示なし
												//  1～	:詳細エラー表示
												// 99	:エラー登録しない詳細エラー(表示なし)
	uchar				Ec_Settlement_Sts;		// 決済状態
	uchar				timer_recept_send_busy;	// 1=精算中のec受入可送信タイマー動作中
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	uchar				dmy;
	uchar				brandsel_stop;			// ブランド処理停止
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	ushort				Product_Select_Brand;	// 商品選択データ送信のブランド番号
	ushort				ec_MessagePtnNum;		// LCD表示パターン
												//  0:表示なし
												//  1:クレジット処理中
												//  2:電子マネー処理中
												//  3:カード抜き取り案内
												//  4:決済結果NG受信時のカード抜き取り案内
												// 98:決済結果NG受信時の詳細エラーによるエラー表示
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
	uchar				anm_ec_emoney;			// 案内放送フラグ（0=未放送,1=受付可待ち,0xFF=放送済み）
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
// MH810103 GG118808_GG118908(S) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
	uchar				ec_single_timeout;		// 電子マネーシングルのタイムアウトフラグ（1=タイムアウトあり）
// MH810103 GG118808_GG118908(E) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
// MH810105(S) MH364301 QRコード決済対応
	uchar				Ec_FailureContact_Sts;	// 障害連絡票発行処理
												//  0:障害連絡票発行不可
												//  1:障害連絡票発行待ち
												//  2:障害連絡票発行後
// MH810105(E) MH364301 QRコード決済対応
};
extern	struct ECCTL_rec	ECCTL;

// コマンド
#define	S_PRINT_DATA		0x1C	// 印字依頼データ
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//#define	S_RECEIVE_DEEMED	0x29	// 決済精算中止(復決済)ﾃﾞｰﾀ
#define	S_RECEIVE_DEEMED	0x49	// 直前取引ﾃﾞｰﾀ
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
#define	S_SETT_STS_DATA		0x28	// 決済状態データ

#define	S_BRAND_SET_DATA	0x14	// ブランド設定データ1
#define	S_BRAND_STS_DATA	0x1D	// ブランド状態データ1
#define	S_BRAND_SET_DATA2	0x24	// ブランド設定データ2
#define	S_BRAND_STS_DATA2	0x2D	// ブランド状態データ2

#define	S_VOLUME_CHG_DATA	0x31	// 音量変更データ
#define	S_READER_MNT_DATA	0x32	// リーダメンテナンスデータ
#define	S_BRAND_SEL_DATA	0x33	// ブランド選択データ

#define	S_VOLUME_RES_DATA	0x34	// 音量変更結果データ
#define	S_MODE_CHG_DATA		0x35	// リーダメンテナンス結果データ
#define	S_BRAND_RES_DATA	0x36	// ブランド選択結果データ
// MH810103 GG119202(S) ブランド選択結果応答データ
#define	S_BRAND_RES_RSLT_DATA	0x39	// ブランド選択結果応答データ
// MH810103 GG119202(E) ブランド選択結果応答データ
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
#define	S_BRAND_RES_RSLT_DATA_with_HOLD	0x139	// ブランド選択結果応答データ
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 QRコード決済対応
#define S_SUB_BRAND_RES_DATA	0x4D	// サブブランドデータ
// MH810105(E) MH364301 QRコード決済対応

// 制御データのbit
#define	S_CNTL_REBOOT_OK_BIT			0x02	// 再起動可
#define	S_CNTL_MIRYO_ZANDAKA_END_BIT	0x04	// 未了残高終了通知
#define	S_CNTL_PRICE_CHANGE_BIT			0x08	// 金額変更
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
#define	S_CNTL_TRANSACT_END_BIT			0x80	// 取引終了
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応

#define	ECARDID_SIZE_SUICA			16			// SuicaカードIDサイズ
#define	ECARDID_SIZE_SAPICA			17			// SAPICAカードIDサイズ
#define	ECARDID_SIZE_KOUTSUU		17			// 交通系ICカードIDサイズ
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
#define	ECARDID_SIZE_CREDIT			16			// クレジットカードIDサイズ
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

#define EC_CMD_LEN_CTRL_DATA		3			// 制御データコマンド長
#define EC_CMD_LEN_SLCT_DATA		74			// 選択商品データコマンド長
#define EC_CMD_LEN_INDI_DATA		88			// 個別データコマンド長
#define EC_CMD_LEN_BSET_DATA		233			// ブランド設定データコマンド長
#define EC_CMD_LEN_VSET_DATA		21			// 音量変更データコマンド長
#define EC_CMD_LEN_MENT_DATA		5			// リーダメンテナンスデータコマンド長
// MH810103 GG119202(S) ブランド選択データフォーマット変更
//#define EC_CMD_LEN_BSEL_DATA		43			// ブランド選択データコマンド長
#define EC_CMD_LEN_BSEL_DATA		45			// ブランド選択データコマンド長
// MH810103 GG119202(E) ブランド選択データフォーマット変更
// MH810103 GG119202(S) ブランド選択結果応答データ
#define EC_CMD_LEN_BRRT_DATA		5			// ブランド選択結果応答データコマンド長
// MH810103 GG119202(E) ブランド選択結果応答データ

#define EC_BRAND_NAME_LEN			20			// ブランド名称長
// MH810105(S) MH364301 QRコード決済対応
#define	EC_SUB_BRAND_MAX			124			// サブブランド最大数
// MH810105(E) MH364301 QRコード決済対応

// 取引データの取引種別
enum {
	EPAY_RESULT_NONE = 0x00,		// その他
	EPAY_RESULT_OK,					// OK
	EPAY_RESULT_NG,					// NG
	EPAY_RESULT_CANCEL,				// 決済キャンセル
	EPAY_RESULT_CANCEL_PAY_OFF,		// 精算キャンセル
	EPAY_RESULT_MIRYO,				// 未了確定
// MH810103 GG119202(S) 未了仕様変更対応
//	EPAY_RESULT_MIRYO_AFTER_OK,		// 未了確定後決済OK
// MH810103 GG119202(E) 未了仕様変更対応
	EPAY_RESULT_MIRYO_ZANDAKA_END,	// 未了残高照会完了
// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
	EPAY_RESULT_PAY_MIRYO = 0x11,	// 支払未了（決済OK）
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
};

// 決済状態データの決済状態
enum {
	EC_SETT_STS_NONE = 0x00,		// 下記以外
	EC_SETT_STS_MIRYO = 0x02,		// 未了中
	EC_SETT_STS_CARD_PROC,			// カード処理中
	EC_SETT_STS_INQUIRY_OK,			// 照会OK
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
	EC_SETT_STS_DEEMED_SETTLEMENT,	// みなし決済
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
};

// 取引データ(／アラームレシート)
// ブランド番号：取引データ/ブランド状態データ受信時のブランド番号
#define		BRANDNO_KOUTSUU		1000
// MH810105(S) MH364301 PiTaPa対応
//#define		BRANDNO_SAPICA		1025		// 仮番号
#define		BRANDNO_SAPICA		1030		// 仮番号
// MH810105(E) MH364301 PiTaPa対応
#define		BRANDNO_EDY			1020
#define		BRANDNO_WAON		1021
#define		BRANDNO_NANACO		1022
#define		BRANDNO_ID			1023
#define		BRANDNO_QUIC_PAY	1024
// MH810105(S) MH364301 PiTaPa対応
#define		BRANDNO_PITAPA		1025
// MH810105(E) MH364301 PiTaPa対応
#define		BRANDNO_CREDIT		2000
// MH810105(S) MH364301 QRコード決済対応
#define		BRANDNO_QR			3000
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
#define		BRANDNO_TCARD		7000
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
#define		BRANDNO_HOUJIN		8000
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
#define		BRANDNO_HOUSE_S		7001
#define		BRANDNO_HOUSE_E		7999
#define		BRANDNO_for_SETTLEMENT		6999
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
#define		BRANDNO_UNKNOWN		0xffff
#define		BRANDNO_ZERO		0				// ブランド番号0

// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
//#define		EC_BRAND_MAX		10				// ブランド数最大値
//#define		EC_BRAND_TOTAL_MAX	(EC_BRAND_MAX*2)	// 総ブランド数最大値
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
#define		EC_VOL_MAX			6				// 音量変更パターン数最大値

// MH810103 MHUT40XX(S) Edy・WAON対応
//#define		WAON_POINT_JMB_MILE		50001		// カード分類：JMBマイル
// MH810103 MHUT40XX(E) Edy・WAON対応

// ブランド選択結果データ
#define		EC_BRAND_SELECTED		1			// ブランド選択済み
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//#define		EC_BRAND_UNSELECTED		0			// ブランド未選択
#define		EC_BRAND_CANCELED		0			// ブランド選択キャンセル
#define		EC_BRAND_UNSELECTED		2			// ブランド未選択

// ブランド選択結果データ 理由コード
#define		EC_BRAND_REASON_0		0			// キャンセルボタンが押された。
#define		EC_BRAND_REASON_1		1			// タイムアウトした。
#define		EC_BRAND_REASON_99		99			// ブランド選択データの処理ができない。
#define		EC_BRAND_REASON_201		201			// カードの問い合わせができない。
#define		EC_BRAND_REASON_202		202			// カードの問い合わせ結果を受信できない。
#define		EC_BRAND_REASON_203		203			// 誤りデータが含まれるカードを読み取りした。
#define		EC_BRAND_REASON_204		204			// データが破損しているカードを読み取りした。
#define		EC_BRAND_REASON_205		205			// 意図しないカードを読み取りした。
#define		EC_BRAND_REASON_206		206			// 有効期限切れのカードを読み取りした。
#define		EC_BRAND_REASON_207		207			// ネガカードを読み取りした。
#define		EC_BRAND_REASON_208		208			// 有効なカードであるが使用できない。
#define		EC_BRAND_REASON_209		209			// 有効なカードであるが支払い限度額を超えている。
#define		EC_BRAND_REASON_210		210			// 有効なカードであるが残高が不足している。
#define		EC_BRAND_REASON_211		211			// 受付済みのカードを読み取りした。
#define		EC_BRAND_REASON_212		212			// 読み取り限度数を超えてカードを読み取りした。
#define		EC_BRAND_REASON_299		299			// その他のエラー
#define		EC_BRAND_REASON_INVALID 0xffff		// 無効値(内部コードです。決済リーダに送信不可です)
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

// MH810103 GG119202(S) 開局・有効条件変更
//// 精算可否判定種別 - isEcReady()の引数
//enum {
//	EC_CHECK_STATUS = 1,
//	EC_CHECK_EMONEY,
//	EC_CHECK_CREDIT,
//};
// MH810103 GG119202(E) 開局・有効条件変更

typedef union{
	ushort		status_data;					// 2Byte
	struct {
		// 状態データ1
		uchar	RebootReq:1;					// Bit 7 = 再起動要求（0：再起動要求なし 1：再起動要求）
		uchar	DetectionCard:1;				// Bit 6 = カード差し込み状態（0：カードなし 1：カードあり）
		uchar	ReqTimerUpdate:1;				// Bit 5 = 商品先選択タイマー更新依頼（0：更新依頼 1：なし）
		uchar	MiryoStatus:1;					// Bit 4 = 処理未了状態（0：未了中で無い 1：処理未了中）
		uchar	MiryoZandakaStop:1;				// Bit 3 = 未了残高中止（0：なし 1：未了残高照会中止）
		uchar	Running:1;						// Bit 2 = 実行中（0：実行中で無い 1：実行中）
		uchar	TradeCansel:1;					// Bit 1 = 取引ｷｬﾝｾﾙ（0：取引ｷｬﾝｾﾙで無い 1：取引ｷｬﾝｾﾙ受付）
		uchar	ReceptStatus:1;					// Bit 0 = 受付状態（0：受付可 1：受付不可）
		// 状態データ2
// MH810103 GG119202(S) 未了仕様変更対応
//		uchar	YOBI:8;							// Bit 0-7 = 予備
// MH810103 MHUT40XX(S) Edy・WAON対応
//		uchar	YOBI:6;							// Bit 2-7 = 予備
// MH810104 GG119201(S) リーダ直取中は制御データ（受付許可）を送信しない
//		uchar	YOBI:5;							// Bit 3-7 = 予備
		uchar	YOBI:4;							// Bit 4-7 = 予備
		uchar	LastSetteRunning:1;				// Bit 3 = 直前取引要求中（0：要求中で無い 1：要求中）
// MH810104 GG119201(E) リーダ直取中は制御データ（受付許可）を送信しない
		uchar	CardProcTimerUpdate:1;			// Bit 2 = カード読取待ちタイマ更新依頼（0：なし １：更新依頼）
// MH810103 MHUT40XX(E) Edy・WAON対応
		uchar	MiryoConfirm:1;					// Bit 1 = 未了確定（0：なし 1：未了確定）
		uchar	ZandakaInquiry:1;				// Bit 0 = 残高照会中（0：なし 1：残高照会中）
// MH810103 GG119202(E) 未了仕様変更対応
	} StatusInfo;
} t_EC_STATUS_DATA_INFO;
extern t_EC_STATUS_DATA_INFO	EC_STATUS_DATA;				// 状態データ
extern t_EC_STATUS_DATA_INFO	PRE_EC_STATUS_DATA;			// 前回受信した状態データ
extern t_EC_STATUS_DATA_INFO	EC_STATUS_DATA_WAIT;		// 状態データ(受信待ち用)

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// Ec_infoがpackされているため
#pragma pack
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
typedef struct {
// 交通系IC
	uchar				SPRW_ID[13];			// SPRWID
	uchar				dmy1;
	uchar				Kamei[30];				// 加盟店名
	uchar				TradeKind;				// 取引種別
	uchar				dmy2;
} EC_TRADE_RES_KOUTSUU;
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正

typedef	struct {
// Edy
// MH810103 MHUT40XX(S) Edy・WAON対応
//	long				DealNo;					// 取引通番(hex:4byte)
//	long				CardDealNo;				// カード取引通番(hex:4byte)
//	long				TerminalNo[2];			// 端末番号(hex:8byte)
	uchar				DealNo[10];				// Edy取引通番
	uchar				CardDealNo[5];			// カード取引通番
	uchar				dmy;
	uchar				TerminalNo[8];			// 上位端末ID
	uchar				Kamei[30];				// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
} EC_TRADE_RES_EDY;

typedef	struct {
// nanaco
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	long				TerminalNo[3];			// 上位端末ID(bcd:10byte)
//	long				DealNo;					// 上位取引通番(hex:4byte)
	uchar				Kamei[30];				// 加盟店名
	uchar				DealNo[6];				// 端末取引通番
	uchar				TerminalNo[20];			// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} EC_TRADE_RES_NANACO;

typedef	struct {
// WAON
// MH810103 MHUT40XX(S) Edy・WAON対応
//	long				GetPoint;				// 今回ポイント(hex:4byte)
//	long				TotalPoint;				// 累計ポイント(hex:4byte)
//	long				CardType;				// カード分類(bcd:4byte)
//	long				PeriodPoint;			// ２年前までに獲得したポイント(hex:4byte)
//	long				Period;					// ２年前までに獲得したポイントの有効期限(bcd:4byte)
//	uchar				SPRW_ID[14];			// SPRWID(実際は13桁)
//	uchar				PointStatus;			// 利用可否(hex:1byte)
//	uchar				DealCode;				// 取引種別コード
	ulong				GetPoint;				// 今回ポイント(hex:4byte)
	ulong				TotalPoint[2];			// 累計ポイント(hex:8byte)
	uchar				SPRW_ID[13];			// SPRWID
	uchar				PointStatus;			// 累計ポイントメッセージ
												// 0=「累計WAONポイント数は、お近くのWAONステーションで確認ください。」
												// 1=「このWAONカードはポイント対象外です。」
												// 2=「JMB WAONはマイルがつきます。」
	uchar				Kamei[30];				// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
} EC_TRADE_RES_WAON;

typedef	struct {
// SAPICA
	uchar				Terminal_ID[8];			// 物販端末ID(実際は 7桁)
	long				Details_ID;				// 一件明細ID(hex:4byte)
} EC_TRADE_RES_SAPICA;

typedef	struct {
// iD
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	uchar				Terminal_ID[8];			// 物販端末ID(実際は 7桁)
//	long				Details_ID;				// 一件明細ID(hex:4byte)
	uchar				Kamei[30];				// 加盟店名
	uchar				Approval_No[7];			// 承認番号
	uchar				dmy1;
	uchar				TerminalNo[13];			// 上位端末ID
	uchar				dmy2;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} EC_TRADE_RES_ID;

typedef	struct {
// QuicPay
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	uchar				Terminal_ID[8];			// 物販端末ID(実際は 7桁)
//	long				Details_ID;				// 一件明細ID(hex:4byte)
	uchar				Kamei[30];				// 加盟店名
	uchar				Approval_No[7];			// 承認番号
	uchar				dmy1;
	uchar				TerminalNo[13];			// 上位端末ID
	uchar				dmy2;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} EC_TRADE_RES_QUIC_PAY;

// MH810105(S) MH364301 PiTaPa対応
typedef	struct {
// PiTaPa
	uchar				Kamei[30];				// 加盟店名
	uchar				Approval_No[8];			// 承認番号
	uchar				TerminalNo[13];			// 上位端末ID
	uchar				dummy1;
	uchar				Slip_No[5];				// 伝票番号
	uchar				dummy2;
	uchar				Reason_Code[6];			// 理由コード
} EC_TRADE_RES_PITAPA;
// MH810105(E) MH364301 PiTaPa対応

typedef	struct {
// Credit
	long				Slip_No;				// 端末処理通番（伝票番号）
	unsigned char		Approval_No[7];			// 承認番号
	unsigned char		KID_Code[6];			// KIDコード
	unsigned char		Id_No[13];				// 端末識別番号
	unsigned char		Credit_Company[24];		// クレジットカード会社名
	unsigned char		Identity_Ptrn;			// 本人確認パターン
} EC_TRACE_RES_CREDIT;

// MH810105(S) MH364301 QRコード決済対応
typedef struct {
// QR
	uchar	PayKind;							// 支払種別
	uchar	dmy1;
	uchar	PayTerminalNo[20];					// 支払端末ID
	uchar	MchTradeNo[32];						// Mch取引番号
} EC_TRADE_RES_QR;
// MH810105(E) MH364301 QRコード決済対応

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
#pragma unpack
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

typedef	struct {
	unsigned char		Result;										// 決済データ
	unsigned char		Column_No;									// 決済コラム番号
	ushort				brand_no;									// ブランド番号
	long				settlement_data;							// 実際の決済額
	long				settlement_data_before;						// 決算前Suica残高
	long				settlement_data_after;						// 決済後Suica残高
	unsigned char		Card_ID[20];								// Card ID
// MH810103 GG119202(S) 電子マネー対応
//	unsigned char		inquiry_num[16];							// 問合せ番号
	unsigned char		inquiry_num[15];							// 問い合わせ番号
// MH810105(S) MH364301 QRコード決済対応
//	unsigned char		dummy;										// 問い合わせ番号と合わせて16byte
	t_E_FLAG			E_Flag;										// ﾌﾗｸﾞ管理(uchar)
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(E) 電子マネー対応
	struct	clk_rec		settlement_time;							// 精算時刻
// MH810105(S) MH364301 QRコード決済対応
//// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//	unsigned char		Termserial_No[30];							// 端末識別番号
//// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
	uchar				QR_Kamei[30];								// 加盟店名（QRコード決済）
// MH810105(E) MH364301 QRコード決済対応

	// 以降、ブランド毎に受信するデータ内容
	union {
	// ブランド別取引データ
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
		EC_TRADE_RES_KOUTSUU	Koutsuu;
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		EC_TRADE_RES_EDY		Edy;
		EC_TRADE_RES_NANACO		Nanaco;
		EC_TRADE_RES_WAON		Waon;
		EC_TRADE_RES_SAPICA		Sapica;
		EC_TRADE_RES_ID			Id;
		EC_TRADE_RES_QUIC_PAY	QuicPay;
		EC_TRACE_RES_CREDIT		Credit;
// MH810105(S) MH364301 QRコード決済対応
		EC_TRADE_RES_QR			Qr;
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
//	// バックアップ/リストアの対象とするので、将来のブランド追加用にサイズに余裕を持たせておく
		EC_TRADE_RES_PITAPA		Pitapa;
		// EC_TRADE_RES_PITAPAが64バイトのため、
		// 64バイトを超える場合はdummyもサイズ変更すること
// MH810105(E) MH364301 PiTaPa対応
		uchar					dummy[64];
	} Brand;
} EC_SETTLEMENT_RES;

extern	EC_SETTLEMENT_RES	Ec_Settlement_Res;	// ECデータ管理構造体
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//extern	unsigned char		Ec_Settlement_Sts;	// 決済状態データ
//extern 	short				ec_MessagePtnNum;	// LCD表示パターン
//												//  0:表示なし
//												//  1:クレジット処理中
//												//  2:カード処理中
//												//  3:カード抜き取り案内
//												//  4:決済結果NG受信時のカード抜き取り案内
//												// 98:決済結果NG受信時の詳細エラーによるエラー表示
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

#define	EC_CARDDATA_LEN			128				// カードデータ長さ

// ブランド毎制御
typedef	struct {
	ushort	no;			// ブランド番号
	uchar	reserved;
	uchar	status;		// ブランド状態
						//   b0:1=開局,0=閉局
						//   b1:1=無効,0=有効
						//   b2:1=一件明細満杯,0=空きあり
						//   b3:1=サービス停止中,0=なし
						//   b4-7:未定義
						// ブランド設定
						//   上位bit(0xh=開局/1xh=閉局)
						//   下位bit(x0h=有効/x1h=無効)
	uchar	name[20];	// ブランド名称
} EC_BRAND_CTRL;

typedef	struct {
	uchar	num;		// ブランド数
	uchar	dmy;
	EC_BRAND_CTRL	ctrl[EC_BRAND_MAX];	// ブランド毎テーブル
} EC_BRAND_TBL;

// 音量パターン毎制御
typedef	struct {
	ushort	time;		// 切替時刻
	uchar	vol;		// 音量
	uchar	dmy;
} EC_VOLUME_CTRL;

typedef	struct {
	uchar	num;		// 指定された切替パターン
	uchar	dmy;
	EC_VOLUME_CTRL	ctrl[EC_VOL_MAX];	// ブランド毎テーブル
} EC_VOLUME_TBL;

typedef	struct {
	uchar	cmd;		// メンテナンスコマンド
	uchar	mode;		// モード移行指令データ
	uchar	vol;		// テスト音量データ
	uchar	dmy;
} EC_MNT_TBL;

typedef	struct {
// MH810103 GG119202(S) ブランド選択データフォーマット変更
	uchar	btn_enable;	// ボタン有無指定
	uchar	btn_type;	// ボタンタイプ
// MH810103 GG119202(E) ブランド選択データフォーマット変更
	uchar	num;		// ブランド数
	uchar	dmy;
	ushort	no[EC_BRAND_TOTAL_MAX];		// ブランド番号
} EC_BRAND_SEL_TBL;

typedef	struct {
	uchar	res;		// ブランド選択結果
	uchar	siz;		// カードデータサイズ
	ushort	no;			// 選択ブランドNo
	uchar	dat[EC_CARDDATA_LEN];	// カードデータ(ASCII)
} EC_BRAND_RES_TBL;

// MH810103 GG119202(S) ブランド選択結果応答データ
typedef	struct {
	uchar	res_rslt;	// ブランド選択適用結果
	ushort	no;			// 結果OK：ブランド番号／結果NG：理由
} EC_BRAND_RES_RSLT_TBL;
// MH810103 GG119202(E) ブランド選択結果応答データ

// MH810105(S) MH364301 QRコード決済対応
typedef union {
	uchar	BYTE;
	struct {
		uchar	YOBI:5;
		uchar	MORE_LESS:2;			// 1:more、2:less
		uchar	RECV:1;					// 0:受信、1:未受信
	} BIT;
} t_TBL_STS;
typedef	struct {
	t_TBL_STS	tbl_sts;				// テーブル状態
	uchar	brand_num;					// ブランド数
	ushort	brand_no[EC_BRAND_MAX];		// ブランド番号
	uchar	sub_brand_num[EC_BRAND_MAX];// サブブランド数
	uchar	sub_brand_no[EC_SUB_BRAND_MAX];	// サブブランド種別
} EC_SUB_BRAND_TBL;
// MH810105(E) MH364301 QRコード決済対応

// 決済リーダ アラーム取引ログ用構造体(フラップ/ロックシステム用)
typedef struct {
	uchar				syu;			// 種別
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//	uchar				yobi[3];
	uchar				yobi[2];
	uchar				reason;			// 未了発生理由
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
	ulong				WPlace;			// 駐車位置ﾃﾞｰﾀ
// MH810103 MH810103(s) 電子マネー対応 未了取引記録対応
	uchar				CarSearchFlg;								// 車番/日時検索種別
	uchar				CarSearchData[6];							// 車番/日時検索データ
	uchar				CarNumber[4];								// 車番(精算)
	uchar				CarDataID[32];								// 車番データID
// MH810103 MH810103(e) 電子マネー対応 未了取引記録対応
	date_time_rec2		TInTime;		// 入庫日時
	date_time_rec2		alarm_time;		// アラーム取引発生時刻
	EC_SETTLEMENT_RES	alarm_data;		// アラーム取引情報
} t_ALARM_SETTLEMENT_DATA;

// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//#define		EC_ALARM_LOG_CNT	5			// アラーム取引ログ数
#define		EC_ALARM_LOG_CNT	20		// アラーム取引ログ数
typedef enum {
	ALARM_REASON_HIKISARI_OK = 1,		// 未了確定後 引き去り済
	ALARM_REASON_HIKISARI_NG,			// 未了確定後 未引き去り
	ALARM_REASON_TIMEOUT,				// 未了確定後 タイムアウト
// MH810105(S) MH364301 QRコード決済対応
	ALARM_REASON_CHECKFAIL,				// 支払確認失敗
// MH810105(E) MH364301 QRコード決済対応
} ALARM_REASON;
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする

typedef struct {
	ushort					array_cnt;		// =EC_ALARM_LOG_CNT
	ushort					data_size;		// =sizeof(t_ALARM_SETTLEMENT_DATA)
	ushort					log_cnt;		// ログ履歴件数
	ushort					log_wpt;		// ログ書込みポインタ
	t_ALARM_SETTLEMENT_DATA	log[EC_ALARM_LOG_CNT];
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
	Receipt_data			paylog[EC_ALARM_LOG_CNT];	// 再精算情報
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
} Ec_Alarm_Log_rec;
extern	Ec_Alarm_Log_rec	EC_ALARM_LOG_DAT;		/* アラーム取引ログ				*/

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
typedef struct {
	EC_SETTLEMENT_RES		Ec_Res;				// 復電用決済結果データ
} EMoneyAlarmFukuden;
extern	EMoneyAlarmFukuden	EcAlarm;			// 決済リーダ復電用アラーム取引データ
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

extern const unsigned char	ec_tbl[][2];
extern	EC_BRAND_TBL		RecvBrandTbl[2];	// リーダ受信したブランド制御テーブル構造体
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
extern	EC_BRAND_TBL		RecvBrandTblTmp[2];	// ブランドテーブルのtmp
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
extern	EC_BRAND_TBL		BrandTbl[2];		// ブランド制御テーブル構造体
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//extern unsigned char		RecvVolumeRes;		// 受信した音量変更結果データ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
extern	EC_VOLUME_TBL		VolumeTbl;			// 音量変更制御テーブル構造体
extern	EC_MNT_TBL			RecvMntTbl;			// リーダ受信したメンテナンステーブル構造体
extern	EC_MNT_TBL			MntTbl;				// リーダメンテナンステーブル構造体
extern	EC_BRAND_RES_TBL	RecvBrandResTbl;	// リーダ受信したブランド選択結果テーブル構造体
extern	EC_BRAND_SEL_TBL	BrandSelTbl;		// ブランド選択テーブル構造体
// MH810103 GG119202(S) ブランド選択結果応答データ
extern	EC_BRAND_RES_RSLT_TBL	BrandResRsltTbl;	// ブランド選択結果応答テーブル構造体
// MH810103 GG119202(E) ブランド選択結果応答データ
extern	unsigned char		Ec_Settlement_Rec_Code[3];	// 決済結果詳細エラーコード
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//extern	unsigned char		ec_Disp_No;			// 決済結果詳細エラーコードの案内表示パターン
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810105(S) MH364301 QRコード決済対応
extern	EC_SUB_BRAND_TBL	RecvSubBrandTbl;	// サブブランドテーブル構造体
// MH810105(E) MH364301 QRコード決済対応

extern	char			EcEdyTerminalNo[20];	// Edy上位端末ID

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//extern	uchar			timer_recept_send_busy;	// 1=精算中のec受入可送信タイマー動作中
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

extern	t_JVMA_SETUP	jvma_setup;
extern	void			*const jvma_init_tbl[];
// GG116202(S) JVMAリセット回数制限対応
extern	uchar			Ec_Jvma_Reset_Count;
// GG116202(E) JVMAリセット回数制限対応
// MH810103 GG119202(S) 異常データフォーマット変更
extern	uchar			err_data2;
// MH810103 GG119202(E) 異常データフォーマット変更

/*** function prototype ***/

// ope_ec_ctrl.c
extern	uchar 	Ec_Snd_Que_Regist(uchar kind, uchar *snddata);
extern	short 	Ec_Snd_Que_Read(uchar *kind, uchar *snddata);
extern	void	Ec_Data_Snd(uchar kind, void *snddata);
extern  void	Ec_Pri_Data_Snd(ushort req, uchar type);

extern	short	Ope_Ec_Event(ushort msg, uchar ope_faze);

extern	uchar	convert_brandno(const ushort brandno);
// MH810103 GG119202(S) 不要関数削除
//extern	void	ec_usable_req(ushort msg);
// MH810103 GG119202(E) 不要関数削除
extern	long	settlement_amount_ex(uchar *buf, uchar count);

extern	char	Ope_EcArmClearCheck(ushort msg);
extern	void	Ope_TimeOut_Cyclic_Disp(uchar ope_faze, ushort e_pram_set);
extern	void	Ope_TimeOut_Recept_Send(uchar ope_faze, ushort e_pram_set);
extern	void	ec_auto_cancel_timeout(void);

extern	void	EcSettlementPhaseError(uchar *dat, uchar kind);
// MH810105(S) MH364301 QRコード決済対応
//extern	uchar	EcUseKindCheck(uchar kind);
extern	uchar	EcUseKindCheck(uchar kind);				// 電子マネー決済、QRコード決済チェック
extern	uchar	EcUseEMoneyKindCheck(uchar kind);		// 電子マネー決済チェック
// MH810105(E) MH364301 QRコード決済対応

extern	void	ec_miryo_timeout(void);
extern	uchar	get_enable_multisettle(void);			// 今有効なブランド数
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
extern	uchar	check_enable_multisettle(void);			// 機能が有効なブランド数
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

// MH810103 GG119202(S) 開局・有効条件変更
//extern	uchar	isEcReady( const uchar kind );
// MH810103 GG119202(E) 開局・有効条件変更
// MH810103 GG119202(S) 不要処理削除
//extern	uchar	isAnyEcBrandReady(const ushort *brand_ec_ary, const ushort ary_size);
// MH810103 GG119202(E) 不要処理削除
extern	uchar	isEcBrandNoReady(const ushort brand_no);
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//extern	uchar	isEcBrandNoEMoney(const ushort brand_no);
extern	uchar	isEcBrandNoEMoney(const ushort brand_no, uchar zandaka_chk);
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
extern	ushort	ConvertEcBrandToNo(const ushort brand_ec);
extern	uchar	isEcBrandNoEnabledForSetting(const ushort brand_no);
extern	uchar	isEcBrandNoEnabledForRecvTbl(const ushort brand_no);
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//extern	uchar	isEcEnabled( const uchar kind );
//extern	uchar	isAnyEcBrandEnabled(const ushort *brand_ec_ary, const ushort ary_size);
extern	uchar	isEcEmoneyEnabled(uchar open_chk, uchar zandaka_chk);
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH810103 GG119202(S) 開局・有効条件変更
extern	uchar	isEcBrandNoEnabled(ushort brand_no);
// MH810103 GG119202(E) 開局・有効条件変更
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
extern	uchar	isEcBrandNoEnabledNoCtrl(ushort brand_no);
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
// MH810103 GG119202(S) ブランド名はブランド情報から取得する
extern	int		getBrandName(EC_BRAND_CTRL *brand, uchar *buff);
// MH810103 GG119202(E) ブランド名はブランド情報から取得する
extern	char	EcFirst_Pay_Dsp( void );
extern	uchar	ec_split_data_check(void);
extern	void	Ope_EcEleUseDsp( void );
extern	void	SetEcVolume( void );
extern	void	EcDeemedSettlement( uchar *ope_faze );
extern	void	ReqEcDeemedJnlPrint( void );
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
extern	void	ec_BrandResultDisp(ushort brand_no, ushort reason_code);
extern	void 	ec_WarningMessage( ushort num );
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
extern	void	ec_MessageAnaOnOff( short OnOff, short num );
extern	void	Ope_EcPayStart(void);
extern	void	ec_wakeup_timeout(void);
extern	void	ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes);
// MH810105(S) MH364301 QRコード決済対応
extern	void	EcRegistMiryoTimeoutError(EC_SETTLEMENT_RES *pRes, char err);
// MH810105(E) MH364301 QRコード決済対応
extern	void	ec_start_removal_wait(void);
extern	void	ec_flag_clear(uchar init);
extern	void	EcSendCtrlEnableData(void);
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
extern  BOOL	Ec_check_cre_pay_limit(void);
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
extern	ushort	isEcBrandStsDataRecv(void);
extern	void	setEcBrandStsDataRecv( uchar kind );
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
extern	void	EcAlarmLog_Clear( void );									// ログクリア
extern	void	EcAlarmLog_Regist( EC_SETTLEMENT_RES *data );				// ログ登録
extern	void	EcAlarmLog_RepayLogRegist(Receipt_data *data);				// 再精算情報登録
extern	ushort	EcAlarmLog_GetCount( void );								// ログ件数取得
extern	short	EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log, Receipt_data *paylog );	// ログ取得
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
// MH810103 GG119202(S) 起動シーケンス不具合修正
extern	void	Ec_check_PendingBrandNego(void);
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) JVMAリセット処理変更
extern	void	Ec_check_PendingJvmaReset(void);
// MH810103 GG119202(E) JVMAリセット処理変更
// MH810103 GG119202(S) みなし決済扱い時の動作
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//extern	void	EcRegistSettlementAbort(void);
extern	void	EcRegistDeemedSettlementError(uchar kind);
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
extern	void	EcBrandClear(void);
extern	BOOL	EcCheckBrandSelectTable(BOOL in_mony_flg);
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) ハウスカードによるカード処理中タイムアウトは精算中止とする
extern	BOOL	IsSettlementBrand(ushort brandno);
// MH810103 GG119202(E) ハウスカードによるカード処理中タイムアウトは精算中止とする
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
extern	void	EcEnableNyukin(void);
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
// MH810103(s) 電子マネー対応 精算状態通知をOpe_modにより切り替える
extern	uchar isEcEmoneyEnabledNotCtrl(uchar open_chk, uchar zandaka_chk);
// MH810103(e) 電子マネー対応 精算状態通知をOpe_modにより切り替える
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
extern	uchar	EcRecvDeemed_DataSet(uchar *buf);
extern	void	EcRecvDeemed_RegistPri( void );
extern	void 	EcErrCodeChk(uchar *err_code, uchar err_brand_index);
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
extern	uchar	IsEcMiryoHikisariNG(Receipt_data *pData);
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する
// MH810105(S) MH364301 QRコード決済対応
extern	int		EcSubBrandCheck(ushort brand_no);
extern	int		EcGetSubBrandName(ushort brand_no, uchar kind, uchar idx, uchar *buff);
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
extern	uchar	EcBrandPostPayCheck(uchar brand_index);
// MH810105(E) MH364301 PiTaPa対応

// jvma_comm.c
extern	void	jvma_init(void);
extern	void	jvma_trb(uchar err_kind);
extern	void	jvma_event(void);
extern	void	jvma_command_set(void);
extern	short	jvma_act(void);

// ec_ctrl.c
extern	void	ec_init(void);
extern	short	ec_recv(uchar *buf, short size);
extern	void 	Ec_recv_data_regist(uchar *buf, ushort size);

#endif	// _EC_DEF_H_
