#ifndef _KASAGO_DEF_H_
#define _KASAGO_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#include 	"nonfilesys.h"

#define USE_DHCP_V4				1
#define NOUSE_DHCP_V4			2

#define PPP_FLAG_0				0
#define PPP_FLAG_1				1

#define PPP_FIRSTINI			0		// 起動時の初期化
#define PPP_SECONDINI			1		// 再接続時の初期化

#define MAX_USERS				5
#define MAX_CHARS				16

#define FTPD_MAX_CONN			5		// 同時に受け入れることができるFTP接続
#define FTPD_MAX_BACK			2		// FTPサーバソケットで許可する接続保留中キューの最大数
#define FTPD_MAX_IDLETIMEOUT	300
#define RAMFS_BLOCKCOUNT		10

enum {
	ROUTER_OFF = 0,
	ROUTER_INIT,
	ROUTER_IDLE
};

#define ZERO_ADDR_V4			"0.0.0.0"

#define _Normal              	0     //: OK
#define _InitializeError     	-1    //: NG  ：Initialize
#define _LinkLayerError      	-2    //:     ：Link Layer Initialize
#define _AddDriverError      	-3    //:     ：driver addition
#define _SetIpAddressError   	-4    //:     ：IP Address setting
#define _SetGatewayError     	-5    //:     ：Gateway setting
#define _SetMtuError         	-6    //:     ：MTU setting
#define _SetDnsError         	-7    //:     ：DNS setting
#define _SetIpv6AddressError 	-8    //:     ：IP Address setting(v6)
#define _SetIpv6DnsError     	-9    //:     ：DNS setting(v6)
#define _SetMcastInterfaceError -10   //:     ：Multicast Interface setting

#define	KSG_PPP_STATE_CLOSE				0
#define	KSG_PPP_STATE_OPEN_STATRT		1
#define	KSG_PPP_STATE_OPEN				2
#define	KSG_PPP_STATE_CLOSE_START		3


extern int KSG_Initialize(char v4Dhcp_flg);

/* FTP 関連 */
extern int tfRamfsInit(ttRamfsBlock blCount);

/* ether 関連 */
extern int  ether24J600DrvOpen(ttUserInterface);
extern int  ether24J600DrvClose(ttUserInterface);
extern int  ether24J600Recv(ttUserInterface, char TM_FAR **, int TM_FAR *, ttUserBufferPtr);
extern int  ether24J600DriverIoctl(ttUserInterface, int, void TM_FAR *, int);
extern int  ether24J600GetPhysAddr( ttUserInterface, char TM_FAR * );
extern int  ether24J600Send( ttUserInterface interfaceHandle,char TM_FAR * dataPtr,int dataLength,int flag );

extern ttUserInterface	gInterfaceHandle_Ether;								/* KASAGO Interface Handle			*/
extern ttUserLinkLayer	gLinkLayerHandle_Ether;								/* KASAGO Link Layer Handle			*/
                                 
/* PPP 関連 */
extern int ppp0DrvOpen( ttUserInterface interfaceHandle );
extern int ppp0DrvClose( ttUserInterface interfaceHandle );
extern int ppp0DrvSend( ttUserInterface interfaceHandle,char TM_FAR *   dataPtr,int dataLength,int flag );
extern int ppp0DrvRecv( ttUserInterface interfaceHandle,
			char TM_FAR * TM_FAR * dataPtrPtr,int TM_FAR * dataSizePtr,ttUserBufferPtr packetPtrPtr );
extern int ppp0DrvIoctl( ttUserInterface interfaceHandle,int  flag,void TM_FAR *   optionPtr,int optionLen );
extern int ppp0DrvFreeReceiveBuffer( ttUserInterface interfaceHandle,char TM_FAR *   dataPtr );

extern int				KSG_gkasagoStarted;

extern short KSG_RauGetAntLevel( unsigned short mode, unsigned short interval );

extern int              KSG_gPpp_RauStarted;
extern ttUserLinkLayer	gLinkLayerHandle_PPP_rau;				// KASAGO Link Layer Handle
extern ttUserInterface	gInterfaceHandle_PPP_rau;				// KASAGO Interface Handle
extern int				KSG_gAntLevel;							// アンテナレベル
extern int				KSG_gReceptionLevel;					// 受信電力指標
extern	unsigned char	KSG_PPP_RAU_state;						// PPP状態
extern	unsigned char	KSG_AntennaLevelCheck;					// アンテナレベルチェック状態 0:未計測 1:計測中
extern	unsigned char	KSG_AntAfterPppStarted;					// アンテナレベルチェックによるE7778抑止用


extern int KSG_ClosePPPSession(ttUserInterface interfaceHandle);
extern int KSG_StartPPPSession(ttUserInterface interfaceHandle);
extern int KSG_GetInformation(ttUserInterface interfaceHandle,char ppp_Flag);
extern int KSG_initPppClient_rau( unsigned char iniMode );


#ifdef __cplusplus
}
#endif

#endif /* _KASAGO_DEF_H_ */
