/*[]----------------------------------------------------------------------[]*/
/*|		�VI/F�Ձi�e�@�j�̑Ύq�@�ʐM�^�X�N���Ŏg�p����uRAM�v�̈�		   |*/
/*|			�ECPU����RAM���WKbyte�����Ȃ��̂Œ��ӂ��邱�ƁB				   |*/
/*|			�E�O�t��SRAM��512Kbyte����B								   |*/
/*|			�ECPU����RAM�́A���R�O�N���A�̈�		 �usection "B_InRAM"�v |*/
/*|			�E�O�t��SRAM�́A(�O��f�[�^)�ێ�����̈� �usection "B_ExtHold�v|*/
/*|							���������̈�			 �usection "B"�v	   |*/
/*|							(�����l�̂Ȃ��f�[�^���O�N���A�����̈�̂���) |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"


#pragma section					/* "B" : ���������f�[�^�̈�i=0�N���A�����j�O�t��S-RAM */
/*==================================*/
/*		SRAM area define			*/
/*==================================*/
/*----------------------------------*/
/*			toScom.c				*/
/*----------------------------------*/
t_toScom_Matrix		toScom_Matrix;						/* �v���g�R������}�g���N�X */
t_toScom_Polling	toScom_toSlave;						/* �q�@�|�[�����O��� */
t_toScom_Timer		toScom_Timer;						/* �ʐM�^�C�}�[�Q */
uchar				toScom_RcvPkt[TOS_SCI_RCVBUF_SIZE];	/* ��M(�d��)�o�b�t�@�i�^�X�N���x���j*/
ushort				toScom_PktLength;					/* ��M(�d��)�� */
uchar				toScom_ucAckSlave;					// ACK���M���[��No.
uchar				toScom_first;						// �����t���O
ulong				toScom_StartTime;					// �����ݒ著�M�^�C�}�[

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
uchar			toScomdr_RcvData[TOS_SCI_RCVBUF_SIZE];	/* ��M�f�[�^�o�b�t�@�i�h���C�o���x���j*/
ushort			toScomdr_RcvLength;						/* ��M�f�[�^���i�o�C�g���j*/
uchar			toScomdr_f_RcvCmp;						/* ��M�C�x���g */
														/*		0�F������M���Ă��Ȃ� */
														/*		1�F������M���Ă��� */
														/*		2�F��M�G���[���� */
														/*		3�FIFM�[���ԃ����N���x���Ń^�C���A�E�g���������B*/
														/*		4�FIFM�R�}���h���M��A�����҂��������o�߂����B*/
														/*		5�F�|�[�����O�������o�߂����B*/
