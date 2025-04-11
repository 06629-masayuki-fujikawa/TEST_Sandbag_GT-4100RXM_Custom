/*==========================================================================*/
/*  Include                                                                 */
/*==========================================================================*/
/* KASAGO */
#include	<trsocket.h>
#include	<trmacro.h>
#include	<trtype.h>

#include	"ksg_def.h"
#include	"ksgRauTable.h"
#include	"system.h"	
#include	"prm_tbl.h"	
#include	"raudef.h"
#include	"ksgRauModem.h"

/*==========================================================================*/
/*  Global                                                                  */
/*==========================================================================*/
extern int	KSG_gPpp_RauStarted;

/* �����ꂽ���[�U */
static char user[ MAX_USERS ][ MAX_CHARS + 1 ] = 
{
	"elmic"
};

/* �����ꂽ�p�X���[�h */
static char pass[ MAX_USERS ][ MAX_CHARS + 1 ] = 
{
	"elmicsys"
};

unsigned char	KSG_PPP_RAU_state = KSG_PPP_STATE_CLOSE;

/****************************************************************************/
/*  Function:                                                               */
/*      myPppNotifyFunction( )                                              */
/*                                                                          */
/*  Description:                                                            */
/*      PPP�C�x���g��ʒm����B                                             */
/****************************************************************************/
extern void myPppNotifyFunction( ttUserInterface interfaceHandle, int flags )
{
	ttUserIpAddress ip_addr;
	char cPPP_Flag;
//	char ip[20];
//	ttUserIpAddress ftp_ip_addr;

	switch( flags )
	{
		case TM_LL_OPEN_STARTED:      printf( "TM_LL_OPEN_STARTED (0x%03x)\n", flags );      break;
		case TM_LL_OPEN_COMPLETE:     printf( "TM_LL_OPEN_COMPLETE (0x%03x)\n", flags );     break;
		case TM_LL_OPEN_FAILED:       printf( "TM_LL_OPEN_FAILED (0x%03x)\n", flags );       break;
		case TM_LL_CLOSE_STARTED:     printf( "TM_LL_CLOSE_STARTED (0x%03x)\n", flags );     break;
		case TM_LL_CLOSE_COMPLETE:    printf( "TM_LL_CLOSE_COMPLETE (0x%03x)\n", flags );    break;
		case TM_LL_LCP_UP:            printf( "TM_LL_LCP_UP (0x%03x)\n", flags );            break;
		case TM_LL_PAP_UP:            printf( "TM_LL_PAP_UP (0x%03x)\n", flags );            break;
		case TM_LL_CHAP_UP:           printf( "TM_LL_CHAP_UP (0x%03x)\n", flags );           break;
		case TM_LL_IP4_OPEN_FAILED:   printf( "TM_LL_IP4_OPEN_FAILED (0x%03x)\n", flags );   break;
		case TM_LL_IP6_OPEN_FAILED:   printf( "TM_LL_IP6_OPEN_FAILED (0x%03x)\n", flags );   break;
		case TM_LL_IP4_OPEN_COMPLETE: printf( "TM_LL_IP4_OPEN_COMPLETE (0x%03x)\n", flags ); break;
		case TM_LL_IP6_OPEN_COMPLETE: printf( "TM_LL_IP6_OPEN_COMPLETE (0x%03x)\n", flags ); break;
		default: printf( "??event??" ); break;
	}
	if( interfaceHandle == gInterfaceHandle_PPP_rau){
		cPPP_Flag = PPP_FLAG_0;
	}
	else{
		return;
	}
	switch( flags )
	{
		case TM_LL_OPEN_COMPLETE:
		{
			if( cPPP_Flag == PPP_FLAG_0 ){
				// Parking Web���̃��[�e�B���O���̐ݒ�
				if( KSG_RauConf.Dpa_IP_h.FULL != 0 ){
					ip_addr = htonl(KSG_RauConf.Dpa_IP_h.FULL);
					tfAddStaticRoute( interfaceHandle,
										ip_addr,
										inet_addr("255.0.0.0"),
										inet_addr("0.0.0.0"),
										14);
				}
				// �Z���^�[�N���W�b�g���̃��[�e�B���O���̐ݒ�
				if( KSG_RauConf.Dpa_IP_Cre_h.FULL != 0 ){
					ip_addr = htonl(KSG_RauConf.Dpa_IP_Cre_h.FULL);
					tfAddStaticRoute( interfaceHandle,
										ip_addr,
										inet_addr("255.0.0.0"),
										inet_addr("0.0.0.0"),
										14);
				}
				KSG_gPpp_RauStarted = 1;
				KSG_PPP_RAU_state = KSG_PPP_STATE_OPEN;
				SetUpNetState(NET_STA_PPP_OPEN);
				SetDownNetState(NET_STA_PPP_OPEN);
				KSG_AntAfterPppStarted = 0;						// �A���e�i���x���`�F�b�N�ɂ��E7778�}�~�p
			}
			// �I�[�v�����������̂ŁAIP�A�h���X�Ȃǎ擾
			KSG_GetInformation(interfaceHandle,cPPP_Flag);
		}
		break;
		case TM_LL_CLOSE_STARTED:
		{
			if( cPPP_Flag == PPP_FLAG_0 ){
				if( KSG_gPpp_RauStarted ){
					KSG_gPpp_RauStarted = 0;
					KSG_PPP_RAU_state = KSG_PPP_STATE_CLOSE_START;
					SetUpNetState(NET_STA_PPP_CLOSING);
					SetDownNetState(NET_STA_PPP_CLOSING);
				// Rau��PPP�Z�b�V�����؂�ŏI���������{�B
					KSG_ClosePPPSession(interfaceHandle);
				}
			}
		}
		break;
		case TM_LL_CLOSE_COMPLETE:
		{
			if( cPPP_Flag == PPP_FLAG_0 ){
				KSG_PPP_RAU_state = KSG_PPP_STATE_CLOSE;
				SetUpNetState(NET_STA_PPP_CLOSE);
				SetDownNetState(NET_STA_PPP_CLOSE);
				// Rau�ؒf��̍Đڑ��p��KASAGO������
				KSG_initPppClient_rau(PPP_SECONDINI);
				KSG_RauMdmPostPppClose();
			}
		}
		break;
		case TM_LL_OPEN_STARTED:
		{
			KSG_PPP_RAU_state = KSG_PPP_STATE_OPEN_STATRT;
			// tfOpenInterface
			KSG_StartPPPSession(interfaceHandle);
		}
		break;
	}
}

// �F�؂Ȃ��ׁ̈A���g�p
/****************************************************************************/
/*  Function:                                                               */
/*      tkfPapUserAuthCheck( )                                              */
/*                                                                          */
/*  Description:                                                            */
/*      PAP���[�U�F��                                                       */
/*                                                                          */
/****************************************************************************/
extern int tkfPapUserAuthCheck( char * userName, char * passWord )
{
	char index; /* ���[�U�����C���f�b�N�X */

	index = 0;
	while( index < MAX_USERS ){
		if(    ( strcmp( userName, user[ index ] ) == 0 ) 
			&& ( strcmp( passWord, pass[ index ] ) == 0 ) )
		{
			return 1;
		}
		index++;
	}
	return 0;
}

// �F�؂Ȃ��ׁ̈A���g�p
/****************************************************************************/
/*  Function:                                                               */
/*      tkfChapUserAuthCheck( )                                             */
/*                                                                          */
/*  Description:                                                            */
/*      CHAP���[�U�F��                                                      */
/*                                                                          */
/****************************************************************************/
extern char * tkfChapUserAuthCheck( char * userName )
{
	char index; /* ���[�U�����C���f�b�N�X */

	index = 0;
	while( index < MAX_USERS )
	{
		if( strcmp( userName, user[ index ] ) == 0 )
		{
			return pass[ index ];
		}
		index++;
	}
	return (char *)0;
}
