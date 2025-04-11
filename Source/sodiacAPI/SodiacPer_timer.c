/*************************************************************************/ /**
/*	\file	SodiacPer_timer.c
 *	\brief	Sodiac �y���t�F���� �^�C�}�[ for RL78/G13
 *	\date	CREATE : 17/Nov/2008 Arex.Co.Ltd,
 *	\date	Update : 2012/03/08  Arex.Co.Ltd,
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "iodefine.h"
#include "SodiacPer_timer.h"


/*************************************************************************/ /**
 *	PWM�^�C�}�[�J�n�֐�
 *	\date	CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date	UPDATE : 2009/10/20	Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_start_timer(void)
{
	/* ���荞�ݐݒ� */
	IEN(TPU0,TGI0A)=1;
	TPU0.TIER.BIT.TGIEA = 1;

	/* �^�C�}�[�̐ݒ� */
	TPUA.TSTR.BIT.CST0 = 1;		/* ����J�n */
}

/*************************************************************************/ /**
 *	PWM�^�C�}�[��~�֐�
 *	\date	CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date	UPDATE : 2009/10/20 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_stop_timer(void)
{
	/* �^�C�}�[�J�E���g��~ */
	TPUA.TSTR.BIT.CST0 = 0;		/* ����J�n */
	/* �^�C�}�[���荞�݋֎~ */
	TPU0.TIER.BIT.TGIEA = 0;
}

extern unsigned short	*GSODIAC_usaData;

/*************************************************************************/ /**
 *	PWM�^�C�}�[�ݒ�֐�
 *	\param[in]	usTop   : �^�C�}�[�� PWM ������ݒ肷�鐔�l
 *	\param[in]	usData  : �^�C�}�[�� ���� �f���[�e�B�[���ݒ肷�鐔�l
 *	\param[in]	usCount : �I�[�o�[�T���v�����O�̉�
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date		UPDATE : 2010/06/15 Iwasawa H.(Arex)
 *	\date		UPDATE : 2012/03/08 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_set_timer(unsigned short usTop, unsigned short usData, unsigned char usCount)
{
	/* DMA�̐ݒ� */
	DMAC0.DMCNT.BIT.DTE = 0;

	DMAC0.DMDAR = (unsigned long)&TPU0.TGRD; /* �����f�[�^�]���惌�W�X�^ */
	DMAC0.DMSAR = (unsigned long)GSODIAC_usaData; /* �����o�b�t�@ */

	DMAC0.DMCRA = usCount;

	DMAC0.DMTMD.BIT.DCTG  = 1;	/* �y���t�F��������N������ */
	DMAC0.DMTMD.BIT.SZ    = 1;	/* 16bit�]�� */
	DMAC0.DMTMD.BIT.DTS   = 2;	/* ���s�[�g�u���b�N�� */
	DMAC0.DMTMD.BIT.MD    = 0;	/* �m�[�}�����[�h */

	DMAC0.DMINT.BIT.DTIE  = 1;	/* DMA�I�����荞�݋��� */

	DMAC0.DMAMD.BIT.SM    = 2;	/* �]�����C���N�������g */
	DMAC0.DMAMD.BIT.SARA  = 0;
	DMAC0.DMAMD.BIT.DM    = 0;	/* �]����Œ� */
	DMAC0.DMAMD.BIT.DARA  = 0;

	DMAC0.DMCSL.BIT.DISEL = 0;	/* �^�C�}�[���荞�݂͍s��Ȃ� */

	DMAC0.DMCNT.BIT.DTE   = 1;	/* DMA0���� */

	DMAC.DMAST.BIT.DMST   = 1;	/* (��)�S�̂̃V�X�e���Őݒ肵�Ă������� �SDMA�L�� */

	ICU.DMRSR0.BIT.DMRS   = VECT_TPU0_TGI0A;	/* TPU0A���荞�݂��N���v���� */
	IPR(DMAC,DMAC0I)=15;		/* �ō����x�� */
	IEN(DMAC,DMAC0I)=1;			/* ���荞�ݗL�� */

	/* �ȉ���PWM�|�[�g�̐ݒ� */
	/* �ݒ肷��|�[�g��ėp���͂� */
	PORT3.PDR.BIT.B3 = 0;
	PORT3.PMR.BIT.B3 = 0;

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWE�̏����������� */
	MPC.PWPR.BIT.PFSWE = 1;		/* PFS�̏����������� */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWE�̏��������֎~ */

	/* �@�\�I�� */
	MPC.P33PFS.BIT.PSEL = 0x03;	/* TIOCD0 */

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWE�̏����������� */
	MPC.PWPR.BIT.PFSWE = 0;		/* PFS�̏��������֎~ */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWE�̏��������֎~ */

	/* �@�\���s */
	PORT3.PMR.BIT.B3 = 1;		/* TIOCD0 */

	/* �ȉ��̓^�C�}�[�ݒ� */
	MSTP_TPU0 = 0;				/* �^�C�}�[�N�� */
	TPU0.TCR.BIT.TPSC = 0;		/* �N���b�N�I�� P��1 */
	TPU0.TCR.BIT.CKEG = 1;		/* �����オ��G�b�W�g���K */
	TPU0.TCR.BIT.CCLR = 1;		/* �N���A�v�� TGRA�R���y�A�}�b�` */

	TPU0.TIORL.BIT.IOD = 5;		/* �����l1 �R���y�A��0 */

	TPU0.TMDR.BIT.BFA = 0;		/* TGRC�̓m�[�}�� */
	TPU0.TMDR.BIT.BFB = 0;		/* TGRD�̓m�[�}�� */
	/* PWM�����ݒ�  */
	TPU0.TGRA = usTop;
	/* �����f�[�^�ݒ� */
	TPU0.TGRD = usData;

	TPU0.TMDR.BIT.MD = 3;		/* PWM���[�h2 */
}

/*************************************************************************/ /**
 *	PWM�^�C�}�[�Đݒ�֐�
 *
 *	\param[in]	usTop   : �^�C�}�[�� PWM ������ݒ肷�鐔�l
 *	\param[in]	usData  : �^�C�}�[�� ���� �f���[�e�B�[���ݒ肷�鐔�l
 *	\param[in]	usCount : �I�[�o�[�T���v�����O�̉�
 *	\date		CREATE : 2010/09/14
 *	\date		UPDATE : 2011/05/14 Iwasawa H.(Arex)
 *	\date		UPDATE : 2012/03/08 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_recycle_timer(unsigned short usTop, unsigned short usData, unsigned char usCount)
{
	TPU0.TGRA = usTop;
	TPU0.TGRD = usData;
}

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2012		  *
 **/ /*************************************************************************/
