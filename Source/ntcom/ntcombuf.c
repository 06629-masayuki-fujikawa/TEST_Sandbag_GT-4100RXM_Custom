// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/****************************************************************************/
/*																			*/
/*		システム名　:　NT-NET通信制御部										*/
/*		ファイル名	:  ntcombuf.c											*/
/*		機能		:  対OPE送受信処理										*/
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
 *| summary	: NT-NET→NTComの送信データ設定
 *| param	: pData		NT-NETデータ（伝送データの先頭）
 *|			  size		データサイズ
 *|			  type		データ種別(0:通常 1:優先)
 *| return	: NTNET_NORMAL			正常
 *|			  NTNET_ERR_BUFFERFULL	バッファフル
 *|			  NTNET_ERR_BLOCKOVER	NTNETブロックオーバー（優先、同報：1ブロック、通常、26ブロック以上のデータ）
 *|			  NTNET_ERR_OTHER		その他のエラー
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
eNTNET_RESULT	NTCom_SetSendData(const uchar* pData, ushort size, uchar type)
{
	eNTNET_RESULT	ret;

	if(NTComData_Ctrl.terminal_status != NT_ABNORMAL) {
		if(type == NTNET_NORMAL_DATA) {
			// 通常データ
			ret = NTCom_SetSendDataNormal(pData, size);
		}
		else if(type == NTNET_PRIOR_DATA) {
			// 優先データ
			ret = NTCom_SetSendDataPrior(pData, size);
		}
		else {
			// その他エラー
			ret =  NTNET_ERR_OTHER;
		}
	}
	else {
		// 通信異常中はバッファフルとして返す
		ret = NTNET_ERR_BUFFERFULL;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの送信データ取得
 *| retrun	: NULL以外	受信データ
 *|			  NULL		受信データなし
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort NTCom_GetSendData(uchar type, T_NT_BLKDATA* pRecvData)
{
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	if(type == NTNET_NORMAL_DATA) {
		// 通常データ
		if(NTComOpeRecvNormalDataBuf.count == 0) {
			return 0;								// 受信データなし
		}

		// 受信データポインタ取得
		memset(pRecvData, 0, sizeof(T_NT_BLKDATA));
		u.uc[0] = NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][5];
		u.uc[1] = NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][6];
		pRecvData->len = u.us;
		memcpy(pRecvData->data, &NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][0], u.us);

		// 読込みインデックス更新
		++NTComOpeRecvNormalDataBuf.readIndex;
		if(NTComOpeRecvNormalDataBuf.readIndex == NT_NORMAL_BLKNUM_MAX) {
			NTComOpeRecvNormalDataBuf.readIndex = 0;
		}
		--NTComOpeRecvNormalDataBuf.count;				// データ格納数減算
	}
	else if(type == NTNET_PRIOR_DATA) {
		// 優先データ
		if(NTComOpeRecvPriorDataBuf.count == 0) {
			return 0;								// 受信データなし
		}

		// 受信データポインタ取得
		memset(pRecvData, 0, sizeof(T_NT_BLKDATA));
		u.uc[0] = NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][5];
		u.uc[1] = NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][6];
		pRecvData->len = u.us;
		memcpy(pRecvData->data, &NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.readIndex][0], pRecvData->len);

		// 読込みインデックス更新
		++NTComOpeRecvPriorDataBuf.readIndex;
		if(NTComOpeRecvPriorDataBuf.readIndex == NT_PRIOR_BLKNUM_MAX) {
			NTComOpeRecvPriorDataBuf.readIndex = 0;
		}
		--NTComOpeRecvPriorDataBuf.count;				// データ格納数減算
	}
	else {
		return 0;
	}
	return (ushort)pRecvData->len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_CanSetSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの送信データ設定可能チェック
 *|			  type		データ種別(0:通常 1:優先)
 *| retrun	: TRUE		空きあり
 *|			  FALSE		空きなし(バッファFULL)
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	NTCom_CanSetSendData(uchar type)
{
	BOOL ret = FALSE;

	if(type == NTNET_NORMAL_DATA) {
		// 通常データ
		if(NTComOpeRecvNormalDataBuf.count < NT_NORMAL_BLKNUM_MAX) {
			ret = TRUE;
		}
	}
	else if(type == NTNET_PRIOR_DATA) {
		// 優先データ
		if(NTComOpeRecvPriorDataBuf.count < NT_PRIOR_BLKNUM_MAX) {
			ret = TRUE;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_SetSendDataNormal
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの通常送信データ設定
 *| param	: pData		NT-NETデータ（伝送データの先頭）
 *|			  size		データサイズ
 *| return	: NTNET_NORMAL			正常
 *|			  NTNET_ERR_BUFFERFULL	バッファフル
 *|			  NTNET_ERR_BLOCKOVER	NTNETブロックオーバー（優先、同報：1ブロック、通常、26ブロック以上のデータ）
 *|			  NTNET_ERR_OTHER		その他のエラー
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static eNTNET_RESULT	NTCom_SetSendDataNormal(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// 送信NT-NETデータ
	t_NtNetBlk*	pPacketData;						// パケットデータ
	uchar*	pNtData;
	ushort	remainDataSize;							// 残りデータサイズ
	ushort	packetDataSize;							// パケットデータサイズ
	ushort	size0Cut;								// 0カットしたデータサイズ
	ushort	headerSize;								// パケットヘッダサイズ
	ushort	writeBlock = 0;							// 今回各込みデータ数
	uchar	block = 0;								// ブロック
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
		// 送信中はバッファフルとする
		return NTNET_ERR_BUFFERFULL;
	}
	if(NTComOpeRecvNormalDataBuf.count >= 1) {
		// バッファフル
		return NTNET_ERR_BUFFERFULL;
	}

	pBasicData = (DATA_BASIC*)pData;

	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_NtNetBlkHeader) + 5;					// ヘッダサイズ

	while(remainDataSize)
	{
		memset(&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.writeIndex][0], 0, NT_BLKSIZE);
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.writeIndex][0];
		++block;												// ブロック更新(1～)

		// 1パケットのサイズを求める
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//データを送信バッファに格納
		memcpy(pPacketData->data.data, pNtData, packetDataSize);
		remainDataSize -= packetDataSize;						// 残りデータサイズ更新

		// 0カットサイズを求める
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}

		// パケットヘッダ部の作成
		memcpy(pPacketData->header.sig, NT_PKT_SIG, 5);			// "NTCOM"

		u.us = headerSize + size0Cut;							// データサイズ(ヘッダ+0カット後データ)
		pPacketData->header.len[0] = u.uc[0];
		pPacketData->header.len[1] = u.uc[1];

		pPacketData->header.vect = 0;							// 転送方法(下位→上位)

		pPacketData->header.terminal_no = NTCom_InitData.TerminalNum;	// 転送先端末No.

		pPacketData->header.lower_terminal_no1 = 0;				// 下位転送用端末No.1

		pPacketData->header.lower_terminal_no2 = 0;				// 下位転送用端末No.2

		pPacketData->header.link_no = 0;						// パケットシーケンスNo.

		pPacketData->header.status = 0;							// 端末ステータス(0:正常)

		pPacketData->header.broadcast_rcvd = 0;					// 同報受信済みフラグ

		u.us = packetDataSize;									// データサイズ(0カット前データ)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];

		pPacketData->header.mode = 0;							// パケット優先モード(0:通常データ)

		pPacketData->header.telegram_type = STX;				// STX

		pPacketData->header.res_detail = 0;						// 応答詳細(0:正常)

		// データヘッダー部の作成
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

		++NTComOpeRecvNormalDataBuf.writeIndex;						// 書込みインデックス更新
		if(NTComOpeRecvNormalDataBuf.writeIndex == NT_NORMAL_BLKNUM_MAX) {
			NTComOpeRecvNormalDataBuf.writeIndex = 0;
		}
		++NTComOpeRecvNormalDataBuf.count;								// データ格納数加算
		if(NTComOpeRecvNormalDataBuf.count > NT_NORMAL_BLKNUM_MAX) {		// 最古のデータを上書きしたか
			ret = NTNET_DEL_OLD;
			NTComOpeRecvNormalDataBuf.count = NT_NORMAL_BLKNUM_MAX;
		}
		if(pPacketData->data.is_lastblk == 0) {					// 中間データ
			pNtData += packetDataSize;
		}
	}
	NTComOpeRecvNormalDataBuf.fixed = TRUE;

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_SetSendDataPrior
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの優先送信データ設定
 *| param	: pData		NT-NETデータ（伝送データの先頭）
 *|			  size		データサイズ
 *| return	: NTNET_NORMAL			正常
 *|			  NTNET_ERR_BUFFERFULL	バッファフル
 *|			  NTNET_ERR_BLOCKOVER	NTNETブロックオーバー（優先、同報：1ブロック、通常、26ブロック以上のデータ）
 *|			  NTNET_ERR_OTHER		その他のエラー
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static eNTNET_RESULT	NTCom_SetSendDataPrior(const uchar* pData, ushort size)
{
	DATA_BASIC*	pBasicData;							// 送信NT-NETデータ
	t_NtNetBlk*	pPacketData;						// パケットデータ
	uchar*	pNtData;
	ushort	remainDataSize;							// 残りデータサイズ
	ushort	packetDataSize;							// パケットデータサイズ
	ushort	size0Cut;								// 0カットしたデータサイズ
	ushort	headerSize;								// パケットヘッダサイズ
	ushort	writeBlock = 0;							// 今回各込みデータ数
	uchar	block = 0;								// ブロック
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
		// 送信中はバッファフル
		return NTNET_ERR_BUFFERFULL;
	}
	if(NTComOpeRecvPriorDataBuf.count >= 1) {
		// バッファフル
		return NTNET_ERR_BUFFERFULL;
	}

	pBasicData = (DATA_BASIC*)pData;

	pNtData = (uchar*)pData + 3;
	headerSize = sizeof(t_NtNetBlkHeader) + 5;					// ヘッダサイズ

	while(remainDataSize)
	{
		memset(&NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.writeIndex][0], 0, NT_BLKSIZE);
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvPriorDataBuf.ntDataBuf[NTComOpeRecvPriorDataBuf.writeIndex][0];
		++block;												// ブロック更新(1～)

		// 1パケットのサイズを求める
		if(remainDataSize >= NTNET_BLKDATA_MAX) {
			packetDataSize = NTNET_BLKDATA_MAX;
		}
		else {
			packetDataSize = remainDataSize;
		}

		//データを送信バッファに格納
		memcpy(pPacketData->data.data, pNtData, packetDataSize);
		remainDataSize -= packetDataSize;						// 残りデータサイズ更新

		// 0カットサイズを求める
		size0Cut = packetDataSize;
		while(size0Cut) {
			if(pPacketData->data.data[size0Cut - 1] != 0) {
				break;
			}
			--size0Cut;
		}

		// パケットヘッダ部の作成
		memcpy(pPacketData->header.sig, NT_PKT_SIG, 5);			// "NTCOM"

		u.us = headerSize + size0Cut;							// データサイズ(ヘッダ+0カット後データ)
		pPacketData->header.len[0] = u.uc[0];
		pPacketData->header.len[1] = u.uc[1];

		pPacketData->header.vect = 0;							// 転送方法(下位→上位)

		pPacketData->header.terminal_no = NTCom_InitData.TerminalNum;	// 転送先端末No.

		pPacketData->header.lower_terminal_no1 = 0;				// 下位転送用端末No.1

		pPacketData->header.lower_terminal_no2 = 0;				// 下位転送用端末No.2

		pPacketData->header.link_no = 0;						// パケットシーケンスNo.

		pPacketData->header.status = 0;							// 端末ステータス(0:正常)

		pPacketData->header.broadcast_rcvd = 0;					// 同報受信済みフラグ

		u.us = packetDataSize;									// データサイズ(0カット前データ)
		pPacketData->header.len_before0cut[0] = u.uc[0];
		pPacketData->header.len_before0cut[1] = u.uc[1];

		pPacketData->header.mode = 1;							// パケット優先モード(1:優先データ)

		pPacketData->header.telegram_type = STX;				// STX

		pPacketData->header.res_detail = 0;						// 応答詳細(0:正常)

		// データヘッダー部の作成
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

		++NTComOpeRecvPriorDataBuf.writeIndex;						// 書込みインデックス更新
		if(NTComOpeRecvPriorDataBuf.writeIndex == NT_PRIOR_BLKNUM_MAX) {
			NTComOpeRecvPriorDataBuf.writeIndex = 0;
		}
		++NTComOpeRecvPriorDataBuf.count;								// データ格納数加算
		if(NTComOpeRecvPriorDataBuf.count > NT_PRIOR_BLKNUM_MAX) {		// 最古のデータを上書きしたか
			ret = NTNET_DEL_OLD;
			NTComOpeRecvPriorDataBuf.count = NT_PRIOR_BLKNUM_MAX;
		}
		if(pPacketData->data.is_lastblk == 0) {					// 中間データ
			pNtData += packetDataSize;
		}
	}
	NTComOpeRecvPriorDataBuf.fixed = TRUE;

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendPriorDataID
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの優先送信データバッファにあるデータ種別を取得する
 *| param	: none
 *| return	: データID	: 優先送信バッファに格納されているデータ種別
 *|			: 0			: 優先送信バッファにデータなし
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
uchar	NTCom_GetSendPriorDataID(void)
{
	t_NtNetBlk*	pPacketData;						// パケットデータ

	if( (NTComOpeRecvPriorDataBuf.count == 0) &&	// 優先データなし
		(NTComOpeRecvPriorDataBuf.fixed == FALSE) ){
		return 0;
	}

	if(NTComOpeRecvPriorDataBuf.count == 0) {
		pPacketData = (t_NtNetBlk*)&NTCom_SndBuf_Prior.blkdata->data[0];
	}
	else {
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvPriorDataBuf.ntDataBuf[0][0];
	}
	return pPacketData->data.data_type;				// データ種別を返す
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_GetSendNormalDataID
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET→NTComの通常送信データバッファにあるデータ種別を取得する
 *| param	: none
 *| return	: データID	: 通常送信バッファに格納されているデータ種別
 *|			: 0			: 通常送信バッファにデータなし
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
uchar	NTCom_GetSendNormalDataID(void)
{
	t_NtNetBlk*	pPacketData;						// パケットデータ

	if( (NTComOpeRecvNormalDataBuf.count == 0) &&	// 通常データなし
		(NTComOpeRecvNormalDataBuf.fixed == FALSE) ) {
		return 0;
	}

	if(NTComOpeRecvNormalDataBuf.count == 0) {
		pPacketData = (t_NtNetBlk*)&NTCom_SndBuf_Normal.blkdata->data[0];
	}
	else {
		pPacketData = (t_NtNetBlk*)&NTComOpeRecvNormalDataBuf.ntDataBuf[NTComOpeRecvNormalDataBuf.readIndex][0];
	}
	return pPacketData->data.data_type;				// データ種別を返す
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_ClearData
 *[]----------------------------------------------------------------------[]
 *| summary	: NTComの送受信データをクリアする
 *| param	: mode	0:RAMクリア 1:メンテナンスクリア
 *| return	: none
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void	NTCom_ClearData(uchar mode)
{
	unsigned char	initFlag = 0;

	if(4 == read_rotsw()) {
		initFlag = 1;
	}
	else if( memcmp( NTCom_memory_pass, NTCom_pass_string, 10 ) != 0 ){
		// プログラム切替後、初回のみ初期化を行う
		initFlag = 1;
	}
	else if(mode) {
		// メンテナンスからのクリア
		initFlag = 1;
	}

	if(mode) {
		NTComSci_Stop();								// シリアル送受信停止
	}

	NTCom_Init();

	// 対OPE送受信バッファクリア
	if(initFlag) {
		memcpy( NTCom_memory_pass, NTCom_pass_string, 10 );
		memset(&NTComOpeRecvNormalDataBuf, 0, sizeof(NTComOpeRecvNormalDataBuf));	// NTComタスク通常受信バッファ（対OPE層）
		memset(&NTComOpeRecvPriorDataBuf, 0, sizeof(NTComOpeRecvPriorDataBuf));		// NTComタスク優先受信バッファ（対OPE層）
		memset(&NTComOpeSendNormalDataBuf, 0, sizeof(NTComOpeSendNormalDataBuf));	// NTComタスク通常送信バッファ（対OPE層）
		memset(&NTComOpeSendPriorDataBuf, 0, sizeof(NTComOpeSendPriorDataBuf));		// NTComタスク優先送信バッファ（対OPE層）

		NTComData_Clear();
		NTComComm_Clear();
	}

	if(mode) {
		NTCom_FuncStart();
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
