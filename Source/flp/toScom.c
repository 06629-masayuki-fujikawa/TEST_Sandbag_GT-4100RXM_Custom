/*[]----------------------------------------------------------------------[]*/
/*|		�Ύq�@�ʐM�^�X�N ����M�����i�VI/F�� �e�j						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"mnt_def.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"lkcom.h"
#include	"LKmain.h"

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toScom_Init(void);
void	toScom_TimValInit(void);
void	toScom_Main(void);
void	toScom_Polling(void);

void	toScom_Event1(void);
BOOL	toScom_CheckMatrix(uchar *pData, ushort uiLength);
void	toSmail_SendStatusToLock(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendVersionToLock(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendVersionToFLAP(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendTestResultCRR(uchar ucTerm, t_IF_Packet *pAns);

void	toScom_Event2(void);
void	toScom_Event3(void);
void	toScom_Event4(void);

void	toScom_ComPortReset(void);

uchar	toScom_SendI49(t_IF_I49_mail *pI49);
uchar	toScom_SendI49_CRR(t_Flap_I49_mail *pI49);
uchar	toScom_SendV56(int iSlave);
uchar	toScom_SendT54( uchar iSlave );
uchar	toScom_SendR52(int iSlave, uchar ucRequest);
uchar	toScom_SendW57(t_IF_W57_mail *pW57);
uchar	toScom_SendFlapW57(t_Flap_W57_mail *pFlapW57);
BOOL	toScom_WaitSciSendCmp(unsigned short Length);
void	toScom_gotoIDLE(void);
void	toScom_InitMatrix(void);

void	toScom_NextInitData(uchar ucSlave);	/* �����ݒ�f�[�^�D�摗�M */
void	toScom_InitFlapData();				/* Flap�����ݒ�f�[�^���M */
BOOL	toScom_NextRetry(void);				/* �D��đ� */

void	toScom_ChackAnswer(uchar ucSlave, uchar ucLastCmd, t_IF_Packet *psAnswer);
BOOL	toScom_IsSkip(uchar ucNowSlave);
BOOL	toScom_IsSkipRecover(uchar ucNowSlave);
BOOL	toScom_IsRetryOver(uchar ucNowSlave);
void	toScom_ResetRetry(uchar ucNowSlave);
void	toScom_ResetSkip(uchar ucNowSlave);
BOOL	toScom_Send_or_NextRetry(uchar ucNowSlave, uchar what_do);
void	toScom_SetNextPolling(int iPolling);
void	toSmail_SendStatusToFlap(uchar ucTerm, t_IF_Packet *pAns);
void	toSmail_SendLoopData(uchar ucTerm, t_IF_Packet *pAns);


/* �O���Q�Ɗ֐��i�f�o�b�O�p�r�Ȃ̂ŁA�����Ő錾�j*/
extern void	dbgprint_Queue(int line, char *file, t_IF_Queue_ctl *pQueue, char *pcQueName);	/* ���̂́AtoSmain.c �ɂ��� */


/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Init() 		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		toS communication parts initialize routine			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Init(void)
{
	toScom_TimValInit();				/* timer value & retry count set */

	toScom_2msTimerStop();				/* �����ԊĎ��p�i 2msec Timer�j*/
	toScom_LinkTimerStop();				/* ��M�Ď��p  �i10msec Timer�j*/
	toScom_LineTimerStop();				/* ����Ď��p  �i10msec Timer�j*/

	toScom_gotoIDLE();					/* goto IDLE (and timer stop) */

	toScom_SetNextPolling(0);			/* �|�[�����O�Ԋu�����߂� */
	toScom_toSlave.ucOrder  = (uchar)-1;	/* ��ԗv�����鑊��(�|�[�����O����)�q�@�́A(�C���f�b�N�X�����Ȃ̂�)�O����n�߂� */
										/* �|�[�����O���́{�P����邽�ߏ����l���|�P�ɂ���								*/
	toScom_ucAckSlave = TOSCOM_NO_ACK_TERMINAL;
	toScom_first = 1;	// ���������
	toScom_StartTime = LifeTimGet();
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_TimValInit() 		                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		timer value & retry count initialize.                          	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| 	This value will change when initial setting data receive.		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_TimValInit(void)
{
	/*
	*	�`�����x�ɂ���āA
	*	�����ԊĎ��^�C�}�l�����߂�B
	*	�E�P�`�����N�^�]�����Ԃ���Z�o����H
	*	�E�X�^�[�g�r�b�g1�{8�r�b�g���{�p���e�B�Ȃ��{�X�g�b�v�r�b�g1��10�r�b�g��1�L�����N�^�B
	*	�E 2400bps�F1�b�� 240�L�����N�^�A1�L�����N�^��4.1666666msec
	*	   9600bps�F1�b�� 960�L�����N�^�A1�L�����N�^��1.0416666msec
	*	  19200bps�F1�b��1920�L�����N�^�A1�L�����N�^��0.5208333msec
	*	  38400bps�F1�b��3840�L�����N�^�A1�L�����N�^��0.2604166msec
	*/
	switch(prm_get( COM_PRM,S_TYP,75,1,1 ))		// 03-0075�E �{�[���[�g
	{
	case 1:										// 19200bps
		toS_RcvCtrl.usBetweenChar = 2;			//�uNT-NET�ʐM�d�l���v��15/21���� t5��3msec(2ms�^�C�}�[���4ms)
		break;
	case 2:										// 9600bps
		toS_RcvCtrl.usBetweenChar = 3;			//�uNT-NET�ʐM�d�l���v��15/21���� t5��5msec(2ms�^�C�}�[���6ms)
		break;
	case 4:										// 2400bps
		toS_RcvCtrl.usBetweenChar = 10;			//�uNT-NET�ʐM�d�l���v��15/21���� t5��20msec(2ms�^�C�}�[)
		break;
	default:									// 38400bps
		// NOTE:���̃V���A���f�o�C�X���������ɓ��삵(I2C��SPI�Ȃ�)�A�ߏ�ɕ��ׂ������������Ƀ^�X�N���x����
		// �������ǂ����Ȃ��Ȃ�\�������邽�߁A�����ԃ^�C�}�[���킴�ƐL�΂�(�}�[�W��2�{)���Ԃ�����������
		toS_RcvCtrl.usBetweenChar = 2*2;		//�uNT-NET�ʐM�d�l���v��15/21���� t5��2msec(2ms�^�C�}�[)
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Main() 		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		toS function communication parts Main routine					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Main( void )
{
	/*
	*	toScomdr_f_RcvCmp�̒l�́AtoScomdr_Main(toScomdr.c)�ɂč����B
	*/
	/* �V���A����M�C�x���g�́H*/
	switch (toScomdr_f_RcvCmp) {
		case 1:							/* 1= �e�@����̓d����(�����܂�)��M */
			toScom_Event1();
			break;
		case 2:							/* 2= �V���A����M�G���[�����iSCI���x���j*/
			toScom_Event2();
			break;
		case 3:							/* 3= �e�q�ԃ����N(�`��)���x���Ń^�C���A�E�g�����i���g�p�j*/
			toScom_Event3();
			break;
		case 4:							/* 4= �e�̗v���ɑ΂��Ďq���������A�܂��̓|�[�����O�����o�� */
			toScom_Event4();
			break;

		default:
			break;
	}

	/* �V���A����M�C�x���g�����Z�b�g */
	toScomdr_f_RcvCmp = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event1() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		��M��������													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event1( void )
{
t_IF_Packet	*pPacket;
	ushort	dataSize;
	ushort	CRCsize;
	uchar	termNo;
	BOOL	bCRC;
	BOOL	bAnswer;
	uchar	nowTerm;
	uchar	nowCmd;
	nowTerm = 0;

	/*
	*	�^�X�N���x���̎�M�o�b�t�@���g�p����B
	*	�E���荞�݃n���h�����x���̎�M�o�b�t�@���h���C�o���x���̎�M�o�b�t�@�i���荞�݃n���h�����]���j
	*	�E�h���C�o���x���̎�M�o�b�t�@���^�X�N���x���̎�M�o�b�t�@�i�h���C�o���]���j
	*/

	/* �^�[�~�i��No.�܂ł���M���Ģ���Ȃ���Ȃ�A�����q�@�����ł��Ȃ� */
	bAnswer = toScom_CheckMatrix(toScom_RcvPkt, toScom_PktLength);
	if (!bAnswer) {
		IFM_SlaveError(E_IFS_PACKET_Headerless, E_IF_BOTH, (int)nowTerm);
		toS_ComErr.ulRcv_Size++;
		toScom_Matrix.State  = 0;		/* �����M�̂��߂ɁBS0�F��M�ҋ@�i�A�C�h���j�� */
		return;		/* ���� */
	}

	/*
	*	��M�����ł����ɗ���B
	*	�̂ŁA��M�d���̃`�F�b�N���e�͈ȉ��B�i���Ԃ��d�v�j
	*		�@�w�b�_����="IFCOM" �͐��������H
	*		�A���������������肩�H
	*		�B�f�[�^��(�f�[�^�T�C�Y)�͐��������H
	*		�CCRC�͈�v���邩�H
	*		�E�f�[�^��ʂ͐��������H
	*		�E���M�����R�}���h�̐������������H
	*/
	pPacket  = (t_IF_Packet *)&toScom_RcvPkt[0];					/* �d���`�F�b�N�̂��߃e���v���[�g���킹 */
	nowTerm  = toScom_toSlave.ucNow;								/* ���ʐM������擾���Ă��� */

	/* �@�w�b�_�����G���[ */
/* �q�@�u�w�b�_������蓦�����v�f�o�b�O�p */
	if (memcmp(pPacket->sHead.c_String, IF_HEADER_STR, IF_HEADER_LEN))
	{
		/* �w�b�_�����G���[[����] */
		IFM_SlaveError(E_IFS_PACKET_HeadStr, E_IF_BOTH, (int)nowTerm);
		return;		/* ���� */
	}

	/* �A���������������肩��ł͂Ȃ��B*/
	if( IFS_CRR_OFFSET > nowTerm ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
		termNo   = (uchar)astoinl(&pPacket->sHead.c_TerminalNo[0], 2);	// �^�[�~�i��No���擾
		if (termNo != (nowTerm + 1)) {
			/* ���肪�Ⴄ[����] */
			IFM_SlaveError(E_IFS_PACKET_WrongSlave, E_IF_BOTH, (int)nowTerm);
			return;		/* ���� */
		}
	} else {						// CRR��ւ̃A�N�Z�X
		if( 'V' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd 		// ���O���M�R�}���h��'V'�o�[�W�����m�F�̏ꍇ
			|| 'I' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd 		// ���O���M�R�}���h��'I'�������̏ꍇ
			|| 'T' == IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd ){	// ���O���M�R�}���h��'T'�������̏ꍇ
			if( 'F' != pPacket->sHead.c_TerminalNo[0]
				|| (nowTerm + 1) - IFS_CRR_OFFSET != astoinl(&pPacket->sHead.c_TerminalNo[1], 1) ){	// 'F1'�`'F3'������������
				/* ���肪�Ⴄ[����] */
				IFM_SlaveError(E_IFS_PACKET_WrongSlave, E_IF_BOTH, (int)nowTerm);
				return;		/* ���� */
			}
		}
	}

	/* �B�f�[�^���G���[ */
	dataSize = (ushort)c_arraytoint32(&pPacket->sHead.c_Size[0], 2);/* �f�[�^�T�C�Y���擾���Ă��� */
	if (toScom_PktLength != dataSize) {
		/* �f�[�^�T�C�Y�ُ�[����] */
		IFM_SlaveError(E_IFS_PACKET_DataSize, E_IF_BOTH, (int)nowTerm);
		return;		/* ���� */
	}

	/*
	*	�CCRC�G���[�`�F�b�N
	*	�E�d�����CRC�f�[�^�ʒu�́A	toScom_RcvPkt[dataSize-2]��
	*								toScom_RcvPkt[dataSize-1]
	*	�ECRC�Ώۂ́ACRC���g���܂܂��B
	*/
	CRCsize = dataSize - 2;
	bCRC     = CheckCRC(toScom_RcvPkt, CRCsize, &toScom_RcvPkt[CRCsize]);
	if( IFS_CRR_OFFSET > nowTerm ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
		nowCmd   = IFM_LockTable.sSlave[nowTerm].ucWhatLastCmd;		/* ���O���M�R�}���h���擾���Ă��� */
	} else {						// CRR��ւ̃A�N�Z�X
		nowCmd   = IFM_LockTable.sSlave_CRR[nowTerm - IFS_CRR_OFFSET].ucWhatLastCmd;	// CRR��ւ̃A�N�Z�X�̏ꍇ�AnowTerm �ɂ�+100�����l������Ă��邽�߁A-100���K�v
	}
	if (!bCRC) {
		/* CRC�G���[[����] */
		IFM_SlaveError(E_IFS_PACKET_CRC, E_IF_BOTH, (int)nowTerm);
		return;		/* ���� */
	}

	/*
	*	�����ɂ���΁A��M�d��OK
	*/
	/* ���̗v���ɑ΂��āA�����Ԃ��ꂽ���H�����ɂ߂� */
	toScom_ChackAnswer(nowTerm, nowCmd, pPacket);
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ChackAnswer() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�q�@�����`�F�b�N												   |*/
/*|		�E���̗v���ɑ΂��āA�����Ԃ��ꂽ���H�����ɂ߂�B				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ChackAnswer(uchar ucSlave, uchar ucLastCmd, t_IF_Packet *psAnswer)
{
	int		iAnswer;	/* �ǂ�ȉ������������H�敪 */
						/*		1�F���K�̉��� */
						/*		2�FNAK�ԐM */
						/*		3�F������ */
						/*		4�F�s���ȉ��� */
	BOOL	bRetry;
	char	cType;		// 0:���b�N, 1:�t���b�v
	iAnswer = 1;

	/*
	*	�ǂ�ȗv���ɑ΂��鉞�����H�ɂ��
	*	�E���҂��������Ȃ�A    ���M�����Ȃ̂ŕێ����Ă������M�f�[�^�͕s�v�ƂȂ�B
	*	�E���҂��O���������Ȃ�A���M�����Ȃ̂ŕێ����Ă������M�f�[�^�͕K�v�ƂȂ�B�E�E�E�đ��̂���
	*/
	if (psAnswer) {
		switch (ucLastCmd) {

			case 'I':	/* (49H)�u�����ݒ�f�[�^�v�𑗂��Ă����̂Ȃ�uACK�v�����҂��� */
			case 'W':	/* (57H)�u��ԏ����݃f�[�^�v�𑗂��Ă����̂Ȃ�uACK�v�����҂��� */
				if( IFS_CRR_OFFSET > ucSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
					switch (psAnswer->sHead.c_Kind[0]) {
						//���Ғʂ�
						case IF_ACK:	/* �m�艞�� */
							toScom_Send_or_NextRetry(ucSlave, 1);	/* ���M�f�[�^�͕s�v */
							iAnswer = 1;		/* ���K�̉��� */
							// ACK����M�����[��No.��ޔ�
							toScom_ucAckSlave = ucSlave;
							/*
							*	2005.05.20 �����E�ǉ�(��)
							*	�o�[�W�����v���E�����Ώ�
							*/
							if (ucLastCmd == 'I') {
								/* ���̂��Ɓu�o�[�W�����v���v���o�����Ƃ��ł���l�ɂ��� */
								IFM_LockTable.sSlave[ucSlave].bInitialACK = 1;	/* �u�����ݒ�f�[�^�v�𑗂���ACK�ԐM���ꂽ */
							}
							break;

						case IF_NAK:	/* �ے艞�� */
							bRetry = toScom_Send_or_NextRetry(ucSlave, 2);	/*�u(NAK�ԐM�̂���)�A���đ��v���� */
							if (bRetry) {
								iAnswer = 2;	/* NAK�đ� */
							} else {
								iAnswer = 3;	/* NAK���g���C�I�[�o�[ */
							}
							break;

						//���ҊO��
						case 'A':		/* �q�@��ԃf�[�^ */
						case 'V':		/* �q�@�o�[�W�����f�[�^ */
						default:		/* �񉞓� */
							toScom_Send_or_NextRetry(ucSlave, 3);	/*�u(�������̂���)����đ��v���� */
							iAnswer = 4;		/* �s���ȉ��� */
							break;
					}
				} else {						// CRR��ւ̃A�N�Z�X
					switch (psAnswer->sHead.c_Kind[0]) {
						case IF_ACK:	/* �m�艞�� */
						default:		/* �񉞓� */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (�D��)�L���[���O���B*/
							iAnswer = 1;		/* ���K�̉����Ŗ����Ă��A�|�[�����O�Ԋu��ς��Ȃ����߁A1��Ԃ� */
							break;
					}
				}
				break;

			/* (52H)�u��ԗv���f�[�^�v�𑗂��Ă����̂Ȃ�u�q�@��ԃf�[�^�v�����҂��� */
			case 'R':
				/*
				*	�u��ԗv���f�[�^�v�́A�u(�������̂���)����đ��v���Ȃ������Ȃ̂ŁA��O
				*/
				switch (psAnswer->sHead.c_Kind[0]) {
					//���Ғʂ�
					case 'A':		/* �q�@��ԃf�[�^ */
						cType = LKcom_GetAccessTarminalType((uchar)(ucSlave + 1));
						if(cType == LKCOM_TYPE_LOCK) {						// ���b�N���u
							if(psAnswer->unBody.sA41.c_Type[0] != (uchar)'0') {	// ���u�ƃf�[�^��ʂ���v���Ă��邩
								err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_TYPEDISCODE, (char)1, 0, 0 );	// ���u�E�f�[�^�s��v�G���[�o�^
								toScom_Send_or_NextRetry(ucSlave, 3);			/*�u(�������̂���)����đ��v���� */
								iAnswer = 5;		/* �������Ƃ��Č㏈������ */
								break;
							}
						}
						else if(cType == LKCOM_TYPE_FLAP){					// �t���b�v
							if(psAnswer->unBody.sFlapA41.c_Type[0] != (uchar)'F') {	// ���u�ƃf�[�^��ʂ���v���Ă��邩
								err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_TYPEDISCODE, (char)1, 0, 0 );	// ���u�E�f�[�^�s��v�G���[�o�^
								toScom_Send_or_NextRetry(ucSlave, 3);
								iAnswer = 5;		/* �������Ƃ��Č㏈������ */
								break;
							}
						}
						if (psAnswer->unBody.sA41.c_Request[0] == '0') {	/*'0':�e�@�ɑ΂���v���Ȃ��H*/
							if(cType == LKCOM_TYPE_LOCK) {					// ���b�N���u
								toSmail_SendStatusToLock(ucSlave, psAnswer);	/* �����b�N�Ǘ��^�X�N�֒m�点�� */
								if(0 == IFM_LockTable.sSlave[ucSlave].bInitialsndACK){	// �N�����ɏ����ݒ�f�[�^�𑗂��Ă��Ȃ�
									IFM_LockTable.sSlave[ucSlave].bInitialsndACK = 1;// �����ݒ�f�[�^�𑗐M����
									toScom_NextInitData(ucSlave);
								}
							}
							else if(cType == LKCOM_TYPE_FLAP) {				// �t���b�v
								toSmail_SendStatusToFlap(ucSlave, psAnswer);	/* �����b�N�Ǘ��^�X�N�֒m�点�� */
							}
						}
						else if (psAnswer->unBody.sA41.c_Request[0] == '1') {/*'1':�����ݒ�f�[�^�v���H*/
							if(cType == LKCOM_TYPE_LOCK) {					// ���b�N���u
								toSmail_SendStatusToLock(ucSlave, psAnswer);	/* �����b�N�Ǘ��^�X�N�֒m�点�� */
								IFM_LockTable.sSlave[ucSlave].bInitialsndACK = 1;// �����ݒ�f�[�^�𑗐M����
								toScom_NextInitData(ucSlave);					/* �����́u�����ݒ�f�[�^�v��D�悵�đ���i�����ݒ�f�[�^�𑗂��Ă��Ȃ��̂Łj*/
							}
						}													/*   �Ԃ��ꂽ�Z���T�[��Ԃ͖����Ȃ̂ŁA���b�N�Ǘ��^�X�N�֒m�点��K�v�Ȃ� */
						else {												/*'2':�Z���T�[���m�� */
							;												/* ���������Ȃ�		2005.08.26 �����E�ǉ� */
						}													/*   �Ԃ��ꂽ�Z���T�[��Ԃ͖��m��Ȃ̂ŁA���b�N�Ǘ��^�X�N�֒m�点��K�v�Ȃ� */
						toScom_Send_or_NextRetry(ucSlave, 1);				/* ���M�f�[�^�͕s�v */
						iAnswer = 1;		/* ���K�̉��� */
						break;

					case IF_NAK:	/* �ے艞�� */
						bRetry = toScom_Send_or_NextRetry(ucSlave, 2);				/*�u(NAK�ԐM�̂���)�A���đ��v���� */
						if (bRetry) {
							iAnswer = 2;	/* NAK�đ� */
						} else {
							iAnswer = 3;	/* NAK���g���C�I�[�o�[ */
						}
						break;

					case 'S':		// ���[�v�f�[�^����
						toSmail_SendLoopData(ucSlave, psAnswer);			/* �����b�N�Ǘ��^�X�N�֒m�点�� */
						toScom_Send_or_NextRetry(ucSlave, 1);				/* ���M�f�[�^�͕s�v */
						break;

					//���ҊO��
					case IF_ACK:	/* �m�艞�� */
					case 'V':		/* �q�@�o�[�W�����f�[�^ */
					default:		/* �񉞓� */
						toScom_Send_or_NextRetry(ucSlave, 3);				/*�u(�������̂���)����đ��v���� */
						iAnswer = 4;		/* �s���ȉ��� */
						break;
				}
				break;

			case 'V':	/* (56H)�u��ԗv���f�[�^�i�o�[�W�����v���j�v�𑗂��Ă����̂Ȃ�u�q�@�o�[�W�����f�[�^�v�����҂��� */
				if( IFS_CRR_OFFSET > ucSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
					switch (psAnswer->sHead.c_Kind[0]) {
						//���Ғʂ�
						case 'V':		/* �q�@�o�[�W�����f�[�^ */
							toSmail_SendVersionToLock(ucSlave, psAnswer);		/* ���b�N�Ǘ��^�X�N�֒m�点�� */
							toScom_Send_or_NextRetry(ucSlave, 1);				/* ���M�f�[�^�͕s�v */
							iAnswer = 1;		/* ���K�̉��� */
							break;

						case IF_NAK:	/* �ے艞�� */
							bRetry = toScom_Send_or_NextRetry(ucSlave, 2);	/*�u(NAK�ԐM�̂���)�A���đ��v���� */
							if (bRetry) {
								iAnswer = 2;	/* NAK�đ� */
							} else {
								iAnswer = 3;	/* NAK���g���C�I�[�o�[ */
							}
							break;

						//���ҊO��
						case IF_ACK:	/* �m�艞�� */
						case 'A':		/* �q�@��ԃf�[�^ */
						default:		/* �񉞓� */
							toScom_Send_or_NextRetry(ucSlave, 4);	/* �đ��͍s��Ȃ� */
							iAnswer = 4;		/* �s���ȉ��� */
							break;
					}
				} else {						// CRR��ւ̃A�N�Z�X
					switch (psAnswer->sHead.c_Kind[0]) {
						//���Ғʂ�
						case 'v':		/* CRR�o�[�W�����f�[�^ */
							toSmail_SendVersionToFLAP(ucSlave, psAnswer);		/* ���b�N�Ǘ��^�X�N�֒m�点�� */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (�D��)�L���[���O���B*/
							iAnswer = 1;		/* ���K�̉��� */
							break;
						default:		/* ���g���C�͂��Ȃ� */
							DeQueue(&toSque_Ctrl_Select, 0);			/* (�D��)�L���[���O���B*/
							iAnswer = 1;		/* ���K�̉����ł͂Ȃ����A�|�[�����O�Ԋu��ς��Ȃ����߁A1��Ԃ� */
							break;
					}
				}
				break;
			case 'T':	/* (54H)�uCRR�܂�Ԃ��e�X�g�v�𑗂��Ă����̂Ȃ�u�܂�Ԃ��e�X�g���ʁv�����҂��� */
				switch (psAnswer->sHead.c_Kind[0]) {
					//���Ғʂ�
					case 't':		/* CRR�܂�Ԃ��e�X�g���� */
						toSmail_SendTestResultCRR(ucSlave, psAnswer);		/* ���b�N�Ǘ��^�X�N�֒m�点�� */
						DeQueue(&toSque_Ctrl_Select, 0);			/* (�D��)�L���[���O���B*/
						iAnswer = 1;		/* ���K�̉��� */
						break;
					default:		/* ���g���C�͂��Ȃ� */
						DeQueue(&toSque_Ctrl_Select, 0);			/* (�D��)�L���[���O���B*/
						iAnswer = 1;		/* ���K�̉����ł͂Ȃ����A�|�[�����O�Ԋu��ς��Ȃ����߁A1��Ԃ� */
						break;
				}
				break;
			/* �����ɗ�����A�o�O�ł��B*/
			default:
				toScom_Send_or_NextRetry(ucSlave, 3);			/*�u(�������̂���)����đ��v���� */
				iAnswer = 4;				/* �s���ȉ��� */
				break;
		}
	}
	/* ���ҊO��E�E�E������ */
	else {
		if( IFS_CRR_OFFSET > ucSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
			if (ucLastCmd == 'V') {
				toScom_Send_or_NextRetry(ucSlave, 4);		/* �đ��͍s��Ȃ� */
			} else {
				toScom_Send_or_NextRetry(ucSlave, 3);		/*�u(�������̂���)����đ��v���� */
			}
			iAnswer = 5;
		} else {						// CRR��ւ̃A�N�Z�X�̏ꍇ�A�������ł��������Ȃ�
			DeQueue(&toSque_Ctrl_Select, 0);	/* (�D��)�L���[���O���B*/
			iAnswer = 1;						/* ���K�̉����ł͂Ȃ����A�|�[�����O�Ԋu��ς��Ȃ����߁A1��Ԃ� */
		}
	}

	/* �ǂ�ȉ����ŁA�ǂ��Ώ��������H*/
	switch (iAnswer) {
		case 1:		/* ���K�̉����Ȃ�A*/
			toScom_LineTimerStop();					/* ����Ď��^�C�}���� */
			toScom_toSlave.bWaitAnser = 0;			/* �����������Ȃ̂ŁA�����҂��N���A */
			toScom_SetNextPolling(iAnswer);			/* �|�[�����O�Ԋu�����߂� */
			break;

		case 2:		/* NAK�đ��Ȃ�A*/
			break;									/* ���ɍđ����Ă���̂ŁA�����ł͉������Ȃ� */

		case 3:		/* NAK���g���C�I�[�o�[ */
		case 4:		/* �s���ȉ��� */
		case 5:		/* ������ */
			toScom_LineTimerStop();					/* ����Ď��^�C�}���� */
			toScom_toSlave.bWaitAnser = 0;			/* �������������ᖳ�����ǁA�����҂��ł��Ȃ�����N���A */
			toScom_SetNextPolling(iAnswer);			/* �|�[�����O�Ԋu�����߂� */
			break;

		/* �����ɗ�����A�o�O�ł��B*/
		default:
			/* �v���O�����s���E�E�E�o�O�����H[����] */
			IFM_SlaveError(E_IFS_BUG, E_IF_BOTH, (int)ucSlave);
			break;
	}

	/* �G���[�����Â��� */
	switch (iAnswer) {
		case 1:		/* ���K�̉����E�E�E������(�X�L�b�v)[����]���� */
			IFM_SlaveError(E_IFS_NO_Answer, E_IF_RECOVER, (int)ucSlave);
			break;

		case 2:		/* NAK�đ� */
			break;

		case 3:		/* NAK���g���C�I�[�o�[�E�E�E[����] */
			IFM_SlaveError(E_IFS_NAK_Retry, E_IF_BOTH, (int)ucSlave);
			break;

		case 4:		/* �s���ȉ��� */
			switch (psAnswer->sHead.c_Kind[0]) {
				case 'A':
				case 'V':
				case IF_ACK:
				case IF_NAK:	/* ���҂����������Ԃ���Ȃ��E�E�E�v���g�R���ُ�[����] */
					IFM_SlaveError(E_IF_Protocol, E_IF_BOTH, (int)ucSlave);
					break;

				default:		/* �s��(�f�[�^���)�d��[����] */
					IFM_SlaveError(E_IFS_PACKET_WrongKind, E_IF_BOTH, (int)ucSlave);
					break;
			}
			break;

		case 5:		/* ������ */
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_IsSkip() 		                                   |*/
/*|             toScom_IsSkipRecover() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�X�L�b�v����													   |*/
/*|		�E�������X�L�b�v����											   |*/
/*|		�E�������X�L�b�v��������										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* �������X�L�b�v���� */
BOOL	toScom_IsSkip(uchar ucNowSlave)
{
	uchar	skipLimit = IFM_LockTable.toSlave.ucSkipNoAnswer;

	/* ������̓X�L�b�v�����Ȃ̂ŃJ�E���g�s�v */
	if (skipLimit >= 99) {
		return 0;
	}
	/* �L����̓X�L�b�v����̂ŃJ�E���g�v */
	else {
		/* �X�L�b�v�܂Łu�P�\�v����H*/
		if (IFM_LockTable.sSlave[ucNowSlave].ucSkipCount) {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount--;
			return 0;
		}
		/* �X�L�b�v���邱�ƂȂ��� */
		else {
			/* ������(�X�L�b�v)[����] */
			IFM_SlaveError(E_IFS_NO_Answer, E_IF_OCCUR, (int)ucNowSlave);
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = skipLimit;	/* �J�E���g�͎n�߂����蒼�� */
			IFM_LockTable.sSlave[ucNowSlave].bSkip = 1;					/* �X�L�b�v���� */
			IFM_LockTable.sSlave[ucNowSlave].bComm = 0;					/* �X�L�b�v����̂Łu�q�@�ւ̒ʐM�s�v�v�� */
			return 1;
		}
	}
}
/* �������X�L�b�v�������� */
BOOL	toScom_IsSkipRecover(uchar ucNowSlave)
{
	uchar	skipLimit = IFM_LockTable.toSlave.ucSkipNoAnswer;

	/* ������̓X�L�b�v�����Ȃ̂ŃJ�E���g�s�v */
	if (skipLimit >= 99) {
		return 0;
	}
	/* �L����̓X�L�b�v��������̂ŃJ�E���g�v */
	else {
		/* �X�L�b�v�����܂Łu�P�\�v����H*/
		if (IFM_LockTable.sSlave[ucNowSlave].ucSkipCount) {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount--;
			return 0;
		}
		/* �X�L�b�v�������邱�ƂȂ��� */
		else {
			IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = 1;	/* �J�E���g�͕����Ȃ̂Łu�P�v�� */
			IFM_LockTable.sSlave[ucNowSlave].bSkip = 0;			/* �X�L�b�v������ */
			IFM_LockTable.sSlave[ucNowSlave].bComm = 1;			/* �X�L�b�v��������̂Łu�q�@�ւ̒ʐM�K�v�v�� */
			return 1;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_IsRetryOver() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���g���C�I�[�o�[����											   |*/
/*|		�E���g���C�I�[�o�[�������ۂ��H��Ԃ��B							   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_IsRetryOver(uchar ucNowSlave)
{
	uchar	retryLimit = IFM_LockTable.toSlave.ucRetry;

	/* ������̓��g���C�����ςȂ��Ȃ̂ŃJ�E���g�s�v */
	if (retryLimit >= 99) {
		return 0;
	}
	/* �L����̓��g���C�I�[�o�[���������̂ŃJ�E���g�v */
	else {
		/* �đ��񐔓��Ȃ�A�J�E���g�_�E���̂� */
		if (IFM_LockTable.sSlave[ucNowSlave].ucNakRetry) {
			IFM_LockTable.sSlave[ucNowSlave].ucNakRetry--;
			return 0;
		}
		/* �đ��񐔃I�[�o�[�Ȃ�A*/
		else {							/* �J�E���g�͎n�߂����蒼�� */
			IFM_LockTable.sSlave[ucNowSlave].ucNakRetry = retryLimit;
			return 1;					/*�u���g���C�I�[�o�[�����v��Ԃ� */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ResetRetry() 	                                   |*/
/*|             toScom_ResetSkip() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�X�L�b�v����													   |*/
/*|		�ENAK���g���C����񐔂��Z�b�g�i�����l�ݒ�j����					   |*/
/*|		�E�������X�L�b�v����񐔂��Z�b�g�i�����l�ݒ�j����				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ResetRetry(uchar ucNowSlave)
{
	IFM_LockTable.sSlave[ucNowSlave].ucNakRetry = IFM_LockTable.toSlave.ucRetry;
}
void	toScom_ResetSkip(uchar ucNowSlave)
{
	IFM_LockTable.sSlave[ucNowSlave].ucSkipCount = IFM_LockTable.toSlave.ucSkipNoAnswer;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_CheckMatrix()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Check protocol from IFS received data	                     	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_CheckMatrix(uchar *pData, ushort uiLength)
{
	BOOL		answer;

	/* �����s�\�ŏ����� */
	answer  = 0;

	/* ��M�f�[�^���ɂ�锻�� */
	if (uiLength < 5) {
		toScom_Matrix.State  = 1;			/* S1�F�w�b�_�����҂� */
	}
	else if (uiLength < (5+2)) {
		toScom_Matrix.State  = 2;			/* S2�F�f�[�^�T�C�Y�҂� */
	}
	/* �f�[�^�T�C�Y�܂ł͎�M���� */
	else {
		if (uiLength < (5+2+1)) {
			toScom_Matrix.State  = 3;		/* S3�F�f�[�^��ʑ҂� */
		}
		else if (uiLength < (5+2+1+2)) {
			toScom_Matrix.State  = 4;		/* S4�F�^�[�~�i��No.�҂� */
		}
		/* �^�[�~�i��No.�܂ł͎�M���� */
		else {
			answer = 1;						/* �����\ */
			toScom_Matrix.State  = 0;		/* S0�F��M�ҋ@�i�A�C�h���j�� */
		}
	}

	return answer;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendStatusToLock()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���b�N�Ǘ��^�X�N�ւ̏�ԕω��ʒm								   |*/
/*|		�E���b�N�Ǘ��e�[�u����̃Z���T�[��Ԃƕω�������Ƃ��̂ݒʒm����B |*/
/*|		�E�ʒm�́A����܂ł̃��[�������߁A�L���[�����ɂ���B			   |*/
/*|		�E���[���s�̗p�̗��R�͈ȉ��B									   |*/
/*|			1)���b�N�Ǘ��^�X�N�́A										   |*/
/*|			  �ΐ��Z�@�ʐM�^�X�N����́u���b�N����f�[�^�v��			   |*/
/*|			  �{�^�X�N�փ��[���]�����邽�߁A							   |*/
/*|			  �������ɖ{�^�X�N���u�q�@��ԃf�[�^�v�����[������ƁA		   |*/
/*|			  ���̃n���h�����O�菇�ɂăf�b�h���b�N�������N�����\������B |*/
/*|			2)���b�N�Ǘ��^�X�N�́A�{�^�X�N����́u�q�@��ԃf�[�^�v��	   |*/
/*|			  �ΐ��Z�@�ʐM�^�X�N����́u���b�N����f�[�^�v��			   |*/
/*|			  �������Ɏ󂯎�����ꍇ�A�u�q�@��ԃf�[�^�v��D�揈�����ׂ��B |*/
/*|			  ���[���ʒm�ɂ����ꍇ���b�N�Ǘ��^�X�N�́A���̔��肪�ʓ|�B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	�����E�ύX�B��L�ύX�B                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendStatusToLock(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	int				ni;
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

#if 0
	BOOL		bSomeone;		/*�i��Ԏ��ł��j�ω�����t���O */
	/*
	*	��ԃf�[�^��ʒm���ׂ����H
	*	�E�ω���m��
	*
	*	2005.04.19 �����E�l�@
	*	�{���A
	*	�Ύq�@�ʐM�^�X�N���u��ԃf�[�^��ʒm���ׂ����ۂ��H�v�𔻒f����K�v�͖����B
	*	���f����̂́A���b�N�Ǘ��^�X�N�̖�ڂł���A�z���s�ׂł���B
	*	�������Ȃ���A
	*	�L���[�ɂ͌��肪����̂Łu�ʒm���ׂ���ԃf�[�^�v�݂̂��L���[�C���O���邱�Ƃ�
	*	�L���[���̐ߖ�ɂȂ�B
	*	S-RAM�ɗ]�T������΁A�w�{���̖������S�x�ɓO��
	*	S-RAM�ɗ]�T��������΁w�z���s�ׁx���~�ނ𓾂Ȃ��B
	*
	*	�������i�K�ŁA���̏����R���p�C���𐶂������H�E�����H�𔻒f���Ă��������B
	*/
	bSomeone = 0;
	for (ni=0; ni < 8; ni++) {
		if (IFM_LockTable.sSlave[ucTerm].sLock[ni].unSensor.Byte != pAns->unBody.sA41.unSensor[ni].Byte) {
			bSomeone = 1;	/* �ω�����Ȃ̂ŁA*/
			break;			/*�i��������ȏ�j���ׂ�K�v�Ȃ� */
		}
	}
	if (!bSomeone) {		/* �ω������Ȃ�A*/
		return;				/* �����^�[�� */
	}
#endif

	/*
	*	�����ɗ�����u�ω�����v�Ƃ�������
	*/
	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sA41.sCommon.ucKind     = 'A';
	pQbody->sA41.sCommon.ucTerminal = ucTerm;
	for (ni=0; ni < 8; ni++) {
		pQbody->sA41.unSensor[ni]   = pAns->unBody.sA41.unSensor[ni];
	}
	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 �����E�C��
	*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
	*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendVersionToLock()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���b�N�Ǘ��^�X�N�ւ̃o�[�W�����ʒm								   |*/
/*|		�E�q�@�o�[�W����������M�����Ƃ��Ăяo����A					   |*/
/*|		  ���b�N�Ǘ��^�X�N�֒ʒm����B									   |*/
/*|		�E�ʒm�́A����܂ł̃��[�������߁A�L���[�����ɂ���B			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	�����E�ύX�B��L�ύX�B                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendVersionToLock(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sV56.sCommon.ucKind     = 'V';
	pQbody->sV56.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sV56.ucVersion, pAns->unBody.sV56.c_Version, sizeof(pQbody->sV56.ucVersion));

	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 �����E�C��
	*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
	*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendVersionToFLAP()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�t���b�v�Ǘ��^�X�N�ւ̃o�[�W�����ʒm							   |*/
/*|		�E�q�@�o�[�W����������M�����Ƃ��Ăяo����A					   |*/
/*|		  �t���b�v�Ǘ��^�X�N�֒ʒm����B								   |*/
/*|		�E�ʒm�́A����܂ł̃��[�������߁A�L���[�����ɂ���B			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	�����E�ύX�B��L�ύX�B                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendVersionToFLAP(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sFlapv76.sCommon.ucKind     = 'v';
	pQbody->sFlapv76.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapv76.ucVersion, pAns->unBody.sFlapv76.c_Version, sizeof(pQbody->sFlapv76.ucVersion));

	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 �����E�C��
	*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
	*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendTestResultCRR()                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR��܂�Ԃ��e�X�g����										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa		                                           |*/
/*| Date        :  2013/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendTestResultCRR(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sFlapt74.sCommon.ucKind     = 't';
	pQbody->sFlapt74.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapt74.ucTestResult, pAns->unBody.sFlapt74.c_TestResult, sizeof(pQbody->sFlapt74.ucTestResult));

	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event2() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�V���A����M�G���[�����iSCI���x���j                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event2(void)
{
	toScom_gotoIDLE();				/* �A�C�h����Ԃ֖߂�(�^�C�}�[���~�߂�) */

/*
*	2005.04.28
*	�t���[�~���O�G���[�����߂Ĕ��������B
*	�̂ŁA���̃��J�o�[�B
*/
	toScom_SetNextPolling(0);		/* �|�[�����O���ĊJ���� */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event3() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�����N(�`��)���x���Ń^�C���A�E�g�����i���g�p�j					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event3(void)
{

	toScom_LinkTimerStop();			/* ���̃^�C�}���� */

	/* �ǂ̏�ԂŃ^�C���A�E�g�����������H*/
	switch (toScom_Matrix.State) {
		case 0:		/* S0�F�j���[�g���� */
			break;

		case 1:		/* S1�F�w�b�_�����҂� */
		case 2:		/* S2�F�f�[�^�T�C�Y�҂� */
		case 3:		/* S3�F�f�[�^��ʑ҂� */
			break;

		case 4:		/* S4�F�^�[�~�i��No.�҂� */
		case 5:		/* S5�F�f�[�^�҂� */
		case 6:		/* S6�FCRC�҂� */
			break;
	}
	toScom_InitMatrix();			/* �j���[�g������ */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Event4() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Communication Line time-out occurred   	                   		   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		������/�|�[�����O�����o��										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Event4(void)
{
	/* ����Ď��^�C�}���� */
	toScom_LineTimerStop();

	/* ����Ď��^�C���A�E�g���i�������j�H*/
	if (toScom_toSlave.bWaitAnser) {

		/*
		*	�q�@������܂�i���݂��Ȃ��H�j�Ȃ̂Łu�^�C���A�E�g�v�����B
		*	�E����đ��̑ΏۂƂȂ�B
		*	�E�����p�P�b�g���A���Ă��Ȃ��̂ŁA�����ɂ� NULL�|�C���^�w��
		*	  �Ăяo����ł́A������ NULL�|�C���^�Ȃ�u�������v�Ɣ��f���Ă����B
		*/
		if( IFS_CRR_OFFSET > toScom_toSlave.ucNow ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
			toScom_ChackAnswer(toScom_toSlave.ucNow, IFM_LockTable.sSlave[toScom_toSlave.ucNow].ucWhatLastCmd, (t_IF_Packet *)0);
		} else {					// CRR��ւ̃A�N�Z�X�̏ꍇ
			toScom_ChackAnswer(toScom_toSlave.ucNow, IFM_LockTable.sSlave_CRR[ toScom_toSlave.ucNow - IFS_CRR_OFFSET ].ucWhatLastCmd, (t_IF_Packet *)0);
		}
	}
	/* �|�[�����O�Ԋu�o�߂��H*/
	else {
		toScom_Polling();			/* �q�@�փ|�[�����O(��ԗv�����o��) */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Polling() 		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|											                       		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_Polling(void)
{
struct t_IF_Queue	*pSend;
	int		ni;
	uchar	next, now;
	BOOL	bContinuePolling;
	ushort	Init_timer;

	/*
	*	���ɁAtoS_main(toSmain.c) ��
	*	���[���{�b�N�X�����M���[�������o��
	*	�q�@�Z���N�e�B���O�E�L���[�֊i�[���Ă���B
	*	�E�����ݒ�f�[�^
	*	�E��ԏ����݃f�[�^
	*	�E��ԗv���f�[�^�i�o�[�W�����v���j�E�E�E2005.04.14 �����E�ǉ�
	*		1)���[����M�����m
	*		2)���[������Ȃ�L���[�ɂԂ�����
	*	�����ł́A���̃L���[������o���ēK�؂ȏ���������B
	*/

	/*
	*	�����Ȃ�Z���N�e�B���O�H
	*	�E���Ԃ����킷�D��I���M�R�}���h�����邩�H
	*	�E�L���[�ɗ��܂��Ă���Ȃ�u�D��I���M�R�}���h������I�v�Ƃ������ƁB
	*/
	pSend = NULL;
	if( toScom_first == 1 ){	// �N����1���
		Init_timer = TOSCOM_INIT_TIME_FIRST;	// 3�b�ő��M
	} else {
		Init_timer = TOSCOM_INIT_TIME;			// 60�b���ɑ��M
	}
	if (LifePastTimGet(toScom_StartTime) > Init_timer){
		toScom_first = 0;						// �������t���O�𗎂Ƃ�
		toScom_InitFlapData();					// CRR���(�t���b�v)�����ݒ著�M
		toScom_StartTime = LifeTimGet();
	}
	if (toScom_ucAckSlave == TOSCOM_NO_ACK_TERMINAL) {		// ACK��M�[���Ȃ���
		if(pSend = GetQueue(&toSque_Ctrl_Select)) {			// �L���[�Ƀf�[�^������Α��M

		/*
		*	�V�K���M�ŌĂяo�����߂̏���
		*	�E���݂̒ʐM����q�@�́A�L���[���̂̂���
		*	�E���O���M�R�}���h�́u�D��L���[�ɂ���v�i�Z���N�e�B���O�ŏ��̃P�[�X�j
		*	�E�V�K�Ȃ̂ŁA�đ��J�E���^�A�X�L�b�v�J�E���^�͏����l���Z�b�g
		*	�����́u�D��I���M�R�}���h�v�����M���s������A�iNAK�ԐM����������������j
		*	  �L���[����O���ꏇ�ԃ{�[�h�ɂȂ����̂ŁA����͂��̃L���[�ɂ͖����B
		*/
		now = pSend->pData->sCommon.ucTerminal;				/* �L���[���̂̑��M��q�@���擾 */
		toScom_toSlave.ucNow = now;							/* ���݂̒ʐM����ɂ��� */
		if( IFS_CRR_OFFSET > now ){													/* CRR��ւ̃A�N�Z�X�ł͂Ȃ�	*/
			IFM_LockTable.sSlave[now].ucWhereLastCmd = 1;		/*�u�D��L���[�ɂ���v*/
		} else {
			IFM_LockTable.sSlave_CRR[now - IFS_CRR_OFFSET].ucWhereLastCmd = 1;		/*�u�D��L���[�ɂ���v*/
		}
		toScom_Send_or_NextRetry(now, 0);					/* �V�K���M */
		toScom_ResetRetry(now);								/* �đ��J�E���^������ */
		toScom_ResetSkip(now);								/* �X�L�b�v�J�E���^������ */
		}
	}
	/*
	*	�|�[�����O
	*	�E�L���[�ɗ��܂��Ă��Ȃ�����A���ԂɁu��ԗv��/�đ��R�}���h�v���o���B
	*	�E���ԃ{�[�h�ɖ����E�E�E��ԗv���f�[�^
	*	  ���ԃ{�[�h�ɂ���E�E�E�O�񖳉��������ƂȂ����A�ȉ��̉��ꂩ
	*		(49H�F'I')�����ݒ�f�[�^
	*		(57H�F'W')��ԏ�������
	*		(52H�F'R')��ԗv���i�o�[�W�����v���j
	*/
	if(pSend == NULL) {										// ACK��M�[������or���M�f�[�^����
		if(toScom_ucAckSlave == TOSCOM_NO_ACK_TERMINAL) {
			next = (uchar)(toScom_toSlave.ucOrder + 1);		/* ���̏��Ԃ̒ʐM����������߂��� */
		}
		else {
			// W�R�}���h���M��̓��쒆��Ԃ��擾����ׂɁAACK����M�����[���ɑ΂��ă|�[�����O���s��
			next = toScom_ucAckSlave;
			toScom_ucAckSlave = TOSCOM_NO_ACK_TERMINAL;		// ACK�҂��[�������Ƃ���
		}
		for (ni=0; ni < IF_SLAVE_MAX; ni++, next++) {		/* �q�̑䐔�� */
			if (next >= IF_SLAVE_MAX) {						/* �����ő吔�ȏ�Ȃ�A*/
				next = 0;									/* ���[�e�[�V���� */
			}
			/* �ʐM�\�Ȏq�@�Ɍ���i�X�L�b�v���́uFALSE�v�Ȃ̂őΏۊO�j*/
			if (IFM_LockTable.sSlave[next].bComm) {			/* ���̎q�@�֒ʐM����K�v���邩�H*/
				toScom_toSlave.ucOrder = (uchar)next;		/* ���̏��Ԃ̒ʐM������m�� */

				/*
				*	�V�K���M�ŌĂяo�����߂̏���
				*	�E���݂̒ʐM����q�@�́A��Ō��߂�����
				*	�E���O���M�R�}���h�́u���܁A�����ɂ��邩�H�v����
				*		���ԃ{�[�h��ɁA���ɂ���E�E�E�i�Z���N�e�B���O�đ��̃P�[�X�j
				*		���ԃ{�[�h��ɁA�����E�E�E�E�E�i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j
				*/
				toScom_toSlave.ucNow = next;			/* ��Ō��߂����Ԃ̑���ɂ��� */
				if (IFM_LockTable.sSlave[next].psNext) {
					IFM_LockTable.sSlave[next].ucWhereLastCmd = 2;	/*�u���ԃ{�[�h�ɂ���v*/
				} else {
					IFM_LockTable.sSlave[next].ucWhereLastCmd = 0;	/*�u�����ɂ��Ȃ��v*/
				}
				toScom_Send_or_NextRetry(next, 0);			/* �V�K���M/�đ� */
				break;										/* ���̈�񂾂��|�[���O */
		 	}
		 	/* �ʐM�s�\�Ȏq�@�ł��E�E�E */
		 	else if (IFM_LockTable.sSlave[next].bSkip) {	/* �X�L�b�v���Ȃ�A*/
				toScom_IsSkipRecover(next);					/* �����̃`�����X���� */
			}
		}

		/* �S�Ă̎q�@���A�ʐM�s�\�ɂȂ������̂��߂ɁE�E�E*/
		bContinuePolling = toScom_Is_LineTimerStop();		/* �^�C�}�������ĂȂ��Ȃ�A*/
		if (bContinuePolling) {								/* ��x�Ɩ{�֐��ɗ��Ȃ��Ȃ邱�Ƃ�����邽�� */
			toScom_SetNextPolling(5);						/* �����������Ń|�[�����O�Ԋu�����߂� */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_ComPortReset() 	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication device reset. and other process. (*1 proc)		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_ComPortReset(void)
{
	/* SCI�|�[�g���~(���荞�݂��֎~�ɂ���) */
	toSsci_Stop();

	/* ��M�h���C�o�������� */
	toScomdr_Init();

	/*
	*	SCI�|�[�g��������
	*	�E������������
	*	�E2400/$9600/19200/38400bps
	*	�E8�r�b�g�f�[�^��
	*	�E�X�g�b�v�r�b�g=1
	*	�E�p���e�B�Ȃ�
	*/
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendI49()                                           |*/
/*|             toScom_SendR52()                                           |*/
/*|             toScom_SendW57()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		send polling/selecting code to TF								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	input  : terminal number to be polling/selecting.					   |*/
/*|	return : 1=OK,  0=Couldn't send 									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_SendI49(t_IF_I49_mail *pI49)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	ushort		ni;
	BOOL		bResult;
	int			iSlave;

	/* �w�b�_ */
	iSlave   = (int)pI49->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_I49);	/* �����ݒ�f�[�^�����擾���Ă��� */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);					/* �f�[�^�T�C�Y */
			    pPacket->sHead.c_Kind[0]     = pI49->sCommon.ucKind;					/* �f�[�^��ʁi'I'�F49H�j*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);				/* ����^�[�~�i��No. */
	/* �{�f�B */
	for (ni=0; ni < 8; ni++) {									/* ���b�N���u�̐ڑ��i��ԁj*/
		intoas(&pPacket->unBody.sI49.c_Connect[ni], (ushort)pI49->ucConnect[ni], 1);
	}															/* ���o�Ɏԗ����m�^�C�}�[�E�E�Ex150msec�͂ǂ�����H*/
	intoas(&pPacket->unBody.sI49.c_EntryTimer[0],   (ushort)pI49->usEntryTimer, 2);
	intoas(&pPacket->unBody.sI49.c_ExitTimer[0],    (ushort)pI49->usExitTimer,  2);
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sI49.ucCRC16, CRC16_R_SHIFT);

	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* ���݂̒ʐM����q�@ */
	IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h�������Ă��� */
	toScom_toSlave.bWaitAnser = 1;								/* �����҂����� */
	toScom_toSlave.ucActionLock = 0xff;							/* ���b�N���uNo.�́A����ΏۊO */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* ���M������҂� */

//�f�o�b�O
toScom_toSlave.dbg.LastCmd = pPacket->sHead.c_Kind[0];
toScom_toSlave.dbg.sndI49++;

	return	bResult;
}

BOOL	toScom_SendR52(int iSlave, uchar ucRequest)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* �w�b�_ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_R52);	/* ��ԗv���f�[�^�����擾���Ă��� */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* �f�[�^�T�C�Y */
			    pPacket->sHead.c_Kind[0]     = 'R';								/* �f�[�^��ʁi'R'�F52H�j*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);		/* ����^�[�~�i��No. */
	/* �{�f�B */
	intoas(&pPacket->unBody.sR52.c_Request[0],(ushort)ucRequest, 1);			/* �v�����e�i'0'�F�q�@��ԃf�[�^�v��/'1'�F�o�[�W�����v���j*/
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sR52.ucCRC16, CRC16_R_SHIFT);

	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* ���݂̒ʐM����q�@ */
	/*�u�q�@��ԃf�[�^�v���v�H*/
	if (ucRequest == 0 || ucRequest == 2) {
		IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h��('R'�̂܂�)�����Ă��� */
	}
	/* �u�o�[�W�����v���v*/
	else {
		IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = 'V';		/* ���̎q�@�ւ̑��M�R�}���h��'V'�ɏ��������i�����H�j*/
	}
	toScom_toSlave.bWaitAnser = 1;								/* �����҂����� */
	toScom_toSlave.ucActionLock = 0xff;							/* ���b�N���uNo.�́A����ΏۊO */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* ���M������҂� */

//�f�o�b�O
toScom_toSlave.dbg.LastCmd = pPacket->sHead.c_Kind[0];
toScom_toSlave.dbg.sndR52++;

	return	bResult;
}

BOOL	toScom_SendW57(t_IF_W57_mail *pW57)
/*�uucAction�v�͎g��Ȃ��B��~/����ł͖����u(0)���슮��/(1)���쒆��\����ԁi�������ݕs�j�v�ƂȂ��� */
/*�uucMaker�v��ǉ������B���b�N���u���[�J�[ID�i(49H�F'I')�����ݒ�f�[�^�́u���b�N���u�̐ڑ��v���l�j	2005.03.30 �����E�ǉ� */
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;
	int			iSlave;

	/* �w�b�_ */
	iSlave   = (int)pW57->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];											/* ���M�o�b�t�@�̐擪���|�C���g */
	dataSize = sizeof(t_IF_Header) + sizeof(t_IF_Body_W57);							/* ��ԏ����݃f�[�^�����擾���Ă��� */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);					/* �f�[�^�T�C�Y */
			    pPacket->sHead.c_Kind[0]     = pW57->sCommon.ucKind;					/* �f�[�^��ʁi'W'�F57H�j*/
	intoas(  &pPacket->sHead.c_TerminalNo[0],  (ushort)(iSlave + 1), 2);				/* ����^�[�~�i��No. */
	/* �{�f�B */
	if( LKcom_Search_Ifno( (uchar)(iSlave + 1) ) == 0) {
	intoas(  &pPacket->unBody.sW57.c_LockOnTime[0], (ushort)pW57->usLockOnTime,  2);	/* ���b�N���u���̐M���o�͎��� */
	intoas(  &pPacket->unBody.sW57.c_LockOffTime[0],(ushort)pW57->usLockOffTime, 2);	/* ���b�N���u�J���̐M���o�͎��� */
	intoas(  &pPacket->unBody.sW57.c_RoomNo[0],     (ushort)(pW57->ucLock + 1),  1);	/* ���b�N���uNo. */
	intoas(  &pPacket->unBody.sW57.c_MakerID[0],    (ushort)pW57->ucMakerID, 1);		/* ���b�N���u���[�J�[ID		2005.03.30 �����E�ǉ� */
				pPacket->unBody.sW57.unCommand =              pW57->unCommand;			/* �������݃Z���T�[��Ԃ𒼑�� */
																						/*	bit4�F(0)���b�N����/(1)���b�N�������� */
																						/*	bit7�F(0)�����e�i���X����/(1)�����e�i���X�� */
	}
	else {		// �t���b�v
		intoas(  &pPacket->unBody.sW57.c_LockOnTime[0], 0,  2);	/* ���b�N���u���̐M���o�͎��� */
		intoas(  &pPacket->unBody.sW57.c_LockOffTime[0], 0, 2);	/* ���b�N���u�J���̐M���o�͎��� */
		pPacket->unBody.sW57.c_RoomNo[0] ='0';					/* ���b�N���uNo. */
		pPacket->unBody.sW57.c_MakerID[0] = '0';				/* ���b�N���u���[�J�[ID		2005.03.30 �����E�ǉ� */
		pPacket->unBody.sW57.unCommand = pW57->unCommand;		/* �������݃Z���T�[��Ԃ𒼑�� */
	}
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sW57.ucCRC16, CRC16_R_SHIFT);

	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = (uchar)iSlave;					/* ���݂̒ʐM����q�@ */
	IFM_LockTable.sSlave[iSlave].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h�������Ă��� */
	toScom_toSlave.bWaitAnser = 1;								/* �����҂����� */
	toScom_toSlave.ucActionLock = (uchar)pW57->ucLock;			/* ���M����u���b�N���uNo.�v��ێ����Ă��� */
	bResult = toScom_WaitSciSendCmp(dataSize);						/* ���M������҂� */
	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendI49_CRR()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR���(FLAP)�����ݒ�											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendI49_CRR(t_Flap_I49_mail *pI49)
{
	t_IF_Packet	*pPacket;
	ushort		dataSize;
	ushort		ni;
	BOOL		bResult;
	int			iSlave;

	/* �w�b�_ */
	iSlave   = (int)pI49->sCommon.ucTerminal;
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_I49);				/* �����ݒ�f�[�^�����擾���Ă��� */
	memcpy( &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);		/* �f�[�^�T�C�Y */
	pPacket->sHead.c_Kind[0] = pI49->sCommon.ucKind;						/* �f�[�^��ʁi'I'�F49H�j*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){										/* ����CRR���No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* �{�f�B */
	for (ni=0; ni < IFS_CRRFLAP_MAX; ni++) {									/* ���b�N���u�̐ڑ��i��ԁj*/
		intoas(&pPacket->unBody.sFlapI49.ucConnect[ni][0], (ushort)pI49->ucConnect[ni], 2);
	}
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapI49.ucCRC16, CRC16_R_SHIFT);

	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = (uchar)iSlave;									/* ���݂̒ʐM����q�@ +100�����܂܂̒l�ɂ��Ă���*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h�������Ă��� */
	toScom_toSlave.bWaitAnser = 1;											/* �����҂����� */
	toScom_toSlave.ucActionLock = 0xff;										/* ���b�N���uNo.�́A����ΏۊO */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* ���M������҂� */

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendV56()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR���(FLAP)�o�[�W�����m�F										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendV56(int iSlave)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* �w�b�_ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_V56);					/* �����ݒ�f�[�^�����擾���Ă��� */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* �f�[�^�T�C�Y */
			    pPacket->sHead.c_Kind[0]     = 'V';								/* �f�[�^��ʁi'V'�F56H�j*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){											/* ����CRR���No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* �{�f�B */
	memset( pPacket->unBody.sFlapV56.dummy, '0' ,8);
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapV56.ucCRC16, CRC16_R_SHIFT);

	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = (uchar)iSlave;									/* ���݂̒ʐM����q�@ +100�����܂܂̒l�ɂ��Ă���*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h�������Ă��� */
	toScom_toSlave.bWaitAnser = 1;											/* �����҂����� */
	toScom_toSlave.ucActionLock = 0xff;										/* ���b�N���uNo.�́A����ΏۊO */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* ���M������҂� */

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SendT54()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR��܂�Ԃ��e�X�g											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toScom_SendT54(uchar iSlave)
{
t_IF_Packet	*pPacket;
	ushort		dataSize;
	BOOL		bResult;

	/* �w�b�_ */
	pPacket  = (t_IF_Packet *)&toS_SndBuf[0];
	dataSize = sizeof(t_IF_Header) + sizeof(t_FLAP_Body_T54);					/* �����ݒ�f�[�^�����擾���Ă��� */
	memcpy(	   &pPacket->sHead.c_String[0],    IF_HEADER_STR, IF_HEADER_LEN);	/* �w�b�_������("IFCOM"�Œ�) */
	c_int32toarray(&pPacket->sHead.c_Size[0],      (ulong)dataSize, 2);			/* �f�[�^�T�C�Y */
			    pPacket->sHead.c_Kind[0]     = 'T';								/* �f�[�^��ʁi'T'�F54H�j*/
	pPacket->sHead.c_TerminalNo[0] = 'F';
	switch( iSlave - IFS_CRR_OFFSET ){											/* ����CRR���No. */
	case 0:
		pPacket->sHead.c_TerminalNo[1] = '1';
		break;
	case 1:
		pPacket->sHead.c_TerminalNo[1] = '2';
		break;
	case 2:
		pPacket->sHead.c_TerminalNo[1] = '3';
		break;
	default:	break;
	}
	/* �{�f�B */
	memset( pPacket->unBody.sFlapT54.dummy, '0' ,8);
	/* CRC */
	/* �ΏۃT�C�Y�́ACRC���g���܂܂��BCRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	crc_ccitt((ushort)(dataSize - 2), toS_SndBuf, pPacket->unBody.sFlapT54.ucCRC16, CRC16_R_SHIFT);
	
	/* �q�@�֑��M */
	toScom_toSlave.ucNow  = iSlave;											/* ���݂̒ʐM����q�@ +100�����܂܂̒l�ɂ��Ă���*/
	IFM_LockTable.sSlave_CRR[iSlave- IFS_CRR_OFFSET].ucWhatLastCmd = pPacket->sHead.c_Kind[0];	/* ���̎q�@�ւ̑��M�R�}���h�������Ă��� */
	toScom_toSlave.bWaitAnser = 1;											/* �����҂����� */
	toScom_toSlave.ucActionLock = 0xff;										/* ���b�N���uNo.�́A����ΏۊO */
	bResult = toScom_WaitSciSendCmp(dataSize);								/* ���M������҂� */

	toScom_LineTimerStop();	// �|�[�����O�^�C�}�[���~����

	return	bResult;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_WaitSciSendCmp()    	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		SCI���M�i���M�����҂��j											   |*/
/*|		�E�w�肳�ꂽ���M�o�C�g�����A									   |*/
/*|		�E���M�o�b�t�@�擪����SCI�o�͂���B								   |*/
/*|		�E���M�����܂ő҂��܂��B										   |*/
/*|		�����M�����҂��^�C���A�E�g�́u�H�vsec�ł��B						   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	input  : ���M�o�C�g��												   |*/
/*|	return : ���M����													   |*/
/*|				1 = ����I���i���M�����j								   |*/
/*|				0 = �ُ�I���i���M�����B�H�b�҂��Ă����M�����ɂȂ�Ȃ��j   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2004-01-27                                              |*/
/*| Update      :  2005-01-18	�����E����                                 |*/
/*| 				�E���M�O��Wait�͕s�v			                       |*/
/*| 				�ERS485�ʐM�Ȃ̂ő��M�h���C�o(��ݽЯ�)����/�֎~��Ώ�  |*/
/*| Update      :  2005-06-13	�����E����                                 |*/
/*| 				�E���M�O��Wait�𕜊�			                       |*/
/*| 				�ERS485�ʐM�̑��M�h���C�o(��ݽЯ�)����/�֎~��          |*/
/*| 				  ���荞�݃n���h���ֈړ�                               |*/
/*| 				�E�֐��̖߂�l��Boolean�^�ɕύX                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_WaitSciSendCmp(unsigned short Length)
{
	ulong	StartTime;

	/* ���M�O�̑҂��E�E�E�q�@�ɑ΂��Ă��u�҂��v�K�v */
	if (toScom_Timer.usSendWait != 0) {
		xPause2ms((ulong)toScom_Timer.usSendWait);
	}

	/* ���M�v����NG ? */
	if (!toSsci_SndReq(Length)) {
		return	(BOOL)0;					/* ���M�G���[�Ń��^�[�� */
	}

	/* ���M������҂� */
	StartTime = LifeTim2msGet();
	// ���Z�������ɉ����Đ���FROM�����݂̂��߃^�C���A�E�g���Ă��܂����ߎ��Ԃ𒷂����
	while (0 == LifePastTim2msGet(StartTime, 500L)) {		/* 1sec�o�߂������H*/
		taskchg( IDLETSKNO );				// IDLE�^�X�N�֐؂�ւ� 
		if (1 == toSsci_IsSndCmp()) {		/* ���M�������H*/

			/* �������牞���Ď��X�^�[�g */
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);

			return	(BOOL)1;				/* ���M����I���Ń��^�[�� */
		}
	}
	/* �������牞���Ď��X�^�[�g */
	// �����ŊĎ����X�^�[�g���Ȃ��ƃ|�[�����O����~����
	toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);
	/* �����ɗ���̂́A���ݎ����{100msec(1sec�H)�o�߂��Ă����M�������Ȃ������Ƃ��B*/
	return	(BOOL)0;						/* ���M�G���[�Ń��^�[�� */
	/* This case will be only flow control useing. 	*/
	/* in this time, flow control doesn't use.		*/
	/* Therefore I must return OK status.			*/
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_gotoIDLE() 		    	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		phase change to IDLE                     						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba                                                 |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_gotoIDLE(void)
{
	toScom_2msTimerStop();			/* �����ԊĎ��^�C�}���� */
	toScom_LinkTimerStop();			/* ��M�Ď��^�C�}���� */
	toScom_LineTimerStop();			/* ����Ď��^�C�}���� */

	toScom_InitMatrix();			/* toS�[���ԃ����N���x�����A�C�h���֏����� */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_InitMatrix()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		initialize toS protocol information data	                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_InitMatrix(void)
{
	toScom_Matrix.State = 0;		/* �j���[�g����(S0)�� */

	toScom_LinkTimerStop();			/* ��M�Ď��^�C�}���� */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_NextInitData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�����ݒ�f�[�^�D�摗�M						                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_NextInitData(uchar ucSlave)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	int				ni;
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	if(!( GetCarInfoParam() & 0x01 )){// ���֐ݒ�Ȃ�
		// �ʐMI/F�����b�N��/�t���b�v�����ɋ��ʂł��邪���b�N���̂ݏ����ݒ�f�[�^���K�v�ƂȂ邽��
		return;
	}
	//NOTE:�����ݒ�f�[�^���g��PAYcom_InitDataSave()�֐��ŃZ�b�g�����

	/*
	*	���̎q�@�ցu�����ݒ�f�[�^�v�𑗂�B
	*	�E���̃Z���N�e�B���O�̂��߂ɁA�L���[�C���O����B
	*	�E���܂܂ł́A���̎q�@�̏��̓`�����ɂ���B
	*	  ���j�u�����v�`�����ɂ��邩�H
	*		1)�Z���T�[���
	*		2)�X�L�b�v���t���O
	*		3)�o�[�W����
	*		4)���́D�D�D�ȂɁH
	*/
	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* ���̂������N */
		bQfull = 0;									/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}
	/* ���b�N���u�Ǘ��e�[�u�����炻�ꂼ��̎q�@�p�ɕϊ� */
	pQbody->sCommon.ucKind     = 'I';											/* �����ݒ�f�[�^ */
	pQbody->sCommon.ucTerminal = ucSlave;										/* �^�[�~�i��No.�i�ǂ̎q�@�ւ��H����肷��j*/
	for (ni=0; ni < 8; ni++) {
		pQbody->sI49.ucConnect[ni] = IFM_LockTable.sSlave[ucSlave].sLock[ni].ucConnect;	/* ���b�N���u�̐ڑ� */
	}
	pQbody->sI49.usEntryTimer  = IFM_LockTable.sSlave[ucSlave].usEntryTimer;	/* ���Ɏԗ����m�^�C�}�[ */
	pQbody->sI49.usExitTimer   = IFM_LockTable.sSlave[ucSlave].usExitTimer;		/* �o�Ɏԗ����m�^�C�}�[ */
	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));

	/*
	*	2005.07.12 �����E�C��
	*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
	*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
	*/
	if (bQfull) {
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);				/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����] */
	} else {
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);			/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����]���� */
	}

	/* ���܂܂ł́A���̎q�@�̏��̓`�����ɂ��� */
	for (ni=0; ni < 8; ni++) {
		IFM_LockTable.sSlave[ucSlave].sLock[ni].unSensor.Byte = 0;			/* �Z���T�[�����N���A */
	}
	IFM_LockTable.sSlave[ucSlave].bSkip = 0;								/* �X�L�b�v�������� */
	memset(IFM_LockTable.sSlave[ucSlave].cVersion, 0, 8);					/* �o�[�W�������N���A */
}
/*[]----------------------------------------------------------------------[]*/
/*|             toScom_InitFlapData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		Flap�����ݒ�f�[�^���M						                   	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2012/12/6                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_InitFlapData()
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	int				i,ni;
	BOOL			bQfull;	

	/* ���b�N���u�Ǘ��e�[�u�����炻�ꂼ��̎q�@�p�ɕϊ� */
	for( i = 0 ; i < IFS_CRR_MAX ; i++ ){
		//	�ʐM����K�v���Ȃ�CRR���(�^�[�~�i��No.���U���Ă��Ȃ��j�͑��M���Ȃ�
		if (!IFM_LockTable.sSlave_CRR[i].bComm ){
			continue;
		}
		/* �󂫃L���[�݂邩�H*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* ���̂������N */
			bQfull = 0;									/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����]���� */
		}
		/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
		else {
			bQfull = 1;									/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����] */
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
			pQbody = pQbuff->pData;
		}

		pQbody->sCommon.ucKind     = 'I';											/* �����ݒ�f�[�^ */
		pQbody->sCommon.ucTerminal = i + IFS_CRR_OFFSET;							/* CRR���No. +100���邱�ƂŃ^�[�~�i��No.�Ƃ̍��ʉ�������*/
		for (ni=0; ni < IFS_CRRFLAP_MAX; ni++) {
			pQbody->sFlapI49.ucConnect[ni] = IFM_LockTable.ucConnect_Tbl[i][ni];	// �^�[�~�i���ԍ��̎w��
		}
		/* �����փL���[�C���O */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
		if (bQfull) {
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);				/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����] */
		} else {
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);			/* �L���[�t���i�Z���N�e�B���O�҂����X�g�j[����]���� */
		}
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_Send_or_NextRetry()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�V�K���M/�A���đ�/����đ�										   |*/
/*|		�E�q�@�ւ̓d�����M�ɂ����āA�V�K���M/�A���đ�/����đ���		   |*/
/*|		  ���ꂼ��̃P�[�X�ʂɉ��������M�v���������s����B				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	toScom_Send_or_NextRetry(uchar ucNowSlave, uchar what_do)
{
struct t_IF_Queue	*pWhat;		/* ���ׂ�L���[ */
t_IFM_Queue			*pMail;		/* ���M/�ړ�����o�P�b�g */
	uchar			where;
	uchar			nowCmd;
	BOOL			bRetryOver;
	BOOL			bSkip;
	uchar			ucRequest;
	BOOL			bRet = 0;

	/*
	*	�q�@����ACK���Ԃ��ꂽ�Ƃ��̂݁A�L���[���O���̂������B
	*	NAK���Ԃ��ꂽ��đ����ׂ�������D�D�D�B
	*
	*	2005.04.04
	*	�đ��́A�A���ł͂Ȃ�����̃Z���N�e�B���O�ɉ􂳂��̂�
	*	��U�O���čēx�����ɂȂ��������ƂŁA�Ώ��B
	*
	*	2005.04.11
	*	NAK�đ��́A�A���ɕύX���ꂽ�B
	*	����āA���g���C�񐔓��Ȃ瑦�đ�����B
	*	�Ȃ̂ŁA�L���[/���ԃ{�[�h�Ɏc�����܂܁B
	*	�������́A����|�[�����O���Ԃ̂Ƃ��đ�����B
	*	�Ȃ̂ŁA���ԃ{�[�h�ɂȂ�������B
	*/
	if( IFS_CRR_OFFSET > ucNowSlave ){							/* CRR��ւ̃A�N�Z�X�ł͂Ȃ�	*/
		where = IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd;
	} else {
		where = IFM_LockTable.sSlave_CRR[ucNowSlave - IFS_CRR_OFFSET].ucWhereLastCmd;
	}
	/* ������邩�H�́A�ʐM���ʎ��� */
	switch (what_do) {

		/*
		*	���M�̂݁i���ʂȂ��j
		*	�E���M�R�}���h�����߂�
		*	�E���M����B
		*/
		case 0:
			/* �����ɂ��邩�H�Ŏ���Ă���ʒu���قȂ� */
			switch (where) {
				case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
					nowCmd = 'R';	/*�u��ԗv���f�[�^�v*/
					break;
				case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
					pWhat = GetQueue(&toSque_Ctrl_Select);
					nowCmd = pWhat->pData->sCommon.ucKind;
					pMail = (t_IFM_Queue *)pWhat->pData;

					/* ���ɏ��ԑ҂������遨��������������܂ŐV���ȃR�}���h�͏o���Ȃ��̂ł́H */
					if( IFS_CRR_OFFSET > ucNowSlave ){			/* CRR��ւ̃A�N�Z�X�ł͂Ȃ�	*/
						if (IFM_LockTable.sSlave[ucNowSlave].psNext) {
						}
					}
					break;
				case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					nowCmd = pMail->sCommon.ucKind;
					break;
				default:
					return bRet;
			}
			/* ���ꂪ���̃R�}���h���H�ŁA�Ăяo�����M�֐����قȂ� */
			switch (nowCmd) {
				case 'I':	/* �����ݒ�f�[�f�[�^ */
					if( IFS_CRR_OFFSET > ucNowSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
						toScom_SendI49(&pMail->sI49);
					} else {
						toScom_SendI49_CRR(&pMail->sFlapI49);
					}
					break;
				case 'W':	/* ��ԏ������݃f�[�^ */
					toScom_SendW57(&pMail->sW57);
					break;
				case 'R':	/* ��ԗv���f�[�^ */
					if( IFS_CRR_OFFSET > ucNowSlave ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
						if (where == 0) {		/* �|�[�����O�Ȃ�A�q�@��ԃf�[�^�v�� */
							ucRequest = 0;
						} else {				/* �L���[�Ȃ�A(���Ԃ�)�o�[�W�����v�� */
							ucRequest = pMail->sR52.ucRequest;
						}
						toScom_SendR52((int)ucNowSlave, ucRequest);
					} else {
						toScom_SendV56((int)ucNowSlave);
					}
					break;
				case 'T':	/* �܂�Ԃ��e�X�g */
					toScom_SendT54(ucNowSlave);
					break;
				default:
					return bRet;
			}
			break;

		/*
		*	�ʐM�菇�����i���퉞�����Ԃ��ꂽ�j
		*	�E���M�ς݃R�}���h�̌�n��
		*	  1)�D��L���[�ɂ���Ȃ�A�O��
		*	  2)���ԃ{�[�h�ɂ́u�Ȃ��v�ɂ���
		*	  3)���O���M�R�}���h�́u�����ɂ��Ȃ��v�ɂ���
		*	�E�đ��J�E���^������
		*	�E�X�L�b�v�J�E���^������
		*/
		case 1:
			/* �����ɂ��������H�Ō�n�����قȂ� */
			switch (where) {
				case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
					break;								/* �������Ȃ� */
				case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
					pWhat = DeQueue(&toSque_Ctrl_Select, 0);	/* (�D��)�L���[���O���B*/
					pMail = (t_IFM_Queue *)pWhat->pData;
					/*
					*	2005.05.17 �����E�ǉ�
					*	�u���퉞�����Ԃ��ꂽ�v���Ă��Ƃ͑���q�@�ɓ͂������Ă��ƁB
					*	�Ȃ̂ŁA���̊֐����Ăяo���iLCM\LCMmain.c �ɂ���j
					*	�w��ԏ������݁x�d���������Ȃ�u���쒆�r�b�g�𗧂Ă�v
					*/
					LCM_SetStartAction(pMail);
					break;
				case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					if (!pMail) {
						break;
					}
					pMail->sCommon.bUsedByQue = 0;		/* ���ԃ{�[�h�ɂ��������̂́u���g�p�v�ɂ��� */
					/*
					*	2005.05.17 �����E�ǉ�
					*	�u���퉞�����Ԃ��ꂽ�v���Ă��Ƃ͑���q�@�ɓ͂������Ă��ƁB
					*	�Ȃ̂ŁA���̊֐����Ăяo���iLCM\LCMmain.c �ɂ���j
					*	�w��ԏ������݁x�d���������Ȃ�u���쒆�r�b�g�𗧂Ă�v
					*/
					LCM_SetStartAction(pMail);
					break;
				default:
					break;
			}
			IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* ���ԃ{�[�h�ɂ́u�Ȃ��v�ɂ��� */
			IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* ���O���M�R�}���h�́u�����ɂ��Ȃ��v�ɂ��� */

			toScom_ResetRetry(toScom_toSlave.ucNow);	/* �đ��J�E���^������ */
			toScom_ResetSkip(toScom_toSlave.ucNow);		/* �X�L�b�v�J�E���^������ */
			break;

		/*
		*	�ʐM�菇���s�iNAK�ԐM���ꂽ�̂ŁA�A���đ��j
		*	�E�đ��\/���g���C�I�[�o�[�𔻒�
		*	�E�đ��\�Ȃ�
		*		1)�đ��R�}���h�����߂�
		*		2)�đ�����B
		*		3)���M�����R�}���h�������Ă���
		*	�E���g���C�I�[�o�[�Ȃ�A
		*		1)���M�ς݃R�}���h�̌�n���i�ʐM�菇�������Ɠ��l�j
		*	�E�X�L�b�v�J�E���^������
		*/
		case 2:
			/* �đ��\�H*/
			bRetryOver = toScom_IsRetryOver(ucNowSlave);	/* �đ��\/���g���C�I�[�o�[�ŏ������قȂ� */
			if (!bRetryOver) {

				/* �����ɂ��������H�Ŏ���Ă���ʒu���قȂ�B*/
				switch (where) {
					case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
						nowCmd = 'R';	/*�u��ԗv���f�[�^�v*/
						break;
					case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
						pWhat = GetQueue(&toSque_Ctrl_Select);
						nowCmd = pWhat->pData->sCommon.ucKind;
						pMail = (t_IFM_Queue *)pWhat->pData;
						break;
					case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						nowCmd = pMail->sCommon.ucKind;
						break;
					default:
						return bRet;
				}
				/* ���ꂪ���̃R�}���h���H�ŁA�Ăяo�����M�֐����قȂ� */
				switch (nowCmd) {
					case 'I':	/* �����ݒ�f�[�f�[�^ */
						toScom_SendI49(&pMail->sI49);
						break;
					case 'W':	/* ��ԏ������݃f�[�^ */
						toScom_SendW57(&pMail->sW57);
						break;
					case 'R':	/* ��ԗv���f�[�^ */
						if (where == 0) {		/* �|�[�����O�Ȃ�A�q�@��ԃf�[�^�v�� */
							ucRequest = 0;
						} else {				/* �L���[�Ȃ�A(���Ԃ�)�o�[�W�����v�� */
							ucRequest = pMail->sR52.ucRequest;
						}
						toScom_SendR52((int)ucNowSlave, ucRequest);
						break;
					default:
						return bRet;
				}
				bRet = 1;	/* NAK�đ����� */
			}
			/* ���g���C�I�[�o�[ */
			else {
				/* �����ɂ��������H�Ō�n�����قȂ� */
				switch (where) {
					case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
						break;								/* �������Ȃ� */
					case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
						DeQueue(&toSque_Ctrl_Select, 0);	/* (�D��)�L���[���O���B*/
						break;
					case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						if (!pMail) {
							break;
						}
						pMail->sCommon.bUsedByQue = 0;	/* ���ԃ{�[�h�ɂ��������̂́u���g�p�v�ɂ��� */
						break;
					default:
						break;
				}
				IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* ���ԃ{�[�h�ɂ́u�Ȃ��v�ɂ��� */
				IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* ���O���M�R�}���h�́u�����ɂ��Ȃ��v�ɂ��� */
			}

			toScom_ResetSkip(toScom_toSlave.ucNow);		/* �X�L�b�v�J�E���^������ */
			break;

		/*
		*	�ʐM�菇���s�i�������������̂ŁA����đ��j
		*	�E�X�L�b�v����/�Ȃ��𔻒�
		*	�E�X�L�b�v�Ȃ��Ȃ�A
		*		1)�D��L���[�ɂ���Ȃ�A�t���ւ��i����đ��̂��߂Ɂj
		*	�E�X�L�b�v����Ȃ�A
		*		1)���M�ς݃R�}���h�̌�n���i�ʐM�菇�������Ɠ��l�j
		*/
		case 3:
			/* �܂��X�L�b�v�ł͂Ȃ��H*/
			bSkip = toScom_IsSkip(ucNowSlave);		/* �X�L�b�v����/�Ȃ��ŏ������قȂ� */
			if (!bSkip) {
				/* �����ɂ��������H�ŕt���ւ����������� */
				switch (where) {
					case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
						break;
					case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
						pWhat = DeQueue(&toSque_Ctrl_Select, 0);				/* (�D��)�L���[���O���B*/
						IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)pWhat->pData;	/* �t���ւ� */
						pWhat->pData->sCommon.bUsedByQue = 1;					/* ���ԃ{�[�h�ɕt���ւ��Ȃ̂Łu�g�p�v�ɂ��� */
						IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 2;	/* �ꏊ�͏��ԃ{�[�h�� */
						break;
					case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
						break;
				}
			}
			/* �X�L�b�v�ɂȂ��� */
			else {
				/* �����ɂ��������H�Ō�n�����قȂ� */
				switch (where) {
					case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
						break;								/* �������Ȃ� */
					case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
						DeQueue(&toSque_Ctrl_Select, 0);	/* (�D��)�L���[���O���B*/
						break;
					case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
						pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
						if (!pMail) {
							break;
						}
						pMail->sCommon.bUsedByQue = 0;	/* ���ԃ{�[�h�ɂ��������̂́u���g�p�v�ɂ��� */
						break;
					default:
						break;
				}
				IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* ���ԃ{�[�h�ɂ́u�Ȃ��v�ɂ��� */
				IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* ���O���M�R�}���h�́u�����ɂ��Ȃ��v�ɂ��� */
			}

			break;

		case 4:
			/* �����ɂ��������H�Ō�n�����قȂ� */
			switch (where) {
				case 0:		/* �����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j*/
					break;								/* �������Ȃ� */
				case 1:		/* �D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j*/
					DeQueue(&toSque_Ctrl_Select, 0);	/* (�D��)�L���[���O���B*/
					break;
				case 2:		/* ���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j*/
					pMail = IFM_LockTable.sSlave[ucNowSlave].psNext;
					if (!pMail) {
						break;
					}
					pMail->sCommon.bUsedByQue = 0;		/* ���ԃ{�[�h�ɂ��������̂́u���g�p�v�ɂ��� */
					break;
				default:
					break;
			}
			IFM_LockTable.sSlave[ucNowSlave].psNext = (t_IFM_Queue *)0;	/* ���ԃ{�[�h�ɂ́u�Ȃ��v�ɂ��� */
			IFM_LockTable.sSlave[ucNowSlave].ucWhereLastCmd = 0;		/* ���O���M�R�}���h�́u�����ɂ��Ȃ��v�ɂ��� */

			toScom_ResetRetry(toScom_toSlave.ucNow);	/* �đ��J�E���^������ */
			toScom_ResetSkip(toScom_toSlave.ucNow);		/* �X�L�b�v�J�E���^������ */

			break;

		default:
			break;
	}

	return bRet;
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_SetNextPolling()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���̃|�[�����O���������߂�										   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScom_SetNextPolling(int iPolling)
{
	int		ni;
	int		alive;
	ushort	usPolling;

	/* �|�[�����O�ł���l�������߂� */
	alive = 0;
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		if (IFM_LockTable.sSlave[ni].bComm) {
			alive++;
		}
	}

	/* �ЂƂ������Ƀ|�[�����O����Ԋu */
	if (alive == 1) {
// �|�[�����O�Ԋu�͐ݒ�ɏ]���̂���{
		usPolling = IFM_LockTable.toSlave.usPolling;	// �����ݒ�f�[�^�ɏ]��
	}
	/* �N�ɂ��|�[�����O�ł��Ȃ��i�N���������Ԃ�܂Ł��X�L�b�v��������܂Łj�Ƃ��̊Ԋu */
	else if (alive == 0) {
		usPolling = IFM_LockTable.toSlave.usPolling;	// �����ݒ�f�[�^�ɏ]��
	}
	/* �Q�l�ȏ㐶���Ă���Ƃ��Ƀ|�[�����O����Ԋu */
	else {
		if (iPolling == 0 ||								/* �N�����A�܂��� */
			iPolling == 1)									/* ���K�������́A*/
		{													/* �|�[�����O����̂ŁA*/
			usPolling = IFM_LockTable.toSlave.usPolling;	/* �����ݒ�f�[�^�ɏ]�� */
		}													/* ����ȊO�́A*/
		else {												/* �������̃|�[�����O�������̂ŁA*/
			usPolling = 0;									/* ���o�� */
		}
	}

	/* �|�[�����O�p�^�C�}�ĊJ */
	toScom_LineTimerStart(usPolling);
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScom_GetSlaveNow()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���̒ʐM������擾����											   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		toScom_GetSlaveNow(void)
{
	return (int)toScom_toSlave.ucNow;
}
/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendStatusToFlap()                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���b�N�Ǘ��^�X�N�ւ̏�ԕω��ʒm								   |*/
/*|		�E���b�N�Ǘ��e�[�u����̃Z���T�[��Ԃƕω�������Ƃ��̂ݒʒm����B |*/
/*|		�E�ʒm�́A����܂ł̃��[�������߁A�L���[�����ɂ���B			   |*/
/*|		�E���[���s�̗p�̗��R�͈ȉ��B									   |*/
/*|			1)���b�N�Ǘ��^�X�N�́A										   |*/
/*|			  �ΐ��Z�@�ʐM�^�X�N����́u���b�N����f�[�^�v��			   |*/
/*|			  �{�^�X�N�փ��[���]�����邽�߁A							   |*/
/*|			  �������ɖ{�^�X�N���u�q�@��ԃf�[�^�v�����[������ƁA		   |*/
/*|			  ���̃n���h�����O�菇�ɂăf�b�h���b�N�������N�����\������B |*/
/*|			2)���b�N�Ǘ��^�X�N�́A�{�^�X�N����́u�q�@��ԃf�[�^�v��	   |*/
/*|			  �ΐ��Z�@�ʐM�^�X�N����́u���b�N����f�[�^�v��			   |*/
/*|			  �������Ɏ󂯎�����ꍇ�A�u�q�@��ԃf�[�^�v��D�揈�����ׂ��B |*/
/*|			  ���[���ʒm�ɂ����ꍇ���b�N�Ǘ��^�X�N�́A���̔��肪�ʓ|�B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-04-18	�����E�ύX�B��L�ύX�B                     |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSmail_SendStatusToFlap(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	/*
	*	�����ɗ�����u�ω�����v�Ƃ�������
	*/
	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sFlapA41.sCommon.ucKind     = 'A';
	pQbody->sFlapA41.sCommon.ucTerminal = ucTerm;
	pQbody->sFlapA41.t_FlapSensor.c_LoopSensor		= pAns->unBody.sFlapA41.t_FlapSensor.c_LoopSensor;
	pQbody->sFlapA41.t_FlapSensor.c_ElectroSensor   = pAns->unBody.sFlapA41.t_FlapSensor.c_ElectroSensor;
	pQbody->sFlapA41.t_FlapSensor.c_LockPlate		= pAns->unBody.sFlapA41.t_FlapSensor.c_LockPlate;
	pQbody->sFlapA41.t_FlapSensor.c_FlapSensor		= pAns->unBody.sFlapA41.t_FlapSensor.c_FlapSensor;
	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	/*
	*	2005.07.12 �����E�C��
	*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
	*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
	*/
	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             toSmail_SendLoopData()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���[�v�f�[�^�����̎�M��ʒm����								   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	toSmail_SendLoopData(uchar ucTerm, t_IF_Packet *pAns)
{
t_IFM_Queue			*pQbody;	/* �L���[���� */
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	BOOL			bQfull;		/* 2005.07.12 �����E�ǉ� */

	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Status, sizeof(toSque_Body_Status));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Status, sizeof(toSque_Buff_Status));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* �L���[���̂������N */
		bQfull = 0;									/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		bQfull = 1;									/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}

	/*�u�q�@��ԃf�[�^�v�L���[���̂���� */
	pQbody->sFlapS53.sCommon.ucKind     = 'S';
	pQbody->sFlapS53.sCommon.ucTerminal = ucTerm;
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucLoopCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucLoopCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucLoopCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOffBaseCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOffBaseCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOffBaseCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOnLevelCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOnLevelCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOnLevelCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOffLevelCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOffLevelCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOffLevelCount));
	memcpy(pQbody->sFlapS53.t_LoopCounter.ucOnBaseCount,
			 pAns->unBody.sFlapS53.t_LoopCounter.ucOnBaseCount, sizeof(pQbody->sFlapS53.t_LoopCounter.ucOnBaseCount));
	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Status, pQbuff, (-1));

	if (bQfull) {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_OCCUR);		/* �L���[�t���i�q�@��ԃ��X�g�j[����] */
	} else {
		IFM_MasterError(E_IFM_QFULL_SlaveStatus, E_IF_RECOVER);		/* �L���[�t���i�q�@��ԃ��X�g�j[����]���� */
	}
}
