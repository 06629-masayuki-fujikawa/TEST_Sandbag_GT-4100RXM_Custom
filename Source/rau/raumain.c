/************************************************************************************/
/*																					*/
/*		システム名　:　RAUシステム													*/
/*																					*/
/*		ファイル名	:  raumain.c													*/
/*		機能		:  メインループ処理												*/
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
uchar			RAU_CheckNearfullState = 1;			// 1:ニアフルチェックを行う
void	RAU_LogCheckState(void);

/****************************************************************/
/*	関数名		=	RAU_FuncMain								*/
/*	機能		=	常時起動部									*/
/*  メイン部をイニシャル起動部と 常時起動部に関数を分ける		*/
/*  本関数は RAUタスク起動毎に毎回Callされる(IDLE時)			*/
/****************************************************************/
void	RAU_FuncMain( void )
{
	switch(RAUmainState) {
	case 0:									// ニアフル状態チェック
		if(RAU_CheckNearfullState == 1) {	// 1:ニアフルチェックを行う
			RAUdata_CheckNearFullLogData();
			RAU_CheckNearfullState = 0;
		}
		++RAUmainState;
		break;
	case 1:
		// OPE層からの受信処理
		RAU_MainRecvProcess();
	
		// スルーデータ処理
		RAU_SetRevNtData();

		// HOST側 受信データマトリクス処理
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
 *| summary	: OPE層からの受信処理を行う
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_MainRecvProcess(void)
{
	t_RemoteNtNetBlk*	pRecvData;							// 受信データポインタ
	ushort	size;											// 受信データサイズ
	eRAU_TABLEDATA_SET result;								// 受信NT-NETブロック状態
	union {
		uchar	uc[2];
		ushort	us;
	} u;
	
	pRecvData = (t_RemoteNtNetBlk*)RAU_GetSendNtData();
	if(pRecvData) {											// 受信データあり
		u.uc[0] = pRecvData->header.packet_size[0];
		u.uc[1] = pRecvData->header.packet_size[1];
		size = u.us;
		result = (eRAU_TABLEDATA_SET)RAUid_SetNtBlockData(pRecvData, size);
		// 結果を比較する｡
		if (result == RAU_DATA_BLOCK_INVALID1) {
			// NT-NETブロックNo異常（今回無効）0x85
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
			RAU_err_chk(ERR_NTNET_NTBLKNO_VALID, 2, 0, 0, NULL);
		
		} else if (result == RAU_DATA_BLOCK_INVALID2) {
			// NT-NETブロックNo異常（今回有効）0x86
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
			RAU_err_chk(ERR_NTNET_NTBLKNO_INVALID, 2, 0, 0, NULL);
		
		} else if (result == RAU_DATA_CONNECTING_FROM_HOST) {
			// HOSTからのコネクション中によりデータ受信不可 0x90
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);
		
		} else if (result >= RAU_DATA_BUFFERFULL) {
			// 受信バッファフル　データ破棄による再送要求　0x82
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 1);

			// フルのバッファごとにエラーを送信。

		} else {
			// 上記の条件に一致しなかったら、多分正常｡
			
			// ARCブロック送受信結果　「正常」を送信｡
			NTNET_RAUResult_Send(pRecvData->data.system_id, pRecvData->data.data_type, 0);
		}
	}
	
	return;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_err_chk
 *[]----------------------------------------------------------------------[]
 *| summary	: モジュールIDを自動判別し、エラー登録を行う
 *[]----------------------------------------------------------------------[]*
 *|	param	: code		エラーコード
 *|			  kind		0=解除 1=発生 2=発生解除
 *|			  f_data	0=エラー情報無し 1=エラー情報有り(ascii) 2:(bin) 
 *|			  err_ctl	0:発生中チェックする 1:しない 
 *|			  pData		登録する付属データ
 *[]----------------------------------------------------------------------[]*
 *|	return	: void
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_err_chk(char code, char kind, char f_data, char err_ctl, void *pData)
{
	char	module;									// モジュールID
// MH322914 (s) kasiyama 2016/07/12 遠隔NT-NETLAN接続時のエラーコードをFOMA接続時のエラーコードに合わせる[共通バグNo.1200](MH341106)
//	char	errCode;
//
//	if(RauConfig.modem_exist == 0) {				// モデム
//		module = ERRMDL_FOMAIBK;
//		switch(code){
//		case ERR_RAU_DPA_BLK_VALID_D:				// エラーコード151(下り回線受信データブロック番号異常[今回有効])
//			errCode = ERR_RAU_FMA_BLK_VALID_D;
//			break;
//		case ERR_RAU_DPA_BLK_INVALID_D:				// エラーコード152(下り回線受信データブロック番号異常[今回無効])
//			errCode = ERR_RAU_FMA_BLK_INVALID_D;
//			break;
//		case ERR_RAU_DPA_RECV_CRC_D:				// エラーコード154(下り回線受信データＣＲＣエラー)
//			errCode = ERR_RAU_FMA_RECV_CRC_D;
//			break;
//		case ERR_RAU_DPA_RECV_LONGER_D:				// エラーコード155(下り回線受信データ長異常)
//			errCode = ERR_RAU_FMA_RECV_LONGER_D;
//			break;
//		case ERR_RAU_DPA_RECV_SHORTER_D:			// エラーコード156(下り回線受信データ長異常)
//			errCode = ERR_RAU_FMA_RECV_SHORTER_D;
//			break;
//		case ERR_RAU_DPA_SEND_LEN_D:				// エラーコード192(下り回線送信データ長オーバー)
//			errCode = ERR_RAU_FMA_SEND_LEN_D;
//			break;
//		case ERR_RAU_DPA_RECV_LEN_D:				// エラーコード193(下り回線受信データ長オーバー)
//			errCode = ERR_RAU_FMA_RECV_LEN_D;
//			break;
//		case ERR_RAU_DPA_SEND_RETRY_OVER_D:			// エラーコード195(下り回線データ再送回数オーバー)
//			errCode = ERR_RAU_FMA_SEND_RETRY_OVER_D;
//			break;
//		case ERR_RAU_DPA_RECV_NAK99_D:				// エラーコード197(下り回線送信データ強制停止受信)
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
	if(RauConfig.modem_exist == 0) {				// モデム
		module = ERRMDL_FOMAIBK;
	}
	else {											// Ethernet
		module = ERRMDL_LANIBK;
	}
	err_chk2(module, code, kind, f_data, err_ctl, pData);
// MH322914 (e) kasiyama 2016/07/12 遠隔NT-NETLAN接続時のエラーコードをFOMA接続時のエラーコードに合わせる[共通バグNo.1200](MH341106)
}

/*[]----------------------------------------------------------------------[]*
 *|     RAU_SetRevNtData
 *[]----------------------------------------------------------------------[]*
 *|	送信可能なNT-NETデータの有無をチェック
 *[]----------------------------------------------------------------------[]*
 *|	param	: none
 *[]----------------------------------------------------------------------[]*
 *|	return	: TRUE =送信可能なNT-NETデータ有り
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
	
	// 送信途中の電文があるかチェック
	if (RAUarc_SndNtBlk.ntblknum == RAUARC_NODATA) {
		RAUarc_SndNtBlk.ntblknum = RAUdata_GetRcvNtDataBlkNum();
		if (RAUarc_SndNtBlk.ntblknum <= 0) {
			RAUarc_SndNtBlk.ntblknum = RAUARC_NODATA;
			return FALSE;
		}
		RAUarc_SndNtBlk.snd_ntblknum = 0;
		RAUarc_SndNtBlk.ntblkno_retry = 0;

	}
	
	/* 送信ブロックデータを取得する */
	
	index = 0;
	headerLen = (ushort)(sizeof(t_RemoteNtNetBlk) - NTNET_BLKDATA_MAX);
	memset(&RecvNtnetDt, 0x00, sizeof(RECV_NTNET_DT));
	pNtRecv = (uchar*)&RecvNtnetDt;
	while(RAUarc_SndNtBlk.ntblknum != RAUARC_NODATA) {
		memset(RAU_TempRecvNtData, 0, sizeof(RAU_TempRecvNtData));
		
		RAUarc_SndNtBlk.ntblk.len = RAUdata_GetRcvNtBlkData(RAUarc_SndNtBlk.ntblk.data, RAUarc_SndNtBlk.snd_ntblknum+1);
		--RAUarc_SndNtBlk.ntblknum;
		//NT-NET受信バッファに格納
		pRemoteData = (t_RemoteNtNetBlk*)RAUarc_SndNtBlk.ntblk.data;
		u.uc[0] = pRemoteData->header.len_before0cut[0];
		u.uc[1] = pRemoteData->header.len_before0cut[1];
		if(pRemoteData->data.blk_no == 1) {				// 先頭ブロック
			// ブロックNo.が1ならばNTNETデータの先頭部分を作成する
			memcpy(pNtRecv, &pRemoteData->data.system_id, 3);	// システムID～データ保持フラグまでコピー
			index += 3;
			memcpy(RAU_TempRecvNtData, &pRemoteData->data.data, RAUarc_SndNtBlk.ntblk.len - headerLen);
		}
		else {
			if(RAUarc_SndNtBlk.ntblk.len > headerLen) {	// 全て0カットされていない
				memcpy(RAU_TempRecvNtData, &pRemoteData->data.data, RAUarc_SndNtBlk.ntblk.len - headerLen);
			}
		}
		memcpy(pNtRecv + index, RAU_TempRecvNtData, u.us);
		index += NTNET_BLKDATA_MAX;					// 次の書込みインデックス
		++RAUarc_SndNtBlk.snd_ntblknum;
		
		if(RAUarc_SndNtBlk.ntblknum == 0) {
			RAUdata_DelRcvNtData();						// 全て処理したのでデータ削除
			NTNET_CtrlRecvData();						// 受信データの解析と処理を行う
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
	short	Lno;									// ログID
	BOOL	strage;									// FROM/SRAM
	uchar	nearfullCheck = 0;						// ニアフルチェックフラグ
	
	for(Lno = eLOG_PAYMENT; Lno < eLOG_MAX; ++Lno) {
		if(LOG_DAT[Lno].writeLogFlag[eLOG_TARGET_REMOTE] == 1) {		// ログ書込みあり

			strage = (Lno < eLOG_COINBOX) ? 1 : 0;	// FROM or SRAM
			// ニアフルチェック
			if( strage == 0){// RAMのみで管理するログの場合
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

			if(nearfullCheck == 0 && RAUdata_CanSendTableData(Lno)) {	// ニアフルチェック対象か？
				nearfullCheck = 1;										// ニアフルチェックフラグON
			}
			
			if(LOG_DAT[Lno].writeFullFlag[eLOG_TARGET_REMOTE] == 1) {	// バッファフルチェックが必要か？
				Log_CheckBufferFull(TRUE, Lno, eLOG_TARGET_REMOTE);
			}
			LOG_DAT[Lno].writeFullFlag[eLOG_TARGET_REMOTE] = 0;			// データフルフラグON
		}
		LOG_DAT[Lno].writeLogFlag[eLOG_TARGET_REMOTE] = 0;				// ログ書込みフラグクリア
	}
	
	// ニアフルチェックを要求する
	if(nearfullCheck) {
		RAU_RequestCheckNearfull();
	}

}
/*[]----------------------------------------------------------------------[]*
 *|     RAU_GetCenterSeqNo
 *[]----------------------------------------------------------------------[]*
 *|	センター追番を取得する
 *[]----------------------------------------------------------------------[]*
 *|	param	: type	: 追番種別
 *[]----------------------------------------------------------------------[]*
 *|	return	: 種別に対応するセンター追番
 *|			: 0		: 追番種別不正
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ulong	RAU_GetCenterSeqNo(RAU_SEQNO_TYPE type)
{
	if(type < RAU_SEQNO_TYPE_COUNT) {			// 追番種別が範囲内か
		return Rau_SeqNo[type];
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*
 *|     RAU_UpdateCenterSeqNo
 *[]----------------------------------------------------------------------[]*
 *|	センター追番を更新する
 *[]----------------------------------------------------------------------[]*
 *|	param	: type	: 追番種別
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_UpdateCenterSeqNo(RAU_SEQNO_TYPE type)
{
	if(type < RAU_SEQNO_TYPE_COUNT) {			// 追番種別が範囲内か
		++Rau_SeqNo[type];
		if(Rau_SeqNo[type] == 0) {
			Rau_SeqNo[type] = 1;				// センター追番は1～0xffffffffまで
		}
	}
}

// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
//[]----------------------------------------------------------------------[]
//|			データ再送待ちタイマーキャンセル
//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
void	RAU_Cancel_RetryTimer()
{
	RAU_Tm_data_rty.tm = 0;
	RAU_f_data_rty_ov = 1;
}
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正