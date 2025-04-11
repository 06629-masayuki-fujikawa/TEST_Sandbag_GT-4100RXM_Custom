#include    <trsocket.h>
#include    <trmacro.h>
#include    <trtype.h>
#include    <trproto.h>
#include    <trglobal.h>

#include 	"ksg_def.h"
#include	<machine.h>													/*							*/
#include	<string.h>													/*							*/
#include	"system.h"	
#include	"prm_tbl.h"	

int				KSG_gkasagoStarted;										/* KASAGO start flag				*/
int             KSG_gPpp_RauStarted;
ttUserInterface	gInterfaceHandle_Ether;									/* KASAGO Interface Handle			*/
ttUserInterface	gInterfaceHandle_PPP_rau;								/* KASAGO Interface Handle(Rau)		*/
ttUserLinkLayer	gLinkLayerHandle_Ether;									/* KASAGO Link Layer Handle			*/
ttUserLinkLayer	gLinkLayerHandle_PPP_rau;								/* KASAGO Link Layer Handle(Rau)	*/

unsigned long	KSG_Now_t;												/* 現在時刻(10ms単位)		*/
unsigned short	mSEC;													/* ﾐﾘ秒取得用				*/

static void tfDHCPInterfaceCB(	ttUserInterface		interfaceHandle,int	errorCode);

extern int EtherOpen(ttUserInterface interfaceHandle);
extern int EtherClose(ttUserInterface interfaceHandle);
extern int EtherSend(ttUserInterface interfaceHandle, char TM_FAR *dataPtr, int dataLength, int flag);
extern int EtherReceive(ttUserInterface interfaceHandle, char TM_FAR **dataPtr, int TM_FAR *dataLength, ttUserBufferPtr userBufferHandlePtr);
extern int EtherFreeReceiveBuffer(ttUserInterface interfaceHandle, char TM_FAR *dataPtr);
extern int EtherIoctl(ttUserInterface interfaceHandle, int flag, void TM_FAR *optionPtr, int optionLen);
extern int EtherGetPhyAddr(ttUserInterface interfaceHandle, char TM_FAR *physicalAddress);

/*
* Funtion Name  : KSG_Initialize
* Description   : KASAGO Initialize
* Parameters    : dhcp flag
* Returns       : 
*    _Normal                 0    : OK
*    _InitializeError       -1    : NG  ：Initialize
*    _LinkLayerError      	-2    :     ：Link Layer Initialize
*    _AddDriverError      	-3    :     ：driver addition
*    _SetIpAddressError   	-4    :     ：IP Address setting
*    _SetGatewayError     	-5    :     ：Gateway setting
*    _SetMtuError         	-6    :     ：MTU setting
*    _SetDnsError         	-7    :     ：DNS setting
*    _SetIpv6AddressError 	-8    :     ：IP Address setting(v6)
*    _SetIpv6DnsError     	-9    :     ：DNS setting(v6)
*    _SetMcastInterfaceError -10  :     ：Multicast Interface setting
*
*/
int KSG_Initialize(char v4Dhcp_flg)
{
    int errorCode;
	char ip[20];
	char subnet[20];
	char defgateway[20];

	KSG_gkasagoStarted = 0;												// KASAGO start flag
	KSG_gPpp_RauStarted = 0;
	gInterfaceHandle_Ether = 0;											// KASAGO Interface Handle
	gInterfaceHandle_PPP_rau = 0;										// KASAGO Interface Handle(Rau) 初期化
	gLinkLayerHandle_Ether = 0;											// KASAGO Link Layer Handle
	gLinkLayerHandle_PPP_rau = 0;										// KASAGO Link Layer Handle(Rau) 初期化

	memset( ip, 0, 20);
	memset( subnet, 0, 20);
	memset( defgateway, 0, 20);
	///------------------------------------------------------------------
	/// プロトコルスタックを初期化する
	///------------------------------------------------------------------
	errorCode = tfInitTreckOptions(TM_OPTION_TICK_LENGTH, 10);
	if( errorCode != TM_ENOERROR ){
		return( _InitializeError );
	}
	errorCode = tfStartTreck();
	if( errorCode != TM_ENOERROR ){
		return( _InitializeError );
	}
	KSG_gkasagoStarted = 1;

	///------------------------------------------------------------------
	/// LAN初期化					                                    
	///------------------------------------------------------------------
	gLinkLayerHandle_Ether = tfUseEthernet( );
	if( gLinkLayerHandle_Ether == 0 ){    
		return( _LinkLayerError );
	}

	///------------------------------------------------------------------
	/// LANドライバ追加                                                  
	///------------------------------------------------------------------
	gInterfaceHandle_Ether = tfAddInterface(
									"ether0",				 			///* Device Name
									gLinkLayerHandle_Ether,				///* Link Layer Handle
									EtherOpen,
									EtherClose,
									EtherSend,
									EtherReceive,
									EtherFreeReceiveBuffer,
									0,									// EtherIoctl,
									EtherGetPhyAddr,
									(int TM_FAR *)&errorCode			///* Error-Code Returning Pointer
							);
	if( ( gInterfaceHandle_Ether == 0 ) || ( errorCode != TM_ENOERROR ) ){
		return _AddDriverError;
	}

	// PPP-Rau クライアントを初期化 
	errorCode = KSG_initPppClient_rau(PPP_FIRSTINI);
	if( errorCode != TM_ENOERROR ){
		return( errorCode );
	}

	// DNS Initialize
	errorCode = tfDnsInit(TM_BLOCKING_OFF);	// ノンブロッキングモード
	if( errorCode != TM_ENOERROR ){
		return( errorCode );
	}
	///------------------------------------------------------------------
	/// ramfs Initialize                                                 
	///------------------------------------------------------------------
	errorCode = tfRamfsInit(RAMFS_BLOCKCOUNT);
	if( errorCode != TM_ENOERROR ){    
		return( errorCode );
	}

	if (v4Dhcp_flg == USE_DHCP_V4){
		///------------------------------------------------------------------
		/// DHCP Initialize                                                  
		///------------------------------------------------------------------
		errorCode = tfUseDhcp(gInterfaceHandle_Ether, tfDHCPInterfaceCB);
		if( errorCode != TM_ENOERROR ){    
			return( errorCode );
		}
		errorCode = tfOpenInterface (gInterfaceHandle_Ether, 0UL, 0UL, TM_DEV_IP_DHCP|TM_DEV_IP_FORW_ENB, 1);
	}else{
		sprintf(ip, "%03d.%03d.%03d.%03d",
					prm_get(COM_PRM,S_MDL,2,3,4),
					prm_get(COM_PRM,S_MDL,2,3,1),
					prm_get(COM_PRM,S_MDL,3,3,4),
					prm_get(COM_PRM,S_MDL,3,3,1));
		sprintf(subnet, "%03d.%03d.%03d.%03d",
					prm_get(COM_PRM,S_MDL,4,3,4),
					prm_get(COM_PRM,S_MDL,4,3,1),
					prm_get(COM_PRM,S_MDL,5,3,4),
					prm_get(COM_PRM,S_MDL,5,3,1));
		errorCode = tfOpenInterface (gInterfaceHandle_Ether,
									inet_addr(ip),
									inet_addr(subnet), TM_DEV_IP_FORW_ENB, 1);
		if( errorCode != TM_ENOERROR ){    
			return( errorCode );
		}

		// デフォルトゲートウェイ設定
		sprintf(defgateway, "%03d.%03d.%03d.%03d",
					prm_get(COM_PRM,S_MDL,6,3,4),
					prm_get(COM_PRM,S_MDL,6,3,1),
					prm_get(COM_PRM,S_MDL,7,3,4),
					prm_get(COM_PRM,S_MDL,7,3,1));
#ifdef TM_USE_STRONG_ESL
		errorCode = tfAddDefaultGateway(gInterfaceHandle_Ether, inet_addr(defgateway), 0);
#else
		errorCode = tfAddDefaultGateway(gInterfaceHandle_Ether, inet_addr(defgateway));
#endif /* TM_USE_STRONG_ESL */
		if (errorCode != TM_ENOERROR){
			return( errorCode );
		}
	}
	return errorCode;
}


/*
* Funtion Name  : tfDHCPInterfaceCB
* Description   : DHCP Interface Callback function
* Parameters    : ttUserInterface					interfaceHandle	:interface handle
*               : int								errorCode		:error code
* Returns       : 
*/
static void tfDHCPInterfaceCB(	ttUserInterface					interfaceHandle,
								int								errorCode			)
{
	if (errorCode == TM_ENOERROR){
		/*------------------------------------------------------------------*/
		/* ftp deamon start                                                 */
		/*------------------------------------------------------------------*/
	}
}
