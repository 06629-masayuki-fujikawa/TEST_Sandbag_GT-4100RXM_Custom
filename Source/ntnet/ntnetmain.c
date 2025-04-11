// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A08(S) NTNET通信制御対応（標準UT4000：MH341111流用）
	// UT4000の "ntnetmain.c"をベースにFT4000の差分を移植して対応
// MH364300 GG119A08(E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *| System      : FT4000
 *| Module      : NT-NETメイン処理
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"mnt_def.h"
#include	"rau.h"
#include	"ntcom.h"


static	void	NTNET_SendMain(void);
static	void	NTNET_ReceiveMain(void);
static	void	NTNET_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl);
static	void	NTNET_EraseSendDataForce(t_Ntnet_SendDataCtrl* pCtrl);


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNETデータ処理メイン処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Main(void)
{
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
//// MH341110(S) A.Iiizumi 2017/11/14 新相互処理機能(GG107200)
//	Multi_lockctrl_timeout();
//// MH341110(E) A.Iiizumi 2017/11/14 新相互処理機能(GG107200)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
	NTNET_SendMain();									// 送信処理
	
	NTNET_ReceiveMain();								// 受信処理
	
	// 精算データバッファフルチェック
	if(LOG_DAT[eLOG_PAYMENT].writeFullFlag[eLOG_TARGET_NTNET] == 1) {	// バッファフルチェックが必要か？
		Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_NTNET);
		LOG_DAT[eLOG_PAYMENT].writeFullFlag[eLOG_TARGET_NTNET] = 0;		// バッファフルチェックフラグOFF
	}
	// Ｔ合計データバッファフルチェック
	if(LOG_DAT[eLOG_TTOTAL].writeFullFlag[eLOG_TARGET_NTNET] == 1) {	// バッファフルチェックが必要か？
		Log_CheckBufferFull(TRUE, eLOG_TTOTAL, eLOG_TARGET_NTNET);
		LOG_DAT[eLOG_TTOTAL].writeFullFlag[eLOG_TARGET_NTNET] = 0;		// バッファフルチェックフラグOFF
	}

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_SendMain
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNETデータ送信処理メイン処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_SendMain(void)
{
	ushort	size = 0;									// NTNETデータサイズ
	ushort	ret;
	t_Ntnet_SendDataCtrl* pCtrl = NULL;					// 送信制御データ
	
	// 優先データ取得
	pCtrl = &NTNET_SendCtrl[0];
	if(pCtrl->length == 0) {							// 再送待ちデータ無し
		size = NTBUF_GetSndNtData(pCtrl, NTNET_BUF_PRIOR);
		pCtrl->length = size;
	}
	else {												// 再送データあり
		size = pCtrl->length;
	}
	if(size == 0) {										// 優先データ無し
		// 通常データ取得
		pCtrl = &NTNET_SendCtrl[1];
		if(pCtrl->length == 0) {					// 再送待ちデータ無し
			size = NTBUF_GetSndNtData(pCtrl, NTNET_BUF_NORMAL);
			pCtrl->length = size;
		}
		else {										// 再送データあり
			size = pCtrl->length;
		}
	}

	if(NTCom_Condit) {									// NTComタスク起動状態
		if(size && pCtrl) {								// 送信データあり
			ret = NTCom_SetSendData(pCtrl->data, size, pCtrl->type);
			switch(ret) {
			case NTNET_NORMAL:							// NTNET通信部のバッファに格納
				NTNET_EraseSendData(pCtrl);				// 送信データを削除する
				break;
			case NTNET_ERR_BUFFERFULL:					// バッファフル
				// バッファフルの場合は何もしない
				break;
			case NTNET_ERR_BLOCKOVER:					// ブロックオーバー
			default:
				// arcnetを介さないため、ブロックオーバーの場合は送信したデータがおかしいので、
				// リトライせずにエラーとし削除する
				NTNET_EraseSendDataForce(pCtrl);		// 送信データ削除
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_ReceiveMain
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNETデータ受信処理メイン処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_ReceiveMain(void)
{
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データを削除する
 *|				ログの場合は送信済にする
 *|				送信バッファデータの場合は何もしない
 *| param	: pCtrl		: 削除する送信データの制御データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl)
{
	NTBUF_EraseSendData(pCtrl, FALSE);					// 送信データ削除
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendDataForce
 *[]----------------------------------------------------------------------[]
 *| summary	: T合計など複数データの送信途中でも送信データ削除する
 *|				ログの場合は送信済にする
 *|				送信バッファデータの場合は何もしない
 *| param	: pCtrl		: 削除する送信データの制御データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	NTNET_EraseSendDataForce(t_Ntnet_SendDataCtrl* pCtrl)
{
	NTBUF_EraseSendData(pCtrl, TRUE);					// 送信データ削除(強制)
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_SetReceiveData
 *[]----------------------------------------------------------------------[]
 *| summary	: 通信部の受信データ格納API
 *| param	: pNtData	: バッファに格納するNTNETデータ
 *| 		  size		: NTNETデータサイズ
 *| 		  type		: 格納先バッファ種別
 *| return	: TRUE - 送信WAIT中
 *[]----------------------------------------------------------------------[]*/
eNTNET_RESULT	NTNET_SetReceiveData(const uchar* pNtData, ushort size, uchar type)
{
	eNTNET_BUF_KIND kind;
	
	if(!pNtData || size == 0) {
		return NTNET_ERR_OTHER;
	}
	
	switch(type) {
	case NTNET_PRIOR_DATA:								// 優先データ
		kind = NTNET_BUF_PRIOR;
		break;
	case NTNET_NORMAL_DATA:								// 通常データ
		kind = NTNET_BUF_NORMAL;
		break;
	case NTNET_BROADCAST:								// 同報データ
		kind = NTNET_BUF_BROADCAST;
		break;
	default:
		return NTNET_ERR_OTHER;
	}
	
	NTBUF_SetRcvNtData((void*)pNtData, size, kind);		// 対応するバッファへ書き込む
	
	// バッファへ格納する／しないに関わらず正常終了
	return NTNET_NORMAL;
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
