/*==========================================================================*/
/*  Include                                                                 */
/*==========================================================================*/
/* KASAGO */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#include "ksg_def.h"
#include "..\ksg_rau\ksgRauTable.h"
#include	"system.h"	
#include	"prm_tbl.h"	
#include	"raudef.h"

/*==========================================================================*/
/*  Prototype                                                               */
/*==========================================================================*/
extern ttUserLinkLayer	gLinkLayerHandle_PPP;							/* KASAGO Link Layer Handle			*/
extern ttUserInterface	gInterfaceHandle_PPP_rau;						/* KASAGO Interface Handle			*/
extern int				KSG_gPpp_RauStarted;

ttUserIpAddress	KSG_gPPP_RauRemoteIpAddress;							// rau サーバーIP
ttUserIpAddress	KSG_gPPP_RauDNSIpAddressPri;							// rau DNSアドレス(プライマリ)
ttUserIpAddress	KSG_gPPP_RauDNSIpAddressSec;							// rau DNSアドレス(セカンダリ)
ttUserIpAddress	KSG_gPPP_RauOwnIpAddress;								// rau 自IP

extern void myPppNotifyFunction(ttUserInterface interfaceHandle,int flags);
/* Set PPP options of LCP, IPCP, PAP, CHAP and IPHC. */
static void setPppOptionLcpC( ttUserInterface interfaceHandle );
static void setPppOptionIpcpC( ttUserInterface interfaceHandle );

/****************************************************************************/
/*  Function:                                                               */
/*      KSG_initPppClient_rau(iniMode)                                      */
/*          iniMode=0(起動時の初期化)                                       */
/*                 =1(再接続時の初期化)                                     */
/*  Description:                                                            */
/*      KASAGO初期化処理( PPP client )                                      */
/****************************************************************************/
extern int KSG_initPppClient_rau( unsigned char iniMode )
{
	int		errorCode;

	if( iniMode == PPP_FIRSTINI){
		// PPP クライアントリンクレイヤを初期化
		gLinkLayerHandle_PPP_rau = tfUseAsyncPpp((ttUserLnkNotifyFuncPtr)myPppNotifyFunction );
		if( gLinkLayerHandle_PPP_rau  == NULL )
		{
			return TM_EINVAL;
		}

		// インタフェースドライバを追加する
		gInterfaceHandle_PPP_rau = tfAddInterface(
											"ppp0",                       	///* name of the device */
											gLinkLayerHandle_PPP_rau,     	///* Link Layer to use */
											ppp0DrvOpen,                    ///* Open Function */
											ppp0DrvClose,                   //// Close Function */
											ppp0DrvSend,                    ///* Send Function */
											ppp0DrvRecv,                    ///* Receive Function */
											ppp0DrvFreeReceiveBuffer,       ///* Free a Receive Buffer Function */
											ppp0DrvIoctl,                   ///* IOCTL Function */
											NULL,                         	///* Get Physical Address Function */
											&errorCode );
		if( ( gInterfaceHandle_PPP_rau == NULL ) || ( errorCode != TM_ENOERROR ) )
		{
			return errorCode;
		}
	}

	// LCPオプション設定
	setPppOptionLcpC( gInterfaceHandle_PPP_rau );

	// IPCPオプション設定
	setPppOptionIpcpC( gInterfaceHandle_PPP_rau );

	KSG_gPpp_RauStarted = 0;

	return errorCode;
}

/****************************************************************************/
/*  Function:                                                               */
/*      setPppOptionLcp( )                                                  */
/*                                                                          */
/*  Description:                                                            */
/*      LCPオプション設定                                                   */
/*                                                                          */
/****************************************************************************/
static void setPppOptionLcpC( ttUserInterface interfaceHandle )
{
//	int				errorCode;
	char			optionValue;
//	unsigned short	auth;			/* PAP/CHAP認証プロトコル */
	unsigned long	accm;			/* ACCM */
	unsigned short	mru;			/* 最大受信バイト数 */


	// プロトコルフィールド圧縮オプションセット
	optionValue = 1;
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_LCP_PROTOCOL,
					TM_PPP_OPT_WANT,
					TM_LCP_PROTOCOL_COMP,
					&optionValue,
					sizeof( optionValue ) );

	// ACCMオプションセット
	accm = 0xffffffff;
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_LCP_PROTOCOL,
					TM_PPP_OPT_ALLOW,
					TM_LCP_ACCM,
					(char *)&accm,
					sizeof( accm ) );

	// 最大受信単位オプションセット
	mru = 1500;
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_LCP_PROTOCOL,
					TM_PPP_OPT_WANT,
					TM_LCP_MAX_RECV_UNIT,
					(char *)&mru,
					sizeof( mru ) );

	return;
}

/****************************************************************************/
/*  Function:                                                               */
/*      setPppOptionIpcp( )                                                 */
/*                                                                          */
/*  Description:                                                            */
/*      IPCPオプション設定                                                  */
/*                                                                          */
/****************************************************************************/
static void setPppOptionIpcpC( ttUserInterface interfaceHandle )
{
	ttUserIpAddress	ip_addr;

	ip_addr = inet_addr("0.0.0.0");										// 全てのIPを許可
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_IPCP_PROTOCOL,
					TM_PPP_OPT_ALLOW,
					TM_IPCP_IP_ADDRESS,
					(char *)&ip_addr,
					sizeof( ip_addr ) );

	return;
}

/****************************************************************************/
/*  Function:                                                               */
/*      KSG_StartPPPSession( )                                              */
/*                                                                          */
/*  Description:                                                            */
/*      PPP開始処理                                                         */
/*                                                                          */
/****************************************************************************/
int KSG_StartPPPSession(ttUserInterface interfaceHandle)
{
	int    errorCode;
	ttUserIpAddress ip_addr;
	ttUserIpAddress subnet_addr;

	if( interfaceHandle == gInterfaceHandle_PPP_rau){
		if( KSG_RauConf.Dpa_IP_m.FULL != 0 ){
			ip_addr = htonl(KSG_RauConf.Dpa_IP_m.FULL);				// 自IPアドレス取得
			subnet_addr = htonl(KSG_RauConf.netmask.FULL);			// サブネット取得
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム切断後のPPP再接続の問題修正
// NOTE:PPP切断時にコールするtfCloseInterface()関数を呼び出すと、PPPのオプション設定は全て初期化
// される(KASAGO仕様マニュアル記載あり)そのため再接続時に行うtfOpenInterface()関数呼び出しの前に
// 必ずPPPのオプション設定tfPppSetOption()関数をコールして設定を行う必要がある。
// PPP切断時、KASAGOからイベント通知を受けるI/F myPppNotifyFunction()関数の中で
// TM_LL_CLOSE_COMPLETEイベントを受信できないことがある。
// 現状 TM_LL_CLOSE_COMPLETEイベント受信にて再接続時のPPPのオプション設定を行っている
// このケースでは再接続はできなくなる問題が発生するため
// 再接続時にPPPのオプション設定を再度行う
			KSG_initPppClient_rau(PPP_SECONDINI);
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム切断後のPPP再接続の問題修正
		}
	}
	SetUpNetState(NET_STA_PPP_OPENNING);
	SetDownNetState(NET_STA_PPP_OPENNING);

	errorCode = tfOpenInterface( interfaceHandle,
								ip_addr,
								subnet_addr,
								0,
								1) ;
	return errorCode;
}

/****************************************************************************/
/*  Function:                                                               */
/*      KSG_ClosePPPSession( )                                              */
/*                                                                          */
/*  Description:                                                            */
/*      PPP終了処理                                                         */
/*                                                                          */
/****************************************************************************/
int KSG_ClosePPPSession(ttUserInterface interfaceHandle)
{
	int    errorCode;
	errorCode = tfCloseInterface( interfaceHandle);
	KSG_gPpp_RauStarted = 0;
	KSG_PPP_RAU_state = KSG_PPP_STATE_CLOSE_START;
	SetUpNetState(NET_STA_PPP_CLOSING);
	SetDownNetState(NET_STA_PPP_CLOSING);

	return errorCode;
}

/****************************************************************************/
/*  Function:                                                               */
/*      KSG_GetInformation( )                                               */
/*                                                                          */
/*  Description:                                                            */
/*      IPアドレス等の取得                                                  */
/*                                                                          */
/****************************************************************************/
int KSG_GetInformation(ttUserInterface interfaceHandle,char ppp_Flag)
{
	int    errorCode;
	ttUserIpAddress*	ptempRemoteIpAddress;
	ttUserIpAddress*	ptempDNSIpAddressPri;
	ttUserIpAddress*	ptempDNSIpAddressSec;
	ttUserIpAddress*	ptempOwnIpAddress;

	if( ppp_Flag == PPP_FLAG_0){
		ptempRemoteIpAddress = &KSG_gPPP_RauRemoteIpAddress;
		ptempDNSIpAddressPri = &KSG_gPPP_RauDNSIpAddressPri;
		ptempDNSIpAddressSec = &KSG_gPPP_RauDNSIpAddressSec;
		ptempOwnIpAddress 	 = &KSG_gPPP_RauOwnIpAddress;
	}

	// 自IP取得
	if (tfGetIpAddress(interfaceHandle,ptempOwnIpAddress,0) == 0){
	}
	else{
		*ptempOwnIpAddress = 0;
	}
	// サーバーIP取得
	if (tfGetPppPeerIpAddress(interfaceHandle,ptempRemoteIpAddress) == 0){
	}
	else{
		*ptempRemoteIpAddress = 0;
	}
	// DNSサーバー（プライマリ）IP取得
	if (tfGetPppDnsIpAddress(interfaceHandle,ptempDNSIpAddressPri,TM_DNS_PRIMARY) == 0){
	}
	else{
		// 取得できないので削除
		*ptempDNSIpAddressPri = 0;
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_PRI_SERVER);
	}
	// DNSサーバー（セカンダリ）IP取得
	if (tfGetPppDnsIpAddress(interfaceHandle,ptempDNSIpAddressSec,TM_DNS_SECONDARY) == 0)
	{
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_SEC_SERVER);
		if( errorCode != TM_ENOERROR ){
		}
	}
	else{
		// 取得できないので削除
		*ptempDNSIpAddressSec = 0;
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_PRI_SERVER);
	}
	return errorCode;
}
