/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModemMain.c	:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					:���ѐ���Ҳ�											  : */
/*	���߲�		:					:														  : */
/*	�ް�ޮ�		:					:														  : */
/*	���ޯ�CPU	:					:---------------------------------------------------------: */
/*	�Ή��@��	:					: 														  : */
/*																								*/
/************************************************************************************************/
																		/*							*/
#include	<machine.h>													/*							*/
#include	<string.h>													/*							*/
#include	"system.h"													/*							*/
#include	"ksgmac.h"													/*							*/
#include	"ksg_def.h"
#include	"ksgRauTable.h"												/* ð����ް���`			*/
#include	"ksgRauModem.h"												/* ���ъ֘A�֐��S			*/
#include	"ksgRauModemData.h"											/* ���ъ֘A�ް���`			*/

extern BITS	KSG_uc_mdm_res_Foma_f;
extern BITS	KSG_uc_mdm_ErrSndReq_f;
																		/*							*/
/*[]------------------------------------------------------------------------------------------[]*/
/*|	�e���޲��̋N��																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_RauDeviceStart()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauDeviceStart( void )	
{																		/*							*/
	/* MobileArk�̋N��			*/
	KSG_mdm_Condit = 1;													/* 							*/
	KSG_mdm_TCPcnct_req = 1;											/*							*/
	KSG_mdm_ActFunc.BIT.B0 = 1;		KSG_mdm_ActFunc.BIT.B1 = 1;			/* �ڑ�����޲				*/
	KSG_mdm_Start.BIT.B0 = 1;											/* MobileArk�J�n			*/
}	

/*[]------------------------------------------------------------------------------------------[]*/
/*|	�����ް�������																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmEarlyDataInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauMdmEarlyDataInit( void )
{																		/*							*/
	KSG_mdm_ActFunc.BYTE = 0;											/*							*/
	KSG_mdm_Start.BYTE = 0;												/* ���ђ�~					*/
	KSG_mdm_f_TmStart = 0;												/* ���ԊĎ��@�\��~			*/
	KSG_RauModemOnline = 0;												/* νĉ������׸� �ؒf		*/
	KSG_dials.BYTE = 0;													/* �޲�ٽð�� ؾ��			*/
	KSG_mdm_status = 0;													/* ���ѐڑ��ð��			*/
	KSG_mdm_init_sts = 0;												/* ���я����������~		*/
	KSG_modem_req = 0;													/* ���ѐڑ�ظ����׸�		*/
	KSG_mdm_TCP_CNCT = 0;												/* TCP�ȸ��ݏ���׸�		*/
	KSG_line_stat_h = (KSG_CD_SIG | KSG_DR_SIG | KSG_CS_SIG);			/* ν� ײݽð��				*/
	KSG_uc_MdmInitRetry = 0;											// �d��ON���s�����g���C�J�E���^ //
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���т̏�����																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModemInit()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModemInit( void )
{																		/*							*/

	KSG_dials.BYTE = 0;													/* �޲�ٽð�� ؾ��			*/
	KSG_txdlen_mdm = 0;													/* ���M�ް���				*/
	KSG_rxdlen_mdm = 0;													/* ��M�ް���				*/
	KSG_RauMdmRcvQueInit();												/* ��M����̏�����			*/
	KSG_RauMdmSndQueInit();												/* ���M����̏�����			*/
	KSG_RauMdmTimerInit();												/* ���ԊĎ��@�\������		*/
 	KSG_mdm_Condit = 0;													/* 							*/
	KSG_dial_go = 0;													/* �޲�ِؒf				*/
	KSG_net_online = 0;													/* ��ײ��׸ސؒf			*/
	KSG_rx_save_len = 0;												/* ��M�����ް���			*/
	KSG_ptr_rx_save = &KSG_mdm_r_buf[0];								/* ��M�ް��i�[				*/
	KSG_f_ReDial_ov = KSG_SET;											/* ��޲����ϒ�~			*/
	KSG_Tm_Gurd.tm = 0;
	KSG_Tm_Gurd.BIT.bit0 = (ushort)-10;	// 1S //
	KSG_f_Gurd_rq = KSG_SET;
	KSG_mdm_e_flag.BYTE = 0;											/* ���Ѵװ�׸޸ر			*/
	KSG_mdm_cut_req = 0;												/* ���ѐؒfظ����׸�OFF		*/
	KSG_disconect_flg = 0;												/* TCP�ȸ��ݐؒf�׸�		*/
	KSG_tcp_inout = 2;													/* TCP���M�N�����			*/
	KSG_RauMdmSigInfoInit();											/* �M�� ������ر������	*/
	KSG_Tm_Reset_t = 0;													/* �@��ؾ����ϒ�~			*/
	KSG_uc_AtRetryCnt = 0;						// 3 times retury counter //
	KSG_uc_AtResultWait = 0;					// 0:-- 1:Waiting for Result Code //
	KSG_uc_ConnAftWait = 0;						// CONNECT ( 500m ) -> PPP //
	KSG_uc_FomaWaitDRFlag = 0;					// 100msec. ER ON �� //
	KSG_uc_FomaPWRphase = 0;					// �d���Ǘ��t�F�[�Y 0=IDLE, 1=CD off �Ď�, 2=PWR�M�� HIGH //
	KSG_uc_mdm_res_Foma_f.BYTE = 0;
	KSG_uc_mdm_ErrSndReq_f.BYTE = 0;
	KSG_Tm_ReDialTime.tm = 0;

}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	����Ҳݏ���																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModemMain()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauModemMain( void )
{																		/*							*/
	switch( KSG_mdm_Condit ) {
	// --- ������ ----------------------------------------------------------------------------- */
	case 1:																/*							*/
		if( KSG_mdm_Start.BYTE & KSG_mdm_ActFunc.BYTE ) {				/*							*/
			KSG_mdm_Condit = 3;											/*							*/
			KSG_mdm_f_TmStart = 1;										/* ���ԊĎ��@�\�J�n			*/
			KSG_RauMdmInitReq();										/* ���я������v������		*/
		}																/*							*/
		break;															/*							*/

	// ---------------------------------------------------------------------------------------- */
	case 3:																/*							*/
		if( ( KSG_mdm_status < 2 ) && ( KSG_f_data_rty_ov == 1 ) ) {	/* DTE�m���O				*/
			KSG_RauMdmInitReq();										/* ���я������v������		*/
		}																/*							*/
		KSG_RauMdmControl();											/* ���Ѻ��۰�				*/
		break;															/*							*/

	// ---------------------------------------------------------------------------------------- */
	default:															/*							*/
		KSG_Tm_No_Action.tm = 0;										/* ���ʐM��ϋN����~		*/
		KSG_Tm_TCP_DISCNCT.tm = 0;										/* TCP�ȸ��ݐؒf�҂���ϒ�~	*/
		break;															/*							*/
	}																	/*							*/
// MH322914 (s) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)
//	if(( KSG_mdm_cut_req >= 2 ) && ( KSG_mdm_status < 4 )) KSG_mdm_cut_req = 0;	/* modem�ؒfظ����׸�OFF	*/
// MH322914 (e) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)

	// ���f���X�e�[�^�X��ԋp����
	return KSG_mdm_status;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	��M����̏�����																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmRcvQueInit( void )
{																		/*							*/
	KSG_mdm_Rcv_Ctrl.Count = 0;											/* �i�[���̸ر				*/
	KSG_mdm_Rcv_Ctrl.ReadIdx = 0;										/* ��o�߲�ĸر				*/
	KSG_mdm_Rcv_Ctrl.WriteIdx = 0;										/* �i�[�߲�ĸر				*/
	KSG_mdm_Rcv_Ctrl.dummy = 0;											/*							*/
}																		/*							*/

// ���g�p
/*[]------------------------------------------------------------------------------------------[]*/
/*|	��M����̐ݒ�																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueSet()											  			   |*/
/*| PARAMETER1  : unsigned char *pData  :	��M�ް�										   |*/
/*| PARAMETER2  : unsigned short len  	:	������											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmRcvQueSet( unsigned char *pData, unsigned int len )
{																		/*							*/
	if( len > KSG_MDM_DATA_MAX )	len = KSG_MDM_DATA_MAX;				/* �������̐���				*/
	KSG_mdm_Rcv_Ctrl.modem_data[ KSG_mdm_Rcv_Ctrl.WriteIdx ].Len = len;	/* �������̊i�[				*/
	memcpy( (char *)KSG_mdm_Rcv_Ctrl.modem_data							/* ������̊i�[				*/
				[ KSG_mdm_Rcv_Ctrl.WriteIdx ].Dat, pData, (size_t)len );/*							*/
	++KSG_mdm_Rcv_Ctrl.Count;											/* �i�[���̉��Z				*/
	++KSG_mdm_Rcv_Ctrl.WriteIdx;										/* �i�[�߲�Ẳ��Z			*/
	KSG_mdm_Rcv_Ctrl.WriteIdx &= 3;										/* �i�[�߲�Ĕ͈͐���		*/
}																		/*							*/

// ���g�p
/*[]------------------------------------------------------------------------------------------[]*/
/*|	��M����̓Ǐo��																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmRcvQueRead()													   |*/
/*| PARAMETER1  : unsigned char *pData  :	��M�ް�										   |*/
/*| PARAMETER2  : unsigned short len  	:	������											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauMdmRcvQueRead( unsigned char *pData, unsigned short *len )
{																		/*							*/
	unsigned short	w_len;												/*							*/
																		/*							*/
	if( 0 == KSG_mdm_Rcv_Ctrl.Count )	return	0x00;					/* ��M��������Ώo��		*/
	w_len = KSG_mdm_Rcv_Ctrl.modem_data[ KSG_mdm_Rcv_Ctrl.ReadIdx ].Len;/* �������̒��o				*/
	if( w_len > KSG_MDM_DATA_MAX )	w_len = KSG_MDM_DATA_MAX;			/* �������̐���				*/
	*len = w_len;														/* ��������Ԃ�				*/
	memcpy( pData, (char *)KSG_mdm_Rcv_Ctrl.modem_data					/* ��M���������o			*/
				[ KSG_mdm_Rcv_Ctrl.ReadIdx ].Dat, (size_t)w_len );		/*							*/
	--KSG_mdm_Rcv_Ctrl.Count;											/* �i�[�����Z				*/
	++KSG_mdm_Rcv_Ctrl.ReadIdx;											/* ��o�߲�ĉ��Z			*/
	KSG_mdm_Rcv_Ctrl.ReadIdx &= 3;										/* ��o�߲�Ĕ͈͐���		*/
	return 0x01;														/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���M����̏�����																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueInit()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSndQueInit( void )
{																		/*							*/
	KSG_mdm_Snd_Ctrl.Count = 0;											/* �i�[���ر				*/
	KSG_mdm_Snd_Ctrl.ReadIdx = 0;										/* ��o�߲�ĸر				*/
	KSG_mdm_Snd_Ctrl.WriteIdx = 0;										/* �i�[�߲�ĸر				*/
	KSG_mdm_Snd_Ctrl.dummy = 0;											/* 							*/
}																		/*							*/

// ���g�p
/*[]------------------------------------------------------------------------------------------[]*/
/*|	���M����̐ݒ�																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueSet()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSndQueSet( unsigned char *pData, unsigned int len , unsigned char ch )
{																		/*							*/
	if( len > KSG_MDM_DATA_MAX ) len = KSG_MDM_DATA_MAX;				/* �������͈͐���			*/
	KSG_mdm_Snd_Ctrl.modem_data											/* �������̊i�[				*/
			[ KSG_mdm_Snd_Ctrl.WriteIdx ].Len = len;					/*							*/
	memcpy( (char *)KSG_mdm_Snd_Ctrl.modem_data							/* �����̊i�[				*/
			[ KSG_mdm_Snd_Ctrl.WriteIdx ].Dat, pData, (size_t)len );	/*							*/
	KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.WriteIdx ].ch = ch;	/* �w�迹��(UDP�̂�)		*/
	++KSG_mdm_Snd_Ctrl.Count;											/* �i�[�����Z				*/
	++KSG_mdm_Snd_Ctrl.WriteIdx;										/* �i�[�߲�ĉ��Z			*/
	KSG_mdm_Snd_Ctrl.WriteIdx &= 3;										/* �i�[�߲�Đ���			*/
}																		/*							*/

// ���g�p
/*[]------------------------------------------------------------------------------------------[]*/
/*|	���M����̓Ǐo��																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSndQueRead()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned char	KSG_RauMdmSndQueRead( unsigned char *pData, unsigned int *len, unsigned char *ch, unsigned char kind )
{																		/*							*/
	unsigned int	w_len;												/*							*/
																		/*							*/
	if( 0 == KSG_mdm_Snd_Ctrl.Count )	return	0;						/* ���M�������Ȃ�o��		*/
	if( kind )						return	1;							/* �L���m�F�����Ȃ�o��		*/
	w_len = KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.ReadIdx ].Len;/* �i�[���������擾			*/
	if( w_len > KSG_MDM_DATA_MAX )		w_len = KSG_MDM_DATA_MAX;		/* �i�[����������			*/
	*len = w_len;														/* �i�[���������			*/
	*ch = KSG_mdm_Snd_Ctrl.modem_data[ KSG_mdm_Snd_Ctrl.ReadIdx ].ch;	/* �w�迹�Ă��			*/
	memcpy( pData, (char *)KSG_mdm_Snd_Ctrl.modem_data					/* �i�[��������			*/
			[ KSG_mdm_Snd_Ctrl.ReadIdx ].Dat, (size_t)w_len );			/*							*/
	--KSG_mdm_Snd_Ctrl.Count;											/* �i�[�����Z				*/
	++KSG_mdm_Snd_Ctrl.ReadIdx;											/* ��o�߲�����Z			*/
	KSG_mdm_Snd_Ctrl.ReadIdx &= 3;										/* ��o�߲���͈͐���		*/
	return	1;															/* 							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	������																					   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSumCheck()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSumCheck( unsigned char *pData, unsigned short len, unsigned char *SUM )
{																		/*							*/
	unsigned short	i, s;												/*							*/
																		/*							*/
	for( i = 0, s = 0; i < len; i++ )									/* ������S�̂�ώZ			*/
		s = s + (unsigned short)pData[i];								/*							*/
	s = s & 0x00ff;														/* �ŉ����޲Ă�L��			*/
	i = ( s & 0x00f0 )>>4;												/* ���4�ޯĒ��o			*/
	if( i >= 0x000a )	SUM[0] = (unsigned char)( i + 0x0037 );			/* A�`F��ASCII�ϊ�			*/
	else				SUM[0] = (unsigned char)( i + 0x0030 );			/* 0�`9��ASCII�ϊ�			*/
	i = s & 0x000f;														/* ����4�ޯ�				*/
	if( i >= 0x000a )	SUM[1] = (unsigned char)( i + 0x0037 );			/* A�`F��ASCII�ϊ�			*/
	else				SUM[1] = (unsigned char)( i + 0x0030 );			/* 0�`9��ASCII�ϊ�			*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	PPP�ؒf���̌㏈��																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmPostPppClose()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmPostPppClose( void )
{
	// ���f���ؒf
	// �A���e�i���x���`�F�b�N�Ȃ烂�f��OFF���Ȃ�
	if(KSG_AntennaLevelCheck == 0) {
		KSG_RauModem_OFF();
	}
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	PPP�ؒf���̌㏈��																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauClosePPPSession()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauClosePPPSession( void )
{
	if(gInterfaceHandle_PPP_rau && KSG_gPpp_RauStarted) {	// �n���h���L���Őڑ���?
		KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
	}
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	���f���ؒf�v����Ԏ擾																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauGetMdmCutState()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
uchar	KSG_RauGetMdmCutState( void )
{
	return KSG_mdm_cut_req;
}
/*[]------------------------------------------------------------------------------------------[]*/
/*|	���f���ؒf�v����Ԑݒ�																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauSetMdmCutState()													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauSetMdmCutState( unsigned char modemCutRequestState )
{
	KSG_mdm_cut_req = modemCutRequestState;
}
