/*[]----------------------------------------------------------------------[]*/
/*|		新I/F盤（親機）の対子機通信タスク内で使用する「RAM」領域		   |*/
/*|			・CPU内蔵RAMが８Kbyteしかないので注意すること。				   |*/
/*|			・外付けSRAMは512Kbyteある。								   |*/
/*|			・CPU内蔵RAMは、当然０クリア領域		 「section "B_InRAM"」 |*/
/*|			・外付けSRAMは、(前回データ)保持する領域 「section "B_ExtHold」|*/
/*|							未初期化領域			 「section "B"」	   |*/
/*|							(初期値のないデータ→０クリアされる領域のこと) |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"


#pragma section					/* "B" : 未初期化データ領域（=0クリアされる）外付けS-RAM */
/*==================================*/
/*		SRAM area define			*/
/*==================================*/
/*----------------------------------*/
/*			toScom.c				*/
/*----------------------------------*/
t_toScom_Matrix		toScom_Matrix;						/* プロトコル制御マトリクス */
t_toScom_Polling	toScom_toSlave;						/* 子機ポーリング情報 */
t_toScom_Timer		toScom_Timer;						/* 通信タイマー群 */
uchar				toScom_RcvPkt[TOS_SCI_RCVBUF_SIZE];	/* 受信(電文)バッファ（タスクレベル）*/
ushort				toScom_PktLength;					/* 受信(電文)長 */
uchar				toScom_ucAckSlave;					// ACK送信元端末No.
uchar				toScom_first;						// 初期フラグ
ulong				toScom_StartTime;					// 初期設定送信タイマー

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
uchar			toScomdr_RcvData[TOS_SCI_RCVBUF_SIZE];	/* 受信データバッファ（ドライバレベル）*/
ushort			toScomdr_RcvLength;						/* 受信データ長（バイト数）*/
uchar			toScomdr_f_RcvCmp;						/* 受信イベント */
														/*		0：何も受信していない */
														/*		1：何か受信している */
														/*		2：受信エラー発生 */
														/*		3：IFM端末間リンクレベルでタイムアウト発生した。*/
														/*		4：IFMコマンド送信後、応答待ち期限を経過した。*/
														/*		5：ポーリング時刻を経過した。*/
