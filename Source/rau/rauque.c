/****************************************************************************/
/*																			*/
/*		�V�X�e�����@:�@RAU�V�X�e��											*/
/*		�t�@�C����	:  rauque.c												*/
/*		�@�\		:  �L���[�֌W����										*/
/*																			*/
/****************************************************************************/

#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"rau.h"
#include	"rauIDproc.h"


/********************* FUNCTION DEFINE **************************/

void	RAUque_Init(void);
T_SEND_DATA_ID_INFO* RAUque_GetSendDataInfo( void );
void RAUque_UpDate(T_SEND_DATA_ID_INFO *pSendIdInfo);
BOOL RAUque_IsLastID( void );
void	RAUque_DeleteQue(void);

extern	void	RAUdata_GetNtDataInfoThrough(T_SEND_DATA_ID_INFO *pSendDataInfo);
extern	void	RAUdata_GetNtDataInfo(T_SEND_DATA_ID_INFO *pSendDataInfo, RAU_SEND_REQUEST *pReqInfo, uchar ucSendReq);
extern	void	RAUdata_DelNtData(RAU_DATA_TABLE_INFO *pBuffInfo, uchar cPosition);
extern	void	RAUdata_DeleteTsumGroup(ushort ucSendCount);
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
extern	int		count_tsum_data(struct _ntman *ntman, uchar count_tsum);

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
void RAUque_Init(void)
{
	memset( RAUque_SendDataInfo, 0, sizeof(RAUque_SendDataInfo) );
	RAUque_CurrentSendData = (ushort)-1;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	���M�Ώۃf�[�^�擾
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
T_SEND_DATA_ID_INFO* RAUque_GetSendDataInfo(void)
{
	ushort	i;


	for( i=RAUque_CurrentSendData+1; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){
		if( RAUque_SendDataInfo[i].send_req != 0 ){	// ���M�Ώ�
			RAUque_CurrentSendData = i;				// �Q�Ƃ��Ă���e�[�u���̔ԍ�(0:���Ƀf�[�^, 1:�o�Ƀf�[�^, 2:���Z�f�[�^�c)
			if(i == RAU_TOTAL_TABLE) {
				if( (RauConfig.id30_41_mask_flg == 0) &&				// �s���v�W�v�f�[�^�̑��M�}�X�N�Ȃ�
					(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026�C 0:���M���Ȃ� 1:���M����
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType = 1;
						RAUhost_CreateTsumData_r10(rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType);
					}
					else{
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
						rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType = 1;
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
						RAUhost_CreateTsumData();
					}
					RAUque_SendDataInfo[RAU_TOTAL_TABLE].send_data_count = count_tsum_data(&RAU_tsum_man.man, 0);
				}
				else {
					RAUque_SendDataInfo[RAU_TOTAL_TABLE].send_data_count = 0;
				}
			}
			else if(i == RAU_GTOTAL_TABLE) {
				if( (RauConfig.id30_41_mask_flg == 0) &&				// �s���v�W�v�f�[�^�̑��M�}�X�N�Ȃ�
					(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026�C 0:���M���Ȃ� 1:���M����
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						rau_data_table_info[RAU_GTOTAL_TABLE].ui_syuType = 11;
						RAUhost_CreateTsumData_r10(rau_data_table_info[RAU_GTOTAL_TABLE].ui_syuType);
					}
					else{
						RAUhost_CreateTsumData();
					}
					RAUque_SendDataInfo[RAU_GTOTAL_TABLE].send_data_count = count_tsum_data(&RAU_tsum_man.man, 0);
				}
				else {
					RAUque_SendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
				}
			}
			return &RAUque_SendDataInfo[i];			// �ΏۂɂȂ��Ă���e�[�u�����
		}
	}
	return (T_SEND_DATA_ID_INFO *)NULL;
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
BOOL RAUque_IsLastID(void)
{
	int	i;
	int	iLast;

	iLast = -1;

	for( i=RAUque_CurrentSendData; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){		// �X���[�f�[�^�e�[�u��(������)�͏��O
		if( RAUque_SendDataInfo[i].send_req != 0 ){	// ���ݎQ�Ƃ��Ă���e�[�u�������̑��M�\��e�[�u���𒲍�
			iLast = i;
		}
	}
	if( RAUque_CurrentSendData == iLast ){			// ���ݎQ�Ƃ��Ă���e�[�u�����Ō�
		return TRUE;
	} else {
		return FALSE;
	}
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
void RAUque_UpDate(T_SEND_DATA_ID_INFO *pSendIdInfo)
{
	// ���M�����f�[�^���𑝉�
	pSendIdInfo->send_complete_count += (pSendIdInfo->fill_up_data_count + pSendIdInfo->crc_err_data_count);

	// �[�U�����f�[�^���ECRC�G���[�f�[�^����0�N���A�
	pSendIdInfo->crc_err_data_count = 0;
	pSendIdInfo->fill_up_data_count = 0;
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
void RAUque_DeleteQue(void){
	// �ʐM�I�����@�i����I���@�ȏ�I���j�ɃL���[�ɓo�^����Ă���A���M���������f�[�^���폜����

	int i;
	int j;
	uchar	endDataId;

	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;							// �t�H�[�}�b�gRevNo.10�̏W�v�I���ʒm�f�[�^�̃f�[�^���
	}
	else {
		endDataId = 41;							// �t�H�[�}�b�gRevNo.10�ȊO�̏W�v�I���ʒm�f�[�^�̃f�[�^���
	}

	for( i=0; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){	// �X���[�f�[�^(������)�͏���
		if (RAUque_SendDataInfo[i].send_req != 0) {
			// ���M�ΏۂɂȂ��Ă�����B
			if (RAUque_SendDataInfo[i].pbuff_info->uc_DataId == endDataId) {
				// T���v�W�v�f�[�^�����ʂ̏�����������
				RAUdata_DeleteTsumGroup(RAUque_SendDataInfo[i].send_complete_count);
			} else {
				for (j = 0; j < RAUque_SendDataInfo[i].send_complete_count; j++) {
					// ���M�������������������
					RAUdata_DelNtData(RAUque_SendDataInfo[i].pbuff_info, 1);
				}
			}
			RAUque_SendDataInfo[i].pbuff_info->ui_SendDataCount = 0;
		}
	}

	// �X���[�f�[�^�̑��M���܂ŃN���A���Ă����̂Ńe�[�u���f�[�^�������N���A����
	memset(RAUque_SendDataInfo, 0, sizeof(T_SEND_DATA_ID_INFO)*RAU_SEND_THROUGH_DATA_TABLE);

}
