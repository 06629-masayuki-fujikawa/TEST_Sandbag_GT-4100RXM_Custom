// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

uchar				NTCom_Condit = 0;						/* task condition */

/* ntcomtimer.c */
/* 1ms精度グループ */
T_NT_TIM_CTRL		NTComTimer_1ms[NT_TIM_1MS_CH_MAX];
/* 10ms精度グループ */
T_NT_TIM_CTRL		NTComTimer_10ms[NT_TIM_10MS_CH_MAX];
/* 100ms精度グループ */
T_NT_TIM_CTRL		NTComTimer_100ms[NT_TIM_100MS_CH_MAX];
/* タイマONOFF制御 */
BOOL				NTComTimer_Enable = FALSE;				/* TRUE=有効 */

/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT4		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:アドレス空間0x06039000-0x060FFFFF(1MB)
T_NTCom_SndWork		NTCom_SndWork;
T_NT_SND_TELEGRAM	NTCom_Tele_Broadcast;
T_NT_SND_TELEGRAM	NTCom_Tele_Prior;
T_NT_SND_TELEGRAM	NTCom_Tele_Normal;


/* ntcomcomm.c */
T_NT_BLKDATA		send_blk;								// 送信バッファ		初期化時に引数で渡す
T_NT_BLKDATA		receive_blk;							// 受信バッファ


/* ntcomcomdr.c */
T_NT_COMDR_CTRL		NTComComdr_Ctrl;						/* COMDR制御データ */


/* ntcomdata.c */
/* データ管理機能 制御データ */
T_NTComData_Ctrl		NTComData_Ctrl;
/* エラー情報バッファ */
T_NT_ERR_INFO		NTCom_Err;

/* ntcommain.c */
uchar				NTCom_SciPort;							/* use SCI channel number (0 or 2) */
/** received command from IBW **/
//t_NTCom_RcvQueCtrl	NTCom_RcvQueCtrl;						/* queue control data */
															/* If you want to get this data, you call "NTCom_ArcRcvQue_Read()" */
/** initial setting command received flag **/
T_NT_INITIAL_DATA	NTCom_InitData;							/* initial setting data */
t_NT_ComErr			NTCom_ComErr;


/* ntcomsci.c */
/* SCI制御データ */
T_NT_SCI_CTRL		NTComSci_Ctrl;


/* ntcomslave.c */
T_NT_BLKDATA		*NTComSlave_send_blk;					// 送信バッファ		初期化時に引数で渡す
T_NT_BLKDATA		*NTComSlave_receive_blk;				// 受信バッファ
T_NTCOM_SLAVE_CTRL	NTComComm_Ctrl_Slave;					// 従局管理用構造体

// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
