/****************************************************************************/
/*																			*/
/*		�V�X�e�����@:�@RAU�V�X�e��											*/
/*		�t�@�C����	:  raubuf.c												*/
/*		�@�\		:  �΃��C������M����									*/
/*																			*/
/****************************************************************************/

#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"raudef.h"

RauNtDataBuf	RauOpeSendNtDataBuf;					// RAU�^�X�N���M�o�b�t�@�i��OPE�w�j
RauNtDataBuf	RauOpeRecvNtDataBuf;					// RAU�^�X�N��M�o�b�t�@�i��OPE�w�j
static	BOOL	RAU_IsTransactionData(uchar dataType);

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: RAU�̎�M�o�b�t�@��NT-NET�f�[�^���i�[����
 *| param	: pData		NT-NET�f�[�^�i�`���f�[�^�̐擪�j
 *|			  size		�f�[�^�T�C�Y
 *| retrun	: NTNET_BUFSET_NORMAL	����
 *|			  NTNET_BUFSET_DEL_OLD	�ŌẪf�[�^���㏑������
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	RAU_SetSendNtData(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// ���MNT-NET�f�[�^
	t_RemoteNtNetBlk*	pPacketData;				// �p�P�b�g�f�[�^
	uchar*	pNtData;
	ushort	remainDataSize;							// �c��f�[�^�T�C�Y
	ushort	packetDataSize;							// �p�P�b�g�f�[�^�T�C�Y
	ushort	size0Cut;								// 0�J�b�g�����f�[�^�T�C�Y
	ushort	headerSize;								// �p�P�b�g�w�b�_�T�C�Y
	ushort	writeBlock = 0;							// ����e���݃f�[�^��
	uchar	block = 0;								// �u���b�N
	uchar	ret = NTNET_BUFSET_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	uchar 	*dt;

	remainDataSize = size - 3;

	dt = (uchar *)pData;
	if (dt[0] != REMOTE_SYSTEM) {				//	SystemID != 30
		if(TRUE == RAU_IsTransactionData(dt[1])) {	// �g�����U�N�V�����f�[�^�̓��O�ɕۑ������͂�
			return ret;
		}
	}
	
	writeBlock = (size / NTNET_BLKDATA_MAX);
	if(size % NTNET_BLKDATA_MAX) {
		++writeBlock;
	}
	
	if(RauOpeRecvNtDataBuf.count >= RAU_NTDATABUF_COUNT) {
		// �o�b�t�@�t���̂��ߌÂ��f�[�^���㏑������
		RauOpeRecvNtDataBuf.readIndex += writeBlock;
		if(RauOpeRecvNtDataBuf.readIndex >= RAU_NTDATABUF_COUNT) {
			RauOpeRecvNtDataBuf.readIndex -= RAU_NTDATABUF_COUNT;
		}
	}

	pBasicData = (DATA_BASIC*)pData;
	
	// ���u����d���̏ꍇ�A�f�[�^�ێ��t���O�̍ŏ�ʃr�b�g������/�蓮������
	if(pBasicData->DataKeep  & 0x80) {
		z_NtRemoteAutoManual = REMOTE_MANUAL;
	}
	else {
		z_NtRemoteAutoManual = REMOTE_AUTO;
	}
	pBasicData->DataKeep &= 0x7f;

	pNtData = (uchar*)pData + 3;

	headerSize = sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX;	// �w�b�_�T�C�Y
	
	while(remainDataSize)
	{
		memset(&RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.writeIndex][0], 0, RAU_BLKSIZE);
		pPacketData = (t_RemoteNtNetBlk*)&RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.writeIndex][0];
		++block;												// �u���b�N�X�V(1�`)
		
		// 1�p�P�b�g�̃T�C�Y�����߂�
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//�f�[�^�𑗐M�o�b�t�@�Ɋi�[
		memcpy(&pPacketData->data.data, pNtData, packetDataSize);

		remainDataSize -= packetDataSize;						// �c��f�[�^�T�C�Y�X�V
		
		// 0�J�b�g�T�C�Y�����߂�
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}
		
		// �p�P�b�g�w�b�_�̍쐬
		u.us = headerSize + size0Cut;							// �f�[�^�T�C�Y(�w�b�_+0�J�b�g��f�[�^)
		pPacketData->header.packet_size[0] = u.uc[0];
		pPacketData->header.packet_size[1] = u.uc[1];
		u.us = packetDataSize;									// �f�[�^�T�C�Y(0�J�b�g�O�f�[�^)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];
		pPacketData->data.blk_no = block;						// ID1(�u���b�N�ԍ�)
		if(remainDataSize) {
			pPacketData->data.is_lastblk = 0;					// ID2(����)
		}
		else {
			pPacketData->data.is_lastblk = 1;					// ID2(�ŏI)
		}
		pPacketData->data.system_id = pBasicData->SystemID;		// ID3(�V�X�e��ID)
		pPacketData->data.data_type = pBasicData->DataKind;		// ID4(�f�[�^ID)
		pPacketData->data.save = pBasicData->DataKeep;			// �f�[�^�ێ��t���O
		
		++RauOpeRecvNtDataBuf.writeIndex;						// �����݃C���f�b�N�X�X�V
		if(RauOpeRecvNtDataBuf.writeIndex == RAU_NTDATABUF_COUNT) {
			RauOpeRecvNtDataBuf.writeIndex = 0;
		}
		++RauOpeRecvNtDataBuf.count;								// �f�[�^�i�[�����Z
		if(RauOpeRecvNtDataBuf.count > RAU_NTDATABUF_COUNT) {		// �ŌẪf�[�^���㏑��������
			ret = NTNET_BUFSET_DEL_OLD;
			RauOpeRecvNtDataBuf.count = RAU_NTDATABUF_COUNT;
		}
		if(pPacketData->data.is_lastblk == 0) {					// ���ԃf�[�^
			pNtData += packetDataSize;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�o�b�t�@����NT-NET�f�[�^���擾����
 *| retrun	: NULL�ȊO	��M�f�[�^
 *|			  NULL		��M�f�[�^�Ȃ�
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar*	RAU_GetSendNtData(void)
{
	uchar*	pRecvData;								// ��M�f�[�^
	
	if(RauOpeRecvNtDataBuf.count == 0) {
		return FALSE;								// ��M�f�[�^�Ȃ�
	}
	
	// ��M�f�[�^�|�C���^�擾
	pRecvData = &RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.readIndex][0];

	// �Ǎ��݃C���f�b�N�X�X�V
	++RauOpeRecvNtDataBuf.readIndex;
	if(RauOpeRecvNtDataBuf.readIndex == RAU_NTDATABUF_COUNT) {
		RauOpeRecvNtDataBuf.readIndex = 0;
	}
	--RauOpeRecvNtDataBuf.count;						// �f�[�^�i�[�����Z
	
	return pRecvData;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_IsTransactionData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O�ɕۑ�����f�[�^���`�F�b�N����
 *| retrun	: TRUE		���O�ɕۑ�����f�[�^
 *|			  NULL		���O�ɕۑ����Ȃ��f�[�^�i�X���[�f�[�^�j
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAU_IsTransactionData(uchar dataType)
{
	BOOL	ret = FALSE;
	
	switch(dataType) {
	case 20:	// ���Ƀf�[�^
	case 22:	// 
	case 23:	// ���Z�f�[�^
	case 54:	// ���Ƀf�[�^
	case 56:	// ���Z�f�[�^�i���O�j
	case 57:	// ���Z�f�[�^�i�o���j
	case 58:	// ���ԑ䐔�f�[�^
	case 120:	// �G���[�f�[�^
	case 121:	// �A���[���f�[�^
	case 122:	// ���j�^�f�[�^
	case 123:	// ���샂�j�^�f�[�^
	case 131:	// �R�C�����ɏW�v�f�[�^
	case 133:	// �������ɏW�v�f�[�^
	case 135:	// �ޑK�Ǘ��W�v�f�[�^
	case 236:	// ���ԑ䐔�f�[�^
	case 126:	// ���K�Ǘ��f�[�^
	case 125:	// ���u�Ď��f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	case 61:	// �������ԏ��f�[�^
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
		ret = TRUE;
		break;
	default:	// �W�v�f�[�^
		if ((dataType >= 30 && dataType <= 38) || dataType == 41) {
			ret = TRUE;
		}
		if ((dataType == 42) || (dataType == 43) || (dataType == 45) || (dataType == 46) || (dataType == 53)) {
			ret = TRUE;
		}
		break;
	}
	return ret;
}
