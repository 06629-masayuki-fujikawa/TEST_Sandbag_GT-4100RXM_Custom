/*[]----------------------------------------------------------------------[]*/
/*| JVMA Communication Control											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019.02.07                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"suica_def.h"
#include	"ec_def.h"

//---------------------------------------------------------------------------
// Production Note
// suica_ctrl.c�Ɏ���������Ȃ����߁A���\�[�X�����~���ɍ�蒼�������̂ł��B
// ���̂��߁A�d������@�\�͈�t����܂��B
//---------------------------------------------------------------------------
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//void	jvma_init( void );
//void	jvma_trb( unsigned char err_kind );
//void	jvma_event( void );
//
//void	jvma_command_set( void );
//short	jvma_act( void );
// MH810103 GG119202(E) JVMA���Z�b�g�����s�

// local
void	jvma_nop( void );
void	jvma_event_ex( void );
short	jvma_phase_01( void );
short	jvma_phase_02( void );
short	jvma_phase_03( void );
short	jvma_phase_04( void );
uchar 	jvma_send_que_check( void );

/*[]----------------------------------------------------------------------[]*/
/*| JVMA Line Initialize                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : jvma_init                                               |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_init( void )
{
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
	Ec_Jvma_Reset_Count = 0;											// ���Z�b�g�񐔂��N���A
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
	memset(&jvma_setup, 0, sizeof(jvma_setup));
	jvma_setup.init = jvma_nop;
	jvma_setup.event = jvma_nop;
	jvma_setup.recv = (short (*)(uchar*, short))jvma_nop;
	jvma_setup.log_regist = Suica_Log_regist;
	jvma_setup.type = 0;
	jvma_setup.type = (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 3);		// SX-20�ڑ��ݒ�擾
// MH810103 GG119202(S) �ڑ��ݒ�Q�Ə������P
	// 02-0024�C�́A�ڑ��Ȃ�:0�A�ڑ�����:1�̂ݗL��
	if( jvma_setup.type != 1 ){
		jvma_setup.type = 0;
	}
// MH810103 GG119202(E) �ڑ��ݒ�Q�Ə������P
	if(prm_get(COM_PRM, S_PAY, 24, 1, 6) != 0) {
		if(jvma_setup.type == 0) {
			jvma_setup.type = 4;	// ���σ��[�_�L��
		}
	}
	switch(jvma_setup.type) {
	case	0:
	default:
		return;
	case	1:
// MH810103 GG119202(S) �ڑ��ݒ�Q�Ə������P
//	�{�^���t���Ȃ����߁A�폜
//	case	2:		// �x���{�^���t��
// MH810103 GG119202(E) �ڑ��ݒ�Q�Ə������P
		jvma_setup.mdl = ERRMDL_SUICA;
		jvma_setup.init = suica_init;
		jvma_setup.event = suica_event;
		jvma_setup.recv = suica_recv;
		break;
	case	4:
		jvma_setup.event = jvma_event_ex;
		if (jvma_init_tbl[jvma_setup.type] == NULL) {
			return;
		}
		jvma_setup.init = (void (*)(void))jvma_init_tbl[jvma_setup.type];
// MH810103 GG119202(S) �N������������
		// �N�����̏������������ł��s���B
		ec_flag_clear(1);							// �֘A�t���O������
// MH810103 GG119202(E) �N������������
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Trouble   	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicatrb                                                |*/
/*| PARAMETER	 : err_kind:�װ���							               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_trb( unsigned char err_kind )
{
	// �������\��ς݁H or ���������H
	if (Suica_Rec.Data.BIT.RESET_RESERVED ||
		!Suica_Rec.Data.BIT.INITIALIZE) {
		return;
	}
	if( err_kind == ERR_SUICA_COMFAIL ){									/* �װ��ʂ��ʐM�s�ǂ̏ꍇ */
		err_suica_chk(  &suica_errst ,&suica_errst_bak,ERR_SUICA_COMFAIL );	/* �װ���� */
		Suica_Rec.suica_err_event.BIT.COMFAIL = 1;							/* �ʐM�s�Ǵװ�׸�ON */
		suica_errst_bak = suica_errst;										/* �װ��Ԃ��ޯ����� */
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
		if (isEC_USE()) {
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
		Suica_Rec.Data.BIT.CTRL_CARD = 0;									// �J�[�h�Ȃ��Ƃ���
// MH810103 GG119202(S) �N���V�[�P���X��������������
//		Suica_Rec.Data.BIT.BRAND_STS_RCV = 0;								// Brand Status Receive
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																			// �u�����h�l�S�V�G�[�V�����Ǘ����N���A
// MH810103 GG119202(E) �N���V�[�P���X��������������
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
		EcEnableNyukin();
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;								// �u�����h�I���́u�߂�v���o�t���O������
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
		Suica_Rec.Data.BIT.SELECT_SND = 0;									// �I�����i�f�[�^���M�� ����
		Suica_Rec.Data.BIT.BRAND_SEL = 0;									// �u�����h�I�� ����
		EcBrandClear();														// �u�����h�I�����ʃN���A
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		Ec_Settlement_Sts = EC_SETT_STS_NONE;								/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
		ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;							/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {							// ���Ϗ������ɒʐM�ُ킪�����H
			queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );				// ope�֒ʒm
			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;						// ���Ϗ�������M�t���O������
		}
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
//		else {
		{
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
			queset( OPETCBNO, EC_EVT_COMFAIL, 0, 0 );						// �ʐM�s�ǔ�����ope�֒ʒm
		}
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
		}
		else {
			queset( OPETCBNO, EC_EVT_COMFAIL, 0, 0 );						// �ʐM�s�ǔ�����ope�֒ʒm
		}
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
	}
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
	else if (err_kind == ERR_SUICA_RECEIVE) {
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
		Suica_Rec.Data.BIT.CTRL_CARD = 0;									// �J�[�h�Ȃ��Ƃ���
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																			// �u�����h�l�S�V�G�[�V�����Ǘ����N���A
		EcEnableNyukin();
		Suica_Rec.Data.BIT.SELECT_SND = 0;									// �I�����i�f�[�^���M�� ����
		Suica_Rec.Data.BIT.BRAND_SEL = 0;									// �u�����h�I�� ����
		edy_dsp.BIT.suica_zangaku_dsp = 0;
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
		queset( OPETCBNO, EC_EVT_NOT_AVAILABLE_ERR, 0, 0 );					// ���σ��[�_�؂藣���G���[������ope�֒ʒm
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//		return;
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
	}
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX

// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
//	// ������INITIALIZE�𗎂Ƃ��ƁA���Z�b�g�O�Ƀ��[�_�[�C�x���g�����ŏ����������Ă��܂�
//	// �����ŏ����������s����Ɛ��Z���̏ꍇ�Ɍ������Z�ł��Ȃ��Ȃ邽�߂����ł͎��{���Ȃ�
//	Suica_Rec.Data.BIT.RESET_RESERVED = 1;									// ��������\�񂷂�
//	// �������V�[�P���X�\�񒆂Ȃ̂ŏ������ς݃t���O���N���A����
//	Suica_Rec.Data.BIT.INITIALIZE = 0;
//// GG119200(S) �N���V�[�P���X�s��C��
//	// �������V�[�P���X�\�񒆂Ȃ̂Ńt�F�[�Y�����������ɍX�V����
//	ECCTL.phase = EC_PHASE_INIT;
//// GG119200(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
	// �ʐM�s�ǁA�ُ�f�[�^��M����JVMA���Z�b�g����������܂�
	// ���σ��[�_�Ƃ̒ʐM�͍s�킸�A�R�C�����b�N���̒ʐM��D�悳����
	// ec_init()����ECCTL.phase���X�V����
	Suica_Rec.Data.BIT.INITIALIZE = 0;
	ECCTL.phase = EC_PHASE_WAIT_RESET;
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
}

/*[]----------------------------------------------------------------------[]*/
/*| JVMA Line Event                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_event( void )
{
	jvma_setup.event();
}

/*[]----------------------------------------------------------------------[]*/
/*| no operation                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : jvma_nop                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	jvma_nop(void)
{
// no operation
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_event_ex( void )
{
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//	if( !Suica_Rec.Data.BIT.INITIALIZE ){		/* ���σ��[�_�������ρH */
//// GG119200(S) �N���V�[�P���X�s��C��
////// MH321800(S) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
//////		if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ){	/* ���Z���H�A���Z�����H */
////		if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ||	/* ���Z���H�A���Z�����H */
////			Suica_Rec.Data.BIT.RESET_RESERVED != 0 ){		// �������ۗ���
////// MH321800(E) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
//		// �������ۗ����͉������M���Ȃ�
//		// �i�Ԏ��ԍ����́`���Z�����j�A���A���������̏ꍇ�͉������M���Ȃ�
//		if( (Suica_Rec.Data.BIT.RESET_RESERVED != 0) ||
//// GG119200(S) JVMA���Z�b�g�����ύX
////			((OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) &&
//			((1 <= OPECTL.Ope_mod && OPECTL.Ope_mod <= 3) &&
//// GG119200(E) JVMA���Z�b�g�����ύX
//			 (ECCTL.phase == EC_PHASE_INIT)) ){
//// GG119200(E) �N���V�[�P���X�s��C��
//			return;
//		} else {
//// GG119200(S) �N���V�[�P���X�s��C��
//			// �t�F�[�Y�����������̏ꍇ�̓X�^���o�C�R�}���h�����蒼��
//			if (ECCTL.phase == EC_PHASE_INIT) {
//// GG119200(E) �N���V�[�P���X�s��C��
//			// ���Z���A���Z�����ȊO�͍ēx�N���V�[�P���X����������
//			jvma_setup.init();
//			return;
//// GG119200(S) �N���V�[�P���X�s��C��
//			}
//// GG119200(E) �N���V�[�P���X�s��C��
//		}
//	}
	if (!Suica_Rec.Data.BIT.INITIALIZE) {
		switch (ECCTL.phase) {
		case EC_PHASE_INIT:
			// ���σ��[�_�̋N���V�[�P���X�������A���A���������̏ꍇ��
			// �X�^���o�C�R�}���h�����蒼��
			jvma_setup.init();
			return;
		case EC_PHASE_WAIT_RESET:
			// JVMA���Z�b�g�҂����͉������M���Ȃ�
			return;
		default:
			break;
		}
	}
// MH810103 GG119202(E) JVMA���Z�b�g�����s�

	jvma_command_set();							/* ���s�������ނ�I������ */
	switch( jvma_act() ){						/* SuicaҲ�ۼޯ� */
	case 1: /* OK */
		break;
	case 2: 									/* Retry Err */
		suica_errst = suica_err[0];
		break;
	case 4: /* ACK4 */
		ex_errlg( jvma_setup.mdl, ERR_EC_ACK4RECEIVE, 2, 0 ); /* �װ۸ޓo�^(ACK4��M) */
		suica_errst = suica_err[0];
		if (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
			// ���Z���A�܂��́A���Z������Ԃ͉������Ȃ�
			// suica_errst�ɃG���[���Z�b�g�����cnmtask��jvma_trb()�����s����Ă��܂�����
			// �������ς݃t���O���N���A���đҋ@��Ԃŏ������V�[�P���X���J�n����
			// �i���Z�b�g������܂�ACK4�̏�Ԃ��ێ�����j
			Suica_Rec.Data.BIT.INITIALIZE = 0;
			break;
		}
		cnwait( 50 );							/* 1sec wait */
		jvma_setup.init();						/* Excute Initial */
		break;
	case -1: 									/* Receive Time Out */
	case -9: 									/* Send Time Out */
		if( (suica_errst & (S_COM_RCV_TIMEOUT_ERR|S_COM_SND_TIMEOUT_ERR) ) == 0 ){	/* Already Error? YJ */
			suica_errst = suica_err[0];
		}
		break;
	default:
		break;
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_command_set                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_command_set( void )
{
	uchar	snd_kind = 0;
	memset( suica_work_buf,0,sizeof( suica_work_buf ));
	Suica_Rec.Snd_Size = Ec_Snd_Que_Read(&snd_kind, suica_work_buf);
	Suica_Rec.snd_kind = 0;
	if (Suica_Rec.Snd_Size == 0) {
		Suica_Rec.Com_kind = 2;			/* Polling���{ */
		return;
	}
	Suica_Rec.snd_kind = snd_kind;

	Suica_Rec.Snd_Buf[0] = (unsigned char)(Suica_Rec.Snd_Size-1);	/* ���M�ް����{��ʂ̍��v���� */
	Suica_Rec.Snd_Buf[1] = Suica_Rec.snd_kind;						/* ���M��� */
	Suica_Rec.Com_kind = 3;												/* �ް����M�v�� */
	memcpy( &Suica_Rec.Snd_Buf[2],suica_work_buf,((sizeof(Suica_Rec.Snd_Buf[2]))*(Suica_Rec.Snd_Size-2)));	/* ���M�ޯ̧���ް���� */
	Suica_Rec.Snd_Buf[Suica_Rec.Snd_Size] = bcccal( (char*)Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size );		/* BCC�ް��t�� */
	Suica_Rec.Snd_Size = Suica_Rec.Snd_Size+1;															/* ���M�ް���İ�ٻ��޾�� */
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica-reader Communication Main  				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicaact                                                |*/
/*| PARAMETER	 : None										               |*/
/*| RETURN VALUE : short		| 1:OK	<>1:Error                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_act( void )
{
	short	rtn;

	rtn = 0;

	suica_errst = 0;								
	memset(suica_err,0,sizeof(suica_err));			/* �װ�i�[�ر�̏����� */
	Suica_Rec.faze = 1;								/* ����ޑ��M̪��ވڍs */

	for( ; rtn==0 ; ){
		switch( Suica_Rec.faze ){
		case 1:										/* ����ޑ��M */
			rtn = jvma_phase_01();
			break;
		case 2:										/* ����ޑ��M���ʎ擾̪��� */
			rtn = jvma_phase_02();
			break;
		case 3:										/* �ް����M̪��� */
			TXRXWAIT = 3;		// 3 * 20 = 60ms
			rtn = jvma_phase_03();
			TXRXWAIT = 0;
			break;
		case 4:										/* �ް����M���ʎ�M̪��� */
			rtn = jvma_phase_04();
			break;
		default:
			break;
		}
		switch( rtn ) {
		case 0:	 								/* Continue */
			continue;
			break;
		case 4:  								/* ACK4 */
			break;
		case -9: 								/* ���MTimeOut */
			if( suica_errst & S_COM_SND_TIMEOUT_ERR ){	/* Already Timeout?  */
				suica_err[0] |= S_COM_SND_TIMEOUT_ERR;
				break;
			}
			suica_err[2]++;						/* Time Out Retry Counter +1 */
			if( suica_err[2] <= SUICA_RTRY_COUNT_31 ){	/* ��ײ��31�����̏ꍇ */
				Suica_Rec.faze = 1;						/* ��ײ�����{ */
				Suica_Rec.Com_kind = 5;
				rtn = 0;
			}else{
				suica_err[0] |= S_COM_SND_TIMEOUT_ERR;	/*�װ��� */
			}
			break;
		case -1: 								/* ��MTimeOut */
			if( Suica_Rec.Status == STANDBY_SND_AFTER ){	/* �ð��������޲����ޑ��M�� */
				Suica_Rec.Com_kind = 0;						/* �ēx�A����޲����ނ𑗐M���� */
				Suica_Rec.faze = 1;
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;		/*�װ��� */
				break;
			}
				
			if( suica_errst & S_COM_RCV_TIMEOUT_ERR ){		/* Already Timeout?  */
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;
				break;
			}
			suica_err[1]++;						/* Time Out Retry Counter +1 */
			if( suica_err[1] <= SUICA_RTRY_COUNT_31 ){		/* ��ײ��31�����̏ꍇ */
				Suica_Rec.faze = 1;							/* ��ײ�����{ */
				rtn = 0;
			}else{
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;		/*�װ��� */
			}
			break;
		default:
			break;
		}
	}
	// ���g���C���������Ă���H
	if( (suica_err[1] != 0) || (suica_err[2] != 0) ||
		(suica_err[3] != 0) || (suica_err[4] != 0) ) {
		// �G���[����
// GG116202(S) ���͍ėv���A�o�͍ėv���̃G���[�͓o�^���Ȃ�
//		err_chk((char)jvma_setup.mdl, ERR_EC_REINPUT, 0, 0, 0 );
//		err_chk((char)jvma_setup.mdl, ERR_EC_REOUTPUT, 0, 0, 0 );
// GG116202(E) ���͍ėv���A�o�͍ėv���̃G���[�͓o�^���Ȃ�
		err_chk((char)jvma_setup.mdl, ERR_EC_NAKRECEIVE, 0, 0, 0 );
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 1...Send Command                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_01		                                           |*/
/*| PARAMETER	 : None									                   |*/
/*| RETURN VALUE : short 	| 0:Continue -9:Send Timeout                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_01()
{
	short	sig;
	unsigned char	*scp;

	sig = Suica_Rec.Com_kind;			/* ���M����ނ�ܰ��ر�ɐݒ� */
// MH810103 GG119202(S) JVMA�ʐM����s��C��
//	if(isEC_USE()) {
//		scp = (unsigned char *)&ec_tbl[sig][0];				/* ���M�����ID��ݒ� */
//	} else {
//		scp = (unsigned char *)&suica_tbl[sig][0];			/* ���M�����ID��ݒ� */
//	}
	// Com_kind��6�ōēx���͗v���������Ƃ��Ă��A���͗v���Ƃ��ď�������B
	// ���̊֐���SX-20�ł͎g���Ȃ�
	if (sig == 6) {
		sig = 2;
	}
	scp = (unsigned char *)&ec_tbl[sig][0];					/* ���M�����ID��ݒ� */
	if (sig == 2) {
		if (suica_rcv_que.count == SUICA_QUE_MAX_COUNT) {
			return 10;		// �󂫔ԍ���NOP���^�[���l�Ƃ���
		}
	}
// MH810103 GG119202(E) JVMA�ʐM����s��C��
	SUICA_RCLR();						// ��M�o�b�t�@�N���A
	if( CNM_CMD( scp, sig ) ) {			/* Send COMMAND */
		return( -9 );					/* TIME OUT */
	}
	Suica_Rec.Status = (unsigned char)(sig+1); /* ���݂̏�Ԃ�ۑ� */

	switch( sig ){
	case 2:
	case 0:
	case 1:
	case 4:
	case 6:
		Suica_Rec.faze = 2;			/* ����ޑ��M���ʎ擾̪��ނɈڍs */
		break;
	case 3:
	case 5:
		Suica_Rec.faze = 3;			/* �ް����M̪��ނɈڍs */
		break;
	default:
		break;
	}

// GG116202(S) ���͍ėv���A�o�͍ėv���̃G���[�͓o�^���Ȃ�
//	// �������V�[�P���X���̓G���[�o�^���Ȃ�
//	if (Suica_Rec.Data.BIT.INITIALIZE) {
//		if( sig == 4 ){
//			err_chk((char)jvma_setup.mdl, ERR_EC_REINPUT, 1, 0, 0 );
//		}else if( sig == 5 ){
//			err_chk((char)jvma_setup.mdl, ERR_EC_REOUTPUT, 1, 0, 0 );
//		}
//	}
// GG116202(E) ���͍ėv���A�o�͍ėv���̃G���[�͓o�^���Ȃ�
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 2...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_02                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    | 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_02( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* �ð���̕ێ� */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
		// 0�ȊO��-1�i�^�C���A�E�g�j
		if( 2 == Suica_Rec.Com_kind ){										// ���͗v�����M��̖�����
			Suica_Rec.Com_kind = 6;											// ���͗v���R�}���h���M
			Suica_Rec.faze = 1;
			return 0;
		}
		Suica_Rec.Com_kind = 2;		// �Q���������Ƃɂ���
jvma_02_10:
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		if( 2 == Suica_Rec.Com_kind ){										// ���͗v�����M��̖�����
			Suica_Rec.Com_kind = 4;											// ���͍ėv���R�}���h���M
		}																	// �ꊇ�v���̖������͍ēx�ꊇ�v�����s�����ƂƂ���
		return( rtn );
	}

	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* ��M�ް���͏��� */
	case 1: /* ACK1 */
		if( ( status == STANDBY_SND_AFTER ||							/* ����ޑ��M��A�ް����M�v��������ꍇ�A   */
			  status == INPUT_SND_AFTER   ||							/* �o�͗v������ޑ��M��A�ް����M���s�� */
			  status == REINPUT_SND_AFTER )) {
			  if( jvma_send_que_check() ){
				jvma_command_set();
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 3;									/* �o�͗v������ޑ��M�������s�� */
				rtn = 0;
				break;
			  }
		}
		rtn = 1;														/* ���͗v�����s�� */
		break;
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		if( (Suica_Rec.Com_kind == 0) ||								// ����޲����ޑ��M��
			(Suica_Rec.Com_kind == 1) ){								// �ꊇ�v�����M��
			goto jvma_02_10;											// ����
		}
		rtn = 1;														
		break;
	case 6: /* NAK */
		if( (Suica_Rec.Com_kind == 0) ||								// ����޲����ޑ��M��
			(Suica_Rec.Com_kind == 1) ){								// �ꊇ�v�����M��
			goto jvma_02_10;											// �đ�
		}
		rtn = 1;
		Suica_Rec.Status = DATA_RCV;									// ��UNIT�ւ�POL�Ɉڂ�
		break;
	case 4: /* ACK4 */
		Suica_Rec.Com_kind = 0;											/* ����޲����ޑ��M */
		Suica_Rec.faze = 1;
		Suica_Rec.Status = DATA_RCV;									// �������ݽ���ł���΁A��UNIT�ւ�POL�Ɉڂ莟�̓X�^���o�C
																		// �������ݽ���łȂ���΁A�����X�^���o�C���M
		break;
	case 5: /* ACK5 */
		goto jvma_02_10;
		break;

	case 9:	/* LRC ERROR */												// PHY error�A�����O�X�ُ���܂�
		if( Suica_Rec.Com_kind == 0 ){									// ����޲���M��
			goto jvma_02_10;											// �����i����޲����ނ̍đ��j
		}
		if( suica_errst & S_LRC_ERR ){									/* Already RetryErr?  */
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;													/* ��ײ�񐔂��ı��� */
		if( suica_err[4] <= SUICA_RTRY_COUNT_31 ){						// ��ײ���ް�łȂ�
			if( 2 == Suica_Rec.Com_kind ){								// ���͗v�����M��̍đ��i�ꊇ�v�����͍đ����ꊇ�j
				Suica_Rec.Com_kind = 4;									// ���͍ėv���R�}���h���M
			}
			Suica_Rec.faze = 1;
			rtn = 0;													/* Retry */
		}else{															/* ��ײ�񐔵��ް */
			suica_err[0] |= S_LRC_ERR;									/* �װ��� */
			rtn = 2;
		}
		break;
	default:
		goto jvma_02_10;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 3...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_03                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short	| 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_03( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* �ð���̕ێ� */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
jvma_03_10:
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		return( rtn );
	}

	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* ��M�ް���͏��� */
	case 1: /* ACK1 */
		if( CNM_SND(Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size) == 0 ){			/* �ް����M */
			Suica_Rec.snd_kind = 0;
			jvma_setup.log_regist( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* �ʐM���O�o�^ */
			Suica_Rec.faze = 4;								/* �ް����M��A���M����̪��ނɈڍs */
			rtn = 0;
			break;
		}else{
			rtn = -9;
			break;
		}
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		rtn = 1;											/* OK */
		break;
	case 4: /* ACK4 */										/* �o�͗v���E�o�͍ėv������ޑ��M���ACK4��M�� */
		Suica_Rec.faze = 1;									/* ����޲����ޑ��M */
		Suica_Rec.Com_kind = 0;
		break;
	case 5: /* ACK5 */
		if( status == REOUTPUT_SND_AFTER ){					/* �o�͍Ďw�ߑ��M�� */
// MH810104(S) MH321800(S) ACK5��M�Ńt�F�[�Y���ꂪ��������
			// �f�[�^���M���ACK1��M���̌㏈�����o�͍Ďw�ߑ��M���ACK5��M�ł��s��
			if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;		// Suica���Z���t���OON�I
				Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
				Suica_Rec.Data.BIT.SELECT_SND = 0;			// �I�����i�f�[�^���M�� ����
			} else if(Suica_Rec.Snd_Buf[1] == S_BRAND_SEL_DATA) {
				Suica_Rec.Data.BIT.SEND_CTRL80 = 1;			// �I�����i�f�[�^�𑗐M����܂ł͎���I���𓊂��Ȃ�
			} else if (Suica_Rec.Snd_Buf[1] == S_CNTL_DATA &&
					(Suica_Rec.Snd_Buf[2] & S_CNTL_REBOOT_OK_BIT) == S_CNTL_REBOOT_OK_BIT){
				// �������ς݃t���O���N���A
				Suica_Rec.Data.BIT.INITIALIZE = 0;			// ����f�[�^ && �ċN���t���OON = �������t���OOFF(�N���V�[�P���X�N��!)
				// �ċN���v���ɉ��������ꍇ�AINITIALIZE=0�ɂȂ�A�ʐM�s�ǂ������Ă��t���O�ނ����������Ȃ��̂ł����Ŏ��{����
				ec_flag_clear(1);
			}
// MH810104(E) MH321800(E) ACK5��M�Ńt�F�[�Y���ꂪ��������
			Suica_Rec.faze = 4;								// ���M�o�����Ɣ��f���A���M�f�[�^����
			if( jvma_send_que_check() ){					/* ����ޑ��M�v��������ꍇ */
				jvma_command_set();
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 3;						/* �o�͗v������ޑ��M�������s�� */
				rtn = 0;		
			}
			else{
				Suica_Rec.faze = 3;							// ���M�f�[�^�����̂��߂ɕς����l�����ɖ߂�
				rtn = 1;									// ���̎����ɏo�͗v������(ACK5���̓f�[�^�̍đ������Ȃ�)
			}
		}
		else{												// �o�͎w��
			goto jvma_03_10;								// �đ�
		}
		break;
	case 6: /* NAK */
		ex_errlg( jvma_setup.mdl, ERR_EC_NAKRECEIVE, 1, 0 );/* �װ۸ޓo�^(NAK��M) */
		if( suica_errst & S_NAK_RCV_ERR ){					/* Already RetryErr?  */
			suica_err[0] |= S_NAK_RCV_ERR;					/* �װ���  */
			rtn = 2;
			break;
		}
		suica_err[3]++;										/* ��ײ�񐔂��ı��� */
		if( suica_err[3] <= SUICA_RTRY_COUNT_21 ){			/* ��ײ�񐔂�21�񖢖� */
			Suica_Rec.faze = 1;								/* ��ײ���{ */
			wait2us( 2500L ); 								/* 5ms wait */
			rtn = 0;
		}else{
			suica_err[0] |= S_NAK_RCV_ERR;					/* �װ��� */
			rtn = 2;
		}
		break;
	default:
		break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 4...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_04                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short | 0:Continue 1:OK ETC:Error                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_04( void )
{
	short	rtn;
	
	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		Suica_Rec.Com_kind = 5;												// ���͏o�͍Ďw�ߑ��M
		return( rtn );
	}
	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){
	case 1: /* ACK1 */
		if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;						// Suica���Z���t���OON�I
			Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			Suica_Rec.Data.BIT.SELECT_SND = 0;							// �I�����i�f�[�^���M�� ����
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		} else if(Suica_Rec.Snd_Buf[1] == S_BRAND_SEL_DATA) {
			Suica_Rec.Data.BIT.SEND_CTRL80 = 1;			// �I�����i�f�[�^�𑗐M����܂ł͎���I���𓊂��Ȃ�
// MH810103 GG119202(S) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
//// GG119200(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			Suica_Rec.Data.BIT.BRAND_SEL = 1;			// �u�����h�I��
//// GG119200(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(E) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
		} else if (Suica_Rec.Snd_Buf[1] == S_CNTL_DATA &&
				   (Suica_Rec.Snd_Buf[2] & S_CNTL_REBOOT_OK_BIT) == S_CNTL_REBOOT_OK_BIT){
			// �������ς݃t���O���N���A
			Suica_Rec.Data.BIT.INITIALIZE = 0;			// ����f�[�^ && �ċN���t���OON = �������t���OOFF(�N���V�[�P���X�N��!)
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
			// �ċN���v���ɉ��������ꍇ�AINITIALIZE=0�ɂȂ�A�ʐM�s�ǂ������Ă��t���O�ނ����������Ȃ��̂ł����Ŏ��{����
			ec_flag_clear(1);
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
		}
	// no break;
	case 5: /* ACK5 */
		if( jvma_send_que_check() ){
			jvma_command_set();
			Suica_Rec.faze = 1;
			rtn = 0;
		}else{
		 	rtn = 1;
		}
		break;
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		rtn = 1; /* OK */
		break;
	case 4: /* ACK4 */										/* �o�͗v���E�o�͍ėv������ޑ��M���ACK4��M�� */
		Suica_Rec.faze = 1;									/* ����޲����ޑ��M */
		Suica_Rec.Com_kind = 0;
		break;
	case 6: /* NAK */
		ex_errlg( jvma_setup.mdl, ERR_EC_NAKRECEIVE, 1, 0 );/* �װ۸ޓo�^(NAK��M) */
		if( suica_errst & S_NAK_RCV_ERR ){					/* Already RetryErr?  */
			suica_err[0] |= S_NAK_RCV_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;
		if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
			Suica_Rec.faze = 1;
			Suica_Rec.Com_kind = 5;							// �o�͍Ďw��
			rtn = 0;
		}else{
			suica_err[0] |= S_NAK_RCV_ERR;
			rtn = 2;
		}
		break;

	case 9:	/* LRC ERROR */
	default:
		if( suica_errst & S_LRC_ERR ){						/* Already RetryErr?  */
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;
		if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
			Suica_Rec.Com_kind = 5;							// ���͏o�͍Ďw��
			Suica_Rec.faze = 1;
			rtn = 0;										/* Retry */
		}else{
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
		}
		break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Snd Data que Check                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Suica_que_check                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
uchar jvma_send_que_check( void )
{
	uchar i;

	if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind && Suica_Rec.faze == 4 ){
		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind = 0;

		if( Suica_Snd_Buf.read_wpt > 3 ){
			Suica_Snd_Buf.read_wpt = 0;
		}
		else{
			Suica_Snd_Buf.read_wpt++;
		}
	}

	for( i=0;i<5;i++ ){
		if( Suica_Snd_Buf.Suica_Snd_q[i].snd_kind ){
			return 1;
		}
	}
	if( Suica_Snd_Buf.read_wpt != Suica_Snd_Buf.write_wpt ){	// �����M�ް����Ȃ��ARpt��Wpt������Ă���ꍇ
		Suica_Snd_Buf.read_wpt = Suica_Snd_Buf.write_wpt;		// Rpt��Wpt�ɂ��킹�A���M�̖������Ȃ���
		// �G���[�o�^
		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_SND_QUE_GAP, (char)2, (char)0, (char)0, (void*)NULL);
		Suica_Rec.Data.BIT.COMM_ERR = 1;
	}

	return 0;
}
