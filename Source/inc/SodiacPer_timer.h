/*************************************************************************/ /**
 *	\file	SodiacPer_timer.h
 *	\brief	Sodiac �y���t�F���� �^�C�}�[�֘A
 *	\author	Copyright(C) Arex Co. 2008-2011 All right Reserved.
 *	\author	������ЃA���b�N�X 2008-2011
 *	\date	CREATE : 17/Nov/2008
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#ifndef	_defSodiacPer_timer_H_
#define	_defSodiacPer_timer_H_

/*************************************************************************/ /**
 *	PWM�^�C�}�[�J�n�֐�
 *
 *	\date	CREATE : 17/Nov/2008
 *	\author	Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern void	sodiac_start_timer(void);

/*************************************************************************/ /**
 *	PWM�^�C�}�[��~�֐�
 *
 *	\date	CREATE : 17/Nov/2008
 *	\author	Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern void	sodiac_stop_timer(void);

/*************************************************************************/ /**
 *	PWM�^�C�}�[�ݒ�֐�
 *
 *	\date		CREATE : 17/Nov/2008
 *	\author		Y.Tamaki(Arex)
 *	\param[in]	usTop   : PWM�����J�E���^�ɐݒ肷��l
 *	\param[in]	usData  : ����PWM�f���[�e�B�[�ɐݒ肷��l
 *	\param[in]	ucCount : �I�[�o�[�T���v�����O��
 **/ /*************************************************************************/
extern void	sodiac_set_timer(unsigned short usTop, unsigned short usData, unsigned char ucCount);

/*************************************************************************/ /**
 *	PWM�^�C�}�[�Đݒ�֐�
 *
 *	\date		CREATE : 2010/09/14
 *	\author		Iwasawa H.(Arex)
 *	\param[in]	usTop   : PWM�����J�E���^�ɐݒ肷��l
 *	\param[in]	usData  : ����PWM�f���[�e�B�[�ɐݒ肷��l
 *	\param[in]	ucCount : �I�[�o�[�T���v�����O��
 **/ /*************************************************************************/
extern void	sodiac_recycle_timer(unsigned short usTop, unsigned short usData, unsigned char ucCount);

#endif	/* _defSodiacPer_timer_H_ */

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2011		  *
 **/ /*************************************************************************/
