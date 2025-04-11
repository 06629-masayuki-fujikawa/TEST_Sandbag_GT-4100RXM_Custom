// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *| NT-NET通信処理部
 *[]----------------------------------------------------------------------[]
 *| Author      : S.Takahashi
 *| Date        : 2013.02.22
 *| Update      :
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/*----------------------------------*/
/*		function external define	*/
/*----------------------------------*/

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	NTCom_Init( void );
void	NTCom_ConfigInit( void );



/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_Init()                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          NT task initial routine                                       |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NTCom_Init( void )
{

	NTCom_ConfigInit();

	/* ntcomcomm.c */
	memset(&send_blk, 0, sizeof(T_NT_BLKDATA));					// 送信バッファ		初期化時に引数で渡す
	memset(&receive_blk, 0, sizeof(T_NT_BLKDATA));				// 受信バッファ

	/* ntcomcomdr.c */
	memset(&NTComComdr_Ctrl, 0, sizeof(T_NT_COMDR_CTRL));			/* COMDR制御データ */

	/* ntcomdata.c */
	/* エラー情報バッファ */
	memset(&NTCom_Err, 0, sizeof(T_NT_ERR_INFO));

	/* ntcommain.c */
	NTCom_Condit = 0;												/* task condition */
	NTCom_SciPort = 0;											/* use SCI channel number (0 or 2) */

	/** initial setting command received flag **/
	memset(&NTCom_ComErr, 0, sizeof(t_NT_ComErr));

	/* ntcomsci.c */
	/* SCI制御データ */
	memset(&NTComSci_Ctrl, 0, sizeof(T_NT_SCI_CTRL));

	/* ntcomtimer.c */
	/* 1ms精度グループ */
	memset(&NTComTimer_1ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_1MS_CH_MAX);
	/* 10ms精度グループ */
	memset(&NTComTimer_10ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_10MS_CH_MAX);
	/* 100ms精度グループ */
	memset(&NTComTimer_100ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_100MS_CH_MAX);
	/* タイマONOFF制御 */
	NTComTimer_Enable = FALSE;	/* TRUE=有効 */

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_ConfigInit
 *[]----------------------------------------------------------------------[]
 *| summary	: 共通パラメータ関連する設定を初期化する
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTCom_ConfigInit( void )
{

	/* 主局／従局 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.ExecMode = prm_get(COM_PRM, S_NTN, 2, 1, 1);
	NTCom_InitData.ExecMode = prm_get(COM_PRM, S_SSS, 2, 1, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 子機接続台数(主局時)／自端末No.(従局時) */
// MH364300 GG119A23(S) // GG122600(S) Y.Tanizaki NT-NETターミナルNo拡張(8->16)
//	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_NTN, 3, 1, 1);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_NTN, 3, 2, 1);
	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_SSS, 3, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
// MH364300 GG119A23(E) // GG122600(E) Y.Tanizaki NT-NETターミナルNo拡張(8->16)

	/* 通信BPS */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Baudrate = prm_get(COM_PRM, S_NTN, 4, 1, 1);
	NTCom_InitData.Baudrate = prm_get(COM_PRM, S_SSS, 4, 1, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* ENQ送信後の応答待ち時間(t1) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t1 = prm_get(COM_PRM, S_NTN, 5, 4, 1);
	NTCom_InitData.Time_t1 = prm_get(COM_PRM, S_SSS, 5, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 受信データ最大時間(t2) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t2 = prm_get(COM_PRM, S_NTN, 6, 4, 1);
	NTCom_InitData.Time_t2 = prm_get(COM_PRM, S_SSS, 6, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* テキスト送信後の応答待ち時間(t3) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t3 = prm_get(COM_PRM, S_NTN, 7, 4, 1);
	NTCom_InitData.Time_t3 = prm_get(COM_PRM, S_SSS, 7, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 次局とのインターバル時間(t4) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t4 = prm_get(COM_PRM, S_NTN, 8, 4, 1);
	NTCom_InitData.Time_t4 = prm_get(COM_PRM, S_SSS, 8, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 文字間タイマー(t5) */
	// NOTE:他のシリアルデバイス等が同時に動作し(I2CやSPIなど)、過剰に負荷がかかった時にタスクレベルで
	// 処理が追いつかなくなる可能性があるため、文字間タイマーをわざと伸ばし(マージン2倍)時間かせぎをする
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t5 = prm_get(COM_PRM, S_NTN, 9, 4, 1) * 2;
	NTCom_InitData.Time_t5 = prm_get(COM_PRM, S_SSS, 9, 4, 1) * 2;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 同報ENQ後のデータ送信Wait時間(t6) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t6 = prm_get(COM_PRM, S_NTN, 10, 4, 1);
	NTCom_InitData.Time_t6 = prm_get(COM_PRM, S_SSS, 10, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 受信データ最大時間(t7) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t7 = prm_get(COM_PRM, S_NTN, 11, 4, 1);
	NTCom_InitData.Time_t7 = prm_get(COM_PRM, S_SSS, 11, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 同報処理前のWAIT時間(t8) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t8 = prm_get(COM_PRM, S_NTN, 12, 4, 1);
	NTCom_InitData.Time_t8 = prm_get(COM_PRM, S_SSS, 12, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 送信ウェイトタイマー(t9) */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_t9 = prm_get(COM_PRM, S_NTN, 13, 4, 1);
	NTCom_InitData.Time_t9 = prm_get(COM_PRM, S_SSS, 13, 4, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* データリトライ受信回数 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Retry = prm_get(COM_PRM, S_NTN, 14, 2, 1);
	NTCom_InitData.Retry = prm_get(COM_PRM, S_SSS, 14, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* ブロック送信単位 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.BlkSndUnit = prm_get(COM_PRM, S_NTN, 15, 2, 1);
	NTCom_InitData.BlkSndUnit = prm_get(COM_PRM, S_SSS, 15, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 通信回線監視タイマー */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_LineMonitoring = prm_get(COM_PRM, S_NTN, 16, 2, 1);
	NTCom_InitData.Time_LineMonitoring = prm_get(COM_PRM, S_SSS, 16, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 無応答エラー判定回数 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.NoResNum = prm_get(COM_PRM, S_NTN, 17, 2, 1);
	NTCom_InitData.NoResNum = prm_get(COM_PRM, S_SSS, 17, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* 無応答時スキップ周期 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.SkipCycle = prm_get(COM_PRM, S_NTN, 18, 2, 1);
	NTCom_InitData.SkipCycle = prm_get(COM_PRM, S_SSS, 18, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

	/* MAINモジュールからの受信応答待ち時間 */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）
//	NTCom_InitData.Time_ResFromMain = prm_get(COM_PRM, S_NTN, 19, 2, 1);
	NTCom_InitData.Time_ResFromMain = prm_get(COM_PRM, S_SSS, 19, 2, 1);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（設定アドレス変更）

}

/*[]----------------------------------------------------------------------[]*/
/*|             ntcomtask()                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|          NT task main routine                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	ntcomtask( void )
{
	NTCom_Init();

	NTCom_FuncStart();			/* function start */

	for( ;; ){
		taskchg( IDLETSKNO );

		NTCom_FuncMain();			/* RAU process exec */
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
