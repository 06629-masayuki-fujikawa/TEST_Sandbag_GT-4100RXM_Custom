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

ttUserIpAddress	KSG_gPPP_RauRemoteIpAddress;							// rau �T�[�o�[IP
ttUserIpAddress	KSG_gPPP_RauDNSIpAddressPri;							// rau DNS�A�h���X(�v���C�}��)
ttUserIpAddress	KSG_gPPP_RauDNSIpAddressSec;							// rau DNS�A�h���X(�Z�J���_��)
ttUserIpAddress	KSG_gPPP_RauOwnIpAddress;								// rau ��IP

extern void myPppNotifyFunction(ttUserInterface interfaceHandle,int flags);
/* Set PPP options of LCP, IPCP, PAP, CHAP and IPHC. */
static void setPppOptionLcpC( ttUserInterface interfaceHandle );
static void setPppOptionIpcpC( ttUserInterface interfaceHandle );

/****************************************************************************/
/*  Function:                                                               */
/*      KSG_initPppClient_rau(iniMode)                                      */
/*          iniMode=0(�N�����̏�����)                                       */
/*                 =1(�Đڑ����̏�����)                                     */
/*  Description:                                                            */
/*      KASAGO����������( PPP client )                                      */
/****************************************************************************/
extern int KSG_initPppClient_rau( unsigned char iniMode )
{
	int		errorCode;

	if( iniMode == PPP_FIRSTINI){
		// PPP �N���C�A���g�����N���C����������
		gLinkLayerHandle_PPP_rau = tfUseAsyncPpp((ttUserLnkNotifyFuncPtr)myPppNotifyFunction );
		if( gLinkLayerHandle_PPP_rau  == NULL )
		{
			return TM_EINVAL;
		}

		// �C���^�t�F�[�X�h���C�o��ǉ�����
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

	// LCP�I�v�V�����ݒ�
	setPppOptionLcpC( gInterfaceHandle_PPP_rau );

	// IPCP�I�v�V�����ݒ�
	setPppOptionIpcpC( gInterfaceHandle_PPP_rau );

	KSG_gPpp_RauStarted = 0;

	return errorCode;
}

/****************************************************************************/
/*  Function:                                                               */
/*      setPppOptionLcp( )                                                  */
/*                                                                          */
/*  Description:                                                            */
/*      LCP�I�v�V�����ݒ�                                                   */
/*                                                                          */
/****************************************************************************/
static void setPppOptionLcpC( ttUserInterface interfaceHandle )
{
//	int				errorCode;
	char			optionValue;
//	unsigned short	auth;			/* PAP/CHAP�F�؃v���g�R�� */
	unsigned long	accm;			/* ACCM */
	unsigned short	mru;			/* �ő��M�o�C�g�� */


	// �v���g�R���t�B�[���h���k�I�v�V�����Z�b�g
	optionValue = 1;
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_LCP_PROTOCOL,
					TM_PPP_OPT_WANT,
					TM_LCP_PROTOCOL_COMP,
					&optionValue,
					sizeof( optionValue ) );

	// ACCM�I�v�V�����Z�b�g
	accm = 0xffffffff;
	tfPppSetOption( interfaceHandle,
					(int)TM_PPP_LCP_PROTOCOL,
					TM_PPP_OPT_ALLOW,
					TM_LCP_ACCM,
					(char *)&accm,
					sizeof( accm ) );

	// �ő��M�P�ʃI�v�V�����Z�b�g
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
/*      IPCP�I�v�V�����ݒ�                                                  */
/*                                                                          */
/****************************************************************************/
static void setPppOptionIpcpC( ttUserInterface interfaceHandle )
{
	ttUserIpAddress	ip_addr;

	ip_addr = inet_addr("0.0.0.0");										// �S�Ă�IP������
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
/*      PPP�J�n����                                                         */
/*                                                                          */
/****************************************************************************/
int KSG_StartPPPSession(ttUserInterface interfaceHandle)
{
	int    errorCode;
	ttUserIpAddress ip_addr;
	ttUserIpAddress subnet_addr;

	if( interfaceHandle == gInterfaceHandle_PPP_rau){
		if( KSG_RauConf.Dpa_IP_m.FULL != 0 ){
			ip_addr = htonl(KSG_RauConf.Dpa_IP_m.FULL);				// ��IP�A�h���X�擾
			subnet_addr = htonl(KSG_RauConf.netmask.FULL);			// �T�u�l�b�g�擾
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f���ؒf���PPP�Đڑ��̖��C��
// NOTE:PPP�ؒf���ɃR�[������tfCloseInterface()�֐����Ăяo���ƁAPPP�̃I�v�V�����ݒ�͑S�ď�����
// �����(KASAGO�d�l�}�j���A���L�ڂ���)���̂��ߍĐڑ����ɍs��tfOpenInterface()�֐��Ăяo���̑O��
// �K��PPP�̃I�v�V�����ݒ�tfPppSetOption()�֐����R�[�����Đݒ���s���K�v������B
// PPP�ؒf���AKASAGO����C�x���g�ʒm���󂯂�I/F myPppNotifyFunction()�֐��̒���
// TM_LL_CLOSE_COMPLETE�C�x���g����M�ł��Ȃ����Ƃ�����B
// ���� TM_LL_CLOSE_COMPLETE�C�x���g��M�ɂčĐڑ�����PPP�̃I�v�V�����ݒ���s���Ă���
// ���̃P�[�X�ł͍Đڑ��͂ł��Ȃ��Ȃ��肪�������邽��
// �Đڑ�����PPP�̃I�v�V�����ݒ���ēx�s��
			KSG_initPppClient_rau(PPP_SECONDINI);
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f���ؒf���PPP�Đڑ��̖��C��
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
/*      PPP�I������                                                         */
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
/*      IP�A�h���X���̎擾                                                  */
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

	// ��IP�擾
	if (tfGetIpAddress(interfaceHandle,ptempOwnIpAddress,0) == 0){
	}
	else{
		*ptempOwnIpAddress = 0;
	}
	// �T�[�o�[IP�擾
	if (tfGetPppPeerIpAddress(interfaceHandle,ptempRemoteIpAddress) == 0){
	}
	else{
		*ptempRemoteIpAddress = 0;
	}
	// DNS�T�[�o�[�i�v���C�}���jIP�擾
	if (tfGetPppDnsIpAddress(interfaceHandle,ptempDNSIpAddressPri,TM_DNS_PRIMARY) == 0){
	}
	else{
		// �擾�ł��Ȃ��̂ō폜
		*ptempDNSIpAddressPri = 0;
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_PRI_SERVER);
	}
	// DNS�T�[�o�[�i�Z�J���_���jIP�擾
	if (tfGetPppDnsIpAddress(interfaceHandle,ptempDNSIpAddressSec,TM_DNS_SECONDARY) == 0)
	{
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_SEC_SERVER);
		if( errorCode != TM_ENOERROR ){
		}
	}
	else{
		// �擾�ł��Ȃ��̂ō폜
		*ptempDNSIpAddressSec = 0;
		errorCode = tfDnsSetServer(*ptempDNSIpAddressPri, TM_DNS_PRI_SERVER);
	}
	return errorCode;
}
