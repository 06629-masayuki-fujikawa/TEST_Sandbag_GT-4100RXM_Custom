/*[]----------------------------------------------------------------------[]*/
/*|		対子機通信タスク 外部参照(領域)宣言								   |*/
/*|			・ここには、新I/F盤（親機）内の「対子機通信」タスクが		   |*/
/*|			  使用する領域の外部参照宣言します。						   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _TO_SDATA_H_
#define _TO_SDATA_H_

#include	"IF.h"
#include	"IFM.h"
#include	"toS.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/


/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define		TOSCOM_NO_ACK_TERMINAL	(uchar)0xff		// ACK待ち端末なし


/*==================================*/
/*		InRAM area define			*/
/*==================================*/


/*----------------------------------*/
/*			toStimr.c				*/
/*----------------------------------*/
extern	ushort	toScom_Tx2ms_Char;					/* 文字間監視用（ 1msec Timer）*/
extern	ushort	toScom_Tx10ms_Link;					/* 受信監視用  （10msec Timer）*/
extern	ushort	toScom_Tx10ms_Line;					/* 回線監視用  （10msec Timer）*/

/*----------------------------------*/
/*			toSmain.c				*/
/*----------------------------------*/
extern	uchar				toS_f_TmStart;		/** timer function start flag **/
												/* 1=Start, 0=don't do process */

/*----------------------------------*/
/*			toScom.c				*/
/*----------------------------------*/
extern	t_toScom_Timer		toScom_Timer;		/* timer value */
extern	t_toScom_Matrix		toScom_Matrix;		/* */
extern	t_toScom_Polling	toScom_toSlave;		/* 子機ポーリング情報 */
extern	uchar				toScom_RcvPkt[TOS_SCI_RCVBUF_SIZE];	/* 受信(電文)バッファ（タスクレベル）*/
extern	ushort				toScom_PktLength;	/* 受信(電文)長 */
extern	uchar				toScom_ucAckSlave;	// ACK送信元端末No.
extern	uchar				toScom_first;		// 初期フラグ
extern	ulong				toScom_StartTime;	// 初期設定送信タイマー

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
extern	uchar			toScomdr_RcvData[TOS_SCI_RCVBUF_SIZE];	/* 受信バッファ（ドライバレベル）*/
extern	ushort			toScomdr_RcvLength;						/* 受信データ長 */
extern	uchar			toScomdr_f_RcvCmp;						/* 受信イベント */
extern	t_SciRcvError	toS_RcvErr;								/* 受信エラー情報 */
extern	t_SciComError	toS_ComErr;								/* 通信エラー情報 */

#endif	// _TO_SDATA_H_
