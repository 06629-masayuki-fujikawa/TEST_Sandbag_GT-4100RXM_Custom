/************************************************************************************/
/*																					*/
/*		�V�X�e�����@:�@RAU�V�X�e��													*/
/*		�t�@�C����	:  rauhost.c													*/
/*		�@�\		:  Host���ʐM�A��ԑJ�ڏ���										*/
/*																					*/
/************************************************************************************/

#define	GLOBAL extern
#include	<string.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"

#include	"rau.h"
#include	"rauIDproc.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"iodefine.h"
#include	<stdlib.h>

#include	"trsocket.h"

#include	"ksgRauModem.h"

#define	RAU_RCV_SEQ_TIME_UP			300		// ��M�V�[�P���X�L�����Z���J�E���g�i300�b�j
#define	RAU_SND_SEQ_TIME_UP			300		// ���M�V�[�P���X�L�����Z���J�E���g�i300�b�j
#define	RAU_ERR_RCV_SEQ_TIM_OVER	0x0A	// �G���[�R�[�h�P�O(��M�V�[�P���X�t���O���X�|���X�^�C���A�E�g)

void	RAU_Mh_sdl(void);						// �z�X�g�ʐM���̏�ԑJ�ڏ���
void	DPA2Port_RestoreOfSendData(int port);	// �p�P�b�g�̍đ��M
void	DPA2Port_SendNextDataToHost(int port);	// ���̃p�P�b�g�̑��M����
void	DPA2Port_AckWait_TimeOut(int port);		// �`�b�j�҂��^�C�}�̃^�C���A�E�g����
void	DPA2Port_AckWait_Disconnected(int port);		// �`�b�j�҂�����TCP�ؒf������
void	RAU_Mh_sum_check(void);						// �z�X�g�ʐM���̃`�F�b�N�T���`�F�b�N����
void	DPA2Port_BeginToSendThroughData(void);	// �������ւ̑��M�f�[�^����
void	DPA2Port_BeginToSendTableData(void);	// ������ւ̑��M�f�[�^����
int		DPA2Port_RecvDataCheck(int port);		// ��M�f�[�^�̎�ʃ`�F�b�N
uchar	RAU_dat_check(int port);				// �z�X�g�ʐM���̎�M�f�[�^���ʏ���
void	RAU_DpaSndHedSet(uchar ucLastPacket, int port);	// �z�X�g�ʐM���̂c���o���w�b�_�[���o�^����
void	DPA2Port_DpaSeqIncrement(int port);		// �f�[�^���M�p�V�[�P���X�ԍ��̃C���N�������g
void	RAUhost_Init(void);
void	RAUhost_SetSendRequeat(RAU_SEND_REQUEST *send_request);
void	RAUhost_ClearSendRequeat(void);
void	RAUhost_CreateResponseData(uchar *pBuff, uchar ucResult, ushort uiExtra, ulong uiSeqNum, ushort *uiLen);
void	RAUhost_SetError(uchar ucErrCode);
void	DPA2Port_CreateDopaPacket(int port);
BOOL	RAUhost_GetNtData(void);
BOOL	DPA2Port_GetNtData(T_SEND_DATA_ID_INFO *pSendDataInfo, T_SEND_NT_DATA *pSendNtData);
void	RAUhost_CreateDopaPacketTsum(void);
BOOL	RAUhost_GetNtDataTsum(void);
short	RAUhost_CreateDopaPacketCash(void);
uchar	RAUhost_GetSndSeqFlag(void);
uchar	RAUhost_GetRcvSeqFlag(void);
void	RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag);
void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
BOOL	RAUhost_isDataReadEnd(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
short	RAUhost_IdentifyDataType(int port);
void	RAUhost_DwonLine_common_timeout(void);
short	RAUhost_CreateDopaPacketCTI(void);
BOOL	RAUhost_GetNtDataCenterTermInfo(void);
extern	BOOL RAUdata_GetCenterTermInfoData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);

extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAUarc_ErrEnque( uchar ModuleCode, uchar ErrCode, uchar f_Occur );
extern	BOOL	RAUdata_GetNtData(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
extern	void	RAUdata_Create0Data(RAU_NTDATA_HEADER *pNtDataHeader, uchar ucDataId);
extern	void	RAUdata_ReadNtData(uchar *pData, ushort wReadLen, RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
extern	BOOL	RAUdata_GetTsumData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);
extern	BOOL	RAUdata_GetCashData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);
extern	void	RAUdata_TableClear(uchar ucTableKind);
extern	void	RAUque_UpDate(T_SEND_DATA_ID_INFO *pSendIdInfo);
extern	T_SEND_DATA_ID_INFO*	RAUque_GetSendDataInfo( void );
extern	void	RAUque_DeleteQue(void);
extern	BOOL	RAUque_IsLastID( void );
extern	eRAU_TABLEDATA_SET	RAUid_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
extern	void	RAUdata_GetNtDataInfo(T_SEND_DATA_ID_INFO *pSendDataInfo, RAU_SEND_REQUEST *pReqInfo, uchar ucSendReq);
extern	void	RAUdata_GetNtDataInfoThrough(T_SEND_DATA_ID_INFO *pSendDataInfo);
extern	int		count_tsum_data(struct _ntman *ntman, uchar count_tsum);
extern	int		check_cash_que(char *que);
extern  void    data_rau_init(void);
extern	void	RAU_IniDatUpdate( void );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void RAUid_CommuTest_Error(void);
extern	void	RAUid_CommuTest_Result(void);

extern	void	RAUdata_DeleteTsumGroup(ushort ucSendCount);
extern	ulong	RAU_c_1mPastTimGet( ulong StartTime );
extern	void	Credit_SetError(uchar ucErrCode);

const uchar uc_dpa_rem_str[13]={'A','M','A','N','O','_','R','E','M','O','T','E',0};
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:�A�h���X���0x06039000-0x060FFFFF(1MB)

tRauCtrl RauCtrl;
tRauConfig	RauConfig;

tCreditCtrl CreditCtrl;
uchar	RAU_LogData[RAU_LOG_BUFF_SIZE];
uchar	RAU_NtLogData[RAU_LOG_BUFF_SIZE];
uchar	RAU_NtBlockLogData[RAU_LOG_BUFF_SIZE];
/*------------------------------------------------------------------------------*/
#pragma	section	
/*------------------------------------------------------------------------------*/
ushort	RAU_NtLogDataIndex = 0;
ushort	RAU_NtLogDataSize = 0;

ushort	RAUhost_ConvertNtBlockData(const uchar* pData, ushort size, uchar* pNtBlockData, uchar *blockCount);
short	RAUhost_CreateTableDataPacket(void);
void	RAUhost_CreateTsumData(void);
void	RAUhost_CreateTsumData_r10(ushort type);
short	RAUhost_ConvertDataIdToLogData(uchar dataId);
int		RAUhost_Receive(int port);

extern	int		KSG_gPpp_RauStarted;				// PPP�ڑ��t���O

//==============================================================================
//	�z�X�g�Ƃ̃f�[�^����M
//
//	@argument	�Ȃ�
//
//	@return		�Ȃ�
//
//	@attention	�z�X�g����̎�M�f�[�^���L���[�ɋl�߂�̂�
//				�c�o�`���W���[���̒S���ł��B
//
//	@note		�q�`�t���W���[���ɏ������ڂ����Ƃ�����I�ɃR�[�������
//				���[�`���ł��B�L���[�ɗ��܂�����M�f�[�^�̎��o����
//				���M�L���[�ւ̃f�[�^�����݂�S�����܂��B
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAU_Mh_sdl(void)
{
	if( RAU_f_RcvSeqCnt_rq == 1 ){								// �z�X�g���h�a�v����̃f�[�^��v�����Ă���Ƃ�(�^�C�}�̍쓮����)
		if(	RAU_Tm_RcvSeqCnt >= RAU_RCV_SEQ_TIME_UP ){			// �v���f�[�^�̎�M����T�O�b�o���Ă��h�a�v����̃f�[�^�����Ȃ���
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);		// �����Ȃ��Ɣ��f���Ď�M�V�[�P���X���O�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;								// ��M�V�[�P���X�L�����Z���^�C�}���싖��(����)
// 			RAU_Tm_RcvSeqCnt = 0;								// ��M�V�[�P���X�L�����Z���^�C�}(��~)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			RAUhost_SetError(ERR_RAU_RCV_SEQ_TIM_OVER);			// �G���[�R�[�h�P�O(��M�V�[�P���X�t���O���X�|���X�^�C���A�E�g)
			if( RemoteCheck_Buffer116_117.CheckNo != 0 ){		// �ʐM�`�F�b�N�v���f�[�^���g�n�r�s����󂯕t���Ă���B
				if( RemoteCheck_Buffer116_117.Re.sult.Code < 7 ){	// �u�����҂��v���O�̒i�K(�u���f�[�^��M�v���ĂȂ��Ƃ�)
					RemoteCheck_Buffer116_117.Re.sult.Code = 8;		// ���ʃR�[�h(������)�Ƃ���B
				}
				RAU_f_CommuTest_ov = 1;							// �ʐM�`�F�b�N�^�C�}�����^�C���A�E�g
			}
		}
	}
	if(RAU_f_SndSeqCnt_rq == 1){
		if(RAU_Tm_SndSeqCnt >= RAU_SND_SEQ_TIME_UP){
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);
		}
	}

	if( RauCT_SndReqID == 100 ){								// �ʐM�`�F�b�N�f�[�^(�h�c�P�O�O)���M�v����
		if(KSG_RauGetMdmCutState() != 0 ){						// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"��"�ؒf��"�ɂȂ����B
			RemoteCheck_Buffer100_101.Re.sult.Code = 5;			// ���ʃR�[�h(�T�[�o�[�_�E��)�Ƃ���B
			RAU_f_CommuTest_ov = 1;								// �ʐM�`�F�b�N�^�C�}�����^�C���A�E�g
		} else {
			if( RemoteCheck_Buffer100_101.CheckNo == 0 ){		// �ʐM�`�F�b�N���f�����J�n(���f�v���d���̎�M�ɂ��o�b�t�@���O�N���A����邽��)
				RauCT_SndReqID = 0;								// �ʐM�`�F�b�N�f�[�^(�h�c�P�O�O)���M�v���N���A
				RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X���O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
//				if( DPA_TCPcnct_req == 1 ){						// ������̃R�l�N�V�����v�����������B
				if( RauCtrl.TCPcnct_req == 1 ){					// ������̃R�l�N�V�����v�����������B
					RauCtrl.TCPcnct_req = 0;					// �ڑ�����߂�����B
				}
				if( RauCtrl.tcpConnectFlag.port.upload ) {		// ������s�b�o�ڑ����������B
					RAU_uc_retrycnt_h = 0;						// ������̑��M���g���C�J�E���^�N���A
					RAU_Tm_Ackwait.tm = 0;						// ������̂`�b�j��M�҂��^�C�}�N���A
					RAU_ui_data_length_h = 0;					// ��M�f�[�^���N���A
					RAU_uc_mode_h = S_H_IDLE;					// �`�b�j�҂����������ăA�C�h���Ɉڍs
					RAU_Tm_TCP_DISCNCT.tm = 0;
				}
			}
		}
	}

	if( RAU_f_CommuTest_ov == 1 ){	// �ʐM�`�F�b�N�^�C�}�^�C���A�E�g(�h�a�j���g�n�r�s�܂��͂h�a�v�ɑ΂��ĒʐM�`�F�b�N���ʃf�[�^�𑗐M)
		RAU_Tm_CommuTest.tm = 0;	// �ʐM�`�F�b�N�^�C�}�^�C�}(��~)
		RAUid_CommuTest_Error();
	}

	if (RAUhost_Receive(UPLINE) != 0) {				// ��M�L���[����f�[�^�ǂݍ���(���������̃f�[�^)
		switch( DPA2Port_RecvDataCheck(UPLINE) ){	// ��M�f�[�^(RAU_huc_rcv_work_h)�̎�ʔ���
		case 1:						// �`�b�j��M
			DPA2Port_SendNextDataToHost(UPLINE);	// ���̑��M�f�[�^�𑗐M�L���[�ɋl�߂�B
			break;
		case 2:						// �m�`�j��M
			DPA2Port_AckWait_TimeOut(UPLINE);		// �f�[�^�̍đ��M����(�K��񐔃I�[�o�[�Ń��f���ؒf�v��)
			break;
		default:					// �j���f�[�^�E���̑��f�[�^
			break;					// ���������`�b�j�E�m�`�j�ȊO�̎�M�͖{�����蓾�Ȃ��̂œǂݎ̂āB
		}
	} else {
		if( RAU_uc_mode_h == S_H_IDLE ){					// ��������`�b�j�҂��łȂ��Ƃ�
			DPA2Port_BeginToSendTableData();			// ������ɑ΂��đ��M���Ȃ���΂����Ȃ��e�[�u���f�[�^������Α��M����������B
		}
		if( RAU_GetUpNetState() == RAU_NET_PPP_OPENED ) {	// TCP�ؒf
			if( RAU_uc_mode_h == S_H_ACK ){			// �`�b�j��M�҂�
				if( RAU_f_Ackwait_reconnect_rq == 0 && RAU_f_Ackwait_reconnect_ov == 0 ){			// �Đڑ��^�C�}�N�����Ă��Ȃ�
					DPA2Port_AckWait_Disconnected(UPLINE);	// ������pTCP�ؒf������
				}
			}
		}
		if( RAU_f_Ackwait_reconnect_ov == 1 ){					// �Đڑ��^�C�}�^�C���A�E�g
			RAU_Tm_Ackwait_reconnect.tm = 0;				// �Đڑ��^�C�}�L�����Z��
			DPA2Port_RestoreOfSendData(UPLINE);			// �p�P�b�g�̍đ��M
			RauCtrl.TCPcnct_req = 1;				// TCP�ڑ��v��
		}
		if( RAU_f_Ackwait_ov == 1 ){						// ������p�`�b�j�҂��^�C�}�^�C���A�E�g
			RAU_Tm_Ackwait.tm = 0;							// ������p�`�b�j��M�҂��^�C�}�L�����Z��
			if( RAU_uc_mode_h == S_H_ACK ){					// �`�b�j��M�҂�
				if( KSG_RauGetMdmCutState() == 0 ){			// ���f���ؒf�X�e�[�^�X��"����"��Ԃ̂Ƃ�
					RAU_uc_retrycnt_h++;					// ���g���C�J�E���g
					DPA2Port_AckWait_TimeOut(UPLINE);	// ������p�^�C���A�E�g����
				}
			}
		}
	}

	if (RAUhost_Receive(DOWNLINE) != 0) {			// ��M�L���[�Ƀf�[�^����(����������̃f�[�^)
		switch( DPA2Port_RecvDataCheck(DOWNLINE) ){	// ��M�f�[�^(RAU_huc_rcv_work_h)�̎�ʔ���
		case 1:						// �`�b�j��M
			DPA2Port_SendNextDataToHost(DOWNLINE);	// ���̑��M�f�[�^�𑗐M�L���[�ɋl�߂�B
			break;
		case 2:						// �m�`�j��M
			DPA2Port_AckWait_TimeOut(DOWNLINE);		// �f�[�^�̍đ��M����(�K��񐔃I�[�o�[�ŃX���[�f�[�^�폜)
			break;
		case 3:						// ���̑��f�[�^
			RAU_Mh_sum_check();			// ��M�f�[�^�̃e�[�u���ւ̕ۑ����`�b�j�܂��͂m�`�j���������̑��M�L���[�ɋl�߂�B
			break;
		default:					// �j���f�[�^
			break;					// ���҂��Ă����`�b�j�E�m�`�j�łȂ����ߓǂݎ̂āB
		}
		if( RemoteCheck_Buffer100_101.CheckNo != 0 ){	// �ʐM�`�F�b�N�v���f�[�^���h�a�j����󂯕t���Ă���B
			RemoteCheck_Buffer100_101.Re.sult.Code = 7;	// ���ʃR�[�h��(���f�[�^��M)�Ƃ���B
		}
	} else {
		if( RAU_uc_mode_h2 == S_H_IDLE ){				// ���������`�b�j�҂��łȂ��Ƃ�
			DPA2Port_BeginToSendThroughData();		// �������ɑ΂��đ��M���Ȃ���΂����Ȃ��X���[�f�[�^������Α��M����������B
		}
		if( RAU_GetDownNetState() == RAU_NET_PPP_OPENED ) {	// TCP�ؒf
			if( RAU_uc_mode_h2 == S_H_ACK ){			// �`�b�j��M�҂�
				DPA2Port_AckWait_Disconnected(DOWNLINE);	// �������pTCP�ؒf������
			}
		}
		if( RAU_f_Ackwait2_ov == 1 ){					// �`�b�j�҂��^�C�}�^�C���A�E�g
			RAU_Tm_Ackwait2.tm = 0;						// �`�b�j��M�҂��^�C�}�L�����Z��
			if( RAU_uc_mode_h2 == S_H_ACK ){			// �`�b�j��M�҂�
				RAU_uc_retrycnt_h2++;					// ���g���C�J�E���g
				DPA2Port_AckWait_TimeOut(DOWNLINE);	// �������p�^�C���A�E�g����
			}
		}
	}
}

//==============================================================================
//	�O�J�b�g��̃f�[�^���Z�o
//
//	@argument	*pData		�f�[�^��
//	@argument	length		�f�[�^��̒���
//
//	@return		�O�J�b�g��̃f�[�^��̒���
//
//	@note		�f�[�^��̉E����O���A�����Ă��鐔�����̃f�[�^�񒷂��獷������
//
//	@author		2007.02.26:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
ushort GetZeroCut_len(uchar *pData, ushort length)
{
	if( length != 0 ){
		while( length ){
			if( pData[length-1] != 0x00 ){
				break;
			}
			length--;
		}
	}

	return length;
}

//==============================================================================
//	�ʐM�`�F�b�N���ʃf�[�^�̐���
//
//	@argument	*pData		���������f�[�^���i�[����o�b�t�@
//	@argument	id			�쐬����f�[�^�̂h�c
//
//	@return		�f�[�^��̒���
//
//	@note		�h�a�j���ێ����Ă���ʐM�`�F�b�N���ʃf�[�^�������[�g�p�P�b�g������B
//
//	@author		2007.02.26:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
ushort RAUhost_CreateDopaPacketTest(uchar *pData, uchar id)
{
	ushort	len;

	switch( id ){
	case 100:
		len = GetZeroCut_len((uchar *)&RemoteCheck_Buffer100_101.data_len, 36);		// �m�s�|�m�d�s�f�[�^���O�J�b�g
		RAU_Word2Byte((unsigned char *)&RemoteCheck_Buffer100_101.data_len, len);	// ���ʕ��{�ŗL����(�O�J�b�g��)�f�[�^��
		memcpy(&pData[DOPA_HEAD_SIZE], (char *)&RemoteCheck_Buffer100_101.data_len, len );
		break;
	case 117:
		len = GetZeroCut_len((uchar *)&RemoteCheck_Buffer116_117.data_len, 35);		// �m�s�|�m�d�s�f�[�^���O�J�b�g
		RAU_Word2Byte((unsigned char *)&RemoteCheck_Buffer116_117.data_len, len);	// ���ʕ��{�ŗL����(�O�J�b�g��)�f�[�^��
		memcpy(&pData[DOPA_HEAD_SIZE], (char *)&RemoteCheck_Buffer116_117.data_len, len );
		break;
	default:
		return( 0 );
	}
	len = len + DOPA_HEAD_SIZE + 2;									// �p�P�b�g��(�w�b�_���{�f�[�^���{�b�q�b)
	c_int32toarray(pData, (ulong)len, 4);							// �p�P�b�g���Z�b�g
	memcpy( &pData[4], uc_dpa_rem_str, 12 );						// "AMANO_REMOTE"
	c_int32toarray(&pData[16], RAU_ui_RAUDPA_seq, 3);					// �V�[�P���XNo.
	pData[19] = 0;													// �f�[�^���R�[�h(���g�p�O�Œ�)
	pData[20] = 0;													// �d�����ʃR�[�h(0:���f�v���Ȃ�, 1:���f�v������)
	pData[21] = 0;													// �s�b�o�u���b�N�i���o�[
	pData[22] = 1;													// �s�b�o�u���b�N�i���o�[
	pData[23] = 1;													// �s�b�o�u���b�N�X�e�[�^�X
	pData[24] = 0;													// �\��
	pData[25] = 0;													// �\��
	RAU_CrcCcitt( len - 2, 0, pData, NULL, &pData[len - 2], 1 );	// �b�q�b�Z�o

	return( len );
}

//==============================================================================
//	�p�P�b�g�̍đ��M
//
//	@argument	port	�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���w�b�_�͑��M���s���Ɠ������̂��g�p���܂��B
//
//	@note		�g�n�r�s����m�`�j����M�܂��͂`�b�j�����Ȃ��Ƃ��ɃR�[������
//				���M�Ɏ��s�����p�P�b�g���e�[�u���������đ��M�L���[�ɓ��꒼���܂��B
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_RestoreOfSendData(int port)
{
	if( port == UPLINE ){												// �����
			DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// �O�񑗐M�����c���o���p�P�b�g��������p�̑��M�L���[�Ƀf�[�^���Z�b�g
			RAU_Tm_TCP_DISCNCT.tm = 0;									// �s�b�o�ؒf�҂��^�C�}��~
			RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;	// �`�b�j�҂��^�C�}�N��
			RAU_f_Ackwait_rq = 1;
			RAU_f_Ackwait_ov = 0;
	} else {															// ������
			DpaSndQue_Set( RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE );	// �O�񑗐M�����c���o���p�P�b�g�������p�̑��M�L���[�Ƀf�[�^���Z�b�g
			RAU_Tm_Port_Watchdog.tm = 0;								// �������ʐM�Ď��^�C�}(��~)
			RAU_Tm_Ackwait2.bits0.bit_0_13 = RauConfig.Dpa_ack_wait_tm;	// �`�b�j�҂��^�C�}�N��
			RAU_f_Ackwait2_rq = 1;
			RAU_f_Ackwait2_ov = 0;
	}
}

//==============================================================================
//	��M�f�[�^�̎�ʃ`�F�b�N
//
//	@argument	port	�f�[�^����M�����|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		0		�j���f�[�^
//	@return		1		�`�b�j��M
//	@return		2		�m�`�j��M
//	@return		3		�f�[�^��M
//
//	@attention	�m�`�j�X�X(�đ��M�̒�~�v��)����M�����Ƃ��͂`�b�j�����Ƃ���B
//
//	@note		�z�X�g�����M�����f�[�^����͂��A���̎�ʂ�Ԃ��܂��B
//				�V�[�P���X�ԍ������Ғl�ƈقȂ�Ƃ��͔j���f�[�^�ƂȂ�܂��B
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
int DPA2Port_RecvDataCheck(int port)
{
	int		ResData;
	ulong	SendDataSeqNo, RecvDataSeqNo;
	uchar	ucCrc[2];

	ResData = RAU_dat_check(port);	// ��M�f�[�^(RAU_huc_rcv_work_h)�̎�ʔ���
	switch( ResData ){
	case 1:						// �`�b�j��M
	case 2:						// �m�`�j��M
		if( (port == UPLINE)&&(RauCtrl.DPA_TCP_DummySend != 0) ){		// �������"AMANO__DUMMY"�𑗐M�����ꍇ�̉���
			if( RauConfig.tcp_disconect_flag == 0 ) {					// �ؒf����
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// ������̂s�b�o�ڑ���ؒf
			}
			RauCtrl.DPA_TCP_DummySend = 0;
			if(RauCT_SndReqID == 100) {									// �ʐM�e�X�g�ɂ��_�~�[�f�[�^���M?
				RAUid_CommuTest_Result();
				break;
			}
			ResData = 0;				// �j���f�[�^�Ƃ���B
			break;
		}

		if( (port == UPLINE)&&(RAU_uc_mode_h == S_H_IDLE) ){	// ���������̃f�[�^��M�ł`�b�j�҂��łȂ��Ƃ�
			ResData = 0;		// �j���f�[�^�Ƃ���B
			break;
		}
		if( (port == DOWNLINE)&&(RAU_uc_mode_h2 == S_H_IDLE) ){	// ����������̃f�[�^��M�ł`�b�j�҂��łȂ��Ƃ�
			ResData = 0;		// �j���f�[�^�Ƃ���B
			break;
		}


		RecvDataSeqNo = c_arraytoint32( &RAU_huc_rcv_work_h[16], 3 );		// ��M�f�[�^(�`�b�j�E�m�`�j)�̃V�[�P���XNo.���擾
		if( port == UPLINE ){	// �����
			SendDataSeqNo = c_arraytoint32( &RAU_uc_txdata_h_keep[16], 3 );	// ���M�f�[�^�̃V�[�P���XNo.���擾
		} else {				// ������
			SendDataSeqNo = c_arraytoint32( &RAU_uc_txdata_h_keep2[16], 3 );// ���M�f�[�^�̃V�[�P���XNo.���擾
		}

		if( RecvDataSeqNo != SendDataSeqNo ){		// ���M�f�[�^�ɑ΂���`�b�j�E�m�`�j�łȂ�
			ResData = 0;							// �j���f�[�^�Ƃ���B
			break;									// �������^�C�}���^�C���A�E�g���邩����ȉ���������܂ő҂�
		}
		RAU_CrcCcitt(RAU_ui_data_length_h-2, 0, RAU_huc_rcv_work_h, NULL, ucCrc, 1);	// �b�q�b�Z�o

		if( (RAU_huc_rcv_work_h[RAU_ui_data_length_h-2] != ucCrc[0])
			||( RAU_huc_rcv_work_h[RAU_ui_data_length_h-1] != ucCrc[1]) ){	// �b�q�b���قȂ�B
			ResData = 0;							// �j���f�[�^�Ƃ���B
			if( port == UPLINE ){	// �����
				RAUhost_SetError(ERR_RAU_DPA_RECV_CRC);		// �G���[�R�[�h�T�S(�������M�f�[�^�b�q�b�G���[)
			} else {				// ������
				RAUhost_SetError(ERR_RAU_DPA_RECV_CRC_D);	// �G���[�R�[�h�W�Q(��������M�f�[�^�b�q�b�G���[)
			}
			break;									// �������^�C�}���^�C���A�E�g���邩����ȉ���������܂ő҂�
		}
		break;
	default:	// ���̑��̃f�[�^
		ResData = 0;			// �j���f�[�^�Ƃ���B
		break;
	case 0:		// �f�[�^��M
		ResData = 3;
		break;
	}

	return( ResData );
}

//==============================================================================
//	���̃p�P�b�g�̑��M����
//
//	@argument	port			�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		�P�p�P�b�g�̃f�[�^���M�ɑ΂��ăz�X�g����̂`�b�j����M�����Ƃ�
//				�{�֐����Ăяo����܂��B�f�[�^�̏I�[�łȂ��ꍇ��
//				���̃p�P�b�g���쐬���鏀�������܂��B
//
//	@see		Mh_next_send()
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_SendNextDataToHost(int port)
{
	short	logId;
	ushort	overwriteCount;				// ���M���ɏ㏑�����ꂽ�����M�f�[�^��
	uchar	endDataId;
	short	totalLogId;

	if( (pRAUhost_SendIdInfo->send_req == 2) &&
		(pRAUhost_SendIdInfo->send_data_count == 0) ) {
		// �o�b�`���M����0���f�[�^�͑��M���Ă��Ȃ��̂ő��M�V�[�P���X�̃C���N�������g�͂��Ȃ�
	}
	else {
		DPA2Port_DpaSeqIncrement( port );	// ���M�V�[�P���XNo.�C���N�������g
	}

	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;
	}
	else {
		endDataId = 41;
	}

	if( port == UPLINE ){	// �����
		if( RauCT_SndReqID != 0 ){									// �ʐM�`�F�b�N�f�[�^�𑗐M�ς�
			if( RauConfig.tcp_disconect_flag == 0 ) {					// �ؒf����
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// ������̂s�b�o�ڑ���ؒf
			}
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M���I�������̂ő��M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
			RauCT_SndReqID = 0;
			RAU_uc_retrycnt_h = 0;										// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
			RAU_Tm_Ackwait.tm = 0;										// ������p�`�b�j��M�҂��^�C�}�N���A
			RAU_uc_retrycnt_reconnect_h = 0;							// TCP�ؒf�����g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
			RAU_Tm_Ackwait_reconnect.tm = 0;							// ������p�Đڑ��^�C�}�N���A
			RAU_ui_data_length_h = 0;
			RAU_uc_mode_h = S_H_IDLE;									// �`�b�j�҂�������
			return;
		}

		RAUque_UpDate(pRAUhost_SendIdInfo);	// �Q�Ƃ��Ă���e�[�u���̑��M�󋵂��X�V

		// �s���v�c�胍�O�����M����
		if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId == endDataId){
			if(pRAUhost_SendIdInfo->pbuff_info->ui_syuType == 1) {
				totalLogId = eLOG_TTOTAL;
			}
			else {
				totalLogId = eLOG_GTTOTAL;
			}
			if( (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->send_complete_count)	// ���ݎQ�Ƃ��Ă���e�[�u���̃f�[�^�͑S���������B
				|| (pRAUhost_SendIdInfo->send_data_count == 0) ){	// �O���f�[�^�̏ꍇ�́A���̃e�[�u�������M�\��ɂȂ��ĂȂ�������B
				// ���M��������LOG���[�h�|�C���^�X�V
				Ope_Log_TargetVoidReadPointerUpdate(totalLogId, eLOG_TARGET_REMOTE);
				if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
					pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
					pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				// �s���v�f�[�^�̏ꍇ�͎c�胍�O�����݂���Α����đ��M
				if(Ope_Log_UnreadCountGet(totalLogId, eLOG_TARGET_REMOTE) > 0){
					if( (RauConfig.id30_41_mask_flg == 0) &&				// �s���v�W�v�f�[�^�̑��M�}�X�N�Ȃ�
						(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026�C 0:���M���Ȃ� 1:���M����
						RAUdata_DeleteTsumGroup(pRAUhost_SendIdInfo->send_complete_count);
						pRAUhost_SendIdInfo->send_complete_count = 0;
						pRAUhost_SendIdInfo->fill_up_data_count = 0;
						pRAUhost_SendIdInfo->crc_err_data_count = 0;
						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
							if(totalLogId == eLOG_TTOTAL) {
								pRAUhost_SendIdInfo->pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
								pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 1;	// T���v
							}
							else {
								pRAUhost_SendIdInfo->pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
								pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 11;	// GT���v
							}
							RAUhost_CreateTsumData_r10(pRAUhost_SendIdInfo->pbuff_info->ui_syuType);
						}
						else{
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
							pRAUhost_SendIdInfo->pbuff_info->ui_syuType = 1;	// T���v
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
							RAUhost_CreateTsumData();
						}
						pRAUhost_SendIdInfo->send_data_count = count_tsum_data(&RAU_tsum_man.man, 0);
						pRAUhost_SendIdInfo->send_complete_count = 0;

						RAU_uc_retrycnt_h = 0;		// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
						RAU_Tm_Ackwait.tm = 0;		// ������p�`�b�j��M�҂��^�C�}�N���A
						RAU_uc_retrycnt_reconnect_h = 0;		// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
						RAU_Tm_Ackwait_reconnect.tm = 0;		// ������p�Đڑ��҂��^�C�}�N���A
						RAU_ui_data_length_h = 0;
						RAU_uc_mode_h = S_H_IDLE;	// �`�b�j�҂�������

						RAU_uiTcpBlockNo = 1;	// ���̃e�[�u���f�[�^�𑗂�Ƃ��͂s�b�o�u���b�N���P����J�n
						RAU_ucTcpBlockSts = 0;	// ���̃p�P�b�g���ŏI�ɂȂ�ꍇ�͌�łP�ɂȂ�B
						return;
					}
				}
			}
		}

		if( (RAUque_IsLastID() == FALSE)||(RAU_ucTcpBlockSts == 0) ){	// ���M���������͓̂r���̃p�P�b�g
			if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId != endDataId){
				// ���M��������LOG���[�h�|�C���^�X�V
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId != -1){
					overwriteCount = Ope_Log_TargetVoidReadPointerUpdate(logId, eLOG_TARGET_REMOTE);
					if(overwriteCount) {
						// ���M���ɖ����M�f�[�^���㏑������Ă���Α��M�ς݃f�[�^���ɉ��Z����
						pRAUhost_SendIdInfo->send_complete_count += overwriteCount;
					}
					if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
					}
				}
			}
			if( (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->send_complete_count)	// ���ݎQ�Ƃ��Ă���e�[�u���̃f�[�^�͑S���������B
				|| (pRAUhost_SendIdInfo->send_data_count == 0) ){	// �O���f�[�^�̏ꍇ�́A���̃e�[�u�������M�\��ɂȂ��ĂȂ�������B
				pRAUhost_SendIdInfo = RAUque_GetSendDataInfo();		// �����Ŏ��̃e�[�u���ɐ؂�ւ�(���̑��M�\��e�[�u�����Ȃ���΂m�t�k�k���Ԃ�)
				memset( &RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA) );
				RAU_uiTcpBlockNo = 1;	// ���̃e�[�u���f�[�^�𑗂�Ƃ��͂s�b�o�u���b�N���P����J�n
				RAU_ucTcpBlockSts = 0;	// ���̃p�P�b�g���ŏI�ɂȂ�ꍇ�͌�łP�ɂȂ�B
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4532 �y���؉ێw�E�����z���Z�ōd�ݓ�����Ɏ������{�^���������ƁA�ȍ~�̐��Z�Ő��Z�f�[�^��NT-NET�T�[�o�ɑ��M����Ȃ�(No.25))
				// �����Ȃ��A�ؒf�L�Ȃ��x�ؒf����
				if(pRAUhost_SendIdInfo == NULL){
					if( RauConfig.tcp_disconect_flag == 0 ) {					// �ؒf����
						RAU_Tm_TCP_DISCNCT.tm = 0;
						RAU_f_TCPdiscnct_rq = 0;
						RAU_f_TCPdiscnct_ov = 1;	// ������̂s�b�o�ڑ���ؒf
					}
				}
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4532 �y���؉ێw�E�����z���Z�ōd�ݓ�����Ɏ������{�^���������ƁA�ȍ~�̐��Z�Ő��Z�f�[�^��NT-NET�T�[�o�ɑ��M����Ȃ�(No.25))
			}
		} else {													// ���M���������͍̂ŏI�p�P�b�g
			// ���M��������LOG���[�h�|�C���^�X�V
			if(pRAUhost_SendIdInfo->pbuff_info->uc_DataId != endDataId){
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId != -1){
					Ope_Log_TargetVoidReadPointerUpdate(logId, eLOG_TARGET_REMOTE);
					if(pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus == RAU_NEARFULL_NEARFULL){
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatus = RAU_NEARFULL_CANCEL;
						pRAUhost_SendIdInfo->pbuff_info->i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
					}
				}
			}
			if( RauConfig.tcp_disconect_flag == 0 ) {					// �ؒf����
				RAU_Tm_TCP_DISCNCT.tm = 0;
				RAU_f_TCPdiscnct_rq = 0;
				RAU_f_TCPdiscnct_ov = 1;	// ������̂s�b�o�ڑ���ؒf
			}

			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M���I�������̂ő��M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
			RAUque_DeleteQue();			// �e�[�u�����瑗�M���������f�[�^���폜
			pRAUhost_SendIdInfo = NULL;	// �e�[�u���̎Q�Ƃ���߂�B
		}
		RAU_uc_retrycnt_h = 0;		// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
		RAU_Tm_Ackwait.tm = 0;		// ������p�`�b�j��M�҂��^�C�}�N���A
		RAU_uc_retrycnt_reconnect_h = 0;		// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
		RAU_Tm_Ackwait_reconnect.tm = 0;		// ������p�Đڑ��҂��^�C�}�N���A
		RAU_ui_data_length_h = 0;
		RAU_uc_mode_h = S_H_IDLE;	// �`�b�j�҂�������
	} else {										// ������
		RAUque_UpDate(pRAUhost_SendThroughInfo);	// ���M���Ă���f�[�^�̏����X�V
		if( RAU_ucTcpBlockSts2 == 0 ){					// ���M���������͓̂r���̃p�P�b�g
			if( pRAUhost_SendThroughInfo->send_data_count <= pRAUhost_SendThroughInfo->send_complete_count ){	// ���M�X���[�f�[�^�e�[�u���̃f�[�^�͑S���������B
				pRAUhost_SendThroughInfo = NULL;	// ���ɑ���f�[�^�͂Ȃ��B
				memset( &RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA) );
				RAU_uiTcpBlockNo2 = 1;	// ���̃X���[�f�[�^�𑗂�Ƃ��͂s�b�o�u���b�N���P����J�n  
				RAU_ucTcpBlockSts2 = 0;	// ���̃p�P�b�g���ŏI�ɂȂ�ꍇ�͌�łP�ɂȂ�B  
			}
		} else {					// ���M���������͍̂ŏI�p�P�b�g
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]

			if( RauConfig.Dpa_port_watchdog_tm == 0 ){		// �������ʐM�Ď��^�C�}�g�p�֎~
				RAU_Tm_Port_Watchdog.tm = 0;
			} else {										// �������ʐM�Ď��^�C�}�g�p����
				RAU_Tm_Port_Watchdog.bits0.bit_0_13 = RauConfig.Dpa_port_watchdog_tm;
				RAU_f_Port_Watchdog_rq = 1;					// �������ʐM�Ď��^�C�}(�N��)
				RAU_f_Port_Watchdog_ov = 0;					// �^�C���A�E�g����܂łɑ��肪�s�b�o�R�l�N�V�������N���[�Y���Ȃ��Ƃ����炩��؂�܂��B
			}

			RAUdata_TableClear(0);	// �X���[�f�[�^�̍폜
			memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// �X���[�f�[�^�p�e�[�u�����N���A
			pRAUhost_SendThroughInfo = NULL;
		}
		RAU_uc_retrycnt_h2 = 0;		// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
		RAU_Tm_Ackwait2.tm = 0;		// �`�b�j��M�҂��^�C�}�L�����Z��
		RAU_ui_data_length_h = 0;
		RAU_uc_mode_h2 = S_H_IDLE;	// �`�b�j�҂�������
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_Mh_sum_check
 *[]----------------------------------------------------------------------[]
 *| summary	: �`�F�b�N�T���̃`�F�b�N����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAU_Mh_sum_check(void)
{
	eRAU_TABLEDATA_SET	bRet;
	ulong	ui_w_seq;
	uchar	ucCrc[2];
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if( RauConfig.Dpa_port_watchdog_tm == 0 ){	// �������Ď��^�C�}�g�p�֎~
		RAU_Tm_Port_Watchdog.tm = 0;
	} else {							// �������Ď��^�C�}�g�p����
		RAU_Tm_Port_Watchdog.bits0.bit_0_13 = RauConfig.Dpa_port_watchdog_tm;
		RAU_f_Port_Watchdog_rq = 1;			// �������Ď��^�C�}(�N��)
		RAU_f_Port_Watchdog_ov = 0;
	}
	ui_w_seq = c_arraytoint32( &RAU_huc_rcv_work_h[16], 3 );		// ��M�f�[�^�̃V�[�P���XNo.���擾
	RAU_CrcCcitt(RAU_ui_data_length_h-2, 0, RAU_huc_rcv_work_h, NULL, ucCrc, 1 );	// �b�q�b�Z�o
	if( ( ucCrc[0] == RAU_huc_rcv_work_h[RAU_ui_data_length_h-2] )&&	// �Z�o�����b�q�b��
		( ucCrc[1] == RAU_huc_rcv_work_h[RAU_ui_data_length_h-1] ) ){	// �擾�����b�q�b����v

		if( (( ui_w_seq != 0 )&&			// �V�[�P���XNo.0�͋�����M(���Z�b�g)�Ȃ̂ŏ��O
			( ui_w_seq == RAU_ui_seq_bak )) ){	// �O��̃V�[�P���XNo.�Ɣ�r
			bRet = RAU_DATA_NORMAL;			// �O��Ɠ���ȃf�[�^����M(���f�[�^��j�����Ă`�b�j�ԐM)
		} else {
			u.uc[0] = RAU_huc_rcv_work_h[21];	// �s�b�o�u���b�N�i���o�[�̎��o��
			u.uc[1] = RAU_huc_rcv_work_h[22];
			bRet = RAUid_SetRcvNtData(&RAU_huc_rcv_work_h[26], RAU_ui_data_length_h - 28, u.us, RAU_huc_rcv_work_h[23]);	// ��M�f�[�^(�v���f�[�^)�̏���
		}
		switch( bRet ){
		case RAU_DATA_NORMAL:
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_ACK, 0, ui_w_seq, &RAU_ui_txdlength_h2);		// �`�b�j�f�[�^�̍쐬
			RAU_ui_seq_bak = ui_w_seq;																// �O��̃V�[�P���XNo.��ۑ�
			break;
		case RAU_DATA_CONNECTING_FROM_HOST:	// RAUid_SetRcvNtData�֐��Ńe�[�u�����M���Ƀe�[�u�����M�v������M����Ƃ����Ԃ��B
			RAU_uc_rcvretry_cnt = 0;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 90, ui_w_seq, &RAU_ui_txdlength_h2);		// �m�`�j�X�O�f�[�^�쐬
			break;
		default:
			if( RAU_uc_rcvretry_cnt < RauConfig.Dpa_data_rcv_rty_cnt ){	// ��M���g���C�񐔃`�F�b�N
				RAU_uc_rcvretry_cnt++;
				RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 0, ui_w_seq, &RAU_ui_txdlength_h2);	// �m�`�j�f�[�^�̍쐬
			} else {										// ���g���C�񐔃I�[�o�[
				RAU_uc_rcvretry_cnt = 0;
				RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 99, ui_w_seq, &RAU_ui_txdlength_h2);	// �m�`�j�X�X�f�[�^�쐬
			}
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	} else {	// ��M�f�[�^���ƃw�b�_�̃f�[�^�����s��v�܂��̓`�F�b�N�T���m�f
		RAUhost_SetError(ERR_RAU_DPA_RECV_CRC_D);				// �G���[�R�[�h�W�Q(��������M�f�[�^�b�q�b�G���[)
		if( RAU_uc_rcvretry_cnt < RauConfig.Dpa_data_rcv_rty_cnt ){			// ��M���g���C�񐔃`�F�b�N
			RAU_uc_rcvretry_cnt++;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 0, ui_w_seq, &RAU_ui_txdlength_h2);	// �m�`�j�f�[�^�쐬
		} else {												// ���g���C�񐔃I�[�o�[
			RAU_uc_rcvretry_cnt = 0;
			RAUhost_CreateResponseData(RAU_huc_txdata_h2, RAU_NAK, 99, ui_w_seq, &RAU_ui_txdlength_h2);	// �m�`�j�X�X�f�[�^�쐬
		}
	}

	memset( RAU_huc_rcv_work_h, 0x00, RAU_RCV_MAX_H );				// ��M�f�[�^�o�b�t�@(���[�N)�N���A
	RAU_ui_data_length_h = 0;										// ��M�f�[�^���N���A

	DpaSndQue_Set(RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE);	// ���������M�L���[�ɕԐM�f�[�^���Z�b�g

	RAU_uc_mode_h2 = S_H_IDLE;	// �`�b�j�҂����������ăA�C�h���Ɉڍs
}

//==============================================================================
//	�`�b�j�҂��^�C�}�̃^�C���A�E�g����
//
//	@argument	port			�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		�z�X�g�ɑ΂��ăf�[�^�𑗐M�����Ƃ��`�b�j�̗��Ȃ��܂܈�莞�Ԃ��߂����
//				�{�֐����Ăяo����܂��B
//
//	@see		Mh_time_out
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_AckWait_TimeOut(int port)
{
	if( port == UPLINE ){	// �����
		if( RAU_uc_retrycnt_h <= RauConfig.Dpa_data_rty_cn ){	// ���g���C�J�E���^�`�F�b�N
			DPA2Port_RestoreOfSendData( port );			// �p�P�b�g�̍đ��M
		} else {										// ���g���C�I�[�o�[
			DPA2Port_DpaSeqIncrement( port );			// ���f���ؒf��f�[�^�̍đ��M������̂ŃV�[�P���XNo.��ς���B
			RAU_uc_retrycnt_h = 0;							// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait.tm = 0;							// ������p�`�b�j��M�҂��^�C�}�N���A
			RAU_uc_retrycnt_reconnect_h = 0;				// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait_reconnect.tm = 0;				// ������p�Đڑ��҂��^�C�}�N���A
			RAU_uc_mode_h = S_H_IDLE;						// �`�b�j�҂����������ăA�C�h���Ɉڍs
			RAUque_DeleteQue();							// �e�[�u�����瑗�M���������f�[�^���폜
			pRAUhost_SendIdInfo = NULL;					// �e�[�u���̎Q�Ƃ���߂�B
			KSG_RauSetMdmCutState(1);					// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"�ɂ���B
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER);	// �G���[�R�[�h�X�T(������f�[�^�đ��񐔃I�[�o�[)���h�a�v�ɒʒm
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
		}
	} else {				// ������
		if( RAU_uc_retrycnt_h2 <= RauConfig.Dpa_data_snd_rty_cnt ){	// ���g���C�J�E���^�`�F�b�N
			DPA2Port_RestoreOfSendData( port );			// �p�P�b�g�̍đ��M
		} else {										// ���g���C�I�[�o�[
			DPA2Port_DpaSeqIncrement( port );			// �������̃X���[�f�[�^�𑗂�Ȃ��̂ŃV�[�P���XNo.��ς���B
			RAU_uc_retrycnt_h2 = 0;							// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait2.tm = 0;							// �������p�`�b�j��M�҂��^�C�}�N���A
			RAU_uc_mode_h2 = S_H_IDLE;						// �`�b�j�҂����������ăA�C�h���Ɉڍs
			// �X���[�f�[�^���M�`�b�j�҂����g���C�I�[�o�[���A���M��񂪃N���A����Ă��Ȃ������̂ŃN���A����
			memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// �X���[�f�[�^�p�e�[�u�����N���A
			pRAUhost_SendThroughInfo = NULL;			// �e�[�u���̎Q�Ƃ���߂�B
			KSG_RauSetMdmCutState(1);					// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"�ɂ���B
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER_D);// �G���[�R�[�h�W�V(�������f�[�^�đ��񐔃I�[�o�[)���h�a�v�ɒʒm
			RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 			RAU_f_RcvSeqCnt_rq = 0;							// �z�X�g���h�a�v����̃f�[�^��v�����Ă����Ԃ̉���
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			RAUdata_TableClear(0);						// �X���[�f�[�^�̍폜
		}
	}
}

//==============================================================================
//	�`�b�j�҂��^�C�}��TCP�ؒf������
//
//	@argument	port			�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		�z�X�g�ɑ΂��ăf�[�^�𑗐M�����Ƃ��`�b�j�̗��Ȃ��܂�TCP�ؒf������
//				�{�֐����Ăяo����܂��B
//				ParkingWeb���[�h�o�����T�[�Ή��Œǉ�
//
//	@see		Mh_time_out
//
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_AckWait_Disconnected(int port)
{
	RAU_uc_retrycnt_reconnect_h++;	// TCP�ؒf�����g���C�J�E���g
	if( port == UPLINE ){	// �����
		if( RAU_uc_retrycnt_reconnect_h <= RauConfig.Dpa_data_rty_cn_disconnected ){	// ���g���C�J�E���^�`�F�b�N
			RAU_uc_retrycnt_h = 0;							// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait.tm = 0;							// ������p�`�b�j��M�҂��^�C�}�N���A
			RAU_Tm_Ackwait_reconnect.bits0.bit_0_13 = RauConfig.Dpa_com_wait_reconnect_tm;	// �Đڑ��^�C�}�N��
			RAU_f_Ackwait_reconnect_rq = 1;
			RAU_f_Ackwait_reconnect_ov = 0;
		} else {										// ���g���C�I�[�o�[
			DPA2Port_DpaSeqIncrement( port );			// ���f���ؒf��f�[�^�̍đ��M������̂ŃV�[�P���XNo.��ς���B
			RAU_uc_retrycnt_h = 0;							// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait.tm = 0;							// ������p�`�b�j��M�҂��^�C�}�N���A
			RAU_uc_retrycnt_reconnect_h = 0;				// ���g���C�J�E���^�N���A
			RAU_Tm_Ackwait_reconnect.tm = 0;				// ������p�Đڑ��҂��^�C�}�N���A
			RAU_uc_mode_h = S_H_IDLE;						// �`�b�j�҂����������ăA�C�h���Ɉڍs
			RAUque_DeleteQue();							// �e�[�u�����瑗�M���������f�[�^���폜
			pRAUhost_SendIdInfo = NULL;					// �e�[�u���̎Q�Ƃ���߂�B
			KSG_RauSetMdmCutState(1);					// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"�ɂ���B
			RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER);	// �G���[�R�[�h�X�T(������f�[�^�đ��񐔃I�[�o�[)���h�a�v�ɒʒm
			RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
		}
	} else {				// ������
		// ACK�҂����g���C�I�[�o�[�Ɠ����������s���B�������A���f���̐ؒf�͍s��Ȃ��B
		DPA2Port_DpaSeqIncrement( port );			// �������̃X���[�f�[�^�𑗂�Ȃ��̂ŃV�[�P���XNo.��ς���B
		RAU_uc_retrycnt_h2 = 0;							// ���g���C�J�E���^�N���A
		RAU_Tm_Ackwait2.tm = 0;							// �������p�`�b�j��M�҂��^�C�}�N���A
		RAU_uc_mode_h2 = S_H_IDLE;						// �`�b�j�҂����������ăA�C�h���Ɉڍs
		// �X���[�f�[�^���M�`�b�j�҂����g���C�I�[�o�[���A���M��񂪃N���A����Ă��Ȃ������̂ŃN���A����
		memset(&RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(T_SEND_DATA_ID_INFO));	// �X���[�f�[�^�p�e�[�u�����N���A
		pRAUhost_SendThroughInfo = NULL;			// �e�[�u���̎Q�Ƃ���߂�B
		RAUhost_SetError(ERR_RAU_DPA_SEND_RETRY_OVER_D);// �G���[�R�[�h�W�V(�������f�[�^�đ��񐔃I�[�o�[)���h�a�v�ɒʒm
		RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 		RAU_f_RcvSeqCnt_rq = 0;							// �z�X�g���h�a�v����̃f�[�^��v�����Ă����Ԃ̉���
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
		RAUdata_TableClear(0);						// �X���[�f�[�^�̍폜
	}
}

//==============================================================================
//	�z�X�g�֑��M����e�[�u���f�[�^�̏���
//
//	@argument	�Ȃ�
//
//	@return		�Ȃ�
//
//	@attention	�����p�P�b�g�Ɍׂ�f�[�^�𑗐M����ꍇ�A�Q�p�P�b�g�ڈȍ~�̃f�[�^��
//				DPA2Port_SendNextDataToHost()���p�ӂ������̂��L���[�ɋl�߂܂��B
//
//	@note		�z�X�g�֏�������g�p���ăe�[�u���f�[�^�̑��M���s���邩�𔻒f��
//				(�\�ł����)���M�f�[�^���P�p�P�b�g���p�ӂ��đ��M�L���[�ɋl�ߍ��݂܂��B
//
//	@see		DPA2Port_SendNextDataToHost
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_BeginToSendTableData(void)
{
	int		i;

	if( KSG_RauGetMdmCutState() != 0 )	// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"��"�ؒf��"�̂Ƃ�
		return;

	if( KSG_gPpp_RauStarted == 0 )	// ���f���X�e�[�^�X��"�ʐM��"�łȂ��Ƃ�
		return;

	if( (RauCtrl.tcpConnectFlag.port.upload == 1) && (Rau_SedEnable == 1) ){	// �s�b�o�ڑ��� && ���M��
		Rau_SedEnable = 0;
		if( RauCT_SndReqID != 0 ){											// �ʐM�`�F�b�N�f�[�^���M�v������
			RAU_ui_txdlength_h = RAUhost_CreateDopaPacketTest(RAU_huc_txdata_h, RauCT_SndReqID);
			if( RAU_ui_txdlength_h ){											// �ʐM�`�F�b�N����[�m�f]�f�[�^�̃p�P�b�g������
				RAU_uc_mode_h = S_H_ACK;										// �`�b�j�҂���Ԃ֑J��
				RAU_Tm_TCP_DISCNCT.tm = 0;									// �s�b�o�ؒf�҂��^�C�}��~
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
//				RAU_f_No_Action_ov = 1;									// �_�~�[�f�[�^�𑗐M���邽�߂ɖ��ʐM�^�C�}���^�C���A�E�g������
				if(	prm_get( COM_PRM,S_NTN,121,1,1) != 0) {					// ���ԏ�Z���^�[�`��
					RAU_f_No_Action_ov = 1;									// �_�~�[�f�[�^�𑗐M���邽�߂ɖ��ʐM�^�C�}���^�C���A�E�g������
				}
				else {
					DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// ������p�̑��M�L���[�Ƀf�[�^���Z�b�g
				}
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
				memcpy( RAU_uc_txdata_h_keep, RAU_huc_txdata_h, DOPA_HEAD_SIZE );	// ���M����p�P�b�g�̃w�b�_����(�c���o���w�b�_)���L��
				RAU_ui_txdlength_h_keep = RAU_ui_txdlength_h;
				RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;		// �`�b�j�҂��^�C�}�N��
				RAU_f_Ackwait_rq = 1;
				RAU_f_Ackwait_ov = 0;
				return;
			}
		}
	}

	if( (RauCtrl.tcpConnectFlag.port.upload == 1)	// �s�b�o�ڑ���
		&&(pRAUhost_SendIdInfo != NULL)		// ���݃e�[�u�����Q�ƒ�
		&&((pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->send_complete_count)||(pRAUhost_SendIdInfo->send_data_count == 0)) ){	// ���ݎQ�Ƃ��Ă���e�[�u���ɑ��M���ׂ��f�[�^���܂�����B


		if( RAUhost_Error01 == 1 ){													// �G���[�R�[�h�O�P(������ʐM�G���[)������
			RAUhost_Error01 = 0;
			RAU_err_chk(ERR_RAU_HOST_COMMUNICATION, 0, 0, 0, NULL);	// �G���[�O�P(����)���h�a�v�ɒʒm
		}

		if(RAUhost_IdentifyDataType( UPLINE ) == -1) {	// ���M����f�[�^���P�p�P�b�g���������ăo�b�t�@"RAU_huc_txdata_h"�ɏ[�U
			DPA2Port_SendNextDataToHost(UPLINE);		// ���̑��M�f�[�^�𑗐M�L���[�ɋl�߂�B
			return;
		}

		RAU_uc_mode_h = S_H_ACK;		// �`�b�j�҂���Ԃ֑J��
		RAU_Tm_TCP_DISCNCT.tm = 0;	// �s�b�o�ؒf�҂��^�C�}��~

		DpaSndQue_Set( RAU_huc_txdata_h, RAU_ui_txdlength_h, UPLINE );		// ������p�̑��M�L���[�Ƀf�[�^���Z�b�g

		memcpy( RAU_uc_txdata_h_keep, RAU_huc_txdata_h, DOPA_HEAD_SIZE );	// ���M����p�P�b�g�̃w�b�_����(�c���o���w�b�_)���L��
		RAU_ui_txdlength_h_keep = RAU_ui_txdlength_h;

		RAU_Tm_Ackwait.bits0.bit_0_13 = RauConfig.Dpa_com_wait_tm;		// �`�b�j�҂��^�C�}�N��
		RAU_f_Ackwait_rq = 1;
		RAU_f_Ackwait_ov = 0;
	} else {
		if( ((RauConfig.tcp_disconect_flag == 0) &&
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4532 �y���؉ێw�E�����z���Z�ōd�ݓ�����Ɏ������{�^���������ƁA�ȍ~�̐��Z�Ő��Z�f�[�^��NT-NET�T�[�o�ɑ��M����Ȃ�(No.25))
//			 (RAU_GetUpNetState() == RAU_NET_PPP_OPENED))	// ������̃R�l�N�V�����X�e�[�^�X�͂o�o�o�ڑ���
			// �Ȃ����Ă��炻�̂܂�
			 ((RAU_GetUpNetState() == RAU_NET_PPP_OPENED) ||		// ������̃R�l�N�V�����X�e�[�^�X�͂o�o�o�ڑ���
			  (RAU_GetUpNetState() == RAU_NET_TCP_CONNECTIED)) ) 	// ������̃R�l�N�V�����X�e�[�^�X�͂s�b�o�ڑ���
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4532 �y���؉ێw�E�����z���Z�ōd�ݓ�����Ɏ������{�^���������ƁA�ȍ~�̐��Z�Ő��Z�f�[�^��NT-NET�T�[�o�ɑ��M����Ȃ�(No.25))
		||  ((RauConfig.tcp_disconect_flag != 0) &&
			 ((RAU_GetUpNetState() == RAU_NET_PPP_OPENED) ||		// ������̃R�l�N�V�����X�e�[�^�X�͂o�o�o�ڑ���
			  (RAU_GetUpNetState() == RAU_NET_TCP_CONNECTIED))) ){	// ������̃R�l�N�V�����X�e�[�^�X�͂s�b�o�ڑ���
			if( pRAUhost_SendIdInfo == NULL ){	// ���݃e�[�u�����Q��
				if( RAU_f_TCPtime_wait_ov == 1 ){		// �s�b�o�R�l�N�V�����s�h�l�d�Q�v�`�h�s�����҂��^�C�}�^�C���A�E�g
					for(i=0; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){	// ���ׂẴe�[�u�����(�������X���[�f�[�^�͏���)���N���A
						memset( &RAUque_SendDataInfo[i], 0, sizeof(RAUque_SendDataInfo[0]) );
					}
					RAUdata_GetNtDataInfo(RAUque_SendDataInfo, &RAUhost_SendRequest, RAUhost_SendRequestFlag);	// �S�e�[�u�����"���M�Ώۃf�[�^"���̏����擾����B
					RAUque_CurrentSendData = (ushort)-1;			// ���ꂩ�珉�߂đ��M����f�[�^�̃e�[�u���ԍ�(0:���Ƀf�[�^, 1:�o�Ƀf�[�^, 2:���Z�f�[�^�c)
					RAUhost_SendRequestFlag = 0;					// �h�a�v�܂��̓z�X�g����̑��M�v���f�[�^����t���O(����)
					pRAUhost_SendIdInfo = RAUque_GetSendDataInfo();	// ���ꂩ�瑗�M����f�[�^�����݂���e�[�u�����Q��(RAUque_CurrentSendData �� ���M����ɓ���ŏ��̃e�[�u���ԍ�)
					memset( &RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA) );
					RAU_uc_retrycnt_h = 0;			// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
					RAU_uc_retrycnt_reconnect_h = 0;			// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j

					if( pRAUhost_SendIdInfo != NULL ){	// �S�e�[�u������������ǁA�ǂ̃e�[�u���̑��M�ΏۂɂȂ��Ă��Ȃ������Ƃ��͂m�t�k�k
						RauCtrl.TCPcnct_req = 1;		// �s�b�o�ڑ��v���t���O(�ڑ��v��)
						RAU_uiTcpBlockNo = 1;
						RAU_ucTcpBlockSts = 0;
						RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND);	// ���M�V�[�P���X�P(�g�n�r�s�ւ̃e�[�u�����M�J�n���)�ɂ���B

						if( RauCT_SndReqID != 0 ){						// �ʐM�`�F�b�N�f�[�^���M�v������
							RauCT_SndReqID = 0;							// ���M���~
						}
					} else {
						if( RauCT_SndReqID != 0 ){			// �ʐM�`�F�b�N�f�[�^���M�v������
							RauCtrl.TCPcnct_req = 1;		// �s�b�o�ڑ��v���t���O(�ڑ��v��)
							RAU_uiTcpBlockNo = 1;
							RAU_ucTcpBlockSts = 0;
							RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND);	// ���M�V�[�P���X�P(�g�n�r�s�ւ̃e�[�u�����M�J�n���)�ɂ���B
							Rau_SedEnable = 1;
							return;
						}
						else {
							// �ؒf����Ȃ�ΐؒf����
							if( RauConfig.tcp_disconect_flag == 0 && RauCtrl.tcpConnectFlag.port.upload == 1 ) {	// �ؒf�����TCP�ڑ���
								RAU_Tm_DISCONNECT.tm = 0;
								RAU_f_discnnect_rq = 0;
								RAU_f_discnnect_ov = 1;	// ������̂s�b�o�ڑ���ؒf
							}
							RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M���I�������̂ő��M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
							RAU_uc_retrycnt_h = 0;									// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
							RAU_Tm_Ackwait.tm = 0;									// ������p�`�b�j��M�҂��^�C�}�N���A
							RAU_uc_retrycnt_reconnect_h = 0;						// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
							RAU_Tm_Ackwait_reconnect.tm = 0;						// ������p�Đڑ��҂��^�C�}�N���A
							RAU_ui_data_length_h = 0;
							RAU_uc_mode_h = S_H_IDLE;								// �`�b�j�҂�������
						}
					}
				}
// MH810105(S) R.Endo 2021/12/27 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			// NOTE:�ȉ��̒ʐM�f���m�����ɂ���
			//   �EIBK�ł͓��삵�Ȃ������Ƃ��č폜����A�ʐM�f���m�͕ʓr�쐬�Ƃ���Ă���B
			//   �E�f�o�b�K���g�p���Ēʂ��菇�͈ȉ��̒ʂ�B
			//     1. �ʐM�`�F�b�N�f�[�^���M�v������(RauCT_SndReqID != 0)�̑��M�V�[�P���X�P�ݒ菈��
			//        (RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_TABLE_SEND))�Ƀu���[�N�|�C���g��ݒ肷��B
			//     2. LAN Simulator����ʐM�`�F�b�N�v���f�[�^�𑗐M����B
			//     3. �u���[�N�|�C���g�Œ�~��ALAN Simulator���N���[�Y����B
			//     4. ��L���蕶�Ƀu���[�N�|�C���g��ݒ肵�A�������ĊJ����B
			//     5. �u���[�N�|�C���g�Œ�~��ATCP�ڑ��v���t���O(RauCtrl.TCPcnct_req)��ڑ��v���Ȃ�(0)�ɏ��������A
			//        �������ĊJ����B
// MH810105(E) R.Endo 2021/12/27 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			} else if( RauCtrl.TCPcnct_req == 0 && RauCtrl.tcpConnectFlag.port.upload == 0){	// ���肩�������̂s�b�o�R�l�N�V�������N���[�Y���ꂽ�B
				RAU_uc_retrycnt_h = 0;								// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
				RAU_Tm_Ackwait.tm = 0;								// ������p�`�b�j��M�҂��^�C�}�N���A
				RAU_uc_retrycnt_reconnect_h = 0;					// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
				RAU_Tm_Ackwait_reconnect.tm = 0;					// ������p�Đڑ��҂��^�C�}�N���A
				RAU_ui_data_length_h = 0;							// ��M�f�[�^���N���A
				RAU_uc_mode_h = S_H_IDLE;							// �`�b�j�҂����������ăA�C�h���Ɉڍs
				RAUque_DeleteQue();								// �e�[�u�����瑗�M���������f�[�^�̂ݏ����B
				pRAUhost_SendIdInfo = NULL;						// �e�[�u���̎Q�Ƃ���߂�B
				KSG_RauSetMdmCutState(1);						// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"�ɂ���B
				RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)
// MH810105(S) R.Endo 2021/12/27 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;							// �z�X�g���h�a�v����̃f�[�^��v�����Ă����Ԃ̉���
				// NOTE:�ȉ��̎�M�V�[�P���X�O�ݒ菈���ɂ���
				//   ���PC��IBK���o�R����IBW�Ƀf�[�^�v�����Ă���Œ��ɏ���������PC���ؒf���ꂽ�ꍇ�A
				//   IBK�Ƃ��Ă�IBW����̉����f�[�^�����PC�ɑ��M����K�v���Ȃ��ׁAIBW����̉����҂����������Ă���B
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(E) R.Endo 2021/12/27 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
			}
		}
	}
}

//==============================================================================
//	�z�X�g�֑��M����X���[�f�[�^�̏���
//
//	@argument	�Ȃ�
//
//	@return		�Ȃ�
//
//	@attention	�X���[�f�[�^�����M�ł���͎̂�M�V�[�P���X���Q�̂Ƃ��Ɍ����܂��B
//
//	@note		�z�X�g�։��������g�p���ăX���[�f�[�^�̑��M���s���邩�𔻒f��
//				(�\�ł����)���M�f�[�^���P�p�P�b�g���p�ӂ��đ��M�L���[�ɋl�ߍ��݂܂��B
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_BeginToSendThroughData(void)
{
	if( KSG_RauGetMdmCutState() != 0 )	// ���f���ؒf�X�e�[�^�X��"�ؒf�v��"��"�ؒf��"�̂Ƃ�
		return;

	if( KSG_gPpp_RauStarted == 0 )	// ���f���X�e�[�^�X��"�ʐM��"�łȂ��Ƃ�
		return;

	if( (RauCtrl.tcpConnectFlag.port.download == 1)	// �s�b�o�ڑ���
		&&(pRAUhost_SendThroughInfo != NULL)	// ���݃e�[�u�����Q�ƒ�
		&&(pRAUhost_SendThroughInfo->send_data_count > pRAUhost_SendThroughInfo->send_complete_count) ){	// ���ݎQ�Ƃ��Ă���e�[�u���ɑ��M���ׂ��f�[�^���܂�����B


		RAUhost_IdentifyDataType( DOWNLINE );	// ���M����f�[�^���P�p�P�b�g���������ăo�b�t�@"RAU_huc_txdata_h2"�ɏ[�U

		RAU_uc_mode_h2 = S_H_ACK;		// �`�b�j�҂���Ԃ֑J��

		DpaSndQue_Set( RAU_huc_txdata_h2, RAU_ui_txdlength_h2, DOWNLINE );	// �������p�̑��M�L���[�Ƀf�[�^���Z�b�g

		memcpy( RAU_uc_txdata_h_keep2, RAU_huc_txdata_h2, DOPA_HEAD_SIZE );	// ���M����p�P�b�g�̃w�b�_����(�c���o���w�b�_)���L��
		RAU_ui_txdlength_h_keep2 = RAU_ui_txdlength_h2;

		RAU_Tm_Ackwait2.bits0.bit_0_13 = RauConfig.Dpa_ack_wait_tm;	// �`�b�j�҂��^�C�}�N��
		RAU_f_Ackwait2_rq = 1;
		RAU_f_Ackwait2_ov = 0;
	} else {
		if( RAU_GetDownNetState() == RAU_NET_TCP_CONNECTIED){	// �������̃R�l�N�V�����X�e�[�^�X�͂s�b�o�ڑ��m����
			if( pRAUhost_SendThroughInfo == NULL ){		// ���M�v�����������Ă���ŏ��̑��M�Ώۃf�[�^
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M�������)
					memset( &RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE], 0, sizeof(RAUque_SendDataInfo[0]) );
					RAUdata_GetNtDataInfoThrough(RAUque_SendDataInfo);				// �X���[�f�[�^�e�[�u�����"���M�Ώۃf�[�^��"���̏����擾����B

					if( RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].send_req != 0 ){	// �X���[�f�[�^�����M�e�[�u���ɂ���
						pRAUhost_SendThroughInfo = &RAUque_SendDataInfo[RAU_SEND_THROUGH_DATA_TABLE];
					} else {
						pRAUhost_SendThroughInfo = NULL;
					}
					memset( &RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA) );
					RAU_uc_retrycnt_h2 = 0;			// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j

					if( pRAUhost_SendThroughInfo != NULL ){
						RAU_uiTcpBlockNo2 = 1;
						RAU_ucTcpBlockSts2 = 0;
						RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_THROUGH_SEND);	// ��M�V�[�P���X�R(�g�n�r�s�ւ̃X���[�f�[�^���M�J�n���)�ɂ���B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 						//NOTE:���C�����牞���f�[�^��M��AHOST�փf�[�^���M�̃��g���C���Ƀ^�C���A�E�g���N����Ǝ�M�V�[�P���X�t���O���u0�v�Ƀ��Z�b�g����
// 						//�e�[�u���f�[�^�̑��M���J�n����t�F�[�Y������邱�Ƃ�����B����A���C�������M�����X���[�f�[�^���ŏI�p�P�b�g�܂ő��M�������_��
// 						//�^�C�}�N���A���Ă��邪�A��M�V�[�P���X�L�����Z���^�C�}�̓��C������X���[�f�[�^����M�������_�ŃN���A���ׂ��ł���
// 						RAU_f_RcvSeqCnt_rq = 0;//���C�����牞���f�[�^����M�����̂Ŏ�M�V�[�P���X�L�����Z���^�C�}��~
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
					}
				}
			}
		}
		else{	// �s�b�o�ڑ��m�����ł͂Ȃ�
			// �������ŃX���[�f�[�^�����𑗐M����ۂɂs�b�o���ڑ����̏ꍇ�A�e�[�u���Ƀf�[�^��
			// �c��A����s�b�o�ڑ��m�����ɑ��M����Ă��܂��ׁA�e�[�u������폜���Ă���
			if( pRAUhost_SendThroughInfo == NULL ){
			// �������M�O
				if( RAUhost_GetRcvSeqFlag() == RAU_RCV_SEQ_FLG_THROUGH_RCV ){		// ��M�V�[�P���X�Q(�g�n�r�s����̗v���f�[�^��M�������)
					RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);					// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 					RAU_f_RcvSeqCnt_rq = 0;											// ��M�V�[�P���X�L�����Z���^�C�}���싖��(����)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
				}
			}
			else {
			// �������M��
				RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);						// ��M�V�[�P���X�O(�ʏ���)�ɖ߂��B
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 				RAU_f_RcvSeqCnt_rq = 0;												// ��M�V�[�P���X�L�����Z���^�C�}���싖��(����)
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
				RAUdata_TableClear(0);												// �X���[�f�[�^�̍폜
				pRAUhost_SendThroughInfo = NULL;									// ���M�X���[�f�[�^�e�[�u�����|�C���^�N���A
				if( RAU_uc_mode_h2 == S_H_ACK ){									// �`�b�j��M�҂�
					DPA2Port_DpaSeqIncrement(DOWNLINE);								// �V�[�P���XNo.�C���N�������g
				}
				RAU_uc_retrycnt_h2 = 0;												// ���g���C�J�E���^���C�j�V�����C�Y�i�O�N���A�j
				RAU_Tm_Ackwait2.tm = 0;												// �`�b�j��M�҂��^�C�}�L�����Z��
				RAU_ui_data_length_h = 0;											// ��M�f�[�^���N���A
				RAU_uc_mode_h2 = S_H_IDLE;											// �`�b�j�҂�������
			}
		}
	}
}

//==============================================================================
//	��M�f�[�^�̎�ʔ���
//
//	@argument	port	�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		0		�f�[�^��M
//	@return		1		�`�b�j��M
//	@return		2		�m�`�j��M
//
//	@attention	�m�`�j�X�X(�đ��M�̒�~�v��)����M�����Ƃ��͂`�b�j�����Ƃ���B
//
//	@note		�z�X�g�����M�����f�[�^����͂��A���̎�ʂ�Ԃ��܂��B
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAU_dat_check(int port)
{
	uchar	result;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	u.uc[0] = RAU_huc_rcv_work_h[21];
	u.uc[1] = RAU_huc_rcv_work_h[22];

	result = 0;
	if( u.us == 1 ){					// �s�b�o�u���b�N�i���o�[���P
		if( RAU_huc_rcv_work_h[23] == 1 ){	// �s�b�o�u���b�N�X�e�[�^�X���P
			switch( RAU_huc_rcv_work_h[26] ){
			case 0x06:					// �`�b�j
				result = 1;
				break;
			case 0x15:					// �m�`�j
				u.uc[0] = RAU_huc_rcv_work_h[24];
				u.uc[1] = RAU_huc_rcv_work_h[25];
				if( u.us == 99 ){			// �\�����ڂX�X(�m�`�j�X�X)
					if( port == UPLINE ){	// �����
						RAUhost_SetError(ERR_RAU_DPA_RECV_NAK99);	// �G���[�R�[�h�X�V(��������M�f�[�^������~��M)���h�a�v�ɒʒm
					} else {				// ������
						RAUhost_SetError(ERR_RAU_DPA_RECV_NAK99_D);	// �G���[�R�[�h�W�W(���������M�f�[�^������~��M)���h�a�v�ɒʒm
					}
					result = 1;			// �`�b�j�Ɠ�������
				} else {
					result = 2;
				}
				break;
			}
		}
	}
	return	result;
}


/****************************************************************/
/*	�֐���		=	RAU_DpaSndHedSet							*/
/*	�@�\		=	Dopa���M�f�[�^ �w�b�_�[���o�^����			*/
/*	���l		=												*/
/****************************************************************/
void	RAU_DpaSndHedSet(uchar ucLastPacket, int port)
{
	ulong	seq_no;
	uchar	*pTxData;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if( port == UPLINE ){	// �����
		seq_no = RAU_ui_RAUDPA_seq;
		u.us = RAU_uiTcpBlockNo;
		pTxData = RAU_huc_txdata_h;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_h, 4);	// �f�[�^���Z�b�g
	}
	else if(port == CREDIT){
		seq_no = Credit_RAUDPA_seq;
		u.us = Credit_TcpBlockNo;
		pTxData = RAU_huc_txdata_CRE;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_CRE, 4);	// �f�[�^���Z�b�g
	} else {				// ������
		seq_no = RAU_ui_RAUDPA_seq2;
		u.us = RAU_uiTcpBlockNo2;
		pTxData = RAU_huc_txdata_h2;
		c_int32toarray(&pTxData[0], (ulong)RAU_ui_txdlength_h2, 4);// �f�[�^���Z�b�g
	}

	memcpy( &pTxData[4], uc_dpa_rem_str, 12 );					// "AMANO_REMOTE"�Z�b�g

	c_int32toarray(&pTxData[16], seq_no, 3);					// �V�[�P���XNo.�Z�b�g

	pTxData[19] = 0;											// �f�[�^���R�[�h(���g�p�O�Œ�)
	pTxData[20] = 0;											// �d�����ʃR�[�h(0:���f�v���Ȃ�, 1:���f�v������)

	pTxData[21] = u.uc[0];										// �s�b�o�u���b�N�i���o�[
	pTxData[22] = u.uc[1];

	pTxData[23] = ucLastPacket;									// �s�b�o�u���b�N�X�e�[�^�X

	pTxData[24] = 0x00;											// �\��
	pTxData[25] = 0x00;											// �\��
}

//==============================================================================
//	�f�[�^���M�p�V�[�P���X�ԍ��̃C���N�������g
//
//	@argument	port			�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		���M����p�P�b�g�f�[�^�ɕt������V�[�P���V�����ԍ���
//				�s�b�o�u���b�N�ԍ��̉��Z���s���܂��B
//
//	@see		RAU_DpaSeqAdd
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_DpaSeqIncrement(int port)
{
	if( port == UPLINE ){	// �����
		// �V�[�P���V�����ԍ�
		RAU_ui_RAUDPA_seq++;
		if( RAU_ui_RAUDPA_seq > 999 ){
			RAU_ui_RAUDPA_seq = 1;
		}

		// �s�b�o�u���b�N�ԍ�
		if( RAU_uiTcpBlockNo == 65535 ){
			RAU_uiTcpBlockNo = 1;
		} else {
			RAU_uiTcpBlockNo++;
		}
	} else {				// ������
		// �V�[�P���V�����ԍ�
		RAU_ui_RAUDPA_seq2++;
		if( RAU_ui_RAUDPA_seq2 > 999 ){
			RAU_ui_RAUDPA_seq2 = 1;
		}

		// �s�b�o�u���b�N�ԍ�
		if( RAU_uiTcpBlockNo2 == 65535 ){
			RAU_uiTcpBlockNo2 = 1;
		} else {
			RAU_uiTcpBlockNo2++;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_Init()
 *[]----------------------------------------------------------------------[]*
 *|	�������֐�
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_Init(void)
{
	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)
	RAU_ucTcpBlockSts = 0;
	RAU_uiTcpBlockNo = 1;
	RAU_ucTcpBlockSts2 = 0;
	RAU_uiTcpBlockNo2 = 1;
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 	RAU_f_RcvSeqCnt_rq = 0;
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]

	RAUhost_SendRequestFlag = 0;			// ���M�v���f�[�^����t���O
	RAUhost_Error01 = 0;					// �G���[�R�[�h�O�P������
	memset(&RAUhost_SendNtDataInfo, 0x00, sizeof(T_SEND_NT_DATA));		// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
	memset(&RAUhost_SendNtThroughInfo, 0x00, sizeof(T_SEND_NT_DATA));	// ���M�X���[�f�[�^�e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
	pRAUhost_SendIdInfo = NULL;				// ���M�Ώۂ̃e�[�u���o�b�t�@���̎Q��
	pRAUhost_SendThroughInfo = NULL;		// ���M�X���[�f�[�^�e�[�u���̎Q�Ƃ���߂�B

	memset(&RemoteCheck_Buffer100_101, 0, 64);
	memset(&RemoteCheck_Buffer116_117, 0, 64);
	RauCT_SndReqID = 0;
	Rau_SedEnable = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_SetError()
 *[]----------------------------------------------------------------------[]*
 *|	�G���[��ݒ肷��
 *[]----------------------------------------------------------------------[]*
 *|	patam	ucErrCode	�G���[�R�[�h
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_SetError(uchar ucErrCode)
{
	switch( ucErrCode ){
	case ERR_RAU_HOST_COMMUNICATION:							// �G���[�R�[�h�O�P(������ʐM�G���[)
		RAU_err_chk(ucErrCode, 0x01,0, 0, NULL );				// �G���[(����)���h�a�v�ɒʒm
		RAUhost_Error01 = 1;									// �G���[�O�P�����t���O�Z�b�g

		RAU_uc_retrycnt_h = 0;										// ������̑��M���g���C�J�E���^�N���A
		RAU_Tm_Ackwait.tm = 0;										// ������̂`�b�j��M�҂��^�C�}�N���A
		RAU_uc_retrycnt_reconnect_h = 0;							// ������̑��M���g���C�J�E���^�N���A
		RAU_Tm_Ackwait_reconnect.tm = 0;							// ������̍Đڑ��҂��^�C�}�N���A

		RAUque_DeleteQue();										// �e�[�u�����瑗�M���������f�[�^�̂ݏ����B
		pRAUhost_SendIdInfo = NULL;								// �e�[�u���̎Q�Ƃ���߂�B

		RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M�V�[�P���X�O��(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B

		RAU_ui_data_length_h = 0;									// ��M�f�[�^���N���A
		RAU_uc_mode_h = S_H_IDLE;									// �`�b�j�҂����������ăA�C�h���Ɉڍs
		break;
	case ERR_RAU_TCP_CONNECT:									// �G���[�R�[�h�X�P(������s�b�o�R�l�N�V�����^�C���A�E�g)
		if( RAUhost_Error01 == 0 ){								// �G���[�R�[�h�O�P�������̓G���[�Ƃ��Ȃ��̂Łc
			RAU_err_chk(ucErrCode, 0x02, 0, 0, NULL );			// �G���[(������������)���h�a�v�ɒʒm
		}
		break;
	default:
		RAU_err_chk(ucErrCode, 0x02, 0, 0, NULL);				// �G���[(������������)���h�a�v�ɒʒm
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_SetSendRequeat()
 *[]----------------------------------------------------------------------[]*
 *|	���M�v����ݒ肷��
 *[]----------------------------------------------------------------------[]*
 *|	patam	pSendRequest	���M�v���f�[�^
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_SetSendRequeat(RAU_SEND_REQUEST *pSendRequest) {

	RAUhost_SendRequestFlag = 1;
	memcpy(&RAUhost_SendRequest, pSendRequest, sizeof(RAU_SEND_REQUEST));
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ClearSendRequeat()
 *[]----------------------------------------------------------------------[]*
 *|	���M�v������������
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_ClearSendRequeat(void) {
	RAUhost_SendRequestFlag = 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateResponseData()
 *[]----------------------------------------------------------------------[]*
 *|	����Dopa�p�P�b�g�𐶐�����
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuff		�d���𐶐�����o�b�t�@
 *|	patam	ucResult	��������
 *|	patam	uiExtra		�\������
 *|	patam	uiSeqNum	�V�[�P���X��
 *|	patam	uiLen		���M�T�C�Y
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_CreateResponseData(uchar *pBuff, uchar ucResult, ushort uiExtra, ulong uiSeqNum, ushort *uiLen)
{
	uchar		ucCrc[2];
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	c_int32toarray(pBuff, 29, 4);									// �f�[�^���Z�b�g
	memcpy(&pBuff[4], uc_dpa_rem_str, 12);							// "AMANO_REMOTE"�Z�b�g
	c_int32toarray(&pBuff[16], uiSeqNum, 3);						// �V�[�P���XNo.�Z�b�g
	pBuff[19] = 0;													// �f�[�^���R�[�h(���g�p�O�Œ�)
	pBuff[20] = 0;													// �d�����ʃR�[�h(0:���f�v���Ȃ�, 1:���f�v������)
	u.us = 1;
	pBuff[21] = u.uc[0];											// �s�b�o�u���b�N�i���o�[
	pBuff[22] = u.uc[1];
	pBuff[23] = 1;													// �s�b�o�u���b�N�X�e�[�^�X
	u.us = uiExtra;
	pBuff[24] = u.uc[0];											// �\��
	pBuff[25] = u.uc[1];											// �\��
	pBuff[26] = ucResult;											// ����

	RAU_CrcCcitt(27, 0, pBuff, NULL, ucCrc, 1 );					// �b�q�b�Z�o
	pBuff[27] = ucCrc[0];
	pBuff[28] = ucCrc[1];

	*uiLen = 29;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_IdentifyDataType()
 *[]----------------------------------------------------------------------[]*
 *|	�h�c�ɂ���ăp�P�b�g�̐������@��؂芷����B
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	short	0:�p�P�b�g����OK -1:�p�P�b�g����NG
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short RAUhost_IdentifyDataType(int port){

	short	ret = 0;

	if( port == UPLINE ){
	switch(pRAUhost_SendIdInfo->pbuff_info->uc_DataId){
	case 41:														// �s���v�f�[�^
	case 53:														// �t�H�[�}�b�gRevNo.10 �s�^�f�s���v�f�[�^
		RAUhost_CreateDopaPacketTsum();
		break;
	case 126:														// ���K�Ǘ��f�[�^
		ret = RAUhost_CreateDopaPacketCash();
		break;
	case 65:														// �Z���^�[�p�[�����f�[�^
		ret = RAUhost_CreateDopaPacketCTI();
		break;
	case 20:														// ���Ƀf�[�^
// MH810100(S) m.saito 2020/05/15 �Ԕԃ`�P�b�g���X(#4169 ���Z�f�[�^�̑O��0���f�[�^�𑗐M���Ă��܂��s�)
	case 22:														// ���Z�f�[�^�i���O�j
// MH810100(E) m.saito 2020/05/15 �Ԕԃ`�P�b�g���X(#4169 ���Z�f�[�^�̑O��0���f�[�^�𑗐M���Ă��܂��s�)
	case 23:														// ���Z�f�[�^�i�o���j
	case 120:														// �G���[�f�[�^
	case 121:														// �A���[���f�[�^
	case 122:														// ���j�^�f�[�^
	case 123:														// ���샂�j�^�f�[�^
	case 131:														// �R�C�����ɏW�v���v�f�[�^
	case 133:														// �������ɏW�v���v�f�[�^
	case 236:														// ���ԑ䐔�f�[�^
	case 135:														// �ޑK�Ǘ��f�[�^
	case 125:														// ���u�Ď��f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	case 61:														// �������ԏ�ԃf�[�^
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
		ret = RAUhost_CreateTableDataPacket();
		break;
	default:
		DPA2Port_CreateDopaPacket( port );
		break;
	}
	}else{
		DPA2Port_CreateDopaPacket( port );
	}
	return ret;
}

//==============================================================================
//	�z�X�g�֑��M����f�[�^�p�P�b�g�̍쐬
//
//	@argument	port			�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		�Ȃ�
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		���M����p�P�b�g�f�[�^�ɕt������V�[�P���V�����ԍ���
//				�s�b�o�u���b�N�ԍ��̉��Z���s���܂��B
//
//	@see		RAUhost_CreateDopaPacket
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void DPA2Port_CreateDopaPacket(int port)
{
	ushort				uiReadLen;
	ushort				uiRemain;
	uchar				ucReadEnd;
	T_SEND_DATA_ID_INFO	*pSendDataInfo;
	T_SEND_NT_DATA		*pSendNtData;
	uchar				*pTxData;
	ushort				txdlength;
	ushort				PacketDataSize;		// �p�P�b�g�̃f�[�^���T�C�Y(�c���o��:�P�S�Q�O�o�C�g, �w�o������:�T�O�O�o�C�g)
	ulong				Start_1msLifeTime;	// �d����M���_��1ms���C�t�^�C�}�[
	ulong				Past_1msTime;		// ��M�`���݂܂ł̌o�ߎ��ԁix1ms�j
	union {
		ushort	us;
		uchar	uc[2];
	} u;

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// ���f������:�c���o��
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// ���f���Ȃ�:Ethernet
	}

	if( port == UPLINE ){	// �����
		pSendDataInfo = pRAUhost_SendIdInfo;
		pSendNtData = &RAUhost_SendNtDataInfo;
		pTxData = RAU_huc_txdata_h;
	}
	else if(port == CREDIT){	// �N���W�b�g���
		pSendDataInfo = &Credit_SendIdInfo;
		pSendNtData = &Credit_SendNtDataInfo;
		pTxData = RAU_huc_txdata_CRE;
	} else {				// ������
		pSendDataInfo = pRAUhost_SendThroughInfo;
		pSendNtData = &RAUhost_SendNtThroughInfo;
		pTxData = RAU_huc_txdata_h2;
	}

	ucReadEnd = 0;
	txdlength = 0;
	if( pSendDataInfo->send_data_count > 0 ){	// ���M�f�[�^����
		while( ucReadEnd == 0 ){
			// �m�s�f�[�^�̏����擾����
			if( DPA2Port_GetNtData(pSendDataInfo, pSendNtData) == FALSE ){	// �m�s�f�[�^�擾���s
				if(port == CREDIT){
					/* �N���W�b�g�ł�0���f�[�^���M���Ȃ� */
					txdlength = 0;
					ucReadEnd = 2;
					break;
				}
				RAUdata_Create0Data( (RAU_NTDATA_HEADER*)&pTxData[DOPA_HEAD_SIZE], pSendDataInfo->pbuff_info->uc_DataId );	// �O���f�[�^�̍쐬
				txdlength = 10;
				ucReadEnd = 2;
				break;
			}

			// �m�s�f�[�^�̎c�ʂ��Z�o
			if( pSendNtData->nt_data_end > pSendNtData->nt_data_next ){
				uiRemain = pSendNtData->nt_data_end - pSendNtData->nt_data_next;
			} else {
				uiRemain = (&(pSendDataInfo->pbuff_info->pc_AreaTop[pSendDataInfo->pbuff_info->ul_AreaSize]) - pSendNtData->nt_data_next)
							+ (pSendNtData->nt_data_end - pSendDataInfo->pbuff_info->pc_AreaTop);
			}

			// �m�s�f�[�^����ǂݏo���ʂ�����
			if( (PacketDataSize - txdlength) < uiRemain ){
				uiReadLen = (PacketDataSize - txdlength);
			} else {
				uiReadLen = uiRemain;
			}

			// �c���o���p�P�b�g�ɂm�s�f�[�^���[�U
			RAUdata_ReadNtData(&pTxData[DOPA_HEAD_SIZE + txdlength], uiReadLen, pSendDataInfo->pbuff_info, pSendNtData);
			/* ���uNT-NET���M�f�[�^�C������,�d���f�[�^���(ID4)��229 ��               */
			/* Main������M�������v�f�[�^�d�����O���֑��M����Ƃ���ł���Ɣ��f���� */
			if(	( port == DOWNLINE ) &&								// ������
				( pTxData[DOPA_HEAD_SIZE + txdlength +8] == 229) )	// NT-NET�d��ID��229
			{
				/* �����܂łŎ��v�f�[�^�d���̃����[�g�p�P�b�g�Ɗm��                                                 */
				/* NT-NET���v�f�[�^�d���͏]���̌`��(33byte)��e-timing�����ɑΉ������`��(41byte)��2�ʂ葶�݂���B    */
				/* �`���̈Ⴂ��0�J�b�g�O�̃f�[�^�T�C�Y�Ŕ��ʂ�e-timing�����ɑΉ������`���̏ꍇ�ɂ͓��ꏈ�����s���B  */
				u.uc[0] = pTxData[DOPA_HEAD_SIZE + txdlength +2];
				u.uc[1] = pTxData[DOPA_HEAD_SIZE + txdlength +3];								// 0�J�b�g�O�̃f�[�^�T�C�Yget
				if( u.us >= 41 ){																// �V���v�f�[�^�d���i�␳�l�t���j�ł����
					memcpy( &Start_1msLifeTime, &pTxData[DOPA_HEAD_SIZE + txdlength +47], 4 );	// ��M���_�̃��C�t�^�C�}�[�lget
					Past_1msTime = RAU_c_1mPastTimGet( Start_1msLifeTime );						// ��M���_����̌o�ߎ���get
					memcpy( &pTxData[DOPA_HEAD_SIZE + txdlength +43], &Past_1msTime, 4 );		// ��M���_����̌o�ߎ���set
				}
			}
			txdlength += uiReadLen;	// �ǂݍ��ݗʂ𑝉�
			if( RAUhost_isDataReadEnd(pSendDataInfo->pbuff_info, pSendNtData) ){	// �P�m�s�f�[�^�̍Ō�܂ŏ������ݏI��
				pSendDataInfo->fill_up_data_count++;	// fill���C���N�������g
			}

			// �������ݏI���`�F�b�N
			if( pSendDataInfo->send_data_count <= (pSendDataInfo->send_complete_count
												 + pSendDataInfo->fill_up_data_count
												 + pSendDataInfo->crc_err_data_count) ){
				ucReadEnd = 2;	// �Q�Ƃ��Ă���e�[�u���ɂ́A����ȏ�̃f�[�^���Ȃ��
			} else if ( txdlength >= PacketDataSize ){	// �p�P�b�g�T�C�Y�����ς��ɋl�ߍ��񂾁B
				ucReadEnd = 1;
			}
		}
	} else if(port == CREDIT){
		txdlength = 0;
		ucReadEnd = 2;
	} else {	// ���M�f�[�^�Ȃ�
		RAUdata_Create0Data( (RAU_NTDATA_HEADER*)&pTxData[DOPA_HEAD_SIZE], pSendDataInfo->pbuff_info->uc_DataId );	// �O���f�[�^�̍쐬
		txdlength = 10;
		ucReadEnd = 2;
	}
	txdlength = DOPA_HEAD_SIZE + txdlength + 2;		// �f�[�^���C���

	if( port == UPLINE ){										// �����
//		if( (uc_data_cancel_h == 1)								// ���f�v������(��Ή�)
//			||(RAUque_IsLastID() == TRUE)&&(ucReadEnd == 2) ){	// ���M�ΏۂɂȂ��Ă���Ō�̂h�c�̑S�f�[�^�̏[�U���I������B
		if( ucReadEnd == 2 ){									// �Q�Ƃ��Ă���e�[�u��(�h�c)�̑S�f�[�^�̏[�U���I������B
			RAU_ucTcpBlockSts = 1;									// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
		}
		RAU_ui_txdlength_h = txdlength;
		RAU_DpaSndHedSet(RAU_ucTcpBlockSts, port);					// �w�b�_�[�ݒ�
	}
	else if(port == CREDIT){
		if( ucReadEnd == 2 ){									// �S�X���[�f�[�^�̏[�U���I������B
			Credit_TcpBlockSts = 1;									// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
		}
		RAU_ui_txdlength_CRE = txdlength;
		RAU_DpaSndHedSet(Credit_TcpBlockSts, port);					// �w�b�_�[�ݒ�
	} else {													// ������
		if( ucReadEnd == 2 ){									// �S�X���[�f�[�^�̏[�U���I������B
			RAU_ucTcpBlockSts2 = 1;									// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
		}
		RAU_ui_txdlength_h2 = txdlength;
		RAU_DpaSndHedSet(RAU_ucTcpBlockSts2, port);					// �w�b�_�[�ݒ�
	}

	RAU_CrcCcitt( txdlength - 2, 0, pTxData, NULL, &pTxData[txdlength - 2], 1 );	// �b�q�b�Z�o
}

//==============================================================================
//	���M�m�s�f�[�^�̎擾
//
//	@argument	*pSendDataInfo	���M�Ώۃe�[�u���̏��
//	@argument	*pSendNtData	���M�f�[�^�̂m�s�]�m�d�s�f�[�^�P�ʂł̏��
//
//	@return		TRUE			�擾����
//	@return		FALSE			�擾���s
//
//	@attention	�c���o���̂Q�|�[�g�Ή���p���[�`���ł��B
//
//	@note		�z�X�g�֑��M����m�s�f�[�^���e�[�u�������擾���܂��B
//
//	@see		RAUhost_GetNtData
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
BOOL DPA2Port_GetNtData(T_SEND_DATA_ID_INFO *pSendDataInfo, T_SEND_NT_DATA *pSendNtData)
{
	if( pSendNtData->nt_data_start == NULL ){	// �m�t�k�k��������f�[�^���擾����
		while( pSendDataInfo->send_data_count > pSendDataInfo->crc_err_data_count ){	// �b�q�b�G���[�̂m�s�f�[�^�͑��M�ΏۂƂ͂��Ȃ�(�ǂݔ�΂�)
			if( RAUdata_GetNtData(pSendDataInfo->pbuff_info, pSendNtData) == TRUE ){	// �m�s�f�[�^�̏����擾 ��pSendNtData
				return TRUE;	// �擾�����f�[�^�͂b�q�b����
			} else {
				pSendDataInfo->crc_err_data_count++;	// �f�[�^���b�q�b�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
			}
		}
		return FALSE;	// �S���b�q�b�G���[��������A�d�����Ȃ��̂łO���f�[�^�𑗂�B
	} else {
		if( RAUhost_isDataReadEnd(pSendDataInfo->pbuff_info, pSendNtData) ){	// �擾�ς݂̂m�s�f�[�^���Ō�܂œǂ�ł���
			while( pSendDataInfo->send_data_count > (pSendDataInfo->send_complete_count
				+ pSendDataInfo->fill_up_data_count + pSendDataInfo->crc_err_data_count) ){
				if( RAUdata_GetNtData(pSendDataInfo->pbuff_info, pSendNtData) == TRUE ){
					return TRUE;	// �擾�����f�[�^�͂b�q�b����
				} else {
					pSendDataInfo->crc_err_data_count++;	// �f�[�^���b�q�b�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
				}
			}
		} else {
			return TRUE;
		}
	}
	return FALSE;	// �r������S���b�q�b�G���[
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
void RAUhost_CreateDopaPacketTsum(void) {

	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// �p�P�b�g�̃f�[�^���T�C�Y(�c���o��:�P�S�Q�O�o�C�g, �w�o������:�T�O�O�o�C�g)

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// ���f������:�c���o��
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// ���f���Ȃ�:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// �f�[�^����
		while (ucReadEnd == 0) {

			// �m�s�f�[�^�̏����擾����
			if (RAUhost_GetNtDataTsum() == FALSE) {
				// �f�[�^0��
				RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				RAU_ui_txdlength_h = 10;
				ucReadEnd = 2;
				break;
			}

			// �m�s�f�[�^�̎c�ʂ��Z�o
			uiRemain = RAUhost_SendNtDataInfo.nt_data_end - RAUhost_SendNtDataInfo.nt_data_next;


			// �m�s�f�[�^����ǂݏo���ʂ�����
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// �c�������p�P�b�g�ɂm�s�f�[�^���[�U
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h], RAUhost_SendNtDataInfo.nt_data_next, uiReadLen );
			

			// ���񏑂����݈ʒu���R�s�[�����������i�߂�
			RAUhost_SendNtDataInfo.nt_data_next += uiReadLen;

			// �ǂݍ��ݗʂ𑝉�
			RAU_ui_txdlength_h += uiReadLen;

			if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {
				// �P�m�s�f�[�^�̍Ō�܂ŏ������񂾂̂ŁAfill���C���N�������g
				pRAUhost_SendIdInfo->fill_up_data_count++;
			}

			// �������ݏI���`�F�b�N
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// ����ȏ�f�[�^���Ȃ���΁A���M�
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// �o�b�t�@�𖞂����Ă����瑗�M�
				ucReadEnd = 1;
			}
			break;
		}
	} else {
		// �f�[�^0��
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// �f�[�^���C���
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// ���f�v������
	if( ucReadEnd == 2 ){	// �Q�Ƃ��Ă���e�[�u��(�h�c�͕���)�̑S�f�[�^�̏[�U���I������B
		RAU_ucTcpBlockSts = 1;	// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
	}

	// �w�b�_�[�ݒ�
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// �b�q�b�Z�o
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );


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
BOOL RAUhost_GetNtDataTsum(void) {

	BOOL ret;

	ret = FALSE;

	if (RAUhost_SendNtDataInfo.nt_data_start == NULL) {				// NULL��������f�[�^���擾����

		while (pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->crc_err_data_count ) {

			if (RAUdata_GetTsumData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
				ret = TRUE;
				break;
			} else {
				// �f�[�^��CRC�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
				pRAUhost_SendIdInfo->crc_err_data_count++;
			}

		}
		if (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->crc_err_data_count) {
			// �S��CRC�G���[��������A�d�����Ȃ��̂�0���f�[�^�𑗂�B
			ret = FALSE;
		}
	} else {
		if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {			// �f�[�^�̍Ō�܂œǂ�ł���

			while (pRAUhost_SendIdInfo->send_data_count > (pRAUhost_SendIdInfo->send_complete_count
															+ pRAUhost_SendIdInfo->fill_up_data_count
															+ pRAUhost_SendIdInfo->crc_err_data_count)) {

				if (RAUdata_GetTsumData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
					ret = TRUE;
					break;
				} else {
					// �f�[�^��CRC�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
					pRAUhost_SendIdInfo->crc_err_data_count++;
				}
			}
		} else {
			ret = TRUE;
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
 *|	return	short	0:�p�P�b�g����쐬 -1:�p�P�b�g�쐬�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short RAUhost_CreateDopaPacketCash(void) {

	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// �p�P�b�g�̃f�[�^���T�C�Y(�c���o��:�P�S�Q�O�o�C�g, �w�o������:�T�O�O�o�C�g)
	ushort		logCount;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	short		ret;

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// ���f������:�c���o��
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// ���f���Ȃ�:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// �f�[�^����
		while (ucReadEnd == 0) {
			if(RAU_NtLogDataIndex == 0) {
				logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
				// ���O�f�[�^���������0���f�[�^�𑗐M����
				if(logCount == 0) {
					// �f�[�^0��
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
				Ope_Log_TargetDataVoidRead(eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
				startFlg = FALSE;

// ���Z�Ɋ֘A����Ƃ��A�O��Ɠ����Ȃ瑗�M���Ȃ� or �v�����������Ƃ��͂O�f�[�^�𑗐M����
				ret = NTNET_Edit_Data126((TURI_KAN *)RAU_LogData, (DATA_KIND_126 *)RAU_NtLogData);
				if(ret == 0) {
					// �f�[�^0��
					if (pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;		// �������M���͑��M���Ȃ�
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					// ���K�Ǘ��f�[�^��0���f�[�^���������M�����s��΍�
					pRAUhost_SendIdInfo->fill_up_data_count++;	// �[�U�ς݂Ƃ���
					break;
				}

				RAU_NtLogDataIndex = 0;
				RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, sizeof(DATA_KIND_126), RAU_NtBlockLogData, &blockCount);
			
			}

			// �m�s�f�[�^�̎c�ʂ��Z�o
			uiRemain = RAU_NtLogDataSize - RAU_NtLogDataIndex;


			// �m�s�f�[�^����ǂݏo���ʂ�����
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// �c�������p�P�b�g�ɂm�s�f�[�^���[�U
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h],
							&RAU_NtBlockLogData[RAU_NtLogDataIndex], uiReadLen );


			// ���񏑂����݈ʒu���R�s�[�����������i�߂�
			RAU_NtLogDataIndex += uiReadLen;

			// �ǂݍ��ݗʂ𑝉�
			RAU_ui_txdlength_h += uiReadLen;

			if(RAU_NtLogDataSize <= RAU_NtLogDataIndex) {
				// �P�m�s�f�[�^�̍Ō�܂ŏ������񂾂̂ŁAfill���C���N�������g
				pRAUhost_SendIdInfo->fill_up_data_count++;
				RAU_NtLogDataIndex = 0;
			}				

			// �������ݏI���`�F�b�N
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// ����ȏ�f�[�^���Ȃ���΁A���M�
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// �o�b�t�@�𖞂����Ă����瑗�M�
				ucReadEnd = 1;
			}

		}
	} else {
		if(pRAUhost_SendIdInfo->send_req == 2) {
			pRAUhost_SendIdInfo->send_data_count = 0;
			return -1;
		}
		// �f�[�^0��
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// �f�[�^���C���
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// ���f�v������
	if( ucReadEnd == 2 ){	// �Q�Ƃ��Ă���e�[�u��(�h�c)�̑S�f�[�^�̏[�U���I������B
		RAU_ucTcpBlockSts = 1;	// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
	}

	// �w�b�_�[�ݒ�
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// �b�q�b�Z�o
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

//==============================================================================
//	���M�V�[�P���X�t���O�̃Z�b�g
//
//	@argument	ucSndSeqFlag	���M�V�[�P���X�t���O
//
//	@return		�Ȃ�
//
//	@note		������̎g�p�󋵂ɂ���Ԃ�ω������܂��B
//
//	@see		RAUhost_SetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag)
{
	RAU_ucSendSeqFlag = ucSndSeqFlag;
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
// 	if(ucSndSeqFlag == RAU_RCV_SEQ_FLG_NORMAL){
	if ( ucSndSeqFlag == RAU_SND_SEQ_FLG_NORMAL ) {
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
		RAU_f_SndSeqCnt_rq = 0;
	}
	else{
		RAU_f_SndSeqCnt_rq = 1;
	}
	RAU_Tm_SndSeqCnt = 0;
}

//==============================================================================
//	��M�V�[�P���X�t���O�̃Z�b�g
//
//	@argument	ucRcvSeqFlag	��M�V�[�P���X�t���O
//
//	@return		�Ȃ�
//
//	@note		�������̎g�p�󋵂ɂ���Ԃ�ω������܂��B
//
//	@see		RAUhost_SetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag)
{
	RAU_ucReceiveSeqFlag = ucRcvSeqFlag;
// MH810105(S) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
	switch ( ucRcvSeqFlag ) {
	case RAU_RCV_SEQ_FLG_WAIT:			// �P�F�g�n�r�s����̗v���f�[�^��M�����҂����(�h�a�v����̃X���[�f�[�^��M�֎~)
		RAU_Tm_RcvSeqCnt = 0;
		RAU_f_RcvSeqCnt_rq = 1;
		break;
	case RAU_RCV_SEQ_FLG_THROUGH_RCV:	// �Q�F�g�n�r�s����̗v���f�[�^��M�������
		break;
	case RAU_RCV_SEQ_FLG_THROUGH_SEND:	// �R�F�g�n�r�s�ւ̃X���[�f�[�^���M�J�n���(�h�a�v����̃X���[�f�[�^��M�֎~)
	// NOTE: ���C�����牞���f�[�^��M��AHOST�փf�[�^���M�̃��g���C���Ƀ^�C���A�E�g���N����Ǝ�M�V�[�P���X�t���O���u0�v�Ƀ��Z�b�g����
	//       �e�[�u���f�[�^�̑��M���J�n����t�F�[�Y������邱�Ƃ�����B����A���C�������M�����X���[�f�[�^���ŏI�p�P�b�g�܂ő��M�������_��
	//       �^�C�}�N���A���Ă��邪�A��M�V�[�P���X�L�����Z���^�C�}�̓��C������X���[�f�[�^����M�������_�ŃN���A���ׂ��ł���B
	case RAU_RCV_SEQ_FLG_NORMAL:		// �O�F�ʏ���(�h�a�v����̗v���f�[�^��M����)
	default:
		RAU_f_RcvSeqCnt_rq = 0;
		break;
	}
// MH810105(E) R.Endo 2021/12/07 �Ԕԃ`�P�b�g���X3.0 #6191 �y����w�E�zNT-NET���M�v���f�[�^��M����5�����E7210����/���� [���ʉ��P���� No1524]
}

//==============================================================================
//	���M�V�[�P���X�t���O�̎擾
//
//	@argument	�Ȃ�
//
//	@return		�t���O�̒l
//
//	@note		���݂̏�����̏�Ԃ��擾���܂��B
//
//	@see		RAUhost_GetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAUhost_GetSndSeqFlag(void)
{
	return RAU_ucSendSeqFlag;
}

//==============================================================================
//	�V�[�P���X�t���O�̎擾
//
//	@argument	�Ȃ�
//
//	@return		�t���O�̒l
//
//	@note		���݂̉������̏�Ԃ��擾���܂��B
//
//	@see		RAUhost_GetReceiveSeqFlag
//
//	@author		2006.10.24:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
uchar RAUhost_GetRcvSeqFlag(void)
{
	return RAU_ucReceiveSeqFlag;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_isDataReadEnd()
 *[]----------------------------------------------------------------------[]*
 *|	���݃o�b�t�@�ɏ[�U���̃f�[�^���Ō�܂ŏ[�U���I����������肷��B
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo		�o�b�t�@���
 *|	patam	pSendNtDataInf	���M�f�[�^���
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUhost_isDataReadEnd(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf) {

	BOOL ret;

	ret = FALSE;

	// �m�s�f�[�^�̎c�ʂ��Z�o
	if (pSendNtDataInf->nt_data_end > pSendNtDataInf->nt_data_start) {
		// 	start < end	:	end <= next
		if (pSendNtDataInf->nt_data_end <= pSendNtDataInf->nt_data_next) {
			ret = TRUE;
		}
	} else {
		// 	start > end	:	end <= next && next < start
		if ((pSendNtDataInf->nt_data_end <= pSendNtDataInf->nt_data_next) && (pSendNtDataInf->nt_data_next < pSendNtDataInf->nt_data_start)) {
			ret = TRUE;
		}
	}

	return ret;
}
//==============================================================================
// TCP Connection �v�����s���̃N���A����
//
// TCP Connection 3����ײ���s��.
// (�֐�.RAUhost_SetError( ) : case RAU_ERR_HOST_COMMUNICATION:
// �Ɠ����̏������s��.
//
// CD �� OFF ���ꂽ���� TCP CONNECTION���̏ꍇ�ɃR�[�������.
//==============================================================================
void RAUhost_TcpConnReq_Clear ( void )
{
	if ( RauCtrl.TCPcnct_req == 1 && pRAUhost_SendIdInfo != NULL ){
		RAU_uc_retrycnt_h = 0;										// ������̑��M���g���C�J�E���^�N���A
		RAU_Tm_Ackwait.tm = 0;										// ������̂`�b�j��M�҂��^�C�}�N���A
		RAU_uc_retrycnt_reconnect_h = 0;							// ������̑��M���g���C�J�E���^�N���A
		RAU_Tm_Ackwait_reconnect.tm = 0;							// ������̍Đڑ��҂��^�C�}�N���A

		RAUque_DeleteQue();										// �e�[�u�����瑗�M���������f�[�^�̂ݏ����B
		pRAUhost_SendIdInfo = NULL;								// �e�[�u���̎Q�Ƃ���߂�B

		RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M�V�[�P���X�O��(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B

		RAU_ui_data_length_h = 0;									// ��M�f�[�^���N���A
		RAU_uc_mode_h = S_H_IDLE;									// �`�b�j�҂����������ăA�C�h���Ɉڍs
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_DwonLine_common_timeout()
 *[]----------------------------------------------------------------------[]*
 *|	�������ʐM�Ď��^�C�}�[���^�C���A�E�g�������̏���
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	A.Iiizumi
 *| Date	:	2010-07-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUhost_DwonLine_common_timeout(void)
{
	if(RAUhost_GetRcvSeqFlag()!=RAU_RCV_SEQ_FLG_NORMAL ){
// ��M�V�[�P���X�O�ȊO�i�r���̃p�P�b�g�܂ł���M���j��FOMA�̓d�g���؂�^�C���A�E�g������
// ��M�V�[�P���X���O�ɖ߂��A�A�C�h����Ԃɖ߂��B
// ��M�V�[�P���X���O�ɖ߂��Ȃ��Ǝ���HOST����X���[�f�[�^����M�������G���[�i��M�u���b�N�ԍ��ُ� �R�[�h81�j�ƂȂ邽��
		RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)
		RAUdata_TableClear(0);// �X���[�f�[�^�̍폜
		RAU_uiLastTcpBlockNum = 0;
		RAU_uiDopaReceiveLen = 0;
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTableDataPacket()
 *[]----------------------------------------------------------------------[]*
 *|	�z�X�g�֑��M����e�[�u���f�[�^�p�P�b�g�̍쐬
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	short	0:�p�P�b�g����쐬 -1:�p�P�b�g�쐬�Ȃ�
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
short RAUhost_CreateTableDataPacket(void)
{
	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// �p�P�b�g�̃f�[�^���T�C�Y(�c���o��:�P�S�Q�O�o�C�g, �w�o������:�T�O�O�o�C�g)
	ushort		logCount;
	ushort		readCount = 0;
	ushort		unreadCount = 0;
	short		logId = 0;
	uchar		dataId;
	ushort		size;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	DATA_KIND_120 *dataKind120;
	DATA_KIND_121 *dataKind121;
	DATA_KIND_122 *dataKind122;
	DATA_KIND_123 *dataKind123;
	DATA_KIND_63 *dataKind63;
	DATA_KIND_64 *dataKind64;
	Err_log		*errLog;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
	Receipt_data	*p_RcptDat;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// ���f������:�c���o��
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// ���f���Ȃ�:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// �f�[�^����
		while (ucReadEnd == 0) {
			if(RAU_NtLogDataIndex == 0) {
				logId = RAUhost_ConvertDataIdToLogData(pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
				if(logId == -1){
					// �f�[�^0��
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
				logCount = Ope_Log_UnreadCountGet(logId, eLOG_TARGET_REMOTE);
				// ���O�f�[�^���������0���f�[�^�𑗐M����
				if(logCount == 0){
					// �f�[�^0��
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					break;
				}
// MH810100(S)
				// �A�����l�����Ĉ�xtaskchg
				taskchg(IDLETSKNO);
// MH810100(E)
				Ope_Log_TargetDataVoidRead(logId, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
				startFlg = FALSE;

				size = 0;
				switch(pRAUhost_SendIdInfo->pbuff_info->uc_DataId){
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//				case 20:											// ���Ƀf�[�^
//					dataId = NTNET_Edit_isData20_54((enter_log *)RAU_LogData);
//					if(dataId == 20){
//						NTNET_Edit_Data20((enter_log *)RAU_LogData, (DATA_KIND_20 *)RAU_NtLogData);
//						size = sizeof(DATA_KIND_20);
//						readCount++;
//					}
//					else if(dataId == 54){
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
////							size = NTNET_Edit_Data54_r10((enter_log *)RAU_LogData, (DATA_KIND_54_r10 *)RAU_NtLogData);
//							size = NTNET_Edit_Data54_r13((enter_log *)RAU_LogData, (DATA_KIND_54_r13 *)RAU_NtLogData);
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//						}
//						else {
//							size = NTNET_Edit_Data54((enter_log *)RAU_LogData, (DATA_KIND_54 *)RAU_NtLogData);
//						}
//						readCount++;
//					}
//					break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
				case 23:											// ���Z�f�[�^
					dataId = NTNET_Edit_isData22_56((Receipt_data *)RAU_LogData);
					if(dataId == 22){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						p_RcptDat = (Receipt_data *)RAU_LogData;
						if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_receive == 0) {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
						NTNET_Edit_Data22((Receipt_data *)RAU_LogData, (DATA_KIND_22 *)RAU_NtLogData);
						size = sizeof(DATA_KIND_22);
						readCount++;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						}
						else {
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
					}
					else if(dataId == 56){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						p_RcptDat = (Receipt_data *)RAU_LogData;
						if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_receive == 0) {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
////							size = NTNET_Edit_Data56_r10((Receipt_data *)RAU_LogData, (DATA_KIND_56_r10 *)RAU_NtLogData);
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
////							size = NTNET_Edit_Data56_r14((Receipt_data *)RAU_LogData, (DATA_KIND_56_r14 *)RAU_NtLogData);
//							size = NTNET_Edit_Data56_r17((Receipt_data *)RAU_LogData, (DATA_KIND_56_r17 *)RAU_NtLogData);
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//						}
//						else {
//							size = NTNET_Edit_Data56((Receipt_data *)RAU_LogData, (DATA_KIND_56 *)RAU_NtLogData);
//						}
						// SetAddPayData()�Ŋ��ɐݒ�34-0121���Q�Ƃ��Ă���̂ŁA
						// �����Ŕ��肷��K�v�͂Ȃ�
						size = NTNET_Edit_Data56_rXX((Receipt_data *)RAU_LogData, (DATA_KIND_56_rXX *)RAU_NtLogData);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
						readCount++;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						}
						else {
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
					}
					break;
				case 120:											// �G���[�f�[�^
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
						NTNET_Edit_Data63((Err_log *)RAU_LogData, (DATA_KIND_63 *)RAU_NtLogData);
						dataKind63 = (DATA_KIND_63 *)RAU_NtLogData;

						errLog = (Err_log *)RAU_LogData;
						if( (dataKind63->Errsyu == ERRMDL_FLAP_CRB ) &&
							(errLog->Errinf == 0) &&
							((dataKind63->Errcod == ERR_FLAPLOCK_LOCKCLOSEFAIL) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_LOCKOPENFAIL) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_DOWNRETRYOVER) ||
							 (dataKind63->Errcod == ERR_FLAPLOCK_DOWNRETRY)) ){
							// err_chk2�ŏ�L�̃G���[���O�o�^����18-0019�C�E=0�Ȃ�G���[����t��
							// ���Ȃ��悤�ɂ��Ă���̂ŁA�����ł̓G���[��ʁA�G���[�R�[�h�A�G���[���Ȃ�
							// �̏ꍇ�̓G���[�f�[�^�𑗐M���Ȃ��悤�ɂ���
							// ���G���[���Ȃ��ׁ̈A�Ԏ������Ƀt���b�v�A���b�N�̐؂蕪�����ł��Ȃ��̂�
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iE96XX�����u�ő��M���Ȃ��j
						else if(dataKind63->Errsyu == ERRMDL_ALARM) {		// E96XX
							// E96XX�͑��M���Ȃ�
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iE96XX�����u�ő��M���Ȃ��j
						else {
							if(dataKind63->Errlev >= RauConfig.error_send_level){
								size = sizeof(DATA_KIND_63);
								readCount++;
							}
							else{
								unreadCount++;
								pRAUhost_SendIdInfo->send_data_count--;
							}
						}
						break;
					}
					NTNET_Edit_Data120((Err_log *)RAU_LogData, (DATA_KIND_120 *)RAU_NtLogData);
					dataKind120 = (DATA_KIND_120 *)RAU_NtLogData;

					// �ׯ�ߑ��u�㏸���~ۯ��ň󎚂��Ȃ��ݒ莞�͒ʐM���󎚂��������\�����s��Ȃ�(۸ޓo�^�̂�)
					errLog = (Err_log *)RAU_LogData;
					if( (dataKind120->Errsyu == ERRMDL_FLAP_CRB ) &&
						(errLog->Errinf == 0) &&
						((dataKind120->Errcod == ERR_FLAPLOCK_LOCKCLOSEFAIL) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_LOCKOPENFAIL) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_DOWNRETRYOVER) ||
						 (dataKind120->Errcod == ERR_FLAPLOCK_DOWNRETRY)) ){
						// err_chk2�ŏ�L�̃G���[���O�o�^����18-0019�C�E=0�Ȃ�G���[����t��
						// ���Ȃ��悤�ɂ��Ă���̂ŁA�����ł̓G���[��ʁA�G���[�R�[�h�A�G���[���Ȃ�
						// �̏ꍇ�̓G���[�f�[�^�𑗐M���Ȃ��悤�ɂ���
						// ���G���[���Ȃ��ׁ̈A�Ԏ������Ƀt���b�v�A���b�N�̐؂蕪�����ł��Ȃ��̂�
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					else {
						if(dataKind120->Errlev >= RauConfig.error_send_level){
							size = sizeof(DATA_KIND_120);
							readCount++;
						}
						else{
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
					}
					break;
				case 121:											// �A���[���f�[�^
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
						NTNET_Edit_Data64((Arm_log *)RAU_LogData, (DATA_KIND_64 *)RAU_NtLogData);
						dataKind64 = (DATA_KIND_64 *)RAU_NtLogData;
						if(dataKind64->Armlev >= RauConfig.alarm_send_level){
							size = sizeof(DATA_KIND_64);
							readCount++;
						}
						else{
							unreadCount++;
							pRAUhost_SendIdInfo->send_data_count--;
						}
						break;
					}
					NTNET_Edit_Data121((Arm_log *)RAU_LogData, (DATA_KIND_121 *)RAU_NtLogData);
					dataKind121 = (DATA_KIND_121 *)RAU_NtLogData;
					if(dataKind121->Armlev >= RauConfig.alarm_send_level){
						size = sizeof(DATA_KIND_121);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 122:											// ���j�^�f�[�^
					NTNET_Edit_Data122((Mon_log *)RAU_LogData, (DATA_KIND_122 *)RAU_NtLogData);
					dataKind122 = (DATA_KIND_122 *)RAU_NtLogData;
// MH810103 GG119202 (s) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
//					if(dataKind122->Monlev >= RauConfig.monitor_send_level){
					if( 0 == chk_mon_send_ntnet(((Mon_log*)RAU_LogData)->MonKind,((Mon_log*)RAU_LogData)->MonCode)){
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}else if(dataKind122->Monlev >= RauConfig.monitor_send_level){
// MH810103 GG119202 (e) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
						size = sizeof(DATA_KIND_122);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 123:											// ���샂�j�^�f�[�^
					NTNET_Edit_Data123((Ope_log *)RAU_LogData, (DATA_KIND_123 *)RAU_NtLogData);
					dataKind123 = (DATA_KIND_123 *)RAU_NtLogData;
// MH810103 GG119202 (s) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
//					if(dataKind123->OpeMonlev >= RauConfig.opemonitor_send_level){
					if( 0 == chk_opemon_send_ntnet(((Ope_log*)RAU_LogData)->OpeKind,((Ope_log*)RAU_LogData)->OpeCode)){
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}else if(dataKind123->OpeMonlev >= RauConfig.opemonitor_send_level){
// MH810103 GG119202 (e) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
						size = sizeof(DATA_KIND_123);
						readCount++;
					}
					else{
						unreadCount++;
						pRAUhost_SendIdInfo->send_data_count--;
					}
					break;
				case 131:											// �R�C�����ɏW�v���v�f�[�^
					NTNET_Edit_Data131((COIN_SYU *)RAU_LogData, (DATA_KIND_130 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_130);
					readCount++;
					break;
				case 133:											// �������ɏW�v���v�f�[�^
					NTNET_Edit_Data133((NOTE_SYU *)RAU_LogData, (DATA_KIND_132 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_132);
					readCount++;
					break;
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//				case 236:											// ���ԑ䐔�f�[�^
//					dataId = NTNET_Edit_isData236_58((ParkCar_log *)RAU_LogData);
//					if(dataId == 236){
//						NTNET_Edit_Data236((ParkCar_log *)RAU_LogData, (DATA_KIND_236 *)RAU_NtLogData);
//						size = sizeof(DATA_KIND_236);
//						readCount++;
//					}
//					else if(dataId == 58){
//						if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//							size = NTNET_Edit_Data58_r10((ParkCar_log *)RAU_LogData, (DATA_KIND_58_r10 *)RAU_NtLogData);
//						}
//						else {
//							size = NTNET_Edit_Data58((ParkCar_log *)RAU_LogData, (DATA_KIND_58 *)RAU_NtLogData);
//						}
//						readCount++;
//					}
//					break;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				case 135:											// �ޑK�Ǘ��W�v�f�[�^
					NTNET_Edit_Data135((TURI_KAN *)RAU_LogData, (DATA_KIND_135 *)RAU_NtLogData);
					size = sizeof(DATA_KIND_135);
					readCount++;
					break;
				case 125:
					NTNET_Edit_Data125_DL((Rmon_log *)RAU_LogData, (DATA_KIND_125_DL *)RAU_NtLogData);
					size = sizeof(DATA_KIND_125_DL);
					readCount++;
					break;
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//				case 61:											//�������ԏ��f�[�^
//					NTNET_Edit_Data61((LongPark_log_Pweb *)RAU_LogData, (DATA_KIND_61_r10 *)RAU_NtLogData);
//					size = sizeof(DATA_KIND_61_r10);
//					readCount++;
//					break;
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				default:
					// �f�[�^0��
					if(pRAUhost_SendIdInfo->send_req == 2) {
						pRAUhost_SendIdInfo->send_data_count = 0;
						return -1;
					}
					RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
					RAU_ui_txdlength_h = 10;
					ucReadEnd = 2;
					readCount++;
					break;
				}
				if(size != 0){
					RAU_NtLogDataIndex = 0;
					RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);
				}
				else{
					if(logCount == unreadCount){
						pRAUhost_SendIdInfo->send_data_count = 0;
						if(pRAUhost_SendIdInfo->send_req == 2) {
							return -1;
						}
						// �S�����M���x��������0���f�[�^���M
						RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
						RAU_ui_txdlength_h = 10;
						ucReadEnd = 2;
						break;
					}
					if((logCount - (readCount + unreadCount)) > 0){
						continue;
					}
					else{
						ucReadEnd = 2;
						break;
					}
				}
			}
			// �m�s�f�[�^�̎c�ʂ��Z�o
			uiRemain = RAU_NtLogDataSize - RAU_NtLogDataIndex;

			// �m�s�f�[�^����ǂݏo���ʂ�����
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				Ope_Log_UndoTargetDataVoidRead(logId, eLOG_TARGET_REMOTE);	// ��ǂ݈ʒu��߂�
				ucReadEnd = 1;
				break;
			} else {
				uiReadLen = uiRemain;
			}

			// �c�������p�P�b�g�ɂm�s�f�[�^���[�U
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h],
							&RAU_NtBlockLogData[RAU_NtLogDataIndex], uiReadLen );
			

			// ���񏑂����݈ʒu���R�s�[�����������i�߂�
			RAU_NtLogDataIndex += uiReadLen;

			// �ǂݍ��ݗʂ𑝉�
			RAU_ui_txdlength_h += uiReadLen;

			if(RAU_NtLogDataSize <= RAU_NtLogDataIndex) {
				// �P�m�s�f�[�^�̍Ō�܂ŏ������񂾂̂ŁAfill���C���N�������g
				pRAUhost_SendIdInfo->fill_up_data_count++;
				RAU_NtLogDataIndex = 0;
			}				
			
			// �������ݏI���`�F�b�N
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// ����ȏ�f�[�^���Ȃ���΁A���M�
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// �o�b�t�@�𖞂����Ă����瑗�M�
				ucReadEnd = 1;
			}

		}
	} else {
		if(pRAUhost_SendIdInfo->send_req == 2) {
			pRAUhost_SendIdInfo->send_data_count = 0;
			return -1;
		}
		// �f�[�^0��
		RAUdata_Create0Data((RAU_NTDATA_HEADER*)&RAU_huc_txdata_h[DOPA_HEAD_SIZE], pRAUhost_SendIdInfo->pbuff_info->uc_DataId);
		RAU_ui_txdlength_h = 10;
		ucReadEnd = 2;
	}

	// �f�[�^���C���
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// ���f�v������
	if( ucReadEnd == 2 ){	// �Q�Ƃ��Ă���e�[�u��(�h�c)�̑S�f�[�^�̏[�U���I������B
		RAU_ucTcpBlockSts = 1;	// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
	}

	// �w�b�_�[�ݒ�
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// �b�q�b�Z�o
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTsumData()
 *[]----------------------------------------------------------------------[]*
 *|	�W�v�f�[�^�̍쐬�Atsum�i�[
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAUhost_CreateTsumData(void)
{
	ushort		logCount;
	uchar		i,j;
	ushort		size;
	ushort		zerosize;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	t_RemoteNtNetBlk*	pPacketData;				// �p�P�b�g�f�[�^

	// tsum�̈�̃��Z�b�g
	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	
	logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
	// ���O�f�[�^���������0���f�[�^�𑗐M����
	if(logCount != 0) {
		Ope_Log_TargetDataVoidRead(eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		startFlg = FALSE;
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		for(i=0; i<8; i++) {
		for(i=0; i<7; i++) {
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			switch(i) {
			case 0:
				// �W�v��{�f�[�^(30)
				NTNET_Edit_SyukeiKihon((SYUKEI *)RAU_LogData, 1, (DATA_KIND_30 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_30);
				break;
			case 1:
				// ������ʖ��W�v�f�[�^(31)
				NTNET_Edit_SyukeiRyokinMai((SYUKEI *)RAU_LogData, 1, (DATA_KIND_31 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_31);
				break;
			case 2:
				// ���ޏW�v�f�[�^(32)
				NTNET_Edit_SyukeiBunrui((SYUKEI *)RAU_LogData, 1, (DATA_KIND_32 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_32);
				break;
			case 3:
				// �����W�v�f�[�^(33)
				NTNET_Edit_SyukeiWaribiki((SYUKEI *)RAU_LogData, 1, (DATA_KIND_33 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_33);
				break;
			case 4:
				// ����W�v�f�[�^(34)
				NTNET_Edit_SyukeiTeiki((SYUKEI *)RAU_LogData, 1, (DATA_KIND_34 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_34);
				break;
			case 5:
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//				// �Ԏ����W�v�f�[�^(35)
//				NTNET_Edit_SyukeiShashitsuMai((SYUKEI *)RAU_LogData, 1, (DATA_KIND_35 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_35);
//				break;
//			case 6:
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				// ���K�W�v�f�[�^(36)
				NTNET_Edit_SyukeiKinsen((SYUKEI *)RAU_LogData, 1, (DATA_KIND_36 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_36);
				break;
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			case 7:
			case 6:
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				// �W�v�I���ʒm�f�[�^(41)
				NTNET_Edit_SyukeiSyuryo((SYUKEI *)RAU_LogData, 1, (DATA_KIND_41 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_41);
				break;
			default:
				break;
			}

			RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);

			// tsum�̈揑����
			for(j=0; j<blockCount; j++){
				pPacketData = (t_RemoteNtNetBlk*)(RAU_NtBlockLogData + (j * 970));
				zerosize = pPacketData->header.packet_size[0] * 0x100;
				zerosize += pPacketData->header.packet_size[1];
				write_tsum((char*)pPacketData, zerosize);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_CreateTsumData_r10()
 *[]----------------------------------------------------------------------[]*
 *|	�W�v�f�[�^�̍쐬�Atsum�i�[(�t�H�[�}�b�gRevNo.10�p)
 *[]----------------------------------------------------------------------[]*
 *|	patam	ushort	type	: 1=�s���v 2=�f�s���v
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAUhost_CreateTsumData_r10(ushort type)
{
	ushort		logCount;
	uchar		i,j;
	ushort		size;
	ushort		zerosize;
	uchar		blockCount;
	BOOL		startFlg = TRUE;
	t_RemoteNtNetBlk*	pPacketData;				// �p�P�b�g�f�[�^

	// tsum�̈�̃��Z�b�g
	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);

	if(type == 1) {
		logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
	}
	else if(type == 11){
		logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
	}
	else {
		return;
	}
	// ���O�f�[�^���������0���f�[�^�𑗐M����
	if(logCount != 0) {
		if(type == 1) {
			Ope_Log_TargetDataVoidRead(eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		}
		else {
			Ope_Log_TargetDataVoidRead(eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, startFlg);
		}
		startFlg = FALSE;
		for(i=0; i<5; i++) {
			switch(i) {
			case 0:
				// �W�v��{�f�[�^(42)
// �d�l�ύX(S) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//				NTNET_Edit_SyukeiKihon_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_42 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_42);
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//				NTNET_Edit_SyukeiKihon_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_42_r13 *)RAU_NtLogData);
//				size = sizeof(DATA_KIND_42_r13);
				NTNET_Edit_SyukeiKihon_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_42 *)RAU_NtLogData);
				size = sizeof(DATA_KIND_42);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
				break;
			case 1:
				// ������ʖ��W�v�f�[�^(43)
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//				NTNET_Edit_SyukeiRyokinMai_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//				NTNET_Edit_SyukeiRyokinMai_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
				NTNET_Edit_SyukeiRyokinMai_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_43 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
				size = sizeof(DATA_KIND_43);
				break;
			case 2:
				// �����W�v�f�[�^(45)
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//				NTNET_Edit_SyukeiWaribiki_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//				NTNET_Edit_SyukeiWaribiki_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
				NTNET_Edit_SyukeiWaribiki_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_45 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
				size = sizeof(DATA_KIND_45);
				break;
			case 3:
				// ����W�v�f�[�^(46)
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//				NTNET_Edit_SyukeiTeiki_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//				NTNET_Edit_SyukeiTeiki_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
				NTNET_Edit_SyukeiTeiki_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_46 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
				size = sizeof(DATA_KIND_46);
				break;
			case 4:
				// �W�v�I���ʒm�f�[�^(53)
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//				NTNET_Edit_SyukeiSyuryo_r10((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//				NTNET_Edit_SyukeiSyuryo_r13((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
				NTNET_Edit_SyukeiSyuryo_rXX((SYUKEI *)RAU_LogData, type, (DATA_KIND_53 *)RAU_NtLogData);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
				size = sizeof(DATA_KIND_53);
				break;
			default:
				break;
			}

			RAU_NtLogDataSize = RAUhost_ConvertNtBlockData(RAU_NtLogData, size, RAU_NtBlockLogData, &blockCount);

			// tsum�̈揑����
			for(j=0; j<blockCount; j++){
				pPacketData = (t_RemoteNtNetBlk*)(RAU_NtBlockLogData + (j * 970));
				zerosize = pPacketData->header.packet_size[0] * 0x100;
				zerosize += pPacketData->header.packet_size[1];
				write_tsum((char*)pPacketData, zerosize);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ConvertNtBlockData()
 *[]----------------------------------------------------------------------[]*
 *|	�m�s�f�[�^1�u���b�N�쐬
 *[]----------------------------------------------------------------------[]*
 *|	patam	const uchar* pData	:
 *|			ushort size			:
 *|			uchar* pNtBlockData	:
 *[]----------------------------------------------------------------------[]*
 *|	return	1�u���b�N�ڂ̃T�C�Y
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	RAUhost_ConvertNtBlockData(const uchar* pData, ushort size, uchar* pNtBlockData, uchar *blockCount)
{
	DATA_BASIC*	pBasicData;							// ���MNT-NET�f�[�^
	t_RemoteNtNetBlk*	pPacketData;				// �p�P�b�g�f�[�^
	uchar*	pNtData;
	ushort	remainDataSize;							// �c��f�[�^�T�C�Y
	ushort	packetDataSize;							// �p�P�b�g�f�[�^�T�C�Y
	ushort	size0Cut;								// 0�J�b�g�����f�[�^�T�C�Y
	ushort	headerSize;								// �p�P�b�g�w�b�_�T�C�Y
	ushort	totalSize = 0;
	uchar	block = 0;								// �u���b�N
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	ushort	readNtdata = 0;
	
	remainDataSize = size - 3;
	
	pBasicData = (DATA_BASIC*)pData;
	
	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX;	// �w�b�_�T�C�Y
	
	while(remainDataSize)
	{
		pPacketData = (t_RemoteNtNetBlk*)(pNtBlockData + (block * 970));
		++block;												// �u���b�N�X�V(1�`)

		// 1�p�P�b�g�̃T�C�Y�����߂�
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//�f�[�^�𑗐M�o�b�t�@�Ɋi�[
		memcpy(pPacketData->data.data, pNtData + readNtdata, packetDataSize);
		readNtdata += packetDataSize;
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
		
		totalSize += size0Cut + 10;
	}
	*blockCount = block;
	return totalSize;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUhost_ConvertDataIdToLogData()
 *[]----------------------------------------------------------------------[]*
 *|	�m�s�f�[�^1�u���b�N�쐬
 *[]----------------------------------------------------------------------[]*
 *|	patam	none
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	S.Takahashi
 *| Date	:	2012-10-12
 *| Update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
short	RAUhost_ConvertDataIdToLogData(uchar dataId)
{
	short	logId = -1;

	switch(dataId){
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	case 20:											// ���Ƀf�[�^
//		logId = eLOG_ENTER;
//		break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	case 23:											// ���Z�f�[�^
		logId = eLOG_PAYMENT;
		break;
	case 120:											// �G���[�f�[�^
		logId = eLOG_ERROR;
		break;
	case 121:											// �A���[���f�[�^
		logId = eLOG_ALARM;
		break;
	case 122:											// ���j�^�f�[�^
		logId = eLOG_MONITOR;
		break;
	case 123:											// ���샂�j�^�f�[�^
		logId = eLOG_OPERATE;
		break;
	case 131:											// �R�C�����ɏW�v���v�f�[�^
		logId = eLOG_COINBOX;
		break;
	case 133:											// �������ɏW�v���v�f�[�^
		logId = eLOG_NOTEBOX;
		break;
	case 236:											// ���ԑ䐔�f�[�^
		logId = eLOG_PARKING;
		break;
	case 41:											// �s�W�v
		logId = eLOG_TTOTAL;
		break;
	case 53:											// �t�H�[�}�b�gRevNo.10 �s�^�f�s�W�v
		if(pRAUhost_SendIdInfo->pbuff_info->ui_syuType == 1) {
			logId = eLOG_TTOTAL;
		}
		else {
			logId = eLOG_GTTOTAL;
		}
		break;
	case 126:											// ���K�Ǘ�
		logId = eLOG_MONEYMANAGE;
		break;
	case 135:											// �ޑK�Ǘ��W�v�f�[�^
		logId = eLOG_MNYMNG_SRAM;
		break;
	case 125:											// ���u�Ď�
		logId = eLOG_REMOTE_MONITOR;
		break;
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	case 61:											// ��������(ParkingWeb�p)
		logId = eLOG_LONGPARK_PWEB;
		break;
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	default:
		break;
	}

	return logId;
}

//==============================================================================
//	�T�[�o�[����̃f�[�^��M
//
//	@argument	port	�Ώۃ|�[�g(UPLINE:�����, DOWNLINE:������)
//
//	@return		0		�f�[�^��M�Ȃ�
//	@return		1		�f�[�^��M����
//
//	@author		2013.07.13:MATSUSHITA
//======================================== Copyright(C) 2013 AMANO Corp. =======

/*
//  �����[�g�p�P�b�g
//	+0		�p�P�b�g�f�[�^��			(4)		�|�{�@�|�|�|�|�|�|�|�|�|�{
//	+4		"AMANO_REMOTE"				(12)	�@�b					�b
//	+16		�V�[�P���X��				(3)		�@�b					�b
//	+19		�f�[�^���R�[�h				(1)		�@�b�����[�g�w�b�_		�b�p�P�b�g�f�[�^��
//	+20		�d�����ʃR�[�h				(1)		�@�b (26 byte)			�b
//	+21		�����[�g�u���b�NNo.			(2)		�@�b					�b
//	+23		�����[�g�u���b�N�X�e�[�^�X	(1)		�@�b					�b
//	+24		�\��						(2)		�|�{					�b
//	+26		�����[�g�u���b�N�f�[�^		(1420/max)						�b
//	+??		CRC16						(2)		�|�|�|�|�|�|�|�|�|�|�|�|�{
//
//  �����f�[�^
//	+0		�p�P�b�g�f�[�^��			(4)		�|�{
//	+4		"AMANO_REMOTE"				(12)	�@�b
//	+16		�V�[�P���X��				(3)		�@�b
//	+19		�f�[�^���R�[�h				(1)		�@�b�����[�g�w�b�_
//	+20		�d�����ʃR�[�h				(1)		�@�b
//	+21		�����[�g�u���b�NNo.			(2)		�@�b
//	+23		�����[�g�u���b�N�X�e�[�^�X	(1)		�@�b
//	+24		�����X�e�[�^�X				(2)		�|�{
//	+26		ACK/NAK						(1)
//	+27		CRC16						(2)
*/
int		RAUhost_Receive(int port)
{
	uchar	*buff;
	ushort	bufflen, len, wlen;
	ulong	pklen;
	int		ret;
	int		ch;
	int		ofs;

// ��M�f�[�^�ǎ��
	ch = port - 1;					// UPLINE(1)�ADOWNLINE(2)�ACREDIT(3)��0�`2
	buff = RAU_huc_rcv_buf[ch];		// �|�[�g����M�o�b�t�@
	bufflen = RAU_huc_rcv_len[ch];	// �|�[�g����M�ς݃f�[�^��
	len = 0;						// �����M�f�[�^��
	wlen = 0;

	if (RAU_DpaRcvQue_Read(RAU_huc_rcv_tmp, &len, port) != 0) {
	// �f�[�^������΃|�[�g���̃o�b�t�@�ɓ]��
		wlen = RAU_RCV_MAX_H - bufflen;
		if (wlen > len) {
		// �󂫃G���A���[���ɂ���
			wlen = len;				// �S�ē]������
		}
		memcpy(&buff[bufflen], RAU_huc_rcv_tmp, wlen);
		bufflen += wlen;
		len -= wlen;				// �o�b�t�@�ۑ����������i���O�Ȃ�tmp�Ƀf�[�^���c���Ă���j
	}

// ��M�p�P�b�g�`�F�b�N
	ret = 0;
	while(bufflen > 4) {
	// �p�P�b�g�f�[�^���̎��̃f�[�^������
		if (buff[4] == uc_dpa_rem_str[0]) {
		// �V�O�j�`���擪����v
			if (bufflen < 16) {
				break;		// �V�O�j�`���S�̂�������Α҂�
			}
			if (memcmp(&buff[4], uc_dpa_rem_str, 12) == 0) {
			// ��M�p�P�b�g���o
				pklen = c_arraytoint32(buff, 4);				// �p�P�b�g���擾
				if (pklen > RAU_RCV_MAX_H) {
					switch (port) {
					case UPLINE:
						RAUhost_SetError(ERR_RAU_DPA_RECV_LONGER);
						break;
					case DOWNLINE:
						RAUhost_SetError(ERR_RAU_DPA_RECV_LONGER_D);
						break;
					case CREDIT:
						Credit_SetError(ERR_CREDIT_DPA_RECV_LONGER);
						break;
					default:
						break;
					}
					bufflen = 0;		// ���������������̂őS�f�[�^�j��
					len = 0;
				}
				else if (pklen <= bufflen) {
				// �S�f�[�^��M�ς�
					// �N���W�b�g���
					if (port == CREDIT) {
						memcpy(RAU_huc_rcv_work_CRE, buff, pklen);		// ��M�p�P�b�g����͗p���[�N�ɓ]��
						RAU_ui_data_length_CRE = pklen;					// �p�P�b�g�����Z�b�g
					}
					// ������A������
					else {
						memcpy(RAU_huc_rcv_work_h, buff, pklen);		// ��M�p�P�b�g����͗p���[�N�ɓ]��
						RAU_ui_data_length_h = pklen;					// �p�P�b�g�����Z�b�g
					}
					bufflen -= pklen;
					if (bufflen != 0) {
						memcpy(buff, &buff[pklen], bufflen);	// �c�肪����ΑO�ɂ߂�
					}
					ret = 1;
				}
				break;
			}
		// �V�O�j�`�����s��v
		}
	// �V�O�j�`��������Ȃ��̂ł��炵�Ă�蒼��
		for (ofs = 5; ofs < bufflen; ofs++) {
			if (buff[ofs] == uc_dpa_rem_str[0]) {
				break;
			}
		}
		ofs -= 4;
		bufflen -= ofs;
		memcpy(buff, &buff[ofs], bufflen);
	}
	if (len != 0) {
	// �c�]������Εۑ�
		memcpy(&buff[bufflen], &RAU_huc_rcv_tmp[wlen], len);		// ���̏ꍇ�I�[�o�[�t���[�͖����͂�
		bufflen += len;
	}
	RAU_huc_rcv_len[ch] = bufflen;
	return ret;
}

short RAUhost_CreateDopaPacketCTI(void)
{
	ushort		uiReadLen;
	ushort		uiRemain;
	uchar		ucReadEnd;
	ushort		PacketDataSize;		// �p�P�b�g�̃f�[�^���T�C�Y(�c���o��:�P�S�Q�O�o�C�g, �w�o������:�T�O�O�o�C�g)

	if( RauConfig.modem_exist == 0 ) {
		PacketDataSize = DOPA_DATA_SIZE;	// ���f������:�c���o��
	}
	else {
		PacketDataSize = XPORT_DATA_SIZE;	// ���f���Ȃ�:Ethernet
	}

	ucReadEnd = 0;
	RAU_ui_txdlength_h = 0;

	if (pRAUhost_SendIdInfo->send_data_count > 0) {

		// �f�[�^����
		while (ucReadEnd == 0) {

			// �m�s�f�[�^�̏����擾����
			if (RAUhost_GetNtDataCenterTermInfo() == FALSE) {
				// �f�[�^0���͑���Ȃ�
				return -1;
			}

			// �m�s�f�[�^�̎c�ʂ��Z�o
			uiRemain = RAUhost_SendNtDataInfo.nt_data_end - RAUhost_SendNtDataInfo.nt_data_next;

			// �m�s�f�[�^����ǂݏo���ʂ�����
			if ((PacketDataSize - RAU_ui_txdlength_h) < uiRemain) {
				uiReadLen = (PacketDataSize - RAU_ui_txdlength_h);
			} else {
				uiReadLen = uiRemain;
			}

			// �c�������p�P�b�g�ɂm�s�f�[�^���[�U
			memcpy(&RAU_huc_txdata_h[DOPA_HEAD_SIZE + RAU_ui_txdlength_h], RAUhost_SendNtDataInfo.nt_data_next, uiReadLen );

			// ���񏑂����݈ʒu���R�s�[�����������i�߂�
			RAUhost_SendNtDataInfo.nt_data_next += uiReadLen;

			// �ǂݍ��ݗʂ𑝉�
			RAU_ui_txdlength_h += uiReadLen;

			if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {
				// �P�m�s�f�[�^�̍Ō�܂ŏ������񂾂̂ŁAfill���C���N�������g
				pRAUhost_SendIdInfo->fill_up_data_count++;
			}

			// �������ݏI���`�F�b�N
			if (pRAUhost_SendIdInfo->send_data_count <= (pRAUhost_SendIdInfo->send_complete_count
														+ pRAUhost_SendIdInfo->fill_up_data_count
														+ pRAUhost_SendIdInfo->crc_err_data_count)) {
				// ����ȏ�f�[�^���Ȃ���΁A���M�
				ucReadEnd = 2;
			} else if (RAU_ui_txdlength_h >= PacketDataSize) {
				// �o�b�t�@�𖞂����Ă����瑗�M�
				ucReadEnd = 1;
			}

		}
	} else {
		// �f�[�^0���͑���Ȃ�
		return -1;
	}

	// �f�[�^���C���
	RAU_ui_txdlength_h = DOPA_HEAD_SIZE + RAU_ui_txdlength_h + 2;

	// ���f�v������
	if( ucReadEnd == 2 ){	// �Q�Ƃ��Ă���e�[�u��(�h�c)�̑S�f�[�^�̏[�U���I������B
		RAU_ucTcpBlockSts = 1;	// �s�b�o�u���b�N�X�e�[�^�X�P�i���I�[�j
	}

	// �w�b�_�[�ݒ�
	RAU_DpaSndHedSet(RAU_ucTcpBlockSts, UPLINE);

	// �b�q�b�Z�o
	RAU_CrcCcitt(RAU_ui_txdlength_h - 2, 0, RAU_huc_txdata_h, NULL, &RAU_huc_txdata_h[RAU_ui_txdlength_h - 2], 1 );

	return 0;
}

BOOL RAUhost_GetNtDataCenterTermInfo(void)
{
	BOOL ret;

	ret = FALSE;

	if (RAUhost_SendNtDataInfo.nt_data_start == NULL) {				// NULL��������f�[�^���擾����

		while (pRAUhost_SendIdInfo->send_data_count > pRAUhost_SendIdInfo->crc_err_data_count ) {

			if (RAUdata_GetCenterTermInfoData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
				ret = TRUE;
				break;
			} else {
				// �f�[�^��CRC�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
				pRAUhost_SendIdInfo->crc_err_data_count++;
			}

		}
		if (pRAUhost_SendIdInfo->send_data_count <= pRAUhost_SendIdInfo->crc_err_data_count) {
			// �S��CRC�G���[��������A�d�����Ȃ��̂ŉ�������Ȃ��B
			ret = FALSE;
		}
	} else {
		if (RAUhost_SendNtDataInfo.nt_data_end <= RAUhost_SendNtDataInfo.nt_data_next) {			// �f�[�^�̍Ō�܂œǂ�ł���

			while (pRAUhost_SendIdInfo->send_data_count > (pRAUhost_SendIdInfo->send_complete_count
															+ pRAUhost_SendIdInfo->fill_up_data_count
															+ pRAUhost_SendIdInfo->crc_err_data_count)) {

				if (RAUdata_GetCenterTermInfoData(&RAUhost_SendNtDataInfo, RAU_special_table_send_buffer) == TRUE) {
					ret = TRUE;
					break;
				} else {
					// �f�[�^��CRC�G���[�������̂Ŏ��̃f�[�^���擾���ɍs��
					pRAUhost_SendIdInfo->crc_err_data_count++;
				}
			}
		} else {
			ret = TRUE;
		}
	}
	return ret;
}
