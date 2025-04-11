/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		FB7000ｼﾘｱﾙ通信関連変数										  										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :  																							   |*/
/*| Date        :  																							   |*/
/*| Update      :																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
																				/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"fbcom.h"															/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	fbsci.c		*/																/*								*/
/*--------------*/																/*								*/
t_FBSciRcvCtrl	FB_RcvCtrl;														/*								*/
t_FBSciSndCtrl	FB_SndCtrl;														/*								*/
																				/*								*/
uchar			FB_RcvBuf[ FB_SCI_RCVBUF_SIZE ];								/*								*/
uchar			FB_SndBuf[ FB_SCI_SNDBUF_SIZE ];								/*								*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	fbcom.c		*/																/*								*/
/*--------------*/																/*								*/
uchar	FBcom_RcvData[FB_RCV_BUF_SIZE];											/*								*/
																				/*								*/
ushort			FBcom_Timer_2_Value;											/* 文字受信ｳｪｲﾄ					*/
ushort			FBcom_Timer_4_Value;											/* 次局ｲﾝﾀｰﾊﾞﾙ					*/
ushort			FBcom_Timer_5_Value;											/* 文字間						*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	fbcomdr.c	*/																/*								*/
/*--------------*/																/*								*/
uchar			FBcomdr_f_RcvCmp;												/*								*/
uchar			FBcomdr_SciErrorState;											/* シリアルエラー状態			*/
ushort			FBcomdr_RcvLength;												/* temporary buffer clear		*/
uchar			FBcomdr_RcvData[FB_SCI_RCVBUF_SIZE];								/*								*/
