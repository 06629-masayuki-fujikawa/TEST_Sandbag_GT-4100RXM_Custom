//[]----------------------------------------------------------------------[]
///	@brief			決済リーダR/W Operation control
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"suica_def.h"
#include	"ec_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
#include	"flp_def.h"
#include	"lkmain.h"

/*
	CRW-MJAマルチマネーR/Wの制御を行ないます。
	ope_suica_ctrl.cを下敷きに受信処理をカスタマイズしています。
*/

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

extern	uchar	suica_dsp_buff[30];						// 画面表示用表示文字列格納領域
extern	uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// 通信ログ用一時編集領域
extern	uchar	wrcvbuf[S_BUF_MAXSIZE+1];				// データ受信用の一時編集領域
extern	uchar	err_wk[40];
extern	uchar	moni_wk[10];
// MH810105(S) MH364301 QRコード決済対応
//// MH810103 GG119202(S) みなし決済扱い時の動作
//extern	const uchar	msg_idx[4];
//// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810105(E) MH364301 QRコード決済対応

extern	const char *EcBrandKind_prn[];
// MH810105(S) MH364301 QRコード決済対応
extern	const char *QrBrandKind_prn[];
// MH810105(E) MH364301 QRコード決済対応

#define		Ec_Status_Chg()		Suica_Status_Chg()	// Rename

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static	uchar	Ec_Read_RcvQue(void);
static	void	ec_data_save(unsigned char *buf, short siz);
static	void	set_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl);
static	void	check_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl);
// MH810105(S) MH364301 QRコード決済対応
static	void	check_sub_brand_tbl(uchar *p, EC_SUB_BRAND_TBL *tbl);
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
static	void	update_brand_tbl(EC_BRAND_TBL *srctbl, EC_BRAND_TBL *recvtbl);
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//static	void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf);
static	void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf, uchar errchk);
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

static  void	snd_enable_timeout(void);
static  void	snd_disable_timeout(void);
static	void	snd_no_response_timeout(void);
// MH810103 GG119202(S) 不要処理削除
//static	void	status_err_timeout(void);
// MH810103 GG119202(E) 不要処理削除
static	void	not_open_timeout(void);

static short	EcRecvStatusData(uchar*);
static short	EcRecvSettData(uchar*);
static short	EcRecvBrandData(uchar*);

static	void	ReqEcAlmRctPrint(uchar prikind, EC_SETTLEMENT_RES *data);

static	void	recv_unexpected_status_data(void);
static	void	save_deemed_data(short brandno);

// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
//static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code);
static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code, uchar brand_index);
// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
//// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
//static	uchar	ec_errcode_err_chk_judg( uchar *ec_err_code );
//// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
static	uchar	ec_errcode_err_chk_judg(uchar *wk_err_code, const uchar ErrTbl[][3], uchar max);
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する
//static	void	ec_WarningMessage(uchar num);
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

static	void	ecRegistBrandError( EC_BRAND_CTRL *brand_ctrl, uchar err_type );
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
static	void	ec_set_brand_selected( void );
static	BOOL	ec_check_valid_brand( ushort brand_no );
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
// MH810103 GG119202(S) 起動シーケンス不具合修正
static	void	setEcBrandNegoComplete(void);
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
static BOOL	IsValidBrand(int brand_i, int brand_j);
static BOOL	IsBrandSpecified(ushort brandno);
// MH810103 GG119202(S) ハウスカードによるカード処理中タイムアウトは精算中止とする
//static BOOL IsSettlementBrand(ushort brandno);
// MH810103 GG119202(E) ハウスカードによるカード処理中タイムアウトは精算中止とする
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応参考）
static	uchar	EcNotifyCardData(ushort brand_no, uchar *dat);
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応参考）
// MH810103 MHUT40XX(S) Edy・WAON対応
static	void	EcTimerUpdate(void);
// MH810103 MHUT40XX(E) Edy・WAON対応

// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
extern	void	lcdbm_notice_dsp2( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info );
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//static	void	EcSetDeemedSettlementResult(EC_SETTLEMENT_RES *data);
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善
// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
static	uchar	EcGetActionWhenFailure( ushort brand_no );
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
// MH810105 GG119202(S) 電子マネー決済エラー発生時の画面戻り仕様変更
static	uchar	IsEcContinueReading(uchar *pErrCode);
// MH810105 GG119202(E) 電子マネー決済エラー発生時の画面戻り仕様変更
// MH810105(S) MH364301 QRコード決済対応
static	uchar	IsQrContinueReading(uchar *pErrCode);
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
static	uchar	EcGetMiryoTimeoutAction(ushort brand_no);
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更


/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/
typedef short (*CMD_FUNC)(uchar*);

typedef struct{
	unsigned char Command;
	CMD_FUNC Func;
}RCV_CMD_FUNC;

static	const
RCV_CMD_FUNC EcTbl[] = {
//	{ 受信ｺﾏﾝﾄﾞﾋﾞｯﾄ,処理関数},				/* 処理名 */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	{ 0x02,			EcRecvSettData},		/* 決済結果データ */
//	{ 0x08,			EcRecvStatusData},		/* 状態データ */
//	{ 0x10,			EcRecvBrandData},		/* ブランド選択結果データ */
	{ 0x01,			EcRecvSettData},		/* 決済結果データ */
	{ 0x02,			EcRecvStatusData},		/* 状態データ */
	{ 0x08,			EcRecvBrandData},		/* ブランド選択結果データ */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
};

// ブランド別設定参照テーブル
typedef struct {
	ushort		brand_no;				// BRANDNO_*
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//	uchar		address;				// 共通パラメーター アドレス
//	uchar		pos;					// 共通パラメーター 設定位置
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH810103 GG119202(S) 決済ありブランド選択設定を参照する
	uchar		address;				// 共通パラメーター アドレス
	uchar		pos;					// 共通パラメーター 設定位置
// MH810103 GG119202(E) 決済ありブランド選択設定を参照する
	uchar		brand_err_no;			// ブランド異常エラー番号
} EC_BRAND_ENABLED_PARAM_TBL;

// MH810105(S) MH364301 QRコード決済対応
typedef struct {
	uchar		brand_no;				// サブブランド種別
	uchar		address;				// 共通パラメーター アドレス
	uchar		pos;					// 共通パラメーター 設定位置
	uchar		dmy;
} EC_SUB_BRAND_ENABLED_PARAM_TBL;
// MH810105(E) MH364301 QRコード決済対応

// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
//	// 各ブランドの使用有無設定対応 (共通パラメーター 50-xxxx)
//	//	BRANDNO_*			address	pos    brand_err_no
//	{	BRANDNO_KOUTSUU,	1,		6,	ERR_EC_KOUTU_ABNORMAL,		},
//	{	BRANDNO_QUIC_PAY,	1,		5,	ERR_EC_QUICPAY_ABNORMAL,	},
//	{	BRANDNO_ID,			1,		4,	ERR_EC_ID_ABNORMAL,			},
//	{	BRANDNO_SAPICA,		1,		3,	0,							},
//	{	BRANDNO_WAON,		1,		2,	ERR_EC_WAON_ABNORMAL,		},
//	{	BRANDNO_NANACO,		1,		1,	ERR_EC_NANACO_ABNORMAL,		},
//	{	BRANDNO_EDY,		2,		6,	ERR_EC_EDY_ABNORAML,		},
////	{	BRANDNO_HOUJIN,		2,		2,	0,							},
//	{	BRANDNO_CREDIT,		2,		1,	ERR_EC_CREDIT_ABNORMAL,		},
//};
// MH810103 GG119202(S) 決済ありブランド選択設定を参照する
//const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
//	//	BRANDNO_*			brand_err_no
//	{	BRANDNO_KOUTSUU,	ERR_EC_KOUTU_ABNORMAL,		},
//	{	BRANDNO_QUIC_PAY,	ERR_EC_QUICPAY_ABNORMAL,	},
//	{	BRANDNO_ID,			ERR_EC_ID_ABNORMAL,			},
//	{	BRANDNO_SAPICA,		0,							},
//	{	BRANDNO_WAON,		ERR_EC_WAON_ABNORMAL,		},
//	{	BRANDNO_NANACO,		ERR_EC_NANACO_ABNORMAL,		},
//	{	BRANDNO_EDY,		ERR_EC_EDY_ABNORAML,		},
//	{	BRANDNO_CREDIT,		ERR_EC_CREDIT_ABNORMAL,		},
//// 非決済ブランド
//	{	BRANDNO_TCARD,		ERR_EC_TCARD_ABNORAML,		},
//// 後決済ブランド
//	{	BRANDNO_HOUJIN,		ERR_EC_HOUJIN_ABNORAML,		},
//};
	// 決済ありブランドのみ
const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
	//	brand_no			address	pos	brand_err_no
	{	BRANDNO_KOUTSUU,	1,		6,	ERR_EC_KOUTU_ABNORMAL,		},
	{	BRANDNO_QUIC_PAY,	1,		5,	ERR_EC_QUICPAY_ABNORMAL,	},
	{	BRANDNO_ID,			1,		4,	ERR_EC_ID_ABNORMAL,			},
	{	BRANDNO_SAPICA,		1,		3,	0,							},
	{	BRANDNO_WAON,		1,		2,	ERR_EC_WAON_ABNORMAL,		},
	{	BRANDNO_NANACO,		1,		1,	ERR_EC_NANACO_ABNORMAL,		},
	{	BRANDNO_EDY,		2,		6,	ERR_EC_EDY_ABNORAML,		},
	{	BRANDNO_CREDIT,		2,		1,	ERR_EC_CREDIT_ABNORMAL,		},
// MH810105(S) MH364301 QRコード決済対応
	{	BRANDNO_QR,			2,		2,	ERR_EC_QR_ABNORMAL,			},
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
	{	BRANDNO_PITAPA,		2,		5,	ERR_EC_PITAPA_ABNORMAL		},
// MH810105(E) MH364301 PiTaPa対応
};
// MH810103 GG119202(E) 決済ありブランド選択設定を参照する
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない

// MH810105(S) MH364301 QRコード決済対応
// サブブランドを持つブランド番号が追加された場合は
// 下記テーブルに追加すること
const ushort EcSubBrandUseTbl[] = {
	BRANDNO_QR,
};

// MH810105(S) MH364301 QRコード決済対応（仕様変更）
//const EC_SUB_BRAND_ENABLED_PARAM_TBL QrSubBrandEnabledParamTbl[] = {
//	//	brand_no			address	pos
//	{	DPAY,				50,		1	},
//	{	PAYPAY,				50,		2,	},
//	{	AUPAY,				50,		3,	},
//	{	RAKUTENPAY,			50,		4,	},
//	{	MERPAY,				50,		5,	},
//};
// MH810105(E) MH364301 QRコード決済対応（仕様変更）
// MH810105(E) MH364301 QRコード決済対応

// MH810103 GG119202(S) 不要処理削除
//const ushort EcBrandEmoney[] = {
//	// 電子マネーのブランド
//	EC_UNKNOWN_USED,
//	EC_ZERO_USED,
//	EC_EDY_USED,
//	EC_NANACO_USED,
//	EC_WAON_USED,
//	EC_SAPICA_USED,
//	EC_KOUTSUU_USED,
//	EC_ID_USED,
//	EC_QUIC_PAY_USED,
//};
//
//const ushort EcBrandCredit[] = {
//	// クレジットのブランド
//	EC_CREDIT_USED,
//};
// MH810103 GG119202(E) 不要処理削除

typedef struct {
	//	EC_*_USEDとBRANDNO_*の対応定義用
	ushort			brand_ec;				// EC_*_USED
	ushort			brand_no;				// BRANDNO_*
} EC_BRAND_NO_CONV_TBL;

// 決済ブランドのみ
const EC_BRAND_NO_CONV_TBL EcBrandNoConvTbl[] = {
	//	EC_*_USEDとBRANDNO_*の対応
	//	EC_*_USED				BRANDNO_*
	{	EC_EDY_USED,			BRANDNO_EDY			},
	{	EC_NANACO_USED,			BRANDNO_NANACO		},
	{	EC_WAON_USED,			BRANDNO_WAON		},
	{	EC_SAPICA_USED,			BRANDNO_SAPICA		},
	{	EC_KOUTSUU_USED,		BRANDNO_KOUTSUU		},
	{	EC_ID_USED,				BRANDNO_ID			},
	{	EC_QUIC_PAY_USED,		BRANDNO_QUIC_PAY	},
	{	EC_CREDIT_USED,			BRANDNO_CREDIT		},
// MH810105(S) MH364301 QRコード決済対応
	{	EC_QR_USED,				BRANDNO_QR,			},
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
	{	EC_PITAPA_USED,			BRANDNO_PITAPA		},
// MH810105(E) MH364301 PiTaPa対応
};

// エラーコード文字 下記のEcErrCodeTblと一致
enum {
	EC_ERR_C,
	EC_ERR_D,
	EC_ERR_E,
	EC_ERR_G,
	EC_ERR_K,
	EC_ERR_P,
	EC_ERR_S,
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
	EC_ERR_a,
	EC_ERR_c,
	EC_ERR_d,
	EC_ERR_e,
	EC_ERR_g,
	EC_ERR_h,
	EC_ERR_i,
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
// MH810105(S) MH364301 QRコード決済対応
	EC_ERR_QR_P,
	EC_ERR_QR_Q,
// MH810105(E) MH364301 QRコード決済対応
	EC_ERR_MAX,
};

// 各エラーコード番号にエラーが存在しない = 0
// 只今、お取り扱いができません/只今、クレジットカードはお取り扱いできません = 1
// このカードは無効です = 2
// このカードは使えません = 3
// このカードは期限切れです = 4
// カードの読み取りに失敗しました = 5
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
//// カードの残高がありません = 6
// 残高不足です = 6
// MH810105(S) MH364301 QRコード決済対応
// 取引に失敗しました。他の支払方法をご利用ください = 10
// このコードは期限切れです = 11
// コードを更新し、もう一度やり直してください = 12
// このコードは使えません = 13
// このコードは残高不足です = 14
// 精算限度額を超えています。他の支払方法をご利用ください = 15
// もう一度やり直してください = 16
// MH810105(E) MH364301 QRコード決済対応
// MH810105 GG119202(S) iD決済時の案内表示対応
// iDアプリがロックされています = 94
// MH810105 GG119202(E) iD決済時の案内表示対応
// GG118809_GG118909(S) 未引き去り確認時の未了理由に対応する
// 未了残高照会で未引き去りを確認 = 96
// GG118809_GG118909(E) 未引き去り確認時の未了理由に対応する
// 放送なし = 98
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
// 各詳細エラーにて表示する上記、文字番号を設定
const uchar EcErrCodeTbl[EC_ERR_MAX][ERR_NUM_MAX] = {
//		0  1  2  3	4  5  6  7  8  9 
	{	// Cコードエラー
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 1, 1, 1, 1, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		1, 1, 0, 1, 1, 1, 1, 1, 1, 0,  	/* ｺｰﾄﾞ：50～59 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Dコードエラー
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 	/* ｺｰﾄﾞ： 0～ 9 */
		1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 	/* ｺｰﾄﾞ：10～19 */
		1, 1, 2, 1, 5, 5, 5, 5, 1, 1, 	/* ｺｰﾄﾞ：20～29 */
		1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 	/* ｺｰﾄﾞ：30～39 */
		1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 	/* ｺｰﾄﾞ：40～49 */
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 	/* ｺｰﾄﾞ：50～59 */
		1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 	/* ｺｰﾄﾞ：70～79 */
		1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 	/* ｺｰﾄﾞ：80～89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Eコードエラー
		0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Gコードエラー
		0, 0, 0, 0, 0, 0, 6, 3, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 3, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 3, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		3, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 3, 3, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 3, 3, 3, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		3, 2, 0, 0, 0, 3, 0, 3, 3, 3,  	/* ｺｰﾄﾞ：60～69 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  	/* ｺｰﾄﾞ：70～79 */
		3, 3, 0, 4, 0, 3, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 1, 0, 0, 3, 1, 3, 3, 3, 3,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Kコードエラー
		0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Pコードエラー
		0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 5, 0, 0, 5, 5,  	/* ｺｰﾄﾞ：60～69 */
		5, 5, 5, 5, 5, 5, 5, 0, 5, 0,  	/* ｺｰﾄﾞ：70～79 */
		5, 5, 0, 5, 5, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Sコードエラー
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 1, 0, 1, 0, 0, 0, 1, 1,  	/* ｺｰﾄﾞ：10～19 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  	/* ｺｰﾄﾞ：80～89 */
		0, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
	{	// aコードエラー
		0,98, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// cコードエラー
// MH810105 GG119202(S) 決済エラーコードテーブル見直し
//// MH810103 MHUT40XX(S) Edy・WAON対応
////		0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
//		0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 	/* ｺｰﾄﾞ： 0～ 9 */
//// MH810103 MHUT40XX(E) Edy・WAON対応
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 	/* ｺｰﾄﾞ： 0～ 9 */
// MH810105 GG119202(E) 決済エラーコードテーブル見直し
// MH810105 GG119202(S) 決済エラーコードテーブル見直し
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
// MH810105(S) MH364301 PiTaPa対応
//	   98, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
	   98, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
// MH810105(E) MH364301 PiTaPa対応
// MH810105 GG119202(E) 決済エラーコードテーブル見直し
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// dコードエラー
// MH810105 GG119202(S) 決済エラーコードテーブル見直し
//// MH810103 MHUT40XX(S) Edy・WAON対応
////		0, 6, 5, 0,98, 3, 0, 3, 3, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
//		0, 6, 5, 5,98, 3, 3, 3, 3, 3, 	/* ｺｰﾄﾞ： 0～ 9 */
//// MH810103 MHUT40XX(E) Edy・WAON対応
		0, 6, 5, 5,98, 3, 3, 0, 0, 3, 	/* ｺｰﾄﾞ： 0～ 9 */
// MH810105 GG119202(E) 決済エラーコードテーブル見直し
// MH810105(S) MH364301 PiTaPa対応
//// MH810104(S) nanaco・iD・QUICPay対応2
////// MH810103(s) 電子マネー対応 #5458 一部の詳細エラーコード受信時に仕様書の記載と異なるエラーが発生する
//////		0, 0, 0, 3, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
////		0, 0, 0, 3, 5, 98, 98, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
////// MH810103(s) 電子マネー対応 #5458 一部の詳細エラーコード受信時に仕様書の記載と異なるエラーが発生する
//		0, 0, 3, 3, 5, 98, 98, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
//// MH810104(E) nanaco・iD・QUICPay対応2
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 3, 3, 5, 98, 98, 0, 0, 1,  	/* ｺｰﾄﾞ：10～19 */
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
// MH810105(E) MH364301 PiTaPa対応
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// eコードエラー
// MH810105 GG119202(S) iD決済時の案内表示対応
//		0, 0, 3, 3, 4, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 3, 3, 4,94, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
// MH810105 GG119202(E) iD決済時の案内表示対応
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// gコードエラー
		3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// hコードエラー
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) Edy・WAON対応
////		0, 0, 0,98, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
//		0, 0, 3,98, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
//// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810105 GG119202(S) 決済エラーコードテーブル見直し
//		0, 3, 3,98, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
//// MH810104(E) nanaco・iD・QUICPay対応2
		0, 0, 3,98, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
// MH810105 GG119202(E) 決済エラーコードテーブル見直し
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
// MH810103 GG118809_GG118909(S) 未引き去り確認時の未了理由に対応する
//	   98,98, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	   96,98, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
// MH810103 GG118809_GG118909(E) 未引き去り確認時の未了理由に対応する
	},
	{	// iコードエラー
// MH810105(S) MH364301 QRコード決済対応
//		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
// MH810105(E) MH364301 QRコード決済対応
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
// MH810105(S) MH364301 QRコード決済対応
	{	// Pコードエラー（QR）
	   10,10,10, 0, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
	   10,10,10, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
	   11,12,13, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
	   14,10,15, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
	   10, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
	{	// Qコードエラー（QR）
	   10, 0,10,10, 0, 0, 0, 0, 0, 0, 	/* ｺｰﾄﾞ： 0～ 9 */
	   10,10,10, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：10～19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：20～29 */
		0, 0, 0,10,16,16, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：30～39 */
	   16, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：40～49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：50～59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：60～69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：70～79 */
	   10, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：80～89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ｺｰﾄﾞ：90～99 */
	},
// MH810105(E) MH364301 QRコード決済対応
};

// エラーコード文字 下記のEcTimeoutErrTblと一致
enum {
	NG308,
// MH810103 GG118809_GG118909(S) 未引き去り確認時の未了理由に対応する
//// LH039701_LH040101(S) キャンセルボタン押下時にエラー登録しない
//	NGa01,
//// LH039701_LH040101(E) キャンセルボタン押下時にエラー登録しない
// MH810103 GG118809_GG118909(E) 未引き去り確認時の未了理由に対応する
	NG_ERR_MAX,
};

// 下記テーブル内の詳細エラーを処理中受信前に受信した場合、タイムアウトエラーとして扱う
const uchar EcTimeoutErrTbl[NG_ERR_MAX][3] = {
	{'3', '0', '8'},	// 読み取り待ちタイムアウト
// MH810103 GG118809_GG118909(S) 未引き去り確認時の未了理由に対応する
	// 未了残高照会でキャンセルボタンを押下したことを記録残すため、
	// テーブルから削除する
//// LH039701_LH040101(S) キャンセルボタン押下時にエラー登録しない
//	{'a', '0', '1'},	// 読み取りキャンセル
//// LH039701_LH040101(E) キャンセルボタン押下時にエラー登録しない
// MH810103 GG118809_GG118909(E) 未引き去り確認時の未了理由に対応する
};

// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
// 取引内容＝未了確定(失敗)となる詳細エラーコード
static const uchar EcMiryoFailErrTbl[3] = {
	'h', '9', '0',		// 引き去りがされていない
};
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する

// MH810105 GG119202(S) 電子マネー決済エラー発生時の画面戻り仕様変更
static const uchar EcContinueReadingTbl[3] = {
	'd', '0', '2',		// 複数枚カード検出
};
// MH810105 GG119202(E) 電子マネー決済エラー発生時の画面戻り仕様変更

// MH810105(S) MH364301 QRコード決済対応
// エラーコード文字 下記のQrContinueReadingTblと一致
enum {
	NG306,
	NGP20,
	NGP21,
	NG_QR_ERR_MAX,
};
static const uchar QrContinueReadingTbl[NG_QR_ERR_MAX][3] = {
	{'3', '0', '6',},		// 読み取り失敗
	{'P', '2', '0',},		// 期限切れコード
	{'P', '2', '1',},		// コードエラー
};
// MH810105(E) MH364301 QRコード決済対応

static uchar Card_Work[20];
static EC_SETTLEMENT_RES alm_rct_info;

// MH810105(S) MH364301 QRコード決済対応
const uchar *sub_brand_name_list_QR[QR_PAY_KIND_MAX] = {
	"ｺｰﾄﾞ決済",			// [00]
	"Alipay",			// [01]
	"d払い",			// [02]
	"LINE Pay",			// [03]
	"PayPay",			// [04]
	"楽天ペイ",			// [05]
	"WeChat Pay",		// [06]
	"銀聯 QR コード",	// [07]
	"au PAY",			// [08]
	"メルペイ",			// [09]
	"Origami Pay",		// [10]
	"銀行 Pay",			// [11]
	"QUO カード Pay",	// [12]
	"VIA-Dash",			// [13]
	"VIA-Global Pay",	// [14]
	"Smart Code",		// [15]
	"J-Coin Pay",		// [16]
	"Amazon Pay",		// [17]
	"JKO Pay",			// [18]
	"GLN Pay",			// [19]
	"Bank Pay",			// [20]
	"FamiPay",			// [21]
	"atone",			// [22]
};
// MH810105(E) MH364301 QRコード決済対応

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/
//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data que Regist
//[]----------------------------------------------------------------------[]
///	@return			0
//[]----------------------------------------------------------------------[]
///	@note			Suica_Snd_regist()を参考(中身を分割する)
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	Ec_Snd_Que_Regist(uchar kind, uchar *snddata)
{
	uchar	count_status, Read_pt;

	count_status = 1;															// ﾊﾞｯﾌｧｽﾃｰﾀｽｾｯﾄ
	for (Read_pt = 0; Read_pt < 5; Read_pt++) {									// 未送信ﾃﾞｰﾀの検索
		if (!Suica_Snd_Buf.Suica_Snd_q[Read_pt].snd_kind) {						// 未送信ﾃﾞｰﾀがあれば
			count_status = 0;													// ﾊﾞｯﾌｧｽﾃｰﾀｽを更新
			break;
		}
	}
	if (count_status != 0) {													// ﾊﾞｯﾌｧがﾌﾙの場合
		for (Read_pt = 0; Read_pt < BUF_MAX_DELAY_COUNT; Read_pt++) {			// 未送信ﾃﾞｰﾀの検索
			xPause(BUF_MAX_DELAY_TIME);											// ﾊﾞｯﾌｧMAX時の送信完了待ち
			if (!Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind) {	// 未送信ﾃﾞｰﾀがあれば送信ﾃﾞｰﾀの登録実行
				break;
			}
		}
	}

	if (Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind != 0) {		// ﾃﾞｰﾀがすでに格納済みの場合
		if (Suica_Snd_Buf.read_wpt > 3) {										// ﾘｰﾄﾞﾎﾟｲﾝﾀがMAX以上？
			Suica_Snd_Buf.read_wpt = 0;											// ｷｭｰの先頭を指定
		} else {
			Suica_Snd_Buf.read_wpt++;											// ﾘｰﾄﾞﾎﾟｲﾝﾀをｶｳﾝﾄUP
		}
	}

	Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind = kind;
	memcpy(Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].Snd_Buf, snddata, (size_t)S_BUF_MAXSIZE);

	if (Suica_Snd_Buf.write_wpt > 3) {
		Suica_Snd_Buf.write_wpt = 0;
	} else {
		Suica_Snd_Buf.write_wpt++;
	}

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data que read
//[]----------------------------------------------------------------------[]
///	@return			送信データ長
//[]----------------------------------------------------------------------[]
///	@note			Suica_Snd_regist()を参考(中身を分割する)
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	Ec_Snd_Que_Read(uchar *kind, uchar *snddata)
{
	short	wpkind;

	wpkind = 0;
	if (Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind != 0) {
		memcpy(snddata, Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].Snd_Buf, (size_t)S_BUF_MAXSIZE);
		*kind = Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind;
		switch (*kind) {
		case S_CNTL_DATA: 									/* 制御ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_CTRL_DATA;
			break;
		case S_SELECT_DATA:									/* 選択商品ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_SLCT_DATA;
			break;
		case S_INDIVIDUAL_DATA:								/* 個別ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_INDI_DATA;
			break;
		case S_BRAND_SET_DATA:								/* ブランド設定ﾃﾞｰﾀの送信要求 */
		case S_BRAND_SET_DATA2:								/* ブランド設定ﾃﾞｰﾀ2の送信要求 */
			wpkind = EC_CMD_LEN_BSET_DATA;
			break;
		case S_VOLUME_CHG_DATA:								/* 音量変更ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_VSET_DATA;
			break;
		case S_READER_MNT_DATA:								/* リーダメンテナンスﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_MENT_DATA;
			break;
		case S_BRAND_SEL_DATA:								/* ブランド選択ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_BSEL_DATA;
			break;
// MH810103 GG119202(S) ブランド選択結果応答データ
		case S_BRAND_RES_RSLT_DATA:							/* ブランド選択結果応答ﾃﾞｰﾀの送信要求 */
			wpkind = EC_CMD_LEN_BRRT_DATA;
			break;
// MH810103 GG119202(E) ブランド選択結果応答データ
		default:											/* ｺﾏﾝﾄﾞ送信要求なし */
			break;
		}
	}
	return wpkind;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data Set
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			Suica_Data_Snd()を参考
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ec_Data_Snd(uchar kind, void *snddata)
{
	int		i, j;
	uchar	*p;
	long	tmp = 0L;
	ulong	paydata = 0L;
	EC_BRAND_TBL		*brand_tbl;
	EC_VOLUME_TBL		*volume_tbl;
	EC_MNT_TBL			*mnt_tbl;
	EC_BRAND_SEL_TBL	*brandsel_tbl;
	uchar	work_buf[3];
// MH810103 GG119202(S) ブランド選択結果応答データ
	EC_BRAND_RES_RSLT_TBL	*brandresrslt_tbl;
// MH810103 GG119202(E) ブランド選択結果応答データ

	/* 決済リーダ未使用？ */
	if (isEC_USE() == 0) {
	// 決済リーダ接続無し
		return;
	}
	if (Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR != 0) {
	// 受信した決済額と受信した選択商品データとで差異があった
		return;			// 以降すべての電文送信せずに抜ける
	}
	if (OPECTL.Ope_mod == 13) {
	/* ﾌｪｰｽﾞが修正精算だった場合電文送信しない */
		return;
	}

	memset(work_buf, 0, sizeof(work_buf));
	memset(suica_work_buf, 0, sizeof(suica_work_buf));
	Suica_Rec.snd_kind = kind;	/* 送信種別のｾｯﾄ */

	switch (kind) {
	case S_CNTL_DATA:
	/* 制御ﾃﾞｰﾀ作成 */
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応(コメントのみ)
		// 制御データを送信する場合、必ずEc_Pri_Data_Snd()経由で当該関数を起動すること
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応(コメントのみ)
		memcpy(&suica_work_buf, snddata, sizeof(suica_work_buf[0]));
		if (Suica_Rec.Data.BIT.INITIALIZE == 0) {			/* 初期化終了？ */
			return;
		}
		if (suica_work_buf[0] == 0x80 && Suica_Rec.Data.BIT.SEND_CTRL80 != 0) {	// 送信対象が取引終了(0x80)で送信可能状態でない場合
			return;																// 送信しないで抜ける
		}
		if (suica_work_buf[0] == 0x80) {
			Suica_Rec.Data.BIT.SEND_CTRL80 = 1;									// 取引終了送信済みフラグセット
		}
Suica_Data_Snd_10:
		if ((suica_work_buf[0] & 0x01 ) == 0) {
		// 受付禁止/取引終了
			// 「カード処理中」は受付禁止を送信しない
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			if(Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
			if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				return;
			}
// MH810103 GG119202(S) マルチブランド仕様変更対応
//// GG119200(S) ブランド選択待ち中に入金で金額変更ビットを立てない
//			if(( suica_work_buf[0] & S_CNTL_PRICE_CHANGE_BIT ) &&	// 金額変更ビットONかつ
//			   ( !Suica_Rec.Data.BIT.ADJUSTOR_NOW )){				// 選択商品データ未送信？
//				suica_work_buf[0] &= ~S_CNTL_PRICE_CHANGE_BIT;		// 金額変更ビットをOFFして受付禁止を送信する
//			}
//// GG119200(E) ブランド選択待ち中に入金で金額変更ビットを立てない
// MH810103 GG119202(E) マルチブランド仕様変更対応
			Suica_Rec.Data.BIT.OPE_CTRL = 0;
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//			if (suica_work_buf[0] == 0 || suica_work_buf[0] == S_CNTL_PRICE_CHANGE_BIT) {
//			// 受付禁止/金額変更
			if( !( suica_work_buf[0] & S_CNTL_REBOOT_OK_BIT ) ){	// 再起動可bit OFF？
				// 再起動可bitがOFFのもののみ待ち合わせ/再送リトライ対象とする
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
				tmp = EC_DISABLE_WAIT_TIME;
				LagTim500ms(LAG500_SUICA_NONE_TIMER, (short)((tmp * 2) + 1), snd_disable_timeout);	// 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
				LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );					// 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ
				Status_Retry_Count_OK = 0;										/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
				LagCan500ms(LAG500_EC_NOT_READY_TIMER);							// リーダNotReadyタイマー中の受付禁止送信でタイマーキャンセル
				ECCTL.not_ready_timer = 0;
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
				ECCTL.prohibit_snd_bsy = (uchar)( suica_work_buf[0] | 0x01 );	// 応答監視対象指令種別保存(受付許可/禁止bitをONする)
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
			}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
			// 選択商品データ送信中を解除：送信データはキューイングされるため、状態データ受信時まだキューにある可能性があるので、打ち消す送信で解除する
			Suica_Rec.Data.BIT.SELECT_SND = 0;									// 選択商品データ送信中　を解除
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		} else if ((suica_work_buf[0] & 0x01) == 1) {
		// 受付許可
			if ((OPECTL.op_faz == 8)											// 電子媒体停止中か
			||	(OPECTL.InquiryFlg != 0)										// カード処理中か
			||	(OPECTL.ChkPassSyu != 0)										// ｱﾝﾁﾊﾟｽﾁｪｯｸ中か
			||	((Suica_Rec.suica_err_event.BYTE & 0xFB)!= 0)					// 決済リーダエラー発生(開局異常は除く)
			||	(Suica_Rec.Data.BIT.COMM_ERR != 0)								// 決済リーダ通信異常発生
			||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {						// Suica未了タイムアウト
				return;
			}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			if (OPECTL.Ope_mod == 2 && 
//				(!WAR_MONEY_CHECK && !isEcEnabled(EC_CHECK_CREDIT)) ) {
//			// 精算中に現金が投入され、クレジット決済不可の場合は受付許可を送信しない
//				return;															// 電文送信せずに抜ける
//			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			Suica_Rec.Data.BIT.OPE_CTRL = 1;
			LagCan500ms(LAG500_SUICA_NONE_TIMER);								// 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
		}
		Ope_Suica_Status = 1;
		break;

	case S_SELECT_DATA:
	/* 選択商品ﾃﾞｰﾀ作成 */
		if ((OPECTL.op_faz == 8)								// 電子媒体停止中か
		||	(OPECTL.ChkPassSyu != 0)							// ｱﾝﾁﾊﾟｽﾁｪｯｸ中か
		||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {			// リーダ未了タイムアウト
		// Ec停止条件の場合は、選択商品データを送信せずに停止処理を実行する
			Suica_Rec.snd_kind = S_CNTL_DATA;
			memset(suica_work_buf, 0, sizeof(suica_work_buf));
			goto Suica_Data_Snd_10;
		}
		else if (!Suica_Rec.Data.BIT.ADJUSTOR_START) {
		// 選択商品データ送信不可状態
			return;												// 送信せずに抜ける
		}
		else if (OPECTL.InquiryFlg != 0 ||
				 Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// カード処理中、または、未了中は選択商品データを送信しない
			return;
		}
		else if (ECCTL.phase != EC_PHASE_PAY) {
		// 精算可以外
			return;
		}
		else if (Suica_Rec.suica_err_event.BYTE != 0) {
		// 決済リーダエラー発生中(E3201～06,09,59,62～64)
			return;
		}
		else if( Suica_Rec.Data.BIT.COMM_ERR ) {
			// 決済リーダ通信異常
			return;
		}
// MH810103 GG119202(S) 再度、選択商品データを送信することがある
		else if(Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {
			// 取引キャンセル、または、3データ(状態、決済状態、決済結果)受信で
			// Suica_Rec.Data.BIT.ADJUSTOR_NOWはクリアされる
			return;
		}
// MH810103 GG119202(E) 再度、選択商品データを送信することがある
// MH810103 GG119202(S) 電子マネー決済時はジャーナルプリンタ接続設定を参照する
		else if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) &&
				Suica_Rec.Data.BIT.PRI_NG != 0) {
			// 電子マネーブランドの場合はジャーナルプリンタ使用不可時は
			// 選択商品データを送信しない
			return;
		}
// MH810103 GG119202(E) 電子マネー決済時はジャーナルプリンタ接続設定を参照する

		tmp = 10000;
		suica_work_buf[0] = 0x01;
		memcpy(&paydata, snddata, sizeof(paydata));
		// いったんワークに価格にセット
		work_buf[2] = binbcd((uchar)(paydata / tmp));
		paydata %= tmp;
		tmp /= 100;
		work_buf[1] = binbcd((uchar)(paydata / tmp));
		paydata %= tmp;
		work_buf[0] = binbcd((uchar)paydata);

		p = &suica_work_buf[4];
		for(i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for (j = 0; j < EC_BRAND_MAX; j++) {
				if(RecvBrandTbl[i].num <= j) {
					p += 3;
					continue;
				}
				// ブランドを選択している場合は選択したブランドのみ価格をセットする
// MH810103 GG119202(S) 開局・有効条件変更
//				if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01 && 			// 開局しているブランドで
//					((RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// ブランド選択済み、かつ、選択したブランド
				// ブランド選択条件にブランド利用可否を見ているため、ここでは見ない
				if ( ((RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// 選択したブランド
// MH810103 GG119202(E) 開局・有効条件変更
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//					 RecvBrandTbl[i].ctrl[j].no == RecvBrandResTbl.no) ||
//					 RecvBrandResTbl.res == EC_BRAND_UNSELECTED) ) {			// または、ブランドを選択していない
					 RecvBrandTbl[i].ctrl[j].no == RecvBrandResTbl.no)) ) {
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
				// 有効
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//					if( (WAR_MONEY_CHECK && !e_incnt) ||
//						RecvBrandTbl[i].ctrl[j].no == BRANDNO_CREDIT) {
//						// 入金されていない、または、クレジット
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
						*p++ = work_buf[0];
						*p++ = work_buf[1];
						*p++ = work_buf[2];
						// 送信するブランド番号を保持
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						Product_Select_Brand = RecvBrandTbl[i].ctrl[j].no;
						ECCTL.Product_Select_Brand = RecvBrandTbl[i].ctrl[j].no;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//					} else {
//						// 入金されている場合は電子マネーは無効
//						p += 3;
//					}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
				} else {
				// 無効
					p += 3;
				}
			}
		}
		// 精算機の現在時刻をセット
		time_set_snd(p, &CLK_REC);

		Product_Select_Data = *((long*)snddata);
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		Suica_Rec.Data.BIT.SELECT_SND = 1;					// 選択商品データ送信中
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		break;

	case S_INDIVIDUAL_DATA:
	/* 個別ﾃﾞｰﾀ作成 */
		suica_work_buf[0] = 0x00;							// メーカコード
		suica_work_buf[1] = 0x62;							// 種別(98：精算機をセットする)
		suica_work_buf[2] = 0x00;							// ソフトバージョン
		time_set_snd(&suica_work_buf[3], snddata);			// 現在時刻(精算機の時刻をリーダにセットする)

		suica_work_buf[11] = 0xFE;							// 商品先選択タイマ(254秒固定)
// MH810103 MHUT40XX(S) Edy・WAON対応
//		suica_work_buf[22] = OPE_EC_MIRYO_TIME;				// 決済リトライタイマ値(255秒固定)
		suica_work_buf[22] = 0;								// 決済リトライタイマ値(0固定)
// MH810103 MHUT40XX(E) Edy・WAON対応
	// ※ブランド数を99にすることで全ブランド情報の受信が可能となる
		suica_work_buf[23] = 0x99;							// ブランド数
// MH810103 GG119202(S) 個別データフォーマット変更
		memcpy( &suica_work_buf[24], VERSNO.ver_part, 8 );	// ソフトウェアバージョン
// MH810103 GG119202(E) 個別データフォーマット変更
		break;

	case S_BRAND_SET_DATA:
	case S_BRAND_SET_DATA2:
	/* ブランド設定ﾃﾞｰﾀ作成 */
		brand_tbl = (EC_BRAND_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(ECCTL.brand_num);									// ブランド数
		for (i = 0; i < brand_tbl->num; i++) {
			*p++ = binbcd((uchar)(brand_tbl->ctrl[i].no % 100));		// ブランド番号
			*p++ = binbcd((uchar)(brand_tbl->ctrl[i].no / 100));
			*p++ = brand_tbl->ctrl[i].status;							// ブランド設定
			memcpy(p, brand_tbl->ctrl[i].name, EC_BRAND_NAME_LEN);		// ブランド名称
			p += EC_BRAND_NAME_LEN;
		}
		break;

	case S_VOLUME_CHG_DATA:
	/* 音量変更ﾃﾞｰﾀ作成 */
		volume_tbl = (EC_VOLUME_TBL*)snddata;
		p = &suica_work_buf[0];
		if( volume_tbl->num > EC_VOL_MAX ) {
			volume_tbl->num = EC_VOL_MAX;
		}
		*p++ = binbcd(volume_tbl->num);									// 切替パターン数
		i = 0;
		do {
			// 切換なし(パターン数=0)でも、パターン1にはデータをセットする(音量をセットするため)
			*p++ = binbcd((uchar)(volume_tbl->ctrl[i].time / 100));		// 切替時刻(時)
			*p++ = binbcd((uchar)(volume_tbl->ctrl[i].time % 100));		// 切替時刻(分)
			*p++ = binbcd(volume_tbl->ctrl[i].vol);						// 音量
			i++;
		} while( i < volume_tbl->num );
		break;

	case S_READER_MNT_DATA:
	/* リーダメンテナンスﾃﾞｰﾀ作成 */
		mnt_tbl = (EC_MNT_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(mnt_tbl->cmd);									// コマンド
		*p++ = binbcd(mnt_tbl->mode);									// モード
		*p++ = binbcd(mnt_tbl->vol);									// テスト音量
		break;

	case S_BRAND_SEL_DATA:
	/* ブランド選択ﾃﾞｰﾀ作成 */
		if ((OPECTL.op_faz == 8)								// 電子媒体停止中か
		||	(OPECTL.ChkPassSyu != 0)							// ｱﾝﾁﾊﾟｽﾁｪｯｸ中か
		||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {			// リーダ未了タイムアウト
		// Ec停止条件の場合は、選択商品データを送信せずに停止処理を実行する
			Suica_Rec.snd_kind = S_CNTL_DATA;
			memset(suica_work_buf, 0, sizeof(suica_work_buf));
			goto Suica_Data_Snd_10;
		}
		else if (!Suica_Rec.Data.BIT.ADJUSTOR_START) {
		// ブランド選択ﾃﾞｰﾀ送信不可状態
			return;												// 送信せずに抜ける
		}
		else if (OPECTL.InquiryFlg != 0 ||
				 Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// カード処理中、または、未了中はブランド選択データを送信しない
			return;
		}
		else if (ECCTL.phase != EC_PHASE_PAY) {
		// 精算可以外
			return;
		}
		else if (Suica_Rec.suica_err_event.BYTE != 0) {
		// 決済リーダエラー発生中(E3201～06,09,59,62～64)
			return;
		}
		else if( Suica_Rec.Data.BIT.COMM_ERR ) {
			// 決済リーダ通信異常
			return;
		}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応参考）
		else if (Suica_Rec.Data.BIT.BRAND_SEL != 0) {
			// 状態データ（受付不可）受信、または、ブランド選択結果応答データ送信で
			// Suica_Rec.Data.BIT.BRAND_SELはクリアされる
			return;
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応参考）

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		Suica_Rec.Data.BIT.BRAND_CAN = 0;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) ブランド選択中フラグがクリアされない対策
//※ここでBRAND_SEL=1とすることで、選択データ未送信でも、その後の取り消しで、BRAND_CANをセットできるようにする
//（BRAND_CANがONになっていないと、受付不可でBRAND_SELをクリアできないまま次の精算を始めてしまう）
		Suica_Rec.Data.BIT.BRAND_SEL = 1;			// ブランド選択中
// MH810103 GG119202(E) ブランド選択中フラグがクリアされない対策
		brandsel_tbl = (EC_BRAND_SEL_TBL*)snddata;
		p = &suica_work_buf[0];
		if( brandsel_tbl->num > EC_BRAND_TOTAL_MAX ) {
			brandsel_tbl->num = EC_BRAND_TOTAL_MAX;
		}
// MH810103 GG119202(S) ブランド選択データフォーマット変更
		*p++ = binbcd(brandsel_tbl->btn_enable);						// ボタン有無指定
		*p++ = binbcd(brandsel_tbl->btn_type);							// ボタンタイプ
// MH810103 GG119202(E) ブランド選択データフォーマット変更
		*p++ = binbcd(brandsel_tbl->num);								// ブランド数
		for (i = 0; i < brandsel_tbl->num; i++) {
			*p++ = binbcd((uchar)(brandsel_tbl->no[i] % 100));			// ブランド番号
			*p++ = binbcd((uchar)(brandsel_tbl->no[i] / 100));
		}
		break;

// MH810103 GG119202(S) ブランド選択結果応答データ
	case S_BRAND_RES_RSLT_DATA:
		brandresrslt_tbl = (EC_BRAND_RES_RSLT_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(brandresrslt_tbl->res_rslt);
		*p++ = binbcd((uchar)(brandresrslt_tbl->no % 100));			// ブランド番号
		*p++ = binbcd((uchar)(brandresrslt_tbl->no / 100));
		break;
// MH810103 GG119202(E) ブランド選択結果応答データ

	default:
		break;
	}

	Ec_Snd_Que_Regist(Suica_Rec.snd_kind, suica_work_buf);
	CNMTSK_START = 1;												/* タスク起動 */
}

//[]----------------------------------------------------------------------[]
///	@brief			送信データセットの前処理
//[]----------------------------------------------------------------------[]
///	@param[in]		req		: 要求コマンド
///	@param[in]		type	: 送信データ(1byte分)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			Suica_Ctrl()を参考
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ec_Pri_Data_Snd(ushort req, uchar type)
{
	uchar ctrl_work = type;
	uchar i, j;
// MH810103 GG119202(S) E3210登録処理修正
//	short timer;
// MH810103 GG119202(E) E3210登録処理修正
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	ushort no;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	switch (req) {
	case S_CNTL_DATA:
		if (EC_STATUS_DATA.StatusInfo.RebootReq &&
			OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3) {
			// 再起動要求bitONで精算中、精算完了以外であれば
			// 再起動OKbitをONする
			ctrl_work |= S_CNTL_REBOOT_OK_BIT;
// MH810103 GG119202(S) E3210登録処理修正
			// 再起動可送信後は初期化シーケンス完了の監視をしない
//// MH321800(S) E3210登録タイミング変更
////			// 起動完了待ちタイマ
////			timer = (short)prm_get(COM_PRM, S_ECR, 20, 3, 1);
////			if (timer == 0) {
////				timer = 300;
////			} else if(timer < 10){
////				timer = 10;
////			}
//			// 再起動要求から3分(固定)経過しても決済リーダの初期化シーケンスが完了しない場合、
//			// E3210を登録する
//			timer = OPE_EC_WAIT_BOOT_TIME;
//// MH321800(E) E3210登録タイミング変更
//			timer *= 2;
//			LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH810103 GG119202(E) E3210登録処理修正
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
			// 決済リーダバージョンアップ中フラグセット
			EcReaderVerUpFlg = 1;
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応
// MH810103 GG119202(S) 決済リーダからの再起動要求を記録する
			err_chk((char)jvma_setup.mdl, ERR_EC_REBOOT, 1, 0, 0);	// エラー登録
// MH810103 GG119202(E) 決済リーダからの再起動要求を記録する
		// リブート許可送信後ACKを受けた時点で、INITIALIZEをクリアし再初期化
		}

// MH810103 GG119202(S) マルチブランド仕様変更対応
//		if (type != 0x80) {
//			RecvBrandResTbl.res = EC_BRAND_UNSELECTED;		// 制御データを送るときは一旦、選択ブランドなしに戻し
//			RecvBrandResTbl.no = BRANDNO_UNKNOWN;			// 受付可後はブランド選択をやり直す
//// GG119202(S) ブランド選択後のキャンセル処理変更
////			Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;			// ブランド選択の「戻る」検出フラグ初期化
//// GG119202(E) ブランド選択後のキャンセル処理変更
//		}
// MH810103 GG119202(E) マルチブランド仕様変更対応
// MH810103 GG119202(S) 未了仕様変更対応
//		if (((ctrl_work & 0x01) == 0) &&
//			Suica_Rec.Data.BIT.CTRL_MIRYO != 0 &&
//			Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0 &&
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 0) {
//			// 未了確定後の受付禁止送信時は未了残高終了通知bitをONする
//			ctrl_work |= S_CNTL_MIRYO_ZANDAKA_END_BIT;
//			// 未了確定後に未了残高終了通知を送信する際、dsp_changeを2にする
//			dsp_change = 2;
		if (((ctrl_work & 0x01) == 0) && ((ctrl_work & 0x80) != 0x80 ) &&	// 受付禁止、取引終了以外
			(Suica_Rec.Data.BIT.CTRL_MIRYO != 0) &&							// 未了中
			(Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) &&						// 未了確定(状態データ)受信済み
			(Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 0)) {					// 未了タイムアウト前にとりけしボタン押下
			// 未了確定後の受付禁止送信時は未了残高終了通知bitをONする
			ctrl_work |= S_CNTL_MIRYO_ZANDAKA_END_BIT;
// MH810103 GG119202(E) 未了仕様変更対応
		}
		if (type != 1) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
			if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&		// ブランド選択中
				OPECTL.InquiryFlg == 0) {					// 既にカード処理中ではないとき
				Suica_Rec.Data.BIT.BRAND_CAN = 1;			// キャンセルあり
			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			timer_recept_send_busy = 0;
			ECCTL.timer_recept_send_busy = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			Lagcan(OPETCBNO, TIMERNO_EC_RECEPT_SEND);		// 受付許可送信タイマー停止
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
			if( ECCTL.prohibit_snd_bsy != 0 ){				// 制御データ（受付禁止）応答監視中
				// 監視中は制御データ（受付禁止）を送信しない
				if( ctrl_work & S_CNTL_TRANSACT_END_BIT ){	// 取引終了bit ON？
					// 次送信時に取引終了を送信する為、取引終了送信要求あり状態とする
					ECCTL.transact_end_snd = ctrl_work;
				}
				break;
			}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
		}
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
		// 各状態を初期化して送信を行う
		ECCTL.prohibit_snd_bsy = 0;							// 制御データ（受付禁止）応答監視なし状態とする
		ECCTL.transact_end_snd = 0;							// 取引終了送信要求なし状態とする
		LagCan500ms(LAG500_SUICA_NONE_TIMER);				// 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		// ブランド選択中の金額変更は受付禁止にして送信する
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW == 0) {
			if (ctrl_work == S_CNTL_PRICE_CHANGE_BIT) {
				ctrl_work = 0;
			}
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		Ec_Data_Snd(S_CNTL_DATA, &ctrl_work);				// 制御ﾃﾞｰﾀ可送信
		break;
	case S_SELECT_DATA:
		if (e_incnt <= 0) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//// GG119202(S) クレジットカード精算限度額設定対応
//			if( ENABLE_MULTISETTLE() == 1 ){				// 単一決済有効
//				ec_set_brand_selected();					// 有効ブランドを選択済みとする
//			}
//// GG119202(E) クレジットカード精算限度額設定対応
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			w_settlement = ryo_buf.zankin;
			Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
		}
		break;
	case S_VOLUME_CHG_DATA:
		Ec_Data_Snd(S_VOLUME_CHG_DATA, &VolumeTbl);			// 音量変更ﾃﾞｰﾀ送信
		break;
	case S_READER_MNT_DATA:
		Ec_Data_Snd(S_READER_MNT_DATA, &MntTbl);			// リーダメンテナンスﾃﾞｰﾀ送信
		break;
	case S_BRAND_SEL_DATA:
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;				// 「戻る」初期化
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
		memset(&BrandSelTbl, 0x00, sizeof(BrandSelTbl));
// MH810103 GG119202(S) ブランド選択データフォーマット変更
		// 決済リーダにブランド選択ボタンを表示する
		BrandSelTbl.btn_enable = 1;							// ボタン有
		// 決済リーダに「はい」、「いいえ」の2択ボタンを表示しない
		BrandSelTbl.btn_type = 0;							// 2択表示なし
// MH810103 GG119202(E) ブランド選択データフォーマット変更
		for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for (j = 0; j < EC_BRAND_MAX; j++) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//// GG119202(S) クレジットカード精算限度額設定対応
////				if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01) {
//				if( ec_check_valid_brand( RecvBrandTbl[i].ctrl[j].no ) ){	// ブランド有効？
//// GG119202(E) クレジットカード精算限度額設定対応
				if( IsValidBrand( i, j ) ){					// ブランド有効？
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
					BrandSelTbl.no[BrandSelTbl.num++] = RecvBrandTbl[i].ctrl[j].no;
				}
			}
		}
		Ec_Data_Snd(S_BRAND_SEL_DATA, &BrandSelTbl);		// ブランド選択ﾃﾞｰﾀ送信
		break;
// MH810103 GG119202(S) ブランド選択結果応答データ
	case S_BRAND_RES_RSLT_DATA:
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	case S_BRAND_RES_RSLT_DATA_with_HOLD:
		// ブランド選択結果応答でNGを送信する場合は、予め BrandResRsltTbl.noに「理由コード」をセットして
		// type=0で呼び出すこと
		no = BrandResRsltTbl.no;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		memset(&BrandResRsltTbl, 0x00, sizeof(BrandResRsltTbl));
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		BrandResRsltTbl.res_rslt = RecvBrandResTbl.res;
//		BrandResRsltTbl.no = RecvBrandResTbl.no;
		// type : ブランド選択結果 1:OK, 0:NG
		BrandResRsltTbl.res_rslt = type;
		if(type == 0) {	// NG
			BrandResRsltTbl.no = no;
			if (no == EC_BRAND_REASON_99) {					// ブランド選択データの処理ができない。
				ECCTL.brandsel_stop = 1;					// ブランド選択処理停止
			}
		} else {
			BrandResRsltTbl.no = RecvBrandResTbl.no;
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		Ec_Data_Snd(S_BRAND_RES_RSLT_DATA, &BrandResRsltTbl);		// ブランド選択結果応答ﾃﾞｰﾀ送信
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		Suica_Rec.Data.BIT.BRAND_SEL = 0;					// ブランド選択中を解除
		if (req == S_BRAND_RES_RSLT_DATA) {
			EcBrandClear();									// ブランド選択結果クリア
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		break;
// MH810103 GG119202(E) ブランド選択結果応答データ
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_Ec_Event
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			:getmassegeにて取得したID 
///					ope_faze	:ｺｰﾙ元のﾌｪｰｽ
///	@return			ret			:精算終了判定
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short Ope_Ec_Event(ushort msg, uchar ope_faze)
{
	short	ret = 0;
	uchar	i;

	for ( ; suica_rcv_que.count != 0; ) {							// 受信済みのデータを全て処理するまで回す
		if (Ec_Read_RcvQue() == 1) {								// 受信待ち合わせ中の場合は
			continue;												// 後続のデータ受信させる為、解析処理はさせない
		}

		for (i = 0; i < TBL_CNT(EcTbl); i++) {						// 受信データテーブル検索実行
			if (EcTbl[i].Command == (Suica_Rec.suica_rcv_event.BYTE & EcTbl[i].Command)) {	// 受信したデータが処理可能な電文の場合
				if ((ret = EcTbl[i].Func(&ope_faze)) != 0) {		// 電文毎の解析処理を実行
					break;
				}
			}
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			受信キューから電文をReadする
//[]----------------------------------------------------------------------[]
///	@return			ret 0:ﾃﾞｰﾀ種別毎の解析処理実施 1:受信ﾃﾞｰﾀ待ち合わせ
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
uchar	Ec_Read_RcvQue(void)
{
	uchar	*pData = (uchar*)&suica_rcv_que.rcvdata[suica_rcv_que.readpt];	// 受信ｷｭｰのﾎﾟｲﾝﾀをﾜｰｸ領域設定
	uchar	i, bc;
	uchar	ret = 1;
	ushort	cnt;
	ushort	w_counter[3];

	memcpy(w_counter, &suica_rcv_que.readpt, sizeof(w_counter));			// ﾎﾟｲﾝﾀ処理領域の初期化

	for (i = 0; (i < 6 && *pData != ack_tbl[i]); i++) {						// 受信種別(ACK1～NACK)検索
		;
	}
	if (i == 1) {															// ACK2 データ受信 ?
		pData += 2;															// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
		ec_data_save(pData, (short)*(pData-1)); 							// 受信ﾃﾞｰﾀの解析処理
		if (ec_split_data_check() == 0) {									// ﾃﾞｰﾀ待ち合わせ判定処理
			ret = 0;														// 種別毎の解析処理実施
		}
	} else if ((i == 2) || (i == 0)) {										// ACK3 or ACK1 データ受信?
		bc = *(pData+1);													// 受信ﾃﾞｰﾀ数を保持
		pData += 2;															// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
		for (cnt = 0; cnt < bc; ) {											// 全ての受信ﾃﾞｰﾀの解析が完了するまでﾙｰﾌﾟ
			cnt += (*pData + 1);											// ﾃﾞｰﾀ種別毎のﾃﾞｰﾀｻｲｽﾞを加算
			pData++;														// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新
			ec_data_save(pData,(short)*(pData-1)); 							// 受信ﾃﾞｰﾀの解析処理
			pData += *(pData-1);											// ﾃﾞｰﾀﾎﾟｲﾝﾀの更新(次のﾃﾞｰﾀまで進める)
		}
		if (ec_split_data_check() == 0) {									// ﾃﾞｰﾀ待ち合わせ判定処理
			ret = 0;														// 種別毎の解析処理実施
		}
	}

	if (++w_counter[0] >= SUICA_QUE_MAX_COUNT) {							// ﾘｰﾄﾞﾎﾟｲﾝﾀがMAXに到達
		w_counter[0] = 0;													// ﾘｰﾄﾞﾎﾟｲﾝﾀを先頭に移動
	}
	if (w_counter[2] != 0) {												// 受信ﾃﾞｰﾀｶｳﾝﾄ数が０でない場合
		w_counter[2]--;														// ﾃﾞｸﾘﾒﾝﾄ実施
	}

	nmisave(&suica_rcv_que.readpt, w_counter, sizeof(w_counter));			// ﾎﾟｲﾝﾀの更新を停電保障で行う
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			受信ﾃﾞｰﾀをｾｰﾌﾞし、ﾒｯｾｰｼﾞｷｭｰを登録する
//[]----------------------------------------------------------------------[]
///	@param[in]		*buf 	：受信キューから取り出したデータ格納領域
///					siz		：受信データサイズ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	ec_data_save(unsigned char *buf, short siz)
{
	int				i;
	short			brandno;
	unsigned long	work, work_brand;
	unsigned char	*p;
	unsigned char	*pt;
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応(未使用変数削除)
//	unsigned char	status_timer;
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応(未使用変数削除)
	unsigned char	pay_ng;
	unsigned char	brand_index;
	unsigned char	kind;
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	T_FrmReceipt	DeemedData;		// 決済精算中止(復決済)印字要求用データ
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810103 GG119202(S) 電子マネー決済時の決済結果受信待ちタイマ追加
//	unsigned short	ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
//
//	if(ec_inquiry_wait_time == 0) {
//		ec_inquiry_wait_time = 360;
//	} else if(ec_inquiry_wait_time < 240){
//		ec_inquiry_wait_time = 240;
//	}
//	ec_inquiry_wait_time *= 50;
	unsigned short	ec_inquiry_wait_time = 0;
// MH810103 GG119202(E) 電子マネー決済時の決済結果受信待ちタイマ追加

	pt = buf + 1;	/* ﾃﾞｰﾀ部の先頭ｱﾄﾞﾚｽ(種別の次) */
	memset(wrcvbuf,0,sizeof(wrcvbuf));
	memcpy(wrcvbuf, pt, (size_t)siz);
// GG116202(S) JVMAリセット回数制限対応
	if (!hasEC_JVMA_RESET_CNT_EXCEEDED) {							// JVMAリセットリトライ回数以下
// GG116202(E) JVMAリセット回数制限対応
// MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
	// 通信不良フラグをクリアする
	Suica_Rec.suica_err_event.BIT.COMFAIL = 0;
// MH321800(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
// GG116202(S) JVMAリセット回数制限対応
	}
// GG116202(E) JVMAリセット回数制限対応

	switch (*buf) {													/* 受信ﾃﾞｰﾀ判定 */
	case S_ERR_DATA: /* 異常データ */
		err_ec_chk(wrcvbuf, &err_data);								/* ｴﾗｰﾁｪｯｸ */

		err_data = wrcvbuf[0];
// MH810103 GG119202(S) 不要処理削除
//		work = EC_STATUS_TIME;										/* CRW-MJA状態監視ﾀｲﾏ値取得 */
//		if (err_data != 0) {										/* 異常発生？ */
//			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* 異常ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
//
//			if (work) {												/* CRW-MJA状態監視ﾀｲﾏ値が正常値？ */
//				LagTim500ms(LAG500_SUICA_STATUS_TIMER, (short)(120*work), status_err_timeout);	// CRW-MJA状態監視ﾀｲﾏ開始
//			}
//		} else {													/* ｴﾗｰ解除 */
//			LagCan500ms(LAG500_SUICA_STATUS_TIMER);					/* CRW-MJA状態監視ﾀｲﾏﾘｾｯﾄ */
//		}
		if (err_data != 0) {										/* 異常発生？ */
			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* 異常ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
// MH810103 GG119202(S) JVMAリセット処理不具合
			jvma_trb(ERR_SUICA_RECEIVE);
// MH810103 GG119202(E) JVMAリセット処理不具合
		}
// MH810103 GG119202(E) 不要処理削除
// MH810103 GG119202(S) 異常データフォーマット変更
		// 2バイト目のビットはJVMAリセット対象外とする
		err_ec_chk2(&wrcvbuf[1], &err_data2);						// エラーチェック
		err_data2 = wrcvbuf[1];
		if (err_data2 != 0) {
			// ERR_RECEIVE2フラグは決済リーダ利用不可となるが、JVMAリセットはしない
			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE2 = 1;
		}
// MH810103 GG119202(E) 異常データフォーマット変更
		break;

	case S_FIX_DATA: /* 固定データ */
		memcpy(fix_data,wrcvbuf, sizeof(fix_data));					/* 受信ﾃﾞｰﾀを画面表示用に保持 */
		break;

	case S_BRAND_STS_DATA:	/* ブランド状態データ1	*/
	case S_BRAND_STS_DATA2:	/* ブランド状態データ2	*/
// MH810103 GG119202(S) 起動シーケンス不具合修正
//		if(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
//			// 精算中のブランド状態データは無視する
//			break;
//		}
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
//		if ( Suica_Rec.Data.BIT.BRAND_STS_RCV == 0 ) {				/* 起動後、一度もﾌﾞﾗﾝﾄﾞ状態ﾃﾞｰﾀ受信してない ？*/
//			Suica_Rec.Data.BIT.BRAND_STS_RCV = 1;					/* ﾌﾞﾗﾝﾄﾞ状態ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
//		}
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
		brand_index = 0;
		kind = S_BRAND_SET_DATA;
		if(*buf == S_BRAND_STS_DATA2) {
			brand_index = 1;
			kind = S_BRAND_SET_DATA2;
		}
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
//		memset(&RecvBrandTbl[brand_index], 0, sizeof(EC_BRAND_TBL));		// 受信テーブルクリア
//		p = &wrcvbuf[0];
//		RecvBrandTbl[brand_index].num = bcdbin(*p++);						// ブランド総数
//		ECCTL.brand_num = RecvBrandTbl[brand_index].num;					// ブランド総数を保持
//		if( RecvBrandTbl[brand_index].num > EC_BRAND_MAX ) {
//			if(*buf == S_BRAND_STS_DATA) {
//				RecvBrandTbl[brand_index].num = EC_BRAND_MAX;				// ブランド状態データ１のブランド数
//			} else {
//				RecvBrandTbl[brand_index].num -= EC_BRAND_MAX;				// ブランド状態データ２のブランド数
//			}
//		}
//		for (i = 0; i < RecvBrandTbl[brand_index].num; i++) {
//			RecvBrandTbl[brand_index].ctrl[i].no		= bcdbin3(p);		// ブランド番号
//			p += 2;
//			RecvBrandTbl[brand_index].ctrl[i].status	= *p++;				// ブランド状態
//			memcpy(RecvBrandTbl[brand_index].ctrl[i].name, p, 20);			// ブランド名称
//			p += 20;
//		}
		memset(&RecvBrandTblTmp, 0, sizeof(RecvBrandTblTmp));				// 受信テーブルクリア
		p = &wrcvbuf[0];
		RecvBrandTblTmp[brand_index].num = bcdbin(*p++);					// ブランド総数
		ECCTL.brand_num = RecvBrandTblTmp[brand_index].num;					// ブランド総数を保持
		if( RecvBrandTblTmp[brand_index].num > EC_BRAND_MAX ) {
			if(*buf == S_BRAND_STS_DATA) {
				RecvBrandTblTmp[brand_index].num = EC_BRAND_MAX;			// ブランド状態データ１のブランド数
			} else {
				RecvBrandTblTmp[brand_index].num -= EC_BRAND_MAX;			// ブランド状態データ２のブランド数
			}
		}
		for (i = 0; i < RecvBrandTblTmp[brand_index].num; i++) {
			RecvBrandTblTmp[brand_index].ctrl[i].no		= bcdbin3(p);		// ブランド番号
			p += 2;
			RecvBrandTblTmp[brand_index].ctrl[i].status	= *p++;				// ブランド状態
			memcpy(RecvBrandTblTmp[brand_index].ctrl[i].name, p, 20);		// ブランド名称
			p += 20;
		}
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
		queset(OPETCBNO, EC_BRAND_UPDATE, 0, NULL);							// ブランド状態更新を通知

// MH810103 GG119202(S) ブランド状態判定処理変更
//		if (ECCTL.phase == EC_PHASE_PAY) {
//		// 精算中
//			ECCTL.phase = EC_PHASE_BRAND;									// ブランド合わせ中
//			ECCTL.step = 0;
//		}
//		// else {}		// その他のフェーズではフェーズを変えずにブランド設定データのみを送信する
// MH810103 GG119202(E) ブランド状態判定処理変更

		// ブランド制御テーブル更新
// MH810103 GG119202(S) 起動シーケンス不具合修正
//// GG119202(S) ブランドテーブルをバックアップ対象にする
////		set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTbl[brand_index]);
//		set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
//		update_brand_tbl(&RecvBrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
//// GG119202(E) ブランドテーブルをバックアップ対象にする
//		Ec_Data_Snd(kind, &BrandTbl[brand_index]);							// ブランド設定データ送信
		// ブランド合わせ中以外に受信した場合はテーブル更新しない
		if (ECCTL.phase == EC_PHASE_BRAND) {
			set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
			update_brand_tbl(&RecvBrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) ブランドテーブルが消えない不具合
			if (brand_index == 0 && ECCTL.brand_num <= EC_BRAND_MAX) {
				// ブランド数が10以下の場合、
				// ブランド状態データ2は通知されないのでこのタイミングでクリアする
				nmicler(&RecvBrandTbl[1], sizeof(RecvBrandTbl[1]));
			}
// MH810103 GG119202(E) ブランドテーブルが消えない不具合
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
			setEcBrandStsDataRecv( kind );									// ブランド状態データ受信判定情報設定
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
// MH810103 GG119202(S) 起動シーケンス不具合修正
			if (OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3) {
				// 精算中以外の場合、ブランド設定データを送信する
				// 精算中の場合は待機状態に戻ったタイミングで送信する
				Ec_Data_Snd(kind, &BrandTbl[brand_index]);					// ブランド設定データ送信
				if (isEcBrandStsDataRecv()) {
					// ブランドネゴシエーション完了
					if (ECCTL.phase == EC_PHASE_BRAND) {
						Lagcan(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT);
						// ブランド合わせ完了後は音量変更フェーズに移行する
						ECCTL.phase = EC_PHASE_VOL_CHG;						// 音量変更中
						ECCTL.step = 0;

						// 音量変更データ送信
						SetEcVolume();
						Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
						Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// 受信待ちタイマ開始
					}
				}
				else {
					// 次のブランド状態データ受信待ち
					Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);		// 受信待ちタイマ開始
				}
			}
		}
// MH810103 GG119202(E) 起動シーケンス不具合修正
		break;

	case S_TIME_DATA: /* 時間同期データ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA初期化終了？ */
			break;
		}
		// ※決済リーダでは整時処理なし
		break;

	case S_STATUS_DATA: /* 状態データ */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (timer_recept_send_busy == 2) {
//			timer_recept_send_busy = 0;										// 監視終了
		if (ECCTL.timer_recept_send_busy == 2) {
			ECCTL.timer_recept_send_busy = 0;								// 監視終了
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			Lagcan( OPETCBNO, TIMERNO_EC_RECEPT_SEND );
		}
		PRE_EC_STATUS_DATA.status_data = EC_STATUS_DATA.status_data;
		memcpy(&EC_STATUS_DATA, wrcvbuf, sizeof(EC_STATUS_DATA));	/* 受信ﾃﾞｰﾀｽﾃｰﾀｽｾｯﾄ */

// MH810105 GG119202(S) リーダから直取中の状態データ受信した際の新規アラームを設ける
		if( EC_STATUS_DATA.StatusInfo.LastSetteRunning &&					// 今回受信データの直取中bit ON
		    !PRE_EC_STATUS_DATA.StatusInfo.LastSetteRunning ){				// 前回受信データの直取中bit OFF
			// 「決済リーダ利用不可」登録
			alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 1);					// ｱﾗｰﾑﾛｸﾞ登録（登録）
		}
		else if( !EC_STATUS_DATA.StatusInfo.LastSetteRunning &&				// 今回受信データの直取中bit OFF
		    PRE_EC_STATUS_DATA.StatusInfo.LastSetteRunning ){				// 前回受信データの直取中bit ON
			if( ac_flg.ec_recv_deemed_fg == 0 ){							// 直前取引データの処理待ちではない
				// 「決済リーダ利用不可」解除
				alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 0);				// ｱﾗｰﾑﾛｸﾞ登録（解除）
			}
		}
// MH810105 GG119202(E) リーダから直取中の状態データ受信した際の新規アラームを設ける

		if ((PRE_EC_STATUS_DATA.status_data ^ EC_STATUS_DATA.status_data) == 0x2000) {	// 商品先選択タイマー更新依頼bitのみ変化
			// 商品先選択タイマ更新依頼bitのみ変化した場合は無視する
			break;
		}
		if( !Suica_Rec.Data.BIT.MIRYO_CONFIRM && EC_STATUS_DATA.StatusInfo.MiryoZandakaStop != 0 ){
			// 未了確定前に未了残高中止を受信した場合は無視する
			break;
		}
// MH810103 MHUT40XX(S) Edy・WAON対応
		if (EC_STATUS_DATA.StatusInfo.CardProcTimerUpdate != 0) {
			// タイマ更新依頼あり
			EcTimerUpdate();
		}
// MH810103 MHUT40XX(E) Edy・WAON対応

		Suica_Rec.Data.BIT.PRE_CTRL = Suica_Rec.Data.BIT.CTRL;		// 前回受信ﾃﾞｰﾀのﾘｰﾀﾞｰ状態を保持しておく
		Suica_Rec.Data.BIT.CTRL = EC_STATUS_DATA.StatusInfo.ReceptStatus ^ 0x01;	/* 受信ﾃﾞｰﾀの1Byte目を参照し、ﾘｰﾀﾞｰ状態として取得する */

		if(EC_STATUS_DATA.StatusInfo.DetectionCard &&				/* カード差込Bit ON */
			!Suica_Rec.Data.BIT.CTRL_CARD) {						/* カード差込状態OFF */
			Suica_Rec.Data.BIT.CTRL_CARD = 1;						/* カード差込状態をON */
		} else if(!EC_STATUS_DATA.StatusInfo.DetectionCard &&		/* カード差込Bit OFF */
			Suica_Rec.Data.BIT.CTRL_CARD) {							/* カード差込状態ON */
			Suica_Rec.Data.BIT.CTRL_CARD = 0;						/* カード差込状態をOFF */
			queset(OPETCBNO, EC_EVT_CARD_STS_UPDATE, 0, NULL);		// カード抜き取りを通知
		}

		if (EC_STATUS_DATA.StatusInfo.MiryoStatus &&				/* 未了Bitをチェック */
			!Suica_Rec.Data.BIT.CTRL_MIRYO)	{						/* まだ未了が発生していない */

			if(OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod != 2) {
				// メンテナンス中、精算中以外に処理未了を受信
				recv_unexpected_status_data();
				break;
// MH810103 GG119202(S) マルチブランド仕様変更対応
//// GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
////// GG119202(S) クレジットカード精算限度額設定対応
//////			} else if( (ENABLE_MULTISETTLE() == 0 &&				// クレジット決済のみ有効
////			} else if( (ENABLE_MULTISETTLE() == 1 &&				// クレジット決済のみ有効
////// GG119202(E) クレジットカード精算限度額設定対応
////						isEcEnabled(EC_CHECK_CREDIT)) ||			// または
//			} else if( ENABLE_MULTISETTLE() == 1 &&					// 単一決済で
//// GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
//						RecvBrandResTbl.no == BRANDNO_CREDIT) {		// クレジット決済を選択済み
			} else if( !isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){	// 電子マネーブランド以外が選択済み
// MH810103 GG119202(E) マルチブランド仕様変更対応
				// クレジット決済で処理未了を受信
				recv_unexpected_status_data();
				break;
			}
// MH810103 GG119202(S) 未了発生時の画面表示不具合
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
			if (Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0) {
				// 未了タイムアウト後の未了中受信は無視する
				break;
			}
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
			ec_MessageAnaOnOff(0, 0);
// MH810103 GG119202(E) 未了発生時の画面表示不具合
			Suica_Rec.Data.BIT.CTRL_MIRYO = 1;						/* 未了ｽﾃｰﾀｽをｾｯﾄ */
// MH810103 GG119202(S) 未了仕様変更対応
			Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;					// 未了確定(状態データ)受信をクリア
// MH810103 GG119202(E) 未了仕様変更対応
// MH810103 GG119202(S) 未了中に異常データ受信でとりけしボタン押下待ちにならない
			OPECTL.InquiryFlg = 0;									// 外部照会中フラグOFF
// MH810103 GG119202(E) 未了中に異常データ受信でとりけしボタン押下待ちにならない
// MH810104 GG119201(S) 期限切れ定期券の精算で、磁気リーダに定期券が抜き取り待ちの状態でも未了発生すると磁気リーダのシャッターが閉じてしまう
			if ((RD_mod < 10) || (RD_mod > 13)) {					// 抜き取り待ちor移動中以外
// MH810104 GG119201(E) 期限切れ定期券の精算で、磁気リーダに定期券が抜き取り待ちの状態でも未了発生すると磁気リーダのシャッターが閉じてしまう
			read_sht_cls();											/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ */
// MH810104 GG119201(S) 期限切れ定期券の精算で、磁気リーダに定期券が抜き取り待ちの状態でも未了発生すると磁気リーダのシャッターが閉じてしまう
			}
// MH810104 GG119201(E) 期限切れ定期券の精算で、磁気リーダに定期券が抜き取り待ちの状態でも未了発生すると磁気リーダのシャッターが閉じてしまう
			cn_stat( 2, 2 );										/* 入金不可 */
			if( MIFARE_CARD_DoesUse ){								// Mifareが有効な場合
				op_MifareStop_with_LED();							// Mifare無効
			}
// MH810103 MHUT40XX(S) Edy・WAON対応
//			work = OPE_EC_MIRYO_TIME;								/* 決済ﾘﾄﾗｲﾀｲﾏｰ値取得 */
//			LagTim500ms(LAG500_SUICA_MIRYO_RESET_TIMER, (short)(2*work+1), ec_miryo_timeout);	// 未了状態監視ﾀｲﾏ開始
			EcTimerUpdate();										// 未了監視タイマ開始
// MH810103 MHUT40XX(E) Edy・WAON対応
			wmonlg(OPMON_EC_MIRYO_START, NULL, 0);					// モニタ登録
		} else {
			switch ((EC_STATUS_DATA.status_data >> 8) & 0x0003) {	/* 下2Bitの状態をチェック */
			case 0:		/* 受付可 */
				if (!Ope_Suica_Status) {							/* 精算機側からの送信要求なし */
					Suica_Rec.Data.BIT.OPE_CTRL = 1;				/* 最後に精算機側からの送信要求は受付許可とする */
				}
				if (Suica_Rec.Data.BIT.OPE_CTRL) {					/* 最後に精算機側からの送信要求は受付許可？ */
					LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );	/* 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
					LagCan500ms(LAG500_EC_NOT_READY_TIMER);			/* 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
					ECCTL.not_ready_timer = 0;
					if (ECCTL.pon_err_alm_regist == 0) {
					// 起動後最初の受付可受信
						ECCTL.pon_err_alm_regist = 1;				// 以降、アラーム／エラー登録する
					}
					err_chk((char)jvma_setup.mdl, ERR_EC_OPEN, 0, 0, 0); 	/* ｴﾗｰﾛｸﾞ登録（解除）*/
					alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 0);			/* ｱﾗｰﾑﾛｸﾞ登録（解除）*/
					Suica_Rec.suica_err_event.BIT.OPEN = 0;			/* 開局異常ﾌﾗｸﾞを解除 */
					Status_Retry_Count_OK = 0;						/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
// MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
					err_ec_clear();
// MH810103 GG116201(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
				}
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//				else {												/* 最後に精算機側からの送信要求は受付禁止？ */
//					status_timer = EC_DISABLE_WAIT_TIME;
//					LagTim500ms(LAG500_SUICA_NONE_TIMER, (short)((status_timer * 2) + 1), snd_disable_timeout);	// 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始(5s)
//				}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
				break;
			case 0x01:	/* 受付不可 */
				if (!Ope_Suica_Status) {							/* 精算機側からの送信要求なし */
					Suica_Rec.Data.BIT.OPE_CTRL = 0; 				/* 最後に精算機側からの送信要求は受付禁止とする */
				}
				if (!Suica_Rec.Data.BIT.INITIALIZE) {				/* 初期化終了？ */
					break;
				}
				if (!Suica_Rec.Data.BIT.OPE_CTRL) {					/* 最後に精算機側からの送信要求は受付禁止？ */
					LagCan500ms(LAG500_SUICA_NONE_TIMER);			/* 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
					if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {		/* 未了状態から受付不可を受信？ */ 
						if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {		/* 未了タイムアウト後(初期化終了はチェック済みなので省略する)	*/
						// 未了タイムアウト
						// ※snd_no_response_timeout()の処理内容に合わせる
							LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);	/* 未了状態監視ﾀｲﾏﾘｾｯﾄ */
							Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* 未了状態解除 */
// MH810103 GG119202(S) 未了仕様変更対応
//							if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//								// 未了確定受信なしの場合はカードIDを'?'で埋める
//								Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;	// ブランド番号
//								Ec_Settlement_Res.settlement_data = Product_Select_Data;	// 決済額
//								memset(Ec_Settlement_Res.Card_ID, '\?', sizeof(Ec_Settlement_Res.Card_ID));
//							}
//							else {
//								Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;		// 未了確定受信フラグクリア
//							}
							Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// 未了確定(状態データ)受信をクリア
// MH810103 GG119202(E) 未了仕様変更対応
							if (Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {		/* 決済中の取引キャンセル応答が無い場合 */
								EC_STATUS_DATA.StatusInfo.TradeCansel = 1;	/* 取引ｷｬﾝｾﾙﾌﾗｸﾞもON */
							}
						}
					}
					Status_Retry_Count_NG = 0;						/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
				}
				break;
			case 0x02:	/* 取引キャンセル受付 */
			case 0x03:	/* 受付不可＆取引キャンセル受付 */
				LagCan500ms(LAG500_SUICA_NONE_TIMER);				/* 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
// MH810105 GG119202(S) 未了確定の直取データ受信時にR0176を登録する
				// 未了残高照会でとりけしボタン押下した場合に制御データ（受付禁止）に対して
				// 状態データ（受付不可＆取引キャンセル）が応答されることはないので
				// 下記処理を削除する
//				if (Suica_Rec.Data.BIT.CTRL_MIRYO) {				/* 未了状態？ */
//					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* 未了状態解除 */
//					LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);	/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */
//// MH810103 GG119202(S) 未了仕様変更対応
//					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// 未了確定(状態データ)受信をクリア
//					if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {	// 未了タイムアウト後(初期化終了はチェック済みなので省略する)
//						Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 0;		// 未了タイムアウト処理実施済みクリア
//					}
//// MH810103 GG119202(E) 未了仕様変更対応
//					if (!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3) {	/* 未了後の動作は全精算媒体を停止しない場合 かつ精算中止処理中ではない */
//						miryo_timeout_after_proc();
//					}
//					wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// モニタ登録
//				}
// MH810105 GG119202(E) 未了確定の直取データ受信時にR0176を登録する
				Status_Retry_Count_NG = 0;							/* ﾘﾄﾗｲｶｳﾝﾄ数を初期化 */
				break;
			default:
				break;
			}
		}

		if (Suica_Rec.Data.BIT.INITIALIZE) {						/* 初期化終了？ */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* 制御ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
			Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;		/* 制御ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		}
		Ope_Suica_Status = 0;
		break;

	case S_SETT_STS_DATA:	/* 決済状態データ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA初期化終了？ */
			break;
		}
		if(OPECTL.Ope_mod != 2) {
			// 精算中以外の決済状態データは無視する
			break;
		}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		if (!Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
			// 選択商品データを送信していないのに決済状態を受信した場合は無視する
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW == 0 &&					// 選択商品データを送信していない
			Suica_Rec.Data.BIT.BRAND_SEL == 0) {					// ブランド選択をしていない
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			break;
		}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		Ec_Settlement_Sts = wrcvbuf[0];
//		brandno = bcdbin3(&wrcvbuf[1]);
//		switch (Ec_Settlement_Sts) {
		ECCTL.Ec_Settlement_Sts = wrcvbuf[0];
		brandno = bcdbin3(&wrcvbuf[1]);
		switch (ECCTL.Ec_Settlement_Sts) {
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
		case EC_SETT_STS_MIRYO:				/* 未了中 */
			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;							// 決済処理中
// MH810103 GG119202(S) 未了中の停電で決済精算中止（復）を印字しない
//			save_deemed_data(brandno);											// 復電用データ保存
//			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );						// 問合せ(処理中)結果待ちタイマーリセット
//// GG119202(S) 未了仕様変更対応
//			// 未了確定受信なしの場合はカードIDを'*'で埋める
//			Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;			// ブランド番号
//			Ec_Settlement_Res.settlement_data = Product_Select_Data;			// 決済額
//			memset(Ec_Settlement_Res.Card_ID, '*', sizeof(Ec_Settlement_Res.Card_ID));
//																				// Card ID
//// GG119202(E) 未了仕様変更対応
			// カード処理中受信後に未了中を受信するため、未了中受信時に復電用データを保存しない
			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );						// 問合せ(処理中)結果待ちタイマーリセット
// MH810103 GG119202(E) 未了中の停電で決済精算中止（復）を印字しない
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//			EcDeemedLog.MiryoFlg = 1;											// 未了中
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			break;
		case EC_SETT_STS_CARD_PROC:			/* カード処理中 */
			if( Suica_Rec.Data.BIT.CTRL_MIRYO ){								// すでに未了中なら破棄
				break;
			}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;							// 決済処理中
//			save_deemed_data(brandno);											// 復電用データ保存
			if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
				Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;						// 決済処理中
				save_deemed_data(brandno);										// 復電用データ保存
			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

			// 外部照会があるとき、磁気券による停止待ちはキャンセルして券を返却する（遅れてカードを受け取ったときも同様）
			// 精算完了はOPECTL.credit_in_monyで保持し、外部照会終了後に再判定する
			// 取り消しは受け付けなかったことにする
			if (OPECTL.op_faz == 8) {											// 電子媒体停止待ち合わせ中
				if (CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ) {	// STOP_REASONがカード挿入の場合はｶｰﾄﾞの吐き戻しを行う
					// カード挿入による停止待ちの場合はカードを返却する
					if (OPECTL.other_machine_card) {
						opr_snd( 13 );											// 保留位置からでも戻す
					}
					else {
						opr_snd( 2 );											// 前排出
					}
				}
				else {
					// 精算完了による停止待ちの場合はすれ違い入金フラグをセットして
					// 精算完了を待たせる
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
					// ブランド選択時のＴ、法人カード読取りと精算完了が同時に発生したときも
					// 状態を一時的に戻してブランド選択結果が来るのを待ち合わせる。
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
					OPECTL.credit_in_mony = 1;									// ｸﾚｼﾞｯﾄすれ違い入金あり
					read_sht_cls();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ
				}
				// 停止待ちステータスをクリアしてop_fazを入金中に戻す
				CCT_Cansel_Status.BYTE = 0;										// 停止待ちステータスをクリア
				OPECTL.op_faz = 1;												// 入金中
			}
			else {
// MH810103 GG119202(S) 磁気リーダの抜き取り待ち中はシャッター閉めない
				if ((RD_mod < 10) || (RD_mod > 13)) {							// 抜き取り待ちor移動中以外
// MH810103 GG119202(E) 磁気リーダの抜き取り待ち中はシャッター閉めない
				read_sht_cls();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ
// MH810103 GG119202(S) 磁気リーダの抜き取り待ち中はシャッター閉めない
				}
// MH810103 GG119202(E) 磁気リーダの抜き取り待ち中はシャッター閉めない
				if (OPECTL.op_faz == 3 ||										// とりけし
// MH810103 GG119202(S) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
//					OPECTL.op_faz == 9) {										// 電子媒体停止待ち合わせ中(とりけし)
					OPECTL.op_faz == 9 ||										// 電子媒体停止待ち合わせ中(とりけし)
					time_out == 1) {											// 初回入金料金表示時の無操作タイムアウト
// MH810103 GG119202(E) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
					// とりけし処理中にカード処理中受信はとりけし処理を止める
					// 停止待ちステータスをクリアしてop_fazを入金中に戻す
					OPECTL.CAN_SW = 0;											// とりけしボタンOFF
// MH810103 GG119202(S) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
					time_out = 0;												// 無操作タイムアウトフラグOFF
// MH810103 GG119202(E) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
					Suica_Rec.Data.BIT.ADJUSTOR_START = 1;						// 精算開始フラグセット
					cansel_status.BYTE = 0;										// 停止待ちステータスをクリア
					OPECTL.op_faz = 1;											// 入金中
					ac_flg.cycl_fg = 10;										// 入金中
				}
			}

			Lagcan( OPETCBNO, 1 );												// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
			Lagcan( OPETCBNO, 2 );												// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾒｯｸ監視用)
			Lagcan( OPETCBNO, 8 );												// ﾀｲﾏｰ8ﾘｾｯﾄ(入金後戻り判定ﾀｲﾏｰ)
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );							// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰﾘｾｯﾄ
			LagCan500ms(LAG500_SUICA_NONE_TIMER);								// 制御データ(受付禁止)の応答待ちタイマーリセット
// MH810103 GG119202(S) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
			LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);						// 待機復帰用最終タイマリセット
// MH810103 GG119202(E) 無操作タイムアウトと電子マネータッチ同時で決済リーダがリセットする
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
			ECCTL.prohibit_snd_bsy = 0;											// 制御データ(受付禁止)の応答監視状態クリア
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
// MH810103 MHUT40XX(S) Edy・WAON対応／決済結果受信待ちタイマ見直し
//// GG119202(S) 電子マネー決済時の決済結果受信待ちタイマ追加
//			if (brandno == BRANDNO_CREDIT) {
//				ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
//// GG119202(S) カード処理中タイマ値変更
////				if (ec_inquiry_wait_time == 0) {
////					ec_inquiry_wait_time = 360;
////				}
////				else if (ec_inquiry_wait_time < 240) {
////					ec_inquiry_wait_time = 240;
////				}
//				if (ec_inquiry_wait_time == 0) {
//					ec_inquiry_wait_time = 90;
//				}
//				else if (ec_inquiry_wait_time < 70) {
//					ec_inquiry_wait_time = 70;
//				}
//// GG119202(E) カード処理中タイマ値変更
//			}
//			else {
//// MHUT40XX(S) Edy・WAON対応
////				// クレジットカード以外（電子マネー／会員カード等）
//				// クレジットカード・電子マネー以外
//				if (!IsSettlementBrand((ushort)brandno)) {
//// MHUT40XX(E) Edy・WAON対応
//				ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 2, 1);
//				if (ec_inquiry_wait_time == 0) {
//					ec_inquiry_wait_time = 50;
//				}
//				else if (ec_inquiry_wait_time < 40) {
//					ec_inquiry_wait_time = 40;
//				}
//// MHUT40XX(S) Edy・WAON対応
//				}
//// MHUT40XX(E) Edy・WAON対応
//			}
//// MHUT40XX(S) Edy・WAON対応
//			if (isEcBrandNoEMoney((ushort)brandno, 0)) {
//				// 電子マネーブランドのタイマ開始
//				EcTimerUpdate();
//			}
//			else {
//// MHUT40XX(E) Edy・WAON対応
//			ec_inquiry_wait_time *= 50;
//// GG119202(E) 電子マネー決済時の決済結果受信待ちタイマ追加
//			Lagtim( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, ec_inquiry_wait_time );	// 問合せ(処理中)結果待ちタイマー起動
//// MHUT40XX(S) Edy・WAON対応
//			}
//// MHUT40XX(E) Edy・WAON対応
			ec_inquiry_wait_time = 0;
// MH810105(S) MH364301 QRコード決済対応
//			if (isEcBrandNoEMoney((ushort)brandno, 0)) {
//				// 電子マネーブランドのタイマ開始
			if (EcUseKindCheck(convert_brandno((ushort)brandno))) {
				// 電子マネーブランド、QRコード決済ブランドのタイマ開始
// MH810105(E) MH364301 QRコード決済対応
				EcTimerUpdate();
			}
			else {
				if (brandno == BRANDNO_CREDIT) {
					// クレジットカード
					ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
					if (ec_inquiry_wait_time == 0) {
						ec_inquiry_wait_time = 180;
					}
					else if (ec_inquiry_wait_time < 140) {
						ec_inquiry_wait_time = 140;
					}
				}
				else {
					// 電子マネー・クレジットカード以外
					ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 3, 1);
					if (ec_inquiry_wait_time == 0) {
						ec_inquiry_wait_time = 90;
					}
					else if (ec_inquiry_wait_time < 70) {
						ec_inquiry_wait_time = 70;
					}
				}
				ec_inquiry_wait_time *= 50;
				Lagtim( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, ec_inquiry_wait_time );	// 問合せ(処理中)結果待ちタイマー起動
			}
// MH810103 MHUT40XX(E) Edy・WAON対応／決済結果受信待ちタイマ見直し

			OPECTL.InquiryFlg = 1;												// 外部照会中フラグON
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
			if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&
				Suica_Rec.Data.BIT.BRAND_CAN != 0) {
				Suica_Rec.Data.BIT.BRAND_CAN = 0;								// ブランド選択取り消しを無かったことにする
			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			cn_stat( 2, 2 );													// 入金不可
			if( MIFARE_CARD_DoesUse ){											// Mifareが有効な場合
				op_MifareStop_with_LED();										// Mifare無効
			}
			if(brandno == BRANDNO_CREDIT) {
				ec_MessageAnaOnOff( 1, 1 );										// クレジットカード処理中
			} else {
				ec_MessageAnaOnOff( 1, 2 );										// カード処理中
			}
			if( OPECTL.op_faz == 0 ){											// 未入金
				OPECTL.op_faz = 1;												// 入金中
			}
			break;
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
		case EC_SETT_STS_DEEMED_SETTLEMENT:	// みなし決済
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
		case EC_SETT_STS_INQUIRY_OK:		/* 照会OK */
		case EC_SETT_STS_NONE:				/* 上記以外 */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			Suica_Rec.suica_rcv_event.BIT.SETTSTS_DATA = 1;
			Suica_Rec.suica_rcv_event.EC_BIT.SETTSTS_DATA = 1;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			break;
		default:
			break;
		}
// MH810103 MH810103(s) 電子マネー対応
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//		// 決済状態ﾃﾞｰﾀ_LCD_ICC_SETTLEMENT_STSをOpeTaskへ送信
//		queset(OPETCBNO, LCD_ICC_SETTLEMENT_STS, sizeof(unsigned char), &Ec_Settlement_Sts);		// 決済状態ﾃﾞｰﾀ
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
		// 決済状態ﾃﾞｰﾀ_LCD_ICC_SETTLEMENT_STSをOpeTaskへ送信
		queset(OPETCBNO, LCD_ICC_SETTLEMENT_STS, sizeof(unsigned char), &ECCTL.Ec_Settlement_Sts);		// 決済状態ﾃﾞｰﾀ
// MH810103 MH810103(e) 電子マネー対応
		break;

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	case S_RECEIVE_DEEMED:	/* 決済精算中止(復決済)データ */
//
//		memset(&DeemedData, 0, sizeof(T_FrmReceipt));
//		work_brand = convert_brandno( bcdbin3(&wrcvbuf[2]) );
//
//		/* 受信ﾃﾞｰﾀｾｯﾄ */
//		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf);
//		// EcRecvDeemedDataに受信データをセット
//		EcElectronSet_DeemedData(&Ec_Settlement_Res);
//
//		if (work_brand != EC_UNKNOWN_USED) {
//			ac_flg.cycl_fg = 57;										// 57:Ｔｴﾘｱ集計加算完了
//			// 不明ブランド以外ログ登録
//			Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
//		}
//
//		if( PrnJnlCheck() == ON ){
//			DeemedData.prn_kind = J_PRI;								/* ﾌﾟﾘﾝﾀ種別：ｼﾞｬｰﾅﾙ */
//			DeemedData.prn_data = &EcRecvDeemedData;					/* 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ */
//
//			// pritaskへ通知
//			queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &DeemedData);
//		}
//		break;
//
	case S_RECEIVE_DEEMED:	/* 決済直前取引データ */

		// データをセットしない場合は抜ける
		if( !EcRecvDeemed_DataSet( wrcvbuf ) ){
			break;
		}

		// 待機または休業の場合、ログ登録、印字処理する
		if( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ){
			EcRecvDeemed_RegistPri();
		}
		// 待機または休業ではない場合、処理フラグをONする（待機遷移後に実施する）
		else{
			ac_flg.ec_recv_deemed_fg = 1;							// 直前取引データ処理フラグON
		}
		break;
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	case S_SETTLEMENT_DATA:	/* 決済結果データ */
		if (wrcvbuf[0] == 0) {
			break;
		}
		work_brand = convert_brandno( bcdbin3(&wrcvbuf[2]) );

		switch (wrcvbuf[0]) {
		case EPAY_RESULT_OK:										// 決済OK
// MH810103 GG119202(S) 未了仕様変更対応
//		case EPAY_RESULT_MIRYO_AFTER_OK:							// 未了確定後決済OK
// MH810103 GG119202(E) 未了仕様変更対応
		case EPAY_RESULT_MIRYO_ZANDAKA_END:							// 未了残高照会完了
			memset(moni_wk, 0, sizeof(moni_wk));

			moni_wk[0] = (uchar)(work_brand - EC_USED + 1);			// カード種別(0だと情報なしとなってしまうため、1～としておく)

			memset(Card_Work, 0x20, sizeof(Card_Work));
			memcpyFlushRight(Card_Work, &wrcvbuf[13], sizeof(Card_Work), 20);
			memcpy(&moni_wk[1], &Card_Work[16], 4);					// カード番号の下４桁をセット
			moni_wk[5] = wrcvbuf[0];								// 決済結果
// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
			if (ECCTL.Ec_Settlement_Sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
				// みなし決済通知時は支払未了（0x11）をセットする
				moni_wk[5] = EPAY_RESULT_PAY_MIRYO;
				// カード番号がスペース埋めの場合は'*'をセットする
				if (!strncmp((char*)&moni_wk[1], "    ", 4)) {
					memset(&moni_wk[1], '*', 4);
				}
			}
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
			switch(work_brand)
			{
				case EC_CREDIT_USED:
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// 決済金額
					if (work > 0xffff) {
					// 取引額がshort値以上なら補正
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					wmonlg(OPMON_EC_CRE_SETTLEMENT, moni_wk, 0);	// モニタ登録
					break;
				case EC_EDY_USED:
				case EC_NANACO_USED:
				case EC_WAON_USED:
				case EC_SAPICA_USED:
				case EC_KOUTSUU_USED:
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
//				case EC_ID_USED:
//				case EC_QUIC_PAY_USED:
//				case EC_UNKNOWN_USED:
//				case EC_ZERO_USED:
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
					work = settlement_amount_ex(&wrcvbuf[7], 3);	// 決済前残額
					if (work > 0xffff) {
					// 決済前残額がshort値以上なら補正
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					work = settlement_amount_ex(&wrcvbuf[10], 3);	// 決済後残額
					if (work > 0xffff) {
					// 決済後残額がshort値以上なら補正
						work = 0xffff;
					}
					moni_wk[8] = (uchar)(work & 0xff);
					moni_wk[9] = (uchar)((work >> 8) & 0xff);
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// モニタ登録
					break;
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
				case EC_ID_USED:
				case EC_QUIC_PAY_USED:
// MH810105(S) MH364301 PiTaPa対応
				case EC_PITAPA_USED:
// MH810105(E) MH364301 PiTaPa対応
				case EC_UNKNOWN_USED:
				case EC_ZERO_USED:
					if (work_brand == EC_ID_USED) {
						memset(&moni_wk[2], '*', 3);				// 下3桁マスク
					}
					else if (work_brand == EC_QUIC_PAY_USED) {
						memset(&moni_wk[1], '*', 4);				// 下4桁マスク
					}
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// 決済金額
					if (work > 0xffff) {
					// 取引額がshort値以上なら補正
						work = 0xffff;
					}
					moni_wk[6] = (uchar)((work >> 8) & 0xff);
					moni_wk[7] = (uchar)(work & 0xff);
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// モニタ登録
					break;
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
// MH810105(S) MH364301 QRコード決済対応
				case EC_QR_USED:
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// 決済金額
					if (work > 0xffff) {
					// 取引額がshort値以上なら補正
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					moni_wk[8] = wrcvbuf[44];	// 支払種別
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// モニタ登録
					break;
// MH810105(E) MH364301 QRコード決済対応
				default:
					break;
			}
			break;
		default:
			break;
		}

		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA初期化終了？ */
			// 初期化中に有効な決済結果データを受信した場合は決済異常として登録する
			EcSettlementPhaseError(wrcvbuf, 7);
			break;
		}
		if (Suica_Rec.Data.BIT.PAY_CTRL == 1) {								/* すでに１度、電子決済を行っている場合 */
			EcSettlementPhaseError(wrcvbuf, 3);								/* エラー登録 */
			break;															/* 以後の処理をしないので抜ける */
		}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&			/* すでに決済結果データ受信済みで */
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) {  	/* まだOpeが処理を行っていない場合 or */
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1 &&		/* すでに決済結果データ受信済みで */
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) {	/* まだOpeが処理を行っていない場合 or */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			EcSettlementPhaseError(wrcvbuf, 2);								/* エラー登録 */
			break;															/* 以後の処理をしないので抜ける */
		}
		if (!Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
			// 決済中ではないのに有効な決済結果データを受信した場合は決済異常として登録する
			EcSettlementPhaseError(wrcvbuf, 8);
			break;
		}
		if (StopStatusGet(1) == 0x07) {
			// 精算中止時で全モジュール停止後に決済結果データを受信
			EcSettlementPhaseError(wrcvbuf, 9);
			break;
		}

		/* 受信ﾃﾞｰﾀｾｯﾄ */
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf);
		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf, 1);
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

		pay_ng = 1;
		// 受信したブランド番号が0？
		if (work_brand != EC_ZERO_USED) {
			// 選択商品データにて送信したブランド番号と一致？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			if( Product_Select_Brand == Ec_Settlement_Res.brand_no ){
			if( ECCTL.Product_Select_Brand == Ec_Settlement_Res.brand_no ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				pay_ng = 0;
				// E3265解除
				err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 0, 0, 0);
			} else {
				// 一致しなかった、エラー(E3263)登録
				pay_ng = 1;
			}
		} else {
			// 受信したブランド番号が0ならエラー(E3263)登録しない
			pay_ng = 0;
		}

		if (pay_ng != 0) {
		// 未対応／未設定ブランドの取引データ受信
		// 以降、同一精算内での電子マネー利用不可(※減額、集計は行わないが、モニタ登録は行う)
			err_chk((char)jvma_setup.mdl, ERR_EC_PAYBRAND_ERR, 1, 0, 0);// ｴﾗｰﾛｸﾞ登録（登録）
			// E3263発生後に精算を中止するとE3259エラーが発生した。
			// 改善連絡#10の改善によりE3263の発生条件を見直したが、
			// E3263が発生した場合、E3262も同時に発生させて
			// 電子マネーを利用不可とする。
			// 2019/07/29 本事象が発生した場合、決済を完了させてこれ以降の決済を不可とするように変更
			Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;			// EC決済異常発生フラグ
		}

		switch (Ec_Settlement_Res.Result) {
		case EPAY_RESULT_OK:					// 決済OK
// MH810103 GG119202(S) 未了残高照会中のとき、決済額(0円含む)と商品選択データで差がある場合、取引異常となり、取消キー押下待ちとなるが、キーが効かない修正
		//取引異常の未了確定は、取引異常せず、取引失敗/取り消し画面表示する
//		case EPAY_RESULT_MIRYO:					// 未了確定
// MH810103 GG119202(E) 未了残高照会中のとき、決済額(0円含む)と商品選択データで差がある場合、取引異常となり、取消キー押下待ちとなるが、キーが効かない修正
// MH810103 GG119202(S) 未了仕様変更対応
//		case EPAY_RESULT_MIRYO_AFTER_OK:		// 未了確定後決済OK
// MH810103 GG119202(E) 未了仕様変更対応
		case EPAY_RESULT_MIRYO_ZANDAKA_END:		// 未了残高照会完了
			if (Product_Select_Data != Ec_Settlement_Res.settlement_data) {
			// 要求額と決済額が不一致
				// エラーログ登録
				memset(err_wk, 0, sizeof(err_wk));
				memcpy(err_wk, "\?\?-0000   00000:00000", 21);
				// カード種別
				memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QRコード決済対応
				if( work_brand == EC_QR_USED ){							// QRの場合、支払種別をセット
					memset(&err_wk[3], 0x20, 4);						// ４桁分スペースをセット
					memcpy(&err_wk[3], QrBrandKind_prn[Ec_Settlement_Res.Brand.Qr.PayKind], 2);
				}
				else{
// MH810105(E) MH364301 QRコード決済対応
				// カード番号
				memset(Card_Work, 0x20, sizeof(Card_Work));
				memcpyFlushRight(Card_Work, Ec_Settlement_Res.Card_ID, sizeof(Card_Work), sizeof(Ec_Settlement_Res.Card_ID));
				memcpy(&err_wk[3], &Card_Work[16], 4);					// カード番号の下４桁をセット
// MH810105(S) MH364301 QRコード決済対応
				}
// MH810105(E) MH364301 QRコード決済対応
				// 取引指示額
				work = Product_Select_Data;
				if (work > 0xffff) {
				// 取引指示額がshort値以上なら補正
					memcpy(&err_wk[10], "FFFFH", 5);
				} else {
					intoasl(&err_wk[10], work, 5);
				}
				// 取引種別
				if (Ec_Settlement_Res.Result != EPAY_RESULT_OK) {
					memcpy(&err_wk[7], "(A)", 3);
				}
				// 取引額
				work = Ec_Settlement_Res.settlement_data;
				if (work > 0xffff) {
				// 取引額がshort値以上なら補正
					memcpy(&err_wk[16], "FFFFH", 5);
				} else {
					intoasl(&err_wk[16], work, 5);
				}
				err_chk2((char)jvma_setup.mdl, ERR_EC_PAY_ERR, 1, 1, 0, err_wk);	/* ｴﾗｰﾛｸﾞ登録（登録）*/
				// E3262登録（決済不良：ASCII） 			"E3262   08/03/17 14:38   発生/解除"
				//											"        ED-3456   01000:01100     "
				//											"        ED-3456(A)01000:01100     "
				//												 種別、カード番号下4桁、取引種別、取引指示額、取引額
#if (4 != AUTO_PAYMENT_PROGRAM)										// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
// ※基本的に(0 == AUTO_PAYMENT_PROGRAM)なので、有効
//   決済リーダでもSUICA_LOG_REC,SUICA_LOG_REC_FOR_ERRの領域を使用する
				memcpy(&SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof(struct suica_log_rec));	// 異常ﾛｸﾞとして現在の通信ﾛｸﾞを登録する。
#endif
				Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);								// 制御ﾃﾞｰﾀ（取引終了）を送信する
				ec_MessageAnaOnOff( 0, 0 );										// 表示中の文言を消す
				OPECTL.InquiryFlg = 0;											// 外部照会中フラグOFF
				Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR = 1;					// 受信した決済額と送信した選択商品データに差異がある
				Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;							// 未了タイムアウトが発生したことにする
				Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// 選択商品データ送信可否フラグクリア
// MH810103 GG119202(S) 未了残高照会中で要求額と決済額が不一致の決済結果：未了残高照会完了を受信したとき、取消キーがきかない。
				Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;
				Suica_Rec.Data.BIT.CTRL_MIRYO = 0;
// MH810103 GG119202(E) 未了残高照会中で要求額と決済額が不一致の決済結果：未了残高照会完了を受信したとき、取消キーがきかない。
// MH810103 MHUT40XX(S) カード抜き取り待ちメッセージ表示が上書きされる
				// E3262発生で決済リーダ切り離しするので、カード差し込み状態はクリアする
				Suica_Rec.Data.BIT.CTRL_CARD = 0;								// カード差し込み状態をクリア
// MH810103 MHUT40XX(E) カード抜き取り待ちメッセージ表示が上書きされる
				dsp_change = 0;													// 未了表示ﾌﾗｸﾞを初期化
				queset(OPETCBNO, SUICA_PAY_DATA_ERR, 0, NULL);					// 決済異常を通知
// MH810103 GG119202(S) E3259、E3262、E3263発生でJVMAリセットしない
				// 決済不良発生時はJVMAリセットが完了するまで
				// 決済リーダとの通信は行わず、コインメック等の通信を優先させる
				// ec_init()内でECCTL.phaseを更新する
				Suica_Rec.Data.BIT.INITIALIZE = 0;
				ECCTL.phase = EC_PHASE_WAIT_RESET;
// MH810103 GG119202(E) E3259、E3262、E3263発生でJVMAリセットしない
// MH810103 GG119202(S) E3262発生時にみなし決済になってしまう
				Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );					// 問合せ(処理中)結果待ちタイマーリセット
// MH810103 GG119202(E) E3262発生時にみなし決済になってしまう
				return;															// 決済処理はしない（ただ取りとする）
			}
// MH810103 GG119202(S) 決済状態（みなし決済）の判定修正
			if (ECCTL.Ec_Settlement_Sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
// MH810105(S) MH364301 みなし決済データ受信エラー追加
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_DEEMED_SETTLEMENT, 2, 0, 0);
// MH810105(E) MH364301 みなし決済データ受信エラー追加
				// 決済状態=みなし決済の場合は設定50-0014に従う
				EcDeemedSettlement(&OPECTL.Ope_mod);
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
//				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT)) {
//					// 休業であれば、決済結果データを処理しない
//// MH810104 GG119202(S) みなし休業になるとき取引終了が出ない
//					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// 制御ﾃﾞｰﾀ（取引終了）を送信する
//// MH810104 GG119202(E) みなし休業になるとき取引終了が出ない
//					return;
//				}
				if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0) {
					// 決済状態＝みなし決済受信で「決済OKとして精算完了」しない場合は
					// 決済結果データを処理しない
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// 制御ﾃﾞｰﾀ（取引終了）を送信する
					return;
				}
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善
// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
//// MH810103 GG118807_GG118907(S) 未了残高照会中に通信不良発生でみなし決済になる
//				if (Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0) {
//					// 未了確定後は決済結果データを処理しない
//					return;
//				}
//// MH810103 GG118807_GG118907(E) 未了残高照会中に通信不良発生でみなし決済になる
//// MH810104 GG119201(S) WAON精算時、未了残高処理中に決済状態(みなし決済、結果OK)受信で処理未了取引記録を2件登録する
//				if ( work_brand == EC_WAON_USED &&
//// MH810104 GG119202(S) WAON精算時、現金とみなし決済の競合でみなし決済が完了しない
////					 OPECTL.op_faz == 8 &&
////					 CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){
////					// ブランドがWAONであり、未了確定済であれば決済結果データを処理しない
////					// ブランドがWAONであり50-0011①=2以外の場合、Suica_Rec.Data.BIT.MIRYO_TIMEOUTフラグがONにならないため
////					// CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END(精算完了)によるOPECTL.op_faz=8(停止中)である場合
////					// 決済結果データを処理しないようにここでreturnする。
//					 EcDeemedLog.MiryoFlg == 2 &&
//					 IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON) == 0 ){
//					// ブランドがWAONであり、未了確定済であれば決済結果データを処理しない
//					// ブランドがWAONであり50-0011①=2以外の場合(精算を完了させる場合)、Suica_Rec.Data.BIT.MIRYO_TIMEOUTフラグがONにならないため
//					// EcDeemedLog.MiryoFlg=2(未了確定)でありERR_EC_ALARM_TRADE_WAON(E3267)のエラーが発生していない場合
//					// 決済結果データを処理しないようにここでreturnする。
//// MH810104 GG119202(E) WAON精算時、現金とみなし決済の競合でみなし決済が完了しない
//					return;
//				}
//// MH810104 GG119201(E) WAON精算時、未了残高処理中に決済状態(みなし決済、結果OK)受信で処理未了取引記録を2件登録する
// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
//// MH810104 GG119201(S) 現金投入と同時にカードタッチ→現金で精算完了せずに電子マネーで精算完了してしまう
//				if ( ryo_buf.zankin == 0 &&
//					 PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0){
//					// 決済状態=みなし決済(50-0014⑥=1)と入金の競合により残金が0円である場合は
//					// 現金での精算完了をさせるため(deemed_sett_fin：みなし決済による精算完了フラグはOFF)
//					// 決済結果データを処理しないようにここでreturnする。
//					return;
//				}
//// MH810104 GG119201(E) 現金投入と同時にカードタッチ→現金で精算完了せずに電子マネーで精算完了してしまう
//// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
//				if (Ec_Settlement_Res.Result == EPAY_RESULT_NG) {
//					// みなし決済通知＋精算継続（50-0014④=2）の場合は
//					// ここで抜ける
//					return;
//				}
//// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善
			}
// MH810103 GG119202(E) 決済状態（みなし決済）の判定修正
			break;
// MH810103 GG119202(S) カード処理中受信後の決済NG（304）の扱い
		case EPAY_RESULT_NG:
			if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
				// 未了発生中
				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
// MH810104 GG119202(S) みなし休業になるとき取引終了が出ない
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// 制御ﾃﾞｰﾀ（取引終了）を送信する
// MH810104 GG119202(E) みなし休業になるとき取引終了が出ない
// MH810103 GG119001_GG119101(S) 未了残高照会中に通信不良発生でみなし決済になる
//					if (Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) {
//						// 未了確定後の決済NG（304）受信は未了残高照会タイムアウト扱いとする
//						ec_miryo_timeout();
//					}
//					else {
//						// 未了確定前の決済NG（304）受信はみなし決済扱いとする
//						EcDeemedSettlement(&OPECTL.Ope_mod);
//					}
					// 未了確定前の決済NG（304）受信はみなし決済扱いとする
					// 未了確定後の決済NG（304）受信は未了残高照会タイムアウト扱いとする
					EcDeemedSettlement(&OPECTL.Ope_mod);
// MH810103 GG119001_GG119101(E) 未了残高照会中に通信不良発生でみなし決済になる
				}
				// 未了発生中の決済NGは無視する
				return;
			}
			else if (OPECTL.InquiryFlg != 0) {
				// カード処理中受信済み
// MH810103 GG119202(S) クレジット決済で決済NG（304）受信するとみなし決済してしまう
//				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304) &&
// MH810105(S) MH364301 QRコード決済対応
//					isEcBrandNoEMoney(Ec_Settlement_Res.brand_no, 0)) {
					EcUseKindCheck(convert_brandno(Ec_Settlement_Res.brand_no))) {	//電子マネー使用中？
// MH810105(E) MH364301 QRコード決済対応
					// 電子マネーブランドのみみなし決済扱いとする
// MH810103 GG119202(E) クレジット決済で決済NG（304）受信するとみなし決済してしまう
					// カード処理中受信後の決済NG（304）受信はみなし決済扱いとする
// MH810104 GG119202(S) みなし休業になるとき取引終了が出ない
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);					// 制御ﾃﾞｰﾀ（取引終了）を送信する
// MH810104 GG119202(E) みなし休業になるとき取引終了が出ない
					EcDeemedSettlement(&OPECTL.Ope_mod);
					return;
				}
			}
			break;
// MH810103 GG119202(E) カード処理中受信後の決済NG（304）の扱い
		default:
			break;
		}
		// ※現状、残額ありケースはありえないが、すれ違い対策として残しておく
		if (ryo_buf.zankin > Ec_Settlement_Res.settlement_data && 			/* 残額がまだある？ */
			OPECTL.op_faz != 3 && 											/* 精算中止処理中ではない */
			(OPECTL.op_faz == 8 && wrcvbuf[0] == EPAY_RESULT_OK)) {			/* 電子媒体停止処理中？ */
			cn_stat(3, 2);													// 紙幣 & COIN再入金可
			if (!pas_kep || ((RED_stat[2] & TICKET_MOUTH) ==  TICKET_MOUTH)) {	/* 定期券が挿入口にある */
				read_sht_opn();												/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ */
			}
		}
// ﾃｽﾄ用 引去り金額を元に戻す
#ifdef	EC_DEBUG
		if (CPrmSS[S_SCA][98] != 0L) {
			Ec_Settlement_Res.settlement_data *= 10;
		}
#endif
// ﾃｽﾄ用 引去り金額を元に戻す

		if (Suica_Rec.Data.BIT.INITIALIZE) {								/* 初期化終了？ */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				/* 取引ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
			Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;			/* 取引ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		}

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&			// 決済結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON?
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1 &&		// 決済結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON?
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			(Ec_Settlement_Res.Result == EPAY_RESULT_OK ||					// 決済OK？
// MH810103 GG119202(S) 未了仕様変更対応
//			 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_AFTER_OK ||		// 未了確定後決済OK？
// MH810103 GG119202(E) 未了仕様変更対応
			 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END)) {	// 未了残高照会完了？
			// このﾌﾗｸﾞをｸﾘｱするﾀｲﾐﾝｸﾞは精算ﾛｸﾞ及び精算中止ﾛｸﾞに決済データを登録するか、
			// 次精算時の精算を開始した場合とする。
			Suica_Rec.Data.BIT.LOG_DATA_SET = 1;							/* 決済結果ﾃﾞｰﾀ受信済み */

			// このフラグは精算を開始する前及び決済が行われた場合にクリアし、
			// セットするタイミングは精算開始後、駐車料金が発生した場合とする
			Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							/* 選択商品データ送信可否フラグクリア */
		}
// MH810103 GG119202(S) E3259、E3262、E3263発生でJVMAリセットしない
		if (pay_ng != 0) {
			// 未設定ブランド取引データ受信時はJVMAリセットが完了するまで
			// 決済リーダとの通信は行わず、コインメック等の通信を優先させる
			// ec_init()内でECCTL.phaseを更新する
			EcBrandClear();
			Suica_Rec.Data.BIT.INITIALIZE = 0;
			ECCTL.phase = EC_PHASE_WAIT_RESET;
		}
// MH810103 GG119202(E) E3259、E3262、E3263発生でJVMAリセットしない
		break;

	case S_VOLUME_RES_DATA:	/* 音量変更結果データ */
		if(!Suica_Rec.Data.BIT.INITIALIZE) {
// MH810103 GG119202(S) 起動シーケンス不具合修正
			if (ECCTL.phase != EC_PHASE_VOL_CHG) {
				// フェーズ違いで受信した場合は無視する
				break;
			}
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
			if( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV == 0 ) {	// 音量変更結果データ未受信
				Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV = 1;		// 音量変更結果データ受信済み
// MH810103 GG119202(S) 起動シーケンス不具合修正
				Lagcan(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT);
				setEcBrandNegoComplete();
// MH810103 GG119202(E) 起動シーケンス不具合修正
			}
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
			break;
		}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		RecvVolumeRes = wrcvbuf[0];									// 音量変更結果を保存
		ECCTL.RecvVolumeRes = wrcvbuf[0];							// 音量変更結果を保存
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		// メンテに通知
		queset(OPETCBNO, EC_VOLUME_UPDATE, 0, NULL);			// 音量変更更新を通知
		break;
	case S_MODE_CHG_DATA:	/* リーダメンテナンス結果データ */
		if(!Suica_Rec.Data.BIT.INITIALIZE) {
			break;
		}
		p = &wrcvbuf[0];
		RecvMntTbl.cmd = *p++;										// リーダメンテナンス結果を保存
		RecvMntTbl.mode = *p++;
		RecvMntTbl.vol = *p++;
		queset(OPETCBNO, EC_MNT_UPDATE, 0, NULL);					// リーダメンテナンス更新を通知
		break;
	case S_BRAND_RES_DATA:	/* ブランド選択結果データ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA初期化終了？ */
			break;
		}
		p = &wrcvbuf[0];
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//		if(RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// 選択状態から
//			*p == EC_BRAND_UNSELECTED) {							// 未選択状態に戻された
//			Suica_Rec.Data.BIT.SLCT_BRND_BACK = 1;					// 「戻る」検出
//		}
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
		RecvBrandResTbl.res = *p++;									// ブランド選択結果を保存
		if(RecvBrandResTbl.res == EC_BRAND_SELECTED) {				// 選択
			RecvBrandResTbl.no  = bcdbin3(p);
			p += 2;
			RecvBrandResTbl.siz = *p++;
			memcpy(RecvBrandResTbl.dat, p, EC_CARDDATA_LEN);
		} else {													// 未選択
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			RecvBrandResTbl.no  = (ushort)BRANDNO_UNKNOWN;
			RecvBrandResTbl.no  = bcdbin3(p);
			p += 2;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			RecvBrandResTbl.siz = 0;
			memset(RecvBrandResTbl.dat, 0x20, EC_CARDDATA_LEN);
		}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		Suica_Rec.suica_rcv_event.BIT.BRAND_DATA = 1;				// ブランド選択結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON
		Suica_Rec.suica_rcv_event.EC_BIT.BRAND_DATA = 1;			// ブランド選択結果ﾃﾞｰﾀ受信ﾌﾗｸﾞON
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		break;
// MH810105(S) MH364301 QRコード決済対応
	case S_SUB_BRAND_RES_DATA:	/*サブブランド一覧データ*/
		memset(&RecvSubBrandTbl, 0, sizeof(RecvSubBrandTbl));
		p = &wrcvbuf[0];
		check_sub_brand_tbl(p, &RecvSubBrandTbl);
		break;
// MH810105(E) MH364301 QRコード決済対応

	default:
		return;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド合わせ
//[]----------------------------------------------------------------------[]
///	@param[out]		*tbl	：ブランド制御テーブル
///	@param[in]		index	：受信ブランドテーブルインデックス
///	@param[in]		*recvtbl：受信ブランド状態テーブル
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	set_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl)
{
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	int		i, j;
//	EC_BRAND_CTRL ctrl;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	
// 受信データチェック
	// ブランド合わせチェック
	check_brand_tbl(tbl, recvtbl);

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//※ソート不要
//// 受信したブランド状態データを送信するブランド設定データにソートしてセット(ブランド番号の若順に並び替える)
//	for (i = 0; i < (tbl->num-1); i++) {
//		for (j = 0; j < (tbl->num-1-i); j++) {
//			if (tbl->ctrl[j].no > tbl->ctrl[j+1].no) {
//				memcpy(&ctrl, &(tbl->ctrl[j]), sizeof(EC_BRAND_CTRL));
//				memcpy(&(tbl->ctrl[j]), &(tbl->ctrl[j+1]), sizeof(EC_BRAND_CTRL));
//				memcpy(&(tbl->ctrl[j+1]), &ctrl, sizeof(EC_BRAND_CTRL));
//			}
//		}
//	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド合わせチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		*tbl		：送信したブランド制御テーブル
///	@param[in]		*recvtbl	：受信ブランド状態テーブル
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	check_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl)
{
	int		i, j;
	int		err = 1;		// 1=有効ブランドなし

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//// 受信データチェック
//	if (recvtbl->num == 0) {
//		return;
//	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	tbl->num = 0;
	for (i=0; i<recvtbl->num; i++) {
		if (recvtbl->ctrl[i].no == 0) {
			continue;
		}
		tbl->num++;
		tbl->ctrl[i].no = recvtbl->ctrl[i].no;
		memcpy(&(tbl->ctrl[i].name[0]), &(recvtbl->ctrl[i].name[0]), 20);
		if (isEcBrandNoEnabledForSetting(recvtbl->ctrl[i].no)) {
// MH810103 GG119202(S) ブランド状態判定処理変更
//			if (recvtbl->ctrl[i].status == 0x01) {
			if (isEC_BRAND_STS_KAIKYOKU(recvtbl->ctrl[i].status)) {
// MH810103 GG119202(E) ブランド状態判定処理変更
			// 受信した状態：開局、有効→一致
				ecRegistBrandError(&recvtbl->ctrl[i], 0);			// エラー解除
				tbl->ctrl[i].status = 0x00;		// 有効、開局
				err = 0;
			} else {
			// 受信した状態：開局、有効以外→不一致
				ecRegistBrandError(&recvtbl->ctrl[i], 1);			// エラー登録
				tbl->ctrl[i].status = 0x11;		// 無効、開局禁止
			}
		} else {
			tbl->ctrl[i].status = 0x11;		// 無効、開局禁止
		}
	}

	if (err != 0) {
	// 有効ブランドなし
		// 設定済みのブランドも含めてチェックする
		for(i = 0; i < _countof(BrandTbl); i++) {
			for(j = 0; j < BrandTbl[i].num; j++) {
				if(BrandTbl[i].ctrl[j].status == 0) {
					err = 0;		// 既に有効なブランドが設定されている
					break;
				}
			}
			if(err == 0) {
				break;
			}
		}
		// 再度有効ブランドの有無をチェック
		if(err != 0) {
			Suica_Rec.suica_err_event.BIT.BRAND_NONE = 1;	// 有効ブランドなしﾌﾗｸﾞON
		} else {
			Suica_Rec.suica_err_event.BIT.BRAND_NONE = 0;	// 有効ブランドなしﾌﾗｸﾞOFF
		}
	} else {
	// 一つでもブランドステータスが一致
		Suica_Rec.suica_err_event.BIT.BRAND_NONE = 0;	// 有効ブランドなしﾌﾗｸﾞOFF
	}
// MH810103 GG119202(S) 起動シーケンス不具合修正
//	if (ECCTL.phase == EC_PHASE_BRAND) {
//	// ※ブランド合わせ完了時、フェーズがブランド合わせ中ならフェーズを変える
//		ECCTL.phase = EC_PHASE_PAY;						// ブランド合わせ中解除
//		ECCTL.step = 0;
//	}
// MH810103 GG119202(E) 起動シーケンス不具合修正
}

// MH810105(S) MH364301 QRコード決済対応
// MH810105(S) MH364301 QRコード決済対応（仕様変更）
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダ サブブランド使用設定判定
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_no		: ブランド番号
/////	@param[in]		sub_brand_no	: サブブランド種別
/////	@return			0:未定義サブブランド、1:未使用設定サブブランド、2:使用設定サブブランド
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2021/11/26<br>
/////					Update	:
////[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
//uchar isSubBrandNoEnabledForSetting(const ushort brand_no, const uchar sub_brand_no)
//{
//	uchar	ret = 0;
//	uchar	tbl_cnt;
//	ushort	i;
//	const EC_SUB_BRAND_ENABLED_PARAM_TBL *tbl;
//
//
//	if( isEC_USE() == 0 ){
//		return ret;
//	}
//
//	switch (brand_no) {
//	case BRANDNO_QR:
//		tbl = QrSubBrandEnabledParamTbl;
//		tbl_cnt = TBL_CNT(QrSubBrandEnabledParamTbl);
//		break;
//	default:
//		return ret;
//	}
//
//	// 決済リーダーから送られてきたサブブランドが使われていたら0、使われてなかったら1を返す
//	for (i = 0; i < tbl_cnt; i++, tbl++) {
//		if (sub_brand_no == tbl->brand_no) {
//			if (prm_get(COM_PRM, S_ECR, (short)tbl->address, 1, (char)tbl->pos) == 1) {
//				// 使用設定サブブランド
//				ret = 2;
//			}
//			else {
//				// 未使用設定サブブランド
//				ret = 1;
//			}
//			break;
//		}
//	}
//	return ret;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			サブブランドテーブルチェック（more）
////[]----------------------------------------------------------------------[]
/////	@param[in]		recv_tbl	: サブブランドテーブル
/////	@return			0:テーブルに使用設定サブブランド全てあり
/////					1:テーブルに未使用設定サブブランドあり
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2022/03/30
////[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
//static uchar check_sub_brand_tbl_for_more(EC_SUB_BRAND_TBL *recv_tbl)
//{
//	ushort	i, j;
//	ushort	brand_no;
//	uchar	sub_brand_no;
//	uchar	ofs;
//	uchar	tbl_sts;
//
//
//	ofs = 0;
//	tbl_sts = 0;
//	for (i = 0; i < recv_tbl->brand_num; i++) {
//		// ブランド番号
//		brand_no = recv_tbl->brand_no[i];
//
//		for (j = 0; j < recv_tbl->sub_brand_num[i]; j++) {
//			sub_brand_no = recv_tbl->sub_brand_no[ofs+j];
//			if (isSubBrandNoEnabledForSetting(brand_no, sub_brand_no) != 2) {
//				// 未使用設定、または、未定義サブブランド
//				err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 1, 0, 0);	// エラー登録
//				tbl_sts = 1;
//				break;
//			}
//		}
//
//		if (tbl_sts != 0) {
//			// エラー発生済みであれば、ループを抜ける
//			break;
//		}
//
//		// オフセット更新
//		ofs += recv_tbl->sub_brand_num[i];
//	}
//
//	return tbl_sts;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			サブブランドテーブルチェック（less）
////[]----------------------------------------------------------------------[]
/////	@param[in]		recv_tbl	: サブブランドテーブル
/////	@param[in]		chk_brand	: 検索ブランド番号
/////	@param[in]		chk_tbl		: 検索サブブランド設定テーブル
/////	@param[in]		chk_tbl_cnt	: 検索サブブランド設定テーブル件数
/////	@return			0:使用するサブブランドを全て受信した、
/////					2:使用するサブブランドを受信していない
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2022/03/30
////[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
//static uchar check_sub_brand_tbl_for_less(EC_SUB_BRAND_TBL *recv_tbl, ushort chk_brand,
//											const EC_SUB_BRAND_ENABLED_PARAM_TBL *chk_tbl,
//											uchar chk_tbl_cnt)
//{
//	ushort	i, j, k;
//	uchar	sub_brand_no;
//	uchar	ofs;
//	uchar	chk;
//	uchar	tbl_sts;
//
//
//	// ブランド番号チェック
//	chk = 0;
//	tbl_sts = 0;
//	for (k = 0; k < recv_tbl->brand_num; k++) {
//		// ブランド番号
//		if (chk_brand == recv_tbl->brand_no[k]) {
//			chk = 1;
//			break;
//		}
//	}
//
//	if (chk == 0) {
//		// ブランド番号なし
//		return 1;
//	}
//
//	// オフセットを計算
//	ofs = 0;
//	for (i = 0; i < k; i++) {
//		ofs += recv_tbl->sub_brand_num[i];
//	}
//
//	// 検索サブブランド設定テーブルと受信サブブランドテーブルを比較する
//	for (i = 0; i < chk_tbl_cnt; i++, chk_tbl++) {
//		if (prm_get(COM_PRM, S_ECR, (short)chk_tbl->address, 1, (char)chk_tbl->pos) == 0) {
//			// サブブランド設定がない
//			continue;
//		}
//
//		chk = 0;
//		for (j = 0; j < recv_tbl->sub_brand_num[k]; j++) {
//			sub_brand_no = recv_tbl->sub_brand_no[ofs+j];
//			if (chk_tbl->brand_no == sub_brand_no) {
//				chk = 1;
//				break;
//			}
//		}
//
//		if (chk == 0) {
//			// 使用設定したサブブランドを受信していない
//			err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 1, 0, 0);	// エラー登録
//			tbl_sts = 2;
//			break;
//		}
//	}
//
//	return tbl_sts;
//}
// MH810105(E) MH364301 QRコード決済対応（仕様変更）

//[]----------------------------------------------------------------------[]
///	@brief			サブブランド合わせチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: サブブランド一覧データ
///	@param[in]		recv_tbl	: サブブランドテーブル
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31<br>
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	check_sub_brand_tbl(uchar *data, EC_SUB_BRAND_TBL *recv_tbl)
{
	ushort	i, j;
	uchar	idx;
	uchar	ofs;

	recv_tbl->tbl_sts.BYTE = 0;
	// サブブランドテーブル受信
	recv_tbl->tbl_sts.BIT.RECV = 1;
	// ブランド数セット
	idx = 0;
	recv_tbl->brand_num = data[idx++];

	ofs = 0;
	for (i = 0; i < recv_tbl->brand_num; i++) {
		// ブランド番号セット
		recv_tbl->brand_no[i] = bcdbin3(&data[idx]);
		idx += 2;

		// サブブランド数セット
		recv_tbl->sub_brand_num[i] = data[idx++];

		// サブブランド種別セット
		for (j = 0; j < recv_tbl->sub_brand_num[i]; j++) {
			recv_tbl->sub_brand_no[ofs+j] = data[idx++];
		}

		// オフセット更新
		ofs += recv_tbl->sub_brand_num[i];
	}

// MH810105(S) MH364301 QRコード決済対応（仕様変更）
//	// E3279
//	// 精算機で使用設定していないサブブランドを受信した(more)
//	recv_tbl->tbl_sts.BIT.MORE_LESS = check_sub_brand_tbl_for_more(recv_tbl);
//	if (recv_tbl->tbl_sts.BIT.MORE_LESS != 0) {
//		// エラー発生済みであれば、lessのチェックなし
//		return;
//	}
//
//	// 精算機で設定した使用設定したサブブランドを受信しなかった(less)
//	// QRコード決済
//	recv_tbl->tbl_sts.BIT.MORE_LESS = check_sub_brand_tbl_for_less(recv_tbl, BRANDNO_QR,
//																	QrSubBrandEnabledParamTbl,
//																	(uchar)TBL_CNT(QrSubBrandEnabledParamTbl));
//
//	if (recv_tbl->tbl_sts.BIT.MORE_LESS == 0) {
//		// more、lessのチェックOKのため、エラー解除
//		err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 0, 0, 0);	// エラー解除
//	}
// MH810105(E) MH364301 QRコード決済対応（仕様変更）
}

//[]----------------------------------------------------------------------[]
///	@brief			サブブランドを持つブランド番号チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: ブランド番号
///	@return			-1:サブブランドを持たない、
///					 0:サブブランドなし、
///					 1～:サブブランドインデックス
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
int EcSubBrandCheck(ushort brand_no)
{
	uchar	i;
	uchar	chk;

	// サブブランドを持つブランド番号か？
	chk = 0;
	for (i = 0; i < TBL_CNT(EcSubBrandUseTbl); i++) {
		if (brand_no == EcSubBrandUseTbl[i]) {
			chk = 1;
			break;
		}
	}

	if (chk == 0) {
		// サブブランドを持たない
		return -1;
	}

	// サブブランドテーブルから検索
	for (i = 0; i < RecvSubBrandTbl.brand_num; i++) {
		// ブランド番号が一致、かつ、サブブランドがある
		if (brand_no == RecvSubBrandTbl.brand_no[i] &&
			RecvSubBrandTbl.sub_brand_num[i] > 0) {
			return (int)(i+1);
		}
	}

	// サブブランドなし
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			サブブランド名取得
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: ブランド番号
///	@param[in]		kind		: 0:サブブランドテーブルから検索、
///								  1:サブブランド名前リストから取得、
///	@param[in]		idx			: kind=0 サブブランドインデックス、
///								  kind=1 サブブランド種別
///	@param[in,out]	buff		: サブブランド名
///	@return			サブブランド名の長さ
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
int EcGetSubBrandName(ushort brand_no, uchar kind, uchar idx, uchar *buff)
{
	uchar	i;
	uchar	ofs;
	ushort	sub_brand_no = 0;
	const uchar	**list;
	uchar	list_cnt;
	int		len = 0;


	switch (brand_no) {
	case BRANDNO_QR:
		list = sub_brand_name_list_QR;
		list_cnt = QR_PAY_KIND_MAX;
		break;
	default:
		return 0;
	}


	// サブブランドテーブルから検索
	if (kind == 0) {
		ofs = 0;
		for (i = 0; i < RecvSubBrandTbl.brand_num; i++) {
			// ブランド番号
			if (brand_no == RecvSubBrandTbl.brand_no[i]) {
				sub_brand_no = RecvSubBrandTbl.sub_brand_no[ofs+idx];
				break;
			}

			// オフセット更新
			ofs += RecvSubBrandTbl.sub_brand_num[i];
		}

		if (sub_brand_no == 0) {
			// サブブランド種別０はなし
			return 0;
		}
	}
	// サブブランド名前リストから取得
	else if (kind == 1) {
		sub_brand_no = idx;
	}


	if (sub_brand_no >= list_cnt) {
		sub_brand_no = 0;
	}
	strcpy((char*)buff, (char*)list[sub_brand_no]);
	len = (int)strlen((char*)buff);

	return len;
}
// MH810105(E) MH364301 QRコード決済対応

// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
//[]----------------------------------------------------------------------[]
///	@brief			ブランドテーブル更新
//[]----------------------------------------------------------------------[]
///	@param[in]		*srctbl		：精算機が保持しているブランドテーブル
///	@param[in]		*recvtbl	：受信ブランド状態テーブル
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/09<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void update_brand_tbl(EC_BRAND_TBL *srctbl, EC_BRAND_TBL *recvtbl)
{
	ushort	i, j;

	if (memcmp(srctbl, recvtbl, sizeof(EC_BRAND_TBL)) == 0) {
		// 一致する場合は何もしない
		return;
	}

	// ブランド追加／削除チェック
	for (i = 0; i < srctbl->num; i++) {
		for (j = 0; j < recvtbl->num; j++) {
			// ブランドNo.が一致するか？
			if (srctbl->ctrl[i].no == recvtbl->ctrl[j].no) {
				// ヒットしたのでreservedにしるしをつける
				recvtbl->ctrl[j].reserved = 1;
				break;
			}
		}
		if (j >= recvtbl->num) {
			// @todo ブランド削除を記録
		}
	}

	for (i = 0; i < recvtbl->num; i++) {
		// reservedが1か？
		if (recvtbl->ctrl[i].reserved == 0) {
			// @todo ブランド追加を記録
		}
		recvtbl->ctrl[i].reserved = 0;
	}

	// ブランドテーブルを更新(受信したブランドテーブルで上書き)
	nmisave(srctbl, recvtbl, (short)sizeof(EC_BRAND_TBL));
}
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする

//[]----------------------------------------------------------------------[]
///	@brief			決済データセット
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	:	データセット構造体
///	@param[in]		*buf	:	受信データ
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
///	@param[in]		errchk	:	詳細エラーコード登録有無（0=エラー登録しない　1=エラー登録する）
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf)
void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf, uchar errchk)
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
{
	uchar	brand_index = convert_brandno( bcdbin3(&buf[2]) );
	ushort	brand_no;
	uchar	*p = buf;
// MH810105 GG119202(S) 処理未了取引集計仕様改善
	ulong	work;
// MH810105 GG119202(E) 処理未了取引集計仕様改善

	// 取引データ用の作業領域を初期化
	memset(data, 0, sizeof(EC_SETTLEMENT_RES));
	memset(data->Card_ID, 0x20, sizeof(data->Card_ID));

	data->Result					= *p++;										// 決済結果
	data->Column_No					= *p++;										// 決済コラム番号
	data->brand_no					= bcdbin3(p);								// ブランド番号
	p += 2;
	data->settlement_data			= settlement_amount_ex(p, 3);				// 決済額
	p += 3;
	data->settlement_data_before	= settlement_amount_ex(p, 3);				// 決済前残高
	p += 3;
	data->settlement_data_after		= settlement_amount_ex(p, 3);				// 決済後残高
	p += 3;
	memcpy(data->Card_ID, p, sizeof(data->Card_ID));							// カードID
	p += 20;

	// 決済時刻
	data->settlement_time.seco		= bcdbin(*p++);
	data->settlement_time.minu		= bcdbin(*p++);
	data->settlement_time.hour		= bcdbin(*p++);
	data->settlement_time.week		= bcdbin(*p++);
	data->settlement_time.date		= bcdbin(*p++);
	data->settlement_time.mont		= bcdbin(*p++);
	data->settlement_time.year		= bcdbin3(p);
	p += 2;

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	// 詳細エラーコード
//	memcpy(Ec_Settlement_Rec_Code, p, sizeof(Ec_Settlement_Rec_Code));			// 詳細エラーコード
//	memset(err_wk, 0, sizeof(err_wk));
//	memcpy(err_wk, "詳細コード000", 13);
//	memcpy(&err_wk[10], Ec_Settlement_Rec_Code, sizeof(Ec_Settlement_Rec_Code));
//// GG119202(S) 決済リーダ関連判定処理見直し
////	ec_Disp_No = 0;																// 決済結果詳細エラーコードの案内表示パターン
//	ECCTL.ec_Disp_No = 0;														// 決済結果詳細エラーコードの案内表示パターン
//// GG119202(E) 決済リーダ関連判定処理見直し
//
//	// 詳細エラーコードが 000 ではない
//	if (!(Ec_Settlement_Rec_Code[0] == '0' &&
//		  Ec_Settlement_Rec_Code[1] == '0' &&
//		  Ec_Settlement_Rec_Code[2] == '0')) {
//// GG119202(S) 電子マネー系詳細エラーコード追加
////			ec_errcode_err_chk(err_wk, Ec_Settlement_Rec_Code);					// 詳細エラー登録
//			ec_errcode_err_chk(err_wk, Ec_Settlement_Rec_Code, brand_index);	// 詳細エラー登録
//// GG119202(E) 電子マネー系詳細エラーコード追加
//	}
//	// 詳細エラーコード201ではない && ブランド番号が 0
//	if(!(Ec_Settlement_Rec_Code[0] == '2' &&
//		 Ec_Settlement_Rec_Code[1] == '0' &&
//		 Ec_Settlement_Rec_Code[2] == '1') &&
//		 brand_index == EC_ZERO_USED){
//			err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 1, 0, 0); 		// E3265登録
//	}
// MH810105(S) MH364301 QRコード決済対応
	if (data->brand_no == BRANDNO_QR) {
		// 詳細エラーでサブブランドを使用するため、エラーチェックよりも先に保存
		data->Brand.Qr.PayKind = *(p+3);										// 支払種別
	}
// MH810105(E) MH364301 QRコード決済対応
	memcpy(Ec_Settlement_Rec_Code, p, sizeof(Ec_Settlement_Rec_Code));			// 詳細エラーコード
// MH810105 GG119202(S) 案内表示パターンをクリアする
	ECCTL.ec_Disp_No = 0;														// 決済結果詳細エラーコードの案内表示パターン
// MH810105 GG119202(E) 案内表示パターンをクリアする
// MH810105(S) MH364301 PiTaPa対応
	if(data->brand_no == BRANDNO_PITAPA){
		memcpy(&data->Brand.Pitapa.Reason_Code, p+74, sizeof(data->Brand.Pitapa.Reason_Code));	// 理由コード(6バイト)
	}
// MH810105(E) MH364301 PiTaPa対応
	// errchk=1の場合詳細エラー登録する
	if( errchk ){
		EcErrCodeChk( Ec_Settlement_Rec_Code, brand_index );
	}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	p += 3;

	brand_no = data->brand_no;
	// 不明ブランド？ || ブランド番号0？
	if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
		// 選択商品データで送信したブランドで固有データをセットする
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		brand_no = Product_Select_Brand;
		brand_no = ECCTL.Product_Select_Brand;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	}
// MH810105 GG119202(S) 処理未了取引集計仕様改善
	if (ECCTL.ec_Disp_No == 96) {
		// 未引き去り確認時はR0252を登録する
		memset(moni_wk, 0, sizeof(moni_wk));
		moni_wk[0] = (uchar)(brand_index - EC_USED + 1);	// カード種別
		memcpyFlushRight(Card_Work, data->Card_ID, sizeof(Card_Work), sizeof(data->Card_ID));
		memcpy(&moni_wk[1], &Card_Work[16], 4);				// カード番号の下４桁をセット
		moni_wk[5] = data->Result;							// 決済結果
		work = data->settlement_data_before;				// 決済前残額
		if (work > 0xffff) {
		// 決済前残額がshort値以上なら補正
			work = 0xffff;
		}
		moni_wk[6] = (uchar)(work & 0xff);
		moni_wk[7] = (uchar)((work >> 8) & 0xff);
		moni_wk[8] = moni_wk[6];							// 決済後残高（決済前残高と同じ金額をセットする）
		moni_wk[9] = moni_wk[7];
		wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);			// モニタ登録
	}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
	// 電子マネー決済データ解析
	switch (brand_no) {
// MH810103 GG119202(S) 電子マネー対応
	case	BRANDNO_KOUTSUU:
	// 交通系
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
		p += 15;
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
// MH810105(S) MH364301 不要変数削除
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
// MH810105(E) MH364301 不要変数削除
		p += 30;
		memcpy(&data->Brand.Koutsuu.SPRW_ID, p, sizeof(data->Brand.Koutsuu.SPRW_ID));	// SPRWID(13バイト)
		p += 13;
		memcpy(&data->Brand.Koutsuu.Kamei, p, sizeof(data->Brand.Koutsuu.Kamei));		// 加盟店名(30バイト)
		p += 30;
		data->Brand.Koutsuu.TradeKind = *p++;										// 取引種別(1バイト)
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		break;
// MH810103 GG119202(E) 電子マネー対応
	case	BRANDNO_EDY:
	// Edy
// MH810103 MHUT40XX(S) Edy・WAON対応
//		memcpy(&data->Brand.Edy.DealNo, p, 4);										// 取引通番(4バイトそのままコピー)
//		p += 4;
//		memcpy(&data->Brand.Edy.CardDealNo, p, 4);									// カード取引通番(4バイトそのままコピー)
//		p += 4;
//		memcpy(&data->Brand.Edy.TerminalNo, p, 8);									// 端末番号(8バイトそのままコピー)
//		p += 8;
//		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));
//		sprintf(EcEdyTerminalNo, "%08lX%08lX"
//			, data->Brand.Edy.TerminalNo[0], data->Brand.Edy.TerminalNo[1]);		// 上位端末ID（16進数：16桁、0パディング）
		memcpy(&data->Brand.Edy.DealNo, p, 10);										// 取引通番(10バイトそのままコピー)
		p += 10;
		memcpy(&data->Brand.Edy.CardDealNo, p, 5);									// カード取引通番(5バイトそのままコピー)
		p += 5;
		memcpy(&data->Brand.Edy.TerminalNo, p, 8);									// 上位端末ID(8バイトそのままコピー)
		p += 8;
// MH810105 GG119202(S) Edy上位端末IDが消える
		if (data->Result == EPAY_RESULT_OK ||
			data->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
// MH810105 GG119202(E) Edy上位端末IDが消える
		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));
		memcpy(EcEdyTerminalNo, data->Brand.Edy.TerminalNo, sizeof(data->Brand.Edy.TerminalNo));	// 上位端末ID
// MH810105 GG119202(S) Edy上位端末IDが消える
		}
// MH810105 GG119202(E) Edy上位端末IDが消える
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810103 GG119202(S) 電子マネー対応
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
		p += 15;
// MH810103 GG119202(E) 電子マネー対応
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
// MH810105(S) MH364301 不要変数削除
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
// MH810105(E) MH364301 不要変数削除
		p += 30;
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
// MH810103 MHUT40XX(S) Edy・WAON対応
		memcpy(&data->Brand.Edy.Kamei, p, sizeof(data->Brand.Edy.Kamei));			// 加盟店名(30バイト)
		p += 30;
// MH810103 MHUT40XX(E) Edy・WAON対応
		break;
	case	BRANDNO_NANACO:
	// nanaco
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		data->Brand.Nanaco.TerminalNo[0] = settlement_amount_ex(p, 4);				// 上位端末ID①
//		p += 4;
//		data->Brand.Nanaco.TerminalNo[1] = settlement_amount_ex(p, 4);				// 上位端末ID②
//		p += 4;
//		data->Brand.Nanaco.TerminalNo[2] = settlement_amount_ex(p, 4);				// 上位端末ID③
//		p += 4;
//		memcpy(&data->Brand.Nanaco.DealNo, p, 4);									// 上位取引通番(4バイトそのままコピー)
//		p += 4;
//// GG119202(S) 電子マネー対応
//		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
//		p += 15;
//// GG119202(E) 電子マネー対応
//// GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
//		p += 30;
//// GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		memcpy(&data->Brand.Nanaco.Kamei, p, sizeof(data->Brand.Nanaco.Kamei));		// 加盟店名(30バイト)
		p += 30;
		memcpy(&data->Brand.Nanaco.DealNo, p, sizeof(data->Brand.Nanaco.DealNo));	// 端末取引通番(6バイト)
		p += 6;
		memcpy(&data->Brand.Nanaco.TerminalNo, p, sizeof(data->Brand.Nanaco.TerminalNo));	// 上位端末ID(20バイト)
		p += 20;
// MH810105(S) MH364301 不要変数削除
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイト)
// MH810105(E) MH364301 不要変数削除
		p += 30;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイト)
		p += 15;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
	case	BRANDNO_WAON:
	// WAON
// MH810103 MHUT40XX(S) Edy・WAON対応
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
		p += 15;
// MH810105(S) MH364301 不要変数削除
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
// MH810105(E) MH364301 不要変数削除
		p += 30;
		memcpy(&data->Brand.Waon.SPRW_ID, p, sizeof(data->Brand.Waon.SPRW_ID));		// SPRWID(13バイト)
		p += 13;
		memcpy(&data->Brand.Waon.Kamei, p, sizeof(data->Brand.Waon.Kamei));			// 加盟店名(30バイト)
		p += 30;
// MH810103 MHUT40XX(E) Edy・WAON対応
		memcpy(&data->Brand.Waon.GetPoint, p, 4);									// 今回ポイント(4バイトそのままコピー)
		p += 4;
// MH810103 MHUT40XX(S) Edy・WAON対応
//		memcpy(&data->Brand.Waon.TotalPoint, p, 4);									// 累計ポイント(4バイトそのままコピー)
//		p += 4;
//		data->Brand.Waon.CardType		= settlement_amount_ex(p, 4);				// カード分類
//		p += 4;
//		memcpy(&data->Brand.Waon.PeriodPoint, p, 4);								// ２年前までに獲得したポイント(4バイトそのままコピー)
//		p += 4;
//		data->Brand.Waon.Period			= settlement_amount_ex(p, 4);				// ２年前までに獲得したポイントの有効期限
//		p += 4;
//		memcpy(&data->Brand.Waon.SPRW_ID, p, 14);									// SPRWID
//		p += 14;
//		data->Brand.Waon.PointStatus	= *p++;										// 利用可否
//		data->Brand.Waon.DealCode		= *p++;										// 取引種別コード
//// GG119202(S) 電子マネー対応
//		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
//		p += 15;
//// GG119202(E) 電子マネー対応
//// GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
//		p += 30;
//// GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		memcpy(&data->Brand.Waon.TotalPoint, p, 8);									// 累計ポイント(8バイトそのままコピー)
		p += 8;
		data->Brand.Waon.PointStatus = *p++;										// 累計ポイントメッセージ(1バイト)
// MH810103 MHUT40XX(E) Edy・WAON対応
		break;
	case	BRANDNO_SAPICA:
	// SAPICA
		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// 物販端末ID
		p += 8;
		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// 一件明細ID(4バイトそのままコピー)
		p += 4;
// MH810103 GG119202(S) 電子マネー対応
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイトそのままコピー)
		p += 15;
// MH810103 GG119202(E) 電子マネー対応
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
// MH810105(S) MH364301 不要変数削除
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// 端末識別番号(30バイトそのままコピー)
// MH810105(E) MH364301 不要変数削除
		p += 30;
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		break;
	case	BRANDNO_ID:
	// ID
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// 物販端末ID
//		p += 8;
//		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// 一件明細ID(4バイトそのままコピー)
//		p += 4;
		memcpy(&data->Brand.Id.Kamei, p, sizeof(data->Brand.Id.Kamei));				// 加盟店名(30バイト)
		p += 30;
		memcpy(&data->Brand.Id.Approval_No, p, sizeof(data->Brand.Id.Approval_No));	// 承認番号(7バイト)
		p += 7;
		memcpy(&data->Brand.Id.TerminalNo, p, sizeof(data->Brand.Id.TerminalNo));	// 上位端末ID(13バイト)
		p += 13;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイト)
		p += 15;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
	case	BRANDNO_QUIC_PAY:
	// QUIC_PAY
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// 物販端末ID
//		p += 8;
//		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// 一件明細ID(4バイトそのままコピー)
//		p += 4;
		memcpy(&data->Brand.QuicPay.Kamei, p, sizeof(data->Brand.QuicPay.Kamei));	// 加盟店名(30バイト)
		p += 30;
		memcpy(&data->Brand.QuicPay.Approval_No, p, sizeof(data->Brand.QuicPay.Approval_No));	// 承認番号(7バイト)
		p += 7;
		memcpy(&data->Brand.QuicPay.TerminalNo, p, sizeof(data->Brand.QuicPay.TerminalNo));	// 上位端末ID(13バイト)
		p += 13;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// 問合せ番号(15バイト)
		p += 15;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
// MH810105(S) MH364301 PiTaPa対応
	case	BRANDNO_PITAPA:
	// PiTaPa
		memcpy(&data->Brand.Pitapa.Kamei, p, sizeof(data->Brand.Pitapa.Kamei));				// 加盟店名(30バイト)
		p += 30;
		memcpy(&data->Brand.Pitapa.Slip_No, p, sizeof(data->Brand.Pitapa.Slip_No));			// 伝票番号(5バイト)
		p += 5;
		memcpy(&data->Brand.Pitapa.TerminalNo, p, sizeof(data->Brand.Pitapa.TerminalNo));	// 上位端末ID(13バイト)
		p += 13;
		memcpy(&data->Brand.Pitapa.Approval_No, p, sizeof(data->Brand.Pitapa.Approval_No));	// 承認番号(8バイト)
		p += 8;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));							// 問合せ番号(15バイト)
		p += 15;
		memcpy(&data->Brand.Pitapa.Reason_Code, p, sizeof(data->Brand.Pitapa.Reason_Code));	// 理由コード(6バイト)
		p += 6;
		break;
// MH810105(E) MH364301 PiTaPa対応
	case	BRANDNO_CREDIT:		// クレジットカード決済
		data->Brand.Credit.Slip_No = (10000U * bcdbin(*(p+2))) + (100U * bcdbin(*(p+1))) + bcdbin(*p);		// 端末処理通番（伝票番号）
		p += 3;
		memcpy(data->Brand.Credit.Approval_No, p, 7);								// 承認番号
		p += 7;
		// 0x20埋め、コード下3桁を左詰め
// MH321800(S) NT-NET精算データ仕様変更
//		memcpyFlushLeft2( &data->Brand.Credit.KID_Code[0], &p[3], sizeof(data->Brand.Credit.KID_Code), 3 );	// KID コード
		memcpyFlushLeft( &data->Brand.Credit.KID_Code[0], &p[3], sizeof(data->Brand.Credit.KID_Code), 3 );	// KID コード
// MH321800(E) NT-NET精算データ仕様変更
		p += 6;
		memcpy(data->Brand.Credit.Id_No, p, 13);									// 端末識別番号
		p += 13;
		memcpy(data->Brand.Credit.Credit_Company, p, 24);							// クレジットカード会社名
		p += 24;
		data->Brand.Credit.Identity_Ptrn = *p++;									// 本人確認パターン
		break;
// MH810105(S) MH364301 QRコード決済対応
	case	BRANDNO_QR:
	// QRコード
		p += 1;
		// 支払種別コード
		p += 16;
		memcpy(&data->settlement_data_after, p, sizeof(data->settlement_data_after));	// 残高
		p += 4;
		memcpy(data->QR_Kamei, p, sizeof(data->QR_Kamei));								// 加盟店名
		p += 30;
		memcpy(data->Brand.Qr.PayTerminalNo, p, sizeof(data->Brand.Qr.PayTerminalNo));	// 支払端末ID
		p += 32;
		memcpy(data->inquiry_num, p, sizeof(data->inquiry_num));						// 取引番号
		p += 15;
		memcpy(data->Brand.Qr.MchTradeNo, p, sizeof(data->Brand.Qr.MchTradeNo));		// Mch取引番号
		p += 32;
		break;
// MH810105(E) MH364301 QRコード決済対応
	default	:
	// UnKnown
		break;
	}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
	// 決済結果情報_LCD_ICC_SETTLEMENT_RSLTをOpeTaskへ送信
// MH810103 MH810103(s) 電子マネー対応 #5592 【電子マネー検定対応】『直前取引内容要求』の結果をジャーナル記録する
//	queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// 決済結果情報
	if(errchk){
		// 直前取引から呼ばれた場合以外に送信する
// MH810105(S) #6199 決済処理中の障害発生でみなし決済＋決済結果OK受信時、50-0014の設定を無視して処理が完了する
//		queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// 決済結果情報
		// みなしの時は、EcDeemedSettlementで処理させる
		if (ECCTL.Ec_Settlement_Sts != EC_SETT_STS_DEEMED_SETTLEMENT) {
			queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// 決済結果情報
		}
// MH810105(E) #6199 決済処理中の障害発生でみなし決済＋決済結果OK受信時、50-0014の設定を無視して処理が完了する
	}
// MH810103 MH810103(e) 電子マネー対応 #5592 【電子マネー検定対応】『直前取引内容要求』の結果をジャーナル記録する
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド番号→ブランドインデックス変換
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno	:	ブランド番号(bin)
///	@return			ブランドインデックス(EC_EDY_USED～)
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	convert_brandno(const ushort brandno)
{
	uchar	index;

	switch (brandno) {
	case BRANDNO_ZERO:
		index = EC_ZERO_USED;
		break;
	case BRANDNO_KOUTSUU:
		index = EC_KOUTSUU_USED;
		break;
	case BRANDNO_EDY:
		index = EC_EDY_USED;
		break;
	case BRANDNO_NANACO:
		index = EC_NANACO_USED;
		break;
	case BRANDNO_WAON:
		index = EC_WAON_USED;
		break;
	case BRANDNO_SAPICA:
		index = EC_SAPICA_USED;
		break;
	case BRANDNO_ID:
		index = EC_ID_USED;
		break;
	case BRANDNO_QUIC_PAY:
		index = EC_QUIC_PAY_USED;
		break;
// MH810105(S) MH364301 PiTaPa対応
	case BRANDNO_PITAPA:
		index = EC_PITAPA_USED;
		break;
// MH810105(E) MH364301 PiTaPa対応
	case BRANDNO_CREDIT:
		index = EC_CREDIT_USED;
		break;
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	case BRANDNO_TCARD:
		index = EC_TCARD_USED;
		break;
	case BRANDNO_HOUJIN:
		index = EC_HOUJIN_USED;
		break;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 QRコード決済対応
	case BRANDNO_QR:
		index = EC_QR_USED;
		break;
// MH810105(E) MH364301 QRコード決済対応
	default:	// unknown
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		// @todo ハウスカードは暫定
		if (brandno >= BRANDNO_HOUSE_S && brandno <= BRANDNO_HOUSE_E) {
			index = EC_HOUSE_USED;
			break;
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		index = EC_UNKNOWN_USED;
		break;
	}

	return index;
}

// MH810103 GG119202(S) 不要関数削除
////[]----------------------------------------------------------------------[]
/////	@brief			制御データ：受付許可要求
////[]----------------------------------------------------------------------[]
/////	@param[in]		msg		:	イベントメッセージ
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			G.So
/////	@date			Create	:	2019/02/07<br>
/////					Update
/////	@file			ope_ec_ctrl.c
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	ec_usable_req(ushort msg)
//{
//	if (ECCTL.phase != EC_PHASE_PAY) {
//	// 精算中以外は要求不可
//		return;
//	}
//	if (OPECTL.Mnt_mod != 0) {
//	// メンテナンス中
//		return;
//	}
//	else {
//	// 運用
//		if (OPECTL.Ope_mod == 100 || OPECTL.Ope_mod == 110) {
//		// 休業・警報アラームの場合
//			return;
//		}
//	}
//
//	if ((Suica_Rec.Data.BIT.CTRL == 0)
//	&&	(Suica_Rec.Data.BIT.OPE_CTRL == 0)) {
//	// 受付不可状態 && 受付許可要求していない
//		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);		// 電子マネー利用を可にすることで選択商品データを送信する
//	}
//}
// MH810103 GG119202(E) 不要関数削除

//[]----------------------------------------------------------------------[]
///	@brief			受信データ待ち処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret: 0:Continue	1:recv wait
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	ec_split_data_check(void)
{
	uchar	ret = 0;
	uchar	rcv_judge = 0;
	t_Suica_rcv_event	w_rcv_data;

	w_rcv_data.BYTE = Suica_Rec.suica_rcv_event.BYTE;							// 受信データ情報をワークエリアにセット
	if (!w_rcv_data.BYTE) {														// 受信ｲﾍﾞﾝﾄが何も無い場合
		return 1;																// 受信解析処理はしない
	}

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	rcv_judge = 0x2A;															// 決済状態データ、状態データ、決済結果データ
	rcv_judge = SETTLEMENT_COMPLETE_COND;										// 決済状態データ、状態データ、決済結果データ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	w_rcv_data.BYTE &= rcv_judge;												// 表示依頼・決済結果・状態データのみ抽出
	if (!Suica_Rec.Data.BIT.DATA_RCV_WAIT) {									// 待ち合わせ中？
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {									// CRW-MJA精算中？(選択商品データ送信時にセットされる)
		/* 決済リーダは決済終了時、
			成功～OK結果と受付不可
			失敗～NG結果と取引キャンセル＋受付不可
			を送ってくる。（未了終了、NG終了時も同様でこの点がSX-10と若干異なる）
		*/
		/*	受付禁止制御と未了とがぶつかると、実行中を通知してくる。
			未了終了時に、決済結果、状態データ、決済結果データが揃っても、状態データがまだ未了＋実行中
			になっているときがあり、「未了」のため精算が完了できないでロックする。引き続き受付不可
			がくるので待ち合わせる。
		*/
			if (w_rcv_data.BYTE == rcv_judge) {									// 全てのデータがあるとき
				if (EC_STATUS_DATA.StatusInfo.MiryoStatus != 0	||				// 未了発生、
					EC_STATUS_DATA.StatusInfo.Running != 0) {					// 実行中の場合
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					w_rcv_data.BIT.STATUS_DATA = 0;								// 状態データを無効にして解除されるのを待つ
					w_rcv_data.EC_BIT.STATUS_DATA = 0;							// 状態データを無効にして解除されるのを待つ
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
																				// 未了、実行中はEC_STATUS_DATA_WAITに保存しない
																				// 未了発生時、状態データだけで実行してしまうのも防止する
				}
			}
			if (w_rcv_data.BYTE == rcv_judge &&									// 全てのデータがあり
				EC_STATUS_DATA.StatusInfo.ReceptStatus != 0) {					// 受付不可
			// 決済結果 + 状態(受付不可) + 決済状態データ受信済み
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// CRW-MJA精算中フラグクリア
				ret = 0;														// 明示的に0をセット
			}
		/*	WAONを使って未了タイムアウトが起きた時、決済結果でNGが通知されるが取引データが
			付属して送られて来なかったことがあった。決済結果があると取引データで未了終了判
			定を行うため、取引データを待ち合わせる。単なる中止では決済結果が来ないため、状
			態データで取引を終了する。
		*/
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// 決済結果がなくて
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0)							// 状態データがあるとき
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// 決済結果がなくて
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0)						// 状態データがあるとき
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				&&	 (EC_STATUS_DATA.StatusInfo.TradeCansel != 0)
				&&	 (EC_STATUS_DATA.StatusInfo.ReceptStatus != 0)) {
			// 状態データ受信(取引キャンセル + 受付不可)
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// CRW-MJA精算中フラグクリア
				ret = 0;														// 明示的に0をセット
			}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// 決済結果がなくて
//				&&	 (w_rcv_data.BIT.SETTSTS_DATA != 0) &&						// 決済状態データがあるとき
//					(Ec_Settlement_Sts == EC_SETT_STS_MIRYO)					// 未了中か
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// 状態データがあるとき
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// 決済結果がなくて
				&&	 (w_rcv_data.EC_BIT.SETTSTS_DATA != 0) &&					// 決済状態データがあるとき
					(isEC_STS_MIRYO())											// 未了中か
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// 状態データがあるとき
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					(EC_STATUS_DATA.StatusInfo.MiryoStatus ||					// 未了発生か
					  EC_STATUS_DATA.StatusInfo.Running)) {						// 実行中の場合は、精算状態は継続し、OpeTaskへの通知を行う
				ret = 0;														// 明示的に0をセット
			}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// 決済結果がなくて
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// 状態データがあるとき
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// 決済結果がなくて
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// 状態データがあるとき
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) 取消ボタン押下時、制御データ：禁止を送信し、状態データのみ応答があるとき、「もう一度同じカードをタッチしてください」と表示する修正
					 (isEC_CONF_MIRYO() == 0) &&								// 状態データ(未了確定)受信していないとき
// MH810103 GG119202(E) 取消ボタン押下時、制御データ：禁止を送信し、状態データのみ応答があるとき、「もう一度同じカードをタッチしてください」と表示する修正
					  (!EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// 受付可かつ取引キャンセル受付BITが立っていない場合は、精算状態は継続し、OpeTaskへの通知は行わない
					   !EC_STATUS_DATA.StatusInfo.TradeCansel)) {
				ret = 0;														// 明示的に0をセット
			}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// 決済結果がなくて
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// 状態データがあるとき
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// 決済結果がなくて
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// 状態データがあるとき
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					  EC_STATUS_DATA.StatusInfo.RebootReq) {					// 再起動要求BITが立っている場合は、精算状態は継続し、OpeTaskへの通知を行う
				ret = 0;														// 明示的に0をセット
			}
// MH810103 GG119202(S) 未了仕様変更対応
//			else if (w_rcv_data.BIT.SETTLEMENT_DATA != 0 &&						// 決済結果あり
//					 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO &&			// 決済結果=未了確定
//					 w_rcv_data.BIT.SETTSTS_DATA == 0 &&						// 決済状態データなし
//					 w_rcv_data.BIT.STATUS_DATA == 0) {							// 状態データなし
//				ret = 0;														// 明示的に0をセット
//			}
// MH810103 GG119202(E) 未了仕様変更対応
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//			else if(Suica_Rec.Data.BIT.SLCT_BRND_BACK) {						// ブランド選択状態から未選択になった
//				ret = 0;
//			}
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
			else if (w_rcv_data.EC_BIT.BRAND_DATA != 0) {
				ret = 0;
			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 状態データ（未了確定）受信のデータ待ち合わせ処理修正
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// 決済結果がなくて
				&&	 (w_rcv_data.EC_BIT.SETTSTS_DATA == 0)						// 決済状態データがなくて
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0)						// 状態データがあるとき
				&&	  isEC_CONF_MIRYO()) {										// 状態データ(未了確定)BITが立っている場合
				ret = 0;
			}
// MH810103 GG119202(E) 状態データ（未了確定）受信のデータ待ち合わせ処理修正
			else {
				EC_STATUS_DATA_WAIT.status_data = 0;
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if (w_rcv_data.BIT.STATUS_DATA) {								// 受信したデータが制御データで待ち合わせの場合
				if (w_rcv_data.EC_BIT.STATUS_DATA) {							// 受信したデータが制御データで待ち合わせの場合
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					EC_STATUS_DATA_WAIT.status_data = EC_STATUS_DATA.status_data;	// 制御情報を待ち合わせようにセーブ
				}
				Suica_Rec.rcv_split_event.BYTE = w_rcv_data.BYTE;				// 待ち合わせ用のエリアに状態を保存
				Suica_Rec.Data.BIT.DATA_RCV_WAIT = 1;							// データ受信待ちフラグセット
				ret = 1;														// 受信処理はしない（待ち合わせ中）
			}
		}
	} else {
		Suica_Rec.rcv_split_event.BYTE |= w_rcv_data.BYTE;						// 受信したデータの状態を待ち合わせエリアに反映

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (w_rcv_data.BIT.STATUS_DATA) {										// 受信したデータが状態データの場合
		if (w_rcv_data.EC_BIT.STATUS_DATA) {									// 受信したデータが状態データの場合
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			EC_STATUS_DATA.status_data |= EC_STATUS_DATA_WAIT.status_data;		// 待ち合わせ用のデータをマージする
		}

		if (Suica_Rec.rcv_split_event.BYTE == rcv_judge &&						// 全てのデータがあり
			EC_STATUS_DATA.StatusInfo.ReceptStatus != 0) {							// 受付不可
		// 決済結果 + 状態 + 決済状態受信済み(結果は気にしない)
			ret = 0;															// 明示的に0をセット
		}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		else if ((Suica_Rec.rcv_split_event.BIT.SETTLEMENT_DATA == 0)			// 決済結果がなくて
//			&&	 (Suica_Rec.rcv_split_event.BIT.STATUS_DATA != 0)				// 状態データがあるとき
		else if ((Suica_Rec.rcv_split_event.EC_BIT.SETTLEMENT_DATA == 0)		// 決済結果がなくて
			&&	 (Suica_Rec.rcv_split_event.EC_BIT.STATUS_DATA != 0)			// 状態データがあるとき
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			&&	 (EC_STATUS_DATA.StatusInfo.TradeCansel != 0)
			&&	 (EC_STATUS_DATA.StatusInfo.ReceptStatus != 0)) {
		// 状態データ受信(取引キャンセル + 受付不可)
			ret = 0;															// 明示的に0をセット
		}
		else {
			ret = 1;															// 受信処理はしない（待ち合わせ中）
		}
		if (ret == 0) {
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;								// CRW-MJA精算中フラグクリア
			Suica_Rec.suica_rcv_event.BYTE = Suica_Rec.rcv_split_event.BYTE;	// 受信判定用のエリアを更新
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;								// 待ち合わせフラグクリア
		/* DATA_RCV_WAITセット時に移動
			EC_STATUS_DATA_WAIT.status_data = 0;								// 状態データ待ち合わせ用エリアクリア
		*/
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			拡張データ変換
//[]----------------------------------------------------------------------[]
///	@param[in]		*buf	:	変換データ先頭位置
///	@param[in]		count	:	変換バイト数
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
long	settlement_amount_ex(unsigned char *buf, unsigned char count)
{
	long	pay;

	for (pay = 0; count > 0 ;count--) {
		pay *= 100;
		pay += bcdbin(buf[count-1]);
	}

	return (pay);
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EcArmClearCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassegeにて取得したID
///	@return			ret				: 0=処理継続 1=ﾙｰﾌﾟ処理をContinueする
//[]----------------------------------------------------------------------[]
///	@note			Ope_ArmClearCheck()から呼ばれる
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
char	Ope_EcArmClearCheck(ushort msg)
{
	char	ret = 0;
	char	exec = 0;

	// エラー表示の反転ﾒｯｾｰｼﾞ表示を消去する
	switch (msg) {
	case SUICA_EVT:											// リーダからの受信ﾃﾞｰﾀ
		// カードタッチで、取引データ、状態データ、決済結果データを必ず受信する(ec_split_data_check()参照)
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 0) {
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 0) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		// 取引データ受信なし
			break;
		}
		else
		if (Ec_Settlement_Res.Result == EPAY_RESULT_NG) {
		// 取引データ受信あり && 結果=残高不足or決済NG
			break;
		}
	// no break;
	case COIN_EVT:											// Coin Mech event
	case NOTE_EVT:											// Note Reader event
		if ((COIN_EVT == msg && !(OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5))
		||	(NOTE_EVT == msg && !(OPECTL.NT_QSIG == 1 || OPECTL.NT_QSIG == 5))) {
			break;
		}
	// no break;
	case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
	case KEY_TEN_F4:										// 取消ﾎﾞﾀﾝ ON
// MH810105(S) MH364301 休業画面に＜取引に失敗しました＞が表示される
	case AUTO_CANCEL:										// 精算自動キャンセル
// MH810105(E) MH364301 休業画面に＜取引に失敗しました＞が表示される
	case ELE_EVENT_CANSEL:									// 終了イベント（精算中止）
	case ELE_EVT_STOP:										// 精算完了イベント（電子媒体の停止）
	case COIN_RJ_EVT:										// 精算中止(レバー)
		if (msg == ELE_EVT_STOP && CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END) {
		// 精算完了時以外は抜ける
			break;
		}
		if ( (msg == COIN_RJ_EVT || msg == KEY_TEN_F4) && OPECTL.CAN_SW == 1) {
		// 取消ﾎﾞﾀﾝの再押下の場合
			ret = 1;										// 取消処理をさせない（ｲﾍﾞﾝﾄ破棄）
		}
		if (OPECTL.InquiryFlg != 0 || Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// カード処理中？ or 未了発生中？
			break;
		}
// MH810105(S) MH364301 QRコード決済対応
		if (ECCTL.Ec_FailureContact_Sts == 1) {
		// 障害連絡票発行中？
			break;
		}
// MH810105(E) MH364301 QRコード決済対応

		// Suica残額不足表示中 か Suicaの未了ﾀｲﾑｱｳﾄ後のﾒｯｾｰｼﾞ表示中
		if (dsp_fusoku != 0) {
			exec = 1;
		}
		else if (Suica_Rec.Data.BIT.MIRYO_ARM_DISP != 0) {
// MH810105(S) MH364301 休業画面に＜取引に失敗しました＞が表示される
//			if (!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_F4 || (OPECTL.op_faz == 3 || OPECTL.op_faz == 9)) {
			if (!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_F4 || msg == AUTO_CANCEL ||
				(OPECTL.op_faz == 3 || OPECTL.op_faz == 9)) {
// MH810105(E) MH364301 休業画面に＜取引に失敗しました＞が表示される
				exec = 1;
			}
		}
		else if (DspSts == LCD_WMSG_ON) {
			exec = 1;
		}
		if (exec != 0) {
			LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);	// 残額不足表示消去用のﾀｲﾏｰｷｬﾝｾﾙ
			op_SuicaFusokuOff();							// 現在表示中のﾒｯｾｰｼﾞを消去
			dsp_change = 0;
		}
		break;

	default :
		break;
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_Cyclic_Disp
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:ｺｰﾙ元のﾌｪｰｽﾞ
///	@param[in]		e_pram_set		:電子決済使用設定
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			構造体名などはSuicaのOpe_TimeOut_10()のまま
///					(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/03/15<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ope_TimeOut_Cyclic_Disp(uchar ope_faze, ushort e_pram_set)
{
	switch (ope_faze) {
	case	0:
		if (e_pram_set == 1) {
		// 電子ﾏﾈｰ使用可能設定？
			op_mod01_dsp_sub();
			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]]);	// 6行目に画面切替用表示をする
			Lagtim(OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[1] * 50));			// 画面切替用ﾀｲﾏｰ起動(サイクリック表示タイマー)
		}
		break;

	case	2:
		if (OPECTL.op_faz == 8) {
		// ｸﾚｼﾞｯﾄ使用で電子媒体停止中の場合は処理をせずに抜ける
			break;
		}

		if (EcFirst_Pay_Dsp() != 0) {
		// 初回表示（料金表示）
			if (!Ex_portFlag[EXPORT_CHGNEND]) {
			// ～が利用できます表示かつ釣銭不足でない時の場合
				dspCyclicMsgRewrite(4);
				Lagtim(OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50));	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
			}
			if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9) {
			// 取消し処理実行中の場合
				if (Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL){
				// リーダが有効かつ最後に送信したのが受付許可の場合
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);										// 利用を不可にする
				}
			}
		}
		break;

	default :
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_Recept_Send
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:ｺｰﾙ元のﾌｪｰｽﾞ
///	@param[in]		e_pram_set		:電子決済使用設定
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			構造体名などはSuicaのOpe_TimeOut_11()のまま
///					(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/03/15<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ope_TimeOut_Recept_Send( uchar ope_faze, ushort  e_pram_set )
{
// NOTE:このタイマーは受付許可送信専用とし、リーダー制御内部でのみ使用します。
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if (timer_recept_send_busy == 2) {
//	// 時間内に状態データの受信が無かった場合再送する
//		timer_recept_send_busy = 0;
//		if ((Suica_Rec.suica_err_event.BYTE & 0xFB) == 0) {		// 開局異常以外のエラーが無ければ再送
//			EcSendCtrlEnableData();
//		}
//		return;
//	}
//	if (timer_recept_send_busy != 1) {
//	// 送信要求していないなら何もしない
//		return;
//	}
//	timer_recept_send_busy = 0;	// 精算中のec受入可送信タイマー停止中
	if (ECCTL.timer_recept_send_busy == 2) {
	// 時間内に状態データの受信が無かった場合再送する
		ECCTL.timer_recept_send_busy = 0;
		if ((Suica_Rec.suica_err_event.BYTE & 0xFB) == 0) {		// 開局異常以外のエラーが無ければ再送
			EcSendCtrlEnableData();
		}
		return;
	}
	if (ECCTL.timer_recept_send_busy != 1) {
	// 送信要求していないなら何もしない
		return;
	}
	ECCTL.timer_recept_send_busy = 0;	// 精算中のec受入可送信タイマー停止中
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810104 GG119202(S) 制御データ（受付許可）の応答待ちタイマを止める
	if ((Suica_Rec.suica_err_event.BYTE & 0xFB) != 0) {
		// 受付許可が送信できない状態の場合はタイマを停止する
		LagCan500ms(LAG500_SUICA_NO_RESPONSE_TIMER);
		// リトライカウント数をタイマが停止したタイミングで初期化する
		Status_Retry_Count_OK = 0;		//ﾘﾄﾗｲｶｳﾝﾄ数を初期化
		return;
	}
// MH810104 GG119202(E) 制御データ（受付許可）の応答待ちタイマを止める
	switch (ope_faze) {
	case	0:
// MH810103 MH321800(S) 未了残高照会タイムアウト画面が消えない
		if (OPECTL.Mnt_mod != 0) {
			// メンテ画面は一旦、制御データ（受付許可）を送信する
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);										// リーダ利用を可にする
			ECCTL.timer_recept_send_busy = 2;
			break;
		}
		// no break
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
// MH810103 GG116201(S) 受付許可が送信されない不具合修正
	case	1:
// MH810103 MH321800(E) 受付許可が送信されない不具合修正
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
		// まだ直前取引データ処理をしていなければ受付可を送信しない
		// 直前取引データ処理（ログ登録）が完了したら受付可を送信する
		if( ac_flg.ec_recv_deemed_fg ){
			break;
		}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810104 GG119201(S) リーダ直取中は制御データ（受付許可）を送信しない
		if( EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
			// 直前取引要求中は無視する
			break;
		}
// MH810104 GG119201(E) リーダ直取中は制御データ（受付許可）を送信しない
		if (!Suica_Rec.Data.BIT.CTRL) {
		// 取引キャンセル後の制御ﾃﾞｰﾀ送信
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);									// リーダ利用を可にする
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			timer_recept_send_busy = 2;
			ECCTL.timer_recept_send_busy = 2;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		}
		break;

	case	2:
		if (!Suica_Rec.Data.BIT.CTRL_MIRYO	&&									// 精算未了状態で無い場合で かつ
// MH810104 GG119202(S) 制御データ（受付許可）の応答待ちタイマを止める
			!Suica_Rec.Data.BIT.MIRYO_TIMEOUT &&								// 未了タイムアウト発生していない
// MH810104 GG119202(E) 制御データ（受付許可）の応答待ちタイマを止める
			!Suica_Rec.Data.BIT.CTRL		&&									// 制御ﾃﾞｰﾀが不可 かつ
			OPECTL.op_faz != 3				&&									// 取消しボタン押下時で無い かつ
			ryo_buf.zankin != 0L) {												// 残額がある場合
			if (OPECTL.op_faz == 9) {
				break;
			}
			if (OPECTL.op_faz == 8) {
			// Suica停止待ち合わせ時か
				break;
			}
			if (ope_faze == 22) {
			} else {
				if (MifStat == MIF_WROTE_FAIL) {
				// Mifare書込み失敗
					break;
				}
			}
// MH810104 GG119202(S) 制御データ（受付許可）の応答待ちタイマを止める
			if (OPECTL.ChkPassSyu != 0) {
				break;
			}
// MH810104 GG119202(E) 制御データ（受付許可）の応答待ちタイマを止める
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//			if ( (!WAR_MONEY_CHECK || e_incnt) &&
//				 (!isEcEnabled(EC_CHECK_CREDIT)) ) {
//			// 入金あり or 電子マネー使用済み、かつ、クレジット精算不可
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			if (ENABLE_MULTISETTLE() == 0) {
			if (ENABLE_MULTISETTLE() == 0 && 					// 利用可能ブランドなし
				check_enable_multisettle() <= 1) {				// マルチブランドではない
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			// 決済可能なブランドなしの場合は受付許可を送信しない
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
				break;
			}
			if( Suica_Rec.Data.BIT.CTRL_CARD ){
			// ｶｰﾄﾞ差込中は送信しない
				break;
			}
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
			// まだ直前取引データ処理をしていなければ受付可を送信しない
			// 直前取引データ処理（ログ登録）が完了したら受付可を送信する
			if( ac_flg.ec_recv_deemed_fg ){
				break;
			}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810104 GG119201(S) リーダ直取中は制御データ（受付許可）を送信しない
			if( EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
				// 直前取引要求中は無視する
				break;
			}
// MH810104 GG119201(E) リーダ直取中は制御データ（受付許可）を送信しない
// MH810104 GG119202(S) みなし休業になるとき取引終了が出ない
			if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT)) {
				// みなし休業時は無視する
				break;
			}
// MH810104 GG119202(E) みなし休業になるとき取引終了が出ない
// MH810105(S) MH364301 QRコード決済対応
			if (isEC_PAY_CANCEL()) {
				// 精算中止、または、休業による
				// とりけしボタン押下待ち中はリーダ利用不可
				break;
			}
// MH810105(E) MH364301 QRコード決済対応
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
			if (OPECTL.f_rtm_remote_pay_flg != 0) {
				// 遠隔精算開始受付中
				break;
			}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);									// リーダ利用を可にする
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			timer_recept_send_busy = 2;
			ECCTL.timer_recept_send_busy = 2;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			w_settlement = 0;													// 残高不足時の精算額（０時は残高不足でないを意味する）
		}
		break;

// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
	case	100:
		// 休業状態は一旦、制御データ（受付許可）を送信する
		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);										// リーダ利用を可にする
		ECCTL.timer_recept_send_busy = 2;
		break;
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない

	default :
		break;
	}
// 受付許可送信後、応答監視する
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if (timer_recept_send_busy == 2) {
	if (ECCTL.timer_recept_send_busy == 2) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		Lagtim(OPETCBNO, TIMERNO_EC_RECEPT_SEND, 5*50);
	}
// MH810104 GG119201(S) タイマを停止した際にリトライカウント数を初期化する
	else {
		// 受付許可が送信できない状態の場合はタイマを停止する
		LagCan500ms(LAG500_SUICA_NO_RESPONSE_TIMER);
		// リトライカウント数をタイマが停止したタイミングで初期化する
		Status_Retry_Count_OK = 0;												//ﾘﾄﾗｲｶｳﾝﾄ数を初期化
	}
// MH810104 GG119201(E) タイマを停止した際にリトライカウント数を初期化する
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_enable_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			snd_ctrl_timeout2()を参考
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_enable_timeout(void)
{
	ushort	tmp = 0;

	LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );				/* 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */

	// 最後に送信したデータが受付禁止の場合はタイムアウト処理をやらない
	if( Suica_Rec.Data.BIT.OPE_CTRL == 0 ){
		return;
	}

	if (Suica_Rec.Data.BIT.CTRL != 0) {
	/* 状態が受付可の場合 */
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* 状態ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
		Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;		/* 状態ﾃﾞｰﾀ受信ﾌﾗｸﾞON */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		EC_STATUS_DATA.StatusInfo.ReceptStatus = 0;				/* 状態ﾃﾞｰﾀ内容に受付可ｾｯﾄ */
		EcRecvStatusData(&OPECTL.Ope_mod);						/* 状態データ受信処理実施 */
	} else {

		Suica_Rec.suica_err_event.BIT.OPEN = 1;					/* 開局異常ﾌﾗｸﾞをｾｯﾄ */
		if (ECCTL.not_ready_timer == 0) {						// リーダー受付許可制御失敗未発生
			if (ECCTL.pon_err_alm_regist != 0) {
			// 起動時以降のリーダー受付許可制御失敗
				alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 1);			// ｱﾗｰﾑﾛｸﾞ登録（登録）
			}
			ECCTL.not_ready_timer = 1;
			tmp = (ushort)prm_get(COM_PRM, S_ECR, 6, 3, 1);
			if(tmp == 0){										// 0秒設定は70秒として扱う
				tmp = 70;
			} else if(tmp < 10){								// 10秒未満は10秒として扱う
				tmp = 10;
			}
			tmp = tmp - EC_ENABLE_WAIT_TIME;					// 受付許可タイムアウトと併せて,1秒単位(10秒～999秒)
			LagTim500ms(LAG500_EC_NOT_READY_TIMER, (short)((tmp * 2) + 1), not_open_timeout);	// 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏ開始
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_disable_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			snd_ctrl_timeout3()を参考
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_disable_timeout(void)
{
	uchar	Retry_count = 0;
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
	uchar	cmd_typ = ECCTL.prohibit_snd_bsy;
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応

// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
	if( cmd_typ ){											// 制御データ（受付禁止）応答監視中
		ECCTL.prohibit_snd_bsy = 0;							// 制御データ（受付禁止）応答監視終了
	}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
	LagCan500ms(LAG500_SUICA_NONE_TIMER);					/* 受付禁止制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
	if ((cmd_typ & S_CNTL_MIRYO_ZANDAKA_END_BIT) != 0) {
		// 未了残高照会キャンセルの応答待ちタイムアウトでみなし決済とするため、ここで抜ける
		return;
	}
// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う
	if ((Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 1)
	&&	(EC_STATUS_DATA.StatusInfo.Running != 0)) {
	// リーダから応答がなく、未了タイムアウトした場合
	// 精算機のタイマーにて未了タイムアウトになった後の受付禁止の応答が実行中の場合は永久に受付禁止を繰り返す。
	// ただし、受付禁止の応答が実行中BITが立たないデータを受信すれば、通常のリトライ処理に移行する。
		EC_STATUS_DATA.StatusInfo.Running = 0;				// 不可の応答で実行中を受信し、再度受付禁止を送信する場合は実行中ﾌﾗｸﾞを落とす
		Status_Retry_Count_NG = 0;							// 実行中で返ってきた場合にはリトライカウントをクリアする。
		Ec_Pri_Data_Snd(S_CNTL_DATA, 0);					/* 制御ﾃﾞｰﾀ（受付禁止）を送信 */
		return;
	}

	// 最後に送信したデータが受付許可の場合はタイムアウト処理をやらない
	if (Suica_Rec.Data.BIT.OPE_CTRL == 1) {
		return;
	}

	if ((Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR)) {
	/* リーダが受付可状態？ */
		Retry_count = EC_DISABLE_RETRY_CNT;
		if (Status_Retry_Count_NG < Retry_count) {
		/* 現在のｶｳﾝﾄ数が設定値以下？ */
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);				/* 制御ﾃﾞｰﾀ（受付禁止）を送信 */
			if( ECCTL.transact_end_snd ){					// 応答待ち中に取引終了送信要求あり？
				cmd_typ = ECCTL.transact_end_snd;			// 取引終了を送信する
				Status_Retry_Count_NG = 0;					// ﾘﾄﾗｲｶｳﾝﾄ=0から再送信
			}else if( cmd_typ ){							// 受付禁止応答監視中？
				cmd_typ &= 0xfe;							// 指令データを受付禁止(受付許可/禁止bitをOFF(禁止))とする
			}	// cmd_typ = 0 の時は受付禁止のみの制御データを送信
			Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );		// 制御ﾃﾞｰﾀ（受付禁止）を送信
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
			Status_Retry_Count_NG++;						/* ﾘﾄﾗｲｶｳﾝﾄをｲﾝｸﾘﾒﾝﾄ */
			return;
		} else {
		// リトライオーバー
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//			snd_no_response_timeout();
			if( ECCTL.transact_end_snd ){					// 応答待ち中に取引終了送信要求あり？
				cmd_typ = ECCTL.transact_end_snd;			// 取引終了を送信する
				Status_Retry_Count_NG = 0;					// ﾘﾄﾗｲｶｳﾝﾄ=0から再送信
				Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );	// 制御ﾃﾞｰﾀ（受付禁止）を送信
			}else{
				snd_no_response_timeout();
			}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
		}
	} else {
	/* リーダが受付不可状態？ */
		snd_no_response_timeout();
	}
	Status_Retry_Count_NG = 0;								/* ﾘﾄﾗｲｶｳﾝﾄをｸﾘｱ */
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_no_response_timeout(未了タイムアウトが無応答時の動作)
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			No_Responce_Timeout()を参考
///					構造体名などはSuicaのまま(従来のSuica削除時に変更すること)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_no_response_timeout(void)
{
	Suica_Rec.Data.BIT.CTRL = 0;
	if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER && Suica_Rec.Data.BIT.CTRL_MIRYO) {
	/* 未了状態 */
		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* 未了状態解除 */
// MH810103 GG119202(S) 未了仕様変更対応
//		if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//			// 未了確定受信なしの場合はカードIDを'?'で埋める
//			Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;	// ブランド番号
//			Ec_Settlement_Res.settlement_data = Product_Select_Data;	// 決済額
//			memset(Ec_Settlement_Res.Card_ID, '\?', sizeof(Ec_Settlement_Res.Card_ID));
//		}
//		else {
//			Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;		// 未了確定受信フラグクリア
//		}
		Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// 未了確定(状態データ)受信をクリア
// MH810103 GG119202(E) 未了仕様変更対応
// MH810105 GG119202(S) 未了確定の直取データ受信時にR0176を登録する
//// MH810103 GG119202(S) 未了中に異常データ受信でとりけしボタン押下待ちにならない
//		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// モニタ登録
//// MH810103 GG119202(E) 未了中に異常データ受信でとりけしボタン押下待ちにならない
// MH810105 GG119202(E) 未了確定の直取データ受信時にR0176を登録する
	}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;		/* 状態ﾃﾞｰﾀ受信ﾌﾗｸﾞON */	
	Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;	/* 状態ﾃﾞｰﾀ受信ﾌﾗｸﾞON */	
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;			/* 状態ﾃﾞｰﾀ内容に受付不可ｾｯﾄ */
	if (Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {
	/* 決済中の受付禁止応答が無い場合 */
		EC_STATUS_DATA.StatusInfo.TradeCansel = 1;		/* 取引ｷｬﾝｾﾙﾌﾗｸﾞもON */
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;			// CRW-MJA精算中フラグクリア
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
	}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	Ec_Settlement_Sts = EC_SETT_STS_NONE;				/* 決済状態もOFFにしておく */
	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;			/* 決済状態もOFFにしておく */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	EcRecvStatusData(&OPECTL.Ope_mod);					/* 制御データ受信処理実施 */
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
	EC_STATUS_DATA.StatusInfo.TradeCansel = 0;			// 取引ｷｬﾝｾﾙﾌﾗｸﾞOFF
	ECCTL.transact_end_snd = 0;							// 取引終了送信要求なし状態とする
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
}

// MH810103 GG119202(S) 不要処理削除
////[]----------------------------------------------------------------------[]
/////	@brief			status_err_timeout
////[]----------------------------------------------------------------------[]
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			G.So
/////	@date			Create	:	2019/02/07<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	status_err_timeout(void)
//{
//	unsigned char wks = 0;
//
//	LagCan500ms(LAG500_SUICA_STATUS_TIMER);				/* CRW-MJA状態監視ﾀｲﾏﾘｾｯﾄ */
//	wks = EC_INIT_RETRY_CNT;
//	if (Retrycount < wks) {								/* 再初期化ﾘﾄﾗｲ回数ｵｰﾊﾞｰ？ */
//		suicatrb(ERR_SUICA_RECEIVE);					/* 初期化処理実行 */
//		Retrycount++;									/* ﾘﾄﾗｲｶｳﾝﾀの更新 */
//	}
//}
// MH810103 GG119202(E) 不要処理削除

//[]----------------------------------------------------------------------[]
///	@brief			not_open_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	not_open_timeout(void)
{
	LagCan500ms(LAG500_EC_NOT_READY_TIMER);					/* 受付許可制御ﾃﾞｰﾀ送信無応答ﾀｲﾏﾘｾｯﾄ */
	ECCTL.not_ready_timer = 0;
	if (ECCTL.pon_err_alm_regist == 0) {
		ECCTL.pon_err_alm_regist = 1;						// 以降、アラーム／エラー登録する
	// 起動後最初の受付可受信
		alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 1);				/* ｱﾗｰﾑﾛｸﾞ登録（登録）*/
	}
	err_chk((char)jvma_setup.mdl, ERR_EC_OPEN, 1, 0, 0);	/* ｴﾗｰﾛｸﾞ登録（登録）*/
}

//[]----------------------------------------------------------------------[]
///	@brief			ec_auto_cancel_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ec_auto_cancel_timeout(void)
{
	LagCan500ms(LAG500_EC_AUTO_CANCEL_TIMER);			// アラーム取引後の精算自動キャンセルタイマ
	queset(OPETCBNO, AUTO_CANCEL, 0, NULL);				// 自動精算中止
}

// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//[]----------------------------------------------------------------------[]
///	@brief			ec_check_cre_pay_limit
//[]----------------------------------------------------------------------[]
///	@return			ret			：TRUE	:クレジット精算可能(限度額以下)
///								：FALSE	:クレジット精算不可(限度額オーバー)
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	:	2020/05/14<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL	Ec_check_cre_pay_limit(void)
{
	ulong	limit = (ulong)prm_get( COM_PRM, S_CRE, 2, 6, 1 );

	if(( !limit ) || ( OPECTL.Ope_mod != 2 )){	// 精算限度額チェックしない？
		return TRUE;	// クレジット精算可能
	}
	if( ryo_buf.zankin <= limit ){	// 精算残高はクレジット精算限度額以下？
		return TRUE;	// クレジット精算可能(限度額以下)
	}

	return FALSE;	// クレジット精算不可(限度額オーバー)
}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応

//[]----------------------------------------------------------------------[]
///	@brief			状態データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	：現在のOpeﾌｪｰｽﾞ
///	@return			ret			：0固定
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	EcRecvStatusData(uchar *ope_faze)
{
	short	ret = 0;
	uchar	type = 0;
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
	uchar	cmd_typ = ECCTL.prohibit_snd_bsy;
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
//// MH810103 GG119202(S) カード処理中受信後の決済NG（304）の扱い
//	uchar	brand_index = 0;
//// MH810103 GG119202(E) カード処理中受信後の決済NG（304）の扱い
// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う

// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
	if( cmd_typ ){														// 制御データ（受付禁止）応答監視中
		ECCTL.prohibit_snd_bsy = 0;										// 制御データ（受付禁止）応答監視終了
	}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
// MH810103 GG119202(S) マルチブランド仕様変更対応
//	if (Suica_Rec.Data.BIT.CTRL == 0) {									// 受付不可を受けたとき
//		RecvBrandResTbl.res = EC_BRAND_UNSELECTED;						// 一旦、選択ブランドなしに戻し
//		RecvBrandResTbl.no = BRANDNO_UNKNOWN;							// 受付可後はブランド選択をやり直す
//	/*	Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;							// ブランド選択の「戻る」検出フラグ初期化	*/
//	}
// MH810103 GG119202(E) マルチブランド仕様変更対応
// MH810103 GG119201(S) リーダのキャンセル待ち中に精算機をメンテナンスモードに切り替えると、電源OFF/ONするまで駐車券が返却されない
//	if (OPECTL.Mnt_mod != 0) {
	// フラップ式では発生しないがゲート式と処理を合わせる
	if (OPECTL.Ope_mod == 0 && OPECTL.Mnt_mod != 0) {
// MH810103 GG119201(E) リーダのキャンセル待ち中に精算機をメンテナンスモードに切り替えると、電源OFF/ONするまで駐車券が返却されない
// MH810103 GG118809_GG118909(S) 決済リーダ排他処理考慮漏れ対応
		if (PRE_EC_STATUS_DATA.StatusInfo.ZandakaInquiry &&
			!EC_STATUS_DATA.StatusInfo.ZandakaInquiry) {
			edy_dsp.BIT.suica_zangaku_dsp = 0;
		}
// MH810103 GG118809_GG118909(E) 決済リーダ排他処理考慮漏れ対応
		if(Suica_Rec.Data.BIT.CTRL ||
// MH810103 GG119202(S) E3209が発生してしまう問題の対策
			Suica_Rec.Data.BIT.OPE_CTRL ||
// MH810103 GG119202(E) E3209が発生してしまう問題の対策
		   (!PRE_EC_STATUS_DATA.StatusInfo.RebootReq &&
			EC_STATUS_DATA.StatusInfo.RebootReq)) {
			// メンテナンス中に受付許可を受信した
			// 再起動要求bitが0->1に変化した
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);		// 受付禁止送信
		}
		if (EC_STATUS_DATA.StatusInfo.TradeCansel) {
			// メンテナンス中に取引キャンセルを受信した
			recv_unexpected_status_data();
		}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 0;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		return ret;
	}
	Ec_Status_Chg();

	// 再起動要求bitが0->1に変化したときのみ制御データで応答する
	if (!PRE_EC_STATUS_DATA.StatusInfo.RebootReq &&
		EC_STATUS_DATA.StatusInfo.RebootReq) {
		type = Suica_Rec.Data.BIT.CTRL;				// 受付許可／禁止をセット
		Ec_Pri_Data_Snd(S_CNTL_DATA, type);			// 制御データ送信
	}

	switch (*ope_faze) {
// MH810103(s) 電子マネー対応 待機画面での残高照会可否
// 	case	0:	// 待機画面
// 	case	1:	// 車室番号入力画面
// 		if (Suica_Rec.Data.BIT.CTRL == 0 && EC_STATUS_DATA.StatusInfo.ReceptStatus) {
// 			if (OPECTL.op_faz == 8) { 										// 電子媒体停止処理中
// 				Op_StopModuleWait_sub(0);									// CRW-MJA停止済み処理
// 			}
// 			EcSendCtrlEnableData();											// 制御データ(受付許可)送信
// 		}
// // GG119202(S) 制御データ（受付禁止）の応答待ち対応
// 		else if( Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// 受付許可受信かつ最後に送信したのは受付禁止？
// 			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// 制御データ(受付禁止)送信
// 		}
// // GG119202(E) 制御データ（受付禁止）の応答待ち対応
// 		break;
	case	0:	// 待機画面
		if (!isEcEmoneyEnabled(0, 0)) {
			// 電子マネーなし時は受付許可送信後、受付可になるまで受付許可を送信する
			if (Suica_Rec.Data.BIT.CTRL == 0) {
				EcSendCtrlEnableData();										// 制御データ(受付許可)送信
			}
// MH810103(s) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
			// 不可送信
			lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,1);
// MH810103(e) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
		}
		else {
			if(OPECTL.lcd_query_onoff != 0){								// 待機画面での残高照会可否(0:NG/1:OK/2:その他)
				if (Suica_Rec.Data.BIT.CTRL == 0) {
					EcSendCtrlEnableData();										// 制御データ(受付許可)送信
				}
// MH810103(s) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
				else{
					// 可能送信
					lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,0);
				}
// MH810103(e) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
			}else{
				if (Suica_Rec.Data.BIT.CTRL == 1 || Suica_Rec.Data.BIT.OPE_CTRL == 1) {
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// 制御データ(受付禁止)送信
				}
// MH810103(s) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
				// 不可送信
				lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,1);
// MH810103(e) 電子マネー対応 #5519 待機画面でVP6800へ不許可を送信しないようにする
			}
		}
		break;
	case	1:	// 車室番号入力画面
// MH810104 GG119201(S) 直前取引データ処理中は受付可を受信したら受付禁止を送信する
		if( Suica_Rec.Data.BIT.CTRL &&
			(ac_flg.ec_recv_deemed_fg || EC_STATUS_DATA.StatusInfo.LastSetteRunning)){
			// 直取データをまだ処理しておらず、受付可を受信したら受付禁止を送信する
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// 制御データ(受付禁止)送信
			break;
		}
// MH810104 GG119201(E) 直前取引データ処理中は受付可を受信したら受付禁止を送信する
		if (Suica_Rec.Data.BIT.CTRL == 0 && EC_STATUS_DATA.StatusInfo.ReceptStatus) {
			if (OPECTL.op_faz == 8) { 										// 電子媒体停止処理中
				Op_StopModuleWait_sub(0);									// CRW-MJA停止済み処理
			}
			EcSendCtrlEnableData();											// 制御データ(受付許可)送信
		}
		else if( Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// 受付許可受信かつ最後に送信したのは受付禁止？
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// 制御データ(受付禁止)送信
		}
		break;
// MH810103(e) 電子マネー対応 待機画面での残高照会可否

	case	2:		// 精算中
		if (!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// 精算未了状態で無い場合のみ処理を実施
			if (Suica_Rec.Data.BIT.CTRL) {									// 制御ﾃﾞｰﾀが可状態の時は
		// 受付可－－－－－－－→
				if (OPECTL.op_faz == 3 ||									// 電子媒体停止待ち合わせ中？
					OPECTL.op_faz == 8) {
					// 電子媒体停止待ち合わせ中での受付可受信時は、ﾌｪｰｽﾞ合わせの為に受付禁止を送信する。
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
					break;
				}
// MH810104 GG119201(S) 直前取引データ処理中は受付可を受信したら受付禁止を送信する
				if( ac_flg.ec_recv_deemed_fg || EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
					// 直取データをまだ処理しておらず、受付可を受信したら受付禁止を送信する
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );						// 制御データ(受付禁止)送信
					break;
				}
// MH810104 GG119201(E) 直前取引データ処理中は受付可を受信したら受付禁止を送信する
				if (OPECTL.op_faz >= 9 || 									// 精算中止時の決済リーダ停止待ち合わせ時(正常に停止していない場合)か
					EC_STATUS_DATA.StatusInfo.Running) {					// 受信ステータスが実行中
					break;
				}
				if (EC_STATUS_DATA.StatusInfo.ReqTimerUpdate != 0) {
				// ※次にペアとなる状態データが再度来るので、商品先選択タイマー更新依頼時の状態データは受け捨てする
					break;
				}
				if ((Suica_Rec.Data.BIT.CTRL == 1)
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
//				&&	(Suica_Rec.Data.BIT.PRE_CTRL == Suica_Rec.Data.BIT.CTRL)) {
//				// ※受信したﾘｰﾀﾞ状態が受付可 && 変化なし→選択商品データを送信しない
				&& (( Suica_Rec.Data.BIT.PRE_CTRL == Suica_Rec.Data.BIT.CTRL) ||
					( !Suica_Rec.Data.BIT.OPE_CTRL ))){
					// ※受信したﾘｰﾀﾞ状態が受付可 && (変化なし || 最後に送信したのは受付禁止)→選択商品データを送信しない
					if( !Suica_Rec.Data.BIT.OPE_CTRL ){						// 最後に送信したのは受付禁止
						// 受付禁止を送信したのに受付可能を受信したので受付禁止を再送する
						if( cmd_typ & 0x80 ){								// 最後に送信した受付禁止の取引終了BitON？
							Suica_Rec.Data.BIT.SEND_CTRL80 = 0;				// 取引終了データ送信状態フラグクリア
						}
						if( ECCTL.transact_end_snd ){						// 応答待ち中に取引終了送信要求あり？
							cmd_typ = ECCTL.transact_end_snd;				// 取引終了を送信する
						}else if( cmd_typ ){								// 受付禁止応答監視中？
							cmd_typ &= 0xfe;								// 指令データを受付禁止(受付許可/禁止bitをOFF(禁止))とする
						}	// cmd_typ = 0 の時は受付禁止のみの制御データを送信
						Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );			// 制御ﾃﾞｰﾀ（受付禁止）を送信
					}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応
					break;
				}

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//				if (WAR_MONEY_CHECK && !e_incnt) {
//				// 未入金・電子マネー未使用
//					if (OPECTL.op_faz < 2) {									// 精算中であること
//// GG119202(S) クレジットカード精算限度額設定対応
////						if ((ENABLE_MULTISETTLE() == 0) || 						// 単一決済 ||
////							(RecvBrandResTbl.res == EC_BRAND_SELECTED)) {		// ブランド選択済みであること
//						if( ENABLE_MULTISETTLE() == 0 ){						// 決済可能ブランドなし
//							Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// 制御データ(受付禁止)送信
//						}
//						else if( ENABLE_MULTISETTLE() == 1 ){ 					// 単一決済有効
//// GG119202(E) クレジットカード精算限度額設定対応
//							if (w_settlement != 0) {							// 残高不足時の引落し時
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// 残高を選択商品ﾃﾞｰﾀとして送信する
//							} else {											// 通常時
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);				// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
//								w_settlement = 0;								// 選択商品データ編集領域をｸﾘｱする
//							}
//						} else {
//							// 複数決済、かつ、ブランド未選択
//							Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// ブランド選択データを送信する
//						}
//					}
//				} else {
//				// 入金あり
//					if (OPECTL.op_faz < 2) {
//						if (isEcReady(EC_CHECK_CREDIT)) {						// クレジット決済可能
//							if( RecvBrandResTbl.no != BRANDNO_CREDIT ){			// ブランド選択でクレジットを選択していない時
//								RecvBrandResTbl.res = EC_BRAND_UNSELECTED;		// 何もブランド選択されていない事にする(入金ありで電子マネーはありえない為)
//							}
//							if (w_settlement != 0) {							// 残高不足時の引落し時
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// 残高（10円未満切捨て）を選択商品ﾃﾞｰﾀとして送信する
//							} else {											// 通常時
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);				// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
//								w_settlement = 0;								// 選択商品データ編集領域をｸﾘｱする
//							}
//						}
//					}
//				}
				if (OPECTL.op_faz < 2) {										// 精算中であること
					// 画面遷移先は下記の通り
					// 1.決済NG受信（残高不足など）（カード読み取りタイムアウトは除く）→カード読み取り待ち画面
					// 2.サービス券等挿入→カード読み取り待ち画面
					// 3.現金投入→カード読み取り待ち画面
					//   ※電子マネーは現金併用不可のため支払い選択画面に戻る
					// 4.カード読み取り待ちタイムアウト→支払い選択画面
					// 5.キャンセルボタン押下→支払い選択画面
					switch(check_enable_multisettle()) {
					default:													// 複数
						if( RecvBrandResTbl.res != EC_BRAND_UNSELECTED ) {		// ブランド選択済み
							if (! ec_check_valid_brand(RecvBrandResTbl.no)) {
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
								EcEnableNyukin();
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
								EcBrandClear();									// 選択解除
							}
						}
						if( RecvBrandResTbl.res == EC_BRAND_UNSELECTED ) {		// ブランド選択でクレジットを選択していない時
							// 複数決済、かつ、ブランド未選択
							Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// ブランド選択データを送信する
							break;
						}
					//	break;
					case	1:													// 単一
						{
						// 残額不足は決済NGになるため、残額を引き去る機能は削除する
// MH810103 GG119202(S) 0円の選択商品データを送信してしまう
							if (RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// ブランド未選択状態
								ec_set_brand_selected();
							}
// MH810103 GG119202(E) 0円の選択商品データを送信してしまう
							Ec_Pri_Data_Snd(S_SELECT_DATA, 0);					// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
							w_settlement = 0;									// 選択商品データ編集領域をｸﾘｱする
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
							op_EcEmoney_Anm();
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
						}
						break;
					case	0:													// ブランドなし
						Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );						// 制御データ(受付禁止)送信
						break;
					}
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			}
			else {															// 制御ﾃﾞｰﾀが不可状態の時で
		// 受付不可－－－－－－→
				if (EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// 受付不可データ及び
					EC_STATUS_DATA.StatusInfo.TradeCansel) {				// 取引ｷｬﾝｾﾙ受付データを受信時
// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
					// 未了残高照会中に障害発生した場合はEcDeemedSettlement()が呼ばれて
					// 設定50-0014に従うため、下記処理は削除する
//					if ( dsp_change == 1 ) {								// 未了残高終了通知を未送信の受付不可取引キャンセル受信時
//					// ※未了タイムアウト時の受付禁止要求に対して無応答の場合、snd_no_response_timeout()から呼ばれる
//						op_SuicaFusokuOff();								// 精算未了時の反転ﾒｯｾｰｼﾞ表示を消去する
//						dsp_change = 0;
//						miryo_timeout_after_mif(0);
//// MH810104 GG119201(S) 現金投入と同時にカードタッチ→未了残高照会中にみなし決済受信で画面がロックする
//						if( ryo_buf.nyukin && OPECTL.credit_in_mony ){		// 入金ありで電子マネーとのすれ違い入金の場合
//							// 入金額 >= 駐車料金の状態で未了タイムアウトした場合は
//							// 精算完了として扱い、とりけしボタン押下待ちにしない
//							in_mony ( COIN_EVT, 0 );						// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//							OPECTL.credit_in_mony = 0;						// すれ違いﾌﾗｸﾞｸﾘｱ
//						}
//// MH810104 GG119201(E) 現金投入と同時にカードタッチ→未了残高照会中にみなし決済受信で画面がロックする
//						nyukin_delay_check(nyukin_delay, delay_count);		// 保留中のｲﾍﾞﾝﾄがあれば再postする
//						ec_miryo_timeout_after_disp(&Ec_Settlement_Res);	// 未了タイムアウト後の表示を行う
//						Lagcan( OPETCBNO, 1 );										// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
//// MH810103 GG119202(S) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
////						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
//// MH810103 GG119202(E) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
//// MH810103 GG119202(S) カード処理中受信後の決済NG（304）の扱い
//						brand_index = convert_brandno(Ec_Settlement_Res.brand_no);
//						if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//							// 選択商品データで送信したブランドを決済結果で受信した事にする
//							Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
//							brand_index = convert_brandno(ECCTL.Product_Select_Brand);
//						}
//						ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// アラーム取引集計
//						ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// アラームレシート印字要求
//						EcAlarmLog_Regist(&Ec_Settlement_Res);							// 処理未了取引ログ登録
//// MH810103 GG119202(E) カード処理中受信後の決済NG（304）の扱い
//					}
// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う
// MH810103 GG119202(S) 未了仕様変更対応
//					else if ( dsp_change == 2 ){							// 未了残高終了通知を送信した後の受付不可取引キャンセル受信時
//						nyukin_delay_check(nyukin_delay, delay_count);		// 保留中のｲﾍﾞﾝﾄがあれば再postする
//					}
// MH810103 GG119202(E) 未了仕様変更対応
					if (Suica_Rec.Data.BIT.SEND_CTRL80 == 0) {
						Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);					// 制御ﾃﾞｰﾀ（取引終了）を送信
					}
// MH810103 GG119202(S) カード抜き取り待ちメッセージが表示されない
					if (OPECTL.op_faz < 2) {								// 精算中
						// カードありの場合はカード抜き取り待ちを行う
						ec_start_removal_wait();
					}
// MH810103 GG119202(E) カード抜き取り待ちメッセージが表示されない
				}
// MH810103 GG119202(S) 制御データ（受付禁止）の応答待ち対応
				else if( !Suica_Rec.Data.BIT.OPE_CTRL ){					// 最後に送信したのは受付禁止？
					if( ECCTL.transact_end_snd ){							// 応答待ち中に取引終了送信要求あり？
						Ec_Pri_Data_Snd( S_CNTL_DATA, ECCTL.transact_end_snd );	// 取引終了を送信する
					}
				}
// MH810103 GG119202(E) 制御データ（受付禁止）の応答待ち対応

				// 保留していたイベントを再開する
				if(OPECTL.holdPayoutForEcStop) {
					OPECTL.holdPayoutForEcStop = 0;
					queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
				}

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
				if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&
					Suica_Rec.Data.BIT.BRAND_CAN != 0) {
				// ブランド選択中に受付不可を受信したとき、精算機からの受付禁止に由来するとき
				// ブランド選択結果の通知が来ないのでここで状態をクリアする
					Suica_Rec.Data.BIT.BRAND_SEL = 0;
					Suica_Rec.Data.BIT.BRAND_CAN = 0;
					EcBrandClear();
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

				if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9 || time_out) {	// 精算中止ﾎﾞﾀﾝ押下でSuica停止待ち合わせ時は
					Op_Cansel_Wait_sub(0);									// CRW-MJA停止済みとして停止判断を行う
					break;
				}

				if (OPECTL.op_faz == 8) {									// 電子媒体停止待ち合わせ時は
					if (Op_StopModuleWait_sub(0) == 1) {					// CRW-MJA停止済みとして停止判断を行う
						ret = 0;
						break;
					}
				}

				// 状態データ（受付不可）受信時に制御データ（受付許可）を200ms後に送信する
				// 取引キャンセル、または、3データ(状態、決済状態、決済結果)受信時は制御データ（受付許可）を送信しない
				if (EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// ﾘｰﾀﾞｰが受付不可状態となった場合で
					!EC_STATUS_DATA.StatusInfo.TradeCansel &&				// 取引ｷｬﾝｾﾙなし
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					!Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA) {	// 決済結果データを未処理の場合は、再活性化処理のタイマーをスタート
					!Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA) {	// 決済結果データを未処理の場合は、再活性化処理のタイマーをスタート
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
																			// 受付許可電文を送信するかどうかはタイムアウト先で判定する
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//					if (!time_out && Suica_Rec.Data.BIT.CTRL_CARD == 0) {	// 待機復帰タイマータイムアウト以前、抜き取るカードなしなら
					if (!time_out && Suica_Rec.Data.BIT.CTRL_CARD == 0 &&	// 待機復帰タイマータイムアウト以前、抜き取るカードなしなら
						Suica_Rec.Data.BIT.BRAND_SEL == 0) {				// ブランド選択中の受付不可変化は待ち合わせる
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810100(S) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
//						EcSendCtrlEnableData();							// 制御データ(受付許可)送信
						if( lcdbm_Flag_QRuse_ICCuse == 0 ){					// QR利用中ではない(QR利用中は入庫情報受信で受付許可を送信する)
							EcSendCtrlEnableData();							// 制御データ(受付許可)送信
						}
// MH810100(E) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
					}
				}
			}
		} else {
		// 未了発生－－－－－－→
			if (EC_STATUS_DATA.StatusInfo.MiryoZandakaStop != 0) {			// 未了残高中止受信
				// 未了残高中止受信時は自動とりけしとする
				ec_auto_cancel_timeout();
				break;
			}
			if (MIFARE_CARD_DoesUse) {										// Mifareが有効な場合
				LagCan500ms(LAG500_MIF_LED_ONOFF);							// Mifare再有効化ﾀｲﾏｰｽﾄｯﾌﾟ
				op_MifareStop_with_LED();									// Mifare無効
			}
			Lagcan(OPETCBNO, 1);											// ｵﾍﾟﾚｰｼｮﾝ関連ﾀｲﾏｰﾘｾｯﾄ
			Lagcan(OPETCBNO, 8);											// 入金後戻り判定ﾀｲﾏｰﾘｾｯﾄ
			Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);						// サイクリック表示タイマーﾘｾｯﾄ
// MH810103 GG119202(S) 未了発生の案内表示処理修正
//			dsp_change = 1;													// 未了ｱﾗｰﾑ表示中ﾌﾗｸﾞｾｯﾄ
// MH810103 GG119202(E) 未了発生の案内表示処理修正
// MH810103 GG119202(S) 未了仕様変更対応
//			lcd_wmsg_dsp_elec(1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF);		// "もう一度、同じカードを"
			if (EC_STATUS_DATA.StatusInfo.MiryoConfirm &&
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
//				Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {					// 未了発生中に未了確定(状態データ)受信
				Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0 &&					// 未了発生中に未了確定(状態データ)受信
				isEcBrandNoEMoney(RecvBrandResTbl.no, 1)) {					// 残高照会可能なブランド
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
				op_SuicaFusokuOff();
// MH810103(s) 電子マネー対応 未了通知
//				lcd_wmsg_dsp_elec( 1, ERR_CHR[20], ERR_CHR[53], 0, 0, COLOR_RED, LCD_BLINK_OFF); // "カード取引に失敗しました"
//																								 // "引き続き未了残高照会を行います"
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x05);	// 未了(決済タイムアウト) // "カード取引に失敗しました"
																							 // "引き続き未了残高照会を行います"
// MH810103(e) 電子マネー対応 未了通知
				Suica_Rec.Data.BIT.MIRYO_CONFIRM = 1;						// 未了確定(状態データ)受信済みをセット
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//				EcDeemedLog.MiryoFlg = 2;									// 未了残高照会中
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			}
// MH810103 GG119202(S) 未了中に異常データ受信でとりけしボタン押下待ちにならない
//			else if (EC_STATUS_DATA.StatusInfo.MiryoStatus) {				// 処理未了受信
			else if (EC_STATUS_DATA.StatusInfo.MiryoStatus &&				// 処理未了受信
// MH810103 GG119202(S) 状態データ（未了確定）受信のデータ待ち合わせ処理修正
					 isEC_CONF_MIRYO() == 0 &&								// 未了確定未発生
// MH810103 GG119202(E) 状態データ（未了確定）受信のデータ待ち合わせ処理修正
// MH810103 GG119202(S) 未了発生の案内表示処理修正
					 dsp_change == 0 &&										// 未了ｱﾗｰﾑ表示中ﾌﾗｸﾞ未ｾｯﾄ
// MH810103 GG119202(E) 未了発生の案内表示処理修正
					 Suica_Rec.Data.BIT.MIRYO_TIMEOUT == 0) {				// 未了タイムアウト未発生
// MH810103 GG119202(E) 未了中に異常データ受信でとりけしボタン押下待ちにならない
// MH810103(s) 電子マネー対応 未了通知
//				lcd_wmsg_dsp_elec(1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF);	// "もう一度、同じカードを"
//																								// "タッチしてください"
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x03);	// 未了(再タッチ待ち) 	// "もう一度、同じカードを"
																							// "タッチしてください"
// MH810103(e) 電子マネー対応 未了通知
// MH810103 GG119202(S) 未了発生の案内表示処理修正
				dsp_change = 1;												// 未了ｱﾗｰﾑ表示中ﾌﾗｸﾞｾｯﾄ
// MH810103 GG119202(E) 未了発生の案内表示処理修正
			}
// MH810103 GG119202(E) 未了仕様変更対応
																								// "タッチしてください"
			if (dsp_fusoku) {												// 不足アラーム表示中？
				LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);				// 不足アラーム消去ﾀｲﾏｰを止める
			}
		}
		break;

	case	3:		// 精算完了
		if (Suica_Rec.Data.BIT.SEND_CTRL80 == 0) {
		// 送信していなければ
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);								// 制御ﾃﾞｰﾀ（取引終了）を送信する
		} else if (Suica_Rec.Data.BIT.CTRL) {
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);								// 制御ﾃﾞｰﾀ（受付禁止）を送信する
		}
		if (EC_STATUS_DATA.StatusInfo.TradeCansel) {
			// 精算完了後に取引キャンセルを受信した
			recv_unexpected_status_data();
		}
		break;

// MH810103 GG119202(S) 残高照会ボタン非表示の暫定対策
	case	100:	// 休業
// MH810103 GG119202(S) E3209が発生してしまう問題の対策
// MH810103 GG118809_GG118909(S) 決済リーダ排他処理考慮漏れ対応
		if (PRE_EC_STATUS_DATA.StatusInfo.ZandakaInquiry &&
			!EC_STATUS_DATA.StatusInfo.ZandakaInquiry) {
			edy_dsp.BIT.suica_zangaku_dsp = 0;
		}
// MH810103 GG118809_GG118909(E) 決済リーダ排他処理考慮漏れ対応
		// 下記の処理は制御データ（受付許可）に対して状態データ（受付不可）を受信したときに
		// 何も送信しないため、LAG500_SUICA_NO_RESPONSE_TIMERのタイマを止める契機がなくなる
		// 電子マネーなし時は受付不可状態にする必要がないので、電子マネーあり／なしで処理を分ける
//		if (Suica_Rec.Data.BIT.CTRL == 1 && !EC_STATUS_DATA.StatusInfo.ReceptStatus) {
		if (!isEcEmoneyEnabled(0, 0)) {
			// 電子マネーなし時は受付許可送信後、受付可になるまで受付許可を送信する
			if (Suica_Rec.Data.BIT.CTRL == 0) {
				EcSendCtrlEnableData();										// 制御データ(受付許可)送信
			}
		}
		else {
			// 電子マネーあり時は受付許可送信後、受付不可になるまで受付禁止を送信する
			if (Suica_Rec.Data.BIT.CTRL == 1 || Suica_Rec.Data.BIT.OPE_CTRL == 1) {
// MH810103 GG119202(E) E3209が発生してしまう問題の対策
			// 未了残高照会タイムアウト時のリーダ画面遷移契機が制御データ（受付許可）受信のため
			// 休業状態で制御データ（受付許可）を送信するが、残高照会ボタンが表示されてしまう。
			// そのため、一旦、受付可にしてから受付不可にする暫定対策を行う。
			// 残高照会シーケンス変更の本対策後にこの処理を削除する。
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// 制御データ(受付禁止)送信
// MH810103 GG119202(S) E3209が発生してしまう問題の対策
			}
// MH810103 GG119202(E) E3209が発生してしまう問題の対策
		}
		break;
// MH810103 GG119202(E) 残高照会ボタン非表示の暫定対策
	}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 0;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			取引(決済結果)データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	：現在のOpeﾌｪｰｽﾞ
///	@return			ret			：0固定
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
short	EcRecvSettData(uchar *ope_faze)
{
	short	ret = 0;
	uchar	brand_index = 0;

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if ((Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// 決済結果ﾃﾞｰﾀを受信し
//		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0))		// 決済結果ﾃﾞｰﾀ未処理の場合
	if ((Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 		// 決済結果ﾃﾞｰﾀを受信し
		(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0))	// 決済結果ﾃﾞｰﾀ未処理の場合
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	{
		Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;						// みなし決済トリガ発生OFF
		switch (OPECTL.Ope_mod) {
		case 2:															// 料金表示,入金処理フェーズ
			// 「決済結果データ」電文を処理できるフェーズ
			// （ここでしか受信しないはず）
			break;
		default:	// 「決済結果データ」電文を処理しない処理フェーズで受信した場合
			EcSettlementPhaseError((uchar*)&Ec_Settlement_Res, 1);		// エラー登録処理
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;		// 決済結果データ処理済にする
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 1;	// 決済結果データ処理済にする
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			break;
		}
	}

	switch (*ope_faze) {
	case	2:
	// 精算中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		if (Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) {	// 決済結果ﾃﾞｰﾀ受信済みで未処理の場合
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;
		if (Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) {	// 決済結果ﾃﾞｰﾀ受信済みで未処理の場合
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 1;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			
			brand_index = convert_brandno(Ec_Settlement_Res.brand_no);
			if (dsp_change) {											// 精算未了ﾒｯｾｰｼﾞが表示したままの場合
				op_SuicaFusokuOff();
// MH810103 GG119202(S) 未了仕様変更対応
//				if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {	// 未了確定
//					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 1;				// 未了確定受信
//
//					// 未了残高照会表示
//					lcd_wmsg_dsp_elec( 1, ERR_CHR[20], ERR_CHR[53], 0, 0, COLOR_RED, LCD_BLINK_OFF); // "カード取引に失敗しました"
//																									 // "引き続き未了残高照会を行います"
//					dsp_change = 1;
//					// 不明ブランド？ || ブランド番号0？
//					if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//						// 選択商品データで送信したブランドを決済結果で受信した事にする
//						Ec_Settlement_Res.brand_no = Product_Select_Brand;
//						brand_index = convert_brandno(Product_Select_Brand);
//					}
//					ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// アラーム取引集計
//					ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// アラームレシート印字要求
//					EcAlarmLog_Regist(&Ec_Settlement_Res);							// 処理未了取引ログ登録
//					break;
//				}
//				else {
//					// 未了確定以外を受信
//					dsp_change = 0;
//				}
				dsp_change = 0;
// MH810103 GG119202(E) 未了仕様変更対応
			}
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// 制御ﾃﾞｰﾀ（取引終了）を送信する

			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// 問合せ(処理中)結果待ちタイマーリセット
			ec_MessageAnaOnOff( 0, 0 );
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
			Ope2_ErrChrCyclicDispStop();								// サイクリック表示停止
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応

			switch (Ec_Settlement_Res.Result) {
			case	EPAY_RESULT_CANCEL_PAY_OFF:							// 処理キャンセル

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) カードタッチ後に支払い選択画面に戻らない
//				if (check_enable_multisettle() > 1 &&
//					OPECTL.InquiryFlg == 0) {
//					// マルチブランド時の処理キャンセル(カード処理中なし)は精算中止しない
//					// 支払い選択画面に戻る
				if (check_enable_multisettle() > 1) {
					// 電子マネー決済時（マルチブランド）の処理キャンセルは支払い選択画面に戻る
					// クレジットカード決済時（マルチブランド）の
					// 処理キャンセル（カード処理中受信前）は支払い選択画面に戻る
					if (RecvBrandResTbl.no == BRANDNO_CREDIT &&
						OPECTL.InquiryFlg != 0) {
						// クレジットカード決済時（マルチブランド）の
						// 処理キャンセル（カード処理中受信後）は精算中止する
						goto EC_CANCEL_PAY_OFF_DUMMY;
					}
// MH810103 GG119202(S) 電子マネータッチ後のキャンセルで入金できない
					if (OPECTL.InquiryFlg != 0) {
						// 電子マネー読み取り待ち画面のみタッチ後もキャンセルボタン押下可能
						// 決済NG受信時と同様の処理をする
						ac_flg.ec_deemed_fg = 0;								// みなし決済復電用フラグOFF
						Lagcan( OPETCBNO, 1 );									// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);				// opeへ通知
					}
// MH810103 GG119202(E) 電子マネータッチ後のキャンセルで入金できない
					OPECTL.InquiryFlg = 0;								// 外部照会中フラグOFF
// MH810103 GG119202(E) カードタッチ後に支払い選択画面に戻らない
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
					EcEnableNyukin();
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
					EcBrandClear();
					Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動
					break;
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG118808_GG118908(S) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
				else {
					if (RecvBrandResTbl.no == BRANDNO_CREDIT) {
						// クレジットカード決済時の処理キャンセルは精算中止する
						goto EC_CANCEL_PAY_OFF_DUMMY;
					}
					// 電子マネーシングル設定でキャンセルボタン押下した場合は
					// 支払方法選択画面を表示する
					ECCTL.ec_single_timeout = 1;
					if (OPECTL.InquiryFlg != 0) {
						// 電子マネー読み取り待ち画面のみタッチ後もキャンセルボタン押下可能
						// 決済NG受信時と同様の処理をする
						ac_flg.ec_deemed_fg = 0;						// みなし決済復電用フラグOFF
						Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// opeへ通知
					}
					OPECTL.InquiryFlg = 0;								// 外部照会中フラグOFF
					EcBrandClear();
					Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動
					break;
				}
// MH810103 GG118808_GG118908(E) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
// MH810103 GG119202(S) 電子マネーシングルは決済NG（308）受信で精算中止する
EC_CANCEL_PAY_OFF_DUMMY:
// MH810103 GG119202(E) 電子マネーシングルは決済NG（308）受信で精算中止する
// MH810103 GG119202(S) 決済精算中止印字処理修正
//				ac_flg.ec_alarm = 0;									// みなし決済復電用フラグOFF
				ac_flg.ec_deemed_fg = 0;								// みなし決済復電用フラグOFF
// MH810103 GG119202(E) 決済精算中止印字処理修正
				Lagcan(OPETCBNO, 8);									// 入金後戻り判定ﾀｲﾏｰﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagcan(OPETCBNO, 1);									// ｵﾍﾟﾚｰｼｮﾝ関連ﾀｲﾏｰﾘｾｯﾄ
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// サイクリック表示タイマーﾘｾｯﾄ

				if (OPECTL.InquiryFlg == 0) {							// カード処理中受信なし
					// 保留していたイベントを再開する
					if(OPECTL.holdPayoutForEcStop) {
						OPECTL.holdPayoutForEcStop = 0;
						queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
					}
					if (EC_STATUS_DATA.StatusInfo.ReceptStatus) {		// 受付不可状態
						if ((OPECTL.op_faz == 8 && 						// 電子媒体停止待ち合わせ中(精算完了)
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END) ||
							OPECTL.op_faz == 3 ||						// とりけし
							OPECTL.op_faz == 9) {						// 電子媒体停止待ち合わせ中(とりけし)
							break;
						}
					}
				}
				OPECTL.InquiryFlg = 0;									// 外部照会中フラグOFF
				queset(OPETCBNO, EC_EVT_CANCEL_PAY_OFF, 0, 0);			// opeへ通知
				break;
			case	EPAY_RESULT_NG:										// 決済NG
			case	EPAY_RESULT_CANCEL:									// 決済キャンセル
			// 決済NG(使用不可ｶｰﾄﾞﾀｯﾁ,残高不足など)受信時

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if( ec_Disp_No == 99 ){									// タイムアウトエラーを受信した
				if( ECCTL.ec_Disp_No == 99 ){							// タイムアウトエラーを受信した
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810105 GG119202(S) iDの読み取り待ちタイムアウトで案内表示
					if (RecvBrandResTbl.no == BRANDNO_ID) {
						// iDのタイムアウトのみ案内表示する
						ECCTL.ec_Disp_No = 89;
						ec_WarningMessage((ushort)ECCTL.ec_Disp_No);
					}
// MH810105 GG119202(E) iDの読み取り待ちタイムアウトで案内表示
// MH810105(S) MH364301 QRコード決済対応
					else if (RecvBrandResTbl.no == BRANDNO_QR) {
						// QRコード決済のタイムアウト案内表示する
						ECCTL.ec_Disp_No = 16;
						ec_WarningMessage((ushort)ECCTL.ec_Disp_No);
					}
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) 電子マネーシングルは決済NG（308）受信で精算中止する
//					// タイムアウトエラー受信時は何も処理しない
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//					if (ECCTL.brand_num == 1) {
					if (check_enable_multisettle() == 1) {
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
						// クレジットカード（シングル）
						if (RecvBrandResTbl.no == BRANDNO_CREDIT) {
							// 何もしない
						}
// MH810105(S) MH364301 QRコード決済対応
//						// 電子マネー（シングル）
//						else if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
						// 電子マネー（シングル）or QR（シングル）
						else if (EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG118808_GG118908(S) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
//							// 精算中止処理を行う
//							goto EC_CANCEL_PAY_OFF_DUMMY;
							// 電子マネーシングル設定で読み取り待ちタイムアウトした場合は
							// 支払方法選択画面を表示する
							ECCTL.ec_single_timeout = 1;
							EcBrandClear();
							Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動
// MH810103 GG118808_GG118908(E) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
						}
					}
					else {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//						// マルチブランド時はブランドに関わらず何もしない
						// マルチブランド時のタイムアウトは精算中止しない
						// 支払い選択画面に戻る
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
						EcEnableNyukin();
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
						EcBrandClear();
						Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
					}
// MH810103 GG119202(E) 電子マネーシングルは決済NG（308）受信で精算中止する
					break;
				}

// MH810103 GG119202(S) 決済精算中止印字処理修正
//				ac_flg.ec_alarm = 0;									// みなし決済復電用フラグOFF
				ac_flg.ec_deemed_fg = 0;								// みなし決済復電用フラグOFF
// MH810103 GG119202(E) 決済精算中止印字処理修正
				Lagcan( OPETCBNO, 1 );									// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)

				if (OPECTL.InquiryFlg != 0) {							// カード処理中受信中
					OPECTL.InquiryFlg = 0;								// 外部照会中フラグOFF
				//※再度精算させるため、holdPayoutForEcStopの判断はopemainで行う
					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);			// opeへ通知
				}
				else {													// カード処理中受信せずに決済結果受信
				//※カード読取り失敗時など
					// 保留していたイベントを再開する
					if(OPECTL.holdPayoutForEcStop) {
						OPECTL.holdPayoutForEcStop = 0;
						queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
					}
				//※決済結果を受けたとき、状態データは受付不可が来るはず
					if (EC_STATUS_DATA.StatusInfo.ReceptStatus) {		// 受付不可状態
						if ((OPECTL.op_faz == 8 &&						// 電子媒体停止待ち合わせ中(精算完了)
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END) ||
							OPECTL.op_faz == 3 ||						// とりけし
							OPECTL.op_faz == 9) {						// 電子媒体停止待ち合わせ中(とりけし)
							break;
						}
						// ｶｰﾄﾞ差込中？
						else if( Suica_Rec.Data.BIT.CTRL_CARD ){
				//※この（カード処理中ではなく決済NG、キャンセルになる）場合、カードありとはならないはず
							if( OPECTL.op_faz == 8 &&
								(CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ)) {	// STOP_REASONがカード挿入の場合はｶｰﾄﾞの吐き戻しを行う
								if( OPECTL.other_machine_card ) {
									opr_snd( 13 );						// 保留位置からでも戻す
								}
								else {
									opr_snd( 2 );						// 前排出
								}
								CCT_Cansel_Status.BYTE = 0;				// 停止待ちステータスをクリア
							}
							else {
								read_sht_cls();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ
							}
							cn_stat( 2, 2 );							// 入金不可
						}
					}
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
					if (Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR != 0) {
						EcEnableNyukin();
						EcBrandClear();
					}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
				}

				// NG受信時のメッセージ、アナウンス
// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
				if( Ec_Settlement_Res.Column_No == 0xFF ){				// みなし決済による結果NG処理ならエラー表示させない
					Ec_Settlement_Res.Column_No = 0;
				}
				else {
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				ec_WarningMessage(ec_Disp_No);
				ec_WarningMessage(ECCTL.ec_Disp_No);
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
				}
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
// MH810105(S) MH364301 QRコード決済対応
				if (RecvBrandResTbl.no == BRANDNO_QR) {
					if (!IsQrContinueReading(Ec_Settlement_Rec_Code)) {
						EcEnableNyukin();
						EcBrandClear();
					}
				} else {
// MH810105(E) MH364301 QRコード決済対応
// MH810105 GG119202(S) 電子マネー決済エラー発生時の画面戻り仕様変更
				if (!IsEcContinueReading(Ec_Settlement_Rec_Code)) {
					if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
						// 電子マネー決済で支払方法選択画面に戻るエラー
						EcEnableNyukin();
						EcBrandClear();
					}
				}
// MH810105 GG119202(E) 電子マネー決済エラー発生時の画面戻り仕様変更
// MH810105(S) MH364301 QRコード決済対応
				}
// MH810105(E) MH364301 QRコード決済対応

				// ｶｰﾄﾞあり？
				if( Suica_Rec.Data.BIT.CTRL_CARD ){
					// ｶｰﾄﾞ差込中？ NGメッセージ表示を(6秒)待ってカード抜き取りｱﾅｳﾝｽ開始
					LagTim500ms(LAG500_EC_START_REMOVAL_TIMER, (short)(6*2+1), ec_start_removal_wait);
				}
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動(サイクリック表示はさせる)
				break;
			case	EPAY_RESULT_OK:										// 決済OK
// MH810103 GG119202(S) 未了仕様変更対応
//			case	EPAY_RESULT_MIRYO_AFTER_OK:							// 未了確定後決済OK
// MH810103 GG119202(E) 未了仕様変更対応
			case	EPAY_RESULT_MIRYO_ZANDAKA_END:						// 未了残高照会完了
				// 決済結果が完了応答の時（残高不足後の再タッチ時を含む）

				if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {				// 未了状態発生中？ 
					LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);		// 未了状態監視ﾀｲﾏﾘｾｯﾄ
					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					// 未了状態解除
// MH810103 GG119202(S) 未了仕様変更対応
					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;				// 未了確定(状態データ)受信をクリア
// MH810103 GG119202(E) 未了仕様変更対応
				}
				Lagcan(OPETCBNO, 8);									// 入金後戻り判定ﾀｲﾏｰﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagcan(OPETCBNO, 1);									// ｵﾍﾟﾚｰｼｮﾝ関連ﾀｲﾏｰﾘｾｯﾄ
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// サイクリック表示タイマーﾘｾｯﾄ
				nyukin_delay_check(nyukin_delay, delay_count);			// 保留中のｲﾍﾞﾝﾄがあれば再postする

				// 不明ブランド？ || ブランド番号0？
				if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
					// 選択商品データで送信したブランドを決済結果で受信した事にする
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					Ec_Settlement_Res.brand_no = Product_Select_Brand;
//					brand_index = convert_brandno(Product_Select_Brand);
					Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
					brand_index = convert_brandno(ECCTL.Product_Select_Brand);
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				}
				// 選択商品データにて送信したブランド番号と一致しない？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				else if ( Product_Select_Brand != Ec_Settlement_Res.brand_no ){
				else if ( ECCTL.Product_Select_Brand != Ec_Settlement_Res.brand_no ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					// ブランド不一致フラグ
					PayData.Electron_data.Ec.E_Status.BIT.brand_mismatch = 1;
				}

				// 決済ブランド毎の決済処理を実施
				switch(brand_index) {
					case EC_EDY_USED:
					case EC_NANACO_USED:
					case EC_WAON_USED:
					case EC_KOUTSUU_USED:
					case EC_SAPICA_USED:
					case EC_ID_USED:
					case EC_QUIC_PAY_USED:
// MH810105(S) MH364301 PiTaPa対応
					case EC_PITAPA_USED:
// MH810105(E) MH364301 PiTaPa対応
						e_inflg = 1;										// 電子マネー決済である
						e_pay = Ec_Settlement_Res.settlement_data;			// 電子マネー決済額を保存
// MH810103 GG119202(S) みなし決済時は残高表示しない
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//						if (Ec_Settlement_Res.Column_No == 0xFF) {
						if (Ec_Settlement_Res.Column_No == 0xFF ||
// MH810105(S) MH364301 PiTaPa対応
//							brand_index == EC_ID_USED ||
//							brand_index == EC_QUIC_PAY_USED) {
							EcBrandPostPayCheck(brand_index)) {
// MH810105(E) MH364301 PiTaPa対応
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
							e_zandaka = -1;									// みなし決済時は残高を非表示とする
						}
						else {
// MH810103 GG119202(E) みなし決済時は残高表示しない
						e_zandaka = Ec_Settlement_Res.settlement_data_after;// 電子マネー残高を保存
// MH810103 GG119202(S) みなし決済時は残高表示しない
						}
// MH810103 GG119202(E) みなし決済時は残高表示しない
						EcElectronSet_PayData(&Ec_Settlement_Res);			// 電子マネーで精算
						ret = al_emony(SUICA_EVT , 0);						// 電子マネー精算処理
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//						if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END ||
//							Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {
//							// 未了残高照会完了、または、みなし決済（未了中）は未了回数をカウントする
//							ec_alarm_syuu(brand_index, (ulong)Product_Select_Data);	// アラーム取引集計
//							if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {
//								// みなし決済の場合はみなし用決済結果データを作成する
//								EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//							}
						if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
							ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// アラームレシート印字要求
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//							EcAlarmLog_Regist(&Ec_Settlement_Res);					// 処理未了取引ログ登録
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
						}
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
						break;
					case EC_CREDIT_USED:
						EcCreUpdatePayData(&Ec_Settlement_Res);				// クレジット決済データ保存
						ret = al_emony(EC_EVT_CRE_PAID , 0);				// クレジット精算処理
						break;
// MH810105(S) MH364301 QRコード決済対応
					case EC_QR_USED:
						q_inflg = 1;										// QRコード決済である
						e_pay = Ec_Settlement_Res.settlement_data;			// QRコード決済額を保存
						e_zandaka = -1;										// 非表示とする
						EcQrSet_PayData(&Ec_Settlement_Res);				// QRコードで精算
						ret = al_emony(EC_EVT_QR_PAID , 0);					// QRコード精算処理
						break;
// MH810105(E) MH364301 QRコード決済対応
					default:
						break;
				}

// MH810103 GG119202(S) 決済停止エラーの登録
				if (Ec_Settlement_Res.Column_No != 0xFF) {
// MH810103 GG119202(E) 決済停止エラーの登録
				// 301エラー解除
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_301, 0, 0, 0);
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
				// 電子マネー系重大詳細エラー解除
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 0, 0, 0);
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
// MH810103 GG119202(S) 決済停止エラーの登録
					// 決済OK受信でJVMAリセット積算回数をクリア
					Ec_Jvma_Reset_Count = 0;
				}
// MH810103 GG119202(E) 決済停止エラーの登録

				if (brand_index != EC_CREDIT_USED && ryo_buf.dsp_ryo) {
					EcSendCtrlEnableData();								// 制御データ(受付許可)送信
					if (OPECTL.op_faz == 3) {							// 取消し中の場合
						Op_Cansel_Wait_sub(0);							// CRW-MJA停止済み処理実行
					}
				}
				// 保留していたイベントを再開する
				if(OPECTL.holdPayoutForEcStop) {
					OPECTL.holdPayoutForEcStop = 0;
					queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
				}
				break;
// MH810103 GG119202(S) 未了仕様変更対応
			case	EPAY_RESULT_MIRYO:									// 決済未了確定
// MH810103 GG119202(S) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
				Lagcan(OPETCBNO, 8);									// 入金後戻り判定ﾀｲﾏｰﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
// MH810103 GG119202(E) 未了残高照会タイムアウトしてから10分経過で制御データ（受付許可）が送信される
				LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);			// 未了状態監視ﾀｲﾏﾘｾｯﾄ
				if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {				// 未了状態発生中？ 
					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					// 未了状態解除
					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;				// 未了確定(状態データ)受信をクリア
					wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);			// モニタ登録
				}
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH810103 GG118808_GG118908(S) 現金投入と同時にカードタッチ→未了発生でロックする
//				if( ryo_buf.nyukin && OPECTL.credit_in_mony ){			// 入金ありで電子マネーとのすれ違い入金の場合
//					// 入金額 >= 駐車料金の状態で未了確定を受信した場合は
//					// 精算完了として扱い、とりけしボタン押下待ちにしない
//					if( OPECTL.credit_in_mony == 1 ){
//						in_mony ( COIN_EVT, 0 );						// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//					}
//					else{/* OPECTL.credit_in_mony == 2 */
//						in_mony ( NOTE_EVT, 0 );						// 紙幣ｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//					}
//					OPECTL.credit_in_mony = 0;							// すれ違いﾌﾗｸﾞｸﾘｱ
//				}
//// MH810103 GG118808_GG118908(E) 現金投入と同時にカードタッチ→未了発生でロックする
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
				OPECTL.InquiryFlg = 0;									// 外部照会中フラグOFF
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH810103 MHUT40XX(S) Edy・WAON対応
//				ac_flg.ec_deemed_fg = 0;								// みなし決済復電用フラグOFF
//// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
				ope_anm( AVM_STOP );									// 放送停止ｱﾅｳﾝｽ
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// サイクリック表示タイマーﾘｾｯﾄ
				nyukin_delay_check(nyukin_delay, delay_count);			// 保留中のｲﾍﾞﾝﾄがあれば再postする

				ec_miryo_timeout_after_disp(&Ec_Settlement_Res);		// 取引失敗/取り消し画面表示

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
// 処理未了取引集計、アラームレシート印字要求、処理未了取引ログ登録はec_miryo_timeout_after_dispにて行うように変更
//				if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//					// 選択商品データで送信したブランドを決済結果で受信した事にする
//					Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
//					brand_index = convert_brandno(ECCTL.Product_Select_Brand);
//				}
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////				ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// アラーム取引集計
////				ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// アラームレシート印字要求
////				EcAlarmLog_Regist(&Ec_Settlement_Res);							// 処理未了取引ログ登録
//				if (ECCTL.ec_Disp_No != 96 &&									// 未引き去り確認ではない
//					brand_index != EC_ID_USED &&								// iD決済ではない
//					brand_index != EC_QUIC_PAY_USED) {							// QUICPay決済ではない
//					// 決済未了確定は処理未了取引の支払不明回数をカウントする
//					ec_alarm_syuu(0, (ulong)Product_Select_Data);		// 処理未了取引（支払不明）集計
//					ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);		// アラームレシート印字要求
//					EcAlarmLog_Regist(&Ec_Settlement_Res);				// 処理未了取引ログ登録
//				}
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
				break;
// MH810103 GG119202(E) 未了仕様変更対応
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド選択結果データ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	：現在のOpeﾌｪｰｽﾞ
///	@return			ret			：0固定
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/13<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	EcRecvBrandData(uchar *ope_faze)
{
	short	ret = 0;
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	uchar	inq;
	uchar	can;
// GG129000(S) #7001 【アミュプラザ鹿児島現地問題No.20】期限切れのJQカードを読み取り後にQRコードリーダーが読み取り再開しない（GM803002流用）
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒とする
	}
// GG129000(E) #7001 【アミュプラザ鹿児島現地問題No.20】期限切れのJQカードを読み取り後にQRコードリーダーが読み取り再開しない（GM803002流用）

	if (isEC_STS_CARD_PROC()) {							// 決済状態 処理中の場合は待ち合わせる。
		return ret;										// 無効
	}
	Suica_Rec.suica_rcv_event.EC_BIT.BRAND_DATA = 0;
	inq = OPECTL.InquiryFlg;			// 決済状態の通知を受けていたか
	OPECTL.InquiryFlg = 0;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) ハウスカードによるカード処理中タイムアウトは精算中止とする
	if (inq != 0) {
		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );	// 問合せ(処理中)結果待ちタイマーリセット
	}
// MH810103 GG119202(E) ハウスカードによるカード処理中タイムアウトは精算中止とする
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	can = (uchar)Suica_Rec.Data.BIT.BRAND_CAN;
	Suica_Rec.Data.BIT.BRAND_CAN = 0;	// 中止要求と同時に選択が発生した
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	switch (*ope_faze) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	case	0:	// 待機画面
//	case	1:	// 車室番号入力画面
//	case	3:	// 精算完了
	default:
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		break;

	case	2:		// 精算中
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		if (!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// 精算未了状態で無い場合のみ処理を実施
//			if (OPECTL.op_faz == 3 ||										// 電子媒体停止待ち合わせ中？
//				OPECTL.op_faz == 8) {
//				// 電子媒体停止待ち合わせ中での受付可受信時は、ﾌｪｰｽﾞ合わせの為に受付禁止を送信する。
//				Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//				break;
//			}
//			if (OPECTL.op_faz >= 9 || 										// 精算中止時の決済リーダ停止待ち合わせ時(正常に停止していない場合)か
//				EC_STATUS_DATA.StatusInfo.Running) {						// 受信ステータスが実行中
//				break;
//			}
		if (Suica_Rec.Data.BIT.BRAND_SEL) {									// ブランド選択をしている場合
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			if (OPECTL.op_faz < 2) {										// 精算中
				if(RecvBrandResTbl.res == EC_BRAND_SELECTED) {				// ブランドを選択した
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//					if(isEcBrandNoReady(RecvBrandResTbl.no)) {				// 選択したブランドは開局？（念のための確認）
//						if ((WAR_MONEY_CHECK && !e_incnt) ||				// 未入金か
//							(RecvBrandResTbl.no == BRANDNO_CREDIT) ) {		// 選択したブランドがクレジット
//							// 選択商品データ送信
//							if (w_settlement != 0) {
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);
//							} else {
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);
//								w_settlement = 0;
//							}
//						}
//					} else {
//						// ありえないが、送信したブランド以外のブランドが選択されている
//						// 受付不可からやり直し（→受付可→ブランド選択データ）
//						Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//					}
//				}
//				else {
//					// 未選択にされたので制御データ（受付禁止）を送信する
//					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//
//				}
					if (IsBrandSpecified(RecvBrandResTbl.no)) {			// 選択要求を出したブランドは受け付ける
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応参考）
						// 決済ありブランドが選択された場合はブランド選択結果応答データ(OK)を送信する
						// 決済なしブランド／後決済ブランドが選択された場合はカードデータのチェックをしたあとに
						// ブランド選択結果応答データ(OK/NG)を送信する
						if (!IsSettlementBrand(RecvBrandResTbl.no)) {
							// 決済なしブランド／後決済ブランド
							if (!EcNotifyCardData(RecvBrandResTbl.no, RecvBrandResTbl.dat)) {
								// 未定義はブランドはＯＫ応答した上でキャンセル相当処理
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// ブランド選択結果応答OK
								if (inq != 0) {
									ec_MessageAnaOnOff(0, 0);
									EnableNyukin();
								}
								queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
							}
						}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応参考）
						else {
						// 決済ブランドは選択商品データ送信
							if (can != 0) {
							// すれ違いで受付禁止送信済みならば、選択結果応答送信後、受付不可になるので選択商品データが出せない。
							// →OKして読み捨てる
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// ブランド選択結果応答OK
								queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
							}
							else {
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA_with_HOLD, 1);		// 決済選択ありのとき、ブランド選択結果応答データセットを送信する
																							// ブランド選択結果は保持する
								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);
								w_settlement = 0;
// MH810103 GG119202(S) 電子マネーブランド選択時の案内放送
								if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
									// 電子マネーブランド選択時の案内放送
									ope_anm(AVM_SELECT_EMONEY);
// MH810103(s) 電子マネー対応 電子マネーブランド選択時の案内放送
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
// MH810103(e) 電子マネー対応 電子マネーブランド選択時の案内放送
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
									cn_stat( 2, 2 );										// 入金不可
									if (LCDNO == 3) {
										// 「硬貨または紙幣で精算して下さい」非表示のため再描画
										OpeLcd(3);
									}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
								}
// MH810103 GG119202(E) 電子マネーブランド選択時の案内放送
// MH810105(S) MH364301 QRコード決済対応
								else if( RecvBrandResTbl.no == BRANDNO_QR ){
// MH810105(S) MH364301 QRコード決済対応 GT-4100
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
// MH810105(E) MH364301 QRコード決済対応 GT-4100
									cn_stat( 2, 2 );										// 入金不可
									if (LCDNO == 3) {
										// 「硬貨または紙幣で精算して下さい」非表示のため再描画
										OpeLcd(3);
									}
								}
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6412 休業の案内表示(ポップアップ)されないことがある
								else if( RecvBrandResTbl.no != BRANDNO_ZERO ){
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
								}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6412 休業の案内表示(ポップアップ)されないことがある
							}
						}
						return ret;		// ここまで
					}
				//	else	応答して読み捨て
				}
				else if (RecvBrandResTbl.res == EC_BRAND_CANCELED) {
					BrandResRsltTbl.no = RecvBrandResTbl.no;	// 理由コードはブランド選択結果で受信したコードをそのままセットする
					Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);	// ブランド選択結果応答NG
					if (inq != 0) {
						ec_MessageAnaOnOff(0, 0);
						EnableNyukin();
					}
					queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
					return ret;
				}
				else {
					BrandResRsltTbl.no = RecvBrandResTbl.no;	// 理由コードはブランド選択結果で受信したコードをそのままセットする
					Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);	// ブランド選択結果応答NG
					if (inq != 0) {
						ec_MessageAnaOnOff(0, 0);
						EnableNyukin();
					}
					queset( OPETCBNO, EC_BRAND_RESULT_UNSELECTED, 0, 0);
// GG129000(S) #7001 【アミュプラザ鹿児島現地問題No.20】期限切れのJQカードを読み取り後にQRコードリーダーが読み取り再開しない（GM803002流用）
					if (BrandResRsltTbl.no == EC_BRAND_REASON_205) {
						// 意図しないカードの読取時
						// QR開始要求送信
						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
						// 応答待ちタイマースタート
						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
					}
// GG129000(E) #7001 【アミュプラザ鹿児島現地問題No.20】期限切れのJQカードを読み取り後にQRコードリーダーが読み取り再開しない（GM803002流用）
					return ret;
				}
			}
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		break;
	}
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//	Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
// ※※　ブランド選択結果は必ず応答する必要がある　※※
	// 決済ありブランドの場合は適用OKを送信する。(JVMAコマンド仕様(電子決済)_200817_Rev4.07参照)
	if ((RecvBrandResTbl.res == EC_BRAND_SELECTED) 
     && (IsSettlementBrand(RecvBrandResTbl.no))) {
		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// ブランド選択結果応答OK
	} else {
		BrandResRsltTbl.no = EC_BRAND_REASON_205;					// 意図しないカード読取り
		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// ブランド選択結果応答NG
	}
	if (inq != 0) {													// 精算完了とブランド選択がすれ違う場合、選択結果は読み捨てる、
		ec_MessageAnaOnOff(0, 0);									// 選択時のカード処理中をここでクリアする
		EnableNyukin();
	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 選択ブランドが処理できない場合の動作
	if (OPECTL.op_faz < 2) {										// 精算中
		// ブランド選択データ再送するため、制御データ（受付禁止）を送信する
		Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );							// 制御データ(受付禁止)送信
	}
// MH810103 GG119202(E) 選択ブランドが処理できない場合の動作

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			取引データ受信ﾌｪｰｽﾞ異常時のエラー登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat ： 取引データ
///	@param[in]		kind ： 1:決済不可なﾌｪｰｽﾞで取引データを受信
///				   		 ： 2:Opeが処理する前に再度取引データを受信
///				   		 ： 3:Opeが一度決済したのにも関わらず、再度
///				   		 	  取引データを受信した場合
///				   		 ： 4:取引データを受信後、精算LOG等に登録
///				   			  する前にﾌｪｰｽﾞが変わった場合
///				   		 ： 5:受信した取引データをOPEタスクが処理
///				   			  しないうちに精算機がリセットした場合
///				   		 ： 6:OPEタスクが受信イベントを処理する前に
///				   			  受信キューがフルになり，上書きしようと
///				   			  しているバッファに格納されているデータ
///				   			  が取引データだった場合
///				   		 ： 7:イニシャルシーケンスで有効な決済結果データ
///				   			  を受信した場合
///				   		 ： 8:精算開始後、選択商品データを送信していない
///				   			  にもかかわらず決済結果データを受信した場合
///						 ： 9:精算中止完了後に決済結果データを受信した場合
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	EcSettlementPhaseError(uchar *dat, uchar kind)
{
	uchar	work_brand;
	long	work;
	EC_SETTLEMENT_RES *work_set;

	memset(err_wk, 0, sizeof(err_wk));
	memcpy(&err_wk[0], "\?\?-0000   00000 fz:000-00", 25);
	if (kind == 1 || kind == 4) {									// 決済結果データエリアにすでにデータが格納されている場合はこっち
		work_set = (EC_SETTLEMENT_RES *)dat;						// データセット

		// カード種別
		work_brand = convert_brandno(work_set->brand_no);
		memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QRコード決済対応
		if( work_brand == EC_QR_USED ){							// QRの場合、支払種別をセット
			memset(&err_wk[3], 0x20, 4);						// ４桁分スペースをセット
			memcpy(&err_wk[3], QrBrandKind_prn[work_set->Brand.Qr.PayKind], 2);
		}
		else{
// MH810105(E) MH364301 QRコード決済対応
		// カード番号
		memset(Card_Work, 0x20, sizeof(Card_Work));
		memcpyFlushRight(Card_Work, work_set->Card_ID, sizeof(Card_Work), sizeof(work_set->Card_ID));
		memcpy(&err_wk[3], &Card_Work[16], 4);					// カード番号の下４桁をセット
// MH810105(S) MH364301 QRコード決済対応
		}
// MH810105(E) MH364301 QRコード決済対応
		// 取引種別
		if (work_set->Result == EPAY_RESULT_MIRYO ||
// MH810103 GG119202(S) 未了仕様変更対応
//			work_set->Result == EPAY_RESULT_MIRYO_AFTER_OK ||
// MH810103 GG119202(E) 未了仕様変更対応
			work_set->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			memcpy(&err_wk[7], "(A)", 3);
		}
		// 取引額
		work = work_set->settlement_data;
		if (work > 0xffff) {
		// 取引額がshort値以上なら補正
			memcpy(&err_wk[10], "FFFFH", 5);
		} else {
			intoasl(&err_wk[10], work, 5);
		}
		// 精算機処理状態(OPEﾌｪｰｽﾞ)
		intoas(&err_wk[19], (ushort)OPECTL.Ope_mod, 3);				// 登録時のｵﾍﾟﾌｪｰｽﾞをセット
		// 登録要因
		intoas(&err_wk[23], (ushort)kind, 2);						// 登録種別をセット
	}else{															// 受信データをそのまま使う場合はこっち
		if (kind == 5) {
			if (dat[0] == 0) {
				return;		// 決済結果＝なしは読み捨てる
			}
		}

		// カード種別
		work_brand = convert_brandno( bcdbin3(&dat[2]) );
		memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QRコード決済対応
		if( work_brand == EC_QR_USED ){							// QRの場合、支払種別をセット
			memset(&err_wk[3], 0x20, 4);						// ４桁分スペースをセット
			memcpy(&err_wk[3], QrBrandKind_prn[dat[44]], 2);
		}
		else{
// MH810105(E) MH364301 QRコード決済対応
		// カード番号
		memset(Card_Work, 0x20, sizeof(Card_Work));
		memcpyFlushRight(Card_Work, &dat[13], sizeof(Card_Work), 20);
		memcpy(&err_wk[3], &Card_Work[16], 4);					// カード番号の下４桁をセット
// MH810105(S) MH364301 QRコード決済対応
		}
// MH810105(E) MH364301 QRコード決済対応
		// 取引種別
		if (dat[0] == EPAY_RESULT_MIRYO ||
// MH810103 GG119202(S) 未了仕様変更対応
//			dat[0] == EPAY_RESULT_MIRYO_AFTER_OK ||
// MH810103 GG119202(E) 未了仕様変更対応
			dat[0] == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			memcpy(&err_wk[7], "(A)", 3);
		}
		// 取引額
		work = settlement_amount_ex(&dat[4], 3);					// 取引額(BCD形式の決済額をBin形式に変換)
		if (work > 0xffff) {
		// 取引額がshort値以上なら補正
			memcpy(&err_wk[10], "FFFFH", 5);
		} else {
			intoasl(&err_wk[10], work, 5);
		}
		// 精算機処理状態(OPEﾌｪｰｽﾞ)
		intoas(&err_wk[19], (ushort)OPECTL.Ope_mod, 3);				// 登録時のｵﾍﾟﾌｪｰｽﾞをセット
		// 登録要因
		intoas(&err_wk[23], (ushort)kind, 2);						// 登録種別をセット
	}
// MH321800(S) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
	// 決済結果OK時はE3259を登録しリーダを切り離す
	if( dat[0] == EPAY_RESULT_OK ){
// MH321800(E) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
	err_chk2((char)jvma_setup.mdl, (char)ERR_EC_SETTLE_ERR, (char)1, (char)1, (char)1, (void*)err_wk);
			// E3259登録（決済額パラメータあり：ASCII） "E3259   08/03/17 14:38   発生/解除"
			//											"        ED-3456   01000 fz:003-1  "
			//											"        ED-3456(A)01000 fz:003-1  "
			//												 種別、カード番号下4桁、取引種別、取引額、その時のOPmod、登録要因
// MH321800(S) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
	}
	// 決済結果OK以外はE3249を登録しリーダを切り離さない
	else {
// MH810105(S) MH364301 E3249を発生／解除別に変更
//		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_NOTOK_SETTLE_ERR, (char)2, (char)1, (char)1, (void*)err_wk);
		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_NOTOK_SETTLE_ERR, (char)1, (char)1, (char)1, (void*)err_wk);
// MH810105(E) MH364301 E3249を発生／解除別に変更
			// E3249登録（決済額パラメータあり：ASCII） "E3249   08/03/17 14:38   発生/解除"
			//											"        ED-3456   01000 fz:003-1  "
			//											"        ED-3456(A)01000 fz:003-1  "
			//												 種別、カード番号下4桁、取引種別、取引額、その時のOPmod、登録要因	
	}
// MH321800(E) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
#if (4 != AUTO_PAYMENT_PROGRAM)										// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
// ※基本的に(0 == AUTO_PAYMENT_PROGRAM)なので、有効
//   決済リーダでもSUICA_LOG_REC,SUICA_LOG_REC_FOR_ERRの領域を使用する
	memcpy(&SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof(struct suica_log_rec));	// 異常ﾛｸﾞとして現在の通信ﾛｸﾞを登録する。
#endif

	// 重複登録を防ぐためにすでに他の要因でエラー登録されていた場合は、kind=4の時の
	// エラー登録要因ﾌﾗｸﾞをクリアする
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;

// MH321800(S) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
//	Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;				// EC決済異常発生フラグ
// MH810105(S) MH364301 E3249を発生／解除別に変更
//	// 決済結果OK時はE3259を登録しリーダを切り離す
//	if( dat[0] == EPAY_RESULT_OK ){
	{																// E3249,E3259発生時にはJVMAリセットさせる
// MH810105(E) MH364301 E3249を発生／解除別に変更
		Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;			// EC決済異常発生フラグ
// MH810103 GG119202(S) E3259、E3262、E3263発生でJVMAリセットしない
		// 決済異常発生時はJVMAリセットが完了するまで
		// 決済リーダとの通信は行わず、コインメック等の通信を優先させる
		// ec_init()内でECCTL.phaseを更新する
		Suica_Rec.Data.BIT.INITIALIZE = 0;
		ECCTL.phase = EC_PHASE_WAIT_RESET;
		queset( OPETCBNO, EC_EVT_NOT_AVAILABLE_ERR, 0, 0 );			// 決済リーダ切り離しエラー発生をopeへ通知
// MH810103 GG119202(E) E3259、E3262、E3263発生でJVMAリセットしない
	}
// MH321800(E) フェーズエラー発生時に決済OK以外はリーダー切り離ししない

// MH810105(S) MH364301 E3249を発生／解除別に変更
	// E3249,E3259発生時にJVMAリセットを行うため以下の処理
	// 制御ﾃﾞｰﾀ（取引終了）を送信する必要がない。
//// MH810104(S) MH321800(S) ACK5受信でフェーズずれが発生する
//	// 決済結果データ受信時にフェーズずれが発生した場合は取引終了を送信する
//	switch (kind) {
//	case 4:
//	case 5:
//		// 取引終了送信済み、または、復電時のため送信しない
//		break;
//	default:
//		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// 制御ﾃﾞｰﾀ（取引終了）を送信する
//		break;
//	}
//// MH810104(E) MH321800(E) ACK5受信でフェーズずれが発生する
// MH810105(E) MH364301 E3249を発生／解除別に変更
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダが使用されたかどうかの判定を行う
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	：使用カード種別
///	@return			ret		：1：使用済み 0：未使用
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	EcUseKindCheck(uchar kind)
{
	uchar ret = 0;

	if ((kind == EC_EDY_USED)	 ||	// Edy使用時
		(kind == EC_NANACO_USED) ||	// nanaco使用時
		(kind == EC_WAON_USED)	 ||	// WAON使用時
		(kind == EC_KOUTSUU_USED)||	// 交通系ICカード使用時
		(kind == EC_SAPICA_USED) ||	// SAPICA使用時
		(kind == EC_ID_USED)	 ||	// iD使用時
// MH810105(S) MH364301 QRコード決済対応
//		(kind == EC_QUIC_PAY_USED)) {	// QUICPay使用時
		(kind == EC_QUIC_PAY_USED)	||	// QUICPay使用時
// MH810105(S) MH364301 PiTaPa対応
		(kind == EC_PITAPA_USED) ||		// PiTaPa使用時
// MH810105(E) MH364301 PiTaPa対応
		(kind == EC_QR_USED)) {			// QR使用時
// MH810105(E) MH364301 QRコード決済対応
		// クレジットカードは、ここでは判定しない
		ret = 1;
	}
	return ret;	
}

// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			電子マネー決済チェック（決済リーダ）
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 使用カード種別
///	@return			ret		: 1：使用済み、0：未使用
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/08/03
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcUseEMoneyKindCheck(uchar kind)
{
	uchar ret = 0;

	if ((kind == EC_EDY_USED)		||	// Edy使用時
		(kind == EC_NANACO_USED)	||	// nanaco使用時
		(kind == EC_WAON_USED)		||	// WAON使用時
		(kind == EC_KOUTSUU_USED)	||	// 交通系ICカード使用時
		(kind == EC_SAPICA_USED)	||	// SAPICA使用時
		(kind == EC_ID_USED)		||	// iD使用時
// MH810105(S) MH364301 PiTaPa対応
		(kind == EC_PITAPA_USED) 	||	// PiTaPa使用時
// MH810105(E) MH364301 PiTaPa対応
		(kind == EC_QUIC_PAY_USED)) {	// QUICPay使用時
		// クレジットカード、QRコードはここでは判定しない
		ret = 1;
	}
	return ret;	
}
// MH810105(E) MH364301 QRコード決済対応

//[]----------------------------------------------------------------------[]
///	@brief			アラームレシート印字要求
//[]----------------------------------------------------------------------[]
///	@param			prikind		:	印字先
///	@param			*data		:	受信取引データ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ReqEcAlmRctPrint(uchar prikind, EC_SETTLEMENT_RES *data)
{
// ステータス受信と取引データ受信時があるため
	T_FrmEcAlmRctReq	FrmAlmRctReq;

	if (prikind == 0) {
	// 印字先設定なし
		return;
	}

	memset(&FrmAlmRctReq, 0, sizeof(FrmAlmRctReq));
	memset(&alm_rct_info, 0, sizeof(alm_rct_info));
	memcpy(&alm_rct_info, data, sizeof(alm_rct_info));						// アラームレシートデータｾｯﾄ
	FrmAlmRctReq.prn_kind = prikind;										// 印字先
	memcpy(&FrmAlmRctReq.TOutTime, &CLK_REC, sizeof(date_time_rec));		// 現在時刻ｾｯﾄ
	FrmAlmRctReq.alm_rct_info = &alm_rct_info;
	queset(PRNTCBNO, PREQ_EC_ALM_R, sizeof(FrmAlmRctReq), &FrmAlmRctReq);
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 処理未了取引ログクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EcAlarmLog_Clear( void )
{
	// データクリア
	nmicler(&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT));
	EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
	EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 処理未了取引ログ登録
//[]----------------------------------------------------------------------[]
///	@param[in]		*data 	: 決済データ
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EcAlarmLog_Regist( EC_SETTLEMENT_RES *data )
{
	t_ALARM_SETTLEMENT_DATA	*alarm_settlement;
	short					ctpt[2];
	ushort					num;
// MH810103 GG119202(S) 処理未了取引記録のカード番号はマスクして印字する
	ushort					i, max;
// MH810103 GG119202(E) 処理未了取引記録のカード番号はマスクして印字する


	// データクリア
	alarm_settlement = &EC_ALARM_LOG_DAT.log[EC_ALARM_LOG_DAT.log_wpt];
	memset(alarm_settlement, 0, (size_t)EC_ALARM_LOG_DAT.data_size);
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
	memset(&EC_ALARM_LOG_DAT.paylog[EC_ALARM_LOG_DAT.log_wpt], 0, sizeof(Receipt_data));
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

	// データセット
	memcpy(&alarm_settlement->alarm_time, &CLK_REC, sizeof(date_time_rec));				// 発生時刻
	alarm_settlement->alarm_time.Sec = (ushort)CLK_REC.seco;

	memcpy(&alarm_settlement->alarm_data, data, sizeof(EC_SETTLEMENT_RES));			// 未了取引データ
	// PayDataはPayData_setまでセットされないため、PayData_setでセットされるのと同様にセットする
// MH810103 GG119202(S) 処理未了取引記録のカード番号はマスクして印字する
	// 未了タイムアウト発生時の決済結果データにはマスクされないカード番号が通知されるため、
	// ここでカード番号をマスクする（ジャーナルに印字するカード番号はマスクしない）
	switch (alarm_settlement->alarm_data.brand_no) {
	case BRANDNO_KOUTSUU:
		max = ECARDID_SIZE_KOUTSUU - 4;
		for (i = 2; i < max; i++) {
			alarm_settlement->alarm_data.Card_ID[i] = '*';								// 上位2桁、下位4桁以外をマスク
		}
		break;
// MH810103 MHUT40XX(S) Edy・WAON対応
	case BRANDNO_EDY:
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	case BRANDNO_NANACO:
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
// MH810104(E) nanaco・iD・QUICPay対応2
		memset(&alarm_settlement->alarm_data.Card_ID[0], '*', 12);						// 下位4桁以外をマスク
		break;
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	case BRANDNO_ID:
//	case BRANDNO_QUIC_PAY:
//		// iD, QUICPayはec_miryo_timeout_after_disp()内で
//		// 処理未了レシート用のマスク処理を行うため、ここでは不要
//		break;
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
	case BRANDNO_ID:
		memset(&alarm_settlement->alarm_data.Card_ID[13], '*', 3);						// 下位3桁をマスク
		break;
	case BRANDNO_QUIC_PAY:
		memset(&alarm_settlement->alarm_data.Card_ID[16], '*', 4);						// 下位4桁をマスク
		break;
// MH810104(E) nanaco・iD・QUICPay対応2
	default:
// MH810103 MHUT40XX(S) Edy・WAON対応
	case BRANDNO_WAON:
		// WAONはマスクされて通知される
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
	case BRANDNO_NANACO:
		// nanacoはマスクされて通知される
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
		break;
	}
// MH810103 GG119202(E) 処理未了取引記録のカード番号はマスクして印字する
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
	// 未了発生理由
	alarm_settlement->reason = 0;
	switch (data->Result) {
	case EPAY_RESULT_MIRYO_ZANDAKA_END:
		alarm_settlement->reason = ALARM_REASON_HIKISARI_OK;
		break;
	case EPAY_RESULT_MIRYO:
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118809_GG118909(S) 未引き去り確認時の未了理由に対応する
//		if (ECCTL.ec_Disp_No == 96) {
//			// 未引き去り確認
//			alarm_settlement->reason = ALARM_REASON_HIKISARI_NG;
//			break;
//		}
//		// 未了残高照会タイムアウト、または、キャンセル
//// MH810103 GG118809_GG118909(E) 未引き去り確認時の未了理由に対応する
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		alarm_settlement->reason = ALARM_REASON_TIMEOUT;
		break;
	default:
// MH810105(S) MH364301 QRコード決済対応
//		// みなし決済時は理由＝タイムアウトとする
//		alarm_settlement->reason = ALARM_REASON_TIMEOUT;
		// 決済処理中に障害が発生した場合は支払確認失敗とする
		alarm_settlement->reason = ALARM_REASON_CHECKFAIL;
// MH810105(E) MH364301 QRコード決済対応
		break;
	}
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
	// 車室番号
	alarm_settlement->syu = (char)(ryo_buf.syubet + 1);									// 種別

	if( PayData.teiki.update_mon ){														// 更新精算?
		alarm_settlement->WPlace = 9999;												// 更新時は車室番号9999とする。
	}else if( ryo_buf.pkiti == 0xffff ){												// ﾏﾙﾁ精算?
		alarm_settlement->WPlace = LOCKMULTI.LockNo;									// 接客用駐車位置番号
// MH810103 GG119202(S) 精算中止（復）印字で車室番号が印字されない
//	}else if( ryo_buf.pkiti == 0 || ryo_buf.pkiti > LOCK_MAX ){							// 駐車位置番号が範囲外
//		alarm_settlement->WPlace = 0;													// 車室番号0とする
	}else if( ryo_buf.pkiti == 0 ){														// 後日精算？
		// 遠隔精算（後日精算）は駐車位置番号＝０のため、受信した車室情報をセットする
		alarm_settlement->WPlace = OPECTL.Op_LokNo;
// MH810103 GG119202(E) 精算中止（復）印字で車室番号が印字されない
	}else{
		num = ryo_buf.pkiti - 1;
		alarm_settlement->WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
																						// 接客用駐車位置番号
	}
// MH810103(s) 電子マネー対応 未了取引記録対応
	alarm_settlement->CarSearchFlg = PayData.CarSearchFlg;		// 車番/日時検索種別
	memcpy(alarm_settlement->CarSearchData, PayData.CarSearchData, sizeof(alarm_settlement->CarSearchData));// 車番/日時検索データ
	memcpy(alarm_settlement->CarNumber, PayData.CarNumber, sizeof(alarm_settlement->CarNumber));			// 車番(精算)
	memcpy(alarm_settlement->CarDataID, PayData.CarDataID, sizeof(alarm_settlement->CarDataID));			// 車番データID
// MH810103(e) 電子マネー対応 未了取引記録対応

	// 入庫時刻
	alarm_settlement->TInTime.Year = car_in_f.year;										// 入庫 年
	alarm_settlement->TInTime.Mon  = car_in_f.mon;										//      月
	alarm_settlement->TInTime.Day  = car_in_f.day;										//      日
	alarm_settlement->TInTime.Hour = car_in_f.hour;										//      時
	alarm_settlement->TInTime.Min  = car_in_f.min;										//      分

	// ログ管理情報更新
	ctpt[0] = EC_ALARM_LOG_DAT.log_cnt;
	if (ctpt[0] < EC_ALARM_LOG_CNT) {	
		ctpt[0]++;
	}
	ctpt[1] = EC_ALARM_LOG_DAT.log_wpt + 1;
	if (ctpt[1] >= EC_ALARM_LOG_CNT) {
		ctpt[1] = 0;
	}
	nmisave(&EC_ALARM_LOG_DAT.log_cnt, ctpt, 4);
}

// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
//[]----------------------------------------------------------------------[]
///	@brief			処理未了取引の再精算情報登録
//[]----------------------------------------------------------------------[]
///	@param[in]		*data 	: 再精算情報
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/03<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcAlarmLog_RepayLogRegist(Receipt_data *data)
{
	t_ALARM_SETTLEMENT_DATA	*alarm;
	ushort	i;
	ushort	log_cnt;
	int		rpt;

	log_cnt = EcAlarmLog_GetCount();
	rpt = EC_ALARM_LOG_DAT.log_wpt;
	// ライトポインタ-1から検索を開始する
	for (i = 0; i < log_cnt; i++) {
		rpt--;
		if (rpt < 0) {
			rpt = EC_ALARM_LOG_CNT - 1;
		}
		alarm = &EC_ALARM_LOG_DAT.log[rpt];
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//		if (alarm->reason == ALARM_REASON_HIKISARI_OK) {
//			// 未了確定後引き去り済みであれば、再精算情報を登録しない
//			continue;
//		}
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善

// MH810103(s) 電子マネー対応 #5525 処理未了取引に対応する再精算情報が印字されない
//		// 該当する処理未了取引を検索する(車室番号、料金種別、入庫時刻)
//		if (data->WPlace == alarm->WPlace &&
//			data->syu == alarm->syu &&
		// 該当する処理未了取引を検索する(車番データID、入庫時刻)
		if (!memcmp(&data->CarDataID, &alarm->CarDataID, sizeof(alarm->CarDataID)) &&
// MH810103(e) 電子マネー対応 #5525 処理未了取引に対応する再精算情報が印字されない
			!memcmp(&data->TInTime, &alarm->TInTime, sizeof(date_time_rec))) {
			// 再精算情報をセットする
			memcpy(&EC_ALARM_LOG_DAT.paylog[rpt], data, sizeof(*data));
			break;
		}
	}
}
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 処理未了取引ログ件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			処理未了取引ログ件数
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort EcAlarmLog_GetCount( void )
{
	return EC_ALARM_LOG_DAT.log_cnt;
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 処理未了取引ログ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		idx 	: 取得するログの位置(0=最新～4=5件目)
///	@param[out]		*log 	: 決済データ
///	@return			short	: 1:データセットOK、0:件数0、-1:該当データなし
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
//short EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log )
short EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log, Receipt_data *paylog )
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
{
	int	pos;

	if (EC_ALARM_LOG_DAT.log_cnt == 0) {
	// 0件
		return 0;
	}
	
	if ((idx < 0) || (idx >= EC_ALARM_LOG_DAT.log_cnt)) {
	// idx範囲外
		return -1;
	}
	
	pos = (EC_ALARM_LOG_CNT + EC_ALARM_LOG_DAT.log_wpt - (idx + 1)) % EC_ALARM_LOG_CNT;

	// ログコピー
	memcpy(log, &EC_ALARM_LOG_DAT.log[pos], (size_t)EC_ALARM_LOG_DAT.data_size);
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
	memcpy(paylog, &EC_ALARM_LOG_DAT.paylog[pos], sizeof(Receipt_data));
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			未了タイマアウト処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/18<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_miryo_timeout( void )
{
	LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );		/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */

	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;				// 応答がなく、未了タイムアウトした場合

// MH810103 GG119202(S) 未了仕様変更対応
//	if (dsp_change) {
//		op_SuicaFusokuOff();							// 精算未了時の反転ﾒｯｾｰｼﾞ表示を消去する
//		dsp_change = 0;
//		nyukin_delay_check(nyukin_delay, delay_count);	// 保留中のｲﾍﾞﾝﾄがあれば再postする
//		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// モニタ登録
//	}
// MH810103 GG119202(E) 未了仕様変更対応

// MH810103 GG119001_GG119101(S) 未了残高照会中に通信不良発生でみなし決済になる
//// GG119202(S) カード処理中受信後の決済NG（304）の扱い
//	// 決済情報がセットされていないので、ここでセットする
//	// ブランド番号
//	Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;
//	// 支払額
//	Ec_Settlement_Res.settlement_data = Product_Select_Data;
//	// 問い合わせ番号
//	memset(Ec_Settlement_Res.inquiry_num, '*', sizeof(Ec_Settlement_Res.inquiry_num));
//	Ec_Settlement_Res.inquiry_num[4] = '-';
//	switch (Ec_Settlement_Res.brand_no) {
//	case BRANDNO_KOUTSUU:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_KOUTSUU);
//		// SPRWID
//		memset(Ec_Settlement_Res.Brand.Koutsuu.SPRW_ID, '*', sizeof(Ec_Settlement_Res.Brand.Koutsuu.SPRW_ID));
//		break;
//// MHUT40XX(S) Edy・WAON対応
//	case BRANDNO_EDY:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// Edy取引通番
//		memset(Ec_Settlement_Res.Brand.Edy.DealNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.DealNo));
//		// カード取引通番
//		memset(Ec_Settlement_Res.Brand.Edy.CardDealNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.CardDealNo));
//		// 上位端末ID
//		memset(Ec_Settlement_Res.Brand.Edy.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.TerminalNo));
//		break;
//	case BRANDNO_WAON:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// SPRWID
//		memset(Ec_Settlement_Res.Brand.Waon.SPRW_ID, '*', sizeof(Ec_Settlement_Res.Brand.Waon.SPRW_ID));
//		break;
//// MHUT40XX(E) Edy・WAON対応
//// MHUT40XX(S) nanaco・iD・QUICPay対応
//	case BRANDNO_NANACO:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// 端末取引通番
//		memset(Ec_Settlement_Res.Brand.Nanaco.DealNo, '*', sizeof(Ec_Settlement_Res.Brand.Nanaco.DealNo));
//		// 上位端末ID
//		memset(Ec_Settlement_Res.Brand.Nanaco.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Nanaco.TerminalNo));
//		break;
//	case BRANDNO_ID:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// 上位端末ID
//		memset(Ec_Settlement_Res.Brand.Id.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Id.TerminalNo));
//		break;
//	case BRANDNO_QUIC_PAY:
//		// カード番号
//		memset(Ec_Settlement_Res.Card_ID, '*', sizeof(Ec_Settlement_Res.Card_ID));
//		// 上位端末ID
//		memset(Ec_Settlement_Res.Brand.QuicPay.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.QuicPay.TerminalNo));
//		break;
//// MHUT40XX(E) nanaco・iD・QUICPay対応
//	default:
//		break;
//	}
//// GG119202(E) カード処理中受信後の決済NG（304）の扱い
//// GG119202(S) 未了中に異常データ受信でとりけしボタン押下待ちにならない
//	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0) {
//		// 異常データ受信時は制御データ（受付禁止）が送信不可のため、
//		// 状態データ無応答処理を行い、とりけしボタン押下待ちに遷移させる
//		snd_no_response_timeout();
//		return;
//	}
//// GG119202(E) 未了中に異常データ受信でとりけしボタン押下待ちにならない
//	Ec_Pri_Data_Snd(S_CNTL_DATA, 0);					// 制御ﾃﾞｰﾀ（受付禁止）を送信する
// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
//	if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//		// 未了確定前は問合せ(処理中)待ちタイムアウトとする
//		queset(OPETCBNO, EC_INQUIRY_WAIT_TIMEOUT, 0, 0);
//	}
//	else {
//		// 未了確定後は状態データ無応答処理を行い、とりけしボタン押下待ちに遷移させる
//		EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//		snd_no_response_timeout();
//		ac_flg.ec_deemed_fg = 0;						// みなし決済復電用フラグOFF
//	}
	// 未了確定後も問合せ(処理中)待ちタイムアウトとする
	queset(OPETCBNO, EC_INQUIRY_WAIT_TIMEOUT, 0, 0);
// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う
// MH810103 GG119001_GG119101(E) 未了残高照会中に通信不良発生でみなし決済になる
}

//[]----------------------------------------------------------------------[]
///	@brief			複数決済可能か判断処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
/////	@return			1:複数決済可, 0:複数決済不可
///	@return			=  0:決済可能ブランドなし
///					=  1:単一決済有効
///					>= 2:複数決済有効
///	@note			入金状況、精算限度額等をチェックした上で決済可能な
///					ブランド数を返却する
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar get_enable_multisettle(void)
{
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//	uchar ret, eMenoy, credit, houjin;
	uchar ret;
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
	int i, j;

// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//	ret = eMenoy = credit = houjin = 0;
	ret = 0;
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			if ((RecvBrandTbl[i].ctrl[j].status & 0x01) != 0x01) {
//				continue;
//			}
//			switch(RecvBrandTbl[i].ctrl[j].no) {
//				case BRANDNO_KOUTSUU:
//				case BRANDNO_EDY:
//				case BRANDNO_NANACO:
//				case BRANDNO_WAON:
//				case BRANDNO_SAPICA:
//				case BRANDNO_ID:
//				case BRANDNO_QUIC_PAY:
//// GG119202(S) クレジットカード精算限度額設定対応
////					eMenoy = 1;
//					if( WAR_MONEY_CHECK && !e_incnt ){	// 入金なし
//						ret += 1;
//					}
//// GG119202(E) クレジットカード精算限度額設定対応
//					break;
//				case BRANDNO_CREDIT:
//// GG119202(S) クレジットカード精算限度額設定対応
////					credit = 1;
//					if( Ec_check_cre_pay_limit() ){	// クレジット精算可(限度額以下)
//						ret += 1;
//					}
//// GG119202(E) クレジットカード精算限度額設定対応
//					break;
//				case BRANDNO_HOUJIN:
//// GG119202(S) クレジットカード精算限度額設定対応
////					houjin = 1;
//					ret += 1;
//// GG119202(E) クレジットカード精算限度額設定対応
//					break;
//				default:
//					break;
//			}
			if (IsValidBrand(i, j)) {
				ret += 1;
			}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//			if ((eMenoy + credit + houjin) > 1){
//				ret = 1;
//				break;
//			}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
		}

// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//		if (ret == 1) {
//			break;
//		}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
	}

	return ret;
}

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//[]----------------------------------------------------------------------[]
///	@brief			複数決済可能か判断処理(精算状況に依存しない)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			=  0:決済可能ブランドなし
///					=  1:単一決済有効
///					>= 2:複数決済有効
///	@note			決済可能なブランド数を返却する
///	@author			A.Shindoh
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/06/24<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar check_enable_multisettle(void)
{
	uchar ret;
	uchar ret2;
// MH810105(S) MH364301 QRコード決済対応
	uchar QRCheck = FALSE;
// MH810105(E) MH364301 QRコード決済対応
	int i, j;

	ret = 0;
	ret2 = 0;
	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
// MH810103 GG119202(S) ブランド状態判定処理変更
//			if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01) {		// 開局している
			if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[i].ctrl[j].status)) {	// 開局している
// MH810103 GG119202(E) ブランド状態判定処理変更
				switch(RecvBrandTbl[i].ctrl[j].no) {
				case BRANDNO_TCARD:		// T-カード 					
//					if (Tcard_Use_Chk() != 0) {							// Tカード使用可
//						ret += 1;
//					}
					break;
				case BRANDNO_HOUJIN:	// 法人カード
//					if (prm_get(COM_PRM, S_RPK, 1, 1, 1) == 1 &&		// Rism通信設定あり
//						prm_get(COM_PRM, S_RPK, 105, 1, 1) == 1){		// 法人カード設定あり  
//						ret += 1;
//					}
					break;
// MH810105(S) MH364301 QRコード決済対応
				case BRANDNO_QR:		// QRコード決済
					QRCheck = TRUE;
					// no break;
// MH810105(E) MH364301 QRコード決済対応
				default:
					if (IsSettlementBrand(RecvBrandTbl[i].ctrl[j].no)) {
						ret2++;
					}
					ret += 1;
					break;
				}
			}
		}
	}
	if (ret == 1 && ret2 == 0) {
	// 唯一のブランドが非決済ならマルチブランドを選択
		ret = 2;
	}
// MH810103 GG118808_GG118908(S) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
	if (ret == 1 && ECCTL.ec_single_timeout == 1) {
		// シングルブランドで読み取り待ちタイムアウト、キャンセルボタン押下された場合は
		// 以降はマルチブランドとして扱う
		ret += 1;
	}
// MH810103 GG118808_GG118908(E) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
// MH810105(S) MH364301 QRコード決済対応
	if (ret == 1 && QRCheck == TRUE) {
		// QRコード決済のシングルブランドはマルチブランドとして扱う
		ret = 2;
	}
// MH810105(E) MH364301 QRコード決済対応
	if (ret > 1) {
	// ブランド選択処理停止後は０にする
		if (ECCTL.brandsel_stop != 0) {
			ret = 0;
		}
	}

	return ret;
}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

// MH810103 GG119202(S) 開局・有効条件変更
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダ 精算可否判定
////[]----------------------------------------------------------------------[]
/////	@param[in]		kind:	判定種別
/////	@return			1:精算可, 0:精算不可
/////	@author			hosoda
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/05<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isEcReady( const uchar kind )
//{
//	uchar ret = 0;
//
//	// 共通チェック
//	if ( isEC_USE()											// 決済リーダ使用設定
//		&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
//// GG119202(S) 精算可能判定処理修正
//		&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
//// GG119202(E) 精算可能判定処理修正
//// GG119202(S) 接客画面の電子マネー対応
//		&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
//// GG119202(E) 接客画面の電子マネー対応
//		// TODO: 判定条件設定 (仕様調整中)
//		// ジャーナルプリンタが使用可能
//		// レシートプリンタが使用可能(使用不可時に決済不可設定時)
//	) {
//		// 判定種別ごとのチェック
//		switch (kind)
//		{
//		case EC_CHECK_STATUS:								// 精算不可ではないか
//			return 1;
//			break;
//
//		case EC_CHECK_EMONEY:								// 電子マネーで精算可能か
//// GG119202(S) 不要処理削除
////			if ( isAnyEcBrandReady(EcBrandEmoney, TBL_CNT(EcBrandEmoney)) ) {
//			if (isEcEmoneyEnabled(1, 0)) {
//// GG119202(E) 不要処理削除
//				ret = 1;
//			}
//			break;
//
//		case EC_CHECK_CREDIT:								// クレジットで精算可能か
//// GG119202(S) 不要処理削除
////			if ( isAnyEcBrandReady(EcBrandCredit, TBL_CNT(EcBrandCredit)) ) {
//			if (isEcBrandNoReady(BRANDNO_CREDIT)) {
//// GG119202(E) 不要処理削除
//				ret = 1;
//			}
//			break;
//
//		default:
//			break;
//		}	
//	}
//
//	return ret;
//}
//[]----------------------------------------------------------------------[]
///	@brief			ブランド利用可能チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	ブランド番号
///	@return			1:利用可, 0:利用不可
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/10/7<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//　ブランドが精算に利用できるかをチェックする。
//　リーダーのエラー状態などは下部関数では見ていないためここで実施する。
//　単に開局しているかはisEcBrandNoReady()を使用すること。
//　電子マネーを複合的にチェックしたい場合は、isEcEmoneyEnabled()を使用すること。(Ready,Enable両対応です。)
uchar isEcBrandNoEnabled(ushort brand_no)
{
	uchar	ret = 0;

	// 共通チェック
	if ( isEC_USE()											// 決済リーダ使用設定
		&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
		&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
		&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
	) {
		if (ec_check_valid_brand(brand_no)) {
			ret = 1;
		}
	}
	return ret;
}
// MH810103 GG119202(E) 開局・有効条件変更
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
uchar isEcBrandNoEnabledNoCtrl(ushort brand_no)
{
	uchar	ret = 0;

	// 共通チェック
	if ( isEC_USE()											// 決済リーダ使用設定
		&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
		&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
//		&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
	) {
		if (ec_check_valid_brand(brand_no)) {
			ret = 1;
		}
	}
	return ret;
}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える

// MH810103 GG119202(S) 不要処理削除
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダ 精算可否判定 (指定ブランドのうちいずれか)
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_ec_ary:	判定するブランドの配列 (EC_*_USED)
/////	@param[in]		ary_size: brand_ec_aryの要素数
/////	@return			1:いずれかで精算可, 0:すべて精算不可
/////	@author			hosoda
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/20<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isAnyEcBrandReady(const ushort *brand_ec_ary, const ushort ary_size)
//{
//	uchar	ret = 0;
//	int		i;
//	ushort	brand_no;
//
//	for(i = 0; i < ary_size; i++)
//	{
//		brand_no = ConvertEcBrandToNo(brand_ec_ary[i]);		// EC_*_USEDからBRANDNO_*への変換
//		if ( brand_no == 0 ) {
//		// 不明ブランド
//			continue;
//		}
//
//		if ( isEcBrandNoReady(brand_no) ) {
//		// ブランドが精算可能
//			ret = 1;
//			break;			// 1ブランドでも精算可能なら判定終了
//		}
//	}
//
//	return ret;
//}
// MH810103 GG119202(E) 不要処理削除

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 精算可否判定 (指定ブランド番号)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	判定するブランド番号(BRANDNO_*)
///	@return			1:精算可, 0:精算不可
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoReady(const ushort brand_no)
{
	uchar	ret = 0;
	uchar	found = 0;
	int		i, j;

	for(i = 0; i < TBL_CNT(RecvBrandTbl); i++)
	{
		for(j = 0; j < RecvBrandTbl[i].num; j++)
		{
			if ( RecvBrandTbl[i].ctrl[j].no == brand_no ) {
// MH810103 GG119202(S) ブランド状態判定処理変更
//				if ( (RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01 ) {
//				// 無効,一件明細満杯,サービス停止のいずれでもなく開局済み
				if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[i].ctrl[j].status)) {
					// 開局＆有効
// MH810103 GG119202(E) ブランド状態判定処理変更
					ret = 1;
				}
				found = 1;
				break;
			}
		}
		if ( found ) {
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 電子マネー (指定ブランド番号)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	判定するブランド番号(BRANDNO_*)
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
///	@param[in]		zandaka_chk:残高照会可能かチェック
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
///	@return			1:電子マネー, 0:電子マネー以外
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/03/18<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//uchar isEcBrandNoEMoney(const ushort brand_no)
uchar isEcBrandNoEMoney(const ushort brand_no, uchar zandaka_chk)
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
{
	uchar	ret = 0;

	switch(brand_no)
	{
		case BRANDNO_KOUTSUU:
		case BRANDNO_EDY:
		case BRANDNO_NANACO:
		case BRANDNO_WAON:
		case BRANDNO_SAPICA:
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
// MH810105(S) MH364301 PiTaPa対応
		case BRANDNO_PITAPA:
// MH810105(E) MH364301 PiTaPa対応
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
			if (zandaka_chk) {
				// 残高照会可能チェック
// MH810105(S) MH364301 PiTaPa対応
//				switch (brand_no) {
//				case BRANDNO_ID:
//				case BRANDNO_QUIC_PAY:
//					// 残高照会不可
//					return 0;
//				default:
//					// 残高照会可能
//					break;
//				}
				if(EcBrandPostPayCheck(convert_brandno(brand_no))) {
					// 残高照会不可
					return 0;
				}
// MH810105(E) MH364301 PiTaPa対応
			}
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			EC_*_USEDからBRANDNO_*への変換
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_ec:	ブランド(EC_*_USED)
///	@return			ブランド番号(BRANDNO_*), 0:変換不可
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort ConvertEcBrandToNo(const ushort brand_ec)
{
	ushort	brand_no = 0;
	int		i;

	// 非決済ブランドには対応しない
	for(i = 0; i < TBL_CNT(EcBrandNoConvTbl); i++)
	{
		if ( brand_ec == EcBrandNoConvTbl[i].brand_ec ) {
			brand_no = EcBrandNoConvTbl[i].brand_no;
			break;
		}
	}

	return brand_no;
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ 使用設定判定 (指定ブランド番号)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	判定するブランド番号(BRANDNO_*)
///	@return			1:使用する, 0:使用しない
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoEnabledForSetting(const ushort brand_no)
{
	uchar	ret = 0;
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//	int		i;
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH810103 GG119202(S) 決済ありブランド選択設定を参照する
	ushort	i;
// MH810103 GG119202(E) 決済ありブランド選択設定を参照する

// MH810103 GG116201(S) 決済リーダ接続なしならブランド使用設定を参照しない
	if( isEC_USE() == 0 ){
		return ret;
	}
// MH810103 GG116201(E) 決済リーダ接続なしならブランド使用設定を参照しない

// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//	for(i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++) {
//		if ( brand_no == EcBrandEnabledParamTbl[i].brand_no ) {
//			if (prm_get(COM_PRM, S_ECR, EcBrandEnabledParamTbl[i].address, 1, EcBrandEnabledParamTbl[i].pos) != 0) {
//				ret = 1;
//			}
//			break;
//		}
//	}
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
// MH810103 GG119202(S) 決済ありブランド選択設定を参照する
	if (IsSettlementBrand(brand_no)) {
		// 決済ありブランド
		ret = 1;
		for (i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++) {
			if (brand_no == EcBrandEnabledParamTbl[i].brand_no) {
				if (prm_get(COM_PRM, S_ECR, EcBrandEnabledParamTbl[i].address, 1, EcBrandEnabledParamTbl[i].pos) == 0) {
					// 使用なし
					ret = 0;
				}
				break;
			}
		}
	}
	else {
		// 決済なしブランド／後決済ブランド
// MH810103 GG119202(E) 決済ありブランド選択設定を参照する
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	switch(brand_no) {
	case BRANDNO_TCARD:
	case BRANDNO_HOUJIN:
//		if (prm_get(COM_PRM, S_RPK, 1, 1, 1) == 1) {		// Rism通信設定あり
//			ret = 1;
//		} else {
//			ret = 0;
//		}
		break;
	default:
		ret = 1;
		break;
	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 決済ありブランド選択設定を参照する
	}
// MH810103 GG119202(E) 決済ありブランド選択設定を参照する

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド有効判定（受信ブランドテーブルから）
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	判定するブランド番号(BRANDNO_*)
///	@return			1:有効, 0:無効
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoEnabledForRecvTbl(const ushort brand_no)
{
	uchar	ret = 0;
	int		i, j;

// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
	if (!isEC_USE()) {
		return ret;
	}
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// 受信したブランドテーブルと一致するか？
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//// GG119202(S) 未了仕様変更対応
////			if (brand_no == RecvBrandTbl[i].ctrl[j].no) {
//			if (brand_no == RecvBrandTbl[i].ctrl[j].no &&
//				RecvBrandTbl[i].ctrl[j].status == 0x01) {
//			// 受信した状態：開局の場合、有効
//// GG119202(E) 未了仕様変更対応
			if (brand_no == RecvBrandTbl[i].ctrl[j].no) {
				// 開局状態は見ない
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
				ret = 1;
				break;
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}

// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダ ブランド有効判定
////[]----------------------------------------------------------------------[]
/////	@param[in]		kind:	判定種別
/////	@return			1:有効, 0:無効
/////	@author			T.Nagai
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/09/10<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isEcEnabled( const uchar kind )
//{
//	uchar ret = 0;
//
//	// 共通チェック
//	if ( isEC_USE() ) {										// 決済リーダ使用設定
//		// 判定種別ごとのチェック
//		switch (kind) {
//		case EC_CHECK_EMONEY:								// 電子マネーで精算可能か
//			if ( isAnyEcBrandEnabled(EcBrandEmoney, TBL_CNT(EcBrandEmoney)) ) {
//				ret = 1;
//			}
//			break;
//
//		case EC_CHECK_CREDIT:								// クレジットで精算可能か
//			if ( isAnyEcBrandEnabled(EcBrandCredit, TBL_CNT(EcBrandCredit)) ) {
//				ret = 1;
//			}
//			break;
//
//		default:
//			break;
//		}
//	}
//
//	return ret;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダ ブランド有効判定 (指定ブランドのうちいずれか)
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_ec_ary:	判定するブランドの配列 (EC_*_USED)
/////	@param[in]		ary_size: brand_ec_aryの要素数
/////	@return			1:いずれか有効, 0:すべて無効
/////	@author			T.Nagai
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/09/10<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isAnyEcBrandEnabled(const ushort *brand_ec_ary, const ushort ary_size)
//{
//	uchar	ret = 0;
//	int		i;
//	ushort	brand_no;
//
//	for(i = 0; i < ary_size; i++) {
//		brand_no = ConvertEcBrandToNo(brand_ec_ary[i]);		// EC_*_USEDからBRANDNO_*への変換
//		if ( brand_no == 0 ) {
//		// 不明ブランド
//			continue;
//		}
//
//		if ( isEcBrandNoEnabledForRecvTbl(brand_no) ) {
//		// ブランドが受信したブランドテーブルと一致するか
//			ret = 1;
//			break;			// 1ブランドでも一致するなら判定終了
//		}
//	}
//
//	return ret;
//}
//[]----------------------------------------------------------------------[]
///	@brief			電子マネー利用可能チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		open_chk:		決済可能チェック
///	@param[in]		zandaka_chk:	残高照会可能チェック
///	@return			1:電子マネー利用可, 0:電子マネー利用不可
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar isEcEmoneyEnabled(uchar open_chk, uchar zandaka_chk)
{
	uchar	ret = 0;
	int		i, j;

	if (!isEC_USE()) {
		return ret;
	}

// MH810103 GG119202(S) 開局・有効条件変更
	if (open_chk || zandaka_chk) {
		if (!Suica_Rec.suica_err_event.BYTE					// リーダー使用可
			&& Suica_Rec.Data.BIT.INITIALIZE				// 初期化完了
			&& Suica_Rec.Data.BIT.CTRL) {					// 受付可状態
			;		// コンディションOK
		}
		else {
			return ret;
		}
	}
// MH810103 GG119202(E) 開局・有効条件変更

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// 受信したブランドテーブルに電子マネーブランドが含まれているか？
			if (isEcBrandNoEMoney(RecvBrandTbl[i].ctrl[j].no, zandaka_chk)) {
				if (open_chk) {
					// 決済可能チェック
// MH810103 GG119202(S) 開局・有効条件変更
//					if (!SUICA_USE_ERR &&							// エラー未発生
//// GG119202(S) 接客画面の電子マネー対応
//						Suica_Rec.Data.BIT.CTRL &&					// 受付可状態
//// GG119202(E) 接客画面の電子マネー対応
//						RecvBrandTbl[i].ctrl[j].status == 0x01) {	// 開局・有効
					if (IsValidBrand(i, j)) {
// MH810103 GG119202(E) 開局・有効条件変更
						ret = 1;
						break;
					}
				}
				else {
					ret = 1;
					break;
				}
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない

// MH810103 GG119202(S) ブランド名はブランド情報から取得する
//[]----------------------------------------------------------------------[]
//	@brief		ブランド名取得
//[]----------------------------------------------------------------------[]
//	@param[in]	*brand	: ブランド情報
//	@param[out]	*buff	: ブランド名
//	@return		ブランド名の長さ
//[]----------------------------------------------------------------------[]
//	@author		MATSUSHITA
//	@date		Create	: 20/10/01<br>
//				Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
int	getBrandName(EC_BRAND_CTRL *brand, uchar *buff)
{
	int		i, j;

	j = (int)sizeof(brand->name);

	for (i = 0; i < j && brand->name[i] != '\0'; i++) {}	// 文字列終端へ
	for (i--; i >= 0 && (brand->name[i] == ' '); i--) {}		// スペースカット
	i++;
	if (i == 0) {
		buff[0] = '(';
		buff[5] = ')';
		intoasl(&buff[1], (ulong)brand->no, 4);
		i = 6;
	}
	else {
		memcpy(buff, brand->name, (size_t)i);
	}
	buff[i] = '\0';
	return i;
}
// MH810103 GG119202(E) ブランド名はブランド情報から取得する

//[]----------------------------------------------------------------------[]
///	@brief			初回表示画面判定関数
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret ：0:割引(入金済み)画面 1:初回画面
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/03/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
char	EcFirst_Pay_Dsp( void )
{
	char	ret = 0;

	if((0 == ryo_buf.waribik) &&
	   (0 == c_pay) &&
	   (0 == e_pay) &&
	   ( !carduse() ) &&
	   ( vl_ggs.ryokin == 0 &&								// vl_ggs ： 減額精算領域 が0の時にｻｲｸﾘｯｸ表示再開
	   ( vl_frs.lockno == 0 || vl_frs.antipassoff_req) ) &&	// vl_frs ： 振替精算領域 が0、もしくは定期利用有りの時にｻｲｸﾘｯｸ表示再開
	   (ryo_buf.nyukin == 0)){
		ret = 1;
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EcEleUseDsp
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/04/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_EcEleUseDsp( void )
{
	e_incnt = 0;															// 精算内の「電子マネー」使用回数（含む中止）をクリア
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	memset(&RecvBrandResTbl, 0x00, sizeof(RecvBrandResTbl));				// ブランド選択結果データ初期化
//	RecvBrandResTbl.no = (ushort)BRANDNO_UNKNOWN;							// 選択ブランド番号は不明で初期化
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	timer_recept_send_busy = 0;
	ECCTL.timer_recept_send_busy = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) ブランド選択シーケンス不具合
	EcBrandClear();
// MH810103 GG119202(E) ブランド選択シーケンス不具合
	EcSendCtrlEnableData();
	Suica_Rec.Data.BIT.PAY_CTRL = 0;										// Suica精算ﾌﾗｸﾞをﾘｾｯﾄ
	DspChangeTime[0] = (ushort)CPrmSS[S_SCA][6];							// 残高表示時間ｾｯﾄ

	DspChangeTime[1] = 2; 													// 画面切り替え時間ｾｯﾄ

	Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[1]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ起動
	Suica_Rec.Data.BIT.SEND_CTRL80 = 0;										// 取引終了データ送信状態フラグクリア
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;									// 未了タイムアウトフラグクリア
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
}

//[]----------------------------------------------------------------------[]
///	@brief			recv_unexpected_status_data
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/05/09<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void recv_unexpected_status_data(void)
{
// MH321800(S) E3269を登録しない仮対応
//	memset(err_wk, 0, sizeof(err_wk));
//
//	// オペレーションモード
//	intoas(&err_wk[0], (ushort)OPECTL.Mnt_mod, 2);
//	err_wk[2] = '-';
//	intoas(&err_wk[3], (ushort)OPECTL.Ope_mod, 3);
//	err_wk[6] = ' ';
//	// MVT状態データ
//	hxtoas2(&err_wk[7], EC_STATUS_DATA.status_data);
//
//	err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_IGL_STS_DATA, 2, 1, 1, err_wk);	/* ｴﾗｰﾛｸﾞ登録（登録）*/
// MH321800(E) E3269を登録しない仮対応
}

//[]----------------------------------------------------------------------[]
///	@brief			SetEcVolume
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	: 2019/05/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void SetEcVolume( void )
{
	short i;
	memset(&VolumeTbl, 0x0,sizeof(EC_VOLUME_TBL));
	VolumeTbl.num = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if(VolumeTbl.num > 3) {
		VolumeTbl.num = 3;
	}

	if(VolumeTbl.num == 0) {
		VolumeTbl.ctrl[0].time = (ushort)prm_get(COM_PRM, S_SYS, 54, 4, 1);
		VolumeTbl.ctrl[0].vol = (uchar)prm_get(COM_PRM, S_ECR, 27, 2, 5);
		if(VolumeTbl.ctrl[0].vol == 0) {
			VolumeTbl.ctrl[0].vol = 1;
		} else if(VolumeTbl.ctrl[0].vol > EC_MAX_VOLUME) {
			VolumeTbl.ctrl[0].vol = EC_MAX_VOLUME;
		}
	} else {
		for(i=0; i < VolumeTbl.num; i++) {
			VolumeTbl.ctrl[i].time = (ushort)prm_get(COM_PRM, S_SYS, (short)(54+i), 4, 1);
			VolumeTbl.ctrl[i].vol = (uchar)(prm_get(COM_PRM, S_ECR, 27, 2, (char)(5-(i*2))));
			if(VolumeTbl.ctrl[i].vol > EC_MAX_VOLUME) {
				VolumeTbl.ctrl[i].vol = EC_MAX_VOLUME;
			}
		}
	}
}

// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
////[]----------------------------------------------------------------------[]
/////	@brief			EcDeemedSettlement
////[]----------------------------------------------------------------------[]
/////	@param[in]		uchar *ope_faze
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			Y.Tanizaki
/////	@date			Create	: 2019/06/10<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void EcDeemedSettlement( uchar *ope_faze )
//{
//// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//	uchar	sts = ECCTL.Ec_Settlement_Sts;
//// MH810103 GG119202(E) 決済状態に「みなし決済」追加
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
////	uchar	brand_index = 0;
////// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
//	uchar	process = 0;											// 0：みなし決済で精算完了
//																	// 1：休業
//																	// 2：クレジット特有の処理(決済結果NGとする)
//																	// 3：QRコード特有の処理(障害連絡表発行)
//// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
//
//	// 状態のクリア
//	OPECTL.InquiryFlg = 0;											// カード処理中フラグOFF
//// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
////	Ec_Settlement_Sts = EC_SETT_STS_NONE;							// 決済状態のクリア
//	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;						// 決済状態のクリア
//// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
//// MH810105 GG119202(S) 未了残高照会中に障害が発生した場合は設定50-0014に従う
////// MH810103 GG119001_GG119101(S) 未了残高照会中に通信不良発生でみなし決済になる
////	if (Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) {
////		ec_miryo_timeout();
////		return;
////	}
////// MH810103 GG119001_GG119101(E) 未了残高照会中に通信不良発生でみなし決済になる
//// MH810105 GG119202(E) 未了残高照会中に障害が発生した場合は設定50-0014に従う
//
//// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
////// MH810103 GG119202(S) みなし決済扱い時の動作
////	// みなし決済扱い時の動作（50-0014）
////	if (prm_get(COM_PRM, S_ECR, 14, 1, 1) == 1) {
////		// 休業とする
//	// 決済処理中に障害が発生した時の動作(50-0014)
//	process = EcGetActionWhenFailure( EcDeemedLog.EcDeemedBrandNo );
//
//	if( process == 2 || process == 3 ){
//		// 決済結果NGとして精算継続(クレジット、電子マネー（ポストペイ型）)
//		// 障害連絡票を発行し決済結果NGとして精算継続(QRコード)
//		Ec_Settlement_Res.Result = EPAY_RESULT_NG;					// 決済結果NGとして処理させる
//		OPECTL.InquiryFlg = 1;										// カード処理中フラグON
//
//		// 決済結果データを受信したことにする
//		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
//		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
//		Suica_Rec.Data.BIT.CTRL_CARD = 0;							// みなし決済時はカード差込OFFにする(ONでもOFFを検出できないため)
//		Ec_Settlement_Res.Column_No = 0xFF;
//		EcRecvSettData(ope_faze);									// 決済結果NGを受信したとして処理させる
//
//		if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {					// 未了状態発生中？ 
//			LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);			// 未了状態監視ﾀｲﾏﾘｾｯﾄ
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// 未了状態解除
//		}
//		Suica_Rec.Data.BIT.CTRL = 0;
//		EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;					// 状態ﾃﾞｰﾀ内容に受付不可ｾｯﾄ
//		EcRecvStatusData(ope_faze);
//// MH810105(S) #6199 決済処理中の障害発生でみなし決済＋決済結果OK受信時、50-0014の設定を無視して処理が完了する
//		// 状態を更新
//		dspCyclicErrMsgRewrite();
//
//		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// ポップアップ消去(正常終了)
//// MH810105(E) #6199 決済処理中の障害発生でみなし決済＋決済結果OK受信時、50-0014の設定を無視して処理が完了する
//
//		if( process == 3 ){
//// TODO：障害連絡表発行
//		}
//		return;
//	}
//	else if( process == 1 ){
//		// 休業とする
//// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作
//
//// MH810103 GG119202(S) 決済状態（みなし決済）の判定修正
//		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// 問合せ(処理中)結果待ちタイマーリセット
//// MH810103 GG119202(E) 決済状態（みなし決済）の判定修正
//		ec_MessageAnaOnOff(0, 0);
//// MH810103 GG118808_GG118908(S) 現金投入と同時にカードタッチ→未了発生でロックする
//		if( ryo_buf.nyukin && OPECTL.credit_in_mony ){				// 入金ありで電子マネーとのすれ違い入金の場合
//			// 入金額 >= 駐車料金の状態でみなし決済の扱い＝休業の場合は
//			// 精算完了として扱い、とりけしボタン押下待ちにしない
//			if( OPECTL.credit_in_mony == 1 ){
//				in_mony ( COIN_EVT, 0 );							// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//			}
//			else{/* OPECTL.credit_in_mony == 2 */
//				in_mony ( NOTE_EVT, 0 );							// 紙幣ｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//			}
//			OPECTL.credit_in_mony = 0;								// すれ違いﾌﾗｸﾞｸﾘｱ
//		}
//
//		if (ryo_buf.zankin > 0) {
//			// 残金>0の場合のみ表示する
//// MH810103 GG118808_GG118908(E) 現金投入と同時にカードタッチ→未了発生でロックする
//
//// MH810103 GG119202(S) 決済状態に「みなし決済」追加
////		EcRegistSettlementAbort();
//		// 決済処理失敗(休業)
//		EcRegistDeemedSettlementError(1);
//// MH810103 GG119202(E) 決済状態に「みなし決済」追加
//
//// MH810103(s) 電子マネー対応 未了通知
//		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0A);	// 10=10=みなし決済休業案内
//// MH810103(e) 電子マネー対応 未了通知
//// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
////		Ope2_ErrChrCyclicDispStart(2000, msg_idx);	// ワーニングサイクリック表示(2秒間隔)
//		Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);	// ワーニングサイクリック表示(2秒間隔)
//// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
//		LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER, (short)((30 * 2) + 1), ec_auto_cancel_timeout);	// 精算自動キャンセルタイマ開始
//// MH810103 GG118808_GG118908(S) 現金投入と同時にカードタッチ→未了発生でロックする
//		}
//// MH810103 GG118808_GG118908(E) 現金投入と同時にカードタッチ→未了発生でロックする
//
//// MH810103 GG119202(S) 未了中に通信不良で休業画面に遷移しない
//		if(Suica_Rec.Data.BIT.CTRL_MIRYO) {
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// 未了状態解除
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
////// GG119202(S) カード処理中受信後の決済NG（304）の扱い
////			if (!IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
////// GG119202(E) カード処理中受信後の決済NG（304）の扱い
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// 応答がなく、未了タイムアウトした場合
//			Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;					// 未了タイムアウト発生
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// モニタ登録
////// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//////// GG119202(S) カード処理中受信後の決済NG（304）の扱い
//////			}
//////// GG119202(E) カード処理中受信後の決済NG（304）の扱い
////			// 「みなし決済扱い後の動作＝休業」の場合も未了回数をカウントする
////			brand_index = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
////			ec_alarm_syuu(brand_index, (ulong)Product_Select_Data);	// アラーム取引集計
////			if (sts != EC_SETT_STS_DEEMED_SETTLEMENT) {
////				// 決済状態＝「みなし決済」以外はみなし用決済結果データを作成する
////				EcSetDeemedSettlementResult(&Ec_Settlement_Res);
////			}
////			ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// アラームレシート印字要求
////			EcAlarmLog_Regist(&Ec_Settlement_Res);					// 処理未了取引ログ登録
////// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//		}
//		Suica_Rec.Data.BIT.CTRL = 0;
//		Suica_Rec.Data.BIT.OPE_CTRL = 0;
//// MH810103 GG119202(E) 未了中に通信不良で休業画面に遷移しない
//		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;		// ｱﾗｰﾑﾒｯｾｰｼﾞ表示用ﾌﾗｸﾞｾｯﾄ
//		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;		// 商品選択データ送信可否フラグクリア
//// MH810103 GG119202(S) カード処理中受信後の決済NG（304）の扱い
//		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;		// 決済中判定フラグクリア
//// MH810103 GG119202(E) カード処理中受信後の決済NG（304）の扱い
//// MH810104 GG119202(S) みなし休業時のフラグクリア
//		Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;		// 待ち合わせフラグクリア
//		Suica_Rec.suica_rcv_event.BYTE = 0;			// 受信イベントフラグクリア
//// MH810104 GG119202(E) みなし休業時のフラグクリア
//		ac_flg.ec_deemed_fg = 0;					// みなし決済復電用フラグOFF
//		return;
//	}
//// MH810103 GG119202(E) みなし決済扱い時の動作
//
//// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//	// みなし決済(精算完了)
//	EcRegistDeemedSettlementError(0);
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//	// みなし決済時はdeemed_sett_finフラグをONにする
//	// deemed_sett_finフラグのみONされている時は、みなし決済＋決済結果Ok受信となる
//	// 通常のみなし決済はdeemed_sett_finとdeemed_settlementのフラグがONになる
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin = 1;
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
//	if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
//		// 決済状態＝「みなし決済」は決済結果データも同時に通知されるため、
//		// エラー登録のみとする
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// 未了状態解除
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// 応答がなく、未了タイムアウトした場合
//// MH810105 GG119202(S) 未了確定の直取データ受信時にR0176を登録する
////			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// モニタ登録
//// MH810105 GG119202(E) 未了確定の直取データ受信時にR0176を登録する
//		}
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//		return;
//	}
//// MH810103 GG119202(E) 決済状態に「みなし決済」追加
//
//	// みなし決済データ作成
//	memset(&Ec_Settlement_Res,0, sizeof(Ec_Settlement_Res));
//	Ec_Settlement_Res.Result = EPAY_RESULT_OK;						// 決済結果は常にOK
//	Ec_Settlement_Res.Column_No = 0xFF;								// 決済コラム番号は通常1のため、0xFFをみなし決済とする
//	Ec_Settlement_Res.settlement_data = Product_Select_Data;		// 選択商品データで送信した決済額をセット
//
//	// ブランド番号は決済状態データ受信時に保存したものを使用する
//	Ec_Settlement_Res.brand_no = (ushort)EcDeemedLog.EcDeemedBrandNo;
//
//	// 精算日時は不明のため精算機の日時をセット
//	memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(struct clk_rec));
//
//	if(Suica_Rec.Data.BIT.CTRL_MIRYO) {
//		LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );				/* 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ */
//		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;							// 未了状態解除
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
////// GG119202(S) カード処理中受信後の決済NG（304）の扱い
////		if (!IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
////// GG119202(E) カード処理中受信後の決済NG（304）の扱い
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//		Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;						// 応答がなく、未了タイムアウトした場合
//// MH810105 GG119202(S) 未了確定の直取データ受信時にR0176を登録する
////		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);					// モニタ登録
//// MH810105 GG119202(E) 未了確定の直取データ受信時にR0176を登録する
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
////// GG119202(S) カード処理中受信後の決済NG（304）の扱い
////		}
////// GG119202(E) カード処理中受信後の決済NG（304）の扱い
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
//	}
//
//// MH810103 GG119202(S) 決済状態に「みなし決済」追加
////// GG116201(S) みなし決済発生時にエラー登録する
////	// みなし決済エラー登録(発生解除同時)
////	err_chk((char)jvma_setup.mdl, ERR_EC_DEEMED_SETTLEMENT, 2, 0, 0);
////// GG116201(E) みなし決済発生時にエラー登録する
//// MH810103 GG119202(E) 決済状態に「みなし決済」追加
//
//	// 決済結果データを受信したことにする
//// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
////	Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;
//	Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
//// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
//	Suica_Rec.Data.BIT.LOG_DATA_SET = 1;
//	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;
//// MH810103 MHUT40XX(S) Edy・WAON対応
//	Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
//// MH810103 MHUT40XX(E) Edy・WAON対応
//	Suica_Rec.Data.BIT.CTRL_CARD = 0;								// みなし決済時はカード差込OFFにする(ONでもOFFを検出できないため)
//	EcRecvSettData(ope_faze);
//
//	// 状態データを受信したことにする
//	Suica_Rec.Data.BIT.CTRL = 0;
//	EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;						// 状態ﾃﾞｰﾀ内容に受付不可ｾｯﾄ
//	EcRecvStatusData(ope_faze);
//}
//[]----------------------------------------------------------------------[]
///	@brief			決済処理中に障害が発生した時の動作
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze	: OPEの状態
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/16
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void EcDeemedSettlement(uchar *ope_faze)
{
	uchar	sts = ECCTL.Ec_Settlement_Sts;
	uchar	ec_result = Ec_Settlement_Res.Result;
	uchar	process = 0;											// 0：決済OKとして精算完了
																	// 1：休業
																	// 2：精算中止
																	// 3：QRコード決済特有の処理（障害連絡票発行）
// MH810105(S) MH364301 休業案内に問い合わせ先を表示する
//	const uchar	msg_idx[4] = {										// 休業時のメッセージ
//		76, 50,														// "＜取引に失敗しました＞"
//																	// "係員をお呼び下さい"
//		84, 52,														// "とりけしボタンを押すと"
//																	// "画面を終了します"
//	};
	uchar	msg_idx[4] = {											// 休業時のメッセージ
		76,  0,														// "取引に失敗しました"
																	// ""
		84, 52,														// "とりけしボタンを押すと"
																	// "画面を終了します"
	};
	uchar	disp;
// MH810105(E) MH364301 休業案内に問い合わせ先を表示する


	// 状態のクリア
	OPECTL.InquiryFlg = 0;											// カード処理中フラグOFF
	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;						// 決済状態のクリア

	if (ec_result == EPAY_RESULT_MIRYO) {
		// 未了残高照会タイムアウト時の動作
		process = EcGetMiryoTimeoutAction(EcDeemedLog.EcDeemedBrandNo);
	}
	else {
		// 決済処理中に障害が発生した時の動作
		process = EcGetActionWhenFailure(EcDeemedLog.EcDeemedBrandNo);
	}
	if (process == 0) {
		// 0：決済結果OK（みなし決済）として精算完了
		EcRegistDeemedSettlementError(0);							// みなし決済（精算完了）

		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			// 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ
			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// 未了状態解除
			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// 応答がなく、未了タイムアウトした場合
		}

		// みなし決済時はdeemed_sett_finフラグをONにする
		// deemed_sett_finフラグのみONされている時は、みなし決済＋決済結果OK受信となる
		// 通常のみなし決済はdeemed_sett_finとdeemed_settlementのフラグがONになる
		PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin = 1;
		if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
			// 決済状態＝「みなし決済」は決済結果データも同時に通知されるため、
			// エラー登録のみとする
			return;
		}

		if (isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 1) &&
			ec_result == EPAY_RESULT_MIRYO) {
			// プリペイド型で決済→未了確定受信時
			// 決済OK受信処理を行うため、フラグクリアする
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 0;
		}

		// みなし決済データ作成
		memset(&Ec_Settlement_Res, 0, sizeof(Ec_Settlement_Res));
		Ec_Settlement_Res.Result = EPAY_RESULT_OK;					// 決済結果は常にOK
		Ec_Settlement_Res.Column_No = 0xFF;							// 決済コラム番号は通常1のため、0xFFをみなし決済とする
		Ec_Settlement_Res.settlement_data = Product_Select_Data;	// 選択商品データで送信した決済額をセット

		// ブランド番号は決済状態データ受信時に保存したものを使用する
		Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;

		// 精算日時は不明のため精算機の日時をセット
		memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(CLK_REC));

		// 決済結果データを受信したことにする
		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
		Suica_Rec.Data.BIT.LOG_DATA_SET = 1;
		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;
		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
		Suica_Rec.Data.BIT.CTRL_CARD = 0;							// みなし決済時はカード差込OFFにする(ONでもOFFを検出できないため)
		EcRecvSettData(ope_faze);

		// 状態データを受信したことにする
		Suica_Rec.Data.BIT.CTRL = 0;
		EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;					// 状態ﾃﾞｰﾀ内容に受付不可ｾｯﾄ
		EcRecvStatusData(ope_faze);
	}
	else {
		// 1：休業
		// 2：精算中止
		// 3：障害連絡票を発行して精算中止

		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// 問合せ(処理中)結果待ちタイマーリセット
		ec_MessageAnaOnOff(0, 0);

		if( ryo_buf.nyukin && OPECTL.credit_in_mony ){				// 入金ありで電子マネーとのすれ違い入金の場合
			// 入金額 >= 駐車料金の状態で
			// 決済処理中に障害が発生した時の動作＝休業、精算中止の場合は
			// 精算完了として扱い、とりけしボタン押下待ちにしない
			in_mony ( COIN_EVT, 0 );								// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
			OPECTL.credit_in_mony = 0;								// すれ違いﾌﾗｸﾞｸﾘｱ
		}

		if (ryo_buf.zankin > 0) {
			// 残金がある場合
			switch (process) {
			case 1:		// 休業
				if (ec_result != EPAY_RESULT_MIRYO) {
					EcRegistDeemedSettlementError(1);				// 決済処理失敗（休業）
				}
// MH810105(S) MH364301 休業案内に問い合わせ先を表示する
//				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);		// ワーニングサイクリック表示(2秒間隔)
				// 問い合わせ先表示
				disp = (uchar)prm_get(COM_PRM, S_DSP, 37, 1, 1);
				if (disp > 5) {
					disp = 0;
				}

				msg_idx[1] = (uchar)(disp + 78);
				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 1);		// ワーニングサイクリック表示(2秒間隔)
// MH810105(E) MH364301 休業案内に問い合わせ先を表示する
				LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER,
							(short)((30 * 2) + 1),
							ec_auto_cancel_timeout);				// 精算自動キャンセルタイマ開始
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0A);	// 10=10=みなし決済休業案内
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				break;
			case 2:		// 精算中止
				EcRegistDeemedSettlementError(3);					// 決済処理失敗（精算中止）
				if (ec_result == EPAY_RESULT_MIRYO) {
					miryo_timeout_after_disp();
				}
				else {
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
				}
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				break;
			case 3:		// 障害連絡票を発行して精算中止
				EcRegistDeemedSettlementError(3);					// 決済処理失敗（精算中止）
				if (Ope_isPrinterReady() &&							// レシート出力可
					sts == EC_SETT_STS_DEEMED_SETTLEMENT) {			// 決済状態＝「みなし決済」受信
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[77], 0, 0, COLOR_RED, LCD_BLINK_OFF);
					ECCTL.Ec_FailureContact_Sts = 1;				// 1:障害連絡票発行処理中
					if( OPECTL.RECI_SW == 1 ){						// 領収書ﾎﾞﾀﾝ使用（押下）済？
						OPECTL.RECI_SW = 0;							// 領収書ﾎﾞﾀﾝ未使用
					}
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0D);	// 13=障害連絡票発行案内(コード決済)
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				}
				else {
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
					ECCTL.Ec_FailureContact_Sts = 0;				// 0:障害連絡票発行不可（すでに0であるが明示的にｾｯﾄ）
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0F);	// 15=障害連絡票発行失敗(コード決済)
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
				}
				break;
			default:
				break;
			}
		}

		if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
			// 決済状態＝「みなし決済」は決済結果データも同時に通知されるため、
			// 決済結果のみ書き換える
			Ec_Settlement_Res.Result = EPAY_RESULT_NG;				// 決済結果NGとして処理させる
			// 復電用決済結果データを更新
			memcpy(&EcAlarm.Ec_Res, &Ec_Settlement_Res, sizeof(Ec_Settlement_Res));
		}
		else {
			if (ec_result == EPAY_RESULT_MIRYO) {
				// 精算日時は不明のため精算機の日時をセット
				memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(CLK_REC));
				// 復電用決済結果データを更新
				memcpy(&EcAlarm.Ec_Res, &Ec_Settlement_Res, sizeof(Ec_Settlement_Res));
			}
		}

		if (ryo_buf.zankin > 0) {
			if( sts != EC_SETT_STS_DEEMED_SETTLEMENT ){
				// みなしによる決済中止であり
				// 決済状態＝「みなし決済」以外はdeemSettleCancalﾌﾗｸﾞをON
				EcAlarm.Ec_Res.E_Flag.BIT.deemSettleCancal = 1;
			}
			// 残金がある場合
			// 精算中止データを送信するため、決済情報をPayDataにセット
			switch (EcDeemedLog.EcDeemedBrandNo) {
			case BRANDNO_CREDIT:
				EcCreUpdatePayData(&EcAlarm.Ec_Res);
				break;
			case BRANDNO_QR:
				EcQrSet_PayData(&EcAlarm.Ec_Res);
				break;
			default:
				EcElectronSet_PayData(&EcAlarm.Ec_Res);
				break;
			}
		}

		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			// 決済ﾘﾄﾗｲﾀｲﾏﾘｾｯﾄ
			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// 未了状態解除
			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// 応答がなく、未了タイムアウトした場合
			Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;					// 未了タイムアウト発生
		}

		if (ec_result != EPAY_RESULT_MIRYO) {
			// 未了確定受信時以外
			Suica_Rec.Data.BIT.CTRL = 0;
			Suica_Rec.Data.BIT.OPE_CTRL = 0;
			Suica_Rec.Data.BIT.ADJUSTOR_START = 0;					// 商品選択データ送信可否フラグクリア
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;					// 決済中判定フラグクリア
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;					// 待ち合わせフラグクリア
			Suica_Rec.suica_rcv_event.BYTE = 0;						// 受信イベントフラグクリア
		}

		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;						// ｱﾗｰﾑﾒｯｾｰｼﾞ表示用ﾌﾗｸﾞｾｯﾄ
		ac_flg.cycl_fg = 10;										// 入金中
		ac_flg.ec_deemed_fg = 0;									// みなし決済復電用フラグOFF
	}
}
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善

//[]----------------------------------------------------------------------[]
///	@brief			save_deemed_data
//[]----------------------------------------------------------------------[]
///	@param[in]		short brandno
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	: 2019/06/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	save_deemed_data(short brandno)
{
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//	ushort num = 0;

//	// みなし決済復電用データ保存
//	memset(&EcDeemedLog, 0, sizeof(EcDeemedLog));
//	EcDeemedLog.PayPrice = Product_Select_Data;						// みなし決済額
//	EcDeemedLog.syu = (uchar)(ryo_buf.syubet + 1);					// 駐車種別
//	if( PayData.teiki.update_mon ){									// 更新精算?
//		EcDeemedLog.WPlace = 9999;									// 更新時は車室番号9999とする。
//		EcDeemedLog.update_mon = PayData.teiki.update_mon;			// 更新月数
//	}else if( ryo_buf.pkiti == 0xffff ){							// ﾏﾙﾁ精算?
//		EcDeemedLog.WPlace = LOCKMULTI.LockNo;						// 接客用駐車位置番号
//// GG119202(S) 精算中止（復）印字で車室番号が印字されない
////	}else if( !OPECTL.Pr_LokNo ){									// 後日精算？
//	}else if( ryo_buf.pkiti == 0 ){									// 後日精算？
//		// 遠隔精算（後日精算）は駐車位置番号＝０のため、受信した車室情報をセットする
//// GG119202(E) 精算中止（復）印字で車室番号が印字されない
//		EcDeemedLog.WPlace = OPECTL.Op_LokNo;
//	}else{
//		num = ryo_buf.pkiti - 1;
//		EcDeemedLog.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// 接客用駐車位置番号
//	}
//	if( ryo_buf.ryo_flg < 2 ){										// 駐車券精算処理
//		EcDeemedLog.WPrice = ryo_buf.tyu_ryo;						// 駐車料金
//	}else{
//		EcDeemedLog.WPrice = ryo_buf.tei_ryo;						// 定期料金
//	}
//// MH810100(S) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
//	EcDeemedLog.CarSearchFlg = PayData.CarSearchFlg;
//	memcpy(EcDeemedLog.CarSearchData, PayData.CarSearchData, sizeof(EcDeemedLog.CarSearchData));
//	memcpy(EcDeemedLog.CarNumber, PayData.CarNumber, sizeof(EcDeemedLog.CarNumber));
//	memcpy(EcDeemedLog.CarDataID, PayData.CarDataID, sizeof(EcDeemedLog.CarDataID));
//// MH810100(E) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
//	EcDeemedLog.TInTime.Year = car_in_f.year;						// 入庫 年
//	EcDeemedLog.TInTime.Mon  = car_in_f.mon;						//      月
//	EcDeemedLog.TInTime.Day  = car_in_f.day;						//      日
//	EcDeemedLog.TInTime.Hour = car_in_f.hour;						//      時
//	EcDeemedLog.TInTime.Min  = car_in_f.min;						//      分
//	EcDeemedLog.EcDeemedBrandNo = (ushort)brandno;
//// GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
////	memcpy(&EcDeemedLog.EcDeemedDate, &CLK_REC, sizeof(date_time_rec));
//	EcDeemedLog.TOutTime.Year = car_ot_f.year;						// 出庫 年
//	EcDeemedLog.TOutTime.Mon  = car_ot_f.mon;						//      月
//	EcDeemedLog.TOutTime.Day  = car_ot_f.day;						//      日
//	EcDeemedLog.TOutTime.Hour = car_ot_f.hour;						//      時
//	EcDeemedLog.TOutTime.Min  = car_ot_f.min;						//      分
//// GG119202(E) ICクレジットみなし決済印字フォーマット変更対応
//// GG119202(S) 決済精算中止印字処理修正
////	ac_flg.ec_alarm = 3;	// 復電用フラグセット
//	ac_flg.ec_deemed_fg = 1;	// みなし決済復電用フラグセット
//// GG119202(E) 決済精算中止印字処理修正
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	EC_SETTLEMENT_RES	*pRes;
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	// みなし決済復電用データ保存
	memset(&EcDeemedLog, 0, sizeof(EcDeemedLog));
	EcDeemedLog.PayPrice = Product_Select_Data;					// みなし決済額
	EcDeemedLog.EcDeemedBrandNo = (ushort)brandno;				// ブランド番号

// MH810100(S) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
	EcDeemedLog.CarSearchFlg = PayData.CarSearchFlg;
	memcpy(EcDeemedLog.CarSearchData, PayData.CarSearchData, sizeof(EcDeemedLog.CarSearchData));
	memcpy(EcDeemedLog.CarNumber, PayData.CarNumber, sizeof(EcDeemedLog.CarNumber));
	memcpy(EcDeemedLog.CarDataID, PayData.CarDataID, sizeof(EcDeemedLog.CarDataID));
// MH810100(E) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	pRes = &EcAlarm.Ec_Res;
	memset(pRes, 0, sizeof(*pRes));
	pRes->Result = EPAY_RESULT_NG;								// 決済結果NGとして処理させる
	pRes->settlement_data = EcDeemedLog.PayPrice;				// 決済額をセット
	pRes->brand_no = EcDeemedLog.EcDeemedBrandNo;				// 決済ブランドをセット
	// 現在の日時をセット
	memcpy(&pRes->settlement_time, &CLK_REC, sizeof(CLK_REC));

	// カード番号をスペースで初期化
	memset(pRes->Card_ID, 0x20, sizeof(pRes->Card_ID));
	// 問い合わせ番号
	memset(pRes->inquiry_num, '*', sizeof(pRes->inquiry_num));
	pRes->inquiry_num[4] = '-';
	switch (brandno) {
	case BRANDNO_CREDIT:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_CREDIT);
		// 承認No.
		memset(pRes->Brand.Credit.Approval_No, '*',
				sizeof(pRes->Brand.Credit.Approval_No));
		// 端末No.
		memset(pRes->Brand.Credit.Id_No, '*',
				sizeof(pRes->Brand.Credit.Id_No));
		break;
	case BRANDNO_KOUTSUU:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
		// 端末番号
		memset(pRes->Brand.Koutsuu.SPRW_ID, '*',
				sizeof(pRes->Brand.Koutsuu.SPRW_ID));
		break;
	case BRANDNO_EDY:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// 上位端末ID
		memset(pRes->Brand.Edy.TerminalNo, '*',
				sizeof(pRes->Brand.Edy.TerminalNo));
		// Edy取引通番
		memset(pRes->Brand.Edy.DealNo, '*',
				sizeof(pRes->Brand.Edy.DealNo));
		// カード取引通番
		memset(pRes->Brand.Edy.CardDealNo, '*',
				sizeof(pRes->Brand.Edy.CardDealNo));
		break;
	case BRANDNO_WAON:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// SPRWID
		memset(pRes->Brand.Waon.SPRW_ID, '*',
				sizeof(pRes->Brand.Waon.SPRW_ID));
		break;
	case BRANDNO_NANACO:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// 端末取引通番
		memset(pRes->Brand.Nanaco.DealNo, '*',
				sizeof(pRes->Brand.Nanaco.DealNo));
		// 上位端末ID
		memset(pRes->Brand.Nanaco.TerminalNo, '*',
				sizeof(pRes->Brand.Nanaco.TerminalNo));
		break;
	case BRANDNO_QUIC_PAY:
		// カード番号
		memset(pRes->Card_ID, '*', sizeof(pRes->Card_ID));
		// 上位端末ID
		memset(pRes->Brand.QuicPay.TerminalNo, '*',
				sizeof(pRes->Brand.QuicPay.TerminalNo));
		// 承認番号
		memset(pRes->Brand.QuicPay.Approval_No, '*',
				sizeof(pRes->Brand.QuicPay.Approval_No));
		break;
	case BRANDNO_ID:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// 上位端末ID
		memset(pRes->Brand.Id.TerminalNo, '*',
				sizeof(pRes->Brand.Id.TerminalNo));
		// 承認番号
		memset(pRes->Brand.Id.Approval_No, '*',
				sizeof(pRes->Brand.Id.Approval_No));
		break;
	case BRANDNO_PITAPA:
		// カード番号
		memset(pRes->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
		// 上位端末ID
		memset(pRes->Brand.Pitapa.TerminalNo, '*',
				sizeof(pRes->Brand.Pitapa.TerminalNo));
		// 承認番号
		memset(pRes->Brand.Pitapa.Approval_No, '*',
				sizeof(pRes->Brand.Pitapa.Approval_No));
		// 伝票番号
		memset(pRes->Brand.Pitapa.Slip_No, '*',
				sizeof(pRes->Brand.Pitapa.Slip_No));
		break;
	case BRANDNO_QR:
		// 端末ID
		memset(pRes->Brand.Qr.PayTerminalNo, '*',
				sizeof(pRes->Brand.Qr.PayTerminalNo));
		// 取引番号
		pRes->inquiry_num[4] = '*';
		break;
	default:
		break;
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

	ac_flg.ec_deemed_fg = 1;									// みなし決済復電用フラグセット
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
}

//[]----------------------------------------------------------------------[]
///	@brief			ReqEcDeemedJnlPrint
//[]----------------------------------------------------------------------[]
///	@param			void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	:	2019/06/11<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ReqEcDeemedJnlPrint(void)
{
// MH810103 GG119202(S) 決済精算中止印字処理修正
//// GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
////	static T_FrmEcDeemedJnlReq	FrmDeemedJnlReq;
////	static EcDeemedFukudenLog	deemed_jnl_info;
////
////	memset(&FrmDeemedJnlReq, 0, sizeof(T_FrmEcAlmRctReq));
////	memset(&deemed_jnl_info, 0, sizeof(deemed_jnl_info));
////
////	FrmDeemedJnlReq.prn_kind = J_PRI;											// 印字先
////	memcpy(&deemed_jnl_info, &EcDeemedLog, sizeof(EcDeemedLog));				// みなし決済情報
////	FrmDeemedJnlReq.deemed_jnl_info = &deemed_jnl_info;
////	queset(PRNTCBNO, PREQ_EC_DEEMED_LOG, sizeof(FrmDeemedJnlReq), &FrmDeemedJnlReq);
//
//	T_FrmReceipt ReceiptPreqDeemedData;
//
//	memset( &ReceiptPreqDeemedData, 0, sizeof(T_FrmReceipt) );
//
//	PayData.WFlag = ON;															// 復電フラグON
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;				// みなし決済フラグON
//	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];									// 機械№
//	// EcDeemedLogの各データをReceipt_data型に入れ替える
//	PayData.teiki.update_mon = EcDeemedLog.update_mon;
//	PayData.syu = EcDeemedLog.syu;
//	PayData.WPrice = EcDeemedLog.WPrice;
//	PayData.Electron_data.Ec.pay_ryo = EcDeemedLog.PayPrice;
//	memcpy( (date_time_rec *)&PayData.TInTime, &EcDeemedLog.TInTime, sizeof(date_time_rec) );
//	PayData.Electron_data.Ec.e_pay_kind = convert_brandno( EcDeemedLog.EcDeemedBrandNo );
//	memcpy( (date_time_rec *)&PayData.TOutTime, &EcDeemedLog.TOutTime, sizeof(date_time_rec) );
//
//	// ｸﾚｼﾞｯﾄｶｰﾄﾞ決済の場合、ｸﾚｼﾞｯﾄ支払い額にも料金をセット
//	if( EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT ){
//		ryo_buf.credit.pay_ryo = EcDeemedLog.PayPrice;
//	}
//
//	// 入金中でなければ、決算精算中止(復)を印字する
//	// 入金中であれば、従来通りの精算中止(復)を印字する
//	if( ac_flg.cycl_fg == 1 ){
//		CountGet( PAYMENT_COUNT, &PayData.Oiban );									// 精算追番
//		// ryo_bufからPayDataに入れ替える
//		PayData.credit.pay_ryo = ryo_buf.credit.pay_ryo;
//
//		ReceiptPreqDeemedData.prn_kind = J_PRI;										// 印字先
//		ReceiptPreqDeemedData.prn_data = (Receipt_data *)&PayData;
//		queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqDeemedData);
//	}
//// GG119202(E) ICクレジットみなし決済印字フォーマット変更対応

// MH810105(S) MH364301 QRコード決済対応
//	// みなしフラグをセットする
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
	// みなし決済（みなし決済結果未受信）による精算中止フラグをセット
	PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal = 1;
	// 未了確定／障害発生フラグをセットする
	PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
// MH810105(E) MH364301 QRコード決済対応
	// 決済種別をセットする
	PayData.Electron_data.Ec.e_pay_kind = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
	// 料金をセットする
	if (EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT) {
		ryo_buf.credit.pay_ryo = EcDeemedLog.PayPrice;
	}
	else {
		PayData.Electron_data.Ec.pay_ryo = EcDeemedLog.PayPrice;
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//		if (EcDeemedLog.MiryoFlg != 0) {
//			// 未了中の復電は未了回数をカウントする
//			ec_alarm_syuu(PayData.Electron_data.Ec.e_pay_kind, PayData.Electron_data.Ec.pay_ryo);
//// MH810103 GG118808_GG118908(S) 再タッチ待ち中に復電で処理未了取引記録が登録されない
//			EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//			ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// アラームレシート印字要求
//			EcAlarmLog_Regist(&Ec_Settlement_Res);					// 処理未了取引ログ登録
//// MH810103 GG118808_GG118908(E) 再タッチ待ち中に復電で処理未了取引記録が登録されない
//// MH810104 GG119201(S) 再タッチ待ち中の復電でR0176が登録されない
//			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// モニタ登録
//// MH810104 GG119201(E) 再タッチ待ち中の復電でR0176が登録されない
//			EcDeemedLog.MiryoFlg = 0;
//		}
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善
	}

// MH810103 GG119202(S) みなし決済扱い時の動作
	// みなし決済扱い時の動作（復電）（50-0015）
	if (prm_get(COM_PRM, S_ECR, 15, 1, 1) == 1) {
		// 休業とする
		queset( OPETCBNO, EC_EVT_DEEMED_SETTLE_FUKUDEN, 0, 0 );
	}
// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810103 GG119202(E) 決済精算中止印字処理修正
// MH810105(S) MH364301 QRコード決済対応
	// 決済精算中止（復）ではEcElectronSet_PayData等、PayData処理関数を通らない
	// ここで復電時のID152データをセットする
	Ec_Data152Save();
// MH810105(E) MH364301 QRコード決済対応
}

//[]----------------------------------------------------------------------[]
///	@brief			ec_errcode_err_chk
//[]----------------------------------------------------------------------[]
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
/////	@param			uchar *err_wk, uchar *ec_err_code
///	@param			uchar *err_wk, uchar *ec_err_code, uchar brand_index
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			D.Inaba
///	@date			Create	:	2019/10/16<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
//static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code)
static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code, uchar brand_index)
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
{
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
	uchar ec_Disp_No = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	uchar registed_errcode = 0;				// 0 = エラー未登録　1 = エラー登録
	uchar err_code_under2 = (uchar)astoin(&ec_err_code[1], 2);	// エラーコード下2桁
// MH810105(S) MH364301 QRコード決済対応
	uchar subbrand_index = Ec_Settlement_Res.Brand.Qr.PayKind;
// MH810105(E) MH364301 QRコード決済対応

	if( OPECTL.InquiryFlg == 0 ){			// カード処理中ではない？
// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
//		if( ec_errcode_err_chk_judg( &ec_err_code[0] ) ){	// タイムアウトエラー？
		if( ec_errcode_err_chk_judg( &ec_err_code[0], EcTimeoutErrTbl, NG_ERR_MAX ) ){	// タイムアウトエラー？
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する
			// タイムアウトエラーの場合、ec_Disp_Noを 99 にする
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//			ec_Disp_No = 99;
			ECCTL.ec_Disp_No = 99;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			return;
		}
	}

// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
// MH810105(S) MH364301 QRコード決済対応
//	if ('a' <= ec_err_code[0] && ec_err_code[0] <= 'z') {
	if (('a' <= ec_err_code[0] && ec_err_code[0] <= 'z') || 
		('A' <= ec_err_code[0] && ec_err_code[0] <= 'Z')) {
// MH810105(E) MH364301 QRコード決済対応
		sprintf((char*)&err_wk[13], "-%s", EcBrandKind_prn[brand_index - EC_USED]);
// MH810105(S) MH364301 PiTaPa対応
// 支払方法がPiTaPaかつ、理由コードがセットされている場合のみ
// ジャーナル印字データに理由コードを追加する
		if(brand_index == EC_PITAPA_USED && Ec_Settlement_Res.Brand.Pitapa.Reason_Code[0] != 0x00){
			memcpy(&err_wk[16],"理由コード",10);
			memcpy(&err_wk[26],
					Ec_Settlement_Res.Brand.Pitapa.Reason_Code,
					sizeof(Ec_Settlement_Res.Brand.Pitapa.Reason_Code));
		}
// MH810105(E) MH364301 PiTaPa対応
	}
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
	switch( ec_err_code[0] ){
		case 'C':
			if(EcErrCodeTbl[EC_ERR_C][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_C][err_code_under2];
			}
			break;
		case 'D':
			if(EcErrCodeTbl[EC_ERR_D][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_D][err_code_under2];
			}
			break;
		case 'E':
			if(EcErrCodeTbl[EC_ERR_E][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_E][err_code_under2];
			}
			break;
		case 'G':
			if(EcErrCodeTbl[EC_ERR_G][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_G, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_G][err_code_under2];
// MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更 #5401 
				if (brand_index == EC_WAON_USED &&
					err_code_under2 == 6 ) {
					// 表示するメッセージを変更する（G06）
					ec_Disp_No = 198;
				}
// MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更 #5401 
			}
			break;
		case 'K':
			if(EcErrCodeTbl[EC_ERR_K][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_K][err_code_under2];
			}
			break;
		case 'P':
// MH810105(S) MH364301 QRコード決済対応
			if (brand_index == EC_QR_USED) {
				// QRコード用エラー
				switch (err_code_under2) {
				// 重大詳細エラーコード
				case 0:
				case 1:
				case 2:
					if (EcErrCodeTbl[EC_ERR_QR_P][err_code_under2]) {
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_P][err_code_under2];
					}
					break;
				// 通常詳細エラーコード
				default:
					if (EcErrCodeTbl[EC_ERR_QR_P][err_code_under2]) {
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_P][err_code_under2];
					}
					break;
				}
				break;
			}
// MH810105(E) MH364301 QRコード決済対応
			if(EcErrCodeTbl[EC_ERR_P][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_P][err_code_under2];
			}
			break;
		case 'S':
			if(EcErrCodeTbl[EC_ERR_S][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_S][err_code_under2];
			}
			break;
// MH810105(S) MH364301 QRコード決済対応
		case 'Q':
			// QRコード用エラー
			switch (err_code_under2) {
			// 重大詳細エラーコード
			case 0:
			case 3:
				if (EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2]) {
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
					registed_errcode = 1;
					ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2];
				}
				break;
			// 通常詳細エラーコード
			default:
				if (EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2]) {
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
					registed_errcode = 1;
					ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2];

					if (err_code_under2 == 35 && subbrand_index == ALIPAY) {	// Q35の時
						// 取引に失敗しました
						// 他の支払方法をご利用ください 
						ec_Disp_No = 10;		// ALIPAYのみメッセージが変更
					}
				}
				break;
			}
			break;
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
		case 'a':
			if(EcErrCodeTbl[EC_ERR_a][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_a][err_code_under2];
			}
			break;
		case 'c':
			if(EcErrCodeTbl[EC_ERR_c][err_code_under2]){
// MH810105 GG119202(S) 決済エラーコードテーブル見直し
//				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
// MH810105(S) MH364301 PiTaPa対応
//				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				switch (err_code_under2) {
				// 重大詳細エラーコード
				case 11:
				case 12:
				case 13:
				case 14:
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
					break;
				default:
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
					break;
				}
// MH810105(E) MH364301 PiTaPa対応
// MH810105 GG119202(E) 決済エラーコードテーブル見直し
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_c][err_code_under2];
			}
			break;
		case 'd':
			if(EcErrCodeTbl[EC_ERR_d][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_d][err_code_under2];
// MH810103 MHUT40XX(S) Edy・WAON対応
				if (brand_index == EC_WAON_USED &&
					err_code_under2 == 9) {
					// 表示するメッセージを変更する（d09）
					ec_Disp_No = 97;
				}
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810103 MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
				else if (brand_index == EC_WAON_USED &&
					err_code_under2 == 1 ) {
					// 表示するメッセージを変更する（d01）
					ec_Disp_No = 198;
				}
// MH810103 MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
// MH810105 GG119202(S) iD決済時の案内表示対応
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 5) {
					// 表示するメッセージを変更する（d05）
					ec_Disp_No = 92;
				}
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 3) {
					// 表示するメッセージを変更する（d03）
					ec_Disp_No = 91;
				}
// MH810105 GG119202(E) iD決済時の案内表示対応
			}
			break;
		case 'e':
			if(EcErrCodeTbl[EC_ERR_e][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_e][err_code_under2];
// MH810103 MHUT40XX(S) Edy・WAON対応
				if (brand_index == EC_WAON_USED &&
					(err_code_under2 == 3 || err_code_under2 == 4)) {
					// 表示するメッセージを変更する（e03, e04）
					ec_Disp_No = 97;
				}
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810105 GG119202(S) iD決済時の案内表示対応
				if (brand_index == EC_ID_USED &&
					(err_code_under2 == 2 || err_code_under2 == 3)) {
					// 表示するメッセージを変更する（e02, e03）
					ec_Disp_No = 93;
				}
// MH810105 GG119202(E) iD決済時の案内表示対応
			}
			break;
		case 'g':
			if(EcErrCodeTbl[EC_ERR_g][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_g][err_code_under2];
			}
			break;
		case 'h':
			if(EcErrCodeTbl[EC_ERR_h][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_h][err_code_under2];
// MH810105 GG119202(S) iD決済時の案内表示対応
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 3) {
					// 表示するメッセージを変更する（h03）未了確定時のエラー
					ec_Disp_No = 90;
				}
// MH810105 GG119202(E) iD決済時の案内表示対応
			}
			break;
		case 'i':
			if(EcErrCodeTbl[EC_ERR_i][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_i][err_code_under2];
			}
			break;
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加
		default: // 特定のエラー文字では無かった
			break;
	}

	//	まだ一致したエラーがない(エラー登録できていない)
	if( !registed_errcode ){
		switch( ec_err_code[0] ){
			case '1':	// 1XX
				switch ( err_code_under2 ){
					case 0:		// 100
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_100, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 101
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_101, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					default:
						break;
				}
				break;
			case '2':	// 2XX
				switch ( err_code_under2 ){
					case 0:		// 200
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_200, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 201
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_201, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					default:
						break;
				}
				break;
			case '3':	// 3XX
				switch ( err_code_under2 ){
					case 0:		// 300
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_300, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 301
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_301, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 4:		// 304
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 5:		// 305
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_305, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 6:		// 306
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_306, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
// MH810105(S) MH364301 QRコード決済対応
//						ec_Disp_No = 5;
						if (brand_index == EC_QR_USED) {
							ec_Disp_No = 17;	// コードの読み取りに失敗しました
						} else {
							ec_Disp_No = 5;		// カードの読み取りに失敗しました。
						}
// MH810105(E) MH364301 QRコード決済対応
						break;
					case 7:		// 307
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_307, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 5;
						break;
					case 8:		// 308
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_308, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
// MH810105(S) MH364301 QRコード決済対応
//						ec_Disp_No = 5;
						if (brand_index == EC_QR_USED) {
							ec_Disp_No = 17;		// コードの読み取りに失敗しました
						} else {
							ec_Disp_No = 5;			// カードの読み取りに失敗しました。
						}
// MH810105(E) MH364301 QRコード決済対応
						break;
					case 9:		// 309
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_309, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 3;
						break;
					case 11:	// 311
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_311, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 5;
						break;
					default:
						break;
				}
				break;
			default: // 特定のエラー数字では無かった
				break;
		}
	}

	if( !registed_errcode ){
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
		ECCTL.ec_Disp_No = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		// 未登録詳細エラー ※ここまでエラー登録できない、未登録エラーとして登録
		err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_UNREG, 2, 1, 0, (void*)err_wk);
	}
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
	else {
		ECCTL.ec_Disp_No = ec_Disp_No;
	}
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
}

// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
////[]----------------------------------------------------------------------[]
/////	@brief			タイムアウトエラーチェック
////[]----------------------------------------------------------------------[]
/////	@param[in]		wk_err_code : 詳細エラーコード
/////	@return			ret			: 1=タイムアウトエラー, 0=タイムアウトエラーではない
/////	@author			D.Inaba
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/12/04<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//static uchar ec_errcode_err_chk_judg( uchar *wk_err_code )
//{
//	uchar	ret = 0;
//	uchar	i;
//
//	// EcTimeoutErrTblに受信した詳細エラーがあるかどうか判定
//	for( i = 0; i < NG_ERR_MAX; i++){
//		if( wk_err_code[0] == EcTimeoutErrTbl[i][0] &&
//			wk_err_code[1] == EcTimeoutErrTbl[i][1] &&
//			wk_err_code[2] == EcTimeoutErrTbl[i][2] ){
//			ret = 1;
//			break;
//		}
//	}
//
//	return (ret);
//}
//[]----------------------------------------------------------------------[]
///	@brief			詳細エラーコードチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		wk_err_code : 詳細エラーコード
///	@param[in]		ErrTbl		: 比較したいエラーコードが格納されているテーブル
///	@return			ret			: 0：テーブルにない、1：テーブルにある
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/16<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static uchar ec_errcode_err_chk_judg( uchar *wk_err_code, const uchar ErrTbl[][3], uchar max )
{
	uchar	ret = 0;
	uchar	i;

	for( i = 0; i < max; i++){
		if( !memcmp( &wk_err_code[0], &ErrTbl[i][0], sizeof(uchar)*3 ) ){
			ret = 1;
			break;
		}
	}

	return (ret);
}
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//[]----------------------------------------------------------------------[]
///	@brief			ブランド選択結果表示処理
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/09/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void ec_BrandResultDisp(ushort brand_no, ushort reason_code)
{
	ushort	num = 0;										// "    このカードは使えません    "

	// エラーメッセージの表示
	switch(reason_code) {
	case EC_BRAND_REASON_0:									// キャンセルボタンが押された。
	case EC_BRAND_REASON_1:									// タイムアウトした。
	case EC_BRAND_REASON_99:								// ブランド選択データの処理ができない。
		return;												// 表示＆アナウンスなし

	case EC_BRAND_REASON_201:								// カードの問い合わせができない。
	case EC_BRAND_REASON_202:								// カードの問い合わせ結果を受信できない。
		num = 1;											// "    只今、お取り扱いが出来ません    "
		break;
	case EC_BRAND_REASON_203:								// 誤りデータが含まれるカードを読み取りした。
		num = 8;											// "再度カードをスライドして下さい"
		break;
	case EC_BRAND_REASON_204:								// データが破損しているカードを読み取りした。
	case EC_BRAND_REASON_205:								// 意図しないカードを読み取りした。
		num = 5;											// "カードの読み取りに失敗しました"
		break;
	case EC_BRAND_REASON_206:								// 有効期限切れのカードを読み取りした。
		num = 4;											// "    このカードは期限切れです    "
		break;
	case EC_BRAND_REASON_207:								// ネガカードを読み取りした。
		if (brand_no == BRANDNO_HOUJIN) {
			num = 7;										// "このカードはお取扱いできません"
		} else {
			num = 2;										// "    このカードは無効です    "
		}
		break;
	case EC_BRAND_REASON_208:								// 有効なカードであるが使用できない。
		if (brand_no == BRANDNO_TCARD) {
//			num = FX_WARN_MSG(51);							// " Ｔカードはお取扱いできません "
		} else {
			num = 1;										// "    只今、お取り扱いが出来ません    "
		}
		break;
	case EC_BRAND_REASON_209:								// 有効なカードであるが支払い限度額を超えている。
		// 現行機は精算限度額を超えている場合は、「法人カードでの\r\n精算限度額を超えています」と表示するが、
		// 決済リーダでは精算限度額を超えている場合はブランド選択に含まれないため、本エラーが発生することは正常系において存在しない。
		// 従来は磁気リーダからカード抜き取りタイミングでメッセージを消去しているが、決済リーダにはそのタイミングがないこと。
		// 複数行表示のワーニング表示機能に対応していないことから、このカードは使えません に差し替える(尾之内様相談済み　2020/09/01)
		break;
	case EC_BRAND_REASON_210:								// 有効なカードであるが残高が不足している。
		num = 6;											// 残高不足です
		break;
	case EC_BRAND_REASON_211:								// 受付済みのカードを読み取りした。
		if (brand_no == BRANDNO_TCARD) {
//			num = FX_WARN_MSG(53);							// "    Ｔカードは受付済みです    "
		}
		break;
	case EC_BRAND_REASON_212:								// 読み取り限度数を超えてカードを読み取りした。
		num = 9;											// "    限度枚数を超えています    "
		break;
//	case EC_BRAND_REASON_299:								// その他のエラー
	default:
		break;
	}
	ec_WarningMessage(num);
}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

//[]----------------------------------------------------------------------[]
///	@brief			ec_WarningMessage
//[]----------------------------------------------------------------------[]
///	@param			uchar num
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/10/17<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//static void ec_WarningMessage( uchar num )
void ec_WarningMessage( ushort num )
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
{

	short	avm_no = 0;
	const uchar	*massage = NULL;
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
	ulong	add_info = 0;
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)

	switch( num ){
		case 1:
			massage = ERR_CHR[44];			// "    只今、お取り扱いが出来ません    "
			avm_no = AVM_CARD_ERR5;			// ｱﾅｳﾝｽ「只今、お取り扱いが出来ません」
			break;
		case 2:
			massage = ERR_CHR[3];			// "    このカードは無効です    "
			avm_no = AVM_CARD_ERR3;			// ｱﾅｳﾝｽ「このカードは無効です」
			break;
		case 0:
		case 3:
			massage = ERR_CHR[1];			// "    このカードは使えません    "
			avm_no = AVM_CARD_ERR1;			// ｱﾅｳﾝｽ「このカードは使えません」
			break;
		case 4:
			massage = ERR_CHR[43];			// "    このカードは期限切れです    "
			avm_no = AVM_KIGEN_END;			// ｱﾅｳﾝｽ「このカードは期限切れです」
			break;
		case 5:
			massage = ERR_CHR[54];			// "    カードの読み取りに失敗しました    "
			avm_no = AVM_CARD_ERR6;			// ｱﾅｳﾝｽ「カードの読み取りに失敗しました」
			break;
		case 6:
			massage = ERR_CHR[1];			// "    このカードは使えません    "
											// "  残高不足（残高０００００円）"
// MH810103 GG119202(S) 案内放送の電子マネー対応
//			avm_no = AVM_CARD_ERR7;			// ｱﾅｳﾝｽ「カードの残高がありません」
			avm_no = AVM_CARD_ERR7;			// ｱﾅｳﾝｽ「残高不足です」
// MH810103 GG119202(E) 案内放送の電子マネー対応
			break;
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応参考）
		case 7:
			massage = ERR_CHR[41];			// "このカードはお取扱いできません"
			avm_no = AVM_CARD_ERR4;			// ｱﾅｳﾝｽ 「このカードはお取り扱いできません」
			break;
		case 8:
			massage = ERR_CHR[55];			// "再度カードをスライドして下さい"
			avm_no = AVM_CARD_RETRY;		// ｱﾅｳﾝｽ 「再度矢印の方向に入れて下さい」
			break;
		case 9:
			massage = ERR_CHR[8];			// "    限度枚数を超えています    "
			avm_no = AVM_CARD_ERR1;			// ｱﾅｳﾝｽ 「このカードは使えません」
			break;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応参考）
// MH810105(S) MH364301 QRコード決済対応
		case 10:
			massage = ERR_CHR[58];			// "他の支払方法でお支払いください"
			break;
		case 11:
			massage = ERR_CHR[70];			// "このコードは期限切れです"
			break;
		case 12:
			massage = ERR_CHR[71];			// "このコードは使えません"
			break;
		case 13:
			massage = ERR_CHR[71];			// "このコードは使えません"
			break;
		case 14:
			massage = ERR_CHR[72];			// "このコードは残高不足です"
			break;
		case 15:
			massage = OPE_CHR_CRE[2];		// "精算限度額を超えています"
			break;
		case 16:
			massage = ERR_CHR[59];			// "もう一度やり直してください"
			break;
		case 17:
			massage = ERR_CHR[73];			// "コードの読み取りに失敗しました"
			break;
// MH810105(E) MH364301 QRコード決済対応
// MH810105 GG119202(S) iDの読み取り待ちタイムアウトで案内表示
		case 89:							// 読み取り待ちタイムアウト（iD）
			massage = ERR_CHR[59];			// "  もう一度やり直してください  "
			// 案内放送なし
			break;
// MH810105 GG119202(E) iDの読み取り待ちタイムアウトで案内表示
// MH810105 GG119202(S) iD決済時の案内表示対応
		case 91:							// iD:d05
			massage = ERR_CHR[59];			// "  もう一度やり直してください  "
			avm_no = AVM_CARD_ERR1;			// ｱﾅｳﾝｽ 「このカードは使えません」
			break;
		case 92:							// iD:d03
			massage = ERR_CHR[58];			// " 他の支払方法をご利用ください "
			avm_no = AVM_CARD_ERR6;			// ｱﾅｳﾝｽ「カードの読み取りに失敗しました」
			break;
		case 93:							// iD:e02,e03
			massage = ERR_CHR[60];			// " カード発行元にご連絡ください "
			avm_no = AVM_CARD_ERR1;			// ｱﾅｳﾝｽ 「このカードは使えません」
			break;
		case 94:							// iD:e05
			massage = ERR_CHR[58];			// " 他の支払方法をご利用ください "
			avm_no = AVM_CARD_ERR6;			// ｱﾅｳﾝｽ「カードの読み取りに失敗しました」
			break;
// MH810105 GG119202(E) iD決済時の案内表示対応
// MH810105 MH321800(S) WAON決済時の残高不足メッセージ内容を変更
		case 95:
			massage = ERR_CHR[57];			// " 残高不足  他の支払方法へ変更 "
			avm_no = AVM_CARD_ERR7;			// ｱﾅｳﾝｽ「残高不足です」
			break;
// MH810105 MH321800(E) WAON決済時の残高不足メッセージ内容を変更
// MH810103 MHUT40XX(S) Edy・WAON対応
		case 97:
			massage = ERR_CHR[57];			// "このカードはお取扱いできません"
			avm_no = AVM_CARD_ERR1;			// ｱﾅｳﾝｽ「このカードは使えません」
			break;
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810103 MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
		case 198:
			massage = ERR_CHR[1];			// "    このカードは使えません    "
											// "  残高不足（残高０００００円）"
			avm_no = AVM_CARD_ERR7;			// ｱﾅｳﾝｽ「残高不足です」
			break;
// MH810103 MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
		case 99:
		default:
			// 表示も音声案内もさせない
			break;
	}

	if( massage != NULL ){
// MH810103 MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
//		if( num == 6 ){
		if( num == 6 || num == 198){
// MH810103 MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
			// 残高不足表示
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//			dsp_fusoku = 1;
//			op_SuicaFusoku_Msg((ulong)Ec_Settlement_Res.settlement_data_before, suica_dsp_buff, 1);
//			lcd_wmsg_dsp_elec(1, suica_dsp_buff, massage, 1, 1, COLOR_RED, LCD_BLINK_OFF);
			add_info = (ulong)Ec_Settlement_Res.settlement_data_before;
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
		} else {
			// 6sec表示
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//			Ope2_WarningDisp( 6*2, massage );
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
		}
// MH810103 MH810103(s) 電子マネー対応 エラーポップアップ変更
//// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//		lcdbm_notice_dsp2( POP_INQUIRY_NG, num, add_info );
//// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
		if( Ec_Settlement_Res.brand_no == BRANDNO_CREDIT){
			lcdbm_notice_dsp2( POP_INQUIRY_NG, num, add_info );
		}else{
			lcdbm_notice_dsp2( POP_EMONEY_INQUIRY_NG, num, add_info );
		}
// MH810103 MH810103(e) 電子マネー対応 エラーポップアップ変更
// MH810105(S) MH364301 QRコード決済対応
//		if( avm_no != 0 ){
//			ope_anm(avm_no);
//		}
		if (RecvBrandResTbl.no == BRANDNO_QR) {
			// QR決済の場合はブザー音（NG）を鳴らす
			BUZPIPI();
		}
		else{
			if( avm_no != 0 ){
				ope_anm(avm_no);
			}
		}
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		ec_MessagePtnNum = 98;
		ECCTL.ec_MessagePtnNum = 98;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランドエラー登録
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_ctrl:	ブランド情報
///	@param[in]		err_type:	1:登録 0：解除
///	@author			D.Inaba
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void ecRegistBrandError( EC_BRAND_CTRL *brand_ctrl, uchar err_type )
{
	char	err_no = 0;
	int		i;
	long	brand_no;

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	if (brand_ctrl->no >= BRANDNO_HOUSE_S &&
		brand_ctrl->no <= BRANDNO_HOUSE_E) {
		err_no = ERR_EC_HOUSE_ABNORAML;
	}
	else {
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	for(i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++){
		if ( brand_ctrl->no == EcBrandEnabledParamTbl[i].brand_no ) {
			err_no = EcBrandEnabledParamTbl[i].brand_err_no;
		}
	}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	// 一致するエラーがあった = エラー登録(解除)
	if( err_no ){
		memset(err_wk, 0, sizeof(err_wk));
		memcpy(&err_wk[0], "\?\?-00", 5);
		brand_no = convert_brandno(brand_ctrl->no);
		memcpy(&err_wk[0], EcBrandKind_prn[brand_no - EC_USED], 2);

		sprintf((char*)&err_wk[3], "%02X", brand_ctrl->status);

		err_chk2((char)jvma_setup.mdl, err_no, (char)err_type, (char)1, (char)0, (void*)err_wk);	// エラー登録
	}
}

//[]-----------------------------------------------------------------------[]
//| LCD表示とエラー時のアナウンスを行う										|
//[]-----------------------------------------------------------------------[]
//| MODULE NAME  : ec_MessageAnaOnOff 	 									|
//| PARAMETER    : OnOff(i) : 1:表示する 0:表示を戻す						|
//|				   num(i)   : cre 内部表示ﾊﾟﾀｰﾝ番号							|
//| RETURN VALUE : void														|
//| COMMENT		 : creMessageAnaOnOffと同処理								|
//[]-----------------------------------------------------------------------[]
//| Date         : 2019-11-08												|
//| Update       :															|
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.----[]
void ec_MessageAnaOnOff( short OnOff, short num )
{
	
	// 前回の後処理(表示を元に戻す)
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	switch( ec_MessagePtnNum ){
	switch( ECCTL.ec_MessagePtnNum ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	case 1:
	case 2:
		blink_end();
		Vram_BkupRestr( 0, 6, 1 );
		Vram_BkupRestr( 0, 7, 1 );
		break;
	case 3:
		blink_end();
		if( LCDNO == 3 ){		// 初回料金表示のみ Line=6 restore
			Vram_BkupRestr( 0, 6, 1 );
		}
		Vram_BkupRestr( 0, 7, 1 );
		break;
	case 4:
		blink_end();
		Vram_BkupRestr( 0, 7, 1 );
		break;
// MH321800(S) 処理中の表示が消えない不具合修正
	case 98:
		// LAG500_ERROR_DISP_DELAYﾀｲﾏｰより先にec_MessageAnaOnOffが呼ばれた場合
		// ここで表示を元に戻す
		Ope2_WarningDispEnd();
		break;
// MH321800(E) 処理中の表示が消えない不具合修正
	default:
		break;
	}

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	ec_MessagePtnNum = 0;
	ECCTL.ec_MessagePtnNum = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

	// 今回の処理(新たに表示させる)
	if( OnOff == 1 ){
		// 画面表示
		switch( num ){
		case 1:		// クレジットカード処理中
			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[158] );	// 反転 "クレジットカード処理中です"
			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// 点滅 "しばらくお待ち下さい"
			break;
		case 2:		// カード処理中
			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
// MH810105 GG119202(S) iD決済時の案内表示対応
// MH810105(S) MH364301 QRコード決済対応
//			if (RecvBrandResTbl.no == BRANDNO_ID) {
			if (RecvBrandResTbl.no == BRANDNO_ID ||
				RecvBrandResTbl.no == BRANDNO_QR) {
// MH810105(E) MH364301 QRコード決済対応
				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// 空行
				blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR_CRE[14] );	// 点滅 "決済リーダの画面に従って下さい"
			}
			else {
// MH810105 GG119202(E) iD決済時の案内表示対応
			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[159] );	// 反転 "カード処理中です"
			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// 点滅 "しばらくお待ち下さい"
// MH810105 GG119202(S) iD決済時の案内表示対応
			}
// MH810105 GG119202(E) iD決済時の案内表示対応
			break;
		case 3:		// カード抜き取り待ち案内(6行目、7行目)
			if( LCDNO == 3 ){													// 初回料金表示 Line=6 Backup して、空白で上書き
				Vram_BkupRestr( 1, 6, 0 );
				grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// "　　　　　　　　　　"
			}
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			blink_reg( 7, 0, 30, 0, COLOR_RED, OPE_CHR[160] );					// 点滅"　　カードを抜いてください　　"
			break;
		case 4:		// 決済結果NG受信時のｶｰﾄﾞ抜き取り待ち案内(7行目のみを変更する)
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			blink_reg( 7, 0, 30, 0, COLOR_RED, OPE_CHR[160] );					// 点滅"　　カードを抜いてください　　"
			break;
		default:
			break;
		}

		// 音声案内
		switch( num ){
		case 1:
		case 2:
			// クレジットカード(カード)処理中は音声ｱﾅｳﾝｽをしない
			break;
		case 3:
		case 4:
// MH810103 GG119202(S) 案内放送の電子マネー対応
//			ope_anm( AVM_ICCARD_ERR1 );			// カードをお取りください
			ope_anm( AVM_ICCARD_ERR1 );			// カードを抜いてください
// MH810103 GG119202(E) 案内放送の電子マネー対応
			break;
		default:
			break;
		}

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		ec_MessagePtnNum = num;					// 表示中の表示パターンを保持
		ECCTL.ec_MessagePtnNum = num;					// 表示中の表示パターンを保持
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	}
	// ec_MessageAnaOnOff( 0, 0 )が呼ばれた際
	// 上記、前回の後処理(表示を元に戻す)では表示を戻しきれない場合がある為、
	// ここで再度8行目の表示をする
	else {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		if (! isEC_USE()) {
			return;
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 インボイス対応
//		if( num == 0 ){
		if( num == 0 && OPECTL.f_RctErrDisp != 1 ){
// MH810105(E) MH364301 インボイス対応
			if( LCDNO == 4 || LCDNO == 5 ){
				Lcd_Receipt_disp();
			}else{
				dsp_intime( 7, OPECTL.Pr_LokNo );			// 入庫時刻表示
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			精算開始時の決済リーダ処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/25<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_EcPayStart(void)
{
	if (!isEC_USE()) {
		return;
	}

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	if (Suica_Rec.Data.BIT.BRAND_SEL != 0) {
		return;		// ブランド選択中は待つ（前回分のブランド選択処理、後処理が未完了）
	}
	EcBrandClear();
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	if (!Suica_Rec.Data.BIT.CTRL) {								// 受付不可状態
		if (!Suica_Rec.Data.BIT.OPE_CTRL) {
			EcSendCtrlEnableData();
		}
	}
	else {														// 受付可状態
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//// GG119202(S) クレジットカード精算限度額設定対応
////		if (ENABLE_MULTISETTLE() == 0) {						// 複数決済無効
//		if( ENABLE_MULTISETTLE() == 0 ){						// 決済可能ブランドなし
//			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// 制御ﾃﾞｰﾀ（受付禁止）を送信する
//		}
//		else if( ENABLE_MULTISETTLE() == 1 ){					// 単一決済有効
//// GG119202(E) クレジットカード精算限度額設定対応
//			Ec_Pri_Data_Snd( S_SELECT_DATA, 0 );				// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
//		}
//		else {													// 複数決済有効
//// GG119202(S) マルチブランド仕様変更対応
//			// マルチブランド時は必ずブランド選択データを送信して
//			// 支払い選択画面を表示させる
//// GG119202(E) マルチブランド仕様変更対応
//			Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// ブランド選択データを先に送信する
//		}
		if (!Suica_Rec.Data.BIT.OPE_CTRL) {
			return;		// 受付禁止処理中なので、状態通知を待つ
		}
		switch( check_enable_multisettle() ){
		case	0:
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// 制御ﾃﾞｰﾀ（受付禁止）を送信する
			break;
		case	1:		// シングル
			if( ENABLE_MULTISETTLE() == 0 ){					// 今、決済可能ブランドはない
				Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );				// 制御ﾃﾞｰﾀ（受付禁止）を送信する
			}
			else {
				ec_set_brand_selected();
				Ec_Pri_Data_Snd( S_SELECT_DATA, 0 );			// 駐車料金を選択商品ﾃﾞｰﾀとして送信する
			}
			break;
		default:		// マルチ
			Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// ブランド選択データを先に送信する
			break;
		}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			起動完了待ちタイムアウト処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_wakeup_timeout(void)
{
	// E3210発生
	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 1, 0, 0 );
}

// MH810105(S) MH364301 QRコード決済対応
////[]----------------------------------------------------------------------[]
/////	@brief			未了タイムアウト処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		*pRes	：決済結果データ
/////	@return			ret		：none
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/11/13<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes)
//{
//	char	err = 0;
//	uchar	brand_index;
//// MH810103 GG119202(S) 処理未了取引記録のカード番号はマスクして印字する
//	ushort	i, max;
//// MH810103 GG119202(E) 処理未了取引記録のカード番号はマスクして印字する
//// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
//	Ope2_ErrChrCyclicDispStop();							// サイクリック表示停止
//// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
//
//	// 未了残高照会タイムアウト時の動作を以下の3ケースに分岐する
//	// 非WAON:とりけしボタンで精算終了
//	// WAON  :WAON割引として残額を全て割引いて出庫させる
//	//       :休業
//	brand_index = convert_brandno(pRes->brand_no);
//	if (brand_index == EC_WAON_USED) {
//// MH810103 GG118809_GG118909(S) WAONの未了残高照会で未引き去りを確認した場合はとりけしボタン押下待ちとする
//// GG118808_GG118908(S) 現金投入と同時にカードタッチ→未了発生でロックする
////		if (ECCTL.ec_Disp_No == 96) {
//		if (ECCTL.ec_Disp_No == 96 ||
//			ryo_buf.zankin == 0) {
//// MH810103 GG118808_GG118908(E) 現金投入と同時にカードタッチ→未了発生でロックする
//			// 未引き去り確認
//			miryo_timeout_after_disp();						// 未了タイムアウト後の表示を行う
//			err = 1;
//			// この後、とりけしボタン押下待ちとする
//// MH810103(s) 電子マネー対応 未了通知
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
//// MH810103(e) 電子マネー対応 未了通知
//		}
//		else
//// MH810103 GG118809_GG118909(E) WAONの未了残高照会で未引き去りを確認した場合はとりけしボタン押下待ちとする
//	// WAONの時
//		if (prm_get(COM_PRM, S_ECR, 11, 1, 6) == 2) {
//		// 未了タイムアウト時は休業とする
//			miryo_timeout_after_disp();						// 未了タイムアウト後の表示を行う
//			err = 2;										// E3267を通知
//			// この後、とりけしボタン押下もしくは自動キャンセルで精算機は休業とする
//// MH810103(s) 電子マネー対応 未了通知
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
//// MH810103(e) 電子マネー対応 未了通知
//		}
//		else {
//		// 出庫させる
//			al_emony_sousai();								// 差額を相殺（必ず精算完了）
//			err = (char)prm_get(COM_PRM, S_ECR, 11, 1, 5);
//		}
//	}
//	else {
//	// WAON以外
//		miryo_timeout_after_disp();							// 未了タイムアウト後の表示を行う
//		err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
//// MH810105(S) 2021/12/03 iD対応
////// MH810103(s) 電子マネー対応 未了通知
////		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
////// MH810103(e) 電子マネー対応 未了通知
//		if( brand_index != EC_ID_USED){
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
//		}else{
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0c);	// 12=未了(決済タイムアウト)(iD)
//		}
//// MH810105(E) 2021/12/03 iD対応
//	}
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
//	if (ECCTL.ec_Disp_No == 96 ||							// 未引き去り確認（支払失敗）
//		brand_index == EC_ID_USED ||						// iD決済
//		brand_index == EC_QUIC_PAY_USED) {					// QUICPay決済
//		// E3266を登録しない
//		err = 0;
//	}
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//
//	// エラー登録
//	if (err != 0) {
//		memset(err_wk, 0, sizeof(err_wk));
//		memset(&err_wk[0], ' ', 31);
//
//		// カード種別,区切り文字
//		memcpy(&err_wk[0], "\?\?:", 3);
//		memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);
//		
//		// カード番号
//		if (brand_index != EC_UNKNOWN_USED && brand_index != EC_ZERO_USED) {
//			
//			if (brand_index == EC_SAPICA_USED) {
//			// SAPICA
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_SAPICA);	// カード番号
//			}
//			else if (brand_index == EC_KOUTSUU_USED) {
//			// 交通系ICカード
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_KOUTSUU);	// カード番号
//			}
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//			else if (brand_index == EC_QUIC_PAY_USED) {
//			// QUICPay
//				memcpy(&err_wk[3], &pRes->Card_ID, sizeof(pRes->Card_ID));	// カード番号
//			}
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
//			else {
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_SUICA);		// カード番号
//			}
//// MH810103 GG119202(S) 処理未了取引記録のカード番号はマスクして印字する
////			if (brand_index == EC_WAON_USED) {
////			// WAON
////				memset(&err_wk[3], '*', 4);									// 上位4桁マスキング
////			}
////			else if (brand_index == EC_NANACO_USED) {
////			// NANACO
////				memset(&err_wk[15], '*', 4);								// 下位4桁マスキング
////			}
//			switch (brand_index) {
//			case EC_KOUTSUU_USED:
//				max = ECARDID_SIZE_KOUTSUU - 4;
//				for (i = 2; i < max; i++) {
//					err_wk[3+i] = '*';										// 上位2桁、下位4桁以外をマスク
//				}
//				break;
//// MH810103 MHUT40XX(S) Edy・WAON対応
////			case EC_WAON_USED:
////				memset(&err_wk[3], '*', 4);									// 上位4桁マスキング
//// MH810104(S) nanaco・iD・QUICPay対応2
////// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
////			case EC_NANACO_USED:
////// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
//// MH810104(E) nanaco・iD・QUICPay対応2
//			case EC_EDY_USED:
//				memset(&err_wk[3], '*', 12);								// 下位4桁以外をマスク
//// MH810103 MHUT40XX(E) Edy・WAON対応
//				break;
//// MH810104(S) nanaco・iD・QUICPay対応2
////// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//////			case EC_NANACO_USED:
//////				memset(&err_wk[15], '*', 4);								// 下位4桁マスキング
//////				break;
////			case EC_ID_USED:
////				memset(&err_wk[16], '*', 3);								// 下位3桁をマスク
////				memset(&pRes->Card_ID[13], '*', 3);							// 下位3桁をマスク
////				break;
////			case EC_QUIC_PAY_USED:
////				memset(&err_wk[19], '*', 4);								// 下位4桁をマスク
////				memset(&pRes->Card_ID[16], '*', 4);							// 下位4桁をマスク
////				break;
////// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////			case EC_ID_USED:
////				memset(&err_wk[16], '*', 3);								// 下位3桁をマスク
////				break;
////			case EC_QUIC_PAY_USED:
////				memset(&err_wk[19], '*', 4);								// 下位4桁をマスク
////				break;
////// MH810104(E) nanaco・iD・QUICPay対応2
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//			default:
//// MH810103 MHUT40XX(S) Edy・WAON対応
//			case EC_WAON_USED:
//				// WAONはマスクされて通知される
//// MH810103 MHUT40XX(E) Edy・WAON対応
//// MH810104 GG119202(S) nanaco・iD・QUICPay対応
//			case EC_NANACO_USED:
//				// nanacoはマスクされて通知される
//// MH810104 GG119202(E) nanaco・iD・QUICPay対応
//				break;
//			}
//// MH810103 GG119202(E) 処理未了取引記録のカード番号はマスクして印字する
//		}
//		
//		// 取引額
//		memset(suica_work_buf, 0, sizeof(suica_work_buf));
//		if (pRes->settlement_data >= 1000) {
//		// 4桁以上
//			sprintf((char*)suica_work_buf, "\\%ld,%03ld", pRes->settlement_data/1000, pRes->settlement_data%1000);
//		}
//		else {
//		// 3桁以下
//			sprintf((char*)suica_work_buf, "\\%ld", pRes->settlement_data);
//		}
//		sprintf((char*)&err_wk[23], "%8s", suica_work_buf);
//
//		// エラーログ登録
//		if (err == 1) {
//		// E3266を通知
//			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE, 2, 1, 0, (void*)err_wk);
//		} else {
//		// E3267を通知
//			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON, 1, 1, 0, (void*)err_wk);
//		}
//	}
//}
//[]----------------------------------------------------------------------[]
///	@brief			未了残高照会タイムアウト処理
//[]----------------------------------------------------------------------[]
///	@param[in]		pRes	：決済結果データ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/07
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes)
{
	uchar	brand_index;

	// 未了残高照会タイムアウト時の動作
	// WAON：精算完了、休業
	// WAON以外のプリペイド型：精算完了、休業、精算中止
	// ポストペイ型：精算中止
	brand_index = convert_brandno(pRes->brand_no);
	if (!EcBrandPostPayCheck(brand_index)) {
		// プリペイド型
		if (ECCTL.ec_Disp_No == 96) {
			// 未引き去りの場合
			miryo_timeout_after_disp();							// 未了タイムアウト後の表示を行う
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
		}
		else {
			ReqEcAlmRctPrint(J_PRI, pRes);						// アラームレシート印字要求
			EcDeemedSettlement(&OPECTL.Ope_mod);				// 障害発生時の処理
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
			// EcDeemedSettlement 内でLCDへ通知
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
		}
	}
	else {
		// ポストペイ型
		miryo_timeout_after_disp();								// 未了タイムアウト後の表示を行う
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
		if( brand_index != EC_ID_USED){
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
		}else{
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0c);	// 12=未了(決済タイムアウト)(iD)
		}
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善 GT-4100
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			未了タイムアウトエラー登録
//[]----------------------------------------------------------------------[]
///	@param[in]		*pRes	：決済結果データ
///	@param[in]		err		：エラー番号
///							  0: エラー通知なし
///							  1: エラー(E3266)を通知する
///							  2: エラー(E3267)を通知する
///	@return			ret		：none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/02<br>
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void EcRegistMiryoTimeoutError(EC_SETTLEMENT_RES *pRes, char err)
{
	uchar	brand_index;
	ushort	size;

	brand_index = convert_brandno(pRes->brand_no);							// ブランド取得

	// エラー登録
	if (err != 0) {
		memset(err_wk, 0, sizeof(err_wk));
		memset(&err_wk[0], ' ', 31);

		// カード種別,区切り文字
		memcpy(&err_wk[0], "\?\?:", 3);
		memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);

		// カード番号
		if (brand_index != EC_UNKNOWN_USED && brand_index != EC_ZERO_USED) {

			switch (brand_index) {
			case EC_KOUTSUU_USED:
				// 17桁
				size = ECARDID_SIZE_KOUTSUU;
				break;
			default:
				// 16桁
				size = ECARDID_SIZE_SUICA;
				break;
			}
			memcpy(&err_wk[3], &pRes->Card_ID, (size_t)size);

			// マスク処理
			switch (brand_index) {
			case EC_KOUTSUU_USED:
				memset(&err_wk[5], '*', 11);								// 上位2桁、下位4桁以外をマスク
				break;
			case EC_EDY_USED:
				memset(&err_wk[3], '*', 12);								// 下位4桁以外をマスク
				break;
			default:
			case EC_WAON_USED:
				// WAONはマスクされて通知される
			case EC_NANACO_USED:
				// nanacoはマスクされて通知される
				break;
			}
		}

		// 取引額
		memset(suica_work_buf, 0, sizeof(suica_work_buf));
		if (pRes->settlement_data >= 1000) {
		// 4桁以上
			sprintf((char*)suica_work_buf, "\\%ld,%03ld", pRes->settlement_data/1000, pRes->settlement_data%1000);
		}
		else {
		// 3桁以下
			sprintf((char*)suica_work_buf, "\\%ld", pRes->settlement_data);
		}
		sprintf((char*)&err_wk[23], "%8s", suica_work_buf);

		// エラーログ登録
		if (err == 1) {
		// E3266を通知
			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE, 2, 1, 0, (void*)err_wk);
		} else {
		// E3267を通知
			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON, 1, 1, 0, (void*)err_wk);
		}
	}
}
// MH810105(E) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			決済結果NG受信時のｶｰﾄﾞ抜き取りｱﾅｳﾝｽ起動
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/05<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_start_removal_wait(void)
{
	ushort	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);

	if(ec_removal_wait_time == 0) {
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;

// MH810103 GG119202(S) 紙幣抜き取り待ちメッセージが消える
	if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9) {
		return;
	}
// MH810103 GG119202(E) 紙幣抜き取り待ちメッセージが消える

	// ｶｰﾄﾞ差込中？
	if( Suica_Rec.Data.BIT.CTRL_CARD ){
		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );			// 抜き取りﾀｲﾏｰ起動
		ec_MessageAnaOnOff( 1, 4 );													// 「カードをお取り下さい」表示とアナウンスをする
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ関連フラグクリア
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_flag_clear(uchar init)
{
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
	ushort	i, j;
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする

	if (!isEC_USE()) {
		return;
	}

	if (init) {
	// 起動時初期化
		Suica_Rec.Data.BIT.INITIALIZE = 0;						// Init Suica Reader
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
//		Suica_Rec.Data.BIT.BRAND_STS_RCV = 0;					// Brand Status Receive
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
		Suica_Rec.Data.BIT.CTRL_CARD = 0;						// カードなし
// MH810103 GG119202(S) JVMAリセット処理変更
		Suica_Rec.suica_err_event.BYTE &= (uchar)(~EC_NOT_AVAILABLE_ERR);	// 決済リーダ切り離しエラーをクリアする
// MH810103 GG119202(E) JVMAリセット処理変更
		Suica_Rec.suica_err_event.BIT.BRAND_NONE = 1;			// 有効ブランドなしで初期化

		memset(&ECCTL, 0, sizeof(ECCTL));
		memset(&BrandTbl[0], 0, sizeof(BrandTbl));
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
//		memset(&RecvBrandTbl[0], 0, sizeof(RecvBrandTbl));
		for(i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for(j = 0; j < RecvBrandTbl[i].num; j++) {
				RecvBrandTbl[i].ctrl[j].status = 0x0A;			// 閉局／無効／サービス停止中
			}
		}
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
		memset(&RecvMntTbl, 0, sizeof(RecvMntTbl));
		memset(&MntTbl, 0, sizeof(MntTbl));
		memset(&BrandSelTbl, 0, sizeof(BrandSelTbl));

		Status_Retry_Count_OK = 0;								// 受付可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
		Status_Retry_Count_NG = 0;								// 受付不可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ

		ECCTL.phase = EC_PHASE_INIT;							// 初期化中
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;					// ブランド選択の「戻る」検出フラグ初期化
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
		Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;				// 決済処理中受信フラグ初期化
		Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;					// 未了確定フラグ初期化
// MH810103 GG119202(S) ブランド選択結果初期化処理
//		memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
		EcBrandClear();
// MH810103 GG119202(E) ブランド選択結果初期化処理
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		Ec_Settlement_Sts = EC_SETT_STS_NONE;					/* 決済状態もOFFにしておく */
		ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;				/* 決済状態もOFFにしておく */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																// ブランドネゴシエーション管理情報クリア
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
// MH810103 GG119202(S) ブランド選択結果応答データ
		memset(&BrandResRsltTbl, 0, sizeof(BrandResRsltTbl));
// MH810103 GG119202(E) ブランド選択結果応答データ
	}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	else {
	// 新しい精算を始める前に、みなし情報をリセットする
		EcDeemedLog.EcDeemedBrandNo = BRANDNO_UNKNOWN;
// MH810103 GG119202(S) ブランド選択中フラグがクリアされない対策
		Suica_Rec.Data.BIT.SELECT_SND = 0;							// 選択商品データ送信中 解除
		Suica_Rec.Data.BIT.BRAND_SEL = 0;							// ブランド非選択
// MH810103 GG119202(E) ブランド選択中フラグがクリアされない対策
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
		ECCTL.anm_ec_emoney = 0;
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
// MH810103 GG118808_GG118908(S) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
		ECCTL.ec_single_timeout = 0;
// MH810103 GG118808_GG118908(E) 電子マネーシングル設定の読み取り待ちタイムアウト処理変更
	}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

	OPECTL.holdPayoutForEcStop = 0;								// 払出可イベント保留フラグ初期化
	memset(&Ec_Settlement_Res, 0, sizeof(Ec_Settlement_Res));	// 決済結果データ初期化

// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	ec_MessagePtnNum = 0;
//	Product_Select_Brand = 0;
	ECCTL.ec_MessagePtnNum = 0;
	ECCTL.Product_Select_Brand = 0;
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	ECCTL.brandsel_stop = 0;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 QRコード決済対応
	ECCTL.Ec_FailureContact_Sts = 0;							// 0:障害連絡票発行不可
// MH810105(E) MH364301 QRコード決済対応
}

//[]----------------------------------------------------------------------[]
///	@brief			制御データ(受付許可)送信
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/01/08<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcSendCtrlEnableData(void)
{
	ushort	time = OPE_EC_RECEPT_SEND_TIME;

// MH810103 GG119202(S) 理由コード９９受信時は受付許可を送信しない
	if (ECCTL.brandsel_stop != 0) {
		// ブランド処理停止時は受付許可を送信しない
		return;
	}
// MH810103 GG119202(E) 理由コード９９受信時は受付許可を送信しない
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if (timer_recept_send_busy != 0) {
	if (ECCTL.timer_recept_send_busy != 0) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		return;			// 送信遅延タイマー実行中は２重に掛けない
	}
	if (Status_Retry_Count_OK == 0) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//// GG119202(S) クレジットカード精算限度額設定対応
//		if(( OPECTL.Ope_mod == 2 ) &&							// 精算中かつ
//		   ( ENABLE_MULTISETTLE() == 0 )){						// 決済可ブランドなし
//			return;	// 制御データ(受付許可)は送信しない
//		}
//// GG119202(E) クレジットカード精算限度額設定対応
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		// 制御データ(受付許可)送信から10秒経過しても
		// 状態データ(受付可)を受信しない場合はA0193を登録する。
		LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(EC_ENABLE_WAIT_TIME*2+1), snd_enable_timeout );
	}
	Lagtim(OPETCBNO, TIMERNO_EC_RECEPT_SEND, time);		// 受付許可送信タイマースタート
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	timer_recept_send_busy = 1;							// ec受入可送信タイマー動作中
	ECCTL.timer_recept_send_busy = 1;					// ec受入可送信タイマー動作中
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

	Status_Retry_Count_OK = 1;
}
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
//[]----------------------------------------------------------------------[]
///	@brief			ブランド状態データ受信判定情報設定
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	：ブランド状態データ種別
///	@return			ret		：none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/22<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void setEcBrandStsDataRecv( uchar kind )
{
	switch (kind) {					// 受信ﾃﾞｰﾀ判定
	case S_BRAND_SET_DATA: 			// ブランド状態データ１受信
		if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV == 0 ) &&
			( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV = 1;	// ブランド状態データ１(1回目)受信済み
		}
		else if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV == 1 ) &&
				 ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV = 1;	// ブランド状態データ１(2回目)受信済み
		}
		break;
		
	case S_BRAND_SET_DATA2: 		// ブランド状態データ２受信
		if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV == 0 ) &&
			( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV = 1;	// ブランド状態データ２(1回目)受信済み
		}
		else if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV == 1 ) &&
				 ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV = 1;	// ブランド状態データ２(2回目)受信済み
		}
		break;

	default:
		break;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			ブランド状態データ受信チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			TRUE:全て受信済み, FALSE:未受信データあり
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/22<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort isEcBrandStsDataRecv(void)
{
	ushort	ret = FALSE;

	if( ( ECCTL.brand_num <= EC_BRAND_MAX ) &&
		( ( Suica_Rec.ec_negoc_data.brand_rcv_event.ULONG & EC_BRAND10_STS_ALL_RECV ) == EC_BRAND10_STS_ALL_RECV ) ) {
																	// ブランド数が10以下、ブランドネゴシエーション１が完了していない
		ret = TRUE;
	}
	else if( ( ( ECCTL.brand_num <= EC_BRAND_TOTAL_MAX ) && ( ECCTL.brand_num > EC_BRAND_MAX ) ) &&
		( ( Suica_Rec.ec_negoc_data.brand_rcv_event.ULONG & EC_BRAND20_STS_ALL_RECV ) == EC_BRAND20_STS_ALL_RECV ) ) {
																	// ブランド数が10以上、ブランドネゴシエーション１、２が完了していない
		ret = TRUE;
	}

	return( ret );
}
// MH810103 GG119202(E) 起動シーケンス完了条件見直し

// MH810103 GG119202(S) 起動シーケンス不具合修正
//[]----------------------------------------------------------------------[]
///	@brief			ブランドネゴシエーション完了
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	：ブランド状態データ種別
///	@return			ret		：none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/15<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void setEcBrandNegoComplete(void)
{
	Suica_Rec.Data.BIT.INITIALIZE = 1;				// 初期化シーケンス完了
	ECCTL.phase = EC_PHASE_PAY;						// 精算中
	ECCTL.step = 0;

	LagCan500ms(LAG500_EC_WAIT_BOOT_TIMER);			// 起動完了待ちタイマ停止
	// E3210解除
	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 0, 0, 0 );
// MH810103 GG119202(S) 決済リーダからの再起動要求を記録する
	// E3213解除
	err_chk((char)jvma_setup.mdl, ERR_EC_REBOOT, 0, 0, 0);	// エラー解除
// MH810103 GG119202(E) 決済リーダからの再起動要求を記録する
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
	// 決済リーダバージョンアップ中フラグクリア
	EcReaderVerUpFlg = 0;
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応

// MH810103 GG119202(S) JVMAリセット処理不具合
	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0) {
		// 起動シーケンス中に異常データ受信した場合はJVMAリセットを行う
		jvma_trb(ERR_SUICA_RECEIVE);
		return;
	}
// MH810103 GG119202(E) JVMAリセット処理不具合
	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTaskに初期化完了を通知
}

//[]----------------------------------------------------------------------[]
///	@brief			保留していたブランドネゴシエーション再開
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		：none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/15<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Ec_check_PendingBrandNego(void)
{
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	MsgBuf	*msb;
	ushort	msg;
	uchar	brand_index;
	uchar	kind;

	if (!isEC_USE()) {
		return;
	}

	if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
		// JVMAリセットリトライオーバーしている場合は何もしない
		return;
	}
	if (ECCTL.phase != EC_PHASE_BRAND) {
		// ブランド合わせ中フェーズ以外は何もしない
		return;
	}

	if (Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV != 0) {
		// ブランド状態データ受信済み
		brand_index = 0;
		kind = S_BRAND_SET_DATA;
		if (ECCTL.brand_num > EC_BRAND_MAX &&
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV != 0) {
			brand_index = 1;
			kind = S_BRAND_SET_DATA2;
		}
		Ec_Data_Snd(kind, &BrandTbl[brand_index]);	// ブランド設定データ送信
		if (isEcBrandStsDataRecv()) {
			// ブランドネゴシエーション完了
			ECCTL.phase = EC_PHASE_VOL_CHG;			// 音量変更中
			ECCTL.step = 0;

			// 音量変更データ送信
			SetEcVolume();
			Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
		}
		else {
			// 次のブランド状態データ受信待ち
		}
	}
	else {
		// ブランド状態データ未受信の場合はOpe_Ec_Event()での受信処理を待つ
	}
	// ブランド状態データ受信待ち
	Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// 受信待ちタイマ開始

	// ブランドネゴシエーション完了を待つ
	// SUICA_EVTとEC_BRAND_NEGO_WAIT_TIMEOUTを処理する
	Target_WaitMsgID.Count = 2;
	Target_WaitMsgID.Command[0] = SUICA_EVT;
	Target_WaitMsgID.Command[1] = EC_BRAND_NEGO_WAIT_TIMEOUT;
	do {
		taskchg( IDLETSKNO );

		if (ECCTL.phase == EC_PHASE_INIT) {
			// JVMAリセットした場合はループを抜ける
			break;
		}
		if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
			// JVMAリセットリトライオーバーした場合はループを抜ける
			break;
		}

		msb = Target_MsgGet(OPETCBNO, &Target_WaitMsgID);		// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if (msb == NULL) {
			continue;
		}

		msg = msb->msg.command;
		FreeBuf(msb);

		switch (msg) {
		case SUICA_EVT:
			// 受信処理
			Ope_Ec_Event(msg, OPECTL.Ope_mod);
			break;
		case EC_BRAND_NEGO_WAIT_TIMEOUT:
			// フェーズを初期化中に戻してスタンバイコマンドからやり直し
			ECCTL.phase = EC_PHASE_INIT;
			ECCTL.step = 0;
			break;
		default:
			break;
		}

	} while (ECCTL.phase != EC_PHASE_PAY);
}
// MH810103 GG119202(E) 起動シーケンス不具合修正

// MH810103 GG119202(S) クレジットカード精算限度額設定対応
//[]----------------------------------------------------------------------[]
///	@brief			単一ブランド時、有効ブランドを選択済みとする処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/25<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void ec_set_brand_selected( void )
{
	int i, j;

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	EcBrandClear();
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	for( i = 0; i < TBL_CNT(RecvBrandTbl); i++ ){
		for(j = 0; j < RecvBrandTbl[i].num; j++){
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			if( ec_check_valid_brand( RecvBrandTbl[i].ctrl[j].no ) ){	// ブランド有効？
			if( IsValidBrand( i, j ) ){								// ブランド有効？
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
				// 単一ブランド時なので有効ブランドを見つけたら選択済みとして戻る
				RecvBrandResTbl.no = RecvBrandTbl[i].ctrl[j].no;	// 有効ブランドNo.セット
				RecvBrandResTbl.res = EC_BRAND_SELECTED;	// ブランド選択済みとする
				return;
			}
		}
	}

	// 単一ブランド有効の際に起動されるのでこのルートは通らないはずだが
	// もし有効ブランドなしとなった場合はブランド未選択をセットして戻る
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//	RecvBrandResTbl.no = BRANDNO_UNKNOWN;
//	RecvBrandResTbl.res = EC_BRAND_UNSELECTED;	// ブランド未選択とする
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			指定ブランド決済有効/無効判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	:判定するブランドNo.
///	@return			ret			:FALSE(0) = 決済無効/TRUE(1) = 決済有効
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/25<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static BOOL ec_check_valid_brand( ushort brand_no )
{
	int i, j;
	BOOL ret = FALSE;	// ブランド決済無効

	for( i = 0; i < TBL_CNT(RecvBrandTbl); i++ ){
		for( j = 0; j < EC_BRAND_MAX; j++ ){
			if( RecvBrandTbl[i].ctrl[j].no == brand_no ){	// ブランドNo.一致？
				// ブランドNo.一致なら決済有効/無効をチェックして戻る
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//				if(( RecvBrandTbl[i].ctrl[j].status & 0x01 ) == 0x01 ){	// 開局状態？
//					switch( RecvBrandTbl[i].ctrl[j].no ){
//						case BRANDNO_KOUTSUU:
//						case BRANDNO_EDY:
//						case BRANDNO_NANACO:
//						case BRANDNO_WAON:
//						case BRANDNO_SAPICA:
//						case BRANDNO_ID:
//						case BRANDNO_QUIC_PAY:
//							if( WAR_MONEY_CHECK && !e_incnt ){	// 入金なし
//								ret = TRUE;	// 決済有効
//							}
//							break;	// 入金ありなら決済無効
//						case BRANDNO_CREDIT:
//							if( Ec_check_cre_pay_limit() ){	// 精算限度額以下
//								ret = TRUE;	// 決済有効
//							}
//							break;	// 精算限度額超過なら決済無効
//						case BRANDNO_HOUJIN:
//							ret = TRUE;		// 決済有効
//							break;
//						default:
//							// 未定義ブランドは決済無効
//							break;
//					}
//				}	// 閉局状態なら決済無効
//				return ret;
				return IsValidBrand(i, j);
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
			}
		}
	}

	return ret;	// 指定ブランドNo.一致なしなら決済無効
}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//[]----------------------------------------------------------------------[]
///	@brief			ブランドデータクリア
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/02 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcBrandClear(void)
{
	memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
	RecvBrandResTbl.res = EC_BRAND_UNSELECTED;			// 選択ブランドなし
	RecvBrandResTbl.no = BRANDNO_UNKNOWN;				// 選択ブランドなし 
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド選択データの変更チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		in_mony_flg TRUE: 入金中 FALSE:入金中でない
///                 入金中の場合は一時的にryo_buf.zankinを更新してから判定する
///	@return			TRUE :変更あり
///					FALSE:変更なし
///  @note			ブランド選択中でない場合はTRUEを返す。
///                 ブランド選択中は前回送信したブランドデータと変更がある
//                  場合にTRUEを返す。そうでない場合はFALSEを返す。
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/07 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL EcCheckBrandSelectTable(BOOL in_mony_flg)
{
	BOOL ret;
	EC_BRAND_SEL_TBL brand_sel_tbl;
	int i, j;
	unsigned long tmp_zankin;

	if (Suica_Rec.Data.BIT.BRAND_SEL == 0) {	// ブランド選択中でない
		ret = TRUE;
	} else  {
		tmp_zankin = ryo_buf.zankin;
		if (in_mony_flg) {	// 入金中は残金を一時的に更新する
			// 入金中はryo_buf.zankinが未更新である。
			// クレジットカード、法人カードのブランド選択データ判定結果に影響するため
			// 一時的に更新して判定後に元に戻す。
			//  (ryo_buf.zankinは複電処理で参照していないため、一時的に更新しても問題ない)
			ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
		}
		
		memset(&brand_sel_tbl, 0x00, sizeof(brand_sel_tbl));
		for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			// ブランド選択データ作成
			for (j = 0; j < EC_BRAND_MAX; j++) {
				if( IsValidBrand( i, j ) ){				// ブランド有効？
					brand_sel_tbl.no[brand_sel_tbl.num++] = RecvBrandTbl[i].ctrl[j].no;
				}
			}
		}

		ryo_buf.zankin = tmp_zankin;

		// 前回のブランド選択データと比較する。
		if (memcmp(&brand_sel_tbl.no, &BrandSelTbl.no, sizeof(brand_sel_tbl.no)) != 0) {
			ret = TRUE;									/* ブランドテーブルが異なる */
		} else {
			ret = FALSE;										// ブランドテーブルが同じ場合は金額変更を送信しない
		}
	}
	return	ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド有効判定
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_i : ブランドインデックス
///	@param[in]		brand_j : ブランドインデックス
///	@return			TRUE:有効  FALSE:無効
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/12 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static BOOL IsValidBrand(int brand_i, int brand_j)
{
	BOOL ret = FALSE;
	ushort brand_no;

// MH810103 GG119202(S) 開局・有効条件変更
	if (OPECTL.Ope_mod != 2) {
		// 精算中以外は開局状態のみ判定する
// MH810103 GG119202(S) サービス停止中ブランドはブランド設定データにセットしない
//		if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
		if (isEC_BRAND_STS_ENABLE(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
// MH810103 GG119202(E) サービス停止中ブランドはブランド設定データにセットしない
			return TRUE;
		}
	}
// MH810103 GG119202(E) 開局・有効条件変更

// MH810103 GG119202(S) ブランド状態判定処理変更
//	if( (RecvBrandTbl[brand_i].ctrl[brand_j].status & 0x01 ) == 0x01 ){
// MH810103 GG119202(S) サービス停止中ブランドはブランド設定データにセットしない
//	if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
	if (isEC_BRAND_STS_ENABLE(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
// MH810103 GG119202(E) サービス停止中ブランドはブランド設定データにセットしない
// MH810103 GG119202(E) ブランド状態判定処理変更
		brand_no = RecvBrandTbl[brand_i].ctrl[brand_j].no;
		switch( brand_no ) {
		case BRANDNO_KOUTSUU:
		case BRANDNO_EDY:
		case BRANDNO_NANACO:
		case BRANDNO_WAON:
		case BRANDNO_SAPICA:
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
// MH810105(S) MH364301 PiTaPa対応
		case BRANDNO_PITAPA:
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
		case BRANDNO_QR:
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) 電子マネー決済時はジャーナルプリンタ接続設定を参照する
			if (Suica_Rec.Data.BIT.PRI_NG != 0) {
				// ジャーナルプリンタ使用不可時は電子マネー決済不可
				ret = FALSE;
				break;
			}
// MH810103 GG119202(E) 電子マネー決済時はジャーナルプリンタ接続設定を参照する
// MH810103 GG118807_GG118907(S) 交通系ICの限度額を10万円とする
			if (brand_no == BRANDNO_KOUTSUU &&
				ryo_buf.zankin >= EC_PAY_LIMIT_KOUTSUU) {
				// 料金10万円以上の場合は交通系ICを
				// ブランド選択データに含めない
				ret = FALSE;
				break;
			}
// MH810103 GG118807_GG118907(E) 交通系ICの限度額を10万円とする
			if ( WAR_MONEY_CHECK && !e_incnt ) {	// 入金なし
				ret = TRUE;
			} else {
				ret = FALSE;
			}
			break;
		case BRANDNO_CREDIT:
			ret = Ec_check_cre_pay_limit();		// クレジット精算可(限度額以下)
			break;
		case BRANDNO_TCARD:
//			if (( chk_Tcard_use() == 0 )			// Tカード使用可
//			 && ( T_Card_Use == 0 )     ) {		// Tカード未使用
//				ret = TRUE;
//			 } else {
//			 	ret = FALSE;
//			 }	
			break;
		case BRANDNO_HOUJIN:
//			ret = ec_check_houjin_use();
			break;
		default:
			ret = FALSE;
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ブランド有効判定(選択要求をだしたか)
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno : ブランド番号
///	@return			TRUE:有効  FALSE:無効
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/26 MATSUSHITA <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static	BOOL	IsBrandSpecified(ushort brandno)
{
	int i;

	for (i = 0; i < BrandSelTbl.num; i++) {
		if (BrandSelTbl.no[i] == brandno) {
			return TRUE;
		}
	}
	return FALSE;
}

//[]----------------------------------------------------------------------[]
///	@brief			決済有りブランド判定
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno : ブランド番号
///	@return			TRUE:有効  FALSE:無効
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/09/07 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810103 GG119202(S) ハウスカードによるカード処理中タイムアウトは精算中止とする
//static	BOOL	IsSettlementBrand(ushort brandno)
BOOL	IsSettlementBrand(ushort brandno)
// MH810103 GG119202(E) ハウスカードによるカード処理中タイムアウトは精算中止とする
{
	return (BOOL)(brandno <= BRANDNO_for_SETTLEMENT);
}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応参考）
//[]----------------------------------------------------------------------[]
///	@brief			カードデータ通知（決済なしブランド／後決済ブランド）
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	:選択ブランドNo.
///	@param[in]		*dat		:カードデータ
///	@return			ret			:0=未定義ブランド
///								:1=定義された決済なしブランド／後決済ブランド
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/09/24<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar EcNotifyCardData(ushort brand_no, uchar *dat)
{
	uchar	ret = 0;

	switch (brand_no) {
	// 決済なしブランド／後決済ブランドの場合はOPEタスクへイベントを通知して
	// カードデータチェックを行ってください
	// カードデータOKの場合はEc_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1)を実行してください
	// カードデータNGの場合はEc_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0)を実行してください
	case BRANDNO_TCARD:			// Tカード
//		memset(&MAGred_HOUJIN[0], 0x00, sizeof(MAGred_HOUJIN));	
//		memcpy(&MAGred_HOUJIN[MAG_ID_CODE], dat, EC_CARDDATA_LEN);	// MAGred_HOUJIN 256 byte > RecvBrandResTbl.dat 128byte
//		queset( OPETCBNO, EC_BRAND_RESULT_TCARD, 0, 0);				// opetaskにブランド選択結果 Tカードを通知
//		ret = 1;
		break;
	case BRANDNO_HOUJIN:		// 法人カード
//		memset(&MAGred_HOUJIN[0], 0x00, sizeof(MAGred_HOUJIN));	
//		memcpy(&MAGred_HOUJIN[MAG_ID_CODE], dat, EC_CARDDATA_LEN);	// MAGred_HOUJIN 256 byte > RecvBrandResTbl.dat 128byte
//		queset( OPETCBNO, EC_BRAND_RESULT_HOUJIN, 0, 0); 			// opetaskにブランド選択結果 Houjinカードを通知
//		ret = 1;
		break;
	default:
		break;
	}
	return ret;
}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応参考）

// MH810103 GG119202(S) JVMAリセット処理変更
//[]----------------------------------------------------------------------[]
///	@brief			保留していたJVMAリセット要求
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/20
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Ec_check_PendingJvmaReset(void)
{
// MH810103 GG119202(S) JVMAリセット処理不具合
//	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) == 0 &&
//		 Suica_Rec.suica_err_event.BIT.COMFAIL == 0) {
//		// 通信不良、または、決済リーダ切り離しエラー未発生は何もしない
//		return;
//	}
	short	timer;

	if (ECCTL.phase != EC_PHASE_WAIT_RESET) {
		// リセット待ち以外は何もしない
		return;
	}
// MH810103 GG119202(E) JVMAリセット処理不具合
// MH810103 GG119202(S) JVMAリセット処理変更
//	if (Suica_Rec.Data.BIT.RESET_COUNT != 0) {
	if (Suica_Rec.Data.BIT.RESET_COUNT != 0 ||
		Suica_Rec.Data.BIT.RESET_RESERVED != 0) {
		// JVMAリセット要求中は何もしない
// MH810103 GG119202(E) JVMAリセット処理変更
		// JVMAリセット済みでスタンバイに対する応答なしは何もしない
		return;
	}
	if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
		// JVMAリセットリトライオーバーは何もしない
		return;
	}
	else {
		// JVMAリセットリトライ回数インクリメント
		Ec_Jvma_Reset_Count++;
		if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
			// JVMAリセットリトライオーバーは何もしない
// MH810103 GG119202(S) 決済停止エラーの登録
			err_chk( (char)jvma_setup.mdl, ERR_EC_PAY_STOP, 1, 0, 0 );
// MH810103 GG119202(E) 決済停止エラーの登録
			return;
		}
	}

	// JVMAリセットを要求する
	Suica_Rec.Data.BIT.RESET_RESERVED = 1;
// MH810103 GG119202(S) JVMAリセット処理不具合
//// GG119202(S) JVMAリセット処理変更
//	// jvma_trb()より移動
//	// 初期化シーケンス予約中なので初期化済みフラグをクリアする
//	Suica_Rec.Data.BIT.INITIALIZE = 0;
//	// 初期化シーケンス予約中なのでフェーズを初期化中に更新する
//	ECCTL.phase = EC_PHASE_INIT;
//// GG119202(E) JVMAリセット処理変更
	// コインメックタスクでのJVMAリセット待ち
	xPause(16);

	// JVMAリセットから3分(固定)経過しても決済リーダの初期化シーケンスが完了しない場合、
	// E3210を登録する
	timer = OPE_EC_WAIT_BOOT_TIME * 2;
	LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH810103 GG119202(E) JVMAリセット処理不具合
}
// MH810103 GG119202(E) JVMAリセット処理変更

// MH810103 GG119202(S) みなし決済扱い時の動作
//[]----------------------------------------------------------------------[]
///	@brief			みなし決済扱いのエラー登録
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=みなし決済(精算完了), 1=決済処理失敗(休業),
///							  2=決済処理失敗(休業)(復電),
///							  3=決済処理失敗(精算中止)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/20
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//void EcRegistSettlementAbort(void)
void EcRegistDeemedSettlementError(uchar kind)
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
{
	uchar	brand_index;
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
	uchar	reason;
// MH810103 GG119202(E) 決済状態に「みなし決済」追加

	memset(err_wk, 0, sizeof(err_wk));
	memcpy(err_wk, "\?\?:", 3);
	// カード種別
	brand_index = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
	memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);
// MH810103 MHUT40XX(S) E3247印字時の理由コード間違いを修正
	if (kind == 2) {
		// 精算機電源OFF
		reason = 5;
	}
	else
// MH810103 MHUT40XX(E) E3247印字時の理由コード間違いを修正
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
	if (Suica_Rec.suica_err_event.BIT.COMFAIL != 0) {
		// 通信不良
		reason = 1;
	}
	else if (Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL != 0) {
		// カード処理中タイムアウト
		reason = 2;
	}
// MH810103 GG119202(S) カード処理中受信後の決済NG（304）の扱い
	else if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
		// カード処理中受信後に決済NG（304）受信（未了確定前）
		reason = 4;
	}
// MH810103 GG119202(E) カード処理中受信後の決済NG（304）の扱い
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	else if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {
		// 未了残高照会タイムアウト（50-0010、11①＝1）
		reason = 6;
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	else {
		// 決済状態=「みなし決済」受信
		reason = 3;
	}
	sprintf((char*)&err_wk[3], "%02d", reason);
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
	// 取引額
	memset(suica_work_buf, 0, sizeof(suica_work_buf));
	if (EcDeemedLog.PayPrice >= 1000) {
	// 4桁以上
		sprintf((char*)suica_work_buf, "\\%ld,%03ld", EcDeemedLog.PayPrice/1000, EcDeemedLog.PayPrice%1000);
	}
	else {
	// 3桁以下
		sprintf((char*)suica_work_buf, "\\%ld", EcDeemedLog.PayPrice);
	}
// MH810103 GG119202(S) 決済状態に「みなし決済」追加
//	sprintf((char*)&err_wk[3], " %8s", suica_work_buf);
//	err_chk2((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT, 1, 1, 0, (void*)err_wk);
	sprintf((char*)&err_wk[5], " %8s", suica_work_buf);
	if (kind == 0) {
		err_chk2((char)jvma_setup.mdl, ERR_EC_DEEMED_SETTLEMENT, 2, 1, 0, (void*)err_wk);
	}
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
	else if (kind == 3) {
		err_chk2((char)jvma_setup.mdl, ERR_EC_PROCESS_FAILURE, 2, 1, 0, (void*)err_wk);
	}
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善
	else {
		err_chk2((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT, 1, 1, 0, (void*)err_wk);
	}
// MH810103 GG119202(E) 決済状態に「みなし決済」追加
}
// MH810103 GG119202(E) みなし決済扱い時の動作

// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
//[]----------------------------------------------------------------------[]
///	@brief			決済リーダ接続時の入金許可制御
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/12/07
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcEnableNyukin(void)
{
	if (!isEC_USE()) {
		return;
	}

	if (check_enable_multisettle() <= 1) {
		// マルチブランド以外は何もしない
		return;
	}
// MH810105(S) MH364301 QRコード決済対応
//	if (!isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// 電子マネー選択状態でなければ、何もしない
	if (!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// 電子マネー選択状態でなければ、何もしない
		// QRコード選択状態でなければ、何もしない
// MH810105(E) MH364301 QRコード決済対応
		return;
	}
	if (Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {
		// 決済処理中は何もしない
		return;
	}

	queset(OPETCBNO, EC_EVT_ENABLE_NYUKIN, 0, NULL);
}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする

// MH810103 MHUT40XX(S) Edy・WAON対応／決済結果受信待ちタイマ見直し
//[]----------------------------------------------------------------------[]
///	@brief			タイマ更新制御
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/02/19<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void EcTimerUpdate(void)
{
	ushort	wait_time;

	if (!isEC_STS_CARD_PROC() &&
		!Suica_Rec.Data.BIT.CTRL_MIRYO) {
		// カード処理中、未了中以外はタイマ更新しない
		return;
	}

	// カード処理中、未了中の監視タイマを更新する
	// 決済リーダからタイマ更新依頼が定期的に通知される
	switch (RecvBrandResTbl.no) {
	case BRANDNO_KOUTSUU:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_EDY:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 41, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_WAON:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 42, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	case BRANDNO_NANACO:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 43, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//	case BRANDNO_ID:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 44, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//	case BRANDNO_QUIC_PAY:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 45, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
	case BRANDNO_NANACO:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 43, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_ID:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 44, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_QUIC_PAY:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 45, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810104(E) nanaco・iD・QUICPay対応2
// MH810105(S) MH364301 QRコード決済対応
	case BRANDNO_QR:
		// QRコード
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 47, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
	case BRANDNO_PITAPA:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 46, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810105(E) MH364301 PiTaPa対応
	default:
// MH810105(S) MH364301 QRコード決済対応
//		// 電子マネーブランドのみ更新する
		// 電子マネーブランド、QRコードブランド以外は更新しない
// MH810105(E) MH364301 QRコード決済対応
		return;
	}

	if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		wait_time = (wait_time * 2) + 1;
		LagTim500ms(LAG500_SUICA_MIRYO_RESET_TIMER, (short)wait_time, ec_miryo_timeout);
	}
	else if (isEC_STS_CARD_PROC()) {
		wait_time *= 50;
		Lagtim(OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, wait_time);
	}
}
// MH810103 MHUT40XX(E) Edy・WAON対応／決済結果受信待ちタイマ見直し
// MH810103(s) 電子マネー対応 精算状態通知をOpe_modにより切り替える
//[]----------------------------------------------------------------------[]
///	@brief			電子マネー利用可能チェック(受付可否見ない版)
//[]----------------------------------------------------------------------[]
///	@param[in]		open_chk:		決済可能チェック
///	@param[in]		zandaka_chk:	残高照会可能チェック
///	@return			1:電子マネー利用可, 0:電子マネー利用不可
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar isEcEmoneyEnabledNotCtrl(uchar open_chk, uchar zandaka_chk)
{
	uchar	ret = 0;
	int		i, j;

	if (!isEC_USE()) {
		return ret;
	}

	// 受付可否は見ない
	if (open_chk || zandaka_chk) {
		if (!Suica_Rec.suica_err_event.BYTE					// リーダー使用可
			&& Suica_Rec.Data.BIT.INITIALIZE				// 初期化完了
//			&& Suica_Rec.Data.BIT.CTRL						// 受付可状態
			) {	
			;		// コンディションOK
		}
		else {
			return ret;
		}
	}

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// 受信したブランドテーブルに電子マネーブランドが含まれているか？
			if (isEcBrandNoEMoney(RecvBrandTbl[i].ctrl[j].no, zandaka_chk)) {
				if (open_chk) {
					// 決済可能チェック
					if (IsValidBrand(i, j)) {
						ret = 1;
						break;
					}
				}
				else {
					ret = 1;
					break;
				}
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}
// MH810103(e) 電子マネー対応 精算状態通知をOpe_modにより切り替える

// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
////[]----------------------------------------------------------------------[]
/////	@brief			みなし決済発生時の決済結果データセット
////[]----------------------------------------------------------------------[]
/////	@param[out]		*data	: 決済結果データ
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2021/03/05
////[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//static void EcSetDeemedSettlementResult(EC_SETTLEMENT_RES *data)
//{
//	// ブランド番号
//	data->brand_no = EcDeemedLog.EcDeemedBrandNo;
//	// 支払額
//	data->settlement_data = Product_Select_Data;
//	// 問い合わせ番号
//	memset(data->inquiry_num, '*', sizeof(data->inquiry_num));
//	data->inquiry_num[4] = '-';
//
//	memset(data->Card_ID, 0x20, sizeof(data->Card_ID));
//	switch (data->brand_no) {
//	case BRANDNO_KOUTSUU:
//		// カード番号
//		memset(data->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
//		// SPRWID
//		memset(data->Brand.Koutsuu.SPRW_ID, '*', sizeof(data->Brand.Koutsuu.SPRW_ID));
//		break;
//	case BRANDNO_EDY:
//		// カード番号
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// Edy取引通番
//		memset(data->Brand.Edy.DealNo, '*', sizeof(data->Brand.Edy.DealNo));
//		// カード取引通番
//		memset(data->Brand.Edy.CardDealNo, '*', sizeof(data->Brand.Edy.CardDealNo));
//		// 上位端末ID
//		memset(data->Brand.Edy.TerminalNo, '*', sizeof(data->Brand.Edy.TerminalNo));
//		break;
//	case BRANDNO_WAON:
//		// カード番号
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// SPRWID
//		memset(data->Brand.Waon.SPRW_ID, '*', sizeof(data->Brand.Waon.SPRW_ID));
//		break;
//// MH810104 GG119202(S) nanaco・iD・QUICPay対応
//	case BRANDNO_NANACO:
//		// カード番号
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// 端末取引通番
//		memset(data->Brand.Nanaco.DealNo, '*', sizeof(data->Brand.Nanaco.DealNo));
//		// 上位端末ID
//		memset(data->Brand.Nanaco.TerminalNo, '*', sizeof(data->Brand.Nanaco.TerminalNo));
//		break;
//	case BRANDNO_ID:
//		// カード番号
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// 上位端末ID
//		memset(data->Brand.Id.TerminalNo, '*', sizeof(data->Brand.Id.TerminalNo));
//		break;
//	case BRANDNO_QUIC_PAY:
//		// カード番号
//		memset(data->Card_ID, '*', sizeof(data->Card_ID));
//		// 上位端末ID
//		memset(data->Brand.QuicPay.TerminalNo, '*', sizeof(data->Brand.QuicPay.TerminalNo));
//		break;
//// MH810104 GG119202(E) nanaco・iD・QUICPay対応
//	default:
//		break;
//	}
//}
//// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105 GG119202(E) 処理未了取引集計仕様改善

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//[]----------------------------------------------------------------------[]
///	@brief			受信した直前取引データを格納する関数
//[]----------------------------------------------------------------------[]
///	@return			ret		:	FALSE=セットしない(以降の処理しない)/TRUE=セット完了(以降の処理に移行)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/05/07
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcRecvDeemed_DataSet(uchar *buf)
{
	ec_settlement_data_save(&Ec_Settlement_Res, buf, 0);

	if( Ec_Settlement_Res.Result == EPAY_RESULT_CANCEL_PAY_OFF ){
		// ブランドに関係なく、決済結果＝処理キャンセルの場合は何も処理しない
		return FALSE;
	}
	if( (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END && Ec_Settlement_Res.brand_no == BRANDNO_CREDIT)||
		(Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO && Ec_Settlement_Res.brand_no == BRANDNO_CREDIT) ){
		// ブランドがクレジットであり、決済結果＝未了残高照会完了or未了確定の場合は何も処理しない
		return FALSE;
	}
// MH810105(S) MH364301 QRコード決済対応
	if (Ec_Settlement_Res.brand_no == BRANDNO_QR &&
		(Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO ||
		 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END)) {
		// ブランドがQRコードで決済結果＝未了確定、未了残高照会完了の場合は何も処理しない
		return FALSE;
	}
	if (Ec_Settlement_Res.brand_no != BRANDNO_QR &&
		Ec_Settlement_Res.Result == EPAY_RESULT_PAY_MIRYO) {
		// ブランドがQRコード以外で決済結果＝支払未了の場合は何も処理しない
		return FALSE;
	}
// MH810105(E) MH364301 QRコード決済対応

	// EcRecvDeemedDataに受信データをセット
	EcElectronSet_DeemedData(&Ec_Settlement_Res);
	// エラーコードもEcRecvDeemedDataにセットする
	memcpy( &EcRecvDeemedData.EcErrCode, &Ec_Settlement_Rec_Code, sizeof(EcRecvDeemedData.EcErrCode) );
// MH810105 GG119202(S) 未了確定の直取データ受信時にR0176を登録する
	if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {
		// 直取結果＝未了確定の場合はR0176を登録する
		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);
	}
// MH810105 GG119202(E) 未了確定の直取データ受信時にR0176を登録する

	return TRUE;
}
//[]----------------------------------------------------------------------[]
///	@brief			直前取引データのログ登録、印字処理
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/28
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcRecvDeemed_RegistPri( void )
{
	uchar	brand;
	T_FrmReceipt	DeemedData;

	memset(&DeemedData, 0, sizeof(T_FrmReceipt));
	brand = EcRecvDeemedData.Electron_data.Ec.e_pay_kind;

	// 出庫時刻に精算機の現在時刻をセットする（印字時刻としても扱う）
	memcpy(&EcRecvDeemedData.TOutTime, &CLK_REC, sizeof(date_time_rec));

	if ( brand != EC_UNKNOWN_USED && EcRecvDeemedData.EcResult != EPAY_RESULT_NG ) {
		// 不明ブランド以外ログ登録
		// 決済結果NGの場合はログ登録しない
		ac_flg.cycl_fg = 57;											// 57:決済精算中止時
		ac_flg.ec_recv_deemed_fg = 0;									// 直前取引データ処理フラグOFF
		Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
	}
	else {
		ac_flg.cycl_fg = 58;											// 58:決済精算中止データ受付
		ac_flg.ec_recv_deemed_fg = 0;									// 直前取引データ処理フラグOFF
	}

	if( PrnJnlCheck() == ON ){
		DeemedData.prn_kind = J_PRI;									/* ﾌﾟﾘﾝﾀ種別：ｼﾞｬｰﾅﾙ */
		DeemedData.prn_data = &EcRecvDeemedData;						/* 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ */

		// pritaskへ通知
		queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &DeemedData);
	}

	// 詳細エラーコードを登録する
	EcErrCodeChk( EcRecvDeemedData.EcErrCode, brand );

	ac_flg.cycl_fg = 0;													/* 直前取引データ印字処理完了*/
// MH810105 GG119202(S) リーダから直取中の状態データ受信した際の新規アラームを設ける
	if( !EC_STATUS_DATA.StatusInfo.LastSetteRunning ){					// 状態データの直取中bit OOFF
		// 「決済リーダ利用不可」解除
		alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 0);					// ｱﾗｰﾑﾛｸﾞ登録（解除）
	}
// MH810105 GG119202(E) リーダから直取中の状態データ受信した際の新規アラームを設ける	
}

//[]----------------------------------------------------------------------[]
///	@brief			詳細エラーコード、エラー登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]		err_code	   　：　発生した詳細エラーコード
///	@param[in]		err_brand_index　：　発生した詳細エラーコードのブランド
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/27
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcErrCodeChk( uchar *err_code, uchar err_brand_index )
{
	// 詳細エラーコード
	memset(err_wk, 0, sizeof(err_wk));
	memcpy(err_wk, "詳細コード000", 13);
	memcpy(&err_wk[10], err_code, sizeof(Ec_Settlement_Rec_Code));

	ECCTL.ec_Disp_No = 0;												// 決済結果詳細エラーコードの案内表示パターン

	// 詳細エラーコードが 000 ではない
	if (!(err_code[0] == '0' &&
		  err_code[1] == '0' &&
		  err_code[2] == '0')) {
		ec_errcode_err_chk(err_wk, err_code, err_brand_index);			// 詳細エラー登録
	}
	// 詳細エラーコード201ではない && ブランド番号が 0
	if(!(err_code[0] == '2' &&
		 err_code[1] == '0' &&
		 err_code[2] == '1') &&
		 err_brand_index == EC_ZERO_USED){
		err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 1, 0, 0);	// E3265登録
	}
}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

// MH810105 GG119202(S) 決済処理中に障害が発生した時の動作
//[]----------------------------------------------------------------------[]
///	@brief			決済処理中に障害が発生した時のブランド毎の動作(50-0014)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: ブランドNo
///	@return			ret			: 0=みなし決済で精算完了
///								: 1=休業
///								: 2=クレジット特有の処理(決済結果NGとする)
///								: 3=QRコード特有の処理(障害連絡表発行+決済結果NG)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/16<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static uchar EcGetActionWhenFailure( ushort brand_no )
{
	uchar	brand_system = 0;									// 1：電子マネー（プリペイド） 2：電子マネー（ポストペイ）
																// 3：クレジット 4：QRコード
	uchar	wk_process = 0;										// 0：みなし決済で精算完了
	char	pos;

	// みなしが発生した決済方法を確認し
	// それぞれの設定により処理を変更する
	brand_system = isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 0);
	if (brand_system != 0) {
		if (isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 1)) {
			// 電子マネー（プリペイド）
			brand_system = 1;
		}
		else {
			// 電子マネー（ポストペイ）
			brand_system = 2;
		}
	}

	// 電子マネーではなかった
	if( brand_system == 0 ){
		// クレジットかをチェック
		if( brand_no == BRANDNO_CREDIT ){
			brand_system = 3;									// 2：クレジット
		}
// MH810105(S) MH364301 QRコード決済対応
		else if( brand_no == BRANDNO_QR ){
			brand_system = 4;									// 4：QRコード系
		}
// MH810105(E) MH364301 QRコード決済対応
	}

	switch( brand_system ){
		case 1:													// 電子マネー（プリペイド）
// MH810105(S) MH364301 QRコード決済対応
			if (brand_no == BRANDNO_WAON) {
				switch (prm_get(COM_PRM, S_ECR, 14, 1, 4)){
				case 1:
					wk_process = 1;								// 1：休業
					break;
				default:
					wk_process = 0;								// 0：みなし決済で精算完了
					break;
				}
			}
			else {
// MH810105(E) MH364301 QRコード決済対応
			switch (prm_get(COM_PRM, S_ECR, 14, 1, 2)){
				case 1:
					wk_process = 1;								// 1：休業
					break;
// MH810105(S) MH364301 決済処理中に障害が発生した時の動作処理改善
				case 2:
					wk_process = 2;								// 2：精算中止
					break;
// MH810105(E) MH364301 決済処理中に障害が発生した時の動作処理改善
				default:
					wk_process = 0;								// 0：みなし決済で精算完了
					break;
			}
// MH810105(S) MH364301 QRコード決済対応
			}
// MH810105(E) MH364301 QRコード決済対応
			break;
		case 2:													// 電子マネー（ポストペイ）
		case 3:													// クレジット
			// パラメータの参照位置
			if (brand_system == 2) {
				pos = 3;
			}
			else {
				pos = 1;
			}
			switch (prm_get(COM_PRM, S_ECR, 14, 1, pos)){
				case 1:
					wk_process = 1;								// 1：休業
					break;
				case 2:
					wk_process = 2;								// 2：クレジット特有の処理(決済結果NGとする)
					break;
				default:
					wk_process = 0;								// 0：みなし決済で精算完了
					break;
			}
			break;
		case 4:													// QRコード系
// MH810105(S) MH364301 QRコード決済対応
//			switch (prm_get(COM_PRM, S_ECR, 14, 1, 4)){
//				case 1:
//					wk_process = 1;								// 1：休業
//					break;
//				case 2:
//					wk_process = 3;								// 3：QRコード特有の処理(障害連絡表発行+決済結果NG)
//					break;
//				default:
//					wk_process = 0;								// 0：みなし決済で精算完了
//					break;
//			}
			switch (prm_get(COM_PRM, S_ECR, 14, 1, 5)){
			case 1:
				wk_process = 1;									// 1：休業
				break;
			case 2:
				wk_process = 0;									// 0：みなし決済で精算完了
				break;
			default:
				wk_process = 3;									// 3：QRコード特有の処理(障害連絡表発行+精算中止)
				break;
			}
// MH810105(E) MH364301 QRコード決済対応
			break;
		default:
			break;
	}
	return wk_process;
}
// MH810105 GG119202(E) 決済処理中に障害が発生した時の動作

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//[]----------------------------------------------------------------------[]
///	@brief			未了残高照会タイムアウト時の動作
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: ブランドNo.
///	@return			ret			: 0=みなし決済で精算完了
///								: 1=休業
///								: 2=精算中止
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/17
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static uchar EcGetMiryoTimeoutAction(ushort brand_no)
{
	uchar	wk_process = 0;
	char	err;


	if (brand_no == BRANDNO_WAON) {
		switch (prm_get(COM_PRM, S_ECR, 11, 1, 6)) {
		case 2:
			wk_process = 1;							// 1：休業

			// E3267を通知する
			err = 2;
			break;
		default:
			wk_process = 0;							// 0：みなし決済で精算完了

			// 未了タイムアウト通知方法を取得
			err = (char)prm_get(COM_PRM, S_ECR, 11, 1, 5);
			break;
		}
	}
	else {
		switch (prm_get(COM_PRM, S_ECR, 10, 1, 6)) {
		case 2:
			wk_process = 1;							// 1：休業

			// E3267を通知する
			err = 2;
			break;
		case 0:
			wk_process = 2;							// 2：精算中止

			// 未了タイムアウト通知方法を取得
			err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
			break;
		default:
			wk_process = 0;							// 0：みなし決済で精算完了

			// 未了タイムアウト通知方法を取得
			err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
			break;
		}
	}

	// 未了残高照会タイムアウト時のエラー登録
	EcRegistMiryoTimeoutError(&Ec_Settlement_Res, err);

	return wk_process;
}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

// MH810105 GG119202(S) みなし決済プリントに直前取引内容を印字する
//[]----------------------------------------------------------------------[]
///	@brief			未了引き去り失敗判定
//[]----------------------------------------------------------------------[]
///	@param[in]		pData		: PayDataのポインタ
///	@return			ret			: 0=引き去り失敗以外
///								: 1=引き去り失敗
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/30
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsEcMiryoHikisariNG(Receipt_data *pData)
{
	uchar	ret = 0;

	// 決済結果＝未了確定、かつ、詳細エラー＝h90の場合は
	// 未了引き去り失敗と判定する
	if (pData->EcResult == EPAY_RESULT_MIRYO &&
		!memcmp(pData->EcErrCode, EcMiryoFailErrTbl, 3)) {
		ret = 1;
	}
	return ret;
}
// MH810105 GG119202(E) みなし決済プリントに直前取引内容を印字する

// MH810105 GG119202(S) 電子マネー決済エラー発生時の画面戻り仕様変更
//[]----------------------------------------------------------------------[]
///	@brief			読み取り待ち継続判定
//[]----------------------------------------------------------------------[]
///	@param[in]		pErrCode	: 詳細エラーのポインタ
///	@return			ret			: 0=読み取り待ち継続なし
///								: 1=読み取り待ち継続あり
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/09/17
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsEcContinueReading(uchar *pErrCode)
{
	uchar	ret = 0;

	// 読み取り待ち継続するエラーを判定する
	if (!memcmp(pErrCode, EcContinueReadingTbl, 3)) {
		ret = 1;
	}
	return ret;
}
// MH810105 GG119202(E) 電子マネー決済エラー発生時の画面戻り仕様変更

// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			読み取り待ち継続判定
//[]----------------------------------------------------------------------[]
///	@param[in]		pErrCode	: 詳細エラーのポインタ
///	@return			ret			: 0=読み取り待ち継続なし
///								: 1=読み取り待ち継続あり
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/12/16
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsQrContinueReading(uchar *pErrCode)
{
	uchar	ret = 0;
	uchar	i = 0;

	// 読み取り待ち継続するエラーを判定する
	for (i = 0;i < TBL_CNT(QrContinueReadingTbl);i++) {
		if (!memcmp(pErrCode, QrContinueReadingTbl[i], 3)) {
			ret = 1;
			break;
		}
	}
	return ret;
}
// MH810105(E) MH364301 QRコード決済対応

// MH810105(S) MH364301 PiTaPa対応
//[]----------------------------------------------------------------------[]
///	@brief			電子マネーブランドポストペイ型チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_index	: ブランド番号
///	@return			ret			: 0=プリペイド型
///								: 1=ポストペイ型
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/29
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcBrandPostPayCheck( uchar brand_index )
{
	uchar	ret = 0;

	// ポストペイ型かを判定する
	switch ( brand_index ){
		case EC_ID_USED:			// iD決済
		case EC_QUIC_PAY_USED:		// QUICPay決済
		case EC_PITAPA_USED:		// PiTaPa決済
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}
// MH810105(E) MH364301 PiTaPa対応
