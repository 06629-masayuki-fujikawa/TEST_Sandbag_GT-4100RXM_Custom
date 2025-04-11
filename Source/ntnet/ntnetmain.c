// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A08(S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
	// UT4000�� "ntnetmain.c"���x�[�X��FT4000�̍������ڐA���đΉ�
// MH364300 GG119A08(E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *| System      : FT4000
 *| Module      : NT-NET���C������
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"mnt_def.h"
#include	"rau.h"
#include	"ntcom.h"


static	void	NTNET_SendMain(void);
static	void	NTNET_ReceiveMain(void);
static	void	NTNET_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl);
static	void	NTNET_EraseSendDataForce(t_Ntnet_SendDataCtrl* pCtrl);


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET�f�[�^�������C������
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Main(void)
{
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
//// MH341110(S) A.Iiizumi 2017/11/14 �V���ݏ����@�\(GG107200)
//	Multi_lockctrl_timeout();
//// MH341110(E) A.Iiizumi 2017/11/14 �V���ݏ����@�\(GG107200)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
	NTNET_SendMain();									// ���M����
	
	NTNET_ReceiveMain();								// ��M����
	
	// ���Z�f�[�^�o�b�t�@�t���`�F�b�N
	if(LOG_DAT[eLOG_PAYMENT].writeFullFlag[eLOG_TARGET_NTNET] == 1) {	// �o�b�t�@�t���`�F�b�N���K�v���H
		Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_NTNET);
		LOG_DAT[eLOG_PAYMENT].writeFullFlag[eLOG_TARGET_NTNET] = 0;		// �o�b�t�@�t���`�F�b�N�t���OOFF
	}
	// �s���v�f�[�^�o�b�t�@�t���`�F�b�N
	if(LOG_DAT[eLOG_TTOTAL].writeFullFlag[eLOG_TARGET_NTNET] == 1) {	// �o�b�t�@�t���`�F�b�N���K�v���H
		Log_CheckBufferFull(TRUE, eLOG_TTOTAL, eLOG_TARGET_NTNET);
		LOG_DAT[eLOG_TTOTAL].writeFullFlag[eLOG_TARGET_NTNET] = 0;		// �o�b�t�@�t���`�F�b�N�t���OOFF
	}

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_SendMain
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET�f�[�^���M�������C������
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_SendMain(void)
{
	ushort	size = 0;									// NTNET�f�[�^�T�C�Y
	ushort	ret;
	t_Ntnet_SendDataCtrl* pCtrl = NULL;					// ���M����f�[�^
	
	// �D��f�[�^�擾
	pCtrl = &NTNET_SendCtrl[0];
	if(pCtrl->length == 0) {							// �đ��҂��f�[�^����
		size = NTBUF_GetSndNtData(pCtrl, NTNET_BUF_PRIOR);
		pCtrl->length = size;
	}
	else {												// �đ��f�[�^����
		size = pCtrl->length;
	}
	if(size == 0) {										// �D��f�[�^����
		// �ʏ�f�[�^�擾
		pCtrl = &NTNET_SendCtrl[1];
		if(pCtrl->length == 0) {					// �đ��҂��f�[�^����
			size = NTBUF_GetSndNtData(pCtrl, NTNET_BUF_NORMAL);
			pCtrl->length = size;
		}
		else {										// �đ��f�[�^����
			size = pCtrl->length;
		}
	}

	if(NTCom_Condit) {									// NTCom�^�X�N�N�����
		if(size && pCtrl) {								// ���M�f�[�^����
			ret = NTCom_SetSendData(pCtrl->data, size, pCtrl->type);
			switch(ret) {
			case NTNET_NORMAL:							// NTNET�ʐM���̃o�b�t�@�Ɋi�[
				NTNET_EraseSendData(pCtrl);				// ���M�f�[�^���폜����
				break;
			case NTNET_ERR_BUFFERFULL:					// �o�b�t�@�t��
				// �o�b�t�@�t���̏ꍇ�͉������Ȃ�
				break;
			case NTNET_ERR_BLOCKOVER:					// �u���b�N�I�[�o�[
			default:
				// arcnet����Ȃ����߁A�u���b�N�I�[�o�[�̏ꍇ�͑��M�����f�[�^�����������̂ŁA
				// ���g���C�����ɃG���[�Ƃ��폜����
				NTNET_EraseSendDataForce(pCtrl);		// ���M�f�[�^�폜
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_ReceiveMain
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET�f�[�^��M�������C������
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_ReceiveMain(void)
{
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^���폜����
 *|				���O�̏ꍇ�͑��M�ςɂ���
 *|				���M�o�b�t�@�f�[�^�̏ꍇ�͉������Ȃ�
 *| param	: pCtrl		: �폜���鑗�M�f�[�^�̐���f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl)
{
	NTBUF_EraseSendData(pCtrl, FALSE);					// ���M�f�[�^�폜
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendDataForce
 *[]----------------------------------------------------------------------[]
 *| summary	: T���v�ȂǕ����f�[�^�̑��M�r���ł����M�f�[�^�폜����
 *|				���O�̏ꍇ�͑��M�ςɂ���
 *|				���M�o�b�t�@�f�[�^�̏ꍇ�͉������Ȃ�
 *| param	: pCtrl		: �폜���鑗�M�f�[�^�̐���f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_EraseSendDataForce(t_Ntnet_SendDataCtrl* pCtrl)
{
	NTBUF_EraseSendData(pCtrl, TRUE);					// ���M�f�[�^�폜(����)
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_SetReceiveData
 *[]----------------------------------------------------------------------[]
 *| summary	: �ʐM���̎�M�f�[�^�i�[API
 *| param	: pNtData	: �o�b�t�@�Ɋi�[����NTNET�f�[�^
 *| 		  size		: NTNET�f�[�^�T�C�Y
 *| 		  type		: �i�[��o�b�t�@���
 *| return	: TRUE - ���MWAIT��
 *[]----------------------------------------------------------------------[]*/
eNTNET_RESULT	NTNET_SetReceiveData(const uchar* pNtData, ushort size, uchar type)
{
	eNTNET_BUF_KIND kind;
	
	if(!pNtData || size == 0) {
		return NTNET_ERR_OTHER;
	}
	
	switch(type) {
	case NTNET_PRIOR_DATA:								// �D��f�[�^
		kind = NTNET_BUF_PRIOR;
		break;
	case NTNET_NORMAL_DATA:								// �ʏ�f�[�^
		kind = NTNET_BUF_NORMAL;
		break;
	case NTNET_BROADCAST:								// ����f�[�^
		kind = NTNET_BUF_BROADCAST;
		break;
	default:
		return NTNET_ERR_OTHER;
	}
	
	NTBUF_SetRcvNtData((void*)pNtData, size, kind);		// �Ή�����o�b�t�@�֏�������
	
	// �o�b�t�@�֊i�[����^���Ȃ��Ɋւ�炸����I��
	return NTNET_NORMAL;
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
