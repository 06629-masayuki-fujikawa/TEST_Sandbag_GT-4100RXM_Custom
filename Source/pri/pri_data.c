/*[]---------------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀｰ制御部：制御ﾃﾞｰﾀｴﾘｱ												|*/
/*|																				|*/
/*|	ﾌｧｲﾙ名称	:	Pri_Data.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"Pri_def.h"



/*------------------------------------------*/
/*	非初期化ｴﾘｱ								*/
/*------------------------------------------*/


/*------------------------------------------*/
/*	初期化ｴﾘｱ								*/
/*------------------------------------------*/

PRN_DATA_BUFF	rct_prn_buff;			// ﾚｼｰﾄ(SCI0)	送受信ﾃﾞｰﾀ管理ﾊﾞｯﾌｧ
PRN_DATA_BUFF	jnl_prn_buff;			// ｼﾞｬｰﾅﾙ(SCI3)	送受信ﾃﾞｰﾀ管理ﾊﾞｯﾌｧ
// MH810104 GG119201(S) 電子ジャーナル対応
uchar			eja_work_buff[PRN_EJA_BUF_SIZE];	// 電子ジャーナル用ワークバッファ
// MH810104 GG119201(E) 電子ジャーナル対応

PRN_PROC_DATA	rct_proc_data;			// ﾚｼｰﾄ		印字処理制御ﾃﾞｰﾀ
PRN_PROC_DATA	jnl_proc_data;			// ｼﾞｬｰﾅﾙ	印字処理制御ﾃﾞｰﾀ

uchar			rct_init_sts;			// ﾚｼｰﾄ		初期化状態(０：初期化完了待ち／１：初期化完了／２：初期化失敗／３：未接続)
uchar			jnl_init_sts;			// ｼﾞｬｰﾅﾙ	初期化状態(０：初期化完了待ち／１：初期化完了／２：初期化失敗／３：未接続)
uchar			rct_init_rty;			// ﾚｼｰﾄ		初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲｶｳﾝﾀｰ
uchar			jnl_init_rty;			// ｼﾞｬｰﾅﾙ	初期化ｺﾏﾝﾄﾞ送信ﾘﾄﾗｲｶｳﾝﾀｰ

uchar			GyouCnt_r;				// ﾚｼｰﾄ		送信済み印字ﾃﾞｰﾀ行数
uchar			GyouCnt_j;				// ｼﾞｬｰﾅﾙ	送信済み印字ﾃﾞｰﾀ行数

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
uchar			GyouCnt_All_r;			// ﾚｼｰﾄ		送信済み印字ﾃﾞｰﾀ全行数
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
uchar			GyouCnt_All_j;			// ｼﾞｬｰﾅﾙ	送信済み印字ﾃﾞｰﾀ全行数
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

MSG 			NextMsg_r;				// ﾚｼｰﾄ		印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧ
MSG 			NextMsg_j;				// ｼﾞｬｰﾅﾙ	印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧ

uchar			PriDummyMsg;			// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録状態（ON：登録あり／OFF：登録なし）
MsgBuf			*TgtMsg;				// 処理対象ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ

uchar			terget_pri;				// 処理対象ﾌﾟﾘﾝﾀ種別
uchar			next_prn_msg;			// 印字処理中の次編集要求ﾒｯｾｰｼﾞﾌﾗｸﾞ	（ON：次編集要求／OFF：新規編集要求）
uchar			syuukei_kind;			// 印字処理中の集計種別

uchar			rct_goukei_pri;			// 合計記録印字状態（ﾚｼｰﾄ用）	※ON：印字処理中
uchar			jnl_goukei_pri;			// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）	※ON：印字処理中
uchar			rct_atsyuk_pri;			// 自動集計印字状態（ﾚｼｰﾄ用）	※ON：印字処理中
uchar			jnl_atsyuk_pri;			// 自動集計印字状態（ｼﾞｬｰﾅﾙ用）	※ON：印字処理中
uchar			turikan_pri_status;		// 釣銭管理ﾌﾟﾘﾝﾄが印字終了をｾｯﾄする

uchar			prn_zero;				// 集計ﾃﾞｰﾀ０時印字有無（設定ﾃﾞｰﾀ）
date_time_rec	Repri_Time;				// 再ﾌﾟﾘﾝﾄ日時（前回集計印字用）
uchar			Repri_kakari_no;		// 再ﾌﾟﾘﾝﾄ係員No.

uchar			header_hight;			// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀの縦ｻｲｽﾞ（ﾄﾞｯﾄ数）
uchar			gyoukan_data;			// 改行幅（ﾄﾞｯﾄ数）指定（＝０:設定ﾃﾞｰﾀに従う／≠０:本指定ﾃﾞｰﾀに従う）

char			prn_work[64];			// 印字処理ﾜｰｸ
char			cTmp1[256];				// 印字処理ﾜｰｸ
char			cTmp2[64];				// 印字処理ﾜｰｸ
char			cMnydata[64];			// 印字処理ﾜｰｸ
char			cEditwk1[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ１
char			cEditwk2[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ２
char			cEditwk3[64];			// 印字ﾃﾞｰﾀ編集ﾜｰｸ３

char			cErrlog[160];			// ｴﾗｰﾛｸﾞ用（ｴﾗｰ情報登録ﾊﾞｯﾌｧ）

ulong			Header_Rsts;			// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ読込状態
ulong			Footer_Rsts;			// ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			Header_Data[4][36];		// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀ
char			Footer_Data[4][36];		// ﾌｯﾀｰ印字ﾃﾞｰﾀ
ulong			AcceptFooter_Rsts;			// 受付券ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			AcceptFooter_Data[4][36];	// 受付券ﾌｯﾀｰ印字ﾃﾞｰﾀ

char			Logo_Fdata[24][56];		// ﾛｺﾞ印字ﾃﾞｰﾀ読込ﾜｰｸ
char			Logo_Fla_data[24];		// ﾛｺﾞ印字ﾃﾞｰﾀ変換ﾜｰｸ1（ﾌﾗｯｼｭﾒﾓﾘのﾃﾞｰﾀ）
char			Logo_Reg_data[24];		// ﾛｺﾞ印字ﾃﾞｰﾀ変換ﾜｰｸ2（ﾌﾗｯｼｭﾒﾓﾘを変換したﾃﾞｰﾀ）
uchar			Logo_Reg_sts_rct;		// ﾛｺﾞ登録処理状態（ﾚｼｰﾄ）
uchar			Logo_Reg_sts_jnl;		// ﾛｺﾞ登録処理状態（ｼﾞｬｰﾅﾙ）

t_Pri_Work		Pri_Work[2];			// プリンタタスクワークエリア
										// 精算LOG・集計LOG印字に使用時
										//   [0]=レシートプリンタ用、[1]=ジャーナルプリンタ用

ulong			Syomei_Rsts;			// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ読込状態
ulong			Kamei_Rsts;				// 加盟店名印字ﾃﾞｰﾀ読込状態
char			Syomei_Data[4][36];		// 支払ｺｰﾄﾞ・署名欄印字ﾃﾞｰﾀ
char			Kamei_Data[4][36];		// 加盟店名印字ﾃﾞｰﾀ

uchar			JR_Print_Wait_tim[2]={0xff, 0xff};
										// 以下ﾕﾆｯﾄの動き始めからの経過時間(単位:20ms)
										// [0] = ﾚｼｰﾄﾌﾟﾘﾝﾀ
										// [1] = ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ

uchar			f_Prn_R_SendTopChar;	// 1=ﾚｼｰﾄﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前（未送信）
										// 0=先頭1ｷｬﾗｸﾀ送信要求後
uchar			f_Prn_J_SendTopChar;	// 1=ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前（未送信）
										// 0=先頭1ｷｬﾗｸﾀ送信要求後
uchar 			Cal_Parameter_Flg;		// 設定値異常フラグ 0:正常/1:異常

ulong			TCardFtr_Rsts;			// Ｔカードﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			TCardFtr_Data[T_FOOTER_GYO_MAX][36];	// Ｔカードﾌｯﾀｰ印字ﾃﾞｰﾀ
ulong			AzuFtr_Rsts;			// Ｔカードﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			AzuFtr_Data[AZU_FTR_GYO_MAX][36];	// Ｔカードﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
ulong			CreKbrFtr_Rsts;			// 過払いﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			CreKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// 過払いﾌｯﾀｰ印字ﾃﾞｰﾀ
ulong			EpayKbrFtr_Rsts;			// 過払いﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			EpayKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// 過払いﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
ulong			FutureFtr_Rsts;				// 後日支払額ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			FutureFtr_Data[FUTURE_FTR_GYO_MAX][36];	// 後日支払額ﾌｯﾀｰ印字ﾃﾞｰﾀ
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
ulong			EmgFooter_Rsts;			// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀ読込状態
char			EmgFooter_Data[EMGFOOT_GYO_MAX][36];	// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀ
// MH810105(E) MH364301 QRコード決済対応
ushort			Pri_Tasck[TSKMAX][2];		// タスク使用量、％
ulong			Pri_Pay_Syu[2][2];			// 精算、集計データサイズ
uchar			Pri_program;				// デバッグNo
uchar			f_partial_cut;				// パーシャルカット動作フラグ(パーシャルカットが実行されたことを示すフラグ 0:未実行 1:実行)

uchar			rct_timer_end;			// レシートプリンタ印字完了後タイマー
uchar			PriBothPrint;			// プリンタ＋ジャーナル印字フラグ
uchar			BothPrnStatus;			// プリンタ＋ジャーナル印字のエラーステータス
ushort			BothPrintCommand;		// プリンタ＋ジャーナル印字のエラーコマンド
// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
uchar			ryo_inji;				// レシートプリンタ領収証印字状態 0=印字中ではない/1=印字中
// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
EC_SETTLEMENT_RES	Deemedalmrct;
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
PRN_RCV_DATA_BUFF_R	pri_rcv_buff_r;
uchar			ryo_stock;				// レシートデータストックフラグ 0:ストックなし 1:ストックあり 2:ストック破棄 0xff:ストック印字完了
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
uchar			RegistNum[14];			// インボイスの登録番号（T+13桁の数字）
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）

