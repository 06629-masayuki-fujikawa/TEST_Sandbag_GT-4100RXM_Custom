/*************************************************************************/ /**
 *	\file	SodiacPer_memory.h
 *	\brief	Sodiac �y���t�F���� ������
 *	\author	Copyright(C) Arex Co. 2008-2011 All right Reserved.
 *	\author	������ЃA���b�N�X 2008-2011
 *	\date	CREATE : 17/Nov/2008
 *	\date	2011.03.02 Iwasawa H. sodiac_read_end �֐��̒ǉ�
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "SodiacAPI.h"

#ifndef	_defSodiacPer_memory_H_
#define	_defSodiacPer_memory_H_

/* �����f�[�^�ǂݍ��݊֐��v���g�^�C�v */
/*************************************************************************/ /**
 *	�������Ǎ��݊֐�
 *
 *	�w�肳�ꂽ�A�h���X�̃f�[�^��1byte�ǂݍ���
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern unsigned char	sodiac_dataread(int iChannel,unsigned long ulAddress);
/*************************************************************************/ /**
 *	�������Ǎ��݊֐�
 *
 *	�w�肳�ꂽ�Ԓn����w�肳�ꂽ�T�C�Y�����f�[�^��ǂݍ���
 *	\param[out]	vpPt      : �f�[�^�i�[�o�b�t�@
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\param[in]	iSize     : �f�[�^�ǂݏo���T�C�Y
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern D_SODIAC_E		sodiac_nread(int iChannel,void *vpPt, unsigned long ulAddress, int iSize);

/* �w�b�_�ǂݍ��݊֐��v���g�^�C�v */
/*************************************************************************/ /**
 *	�������Ǎ��݊֐�
 *
 *	�w�肳�ꂽ�A�h���X�̃f�[�^��1byte�ǂݍ���
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern unsigned char	sodiac_headerread(unsigned long ulAddress);

/*************************************************************************/ /**
 *	�������Ǎ��݊֐�
 *
 *	�w�肳�ꂽ�Ԓn����w�肳�ꂽ�T�C�Y�����f�[�^��ǂݍ���
 *	\param[out]	vpPt      : �f�[�^�i�[�o�b�t�@
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\param[in]	iSize     : �f�[�^�ǂݏo���T�C�Y
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern D_SODIAC_E		sodiac_hnread(void *vpPt, unsigned long ulAddress, int iSize);

#endif	/* _defSodiacPer_memory_H_ */

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2011		  *
 **/ /*************************************************************************/
