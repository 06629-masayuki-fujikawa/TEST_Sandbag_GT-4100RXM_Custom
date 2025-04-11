/****************************************************************************/
/*                                                                          */
/*      �V�X�e�����@:�@RAU�V�X�e��                                          */
/*                                                                          */
/*      �t�@�C����  :  IDPROC.C                                             */
/*      �@�\            :  �h�c���ڍ׏���                                   */
/*                                                                          */
/*  Version     :                                                           */
/*                                                                          */
/*  ���ŗ���                                                                */
/****************************************************************************/

#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"rauIDproc.h"
#include	"rau.h"
#include	"ksgRauModemData.h"

const uchar RAU_TableDataTbl[]={1,4,100,103,109,142,243,154,156};	// �g�n�r�s�����M���鉓�u�V�X�e���d��(�V�X�e���h�c���P�O�`�P�X�̂���)�łh�a�v����̉����d���̂������
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή�
const uchar RAU_TableDataTbl_PIP[]={1,3,5,7,10};					// �g�n�r�s�����M���鉓�u�V�X�e���d��(�V�X�e���h�c���P�U�̂���)�łh�a�v����̉����d���̂������
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή�
const uchar RAU_RemoteDataTbl[]={80};	// �g�n�r�s�����M���鉓�u�ŗL�d��(�V�X�e���h�c���R�O�̂���)�łh�a�v����̉����d���̂������

uchar	RAU_ibw_receive_data[RAU_IBW_TEMP_BUFF_SIZE];	// �����uPC�ւ̑��M(IBW����̎�M)�̂�32block�Ɋg�� 2006.08.31:m-onouchi
uchar	RAU_host_receive_data[RAU_HOST_TEMP_BUFF_SIZE];

extern		RAU_TIME_AREA	RAU_Tm_Port_Watchdog;// �������ʐM�Ď��^�C�}(�������̂s�b�o�ڑ������ؒf����)
extern	int		KSG_gPpp_RauStarted;				// PPP�ڑ��t���O

extern	void	RAU_data_table_init(void);
extern	uchar	RAU_ID98new_SaveToTmpBuf( uchar *pData, uchar Kind );
extern	void	RAU_7segErrSet( uchar Kind, uchar f_OnOff );
extern	eRAU_TABLEDATA_SET	RAUdata_SetTableData(uchar *pData, ushort wDataLen, uchar ucDataId);
extern	eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
extern	BOOL	RAUdata_CheckNearFull( void );
extern	void	RAUdata_CreatRauConfig1(RAU_CONFIG1 *pConfig1);
extern	void	RAUdata_CreatRauConfig2(RAU_CONFIG2 *pConfig2);
extern	void	RAUdata_TableClear(uchar ucTableKind);
extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void	RAUhost_SetSendRequeat(RAU_SEND_REQUEST *send_request);
extern	void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
extern	void	RAUhost_SetError(uchar ucErrCode);

void	RAUid_Init(void);
eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
eRAU_TABLEDATA_SET	RAUid_IbwDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_HostDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_IbwThroughDataAnalyze(uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_HostThroughDataAnalyze(uchar ucDataId, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_IbkControl(RAU_CONTROL_REQUEST *pCtrlReq);
BOOL	RAU_isTableData(uchar ucDataId);
void	RAUid_SendRequest(RAU_SEND_REQUEST *pSendReq);
BOOL	RAUid_TableDataClear(RAU_CLEAR_REQUEST *pClearReq);
BOOL	RAUid_TableDataInitialize( void );
eRAU_TABLEDATA_SET RAUid_CommuTest(RAU_COMMUNICATION_TEST *pNTdata);
BOOL	RAUid_CheckRcvDopaData(uchar *pData, ushort uiDataLen);
eRAU_TABLEDATA_SET	RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
BOOL	RAUid_RcvSeqCnt_Start(uchar ucSystemId, uchar ucDataId);
void	RAUid_Unpack0Cut(uchar *ucData);

extern	void	RAUdata_CreateNearFullData(RAU_NEAR_FULL *pNearFullData);
extern	void RAUdata_CreatTableDataCount(RAU_TABLE_DATA_COUNT *pDataCount);

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_Init()
 *[]----------------------------------------------------------------------[]*
 *|	������
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-07-20
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_Init(void) {
	RAU_ucLastNtBlockNum = 0;
	RAU_uiArcReceiveLen = 0;

	RAU_uiLastTcpBlockNum = 0;
	RAU_uiDopaReceiveLen = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_SetNtBlockData()
 *[]----------------------------------------------------------------------[]*
 *|	param	pBlockData	��M�f�[�^�̂m�s�u���b�N
 *|			size		��M�f�[�^�̂m�s�u���b�N�T�C�Y
 *[]----------------------------------------------------------------------[]*
 *|	return	eRAU_TABLEDATA_SET
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size)
{
	eRAU_TABLEDATA_SET	ret;
	eRAU_TABLEDATA_SET	temp;
	uchar ucBlockNum;
	uchar ucBlockSts;
	union {
		uchar	uc[2];
		ushort	us;
	}u;
	ulong		wkul;

	ucBlockNum = pBlockData->data.blk_no;
	ucBlockSts = pBlockData->data.is_lastblk;

	if (RAU_ucLastNtBlockNum == 0) {
		// ���݃u���b�N�O�i�d���m�蒼��j
		if (ucBlockNum == 1) {
			// ����P
			memcpy(&RAU_ibw_receive_data[2], pBlockData, size);
			RAU_uiArcReceiveLen = size;
			RAU_ucLastNtBlockNum = 1;
			ret = RAU_DATA_NORMAL;
		} else {
			// ����1�ȊO�@�@**�u���b�N�i���o�[�ُ퍡�񖳌�**
			RAUhost_SetError(RAU_ERR_BLK_INVALID);	// �G���[�R�[�h�Q�O(�m�s�|�m�d�s�u���b�N�ԍ��ُ�[���񖳌�])
			RAU_uiArcReceiveLen = 0;
			RAU_ucLastNtBlockNum = 0;
			ret = RAU_DATA_BLOCK_INVALID1;
		}
	} else {
		// ���݃u���b�N0�ȊO
		if (ucBlockNum == 1) {
			// �u���b�N�i���o�[�P�������݁@**�u���b�N�i���o�[�ُ퍡��L��**
			RAUhost_SetError(RAU_ERR_BLK_VALID);	// �G���[�R�[�h�P�X(�m�s�|�m�d�s�u���b�N�ԍ��ُ�[����L��])
			memcpy(&RAU_ibw_receive_data[2], pBlockData, size);
			RAU_uiArcReceiveLen = size;
			RAU_ucLastNtBlockNum = 1;
			ret = RAU_DATA_BLOCK_INVALID2;
		} else if ((RAU_ucLastNtBlockNum + 1) == ucBlockNum) {
			// �u���b�N�i���o�[��r�@�O��{�P
			memcpy(&RAU_ibw_receive_data[RAU_uiArcReceiveLen + 2], pBlockData, size);
			RAU_uiArcReceiveLen += size;
			RAU_ucLastNtBlockNum++;
			ret = RAU_DATA_NORMAL;
		} else {
			// ����ȊO�@�@**�u���b�N�i���o�[�ُ퍡�񖳌�**
			RAUhost_SetError(RAU_ERR_BLK_INVALID);	// �G���[�R�[�h�Q�O(�m�s�|�m�d�s�u���b�N�ԍ��ُ�[���񖳌�])
			RAU_uiArcReceiveLen = 0;
			RAU_ucLastNtBlockNum = 0;
			ret = RAU_DATA_BLOCK_INVALID1;
		}
	}

	if ((ret == RAU_DATA_NORMAL) || (ret == RAU_DATA_BLOCK_INVALID2)) {
		// �ŏI�u���b�N
		if (ucBlockSts == 1) {
			// �h�c���ʏ������Ńe�[�u���o�b�t�@�ɃR�s�[

			u.us = RAU_uiArcReceiveLen + 2;
			RAU_ibw_receive_data[0] = u.uc[0];
			RAU_ibw_receive_data[1] = u.uc[1];
			/* ARCNET��M�f�[�^�C�V�X�e��ID(ID3)��10�`19,�d���f�[�^���(ID4)��229 �� */
			/* Main������M�������v�f�[�^�d���Ɣ��f���� */
			if(	(10 <= RAU_ibw_receive_data[9]) &&
				(RAU_ibw_receive_data[9] <= 19) &&	// �V�X�e���h�c�i�h�c�R�j�F���u�V�X�e���n��
				(RAU_ibw_receive_data[10] == 229) )	// �f�[�^�h�c�i�h�c�S�j  �F���v�f�[�^
			{
				/* �����܂łŎ��v�f�[�^�d����MArcnet�t���[���Ɗm��                                                 */
				/* NT-NET���v�f�[�^�d���͏]���̌`��(33byte)��e-timing�����ɑΉ������`��(41byte)��2�ʂ葶�݂���B    */
				/* �`���̈Ⴂ��0�J�b�g�O�̃f�[�^�T�C�Y�Ŕ��ʂ�e-timing�����ɑΉ������`���̏ꍇ�ɂ͓��ꏈ�����s���B  */
				u.uc[0] = RAU_ibw_receive_data[4];
				u.uc[1] = RAU_ibw_receive_data[5];					// 0�J�b�g�O�̃f�[�^�T�C�Yget
				if( u.us >= 41 ){									// �V���v�f�[�^�d���i�␳�l�t���j�ł����
					RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);		// 0�J�b�g���ꂽ�f�[�^�𕜌�����
					u.us += NT_DOPANT_HEAD_SIZE;					// 0�J�b�g�O�f�[�^�T�C�Y+�����[�g�w�b�_(10byte)=�p�P�b�g�f�[�^�T�C�Y
					RAU_ibw_receive_data[2] = u.uc[0];				// �p�P�b�g�f�[�^�T�C�Y��L��
					RAU_ibw_receive_data[3] = u.uc[1];				// (����4byte�Ƀ��C�t�^�C�}�[�l���Z�b�g���邽�߃t���T�C�Y�ɂȂ�)
					RAU_uiArcReceiveLen = u.us;							// NT-NET�d���̃f�[�^�����ēx�Z�b�g(���v�f�[�^��1�u���b�N�ł��邽�ߐ��藧��)
					u.us += 2;										// NT-NET�d���̃f�[�^��+�f�[�^���i�[�G���A(2byte)���ēx�Z�b�g
					RAU_ibw_receive_data[0] = u.uc[0];				// 
					RAU_ibw_receive_data[1] = u.uc[1];				// 
					wkul = RAU_c_1mTim_Start();
					memcpy( &RAU_ibw_receive_data[49], &wkul, 4 );	// �d���Ɍ��݂̃��C�t�^�C�}�[�l���Z�b�g
				}
			}
			RAU_CrcCcitt(RAU_uiArcReceiveLen, 0, &RAU_ibw_receive_data[2], (uchar*)NULL, &RAU_ibw_receive_data[RAU_uiArcReceiveLen + 2], 1 );

			RAU_ucLastNtBlockNum = 0;

			temp = RAUid_IbwDataAnalyze(RAU_ibw_receive_data[9], RAU_ibw_receive_data[10], RAU_uiArcReceiveLen + 4);
			if (temp != RAU_DATA_NORMAL) {
				ret = temp;
			}
			RAU_uiArcReceiveLen = 0;
		} else {
			// �ŏI�u���b�N�ł͂Ȃ��A�u���b�N�i���o�[�R�Q�ɓ��B�@**�u���b�N�i���o�[�ُ퍡�񖳌�**	
			if (ucBlockNum >= RAU_SEND_NTBLKNUM_MAX && ret == RAU_DATA_NORMAL) {	// ��IBW����̎�M(���uPC�ւ̑��M)�̂�32block�Ɋg�� 2006.08.31:m-onouchi
				// �����܂Ő��픻��łR�Q�u���b�N�ȏゾ������A�G���[
				RAUhost_SetError(RAU_ERR_BLK_OVER_ARC);	// �G���[�R�[�h�T�O(�m�s�|�m�d�s��M�u���b�N�I�[�o�[)
				RAU_uiArcReceiveLen = 0;
				RAU_ucLastNtBlockNum = 0;
				ret = RAU_DATA_BLOCK_INVALID1;
			}
		}
	} 
	return ret;
}

//==============================================================================
//	�z�X�g�����M�����m�s�|�m�d�s�d���̕ۑ�
//
//	@argument	*pData		�z�X�g�����M�����f�[�^(�c���o���w�b�_���������f�[�^���{�b�q�b)
//	@argument	uiDataLen	�z�X�g�����M�����f�[�^�̒���(�c���o���w�b�_�Ƃb�q�b���������f�[�^��)
//	@argument	uiTcpBlkNo	�s�b�o�u���b�N�m���D
//	@argument	ucTcpBlkSts	�s�b�o�u���b�N�X�e�[�^�X
//
//	@return		RAU_DATA_NORMAL					�f�[�^�̕ۑ������܂��͑��M�v���f�[�^(�h�c�W�P)�𐳏��M
//	@return		RAU_DATA_BLOCK_INVALID1			�c���o���p�P�b�g�܂��͂m�s�|�m�d�s�d���̍\���G���[
//	@return		RAU_DATA_CONNECTING_FROM_HOST	�e�[�u���f�[�^���M���Ƀz�X�g���瑗�M�v�����󂯂����ߎ󂯕t���s��
//	@return		RAU_DATA_BUFFERFULL				�o�b�t�@����t
//
//	@attention	�V�X�e���h�c�R�O�̃f�[�^�͎�M�X���[�f�[�^�e�[�u���ɂ͓�����܂���B
//
//	@note		�z�X�g���P�c���o���p�P�b�g��M���閈�ɃR�[������
//				�ŏI�p�P�b�g����M�����Ƃ��A��M�V�[�P���X���ύX�����Ƌ���
//				�擾�����m�s�|�m�d�s�d������M�X���[�f�[�^�e�[�u���ɕۑ����܂��B
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
eRAU_TABLEDATA_SET RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts)
{
	eRAU_TABLEDATA_SET	ret;
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	ulong		wkul;

	ret = RAU_DATA_BLOCK_INVALID1;
	if( RAU_uiLastTcpBlockNum == 0 ){	// �Ō�Ɏ�M�����s�b�o�u���b�N�̔ԍ�(�O�͏����������u���b�N�X�e�[�^�X�P�̃f�[�^�����������Ƃ�)
		if( uiTcpBlkNo == 1 ){		// �ŏ��̂s�b�o�u���b�N(�s�b�o�u���b�N�ԍ����P)
			memcpy(&RAU_host_receive_data[2], pData, uiDataLen);	// �z�X�g�����M�����p�P�b�g��(����ȓd���ł��K������)�o�b�t�@�̐擪�Ɋi�[
			RAU_uiLastTcpBlockNum = 1;								// �Ō�Ɏ�M�����s�b�o�u���b�N�̔ԍ�
			RAU_uiDopaReceiveLen = uiDataLen;						// ��M�����f�[�^�̃T�C�Y(�ŏ��Ȃ̂ł��̂܂ܑ��)
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 			RAU_Tm_RcvSeqCnt = 0;									// �^�C�}�l�N���A
// 			RAU_f_RcvSeqCnt_rq = 1;									// ��M�V�[�P���X�L�����Z���^�C�}���싖��(�N��)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_WAIT);		// �g�n�r�s���牽����M�����̂Ŏ�M�V�[�P���X���P�ɕύX
			RAUdata_TableClear(0);								// �X���[�f�[�^�̍폜
			ret = RAU_DATA_NORMAL;
		} else {					// �ŏ��Ȃ̂ɂs�b�o�u���b�N�ԍ����P����Ȃ��̂͂m�f
			RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);		// �G���[�R�[�h�W�P(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
			RAU_uiLastTcpBlockNum = 0;
			RAU_uiDopaReceiveLen = 0;
		}
	} else {						// �r���̃p�P�b�g����M��
		if( uiTcpBlkNo == 1 ){		// �r���p�P�b�g��M���ɂs�b�o�u���b�N�ԍ��P�̃u���b�N����M
			RAUhost_SetError(ERR_RAU_DPA_BLK_VALID_D);			// �G���[�R�[�h�W�O(��������M�f�[�^�u���b�N�ԍ��ُ�[����L��])
			memcpy(&RAU_host_receive_data[2], pData, uiDataLen);	// �z�X�g�����M�����p�P�b�g��(����ȓd���ł��K������)�o�b�t�@�̐擪�Ɋi�[
			RAU_uiLastTcpBlockNum = 1;								// �Ō�Ɏ�M�����s�b�o�u���b�N�̔ԍ�
			RAU_uiDopaReceiveLen = uiDataLen;						// ��M�����f�[�^�̃T�C�Y(�ŏ��Ȃ̂ł��̂܂ܑ��)
			ret = RAU_DATA_NORMAL;
		} else if( (RAU_uiLastTcpBlockNum + 1) == uiTcpBlkNo ){	// �s�b�o�u���b�N�ԍ����A�ԂɂȂ��Ă��邱�Ƃ��m�F�B
			if( RAU_uiDopaReceiveLen + uiDataLen <= RAU_HOST_TEMP_BUFF_SIZE ){	// ��M�����p�P�b�g��ۑ����Ă��o�b�t�@�͈��Ȃ��B
				memcpy(&RAU_host_receive_data[2 + RAU_uiDopaReceiveLen], pData, uiDataLen);	// �o�b�t�@�̑��������Ɏ�M�����p�P�b�g���i�[
				RAU_uiLastTcpBlockNum = uiTcpBlkNo;			// �Ō�Ɏ�M�����s�b�o�u���b�N�̔ԍ�
				RAU_uiDopaReceiveLen += uiDataLen;			// ���܂Ŏ�M�����f�[�^�̃T�C�Y(���v)
				ret = RAU_DATA_NORMAL;
			} else {				// ��M�����p�P�b�g��ۑ�����ƃo�b�t�@������B
// MH322914 (s) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
				RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);	// �G���[�R�[�h�W�P(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X���O�ɖ߂�
// MH322914 (e) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
				RAU_uiLastTcpBlockNum = 0;
				RAU_uiDopaReceiveLen = 0;
			}
// MH322914 (s) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
		}
		else if (uiTcpBlkNo <= RAU_uiLastTcpBlockNum) {
		// ��M�ς݂̃u���b�N�ɂ��Ă`�b�j�ŉ�������
			ret = RAU_DATA_NORMAL;
// MH322914 (e) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
		} else {	// �s�b�o�u���b�N�ԍ����A�ԂɂȂ��Ă��Ȃ�
			RAUhost_SetError(ERR_RAU_DPA_BLK_INVALID_D);		// �G���[�R�[�h�W�P(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
// MH322914 (s) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);		// ��M�V�[�P���X���O�ɖ߂�
// MH322914 (e) kasiyama 2016/07/12 ��M�m�f�ɂ��e�[�u�����M����~������̉��P[���ʃo�ONo.1256](MH341106)
			RAU_uiLastTcpBlockNum = 0;
			RAU_uiDopaReceiveLen = 0;
		}
	}

	if( ucTcpBlkSts == 1 && ret == RAU_DATA_NORMAL ){	// �ŏI�s�b�o�u���b�N���G���[�Ȃ�����Ƀo�b�t�@�Ɋi�[�ł����B
		if( RAUid_CheckRcvDopaData(&RAU_host_receive_data[2], RAU_uiDopaReceiveLen) == TRUE ){	// �m�s�f�[�^�̍\���`�F�b�N(�n�j)
			/* ���uNT-NET��M�f�[�^�C�V�X�e��ID(ID3)��10�`19,�d���f�[�^���(ID4)��119 �� */
			/* �O�������M�������v�f�[�^�d���Ɣ��f����                                  */
			if(	(10 <= RAU_host_receive_data[9]) &&
				(RAU_host_receive_data[9] <= 19) &&		// �V�X�e���h�c�i�h�c�R�j�F���u�V�X�e���n��
				(RAU_host_receive_data[10] == 119) )	// �f�[�^�h�c�i�h�c�S�j  �F���v�f�[�^
			{
				/* �����܂łŎ��v�f�[�^�d����MArcnet�t���[���Ɗm��                                                 */
				/* NT-NET���v�f�[�^�d���͏]���̌`��(33byte)��e-timing�����ɑΉ������`��(41byte)��2�ʂ葶�݂���B    */
				/* �`���̈Ⴂ��0�J�b�g�O�̃f�[�^�T�C�Y�Ŕ��ʂ�e-timing�����ɑΉ������`���̏ꍇ�ɂ͓��ꏈ�����s���B  */
				u.uc[0] = RAU_host_receive_data[4];
				u.uc[1] = RAU_host_receive_data[5];					// 0�J�b�g�O�̃f�[�^�T�C�Yget
				if( u.us >= 41 ){								// �V���v�f�[�^�d���i�␳�l�t���j�ł����
					RAUid_Unpack0Cut(&RAU_host_receive_data[2]);	// 0�J�b�g���ꂽ�f�[�^�𕜌�����
					u.us += NT_DOPANT_HEAD_SIZE;				// 0�J�b�g�O�f�[�^�T�C�Y+�����[�g�w�b�_(10byte)=�p�P�b�g�f�[�^�T�C�Y
					RAU_host_receive_data[2] = u.uc[0];				// �p�P�b�g�f�[�^�T�C�Y��L��
					RAU_host_receive_data[3] = u.uc[1];				// (����4byte�Ƀ��C�t�^�C�}�[�l���Z�b�g���邽�߃t���T�C�Y�ɂȂ�)
					RAU_uiDopaReceiveLen = u.us;					// NT-NET�d���̃f�[�^�����ēx�Z�b�g(���v�f�[�^��1�u���b�N�ł��邽�ߐ��藧��)
					wkul = RAU_c_1mTim_Start();
					memcpy( &RAU_host_receive_data[49], &wkul, 4 );	// �d���Ɍ��݂̃��C�t�^�C�}�[�l���Z�b�g
				}
			}
			u.us = RAU_uiDopaReceiveLen + 2;		// ����M�T�C�Y���o�b�t�@�̐擪�Q�o�C�g�����ɕۑ�
			RAU_host_receive_data[0] = u.uc[0];
			RAU_host_receive_data[1] = u.uc[1];
			RAU_CrcCcitt(RAU_uiDopaReceiveLen, 0, &RAU_host_receive_data[2], (uchar*)NULL, &RAU_host_receive_data[RAU_uiDopaReceiveLen + 2], 1 );	// �b�q�b�Z�o
			ret = RAUid_HostDataAnalyze(RAU_host_receive_data[9], RAU_host_receive_data[10], RAU_uiDopaReceiveLen + 4);	// ���M�v���f�[�^(�h�c�W�P)�ȊO�͎�M�X���[�f�[�^�e�[�u���ɕۑ�
		} else {
			ret = RAU_DATA_BLOCK_INVALID1;		// �m�s�f�[�^�̍\���ɖ�肠��B
		}
		RAU_uiLastTcpBlockNum = 0;
		RAU_uiDopaReceiveLen = 0;
		//1ID��M�����ɂĉ������̒ʐM�Ď��^�C�}�[�̊Ď����~����
		RAU_Tm_Port_Watchdog.tm = 0;			// �������ʐM�Ď��^�C�}(��~)
	}

	return ret;
}
/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	IBK�ŏ����ΏۂɂȂ��Ă���e�[�u���f�[�^�����f����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAU_isTableData(uchar ucDataId) {

	BOOL ret = FALSE;

	switch (ucDataId) {
	case 0x14:		// ���Ƀf�[�^
	case 0x15:		// �o�Ƀf�[�^
	case 0x16:		// ���Z�f�[�^�i���O�j
	case 0x17:		// ���Z�f�[�^�i�o���j
	case 0x36:		// ���Ƀf�[�^
	case 0x37:		// �o�Ƀf�[�^
	case 0x38:		// ���Z�f�[�^�i���O�j
	case 0x39:		// ���Z�f�[�^�i�o���j
	case 0x1E:		// T���v�W�v�f�[�^
	case 0x1F:		// T���v�W�v�f�[�^
	case 0x20:		// T���v�W�v�f�[�^
	case 0x21:		// T���v�W�v�f�[�^
	case 0x22:		// T���v�W�v�f�[�^
	case 0x23:		// T���v�W�v�f�[�^
	case 0x24:		// T���v�W�v�f�[�^
	case 0x25:		// T���v�W�v�f�[�^
	case 0x26:		// T���v�W�v�f�[�^
	case 0x29:		// T���v�W�v�f�[�^
	case 0x78:		// �G���[�f�[�^�e�[�u��
	case 0x79:		// �A���[���f�[�^�e�[�u��
	case 0x7A:		// ���j�^�f�[�^�e�[�u��
	case 0x7B:		// ���샂�j�^�f�[�^�e�[�u��
	case 0x83:		// �R�C�����ɏW�v���v�f�[�^�e�[�u��
	case 0x85:		// �������ɏW�v���v�f�[�^�e�[�u��
	case 0xEC:		// ���ԑ䐔�f�[�^�e�[�u��
	case 0x3A:		// Web�p���ԑ䐔�f�[�^�e�[�u��
	case 0xED:		// ���䐔�E���ԃf�[�^�e�[�u��
	case 0x7E:		// ���K�Ǘ��f�[�^
		ret = TRUE;
		break;
	default:
		ret = FALSE;
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_IbwDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	�h�a�v�����M�����m�s�f�[�^�𕪐͂���
 *[]----------------------------------------------------------------------[]*
 *|	patam		�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbwDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret = RAU_DATA_NORMAL;		// #002(S/E) 2007.12.13 m-onouchi

	if( RemoteCheck_Buffer116_117.CheckNo != 0 ){	// �ʐM�`�F�b�N�v���f�[�^���g�n�r�s����󂯕t���Ă���B
		RemoteCheck_Buffer116_117.Re.sult.Code = 7;	// ���ʃR�[�h��(���f�[�^��M)�Ƃ���B
	}

	switch( ucSystemId ){	// �V�X�e���h�c�i�h�c�R�j
	case 0x1E:				// �R�O(���u�ŗL�d��)
		ret = RAUid_IbwThroughDataAnalyze(ucDataId, uiDataLen);
		break;
	case 40:
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M�������)�̂Ƃ�
			ret = RAUdata_SetThroughData(RAU_ibw_receive_data, uiDataLen, 0);	// ���M�X���[�f�[�^�o�b�t�@(�h�a�j���g�n�r�s)�ɓo�^
		} else {	// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M������ԂƂ��ȊO�͂h�a�v����̃X���[�f�[�^���󂯕t���Ȃ�(�đ���v��)
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
		break;
	default:
		if(10 <= ucSystemId && ucSystemId <= 19){	// ���u�V�X�e���n��
			ret = RAUdata_SetTableData(RAU_ibw_receive_data, uiDataLen, ucDataId);	// �e�[�u���o�b�t�@�ɓo�^
			if (RAUdata_CheckNearFull() == TRUE) {									// �j�A�t����ԃ`�F�b�N
				RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);		// ��ԕω������̂Œʒm
				RAU_Word2Byte(RAU_temp_data, 32);
				RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
				RAUdata_SetThroughData(RAU_temp_data, 34, 1);
			}
			break;
		}
		break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_HostDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	�g�n�r�s�����M�����m�s�f�[�^�𕪐͂���
 *[]----------------------------------------------------------------------[]*
 *|	patam		�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_HostDataAnalyze(uchar ucSystemId, uchar ucDataId, ushort uiDataLen) {

	eRAU_TABLEDATA_SET	ret;

	switch( ucSystemId ){	// �V�X�e���h�c�i�h�c�R�j
	case 0x1E:				// �R�O(���u�ŗL�d��)
		ret = RAUid_HostThroughDataAnalyze(ucDataId, uiDataLen);
		break;
	default:
		ret = RAUdata_SetThroughData(RAU_host_receive_data, uiDataLen, 1);	// �X���[�f�[�^�Ƃ��ēo�^
		if( ret == RAU_DATA_NORMAL ){									// �o�^����
			if( RAUid_RcvSeqCnt_Start(ucSystemId, ucDataId) ){			// ��M�V�[�P���X�L�����Z���^�C�}�̋N��(�h�a�v����̉����d���̂���f�[�^�̂�)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_RCV);		// �g�n�r�s����̗v���f�[�^��o�^�����̂Ŏ�M�V�[�P���X���Q�ɕύX
			} else {													// �h�a�v����̉����d���̂Ȃ��f�[�^
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);			// �g�n�r�s����̗v���f�[�^��o�^�����̂Ŏ�M�V�[�P���X���O�ɕύX
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;									// ��M�V�[�P���X�L�����Z���^�C�}���싖��(����)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			}
		}
		break;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_ThroughDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	�h�a�j���ŏ�������X���[�f�[�^�𕪐͂���
 *[]----------------------------------------------------------------------[]*
 *|	patam		�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbwThroughDataAnalyze(uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret;

	// �h�a�v�����M�����V�X�e���h�c�R�O(���u�ŗL�d��)
	if( ucDataId == 0x3C ){			// ���u�h�a�j����f�[�^(�h�a�v���h�a�j)
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ||		// ��M�V�[�P���X�O�܂���
			RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){	// ��M�V�[�P���X�Q�Ƃ����u�h�a�j����f�[�^���󂯕t����B
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			ret = RAUid_IbkControl((RAU_CONTROL_REQUEST*)(&RAU_ibw_receive_data[2]));	// �v���t���O�ɉ���������(����, �ݒ�P, �ݒ�Q)
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if( ucDataId == 0x3D ){	// ���M�v���f�[�^(�h�a�v���h�a�j)
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL &&	// ���M�V�[�P���X�O����
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){	// ��M�V�[�P���X�O�̂Ƃ��̂݃N���A�v�����󂯕t����B
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_SendRequest((RAU_SEND_REQUEST*)&RAU_ibw_receive_data[2]);				// �z�X�g���ɑ��M�v���t���O��ݒ�
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x3E) {	// �e�[�u���N���A�v���f�[�^(�h�a�v���h�a�j)
		if( ( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL ||	// ���M�V�[�P���X�O�܂��͏��TCP�ؒf���A����
			  RauCtrl.tcpConnectFlag.port.upload != 1 )&&
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){		// ��M�V�[�P���X�O�̂Ƃ��̂݃N���A�v�����󂯕t����B
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_TableDataClear((RAU_CLEAR_REQUEST*)(&RAU_ibw_receive_data[2]));	// �e�[�u���f�[�^�N���A
			ret = RAU_DATA_NORMAL;
			if (RAUdata_CheckNearFull() == TRUE) {								// �j�A�t����ԃ`�F�b�N
				RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);		// ��ԕω����Ă�����ʒm
				RAU_Word2Byte(RAU_temp_data, 32);
				RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
				RAUdata_SetThroughData(RAU_temp_data, 34, 1);	// ��M�X���[�f�[�^�o�b�t�@(�h�a�j���h�a�v)�ɓo�^
			}
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x42) {	// ���u�h�a�j�C�j�V�����C�Y�f�[�^(�h�a�v���h�a�j)
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL &&	// ���M�V�[�P���X�O����
			RAUhost_GetRcvSeqFlag()==RAU_RCV_SEQ_FLG_NORMAL ){	// ��M�V�[�P���X�O�̂Ƃ��̂݃N���A�v�����󂯕t����B
			RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
			RAUid_TableDataInitialize();									// �C�j�V�����C�Y
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// ��M�V�[�P���X���O(�ʏ���)�ɂ���B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	} else if (ucDataId == 0x64){	// �h�c�P�O�O�F�ʐM�`�F�b�N�v���f�[�^(�h�a�v���h�a�j)
		RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);		// �d�����O�J�b�g����Ă���ꍇ�A�����łb�q�b���j�󂳂��B
		RAU_Word2Byte(&RAU_ibw_receive_data[0], 62);	// �m�s�|�m�d�s�d���T�C�Y�{�Q(�t���T�C�Y)
		RAU_Word2Byte(&RAU_ibw_receive_data[2], 60);	// ���ʕ��{�ŗL����(�O�J�b�g��)�f�[�^��
		RAU_CrcCcitt(60, 0, &RAU_ibw_receive_data[2], (uchar*)NULL, &RAU_ibw_receive_data[62], 1 );	// �b�q�b�ĎZ�o
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_ibw_receive_data);
	} else if (ucDataId == 0x75){	// �h�c�P�P�V�F�ʐM�`�F�b�N���ʃf�[�^(�h�a�v���h�a�j)
		RAUid_Unpack0Cut(&RAU_ibw_receive_data[2]);
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_ibw_receive_data);
	} else {						// ����ȊO
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M�������)�̂Ƃ�
			ret = RAUdata_SetThroughData(RAU_ibw_receive_data, uiDataLen, 0);	// ���M�X���[�f�[�^�o�b�t�@(�h�a�j���g�n�r�s)�ɓo�^
		} else {	// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M������ԂƂ��ȊO�͂h�a�v����̃X���[�f�[�^���󂯕t���Ȃ�(�đ���v��)
			ret = RAU_DATA_CONNECTING_FROM_HOST;
		}
	}

	return ret;
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUid_ThroughDataAnalyze()
 *[]----------------------------------------------------------------------[]*
 *|	�h�a�j���ŏ�������X���[�f�[�^�𕪐͂���
 *[]----------------------------------------------------------------------[]*
 *|	patam		�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET RAUid_HostThroughDataAnalyze(uchar ucDataId, ushort uiDataLen)
{
	eRAU_TABLEDATA_SET	ret;
	RAU_SEND_REQUEST	*pSendReq;

	// �g�n�r�s�����M�����V�X�e���h�c�R�O(���u�ŗL�d��)
	if( ucDataId == 0x51 ){	// ���M�v���f�[�^
		if( RAUhost_GetSndSeqFlag()==RAU_SND_SEQ_FLG_NORMAL ){	// ���M�V�[�P���X�O�̂Ƃ�����
			RAUid_Unpack0Cut(&RAU_host_receive_data[2]);
			pSendReq = (RAU_SEND_REQUEST*)&RAU_host_receive_data[2];
			pSendReq->turi_manage = 0;							// �z�X�g����̒ޑK�Ǘ����M�v���͍s��Ȃ�
			RAUid_SendRequest((RAU_SEND_REQUEST*)&RAU_host_receive_data[2]);	// �z�X�g���ɑ��M�v���t���O��ݒ�
			ret = RAU_DATA_NORMAL;
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// �g�n�r�s����̑��M�v���ɑ΂��Ăh�a�j���Ǝ��ɑΉ�����̂ŃV�[�P���X���O�ɕύX
		} else {	// ���M�V�[�P���X�P(�g�n�r�s�ւ̃e�[�u�����M�J�n���)�̂Ƃ��͂m�`�j�X�O���g�n�r�s�ɕԂ��B
			ret = RAU_DATA_CONNECTING_FROM_HOST;
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// ��M�V�[�P���X���O�ɕύX
		}
	} else if (ucDataId == 0x65){	// �h�c�P�O�P�F�ʐM�`�F�b�N���ʃf�[�^(�g�n�r�s���h�a�j)
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_host_receive_data);
	} else if (ucDataId == 0x74){	// �h�c�P�P�U�F�ʐM�`�F�b�N�v���f�[�^(�g�n�r�s���h�a�j)
		RAUid_Unpack0Cut(&RAU_host_receive_data[2]);	// �d�����O�J�b�g����Ă���ꍇ�A�����łb�q�b���j�󂳂��B
		RAU_Word2Byte(&RAU_host_receive_data[0], 62);	// �m�s�|�m�d�s�d���T�C�Y�{�Q(�t���T�C�Y)
		RAU_Word2Byte(&RAU_host_receive_data[2], 60);	// ���ʕ��{�ŗL����(�O�J�b�g��)�f�[�^��
		RAU_CrcCcitt(60, 0, &RAU_host_receive_data[2], (uchar*)NULL, &RAU_host_receive_data[62], 1 );	// �b�q�b�ĎZ�o
		ret = RAUid_CommuTest((RAU_COMMUNICATION_TEST*)RAU_host_receive_data);
	} else {				// ����ȊO
		ret = RAUdata_SetThroughData(RAU_host_receive_data, uiDataLen, 1);	// �X���[�f�[�^�o�b�t�@�ɓo�^
		if( ret == RAU_DATA_NORMAL ){									// �o�^����
			if( RAUid_RcvSeqCnt_Start(0x1E, ucDataId) ){				// ��M�V�[�P���X�L�����Z���^�C�}�̋N��(�h�a�v����̉����d���̂���f�[�^)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_RCV);		// �g�n�r�s����̗v���f�[�^��o�^�����̂Ŏ�M�V�[�P���X���Q�ɕύX
			} else {													// �h�a�v����̉����d���̂Ȃ��f�[�^
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);			// �g�n�r�s����̗v���f�[�^��o�^�����̂Ŏ�M�V�[�P���X���O�ɕύX
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;									// ��M�V�[�P���X�L�����Z���^�C�}���싖��(����)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			}
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUid_IbkControl(RAU_CONTROL_REQUEST *pCtrlReq) {

	// �e��v���t���O���`�F�b�N�i�d�l�㓯���ɕ����̃t���O���ݒ肳��邱�Ƃ͂Ȃ��̂ŁA�ォ�珇���`�F�b�N���顁j
	eRAU_TABLEDATA_SET ret;

	if(pCtrlReq->table_data_count == 1){										// �e�[�u�������f�[�^�v��
		RAUdata_CreatTableDataCount((RAU_TABLE_DATA_COUNT*)&RAU_temp_data[2]);		// �V�X�e���h�c�R�O, �f�[�^�h�c�W�R
		RAU_Word2Byte(RAU_temp_data, 50);											// �擪�Q�o�C�g�ɂP�m�s�|�m�d�s�d��(���u���b�N�ł͂Ȃ�)�̒����{�Q���Z�b�g(�T�O�o�C�g)
		RAU_CrcCcitt(48, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[50], 1 );	// �I�[�Q�o�C�g�ɂb�q�b���Z�o���ăZ�b�g(�擪�Q�o�C�g�����̒����ɂ͊܂܂�Ă��Ȃ�)
		ret = RAUdata_SetThroughData(RAU_temp_data, 52, 1);							// ��M�X���[�f�[�^�o�b�t�@(�h�a�j���h�a�v)�ɓo�^
	} else {																	// ���m�̗v���t���O���m�F�ł��Ȃ�����
		ret = RAU_DATA_BUFFERFULL;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_SendRequest(RAU_SEND_REQUEST *pSendReq) {

	RAUhost_SetSendRequeat(pSendReq);
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAUid_TableDataClear(RAU_CLEAR_REQUEST *pClearReq) {

//	data_table_init();

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	// ���Ƀf�[�^			�iID20�j
//	if (pClearReq->in_parking == 1) {
//		RAUdata_TableClear(20);
//		Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_REMOTE);			// ����
//	}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	// �o�Ƀf�[�^			�iID21�j
//	if (pClearReq->out_parking == 1) {
//		RAUdata_TableClear(21);
//	}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// ���Z�f�[�^			�iID22 23�j
	if (pClearReq->paid_data == 1) {
		RAUdata_TableClear(22);
		RAUdata_TableClear(23);
		Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_REMOTE);			// ���Z
	}
	// T���v�W�v�f�[�^		�iID30�`38�E41�j
	if (pClearReq->total == 1) {
		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAUdata_TableClear(53);
			Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_REMOTE);		// T�W�v
			Ope_Log_UnreadToRead(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);		// GT�W�v
		}
		else {
			RAUdata_TableClear(41);
			Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_REMOTE);			// T�W�v
		}
	}
	// �G���[�f�[�^			�iID120�j
	if (pClearReq->error == 1) {
		RAUdata_TableClear(120);
		Ope_Log_UnreadToRead(eLOG_ERROR, eLOG_TARGET_REMOTE);			// �װ
	}
	// �A���[���f�[�^		�iID121�j
	if (pClearReq->alarm == 1) {
		RAUdata_TableClear(121);
		Ope_Log_UnreadToRead(eLOG_ALARM, eLOG_TARGET_REMOTE);			// �װ�
	}
	// ���j�^�[�f�[�^		�iID122�j
	if (pClearReq->monitor == 1) {
		RAUdata_TableClear(122);
		Ope_Log_UnreadToRead(eLOG_MONITOR, eLOG_TARGET_REMOTE);			// ���
	}
	// ���샂�j�^�[�f�[�^	�iID123�j
	if (pClearReq->ope_monitor == 1) {
		RAUdata_TableClear(123);
		Ope_Log_UnreadToRead(eLOG_OPERATE, eLOG_TARGET_REMOTE);			// �������
	}
	// �R�C�����ɏW�v�f�[�^	�iID131�j
	if (pClearReq->coin_total == 1) {
		RAUdata_TableClear(131);
		Ope_Log_UnreadToRead(eLOG_COINBOX, eLOG_TARGET_REMOTE);			// ��݋��ɏW�v	(ram�̂�)
	}
	// �������ɏW�v�f�[�^	�iID133�j
	if (pClearReq->money_toral == 1) {
		RAUdata_TableClear(133);
		Ope_Log_UnreadToRead(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);			// �������ɏW�v	(ram�̂�)
	}
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	// ���ԑ䐔�E���ԃf�[�^	�iID236�j
//	if (pClearReq->parking_num == 1) {
//		RAUdata_TableClear(236);
//		Ope_Log_UnreadToRead(eLOG_PARKING, eLOG_TARGET_REMOTE);			// ���ԑ䐔�ް�
//	}
//	// ���䐔�E���ԃf�[�^	�iID237�j
//	if (pClearReq->area_num == 1) {
//		RAUdata_TableClear(237);
//	}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// ���K�Ǘ��f�[�^		�iID126�j
	if (pClearReq->money_manage == 1) {
		RAUdata_TableClear(126);
		Ope_Log_UnreadToRead(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);		// ���K�Ǘ�		(ram�̂�)
	}
	// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	if (pClearReq->turi_manage == 1) {
		RAUdata_TableClear(135);
		Ope_Log_UnreadToRead(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);		// �ޑK�Ǘ�		(ram�̂�)
	}
	// ���u�Ď��f�[�^		�iID125�j
	if (pClearReq->rmon == 1) {
		RAUdata_TableClear(125);
		Ope_Log_UnreadToRead(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);	// ���u�Ď��f�[�^
	}
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	// �������ԏ�ԃf�[�^		�iID61�j
//	if (pClearReq->long_park == 1) {
//		RAUdata_TableClear(61);
//		Ope_Log_UnreadToRead(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);	// �������ԏ�ԃf�[�^
//	}
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL RAUid_TableDataInitialize( void )
{
	int		i;
	short	NearFullStatus[RAU_TABLE_MAX];

	for( i=0; i<RAU_TABLE_MAX; i++ ){
		NearFullStatus[i] = rau_data_table_info[i].i_NearFullStatus;		// �N���A����O�̃o�b�t�@��Ԃ��擾
	}

	RAU_data_table_init();	// �S�e�[�u��������(i_NearFullStatus, i_NearFullStatusBefore ���ɉ������)

	for( i=0; i<RAU_TABLE_MAX; i++ ){
		rau_data_table_info[i].i_NearFullStatusBefore = NearFullStatus[i];	// �O��̃o�b�t�@��ԂƂ���B
	}

	return TRUE;
}

//==============================================================================
//	�ʐM�`�F�b�N�f�[�^�̎�M����
//
//	@argument	*pNTdata	�ʐM�`�F�b�N�v���f�[�^ or ���ʃf�[�^
//
//	@return		RAU_DATA_NORMAL					����
//	@return		RAU_DATA_BLOCK_INVALID1			�u���b�N�i���o�[�ُ�ɂ��f�[�^�j��(���񖳌�)
//	@return		RAU_DATA_BLOCK_INVALID2			�u���b�N�i���o�[�ُ�ɂ��f�[�^�j��(����L��)
//	@return		RAU_DATA_CONNECTING_FROM_HOST	�z�X�g����̐ڑ����̂��ߎ󂯕t���s��(�f�[�^���M�v��)
//	@return		RAU_DATA_BUFFERFULL				�o�b�t�@����t
//
//	@note		�g�n�r�s�Ƃh�a�v���瑗�M����Ă���ʐM�`�F�b�N�v���f�[�^�܂���
//				���ʃf�[�^����͂��K�؂ȃo�b�t�@�ɓo�^����B
//
//	@author		2007.02.13:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
eRAU_TABLEDATA_SET RAUid_CommuTest(RAU_COMMUNICATION_TEST *pNTdata)
{
	eRAU_TABLEDATA_SET		ret;
	RAU_COMMUNICATION_TEST	*pBuff;
	uchar					Direction = 0;
	uchar					ResultCode;

	if(KSG_gPpp_RauStarted) {							// PPP�m����
		switch( pNTdata->id4 ){							// ��M�����d��
		case 100:										// �h�a�v�����M�����ʐM�`�F�b�N�v���f�[�^
			pBuff = &RemoteCheck_Buffer100_101;
			if( pNTdata->Re.quest.Kind == 0 ){			// �v�����(�`�F�b�N�J�n)
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){
					if( RAUhost_GetSndSeqFlag() == RAU_SND_SEQ_FLG_NORMAL ){	// �㉺������g�p
						if( pNTdata->Re.quest.TimeOut != 0 ){
							RAU_Tm_CommuTest.bits0.bit_0_13 = ((int)~pNTdata->Re.quest.TimeOut)+1;
							RAU_f_CommuTest_rq = 1;		// �ʐM�`�F�b�N�p�^�C�}�N��
							RAU_f_CommuTest_ov = 0;
						}
						*pBuff = *pNTdata;
						ResultCode = 0;
						RauCT_SndReqID = pNTdata->id4;	// �ʐM�`�F�b�N�f�[�^(�h�c�P�O�O)���M�v���Z�b�g
						Direction = 0;					// ���M����(�g�n�r�s)
						ret = RAU_DATA_NORMAL;			// �h�a�v�ɂ͂`�q�b�u���b�N"��M����"����(0x00)��Ԃ��B
					} else {							// ������̂ݎg�p��
						ResultCode = 2;					// ���ʃR�[�h(����g�p��)
						Direction = 1;					// ���M����(�h�a�v)
					}
				} else {								// �������g�p��
					ResultCode = 7;						// ���ʃR�[�h(����g�p��)
					Direction = 1;						// ���M����(�h�a�v)
				}
			} else {									// �v�����(�`�F�b�N���~)
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// ���������g�p
					if( pNTdata->CheckNo == pBuff->CheckNo ){				// �`�F�b�N����v
						RAU_Tm_CommuTest.tm = 0;		// �ʐM�`�F�b�N�^�C�}�^�C�}(��~)
						if( RauCtrl.TCPcnct_req == 1 ){	// ������s�b�o�R�l�N�V�����v���t���O(�I�����)
							ResultCode = 4;				// ���ʃR�[�h(�ڑ���)
						} else {						// ������̂s�b�o�R�l�N�V�����͈�x�m���ς�
							ResultCode = 6;				// ���ʃR�[�h(�����҂�)
						}
					} else {
						ResultCode = 1;					// ���ʃR�[�h(���`�F�b�N)
					}
				} else {								// �������g�p��
					ret = RAU_DATA_CONNECTING_FROM_HOST;// �h�a�v�ɂ͂`�q�b�u���b�N"��M����"�f�[�^�j��(0x90)��Ԃ��B
					ResultCode = 0;
				}
				Direction = 1;							// ���M����(�h�a�v)
			}
			break;
		case 116:										// �g�n�r�s�����M�����ʐM�`�F�b�N�v���f�[�^
			pBuff = &RemoteCheck_Buffer116_117;
			if( pNTdata->Re.quest.Kind == 0 ){			// �v�����(�`�F�b�N�J�n)
				if( pNTdata->Re.quest.TimeOut != 0 ){
					RAU_Tm_CommuTest.bits0.bit_0_13 = ((int)~pNTdata->Re.quest.TimeOut)+1;
					RAU_f_CommuTest_rq = 1;				// �ʐM�`�F�b�N�p�^�C�}�N��
					RAU_f_CommuTest_ov = 0;
				}
				*pBuff = *pNTdata;
				Direction = 1;							// ���M����(�h�a�v)
				RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// ��M�X���[�f�[�^�o�b�t�@(�h�a�j���h�a�v)�ɓo�^
				ResultCode = 0;
				ret = RAU_DATA_NORMAL;					// �g�n�r�s�ɂ͂`�b�j��Ԃ��B
			} else {									// �v�����(�`�F�b�N���~)
				if( pNTdata->CheckNo == pBuff->CheckNo ){	// �`�F�b�N����v
					RAU_Tm_CommuTest.tm = 0;			// �ʐM�`�F�b�N�^�C�}�^�C�}(��~)
					memset(pBuff, 0, 64);				// �o�b�t�@�N���A
					RauCT_SndReqID = 0;					// �ʐM�`�F�b�N�f�[�^(�h�c�P�P�U)���M�v���N���A
					ResultCode = 6;						// ���ʃR�[�h(�����҂�)
				} else {
					ResultCode = 1;						// ���ʃR�[�h(���`�F�b�N)
				}
				Direction = 0;							// ���M����(�g�n�r�s)
			}
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// �g�n�r�s����̗v���f�[�^�����������̂Ŏ�M�V�[�P���X���O�ɕύX
			break;
		default:
			if( pNTdata->id4 == 101 ){					// �g�n�r�s�����M�����ʐM�`�F�b�N���ʃf�[�^
				pBuff = &RemoteCheck_Buffer100_101;
				Direction = 1;							// ���M����(�h�a�v)
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// �g�n�r�s����̌��ʃf�[�^����������̂Ŏ�M�V�[�P���X���O�ɕύX
			} else {
				pBuff = &RemoteCheck_Buffer116_117;
				Direction = 0;							// ���M����(�g�n�r�s)
			}
			if( pNTdata->CheckNo == pBuff->CheckNo ){	// �`�F�b�N����v
				RAU_Tm_CommuTest.tm = 0;				// �ʐM�`�F�b�N�^�C�}�^�C�}(��~)
				if( Direction == 1 ){					// ���M����(�h�a�v)
					ret = RAUdata_SetThroughData((uchar *)pNTdata, 64, Direction);	// ��M�X���[�f�[�^�o�b�t�@(�h�a�j���h�a�v)�ɓo�^
					if( ret == RAU_DATA_NORMAL ){
						memset(pBuff, 0, 64);
						RauCT_SndReqID = 0;				// �ʐM�`�F�b�N�f�[�^���M�v���N���A
					}
				} else {								// ���M����(�g�n�r�s)
					*pBuff = *pNTdata;
					RauCT_SndReqID = pNTdata->id4;		// �ʐM�`�F�b�N�f�[�^(�h�c�P�P�V)���M�v���Z�b�g
					ret = RAU_DATA_NORMAL;				// �h�a�v�ɂ͂`�q�b�u���b�N"��M����"����(0x00)��Ԃ��B
				}
			} else {
				ret = RAU_DATA_NORMAL;					// �ǂݎ̂Ăă`�F�b�N������v����f�[�^��҂B
			}
			ResultCode = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	} else {	// �c���o���ʐM�[���܂��͂w�o���������I�����C����ԂɂȂ��Ă��Ȃ�
		if( pNTdata->id4 == 100 ){
			pBuff = &RemoteCheck_Buffer100_101;
		} else {
			pBuff = &RemoteCheck_Buffer116_117;
		}
		Direction = 1;									// ���M����(�h�a�v)
		if(RauConfig.modem_exist == 0) {				// ���f������
			if( KSG_mdm_status < 2 ){					// DTE�m���O
				ResultCode = 11;						// 11:FOMA���W���[���ڑ��s��
			}
			else if( KSG_mdm_status < 3 ){				// ���ڑ�
				ResultCode = 12;						// 12:�_�C�A�����M���s
			}
			else if(KSG_gPpp_RauStarted == 0) {
				ResultCode = 13;						// 13:PPP���ڑ�
			}
		}
		else {											// LAN�ݒ�
			// LAN�ݒ�̏ꍇ�����͒ʂ�Ȃ��͂�
			ResultCode = 3;								// ���ʃR�[�h(�ڑ��s�\)
		}

	}

	if( ResultCode != 0 ){	// "�ʐM�e�X�g�n�j"�ȊO�͒ʐM�`�F�b�N����[�m�f]�f�[�^���o��
		*pBuff = *pNTdata;
		pBuff->Re.quest.Kind = 0;
		pBuff->Re.quest.TimeOut = 0;
		pBuff->NTpacket_len[1] = 62;					// �擪�Q�o�C�g��(�擪�Q�o�C�g���܂�)�b�q�b�L�������������d����
		pBuff->data_len[1] = 60;						// �f�[�^�����O�J�b�g���������{�w�b�_���̂P�O�o�C�g
		pBuff->data_len_zero_cut[1] = 50;				// �O�J�b�g����O�̃f�[�^���̒���
		pBuff->id4 = pNTdata->id4 + 1;					// �ʐM�`�F�b�N����[�m�f]�f�[�^
		pBuff->Re.sult.Code = ResultCode;				// ���ʃR�[�h
		RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// �����Q�o�C�g�ɂb�q�b���Z�o���ċL��
		if( Direction == 0 ){							// ���M����(�g�n�r�s)
			RauCT_SndReqID = pBuff->id4;				// �ʐM�`�F�b�N����[�m�f]�f�[�^(�P�P�V)���M�v���Z�b�g
		} else {										// ���M����(�h�a�v)
			ret = RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// �X���[�f�[�^�o�b�t�@�ɓo�^
			memset(pBuff, 0, 64);						// ����Ń`�F�b�N�����O�ɂȂ�ʐM�`�F�b�N�f�[�^���M�v��(RauCT_SndReqID)���O�łȂ��̂Œ��f�ƂȂ�B
		}
	}

	return ret;
}

//==============================================================================
//	�ʐM�`�F�b�N�̃G���[����
//
//	@argument	void
//
//	@return		void
//
//	@attention	�ʐM�`�F�b�N�^�C�}�̃I�[�o�t���[�t���O���֐��R�[���̏����ł��B
//
//	@note		�ʐM�`�F�b�N���̃^�C���A�E�g�܂��͒ʐM��Q����������ƃR�[������
//				�h�a�v�܂��͂g�n�r�s�֒ʐM�`�F�b�N����[�m�f]�f�[�^�𑗐M����B
//
//	@author		2007.02.13:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
void RAUid_CommuTest_Error(void)
{
	RAU_COMMUNICATION_TEST	*pBuff;
	uchar					Direction;

	if( RemoteCheck_Buffer100_101.CheckNo != 0 ){			// �ʐM�`�F�b�N�v���f�[�^���h�a�v����󂯕t���Ă���B
		pBuff = &RemoteCheck_Buffer100_101;
		Direction = 1;										// ���M����(�h�a�v)
	} else if( RemoteCheck_Buffer116_117.CheckNo != 0 ){	// �ʐM�`�F�b�N�v���f�[�^���g�n�r�s����󂯕t���Ă���B
		pBuff = &RemoteCheck_Buffer116_117;
		Direction = 0;										// ���M����(�g�n�r�s)
	} else {
		return;
	}
	pBuff->Re.quest.Kind = 0;
	pBuff->Re.quest.TimeOut = 0;
	pBuff->NTpacket_len[1] = 62;					// �擪�Q�o�C�g��(�擪�Q�o�C�g���܂�)�b�q�b�L�������������d����
	pBuff->data_len[1] = 60;						// �f�[�^�����O�J�b�g���������{�w�b�_���̂P�O�o�C�g
	pBuff->data_len_zero_cut[1] = 50;				// �O�J�b�g����O�̃f�[�^���̒���
	pBuff->id4 = pBuff->id4 + 1;					// �ʐM�`�F�b�N����[�m�f]�f�[�^
	if( pBuff->Re.sult.Code == 0){
		if( (Direction == 1)&&(RauCtrl.TCPcnct_req == 1) ){
			pBuff->Re.sult.Code = 4;				// ���ʃR�[�h��(�ڑ���)�Ƃ���B
		} else {
			pBuff->Re.sult.Code = 6;				// ���ʃR�[�h(�����҂�)
		}
	}
	RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// �����Q�o�C�g�ɂb�q�b���Z�o���ċL��
	if( Direction == 0 ){							// ���M����(�g�n�r�s)
		if( RAUhost_GetSndSeqFlag() == RAU_SND_SEQ_FLG_NORMAL ){	// ��������g�p
			RauCT_SndReqID = pBuff->id4;			// �ʐM�`�F�b�N����[�m�f]�f�[�^(�h�c�P�P�V)���M�v���Z�b�g
		} else {
			memset(pBuff, 0, 64);
			RauCT_SndReqID = 0;						// �ʐM�`�F�b�N�f�[�^���M�v���N���A
		}
	} else {										// ���M����(�h�a�v)
		if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// ���������g�p
			RAUdata_SetThroughData((uchar *)pBuff, 64, Direction);	// �X���[�f�[�^�o�b�t�@�ɓo�^
		}
		memset(pBuff, 0, 64);	// ����Ń`�F�b�N�����O�ɂȂ�ʐM�`�F�b�N�f�[�^���M�v��(RauCT_SndReqID)���O�łȂ��̂Œ��f�����ƂȂ�B
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_CheckRcvDopaData()
 *[]----------------------------------------------------------------------[]*
 *|	��M����Dopa�p�P�b�g��NT�f�[�^�Ƃ��Ă̐��������`�F�b�N����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	TRUE	����
 *|	return	FALSE	�ُ�
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUid_CheckRcvDopaData(uchar *pData, ushort uiDataLen) {

	ushort uiPos;
	uchar	i;
	uchar	ucLastBlkNo;
	BOOL ret;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	uiPos = 0;
	ucLastBlkNo = 0;
	ret = FALSE;

	for (i = 0; i < RAU_RECEIVE_NTBLKNUM_MAX; i++) {	// 26�u���b�N���`�F�b�N���čŏI�u���b�N��������Ȃ���΁A�ُ�B
		u.uc[0] =  pData[uiPos];
		u.uc[1] =  pData[uiPos + 1];

		if (10 <= u.us && u.us <= 970) {				// �����`�F�b�N
			// 1�u���b�N������̒�����10�ȏ�970�ȉ��ł���ΐ���

			if (ucLastBlkNo + 1 == pData[uiPos + 5]) {	// �u���b�NNo.�`�F�b�N
				// �O��̃u���b�NNO.�{�P�ƈ�v���Ȃ���΁A�ُ�
				// �u���b�NNo.���C���N�������g
				ucLastBlkNo++;
			} else {
				break;
			}

			if (pData[uiPos + 6] == 0x01) {				// �ŏI�u���b�N�`�F�b�N
				// 26�u���b�N�ȓ��ɍŏI�u���b�N��������ΐ���
				ret = TRUE;
				break;
			}

			// ���u���b�N�擪�ʒu���i�[
			uiPos += u.us;
		} else {
			break;
		}
	}
	return ret;
}

//==============================================================================
//	��M�V�[�P���X�L�����Z���^�C�}�̋N��
//
//	@argument	ucSystemId		�V�X�e���h�c
//	@argument	ucDataId		�f�[�^�h�c
//
//	@return		TRUE			�^�C�}�N������(�h�a�v����̉����d�����������)
//	@return		FALSE			�^�C�}�N���Ȃ�(�h�a�v����̉����d�����Ȃ�����)
//
//	@note		�g�n�r�s�����M���鉓�u�V�X�e���d���łh�a�v����̉����d����
//				������̂̏ꍇ�h�a�v����̉�����҂^�C�}���N������B
//
//	@see		RAUid_GetRcvSeqFlagTable
//	@see		RAUid_GetRcvSeqFlagRemote
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
BOOL RAUid_RcvSeqCnt_Start(uchar ucSystemId, uchar ucDataId)
{
	int	i;

	switch (ucSystemId) {
	case 30:											// �V�X�e���h�c���R�O(���u�ŗL�d��)
		for( i = 0; i < sizeof(RAU_RemoteDataTbl); i++ ){
			if( RAU_RemoteDataTbl[i] == ucDataId ){
				RAU_f_RcvSeqCnt_rq = 1;					// ��M�V�[�P���X�L�����Z���^�C�}���싖��(�N��)
				RAU_Tm_RcvSeqCnt = 0;					// �^�C�}�l�N���A
				return TRUE;
			}
		}
		break;
	case 40:											// �V�X�e���h�c���S�O
		// �V�X�e��ID 40�̃f�[�^�ŉ������K�v�ȓd������M����ꍇ�AucDataId���`�F�b�N���邱��
		break;
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή�
	case PIP_SYSTEM:
		for( i = 0; i < sizeof(RAU_TableDataTbl_PIP); i++ ){
			if( RAU_TableDataTbl_PIP[i] == ucDataId ){
				RAU_f_RcvSeqCnt_rq = 1;				// ��M�V�[�P���X�L�����Z���^�C�}���싖��(�N��)
				RAU_Tm_RcvSeqCnt = 0;				// �^�C�}�l�N���A
				return TRUE;
			}
		}
		break;
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή�
	default:
		if( 10 <= ucSystemId && ucSystemId <= 19 ){		// �V�X�e���h�c���P�O�`�P�X(���u�V�X�e���d��)
			for( i = 0; i < sizeof(RAU_TableDataTbl); i++ ){
				if( RAU_TableDataTbl[i] == ucDataId ){
					RAU_f_RcvSeqCnt_rq = 1;				// ��M�V�[�P���X�L�����Z���^�C�}���싖��(�N��)
					RAU_Tm_RcvSeqCnt = 0;				// �^�C�}�l�N���A
					return TRUE;
				}
			}
		}
		break;
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_Unpack0Cut()
 *[]----------------------------------------------------------------------[]*
 *|	0�J�b�g���ꂽ�f�[�^�𕜌�����
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUid_Unpack0Cut(uchar *ucData) {

	ushort uiBefore0Cut;
	ushort uiAfter0Cut;

	union {
		uchar	uc[2];
		ushort	us;
	}u;

	u.uc[0] = ucData[0];
	u.uc[1] = ucData[1];

	// 0�J�b�g��̃T�C�Y���擾
	uiAfter0Cut = u.us - NT_DOPANT_HEAD_SIZE;

	u.uc[0] = ucData[2];
	u.uc[1] = ucData[3];

	// 0�J�b�g�O�̃T�C�Y���擾
	uiBefore0Cut = u.us;

	while (uiBefore0Cut > uiAfter0Cut) {
		ucData[uiAfter0Cut + NT_DOPANT_HEAD_SIZE] = 0x00;
		uiAfter0Cut++;
	}
}

//==============================================================================
//	�ʐM�`�F�b�N�̃G���[����
//
//	@argument	void
//
//	@return		void
//
//	@attention	�_�~�[�f�[�^�ɂ��ʐM�e�X�g���ʂ�OPE���M����
//
//	@note		�ʐM�`�F�b�N���̃_�~�[�f�[�^�̉�����M���̃R�[������
//======================================== Copyright(C) 2012 AMANO Corp. =======
void RAUid_CommuTest_Result(void)
{
	RAU_COMMUNICATION_TEST	*pBuff;

	if( RemoteCheck_Buffer100_101.CheckNo == 0 ){	// �ʐM�`�F�b�N�v���f�[�^���󂯕t���Ă��Ȃ�
		return;
	}
	pBuff = &RemoteCheck_Buffer100_101;

	RAU_Tm_CommuTest.tm = 0;						// �ʐM�`�F�b�N�^�C�}�^�C�}(��~)
	
	pBuff->Re.quest.Kind = 0;
	pBuff->Re.quest.TimeOut = 0;
	pBuff->NTpacket_len[1] = 62;					// �擪�Q�o�C�g��(�擪�Q�o�C�g���܂�)�b�q�b�L�������������d����
	pBuff->data_len[1] = 60;						// �f�[�^�����O�J�b�g���������{�w�b�_���̂P�O�o�C�g
	pBuff->data_len_zero_cut[1] = 50;				// �O�J�b�g����O�̃f�[�^���̒���
	pBuff->id4 = pBuff->id4 + 1;					// �ʐM�`�F�b�N����[�m�f]�f�[�^
	pBuff->Re.sult.Code = 0;

	RAU_CrcCcitt(60, 0, &pBuff->data_len[0], (uchar*)NULL, pBuff->crc, 1 );	// �����Q�o�C�g�ɂb�q�b���Z�o���ċL��
	
	if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_NORMAL ){	// ���������g�p
		RAUdata_SetThroughData((uchar *)pBuff, 64, 1);	// �X���[�f�[�^�o�b�t�@�ɓo�^�i1:OPE�����j
	}
	RauCT_SndReqID = 0;								// �ʐM�`�F�b�N�f�[�^���M�v���N���A
	memset(pBuff, 0, 64);
	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)
}
