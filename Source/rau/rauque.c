/****************************************************************************/
/*																			*/
/*		システム名　:　RAUシステム											*/
/*		ファイル名	:  rauque.c												*/
/*		機能		:  キュー関係処理										*/
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
 *|	送信対象データ取得
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
		if( RAUque_SendDataInfo[i].send_req != 0 ){	// 送信対象
			RAUque_CurrentSendData = i;				// 参照しているテーブルの番号(0:入庫データ, 1:出庫データ, 2:精算データ…)
			if(i == RAU_TOTAL_TABLE) {
				if( (RauConfig.id30_41_mask_flg == 0) &&				// Ｔ合計集計データの送信マスクなし
					(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026④ 0:送信しない 1:送信する
					if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType = 1;
						RAUhost_CreateTsumData_r10(rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType);
					}
					else{
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
						rau_data_table_info[RAU_TOTAL_TABLE].ui_syuType = 1;
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
						RAUhost_CreateTsumData();
					}
					RAUque_SendDataInfo[RAU_TOTAL_TABLE].send_data_count = count_tsum_data(&RAU_tsum_man.man, 0);
				}
				else {
					RAUque_SendDataInfo[RAU_TOTAL_TABLE].send_data_count = 0;
				}
			}
			else if(i == RAU_GTOTAL_TABLE) {
				if( (RauConfig.id30_41_mask_flg == 0) &&				// Ｔ合計集計データの送信マスクなし
					(RauConfig.serverTypeSendTsum != 0) ){				// 34-0026④ 0:送信しない 1:送信する
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
			return &RAUque_SendDataInfo[i];			// 対象になっているテーブル情報
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

	for( i=RAUque_CurrentSendData; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){		// スルーデータテーブル(下り回線)は除外
		if( RAUque_SendDataInfo[i].send_req != 0 ){	// 現在参照しているテーブルより後ろの送信予定テーブルを調査
			iLast = i;
		}
	}
	if( RAUque_CurrentSendData == iLast ){			// 現在参照しているテーブルが最後
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
	// 送信完了データ数を増加
	pSendIdInfo->send_complete_count += (pSendIdInfo->fill_up_data_count + pSendIdInfo->crc_err_data_count);

	// 充填完了データ数・CRCエラーデータ数を0クリア｡
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
	// 通信終了時　（正常終了　以上終了）にキューに登録されている、送信完了したデータを削除する｡

	int i;
	int j;
	uchar	endDataId;

	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;							// フォーマットRevNo.10の集計終了通知データのデータ種別
	}
	else {
		endDataId = 41;							// フォーマットRevNo.10以外の集計終了通知データのデータ種別
	}

	for( i=0; i<RAU_SEND_THROUGH_DATA_TABLE; i++ ){	// スルーデータ(下り回線)は除く
		if (RAUque_SendDataInfo[i].send_req != 0) {
			// 送信対象になっていたら。
			if (RAUque_SendDataInfo[i].pbuff_info->uc_DataId == endDataId) {
				// T合計集計データだけ別の消し方をする｡
				RAUdata_DeleteTsumGroup(RAUque_SendDataInfo[i].send_complete_count);
			} else {
				for (j = 0; j < RAUque_SendDataInfo[i].send_complete_count; j++) {
					// 送信完了した分だけ消す｡
					RAUdata_DelNtData(RAUque_SendDataInfo[i].pbuff_info, 1);
				}
			}
			RAUque_SendDataInfo[i].pbuff_info->ui_SendDataCount = 0;
		}
	}

	// スルーデータの送信情報までクリアしていたのでテーブルデータ分だけクリアする
	memset(RAUque_SendDataInfo, 0, sizeof(T_SEND_DATA_ID_INFO)*RAU_SEND_THROUGH_DATA_TABLE);

}
