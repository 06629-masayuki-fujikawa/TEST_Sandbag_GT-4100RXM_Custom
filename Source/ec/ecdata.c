/*[]----------------------------------------------------------------------[]*/
/*| ecﾃﾞｰﾀ定義                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019-03-15                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<string.h>
#include	"suica_def.h"
#include	"system.h"

/* 決済リーダ コマンド */
const unsigned char	ec_tbl[][2] = {
	{ 0xC0, 0x3F },						// スタンバイ要求
	{ 0xC1, 0x3E },						// 一括要求
	{ 0xC2, 0x3D },						// 入力要求
	{ 0xC3, 0x3C },						// 出力指令
	{ 0xC4, 0x3B },						// 入力要求の再送、一括要求の再送
	{ 0xC5, 0x3A },						// 出力指令の再送
	{ 0xC2, 0x3D }						// 入力要求後の入力要求再送
};

struct ECCTL_rec	ECCTL;						// 決済リーダ制御
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//unsigned char		timer_recept_send_busy;		// 1=精算中のec受入可送信タイマー動作中
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
//EC_BRAND_TBL		RecvBrandTbl[2];			// 受信したブランド制御テーブル構造体
EC_BRAND_TBL		RecvBrandTblTmp[2];			// ブランドテーブルのtmp
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
EC_BRAND_TBL		BrandTbl[2];				// ブランド制御テーブル構造体
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//unsigned char		RecvVolumeRes;				// 受信した音量変更結果データ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
EC_VOLUME_TBL		VolumeTbl;					// 音量変更制御テーブル構造体
EC_MNT_TBL			RecvMntTbl;					// 受信したメンテナンステーブル構造体
EC_MNT_TBL			MntTbl;						// メンテナンステーブル構造体
EC_BRAND_RES_TBL	RecvBrandResTbl;			// リーダ受信したブランド選択結果テーブル構造体
EC_BRAND_SEL_TBL	BrandSelTbl;				// ブランド選択テーブル構造体
EC_SETTLEMENT_RES	Ec_Settlement_Res;			// 決済結果データ
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//unsigned char		Ec_Settlement_Sts;			// 決済状態データ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
t_EC_STATUS_DATA_INFO	EC_STATUS_DATA;			// 状態データ
t_EC_STATUS_DATA_INFO	PRE_EC_STATUS_DATA;		// 前回受信した状態データ
t_EC_STATUS_DATA_INFO	EC_STATUS_DATA_WAIT;	// 状態データ(受信待ち用)
unsigned char		Ec_Settlement_Rec_Code[3];	// 決済結果詳細エラーコード
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//unsigned char		ec_Disp_No;					// 決済結果詳細エラーコードの案内表示パターン
//short				ec_MessagePtnNum;			// LCD表示パターン
//unsigned short		Product_Select_Brand;		// 商品選択データ送信のブランド番号
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG116202(S) JVMAリセット回数制限対応
uchar				Ec_Jvma_Reset_Count;		// JVMAリセット回数
// MH810103 GG116202(E) JVMAリセット回数制限対応
// MH810103 GG119202(S) ブランド選択結果応答データ
EC_BRAND_RES_RSLT_TBL	BrandResRsltTbl;		// ブランド選択結果応答テーブル構造体
// MH810103 GG119202(E) ブランド選択結果応答データ
// MH810103 GG119202(S) 異常データフォーマット変更
uchar				err_data2;					// 異常データ（2バイト目）
// MH810103 GG119202(E) 異常データフォーマット変更
// MH810105(S) MH364301 QRコード決済対応
EC_SUB_BRAND_TBL	RecvSubBrandTbl;			// サブブランドテーブル構造体
// MH810105(E) MH364301 QRコード決済対応
