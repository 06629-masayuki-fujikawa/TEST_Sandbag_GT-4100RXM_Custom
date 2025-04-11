/*************************************************************************/ /**
 *	\file	SodiacPer_timer.h
 *	\brief	Sodiac ペリフェラル タイマー関連
 *	\author	Copyright(C) Arex Co. 2008-2011 All right Reserved.
 *	\author	株式会社アレックス 2008-2011
 *	\date	CREATE : 17/Nov/2008
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#ifndef	_defSodiacPer_timer_H_
#define	_defSodiacPer_timer_H_

/*************************************************************************/ /**
 *	PWMタイマー開始関数
 *
 *	\date	CREATE : 17/Nov/2008
 *	\author	Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern void	sodiac_start_timer(void);

/*************************************************************************/ /**
 *	PWMタイマー停止関数
 *
 *	\date	CREATE : 17/Nov/2008
 *	\author	Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern void	sodiac_stop_timer(void);

/*************************************************************************/ /**
 *	PWMタイマー設定関数
 *
 *	\date		CREATE : 17/Nov/2008
 *	\author		Y.Tamaki(Arex)
 *	\param[in]	usTop   : PWM周期カウンタに設定する値
 *	\param[in]	usData  : 初期PWMデューティーに設定する値
 *	\param[in]	ucCount : オーバーサンプリング回数
 **/ /*************************************************************************/
extern void	sodiac_set_timer(unsigned short usTop, unsigned short usData, unsigned char ucCount);

/*************************************************************************/ /**
 *	PWMタイマー再設定関数
 *
 *	\date		CREATE : 2010/09/14
 *	\author		Iwasawa H.(Arex)
 *	\param[in]	usTop   : PWM周期カウンタに設定する値
 *	\param[in]	usData  : 初期PWMデューティーに設定する値
 *	\param[in]	ucCount : オーバーサンプリング回数
 **/ /*************************************************************************/
extern void	sodiac_recycle_timer(unsigned short usTop, unsigned short usData, unsigned char ucCount);

#endif	/* _defSodiacPer_timer_H_ */

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2011		  *
 **/ /*************************************************************************/
