/*[]---------------------------------------------------------------------------[]*/
/*|		���������䕔�FҲݏ���													|*/
/*|																				|*/
/*|	̧�ٖ���	:	PriTask.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"ope_def.h"
#include	"Pri_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

/*[]-----------------------------------------------------------------------[]*/
/*|		���������������														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_int( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-26													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_int( void )
{
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;		// ��Mү���ފi�[�ޯ̧
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//	Target_WaitMsgID.Count = 5;
	Target_WaitMsgID.Count = 8;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_R;
	Target_WaitMsgID.Command[1] = PREQ_INIT_END_J;
	Target_WaitMsgID.Command[2] = TIMEOUT1;
	Target_WaitMsgID.Command[3] = PREQ_SND_COMP_RP;
	Target_WaitMsgID.Command[4] = PREQ_SND_COMP_JP;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	Target_WaitMsgID.Command[5] = PREQ_MACHINE_INFO;	// �@����ʒm
	Target_WaitMsgID.Command[6] = PREQ_JUSHIN_INFO;		// �v�����^�X�e�[�^�X�ω��ʒm
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
//	Target_WaitMsgID.Command[7] = TIMEOUT9;				// �����^�����ݒ�҂��^�C���A�E�g
	Target_WaitMsgID.Command[7] = EJA_INIT_WAIT_TIMEOUT;	// �����^�����ݒ�҂��^�C���A�E�g
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	f_partial_cut = 1;// �p�[�V�����J�b�g������s�ς�(�N���O���V�[�g�v�����^���p�[�V�����J�b�g�������ǂ����킩��Ȃ����ߎ��s�����ƌ��Ȃ��Ă���)

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	memset(&eja_prn_buff, 0, _offsetof(PRN_EJA_DATA_BUFF, eja_proc_data));
	eja_prn_buff.EjaUseFlg = -1;
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�N���������C���j
//	PrnCmd_InfoReq(PINFO_MACHINE_INFO);													// �@����v��
	if (PrnJnlCheck() == ON) {															// �ެ���������ڑ�����H
		PrnCmd_InfoReq(PINFO_MACHINE_INFO);												// �@����v��
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�N���������C���j
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	PrnInit();																			// ���������������ޑ��M
	Lagtim( OPETCBNO, 1, PRN_INIT_TIMER );												// ����������������҂���ϰ���āi�R�b�j

	/*----------------------------------------------*/
	/*	����������������҂�����ٰ��					*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );															// ���������ؑ�
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if( NULL == msb ){																// ���҂���ҰقȂ��i����M�j
			continue;																	// ��Mү���ނȂ� �� continue
		}

		//	��Mү���ނ���
		memcpy( &msg , msb , sizeof(MsgBuf) );											// ��Mү���ފi�[
		FreeBuf( msb );																	// ��Mү�����ޯ̧�J��

		switch( msg.msg.command ){														// ү���޺���ނɂ��e�����֕���

			case PREQ_INIT_END_R:														// ��ڼ��������������I���ʒm��

				rct_init_sts = 1;														// ڼ���������������Ծ�āi�����������j
				break;

			case PREQ_INIT_END_J:														// ���ެ���������������I���ʒm��

				jnl_init_sts = 1;														// �ެ����������������Ծ�āi�����������j
				break;

			case TIMEOUT1:																// ������������������҂���ϰ��ѱ�ā�

				if( rct_init_sts == 0 ){												// ڼ��������������

					// �����������҂��̏ꍇ
					if( rct_init_rty < RCT_INIT_RCNT ){
						//	��������ײ�񐔂��I�����Ă��Ȃ��ꍇ
						PrnInit_R();													// ڼ��������������i��ײ�j
						rct_init_rty++;													// ��ײ���ā{�P
					}
					else{
						//	��������ײ�񐔂��I�������ꍇ
						rct_init_sts = 2;												// ڼ���������������Ծ�āi���������s�j
						Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );// �G���[���o�^�iڼ����������������s�j
					}
				}

				if( jnl_init_sts == 0 ){												// �ެ���������������

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					if ( eja_prn_buff.EjaUseFlg == 0 ) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					// �����������҂��̏ꍇ
					if( jnl_init_rty < JNL_INIT_RCNT ){
						//	��������ײ�񐔂��I�����Ă��Ȃ��ꍇ
						PrnInit_J();													// �ެ���������������i��ײ�j
						jnl_init_rty++;													// ��ײ���ā{�P
					}
					else{
						//	��������ײ�񐔂��I�������ꍇ
						jnl_init_sts = 2;												// �ެ����������������Ծ�āi���������s�j
						Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );// �G���[���o�^�i�ެ�����������������s�j
					}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				}
				if( (rct_init_sts == 0) || (jnl_init_sts == 0) ){
					// ڼ�āA�ެ��قǂ��炩����ł��������������I�����Ă��Ȃ��ꍇ
					Lagtim( OPETCBNO, 1, PRN_INIT_TIMER );								// ����������������҂���ϰ���āi�R�b�j
				}

				break;
			case	PREQ_SND_COMP_RP:
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
				
			case	PREQ_SND_COMP_JP:
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			case PREQ_MACHINE_INFO:			// �@����ʒm
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
				if (jnl_init_sts != 0) {
					// �����������I����͖�������
					break;
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
				if (eja_prn_buff.EjaUseFlg < 0) {
					// �@����v��
					PrnCmd_InfoReq(PINFO_MACHINE_INFO);
					break;
				}
				if (isEJA_USE()) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
// MH810105 MH321800(S) ���b�NSD��}����E2203����������
					if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
						// �uSD�J�[�h�g�p�s�v�A�u�t�@�C���V�X�e���ُ�v�̏ꍇ�͏��������s�Ƃ���
						jnl_init_sts = 2;	// �ެ����������������Ծ�āi���������s�j
						break;
					}
// MH810105 MH321800(E) ���b�NSD��}����E2203����������
					if ((jnl_prn_buff.PrnState[0] & 0x40) == 0) {
						// �����^�����ݒ�=���ݒ�̃v�����^�X�e�[�^�X��҂�
						LagTim500ms(LAG500_EJA_RESET_WAIT_TIMER,
									(short)((PRN_SD_MOUNT_WAIT_TIMER*2)+1),
									PrnCmd_ResetTimeout);
						break;
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
					// �����ݒ�
					PrnCmd_InitSetting();	// [�����^�����ݒ�]
				}
				else {
					PrnInit_J();			// [������]
				}
				break;

			case PREQ_JUSHIN_INFO:			// �v�����^�X�e�[�^�X�ω��ʒm
				PrnCmd_Sts_Proc();
				break;

// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
//			case TIMEOUT9:					// �����^�����ݒ�҂��^�C���A�E�g
			case EJA_INIT_WAIT_TIMEOUT:		// �����^�����ݒ�҂��^�C���A�E�g
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
				if (isEJA_USE()) {
					if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
						// �uSD�J�[�h�g�p�s�v�A�u�t�@�C���V�X�e���ُ�v�̏ꍇ�͏��������s�Ƃ���
						jnl_init_sts = 2;	// �ެ����������������Ծ�āi���������s�j
						break;
					}
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�v���e���I�[�v�����Ă���Ƌ@����v���^�C���A�E�g����j
				else {
					if (jnl_prn_buff.PrnState[0] != 0) {
						// �@����v���̃^�C���A�E�g�A���A�v�����^�X�e�[�^�X��M�ς݂̏ꍇ��
						// �W���[�i���v�����^�ڑ��Ɣ��肷��
						// �v���e���I�[�v�������܂܋N������Ƌ@����v���ɑ΂��鉞����
						// �Ԃ��Ȃ��W���[�i���v�����^�����邽��
						eja_prn_buff.EjaUseFlg = 0;
						queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
						break;
					}
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�v���e���I�[�v�����Ă���Ƌ@����v���^�C���A�E�g����j
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
				// �n�[�h���Z�b�g
				PrnCmd_EJAReset(1);
				break;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

			default:
				break;

		}
		if( (rct_init_sts != 0) && (jnl_init_sts != 0) ){								// ڼ�āA�ެ��ٗ����Ƃ������������I���H

			// �����������I���̏ꍇ
			Lagcan( OPETCBNO, 1 );														// ����������������҂���ϰ����
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
			Lagcan( OPETCBNO, TIMERNO_EJA_INIT_WAIT );									// �����^�����ݒ�҂���ϰ����
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
			break;																		// �����������𔲂���
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���������Ҳݏ���													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	pritask( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	pritask( void )
{
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;		// ��Mү���ފi�[�ޯ̧
	MsgBuf		*OutMsg;	// ү���ޓo�^�p�ޯ̧�߲���
	uchar		pri_kind;	// �󎚑Ώ���������
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	uchar		cmd;
	uchar		ret;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

	/*----------------------------------------------*/
	/*	����������������҂�ٰ��						*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );									// ���������ؑ�

		if( (rct_init_sts != 0) && (jnl_init_sts != 0) ){		// ڼ�āA�ެ��ٗ����Ƃ������������I���H

			// �����������I��

			Header_Rsts	= FLT_ReadHeader( &Header_Data[0][0] );	// ͯ�ް���ް����ׯ����؂���Ǎ�
			Footer_Rsts	= FLT_ReadFooter( &Footer_Data[0][0] );	// ̯�����ް����ׯ����؂���Ǎ�
			AcceptFooter_Rsts = FLT_ReadAcceptFooter( &AcceptFooter_Data[0][0] );
			Syomei_Rsts	= FLT_ReadSyomei( &Syomei_Data[0][0] );	// ���������ް����ׯ����؂���Ǎ�
			Kamei_Rsts	= FLT_ReadKamei( &Kamei_Data[0][0] );	// �����X���ް����ׯ����؂���Ǎ�
			AzuFtr_Rsts = FLT_ReadAzuFtr( &AzuFtr_Data[0][0] );
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
			CreKbrFtr_Rsts = FLT_ReadCreKbrFtr( &CreKbrFtr_Data[0][0] );
			EpayKbrFtr_Rsts = FLT_ReadEpayKbrFtr( &EpayKbrFtr_Data[0][0] );
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
			FutureFtr_Rsts = FLT_ReadFutureFtr( &FutureFtr_Data[0][0] );
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			EmgFooter_Rsts = FLT_ReadEmgFooter( &EmgFooter_Data[0][0] );	// ��Q�A���[̯�����ް����ׯ����؂���Ǎ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
			PrnGetRegistNum();
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			// �N�����̃G���[�`�F�b�N
			PrnCmd_EJAErrRegist();
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			// pritask���̃��b�Z�[�W�N���A
			PrnMsgBoxClear();
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			// �󎚓r���̃f�[�^�̏����ݍĊJ
			ret = PrnCmd_ReWriteCheck();
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			break;												// �����������҂�ٰ�߂𔲂���
		}
	}

	/*----------------------------------------------*/
	/*	������������Ҳ�ٰ��							*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );												// ���������ؑ�

		PriDummyMsg = OFF;													// ��аү���ޓo�^��ԁF�n�e�e
		TgtMsg = NULL;														// ����ү���ޕۑ���ԁF�n�e�e
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (ret != 0) {
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//			PrnCmd_ReWriteStart(1);											// �f�[�^�����ݍĊJ�i���d�j
//			ret = 0;
			switch (ret) {
			case 98:
				// �f�[�^�����ݓr���ŕ��d�̈󎚒��i�󎚑҂��Ȃ��j
				// �󎚃f�[�^�L���[�̈󎚒��i�󎚑҂��Ȃ��j
				if (rct_proc_data.Printing == 0 &&
					jnl_proc_data.Printing == 0) {
					ret = 0;
				}
				break;
			case 99:
				// �f�[�^�����ݓr���ŕ��d�̈󎚒��i�󎚑҂�����j
				if (rct_proc_data.Printing == 0 &&
					jnl_proc_data.Printing == 0) {
					// �f�[�^�����ݓr���ŕ��d�̈󎚊���
					PrnCmd_ReWriteCheck();
					// �f�[�^�����ݍĊJ�i���d�j
					PrnCmd_ReWriteStart(1);
					ret = 0;
				}
				break;
			default:
				// �f�[�^�����ݓr���ŕ��d�̈󎚊J�n
				// �󎚃f�[�^�L���[�̈󎚊J�n
				PrnCmd_ReWriteStart(1);
				if (ret == 3) {
					// �f�[�^�����ݓr���ŕ��d�̈󎚒��i�󎚑҂�����j
					ret = 99;
				}
				else {
					// �f�[�^�����ݓr���ŕ��d�̈󎚒��i�󎚑҂��Ȃ��j
					// �󎚃f�[�^�L���[�̈󎚒��i�󎚑҂��Ȃ��j
					ret = 98;
				}
				break;
			}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

		_di();																// �����݋֎~

		for( ; ; ){															// �����Ώ�ү���ގ擾����ٰ��

			if( ( msb = GetMsg( PRNTCBNO ) ) == NULL ){						// ����������Mү���ގ擾
				break;														// ��Mү���ނȂ� �� �����I��
			}
			// ��Mү���ނ���
			if( TgtMsGet( msb ) == OK ){									// �����ΏۂƂ���ү���ނ��擾����܂�ү���ގ�M���J��Ԃ�
				break;														// �����Ώ�ү���ގ擾���� �� �����I��
			}
		}
		_ei();																// �����݋֎~����

		if( msb == NULL ){
			continue;														// ��Mү���ނȂ��� continue�i���������ؑցj
		}
		// ��Mү���ނ���
		memcpy( &msg ,msb ,sizeof(MsgBuf) );								// ����ү���ށ���Mү����
		FreeBuf( msb );														// ��Mү�����ޯ̧�J��

		if( msg.msg.command == PREQ_DUMMY ){

			// ��аү���ގ�M�i�S�o�^ү���ތ����I���j�̏ꍇ
			if( TgtMsg == NULL ){
				// ����ү���ޕۑ��Ȃ�
				continue;													// �����Ώ�ү���ނȂ� �� continue�i���������ؑցj
			}
			// ����ү���ޕۑ�����
			memcpy( &msg ,TgtMsg ,sizeof(MsgBuf) );							// ����ү���ށ��ۑ�ү����
			FreeBuf( TgtMsg );												// �ۑ�ү�����ޯ̧�J��
		}

		if( CMND_CHK1_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK1_ENO ){

			//	����������ү���ނ̏ꍇ

			/*----------------------------------------------*/
			/*	�����ΏۂƂ����������ʂ��擾����			*/
			/*----------------------------------------------*/
			pri_kind = msg.msg.data[0];		// ү�����ް�����������ʂ��擾

			if( msg.msg.command & INNJI_NEXTMASK ){

				//	�󎚏������̎��ҏW�v��ү����
				terget_pri	= (uchar)( (pri_kind & NEXT_PRI_MASK) >> 4 );	// �����Ώ��������ʾ��
				next_prn_msg = ON;											// ���ҏW�v��ү�����׸ށFON
			}
			else{
				//	���̑���ү����
				next_prn_msg	= OFF;										// ���ҏW�v��ү�����׸ށFOFF

				if( msg.msg.command == PREQ_TGOUKEI			// �u�s���v�v
				||	msg.msg.command == PREQ_GTGOUKEI 		// �u�f�s���v�v
				||	msg.msg.command == PREQ_MTGOUKEI ) {	// �u�l�s���v�v


					//	���v�L�^�󎚗v���̏ꍇ
					terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);			// �����Ώ��������ʾ��

					if( terget_pri == 0 ){

						// �u�󎚐�������̐ݒ聁�Ȃ��v�̏ꍇ
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// �u�󎚏I���i����I���j�vү���ޑ��M
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// �󎚐�������Ȃ� �� continue
					}
				}
				else if( YES == PrnGoukeiChk( msg.msg.command ) ){		// �󎚗v����ʁ����v�L�^�H
					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						// ope�ő��삷��̂�turikan_pri_status��1��N���A����
						terget_pri = PrnGoukeiPri(msg.msg.command);			// ���v�L�^�󎚂̏����Ώ��������ʾ��
					}
					else {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);		// �����Ώ��������ʾ��
					}

					if( terget_pri == 0 ){

						// �u�󎚐�������̐ݒ聁�Ȃ��v�̏ꍇ
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// �u�󎚏I���i����I���j�vү���ޑ��M
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// �󎚐�������Ȃ� �� continue
					}
				}
				else if( msg.msg.command == PREQ_TURIKAN  )  {				// �ޑK�Ǘ����v�H
					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						// ope�ő��삷��̂�turikan_pri_status��1��N���A����
						turikan_pri_status = 0;								// �ޑK�Ǘ�����Ă��󎚏I����Ă���
						terget_pri = PrnGoukeiPri(msg.msg.command);			// ���v�L�^�󎚂̏����Ώ��������ʾ��
					}
					else {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);		// �����Ώ��������ʾ��
					}

					if( terget_pri == 0 ){

						// �u�󎚐�������̐ݒ聁�Ȃ��v�̏ꍇ
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// �u�󎚏I���i����I���j�vү���ޑ��M
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// �󎚐�������Ȃ� �� continue
					}
				}
				else if( msg.msg.command == PREQ_AT_SYUUKEI ){				// �󎚗v����ʁ������W�v�H

					//	�����W�v�󎚗v���̏ꍇ

					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						terget_pri = PrnAsyuukeiPri();						// �����W�v�󎚂̏����Ώ��������ʾ��
					}
					else {
						terget_pri = J_PRI;
					}

					if( terget_pri == 0 ){

						// �u�󎚐�������̐ݒ聁�Ȃ��v�̏ꍇ
						rct_atsyuk_pri = OFF;
						jnl_atsyuk_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// �u�󎚏I���i����I���j�vү���ޑ��M
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// �󎚐�������Ȃ� �� continue
					}
				}
				else{
					//	���v�L�^�󎚗v���A�����W�v�󎚗v���@�ȊO�̏ꍇ
					if(  msg.msg.command != PREQ_INNJI_TYUUSHI ) {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);			// �����Ώ��������ʾ��
					}
				}
			}
			/*----------------------------------------------*/
			/*	�ެ������������								*/
			/*----------------------------------------------*/
			if( PrnJnlCheck() != ON ){		// �ެ���������ڑ�����H

				//	�ެ���������ڑ��Ȃ��̏ꍇ

				if( terget_pri == J_PRI ){
					continue;							// �ެ�������������v�� �� continue
				}
				else if( terget_pri == RJ_PRI ){
					terget_pri = R_PRI;					// ڼ�ā��ެ��ُ����v�� �� �Ώۂ�ڼ��������݂̂Ƃ���
				}

			}
			/*----------------------------------------------*/
			/*	���ް��ҏW�����^�s������				*/
			/*----------------------------------------------*/
			if( CMND_CHK2_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK2_ENO ){

				// �󎚏����^�s�������Ώ�ү���ށi���ް��ҏW�v���j�̏ꍇ

				if( RcvCheckWait( &msg.msg ) == NG ){	// �󎚏����^�s������
					continue;							// �󎚏����s�� �� continue
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//				if (terget_pri == J_PRI && isEJA_USE()) {
//					// FT-4000�ł̓��V�[�g�{�W���[�i���̓����󎚂͂Ȃ�
//					// �f�[�^�����݊J�n��ʒm����
//					PrnCmd_WriteStartEnd(0, &msg.msg);
//				}
//				else if (terget_pri == RJ_PRI && isEJA_USE()) {
				if (terget_pri == RJ_PRI && isEJA_USE()) {
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
					// ���V�[�g�{�W���[�i���󎚗v�����̕��d�΍�Ƃ���
					// �󎚐���f�[�^�ƈ󎚃��b�Z�[�W��ێ�����
					memcpy(&eja_prn_buff.eja_proc_data, &rct_proc_data, sizeof(rct_proc_data));
					memcpy(&eja_prn_buff.PrnMsg, &msg.msg, sizeof(msg.msg));
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			}
			prn_edit_wk_clr();							// �ҏW����ܰ��ر
		}

		/*----------------------------------------------*/
		/*	��Mү���ޏ���								*/
		/*----------------------------------------------*/
		if( CMND_CHK2_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK2_ENO && Inji_Cancel_chk( terget_pri ) != YES ){	// �󎚒��~���ł͂Ȃ��ꍇ�B
			if( terget_pri == R_PRI || terget_pri == RJ_PRI ){
				Prn_LogoHead_PostHead ( );		/* ��󎚂őS�ďo���Ȃ������ꍇ�A�c����󎚂��A����s���� */
			}
		}

		if( CMND_CHK2_SNO <= msg.msg.command && msg.msg.command <= CMND_CHK2_ENO && terget_pri == RJ_PRI){
			// ���V�[�g�{�W���[�i���̏ꍇ�̓W���[�i���󎚂�����t���O�𗧂ĂāA�Ώۂ����V�[�g�݂̂���
			PriBothPrint = 1;
			terget_pri = R_PRI;
		}
		
		switch( msg.msg.command ){	// ү���޺���ނɂ��e�����֕���

			case	PREQ_RYOUSYUU:			// ���̎��؁��󎚗v��	�����Z���~�󎚂��܂�

				PrnRYOUSYUU( &msg.msg );
				break;

			case	PREQ_AZUKARI:			// ���a��؁��󎚗v��

				PrnAZUKARI( &msg.msg );
				break;

			case	PREQ_UKETUKE:			// ����t�����󎚗v��

				PrnUKETUKE( &msg.msg );
				break;

			case	PREQ_TSYOUKEI:			// ���s���v���󎚗v��

				syuukei_kind = TSYOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI:			// ���s���v���󎚗v��

				syuukei_kind = TGOUKEI;		// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTSYOUKEI:			// ���f�s���v���󎚗v��

				syuukei_kind = GTSYOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTGOUKEI:			// ���f�s���v���󎚗v��

				syuukei_kind = GTGOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;
				
			case	PREQ_MTSYOUKEI:			// ���l�s���v���󎚗v��

				syuukei_kind = MTSYOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_MTGOUKEI:			// ���l�s���v���󎚗v��

				syuukei_kind = MTGOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TSYOUKEI:		// �������s���v���󎚗v��

				syuukei_kind = F_TSYOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TGOUKEI:			// �������s���v���󎚗v��

				syuukei_kind = F_TGOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTSYOUKEI:		// �������f�s���v���󎚗v��

				syuukei_kind = F_GTSYOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTGOUKEI:		// �������f�s���v���󎚗v��

				syuukei_kind = F_GTGOUKEI;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI_Z:			// ���O��s���v���󎚗v��

				syuukei_kind = TGOUKEI_Z;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTGOUKEI_Z:		// ���O��f�s���v���󎚗v��

				syuukei_kind = GTGOUKEI_Z;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_MTGOUKEI_Z:		// ���O��l�s���v���󎚗v��

				syuukei_kind = MTGOUKEI_Z;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TGOUKEI_Z:		// ���O�񕡐��s���v���󎚗v��

				syuukei_kind = F_TGOUKEI_Z;	// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTGOUKEI_Z:		// ���O�񕡐��f�s���v���󎚗v��

				syuukei_kind = F_GTGOUKEI_Z;// �W�v��ʾ��
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI_J:			// ���s���v��񁄈󎚗v��

				syuukei_kind = TGOUKEI;		// �W�v��ʾ��
				PrnSYUUKEI_LOG( &msg.msg );
				break;

			case	PREQ_GTGOUKEI_J:		// ���f�s���v��񁄈󎚗v��

				syuukei_kind = GTGOUKEI;		// �W�v��ʾ��
				PrnSYUUKEI_LOG( &msg.msg );
				break;

			case	PREQ_AT_SYUUKEI:		// �������W�v���󎚗v��

				syuukei_kind = TGOUKEI;		// �W�v��ʾ�āi�����W�v�͂s���v�Ɠ��l�j

				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_COKINKO_G:			// ����݋��ɍ��v���󎚗v��

				syuukei_kind = COKINKO_G;	// �W�v��ʾ��
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_S:			// ����݋��ɏ��v���󎚗v��

				syuukei_kind = COKINKO_S;	// �W�v��ʾ��
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_Z:			// ���O��݋��ɍ��v���󎚗v��

				syuukei_kind = COKINKO_Z;	// �W�v��ʾ��
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_J:			// ����݋��ɏW�v��񁄈󎚗v��

				syuukei_kind = COKINKO_G;	// �W�v��ʾ��
				PrnCOKI_JO( &msg.msg );
				break;

			case	PREQ_SIKINKO_G:			// ���������ɍ��v���󎚗v��

				syuukei_kind = SIKINKO_G;	// �W�v��ʾ��
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_S:			// ���������ɏ��v���󎚗v��

				syuukei_kind = SIKINKO_S;	// �W�v��ʾ��
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_Z:			// ���O�񎆕����ɍ��v���󎚗v��

				syuukei_kind = SIKINKO_Z;	// �W�v��ʾ��
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_J:			// ���������ɏW�v��񁄈󎚗v��

				syuukei_kind = SIKINKO_G;	// �W�v��ʾ��
				PrnSIKI_JO( &msg.msg );
				break;

			case	PREQ_TURIKAN:			// ���ޑK�Ǘ��W�v���󎚗v��

				syuukei_kind = TURIKAN;		// �W�v��ʾ��
				PrnTURIKAN( &msg.msg );
				break;

			case	PREQ_TURIKAN_S:			// ���ޑK�Ǘ��W�v�i���v�j���󎚗v��

				syuukei_kind = TURIKAN_S;	// �W�v��ʾ��
				PrnTURIKAN( &msg.msg );
				break;

			case	PREQ_TURIKAN_LOG:		// ���ޑK�Ǘ��W�v�������󎚗v��

				syuukei_kind = TURIKAN;		// �W�v��ʾ��
				PrnTURIKAN_LOG( &msg.msg );
				break;

			case	PREQ_ERR_JOU:			// ���G���[��񁄈󎚗v��

				PrnERR_JOU( &msg.msg );
				break;

			case	PREQ_ERR_LOG:			// ���G���[��񗚗����󎚗v��

				PrnERR_LOG( &msg.msg );
				break;

			case	PREQ_ARM_JOU:			// ���A���[����񁄈󎚗v��

				PrnARM_JOU( &msg.msg );
				break;

			case	PREQ_ARM_LOG:			// ���A���[����񗚗����󎚗v��

				PrnARM_LOG( &msg.msg );
				break;

			case	PREQ_OPE_JOU:			// �������񁄈󎚗v��

				PrnOPE_JOU( &msg.msg );
				break;

			case	PREQ_OPE_LOG:			// �������񗚗����󎚗v��

				PrnOPE_LOG( &msg.msg );
				break;

			case	PREQ_MON_JOU:			// �����j�^��񁄈󎚗v��

				PrnMON_JOU( &msg.msg );
				break;

			case	PREQ_MON_LOG:			// �����j�^��񗚗����󎚗v��

				PrnMON_LOG( &msg.msg );
				break;

			case	PREQ_RMON_JOU:			// �����u�Ď���񁄈󎚗v��
				PrnRMON_JOU( &msg.msg );
				break;

			case	PREQ_RMON_LOG:			// �����u�Ď���񗚗����󎚗v��
				PrnRMON_LOG( &msg.msg );
				break;

			case	PREQ_NG_LOG:			// ���s�����O���󎚗v��

				PrnNG_LOG( &msg.msg );
				break;
			case	PREQ_IO_LOG:			// �����o�Ƀ��O���󎚗v��

				PrnIO_LOG( &msg.msg );
				break;

			case	PREQ_SETTEIDATA:		// ���ݒ�f�[�^���󎚗v��

				PrnSETTEI( &msg.msg );
				break;

			case	PREQ_DOUSACNT:			// ������J�E���g���󎚗v��

				PrnDOUSAC( &msg.msg );
				break;

			case	PREQ_LK_DOUSACNT:		// ��ۯ����u���춳�ā��󎚗v��

				PrnLOCK_DCNT( &msg.msg );
				break;

			case	PREQ_LOCK_PARA:			// ���Ԏ��p�����[�^���󎚗v��

				PrnLOCK_PARA( &msg.msg );
				break;

			case	PREQ_LOCK_SETTEI:		// �����b�N���u�ݒ聄�󎚗v��

				PrnLOCK_SETTEI( &msg.msg );
				break;

			case	PREQ_PKJOU_NOW:			// ���Ԏ����i���݁j���󎚗v��

				PrnPKJOU_NOW( &msg.msg );
				break;

			case	PREQ_PKJOU_SAV:			// ���Ԏ����i�ޔ��j���󎚗v��

				PrnPKJOU_SAV( &msg.msg );
				break;

			case	PREQ_TEIKI_DATA1:		// ������L���^�������󎚗v��

				PrnTEIKIDATA1( &msg.msg );
				break;

			case	PREQ_TEIKI_DATA2:		// ��������Ɂ^�o�Ɂ��󎚗v��

				PrnTEIKIDATA2( &msg.msg );
				break;

			case	PREQ_SVSTIK_KIGEN:		// �����޽���������󎚗v��

				PrnSVSTIK_KIGEN( &msg.msg );
				break;

			case	PREQ_SPLDAY:			// �����ʓ����󎚗v��

				PrnSPLDAY( &msg.msg );
				break;

			case	PREQ_TEIRYUU_JOU:		// ���◯�ԏ�񁄈󎚗v��

				PrnTEIRYUU_JOU( &msg.msg );
				break;

			case	PREQ_TEIFUK_JOU:		// ���╜�d��񁄈󎚗v��

				PrnTEIFUK_JOU( &msg.msg );
				break;

			case	PREQ_TEIFUK_LOG:		// ���╜�d�������󎚗v��

				PrnTEIFUK_LOG( &msg.msg );
				break;

			case	PREQ_KOBETUSEISAN:		// ���ʐ��Z��񁄈󎚗v��	

				PrnSEISAN_LOG( &msg.msg );
				break;

			case	PREQ_FUSKYO_JOU:		// ���s���E�����o�ɏ�񁄈󎚗v��

				PrnFUSKYO_JOU( &msg.msg );
				break;

			case	PREQ_FUSKYO_LOG:		// ���s���E�����o�ɗ������󎚗v��

				PrnFUSKYO_LOG( &msg.msg );
				break;

			case	PREQ_IREKAE_TEST:		// ���p������ւ����e�X�g���󎚗v��	

				PrnIREKAE_TEST( &msg.msg );
				break;

			case	PREQ_PRINT_TEST:		// ���v�����^�e�X�g���󎚗v��	

				PrnPRINT_TEST( &msg.msg );
				break;

			case	PREQ_LOGO_REGIST:		// �����S�󎚃f�[�^���o�^�v��	

				PrnLOGO_REGIST( &msg.msg );
				break;

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//			case	PREQ_CREDIT_CANCEL:		// ���N���W�b�g�����񁄈󎚗v��
//				break;
//
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			case	PREQ_CREDIT_USE:		// ���N���W�b�g���p���ׁ��󎚗v��
//			case	PREQ_HOJIN_USE:
//				PrnCRE_USE( &msg.msg );
//				break;
//
//			case	PREQ_CREDIT_UNSEND:		// ���N���W�b�g�����M����˗��f�[�^���󎚗v��
//				PrnCRE_UNSEND( &msg.msg );
//				break;
//			case	PREQ_CREDIT_SALENG:		// ���N���W�b�g���㋑�ۃf�[�^���󎚗v��
//				PrnCRE_SALENG( &msg.msg );
//				break;
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

			case	PREQ_INNJI_TYUUSHI:		// �󎚒��~�v��

				PrnINJI_TYUUSHI( &msg.msg );
				break;

			case	PREQ_INNJI_END:			// �󎚕ҏW�ް��P��ۯ��󎚏I��

				PrnINJI_END( &msg.msg );
				break;

			case	PREQ_PRINTER_ERR:		// �������װ����

				if( msg.msg.data[0] == R_PRI ){
					// ڼ��������װ�����ݔ���
					Prn_errlg( ERR_PRNT_ERR_IRQ, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �G���[���o�^�iڼ��������װ�����ݔ����j
				}
				else{
					// �ެ���������װ�����ݔ���
					Prn_errlg( ERR_PRNT_ERR_IRQ, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �G���[���o�^�i�ެ���������װ�����ݔ����j
				}
		 		LagTim500ms( LAG500_PRINTER_ERR_RECIEVE, 60*2, PriErrRecieve );	// �P����ϰ
				break;

			case	PREQ_JUSHIN_ERR1:		// ��M�G���[�P�i�߰�߰Ʊ���ވȊO�̽ð���ω��j
				if( (OutMsg = GetBuf()) != NULL )
				{
					OutMsg->msg.command = msg.msg.command;	// �����	�F��M�G���[�P
					OutMsg->msg.data[0] = terget_pri;		// data[0]	�F�������ʁiڼ�ā^�ެ��فj
					PutMsg( OPETCBNO, OutMsg );				// ���ڰ��������ү���ޑ��M
				}

				PrnERREND_PROC();							// �󎚈ُ�I������

				break;

			case	PREQ_JUSHIN_ERR2:		// ��M�G���[�Q(�߰�߰Ʊ���ޕω���)
				if( (OutMsg = GetBuf()) != NULL )
				{
					if( terget_pri == R_PRI ){
						Lagtim( PRNTCBNO ,3 ,PRN_PNEND_TIMER );	// ڼ��		�F�߰�߰Ʊ������ϰ���āi�P�O�b�j
					}
					else{
						Lagtim( PRNTCBNO ,4 ,PRN_PNEND_TIMER );	// �ެ���	�F�߰�߰Ʊ������ϰ���āi�P�O�b�j
					}
					OutMsg->msg.command = PREQ_JUSHIN_ERR1;		// �����	�F��M�G���[�P
					OutMsg->msg.data[0] = terget_pri;			// data[0]	�F�������ʁiڼ�ā^�ެ��فj
					PutMsg( OPETCBNO, OutMsg );					// ���ڰ��������ү���ޑ��M
				}
				break;

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			case PREQ_JUSHIN_INFO:			// �v�����^�X�e�[�^�X�ω��ʒm�i�d�q�W���[�i���p�j
				if ((OutMsg = GetBuf()) != NULL) {
					OutMsg->msg.command = msg.msg.command;	// �����	�F�v�����^�X�e�[�^�X�ω��ʒm
					OutMsg->msg.data[0] = terget_pri;		// data[0]	�F�������ʁi�ެ��فj
					PutMsg( OPETCBNO, OutMsg );				// ���ڰ��������ү���ޑ��M
				}

				PrnERREND_PROC();							// �󎚈ُ�I������
				PrnCmd_Sts_Proc();							// �v�����^�X�e�[�^�X����
				break;

			case PREQ_PRN_INFO:				// �v�����^���v��
				cmd = msg.msg.data[1];
				PrnCmd_InfoReq(cmd);
				break;

			case PREQ_CLOCK_REQ:			// �����ݒ�v��
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
				if (jnl_proc_data.Printing != 0 &&
					(jnl_prn_buff.PrnState[0] & 0x40) == 0) {
					// �����ݒ��A���A�����^�����ݒ聂���ݒ�̏ꍇ��
					// �����݊�����҂�
					eja_prn_buff.PrnClkReqFlg = 1;
					break;
				}
				PrnCmd_Clock();
				break;
			case PREQ_INIT_SET_REQ:			// �����^�����ݒ�v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
// MH810105 MH321800(S) ���b�NSD��}����E2203����������
				if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
					// �uSD�J�[�h�g�p�s�v�A�u�t�@�C���V�X�e���ُ�v�̏ꍇ��
					// �����^�����ݒ�R�}���h�𑗐M���Ȃ�
					break;
				}
// MH810105 MH321800(E) ���b�NSD��}����E2203����������
				PrnCmd_InitSetting();
				break;

			case PREQ_CLOCK_SET_CMP:		// �����ݒ芮��
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
				if (ret != 0) {
					// ���d��̃f�[�^�����ݍĊJ��
					break;
				}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
				PrnCmd_ReWriteStart(0);						// �f�[�^�����ݍĊJ
				break;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

			case TIMEOUT1:	//	ڼ�������BusyTimer	���ضް�́ASci0Snd
				// BUSY��UART�R���g���[���[�Ő��䂷��̂ŁA���M�ُ펞�̐���Ƃ��Ďg�p����B
				// �҂����Ԃ͓����Ƃ���i�P�O�b�ȏ�F��P�P�b�j
				if( rct_prn_buff.PrnBufCnt ){				// ���M�������H
					rct_prn_buff.PrnBusyCnt++;				// ������޼ް�p���Ď������X�V
					if( (rct_prn_buff.PrnBusyCnt >= PRN_BUSY_TCNT) && (rct_proc_data.Printing != 0) ){

						//	�󎚏�������������޼ް���P�O�b�ȏ�p�������ꍇ
						err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 1, 0, 0 );// ڼ��������ʐM�ُ�o�^
						Inji_ErrEnd(						// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
										rct_proc_data.Printing,
										PRI_ERR_BUSY,
										R_PRI
									);
						JP_I2CSndReq( I2C_PRI_REQ );		/* �W���[�i���ɑ��M����f�[�^������Α��M���� */
					}
					else{
						Lagtim( PRNTCBNO, 1, PRN_BUSY_TIMER );	// �T�b wait
					}
				}
				break;

			case TIMEOUT2:	//	�ެ��������BusyTimer	���ضް�́ASci3Snd
				// BUSY��UART�R���g���[���[�Ő��䂷��̂ŁA���M�ُ펞�̐���Ƃ��Ďg�p����B
				// �҂����Ԃ͓����Ƃ���i�P�O�b�ȏ�F��P�P�b�j
				if( jnl_prn_buff.PrnBufCnt ){				// ���M�������H
					jnl_prn_buff.PrnBusyCnt++;				// ������޼ް�p���Ď������X�V
					if( (jnl_prn_buff.PrnBusyCnt >= PRN_BUSY_TCNT) && (jnl_proc_data.Printing != 0) ){

						//	�󎚏�������������޼ް���P�O�b�ȏ�p�������ꍇ
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
						if (isEJA_USE()) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�ʐM�s�ǌ��o���̓��Z�b�g���Ȃ��j
							Lagcan(PRNTCBNO, 10);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�ʐM�s�ǌ��o���̓��Z�b�g���Ȃ��j
							err_chk(ERRMDL_EJA, ERR_EJA_COMFAIL, 1, 0, 0);	// �ʐM�s�Ǔo�^
						}
						else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
						err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 1, 0, 0 );// �ެ���������ʐM�ُ�o�^
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
						}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
						Inji_ErrEnd(						// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
										jnl_proc_data.Printing,
										PRI_ERR_BUSY,
										J_PRI
									);
						RP_I2CSndReq( I2C_PRI_REQ );		/* ���V�[�g�ɑ��M����f�[�^������Α��M���� */
					}
					else{
						Lagtim( PRNTCBNO, 2, PRN_BUSY_TIMER );	// �T�b wait
					}
				}
				break;

			case TIMEOUT3:	// ڼ���߰�߰Ʊ����Timer
				if( rct_prn_buff.PrnState[0] != rct_prn_buff.PrnStWork )
				{
					rct_prn_buff.PrnState[2] = rct_prn_buff.PrnState[1];
					rct_prn_buff.PrnState[1] = rct_prn_buff.PrnState[0];
					rct_prn_buff.PrnState[0] = rct_prn_buff.PrnStWork;
					if( (OutMsg = GetBuf()) != NULL )
					{
						OutMsg->msg.command = PREQ_JUSHIN_ERR1;	// �����	�F��M�G���[�P
						OutMsg->msg.data[0] = R_PRI;			// data[0]	�F�������ʁiڼ�āj
						PutMsg( OPETCBNO, OutMsg );				// ���ڰ��������ү���ޑ��M
					}
				}
				break;

			case TIMEOUT4:	// �ެ����߰�߰Ʊ����Timer
				if( jnl_prn_buff.PrnState[0] != jnl_prn_buff.PrnStWork )
				{
					jnl_prn_buff.PrnState[2] = jnl_prn_buff.PrnState[1];
					jnl_prn_buff.PrnState[1] = jnl_prn_buff.PrnState[0];
					jnl_prn_buff.PrnState[0] = jnl_prn_buff.PrnStWork;
					if( (OutMsg = GetBuf()) != NULL )
					{
						OutMsg->msg.command = PREQ_JUSHIN_ERR1;	// �����	�F��M�G���[�P
						OutMsg->msg.data[0] = J_PRI;			// data[0]	�F�������ʁi�ެ��فj
						PutMsg( OPETCBNO, OutMsg );				// ���ڰ��������ү���ޑ��M
					}
				}
				break;

			case TIMEOUT5:	// ���ް��ҏW�x����ϰ�iڼ�āj

				if( rct_proc_data.EditWait == ON ){	// �ҏW�x����ϰ�N�����H
					PrnNext( &NextMsg_r, R_PRI );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�iڼ�āj
					rct_proc_data.EditWait = OFF;	// ���ް��ҏW�x����ϰ�N�����ؾ��
				}

				break;

			case TIMEOUT6:	// ���ް��ҏW�x����ϰ�i�ެ��فj

				if( jnl_proc_data.EditWait == ON ){	// �ҏW�x����ϰ�N�����H
					PrnNext( &NextMsg_j, J_PRI );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�i�ެ��فj
					jnl_proc_data.EditWait = OFF;	// ���ް��ҏW�x����ϰ�N�����ؾ��
				}

				break;

			case TIMEOUT7:	// ���V�[�g�󎚊�����^�C�}�[
				rct_timer_end = 0;					// ���̃t���O��ON�̏ꍇ�́A���V�[�g�v�����^�󎚒��Ƃ��A�W���[�i���v�����^�ւ̈󎚂�}�~
				break;

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
//			case TIMEOUT8:		// �f�[�^�����݊����҂��^�C�}
			case TIMEOUT10:		// �f�[�^�����݊����҂��^�C�}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�ʐM�s�ǌ��o���̓��Z�b�g���Ȃ��j
				if (jnl_prn_buff.PrnBufCnt) {
					// �f�[�^���M���̓^�C�}���Z�b�g
					Lagtim(PRNTCBNO, 10, PRN_WRITE_CMP_TIMER);
					break;
				}
				if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
					// �ʐM�s�ǔ������̓\�t�g���Z�b�g���Ȃ�
					break;
				}
				// no break
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�ʐM�s�ǌ��o���̓��Z�b�g���Ȃ��j
			case PREQ_FS_ERR:	// �t�@�C���V�X�e���ُ�ʒm
				PrnCmd_EJAReset(0);
				break;

			case TIMEOUT9:		// �����^�����ݒ�҂��^�C���A�E�g
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g�v���C�x���g�ǉ��j
			case PREQ_RESET:	// ���Z�b�g�v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g�v���C�x���g�ǉ��j
				PrnCmd_EJAReset(1);
				break;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

			case	PREQ_SUICA_LOG:			// ��Suica�ʐM���O���󎚗v��

				PrnSuica_LOG( &msg.msg );
				break;
			case	PREQ_SUICA_LOG2:		// ��Suica�ʐM���O���߃f�[�^���󎚗v��

				PrnSuica_LOG2( &msg.msg );
				break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//			case	PREQ_EDY_STATUS:		// ��Edy�ݒ聕�X�e�[�^�X���󎚗v��
//
//				PrnEdy_Status( &msg.msg );
//				break;
//
//			case	PREQ_EDY_ARM_R:			// ���װ�ڼ�ā��󎚗v��	
//
//				PrnEdyArmPri( &msg.msg );
//				break;
//
//			case	PREQ_EDY_ARMPAY_LOG:	// ���A���[��������LOG���󎚗v��
//
//				PrnEDYARMPAY_LOG( &msg.msg );
//				break;
//
//			case	PREQ_EDY_SHIME_R:		// ���d�������ߋL�^���󎚗v��	
//
//				PrnEdyShimePri( &msg.msg );
//				break;
//
//			case	PREQ_EDY_SHIME_LOG:		// ���d�������ߋL�^��񁄈󎚗v��
//
//				PrnEDYSHIME_LOG( &msg.msg );
//				break;
//#endif
//			case	PREQ_EDY_USE_LOG:		// ���d�������p���ׁ��󎚗v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			case	PREQ_SCA_USE_LOG:		// ���r�����������p���ׁ��󎚗v��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case	PREQ_EC_USE_LOG:		// �����σ��[�_���p���ׁ��󎚗v��
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			case	PREQ_EC_MINASHI_LOG:	// ���݂Ȃ����σv�����g���󎚗v��
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
				PrnDigi_USE( &msg.msg );	// �J�[�h�����p���׈󎚏���
				break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case	PREQ_EDY_SYU_LOG:		// ���d�����W�v���󎚗v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			case	PREQ_SCA_SYU_LOG:		// ���r���������W�v���󎚗v��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case	PREQ_EC_SYU_LOG:		// �����σ��[�_�W�v���󎚗v��
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
				PrnDigi_SYU( &msg.msg );
				break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case	PREQ_EC_ALM_R:			// ���װ�ڼ�ā��󎚗v��	
				PrnEcAlmRctPri( &msg.msg );
				break;
			case	PREQ_EC_BRAND_COND:		// �����σ��[�_�u�����h��ԁ��󎚗v��
				PrnEcBrandPri( &msg.msg );
				break;
			case	PREQ_EC_ALARM_LOG:		// ������������L�^�󎚗v��
				PrnEcAlmRctLogPri( &msg.msg );
				break;
// MH810103 GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
//			case	PREQ_EC_DEEMED_LOG:		// �݂Ȃ����ϕ��d���O�󎚗v��	
//				PrnEcDeemedJnlPri( &msg.msg );
//				break;
// MH810103 GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
			case	PREQ_RECV_DEEMED:		// ���ϐ��Z���~(������)�f�[�^�󎚗v��
				PrnRecvDeemedDataPri( &msg.msg );
				break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�				
			case	PREQ_ATEND_INV_DATA:	// ���W�������f�[�^���󎚗v��

				PrnAtendValidData( &msg.msg );
				break;
			case	PREQ_CHARGESETUP:		// �������ݒ�󎚁��󎚗v��

				PrnCHARGESETUP( &msg.msg );
				break;
			case	SETDIFFLOG_PRINT:		// �ݒ�ύX������
				PrnSetDiffLogPrn ( &msg.msg );
				break;
			case	PREQ_RYOSETTEI_LOG:
				PrnRrokin_Setlog(&msg.msg);		/* �����ݒ�ʐM���O�v�����g */
				break;
			case	PREQ_CHK_PRINT:
				PrnSettei_Chk(&msg.msg);
				break;
			case PREQ_LOOP_DATA:				// �t���b�v���[�v�f�[�^�v�����ʈ�
				PrnFlapLoopData(&msg.msg);
				break;
			case PREQ_RT_PAYMENT_LOG:			// �����e�X�g���O�v�����g
				RT_PrnSEISAN_LOG( &msg.msg );
				break;
			case PREQ_MNT_STACK:				// �X�^�b�N�g�p�ʈ�
				PrnMntStack( &msg.msg );
				break;
			case PREQ_CHKMODE_RESULT:
				PrnChkResult(&msg.msg);
				break;
// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
			case PREQ_DEBUG:					// �f�o�b�O�p�󎚗v��
				PrnDEBUG(&msg.msg);
				break;
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			case	PREQ_RECV_FAILURECONTACT:	// ��Q�A���[�f�[�^�󎚗v��
				PrnFailureContactDataPri( &msg.msg );
				break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			case	PREQ_STOCK_PRINT:
				PrnRYOUSYUU_StockPrint();
				break;
			case	PREQ_STOCK_CLEAR:
				PrnRYOUSYUU_StockClear();
				break;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			default:		// ���̑�
				break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�����Ώ�ү���ގ擾													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	TgtMsGet( *msb )									|*/
/*|																			|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	��Mү�����ޯ̧�߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret���擾����								|*/
/*|							NG : �擾�Ȃ�									|*/
/*|							OK : �擾����									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	TgtMsGet( MsgBuf *msb )
{
	uchar	ret;		// �߂�l
	uchar	tgtchk;		// ү������������
	ushort	cmd1;	// ��Mү���޺����
	ushort	cmd2;	// ��Mү���޺����
	MsgBuf	*OutMsg;	// ү���ޓo�^�p�ޯ̧�߲���

	if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){
		
		cmd1 = msb->msg.command;
		cmd2 = msb->msg.command & MSG_CMND_MASK;
		if( ( CMND_CHK2_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK2_ENO ) ){
			if( !(cmd1 & INNJI_NEXTMASK) ){
				// �V�K�󎚗v���̏ꍇ
				if(msb->msg.data[0] == J_PRI && rct_timer_end) {
					goto Lexclsive;						// ���b�Z�[�W�ēo�^������
				}
			}
		}
		else if(cmd2 == PREQ_INNJI_TYUUSHI){
			if(PriBothPrint == 2 && rct_timer_end){
				/* �����󎚎w��Ń��V�[�g�ւ̈󎚂��������ăW���[�i���ւ̈󎚂�҂��Ă��� */
				msb->msg.data[0] = J_PRI;
				goto Lexclsive;						// ���b�Z�[�W�ēo�^������
			}
			if(msb->msg.data[0] == J_PRI && rct_timer_end) {
				goto Lexclsive;						// ���b�Z�[�W�ēo�^������
			}
		}
		// �󎚏������̎��ҏW�v���̏ꍇ
		ret = OK;
	}
	else{
		// �󎚏������̏ꍇ
Lexclsive:
		tgtchk = TgtMsgChk( msb );						// �󎚏������̏����Ώ�ү���ޔ���

		if( tgtchk == OK ){								// ����H

			// �����Ώ�ү���ނ̏ꍇ

			if( msb->msg.command == PREQ_DUMMY ){

				// ��аү���ށi�S�o�^ү���ނ̌����I���j�̏ꍇ

				ret = OK;								// �擾���ʁ��擾����
			}
			else if(msb->msg.command == PREQ_INNJI_TYUUSHI){		
			/* ���~�v���������ɑΏۃv�����^���󎚒��ł͂Ȃ��ꍇ�A���~���悤�Ƃ��Ă���󎚗v�����܂���������Ă��Ȃ��\�������� */
			/* �ȍ~�̃��b�Z�[�W�L���[�ɑΏۃv�����^�̈󎚗v��������΁A���~�v�����ēo�^���� */
				if( Cancel_repue_chk((T_FrmPrnStop*)msb->msg.data)){	
					/* �L�����Z���̍ēo�^���s�� */
					if( PriDummyMsg == OFF ){

						// ��аү���ޓo�^�ς݂łȂ��ꍇ�i�S�o�^ү���ނ̌����I���𔻒f����ׁA��аү���ނ�o�^����j

						if( (OutMsg = GetBuf()) != NULL ){		// ү���ޑ��M�ޯ̧�擾

							// ��аү���ޑ��M�ޯ̧�擾�n�j

							OutMsg->msg.command = PREQ_DUMMY;
							PutMsg( PRNTCBNO, OutMsg );			// ��аү���ޓo�^
							PriDummyMsg = ON;					// ��аү���ޓo�^�׸ށF�n�m

							PutMsg( PRNTCBNO, msb );			// ��Mү���ލēo�^
						}
						else{
							// ��аү���ޑ��M�ޯ̧�擾�m�f
							FreeBuf( msb );						// ��Mү�����ޯ̧�J��
						}
					}else{
						PutMsg( PRNTCBNO, msb );				// ��Mү���ލēo�^
					}
					ret = NG;
				}
				else{
					ret = OK;
				}
			}
			else{
				// ��аү���ވȊO
				if( PriDummyMsg == OFF ){

					// ��аү���ޓo�^�ς݂łȂ��i��Mү���ނ̍ēo�^�Ȃ��j�ꍇ

					ret = OK;							// �擾���ʁ��擾����
				}
				else{
					// ��аү���ޓo�^�ς݁i��Mү���ނ̍ēo�^����j�̏ꍇ

					if( TgtMsg == NULL ){

						// ����ү���ޕۑ�������Ă��Ȃ��ꍇ�i��Mү���ނ�ۑ�����j

						TgtMsg	= msb;					// �����Ώ�ү���ޕۑ�
						ret		= NG;					// �擾���ʁ��擾�Ȃ�
					}
					else{
						// ����ү���ޕۑ������ɂ���Ă���ꍇ�i��Mү���ނ�ۑ��ł��Ȃ��̂ōēo�^����j

						PutMsg( PRNTCBNO, msb );		// ��Mү���ލēo�^
						ret = NG;						// �擾���ʁ��擾�Ȃ�
					}
				}
			}
		}
		else{
			// �����Ώ�ү���ނłȂ��ꍇ�i��Mү���ނ��ēo�^����j
			if( PriDummyMsg == OFF ){

				// ��аү���ޓo�^�ς݂łȂ��ꍇ�i�S�o�^ү���ނ̌����I���𔻒f����ׁA��аү���ނ�o�^����j

				if( (OutMsg = GetBuf()) != NULL ){		// ү���ޑ��M�ޯ̧�擾

					// ��аү���ޑ��M�ޯ̧�擾�n�j

					OutMsg->msg.command = PREQ_DUMMY;
					PutMsg( PRNTCBNO, OutMsg );			// ��аү���ޓo�^
					PriDummyMsg = ON;					// ��аү���ޓo�^�׸ށF�n�m

					PutMsg( PRNTCBNO, msb );			// ��Mү���ލēo�^
				}
				else{
					// ��аү���ޑ��M�ޯ̧�擾�m�f
					FreeBuf( msb );						// ��Mү�����ޯ̧�J��
				}
			}else{
				PutMsg( PRNTCBNO, msb );				// ��Mү���ލēo�^
			}
			ret = NG;									// �擾���ʁ��擾�Ȃ�
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚏������̏����Ώ�ү���ޔ���										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	TgtMsgChk( *msb )									|*/
/*|																			|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	��Mү�����ޯ̧�߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret�����茋��								|*/
/*|							OK : �����Ώ�ү����								|*/
/*|							NG : �����Ώ�ү���ވȊO							|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	TgtMsgChk( MsgBuf *msb )
{
	uchar	ret;	// �߂�l
	ushort	cmd1;	// ��Mү���޺����
	ushort	cmd2;	// ��Mү���޺����

	cmd1 = msb->msg.command;
	cmd2 = msb->msg.command & MSG_CMND_MASK;

	if( ( CMND_CHK1_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK1_ENO ) ){

		//	����������ү����(0x601�`0x6ff)�̏ꍇ

		if( ( CMND_CHK2_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK2_ENO ) ){

			// �󎚗v��ү���ނ̏ꍇ

			if( cmd1 & INNJI_NEXTMASK ){

				// �󎚏������̎��ҏW�v���̏ꍇ
				ret = OK;						// ���茋�ʁF�n�j
			}
			else{
				// �V�K�󎚗v���̏ꍇ
				ret = NG;						// ���茋�ʁF�m�f
			}
		}
		else{
			// �󎚗v��ү���ވȊO�̏ꍇ
			ret = OK;							// ���茋�ʁF�n�j
		}
	}
	else{
		// ����������ү���ވȊO�̏ꍇ
		ret = OK;								// ���茋�ʁF�n�j
	}
	return( ret );
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚃L�����Z������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Cancel_repue_chk( T_FrmPrnStop* data )				|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	��Mү�����ޯ̧�߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret�����茋��								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar Cancel_repue_chk(T_FrmPrnStop* data)
{
	uchar pri_kind = data->prn_kind;
	uchar	ret = 0;
	
	switch(pri_kind){
		case R_PRI:
			if(rct_proc_data.Printing == 0){
				ret = PreqMsgChk( R_PRI );
			}
			break;
		case J_PRI:
			if(jnl_proc_data.Printing == 0 || PriBothPrint == 2){
				ret = PreqMsgChk( J_PRI );
			}
			break;
		case RJ_PRI:
		default:
			break;
	}
	
	return ret;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚗v������														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Cancel_repue_chk( T_FrmPrnStop* data )				|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	��Mү�����ޯ̧�߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret�����茋��								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
char	PreqMsgChk( char kind ){
	MsgBuf	*msg_add;										// Ұٷ���������Ώ�Ұٱ��ڽ��Ĵر
	ushort	MailCommand;									// Ұٷ���������Ώ�Ұق�ү����ID
	ulong	ist;											// ���݂̊�����t���
	uchar	i;
	char	ret = 0;
	
	if( tcb[PRNTCBNO].msg_top == NULL ) {							// ҰقȂ�
		return( NULL );
	}

	ist = _di2();											// �����݋֎~
	msg_add = tcb[PRNTCBNO].msg_top;								// �����Ώ�Ұٱ��ڽget�i�ŏ��͐擪�j

	// ��������
	for( i=0; i<MSGBUF_CNT; ++i ){							// Ұٷ���� �SҰ������iMSGBUF_CNT�͒P��Limitter�j

		MailCommand = msg_add->msg.command;					// �����Ώ�Ұق�ү����IDget

		if( ( CMND_CHK2_SNO <= MailCommand ) && ( MailCommand <= CMND_CHK2_ENO ) ){	// �󎚗v���̏ꍇ
			if(msg_add->msg.data[0] == kind ){					// �f�[�^�̐擪���󎚎��
				if(PriBothPrint == 2){
					if(MailCommand == BothPrintCommand){
						ret = 1;
						break;
					}
				}
				else{
					ret = 1;
					break;
				}
			}
		}

		// ����Ұقցi���������j
		msg_add = (MsgBuf*)(msg_add->msg_next);				// �������Ώ�Ұٱ��ڽget
		if( NULL == msg_add ){								// ��ҰقȂ�
			ret = 0;
			break;
		}

		// 16���1�� WDT�ر
		if( 0x0f == (i & 0x0f) ){
			WACDOG;
		}
	}
	_ei2( ist );
	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚏����^�s������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	RcvCheckWait( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �󎚏�����									|*/
/*|							NG : �󎚏����s��								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	RcvCheckWait( MSG *msg )
{
	uchar	pri_sts;		// ��������
	uchar	ret = OK;		// �߂�l
	uchar	ret_rct = OK;	// �󎚉^�s�iڼ�āj
	uchar	ret_jnl = OK;	// �󎚉^�s�i�ެ��فj

	if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){	// �󎚏������H

		//	�󎚏������łȂ��ꍇ

		f_Prn_R_SendTopChar = 1;								// 1=ڼ��������󎚗v���̐擪�s�������M�O
		f_Prn_J_SendTopChar = 1;								// 1=�ެ���������󎚗v���̐擪�s�������M�O

		switch( terget_pri ){	// �����Ώ��������ʁH

			case	R_PRI:		// ڼ��

				if( msg->command == PREQ_AT_SYUUKEI ){			// �����W�v �󎚗v���H
					rct_atsyuk_pri = ON;						// �����W�v�󎚏�ԁiڼ�ėp�j���
				}

				/*------	ڼ��������󎚊J�n����	-----*/
				pri_sts = PriStsCheck( R_PRI );					// ڼ��������������
				if( pri_sts != PRI_ERR_NON ){

					// �󎚕s��Ԃ̏ꍇ
					Inji_ErrEnd( msg->command, pri_sts, R_PRI );// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
					ret = NG;
				}
				else{
					// �󎚉\��Ԃ̏ꍇ
					prn_proc_data_clr_R();						// ڼ�ĕҏW���������ް��ر
					rct_proc_data.Printing = msg->command;		// ��Mү���޺���ޕۑ�

					if( YES == PrnGoukeiChk( msg->command ) ){	// ���v�L�^ �󎚗v���H
						rct_goukei_pri = ON;					// ���v�L�^�󎚏�ԁiڼ�ėp�j���
						jnl_goukei_pri = OFF;					// ���v�L�^�󎚏�ԁi�ެ��ٗp�jؾ��
					}
				}
				break;

			case	J_PRI:		// �ެ���

				if( msg->command == PREQ_AT_SYUUKEI ){			// �����W�v �󎚗v���H
					jnl_atsyuk_pri = ON;						// �����W�v�󎚏�ԁi�ެ��ٗp�j���
				}

				/*------	�ެ���������󎚊J�n����		-----*/
				pri_sts = PriStsCheck( J_PRI );					// �ެ���������������
				if( pri_sts != PRI_ERR_NON ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					if (pri_sts == PRI_ERR_WAIT_INIT) {
						PrnCmd_MsgResend(msg);
						ret = NG;
						break;
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

					// �󎚕s��Ԃ̏ꍇ
					Inji_ErrEnd( msg->command, pri_sts, J_PRI );// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
					ret = NG;
				}
				else{
					// �󎚉\��Ԃ̏ꍇ
					prn_proc_data_clr_J();						// �ެ��ٕҏW���������ް��ر
					jnl_proc_data.Printing = msg->command;		// ��Mү���޺���ޕۑ�

					if( YES == PrnGoukeiChk( msg->command ) ){	// ���v�L�^ �󎚗v���H
						if(PriBothPrint == 0) {
							rct_goukei_pri = OFF;				// ���v�L�^�󎚏�ԁiڼ�ėp�jؾ��
						}
						jnl_goukei_pri = ON;					// ���v�L�^�󎚏�ԁi�ެ��ٗp�j���
					}
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
					// �󎚗v����M���Ƀf�[�^�����݊J�n��ʒm����
					PrnCmd_WriteStartEnd(0, msg);
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
				}
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				if( msg->command == PREQ_AT_SYUUKEI ){			// �����W�v �󎚗v���H

					// �����W�v�󎚗v���̏ꍇ
					rct_atsyuk_pri = ON;						// �����W�v�󎚏�ԁiڼ�ėp�j���
					jnl_atsyuk_pri = ON;						// �����W�v�󎚏�ԁi�ެ��ٗp�j���
				}
				/*------	ڼ��������󎚊J�n����	-----*/

				pri_sts = PriStsCheck( R_PRI );					// ڼ��������������
				if( pri_sts != PRI_ERR_NON ){

					// �󎚕s��Ԃ̏ꍇ
					Inji_ErrEnd( msg->command, pri_sts, R_PRI );// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
					ret_rct = NG;
				}
				else{
					// �󎚉\��Ԃ̏ꍇ
					prn_proc_data_clr_R();						// ڼ�ĕҏW���������ް��ر
					rct_proc_data.Printing = msg->command;		// ��Mү���޺���ޕۑ�
					if( msg->command == PREQ_AT_SYUUKEI ){		// �����W�v �󎚗v���H
						rct_goukei_pri = ON;					// ���v�L�^�󎚏�ԁiڼ�ėp�j���
					}
				}

				/*------	�ެ���������󎚊J�n����		-----*/
				pri_sts = PriStsCheck( J_PRI );					// �ެ���������������
				if( pri_sts != PRI_ERR_NON ){

					// �󎚕s��Ԃ̏ꍇ
					Inji_ErrEnd( msg->command, pri_sts, J_PRI );// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
					ret_jnl = NG;
				}
				else{
					// �󎚉\��Ԃ̏ꍇ
					prn_proc_data_clr_J();						// �ެ��ٕҏW���������ް��ر
					// �����ł�jnl_proc_data.Printing�͐ݒ肹��
					if(	rct_proc_data.Printing == 0) {
						jnl_proc_data.Printing = msg->command;	// ��Mү���޺���ޕۑ�
					}
					// �ʂ̃��b�Z�[�W�ɂ����W���[�i���󎚎��ɃZ�b�g����
					if( msg->command == PREQ_AT_SYUUKEI ){		// �����W�v �󎚗v���H
						jnl_goukei_pri = ON;					// ���v�L�^�󎚏�ԁiڼ�ėp�j���
					}
// GG129001(S) �d�q�W���[�i���Ή��i�����݊J�n�R�}���h��2�񑗐M�����j
//// MH364301(S) �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//					// �󎚗v����M���Ƀf�[�^�����݊J�n��ʒm����
//					PrnCmd_WriteStartEnd(0, msg);
//// MH364301(E) �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
// GG129001(E) �d�q�W���[�i���Ή��i�����݊J�n�R�}���h��2�񑗐M�����j
				}

				if( YES == PrnGoukeiChk( msg->command ) ){		// ���v�L�^ �󎚗v���H

					//	���v�L�^�̏ꍇ�Aڼ�āA�ެ��قƂ��Ɉ󎚉\�ȏꍇ�݈̂󎚂��J�n����i������󎚕s�̏ꍇ�󎚂��Ȃ��j
					//	�i�󎚂�����I���������ݸނŏW�v�ް��ر���s���ׁA��������������󎚉\�ȏꍇ�݈̂󎚊J�n�Ƃ���j
					if( ret_rct == OK && ret_jnl == OK ){

						// ڼ�āA�ެ��قƂ��Ɉ󎚉\�̏ꍇ
						rct_goukei_pri = ON;					// ���v�L�^�󎚏�ԁiڼ�ėp�j���
						jnl_goukei_pri = ON;					// ���v�L�^�󎚏�ԁi�ެ��ٗp�j���
					}
					else{
						// ڼ�āA�ެ��قǂ��炩����ł��󎚕s�̏ꍇ
							rct_proc_data.Printing = 0;
							jnl_proc_data.Printing = 0;
							rct_goukei_pri = OFF;				// ���v�L�^�󎚏�ԁiڼ�ėp�jؾ��
							jnl_goukei_pri = OFF;				// ���v�L�^�󎚏�ԁi�ެ��ٗp�jؾ��
							ret = NG;
					}
				}
				else{

					//	���v�L�^�󎚈ȊO�̏ꍇ�A�󎚉\��������ֈ󎚂��s��

					if( ret_rct == NG && ret_jnl == NG ){

						// ڼ�āA�ެ��قƂ��Ɉ󎚕s�̏ꍇ
						ret = NG;
					}
					else if( ret_rct == OK && ret_jnl == NG ){

						// ڼ�Ă݈̂󎚉̏ꍇ
						terget_pri = R_PRI;						// �����Ώۂ�ڼ��������݂̂Ƃ���
					}
					else if( ret_rct == NG && ret_jnl == OK ){

						// �ެ��ق݈̂󎚉̏ꍇ
						terget_pri = J_PRI;						// �����Ώۂ�ެ���������݂̂Ƃ���
					}
				}

				break;

			default:
				ret = NG;		// ���̑��i�������ʴװ�j
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	}
	else{
		//	�󎚏������̏ꍇ

		if( next_prn_msg == ON ){					// �󎚏������̎��ҏW�v���H

			//	�󎚏������̎��ҏW�v��

			switch( terget_pri ){
				case	R_PRI:						// ڼ��
					ret = PriOutCheck(R_PRI);		// ������o�͉\����(ڼ��)
					break;
				case	J_PRI:						// �ެ���
					ret = PriOutCheck(J_PRI);		// ������o�͉\�����i�ެ��فj
					break;
				default:							// ���̑��i�������ʴװ�j
					ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}
			if( ret == OK){							// ����������n�j�H
				msg->command &= MSG_CMND_MASK;		// Ͻ��׸ނ�ؾ�Ă�����ޕ����݂̂Ƃ���
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
				if (terget_pri == J_PRI) {
					if (jnl_proc_data.Split == 0) {
						// ���d��̍Ĉ󎚊J�n���Ƀf�[�^�����݊J�n��ʒm����
						PrnCmd_WriteStartEnd(0, msg);
					}
					else if (jnl_proc_data.Final != 0) {
						// �ŏI�u���b�N�󎚊������ėv��ꍇ�͈󎚏������Ȃ�
						ret = NG;
					}
				}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			}
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ󎚏����^�s������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriRctCheck( )										|*/
/*|																			|*/
/*|	PARAMETER		:	NON													|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �󎚏�����									|*/
/*|							NG : �󎚏����s��								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriRctCheck( void )
{
	uchar	ret = OK;		// �߂�l

	if( ((rct_proc_data.Printing == 0) && (PriStsCheck(R_PRI) != PRI_ERR_NON)) ||
		((rct_proc_data.Printing != 0) && (PriOutCheck(R_PRI) != OK)) ) {
	//  (�󎚏������łȂ�              && �󎚕s��)                            ||
	//  (�󎚏�����                    && �ҏW�J�nNG)
		ret = NG;
	}

	return( ret );
}


/*----------------------------------------------------------------------------------------------*/
/*		��Mү���ޑΉ�����																		*/
/*----------------------------------------------------------------------------------------------*/

/*[]-----------------------------------------------------------------------[]*/
/*|		�̎��؈󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRYOUSYUU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnRYOUSYUU( MSG *msg )
{
	T_FrmReceipt	*msg_data;		// ��Mү�����ް��߲���
	Receipt_data	*Ryousyuu;		// �̎��؈��ް��߲���
	date_time_rec	*PriTime;		// ������ē����ް��߲���


	msg_data = (T_FrmReceipt *)msg->data;	// ��Mү�����ް��߲������
	Ryousyuu = msg_data->prn_data;			// �̎��؈��ް��߲������
	PriTime = &msg_data->PriTime;			// ������ē����ް��߲���

	if( next_prn_msg == OFF ){				// �V�K�̗̎��؈󎚗v���H

		//	�V�K�̗̎��؈󎚗v���̏ꍇ
		PrnRYOUSYUU_datachk( Ryousyuu );	// �̎����ް�����
		if( msg_data->reprint == OFF ){
			// �ʏ�󎚗v��
			memset( &Repri_Time, 0, sizeof(Repri_Time) );							// �Ĕ��s�����O�ر
			Repri_kakari_no = 0;													// ������ČW��No. 0�ر
		}
		else{
			// �Ĕ��s�󎚗v��
			memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// �Ĕ��s�����ް����
			Repri_kakari_no = msg_data->kakari_no;									// ������ČW��No.���
		}
		if( Ryousyuu->testflag == ON ){
			// �e�X�g�󎚗v��
			memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// �e�X�g���s�����ް����
		}
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
				if( isNewModel_R() && next_prn_msg == OFF ){
					// �V���V�[�g�v�����^�̏ꍇ�͗̎��؈󎚂𗭂߈󎚂Ƃ���
					if( !IS_INVOICE && Ryousyuu->WFusoku && prm_get(COM_PRM, S_SYS, 44, 1, 1) == 0 ){
						// ��C���{�C�X�a��ؒP�Ɣ��s���݈̂󎚊����܂őҋ@����(200ms)
						// ���V�[�g�v�����^�N�����͗��߈󎚕s�̈�
						xPause_PRNTSK(20);
					}

					// �f�[�^�����݊J�n
					PCMD_WRITE_START(R_PRI);
				}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
				RYOUSYUU_edit( Ryousyuu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �̎��؈��ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				RYOUSYUU_edit( Ryousyuu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �̎��؈��ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				RYOUSYUU_edit( Ryousyuu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �̎��؈��ް��ҏW�iڼ�āj
				RYOUSYUU_edit( Ryousyuu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �̎��؈��ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		// ���V�[�g�v�����^�̗̎��ؕҏW�����I��
		if( rct_proc_data.Final == ON && terget_pri == R_PRI ){
			if( isNewModel_R() ){													// �V�v�����^
				if( msg_data->dummy != 1 ){											// �X�g�b�N�Ώۃf�[�^�ł͂Ȃ�
					// �W���[�i���󎚊�����҂��Ȃ��f�[�^�̂���
					// �󎚊J�n�i�������ݏI���j�𑗐M����
					MsgSndFrmPrn(PREQ_STOCK_PRINT, R_PRI, 0);
				}
				else{																// �X�g�b�N�Ώۃf�[�^
					// �W���[�i���󎚊�����҂f�[�^�̂���
					// �󎚊J�n�i�������ݏI���j�͂܂����M���Ȃ�
					// RP_I2CSndReq( I2C_EVENT_QUE_REQ )�����{���������߃v�����^�N���������s��
					PrnOut( R_PRI );
				}
				// �V�v�����^�ł̏I�������iEnd_Set�j�͂����ł͂��Ȃ�
				return;
			}
		}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�a��؈󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnAZUKARI( MSG *msg )
{
	T_FrmAzukari	*msg_data;		// ��Mү�����ް��߲���
	Azukari_data	*Azukari;		// �a��؈��ް��߲���


	msg_data = (T_FrmAzukari *)msg->data;	// ��Mү�����ް��߲������
	Azukari = msg_data->prn_data;			// �a��؈��ް��߲������


	PrnAZUKARI_datachk( Azukari );			// �a����ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			AZUKARI_edit( Azukari, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �a��؈��ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			AZUKARI_edit( Azukari, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �a��؈��ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			AZUKARI_edit( Azukari, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �a��؈��ް��ҏW�iڼ�āj
			AZUKARI_edit( Azukari, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �a��؈��ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��t���󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnUKETUKE( MSG *msg )
{
	T_FrmUketuke	*msg_data;		// ��Mү�����ް��߲���
	Uketuke_data	*Uketuke;		// ��t�����ް��߲���
	date_time_rec	*PriTime;		// ������ē����ް��߲���


	msg_data = (T_FrmUketuke *)msg->data;	// ��Mү�����ް��߲������
	Uketuke = msg_data->prn_data;			// ��t�����ް��߲������
	PriTime = &msg_data->PriTime;			// ������ē����ް��߲���


	PrnUKETUKE_datachk( Uketuke );			// ��t���ް�����

	if( msg_data->reprint == OFF ){
		// �ʏ�󎚗v��
		memset( &Repri_Time, 0, sizeof(Repri_Time) );								// �Ĕ��s�����O�ر
		Repri_kakari_no = 0;														// ������ČW��No. 0�ر
	}
	else if( msg_data->reprint == 2 ){
		memset( &Repri_Time, 0, sizeof(Repri_Time) );								// �Ĕ��s�����O�ر
		Repri_kakari_no = msg_data->kakari_no;										// ������ČW��No.���
	}
	else{
		// �Ĕ��s�󎚗v��
		memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );		// �Ĕ��s�����ް����
		Repri_kakari_no = msg_data->kakari_no;										// ������ČW��No.���
	}

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			UKETUKE_edit( Uketuke, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��t�����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			UKETUKE_edit( Uketuke, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��t�����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			UKETUKE_edit( Uketuke, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��t�����ް��ҏW�iڼ�āj
			UKETUKE_edit( Uketuke, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��t�����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�W�v�󎚗v��ү���ގ�M����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSYUUKEI( MSG *msg )
{
	T_FrmSyuukei	*msg_data;		// ��Mү�����ް��߲���
	SYUKEI			*Syuukei;		// �W�v���ް��߲���
	date_time_rec	*PriTime;		// ������ē����ް��߲���
// MH810105 GG119202(S) T���v�A���󎚑Ή�
	T_FrmSyuukeiEc	*msg_ec;
// MH810105 GG119202(E) T���v�A���󎚑Ή�


	msg_data = (T_FrmSyuukei *)msg->data;	// ��Mү�����ް��߲������
	Syuukei = msg_data->prn_data;			// �W�v���ް��߲������
	PriTime = &msg_data->PriTime;			// ������ē����ް��߲���

	if( next_prn_msg == OFF ){				// �V�K�̏W�v�󎚗v���H

		//	�V�K�̏W�v�󎚗v���̏ꍇ
		PrnSYUUKEI_datachk( Syuukei );											// �W�v�ް�����
		memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// ������ē����ް����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

// MH810105 GG119202(S) T���v�A���󎚑Ή�
		if (msg_data->print_flag != 0) {
			// T���v�A����
			msg_ec = (T_FrmSyuukeiEc *)msg->data;
			switch (terget_pri) {
			case	R_PRI:
				SYUUKEI_EC_edit( Syuukei, R_PRI, msg_data->print_flag, &msg_ec->Ec, (PRN_PROC_DATA *)&rct_proc_data );	// �W�v���ް��ҏW�iڼ�āj
				break;
			case	J_PRI:
				SYUUKEI_edit( Syuukei, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �W�v���ް��ҏW�i�ެ��فj
				break;
			default:
				return;
			}
		}
		else {
// MH810105 GG119202(E) T���v�A���󎚑Ή�
		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SYUUKEI_edit( Syuukei, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �W�v���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SYUUKEI_edit( Syuukei, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �W�v���ް��ҏW�i�ެ��فj
				break;


			default:			// ���̑��i�������ʴװ�j
				return;
		}
// MH810105 GG119202(S) T���v�A���󎚑Ή�
		}
// MH810105 GG119202(E) T���v�A���󎚑Ή�
		End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�W�v�����󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSYUUKEI_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SYUUKEILOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �W�v�������ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SYUUKEILOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �W�v�������ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				SYUUKEILOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �W�v�������ް��ҏW�iڼ�āj
				SYUUKEILOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �W�v�������ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��݋��ɏW�v�󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_SK( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCOKI_SK( MSG *msg )
{
	T_FrmCoSyuukei	*msg_data;		// ��Mү�����ް��߲���
	COIN_SYU		*Coinkinko;		// ��݋��ɏW�v�ް��߲���
	date_time_rec	*PriTime;		// ������ē����ް��߲���

	msg_data	= (T_FrmCoSyuukei *)msg->data;	// ��Mү�����ް��߲������
	Coinkinko	= msg_data->prn_data;			// ��݋��ɏW�v�ް��߲������
	PriTime		= &msg_data->PriTime;			// ������ē����ް��߲���

	PrnCOKI_SK_datachk( Coinkinko );												// ��݋��ɏW�v�ް�����
	memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );			// ������ē����ް����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			CoKiSk_edit( Coinkinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��݋��ɏW�v���ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			CoKiSk_edit( Coinkinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��݋��ɏW�v���ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			CoKiSk_edit( Coinkinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��݋��ɏW�v���ް��ҏW�iڼ�āj
			CoKiSk_edit( Coinkinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��݋��ɏW�v���ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��݋��ɏW�v���󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_JO( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCOKI_JO( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				CoKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��݋��ɏW�v�����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				CoKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��݋��ɏW�v�����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				CoKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��݋��ɏW�v�����ް��ҏW�iڼ�āj
				CoKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��݋��ɏW�v�����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������ɏW�v�󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_SK( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSIKI_SK( MSG *msg )
{
	T_FrmSiSyuukei	*msg_data;		// ��Mү�����ް��߲���
	NOTE_SYU		*Siheikinko;	// �������ɏW�v�ް��߲���
	date_time_rec	*PriTime;		// ������ē����ް��߲���

	msg_data	= (T_FrmSiSyuukei *)msg->data;	// ��Mү�����ް��߲������
	Siheikinko	= msg_data->prn_data;			// �������ɏW�v�ް��߲������
	PriTime		= &msg_data->PriTime;			// ������ē����ް��߲���

	PrnSIKI_SK_datachk( Siheikinko );											// �������ɏW�v�ް�����
	memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );		// ������ē����ް����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			SiKiSk_edit( Siheikinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �������ɏW�v���ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			SiKiSk_edit( Siheikinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �������ɏW�v���ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			SiKiSk_edit( Siheikinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �������ɏW�v���ް��ҏW�iڼ�āj
			SiKiSk_edit( Siheikinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �������ɏW�v���ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������ɏW�v���󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_JO( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSIKI_JO( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SiKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �������ɏW�v�����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SiKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �������ɏW�v�����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				SiKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �������ɏW�v�����ް��ҏW�iڼ�āj
				SiKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �������ɏW�v�����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );				// ���ް��P��ۯ��ҏW�I��������

	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ޑK�Ǘ��W�v�󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTURIKAN( MSG *msg )
{
	T_FrmTuriKan	*msg_data;		// ��Mү�����ް��߲���
	TURI_KAN		*TuriKan;		// �ޑK�Ǘ��W�v���ް��߲���


	msg_data = (T_FrmTuriKan *)msg->data;	// ��Mү�����ް��߲������
	TuriKan = msg_data->prn_data;			// �ޑK�Ǘ��W�v���ް��߲������

	if( next_prn_msg == OFF ){				// �V�K�̒ޑK�Ǘ��W�v�󎚗v���H

		//	�V�K�̏W�v�󎚗v���̏ꍇ
		PrnTURIKAN_datachk( TuriKan );											// �ޑK�Ǘ��W�v�ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				TURIKAN_edit( TuriKan, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ޑK�Ǘ��W�v���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				TURIKAN_edit( TuriKan, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ޑK�Ǘ��W�v���ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				TURIKAN_edit( TuriKan, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ޑK�Ǘ��W�v���ް��ҏW�iڼ�āj
				TURIKAN_edit( TuriKan, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ޑK�Ǘ��W�v���ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ޑK�Ǘ��W�v�����󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTURIKAN_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				TURIKANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ޑK�Ǘ��W�v�������ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				TURIKANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ޑK�Ǘ��W�v�������ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				TURIKANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ޑK�Ǘ��W�v�������ް��ҏW�iڼ�āj
				TURIKANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ޑK�Ǘ��W�v�������ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );				// ���ް��P��ۯ��ҏW�I��������

	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�G���[���󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERR_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERR_JOU( MSG *msg )
{
	T_FrmErrJou		*msg_data;		// ��Mү�����ް��߲���
	Err_log			*Errlog;		// �װ����ް��߲���

	msg_data	= (T_FrmErrJou *)msg->data;			// ��Mү�����ް��߲������
	Errlog		= msg_data->prn_data;				// �װ����ް��߲������

	PrnERRJOU_datachk( Errlog );					// �װ����ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			ERRJOU_edit( Errlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �װ�����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			ERRJOU_edit( Errlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �װ�����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			ERRJOU_edit( Errlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �װ�����ް��ҏW�iڼ�āj
			ERRJOU_edit( Errlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �װ�����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�G���[��񗚗��󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERR_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERR_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnERRLOG_datachk( msg_data );			// �װ��񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				ERRLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				ERRLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				ERRLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				ERRLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�A���[�����󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARM_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnARM_JOU( MSG *msg )
{
	T_FrmArmJou		*msg_data;		// ��Mү�����ް��߲���
	Arm_log			*Armlog;		// �װя���ް��߲���

	msg_data	= (T_FrmArmJou *)msg->data;			// ��Mү�����ް��߲������
	Armlog		= msg_data->prn_data;				// �װя���ް��߲������

	PrnARMJOU_datachk( Armlog );					// �װя���ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			ARMJOU_edit( Armlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �װя����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			ARMJOU_edit( Armlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �װя����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			ARMJOU_edit( Armlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �װя����ް��ҏW�iڼ�āj
			ARMJOU_edit( Armlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �װя����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�A���[����񗚗��󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARM_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnARM_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnARMLOG_datachk( msg_data );			// �װя�񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				ARMLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װя�񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				ARMLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װя�񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				ARMLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װя�񗚗����ް��ҏW�iڼ�āj
				ARMLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װя�񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPE_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOPE_JOU( MSG *msg )
{
	T_FrmOpeJou		*msg_data;		// ��Mү�����ް��߲���
	Ope_log			*Opelog;		// �������ް��߲���

	msg_data	= (T_FrmOpeJou *)msg->data;			// ��Mү�����ް��߲������
	Opelog		= msg_data->prn_data;				// �������ް��߲������

	PrnOPEJOU_datachk( Opelog );					// �������ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			OPEJOU_edit( Opelog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ��������ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			OPEJOU_edit( Opelog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ��������ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			OPEJOU_edit( Opelog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ��������ް��ҏW�iڼ�āj
			OPEJOU_edit( Opelog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ��������ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�����񗚗��󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPE_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOPE_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnOPELOG_datachk( msg_data );			// �����񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				OPELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				OPELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				OPELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����񗚗����ް��ҏW�iڼ�āj
				OPELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���j�^���󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMON_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	machida kei													|*/
/*|	Date	:	2005-11-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMON_JOU( MSG *msg )
{
	T_FrmMonJou		*msg_data;		// ��Mү�����ް��߲���
	Mon_log			*Monlog;		// �ް��߲���

	msg_data	= (T_FrmMonJou *)msg->data;			// ��Mү�����ް��߲������
	Monlog		= msg_data->prn_data;				// �ް��߲������

	PrnMONJOU_datachk( Monlog );					// �ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			MONJOU_edit( Monlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			MONJOU_edit( Monlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			MONJOU_edit( Monlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ް��ҏW�iڼ�āj
			MONJOU_edit( Monlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���j�^��񗚗��󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMON_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	R.HARA														|*/
/*|	Date	:	2006-02-28													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnMON_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnMONLOG_datachk( msg_data );			// ���j�^��񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				MONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				MONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				MONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����񗚗����ް��ҏW�iڼ�āj
				MONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}


/*[]-----------------------------------------------------------------------[]*/
/*|		�s�����O�󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNG_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-02-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnNG_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		NgLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �s�������O���ް��ҏW�iڼ�āj
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���o�Ƀ��O�󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnIO_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnIO_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				IoLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �s�������O���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				IoLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �s�������O���ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				IoLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �s�������O���ް��ҏW�iڼ�āj
				IoLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �s�������O���ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}


/*[]-----------------------------------------------------------------------[]*/
/*|		�ݒ��ް��󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSETTEI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSETTEI( MSG *msg )
{
	T_FrmSetteiData	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmSetteiData *)msg->data;	// ��Mү�����ް��߲������


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ݒ��ް����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ݒ��ް����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				SETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �ݒ��ް����ް��ҏW�iڼ�āj
				SETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �ݒ��ް����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );															// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���춳�Ĉ󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnDOUSAC( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnDOUSAC( MSG *msg )
{
	T_FrmDousaCnt	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmDousaCnt *)msg->data;		// ��Mү�����ް��߲������

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			DOUSAC_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���춳�Ĉ��ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			DOUSAC_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���춳�Ĉ��ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			DOUSAC_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���춳�Ĉ��ް��ҏW�iڼ�āj
			DOUSAC_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���춳�Ĉ��ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );															// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ۯ����u���춳�Ĉ󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_DCNT( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-07-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_DCNT( MSG *msg )
{
	T_FrmLockDcnt	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmLockDcnt *)msg->data;				// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ۯ����u���춳�Ĉ��ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ۯ����u���춳�Ĉ��ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ۯ����u���춳�Ĉ��ް��ҏW�iڼ�āj
				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ۯ����u���춳�Ĉ��ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );																// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�Ԏ����Ұ��󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_PARA( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_PARA( MSG *msg )
{
	T_FrmLockPara	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmLockPara *)msg->data;				// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				LOCKPARA_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����Ұ����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				LOCKPARA_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����Ұ����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				LOCKPARA_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����Ұ����ް��ҏW�iڼ�āj
				LOCKPARA_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����Ұ����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );																// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ۯ����u�ݒ�󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_SETTEI( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_SETTEI( MSG *msg )
{
	T_FrmLockSettei	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmLockSettei *)msg->data;			// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				LOCKSETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ۯ����u�ݒ���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				LOCKSETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ۯ����u�ݒ���ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				LOCKSETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ۯ����u�ݒ���ް��ҏW�iڼ�āj
				LOCKSETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ۯ����u�ݒ���ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );																// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�Ԏ����i���݁j�󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPKJOU_NOW( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPKJOU_NOW( MSG *msg )
{
	T_FrmPkjouNow	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmPkjouNow *)msg->data;				// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��
				PKJOUNOW_edit( msg_data->Kikai_no, R_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����i���݁j���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���
				PKJOUNOW_edit( msg_data->Kikai_no, J_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����i���݁j���ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���
				PKJOUNOW_edit( msg_data->Kikai_no, R_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����i���݁j���ް��ҏW�iڼ�āj
				PKJOUNOW_edit( msg_data->Kikai_no, J_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����i���݁j���ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );																// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�Ԏ����i�ޔ��j�󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPKJOU_SAV( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPKJOU_SAV( MSG *msg )
{
	T_FrmPkjouSav	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmPkjouSav *)msg->data;				// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				PKJOUSAV_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����i�ޔ��j���ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				PKJOUSAV_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����i�ޔ��j���ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				PKJOUSAV_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �Ԏ����i�ޔ��j���ް��ҏW�iڼ�āj
				PKJOUSAV_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �Ԏ����i�ޔ��j���ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );																// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		����L���^�����󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKIDATA1( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIKIDATA1( MSG *msg )
{
	T_FrmTeikiData1	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmTeikiData1 *)msg->data;			// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){							// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnTEIKID1_datachk( msg_data );					// ����L���^�����󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				TEIKID1_edit(							// ����L���^�������ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// �ެ���

				TEIKID1_edit(							// ����L���^�������ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				TEIKID1_edit(							// ����L���^�������ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				TEIKID1_edit(							// ����L���^�������ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );						// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������Ɂ^�o�Ɉ󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKIDATA2( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIKIDATA2( MSG *msg )
{
	T_FrmTeikiData2	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmTeikiData2 *)msg->data;			// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){							// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnTEIKID2_datachk( msg_data );					// ������Ɂ^�o�Ɉ󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				TEIKID2_edit(							// ������Ɂ^�o�Ɉ��ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// �ެ���

				TEIKID2_edit(							// ������Ɂ^�o�Ɉ��ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				TEIKID2_edit(							// ������Ɂ^�o�Ɉ��ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				TEIKID2_edit(							// ������Ɂ^�o�Ɉ��ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );						// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���޽�������󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSVSTIK_KIGEN( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSVSTIK_KIGEN( MSG *msg )
{
	T_FrmStikKigen	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmStikKigen *)msg->data;		// ��Mү�����ް��߲������

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���޽���������ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���޽���������ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���޽���������ް��ҏW�iڼ�āj
			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���޽���������ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );															// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���ʓ��󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSPLDAY( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-26													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSPLDAY( MSG *msg )
{
	T_FrmSplDay	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmSplDay *)msg->data;		// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ʓ����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ʓ����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ʓ����ް��ҏW�iڼ�āj
				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ʓ����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );															// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�◯�ԏ��󎚗v��ү���ގ�M����									*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIRYUU_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-07-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIRYUU_JOU( MSG *msg )
{
	T_FrmTeiRyuuJou	*msg_data;					// ��Mү�����ް��߲���

	msg_data = (T_FrmTeiRyuuJou *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				TRJOU_edit(										// �◯�ԏ����ް��ҏW�iڼ�āj
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							R_PRI,
							(PRN_PROC_DATA *)&rct_proc_data );

				break;

			case	J_PRI:		// �ެ���

				TRJOU_edit(										// �◯�ԏ����ް��ҏW�i�ެ��فj
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							J_PRI,
							(PRN_PROC_DATA *)&jnl_proc_data );

				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				TRJOU_edit(										// �◯�ԏ����ް��ҏW�iڼ�āj
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							R_PRI,
							(PRN_PROC_DATA *)&rct_proc_data );

				TRJOU_edit(										// �◯�ԏ����ް��ҏW�i�ެ��فj
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							J_PRI,
							(PRN_PROC_DATA *)&jnl_proc_data );

				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );								// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�╜�d���󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIFUK_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIFUK_JOU( MSG *msg )
{
	T_FrmTeiFukJou	*msg_data;		// ��Mү�����ް��߲���
	Pon_log			*Ponlog;		// �╜�d����ް��߲���

	msg_data	= (T_FrmTeiFukJou *)msg->data;			// ��Mү�����ް��߲������
	Ponlog		= &(msg_data->Ponlog);					// �╜�d����ް��߲������

	PrnTFJOU_datachk( Ponlog );							// �╜�d����ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			TFJOU_edit( Ponlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �╜�d�����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			TFJOU_edit( Ponlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �╜�d�����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			TFJOU_edit( Ponlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �╜�d�����ް��ҏW�iڼ�āj
			TFJOU_edit( Ponlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �╜�d�����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�╜�d�����󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIFUK_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIFUK_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��
				TFLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �╜�d�������ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���
				TFLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �╜�d�������ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���
				TFLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �╜�d�������ް��ҏW�iڼ�āj
				TFLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �╜�d�������ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );															// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ʐ��Z���󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSEISAN_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSEISAN_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
				SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�s���E�����o�ɏ��󎚗v��ү���ގ�M����							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFUSKYO_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFUSKYO_JOU( MSG *msg )
{
	T_FrmFusKyo		*msg_data;		// ��Mү�����ް��߲���
	flp_log			*fuskyo;		// �s�������o�ɏ���ް��߲���

	msg_data	= (T_FrmFusKyo *)msg->data;				// ��Mү�����ް��߲������
	fuskyo		= (flp_log *)(msg_data->prn_data);		// �s�������o�ɏ���ް��߲�����

	PrnFKJOU_datachk( fuskyo );							// �s�������o�ɏ���ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			FUSKYOJOU_edit( fuskyo, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �s�������o�ɏ����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			FUSKYOJOU_edit( fuskyo, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �s�������o�ɏ����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			FUSKYOJOU_edit( fuskyo, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �s�������o�ɏ����ް��ҏW�iڼ�āj
			FUSKYOJOU_edit( fuskyo, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �s�������o�ɏ����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�s���E�����o�ɗ����󎚗v��ү���ގ�M����							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFUSKYO_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-15													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFUSKYO_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				FUSKYOLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �s���E�����o�ɗ������ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				FUSKYOLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �s���E�����o�ɗ������ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				FUSKYOLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �s���E�����o�ɗ������ް��ҏW�iڼ�āj
				FUSKYOLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �s���E�����o�ɗ������ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�p�����֎��e�X�g�󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnIREKAE_TEST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnIREKAE_TEST( MSG *msg )
{

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			IREKAETST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �p�����֎�ýĈ��ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			IREKAETST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �p�����֎�ýĈ��ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			IREKAETST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// �p�����֎�ýĈ��ް��ҏW�iڼ�āj
			IREKAETST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// �p�����֎�ýĈ��ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );																	// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�����ýĈ󎚗v��ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPRINT_TEST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-06													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPRINT_TEST( MSG *msg )
{


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				PRITEST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����ýĈ��ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				PRITEST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����ýĈ��ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				PRITEST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �����ýĈ��ް��ҏW�iڼ�āj
				PRITEST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �����ýĈ��ް��ҏW�i�ެ��فj

				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );											// ���ް��P��ۯ��ҏW�I��������
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���S�󎚃f�[�^�o�^�v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOGO_REGIST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-07													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOGO_REGIST( MSG *msg )
{

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			LOGOREG_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���S�󎚃f�[�^�o�^�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			LOGOREG_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���S�󎚃f�[�^�o�^�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			LOGOREG_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���S�󎚃f�[�^�o�^�iڼ�āj
			LOGOREG_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���S�󎚃f�[�^�o�^�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );											// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚒��~ү���ގ�M����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI( MSG *msg )
{
	switch( terget_pri ){	// �Ώ�������H

		case	R_PRI:		// ڼ��
			PrnINJI_TYUUSHI_R( msg );	// �󎚒��~ү���ގ�M�����iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			PrnINJI_TYUUSHI_J( msg );	// �󎚒��~ү���ގ�M�����i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnINJI_TYUUSHI_R( msg );	// �󎚒��~ү���ގ�M�����iڼ�āj
			PrnINJI_TYUUSHI_J( msg );	// �󎚒��~ү���ގ�M�����i�ެ��فj
			break;

		default:			// ���̑��i�Ώ�������װ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ󎚒��~ү���ގ�M����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI_R( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI_R( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// �ҏW���������ް��߲���


	p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// �ҏW���������ް��߲������

	if( p_proc_data->Printing != 0 ){				// �󎚏������H

		//	�󎚏������̏ꍇ
		if( p_proc_data->Tyushi_Cmd == OFF ){		// �󎚒��~�v������M�H

			Rct_top_edit( ON );						// ڼ�Đ擪���ް��ҏW�����iۺވ󎚁^ͯ�ް�󎚁^�p����Đ���j
			PrnOut( R_PRI );						// ������N��

			p_proc_data->Tyushi_Cmd = ON;			// �󎚒��~ү���ގ�M�׸�ON
			p_proc_data->Final = OFF;				// �����󎚍ŏI��ۯ��ʒm�FOFF
		}
	}
	else{
		//	�󎚏������łȂ��ꍇ
		Inji_Cancel( msg, R_PRI );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و󎚒��~ү���ގ�M����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI_J( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI_J( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// �ҏW���������ް��߲���


	p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// �ҏW���������ް��߲������

	if( p_proc_data->Printing != 0 ){				// �󎚏������H

		//	�󎚏������̏ꍇ
		if( p_proc_data->Tyushi_Cmd == OFF ){		// �󎚒��~�v������M�H

			p_proc_data->Tyushi_Cmd = ON;			// �󎚒��~ү���ގ�M�׸�ON
			p_proc_data->Final = OFF;				// �����󎚍ŏI��ۯ��ʒm�FOFF
		}
	}
	else{
		//	�󎚏������łȂ��ꍇ
		Inji_Cancel( msg, J_PRI );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚕ҏW�ް��P��ۯ��󎚏I��ү���ގ�M����							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_END( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_END( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// �ҏW���������ް��߲���
	MSG				*p_next_msg;		// �󎚗v��ү���ޕۑ��ޯ̧�߲��
	uchar			gyou_cnt;			// �󎚗v���s��
	ushort			timer;				// ���ް��ҏW�x����ϰ�l


	switch( terget_pri ){	// �Ώ�������H

		case	R_PRI:		// 	ڼ��

			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// ڼ�ĕҏW���������ް��߲�����
			p_next_msg	= (MSG *)&NextMsg_r;				// ڼ�Ĉ󎚗v��ү���ޕۑ��ޯ̧�߲�����

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
			GyouCnt_All_r += GyouCnt_r;						// ���M�ς݈��ް��S�s���擾
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
			gyou_cnt	= GyouCnt_r;						// ���M�ς݈��ް��s���擾
			GyouCnt_r	= 0;								// ���M�ς݈��ް��s���O�ر

			break;

		case	J_PRI:		// 	�ެ���

			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// �ެ��ٕҏW���������ް��߲�����
			p_next_msg	= (MSG *)&NextMsg_j;				// �ެ��و󎚗v��ү���ޕۑ��ޯ̧�߲�����

// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			GyouCnt_All_j += GyouCnt_j;						// ���M�ς݈��ް��S�s���擾
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			gyou_cnt	= GyouCnt_j;						// ���M�ς݈��ް��s���擾
			GyouCnt_j	= 0;								// ���M�ς݈��ް��s���O�ر

			break;

		default:			// ���̑��i�Ώ�������װ�j
			return;
	}
	if( p_proc_data->Printing != 0 ){

		// �󎚏������̏ꍇ

		if( (p_proc_data->Final != ON) && (p_next_msg->command != 0) ){

			// �ŏI��ۯ��̈󎚏I���łȂ��ꍇ

			if( p_proc_data->EditWait == OFF ){					// ���ް��ҏW�x����ϰ�N����ԁH

				// �x����ϰ�N�����łȂ��ꍇ

				timer = (ushort)(PRN_WAIT_TIMER * gyou_cnt);	// ���ް��ҏW�x����ϰ�v�Z�i���M�ς݈��ް��s���~��ϰ�l�j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
				if (terget_pri == J_PRI && isEJA_USE()) {
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//					// �d�q�W���[�i���͒x���Ȃ��Ƃ���
//					timer = 0;
					// �d�q�W���[�i���ւ̈󎚃f�[�^��1�u���b�N���Ƃɏ����݊J�n�E�I���𑗐M���Ȃ��悤�ύX�����̂�
					// �Œ�Œx���^�C�}�𓮍삳����i20�~5��100ms�j
					timer = 5;
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

				if( timer == 0 ){
					// ���ް��ҏW�x���Ȃ�
					PrnNext( p_next_msg, terget_pri );			// ���ް�����ۯ��ҏW�v��ү���ޑ��M
				}
				else{
					// ���ް��ҏW�x������
					if( terget_pri == R_PRI ){
						Lagtim( PRNTCBNO , 5 , timer  );		// ���ް��ҏW�x����ϰ���āiڼ�āj
					}
					else{
						Lagtim( PRNTCBNO , 6 , timer  );		// ���ް��ҏW�x����ϰ���āi�ެ��فj
					}
					p_proc_data->EditWait = ON;					// ���ް��ҏW�x����ϰ�N����Ծ��
				}
				p_proc_data->Split ++;							// �󎚏���������ۯ����{�P
			}
		}
		else{
			// �ŏI��ۯ��̈󎚏I���̏ꍇ
			prn_proc_data_clr( terget_pri );				// ��������̐���ر������
			p_next_msg->command = 0;
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
			if (eja_prn_buff.PrnClkReqFlg != 0) {
				PrnCmd_Clock();
				eja_prn_buff.PrnClkReqFlg = 0;
			}
			if (eja_prn_buff.PrnInfReqFlg != 0) {
				PrnCmd_InfoReq(eja_prn_buff.PrnInfReqFlg);
				eja_prn_buff.PrnInfReqFlg = 0;
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚈ُ�I�����䏈��												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERREND_PROC( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERREND_PROC( void )
{
	PRN_DATA_BUFF	*p_data_buff;		// ���������M�ް��Ǘ��ް��߲���
	PRN_PROC_DATA	*p_proc_data;		// �ҏW���������ް��߲���
	ushort			command;			// �󎚏����������
	uchar			pri_sts;			// ������ð��


	switch( terget_pri ){	// �Ώ�������H

		case	R_PRI:		// 	ڼ��

			p_data_buff = (PRN_DATA_BUFF *)&rct_prn_buff;	// ���������M�ް��Ǘ��ް��߲����iڼ�āj
			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// �ҏW���������ް��߲�����		�iڼ�āj
			break;

		case	J_PRI:		// 	�ެ���

			p_data_buff = (PRN_DATA_BUFF *)&jnl_prn_buff;	// ���������M�ް��Ǘ��ް��߲����i�ެ��فj
			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// �ҏW���������ް��߲�����		�i�ެ��فj
			break;

		default:			// ���̑��i�Ώ�������װ�j
			return;
	}
	command	= p_proc_data->Printing;						// �󎚏������̈󎚗v�������			�擾
	pri_sts	= (uchar)(p_data_buff->PrnState[0] & 0x0e);		// ��������ݽð���iƱ���޽ð���͏����j	�擾

// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//	if( command != 0 && pri_sts != 0 ){
	if( command != 0 && pri_sts != 0 && isPrnRetryOver() ){
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j

		//	�󎚏������@���@�󎚕s��ԁ@�̏ꍇ
		Inji_ErrEnd( command, PRI_ERR_STAT, terget_pri );	// �󎚈ُ�I�������i�ُ�I��ү���ޑ��M�j
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�̎��؈󎚗v���ް���������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRYOUSYUU_datachk( *Ryousyuu )					|*/
/*|																			|*/
/*|	PARAMETER		:	Receipt_data *Ryousyuu = �̎����ް��߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnRYOUSYUU_datachk( Receipt_data *Ryousyuu )
{
	uchar		ret = OK;		// ��������
	uchar		data_no;		// �������ް�No.
	wari_tiket	*p_wari_tiket;	// �������ް��߲���
	uchar		wtik_syu;		// ���������
	wari_tiket	wari_dt;

	//	�󎚎�ʂP�i�ʏ�^���d�j����
	if( NG == Prn_data_chk( RYOUSYUU_PRN_SYU1, (unsigned long)Ryousyuu->WFlag ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�󎚎�ʂQ�i�ʏ�^���Z���~�j����
	if( NG == Prn_data_chk( RYOUSYUU_PRN_SYU2, (unsigned long)Ryousyuu->chuusi ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���Ԉʒu�ް�����
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Ryousyuu->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���Z�������
	if( NG == Prn_data_chk( SEISAN_SYU, (unsigned long)Ryousyuu->Seisan_kind ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���Ɏ�������
	if( NG == DateChk( (date_time_rec *)&Ryousyuu->TInTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�o�Ɏ�������
	if( NG == DateChk( (date_time_rec *)&Ryousyuu->TOutTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�����������
	if( NG == Prn_data_chk( RYOUKIN_SYU, (unsigned long)Ryousyuu->syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�����
	if( Ryousyuu->teiki.id != 0 ){	// ������g�p�H
		//	������������
		if( NG == Prn_data_chk( TEIKI_SYU, (unsigned long)Ryousyuu->teiki.syu ) ){
			Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
			ret = NG;
		}
	}
	//	������
	for( data_no = 0 ; data_no < WTIK_USEMAX ; data_no++ ){

		disc_wari_conv( &Ryousyuu->DiscountData[data_no], &wari_dt );
		p_wari_tiket = &wari_dt;									// �����Ώۊ������ް��߲������
		wtik_syu = p_wari_tiket->tik_syu;							// ����������ް��擾

			//	�������ް�����
			switch( wtik_syu ){

				case	SERVICE:	// ���޽��
				case	C_SERVICE:	// ���Z���~���޽��
					//	���ԏ�m���D�������
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					//	���޽���������
					if( NG == Prn_data_chk( SERVICE_SYU, (unsigned long)p_wari_tiket->syubetu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
// MH810100(S) 2020/08/26 #4763�y���񐸎Z�Ŋ����𖞊z�K�p���ꂸ�ɍĐ��Z���s����E0251����������
// 0�������e����̂ŃR�����g�A�E�g
//					//	�g�p��������
//					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
//						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
//						ret = NG;
//					}
// MH810100(E) 2020/08/26 #4763�y���񐸎Z�Ŋ����𖞊z�K�p���ꂸ�ɍĐ��Z���s����E0251����������
					break;

				case	KAKEURI:	// �|����
				case	C_KAKEURI:	// ���Z���~�|����
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
				case	SHOPPING:	// ��������
				case 	C_SHOPPING:	// ��������
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
					//	���ԏ�m���D�������
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
// GG124100(S) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6580 �X�ԍ�101�ȏ�̊������g���Đ��Z�����ꍇ�A�󎚗v���f�[�^�G���[(E0251)���������� [���ʉ��P���� No1530]
// 					//	�X�m���D����
// 					if( NG == Prn_data_chk( MISE_NO, (unsigned long)p_wari_tiket->syubetu ) ){
// 						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
// 						ret = NG;
// 					}
// GG124100(E) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6580 �X�ԍ�101�ȏ�̊������g���Đ��Z�����ꍇ�A�󎚗v���f�[�^�G���[(E0251)���������� [���ʉ��P���� No1530]
// MH810100(S) 2020/08/26 #4763�y���񐸎Z�Ŋ����𖞊z�K�p���ꂸ�ɍĐ��Z���s����E0251����������
// 0�������e����̂ŃR�����g�A�E�g
//					//	�g�p��������
//					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
//						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
//						ret = NG;
//					}
// MH810100(E) 2020/08/26 #4763�y���񐸎Z�Ŋ����𖞊z�K�p���ꂸ�ɍĐ��Z���s����E0251����������
					break;

				case	KAISUU:		// �񐔌�
				case	C_KAISUU:	// ���Z���~�񐔌�
					//	���ԏ�m���D�������
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					//	�g�p��������
					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					break;

				case	WARIBIKI:	// ������
					//	���ԏ�m���D�������
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					//	�����������
					if( NG == Prn_data_chk( WARIBIKI_SYU, (unsigned long)p_wari_tiket->syubetu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					//	�g�p��������
					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					break;

				case	PREPAID:	// �v���y�C�h�J�[�h
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
						ret = NG;
					}
					break;

				default:		// ���̑�
					break;
			}
	}
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
	//	���Z�����Z�������
	if( NG == Prn_data_chk( UNPAID_SYU, (unsigned long)Ryousyuu->shubetsu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)

	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�a��؈󎚗v���ް���������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI_datachk( *Azukari )						|*/
/*|																			|*/
/*|	PARAMETER		:	Azukari_data *Azukari = �a����ް��߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnAZUKARI_datachk( Azukari_data *Azukari )
{
	uchar		ret = OK;	// ��������


	//	���Ԉʒu�ް�����
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Azukari->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	��������
	if( NG == DateChk( (date_time_rec *)&Azukari->TTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��t���󎚗v���ް���������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnUKETUKE_datachk( *Uketuke )						|*/
/*|																			|*/
/*|	PARAMETER		:	Uketuke_data *Uketuke = ��t���ް��߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnUKETUKE_datachk( Uketuke_data *Uketuke )
{
	uchar		ret = OK;	// ��������


	//	���Ԉʒu�ް�����
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Uketuke->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���s��������
	if( NG == DateChk( (date_time_rec *)&Uketuke->ISTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���Ɏ�������
	if( NG == DateChk( (date_time_rec *)&Uketuke->TTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�W�v�󎚗v���ް���������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI_datachk( *Syuukei )						|*/
/*|																			|*/
/*|	PARAMETER		:	SYUKEI *Syuukei = �W�v�ް��߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnSYUUKEI_datachk( SYUKEI *Syuukei )
{
	uchar	ret = OK;	// ��������


	//	����W�v��������
	if( NG == DateChk( (date_time_rec *)&Syuukei->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�O��W�v��������
	if( NG == DateChk( (date_time_rec *)&Syuukei->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��݋��ɏW�v�󎚗v���ް���������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_SK_datachk( *Coinkinko )					|*/
/*|																			|*/
/*|	PARAMETER		:	COIN_SYU	*Coinkinko	= ��݋��ɏW�v�ް��߲���		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnCOKI_SK_datachk( COIN_SYU	*Coinkinko )
{
	uchar	ret = OK;	// ��������


	//	����W�v��������
	if( NG == DateChk( (date_time_rec *)&Coinkinko->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�O��W�v��������
	if( NG == DateChk( (date_time_rec *)&Coinkinko->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������ɏW�v�󎚗v���ް���������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_SK_datachk( *Siheikinko )					|*/
/*|																			|*/
/*|	PARAMETER		:	NOTE_SYU	*Siheikinko	= �������ɏW�v�ް��߲���	|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnSIKI_SK_datachk( NOTE_SYU	*Siheikinko )
{
	uchar	ret = OK;	// ��������


	//	����W�v��������
	if( NG == DateChk( (date_time_rec *)&Siheikinko->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�O��W�v��������
	if( NG == DateChk( (date_time_rec *)&Siheikinko->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ޑK�Ǘ��W�v�󎚗v���ް���������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN_datachk( *TuriKan )						|*/
/*|																			|*/
/*|	PARAMETER		:	TURI_KAN *TuriKan = �ޑK�Ǘ��W�v�ް��߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTURIKAN_datachk( TURI_KAN *TuriKan )
{
	uchar	ret = OK;	// ��������


	//	����W�v��������
	if( NG == DateChk( (date_time_rec *)&TuriKan->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�O��W�v��������
	if( NG == DateChk( (date_time_rec *)&TuriKan->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�װ���󎚗v���ް���������												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERRJOU_datachk( *Errlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Err_log	*Errlog= �װ����ް��߲���						|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnERRJOU_datachk( Err_log *Errlog )
{
	uchar	ret = OK;	// ��������


	//	������������
	if( NG == DateChk( (date_time_rec *)&Errlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�װ�������
	if( Errlog->Errsyu > ERR_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�װ��������
	if( Errlog->Errcod > ERR_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�װ��񗚗��󎚗v���ް���������											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERRLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnERRLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�w���������
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�װя��󎚗v���ް���������											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARMJOU_datachk( *Errlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Arm_log	*Armlog	= �װя���ް��߲���					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnARMJOU_datachk( Arm_log *Armlog )
{
	uchar	ret = OK;	// ��������


	//	������������
	if( NG == DateChk( (date_time_rec *)&Armlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�װю������
	if( Armlog->Armsyu > ARM_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�װѺ�������
	if( Armlog->Armcod > ARM_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�װя�񗚗��󎚗v���ް���������										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARMLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-13														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnARMLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�w���������
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		������󎚗v���ް���������											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPEJOU_datachk( *Opelog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Ope_log	*Opelog	= �������ް��߲���					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnOPEJOU_datachk( Ope_log *Opelog )
{
	uchar	ret = OK;	// ��������


	//	���쎞������
	if( NG == DateChk( (date_time_rec *)&Opelog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	����������
	if( Opelog->OpeKind > OPE_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���캰������
	if( Opelog->OpeCode > OPE_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�����񗚗��󎚗v���ް���������										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPELOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-13														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnOPELOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�w���������
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		���j�^���󎚗v���ް���������											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMONJOU_datachk( *Monlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Mon_log	*Monlog	= ���j�^����ް��߲���					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	machida kei														|*/
/*|	Date	:	2005-11-30														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnMONJOU_datachk( Mon_log *Monlog )
{
	uchar	ret = OK;	// ��������


	//	������������
	if( NG == DateChk( (date_time_rec *)&Monlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���j�^�������
	if( Monlog->MonKind > MON_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���j�^��������
	if( Monlog->MonCode > MON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		���j�^��񗚗��󎚗v���ް���������										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMONLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	R.HARA															|*/
/*|	Date	:	2006-02-28														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
uchar	PrnMONLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�w���������
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		����L���^�����󎚗v���ް���������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKID1_datachk( *msg_data )						|*/
/*|																			|*/
/*|	PARAMETER		:	T_FrmTeikiData1 *msg_data = ү�����ް��߲���		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTEIKID1_datachk( T_FrmTeikiData1 *msg_data )
{
	uchar		ret = OK;		// ��������


	//	���ԏ�m���D�������
	if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)msg_data->Pkno_syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�󎚗v���������(0:�S�w��A1:�L���̂݁A2:�����̂�)
	if( msg_data->Req_syu > 2 ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������Ɂ^�o�Ɉ󎚗v���ް���������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKID2_datachk( *msg_data )						|*/
/*|																			|*/
/*|	PARAMETER		:	T_FrmTeikiData2 *msg_data = ү�����ް��߲���		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : ����										|*/
/*|							NG : �ُ�										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTEIKID2_datachk( T_FrmTeikiData2 *msg_data )
{
	uchar		ret = OK;		// ��������


	//	���ԏ�m���D�������
	if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)msg_data->Pkno_syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�󎚗v���������(0:�S�w��A1:���ɂ̂݁A2:�o�ɂ̂݁A3:����̂�)
	if( msg_data->Req_syu > 3 ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�╜�d���󎚗v���ް���������											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTFJOU_datachk( *Ponlog )								|*/
/*|																				|*/
/*|	PARAMETER		:	Pon_log	*Ponlog= �╜�d����ް��߲���					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-04-27														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnTFJOU_datachk( Pon_log *Ponlog )
{
	uchar	ret = OK;	// ��������


	//	��d��������
	if( NG == DateChk( (date_time_rec *)&Ponlog->Pdw_Date ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���d��������
	if( NG == DateChk( (date_time_rec *)&Ponlog->Pon_Date ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�s���E�����o�ɏ��󎚗v���ް���������									|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFKJOU_datachk( *fuskyo )								|*/
/*|																				|*/
/*|	PARAMETER		:	flp_log	*fuskyo= �s���E�����o�ɏ���ް��߲���			|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-08-02														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnFKJOU_datachk( flp_log *fuskyo )
{
	uchar	ret = OK;	// ��������


	//	������������
	if( NG == DateChk( (date_time_rec *)&fuskyo->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	���Ɏ�������
	if( NG == DateChk( (date_time_rec *)&fuskyo->In_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�װ��M���荞�݂̔�����A�����Ĵװ�o�^�����Ȃ�����						|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriErrRecieve( void )									|*/
/*|	PARAMETER		:	void													|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Hashimoto														|*/
/*|	Date	:	2006-01-12														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
void PriErrRecieve( void )
{
	_di();
	PriErrCount = 0;					// �װ��M�̓o�^��������BSci3Err()�Q�ƁB
	_ei();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		Suica�ʐM���O�󎚗v��ү���ގ�M����	�@�@							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuica_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka													|*/
/*|	Date	:	2005-07-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnSuica_LOG( MSG *msg )
{
	T_FrmLogPriReq1		*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnSuicaLOG_datachk( msg_data );			// �װ��񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				Suica_Log_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				Suica_Log_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				Suica_Log_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				Suica_Log_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		Suica�ʐM���O���߃f�[�^�p�󎚗v��ү���ގ�M����	�@�@				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuica_LOG2( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	A.iiizumi													|*/
/*|	Date	:	2012-09-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnSuica_LOG2( MSG *msg )
{
	T_FrmLogPriReq1		*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnSuicaLOG_datachk( msg_data );			// �װ��񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				Suica_Log_edit2( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				Suica_Log_edit2( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				Suica_Log_edit2( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				Suica_Log_edit2( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
/*[]---------------------------------------------------------------------------[]*/
/*|		Suica�ʐM۸ވ󎚗v���ް���������										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuicaLOG_datachk( *msg_data )						|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka												    	|*/
/*|	Date	:	2005-07-27												    	|*/
/*|	Update	:														�@   		|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
uchar	PrnSuicaLOG_datachk( T_FrmLogPriReq1 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�󎚎�������
	if( NG == DateChk( (date_time_rec *)&msg_data->PriTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
///*[]-----------------------------------------------------------------------[]*/
///*|		�ڼޯė��p�����ް��󎚗v��ү���ގ�M����							|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_USE( *msg )									|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Author	:	M.Yanase(COSMO)												|*/
///*|	Date	:	2006-07-14													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnCRE_USE( MSG *msg )
//{
//	T_FrmLogPriReq3	*msg_data;		// ��Mү�����ް��߲���
//
//	msg_data = (T_FrmLogPriReq3 *)msg->data;	// ��Mү�����ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//		CreUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���p���׃��O���ް��ҏW�iڼ�āj
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
///*[]-----------------------------------------------------------------------[]*/
///*|		�ڼޯĖ����M����˗��ް��󎚗v��ү���ގ�M����						|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_UNSEND( *msg )								|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Date	:	2013-07-01													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
//void	PrnCRE_UNSEND( MSG *msg )
//{
//	T_FrmUnSendPriReq	*msg_data;		// ��Mү�����ް��߲���
//
//	msg_data = (T_FrmUnSendPriReq *)msg->data;	// ��Mү�����ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//		CreUnSend_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ����˗��ް����ް��ҏW�iڼ�āj
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
///*[]-----------------------------------------------------------------------[]*/
///*|		�ڼޯĔ��㋑���ް��󎚗v��ү���ގ�M����							|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_SALENG( *msg )								|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Date	:	2013-07-01													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
//void	PrnCRE_SALENG( MSG *msg )
//{
//	T_FrmSaleNGPriReq	*msg_data;		// ��Mү�����ް��߲���
//
//	msg_data = (T_FrmSaleNGPriReq *)msg->data;	// ��Mү�����ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//		CreSaleNG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���㋑���ް����ް��ҏW�iڼ�āj
//		End_Set( msg, terget_pri );				// ���ް��P��ۯ��ҏW�I��������
//	}
//}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// #ifdef	FUNCTION_MASK_EDY
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Edy�ݒ�&�ð������Ĉ󎚗v��ү���ގ�M�����@							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdy_Status( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	T.Namioka													|*/
// /*|	Date	:	2005-07-27													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdy_Status( MSG *msg )
//{
//	T_FrmLogPriReq1		*msg_data;		// ��Mү�����ް��߲���
//
//
//	msg_data = (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������
//
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//
//		switch( terget_pri ){	// �󎚐�������H
//
//			case	R_PRI:		// ڼ��
//
//				Edy_Status_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
//				break;
//
//			case	J_PRI:		// �ެ���
//
//				Edy_Status_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//
//				Edy_Status_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
//				Edy_Status_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Edy�װ�ڼ�Ĉ󎚗v��ү���ގ�M����	�@�@							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdyArmPri( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	T.Namioka													|*/
// /*|	Date	:	2006-10-31													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdyArmPri( MSG *msg )
//{
//	T_FrmEdyArmReq	*msg_data;		// ��Mү�����ް��߲���
//	edy_arm_log		*pribuf;		// �̎��؈��ް��߲���
//
//
//	msg_data = (T_FrmEdyArmReq *)msg->data;	// ��Mү�����ް��߲������
//	pribuf = msg_data->priedit;				// �̎��؈��ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//
//		switch( terget_pri ){	// �󎚐�������H
//
//			case	R_PRI:		// ڼ��
//
//				Edy_Arm_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
//				break;
//
//			case	J_PRI:		// �ެ���
//
//				Edy_Arm_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//
//				Edy_Arm_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
//				Edy_Arm_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		�A���[��������󎚗v��ү���ގ�M����								|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEDYARMPAY_LOG( *msg )							|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	suzuki														|*/
// /*|	Date	:	2006-16-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEDYARMPAY_LOG( MSG *msg )
//{
//	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���
//
//
//	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//
//		switch( terget_pri ){	// �󎚐�������H
//
//			case	R_PRI:		// ڼ��
//
//				EDYARMPAYLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
//				break;
//
//			case	J_PRI:		// �ެ���
//
//				EDYARMPAYLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//
//				EDYARMPAYLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
//				EDYARMPAYLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		�s���v���Edy���ߋL�^�󎚗v��ү���ގ�M����							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdyShimePri( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	Suzuki														|*/
// /*|	Date	:	2006-12-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdyShimePri( MSG *msg )
//{
//	T_FrmEdyShimeReq	*msg_data;		// ��Mү�����ް��߲���
//	edy_shime_log		*pribuf;		// �̎��؈��ް��߲���
//
//
//	msg_data = (T_FrmEdyShimeReq *)msg->data;	// ��Mү�����ް��߲������
//	pribuf = msg_data->priedit;					// Edy���ߋL�^���ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//
//		switch( terget_pri ){	// �󎚐�������H
//
//			case	R_PRI:		// ڼ��
//
//				Edy_Shime_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
//				break;
//
//			case	J_PRI:		// �ެ���
//
//				Edy_Shime_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//
//				Edy_Shime_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
//				Edy_Shime_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		�d�������ߋL�^���󎚗v��ү���ގ�M����							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEDYSHIME_LOG( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	suzuki														|*/
// /*|	Date	:	2006-12-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEDYSHIME_LOG( MSG *msg )
//{
//	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���
//
//
//	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
//
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else{
//
//		switch( terget_pri ){	// �󎚐�������H
//
//			case	R_PRI:		// ڼ��
//
//				EDYSHIMELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );
//				break;
//
//			case	J_PRI:		// �ެ���
//
//				EDYSHIMELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//
//				EDYSHIMELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );
//				EDYSHIMELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
//#endif
//
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
/*[]-----------------------------------------------------------------------[]*/
/*|		�ߓd���h�~�̂��߂�24V�n���j�b�g�쓮�̔r��							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| ڼ��������C�ެ���������͋N�����ɓd���s�[�N�ƂȂ�A������				|*/
/*| �����ɋN�������ꍇ�A���u�d���i24V�j�̃��~�b�g�𒴂��Ă��܂��̂ŁA�N��	|*/
/*| �C�x���g���������������ꍇ�́A��҂�delay����������B					|*/
/*| �eUnit���ɁA�����N���֎~���Ԃ��قȂ�B									|*/
/*|																			|*/
/*| �m�����N���֎~���ԁn													|*/
/*|   �@ �v�����^�iR&J��2��j�� 100ms���ԋ֎~�B								|*/
/*|																			|*/
/*| ��LUnit�̋N���O�ɂ��̊֐���Call���鎖�Ƃ��A�N���֎~���Ԃł����		|*/
/*| �@ �v�����^�͎��Ԍo�߂���܂Ŗ{�֐���wait����B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Printer_Lifter_Wait( ReqUnit )						|*/
/*|	PARAMETER		:	ReqUnit : ����J�n�v�������j�b�g					|*/
/*|								    0=���V�[�g�v�����^						|*/
/*|								    1=�W���[�i���v�����^					|*/
/*|	RETURN VALUE	:	1 = �쓮�\										|*/
/*|							ReqUnit = 0 or 1 �̏ꍇ�A�߂�l��1�Œ�B		|*/
/*|						0 = �쓮�s��										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	namioka														|*/
/*|	Date	:	2007/05/31													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.----[]*/
#define		PRINT_START_INHIBIT_TIME	10			// ������󎚊J�n�� ��Unit����֎~����(x20ms) 200ms

uchar Printer_Lifter_Wait( char ReqUnit )
{
	ushort	wkJR_Print_Wait_tim[2];										// ������쓮����̌o�ߎ��� (20ms)
	uchar	f_Wait;														// 1=����֎~���Ԓ�
	uchar	ret=1;


	while( 1 ){

		/* �^�C�}�l�Ǐo���i���荞�ݏ����Ƌ��L����J�E���^�̂��߁j*/
		wkJR_Print_Wait_tim[0] = (ushort)JR_Print_Wait_tim[0];
		wkJR_Print_Wait_tim[1] = (ushort)JR_Print_Wait_tim[1];
		f_Wait = 0;														// �t���O�������i����\���ԁj

		/* �v�������j�b�g���䕔 ���ɕ��� */
		switch( ReqUnit ){
		case 0:	// ڼ�����������J�n�H
			if( (jnl_proc_data.Printing != 0) &&						// �ެ��ٓ��쒆��
				(wkJR_Print_Wait_tim[1] < PRINT_START_INHIBIT_TIME) )	// ���Ưē���֎~���Ԃł���
			{
				f_Wait = 1;												// ����֎~���ԂƂ���
			}
			break;

		case 1:	// �ެ������������J�n�H
			if( (rct_proc_data.Printing != 0) &&						// ڼ�ē��쒆��
				(wkJR_Print_Wait_tim[0] < PRINT_START_INHIBIT_TIME) )	// ���Ưē���֎~���Ԃł���
			{
				f_Wait = 1;												// ����֎~���ԂƂ���
			}
			break;

		}

		/* ����Ă��҂������ꍇ�̏��� */
		if( f_Wait ){
			xPause_PRNTSK( 2L );										// ���������� 20ms wait
		}
		else
			break;
	}
	return	ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�����E�r�����������p�����ް��󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				��Mү�����߲���
///	@return			void
///	@author			Y.Ise
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	PrnDigi_USE( MSG *msg )
{
	T_FrmLogPriReq4	*msg_data;					// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq4 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~����(�ҏW�ް��ر ���u�󎚏I��(��ݾ�)�vү���ޑ��M)
	}
	else{
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//// MHUT40XX(S) D.Inaba IC�N���W�b�g�Ή�(����ύX)
////		if(msg->command == PREQ_SCA_USE_LOG)
//		if(msg->command == PREQ_SCA_USE_LOG || msg->command == PREQ_EC_USE_LOG)
//// MHUT40XX(E) D.Inaba IC�N���W�b�g�Ή�(����ύX)
//		{
//			switch( terget_pri ){	// �󎚐�������H
//			case	R_PRI:		// ڼ��
//				SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			case	J_PRI:		// �ެ���
//				SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//				SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���p���׃��O�󎚏���
//				SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//			}
//		}
		switch( terget_pri ){	// �󎚐�������H
		case	R_PRI:		// ڼ��
			SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data, (short)msg->command );		// ���p���׃��O�󎚏���
			break;

		case	J_PRI:		// �ެ���
			SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, (short)msg->command );		// ���p���׃��O�󎚏���
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data, (short)msg->command );		// ���p���׃��O�󎚏���
			SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, (short)msg->command );		// ���p���׃��O�󎚏���
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		else if(msg->command == PREQ_EDY_USE_LOG)
//		{
//			switch( terget_pri ){	// �󎚐�������H
//			case	R_PRI:		// ڼ��
//				EdyUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			case	J_PRI:		// �ެ���
//				EdyUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			case	RJ_PRI:		// ڼ�ā��ެ���
//				EdyUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���p���׃��O�󎚏���
//				EdyUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���p���׃��O�󎚏���
//				break;
//
//			default:			// ���̑��i�������ʴװ�j
//				return;
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		End_Set( msg, terget_pri );				// ���ް��P��ۯ��ҏW�I��������
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�����E�r���������W�v���󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			��Mү�����߲���
///	@return			void
///	@author			Y.Ise
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	PrnDigi_SYU( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		if(msg->command == PREQ_SCA_SYU_LOG)
		{
			switch( terget_pri ){	// �󎚐�������H

				case	R_PRI:		// ڼ��

					SuicaSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���Z���~�����ް��ҏW�iڼ�āj
					break;

				case	J_PRI:		// �ެ���

					SuicaSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���Z���~�����ް��ҏW�i�ެ��فj
					break;

				case	RJ_PRI:		// ڼ�ā��ެ���

					SuicaSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���Z���~�����ް��ҏW�iڼ�āj
					SuicaSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���Z���~�����ް��ҏW�i�ެ��فj
					break;

				default:			// ���̑��i�������ʴװ�j
					return;
			}
		}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		else if(msg->command == PREQ_EDY_SYU_LOG)
//		{
//			switch( terget_pri ){	// �󎚐�������H
//
//				case	R_PRI:		// ڼ��
//
//					EdySyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���Z���~�����ް��ҏW�iڼ�āj
//					break;
//
//				case	J_PRI:		// �ެ���
//
//					EdySyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���Z���~�����ް��ҏW�i�ެ��فj
//					break;
//
//				case	RJ_PRI:		// ڼ�ā��ެ���
//
//					EdySyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���Z���~�����ް��ҏW�iڼ�āj
//					EdySyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���Z���~�����ް��ҏW�i�ެ��فj
//					break;
//
//				default:			// ���̑��i�������ʴװ�j
//					return;
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�����W�v��)
		else if(msg->command == PREQ_EC_SYU_LOG)
		{
			switch( terget_pri ){	// �󎚐�������H

				case	R_PRI:		// ڼ��
					EcSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���Z���~�����ް��ҏW�iڼ�āj
					break;

				case	J_PRI:		// �ެ���

					EcSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���Z���~�����ް��ҏW�i�ެ��فj
					break;

				case	RJ_PRI:		// ڼ�ā��ެ���

					EcSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���Z���~�����ް��ҏW�iڼ�āj
					EcSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���Z���~�����ް��ҏW�i�ެ��فj
					break;

				default:			// ���̑��i�������ʴװ�j
					return;
			}
		}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�����W�v��)
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�W���L���f�[�^�󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAtendValidData( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnAtendValidData( MSG *msg )
{
	T_FrmLogPriReq1	*msg_data;							// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq1 *)msg->data;			// ��Mү�����ް��߲������


	if( YES == Inji_Cancel_chk( terget_pri ) ){			// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );					// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				AtendValidData_edit(						// �W���L���f�[�^���ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// �ެ���

				AtendValidData_edit(						// �W�������f�[�^���ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				AtendValidData_edit(						// �W���L���f�[�^���ް��ҏW�iڼ�āj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				AtendValidData_edit(						// �W�������f�[�^���ް��ҏW�i�ެ��فj
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}
		End_Set( msg, terget_pri );						// ���ް��P��ۯ��ҏW�I��������
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ��Mү�����ޯ̧�߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/01/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	PrnCHARGESETUP( MSG *msg )
{
	T_FrmChargeSetup	*msg_data;		// ��Mү�����ް��߲���

	msg_data	= (T_FrmChargeSetup *)msg->data;	// ��Mү�����ް��߲������
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��
			ChargeSetup_edit(R_PRI, (PRN_PROC_DATA *)&rct_proc_data, msg_data);
			break;

		case	J_PRI:		// �ެ���
			ChargeSetup_edit(J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, msg_data);
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			ChargeSetup_edit(R_PRI, (PRN_PROC_DATA *)&rct_proc_data, msg_data);
			ChargeSetup_edit(J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, msg_data);
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ��Mү�����ޯ̧�߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/01/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	PrnSetDiffLogPrn( MSG *msg )
{
	T_FrmLogPriReq1 *msg_data;		// ��Mү�����ް��߲���

	msg_data	= (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&rct_proc_data, R_PRI);
			break;

		case	J_PRI:		// �ެ���
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&jnl_proc_data, J_PRI);
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&rct_proc_data, R_PRI);
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&jnl_proc_data, J_PRI);
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
	}

	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�ʐM���O�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ��Mү�����ޯ̧�߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	:	2009/10/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnRrokin_Setlog(MSG *msg)
{
	T_FrmLogPriReq2	*msg_data;		// ��Mү�����ް��߲���

	msg_data	= (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	J_PRI:		// �ެ���
			PrnRrokin_Setlog_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ڼ��
			PrnRrokin_Setlog_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnRrokin_Setlog_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnRrokin_Setlog_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������	
}
//[]----------------------------------------------------------------------[]
///	@brief			�H�ꌟ���p�󎚏���
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ��Mү�����ޯ̧�߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	: 2011/02/21<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnSettei_Chk(MSG *msg)
{
	T_FrmAnyData *msg_data;
	msg_data	= (T_FrmAnyData *)msg->data;	// ��Mү�����ް��߲������
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	J_PRI:		// �ެ���
			PrnSettei_Chk_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ڼ��
			PrnSettei_Chk_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnSettei_Chk_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnSettei_Chk_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������	
	
}
//[]----------------------------------------------------------------------[]
///	@brief			�t���b�v���[�v�f�[�^�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ��Mү�����ޯ̧�߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	:	2009/10/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnFlapLoopData(MSG *msg)
{
	T_FrmAnyData	*msg_data;		// ��Mү�����ް��߲���

	msg_data	= (T_FrmAnyData *)msg->data;	// ��Mү�����ް��߲������
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	J_PRI:		// �ެ���
			PrnFlapLoopData_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ڼ��
			PrnFlapLoopData_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnFlapLoopData_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnFlapLoopData_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// ���ް��P��ۯ��ҏW�I��������	
}
//[]-----------------------------------------------------------------------[]*/
//|		�����e�X�g���󎚗v��ү���ގ�M����									|*/
//[]-----------------------------------------------------------------------[]*/
//|	MODULE NAME		:	RT_PrnSEISAN_LOG( *msg )							|*/
//|																			|*/
//|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
//|																			|*/
//|	RETURN VALUE	:	void												|*/
//|																			|*/
//[]-----------------------------------------------------------------------[]*/
//|	Author	:	Y.Shiraishi													|*/
//|	Date	:	2012-10-15													|*/
//|	Update	:																|*/
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	RT_PrnSEISAN_LOG( MSG *msg )
{
	T_FrmLogPriReq1	*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				RT_SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				RT_SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				RT_SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �ʐ��Z�����ް��ҏW�iڼ�āj
				RT_SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �ʐ��Z�����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
void	PrnMntStack(MSG *msg)
{
	T_FrmLogPriReq1		*msg_data;		// ��Mү�����ް��߲���


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// ��Mү�����ް��߲������


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				Mnt_Stack_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				Mnt_Stack_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				Mnt_Stack_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				Mnt_Stack_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
	
}
void	PrnChkResult(MSG *msg)
{
	T_FrmChk_result		*msg_data;
	msg_data = (T_FrmChk_result *)msg->data;
	
	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				ChkResult_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				ChkResult_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				ChkResult_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ��񗚗����ް��ҏW�iڼ�āj
				ChkResult_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ��񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
	
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���b�Z�[�W�L���[�̐擪�ɑ}��										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPushFrontMsg( MsgBuf *msg )						|*/
/*|	PARAMETER		:	MsgBuf *msg											|*/
/*|	RETURN VALUE	:														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPushFrontMsg( MsgBuf *msg )
{
	ulong	ist;													// ���݂̊�����t���

	ist = _di2();
	tcb[PRNTCBNO].event = MSG_SET;
	if( tcb[PRNTCBNO].msg_top == NULL ){
		tcb[PRNTCBNO].msg_top = msg;
		tcb[PRNTCBNO].msg_end = msg;
	}
	else {
		msg->msg_next = (char*)tcb[PRNTCBNO].msg_top;
		tcb[PRNTCBNO].msg_top = msg;
	}
	_ei2( ist );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		���u�Ď����󎚗v���ް���������										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMONJOU_datachk( *Rmonlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Rmon_log	*Rmonlog	= ���u�Ď�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai															|*/
/*|	Date	:	2014-12-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.--------[]*/
uchar	PrnRMONJOU_datachk( Rmon_log *Rmonlog )
{
	uchar	ret = OK;	// ��������


	//	������������
	if( NG == DateChk( (date_time_rec *)&Rmonlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�[���@�\�ԍ�����
	if( Rmonlog->RmonFuncNo > RMON_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�����ԍ�����
	if( Rmonlog->RmonProcNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�������ԍ�����
	if( Rmonlog->RmonProcInfoNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�󋵔ԍ�����
	if( Rmonlog->RmonStatusNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	//	�󋵏ڍהԍ�����
	if( Rmonlog->RmonStatusDetailNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		���u�Ď���񗚗��󎚗v���ް���������									|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMONLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ү�����ް��߲���		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : ����											|*/
/*|							NG : �ُ�											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai															|*/
/*|	Date	:	2014-12-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.--------[]*/
uchar	PrnRMONLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ��������

	//	�w���������
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// �󎚗v���ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���u�Ď����󎚗v��ү���ގ�M����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMON_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai														|*/
/*|	Date	:	2014-12-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.----[]*/
void	PrnRMON_JOU( MSG *msg )
{
	T_FrmRmonJou		*msg_data;		// ��Mү�����ް��߲���
	Rmon_log			*Rmonlog;		// �ް��߲���

	msg_data	= (T_FrmRmonJou *)msg->data;		// ��Mү�����ް��߲������
	Rmonlog		= msg_data->prn_data;				// �ް��߲������

	PrnRMONJOU_datachk( Rmonlog );					// �ް�����

	switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			RMONJOU_edit( Rmonlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			RMONJOU_edit( Rmonlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			RMONJOU_edit( Rmonlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ���ް��ҏW�iڼ�āj
			RMONJOU_edit( Rmonlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ���ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	End_Set( msg, terget_pri );												// ���ް��P��ۯ��ҏW�I��������
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���u�Ď���񗚗��󎚗v��ү���ގ�M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMON_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	��Mү�����߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai														|*/
/*|	Date	:	2014-12-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.----[]*/
void	PrnRMON_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// ��Mү�����ް��߲������

	if( next_prn_msg == OFF ){					// �V�K�̈󎚗v���H

		//	�V�K�̈󎚗v���̏ꍇ
		PrnRMONLOG_datachk( msg_data );			// ���u�Ď���񗚗��󎚗v���ް�����
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else{

		switch( terget_pri ){	// �󎚐�������H

			case	R_PRI:		// ڼ��

				RMONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���u�Ď���񗚗����ް��ҏW�iڼ�āj
				break;

			case	J_PRI:		// �ެ���

				RMONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���u�Ď���񗚗����ް��ҏW�i�ެ��فj
				break;

			case	RJ_PRI:		// ڼ�ā��ެ���

				RMONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���u�Ď���񗚗����ް��ҏW�iڼ�āj
				RMONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���u�Ď���񗚗����ް��ҏW�i�ެ��فj
				break;

			default:			// ���̑��i�������ʴװ�j
				return;
		}

		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_�A���[�����V�[�g�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			��Mү�����߲���
///	@return			void
///	@author			Inaba
///	@attention		None
///	@note			PrnSandenAlmRctPri�ڐA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PrnEcAlmRctPri( MSG *msg )
{
	T_FrmEcAlmRctReq	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmEcAlmRctReq *)msg->data;	// ��Mү�����ް��߲������

	if (YES == Inji_Cancel_chk(terget_pri)) {		// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else {
	// ������W���[�i���̂�
		switch (terget_pri) {	// �󎚐�������H
		case	R_PRI:		// ڼ��
			Ec_AlmRct_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			Ec_AlmRct_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			Ec_AlmRct_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�iڼ�āj
			Ec_AlmRct_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �װ�ڼ�ď�񗚗����ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

//[]----------------------------------------------------------------------[]
//	@brief			���σ��[�_ �u�����h��Ԉ󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
//	@param[in]		msg			��Mү�����߲���
//	@return			void
//	@author			Inaba
//	@attention		None
//	@note			PrnSandenBrandPri�ڐA
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/02/14<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	PrnEcBrandPri( MSG *msg )
{
	T_FrmEcBrandReq	*msg_data;		// ��Mү�����ް��߲���

	msg_data = (T_FrmEcBrandReq *)msg->data;	// ��Mү�����ް��߲������

	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	/* �Ώۂ��@�B���ƒ��ߎ��������Ȃ��̂ŁA�f�[�^�`�F�b�N�����͂��Ȃ� */
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��

			Ec_Brand_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���σ��[�_�u�����h��Ԉ��ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			Ec_Brand_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���σ��[�_�u�����h��Ԉ��ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			Ec_Brand_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ���σ��[�_�u�����h��Ԉ��ް��ҏW�iڼ�āj
			Ec_Brand_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ���σ��[�_�u�����h��Ԉ��ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}

//[]----------------------------------------------------------------------[]
//	@brief			��������������O�󎚗v��ү���ގ�M����
//[]----------------------------------------------------------------------[]
//	@param[in]		msg			��Mү�����߲���
//	@return			void
//	@author			Inaba
//	@attention		None
//	@note			PrnSandenAlmRctLogPri�ڐA
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/02/14<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	PrnEcAlmRctLogPri( MSG *msg )
{
	T_FrmEcAlarmLog	*msg_data;		// ��Mү�����ް��߲���
	
	msg_data = (T_FrmEcAlarmLog *)msg->data;	// ��Mү�����ް��߲������
	
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// �󎚒��~ү���ގ�M�ς݁H

		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	// �����V�[�g�̂�
	else{
		switch( terget_pri ){	// �󎚐�������H

		case	R_PRI:		// ڼ��
			Ec_AlmRctLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��������������O���ް��ҏW�����iڼ�āj
			break;
			
		case	J_PRI:		// �ެ���
			Ec_AlmRctLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��������������O���ް��ҏW�����i�ެ��فj
			break;
			
		case	RJ_PRI:		// ڼ�ā��ެ���
			Ec_AlmRctLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��������������O���ް��ҏW�����iڼ�āj
			Ec_AlmRctLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ��������������O���ް��ҏW�����i�ެ��فj
			break;
			
		default:			// ���̑��i�������ʴװ�j
			return;
		}
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
		if (rct_proc_data.Prn_no_wk[0] == 2) {
			queset(OPETCBNO, OPE_EC_ALARM_LOG_PRINT_1_END, 0, NULL);					// ��������������O1���󎚊���
		}
	}
}

// MH810103 GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_�݂Ȃ����ϕ��d���O�󎚗v��ү���ގ�M����
////[]----------------------------------------------------------------------[]
/////	@param[in]		msg			��Mү�����߲���
/////	@return			void
/////	@author			Inaba
/////	@attention		None
/////	@note			PrnSandenAlmRctPri�ڐA
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/14<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	PrnEcDeemedJnlPri( MSG *msg )
//{
//	T_FrmEcDeemedJnlReq	*msg_data;		// ��Mү�����ް��߲���
//
//	msg_data = (T_FrmEcDeemedJnlReq *)msg->data;	// ��Mү�����ް��߲������
//
//	if (YES == Inji_Cancel_chk(terget_pri)) {		// �󎚒��~ү���ގ�M�ς݁H
//		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
//		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
//	}
//	else {
//	// ������W���[�i���̂�
//		switch (terget_pri) {	// �󎚐�������H
//		case	R_PRI:		// ڼ��
//			Ec_DeemedJnl_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �݂Ȃ����ϕ��d��񗚗����ް��ҏW�iڼ�āj
//			break;
//
//		case	J_PRI:		// �ެ���
//			Ec_DeemedJnl_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �݂Ȃ����ϕ��d��񗚗����ް��ҏW�i�ެ��فj
//			break;
//
//		case	RJ_PRI:		// ڼ�ā��ެ���
//			Ec_DeemedJnl_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �݂Ȃ����ϕ��d��񗚗����ް��ҏW�iڼ�āj
//			Ec_DeemedJnl_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �݂Ȃ����ϕ��d��񗚗����ް��ҏW�i�ެ��فj
//			break;
//
//		default:			// ���̑��i�������ʴװ�j
//			return;
//		}
//		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
//	}
//}
// MH810103 GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�

//[]----------------------------------------------------------------------[]
///	@brief			���ϐ��Z���~�i�����ρj��
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			��Mү�����߲���
///	@return			void
///	@author			Inaba
///	@attention		None
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/10/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PrnRecvDeemedDataPri( MSG *msg )
{
	T_FrmReceipt	*msg_data;		// ��Mү�����ް��߲���
	Receipt_data	*msg_data_receipt;	// ��Mү�����ް�����Receipt_data�߲���

	msg_data = (T_FrmReceipt *)msg->data;	// ��Mү�����ް��߲������
	msg_data_receipt = (Receipt_data *)msg_data->prn_data;	// Receipt_data�ް��߲������

	if (YES == Inji_Cancel_chk(terget_pri)) {		// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );				// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else {
	// ������W���[�i���̂�
		switch (terget_pri) {	// �󎚐�������H
		case	R_PRI:		// ڼ��
			Recv_DeemedData_edit( msg_data_receipt, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �݂Ȃ����σf�[�^�i�����ρj�iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			Recv_DeemedData_edit( msg_data_receipt, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �݂Ȃ����σf�[�^�i�����ρj�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			Recv_DeemedData_edit( msg_data_receipt, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// �݂Ȃ����σf�[�^�i�����ρj�iڼ�āj
			Recv_DeemedData_edit( msg_data_receipt, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// �݂Ȃ����σf�[�^�i�����ρj�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
		}
		End_Set( msg, terget_pri );					// ���ް��P��ۯ��ҏW�I��������
	}
}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
//[]----------------------------------------------------------------------[]
///	@brief			�f�o�b�O�p��
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			��Mү�����߲���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/12/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void	PrnDEBUG( MSG *msg )
{
	T_FrmDebugData *msg_data = (T_FrmDebugData *)msg->data;	// ��Mү�����ް��߲������

	if ( YES == Inji_Cancel_chk(terget_pri) ) {	// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel(msg, terget_pri);			// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	} else {
		switch( terget_pri ) {	// �󎚐�������H
		case J_PRI:				// �ެ���
			PrnDEBUG_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case RJ_PRI:			// ڼ�ā��ެ���
			PrnDEBUG_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case R_PRI:				// ڼ��
		default:
			return;
		}
	}

	End_Set(msg, terget_pri);	// ���ް��P��ۯ��ҏW�I��������
}
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[��
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			��Mү�����߲���
///	@return			void
///	@author			Sato
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/12<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void	PrnFailureContactDataPri( MSG *msg )
{
	T_FrmReceipt	*FailureContactData;

	FailureContactData = (T_FrmReceipt *)msg->data;				// ��Q�A���[���ް�

	if (YES == Inji_Cancel_chk(terget_pri)) {					// �󎚒��~ү���ގ�M�ς݁H
		//	�󎚒��~ү���ގ�M�ς݂̏ꍇ
		Inji_Cancel( msg, terget_pri );							// �󎚒��~�����i�ҏW���������ް��ر���u�󎚏I���i��ݾفj�vү���ޑ��M�j
	}
	else {
		switch (terget_pri) {	// �󎚐�������H
		case	R_PRI:			// ڼ��
			Recv_FailureContactData_edit( FailureContactData->prn_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ��Q�A���[�iڼ�āj
			break;
		default:				// ���̑��i�������ʴװ�j
			return;
		}
		End_Set( msg, terget_pri );								// ���ް��P��ۯ��ҏW�I��������
	}
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
