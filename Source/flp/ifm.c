/*[]----------------------------------------------------------------------[]*/
/*|		RAM Memory Data in New I/F(Master)								   |*/
/*|			�E�����ɂ́A�VI/F�Ձi�e�@�j���̑S�R�^�X�N���Q�Ƃ���̈��	   |*/
/*|			  ��`���܂��B												   |*/
/*|			�ECPU����RAM�ƁA�O�t��S-RAM�����݂��Ă��܂��B				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-03-22                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"LKcom.h"
#include	"IFM.h"
#include	"LCM.h"

/*==================================*/
/*		InRAM area define			*/
/*==================================*/
/*
*	2005.04.07 ���{�����E��茈��
*	�@�^�C�}�[�֘A�ϐ�
*	�A�X�^�b�N�̈�i�SKbyte�j
*	�B�V���A������M�o�b�t�@�i���荞�݃n���h�����L�̂��́j
*/

/*----------------------------------*/
/*			LCMtime.c				*/
/*----------------------------------*/
//�^�C�}�[�J�E���^
ushort	LCMtim_Tx10ms_Action[IF_SLAVE_MAX][IFS_ROOM_MAX];				/* 10msec�P�ʁF����^�C�}�[(�̏ጟ�m) */
ushort	LCMtim_Tx1sec_Retry[IF_SLAVE_MAX][IFS_ROOM_MAX];				/*   1sec�P�ʁF�̏᎞���g���C�Ԋu */
ushort	LCMtim_Tx1sec_Wait;						/*   1sec�P�ʁF�ȉ��Ɏg�p���� */
												/*				�@�o�[�W�����v���ɑ΂��鉞���҂� */
												/*				�A�S���b�N�J/�� */
												/*				�B�S/�ʃ��b�N�J�e�X�g */
uchar	LCM_f_TmStart;							/* �^�C�}�[����J�n�t���O */
												/*		1=�^�C�}�[���삷��i�J�E���g����j*/
												/*		0=�^�C�}�[���삵�Ȃ��i�J�E���g���Ȃ��j*/

/*----------------------------------*/
/*	toS\toStime.c					*/
/*----------------------------------*/
ushort	toScom_Tx2ms_Char;						/* �����ԊĎ��p�i 1msec Timer�j*/
ushort	toScom_Tx10ms_Link;						/* ��M�Ď��p  �i10msec Timer�j*/
ushort	toScom_Tx10ms_Line;						/* ����Ď��p  �i10msec Timer�j*/
uchar	toS_f_TmStart = 0;						/* �^�C�}�[����J�n�t���O */
												/*		1=�^�C�}�[���삷��i�J�E���g����j*/
												/*		0=�^�C�}�[���삵�Ȃ��i�J�E���g���Ȃ��j*/
/*----------------------------------*/
/*	toS\toSsci.c					*/
/*----------------------------------*/
t_SciRcvCtrl	toS_RcvCtrl;					/* SCI��M�Ǘ����i���荞�݃n���h���g�p�j*/
t_SciSndCtrl	toS_SndCtrl;					/* SCI���M�Ǘ����i���荞�݃n���h���g�p�j*/
unsigned char	toS_RcvBuf[TOS_SCI_RCVBUF_SIZE];/* SCI��M�o�b�t�@�i���荞�݃n���h���g�p�j*/
unsigned char	toS_SndBuf[TOS_SCI_SNDBUF_SIZE];/* SCI���M�o�b�t�@�i���荞�݃n���h���g�p�j*/
t_SciRcvError	toS_RcvErr;						/* �V���A����M�G���[���i�ȉ��̔����񐔁j*/
												/*		�@�I�[�o�[�����G���[������ */
												/*		�A�t���[�~���O�G���[������ */
												/*		�B�p���e�B�G���[������ */
t_SciComError	toS_ComErr;						/* �ʐM�G���[��� */

/*==================================*/
/*		SRAM area define			*/
/*==================================*/
/*
*	2005.04.07 ���{�����E��茈��
*	�@�����ݒ�f�[�^�i���b�N�Ǘ��e�[�u���Ɋ܂܂��j
*	�A�L���[
*	�B�q�@�̏�ԁi���b�N�Ǘ��e�[�u���Ɋ܂܂��j
*/
//���ʁi���L�҂Ȃ��j
t_IFM_InitData	IFM_LockTable;				/* ���b�N���u�Ǘ��e�[�u���i�����ݒ�f�[�^�A�q�@�̏�Ԃ��܂ށj*/

/************************************
*	�L���[
*	�E�Z���N�e�B���O�҂����X�g
*	�E�q�@��ԃ��X�g
*	�E���b�N���슮���҂����X�g
*************************************/

/*
*	�Z���N�e�B���O�҂����X�g
*	�E�Ύq�@�ʐM�^�X�N���A�q�@�֑��肽���u�f�[�^����v�̂Ƃ��A���ߍ��ރL���[�B
*	  �|�[�����O�������o�߂�����A�|�[�����O(��ԗv���f�[�^)��������
*	  �q�@�֑������B�E�E�E(�����Ȃ�)�Z���N�e�B���O�B
*	�E�ȉ����Ώ�
*		1)(49H�F'I')�����ݒ�f�[�^
*		2)(57H�F'W')��ԏ������݃f�[�^
*		3)(52H�F'R')��ԗv���f�[�^�i�o�[�W�����v���j
*		��(52H�F'R')��ԗv���f�[�^�i���|�[�����O�j�͗��܂�Ȃ��̂Œ��ӂ̂��ƁB
*
*	2005.06.22 �����E�ύX
*	�E�L���[���̂́A���ԃ{�[�h�ɂȂ��ꂽ�܂܉�����Ȃ��Ƃ������邽�߁A���߂Ɋm�ۂ���B
*/
t_IFM_Queue			toSque_Body_Select[TOS_Q_WAIT_SELECT * 2];	/* �L���[���� */
struct t_IF_Queue	toSque_Buff_Select[TOS_Q_WAIT_SELECT];		/* �L���[�{�� */
t_IF_Queue_ctl		toSque_Ctrl_Select;							/* �L���[�Ǘ���� */

/*
*	�q�@��ԃ��X�g
*	�E�Ύq�@�ʐM�^�X�N���A��M�����d�������b�N�Ǘ��^�X�N�֒m�点�����Ƃ��A���ߍ��ރL���[�B
*	�E�ȉ����Ώ�
*		1)(41H�F'A')�q�@��ԃf�[�^
*		2)(56H�F'V')�q�@�o�[�W�����f�[�^
*/
t_IFM_Queue			toSque_Body_Status[LCM_Q_SLAVE_STATUS];	/* �L���[���� */
struct t_IF_Queue	toSque_Buff_Status[LCM_Q_SLAVE_STATUS];	/* �L���[�{�� */
t_IF_Queue_ctl		toSque_Ctrl_Status;						/* �L���[�Ǘ���� */

/*
*	���b�N���쓮�슮���҂����X�g
*	�E���b�N�Ǘ��^�X�N���A�q�@�֓���v���𑗂肽�����u���쒆�v�̂Ƃ��A���ߍ��ރL���[�B
*	  �q�@��ԃf�[�^�ɂāu���슮���v������A�q�@�֓���v������B�܂���
*	  �q�@�֑���������v�����^�C���A�E�g������A���̓���v������B
*	�E�ȉ����Ώ�
*		1)(57H�F'W')��ԏ������݃f�[�^
*/
t_IFM_Queue			LCMque_Body_Action[LCM_Q_WAIT_ACTION];	/* �L���[���� */
struct t_IF_Queue	LCMque_Buff_Action[LCM_Q_WAIT_ACTION];	/* �L���[�{�� */
t_LCMque_Ctrl_Action  LCMque_Ctrl_Action;	// �L���[�Ǘ����

/* ���b�N���u���[�J�[�ʂ̐M���o�͎��ԃe�[�u�� */
t_IF_Signal	IF_SignalOut[LOCK_MAKER_END];



/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
//
uchar	IFM_GetBPSforSalve(void);

//�L���[�֘A
void				EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere);
struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl);
int					CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere);
struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere);
struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize);
t_IFM_Queue			*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize);


//�G���[����/��������
void	IFM_MasterError(int iError, uchar ucNow);
void	IFM_SlaveError(int iError, uchar ucNow, int iSlave);

void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry);
struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl);
struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere);
int	MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock);

void	PAYcom_NoticeError(uchar ucTermNo, uchar ucError, uchar ucState);

int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno);
int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount);
int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl);

#pragma section
/*----------------------------------*/
/*			table define			*/
/*----------------------------------*/

/* ���b�N���u���[�J�[�ʂ̐M���o�͎��ԃe�[�u���i�f�t�H���g�j*/
const t_IF_Signal	IF_SignalOutDefault[LOCK_MAKER_END] = {
	{	 0,		 0},			/* [0]�F�ڑ��Ȃ��ɂ͗��Ȃ� */
	{	40,		40},			/* [1]�F=4000msec	�p�c�G���W�j�A�����O�����b�N���u�i���]�ԁj	*/
	{	20,		20},			/* [2]�F=2000msec	�p�c�G���W�j�A�����O�����b�N���u�i�o�C�N�j	*/
	{	 4,		 4},			/* [3]�F= 400msec	�g�����쏊�����b�N���u�i���]�ԁA�o�C�N���p�j*/
	{	 4,		 4},			/* [4]�F= 400msec	�R���Y�W���p�������b�N���u�i���]�ԁj		*/
	{	 4,		 4}				/* [5]�F= 400msec	HID�����b�N���u�i���]�ԁA�o�C�N���p�j		*/
};

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_GetBPSforSalve() 		   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�`�����x���擾�i�Ύq�@�j										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	IFM_GetBPSforSalve(void)
{
	/* DIP-SW����ǂݏo�����A�`�����x��Ԃ��B*/
	return IFM_LockTable.ucBitRateForSlave;
}


/*[]----------------------------------------------------------------------[]*/
/*|             IFM_MasterError()			   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�G���[�ʒm����(�e����)											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_MasterError(int iError, uchar ucNow)
{
	uchar	last;
	uchar	errorCode;

	/*�u�F���ł��Ȃ��v�G���[�͏��O */
	if (iError >= E_IFM_END ||
		iError < 0)
	{
		return;
	}

	/* ���񂪁u�����������H�v�ŕ��� */
	last = IFM_LockTable.sIFMerror[iError].ucState;
	switch (ucNow) {
		case 0:		/* �����E�E�E���ɔ������Ă�����H�̏����t */
			if (last == 1) {
				IFM_LockTable.sIFMerror[iError].ucState = ucNow;	/* ������ */
			}
			/* ���ɉ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
			else {
				return;
			}
			break;

		case 1:		/* �����E�E�E�O�ɔ������Ă��Ȃ���΁H�̏����t */
			if (last != 1) {
				IFM_LockTable.sIFMerror[iError].ucState = ucNow;	/* ������ */
				IFM_LockTable.sIFMerror[iError].ulCount++;			/* ������������ */
			}
			/* ���ɔ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
			else {
				return;
			}
			break;

		case 2:		/* ���������E�����E�E�E�����Ȃ� */
			IFM_LockTable.sIFMerror[iError].ucState = ucNow;		/* ���������E������ */
			IFM_LockTable.sIFMerror[iError].ulCount++;				/* ������������ */
			break;

		default:
			/* �����u�ρv�Ȃ̂Œʒm�s�v */
			return;
	}

	/*
	*	�����ɂēn�����G���[�́A�����Ǘ��ԍ��ł���B
	*	�Ȃ̂ŁA
	*	���Z�@�ւ̃G���[�R�[�h�֕ϊ�/��������B
	*/
	switch (iError) {
		case E_PAY_Parity:				errorCode =  1;	break;
		case E_PAY_Frame:				errorCode =  2;	break;
		case E_PAY_Overrun:				errorCode =  3;	break;
		case E_PAY_PACKET_HeadStr:		errorCode =  4;	break;
		case E_PAY_PACKET_DataSize:		errorCode =  5;	break;
		case E_PAY_PACKET_WrongKind:	errorCode =  6;	break;
		case E_PAY_NAK_Retry:			errorCode =  7;	break;
		case E_PAY_NO_Answer:			errorCode =  8;	break;
		case E_PAY_PACKET_WrongCode:
		case E_PAY_PACKET_CRC:			errorCode =  9;	break;
		case E_PAY_PACKET_Headerless:	errorCode = 10;	break;
		case E_NT_NET_Protocol:			errorCode = 11;	break;
		case E_PAY_PACKET_DUPLICATE:	errorCode = 12;	break;
		case E_PAY_PACKET_Timeover:		errorCode = 13;	break;
		case E_PAY_QFULL_PollingWait:	errorCode = 20;	break;
		case E_IFM_QFULL_SlaveStatus:	errorCode = 21;	break;
		case E_IFM_QFULL_LockAction:	errorCode = 22;	break;
		case E_IFS_QFULL_SelectingWait:	errorCode = 23;	break;
		case E_IFM_INIT_Data:			errorCode = 30;	break;
		case E_IFM_CTRL_Command:		errorCode = 31;	break;
		case E_IFM_TEST_Next:			errorCode = 32;	break;
		case E_IFM_MODE_Change:			errorCode = 33;	break;
		case E_IFM_Password:			errorCode = 90;	break;

		case E_IFM_BUG:
		default:
			return;
	}

	/* ���Z�@�֒ʒm���� */
	PAYcom_NoticeError(0, errorCode, ucNow);
}

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_SlaveError()			   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�G���[�ʒm����(�q����)											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_SlaveError(int iError, uchar ucNow, int iSlave)
{
	uchar	last;
	uchar	errorCode;

	/*�u�F���ł��Ȃ��v�G���[�͏��O */
	if (iError >= E_IFS_END ||
		iError < 0)
	{
		return;
	}

	/* �O�񂪁u�����������H�v�ŕ��� */
	if( IFS_CRR_OFFSET > iSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
		last = IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState;
		switch (ucNow) {
			case 0:		/* �����E�E�E���ɔ������Ă�����H�̏����t */
				if (last == 1) {
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;	/* ������ */
				}
				/* ���ɉ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
				else {
					return;
				}
				break;

			case 1:		/* �����E�E�E�O�ɔ������Ă��Ȃ���΁H�̏����t */
				if (last != 1) {
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;	/* ������ */
					IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ulCount++;		/* ������������ */
				}
				/* ���ɔ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
				else {
					return;
				}
				break;

			case 2:		/* ���������E�����E�E�E�����Ȃ� */
				IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ucState = ucNow;		/* ���������E������ */
				IFM_LockTable.sSlave[iSlave].sIFSerror[iError].ulCount++;			/* ������������ */
				break;

			default:
				break;
		}
	} else {						// CRR��ւ̃A�N�Z�X�̏ꍇ
		last = IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState;
		switch (ucNow) {
			case 0:		/* �����E�E�E���ɔ������Ă�����H�̏����t */
				if (last == 1) {
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;	/* ������ */
				}
				/* ���ɉ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
				else {
					return;
				}
				break;

			case 1:		/* �����E�E�E�O�ɔ������Ă��Ȃ���΁H�̏����t */
				if (last != 1) {
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;	/* ������ */
					IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ulCount++;		/* ������������ */
				}
				/* ���ɔ������Ă���̂Ȃ�u�ω��Ȃ��v�Ȃ̂Œʒm�s�v */
				else {
					return;
				}
				break;

			case 2:		/* ���������E�����E�E�E�����Ȃ� */
				IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ucState = ucNow;		/* ���������E������ */
				IFM_LockTable.sSlave_CRR[ iSlave - IFS_CRR_OFFSET ].sIFSerror[iError].ulCount++;			/* ������������ */
				break;

			default:
				break;
		}
	}

	/*
	*	�����ɂēn�����G���[�́A�����Ǘ��ԍ��ł���B
	*	�Ȃ̂ŁA
	*	���Z�@�ւ̃G���[�R�[�h�֕ϊ�/��������B
	*/
	switch (iError) {
		case E_IFS_Parity:				errorCode =  1;	break;
		case E_IFS_Frame:				errorCode =  2;	break;
		case E_IFS_Overrun:				errorCode =  3;	break;
		case E_IFS_PACKET_HeadStr:		errorCode =  4;	break;
		case E_IFS_PACKET_DataSize:		errorCode =  5;	break;
		case E_IFS_PACKET_WrongKind:	errorCode =  6;	break;
		case E_IFS_NAK_Retry:			errorCode =  7;	break;
		case E_IFS_NO_Answer:			errorCode =  8;	break;
		case E_IFS_PACKET_CRC:			errorCode =  9;	break;
		case E_IFS_PACKET_Headerless:	errorCode = 10;	break;
		case E_IF_Protocol:				errorCode = 11;	break;
		case E_IFS_PACKET_WrongSlave:	errorCode = 14;	break;

		case E_IFS_BUG:
		default:
			return;
	}

	/* ���Z�@�֒ʒm���� */
	PAYcom_NoticeError((uchar)(iSlave +1), errorCode, ucNow);
}

/*[]----------------------------------------------------------------------[]*/
/*|             EnQueue                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�L���[�֑}������B											       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 pQue	�}������L���[���w��								   |*/
/*|			 iWhere	�ǂ��ɑ}�����邩�H���w��i�擪���牽�Ԗځj			   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���j������iWhere�̈Ӗ�											       |*/
/*|				 = 0�F�擪�֑}��									       |*/
/*|				���l�F���Ԃ֑}���i�O���琔����ʒu�B���̈ʒu�̑O�ɑ}���j   |*/
/*|					  ���݂̃L���[���ȏ�̈ʒu���w�肷��ƁA�����ɓ����   |*/
/*|				���l�F�����֑}��									       |*/
/*|																	       |*/
/*|	��j���݂̃L���[�����R�ŁA�O���w�肷��ƁA�擪�֑}��(�V�͂P�Ԗ�)	   |*/
/*|							  �P���w�肷��ƁA�Q�Ԗڂ̑O�֑}��(�V�͂Q�Ԗ�) |*/
/*|							  �Q���w�肷��ƁA�R�Ԗڂ̑O�֑}��(�V�͂R�Ԗ�) |*/
/*|							  �R���w�肷��ƁA�����֑}��(�V�͂S�Ԗ�)	   |*/
/*|							�|�P���w�肷��ƁA�����֑}��				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :  2005-09-07     �L���[�ێ����t���O�̏�������ǉ�         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere)
{
struct t_IF_Queue	*pNow;
	int		ni;

	/* ���A�ЂƂ��Ȃ��Ƃ��́i�����ɓ���邩�H�Ɋւ�炸�j���ׂē������� */
	if (pCtl->usCount == 0) {
		pCtl->usCount = 1;
		pAdd->bQueue = 1;							/* �L���[�{�̎g�p���� */
		pAdd->pData->sCommon.bUsedByQue = 1;		/* �L���[���̎g�p���� */
		pAdd->pData->sCommon.bHold = 0;				/* �L���[�ێ�������		2005.09.07 �����E�ǉ� */
		pAdd->pPrev = NULL;							/* �����̑O�͂��Ȃ� */
		pAdd->pNext = NULL;							/* �����̎������Ȃ� */
		pCtl->pTop = pAdd;							/* �������擪�ɂȂ� */
		pCtl->pEnd = pAdd;							/* �����������ɂȂ� */
	}

	/* ���A�ЂƂȏ゠��H*/
	else if (pCtl->usCount > 0) {

		/* �擪�ɑ}���H*/
		if (iWhere == 0) {
			pAdd->pPrev = NULL;						/* �����̑O�͂��Ȃ� */
			pAdd->pNext = pCtl->pTop;				/* �����̎��͍��܂ł̐擪 */
			pCtl->pTop->pPrev = pAdd;				/* ���܂ł̑O������ */
			pCtl->pTop = pAdd;						/* �������擪�ɂȂ� */
		}

		/* �����ɑ}���H*/
		else if (iWhere < 0 ||						/* ���l�́u�����v�̈� */
				 iWhere >= pCtl->usCount)			/* ���݂̃L���[���ȏ� */
		{
			pAdd->pPrev = pCtl->pEnd;				/* �����̑O�͍��܂ł̖��� */
			pAdd->pNext = NULL;						/* �����̎��͂͂��Ȃ� */
			pCtl->pEnd->pNext = pAdd;				/* ���܂ł̎������� */
			pCtl->pEnd = pAdd;						/* �����������ɂȂ� */
		}

		/* ���Ԃɑ}�� */
		else {
			pNow = pCtl->pTop;						/* �����̎��ɗ���ׂ�����(��pNow)��T�� */
			for (ni=0; ni < iWhere; ni++) {
				pNow = pNow->pNext;
			}										/* pNow�̑O�Ɏ�����}������ */
			pAdd->pPrev = pNow->pPrev;				/* �����̑O�͍��܂ł̑O */
			pAdd->pNext = pNow;						/* �����̎��͍��܂ł��̂��� */
			pNow->pPrev->pNext = pAdd;				/* ���܂ł̑O�̎������� */
			pNow->pPrev = pAdd;						/* ���܂ł̑O������ */
		}

		pCtl->usCount++;							/* �L���[���₷ */
		pAdd->bQueue = 1;							/* �L���[�{�̎g�p���� */
		pAdd->pData->sCommon.bUsedByQue = 1;		/* �L���[���̎g�p���� */
		pAdd->pData->sCommon.bHold = 0;				/* �L���[�ێ�������		2005.09.07 �����E�ǉ� */
	}

	/* �����ɗ�����o�O */
	else {
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueue                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�擪�̃L���[���擾�B										       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*| return : 		�擾�����L���[�ւ̃|�C���^	 						   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���j�߂�l��0�iNULL�|�C���^�j�̏ꍇ�A�擾�ł��Ȃ����Ƃ��Ӗ�����B      |*/
/*|		�߂�l��0�iNULL�|�C���^�j�̏ꍇ�A�擾�ł��Ă͂��邪				   |*/
/*|										 �w��̃L���[����͊O����Ă��Ȃ�  |*/
/*|										 �̂Œ��ӂ̂��ƁB				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl)
{
	if (pCtl->usCount) {
		return pCtl->pTop;
	}

	return (struct t_IF_Queue *)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             CheckQueue                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�w�肳�ꂽ�L���[��T���B									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 pQue	�T���L���[���w��									   |*/
/*|			 iwhat	����T�����H���w��									   |*/
/*| return : 		�������L���[�̈ʒu�i�擪���牽�Ԗځj				   |*/
/*|			�i < 0�j���l�F������Ȃ�����							       |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���j������iwhat�̈Ӗ�											       |*/
/*|			 = 0�F���̂���											       |*/
/*|			 = 1�F													       |*/
/*|			 = 2�F													       |*/
/*|			 = 3�F													       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere)
{
struct t_IF_Queue	*pNow;
	int		ni;

/*
*	�u���̂��́v�̗�
*/
	pNow = pCtl->pTop;
	for (ni=0; ; ni++) {
		if (pNow == pQue) {
			return ni;
		}
		if (!pNow->pNext) {
			return (-1);
		}
		pNow = pNow->pNext;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DeQueue                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�w�肳�ꂽ�L���[���O���B									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 iWhere	�ǂ�����O�����H���w��i�擪���牽�Ԗځj			   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���j������iWhere�̈Ӗ�											       |*/
/*|				 = 0�F�擪���O��									       |*/
/*|				���l�F���Ԃ��O���i�O���琔����ʒu�B���̈ʒu���O���j	   |*/
/*|				�i���݂̃L���[���|�P�j�ȏ�̈ʒu���w�肷��ƁA�������O��   |*/
/*|				���l�F�������O��									       |*/
/*|																	       |*/
/*|	��j���݂̃L���[�����R�ŁA�O���w�肷��ƁA�擪(�P�Ԗ�)���O��		   |*/
/*|							  �P���w�肷��ƁA�Q�Ԗڂ��O��				   |*/
/*|							  �Q���w�肷��ƁA����(�R�Ԗ�)���O��		   |*/
/*|							  �R���w�肷��ƁA�������O��				   |*/
/*|							�|�P���w�肷��ƁA�������O��				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :  2005-09-07     �L���[�ێ����t���O�̏�������ǉ�         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere)
{
struct t_IF_Queue	*pDel;
	int		ni;

	/* ���A�ЂƂ��Ȃ��Ƃ��͉������Ȃ� */
	if (pCtl->usCount == 0) {
		return (struct t_IF_Queue *)0;
	}

	/* ���A�ЂƂ̂Ƃ��́i��������O�����H�Ɋւ�炸�j���ׂē������� */
	if (pCtl->usCount == 1) {
		pDel = pCtl->pTop;						/* ����܂ł̐擪���u�O�����Ώہv*/
		pCtl->usCount = 0;							/* �L���[�Ȃ� */
		pCtl->pTop->bQueue = 0;						/* �L���[�{�̖��g�p�� */
		pCtl->pTop->pData->sCommon.bUsedByQue = 0;	/* �L���[���̖��g�p�� */
		pCtl->pTop->pData->sCommon.bHold = 0;		/* �L���[�ێ�������		2005.09.07 �����E�ǉ� */
		pCtl->pTop = NULL;							/* �擪�͂��Ȃ� */
		pCtl->pEnd = NULL;							/* �����͂��Ȃ� */
	}

	/* ���A�ӂ��ȏ゠��H*/
	else if (pCtl->usCount > 1) {
		/* �擪���O���H*/
		if (iWhere == 0) {
			pDel = pCtl->pTop;						/* ����܂ł̐擪���u�O�����Ώہv*/
			pCtl->pTop = pDel->pNext;				/* �V�����擪�́u����܂ł̐擪�v�̎� */
			pCtl->pTop->pPrev = NULL;				/* �V�����擪�̑O�͂��Ȃ� */
		}
		/* �������O���H*/
		else if (iWhere < 0 ||						/* ���l�́u�����v�̈� */
				 iWhere >= (pCtl->usCount-1))		/*�i���݂̃L���[���|�P�j�ȏ� */
		{
			pDel = pCtl->pEnd;						/* ����܂ł̖������u�O�����Ώہv*/
			pCtl->pEnd = pDel->pPrev;				/* �V���������́u����܂ł̖����v�̑O */
			pCtl->pEnd->pNext = NULL;				/* �V���������̎��͂��Ȃ� */
		}
		/* ���Ԃ��O���H*/
		else {
			pDel = pCtl->pTop;						/*�u�O�����Ώہv�́A*/
			for (ni=0; ni < iWhere; ni++) {			/* �擪����[iWhere]�Ԗ� */
				pDel = pDel->pNext;					/*�u�O�����Ώہv�������Ƃ��A��������ɂ���� */
			}										/* �ォ��A���O�A�����A����ƂȂ� */
			(pDel->pPrev)->pNext = (pDel->pNext);	/* ���O�̎������܂܂ł͎��������A���ꂩ��͒���ƂȂ� */
			(pDel->pNext)->pPrev = (pDel->pPrev);	/* ����̑O�����܂܂ł͎��������A���ꂩ��͒��O�ƂȂ� */
		}

		pCtl->usCount--;							/* �L���[���炷 */
		pDel->bQueue = 0;							/* �L���[�{�̖��g�p�� */
		pDel->pData->sCommon.bUsedByQue = 0;		/* �L���[���̖��g�p�� */
		pDel->pData->sCommon.bHold = 0;				/* �L���[�ێ�������		2005.09.07 �����E�ǉ� */
	}

	/* �����ɗ�����o�O */
	else {
	}

	return pDel;
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueueBuff() 				   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�󂫃L���[�{�̂�T��											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pQ			�ǂ̃L���[�{�̂��H���w��						   |*/
/*|			 byteSize	�L���[�{��(�z��)�T�C�Y���o�C�g���Ŏw��			   |*/
/*| return : �������L���[�{��											   |*/
/*|			 ��0�F�������L���[�{�̂ւ̃|�C���^					       |*/
/*|			 ��0�F������Ȃ�����									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize)
{
	int		ni;
	ushort	max;

	max = (ushort)(byteSize / sizeof(struct t_IF_Queue));

	for (ni=0; ni < max; ni++) {
		if (pQ->bQueue == 0) {
			return pQ;
		}
		pQ++;
	}

	return (struct t_IF_Queue *)0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             GetQueueBody() 				   	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�󂫃L���[���̂�T��											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pQ			�ǂ̃L���[���̂��H���w��						   |*/
/*|			 byteSize	�L���[����(�z��)�T�C�Y���o�C�g���Ŏw��			   |*/
/*| return : �������L���[����											   |*/
/*|			 ��0�F�������L���[���̂ւ̃|�C���^					       |*/
/*|			 ��0�F������Ȃ�����									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-02-24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
t_IFM_Queue	*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize)
{
	int		ni;
	ushort	max;

	max = (ushort)(byteSize / sizeof(t_IFM_Queue));

	for (ni=0; ni < max; ni++) {
		if (pQ->sCommon.bUsedByQue == 0) {
			return pQ;
		}
		pQ++;
	}

	return (t_IFM_Queue *)0;
}


/*[]----------------------------------------------------------------------[]*/
/*|             EnQueue2                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�L���[�֑}������B											       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 pQue	�}������L���[���w��								   |*/
/*|			 iWhere	�ǂ��ɑ}�����邩�H���w��i�擪���牽�Ԗځj			   |*/
/*|			 bRetry	TRUE�F���g���C�^FALSE�F�V�K							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2006-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry)
{
	if (bRetry) {
		EnQueue(&pCtl->Retry, pAdd, iWhere);
	} else {
		EnQueue(&pCtl->New, pAdd, iWhere);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DeQueue2                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���M�p�Ɏw�肳�ꂽ�L���[���O��								       |*/
/*|		(������LCMque_Ctrl_Action��p)								       |*/
/*|		�V�K�L���[�����g���C�L���[�̏��ŃT�[�`						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 iWhere	�ǂ�����O�����H���w��i�擪���牽�Ԗځj			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl)
{
	struct t_IF_Queue	*pQbuff;
	t_IFM_Queue			*pQbody;
	int		del;
	int		index;
	struct t_IF_Queue* pPrev;

	// �V�K�L���[���T�[�`
	pQbuff = GetQueue(&pCtl->New);
	if (pQbuff) {
		index = 0;
		while(pQbuff) {
			if(IFM_Can_Action(LKcom_Search_Ifno(pQbuff->pData->sCommon.ucTerminal + 1))) {
		// ���b�N���u����v���̏ꍇ�̂݁A
		// �V�K�v�����M�惍�b�N���u�ɑ΂��āA���g���C�L���[�ɂ��v��������΂�����폜
		// ������{�֐����ĂԂ̂̓��b�N���u�ɑ΂��鐧�䎞�݂̂Ȃ̂Ŗ������Ɉȉ��̏������s��
		pQbody = pQbuff->pData;
		// ���g���C�L���[���T�[�`
		while (1) {
			del = MatchRetryQueue_Lock(&pCtl->Retry, pQbody->sW57.sCommon.ucTerminal, pQbody->sW57.ucLock);
			// ���b�N���u����v�Ȃ�΂��̗v�����폜
			if (del >= 0) {
				DeQueue(&pCtl->Retry, del);
			}
			if (del == -1) break;
		}
		// �֘A����đ��^�C�}�X�g�b�v
				LCMtim_1secTimerStop((int)pQbuff->pData->sW57.sCommon.ucTerminal, (int)pQbody->sW57.ucLock);
				DeQueue(&pCtl->New, index);
				return pQbuff;
			}
			pPrev = pQbuff;
			pQbuff = pPrev->pNext;
			++index;
		}
	}
	// ���g���C�L���[���T�[�`
	pQbuff = DeQueue(&pCtl->Retry, 0);

	return pQbuff;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RemoveQueue2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�ŌÂ̗v���폜�p�Ɏw�肳�ꂽ�L���[���O��					       |*/
/*|		(������LCMque_Ctrl_Action��p)								       |*/
/*|		���g���C�L���[���V�K�L���[�̏��ŃT�[�`						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pCtl	�ǂ̃L���[���H���w��		 						   |*/
/*|			 iWhere	�ǂ�����O�����H���w��i�擪���牽�Ԗځj			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere)
{
	struct t_IF_Queue	*pQbuff;

	// ���g���C�L���[���T�[�`
	pQbuff = DeQueue(&pCtl->Retry, iWhere);
	if (pQbuff) {
		return pQbuff;
	}
	// �V�K�L���[���T�[�`
	pQbuff = DeQueue(&pCtl->New, iWhere);

	return pQbuff;
}


/*[]----------------------------------------------------------------------[]*/
/*|             MatchRetryQueue_Lock                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCM���g���C�L���[����A�w�肳�ꂽ���b�N���u�ւ̐���d���v����      |*/
/*|		�T�[�`���A�擪����̏��Ԃ�Ԃ�(0�`)							       |*/
/*|		������Ȃ����-1��Ԃ�									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : ucSlave	�q�@�ԍ�				 						   |*/
/*|			 ucLock		���b�N���u�ԍ�									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2005-10-26                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
int MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock)
{
	struct t_IF_Queue *pSearch;
	int current = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (current < pCtl->usCount) {
			if ((pSearch->pData->sW57.sCommon.ucTerminal == ucSlave) &&
				(pSearch->pData->sW57.ucLock == ucLock)) {
				// ��v
				return current;
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             PAYcom_NoticeError()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���Z�@�ւ̃G���[�f�[�^�ʒm										   |*/
/*|		�E�ΐ��Z�@�����҂��L���[�ցu�G���[�f�[�^�v���Ԃ����ށB			   |*/
/*|		�E�����ݒ�f�[�^�����Ă���Ɍ���i����ȑO�͒ʒm�����j			   |*/
/*|		�E�u�L���[�t���v���́A�Â����̂��̂āA�V�������̂��c��			   |*/
/*|		�EIFM_MasterError(IFM.c) ����сA								   |*/
/*|		  IFM_SlaveError(IFM.c) ����A�Ăяo�����						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-21                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PAYcom_NoticeError(uchar ucTermNo, uchar ucError, uchar ucState)
{
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];		// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B

	// �L���[���̂ցu�G���[�f�[�^�v���Z�b�g 
	pQbody->s66.sCommon.ucKind    = 0x66;			// �f�[�^��ʂ́u(66H)�G���[�f�[�^�v�Œ� 
	pQbody->s66.sCommon.bHold     = 1;				// �f�[�^�ێ��t���O
	pQbody->s66.sCommon.t_no  = (uchar)ucTermNo;		// 0����̒[��No��ݒ�
	pQbody->s66.ucErrTerminal  = ucTermNo;			// �[�����F�e�Ȃ� =0 �Œ�A�q�Ȃ� =1�`15
	pQbody->s66.ucErrCode      = ucError;			// �G���[�R�[�h�͈��������� 
	pQbody->s66.ucErrState     = ucState;			// �G���[��Ԃ����������� 
	len = 6;
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);		// �A�v��������M����o�b�t�@�ɃZ�b�g
}

/*[]----------------------------------------------------------------------[]*/
/*|             Seach_Close_command_sameno                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCM�L���[�̐擪���琧��d���v�����T�[�`���A(�t���b�v�㏸)�w��      |*/
/*|		����d��������������Ԏ��̏ꍇ�A�擪����̏��Ԃ�Ԃ�(0�`)      |*/
/*|		������Ȃ����-1��Ԃ�									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	�^�[�~�i��No										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2014-01-20                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno)
{
	struct t_IF_Queue *pSearch;
	int current = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (current < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// �t���b�v���u
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // �t���b�v�㏸�̏ꍇ
					if(pSearch->pData->sW57.sCommon.ucTerminal == tno){// ����^�[�~�i��No
						// ��v
						return current;
					}
				}
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}
/*[]----------------------------------------------------------------------[]*/
/*|             Seach_Close_command                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCM�L���[����A����d���v�����T�[�`���A(�t���b�v�㏸)���w������    |*/
/*|		����d���������A�擪����̏��Ԃ�Ԃ�(0�`)					       |*/
/*|		������Ȃ����-1��Ԃ�									       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : scount	�L���[�̉��Ԗڂ��猟�����邩0�`(0���擪)			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2013-11-28                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount)
{
	struct t_IF_Queue *pSearch;
	int current = 0;
	ushort i;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		for(i = 0 ; i < scount ; i++){// �L���[�̉��Ԗڂ���T�[�`���邩
			pSearch = pSearch->pNext;
			current++;
		}
		while (current < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// �t���b�v���u
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // �t���b�v�㏸�̏ꍇ
					// ��v
					return current;
				}
			}
			pSearch = pSearch->pNext;
			current++;
		}
	}
	return -1;
}
/*[]----------------------------------------------------------------------[]*/
/*|             Get_Close_command_cnt                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		LCM�L���[����A����d���v�����T�[�`���A(�㏸�^���b�N��)���w������  |*/
/*|		����d���������A���̌���Ԃ�(0�`)							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2013-11-28                                              |*/
/*[]------------------------------------- Copyright(C) 20013 AMANO Corp.---[]*/
int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl)
{
	struct t_IF_Queue *pSearch;
	int count = 0;
	int step = 0;

	if (pCtl->usCount != 0) {
		pSearch = pCtl->pTop;
		while (step < pCtl->usCount) {
			if(pSearch->pData->sW57.ucLock == 0) {// �t���b�v���u
				if(pSearch->pData->sW57.unCommand.Byte == (uchar)'4'){ // �t���b�v�㏸�̏ꍇ
					count++;	// ��v
				}
			}
			pSearch = pSearch->pNext;
			step++;
		}
	}
	return count;
}



