#ifndef	_PRI_DEF_H_
#define	_PRI_DEF_H_
/*[]---------------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀｰ制御部：ﾍｯﾀﾞｰﾌｧｲﾙ													|*/
/*|																				|*/
/*|	ﾌｧｲﾙ名称	:	Pri_def.h													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	"mem_def.h"
// MH321800(S) G.So ICクレジット対応
#include "suica_def.h"
// MH321800(E) G.So ICクレジット対応

/*----------------------------------------------------------*/
/*	判定ﾌﾗｸﾞ定義											*/
/*----------------------------------------------------------*/
#define		YES		1
#define		NO		0

#define		PRI_KYOUSEI		0			// 強制出庫
#define		PRI_FUSEI		1			// 不正出庫


/*----------------------------------------------------------*/
/*	デバッグ用 詳細エラーメッセージ印字プリプロセッサ		*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/*	ﾒｯｾｰｼﾞ関連												*/
/*----------------------------------------------------------*/

/*----------------------------------*/
/*	ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ						*/
/*----------------------------------*/
#define		PREQ_RYOUSYUU			0x0601	// 領収証			印字要求
#define		PREQ_AZUKARI			0x0602	// 預り証			印字要求
#define		PREQ_UKETUKE			0x0603	// 受付券			印字要求

#define		PREQ_TSYOUKEI			0x0604	// 集計(Ｔ小計)		印字要求
#define		PREQ_TGOUKEI			0x0605	// 集計(Ｔ合計) 	印字要求
#define		PREQ_GTSYOUKEI			0x0606	// 集計(ＧＴ小計)	印字要求
#define		PREQ_GTGOUKEI			0x0607	// 集計(ＧＴ合計) 	印字要求

#define		PREQ_F_TSYOUKEI			0x0608	// 集計(複数Ｔ小計)	印字要求
#define		PREQ_F_TGOUKEI			0x0609	// 集計(複数Ｔ合計) 印字要求
#define		PREQ_F_GTSYOUKEI		0x060A	// 集計(複数ＧＴ小計)印字要求
#define		PREQ_F_GTGOUKEI			0x060B	// 集計(複数ＧＴ合計)印字要求

#define		PREQ_TGOUKEI_Z			0x060C	// 集計(前回Ｔ合計) 	印字要求
#define		PREQ_GTGOUKEI_Z			0x060D	// 集計(前回ＧＴ合計) 	印字要求
#define		PREQ_F_TGOUKEI_Z		0x060E	// 集計(前回複数Ｔ合計) 印字要求
#define		PREQ_F_GTGOUKEI_Z		0x060F	// 集計(前回複数ＧＴ合計)印字要求

#define		PREQ_TGOUKEI_J			0x0610	// 集計情報(Ｔ合計履歴) 印字要求

#define		PREQ_COKINKO_G			0x0611	// ｺｲﾝ金庫合計		印字要求
#define		PREQ_COKINKO_S			0x0612	// ｺｲﾝ金庫小計		印字要求
#define		PREQ_COKINKO_Z			0x0613	// 前回ｺｲﾝ金庫合計	印字要求
#define		PREQ_COKINKO_J			0x0614	// ｺｲﾝ金庫集計情報	印字要求

#define		PREQ_SIKINKO_G			0x0615	// 紙幣金庫合計		印字要求
#define		PREQ_SIKINKO_S			0x0616	// 紙幣金庫小計		印字要求
#define		PREQ_SIKINKO_Z			0x0617	// 前回紙幣金庫合計	印字要求
#define		PREQ_SIKINKO_J			0x0618	// 紙幣金庫集計情報	印字要求

#define		PREQ_TURIKAN			0x0619	// 釣銭管理			印字要求
#define		PREQ_TURIKAN_S			0x061A	// 釣銭管理（小計）	印字要求
#define		PREQ_TURIKAN_LOG		0x061B	// 釣銭管理履歴		印字要求

#define		PREQ_ERR_JOU			0x061C	// エラー情報		印字要求
#define		PREQ_ERR_LOG			0x061D	// エラー情報履歴	印字要求

#define		PREQ_ARM_JOU			0x061E	// アラーム情報		印字要求
#define		PREQ_ARM_LOG			0x061F	// アラーム情報履歴	印字要求

#define		PREQ_OPE_JOU			0x0620	// 操作情報			印字要求
#define		PREQ_OPE_LOG			0x0621	// 操作情報履歴		印字要求

#define		PREQ_SETTEIDATA			0x0622	// 設定データ		印字要求

#define		PREQ_DOUSACNT			0x0623	// 動作カウント		印字要求

#define		PREQ_LK_DOUSACNT		0x0624	// ﾛｯｸ装置動作ｶｳﾝﾄ	印字要求

#define		PREQ_LOCK_PARA			0x0625	// 車室パラメータ	印字要求
#define		PREQ_LOCK_SETTEI		0x0626	// ロック装置設定	印字要求

#define		PREQ_PKJOU_NOW			0x0627	// 車室情報（現在）	印字要求
#define		PREQ_PKJOU_SAV			0x0628	// 車室情報（退避）	印字要求

#define		PREQ_TEIKI_DATA1		0x0629	// 定期有効／無効	印字要求
#define		PREQ_TEIKI_DATA2		0x062A	// 定期入庫／出庫	印字要求

#define		PREQ_SVSTIK_KIGEN		0x062B	// ｻｰﾋﾞｽ券期限	印字要求

#define		PREQ_SPLDAY				0x062C	// 特別日		印字要求

#define		PREQ_TEIRYUU_JOU		0x062D	// 停留車情報	印字要求

#define		PREQ_TEIFUK_JOU			0x062E	// 停復電情報	印字要求
#define		PREQ_TEIFUK_LOG			0x062F	// 停復電履歴	印字要求

#define		PREQ_KOBETUSEISAN		0x0630	// 個別精算情報	印字要求

#define		PREQ_FUSKYO_JOU			0x0632	// 不正・強制出庫情報	印字要求
#define		PREQ_FUSKYO_LOG			0x0633	// 不正・強制出庫履歴	印字要求

#define		PREQ_ANYDATA_CNT		0x0634	// 各種ﾃﾞｰﾀ格納件数	印字要求

#define		PREQ_IREKAE_TEST		0x0635	// 用紙入れ替え時テスト印字要求

#define		PREQ_PRINT_TEST			0x0636	// プリンタテスト印字要求

#define		PREQ_AT_SYUUKEI			0x0637	// 自動集計印字要求

#define		PREQ_LOGO_REGIST		0x0638	// ロゴ印字データ登録要求

#define		PREQ_MON_JOU			0x0639	// モニタ情報			印字要求
#define		PREQ_MON_LOG			0x063A	// モニタ情報履歴		印字要求
#define		PREQ_CREDIT_CANCEL		0x063B	// クレジット取消情報データ印字要求

#define		PREQ_CREDIT_USE			0x063C	// クレジット利用明細ログ印字要求
#define		PREQ_SUICA_LOG			0x063E	// 通信LOGﾃﾞｰﾀ印字		印字要求
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		PREQ_EDY_ARM_R			0x063F	// Edyｱﾗｰﾑﾚｼｰﾄﾃﾞｰﾀ印字		印字要求
//#define		PREQ_EDY_STATUS			0x0640	// Edy設定＆ステータスデータ印字要求
//#define		PREQ_EDY_ARMPAY_LOG		0x0641	// Edyアラーム取引情報印字要求
//#define		PREQ_EDY_SHIME_R		0x0642	// Ｅｄｙ締め記録ﾚｼｰﾄﾃﾞｰﾀ印字		印字要求
//#define		PREQ_EDY_SHIME_LOG		0x0643	// Ｅｄｙ締め記録印字要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		PREQ_RMON_JOU			0x0644	// 遠隔監視情報			印字要求
#define		PREQ_RMON_LOG			0x0645	// 遠隔監視情報履歴		印字要求

#define		CMND_CHK2_SNO			PREQ_RYOUSYUU		// 印字処理可／不可ﾁｪｯｸ対象ｺﾏﾝﾄﾞ（印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ）開始NO.
enum{
	PREQ_ARC_MOD_VER = 0x0646, // ARCNETバージョン印字
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	PREQ_EDY_USE_LOG,			// Ｅｄｙ利用明細ログ印字要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	PREQ_SCA_USE_LOG,			// Ｓｕｉｃａ利用明細ログ印字要求
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	PREQ_EDY_SYU_LOG,			// Ｅｄｙ集計ログ印字要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	PREQ_SCA_SYU_LOG,			// Ｓｕｉｃａ集計ログ印字要求
	PREQ_NG_LOG,				// 不正券ログ	印字要求
	PREQ_IO_LOG,				// 入出庫ログ	印字要求
	PREQ_ATEND_INV_DATA,		// 係員無効データ印字要求
	PREQ_CHARGESETUP,			// 料金設定印字
	SETDIFFLOG_PRINT,			// 設定更新履歴印字
	PREQ_WCARD_INF,
	PREQ_HOJIN_USE,
	PREQ_RYOSETTEI_LOG,			// 遠隔料金設定ログ印字
	PREQ_CHK_PRINT,
	PREQ_LOOP_DATA,				// フラップループデータ要求結果印字
	PREQ_MNT_STACK,				// スタック使用量印字
	PREQ_CHKMODE_RESULT,
	PREQ_SUICA_LOG2,			// 通信LOG直近データ印字		印字要求
	PREQ_RT_PAYMENT_LOG,			// 料金テストログ印字要求
	PREQ_MTSYOUKEI,				// 集計(ＭＴ小計)			印字要求(0x0659)
	PREQ_MTGOUKEI,				// 集計(ＭＴ合計) 			印字要求(0x065A)
	PREQ_MTGOUKEI_Z,			// 集計(前回ＭＴ合計)		印字要求(0x065B)
	PREQ_GTGOUKEI_J,			// 集計情報(ＧＴ合計履歴) 	印字要求(0x065C)
	PREQ_CREDIT_UNSEND,			// 未送信売上依頼データ	印字要求
	PREQ_CREDIT_SALENG,			// 売上拒否データ	印字要求
// MH321800(S) G.So ICクレジット対応
	PREQ_EC_ALM_R,				// 決済リーダリーダアラームレシート印字要求
	PREQ_EC_SYU_LOG,			// 決済リーダ日毎集計ログ印字要求
	PREQ_EC_USE_LOG,			// 決済リーダ利用明細ログ印字要求
	PREQ_EC_BRAND_COND,			// 決済リーダブランド状態印字要求
	PREQ_EC_ALARM_LOG,			// 処理未了取引記録印字要求
	PREQ_EC_DEEMED_LOG,			// みなし決済復電ログ印字要求
	PREQ_RECV_DEEMED,			// 決済精算中止(復決済)データ印字要求
// MH321800(E) G.So ICクレジット対応
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	PREQ_EC_MINASHI_LOG,		// 決済リーダみなし決済ログ印字要求
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH810105(S) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
	PREQ_DEBUG,					// デバッグ用印字要求
// MH810105(E) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// MH810105(S) MH364301 QRコード決済対応
	PREQ_RECV_FAILURECONTACT,	// 障害連絡票データ印字要求
// MH810105(E) MH364301 QRコード決済対応
	PREQ_MAX,
};
#define	CMND_CHK2_ENO			PREQ_MAX		// 印字処理可／不可ﾁｪｯｸ対象ｺﾏﾝﾄﾞ（印字要求ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ）終了NO.	

#define		PREQ_INNJI_END			0x0680	// １ﾌﾞﾛｯｸ印字終了通知

#define		PREQ_INIT_END_R			0x0681	// ﾚｼｰﾄﾌﾟﾘﾝﾀ	初期化終了通知
#define		PREQ_INIT_END_J			0x0682	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ	初期化終了通知
// MH810104 GG119201(S) 電子ジャーナル対応
#define		PREQ_PRN_INFO			0x0683	// プリンタ情報要求
#define		PREQ_CLOCK_REQ			0x0684	// 時刻設定要求
#define		PREQ_CLOCK_SET_CMP		0x0685	// 時刻設定完了
#define		PREQ_FS_ERR				0x0686	// ファイルシステム異常通知
#define		PREQ_MACHINE_INFO		0x0687	// 機器情報通知
#define		PREQ_SD_INFO			0x0688	// SDカード情報通知
#define		PREQ_SW_VER_INFO		0x0689	// ソフトバージョン通知
#define		PREQ_SD_TEST_RW			0x068A	// SDカードテスト（リードライト）結果通知
#define		PREQ_SD_VER_INFO		0x068B	// SDカードバージョン通知
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810104 GG119201(S) 電子ジャーナル対応（リセット要求イベント追加）
#define		PREQ_RESET				0x068C	// ハードリセット要求
// MH810104 GG119201(E) 電子ジャーナル対応（リセット要求イベント追加）
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
#define		PREQ_INIT_SET_REQ		0x068D	// 時刻／初期設定要求
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）

#define 	PREQ_PRINTER_ERR		0x0690	// プリンターエラー処理
#define 	PREQ_JUSHIN_ERR1		0x0691	// 受信エラー１						※４５ＥＸ：JUSHIN_ERR_SYORI
#define 	PREQ_JUSHIN_ERR2		0x0692	// 受信エラー２(ニアエンド変化時)	※４５ＥＸ：JUSHIN_ERR_SYORI2
// MH810104 GG119201(S) 電子ジャーナル対応
#define 	PREQ_JUSHIN_INFO		0x0693	// プリンタステータス変化通知（電子ジャーナル用）
#define		PREQ_ERR_END			0x0694	// エラー終了（リトライオーバーで終了）
// MH810104 GG119201(E) 電子ジャーナル対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
#define		PREQ_MACHINE_INFO_R		0x0695	// レシートプリンタ機器情報通知
#define		PREQ_STOCK_PRINT		0x0696	// ストック印字開始（書き込み終了）
#define		PREQ_STOCK_CLEAR		0x0697	// ストックデータ破棄（ページバッファクリア）
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

#define		PREQ_SND_COMP_JP		0x06FC	// JP送信完了通知（印字完了ではないので注意）
#define		PREQ_SND_COMP_RP		0x06FD	// RP送信完了通知（印字完了ではないので注意）
#define		PREQ_DUMMY				0x06FE	// ﾀﾞﾐｰﾒｯｾｰｼﾞ（処理対象ﾒｯｾｰｼﾞ検索用）
#define		PREQ_INNJI_TYUUSHI		0x06FF	// 印字中止要求

/*----------------------------------*/
/*	ﾒｯｾｰｼﾞ編集用					*/
/*----------------------------------*/

// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへの印字終了ﾒｯｾｰｼﾞ印字結果(BMode)
#define		PRI_NML_END		0x00	// 正常終了
#define		PRI_CSL_END		0x01	// ｷｬﾝｾﾙ要求による終了
#define		PRI_ERR_END		0x02	// 異常終了

#define		PRI_NO_PAPER	0x10	// 紙切れ

// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへの印字終了ﾒｯｾｰｼﾞ異常終了理由(BStat)
#define		PRI_ERR_NON		0x00	// 印字結果＝正常終了またはｷｬﾝｾﾙ終了の場合
#define		PRI_ERR_STAT	0x01	// ﾌﾟﾘﾝﾀ状態異常
#define		PRI_ERR_BUSY	0x02	// ﾌﾟﾘﾝﾀBUSY
#define		PRI_ERR_ANY		0x03	// その他
// MH810104 GG119201(S) 電子ジャーナル対応
#define		PRI_ERR_WAIT_INIT	0x04	// 初期化完了待ち
// MH810104 GG119201(E) 電子ジャーナル対応

// ﾌﾟﾘﾝﾀ種別
#define		R_PRI			1		// ﾚｼｰﾄ
#define		J_PRI			2		// ｼﾞｬｰﾅﾙ
#define		RJ_PRI			3		// ﾚｼｰﾄ&ｼﾞｬｰﾅﾙ

#define		ERRPRI_TYP_LOG	0		// 履歴印字
#define		ERRPRI_TYP_JOU	1		// 情報印字
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
#define		PRI_QR_AESSIZE	32		// QR駐車券 暗号化対象エリアサイズ
#define		PRI_QR_DATASIZE	124		// QR駐車券 全体サイズ(CRC除く)
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(S) M.Fujikawa 2023/04/27 ゲート式車番チケットレスシステム対応　QRコード駐車券の難読化対応（駐車券の暗号化部分をASCII変換）
#define		PRI_QR_HEADERSIZE		92	// QR駐車券 ヘッダーサイズ
#define		PRI_QR_DATASIZEAS	156	// QR駐車券 全体サイズ（ASCII化したCRCを除く）
// GG129000(E) M.Fujikawa 2023/04/27 ゲート式車番チケットレスシステム対応　QRコード駐車券の難読化対応（駐車券の暗号化部分をASCII変換）

/*----------------------*/
/* ﾒｯｾｰｼﾞｺﾏﾝﾄ編集用		*/
/*----------------------*/
#define		MSG_CMND_MASK		0x06FF	// ｺﾏﾝﾄﾞﾏｽｸ
#define		CMND_CHK1_SNO		0x0601	// ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ開始NO.
#define		CMND_CHK1_ENO		0x06FF	// ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ終了NO.

#define		INNJI_ENDMASK		0x1000	// 印字終了ﾒｯｾｰｼﾞﾌﾗｸﾞ(ｵﾍﾟﾚｰｼｮﾝﾀｽｸへの送信用)
#define		INNJI_NEXTMASK		0x2000	// 印字処理中ﾒｯｾｰｼﾞﾌﾗｸﾞ（ﾌﾟﾘﾝﾀﾀｽｸへの送信用）

/*----------------------*/
/* ﾒｯｾｰｼﾞﾃﾞｰﾀ編集用		*/
/*----------------------*/

// ﾌﾟﾘﾝﾀ種別
#define REQ_PRI_MASK			0x0F	// 対象ﾌﾟﾘﾝﾀ種別ﾏｽｸﾃﾞｰﾀ(ｵﾍﾟﾚｰｼｮﾝﾀｽｸからの印字要求ﾒｯｾｰｼﾞ用)
#define NEXT_PRI_MASK			0xF0	// 対象ﾌﾟﾘﾝﾀ種別ﾏｽｸﾃﾞｰﾀ(ﾌﾟﾘﾝﾀﾀｽｸからの印字要求ﾒｯｾｰｼﾞ用)

#define I2C_PRI_REQ				0		// 印字要求
#define I2C_NEXT_SND_REQ		1		// 印字完了（次印字データ要求）
#define I2C_EVENT_QUE_REQ		2		// 未印字時のイベント発行要求


/*----------------------------------*/
/*	印字要求ﾒｯｾｰｼﾞ印字ﾃﾞｰﾀﾌｫｰﾏｯﾄ	*/
/*----------------------------------*/

// 割引券種別（tik_syu）
#define		SERVICE			0x01	// ｻｰﾋﾞｽ券
#define		KAKEURI			0x02	// 掛売券
#define		KAISUU			0x03	// 回数券
#define		WARIBIKI		0x04	// 割引券
#define		PREPAID			0x05	// プリペイドカード
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
#define		SHOPPING		0x06	// 買物割引
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// GG129000(S) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
#define		SYUBETU			0x07	// 種別割引
// GG129000(E) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]

#define		MISHUU			0x10	// 未収金（減額精算時に使用）
#define		FURIKAE			0x11	// 振替額（振替精算時に使用）
// MH322914(S) K.Onodera 2016/10/11 AI-V対応：振替精算
#define		FURIKAE2		0x20	// 振替額（ParkingWeb版振替精算で使用）
#define		FURIKAE_DETAIL	0x21	// 振替詳細
// MH322914(E) K.Onodera 2016/10/11 AI-V対応：振替精算
#define		SYUUSEI			0x12	// 修正額（修正精算時に使用）

#define		C_SERVICE		0x81	// 精算中止ｻｰﾋﾞｽ券
#define		C_KAKEURI		0x82	// 精算中止掛売券
#define		C_KAISUU		0x83	// 精算中止回数券
#define		C_WARIBIKI		0x84	// 精算中止割引券
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
#define		C_SHOPPING		0x86	// 買物割引
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// GG129000(S) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
#define		C_SYUBETU		0x87	// 種別割引
// GG129000(E) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]

// MH321800(S) Y.Tanizaki ICクレジット対応
#define		INVALID			0xff	// 不明
// MH321800(E) Y.Tanizaki ICクレジット対応

// 駐車場Ｎｏ．種別（pkno_syu）
#define		KIHON_PKNO		0		// 基本駐車場No.
#define		KAKUCHOU_1		1		// 拡張駐車場No.1
#define		KAKUCHOU_2		2		// 拡張駐車場No.2
#define		KAKUCHOU_3		3		// 拡張駐車場No.3


	// 預り証印字ﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			WFlag;		// 印字種別１（0:通常 1:復電）
		uchar			chuusi;		// 印字種別２（0:通常 1:精算中止）
		ulong			WPlace;		// 駐車位置ﾃﾞｰﾀ
		ushort			Kikai_no;	// 機械Ｎｏ
// MH810100(S) Y.Yamauchi 2020/02/18 車番チケットレス（預り証印字修正）
		uchar			CarSearchFlg; // 車番/日時検索種別
		uchar			CarSearchData[6]; // 車番(入力)
		uchar			CarNumber[4]; // 車番(精算)
		uchar			CarDataID[32]; // 車番データID
// MH810100(E) Y.Yamauchi 2020/02/18 車番チケットレス（預り証印字修正）
		uchar			Seisan_kind;// 精算種別
		ST_OIBAN		Oiban;		// 追番(預り追番)
		ST_OIBAN		Oiban2;		// 追番(精算追番 または 中止追番)
		date_time_rec	TTime;		// 発行日時
		ulong			WTotalPrice;// 請求額		※合計
		ulong			WInPrice;	// 入金額		※お預り
		ulong			WChgPrice;	// 払戻金額		※払戻額
		ulong			WFusoku;	// 払戻不足金額	※払出不足額
		ushort			Lok_No;		//ロック装置No
		uchar			Syusei;		// 修正精算（0:なし 1:あり）
	} Azukari_data;

	// 受付券印字ﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		ulong			WPlace;		// 駐車位置ﾃﾞｰﾀ
		ushort			Kikai_no;	// 機械Ｎｏ	
		ulong			Oiban;		// 追番
		date_time_rec	ISTime;		// 発行日時
		date_time_rec	TTime;		// 入庫日時
		ushort			Pword;		// ﾊﾟｽﾜｰﾄﾞ
		ushort			Lok_No;		//ロック装置No
		uchar			PrnCnt;		// 駐車証明書発行回数
	} Uketuke_data;


/*----------------------------------*/
/*	ﾒｯｾｰｼﾞﾃﾞｰﾀ部ﾌｫｰﾏｯﾄ				*/
/*----------------------------------*/

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#pragma	pack
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	// PREQ_RYOUSYUU：領収証印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Receipt_data	*prn_data;		// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
		uchar			reprint;		// 再発行ﾌﾗｸﾞ（OFF：通常/ON：再発行）
		date_time_rec	PriTime;		// 再発行日時
		uchar			kakari_no;		// 係員No.
	} T_FrmReceipt;

	// PREQ_AZUKARI：預り証印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Azukari_data	*prn_data;		// 預り証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmAzukari;

	// PREQ_UKETUKE：受付券印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Uketuke_data	*prn_data;		// 受付券印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
		uchar			reprint;		// 再発行ﾌﾗｸﾞ（OFF：通常/ON：再発行）
		date_time_rec	PriTime;		// 再発行日時
		uchar			kakari_no;		// 係員No.
	} T_FrmUketuke;

	// PREQ_TSYOUKEI	：集計（Ｔ小計）			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TGOUKEI		：集計（Ｔ合計）			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_GTSYOUKEI	：集計（ＧＴ小計）			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_GTGOUKEI	：集計（ＧＴ合計）			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_TSYOUKEI	：集計（複数Ｔ小計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_TGOUKEI	：集計（複数Ｔ合計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_GTSYOUKEI	：集計（複数ＧＴ小計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_GTGOUKEI	：集計（複数ＧＴ合計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TGOUKEI_Z	：集計（前回Ｔ合計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_GTGOUKEI_Z	：集計（前回ＧＴ合計）		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_TGOUKEI_Z	：集計（前回複数Ｔ合計）	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_F_GTGOUKEI_Z：集計（前回複数ＧＴ合計）	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_AT_SYUUKEI	：集計（自動集計）			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		SYUKEI			*prn_data;		// 集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
		date_time_rec	PriTime;		// 再ﾌﾟﾘﾝﾄ日時
// MH810105 GG119202(S) T合計連動印字対応
		ushort			print_flag;		// 各項目毎の印字の有無
// MH810105 GG119202(E) T合計連動印字対応
	} T_FrmSyuukei;

	// PREQ_COKINKO_G	：ｺｲﾝ金庫合計		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_COKINKO_S	：ｺｲﾝ金庫小計		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_COKINKO_Z	：前回ｺｲﾝ金庫合計	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		COIN_SYU		*prn_data;		// ｺｲﾝ金庫集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
		date_time_rec	PriTime;		// 再ﾌﾟﾘﾝﾄ日時
	} T_FrmCoSyuukei;

	// PREQ_SIKINKO_G	：紙幣金庫合計		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_SIKINKO_S	：紙幣金庫小計		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_SIKINKO_Z	：前回紙幣金庫合計	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		NOTE_SYU		*prn_data;		// 紙幣金庫集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
		date_time_rec	PriTime;		// 再ﾌﾟﾘﾝﾄ日時
	} T_FrmSiSyuukei;

	typedef struct {
		uchar			prn_kind;			// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;			// 機械№
		ulong			Oiban;				// 追番
		date_time_rec	NowTime;			// 今回日時
	} T_FrmChargeSetup;

	// PREQ_TURIKAN		：釣銭管理			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TURIKAN_S	：釣銭管理（小計）	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		TURI_KAN		*prn_data;		// 釣銭管理集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmTuriKan;

	// PREQ_ERR_JOU		：エラー情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Err_log			*prn_data;		// ｴﾗｰ情報印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmErrJou;

	// PREQ_ARM_JOU		：アラーム情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Arm_log			*prn_data;		// ｱﾗｰﾑ情報印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmArmJou;

	// PREQ_OPE_JOU		：操作情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Ope_log			*prn_data;		// 操作情報印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmOpeJou;

	// PREQ_MON_JOU		：ﾓﾆﾀ情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Mon_log			*prn_data;		// 操作情報印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmMonJou;

	// PREQ_RMON_JOU	：遠隔監視情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Rmon_log		*prn_data;		// 操作情報印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmRmonJou;

	// PREQ_SETTEIDATA	：設定ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmSetteiData;

	// PREQ_DOUSACNT	：動作ｶｳﾝﾄ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmDousaCnt;

	// PREQ_LK_DOUSACNT	：ﾛｯｸ装置動作ｶｳﾝﾄ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		uchar			Req_syu;		// 要求種別（１：ﾌﾗｯﾌﾟ装置／２：ﾛｯｸ装置）
	} T_FrmLockDcnt;

	// PREQ_LOCK_PARA	：車室パラメータ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmLockPara;

	// PREQ_LOCK_SETTEI	：ロック装置設定印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmLockSettei;

	// PREQ_PKJOU_NOW	：車室情報（現在）印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy2;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		uchar			prn_menu;		// 印字ﾒﾆｭｰ(駐車のみ/空車のみ/全て)
		uchar			dummy;
	} T_FrmPkjouNow;

	// PREQ_PKJOU_SAV	：車室情報（退避）印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmPkjouSav;

	// PREQ_TEIKI_DATA1：定期有効／無効印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		uchar			Pkno_syu;		// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）
		uchar			Req_syu;		// 要求種別（０：全指定／１：有効のみ／２：無効のみ）
	} T_FrmTeikiData1;

	// PREQ_TEIKI_DATA2：定期入庫／出庫印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		uchar			Pkno_syu;		// 駐車場Ｎｏ．種別（基本／拡１／拡２／拡３）
		uchar			Req_syu;		// 要求種別（０：全指定／１：入庫のみ／２：出庫のみ／３：初回のみ）
	} T_FrmTeikiData2;

	// PREQ_SVSTIK_KIGEN：ｻｰﾋﾞｽ券期限印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
	} T_FrmStikKigen;

	// PREQ_SPLDAY：特別日印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
	} T_FrmSplDay;

	// PREQ_TEIRYUU_JOU：停留車情報印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		short			Day;			// 停留日数
		short			Cnt;			// 停留車数
		short			*Data;			// 停留車ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmTeiRyuuJou;

	// PREQ_TEIFUK_JOU：停復電情報印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		Pon_log			Ponlog;			// 停復電情報
	} T_FrmTeiFukJou;

	// PREQ_FUSKYO_JOU	:不正・強制出庫情報	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		flp_log			*prn_data;		// 不正・強制出庫情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	} T_FrmFusKyo;

	// PREQ_ANYDATA_CNT：各種ﾃﾞｰﾀ格納件数印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
	} T_FrmAnyData;

	// PREQ_IREKAE_TEST	：用紙入替時ﾃｽﾄ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	} T_FrmIrekaeTest;

	// PREQ_PREQ_PRINT_TEST	：ﾌﾟﾘﾝﾀﾃｽﾄ印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	} T_FrmPrintTest;

	// PREQ_ERR_LOG		：エラー情報履歴	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_ARM_LOG		：アラーム情報履歴	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_OPE_LOG		：操作情報履歴		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TEIFUK_LOG	：停復電情報履歴	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_MON_LOG		：モニタ情報履歴	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		date_time_rec	PriTime;		// ﾌﾟﾘﾝﾄ日時

	} T_FrmLogPriReq1;

	// PREQ_COKINKO_J		：ｺｲﾝ金庫集計情報		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_SIKINKO_J		：紙幣金庫集計情報		印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_KOBETUSEISAN	：個別精算情報			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_SEISAN_CHUUSI	：精算中止情報			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_FUSKYO_LOG		：不正・強制出庫履歴	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TGOUKEI_J		：集計情報(Ｔ合計履歴) 	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TURIKAN_LOG		：釣銭管理履歴			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		uchar			BType;			// 検索方法(0:全て／1:日付／2:期間)
		date_time_rec	TSttTime;		// 開始日時
		date_time_rec	TEndTime;		// 終了日時
		ushort			LogCount;		// LOG登録件数(個別精算LOG・集計LOGで使用)
										// FlashROM内データ読み取り不可時は件数が減るので
										// 再度FlashROMから取得するのではこの値を使用する
		ushort			Room_no;		// 車室番号
	} T_FrmLogPriReq2;
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		ushort			Kakari_no;		// 係員Ｎｏ．
		uchar			BType;			// 検索方法(0:全て／1:日付／2:期間)
		date_time_rec	TSttTime;		// 開始日時
		date_time_rec	TEndTime;		// 終了日時
		ushort			LogCount;		// LOG登録件数(利用明細ログで使用)
		date_time_rec	PriTime;		// ﾌﾟﾘﾝﾄ日時
	} T_FrmLogPriReq3;

	// PREQ_EDY_USE_LOG		：Edy利用明細			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_SCA_USE_LOG		：Suica利用明細			印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;			// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;			// 機械Ｎｏ．
		ushort			Kakari_no;			// 係員Ｎｏ．
		uchar			BType;				// 検索方法(0:全て／1:日付／2:期間)
		date_time_rec	TSttTime;			// 開始日時
		date_time_rec	TEndTime;			// 終了日時
		ushort			LogCount;			// LOG登録件数(個別精算LOG・集計LOGで使用)
											// FlashROM内データ読み取り不可時は件数が減るので
											// 再度FlashROMから取得するのではこの値を使用する
		ushort			Ffst_no;			// 検索条件が最初に一致したFlashROMの情報番号
		ushort			Flst_no;			// 検索条件が最後に一致したFlashROMの情報番号
	} T_FrmLogPriReq4;

	// PREQ_CREDIT_UNSEND	：未送信売上依頼データ	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		date_time_rec	PriTime;		// ﾌﾟﾘﾝﾄ日時
		struct DATA_BK	*back;			// 売上依頼ﾃﾞｰﾀ
	} T_FrmUnSendPriReq;
	// PREQ_CREDIT_SALENG	：売上拒否データ	印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			Kikai_no;		// 機械Ｎｏ．
		date_time_rec	PriTime;		// ﾌﾟﾘﾝﾄ日時
		CRE_SALENG		*ng_data;		// 売上拒否ﾃﾞｰﾀ
	} T_FrmSaleNGPriReq;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	typedef struct {
//		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
//		edy_arm_log		*priedit;		// 機械Ｎｏ．
//	} T_FrmEdyArmReq;
//
//	typedef struct {
//		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
//		edy_shime_log	*priedit;		// ログデータﾎﾟｲﾝﾀ
//	} T_FrmEdyShimeReq;
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	// PREQ_LOGO_REGIST	：ロゴ印字データ登録要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	} T_FrmLogoRegist;

	// PREQ_INNJI_END：１ﾌﾞﾛｯｸ印字終了ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			prn_sts;		// ﾌﾟﾘﾝﾀ状態
	} T_FrmPrnBend;

	// PREQ_PRINTER_ERR：ﾌﾟﾘﾝﾀｰｴﾗｰﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			ssr_data;		// SSRﾃﾞｰﾀ
	} T_FrmPrnErr0;

	// PREQ_JUSHIN_ERR1：受信ｴﾗｰ１ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			prn_sts;		// ﾌﾟﾘﾝﾀ状態
	} T_FrmPrnErr1;

	// PREQ_JUSHIN_ERR2：受信ｴﾗｰ２ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			prn_sts;		// ﾌﾟﾘﾝﾀ状態
	} T_FrmPrnErr2;

	// PREQ_INNJI_TYUUSHI：印字中止ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;		// ﾌﾟﾘﾝﾀ種別
										//	R_PRI(1) = ﾚｼｰﾄ
										//	J_PRI(2) = ｼﾞｬｰﾅﾙ
										//	RJ_PRI(3)= ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
	} T_FrmPrnStop;

	// 印字終了ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ（ｵﾍﾟﾚｰｼｮﾝﾀｽｸへの印字終了ﾒｯｾｰｼﾞﾃﾞｰﾀ部）
	typedef struct {
		uchar			BMode;			// 印字結果
										//	PRI_NML_END(0)	：正常終了
										//	PRI_CSL_END(1)	：ｷｬﾝｾﾙ終了
										//	PRI_ERR_END(2)	：異常終了

		uchar			BStat;			// 異常終了理由
										//	PRI_ERR_NON(0)	：印字結果＝正常終了またはｷｬﾝｾﾙ終了の場合
										//	PRI_ERR_STAT(1)	：ﾌﾟﾘﾝﾀ状態異常
										//	PRI_ERR_BUSY(2)	：ﾌﾟﾘﾝﾀBUSY
										//	PRI_ERR_ANY(3)	：その他

		uchar			BPrinStat;		// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ
										//	Bit0:ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ	（1=ﾆｱｴﾝﾄﾞ）
										//	Bit1:ﾌﾟﾗﾃﾝｵｰﾌﾟﾝ		（1=ｵｰﾌﾟﾝ）
										//	Bit2:ﾍﾟｰﾊﾟｰｴﾝﾄﾞ		（1=ｴﾝﾄﾞ）
										//	Bit3:ﾍｯﾄﾞ温度異常	（1=異常）
										//	Bit4～7:未使用

		uchar			BPrikind;		// ﾌﾟﾘﾝﾀ種別
										//	R_PRI(1) = ﾚｼｰﾄ
										//	J_PRI(2) = ｼﾞｬｰﾅﾙ
	} T_FrmEnd;

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#pragma	unpack
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	typedef	union	{
		Receipt_data	Receipt_data;
		Syu_log			Syu_log;
		TURI_KAN		mny_log;
		COIN_SYU		coin_syu;
		NOTE_SYU		note_syu;
		flp_log			Flp_log;
		meisai_log		cre_log;
		Ope_log 		Ope_work;
		Mon_log			Mon_work;
		Arm_log			Arm_work;										// アラームログワークバッファ
		Err_log			Err_work;										// エラーログワークバッファ
		Pon_log			Pon_log_work;									// 停復電ログワークバッファ
		t_Change_data	Change_data;
		Rmon_log		Rmon_work;
	} t_Pri_Work;

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#pragma	pack
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	typedef struct {
		uchar			prn_kind;			// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			prn_inf;			// 印字する分類種別
		date_time_rec	PriTime;			// 再ﾌﾟﾘﾝﾄ日時
		ushort			Kikai_no;			// 機械№
		ushort			Kakari_no;			// 係員№
		syuukei_info	*syuukeiSP;
	} T_FrmSyuukei2;
	typedef struct {
		uchar			prn_kind;
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ulong			Serial_no;
		ushort			Kakari_no;
		ulong			Chk_no;
		ushort			Card_no;
		date_time_rec	ChkTime;
	} T_FrmChk_result;

// MH321800(S) G.So ICクレジット対応
	// PREQ_EC_ALM_R：決済リーダアラームレシート印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;				// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;					// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		date_time_rec	TOutTime;				// 発生時刻
		EC_SETTLEMENT_RES	*alm_rct_info;		// アラームレシート情報
	} T_FrmEcAlmRctReq;

	// PREQ_EC_BRAND_COND：決済リーダブランド状態印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar				prn_kind;			// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar				dummy;
		ulong				job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort				Kikai_no;			// 機械№
		EC_BRAND_TBL		*pridata;			// 印字依頼データ
		EC_BRAND_TBL		*pridata2;			// 印字依頼データ
	} T_FrmEcBrandReq;

	typedef struct {
		uchar				prn_kind;			// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar				dummy;
		ulong				job_id;				// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort				Kikai_no;			// 機械№
// MH810105 GG119202(S) T合計連動印字対応
		ulong				StartTime;			// 開始日時
		ulong				EndTime;			// 終了日時
// MH810105 GG119202(E) T合計連動印字対応
	} T_FrmEcAlarmLog;

	typedef struct {
		uchar			prn_kind;				// ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;					// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		EcDeemedFukudenLog	*deemed_jnl_info;	// みなし決済情報
	} T_FrmEcDeemedJnlReq;
// MH321800(E) G.So ICクレジット対応

// MH810105 GG119202(S) T合計連動印字対応
	// PREQ_TGOUKEI		：T合計連動印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	// PREQ_TGOUKEI_Z	：前回T合計連動印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		T_FrmLogPriReq4	Minashi;			// みなし決済プリント情報
		T_FrmEcAlarmLog	Alarm;				// 処理未了取引記録情報
	} T_FrmTgoukeiEc;

	typedef struct {
		T_FrmSyuukei	syuk;				// T合計情報
		T_FrmTgoukeiEc	Ec;					// T合計連動印字情報
	} T_FrmSyuukeiEc;
// 印字フラグ
#define	TGOUKEI_EC_MINASHI			0x0001	// みなし決済プリント
#define	TGOUKEI_EC_ALARM			0x0002	// 処理未了取引記録
// MH810105 GG119202(E) T合計連動印字対応


// MH810105(S) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
	// PREQ_DEBUG：デバッグ用印字要求ﾒｯｾｰｼﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		uchar			prn_kind;				// [ 1Byte] ﾌﾟﾘﾝﾀ種別
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			dummy;
		ulong			job_id;					// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		uchar			first_line;				// [ 1Byte] 1行目ﾌﾗｸﾞ
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//		struct clk_rec	wk_CLK_REC;				// [12Byte] 日時

// packの関係で、DATE_YMDHMSに置き換え
		DATE_YMDHMS		wk_CLK_YMDHMS;				// [7Byte] 日時
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ushort			wk_CLK_REC_msec;		// [ 2Byte] 日時(msec)
		char			data[33];				// [33Byte] 印字ﾃﾞｰﾀ
	} T_FrmDebugData;
	// NOTE:quesetで送信するメッセージデータのフォーマットなので、
	//      サイズがquesetのバッファ(MSGBUFSZ)を超えないように注意する。
// MH810105(E) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#pragma	unpack
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

/*----------------------------------------------------------*/
/*	ﾌﾟﾘﾝﾀ送信ﾊﾞｯﾌｧｻｲｽﾞ定義									*/
/*----------------------------------------------------------*/
#define	PRNBUF_SIZE				64		// ﾌﾟﾘﾝﾀ送信ﾊﾞｯﾌｧｻｲｽﾞ
#define	PRNQUE_CNT				(64+4)		// ﾌﾟﾘﾝﾀ送信ﾊﾞｯﾌｧ数 (+4:ﾛｺﾞ+ﾍｯﾀﾞｰのﾍｯﾀﾞｰ後印字用に追加) see.Prn_LogoHead_PostHead 

// MH810104 GG119201(S) 電子ジャーナル対応
#define	PRN_EJA_BUF_SIZE		32			// 電子ジャーナル用受信バッファサイズ
// MH810104 GG119201(E) 電子ジャーナル対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
#define	PRN_RCV_BUF_SIZE_R		34			// レシートプリンタ用受信バッファサイズ
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

/*----------------------------------------------------------*/
/*	印字ﾃﾞｰﾀ編集処理関連									*/
/*----------------------------------------------------------*/
// １行印字最大桁数
#define FONTA_MAX_COL			36		// ﾌｫﾝﾄＡ
#define FONTB_MAX_COL			48		// ﾌｫﾝﾄＢ
#define KANJI_MAX_COL			18		// 漢字

// 印字位置
#define		PRI_LEFT			0x01	// 左揃え
#define		PRI_CENTER			0x02	// ｾﾝﾀﾘﾝｸﾞ
#define		PRI_RIGHT			0x03	// 右揃え

// 文字種別
#define		FONT_A_NML			0x01	// ﾌｫﾝﾄＡ 通常
#define		FONT_A_TATEBAI		0x02	// ﾌｫﾝﾄＡ 縦倍角
#define		FONT_A_YOKOBAI		0x03	// ﾌｫﾝﾄＡ 横倍角
#define		FONT_A_4BAI			0x04	// ﾌｫﾝﾄＡ ４倍角

#define		FONT_B_NML			0x11	// ﾌｫﾝﾄＢ 通常
#define		FONT_B_TATEBAI		0x12	// ﾌｫﾝﾄＢ 縦倍角
#define		FONT_B_YOKOBAI		0x13	// ﾌｫﾝﾄＢ 横倍角
#define		FONT_B_4BAI			0x14	// ﾌｫﾝﾄＢ ４倍角

#define		FONT_K_NML			0x31	// 漢字 通常
#define		FONT_K_TATEBAI		0x32	// 漢字 縦倍角
#define		FONT_K_YOKOBAI		0x33	// 漢字 横倍角
#define		FONT_K_4BAI			0x34	// 漢字 ４倍角

// 文字の縦ﾄﾞｯﾄ数（１ﾄﾞｯﾄ=0.125mm）
#define		FONT_A_N_HDOT		24		// ﾌｫﾝﾄＡ 通常	(24×0.125=3mm)
#define		FONT_A_T_HDOT		48		// ﾌｫﾝﾄＡ 縦倍角(48×0.125=6mm)
#define		FONT_A_Y_HDOT		24		// ﾌｫﾝﾄＡ 横倍角(24×0.125=3mm)
#define		FONT_A_4_HDOT		48		// ﾌｫﾝﾄＡ ４倍角(48×0.125=6mm)

#define		FONT_B_N_HDOT		17		// ﾌｫﾝﾄＢ 通常	(17×0.125=2.125mm)
#define		FONT_B_T_HDOT		34		// ﾌｫﾝﾄＢ 縦倍角(34×0.125=4.25mm)
#define		FONT_B_Y_HDOT		17		// ﾌｫﾝﾄＢ 横倍角(17×0.125=2.125mm)
#define		FONT_B_4_HDOT		34		// ﾌｫﾝﾄＢ ４倍角(34×0.125=4.25mm)

#define		FONT_K_N_HDOT		24		// 漢字 通常	(24×0.125=3mm)
#define		FONT_K_T_HDOT		48		// 漢字 縦倍角	(48×0.125=6mm)
#define		FONT_K_Y_HDOT		24		// 漢字 横倍角	(24×0.125=3mm)
#define		FONT_K_4_HDOT		48		// 漢字 ４倍角	(48×0.125=6mm)

// 文字幅ｻｲｽﾞ
#define		FONTA_WIDE			15		// 1.5mm(右スペース０)
#define		KANJI_WIDE			30		// 3.0mm(左右スペース０)
#define		ROW_WIDE			540		// 54mm(用紙印字幅)

// ﾚｼｰﾄｶｯﾄ制御
#define		RCUT_END_FEED		48		// ﾛｰﾙ紙詰まり対策の為ｶｯﾄ後に紙送りするｻｲｽﾞ（ﾄﾞｯﾄ数）	※48×0.125=6mm
#define		RCUT_END_FEED_AH	28		// ﾍｯﾀﾞ印字のみのｶｯﾄ後に紙送りするｻｲｽﾞ（ﾄﾞｯﾄ数）		※28×0.125=3.5mm(改行量分)
#define		RCUT_END_FEED_NP	52		// ﾛｺﾞ・ﾍｯﾀﾞ印字なし時のｶｯﾄ後に紙送りするｻｲｽﾞ（ﾄﾞｯﾄ数）	※52×0.125=6.5mm
#define		RCUT_END_PERTIAL_FEED 8		// パーシャルカット後、次の文字を印字する前にフィードするドッド数(8×0.125 = 1mm)

// ロゴ印字関連
#define		LOGO_HDOT_SIZE		144		// ﾛｺﾞの縦ｻｲｽﾞ（ﾄﾞｯﾄ数）								※144×0.125=18mm
#define		LOGO_WDOT_SIZE		432		// ﾛｺﾞの横ｻｲｽﾞ（ﾄﾞｯﾄ数）								※432×0.125=54mm

#define		LOGO_HEADER_DSIZE	62		// ロゴデータヘッダー部分のデータサイズ
										// ﾌﾗｯｼｭﾒﾓﾘ上のﾃﾞｰﾀ
#define		LOGO_1LINE_DSIZE	56		// ロゴデータ横１ライン（ﾄﾞｯﾄ単位）のデータサイズ
										// ﾌﾗｯｼｭﾒﾓﾘ上のﾃﾞｰﾀﾌｫｰﾏｯﾄ	※432ﾄﾞｯﾄ÷8ﾋﾞｯﾄ=54(+2ﾊﾞｲﾄ：4ﾊﾞｲﾄﾊﾞﾝﾀﾞﾘｰの為)

#define		LOGO_REG_TATE_DOT	24		// ロゴ登録時における文字単位での縦ﾄﾞｯﾄ数
#define		LOGO_LINE_CNT		6		// ロゴ印字データの登録行数（文字単位での行数）
#define		LOGO_1LINE_BCNT		18		// ロゴ印字データ１行分の登録ﾌﾞﾛｯｸ数（１ﾌﾞﾛｯｸ＝２４×２４ﾄﾞｯﾄのﾋﾞｯﾄﾏｯﾌﾟ）

// タイマー値
#define		PRN_INIT_TIMER		50*3	// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰ値		（３秒）
#define		PRN_PWON_TIMER		50*3	// 停復電情報印字要求待ち待ちﾀｲﾏｰ値	（３秒）
#define		PRN_BUSY_TIMER		50*5	// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ信号監視ﾀｲﾏｰ値			（５秒）
#define		PRN_PNEND_TIMER		50*10	// ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ監視ﾀｲﾏｰ値			（１０秒）
#define		PRN_WAIT_TIMER		2		// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ値（印字ﾃﾞｰﾀ１行毎の遅延ﾀｲﾏｰ値）
										// ※印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰは、１行毎に「ﾀｲﾏｰ値×２０ｍｓ」となる
// MH810104 GG119201(S) 電子ジャーナル対応
#define		PRN_PINFO_RES_WAIT_TIMER	(1)			// プリンタ情報応答待ちタイマ（500ms）
#define		PRN_RW_RES_WAIT_TIMER		(60)		// リード・ライトテスト待ちタイマ（60s）
#define		PRN_RESET_WAIT_TIMER		(1)			// リセット完了待ちタイマ（500ms）
#define		PRN_SD_MOUNT_WAIT_TIMER		(30)		// SDマウント完了待ちタイマ（30s）
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//#define		PRN_WRITE_CMP_TIMER			(50*1)		// データ書き込み完了待ちタイマ（1s）
#define		PRN_WRITE_CMP_TIMER			(50*10)		// データ書き込み完了待ちタイマ（10s）
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#define		PRN_INIT_SET_WAIT_TIMER		(50*5)		// 時刻／初期設定待ちタイマ（5s）
// MH810104 GG119201(E) 電子ジャーナル対応

#define		PRN_BUSY_TCNT		3		// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀ値(10sec継続BUSYでｴﾗｰとする)

// その他
#define		EDIT_GYOU_MAX		15		// １ﾌﾞﾛｯｸ編集最大行数
#define		RCT_INIT_RCNT		1		// ﾚｼｰﾄﾌﾟﾘﾝﾀ	初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲ回数
#define		JNL_INIT_RCNT		1		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ	初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲ回数



/*----------------------------------------------------------*/
/*	ﾌﾟﾘﾝﾀ制御ｺﾏﾝﾄﾞ編集										*/
/*----------------------------------------------------------*/
#define LF		0x0A
#define ESC		0x1B
#define FS		0x1C
#define GS		0x1D
																					//		[ 機 能 ]							＜ｺﾏﾝﾄﾞ＞
//							＜単機能＞
#define	PCMD_R_SPACE( p )			PrnCmdLen( (uchar *)"\x1b\x20\x00" , 3 , p )	// [文字の右ｽﾍﾟｰｽ量(0mm)]				＜ESC SPC n(=0x04)＞

#define	PCMD_FONT_A_N( p )			PrnCmdLen( (uchar *)"\x1b\x21\x00" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ通常)]		＜ESC ! n(=0x00)＞
#define	PCMD_FONT_A_T( p )			PrnCmdLen( (uchar *)"\x1b\x21\x10" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ縦倍)]		＜ESC ! n(=0x10)＞
#define	PCMD_FONT_A_Y( p )			PrnCmdLen( (uchar *)"\x1b\x21\x20" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ横倍)]		＜ESC ! n(=0x20)＞
#define	PCMD_FONT_A_4( p )			PrnCmdLen( (uchar *)"\x1b\x21\x30" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＡ４倍)]		＜ESC ! n(=0x30)＞
#define	PCMD_FONT_B_N( p )			PrnCmdLen( (uchar *)"\x1b\x21\x01" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ通常)]		＜ESC ! n(=0x01)＞
#define	PCMD_FONT_B_T( p )			PrnCmdLen( (uchar *)"\x1b\x21\x11" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ縦倍)]		＜ESC ! n(=0x11)＞
#define	PCMD_FONT_B_Y( p )			PrnCmdLen( (uchar *)"\x1b\x21\x21" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ横倍)]		＜ESC ! n(=0x21)＞
#define	PCMD_FONT_B_4( p )			PrnCmdLen( (uchar *)"\x1b\x21\x31" , 3 , p )	// [印字ﾓｰﾄﾞの一括指定(ﾌｫﾝﾄＢ４倍)]		＜ESC ! n(=0x31)＞

#define	PCMD_LINECLR( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x00" , 3 , p )	// [ｱﾝﾀﾞｰﾗｲﾝの指定(解除)]				＜ESC - n(=0x00)＞
#define	PCMD_LINESET1( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x01" , 3 , p )	// [ｱﾝﾀﾞｰﾗｲﾝの指定(１ﾄﾞｯﾄ幅)]			＜ESC - n(=0x01)＞
#define	PCMD_LINESET2( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x02" , 3 , p )	// [ｱﾝﾀﾞｰﾗｲﾝの指定(２ﾄﾞｯﾄ幅)]			＜ESC - n(=0x02)＞

#define	PCMD_KAIGYO_NORMAL( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [改行量指定(標準=4mm)]				＜ESC 3 n(=0x20)＞
#define	PCMD_KAIGYO_NORMAL_1( p )		PrnCmdLen( (uchar *)"\x1b\x33\x18" , 3 , p )	// [改行量指定(標準=4mm)]				＜ESC 3 n(=0x20)＞
//#define	PCMD_KAIGYO_FONTA( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [改行量指定(ﾌｫﾝﾄＡ高さ＋1mm=4mm)]	＜ESC 3 n(=0x20)＞
//#define	PCMD_KAIGYO_FONTB( p )		PrnCmdLen( (uchar *)"\x1b\x33\x19" , 3 , p )	// [改行量指定(ﾌｫﾝﾄＢ高さ＋1mm=3.125mm)]＜ESC 3 n(=0x19)＞
//#define	PCMD_KAIGYO_TATEBAI( p )	PrnCmdLen( (uchar *)"\x1b\x33\x38" , 3 , p )	// [改行量指定(縦倍文字高さ＋1mm=7mm)]	＜ESC 3 n(=0x38)＞
//#define	PCMD_KAIGYO_KANJI( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [改行量指定(漢字高さ＋1mm=4mm)]		＜ESC 3 n(=0x20)＞

#define	PCMD_INIT( p )				PrnCmdLen( (uchar *)"\x1b\x40" , 2 , p )		// [ﾌﾟﾘﾝﾀの初期化]						＜ESC @＞

#define	PCMD_EMPHCLR( p )			PrnCmdLen( (uchar *)"\x1b\x45\x00" , 3 , p )	// [強調印字(解除)]						＜ESC E n(=0x00)＞
#define	PCMD_EMPHSET( p )			PrnCmdLen( (uchar *)"\x1b\x45\x01" , 3 , p )	// [強調印字(指定)]						＜ESC E n(=0x01)＞

#define	PCMD_LEFT( p )				PrnCmdLen( (uchar *)"\x1b\x61\x00" , 3 , p )	// [位置揃え(左揃え)]					＜ESC a n(=0x00)＞
#define	PCMD_CENTER( p )			PrnCmdLen( (uchar *)"\x1b\x61\x01" , 3 , p )	// [位置揃え(ｾﾝﾀﾘﾝｸﾞ)]					＜ESC a n(=0x01)＞
#define	PCMD_RIGHT( p )				PrnCmdLen( (uchar *)"\x1b\x61\x02" , 3 , p )	// [位置揃え(右揃え)]					＜ESC a n(=0x02)＞

#define	PCMD_CUT( p )				PrnCmdLen( (uchar *)"\x1b\x69" , 2 , p )		// [ﾌﾟﾘﾝﾀ用紙ｶｯﾄ]						＜ESC i＞
#define	PCMD_BS_CUT( p )			PrnCmdLen( (uchar *)"\x08\x1b\x69" , 3 , p )	// [4mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ｶｯﾄ]		＜BS,ESC i＞
#define	PCMD_PCUT( p )				PrnCmdLen( (uchar *)"\x1b\x6d" , 2 , p )		// [ﾌﾟﾘﾝﾀ用紙ﾊﾟｰｼｬﾙｶｯﾄ]					＜ESC m＞
#define	PCMD_BS_PCUT( p )			PrnCmdLen( (uchar *)"\x08\x1b\x6d" , 3 , p )	// [4mmﾊﾞｯｸﾌｨｰﾄﾞしてﾌﾟﾘﾝﾀ用紙ﾊﾟｰｼｬﾙｶｯﾄ]	＜BS,ESC m＞

#define	PCMD_STATUS_SEND( p )		PrnCmdLen((uchar*)"\x1b\x76",2 , p )			// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの送信]					＜ESC v＞

#define	PCMD_FONT_K_N( p )			PrnCmdLen( (uchar *)"\x1c\x21\x00" , 3 , p )	// [漢字印字ﾓｰﾄﾞ一括指定(通常)]			＜FS ! n(=0x00)＞
#define	PCMD_FONT_K_Y( p )			PrnCmdLen( (uchar *)"\x1c\x21\x04" , 3 , p )	// [漢字印字ﾓｰﾄﾞ一括指定(横倍)]			＜FS ! n(=0x04)＞
#define	PCMD_FONT_K_T( p )			PrnCmdLen( (uchar *)"\x1c\x21\x08" , 3 , p )	// [漢字印字ﾓｰﾄﾞ一括指定(縦倍)]			＜FS ! n(=0x08)＞
#define	PCMD_FONT_K_4( p )			PrnCmdLen( (uchar *)"\x1c\x21\x0c" , 3 , p )	// [漢字印字ﾓｰﾄﾞ一括指定(４倍)]			＜FS ! n(=0x0c)＞

#define	PCMD_KANJILINECLR( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x00" , 3 , p )	// [漢字ｱﾝﾀﾞｰﾗｲﾝの指定（解除）]			＜FS - n(0x00)＞
#define	PCMD_KANJILINESET1( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x01" , 3 , p )	// [漢字ｱﾝﾀﾞｰﾗｲﾝの指定（１ﾄﾞｯﾄ幅）]		＜FS - n(0x01)＞
#define	PCMD_KANJILINESET2( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x02" , 3 , p )	// [漢字ｱﾝﾀﾞｰﾗｲﾝの指定（２ﾄﾞｯﾄ幅）]		＜FS - n(0x02)＞

#define	PCMD_JIS( p )				PrnCmdLen( (uchar *)"\x1c\x43\x00" , 3 , p )	// [漢字ｺｰﾄﾞ体系選択(JIS)]				＜FS C n(=0x00)＞
#define	PCMD_SHIFTJIS( p )			PrnCmdLen( (uchar *)"\x1c\x43\x01" , 3 , p )	// [漢字ｺｰﾄﾞ体系選択(ｼﾌﾄJIS)]			＜FS C n(=0x01)＞

#define	PCMD_SPACE_KANJI( p )		PrnCmdLen( (uchar *)"\x1c\x53\x0\x0" , 4 , p )	// [漢字ｽﾍﾟｰｽ量の指定(左0mm、右0mm)]	＜FS S n1(=0x00) n2(=0x04)＞

#define	PCMD_KANJI4_CLR( p )		PrnCmdLen( (uchar *)"\x1c\x57\x00" , 3 , p )	// [漢字の４倍角文字(解除)]				＜FS W n(=0x00)＞
#define	PCMD_KANJI4_SET( p )		PrnCmdLen( (uchar *)"\x1c\x57\x01" , 3 , p )	// [漢字の４倍角文字(指定)]				＜FS W n(=0x01)＞

//#define	PCMD_DLBIMG_PRI0( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x00" , 3 , p )	// [ﾀﾞｳﾝﾛｰﾄﾞﾋﾞｯﾄｲﾒｰｼﾞ印字（ﾉｰﾏﾙﾓｰﾄﾞ）]	＜GS / m(=0x00)＞
//#define	PCMD_DLBIMG_PRI1( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x01" , 3 , p )	// [ﾀﾞｳﾝﾛｰﾄﾞﾋﾞｯﾄｲﾒｰｼﾞ印字（横倍ﾓｰﾄﾞ）]	＜GS / m(=0x01)＞
//#define	PCMD_DLBIMG_PRI2( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x02" , 3 , p )	// [ﾀﾞｳﾝﾛｰﾄﾞﾋﾞｯﾄｲﾒｰｼﾞ印字（縦倍ﾓｰﾄﾞ）]	＜GS / m(=0x02)＞
//#define	PCMD_DLBIMG_PRI3( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x03" , 3 , p )	// [ﾀﾞｳﾝﾛｰﾄﾞﾋﾞｯﾄｲﾒｰｼﾞ印字（４倍ﾓｰﾄﾞ）]	＜GS / m(=0x03)＞

#define	PCMD_STATUS_AUTO( p )		PrnCmdLen( (uchar *)"\x1d\x76\x00" , 3 , p )	// [ﾌﾟﾘﾝﾀｽﾃｰﾀｽの自動送信]				＜GS v NULL＞

#define	PCMD_LF( p )				PrnCmdLen( (uchar *)"\x0a" , 1 , p )			// [改行]

// MH810104 GG119201(S) 電子ジャーナル対応
// 電子ジャーナル専用コマンド
#define	PINFO_SD_INFO				0x01		// SDカード情報要求
#define	PINFO_MACHINE_INFO			0x02		// 機器情報要求
#define	PINFO_VER_INFO				0x03		// ソフトバージョン要求
#define	PINFO_FS_INFO				0x04		// ファイルシステム情報要求
#define	PINFO_SD_VER_INFO			0x05		// SDカードバージョン要求
#define	PINFO_SD_TEST_RW			0x10		// SDカードテスト（リードライト）
#define	PINFO_SD_TEST_SEQ			0x11		// SDカードテスト（Seq性能）
#define	PINFO_SD_TEST_4KB			0x12		// SDカードテスト（4KB性能）
#define	PINFO_SD_TEST_512B			0x13		// SDカードテスト（512B性能）
#define	PINFO_SD_TEST_CANCEL		0x1F		// SDカードテスト（キャンセル）

#define	PCMD_WRITE_START(p)			PrnCmdLen((uchar*)"\x1d\x47\x21", 3, p)			// [データ書込み開始]	＜GS G n(=0x21)＞
#define	PCMD_WRITE_END(p)			PrnCmdLen((uchar*)"\x1d\x47\x20", 3, p)			// [データ書込み終了]	＜GS G n(=0x20)＞
#define	PCMD_RESET(p)				PrnCmdLen((uchar*)"\x10\x18", 2, p)				// [ソフトリセット]		＜DLE CAN＞

#define	EJA_MACHINE_INFO			"EJA"		// 機器情報（上位3文字）
#define	isCMD_MACHINE_INFO(p)		(!memcmp(p, "\x1b\x73\x02", 3))					// 機器情報通知
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#define	isCMD_WRITE_START(p)		(!memcmp(p, "\x1d\x47\x21", 3))					// データ書込み開始コマンド
#define	isCMD_ENCRYPT(p)			(!memcmp(p, "\x1d\x4b", 2))						// 暗号化キーコマンド
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
// GG129000(S) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）
#define	PCMD_WRITE_RESET(p)			PrnCmdLen((uchar*)"\x18", 1, p)					// [ページバッファークリア]		＜CAN＞
// GG129000(E) 富士急ハイランド機能改善（領収証発行時のQR印字が遅くて印字途中で引っ張られる）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
#define	PRN_MACHINE_NEWMODEL_R		"2705-A4"										// モデル情報（7桁）
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

/*----------------------------------------------------------*/
/*	制御ｴﾘｱﾃﾞｰﾀ値定義										*/
/*----------------------------------------------------------*/
// 集計種別(syuukei_kind)

//	＜集計＞
#define		TSYOUKEI				1		// Ｔ小計
#define		TGOUKEI					2		// Ｔ合計
#define		GTSYOUKEI				3		// ＧＴ小計
#define		GTGOUKEI				4		// ＧＴ合計
#define		F_TSYOUKEI				5		// 複数Ｔ小計
#define		F_TGOUKEI				6		// 複数Ｔ合計
#define		F_GTSYOUKEI				7		// 複数ＧＴ小計
#define		F_GTGOUKEI				8		// 複数ＧＴ合計
#define		TGOUKEI_Z				9		// 前回Ｔ合計
#define		GTGOUKEI_Z				10		// 前回ＧＴ合計
#define		F_TGOUKEI_Z				11		// 前回複数Ｔ合計
#define		F_GTGOUKEI_Z			12		// 前回複数ＧＴ合計
#define		MTSYOUKEI				13		// ＭＴ小計
#define		MTGOUKEI				14		// ＭＴ合計
#define		MTGOUKEI_Z				15		// 前回ＭＴ合計

//	＜ｺｲﾝ金庫集計＞
#define		COKINKO_G				1		// ｺｲﾝ金庫合計
#define		COKINKO_S				2		// ｺｲﾝ金庫小計
#define		COKINKO_Z				3		// 前回ｺｲﾝ金庫合計

//	＜紙幣金庫集計＞
#define		SIKINKO_G				1		// 紙幣金庫合計
#define		SIKINKO_S				2		// 紙幣金庫小計
#define		SIKINKO_Z				3		// 前回紙幣金庫合計

//	＜釣銭管理集計＞
#define		TURIKAN					1		// 釣銭管理
#define		TURIKAN_S				2		// 釣銭管理（小計）

/*----------------------------------------------------------*/
/*	関数ﾊﾟﾗﾒｰﾀ												*/
/*----------------------------------------------------------*/
// ﾍｯﾀﾞｰ／ﾌｯﾀｰ種別
#define		PRI_HEADER				1		// ﾍｯﾀﾞｰ
#define		PRI_HOOTER				2		// ﾌｯﾀｰ
#define		PRI_TCARD_FOOTER		3		// Tｶｰﾄﾞ専用ﾌｯﾀｰ
#define		PRI_AZU_FOOTER			4		// 預り証専用ﾌｯﾀｰ

// 支払ｺｰﾄﾞ・署名欄／加盟店名 種別
#define		PRI_SYOMEI				1		// 支払ｺｰﾄﾞ・署名欄
#define		PRI_KAMEI				2		// 加盟店名
// ｴﾗｰﾛｸﾞ登録処理（Prn_errlg）用
#define		PRN_ERR_OFF				0		// ｴﾗｰ解除
#define		PRN_ERR_ON				1		// ｴﾗｰ発生
#define		PRN_ERR_ONOFF			2		// ｴﾗｰ発生/解除
#define		PRN_ERR_JOHO_NON		0		// ｴﾗｰ情報あり
#define		PRN_ERR_JOHO_ARI		1		// ｴﾗｰ情報あり

// 印字ﾃﾞｰﾀﾁｪｯｸ処理（Prn_data_chk）用
#define		RYOUSYUU_PRN_SYU1		1		// 領収証印字種別１	ﾁｪｯｸ要求
#define		RYOUSYUU_PRN_SYU2		2		// 領収証印字種別２	ﾁｪｯｸ要求
#define		PKICHI_DATA				3		// 駐車位置ﾃﾞｰﾀ		ﾁｪｯｸ要求
#define		SEISAN_SYU				4		// 精算種別			ﾁｪｯｸ要求
#define		RYOUKIN_SYU				5		// 料金種別			ﾁｪｯｸ要求
#define		TEIKI_SYU				6		// 定期券種別		ﾁｪｯｸ要求
#define		PKNO_SYU				7		// 駐車場Ｎｏ．種別	ﾁｪｯｸ要求
#define		SERVICE_SYU				8		// ｻｰﾋﾞｽ券種別		ﾁｪｯｸ要求
#define		TIKUSE_CNT				9		// 割引券使用枚数	ﾁｪｯｸ要求
#define		MISE_NO					10		// 店Ｎｏ．			ﾁｪｯｸ要求
#define		WARIBIKI_SYU			11		// 割引種別（割引券）ﾁｪｯｸ要求
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
#define		UNPAID_SYU				12		// 精算未精算種別	ﾁｪｯｸ要求
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)

/*----------------------------------------------------------*/
/*	各種ﾃﾞｰﾀ範囲定義										*/
/*----------------------------------------------------------*/

#define		RYOUSYUU_PRN_SYU1_MIN	0		// 領収証印字種別１ﾃﾞｰﾀ最小値
#define		RYOUSYUU_PRN_SYU1_MAX	1		// 領収証印字種別１ﾃﾞｰﾀ最大値

#define		RYOUSYUU_PRN_SYU2_MIN	0		// 領収証印字種別２ﾃﾞｰﾀ最小値
#define		RYOUSYUU_PRN_SYU2_MAX	3		// 領収証印字種別２ﾃﾞｰﾀ最大値

#define		PKICHI_NO_MIN			0		// 駐車位置番号ﾃﾞｰﾀ最小値（駐車位置番号なし）
#define		PKICHI_NO_MAX			9999	// 駐車位置番号ﾃﾞｰﾀ最大値

#define		PKICHI_KU_MIN			0		// 駐車位置区画ﾃﾞｰﾀ最小値（区画なし）
#define		PKICHI_KU_MAX			26		// 駐車位置区画ﾃﾞｰﾀ最大値（1～26：A～Z）

#define		SEISAN_SYU_MIN			0		// 精算種別ﾃﾞｰﾀ最小値
#define		SEISAN_SYU_MAX			2		// 精算種別ﾃﾞｰﾀ最大値

#define		RYOUKIN_SYU_MIN			1		// 料金種別ﾃﾞｰﾀ最小値
#define		RYOUKIN_SYU_MAX			12		// 料金種別ﾃﾞｰﾀ最大値

#define		TEIKI_SYU_MIN			1		// 定期券種別ﾃﾞｰﾀ最小値
#define		TEIKI_SYU_MAX			15		// 定期券種別ﾃﾞｰﾀ最大値

#define		PKNO_SYU_MIN			0		// 駐車場Ｎｏ．種別ﾃﾞｰﾀ最小値
#define		PKNO_SYU_MAX			3		// 駐車場Ｎｏ．種別ﾃﾞｰﾀ最大値

#define		SERVICE_SYU_MIN			1		// ｻｰﾋﾞｽ券種別ﾃﾞｰﾀ最小値
#define		SERVICE_SYU_MAX			15		// ｻｰﾋﾞｽ券種別ﾃﾞｰﾀ最大値

#define		TIKUSE_CNT_MIN			1		// 割引券使用枚数ﾃﾞｰﾀ最小値
#define		TIKUSE_CNT_MAX			99		// 割引券使用枚数ﾃﾞｰﾀ最大値

#define		MISE_NO_MIN				1		// 店Ｎｏ．ﾃﾞｰﾀ最小値
#define		MISE_NO_MAX				100		// 店Ｎｏ．ﾃﾞｰﾀ最大値

#define		WARIBIKI_SYU_MIN		1		// 割引種別（割引券）ﾃﾞｰﾀ最小値
#define		WARIBIKI_SYU_MAX		100		// 割引種別（割引券）ﾃﾞｰﾀ最大値

#define		SIHEI_SYU_MIN			1		// 紙幣種別ﾃﾞｰﾀ最小値
#define		SIHEI_SYU_MAX			4		// 紙幣種別ﾃﾞｰﾀ最大値

#define		COIN_SYU_MIN			1		// コイン種別ﾃﾞｰﾀ最小値
#define		COIN_SYU_MAX			4		// コイン種別ﾃﾞｰﾀ最大値

#define		IN_SIG_MIN				1		// 入庫信号ﾃﾞｰﾀ最小値
#define		IN_SIG_MAX				3		// 入庫信号ﾃﾞｰﾀ最大値

#define		BUNRUI_SYU_MIN			1		// 分類集計種別最小値
#define		BUNRUI_SYU_MAX			9		// 分類集計種別最大値

#define		BUNRUI_MIN				1		// 分類集計区分最小値
#define		BUNRUI_MAX				48		// 分類集計区分最大値

#define		SYUUKEI_SYU_MIN			1		// 集計種別最小値
#define		SYUUKEI_SYU_MAX			15		// 集計種別最大値

#define		COKISK_SYU_MIN			1		// ｺｲﾝ金庫集計種別最小値
#define		COKISK_SYU_MAX			3		// ｺｲﾝ金庫集計種別最大値

#define		SIKISK_SYU_MIN			1		// 紙幣金庫集計種別最小値
#define		SIKISK_SYU_MAX			3		// 紙幣金庫集計種別最大値

#define		HEAD_GYO_MIN			0		// ﾍｯﾀﾞｰ印字行数最小値
#define		HEAD_GYO_MAX			4		// ﾍｯﾀﾞｰ印字行数最大値

#define		HOOT_GYO_MIN			0		// ﾌｯﾀｰ印字行数最小値
#define		HOOT_GYO_MAX			4		// ﾌｯﾀｰ印字行数最大値
#define		SYOMEI_GYO_MIN			0		// 支払いｺｰﾄﾞ・署名欄印字行数最小値
#define		SYOMEI_GYO_MAX			4		// 支払いｺｰﾄﾞ・署名欄印字行数最大値

#define		KAMEI_GYO_MIN			0		// 加盟店名印字行数最小値
#define		KAMEI_GYO_MAX			4		// 加盟店名印字行数最大値

#define		ERR_INFO_SYU_MAX		99		// ｴﾗｰ情報のｴﾗｰ種別最大値（２桁）
#define		ERR_INFO_COD_MAX		99		// ｴﾗｰ情報のｴﾗｰｺｰﾄﾞ最大値（２桁）
#define		ARM_INFO_SYU_MAX		99		// ｱﾗｰﾑ情報のｱﾗｰﾑ種別最大値（２桁）
#define		ARM_INFO_COD_MAX		99		// ｱﾗｰﾑ情報のｱﾗｰﾑｺｰﾄﾞ最大値（２桁）
#define		OPE_INFO_SYU_MAX		99		// 操作情報の操作種別最大値（２桁）
#define		OPE_INFO_COD_MAX		99		// 操作情報の操作ｺｰﾄﾞ最大値（２桁）
#define		MON_INFO_SYU_MAX		99		// モニタ情報の操作種別最大値（２桁）
#define		MON_INFO_COD_MAX		99		// モニタ情報の操作ｺｰﾄﾞ最大値（２桁）
#define		T_FOOTER_GYO_MAX		10		// Tカードフッターの印字可能MAX行数
#define		AZU_FTR_GYO_MAX			4		// 預り証フッターの印字可能MAX行数
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
#define		KBR_FTR_GYO_MAX			4		// 過払いフッターの印字可能MAX行数
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
#define		FUTURE_FTR_GYO_MAX		4		// 後日支払額の印字可能MAX行数
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
#define		EMGFOOT_GYO_MAX			4		// 障害連絡票ﾌｯﾀｰ印字行数最大値
// MH810105(E) MH364301 QRコード決済対応
#define		RMON_INFO_SYU_MAX		99		// 遠隔監視情報の種別最大値
#define		RMON_INFO_COD_MAX		99		// 遠隔監視情報のコード最大値

// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
#define		UNPAID_SYU_MIN			0		// 精算未精算種別最小値
// GG129000(S) T.Nagai 2023/02/15 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//#define		UNPAID_SYU_MAX			1		// 精算未精算種別最大値
#define		UNPAID_SYU_MAX			2		// 精算未精算種別最大値
// GG129000(E) T.Nagai 2023/02/15 ゲート式車番チケットレスシステム対応（遠隔精算対応）
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)

/*--------------------------------------------------*/
/*	ﾌﾟﾘﾝﾀ制御部ﾃﾞｰﾀ構造体定義						*/
/*--------------------------------------------------*/

	// ﾌﾟﾘﾝﾀ送受信ﾃﾞｰﾀ管理ﾊﾞｯﾌｧﾌｫｰﾏｯﾄ
	typedef struct {
		uchar	PrnBuf[PRNQUE_CNT][PRNBUF_SIZE];	// ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ
		ushort	PrnBufWriteCnt;						// ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録数
		ushort	PrnBufReadCnt;						// ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ読込数
		ushort	PrnBufCnt;							// １ﾃﾞｰﾀﾊﾞｯﾌｧｵﾌｾｯﾄ
		ushort	PrnBufCntLen[PRNQUE_CNT];			// ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ登録ﾃﾞｰﾀﾚﾝｸﾞｽ
		uchar	PrnState[3];						// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ [0]:new,[1]:old [2]:MTB(Many Times Before)
		uchar	PrnStateRcvCnt;						// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ受信回数
		uchar	PrnStateMnt;						// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ			※ｼｽﾃﾑﾒﾝﾃﾅﾝｽ用
		uchar	PrnStWork;							// ﾌﾟﾘﾝﾀｽﾃｰﾀｽwork
		uchar	PrnBusyCnt;							// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀ
	}PRN_DATA_BUFF;

	// ﾌﾟﾘﾝﾀ編集処理制御ﾃﾞｰﾀﾌｫｰﾏｯﾄ
	typedef struct {
		ushort	Printing;			// 印字処理状態（処理中のﾒｯｾｰｼﾞｺﾏﾝﾄﾞを格納）
		ushort	Tyushi_Cmd;			// 印字中止ﾒｯｾｰｼﾞ受信ﾌﾗｸﾞ（ON：受信／OFF：未受信）
		ushort	Split;				// 印字処理中のﾌﾞﾛｯｸ数
		ushort	Final;				// 分割印字最終ﾌﾞﾛｯｸ通知（ON：最終ﾌﾞﾛｯｸ編集終了）
		ushort	EditWait;			// 印字ﾃﾞｰﾀ編集待ち状態（ON：印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ起動中）

		ushort	Prn_no;				// 印字処理済みの項目番号
		ushort	Prn_no_data1;		// 印字処理済みの項目番号ｻﾌﾞ情報１
		ushort	Prn_no_data2;		// 印字処理済みの項目番号ｻﾌﾞ情報２
		ushort	Prn_no_wk[10];		// 印字処理汎用ﾜｰｸ

		uchar	Log_Start;			// ﾛｸﾞﾃﾞｰﾀ印字開始ﾌﾗｸﾞ	（OFF:初回の編集／ON:２回目以降の編集）
		uchar	Log_Edit;			// ﾛｸﾞﾃﾞｰﾀ編集中ﾌﾗｸﾞ	（ON:編集中）
		ushort	Log_Count;			// ﾛｸﾞﾃﾞｰﾀ数
		ushort	Log_DataPt;			// ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ
		ushort	Log_Sdate;			// ﾛｸﾞﾃﾞｰﾀ検索開始日
		ushort	Log_Edate;			// ﾛｸﾞﾃﾞｰﾀ検索終了日
		ushort	Log_Stime;			// ﾛｸﾞﾃﾞｰﾀ検索開始時刻
		ushort	Log_Etime;			// ﾛｸﾞﾃﾞｰﾀ検索終了時刻
		ushort	Log_no_wk[2];		// ﾛｸﾞ印字処理汎用ﾜｰｸ
		ushort	Log_Count_inFROM;	// (FlashROM内)ﾛｸﾞﾃﾞｰﾀ数	(0-720)
		ushort	Log_DataPt_inFROM;	// (FlashROM内)ﾛｸﾞﾃﾞｰﾀﾎﾟｲﾝﾀ (0-719)
		ushort	Log_TotalCount;		// ﾛｸﾞﾃﾞｰﾀ数 (SRAM+FlashROM)(0-839)

		uchar	Font_size;			// ﾌｫﾝﾄ文字ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
		uchar	Kanji_size;			// 漢字ﾌｫﾝﾄｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した文字種別を格納）
		uchar	Kaigyou_size;		// 改行ｻｲｽﾞ（ﾌﾟﾘﾝﾀに指定した改行量（幅）をﾄﾞｯﾄ数単位で格納）
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		ulong	Prn_Job_id;			// ジョブID
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	}PRN_PROC_DATA;

// MH810104 GG119201(S) 電子ジャーナル対応
	// 電子ジャーナル受信データ管理バッファ
	typedef struct {
		short	EjaUseFlg;						// 電子ジャーナル接続フラグ
		uchar	PrnInfoReq;						// プリンタ情報要求中フラグ
		uchar	PrnRcvBufReadCnt;				// 受信バッファリードカウント
		uchar	PrnHResetRetryCnt;				// ハードリセットのリトライ回数
		uchar	PrnSResetRetryCnt;				// ソフトリセットのリトライ回数
		uchar	PrnClkReqFlg;					// 時計設定要求フラグ
		uchar	PrnInfReqFlg;					// プリンタ情報要求フラグ（外部）
		uchar	PrnRcvBuf[PRN_EJA_BUF_SIZE];	// 受信バッファ
		PRN_PROC_DATA	eja_proc_data;			// データ書込み開始時の印字処理制御データ
		MSG		PrnMsg;							// データ書込み中のメッセージ
	} PRN_EJA_DATA_BUFF;
// MH810104 GG119201(E) 電子ジャーナル対応

// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
#define	PRN_JOB_ID_MAX		999999
#define	PRN_QUE_COUNT_MAX	1
#define	PRN_QUE_DAT_SIZE	MSGBUFSZ
#define	PRN_QUE_OFFSET		2

	typedef struct {
		ushort	command;						// 印字要求コマンド
		uchar	prndata[PRN_QUE_DAT_SIZE];		// 印字データ
	} PRN_DAT;
	typedef struct {
		ushort	count;							// データ件数
		ushort	wtp;							// ライトポインタ
		ushort	rdp;							// リードポインタ
		PRN_DAT	prn_dat[PRN_QUE_COUNT_MAX];
	} PRN_DAT_QUE;
extern PRN_DAT_QUE	prn_dat_que;
extern ulong	prn_job_id;
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	// 電子ジャーナル受信データ管理バッファ
	typedef struct {
		short	NewModelFlg;					// 新モデルフラグ（-1=初期化前／0=従来（タイムアウト）／1=新）
		uchar	ModelMissFlg;					// モデル不一致フラグ（0=OK／1=不一致エラー）
		uchar	PrnInfoReq;						// プリンタ情報要求中フラグ
		uchar	PrnRcvBufReadCnt;				// 受信バッファリードカウント
		uchar	PrnRcvBuf[PRN_RCV_BUF_SIZE_R];	// 受信バッファ
	} PRN_RCV_DATA_BUFF_R;
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

/*----------------------------------------------*/
/*	ﾌﾟﾘﾝﾀ制御部：関数ﾌﾟﾛﾄﾀｲﾌﾟ					*/
/*----------------------------------------------*/												
/*-------	pritask.c	-------*/
extern	void	prn_int( void );							// ﾌﾟﾘﾝﾀ初期化処理
extern	void	pritask( void );							// ﾌﾟﾘﾝﾀ制御ﾒｲﾝ処理

extern	uchar	TgtMsGet		( MsgBuf *msb );			// 処理対象ﾒｯｾｰｼﾞ取得
extern	uchar	TgtMsgChk		( MsgBuf *msb );			// 処理対象ﾒｯｾｰｼﾞ判定
extern	uchar	RcvCheckWait	( MSG *msg );				// 印字処理可／不可ﾁｪｯｸ
extern	uchar	PriRctCheck		( void );					// ﾚｼｰﾄ印字処理可／不可ﾁｪｯｸ

extern	void	PrnRYOUSYUU		( MSG *msg );				// ＜領収証印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnAZUKARI		( MSG *msg );				// ＜預り証印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnUKETUKE		( MSG *msg );				// ＜受付券印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSYUUKEI		( MSG *msg );				// ＜集計印字要求＞					ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSYUUKEI_LOG	( MSG *msg );				// ＜集計履歴印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCOKI_SK		( MSG *msg );				// ＜ｺｲﾝ金庫集計印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCOKI_JO		( MSG *msg );				// ＜ｺｲﾝ金庫集計情報印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSIKI_SK		( MSG *msg );				// ＜紙幣金庫集計印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSIKI_JO		( MSG *msg );				// ＜紙幣金庫集計情報印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTURIKAN		( MSG *msg );				// ＜釣銭管理集計印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTURIKAN_LOG	( MSG *msg );				// ＜釣銭管理集計履歴印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnERR_JOU		( MSG *msg );				// ＜ｴﾗｰ情報印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnERR_LOG		( MSG *msg );				// ＜ｴﾗｰ情報履歴印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnARM_JOU		( MSG *msg );				// ＜ｱﾗｰﾑ情報印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnARM_LOG		( MSG *msg );				// ＜ｱﾗｰﾑ情報履歴印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnOPE_JOU		( MSG *msg );				// ＜操作情報印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnOPE_LOG		( MSG *msg );				// ＜操作情報履歴印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnMON_JOU		( MSG *msg );				// ＜モニタ情報印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnMON_LOG		( MSG *msg );				// ＜モニタ情報履歴印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSETTEI		( MSG *msg );				// ＜設定ﾃﾞｰﾀ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnDOUSAC		( MSG *msg );				// ＜動作ｶｳﾝﾄ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnLOCK_DCNT	( MSG *msg );				// ＜ﾛｯｸ装置動作ｶｳﾝﾄ印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnLOCK_PARA	( MSG *msg );				// ＜車室ﾊﾟﾗﾒｰﾀ印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnLOCK_SETTEI	( MSG *msg );				// ＜ﾛｯｸ装置設定印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnPKJOU_NOW	( MSG *msg );				// ＜車室情報（現在）印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnPKJOU_SAV	( MSG *msg );				// ＜車室情報（退避）印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTEIKIDATA1	( MSG *msg );				// ＜定期有効／無効印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTEIKIDATA2	( MSG *msg );				// ＜定期入庫／出庫印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSVSTIK_KIGEN	( MSG *msg );				// ＜ｻｰﾋﾞｽ券期限印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSPLDAY		( MSG *msg );				// ＜特別日印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTEIRYUU_JOU	( MSG *msg );				// ＜停留車情報印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTEIFUK_JOU	( MSG *msg );				// ＜停復電情報印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnTEIFUK_LOG	( MSG *msg );				// ＜停復電履歴印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSEISAN_LOG	( MSG *msg );				// ＜個別精算情報印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnFUSKYO_JOU	( MSG *msg );				// ＜不正・強制出庫情報印字要求＞	ﾒｯｾｰｼﾞ受信処理
extern	void	PrnFUSKYO_LOG	( MSG *msg );				// ＜不正・強制出庫履歴印字要求＞	ﾒｯｾｰｼﾞ受信処理
extern	void	PrnANYDATA		( MSG *msg );				// ＜各種ﾃﾞｰﾀ格納件数印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnIREKAE_TEST	( MSG *msg );				// ＜用紙入替時ﾃｽﾄ印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnPRINT_TEST	( MSG *msg );				// ＜ﾌﾟﾘﾝﾀﾃｽﾄ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnLOGO_REGIST	( MSG *msg );				// ＜ロゴ印字データ登録要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnRMON_JOU		( MSG *msg );				// ＜遠隔監視情報印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnRMON_LOG		( MSG *msg );				// ＜遠隔監視情報履歴印字要求＞		ﾒｯｾｰｼﾞ受信処理

extern	void	PrnINJI_TYUUSHI	( MSG *msg );				// ＜印字中止＞						ﾒｯｾｰｼﾞ受信処理
extern	void	PrnINJI_TYUUSHI_R( MSG *msg );				// ＜印字中止（ﾚｼｰﾄ）＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnINJI_TYUUSHI_J( MSG *msg );				// ＜印字中止（ｼﾞｬｰﾅﾙ）＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnINJI_END		( MSG *msg );				// ＜印字編集ﾃﾞｰﾀ１ﾌﾞﾛｯｸ印字終了＞	ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCHARGESETUP	( MSG *msg );				// ＜料金設定印字要求＞				ﾒｯｾｰｼﾞ受信処理
void	PrnSetDiffLogPrn( MSG *msg );
extern	void	RT_PrnSEISAN_LOG( MSG *msg );				// ＜料金テスト情報印字要求＞		ﾒｯｾｰｼﾞ受信処理

extern	void	PrnERREND_PROC	( void );					// 印字異常終了制御処理

extern	uchar	PrnRYOUSYUU_datachk	( Receipt_data *Ryousyuu );			// 領収証印字		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnAZUKARI_datachk	( Azukari_data *Azukari );			// 預り証印字		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnUKETUKE_datachk	( Uketuke_data *Uketuke );			// 受付券印字		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnSYUUKEI_datachk	( SYUKEI *Syuukei );				// 集計印字			要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnCOKI_SK_datachk	( COIN_SYU *Coinkinko );			// ｺｲﾝ金庫集計		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnSIKI_SK_datachk	( NOTE_SYU *Siheikinko );			// 紙幣金庫集計		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnTURIKAN_datachk	( TURI_KAN *TuriKan );				// 釣銭管理集計		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnERRJOU_datachk	( Err_log *Errlog );				// ｴﾗｰ情報			要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnERRLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// ｴﾗｰ情報履歴		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnARMJOU_datachk	( Arm_log *Errlog );				// ｱﾗｰﾑ情報			要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnARMLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// ｱﾗｰﾑ情報履歴		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnOPEJOU_datachk	( Ope_log *Errlog );				// 操作情報			要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnOPELOG_datachk	( T_FrmLogPriReq2 *msg_data );		// 操作情報履歴		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnMONJOU_datachk	( Mon_log *Monlog );				// ﾓﾆﾀ情報			要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnMONLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// ﾓﾆﾀ情報履歴		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnTEIKID1_datachk	( T_FrmTeikiData1 *msg_data );		// 定期有効／無効	要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnTEIKID2_datachk	( T_FrmTeikiData2 *msg_data );		// 定期入庫／出庫	要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnTFJOU_datachk	( Pon_log *Ponlog );				// 停復電情報		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnFKJOU_datachk	( flp_log *fuskyo );				// 不正・強制情報	要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnRMONJOU_datachk	( Rmon_log *Monlog );				// 遠隔監視情報		要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	PrnRMONLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// 遠隔監視情報履歴	要求ﾃﾞｰﾀﾁｪｯｸ処理
extern	void	PriErrRecieve( void );
extern	void	PrnSuica_LOG		( MSG *msg );						// ＜Suica通信ログ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnSuica_LOG2		( MSG *msg );						// ＜Suica通信ログ直近データ印字要求＞		ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCRE_USE( MSG *msg );									// ＜ｸﾚｼﾞｯﾄ利用明細ﾃﾞｰﾀ印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCRE_UNSEND( MSG *msg );								// ＜ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ印字要求＞			ﾒｯｾｰｼﾞ受信処理
extern	void	PrnCRE_SALENG( MSG *msg );								// ＜ｸﾚｼﾞｯﾄ売上拒否ﾃﾞｰﾀ印字要求＞			ﾒｯｾｰｼﾞ受信処理
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	void	PrnEdy_Status		( MSG *msg );						// ＜Edy設定＆ステータス印字要求＞			ﾒｯｾｰｼﾞ受信処理
//extern	void	PrnEdyArmPri		( MSG *msg );						// ＜Edyｱﾗｰﾑﾚｼｰﾄ印字要求＞				ﾒｯｾｰｼﾞ受信処理
//extern	uchar	Prn_edit_EdyKoteiMsg( uchar pri_kind );					// 固定ﾒｯｾｰｼﾞ編集
//extern	uchar	Edy_ArmEdit_Hakkenj( edy_arm_log *Armdata, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// Edy用発券情報
//extern	uchar	Edy_ArmEdit_Teiki( edy_arm_log *Armdata, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Edy用定期券種別、契約Ｎｏ．						
//extern	uchar	Prn_Line_edit( uchar pri_kind, uchar Line_kind );		// 仕切り線編集						
//extern	void	Prn_Edy_Err_SupplEdit( uchar Err_syu, uchar Err_cod, uchar* errdat );	// ｴﾗｰ補足情報印字処理
//extern	void	PrnEDYARMPAY_LOG( MSG *msg );
//extern	void	PrnEdyShimePri		( MSG *msg );						// ＜Edy締め記録ﾚｼｰﾄ印字要求＞				ﾒｯｾｰｼﾞ受信処理
//extern	void	PrnEDYSHIME_LOG( MSG *msg );
//extern	uchar	Edy_Ryosyu_Edit( Receipt_data *Ryousyuu, uchar pri_kind );
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
extern	void	PrnArcModVer( MSG *msg );

extern	void	PrnDigi_USE( MSG *msg );								// ＜Ｅｄｙ利用明細印字要求＞
extern	void	PrnDigi_SYU( MSG *msg );
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	void	EdyUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//extern	void	SuicaUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	SuicaUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data, short log_id );
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	void	EdySyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
extern	void	SuicaSyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(S) G.So ICクレジット対応
extern	void	EcSyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	PrnEcAlmRctPri( MSG *msg );								// 決済リーダアラームレシート印字要求
extern	void	PrnEcBrandPri( MSG *msg );								// 決済リーダブランド状態印字要求
// MH810103 GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
//extern	void	PrnEcDeemedJnlPri( MSG *msg );							// 決済リーダみなし決済復電ログ印字要求
// MH810103 GG119202(E) ICクレジットみなし決済印字フォーマット変更対応
extern	void	PrnRecvDeemedDataPri( MSG *msg );						// 決済リーダ決済精算中止(復決済)データ印字要求
// MH321800(E) G.So ICクレジット対応
extern	uchar	is_match_receipt(Receipt_data* p_dat, ulong Sdate, ulong Edate, ushort MsgId);
extern	void	PrnRrokin_Setlog(MSG *msg);
extern	void	PrnSettei_Chk(MSG *msg);
extern	void	PrnFlapLoopData(MSG *msg);
extern	void	PrnMntStack(MSG *msg);
extern	uchar Cancel_repue_chk(T_FrmPrnStop* data);
extern	char PreqMsgChk( char kind );
extern	void	PrnChkResult(MSG *msg);
extern	void	PrnPushFrontMsg( MsgBuf *msg );
extern	void	PrnDeleteExclusiveMsg( void );
// MH321800(S) G.So ICクレジット対応
extern	void	PrnEcAlmRctLogPri( MSG *msg );
// MH321800(E) G.So ICクレジット対応

// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
extern	void	PrnDEBUG( MSG *msg );
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// MH810105(S) MH364301 QRコード決済対応
extern	void	PrnFailureContactDataPri( MSG *msg );					// 決済リーダ障害連絡票データ印字要求
// MH810105(E) MH364301 QRコード決済対応

/*-------	Pri_Edit.c	-------*/
extern	uchar	RYOUSYUU_edit			( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 領収証印字ﾃﾞｰﾀ編集処理
extern	uchar	RYOUSYUU_edit_hyoudai	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 表題（領収証）
extern	uchar	SEICHUSI_edit_hyoudai	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 表題（精算中止）
extern	uchar	RYOUSYUU_edit_seisanj	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 精算情報
// GG129000(S) H.Fujinaga 2023/01/04 ゲート式車番チケットレスシステム対応（発券情報印字対応）
extern	uchar	RYOUSYUU_edit_hakkenj	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 発券情報
// GG129000(E) H.Fujinaga 2023/01/04 ゲート式車番チケットレスシステム対応（発券情報印字対応）
extern	uchar	RYOUSYUU_edit_intime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 入庫時刻
extern	uchar	RYOUSYUU_edit_outtime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 出庫時刻
extern	uchar	RYOUSYUU_edit_parktime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 駐車時間
extern	uchar	RYOUSYUU_edit_churyo	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 駐車料金
extern	uchar	RYOUSYUU_edit_tupdryo	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期更新料金
extern	uchar	RYOUSYUU_edit_teiki		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期券種別、契約Ｎｏ．
extern	uchar	RYOUSYUU_edit_service	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｻｰﾋﾞｽ券
extern	uchar	RYOUSYUU_edit_misewari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 店割引
extern	uchar	RYOUSYUU_edit_waribiki	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 割引券
extern	uchar	RYOUSYUU_edit_pripay	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );	// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
extern	uchar	RYOUSYUU_edit_kaisuu	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );	// 回数券
extern	uchar	RYOUSYUU_edit_btwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 前回時間割引
extern	uchar	RYOUSYUU_edit_brwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 前回料金割引
extern	uchar	RYOUSYUU_edit_bpwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 前回％割引
extern	uchar	RYOUSYUU_edit_tax		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 消費税
extern	uchar	RYOUSYUU_edit_ccpay		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ支払額
extern	uchar	RYOUSYUU_edit_goukei	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 合計金額
extern	uchar	RYOUSYUU_edit_azukari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 預り金額
extern	uchar	RYOUSYUU_edit_turisen	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 釣り銭額（領収証）
extern	uchar	SEICHUSI_edit_turisen	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 釣り銭額（精算中止）
extern	uchar	RYOUSYUU_edit_turikire	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 釣り切れ情報（領収証）
extern	uchar	SEICHUSI_edit_turikire	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 釣り切れ情報（精算中止）
extern	uchar	RYOUSYUU_edit_ccname	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
extern	uchar	RYOUSYUU_edit_ccid		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員Ｎｏ．
extern	uchar	RYOUSYUU_edit_ccslno	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票Ｎｏ．
extern	uchar	RYOUSYUU_edit_ccapno	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認Ｎｏ．
extern	uchar	RYOUSYUU_edit_syuuseij	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 修正精算情報
extern	uchar	RYOUSYUU_edit_pkno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 駐車位置番号
extern	uchar	RYOUSYUU_edit_tupdmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期更新時の期限案内メッセージ
extern	uchar	RYOUSYUU_edit_tukimsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar msg_no );	// 釣り切れ時の固定ﾒｯｾｰｼﾞ
// 仕様変更(S) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
extern	uchar	RYOUSYUU_edit_Kbrmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// 仕様変更(E) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
extern	uchar	RYOUSYUU_edit_Futuremsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
extern	uchar	RYOUSYUU_edit_tupdlvlmsg( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期更新時のラベル発行メッセージ
extern	uchar	RYOUSYUU_edit_tupdngmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期券更新不可時のメッセージ

extern	uchar	RYOUSYUU_edit_suica     ( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );
extern	uchar	Electron_Ryosyu_Edit	( Receipt_data *Ryousyuu, uchar pri_kind );								// 電子媒体"新"精算情報編集処理
// MH321800(S) G.So ICクレジット対応
extern	uchar	ElectronEc_Ryosyu_Edit	( Receipt_data *Ryousyuu, uchar pri_kind,  PRN_PROC_DATA *p_proc_data );	// 電子媒体"新"精算情報編集処理
// MH321800(E) G.So ICクレジット対応
extern	uchar	RYOUSYUU_edit_content	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar edit_no );	// 利用内容印字処理
extern	uchar	RYOUSYUU_edit_content_sub( char *first_work, ulong second_work, ulong third_work, uchar fourth_work, uchar pri_kind );	// 利用内容印字データ登録処理
// MH810105(S) MH364301 インボイス対応
//extern	void	RYOUSYUU_edit_title_sub	( uchar type,	char	*buf );											// 合計金額項目名編集処理
extern	void	RYOUSYUU_edit_title_sub( Receipt_data *Ryousyuu, uchar type, char *buf, PRN_PROC_DATA *p_proc_data );	// 合計金額項目名編集処理
// MH810105(E) MH364301 インボイス対応

extern	uchar	RYOUSYUU_edit_cctno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// クレジット端末識別番号
// MH321800(S) G.So ICクレジット対応 不要機能削除(CCT)
//extern	uchar	RYOUSYUU_edit_cncno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// クレジットカードセンタ処理日時・センタ通番
// MH321800(E) G.So ICクレジット対応 不要機能削除(CCT)
extern	uchar	RYOUSYUU_edit_ShopAccountNo( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data ); // クレジット加盟店取引番号
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
extern	uchar	RYOUSYUU_edit_Kaimono( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 買物割引
extern	uchar	SYUUKEI_edit_KaimonoWari( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 買物割引
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)

extern	void	AZUKARI_edit			( Azukari_data *Azukari, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 預り証印字ﾃﾞｰﾀ編集

extern	void	UKETUKE_edit			( Uketuke_data *Uketuke, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 受付券印字ﾃﾞｰﾀ編集

extern	uchar	SYUUKEI_edit			( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 集計印字ﾃﾞｰﾀ編集
extern	uchar	SYUUKEI_edit_hyoudai	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 表題
extern	uchar	SYUUKEI_edit_GenUriTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総現金売上額
extern	uchar	SYUUKEI_edit_KakeUriTryo( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総掛売額
extern	uchar	SYUUKEI_edit_UriTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総売上額
extern	uchar	SYUUKEI_edit_TaxTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総税金額
extern	uchar	SYUUKEI_edit_SeiTcnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総精算回数
extern	uchar	SYUUKEI_edit_InOutTcnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 総入・出庫台数
extern	uchar	SYUUKEI_edit_HunSei		( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 紛失精算
extern	uchar	SYUUKEI_edit_RsyuSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 料金種別毎精算
extern	uchar	SYUUKEI_edit_RsyuWari	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 料金種別毎割引
extern	uchar	SYUUKEI_edit_CcrdSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ｸﾚｼﾞｯﾄｶｰﾄﾞ精算
extern	uchar	SYUUKEI_edit_PcrdUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用
extern	uchar	SYUUKEI_edit_KtikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// 回数券使用
extern	uchar	SYUUKEI_edit_StikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ｻｰﾋﾞｽ券使用
extern	uchar	SYUUKEI_edit_MnoUse		( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// 店Ｎｏ．割引
extern	uchar	SYUUKEI_edit_WtikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// 割引券使用
extern	uchar	SYUUKEI_edit_TeikiUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// 定期券使用
extern	uchar	SYUUKEI_edit_TeikiKou	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// 定期券更新
extern	uchar	SYUUKEI_edit_BunruiSk	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 分類集計
extern	void	BunruiSk_Kubun_edit1	( uchar syu, uchar no, char *p_str );												// 分類集計集計範囲編集１
extern	void	BunruiSk_Kubun_edit2	( uchar syu, char *p_str );															// 分類集計集計範囲編集２
extern	void	BunruiSk_Kubun_edit3	( uchar syu, char *p_str );															// 分類集計集計範囲編集３
extern	uchar	SYUUKEI_edit_RyousyuuPriCnt( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 領収証発行回数
extern	uchar	SYUUKEI_edit_InSvstSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ｻｰﾋﾞｽﾀｲﾑ内精算回数
extern	uchar	SYUUKEI_edit_InLagtSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );						// ﾗｸﾞﾀｲﾑ内精算回数
extern	uchar	SYUUKEI_edit_ApassOffSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 強制ｱﾝﾁﾊﾟｽOFF精算回数
extern	uchar	SYUUKEI_edit_HaraiHusoku( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 払出不足
extern	uchar	SYUUKEI_edit_KinkoTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 金庫総入金額
extern	uchar	SYUUKEI_edit_SiheiKinko	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 紙幣金庫
extern	uchar	SYUUKEI_edit_CoinKinko	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// コイン金庫
extern	uchar	SYUUKEI_edit_KinsenData	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 金銭データ
extern	uchar	KinsenData_Coin			( ulong *data, uchar pri_kind );													// 金銭データコイン（金種毎）編集
extern	uchar	SYUUKEI_edit_SeiChusi	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 精算中止
extern	uchar	SYUUKEI_edit_JuGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 循環コイン合計回数
extern	uchar	SYUUKEI_edit_ShGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 紙幣払出機合計回数
extern	uchar	SYUUKEI_edit_CkGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// コイン金庫合計回数
extern	uchar	SYUUKEI_edit_SkGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 紙幣金庫合計回数
extern	uchar	SYUUKEI_edit_TuriModRyo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 釣銭払戻額
extern	uchar	SYUUKEI_edit_AzukariPriCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 預り証発行回数
extern	uchar	SYUUKEI_edit_KyouseiOut	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 強制出庫
extern	uchar	SYUUKEI_edit_HuseiOut	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 不正出庫
extern	uchar	SYUUKEI_edit_SyuuseiSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 修正精算
extern	uchar	SYUUKEI_edit_UketukePriCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 受付券発行回数
extern	uchar	SYUUKEI_edit_ItibetuSk	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 駐車位置別集計
extern	uchar	SYUUKEI_edit_MinyuSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );						// 未入金回数・金額

extern	uchar	SYUUKEI_edit_Electron	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// Suica使用
// MH321800(S) hosoda ICクレジット対応
extern	uchar	SYUUKEI_edit_ElectronEc(SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data);						// 決済リーダ
// MH321800(E) hosoda ICクレジット対応
extern	uchar	SYUUKEI_edit_Electron2	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// PASMO使用
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	uchar	SYUUKEI_edit_Electron3	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// Edy使用
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

extern	uchar	SYUUKEI_edit_Gengaku	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 減額精算
extern	uchar	SYUUKEI_edit_Furikae	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// 振替精算
// MH322914(S) K.Onodera 2016/12/08 AI-V対応
extern	uchar	SYUUKEI_edit_Kabarai	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH322914(E) K.Onodera 2016/12/08 AI-V対応
extern	uchar	SYUUKEI_edit_LagExtCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ラグタイム延長回数

extern	void	SYUUKEILOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// 集計履歴印字ﾃﾞｰﾀ編集

extern	void	CoKiSk_edit				( COIN_SYU *Coinkinko, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集
extern	void	CoKiJo_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ｺｲﾝ金庫集計情報印字ﾃﾞｰﾀ編集
extern	void	CoKiSk_edit_sub			( COIN_SYU *Coinkinko, uchar pri_kind );										// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集ｻﾌﾞ

extern	void	SiKiSk_edit				( NOTE_SYU *Siheikinko, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// 紙幣金庫集計印字ﾃﾞｰﾀ編集
extern	void	SiKiJo_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 紙幣金庫集計情報印字ﾃﾞｰﾀ編集
extern	void	SiKiSk_edit_sub			( NOTE_SYU *Siheikinko, uchar pri_kind );										// 紙幣金庫集計印字ﾃﾞｰﾀ編集ｻﾌﾞ

extern	uchar	TURIKAN_edit			( TURI_KAN *TuriKan, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 釣銭管理集計印字ﾃﾞｰﾀ編集処理
extern	uchar	TuriKanData_Coin		( TURI_KAN *TuriKan, uchar coin_syu, uchar pri_kind );							// 釣銭管理金種毎印字ﾃﾞｰﾀ編集処理
extern	void	TURIKANLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 釣銭管理集計履歴印字ﾃﾞｰﾀ編集	

extern	void	ERRJOU_edit				( Err_log *Errlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ｴﾗｰ情報印字ﾃﾞｰﾀ編集処理
extern	void	ERRLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集処理
extern	void	ERRJOU_edit_data		( Err_log *Errlog, uchar pri_kind, uchar type );								// ｴﾗｰ情報（１ﾃﾞｰﾀ）編集処理

extern	void	ARMJOU_edit				( Arm_log *Armlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ｱﾗｰﾑ情報印字ﾃﾞｰﾀ編集処理
extern	void	ARMLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ｱﾗｰﾑ情報履歴印字ﾃﾞｰﾀ編集処理
extern	void	ARMJOU_edit_data		( Arm_log *Errlog, uchar pri_kind );											// ｱﾗｰﾑ情報（１ﾃﾞｰﾀ）編集処理

extern	void	OPEJOU_edit				( Ope_log *Opelog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 操作情報印字ﾃﾞｰﾀ編集処理
extern	void	OPELOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集処理
extern	void	OPEJOU_edit_data		( Ope_log *Errlog, uchar pri_kind );											// 操作情報（１ﾃﾞｰﾀ）編集処理

extern	void	NgLog_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集
extern	ushort	LogPtrGet				( ushort cnt, ushort wp, ushort max, uchar req );
extern	ushort	UsMnt_IoLog_GetLogCnt(ushort no, ushort* date);
extern	void	IoLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	ushort	LogPtrGet2(ushort no, ulong *inf);															// ログデータポインタ取得処理２（入出庫情報プリント用）
extern	void	MONJOU_edit				( Mon_log *Monlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ﾓﾆﾀ情報印字ﾃﾞｰﾀ編集処理
extern	void	MONLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ﾓﾆﾀ情報履歴印字ﾃﾞｰﾀ編集処理
extern	void	MONJOU_edit_data		( Mon_log *Monlog, uchar pri_kind );											// ﾓﾆﾀ情報（１ﾃﾞｰﾀ）編集処理
extern	void	RMONJOU_edit			( Rmon_log *Rmonlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 遠隔監視情報印字ﾃﾞｰﾀ編集処理
extern	void	RMONLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 遠隔監視情報履歴印字ﾃﾞｰﾀ編集処理
extern	void	RMONJOU_edit_data		( Rmon_log *Rmonlog, uchar pri_kind );											// 遠隔監視情報（１ﾃﾞｰﾀ）編集処理

extern	void	PrnNG_LOG		( MSG *msg );				// ＜不正ログ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	PrnIO_LOG		( MSG *msg );				// ＜入出庫ログ印字要求＞			ﾒｯｾｰｼﾞ受信処理

extern	void	SETTEI_edit				( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 設定ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集
extern	uchar	SETTEI_edit_hyoudai		( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 設定ﾃﾞｰﾀ印字（表題）編集
extern	uchar	SETTEI_edit_sver		( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// 設定ﾃﾞｰﾀ印字（ｿﾌﾄﾊﾞｰｼﾞｮﾝ）編集
extern	uchar	SETTEI_edit_macaddress	( uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	uchar	SETTEI_edit_cpara		( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// 設定ﾃﾞｰﾀ印字（共通ﾊﾟﾗﾒｰﾀ）編集
extern	void	SETTEI_edit_data		( ushort addr, ulong data, uchar pos, uchar	kind );								// 設定ﾃﾞｰﾀの印字ﾃﾞｰﾀ編集処理（ｱﾄﾞﾚｽ番号＆ﾃﾞｰﾀ）

extern	void	DOUSAC_edit				( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集

extern	void	LOCKDCNT_edit			( ushort Kikai_no, uchar Req_syu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ﾛｯｸ装置動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集
extern	void	LOCKPARA_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 車室ﾊﾟﾗﾒｰﾀ印字ﾃﾞｰﾀ編集
extern	void	LOCKSETTEI_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ﾛｯｸ装置設定印字ﾃﾞｰﾀ編集

extern	void	PKJOUNOW_edit			( ushort Kikai_no, uchar pri_kind, uchar prn_menu, PRN_PROC_DATA *p_proc_data );				// 車室情報（現在）印字ﾃﾞｰﾀ編集
extern	void	PKJOUSAV_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 車室情報（退避）印字ﾃﾞｰﾀ編集
extern	void	PKJOU_edit_proc			( uchar Edit_no, ushort Kikai_no, uchar pri_kind, uchar prn_menu, PRN_PROC_DATA *p_proc_data );	// 車室情報印字ﾃﾞｰﾀ編集

extern	void	TEIKID1_edit			( 																				// 定期有効／無効印字ﾃﾞｰﾀ編集
											ushort			Kikai_no,
											ushort			Kakari_no,
											uchar			Pkno_syu,
											uchar			Req_syu,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	void	TEIKID2_edit			( 																				// 定期入庫／出庫印字ﾃﾞｰﾀ編集
											ushort			Kikai_no,
											ushort			Kakari_no,
											uchar			Pkno_syu,
											uchar			Req_syu,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	uchar	TEIKID_edit_hyoudai		( uchar Pri_syu, ushort Kikai_no, ushort Kakari_no, uchar Pkno_syu, uchar pri_kind );	// 定期印字表題ﾃﾞｰﾀ編集処理
extern	uchar	TEIKID_edit_sts			( uchar Edit_no, uchar Pkno_syu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 定期ｽﾃｰﾀｽ印字ﾃﾞｰﾀ編集処理
extern	ushort	TEIKID_edit_idcnt		( uchar Pkno_syu );																// 定期印字ﾃﾞｰﾀID数検索処理
extern	void	TEIKID_edit_id			( ushort id, uchar pos );														// 定期ID印字ﾃﾞｰﾀ編集処理

extern	void	SVSTIKK_edit			( ushort Kikai_no, ushort Kakari_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｻｰﾋﾞｽ券期限印字ﾃﾞｰﾀ編集

extern	void	SPLDAY_edit				( ushort Kikai_no, ushort Kakari_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// 特別日印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_Kikan		( uchar pri_kind, uchar check );																// 特別日（期間）印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_Day			( uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar check);									// 特別日（月日）印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_Hmon		( uchar pri_kind, uchar check );																// 特別日（ﾊｯﾋﾟｰﾏﾝﾃﾞｰ）印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_Yobi		( uchar pri_kind );																// 特別日（曜日）印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_SPYEAR		( uchar pri_kind, uchar check );																// 特別日（特別年月日）印字ﾃﾞｰﾀ編集
extern	void	SPLDAY_edit_SPWEEK		( uchar pri_kind, uchar check );																// 特別日（特別曜日）印字ﾃﾞｰﾀ編集

extern	void	TRJOU_edit				(																				// 停留車情報印字ﾃﾞｰﾀ編集
											ushort			Kikai_no,
											ushort			Kakari_no,
											short			Day,
											short			Cnt,
											short			*Data,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	void	TFJOU_edit				( Pon_log *Ponlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 停復電情報印字ﾃﾞｰﾀ編集
extern	uchar	TFJOU_edit_TimeTtl		( uchar pri_kind );																// 停復電情報：時刻ﾀｲﾄﾙ印字ﾃﾞｰﾀ編集処理
extern	uchar	TFJOU_edit_TimeData		( Pon_log *Ponlog, uchar pri_kind );											// 停復電情報：停電／復電時刻ﾃﾞｰﾀ編集処理
extern	uchar	TFJOU_edit_ClrData		( uchar pri_kind );																// 停復電情報：ｸﾘｱ情報ﾃﾞｰﾀ編集処理

extern	void	TFLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 停復電履歴印字ﾃﾞｰﾀ編集

extern	void	SEISANLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集
extern	void	FUSKYOJOU_edit			( flp_log *fuskyo, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 不正・強制出庫情報印字ﾃﾞｰﾀ編集
extern	void	FUSKYOLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 不正・強制出庫履歴印字ﾃﾞｰﾀ編集
extern	void	FUSKYOLOG_cnt_get		( T_FrmLogPriReq2 *msg_data, ushort *FusCnt, ushort *KyoCnt, PRN_PROC_DATA *p_proc_data );	// 不正・強制出庫ﾃﾞｰﾀ件数検索
extern	void	FUSKYO_edit_data		( flp_log *FusKyo, uchar pri_kind );											// 不正・強制出庫印字ﾃﾞｰﾀ編集

extern	void	ANYDATA_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// 各種ﾃﾞｰﾀ格納件数印字ﾃﾞｰﾀ編集

extern	void	IREKAETST_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// 用紙入替時ﾃｽﾄ印字ﾃﾞｰﾀ編集処理

extern	void	PRITEST_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// ﾌﾟﾘﾝﾀﾃｽﾄ印字ﾃﾞｰﾀ編集処理
extern	void	PRITEST_edit_fontB		( uchar pri_kind );																// ﾌｫﾝﾄＢﾃｽﾄ印字ﾃﾞｰﾀ編集
extern	void	PRITEST_edit_fontA		( uchar pri_kind );																// ﾌｫﾝﾄＡﾃｽﾄ印字ﾃﾞｰﾀ編集
extern	void	PRITEST_edit_Kanji		( uchar pri_kind );																// 漢字（全角）ﾃｽﾄ印字ﾃﾞｰﾀ編集

extern	void	LOGOREG_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// ロゴ印字データ登録処理
extern	void	LOGOREG_edit_sub1		( char *dptr1, char *dptr2 );													// ﾛｺﾞ印字ﾃﾞｰﾀﾋﾞｯﾄﾏｯﾌﾟﾃﾞｰﾀ変換処理

extern	void	Suica_Log_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Suica通信ﾛｸﾞ編集処理
extern	void	Suica_Log_edit2			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Suica通信ﾛｸﾞ直近データ用印字ﾃﾞｰﾀ編集
extern	uchar	PrnSuicaLOG_datachk( T_FrmLogPriReq1 *msg_data );														// Suica印字ﾛｸﾞﾃﾞｰﾀﾁｪｯｸ
extern	ushort	Suica_edit_data( char *logdata, uchar pri_kind ,ushort data_size );										// Suica通信ﾛｸﾞﾃﾞｰﾀ編集処理
extern	ushort	Suica_edit_data2( char *logdata, uchar pri_kind ,PRN_PROC_DATA *p_proc_data, ushort *wpt, struct suica_log_rec *wlogrec );	// Suica通信ﾛｸﾞﾃﾞｰﾀ編集処理
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	void	Edy_Status_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Edy設定＆ステータス編集処理
//extern	void	Edy_Arm_edit			( edy_arm_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// Edyｱﾗｰﾑﾚｼｰﾄ編集処理
//extern	void	Edy_Shime_edit			( edy_shime_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// Edy締め記録ﾚｼｰﾄ編集処理
//extern	void	EDYARMPAYLOG_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// Edyアラーム取引情報印字ﾃﾞｰﾀ編集処理
//extern	uchar	Edy_ArnPayLog_edit( edy_arm_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//extern	void	EDYSHIMELOG_edit( T_FrmLogPriReq2 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//extern	uchar	Edy_ShimeLog_edit( edy_shime_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
extern	void	Ec_AlmRct_edit		(T_FrmEcAlmRctReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// 決済リーダアラームレシートﾃﾞｰﾀ編集処理
extern	uchar	Ec_AlmRct_edit_sub	(EC_SETTLEMENT_RES *almrct, uchar pay_brand, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// 決済リーダアラームレシートﾃﾞｰﾀ編集処理サブ
extern	void	Ec_Brand_edit		(T_FrmEcBrandReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// 決済リーダブランド状態ﾃﾞｰﾀ編集処理
extern	void	Ec_AlmRctLog_edit(T_FrmEcAlarmLog *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理
// MH810105 GG119202(S) T合計連動印字対応
//extern	void	Ec_AlmRctLog_edit_sub(uchar pri_kind, PRN_PROC_DATA *p_proc_data);										// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理
extern	void	Ec_AlmRctLog_edit_sub(uchar pri_kind, ulong StartTime, ulong EndTime, PRN_PROC_DATA *p_proc_data);	// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理
// MH810105 GG119202(E) T合計連動印字対応
// MH810103 GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
//extern	void	Ec_DeemedJnl_edit(T_FrmEcDeemedJnlReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// みなし決済復電ログ印字ﾃﾞｰﾀ編集処理
// MH810103 GG119202(E) ICクレジットみなし決済印字フォーマット変更対応
extern	uchar	Recv_DeemedData_edit(Receipt_data *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);				// 決済精算中止(復決済)ﾃﾞｰﾀ編集処理
// MH321800(E) G.So ICクレジット対応
// MH810105 GG119202(S) T合計連動印字対応
extern	void	SYUUKEI_EC_edit(SYUKEI *Syuukei, uchar pri_kind, ushort print_flag, T_FrmTgoukeiEc *Ec_data, PRN_PROC_DATA *p_proc_data);	// 集計印字ﾃﾞｰﾀ編集
// MH810105 GG119202(E) T合計連動印字対応

extern	void	CreUseLog_edit			( T_FrmLogPriReq3 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 利用明細ログ印字ﾃﾞｰﾀ編集
extern	void	CreUseLog_edit_ccid		( char *dat, uchar pri_kind );													// 利用明細ログ用カード番号編集
extern	void	CreUnSend_edit			( T_FrmUnSendPriReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ編集処理
extern	void	CreSaleNG_edit			( T_FrmSaleNGPriReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ｸﾚｼﾞｯﾄ売上拒否ﾃﾞｰﾀ編集処理

extern	uchar	Prn_edit_logo( uchar pri_kind );																// ﾛｺﾞ印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_headhoot( uchar edit_kind, uchar pri_kind );											// ﾍｯﾀﾞｰ／ﾌｯﾀｰ印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_headhoot_new ( uchar start_num, uchar count, uchar edit_kind, uchar pri_kind );
extern	uchar	Prn_edit_sikiri( uchar pri_kind );																// 仕切り行印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_space( uchar pri_kind );																// スペース行印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_TurikireMsg( uchar msg_no, uchar pri_kind );											// 釣り切れ時の固定ﾒｯｾｰｼﾞ編集
extern	uchar	Prn_edit_KoteiMsg( uchar msg_no, uchar pri_kind );												// 固定ﾒｯｾｰｼﾞ編集
extern	uchar	Prn_edit_Title( const char *prn_data, uchar font, uchar pri_kind );									// 表題印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_KaiKin( const char *prn_data, ulong kaisuu, ulong kingaku, uchar pri_kind );					// 回数／金額印字ﾃﾞｰﾀ編集

extern	uchar	Prn_edit_KenKin( const char *prn_data, ulong kensuu, ulong kingaku, uchar pri_kind );
extern	uchar	Prn_edit_KaiKin_sub( char ptn, const char *prn_data, ulong kaisuu, ulong kingaku, uchar pri_kind );

extern	uchar	Prn_edit_Sitei1line( const char *prn_data, ulong data, const char *tanni1, const char *tanni2, uchar pri_kind );	// 単位指定１行印字ﾃﾞｰﾀ編集
// MH321800(S) Y.Tanizaki ICクレジット対応
extern	uchar	Prn_edit_Sitei1line_2( const char *prn_data, ulong data, const char *tanni1, const char *tanni2, uchar pri_kind );	// 単位指定１行印字ﾃﾞｰﾀ編集(*付表示)
// MH321800(E) Y.Tanizaki ICクレジット対応
extern	uchar	Prn_edit_SkDate( date_time_rec	*NowTime, date_time_rec	*OldTime, uchar pri_kind );				// 集計日時（今回／前回）印字ﾃﾞｰﾀ編集
extern	void	Prn_edit_NowDate( char *edit_wk );																// 現在日時印字ﾃﾞｰﾀ編集
extern	void	Prn_edit_TanKin( char *edit_wk, ulong kingaku );												// 単位金額印字ﾃﾞｰﾀ編集
extern	void	Prn_edit_TanMai( char *edit_wk, ulong maisuu );													// 単位枚数印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_pkposi( char *edit_wk, ulong pk_posi, uchar space );									// 駐車位置（区画／番号）印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_Date( char *edit_wk, date_time_rec *date );											// 日時印字ﾃﾞｰﾀ編集	："yyyy年mm月dd日(ｗ) hh:mm"
extern	uchar	Prn_edit_Date2( char *edit_wk, date_time_rec *date );											// 日時印字ﾃﾞｰﾀ編集	："yy.mm.dd hh:mm"
extern	uchar	Prn_edit_Date3( char *edit_wk, date_time_rec *date );											// 日時印字ﾃﾞｰﾀ編集	："yyyy年mm月dd日(ｗ) hh:mm"
extern	uchar	Prn_edit_Date4( char *edit_wk, date_time_rec *date );											// 日時印字ﾃﾞｰﾀ編集	："yyyy年mm月dd日(ｗ)"
extern	uchar	Prn_edit_RePriDate( uchar pri_kind );															// 再ﾌﾟﾘﾝﾄ日時印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_TstPriDate( uchar pri_kind );															// ﾃｽﾄﾌﾟﾘﾝﾄ日時印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_SyomeiKamei( uchar edit_kind, uchar pri_kind );										// 支払ｺｰﾄﾞ・署名欄／加盟店名印字ﾃﾞｰﾀ編集
// MH321800(S) D.Inaba ICクレジット対応
extern	uchar	Prn_edit_Ec_Syomei( Receipt_data *Ryousyuu, uchar pri_kind );									// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ編集
// MH321800(E) D.Inaba ICクレジット対応
extern	uchar	Prn_edit_KakariNo( ushort Kakari_no, uchar pri_kind );											// 係員Ｎｏ．印字ﾃﾞｰﾀ編集

extern	void	KakariNoSet( char *data, ushort no );															// 係員№印字ﾃﾞｰﾀ編集

extern	void	KikaiNoSet( char *data, ushort no );															// 機械№印字ﾃﾞｰﾀ編集

extern	void	SeisanKindSet( char *data, uchar kind );														// 精算方式印字ﾃﾞｰﾀ編集
extern	void	OibanSet( char *data, ulong no, uchar name_no );												// 追番印字ﾃﾞｰﾀ編集
// GG129000(S) H.Fujinaga 2023/01/04 ゲート式車番チケットレスシステム対応（発券情報印字対応）
extern	void	HakkenKiNoSet( char *data, uchar no );															// 発券機№.印字ﾃﾞｰﾀ編集
extern	void	HakkenNoSet( char *data, ulong no );															// 発券№.	印字ﾃﾞｰﾀ編集
// GG129000(E) H.Fujinaga 2023/01/04 ゲート式車番チケットレスシステム対応（発券情報印字対応）
extern	void	Camma_Set( uchar *ucTmp );																		// ３桁区切り処理
extern	void	Str_Set1( const char *cFirst, const char *cSecond, uchar pri_kind );										// 一行分の印字文字列設定(印字)処理１
extern	void	Str_Set2( const char *cFirst, const char *cSecond, const char *cThird, const char *cFourth, uchar pri_kind );			// 一行分の印字文字列設定(印字)処理２
extern	ushort	Moj_Point( const char *data );																		// 文字幅演算処理
extern	ushort	Spc_Add( char *data, ushort cnt );																// ｽﾍﾟｰｽ挿入処理
extern	void	StrAdd( uchar *ucTmpA, uchar *ucTmpB );															// 文字列の追加処理
extern	void	Prn_LogoHead_PreHead ( void );
extern	void	Prn_LogoHead_PostHead ( void );
extern	void	Prn_LogoHead_Init ( void );
extern	uchar	Prn_LogHead_PostHead_Offs;			/* ﾍｯﾀﾞｰを後で出す場合の開始設定オフセット == 0:印字なし */

extern	void	ArcModVer_edit( T_FrmLogPriReq1 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	uchar	SearchTypeData_edit		( T_FrmLogPriReq2 *data, PRN_PROC_DATA *p_proc_data, const uchar *str );				// 検索方法データ編集(時刻あり)
extern	uchar	SearchTypeData_edit2	( T_FrmLogPriReq2 *data, PRN_PROC_DATA *p_proc_data, const uchar *str );				// 検索方法データ編集(時刻なし)
extern	void	PrnAtendValidData( MSG *msg );																	// ＜係員有効データ印字要求＞				ﾒｯｾｰｼﾞ受信処理
extern	void	AtendValidData_edit( ushort Kikai_no, ushort Kakari_no, uchar	pri_kind, PRN_PROC_DATA	*p_proc_data );// 係員有効印字ﾃﾞｰﾀ編集
extern  void	AtendValid_edit_id( ushort id, uchar pos );														// 係員有効ﾃﾞｰﾀ印字位置編集
extern	uchar	AtendValid_edit_hyoudai( ushort Kikai_no, ushort Kakari_no, uchar pri_kind );					// 表題（係員有効ﾃﾞｰﾀ）
extern  uchar	AtendValid_edit_sts( uchar pri_kind, PRN_PROC_DATA *p_proc_data );								// 係員有効ﾃﾞｰﾀ印字処理
extern	void	ChargeSetup_edit( uchar pri_kind, PRN_PROC_DATA *p_proc_data, T_FrmChargeSetup *msg);			// 料金設定プリント印字ﾃﾞｰﾀ編集
extern	void	PrnSetDiffLog_edit(T_FrmLogPriReq1 *data, PRN_PROC_DATA *p_proc_data, uchar pri_kind);			// 設定更新履歴印字ﾃﾞｰﾀ編集
extern	uchar	Prn_edit_T_foot_edit ( uchar edit_kind, uchar pri_kind );
extern	uchar	Prn_edit_AzuFtr_edit ( uchar pri_kind );
// 仕様変更(S) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
extern	uchar	Prn_edit_KbrFtr_edit ( uchar pri_kind, ushort kind );
// 仕様変更(E) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
extern	uchar	Prn_edit_FutureFtr_edit ( uchar pri_kind );
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
extern	void	PrnRrokin_Setlog_edit( T_FrmLogPriReq2 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	PrnSettei_Chk_edit(T_FrmAnyData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	PrnFlapLoopData_edit(T_FrmAnyData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	Mnt_Stack_edit( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	ChkResult_edit( T_FrmChk_result *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	RT_SEISANLOG_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集
// MH810105(S) MH364301 QRコード決済対応
extern	uchar	Recv_FailureContactData_edit(Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// 障害連絡票ﾃﾞｰﾀ編集処理
extern	uchar	Prn_edit_EmgFooter_edit( uchar pri_kind );
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 インボイス対応
extern	uchar	Disp_Tax_Value( date_time_rec *time );
extern	short	date_exist_check(short y, short m, short d);
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
extern	uchar	RYOUSYUU_edit_taxableDiscount( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );
extern	uchar	RYOUSYUU_edit_totalAmount_taxRate( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
extern	void	PrnGetRegistNum(void);
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）

/*-------	Pri_Cmn.c	-------*/
extern	void	PrnInit( void );										// ﾌﾟﾘﾝﾀ初期化処理
extern	void	PrnInit_R( void );										// ﾌﾟﾘﾝﾀ初期化処理（ﾚｼｰﾄ）
extern	void	PrnInit_J( void );										// ﾌﾟﾘﾝﾀ初期化処理（ｼﾞｬｰﾅﾙ）

extern	void	PrnMode_idle( uchar pri_kind );							// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定
extern	void	PrnMode_idle_R( void );									// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ﾚｼｰﾄ）
extern	void	PrnMode_idle_J( void );									// ﾌﾟﾘﾝﾀ印字ﾓｰﾄﾞ初期状態設定（ｼﾞｬｰﾅﾙ）

extern	void	PrnFontSize( uchar font, uchar pri_kind );				// ﾌﾟﾘﾝﾀ印字文字ｻｲｽﾞ設定

extern	void	prn_proc_data_clr( uchar pri_kind );					// 編集処理制御ﾃﾞｰﾀ初期化
extern	void	prn_proc_data_clr_R( void );							// 編集処理制御ﾃﾞｰﾀ初期化（ﾚｼｰﾄ）
extern	void	prn_proc_data_clr_J( void );							// 編集処理制御ﾃﾞｰﾀ初期化（ｼﾞｬｰﾅﾙ）

extern	void	prn_edit_wk_clr( void );								// 編集処理共通ﾜｰｸｴﾘｱ初期化

extern	uchar	Inji_Cancel_chk( uchar pri_kind );						// 印字中止ﾒｯｾｰｼﾞ受信ﾁｪｯｸ

extern	void	Inji_Cancel( MSG *msg, uchar pri_kind );				// 印字中止処理
extern	void	Inji_Cancel_R( MSG *msg );								// 印字中止処理（ﾚｼｰﾄ）
extern	void	Inji_Cancel_J( MSG *msg );								// 印字中止処理（ｼﾞｬｰﾅﾙ）

extern	void	Inji_ErrEnd( ushort command, uchar pri_sts, uchar pri_kind );	// 印字異常終了処理
extern	void	Inji_ErrEnd_R( ushort command, uchar pri_sts );			// 印字異常終了処理（ﾚｼｰﾄ）
extern	void	Inji_ErrEnd_J( ushort command, uchar pri_sts );			// 印字異常終了処理（ｼﾞｬｰﾅﾙ）

extern	void	End_Set( MSG *msg, uchar pri_kind );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理
extern	void	End_Set_R( MSG *msg );									// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理（ﾚｼｰﾄ）
extern	void	End_Set_J( MSG *msg );									// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了処理（ｼﾞｬｰﾅﾙ）

extern	void	PrnEndMsg( 	ushort	comm,								// 印字終了ﾒｯｾｰｼﾞ送信処理
							uchar	result,
							uchar	stat,
							uchar	pri_kind );
extern	void	PrnEndMsg_R(ushort	comm,								// 印字終了ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
							uchar	result,
							uchar	stat );
extern	void	PrnEndMsg_J(ushort	comm,								// 印字終了ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）
							uchar	result,
							uchar	stat );

extern	void	PrnNext( MSG *Msg, uchar pri_kind );					// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理
extern	void	PrnNext_R( MSG *Msg );									// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ﾚｼｰﾄ）
extern	void	PrnNext_J( MSG *Msg );									// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信処理（ｼﾞｬｰﾅﾙ）

extern	void	MsgSndFrmPrn(	ushort cmd,								// ﾌﾟﾘﾝﾀﾀｽｸへのﾒｯｾｰｼﾞ送信
								uchar data1,
								uchar data2 );

extern	uchar	PrnJnlCheck( void );									// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続有無ﾁｪｯｸ
extern	uchar	PrnGoukeiChk( ushort command );							// 合計記録印字要求の判定
extern	uchar	PrnGoukeiPri( ushort cmd );								// 合計記録の印字先ﾌﾟﾘﾝﾀ検索
extern	uchar	PrnAsyuukeiPri( void );									// 自動集計の印字先ﾌﾟﾘﾝﾀ検索
extern	uchar	PriStsCheck( uchar pri_kind );							// ﾌﾟﾘﾝﾀ状態（印字可／不可）ﾁｪｯｸ
extern	uchar	PriOutCheck( uchar pri_kind );							// ﾌﾟﾘﾝﾀ送信ﾃﾞｰﾀﾊﾞｯﾌｧ状態ﾁｪｯｸ

extern	void	PrnCmdLen(	const uchar *dat,									// ﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理
							ushort len,
							uchar pri_kind );
extern	void	PrnCmdLen_R( const uchar *dat , ushort len );			// ﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理（ﾚｼｰﾄ）
extern	void	PrnCmdLen_J( const uchar *dat , ushort len );			// ﾌﾟﾘﾝﾀへのｺﾏﾝﾄﾞ編集処理（ｼﾞｬｰﾅﾙ）

extern	void	PrnCmd( char *dat, uchar pri_kind );					// ﾌﾟﾘﾝﾀへの文字列ﾃﾞｰﾀ編集処理
extern	void	PrnCmd_R( char *dat );									// ﾌﾟﾘﾝﾀへの文字列ﾃﾞｰﾀ編集処理（ﾚｼｰﾄ）
extern	void	PrnCmd_J( char *dat );									// ﾌﾟﾘﾝﾀへの文字列ﾃﾞｰﾀ編集処理（ｼﾞｬｰﾅﾙ）

extern	void	PrnStr( const char *dat, uchar pri_kind );					// ﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理
extern	void	PrnStr_R( const char *dat );							// ﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理（ﾚｼｰﾄ）
extern	void	PrnStr_J( const char *dat );							// ﾌﾟﾘﾝﾀへの１行分文字列ﾃﾞｰﾀ編集処理（ｼﾞｬｰﾅﾙ）
extern	void	PrnStr_SPDAY( char *dat, uchar pos );

extern	void	PrnOut( uchar pri_kind );								// ﾌﾟﾘﾝﾀ起動処理
extern	void	PrnOut_R( void );										// ﾌﾟﾘﾝﾀ起動処理（ﾚｼｰﾄ）
extern	void	PrnOut_J( void );										// ﾌﾟﾘﾝﾀ起動処理（ｼﾞｬｰﾅﾙ）

extern	void	PrnStop( uchar pri_kind );								// ﾌﾟﾘﾝﾀ印字停止
extern	void	PrnStop_R( void );										// ﾌﾟﾘﾝﾀ印字停止（ﾚｼｰﾄ）
extern	void	PrnStop_J( void );										// ﾌﾟﾘﾝﾀ印字停止（ｼﾞｬｰﾅﾙ）

extern	void	Rct_top_edit( uchar cut );								// ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ編集処理（ﾛｺﾞ印字／ﾍｯﾀﾞｰ印字／用紙ｶｯﾄ制御）
extern	void	Kaigyou_set(	uchar pri_kind,							// 改行ﾃﾞｰﾀ編集処理（改行量＝設定ﾃﾞｰﾀ）
								uchar font,
								uchar cnt );
extern	void	Kaigyou_set2(	uchar pri_kind,							// 改行ﾃﾞｰﾀ編集処理２（改行量＝ﾊﾟﾗﾒｰﾀ指定）
								uchar font,
								uchar cnt,
								uchar data );
extern	void	Feed_set( uchar pri_kind, uchar size );					// 紙送りﾃﾞｰﾀ編集処理

extern	uchar	DateChk( date_time_rec *date );							// 日付ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	Prn_data_chk( uchar req, ulong data );					// 印字ﾃﾞｰﾀﾁｪｯｸ処理
extern	uchar	Prn_log_check(	ushort cnt,								// 各種ﾛｸﾞ情報（ｶｳﾝﾀ、ﾎﾟｲﾝﾀ）ﾁｪｯｸ処理
								ushort wp,
								ushort max,
								uchar log_no );
extern	ushort	Prn_log_search1( ushort cnt, ushort wp, ushort max );	// 各種ﾛｸﾞ情報の最古ﾃﾞｰﾀ検索処理
extern	ushort	Prn_log_search2( ushort no, ushort max );				// 各種ﾛｸﾞ情報のﾃﾞｰﾀﾎﾟｲﾝﾀ更新処理
extern	void	Prn_pkposi_chg( ulong no, ushort *area, ushort *pkno );	// 駐車位置ﾃﾞｰﾀ変換処理
extern	ulong	Prn_pkposi_make( ushort lk_no );						// 駐車位置ﾃﾞｰﾀ作成処理
extern	void	Prn_errlg( uchar err_no, uchar knd, uchar data );		// ﾌﾟﾘﾝﾀ制御部ｴﾗｰ情報登録処理
extern uchar	Prn_edit_sya_syu( char *edit_wk, ushort lk_no ,uchar flg ,uchar M_Flg);		//

extern	uchar Printer_Lifter_Wait( char event );

extern	uchar Prn_JRprintSelect( uchar set_wk, uchar p_kind );
extern	void	RP_I2CSndReq( int type );
extern	void	JP_I2CSndReq( int type );
extern	void	Back_Feed_set( uchar , uchar );
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
extern	uchar	PriJrnExeCheck( void );
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

// MH810104 GG119201(S) 電子ジャーナル対応
extern	void	PrnCmd_InfoReq(uchar kind);
extern	void	PrnCmd_InfoResTimeout(void);
extern	uchar	PrnCmd_InfoRes_Proc(uchar *pBuf, ushort RcvSize);
extern	void	PrnCmd_InfoRes_Msg(void);
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
extern	void	PrnCmd_Clock(void);
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）
extern	void	PrnCmd_InitSetting(void);
extern	void	PrnCmd_ResetTimeout(void);
extern	void	PrnCmd_WriteStartEnd(uchar kind, MSG *msg);
extern	uchar	PrnCmd_CheckSendData(uchar *pBuf);
extern	void	PrnCmd_Sts_Proc(void);
extern	void	PrnCmd_EJAErrRegist(void);
extern	uchar	PrnCmd_ReWriteCheck(void);
extern	void	PrnCmd_ReWriteStart(uchar kind);
extern	void	PrnCmd_EJAReset(uchar kind);
extern	void	PrnCmd_MsgResend(MSG *msg);
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
extern	ulong	PrnDat_WriteQue(ushort command, uchar *buf, ushort size);
extern	void	PrnDat_ReadQue(MSG *msg);
extern	void	PrnDat_DelQue(ushort command, ulong job_id);
extern	ushort	PrnDat_GetQueCount(void);
extern	ulong	PrnDat_GetJobID(void);
extern	void	PrnMsgBoxClear(void);
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
extern	void	PrnDEBUG_edit(T_FrmDebugData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
extern	void	QrCode_set( uchar pri_kind, uchar size, char *string, ushort length  );		// QRコード編集処理
extern	uchar	Prn_edit_qrcode( Receipt_data *Ryousyuu );									// QRコード編集
extern	uchar	RYOUSYUU_edit_PAYQR( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
extern	void	Prn_GetModel_R(void);
extern	void	PrnCmd_InfoReq_R(uchar kind);
extern	void	PrnCmd_InfoResTimeout_R(void);
extern	uchar	PrnCmd_InfoRes_Proc_R(uchar *pBuf, ushort RcvSize);
extern	void	PrnCmd_InfoRes_Msg_R(void);
extern	void	Prn_Model_Chk_R(void);
extern	void	PrnRYOUSYUU_StockPrint(void);
extern	void	PrnRYOUSYUU_StockClear(void);
extern	void	PrnCmd_PLayout_Design(uchar pri_kind, uchar dt);
extern	void	Prn_NewPrint_Clear_R(void);
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

/*-------	Pri_Data.c	-------*/
extern	PRN_DATA_BUFF	rct_prn_buff;			// ﾚｼｰﾄ(SCI0)	送受信ﾃﾞｰﾀ管理ﾊﾞｯﾌｧ
extern	PRN_DATA_BUFF	jnl_prn_buff;			// ｼﾞｬｰﾅﾙ(SCI3)	送受信ﾃﾞｰﾀ管理ﾊﾞｯﾌｧ

// MH810104 GG119201(S) 電子ジャーナル対応
extern	PRN_EJA_DATA_BUFF	eja_prn_buff;		// 電子ジャーナル受信データ管理
#define	isEJA_USE()			(eja_prn_buff.EjaUseFlg > 0)		// 電子ジャーナル接続あり
#define	isPrnInfoReq()		(eja_prn_buff.PrnInfoReq != 0)		// プリンタ情報要求中
#define	SetPrnInfoReq(x)	(eja_prn_buff.PrnInfoReq = (x))		// プリンタ情報要求セット
#define	isPrnInfoInitRecv()	(eja_prn_buff.PrnRcvBufReadCnt == 0)		// プリンタ情報要求の初回受信
#define	isPrnInfoRes(x)		(((x) == 0xFF) || (((x) & 0xC0) == 0xC0))	// プリンタ情報応答
																		// 1バイト目=0xFF（電子ジャーナル）
																		// 1バイト目のbit 7,6が1（ジャーナルプリンタ）
#define	isPrnRetryOver()	(eja_prn_buff.PrnHResetRetryCnt > 0)		// データ書込みリトライオーバー
extern	uchar			eja_work_buff[PRN_EJA_BUF_SIZE];		// 電子ジャーナル用ワークバッファ
// MH810104 GG119201(E) 電子ジャーナル対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
extern	PRN_RCV_DATA_BUFF_R	pri_rcv_buff_r;
extern	uchar			ryo_stock;				// レシートデータストックフラグ 0:ストックなし 1:ストックあり 2:ストック破棄 0xff:ストック印字完了
#define	isNewModel_R()		(pri_rcv_buff_r.NewModelFlg == 1)			// レシートプリンタ新モデル接続中
#define	SetModelMiss_R(x)	(pri_rcv_buff_r.ModelMissFlg = (x))			// レシートプリンタモデル設定不一致エラーフラグセット
#define	isModelMiss_R()		(pri_rcv_buff_r.ModelMissFlg == 1)			// レシートプリンタモデル設定不一致エラー
#define	SetPrnInfoReq_R(x)	(pri_rcv_buff_r.PrnInfoReq = (x))			// レシートプリンタ情報要求セット
#define	isPrnInfoReq_R()	(pri_rcv_buff_r.PrnInfoReq != 0)			// レシートプリンタ情報要求中
#define	isPrnInfoRes_R(x)	((x) == 0xFF)								// プリンタ情報応答 1バイト目=0xFF（電子ジャーナル）
#define	isPrnInfoInitRecv_R()(pri_rcv_buff_r.PrnRcvBufReadCnt == 0)		// プリンタ情報要求の初回受信
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

extern	PRN_PROC_DATA	rct_proc_data;			// ﾚｼｰﾄ		印字処理制御ﾃﾞｰﾀ
extern	PRN_PROC_DATA	jnl_proc_data;			// ｼﾞｬｰﾅﾙ	印字処理制御ﾃﾞｰﾀ

extern	uchar			rct_init_sts;			// ﾚｼｰﾄ		初期化状態(０：初期化完了待ち／１：初期化完了／２：初期化失敗／３：未接続)
extern	uchar			jnl_init_sts;			// ｼﾞｬｰﾅﾙ	初期化状態(０：初期化完了待ち／１：初期化完了／２：初期化失敗／３：未接続)
extern	uchar			rct_init_rty;			// ﾚｼｰﾄ		初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲｶｳﾝﾀｰ
extern	uchar			jnl_init_rty;			// ｼﾞｬｰﾅﾙ	初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲｶｳﾝﾀｰ

extern	uchar			GyouCnt_r;				// ﾚｼｰﾄ		送信済み印字ﾃﾞｰﾀ行数
extern	uchar			GyouCnt_j;				// ｼﾞｬｰﾅﾙ	送信済み印字ﾃﾞｰﾀ行数

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
extern	uchar			GyouCnt_All_r;			// ﾚｼｰﾄ		送信済み印字ﾃﾞｰﾀ全行数
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
extern	uchar			GyouCnt_All_j;			// ｼﾞｬｰﾅﾙ	送信済み印字ﾃﾞｰﾀ全行数
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

extern	MSG 			NextMsg_r;				// ﾚｼｰﾄ		印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧ
extern	MSG 			NextMsg_j;				// ｼﾞｬｰﾅﾙ	印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧ

extern	uchar			PriDummyMsg;			// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録状態（ON：登録あり／OFF：登録なし）
extern	MsgBuf			*TgtMsg;				// 処理対象ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ

extern	uchar			terget_pri;				// 処理対象ﾌﾟﾘﾝﾀ種別
extern	uchar			next_prn_msg;			// 印字処理中の次編集要求ﾒｯｾｰｼﾞﾌﾗｸﾞ	（ON：次編集要求／OFF：新規編集要求）
extern	uchar			syuukei_kind;			// 印字処理中の集計種別

extern	uchar			rct_goukei_pri;			// 合計記録印字状態（ﾚｼｰﾄ用）	※ON：印字処理中
extern	uchar			jnl_goukei_pri;			// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）	※ON：印字処理中
extern	uchar			rct_atsyuk_pri;			// 自動集計印字状態（ﾚｼｰﾄ用）	※ON：印字処理中
extern	uchar			jnl_atsyuk_pri;			// 自動集計印字状態（ｼﾞｬｰﾅﾙ用）	※ON：印字処理中
extern	uchar			turikan_pri_status;		// 釣銭管理ﾌﾟﾘﾝﾄが印字終了をｾｯﾄする

extern	uchar			prn_zero;				// 集計ﾃﾞｰﾀ０時印字有無（設定ﾃﾞｰﾀ）
extern	date_time_rec	Repri_Time;				// 再ﾌﾟﾘﾝﾄ日時（前回集計印字用）
extern	uchar			Repri_kakari_no;		// 再ﾌﾟﾘﾝﾄ係員No.

extern	uchar			header_hight;			// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀの縦ｻｲｽﾞ（ﾄﾞｯﾄ数）

extern	uchar			gyoukan_data;			// 改行幅（ﾄﾞｯﾄ数）指定（＝０:設定ﾃﾞｰﾀに従う／≠０:本指定ﾃﾞｰﾀに従う）

extern	char			prn_work[64];			// 印字処理ﾜｰｸ
extern	char			cTmp1[256];				// 印字処理ﾜｰｸ
extern	char			cTmp2[64];				// 印字処理ﾜｰｸ
extern	char			cMnydata[64];			// 印字処理ﾜｰｸ

extern	char			cEditwk1[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ１
extern	char			cEditwk2[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ２
extern	char			cEditwk3[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ３

extern	char			cErrlog[160];			// ｴﾗｰﾛｸﾞ用

extern	ulong			Header_Rsts;			// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ読込状態
extern	ulong			Footer_Rsts;			// ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			Header_Data[4][36];		// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ
extern	char			Footer_Data[4][36];		// ﾌｯﾀｰ印字ﾃﾞｰﾀ

extern	ulong			AcceptFooter_Rsts;			// 受付券ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			AcceptFooter_Data[4][36];	// 受付券ﾌｯﾀｰ印字ﾃﾞｰﾀ

extern	char			Logo_Fdata[24][56];		// ﾛｺﾞ印字ﾃﾞｰﾀ読込ﾜｰｸ
extern	char			Logo_Fla_data[24];		// ﾛｺﾞ印字ﾃﾞｰﾀ変換ﾜｰｸ1（ﾌﾗｯｼｭﾒﾓﾘのﾃﾞｰﾀ）
extern	char			Logo_Reg_data[24];		// ﾛｺﾞ印字ﾃﾞｰﾀ変換ﾜｰｸ2（ﾌﾗｯｼｭﾒﾓﾘを変換したﾃﾞｰﾀ）
extern	uchar			Logo_Reg_sts_rct;		// ﾛｺﾞ登録処理状態（ﾚｼｰﾄ）
extern	uchar			Logo_Reg_sts_jnl;		// ﾛｺﾞ登録処理状態（ｼﾞｬｰﾅﾙ）

extern	t_Pri_Work		Pri_Work[];				// プリンタタスクワークエリア

extern	const char 		*Sya_Prn[];				//車種

extern	ulong			Syomei_Rsts;			// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ読込状態
extern	ulong			Kamei_Rsts;				// 加盟店名印字ﾃﾞｰﾀ読込状態
extern	char			Syomei_Data[4][36];		// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ
extern	char			Kamei_Data[4][36];		// 加盟店名印字ﾃﾞｰﾀ

extern	uchar			JR_Print_Wait_tim[2];	// ｼﾞｬｰﾅﾙ・ﾚｼｰﾄﾌﾟﾘﾝﾝﾀ印字開始待ちﾀｲﾏ
extern	uchar			f_Prn_R_SendTopChar;	// ﾚｼｰﾄﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前フラグ
extern	uchar			f_Prn_J_SendTopChar;	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前フラグ
extern	ushort			Pri_Tasck[TSKMAX][2];		// タスク使用量、％
extern	ulong			Pri_Pay_Syu[2][2];			// 精算、集計データサイズ
extern	uchar			Pri_program;				// デバッグNo
extern	uchar 			Cal_Parameter_Flg;	// 設定値異常フラグ 0:正常/1:異常
extern	uchar			f_partial_cut;				// パーシャルカット動作フラグ(パーシャルカットが実行されたことを示すフラグ 0:未実行 1:実行)
extern	uchar			rct_timer_end;			// レシートプリンタ印字完了後タイマー
extern	uchar			PriBothPrint;			// プリンタ＋ジャーナル印字フラグ
extern	uchar			BothPrnStatus;			// プリンタ＋ジャーナル印字のエラーステータス
extern	ushort			BothPrintCommand;		// プリンタ＋ジャーナル印字のエラーコマンド
// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
extern	uchar			ryo_inji;				// レシートプリンタ領収証印字状態 0=印字中ではない/1=印字中
// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
extern	EC_SETTLEMENT_RES	Deemedalmrct;
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

/*-------	memdata.c	-------*/
extern	Uketuke_data	UketukeData;			// 受付券印字ﾃﾞｰﾀ
extern	Azukari_data	AzukariData;			// 預り証印字ﾃﾞｰﾀ

/*-------	int_ram.c	-------*/
extern uchar			PriErrCount;			// ｼﾘｱﾙｴﾗｰ割り込みﾌﾗｸﾞ
extern	ulong			TCardFtr_Rsts;			// Tカードﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			TCardFtr_Data[T_FOOTER_GYO_MAX][36];	// Tカードﾌｯﾀｰ印字ﾃﾞｰﾀ

extern	ulong			AzuFtr_Rsts;			// Tカードﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			AzuFtr_Data[AZU_FTR_GYO_MAX][36];	// Tカードﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
extern	ulong			CreKbrFtr_Rsts;							// クレジット過払いﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			CreKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// クレジット過払いﾌｯﾀｰ印字ﾃﾞｰﾀ
extern	ulong			EpayKbrFtr_Rsts;						// 電子マネー過払いﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			EpayKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// 電子マネー過払いﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
extern	ulong			FutureFtr_Rsts;							// 後日支払額ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			FutureFtr_Data[FUTURE_FTR_GYO_MAX][36];	// 後日支払額ﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
extern	ulong			EmgFooter_Rsts;							// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
extern	char			EmgFooter_Data[EMGFOOT_GYO_MAX][36];	// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀ
// MH810105(E) MH364301 QRコード決済対応
// GG129001(S) データ保管サービス対応（登録番号をセットする）
extern	uchar			RegistNum[14];			// インボイスの登録番号（T+13桁の数字）
// GG129001(E) データ保管サービス対応（登録番号をセットする）

#endif	//_PRI_DEF_H_
