/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		KASAGO�^�X�N 																						   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgtask.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �EKASAGO�^�X�N���C������																				   |*/
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
/*| KASAGO�^�X�N����																						   |*/
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

	KSG_RauInitialTbl();												// Rau�f�[�^�e�[�u��������
	KSG_RauSetRauConf();												// Rau�ݒ�̔��f

	KSG_Initialize(NOUSE_DHCP_V4);										// KASAGO������(�ݒ�擾��j

	KSG_RauComInit();													// Rau�pSCI������

	KSG_RauModemInit();													// Rau�p���f��������

	KSG_RauDeviceStart();												// Rau�f�o�C�X����������

	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) == 2){							// Rau�g�p�����擾
		KSG_RauEnable = 1;												// NT-NET�ڑ�����
		if(KSG_RauConf.Dpa_IP_m.FULL == 0 && 0 == prm_get(COM_PRM, S_CEN, 51, 1, 3)) {
			// ���f���ڑ����Ɏ�IP���ݒ肳��Ă��Ȃ��ƁA���Z�b�g���������邽�߂̑΍�
			// ���炪�T�[�o�ƂȂ�ꍇ�݂̂�kasaga������unknown vector����������
			// �N���C�A���g�̏ꍇ(FX��Cappi��Rism)�͔������Ȃ�
			KSG_RauEnable = 0;											// NT-NET�ڑ��Ȃ�
		}
	}
	else if(RAU_Credit_Enabale != 0){
		KSG_RauEnable = 1;												// NT-NET�ڑ�����
		if(KSG_RauConf.Dpa_IP_m.FULL == 0 && RAU_Credit_Enabale == 2) {
			// ���f���ڑ����Ɏ�IP���ݒ肳��Ă��Ȃ��ƁA���Z�b�g���������邽�߂̑΍�
			// ���炪�T�[�o�ƂȂ�ꍇ�݂̂�kasaga������unknown vector����������
			// �N���C�A���g�̏ꍇ(FX��Cappi��Rism)�͔������Ȃ�
			KSG_RauEnable = 0;											// NT-NET�ڑ��Ȃ�
		}
	}
	else{
		KSG_RauEnable = 0;												// NT-NET�ڑ��Ȃ�
	}

	// ���C�����[�v
	while(1) {
	
		taskchg( IDLETSKNO );

        tfTimerExecute();												// KASAGO�^�C�}����

		/* --- LAN ---------------------------------------------------------------------------*/
		if (tfCheckReceiveInterface(gInterfaceHandle_Ether) == 0){		// LAN��M�`�F�b�N
			tfRecvInterface(gInterfaceHandle_Ether);					// LAN��M����
		}

		if (tfCheckSentInterface(gInterfaceHandle_Ether) == 0){			// LAN���M�`�F�b�N
			tfSendCompleteInterface(gInterfaceHandle_Ether, TM_DEV_SEND_COMPLETE_APP);	// LAN���M����
		}

		if(KSG_RauEnable){
			if( KSG_RauConf.modem_exist ){
				// ���f������(LAN)����PPP�ڑ��Ȃ���Ethernet�ڑ�
				KSG_gPpp_RauStarted = 1;
				SetUpNetState(NET_STA_PPP_OPEN);
				SetDownNetState(NET_STA_PPP_OPEN);
			}
			else{
				KSG_RauComdrMain();										// Rau�pSCI���C������

				// Rau�p���f�����C������
				if(( KSG_RauModemMain() >= 4 ) && (KSG_RauModemOnline == 1 )){	// ���f���ڑ�������
					if(KSG_gPpp_RauStarted == 0 && 0 == RAU_GetAntennaLevelState()){
						// PPP���؂�Ă�����Đڑ�
						KSG_StartPPPSession(gInterfaceHandle_PPP_rau);	// Rau�pFOMA PPP�ڑ��J�n
					}
				}
				if (tfCheckSentInterface(gInterfaceHandle_PPP_rau) == 0){	// Rau�p���M�`�F�b�N
					// Rau�p���M��������
					tfSendCompleteInterface(gInterfaceHandle_PPP_rau, TM_DEV_SEND_COMPLETE_APP);
				}
			}
		}
	}
}
