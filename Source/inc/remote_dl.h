#ifndef	_REMOTE_DL_TASK_H_
#define	_REMOTE_DL_TASK_H_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	remote download function <br><br>
///	<b>Copyright(C) 2010 AMANO Corp.</b>
///	CREATE			2010/06/04 Namioka<br>
///	UPDATE			
///	@file			remote_dl.c
///	@date			2010/06/04
///	@version		MH702207
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]


#include "ntnet_def.h"
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
#define CHG_INFO_VERSION					1
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)
typedef enum{
	VER_TYPE_MH,
	VER_TYPE_GM,
	VER_TYPE_GW,
	VER_TYPE_GG,

	VER_TYPE_MAX,
} eVER_TYPE;
// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード(部番チェック変更)

// MH322915(S) K.Onodera 2017/05/22 遠隔ダウンロード修正
#define		UNSENT_TIME_BEFORE_SW	5		// 切換え前の未送信ログの送信開始時間(切替の何分前か)
// MH322915(E) K.Onodera 2017/05/22 遠隔ダウンロード修正

/*
#define	R_DL_IDLE 			0	// 待機状態
#define	R_DL_REQ_RCV 		1	// 遠隔ダウンロード要求受信（開始待ち）
#define	R_SW_WAIT 			2	// プログラム更新待ち（予約）
#define	R_DL_START 			3	// 遠隔ダウンロード実行開始（予約）
#define	R_SW_START 			4	// プログラム切替開始
#define	R_UL_RES_RESV_DL	5	// ダウンロード結果情報送信予約
#define	R_UL_RES_RESV_SW	6	// プログラム更新結果情報送信予約
#define	R_UL_RES_RESV_CN	7	// FTP接続確認情報送信予約
#define	R_DL_EXEC			8	// 遠隔ダウンロード実行（Task起動）
#define	R_UL_DLRES_EX		9	// ダウンロード結果情報送信実行（Task起動）
#define	R_UL_SWRES_EX		10	// 更新結果結果情報送信実行（Task起動）
#define	R_UL_CNRES_EX		11	// FTP接続確認情報送信実行（Task起動）
*/
enum{
	R_DL_IDLE 				=0, // 待機状態
	R_DL_REQ_RCV, 				// 遠隔メンテナンス要求受信（開始待ち）
	R_UP_WAIT,					// アップロード待ち
	R_RESET_WAIT,				// リセット待ち
	R_SW_WAIT, 					// プログラム更新待ち（予約）
// -----------------------------------↑ NOT_DOWNLOADING()
// -----------------------------------↓ DOWNLOADING()
	R_DL_START, 				// 遠隔ダウンロード実行開始（予約）
	R_UP_START,					// アップロード実行開始
	R_RESET_START,				// リセット開始
	R_TEST_CONNECT,				// FTP接続テスト開始
	R_SW_START,					// プログラム切替開始
	R_DL_EXEC,					// 遠隔ダウンロード実行
};

// 遠隔要求番号
enum {
	VER_UP_REQ = 1,					// バージョンアップ要求
	PROGNO_CHG_REQ,					// 部番変更要求
	PARAM_CHG_REQ,					// 設定変更要求
	PARAM_UPLOAD_REQ,				// 設定要求(アップロード)
	RESET_REQ,						// リセット要求
	PROG_ONLY_CHG_REQ,				// プログラム切替要求
	FTP_CHG_REQ,					// FTP設定変更要求
	REMOTE_FEE_REQ,					// 遠隔料金設定要求

	FTP_TEST_REQ = 10,				// FTP接続テスト要求
// MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
	PARAM_DIFF_CHG_REQ = 13,		// 差分設定変更要求
	REMOTE_REQ_MAX
// MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
};
// MH810100(S) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)
//#define REMOTE_REQ_MAX				(REMOTE_FEE_REQ+1)
// MH810100(E) Y.Yamauchi 2019/11/27 車番チケットレス(遠隔ダウンロード)

// 処理種別
enum {
	PROC_KIND_EXEC,					// 実行
	PROC_KIND_EXEC_RT,				// 即時実行
	PROC_KIND_CANCEL,				// 取消
//	PROC_KIND_CHG_TIME,				// 時刻変更
	PROC_KIND_RESV_INFO,			// 予約内容
};

// 遠隔要求応答
enum {
	REQ_NONE,						// 変更なし
	REQ_ACCEPT,						// 受付
	REQ_NOT_ACCEPT,					// 受付不可
	REQ_PROGNO_ERR,					// 要求メインプログラム部番異常
	REQ_RCV_NONE,					// 要求未受信
	REQ_NOT_ACCEPT_RT,				// 受付不可(即時実行用)
	REQ_SW_PROG_NONE,				// プログラムデータなし
	REQ_SW_PROGNO_ERR,				// 切換部番不一致

	REQ_CONN_OK = 8,				// 接続成功
	REQ_CONN_NG,					// 接続失敗

	REQ_PROG_DL_WAIT = 11,			// プログラムダウンロード開始待ち
	REQ_PROG_DL_EXEC,				// プログラムダウンロード中
	REQ_PROG_SW_WAIT,				// プログラム更新開始待ち
	REQ_PARAM_DL_WAIT,				// 設定ダウンロード開始待ち
	REQ_PARAM_DL_EXEC,				// 設定ダウンロード中
	REQ_PARAM_SW_WAIT,				// 設定更新開始待ち
	REQ_PARAM_UL_WAIT,				// 設定アップロード待ち
	REQ_PARAM_UL_EXEC,				// 設定アップロード中
	REQ_RESET_WAIT,					// リセット開始待ち
	
	REQ_TEST_NOW = 20,				// FTP接続テスト中
	
	REQ_CANCEL_ACCEPT = 50,			// 取消受付
	REQ_CANCEL_NOT_ACCEPT,			// 取消受付不可
};

// 切替種別
enum {
	SW_PROG,						// プログラム
	SW_WAVE,						// 音声
	SW_PARAM,						// 設定
	SW_PROG_EX,						// プログラム切換要求
// GG120600(S) // Phase9 パラメータ切替を分ける
	SW_PARAM_DIFF,					// 設定（差分）
// GG120600(E) // Phase9 パラメータ切替を分ける
	SW_MAX,
};

// 要求種別
#define REQ_KIND_VER_UP				(1 << 0)		// バージョンアップ要求
#define REQ_KIND_PROGNO_CHG			(1 << 1)		// 部番変更要求
#define REQ_KIND_PARAM_CHG			(1 << 2)		// 設定変更要求
#define REQ_KIND_PARAM_UL			(1 << 3)		// 設定要求
#define REQ_KIND_RESET				(1 << 4)		// リセット要求
#define REQ_KIND_PROG_ONLY_CHG		(1 << 5)		// プログラム切替要求
#define REQ_KIND_FTP_CHG			(1 << 6)		// FTP設定変更要求
#define REQ_KIND_TEST				(1 << 7)		// 接続テスト
// MH810100(S) Y.Yamauchi 20191224 車番チケットレス(遠隔ダウンロード)
#define REQ_KIND_PARAM_DIF_CHG		(1 << 8)		// 差分設定変更要求
// MH810100(S) Y.Yamauchi 20191224 車番チケットレス(遠隔ダウンロード)
#define REQ_KIND_ALL				(0xFFFFFFFF)

// 開始時刻種別
enum {
	PROG_DL_TIME,					// プログラムダウンロード時間
	PARAM_DL_TIME,					// 設定ダウンロード時間
	PARAM_UP_TIME,					// 設定アップロード時間
	RESET_TIME,						// リセット時間
	PROG_ONLY_TIME,					// プログラム切替時間
// MH810100(S) Y.Yamauchi 20191223 車番チケットレス(遠隔ダウンロード)	
	PARAM_DL_DIF_TIME,				// 差分設定ダウンロード時間
// MH810100(E) Y.Yamauchi 20191223 車番チケットレス(遠隔ダウンロード)	
	TEST_TIME,						// 接続テスト
	TIME_MAX,
};
#define TIME_INFO_MAX				TEST_TIME

// 情報種別
enum {
	INFO_KIND_START,				// 開始時刻
	INFO_KIND_SW,					// 更新時刻
	INFO_KIND_MAX
};

// リトライ種別
enum {
// GG120600(S) // Phase9 リトライの種別を分ける
//	RETRY_KIND_CONNECT,				// 接続リトライ
//	RETRY_KIND_DL,					// ダウンロードリトライ
//	RETRY_KIND_UL,					// アップロードリトライ
	RETRY_KIND_CONNECT_PRG,				// 接続リトライ（プログラム）
	RETRY_KIND_CONNECT_PARAM_UP,		// 接続リトライ（パラメータUP）
	RETRY_KIND_CONNECT_PARAM_DL,		// 接続リトライ（パラメータDL）
	RETRY_KIND_CONNECT_PARAM_DL_DIFF,	// 接続リトライ（パラメータDL差分）
// ------------------------------
	RETRY_KIND_DL_PRG,					// ダウンロードリトライ（プログラム）
	RETRY_KIND_DL_PARAM,				// ダウンロードリトライ（パラメータ）
	RETRY_KIND_DL_PARAM_DIFF,			// ダウンロードリトライ（パラメータ差分）
	RETRY_KIND_UL,						// アップロードリトライ
	RETRY_KIND_MAX,

	RETRY_KIND_CONNECT_MAX = RETRY_KIND_DL_PRG,
// GG120600(E) // Phase9 リトライの種別を分ける
};

// 部番種別
enum {
	PROGNO_KIND_DL,					// ダウンロード部番
	PROGNO_KIND_SW,					// 切換部番
	PROGNO_KIND_MAX
};

// GG120600(S) // Phase9 設定変更通知対応
enum {
	PRM_UP_DATE_NONE = 0,			// なし
	PRM_UP_DATE_REQ,				// センターデータ要求
	PRM_UP_DATE_FTP,				// FTP中

};
// GG120600(E) // Phase9 設定変更通知対応


// 暗号化ファイルの拡張子
#define PROG_FILE_EXT		".enc\r\n"

// パラメータファイル名
#define PARAM_FILE_NAME		"param.enc"
// MH810100(S) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
// パラメータファイル名(bin形式)
#define PARAM_BIN_FILE_NAME		"param.dat"
// MH810100(E) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)

// アップロード結果
#define PARAM_UP_COMP		0
// テスト接続結果
#define TEST_CHECK_COMP		0		// 接続テスト正常終了

// ダウンロード結果
#define	PROG_DL_COMP		0		// プログラムダウンロード正常終了
#define	FOMA_COMM_ERR		1		// FOMA通信異常
#define	CONN_TIMEOUT_ERR	2		// 接続タイムアウト
#define	LOGIN_ERR			3		// ログイン認証エラー
#define	SCRIPT_FILE_ERR		4		// スクリプト内容異常（フォーマット異常）
#define	SCRIPT_FILE_NONE	5		// スクリプトファイルがない
#define	PROG_DL_ERR			6		// ダウンロードしたファイル異常（SUM値が異なる）
#define	PROG_DL_NONE		7		// ダウンロードファイルがない
#define	FLASH_WRITE_ERR		8		// フラッシュへの書込み異常
#define	PROG_DL_RESET		9		// プログラムダウンロード/更新中に精算機リセット
#define TEST_UP_ERR			10		// テストファイルアップロード失敗
#define TEST_DOWN_ERR		11		// テストファイルダウンロード失敗
#define TEST_DEL_ERR		12		// テストファイル削除失敗
#define	FTP_QUIT_ERR		13		// 切断エラー
#define PROG_SUM_ERR		14		// プログラムSUM値エラー
#define PARAM_UP_ERR		20		// アップロード失敗
#define	EXCLUDED			0xff	// 対象外（未実行）

// 更新結果
#define	PROG_SW_COMP		0		// プログラム更新正常終了
#define	BACKUP_ERR			1		// 全データバックアップ失敗
#define	PROG_SW_NONE		2		// 待機面に切り替え対象のデータがない
#define	SW_INFO_WRITE_ERR	3		// 更新情報の書込みに失敗
#define	RESTORE_ERR			4		// リストア失敗

enum {
	CTRL_NONE,
	CTRL_PROG_DL,					// プログラムダウンロード
	CTRL_PARAM_DL,					// 設定ダウンロード
	CTRL_PARAM_UPLOAD,				// 設定アップロード
	CTRL_RESET,						// リセット
	CTRL_PROG_SW,					// プログラム切換
	CTRL_CONNECT_CHK,				// 接続テスト
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
	CTRL_PARAM_DIF_DL,				// 差分設定ダウンロード
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
// GG120600(S) // Phase9 LCD用
	CTRL_PARAM_UPLOAD_LCD,			// 設定アップロード(LCD用)
// GG120600(E) // Phase9 LCD用
};

enum {
// GG120600(S) // Phase9 結果を区別する
//	RES_DL,							// ダウンロード結果
//	RES_SW,							// 更新結果
	RES_DL_PROG,					// ダウンロード結果
	RES_DL_PARAM,					// ダウンロード結果
	RES_DL_PARAM_DIF,				// ダウンロード結果
	RES_SW_PROG,					// 更新結果
	RES_SW_PARAM,					// 更新結果
	RES_SW_PARAM_DIF,				// 更新結果
// GG120600(E) // Phase9 結果を区別する
	RES_UP,							// アップロード結果
	RES_COMM,						// 接続チェック結果
// GG120600(S) // Phase9 LCD用
	RES_UP_LCD,						// アップロード結果LCD用
// GG120600(E) // Phase9 LCD用
	RES_MAX,
};

// リトライステータス
#define RETRY_OVER			0x80	// リトライオーバー
#define RETRY_EXEC			0x40	// リトライ実行中

#define MAKE_FILENAME_SW				4
#define MAKE_FILENAME_PARAM_MKD			5
#define MAKE_FILENAME_PARAM_UP			6
#define MAKE_FILENAME_TEST_UP			7
// MH810100(S) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)
#define MAKE_FILENAME_PARAM_UP_FOR_LCD	8
// MH810100(E) K.Onodera 2019/11/19 車番チケットレス（→LCD パラメータ)

// 処理ステータス
enum {
	EXEC_STS_NONE,					// 未
	EXEC_STS_COMP,					// 済
	EXEC_STS_ERR,					// Ｅ
};

// 遠隔監視データ
#define RMON_OFFSET_REAL_TIME		30000		// 即時実行

// 開始判定経過時間
#define NG_ELAPSED_TIME				30

#define	PROG_CHG_MAX		3		// プログラム更新種別MAX 0：MAIN 1：音声 2：共通パラメータ
									
#define	BACKUP_RETRY_COUNT	3
#define	RESTORE_RETRY_COUNT	3
#define	UNSENT_DATA_RETRY_COUNT	300	// 送信シーケンスキャンセルカウント（300秒）: RAU_SND_SEQ_TIME_UP

#define RESULT_SND_TIMER	90		// プログラム更新後の再起動時の結果情報送信待ちタイマー（秒）
enum{
	REMOTE_AUTO_SCRIPT = 0,	// 遠隔ダウンロード用FTPクライアントスクリプト（自動）
	REMOTE_MANUAL_SCRIPT,	// 遠隔ダウンロード用FTPクライアントスクリプト（Rismからの指定）
	MANUAL_SCRIPT,			// 手動用FTPクライアントスクリプト(メンテナンス)
	PARAMTER_SCRIPT,		// 共通パラメータアップロード用FTPクライアントスクリプト
	REMOTE_NO_SCRIPT,		// スクリプトなし(ParkingWebからの指定)
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ）
	CONNECT_TEST,			// 接続テスト
	LCD_NO_SCRIPT,			// スクリプトなし(LCDへのアップロード)
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ）
	
	// 追加分はここから↑に追加
	SCRIPT_MAX
};


#define TASK_START_FOR_DL()		(remotedl_task_startup_status_get() >= R_DL_EXEC )
#define DOWNLOADING()			(remotedl_status_get() >= R_DL_START)
#define NOT_DOWNLOADING()		(remotedl_status_get() <= R_SW_WAIT )
#define DOOR_OPEN_OR_SOLENOID_UNROCK()	(OPE_SIG_DOOR_Is_OPEN  || !ExIOSignalrd(INSIG_DOOR))

enum{
	FLASH_WRITE_BEFORE = 0,		// FLASH書込み前
	FLASH_FIRST_WRITE_START,	// 初回書込み実行
	FLASH_WRITE_END,			// 書込み完了（断片データ）
	FLASH_WRITE_START,			// 書込み開始（断片データ）
};


typedef struct{
	uchar			status;			// リトライ実行中ステータス
	date_time_rec	time;			// 次回リトライ時間
	uchar			count;			// リトライカウント

}t_retry_info;

typedef union {
	ulong			uladdr;
	uchar			ucaddr[4];
} u_ipaddr;

typedef struct {
	date_time_rec	sw_time;			// 更新時刻
	uchar			exec;				// 更新フラグ
} t_sw_info;

typedef struct {
	u_inst_no		u;					// 命令番号
// GG120600(S) // Phase9 遠隔監視データ変更
	uchar			From;			// 要求元
// GG120600(E) // Phase9 遠隔監視データ変更
	date_time_rec	start_time;			// 開始時刻
	u_ipaddr		ftpaddr;			// FTPサーバアドレス
	ushort			ftpport;			// FTPポート番号
	uchar			ftpuser[24];		// FTPユーザ
	uchar			ftppasswd[24];		// FTPパスワード
} t_dl_info;

typedef struct {
	PRG_HDR			header;
	ulong			base_addr;
	ushort			sum;
	uchar			cur_dir_ent;
	uchar			resv;
// GG129000(S) R.Endo 2022/12/28 車番チケットレス4.0 #6754 プログラムダウンロード処理リトライ後に遠隔監視データ送信せず [共通改善項目 No1540]
	ushort			uscrcTemp;
// GG129000(E) R.Endo 2022/12/28 車番チケットレス4.0 #6754 プログラムダウンロード処理リトライ後に遠隔監視データ送信せず [共通改善項目 No1540]
} t_write_info;

typedef struct {
	date_time_rec	start_time;			// 開始時刻
	date_time_rec	sw_time;			// 更新時刻
	ulong			resv;
	uchar			status[INFO_KIND_MAX];	// 処理ステータス
} t_remote_time_info;

typedef struct {
	t_remote_time_info	time_info[TIME_INFO_MAX];
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	t_remote_time_info	time_info2[3];	// ダミー
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
} t_remote_dl_info;

// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
#define RETRY_KIND_MAX_V0			3
#define RES_MAX_V0					4
#define SW_MAX_V0					4
#define TIME_MAX_V0					6
#define TIME_INFO_MAX_V0			5

typedef struct {
	t_remote_time_info	time_info[TIME_INFO_MAX_V0];
} t_remote_dl_info_V0;

typedef struct {
	u_inst_no		u;					// 命令番号
	date_time_rec	start_time;			// 開始時刻
	u_ipaddr		ftpaddr;			// FTPサーバアドレス
	ushort			ftpport;			// FTPポート番号
	uchar			ftpuser[24];		// FTPユーザ
	uchar			ftppasswd[24];		// FTPパスワード
} t_dl_info_V0;

typedef struct{
	int				req_accept;		// 受付した要求種別
	uchar			connect_type;	// 処理中の種別
	uchar			status;			// 接続ステータス
	uchar			script_type;
	uchar			update;
	uchar			result[RES_MAX_V0];// 結果
	ushort			retry_dl_cnt;	// ダウンロード開始リトライ回数
	ushort			failsafe_timer;	// FTPフェールセーフタイマー
	ushort			monitor_info;	// 端末監視情報
	uchar			param_up;		// 共通パラメータアップロード用の種別
	uchar			exec_info;		// 遠隔ダウンロードの実行状況
	t_retry_info	retry[RETRY_KIND_MAX_V0];	// リトライ情報
	uchar			sw_status[SW_MAX_V0-1];	// sw_updateのバックアップエリア
	uchar			pow_flg;		// 復電フラグ
	ulong			dummy1[10];
// -----------------------------------↑ 実行中情報
// -----------------------------------↓ 受信情報
	t_dl_info_V0	dl_info[TIME_MAX_V0];
	t_sw_info		sw_info[SW_MAX_V0];
	uchar			script[PROGNO_KIND_MAX][12];
	t_write_info	write_info;
	ulong			dummy2[10];
}t_prog_chg_info_V0;
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//typedef struct{
////	uchar			status;			// 接続ステータス
////									
////	uchar			connect_type;	// 接続種別０：プログラムダウンロード
////									//		   １：プログラム更新
////									//		   ２：結果情報送信制御
////									//		   ３：接続確認チェック
////									//		   ４：共通パラメータアップロード
////									
////	uchar			sw_kind;		// 切替種別０：メインプログラム
////									//		   １：サブCPU
////									//		   ２：共通パラメータ
////	uchar			sw_req;			// 切替要求受信
////	uchar			result[RES_MAX];// [0] 遠隔ダウンロード結果
////									// [1] プログラム切り替え結果
////									// [2] 結果情報アップロード結果/FTP接続確認
////	uchar			update;
////	uchar			bk_res;
////	t_dl_info		dl_info[3];
////	date_time_rec	dl_time;
////	uchar			script_type;
////	uchar			script[36];
////	t_retry_info	retry[2];
////	uchar			timeout;
////	uchar			sw_status[3];	// sw_updateのバックアップエリア
////	uchar			param_up;		// 共通パラメータアップロード用の種別
////	uchar			exec_info;		// 遠隔ダウンロードの実行状況
////	uchar			Reserve[9];
//int				req_accept;		// 受付した要求種別
//uchar			connect_type;	// 処理中の種別
//uchar			status;			// 接続ステータス
//uchar			script_type;
//uchar			update;
//uchar			result[RES_MAX];// 結果
//ushort			retry_dl_cnt;	// ダウンロード開始リトライ回数
//ushort			failsafe_timer;	// FTPフェールセーフタイマー
//ushort			monitor_info;	// 端末監視情報
//uchar			param_up;		// 共通パラメータアップロード用の種別
//uchar			exec_info;		// 遠隔ダウンロードの実行状況
//t_retry_info	retry[RETRY_KIND_MAX];	// リトライ情報
//uchar			sw_status[SW_MAX-1];	// sw_updateのバックアップエリア
//uchar			pow_flg;		// 復電フラグ
//ulong			dummy1[10];
//// -----------------------------------↑ 実行中情報
//// -----------------------------------↓ 受信情報
//t_dl_info		dl_info[TIME_MAX];
//t_sw_info		sw_info[SW_MAX];
//uchar			script[PROGNO_KIND_MAX][12];
//t_write_info	write_info;
//ulong			dummy2[10];
//}t_prog_chg_info;
typedef struct{

	int				req_accept;		// 受付した要求種別
	uchar			connect_type;	// 処理中の種別
	uchar			status;			// 接続ステータス
	uchar			script_type;
	uchar			update;
	uchar			result[RES_MAX];// 結果
	uchar			result_dummy[4];// dummy
	ushort			retry_dl_cnt[TIME_INFO_MAX];	// ダウンロード開始リトライ回数
	ushort			retry_dl_cnt_dummy[4];	// ダウンロード開始リトライ回数
	ushort			failsafe_timer;	// FTPフェールセーフタイマー
	ushort			monitor_info;	// 端末監視情報
	uchar			param_up;		// 共通パラメータアップロード用の種別
	uchar			exec_info;		// 遠隔ダウンロードの実行状況
	t_retry_info	retry[RETRY_KIND_MAX];	// リトライ情報
	t_retry_info	retry_dummy[4];	// リトライ情報
	uchar			sw_status[SW_MAX-1];	// sw_updateのバックアップエリア
	uchar			sw_status_dummy[4];	// sw_updateのバックアップエリア
	uchar			pow_flg;		// 復電フラグ
	ulong			dummy1[10];
// -----------------------------------↑ 実行中情報
// -----------------------------------↓ 受信情報
	t_dl_info		dl_info[TIME_MAX];
	t_dl_info		dl_info_dummy[4];
	t_sw_info		sw_info[SW_MAX];
	t_sw_info		sw_info_dummy[4];
	uchar			script[PROGNO_KIND_MAX][12];
	uchar			script_dummy[4][12];
	t_write_info	write_info;
// GG129000(S) R.Endo 2022/12/28 車番チケットレス4.0 #6754 プログラムダウンロード処理リトライ後に遠隔監視データ送信せず [共通改善項目 No1540]
// 	t_write_info	write_info_dummy;
	uchar			write_info_dummy[24];
// GG129000(E) R.Endo 2022/12/28 車番チケットレス4.0 #6754 プログラムダウンロード処理リトライ後に遠隔監視データ送信せず [共通改善項目 No1540]
	union{
		uchar crc[2];
		ushort uscrc;
	}CRC16;
	uchar			dummy4[1];		// 
	uchar			dummy3[1];		// 
	ulong			dummy2[9];
}t_prog_chg_info;
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

extern	uchar	remotedl_status_get( void );
extern	void	remotedl_status_set( uchar );
extern	uchar	remotedl_task_startup_status_get( void );
extern	uchar	remotedl_connect_type_get( void );
extern	void	remotedl_connect_type_set( uchar );
extern	void	remotedl_info_clr( void );
extern	t_prog_chg_info*	remotedl_info_get( void );
extern	uchar	remotedl_restore( void );
extern	uchar	remotedl_BackUP( void );
extern	ushort	GetMessageRemote( void );
extern	void	AutoDL_UpdateTimeCheck( void );
extern	void	remote_dl_check( uchar	*rcvdata );
extern	void	ftp_remote_auto_update( void );
extern	void	ftp_remote_auto_switch( void );
extern	void	ftp_remote_auto_update_sub(void);
extern	void	moitor_regist( ushort , uchar );
extern	ushort	remotedl_disp( void );
extern	void	remotedl_result_set( uchar );
extern	uchar	remotedl_result_get( uchar );
extern	void	remotedl_result_clr( void );
extern	void	MakeRemoteFileName( uchar*, uchar*, char );
extern	uchar	remote_date_chk( date_time_rec *pTime );
extern	void	remotedl_script_typeset( uchar );
extern	uchar	remotedl_script_typeget( void );
extern	const 	char*	remotedl_script_get( void );
extern	uchar	flap_move_chk( void );
extern	uchar	remotedl_update_chk( void );
extern	void	remotedl_update_set( void );
extern	uchar	remotedl_first_comm_get( void );
extern	uchar	retry_info_set( uchar );
extern	uchar	dl_start_chk( date_time_rec*, uchar );
extern	void	remote_evt_recv( ushort, uchar* );
extern	void	Before_DL_Err_Function( uchar );
extern	uchar	now_retry_active_chk( uchar );
extern	void	remote_result_snd_timer( void );
extern	void	retry_info_clr( uchar );
extern	void	retry_info_connect_timer(void);
// GG120600(S) // Phase9 リトライの種別を分ける
extern	void retry_info_connect_timer2(void);
extern	void retry_info_connect_timer3(void);
extern	void retry_info_connect_timer4(void);
// GG120600(E) // Phase9 リトライの種別を分ける
extern	void	retry_count_clr(uchar kind);
extern	void	retry_time_clr(uchar kind);
extern	void	connect_timeout( void );
extern	void	remotedl_timeout_sts_set( uchar );
extern	uchar	remotedl_timeout_sts_get( void );
extern	void	remotedl_connect_req( ushort, uchar );
extern	void	remotedl_sw_update_bk( uchar* );
extern	void	remotedl_sw_update_get( uchar* );
extern	void	remotedl_chg_info_bkup( void );
extern	void	remotedl_chg_info_restore( void );
extern	ushort	Param_Upload_ShortCut( void );
extern	void	Param_Upload_type_set( uchar );
extern	uchar	Param_Upload_type_get( void );
extern	void	remotedl_exec_info_set( uchar );
extern	uchar	remotedl_exec_info_get( void );
extern	uchar	GetSnd_RecvStatus( void );
extern	void	SetSnd_RecvStatus( uchar );
extern	void	get_script_file_name( char*, const char* );
extern	uchar	GetFTPMode( void );
extern uchar	up_wait_chk( date_time_rec *time, uchar status );
extern uchar	reset_start_chk( date_time_rec *time, uchar status );
extern ushort AppServ_MakeRemoteDl_TestConnect(char* pBuff,int nSize);
extern	uchar remotedl_proc_resv_exec(t_ProgDlReq *p, BOOL bRT);
// GG120600(S) // Phase9 遠隔監視データ変更
//extern	uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt);
extern	uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt,uchar from);
// GG120600(E) // Phase9 遠隔監視データ変更
extern	uchar remotedl_proc_chg_time(t_ProgDlReq *p);
extern	uchar remotedl_proc_resv_info(t_ProgDlReq *p);
extern	ulong remotedl_ftp_ipaddr_get(void);
extern	ushort remotedl_ftp_port_get(void);
extern	void remotedl_ftp_user_get(char *user, int size);
extern	void remotedl_ftp_passwd_get(char *passwd, int size);
extern	int remotedl_dl_start_retry_check(void);
extern	int remotedl_sw_start_retry_check(void);
extern	int remotedl_reset_start_retry_check(void);
// GG120600(S) // Phase9 リトライの種別を分ける
//extern	void remotedl_start_retry_clear(void);
extern	void remotedl_start_retry_clear(uchar dl_kind);
// GG120600(E) // Phase9 リトライの種別を分ける
extern	void remotedl_failsafe_timer_clear(void);
extern	int remotedl_failsafe_timer_check(void);
extern	int remotedl_convert_dl_status(void);
extern	void remotedl_accept_request(int request);
extern	void remotedl_clear_request(int request);
extern	uchar remotedl_is_accept(int request);
extern	void remotedl_complete_request(int request);
extern	void remotedl_recv_info_clear(uchar type);
extern	u_inst_no* remotedl_instNo_get(void);
extern	void rmon_regist(ulong Code);
// GG120600(S) // Phase9 遠隔監視データ変更
//extern	void rmon_regist_ex(ulong Code, ulong *pInstNo);
extern	void rmon_regist_ex(ulong Code, ulong *pInstNo,uchar from);
// GG120600(E) // Phase9 遠隔監視データ変更

extern	t_remote_time_info* remotedl_time_info_get(uchar type);
// GG120600(S) // Phase9 上書きで中止時に命令番号が空になる
//extern	void remotedl_time_info_clear(uchar type);
//extern	void remotedl_time_info_clear_all(void);
extern	void remotedl_time_info_clear(uchar type,u_inst_no *pInstNo,uchar from);
extern	uchar remotedl_instFrom_get(void);
// GG120600(E) // Phase9 上書きで中止時に命令番号が空になる
// GG120600(S) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
extern	u_inst_no* remotedl_instNo_get_with_Time_Type(uchar timeType);
extern	uchar remotedl_instFrom_get_with_Time_Type(uchar timeType);
// GG120600(E) // Phase9 #5073 【連動評価指摘事項】遠隔メンテナンス画面から時刻情報のクリアをすると遠隔監視データの命令番号1・2に値が入らない(No.02‐0003)
extern	void remotedl_time_set(uchar kind, uchar type, date_time_rec *pTime);
extern	void remotedl_comp_set(uchar kind, uchar type, uchar status);
extern	void remotedl_ftp_info_set(t_FtpInfo *pFtpInfo);
extern	t_FtpInfo* remotedl_ftp_info_get(void);
extern	ulong remotedl_unsent_data_count_get(void);
extern	void remotedl_prog_no_get(char *pProgNo, int size);
extern	void remotedl_write_info_set(t_write_info *pInfo);
extern	void remotedl_write_info_get(t_write_info *pInfo);
extern	void remotedl_write_info_clear(void);
extern	int remotedl_check_request(uchar ReqKind);
extern	ushort	IsMainProgExist(void);
extern	void remotedl_monitor_info_set(ushort info);
extern	ushort remotedl_monitor_info_get(void);
extern	void remotedl_pow_flg_set(BOOL bFlg);
extern	const	uchar	ReqAcceptTbl[REMOTE_REQ_MAX];
//extern	t_ProgDlReq g_bk_ProgDlReq;
// GG120600(S) // Phase9 設定変更通知対応
extern	void	rmon_regist_ex_FromTerminal(ulong Code);
// GG120600(S) // Phase9 リトライの種別を分ける
extern	void	rmon_regist_with_ConectType(ulong Code,uchar connect_type);
// GG120600(E) // Phase9 リトライの種別を分ける
extern	void	remotedl_arrange_next_request(void);
extern	void	remotedl_cancel_setting(void);
extern	void	remotedl_prog_crc_set( uchar uc1,uchar uc2);
extern	ushort	remotedl_prog_crc_get( void );			
// GG120600(E) // Phase9 設定変更通知対応
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
extern	void	remotedl_chg_info_log0_to1(void);
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする

#endif
