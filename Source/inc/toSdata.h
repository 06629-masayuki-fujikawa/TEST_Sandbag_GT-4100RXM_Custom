/*[]----------------------------------------------------------------------[]*/
/*|		�Ύq�@�ʐM�^�X�N �O���Q��(�̈�)�錾								   |*/
/*|			�E�����ɂ́A�VI/F�Ձi�e�@�j���́u�Ύq�@�ʐM�v�^�X�N��		   |*/
/*|			  �g�p����̈�̊O���Q�Ɛ錾���܂��B						   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _TO_SDATA_H_
#define _TO_SDATA_H_

#include	"IF.h"
#include	"IFM.h"
#include	"toS.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/


/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define		TOSCOM_NO_ACK_TERMINAL	(uchar)0xff		// ACK�҂��[���Ȃ�


/*==================================*/
/*		InRAM area define			*/
/*==================================*/


/*----------------------------------*/
/*			toStimr.c				*/
/*----------------------------------*/
extern	ushort	toScom_Tx2ms_Char;					/* �����ԊĎ��p�i 1msec Timer�j*/
extern	ushort	toScom_Tx10ms_Link;					/* ��M�Ď��p  �i10msec Timer�j*/
extern	ushort	toScom_Tx10ms_Line;					/* ����Ď��p  �i10msec Timer�j*/

/*----------------------------------*/
/*			toSmain.c				*/
/*----------------------------------*/
extern	uchar				toS_f_TmStart;		/** timer function start flag **/
												/* 1=Start, 0=don't do process */

/*----------------------------------*/
/*			toScom.c				*/
/*----------------------------------*/
extern	t_toScom_Timer		toScom_Timer;		/* timer value */
extern	t_toScom_Matrix		toScom_Matrix;		/* */
extern	t_toScom_Polling	toScom_toSlave;		/* �q�@�|�[�����O��� */
extern	uchar				toScom_RcvPkt[TOS_SCI_RCVBUF_SIZE];	/* ��M(�d��)�o�b�t�@�i�^�X�N���x���j*/
extern	ushort				toScom_PktLength;	/* ��M(�d��)�� */
extern	uchar				toScom_ucAckSlave;	// ACK���M���[��No.
extern	uchar				toScom_first;		// �����t���O
extern	ulong				toScom_StartTime;	// �����ݒ著�M�^�C�}�[

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
extern	uchar			toScomdr_RcvData[TOS_SCI_RCVBUF_SIZE];	/* ��M�o�b�t�@�i�h���C�o���x���j*/
extern	ushort			toScomdr_RcvLength;						/* ��M�f�[�^�� */
extern	uchar			toScomdr_f_RcvCmp;						/* ��M�C�x���g */
extern	t_SciRcvError	toS_RcvErr;								/* ��M�G���[��� */
extern	t_SciComError	toS_ComErr;								/* �ʐM�G���[��� */

#endif	// _TO_SDATA_H_
