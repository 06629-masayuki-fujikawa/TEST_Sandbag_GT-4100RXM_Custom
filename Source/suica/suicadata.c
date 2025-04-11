/*[]----------------------------------------------------------------------[]*/
/*| suicaﾃﾞｰﾀ定義                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| Date        : 2006-07-07                                               |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// MH321800(S) G.So ICクレジット対応
#include	<string.h>
// MH321800(E) G.So ICクレジット対応
#include	"suica_def.h"
#include	"system.h"


SUICA_SETTLEMENT_RES Settlement_Res;

SUICA_REC	Suica_Rec;
SUICA_SEND_BUFF	Suica_Snd_Buf;													

unsigned long	pay_dsp;							// 表示依頼データ
unsigned char	err_data;							// 異常データ
t_STATUS_DATA_INFO	STATUS_DATA;					// 状態データ
struct	clk_rec	time_data;							// 時間同期データ
unsigned char	fix_data[2];						// 固定データ
unsigned char	suica_errst;						/* Communication  Status */
unsigned char	suica_errst_bak;					/* Communication  Status */
unsigned char 	suica_err[5];
unsigned char 	Retrycount;							/* Retryｶｳﾝﾄ数 */
unsigned char	dsp_work_buf[2];					// 画面表示用一時退避領域
unsigned char	Status_Retry_Count;					// 受付不可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
unsigned char	Status_Retry_Count_NG;					// 受付不可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
unsigned char	Status_Retry_Count_OK;					// 受付可時の制御ﾃﾞｰﾀ送信ｶｳﾝﾀ
unsigned char	Ope_Suica_Status;					// Ope側からの送信ｽﾃｰﾀｽ
unsigned char	disp_media_flg;						// 画面表示用　精算媒体使用可(0)否(1)
unsigned char	DspWorkerea[7];						// 駐車券挿入待ち画面　利用媒体表示ﾒｯｾｰｼﾞNo.
unsigned short	DspChangeTime[2];					// 画面表示用のﾀｲﾏｰ値ｾｯﾄ領域
// MH321800(S) D.Inaba ICクレジット対応
//unsigned char	dsp_change;							// 画面切替用ﾌﾗｸﾞ
unsigned char	dsp_change;							// 未了発生中の画面表示ﾌﾗｸﾞ 0=未了関係表示なし 1=未了関係表示中 2=未了関係表示中(制御データ未了残高通知終了送信後)
// MH321800(E) D.Inaba ICクレジット対応
unsigned char	dsp_fusoku;							// 残高不足ｱﾗｰﾑ表示制御ﾌﾗｸﾞ
long			w_settlement;						// Suica残高不即時の精算額（０時は残高不足でないを意味する）
unsigned char	suica_fusiku_flg;
unsigned char	mode_Lagtim10;						// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示 1:Suica停止後の受付許可再送信Wait終了
unsigned short	nyukin_delay[DELAY_MAX];			// 未了発生中のﾒｯｾｰｼﾞを保持 [0]:ｺｲﾝﾒｯｸ	[1]:紙幣
unsigned char	delay_count;						// 未了発生中のﾒｯｾｰｼﾞｶｳﾝﾄ数
unsigned short	wk_media_Type;						// 精算媒体（電子マネー）種別
unsigned char	time_out;							// 画面戻りﾀｲﾑｱｳﾄ発生判定領域
// MH321800(S) G.So ICクレジット対応
//long			Product_Select_Data;				// 送信した商品選択データ(決済時の比較用)
// MH321800(E) G.So ICクレジット対応
unsigned char suica_work_buf[S_BUF_MAXSIZE];
struct	clk_rec	suica_work_time;
t_STATUS_DATA_INFO	STATUS_DATA_WAIT;					// 状態データ(受信待ち用)

/* Suica ｺﾏﾝﾄﾞ */
const unsigned char	suica_tbl[][2] = {
	{ 0xE0, 0x1F },									// スタンバイ要求
	{ 0xE1, 0x1E },									// 一括要求
	{ 0xE2, 0x1D },									// 入力要求
	{ 0xE3, 0x1C },									// 出力指令
	{ 0xE4, 0x1B },									// 入力要求の再送、一括要求の再送
// MH321800(S) G.So ICクレジット対応
//	{ 0xE5, 0x1A }};
	{ 0xE5, 0x1A },									// 出力指令の再送
	{ 0xE2, 0x1D },									// 入力要求後の入力要求再送
};
// MH321800(E) G.So ICクレジット対応

// MH321800(S) G.So ICクレジット対応
void	*const	jvma_init_tbl[] = {
//	0				1				2				3				4
	NULL,			NULL,			NULL,			NULL,			ec_init,
//	5				6				7				8				9
	NULL,			NULL,			NULL,			NULL,			NULL
};

#pragma	section	_UNINIT1							/* "B":Uninitialized data area in external RAM1 */
long			Product_Select_Data;				// 送信した選択商品データ(決済時の比較用)
char			EcEdyTerminalNo[20];				// Edy上位端末ID
t_JVMA_SETUP	jvma_setup;							// 復電時などに以前の設定を参照させるため、起動時初期化させない
// MH321800(E) G.So ICクレジット対応
