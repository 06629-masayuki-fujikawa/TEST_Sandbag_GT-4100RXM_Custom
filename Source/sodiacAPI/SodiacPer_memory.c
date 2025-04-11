/*************************************************************************/ /**
 *	\file	SodiacPer_memory.c
 *	\brief	Sodiac �y���t�F���� ������ for RX630
 *	\author	Copyright(C) Arex Co. 2008-2012 All right Reserved.
 *	\author	������ЃA���b�N�X 2008-2012
 *	\date	CREATE : 17/Nov/2008
 *	\date	UPDATE : 2010/01/21
 *	\date	UPDATE : 2010/06/15
 *	\date	UPDATE : 2012/02/14
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "iodefine.h"
#include "vect.h"
#include "SodiacPer_memory.h"
#include "system.h"

// DMA,SPI�̊����ݐ�L���Ԃ�Z�k���đ��̊����ݏ������ғ��������[16]�Ƃ���
//	�d�v ------->	128 -> 16�ɕύX�����Address�����ذ�ނ̔����p�x�������Ȃ�̂�
//					Arex�l�̑Ή�(sodiac_Q_A��No13)�������������_��128 -> 16�̑Ή����s��
#define	BUFFSIZE	(16)		/* �����f�[�^�L���b�V���T�C�Y(byte) */


/*************************************************************************/ /**
 *	�V���A���ʐM���W�X�^
 *	���ۂ̉�H�ɂ��킹�ĕύX���Ă�������
 **/ /*************************************************************************/
#define	SPI_OUT		RSPI0.SPDR	/* ���M�o�b�t�@ */
#define	SPI_IN		RSPI0.SPDR	/* ��M�o�b�t�@ */

union Long {
	unsigned char	ucaByte[4];
	unsigned long	ulLong;
};

unsigned long	g_lAddress=0xffffffffUL;

/* �f�[�^�L���b�V���X�e�[�g */
typedef enum {
	CS_IDLE = 0,				/**< �]������ */
	CS_HEAD,					/**< �w�b�_�A�N�Z�X */
	CS_READ0,					/**< �`�����l��0���[�h */
	CS_READ1,					/**< �`�����l��1���[�h */
}	CASHESTATE;

static CASHESTATE		s_State;				/**< �f�[�^�L���b�V���X�e�[�g */
static unsigned char	s_cReadState;			/**< �t���b�V�����[�h�R�}���h�X�e�[�g */
static unsigned char	*s_pucData;				/**< �f�[�^�i�[�|�C���^ */
static unsigned short	s_usCount;				/**< �ǂݏo���f�[�^�J�E���^ */
static unsigned long	*s_ulFirstAddress[2]={	/**< �o�b�t�@���̐擪�A�h���X */
	&SodiacSoundAddress,&SodiacSoundAddress		/* 	�����f�[�^��ɂb�g���ɕۑ��̈�𕪂��Ă��Ȃ����� */
												/*	�����A�h���X�����Ă���						 */
};

static unsigned long	s_ulStartAddress[2]={	/**< �o�b�t�@���̐擪�A�h���X */
	0xffffffffUL,0xffffffffUL
};

static unsigned char	s_ucaaDataBuff[2][BUFFSIZE];	/**< �L���b�V���o�b�t�@ */
static union Long		s_Long;
unsigned char	Sodiac_FROM_Get;


/*************************************************************************/ /**
 *	SPI���荞�ݏ���
 *	\date	Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
void	Sodiac_From_Rcv( void )
{
	unsigned char	a_ucData=SPI_IN; /* ��M�f�[�^�ǂݎ�� */

	switch(s_cReadState){			/* �R�}���h���M�X�e�[�g */
	case	0:					/* READ�R�}���h���o�I�� */
		SPI_OUT=s_Long.ucaByte[1];	/* �A�h���X b23�`b16 */
		break;
	case	1:
		SPI_OUT=s_Long.ucaByte[2];	/* �A�h���X b15�`b8 */
		break;
	case	2:
		SPI_OUT=s_Long.ucaByte[3];	/* �A�h���X b7�`b0 */
		break;
	case	3:					/* ������f�[�^�ǂݏo�� */
		if(s_usCount == 1){
			RSPI0.SPCMD0.WORD = 0x2703;// SSL�M���͓]�������ŉ���
		}
		SPI_OUT=0xFF;			/* �_�~�[ */
		break;
	default:
		++g_lAddress;
		*s_pucData++ =a_ucData;	/* �f�[�^�i�[ */
		if(--s_usCount){
			if(s_usCount == 1){
				RSPI0.SPCMD0.WORD = 0x2703;// SSL�M���͓]�������ŉ���
			}
			SPI_OUT=0xFF;		/* ���̓ǂݍ��� */
		}else{
			/* �I�� */
			s_State=CS_IDLE;
			RSPI0.SPCR.BIT.SPRIE = 0;	// RSPI��M���荞�݋֎~
		}
		break;
	}
	if(s_cReadState<4)
		++s_cReadState;
	return;
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : �ǂݍ��݂��͂��߂�A�h���X
 *	\date		Update 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
static void SetFlashAddress(unsigned long ulAddress)
{
	s_Long.ulLong=g_lAddress=ulAddress;
	s_cReadState=0;

	while(RSPI0.SPSR.BIT.IDLNF == 1){	}

	RSPI0.SPCMD0.WORD = 0x2783;			// �]���̓r����� SSL�M���̓A�N�e�B�u��ێ�
	RSPI0.SPCR.BIT.SPRIE = 1;			// RSPI��M���荞�݋���
	SPI_OUT=0x03;						/* �t���b�V���ǂݏo���J�n */
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\retval		�ǂ݂����ꂽ�f�[�^(1byte)
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
unsigned char sodiac_dataread(int iChannel,unsigned long ulAddress)
{
	unsigned long	a_ulOffset;
	unsigned long	ist;		// ���݂̊�����t���

	_ei();
	ulAddress += *s_ulFirstAddress[iChannel];
	if( 0xffffffffUL != s_ulStartAddress[iChannel])
	{
		a_ulOffset = ulAddress - s_ulStartAddress[iChannel];
	}
	else
	{
		a_ulOffset = 0;
	}

	/* ���͈͓̔��Ȃ�f�[�^�����݂��� */
	if((s_ulStartAddress[iChannel]<=ulAddress)&&(a_ulOffset<BUFFSIZE)){
		/* �����̃`�����l����ǂݏo���� */
		if(((CS_READ0==s_State)&&(0==iChannel))||
		   ((CS_READ1==s_State)&&(1==iChannel))){
			while(g_lAddress<ulAddress){ /* �ǂݏo�������Ƃ���܂œǂ܂��܂ő҂� */
			}
		}
		return	s_ucaaDataBuff[iChannel][a_ulOffset];
	}
	/* �o�b�t�@�Ƀf�[�^���Ȃ������̂œǂݍ��݂��J�n */

	/* �܂��f�[�^�ǂݍ��݂�����Ă��Ȃ����o�b�t�@�O */
	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=(0==iChannel)?CS_READ0:CS_READ1;
	s_usCount=BUFFSIZE;
	s_pucData=s_ucaaDataBuff[iChannel];
	s_ulStartAddress[iChannel]=ulAddress;

	SetFlashAddress(ulAddress);

	_ei2(ist);
	while(g_lAddress<=ulAddress); 		/* �ǂݏo�������Ƃ���܂œǂ܂��܂ő҂� */

	// �������݌��J��
	return	s_ucaaDataBuff[iChannel][0];
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\retval		�ǂ݂����ꂽ�f�[�^(1byte)
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
unsigned char sodiac_headerread(unsigned long ulAddress)
{
	unsigned char	a_ucData;
	unsigned long	ist;		// ���݂̊�����t���

	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=CS_HEAD;
	s_usCount=1;
	s_pucData=&a_ucData;

	ulAddress += *s_ulFirstAddress[0];
	SetFlashAddress(ulAddress);
	_ei2(ist);
	while(CS_IDLE!=s_State); /* �ǂݏo�����I���܂ő҂� */

	return	a_ucData;
}

/*************************************************************************/ /**
 *	\param[out]	vpPt      : �f�[�^�i�[�o�b�t�@
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\param[in]	iSize     : �f�[�^�ǂݏo���T�C�Y
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
D_SODIAC_E sodiac_nread(int iChannel, void *vpPt, unsigned long ulAddress, int iSize)
{
	unsigned char	*a_pucData;

	if(!vpPt)
		return	D_SODIAC_E_PAR;	/* �����|�C���^ */

	s_ulStartAddress[iChannel]	= 0xffffffffUL;					// �����l�ɖ߂�
	a_pucData=(unsigned char *)vpPt;
	for(;0<iSize;--iSize){
		*a_pucData++ =sodiac_dataread(iChannel, ulAddress++);
	}

	return	D_SODIAC_E_OK;
}

/*************************************************************************/ /**
 *	\param[out]	vpPt      : �f�[�^�i�[�o�b�t�@
 *	\param[in]	ulAddress : �ǂݏo���A�h���X
 *	\param[in]	iSize     : �f�[�^�ǂݏo���T�C�Y
 *	\retval		D_SODIAC_E_OK : ����
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
D_SODIAC_E sodiac_hnread(void *vpPt, unsigned long ulAddress, int iSize)
{
	unsigned long	ist;		// ���݂̊�����t���

	if(!vpPt)
		return	D_SODIAC_E_PAR;	/* �����|�C���^ */

	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=CS_HEAD;
	s_usCount=iSize;
	s_pucData=vpPt;

	ulAddress += *s_ulFirstAddress[0];
	SetFlashAddress(ulAddress);
	_ei2(ist);
	while(CS_IDLE!=s_State); /* �ǂݏo�����I���܂ő҂� */

	return	D_SODIAC_E_OK;
}

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2012		  *
 **/ /*************************************************************************/
