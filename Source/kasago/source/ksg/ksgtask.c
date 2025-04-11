/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGOタスク 																						   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgtask.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・KASAGOタスクメイン処理																				   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2011 AMANO Corp.----------[]*/

#include    <trsocket.h>
#include    <trmacro.h>
#include    <trtype.h>
#include    <trproto.h>
#include    <trglobal.h>

#include	"system.h"
#include	"message.h"
#include	"ksg_def.h"
#include	"ksgmac.h"
#include	"prm_tbl.h"
#include	"ope_def.h"
#include	"mem_def.h"
#include	"..\ksg_rau\ksgRauTable.h"
#include	"..\ksg_rau\ksgRauModemData.h"
#include	"raudef.h"

extern void				KSG_RauSetRauConf( void );
extern void				KSG_RauInitialTbl( void );
extern void				KSG_RauModemInit( void );
extern unsigned char	KSG_RauModemMain( void );
extern void				KSG_RauComdrMain( void );
extern void				KSG_RauComInit( void );
extern void				KSG_RauDeviceStart( void );

extern unsigned char	KSG_Rau_modem_online;

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| KASAGOタスク処理																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : ksgtask()																				   |*/
/*| PARAMETER    : void																						   |*/
/*| RETURN VALUE : void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author       :																							   |*/
/*| Date         : 2011-122-05																				   |*/
/*| Update       :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	ksgtask(void)
{
	long		KSG_RauEnable;

	KSG_RauInitialTbl();												// Rauデータテーブル初期化
	KSG_RauSetRauConf();												// Rau設定の反映

	KSG_Initialize(NOUSE_DHCP_V4);										// KASAGO初期化(設定取得後）

	KSG_RauComInit();													// Rau用SCI初期化

	KSG_RauModemInit();													// Rau用モデム初期化

	KSG_RauDeviceStart();												// Rauデバイス初期化処理

	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) == 2){							// Rau使用条件取得
		KSG_RauEnable = 1;												// NT-NET接続あり
		if(KSG_RauConf.Dpa_IP_m.FULL == 0 && 0 == prm_get(COM_PRM, S_CEN, 51, 1, 3)) {
			// モデム接続時に自IPが設定されていないと、リセットが発生するための対策
			// 自らがサーバとなる場合のみにkasaga内部でunknown vectorが発生する
			// クライアントの場合(FXのCappiやRism)は発生しない
			KSG_RauEnable = 0;											// NT-NET接続なし
		}
	}
	else if(RAU_Credit_Enabale != 0){
		KSG_RauEnable = 1;												// NT-NET接続あり
		if(KSG_RauConf.Dpa_IP_m.FULL == 0 && RAU_Credit_Enabale == 2) {
			// モデム接続時に自IPが設定されていないと、リセットが発生するための対策
			// 自らがサーバとなる場合のみにkasaga内部でunknown vectorが発生する
			// クライアントの場合(FXのCappiやRism)は発生しない
			KSG_RauEnable = 0;											// NT-NET接続なし
		}
	}
	else{
		KSG_RauEnable = 0;												// NT-NET接続なし
	}

	// メインループ
	while(1) {
	
		taskchg( IDLETSKNO );

        tfTimerExecute();												// KASAGOタイマ処理

		/* --- LAN ---------------------------------------------------------------------------*/
		if (tfCheckReceiveInterface(gInterfaceHandle_Ether) == 0){		// LAN受信チェック
			tfRecvInterface(gInterfaceHandle_Ether);					// LAN受信処理
		}

		if (tfCheckSentInterface(gInterfaceHandle_Ether) == 0){			// LAN送信チェック
			tfSendCompleteInterface(gInterfaceHandle_Ether, TM_DEV_SEND_COMPLETE_APP);	// LAN送信処理
		}

		if(KSG_RauEnable){
			if( KSG_RauConf.modem_exist ){
				// モデム無し(LAN)時はPPP接続なしでEthernet接続
				KSG_gPpp_RauStarted = 1;
				SetUpNetState(NET_STA_PPP_OPEN);
				SetDownNetState(NET_STA_PPP_OPEN);
			}
			else{
				KSG_RauComdrMain();										// Rau用SCIメイン処理

				// Rau用モデムメイン処理
				if(( KSG_RauModemMain() >= 4 ) && (KSG_RauModemOnline == 1 )){	// モデム接続中判定
					if(KSG_gPpp_RauStarted == 0 && 0 == RAU_GetAntennaLevelState()){
						// PPPが切れていたら再接続
						KSG_StartPPPSession(gInterfaceHandle_PPP_rau);	// Rau用FOMA PPP接続開始
					}
				}
				if (tfCheckSentInterface(gInterfaceHandle_PPP_rau) == 0){	// Rau用送信チェック
					// Rau用送信完了処理
					tfSendCompleteInterface(gInterfaceHandle_PPP_rau, TM_DEV_SEND_COMPLETE_APP);
				}
			}
		}
	}
}
