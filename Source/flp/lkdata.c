/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF盤ｼﾘｱﾙ通信関連変数										  										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba																					   |*/
/*| Date        :  2005-05-06																				   |*/
/*| Update      :																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
																				/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"LKcom.h"															/*								*/
/*--------------*/																/*								*/
/*	LKcom.c		*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcom_InitFlg;													/* ﾛｯｸ装置通信初期化ﾌﾗｸﾞ		*/
																				/*								*/
t_LKcomLineCtrl	LKcomLineCtrl;													/* 回線ｺﾝﾄﾛｰﾙ					*/
																				/*								*/
t_LK_RCV_INFO	LKcom_RcvData;													/*								*/
																				/*								*/
																				/*								*/
uchar			LKcom_f_SndReq;													/* POL/SEL送信ﾘｸｴｽﾄ				*/
uchar			LKcom_f_TaskStop;												/* ﾛｯｸ装置通信ﾀｽｸ停止ﾌﾗｸﾞ		*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	LKcomdr.c	*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcomdr_RcvData[1024];											/*								*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	LKcomApi.c	*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcom_RcvDtWork[1024];											/* 受信ﾃﾞｰﾀ解析用ﾜｰｸ			*/
																				/*								*/
uchar			MntLockTest[LK_LOCK_MAX];										/* ﾃｽﾄ結果						*/
uchar			MntFlapTest[CAR_LOCK_MAX];										/* ﾃｽﾄ結果格納ｴﾘｱ(未使用)		*/

ulong			MntLockDoCount[BIKE_LOCK_MAX][3];								/* LOCK装置動作ｶｳﾝﾄ				*/
ulong			MntFlapDoCount[TOTAL_CAR_LOCK_MAX][3];							/* ﾌﾗｯﾌﾟ装置動作ｶｳﾝﾄ			*/
ushort			MntLockWkTbl[LOCK_IF_REN_MAX];									/* ﾛｯｸ装置動作ｶｳﾝﾄ受信用変換ﾃｰﾌﾞﾙ	*/
																				/*								*/
t_LKBUF			LockBuf[LK_LOCK_MAX];											/* 状態ﾃﾞｰﾀ受信一時退避ﾊﾞｯﾌｧ	*/
																				/*								*/
uchar			child_mk[LOCK_IF_MAX];											// 子機別ﾛｯｸ装置ﾒｰｶｰ編集
																				/*								*/
																				/*								*/
uchar			flp_m_mode[FLAP_IF_MAX];										/* ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞﾌﾗｸﾞ		*/
																				/*  ON = 手動ﾓｰﾄﾞ中				*/
uchar			lok_m_mode[LOCK_IF_MAX];										/* ﾛｯｸ装置手動ﾓｰﾄﾞﾌﾗｸﾞ			*/
																				/*  ON = 手動ﾓｰﾄﾞ中				*/
uchar			LKcom_Type[LOCK_IF_MAX];										// 端末毎の装置種別（駐車・駐輪）
																				/*------------------------------*/
