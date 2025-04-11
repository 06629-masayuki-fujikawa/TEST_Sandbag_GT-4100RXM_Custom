/****************************************************************************/
/*																			*/
/*		システム名　:　RAUシステム											*/
/*		ファイル名	:  raubuf.c												*/
/*		機能		:  対メイン送受信処理									*/
/*																			*/
/****************************************************************************/

#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"raudef.h"

RauNtDataBuf	RauOpeSendNtDataBuf;					// RAUタスク送信バッファ（対OPE層）
RauNtDataBuf	RauOpeRecvNtDataBuf;					// RAUタスク受信バッファ（対OPE層）
static	BOOL	RAU_IsTransactionData(uchar dataType);

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: RAUの受信バッファにNT-NETデータを格納する
 *| param	: pData		NT-NETデータ（伝送データの先頭）
 *|			  size		データサイズ
 *| retrun	: NTNET_BUFSET_NORMAL	正常
 *|			  NTNET_BUFSET_DEL_OLD	最古のデータを上書きした
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	RAU_SetSendNtData(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// 送信NT-NETデータ
	t_RemoteNtNetBlk*	pPacketData;				// パケットデータ
	uchar*	pNtData;
	ushort	remainDataSize;							// 残りデータサイズ
	ushort	packetDataSize;							// パケットデータサイズ
	ushort	size0Cut;								// 0カットしたデータサイズ
	ushort	headerSize;								// パケットヘッダサイズ
	ushort	writeBlock = 0;							// 今回各込みデータ数
	uchar	block = 0;								// ブロック
	uchar	ret = NTNET_BUFSET_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	uchar 	*dt;

	remainDataSize = size - 3;

	dt = (uchar *)pData;
	if (dt[0] != REMOTE_SYSTEM) {				//	SystemID != 30
		if(TRUE == RAU_IsTransactionData(dt[1])) {	// トランザクションデータはログに保存されるはず
			return ret;
		}
	}
	
	writeBlock = (size / NTNET_BLKDATA_MAX);
	if(size % NTNET_BLKDATA_MAX) {
		++writeBlock;
	}
	
	if(RauOpeRecvNtDataBuf.count >= RAU_NTDATABUF_COUNT) {
		// バッファフルのため古いデータを上書きする
		RauOpeRecvNtDataBuf.readIndex += writeBlock;
		if(RauOpeRecvNtDataBuf.readIndex >= RAU_NTDATABUF_COUNT) {
			RauOpeRecvNtDataBuf.readIndex -= RAU_NTDATABUF_COUNT;
		}
	}

	pBasicData = (DATA_BASIC*)pData;
	
	// 遠隔制御電文の場合、データ保持フラグの最上位ビットが自動/手動を示す
	if(pBasicData->DataKeep  & 0x80) {
		z_NtRemoteAutoManual = REMOTE_MANUAL;
	}
	else {
		z_NtRemoteAutoManual = REMOTE_AUTO;
	}
	pBasicData->DataKeep &= 0x7f;

	pNtData = (uchar*)pData + 3;

	headerSize = sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX;	// ヘッダサイズ
	
	while(remainDataSize)
	{
		memset(&RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.writeIndex][0], 0, RAU_BLKSIZE);
		pPacketData = (t_RemoteNtNetBlk*)&RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.writeIndex][0];
		++block;												// ブロック更新(1～)
		
		// 1パケットのサイズを求める
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//データを送信バッファに格納
		memcpy(&pPacketData->data.data, pNtData, packetDataSize);

		remainDataSize -= packetDataSize;						// 残りデータサイズ更新
		
		// 0カットサイズを求める
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}
		
		// パケットヘッダの作成
		u.us = headerSize + size0Cut;							// データサイズ(ヘッダ+0カット後データ)
		pPacketData->header.packet_size[0] = u.uc[0];
		pPacketData->header.packet_size[1] = u.uc[1];
		u.us = packetDataSize;									// データサイズ(0カット前データ)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];
		pPacketData->data.blk_no = block;						// ID1(ブロック番号)
		if(remainDataSize) {
			pPacketData->data.is_lastblk = 0;					// ID2(中間)
		}
		else {
			pPacketData->data.is_lastblk = 1;					// ID2(最終)
		}
		pPacketData->data.system_id = pBasicData->SystemID;		// ID3(システムID)
		pPacketData->data.data_type = pBasicData->DataKind;		// ID4(データID)
		pPacketData->data.save = pBasicData->DataKeep;			// データ保持フラグ
		
		++RauOpeRecvNtDataBuf.writeIndex;						// 書込みインデックス更新
		if(RauOpeRecvNtDataBuf.writeIndex == RAU_NTDATABUF_COUNT) {
			RauOpeRecvNtDataBuf.writeIndex = 0;
		}
		++RauOpeRecvNtDataBuf.count;								// データ格納数加算
		if(RauOpeRecvNtDataBuf.count > RAU_NTDATABUF_COUNT) {		// 最古のデータを上書きしたか
			ret = NTNET_BUFSET_DEL_OLD;
			RauOpeRecvNtDataBuf.count = RAU_NTDATABUF_COUNT;
		}
		if(pPacketData->data.is_lastblk == 0) {					// 中間データ
			pNtData += packetDataSize;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信バッファからNT-NETデータを取得する
 *| retrun	: NULL以外	受信データ
 *|			  NULL		受信データなし
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar*	RAU_GetSendNtData(void)
{
	uchar*	pRecvData;								// 受信データ
	
	if(RauOpeRecvNtDataBuf.count == 0) {
		return FALSE;								// 受信データなし
	}
	
	// 受信データポインタ取得
	pRecvData = &RauOpeRecvNtDataBuf.ntDataBuf[RauOpeRecvNtDataBuf.readIndex][0];

	// 読込みインデックス更新
	++RauOpeRecvNtDataBuf.readIndex;
	if(RauOpeRecvNtDataBuf.readIndex == RAU_NTDATABUF_COUNT) {
		RauOpeRecvNtDataBuf.readIndex = 0;
	}
	--RauOpeRecvNtDataBuf.count;						// データ格納数減算
	
	return pRecvData;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_IsTransactionData
 *[]----------------------------------------------------------------------[]
 *| summary	: ログに保存するデータかチェックする
 *| retrun	: TRUE		ログに保存するデータ
 *|			  NULL		ログに保存しないデータ（スルーデータ）
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAU_IsTransactionData(uchar dataType)
{
	BOOL	ret = FALSE;
	
	switch(dataType) {
	case 20:	// 入庫データ
	case 22:	// 
	case 23:	// 精算データ
	case 54:	// 入庫データ
	case 56:	// 精算データ（事前）
	case 57:	// 精算データ（出口）
	case 58:	// 駐車台数データ
	case 120:	// エラーデータ
	case 121:	// アラームデータ
	case 122:	// モニタデータ
	case 123:	// 操作モニタデータ
	case 131:	// コイン金庫集計データ
	case 133:	// 紙幣金庫集計データ
	case 135:	// 釣銭管理集計データ
	case 236:	// 駐車台数データ
	case 126:	// 金銭管理データ
	case 125:	// 遠隔監視データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	case 61:	// 長期駐車情報データ
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
		ret = TRUE;
		break;
	default:	// 集計データ
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
