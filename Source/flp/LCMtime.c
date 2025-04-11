/*[]----------------------------------------------------------------------[]*/
/*|		New I/F Master Lock contorol task's timer routines				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"LCM.h"
#include	"LCMdata.h"

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	LCM_TimerInit(void);
void	LCM_10msInt(void);
void	LCM_1secInt(void);
void	LCMtim_10msTimerStart(int iTerm, int iLock, ushort TimVal);
void	LCMtim_10msTimerStop(int iTerm, int iLock);
uchar	LCMtim_10msTimeout(int iTerm, int iLock);
void	LCMtim_1secTimerStart(int iTerm, int iLock, ushort TimVal);
void	LCMtim_1secTimerStop(int iTerm, int iLock);
uchar	LCMtim_1secTimeout(int iTerm, int iLock);


void	LCMtim_1secWaitStart(ushort TimVal);
void	LCMtim_1secWaitStop(void);
uchar	LCMtim_1secWaitTimeout(void);


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimerInit()  	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|				timer initial routine                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimerInit( void )
{

	int		ni,nj;

	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		for (nj=0; nj < IFS_ROOM_MAX_USED; nj++) {
			LCMtim_10msTimerStop(ni,nj);
			LCMtim_1secTimerStop(ni,nj);
		}
	}
	
	LCMtim_1secWaitStop();
}


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_10msInt()  	 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		10ms interval interrupt routine                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_10msInt( void )
{
	int		ni,nj;

	/* active (Y)・・・タスク開始されていることが前提 */
	if (0 != LCM_f_TmStart) {

		/* ロック装置の動作待ち時間 */
		for (ni=0; ni < IF_SLAVE_MAX; ni++) {
			for (nj=0; nj < IFS_ROOM_MAX_USED; nj++) {
				if (0 != (LCMtim_Tx10ms_Action[ni][nj] & 0x7fff)) {
					--LCMtim_Tx10ms_Action[ni][nj];
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_1secInt()  		 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		1sec interval interrupt routine                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_1secInt( void )
{
	int		ni,nj;

	/* active (Y)・・・タスク開始されていることが前提 */
	if (0 != LCM_f_TmStart) {

		/* [explain how to use counter value]	*/
		/*  1) 0x8000 is timer stop 			*/
		/*  2) 0x0000 is time up 				*/
		/*  3) 0x0001 - 0x7FFF is counting 		*/

		/* ロック装置の動作リトライ間隔待ち時間 */
		
		for (ni=0; ni < IF_SLAVE_MAX; ni++) {
			for (nj=0; nj < IFS_ROOM_MAX_USED; nj++) {
				if (0 != (LCMtim_Tx1sec_Retry[ni][nj] & 0x7fff)) {
					--LCMtim_Tx1sec_Retry[ni][nj];
				}
			}
		}

		if (0 != (LCMtim_Tx1sec_Wait & 0x7fff)) {
			--LCMtim_Tx1sec_Wait;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCMtim_xxTimerStart()	  	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		? msec タイマー開始												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCMtim_10msTimerStart(int iTerm, int iLock, ushort TimVal)	{	LCMtim_Tx10ms_Action[iTerm][iLock]  = TimVal; }
void	LCMtim_1secTimerStart(int iTerm, int iLock, ushort TimVal)	{	LCMtim_Tx1sec_Retry[iTerm][iLock]   = TimVal; }
void	LCMtim_1secWaitStart(ushort TimVal)				{	LCMtim_Tx1sec_Wait           = TimVal; }

/*[]----------------------------------------------------------------------[]*/
/*|             LCMtim_xxTimerStop() 		                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		? msec タイマー停止												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCMtim_10msTimerStop(int iTerm, int iLock)	{	LCMtim_Tx10ms_Action[iTerm][iLock]  = 0x8000; }
void	LCMtim_1secTimerStop(int iTerm, int iLock)	{	LCMtim_Tx1sec_Retry[iTerm][iLock]   = 0x8000; }
void	LCMtim_1secWaitStop(void)					{	LCMtim_Tx1sec_Wait           = 0x8000; }

/*[]----------------------------------------------------------------------[]*/
/*|             LCMtim_xxTimeout() 	    	                               |*/
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
uchar	LCMtim_10msTimeout(int iTerm, int iLock)
{
	if (0 == LCMtim_Tx10ms_Action[iTerm][iLock]) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
uchar	LCMtim_1secTimeout(int iTerm, int iLock)
{
	if (0 == LCMtim_Tx1sec_Retry[iTerm][iLock]) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
uchar	LCMtim_1secWaitTimeout(void)
{
	if (0 == LCMtim_Tx1sec_Wait) {
		return	(uchar)1;
	}
	return	(uchar)0;
}
