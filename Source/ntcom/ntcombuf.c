// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/****************************************************************************/
/*																			*/
/*		�V�X�e�����@:�@NT-NET�ʐM���䕔										*/
/*		�t�@�C����	:  ntcombuf.c											*/
/*		�@�\		:  ��OPE����M����										*/
/*																			*/
/****************************************************************************/
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"


static eNTNET_RESULT	NTCom_SetSendDataNormal(const uchar* pData, ushort size);
static eNTNET_RESULT	NTCom_SetSendDataPrior(const uchar* pData, ushort size);

static	const unsigned char NTCom_pass_string[10]={'A','M','A','N','O',' ','N','T','C','\0'};



/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_SetSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̑��M�f�[�^�ݒ�
 *| param	: pData		NT-NET�f�[�^�i�`���f�[�^�̐擪�j
 *|			  size		�f�[�^�T�C�Y
 *|			  type		�f�[�^���(0:�ʏ� 1:�D��)
 *| return	: NTNET_NORMAL			����
 *|			  NTNET_ERR_BUFFERFULL	�o�b�t�@�t��
 *|			  NTNET_ERR_BLOCKOVER	NTNET�u���b�N�I�[�o�[�i�D��A����F1�u���b�N�A�ʏ�A26�u���b�N�ȏ�̃f�[�^�j
 *|			  NTNET_ERR_OTHER		���̑��̃G���[
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
eNTNET_RESULT	NTCom_SetSendData(const uchar* pData, ushort size, uchar type)
{
	eNTNET_RESULT	ret;

	if(NTComData_Ctrl.terminal_status != NT_ABNORMAL) {
		if(type == NTNET_NORMAL_DATA) {
			// �ʏ�f�[�^
			ret = NTCom_SetSendDataNormal(pData, size);
		}
		else if(type == NTNET_PRIOR_DATA) {
			// �D��f�[�^
			ret = NTCom_SetSendDataPrior(pData, size);
		}
		else {
			// ���̑��G���[
			ret =  NTNET_ERR_OTHER;
		}
	}
	else {
		// �ʐM�ُ풆�̓o�b�t�@�t���Ƃ��ĕԂ�
		ret = NTNET_ERR_BUFFERFULL;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̑��M�f�[�^�擾
 *| retrun	: NULL�ȊO	��M�f�[�^
 *|			  NULL		��M�f�[�^�Ȃ�
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort NTCom_GetSendData(uchar type, T_NT_BLKDATA* pRecvData)
{
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if(type == NTNET_NORMAL_DATA) {
		// �ʏ�f�[�^
		if(NTComOpeRecvNormalDataBuf.count == 0) {
			return 0;								// ��M�f�[�^�Ȃ�
		}

		// ��M�f�[�^�|�C���^�擾
		memset(pRecvData, 0, sizeof(T_NT_BLKDATA));
		u.uc[0] = NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][5];
		u.uc[1] = NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][6];
		pRecvData->len = u.us;
		memcpy(pRecvData->data, &NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][0], u.us);

		// �Ǎ��݃C���f�b�N�X�X�V
		++NTComOpeRecvNormalDataBuf.readIndex;
		if(NTComOpeRecvNormalDataBuf.readIndex == NT_NORMAL_BLKNUM_MAX) {
			NTComOpeRecvNormalDataBuf.readIndex = 0;
		}
		--NTComOpeRecvNormalDataBuf.count;				// �f�[�^�i�[�����Z
	}
	else if(type == NTNET_PRIOR_DATA) {
		// �D��f�[�^
		if(NTComOpeRecvPriorDataBuf.count == 0) {
			return 0;								// ��M�f�[�^�Ȃ�
		}

		// ��M�f�[�^�|�C���^�擾
		memset(pRecvData, 0, sizeof(T_NT_BLKDATA));
		u.uc[0] = NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][5];
		u.uc[1] = NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][6];
		pRecvData->len = u.us;
		memcpy(pRecvData->data, &NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][0], pRecvData->len);

		// �Ǎ��݃C���f�b�N�X�X�V
		++NTComOpeRecvPriorDataBuf.readIndex;
		if(NTComOpeRecvPriorDataBuf.readIndex == NT_PRIOR_BLKNUM_MAX) {
			NTComOpeRecvPriorDataBuf.readIndex = 0;
		}
		--NTComOpeRecvPriorDataBuf.count;				// �f�[�^�i�[�����Z
	}
	else {
		return 0;
	}
	return (ushort)pRecvData->len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_CanSetSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̑��M�f�[�^�ݒ�\�`�F�b�N
 *|			  type		�f�[�^���(0:�ʏ� 1:�D��)
 *| retrun	: TRUE		�󂫂���
 *|			  FALSE		�󂫂Ȃ�(�o�b�t�@FULL)
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	NTCom_CanSetSendData(uchar type)
{
	BOOL ret = FALSE;

	if(type == NTNET_NORMAL_DATA) {
		// �ʏ�f�[�^
		if(NTComOpeRecvNormalDataBuf.count < NT_NORMAL_BLKNUM_MAX) {
			ret = TRUE;
		}
	}
	else if(type == NTNET_PRIOR_DATA) {
		// �D��f�[�^
		if(NTComOpeRecvPriorDataBuf.count < NT_PRIOR_BLKNUM_MAX) {
			ret = TRUE;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_SetSendDataNormal
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̒ʏ푗�M�f�[�^�ݒ�
 *| param	: pData		NT-NET�f�[�^�i�`���f�[�^�̐擪�j
 *|			  size		�f�[�^�T�C�Y
 *| return	: NTNET_NORMAL			����
 *|			  NTNET_ERR_BUFFERFULL	�o�b�t�@�t��
 *|			  NTNET_ERR_BLOCKOVER	NTNET�u���b�N�I�[�o�[�i�D��A����F1�u���b�N�A�ʏ�A26�u���b�N�ȏ�̃f�[�^�j
 *|			  NTNET_ERR_OTHER		���̑��̃G���[
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static eNTNET_RESULT	NTCom_SetSendDataNormal(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// ���MNT-NET�f�[�^
	t_NtNetBlk*	pPacketData;						// �p�P�b�g�f�[�^
	uchar*	pNtData;
	ushort	remainDataSize;							// �c��f�[�^�T�C�Y
	ushort	packetDataSize;							// �p�P�b�g�f�[�^�T�C�Y
	ushort	size0Cut;								// 0�J�b�g�����f�[�^�T�C�Y
	ushort	headerSize;								// �p�P�b�g�w�b�_�T�C�Y
	ushort	writeBlock = 0;							// ����e���݃f�[�^��
	uchar	block = 0;								// �u���b�N
	eNTNET_RESULT	ret = NTNET_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	remainDataSize = size - 3;

	writeBlock = (size / NTNET_BLKDATA_MAX);
	if(size % NTNET_BLKDATA_MAX) {
		++writeBlock;
	}

	if(NTComOpeRecvNormalDataBuf.fixed) {
		// ���M���̓o�b�t�@�t���Ƃ���
		return NTNET_ERR_BUFFERFULL;
	}
	if(NTComOpeRecvNormalDataBuf.count >= 1) {
		// �o�b�t�@�t��
		return NTNET_ERR_BUFFERFULL;
	}

	pBasicData = (DATA_BASIC*)pData;

	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_NtNetBlkHeader) + 5;					// �w�b�_�T�C�Y

	while(remainDataSize)
	{
		memset(&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.writeIndex][0], 0, NT_BLKSIZE);
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.writeIndex][0];
		++block;												// �u���b�N�X�V(1�`)

		// 1�p�P�b�g�̃T�C�Y�����߂�
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//�f�[�^�𑗐M�o�b�t�@�Ɋi�[
		memcpy(pPacketData->data.data, pNtData, packetDataSize);
		remainDataSize -= packetDataSize;						// �c��f�[�^�T�C�Y�X�V

		// 0�J�b�g�T�C�Y�����߂�
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}

		// �p�P�b�g�w�b�_���̍쐬
		memcpy(pPacketData->header.sig, NT_PKT_SIG, 5);			// "NTCOM"

		u.us = headerSize + size0Cut;							// �f�[�^�T�C�Y(�w�b�_+0�J�b�g��f�[�^)
		pPacketData->header.len[0] = u.uc[0];
		pPacketData->header.len[1] = u.uc[1];

		pPacketData->header.vect = 0;							// �]�����@(���ʁ����)

		pPacketData->header.terminal_no = NTCom_InitData.TerminalNum;	// �]����[��No.

		pPacketData->header.lower_terminal_no1 = 0;				// ���ʓ]���p�[��No.1

		pPacketData->header.lower_terminal_no2 = 0;				// ���ʓ]���p�[��No.2

		pPacketData->header.link_no = 0;						// �p�P�b�g�V�[�P���XNo.

		pPacketData->header.status = 0;							// �[���X�e�[�^�X(0:����)

		pPacketData->header.broadcast_rcvd = 0;					// �����M�ς݃t���O

		u.us = packetDataSize;									// �f�[�^�T�C�Y(0�J�b�g�O�f�[�^)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];

		pPacketData->header.mode = 0;							// �p�P�b�g�D�惂�[�h(0:�ʏ�f�[�^)

		pPacketData->header.telegram_type = STX;				// STX

		pPacketData->header.res_detail = 0;						// �����ڍ�(0:����)

		// �f�[�^�w�b�_�[���̍쐬
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

		++NTComOpeRecvNormalDataBuf.writeIndex;						// �����݃C���f�b�N�X�X�V
		if(NTComOpeRecvNormalDataBuf.writeIndex == NT_NORMAL_BLKNUM_MAX) {
			NTComOpeRecvNormalDataBuf.writeIndex = 0;
		}
		++NTComOpeRecvNormalDataBuf.count;								// �f�[�^�i�[�����Z
		if(NTComOpeRecvNormalDataBuf.count > NT_NORMAL_BLKNUM_MAX) {		// �ŌẪf�[�^���㏑��������
			ret = NTNET_DEL_OLD;
			NTComOpeRecvNormalDataBuf.count = NT_NORMAL_BLKNUM_MAX;
		}
		if(pPacketData->data.is_lastblk == 0) {					// ���ԃf�[�^
			pNtData += packetDataSize;
		}
	}
	NTComOpeRecvNormalDataBuf.fixed = TRUE;

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_SetSendDataPrior
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̗D�摗�M�f�[�^�ݒ�
 *| param	: pData		NT-NET�f�[�^�i�`���f�[�^�̐擪�j
 *|			  size		�f�[�^�T�C�Y
 *| return	: NTNET_NORMAL			����
 *|			  NTNET_ERR_BUFFERFULL	�o�b�t�@�t��
 *|			  NTNET_ERR_BLOCKOVER	NTNET�u���b�N�I�[�o�[�i�D��A����F1�u���b�N�A�ʏ�A26�u���b�N�ȏ�̃f�[�^�j
 *|			  NTNET_ERR_OTHER		���̑��̃G���[
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static eNTNET_RESULT	NTCom_SetSendDataPrior(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// ���MNT-NET�f�[�^
	t_NtNetBlk*	pPacketData;						// �p�P�b�g�f�[�^
	uchar*	pNtData;
	ushort	remainDataSize;							// �c��f�[�^�T�C�Y
	ushort	packetDataSize;							// �p�P�b�g�f�[�^�T�C�Y
	ushort	size0Cut;								// 0�J�b�g�����f�[�^�T�C�Y
	ushort	headerSize;								// �p�P�b�g�w�b�_�T�C�Y
	ushort	writeBlock = 0;							// ����e���݃f�[�^��
	uchar	block = 0;								// �u���b�N
	eNTNET_RESULT	ret = NTNET_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	remainDataSize = size - 3;

	writeBlock = (size / NTNET_BLKDATA_MAX);
	if(size % NTNET_BLKDATA_MAX) {
		++writeBlock;
	}

	if(NTComOpeRecvPriorDataBuf.fixed) {
		// ���M���̓o�b�t�@�t��
		return NTNET_ERR_BUFFERFULL;
	}
	if(NTComOpeRecvPriorDataBuf.count >= 1) {
		// �o�b�t�@�t��
		return NTNET_ERR_BUFFERFULL;
	}

	pBasicData = (DATA_BASIC*)pData;

	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_NtNetBlkHeader) + 5;					// �w�b�_�T�C�Y

	while(remainDataSize)
	{
		memset(&NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.writeIndex][0], 0, NT_BLKSIZE);
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.writeIndex][0];
		++block;												// �u���b�N�X�V(1�`)

		// 1�p�P�b�g�̃T�C�Y�����߂�
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//�f�[�^�𑗐M�o�b�t�@�Ɋi�[
		memcpy(pPacketData->data.data, pNtData, packetDataSize);
		remainDataSize -= packetDataSize;						// �c��f�[�^�T�C�Y�X�V

		// 0�J�b�g�T�C�Y�����߂�
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}

		// �p�P�b�g�w�b�_���̍쐬
		memcpy(pPacketData->header.sig, NT_PKT_SIG, 5);			// "NTCOM"

		u.us = headerSize + size0Cut;							// �f�[�^�T�C�Y(�w�b�_+0�J�b�g��f�[�^)
		pPacketData->header.len[0] = u.uc[0];
		pPacketData->header.len[1] = u.uc[1];

		pPacketData->header.vect = 0;							// �]�����@(���ʁ����)

		pPacketData->header.terminal_no = NTCom_InitData.TerminalNum;	// �]����[��No.

		pPacketData->header.lower_terminal_no1 = 0;				// ���ʓ]���p�[��No.1

		pPacketData->header.lower_terminal_no2 = 0;				// ���ʓ]���p�[��No.2

		pPacketData->header.link_no = 0;						// �p�P�b�g�V�[�P���XNo.

		pPacketData->header.status = 0;							// �[���X�e�[�^�X(0:����)

		pPacketData->header.broadcast_rcvd = 0;					// �����M�ς݃t���O

		u.us = packetDataSize;									// �f�[�^�T�C�Y(0�J�b�g�O�f�[�^)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];

		pPacketData->header.mode = 1;							// �p�P�b�g�D�惂�[�h(1:�D��f�[�^)

		pPacketData->header.telegram_type = STX;				// STX

		pPacketData->header.res_detail = 0;						// �����ڍ�(0:����)

		// �f�[�^�w�b�_�[���̍쐬
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

		++NTComOpeRecvPriorDataBuf.writeIndex;						// �����݃C���f�b�N�X�X�V
		if(NTComOpeRecvPriorDataBuf.writeIndex == NT_PRIOR_BLKNUM_MAX) {
			NTComOpeRecvPriorDataBuf.writeIndex = 0;
		}
		++NTComOpeRecvPriorDataBuf.count;								// �f�[�^�i�[�����Z
		if(NTComOpeRecvPriorDataBuf.count > NT_PRIOR_BLKNUM_MAX) {		// �ŌẪf�[�^���㏑��������
			ret = NTNET_DEL_OLD;
			NTComOpeRecvPriorDataBuf.count = NT_PRIOR_BLKNUM_MAX;
		}
		if(pPacketData->data.is_lastblk == 0) {					// ���ԃf�[�^
			pNtData += packetDataSize;
		}
	}
	NTComOpeRecvPriorDataBuf.fixed = TRUE;

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendPriorDataID
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̗D�摗�M�f�[�^�o�b�t�@�ɂ���f�[�^��ʂ��擾����
 *| param	: none
 *| return	: �f�[�^ID	: �D�摗�M�o�b�t�@�Ɋi�[����Ă���f�[�^���
 *|			: 0			: �D�摗�M�o�b�t�@�Ƀf�[�^�Ȃ�
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
uchar	NTCom_GetSendPriorDataID(void)
{
	t_NtNetBlk*	pPacketData;						// �p�P�b�g�f�[�^

	if( (NTComOpeRecvPriorDataBuf.count == 0) &&	// �D��f�[�^�Ȃ�
		(NTComOpeRecvPriorDataBuf.fixed == FALSE) ){
		return 0;
	}

	if(NTComOpeRecvPriorDataBuf.count == 0) {
		pPacketData = (t_NtNetBlk*)&NTCom_SndBuf_Prior.blkdata->data[0];
	}
	else {
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvPriorDataBuf.ntDataBuf[0][0];
	}
	return pPacketData->data.data_type;				// �f�[�^��ʂ�Ԃ�
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendNormalDataID
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET��NTCom�̒ʏ푗�M�f�[�^�o�b�t�@�ɂ���f�[�^��ʂ��擾����
 *| param	: none
 *| return	: �f�[�^ID	: �ʏ푗�M�o�b�t�@�Ɋi�[����Ă���f�[�^���
 *|			: 0			: �ʏ푗�M�o�b�t�@�Ƀf�[�^�Ȃ�
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
uchar	NTCom_GetSendNormalDataID(void)
{
	t_NtNetBlk*	pPacketData;						// �p�P�b�g�f�[�^

	if( (NTComOpeRecvNormalDataBuf.count == 0) &&	// �ʏ�f�[�^�Ȃ�
		(NTComOpeRecvNormalDataBuf.fixed == FALSE) ) {
		return 0;
	}

	if(NTComOpeRecvNormalDataBuf.count == 0) {
		pPacketData = (t_NtNetBlk*)&NTCom_SndBuf_Normal.blkdata->data[0];
	}
	else {
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][0];
	}
	return pPacketData->data.data_type;				// �f�[�^��ʂ�Ԃ�
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_ClearData
 *[]----------------------------------------------------------------------[]
 *| summary	: NTCom�̑���M�f�[�^���N���A����
 *| param	: mode	0:RAM�N���A 1:�����e�i���X�N���A
 *| return	: none
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void	NTCom_ClearData(uchar mode)
{
	unsigned char	initFlag = 0;

	if(4 == read_rotsw()) {
		initFlag = 1;
	}
	else if( memcmp( NTCom_memory_pass, NTCom_pass_string, 10 ) != 0 ){
		// �v���O�����ؑ֌�A����̂ݏ��������s��
		initFlag = 1;
	}
	else if(mode) {
		// �����e�i���X����̃N���A
		initFlag = 1;
	}

	if(mode) {
		NTComSci_Stop();								// �V���A������M��~
	}

	NTCom_Init();

	// ��OPE����M�o�b�t�@�N���A
	if(initFlag) {
		memcpy( NTCom_memory_pass, NTCom_pass_string, 10 );
		memset(&NTComOpeRecvNormalDataBuf, 0, sizeof(NTComOpeRecvNormalDataBuf));	// NTCom�^�X�N�ʏ��M�o�b�t�@�i��OPE�w�j
		memset(&NTComOpeRecvPriorDataBuf, 0, sizeof(NTComOpeRecvPriorDataBuf));		// NTCom�^�X�N�D���M�o�b�t�@�i��OPE�w�j
		memset(&NTComOpeSendNormalDataBuf, 0, sizeof(NTComOpeSendNormalDataBuf));	// NTCom�^�X�N�ʏ푗�M�o�b�t�@�i��OPE�w�j
		memset(&NTComOpeSendPriorDataBuf, 0, sizeof(NTComOpeSendPriorDataBuf));		// NTCom�^�X�N�D�摗�M�o�b�t�@�i��OPE�w�j

		NTComData_Clear();
		NTComComm_Clear();
	}

	if(mode) {
		NTCom_FuncStart();
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
