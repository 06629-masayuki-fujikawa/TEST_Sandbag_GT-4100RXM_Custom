/************************************************************************************/
/*																					*/
/*		�V�X�e�����@:�@RAU�V�X�e��													*/
/*																					*/
/*		�t�@�C����	:  raumain.c													*/
/*		�@�\		:  ���C�����[�v����												*/
/*																					*/
/************************************************************************************/

#include	<string.h>
#include	"system.h"
#include	"mem_def.h"
#include	"raudef.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"rau.h"
#include	"rauIDproc.h"
#include	"ntnet_def.h"
#include	"ntnet.h"

/********************* FUNCTION DEFINE **************************/
extern void RAU_Mh_sdl(void);
extern void	Mh_HostNonePolAlmCountUp(void);
extern void	Mt_TermNonePolAlmCountUp(void);

/*********************** EXTERN DATA ****************************/

static	uchar	RAUmainState = 0;

void	RAU_MainRecvProcess(void);
static	BOOL	RAU_SetRevNtData(void);
uchar	RAU_TempRecvNtData[NTNET_BLKDATA_MAX];
uchar			RAU_CheckNearfullState = 1;			// 1:�j�A�t���`�F�b�N���s��
void	RAU_LogCheckState(void);

/****************************************************************/
/*	�֐���		=	RAU_FuncMain								*/
/*	�@�\		=	�펞�N����									*/
/*  ���C�������C�j�V�����N������ �펞�N�����Ɋ֐��𕪂���		*/
/*  �{�֐��� RAU�^�X�N�N�����ɖ���Call�����(IDLE��)			*/
/****************************************************************/
void	RAU_FuncMain( void )
{
	switch(RAUmainState) {
	case 0:									// �j�A�t����ԃ`�F�b�N
		if(RAU_CheckNearfullState == 1) {	// 1:�j�A�t���`�F�b�N���s��
			RAUdata_CheckNearFullLogData();
			RAU_CheckNearfullState = 0;
		}
		++RAUmainState;
		break;
	case 1:
		// OPE�w����̎�M����
		RAU_MainRecvProcess();
	
		// �X���[�f�[�^����
		RAU_SetRevNtData();

		// HOST�� ��M�f�[�^�}�g���N�X����
		RAU_Mh_sdl();
		++RAUmainState;
		break;
	case 2:
		RAU_LogCheckState();
		RAUmainState = 0;
		break;
	default:
		RAUmainState = 0;
		break;
	}

}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_MainRecvProcess
 *[]----------------------------------------------------------------------[]
 *| summary	: OPE�w����̎�M�������s��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_MainRecvProcess(void)
{
	t_RemoteNtNetBlk*	pRecvData;							// ��M�f�[�^�|�C���^
	ushort	size;											// ��M�f�[�^�T�C�Y
	eRAU_TABLEDATA_SET result;								// ��MNT-NET�u���b�N���
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	
	pRecvData = (t_RemoteNtNetBlk*)RAU_GetSendNtData();
	if(pRecvData) {											// ��M�f�[�^����
		u.uc[0] = pRecvData->header.packet_size[0];
		u.uc[1] = pRecvData->header.packet_size[1];
		size = u.us;
		result = (eRAU_TABLEDATA_SET)RAUid_SetNtBlockData(pRecvData, size);
		// ���ʂ��r����
		if (result == RAU_DATA_BLOCK_INVALID1) {
			// NT-NET�u���b�NNo�ُ�i���񖳌��j0x85
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
			RAU_err_chk(ERR_NTNET_NTBLKNO_VALID, 2, 0, 0, NULL);
		
		} else if (result == RAU_DATA_BLOCK_INVALID2) {
			// NT-NET�u���b�NNo�ُ�i����L���j0x86
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
			RAU_err_chk(ERR_NTNET_NTBLKNO_INVALID, 2, 0, 0, NULL);
		
		} else if (result == RAU_DATA_CONNECTING_FROM_HOST) {
			// HOST����̃R�l�N�V�������ɂ��f�[�^��M�s�� 0x90
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
		
		} else if (result >= RAU_DATA_BUFFERFULL) {
			// ��M�o�b�t�@�t���@�f�[�^�j���ɂ��đ��v���@0x82
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);

			// �t���̃o�b�t�@���ƂɃG���[�𑗐M�B

		} else {
			// ��L�̏����Ɉ�v���Ȃ�������A��������
			
			// ARC�u���b�N����M���ʁ@�u����v�𑗐M�
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 0);
		}
	}
	
	return;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_err_chk
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
void	RAU_err_chk(char code, char kind, char f_data, char err_ctl, void *pData)
{
	char	module;									// ���W���[��ID
// MH322914 (s) kasiyama 2016/07/12 ���uNT-NETLAN�ڑ����̃G���[�R�[�h��FOMA�ڑ����̃G���[�R�[�h�ɍ��킹��[���ʃo�ONo.1200](MH341106)
//	char	errCode;
//
//	if(RauConfig.modem_exist == 0) {				// ���f��
//		module = ERRMDL_FOMAIBK;
//		switch(code){
//		case ERR_RAU_DPA_BLK_VALID_D:				// �G���[�R�[�h151(��������M�f�[�^�u���b�N�ԍ��ُ�[����L��])
//			errCode = ERR_RAU_FMA_BLK_VALID_D;
//			break;
//		case ERR_RAU_DPA_BLK_INVALID_D:				// �G���[�R�[�h152(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
//			errCode = ERR_RAU_FMA_BLK_INVALID_D;
//			break;
//		case ERR_RAU_DPA_RECV_CRC_D:				// �G���[�R�[�h154(��������M�f�[�^�b�q�b�G���[)
//			errCode = ERR_RAU_FMA_RECV_CRC_D;
//			break;
//		case ERR_RAU_DPA_RECV_LONGER_D:				// �G���[�R�[�h155(��������M�f�[�^���ُ�)
//			errCode = ERR_RAU_FMA_RECV_LONGER_D;
//			break;
//		case ERR_RAU_DPA_RECV_SHORTER_D:			// �G���[�R�[�h156(��������M�f�[�^���ُ�)
//			errCode = ERR_RAU_FMA_RECV_SHORTER_D;
//			break;
//		case ERR_RAU_DPA_SEND_LEN_D:				// �G���[�R�[�h192(���������M�f�[�^���I�[�o�[)
//			errCode = ERR_RAU_FMA_SEND_LEN_D;
//			break;
//		case ERR_RAU_DPA_RECV_LEN_D:				// �G���[�R�[�h193(��������M�f�[�^���I�[�o�[)
//			errCode = ERR_RAU_FMA_RECV_LEN_D;
//			break;
//		case ERR_RAU_DPA_SEND_RETRY_OVER_D:			// �G���[�R�[�h195(�������f�[�^�đ��񐔃I�[�o�[)
//			errCode = ERR_RAU_FMA_SEND_RETRY_OVER_D;
//			break;
//		case ERR_RAU_DPA_RECV_NAK99_D:				// �G���[�R�[�h197(���������M�f�[�^������~��M)
//			errCode = ERR_RAU_FMA_RECV_NAK99_D;
//			break;
//		default:
//			errCode = code;
//			break;
//		}
//		err_chk2(module, errCode, kind, f_data, err_ctl, pData);
//	}
//	else {											// Ethernet
//		module = ERRMDL_LANIBK;
//		err_chk2(module, code, kind, f_data, err_ctl, pData);
//	}
	if(RauConfig.modem_exist == 0) {				// ���f��
		module = ERRMDL_FOMAIBK;
	}
	else {											// Ethernet
		module = ERRMDL_LANIBK;
	}
	err_chk2(module, code, kind, f_data, err_ctl, pData);
// MH322914 (e) kasiyama 2016/07/12 ���uNT-NETLAN�ڑ����̃G���[�R�[�h��FOMA�ڑ����̃G���[�R�[�h�ɍ��킹��[���ʃo�ONo.1200](MH341106)
}

/*[]----------------------------------------------------------------------[]*
 *|     RAU_SetRevNtData
 *[]----------------------------------------------------------------------[]*
 *|	���M�\��NT-NET�f�[�^�̗L�����`�F�b�N
 *[]----------------------------------------------------------------------[]*
 *|	param	: none
 *[]----------------------------------------------------------------------[]*
 *|	return	: TRUE =���M�\��NT-NET�f�[�^�L��
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	BOOL	RAU_SetRevNtData(void)
{
	ushort	index;
	ushort	headerLen;
	uchar*	pNtRecv;
	t_RemoteNtNetBlk* pRemoteData;
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	
	// ���M�r���̓d�������邩�`�F�b�N
	if (RAUarc_SndNtBlk.ntblknum == RAUARC_NODATA) {
		RAUarc_SndNtBlk.ntblknum = RAUdata_GetRcvNtDataBlkNum();
		if (RAUarc_SndNtBlk.ntblknum <= 0) {
			RAUarc_SndNtBlk.ntblknum = RAUARC_NODATA;
			return FALSE;
		}
		RAUarc_SndNtBlk.snd_ntblknum = 0;
		RAUarc_SndNtBlk.ntblkno_retry = 0;

	}
	
	/* ���M�u���b�N�f�[�^���擾���� */
	
	index = 0;
	headerLen = (ushort)(sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX);
	memset(&RecvNtnetDt, 0x00, sizeof(RECV_NTNET_DT));
	pNtRecv = (uchar*)&RecvNtnetDt;
	while(RAUarc_SndNtBlk.ntblknum != RAUARC_NODATA) {
		memset(RAU_TempRecvNtData, 0, sizeof(RAU_TempRecvNtData));
		
		RAUarc_SndNtBlk.ntblk.len = RAUdata_GetRcvNtBlkData(RAUarc_SndNtBlk.ntblk.data, RAUarc_SndNtBlk.snd_ntblknum+1);
		--RAUarc_SndNtBlk.ntblknum;
		//NT-NET��M�o�b�t�@�Ɋi�[
		pRemoteData = (t_RemoteNtNetBlk*)RAUarc_SndNtBlk.ntblk.data;
		u.uc[0] = pRemoteData->header.len_before0cut[0];
		u.uc[1] = pRemoteData->header.len_before0cut[1];
		if(pRemoteData->data.blk_no == 1) {				// �擪�u���b�N
			// �u���b�NNo.��1�Ȃ��NTNET�f�[�^�̐擪�������쐬����
			memcpy(pNtRecv, &pRemoteData->data.system_id, 3);	// �V�X�e��ID�`�f�[�^�ێ��t���O�܂ŃR�s�[
			index += 3;
			memcpy(RAU_TempRecvNtData, &pRemoteData->data.data, RAUarc_SndNtBlk.ntblk.len - headerLen);
		}
		else {
			if(RAUarc_SndNtBlk.ntblk.len > headerLen) {	// �S��0�J�b�g����Ă��Ȃ�
				memcpy(RAU_TempRecvNtData, &pRemoteData->data.data, RAUarc_SndNtBlk.ntblk.len - headerLen);
			}
		}
		memcpy(pNtRecv + index, RAU_TempRecvNtData, u.us);
		index += NTNET_BLKDATA_MAX;					// ���̏����݃C���f�b�N�X
		++RAUarc_SndNtBlk.snd_ntblknum;
		
		if(RAUarc_SndNtBlk.ntblknum == 0) {
			RAUdata_DelRcvNtData();						// �S�ď��������̂Ńf�[�^�폜
			NTNET_CtrlRecvData();						// ��M�f�[�^�̉�͂Ə������s��
			RAUarc_SndNtBlk.ntblknum = RAUARC_NODATA;
		}
	}

	return TRUE;
}

void	RAU_RequestCheckNearfull(void)
{
	RAU_CheckNearfullState = 1;
}
void	RAU_LogCheckState(void)
{
	short	Lno;									// ���OID
	BOOL	strage;									// FROM/SRAM
	uchar	nearfullCheck = 0;						// �j�A�t���`�F�b�N�t���O
	
	for(Lno = eLOG_PAYMENT; Lno < eLOG_MAX; ++Lno) {
		if(LOG_DAT[Lno].writeLogFlag[eLOG_TARGET_REMOTE] == 1) {		// ���O�����݂���

			strage = (Lno < eLOG_COINBOX) ? 1 : 0;	// FROM or SRAM
			// �j�A�t���`�F�b�N
			if( strage == 0){// RAM�݂̂ŊǗ����郍�O�̏ꍇ
				if(LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullMaximum != 0 ) {
					if(LOG_DAT[Lno].f_unread[eLOG_TARGET_REMOTE] >= LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullMaximum) {
						if(LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus != RAU_NEARFULL_NEARFULL){
							LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatusBefore = LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus;
							LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus = RAU_NEARFULL_NEARFULL;
						}
					}
				}
			}
			else{
				if(LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullMaximum != 0) {
					if((Ope_Log_TotalCountGet(Lno) - LOG_DAT[Lno].unread[eLOG_TARGET_REMOTE]) >= LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullMaximum ) {
						if(LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus != RAU_NEARFULL_NEARFULL){
							LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatusBefore = LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus;
							LOG_DAT[Lno].nearFull[eLOG_TARGET_REMOTE].NearFullStatus = RAU_NEARFULL_NEARFULL;
						}
					}
				}
			}

			if(nearfullCheck == 0 && RAUdata_CanSendTableData(Lno)) {	// �j�A�t���`�F�b�N�Ώۂ��H
				nearfullCheck = 1;										// �j�A�t���`�F�b�N�t���OON
			}
			
			if(LOG_DAT[Lno].writeFullFlag[eLOG_TARGET_REMOTE] == 1) {	// �o�b�t�@�t���`�F�b�N���K�v���H
				Log_CheckBufferFull(TRUE, Lno, eLOG_TARGET_REMOTE);
			}
			LOG_DAT[Lno].writeFullFlag[eLOG_TARGET_REMOTE] = 0;			// �f�[�^�t���t���OON
		}
		LOG_DAT[Lno].writeLogFlag[eLOG_TARGET_REMOTE] = 0;				// ���O�����݃t���O�N���A
	}
	
	// �j�A�t���`�F�b�N��v������
	if(nearfullCheck) {
		RAU_RequestCheckNearfull();
	}

}
/*[]----------------------------------------------------------------------[]*
 *|     RAU_GetCenterSeqNo
 *[]----------------------------------------------------------------------[]*
 *|	�Z���^�[�ǔԂ��擾����
 *[]----------------------------------------------------------------------[]*
 *|	param	: type	: �ǔԎ��
 *[]----------------------------------------------------------------------[]*
 *|	return	: ��ʂɑΉ�����Z���^�[�ǔ�
 *|			: 0		: �ǔԎ�ʕs��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ulong	RAU_GetCenterSeqNo(RAU_SEQNO_TYPE type)
{
	if(type < RAU_SEQNO_TYPE_COUNT) {			// �ǔԎ�ʂ��͈͓���
		return Rau_SeqNo[type];
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*
 *|     RAU_UpdateCenterSeqNo
 *[]----------------------------------------------------------------------[]*
 *|	�Z���^�[�ǔԂ��X�V����
 *[]----------------------------------------------------------------------[]*
 *|	param	: type	: �ǔԎ��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_UpdateCenterSeqNo(RAU_SEQNO_TYPE type)
{
	if(type < RAU_SEQNO_TYPE_COUNT) {			// �ǔԎ�ʂ��͈͓���
		++Rau_SeqNo[type];
		if(Rau_SeqNo[type] == 0) {
			Rau_SeqNo[type] = 1;				// �Z���^�[�ǔԂ�1�`0xffffffff�܂�
		}
	}
}

// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
//[]----------------------------------------------------------------------[]
//|			�f�[�^�đ��҂��^�C�}�[�L�����Z��
//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
void	RAU_Cancel_RetryTimer()
{
	RAU_Tm_data_rty.tm = 0;
	RAU_f_data_rty_ov = 1;
}
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��