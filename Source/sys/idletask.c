/*[]----------------------------------------------------------------------[]*/
/*| idletask, message control                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"Message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"irq1.h"
#include	"flp_def.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ntnetauto.h"
#include	"AppServ.h"
#include	"remote_dl.h"
#include	"can_api.h"
#include	"can_def.h"
#include	"fb_def.h"
#include 	"ksg_def.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"
#include	"updateope.h"
#include	"ifm_ctrl.h"
#include	"raudef.h"
#if (1 == AUTO_PAYMENT_PROGRAM)

#include	"LKcom.h"
#include	"LKmain.h"
#include	"tbl_rkn.h"

static void AutoPaymentIn(t_AutoPayment_In *In);
static void AutoPaymentOut(t_AutoPayment_Out *Out);

#endif

#if (1 == AUTO_PAYMENT_PROGRAM)
static	void	AutoPayment_sub(void);
#endif

extern unsigned short PRG_CPY_S_TM;
extern unsigned short PRG_CPY_R_TM;
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern volatile short TXRXWAIT;
// MH321800(E) G.So IC�N���W�b�g�Ή�


/*[]----------------------------------------------------------------------[]*/
/*| idletask                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : idletask                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hashimoto, modified by Hara                             |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	idletask( void )
{																// �N�����̏���������

// MH810100(S) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
//	switch( read_rotsw() ){										// �����f�[�^�C���X�g�[�����[�h
//		case 5:
//			for( ; ; ) {
//				taskchg( OPETSKNO );										// Change to opration task
//				WACDOG;
//				taskchg( KSGTSKNO );
//				WACDOG;
//				taskchg( FLATSKNO );										// Change to flashrom task
//				WACDOG;
//			}
//			break;
//		case 6:
//			for( ; ; ) {
//				taskchg( OPETSKNO );										// Change to opration task
//				WACDOG;
//			}
//			break;
//	}
// MH810100(E) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)

	for( ; ; ) {
#if (1 == AUTO_PAYMENT_PROGRAM)
		AutoPayment_sub();
#endif
		taskchg( OPETSKNO );										// Change to opration task
		WACDOG;

// MH810100(S) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j

		if( CNMTSK_START ){
			taskchg( CNMTSKNO );									// Change to coin&note control task
			WACDOG;
// MH321800(S) G.So IC�N���W�b�g�Ή�
			while(TXRXWAIT > 0) {
			// phase 3 ���쒆�̓^�C���A�E�g�܂Ő�L����
			// ���R�}���h���M��A���[�_�[�����ACK�҂��^�C���A�E�g�ƂȂ�G���[�o�^����Ă��܂��P�[�X�����邽��
			//   ���̑΍�Ƃ��āAphase 3�̂݃^�X�N���ő�60ms��L����
				taskchg( CNMTSKNO );								// Change to coin&note control task
				WACDOG;
			}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		}

// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//		taskchg( LKCOMTSKNO );										// Change to lkcom task
//		WACDOG;
		taskchg( PKTTSKNO );										// Change to PKT task
		WACDOG;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j

		taskchg(KSGTSKNO);
		WACDOG;

		taskchg( FLATSKNO );										// Change to flashrom task
		WACDOG;
// MH810100(S) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
		taskchg( PRNTSKNO );										// Change to coin&note control task
		WACDOG;
		ntautoSendCtrl();
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		ntautoSendCtrl_pcarsWeb();									//Web�p���ԑ䐔�f�[�^�v��
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		WACDOG;

		if(tcb[CANTCBNO].event == MSG_SET || canevent() != 0){
			taskchg(CANTSKNO);										// Change to can Send Receive task
			WACDOG;
		}
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j
//		cr_snd();													// ذ�ް�ް����M
//
//		taskchg(MRDTSKNO);
//		WACDOG;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j

		if(_is_ntnet_remote() || RAU_Credit_Enabale != 0) {
			taskchg(RAUTSKNO);											// rau task
			WACDOG;
		}
// MH810100(S) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
// 		if( TASK_START_FOR_DL() ){
		// ���u�_�E�����[�h�������Ƀ^�X�N�N����Ԃ�ύX���ꒆ�f���ꂽ�܂܂ƂȂ�P�[�X�����������ׁA
		// �����ł̔������߁Aremotedl_task��taskchg�܂ł͏������I��点��悤�ɂ���B
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
			taskchg( REMOTEDLTSKNO );								// Change to Remote Download task
			WACDOG;
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
// 		}
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
// MH322914(S) K.Onodera 2016/10/12 AI-V�Ή�
//		if (_is_pip()){
		if (_is_Normal_pip()){
// MH322914(E) K.Onodera 2016/10/12 AI-V�Ή�
			taskchg( PIPTSKNO );									// Change to ParkiPro task
			WACDOG;
		}
// MH810100(S) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j
		taskchg( PKTTSKNO );										// Change to PKT task
		WACDOG;
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 �Ԕԃ`�P�b�g���X�iKasago�^�X�N���׌y���j

		if( SHTTER_CTRL ){
			_di();
			SHTTER_CTRL = 0;
			_ei();
			shtctrl();												// ���������
		}
		taskchg( IDLESUBTSKNO );									// Change to idle sub task
		
	}/*for*/
}
//[]----------------------------------------------------------------------[]
///	@brief			idle_subtask
//[]----------------------------------------------------------------------[]
///	@param[in]		none	: 
///	@return			void	: 
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/12<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	idle_subtask( void )
{
// NOTE:�A�C�h���^�X�N�ŃR�[������Ă���T�[�r�X���[�`���̒��ŃG���[�̓o�^���s�����ODB�ɃA�N�Z�X���邽�߂��̎��A
// �A�C�h���^�X�N����A�C�h���^�X�N�փ^�X�N�`�F���W���Ă��܂�����s�\�̂Ȃ邽�߂ɒǉ�����
	for( ; ; ) {
		taskchg( IDLETSKNO );										/* Change task to idletask		*/

		if( TIM500_START ){
			tim500_mon();											// 500ms monitor
		}
		if( FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT ){				// UnKown_Vect�̊��荞�ݏ�������
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_UNKOWNVECT, 2, 0 );		// �G���[�o�^
			FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT = 0;			// UnKown_Vect�̊��荞�݃t���OOFF
		}
		if( FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL ){				//���b�Z�[�W�o�b�t�@�t������
// GG129000(S) R.Endo 2022/12/20 �Ԕԃ`�P�b�g���X4.0 #6765 ���u�_�E�����[�h��������E0011(���b�Z�[�W�L���[�t��)����������ƍċN�����J��Ԃ� [���ʉ��P���� No1539]
// 			err_chk( ERRMDL_MAIN, ERR_MAIN_MESSAGEFULL, 1, 0, 0 );			// ү�����ޯ̧��
// 			FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 0;//���b�Z�[�W�o�b�t�@�t�����o�t���O�N���A
			// ���b�Z�[�W�L���[�t����err_chk�����ŃV�X�e�����Z�b�g���邽�ߐ�Ƀt���O���N���A����
			FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 0;			// ���b�Z�[�W�o�b�t�@�t�����o�t���O�N���A
			err_chk(ERRMDL_MAIN, ERR_MAIN_MESSAGEFULL, 1, 0, 0);	// ���b�Z�[�W�o�b�t�@�t��
// GG129000(E) R.Endo 2022/12/20 �Ԕԃ`�P�b�g���X4.0 #6765 ���u�_�E�����[�h��������E0011(���b�Z�[�W�L���[�t��)����������ƍċN�����J��Ԃ� [���ʉ��P���� No1539]
		}
		I2C_Event_Analize();
		I2C_BusError_Analize();
		Bluetooth_unit();// bluetooth�ʐM�@�\
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| I2C���荞�݃C�x���g����					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : I2C_Event_Analize                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka					                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	I2C_Event_Analize( void )
{
	t_I2C_EVENT	event;
	ulong	ist;													// ���݂̊�����t���
	ushort	msg;

	uchar	rp_err = RP_OV_PA_FR;
	uchar	jp_err = JP_OV_PA_FR;
	
	if( !I2C_Event_Info.USHORT && !RP_OV_PA_FR && !JP_OV_PA_FR ){
		return;
	}
//	event.USHORT = I2C_Event_Info.USHORT;
//	ist = _di2();
	ist = _di2();
	event.USHORT = I2C_Event_Info.USHORT;
	I2C_Event_Info.USHORT = 0;
	RP_OV_PA_FR = 0;
	JP_OV_PA_FR = 0;
	_ei2( ist );
	
	if( event.BIT.RTC_1_MIN_IRQ ){
		RTC_ClkDataReadReq( RTC_READ_QUE_OPE );
	}
// Sc16RecvInSending_IRQ5_Level_L start
//	if( event.BIT.Print1ByteSendReq ){
//		wait2us( 200L );				// 19200(54us x 8 = 432us)
//		wait2us( 100L );				// 19200(54us x 8 = 432us)
//		wait2us( 2000L );				// 19200(54us x 8 = 432us)
//		wait2us( 1500L );				// 19200(54us x 8 = 432us)
//		Sc16SendInI2C_RP();
//		Sc16SendInI2C_JP();
//	}
// Sc16RecvInSending_IRQ5_Level_L end
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//	if( event.USHORT & 0x000e ){
	if( event.USHORT & 0x040e ){
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		if( event.BIT.R_PRINTER_SND_CMP ){
			err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 0, 0, 0 );	// ڼ��������ʐM�ُ����
			if( !rct_init_sts ){
				queset(OPETCBNO, PREQ_SND_COMP_RP, 0, NULL);
			}else{
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���V�[�g�A�W���[�i���r���������P�j
				JP_I2CSndReq( I2C_PRI_REQ );				// ���V�[�g���M�ŕۗ����ꂽ���M���ĊJ
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���V�[�g�A�W���[�i���r���������P�j
			}
		}
		
		if( event.BIT.R_PRINTER_RCV_CMP ){
			if( event.BIT.R_PRINTER_NEAR_END ){
				msg = PREQ_JUSHIN_ERR2;
			}else{
				msg = PREQ_JUSHIN_ERR1;
			}
			MsgSndFrmPrn(	msg,
							R_PRI,
							0 );
		}
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		if (event.BIT.R_PRINTER_RCV_INFO) {
			// �v�����^��񉞓���M
			PrnCmd_InfoRes_Msg_R();
		}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	}
	
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//	if( event.USHORT & 0x0070 ){
	if( event.USHORT & 0x0370 ){
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		if( event.BIT.J_PRINTER_SND_CMP ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			err_chk(ERRMDL_EJA, ERR_EJA_COMFAIL, 0, 0, 0);	// �ʐM�s�ǉ���
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 0, 0, 0 );	// �ެ���������ʐM�ُ����
			if( !jnl_init_sts ){
				queset(OPETCBNO, PREQ_SND_COMP_JP, 0, NULL);
			}else{
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���V�[�g�A�W���[�i���r���������P�j
			// �d�q�W���[�i���ւ̐���R�}���h���M�Ń��V�[�g�󎚃f�[�^�̑��M���u���b�N����邱�Ƃ����邽�߁A
			// �W���[�i�����M�ヌ�V�[�g���c���f�[�^������Α��M����B
			// �i���V�[�g�ƃW���[�i����pritask�Ŕr���Ǘ����ē����Ƀf�[�^�͔������Ȃ��j
				RP_I2CSndReq( I2C_PRI_REQ );				// �W���[�i�����M�ŕۗ����ꂽ���M���ĊJ
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���V�[�g�A�W���[�i���r���������P�j
			}
		}
		
		if( event.BIT.J_PRINTER_RCV_CMP ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			if (event.BIT.EJA_RCV_STS) {
				msg = PREQ_JUSHIN_INFO;
			}else
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			if( event.BIT.J_PRINTER_NEAR_END ){
				msg = PREQ_JUSHIN_ERR2;
			}else{
				msg = PREQ_JUSHIN_ERR1;
			}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			if (!jnl_init_sts) {
				queset(OPETCBNO, msg, 0, NULL);
			}
			else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			MsgSndFrmPrn(	msg,
							J_PRI,
							0 );
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (event.BIT.J_PRINTER_RCV_INFO) {
			// �v�����^��񉞓���M
			PrnCmd_InfoRes_Msg();
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	
	if( rp_err ){
		queset( OPETCBNO, I2C_RP_ERR_EVT, 1, &rp_err );	// ���C���ɒʒm
	}
	
	if( jp_err ){
		queset( OPETCBNO, I2C_JP_ERR_EVT, 1, &jp_err );	// ���C���ɒʒm
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| I2C_Bus�G���[����						                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-03-26                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	I2C_BusError_Analize( void )
{
	t_I2C_BUSERROR		Error;
	ulong				ist;

	ist = _di2();
	Error.USHORT = I2C_BusError_Info.USHORT;
	//	�ȉ���5�̔�������������Bit�� <--- 0����
	I2C_BusError_Info.BIT.I2C_PREV_TOUT = 0;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
	I2C_BusError_Info.BIT.I2C_BUS_BUSY = 0;			// Bit 3 = �o�XBUSY						������������
	I2C_BusError_Info.BIT.I2C_SEND_DATA_LEVEL = 0;	// Bit 2 = ���M�f�[�^���x���G���[		������������
	I2C_BusError_Info.BIT.I2C_CLOCK_LEVEL = 0;		// Bit 1 = �N���b�N���x���G���[			������������ 
	I2C_BusError_Info.BIT.I2C_R_W_MODE = 0;			// Bit 0 = Read/Write���[�h�G���[		������������
	_ei2( ist );

	if (Error.USHORT & 0x0001) {
		err_chk(ERRMDL_I2C, ERR_I2C_R_W_MODE, 2, 0, 0);			// Read/Write���[�h�G���[		������������
	}
	if (Error.USHORT & 0x0002) {
		err_chk(ERRMDL_I2C, ERR_I2C_CLOCK_LEVEL, 2, 0, 0);		// �N���b�N���x���G���[			������������
	}
	if (Error.USHORT & 0x0004) {
		err_chk(ERRMDL_I2C, ERR_I2C_SEND_DATA_LEVEL, 2, 0, 0);	// ���M�f�[�^���x���G���[		������������
	}
	if (Error.USHORT & 0x0008) {
		err_chk(ERRMDL_I2C, ERR_I2C_BUS_BUSY, 2, 0, 0);			// �o�XBUSY						������������
	}
	if (Error.USHORT & 0x0010) {
		err_chk(ERRMDL_I2C, ERR_I2C_PREV_TOUT, 2, 0, 0);		// �O��Status�ω��҂�Timeout	������������
	}

	//	�ȉ���4�͔���/�����ʂȂ̂�Bit�� <--- 0���Ȃ���<- 1���Ă���ӏ���Error���������Ȃ�����<- 0����
	//	�܂�0 -> 1, 1 -> 0�̕ω������������̂�err_chk()��call����
	if(I2cErrEvent.SendRP == 1) {
		I2cErrEvent.SendRP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_SEND_RP, Error.BIT.I2C_BFULL_SEND_RP, 0, 0);	// ���V�[�g�v�����^���M�o�b�t�@�t��(�wI2Cڼ�đ��M�ޯ̧�فx)		����/������
	}
	if(I2cErrEvent.RecvRP == 1) {
		I2cErrEvent.RecvRP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_RECV_RP, Error.BIT.I2C_BFULL_RECV_RP, 0, 0);	// ���V�[�g�v�����^��M�o�b�t�@�t��(�wI2Cڼ�Ď�M�ޯ̧�فx)		����/������
	}
	if(I2cErrEvent.SendJP == 1) {
		I2cErrEvent.SendJP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_SEND_JP, Error.BIT.I2C_BFULL_SEND_JP, 0, 0);	// �W���[�i���v�����^���M�o�b�t�@�t��(�wI2C�ެ��ّ��M�ޯ̧�فx)	����/������
	}
	if(I2cErrEvent.RecvJP == 1) {
		I2cErrEvent.RecvJP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_RECV_JP, Error.BIT.I2C_BFULL_RECV_JP, 0, 0);	// �W���[�i���v�����^��M�o�b�t�@�t��(�wI2C�ެ��َ�M�ޯ̧�فx)	����/������
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| RTC�����i���荞�ݔ���/���v�f�[�^�擾����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RTC_ClkDataReadReq                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka					                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	RTC_ClkDataReadReq( short que )
{
	ushort					ret;
	I2C_REQUEST 			request;
	unsigned char	BcdClock[11];
	extern	void ChgTimedata_RTCBCD( struct	RTC_rec *Time, unsigned char *BCDTime);
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;

	request.TaskNoTo	 			= I2C_TASK_OPE;					// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;					// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_TIME_RCV;			// 
	request.I2cReqDataInfo.RWCnt	= RTC_RW_SIZE;					// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.freq100sec;		// 

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 
	if( ret == RIIC_RSLT_OK ){
		ChgTimedata_RTCBCD(&RTC_CLOCK, BcdClock);
		// ms��LifeTime���
		u_LifeTime.ul = LifeTim2msGet();
		BcdClock[7] = u_LifeTime.uc[0];
		BcdClock[8] = u_LifeTime.uc[1];
		BcdClock[9] = u_LifeTime.uc[2];
		BcdClock[10] = u_LifeTime.uc[3];
		//
		if( que == RTC_READ_QUE_OPE ){
			queset( OPETCBNO, CLOCK_CHG, 11, BcdClock );
		}
		op_RtcFlagRegisterRead();									//flag���W�X�^�擾�v��
	}
	return	ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 500ms�Ď�����                                                          |*/
/*| ���޽/׸���ъĎ��A�װ/�װъĎ��A���CPU���i�Ď�                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : tim500_mon                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hashimoto, modified by Hara                             |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	tim500_mon( void )
{
	uchar	i;
	uchar	saveTIM500_START;
	static uchar on_off = 0;

	_di();
	saveTIM500_START = TIM500_START;
	TIM500_START = 0;
	_ei();

	// �v���O�����܂��͉����f�[�^�����鎞�͒ʏ퓮���500ms�Ԋu�œ_�ł���
	on_off ^= 1;
	ExIOSignalwt(EXPORT_M_LD1, on_off );// ST1 LED��500ms�Ԋu�œ_��(�v���O�����L������)
	if(f_wave_datchk != WAVE_AREA_NODATA) {// �����f�[�^�����݂���ꍇ
		ExIOSignalwt(EXPORT_M_LD2, on_off );// ST2 LED��500ms�Ԋu�œ_��
	} else {// �����f�[�^���������͏���
		ExIOSignalwt(EXPORT_M_LD2, 0 );// ST2 LED������
	}
	for( i=0; i < saveTIM500_START; ++i ){							// �O�񂩂�o�߂������ԕ�

		/* 500ms �֐�Call�^ Lagtim timeout process */
		LagChk500ms();

		/* ���޽��сA׸���ъĎ� */
		SrvTimMng();

		/* ���CPU�Ď����� */
		SUBCPU_MONIT++; 											// ���CPU�Ď������X�V

		if( PassIdBackupTim > 0 ){									// n�������p��ϰ�N��
			PassIdBackupTim--;
		}

		if( RECI_SW_Tim > 0 ){										// �̎��ؔ��s�\��ϰ
			RECI_SW_Tim--;
			if( RECI_SW_Tim == 0 ){
				queset( OPETCBNO, MID_RECI_SW_TIMOUT, 0, NULL );	// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
			}
		}

		if( SUBCPU_MONIT > 360 ){									// 3���o��?
			WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
			_di();
			err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 1, 0, 0 );		// �����i�Ȃ�
			SUBCPU_MONIT = 0;
			if( timerd( &CLK_REC ) != 0 ){
				ex_errlg( ERRMDL_MAIN, ERR_MAIN_CLOCKREADFAIL, 2, 0 );		// ���v�ǂݍ��ݎ��s
			}
			_ei();
			dog_init();
		}
	}

	// 5sec�����ɏo�͐M�������������s��	*/
	SIGCHK_TRG++;
	if( SIGCHK_TRG > 10 ){

		SIGCHK_TRG = 0;
		/* ��؊Ď� */
		memorychk();
		/* �M���o������ */
		sig_chk();

		LkErrTimMng();
	}
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
	// 3sec�����ɒ������Ԃ̃`�F�b�N(�h�A�A���[�����M�p)���s��
	LONG_PARK_COUNT2++;
	if( LONG_PARK_COUNT2 > 6 ){
		LONG_PARK_COUNT2 = 0;
		LongTermParkingCheck_Resend();		// �������ԃ`�F�b�N(�h�A���ɃZ���^�[�ɑ΂��đ��M���鏈��)
	}
	// 10sec�����ɒ������Ԃ̃`�F�b�N(�A���[���f�[�^)���s��
	LONG_PARK_COUNT++;
	if( LONG_PARK_COUNT > 20 ){
		LONG_PARK_COUNT = 0;
		if(f_LongParkCheck_resend == 0){
			// �h�A�A���[�����M�������s���Ă��Ȃ��Ƃ��Ƀ`�F�b�N����
			LongTermParkingCheck();			// �������Ԍ��o(�A���[���f�[�^�ɂ��ʒm)
		}
	}
	
	// 10sec�����ɒ������Ԃ̃`�F�b�N(�������ԃ��O)���s��
	LONG_PARK_COUNT3++;
	if( LONG_PARK_COUNT3 > 20 ){
		LONG_PARK_COUNT3 = 0;
		LongTermParkingCheck_r10_prmcng();		// �ݒ�ύX�ɂ�鋭����������
		if(LongPark_Prmcng.f_prm_cng == 0){
			// �ݒ�ύX�ɂ�鋭�������������s���Ă��Ȃ��Ƃ��Ƀ`�F�b�N����
			LongTermParkingCheck_r10();			// �������Ԍ��o(�������ԃf�[�^�ɂ��ʒm)
		}
	}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810102(S) �d�q�}�l�[�Ή� #5329 ���Z�@�̓d����OFF/ON�������AE2615-265����������
	WAKEUP_COUNT++;								// �N���^�C�}�J�E���^
	if( WAKEUP_COUNT > 120){					// 60Sec
		WAKEUP_COUNT = 0;
		WAKEUP_OVER = 1;						// �N���ς݃t���O
	}
// MH810102(E) �d�q�}�l�[�Ή� #5329 ���Z�@�̓d����OFF/ON�������AE2615-265����������

// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
//// �����A�����Z
//#if (1 == AUTO_PAYMENT_PROGRAM)
//	// 1sec�����Ɏ����A�����Z�������s��
//	if (OPECTL.Seisan_Chk_mod == ON) {
//		AutoPayment.AUTOPAY_TRG++;
//		if (AutoPayment.AUTOPAY_TRG > 1) {
//			AutoPaymentIn(&(AutoPayment.In));		// ���ɏ���
//			AutoPaymentOut(&(AutoPayment.Out));		// ���Z����
//			AutoPayment.AUTOPAY_TRG = 0;
//		}
//	}
//#endif
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z

}


/*[]----------------------------------------------------------------------[]*/
/*| tcb������                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Tcb_Init                                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*| Update       : 2002-07-30 M.Okuda                                      |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	Tcb_Init( void )
{
	short	i;

	for(i=0; i<TCB_CNT; ++i){
		tcb[i].event = MSG_EMPTY;
		tcb[i].level = 0;
		tcb[i].status = 0;
		tcb[i].msg_top = NULL;
		tcb[i].msg_end = NULL;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| mcb������                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Mcb_Init                                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	Mcb_Init( void )
{
	short	i;

	for( i=0; i < (MSGBUF_CNT-1); i++ ){
		msgbuf[i].msg_next = (char *)&msgbuf[i+1];
	}
	msgbuf[i].msg_next = (char *)NULL;

	mcb.msg_top = &msgbuf[0];
	mcb.msg_end = &msgbuf[MSGBUF_CNT-1];
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Mү�����ޯ̧�擾                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetBuf( void )                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ү�����ޯ̧�߲��,���擾��NULL                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*GetBuf( void )
{
	MsgBuf	*msg_add;
	ulong	ist;													// ���݂̊�����t���
// GG124100(S) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
#if DEBUG_MESSAGEFULL
	static ulong	prev_buf_count = 0;
	ulong	buf_count = 0;
	ulong	msg_count = 0;
	ulong	max_msg_count = 0;
	ulong	data = 0;
	uchar	id = 0;
	uchar	max_id = 0;

	ist = _di2();

	// �擾�\�ȃo�b�t�@���J�E���g
	msg_add = mcb.msg_top;
	while ( msg_add ) {
		msg_add = (MsgBuf*)msg_add->msg_next;
		buf_count++;
	}

	// �g�p�ʂ̑����^�X�NID���m�F
	for ( id = 0; id < TCB_CNT; id++ ) {
		msg_add = tcb[id].msg_top;
		while ( msg_add ) {
			msg_add = (MsgBuf*)msg_add->msg_next;
			msg_count++;
		}
		if ( msg_count > max_msg_count ) {
			max_msg_count = msg_count;
			max_id = id;
		}
	}

	_ei2( ist );

	// �A���[���ɐݒ肷������쐬(4��������3���F�擾�\�ȃo�b�t�@�̕ω���)
	if ( prev_buf_count >= buf_count ) {
		data = prev_buf_count - buf_count;
	} else {
		data = buf_count - prev_buf_count;
	}
	prev_buf_count = buf_count;

	// �A���[���ɐݒ肷������쐬(4�������1���F�g�p�ʂ̑����^�X�NID)
	data += (max_id * 1000);

	// �擾�\�ȃo�b�t�@��100�����ł���΃A���[���𔭐�
	if ( buf_count < 100 ) {
		alm_chk2(DEBUG_ALARM_MD, DEBUG_ALARM_NO,
			1, 2, 0, (void *)&data);	// ����
	}

	// �擾�\�ȃo�b�t�@��150�ȏ�ł���΃A���[��������
	if ( buf_count >= 150 ) {
		alm_chk2(DEBUG_ALARM_MD, DEBUG_ALARM_NO,
			0, 2, 0, (void *)&data);	// ����
	}
#endif
// GG124100(E) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]

	if( mcb.msg_top != NULL ){
		ist = _di2();
		msg_add = mcb.msg_top;
		mcb.msg_top = (MsgBuf*)mcb.msg_top->msg_next;
		_ei2( ist );
		return( msg_add );
	}else{
		FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 1;//���b�Z�[�W�o�b�t�@�t�����o
		return( NULL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ү���ޑ��M                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PutMsg( id, msg )                                       |*/
/*| PARAMETER    : id = ���M������ް                                       |*/
/*|                msg = ���Mү�����߲��                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	PutMsg( unsigned char id, MsgBuf *msg )
{
	ulong	ist;													// ���݂̊�����t���

	if( id >= TCB_CNT ){
		FreeBuf( msg );
		return;
	}
	ist = _di2();
	tcb[id].event = MSG_SET;
	if( tcb[id].msg_end != NULL ){
		tcb[id].msg_end->msg_next = (char*)msg;
	}
	msg->msg_next = NULL;
	tcb[id].msg_end = msg;
	if( tcb[id].msg_top == NULL ){
		tcb[id].msg_top = msg;
	}
	_ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| ��ү���ގ擾                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMsg( id )                                            |*/
/*| PARAMETER    : id = �擾������ް                                       |*/
/*| RETURN VALUE : �擾ү�����߲��,�擾ү���ނ��Ȃ����NULL                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*GetMsg( uchar id )
{
	MsgBuf	*msg_add;
	ulong	ist;													// ���݂̊�����t���

	if( tcb[id].msg_top != NULL ) {
		ist = _di2();
		msg_add = tcb[id].msg_top;
		tcb[id].msg_top = (MsgBuf *)tcb[id].msg_top->msg_next;
		if( tcb[id].msg_top == NULL ) {
			tcb[id].msg_end = NULL;
			tcb[id].event = MSG_EMPTY;
		}
		_ei2( ist );
		return( msg_add );
	}else{
		return( NULL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ү�����ޯ̧���                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FreeBuf( msg )                                          |*/
/*| PARAMETER    : msg = �������ү�����ޯ̧���߲��                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
FreeBuf( msg )
MsgBuf	*msg;
{
	ulong	ist;													// ���݂̊�����t���

	ist = _di2();
	mcb.msg_end->msg_next = (char*)msg;
	msg->msg_next = NULL;
	mcb.msg_end = msg;
	if( mcb.msg_top == NULL ){
		mcb.msg_top = msg;
	}
	_ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| ү���ނ���M����܂�WAIT������                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMsgWait                                              |*/
/*| PARAMETER    : id  = ��M�҂�����ү���އ�(TCB)                         |*/
/*|                msg = ��M����ү����                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void GetMsgWait( uchar id, MsgBuf *msg )
{
	MsgBuf	*rcv;

	for( ; ; )
	{
		taskchg( IDLETSKNO );										// Change task to idletask
		if( (rcv = GetMsg( id )) != NULL )
		{
			memcpy( msg , rcv , sizeof(MsgBuf) );
			FreeBuf( rcv );
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Ұٷ������_����Ұق��擾����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : id = �擾������ް                                       |*/
/*|				   pReq = �����Ώ�ү����ID�i�ő� TARGET_MSGGET_PRM_MAX �j|*/
/*| RETURN VALUE : �擾ү�����߲��,�擾ү���ނ��Ȃ����NULL                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda												   |*/
/*| Date         : 2006/06/22                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*Target_MsgGet( uchar id, t_TARGET_MSGGET_PRM *pReq )
{
	MsgBuf	*msg_add;										// Ұٷ���������Ώ�Ұٱ��ڽ��Ĵر
	ushort	MailCommand;									// Ұٷ���������Ώ�Ұق�ү����ID
	ushort	ReqCount;										// �v�����ꂽ�����Ώ�ү����ID��
	ulong	ist;											// ���݂̊�����t���
	uchar	i,j;
	MsgBuf	*msg_add_Prev;									// Ұٷ����(1��O)�����Ώ�Ұٱ��ڽ��Ĵر

	if( tcb[id].msg_top == NULL ) {							// ҰقȂ�
		return( NULL );
	}

	ReqCount = pReq->Count;									// �v�����ꂽ�����Ώ�ү����ID��get

	ist = _di2();											// �����݋֎~
	msg_add = tcb[id].msg_top;								// �����Ώ�Ұٱ��ڽget�i�ŏ��͐擪�j

	// ��������
	for( i=0; i<MSGBUF_CNT; ++i ){							// Ұٷ���� �SҰ������iMSGBUF_CNT�͒P��Limitter�j

		MailCommand = msg_add->msg.command;					// �����Ώ�Ұق�ү����IDget

		for( j=0; j<ReqCount; ++j ){						// �T���ė~����Ұ�ID����
			if( pReq->Command[j] == MailCommand ){			// ����
				goto Target_MsgGet_10;						// ������Ұق𷭰���甲��
			}
		}

		// ����Ұقցi���������j
		msg_add_Prev = msg_add;								// �OҰٱ��ڽ���ށi�������ɕK�v�j
		msg_add = (MsgBuf*)(msg_add->msg_next);				// �������Ώ�Ұٱ��ڽget
		if( NULL == msg_add ){								// ��ҰقȂ�
			break;											// �����I��
		}

		// 16���1�� WDT�ر
		if( 0x0f == (i & 0x0f) ){
			WACDOG;
		}
	}

	// �����ł��Ȃ������ꍇ�i�c��ҰقȂ��j
	_ei2( ist );
	return( NULL );

	// ���������ꍇ
Target_MsgGet_10:
	// Target message�𷭰���甲��

	if( 0 == i ){											// �擪Ұق̏ꍇ
		tcb[id].msg_top = (MsgBuf *)tcb[id].msg_top->msg_next;
		if( tcb[id].msg_top == NULL ) {						// Ұق�1�������Ȃ������ꍇ
			tcb[id].msg_end = NULL;
			tcb[id].event = MSG_EMPTY;
		}
	}
	else if( tcb[id].msg_end == msg_add ){					// ����Ұق̏ꍇ�i2���ȏ�Ұق�����j
		msg_add_Prev->msg_next = NULL;						// �ŏIҰ�ϰ�set
		tcb[id].msg_end = msg_add_Prev;
	}
	else{													// �r��Ұق̏ꍇ�i2���ȏ�Ұق�����j
		msg_add_Prev->msg_next = msg_add->msg_next;			// �ŏIҰ�ϰ�set
	}

	_ei2( ist );
	return( msg_add );
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
/**
 *	���[���L���[����_�������[�����P�����폜����
 *
 *	@param[in]	task_id		�^�X�N�h�c
 *	@param[in]	message_id	���b�Z�[�W�h�c
 *	@retval	NULL		�w�肵�����[���͑��݂��Ȃ�����
 *	@retval	NULL�ȊO	�폜�������[���f�[�^�ւ̃|�C���^
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@see	Target_MsgGet()
 *	@date	2009/11/12(��)
 *	@attention	�Ԓl��NULL�ȊO���Ԃ����ꍇ�ł��A���̃|�C���^���w���̈�̓��[�����폜�ς݂Ȃ̂ŁA
 *				�Ԓl�̓��[�����폜�������ǂ����̔���ɂ̂ݎg�p���邱�ƁB
 */
const MsgBuf *Target_MsgGet_delete1( const unsigned char task_id, const unsigned short message_id )
{
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	MsgBuf	*msb;

	// ��M���������b�Z�[�W�h�c������
	Target_WaitMsgID.Count = 1;
	Target_WaitMsgID.Command[0] = message_id;

	msb = Target_MsgGet( task_id, &Target_WaitMsgID );	// ���҂��郁�[��������M�i���͗��߂��܂܁j
	if( NULL != msb ){
		FreeBuf( msb );		// �ړI�̃��[������M���b�Z�[�W�o�b�t�@����폜����
	}

	return msb;
}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)

/*[]----------------------------------------------------------------------[]*/
/*| �w�莞�������WAIT������                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause                                                  |*/
/*| PARAMETER    : WaitTime = �҂����� (10ms unit value)                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hara                                                    |*/
/*| Date         : 2005-01-24                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause( ulong WaitTime )
{
	ulong	StartTime;

	xPauseStarting = 1;
	StartTime = LifeTimGet();
	for( ;; ){
		taskchg( IDLETSKNO );										// ����ؑ�
		if( WaitTime == 0 ){	// �҂����Ԃ�0ms�Ŏw�肳�ꂽ�ꍇ�ɂ́Aidletask���P�������珈����߂��B
			break;
		}
		
		if( WaitTime <= LifePastTimGet( StartTime ) ){
			break;
		}
	}
	xPauseStarting = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �w�莞�������WAIT������i�v�����^�^�X�N��p�j                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause_PRNTSK                                           |*/
/*| PARAMETER    : WaitTime = �҂����� (10ms unit value)                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                 |*/
/*| Date         : 2007-05-31                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause_PRNTSK( ulong WaitTime )
{
	ulong	StartTime;

	// xPause�ŵ��������N��������
	xPauseStarting_PRNTSK = 1;
	StartTime = LifeTimGet();
	for( ;; ){
		taskchg( IDLETSKNO );										// ����ؑ�
		if( WaitTime == 0 ){	// �҂����Ԃ�0ms�Ŏw�肳�ꂽ�ꍇ�ɂ́Aidletask���P�������珈����߂��B
			break;
		}
		if( WaitTime <= LifePastTimGet( StartTime ) ){
			break;
		}
	}
	xPauseStarting_PRNTSK = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �w�莞�������WAIT������                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause                                                  |*/
/*| PARAMETER    : WaitTime = �҂����� (2ms unit value)                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : akiba                                                   |*/
/*| Date         : 2005-01-24                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause2ms( ulong WaitTime )
{
	ulong	StartTime;

	xPauseStarting = 1;
	StartTime = LifeTim2msGet();
	do{
		taskchg( IDLETSKNO );										// ����ؑ�
	}while( 0 == LifePastTim2msGet(StartTime, WaitTime) );
	xPauseStarting = 0;
}

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| 5sec�����Ɏ����A�����ɏ������s��                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AutoPaymentIn                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void AutoPaymentIn(t_AutoPayment_In *In)
{
	char i = 0;
	ushort offset = INT_CAR_START_INDEX;

	// 5sec��1�����
	if (++(In->count) < 5) {
		return;
	}
	In->count = 0;

	// �󂫎Ԏ�������
	while (i++ < MAX_AUTO_PAYMENT_NO) {	// ������܂őS�Ԏ����
		if (In->index >= (ushort)MAX_AUTO_PAYMENT_NO) {
			In->index = 0;
			offset = (ushort)INT_CAR_START_INDEX;
		}
		if (In->index > (ushort)MAX_AUTO_PAYMENT_CAR) {
			offset = (ushort)(BIKE_START_INDEX - INT_CAR_LOCK_MAX);	// �Ԏ����Ұ� ���֎w��p�̾��
		}
		if (FLAPDT.flp_data[In->index + offset].nstat.bits.b00 == 0) {
			// �ԗ��̂��Ȃ��Ԏ��𔭌� �� �ׯ��/ۯ����u��а������ɂ�ʒm
			if(LKcom_Search_Ifno(LockInfo[In->index + offset].if_oya) == 0) {	// ���b�N�̏ꍇ
				LK_AutoPayment_Rcv(In->index, LockInfo[In->index + offset].if_oya, 
									LockInfo[In->index + offset].lok_no, AUTOPAY_STS_IN);
			} else {															// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
				LK_AutoPayment_Rcv(In->index, LockInfo[In->index + offset].if_oya, 
									1, AUTOPAY_STS_IN);
			}			
			(In->index)++;
			break;
		}
		(In->index)++;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����A�����Z�������s��                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AutoPaymentOut                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Takahashi                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void AutoPaymentOut(t_AutoPayment_Out *Out)
{
	ushort i, max;
	ushort no;
	ushort cnt;
	char   onof;
	
	switch (Out->mode) {
	case 0:
		// �����e�i���X���łȂ��A�ҋ@��Ԃł���ꍇ
		if ((OPECTL.Mnt_mod == 0) && (OPECTL.Ope_mod == 0) && (AppServ_IsLogFlashWriting(eLOG_PAYMENT) == 0) ) {
			cnt = 0;
			no  = (ushort)-1;
			max = (ushort)TOTAL_CAR_LOCK_MAX;
			for(i = 0; i < max; ++i) {
				if(FLAPDT.flp_data[i].mode == FLAP_CTRL_MODE4) {
					++cnt;
				}
			}
			if(cnt < 5) {
				return;
			}
			i   = (ushort)Out->next_no;

			while (i < max) {
				if (FLAPDT.flp_data[i].mode == FLAP_CTRL_MODE4) {
					if (no == (ushort)-1) {
						no = i + (ushort)1;
					}
					if(cnt >= 5) {
						break;
					}
				}
				if (++i == max) {
					i   = 0;
					max = Out->next_no;
				}
			}

			// ���ԑ䐔���T��ȏ�H
			if (cnt >= 5) {
				if(no <= TOTAL_CAR_LOCK_MAX) {
					no -= INT_CAR_START_INDEX;
				}
				else if (no > BIKE_START_INDEX && no <= (BIKE_START_INDEX + BIKE_LOCK_MAX)) {
					no -= BIKE_START_INDEX;
				}
				else {
					break;
				}
				Out->proc_no = no - 1;
				onof = 1;
				
				// ���Ԉʒu�ԍ��̃e���L�[�C�x���g��ʒm
				queset( OPETCBNO, (unsigned short)((no/100)+KEY_TEN0), 1, &onof );	// �P����
				no %= 100;
				queset( OPETCBNO, (unsigned short)((no/10)+KEY_TEN0), 1, &onof ); 	// �Q����
				queset( OPETCBNO, (unsigned short)((no%10)+KEY_TEN0), 1, &onof );	// �R����
				
				Out->mode = 1;
			}
		}
		break;
	case 1:
		// ���Ԉʒu�ԍ�����
		if (OPECTL.Ope_mod == 1) {
			// �U���Z�L�[�C�x���g�ʒm
			onof = 1;
			queset( OPETCBNO, KEY_TEN_F1, 1, &onof );
			Out->mode = 2;
		}
		break;
	case 2:
		// ������
		if (OPECTL.Ope_mod == 2) {
			ryo_buf.in_coin[2] = 1;		// �S�~�d�݂��P�������Ă��鎖�ɂ���
			
			// �U�R�C�����b�N�C�x���g�ʒm
			queset( OPETCBNO, COIN_IN_EVT, 0, NULL );	// ��������
			queset( OPETCBNO, COIN_EN_EVT, 0, NULL );	// ���o�����
			
			Out->mode = 0;
			Out->next_no = Out->proc_no + 1;
		}
		else {
			Out->mode = 0;
		}
		break;
	default:
		break;
	}
}

#endif

#if (1 == AUTO_PAYMENT_PROGRAM)
	static void	Debug_ErrAlm_OnOff( void );
	static void	Debug_LogFull( void );

static	void	AutoPayment_sub()
{
	Debug_ErrAlm_OnOff();
	if( CPrmSS[S_SYS][5] == 2 )
		Debug_LogFull();
}

//------------------------------------------------------------------------------
//	���ʇ�01-0006��ύX���邱�Ƃɂ��A�װ,�װт̔����������s����悤�ɂ���B
//
//	01-0006=�@�A�B�C�D�E
//
//	�@�F0=�װ�C1=�װсC2=����C3=�������
//	�A�B�C�D�F���ށiNT-NET���x���̺��ށj
//	�E�F0=�����C1=�����C2=��������
//
//	��jE0120���� �� 001201
//		A0102���� �� 101020
//		E0070����&���� �� 000702
//
//	�P����s��̓v���O�����łO�N���A����B
//------------------------------------------------------------------------------
static void	Debug_ErrAlm_OnOff( void )
{
	extern	ulong	FLT_WriteParam1(uchar flags);

	char	md, no, knd;
	char	f_Change=0;
	ushort	MonCode;

	if( CPrmSS[S_SYS][6] == 0 ){					// �w������
		return;
	}

	md = (char)prm_get( COM_PRM,S_SYS,6,2,4 );			// ���
	no = (char)prm_get( COM_PRM,S_SYS,6,2,2 );			// �ԍ�
	knd = (char)prm_get( COM_PRM,S_SYS,6,1,1 );			// �ð��
	MonCode = (ushort)prm_get( COM_PRM,S_SYS,6,4,2 );

	switch( (ushort)prm_get( COM_PRM,S_SYS,6,1,6 ) ){	// ��ʂŕ���
	case	0:	// �װ
		err_chk( md, no, knd, 0, 0 );
		f_Change = 1;
		break;

	case	1:	// �װ�
		alm_chk( md, no, knd );
		f_Change = 1;
		break;

	case	2:	// ���
		wmonlg( MonCode, 0, 0 );
		f_Change = 1;
		break;

	case	3:	// �������
		wopelg( MonCode,0,0 );
		f_Change = 1;
		break;

	default:
		return;
	}

	if(	f_Change ){
		CPrmSS[S_SYS][6] = 0;
		(void)FLT_WriteParam1( 0 /*FLT_NOT_EXCLUSIVE*/ );	// FlashROM update
	}
}

/*----------------------------------------------------------*/
/*	���Z���~�C���K�Ǘ��C���&����LOG��Full���ް���o�^����	*/
/*	01-0005=2 �Ŏ��{�B										*/
/*----------------------------------------------------------*/
static void	Debug_LogFull( void )
{

}
#endif
