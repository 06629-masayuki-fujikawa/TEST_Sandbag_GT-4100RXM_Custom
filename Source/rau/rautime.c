/***********************************************************************/
/*																	   */
/*	FILE		:rautime.c											   */
/*																	   */
/***********************************************************************/

#include	"rau.h"
#include	"raudef.h"


/********************* FUNCTION DEFINE **************************/
extern	void	(*x_T1shotFUNADR)(void);	/* One-shot interrupt will call program when time out */
extern	void	(*x_T2shotFUNADR)(void);	/* One-shot interrupt will call program when time out */
static	uchar	RAUTm_100ms;				// 100msタイマカウンタ
static	uchar	RAUTm_1000ms;				// 1sタイマカウンタ
static	uchar	RAUTm_60s;					// 1分タイマカウント


/*[]----------------------------------------------------------------------[]*/
/*|   [API]		RAU_c_1mTim_Start()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Timer start value get. (return 1ms counter)						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Input  : none														   |*/
/*| Return : 1ms inclemental counter value 								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  M.TAKAHASHI                                             |*/
/*| Date        :  2003-09-04                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
ulong	RAU_c_1mTim_Start( void )
{
	return RAU_x_1mLifeTime;
}

/*[]----------------------------------------------------------------------[]*/
/*|   [API]		RAU_c_1mTim_IsTimeout()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Timeout check routine.                      					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Input  : StartTime = when started time value (got by c_Tim_Start())	   |*/
/*| 		 WaitTime  = waiting timer value. 1-F000000h (x1ms unit)	   |*/
/*| Return : 1=timeout,  0=not yet			 							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  M.TAKAHASHI                                             |*/
/*| Date        :  2003-09-04                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
uchar	RAU_c_1mTim_IsTimeout( ulong StartTime, ulong WaitTime )
{
	ulong	NowTime;
	ulong	PastTime;

	NowTime = RAU_c_1mTim_Start();			/* Now 10ms counter value get */
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}

	if( PastTime >= WaitTime ){
		return (uchar)1;
	}
	return (uchar)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|   [API]		RAU_c_1mPastTimGet()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		経過時間(x1ms単位)を得る                      					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Input  : StartTime = when started time value (got by c_Tim_Start())	   |*/
/*| Return : 経過時間　= x1ms値				 							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  okuda	                                               |*/
/*| Date        :  2009-05-07                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ulong	RAU_c_1mPastTimGet( ulong StartTime )
{
	ulong	NowTime;
	ulong	PastTime;

	NowTime = RAU_c_1mTim_Start();			/* Now 10ms counter value get */
	if( NowTime >= StartTime ){
		PastTime = NowTime - StartTime;
	}
	else{
		PastTime = NowTime + ((0xffffffffL - StartTime) + 1);
	}

	return	PastTime;
}

/****************************************************************************/
/*	関数名		=  void RAU_TimerInit()										*/
/*	機能		=  全タイマ停止（立ち上がり時にCallされる）					*/
/*	概略　　　　=　															*/
/****************************************************************************/
void	RAU_TimerInit( void )
{
	RAU_Tm_Reset_t 		= 0;

	RAU_Tm_RcvSeqCnt = 0;				// 受信シーケンスキャンセルタイマー

	RAU_Tm_CommuTest.tm = 0;			// 通信チェックタイマタイマ(停止)
}

/****************************************************************************/
/*	関数名		=  void RAU_20msInt()										*/
/*	機能		=  ソフトタイマ・カウント(20msec割り込み)					*/
/*	概略　　　　=　															*/
/****************************************************************************/
void	RAU_20msInt( void )
{
	if( 0 != RAU_f_TmStart ){

		++RAUTm_1000ms;
		if( RAUTm_1000ms >= 50 ){			// 1s timer
			RAUTm_1000ms = 0;

			if( RAU_f_CommuTest_rq == 1 ){	// 通信チェック用タイマ(起動中)
				RAU_Tm_CommuTest.tm++;
			}

			if( RAU_f_Ackwait2_rq == 1 ){	// 下り回線ＡＣＫ待ちタイマ(起動中)
				RAU_Tm_Ackwait2.tm++;
			}

			if( RAU_f_Ackwait_rq == 1 ){	// 上り回線ＡＣＫ待ちタイマ(起動中)
				RAU_Tm_Ackwait.tm++;
			}
			if( RAU_f_Ackwait_reconnect_rq == 1 ){	// 上り回線再接続待ちタイマ(起動中)
				RAU_Tm_Ackwait_reconnect.tm++;
			}
			if( Credit_Ackwait_rq == 1 ){	// クレジット回線ＡＣＫ待ちタイマ(起動中)
				RAU_Tm_Ackwait_CRE.tm++;
			}
			if( Credit_Ackwait_reconnect_rq == 1 ){	// クレジット回線ＡＣＫ待ち中の再接続タイマ(起動中)
				RAU_Tm_Ackwait_reconnect_CRE.tm++;
			}
			if(Credit_SeqReset_rq == 1){
				Credit_SeqReset_tm++;
			}
			if(RAU_f_SndSeqCnt_rq == 1){
				RAU_Tm_SndSeqCnt++;
			}

			if( RAU_f_RcvSeqCnt_rq == 1 ){
				RAU_Tm_RcvSeqCnt ++;
			}

			// 無通信タイマー(秒)
			if( RAU_f_No_Action_rq == 1 ) {
				RAU_Tm_No_Action.tm++;
			}

			// 下り回線通信監視タイマー(秒)
			if( RAU_f_Port_Watchdog_rq == 1 ) {
				RAU_Tm_Port_Watchdog.tm++;
			}

			// 下りデータ再送待ちタイマ
			++RAUTm_60s;
			if(RAUTm_60s >= 60) {			// 1分経過
				RAUTm_60s = 0;
				if( RAU_f_data_rty_rq == 1 ) {
					++RAU_Tm_data_rty.tm;
				}
			}
		}
		
		++RAUTm_100ms;
		if(RAUTm_100ms >= 5) {				// 100ms経過？
			RAUTm_100ms = 0;
			// コネクション切断待ちタイマー
			if( RAU_f_discnnect_rq == 1 ) {
				RAU_Tm_DISCONNECT.tm++;
			}
			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ
			if( RAU_f_TCPtime_wait_rq == 1 ) {
				RAU_Tm_TCP_TIME_WAIT.tm++;
			}
			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ(クレジット)
			if(Credit_TCPtime_wait_rq == 1){
				CRE_tm_TCP_TIME_WAIT.tm++;
			}
		}
	}
}
