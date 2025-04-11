/*[]----------------------------------------------------------------------[]*/
/*|		New I/F toSlave task's timer routines							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"


/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/
ushort	toScom_Tx100msec;		/* now nobody use */
ushort	toScom_Tx1sec;			/* now nobody use */


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toS_TimerInit(void);
void	toS_2msInt(void);
void	toS_10msInt(void);

void	toScom_2msTimerStart(ushort TimVal);
void	toScom_LinkTimerStart(ushort TimVal);
void	toScom_LineTimerStart(ushort TimVal);
void	toScom_2msTimerStop(void);
void	toScom_LinkTimerStop(void);
void	toScom_LineTimerStop(void);
uchar	toScom_2msTimeout(void);
uchar	toScom_LinkTimeout(void);
uchar	toScom_LineTimeout(void);
uchar	toScom_Is_LineTimerStop(void);


/*[]----------------------------------------------------------------------[]*/
/*|             toS_TimerInit()  	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|				timer initial routine                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_TimerInit(void)
{
	toScom_Tx2ms_Char  = 0x8000;
	toScom_Tx10ms_Link = 0x8000;
	toScom_Tx10ms_Line = 0x8000;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toS_2msInt()  	 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		1m sec interval interrupt routine                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_2msInt(void)
{
	if (0 != toS_f_TmStart) {			/* active (Y) */

		/* communication with server process timer (T1-6) */
		if (0 != (toScom_Tx2ms_Char & 0x7fff)) {
			--toScom_Tx2ms_Char;

			/*
			*	2005.06.09 早瀬・追加
			*	親子間の通信において、子が電文先頭の「(他の子からの)ヘッダ文字」を取り逃がすケースが多々ある。
			*	に、お試し対処。
			*	・割り込みハンドラレベルで文字間タイムアウト検出したら、直ちにバッファリングする。
			*	・それまではタスクレベルで、タイムアウト検出とバッファリングをしていた事が原因かも？
			*	  を検証する目的。
			*/
			if (0 != toScom_2msTimeout()) {
				toScom_2msTimerStop();				/* タイムアウト検出したので、タイマー停止 */

				/* 割り込みレベルの受信バッファから、受信ドライバレベルのバッファへ転送 */
				toScomdr_RcvLength = toS_RcvCtrl.RcvCnt;
				memcpy(toScomdr_RcvData, toS_RcvBuf, (size_t)toScomdr_RcvLength);

				toScomdr_RcvInit();					/* 受信データ長｢など｣をリセット */
				toS_RcvCtrl.RcvCmpFlg = 1;			/* ここで受信完了が確定		2005.06.13 早瀬・追加 */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toS_10msInt()  	 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		10ms interval interrupt routine                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_10msInt(void)
{
	if (0 != toS_f_TmStart) {			/* active (Y) */

		/* [explain how to use counter value]	*/
		/*  1) 0x8000          is timer stop 	*/
		/*  2) 0x0000          is time up 		*/
		/*  3) 0x0001 - 0x7FFF is counting 		*/
		if (0 != (toScom_Tx10ms_Link & 0x7fff)) {
			--toScom_Tx10ms_Link;
		}

		if (0 != (toScom_Tx10ms_Line & 0x7fff)) {
			--toScom_Tx10ms_Line;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_xxTimerStart()	  	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		? msec タイマー開始												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_2msTimerStart(ushort TimVal)		{	toScom_Tx2ms_Char  = TimVal; }
void	toScom_LinkTimerStart(ushort TimVal)	{	toScom_Tx10ms_Link = TimVal; }
void	toScom_LineTimerStart(ushort TimVal)	{	toScom_Tx10ms_Line = TimVal; }

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_xxTimerStop() 		                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		? msec タイマー停止												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_2msTimerStop(void)				{	toScom_Tx2ms_Char  = 0x8000; }
void	toScom_LinkTimerStop(void)				{	toScom_Tx10ms_Link = 0x8000; }
void	toScom_LineTimerStop(void)				{	toScom_Tx10ms_Line = 0x8000; }

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_xxTimeout() 	    	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		? msec タイマーのタイムアウト判定								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	return : 1=timeout													   |*/
/*|			 0=didn't timeout (または、タイマー開始していない)			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_2msTimeout(void)
{
	if (0 == toScom_Tx2ms_Char) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
uchar	toScom_LinkTimeout(void)
{
	if (0 == toScom_Tx10ms_Link) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
uchar	toScom_LineTimeout(void)
{
	if (0 == toScom_Tx10ms_Line) {
		return	(uchar)1;
	}
	return	(uchar)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Is_LineTimerStop() 	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		回線監視タイマーのタイムアウトor停止判定						   |*/
/*|		・本回線監視タイマーは、ポーリング時間経過タイマーとしても		   |*/
/*|		  機能します。													   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	return : 1=タイムアウト、または、タイマー開始していない				   |*/
/*|			 0=タイマーカウント中										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-27                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_Is_LineTimerStop(void)
{
	if (toScom_Tx10ms_Line == 0 ||
		toScom_Tx10ms_Line == 0x8000) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
