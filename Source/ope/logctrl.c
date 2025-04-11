/*[]----------------------------------------------------------------------[]*/
/*| LOG関連関数                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2001.10.31                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"flp_def.h"
#include	"LKcom.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"rau.h"
#include	"rauconstant.h"
#include	"ifm_ctrl.h"
#include	"oiban.h"

#define	LOGOPERATION_COM	0x01	/* 通信 */
#define	LOGOPERATION_PRI	0x02	/* 印字 */
#define	LOGOPERATION_RAM	0x04	/* ログ */
#define	LOGOPERATION_ALL	0xFF	/* 全て実行 */

/* モニタ/操作モニタデータ制御テーブル構造 */

typedef struct {
	uchar	ID;						/* コード */
	uchar	Level;					/* レベル */
	uchar	Operation;				/* 処理方法 */
									/* (LOGOPERATION_COM/LOGOPERATION_PRI/LOGOPERATION_RAMのビット情報) */
//	char	*Msg;					/* メッセージ(H8系CPUではセットしない) */
} t_NTNET_MonitorCtrl;

typedef struct {
	uchar	Kind;					/* 種別 */
	ulong	DataNum;				/* 配列「CtrlData」の要素数 */
	const t_NTNET_MonitorCtrl	*CtrlData;	/* 各コード毎の取り扱い情報 */
} t_NTNET_MonitorTbl;
static TD_SETDIFFLOG_INFO	SetDiffWorkBuff;					// データ編集ワーク
static unsigned char		SetDiffFirstFlag;					// 初回格納判定フラグ
extern unsigned short		SetDiffLogDtHeadPon;				// 個別ヘッダー位置
static long*				CPrmMrr[C_PRM_SESCNT_MAX];			// ミラー領域管理用アドレス配列
extern const ushort AppServ_CParamSizeTbl[C_PRM_SESCNT_MAX];	// セッション毎パラメータ数テーブル
extern t_CParam CPMirror;										// 共通パラメータミラー領域
void SetSetDiff(unsigned char syubetu);
void SetSetDiff_Set(unsigned char syubetu);
void SetDiff_PutLogInfoData( short ses, short addr, long dataBef, long dataAft );
void SetDiff_PutLogInfoHead( unsigned char syubetu );
void SetDiff_PutLogInfoSave( unsigned char cnt_add_flag );
void SetDiffLogBackUp( void );
void SetDiffLogRecoverCheck_Go( void );
void SetDiffLogMirrorAddress(void);
unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon );

extern	ushort	FLT_Get_LogSectorCount(short id);


/* 操作モニタログデータ制御テーブル */
static const t_NTNET_MonitorCtrl c_OpeMonitorKind00[] = {
	{1,		5,	LOGOPERATION_ALL},// 全イニシャライズ起動(スーパーイニシャル)
	{2,		5,	LOGOPERATION_ALL},// データイニシャル起動(ログイニシャル)
};
static const t_NTNET_MonitorCtrl c_OpeMonitorKind01[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
	{3,		5,	LOGOPERATION_ALL},
	{4,		5,	LOGOPERATION_ALL},
	{5,		5,	LOGOPERATION_ALL},
	{6,		5,	LOGOPERATION_ALL},
	{7,		5,	LOGOPERATION_ALL},
	{8,		5,	LOGOPERATION_ALL},
	{11,	5,	LOGOPERATION_ALL},
	{12,	5,	LOGOPERATION_ALL},
	{15,	5,	LOGOPERATION_ALL},
	{20,	5,	LOGOPERATION_ALL},
	{21,	5,	LOGOPERATION_ALL},
	{22,	5,	LOGOPERATION_ALL},
	{28,	5,	LOGOPERATION_ALL},
	{29,	5,	LOGOPERATION_ALL},
	{30,	4,	LOGOPERATION_ALL},
	{31,	4,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind11[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
	{3,		5,	LOGOPERATION_ALL},
	{4,		5,	LOGOPERATION_ALL},
	{5,		5,	LOGOPERATION_ALL},
	{6,		5,	LOGOPERATION_ALL},
	{7,		5,	LOGOPERATION_ALL},
	{8,		3,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind12[] = {
	{1,		3,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{3,		3,	LOGOPERATION_ALL},
	{4,		3,	LOGOPERATION_ALL},
	{5,		4,	LOGOPERATION_ALL},
	{6,		3,	LOGOPERATION_ALL},
	{7,		3,	LOGOPERATION_ALL},		// MT小計
	{8,		4,	LOGOPERATION_ALL},		// MT合計
	{9,		3,	LOGOPERATION_ALL},		// 前回MT合計
	{10,	3,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{12,	3,	LOGOPERATION_ALL},
	{20,	3,	LOGOPERATION_ALL},
	{21,	4,	LOGOPERATION_ALL},
	{22,	3,	LOGOPERATION_ALL},
	{23,	4,	LOGOPERATION_ALL},
	{40,	5,	LOGOPERATION_ALL},		// T合計中止
	{41,	5,	LOGOPERATION_ALL},		// GT合計中止
	{42,	5,	LOGOPERATION_ALL},		// MT合計中止
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind13[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind15[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},	// 全ロック装置開
	{11,	4,	LOGOPERATION_ALL},	// フラップ下降
	{12,	4,	LOGOPERATION_ALL},	// 全フラップ下降
	{21,	4,	LOGOPERATION_ALL},	// フラップセンサ強制OFF
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind16[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},	// 全ロック装置閉
	{11,	4,	LOGOPERATION_ALL},	// フラップ上昇
	{12,	4,	LOGOPERATION_ALL},	// 全フラップ上昇
	{21,	4,	LOGOPERATION_ALL},	// フラップセンサ強制ON
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind17[] = {
	{1,		3,	LOGOPERATION_ALL},
	{2,		3,	LOGOPERATION_ALL},
	{3,		3,	LOGOPERATION_ALL},
	{4,		3,	LOGOPERATION_ALL},
	{5,		3,	LOGOPERATION_ALL},
	{6,		3,	LOGOPERATION_ALL},
	{7,		3,	LOGOPERATION_ALL},
	{8,		3,	LOGOPERATION_ALL},
	{10,	3,	LOGOPERATION_ALL},
	{11,	3,	LOGOPERATION_ALL},
	{12,	3,	LOGOPERATION_ALL},
	{13,	3,	LOGOPERATION_ALL},
	{14,	3,	LOGOPERATION_ALL},
	{15,	3,	LOGOPERATION_ALL},
	{16,	3,	LOGOPERATION_ALL},
	{17,	3,	LOGOPERATION_ALL},
	{18,	3,	LOGOPERATION_ALL},
	{19,	3,	LOGOPERATION_ALL},
	{20,	3,	LOGOPERATION_ALL},
	{21,	3,	LOGOPERATION_ALL},
	{22,	3,	LOGOPERATION_ALL},
	{23,	3,	LOGOPERATION_ALL},
	{24,	3,	LOGOPERATION_ALL},
	{25,	3,	LOGOPERATION_ALL},
	{26,	3,	LOGOPERATION_ALL},
	{50,	3,	LOGOPERATION_ALL},
	{51,	3,	LOGOPERATION_ALL},
	{52,	3,	LOGOPERATION_ALL},
	{53,	3,	LOGOPERATION_ALL},
	{68,	3,	LOGOPERATION_ALL},
	{69,	3,	LOGOPERATION_ALL},
	{70,	3,	LOGOPERATION_ALL},
	{71,	3,	LOGOPERATION_ALL},
	{72,	3,	LOGOPERATION_ALL},
	{73,	3,	LOGOPERATION_ALL},
	{74,	3,	LOGOPERATION_ALL},
	{75,	3,	LOGOPERATION_ALL},

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	{76,	5,	LOGOPERATION_ALL},			// Edyｱﾗｰﾑ取引情報ﾌﾟﾘﾝﾄ
//	{77,	5,	LOGOPERATION_ALL},			// Edyｱﾗｰﾑ取引情報ｸﾘｱ
//	{78,	5,	LOGOPERATION_ALL},			// 関連設定変更
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	{79,	3,	LOGOPERATION_ALL},	// 音声案内時間変更
	{80,	3,	LOGOPERATION_ALL},	// 特別曜日変更
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	{81,	5,	LOGOPERATION_ALL},			// Edyｱﾗｰﾑ締め記録情報ﾌﾟﾘﾝﾄ
//	{82,	5,	LOGOPERATION_ALL},			// Edyｱﾗｰﾑ締め記録情報ｸﾘｱ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	{83,	3,	LOGOPERATION_ALL},
	{90,	5,	LOGOPERATION_ALL},			// プログラムダウンロード実施
	{91,	5,	LOGOPERATION_ALL},			// 共通パラメータダウンロード実施
	{92,	5,	LOGOPERATION_ALL},			// メインプログラム運用面切替予約実施
	{93,	5,	LOGOPERATION_ALL},			// サブプログラム運用面切替予約実施
	{94,	5,	LOGOPERATION_ALL},			// 共通パラメーター運用面切替予約実施
	{95,	3,	LOGOPERATION_ALL},			// 料金設定プリント

	{97,	3,	LOGOPERATION_ALL},
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//	{98,	3,	LOGOPERATION_ALL},
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
	{99,	3,	LOGOPERATION_ALL},

};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind18[] = {
	{1,		3,	(LOGOPERATION_ALL)},
	{2,		3,	(LOGOPERATION_ALL)},
	{3,		3,	(LOGOPERATION_ALL)},
	{4,		3,	(LOGOPERATION_ALL)},
	{5,		3,	(LOGOPERATION_ALL)},
	{6,		3,	(LOGOPERATION_ALL)},
	{30,	5,	(LOGOPERATION_ALL)},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind20[] = {
	{1,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{10,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{20,	4,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind23[] = {
	{1,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{4,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{5,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{6,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{7,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{8,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{9,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{10,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{13,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{14,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{18,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{19,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{20,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{23,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{24,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{25,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{26,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{27,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{28,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{38,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{39,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{41,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{42,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(Park I PROﾁｪｯｸ)履歴登録
	{43,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(遠隔通信ﾁｪｯｸ)履歴登録
	{44,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	{45,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(Edyﾁｪｯｸ)履歴登録 
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	{46,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{47,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(ﾛｯｸ装置動作ｶｳﾝﾄ-ｸﾘｱ)
	{48,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(ﾌﾗｯﾌﾟ動作ｶｳﾝﾄ)
	{49,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 動作ﾁｪｯｸ(ﾌﾗｯﾌﾟ動作ｶｳﾝﾄ-ｸﾘｱ)
	{50,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{61,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{70,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// CRR基板バージョンチェック実施
	{71,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// CRR基板折り返しテスト実施
	{72,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// Rismサーバー接続確認
	{73,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{74,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{75,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{78,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{79,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{80,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
// MH321800(S) D.Inaba ICクレジット対応
	{83,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// 決済リーダチェック実施
// MH321800(E) D.Inaba ICクレジット対応
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind25[] = {
	{2,	5,	LOGOPERATION_ALL},
	{20,	5,	LOGOPERATION_ALL},
	{21,	5,	LOGOPERATION_ALL},	// 車室故障設定（故障なし→あり）
	{22,	5,	LOGOPERATION_ALL},	// 車室故障設定（故障あり→なし）
	{23,	3,	LOGOPERATION_ALL},	// 受付券発行設定
	{70,	3,	LOGOPERATION_ALL},	// クレジット未送信売上再送
	{71,	3,	LOGOPERATION_ALL},	// クレジット未送信売上削除
	{72,	3,	LOGOPERATION_ALL},	// クレジット売上拒否データ削除
	{73,	3,	LOGOPERATION_ALL},	// 接続確認テスト実施
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind80[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{5,		4,	LOGOPERATION_ALL},
	{10,	4,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{12,	4,	LOGOPERATION_ALL},
	{13,	4,	LOGOPERATION_ALL},
	{14,	4,	LOGOPERATION_ALL},
	{15,	4,	LOGOPERATION_ALL},
	{16,	4,	LOGOPERATION_ALL},
	{17,	4,	LOGOPERATION_ALL},
	{28,	4,	LOGOPERATION_ALL},
	{30,	4,	LOGOPERATION_ALL},
	{31,	4,	LOGOPERATION_ALL},
	{32,	4,	LOGOPERATION_ALL},
	{33,	5,	LOGOPERATION_ALL},
	{34,	5,	LOGOPERATION_ALL},
	{35,	5,	LOGOPERATION_ALL},	// SNTP 自動時計更新機能による時計セット
	{36,	5,	LOGOPERATION_ALL},	// SNTP 同期時刻変更
	{40,	4,	LOGOPERATION_ALL},
	{41,	4,	LOGOPERATION_ALL},
	{42,	4,	LOGOPERATION_ALL},	// フラップ下降
	{43,	4,	LOGOPERATION_ALL},	// フラップ上昇
	{44,	4,	LOGOPERATION_ALL},	// 全ロック装置開
	{45,	4,	LOGOPERATION_ALL},	// 全ロック装置閉
	{46,	4,	LOGOPERATION_ALL},	// 全フラップ下降
	{47,	4,	LOGOPERATION_ALL},	// 全フラップ上昇
	{48,	0,	LOGOPERATION_ALL},	// 暗証番号消去
	{51,	5,	LOGOPERATION_ALL},	// 減額精算
	{52,	5,	LOGOPERATION_ALL},	// 振替精算
	{53,	4,	LOGOPERATION_ALL},	// 受付券発行
	{54,	4,	LOGOPERATION_ALL},	// 領収書再発行
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：振替精算
	{55,	5,	LOGOPERATION_ALL},	// 領収書再発行
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：振替精算
	{84,	4,	LOGOPERATION_ALL},		// 遠隔ダウンロード要求結果
	{85,	4,	LOGOPERATION_ALL},		// Rism接続切断要求
	{86,	4,	LOGOPERATION_ALL},		// 遠隔ダウンロード終了
	{87,	4,	LOGOPERATION_ALL},		// プログラム更新終了
	{88,	4,	LOGOPERATION_ALL},		// 遠隔ダウンロード・プログラム更新結果アップロード
	{89,	4,	LOGOPERATION_ALL},		// Rism接続復旧要求
	{90,	4,	LOGOPERATION_ALL},
	{91,	4,	LOGOPERATION_ALL},
	{92,	4,	LOGOPERATION_ALL},		// Ｗカード認証テーブル更新
	{93,	4,	LOGOPERATION_ALL},		// 法人カード認証テーブル更新
	{94,	4,	LOGOPERATION_ALL},		// Ｔカード基本ポイントレート更新
	{95,	4,	LOGOPERATION_ALL},		// Ｔカード期間ポイントレート更新
	{96,	4,	LOGOPERATION_ALL},		// Ｔカード利用可否更新
	{97,	4,	LOGOPERATION_ALL},		// 無効係員データ登録上限
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind81[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{3,		4,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorTbl c_OpeMonitorTbl[] = {
	{0,		_countof(c_OpeMonitorKind00),	c_OpeMonitorKind00},
	{1,		_countof(c_OpeMonitorKind01),	c_OpeMonitorKind01},
	{11,	_countof(c_OpeMonitorKind11),	c_OpeMonitorKind11},
	{12,	_countof(c_OpeMonitorKind12),	c_OpeMonitorKind12},
	{13,	_countof(c_OpeMonitorKind13),	c_OpeMonitorKind13},
	{15,	_countof(c_OpeMonitorKind15),	c_OpeMonitorKind15},
	{16,	_countof(c_OpeMonitorKind16),	c_OpeMonitorKind16},
	{17,	_countof(c_OpeMonitorKind17),	c_OpeMonitorKind17},
	{18,	_countof(c_OpeMonitorKind18),	c_OpeMonitorKind18},
	{20,	_countof(c_OpeMonitorKind20),	c_OpeMonitorKind20},
	{23,	_countof(c_OpeMonitorKind23),	c_OpeMonitorKind23},
	{25,	_countof(c_OpeMonitorKind25),	c_OpeMonitorKind25},
	{80,	_countof(c_OpeMonitorKind80),	c_OpeMonitorKind80},
	{81,	_countof(c_OpeMonitorKind81),	c_OpeMonitorKind81},
};

/* モニタログ制御テーブル */
#define	_MONCODE(n)	(n%100)
static const t_NTNET_MonitorCtrl c_MonitorKind00[] = {
	{_MONCODE(OPMON_RSETUP_REFOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_REFNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_PREOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_PRENG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_RCVOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_RCVNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_EXEOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_EXENG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CHGOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CHGNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CANOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CANNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_NOREQ),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
};
static const t_NTNET_MonitorCtrl c_MonitorKind02[] = {
	{50,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},
// MH321800(S) D.Inaba ICクレジット対応
	{52,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},	// 電子マネー引去り結果電文受信
	{53,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},	// クレジット決済結果電文受信
// MH321800(E) D.Inaba ICクレジット対応
};

static const t_NTNET_MonitorCtrl c_MonitorKind01[] = {
	{1,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{4,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{5,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{6,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{7,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{8,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{13,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{14,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{21,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{22,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{23,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{24,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{25,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{26,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{27,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{28,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{45,	5,	(LOGOPERATION_ALL)},		// カード詰まり発生
	{46,	5,	(LOGOPERATION_ALL)},		// カード詰まり解除
	{50,	5,	(LOGOPERATION_ALL)},		// 初回通信開始
	{51,	5,	(LOGOPERATION_ALL)},		// 初回通信終了
	{52,	5,	(LOGOPERATION_ALL)},		// 手動Ｔ合計によるセンター通信開始
	{53,	5,	(LOGOPERATION_ALL)},		// 自動Ｔ合計によるセンター通信開始
	{54,	5,	(LOGOPERATION_ALL)},		// 手動センター通信開始（メンテ操作）
	{55,	5,	(LOGOPERATION_ALL)},		// 自動センター通信開始
	{56,	5,	(LOGOPERATION_ALL)},		// センター通信終了
	{57,	5,	(LOGOPERATION_ALL)},		// リーダー相互認証鍵更新
// MH810103(s) 電子マネー対応 時刻自動補正のモニタ修正
//	{70,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},		// 時刻自動補正
	{70,	5,	(LOGOPERATION_ALL)},		// 時刻自動補正
// MH810103(e) 電子マネー対応 時刻自動補正のモニタ修正
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
	{71,	5,	(LOGOPERATION_ALL)},		// 集計クリア異常(①印字集計のヘッダーと前回集計のヘッダーの比較)
	{72,	5,	(LOGOPERATION_ALL)},		// 集計クリア異常(②現在集計の追番、今回集計日時、前回集計日時と 印字集計の追番、今回集計日時、前回集計日時の比較)
	{73,	5,	(LOGOPERATION_ALL)},		// 集計クリア異常(③現在集計項目の値と印字集計項目の値の比較)
	{74,	5,	(LOGOPERATION_ALL)},		// 集計クリア異常(④現在集計の今回集計日時が0クリアされているか確認)
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
// MH321800(S) D.Inaba ICクレジット対応
	{75,	5,	(LOGOPERATION_ALL)},		// 電子マネー処理未了発生
	{76,	5,	(LOGOPERATION_ALL)},		// 電子マネー処理未了タイムアウト
// MH321800(E) D.Inaba ICクレジット対応	
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
	{77,	5,	(LOGOPERATION_ALL)},		// 釣銭管理クリア異常(④現在集計の今回集計日時が0クリアされているか確認)
	{78,	5,	(LOGOPERATION_ALL)},		// 釣銭管理クリア異常(⑤現在の追番、係員№、機械№が0クリア去れている場合はクリアキャンセル)
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
	{85,	5,	(LOGOPERATION_ALL)},		// クレジット　コマンド送信要因
	{86,	5,	(LOGOPERATION_ALL)},		// クレジット　コマンド応答結果
	{87,	5,	(LOGOPERATION_ALL)},		// クレジット　コマンドタイムアウト

};

static const t_NTNET_MonitorCtrl c_MonitorKind05[] = {
	{4,		5,	LOGOPERATION_ALL},		// ラグタイム延長
	{5,		4,	LOGOPERATION_ALL},
	{6,		4,	LOGOPERATION_ALL},
	{10,	4,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{15,	4,	LOGOPERATION_ALL},
	{16,	4,	LOGOPERATION_ALL},
	{17,	4,	LOGOPERATION_ALL},
	{18,	4,	LOGOPERATION_ALL},
	{20,	4,	LOGOPERATION_ALL},		// 入庫
	{21,	4,	LOGOPERATION_ALL},		// 出庫
};
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
static const t_NTNET_MonitorCtrl c_MonitorKind06[] = {
	{1,		5,	LOGOPERATION_ALL},		// 
};
#endif
// 調査用 (e)

static const t_NTNET_MonitorTbl c_MonitorTbl[] = {
	{0,		_countof(c_MonitorKind00),	c_MonitorKind00},
	{1,		_countof(c_MonitorKind01),	c_MonitorKind01},
	{2,		_countof(c_MonitorKind02),	c_MonitorKind02},
	{5,		_countof(c_MonitorKind05),	c_MonitorKind05},
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
	{99,	_countof(c_MonitorKind06),	c_MonitorKind06},
#endif
// 調査用 (e)
};

static const t_NTNET_MonitorCtrl	*getMonitorInfo(uchar kind, uchar code, const t_NTNET_MonitorTbl *tbl, size_t tbl_cnt);
#define	_getOpeLogInfo(kind, code)	getMonitorInfo(kind, code, c_OpeMonitorTbl, _countof(c_OpeMonitorTbl))
#define	_getMonLogInfo(kind, code)	getMonitorInfo(kind, code, c_MonitorTbl, _countof(c_MonitorTbl))


static void	Log_fusei_mk( void );
ushort	Ope_Log_GetLogMaxCount( short id );

#define	RAU_NTBUF_NEARFULL_COUNT			6

#define	TOTALLOG_NTDATA_COUNT				8	// 集計ログから作成するNT-NETデータ種類(遠隔Phase2は6種類)
void	Log_SetTotalLogNtSeqNo(short Lno, SYUKEI* pTotalLog);

/*[]----------------------------------------------------------------------[]*/
/*| log登録                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_regist( Lno )                                       |*/
/*| PARAMETER    : ﾛｸﾞ№                                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void Log_regist( short Lno )
{
	T_FrmReceipt	ReceiptPreqData;	// ＜領収証印字要求＞				ﾒｯｾｰｼﾞﾃﾞｰﾀ
	T_FrmFusKyo		FusKyoPreqData;		// ＜不正・強制出庫情報印字要求＞	ﾒｯｾｰｼﾞﾃﾞｰﾀ
	T_FrmTeiFukJou	TeiFukJouPreqData;	// ＜停復電情報印字要求＞			ﾒｯｾｰｼﾞﾃﾞｰﾀ
	T_FrmOpeJou		OpeJouPreqData;		// ＜操作情報印字要求＞				ﾒｯｾｰｼﾞﾃﾞｰﾀ
	uchar			jnl_pri = OFF;		// ｼﾞｬｰﾅﾙ印字有無
	T_FrmMonJou		MonJouPreqData;		// ＜ﾓﾆﾀ情報印字要求＞				ﾒｯｾｰｼﾞﾃﾞｰﾀ
	T_FrmRmonJou	RmonJouPreqData;	// ＜遠隔監視情報印字要求＞			ﾒｯｾｰｼﾞﾃﾞｰﾀ
	const t_NTNET_MonitorCtrl	*MonCtrl;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	ushort SyashitsuNum;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	ushort outkind;
	ushort			logCount;
	ushort			i;
// MH810105(S) MH364301 インボイス対応
	long			jnl_pri_jadge;		// 17-0001の値の一時保存
// MH810105(E) MH364301 インボイス対応

	switch( Lno ){
		case LOG_PAYMENT:	/* 個別精算情報 */
			outkind = 0;
			if( ryo_buf.pkiti != 0xffff ){												// 相互精算でない
				if( (ryo_buf.svs_tim == 1)&&											// サービスタイム中
					(FLAPDT.flp_data[ryo_buf.pkiti-1].lag_to_in.BIT.LAGIN != ON )){		// ラグタイムアップ後以外
					outkind = 99;
				}
			}else{																		// 相互精算のとき
				if( (ryo_buf.svs_tim == 1)&&											// サービスタイム中
					(LOCKMULTI.lock_mlt.lag_to_in.BIT.LAGIN != ON )){					// ラグタイムアップ後以外
					outkind = 99;
				}
			}
// MH810100(S) K.Onodera  2020/02/20 #3912 車番チケットレス(未精算出庫対応)
			if( OpeNtnetAddedInfo.PayMethod == 13 ){	// 後日精算
				outkind = 43;	// 未精算出庫車両精算(後払い)
			}
// MH810100(E) K.Onodera  2020/02/20 #3912 車番チケットレス(未精算出庫対応)
			PayData.OutKind = (uchar)outkind;
			NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
			SetAddPayData();
// GG129002(S) ゲート式車番チケットレスシステム対応（領収証が途中でカットされる現象の対策）
//// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//			if( OPECTL.Ope_mod == 3 ){
//				pri_ryo_stock(R_PRI);
//			}
//// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129002(E) ゲート式車番チケットレスシステム対応（領収証が途中でカットされる現象の対策）
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// 精算データ蓄積
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
			EcAlarmLog_RepayLogRegist(&PayData);
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
			if( PrnJnlCheck() == ON ){
				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
// MH810105(S) MH364301 インボイス対応
//				switch(CPrmSS[S_RTP][1]){	// ｼﾞｬｰﾅﾙ印字条件？
				jnl_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );			// 17-0001の値を変数に保存

				jnl_pri_jadge = invoice_prt_param_check( jnl_pri_jadge );	// インボイス時パラメータ変換

				switch(jnl_pri_jadge){	// ｼﾞｬｰﾅﾙ印字条件？
// MH810105(E) MH364301 インボイス対応
					case	0:
					case	1:
					case	2:
						// ｼﾞｬｰﾅﾙは０円時も記録
						jnl_pri = ON;
						break;
					case	3:
					case	4:
					case	5:
						// ｼﾞｬｰﾅﾙは駐車料金０円時以外記録
						if( PayData.WPrice != 0 ){
							//	駐車料金≠０
							jnl_pri = ON;
						}
						break;

					case	6:
					case	7:
					case	8:
						// ｼﾞｬｰﾅﾙは現金領収０円時以外記録
						if( PayData.WTotalPrice != 0 ){
							//	現金領収額≠０
							jnl_pri = ON;
						}
						break;
					case	9:
					case   10:
					case   11:
// MH321800(S) Y.Tanizaki ICクレジット対応
//						if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
						if( PayData.WTotalPrice != 0 ||
							SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
							EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
// MH321800(E) Y.Tanizaki ICクレジット対応
							//	現金領収額≠０ OR 電子媒体による精算が行われた場合
							jnl_pri = ON;
						}
						break;
// GG129001(S) 領収証プリント設定の読み替え対応
					case   12:
					case   13:
					case   14:
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
//						if(Billingcalculation(&PayData) != 0){
						if(PayData.WTaxPrice != 0){
// GG129001(E) データ保管サービス対応（課税対象額をセットする）
							jnl_pri = ON;
						}
						break;
// GG129001(E) 領収証プリント設定の読み替え対応
					default:
						jnl_pri = OFF;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)

				}

// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//				if( jnl_pri == ON && ryo_buf.ryos_fg == 0){
				if( jnl_pri == ON ){
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
					//	ｼﾞｬｰﾅﾙ印字条件を満たす場合
					ReceiptPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
// MH810104 GG119201(S) 電子ジャーナル対応
//					Pri_Work[1].Receipt_data = PayData;
//					ReceiptPreqData.prn_data = &Pri_Work[1].Receipt_data;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
					ReceiptPreqData.prn_data = &Cancel_pri_work;			// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
// MH810104 GG119201(E) 電子ジャーナル対応
					ReceiptPreqData.reprint = OFF;												// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
					if( IS_INVOICE ){															// インボイス
						if( OPECTL.Ope_mod == 3 && OPECTL.f_DelayRyoIsu == 0 ){					// 精算完了、ジャーナル印字未実施
							// 精算完了時ryo_isu処理前にkan_syuu(Log_regist)を処理した場合、OPECTL.f_DelayRyoIsuが立たないため
							// インボイス設定あり、ジャーナル印字未実施であればジャーナル印字完了待ちとする
							OPECTL.f_DelayRyoIsu = 1;
						}
					}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
				}
			}

// MH810100(S) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, outkind );	// 不正出庫情報あれば消す
// MH810100(E) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			break;
		case LOG_PAYSTOP:	/* 精算中止情報 */
			SetAddPayData();
			memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// 精算データ蓄積
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
			}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
			if( PrnJnlCheck() == ON ){
				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
				ReceiptPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
				ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
				ReceiptPreqData.reprint = OFF;												// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
				queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
			}

// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
//			IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
			break;
// MH810105(S) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
		case LOG_PAYSTOP_FU:												// 復電時の中止データ
			SetAddPayData();
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
			// 割引についてはここでクリアしておく（8021は、NTNET_Edit_Data56_rXXで入れられる）
			memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ	*/
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
			memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
			// ParkingWebに送信する
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// 精算データ蓄積
			if( PrnJnlCheck() == ON ){
				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
				ReceiptPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
				ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
				ReceiptPreqData.reprint = OFF;												// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
				queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
			}
			break;
// MH810105(E) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
		case LOG_ABNORMAL:	/* 不正・強制出庫情報 */
			Log_fusei_mk();																	// Make Data
			// 不正・強制出庫ログは1件印字前に次のデータが発生する可能性があるため印字バッファとして用意する必要有
			if( FLP_LOG_DAT.Flp_wtp < 0 || FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
				FLP_LOG_DAT.Flp_wtp = 0;
			}
			memcpy( &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], &logwork.flp, sizeof( flp_log ) );
			Log_Write(eLOG_ABNORMAL, &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], TRUE);// 不正・強制出庫ログ登録
			if( PrnJnlCheck() == ON ){
				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
				FusKyoPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
				FusKyoPreqData.prn_data = &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp];	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
				queset( PRNTCBNO, PREQ_FUSKYO_JOU, sizeof(T_FrmFusKyo), &FusKyoPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
			}
			if(logwork.flp.Lok_inf == 0){
				IoLog_write(IOLOG_EVNT_FORCE_FIN, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
				CountUp( KIYOUSEI_COUNT );													//強制出庫追い番
				PayInfo_Class = 100;										// 精算情報データ用処理区分を参照する
			}else{
				IoLog_write(IOLOG_EVNT_OUT_ILLEGAL, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
				CountUp( FUSEI_COUNT );														//不正出庫追い番
				PayInfo_Class = 101;										// 精算情報データ用処理区分を参照する
			}
			if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
			}
			FLP_LOG_DAT.Flp_wtp++;															// ﾗｲﾄﾎﾟｲﾝﾀ +1
			if( FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
				FLP_LOG_DAT.Flp_wtp = 0;
			}
			break;
		case LOG_TTOTAL:	/* Ｔ合計情報 */
			Log_SetTotalLogNtSeqNo(LOG_TTOTAL, &TSYU_LOG_WK);											// シーケンシャルNo.の設定
			Log_Write(eLOG_TTOTAL, &TSYU_LOG_WK, TRUE);										// Ｔ合計ログ登録
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// Log_Write()/Log_Write_Pon()内でeLOG_TTOTALの場合はac_flg.syusyu=15とし、更にeLOG_LCKTTLの登録ではac_flg.syusyu=19
// としているが、ここででac_flg.syusyu=19としても、復電処理ではac_flg.syusyu=15/19共に何もせずにac_flg.syusyu=16にす
// ることになるので、ここでは単純に車室毎集計ログ登録をスキップする
//-			Log_Write(eLOG_LCKTTL, &LCKT_LOG_WK, TRUE);										// 車室毎集計ログ(31車室以降)登録
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			break;
		case LOG_GTTOTAL:	/* ＧＴ合計情報 */
			Log_SetTotalLogNtSeqNo(LOG_GTTOTAL, &skyprn);									// シーケンシャルNo.の設定
			Log_Write(eLOG_GTTOTAL, &skyprn, TRUE);											// ＧＴ合計ログ登録
			break;
		case LOG_MONEYMANAGE:	/* 金銭管理情報 */
			// NT-NETでは最新の１件のみを登録する為、登録済みデータを空読みする
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// 金銭管理データのシーケンシャルNo.の設定
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// 金銭管理ログ登録
			turi_kan.SeqNo = GetNtDataSeqNo();											// 釣銭管理データのシーケンシャルNo.の設定
			Log_Write(eLOG_MNYMNG_SRAM, &turi_kan, FALSE);								// 金銭管理ログ登録(SRAM)
			break;
		case LOG_COINBOX:	/* ｺｲﾝ金庫集計情報 */
			coin_syu.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定
			Log_Write(eLOG_COINBOX, &coin_syu, FALSE);									// コイン金庫集計ログ登録
			Make_Log_MnyMng( 10 );														// 金銭管理ログデータ作成
			// NT-NETでは最新の１件のみを登録する為、登録済みデータを空読みする
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// 金銭管理データのシーケンシャルNo.の設定
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// 金銭管理ログ登録
			break;
		case LOG_NOTEBOX:	/* 紙幣金庫集計情報 */
			note_syu.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定
			Log_Write(eLOG_NOTEBOX, &note_syu, FALSE);									// 紙幣金庫集計ログ登録
			Make_Log_MnyMng( 11 );														// 金銭管理ログデータ作成
			// NT-NETでは最新の１件のみを登録する為、登録済みデータを空読みする
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// 金銭管理データのシーケンシャルNo.の設定
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// 金銭管理ログ登録
			break;
		case LOG_POWERON:	/* 停復電情報 */
			memcpy(&Pon_log_work.Pdw_Date, &clk_save, sizeof(date_time_rec));			// 停電時刻登録
			memcpy( &Pon_log_work.Pon_Date,
					&CLK_REC, sizeof( date_time_rec ) );				// 復電時刻登録

			Pon_log_work.Pon_nmi = (!FLAGPT.nmi_flg);					// nmiﾌﾗｸﾞ登録 
			Pon_log_work.Pon_kind = (unsigned char)(( StartingFunctionKey == 1 ) ? 3 :
																					( StartingFunctionKey == 2 ) ? 1 :
																					( StartingFunctionKey == 3 ) ? 2 :
																								StartingFunctionKey	);	/* nmi種類登録 */
			// 共通ﾊﾟﾗﾒｰﾀ FlashROM→SRAMを行った情報をset（Pon_kindのb7を使用：1=行った）
			if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){	// FLASH⇒RAMへのパラメータデータ転送
				Pon_log_work.Pon_kind |= 0x80;
			}
			Log_Write(eLOG_POWERON, &Pon_log_work, FALSE);									// 停復電ログ登録
			if( PrnJnlCheck() == ON ){
				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
				TeiFukJouPreqData.prn_kind = J_PRI;														// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
				memcpy(&TeiFukJouPreqData.Ponlog.Pdw_Date, &clk_save, sizeof(date_time_rec));			// 停電時刻登録
				memcpy( &TeiFukJouPreqData.Ponlog.Pon_Date, &CLK_REC, sizeof( date_time_rec ) );		// 復電時刻登録
				TeiFukJouPreqData.Ponlog.Pon_nmi = (!FLAGPT.nmi_flg);									// nmiﾌﾗｸﾞ登録
				TeiFukJouPreqData.Ponlog.Pon_kind = (unsigned char)(( StartingFunctionKey == 1 ) ? 3 :
																	( StartingFunctionKey == 2 ) ? 1 :
																	( StartingFunctionKey == 3 ) ? 2 :
																				StartingFunctionKey	);	/* nmi種類登録 */
				// 共通ﾊﾟﾗﾒｰﾀ FlashROM→SRAMを行った情報をset（Pon_kindのb7を使用：1=行った）
				if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){	// FLASH⇒RAMへのパラメータデータ転送
					TeiFukJouPreqData.Ponlog.Pon_kind |= 0x80;
				}
				queset( PRNTCBNO, PREQ_TEIFUK_JOU, sizeof(T_FrmTeiFukJou), &TeiFukJouPreqData );		// 印字要求ﾒｯｾｰｼﾞ登録
			}
			break;
		case LOG_ERROR:	/* ｴﾗｰ情報 */
			Err_work.ErrSeqNo = GetNtDataSeqNo();									// シーケンシャルNo.の設定
			Log_Write(eLOG_ERROR, &Err_work, TRUE);									// エラーログ登録
			break;
		case LOG_ALARM:	/* ｱﾗｰﾑ情報 */
			Arm_work.ArmSeqNo = GetNtDataSeqNo();									// シーケンシャルNo.の設定
			Log_Write(eLOG_ALARM, &Arm_work, TRUE);									// アラームログ登録
			break;
		case LOG_OPERATE:	/* 操作情報 */
			MonCtrl = _getOpeLogInfo(OPE_LOG_DAT.Ope_work.OpeKind, OPE_LOG_DAT.Ope_work.OpeCode);
			if (MonCtrl != NULL) {
				OPE_LOG_DAT.Ope_work.OpeLevel = MonCtrl->Level;
				// 操作ログは1件印字前に次のデータが発生する可能性があるため印字バッファとして用意する必要有
				if( OPE_LOG_DAT.Ope_wtp < 0 || OPE_LOG_DAT.Ope_wtp >= OPE_LOG_CNT ){
					OPE_LOG_DAT.Ope_wtp = 0;
				}
				memcpy(&OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp], &OPE_LOG_DAT.Ope_work, sizeof(Ope_log));
				if (MonCtrl->Operation & LOGOPERATION_RAM) {							// コードアトリビュートチェック
					// RAMロギング実行
					OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp].OpeSeqNo = GetNtDataSeqNo();		// シーケンシャルNo.の設定
					Log_Write(eLOG_OPERATE, &OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp], TRUE);	// 操作ログ登録
				}
				if (MonCtrl->Operation & LOGOPERATION_PRI) {							// コードアトリビュートチェック
					if (MonCtrl->Level >= (uchar)prm_get(COM_PRM, S_PRN, 18, 1, 1)) {	// レベル設定チェック
						if (PrnJnlCheck() == ON) {										// ジャーナル印字実行
							OpeJouPreqData.prn_kind = J_PRI;
							OpeJouPreqData.prn_data = &OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp];
							queset(PRNTCBNO, PREQ_OPE_JOU, sizeof(T_FrmOpeJou), &OpeJouPreqData);
						}
					}
					// 以下は印字用バッファの更新
					OPE_LOG_DAT.Ope_wtp++;
					if (OPE_LOG_DAT.Ope_wtp >= OPE_LOG_CNT) {
						OPE_LOG_DAT.Ope_wtp = 0;
					}
				}
			}
			else {
				/* 未定義コード */
			}
			break;
		case LOG_MONITOR:
			MonCtrl = _getMonLogInfo(MON_LOG_DAT.Mon_work.MonKind, MON_LOG_DAT.Mon_work.MonCode);
			if (MonCtrl != NULL) {
				MON_LOG_DAT.Mon_work.MonLevel = MonCtrl->Level;
				// モニタは1件印字前に次のデータが発生する可能性があるため印字バッファとして用意する必要有
				if( MON_LOG_DAT.Mon_wtp < 0 || MON_LOG_DAT.Mon_wtp >= MON_LOG_CNT ){
					MON_LOG_DAT.Mon_wtp = 0;
				}
				memcpy(&MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp], &MON_LOG_DAT.Mon_work, sizeof(Mon_log));
				if (MonCtrl->Operation & LOGOPERATION_RAM) {							// コードアトリビュートチェック
					// RAMロギング実行
					MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp].MonSeqNo = GetNtDataSeqNo();		// シーケンシャルNo.の設定
					Log_Write(eLOG_MONITOR, &MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp], TRUE);	// モニタログ登録
				}
				if (MonCtrl->Operation & LOGOPERATION_PRI) {							// コードアトリビュートチェック
					if(MON_LOG_DAT.Mon_work.MonKind == 1 &&
					  (MON_LOG_DAT.Mon_work.MonCode >= 85 && MON_LOG_DAT.Mon_work.MonCode <= 87) &&
					  0 == prm_get(COM_PRM, S_CRE, 66, 1, 1)) {
					  	// クレジット関連モニタで38-0066=0なら印字しない
					  	jnl_pri = OFF;
					}
					else {
					  	jnl_pri = ON;
					}
					if (MonCtrl->Level >= (uchar)prm_get(COM_PRM, S_PRN, 18, 1, 2) && jnl_pri == ON) {	// レベル設定チェック
						if (PrnJnlCheck() == ON) {										// ジャーナル印字実行
							MonJouPreqData.prn_kind = J_PRI;
							MonJouPreqData.prn_data = &MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp];
							queset(PRNTCBNO, PREQ_MON_JOU, sizeof(T_FrmMonJou), &MonJouPreqData);
						}
					}
					MON_LOG_DAT.Mon_wtp++;
					if (MON_LOG_DAT.Mon_wtp >= MON_LOG_CNT) {
						MON_LOG_DAT.Mon_wtp = 0;
					}
				}
			}
			else {
				/* 未定義コード */
			}
			break;
		case LOG_CREUSE:	/* ｸﾚｼﾞｯﾄ利用明細 */
			memcpy( &meisai_work.PayTime, &PayData.TOutTime, sizeof(meisai_work.PayTime));					// 精算日時
			memcpy( &meisai_work.card_name, &PayData.credit.card_name, sizeof(meisai_work.card_name));		// カード名
			memcpy( &meisai_work.card_no, &PayData.credit.card_no, sizeof(meisai_work.card_no));			// カード番号
			meisai_work.slip_no = PayData.credit.slip_no;													// 伝票番号
			memcpy(meisai_work.AppNoChar, PayData.credit.AppNoChar, sizeof(meisai_work.AppNoChar));			// 承認番号
			memcpy(meisai_work.shop_account_no, PayData.credit.ShopAccountNo,
														sizeof(meisai_work.shop_account_no));				// ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号
			meisai_work.pay_ryo = PayData.credit.pay_ryo;													// 利用金額

			if(PayData.credit.cre_type == CREDIT_CARD) {													// クレジットカード利用
				Log_Write(eLOG_CREUSE, &meisai_work, TRUE);													// クレジット利用明細ログ登録
			}
			else if(PayData.credit.cre_type == CREDIT_HOJIN) {												// 法人カード利用
				Log_Write(eLOG_HOJIN_USE, &meisai_work, TRUE);												// 法人カード利用明細ログ登録
			}
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case	LOG_EDYARM:			// Edyアラーム取引ログ
//			Edy_Arm_log_work.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			Edy_Arm_log_work.TD_Kikai_no = (uchar)PayData.Kikai_no;
//			if( !OPECTL.multi_lk )									// ﾏﾙﾁ精算問合せ中以外?
//				SyashitsuNum = OPECTL.Pr_LokNo;						// ロック装置No
//			else
//				SyashitsuNum = (ushort)(LOCKMULTI.LockNo%10000);	// ロック装置No
//
//			Edy_Arm_log_work.TicketNum = 
//			(ushort)(( LockInfo[(SyashitsuNum-1)].area * 10000L ) + LockInfo[(SyashitsuNum-1)].posi );
//
//			Edy_Arm_log_work.syubet = ryo_buf.tik_syu;
//			Edy_Arm_log_work.teiki = PayData.teiki;
//			memcpy( &Edy_Arm_log_work.TOutTime, &CLK_REC, sizeof( date_time_rec ));
//			memcpy( Edy_Arm_log_work.Edy_CardNo,
//					Edy_Settlement_Res.Edy_CardNo,
//					sizeof( Edy_Settlement_Res.Edy_CardNo ));
//			Edy_Arm_log_work.Pay_Ryo = Edy_Settlement_Res.Pay_Ryo;
//			Edy_Arm_log_work.Pay_Befor = Edy_Settlement_Res.Pay_Befor;
//			Edy_Arm_log_work.Pay_After = Edy_Settlement_Res.Pay_After;
//			Edy_Arm_log_work.Edy_Deal_num = Edy_Settlement_Res.Edy_Deal_num;
//			Edy_Arm_log_work.Card_Deal_num = Edy_Settlement_Res.Card_Deal_num;
//			memcpy( Edy_Arm_log_work.High_Terminal_ID,
//					Edy_Settlement_Res.High_Terminal_ID,
//					sizeof( Edy_Settlement_Res.High_Terminal_ID ));
//
//			Log_Write(eLOG_EDYARM, &Edy_Arm_log_work, FALSE);
//			break;
//
//		case	LOG_EDYSHIME:			// Edy締め記録ログ
//			Log_Write(eLOG_EDYSHIME, &EDY_SHIME_LOG_NOW, FALSE);
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//		case	LOG_ENTER:				// 入庫ログ
//			Enter_data.CMN_DT.DT_54.SeqNo = GetNtDataSeqNo();							// シーケンシャルNo.の設定
//			Log_Write(eLOG_ENTER, &Enter_data, TRUE);
//			break;
		case	LOG_RTPAY:				// RT精算データログ
//			Enter_data.CMN_DT.DT_54.SeqNo = GetNtDataSeqNo();							// シーケンシャルNo.の設定
			Log_Write(eLOG_RTPAY, &RTPay_Data, TRUE);
			break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		case	LOG_RTRECEIPT:			// RT領収証データログ
			Log_Write(eLOG_RTRECEIPT, &RTReceipt_Data, TRUE);
			break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		case	LOG_PARKING:			// 駐車台数データ
			ParkCar_data.CMN_DT.DT_58.SeqNo = GetNtDataSeqNo();							// シーケンシャルNo.の設定
			Log_Write(eLOG_PARKING, &ParkCar_data, TRUE);
			break;
		case	LOG_LONGPARK:			// 長期駐車データ
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//			Log_Write(eLOG_LONGPARK, &LongParking_data, TRUE);
			Log_Write(eLOG_LONGPARK_PWEB, &LongParking_data_Pweb, TRUE);
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
			break;
		case	LOG_MONEYMANAGE_NT:		// 金銭管理データ(NT-NET)
			// NT-NETでは最新の１件のみを登録する為、登録済みデータを空読みする
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// シーケンシャルNo.の設定
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// 金銭管理ログ登録
			break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		case	LOG_RISMEVENT:			// 長期駐車データ
//			Log_Write(eLOG_RISMEVENT, &RismEvent_data, TRUE);
//			break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		case LOG_REMOTE_MONITOR:
			// 遠隔監視は1件印字前に次のデータが発生する可能性があるため印字バッファとして用意する必要有
			if( RMON_LOG_DAT.Rmon_wtp < 0 || RMON_LOG_DAT.Rmon_wtp >= RMON_LOG_CNT ){
				RMON_LOG_DAT.Rmon_wtp = 0;
			}
			memcpy(&RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp], &RMON_LOG_DAT.Rmon_work, sizeof(Rmon_log));
			RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp].RmonSeqNo = GetNtDataSeqNo();	// シーケンシャルNo.の設定
			Log_Write(eLOG_REMOTE_MONITOR, &RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp], TRUE);	// 遠隔監視ログ登録

			if (PrnJnlCheck() == ON) {														// ジャーナル印字実行
				RmonJouPreqData.prn_kind = J_PRI;
				RmonJouPreqData.prn_data = &RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp];
				queset(PRNTCBNO, PREQ_RMON_JOU, sizeof(T_FrmRmonJou), &RmonJouPreqData);
			}
			RMON_LOG_DAT.Rmon_wtp++;
			if (RMON_LOG_DAT.Rmon_wtp >= RMON_LOG_CNT) {
				RMON_LOG_DAT.Rmon_wtp = 0;
			}
			break;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
		// QR確定・取消データログ
		case LOG_DC_QR:
			Log_Write( eLOG_DC_QR, &DC_QR_work, TRUE );
			break;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		// レーンモニタデータログ
		case LOG_DC_LANE:
			Log_Write( eLOG_DC_LANE, &DC_LANE_work, TRUE );
			break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		default:
			break;
	}

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| 不正・強制出庫情報データ作成                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_fusei_mk( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2001-12-20                                              |*/
/*| UpDate       : 2004-06-18 T.Nakayama                                   |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static void Log_fusei_mk( void )
{
	ushort	num;

	logwork.flp.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// 機械No.

	logwork.flp.In_Time.Year = car_in_f.year;				/* 入庫年月日時分 	*/
	logwork.flp.In_Time.Mon  = car_in_f.mon;
	logwork.flp.In_Time.Day  = car_in_f.day;
	logwork.flp.In_Time.Hour = car_in_f.hour;
	logwork.flp.In_Time.Min  = car_in_f.min;
	logwork.flp.Date_Time.Year = car_ot_f.year;				/* 精算年月日時分 	*/
	logwork.flp.Date_Time.Mon  = car_ot_f.mon;
	logwork.flp.Date_Time.Day  = car_ot_f.day;
	logwork.flp.Date_Time.Hour = car_ot_f.hour;
	logwork.flp.Date_Time.Min  = car_ot_f.min;

	num = ryo_buf.pkiti - 1;
	logwork.flp.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	/* 駐車位置 		*/

	logwork.flp.Lok_No = num +1;

	if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 ){
		CountGet( KIYOUSEI_COUNT, &logwork.flp.count );
		logwork.flp.Lok_inf = 0;							/* 強制 			*/
		PayData_set_SK( ryo_buf.pkiti, 0, 0, 1, 0 );		// 不正・強制出庫用精算情報生成
		Log_Write(eLOG_PAYMENT, &PayData, TRUE);
	}
	else{					// 0：通常不正、2：修正不正(料金あり)、3：修正不正(料金なし)
		CountGet( FUSEI_COUNT, &logwork.flp.count );
		logwork.flp.Lok_inf = 1;							/* 不正 			*/
		if( fusei.fus_d[0].kyousei != 0 ){
			logwork.flp.Lok_inf = 2;						/* 修正精算による不正 */
		}
		PayData_set_SK( ryo_buf.pkiti, 0, 0, 3, fusei.fus_d[0].kyousei );			// 不正・強制出庫用精算情報生成
		Log_Write(eLOG_PAYMENT, &PayData, TRUE);
	}

	if( fusei.fus_d[0].kyousei == 3 ){
		logwork.flp.fusei_fee = 0;							/* 駐車料金 		*/
	}else{
		logwork.flp.fusei_fee = ryo_buf.dsp_ryo;			/* 駐車料金 		*/
	}
	logwork.flp.ryo_syu = ryo_buf.tik_syu;					/* 料金種別 		*/

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| Logｴﾘｱ初期化                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : log_init                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	log_init( void )
{
	short i, j;

	if( read_rotsw() == 4 ){	// ----- スーパーイニシャライズ(RSW=4)? -----
		memset(LOG_DAT, 0, sizeof(struct log_record)*eLOG_MAX);
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
//		memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
		for (i = 0; i < eLOG_MAX; i++) {
			for (j = 0; j < eLOG_TARGET_MAX; j++) {
				LOG_DAT[i].void_read[j] = 0;
				LOG_DAT[i].nearFull[j].NearFullMinimum = 0;
				LOG_DAT[i].nearFull[j].NearFullStatus = RAU_NEARFULL_CANCEL;
				LOG_DAT[i].nearFull[j].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			}
		}
	}
	memset( &SUICA_LOG_REC, 0, sizeof( struct suica_log_rec ) );
	memset( &SUICA_LOG_REC_FOR_ERR, 0, sizeof( struct	suica_log_rec ) );
	memset( &suica_rcv_que, 0, sizeof( t_SUICA_RCV_QUE ));
	memset(&OPE_LOG_DAT, 0, sizeof(OPE_LOG_DAT));
	memset(&MON_LOG_DAT, 0, sizeof(MON_LOG_DAT));
	memset(&FLP_LOG_DAT, 0, sizeof(FLP_LOG_DAT));
	memset(&Syuukei_sp, 0, sizeof(Syuukei_sp));
	memset(&RMON_LOG_DAT, 0, sizeof(RMON_LOG_DAT));
	memset(&fan_exe_time, 0, sizeof(fan_exe_time));

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ管理種別取得
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG種別
///	@return		strage	: LOGデータ管理種別(FALSE(=0):SRAM,TRUE(=1):SRAM+FlashROM)
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL Ope_Log_ManageKindGet(short Lno)
{
	// フラッシュに保存するときTRUE
	// フラッシュタスクに処理を依頼できるログがLOG_STRAGEN個なのでそれを使用する。
	if (Lno < LOG_STRAGEN) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG登録件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG種別
///	@return		ushort	: 登録数
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_TotalCountGet( short id )
{
	long				num;
	struct log_record	*lp = LOG_DAT+id;

	if (Ope_Log_ManageKindGet(id) == FALSE) {
	// SRAMのみ
		return (ushort)lp->count[0];
	}
	else {
	// フラッシュあり
		while(FLT_GetLogRecordNum(id, &num) != FLT_NORMAL) {
			taskchg( IDLETSKNO );
		}
		if (LogDatMax[id][0] <= RECODE_SIZE) {
		// SRAM一時保存データあり
			num += lp->count[lp->kind];
		}
		return (ushort)num;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		未読LOG登録件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	target	: 読み出し種別
///	@return		ushort	: 未読LOG登録件数
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_UnreadCountGet( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	num = 0;

	if( !Ope_Log_ManageKindGet( id ) ){		// SRAMのみ
		//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
		num = lp->f_unread[target];			// 未読データ有
	} else {// FROM/SRAM両方
		//  FROM/RAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			num = Ope_Log_TotalCountGet(id) - lp->unread[target];
		}
	}
	return num;
}

//[]----------------------------------------------------------------------[]
///	@brief		未読LOGを読取済とする
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	target	: 読み出し種別
///	@return		ushort	: 未読LOG登録件数
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_UnreadToRead( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;

	if( !Ope_Log_ManageKindGet( id ) ){		// SRAMのみ
		//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
		lp->unread[target] += lp->f_unread[target];
		lp->f_unread[target] = 0;
	} else {// FROM/SRAM両方
		//  FROM/RAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
		lp->unread[target] = Ope_Log_TotalCountGet(id);
	}
	lp->void_read[target] = 0;
	lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
	lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
	Log_CheckBufferFull(FALSE, id, target);
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief		ニアフルチェック
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	target	: 読み出し種別
///	@return		ushort	: ニアフル状態(0:通常 1:ニアフル)
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_CheckNearFull( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	ushort ret = 0;

	if( lp->nearFull[target].NearFullStatus == RAU_NEARFULL_NEARFULL ){
		if( lp->nearFull[target].NearFullStatusBefore != lp->nearFull[target].NearFullStatus ){
			// 現在値を前回値に書き込み、多重発生をなくす｡
			lp->nearFull[target].NearFullStatusBefore = lp->nearFull[target].NearFullStatus;
			ret = 1;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		ニアフル状態取得
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	target	: 読み出し種別
///	@return		ushort	: ニアフル状態(0:通常 1:ニアフル)
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetNearFull( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	
	return lp->nearFull[target].NearFullStatus;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG格納件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@return		ushort	: LOG格納可能件数(FROM+SRAM)
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetLogMaxCount( short id )
{
	ushort	sectorCount = 0;				// 1セクタ分のログ件数
	ushort	maxCount = 0;					// ログ最大件数
	
	sectorCount = LOG_SECORNUM(id);			// SRAMの最大ログ件数
	
	switch(id) {
	case eLOG_PAYMENT:						// 精算
		maxCount += sectorCount + (FLT_LOG0_SECTORMAX * sectorCount);
		break;
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	case eLOG_ENTER:						// 入庫
	case eLOG_RTPAY:						// RT精算データ
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
		maxCount += sectorCount + (FLT_LOG1_SECTORMAX * sectorCount);
		break;
	case eLOG_TTOTAL:						// 集計
		maxCount += sectorCount + (FLT_LOG2_SECTORMAX * sectorCount);
		break;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	case eLOG_LCKTTL:						// 車室毎集計
	case eLOG_RTRECEIPT:					// RT領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		maxCount += sectorCount + (FLT_LOG3_SECTORMAX * sectorCount);
		break;
	case eLOG_ERROR:						// エラー
		maxCount += sectorCount + (FLT_LOG4_SECTORMAX * sectorCount);
		break;
	case eLOG_ALARM:						// アラーム
		maxCount += sectorCount + (FLT_LOG5_SECTORMAX * sectorCount);
		break;
	case eLOG_OPERATE:						// 操作
		maxCount += sectorCount + (FLT_LOG6_SECTORMAX * sectorCount);
		break;
	case eLOG_MONITOR:						// モニタ
		maxCount += sectorCount + (FLT_LOG7_SECTORMAX * sectorCount);
		break;
	case eLOG_ABNORMAL:						// 不正強制出庫
		maxCount += sectorCount + (FLT_LOG8_SECTORMAX * sectorCount);
		break;
	case eLOG_MONEYMANAGE:					// 釣銭管理
		maxCount += sectorCount + (FLT_LOG9_SECTORMAX * sectorCount);
		break;
	case eLOG_PARKING:						// 駐車台数データ
		maxCount += sectorCount + (FLT_LOG10_SECTORMAX * sectorCount);
		break;
	case eLOG_NGLOG:						// 不正情報
		maxCount += sectorCount + (FLT_LOG11_SECTORMAX * sectorCount);
		break;
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	case eLOG_IOLOG:						// 入出庫履歴
	case eLOG_DC_QR:						// QR確定・取消データ
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
		maxCount += sectorCount + (FLT_LOG12_SECTORMAX * sectorCount);
		break;
	case eLOG_CREUSE:						// クレジット利用
		maxCount += sectorCount + (FLT_LOG13_SECTORMAX * sectorCount);
		break;
	case eLOG_iDUSE:						// ID利用
		maxCount += sectorCount + (FLT_LOG14_SECTORMAX * sectorCount);
		break;
	case eLOG_HOJIN_USE:					// 法人カード利用
		maxCount += sectorCount + (FLT_LOG15_SECTORMAX * sectorCount);
		break;
	case eLOG_REMOTE_SET:					// 遠隔料金設定
		maxCount += sectorCount + (FLT_LOG16_SECTORMAX * sectorCount);
		break;
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//	case eLOG_LONGPARK:						// 長期駐車
	case eLOG_LONGPARK_PWEB:				// 長期駐車(ParkingWeb用)
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
		maxCount += sectorCount + (FLT_LOG17_SECTORMAX * sectorCount);
		break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	case eLOG_RISMEVENT:					// RISMイベント
	case eLOG_DC_LANE:						// レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		maxCount += sectorCount + (FLT_LOG18_SECTORMAX * sectorCount);
		break;
	case eLOG_GTTOTAL:						// GT集計
		maxCount += sectorCount + (FLT_LOG19_SECTORMAX * sectorCount);
		break;
	case eLOG_REMOTE_MONITOR:
		maxCount += sectorCount + (FLT_LOG20_SECTORMAX * sectorCount);
		break;
	case eLOG_COINBOX:						// コイン金庫集計(ramのみ)
	case eLOG_NOTEBOX:						// 紙幣金庫集計(ramのみ)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	case eLOG_EDYARM:						// Edyアラーム(ramのみ)
//	case eLOG_EDYSHIME:						// Edy締め(ramのみ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	case eLOG_POWERON:						// 復電(ramのみ)
	case eLOG_MNYMNG_SRAM:					// 金銭管理(SRAM)
		maxCount = sectorCount;
		break;
	default:
		break;
	}
	
	return maxCount;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGニアフルデータ件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetNearFullCount( short id )
{
	ushort	cen_48_456;
	ushort	nearFullCount;

	cen_48_456 = prm_get(COM_PRM,S_NTN, 68, 3, 1);
	if (cen_48_456 > 100) {
		cen_48_456 = 100;
	}
	if (cen_48_456 <= 0) {
		cen_48_456 = 1;
	}
	nearFullCount = Ope_Log_GetLogMaxCount(id) * cen_48_456 / 100;
	if( (Ope_Log_GetLogMaxCount(id) * cen_48_456) % 100 ) {
		nearFullCount++;
	}
	if (nearFullCount <= 0) {
		nearFullCount = 1;
	}
	return nearFullCount;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ取得
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG種別
///	@param[in]	Index	: LOGインデックス番号
///	@param[out]	pSetBuf	: データ格納バッファ
///	@return		uchar	: 1=正常、0=エラー
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

// ログ書込み前後で件数が変わるので、一連のログを参照する場合は、その間にログ書き込みが
// 起こらないよう注意してください。

uchar Ope_Log_1DataGet( short id, ushort Index, void *pSetBuf )
{
	long				num, maximum;
	uchar				*dp;
	struct log_record	*lp = LOG_DAT+id;

	if (Ope_Log_ManageKindGet(id) == FALSE) {
	// SRAMのみの場合は、s2上でリングバッファになる
		maximum = lp->count[0];
		dp = lp->dat.s2;
		if( Index>=maximum ){	// Index error
			return 0;
		}
		memcpy(pSetBuf, dp+Index*LogDatMax[id][0], LogDatMax[id][0]);
		return 1;
	}

// フラッシュ処理から件数を取得（書き込み中なら待ち合わせる）
	while( FLT_GetLogRecordNum(id, &num) != FLT_NORMAL ){
		taskchg( IDLETSKNO );
	}

	if( Index<num ){				// FlashROM内のﾃﾞｰﾀ要求
		if( FLT_TargetLogRead(id, Index, pSetBuf)!=FLT_NORMAL ){
			return 0;
		}
	}
	else{							// SRAM内のﾃﾞｰﾀ要求
		Index -= (ushort)num;
		if (LogDatMax[id][0] > RECODE_SIZE) {
		// SRAMが１面しか無い場合（集計データ、車室毎集計データ）、SRAMに保存できる件数は１になる。
		// 保存後すぐにフラッシュに書き込まれ完了待ちまで行うためSRAMデータは実質上存在しない。
			return 0;
		}

	// SRAMに２面ある場合、運用面側が有効で、待機面分はフラッシュに書き込み後破棄されて存在しない
		maximum = lp->count[lp->kind];
		dp = lp->dat.s1[lp->kind];
		if( Index>=maximum ){	// Index error
			return 0;
		}
		memcpy(pSetBuf, dp+Index*LogDatMax[id][0], LogDatMax[id][0]);
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ取得（読み出し種別）
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[out]	pSetBuf	: データ格納バッファ
///	@param[in]	Index	: 読み出し種別
///	@param[in]	Index	: リードポインタ加算フラグ
///	@return		uchar	: 1=正常、0=エラー
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Ope_Log_TargetDataGet( short id, void *pSetBuf, short target, BOOL update )
{
	uchar	ret = 0;
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;

	if( !Ope_Log_ManageKindGet( id ) ){// SRAMのみ
		//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
		if(lp->f_unread[target] > 0){// 未読データ有
			ret = Ope_Log_1DataGet(id, lp->unread[target], pSetBuf);
			if( ret==1 && update ){
				lp->unread[target]++;
				lp->f_unread[target]--;// 未読データ数減算
				if(lp->f_unread[target] <= lp->nearFull[target].NearFullMinimum){
					// ニアフル解除
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				max_idx = LOG_SECORNUM( id );						// 最大ログ数取得
				if( lp->unread[target] >= max_idx ){				// 未読レコード番号が最大ログ数を超えた？
					lp->unread[target] = 0;							// 未読レコード番号を０に戻す
				}
			}
		}
	} else {// FROM/SRAM両方
		//  FROM/RAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			ret = Ope_Log_1DataGet(id, lp->unread[target], pSetBuf);
			if( ret==1 && update ){
				lp->unread[target]++;
				if((Ope_Log_TotalCountGet(id) - lp->f_unread[target]) <= lp->nearFull[target].NearFullMinimum){
					// ニアフル解除
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
			}
		}
	}
	return ret;
}

// Ope_Log_TargetDataGet()では１回分の空読みのみで複数回数分の空読み
// を行う為に追加
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ空読み（読み出し種別）
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[out]	pSetBuf	: データ格納バッファ
///	@param[in]	Index	: 読み出し種別
///	@param[in]	Index	: 空読み開始フラグ(TRUE:初回空読み FALSE:次回以降空読み)
///	@return		uchar	: 0:エラー 1=正常
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Ope_Log_TargetDataVoidRead( short id, void *pSetBuf, short target, BOOL start )
{
	uchar	ret = 0;
	ushort	voidReadIndex;
	struct log_record	*lp = LOG_DAT+id;

	if(start == TRUE){
		lp->void_read[target] = 0;
		// 空読み開始以降に未送信が上書きされたかが対象となるので、開始時にクリアする
		lp->overWriteUnreadCount[target] = 0;
	}

	if( !Ope_Log_ManageKindGet( id ) ){// SRAMのみ
		//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
		if(lp->f_unread[target] > 0){// 未読データ有
			voidReadIndex = lp->unread[target] + lp->void_read[target];
			if(voidReadIndex >= LOG_SECORNUM(id)) {
				voidReadIndex -= (LOG_SECORNUM(id));
			}
			ret = Ope_Log_1DataGet(id, voidReadIndex, pSetBuf);
		}
	} else {// FROM/SRAM両方
		//  FROM/RAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
		if( Ope_Log_TotalCountGet(id) > (lp->unread[target] + lp->void_read[target]) ){
			ret = Ope_Log_1DataGet(id, lp->unread[target] + lp->void_read[target], pSetBuf);
		}
	}
	if(ret == 1){
		lp->void_read[target]++;
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ空読みを元に戻す
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	Index	: 読み出し種別
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Ope_Log_UndoTargetDataVoidRead( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	
	// 空読み状態であれば1つ戻す
	if(lp->void_read[target]) {
		--lp->void_read[target];
	}
}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// //[]----------------------------------------------------------------------[]
// ///	@brief		集計データLOGの追番と一致する車室毎集計データLOGを検索する
// //[]----------------------------------------------------------------------[]
// ///	@param[in]	syukei	: 集計データLOGポインタ
// ///	@param[in]	lckttl	: 車室毎集計データLOGポインタ
// ///	@return		uchar	: 1=検索OK、0=検索NG
// //[]----------------------------------------------------------------------[]
// ///	@author		y.iiduka
// ///	@date		Create	: 2012/02/07<br>
// ///				Update	: 
// //[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// uchar Ope_Log_ShashitsuLogGet( SYUKEI *syukei, LCKTTL_LOG *lckttl )
// {
// 	ushort	cnt_ttl;
// 	ushort	i;
// 	uchar	ret = 0;
// 
// 	cnt_ttl = Ope_Log_TotalCountGet( eLOG_LCKTTL );			// 車室毎集計データLOG総数取得
// 	for( i = 0; i < cnt_ttl; i++ ){							// 車室毎集計データLOG総数分検索
// 		if( Ope_Log_1DataGet( eLOG_LCKTTL, i, lckttl ) ){	// 車室毎集計データLOG１件取得
// 			if(( lckttl->Oiban.i == syukei->Oiban.i ) &&	// 追番＋今回集計日時が一致？
// 			   ( lckttl->Oiban.w == syukei->Oiban.w ) &&
// 			   ( memcmp( &lckttl->NowTime, &syukei->NowTime, sizeof( date_time_rec ) ) == 0 )){
// 				ret = 1;									// 検索OK
// 				break;
// 			}
// 		}
// 	}
// 
// 	return ret;
// }
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

//[]----------------------------------------------------------------------[]
///	@brief		LOGリードポインタ更新（読み出し種別）
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	Index	: 読み出し種別
///	@return
//[]----------------------------------------------------------------------[]
///	@author		S.Takahashi
///	@date		Create	: 2012/05/02<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Ope_Log_TargetReadPointerUpdate(short id, short target)
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;

	if( !Ope_Log_ManageKindGet( id ) ){// SRAMのみ
		//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
		if(lp->f_unread[target] > 0){// 未読データ有
			lp->unread[target]++;
			lp->f_unread[target]--;// 未読データ数減算
			max_idx = LOG_SECORNUM( id );						// 最大ログ数取得
			if( lp->unread[target] >= max_idx ){				// 未読レコード番号が最大ログ数を超えた？
				lp->unread[target] = 0;							// 未読レコード番号を０に戻す
			}
		}
	} else {// FROM/SRAM両方
		//  FROM/SRAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			lp->unread[target]++;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		空読みLOGのリードポインタ更新（読み出し種別）
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@param[in]	Index	: 読み出し種別
///	@return		空読みからリードポインタ更新までに削除された未送信データ数
//[]----------------------------------------------------------------------[]
///	@author		S.Takahashi
///	@date		Create	: 2012/05/02<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_TargetVoidReadPointerUpdate(short id, short target)
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;
	ushort	ret = 0;

	if(lp->void_read[target] != 0){
		if( !Ope_Log_ManageKindGet( id ) ){// SRAMのみ
			//  RAMのみにデータを持つLOGの場合、unreadはRAMエリアの先頭(s2[RECODE_SIZE2])からどの位置を読むかを示す
			if(lp->f_unread[target] > 0){// 未読データ有
				lp->unread[target] += lp->void_read[target];
				lp->f_unread[target] -= lp->void_read[target];		// 未読データ数減算
				max_idx = LOG_SECORNUM( id );						// 最大ログ数取得
				if( lp->unread[target] >= max_idx ){				// 未読レコード番号が最大ログ数を超えた？
					lp->unread[target] -= max_idx;	// 未読レコード番号を０から振り直す
				}
				if(lp->f_unread[target] <= lp->nearFull[target].NearFullMinimum){
					// ニアフル解除
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				Log_CheckBufferFull(FALSE, id, target);				// バッファフル解除チェック
			}
		} else {// FROM/SRAM両方
			//  FROM/SRAMの両方にデータを持つLOGの場合、unreadは総件数に対して何件リードしているかを示す
			if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
				if(lp->overWriteUnreadCount[target]) {				// 削除された未送信データあり
					lp->unread[target] += lp->void_read[target];
					ret = lp->overWriteUnreadCount[target];
					lp->overWriteUnreadCount[target] = 0;			// 削除データに対応したのでクリアする
				}
				else {
					lp->unread[target] += lp->void_read[target];
				}
			}
			if((Ope_Log_TotalCountGet(id) - lp->unread[target]) <= lp->nearFull[target].NearFullMinimum){
				// ニアフル解除
				lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
				lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			}
			Log_CheckBufferFull(FALSE, id, target);				// バッファフル解除チェック
		}
		lp->void_read[target] = 0;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータバッファフルチェック
//[]----------------------------------------------------------------------[]
///	@param[in]	occur	: TRUE:発生チェック FALSE:解除チェック
///	@param[in]	Lno		: LOG種別
///	@param[in]	target	: チェック対象読み出し種別
/////	@param[in]	strage	: SRAMフラグ
///	@return		void
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Log_CheckBufferFull(BOOL occur, short Lno, short target)
{
	ushort	cnt;
	ushort	maxcnt;
	ushort	errorCode;									// エラーコード
	struct	log_record*	lp;
	uchar	bufferfull = 0;								// 1:バッファフル発生
	uchar	bufferfullRel = 0;							// 1:バッファフル解除
	uchar*	pState = NULL;								// バッファ状態格納ポインタ
	t_NtBufState	*ntbufst;
	BOOL strage = Ope_Log_ManageKindGet(Lno);
	
	// 遠隔NT-NETのみ実装
	switch(target) {
	case eLOG_TARGET_REMOTE:							// 遠隔NT-NET
		if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2) {	// 遠隔NT-NET未使用
			return;										// チェックしない
		}
		break;
	default:
		return;											// チェックしない
	}

	ntbufst = (t_NtBufState*)NTBUF_GetBufState();
	switch(Lno) {
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	case eLOG_ENTER:									// 入庫
//		if( prm_get(COM_PRM, S_NTN, 61, 1, 6) == 1 ){
//			return;										// チェックしない
//		}
//		errorCode = ERR_RAU_ENTRY_BUFERFULL;
//		pState = &ntbufst->car_in;
//		break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	case eLOG_PAYMENT:									// 精算
		if( prm_get(COM_PRM, S_NTN, 61, 1, 5) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_PAYMENT_BUFERFULL;
		pState = &ntbufst->sale;
		break;
	case eLOG_TTOTAL:									// 集計
			if( prm_get(COM_PRM, S_NTN, 61, 1, 4) == 1 ){
				return;									// チェックしない
			}
		errorCode = ERR_RAU_TOTAL_BUFERFULL;
		pState = &ntbufst->ttotal;
		break;
	case eLOG_ERROR:									// エラー
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 3) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 3) == 9) ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_ERROR_BUFERFULL;
		break;
	case eLOG_ALARM:									// アラーム
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 2) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 4) == 9) ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_ALARM_BUFERFULL;
		break;
	case eLOG_MONITOR:									// モニタ
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 1) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 1) == 9) ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_MONITOR_BUFERFULL;
		break;
	case eLOG_OPERATE:									// 操作
		if( (prm_get(COM_PRM, S_NTN, 62, 1, 6) == 1)|| (prm_get(COM_PRM, S_NTN, 37, 1, 2) == 9) ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_OPE_MONITOR_BUFERFULL;
		break;
	case eLOG_COINBOX:									// コイン金庫集計(ramのみ)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 5) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_COIN_BUFERFULL;
		pState = &ntbufst->coin;
		break;
	case eLOG_NOTEBOX:									// 紙幣金庫集計(ramのみ)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 4) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_NOTE_BUFERFULL;
		pState = &ntbufst->note;
		break;
	case eLOG_PARKING:									// 駐車台数データ
		if( prm_get(COM_PRM, S_NTN, 62, 1, 3) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_PARK_CNT_BUFERFULL;
		break;
	case eLOG_MNYMNG_SRAM:								// 釣銭管理集計(ramのみ)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 2) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_TURI_BUFERFULL;
		break;
	case eLOG_GTTOTAL:									// GT集計
		if( prm_get(COM_PRM, S_NTN, 61, 1, 4) == 1 ){
			return;										// チェックしない
		}
		errorCode = ERR_RAU_GTTOTAL_BUFERFULL;
		break;
	case eLOG_REMOTE_MONITOR:
		// @todo 送信マスク設定が追加されたら、設定を参照すること
		errorCode = ERR_RAU_RMON_BUFERFULL;
		break;
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(ログ登録)
	case eLOG_LONGPARK_PWEB:
		errorCode = ERR_RAU_LONGPARK_BUFERFULL;
		break;
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(ログ登録)
	case eLOG_MONEYMANAGE:								// 金銭管理(バッファフルエラーコードなし)
	default:
		return;											// 上記以外はチェックしない
	}
	
	cnt = LOG_SECORNUM(Lno);
	lp = LOG_DAT+Lno;
	
	if(strage) {										// FROM+SRAMのログ
		if(occur == TRUE) {								// 発生チェック
			// FROMに書込む際に未送信データが上書きされる場合はバッファフル発生とする
			maxcnt = Ope_Log_GetLogMaxCount(Lno) - cnt;
			if((Ope_Log_TotalCountGet(Lno) - lp->unread[target]) >= maxcnt) {
				bufferfull = 1;
			}
		}
		else {											// 解除チェック
			if(lp->Fcount == FLT_Get_LogSectorCount(Lno)) {					// FROMが1周している
				if(lp->unread[target] > lp->count[lp->kind]) {				// FROMに送信済みデータがある
					if(cnt <= (lp->unread[target] - lp->count[lp->kind])) {	// 1セクタ分FROMに空きがある
						bufferfullRel = 1;
					}
				}
			}
			else {
				bufferfullRel = 1;
			}
		}
	}
	else {												// SRAMのみのログ
		if(occur == TRUE) {								// 発生チェック
			if(lp->f_unread[target] >= cnt) {
				bufferfull = 1;
			}
		}
		else {											// 解除チェック
			if(lp->f_unread[target] < cnt) {
				bufferfullRel = 1;
			}
		}
	}
	
	if(bufferfull) {
		RAU_err_chk(errorCode, 1, 0, 0, NULL);
		if(pState) {
			if(Lno == eLOG_PAYMENT) {
				// 精算データはニアフルも設定する
				*pState = (uchar)(NTBUF_BUFFER_FULL|NTBUF_BUFFER_NEAR_FULL); 
			}
			else {
				*pState = (uchar)NTBUF_BUFFER_FULL;
			}
		}
	}
	else if(bufferfullRel) {
		RAU_err_chk(errorCode, 0, 0, 0, NULL);
		if(pState) {
			*pState ^= (uchar)NTBUF_BUFFER_FULL;
		}
	}
	else {
		if(Lno == eLOG_PAYMENT) {					// 精算データ
			if(lp->unread[target] > lp->count[lp->kind]) {				// FROMに送信済みデータがある
				if(RAU_NTBUF_NEARFULL_COUNT <= (lp->unread[target] - lp->count[lp->kind])) {
					*pState ^= (uchar)NTBUF_BUFFER_NEAR_FULL;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| LOGデータアクセス権取得 (セマフォフラグ取得)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_SemGet( DtNum )                                     |*/
/*| PARAMETER    : データ番号（"OPE_DTNUM_xxxx" シンボルを使用すること）   |*/
/*| RETURN VALUE : 0=取得失敗（使用中）,1=成功                             |*/
/*|				   ※ 用済み後は必ず開放すること。Log_SemFre(DtNum)		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-20                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	Log_SemGet( uchar DtNum )
{
	ulong	TargetBit;

	TargetBit = 1L << DtNum;

	if( 0 == (Ope_f_Sem_DataAccess & TargetBit) ){			/* 取得可能 (Y) */
		Ope_f_Sem_DataAccess |= TargetBit;
		return	(uchar)1;
	}
	return	(uchar)0;
}

/*[]----------------------------------------------------------------------[]*/
/*| LOGデータアクセス権開放 (セマフォフラグ開放)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_SemFre( DtNum )                                     |*/
/*| PARAMETER    : データ番号（"OPE_DTNUM_xxxx" シンボルを使用すること）   |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-20                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Log_SemFre( uchar DtNum )
{
	ulong	TargetBit;

	TargetBit = 0xffffffff;
	TargetBit ^= (1L << DtNum);
	Ope_f_Sem_DataAccess &= TargetBit;
}

/*[]----------------------------------------------------------------------[]*/
/*| モニタ/操作モニタデータの処理方法取得                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : getMonitorInfo                                          |*/
/*| PARAMETER    : kind = モニタ/操作モニタ種別                            |*/
/*|				   code = モニタ/操作モニタコード						   |*/
/*|				   tbl  = 処理方法情報の検査対象とするテーブル			   |*/
/*|				   cnt  = "tbl"は配列なので、その要素数					   |*/
/*| RETURN VALUE : モニタ/操作モニタデータの処理方法                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const t_NTNET_MonitorCtrl	*getMonitorInfo(uchar kind, uchar code, const t_NTNET_MonitorTbl *tbl, size_t tbl_cnt)
{
	size_t i, j;
	
	for (i = 0; i < tbl_cnt; i++) {
		if (tbl[i].Kind == kind) {
			for (j = 0; j < tbl[i].DataNum; j++) {
				if (tbl[i].CtrlData[j].ID == code) {
					return &tbl[i].CtrlData[j];
				}
			}
			break;
		}
	}
	
	return NULL;
}

/*[]----------------------------------------------------------------------[]*
 *| ascii(2byte) --> bcd(1byte)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : asciitobcd
 *| PARAMETER    : ascii1:ascii(1byte), ascii2:ascii(1byte)
 *| RETURN VALUE : bcd(1byte)
 *| ex) ascii1=0x31, ascii2=0x32 --> retuen(0x12)
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	asciitobcd( uchar ascii1, uchar ascii2 )
{
	uchar	bcd;

	ascii2 &= 0x0f;
	ascii1 <<= 4;
	ascii1 |= ascii2;
	bcd = ascii1;

	return(bcd);
}
/*[]-----------------------------------------------------------------------[]*/
/*|	20文字までのASCII文字列（10進文字:'0'～'9'）を BCD(10byte)に変換		|*/
/*|																			|*/
/*|	後ろ詰めで空きには0x0が格納される。										|*/
/*|	ex) ASCII=(31|32|33) --> BCD=(00|00|00|00|00|00|00|00|01|23)			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : ascii = 変換元Ascii文字列へのポインタ					|*/
/*|				   bcd = 変換後データセットエリアへのポインタ				|*/
/*|				   ascii_len = ascii の文字数								|*/
/*| RETURN VALUE : なし														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : M.Suzuki													|*/
/*| Date         : 2006/02/22												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	ASCIItoBCD( uchar *ascii, uchar *bcd, ushort ascii_len )
{
	uchar	w_ascii[20];	
	uchar	w_bcd[10];	
	ushort	p_ascii, p_bcd, i;

	memset( w_bcd, 0, sizeof(w_bcd) );
	memset( w_ascii, 0, sizeof(w_ascii) );
	memcpy( w_ascii, ascii,  (size_t)ascii_len );

	p_ascii = ascii_len - 1;
	p_bcd = 10 -1;
	
	for(i=0; i<ascii_len; ) {
		if( (ascii_len - i) < 2 ) {
			w_bcd[p_bcd] = asciitobcd( '0', w_ascii[p_ascii] );
		} else {
			w_bcd[p_bcd] = asciitobcd( w_ascii[p_ascii-1], w_ascii[p_ascii] );
		}
		p_ascii -= 2;
		p_bcd--; 
		i += 2;
	}
	memcpy( bcd, w_bcd, sizeof(w_bcd));
	return;
}
/*[]---------------------------------------------------------------------------[]*/
/*|		不正券ログデータ格納処理												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	NgLog_write( card_type, cardno, cardno_len )			|*/
/*|																				|*/
/*|	PARAMETER		:	ushort card_type	=	カード種別						|*/
/*|						char *cardno		=	カード番号ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|						ushort cardno_len	=	カード番号ﾃﾞｰﾀﾚﾝｸﾞｽ				|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki														|*/
/*|	Date	:	2006-02-20														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
void	NgLog_write( ushort card_type, uchar *cardno, ushort cardno_len )
{
	ushort	w_cardno;

	/* カード種別セット */
	nglog_data.Card_type = card_type;
	
	/* カード番号セット */
	switch( card_type ){
		case	NG_CARD_PASS:		// 定期券
		case	NG_CARD_TICKET:		// 駐車券
		case	NG_CARD_AMANO:		// 不正アマノ係員カード
		case	OK_CARD_AMANO:		// 正常アマノ係員カード
			w_cardno = astoin ( cardno, (short)cardno_len );
			nglog_data.Card_No.card_s = w_cardno;
			break;

		case	NG_CARD_CREDIT:		// クレジットカード
		case	NG_CARD_BUSINESS:	// ビジネスカード
			ASCIItoBCD( cardno, nglog_data.Card_No.card_c, cardno_len );
			break;

		default:					// 未定義カード種別
			nglog_data.Card_No.card_s = 0;
			break;
	}

	/* 日付セット（現在日時） */
	memcpy( &nglog_data.NowTime, &CLK_REC, sizeof( date_time_rec ) );
	Log_Write(eLOG_NGLOG, &nglog_data, TRUE);					// 不正ログ登録
	return;
}
/*[]---------------------------------------------------------------------------[]*/
/*|		入出庫ログデータ格納処理												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	IoLog_write( event_type, sub_type )						|*/
/*|																				|*/
/*|	PARAMETER		:	ushort event_type	=	入出庫イベント種別				|*/
/*|						ushort sub_type		=	入出庫イベント種別（詳細）		|*/
/*|						ushort ticketnum	=	券番号（精算開始用）			|*/
/*|						ushort flg			=	復電時の２重登録ﾁｬｯｸ要(1)否(0)	|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki														|*/
/*|	Date	:	2006-03-03														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
void	IoLog_write(ushort event_type, ushort room, ulong ticketnum, ulong flg )
{
// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//	uchar	is_save = 1;																			/* ログ書き込みフラグ(0:不/1:可) */
//	ushort	 cnt;																					// 入出庫ログ格納先, 事象記録先
//	ushort	LockIndex;																				/* 目的の車室番号が格納されているINDEX */
//	ST_OIBAN2	comp_oiban;																			/* 比較対象の追番 */
//	ST_OIBAN2	oiban;																				/* 取得した追番 */
//
//	cnt = 0;
//	is_save = (uchar)LKopeGetLockNum(0, room, &LockIndex);											/* 車室番号からINDEXを取得 */
//	if(is_save)																						/* 車室番号該当あり(記録可能) */
//	{
//		if(LockIndex > BIKE_START_INDEX)															/* 現在車室の装置がロック */
//		{
//			is_save = (uchar)prm_get(COM_PRM, S_TYP, 127, 1, 1);									/* 03-0127⑥から記録可否取得 */
//			IO_LOG_REC.LockIndex = (LockIndex - BIKE_START_INDEX - 1) + INT_CAR_LOCK_MAX;			// ロック装置用の領域は20～
//			if (IO_LOG_REC.LockIndex >= IO_LOG_CNT) {
//				is_save = 0;
//			}
//		}
//		else if(LockIndex <= BIKE_START_INDEX)														/* 現在車室の装置がフラップ */
//		{
//			is_save = (uchar)prm_get(COM_PRM, S_TYP, 127, 1, 2);									/* 03-0127⑤から記録可否取得 */
//			IO_LOG_REC.LockIndex = (LockIndex - INT_CAR_START_INDEX - 1);							// フラップ装置用の領域は0～
//			if (IO_LOG_REC.LockIndex >= IO_LOG_CNT) {
//				is_save = 0;
//			}
//		}
//		else																						/* 上記以外(記録無効設定) */
//		{
//			is_save = 0;																			/* ログ書き込み無効 */
//		}
//	}
//
//	if(is_save)																						/* ログ書き込み可能設定 */
//	{
//		if(event_type == IOLOG_EVNT_ENT_GARAGE)														// ログ種別が入庫
//		{
//			// 指定された車室番号の領域が記録中(既に入庫状態で更に入庫が発生した)
//			if(IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec == 1) {
//				// 次の入庫で記録中の車室は記録終了にし、新たな領域にログの記録を開始する。
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 2;
//				Log_Write(eLOG_IOLOG, &IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], TRUE);			// ログに登録
//			}
//
//			// 入出庫情報ログ格納処理
//			memset(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], 0, sizeof(IoLog_Data));			// ログ格納エリアを初期化
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].Event_type = event_type;		// 事象(入庫)
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].room = room;								// 車室番号 
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 1;									// ログ記録中に変更 
//			// イベント発生日時保存
//			memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime, &CLK_REC, sizeof(date_time_rec));
//			if(flg)																					// フラグに値がある(デバッグ用) 
//			{
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime.Hour = (uchar)(flg);	// 入庫時間(時)
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime.Min = (uchar)(0);		// 入庫時間(分)
//			}
//		}
//		else																						// ログ種別が入庫以外
//		{
//			// 指定された車室番号の領域が記録中(既に入庫状態で更に入庫が発生した)
//			if(IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec == 1) {
//				// 記録中領域で記録可能な位置を探す(最後まで情報があったら記録しない) 
//				for(cnt = 0; (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].Event_type) && (cnt < IO_EVE_CNT); cnt++);
//			}
//
//			if(cnt < IO_EVE_CNT)																	// ログ格納位置が最後まで到達していない
//			{
//				memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, &CLK_REC, sizeof(date_time_rec));	// イベント発生日時保存
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].Event_type = event_type;// 入出庫イベント種別セット
//				comp_oiban = IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].IO_INF.io_l;	// 直前の事象の精算追番を取得
//
//				switch( event_type ){																// イベント毎に格納処理を分ける
//					case	IOLOG_EVNT_AJAST_FIN:													// 精算完了
//						CountGet( PAYMENT_COUNT, (ST_OIBAN *)&oiban );											// 追番の取得
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// 直前の精算追番が今回と同じ
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//						/* 精算追番取得 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_AJAST_STP:													// 精算中止
//						CountGet( CANCEL_COUNT, (ST_OIBAN *)&oiban );											// 追番の取得
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// 直前の精算追番が今回と同じ
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//						/* 精算中止追番取得 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_LOCK_OP_FIN:													// 駐輪ロック開完了
//					case	IOLOG_EVNT_LOCK_OP_UFN:													// 駐輪ロック開未完
//					case	IOLOG_EVNT_FLAP_DW_FIN:													// フラップ下降完了
//					case	IOLOG_EVNT_FLAP_DW_UFN:													// フラップ下降未完
//						/* 前回のイベント種別が今回と同じ */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_OUT_GARAGE:													// 出庫
//						/* 前回のイベント種別が今回と同じ */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_FORCE_FIN:													// 強制出庫完了
//						CountGet( KIYOUSEI_COUNT, (ST_OIBAN *)&oiban );											// 追番の取得
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// 直前の精算追番が今回と同じ
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//					
//			/* イベント発生日時と不正・強制出庫の発生時刻はずれる場合があるので、 現在時刻ではなく強制出庫の発生時刻をセットし直す*/
//						memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, 
//							&FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].Date_Time, sizeof(date_time_rec));
//						/* 強制出庫追番取得 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_OUT_ILLEGAL:													// 不正出庫
//						CountGet( FUSEI_COUNT, (ST_OIBAN *)&oiban );											// 追番の取得
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// 直前の精算追番が今回と同じ
//							is_save = 0;															// ログ追加無し
//							break;																	// 分岐処理終了
//						}
//						/* 不正出庫追番取得 */
//			/* イベント発生日時と不正・強制出庫の発生時刻はずれる場合があるので、 現在時刻ではなく強制出庫の発生時刻をセットし直す*/
//						memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, 
//							&FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].Date_Time, sizeof(date_time_rec));
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// 分岐処理終了
//
//					case	IOLOG_EVNT_LOCK_OP_STA:													// 駐輪ロック開開始
//					case	IOLOG_EVNT_LOCK_CL_STA:													// 駐輪ロック閉開始
//					case	IOLOG_EVNT_LOCK_CL_FIN:													// 駐輪ロック閉完了
//					case	IOLOG_EVNT_LOCK_CL_UFN:													// 駐輪ロック閉未完
//					case	IOLOG_EVNT_FLAP_DW_STA:													// フラップ下降開始
//					case	IOLOG_EVNT_FLAP_UP_STA:													// フラップ上昇開始
//					case	IOLOG_EVNT_FLAP_UP_FIN:													// フラップ上昇完了
//					case	IOLOG_EVNT_FLAP_UP_UFN:													// フラップ上昇未完
//					case	IOLOG_EVNT_RENT_GARAGE:													// 再入庫
//					case	IOLOG_EVNT_FORCE_STA:													// 強制出庫開始
//					case	IOLOG_EVNT_AJAST_STA:													// 精算開始
//					case	IOLOG_EVNT_OUT_ILLEGAL_START:											// 不正出庫発生
//						/* 前回のイベント種別が今回と同じ */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ログ追加無し
//						}
//						break;																		// 分岐処理終了
//
//					default:																		// 上記以外(予期しないイベント)
//						is_save = 0;																// ログ追加無し
//						break;																		// 分岐処理終了
//				}
//				if(is_save)																			// ログ追加可能
//				{
//					if((cnt + 1) >= IO_EVE_CNT)														// 今回の事象が20回目
//					{
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 2;						// 入出庫履歴記録終了
//						Log_Write(eLOG_IOLOG, &IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], TRUE);	// ログに登録
//					}
//				}
//			}
//		}
//	}
//	return;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
}

//------------------------------------------------------------------------------------------
// 設定値エリア比較し、違いのあるアドレスの情報を設定変更履歴バッファに格納する。
// 複数格納用の関数を使用する前に必ずコール
//
// Param:
//  syubetu ( in ) : 変更種別
//
//
//------------------------------------------------------------------------------------------
void SetSetDiff( unsigned char syubetu )
{

	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;						// 設定更新履歴保存未動作
	SetDiffFirstFlag = 0;											// 設定更新履歴初回格納有効
	SetDiffLogBackUp ( );											// 復電時ﾘｶﾊﾞﾘ用ｴﾘｱにﾊﾞｯｸｱｯﾌﾟ
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_ACTIVE;					// 設定更新履歴保存動作中
	if(syubetu == SETDIFFLOG_SYU_DEFLOAD)							// 記録する情報がヘッダのみ
	{
		if(SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.syu == SETDIFFLOG_SYU_PCTOOL)	// 前回ヘッダ情報がPCからの変更要求
		{
			SetDiffFirstFlag = 1;									// 設定更新履歴初回格納無効
			SetSetDiff_Set(syubetu);								// 差分LOGに登録.
		}
		SetDiff_PutLogInfoHead(syubetu);							// デフォルトセットログ格納
	}
	/* 変更種別がＰＣツールからの変更要求 または ＰＣからの設定要求 */
// GG120600(S) // Phase9 設定DLの履歴を残す
//	else if((syubetu == SETDIFFLOG_SYU_PCTOOL) || (syubetu == SETDIFFLOG_SYU_PCDEFLOAD))
	else if((syubetu == SETDIFFLOG_SYU_PCTOOL) || (syubetu == SETDIFFLOG_SYU_PCDEFLOAD) || (syubetu == SETDIFFLOG_SYU_REMSET))
// GG120600(E) // Phase9 設定DLの履歴を残す
	{
		SetDiff_PutLogInfoHead(syubetu);							// 個別ﾍｯﾀﾞｰのみ格納
	}
	else															// 他のパラメータ変更登録
	{
		SetSetDiff_Set(syubetu);									// 差分LOGに登録.
	}
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;						// 設定更新履歴保存未動作
}
//------------------------------------------------------------------------------------------
// 設定値エリアを比較し、違いのあるアドレスの情報を設定変更履歴バッファに格納する。
//
// Param:
//  sta_add ( in ) : 開始アドレス
//  end_add ( in ) : 終了アドレス
//  syubetu ( in ) : 変更種別
//------------------------------------------------------------------------------------------
void SetSetDiff_Set(unsigned char syubetu)
{
	short	ses;													// 参照するセッション
	short	addr;													// 参照するアドレス
	short	end_add;												// 参照中セッションのアドレス数

	for(ses = 0; ses < C_PRM_SESCNT_MAX; ses++)						// セッション毎のログ登録処理
	{
		end_add = (short)AppServ_CParamSizeTbl[ses];				// パラメータ数取得
		for(addr = 1; addr < end_add; addr++)						// パラメータ分処理を繰り返す
		{
			// sta_add(1) end_add(1699) 差分=380 : ICE計測 40～50msec
			WACDOG;													// Reset Watch Dog Counter.
			// 差分チェック
			if(CPrmSS[ses][addr] != CPrmMrr[ses][addr])				// ミラー領域と本領域で値が違う
			{
				if(SetDiffFirstFlag == 0)							// 初回履歴登録
				{
					SetDiffFirstFlag = (unsigned char)1;			// 初回履歴登録完了
					SetDiff_PutLogInfoHead(syubetu);				// 個別ﾍｯﾀﾞｰ PUT
					SetDiffLogBack.cycl = SETDIFFLOG_CYCL_DIFFING;	// ヘッダー格納中→差分情報格納中
				}
				// 1情報 Put
				SetDiff_PutLogInfoData(ses, addr, CPrmMrr[ses][addr], CPrmSS[ses][addr]);
			}
		}
	}
}
//------------------------------------------------------------------------------------------
// 差分データを設定変更履歴バッファに格納する
//
// Param:
//  addr ( in ) : アドレス
//  dataBef ( in ) : 変更前データ ( [x][0],[x][1],[x][2],[x][3]を HEX4桁でとらえたﾊﾞｲﾅﾘｰデータ )
//  dataAft ( in ) : 変更後データ ( [x][0],[x][1],[x][2],[x][3]を HEX4桁でとらえたﾊﾞｲﾅﾘｰデータ )
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoData( short ses, short addr, long dataBef, long dataAft )
{
	// 差分情報の格納件数が上限未満
	if(SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.count < SETDIFFLOG_BAK)
	{
		SetDiffWorkBuff.diff.ses = ses;								// セッション
		SetDiffWorkBuff.diff.add = addr;							// アドレス
		SetDiffWorkBuff.diff.bef = dataBef;							// 変更前設定値
		SetDiffWorkBuff.diff.aft = dataAft;							// 変更後設定値
		SetDiff_PutLogInfoSave(1);									// 個別データ差分情報格納
	}
	SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.zen_cnt++;		// 差分データ全件数は常に加算する
	nmisave(&CPrmMrr[ses][addr], &CPrmSS[ses][addr], 4);			// 設定値をミラー領域へ反映
}
//------------------------------------------------------------------------------------------
// 設定変更履歴バッファにデータ個別ヘッダーを格納する
//
// Param:
//  syubetu (in) : 変更種別
//
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoHead( unsigned char syubetu )
{
	// 参照中情報個別ヘッダーポインタ記憶.今後の差分情報格納時に参照.
	SetDiffLogDtHeadPon = SetDiffLogBuff.Header.InPon;
	SetDiffWorkBuff.head.hed_mark[0] = 'H';							// ヘッダーマーク(上位)
	SetDiffWorkBuff.head.hed_mark[1] = 'D';							// ヘッダーマーク(下位)
	SetDiffWorkBuff.head.syu = syubetu;								// 記録情報種別
	SetDiffWorkBuff.head.count = 0;									// 個別ﾍｯﾀﾞｰとﾀｲﾑｽﾀﾝﾌﾟは含まれない数
	SetDiffWorkBuff.head.zen_cnt = 0;								// 個別ﾍｯﾀﾞｰとﾀｲﾑｽﾀﾝﾌﾟは含まれない数
	SetDiffWorkBuff.head.year = CLK_REC.year;						// 日時情報(年)
	SetDiffWorkBuff.head.mont = CLK_REC.mont;						// 日時情報(月)
	SetDiffWorkBuff.head.date = CLK_REC.date;						// 日時情報(日)
	SetDiffWorkBuff.head.hour = CLK_REC.hour;						// 日時情報(時)
	SetDiffWorkBuff.head.minu = CLK_REC.minu;						// 日時情報(分)
	SetDiff_PutLogInfoSave(0);										// ヘッダー格納処理
}
//------------------------------------------------------------------------------------------
// 設定変更履歴バッファに１情報格納する.
// 格納位置がデータ個別ヘッダーの場合は、その１かたまりデータを破棄する
//
// Param:
//  cnt_add_flag ( in ) : 個別ﾍｯﾀﾞｰのｶｳﾝﾀ加算する(1)/しない(0)
//
// Global:
// SetDiffWorkBuff ( in ) : 格納するデータ
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoSave( unsigned char cnt_add_flag )
{

	if ( SetDiffLogBuff.Header.UseInfoCnt >= SETDIFFLOG_MAX )
	{
		SetDiffLogBuff.Header.UseInfoCnt = 							// 情報使用個数 - n(+1=個別ヘッダー)
				SetDiffLogBuff.Header.UseInfoCnt - (unsigned short)( SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon].head.count + 1 );
		SetDiffLogBuff.Header.OlderPon = 							// ﾊﾞｯﾌｧﾍｯﾀﾞｰの最古ポインタ更新(+1=個別ヘッダー)
				SetDiff_UpdatePon ( SetDiffLogBuff.Header.OlderPon, (unsigned short)(SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon].head.count + 1) );
	}

	// 現在格納位置へ情報格納
	memcpy ( &SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon], &SetDiffWorkBuff, sizeof(TD_SETDIFFLOG_INFO));

	if( cnt_add_flag ){												// 保存情報が差分情報
		SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.count++;		// データ個別ヘッダーのデータ数 + 1
	}
	//バッファヘッダーの入力ポインタ更新
	SetDiffLogBuff.Header.InPon = SetDiff_UpdatePon ( SetDiffLogBuff.Header.InPon, 1 );

	if ( SetDiffLogBuff.Header.UseInfoCnt < SETDIFFLOG_MAX ) {		// 記憶件数が領域の末尾に達していない
		SetDiffLogBuff.Header.UseInfoCnt++;							// ﾊﾞｯﾌｧﾍｯﾀﾞｰの使用情報数＋１
	}
}
//------------------------------------------------------------------------------------------
// 入力ポインター、最古情報ポインターを 指定数だけ進める.ケツの場合は先頭にまわしこむ
//
// Param:
//  OldPon ( in ) : 入力ポインター、最古情報ポインター
//  AddPon ( in ) : 加算する値
//
// Return:
//  加算後の値
//------------------------------------------------------------------------------------------
unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon )
{
	OldPon += AddPon;

	if( OldPon >= SETDIFFLOG_MAX )
	{
		return (unsigned short)( OldPon - SETDIFFLOG_MAX );
	}
	return OldPon;
}
//------------------------------------------------------------------------------------------
// 処理途中の電源断＝＞複電時のリカバーエリアに １履歴情情報とバッファヘッダーをセーブする。
// セーブする情報は、最古データから１履歴情報（つぶされる可能性ある）最大数行う。
//
// これから格納する差分情報の数はまだわからない、ので、常に１履歴情報最大数セーブしてしまう.
//
// SetDiffLogBack.Info[0] から１履歴情報最大＋２(ﾍｯﾀﾞ､ﾀｲﾑｽﾀﾝﾌﾟ)だけ最古データが保存される
//------------------------------------------------------------------------------------------
void SetDiffLogBackUp ( void )
{
	unsigned short	i;
	unsigned short	pon;

	//バッファヘッダーセーブ
	memcpy ( &SetDiffLogBack.Header, &SetDiffLogBuff.Header, sizeof(TD_SETDIFFLOG_INDX));

	//履歴情報セーブ
	pon = SetDiffLogBuff.Header.OlderPon;					//最古ポインター
	for( i = 0; i < SETDIFFLOG_BAK + (2); i++ )
	{
		memcpy ( &SetDiffLogBack.Info[i], &SetDiffLogBuff.Info[pon], sizeof(TD_SETDIFFLOG_INDX));
		pon = SetDiff_UpdatePon ( pon, 1 );
	}
}
//------------------------------------------------------------------------------------------
// 復電時のリカバー
// リカバーエリアのヘッダーを復活後、履歴情報を復活させる
//
// 常に SetDiffLogBack.Info[0] から１履歴情報最大＋ヘッダーだけ最古データが保存されている
//------------------------------------------------------------------------------------------
void SetDiffLogRecoverCheck_Go ( void )
{
	unsigned short	i;
	unsigned short	pon;

	//safe... 基板の一番最初の立ち上げ時,.or.開発時, 可能性ある.(memclr() 前にこの処理行うから)
	if ( SetDiffLogBuff.Header.UseInfoCnt > SETDIFFLOG_MAX ||
	     SetDiffLogBuff.Header.InPon >= SETDIFFLOG_MAX ||
	     SetDiffLogBuff.Header.OlderPon >= SETDIFFLOG_MAX )
	{
		memset ( &SetDiffLogBuff, 0, sizeof(SetDiffLogBuff) );
	}

	// リカバーの必要あるか？
	if ( SetDiffLogBack.cycl == SETDIFFLOG_CYCL_ACTIVE &&
		 SetDiffLogBack.Info[0].head.hed_mark[0] == 'H' &&
		 SetDiffLogBack.Info[0].head.hed_mark[1] == 'D' ) {

		//バッファヘッダー
		memcpy ( &SetDiffLogBuff.Header, &SetDiffLogBack.Header, sizeof(TD_SETDIFFLOG_HEAD));

		pon = SetDiffLogBuff.Header.OlderPon;					//最古ポインター
		for( i = 0; i < SETDIFFLOG_BAK + (2); i++ )
		{
			memcpy ( &SetDiffLogBuff.Info[pon], &SetDiffLogBack.Info[i], sizeof(TD_SETDIFFLOG_INDX));
			pon = SetDiff_UpdatePon ( pon, 1 );
		}
	}
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;

}
//------------------------------------------------------------------------------------------
// 復電時の領域初期化処理
// ミラー領域への情報複写を行う
//
// 引数によって初期化処理と処理手順を変更する
//------------------------------------------------------------------------------------------
void SetDiffLogReset( uchar ope_mode )
{
	switch(ope_mode)												// 引数によって処理を分岐
	{
	case 1:															// 設定更新履歴情報初期化
		memset(&SetDiffLogBuff, 0, sizeof(SetDiffLogBuff));			// 情報保存領域全初期化
		memset(&SetDiffLogBack, 0, sizeof(SetDiffLogBack));			// バックアップ領域初期化
		SetDiffLogDtHeadPon = 0;									// 最新ヘッダ情報位置初期化
	case 2:															// ミラーリング処理
		memcpy(&CPMirror, &CParam, sizeof(t_CParam));				// 共通パラメータをミラー領域へ複写
	case 0:															// 通常起動時
		SetDiffLogMirrorAddress();									// ミラー領域のアドレス配列を適用
		if(SetDiffLogBack.cycl == SETDIFFLOG_CYCL_DIFFING)			// 前回停電直前の動作が登録中
		{
			SetSetDiff_Set(SETDIFFLOG_SYU_NONE);					// 設定更新履歴登録処理再開
		}
		break;														// 分岐処理終了
	}
}
//------------------------------------------------------------------------------------------
// 復電時の領域初期化処理
// ミラー領域への情報複写を行う
//
// 引数によって初期化処理と処理手順を変更する
//------------------------------------------------------------------------------------------
void SetDiffLogMirrorAddress(void)
{
		CPrmMrr[0]	= CPMirror.CParam00;							/*								*/
		CPrmMrr[1]	= CPMirror.CParam01;							/*								*/
		CPrmMrr[2]	= CPMirror.CParam02;							/*								*/
		CPrmMrr[3]	= CPMirror.CParam03;							/*								*/
		CPrmMrr[4]	= CPMirror.CParam04;							/*								*/
		CPrmMrr[5]	= CPMirror.CParam05;							/*								*/
		CPrmMrr[6]	= CPMirror.CParam06;							/*								*/
		CPrmMrr[7]	= CPMirror.CParam07;							/*								*/
		CPrmMrr[8]	= CPMirror.CParam08;							/*								*/
		CPrmMrr[9]	= CPMirror.CParam09;							/*								*/
		CPrmMrr[10]	= CPMirror.CParam10;							/*								*/
		CPrmMrr[11]	= CPMirror.CParam11;							/*								*/
		CPrmMrr[12]	= CPMirror.CParam12;							/*								*/
		CPrmMrr[13]	= CPMirror.CParam13;							/*								*/
		CPrmMrr[14]	= CPMirror.CParam14;							/*								*/
		CPrmMrr[15]	= CPMirror.CParam15;							/*								*/
		CPrmMrr[16]	= CPMirror.CParam16;							/*								*/
		CPrmMrr[17]	= CPMirror.CParam17;							/*								*/
		CPrmMrr[18]	= CPMirror.CParam18;							/*								*/
		CPrmMrr[19]	= CPMirror.CParam19;							/*								*/
		CPrmMrr[20]	= CPMirror.CParam20;							/*								*/
		CPrmMrr[21]	= CPMirror.CParam21;							/*								*/
		CPrmMrr[22]	= CPMirror.CParam22;							/*								*/
		CPrmMrr[23]	= CPMirror.CParam23;							/*								*/
		CPrmMrr[24]	= CPMirror.CParam24;							/*								*/
		CPrmMrr[25]	= CPMirror.CParam25;							/*								*/
		CPrmMrr[26]	= CPMirror.CParam26;							/*								*/
		CPrmMrr[27]	= CPMirror.CParam27;							/*								*/
		CPrmMrr[28]	= CPMirror.CParam28;							/*								*/
		CPrmMrr[29]	= CPMirror.CParam29;							/*								*/
		CPrmMrr[30]	= CPMirror.CParam30;							/*								*/
		CPrmMrr[31]	= CPMirror.CParam31;							/*								*/
		CPrmMrr[32]	= CPMirror.CParam32;							/*								*/
		CPrmMrr[33]	= CPMirror.CParam33;							/*								*/
		CPrmMrr[34]	= CPMirror.CParam34;							/*								*/
		CPrmMrr[35]	= CPMirror.CParam35;							/*								*/
		CPrmMrr[36]	= CPMirror.CParam36;							/*								*/
		CPrmMrr[37]	= CPMirror.CParam37;							/*								*/
		CPrmMrr[38]	= CPMirror.CParam38;							/*								*/
		CPrmMrr[39]	= CPMirror.CParam39;							/*								*/
		CPrmMrr[40]	= CPMirror.CParam40;							/*								*/
		CPrmMrr[41]	= CPMirror.CParam41;							/*								*/
		CPrmMrr[42]	= CPMirror.CParam42;							/*								*/
		CPrmMrr[43]	= CPMirror.CParam43;							/*								*/
		CPrmMrr[44]	= CPMirror.CParam44;							/*								*/
		CPrmMrr[45]	= CPMirror.CParam45;							/*								*/
		CPrmMrr[46]	= CPMirror.CParam46;							/*								*/
		CPrmMrr[47]	= CPMirror.CParam47;							/*								*/
		CPrmMrr[48]	= CPMirror.CParam48;							/*								*/
		CPrmMrr[49]	= CPMirror.CParam49;							/*								*/
		CPrmMrr[50]	= CPMirror.CParam50;							/*								*/
		CPrmMrr[51]	= CPMirror.CParam51;							/*								*/
		CPrmMrr[52]	= CPMirror.CParam52;							/*								*/
		CPrmMrr[53]	= CPMirror.CParam53;							/*								*/
		CPrmMrr[54]	= CPMirror.CParam54;							/*								*/
		CPrmMrr[55]	= CPMirror.CParam55;							/*								*/
		CPrmMrr[56]	= CPMirror.CParam56;							/*								*/
		CPrmMrr[57]	= CPMirror.CParam57;							/*								*/
		CPrmMrr[58]	= CPMirror.CParam58;							/*								*/
		CPrmMrr[59]	= CPMirror.CParam59;							/*								*/
		CPrmMrr[60]	= CPMirror.CParam60;							/*								*/
}

//[]----------------------------------------------------------------------[]
///	@brief		集計ログにNT-NETデータで設定するシーケンシャルNo.を設定する
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno			: ログ種別(LOG_TTOTAL/LOG_GTTOTAL)
///				pTotalLog	: 集計ログデータ
///	@return		none
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	Log_SetTotalLogNtSeqNo(short Lno, SYUKEI* pTotalLog)
{
	ushort	index = 0;

	if(Lno == LOG_TTOTAL || Lno == LOG_GTTOTAL) {			// T合計またはGT合計が対象
		
		// データ種別分のシーケンシャルNo.を設定する
		for(index = 0; index < TOTALLOG_NTDATA_COUNT; ++index) {
			pTotalLog->SeqNo[index] = GetNtDataSeqNo();
		}
	}
}

// テスト用ログ作成(S)
#ifdef TEST_LOGFULL
void Test_Ope_LogFull(void)
{
	ushort i;
	ushort max = 0;
	struct {
		uchar payment;
		uchar total;
		uchar gttotal;
		uchar enter;
		uchar power;
		uchar error;
		uchar alarm;
		uchar operation;
		uchar monitor;
		uchar coinbox;
		uchar notebox;
		uchar turikan;
	} target;				// デバッグ用
	
	memset(&target, 1, sizeof(target));
	
	// 精算データの作成
	if(target.payment) {
		max = Ope_Log_GetLogMaxCount(eLOG_PAYMENT) - 1;
		for(i = 0; i < max; ++i) {
			OPECTL.f_KanSyuu_Cmp = 0;
			OPECTL.Op_LokNo = 1;
			car_in_f.year = car_ot_f.year = CLK_REC.year;
			car_in_f.mon = car_ot_f.mon = CLK_REC.mont;
			car_in_f.day = car_ot_f.day = CLK_REC.date;
			car_in_f.hour = car_ot_f.hour = CLK_REC.hour;
			car_in_f.min = car_ot_f.min = CLK_REC.minu;
			PayData_set(0, 0);
			kan_syuu();
			taskchg(IDLETSKNO);
		}
	}
	
	// 集計データの作成
	if(target.total) {
		max = Ope_Log_GetLogMaxCount(eLOG_TTOTAL) - 1;
		for(i = 0; i < max; ++i) {
			syuukei_prn(PREQ_AT_SYUUKEI, &sky.tsyuk);
			memcpy( &sky.tsyuk.NowTime, &CLK_REC, sizeof(date_time_rec));
			ac_flg.syusyu = 11;							// 11:Ｔ合計印字完了
			Make_Log_TGOUKEI();
			syuukei_clr(0);
			taskchg(IDLETSKNO);
		}
	}

	// GTデータの作成
	if(target.gttotal) {
		max = Ope_Log_GetLogMaxCount(eLOG_GTTOTAL) - 1;
		for(i = 0; i < max; ++i) {
			syuukei_prn(PREQ_AT_SYUUKEI, &sky.gsyuk);
			memcpy( &sky.gsyuk.NowTime, &CLK_REC, sizeof(date_time_rec));
			ac_flg.syusyu = 22;							// 22:ＧＴ合計印字完了
			syuukei_clr(1);
			taskchg(IDLETSKNO);
		}
	}

// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	// 入庫データの作成
//	if(target.enter) {
//		max = Ope_Log_GetLogMaxCount(eLOG_ENTER) - 1;
//		for(i = 0; i < max; ++i) {
//			FLAPDT.flp_data[1].year = CLK_REC.year;
//			FLAPDT.flp_data[1].mont = CLK_REC.mont;
//			FLAPDT.flp_data[1].date = CLK_REC.date;
//			FLAPDT.flp_data[1].hour = CLK_REC.hour;
//			FLAPDT.flp_data[1].minu = CLK_REC.minu;
//			Make_Log_Enter(1);
//			Log_regist(LOG_ENTER);
//			taskchg(IDLETSKNO);
//		}
//	}
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	
	// 停復電データの作成
	if(target.power) {
		max = Ope_Log_GetLogMaxCount(eLOG_POWERON) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_POWERON);
			taskchg(IDLETSKNO);
		}
	}
	
	// エラーデータの作成
	if(target.error) {
		memcpy( &Err_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
		Err_work.Errsyu = 255;											// ｴﾗｰ種別
		Err_work.Errcod = 255;											// ｴﾗｰｺｰﾄﾞ
		Err_work.Errdtc = 2;											// ｴﾗｰ発生/解除
		Err_work.Errlev = 5;											// ｴﾗｰﾚﾍﾞﾙ
		Err_work.ErrDoor = 0;											// ﾄﾞｱ閉状態とする
		Err_work.Errinf = 0;											// ｴﾗｰ情報有無
		max = Ope_Log_GetLogMaxCount(eLOG_ERROR) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_ERROR);										// ｴﾗｰﾛｸﾞ登録
			taskchg(IDLETSKNO);
		}
	}
	
	// アラームデータの作成
	if(target.alarm) {
		memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
		Arm_work.Armsyu = 255;											// ｱﾗｰﾑ種別
		Arm_work.Armcod = 255;											// ｱﾗｰﾑｺｰﾄﾞ
		Arm_work.Armdtc = 2;											// ｱﾗｰﾑ発生/解除
		Arm_work.Armlev = 5;											// ｱﾗｰﾑﾚﾍﾞﾙ
		Arm_work.ArmDoor = 0;											// ﾄﾞｱ閉状態とする
		Arm_work.Arminf = 0;											// ｱﾗｰﾑ情報有無
		max = Ope_Log_GetLogMaxCount(eLOG_ALARM) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_ALARM);										// ｱﾗｰﾑﾛｸﾞ登録
			taskchg(IDLETSKNO);
		}
	}
	
	// 操作モニタデータ
	if(target.operation) {
		max = Ope_Log_GetLogMaxCount(eLOG_OPERATE) - 1;
		for(i = 0; i < max; ++i) {
			wopelg(OPLOG_DOOROPEN, 0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// モニタデータ
	if(target.monitor) {
		max = Ope_Log_GetLogMaxCount(eLOG_MONITOR) - 1;
		for(i = 0; i < max; ++i) {
			wmonlg(OPMON_DOOROPEN, 0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// コイン金庫集計データの作成
	if(target.coinbox) {
		max = Ope_Log_GetLogMaxCount(eLOG_COINBOX) - 1;
		for(i = 0; i < max; ++i) {
			kinko_syu(0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// 紙幣金庫集計データの作成
	if(target.notebox) {
		max = Ope_Log_GetLogMaxCount(eLOG_NOTEBOX) - 1;
		for(i = 0; i < max; ++i) {
			kinko_syu(1, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// 釣銭管理データの作成
	if(target.turikan) {
		max = Ope_Log_GetLogMaxCount(eLOG_MNYMNG_SRAM) - 1;
		for(i = 0; i < max; ++i) {
			turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// 機械№
			turi_kan.Kakari_no = OPECTL.Kakari_Num;								// 係員番号set
			memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// 現在時刻
			turikan_subtube_set();
			CountGet( TURIKAN_COUNT, &turi_kan.Oiban );							// 追番
			CountUp(TURIKAN_COUNT);
			turikan_clr();
			taskchg(IDLETSKNO);
		}
	}
}
#endif	// TEST_LOGFULL
// テスト用ログ作成(E)
// MH810103 GG119202 (s) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
//[]----------------------------------------------------------------------[]
///	@brief			モニタデータ送信対象の判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		MonKind   : 種別
///	@param[in]		MonCode   : コード
///	@return			ret       : 0:送信対象外/1:送信対象
///	@attention		
///	@note			
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar chk_mon_send_ntnet( uchar MonKind,uchar MonCode)
{
	const t_NTNET_MonitorCtrl	*MonCtrl;
	// モニタ情報取得
	MonCtrl = _getMonLogInfo(MonKind, MonCode);
	if(MonCtrl && (MonCtrl->Operation &LOGOPERATION_COM) ){
		// 送信属性あり
		return 1;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			操作モニタデータ送信対象の判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		OpeMonKind: 種別
///	@param[in]		OpeMonCode: コード
///	@return			ret       : 0:送信対象外/1:送信対象
///	@attention		
///	@note			
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar chk_opemon_send_ntnet( uchar OpeMonKind,uchar OpeMonCode)
{
	const t_NTNET_MonitorCtrl	*MonCtrl;
	// 操作モニタ情報取得
	MonCtrl = _getOpeLogInfo(OpeMonKind, OpeMonCode);
	if(MonCtrl && (MonCtrl->Operation &LOGOPERATION_COM) ){
		// 送信属性あり
		return 1;
	}
	return 0;
}
// MH810103 GG119202 (e) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
