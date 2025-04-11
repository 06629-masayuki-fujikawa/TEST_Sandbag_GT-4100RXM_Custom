/*[]----------------------------------------------------------------------[]*/
/*|		Lock Management task in New I/F(Master)							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"mem_def.h"
#include	"mnt_def.h"
#include	"flp_def.h"
#include	"toS.h"
#include	"IFM.h"
#include	"LCM.h"
#include	"LCMdata.h"


/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define		LCM_FLAP_MAKER_COUNT	2


/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/

/* ����ǂ����Ɉړ����邱�ƁB*/
BOOL	IFM_bLockAction[IF_SLAVE_MAX][IFS_ROOM_MAX];
const	struct	LOCKMAKER_REC	LCMFlapmaker_rec[LCM_FLAP_MAKER_COUNT] = {
	{	15,	{	4,	4,	1,	12,	14, 14	}	},		// �T�j�J
// �t���b�v�㏸/���~���Ԃ̓t���b�v�̏㏸���� 14s�{���g���C�Ԋu 12s�{�㏸(���g���C) 12s �{�������v�� �� 42s�Ƃ���
// �������A42s�ł͒������Ɣ��f���A���c�̌���15s�֕ύX����
// �{�̂���̃��g���C�w���͂��Ȃ�(�t���b�v���ł���Ă����)���� ���g���C��0��Ƃ���
	{	LK_TYPE_CONTACT_FLAP,	{	4,	4,	0,	12,	15,	15	}	},		// �ړ_�t���b�v
};


/*----------------------------------*/
/*		external function define	*/
/*----------------------------------*/


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	LCM_init(void);

void	LCM_main(void);

void	LCM_ForwardSlave(void);					// �q�@�֓]��
void	LCM_SlaveStatus(t_IFM_Queue *pStatus);	// �ω�����Ȃ琸�Z�@�։���
void	LCM_FlapStatus(t_IFM_Queue *pStatus);	// �ω�����Ȃ琸�Z�@�։���

void	LCM_NoticeStatus_toPAY( uchar tno, uchar ucLock, uchar ucCarState, uchar ucLockState);
void	LCM_ControlLock(uchar bMode, uchar bLockOff, uchar ucSlave, uchar ucLock, uchar ucMaker);	/* ���b�N���u���� */

void	LCM_RequestVirsion(void);
void	LCM_RequestVirsion_FLAP(void);
void	LCM_Request_CRRTEST( uchar );
BOOL	LCM_WaitingLock(uchar ucOperation, uchar ucSlave, uchar ucLock, uchar ucMaker, BOOL bRetry, t_LKcomCtrl *c_dp);
uchar	LCM_Is_LockOnTrouble(BOOL *pbRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker);
uchar	LCM_Is_LockOffTrouble(uchar *pucState, uchar ucSensor, uchar ucLastStatus, uchar ucMaker);
void	LCM_NextAction(void);
void	LCM_TimeoutAction(void);
void	LCM_TimeoutTrouble(void);
void	LCM_TimerStartAction(uchar ucSlave, uchar ucLock, ushort usAction, BOOL bLockOff);

uchar	LCM_LockState_Initial(uchar ucNow, uchar ucMaker);
uchar	LCM_LockState_bySensor(uchar *pucLastState, uchar ucMaker, uchar ucLast, uchar ucNow, uchar *piRecover);
uchar	LCM_CarState_bySensor(uchar ucNow, uchar ucMaker, int iSlave, int iLock);
uchar	LCM_CarState_Initial(uchar ucNow, uchar ucMaker);

BOOL	LCM_Is_AllOver(uchar ucOperation, uchar ucSlave, uchar ucLock);
void	LCM_TimeoutTest(void);
void	LCM_TestResult(uchar lockState, int slave, int lock);
BOOL	LCM_Is_LockDirNow(uchar ucSlave, uchar ucLock);
uchar	LCM_do_ActionNow(int iSlave, int iLock);
void	LCM_CountAction(uchar lockState, int slave, int lock);

void	IFM_SetAction(uchar ucSlave, uchar ucLock);
void	IFM_ResetAction(uchar ucSlave, uchar ucLock);
BOOL	IFM_Is_Action(uchar ucSlave, uchar ucLock);
uchar	IFM_Get_ActionTerminalNo(void);
ushort	LCM_GetLimitAction(ushort usSigalOutTime);
ushort	LCM_GetLimitOneTest(ushort usLockOnTime, ushort usLockOffTime);

void LCM_TimeoutOperation(void);

void	LCM_NoticeStatusall_toPAY( void );
void	PAYcom_InitDataSave(void);
void	LCM_AnswerQueue_toPAY63(uchar type);
void	LCM_AnswerQueue_toPAY62(void);
void	LCM_AnswerQueue_toPAY64(void);
void	LCM_AnswerQueue_Timeout(void);
void	LCM_ForwardFlapTerminal(void);
void	LCM_NoticeFlapStatus_toPAY(void);
void	LCM_RequestLoopData(void);
void	LCM_RequestForceLoopControl(uchar ucOperation, uchar ucSlave);
uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam);

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_init()  	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|          ���b�N�Ǘ��^�X�N������                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_init(void)
{

	/* �^�C�}�[��J�E���^�N���A */
	LCM_TimerInit();			/* timer counter clear */

	/* ����ŃC���^�[�o���^�C�}���J�E���g���n�߂�B*/
	LCM_f_TmStart = 1;			/* timer process start */
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_main() 	                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          ���b�N�Ǘ��^�X�N�E���C��                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  20111-11-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_main(void)
{
struct t_IF_Queue	*pQbuff;	/* �L���[�{�� */
	int				ni;
	BOOL			bOneQueue;
	t_IFM_Queue*	pIFMQueue;

	// ���Ɉ�L���[�̏����Ȃ�uTRUE(=1)�v���A
	//	���ɑS�L���[�̏����Ȃ�uFALSE(=0)�v���Z�b�g���Ă��������B

	bOneQueue = 1;		/*�i�������j���Ɉ�L���[�̏����Ƃ��� */

	//	�q�@����̎q�@��ԃf�[�^����͂���B
	for (ni=0; ni < toSque_Ctrl_Status.usCount; ni++) {	/* �q�@��ԃ��X�g�ɃL���[����H*/
		/* �q�@�ʐM�^�X�N����M�����u�q�@��ԃf�[�^�v���A�L���[�o�R�Ŏ��o�� */
		pQbuff = DeQueue(&toSque_Ctrl_Status, 0);		/* �L���[(�擪)���O�� */
		if (!pQbuff) {
			break;
		}
		pIFMQueue = pQbuff->pData;
		if( IFS_CRR_OFFSET > pIFMQueue->sCommon.ucTerminal ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
			if( LKcom_Search_Ifno((uchar)(pIFMQueue->sCommon.ucTerminal + 1)) == 0 ){	// �Y���^�[�~�i��No�̐ڑ��̓��b�N���u
				LCM_SlaveStatus(pQbuff->pData);	/* �ω�����Ȃ琸�Z�@�։��� */
			}
			else {		// �t���b�v
				LCM_FlapStatus(pQbuff->pData);	/* �ω�����Ȃ琸�Z�@�։��� */
			}
		} else {												// CRR��ւ̃A�N�Z�X�̏ꍇ�A�t���b�v�֔�΂�
			LCM_FlapStatus(pQbuff->pData);	/* �ω�����Ȃ琸�Z�@�։��� */
		}

		/* ���Ɉ�L���[�̏����Ȃ�A�����Ŕ����� */
		if (bOneQueue) {
			break;
		}
	}

	//	���Z�@����̃��b�N���u����v������͂���B
	// �v���o�b�t�@�Ƀf�[�^���肩���M����
	if( 0 < LKcomTerm.CtrlInfo.Count ){				// �o�b�t�@�Ƀf�[�^�L
		LCM_ForwardSlave();							// �q�@�֓]��
		LKcom_SndDtDec();							// �o�b�t�@1���폜
	}

	/*
	*	���b�N���u�^�C���A�E�g����
	*	�E���슮���^�C���A�E�g
	*	�E�̏჊�g���C�Ԋu�^�C���A�E�g
	*/
	LCM_TimeoutAction();		/* ���슮���^�C���A�E�g */
	LCM_TimeoutTrouble();		/* �̏჊�g���C�Ԋu�^�C���A�E�g */
	LCM_TimeoutOperation();		// �o�[�W�����v���A���b�N���u�J�e�X�g�̃^�C���A�E�g����

}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_ForwardSlave()                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���Z�@�����M�����R�}���h���q�@�֓]������B					   |*/
/*|		�E�ΐ��Z�@�ʐM�^�X�N�o�R�ŁA�R�}���h�̓��[����M����B			   |*/
/*|		�E�Ύq�@�ʐM�^�X�N�o�R�ŁA�]���̓��[�����M����B				   |*/
/*|		�E���Z�@����́u���b�N���u����f�[�^�v��						   |*/
/*|		  �q�@�ւ́u��ԏ������݃f�[�^�v�ւƕϊ�����B					   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�E�����́A���[�����̃A�h���X				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_ForwardSlave(void)
{
	int		slave, lock;
	uchar	maker, doTest;
	uchar	carState, lockState;
	ushort	limitSecond, lockOnTime, lockOffTime;
	ushort	index;						// LockInfo�C���f�b�N�X
	uchar type;
	t_LKcomCtrl		*c_dp;
	c_dp = &LKcomTerm.CtrlData[LKcomTerm.CtrlInfo.R_Index];

	/* �����敪																	*/
	/*		 1 ���b�N���u�i���b�N����j										*/
	/*		 2 ���b�N���u�J�i���b�N�����j										*/
	/*		 3 �ڑ����Ă���S�Ẵ��b�N���u�A����уZ���T�[�̏�ԗv��			*/
	/*		 4 �w�肵�����b�N���u�A����уZ���T�[�̏�ԗv��						*/
	/*		 5 �ڑ����b�N���u�S�ĕi���b�N����j								*/
	/*		 6 �ڑ����b�N���u�S�ĊJ�i���b�N�����j								*/
	/*		 7 �����e�i���X���v��												*/
	/*		 8 ���b�N���u�J�e�X�g�v���i�S�āj									*/
	/*		 9 ���b�N���u�J�e�X�g�v���i�w��j									*/
	/*		10 ����J�E���^�[�N���A�i�S�āj										*/
	/*		11 ����J�E���^�[�N���A�i�w��j										*/
	/*		12 �o�[�W�����v��							2005.04.08 �����E�ǉ�	*/
	/*		13 �G���[��ԗv��							2005.04.08 �����E�ǉ�	*/

// �����I�Ƀ^�C���A�E�g���N�����A�������I��������
	if (IFM_LockTable.ucOperation != 0) {
		// �����ɓ���̂�8�A9�A12�̂Ƃ��̂�(���̓L���[�C���O/���M���ɃN���A���Ă��邽��)
		LCMtim_1secWaitStart(0);	// 0�Z�b�g�ŋ����I�Ƀ^�C���A�E�g�𔭐�������
		LCM_TimeoutOperation();		// �^�C���A�E�g����
	}

	IFM_LockTable.ucOperation = c_dp->kind;
	switch (IFM_LockTable.ucOperation) {
		//�q�@�֓]������
		case 1:		/* ���b�N���u�i���b�N����j*/
		case 2:		/* ���b�N���u�J�i���b�N�����j*/
			/* �Ԏ��̃`�F�b�N */
			if (c_dp->lock >= IFS_ROOM_MAX) {
				IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* ����f�[�^�ُ�[����] */
				break;
			}
			/* ���b�N���u���[�J�[�̃`�F�b�N */
			slave = c_dp->tno-1;												// CRB�̃^�[�~�i��No
			lock  =(uchar)(c_dp->lock-1);										// CRB�̃��b�N���u�ԍ�(1�`6)
			maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
			if (maker == 0 ||
				maker >= LOCK_MAKER_END)
			{
				IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* ����f�[�^�ُ�[����] */
				break;
			}
			/* ���b�N����ł��邩�H�ۂ��𔻒肵�A�ۂȂ�҂����鏈���i�L���[�C���O�j*/
			LCM_WaitingLock( (uchar)c_dp->kind,									// ����
						     (uchar)slave,										// CRB�̃^�[�~�i��No
						     (uchar)lock,										// CRB�̃��b�N���u�ԍ�(1�`6)
						     maker,												// ���b�N���u���[�J�[
						     0, c_dp);											// ���g���C�Ȃ��A�ʐ���
			break;

		case 5:		/* �ڑ����b�N���u�S�ĕi���b�N����j*/
		case 6:		/* �ڑ����b�N���u�S�ĊJ�i���b�N�����j*/
			// �t���b�v�܂��̓��b�N�̍ŏ��̒[��No.���ݒ肳��Ă���̂Ŏ�ʂ��擾����
			type = LKcom_Search_Ifno(c_dp->tno);
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// �^�[�~�i��No����
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRB�ڃ��b�N���u�ԍ�����
					WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						if (type == LKcom_Search_Ifno(slave + 1)) {
							// �㏸�^���~�R�}���h�Ȃ猻�݂̏�Ԃ��`�F�b�N����
							for( index = INT_CAR_START_INDEX; index < LOCK_MAX; ++index ){
								WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
								if( 0 == LKcom_Search_Ifno(slave + 1) ){	// ���b�N�̏ꍇ 
									if( (LockInfo[index].if_oya == (slave + 1)) && (LockInfo[index].lok_no == (lock + 1))){
										break;									// �Ώۂ�LockInfo�C���f�b�N�X����
									}
								} else {									// �t���b�v�̏ꍇ LockInfo[index].lok_no �́u1�v�Ɠǂݑւ���
									if( (LockInfo[index].if_oya == (slave + 1)) && ( 1 == (lock + 1))){
										break;									// �Ώۂ�LockInfo�C���f�b�N�X����
									}
								}
							}
							if(index >= LOCK_MAX){
								index = LOCK_MAX - 1;// for���[�v���Ō�܂ŉ������LCM_CanFlapCommand()�Ŕ͈͊O�A�N�Z�X���Ȃ��悤�u���b�N����
							}
							if(c_dp->kind == 5) {							// �S�㏸
								if(FALSE == LCM_CanFlapCommand(index, 1)) {			// �㏸�R�}���h���s�s��?
									// ���݂̏�Ԃ��㏸����ŏ㏸�ς݂̏ꍇ�́A�㏸�R�}���h���쐬���Ȃ�
									continue;
								}
							}
							else {											// �S���~
								if(FALSE == LCM_CanFlapCommand(index, 0)) {			// �㏸�R�}���h���s�s��?
									// ���݂̏�Ԃ����~����ŉ��~�ς݂̏ꍇ�́A���~�R�}���h���쐬���Ȃ�
									continue;
								}
							}
						LCM_WaitingLock( c_dp->kind, 
									     (uchar)slave, 
									     (uchar)lock, 
									     IFM_LockTable.sSlave[slave].sLock[lock].ucConnect, 
									     0, (t_LKcomCtrl *)0);
						}
					}
				}
			}
			break;

		case 8:		/* ���b�N���u�J�e�X�g�v���i�S�āj*/
			// �e�X�g�Ώۂ̎Ԏ���\�񂷂�i�e�X�g���ʂ̏��������܂ށj
			
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// �^�[�~�i��No����
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRB�ڃ��b�N���u�ԍ�����
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = c_dp->kind;			// �\��
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;						// ���ʂ̓N���A���Ƃ�
					}
					else {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 3;						// ���ʂ́u�ڑ������v�Ŋm��
					}
				}
			}
			// �J�e�X�g�\�ȍŏ��̎Ԏ��T���E�E�E������΁A�A���Ă��Ȃ��i����v�����đ����^�[���j

			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// �^�[�~�i��No����
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRB�ڃ��b�N���u�ԍ�����
					WACDOG;																		// ���u���[�v�̍ۂ�WDG���Z�b�g���s

					// �J�e�X�g�ΏۊO�͏��O
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation != c_dp->kind) {
						continue;
					}
					// ���܊J�e�X�g�ł���H
					maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
					doTest = LCM_do_ActionNow(slave, lock);
					if (doTest == 1) {
						// ����������u���̎Ԏ��v����n�߁A�p���͓����ɔ��肷��
						LCM_WaitingLock(c_dp->kind, (uchar)slave, (uchar)lock, maker, 0, (t_LKcomCtrl *)0);

						//	�J�e�X�g�͂�������J�n
						//	�E�J�e�X�g�����Z�b�g
						//	�E���܊J�e�X�g���ׂ����b�N���u�̐������Ԃ����߂�B
						//	�����̃��b�N���u�̐������Ԃ́A���̓s�x���߂ă^�C�}�[�ĊJ�����@�ɂ���B
						//	  ���̓s�x�Ƃ́ALCM_Is_AllOver()�ɂāA���̃e�X�g�Ώۃ��b�N���u��T���o�����Ƃ��B
						//
						IFM_LockTable.bTest = 1;

						lockOnTime = IF_SignalOut[maker].usLockOnTime;
						lockOffTime = IF_SignalOut[maker].usLockOffTime;
						limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
						LCMtim_1secWaitStart(limitSecond);

						// ��L�œ���v��������A�����߂�Ȃ�
						return;
					}
					// ���܊J�e�X�g�ł��Ȃ����̂́A�����{ 
					IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;		// �J�e�X�g�\����L�����Z��
					IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;		// ���ʂ́u�w��O�v�Ŋm��
				}
			}

			/*
			*	�����ɗ�����A�ЂƂ��J�e�X�g�����{�Ƃ������ƁB
			*	�E�ЂƂ����b�N���u���u�ڑ��Ȃ��v
			*	�E�ЂƂ̎q�@�ւ��ʐM�ł��Ȃ�
			*/
			/* �J�e�X�g���ʒʒm�́u�^�C���A�E�g�v�����ɂ� */
			LCMtim_1secWaitStart(0);	/* �O�Z�b�g���ċ����Ƀ^�C���A�E�g���N�������� */
			break;

		case 9:		/* ���b�N���u�J�e�X�g�v���i�w��j*/
			/* �e�X�g���ʂ������� */
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {						// �^�[�~�i��No����
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRB�ڃ��b�N���u�ԍ�����
					WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;					// ���ʂ̓N���A���Ƃ�
					}
					else {
						IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 3;					// ���ʂ́u�ڑ������v�Ŋm��
					}
				}
			}
			/* �e�X�g�Ώۂ̎Ԏ����H�Ȃ�\�񂷂� */
			slave = c_dp->tno-1;												// CRB�̃^�[�~�i��No
			lock  =(uchar)(c_dp->lock-1);										// CRB�̃��b�N���u�ԍ�(1�`6)
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucConnect) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = c_dp->kind;	/* �\�� */
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;			/* ���ʂ̓N���A���Ƃ� */

				/* ���܊J�e�X�g�ł���H*/
				maker = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;
				doTest = LCM_do_ActionNow(slave, lock);
				if (doTest == 1) {
					/* �ł���Ȃ�u���̎Ԏ��v�𓮍삳���A�p���͓����ɔ��肷�� */
					LCM_WaitingLock( c_dp->kind,								// ����
						     (uchar)slave,										// CRB�̃^�[�~�i��No
						     (uchar)lock,										// CRB�̃��b�N���u�ԍ�(1�`6)
						     maker,												// ���b�N���u���[�J�[
						     0, c_dp);											// ���g���C�Ȃ��A�ʐ���
					/*
					*	�J�e�X�g�͂�������J�n
					*	�E�J�e�X�g�����Z�b�g
					*	�E�S�Ẳ������Ԃ����܂ł̐������Ԃ����߂�
					*/
					/*
					*	�J�e�X�g�́A���̃��b�N���u�݂̂ɑ΂��āB
					*	�E�J�e�X�g�����Z�b�g
					*	�E���̊J�e�X�g���ׂ����b�N���u�̐������Ԃ����߂�B
					*	���J/�Ȃ̂łQ�񕪂̓��쎞�Ԃ��K�v�B
					*/
					IFM_LockTable.bTest = 1;

					/* �d����̐M���o�͎��Ԃ��͈͊O�Ȃ�A�␳����i�{���j*/
					lockOnTime  = c_dp->clos_tm;
					if (lockOnTime == 0) {
						lockOnTime = IF_SignalOut[maker].usLockOnTime;
					}
					/* �d����̐M���o�͎��Ԃ��͈͊O�Ȃ�A�␳����i�J���j*/
					lockOffTime = c_dp->open_tm;
					if (lockOffTime == 0) {
						lockOffTime = IF_SignalOut[maker].usLockOffTime;
					}
					limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
					LCMtim_1secWaitStart(limitSecond);

					/* ��L�œ���v��������A�����߂�Ȃ� */
					return;
				}
				/* ���܊J�e�X�g�ł��Ȃ����̂́A�����{ */
				else {
					IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;	/* �J�e�X�g�\����L�����Z�� */
					IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 0;		/* ���ʂ́u�w��O�v�Ŋm�� */
				}
			}
			/*
			*	�����ɗ�����A�J�e�X�g�����{�Ƃ������ƁB
			*	�E�w�胍�b�N���u���u�ڑ��Ȃ��v
			*	�E�w�胍�b�N���u�̎q�@�֒ʐM�ł��Ȃ�
			*/
			/* �J�e�X�g���ʒʒm�́u�^�C���A�E�g�v�����ɂ� */
			LCMtim_1secWaitStart(0);	/* �O�Z�b�g���ċ����Ƀ^�C���A�E�g���N�������� */
			break;

		//���b�N�Ǘ��e�[�u������E���āA�ΐ��Z�@�����L���[�֑}��
		case 3:		/* �ڑ����Ă���S�Ẵ��b�N���u�A����уZ���T�[�̏�ԗv�� */
			if( LKcom_Search_Ifno( c_dp->tno ) == 0 ){	// �Y���^�[�~�i��No�̐ڑ��̓��b�N���u
				LCM_NoticeStatusall_toPAY();
			}
			break;
		case 4:		/* �w�肵�����b�N���u�A����уZ���T�[�̏�ԗv�� */
			slave = c_dp->tno-1;												// CRB�̃^�[�~�i��No
			lock  =(uchar)(c_dp->lock-1);										// CRB�̃��b�N���u�ԍ�(1�`6)
			carState  = IFM_LockTable.sSlave[slave].sLock[lock].ucCarState;
			lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
			LCM_NoticeStatus_toPAY(c_dp->tno,c_dp->lock, carState, lockState);
			break;
		case 10:	/* ����J�E���^�[�N���A�i�S�āj*/
			// �t���b�v�܂��̓��b�N�̍ŏ��̒[��No.���ݒ肳��Ă���̂Ŏ�ʂ��擾����
			type = LKcom_Search_Ifno(c_dp->tno);
			for (slave=0; slave < IF_SLAVE_MAX; slave++) {										// CRB�̃^�[�~�i��No����
				if(type == LKcom_Search_Ifno((uchar)(slave + 1))) {
				for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {									// CRB�ڃ��b�N���u�ԍ�����
					WACDOG;													// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					IFM_LockAction.sSlave[slave].sLock[lock].ulAction  = 0;	/* �J����̍��v�� */
					IFM_LockAction.sSlave[slave].sLock[lock].ulManual  = 0;	/* I/F��SW�ɂ��蓮�ł̊J�E���썇�v�� */
					IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble = 0;	/* �̏Ⴕ�����v�� */
				}
				}
			}
			break;
		case 11:	/* ����J�E���^�[�N���A�i�w��j*/
			slave = c_dp->tno-1;												// CRB�̃^�[�~�i��No
			lock  =(uchar)(c_dp->lock-1);										// CRB�̃��b�N���u�ԍ�(1�`6)
			IFM_LockAction.sSlave[slave].sLock[lock].ulAction  = 0;		/* �J����̍��v�� */
			IFM_LockAction.sSlave[slave].sLock[lock].ulManual  = 0;		/* I/F��SW�ɂ��蓮�ł̊J�E���썇�v�� */
			IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble = 0;		/* �̏Ⴕ�����v�� */
			break;
		case 7:		/* �����e�i���X���v�� */
			type = LKcom_Search_Ifno(c_dp->tno);
			LCM_AnswerQueue_toPAY63(type);										// (63H)�����e�i���X��񉞓�
			break;

		case 12:	/* �o�[�W�����v�� */
			if( c_dp->tno >= IFS_CRR_OFFSET ){	// CRR���(�t���b�v)�ɑ΂��Ẵo�[�W�����v��
				LCM_RequestVirsion_FLAP();
			} else {
				if( LKcom_Search_Ifno(c_dp->tno) == 0 ){	// �o�[�W�����v���̓��b�N���u�̂�
					LCM_RequestVirsion();
				}
			}
			break;

		case 13:			// �G���[��ԗv��
			// ���Ή�
			break;
		case 14:			// ����|�[�g�����i�S�āj
			LCM_Request_CRRTEST(c_dp->tno);
			break;
		case 15:			// ���[�v�f�[�^�v��
			if( LKcom_Search_Ifno(c_dp->tno) == 1 ){	// ���[�v�f�[�^�v���̓t���b�v�̂�
				LCM_RequestLoopData();
			}
			break;
		case 16:			// ���[�v����ON
		case 17:			// ���[�v����OFF
			LCM_RequestForceLoopControl(c_dp->kind, (uchar)(c_dp->tno - 1));
			break;

		default:
			IFM_MasterError(E_IFM_CTRL_Command, E_IF_BOTH);		/* ����f�[�^�ُ�[����] */
			break;
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_WaitingLock()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���b�N����\����                                                 |*/
/*|		�E���b�N����ł��邩�H�ۂ��𔻒肷��B                             |*/
/*|		  �ł���Ȃ�A�q�@�֗v�����邽�ߎq�@�ʐM�^�X�N�փ��[�����M�B       |*/
/*|		  �ۂȂ�A�҂����邽�߁u���b�N���쓮�슮���҂����X�g�v��           |*/
/*|		          �L���[�C���O����B                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-29                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL LCM_WaitingLock(uchar ucOperation, uchar ucSlave, uchar ucLock, uchar ucMaker, BOOL bRetry, t_LKcomCtrl *c_dp)
{
t_IFM_Queue			*pQbody;
struct t_IF_Queue	*pQbuff;
	ushort			lockOnTime, lockOffTime;
	uchar			action;
	BOOL			bRet;
	BOOL			bLockOff;
	BOOL			bQfull;			/* 2005.07.12 �����E�ǉ� */
	int		del;
	int		scount,in_count;
	t_IFM_Queue	quesave;

	/* �ǂ��������ւ̓���v�����H���u�����敪�v���画�� */
	switch (ucOperation) {
		case 1:		/* ���b�N�{�� */
		case 5:		/* �S���b�N�{�� */
			bLockOff = 0;
			break;
		case 2:		/* ���b�N�J�� */
		case 6:		/* �S���b�N�J�� */
			bLockOff = 1;
			break;
		case 8:		/* �S���b�N�J�e�X�g */
		case 9:		/* �ʃ��b�N�J�e�X�g */
			/* �J�e�X�g�́A����Ԃ̋t�������삩��n�߂� */
			if (LCM_Is_LockDirNow(ucSlave, ucLock)) {	/* ���u���b�N�{�v�Ȃ� */
				bLockOff = 1;							/* �J�����삩��n�߂� */
			} else {									/* ���u���b�N�J�v�Ȃ� */
				bLockOff = 0;							/* �{�����삩��n�߂� */
			}
			break;

		/* �s���ȏ����敪�ɂ��Ăяo�� */
		default:
			return 0;	/*�u�҂����ɗv�������v���� */
	}

	/* �v�����̐M���o�͎��Ԃ��g�����ۂ��H */
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		if (c_dp) {
			// �d����̐M���o�͎��Ԃ��̗p
			lockOnTime = c_dp->clos_tm;						// ���b�N���u���̐M���o�͎���
			lockOffTime = c_dp->open_tm;					// ���b�N���u�J���̐M���o�͎���
		}
		else {
			/* �ߋ��̐M���o�͎��Ԃ��̗p */
			lockOnTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime;		/* ���b�N���u���̐M���o�͎��� */
			lockOffTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime;	/* ���b�N���u�J���̐M���o�͎��� */
		}
	}
	else {
		lockOnTime = 0;						// 0�Œ�
		lockOffTime = 0;					// 0�Œ�
	}

	/* (0)�{��/(1)�J���A������̓��삩�H*/
	if (bLockOff == 0) {/*�u���b�N����!�v���q�@�֗v�� */
		action   = 1;	/* �{��[��]����v������ */
	}
	else {				/*�u���b�N��������!�v���q�@�֗v�� */
		action   = 2;	/* �J��[�J]����v������ */
	}
	IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = ucOperation;	/* �����敪�ޔ� */

	/* ���ݓ���\�A���A�w��Ԏ������쒆�łȂ� */
	if (IFM_LockTable.usWaitCount < IF_ACTION_MAX &&						/* ���쒆��҂ő吔�����ŁA���� */
		IFM_Can_Action(LKcom_Search_Ifno(ucSlave + 1)) &&					// �Ώۑ��u������\��
		!IFM_Is_Action(ucSlave, ucLock))									/* ���̎Ԏ����u���쒆�v�ł͖��� */
	{
// ���g���C�Ȃ�ΐV�K�L���[�̗v����D��
		if (bRetry != 0) {
			// �ȉ��̃f�[�^�쐬/�L���[�C���O�����͖{�֐����̃L���[�C���O�����𗬗p
			/* �󂫃L���[�݂邩�H*/
			pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	/* �󂫎��̂��擾 */
			pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	/* �󂫖{�̂��擾 */
			if (pQbody && pQbuff) {
				pQbuff->pData = pQbody;						/* ���̂������N */
				bQfull = 0;									/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����]���� */
			}
			/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
			else {
				bQfull = 1;									/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����] */
				pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);	/* �L���[(�擪)���O�� */
				pQbody = pQbuff->pData;
			}
			/* �q�@�����u��ԏ������݃f�[�^�v����� */
			pQbody->sW57.sCommon.ucKind     = 'W';			/* ��ԏ������݃f�[�^ */
			pQbody->sW57.sCommon.ucTerminal = ucSlave;		/* �^�[�~�i��No.�i�ǂ̎q�@���H����肷��j*/
			if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
			pQbody->sW57.usLockOffTime      = lockOffTime;	/* ���Ɍ��߂��u�J)�M���o�͎��ԁv���̗p */
			pQbody->sW57.usLockOnTime       = lockOnTime;	/* ���Ɍ��߂��u�{)�M���o�͎��ԁv���̗p */
			pQbody->sW57.ucLock             = ucLock;		/* ���b�N���uNo. */
			pQbody->sW57.ucMakerID          = ucMaker;		/* ���b�N���u���[�J�[ID */
			pQbody->sW57.unCommand.Byte     = 0;			/* ��U�A�S�r�b�g���N���A */
			pQbody->sW57.unCommand.Bit.B4   = bLockOff;		/* (0)�{��/(1)�J���A������̓��� */
			pQbody->sW57.unCommand.Byte     |= 0x40;		/* �r�b�g6���Z�b�g(�K�[�h���ԑΉ�) */
			}
			else {
				pQbody->sW57.usLockOffTime      = 0;
				pQbody->sW57.usLockOnTime       = 0;
				pQbody->sW57.ucLock             = 0;
				pQbody->sW57.ucMakerID          = 0;
				pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);	// �t���b�v�㏸/���~�R�}���h�ɕϊ�
			}

			// ���g���C�L���[�ɃZ�b�g
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 1);		// ���g���C�L���[

			pQbody->sW57.ucOperation = ucOperation;						/* ���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩 */
			pQbody->sW57.bWaitRetry  = bRetry;							/* ��������i���̓���v���́A���񂩃��g���C���j*/
			bRet = 1;	/* �҂̂ŁA�v���̓L���[�C���O���� */
			if (bQfull) {
				IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_OCCUR);	/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����] */
			} else {
				IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_RECOVER);	/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����]���� */
			}
			// �L���[�C���O������A�V�K���M�L���[�����g���C�L���[�̏��ŃT�[�`���A���M
			LCM_NextAction();

			return 1;	// ���{�߂�l���Q�Ƃ��Ă���ӏ��͂Ȃ�
		}
		/* ���Ɍ��߂��u�M���o�͎��ԁv���̗p */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime = lockOnTime;
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime = lockOffTime;

		// ���䑗�M�O�ɁA���ꃍ�b�N���u�ւ̗v�������邩�A���g���C�L���[���T�[�`
		while (1) {
			del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, ucSlave, ucLock);
			// ���b�N���u����v�Ȃ�΂��̗v�����폜
			if (del >= 0) {
				DeQueue(&LCMque_Ctrl_Action.Retry, del);
			}
			if (del == -1) break;
		}
		// �֘A����đ��^�C�}�X�g�b�v
		LCMtim_1secTimerStop((int)ucSlave, (int)ucLock);

		/* �q�@�ʐM�^�X�N�ցu��ԏ������݃f�[�^�v���[���𔭐M */
		LCM_ControlLock(0, bLockOff, ucSlave, ucLock, ucMaker);
		IFM_SetAction(ucSlave, ucLock);										/* ����v�������̂ŁA���쒆�� */

		/*
		*	���܁u���b�N/�����v�������b�N���u�ɑ΂��āA�e�[�u���}�[�N�B
		*	���q�@��ԃf�[�^��Ԃ���A���슮���𔻒肵����}�[�N�O��
		*	�E���슮���҂������J�E���g�A�b�v
		*	�E�u���b�N��ԁv�𓮍쒆��(=0)
		*	�E�����敪�i���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩�j
		*	�E���엚���i�{��[��]/�J��[�J]����̓���v�����������j
		*	�E�̏᎞���g���C��
		*		����Ȃ�A���g���C�񐔂�������
		*		���g���C�Ȃ�A���g���C�񐔂��J�E���g�_�E���i���g���C����K�v�����邩��{�֐����Ăяo���ꂽ�̂Łj
		*/
		IFM_LockTable.usWaitCount++;											/* ���슮���҂������J�E���g */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = ucOperation;	/* ���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩 */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction    = action;		/* �{��[��]/�J��[�J]����̓���v���������� */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry = IFM_LockTable.sSlave[ucSlave].usRetryCount;
		bRet = 0;	/* �҂����ɗv������ */
	}

	/* ���ݓ���s�\�̂��߁A�L���[�C���O */
	else {
		/* �󂫃L���[�݂邩�H*/
		pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	/* �󂫎��̂��擾 */
		pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	/* �󂫖{�̂��擾 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* ���̂������N */
			bQfull = 0;									/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����]���� */
		}
		/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
		else {
			bQfull = 1;									/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����] */
			pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);	/* �L���[(�擪)���O�� */
			pQbody = pQbuff->pData;
		}
		/* �q�@�����u��ԏ������݃f�[�^�v����� */
		pQbody->sW57.sCommon.ucKind     = 'W';			/* ��ԏ������݃f�[�^ */
		pQbody->sW57.sCommon.ucTerminal = ucSlave;		/* �^�[�~�i��No.�i�ǂ̎q�@���H����肷��j*/
		if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
			pQbody->sW57.usLockOffTime      = lockOffTime;	/* ���Ɍ��߂��u�J)�M���o�͎��ԁv���̗p */
			pQbody->sW57.usLockOnTime       = lockOnTime;	/* ���Ɍ��߂��u�{)�M���o�͎��ԁv���̗p */
			pQbody->sW57.ucLock             = ucLock;		/* ���b�N���uNo. */
			pQbody->sW57.ucMakerID          = ucMaker;		/* ���b�N���u���[�J�[ID */
			pQbody->sW57.unCommand.Byte     = 0;			/* ��U�A�S�r�b�g���N���A */
			pQbody->sW57.unCommand.Bit.B4   = bLockOff;		/* (0)�{��/(1)�J���A������̓��� */
			pQbody->sW57.unCommand.Byte     |= 0x40;		/* �r�b�g6���Z�b�g(�K�[�h���ԑΉ�) */
		}
		else {		// �t���b�v
			pQbody->sW57.usLockOffTime      = 0;
			pQbody->sW57.usLockOnTime       = 0;
			pQbody->sW57.ucLock             = 0;
			pQbody->sW57.ucMakerID          = 0;
			pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);	// �t���b�v�㏸/���~�R�}���h�ɕϊ�
		}
		/* �����փL���[�C���O */
		// �L���[�C���O����L���[��I��
		if (bRetry == 0) {
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 0);		// �V�K�L���[
		} else {
			EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 1);		// ���g���C�L���[
		}

		/*
		*	�L���[�C���O�����u���b�N/�����v���郍�b�N���u�ɑ΂��āA�e�[�u���}�[�N�����B
		*	���q�@��ԃf�[�^��Ԃ���A���슮���𔻒肵����A���̂Ƃ��e�[�u���}�[�N�ł���
		*	�E�����敪�i���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩�j
		*	�E���g���C�҂��t���O�i���̓���v���́A���񂩃��g���C���j
		*/
		pQbody->sW57.ucOperation = ucOperation;									/* ���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩 */
		pQbody->sW57.bWaitRetry  = bRetry;										/* ��������i���̓���v���́A���񂩃��g���C���j*/

		bRet = 1;	/* �҂̂ŁA�v���̓L���[�C���O���� */

		/*
		*	2005.07.12 �����E�C��
		*	�L���[�t������/�����𔻒肵���A���̂Ƃ��ɁuIFM_MasterError(IFM.c)�v���Ăяo���Ă͂Ȃ�Ȃ��B
		*	���ڂ����́uPAYcom_NoticeError(PAYnt_net.c)�v���Q�Ƃ̂���
		*/
		if (bQfull) {
			IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_OCCUR);		/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����] */
		} else {
			IFM_MasterError(E_IFM_QFULL_LockAction, E_IF_RECOVER);		/* �L���[�t���i���b�N���쓮�슮���҂����X�g�j[����]���� */
		}

		// �o�ɗD�搧��F�u���~�v�w���i�o�Ɏw���̃^�C�~���O�j�ŐV�K�L���[�̕t���������s��
		if((10 == prm_get(COM_PRM, S_TYP, 100, 2, 1))&& 		// 03-0100�D�E=10�F�^�p�̓t���b�v�̂� ����
		  (0 == prm_get(COM_PRM, S_TYP, 133, 1, 1))&& 			// 03-0133�E�F�o�ɗD�搧�䂠�� ����
		  (bRetry == 0)&&										// �V�K�L���[�ɓo�^ ����
		  // ���g���C�L���[�̓��g���C���ɃZ�b�g����邱�ƁA�܂��V�K�L���[�̏��������ׂďI�����Ă��瓮�삷�邽�ߐ���̑Ώۂɂ��郁���b�g�͏��Ȃ����ߑΏۊO�Ƃ���
		  (bLockOff != 0)){										// �u���~�v�̏ꍇ
			// NOTE:�{�����͑ΏێԎ��̏㏸���삪�I�����Ă��Ȃ����ɉ��~�w���������ꍇ�A���̂܂܏o�ɗD�搧����s����
			// �㏸�w���Ɖ��~�w���̏��Ԃ��t�]���A�Ԃ����Ȃ���ԂŃt���b�v���㏸����\�������邽��
			// ���~�w���������Ƃ��ɓ���Ԏ��̏㏸�w�����c���Ă����ꍇ�A�L���[����폜����
			scount = Seach_Close_command_sameno(&LCMque_Ctrl_Action.New,ucSlave);// �L���[�̐擪�i�ŌÁj���猟���J�n
			if(scount != -1){// ���Ԏ��̃t���b�v�㏸�v���L��
				pQbuff = DeQueue(&LCMque_Ctrl_Action.New, scount);//�Y���L���[���폜
			}
			if(LCMque_Ctrl_Action.New.usCount >= 2 ){// �Ώۂ̃L���[��2�ȏ�̂Ƃ��̂ݎ��s����
				in_count = Get_Close_command_cnt(&LCMque_Ctrl_Action.New);// �L���[�̒��ɉ��u�t���b�v�㏸�v���v�����邩�擾
				if(in_count > 0){//�u�t���b�v�㏸�v���v�����݂��鎞�̂ݎ��s
					scount = 0;// �L���[�̐擪�i�ŌÁj���猟���J�n
					for(;;){
						scount = Seach_Close_command(&LCMque_Ctrl_Action.New,scount);
						if(scount != -1){// �t���b�v�㏸�v���L��
							pQbuff = DeQueue(&LCMque_Ctrl_Action.New, scount);
							memset(&quesave,0,sizeof(quesave));
							memcpy(&quesave,pQbuff->pData,sizeof(quesave));
							// �����ł́uDeQueue�v����̎擾�Ȃ̂ł���Ƃ݂Ȃ��B
							pQbody = GetQueueBody(LCMque_Body_Action, sizeof(LCMque_Body_Action));	// �󂫎��̂��擾
							pQbuff = GetQueueBuff(LCMque_Buff_Action, sizeof(LCMque_Buff_Action));	// �󂫖{�̂��擾
							if (pQbody && pQbuff) {
								pQbuff->pData = pQbody;	// ���̂������N
							}
							// �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j
							else {
								pQbuff = RemoveQueue2(&LCMque_Ctrl_Action, 0);// �L���[(�擪)���O��
								pQbody = pQbuff->pData;
							}
							memcpy(pQbody,&quesave,sizeof(quesave));
							EnQueue2(&LCMque_Ctrl_Action, pQbuff, (-1), 0);// �V�K�L���[
							in_count--;// �L���[�̂��Ȃ����������u�t���b�v�㏸�v���v�̐������炷
							if(in_count <= 0){// ���ׂĂ��Ȃ�����������
								break;
							}
						}else{// �t���b�v�㏸�v������
							break;
						}
					}
				}
			}
		}
	}

// �㏈���̂��鐧��ȊO�̓N���A����
	if ((ucOperation != 8) &&	// �J�e�X�g
		(ucOperation != 9) &&	// �e�X�g
		(ucOperation != 12)) {	// �o�[�W�����v�� �ȊO�Ȃ�
		IFM_LockTable.ucOperation = 0;	// ���ݐ����ʃN���A
	}

	return bRet;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestVirsion()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�o�[�W�����v��                                                     |*/
/*|		�E���Z�@�����M�����u�o�[�W�����v���v���q�@�֓]������B           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_RequestVirsion(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	�E�o�[�W�����v���̑O�ɁA��Ƀe�[�u���ɂ́uno slave�v�������l�ݒ肵�Ă����B
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {			/* �q�@�̐��� */
		if(LKcom_Search_Ifno((uchar)(ni + 1)) == 0) {
			// ���b�N���u�Ȃ�"no slave"�ŏ���������
			memcpy(IFM_LockTable.sSlave[ni].cVersion, "no slave", sizeof(IFM_LockTable.sSlave[ni].cVersion));
		} else {
			// ���b�N���u�ȊO��0�ŏ�����
			memset(IFM_LockTable.sSlave[ni].cVersion, 0, sizeof(IFM_LockTable.sSlave[ni].cVersion));
		}
	}

	// �ȉ��̏�����toS_main()�֐�������p
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		//	�ȉ��͏��O�B
		//	�E�ʐM����K�v���Ȃ��q�@�i���b�N���u���ڑ������j
		//	�E�����ݒ�f�[�^�𑗂����̂�ACK�������Ă��Ȃ��q�@
		//	�E�������X�L�b�v���̎q�@
		if (!IFM_LockTable.sSlave[ni].bComm ||
			!IFM_LockTable.sSlave[ni].bInitialACK ||
			IFM_LockTable.sSlave[ni].bSkip)
		{
			IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
			continue;
		}
		/* �o�[�W�����v��/�����͂P�񂱂����� */
		else {
			if(0 != LKcom_Search_Ifno((uchar)(ni + 1))) {			//���b�N���u�ȊO�̓o�[�W�����`�F�b�N�����Ȃ�
				IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
				continue;
			}
			IFM_LockTable.sSlave[ni].ucContinueCnt = 1;
		}

		/* �󂫃L���[�݂邩�H*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* ���̂������N */
			/* �L���[�t��[����]���� */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
		else {
			/* �L���[�t��[����] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
			pQbody = pQbuff->pData;
		}
		/* ���[���Łu��ԗv���f�[�^�v��������A����́w�o�[�W�����v���x���ƌ��ߑł��ŗǂ� */
		pQbody->sCommon.ucKind     = 'R';				/* ��ԗv���f�[�^�i�o�[�W�����v���j*/
		pQbody->sCommon.ucTerminal = ni;				/* �^�[�~�i��No.�i�ǂ̎q�@�ւ��H����肷��j*/
		pQbody->sR52.ucRequest     = 1;					/* �v�����e�́A�o�[�W�����v�� */
		/* �����փL���[�C���O */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* �o�[�W�����v�����o���̂ŁA���̉������Ԃ����܂ł̐������Ԃ����߂� */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestVirsion_FLAP()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�o�[�W�����v��(FLAP)                                               |*/
/*|		�E���Z�@�����M�����u�o�[�W�����v���v���q�@�֓]������B           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-04-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_RequestVirsion_FLAP(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	�o�[�W�����v���̑O�ɁA��Ƀe�[�u���ɂ́uno slave�v�������l�ݒ肵�Ă����B
	for (ni=0; ni < IFS_CRR_MAX; ni++) {			/* CRR�̐ڑ��ő吔�� */
		memcpy(IFM_LockTable.sSlave_CRR[ni].cVersion, "no slave", sizeof(IFM_LockTable.sSlave_CRR[ni].cVersion));	// "no slave"�ŏ���������
	}

	// �ȉ��̏�����toS_main()�֐�������p
	for (ni=0; ni < IFS_CRR_MAX; ni++) {
		//	�ʐM����K�v���Ȃ�CRR���(�^�[�~�i��No.���U���Ă��Ȃ��j�͑��M���Ȃ�
		if (!IFM_LockTable.sSlave_CRR[ni].bComm ){
			continue;
		}

		/* �󂫃L���[�݂邩�H*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* ���̂������N */
			/* �L���[�t��[����]���� */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
		else {
			/* �L���[�t��[����] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
			pQbody = pQbuff->pData;
		}
		/* ���[���Łu��ԗv���f�[�^�v��������A����́w�o�[�W�����v���x���ƌ��ߑł��ŗǂ� */
		pQbody->sCommon.ucKind     = 'R';				/* ��ԗv���f�[�^�i�o�[�W�����v���j*/
		pQbody->sCommon.ucTerminal = ni+IFS_CRR_OFFSET;	/* CRR���No. +100�����邱�ƂŃ^�[�~�i��No.�Ƌ�ʂ���*/
		/* �����փL���[�C���O */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* �o�[�W�����v�����o���̂ŁA���̉������Ԃ����܂ł̐������Ԃ����߂� */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Request_CRRTEST()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRR�܂�Ԃ��e�X�g                                                  |*/
/*|		�E���Z�@�����M�����u�e�X�g�v���v���q�@�֓]������B               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2013/01/24                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_Request_CRRTEST( uchar tno )
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;

	//	�ʐM����K�v���Ȃ�CRR���(�^�[�~�i��No.���U���Ă��Ȃ��j�͑��M���Ȃ�
	if (!IFM_LockTable.sSlave_CRR[tno - IFS_CRR_OFFSET ].bComm ){
		return;
	}

	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* ���̂������N */
		/* �L���[�t��[����]���� */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		/* �L���[�t��[����] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}
	pQbody->sCommon.ucKind     = 'T';		/* �܂�Ԃ��e�X�g�v�� */
	pQbody->sCommon.ucTerminal = tno;		/* CRR���No. */
	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_SlaveStatus()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�q�@�����M�����u�q�@��ԃf�[�^�v�̕ω������m����B			   |*/
/*|		�E�Ύq�@�ʐM�^�X�N����L���[�ɂĖ{�^�X�N�֎�M���m�炳��܂��B	   |*/
/*|		�E�{�֐��ɂāA�ԗ��̓��o�Ɍ��m�A�����							   |*/
/*|		  ���b�N���u���쌋�ʂ����m���܂��B								   |*/
/*|		�E���m���ʂ͐��Z�@�֒ʒm���܂��B�i�����ݒ�f�[�^��M��Ɍ���j	   |*/
/*|		�E�u�q�@�o�[�W�����f�[�^�v�̌��m���A�{�֐��ɂčs���܂��B		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_SlaveStatus(t_IFM_Queue *pStatus)
{
	int			lock, slave;
t_IF_LockSensor	now, last, change;
	uchar		lockState, carState;
	uchar		lastStatus, maker;
	BOOL		bChange;
	BOOL		bRetry;
	uchar		recover;

	switch (pStatus->sCommon.ucKind) {
		case 'A':	/* �q�@��ԃf�[�^ */
			slave = pStatus->sCommon.ucTerminal;
			for (lock=0; lock < IFS_ROOM_MAX; lock++) {
				WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
				now         = pStatus->sA41.unSensor[lock];
				last        = IFM_LockTable.sSlave[slave].sLock[lock].unSensor;
				change.Byte = (uchar)((now.Byte ^ last.Byte) & 0x1f);
				maker       = IFM_LockTable.sSlave[slave].sLock[lock].ucConnect;

				/* �ڑ������͏��O */
				if (maker == 0) {
					continue;			/* ���̎Ԏ��� */
				}

				/*
				*	2005.05.20 �����E�ǉ�
				*	�����ݒ�f�[�^��M����́A����Ԓʒm����Ώ�
				*	�E��ԕω��ʒm�ς݃t���O�́A�����ݒ�f�[�^��M����Ƀ��Z�b�g����Ă���B
				*	�E�����ł́A�ω����Ă��悤�����܂����K���u�ʏ��ԃf�[�^�v�𑗂����B
				*	�E���b�N���u���ڑ�����Ă�����̂Ɍ���B
				*/
				if (!IFM_LockTable.sSlave[slave].sLock[lock].bChangeState)		/* ��ԕω����ʒm�H*/
				{
					IFM_LockTable.sSlave[slave].sLock[lock].bChangeState = 1;	/* ��ԕω��ʒm�ς݂� */

					/* ���܂̏�Ԃ�����o�� */
					lockState = LCM_LockState_Initial(now.Byte, maker);
					carState  = LCM_CarState_Initial(now.Byte, maker);
					/* �ω��ɂ������̂ŁA�O�������������i�����H�j*/
					IFM_LockTable.sSlave[slave].sLock[lock].ucCarState  = 0;	/* �����l�� */
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = 0;	/* �����l�� */

					goto CHANGE;		/* �ω������ */
				}

				/* �ω��Ȃ��͏��O */
				if (!change.Byte) {
					continue;			/* ���̎Ԏ��� */
				}

				/* �ω�����͍X�V */
				IFM_LockTable.sSlave[slave].sLock[lock].unSensor = now;

				/*
				*	�ԗ����m��Ԃ��m�肳����B
				*/
				if (change.Bit.B2) {		/* �ԗ����m�Z���T�[��Ԃ��ω������H*/
					carState = LCM_CarState_bySensor(now.Byte, maker, slave, lock);
				}
				else {						/* �ԗ����m�Z���T�[��Ԃ��ω��Ȃ��Ȃ�A�ԗ�����/�ԗ������͌���Ԃ̂܂� */
					carState = IFM_LockTable.sSlave[slave].sLock[lock].ucCarState;
				}

				/*
				*	���b�N���u��Ԃ��m�肳����B
				*	�E���쒆�́A���b�N��ԍX�V�s�v�i���슮����ɕω��ʒm����j
				*	  �������A�ԗ����m�͎󂯕t���X�V�������̂ŁA���̑O�ɍς܂���B
				*/

				/*
				*	���ܓ��쒆�͏��O
				*
				*	�������A
				*	�ȉ��̏���������B�i���Ƃ��킩�����j
				*	2005.06.27	�����E�o�O�C��
				*	�ENo.80)AAA ���ɏ�ԂŎ{�����J��������A��r�I���������Ɂu�o�Ɂv����ƁA
				*	 �u�ԗ������v��ԕω������Z�@�ɒʒm����Ȃ���������B
				*	 �����������A�ԗ��ω����m�͎󂯕t��(��)�X�V�����̂ɁA
				*	  �u���ܓ��쒆�v�̂��߁A���O�̗J���ڂɑ����A�ʒm����Ȃ������B
				*	  �u���ܓ��쒆�v�ł��A�ԗ��ω����m�����́A�ʒm����悤�ɂ���B
				*/
				if (now.Bit.B3) {
					/* ���b�N�Z���T�[��Ԃ܂��̓��b�N�J�Z���T�[��Ԃɕω����������� */
					if((change.Bit.B1 == 1) || (change.Bit.B0 == 1)){
						/* �ғ������J�E���g */
						lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;// LCM_CountAction()���̌̏�񐔂̔���ɕK�v
						LCM_CountAction(lockState, slave, lock);
					}
					/* �ԗ��ω����m�����̂Ȃ�A*/
					if (carState != IFM_LockTable.sSlave[slave].sLock[lock].ucCarState) {
						/* (������)���b�N���u��Ԃ́A�O���ԂŊm�肳���Ă��� */
						lockState = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
						goto CHANGE;		/* �ω������ */
					}
					/* �ԗ��ω��Ȃ��Ȃ�A(���̂Ƃ���)���O */
					else {
						continue;			/* ���̎Ԏ��� */
					}
				}

				lastStatus = IFM_LockTable.sSlave[slave].sLock[lock].ucLockState;
				/* ���슮���ɂȂ����i����𔺂����́j*/
				if (change.Bit.B3 &&		/* ���b�N���u�����Ԃ� */
					  !now.Bit.B3)			/* ���쒆(1)�����슮��(0)�֕ω������H*/
				{
					/* ���슮�������̂ŁD�D�D*/
					IFM_ResetAction((uchar)slave, (uchar)lock);	/* ���쒆������ */
					if (IFM_LockTable.usWaitCount) {
						IFM_LockTable.usWaitCount--;				/* ���슮���҂�����߂��i�J�E���g�_�E���j*/
					}
					LCMtim_10msTimerStop(slave, lock);			// �����ŁA���슮������܂ł̃^�C�}�[���� 

					/* �{��/�J���Z���T�[��ԂŁu����/�̏�v�𔻒肷�� */
					if (now.Bit.B4 == 0) {
						lockState = LCM_Is_LockOnTrouble( &bRetry, now.Byte, lastStatus, maker);	/* �{������ɑ΂��āA�̏ᔻ�� */
					}
					else {
						lockState = LCM_Is_LockOffTrouble(&bRetry, now.Byte, lastStatus, maker);	/* �J������ɑ΂��āA�̏ᔻ�� */
					}

					/* �J�e�X�g�̌��ʔ��� */
					if (IFM_LockTable.bTest) {
						LCM_TestResult(lockState, (int)slave, lock);
					}

					/* ���g���C�i�̏�m��/�̏Ⴉ���j�̏��� */
					if (bRetry &&					/* ���g���C����̂́A*/
						(!IFM_LockTable.bTest))		// �J�e�X�g�ł͖���
					{
						/* �̏჊�g���C�̂��߂̃��g���C�Ԋu���ԂŃ^�C�}�[�J�n */
						if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	// ���g���C
						{
							LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	// ��������A���g���C���ԃ^�C�}�[�J�n 
						}
					}

					/* ���슮�������̂ŁA����҂�������Ύ���v������ */
					LCM_NextAction();
				}
				/*
				*	�O�X���瓮�슮���̂܂܁i����𔺂�Ȃ��j�ȉ��̉\������
				*	�E�{���Z���T�[���ω������E�E�E�E�E�E�̏�H
				*	�E�J���Z���T�[���ω������E�E�E�E�E�E�̏�H
				*	�E���b�N���u����������ω������E�E�E����𔺂�Ȃ��ω��́A���肦�Ȃ�
				*	�E�{��/�J���Z���T�[�ω��Ȃ� �E�E�E�E�ԗ��Z���T�݂̂̕ω��������B
				*/
				else {
					lockState = LCM_LockState_bySensor(&IFM_LockTable.sSlave[slave].sLock[lock].ucLockState, maker, last.Byte, now.Byte, &recover);
					/*
					*	2005.05.17 �����E�ǉ�
					*	���J�o�[���Ȃ����H����Ȃ�A�ǂ��������̃��J�o�[���H
					*	�urecover�v�l�Ŕ���
					*		=0�F���J�o�[���Ȃ�
					*		=1�F�{�����샊�J�o�[
					*		=2�F�J�����샊�J�o�[
					*/
					if (recover) {
						/* ���b�N����ł��邩�H�ۂ��𔻒肵�A�ۂȂ�҂����鏈���i�L���[�C���O�j*/
						LCM_WaitingLock(recover, (uchar)slave, (uchar)lock, maker, 0, (t_LKcomCtrl *)0);
					}
				}
CHANGE:
				/* ���b�N�Ǘ��e�[�u���̂���ƕς�����H*/
				bChange = 0;
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucCarState != carState) {
					IFM_LockTable.sSlave[slave].sLock[lock].ucCarState = carState;		/* �ς�����̂ōX�V */
					bChange = 1;
				}
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucLockState != lockState) {
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = lockState;	/* �ς�����̂ōX�V */
					bChange = 1;
				}

				/* �O��ƕς��ĂȂ���Ώ��O */
				if (!bChange) {
					continue;			/* ���̎Ԏ��� */
				}

				if (!IFM_LockTable.bTest)	// �J�e�X�g�����A���O
				{
					LCM_NoticeStatus_toPAY((uchar)(slave+1), (uchar)(lock+1), carState, lockState);		// ���Z�@�@�ցu��ԕω��v��m�点��
				}
			}
			break;

		case 'V':	/* �q�@�o�[�W�����f�[�^ */
			/*
			*	2005.05.20 �����E�ǉ�
			*	�o�[�W�����v���E�����Ώ�
			*/
			slave = pStatus->sCommon.ucTerminal;
			memcpy(IFM_LockTable.sSlave[slave].cVersion, pStatus->sV56.ucVersion, sizeof(IFM_LockTable.sSlave[slave].cVersion));
			if (IFM_LockTable.sSlave[slave].ucContinueCnt) {
				IFM_LockTable.sSlave[slave].ucContinueCnt--;
			}
			if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)slave, 0)) {
				LCMtim_1secWaitStart(0);	/* �O�Z�b�g���ċ����Ƀ^�C���A�E�g���N�������� */
			}
			break;

		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CountAction()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�ғ������J�E���g												   |*/
/*|		�E����v���ɑ΂��鉞�����Ԃ��Ă����Ƃ��ɃJ�E���g����B			   |*/
/*|		  �Ȃ̂ŁA������ӏ��̂݁B�^�C���A�E�g���̓J�E���g�����B		   |*/
/*|		�E�J����̍��v�񐔁i�����E�蓮�̍��v�A���ĊJ�����2��Ɛ�����j|*/
/*|		�EI/F��SW�ɂ��蓮�ł̊J�E���썇�v�񐔁i�����e�i���X����j	   |*/
/*|		�E�̏Ⴕ�����v�񐔁i���~�b�gSW��Ԃ��ω����Ȃ����j				   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_CountAction(uchar lockState, int slave, int lock)
{
	/* ����A�̏�Ɋւ�炸 */
	IFM_LockAction.sSlave[slave].sLock[lock].ulAction += 1;			/* �J����̍��v�� */

	/* �̏ᔻ��̌��ʂ��� */
	if (lockState == 3 ||										/* �{������Ō̏�A�܂���*/
		lockState == 4)											/* �J������Ō̏� */
	{
		IFM_LockAction.sSlave[slave].sLock[lock].ulTrouble++;	/* �̏Ⴕ�����v�� */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CarState_Initial()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���o�ɔ���i�ԗ��Z���T�[���ω��j								   |*/
/*|		�E�{��/�J���Z���T�[�ω��ɂ����o�ɔ��������B					   |*/
/*|		�E�u�C�j�V��������v�ł̋N������́A�Z���T�[�ω��ł͖����Z���T�[�l |*/
/*|		  �݂̂œ��o�ɔ��肵�Ȃ���΂Ȃ�Ȃ��B							   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			0 = �ڑ��Ȃ��i�o�O�H�j										   |*/
/*|			1 = ���Ɂi�ԗ�����j										   |*/
/*|			2 = �o�Ɂi�ԗ������j										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_CarState_Initial(uchar ucNow, uchar ucMaker)
{
t_IF_LockSensor	now;
	uchar		carState;

	/* �P���Ɂu�ԗ��Z���T�[�v�݂̂Ŕ���\ */
	now.Byte = ucNow;
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:
		case LOCK_AIDA_bike:		/* �p�c�o�C�N�́w���o�ɔ���x�́A�q�@���ŋz������	2005.05.24 �����E�ǉ� */
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
		case LOCK_HID_both:
			if (now.Bit.B2) {		/* ���܎ԗ�����Ȃ�A*/
				carState = 1;		/* ���� */
			} else {				/* ���܎ԗ������́A*/
				carState = 2;		/* �o�� */
			}
			break;

		default:
			carState = 0;			/* �ڑ��Ȃ� */
			break;
	}

	return carState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_LockState_Initial()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		��������i�{/�J�Z���T�[���ω��j									   |*/
/*|		�E�{��/�J���Z���T�[���ω��ɂ���Ԕ��������B					   |*/
/*|		�E�u�C�j�V��������v�ł̋N������́A�Z���T�[�ω��ł͖����Z���T�[�l |*/
/*|		  �݂̂ŏ�Ԕ��肵�Ȃ���΂Ȃ�Ȃ��B							   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			1 = �{���ς݁i����j										   |*/
/*|			2 = �J���ς݁i����j										   |*/
/*|			3 = �{���ُ�i�̏�j										   |*/
/*|			4 = �J���ُ�i�̏�j										   |*/
/*|			6 = �w�胍�b�N���u �ڑ��Ȃ��i�o�O�H�j						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_LockState_Initial(uchar ucNow, uchar ucMaker)
{
t_IF_LockSensor	now;
	uchar		lockState;

	/* ���[�J�[�ɂ�蔻�肪�قȂ� */
	now.Byte = ucNow;
	switch (ucMaker) {

		/* �{��/�J�����ɃZ���T�[���� */
		case LOCK_AIDA_bicycle:
			if (now.Bit.B0 ^ now.Bit.B1) {
				if (now.Bit.B0) {
					lockState = 1;	/* �{���ς݁i����j*/
				} else {
					lockState = 2;	/* �J���ς݁i����j*/
				}
			}
			else if (now.Bit.B0 & now.Bit.B1) {
				lockState = 3;		/* �{���ُ�i�̏�j�E�E�E����ON */
			} else {
				lockState = 4;		/* �J���ُ�i�̏�j�E�E�E����OFF */
			}
			break;

		/* �{��/�J�����ɃZ���T�[���� */
		case LOCK_AIDA_bike:
		case LOCK_HID_both:
			lockState = 2;			/* �J���ς݁i����j*/
			break;

		/* �{���Z���T�[�݂̂���i�J���Z���T�[�����j*/
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
			if (now.Bit.B0) {
				lockState = 1;		/* �{���ς݁i����j*/
			} else {
				lockState = 2;		/* �J���ς݁i����j*/
			}
			break;

		default:
			lockState = 6;			/* �w�胍�b�N���u �ڑ��Ȃ� */
			break;
	}

	return lockState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_CarState_bySensor()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���o�ɔ���i�ԗ��Z���T�[�ω��j									   |*/
/*|		�E�ԗ��Z���T�[�ω��ɂ����o�ɔ��������B						   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			0 = �ڑ��Ȃ��i�o�O�H�j										   |*/
/*|			1 = ���Ɂi�ԗ�����j										   |*/
/*|			2 = �o�Ɂi�ԗ������j										   |*/
/*|			3 = �����o�Ɂi�ԗ������B�����e�i���X���[�h���蓮�ŏo�ɂ������j |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_CarState_bySensor(uchar ucNow, uchar ucMaker, int iSlave, int iLock)
{
t_IF_LockSensor	now;
	uchar		carState;

	/* �P���Ɂu�ԗ��Z���T�[�v�݂̂Ŕ���\ */
	now.Byte = ucNow;
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:
		case LOCK_AIDA_bike:		/* �p�c�o�C�N�́w���o�ɔ���x�́A�q�@���ŋz������	2005.05.24 �����E�ǉ� */
		case LOCK_YOSHIMASU_both:
		case LOCK_KOMUZU_bicycle:
		case LOCK_HID_both:
			if (now.Bit.B2) {		/* (�ԗ�������)�ԗ����� �ւ̕ω��Ȃ�A*/
				carState = 1;		/* ���� */
			} else {				/* (�ԗ����聨)�ԗ����� �ւ̕ω��́A*/
				carState = 2;		/* �Ƃ肠�����A�o�� */
			}
			break;

		default:
			carState = 0;			/* �ڑ��Ȃ� */
			break;
	}

	/*
	*	�����o�ɂ̔���i�ȉ���AND�����j
	*	�E�o�ɂƔ��肵����A
	*	�E�����ݒ�f�[�^��M���
	*	�E(���O�̓��삪)�J�����삾����
	*/
	if (carState == 2 &&
		now.Bit.B4)
	{
		carState = 3;	/* �蓮���[�h�ŋ����o�� */

		/* ���b�N���u�����e�i���X���엚�����X�V		2005.06.07 �����E�d�l�ύX */
		IFM_LockTable.sSlave[iSlave].sLock[iLock].ucActionMainte = 3;
	}

	return carState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_LockState_bySensor()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�̏ᔻ��i�ҋ@���̎{/�J�Z���T�[�ω��j							   |*/
/*|		�E�J����𔺂�Ȃ��{��/�J���Z���T�[�ω��ɂ��̏ᔻ�蔻�������B|*/
/*|		�E�u�p�c���]�ԁv�Ɍ���B�i���ԏ�̃t���b�v���������Ă���j	   |*/
/*|		�E�ҋ@���Ɂu�t���b�v�v�͓��܂ꂽ�肵�Ă��̈ʒu�������ƂȂ�\�� |*/
/*|		  ������A������w�z�[���|�W�V�����x�ɖ߂����Ƃ����J�o�[�@�\�Ƃ��� |*/
/*|		  �����Ă��������炵���B		  								   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			1 = �{���ς݁i����j										   |*/
/*|			2 = �J���ς݁i����j										   |*/
/*|			3 = �{���ُ�i�̏�j										   |*/
/*|			4 = �J���ُ�i�̏�j										   |*/
/*|			5 = �̏����												   |*/
/*|			6 = �w�胍�b�N���u �ڑ��Ȃ��i�o�O�H�j						   |*/
/*|		�E��T�����upiRecover�v�ɂāA�ȉ���Ԃ�							   |*/
/*|			0= ���J�o�[���Ȃ�											   |*/
/*|			1= ���J�o�[�{������											   |*/
/*|			2= ���J�o�[�J������											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_LockState_bySensor(uchar *pucLastState, uchar ucMaker, uchar ucLast, uchar ucNow, uchar *piRecover)
{
t_IF_LockSensor	after, before;
	uchar		newState;

	*piRecover = 0;					/*�u���J�o�[���Ȃ��v�ɏ����� */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* �p�c���]�� */
			break;

		default:					/* �ȊO�͑ΏۊO */
			return *pucLastState;	/* ��Ԃ͕ς����Ƀ��^�[�� */
	}

	before.Byte = ucLast;
	after.Byte  = ucNow;
	newState    = *pucLastState;	/* �V��Ԃ́u�O���ԁv����ɂ��� */
	switch (*pucLastState) {
		case 1:		/* ���b�N���u�ς݁i����j	*/
		case 2:		/* ���b�N���u�J�ς݁i����j	*/
		case 5:		/* �̏����					*/
			/* �ω��O���������i�{=1/�J=0�j*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 0) {		/* �{��0 */
					*piRecover = 1;				/* �������J�o�[�u�{������v*/
				}
				else if (after.Bit.B1 == 1) {	/* �J��1 */
					newState = 3;				/* �{������ُ�i�̏�j�� */
				}
			}
			/* �ω��O���J�������i�{=0/�J=1�j*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B1 == 0) {		/* �J��0 */
					*piRecover = 2;				/* �������J�o�[�u�J������v*/
				}
				else if (after.Bit.B0 == 1) {	/* �{��1 */
					newState = 4;				/* �J������ُ�i�̏�j�� */
				}
			}
			break;

		case 3:		/* �{������ُ�i�̏�j		*/
			/* �ω��O������ON�������i�{=1/�J=1�j*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B0 == 0) {		/* �{��0 */
					*piRecover = 1;				/* �������J�o�[�u�{������v*/
				}
				else if (after.Bit.B1 == 0) {	/* �J��0 */
					newState = 5;				/* �̏�����i�{�j�� */
				}
			}
			/* �ω��O������OFF�������i�{=0/�J=0�j*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B1 == 1) {		/* �J��1 */
					newState = 3;				/* �{������ُ�̂܂܂����ǁA��Ԓʒm���������� */
					*pucLastState = 0;			/* �O���Ԃ�ς���i�����H�j*/
				}
				else if (after.Bit.B0 == 1) {	/* �{��1 */
					newState = 5;				/* �̏�����i�{�j�� */
				}
			}
			/* �ω��O���t�������i�{=0/�J=1�j*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B0 == 1 ||		/* �{��1 */
					after.Bit.B1 == 0)			/* �J��0 */
				{
					newState = 3;				/* �{������ُ�̂܂܂����ǁA��Ԓʒm���������� */
					*pucLastState = 0;			/* �O���Ԃ�ς���i�����H�j*/
				}
			}
			break;

		case 4:		/* �J������ُ�i�̏�j		*/
			/* �ω��O������ON�������i�{=1/�J=1�j*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 1)
			{
				if (after.Bit.B1 == 0) {		/* �J��0 */
					*piRecover = 2;				/* �������J�o�[�u�J������v*/
				}
				else if (after.Bit.B0 == 0) {	/* �{��0 */
					newState = 5;				/* �̏�����i�J�j�� */
				}
			}
			/* �ω��O������OFF�������i�{=0/�J=0�j*/
			if (before.Bit.B0 == 0 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 1) {		/* �{��1 */
					newState = 4;				/* �J������ُ�̂܂܂����ǁA��Ԓʒm���������� */
					*pucLastState = 0;			/* �O���Ԃ�ς���i�����H�j*/
				}
				else if (after.Bit.B1 == 1) {	/* �J��1 */
					newState = 5;				/* �̏�����i�J�j�� */
				}
			}
			/* �ω��O���t�������i�{=1/�J=0�j*/
			if (before.Bit.B0 == 1 &&
				before.Bit.B1 == 0)
			{
				if (after.Bit.B0 == 0 ||		/* �{��0 */
					after.Bit.B1 == 1)			/* �J��1 */
				{
					newState = 4;				/* �J������ُ�̂܂܂����ǁA��Ԓʒm���������� */
					*pucLastState = 0;			/* �O���Ԃ�ς���i�����H�j*/
				}
			}
			break;

		default:
			break;
	}

	return newState;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockOnTrouble()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�{������̏ᔻ��												   |*/
/*|		�E�{������ɑ΂��āA�̏ᔻ�������B							   |*/
/*|		�E���[�J�[�ɂ���āA�ȉ��̗�O����B							   |*/
/*|			1)�p�c�o�C�N�̎{���Z���T�[�́A�s�������������Ŕ�����		   |*/
/*|			  ���b�N����Ƃ͘A�����Ȃ��B								   |*/
/*|			2)HID �͎{��/�J���Z���T�[����������B						   |*/
/*|		  ����āA���̂Q���[�J�[�́u����(�̏�Ȃ�)�v�Ƃ���B			   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			1 = �{������ς݁i����j									   |*/
/*|			3 = �{������i�̏�j										   |*/
/*|			5 = �̏����												   |*/
/*|		���{���Z���T�[�A�J���Z���T�[�̉��ꂩ���������b�N���u�́A		   |*/
/*|		  �{���Z���T�[��ON�ł��A										   |*/
/*|		  ���g���C�̑Ώہi=1�j��Ԃ��B									   |*/
/*|		  �������A														   |*/
/*|		  �߂�l�́u���b�N���u��ԁv���A1=�{������ς݁i����j�ŕԂ��B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_Is_LockOnTrouble(BOOL *pbRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker)
{
t_IF_LockSensor	sensor;
	uchar		newStatus;

	/* �Z���T�[��Ԃ�ޔ� */
	sensor.Byte = ucSensor;

	/* ���b�N���u���[�J�[�ɂ��A���݂���Z���T�[���قȂ� */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* �p�c���]�� */
			*pbRetry  = 0;			/* ���g���C�Ȃ��ŏ����� */
			break;					/* �̏ᔻ��� */

		/* �p�c�o�C�N�AHID �͌̏�Ȃ��E�E�E��O */
		case LOCK_AIDA_bike:		/* �p�c�o�C�N�F���p�҂����g�ō����s���Ȃ̂ł����܂� */
		case LOCK_HID_both:			/* HID�F       �{���A�J�����Z���T�[�Ƃ����� */
			*pbRetry = 1;			/* ���g���C�Ώہi�̏Ⴉ���j�ɏ�����		2005.05.16 �����E�ύX */
			return 1;				/* �{���ς݁i����j�ɂ��邵���Ȃ��E�E�E�̏ᔻ��ł��Ȃ��̂ő����^�[�� */

		case LOCK_YOSHIMASU_both:	/* �g�� */
		case LOCK_KOMUZU_bicycle:	/* �R���Y */
			sensor.Bit.B1 = 0;		/* �J���Z���T�[�����Ȃ̂Łu0�v�ɂ��Č̏ᔻ��֗Ղ� */
			*pbRetry = 1;			/* ���g���C�Ώہi�̏Ⴉ���j�ɏ�����		2005.05.16 �����E�ύX */
			break;					/* �̏ᔻ��� */

		default :
			*pbRetry = 0;			/* ���g���C���邱�Ƃ͖��Ӗ� */
			return 3;				/* �{������ُ� */
	}

	/*
	*	�̏ᔻ��
	*	���{�����슮���オ�O��B
	*	�E�{���Z���T�[ON �ł��邱�ƁE�E�E�K�{
	*	�E�J���Z���T�[OFF�ł��邱�ƁE�E�E�C��
	*/
	if (sensor.Bit.B0 == 1 &&		/* �{���Z���T�[ */
		sensor.Bit.B1 == 0)			/* �J���Z���T�[ */
	{
		if (ucLastStatus == 3 ||	/* �O��A�{������ُ킩�H�܂��́A*/
			ucLastStatus == 4)		/* �O��A�J������ُ�Ȃ�A*/
		{
			newStatus = 5;			/* �̏���� */
		} else {
			newStatus = 1;			/* �{���ς݁i����j*/
		}
	}
	else {
		newStatus = 3;				/* �{������ُ� */
		*pbRetry  = 1;				/* ���g���C�Ώہi�̏�m��j*/
	}

	return newStatus;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockOffTrouble()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�J������̏ᔻ��												   |*/
/*|		�E�J������ɑ΂��āA�̏ᔻ�������B							   |*/
/*|		�E���[�J�[�ɂ���āA�ȉ��̗�O����B							   |*/
/*|			1)�p�c�o�C�N�̎{���Z���T�[�́A�s�������������Ŕ�����		   |*/
/*|			  ���b�N����Ƃ͘A�����Ȃ��B								   |*/
/*|			2)HID �͎{��/�J���Z���T�[����������B						   |*/
/*|		  ����āA���̂Q���[�J�[�́u����(�̏�Ȃ�)�v�Ƃ���B			   |*/
/*|		�E�߂�l�i���ʁj�͈ȉ�											   |*/
/*|			2 = �J������ς݁i����j									   |*/
/*|			4 = �J������i�̏�j										   |*/
/*|			5 = �̏����												   |*/
/*|		���{���Z���T�[�A�J���Z���T�[�̉��ꂩ���������b�N���u�́A		   |*/
/*|		  �{���Z���T�[��OFF�ł��A										   |*/
/*|		  ���g���C�̑Ώہi=1�j��Ԃ��B									   |*/
/*|		  �������A														   |*/
/*|		  �߂�l�́u���b�N���u��ԁv���A2=�J������ς݁i����j�ŕԂ��B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_Is_LockOffTrouble(BOOL *piRetry, uchar ucSensor, uchar ucLastStatus, uchar ucMaker)
{
t_IF_LockSensor	sensor;
	uchar		newStatus;

	/* �Z���T�[��Ԃ�ޔ� */
	sensor.Byte = ucSensor;

	/* ���b�N���u���[�J�[�ɂ��A���݂���Z���T�[���قȂ� */
	switch (ucMaker) {
		case LOCK_AIDA_bicycle:		/* �p�c���]�� */
			*piRetry  = 0;			/* ���g���C�Ȃ��ŏ����� */
			break;					/* �̏ᔻ��� */

		/* �p�c�o�C�N�AHID �͌̏�Ȃ��E�E�E��O */
		case LOCK_AIDA_bike:		/* �p�c�o�C�N�F���p�҂����g�ō����s���Ȃ̂ł����܂� */
		case LOCK_HID_both:			/* HID�F       �{���A�J�����Z���T�[�Ƃ����� */
			*piRetry = 1;			/* ���g���C�Ώہi�̏Ⴉ���j�ɏ�����		2005.05.16 �����E�ύX */
			return 2;				/* �J���ς݁i����j�ɂ��邵���Ȃ��E�E�E�̏ᔻ��ł��Ȃ��̂ő����^�[�� */

		case LOCK_YOSHIMASU_both:	/* �g�� */
		case LOCK_KOMUZU_bicycle:	/* �R���Y */
			sensor.Bit.B1 = 1;		/* �J���Z���T�[�����Ȃ̂Łu1�v�ɂ��Č̏ᔻ��֗Ղ� */
			*piRetry  = 1;			/* ���g���C�Ώہi�̏Ⴉ���j�ɏ����� */
			break;					/* �̏ᔻ��� */

		default :
			*piRetry = 0;			/* ���g���C���邱�Ƃ͖��Ӗ� */
			return 4;				/* �J������ُ� */
	}

	/*
	*	�̏ᔻ��
	*	���J�����슮���オ�O��B
	*	�E�{���Z���T�[OFF�ł��邱�ƁE�E�E�K�{
	*	�E�J���Z���T�[ON �ł��邱�ƁE�E�E�C��
	*/
	if (sensor.Bit.B0 == 0 &&		/* �{���Z���T�[ */
		sensor.Bit.B1 == 1)			/* �J���Z���T�[ */
	{
		if (ucLastStatus == 3 ||	/* �O��A�{������ُ킩�H�܂��́A*/
			ucLastStatus == 4)		/* �O��A�J������ُ�Ȃ�A*/
		{
			newStatus = 5;			/* �̏���� */
		} else {
			newStatus = 2;			/* �J���ς݁i����j*/
		}
	}
	else {
		newStatus = 4;				/* �J������ُ� */
		*piRetry  = 1;				/* ���g���C�Ώہi�̏�m��j*/
	}

	return newStatus;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_LockDirNow()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�������������													   |*/
/*|		�E���܁A�{�������ɂ��邩�H���u��������r�b�g�v���画�肷��B	   |*/
/*|		�E�J�e�X�g���A�ǂ���̕����ɓ��삳���邩�H�̔���Ɏg�p����B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�߂�l�F															   |*/
/*|			=1 �{�������ɂ���B											   |*/
/*|			=0 �J�������ɂ���B											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	LCM_Is_LockDirNow(uchar ucSlave, uchar ucLock)
{
	BOOL	nowDir;

	/* .ucAction�́A���O�́u���b�N���u���엚���v������ */
	/*	���̒l�̈Ӗ��́A*/
	/*		=0�F���̓���v�������Ă��Ȃ� */
	/*		=1�F�{���i�j����v������ */
	/*		=2�F�J���i�J�j����v������ */
	switch (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction) {
		case 1:		/* ���O�Ɂu�{���i�j����v�������v�Ȃ�A*/
			nowDir = 1;		/*�u�{�������ɂ���v�� */
			break;

		case 2:		/* ���O�Ɂu�J���i�J�j����v�������v�Ȃ�A*/
			nowDir = 0;		/*�u�J�������ɂ���v�� */
			break;

		case 0:		/*�u���̓���v�������Ă��Ȃ��v���A*/
		default:	/* ��L�ȊO�Ȃ�A*/
			/* ���u���b�N��ԁv���画�f���邵���Ȃ� */
			/*	���u�C�j�V��������v�ŗ����オ��A�܂���x�����b�N������{���Ă��Ȃ��P�[�X��z�� */
			switch (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucLockState) {
				case 1:		/* ���b�N���u�ς݁i����j	*/
				case 3:		/* ����ُ�i�̏�j		*/
					nowDir = 1;		/*�u�{�������ɂ���v�� */
					break;

				case 2:		/* ���b�N���u�J�ς݁i����j	*/
				case 4:		/* �J����ُ�i�̏�j		*/
					nowDir = 0;		/*�u�J�������ɂ���v�� */
					break;

				case 5:		/* �̏����					*/
				case 6:		/* �w�胍�b�N���u�ڑ�����	*/
				default:	/* ��L�ȊO */
									/* ���蓾�Ȃ��P�[�X�Ȃ̂ŁA*/
					nowDir = 0;		/*�u�J�������ɂ���v�Ɗ���؂� */
					break;
			}
			break;
	}

	return	nowDir;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_do_ActionNow()                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���쐧��\����												   |*/
/*|		�E�w�胍�b�N���u�i=�Ԏ��j�ɑ΂��āA���쐧��\���H�ۂ��𔻒肷��B|*/
/*|		�E��ɁA�J�e�X�g���Ɏg�p����B								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�߂�l�F															   |*/
/*|			=2 ���ܐ���s�i�ʐM���Q���Ȃ̂Łj							   |*/
/*|			=1 ����\													   |*/
/*|			=0 ����s��													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	LCM_do_ActionNow(int iSlave, int iLock)
{
	/*
	*	�ȉ��͏��O�B
	*	�E�ʐM����K�v���Ȃ��q�@�i���b�N���u���ڑ������j
	*	�E�����ݒ�f�[�^�𑗂����̂�ACK�������Ă��Ȃ��q�@
	*/
	if (!IFM_LockTable.sSlave[iSlave].bComm ||
		!IFM_LockTable.sSlave[iSlave].bInitialACK)
	{
		return 0;
	}
	else {
		/* �������X�L�b�v���̎q�@�́u���܂͓���ł��Ȃ��v*/
		if (IFM_LockTable.sSlave[iSlave].bSkip) {
			return 2;
		}

		/* ���b�N�ڑ��Ȃ��́u����ł��Ȃ��v*/
		if (!IFM_LockTable.sSlave[iSlave].sLock[iLock].ucConnect) {
			return 0;
		}
		/* ���b�N�ڑ�����́u����ł���v*/
		if (IFM_LockTable.sSlave[iSlave].sLock[iLock].ucConnect < LOCK_MAKER_END) {
			return 1;
		}
		/* ���[�J�[�s�����u����ł��Ȃ��v*/
		else {
			return 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NextAction()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���̓���v��													   |*/
/*|		�E����҂�������Ύ���v������B								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NextAction(void)
{
t_IFM_Queue			*pQbody;
struct t_IF_Queue	*pQbuff;
	uchar			action;
	BOOL			bLockOff;
	uchar			ucSlave, ucLock, ucMaker;

	/* �L���[(�擪)���O�� */
	pQbuff = DeQueue2(&LCMque_Ctrl_Action);
	if (pQbuff) {
		pQbody = pQbuff->pData;
		ucSlave = pQbody->sW57.sCommon.ucTerminal;
		ucLock  = pQbody->sW57.ucLock;
		ucMaker = pQbody->sW57.ucMakerID;

		if( LKcom_Search_Ifno( ucSlave + 1 ) == 0 ){		// ���b�N���u
			/* (0)�{��/(1)�J���A������̓��삩�H*/
			if (pQbody->sW57.unCommand.Bit.B4 == 0) {
				bLockOff = 0;	/*�u���b�N����!�v���q�@�֗v�� */
				action   = 1;	/* �{��[��]����v������ */
			} else {
				bLockOff = 1;	/*�u���b�N��������!�v���q�@�֗v�� */
				action   = 2;	/* �J��[�J]����v������ */
			}
		}
		else {			// �t���b�v
			if (pQbody->sW57.unCommand.Byte == '4') {		// �t���b�v�㏸
				bLockOff = 0;
				action   = 1;
			}
			else if(pQbody->sW57.unCommand.Byte == '5') {	// �t���b�v���~
				bLockOff = 1;
				action   = 2;
			}
			else if(pQbody->sW57.unCommand.Byte == '1') {	// ���[�v����OFF
				bLockOff = 3;
				action   = 3;
			}
			else {											// ���[�v����ON
				bLockOff = 2;
				action   = 4;
			}
		}

		/* �O�����L���[�́u�M���o�͎��ԁv���̗p */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime = pQbody->sW57.usLockOnTime;
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime = pQbody->sW57.usLockOffTime;

		/* �q�@�ʐM�^�X�N�ցu��ԏ������݃f�[�^�v���[���𔭐M */
		LCM_ControlLock(0, bLockOff, ucSlave, ucLock, ucMaker);
		IFM_SetAction(ucSlave, ucLock);											/* ����v�������̂ŁA���쒆�� */

		/*
		*	���܁u���b�N/�����v�������b�N���u�ɑ΂��āA�e�[�u���}�[�N�B
		*	���q�@��ԃf�[�^��Ԃ���A���슮���𔻒肵����}�[�N�O��
		*	�E���슮���҂������J�E���g�A�b�v
		*	�E�u���b�N��ԁv�𓮍쒆��(=0)
		*	�E�����敪�i���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩�j
		*	�E���엚���i�{��[��]/�J��[�J]����̓���v�����������j
		*	�E�̏᎞���g���C��
		*		����Ȃ�A���g���C�񐔂�������
		*		���g���C�Ȃ�A���g���C�񐔂��J�E���g�_�E���i���g���C����K�v�����邩��{�֐����Ăяo���ꂽ�̂Łj
		*/
		IFM_LockTable.usWaitCount++;											/* ���슮���҂������J�E���g */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucOperation = pQbody->sW57.ucOperation;	/* ���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩 */
		IFM_LockTable.sSlave[ucSlave].sLock[ucLock].ucAction    = action;		/* �{��[��]/�J��[�J]����̓���v���������� */
		if (pQbody->sW57.bWaitRetry == 0) {										/* ����́A�̏᎞���g���C�񐔏����� */
			IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry = IFM_LockTable.sSlave[ucSlave].usRetryCount;
		}
		else {																	/* ���g���C�Ȃ̂ŁA*/
			/* ���g���C�񐔂��u99�v��ȏ�́A�������g���C�Ȃ̂Ő�����K�v�Ȃ� */
			if (IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry != 0 &&		/*�u=0�v�̂Ƃ��͏��O	2005.07.12 �����E�ǉ� */
				IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry < 99)
			{
				IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usRetry--;			/* ���g���C�񐔂𐔂��� */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutAction()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���슮���^�C���A�E�g											   |*/
/*|		�E�{�֐��ɂĎq�@�ւ̃��b�N���u����v���ŁA���슮�����Ȃ��������A   |*/
/*|		  �ۂ��H�𔻒肵�܂��B											   |*/
/*|		�E���슮�����Ȃ�������A�̏�Ɣ��f���܂��B						   |*/
/*|		�E�̏�𐸎Z�@�֒ʒm���܂��B									   |*/
/*|		�E�̏჊�g���C�Ԋu���ԂŃ^�C�}�[�N�����܂��B					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutAction(void)
{
	int		slave, lock;
	uchar	lockState;
	BOOL	bChange;

	/* �S�^�C�}�[������ */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
			if (LCMtim_10msTimeout(slave,lock)) {
				LCMtim_10msTimerStop(slave,lock);						/* ���̓��슮���^�C�}�[��~ */

				/* �{��[��]����v�����Ă����̂Ȃ�A*/
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucAction == 1) {
					lockState = 3;			/* �{������ُ� */
				}	
				/* �J��[�J]����v�����Ă����̂Ȃ�A*/
				else if (IFM_LockTable.sSlave[slave].sLock[lock].ucAction == 2) {
					lockState = 4;			/* �J������ُ� */
				}
				else {
					continue;
				}

				/* �J�e�X�g�̌��ʔ��� */
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, slave, lock);
				}

				/* �ғ������J�E���g */
				LCM_CountAction(lockState, slave, lock);

				/* �ω�����/�����𔻒� */
				if (IFM_LockTable.sSlave[slave].sLock[lock].ucLockState == lockState) {
					bChange = 0;
				} else {
					bChange = 1;
					IFM_LockTable.sSlave[slave].sLock[lock].ucLockState = lockState;
				}

				/* �ω��ʒm�i���Z�@�֒m�点��j�̏��� */
				if (!IFM_LockTable.bTest)		// �J�e�X�g���́A���O
				{

					if (bChange) {
						/* ���Z�@�֌̏��m�点�� */
						LCM_NoticeStatus_toPAY(	(uchar)slave + 1, (uchar)lock + 1,
												IFM_LockTable.sSlave[slave].sLock[lock].ucCarState,
												IFM_LockTable.sSlave[slave].sLock[lock].ucLockState);
					}
					/* �̏჊�g���C�̂��߂̃��g���C�Ԋu���ԂŃ^�C�}�[�N�� */
					if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	/* ���g���C */
					{
						LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	/* ��������A���g���C���ԃ^�C�}�[�J�n */
					}
				}

				/*
				*	2005.08.25 �����E�ǉ�
				*	���́u�V���ȓ���v���v���󂯕t���邽�߁A
				*	���슮�����Ȃ������v�����������B
				*/
				/* ���슮�������B���Ƃɂ������̂ŁD�D�D*/
				IFM_ResetAction((uchar)slave, (uchar)lock);	/* ���쒆������ */
				if (IFM_LockTable.usWaitCount) {
					IFM_LockTable.usWaitCount--;				/* ���슮���҂�����߂��i�J�E���g�_�E���j*/
				}
				LCMtim_10msTimerStop(slave, lock);			/* �����ŁA���슮������܂ł̃^�C�}�[���� */
				/* �J�e�X�g�̌��ʔ��� */
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, slave, lock);
				}

				/* ���g���C�̏��� */
				if (!IFM_LockTable.bTest &&								/* �J�e�X�g�ł͖��� */
					IFM_LockTable.sSlave[slave].sLock[lock].usRetry)	/* ���g���C�񐔂��c���Ă���*/
				{
					LCMtim_1secTimerStart(slave, lock, IFM_LockTable.sSlave[slave].usRetryInterval);	/* ��������A���g���C���ԃ^�C�}�[�J�n */
				}

				/* �ғ������J�E���g */

				/* ���슮�������̂ŁA����҂�������Ύ���v������ */
				LCM_NextAction();
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutTrouble()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�̏჊�g���C�Ԋu�^�C���A�E�g									   |*/
/*|		�E�{�֐��ɂĎq�@�ւ̃��b�N���u����v���ŁA�̏჊�g���C�Ԋu���Ԃ�   |*/
/*|		  �o�߂������A�ۂ��H�𔻒肵�܂��B								   |*/
/*|		�E�o�߂�����A���b�N����v�������g���C���܂��B					   |*/
/*|		�E���b�N����ł��邩�H�ۂ��𔻒肵�A							   |*/
/*|		  �ۂȂ�҂����鏈���i�L���[�C���O�j���܂��B					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutTrouble(void)
{
	int		slave, lock;

	/* �S�^�C�}�[������ */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
			if (LCMtim_1secTimeout(slave, lock)) {
				LCMtim_1secTimerStop(slave, lock);				/* ���̃��g���C�Ԋu�^�C�}�[��~ */

				/* ���g���C����H */
				if (IFM_LockTable.sSlave[slave].sLock[lock].usRetry) {
					/* ���b�N����ł��邩�H�ۂ��𔻒肵�A�ۂȂ�҂����鏈���i�L���[�C���O�j*/
					LCM_WaitingLock(IFM_LockTable.sSlave[slave].sLock[lock].ucOperation,
									(uchar)slave,
									(uchar)lock,
									IFM_LockTable.sSlave[slave].sLock[lock].ucConnect,
									1,			/* �L���[�C���O����Ȃ�A�̏჊�g���C���v�� */
									(t_LKcomCtrl *)0);
				}
				/* �������g���C���Ȃ� */
				else {
					continue;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutTest()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�J�e�X�g�����^�C���A�E�g										   |*/
/*|		�E�{�֐��ɂāA�J�e�X�g�̂��߂̐������Ԃ��o�߂�������			   |*/
/*|		  ���ʉ�����Ԃ��܂��B											   |*/
/*|		�E�������ԓ��ɊJ�e�X�g�����������A�{�֐����Ăяo����܂��B	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimeoutTest(void)
{
	int		slave, lock;
	int		del;

	IFM_LockTable.bTest = 0;		/* �J�e�X�g�����N���A */

	/* �^�C���A�E�g�Ȃ̂ŁA�����{�e�X�g���ʂ��L�� */
	for (slave=0; slave < IF_SLAVE_MAX; slave++) {
		for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {
			WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
			/* �S�J�e�X�g�E�E�E�c��̌��ʂ͑S�āuNG�v*/
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation == 8) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 2;	/* NG */
// ���g���C���M�̔�����h��
				// ���g���C�L���[���T�[�`
				while (1) {
					del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, (uchar)slave, (uchar)lock);
					// ���b�N���u����v�Ȃ�΂��̗v�����폜
					if (del >= 0) {
						DeQueue(&LCMque_Ctrl_Action.Retry, del);
					}
					if (del == -1) break;
				}
				// �֘A����đ��^�C�}�X�g�b�v
				LCMtim_1secTimerStop(slave, lock);
			}
			/* �ʊJ�e�X�g�E�E�E���Y���ʂ́uNG�v*/
			if (IFM_LockTable.sSlave[slave].sLock[lock].ucOperation == 9) {
				IFM_LockTable.sSlave[slave].sLock[lock].ucResult = 2;	/* NG */
// ���g���C���M�̔�����h��
				// ���g���C�L���[���T�[�`
				while (1) {
					del = MatchRetryQueue_Lock(&LCMque_Ctrl_Action.Retry, (uchar)slave, (uchar)lock);
					// ���b�N���u����v�Ȃ�΂��̗v�����폜
					if (del >= 0) {
						DeQueue(&LCMque_Ctrl_Action.Retry, del);
					}
					if (del == -1) break;
				}
				// �֘A����đ��^�C�}�X�g�b�v
				LCMtim_1secTimerStop(slave, lock);
				break;
			}
		}
	}
	LCM_AnswerQueue_toPAY62();			// (62H)�J�e�X�g����
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimerStartAction()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���슮���^�C�}�[�N��											   |*/
/*|		�E�����w��̐M���o�͎��ԂɁu�{���v���Ԃ������ă^�C�}�[�J�n���܂��B |*/
/*|		�E���b�N�Ǘ��^�X�N����A�Ύq�@�ʐM�^�X�N�ւ̃��[�����M���		   |*/
/*|		  �Ăяo����܂��B												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TimerStartAction(uchar ucSlave, uchar ucLock, ushort usAction, BOOL bLockOff)
{

	/* ��������A���슮������܂ł̃^�C�}�[�N�� */
	usAction += IFS_ANSWER_WAIT_TIME;								/* �{���́A�q�@15��~�ꏄ�ɗv���鎞�ԁimsec�P�ʁj*/
	LCMtim_10msTimerStart((int)ucSlave, (int)ucLock, usAction);
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_SetStartAction()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���쒆�Z�b�g													   |*/
/*|		�E(���b�N���u)���슮���𔻒肷�邽�߁A�e�[�u����̃Z���T�[��Ԃ�   |*/
/*|		 �u���쒆�r�b�g�ibit:3�j�v�𗧂Ă�B							   |*/
/*|		�E�q�@���ł��M���o�͒��ɁA���̃r�b�g�𗧂ĂĂ��邯�ǁA			   |*/
/*|		  �e�@���̃|�[�����O�Ԋu�ɂ���ẮA�r�b�gON���Ԃ��Ƃ蓦����	   |*/
/*|		  �\������B													   |*/
/*|		�E������������ɂ́A�q�ɓ͂������_�Őe��(�r�b�g���Ă�)������   |*/
/*|		  ���邵���Ȃ��B												   |*/
/*|		���Ύq�@�ʐM�^�X�N����u��ԏ������݁v�𑗐M�������ACK��M����	   |*/
/*|		  �Ăяo����܂��B												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_SetStartAction(t_IFM_Queue *pMail)
{
	uchar	ucSlave;
	uchar	ucLock;

	/*�u��ԏ������݁v�ȊO�͏��O */
	if (pMail->sCommon.ucKind != 'W') {
		return;
	}
	ucSlave = pMail->sCommon.ucTerminal;
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){		// ���b�N���u�̏ꍇ�̂݃`�F�b�N
	/* �����e�i���X�J�n/�I���͏��O�E�E�E�ȉ��̏����́ALCM_NoticeMainte_toIFS(���̃\�[�X)���Q�Ƃ̂��� */
	if (pMail->sW57.usLockOffTime == 0 &&	/* ���b�N���u�J���̐M���o�͎���	=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j*/
		pMail->sW57.usLockOnTime  == 0 &&	/* ���b�N���u���̐M���o�͎���	=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j*/
		pMail->sW57.ucLock        == 0 &&	/* ���b�N���uNo.				=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j*/
		pMail->sW57.ucMakerID     == 0)		/* ���b�N���u���[�J�[ID			=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j*/
	{
		return;
	}

	/* ���쒆�r�b�g���A������ON���Ƃ��Ȃ��Ɠ��슮������ł��Ȃ��\������ */
	}
	ucLock  = pMail->sW57.ucLock;
	IFM_LockTable.sSlave[ucSlave].sLock[ucLock].unSensor.Bit.B3 = 1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_Is_AllOver()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�v���Z�b�g��S�ď����������H�ۂ��𔻒�							   |*/
/*|		�E�����敪��12�i�o�[�W�����v���j								   |*/
/*|				  ���W�i�S���b�N�J�e�X�g�j							   |*/
/*|				  ���X�i�w�胍�b�N�J�e�X�g�j							   |*/
/*|		  �ɂ��āA�S�ď����������H�ۂ���Ԃ��܂��B					   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�߂�l�F															   |*/
/*|			=1 �S�ď�������												   |*/
/*|			=0 ���������͌p������										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	LCM_Is_AllOver(uchar ucOperation, uchar ucSlave, uchar ucLock)
{
	int		ni, nj;
	ushort	limitSecond, lockOnTime, lockOffTime;
	uchar	maker;

	switch (ucOperation) {
		case 12:	/* �o�[�W�����v�� */
		case 15:	/* ���[�v�f�[�^�v�� */
			for (ni = 0; ni < IF_SLAVE_MAX; ni++) {
				if (IFM_LockTable.sSlave[ni].ucContinueCnt) {
					return 0;		/* �������� */
				}
			}
			return 1;	/* �S�ď������I����� */

		case 8:		/* �S���b�N�J�e�X�g */
			for (ni=(int)ucSlave; ni < IF_SLAVE_MAX; ni++) {
				for (nj=(int)ucLock; nj < IFS_ROOM_MAX; nj++) {
					WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					/* ���̑S���b�N�J�e�X�g�Ώۂ�T�� */
					if (IFM_LockTable.sSlave[ni].sLock[nj].ucOperation == ucOperation)
					{
						/* ���܊J�e�X�g�ł���Ȃ� */
						if (LCM_do_ActionNow(ni, nj)) {
							maker = IFM_LockTable.sSlave[ni].sLock[nj].ucConnect;
							LCM_WaitingLock(ucOperation, (uchar)ni, (uchar)nj, maker, 0, (t_LKcomCtrl *)0);

							/*
							*	�E���ɊJ�e�X�g���ׂ����b�N���u�̐������Ԃ����߂�B
							*	�������Ō��߂邩��A�^�C�}�[�ĊJ����B
							*/
							lockOnTime = IF_SignalOut[maker].usLockOnTime;
							lockOffTime = IF_SignalOut[maker].usLockOffTime;
							limitSecond = LCM_GetLimitOneTest(lockOnTime, lockOffTime);
							LCMtim_1secWaitStart(limitSecond);
							return 0;	/* �������� */
						}
						/* ���܊J�e�X�g�ł��Ȃ����̂́A�����{ */
						else {
							IFM_LockTable.sSlave[ni].sLock[nj].ucOperation = 0;		/* �J�e�X�g�\����L�����Z�� */
							IFM_LockTable.sSlave[ni].sLock[nj].ucResult = 0;		/* ���ʂ́u�w��O�v�Ŋm�� */
						}
					}
				}
			}
			return 1;	/* �S�ď������I����� */

		case 9:		/* �ʃ��b�N�J�e�X�g */
			return 1;	/* �S�ď������I����� */

		default :
			return 1;	/* �S�ď������I����� */
	}
}


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TestResult()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�J�e�X�g�̌��ʔ���											   |*/
/*|		�E�J�e�X�g�i�S���b�N/�w�胍�b�N�j�ɂ����āA					   |*/
/*|		  ���̃e�X�g���ʂ𔻒肵										   |*/
/*|		�E�I���Ȃ�A������(�������ԃ^�C���A�E�g)���N�������A			   |*/
/*|		  ���ʉ����𑣂�												   |*/
/*|		�E�����Ȃ�A���̃��b�N���u���e�X�g���ׂ��A����v�����o���B		   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_TestResult(uchar lockState, int slave, int lock)
{
	uchar	result;

	if (lockState == 1 ||	/* ���쌋�ʂ��u�{����ς݁i����j�v*/
		lockState == 2 ||	/* �V�u�J����ς݁i����j�v*/
		lockState == 5)		/* �V�u�̏�����v*/
	{
		result = 1;		/* (�e�X�g����)���� */
	}
	else {
		result = 2;		/* (�e�X�g����)NG */
	}

	/* �J�e�X�g�P��ځH*/
	if (IFM_LockTable.sSlave[slave].sLock[lock].ucResult == 0) {
		IFM_LockTable.sSlave[slave].sLock[lock].ucResult = result;	/* ������ */
		/* �t�����֍ē��� */
		LCM_WaitingLock(	IFM_LockTable.sSlave[slave].sLock[lock].ucOperation,
							(uchar)slave,
							(uchar)lock,
							IFM_LockTable.sSlave[slave].sLock[lock].ucConnect,
							0,
							(t_LKcomCtrl *)0);
	}
	/* �J�e�X�g�Q��ځH*/
	else {
		/*
		*	�P��ڂ̌���OK�Ȃ�A�Q��ڂ̌��ʂ��u�{���ʁv
		*	�P��ڂ̌���NG�Ȃ�A�Q��ڂ̌��ʂɊւ�炸�P��ڂ̌��ʂ��̗p���܂�uNG�v
		*/
		if (IFM_LockTable.sSlave[slave].sLock[lock].ucResult == 1) {
			IFM_LockTable.sSlave[slave].sLock[lock].ucResult = result;	/* �{���� */
		}

		/* �J�e�X�g�I������H*/
		IFM_LockTable.sSlave[slave].sLock[lock].ucOperation = 0;		/* �J�e�X�g�\����N���A�i�e�X�g�ς݂Ȃ̂Łj*/
		if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)slave, (uchar)lock)) {
			LCMtim_1secWaitStart(0);	/* �O�Z�b�g���ċ����Ƀ^�C���A�E�g���N�������� */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             IFM_SetAction()	                                           |*/
/*|             IFM_ResetAction()                                          |*/
/*|             IFM_Is_Action()	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�w�胍�b�N���u�i=�Ԏ��j�ɑ΂�									   |*/
/*|		�E���쒆�i=1�j�փZ�b�g											   |*/
/*|		�E���슮���i=0�j�փ��Z�b�g										   |*/
/*|		�E���쒆���H�ۂ���Ԃ��B										   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���l�F																   |*/
/*|		�q�@�֓���v�����[���𑗐M�����Ƃ��u���쒆�v�Ƃ��邽�ߌĂяo����A |*/
/*|		�q�@��ԃf�[�^�ɂē��슮�����m�炳�ꂽ�Ƃ��u���슮���v�Ƃ��邽�߂� |*/
/*|		�Ăяo�����B													   |*/
/*|		����v�����󂯂��w�w�胍�b�N���u�i=�Ԏ��j�x���A���܁u���쒆�v�Ȃ�  |*/
/*|		���̓��슮���܂ő҂����邩�H�ۂ����A���b�N�Ǘ��^�X�N���x����	   |*/
/*|		���肷�邽�߁A�g�p�����B										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-05-09                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	IFM_SetAction(uchar ucSlave, uchar ucLock)
{
	IFM_bLockAction[ucSlave][ucLock] = 1;
}
void	IFM_ResetAction(uchar ucSlave, uchar ucLock)
{
	IFM_bLockAction[ucSlave][ucLock] = 0;
}
BOOL	IFM_Is_Action(uchar ucSlave, uchar ucLock)
{
	return IFM_bLockAction[ucSlave][ucLock];
}

uchar	IFM_Get_ActionTerminalNo(void)
{
	uchar tno;
	uchar lock;
	
	for(tno = 0; tno < IF_SLAVE_MAX; ++tno) {
		WACDOG;												// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		for(lock = 0; lock < IFS_ROOM_MAX_USED; ++lock) {
			if(IFM_bLockAction[tno][lock]) {
				return (uchar)(tno + 1);
			}
		}
	}
	return 0;
}

BOOL	IFM_Can_Action(char type)
{
	uchar tno;
	uchar lock;
	uchar cmp_cnt = 1;// �u1�v�͓�����1�䂵���t���b�v�^���b�N���u�𐧌�ł��Ȃ��Ƃ����Ӗ�
	uchar act_cnt = 0;
	if(0 != prm_get(COM_PRM, S_TYP, 134, 1, 1)){// 03-0134�E�F�t���b�v���u2�䓯������@�\
		if(0 != type){// �Ώۂ̓t���b�v
			cmp_cnt = 2;// 2��܂œ�������\�Ƃ���
		}
	}
	
	for(tno = 0; tno < IF_SLAVE_MAX; ++tno) {
		WACDOG;												// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		for(lock = 0; lock < IFS_ROOM_MAX_USED; ++lock) {
			if(IFM_bLockAction[tno][lock]) {
				if(type == LKcom_Search_Ifno((uchar)(tno + 1))) {
					act_cnt++;
				}
			}
			if(act_cnt >= cmp_cnt ){// ���쒆����������\�䐔�ȏ�œ���ǉ�����s��
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetLimitAction()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���슮������܂ł̐������Ԏ擾�i10msec�P�ʁj                       |*/
/*|		�E�Ύq�@�ʐM�^�X�N�փ��b�N���u�����v�����Ă���                   |*/
/*|		  ���슮���̏�ԕω��ʒm���͂��܂ł̐������Ԃ����߂�B             |*/
/*|		�E�J/�A����M���o�͎��Ԃ��x�[�X�ɂ���B                          |*/
/*|		�E�{���Z�o�̂��߂̍l�����ׂ��v���́A�ȉ��B                         |*/
/*|		  1)�q�@�ւ̃|�[�����O�Ԋu                                         |*/
/*|		  2)�q�@���A�e�@�����ԗv�����󂯁A��������܂ł̎����l           |*/
/*|		  3)�|�[�����O�Ώۂ̎q�@���A1��݂̂ɂȂ����Ƃ��̃|�[�����O�Ԋu    |*/
/*|		  4)�`�����x�Ɉˑ����镶���ԃ^�C���A�E�g����                       |*/
/*|                                                                        |*/
/*|		�������ɂēn����鎞�Ԃ̒P�ʂ́A100msec�Ƃ���B                    |*/
/*|		���擾���鎞��(�߂�l)�̒P�ʂ́A10msec�Ƃ���B                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	LCM_GetLimitAction(ushort usSigalOutTime)
{
	ulong	betweenChar, plusAlpha;
	ushort	limit_10msec;

	/* �����ԃ^�C���A�E�g���Ȃ��Ǝ�M�����ƔF�����Ȃ� */
	betweenChar = (ulong)(toS_RcvCtrl.usBetweenChar);				// �e�Ǝq�B�P�ʂ�2msec��� ��~2��2��1�{�

	/* �{���������߂�B�P�ʂ�1msec */
	plusAlpha = (ulong)((IFM_LockTable.toSlave.usPolling * 10 + IF_DEFAULT_SLAVE_ANSWER_TIME) * IF_SLAVE_MAX);
	if (plusAlpha == 0) {
		plusAlpha = (ulong)(IF_DEFAULT_toSLAVE_POLLING_ALONE * 10 + IF_DEFAULT_SLAVE_ANSWER_TIME);
	}

	/* 10msec�P�ʂ֑�����i�؂�グ�j*/
	limit_10msec = (ushort)(usSigalOutTime * 10);					/* �����́A100msec�P�ʂȂ̂�10�{�� */
	limit_10msec += (ushort)((plusAlpha + betweenChar + 9) / 10);	/* ���̑��́A1msec�P�ʂȂ̂�1/10�� */
																	/* ����{9��́A10msec�P�ʂŐ؂�グ�邽�� */

	return limit_10msec;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetLimitOneTest()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�J�e�X�g�������Ԏ擾�i�P���b�N���u���j                           |*/
/*|		�E�J/�A�Q�񕪂̓���̐������Ԃ����߂�B                          |*/
/*|		�E�J/�A���슮������܂ł̎��Ԃ��x�[�X�ɂ���B                    |*/
/*|                                                                        |*/
/*|		�������ɂēn����鎞�Ԃ̒P�ʂ́A100msec�Ƃ���B                    |*/
/*|		���擾���鎞��(�߂�l)�̒P�ʂ́A1sec�Ƃ���B                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-06-17                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	LCM_GetLimitOneTest(ushort usLockOnTime, ushort usLockOffTime)
{
	ushort	limitSecond;
	ulong	on_msec, off_msec;

	on_msec  = (ulong)(LCM_GetLimitAction(usLockOnTime)  * 10);	/* 10msec�P�ʂ��A1msec�P�ʂ� */
	off_msec = (ulong)(LCM_GetLimitAction(usLockOffTime) * 10);	/* 10msec�P�ʂ��A1msec�P�ʂ� */

	/* 1msec�P�ʂ��A1sec�P�ʂցi�؂�グ�j*/
	limitSecond = (ushort)((on_msec + off_msec + 999) / 1000);		/* ��{999��́A1sec�P�ʂŐ؂�グ�邽�� */

	return limitSecond;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NoticeStatus_toPAY()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|   ���Z�@�ւ̏�ԕω��ʒm�i���b�N���u�ʁj                               |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NoticeStatus_toPAY( uchar tno, uchar ucLock, uchar ucCarState, uchar ucLockState)
{
	t_IFM_mail	*pQbody;
	int	i;
	int imax;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// �ʏ��ԃf�[�^
	pQbody->s61.sCommon.bHold            = 1;				// �f�[�^�ێ��t���O
	pQbody->s61.sCommon.t_no             = tno;				// �[��No(CRB��)
	pQbody->s61.ucCount                  = 1;				// ��ԃf�[�^���B�����e�i���X�ʒm�̂��߂ɂ́u�P�v�ŏ\�� 
	if(LKcom_Search_Ifno(tno) == 0) {
		i = FLAP_NUM_MAX;					// ���b�N���u�C���f�b�N�X
		imax = LOCK_MAX;						// ���b�N���u�ő�C���f�b�N�X
	}
	else {
		i = CRR_CTRL_START;					// �t���b�v���u�C���f�b�N�X
		imax = TOTAL_CAR_LOCK_MAX;			// �t���b�v���u�ő�C���f�b�N�X
	}

	for( ; i<imax; ++i ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LKcom_Search_Ifno(tno) == 0) {									// ���b�N�̏ꍇ
			if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_no == ucLock)){// �^�[�~�i��No�ƃ��b�N���u�ԍ���v
				// LockInfo��LOCK_MAX�̒l��MAX�ł��邪�A255�Ԏ�������肻�̂܂܃Z�b�g
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfo�̃C���f�b�N�X���Z�b�g
				break;
			}
		} else {															// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
			if( (LockInfo[i].if_oya == tno)&&( 1 == ucLock)){// �^�[�~�i��No�ƃ��b�N���u�ԍ���v
				// LockInfo��LOCK_MAX�̒l��MAX�ł��邪�A255�Ԏ�������肻�̂܂܃Z�b�g
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfo�̃C���f�b�N�X���Z�b�g
				break;
			}
		}
	}
	pQbody->s61.sLock[0].ucCarState  = ucCarState;			// �ԗ����m���		=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j
	pQbody->s61.sLock[0].ucLockState = ucLockState;			// ���b�N���u���	(07H)�����e�i���X��/(08H)�����e�i���X�����̉��ꂩ 
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],7);				// �A�v��������M����o�b�t�@�ɃZ�b�g
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_NoticeStatus_toPAY()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|   ���Z�@�ւ̏�ԕω��ʒm(�S��)                                         |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_NoticeStatusall_toPAY( void )
{
	int i;
	t_IFM_mail *pQbody;
	uchar count;
	ushort len;
	uchar lok_no_local;

	count = 0;
	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// �ʏ��ԃf�[�^
	pQbody->s61.sCommon.bHold            = 1;				// �f�[�^�ێ��t���O
	pQbody->s61.sCommon.t_no             = LOCK_REQ_ALL_TNO;// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	len = 4;												// ��ʁA�ێ��t���O�A�^�[�~�i��No�A�f�[�^���̌v4Byte

	for( i=FLAP_NUM_MAX; i<LOCK_MAX; i++ ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ���b�N�̏ꍇ
			lok_no_local = LockInfo[i].lok_no;
		} else {											// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// �Ԏ��p�����[�^�Ƀ^�[�~�i��No�A���b�N���u�A�ԗL�L
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				// LockInfo��LOCK_MAX�̒l��MAX�ł��邪�A255�Ԏ�������肻�̂܂܃Z�b�g
				pQbody->s61.sLock[count].ucLock = (uchar)i;// LockInfo�̃C���f�b�N�X���Z�b�g	
				// �ԗ����m���	
				pQbody->s61.sLock[count].ucCarState = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucCarState;
				// ���b�N���u���	(07H)�����e�i���X��/(08H)�����e�i���X�����̉��ꂩ
				pQbody->s61.sLock[count].ucLockState = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucLockState;
				count++;
				pQbody->s61.ucCount = count;// �f�[�^��
				len += 5;// ���b�N���uNo,�ԗ����m���,���b�N���u���,�\��2Byte�̌v5Byte
			}
		}
	}
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// �A�v��������M����o�b�t�@�ɃZ�b�g
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY63()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���Z�@�ւ̃����e�i���X���v������								   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY63(uchar type)
{
	int	i, cnt;
	t_IFM_mail *pQbody;
	ushort len;
	uchar lok_no_local;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s63.sCommon.ucKind           = LK_RCV_MENT_ID;	// �����e�i���X���v������
	pQbody->s63.sCommon.bHold            = 1;				// �f�[�^�ێ��t���O
	if(type == 0) {
		pQbody->s63.sCommon.t_no         = LOCK_REQ_ALL_TNO;// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	}
	else {
		pQbody->s63.sCommon.t_no         = FLAP_REQ_ALL_TNO;// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	}
	len = 3;												// ��ʁA�ێ��t���O�A�^�[�~�i��No

// CRA�d���̃����e�i���X���v��������CRA�ŊǗ����Ă��郍�b�N���u�̘A�Ԃ̏����Z�b�g�������A
// CRB IF ��CRA IF �ɕϊ����Ďg�p����ꍇ��LockInfo�̃C���f�b�N�X�ɑΉ����郍�b�N���u�A�Ԃ��Z�b�g����j
	for( i=0, cnt=0; i< LOCK_MAX ; i++ ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ���b�N�̏ꍇ
			lok_no_local = LockInfo[i].lok_no;
		} else {											// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// �Ԏ��p�����[�^�Ƀ^�[�~�i��No�A���b�N���u�A�ԗL�L
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				c_int32toarray(pQbody->s63.sLock[cnt].ucAction,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulAction,4);// �J����̍��v��
				c_int32toarray(pQbody->s63.sLock[cnt].ucManual,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulManual,4);// I/F��SW�ɂ��蓮�ł̊J�E���썇�v��
				c_int32toarray(pQbody->s63.sLock[cnt].ucTrouble,
							   IFM_LockAction.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ulTrouble,4);// �̏Ⴕ�����v��
			}
		}
		len += 12;// 4 * 3byte
		cnt++;
	}

	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// �A�v��������M����o�b�t�@�ɃZ�b�g
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY62()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���Z�@�ւ̃��b�N���u�J�e�X�g����								   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY62(void)
{
	int	i, cnt;
	t_IFM_mail *pQbody;
	ushort len;
	uchar lok_no_local;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s62.sCommon.ucKind           = LK_RCV_TEST_ID;	// ���b�N���u�J�e�X�g����
	pQbody->s62.sCommon.bHold            = 1;				// �f�[�^�ێ��t���O
	pQbody->s62.sCommon.t_no             = LOCK_REQ_ALL_TNO;// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	len = 3;												// ��ʁA�ێ��t���O�A�^�[�~�i��No�̌v3Byte

// CRA�d���̃����e�i���X���v��������CRA�ŊǗ����Ă��郍�b�N���u�̘A�Ԃ̏����Z�b�g�������A
// CRB IF ��CRA IF �ɕϊ����Ďg�p����ꍇ��LockInfo�̃C���f�b�N�X�ɑΉ����郍�b�N���u�A�Ԃ��Z�b�g����j
	for( i=FLAP_NUM_MAX, cnt=0; i< (FLAP_NUM_MAX+IFM_ROOM_MAX_USED) ; i++ ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LKcom_Search_Ifno(LockInfo[i].if_oya) == 0) {	// ���b�N�̏ꍇ
			lok_no_local = LockInfo[i].lok_no;
		} else {											// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
			lok_no_local = 1;
		}			
		if( (0 != LockInfo[i].if_oya) && (0 != lok_no_local) ){// �Ԏ��p�����[�^�Ƀ^�[�~�i��No�A���b�N���u�A�ԗL�L
			if (IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucConnect) {
				pQbody->s62.sLock[cnt].ucResult = IFM_LockTable.sSlave[LockInfo[i].if_oya-1].sLock[lok_no_local-1].ucResult;
			}
		}
		len++;// 1byte
		cnt++;
	}

	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// �A�v��������M����o�b�t�@�ɃZ�b�g
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_toPAY64()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���Z�@�ւ̃o�[�W��������										   |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_toPAY64(void)
{
	int	 ni;
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];					// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s64.sCommon.ucKind           = LK_RCV_VERS_ID;		// �o�[�W��������
	pQbody->s64.sCommon.bHold            = 1;					// �f�[�^�ێ��t���O
	pQbody->s64.sCommon.t_no             = LOCK_REQ_ALL_TNO;	// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	len = 11;													// ��ʁA�ێ��t���O�A�^�[�~�i��No�AI/F�Րe�@�o�[�W�����̌v11Byte
	// I/F�Րe�@�͑��݂��Ȃ��̂ŏȗ�
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		memcpy(pQbody->s64.sIFSlave[ni].cViersion, IFM_LockTable.sSlave[ni].cVersion, 8); // �\�t�g�o�[�W�����i�q�̕��ԁj�́A�q�ɖ₢���킹��H
		len += 8;// 8byte
	}
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);					// �A�v��������M����o�b�t�@�ɃZ�b�g
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_AnswerQueue_Timeout()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�e�X�g�����ʒm													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  yanagawa                                                |*/
/*| Date        :  2013/1/24                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_AnswerQueue_Timeout(void)
{
	t_IFM_mail *pQbody;
	ushort len;

	memset(LKcomdr_RcvData, 0, sizeof(LKcomdr_RcvData));
	pQbody = (t_IFM_mail *)&LKcomdr_RcvData[0];				// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->sCommon.ucKind           = CRR_RCV_TEST_ID;		// �e�X�g����
	pQbody->sCommon.bHold            = 1;					// �f�[�^�ێ��t���O
	pQbody->sCommon.t_no             = LOCK_REQ_ALL_TNO;	// CRB�S�Ă��Ӗ�����̂�FF�Ƃ���
	len = 3;												// ��ʁA�ێ��t���O�A�^�[�~�i��No�AI/F�Րe�@�o�[�W�����̌v11Byte
	LKcom_RcvDataSave(&LKcomdr_RcvData[0],len);				// �A�v��������M����o�b�t�@�ɃZ�b�g
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_ControlLock()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���b�N���u����													   |*/
/*|		�E���Z�@�����M�����u���b�N���u����v����͂�����A�Ăяo����     |*/
/*|		  �Y���̎q�@�ցu��ԏ������݁v�Ƃ��Ă𑗂�B					   |*/
/*|		�E�����e�i���X���[�h�ɂāA�蓮�X�C�b�`�ɂ��u���b�N���u����v��   |*/
/*|		  �Y���̎q�@�ցu��ԏ������݁v�Ƃ��Ă𑗂�B					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-03-31                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LCM_ControlLock(uchar bMode, uchar bLockOff, uchar ucSlave, uchar ucLock, uchar ucMaker)
{
	/* �q�@�����u��ԏ������݃f�[�^�v����� */
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	ushort	actionTimer;
	ushort	lockOnTime, lockOffTime;

	/* �M���o�͎��Ԃ́A�e�[�u������i�l���s���̏ꍇ�A�␳����j*/
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		lockOnTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOnTime;		/* ���b�N���u���̐M���o�͎��� */
		lockOffTime = IFM_LockTable.sSlave[ucSlave].sLock[ucLock].usLockOffTime;	/* ���b�N���u�J���̐M���o�͎��� */
		if (lockOnTime == 0) {
			lockOnTime = IF_SignalOut[ucMaker].usLockOnTime;
		}
		if (lockOffTime == 0) {
			lockOffTime = IF_SignalOut[ucMaker].usLockOffTime;
		}
	}
	else {
		lockOnTime = 0;
		lockOffTime = 0;
	}

	// �󂫃L���[�݂邩�H
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	// �󂫎��̂��擾
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	// �󂫖{�̂��擾
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;												// ���̂������N

		// �L���[�t��[����]����
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	// �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j
	else {
		/* �L���[�t��[����] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);

		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);							// �L���[(�擪)���O��
		pQbody = pQbuff->pData;
	}
	//�u��ԏ������݃f�[�^�v���Z�b�g
	pQbody->sW57.sCommon.ucKind     = 'W';									// ��ԏ������݃f�[�^ 
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){
		pQbody->sW57.usLockOffTime      = lockOffTime;							// ���b�N���u�J���̐M���o�͎��� 
		pQbody->sW57.usLockOnTime       = lockOnTime;							// ���b�N���u���̐M���o�͎��� 
		pQbody->sW57.ucLock             = ucLock;								// ���b�N���uNo. 
		pQbody->sW57.ucMakerID          = ucMaker;								// ���b�N���u���[�J�[ID 
		pQbody->sW57.unCommand.Byte     = 0;									// ��U�A�S�r�b�g���N���A 
		pQbody->sW57.unCommand.Bit.B4   = bLockOff;								// (0)�{��/(1)�J���A������̓��삩�H
		pQbody->sW57.unCommand.Bit.B7   = bMode;								// (0)�ʏ�^�p/(1)�����e�i���X�� 
		pQbody->sW57.unCommand.Byte     |= 0x40;								// �r�b�g6���Z�b�g(�K�[�h���ԑΉ�) 
	}
	else {
		pQbody->sW57.usLockOffTime      = 0;
		pQbody->sW57.usLockOnTime       = 0;
		pQbody->sW57.ucLock             = 0;
		pQbody->sW57.ucMakerID          = 0;
		if(bLockOff == 0 || bLockOff == 1) {
			pQbody->sW57.unCommand.Byte = (uchar)('4' + bLockOff);			// �t���b�v�㏸/���~�R�}���h�ɕϊ�
		}
		else {
			pQbody->sW57.unCommand.Byte = (uchar)('1' + bLockOff - 2);		// �t���b�v����OFF/ON�R�}���h�ɕϊ�
		}
	}
	pQbody->sW57.sCommon.ucTerminal = ucSlave;								// �^�[�~�i��No.�i�ǂ̎q�@���H����肷��j

	// �����փL���[�C���O
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	/*
	*	2005.05.26 �����E���
	*	�q�@�����u��ԏ������݃f�[�^�v�ɂ́A150msec�P�ʂł̎��Ԃ𑗂��Ă���̂ŁA
	*	�e���Ď�����Ƃ��ɂ͎����Ԃ֕ϊ����Ȃ��Ă͂Ȃ�Ȃ��B
	*	���܂��ɁA���̊Ď��Ɏg���^�C�}��10msec�^�C�}�[�Ȃ̂�
	*	����ɒ��ӂ̂��ƁB
	*
	*	2005.05.26 �����E�o�O����
	*	���́u150msec�P�ʂ���̕ϊ��v��Y�ꂽ���߂ɁA
	*	�g���A�R���Y�ȂǒZ�����Ԃ̂��̂�OK�i�{���ɋz������āj�����A
	*	�p�c�o�C�N�́A�M���o�͎����Ԃ��������A���슮���^�C���A�E�g���Ă��܂�
	*	�i������=2000msec �ɑ΂��āA�^�C���A�E�g�l=[20�{150]msec�B20�́~100msec�v�Z�O�̒l�A150=�{��(=�ꏄ�ɗv���鎞��=�q�@15��~�|�[�����O�Ԋu)�j
	*	�̏ስ���œ��슮�������ƔF�����A���̌�̏�ԃf�[�^�ŁA�{���́u���슮���v��m�邱�ƂɂȂ�B
	*	�P������ł́A���Q�͏��Ȃ����A����ł���ԕω�����Ɂu(���߂̃^�C���A�E�g)�̏ၨ�̏����(�{���̓��슮����)�v�ƂȂ�B
	*	�J�e�X�g����ł́A�v���I�ŁA
	*	�����"�{(��)"����ŁA�e�X�g�����i2�񓮍삵���ƔF���j���Ă��܂��A����"�J"����͔�e�X�g�����ɂȂ�A
	*	���g���C�ΏۂƂȂ��āA���x�����g���C���{���n�`�����`���ɂȂ�o�O�ɔY�܂��ꂽ�B
	*/
	/* 100msec�P�ʂ�ϊ�����10msec�^�C�}�l�� */
	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 1 ){
		lockOnTime = IFM_LockTable.sSlave[ucSlave].usFlapUpTime;
		lockOffTime = IFM_LockTable.sSlave[ucSlave].usFlapDownTime;
	}
	if (bLockOff) {
		actionTimer = LCM_GetLimitAction(lockOffTime);
	} else {
		actionTimer = LCM_GetLimitAction(lockOnTime);
	}
	/* ��������A���슮������܂ł̃^�C�}�[�N�� */
	LCM_TimerStartAction(ucSlave, ucLock, actionTimer, bLockOff);
}


/*[]----------------------------------------------------------------------[]*/
/*|             LCM_TimeoutOperation()                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�o�[�W�����v���A���b�N���u�J�e�X�g���^�C���A�E�g�����Ƃ��̏���   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Nishizato                                               |*/
/*| Date        :  2006-10-18                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LCM_TimeoutOperation(void)
{
	if (LCMtim_1secWaitTimeout()) {
		LCMtim_1secWaitStop();			/* ���̐������ԃ^�C�}�[��~ */
		switch (IFM_LockTable.ucOperation) {
			case 12:
				LCM_AnswerQueue_toPAY64();		// (64H)�o�[�W��������
				break;

			case 8:
			case 9:
				LCM_TimeoutTest();		/* �J�e�X�g�����^�C���A�E�g */
				break;
			default:
				break;
		}
		IFM_LockTable.ucOperation = 0;	// ���ݐ����ʃN���A
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             PAYcom_InitDataSave()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|     CRB�֑��M���邽�߂̏����ݒ�f�[�^��ۑ�����B                      |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : �Ȃ�                                                          |*/
/*|	return : �߂�l�Ȃ�                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2010-11-15                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PAYcom_InitDataSave( void )
{
	int	t_no_max, tno, i, cnt,lock;
	t_LockMaker flapMaker;
	uchar	wk_mak;
	uchar	crr_no;		// CRR���No.
	uchar	point_no;	// �ړ_No.
	uchar	lok_no_local;
	t_no_max = (int)LKcom_GetAccessTarminalCount();				// �t���b�v����ՁA���b�N���u�pIF�Վq�@�ڑ��䐔
	/* ���b�N���u�̐ڑ��i��ԁj*/

	if(t_no_max > LOCK_IF_MAX){//�^�[�~�i��No���ő吔���I�[�o�[���Ă�����K�[�h
		t_no_max = LOCK_IF_MAX;
	}
	
	IFM_LockTable.toSlave.usLineWatch    = (ushort)(CPrmSS[S_TYP][56]/10);				// ��M�Ď��^�C�}�[
	IFM_LockTable.toSlave.usPolling      = (ushort)(CPrmSS[S_TYP][57]/10);				// POL�Ԋu
	IFM_LockTable.toSlave.ucRetry        = (uchar )prm_get( 0, S_TYP, 58, 2, 3 );		// ���g���C��
	IFM_LockTable.toSlave.ucSkipNoAnswer = (uchar )prm_get( 0, S_TYP, 58, 2, 1 );		// �������G���[�����

	if(( GetCarInfoParam() & 0x01 )){							// ���֐ݒ肠��
		for( tno=1; tno <= t_no_max; tno++ ){					// �����I�ȃ��b�N���u�̎Ԏ��ԍ��X�^�[�g�ʒu(FLAP_NUM_MAX)����^�[�~�i��No����������
			if( LKcom_Search_Ifno( (uchar)tno ) == 0 ){				// �Y���^�[�~�i��No�̐ڑ��͒���
				wk_mak = child_mk[tno-1];// ���b�N���u���[�J�[�̏����擾
				// ���b�N���u���[�J�[�͎Ԏ����Ƃł͂Ȃ�����CRB�P�ʂł����ݒ�ł��Ȃ�
				if( wk_mak != 0 ){
					IFM_LockTable.sSlave[tno-1].usEntryTimer    = LockMaker[(uchar)(wk_mak-1)].in_tm;	// ���Ɏԗ����m�^�C�}�[
					IFM_LockTable.sSlave[tno-1].usExitTimer     = LockMaker[(uchar)(wk_mak-1)].ot_tm;	// �o�Ɏԗ����m�^�C�}�[
					IFM_LockTable.sSlave[tno-1].usRetryCount    = LockMaker[(uchar)(wk_mak-1)].r_cnt;	// ���g���C��
					IFM_LockTable.sSlave[tno-1].usRetryInterval = LockMaker[(uchar)(wk_mak-1)].r_tim;	// ���g���C�Ԋu
				}else{
					IFM_LockTable.sSlave[tno-1].usEntryTimer = 0;
					IFM_LockTable.sSlave[tno-1].usExitTimer = 0;
					IFM_LockTable.sSlave[tno-1].usRetryCount = 0;
					IFM_LockTable.sSlave[tno-1].usRetryInterval = 0;
				}
				for( i=FLAP_NUM_MAX, cnt=0; i<LOCK_MAX; i++ ){
					WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					if(LKcom_Search_Ifno(tno) == 0) {	// ���b�N�̏ꍇ
						lok_no_local = LockInfo[i].lok_no-1;
					} else {							// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
						lok_no_local = 0;
					}			
					if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_syu != 0) ){						// �^�[�~�i��No��v���ڑ�����H
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].ucConnect = get_lktype(LockInfo[i].lok_syu);
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].usLockOnTime = LockMaker[wk_mak-1].clse_tm * 10;
						IFM_LockTable.sSlave[tno-1].sLock[lok_no_local].usLockOffTime = LockMaker[wk_mak-1].open_tm * 10;
						cnt++;
						if( cnt>=LK_LOCK_MAX ){
							break;
						}
					}
				}
			}
		}
		for( i=0; i<LOCK_MAKER_END; i++ ){																// ���b�N���u���[�J�[�ʐM���o�͎���
			IF_SignalOut[i].usLockOnTime  = (uchar)(LockMaker[i].clse_tm * 10);							// close ����
			IF_SignalOut[i].usLockOffTime = (uchar)(LockMaker[i].open_tm * 10);							// open  ����
		}

		//���������CRB��PAY_InitDataCheck()�֐�����Q�l
		// �q�@(CRB)���L���b�N���u���A�S�āu�ڑ��Ȃ��v/�ǂꂩ�u�ڑ�����v�𔻒�
		for (tno=0; tno < t_no_max; tno++) {															// �q�@�̐���
			cnt = 0;// ���Z�݌v�l=0�Ȃ�A���̎q�@�͑S�Đڑ��Ȃ��Ɣ��f�ł���
			WACDOG;																						// ���u���[�v�̍ۂ�WDG���Z�b�g���s
			for (lock=0; lock < IFS_ROOM_MAX_USED; lock++) {											// ���̎q�@�̃��b�N���u�̐���
				cnt += IFM_LockTable.sSlave[tno].sLock[lock].ucConnect;
			}
			// �����ݒ�f�[�^��M����́A����Ԓʒm����Ώ�
			IFM_LockTable.sSlave[tno].sLock[lock].bChangeState = 0;										// (��x��)��ԕω��ʒm���Ă��Ȃ�
			// �����ݒ�f�[�^��M����́A�ԗ��E���b�N��Ԃ�����������Ώ�
			// �u�ڑ������v�̎Ԏ��ɑ΂��ď�ԕω��͖������ǁA��ԗv���͂��蓾��̂ŁE�E�E
			if (IFM_LockTable.sSlave[tno].sLock[lock].ucConnect == 0) {
				IFM_LockTable.sSlave[tno].sLock[lock].ucCarState  = 0;									// �ԗ���ԁ��u0�F�ڑ������v
				IFM_LockTable.sSlave[tno].sLock[lock].ucLockState = 6;									// ���b�N��ԁ��u6�F�w�胍�b�N���u�ڑ������v
			}
			// �q�@���L���b�N���u���A�S�āu�ڑ��Ȃ��v/�ǂꂩ�u�ڑ�����v�𔻒�
			if(cnt>0){
				IFM_LockTable.sSlave[tno].bSomeone = 1;
			}
			IFM_LockTable.sSlave[tno].bComm = IFM_LockTable.sSlave[tno].bSomeone;						// �q�@�֒ʐM����K�v����/�����𔻒�
			IFM_LockTable.sSlave[tno].bInitialACK = 0;													// �q�@���珉���ݒ�f�[�^�̐��퉞����Ԃ���Ă��Ȃ�
			IFM_LockTable.sSlave[tno].bInitialsndACK = 0;												// �N�����Ɏq�@�֏����ݒ�f�[�^�𑗐M���邽�߂̃t���O

			// �đ��J�E���^�A�X�L�b�v�J�E���^�͏����l���Z�b�g���K�v
			IFM_LockTable.sSlave[tno].ucNakRetry  = IFM_LockTable.toSlave.ucRetry;
			IFM_LockTable.sSlave[tno].ucSkipCount = IFM_LockTable.toSlave.ucSkipNoAnswer;
		}
	}

	// �t���b�v�̐ݒ�
	if(( GetCarInfoParam() & 0x04 )){									// �t���b�v�ڑ��L
		for( tno=0; tno < t_no_max; tno++ ){							// �^�[�~�i��No����������
			if( LKcom_Search_Ifno( (uchar)(tno + 1) ) == 1 ){					// �Y���^�[�~�i��No�̐ڑ��̓t���b�v
				IFM_LockTable.sSlave[tno].bInitialACK = 0;				// �q�@���珉���ݒ�f�[�^�̐��퉞����Ԃ���Ă��Ȃ�
				IFM_LockTable.sSlave[tno].bInitialsndACK = 1;			// �����ݒ�f�[�^�͑��M���Ȃ��̂ő��M�ς݂ɂ���
				IFM_LockTable.sSlave[tno].bInitialACK = 1;				// �����ݒ�f�[�^�͑��M���Ȃ��̂Ŏ�M�ς݂ɂ���
				
				for( i=INT_CAR_START_INDEX, cnt=0; i<TOTAL_CAR_LOCK_MAX; i++ ){
					WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
					if( (LockInfo[i].if_oya == tno + 1)&&(LockInfo[i].lok_syu != 0) ){				// �^�[�~�i��No��v���ڑ�����H
						IFM_LockTable.sSlave[tno].bSomeone = 1;										// �ڑ�����
						IFM_LockTable.sSlave[tno].bComm = IFM_LockTable.sSlave[tno].bSomeone;		// �q�@�֒ʐM����K�v����/�����𔻒�
						IFM_LockTable.sSlave[tno].sLock[0].ucConnect = 
							LCM_GetFlapMakerParam(LockInfo[i].lok_syu, &flapMaker);		// [0]�̂ݎg�p
						
						IFM_LockTable.sSlave[tno].usEntryTimer    = flapMaker.in_tm;	// ���Ɏԗ����m�^�C�}�[
						IFM_LockTable.sSlave[tno].usExitTimer     = flapMaker.ot_tm;	// �o�Ɏԗ����m�^�C�}�[
						IFM_LockTable.sSlave[tno].usRetryCount    = flapMaker.r_cnt;	// ���g���C��
						IFM_LockTable.sSlave[tno].usRetryInterval = flapMaker.r_tim;	// ���g���C�Ԋu

						IFM_LockTable.sSlave[tno].usFlapUpTime  = flapMaker.clse_tm * 10;	// close ����
						IFM_LockTable.sSlave[tno].usFlapDownTime = flapMaker.open_tm * 10;	// open  ����
						// CRR��̗L��/�����𔻒f����
						crr_no = LockInfo[i].lok_no / 100;
						point_no = LockInfo[i].lok_no % 100;
						if( LockInfo[i].lok_syu == LK_TYPE_CONTACT_FLAP && crr_no < IFS_CRR_MAX && point_no > 0 && point_no <= IFS_CRRFLAP_MAX){	// ���b�N��ʂ��ړ_�t���b�v(16) & CRR��� 0�`2 & �ړ_ 1�`15
							IFM_LockTable.sSlave_CRR[crr_no].bComm = 1;	// CRR��Ƀ^�[�~�i��No.�̊��t���L��ꍇ�A�L���ɂ���
							IFM_LockTable.ucConnect_Tbl[ crr_no ][ point_no - 1 ] = LockInfo[i].if_oya;
						}
						break;
					}
				}
			}
			else {
				// ���ڑ��܂��̓t���b�v�ȊO�̏ꍇ�̓X�e�[�^�X���N���A����
				memset(&IFM_FlapSensor[tno], -1, sizeof(IFM_FlapSensor[tno]));
			}
			
		}
		
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             LCM_FlapStatus()                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�q�@�����M�����u�q�@��ԃf�[�^�v�̕ω������m����B			   |*/
/*|		�E�Ύq�@�ʐM�^�X�N����L���[�ɂĖ{�^�X�N�֎�M���m�炳��܂��B	   |*/
/*|		�E�{�֐��ɂāA�ԗ��̓��o�Ɍ��m�A�����							   |*/
/*|		  ���b�N���u���쌋�ʂ����m���܂��B								   |*/
/*|		�E���m���ʂ͐��Z�@�֒ʒm���܂��B�i�����ݒ�f�[�^��M��Ɍ���j	   |*/
/*|		�E�u�q�@�o�[�W�����f�[�^�v�̌��m���A�{�֐��ɂčs���܂��B		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_FlapStatus(t_IFM_Queue *pStatus)
{
	int	i;
	uchar tno;
	uchar lockState = 0;
	uchar carState = 0;

	switch (pStatus->sCommon.ucKind) {
		case 'A':	/* �q�@��ԃf�[�^ */
			tno = pStatus->sCommon.ucTerminal;
			/* �ڑ������͏��O */
			if (IFM_LockTable.sSlave[tno].sLock[0].ucConnect == 0) {
				break;
			}

			// �Ώۃt���b�v��LockInfo�C���f�b�N�X�擾
			for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
				WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
				if( (LockInfo[i].if_oya == tno + 1)&&(LockInfo[i].lok_syu != 0) ){	// �^�[�~�i��No��v���ڑ�����H
					break;
				}
			}
			if(i >= TOTAL_CAR_LOCK_MAX) {
				return;
			}

			// ��ԕω����������fcmain�ɒʒm���Ȃ�
			if((0 == memcmp(&pStatus->sFlapA41.t_FlapSensor, 				// ����̏��(��M�f�[�^)
							&IFM_FlapSensor[tno],							// �O��̏��
							sizeof(pStatus->sFlapA41.t_FlapSensor))) &&
				IFM_LockTable.sSlave[tno].sLock[0].bChangeState == 1) {
				return;
			}
			IFM_LockTable.sSlave[tno].sLock[0].bChangeState = 1;

			// �O��̃X�e�[�^�X�ƍ�����r���u�ʏ��ԃf�[�^�v���쐬����
			lockState = 0;
			if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '1' &&		// ����
					IFM_FlapSensor[tno].c_LockPlate == '1' &&				// �O��F���~��
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '0') {	// ���݁F�ҋ@��
				lockState = 2;
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '2' &&	// ���
					IFM_FlapSensor[tno].c_LockPlate == '2' &&				// �O��F�㏸��
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '0') {	// ���݁F�ҋ@��
				lockState = 1;
			}
		
			if(IFM_FlapSensor[tno].c_LockPlate != '3' &&			// �G���[�܂��͕s�����b�N�ȊO����
				IFM_FlapSensor[tno].c_LockPlate != '4' &&
				IFM_FlapSensor[tno].c_LockPlate != '8') {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '3') {			// ���~�G���[
					lockState = 4;
				}
				else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '4' ||	// �㏸�G���[
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate == '8') {	// �s�����b�N
					lockState = 3;
				}
			}
					
			if(lockState != 0) {											
				/* ���슮�������̂ŁD�D�D*/
				IFM_ResetAction((uchar)tno, 0);			/* ���쒆������ */
				if (IFM_LockTable.usWaitCount) {
					IFM_LockTable.usWaitCount--;		/* ���슮���҂�����߂��i�J�E���g�_�E���j*/
				}

			   	LCMtim_10msTimerStop((int)tno, 0);			// �����ŁA���슮������܂ł̃^�C�}�[���� 

				// �㏸/���~�e�X�g�̌��ʔ���
				if (IFM_LockTable.bTest) {
					LCM_TestResult(lockState, (int)tno, 0);
				}

				if(lockState == 3 || lockState == 4) {		// �G���[�̓��g���C
					if (IFM_LockTable.sSlave[tno].sLock[0].usRetry && !IFM_LockTable.bTest)	// ���g���C�\�H
					{
						LCMtim_1secTimerStart(tno, 0, IFM_LockTable.sSlave[tno].usRetryInterval);	// ��������A���g���C���ԃ^�C�}�[�J�n 
					}
				}
				
				/* �ғ������J�E���g */
				LCM_CountAction(lockState, (int)tno, 0);

				/* ���슮�������̂ŁA����҂�������Ύ���v������ */
				LCM_NextAction();
			}
			
			// �ԗ����m��Ԃ̐ݒ�
			if(pStatus->sFlapA41.t_FlapSensor.c_LoopSensor != '0') {
				switch(pStatus->sFlapA41.t_FlapSensor.c_LoopSensor) {
				case '1':										// OFF���
				case '3':										// �ُ�
				case '4':										// ����OFF
				case '6':										// �s��
					carState = 2;								// �ԗ�����
					break;
				case '2':										// ON���
				case '5':										// ����ON
					carState = 1;								// �ԗ��L��
					break;
				case '0':										// �ڑ�����
				default:
					carState = 0;								// �ڑ�����
					break;
				}
			}
			else {
				switch(pStatus->sFlapA41.t_FlapSensor.c_ElectroSensor) {
				case '1':										// OFF���
					carState = 2;								// �ԗ�����
					break;
				case '2':										// ON���
					carState = 1;								// �ԗ��L��
					break;
				case '0':										// �ڑ�����
				default:
					carState = 0;								// �ڑ�����
					break;
				}
			}

			// ���b�N���u��Ԃ̐ݒ�
			if(IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_FORCE_DOWN ||				// �O�񃁃��e���[�h��
					IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_FORCE_UP) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_FORCE_DOWN &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_FORCE_UP) {
					// �O�񂪃����e���[�h���ŁA���݂������e���[�h�ȊO�Ȃ烁���e���[�h����
					lockState = 8;														// ��ԁF�����e���[�h����
					
					// �����e���[�h��������̏�Ԃ�ʒm���邽�߁A��ԕω����ʒm��Ԃɂ���
					// ����̏�ԃf�[�^��M�Œʒm���s���悤�ɂ���
					IFM_LockTable.sSlave[tno].sLock[0].bChangeState = 0;				// ��ԕω����ʒm�� */
				}
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_DOWN_ERR) {	// ���b�N�F���~�G���[
				lockState = 4;															// ��ԁF���~�ُ�
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_UP_ERR ||	// ���b�N�F�㏸�G���[
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_INVALID) {	// ���b�N�F�s�����b�N
				lockState = 3;															// ��ԁF�㏸�ُ�
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_WAIT) {		// ���b�N�F�ҋ@��
				if('1' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {				// �t���b�v�Z���T�F����
					lockState = 2;														// ��ԁF���~�ς�
				}
				else if('2' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {			// �t���b�v�Z���T�F���
					lockState = 1;														// ��ԁF�㏸�ς�
				}
				else if('0' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor ||			// �t���b�v�Z���T�F����
						'3' == pStatus->sFlapA41.t_FlapSensor.c_FlapSensor) {			// �t���b�v�Z���T�F�ُ�
					if(FLAPDT.flp_data[ i ].nstat.bits.b02 == 1 &&						// �㏸����
					   FLAPDT.flp_data[ i ].nstat.bits.b01 == 0) {						// ���~�ς�
						lockState = 3;													// ��ԁF�㏸�ُ�
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate = FLP_LOCK_UP_ERR;
					}
					if(FLAPDT.flp_data[ i ].nstat.bits.b02 == 0 &&						// ���~����
					   FLAPDT.flp_data[ i ].nstat.bits.b01 == 1) {						// �㏸�ς�
						lockState = 4;													// ��ԁF���~�ُ�
						pStatus->sFlapA41.t_FlapSensor.c_LockPlate = FLP_LOCK_DOWN_ERR;
					}
					else if(pStatus->sFlapA41.t_FlapSensor.c_FlapSensor == '0' &&		// �t���b�v�Z���T�F����
							lockState != 3) {											// ��ԁF�㏸�ُ� �ȊO
						lockState = 1;													// ��ԁF�㏸�ς�
					}
				}
			}
			else if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_DOWN || 		// ���b�N�F���~��
					pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_UP) {		// ���b�N�F�㏸��
				// �㏸���^���~���͒ʒm�̕K�v�͖������A�Z���T��Ԃ��ω����邩������Ȃ��̂ŁA
				// �t���b�v��Ԃ�0�Œʒm����
				lockState = 0;
			}

			if(IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_DOWN_ERR ||
			   IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_UP_ERR ||
			   IFM_FlapSensor[tno].c_LockPlate == FLP_LOCK_INVALID) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_DOWN_ERR &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_UP_ERR &&
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate != FLP_LOCK_INVALID) {
					lockState = 5;															// ��ԁF�̏����
				}
			}
			if(IFM_FlapSensor[tno].c_LockPlate != FLP_LOCK_FORCE_DOWN &&					// �O�񃁃��e���[�h�ȊO
			   IFM_FlapSensor[tno].c_LockPlate != FLP_LOCK_FORCE_UP) {
				if(pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_FORCE_DOWN ||		// ���񃁃��e���[�h
				   pStatus->sFlapA41.t_FlapSensor.c_LockPlate == FLP_LOCK_FORCE_UP) {
					// ���݂̏�Ԃ��������~�^�㏸�Ȃ烁���e���[�h���ֈȍ~
					lockState = 7;															// ��ԁF�����e���[�h��
				}
			}

			// ����̏�Ԃ�ޔ�����
			memcpy(&IFM_FlapSensor[tno], &pStatus->sFlapA41.t_FlapSensor, sizeof(pStatus->sFlapA41.t_FlapSensor));
			
			LCM_NoticeStatus_toPAY(tno + 1, (uchar)1, carState, lockState);
			
			break;
		case 'S':											// ���[�v�f�[�^����
			tno = pStatus->sCommon.ucTerminal;
			memcpy(&IFM_LockTable.sSlave[tno].tLoopCount, &pStatus->sFlapS53.t_LoopCounter, sizeof(IFM_LockTable.sSlave[tno].tLoopCount));
			
			if (IFM_LockTable.sSlave[tno].ucContinueCnt) {
				IFM_LockTable.sSlave[tno].ucContinueCnt--;
			}
			if (LCM_Is_AllOver(IFM_LockTable.ucOperation, (uchar)tno, 0)) {
				LCMtim_1secWaitStart(0);	/* �O�Z�b�g���ċ����Ƀ^�C���A�E�g���N�������� */
			}
			break;
		case 'v':	/* CRR�o�[�W�����f�[�^ */
			tno = pStatus->sCommon.ucTerminal - IFS_CRR_OFFSET;	// ���̎��_�ł�+100����Ă��邽�߁A������-100���K�v
			memcpy(IFM_LockTable.sSlave_CRR[tno].cVersion, &pStatus->sFlapv76.ucVersion, sizeof(IFM_LockTable.sSlave_CRR[tno].cVersion));
			break;
		case 't':	/* CRR�܂�Ԃ��e�X�g���� */
			memcpy( &MntLockTest[0],&pStatus->sFlapt74.ucTestResult, IFS_CRRFLAP_MAX );			/* �܂�Ԃ��e�X�g�̌��ʂ�n��	*/
			LCM_AnswerQueue_Timeout();
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestLoopData()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�t���b�v�ւ̃��[�v�f�[�^�v��                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_RequestLoopData(void)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;
	uchar			ni;

	//	���[�v�f�[�^���N���A����i��ʂɁu���������v��\�������Ԃɏ������j
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {			/* �q�@�̐��� */
		// �t���b�v�E���b�N�Ɋւ�炸�S��0xff�ɂ���
		memset(&IFM_LockTable.sSlave[ni].tLoopCount, 0xff, sizeof(IFM_LockTable.sSlave[ni].tLoopCount));
	}

	// �ȉ��̏�����toS_main()�֐�������p
	for (ni=0; ni < IF_SLAVE_MAX; ni++) {
		//	�ȉ��͏��O�B
		//	�E�ʐM����K�v���Ȃ��q�@�i���b�N���u���ڑ������j
		//	�E�����ݒ�f�[�^�𑗂����̂�ACK�������Ă��Ȃ��q�@
		//	�E�������X�L�b�v���̎q�@
		if (!IFM_LockTable.sSlave[ni].bComm ||
			!IFM_LockTable.sSlave[ni].bInitialACK ||
			IFM_LockTable.sSlave[ni].bSkip) {
			IFM_LockTable.sSlave[ni].ucContinueCnt = 0;
			continue;
		}
		/* �o�[�W�����v��/�����͂P�񂱂����� */
		else {
			IFM_LockTable.sSlave[ni].ucContinueCnt = 1;
		}

		/* �󂫃L���[�݂邩�H*/
		pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
		pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
		if (pQbody && pQbuff) {
			pQbuff->pData = pQbody;						/* ���̂������N */
			/* �L���[�t��[����]���� */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
		}
		/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
		else {
			/* �L���[�t��[����] */
			IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
			pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
			pQbody = pQbuff->pData;
		}
		/* ���[���Łu��ԗv���f�[�^�v��������A����́w�o�[�W�����v���x���ƌ��ߑł��ŗǂ� */
		pQbody->sCommon.ucKind     = 'R';				/* ��ԗv���f�[�^�i�o�[�W�����v���j*/
		pQbody->sCommon.ucTerminal = ni;				/* �^�[�~�i��No.�i�ǂ̎q�@�ւ��H����肷��j*/
		pQbody->sR52.ucRequest     = 2;					/* �v�����e�́A���[�v�f�[�^�v�� */
		/* �����փL���[�C���O */
		EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
	}
	/* �o�[�W�����v�����o���̂ŁA���̉������Ԃ����܂ł̐������Ԃ����߂� */
	LCMtim_1secWaitStart(IFS_VERSION_WAIT_TIME);
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_RequestForceLoopControl()                              |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�������[�vON/OFF�v��                                               |*/
/*|		�E�u�������[�vON/OFF�v���v���w��t���b�v�ɓ]������B               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	LCM_RequestForceLoopControl(uchar ucOperation, uchar ucSlave)
{
	t_IFM_Queue			*pQbody;
	struct t_IF_Queue	*pQbuff;

	if( LKcom_Search_Ifno( (uchar)(ucSlave + 1) ) == 0 ){		// �Y���^�[�~�i��No�̐ڑ��̓��b�N���u
		return;
	}

	// �������[�vON/OFF�́A�t���b�v�̏�ԂɊ֌W���������D��L���[�Ɋi�[����
	
	/* �󂫃L���[�݂邩�H*/
	pQbody = GetQueueBody(toSque_Body_Select, sizeof(toSque_Body_Select));	/* �󂫎��̂��擾 */
	pQbuff = GetQueueBuff(toSque_Buff_Select, sizeof(toSque_Buff_Select));	/* �󂫖{�̂��擾 */
	if (pQbody && pQbuff) {
		pQbuff->pData = pQbody;						/* ���̂������N */
		/* �L���[�t��[����]���� */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_RECOVER);
	}
	/* �󂫃L���[�����Ȃ�A�擪(�Ō�)���O���Ďg���i�Â����͎̂̂Ă�j*/
	else {
		/* �L���[�t��[����] */
		IFM_MasterError(E_IFS_QFULL_SelectingWait, E_IF_OCCUR);
		pQbuff = DeQueue(&toSque_Ctrl_Select, 0);	/* �L���[(�擪)���O�� */
		pQbody = pQbuff->pData;
	}
	/* ���[���Łu��ԗv���f�[�^�v��������A����́w�[�W�����v���x���ƌ��ߑł��ŗǂ� */
	pQbody->sCommon.ucKind     = 'W';				/* ��ԗv���f�[�^�i�o�[�W�����v���j	*/
	pQbody->sW57.usLockOffTime = 0;					/* 0�Œ�							*/
	pQbody->sW57.usLockOnTime  = 0;					/* 0�Œ�							*/
	pQbody->sW57.ucLock        = 0;					/* 0�Œ�							*/
	pQbody->sW57.ucMakerID     = 0;					/* 0�Œ�							*/
	pQbody->sCommon.ucTerminal = ucSlave;			/* �^�[�~�i��No.�i�ǂ̎q�@�ւ��H����肷��j*/
	if(ucOperation == 16) {
		pQbody->sW57.unCommand.Byte = '2';			/* �v�����e�́A�o�[�W�����v�� */
	}
	else {
		pQbody->sW57.unCommand.Byte = '1';			/* �v�����e�́A�o�[�W�����v�� */
	}	

	pQbody->sW57.ucOperation = ucOperation;			/* ���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩 */
	pQbody->sW57.bWaitRetry  = 0;					/* ��������i���̓���v���́A���񂩃��g���C���j*/

	/* �����փL���[�C���O */
	EnQueue(&toSque_Ctrl_Select, pQbuff, (-1));
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_HasFlapForceControl()                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�����㏸/���~���쒆�̃t���b�v�����݂��邩�B						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
BOOL	LCM_HasFlapForceControl()
{
	uchar	tno;
	
	for( tno=0; tno < LOCK_IF_MAX; ++tno ){							// �^�[�~�i��No����������
		if( LKcom_Search_Ifno( (uchar)(tno + 1) ) == 1 ){			// �Y���^�[�~�i��No�̐ڑ��̓t���b�v
			if(IFM_FlapSensor[tno].c_LockPlate == 5 || 				// �������~��
			   IFM_FlapSensor[tno].c_LockPlate == 6) {				// �����㏸��
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             LCM_GetFlapMakerParam()                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�w��t���b�v��ʂ̃p�����[�^���擾����B						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam)
{
	long	i;
	
	memset(pMakerParam, 0, sizeof(t_LockMaker));
	
	for(i = 0; i < LCM_FLAP_MAKER_COUNT; ++i) {
		if(LCMFlapmaker_rec[i].adr == type) {
			memcpy(pMakerParam, &LCMFlapmaker_rec[i].dat, sizeof(t_LockMaker));
			break;
		}
	}
	
	if(i == LCM_FLAP_MAKER_COUNT) {
		return 0;
	}

	return (uchar)(i + 1);
}
/*[]----------------------------------------------------------------------[]*/
/*|     LCM_CanFlapCommand(index, direction)                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     param:	index	LockInfo�̃C���f�b�N�X							   |*/
/*|     		direction	1: �t���b�v�㏸								   |*/
/*|     					0: �t���b�v���~								   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�w�肳�ꂽLockInfo�C���f�b�N�X�̃t���b�v������\���`�F�b�N����   |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	LCM_CanFlapCommand(ushort index, uchar direction)
{
	ushort	tno; 
	t_IF_FlapSensor* pFlapState;
	
	tno = LockInfo[index].if_oya;
	if( tno > LOCK_IF_MAX ) {
		return FALSE;
	}

	if( LKcom_Search_Ifno( tno ) == 0 ){				// �Y���^�[�~�i��No�̐ڑ��̓��b�N���u
		return TRUE;									// ���b�N���u�̓R�}���h���s�\
	}
	
	pFlapState = &IFM_FlapSensor[LockInfo[index].if_oya - 1];
	
	if( pFlapState->c_LockPlate == FLP_LOCK_DOWN ||		// ���~��
		pFlapState->c_LockPlate == FLP_LOCK_UP ) {		// �㏸��
		return FALSE;									// �㏸�܂��͉��~���͑���s��
	}
	
	if( pFlapState->c_LockPlate == FLP_LOCK_WAIT) {		// �ҋ@��
		if( direction == 0 && 							// ���~�R�}���h�v��
			pFlapState->c_FlapSensor == '1' ) {			// ���݉��~
			return FALSE;								// ���������͓���s��
		}
		else if(direction == 1 &&						// �㏸�R�}���h�v��
			pFlapState->c_FlapSensor == '2' ) {			// ���݉��~
			return FALSE;								// ���������͓���s��
		}
	}
	return TRUE;										// �R�}���h���s�\
}
