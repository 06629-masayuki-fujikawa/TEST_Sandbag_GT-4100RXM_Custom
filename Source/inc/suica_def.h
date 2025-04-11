/*[]----------------------------------------------------------------------[]*/
/*| headder file for common suica                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| DATE        : 2006-07-07                                               |*/
/*| UPDATE      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#ifndef _SUICA_DEF_H_
#define _SUICA_DEF_H_

// MH321800(S) G.So ICクレジット対応
#include	"ec_def.h"
#include	"system.h"
// MH321800(E) G.So ICクレジット対応

#define	S_BUF_MAXSIZE		256		// 送受信ﾊﾞｯﾌｧｻｲｽﾞ
#define TBL_CNT(a) (sizeof(a) / sizeof(a[0]))		/* レコード件数算出用マクロ */

// MH321800(S) G.So ICクレジット対応
//#define	SUICA_USE_ERR	(prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1 || \
//						Suica_Rec.suica_err_event.BYTE || \
//						Suica_Rec.Data.BIT.MIRYO_TIMEOUT  )	
#define	SUICA_USE_ERR	(jvma_setup.type == 0 || \
						Suica_Rec.suica_err_event.BYTE || \
						Suica_Rec.Data.BIT.MIRYO_TIMEOUT || \
						!Suica_Rec.Data.BIT.INITIALIZE)
#define	isSX10_USE()			(jvma_setup.type == 1)		// SX-10使用設定(jvma_setup.type == 2は拡張)
#define	isEC_USE()				(jvma_setup.type == 4)		// 決済リーダ使用設定
#define	isEMoneyReader()		(jvma_setup.type != 0)		// 電子マネーカードリーダー接続あり
#define	WAR_MONEY_CHECK			(!Suica_Rec.Data.BIT.MONEY_IN && !ryo_buf.nyukin )
#define	OPE_SUICA_MIRYO_TIME	prm_get(COM_PRM, S_SCA, 13, 3, 1)	// 255
#define	ENABLE_MULTISETTLE()	get_enable_multisettle() 	// 複数決済可能か判断
// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
#define	isEC_MSG_DISP()			(isEC_USE() && ECCTL.ec_MessagePtnNum != 0)		// 決済リーダ関連メッセージ表示中
#define	isEC_REMOVE_MSG_DISP()	(isEC_USE() && ECCTL.ec_MessagePtnNum == 4)		// カード抜き取りメッセージ表示中
#define	isEC_NG_MSG_DISP()		(isEC_USE() && ECCTL.ec_MessagePtnNum == 98)	// 決済NGメッセージ表示中
#define	isEC_STS_CARD_PROC()	(isEC_USE() && \
								 ECCTL.Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC)	// カード処理中
#define	isEC_STS_MIRYO()		(isEC_USE() && \
								 ECCTL.Ec_Settlement_Sts == EC_SETT_STS_MIRYO)	// 未了中
#define	isEC_CTRL_ENABLE()		(isEC_USE() && Suica_Rec.Data.BIT.CTRL && \
								 !SUICA_USE_ERR)								// 決済リーダが受付可
#define	isEC_CARD_INSERT()		(isEC_USE() && Suica_Rec.Data.BIT.CTRL_CARD)	// 決済リーダにカードあり
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) 未了仕様変更対応
#define	isEC_CONF_MIRYO()		(isEC_USE() && Suica_Rec.Data.BIT.MIRYO_CONFIRM)// [0x1B]EC状態データ(未了確定)受信済み
// MH810103 GG119202(E) 未了仕様変更対応
// GG116202(S) JVMAリセット回数制限対応
#define	hasEC_JVMA_RESET_CNT_EXCEEDED	(isEC_USE() && Ec_Jvma_Reset_Count > EC_JVMA_RESET_CNT)		// JVMAリセットリトライ回数越え
#define	isEC_JVMA_RESET_CNT_LESS_THAN	(isEC_USE() && Ec_Jvma_Reset_Count < EC_JVMA_RESET_CNT)		// JVMAリセットリトライ回数未満
// GG116202(E) JVMAリセット回数制限対応
// MH810103 GG119202(S) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
#define	isEC_MIRYO_TIMEOUT()	(isEC_USE() && Suica_Rec.Data.BIT.MIRYO_TIMEOUT)// 未了残高照会タイムアウト
// MH810103 GG119202(E) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
// MH810103 GG119202(S) ブランド状態判定処理変更
#define	isEC_BRAND_STS_KAIKYOKU(x)	(isEC_USE() && (((x) & 0x03) == 0x01))		// ブランド状態が開局＆有効であれば、開局として扱う
// MH810103 GG119202(E) ブランド状態判定処理変更
// MH810103 GG119202(S) サービス停止中ブランドはブランド設定データにセットしない
#define	isEC_BRAND_STS_ENABLE(x)	(isEC_USE() && (((x) & 0x0B) == 0x01))		// ブランド状態が開局＆有効＆サービス稼働中であれば、決済可能
// MH810103 GG119202(E) サービス停止中ブランドはブランド設定データにセットしない
// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
#define	isEC_PAY_CANCEL()		(isEMoneyReader() && PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm)
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）

#define	S_CNTL_DATA			0x10	// 制御ﾃﾞｰﾀ 
#define	S_SELECT_DATA		0x11	// 選択商品ﾃﾞｰﾀ
#define	S_INDIVIDUAL_DATA	0x15	// 個別ﾃﾞｰﾀ 
#define	S_PAY_DATA			0x16	// 精算ﾃﾞｰﾀ
#define	S_DSP_DATA			0x18	// 表示依頼ﾃﾞｰﾀ
#define	S_SETTLEMENT_DATA	0x19	// 決済ﾃﾞｰﾀ
#define	S_ERR_DATA			0x1A	// 異常ﾃﾞｰﾀ
#define	S_STATUS_DATA		0x1B	// 状態ﾃﾞｰﾀ
#define	S_TIME_DATA			0x1F	// 時間同期ﾃﾞｰﾀ
#define	S_FIX_DATA			0x0D	// 固定ﾃﾞｰﾀ

// MH321800(S) Y.Tanizaki ICクレジット対応
//#define	SUICA_RTRY_COUNT_31	10		/* Retry Count 10	*/
//#define	SUICA_RTRY_COUNT_21	10		/* Retry Count 10	*/
#define	SUICA_RTRY_COUNT_31	31		/* Retry Count 31	*/
#define	SUICA_RTRY_COUNT_21	21		/* Retry Count 21	*/
// MH321800(E) Y.Tanizaki ICクレジット対応
#define	SUICA_RTRY_COUNT_3	3	/* Retry Count 3	*/

#define RESPONSE_TIME_LIMIT 250		// SX-10応答待ち時間(約5秒)

#define S_DATA_RCV_TIMEOUT_ERR	0x04	// ﾃﾞｰﾀ受信時のﾀｲﾑｱｳﾄｴﾗｰ
#define S_NAK_RCV_ERR			0x08	// NAK受信時のﾘﾄﾗｲｴﾗｰ
#define S_LRC_ERR				0x10	// LRCｴﾗｰ
// MH321800(S) T.Nagai ICクレジット対応
//#define S_COM_SND_TIMEOUT_ERR	0x40	// ｺﾏﾝﾄﾞ送信後の受信ﾀｲﾑｱｳﾄｴﾗｰ
#define S_COM_RCV_TIMEOUT_ERR	0x40	// ｺﾏﾝﾄﾞ送信後の受信ﾀｲﾑｱｳﾄｴﾗｰ
#define S_COM_SND_TIMEOUT_ERR	0x80	// 送信ﾀｲﾑｱｳﾄｴﾗｰ
// MH321800(E) T.Nagai ICクレジット対応

#define	PAY_RYO_MAX	99990
#define	DELAY_MAX	8

enum {
	STANDBY_BEFORE,					// ｽﾀﾝﾊﾞｲ前
	STANDBY_SND_AFTER,				// ｽﾀﾝﾊﾞｲ送信後
	BATCH_SND_AFTER,				// 一括要求送信後	
	INPUT_SND_AFTER,				// 入力要求送信後
	OUTPUT_SND_AFTER,				// 出力指令送信後
	REINPUT_SND_AFTER,				// 入力再要求送信後
	REOUTPUT_SND_AFTER,				// 出力再指令送信後
	DATA_SND_AFTER,					// ﾃﾞｰﾀ送信後
	DATA_RCV,						// ﾃﾞｰﾀ受信
	MULTI_DATA_RCV_WAIT,			// 複数ﾃﾞｰﾀ待ち
};

typedef	struct {
	unsigned char		Result;										// 決済データ
	unsigned char		Column_No;									// 決済コラム番号
	unsigned char		Set_kind;									// 決算種別
	long				settlement_data;							// 実際の決済額
	long				settlement_data_before;						// 決算前Suica残高
	long				settlement_data_after;						// 決済後Suica残高
	unsigned char		Suica_ID[16];								// Suica ID
} SUICA_SETTLEMENT_RES;

extern	SUICA_SETTLEMENT_RES	Settlement_Res;	// Suicaﾃﾞｰﾀ管理構造体

typedef union{
	unsigned long	ULONG;					// shortからlongに拡張
	struct{	
// MH321800(S) G.So ICクレジット対応
//		unsigned char	YOBI2:8;			// Bit 24-31 = 予備2
//		unsigned char	YOBI1:3;			// Bit 21-23 = 予備1
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
//		unsigned char	YOBI:3;				// Bit 29-31 = 予備
//		unsigned char	MIRYO_CONFIRM:1;	// Bit 28 = 未了確定受信フラグ
//		unsigned char	BRAND_STS_RCV:1;	// Bit 27 = ブランド状態データ受信フラグ
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		unsigned char	YOBI:4;				// Bit 28-31 = 予備
		unsigned char	YOBI:2;				// Bit 30-31 = 予備
		unsigned char	BRAND_SEL:1;		// Bit 29 = ブランド選択中フラグ ブランド選択データ(33H)～ブランド選択結果データ(36H)のとき1になる。
		unsigned char	SELECT_SND:1;		// Bit 28 = 選択商品データ送信中
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		unsigned char	MIRYO_CONFIRM:1;	// Bit 27 = 状態データの未了確定受信フラグ
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
		unsigned char	RESET_RESERVED:1;	// Bit 26 = 初期化予約フラグ
		unsigned char	SETTLMNT_STS_RCV:1;	// Bit 25 = 決済中受信フラグ
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//		unsigned char	SLCT_BRND_BACK:1;	// Bit 24 = ブランド選択の戻る検出フラグ
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		unsigned char	YOBI2:1;			// Bit 24 = 予備
		unsigned char	BRAND_CAN:1;		// Bit 24 = ブランド選択中止要求
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
		unsigned char	CTRL_CARD:1;		// Bit 23 = カード差込状態
		unsigned char	PRE_CTRL:1;			// Bit 22 = 前回受信した制御データフラグ
		unsigned char	COMM_ERR:1;			// Bit 21 = 通信異常で決済リーダ利用不可
// MH321800(E) G.So ICクレジット対応
		unsigned char	MIRYO_NO_ANSWER:1;	// Bit 20 = Suicaから応答がなく、精算機タイマーでタイムアウト判断したときにセットする
		unsigned char	PRI_NG:1;			// Bit 19 = ジャーナル使用不可でSuica利用不可
		unsigned char	MONEY_IN:1;			// Bit 18 = 入金イベント受信済み
		unsigned char	ADJUSTOR_START:1;	// Bit 17 = 精算開始状態フラグ(商品選択データ送信可能フラグ)
		unsigned char	SEND_CTRL80:1;		// Bit 16 = 取引終了データ送信状態フラグ
		unsigned char	STATUS_DATA_SPLIT:1;// Bit 15 = 状態データの分割待ち合わせフラグ
		unsigned char	ADJUSTOR_NOW:1;		// Bit 14 = SUICA精算中判定フラグ
		unsigned char	DATA_RCV_WAIT:1;	// Bit 13 = 受信データ待ち合わせフラグ
		unsigned char	FUKUDEN_SET:1;		// Bit 12 = 複電時に精算データ送信要求有
		unsigned char	LOG_DATA_SET:1;		// Bit 11 = 決済結果データを受信後、精算ログに登録したかどうか
		unsigned char	SETTLEMENT_ERR_RCV:1;	// Bit 10 = Opeが処理する前に決済結果の2重受信発生ﾌﾗｸﾞ
		unsigned char	MIRYO_ARM_DISP:1;	// Bit 9 = Suica未了発生後のメッセージ表示
		unsigned char	MIRYO_TIMEOUT:1;	// Bit 8 = Suica未了発生後タイムアウト
		unsigned char	OPE_CTRL:1;			// Bit 7 = 予備
		unsigned char	PAY_CANSEL:1;		// Bit 6 = 電子ﾏﾈｰの精算中止が行われたかどうか
		unsigned char	PAY_CTRL:1;			// Bit 5 = 精算時にSuica精算が行われたかどうか
		unsigned char	RESET_COUNT:1;		// Bit 4 = ﾘｾｯﾄﾌﾗｸﾞ
		unsigned char	CTRL_MIRYO:1;		// Bit 3 = 処理未了状態
		unsigned char	CTRL:1;				// Bit 2 = 制御データフラグ
		unsigned char	INITIALIZE:1;		// Bit 1 = 初期化フラグ
		unsigned char	EVENT:1;			// Bit 0 = Suicaイベントフラグ
	} BIT;
} t_Suica_Data;

typedef union{
	unsigned char	BYTE;					// Byte
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	struct{
//		unsigned char	YOBI:2;				// Bit 6-7 = 予備
//// MH321800(S) G.So ICクレジット対応
////		unsigned char	FIX_DATA:1;			// Bit 5 = 固定ﾃﾞｰﾀ
////		unsigned char	TIME_DATA:1;		// Bit 4 = 時間同期ﾃﾞｰﾀ
//		unsigned char	SETTSTS_DATA:1;		// Bit 5 = 決済状態ﾃﾞｰﾀ
//		unsigned char	BRAND_DATA:1;		// Bit 4 = ブランド選択ﾃﾞｰﾀ
//// MH321800(E) G.So ICクレジット対応
//		unsigned char	STATUS_DATA:1;		// Bit 3 = 状態ﾃﾞｰﾀ
//// MH321800(S) T.Nagai ICクレジット対応
////		unsigned char	ERR_DATA:1;			// Bit 2 = 異常ﾃﾞｰﾀ
//		unsigned char	YOBI2:1;			// Bit 2 = 予備
//// MH321800(E) T.Nagai ICクレジット対応
//		unsigned char	SETTLEMENT_DATA:1;	// Bit 1 = 決済結果ﾃﾞｰﾀ
//		unsigned char	DSP_DATA:1;			// Bit 0 = 表示依頼ﾃﾞｰﾀ
//	} BIT;
	struct{
		unsigned char	YOBI:2;				// Bit 6-7 = 予備
		unsigned char	FIX_DATA:1;			// Bit 5 = 固定ﾃﾞｰﾀ
		unsigned char	TIME_DATA:1;		// Bit 4 = 時間同期ﾃﾞｰﾀ
		unsigned char	STATUS_DATA:1;		// Bit 3 = 状態ﾃﾞｰﾀ
		unsigned char	ERR_DATA:1;			// Bit 2 = 異常ﾃﾞｰﾀ
		unsigned char	SETTLEMENT_DATA:1;	// Bit 1 = 決済結果ﾃﾞｰﾀ
		unsigned char	DSP_DATA:1;			// Bit 0 = 表示依頼ﾃﾞｰﾀ
	} BIT;
	struct {
		unsigned char	YOBI:4;
		unsigned char	BRAND_DATA:1;		// Bit 3 = ブランド選択ﾃﾞｰﾀ
		unsigned char	SETTSTS_DATA:1;		// Bit 2 = 決済状態ﾃﾞｰﾀ
		unsigned char	STATUS_DATA:1;		// Bit 1 = 状態ﾃﾞｰﾀ
		unsigned char	SETTLEMENT_DATA:1;	// Bit 0 = 決済結果ﾃﾞｰﾀ
	} EC_BIT;
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
} t_Suica_rcv_event;

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
#define	SETTLEMENT_COMPLETE_COND		0x07	// 決済状態データ、状態データ、決済結果データ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
// MH321800(S) G.So ICクレジット対応
//		unsigned char	YOBI:4;				// Bit 4-7 = 予備
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
//		unsigned char	YOBI:1;				// Bit 7 = 予備
		unsigned char	SETT_RCV_FAIL:1;	// Bit 7 = 決済結果受信失敗
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
// ※リーダが受付可にならない条件を追加
		unsigned char	SETTLEMENT_ERR:1;	// Bit 6 = 決済異常
		unsigned char	BRAND_NONE:1;		// Bit 5 = ブランド設定なし(１つでも使用可能ならOK)
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//		unsigned char	BRANDSETUP_NG:1;	// Bit 4 = ブランド設定誤り(E3264)
// MH810103 GG119202(S) 異常データフォーマット変更
//		unsigned char	YOBI2:1;			// Bit 4 = 予備
		unsigned char	ERR_RECEIVE2:1;		// Bit 4 = 異常データ（2バイト目）受信
// MH810103 GG119202(E) 異常データフォーマット変更
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH321800(E) G.So ICクレジット対応
		unsigned char	PAY_DATA_ERR:1;		// Bit 3 = 決済額と商品選択データとの額に差異がある
		unsigned char	OPEN:1;				// Bit 2 = 異常ﾃﾞｰﾀ
		unsigned char	ERR_RECEIVE:1;		// Bit 1 = SX-10異常ﾃﾞｰﾀ受信
		unsigned char	COMFAIL:1;			// Bit 0 = 通信不良
	} BIT;
} t_Suica_Err_event;
// MH810103 MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
//#define	EC_NOT_AVAILABLE_ERR	0x4A		// 決済リーダ切り離しエラー(通信不良除く)
#define	EC_NOT_AVAILABLE_ERR	0xCA		// 決済リーダ切り離しエラー(通信不良除く)
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
// MH810103 MH321800(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
// MH810103 GG119202(S) 起動シーケンス完了条件見直し

#define EC_BRAND10_STS_ALL_RECV		0x00000003UL
#define EC_BRAND20_STS_ALL_RECV		0x0000000FUL

typedef union{
	unsigned long	ULONG;					// ULONG
	struct{
		unsigned long	YOBI:27;			// Bit 5-31 = 予備
		unsigned long	VOL_RES_RCV:1;		// Bit 4 = 音声変更結果データ受信
		unsigned long	BR_STS4_RCV:1;		// Bit 3 = ブランド状態データ２受信(2回目)
		unsigned long	BR_STS3_RCV:1;		// Bit 2 = ブランド状態データ２受信(1回目)
		unsigned long	BR_STS2_RCV:1;		// Bit 1 = ブランド状態データ１受信(2回目)
		unsigned long	BR_STS1_RCV:1;		// Bit 0 = ブランド状態データ１受信(1回目)
	} BIT;
} t_Ec_BrandSts_event;

typedef	struct {
	t_Ec_BrandSts_event	brand_rcv_event;	// ブランド状態受信ｲﾍﾞﾝﾄ詳細
} t_Ec_negoc_Data;
// MH810103 GG119202(E) 起動シーケンス完了条件見直し

typedef struct {
	unsigned char	Status;					// 状態管理ﾃﾞｰﾀ
	unsigned char	Com_kind;				// コマンド種別
	unsigned char	snd_kind;				// ﾃﾞｰﾀ種別
	unsigned char	faze;					// フェーズ管理
	t_Suica_Data 	Data;					// Suica用イベント管理
	unsigned char	Snd_Buf[S_BUF_MAXSIZE];	// 送信ﾃﾞｰﾀ格納領域
	short	Snd_Size;						// 送信ﾃﾞｰﾀｻｲｽﾞ
	unsigned char	Rcv_Buf[S_BUF_MAXSIZE];	// 受信ﾃﾞｰﾀ格納領域
	short	Rcv_Size;						// 送信ﾃﾞｰﾀｻｲｽﾞ
	t_Suica_rcv_event	suica_rcv_event;	// suica受信ｲﾍﾞﾝﾄ詳細
	t_Suica_rcv_event	suica_rcv_event_sumi;	// suica受信ｲﾍﾞﾝﾄ処理済情報
	t_Suica_Err_event	suica_err_event;	// suicaErrｲﾍﾞﾝﾄ詳細
	t_Suica_rcv_event	rcv_split_event;	// suica分割受信ｲﾍﾞﾝﾄ詳細	
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
	t_Ec_negoc_Data		ec_negoc_data;		// 決済リーダ ブランドネゴシエーション管理
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
} SUICA_REC;

typedef struct {
	unsigned char	snd_kind;				// ﾃﾞｰﾀ種別
	unsigned char	Snd_Buf[S_BUF_MAXSIZE]; // 送信ﾃﾞｰﾀﾊﾞｯﾌｧ
} S_SEND_BUF;

typedef struct {
	unsigned char	write_wpt;			// 書込みﾎﾟｲﾝﾀ
	unsigned char	read_wpt;			// 読み込みﾎﾟｲﾝﾀ
	S_SEND_BUF		Suica_Snd_q[5];		// 送信ﾃﾞｰﾀｷｭｰ
} SUICA_SEND_BUFF;
typedef struct{
	unsigned char kind;
	unsigned char bc;
	unsigned char dc;
	unsigned char data[1];
}t_RCVBUFINFO,*p_RCVBUFINFO;
extern unsigned char suica_work_buf[S_BUF_MAXSIZE];
extern struct	clk_rec	suica_work_time;
typedef union{
	unsigned char	status_data;			// Byte
	struct{
		unsigned char	YOBI:3;				// Bit 5-7 = 予備
		unsigned char	MiryoStatus:1;		// Bit 4 = 処理未了状態（0：未了中で無い 1：処理未了中）
		unsigned char	DetectionSwitch:1;	// Bit 3 = 締めｽｲｯﾁ検知（0：未検知状態 1：締めｽｲｯﾁ検知）
		unsigned char	Running:1;			// Bit 2 = 実行中（0：実行中で無い 1：実行中）
		unsigned char	TradeCansel:1;		// Bit 1 = 取引ｷｬﾝｾﾙ（0：取引ｷｬﾝｾﾙで無い 1：取引ｷｬﾝｾﾙ受付）
		unsigned char	ReceptStatus:1;		// Bit 0 = 受付状態（0：受付可 1：受付不可）
	} StatusInfo;
} t_STATUS_DATA_INFO;

extern t_STATUS_DATA_INFO	STATUS_DATA_WAIT;		// 状態データ(受信待ち用)

extern	SUICA_REC	Suica_Rec;	// Suicaﾃﾞｰﾀ管理構造体
extern	unsigned char	suica_errst;		/* Communication  Status 		*/

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)のための定義移動
typedef union{
	unsigned char	BYTE;
	struct {
		unsigned char	YOBI				:7;		/* B1-7 = 予備 */
		unsigned char	suica_zangaku_dsp	:1;		/* B0 = Suica残額表示用制御ﾌﾗｸﾞ */
	} BIT;
} t_Edy_Dsp_erea;

extern t_Edy_Dsp_erea	edy_dsp;

#define	BUF_MAX_DELAY_TIME			100L		// ﾊﾞｯﾌｧMAX時の送信完了待ち時間(1秒)
#define	BUF_MAX_DELAY_COUNT			5			// ﾊﾞｯﾌｧMAX時の送信完了待ち回数(5回)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)のための定義移動

extern unsigned long	pay_dsp;							// 表示依頼データ
extern unsigned char	err_data;							// 異常データ
extern t_STATUS_DATA_INFO	STATUS_DATA;					// 状態データ
// MH321800(S) G.So ICクレジット対応
extern t_STATUS_DATA_INFO	PRE_STATUS_DATA;				// 前回受信した状態データ
// MH321800(E) G.So ICクレジット対応
extern struct			clk_rec	time_data;					// 時間同期データ
extern unsigned char	fix_data[2];						// 固定データ
extern const unsigned char	suica_tbl[][2];
extern unsigned char 	suica_err[5];						/* Communication  Status */
extern unsigned char 	suica_errst;						/* Communication  Status */
extern unsigned char	suica_errst_bak;
extern unsigned char 	Retrycount;							/* Retryｶｳﾝﾄ数 */
extern unsigned char	dsp_work_buf[2];					// 画面表示用一時退避領域
extern unsigned char	Status_Retry_Count_NG;				// 受付不可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
extern unsigned char	Status_Retry_Count_OK;					// 受付不可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
extern unsigned char	Ope_Suica_Status;					// Ope側からの送信ｽﾃｰﾀｽ
extern	SUICA_SEND_BUFF	Suica_Snd_Buf;	// Suicaﾃﾞｰﾀ管理構造体
extern unsigned char	disp_media_flg;						// 画面表示用　精算媒体使用可(0)否(1)
extern unsigned short	DspChangeTime[2];
extern unsigned char	DspWorkerea[7];						// 駐車券挿入待ち画面　利用媒体表示ﾒｯｾｰｼﾞNo.

// MH321800(S) D.Inaba ICクレジット対応
//extern unsigned char	dsp_change;
extern unsigned char	dsp_change;						// 未了発生中の画面表示ﾌﾗｸﾞ 0=未了関係表示なし 1=未了関係表示中 2=未了関係表示中(制御データ未了残高通知終了送信後)
// MH321800(E) D.Inaba ICクレジット対応
extern unsigned char	dsp_fusoku;						// 残高不足ｱﾗｰﾑ表示制御ﾌﾗｸﾞ
extern long	w_settlement;								// Suica残高不即時の精算額（０時は残高不足でないを意味する）
extern unsigned char	suica_fusiku_flg;				// 利用可能媒体ｻｲｸﾘｯｸ表示の停止フラグ（0:表示／1:停止）
extern unsigned char	mode_Lagtim10;					// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示 1:Suica停止後の受付許可再送信Wait終了
extern unsigned short	nyukin_delay[DELAY_MAX];		// 未了発生中のﾒｯｾｰｼﾞを保持 [0]:ｺｲﾝﾒｯｸ	[1]:紙幣
extern unsigned char	delay_count;					// 未了発生中のﾒｯｾｰｼﾞｶｳﾝﾄ数
extern unsigned short	wk_media_Type;					// 精算媒体（電子マネー）種別
extern unsigned	char	time_out;						// 画面戻りﾀｲﾑｱｳﾄ発生判定領域
				
extern long				Product_Select_Data;			// 送信した商品選択データ(決済時の比較用)
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//// MH321800(S) D.Inaba ICクレジット対応
//extern unsigned	short	Product_Select_Brand;			// 商品選択データ送信のブランド番号
//// MH321800(E) D.Inaba ICクレジット対応
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
/*** function prototype ***/

extern	void	suica_save( unsigned char * ,short );
extern	void	suica_init( void );
extern	void	suica_command_set( void );
extern	void	SUICA_RCLR( void );
extern	void	time_get_rcv( struct clk_rec *sndbuf, void *snddata);
extern	void	time_set_snd( unsigned char *sndbuf, void *snddata );
extern	void	Suica_Data_Snd( unsigned char kind, void *snddata );
extern	void	suicatrb( unsigned char err_kind );
extern	void	Suica_reset( void );
extern  void	snd_ctrl_timeout( void );
extern  void	snd_ctrl_timeout2( void );
extern  void	snd_ctrl_timeout3( void );
extern  void	miryo_timeout( void );

extern	void 	suica_event( void );
extern	short	suicaact( void );
extern	short	suica_recv( unsigned char *, short );
extern  long	settlement_amount( unsigned char *buf );

extern  short	Log_Count_search( unsigned char	search_kind );
extern	short	Ope_Suica_Event( unsigned short	msg, unsigned char ope_faze );
extern	void	Ope_EleUseDsp( void );
extern	short	Ope_TimeOut_10( unsigned char ope_faze, unsigned short  e_pram_set );
extern	void	Ope_TimeOut_11( unsigned char ope_faze, unsigned short  e_pram_set );
extern	char	Ope_ArmClearCheck( unsigned short	msg );
extern	char	Ope_MiryoEventCheck( unsigned short	msg );

extern	void	op_SuicaFusokuOff( void );
extern	void	op_SuicaFusoku_Msg( unsigned long dat, unsigned char *buff, unsigned char use_kind );
extern	void	op_mod01_dsp_sub( void );
extern	void	op_mod01_dsp( void );
extern	void	nyukin_delay_check( unsigned short *postdata, unsigned char count );

extern	char	First_Pay_Dsp( void );
extern	void	Cansel_Ele_Start( void );
extern	void	Op_ReturnLastTimer( void );

extern	unsigned char	Op_StopModuleWait( unsigned char );
extern  unsigned char   Op_StopModuleWait_sub( unsigned char );

extern  void	Settlement_rcv_faze_err( unsigned char *dat, unsigned char kind);

extern  unsigned char	rcv_split_data_check( void );
extern  void	Suica_rcv_split_flag_clear( void );

extern	void 	Suica_Log_regist( unsigned char* logdata, unsigned short log_size, unsigned char kind );
extern	unsigned char 	Suica_Snd_regist( unsigned char *kind, unsigned char *snddata );
extern	void	Op_Cansel_Wait( void );
extern  unsigned char    Op_Cansel_Wait_sub( unsigned char );
extern  void    Suica_fukuden_que_check( void );
extern  unsigned char    Miryo_Exec_check( void );
extern  char    in_credit_check( void );
// MH321800(S) G.So ICクレジット対応
extern  void 	miryo_timeout_after_disp( void );
extern  void	lcd_wmsg_dsp_elec( char type, const unsigned char *msg1, const unsigned char *msg2, unsigned char timer, unsigned char ann, unsigned short color, unsigned short blink );
extern  void	Suica_Status_Chg( void );
// MH321800(E) G.So ICクレジット対応
extern  void    miryo_timeout_after_proc( void );
extern  void    miryo_timeout_after_mif( unsigned char );
extern  unsigned short   SuicaErrCharChange( unsigned short ItemNum );
extern  void	Suica_Ctrl( unsigned short req, unsigned char type );

extern  unsigned char	SuicaUseKindCheck( unsigned char );

extern  unsigned char	StopStatusGet( unsigned char );

extern  unsigned char	DailyAggregateDataBKorRES( unsigned char );

extern	void	PayDataErrDisp( void );
extern	void	PayDataErrDispClr( void );

extern	short	SuicaParamGet( void );
// MH322914 (s) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
extern	void	PrinterCheckForSuica( void );
// MH322914 (e) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
//// MH321800(S) G.So ICクレジット対応
//// 決済リーダ 処理未了取引ログ
//extern	void	EcAlarmLog_Clear( void );									// ログクリア
//extern	void	EcAlarmLog_Regist( EC_SETTLEMENT_RES *data );				// ログ登録
//extern	ushort	EcAlarmLog_GetCount( void );								// ログ件数取得
//extern	short	EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log );	// ログ取得
//// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

#endif	// _SUICA_DEF_H_
