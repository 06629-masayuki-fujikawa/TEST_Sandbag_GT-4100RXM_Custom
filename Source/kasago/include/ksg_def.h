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

#define PPP_FIRSTINI			0		// �N�����̏�����
#define PPP_SECONDINI			1		// �Đڑ����̏�����

#define MAX_USERS				5
#define MAX_CHARS				16

#define FTPD_MAX_CONN			5		// �����Ɏ󂯓���邱�Ƃ��ł���FTP�ڑ�
#define FTPD_MAX_BACK			2		// FTP�T�[�o�\�P�b�g�ŋ�����ڑ��ۗ����L���[�̍ő吔
#define FTPD_MAX_IDLETIMEOUT	300
#define RAMFS_BLOCKCOUNT		10

enum {
	ROUTER_OFF = 0,
	ROUTER_INIT,
	ROUTER_IDLE
};

#define ZERO_ADDR_V4			"0.0.0.0"

#define _Normal              	0     //: OK
#define _InitializeError     	-1    //: NG  �FInitialize
#define _LinkLayerError      	-2    //:     �FLink Layer Initialize
#define _AddDriverError      	-3    //:     �Fdriver addition
#define _SetIpAddressError   	-4    //:     �FIP Address setting
#define _SetGatewayError     	-5    //:     �FGateway setting
#define _SetMtuError         	-6    //:     �FMTU setting
#define _SetDnsError         	-7    //:     �FDNS setting
#define _SetIpv6AddressError 	-8    //:     �FIP Address setting(v6)
#define _SetIpv6DnsError     	-9    //:     �FDNS setting(v6)
#define _SetMcastInterfaceError -10   //:     �FMulticast Interface setting

#define	KSG_PPP_STATE_CLOSE				0
#define	KSG_PPP_STATE_OPEN_STATRT		1
#define	KSG_PPP_STATE_OPEN				2
#define	KSG_PPP_STATE_CLOSE_START		3


extern int KSG_Initialize(char v4Dhcp_flg);

/* FTP �֘A */
extern int tfRamfsInit(ttRamfsBlock blCount);

/* ether �֘A */
extern int  ether24J600DrvOpen(ttUserInterface);
extern int  ether24J600DrvClose(ttUserInterface);
extern int  ether24J600Recv(ttUserInterface, char TM_FAR **, int TM_FAR *, ttUserBufferPtr);
extern int  ether24J600DriverIoctl(ttUserInterface, int, void TM_FAR *, int);
extern int  ether24J600GetPhysAddr( ttUserInterface, char TM_FAR * );
extern int  ether24J600Send( ttUserInterface interfaceHandle,char TM_FAR * dataPtr,int dataLength,int flag );

extern ttUserInterface	gInterfaceHandle_Ether;								/* KASAGO Interface Handle			*/
extern ttUserLinkLayer	gLinkLayerHandle_Ether;								/* KASAGO Link Layer Handle			*/
                                 
/* PPP �֘A */
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
extern int				KSG_gAntLevel;							// �A���e�i���x��
extern int				KSG_gReceptionLevel;					// ��M�d�͎w�W
extern	unsigned char	KSG_PPP_RAU_state;						// PPP���
extern	unsigned char	KSG_AntennaLevelCheck;					// �A���e�i���x���`�F�b�N��� 0:���v�� 1:�v����
extern	unsigned char	KSG_AntAfterPppStarted;					// �A���e�i���x���`�F�b�N�ɂ��E7778�}�~�p


extern int KSG_ClosePPPSession(ttUserInterface interfaceHandle);
extern int KSG_StartPPPSession(ttUserInterface interfaceHandle);
extern int KSG_GetInformation(ttUserInterface interfaceHandle,char ppp_Flag);
extern int KSG_initPppClient_rau( unsigned char iniMode );


#ifdef __cplusplus
}
#endif

#endif /* _KASAGO_DEF_H_ */
