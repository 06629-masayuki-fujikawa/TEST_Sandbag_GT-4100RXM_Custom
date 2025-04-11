/*[]-----------------------------------------------------------------------[]*/
/*| Backup/Restore, Download/Upload common subroutine                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005/06/24                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stddef.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"irq1.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"suica_def.h"
#include	"mnt_def.h"

/*--------------------------------------------------*/
/*	定数定義										*/
/*--------------------------------------------------*/
#define	ITEM_MAX_1LINE	100						/* CSVファイル 1行にセットする最大Item数 */
#define	ITEM_MAX_LOCK_INFO	LOCK_MAX			/* 車室データ最大数 */
#define	SEPARATOR_COMMA		0					/* セパレータとしてカンマを使用 */
#define	SEPARATOR_NEW_LINE	1					/* セパレータとして改行（CRLF）を使用 */

/*--------------------------------------------------*/
/*	構造体定義										*/
/*--------------------------------------------------*/

/*	車室データUpload/Download用 制御データ	*/

typedef struct {			/* BIN→CSV変換処理の保存データ */
	ushort		CallCount;			/* Callされた回数								*/
} t_AppServ_Bin_toCsv_LockInfo;

typedef struct {			/* CSV→BIN変換処理の保存データ */
	t_LockInfo	*lock_info_image;	/* バイナリイメージ保存エリアへのポインタ 		*/
									/* (初期化時に要求元から渡される)				*/
	ulong		image_size;			/* lock_info_imageエリアのサイズ 				*/
									/* (初期化時に要求元から渡される)				*/
	ushort		CallCount;			/* Callされた回数								*/
} t_AppServ_Csv_toBin_LockInfo;

/*	車室情報（FLAPDT）Upload/Download用 制御データ	*/

typedef struct {					/* CSV→BIN変換処理の保存データ */
	flp_rec	*	flapdt_image;		/* バイナリイメージ保存エリアへのポインタ 		*/
									/* (初期化時に要求元から渡される)				*/
	ulong		image_size;			/* lock_info_imageエリアのサイズ 				*/
									/* (初期化時に要求元から渡される)				*/
	ushort		CallCount;			/* Callされた回数								*/
} t_AppServ_Csv_toBin_FLAPDT;

/*--------------------------------------------------*/
/*	エリア定義 (backuped)							*/
/*--------------------------------------------------*/

extern	t_AppServ_LockInfoFukuden	AppServ_LockInfoFukuden;

/*--------------------------------------------------*/
/*	エリア定義										*/
/*--------------------------------------------------*/
#pragma	section				/* section "B": Initialized data area in external RAM1 */
							/* sys\mendata.c の "B" sectionに配置してください。OPECTL などがある場所です */

t_AppServ_BinCsvSave	AppServ_BinCsvSave;		/* パラメータ Upload/Download用 保存データ */

t_AppServ_Bin_toCsv_LockInfo	AppServ_B2C_LockInfo;
t_AppServ_Csv_toBin_LockInfo	AppServ_C2B_LockInfo;

ulong AppServ_LogWriteCompleteCheckParam[LOG_STRAGEN];	// LOG Flash書込み完了チェック用 パラメータ */

// 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ条件変更
ushort	AppServ_PParam_Copy;					// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無
												//	OFF:個別ﾊﾟﾗﾒｰﾀをｺﾋﾟｰしない
												//	ON :個別ﾊﾟﾗﾒｰﾀをｺﾋﾟｰする

extern	char	FLT_f_TaskStart;				/* FlashROMタスク起動フラグ */
												/* 0=起動前、1=起動中		*/
t_AppServ_Csv_toBin_FLAPDT		AppServ_CtoB_FLAPDT;

/*--------------------------------------------------*/
/*	テーブル定義									*/
/*--------------------------------------------------*/
#pragma	section				/* section "C" */

/* 共通パラメータ セクション毎アイテム数テーブル */
const	ushort	AppServ_CParamSizeTbl[C_PRM_SESCNT_MAX]={
	            1, C_PRM_SES01+1, C_PRM_SES02+1, C_PRM_SES03+1, C_PRM_SES04+1,
	C_PRM_SES05+1, C_PRM_SES06+1, C_PRM_SES07+1, C_PRM_SES08+1, C_PRM_SES09+1,
	C_PRM_SES10+1, C_PRM_SES11+1, C_PRM_SES12+1, C_PRM_SES13+1, C_PRM_SES14+1,
	C_PRM_SES15+1, C_PRM_SES16+1, C_PRM_SES17+1, C_PRM_SES18+1, C_PRM_SES19+1,
	C_PRM_SES20+1, C_PRM_SES21+1, C_PRM_SES22+1, C_PRM_SES23+1, C_PRM_SES24+1,
	C_PRM_SES25+1, C_PRM_SES26+1, C_PRM_SES27+1, C_PRM_SES28+1, C_PRM_SES29+1,
	C_PRM_SES30+1, C_PRM_SES31+1, C_PRM_SES32+1, C_PRM_SES33+1, C_PRM_SES34+1,
	C_PRM_SES35+1, C_PRM_SES36+1, C_PRM_SES37+1, C_PRM_SES38+1, C_PRM_SES39+1
	,		C_PRM_SES40+1, C_PRM_SES41+1
	,	C_PRM_SES42+1, C_PRM_SES43+1, C_PRM_SES44+1, C_PRM_SES45+1, C_PRM_SES46+1
	,	C_PRM_SES47+1, C_PRM_SES48+1, C_PRM_SES49+1, C_PRM_SES50+1, C_PRM_SES51+1
	,	C_PRM_SES52+1, C_PRM_SES53+1, C_PRM_SES54+1, C_PRM_SES55+1, C_PRM_SES56+1
	,	C_PRM_SES57+1, C_PRM_SES58+1, C_PRM_SES59+1, C_PRM_SES60+1 
};

/* 個別パラメータ セクション毎アイテム数テーブル */
static	const	ushort	AppServ_PParamSizeTbl[P_PRM_SESCNT_MAX]={
	1,			P_PRM_SES01+1,	P_PRM_SES02+1,	P_PRM_SES03+1
};

/* バックアップデータテーブル */
#define	BUP_BACKUP_DATA_COUNT	OPE_DTNUM_MAX			/* バックアップデータ数 */

	/** データ順は不動のこと。この順番は "OPE_DTNUM_xxxx" に準拠 **/
static	const	t_AppServ_AreaInfo	AppServ_BupAreaInfoTbl[BUP_BACKUP_DATA_COUNT]={
	{ (void*)0L				, 0L 					},	// 共通パラメータ (セットしない)
	{ (void*)0L				, 0L 					},	// 個別パラメータ (セットしない)
	{ (void*)&LockInfo[0]	, sizeof(LockInfo) 		},	// 車室設定
	{ (void*)&LockMaker[0]	, sizeof(LockMaker) 	},	// ロック装置設定
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//	{ (void*)&pas_tbl[0]	, sizeof(pas_tbl) 		},	// 定期テーブル	
	{ (void*)0L				, 0L 					},	// 定期テーブル	
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
	{ (void*)&FLAPDT		, sizeof(FLAPDT) 		},	// 入庫（駐車位置）情報
	{ (void*)&Mov_cnt_dat	, sizeof(Mov_cnt_dat) 	},	// 動作カウント
	{ (void*)&SFV_DAT		, sizeof(SFV_DAT)		},	// NT-NETデータ
	{ (void*)&coin_syu		, sizeof(coin_syu) 		},	// コイン金庫集計
	{ (void*)&note_syu		, sizeof(note_syu) 		},	// 紙幣金庫集計
	{ (void*)&turi_kan		, sizeof(turi_kan) 		},	// 金銭管理
	{ (void*)&sky			, sizeof(sky) 			},	// 集計（T,GT,複数台）
	{ (void*)&skybk			, sizeof(skybk) 		},	// 前回集計
	{ (void*)&loktl			, sizeof(loktl) 		},	// 駐車位置別集計
	{ (void*)&loktlbk		, sizeof(loktlbk) 		},	// 前回駐車位置別集計
	{ (void*)&CarCount		, sizeof(CarCount) 		},	// 入出庫台数
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//	{ (void*)&pas_renewal[0], sizeof(pas_renewal) 	},	// 定期更新テーブル	
	{ (void*)0L				, 0L 					},	// 定期更新テーブル	
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
	{ (void*)&cobk_syu		, sizeof(cobk_syu) 		},	// 前回コイン金庫合計
	{ (void*)&nobk_syu		, sizeof(nobk_syu) 		},	// 前回紙幣金庫合計
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//	{ (void*)&cre_saleng	, sizeof(cre_saleng) 	},	// クレジット売上拒否データ
//	{ (void*)&cre_uriage	, sizeof(cre_uriage) 	},	// クレジット売上依頼データ
	{ (void*)0L				, 0L					},	// クレジット売上拒否データ
	{ (void*)0L				, 0L					},	// クレジット売上依頼データ
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
	{ (void*)0L				, 0L					},	// Mifare書込み失敗ｶｰﾄﾞﾃﾞｰﾀ
	{ (void*)&SUICA_LOG_REC_FOR_ERR		, sizeof(SUICA_LOG_REC_FOR_ERR) },	// 決済異常が発生した時のログ情報
// MH810103 GG119202(S) 個別パラメータがリストアされない
//	{ (void*)&Syuukei_sp	, sizeof(Syuukei_sp)	},	// Edy,Suica集計情報
	{ (void*)0L				, 0L					},	// Edy,Suica集計情報
// MH810103 GG119202(E) 個別パラメータがリストアされない
	{ (void*)&Attend_Invalid_table	, sizeof(Attend_Invalid_table)},	// 係員無効ﾃﾞｰﾀ
	{ (void*)0L				, 0L 					},	// 入出庫ログ
	{ (void*)bk_LockInfo	, sizeof(bk_LockInfo) 	},	// 車室設定バックアップエリア（車室故障用）
	{ (void*)&SetDiffLogBuff, sizeof(SetDiffLogBuff)},	// 設定更新履歴
	{ (void*)&LongTermParkingPrevTime, sizeof(LongTermParkingPrevTime)},	// 長期駐車データ前回発報時間
	{ (void*)&Rau_SeqNo		, sizeof(Rau_SeqNo)		},	// センター追番
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//	{ (void*)&cre_slipno, sizeof(cre_slipno)},			// クレジット端末処理通番
	{ (void*)0L				, 0L					},	// クレジット端末処理通番
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	{ (void*)&Date_Syoukei, sizeof(Date_Syoukei)},		//日付切替基準の小計(リアルタイムデータ用)
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
	{ (void*)&EcEdyTerminalNo[0], sizeof(EcEdyTerminalNo)},	// Edy上位端末ID
// MH810103 GG119202(S) 個別パラメータがリストアされない
//	{ (void*)&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT)},	// 決済リーダ アラーム取引ログ
	{ (void*)0L				, 0L					},	// 決済リーダ アラーム取引ログ
// MH810103 GG119202(E) 個別パラメータがリストアされない
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
	{ (void*)&DC_SeqNo		, sizeof(DC_SeqNo)		}, // DC-NET通信用センター追番
	{ (void*)&REAL_SeqNo	, sizeof(REAL_SeqNo)	}, // リアルタイム通信用センター追番
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
// MH810103 GG119202(S) 個別パラメータがリストアされない
	{ (void*)&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT)},	// 決済リーダ アラーム取引ログ
// MH810103 GG119202(E) 個別パラメータがリストアされない
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
	{ (void*)&RecvBrandTbl[0], sizeof(RecvBrandTbl)	},	// 決済リーダから受信したブランドテーブル
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810103 GG119202(S) 個別パラメータがリストアされない
	{ (void*)&Syuukei_sp	, sizeof(Syuukei_sp)	},	// SX,決済リーダ集計情報
// MH810103 GG119202(E) 個別パラメータがリストアされない
};

/* 対象データFROM保存領域情報 */
/** データ順は不動のこと。この順番は "OPE_DTNUM_xxxx" に準拠 **/
static	const	t_AppServ_FromInfo	AppServ_BupAreaFromInfoTbl[BUP_BACKUP_DATA_COUNT]={
// NOTE:従来のバックアップは指定したFROMのエリアに対象のRAM領域を一気にコピーする方式であるが
// FT-4000の場合、シリアルFROMで256Byte単位の書き込しかできない仕様のためRAM領域ごとにセクタ(アドレス)を設定している
// アドレス
	0                  ,	// 共通パラメータ (セットしない)
	0                  ,	// 個別パラメータ (セットしない)
	FLT_BACKUP_SECTOR1 ,	// 車室設定
	FLT_BACKUP_SECTOR2 ,	// ロック装置設定
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//	FLT_BACKUP_SECTOR3 ,	// 定期テーブル	
	0                  ,	// 定期テーブル	
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
	FLT_BACKUP_SECTOR4 ,	// 入庫（駐車位置）情報
	FLT_BACKUP_SECTOR5 ,	// 動作カウント
	FLT_BACKUP_SECTOR6 ,	// NT-NETデータ
	FLT_BACKUP_SECTOR7 ,	// コイン金庫集計
	FLT_BACKUP_SECTOR8 ,	// 紙幣金庫集計
	FLT_BACKUP_SECTOR9 ,	// 金銭管理
	FLT_BACKUP_SECTOR10,	// 集計（T,GT,複数台）
	FLT_BACKUP_SECTOR11,	// 前回集計
	FLT_BACKUP_SECTOR12,	// 駐車位置別集計
	FLT_BACKUP_SECTOR13,	// 前回駐車位置別集計
	FLT_BACKUP_SECTOR14,	// 入出庫台数
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//	FLT_BACKUP_SECTOR15,	// 定期更新テーブル	
	0                  ,	// 定期更新テーブル	
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
	FLT_BACKUP_SECTOR16,	// 前回コイン金庫合計
	FLT_BACKUP_SECTOR17,	// 前回紙幣金庫合計
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//	FLT_BACKUP_SECTOR18,	// クレジット取消情報ログ
//	FLT_BACKUP_SECTOR19,	// クレジット売上収集チェックエリア
	0                  ,	// クレジット取消情報ログ
	0                  ,	// クレジット売上収集チェックエリア
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
	0                  ,	// Mifare書込み失敗ｶｰﾄﾞﾃﾞｰﾀ
	FLT_BACKUP_SECTOR28,	// 決済異常が発生した時のログ情報
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//	FLT_BACKUP_SECTOR20,	// Edy,Suica集計情報
	0                  ,	// Edy,Suica集計情報
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
	FLT_BACKUP_SECTOR21,	// 係員無効ﾃﾞｰﾀ
	0                  ,	// 入出庫ログ
	FLT_BACKUP_SECTOR23,	// 車室設定バックアップエリア（車室故障用）
	FLT_BACKUP_SECTOR24,	// 設定更新履歴
	FLT_BACKUP_SECTOR25,	// 長期駐車データ前回発報時間
	FLT_BACKUP_SECTOR29,	// センター追番
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//	FLT_BACKUP_SECTOR30,	// クレジット端末処理通番
	0                  ,	// クレジット端末処理通番
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	FLT_BACKUP_SECTOR31,	//日付切替基準の小計(リアルタイムデータ用)
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
	FLT_BACKUP_SECTOR32,	// Edy上位端末ID
// MH810103 GG119202(S) 個別パラメータがリストアされない
//	FLT_BACKUP_SECTOR33,	// 決済リーダ アラーム取引ログ
	0					,	// 決済リーダ アラーム取引ログ
// MH810103 GG119202(S) 個別パラメータがリストアされない
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
	FLT_BACKUP_SECTOR34,	// DC-NET通信用センター追番
	FLT_BACKUP_SECTOR35,	// リアルタイム通信用センター追番
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
// MH810103 GG119202(S) 個別パラメータがリストアされない
	FLT_BACKUP_SECTOR36,	// 決済リーダ アラーム取引ログ
// MH810103 GG119202(S) 個別パラメータがリストアされない
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
	FLT_BACKUP_SECTOR37,	// 決済リーダから受信したブランドテーブル
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810103 GG119202(S) 個別パラメータがリストアされない
	FLT_BACKUP_SECTOR38,	// SX,決済リーダ集計情報
// MH810103 GG119202(E) 個別パラメータがリストアされない
};

/* SUM算出対象データ情報 */
/** データ順は不動のこと。この順番は "OPE_DTNUM_xxxx" に準拠 **/
static	const	t_AppServ_AreaInfo	AppServ_SumAreaInfoTbl[BUP_BACKUP_DATA_COUNT]={
	{ (void*)&CParam.CParam01[0]	, sizeof(CParam)-sizeof(CParam.CParam00)	},	// 共通パラメータ
	{ (void*)0L				, 0L 					},	// 個別パラメータ
	{ (void*)&LockInfo[0]	, sizeof(LockInfo) 		},	// 車室設定
	{ (void*)0L				, 0L 					},	// ロック装置設定
	{ (void*)0L				, 0L 					},	// 定期テーブル	
	{ (void*)0L				, 0L 					},	// 入庫（駐車位置）情報
	{ (void*)0L				, 0L 					},	// 動作カウント
	{ (void*)0L				, 0L 					},	// NT-NETデータ
	{ (void*)0L				, 0L 					},	// コイン金庫集計
	{ (void*)0L				, 0L 					},	// 紙幣金庫集計
	{ (void*)0L				, 0L 					},	// 金銭管理
	{ (void*)0L				, 0L 					},	// 集計（T,GT,複数台）
	{ (void*)0L				, 0L 					},	// 前回集計
	{ (void*)0L				, 0L 					},	// 駐車位置別集計
	{ (void*)0L				, 0L 					},	// 前回駐車位置別集計
	{ (void*)0L				, 0L					},	// 入出庫台数
	{ (void*)0L				, 0L					},	// 定期更新テーブル
	{ (void*)0L				, 0L					},	// 前回コイン金庫合計
	{ (void*)0L				, 0L					},	// 前回紙幣金庫合計
	{ (void*)0L				, 0L					},	// クレジット取消情報ログ
	{ (void*)0L				, 0L					},	// クレジット売上収集チェックエリア
	{ (void*)0L				, 0L					},	// Mifare書込み失敗ｶｰﾄﾞﾃﾞｰﾀ
	{ (void*)0L				, 0L					},	// Suica異常ログ
	{ (void*)0L				, 0L					},	// Edy,Suica集計情報
	{ (void*)0L				, 0L					},	// 係員無効ﾃﾞｰﾀ
	{ (void*)0L				, 0L					},	// 入出庫ログ
	{ (void*)0L				, 0L					},	// 車室設定バックアップエリア（車室故障用）
	{ (void*)0L				, 0L					},	// 設定更新履歴
	{ (void*)0L				, 0L					},	// 長期駐車データ前回発報時間
	{ (void*)0L				, 0L					},	// センター追番
	{ (void*)0L				, 0L					},	// クレジット端末処理通番
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	{ (void*)0L				, 0L					},	//日付切替基準の小計(リアルタイムデータ用)
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
	{ (void*)0L				, 0L					},	// Edy上位端末ID
	{ (void*)0L				, 0L					},	// 決済リーダ アラーム取引ログ
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
	{ (void*)0L				, 0L					},	// DC-NET通信用センター追番
	{ (void*)0L				, 0L					},	// リアルタイム通信用センター追番
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
// MH810103 GG119202(S) 個別パラメータがリストアされない
	{ (void*)0L				, 0L					},	// 決済リーダ アラーム取引ログ
// MH810103 GG119202(E) 個別パラメータがリストアされない
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
	{ (void*)0L				, 0L					},	// 決済リーダから受信したブランドテーブル
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810103 GG119202(S) 個別パラメータがリストアされない
	{ (void*)0L				, 0L					},	// SX,決済リーダ集計情報
// MH810103 GG119202(E) 個別パラメータがリストアされない
};

/*--------------------------------------------------*/
/*	内部関数プロトタイプ定義						*/
/*--------------------------------------------------*/
#pragma	section				/* section "P" */

void	AppServ_CnvParam2CSV_Sub1( char *csvdata, ushort *csvdata_len );
void	AppServ_CnvParam2CSV_Sub2( char *csvdata, ushort *csvdata_len,
								   uchar ParamKind, ushort SectionNum, ushort ItemNum );
uchar	AppServ_CnvParam2CSV_Sub3( char *csvdata, ushort *csvdata_len );

ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info );
uchar	AppServ_CnvCsvParam_1ColumnRead( const char *CsvData, ulong *pBinData, const char **pNextCsvData );
uchar	AppServ_CnvCsvParam_SecItemNumGet( const char *CsvData, uchar *pCParam, ushort *pSecNum, ushort *pItemNum );
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)
//ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum );
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)

uchar	AppServ_CnvLockInfo2CSV_SUB(ulong param, char *buf, uchar separator);
#define	_appserv_CheckVal(data, type, member)	appserv_CheckVal(data, sizeof(((type*)0)->member))

int		appserv_CheckVal(ulong data, size_t size);

void	AppServ_ConvErrArmCSV_Sub1		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvErrArmCSV_Sub2		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
uchar	AppServ_ConvErrArmCSV_Sub3		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvErrDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvArmDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvMoniDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvOpeMoniDataCreate	( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	OccurReleaseDataEdit			( char *, ulong *, uchar , t_AppServ_Bin_toCsv_Param * );
void	TimeDataEdit					( char *, ulong *, date_time_rec * );
void	LevelDataEdit					( char *, ulong *, uchar );
uchar	DigitNumCheck					( long );
ulong	DataSizeCheck					( char*, uchar );
void	AppServ_ConvCarInfoDataCreate	( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
uchar	AppServ_CnvCsvFLAPDT_1ColumnRead( const char *CsvData, long *pBinData, const char **pNextCsvData );
void	AppServ_ConvChk_ResultCSV_Sub1( char *csvdata, ulong *csvdata_len );
void	AppServ_ConvChk_ResultCSV_Sub2( char *csvdata, ulong *csvdata_len,ushort SectionNum, ushort ItemNum );
uchar	AppServ_ConvChk_ResultCSV_Sub3( char *csvdata, ulong *csvdata_len );


/*[]-----------------------------------------------------------------------[]*/
/*|	共通・個別パラメータ 情報テーブル作成									|*/
/*|																			|*/
/*|	パラメータデータバックアップ用情報そして使用するデータの作成。			|*/
/*|	パラメータのセクション数とセクション毎のアイテム数をセットする。		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : データ格納エリアへのポインタ								|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	装置起動後 本関数Call前に prm_init() が Callされていること				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_MakeParamInfoTable( t_AppServ_ParamInfoTbl *pTable )
{
	ushort	i;

	/* 有効セクション数 set */
	pTable->csection_num = C_PRM_SESCNT_MAX;
	pTable->psection_num = P_PRM_SESCNT_MAX;

	/** section data top address & item count in section set **/

	/* 共通パラメータ */
	for( i=0; i< C_PRM_SESCNT_MAX; ++i ){
		pTable->csection[i].address  = CPrmSS[i];
		pTable->csection[i].item_num = AppServ_CParamSizeTbl[i];
	}

	/* 個別パラメータ */
	for( i=0; i< P_PRM_SESCNT_MAX; ++i ){
		pTable->psection[i].address  = PPrmSS[i];
		pTable->psection[i].item_num = AppServ_PParamSizeTbl[i];
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	バックアップデータ情報テーブル作成										|*/
/*|																			|*/
/*|	その他データバックアップ用情報そして使用するデータの作成。				|*/
/*|	データ毎の先頭アドレスとレングスをセットする。							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : データ格納エリアへのポインタ								|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_MakeBackupDataInfoTable( t_AppServ_AreaInfoTbl *pTable )
{
	ushort	i;
	pTable->area_num = BUP_BACKUP_DATA_COUNT;				/* area count set */

	for( i=0; i<BUP_BACKUP_DATA_COUNT; ++i ){
		pTable->area[i].address = AppServ_BupAreaInfoTbl[i].address;
		pTable->area[i].size    = AppServ_BupAreaInfoTbl[i].size;
		pTable->from_area[i].from_address = AppServ_BupAreaFromInfoTbl[i].from_address;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	ヘッダ／フッタ送信データフォーマット作成								|*/
/*|																			|*/
/*|	PCからダウンロードされたデータをFlashROM格納形式に変換する。			|*/
/*|	文字データはASCII or Shift-JISに対応。Unicodeは不可。					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = 変換後データ格納エリアへのポインタ			|*/
/*|				   text = 変換元データ(PCから受信したデータ)へのポインタ	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_FormatHeaderFooter( char formated[4][36], const char *text, ushort text_len )
{
	ushort	Line;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', 4*36 );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* 行数Maxチェック */
			if( 4 <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* 文字数Maxチェック */
			if( 36 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	ヘッダ／フッタ送信データフォーマット作成								|*/
/*|																			|*/
/*|	精算機内データに改行コード(CRLF)を挿入する								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = 改行挿入後データ格納エリアへのポインタ		|*/
/*|				   text = 改行挿入元データ(精算機内データ)へのポインタ		|*/
/*| RETURN VALUE : 変換後のデータ長										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : machida													|*/
/*| Date         : 2005/06/30												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToHeaderFooter( char *inserted, const char text[4][36] )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < 4; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 36);
		len += 36;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	
	return len;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（初期化）				|*/
/*|																			|*/
/*|	RAM内データをCSV形式値に変換する。このための前処理。					|*/
/*|	findfirst, findnextのように最初に本関数、以降はAppServ_CnvParam2CSV()で	|*/
/*|	順次読出しを行う。														|*/
/*|	※※ datasizeには1112byte以上を用意すること。							|*/
/*|		 AppServ_CnvParam2CSV()でCSV一行分のデータを（100アイテム分）を		|*/
/*|		 セットするため。													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = 共通・個別パラメータデータの情報テーブル	|*/
/*|								へのポインタ								|*/
/*|				   datasize = AppServ_CnvParam2CSV()で一回にセット可能な	|*/
/*|							  データサイズ。呼び元モジュールのバッファサイズ|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (ワークエリアサイズが小さい)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvParam2CSV_Init( const t_AppServ_ParamInfoTbl *param_info, ulong datasize )
{
	#define	WORK_BUF_MIN_LENGTH		1112

	if( WORK_BUF_MIN_LENGTH > datasize ){		/* work area size error (Y) */
		return	(uchar)0;
	}

	/* 受信データ保存 */
	memcpy( &AppServ_BinCsvSave.param_info, param_info, sizeof(t_AppServ_ParamInfoTbl) );
	AppServ_BinCsvSave.Bin_toCsv_Param.datasize = datasize;

	/* 作業エリアクリア */
	AppServ_BinCsvSave.Bin_toCsv_Param.Phase	 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind		 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Section	 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Item		 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.CallCount = 0;

	return	(uchar)1;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（読出し）				|*/
/*|																			|*/
/*|	RAM内データをCSV形式値に変換し、バッファにセットする。					|*/
/*|	データは100件 又は 1セクション分の小さい方となる。						|*/
/*|	本関数Call前に AppServ_ConvParam2CSV_Init() をCallすること。			|*/
/*|	最初から読み出しなおす場合も、初期化関数をCallすること。				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   finish = 全セクションの変換完了時 1をセット。以外は0。	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV( char *csvdata, ushort *csvdata_len, uchar *finish )
{
	ushort	TotalLen;
	uchar	ret;

	++AppServ_BinCsvSave.Bin_toCsv_Param.CallCount;			/* Callされた回数 +1 		*/
	*finish = 0;

	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Phase ){	/* 先頭行（見出し）送信フェーズ (Y)	*/
		AppServ_CnvParam2CSV_Sub1( csvdata, csvdata_len );	/* 見出し作成 */
		AppServ_BinCsvSave.Bin_toCsv_Param.Phase = 1;		/* 次フェーズはデータ部変換 */
		return;
	}

	/** データ部変換フェーズ **/

	TotalLen = 0;

	AppServ_CnvParam2CSV_Sub2( csvdata, &TotalLen, 			/* 先頭カラム見出し作成 */
									AppServ_BinCsvSave.Bin_toCsv_Param.Kind,
									AppServ_BinCsvSave.Bin_toCsv_Param.Section,
									(ushort)AppServ_BinCsvSave.Bin_toCsv_Param.Item );

	/* 1セクションセット完了 又は 書込みバッファサイズLimitまでデータセット */
	ret = AppServ_CnvParam2CSV_Sub3( csvdata, &TotalLen );
	*csvdata_len = TotalLen;
	if( ret == 1 ){											/* 終了 (Y) */
		*finish = 1;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub1）					|*/
/*|																			|*/
/*|	CSVファイルの１行目見出しを作成する。									|*/
/*|	サイズは493byteとなるが、書き込み用バッファは512byte以上であるので		|*/
/*|	一気に一行分をセットする。												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV_Sub1( char *csvdata, ushort *csvdata_len )
{
	ushort	i;
	ushort	SetLen;
	ushort	TotalLen;
	uchar	Asc[8];

	TotalLen = 0;
	Asc[0] = ',';
	Asc[1] = '[';

	for( i=0; i<100; ++i ){

		cnvdec4( (char*)&Asc[2], (long)i );						/* Bin to Ascii変換 */

		SetLen = 0;
		if( 10 > i ){										/* 数字１文字 (Y) */
			Asc[2] = Asc[5];
			Asc[3] = ']';
			SetLen = 4;
		}
		else{												/* 数字２文字 */
			Asc[2] = Asc[4];
			Asc[3] = Asc[5];
			Asc[4] = ']';
			SetLen = 5;
		}

		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)SetLen );
		TotalLen += SetLen;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;

	*csvdata_len = TotalLen + 2;
	return;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub2）					|*/
/*|																			|*/
/*|	各行の先頭（１カラム目）見出し作成処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   ParamKind = 0:共通， 1:個別								|*/
/*|				   SectionNum = セクション番号（1～99）						|*/
/*|				   ItemNum = 先頭のItem番号（1～9999）						|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV_Sub2( char *csvdata, ushort *csvdata_len,
								   uchar ParamKind, ushort SectionNum, ushort ItemNum )
{
	csvdata[0] = '[';

	if( 0 == ParamKind )
		csvdata[1] = 'C';
	else
		csvdata[1] = 'P';

	cnvdec2( &csvdata[2], (short)SectionNum );
	csvdata[4] = '-';
	cnvdec4( &csvdata[5], (long)ItemNum );
	csvdata[9] = ']';
	*csvdata_len = 10;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub3）					|*/
/*|																			|*/
/*|	データセット（CSV作成）処理												|*/
/*| 1行分のデータをセット。（１セクション終了 又は 100アイテム）			|*/
/*|	末尾にはCR,LFをセットする。(Max1112byteセットする)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   作業エリア（AppServ_BinCsvSave）の情報を使う。			|*/
/*				   更新も行う。												|*/
/*| RETURN VALUE : 1=1セクション終了。 0=書込みバッファLimitによる終了      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvParam2CSV_Sub3( char *csvdata, ushort *csvdata_len )
{
	ushort	TgtSection;				/* target Section number (0- ) */
	ushort	TgtTotalItemCount;		/* total Item count in target Section */

	ushort	TgtItemNum;				/* 次に作成する Item番号 (0- ) */
	ulong	*pTgtSectionTop;		/* Top item data address in target Section */
	ushort	TgtSectionCount;		/* Section count in target Section */

	ushort	TotalLen;				/* 書き込んだサイズ */
	ushort	SetItemCount;			/* 今回書き込んだItem数 */
	uchar	Asc[12];
	uchar	ThisItemLen;

	/* パラメータ情報テーブルから target部の情報抽出 */
	TgtSection	= AppServ_BinCsvSave.Bin_toCsv_Param.Section;
	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Kind ){		/* 共通パラメータ (Y) */
		TgtTotalItemCount = AppServ_BinCsvSave.param_info.csection[TgtSection].item_num;
		pTgtSectionTop    = AppServ_BinCsvSave.param_info.csection[TgtSection].address;
		TgtSectionCount   = AppServ_BinCsvSave.param_info.csection_num;
	}
	else{													/* 個別パラメータ */
		TgtTotalItemCount = AppServ_BinCsvSave.param_info.psection[TgtSection].item_num;
		pTgtSectionTop    = AppServ_BinCsvSave.param_info.psection[TgtSection].address;
		TgtSectionCount   = AppServ_BinCsvSave.param_info.psection_num;
	}

	TotalLen = *csvdata_len;
	SetItemCount = 0;										/* 今回書き込んだItem数クリア */
	TgtItemNum = AppServ_BinCsvSave.Bin_toCsv_Param.Item;	/* target Item number */

	/* 1セクションセット完了 又は 書込みバッファサイズLimitまで */
	while( (TgtItemNum < TgtTotalItemCount) && (SetItemCount < ITEM_MAX_1LINE) ){
		Asc[0] = ',';

		ThisItemLen = intoasl_0sup( &Asc[1], pTgtSectionTop[ TgtItemNum ], (unsigned short)10 );
							/* Long size Hex data change to Decimal Ascii (0 suppress) */
		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)(1+ThisItemLen) );
		TotalLen += (1+ThisItemLen);

		++SetItemCount;
		++TgtItemNum;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;
	TotalLen += 2;

	*csvdata_len = TotalLen;

	/* 終了要因判定 */
	if( TgtItemNum >= TgtTotalItemCount ){		/* 1セクションセット完了 */
		++AppServ_BinCsvSave.Bin_toCsv_Param.Section;			/* 次のセクションへ */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= 0;

		if( TgtSectionCount <= AppServ_BinCsvSave.Bin_toCsv_Param.Section ){
																/* 全セクション終了 (Y) */

			if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Kind ){	/* 共通パラメータ (Y) */
				return	1;		/* 全終了 */
			}
		}
	}

	else{	/* 途中で終了した場合は、Indexをセーブする */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= TgtItemNum;
	}
	return	0;
}


/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータダウンロード時のデータ変換（初期化）				|*/
/*|																			|*/
/*|	PCから受信したCSV形式データをRAM内保存形式に変換する。このための前処理。|*/
/*|	findfirst, findnextのように最初に本関数、以降はAppServ_CnvCSV2Param()で	|*/
/*|	順次読出しを行う。														|*/
/*|	※※ image_sizeは共通＆個別パラメータサイズ（+4byte）以上をセットすること。	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = 共通・個別パラメータデータの情報テーブル	|*/
/*|								へのポインタ								|*/
/*|				   param_image = 変換後のRAMイメージデータ格納エリアへの	|*/
/*|								 ポインタ									|*/
/*|								イメージデータは共通,個別の順にデータが並ぶ	|*/
/*|								様にセットする。							|*/
/*|								（CParam,PParamイメージを連続してセット）	|*/
/*|				   datasize = param_imageのサイズ							|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (イメージエリアサイズが小さい)					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvCSV2Param_Init( const t_AppServ_ParamInfoTbl *param_info, 
									char *param_image, ulong image_size )
{
	ulong	wkl;

	wkl = AppServ_ConvCsvParam_ImageAreaSizeGet( param_info );	/* RAMイメージエリアサイズget */
	if( image_size < wkl ){										/* 作業エリアが小さい (Y) */
		return	(uchar)0;
	}

	/* 受信データ保存 */
	memcpy( &AppServ_BinCsvSave.param_info, (const void*)param_info, sizeof(t_AppServ_ParamInfoTbl) );
	AppServ_BinCsvSave.Csv_toBin_Param.param_image = param_image;
	AppServ_BinCsvSave.Csv_toBin_Param.image_size = image_size;

	/* 作業エリアクリア */
	memset( param_image, 0, image_size );
	memcpy(param_image, &CParam, sizeof(CParam));
	AppServ_BinCsvSave.Bin_toCsv_Param.CallCount = 0;

	return	(uchar)1;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータ ダウンロードサブルーチン							|*/
/*|	RAM内イメージファイルのサイズ取得										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = 共通・個別パラメータデータの情報テーブル	|*/
/*|								へのポインタ								|*/
/*| RETURN VALUE : RAM上イメージファイルのサイズを返す                      |*/
/*|				   CParam00～PParamEndまでのサイズ							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info )
{
	return (ulong)(sizeof(CParam) + sizeof(PParam)) ;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータダウンロード時のデータ変換（読出し）				|*/
/*|																			|*/
/*|	PCから受信したCSV形式データをRAM内保存形式に変換する。					|*/
/*|	本関数Call前に AppServ_ConvCSV2Param_Init() をCallすること。			|*/
/*|	最初から読み出しなおす場合も、初期化関数をCallすること。				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PCから受信したCSV形式データ（１行分）			|*/
/*|							 へのポインタ									|*/
/*|				   csvdata_len = csvdataのサイズ							|*/
/*|				   finish = 全セクションの変換完了時 セットした総レングスを	|*/
/*|							セット。以外は0。								|*/
/*|				   ※末尾は CR 又は EOFをセットしてください。				|*/
/*| RETURN VALUE : 1=OK, 0=CSV形式NG                                        |*/
/*|					   セクション番号，アイテム数が範囲外(オーバー)チェック	|*/
/*|					   は、先頭カラムのコメントのみをチェック対象とし、		|*/
/*|					   以降アイテム数がオーバーした場合は、データは書き込ま	|*/
/*|					   ないだけで正常終了とする。							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCSV2Param(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	/** return code **/
	#define	BIT_DATA_EXIST	1								/* データありbit */
	#define	BIT_NEXT_EXIST	2								/* 次データあり */
	#define	BIT_COMMENT		4								/* コメントカラム */
	#define	BIT_ERROR		0x80							/* エラーあり */

	ushort	i;
	uchar	ret;
	ulong	BinData;
	const char	*pNextCsvData;
	ushort	SecNum, ItemNum;
	ushort	TgtSecTopIndex;			/* ワークエリア先頭からの総Index数 */
	ulong	*pTgtSecTop;			/* 目的セクターの先頭アイテムアドレス */
	char	f_CParam;				/* 1=共通パラメータが対象 */
	ushort	MaxSecNum;				/* セクション数 */
	ushort	MaxItemNum;				/* 該当セクションのアイテム数 */

	++AppServ_BinCsvSave.Bin_toCsv_Param.CallCount;			/* Callされた回数 +1 		*/
	*finish = 0;

	/* 1カラム変換 */
	ret = AppServ_CnvCsvParam_1ColumnRead( csvdata, &BinData, &pNextCsvData );

	if( (0 == (ret & BIT_DATA_EXIST)) && (0 == (ret & BIT_COMMENT)) ){
															/* データなし(いきなりカンマ:Y) */
		/* 先頭行と判断 */
		return	(uchar)1;									/* 終了 */
	}

	if( 0 == (ret & BIT_COMMENT) ){							/* コメントカラム(N) */
		*finish = 1;
		return	(uchar)0;									/* 異常終了 */
	}														/* 先頭がコメントでないのは形式異常 */

	if( 0 == (ret & BIT_NEXT_EXIST) ){						/* 次データなし(Y) */
		return	(uchar)1;									/* 終了 */
	}

	/* 先頭カラムから ターゲットのセクション番号と先頭アイテム番号を得る */
	if( BIT_ERROR == AppServ_CnvCsvParam_SecItemNumGet((const char*)csvdata, (uchar*)&f_CParam, &SecNum, &ItemNum) ){
		*finish = 1;
		return	(uchar)0;									/* 異常終了 */
	}														/* コメント形式異常 */

	/* セクション 及び アイテムの最大数取得 */
	if( 1 == f_CParam ){									/* 共通パラメータ (Y) */
		MaxSecNum = AppServ_BinCsvSave.param_info.csection_num;	/* Maxセクション数 get */
		MaxItemNum = AppServ_BinCsvSave.param_info.csection[SecNum].item_num;
															/* Maxアイテム数 get */
	}
	else{													/* 個別パラメータ */
		MaxSecNum = AppServ_BinCsvSave.param_info.psection_num;	/* Maxセクション数 get */
		MaxItemNum = AppServ_BinCsvSave.param_info.psection[SecNum].item_num;
															/* Maxアイテム数 get */
	}

	/* CSV先頭カラムの指定データ（セクション数，アイテム数）範囲外チェック */
	if( (SecNum >= MaxSecNum) || (ItemNum >= MaxItemNum) ){	/* 番号オーバー (Y) */
/* セクション数・アイテム数が異なった場合、データセットしないで正常終了 */
		return	(uchar)1;									/* 正常終了 */
	}

	/** 受信データをHex変換しワークエリアに格納 **/

	/* 目的セクターの先頭アイテムアドレスget */
	TgtSecTopIndex = AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( f_CParam, SecNum );
						/* パラメータエリア先頭から目的セクター先頭アイテムまでの総アイテム数get */
	pTgtSecTop = (ulong*)AppServ_BinCsvSave.Csv_toBin_Param.param_image;	/* work area top address get */
	pTgtSecTop += (TgtSecTopIndex + ItemNum);

	for( i=0; ; ++i ){
		/* アイテム数範囲外（オーバー）チェック */
		if( (ItemNum+i) >= MaxItemNum ){					/* アイテム数オーバー (Y) */
			break;											/* 処理終了(正常) */
		}

		ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
															/* 1カラムデータ変換 */
		if( 0 != (ret & (BIT_COMMENT | BIT_ERROR)) ){		/* エラーorコメント(Y) */
			*finish = 1;
			return	(uchar)0;								/* 異常終了 */
		}

		if( 0 != (ret & BIT_DATA_EXIST) ){					/* データあり (Y) */
			pTgtSecTop[i] = BinData;						/* ワークエリアにデータセット */
			prm_invalid_change( (short)SecNum, (short)(ItemNum+i), (long*)&pTgtSecTop[i] );
		}	/* データなし時は０とするが、初期化時に０クリアしているのでここでは何もしない */

		if( 0 == (ret & BIT_NEXT_EXIST) ){					/* 次データなし (Y) */
			break;											/* 正常終了 */
		}
	}
	return	(uchar)1;										/* normal 終了 */
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータダウンロード サブルーチン							|*/
/*|																			|*/
/*|	CSV形式データの１カラムをHEXデータに変換。								|*/
/*|	","(カンマ)で区切られたデータ１つの解析と変換を行う。					|*/
/*|	データがセクション番号を示すコメント行である場合は、その情報を返す。	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PCから受信したCSV形式データ					|*/
/*|				   pBinData = HEX変換値セットエリアへのポインタ				|*/
/*|							  return値=1時のみ有効							|*/
/*|				   pNextCsvData = 次データの先頭アドレス					|*/
/*| RETURN VALUE : 0=データなし、1=数値データ有り（pBinDataにセット）		|*/
/*|				   2=コメント行（変換なし）                                 |*/
/*|				   b0=1:BinDataあり（変換実施）								|*/
/*|				   b1=1:次データあり、0:最終データ							|*/
/*|				   b2=1:コメントカラム(先頭)、0:コメントでない				|*/
/*|				   b7=1:データエラーあり									|*/
/*|			（例）															|*/
/*|				   06h=コメントカラムのためデータなし  次データあり			|*/
/*|				   03h=数値データ有り(pBinDataにセット)次データあり			|*/
/*|				   01h=数値データ有り(pBinDataにセット)次データなし(最終)	|*/
/*|				   04h=コメントカラムのためデータなし  次データなし(最終)	|*/
/*|				   80h=形式エラー（変換なし）								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvParam_1ColumnRead( const char *CsvData, ulong *pBinData, const char **pNextCsvData )
{
	char	Asc[12];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret;
	ulong	BinData;

	ret = 0;

	for( i=0, j=0; j<11; ++i ){								/* 最大10文字取得するまで */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* スペースは読み飛ばし */
			continue;
		}

		if( ',' == TgtChar ){								/* 区切り文字(次あり) */
			*pNextCsvData = (char*)&CsvData[i+1];					/* 次アドレスセット */
			ret |= BIT_NEXT_EXIST;
			goto AppServ_CnvCsvParam_1ColumnRead_10;		/* 1カラム取り出し後 処理へ */
		}

		else if( (0x0d == TgtChar) ||						/* 行末尾コード */
				 (0x0a == TgtChar) ||
				 (0x1a == TgtChar) ){

AppServ_CnvCsvParam_1ColumnRead_10:
			/** 1カラム取り出し後 処理 **/
			if( 0 == j ){									/* データなし(Y) */
				//break;
			}
			else if( '[' == Asc[0] ){						/* コメント行(Y) */
				ret |= BIT_COMMENT;
				//break;
			}
			else{
				if( 1 == DecAsc_to_LongBin_withCheck((uchar*)&Asc[0], (uchar)j, &BinData) ){
															/* Ascii -> Hex変換 */
					*pBinData = BinData;					/* Hex data set */
					ret |= BIT_DATA_EXIST;					/* データあり */
				}
				else{										/* 元データ異常 */
					ret |= BIT_ERROR;
				}
				//break;
			}
			break;
		}

		else{												/* 終了コードではない */
			Asc[j++] = TgtChar;
			if( 10 < j ){									/* データ大きすぎerror */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータダウンロード サブルーチン							|*/
/*|																			|*/
/*|	CSV形式データの１カラム目のコメントからターゲットセクション番号と		|*/
/*|	アイテム番号を得る。													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PCから受信したCSV形式データ					|*/
/*|				   pCParam = 1:共通パラメータ、0=個別パラメータ				|*/
/*|				   pSecNum = セクター番号 (1～)								|*/
/*|				   pItemNum = アイテム番号 (0～)							|*/
/*| RETURN VALUE : 0=OK, 80=error											|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvParam_SecItemNumGet( const char *CsvData, uchar *pCParam, ushort *pSecNum, ushort *pItemNum )
{
	char	Asc[12];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret;
	ulong	BinData;

	ret = 0;
	memset(Asc,0,sizeof(Asc));

	for( i=0, j=0; j<11; ++i ){								/* 最大10文字取得するまで */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* スペースは読み飛ばし */
			continue;
		}

		if( ',' == TgtChar){								/* 区切り文字(Y) */

			/** 1カラム取り出し後 処理 **/
			/* データ形式は "[Css-iiii]" or "[Pss-iiii]" だけ受け付ける */
			if( ('[' != Asc[0]) || ('-' != Asc[4]) || (']' != Asc[9]) ){	/* 形式エラー(Y) */
				return	(uchar)BIT_ERROR;
			}

			if( 'C' == Asc[1] ){
				*pCParam = 1;
			}
			else if( 'P' == Asc[1] ){
				*pCParam = 0;
			}
			else{
				return	(uchar)BIT_ERROR;
			}

			/* セクション番号を得る */
			if( 0 == DeciAsc_to_LongBin_withCheck((uchar*)&Asc[2], (uchar)2, &BinData) ){
															/* 10進文字 -> Hex変換 */
				return	(uchar)BIT_ERROR;
			}
			*pSecNum = (ushort)BinData;

			/* 先頭アイテム番号を得る */
			if( 0 == DeciAsc_to_LongBin_withCheck((uchar*)&Asc[5], (uchar)4, &BinData) ){
															/* 10進文字 -> Hex変換 */
				return	(uchar)BIT_ERROR;
			}
			*pItemNum = (ushort)BinData;
			break;
		}

		else{												/* 終了コードではない */
			Asc[j++] = TgtChar;
			if( 10 < j ){									/* データ大きすぎerror */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータダウンロード サブルーチン							|*/
/*|																			|*/
/*|	パラメータエリア先頭から目的セクターの先頭Itemまでの総アイテム数取得	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : SecKind = 1:共通パラメータ、0:個別パラメータ				|*/
/*|				   SecNum = セクター番号 (0～)								|*/
/*| RETURN VALUE : パラメータエリア先頭からの総Item数						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum )
{
	ushort	SkipItemCount;
	ushort	i;

	SkipItemCount = 0;
	for( i=0; i<SecNum; ++i ){
		if( 1 == SecKind ){
			SkipItemCount += AppServ_BinCsvSave.param_info.csection[i].item_num;
		}else{
			SkipItemCount += AppServ_BinCsvSave.param_info.psection[i].item_num;
		}
	}

	if( 0 == SecKind ){					/* 個別パラメータ (Y) */

		/* 共通パラメータの全アイテム分をスキップ分として算出 */
		SecNum = AppServ_BinCsvSave.param_info.csection_num;	/* 共通パラメータセクション数get */
		for( i=0; i<SecNum; ++i ){
			SkipItemCount += AppServ_BinCsvSave.param_info.csection[i].item_num;
		}
	}
	return	SkipItemCount;
}

/*[]----------------------------------------------------------------------[]*/
/*| Calculate SUM (unsigned short毎の演算。結果も(unsigned short)	  	   |*/
/*|																		   |*/
/*|	本関数には以下の条件がある。必ず満足する条件でCallすること。		   |*/
/*|	① レングスは偶数長であること。										   |*/
/*| ② 演算元エリアの先頭アドレスは偶数番地であること。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : kind - SUM算出対象データ(OPE_DTNUM_XXX)				   |*/
/*| RETURN VALUE : none						                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 202006-02-20                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	DataSumUpdate( ushort kind )
{
	ushort 	Sum;
	union	{
		ushort	us[2];
		ulong	ul;
	} u;
	
	if (AppServ_SumAreaInfoTbl[kind].address != NULL) {
		Sum = sumcal_ushort((ushort*)AppServ_SumAreaInfoTbl[kind].address, 
						(ushort)AppServ_SumAreaInfoTbl[kind].size);	/* sum演算 */
		u.us[0] = Sum;
		u.us[1] = ~Sum;
		DataSum[kind].Len = AppServ_SumAreaInfoTbl[kind].size;
		DataSum[kind].Sum = u.ul;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| RAM上データが破損しているか否かのチェック			   |*/
/*|																		   |*/
/*| サムチェックを行い妥当性状況を戻す。								   |*/
/*|	本関数には以下の条件がある。必ず満足する条件でCallすること。		   |*/
/*|	① レングスは偶数長であること。										   |*/
/*| ② 演算元エリアの先頭アドレスは偶数番地であること。					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : kind - SUM算出対象データ(OPE_DTNUM_XXX)				   |*/
/*| RETURN VALUE : 1=OK, 0=NG(サムNG)							           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 202006-02-20                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	DataSumCheck( ushort kind )
{
	ushort 	Sum;
	union	{
		ushort	us[2];
		ulong	ul;
	} u;

	if (AppServ_SumAreaInfoTbl[kind].address == NULL) {
		return 0;	/* SUMを持たないデータ */
	}

	if (DataSum[kind].Len != AppServ_SumAreaInfoTbl[kind].size) {
		return 0;	/* Sum NG(データ長の相違) */
	}
	
	Sum = sumcal_ushort((ushort*)AppServ_SumAreaInfoTbl[kind].address, 
						(ushort)AppServ_SumAreaInfoTbl[kind].size);	/* sum演算 */

	u.ul = DataSum[kind].Sum;
	if( (u.us[0] == Sum) && (u.us[1] == (ushort)(~Sum)) ){
		return	(uchar)1;						/* OK */
	}
	return	(uchar)0;								/* Sum NG(データ破損) */
}

#define	PRMSUM_RAM_OK		0x0001
#define	PRMSUM_RAM_NG		0x0002
#define	PRMSUM_FLASH_OK		0x0010
#define	PRMSUM_FLASH_NG		0x0020
#define	PRMSUM_EQUAL		0x0100
#define	PRMSUM_NOT_EQUAL	0x0200
#define	PRM_COPY_FtoRAM		0x4000
/*[]-----------------------------------------------------------------------[]*/
/*|	FlashROM関連 復電処理													|*/
/*|																			|*/
/*|	復電とメモリークリアを併用した処理。									|*/
/*|	復電処理PhaseでopetaskからCallされるが、この時点でメモリークリアも実施	|*/
/*|	する。																	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : RAMCLR : 強制クリア指示フラグ (1=強制、0=強制でない)		|*/
/*|				   f_CPrmDefaultSet : 1=既に共通ﾊﾟﾗﾒｰﾀへdefaultｾｯﾄ(初期化)済み|*/
/*|				                      0=初期化していない					|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : machida.k												|*/
/*| Date         : 2005/07/26												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_FukudenProc( char RAMCLR, char f_CPrmDefaultSet )
{
	uchar	param_ok;
	ulong	ret;
	uchar	lockparam_ok;
	ulong	ver_flg;
	ushort	sntp_prm_after;		// SNTP同期時刻変更モニタログ登録用(設定34-0122の③④⑤⑥を保持)
	ushort	i;

	/* 内部ワークエリア初期化 */
	for (i = 0; i < LOG_STRAGEN; i++) {
		AppServ_LogWriteCompleteCheckParam[i] = 0xffffffff;	/* LOG Flash書込み完了チェック用 パラメータ */
	}

	FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC = 0;				// ログ登録予約なし
	FLAGPT.event_CtrlBitData.BIT.PARAM_INVALID_CHK = 1;			// 固定パラメータ変換が起動時の処理かどうかを判定する
/* フラッシュ読み書きモジュール初期化 */
	FLT_init(&param_ok, &lockparam_ok, &ver_flg, RAMCLR);

	if( f_ParaUpdate.BYTE != 0 ) {
		if( f_ParaUpdate.BIT.splday == 1 ) {
			UsMnt_datecheck(0);							// 特別日／特別期間の入力日付のチェック
		}
		if( f_ParaUpdate.BIT.tickvalid == 1 ) {
			UsMnt_datecheck(1);							// 券期限の入力日付のチェック
			memset( &tick_valid_data.tic_prm, 0, sizeof(TIC_PRM) );
		}

		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
		if( f_ParaUpdate.BIT.bpara == 1 ) {				// 共通パラメータ設定変更あり
			wopelg( OPLOG_KYOTUPARAWT, 0, 0 );			// 操作履歴登録
			sntp_prm_after = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);
			if (sntp_prm_before != sntp_prm_after) {
			// SNTP同期時刻設定値に変更があった場合、M8036登録
				wopelg( OPLOG_SET_SNTP_SYNCHROTIME, (ulong)sntp_prm_before, (ulong)sntp_prm_after);
			}
			SetSetDiff(SETDIFFLOG_SYU_SYSMNT);			// 設定更新履歴ログ登録
		}
		if( f_ParaUpdate.BIT.cpara == 1 ) {				// 車室パラメータ設定変更あり
			DataSumUpdate(OPE_DTNUM_LOCKINFO);			// SUM更新
			wopelg( OPLOG_SHASHITUPARAWT, 0, 0 );		// 操作履歴登録
		}
		f_ParaUpdate.BYTE = 0;
	}

	if(( TENKEY_F1 == 1 )&&( TENKEY_F2 == 1 )&&( TENKEY_F3 == 1 )){		// ﾛｯｸ装置状態ﾘｽﾄｱ要求(F1&F2&F3 ON)?
	/* FLAPDTリストア */
		if( !( FLT_Restore_FLAPDT() & 0xff000000 ) ){
			grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[66] );						// "       ﾛｯｸ装置状態ﾘｽﾄｱ        ",表示
		}
	}
	if (ver_flg) {
	/* バージョンアップ指定ありなので、デフォルト値セット後、全リストア実行 */
		log_init();
		RAMCLR = memclr( 1 );				// SRAM初期化（無条件ｸﾘｱ）
		prm_clr( PEC_PRM, 1, 0 );			// 個別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		prm_clr( COM_PRM, 1, 0 );			// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		lockinfo_clr(1);					// 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ
		ret = FLT_Restore(FLT_EXCLUSIVE);	// リストア実行
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
			DataSumUpdate(OPE_DTNUM_LOCKINFO);	/* update parameter sum on ram */
		}
	}

// 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ条件変更
	AppServ_PParam_Copy = OFF;							// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無＝ｺﾋﾟｰしない
	
/* パラメータデータ初期化処理 */
	PowonSts_Param = 0;			/* 起動時パラメータ状態セーブエリア初期化（チェック未実施）*/
	
	/* RAM上パラメータデータの有効/無効チェック */
	if (DataSumCheck(OPE_DTNUM_COMPARA) == 0) {
		PowonSts_Param |= PRMSUM_RAM_NG;
	}else{
		PowonSts_Param |= PRMSUM_RAM_OK;
	}
	
	/* FLASH上のパラメータデータ有効/無効チェック */
	if (param_ok) {
		PowonSts_Param |= PRMSUM_FLASH_OK;
		/** ﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝｱｯﾌﾟにより、共通ﾊﾟﾗﾒｰﾀのｻｲｽﾞが大きくなった場合の対策 	**/
		/** FlashROM内のｾｸｼｮﾝ数、各ｾｸｼｮﾝ内のｱｲﾃﾑ数が 現ﾌﾟﾛｸﾞﾗﾑ(SRAM)と異なる場合**/
		/** ① SRAMの共通ﾊﾟﾗﾒｰﾀｴﾘｱをﾃﾞﾌｫﾙﾄｸﾘｱする。								**/
		/** ② FlashROM内にある分のﾃﾞｰﾀをSRAMへ書き込む。						**/
		/**    ※この時 増加した分のﾃﾞｰﾀは更新しない。							**/
		/** ③ 新SRAM内容をMasterとし、FlashROMへﾃﾞｰﾀを書き込む。				**/
		/** ※ この処理はﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝｱｯﾌﾟし、共通ﾊﾟﾗﾒｰﾀのｻｲｽﾞが大きくなった	**/
		/**    場合のみ機能する。												**/
		/**    但しF2&F4，F3&F5押下起動時は強制ﾃﾞﾌｫﾙﾄを優先しFlash内容は廃棄する**/

		if( 2 == FLT_Comp_ComParaSize_FRom_to_SRam() ){		// ｾｸｼｮﾝ数 又は ｱｲﾃﾑ数が違う

			/* ﾊﾟﾗﾒｰﾀ初期化した場合はｾｸｼｮﾝ数が異なってもFlash→SRAMはしない */
			if( f_CPrmDefaultSet ){								// ﾊﾟﾗﾒｰﾀ初期化した
				(void)FLT_WriteParam1(FLT_EXCLUSIVE);			// RAM⇒FLASHへのパラメータデータ転送
			}
			else{
				prm_clr( COM_PRM, 1, 0 );						// 共通ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄset
				FLT_RestoreParam();								// FLASH⇒RAMへのパラメータデータ転送
				DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
																// 個別ﾊﾟﾗﾒｰﾀはｺﾋﾟｰしない（AppServ_PParam_Copy = OFF）
				(void)FLT_WriteParam1(FLT_EXCLUSIVE);			// RAM⇒FLASHへのパラメータデータ転送
			}
		}
	}
	else {
		PowonSts_Param |= PRMSUM_FLASH_NG;
	}
	
	if (PowonSts_Param & PRMSUM_RAM_OK) {
		/* RAM⇒FLASHへのパラメータデータ転送 */
		ret = FLT_WriteParam1(FLT_EXCLUSIVE);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			if ((PowonSts_Param & PRMSUM_RAM_OK)
				&& (PowonSts_Param & PRMSUM_FLASH_OK)) {
				if (_FLT_RtnDetail(ret) == FLT_PARAM_WRITTEN) {
					PowonSts_Param |= PRMSUM_NOT_EQUAL;	/* FLASH!=RAM */
				}
				else {
					PowonSts_Param |= PRMSUM_EQUAL;		/* FLASH==RAM */
				}
			}
		}
		else {
			PowonSts_Param &= ~(PRMSUM_FLASH_OK);
			PowonSts_Param |= PRMSUM_FLASH_NG;			/* 書込み失敗 */
		}
	}
	else if (PowonSts_Param & PRMSUM_FLASH_OK) {
		/* FLASH⇒RAMへのパラメータデータ転送 */
		FLT_RestoreParam();
		DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
		PowonSts_Param |= PRM_COPY_FtoRAM;	/* FLASH⇒RAMへのパラメータデータ転送 */
	}
	else {
		/* FLASH=NG && RAM=NG のケース */
		/* AMANO殿指示によりデフォルト値のセット等はせずにそのまま起動
		  (原則としてありえないケースとのこと) */
	}

/* 車室パラメーター初期化処理 */
	PowonSts_LockParam = 0;			/* 起動時パラメータ状態セーブエリア初期化（チェック未実施）*/
	
	/* RAM上パラメータデータの有効/無効チェック */
	if (DataSumCheck(OPE_DTNUM_LOCKINFO) == 0) {
		PowonSts_LockParam |= PRMSUM_RAM_NG;
	}else{
		PowonSts_LockParam |= PRMSUM_RAM_OK;
	}
	
	/* FLASH上のパラメータデータ有効/無効チェック */
	if (lockparam_ok) {
		PowonSts_LockParam |= PRMSUM_FLASH_OK;
	}
	else {
		PowonSts_LockParam |= PRMSUM_FLASH_NG;
	}
	
	if (PowonSts_LockParam & PRMSUM_RAM_OK) {
		/* RAM⇒FLASHへのパラメータデータ転送 */
		ret = FLT_WriteLockParam1(FLT_EXCLUSIVE);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			if ((PowonSts_LockParam & PRMSUM_RAM_OK)
				&& (PowonSts_LockParam & PRMSUM_FLASH_OK)) {
				if (_FLT_RtnDetail(ret) == FLT_PARAM_WRITTEN) {
					PowonSts_LockParam |= PRMSUM_NOT_EQUAL;	/* FLASH!=RAM */
				}
				else {
					PowonSts_LockParam |= PRMSUM_EQUAL;		/* FLASH==RAM */
				}
			}
		}
		else {
			PowonSts_LockParam &= ~(PRMSUM_FLASH_OK);
			PowonSts_LockParam |= PRMSUM_FLASH_NG;			/* 書込み失敗 */
		}
	}
	else if (PowonSts_LockParam & PRMSUM_FLASH_OK) {
		/* FLASH⇒RAMへのパラメータデータ転送 */
		FLT_RestoreLockParam();
		DataSumUpdate(OPE_DTNUM_LOCKINFO);		/* RAM上パラメータデータのSUM更新 */
		PowonSts_LockParam |= PRM_COPY_FtoRAM;	/* FLASH⇒RAMへのパラメータデータ転送 */
	}
	else {
		/* FLASH=NG && RAM=NG のケース */
		/* AMANO殿指示によりデフォルト値のセット等はせずにそのまま起動
		  (原則としてありえないケースとのこと) */
	}
	if( prm_invalid_check() ){
		FLT_WriteParam1(FLT_EXCLUSIVE);					// RAM⇒FLASHへのパラメータデータ転送
		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
	}

}

//[]----------------------------------------------------------------------[]
///	@brief		LOG Flash書込み開始処理
///
/// FlashROMタスクへRAM上の精算LOGをFlashROMへ書き込むよう要求する。
/// PowerON後の復電処理中にCallされた場合はタスク切替をしない書込みで要求。
/// そうでない場合はタスク切替を行いながら書き込みを行う方式で要求する。
/// この場合 書込みの完了は AppServ_IsSaleLogFlashWriting() で確認すること。
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG種別
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void AppServ_LogFlashWriteReq( short id )
{
	uchar	f_FlashWriteMode;
	ulong	ret_ul;
	uchar	*rec;
	short	num = LOG_DAT[id].count[LOG_DAT[id].kind];
	short	kind = LOG_DAT[id].kind;

	if( LogDatMax[id][0]>RECODE_SIZE ){
		rec = LOG_DAT[id].dat.s2;
	}
	else{
		rec = LOG_DAT[id].dat.s1[LOG_DAT[id].kind];
	}

	if( num==0 ){	/* LOGデータなし */
		return;
	}

	if (AppServ_IsLogFlashWriting(id) != 0) {
		return;		// 既に実行中ならやらない
	}

	if( FLT_f_TaskStart==0 ){						/* FlashROMタスク起動前（復電処理など）*/
		f_FlashWriteMode = FLT_EXCLUSIVE;			/* タスク切替なし（一気書込み）モード */
	}
	else{
		f_FlashWriteMode = FLT_NOT_EXCLUSIVE;		/* タスク切替ありモード */
	}

	/* LOG書込み要求 */
	ret_ul = FLT_WriteLog(id, (const char*)rec, num, f_FlashWriteMode, kind);

	switch( _FLT_RtnKind(ret_ul) ){
	case FLT_NORMAL:
		if( f_FlashWriteMode==FLT_NOT_EXCLUSIVE ){	/* タスク切替ありモード */
			/* 完了チェック時に必要なパラメータ保存 */
			AppServ_LogWriteCompleteCheckParam[id] = ret_ul;
			return;
		}
		else{
			/* 要求中ではない情報セット */
			AppServ_LogWriteCompleteCheckParam[id] = 0xffffffff;
		}
		break;
	case FLT_ERASE_ERR:	/* ※書込み失敗時の動作をここで行う */
	case FLT_WRITE_ERR:
	case FLT_VERIFY_ERR:
	case FLT_PARAM_ERR:
	case FLT_BUSY:
		/* 要求中ではない情報セット */
		AppServ_LogWriteCompleteCheckParam[id] = 0xffffffff;
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG Flash書込み終了チェック処理
///
/// FlashROMへRAM上の精算LOGを書き込み中か否か確認する。
/// OPEは接客開始前に本関数をCallして書き込み中であれば接客を開始しない
/// 様にする。
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG種別
///	@return		1=書込み中、0=書込み中ではない（終了を含む）
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar AppServ_IsLogFlashWriting( short Lno )
{
	uchar	ret_uc;
	ulong	ret_ul;

	if( AppServ_LogWriteCompleteCheckParam[Lno]==0xffffffff ){	// 要求していない
		return	0;
	}

	/* 書込み完了チェック */
	ret_uc = FLT_ChkWriteCmp( AppServ_LogWriteCompleteCheckParam[Lno], &ret_ul);
	if( ret_uc==0 ){		// 未完了
		return	1;
	}

	/* 書込み完了（タスク切替モードでの要求終了）*/
	AppServ_LogWriteCompleteCheckParam[Lno] = 0xffffffff;	// 要求中ではない情報セット

	switch( _FLT_RtnKind(ret_ul) ){		// ステータスチェック
	case	FLT_NORMAL:
		break;
	case	FLT_ERASE_ERR:	/* ※書込み失敗時の動作をここで行う */
	case	FLT_WRITE_ERR:
	case	FLT_VERIFY_ERR:
	case	FLT_PARAM_ERR:
	case	FLT_BUSY:
		break;
	}

	return	0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_ConvLockInfo2CSV_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室データアップロード初期化関数
 *| param	: void
 *| return	: 1:初期化成功
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvLockInfo2CSV_Init( void )
{
	AppServ_B2C_LockInfo.CallCount = 0;

	return 1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvLockInfo2CSV()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室データ変換関数（bin　→　csv）
 *| 		: １件の車室データを１行のcsv形式の文字列に変換する。
 *| param	: csvdata		変換後データの格納先
 *| param	: csvdata_len	変換後データの長さ
 *| param	: finish		変換の終了フラグ（初期化後３２４回）
 *| return	: void
 *[]----------------------------------------------------------------------[]*/
void	AppServ_CnvLockInfo2CSV( char *csvdata, ushort *csvdata_len, uchar *finish )
{
	ushort cnv_len;
	cnv_len = 0;
	// ロック装置種別
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].lok_syu, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// 料金種別
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].ryo_syu, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// 区画
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].area, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// 駐車位置ナンバー
	cnv_len += AppServ_CnvLockInfo2CSV_SUB(LockInfo[AppServ_B2C_LockInfo.CallCount].posi, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// 親I/F盤ナンバー
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].if_oya, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// 親I/F版に対する装置連番
	if( AppServ_B2C_LockInfo.CallCount < BIKE_START_INDEX ) {	// 駐車情報の場合、変換が必要
		cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)( LockInfo[AppServ_B2C_LockInfo.CallCount].lok_no + PRM_LOKNO_MIN ), &csvdata[cnv_len], SEPARATOR_NEW_LINE);	// 表示用に+100した値をわたす
	} else {													// 駐輪は従来通り
		cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].lok_no , &csvdata[cnv_len], SEPARATOR_NEW_LINE);
	}

	// 変換後の長さを設定。
	*csvdata_len = cnv_len;

	// 呼び出し回数インクリメント
	AppServ_B2C_LockInfo.CallCount++;

	// 終了フラグ設定
	if (AppServ_B2C_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
	} else {
		*finish = 0;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvLockInfo2CSV_SUB()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室データ変換関数（bin　→　csv）
 *| 		: 引数で与えられた各パラメータを１０進ASCII文字列に変換する。
 *| param	: param		変換するパラメータ
 *| param	: buf		変換後の文字列を充填するバッファの先頭
 *| param	: separator	変換後の文字列の最後に付加するセパレータ
 *| return	: uchar		変換した文字列の長さ
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvLockInfo2CSV_SUB(ulong param, char *buf, uchar separator)
{
	uchar cnv_len;

	cnv_len = intoasl_0sup((uchar*)buf, param, (ushort)10 );

	if (separator == SEPARATOR_COMMA) {
		buf[cnv_len] = ',';
		cnv_len++;
	} else {
		buf[cnv_len]		= 0x0d;
		buf[cnv_len + 1]	= 0x0a;
		cnv_len += 2;
	}

	return cnv_len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_ConvCSV2LockInfo_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室データアップロード初期化関数（csv　→　bin）
 *| param	: lock_info_image	変換後車室データ格納先ワークエリア
 *| param	: image_size		ワークエリアサイズ
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSV2LockInfo_Init( char *lock_info_image, ulong image_size )
{

	if (image_size < sizeof(LockInfo)) {
		return	(uchar)0;
	}

	AppServ_C2B_LockInfo.lock_info_image = (t_LockInfo*)lock_info_image;
	AppServ_C2B_LockInfo.image_size = image_size;
	AppServ_C2B_LockInfo.CallCount = 0;

	memset(lock_info_image, 0, sizeof(LockInfo));

	return	(uchar)1;

}
/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSV2LockInfo()
 *[]----------------------------------------------------------------------[]
 *| summary	: 
 *| param	: csvdata		変換元データ
 *| param	: csvdata_len	変換元データサイズ
 *| param	: finish		変換終了のフラグ（初期化後３２４回）
 *| return	: 1:OK	0:NG
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSV2LockInfo(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	uchar	ret;
	ulong	BinData;
	const char	*pNextCsvData;

	if (AppServ_C2B_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		if (csvdata[0] == '\r') {
			*finish = 1;
			return 1;
		}
		else {
			return 0;	/* 最終行以降にデータが存在 */
		}
	}
	
	pNextCsvData = csvdata;

	/* ロック装置種別 */
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, lok_syu)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_syu = (uchar)BinData;
	} else {
		return 0;
	}

	// 料金種別
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, ryo_syu)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].ryo_syu = (uchar)BinData;
	} else {
		return 0;
	}

	// 区画
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, area)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].area = (uchar)BinData;
	} else {
		return 0;
	}

	// 駐車位置ナンバー
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, posi)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].posi = BinData;
	} else {
		return 0;
	}

	// 親I/F盤ナンバー
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, if_oya)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].if_oya = (uchar)BinData;
	} else {
		return 0;
	}

	// 親I/F版に対する装置連番
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x01) {
		if( AppServ_C2B_LockInfo.CallCount < BIKE_START_INDEX ) {	// 駐車情報の場合、変換が必要
			if( BinData < PRM_LOKNO_MIN || BinData > PRM_LOKNO_MAX ){	// 範囲チェック( 100～315 有効 )
				// 範囲外の場合は「0」を代入するので、_appserv_CheckValは行う必要がない。
				AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = 0;									// 範囲外の場合、0(接続無状態)を保存
			} else {
				if (!_appserv_CheckVal(BinData- PRM_LOKNO_MIN, t_LockInfo, lok_no)) {
					return 0;
				}
				AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = (uchar)( BinData- PRM_LOKNO_MIN );	// 範囲内の場合、100を引いて保存する(保存領域が1byteのため)
			}
		} else {													// 駐輪は従来通り
			if (!_appserv_CheckVal(BinData, t_LockInfo, lok_no)) {
				return 0;
			}
			AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = (uchar)BinData;
		}
	} else {
		return 0;
	}
	// 呼び出し回数インクリメント
	AppServ_C2B_LockInfo.CallCount++;

	// 終了フラグ設定
	if (AppServ_C2B_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
	} else {
		*finish = 0;
	}

	return 1;

}


/*[]----------------------------------------------------------------------[]
 *|	name	: appserv_CheckVal()
 *[]----------------------------------------------------------------------[]
 *| summary	: データサイズ毎の値範囲チェック
 *| param	: data - 値範囲チェック対象とするデータ
 *|			  size - data代入予定の領域サイズ
 *| return	: 1 = OK
 *|			  0 = NG
 *[]----------------------------------------------------------------------[]*/
int	appserv_CheckVal(ulong data, size_t size)
{
	switch (size) {
	case sizeof(uchar):
		if (data & 0xFFFFFF00) {
			return 0;
		}
		else {
			return 1;
		}
		break;
	case sizeof(ushort):
		if (data & 0xFFFF0000) {
			return 0;
		}
		else {
			return 1;
		}
	case sizeof(ulong):
		return 1;
	default:
		return 0;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	ＥｄｙＡｔコマンド送信データフォーマット作成							|*/
/*|																			|*/
/*|	pcにアップロードするデータ形式に変換する								|*/
/*|	（精算機内データに改行コード(CRLF)を挿入する）							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = 改行挿入後データ格納エリアへのポインタ		|*/
/*|				   text = 改行挿入元データ(精算機内データ)へのポインタ		|*/
/*| RETURN VALUE : 変換後のデータ長										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : suzuki													|*/
/*| Date         : 2006/11/08												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToEdyAtCommand( char *inserted, const char text[6][20] )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < 6; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 20);
		len += 20;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	return len;
}
/*[]-----------------------------------------------------------------------[]*/
/*|	ＥｄｙＡｔコマンド受信データフォーマット作成							|*/
/*|																			|*/
/*|	PCからダウンロードされたデータをFlashROM格納形式に変換する。			|*/
/*|	文字データはASCII or Shift-JISに対応。Unicodeは不可。					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = 変換後データ格納エリアへのポインタ			|*/
/*|				   text = 変換元データ(PCから受信したデータ)へのポインタ	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : suzuki													|*/
/*| Date         : 2006/11/08												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	AppServ_FormatEdyAtCommand( char formated[6][20], const char *text, ushort text_len )
{
	ushort	Line;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', 6*20 );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* 行数Maxチェック */
			if( 6 <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* 文字数Maxチェック */
			if( 20 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード時のデータ変換（初期化）
//[]----------------------------------------------------------------------[]
///	@param[in]		kind 0:ｴﾗｰ 1:ｱﾗｰﾑ 2:ﾓﾆﾀ 3:操作ﾓﾆﾀ
///	@return			void
///	@note			RAM内データをCSV形式値に変換する。このための前処理。<br>
///					findfirst, findnextのように最初に本関数、以降は<br>
///					AppServ_ConvErrArmCSV()で順次読出しを行う。<br>
///					また、1セクション(1行)のデータサイズは32768Byte以下に<br>
///					すること(CR/LF含む)<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Init( uchar kind )
{

	// 本来、Bin_toCsv_Paramは共通パラメータのアップロード用領域だが、ログ系のアップロードにも
	// 必要なエリアのみ使用して併用する。
	ushort	From_kind = eLOG_ERROR;

	/* 作業エリアクリア */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind = kind;			// 作業種別の保持

	if( kind != 4 ){
		switch( kind ){
			case	0:
				From_kind = eLOG_ERROR;
				break;
			case	1:
				From_kind = eLOG_ALARM;
				break;
			case	2:
				From_kind = eLOG_MONITOR;
				break;
			case	3:
				From_kind = eLOG_OPERATE;
				break;
		}
		AppServ_BinCsvSave.Bin_toCsv_Param.datasize = Ope_Log_TotalCountGet( From_kind );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード時のデータ変換
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: 変換後データセットエリアへのポインタ
///	@param[in/out]	csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in/out]	finish		: 書込み終了判定エリア
///	@return			void
///	@attention		本関数Call前に AppServ_ConvErrArmCSV_Init() をCallすること。<br>
///					最初から読み出しなおす場合も、初期化関数をCallすること。<br>
///	@note			RAM内データをCSV形式値に変換し、バッファにセットする。<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV( char *csvdata, ulong *csvdata_len, uchar *finish )
{
	ulong	TotalLen = 0;
	t_AppServ_Bin_toCsv_Param *p = &AppServ_BinCsvSave.Bin_toCsv_Param;
	
	*finish = 0;
	if( 0 == p->Phase ){	/* 先頭行（見出し）送信フェーズ (Y)	*/
		AppServ_ConvErrArmCSV_Sub1( csvdata, csvdata_len, p );	/* 見出し作成 */
		p->Phase = 1;		/* 次フェーズはデータ部変換 */
		return;
	}

	/** データ部変換フェーズ **/
	AppServ_ConvErrArmCSV_Sub2( csvdata, &TotalLen, p );			/* 先頭カラム見出し作成 */

	/* 1セクションセット完了 又は 書込みバッファサイズLimitまでデータセット */
	*finish = AppServ_ConvErrArmCSV_Sub3( csvdata, &TotalLen, p );
	*csvdata_len = TotalLen;
}

const char *ErrCsvTitle[] =	// エラーログ
{
	"E",
	"エラーコード",
	"発生/解除",
	"発生年月日",
	"レベル",
	"補足情報",
	"発生",
	"解除",
};

const char *ArmCsvTitle[] =	// アラームログ
{
	"A",
	"アラームコード",
	"発生/解除",
	"発生年月日",
	"レベル",
	"補足情報",
	"発生",
	"解除",
};

const char *MoniCsvTitle[] = // モニタログ
{
	"R",
	"モニターコード",
	"発生年月日",
	"レベル",
	"モニター情報",
};

const char *OpeMoniCsvTitle[] =	// 操作モニタログ
{
	"M",
	"操作モニターコード",
	"発生年月日",
	"レベル",
	"変更前データ",
	"変更後データ",
};

const char *CarInfoCsvTitle[] =	// 操作モニタログ
{
	""			,"エリア名"	,"mode"		,"nstat"	,"ostat"	,
	"ryo_syu"	,"year"		,"mont"		,"date"		,"hour"		,
	"minu"		,"passwd"	,"uketuke"	,"u_year"	,"u_mont"	,
	"u_date"	,"u_hour"	,"u_minu"	,"bk_syu"	,"s_year"	,
	"s_mont"	,"s_date"	,"s_hour"	,"s_minu"	,"lag_to_in",
	"issue_cnt"	,"bk_wmai"	,"bk_wari"	,"bk_time"	,"bk_pst"	,
	"in_chk_cnt","timer"	,"car_fail"	,"Dummy1"	,"Dummy2"	,
	"Dummy3"	,"Dummy4"	,"Dummy5"	,"Dummy6"	,"Dummy7"	,
	"Dummy8"	,"Dummy9"	,"Dummy10"	,
};
static ulong	LockNumMax = LOCK_MAX;

typedef	void	(*EDITFUNC)(char *,	ulong *, t_AppServ_Bin_toCsv_Param * );
typedef struct{
	uchar		data_max;
	const char	**title_data;
	ulong		*data_count;
	EDITFUNC	EditFunc;
}CSV_EDIT_INFO;

CSV_EDIT_INFO csv_edit[] = 
	{
		{ 6,	ErrCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvErrDataCreate 	},	// [0]エラーデータ
		{ 6,	ArmCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvArmDataCreate 	},	// [1]アラームデータ
		{ 5,	MoniCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvMoniDataCreate 	},	// [2]モニタデータ
		{ 6,	OpeMoniCsvTitle, 	&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvOpeMoniDataCreate },	// [3]操作モニタ
		{ 43,	CarInfoCsvTitle, 	(ulong*)&LockNumMax, 			AppServ_ConvCarInfoDataCreate},	// [4]車室情報
	};
	
//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード時のデータ変換（Sub1）
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: 変換後データセットエリアへのポインタ
///	@param[in/out]	csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ変換時の管理領域
///	@return			void
///	@note			CSVファイルの１行目見出しを作成する。<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Sub1( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	ushort	i;
	ushort	TotalLen = 0;

	for( i=1; i<csv_edit[p->Kind].data_max; ++i ){										// 見出し分ロープ
		sprintf( &csvdata[TotalLen], "[%s]%s",csv_edit[p->Kind].title_data[i],"," );	// 見出し文字列をセット
		TotalLen += (ushort)(strlen(csv_edit[p->Kind].title_data[i])+3);				// 編集サイズをセット
	}

	csvdata[TotalLen] = 0x0d;			// CRをセット
	csvdata[TotalLen+1] = 0x0a;			// LFをセット

	*csvdata_len = TotalLen + 2;		// CR/LF分を加算
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード時のデータ変換（Sub2）
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: 変換後データセットエリアへのポインタ
///	@param[in/out]	csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ変換時の管理領域
///	@return			void
///	@note			各行の先頭（１カラム目）見出し作成処理
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Sub2( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	uchar	code[2]={0};
	
	memset( csvdata, 0, 256 );
	switch( p->Kind ){	// 作業種別
		case 0:			// エラーログ
			Ope_Log_1DataGet( eLOG_ERROR, p->Section, &FTP_buff );	
			memcpy( code, &((Err_log *)FTP_buff)->Errsyu, 2 );	// エラーコードをセット
			break;
		case 1:			// アラームログ
			Ope_Log_1DataGet( eLOG_ALARM, p->Section, &FTP_buff );	
			memcpy( code, &((Arm_log *)FTP_buff)->Armsyu, 2 ); // アラームコードをセット
			break;
		case 2:			// モニタログ
			Ope_Log_1DataGet( eLOG_MONITOR, p->Section, &FTP_buff );	
			memcpy( code, &((Mon_log *)FTP_buff)->MonKind, 2 ); // モニタコードをセット
			break;
		case 3:			// 操作モニタログ
			Ope_Log_1DataGet( eLOG_OPERATE, p->Section, &FTP_buff );	
			memcpy( code, &((Ope_log *)FTP_buff)->OpeKind, 2 ); // 操作モニタコードをセット
			break;
		case 4:			// 車室情報
			sprintf( csvdata, "[%03d]%s", p->Section,",");
			*csvdata_len = 6;
			return;
		default:
			return;
	}

	// 各コードを先頭セルにセット
	sprintf( csvdata, "[%s%03d-%03d]%s", csv_edit[p->Kind].title_data[0],code[0],code[1],",");
	*csvdata_len = 11;
}

//[]----------------------------------------------------------------------[]
///	@brief			共通／個別パラメータアップロード時のデータ変換（Sub3）
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: 変換後データセットエリアへのポインタ
///	@param[in/out]	csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ変換時の管理領域
///	@return			: 1=書込み終了。 0=未終了
///	@note			データセット（CSV作成）処理<br>
///					1行分のデータをセット。（１セクションの終了まで）<br>
///					末尾にはCR,LFをセットする。
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	AppServ_ConvErrArmCSV_Sub3( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	
	switch( p->Kind ){				// 作業種別
		case 0:						// エラーログ
		case 1:						// アラームログ
		case 2:						// モニタログ
		case 3:						// 操作モニタログ
		case 4:						// 車室情報
			csv_edit[p->Kind].EditFunc( csvdata, csvdata_len, p );	// 編集関数コール
			break;
		default:
			return 1;				// 上記以外は編集なし
	}
	
	csvdata[*csvdata_len] = 0x0d;	// CRをセット
	csvdata[*csvdata_len+1] = 0x0a;	// LFをセット
	*csvdata_len += 2;				// CR/LF分のサイズを加算

	// 終了要因判定
	++p->Section;					// 次のセクションへ
	if( *csv_edit[p->Kind].data_count <= p->Section ){
		return	1;					// 全終了 
	}

	return	0;						// 未終了
}

//[]----------------------------------------------------------------------[]
///	@brief			エラーログデータアップロード時のデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: 変換後データセットエリアへのポインタ
///	@param[in]		csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ作成時の管理領域
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	uchar	Asc[10]={0};
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Err_log	*plog;	// エラーデータ編集ポインタ
	
	Ope_Log_1DataGet( eLOG_ERROR, p->Section, &FTP_buff );	
	plog = (Err_log *)FTP_buff;
	
	// 発生/解除情報
	OccurReleaseDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Errdtc, p );

	// 発生年月日情報
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// レベル
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Errlev );

	// 補足情報
	if( plog->Errinf ){														// 補足情報あり？
		if( plog->Errinf == 1 ){											// アスキーデータ？
			sprintf( &csvdata[*csvdata_len], "%s%s", plog->Errdat,"," );	// 補足情報をセット
			*csvdata_len += (ushort)(strlen( (char*)plog->Errdat )+1);		// 編集サイズをセット
		}else{
			switch( plog->Errsyu ){											
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				case ERRMDL_EDY:											// エラーがEdyの場合
//					if( plog->Errcod == 42 || plog->Errcod == 43 ){								// 
//						hxtoas2( Asc, (ushort)((plog->ErrBinDat&0xFFFF0000L) >> 16));	// 上位2ByteをAscii変換
//						hxtoas2( &Asc[4], (ushort)(plog->ErrBinDat&0x0000FFFF) );		// 下位2ByteをAscii変換
//						sprintf( &csvdata[*csvdata_len], "%s%s", Asc,"," );				// 編集データセット
//						*csvdata_len += (ushort)((strlen( (char*)Asc )+1));				// 編集データサイズセット
//					}
//					break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				default:													// その他
					sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ErrBinDat,"," );	// 編集データセット
					*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ErrBinDat )+1);		// 編集サイズセット
					break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			アラームログデータアップロード時のデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: 変換後データセットエリアへのポインタ
///	@param[in]		csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ作成時の管理領域
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvArmDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	ulong	wrkl;
	ushort	wrks;
	Arm_log	*plog;	// アラームデータ編集ポインタ
	
	Ope_Log_1DataGet( eLOG_ALARM, p->Section, &FTP_buff );	
	plog = (Arm_log *)FTP_buff;

	// 発生/解除情報
	OccurReleaseDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Armdtc, p );

	// 発生年月日情報
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// レベル
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Armlev );

	// 補足情報
	if( plog->Arminf ){														// 補足情報あり？
		if( plog->Arminf == 1 && plog->Armsyu != 2){                        // アスキーデータ？
			sprintf( &csvdata[*csvdata_len], "%s%s", plog->Armdat,"," );    // 補足情報をセット
			*csvdata_len += (ushort)(strlen( (char*)plog->Armdat )+1);      // 編集サイズをセット
		}else{
			switch( plog->Armsyu ){
				case 2:
					if((ALARM_GT_MUKOU_PASS_USE <= plog->Armcod) && (plog->Armcod <= ALARM_GT_N_MINUTE_RULE) || 
						(plog->Armcod == ALARM_GT_MIFARE_READ_CRC_ERR)){
						memcpy(&wrkl,&plog->Armdat[0],4);
						memcpy(&wrks,&plog->Armdat[4],2);
						sprintf( &csvdata[*csvdata_len], "%lu", wrkl );   		// 補足情報をセット(駐車場№)
						*csvdata_len += (ushort)DigitNumCheck( (long)wrkl );   		// 編集サイズをセット
						sprintf( &csvdata[*csvdata_len], "%05hu%s", wrks,"," ); // 補足情報をセット(定期ID)
						*csvdata_len += (ushort)(5+1);    						// 編集サイズをセット(５桁+1)
					}
					else {
						sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ArmBinDat,"," );	// 編集データセット
						*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ArmBinDat )+1);       // 編集サイズセット
					}
					break;
				default:
					sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ArmBinDat,"," );	// 編集データセット
					*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ArmBinDat )+1);       // 編集サイズセット
					break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			モニタログデータアップロード時のデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: 変換後データセットエリアへのポインタ
///	@param[in]		csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ作成時の管理領域
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvMoniDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

	ulong	ulinf = 0;
	Mon_log	*plog;	// モニタログ編集ポインタ
	
	Ope_Log_1DataGet( eLOG_MONITOR, p->Section, &FTP_buff );	
	plog = (Mon_log *)FTP_buff;

	// 発生年月日情報
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// レベル
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->MonLevel );

	// モニタ情報
	if( plog->MonInfo[0] != 0 ){							// Asciiデータの場合
		sprintf( &csvdata[*csvdata_len], "%s%10s", plog->MonInfo,"," );	// そのまま補足情報をセット
	}else{													// Binデータの場合
		ulinf = ((ulong)plog->MonInfo[6] << (8 * 3));		// データを編集
		ulinf += ((ulong)plog->MonInfo[7] << (8 * 2));
		ulinf += ((ulong)plog->MonInfo[8] << (8 * 1));
		ulinf += plog->MonInfo[9];
		if( ulinf ){										// データがあれば
			sprintf( &csvdata[*csvdata_len], "%10lu%s", ulinf,"," );	// 編集した補足情報をセット
		}
	}
	
	if( plog->MonInfo[0] != 0 || ulinf != 0 ){				// 編集するデータがある場合
		*csvdata_len += (ushort)(sizeof( plog->MonInfo )+1);// 編集サイズを加算
	}
	

}

//[]----------------------------------------------------------------------[]
///	@brief			操作モニタログデータアップロード時のデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: 変換後データセットエリアへのポインタ
///	@param[in]		csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ作成時の管理領域
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvOpeMoniDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	Ope_log	*plog;	// 操作モニタログ編集ポインタ
	
	Ope_Log_1DataGet( eLOG_OPERATE, p->Section, &FTP_buff );	
	plog = (Ope_log *)FTP_buff;

	// 発生年月日情報
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// レベル
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->OpeLevel );

	// 変更前データ
	if( plog->OpeBefore ){
		sprintf( &csvdata[*csvdata_len], "%lu%s", plog->OpeBefore,"," );
		*csvdata_len += (ushort)(DigitNumCheck((long)plog->OpeBefore)+1);
	}else{
		csvdata[*csvdata_len] = ',';
		(*csvdata_len)++;
	}

	// 変更後データ
	if( plog->OpeAfter ){
		sprintf( &csvdata[*csvdata_len], "%lu%s", plog->OpeAfter,"," );
		*csvdata_len += (ushort)(DigitNumCheck((long)plog->OpeAfter)+1);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			車室情報データアップロード時のデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: 変換後データセットエリアへのポインタ
///	@param[in]		csvdata_len	: 変換後データのサイズをセットするエリアへのポインタ
///	@param[in]		p			: データ作成時の管理領域
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/06/18
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvCarInfoDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

	flp_com	*pdata = &FLAPDT.flp_data[p->Section];	
	uchar	i;
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->mode,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->mode ) + 1); 
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->nstat.word,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->nstat.word ) + 1);
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->ostat.word,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->ostat.word ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->ryo_syu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->ryo_syu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->year ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->date,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->date ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->passwd,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->passwd ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->uketuke,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->uketuke ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->u_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->u_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_syu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_syu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->s_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_year ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_date,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_date ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->lag_to_in.BYTE,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->lag_to_in.BYTE ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->issue_cnt,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->issue_cnt ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_wmai,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_wmai ) + 1);

	sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->bk_wari,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_wari ) + 1);

	sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->bk_time,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_time ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_pst,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_pst ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->in_chk_cnt,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->in_chk_cnt ) + 1);

	sprintf( &csvdata[*csvdata_len], "%ld%s",pdata->timer,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->timer ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->car_fail,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->car_fail ) + 1);

	for( i=0; i<(TBL_CNT(pdata->Dummy)); i++ ){
		sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->Dummy[i],"," );
		*csvdata_len += ( DigitNumCheck( (long)pdata->Dummy[0] ) + 1);
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			発生/解除データ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: 書込みデータ
///	@return			count		: 書込み桁数
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	OccurReleaseDataEdit( char *data, ulong *len, uchar dtc, t_AppServ_Bin_toCsv_Param *p )
{
	uchar	i = 0;
	
	switch( dtc ){
		case	0:
			i = 7;
			break;
		case	1:
			i = 6;
			break;
		default:
			i = 2;
			break;
	}
	sprintf( data, "%s%s",csv_edit[p->Kind].title_data[i],"," );
	*len += (ushort)(strlen(csv_edit[p->Kind].title_data[i])+1);
}

//[]----------------------------------------------------------------------[]
///	@brief			発生年月日データ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: 書込みデータ
///	@return			count		: 書込み桁数
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	TimeDataEdit( char *data, ulong *len, date_time_rec *time )
{
	sprintf( data, "%04d/%02d/%02d %02d:%02d%s",
				time->Year,
				time->Mon,
				time->Day,
				time->Hour,
				time->Min,
				"," );

	*len += 16+1;
}

//[]----------------------------------------------------------------------[]
///	@brief			レベルデータ作成
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: 書込みデータ
///	@return			count		: 書込み桁数
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	LevelDataEdit( char *data, ulong *len, uchar level )
{
	sprintf( data, "%d%s", level,"," );
	*len += 2;
}

//[]----------------------------------------------------------------------[]
///	@brief			データ書込み桁数チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: 書込みデータ
///	@return			count		: 書込み桁数
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	DigitNumCheck( long data )
{
	uchar count = 0;
	
	switch( data ){
		case	0:
			count = 1;
			break;
		case   -1:
			count = 2;
			break;
		default:
			while( data != 0 ){
				data /= 10;
				count++;
			}
			break;
	}
	return count;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログ関連ファイルサイズ取得(FTP用)
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: 書込みデータ
///	@return			count		: 書込み桁数
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ulong	DataSizeCheck( char *data, uchar kind )
{
	ulong Total;
	t_AppServ_Bin_toCsv_Param *p = &AppServ_BinCsvSave.Bin_toCsv_Param;

	Total = 0;
	AppServ_ConvErrArmCSV_Init( kind );
	switch( p->Kind ){	// 作業種別
		case 0:			// エラーログ
			Total = (p->datasize * sizeof( Err_log ));	
			break;
		case 1:			// アラームログ
			Total = (p->datasize * sizeof( Arm_log ));	
			break;
		case 2:			// モニタログ
			Total = (p->datasize * sizeof( Mon_log ));	
			break;
		case 3:			// 操作モニタログ
			Total = (p->datasize * sizeof( Ope_log ));	
			break;
		default:
			break;
	}

	return Total;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSVtoFLAPDT_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室データアップロード初期化関数（csv　→　bin）
 *| param	: lock_info_image	変換後車室データ格納先ワークエリア
 *| param	: image_size		ワークエリアサイズ
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSVtoFLAPDT_Init( char *flapdt_image, ulong image_size )
{

	if (image_size < sizeof(LockInfo)) {
		return	(uchar)0;
	}

	AppServ_CtoB_FLAPDT.flapdt_image = (flp_rec*)flapdt_image;
	AppServ_CtoB_FLAPDT.image_size = image_size;
	AppServ_CtoB_FLAPDT.CallCount = 0;
	memset(flapdt_image, 0, (size_t)image_size);

	return	(uchar)1;
}

#define	Read_Col(a,b,c)	AppServ_CnvCsvFLAPDT_1ColumnRead(a,b,c)
/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSVtoFLAPDT()
 *[]----------------------------------------------------------------------[]
 *| summary	: 
 *| param	: csvdata		変換元データ
 *| param	: csvdata_len	変換元データサイズ
 *| param	: finish		変換終了のフラグ（初期化後３２４回）
 *| return	: 1:OK	0:NG
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSVtoFLAPDT(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	uchar	ret;
	long	BinData;
	const char	*pNextCsvData;
	flp_com	*p = &AppServ_CtoB_FLAPDT.flapdt_image->flp_data[0];
	uchar	i;
	
	if (AppServ_CtoB_FLAPDT.CallCount >= ITEM_MAX_LOCK_INFO) {
		if (csvdata[0] == '\r') {
			*finish = 1;
			return 1;
		}
		else {
			return 0;	/* 最終行以降にデータが存在 */
		}
	}
	
	pNextCsvData = csvdata;

	// コメント行は飛ばして、データ行を取得する。
	while(1){
		ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );

		// エラー
		if( ret & BIT_ERROR )
			return	0;
			
		if( 0 == (ret & BIT_NEXT_EXIST) )						/* 次データなし(Y) */
			return 1;		// 受信継続

		if( ret & BIT_COMMENT )
			continue;

		if( ret & BIT_DATA_EXIST )
			break;


	}
	// マイナスの値も扱うので、ﾃﾞｰﾀの範囲チェックはしない。
	// mode
	if (ret == 0x03) {
		p->mode = (ushort)BinData;
	} else {
		return 0;
	}

	// nstat
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->nstat.word = (ushort)BinData;
	} else {
		return 0;
	}

	// ostat
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->ostat.word = (ushort)BinData;
	} else {
		return 0;
	}

	// ryo_syu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->ryo_syu = (ushort)BinData;
	} else {
		return 0;
	}

	// year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->year = (ushort)BinData;
	} else {
		return 0;
	}

	// mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->mont = (uchar)BinData;
	} else {
		return 0;
	}

	// date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->date = (uchar)BinData;
	} else {
		return 0;
	}

	// hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->hour = (uchar)BinData;
	} else {
		return 0;
	}

	// minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->minu = (uchar)BinData;
	} else {
		return 0;
	}

	// passwd
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->passwd = (ushort)BinData;
	} else {
		return 0;
	}

	// uketuke
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->uketuke = (ushort)BinData;
	} else {
		return 0;
	}

	// u_year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_year = (ushort)BinData;
	} else {
		return 0;
	}

	// u_mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_mont = (uchar)BinData;
	} else {
		return 0;
	}

	// u_date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_date = (uchar)BinData;
	} else {
		return 0;
	}

	// u_hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_hour = (uchar)BinData;
	} else {
		return 0;
	}

	// u_minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_minu = (uchar)BinData;
	} else {
		return 0;
	}

	// bk_syu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_syu = (ushort)BinData;
	} else {
		return 0;
	}

	// s_year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_year = (ushort)BinData;
	} else {
		return 0;
	}

	// s_mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_mont = (uchar)BinData;
	} else {
		return 0;
	}

	// s_date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_date = (uchar)BinData;
	} else {
		return 0;
	}

	// s_hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_hour = (uchar)BinData;
	} else {
		return 0;
	}

	// s_minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_minu = (uchar)BinData;
	} else {
		return 0;
	}

	// lag_to_in
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->lag_to_in.BYTE = (uchar)BinData;
	} else {
		return 0;
	}

	// issue_cnt
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->issue_cnt = (uchar)BinData;
	} else {
		return 0;
	}

	// bk_wmai
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_wmai = (ushort)BinData;
	} else {
		return 0;
	}

	// bk_wari
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_wari = (ulong)BinData;
	} else {
		return 0;
	}

	// bk_time
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_time = (ulong)BinData;
	} else {
		return 0;
	}

	// bk_pst
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_pst = (ushort)BinData;
	} else {
		return 0;
	}

	// in_chk_cnt
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->in_chk_cnt = (short)BinData;
	} else {
		return 0;
	}

	// timer
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->timer = (long)BinData;
	} else {
		return 0;
	}

	// car_fail
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->car_fail = (uchar)BinData;
	} else {
		return 0;
	}

	// Dummy
	for( i=0; i<(TBL_CNT(p->Dummy));i++ ){
		ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
		if (ret == 0x03) {
			p->Dummy[i] = (ulong)BinData;
		} else {
			if( ret & BIT_DATA_EXIST ){
				p->Dummy[i] = (ulong)BinData;
			}else if( i != (TBL_CNT(p->Dummy)-1) ){
				return 0;
			}else{
				return 0;
			}
		}
	}

	// 1Line読み込み後とにRAMのデータに反映させる。
	// 更新中の停電には再度ダウンロードを実施することで対応する
	memcpy(&FLAPDT.flp_data[AppServ_CtoB_FLAPDT.CallCount], p, sizeof(flp_com));		// RAMに反映
	
	// 呼び出し回数インクリメント
	AppServ_CtoB_FLAPDT.CallCount++;

	// 終了フラグ設定
	if (AppServ_CtoB_FLAPDT.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
		wopelg2(OPLOG_CARINFO_DL, 0, 0);	// 車室情報ダウンロード実施
	} else {
		*finish = 0;
	}

	return 1;

}

/*[]-----------------------------------------------------------------------[]*/
/*|	車室情報ダウンロード サブルーチン										|*/
/*|																			|*/
/*|	CSV形式データの１カラムをHEXデータに変換。								|*/
/*|	","(カンマ)で区切られたデータ１つの解析と変換を行う。					|*/
/*|	データがセクション番号を示すコメント行である場合は、その情報を返す。	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PCから受信したCSV形式データ					|*/
/*|				   pBinData = HEX変換値セットエリアへのポインタ				|*/
/*|							  return値=1時のみ有効							|*/
/*|				   pNextCsvData = 次データの先頭アドレス					|*/
/*| RETURN VALUE : 0=データなし、1=数値データ有り（pBinDataにセット）		|*/
/*|				   2=コメント行（変換なし）                                 |*/
/*|				   b0=1:BinDataあり（変換実施）								|*/
/*|				   b1=1:次データあり、0:最終データ							|*/
/*|				   b2=1:コメントカラム(先頭)、0:コメントでない				|*/
/*|				   b7=1:データエラーあり									|*/
/*|			（例）															|*/
/*|				   06h=コメントカラムのためデータなし  次データあり			|*/
/*|				   03h=数値データ有り(pBinDataにセット)次データあり			|*/
/*|				   01h=数値データ有り(pBinDataにセット)次データなし(最終)	|*/
/*|				   04h=コメントカラムのためデータなし  次データなし(最終)	|*/
/*|				   80h=形式エラー（変換なし）								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/06/18												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvFLAPDT_1ColumnRead( const char *CsvData, long *pBinData, const char **pNextCsvData )
{
	char	Asc[15];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret=0;
long	BinData;

	ret = 0;

	for( i=0, j=0; j<15; ++i ){								/* 最大15文字取得するまで */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* スペースは読み飛ばし */
			continue;
		}

		if( ',' == TgtChar ){								/* 区切り文字(次あり) */
			*pNextCsvData = (char*)&CsvData[i+1];					/* 次アドレスセット */
			ret |= BIT_NEXT_EXIST;
			goto AppServ_CnvCsvParam_1ColumnRead_10;		/* 1カラム取り出し後 処理へ */
		}

		else if( (0x0d == TgtChar) ||						/* 行末尾コード */
				 (0x0a == TgtChar) ||
				 (0x1a == TgtChar) ){

AppServ_CnvCsvParam_1ColumnRead_10:
			/** 1カラム取り出し後 処理 **/
			if( 0 == j ){									/* データなし(Y) */
				//break;
			}
			else if( '[' == Asc[0] ){						/* コメント行(Y) */
				ret |= BIT_COMMENT;
				//break;
			}
			else{
				if( 1 == DecAsc_to_LongBin_Minus((uchar*)&Asc[0], (uchar)j, &BinData) ){
															/* Ascii -> Hex変換 */
					*pBinData = BinData;					/* Hex data set */
					ret |= BIT_DATA_EXIST;					/* データあり */
				}
				else{										/* 元データ異常 */
					ret |= BIT_ERROR;
				}
				//break;
			}
			break;
		}

		else{												/* 終了コードではない */
			Asc[j++] = TgtChar;
			if( 15 < j ){									/* データ大きすぎerror */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}


/*[]-----------------------------------------------------------------------[]*/
/*|	Text送信データフォーマット作成											|*/
/*|																			|*/
/*|	PCからダウンロードされたデータをFlashROM格納形式に変換する。			|*/
/*|	文字データはASCII or Shift-JISに対応。Unicodeは不可。					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = 変換後データ格納エリアへのポインタ			|*/
/*|				   text = 変換元データ(PCから受信したデータ)へのポインタ	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/08/28												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	AppServ_FormatTextData( char formated[][36], const char *text, ushort text_len, uchar line_num )
{
	ushort	Line = line_num;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', (size_t)(Line*36) );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* 行数Maxチェック */
			if( line_num <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* 文字数Maxチェック */
			if( 36 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	Text送信データフォーマット作成											|*/
/*|																			|*/
/*|	精算機内データに改行コード(CRLF)を挿入する								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = 改行挿入後データ格納エリアへのポインタ		|*/
/*|				   text = 改行挿入元データ(精算機内データ)へのポインタ		|*/
/*| RETURN VALUE : 変換後のデータ長										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/08/28												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToTextData( char *inserted, const char text[][36], uchar line_num )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < line_num; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 36);
		len += 36;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	
	return len;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード初期化（バイナリ形式対応）
//[]----------------------------------------------------------------------[]
///	@param[in]		kind ログ種別
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2012/05/04
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	AppServ_SaleTotal_LOG_Init( uchar kind, ulong size )
{

	// 本来、Bin_toCsv_Paramは共通パラメータのアップロード用領域だが、ログ系のアップロードにも
	// 必要なエリアのみ使用して併用する。

	/* 作業エリアクリア */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind = kind;			// 作業種別の保持
	AppServ_BinCsvSave.Bin_toCsv_Param.Item = LogDatMax[kind][0];			// 作業種別の保持

	AppServ_BinCsvSave.Bin_toCsv_Param.datasize = (size/LogDatMax[kind][0]);

}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータアップロード初期化（バイナリ形式対応）
//[]----------------------------------------------------------------------[]
///	@param[in]		kind ログ種別
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2012/05/04
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	AppServ_SaleTotal_LOG_edit( char *buf, ulong *len, uchar *finish )
{

	t_AppServ_Bin_toCsv_Param	*p = &AppServ_BinCsvSave.Bin_toCsv_Param;
	
	Ope_Log_1DataGet( p->Kind, p->Section, &FTP_buff );

	p->Section++;
	memcpy( buf, FTP_buff, p->Item );
	*len = p->Item;
	
	if( p->Section >= p->datasize ){
		*finish = 1;
	}
}

void Get_Result_dat(uchar no, uchar cnt, uchar *dat)
{
	
	switch(no){
	case 0:		// 社員番号
	case 1:		// 検査日時
	case 2:		// パターン番号
		break;
	case 3:
		memcpy(dat , &Chk_result.set[0], cnt);
		break;
	case 4:
		memcpy(dat , &Chk_result.moj[0], cnt);
		break;
	case 5:
		memcpy(dat , &Chk_result.key_disp[0], cnt);
		break;
	case 6:
		memcpy(dat , &Chk_result.mag[0], cnt);
		break;
	case 7:
		memcpy(dat , &Chk_result.led_shut[0], cnt);
		break;
	case 8:
		memcpy(dat , &Chk_result.sw[0], cnt);
		break;
	case 9:
		memcpy(dat , &Chk_result.r_print[0], cnt);
		break;
	case 10:
		memcpy(dat , &Chk_result.j_print[0], cnt);
		break;
	case 11:
		memcpy(dat , &Chk_result.ann[0], cnt);
		break;
	case 12:
		memcpy(dat , &Chk_result.sig[0], cnt);
		break;
	case 13:
		memcpy(dat , &Chk_result.coin[0], cnt);
		break;
	case 14:
		memcpy(dat , &Chk_result.note[0], cnt);
		break;
	default:
		break;
	}
}
const	ushort	AppServ_ChkResult_NumTbl[CHK_RESULT_NUMMAX]={
		0,
		CHK_RESULT01,									/* 検査結果01設定数			*/
		CHK_RESULT02,									/* 検査結果02設定数			*/
		CHK_RESULT03,									/* 検査結果03設定数			*/
		CHK_RESULT04,									/* 検査結果04設定数			*/
		CHK_RESULT05,									/* 検査結果05設定数			*/
		CHK_RESULT06,									/* 検査結果06設定数			*/
		CHK_RESULT07,									/* 検査結果07設定数			*/
		CHK_RESULT08,									/* 検査結果08設定数			*/
		CHK_RESULT09,									/* 検査結果09設定数			*/
		CHK_RESULT10,									/* 検査結果10設定数			*/
		CHK_RESULT11,									/* 検査結果11設定数			*/
		CHK_RESULT12,									/* 検査結果12設定数			*/
	
};
char	*Chk_res_p[CHK_RESULT_NUMMAX];				/* ﾊﾟﾗﾒｰﾀｾｯｼｮﾝﾃｰﾌﾞﾙ				*/
/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（初期化）				|*/
/*|																			|*/
/*|	RAM内データをCSV形式値に変換する。このための前処理。					|*/
/*|	findfirst, findnextのように最初に本関数、以降はAppServ_CnvParam2CSV()で	|*/
/*|	順次読出しを行う。														|*/
/*|	※※ datasizeには1112byte以上を用意すること。							|*/
/*|		 AppServ_CnvParam2CSV()でCSV一行分のデータを（100アイテム分）を		|*/
/*|		 セットするため。													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = 共通・個別パラメータデータの情報テーブル	|*/
/*|								へのポインタ								|*/
/*|				   datasize = AppServ_CnvParam2CSV()で一回にセット可能な	|*/
/*|							  データサイズ。呼び元モジュールのバッファサイズ|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (ワークエリアサイズが小さい)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChkResultCSV_Init( void )
{

	/* 作業エリアクリア */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	
	Chk_res_p[0]	= (char *)&Chk_res_ftp.Chk_Res00;											/*								*/
	Chk_res_p[1]	= (char *)&Chk_res_ftp.Chk_Res01;											/*								*/
	Chk_res_p[2]	= (char *)&Chk_res_ftp.Chk_Res02;											/*								*/
	Chk_res_p[3]	= (char *)&Chk_res_ftp.Chk_Res03;											/*								*/
	Chk_res_p[4]	= (char *)&Chk_res_ftp.Chk_Res04;											/*								*/
	Chk_res_p[5]	= (char *)&Chk_res_ftp.Chk_Res05;											/*								*/
	Chk_res_p[6]	= (char *)&Chk_res_ftp.Chk_Res06;											/*								*/
	Chk_res_p[7]	= (char *)&Chk_res_ftp.Chk_Res07;											/*								*/
	Chk_res_p[8]	= (char *)&Chk_res_ftp.Chk_Res08;											/*								*/
	Chk_res_p[9]	= (char *)&Chk_res_ftp.Chk_Res09;											/*								*/
	Chk_res_p[10]	= (char *)&Chk_res_ftp.Chk_Res10;											/*								*/
	Chk_res_p[11]	= (char *)&Chk_res_ftp.Chk_Res11;											/*								*/
	Chk_res_p[12]	= (char *)&Chk_res_ftp.Chk_Res12;											/*								*/

}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（読出し）				|*/
/*|																			|*/
/*|	RAM内データをCSV形式値に変換し、バッファにセットする。					|*/
/*|	データは100件 又は 1セクション分の小さい方となる。						|*/
/*|	本関数Call前に AppServ_ConvParam2CSV_Init() をCallすること。			|*/
/*|	最初から読み出しなおす場合も、初期化関数をCallすること。				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   finish = 全セクションの変換完了時 1をセット。以外は0。	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChkResultCSV( char *csvdata, ulong *csvdata_len, uchar *finish )
{
	ulong	TotalLen;
	uchar	ret;

	*finish = 0;

	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Phase ){	/* 先頭行（見出し）送信フェーズ (Y)	*/
		AppServ_ConvChk_ResultCSV_Sub1( csvdata, csvdata_len );	/* 見出し作成 */
		AppServ_BinCsvSave.Bin_toCsv_Param.Phase = 1;		/* 次フェーズはデータ部変換 */
		AppServ_BinCsvSave.Bin_toCsv_Param.Section = 1;
		return;
	}

	/** データ部変換フェーズ **/

	TotalLen = 0;

	AppServ_ConvChk_ResultCSV_Sub2( csvdata, &TotalLen, 			/* 先頭カラム見出し作成 */
									AppServ_BinCsvSave.Bin_toCsv_Param.Section,
									(ulong)1 );

	/* 1セクションセット完了 又は 書込みバッファサイズLimitまでデータセット */
	ret = AppServ_ConvChk_ResultCSV_Sub3( csvdata, &TotalLen );
	*csvdata_len = TotalLen;
	if( ret == 1 ){											/* 終了 (Y) */
		*finish = 1;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub1）					|*/
/*|																			|*/
/*|	CSVファイルの１行目見出しを作成する。									|*/
/*|	サイズは493byteとなるが、書き込み用バッファは512byte以上であるので		|*/
/*|	一気に一行分をセットする。												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChk_ResultCSV_Sub1( char *csvdata, ulong *csvdata_len )
{
	ushort	i;
	ushort	SetLen;
	ushort	TotalLen;
	uchar	Asc[8];

	TotalLen = 0;
	Asc[0] = ',';
	Asc[1] = '[';

	for( i=0; i<20; ++i ){

		cnvdec4( (char*)&Asc[2], (long)i );						/* Bin to Ascii変換 */

		SetLen = 0;
		if( 10 > i ){										/* 数字１文字 (Y) */
			Asc[2] = Asc[5];
			Asc[3] = ']';
			SetLen = 4;
		}
		else{												/* 数字２文字 */
			Asc[2] = Asc[4];
			Asc[3] = Asc[5];
			Asc[4] = ']';
			SetLen = 5;
		}

		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)SetLen );
		TotalLen += SetLen;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;

	*csvdata_len = TotalLen + 2;
	return;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub2）					|*/
/*|																			|*/
/*|	各行の先頭（１カラム目）見出し作成処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   ParamKind = 0:共通， 1:個別								|*/
/*|				   SectionNum = セクション番号（1～99）						|*/
/*|				   ItemNum = 先頭のItem番号（1～9999）						|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChk_ResultCSV_Sub2( char *csvdata, ulong *csvdata_len,
								   ushort SectionNum, ushort ItemNum )
{
	csvdata[0] = '[';

	cnvdec2( &csvdata[1], (short)SectionNum );
	csvdata[3] = '-';
	cnvdec4( &csvdata[4], (long)ItemNum );
	csvdata[8] = ']';
	csvdata[9] = ',';
	*csvdata_len = 10;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	共通／個別パラメータアップロード時のデータ変換（Sub3）					|*/
/*|																			|*/
/*|	データセット（CSV作成）処理												|*/
/*| 1行分のデータをセット。（１セクション終了 又は 100アイテム）			|*/
/*|	末尾にはCR,LFをセットする。(Max1112byteセットする)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = 変換後データセットエリアへのポインタ			|*/
/*|				   csvdata_len = 変換後データのサイズをセットする			|*/
/*|								 エリアへのポインタ							|*/
/*|				   作業エリア（AppServ_BinCsvSave）の情報を使う。			|*/
/*				   更新も行う。												|*/
/*| RETURN VALUE : 1=1セクション終了。 0=書込みバッファLimitによる終了      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvChk_ResultCSV_Sub3( char *csvdata, ulong *csvdata_len )
{
	char	i;
	ushort	TgtSection;		/* target Section number (0- ) */
	ushort	ItemCount;		/* total Item count in target Section */
	

	ushort	TotalLen;				/* 書き込んだサイズ */

	TgtSection	= AppServ_BinCsvSave.Bin_toCsv_Param.Section;
	ItemCount = AppServ_ChkResult_NumTbl[TgtSection];

	TotalLen = *csvdata_len;
	/*  */
	switch(TgtSection){
		case 1:
			/* 検査日時 */
			sprintf( &csvdata[TotalLen], "%02d%02d%02d%02d%02d%s",
					Chk_res_ftp.Chk_Res01.Chk_date.Year%100,
					Chk_res_ftp.Chk_Res01.Chk_date.Mon,
					Chk_res_ftp.Chk_Res01.Chk_date.Day,
					Chk_res_ftp.Chk_Res01.Chk_date.Hour,
					Chk_res_ftp.Chk_Res01.Chk_date.Min,
					"," );
			TotalLen += 10+1;
			/* 社員番号 */
			sprintf( &csvdata[TotalLen], "%04d%s", Chk_res_ftp.Chk_Res01.Kakari_no,",");
			TotalLen += 5;
			/* 製品号機 */
			memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res01.Machine_No, 6 );
			TotalLen += 6;
			csvdata[TotalLen] = ',';
			TotalLen++;
			/* モデル */
			memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res01.Model, 8 );
			TotalLen += 8;
			csvdata[TotalLen] = ',';
			TotalLen++;
			/* システム構成 */
			sprintf( &csvdata[TotalLen], "%02d%s", Chk_res_ftp.Chk_Res01.System,",");
			TotalLen += 3;
			/* サブチューブ金種 */
			sprintf( &csvdata[TotalLen], "%03d%s", Chk_res_ftp.Chk_Res01.Sub_Money,",");
			TotalLen += 4;
			break;
		case 2:						/* ソフトバージョン */
			for(i=0;i<3;i++){
				memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res02.Version[i], 8 );
				TotalLen+=8;
				csvdata[TotalLen] = ',';
				TotalLen++;
			}
			break;
		case 5:						/* モジュール№ */
			for(i=0;i<27;i++){
				memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res05.Mojule[i], 8 );
				TotalLen+=8;
				csvdata[TotalLen] = ',';
				TotalLen++;
			}
			break;
		case 3:						/* 磁気リーダーディップスイッチ状態 */
			for(i = 0; i<3; i++){
				sprintf( &csvdata[TotalLen], "%01d%01d%01d%01d%s", 
					Chk_res_ftp.Chk_Res03.Dip_sw[i][0],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][1],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][2],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][3],
					","
				);
				TotalLen += 5;
			}
			sprintf( &csvdata[TotalLen], "%01d%s", 
				Chk_res_ftp.Chk_Res03.Dip_sw6,
				","
			);
			TotalLen += 2;
			break;
		default:
			/* 結果が 0 or 1 の項目*/
			for(i=0;i<ItemCount;i++){
				sprintf( &csvdata[TotalLen], "%01d%s", Chk_res_p[TgtSection][i],",");
				TotalLen += 2;
			}
		break;
	}
	

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;
	TotalLen += 2;

	*csvdata_len = TotalLen;

	/* 終了要因判定 */
	TgtSection++;

	if(TgtSection >= CHK_RESULT_NUMMAX){
		return 1;
	}
	else{
		AppServ_BinCsvSave.Bin_toCsv_Param.Section = TgtSection;			/* 次のセクションへ */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= 0;
	}
	return	0;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	サイズ変更されたバックアップデータのリストア処理						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ushort	area, area_num;

	// エリア数の決定(少ないほうに合わせる)
	area_num = area_tbl->area_num;
	if (area_num > area_tbl_wk->area_num) {
		area_num = area_tbl_wk->area_num;
	}

	for (area = 0; area < area_num; area++) {
		// 入庫（駐車位置）情報はRSW=8起動時にリストアされるためここではリストアしない
		if( area == OPE_DTNUM_FLAPDT ){
			continue;
		}

		// アドレスが無い場合は格納対象外
		if(area_tbl_wk->from_area[area].from_address == 0L){
			continue;
		}

// MH321800(S) Y.Tanizaki ICクレジット対応
		if(area == OPE_DTNUM_SKY || area == OPE_DTNUM_SKYBK) {
			// ログバージョンに関わらず集計ログはここでリストア
			AppServ_Restore_SYUKEI(area, area_tbl, area_tbl_wk);
			continue;
		} else if(area == OPE_DTNUM_LOG_DCSYUUKEI) {
			// ログバージョンに関わらず電子マネー種別毎集計はここでリストア
			AppServ_Restore_SYUKEISP(area, area_tbl, area_tbl_wk);
		}
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810105 MH321800(S) 「未了残高照会完了」時にも未了確定をカウントする
		else if(area == OPE_DTNUM_EC_ALARM_TRADE_LOG) {
			//  ログバージョンに関わらず処理未了取引ログはここでリストア
			AppServ_Restore_ECALAMLOG(area, area_tbl_wk);
			continue;
		}
// MH810105 MH321800(E) 「未了残高照会完了」時にも未了確定をカウントする
		// バックアップデータの構造体サイズが変更された場合リストア処理を追加すること
		// (AppServ_Restore_ToV04関数ごと差し替えるイメージ)
		// ログとバックアップデータで共通に使用している構造体は下記の4つ
		// 集計ログ(SYUKEI)、金銭管理ログ(TURI_KAN)、コイン金庫集計ログ(COIN_SYU)、紙幣金庫集計ログ(NOTE_SYU)
		// バージョンダウンは考慮しない

		// プログラムのログバージョン
//		if( LOG_VERSION >= 5) // ←イメージ
		{
			AppServ_Restore_ToV05(area, area_tbl, area_tbl_wk);
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	バックアップデータ（ログバージョン5）のリストア処理						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							|*/
/*|				   area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_ToV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	// バックアップのログバージョン
	switch (bk_log_ver) {
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス（ログバージョン）
//	case 1:
//		AppServ_Restore_V01toV05(area, area_tbl, area_tbl_wk);
//		break;
//	case 2:
//	case 3:
//		AppServ_Restore_V02toV05(area, area_tbl, area_tbl_wk);
//		break;
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス（ログバージョン）
	default:			// ログバージョン不定(MH322902以前)、または、バージョンダウン
		// バックアップ時のデータサイズと一致すればリストアする
		if (area_tbl->area[area].size == area_tbl_wk->area[area].size) {
			// 通常のリストア処理
			Flash2Read(area_tbl->area[area].address,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
		}
		break;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	リストア処理（V01->V05）												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							|*/
/*|				   area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_V01toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	COIN_SYU_V01 *pCoin_V01;
	COIN_SYU_V04 *pCoin_V04;
	NOTE_SYU_V01 *pNote_V01;
	NOTE_SYU_V04 *pNote_V04;
	TURI_KAN_V01 *pTurikan_V01;
	TURI_KAN_V04 *pTurikan_V04;
	int i;

	switch (area) {
	case OPE_DTNUM_COINSYU:			// コイン金庫集計
	case OPE_DTNUM_COIN_BK:			// コイン金庫合計（前回分）
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pCoin_V01 = (COIN_SYU_V01 *)g_TempUse_Buffer;
		pCoin_V04 = (COIN_SYU_V04 *)area_tbl->area[area].address;

		// COIN_SYU_V01のサイズ分コピー
		memcpy(pCoin_V04, pCoin_V01, sizeof(COIN_SYU_V01));
		// 0をセット
		pCoin_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_NOTESYU:			// 紙幣金庫集計
	case OPE_DTNUM_NOTE_BK:			// 紙幣金庫合計（前回分）
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pNote_V01 = (NOTE_SYU_V01 *)g_TempUse_Buffer;
		pNote_V04 = (NOTE_SYU_V04 *)area_tbl->area[area].address;

		// NOTE_SYU_V01のサイズ分コピー
		memcpy(pNote_V04, pNote_V01, sizeof(NOTE_SYU_V01));
		// 0をセット
		pNote_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_TURIKAN:			// 金銭管理
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pTurikan_V01 = (TURI_KAN_V01 *)g_TempUse_Buffer;
		pTurikan_V04 = (TURI_KAN_V04 *)area_tbl->area[area].address;

		// turi_dat(TURI_KAN_V01)の前までコピー
		memcpy(pTurikan_V04, pTurikan_V01, offsetof(TURI_KAN_V01, turi_dat));
		// TURI_kan_rec_V01のサイズ分コピー
		for (i = 0; i < 4; i++) {
			memcpy(&pTurikan_V04->turi_dat[i], &pTurikan_V01->turi_dat[i], sizeof(TURI_kan_rec_V01));
			pTurikan_V04->turi_dat[i].SeqNo = 0;
		}
		// sub_tubeをセット
		pTurikan_V04->sub_tube = pTurikan_V01->sub_tube;
		// 0をセット
		pTurikan_V04->CenterSeqNo = 0;
		pTurikan_V04->SeqNo = 0;
		break;
	default:
		// 通常のリストア処理
		Flash2Read(area_tbl->area[area].address,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		break;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	リストア処理（V02->V05）												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							|*/
/*|				   area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_V02toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	COIN_SYU_V01 *pCoin_V01;
	COIN_SYU_V04 *pCoin_V04;
	NOTE_SYU_V01 *pNote_V01;
	NOTE_SYU_V04 *pNote_V04;
	TURI_KAN_V02 *pTurikan_V02;
	TURI_KAN_V04 *pTurikan_V04;
	int i;

	switch (area) {
	case OPE_DTNUM_COINSYU:			// コイン金庫集計
	case OPE_DTNUM_COIN_BK:			// コイン金庫合計（前回分）
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pCoin_V01 = (COIN_SYU_V01 *)g_TempUse_Buffer;
		pCoin_V04 = (COIN_SYU_V04 *)area_tbl->area[area].address;

		// COIN_SYU_V01のサイズ分コピー
		memcpy(pCoin_V04, pCoin_V01, sizeof(COIN_SYU_V01));
		// 0をセット
		pCoin_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_NOTESYU:			// 紙幣金庫集計
	case OPE_DTNUM_NOTE_BK:			// 紙幣金庫合計（前回分）
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pNote_V01 = (NOTE_SYU_V01 *)g_TempUse_Buffer;
		pNote_V04 = (NOTE_SYU_V04 *)area_tbl->area[area].address;

		// NOTE_SYU_V01のサイズ分コピー
		memcpy(pNote_V04, pNote_V01, sizeof(NOTE_SYU_V01));
		// 0をセット
		pNote_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_TURIKAN:			// 金銭管理
		// バックアップ時のデータサイズで読込み
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pTurikan_V02 = (TURI_KAN_V02 *)g_TempUse_Buffer;
		pTurikan_V04 = (TURI_KAN_V04 *)area_tbl->area[area].address;

		// turi_dat(TURI_KAN_V02)の前までコピー
		memcpy(pTurikan_V04, pTurikan_V02, offsetof(TURI_KAN_V02, turi_dat));
		// TURI_kan_rec_V01のサイズ分コピー
		for (i = 0; i < 4; i++) {
			memcpy(&pTurikan_V04->turi_dat[i], &pTurikan_V02->turi_dat[i], sizeof(TURI_kan_rec_V01));
			// 0をセット
			pTurikan_V04->turi_dat[i].SeqNo = 0;
		}
		// sub_tube～CenterSeqNoをセット
		pTurikan_V04->sub_tube = pTurikan_V02->sub_tube;
		pTurikan_V04->CenterSeqNo = pTurikan_V02->CenterSeqNo;
		// 0をセット
		pTurikan_V04->SeqNo = 0;
		break;
	default:
		// 通常のリストア処理
		Flash2Read(area_tbl->area[area].address,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		break;
	}
}

// MH321800(S) Y.Tanizaki ICクレジット対応
/*[]-----------------------------------------------------------------------[]*/
/*|	集計ログリストア処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							|*/
/*|				   area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanizaki												|*/
/*| Date         : 2019/05/29												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_SYUKEI(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong		ulBuckupLogSize;
	uchar		*pSkyBuckup = NULL;
	struct SKY	*pSkyRestore = NULL;

	switch (area) {
	case OPE_DTNUM_SKY:			// 集計（T,GT,複数台）
	case OPE_DTNUM_SKYBK:		// 前回集計

		if(bk_log_ver == 0 || bk_log_ver == 0xFF) {
			// LOG_VERSIONが不定(MH322902以前)の場合は集計エリアの構造が大きく異なるため、
			// リストアの対象としない
			// 0xFFはFROMの初期値、LOG_VERSIONが定義されていないバージョンからの
			// バージョンアップ時はこの値が入っている
		} else {
			// バックアップ時のデータサイズで読込み
			Flash2Read(g_TempUse_Buffer,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
			pSkyBuckup = (uchar*)g_TempUse_Buffer;
			pSkyRestore = (struct SKY*)area_tbl->area[area].address;

			// バックアップの各集計ログサイズを算出
			ulBuckupLogSize = area_tbl_wk->area[area].size / 4;

			// コピー先のエリアをクリア
			memset(area_tbl->area[area].address, 0, area_tbl->area[area].size);

			// 集計ログをコピー
			memcpy(&pSkyRestore->tsyuk, pSkyBuckup, ulBuckupLogSize);	// T集計
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->gsyuk, pSkyBuckup, ulBuckupLogSize);	// GT集計
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->msyuk, pSkyBuckup, ulBuckupLogSize);	// MT集計
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->fsyuk, pSkyBuckup, ulBuckupLogSize);	// 複数台数集計
		}
		break;
	default:	// 呼び出し元で制限しているけど念のため
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|	日毎集計ログリストア処理(Suica,Dey,EC)									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							|*/
/*|				   area_tbl = バックアップデータ情報(プログラム)			|*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanizaki												|*/
/*| Date         : 2019/05/29												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_SYUKEISP(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	switch (area) {
	case OPE_DTNUM_LOG_DCSYUUKEI:
// MH810103 GG119202(S) 個別パラメータがリストアされない
//		// バックアップ時のデータサイズで読込み
//		Flash2Read(area_tbl->area[area].address,
//				area_tbl_wk->from_area[area].from_address,
//				area_tbl_wk->area[area].size);
// MH810103(s) 日毎集計がリストアされない不具合修正
//		// 日毎集計データ構造体のサイズが増えたため、1セクタではバックアップできない
//		// そのため、日毎集計データのバックアップ位置をOPE_DTNUM_LOG_DCSYUUKEI_ECとした。
//
//		// ・LOG_VERSION=1以前はOPE_DTNUM_LOG_DCSYUUKEIから
//		//   OPE_DTNUM_LOG_DCSYUUKEI_ECへリストアする（先頭のsca_infのみ）
//		// ・LOG_VERSION=2以降はarea_tbl_wk->from_area[OPE_DTNUM_LOG_DCSYUUKEI].from_addressが
//		//   0のため、この関数はコールされない。
//		if (bk_log_ver <= 1) {
//			Flash2Read(area_tbl->area[OPE_DTNUM_LOG_DCSYUUKEI_EC].address,
//					area_tbl_wk->from_area[area].from_address,
//					sizeof(syuukei_info));
//		}
		// 日毎集計データのバックアップ位置をOPE_DTNUM_LOG_DCSYUUKEI_ECに変更
		// サイズは変更なし

		// ・LOG_VERSION=1以前はOPE_DTNUM_LOG_DCSYUUKEIから
		//   OPE_DTNUM_LOG_DCSYUUKEI_ECへリストアする
		// ・LOG_VERSION=2以降はarea_tbl_wk->from_area[OPE_DTNUM_LOG_DCSYUUKEI].from_addressが
		//   0のため、この関数はコールされない。
		if (bk_log_ver <= 1) {
			Flash2Read(area_tbl->area[OPE_DTNUM_LOG_DCSYUUKEI_EC].address,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
		}
// MH810103(e) 日毎集計がリストアされない不具合修正
// MH810103 GG119202(E) 個別パラメータがリストアされない
		break;
	default:	// 呼び出し元で制限しているけど念のため
		break;
	}
}
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810105 MH321800(S) 「未了残高照会完了」時にも未了確定をカウントする
/*[]-----------------------------------------------------------------------[]*/
/*|	処理未了取引ログリストア処理(決済リーダ)									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = バックアップデータ種別							   |*/
/*|				   area_tbl_wk = バックアップデータ情報(Flash)				  |*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : D.Inaba										　　		|*/
/*| Date         : 2021/03/22												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2021 AMANO Corp.---[]*/
void AppServ_Restore_ECALAMLOG(ushort area, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	FromAdrs;
	ulong	datasize;
	ulong	wk_paydatasize = 0;

	switch (area) {
	case OPE_DTNUM_EC_ALARM_TRADE_LOG:
		memset( &EC_ALARM_LOG_DAT, 0, sizeof(EC_ALARM_LOG_DAT) );					// リストア先０クリア

// MH810105 GG119202(S) 処理未了取引記録リストア処理修正
//		// 未了取引ログをリストアする
//		FromAdrs = area_tbl_wk->from_area[area].from_address;						// 先頭アドレス取得
//		datasize = _offsetof(Ec_Alarm_Log_rec, paylog);								// 再精算ログまでのデータサイズ取得
//		Flash2Read( (uchar*)&EC_ALARM_LOG_DAT, FromAdrs, datasize );				// 未了取引ログ　リストア
		// 未了取引ログのヘッダ情報をリストアする
		FromAdrs = area_tbl_wk->from_area[area].from_address;						// 先頭アドレス取得
		datasize = _offsetof(Ec_Alarm_Log_rec, log);								// 未了取引ログまでのデータサイズ取得
		Flash2Read( (uchar*)&EC_ALARM_LOG_DAT, FromAdrs, datasize );				// ヘッダ情報リストア

		// 件数が減った場合は未了取引情報、再精算情報のリストア不可
		if (EC_ALARM_LOG_DAT.array_cnt > EC_ALARM_LOG_CNT) {
			memset(&EC_ALARM_LOG_DAT, 0, datasize);
			EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
			EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
			break;
		}

		// 未了取引情報をリストアする
		FromAdrs += datasize;
		datasize = EC_ALARM_LOG_DAT.data_size * EC_ALARM_LOG_DAT.array_cnt;
		Flash2Read((uchar*)EC_ALARM_LOG_DAT.log, FromAdrs, datasize);				// 未了取引情報リストア
// MH810105 GG119202(E) 処理未了取引記録リストア処理修正

		// LOG_VERSION>=2の時は再精算情報ログ情報をリストアしたいため、一度ここで精算ログサイズを読み出す
		if( bk_log_ver >= 2 ){
			// 本来であれば、FLT_ReadBRLOGSZ( &BR_LOG_SIZE[0] );よりログサイズを取得したいが
			// 起動時の復電処理※より、すでにログサイズを新しいログサイズに書き換えているそのため、
			// バックアップテーブルにある未了取引ログから過去の精算ログサイズを計算する。
			// ※AppServ_FukudenProc→FLT_initにてログサイズを新しくしている。
			wk_paydatasize = (area_tbl_wk->area[area].size - datasize) / EC_ALARM_LOG_DAT.array_cnt;

			// 精算ログサイズに差異がある場合は再精算情報のリストア不可
			if ( wk_paydatasize == LogDatMax[eLOG_PAYMENT][0] ) {
				// 再精算情報をリストアする
				FromAdrs += datasize;
				datasize = sizeof(Receipt_data) * EC_ALARM_LOG_DAT.array_cnt;
				Flash2Read( (uchar*)EC_ALARM_LOG_DAT.paylog, FromAdrs, datasize );		// 再精算情報リストア
			}
		}

		EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
		EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
		break;
	default:	// 呼び出し元で制限しているけど念のため
		break;
	}
}
// MH810105 MH321800(E) 「未了残高照会完了」時にも未了確定をカウントする
