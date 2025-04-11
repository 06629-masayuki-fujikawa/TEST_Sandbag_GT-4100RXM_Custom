// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/************************************************************************************/
/*																					*/
/*		システム名　:　NT-NET通信部													*/
/*																					*/
/*		ファイル名	:  ntmain.c														*/
/*		機能		:  メインループ処理												*/
/*																					*/
/************************************************************************************/
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/********************* FUNCTION DEFINE **************************/
static BOOL	NTCom_MainRecvProcess(void);
static BOOL	NTCom_NtnetRecvProcess(void);
static BOOL	NTCom_IsBroadcastBlock(void);
static BOOL	NTCom_IsPriorBlock(void);
static BOOL	NTCom_IsNormalBlock(void);


/*********************** EXTERN DATA ****************************/
T_NT_BLKDATA	NTComRecvData;								// 受信データポインタ


/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_FuncStart() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		NTCom function start. 											   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	This routine will be called after Initial setting data receiveing.	   |*/
/*| same mean as initialize process.									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTCom_FuncStart( void )
{
	/* NT-NET電文管理機能初期化 */
	NTComData_Start();
	/* タイマー初期化 */
	NTComTimer_Init();
	/* SCIドライバ初期化 */
	NTComComdr_Init(NTCom_InitData.Time_t5, NTCom_SciPort, NTCom_InitData.Baudrate,
				NT_SCI_DATABIT, NT_SCI_STOPBIT, NT_SCI_PARITY);
	/* NT-NETプロトコル層初期化 */
	NTComComm_Start();

	NTCom_Condit = 1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_FuncMain()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|		credit function Main routine									   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	This routine will be called evetytime task active in IDLE phase.       |*/
/*| (Called only Main task phase is IDLE)								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTCom_FuncMain( void )
{
	// タイマーチェック処理
	NTCom_TimoutChk();

	/* NT-NET処理 */
	NTComComm_Main();

	// OPE層からの受信処理
	if(NTCom_MainRecvProcess()) {
		return;
	}

	// NT-NETからの受信処理
	if(NTCom_NtnetRecvProcess()) {
		return;
	}

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_MainRecvProcess
 *[]----------------------------------------------------------------------[]
 *| summary	: OPE層からの受信処理を行う
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static BOOL	NTCom_MainRecvProcess(void)
{
	eNT_DATASET		result = NT_DATA_NORMAL;
	ushort			size;

	if(NTCom_SndBuf_Prior.fixed == FALSE) {
		size = NTCom_GetSendData(NTNET_PRIOR_DATA, &NTComRecvData);
		if(size) {											// 優先受信データあり
			result = NTComData_SetSndBlock(&NTComRecvData);
			if(result == NT_DATA_NORMAL) {
				return TRUE;
			}
		}
	}
	if(NTCom_SndBuf_Normal.fixed == FALSE) {
		size = NTCom_GetSendData(NTNET_NORMAL_DATA, &NTComRecvData);
		if(size) {											// 通常受信データあり
			result = NTComData_SetSndBlock(&NTComRecvData);
			if(result == NT_DATA_NORMAL) {
				return TRUE;
			}
		}
	}
	if (result == NT_DATA_NO_MORE_TELEGRAM) {
		/* ブロックNo.正常(但しバッファに未送信の電文があり保存できない)を通知 */
	} else if (result == NT_DATA_BLOCK_INVALID1) {
		/* NT-NETブロックNo.異常によりデータ破棄（今回無効） */
		NTCom_err_chk(NT_ERR_BLK_ARC1, 2, 0, 0, NULL);
	} else if (result == NT_DATA_BLOCK_INVALID2) {
		/* NT-NETブロックNo.異常によりデータ破棄（今回有効） */
		NTCom_err_chk(NT_ERR_BLK_ARC2, 2, 0, 0, NULL);
	}
	else if (result == NT_DATA_BUFFERFULL) {
		/* バッファ許容量分のNT-NETブロックを受信したが最終ブロックフラグが見つからない */
		NTCom_err_chk(NT_ERR_BLK_OVER_ARC, 2, 0, 0, NULL);
	}

	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_NtnetRecvProcess
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETからの受信処理を行う
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static BOOL	NTCom_NtnetRecvProcess(void)
{
	eNTNET_RESULT	result;
	ushort			HeaderToID3 = sizeof(t_NtNetBlkHeader) + 2;

	/* NT-NETから受信したブロックデータがあればOPEに通知 */
	if (NTCom_IsBroadcastBlock()) {
		// 同報データあり
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len + 3, NTNET_BROADCAST);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Broadcast.telegram);
		}
		return TRUE;
	}
	if (NTCom_IsPriorBlock()) {
		// 優先データあり
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len + 3, NTNET_PRIOR_DATA);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Prior.telegram);
		}
		return TRUE;
	}
	if (NTCom_IsNormalBlock()) {
		// 通常データあり
//		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[HeaderToID3], NTCom_SndWork.len - 2, NTNET_NORMAL_DATA);
		result = NTNET_SetReceiveData(&NTCom_SndWork.sendData[0], NTCom_SndWork.len, NTNET_NORMAL_DATA);
		if(result == NTNET_NORMAL) {
			NTComData_DeleteTelegram(&NTCom_Tele_Normal.telegram);
		}
		return TRUE;
	}

	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_err_chk
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
void	NTCom_err_chk(char code, char kind, char f_data, char err_ctl, void *pData)
{
	ushort	errKind;

	switch (code) {
	case NT_ERR_BLK_ARC1:
	case NT_ERR_BLK_ARC2:
	case NT_ERR_BLK_OVER_ARC:
		errKind = ERRMDL_NTNET;
		break;
	default:
		errKind = ERRMDL_NTNETIBK;
		break;
	}

	switch (kind) {
	case 1:													/* 発生 */
		err_chk2(errKind, (char)code, 1, 0, 0, pData);
		break;
	case 2:													/* 発生＆解除 */
		err_chk2(errKind, (char)code, 2, 0, 0, pData);
		break;
	default:	/* case NTERR_RELEASE: */					/* 解除 */
		err_chk2(errKind, (char)code, 0, 0, 0, pData);
		break;
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsBroadcastBlock() 	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPEに通知するNT-NET同報データブロック電文があるかチェック 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsBroadcastBlock(void)
{
	/* 送信途中の電文があるかチェック */
	if (!NTComData_IsValidTelegram(&NTCom_Tele_Broadcast.telegram)) {
	/* なければ、送信可能な電文があるかチェック */
		if (!NTComData_PeekRcvTele_Broadcast(&NTCom_Tele_Broadcast.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Broadcast.ntblkno_retry = 0;		/* #005 */
	}
	/* 送信ブロックデータを取得する */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Broadcast.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Broadcast.telegram);
		return FALSE;			/* データ取得エラー(このパスは通らないはず) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsPriorBlock()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPEに通知するNT-NET優先データブロック電文があるかチェック 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsPriorBlock(void)
{
	/* 送信途中の電文があるかチェック */
	if (!NTComData_IsValidTelegram(&NTCom_Tele_Prior.telegram)) {
	/* なければ、送信可能な電文があるかチェック */
		if (!NTComData_PeekRcvTele_Prior(&NTCom_Tele_Prior.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Prior.ntblkno_retry = 0;		/* #005 */
	}
	/* 送信ブロックデータを取得する */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Prior.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Prior.telegram);
		return FALSE;			/* データ取得エラー(このパスは通らないはず) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_IsNormalBlock() 	 	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		OPEに通知するNT-NET通常データブロック電文があるかチェック 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-06-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static BOOL	NTCom_IsNormalBlock(void)
{

	if (!NTComData_IsValidTelegram(&NTCom_Tele_Normal.telegram)) {
	/* なければ、送信可能な電文があるかチェック */
		if (!NTComData_PeekRcvTele_Normal(&NTCom_Tele_Normal.telegram)) {
			return FALSE;
		}
		NTCom_Tele_Normal.ntblkno_retry = 0;		/* #005 */
	}
	/* 送信ブロックデータを取得する */
	if (NTComData_GetRecvBlock(&NTCom_Tele_Normal.telegram, &NTCom_SndWork) < 0) {
		NTComData_ResetTelegram(&NTCom_Tele_Normal.telegram);
		return FALSE;			/* データ取得エラー(このパスは通らないはず) */
	}

	memset(&NTComOpeSendNormalDataBuf, 0, sizeof(NTComOpeSendNormalDataBuf));

	return TRUE;
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
