// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/************************************************************************************/
/*																					*/
/*		�V�X�e�����@:�@NT-NET�ʐM��													*/
/*																					*/
/*		�t�@�C����	:  ntmain.c														*/
/*		�@�\		:  ���C�����[�v����												*/
/*																					*/
/************************************************************************************/
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/********************* FUNCTION DEFINE **************************/
static BOOL	NTCom_MainRecvProcess(void);
static BOOL	NTCom_NtnetRecvProcess(void);
static BOOL	NTCom_IsBroadcastBlock(void);
static BOOL	NTCom_IsPriorBlock(void);
static BOOL	NTCom_IsNormalBlock(void);


/*********************** EXTERN DATA ****************************/
T_NT_BLKDATA	NTComRecvData;								// ��M�f�[�^�|�C���^


/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_FuncStart() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		NTCom function start. 											   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	This routine will be called after Initial setting data receiveing.	   |*/
/*| same mean as initialize process.									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTCom_FuncStart( void )
{
	/* NT-NET�d���Ǘ��@�\������ */
	NTComData_Start();
	/* �^�C�}�[������ */
	NTComTimer_Init();
	/* SCI�h���C�o������ */
	NTComComdr_Init(NTCom_InitData.Time_t5, NTCom_SciPort, NTCom_InitData.Baudrate,
				NT_SCI_DATABIT, NT_SCI_STOPBIT, NT_SCI_PARITY);
	/* NT-NET�v���g�R���w������ */
	NTComComm_Start();

	NTCom_Condit = 1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_FuncMain()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		credit function Main routine									   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	This routine will be called evetytime task active in IDLE phase.       |*/
/*| (Called only Main task phase is IDLE)								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTCom_FuncMain( void )
{
	// �^�C�}�[�`�F�b�N����
	NTCom_TimoutChk();

	/* NT-NET���� */
	NTComComm_Main();

	// OPE�w����̎�M����
	if(NTCom_MainRecvProcess()) {
		return;
	}

	// NT-NET����̎�M����
	if(NTCom_NtnetRecvProcess()) {
		return;
	}

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_MainRecvProcess
 *[]----------------------------------------------------------------------[]
 *| summary	: OPE�w����̎�M�������s��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static BOOL	NTCom_MainRecvProcess(void)
{
	eNT_DATASET		result = NT_DATA_NORMAL;
	ushort			size;

	if(NTCom_SndBuf_Prior.fixed == FALSE) {
		size = NTCom_GetSendData(NTNET_PRIOR_DATA, &NTComRecvData);
		if(size) {											// �D���M�f�[�^����
			result = NTComData_SetSndBlock(&NTComRecvData);
			if(result == NT_DATA_NORMAL) {
				return TRUE;
			}
		}
	}
	if(NTCom_SndBuf_Normal.fixed == FALSE) {
		size = NTCom_GetSendData(NTNET_NORMAL_DATA, &NTComRecvData);
		if(size) {											// �ʏ��M�f�[�^����
			result = NTComData_SetSndBlock(&NTComRecvData);
			if(result == NT_DATA_NORMAL) {
				return TRUE;
			}
		}
	}
	if (result == NT_DATA_NO_MORE_TELEGRAM) {
		/* �u���b�NNo.����(�A���o�b�t�@�ɖ����M�̓d��������ۑ��ł��Ȃ�)��ʒm */
	} else if (result == NT_DATA_BLOCK_INVALID1) {
		/* NT-NET�u���b�NNo.�ُ�ɂ��f�[�^�j���i���񖳌��j */
		NTCom_err_chk(NT_ERR_BLK_ARC1, 2, 0, 0, NULL);
	} else if (result == NT_DATA_BLOCK_INVALID2) {
		/* NT-NET�u���b�NNo.�ُ�ɂ��f�[�^�j���i����L���j */
		NTCom_err_chk(NT_ERR_BLK_ARC2, 2, 0, 0, NULL);
	}
	else if (result == NT_DATA_BUFFERFULL) {
		/* �o�b�t�@���e�ʕ���NT-NET�u���b�N����M�������ŏI�u���b�N�t���O��������Ȃ� */
		NTCom_err_chk(NT_ERR_BLK_OVER_ARC, 2, 0, 0, NULL);
	}

	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_NtnetRecvProcess
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET����̎�M�������s��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static BOOL	NTCom_NtnetRecvProcess(void)
{
	eNTNET_RESULT	result;
	ushort			HeaderToID3 = sizeof(t_NtNetBlkHeader) + 2;

	/* NT-NET�����M�����u���b�N�f�[�^�������OPE�ɒʒm */
	if (NTCom_IsBroadcastBlock()) {
		// ����f�[�^����
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len + 3, NTNET_BROADCAST);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Broadcast.telegram);
		}
		return TRUE;
	}
	if (NTCom_IsPriorBlock()) {
		// �D��f�[�^����
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len + 3, NTNET_PRIOR_DATA);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Prior.telegram);
		}
		return TRUE;
	}
	if (NTCom_IsNormalBlock()) {
		// �ʏ�f�[�^����
//		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len - 2, NTNET_NORMAL_DATA);
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[0], NTCom_SndWork.len, NTNET_NORMAL_DATA);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Normal.telegram);
		}
		return TRUE;
	}

	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_err_chk
 *[]----------------------------------------------------------------------[]
 *| summary	: ���W���[��ID���������ʂ��A�G���[�o�^���s��
 *[]----------------------------------------------------------------------[]*
 *|	param	: code		�G���[�R�[�h
 *|			  kind		0=���� 1=���� 2=��������
 *|			  f_data	0=�G���[��񖳂� 1=�G���[���L��(ascii) 2:(bin)
 *|			  err_ctl	0:�������`�F�b�N���� 1:���Ȃ�
 *|			  pData		�o�^����t���f�[�^
 *[]----------------------------------------------------------------------[]*
 *|	return	: void
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTCom_err_chk(char code, char kind, char f_data, char err_ctl, void *pData)
{
	ushort	errKind;

	switch (code) {
	case NT_ERR_BLK_ARC1:
	case NT_ERR_BLK_ARC2:
	case NT_ERR_BLK_OVER_ARC:
		errKind = ERRMDL_NTNET;
		break;
	default:
		errKind = ERRMDL_NTNETIBK;
		break;
	}

	switch (kind) {
	case 1:													/* ���� */
		err_chk2(errKind, (char)code, 1, 0, 0, pData);
		break;
	case 2:													/* ���������� */
		err_chk2(errKind, (char)code, 2, 0, 0, pData);
		break;
	default:	/* case NTERR_RELEASE: */					/* ���� */
		err_chk2(errKind, (char)code, 0, 0, 0, pData);
		break;
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsBroadcastBlock() 	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPE�ɒʒm����NT-NET����f�[�^�u���b�N�d�������邩�`�F�b�N 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsBroadcastBlock(void)
{
	/* ���M�r���̓d�������邩�`�F�b�N */
	if (!NTComData_IsValidTelegram(&NTCom_Tele_Broadcast.telegram)) {
	/* �Ȃ���΁A���M�\�ȓd�������邩�`�F�b�N */
		if (!NTComData_PeekRcvTele_Broadcast(&NTCom_Tele_Broadcast.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Broadcast.ntblkno_retry = 0;		/* #005 */
	}
	/* ���M�u���b�N�f�[�^���擾���� */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Broadcast.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Broadcast.telegram);
		return FALSE;			/* �f�[�^�擾�G���[(���̃p�X�͒ʂ�Ȃ��͂�) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsPriorBlock()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPE�ɒʒm����NT-NET�D��f�[�^�u���b�N�d�������邩�`�F�b�N 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsPriorBlock(void)
{
	/* ���M�r���̓d�������邩�`�F�b�N */
	if (!NTComData_IsValidTelegram(&NTCom_Tele_Prior.telegram)) {
	/* �Ȃ���΁A���M�\�ȓd�������邩�`�F�b�N */
		if (!NTComData_PeekRcvTele_Prior(&NTCom_Tele_Prior.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Prior.ntblkno_retry = 0;		/* #005 */
	}
	/* ���M�u���b�N�f�[�^���擾���� */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Prior.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Prior.telegram);
		return FALSE;			/* �f�[�^�擾�G���[(���̃p�X�͒ʂ�Ȃ��͂�) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsNormalBlock() 	 	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPE�ɒʒm����NT-NET�ʏ�f�[�^�u���b�N�d�������邩�`�F�b�N 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsNormalBlock(void)
{

	if (!NTComData_IsValidTelegram(&NTCom_Tele_Normal.telegram)) {
	/* �Ȃ���΁A���M�\�ȓd�������邩�`�F�b�N */
		if (!NTComData_PeekRcvTele_Normal(&NTCom_Tele_Normal.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Normal.ntblkno_retry = 0;		/* #005 */
	}
	/* ���M�u���b�N�f�[�^���擾���� */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Normal.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Normal.telegram);
		return FALSE;			/* �f�[�^�擾�G���[(���̃p�X�͒ʂ�Ȃ��͂�) */
	}

	memset(&NTComOpeSendNormalDataBuf, 0, sizeof(NTComOpeSendNormalDataBuf));

	return TRUE;
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
