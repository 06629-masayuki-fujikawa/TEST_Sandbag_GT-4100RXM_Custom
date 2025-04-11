/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް�֘A����                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"flp_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

#define		R_TIM_OUT	(15*50)										// 15s����

static short Ope_GT_Settei_Check( uchar gt_flg, ushort pno_syu );
uchar GT_Settei_flg;					// GT̫�ϯĐݒ�NG�׸�
extern void StackCardEject(uchar req);
uchar	chk_JIS2( void );
char	MAGred_HOUJIN[sizeof(MAGred)];					// �@�l�J�[�h�`�F�b�N�p�o�b�t�@
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް�������҂�����                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : red_int( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	red_int( void )
{
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j
//	unsigned short	msg = 0;
//	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
//	int		loop;
//	short	rd_snd;
//	char	ret, i;
//
//	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
//
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) == 0 ){						// ���Cذ�ް�g�p�s��?
//		return;
//	}
//	OPE_red = 0;													// ذ�ް�����r�o�Ƃ��Ȃ�
//
//	Lagtim( OPETCBNO, 3, R_TIM_OUT );								// ��ϰ3(15s)�N��
//
//	rd_snd = i = 0;
//	for( ret = 0; ret == 0; ){
//		if( i == 0 ){
//			switch( RD_mod ){
//				case 0:
//					opr_snd( rd_snd = 0 );
//					i = 1;
//					break;
//				case 6:
//				case 8:
//				case 20:
//					opr_snd( rd_snd = 94 );							// �o�[�W�����v��
//					i = 1;
//					break;
//				case 19:
//					opr_snd( rd_snd = 90 );							// ��ԗv��
//					i = 1;
//					break;
//				case 10:
//				case 11:
//					Lagcan( OPETCBNO, 3 );							// Timer Cancel
//					return;
//				default:
//					i = 1;
//					break;
//			}
//		}
//
//		// ��M������ү����ID������
//		Target_WaitMsgID.Count = 3;
//		Target_WaitMsgID.Command[0] = TIMEOUT3;
//		Target_WaitMsgID.Command[1] = ARC_CR_SND_EVT;
//		Target_WaitMsgID.Command[2] = ARC_CR_E_EVT;
//		for( loop=1; loop ; ){
//			taskchg( IDLETSKNO );									// Change task to idletask
//			msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID );		// ���҂���Ұق�����M�i���͗��߂��܂܁j
//			if( NULL == msb ){										// ���҂���ҰقȂ��i����M�j
//				continue;
//			}
//
//			msg = msb->msg.command;
//			switch( msg ){
//				case TIMEOUT3:
//					loop = 0;
//					break;
//				case ARC_CR_SND_EVT:								// ���M����
//				case ARC_CR_E_EVT:									// �I������ގ�M
//					loop = 0;
//					opr_ctl( msg );									// message���͏���
//					break;
//				default:
//					break;
//			}
//			if( !loop ){
//				FreeBuf( msb );										// ��Mү�����ޯ̧�J��
//			}
//		}
//
//		switch( msg ){
//			case TIMEOUT3:											// ��ϰ3��ѱ��
//				/*** ذ�ސ���ł͂Ȃ� ***/
//				opr_snd( 0 );
//				ret = 1;
//				break;
//			case ARC_CR_SND_EVT:									// ���M����
//			case ARC_CR_E_EVT:										// �I������ގ�M
//				if(( rd_snd == 90 )&&( msg == ARC_CR_E_EVT )){
//					if( prm_get( COM_PRM,S_PAY,21,1,3 ) > 2 ){
//						/*** V��������� ***/
//					}
//					ret = 1;
//				}else{
//					i = 0;											// �Ď��s
//				}
//				break;
//			default:
//				break;
//		}
//	}
//	w_stat2 = 0;													// R����ގ�M New Status
//	Lagcan( OPETCBNO, 3 );											// Timer Cancel

	// �����ő҂����킹�������s���Ă��܂����߁A�֐��X�^�u��
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް���䏈��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_ctl( ret )                                          |*/
/*| PARAMETER    : ret  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_ctl( unsigned short ret )
{
	short	ans;

	ans = 0;														// ����ޑ��M�w��
																	// 0:�����M
	opr_rcv( ret );													// ذ�ް����
	switch( ret ){
		/*--------------------------------------------------------------*/
		/* ذ���ް���M���Aذ�ް���싖����Ă��Ȃ��ꍇ�����r�o����B	*/
		/* ���ذ�޺���ނ���M��Ԃׁ̈B								*/
		/*--------------------------------------------------------------*/
		case ARC_CR_R_EVT:											// ذ���ް���M
			if( OPE_red == 2 ){										// �����r�o?
				ans = 13;											// Yes
			}
			break;

		/*----------------------------------------------*/
		/* ���ވ������������							*/
		/* ذ�ް���싖����Ă��Ȃ����ͼ���������	*/
		/* �������ςݏ�Ԃ̏ꍇ�Aذ�޺���ޑ��M����B	*/
		/*----------------------------------------------*/
		case ARC_CR_EOT_EVT:										// ���ޔ������
			if( RD_mod == 6 ){										// �Ƽ�ي���?
				if(( OPE_red == 2 )||( OPE_red == 3 )){				// �����r�o?
					ans = 3;										// Yes..ذ�޺����(���Ԍ��ۗ���)
				}
				else if( OPE_red == 1 ){							// ���Ԍ��҂�? NJ
					ans = 3;										// Yes..ذ�޺����(���Ԍ��ۗ���)
				}
				if(( OPE_red < 3 )&&( rd_tik == 1 )){				// ���Ԍ���?
					ans = 5;										// Yes..�捞
				}
			}
			break;

		/*--------------------------*/
		/* ذ�޺���ގ�M�ςݲ����	*/
		/* �Ȃɂ����Ȃ�				*/
		/*--------------------------*/
		case ARC_CR_EOT_RCMD:										// ذ�޺���ގ�M�ς�
			break;

		/*--------------------------------------------------*/
		/* ذ�޺���ގ�M�҂������							*/
		/* ذ���ް��҂���Ԃł��̲���Ĕ����͏�NG�ׁ̈A	*/
		/* ذ�޺���ނ𑗐M����B							*/
		/*--------------------------------------------------*/
		case ARC_CR_EOT_RCMD_WT:									// ذ�޺���ގ�M�҂�
			if( RD_mod == 8 ){										// ذ���ް����M�ς�?
				ans = 3;											// Yes..ذ�޺����(���Ԍ��ۗ���)
			}
			break;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//		case ARC_CR_EOT_MOVE_TKT:									// 
//			if( pcard_shtter_ctl == 1 ){							// �x���t�@�C���ɃV���b�^�[����
//				read_sht_opn();
//				pcard_shtter_ctl = 0;
//			}
//			break;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

		/*------------------------------*/
		/* ���M��������āA���M�װ�����	*/
		/* �ް�ޮ��ް���M				*/
		/* �Ȃɂ����Ȃ�					*/
		/*------------------------------*/
		case ARC_CR_SND_EVT:										// ���M����
		case ARC_CR_SER_EVT:										// ���M�װ
			WaitForTicektRemove = 0;								// 0�ر�͂��������ōs������
			if( ( (RD_SendCommand) == 0x06 )||						// ��o���ړ����H
				( (RD_SendCommand) == 0x0A ) )
			{
				WaitForTicektRemove = 1;
			}
		case ARC_CR_VER_EVT:										// �ް�ޮ��ް���M
			break;

		/*----------------------------------------------*/
		/* ���슮�������								*/
		/* �������ςݏ�Ԃ̏ꍇ�Aذ�޺���ޑ��M����B	*/
		/*----------------------------------------------*/
		case ARC_CR_E_EVT:											// �I������ގ�M(���슮��)
			if( WaitForTicektRemove ){								// ���ޔ������
				queset( OPETCBNO, ARC_CR_EOT_EVT, 0, NULL );
			}
			if( RD_mod == 6 ){										// �Ƽ�ي���?
				if( opr_bak != -1 ){								// ����ޕۗ���? NJ
					ans = opr_bak;
					opr_bak = -1;
				}else{
					if(( OPE_red == 2 )||( OPE_red == 3 )){			// �����r�o? NJ
						ans = 3;									// Yes..ذ�޺����(���Ԍ��ۗ���)
					}
					else if( OPE_red == 1 ){						// ���Ԍ��҂�? NJ
						ans = 3;									// Yes..ذ�޺����(���Ԍ��ۗ���)
					}
					if((OPE_red < 3 )&&( rd_tik == 1 )){			// ���Ԍ���?
						ans = 5;									// Yes..�捞
					}
				}
			}
			StackCardEject(2);									/* �A���r�o����h�~�^�C�}�J�n */
			break;

		default:
			if( RD_mod == 6 ){										// �Ƽ�ي���?
				ans = 3;											// Yes..ذ�޺����(���Ԍ��ۗ���)
			}
			break;
	}
	/*--------------*/
	/* ����ޑ��M	*/
	/*--------------*/
	if( ans ){
		if( opr_snd( ans ) != 0 ) {
			opr_snd( 0 );
		}
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް��M�ް�����                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_rcv( ret )                                          |*/
/*| PARAMETER    : ret  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_rcv( unsigned short ret )
{
	unsigned short	i;
	uchar	chk;			// ���ް���������
	uchar	pk_syu;			// ���ԏ���
	unsigned short	cnt;
	uchar	disp_card_chk=0;
	uchar set;

	switch( ret ){
		/*----------------------------------------------*/
		/* ���M��������āA���M�װ�����					*/
		/* �������ςݏ�Ԃ̏ꍇ�Aذ�޺���ޑ��M����B	*/
		/*----------------------------------------------*/
		case ARC_CR_SND_EVT:										// ���M����
			switch( RD_mod ){
				case 1:
				case 3:
				case 7:
				case 10:
					RD_mod++;
					break;
			}
			break;

		/*--------------------------*/
		/* ���M�װ�����				*/
		/* �Ƽ�ٺ���� �O���r�o���M	*/
		/*--------------------------*/
		case ARC_CR_SER_EVT:										// ���M�װ
			set = (uchar)prm_get( COM_PRM,S_PAY,21,1,3 );
			if(set == 1 || set == 2) {
				i = rd_init( 2 );
			}
			else {
				i = rd_init( 4 );									// �v�����^�Ȃ�
			}
			if( i == 0 ){											// ����?(Y)
				RD_mod = 1;
			}else{													// �ُ�?(Y)
				RD_mod = 0;
			}
			break;

		/*------------------*/
		/* ���슮�������	*/
		/*------------------*/
		case ARC_CR_E_EVT:											// �I������ގ�M(���슮��)
			if( RED_REC.ercd != 0 ){								// �װ����
				switch( RED_REC.ercd ){
					case	E_VERIFY:
						err_chk( ERRMDL_READER, ERR_RED_VERIFY, 2, 0, 0 );	// �װ�o�^�F���̧��װ
						break;

					case	E_START:	// �X�^�[�g�����G���[
					case	E_DATA:		// �f�[�^�G���[
					case	E_PARITY:	// �p���e�B�G���[
						err_chk( ERRMDL_READER, (char)(ERR_RED_START+(RED_REC.ercd-E_START)), 2, 0, 0 );	
						break;
						
					default:
						if( (M_R_WRIT == Mag_LastSendCmd) ||			// �����ݺ����
						    (M_R_PRWT == Mag_LastSendCmd) )				// �󎚁E������
						{
							if( RED_REC.ercd == 0x24 ){	//������CRC�G���[
								alm_chk( ALMMDL_SUB2, ALARM_WRITE_CRC_ERR, 2 );/* �װ�۸ޓo�^����		*/
							}else{
								err_chk( ERRMDL_READER, ERR_RED_AFTERWRITE, 2, 0, 0 );	// �װ�o�^�F�����݌�װ����
							}
						}
						else{
							if( RED_REC.ercd == 0x25 ){	//�Ǎ���CRC�G���[
								alm_chk( ALMMDL_SUB2, ALARM_READ_CRC_ERR, 2 );/* �װ�۸ޓo�^����		*/
							}else{
								err_chk( ERRMDL_READER, ERR_RED_OTHERS, 2, 0, 0 );	// �װ�o�^�F���̑��װ����
							}
						}
						break;
					
				}
				if ( RD_PrcWriteFlag != 0 ){
					alm_chk( ALMMDL_SUB2, ALARM_RED_AFTERWRITE_PRC, 2 );	/* �װ�۸ޓo�^ �����E����	*/
					RD_PrcWriteFlag = 0;
				}
			}
			if ( RED_REC.ercd == 0 ){
				if( (M_R_WRIT == Mag_LastSendCmd) || (M_R_PRWT == Mag_LastSendCmd) ){// ������ / �󎚁E������
					RD_PrcWriteFlag = 0;
				}
			}
			Mag_LastSendCmd = '\0';
			switch( RD_mod ){
				case 2:												// �Ƽ�ي���?(Y)
					if( (prm_get( COM_PRM,S_SYS,11,1,1 ) == 1 ) && ( (RED_REC.posi[1] & 0x08) == 0 )){
						err_chk( ERRMDL_READER, ERR_RED_MAGTYPE, 1, 0, 0 );// ���C���[�_�[�^�C�v�ُ픭��
						GT_Settei_flg = 1;
					}
					if( rd_font() == 0 ){
						RD_mod++;
					}else{
						RD_mod = 0;
					}
					break;
				case 4:
				case 13:											// ���슮��
					RD_mod = 6;
				case 6:
					break;
				case 7:
					break;
				case 8:												// ��ذ��
					RD_mod = 6;										// �Ƽ�ي���
					if( RED_REC.ercd != 0 ){
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
						MAGred[MAG_ID_CODE] = 0xff;
						OPECTL.LastCard = OPE_LAST_RCARD_MAG;
						OPECTL.LastCardInfo = (ushort)MAGred[MAG_ID_CODE];
						MagReadErrCodeDisp(RED_REC.ercd);			// ���C�f�[�^�ǎ��G���[�ڍו\��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
						// ���σ��[�_�֘A�����\�����Ȃ烁�b�Z�[�W�͕\�������Ȃ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if( ec_MessagePtnNum != 0 ){
						if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							break;
						}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
						if( OPE_red == 3 ){							// ���Z��?
							ope_anm(AVM_MAGCARD_ERR);
							grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[9] );		// "    ���̃J�[�h�͓ǂ߂܂���    "�װ�\���͔��]�����ɂĕ\��
							Lagtim( OPETCBNO, 7, 5*50 );			// ��ϰ7(5s)�N��(�ǎ�װ�\���p)
						}
					}
					break;
				case 11:											// ���������҂�
					RD_mod = 6;
					if( RED_REC.ercd == E_VERIFY ){					// ���̧��װ
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_VN, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
						RED_REC.ercd = 0;
					}
					if( rd_tik == 2 ){								// ���Ԍ��̎��������҂�?
						rd_tik = 0;
					}
					if( RD_Credit_kep ){
						RD_Credit_kep = 0;
					}
					if(( OPE_red != 3 )&&( OPE_red != 4 )){
						switch( rd_tik ){
							case 1:									// ���Ԍ��ۗ�
								opr_snd( 5 );						// ���Ԍ��捞
								break;
							case 3:									// ���Ԍ��߂��҂�
								opr_snd( 13 );						// ����o���ړ�
								break;
							case 4:									// ���Ԍ������҂�
								opr_snd( 7 );						// ���~��ײ�
								break;
							case 5:									// ���Ԍ��߂��҂�
								opr_snd( 15 );						// �a�����
								break;
							case 6:									// ���Ԍ������҂�
								opr_snd( 14 );						// �a���蒆�~
								break;
						}
					}
					if( pas_kep ){									// ����ۗ�?
						pas_kep = 0;
					}
					break;
				case 12:											// ذ�ޓ��쒆
					rd_faz++;
					if( rd_mov( rd_faz ) != 0 ){
						RD_mod = 0;
					}
					break;
				case 20:											// �ް�ޮݗv���o�͌�
				case 15:											// ��ԗv���o�͌�
					RD_mod = 20;
					break;
				default:
					RD_mod = 0;
					break;
			}
			break;

		/*--------------------------------------------------------------*/
		/* ذ���ް���M���Aذ�ް���싖����Ă��Ȃ��ꍇ�����r�o����B	*/
		/* ���ذ�޺���ނ���M��Ԃׁ̈B								*/
		/*--------------------------------------------------------------*/
		case ARC_CR_R_EVT:											// ذ���ް���M
			RD_PrcWriteFlag = 0;
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			inc_dct( READ_YO, 1 );
//			inc_dct( READ_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
			RD_pos = RDT_REC.rdat[1];
			//�� al_preck���ŁARD_pos > 1�̗��\��������Ă��邽�߁A����̂f�s���ʃr�b�g���}�X�N����
			if( prm_get( COM_PRM,S_SYS,11,1,1 ) == 1 ){
				RD_pos &= 0x03;
			}
			if( RD_mod == 8 ){
				RD_mod = 9;
				memset( MAGred, '\0', sizeof(MAGred) );
				if( RDT_REC.idc2 >= 3 ){
					// �ǂݍ��񂾃J�[�h��GT�t�H�[�}�b�g�`�F�b�N
					if(( RDT_REC.rdat[1] & 0x80) == 0x80){
						MAGred[MAG_GT_APS_TYPE] = 1;						// �f�s����̫�ϯ��׸ނ��
						for( i = MAG_EX_GT_PKNO; i < 9; i++ ){				// �g���������ԏ�No���i�[����
							MAGred[MAG_EX_GT_PKNO] |= (char)(( RDT_REC.rdat[i+3] & 0x80 ) >> (8 - i));
						}
					}
					
					//�S�Ẵp���e�B�r�b�g���O��
					for( i = MAG_ID_CODE,cnt = 0; cnt < RDT_REC.idc2 - 3; i++,cnt++ ){
						MAGred[i] = (char)( RDT_REC.rdat[cnt+3] & 0x7f );
					}
					chk = NG;
						// �g��ID�`�F�b�N�FAPS�̂ݏ������s��
						if(MAGred[MAG_GT_APS_TYPE] == 0){
							chk = Read_Tik_Chk( (uchar *)&MAGred[MAG_ID_CODE], &pk_syu, (uchar)(RDT_REC.idc2 - 3) );	// ���ް�������ID�ϊ�
						}

						if( chk != OK && ck_jis_credit ( (uchar)(RDT_REC.idc2 - 3), (char*)&RDT_REC.rdat[3] ) != 0 ){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//							if( CREDIT_ENABLED() ){
//								RD_Credit_kep = 1;
//							}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
							disp_card_chk = 1;
						}else if( MAGred[MAG_ID_CODE] == 0x1a ){
							pas_kep = (char)-1;							// ����ۗ����
						}
					OPECTL.LastCard = OPE_LAST_RCARD_MAG;
					OPECTL.LastCardInfo = (ushort)MAGred[MAG_ID_CODE];
					if( disp_card_chk == 1 ){							// ���f�[�^��\�����Ȃ��H
						OPECTL.LastCardInfo |= 0x8000;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						// �N���W�b�g��JIS�f�[�^�͓����������ɕێ����Ȃ�
						memset( MAGred, '\0', sizeof(MAGred) );
						memset( RDT_REC.rdat, '\0', sizeof(RDT_REC.rdat) );
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
					}
					OPECTL.CR_ERR_DSP = 0;
					OPECTL.other_machine_card = 0;
					if( (MAGred[MAG_ID_CODE] >= 0x21 && MAGred[MAG_ID_CODE] <= 0x2B) ){
						OPECTL.other_machine_card = 1;
					}
				}
			}else{
				MAGred[MAG_ID_CODE] = 0xff;
				OPECTL.LastCardInfo = (ushort)0;
				OPECTL.CR_ERR_DSP = 0;
			}
			break;

		/*------------------*/
		/* �ް�ޮ��ް���M	*/
		/*------------------*/
		case ARC_CR_VER_EVT:										// �ް�ޮ��ް���M
			RD_mod = 20;
			break;

		/*----------------------*/
		/* ���ވ������������	*/
		/*----------------------*/
		case ARC_CR_EOT_EVT:										// ���ޔ������
			if( RD_mod == 11 ){
				if( OPE_red != 5 ){
					RD_mod = 6;
					if( RED_REC.ercd == E_VERIFY ){					// ���̧��װ
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_VN, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}
				}
				RED_REC.ercd = 0;
				if( rd_tik == 2 ){									// ���Ԍ��̎��������҂�?
					rd_tik = 0;
				}
				if( pas_kep ){										// ����ۗ�?
					pas_kep = 0;
				}
			}
			break;

		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް����ޑ��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_snd( mod )                                          |*/
/*| PARAMETER    : mod  : ���M���e                                         |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	opr_snd( short mod )
{
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j
//	short	ans, cmd;
//	short 	ret;
//	uchar	set;
//
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) == 0 ){						// ���Cذ�ް�g�p�s��?
//		return( 0 );
//	}
//// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
//	if( ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ){
//		switch(mod) {
//		case 90:	// ��ԗv��
//		case 91:	// �o�[�W�����v��
//		case 94:	// ROM����
//		case 95:	// DIPSW���
//			break;
//		default:
//			return( 0 );
//		}
//	}
//// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
//
//	if(( RD_mod == 12 )||( RD_mod == 13 )){							// ذ�ް���쒆?
//		opr_bak = mod;												// Yes
//		return( 0 );
//	}
//
//	RD_SendCommand = 0;
//	switch( mod ){
//		case 2:					// ���[�_�[�������o�����ֈړ�
//		case 13:				// �ۗ��ʒu�����o�����ֈړ�
//		    // --------------------------------------------------------------------------------------------- //
//			// read_sht_opn�֐����ŵ���ݍς݂̏ꍇ�͉������Ȃ��ׁA�����̉ӏ��ł��łɃV���b�^�[�J�v����       //
//			// �|���Ă��Ă����Ȃ��̂ŁA����ݘR���h���ׂɎ��C���[�_�[�֖߂��v�����|���钼�O�ɊJ�v������   //
//		    // --------------------------------------------------------------------------------------------- //
//			if( !OPECTL.Mnt_mod )	// ���Z���[�h�̂Ƃ��̂�
//				read_sht_opn();
//			break;
//		default:
//			break;
//	}
//	ans = 0;
//	switch( mod ){
//		case 0:														// Initial Command
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_init( 2 );
//			}
//			else {
//				ret = rd_init( 4 );									// �v�����^�Ȃ�
//			}
//			if(ret == 0) {
//				RD_mod = 1;
//// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			} else {
//				ans = -1;
//			}
//			break;
//		case 1:														// ذ�޺����(���Ԍ��ۗ�ż)
//			if(( RD_mod == 11 )||(( RD_mod == 8 )&&( w_stat2 ))) break;
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_read( 0 );
//			}
//			else {
//				ret = rd_read( 2 );
//			}
//			if( ret == 0 ) {
//				RD_mod = 7;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 2:														// ��o�����ړ�
//			if( rd_mov( rd_faz = 0 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 3:														// ذ�޺����(���Ԍ��ۗ���)
//			if(( RD_mod == 11 )||(( RD_mod == 8 )&&( w_stat2 )) || CardStackRetry) break;		// �r�o���쒆�̓��[�h�R�}���h�o���Ȃ�
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_read( 1 );
//			}
//			else {
//				ret = rd_read( 2 );
//			}
//			if( ret == 0 ) {
//				RD_mod = 7;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 4:														// ���Ԍ�(�̎���)�捞(��r�o)
//			if( rd_mov( rd_faz = 12 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 5:														// ���Ԍ��捞(��r�o)
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				rd_faz = 2;
//			}
//			else {
//				rd_faz = 33;
//			}
//
//			if( rd_mov( rd_faz ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 6:														// ���Ԍ��̎���
////			RD_pos = tikchu.pos;									// Head pos���
//			if( rd_mov( rd_faz = 4 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 7:														// ���Z���~������(���̧��L��)
////			RD_pos = tikchu.pos;									// Head pos���
//			if( rd_mov( rd_faz = 13 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 8:														// ���Ԍ��󎚎捞
//			if( rd_mov( rd_faz = 14 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 110:													// ���Ԍ��󎚎捞
//			if( rd_mov( rd_faz = 21 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 9:														// �������(���̧��L��)
//			if( rd_mov( rd_faz = 5 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 10:													// ���޽���捞(��r�o)
//		case 11:													// ���޽���捞(�O�ړ�+��r�o)
//			memset( &MDP_buf[0], ' ', 30 );
//			if( CPrmSS[S_DIS][2] ){									// �p��ϰ��󎚂���
//				MDP_buf[29] = 0x58;									// 'X'
//			}
//			MDP_siz = 30;
//			set = (uchar)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				cmd = 6;
//				if( mod == 11 ) cmd = 7;
//			}
//			else {
//				cmd = 33;
//			}
//			if( rd_mov( rd_faz = cmd ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 12:													// ����߲�޶��ޏ���(���̧��L��)
//			if( rd_mov( rd_faz = 10 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 13:													// �����o�����ړ�(���Ԍ��߂�)
//			if( rd_mov( rd_faz = 1 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 14:													// ���Ԍ��a�����(���~)
////			RD_pos = tikchu.pos;									// Head pos���
//			if( rd_mov( rd_faz = 17 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 15:													// ���Ԍ��a�����(���~)
////			RD_pos = tikchu.pos;									// Head pos���
//			if( rd_mov( rd_faz = 16 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 16:													// �������(���̧�����)
//			if( rd_mov( rd_faz = 22 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 17:													// ����߲�޶��ޏ���(���̧�����)
//			if( rd_mov( rd_faz = 26 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 18:													// ���Z���~������(���̧��Ȃ�)
////			RD_pos = (uchar)CRD_DAT.TIK.pos;						// Head pos���
//			if( rd_mov( rd_faz = 28 ) == 0 ) {
//				rd_tik = 2;											// ���Ԍ��߂� �����҂�
//			}else{
//				ans = -1;
//			}
//			break;
//		case 19:													// �񐔌�����(���̧��Ȃ�)
//			if( rd_mov( rd_faz = 29 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 20:													// �񐔌�����(���̧�����)
//			if( rd_mov( rd_faz = 31 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 90:													// ��ԗv��
//			if( rd_test( 3 ) == 0 ) {
//				RD_mod = 15;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 91:													// �ް�ޮݗv��
//			if( rd_test( 4 ) == 0 ) {
//				RD_mod = 16;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 92:													// ذ��ײ�ýėv��
//			if( rd_test( 1 ) == 0 ){
//// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				RD_mod = 17;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 93:													// ��ýėv��
//			if( rd_test( 2 ) == 0 ){
//// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				RD_mod = 18;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 94:													// ROM���ԓǏo���v��
//			// FB7000 ����ݽ Command���M
//			if( rd_FB7000_MntCommandSend( 1, 0, 0 ) == 0 ){			// ROM���ԓǏo���v��
//				RD_mod = 19;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 95:													// ROM���ԓǏo���v��
//			// FB7000 ����ݽ Command���M
//			if( rd_FB7000_MntCommandSend( 9, 0, 0 ) == 0 ){			// �f�B�b�v�X�C�b�`��Ԏ擾�v��
//				RD_mod = 19;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 100:													// ���Ԍ������i�����j
//			if( rd_mov( rd_faz = 18 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 101:													// �p��
//			if( rd_mov( rd_faz = 19 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 102:											// �p����������
//			memset( &MDP_buf[0], ' ', 30 );
//			MDP_siz = 30;
//			if( rd_mov( rd_faz = 36 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 200:													// ���ގ�荞��
//			if( rd_mov( rd_faz = 23 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 201:													// �������(���ޔ��s�p)
//			if( rd_mov( rd_faz = 24 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//
//		case 202:													// �W�����ޏ���(���ޔ��s�p)
//			if( rd_mov( rd_faz = 25 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//
//		default:
//			break;
//	}
//	if( ans == -1 ){
//		RD_mod = 0;
//	}
//	return( ans );
	// �����ő҂����킹�������s���Ă��܂����߁A�֐��X�^�u��
	return -1;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i���C���[�_�[��Ή����j
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް����������                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_int( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_int( void )
{
	if( RD_mod == 0 ){
		opr_snd( 0 );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���޽��ذ��                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_svsck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = �ް��ُ�                                     |*/
/*|                       3 = �����؂�                                     |*/
/*|                       6 = �����O                                       |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      14 = ���x�������ް                                |*/
/*|                      25 = �Ԏ�װ                                      |*/
/*|                      26 = �ݒ�װ                                      |*/
/*|                      27 = ������ʴװ                                  |*/
/*|                      28 = ���x����0�װ                                 |*/
/*|                      30 = ���Z�����װ                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const short pno_dt[8] = { 0x0000, 0x0200, 0x0100, 0x0300, 0x0080, 0x0280, 0x0180, 0x0380 };
const char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };
const char prm_pos[4] = { 0, 5, 3, 1 };
const char magtype[4] = {6,1,2,3};

short	al_svsck( m_gtservic *mag )
{
	short	ret, wk;
	short	cardknd;
	ushort	pkno_syu;
	uchar	chk_end1;		// ���ް���v�׸�	�iOFF=�s��v�^ON=��v�j
	uchar	chk_end2;		// ����ʕs��v�׸�	�iOFF=��v�^ON=�s��v�j
	uchar	tbl_no;			// �����ύXð���No.
	short	tbl_syu;		// �����ύXð����ް��F���
	short	tbl_data;		// �����ύXð����ް��F���e
	short	tbl_syear;		// �����ύXð����ް��F�J�n�N�ް�
	short	tbl_smon;		// �����ύXð����ް��F�J�n���ް�
	short	tbl_sday;		// �����ύXð����ް��F�J�n���ް�
	short	tbl_eyear;		// �����ύXð����ް��F�I���N�ް�
	short	tbl_emon;		// �����ύXð����ް��F�I�����ް�
	short	tbl_eday;		// �����ύXð����ް��F�I�����ް�
	ushort	tbl_sdate;		// �����ύXð����ް��F�J�n�N�����idnrmlzm�ϊ��ް��j
	ushort	tbl_edate;		// �����ύXð����ް��F�I���N�����idnrmlzm�ϊ��ް��j
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu
	int		i;
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	long	role = 0;		// ����
	long	chng = 0;		// �Ԏ�؊�
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j

	CRD_DAT.SVS.pno = 												// ���ԏꇂ���
		(long)mag->servic.svc_pno[0] + (long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GT�t�H�[�}�b�g
		CRD_DAT.SVS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17�i�[
		CRD_DAT.SVS.GT_flg = 1;										//GT�t�H�[�}�b�g�t���O�Z�b�g
	}else{
		CRD_DAT.SVS.GT_flg = 0;										//GT�t�H�[�}�b�g�t���O�Z�b�g	
	}

	CRD_DAT.SVS.knd = (short)( mag->servic.svc_pno[1] & 0x0f );		// ��ʾ��
	CRD_DAT.SVS.cod =												// �X�����
		(short)mag->servic.svc_sno[1] + (((short)mag->servic.svc_sno[0])<<7);
	CRD_DAT.SVS.sts = (short)( mag->servic.svc_sts );				// �ð��

	if( memcmp( mag->servic.svc_sta, ck_dat, 6 ) == 0 ){			// ������?
		CRD_DAT.SVS.std = 0;										// �L���J�n�����
		CRD_DAT.SVS.end = 0xffff;									// �L���I�������
	}else{
		wk = (short)(mag->servic.svc_sta[0]);
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_sta[1], (short)mag->servic.svc_sta[2] ) ){	// �L���J�n������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.SVS.std = 											// �L���J�n�����
			dnrmlzm( wk,(short)mag->servic.svc_sta[1],(short)mag->servic.svc_sta[2] );

		wk = (short)mag->servic.svc_end[0];
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_end[1], (short)mag->servic.svc_end[2] ) ){	// �L���I��������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.SVS.end = 											// �L���I�������
			dnrmlzm( wk,(short)mag->servic.svc_end[1],(short)mag->servic.svc_end[2] );
	}

	ret = 0;
	for( ; ; ){
		if( CRD_DAT.SVS.knd == 0 ){
			if( CRD_DAT.SVS.sts == 0 ){
				cardknd = 12;										// �|����
			}else{
				cardknd = 14;										// ������
			}
		}else{
			cardknd = 11;											// ���޽��
		}

		if( 0L == (ulong)CRD_DAT.SVS.pno ){
			ret = 1;												// ���ԏꇂ�װ
			break;
		}

		// ���z���Z��̃T�[�r�X���ނ͖���
		i = is_paid_remote(&PayData);
		if (i >= 0 && PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) {
			ret = 8;
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 )&&			// ��{�T�[�r�X���g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.SVS.pno )){ 		// ��{���ԏꇂ?
			pkno_syu = KIHON_PKNO;									// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 )&&		// �g��1�T�[�r�X���g�p��
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.SVS.pno )){	// �g��1���ԏꇂ?
			pkno_syu = KAKUCHOU_1;									// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 )&&		// �g��2�T�[�r�X���g�p��
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.SVS.pno )){	// �g��2���ԏꇂ?
			pkno_syu = KAKUCHOU_2;									// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 3 ) == 1 )&&		// �g��3�T�[�r�X���g�p��
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.SVS.pno )){	// �g��3���ԏꇂ?
			pkno_syu = KAKUCHOU_3;									// �g��3
		}
		else{
			ret = 1;												// ���ԏꇂ�װ
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.SVS.GT_flg, pkno_syu ) == 1 ){		// �ݒ�NG�`�F�b�N
			ret = 33;
			break;
		}

		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.SVS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.SVS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > APSF_PKNO_UPPER))) ){		//���ԏ�No�͈̓`�F�b�N
				ret = 1;												// ���ԏꇂ�װ
				break;
			}
		}

		wk = (short)prm_get( COM_PRM, S_DIS, 1, 2, 1 );				// �g�p���x�����ݒ�
		if(wk == 0){
			ret = 28;												// �g�p���x�����ݒ�0��
			break;
		}
		if( card_use[USE_SVC] >= wk ){								// ���޽��,�|����,�������g�p����
			ret = 14;												// ���x�������ް
			break;
		}
		if(cardknd == 11){
			if( 1 == (short)prm_get( COM_PRM, S_DIS, 1, 1, 6 ) ){		// ��ʎg�p���x�����ݒ�H
				if( CRD_DAT.SVS.knd <= 3 ){
					data_adr = 106;											// �g�p���x�����ʂ��ް����ڽ�擾
					data_pos = (char)prm_pos[CRD_DAT.SVS.knd];				// �g�p���x�������ް��ʒu�擾
				}else if( CRD_DAT.SVS.knd <= 6 ){
					data_adr = 107;											// �g�p���x�������ް����ڽ�擾
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-3)];			// �g�p���x�������ް��ʒu�擾
				}else if( CRD_DAT.SVS.knd <= 9 ){
					data_adr = 108;											// �g�p���x�������ް����ڽ�擾
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-6)];			// �g�p���x�������ް��ʒu�擾
				}else if( CRD_DAT.SVS.knd <= 12 ){
					data_adr = 109;											// �g�p���x�������ް����ڽ�擾
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-9)];			// �g�p���x�������ް��ʒu�擾
				}else if( CRD_DAT.SVS.knd <= 15 ){
					data_adr = 110;											// �g�p���x�������ް����ڽ�擾
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-12)];			// �g�p���x�������ް��ʒu�擾
				}
				wk = (short)prm_get( COM_PRM, S_SER, data_adr, 2, data_pos );// �g�p���x�����ݒ�
				if(wk == 0 ){
					ret = 28;												// �g�p���x�����ݒ�0��
					break;
				}
				if(card_use2[CRD_DAT.SVS.knd-1] >= wk ){
					ret = 14;												// ���x�������ް
					break;
				}
			}
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, cardknd, CRD_DAT.SVS.knd, CRD_DAT.SVS.cod, CRD_DAT.SVS.sts ) ){	// 1���Z�̊�����ނ̌������ް?
				ret = 14;											// ���x�������ް
				break;
			}
		}

		// �������ύX����
		chk_end1 = OFF;												// ���ް���v�׸ށFOFF

		for( tbl_no = 1 ; tbl_no <= 3 ; tbl_no++ ){					// �����ύXð��فi�P�`�R�j����

			chk_end2 = OFF;											// ��ʕs��v�׸ށFOFF

			tbl_syu = (short)CPrmSS[S_DIS][8+((tbl_no-1)*6)];		// �����ύXð��قɐݒ肳��Ă����ʂ��擾
			tbl_data= (short)CPrmSS[S_DIS][9+((tbl_no-1)*6)];		// �����ύXð��قɐݒ肳��Ă�����e���擾

			// ���������
			switch( tbl_syu ){										// �ݒ肳��Ă����ʂƌ��ް��̎�ʂ��r

				case	1:											// �ݒ��ʁ��T�[�r�X��

					if( cardknd != 11 ){							// ���ް���ʁ��T�[�r�X���H
						chk_end2 = ON;								// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){						// �ݒ���e���O�i�T�[�r�X���S�āj�ȊO�H
							if( tbl_data != CRD_DAT.SVS.knd ){		// �T�[�r�X�����(A�`C)��v�H
								chk_end2 = ON;						// NO �� ��ʕs��v
							}
						}
					}
					break;

				case	2:											// �ݒ��ʁ��|����
					if( cardknd != 12 ){							// ���ް���ʁ��|�����H
						chk_end2 = ON;								// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){						// �ݒ���e���O�i�|�����S�āj�ȊO�H
							if( tbl_data != CRD_DAT.SVS.cod ){		// �XNo.��v�H
								chk_end2 = ON;						// NO �� ��ʕs��v
							}
						}
					}
					break;

				case	3:											// �ݒ��ʁ�������
					if( cardknd != 14 ){							// ���ް���ʁ��������H
						chk_end2 = ON;								// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){						// �ݒ���e���O�i�������S�āj�ȊO�H
							if( tbl_data != CRD_DAT.SVS.cod ){		// �XNo.��v�H
								chk_end2 = ON;						// NO �� ��ʕs��v
							}
						}
					}
					break;

				case	4:											// �ݒ��ʁ��S��
					break;											// �T�[�r�X���A�|�����A�������S�Ĉ�v�Ɣ��f

				case	0:											// �ݒ��ʁ��Ȃ�
				default:											// �ݒ��ʁ����̑�
					chk_end2 = ON;									// �� ��ʕs��v
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}

			if( chk_end2 == ON ){									// �ݒ肳��Ă����ʂƌ��ް��̎�ʕs��v�H
				continue;											// YES �� ���蒆�̊����ύXð��ٌ����I��
			}

			// �L����������

			tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 5 );	// �ύX�O�̊J�n�i�N�j�擾
			tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 3 );	// �ύX�O�̊J�n�i���j�擾
			tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 1 );	// �ύX�O�̊J�n�i���j�擾

			tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 5 );	// �ύX�O�̏I���i�N�j�擾
			tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 3 );	// �ύX�O�̏I���i���j�擾
			tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 1 );	// �ύX�O�̏I���i���j�擾

			if( (CRD_DAT.SVS.std == 0) && (CRD_DAT.SVS.end == 0xffff) ){					// ���ް��̗L�������H
				// ���ް���������
				if(	(tbl_syear == 0) && (tbl_smon == 0) && (tbl_sday == 0) &&
					(tbl_eyear == 99) && (tbl_emon == 99) && (tbl_eday == 99) ){			// �����ύXð��قɐݒ肳��Ă���ύX�O�L�������H

					chk_end1 = ON;															// ������ �� ���ް���v
				}
			}
			else{
				// ���ް����L����������
				if( tbl_syear >= 80 ){		// �J�n�N�ް��ϊ�(����Q��������S��)
					tbl_syear += 1900;
				}else{
					tbl_syear += 2000;
				}
				if( tbl_eyear >= 80 ){		// �I���N�ް��ϊ�(����Q��������S��)
					tbl_eyear += 1900;
				}else{
					tbl_eyear += 2000;
				}
				tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );	// �L���J�n���ϊ�
				tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );	// �L���I�����ϊ�

				if( (CRD_DAT.SVS.std == tbl_sdate) && (CRD_DAT.SVS.end == tbl_edate) ){		// �J�n�����I������v�H

					chk_end1 = ON;															// YES �� ���ް���v
				}
			}

			if( chk_end1 == ON ){									// ���ް���v�H

				// ���ް��i��ʁ��L�������j�Ɗ����ύXð����ް��i��ʁ��ύX�O�L�������j����v�����ꍇ

				tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 5 );	// �ύX��̊J�n�i�N�j�擾
				tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 3 );	// �ύX��̊J�n�i���j�擾
				tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 1 );	// �ύX��̊J�n�i���j�擾

				tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 5 );	// �ύX��̏I���i�N�j�擾
				tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 3 );	// �ύX��̏I���i���j�擾
				tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 1 );	// �ύX��̏I���i���j�擾

				if(	(tbl_syear == 0) && (tbl_smon == 0) && (tbl_sday == 0) &&
					(tbl_eyear == 99) && (tbl_emon == 99) && (tbl_eday == 99) ){	// �ύX��L�������H

					// �L�������Ȃ��i�������j
					tbl_sdate = 0;
					tbl_edate = 0xffff;
				}
				else{
					if( tbl_eyear >= 80 ){
						// �N��80�ȏ���͂���Ă����璲������B
						tbl_eyear = 79; tbl_emon = 12; tbl_eday = 31;
	 				}
					// �L����������
					if( tbl_syear >= 80 ){		// �J�n�N�ް��ϊ�(����Q��������S��)
						tbl_syear += 1900;
					}else{
						tbl_syear += 2000;
					}
					if( tbl_eyear >= 80 ){		// �I���N�ް��ϊ�(����Q��������S��)
						tbl_eyear += 1900;
					}else{
						tbl_eyear += 2000;
					}

					tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );		// �L���J�n���ϊ�
					tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );		// �L���I�����ϊ�
				}
				CRD_DAT.SVS.std = tbl_sdate;									// �L���J�n����ύX��̊J�n���Ƃ���
				CRD_DAT.SVS.end = tbl_edate;									// �L���I������ύX��̏I�����Ƃ���

				break;															// �� �����ύXð��فi�P�`�R�j�����I��
			}
		}

		if( CRD_DAT.SVS.std > CLK_REC.ndat ){						// �����O
			ret = 6;												// �����O�װ
			break;
		}
		if( CRD_DAT.SVS.end < CLK_REC.ndat ){						// �����؂�
			ret = 3;												// �����؂�װ
			break;
		}

		if( cardknd == 12 ){
			/*** �|���� ***/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			role = prm_get( COM_PRM, S_STO, (short)(1+3*(CRD_DAT.SVS.cod-1)), 1, 1 );
			chng = prm_get( COM_PRM, S_STO, (short)(3+3*(CRD_DAT.SVS.cod-1)), 2, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			if( !rangechk( 1, 100, CRD_DAT.SVS.cod ) ) {			// �X���͈͊O?
				ret = 13;											// ��ʋK��O
				break;
			}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if (( CPrmSS[S_STO][1+3*(CRD_DAT.SVS.cod-1)] == 0L )&&
//				( CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)] == 0L ))
			if (( role == 0L )&&
			    ( chng == 0L ))
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			{
				ret = 26;											// �������ݒ�
				break;
			}

// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if( CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)] ){			// ��ʐؑ֗L��?
//				if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
			if( chng ){												// ��ʐؑ֗L��?
				if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(chng-1)),2,5 ) == 0L ) {									// �Ԏ�ݒ�Ȃ�?
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ret = 27;										// ������ʴװ
					break;
				}
				if(( ryo_buf.nyukin )||( ryo_buf.waribik )||		// �����ς�? or �����ς�?
				   ( e_incnt > 0 )||								// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
				   ( c_pay )) {										// or ����߲�޶��ގg�p����?
					ret = 30;										// ���Z�����װ
					break;
				}
			}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if( CPrmSS[S_STO][1+3*(CRD_DAT.SVS.cod-1)] == 1L ){		// ���Ԋ���?
			if( role == 1L ){										// ���Ԋ���?
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
				if( card_use[USE_PPC] || card_use[USE_NUM] ||		// ����߲�޶��� or �񐔌��g�p�ς�?
					(e_incnt > 0))									// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
				{
					ret = 30;										// ���Z�����װ
					break;
				}
			}
		}else if( cardknd == 14 ){
			/*** ������ ***/
			if(( CPrmSS[S_WAR][1] == 0 )||							// �������g�p���Ȃ��ݒ�?
			   ( !rangechk( 1, 100, CRD_DAT.SVS.sts ) )||			// ������ʔ͈͊O?
			   (( CPrmSS[S_WAR][2+3*(CRD_DAT.SVS.sts-1)] == 0L )&&	// ���ݒ�
			    ( CPrmSS[S_WAR][4+3*(CRD_DAT.SVS.sts-1)] == 0L )))
			{
				ret = 13;											// ��ʋK��O
				break;
			}
		}else{
			/*** ���޽�� ***/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			role = prm_get( COM_PRM, S_SER, (short)(1+3*(CRD_DAT.SVS.knd-1)), 1, 1 );
			chng = prm_get( COM_PRM, S_SER, (short)(3+3*(CRD_DAT.SVS.knd-1)), 2, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			if( !rangechk( 1, SVS_MAX, CRD_DAT.SVS.knd ) ){		// ���޽���͈͊O?
				ret = 13;										// ��ʋK��O
				break;
			}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if (( CPrmSS[S_SER][1+3*(CRD_DAT.SVS.knd-1)] == 0L )&&
//				( CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)] == 0L ))
			if (( role == 0L ) &&
				( chng == 0L ))
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			{
				ret = 26;											// �������ݒ�
				break;
			}

// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if( CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)] ){			// ��ʐؑ֗L��?
//				if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
			if( chng ){												// ��ʐؑ֗L��?
				if( prm_get( COM_PRM,S_SHA,(short)(1+6*(chng-1)),2,5 ) == 0L ) {									// �Ԏ�ݒ�Ȃ�?
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ret = 27;										// ��ʐؑւȂ�
					break;
				}
				if(( ryo_buf.nyukin )||( ryo_buf.waribik )||		// �����ς�? or �����ς�?
				   ( e_incnt > 0 )||								// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
				   ( c_pay )) {										// or ����߲�޶��ގg�p����?
					ret = 30;										// ���Z�����װ
					break;
				}
			}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//			if( CPrmSS[S_SER][1+3*(CRD_DAT.SVS.knd-1)] == 1L ){		// ���Ԋ���?
			if( role == 1L ){										// ���Ԋ���?
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
				if( card_use[USE_PPC] || card_use[USE_NUM] ||		// ����߲�޶��� or �񐔌��g�p�ς�?
					(e_incnt > 0))									// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
				{
					ret = 30;										// ���Z�����װ
					break;
				}
			}
			if( ryo_buf.syubet < 6 ){
				// ���Z�Ώۂ̗������A�`F(0�`5)
				data_adr = 2*(CRD_DAT.SVS.knd-1)+76;					// �g�p�\������ʂ��ް����ڽ�擾
				data_pos = (char)(6-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
			}
			else{
				// ���Z�Ώۂ̗������G�`L(6�`11)
				data_adr = 2*(CRD_DAT.SVS.knd-1)+77;					// �g�p�\������ʂ��ް����ڽ�擾
				data_pos = (char)(12-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
			}
			if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) ){		// �g�p�s�ݒ�H
				ret = 25;												// ���̎Ԏ�̌�
				break;
			}
		}
		vl_now = V_SAK;												// ���޽��(����/��������)
		break;
	}
	if( ret == 0 )
	{
		(void)vl_svschk( (m_gtservic*)mag );							// �����v�Z�p���޽���ް����
		card_use[USE_SVC] += 1;										// ���޽��,�|����,����������+1
		card_use[USE_N_SVC] += 1;									// �V�K���޽��,�|����,����������+1
		if(cardknd == 11){//�T�[�r�X��
			card_use2[CRD_DAT.SVS.knd-1] += 1;									// ���޽��+1
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 1���Z�Ɏg�p�������޽��,�|����,�������̎�ނ������Ă��Ȃ�����������     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CardSyuCntChk()                                         |*/
/*| PARAMETER    : pksy  : ���ԏꇂ                                        |*/
/*|              : cdknd : 11=���޽��, 12=�|����, 13=�񐔌��C14=������     |*/
/*|              : knd   : ���޽��A�`O (1-15, ���޽���ȊO��0)              |*/
/*|              : cod   : �X��(���޽���̏ꍇ�͊|���溰��)                 |*/
/*|              : sts   : �������̊������(�������ȊO��0)                 |*/
/*| RETURN VALUE : ret   : 0 = OK                                          |*/
/*|                        1 = 1���Z���x��ނ𒴂��Ă���                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-29                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	CardSyuCntChk( ushort pksy, short cdknd, short knd, short cod, short sts )
{
	ushort	ret;
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;

	ret = 1;

// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 	if( CardUseSyu >= WTIK_USEMAX ){
	if ( CardUseSyu >= (WTIK_USEMAX - 1) ) {	// ���ꊄ�����퐔���ő�l�ȏ�(�����̔��莞�͔������z���v�̕������O����)
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������

		if( cdknd == 12 ){											// �|����
			cdknd = KAKEURI;
		}
		else if( cdknd == 13 ){										// �񐔌�
			cdknd = KAISUU;
		}
		else if( cdknd == 14 ){										// ������
			cdknd = WARIBIKI;
		}
		else if( cdknd == 15 ){										// ������
			cdknd = PREPAID;
		}
		else{														// ���޽��
			cdknd = SERVICE;
		}

		for( i=0; i<WTIK_USEMAX; i++ ){
			switch( PayData.DiscountData[i].DiscSyu ){
				case NTNET_SVS_T:
				case NTNET_SVS_M:
					tik_syu = SERVICE;
					break;
				case NTNET_KAK_T:
				case NTNET_KAK_M:
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ޏ���`�F�b�N�̕ύX)
				case NTNET_TKAK_T:
				case NTNET_TKAK_M:
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ޏ���`�F�b�N�̕ύX)
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
// 				case NTNET_SHOP_DISC_AMT:
// 				case NTNET_SHOP_DISC_TIME:
// // MH810100(E) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					tik_syu = KAKEURI;
					break;
				case NTNET_FRE:
					tik_syu = KAISUU;
					break;
				case NTNET_WRI_M:
				case NTNET_WRI_T:
					tik_syu = WARIBIKI;
					break;
				case NTNET_PRI_W:
					tik_syu = PREPAID;
					break;
				default :
					tik_syu = 0xff;
					break;
			}
			ParkingNo = CPrmSS[S_SYS][pksy+1];
			if(( tik_syu == cdknd ) &&								// ���킪����?
			   ( PayData.DiscountData[i].ParkingNo == ParkingNo )){	// ���ԏꇂ������?
				if( cdknd == KAKEURI ){								// �|����
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					if( PayData.DiscountData[i].DiscNo == (ushort)cod ){	// �X������?
					if( (PayData.DiscountData[i].DiscNo == (ushort)cod) &&	// �X��
						(PayData.DiscountData[i].DiscInfo1 == (ulong)knd) ){// �������/�������
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
						ret = 0;
						break;
					}
				}
				else if( cdknd == KAISUU ){							// �񐔌�
					ret = 1;
				}
				else if( cdknd == WARIBIKI ){						// ������
					ret = 1;
				}
				else if( cdknd == PREPAID ){						// �v���y�C�h�J�[�h
					ret = 1;
				}
				else{												// ���޽��
					if( (PayData.DiscountData[i].DiscNo == (ushort)knd) &&	// A�`O������
						(PayData.DiscountData[i].DiscInfo1 == (ulong)cod) ){// �|���溰�ނ�����
						ret = 0;
						break;
					}
				}
			}
		}
	}else{
		ret = 0;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ذ��                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = �ް��ُ�                                     |*/
/*|                       3 = �����؂�                                     |*/
/*|                       4 = ���o�ɴװ                                    |*/
/*|                       5 = �������                                     |*/
/*|                       6 = �����O                                       |*/
/*|                       9 = ��d�g�p�װ                                  |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      25 = �Ԏ�װ                                      |*/
/*|                      26 = �ݒ�װ                                      |*/
/*|                      27 = ������ʴװ                                  |*/
/*|                      29 = �⍇���װ                                    |*/
/*|                      30 = ���Z�����װ                                  |*/
/*|                      99 = HOST�֖⍇��                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_pasck( m_gtapspas* mag )
{
	short	ret;
	short	wk, wksy, wkey;
	short	w_syasyu;
	ushort	us_day;
	short	s_ptbl;
	ushort	us_psts[3];
	char	c_prm;
	ulong	ul_AlmPrm;
	uchar	uc_GtAlmPrm[10];
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu
	uchar	KigenCheckResult;
	int		i;
	char	w_cbuff[16];

	CRD_DAT.PAS.pno = 												// ���ԏꇂ���
		(long)mag->apspas.aps_pno[0] + (long)pno_dt[mag->apspas.aps_pno[1]>>4];
	if(mag->magformat.type == 1){//GT�t�H�[�}�b�g
		CRD_DAT.PAS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17�i�[
		CRD_DAT.PAS.GT_flg = 1;													//GT�t�H�[�}�b�g�t���O�Z�b�g
	}else{
		CRD_DAT.PAS.GT_flg = 0;
	}
	CRD_DAT.PAS.knd = (short)(mag->apspas.aps_pno[1]&0x0f);			// ��ʾ��
	CRD_DAT.PAS.cod =												// �l���޾��
		(short)mag->apspas.aps_pcd[1] + (((short)(mag->apspas.aps_pcd[0]))<<7);

	if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 1 ){	// �����؂ꎞ��t����ݒ�
		c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );	// �������ݒ�Get
	}else{
		c_prm = 0;
	}

	wksy = (short)mag->apspas.aps_sta[0];
	if( wksy >= 80 ){
		wksy += 1900;
	}else{
		wksy += 2000;
	}

	if( c_prm == 1 || c_prm == 2 ){									// ������ or �J�n��������
		CRD_DAT.PAS.std = dnrmlzm( 1980, 3, 1 );					// 1980�N3��1��
	}else{
		if( chkdate( wksy, (short)mag->apspas.aps_sta[1], (short)mag->apspas.aps_sta[2] ) ){	// �L���J�n������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.PAS.std = 											// �L���J�n�����
			dnrmlzm( wksy,(short)mag->apspas.aps_sta[1],(short)mag->apspas.aps_sta[2] );
	}

	CRD_DAT.PAS.std_end[0] = mag->apspas.aps_sta[0];
	CRD_DAT.PAS.std_end[1] = mag->apspas.aps_sta[1];
	CRD_DAT.PAS.std_end[2] = mag->apspas.aps_sta[2];

	wkey = (short)mag->apspas.aps_end[0];
	if( wkey >= 80 ){
		wkey += 1900;
	}else{
		wkey += 2000;
	}
	if( c_prm == 1 || c_prm == 3 ){									// ������ or �I����������
		CRD_DAT.PAS.end = dnrmlzm( 2079, 12, 31 );					// 2079�N12��31��
	}else{
		if( chkdate( wkey, (short)mag->apspas.aps_end[1], (short)mag->apspas.aps_end[2] ) ){	// �L���I��������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.PAS.end = 											// �L���I�������
			dnrmlzm( wkey,(short)mag->apspas.aps_end[1],(short)mag->apspas.aps_end[2] );
	}

	CRD_DAT.PAS.std_end[3] = mag->apspas.aps_end[0];
	CRD_DAT.PAS.std_end[4] = mag->apspas.aps_end[1];
	CRD_DAT.PAS.std_end[5] = mag->apspas.aps_end[2];

	CRD_DAT.PAS.sts = mag->apspas.aps_sts;									// �ð�����
	*((long*)CRD_DAT.PAS.trz) = *((long*)mag->apspas.aps_wrt);				// ���������������

	ret = 0;
	for( ; ; ){
		// ������װю��̒��ԏ�ԍ��ް� �ݒ�`���ύX(NT-NET�d���ύX�ɔ���)
		if(mag->magformat.type == 0){
			ul_AlmPrm = (ulong)CRD_DAT.PAS.pno;
			ul_AlmPrm *= 100000L;									// ��������ԏ�set
			ul_AlmPrm += (ulong)CRD_DAT.PAS.cod;					// �����IDset(1�`12000)
		}else{
			memset(uc_GtAlmPrm,0x00,10);						// 0�N���A

			memcpy(&uc_GtAlmPrm[0],&CRD_DAT.PAS.pno,4);			// ���ԏ�No�Z�b�g
			memcpy(&uc_GtAlmPrm[4],&CRD_DAT.PAS.cod,2);			// �����ID�Z�b�g
		}
		if( 0L == (ulong)CRD_DAT.PAS.pno ){
			ret = 1;												// ���ԏꇂ�װ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&					// ��{������g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PAS.pno )){ 				// ��{���ԏꇂ?
			CRD_DAT.PAS.typ = KIHON_PKNO;									// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&				// �g��1������g�p��
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PAS.pno )){			// �g��1���ԏꇂ?
			CRD_DAT.PAS.typ = KAKUCHOU_1;									// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&				// �g��2������g�p��
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PAS.pno )){			// �g��2���ԏꇂ?
			CRD_DAT.PAS.typ = KAKUCHOU_2;									// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&				// �g��3������g�p��
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PAS.pno )){			// �g��3���ԏꇂ?
			CRD_DAT.PAS.typ = KAKUCHOU_3;									// �g��3
		}
		else{
			ret = 1;														// ���ԏꇂ�װ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.PAS.GT_flg, (ushort)CRD_DAT.PAS.typ ) == 1 ){		// �ݒ�NG����
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			ret = 33;
			break;
		}
		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.PAS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.PAS.pno) || (CRD_DAT.PAS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.PAS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.PAS.pno) || (CRD_DAT.PAS.pno > APSF_PKNO_UPPER))) ){	//���ԏ�No�͈̓`�F�b�N
			
				ret = 1;												// ���ԏꇂ�װ
				if(mag->magformat.type == 0){
					alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
				}else{
					alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
				}
				break;
			}
		}
		s_ptbl = ReadPassTbl( (ulong)CRD_DAT.PAS.pno, CRD_DAT.PAS.cod, us_psts );
		if( s_ptbl == -1 ){											// �װ(���l�ُ�)
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		if( us_psts[0] ){											// �����o�^
			ret = 5;												// �����װ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_MUKOU_PASS_USE, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			memset(w_cbuff, 0, sizeof(w_cbuff));
			sprintf(w_cbuff, "%05d", CRD_DAT.PAS.cod );
			NgLog_write( NG_CARD_PASS, (uchar *)w_cbuff, 5 );
			break;
		}

		if( !rangechk( 1, 15, CRD_DAT.PAS.knd ) ||					// �����ʔ͈͊O
			!rangechk( 0, 3, CRD_DAT.PAS.sts ) )					// �ð���͈͊O
		{
			ret = 13;												// ��ʋK��O�װ
			break;
		}
		if( ryo_buf.syubet < 6 ){
			// ���Z�Ώۂ̗������A�`F(0�`5)
			data_adr = 10*(CRD_DAT.PAS.knd-1)+9;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(6-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		else{
			// ���Z�Ώۂ̗������G�`L(6�`11)
			data_adr = 10*(CRD_DAT.PAS.knd-1)+10;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(12-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// �g�p�s�ݒ�H
			ret = 25;												// ���̎Ԏ�̌�
			break;
		}

		if( PayData.teiki.id != 0 ){								// ��d�g�p?
			ret = 9;												// ��d�g�p�װ
			break;
		}

		if( CPrmSS[S_TIK][9] ){										// n�������L��?
			if( (PassIdBackupTim) &&								// n�������N����?
				(PassPkNoBackup == (ulong)CRD_DAT.PAS.pno) &&
				(PassIdBackup   == (ushort)CRD_DAT.PAS.cod) ){		// �O��Ɠ��궰��
				ret = 9;											// ��d�g�p�װ(n�������װ)
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_N_MINUTE_RULE, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_N_MINUTE_RULE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
		}

		if(( mag->apspas.aps_sts == 0 )&&									// �ް���
		   ( CPrmSS[S_TIK][8] == 0 )){								// �������ǂ܂Ȃ��ݒ�?
			ret = 26;												// �ݒ�װ
			break;
		}

		if( CRD_DAT.PAS.std > CRD_DAT.PAS.end ){					// �L�������ް��ُ�(�J�n���I��)
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		/** ����L�������`�F�b�N����(�`�F�b�N������NT-7700�ɍ��킹��) **/
		KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, CLK_REC.ndat, CLK_REC.nmin );
											// ����L�������`�F�b�N�i�߂�F0=�����J�n�����O�C1=�L���������C2=�����I��������j

		if( (1 != KigenCheckResult) &&								// �����؂�
			(prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 0 ) ){	// �����؂ꎞ��t���Ȃ��ݒ�?

			/*** �����؂�Ŋ����؂�����t���Ȃ��ݒ�̎� ***/
			if( 0 == KigenCheckResult ){							// �����O
				ret = 6;											// �����O�װ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
			else{													// �����؂�
				ret = 3;											// �����؂�װ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
			break;
		}

		w_syasyu = 0;
		c_prm = (char)CPrmSS[S_PAS][1+10*(CRD_DAT.PAS.knd-1)];		// �g�p�ړI�ݒ�Get
		if( !rangechk( 1, 14, c_prm ) ){							// ����g�p�ړI�ݒ�͈͊O
			if (c_prm == 0) {										// �g�p�ړI�F���g�p
				ret = 26;											// �ݒ�װ
			}
			else {
				ret = 13;											// ��ʋK��O
			}
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}
		if( rangechk( 3, 14, c_prm ) ){								// ����Ԏ�؊�?
			w_syasyu = c_prm - 2;									// �Ԏ�؊��p�Ԏ��
			if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(w_syasyu-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�
				ret = 27;											// ������ʴװ
				break;
			}
			if(( ryo_buf.waribik )||								// �����ς�?
			   ( ryo_buf.zankin == 0 )||							// �c��0�~�̎�
			   ( e_incnt > 0 ))										// �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
			{
				ret = 30;											// ���Z���Դװ
				break;
			}
		}
		// ���z���Z��̒�����͖���
		i = is_paid_remote(&PayData);
		if (i >= 0 &&
		   ( PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU || 
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
//		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) || 
//// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE && !vl_frs.antipassoff_req)) ) {
			ret = 8;
			break;
		}
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
		i = is_ParkingWebFurikae( &PayData );
		if (i >= 0 &&  ( PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) ){
			ret = 8;
			break;
		}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�

		if(	(OPECTL.Pay_mod == 2)&&						// �C�����Z
			(vl_frs.antipassoff_req)&&					// �C����������g�p
			(prm_get(COM_PRM, S_TYP, 98, 1, 4)==1)){	// ��������������
			if( syusei[vl_frs.lockno-1].tei_id != CRD_DAT.PAS.cod ){
				ret = 13;											// ��ʋK��O
				break;
			}
		}

		ryo_buf.pass_zero = 0;										// ���������0�~���Z�v���Ȃ�

		if( chk_for_inquiry(1) ) {									// �⍇����?
			if( !ERR_CHK[mod_ntibk][1] ){							// NTNET IBK �ʐM����?
				OPECTL.ChkPassSyu = 1;								// ������⍇��������1=APS�����
				OPECTL.ChkPassPkno = (ulong)CRD_DAT.PAS.pno;		// ������⍇�������ԏꇂ
				OPECTL.ChkPassID = CRD_DAT.PAS.cod;					// ������⍇���������ID
				ret = 99;
				break;
			}else{
				OPECTL.ChkPassSyu = 0xfe;							// �ʐM�s��
				wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );		// �ʐM�s�ǎ��̐ݒ�
				if( wk == 0 ){										// NG�Ŏg�p�s��
					ret = 29;										// �ʐM�s��
					break;
				}else if( wk == 1 ){								// ����0�~
					ryo_buf.pass_zero = 1;							// ���������0�~���Z�v������
				}
			}
		}

		if( OPECTL.ChkPassSyu == 1 ){								// ������⍇����̖⍇�����ʎ�M
			memcpy( CRD_DAT.PAS.trz, &PassChk.OutTime.Mon, 4 );		// ����������������M�ް��ɍX�V
		}
		else if( OPECTL.ChkPassSyu == 0xff ){						// ������⍇����̖⍇�����ʎ�M��ѱ��
			wk = (short)prm_get( COM_PRM,S_NTN,36,1,1 );			// �⍇��������ѱ�Ď��̐ݒ�
			if( wk == 0 ){											// NG�Ŏg�p�s��
				ret = 29;										// ������ѱ��
				break;
			}else if( wk == 1 ){									// ����0�~
				ryo_buf.pass_zero = 1;								// ���������0�~���Z�v������
			}
		}

		if(( DO_APASS_CHK )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(2+10*(CRD_DAT.PAS.knd-1)),1,1 ) )&&	// ���o��������ݒ�?
		   ( OPECTL.ChkPassSyu != 0xff )&&							// ������⍇����̖⍇�����ʎ�M��ѱ�ĈȊO
		   ( OPECTL.ChkPassSyu != 0xfe )){							// �ʐM�s��
			if( mag->apspas.aps_sts != 0 ){								// �ް��݂łȂ�?
				wk = CLK_REC.year;
				us_day = dnrmlzm( (short)wk,(short)CRD_DAT.PAS.trz[0],(short)CRD_DAT.PAS.trz[1] );	// ���ތ���ɰ�ײ��

				if( us_day > CLK_REC.ndat ){
					wk--;											// �NϲŽ
					us_day = dnrmlzm( (short)wk,(short)CRD_DAT.PAS.trz[0],(short)CRD_DAT.PAS.trz[1] );	// ���ތ���ɰ�ײ��
				}
				wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );

				if( us_day > wk ){									// ���ތ��� > ���Ɍ���?
					ret = 4;										// ���o�װ
					break;
				}
				if( us_day == wk ){
					wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );

					us_day = tnrmlz( 0, 0, (short)CRD_DAT.PAS.trz[2], (short)CRD_DAT.PAS.trz[3] );
					if( us_day > wk ){
						ret = 4;									// ���o�װ
						break;
					}
				}
			}
		}
		if( KaisuuWaribikiGoukei || c_pay || PayData_Sub.pay_ryo ){ // ����߁E�񐔌��g�p���͒�����̎g�p��s�Ƃ���
			ret = 9;												// ��d�g�p�װ
		}
		break;
	}

	if( ret == 4 ){													// ���o�װ?
		for( wk=0; wk<TKI_CYUSI_MAX; wk++ ){
			if( CRD_DAT.PAS.cod == tki_cyusi.dt[wk].no &&			// ���~�ް��ƈ�v?
				(ulong)CRD_DAT.PAS.pno == tki_cyusi.dt[wk].pk ){
				tkcyu_ichi = (char)(wk + 1);						// ���~�ǎ�ʒu
				ret = 0;											// ����OK
				break;
			}
		}
		if( ret == 4 ){
			if( chk_for_inquiry(2) ) {								// ����߽NG�̎��̂ݖ⍇����?
				if( !ERR_CHK[mod_ntibk][1] ){						// NTNET IBK �ʐM����?
					OPECTL.ChkPassSyu = 1;							// ������⍇��������1=APS�����
					OPECTL.ChkPassPkno = (ulong)CRD_DAT.PAS.pno;	// ������⍇�������ԏꇂ
					OPECTL.ChkPassID = CRD_DAT.PAS.cod;				// ������⍇���������ID
					// �����ł͑��M�v���������Ȃ��Bopemain��ELE_EVT_STOP�œd�q�}�̂̒�~��҂��Ă��瑗�M����
					ret = 99;
				}else{
					wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );	// �ʐM�s�ǎ��̐ݒ�
					if( wk == 0 ){									// NG�Ŏg�p�s��
						ret = 29;									// �ʐM�s��
					}else if( wk == 1 ){							// ����0�~
						ryo_buf.pass_zero = 1;						// ���������0�~���Z�v������
						ret = 0;
					}else if( wk == 2 ){							// ���ގ����Ő��Z
						ret = 0;
					}
				}
			}
			if( ret == 4){		//���o�װ?
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_ANTI_PASS_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_ANTI_PASS_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			}
		}
	}

	if( ret != 99 ){												// �⍇�����Ă��Ȃ�?
		OPECTL.ChkPassSyu = 0;										// ������⍇��������ر
		OPECTL.ChkPassPkno = 0L;									// ������⍇�������ԏꇂ�ر
		OPECTL.ChkPassID = 0L;										// ������⍇���������ID�ر
		blink_end();												// �_�ŏI��
	}

	if( ret == 0 ){													// ����OK?
		if( w_syasyu ){
			vl_now = V_SYU;											// ��ʐ؊�
			syashu = (char)w_syasyu;								// �Ԏ�
		}else{														// �Ԏ�؊��łȂ�
			vl_now = V_TSC;											// �����(���Ԍ����p�L��)
		}
		vl_paschg();

		InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;	// ��������
		InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;	// ������ð��(�ǎ掞)
		InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;			// �����id
		InTeiki_PayData_Tmp.pkno_syu 	 = CRD_DAT.PAS.typ;			// ��������ԏ�m���D��� (0-3:��{,�g��1-3)
		InTeiki_PayData_Tmp.update_mon 	 = 0;						// �X�V����
		InTeiki_PayData_Tmp.s_year 		 = wksy;					// �L�������i�J�n�F�N�j

		InTeiki_PayData_Tmp.s_mon 		 = mag->apspas.aps_sta[1];			// �L�������i�J�n�F���j
		InTeiki_PayData_Tmp.s_day 		 = mag->apspas.aps_sta[2];			// �L�������i�J�n�F���j
		InTeiki_PayData_Tmp.e_year 		 = wkey;					// �L�������i�I���F�N�j
		InTeiki_PayData_Tmp.e_mon 		 = mag->apspas.aps_end[1];			// �L�������i�I���F���j
		InTeiki_PayData_Tmp.e_day 		 = mag->apspas.aps_end[2];			// �L�������i�I���F���j
		memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );	// ������������
		InTeiki_PayData_Tmp.update_rslt1 = 0;						// ����X�V���Z���̍X�V����			�i�@OK�F�X�V�����@�^�@NG�F�X�V���s�@�j
		InTeiki_PayData_Tmp.update_rslt2 = 0;						// ����X�V���Z���̃��x�����s����	�i�@OK�F���픭�s�@�^�@NG�F���s�s�ǁ@�j
	}
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| ����ް��𐸎Z�ް��ر�ɾ��                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*|	al_pasck() �� �L������Ɣ��f�������Call���邱��					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_set()			                               |*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : 100 = OK�i�����؂�ԋ߁F3���ȓ��j					   |*/
/*|					 0 = OK  �����؂�ԋ߂ł͂Ȃ�						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_pasck_set( void )
{

	memcpy( &PayData.teiki, &InTeiki_PayData_Tmp, sizeof(teiki_use) );
	NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);
	NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);
	NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);
	NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);
	PassIdBackup = (ushort)PayData.teiki.id;							// �����id�ޯ�����(n�������p)
	PassIdBackupTim = (ulong)( CPrmSS[S_TIK][9] * 60 * 2 );			// n�������p��ϰ�N��(500ms Timer)
	PassPkNoBackup = (ulong)CRD_DAT.PAS.pno;						// ��������ԏ�ԍ��ޯ�����(n�������p)
	OPECTL.PassNearEnd = vl_paschg();
	card_use[USE_PAS] += 1;											// ���������+1
	return;
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| �����ذ��(�X�V�p)                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_renewal( m_servic *mag )                       |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = �ް��ُ�                                     |*/
/*|                       3 = �����؂�                                     |*/
/*|                       5 = �������                                     |*/
/*|                       6 = �����O                                       |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      50 = �X�V���ԊO�װ                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_pasck_renewal( m_apspas* mag )
{
	short	ret;
	short	wksy, wkey;
	ushort	us_day;
	short	s_ptbl;
	ushort	us_psts[3];
	ushort	us_prm;

	uchar	r_sts;
	ushort	e_ymd[3];


	CRD_DAT.PAS.pno = 												// ���ԏꇂ���
		(short)mag->aps_pno[0] + (short)pno_dt[mag->aps_pno[1]>>4];
	CRD_DAT.PAS.knd = (short)(mag->aps_pno[1]&0x0f);				// ��ʾ��
	CRD_DAT.PAS.cod =												// �l���޾��
		(short)mag->aps_pcd[1] + (((short)(mag->aps_pcd[0]))<<7);


	wksy = (short)mag->aps_sta[0];
	if( wksy >= 80 ){
		wksy += 1900;
	}else{
		wksy += 2000;
	}
	if( chkdate( wksy, (short)mag->aps_sta[1], (short)mag->aps_sta[2] ) ){	// �L���J�n������NG?
		return( 2 );												// �ް��ُ�
	}
	CRD_DAT.PAS.std = dnrmlzm( wksy,(short)mag->aps_sta[1],(short)mag->aps_sta[2] );	// �L���J�n�����

	CRD_DAT.PAS.std_end[0] = mag->aps_sta[0];
	CRD_DAT.PAS.std_end[1] = mag->aps_sta[1];
	CRD_DAT.PAS.std_end[2] = mag->aps_sta[2];

	wkey = (short)mag->aps_end[0];
	if( wkey >= 80 ){
		wkey += 1900;
	}else{
		wkey += 2000;
	}
	if( chkdate( wkey, (short)mag->aps_end[1], (short)mag->aps_end[2] ) ){	// �L���I��������NG?
		return( 2 );												// �ް��ُ�
	}

	e_ymd[0] = wkey;
	e_ymd[1] = (ushort)mag->aps_end[1];
	e_ymd[2] = (ushort)mag->aps_end[2];

	CRD_DAT.PAS.end = dnrmlzm( wkey,(short)mag->aps_end[1],(short)mag->aps_end[2] );	// �L���I�������

	CRD_DAT.PAS.std_end[3] = mag->aps_end[0];
	CRD_DAT.PAS.std_end[4] = mag->aps_end[1];
	CRD_DAT.PAS.std_end[5] = mag->aps_end[2];

	CRD_DAT.PAS.sts = mag->aps_sts;									// �ð�����
	*((long*)CRD_DAT.PAS.trz) = *((long*)mag->aps_wrt);				// ���������������

	ret = 0;
	for( ; ; ){
		if( 0L == (ulong)CRD_DAT.PAS.pno ){
			ret = 1;												// ���ԏꇂ�װ
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&					// ��{������g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PAS.pno )){ 				// ��{���ԏꇂ?
			CRD_DAT.PAS.typ = 0;											// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&				// �g��1������g�p��
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PAS.pno )){			// �g��1���ԏꇂ?
			CRD_DAT.PAS.typ = 1;											// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&				// �g��2������g�p��
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PAS.pno )){			// �g��2���ԏꇂ?
			CRD_DAT.PAS.typ = 2;											// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&				// �g��3������g�p��
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PAS.pno )){			// �g��3���ԏꇂ?
			CRD_DAT.PAS.typ = 3;											// �g��3
		}
		else{
			ret = 1;														// ���ԏꇂ�װ
			break;
		}

		if( prm_get( COM_PRM,S_KOU,4,1,1 ) ){						// �����o�^����(����ð��ð���)����?
			s_ptbl = ReadPassTbl( (ushort)CRD_DAT.PAS.pno, CRD_DAT.PAS.cod, us_psts );
			if( s_ptbl == -1 ){										// �װ(���l�ُ�)
				ret = 13;											// ��ʋK��O�װ
				break;
			}
			if( us_psts[0] ){										// �����o�^
				ret = 5;											// �����װ
				break;
			}
		}

		if( !rangechk( 1, 15, CRD_DAT.PAS.knd ) )					// �����ʔ͈͊O
		{
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		r_sts = (uchar)prm_get( COM_PRM,S_KOU,5,2,1 );
		if( rangechk( 0, 3, r_sts ) ){								// �ǎ�ð���ݒ肪0�`3�F��O�ð���Ȃ�
			if( !rangechk( 0, 3, CRD_DAT.PAS.sts ) ){				// �ð���͈͊O
				ret = 13;											// ��ʋK��O�װ
				break;
			}
		}else{														// �ǎ�ð���ݒ肪0�`3�ȊO�F��O����
			if( !rangechk( 0, 3, CRD_DAT.PAS.sts ) &&				// �ð���͈͊O
				( r_sts != CRD_DAT.PAS.sts ) )						// �ݒ肵����O�ð���ƈقȂ�
			{
				ret = 13;											// ��ʋK��O�װ
				break;
			}
		}

		if( CRD_DAT.PAS.std > CRD_DAT.PAS.end ){					// �L�������ް��ُ�(�J�n���I��)
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		if( prm_get( COM_PRM,S_KOU,3,2,5 ) ){						// �X�V���Ԃ̕��@
			// �w����������I�����{x��
			us_prm = (ushort)prm_get( COM_PRM,S_KOU,3,2,3 );		// �w���
			us_day = dnrmlzm( (short)CLK_REC.year,(short)CLK_REC.mont,(short)us_prm );	// �w�����ɰ�ײ��

			if( CLK_REC.ndat < CRD_DAT.PAS.end ){					// ���ݓ����L���I�������O

				// �X�V�����O����
				if( CLK_REC.year != e_ymd[0] ||						// ���ݔN���ƗL���I���N�����قȂ�
					CLK_REC.mont != e_ymd[1] ){
					ret = 50;										// �X�V���ԊO�װ
					break;
				}

				// �X�V�����O����
				if( CRD_DAT.PAS.end < us_day ){						// �w������I��������Ȃ�X�V�\
					ret = 50;										// �X�V���ԊO�װ
					break;
				}

				// �X�V�����O����
				if( CLK_REC.ndat < us_day ){						// �L���I��������xx���O�Ȃ�X�V�\
					ret = 50;										// �X�V���ԊO�װ
					break;
				}
			}
		}else{
			// ������I��������O��̎w�肵������
			us_prm = (ushort)prm_get( COM_PRM,S_KOU,3,2,3 );		// �X�V�\����(�O)
			if( CRD_DAT.PAS.end >= us_prm ){
				us_day = CRD_DAT.PAS.end - us_prm;
			}else{
				us_day = 0;
			}

			// �X�V�����O����
			if( CLK_REC.ndat < us_day ){							// �L���I��������xx���O�Ȃ�X�V�\
				ret = 50;											// �X�V���ԊO�װ
				break;
			}
		}

		if(( CRD_DAT.PAS.std > CLK_REC.ndat )||						// �����؂�(�J�n������)
		   ( CRD_DAT.PAS.end < CLK_REC.ndat )){						// �����؂�(�I��������)
			/*** �����؂�����t���Ȃ� ***/
			if( CRD_DAT.PAS.std > CLK_REC.ndat ){					// �����O
				ret = 6;											// �����O�װ
				break;
			}
			if( CRD_DAT.PAS.end < CLK_REC.ndat ){					// �����؂�
				// �X�V����������
				us_day = CRD_DAT.PAS.end + (ushort)prm_get( COM_PRM,S_KOU,3,2,1 );	// �X�V�\����(��) or �P�\����

				if( CLK_REC.ndat > us_day ){						// �L���I��������xx����Ȃ�X�V�\
					ret = 6;										// �X�V�����O�װ
					break;
				}
			}
		}

		if( !CPrmSS[S_KOU][7] ){									// �X�V���݂��g�p���Ȃ�
			us_prm = (ushort)CPrmSS[S_KOU][34+10*(CRD_DAT.PAS.knd-1)];	// �g�p�ړI�ݒ�Get
			if( us_prm == 0 ){										// �X�V�ݒ� ���ݒ�
				ret = 13;											// ��ʋK��O
				break;
			}
		}

		ryo_buf.pass_zero = 0;										// ���������0�~���Z�v���Ȃ�

		break;
	}

	if( ret == 0 ){													// ����OK?
		OPECTL.PassNearEnd = 0;
		vl_paschg();
		RenewalMonth = (ushort)CPrmSS[S_KOU][34+10*(CRD_DAT.PAS.knd-1)];	// ������X�V�������
		RenewalFee = (ulong)CPrmSS[S_KOU][30+10*(CRD_DAT.PAS.knd-1)];	// ������X�V���ྯ�
		PayData.teiki.syu = (uchar)CRD_DAT.PAS.knd;					// ��������
		PayData.teiki.status = (uchar)CRD_DAT.PAS.sts;				// ������ð��(�ǎ掞)
		PayData.teiki.id = CRD_DAT.PAS.cod;							// �����id
		PayData.teiki.pkno_syu = CRD_DAT.PAS.typ;					// ��������ԏ�m���D���
		PayData.teiki.update_mon = (uchar)RenewalMonth;				// �X�V����
		PayData.teiki.s_year = wksy;								// �L�������i�J�n�F�N�j
		PayData.teiki.s_mon = mag->aps_sta[1];						// �L�������i�J�n�F���j
		PayData.teiki.s_day = mag->aps_sta[2];						// �L�������i�J�n�F���j
		PayData.teiki.e_year = wkey;								// �L�������i�I���F�N�j
		PayData.teiki.e_mon = mag->aps_end[1];						// �L�������i�I���F���j
		PayData.teiki.e_day = mag->aps_end[2];						// �L�������i�I���F���j
		date_renewal( RenewalMonth, &PayData.teiki.e_year, &PayData.teiki.e_mon, &PayData.teiki.e_day );	// �L�������i�I�����j�X�V
		memcpy( PayData.teiki.t_tim, CRD_DAT.PAS.trz, 4 );			// ������������
		PayData.teiki.update_rslt1 = 0;								// ����X�V���Z���̍X�V����			�i�@OK�F�X�V�����@�^�@NG�F�X�V���s�@�j
		PayData.teiki.update_rslt2 = 0;								// ����X�V���Z���̃��x�����s����	�i�@OK�F���픭�s�@�^�@NG�F���s�s�ǁ@�j
	}
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| ����XX�����X�V����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : date_renewal()                                          |*/
/*| PARAMETER    : renw : �X�V���錎��                                     |*/
/*|                us_y : �X�V����N���߲��                                |*/
/*|                uc_m : �X�V���錎���߲��                                |*/
/*|                uc_d : �X�V��������߲��                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	date_renewal( ushort renw, ushort *us_y, uchar *uc_m, uchar *uc_d )
{
	ushort	us_wk;
	short	s_wky, s_wkm, s_wkd, s_wkd2;

	s_wky = (short)*us_y;
	s_wkm = (short)*uc_m;
	s_wkd = (short)*uc_d;


	if( (ushort)CPrmSS[S_KOU][2] == 0 ){							// ������X�V�v�Z���@
		// ������X�V�v�Z���@�F�L���I���� �{ �P�� �{ �X�V���� �| �P��
		us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );						// �L���I����ɰ�ײ��
		us_wk++;													// +1���X�V
		idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );					// �tɰ�ײ�ނ�1���X�V�������t�����߂�

		// �����X�V
		if( ( s_wkm + renw ) > 12 ){
			// �X�V��12���𒴂��Ă��܂��ꍇ�͔N���X�V����
			s_wkm = ( s_wkm + renw ) - 12;							// ���X�V
			s_wky++;												// �N�X�V
		}else{
			s_wkm += renw;											// ���X�V
		}

		s_wkd2 = medget( s_wky, s_wkm );							// ���̍ŏI������
		if( s_wkd2 < s_wkd ){
			// 1�����X�V�������t�����̌��ł��肦�Ȃ����t�i���̌��̍ŏI�����z���Ă���j�̎�
			// �X�V���͂��̌��̍ŏI���Ƃ���B
			s_wkd = s_wkd2;
		}else{
			// 1�����X�V�������t����������΁A�{���̌v�Z���i-1���j���p�����čs���A�X�V�����Z�o����
			us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );					// �����X�V�������̂��ēxɰ�ײ��
			us_wk--;												// -1���X�V

			idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );				// �tɰ�ײ�ނ�-1���X�V�������t�����߂�
		}
	}else{
		// ������X�V�v�Z���@�F�L���I���� �{ �X�V����

		// �����X�V
		if( ( s_wkm + renw ) > 12 ){
			// �X�V��12���𒴂��Ă��܂��ꍇ�͔N���X�V����
			s_wkm = ( s_wkm + renw ) - 12;							// ���X�V
			s_wky++;												// �N�X�V
		}else{
			s_wkm += renw;											// ���X�V
		}
		if( (ushort)CPrmSS[S_KOU][2] == 1 ){
			// ���܂������͗����֌J�z�ݒ�

			// ���ɰ�ײ�ނ��Ė߂����Ƃɂ�肠�܂������t��␳����
			us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );					// ɰ�ײ��
			idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );				// �tɰ�ײ��
		}else{
			// ���܂������͗����֌J�z���Ȃ��ݒ�

			// �X�V��̓����X�V��̌��̍ŏI���𒴂��Ă���ꍇ�͓������̍ŏI���ɕ␳����B
			s_wkd2 = medget( s_wky, s_wkm );						// ���̍ŏI������
			if( s_wkd > s_wkd2 ){
				s_wkd = s_wkd2;
			}
		}
	}

	*us_y = (ushort)s_wky;
	*uc_m = (uchar)s_wkm;
	*uc_d = (uchar)s_wkd;
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPrcKigenStr										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : "�L�������@�@�w�w�w�w�N�w�w���w�w��"�@������쐬        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mag(in) �v���y�C�h���f�[�^                              |*/
/*|				   PccUkoKigenStr(out) ������<NULL> 31�o�C�g�ȏ�̴ر������O�� |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Okamoto(COSMO)                                        |*/
/*| Date         : 2011-07-20                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	GetPrcKigenStr  ( char *PccUkoKigenStr )
{
	char	swork[10];

	memcpy ( PccUkoKigenStr, OPE_CHR[142], 31 );
	sprintf( swork, "%04d", CRD_DAT.PRE.kigen_year );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[8], 4 );	// "�L�������Q�O�P�P�N    ��    ��"
	sprintf( swork, "%2d", CRD_DAT.PRE.kigen_mon );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[18], 2 );	// "�L�������Q�O�P�P�N�@�V��    ��"
	sprintf( swork, "%2d", CRD_DAT.PRE.kigen_day );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[24], 2 );	// "�L�������Q�O�P�P�N�@�V���Q�O��"
}
//--------------------------------------------------------------------------------------------------------
// �v���y�C�h�J�[�h���C�f�[�^�̔̔����ɁA�L�����ԁi�������j�����Z���|�P�������N����(�L���������j
// ���Z�o���A�o�͂���.
//
// �Z�o��:�L�����Ԃ��P�����̏ꍇ
//
//�@�@�@�̔����F2011�N3��1��		�L���������F2011�N3��31��
//�@�@�@�̔����F2011�N3��15��		�L���������F2011�N4��14��
//�@�@�@�̔����F2011�N2��28��		�L���������F2011�N3��27��
//�@�@�@�̔����F2011�N1��31��		�L���������F2011�N2��28��
//�@�@�@�̔����F2012�N1��31��		�L���������F2012�N2��29��
//
// Param:
//  mag(in) : ذ�ގ��C�ް�
//  clk_Kigen(out) : �L��������
//
// Return: 0
//
//--------------------------------------------------------------------------------------------------------
int		al_preck_Kigen ( m_gtprepid* mag, struct clk_rec *clk_Kigen )
{
	struct	clk_rec		clk_date;
	unsigned short		us1;

	us1 = (unsigned short)prm_get( COM_PRM,S_PRP,11,2,1 );

	clk_date.year = astoin( &mag->prepid.pre_sta[0], 2 );
	clk_date.mont = (unsigned char)astoin( &mag->prepid.pre_sta[2], 2 );
	clk_date.date = (unsigned char)astoin( &mag->prepid.pre_sta[4], 2 );
	clk_date.hour = 0;
	clk_date.minu = 0;

	if ( clk_date.year >= 80 ){
		clk_date.year += 1900;
	}else{
		clk_date.year += 2000;
	}

	if ( us1 == 0 ){
		us1 = 6;			//6����.
	}
	add_month ( &clk_date, us1 );	//n�������Z -1��//

	memcpy ( clk_Kigen, &clk_date, sizeof(struct clk_rec));

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ����߲�޶���ذ��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_preck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = �ް��ُ�                                     |*/
/*|						  3 = �L�������؂�								   |*/
/*|                       7 = �c�z���                                      |*/
/*|                      10 = �\���װ                                      |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      26 = �ݒ�װ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_preck( m_gtprepid* mag )
{
	short	ret, wk;
	short	data[3];
	long temp;
	ushort	pkno_syu;
	struct	clk_rec		clk_wok;
	unsigned short		us1;

	if(mag->magformat.type == 1){
		temp  =  (long)mag->prepid.pre_pno[0] & 0x0000003F;
		temp |= ((long)mag->prepid.pre_pno[1] & 0x0000003F) << 6;
		temp |= ((long)mag->prepid.pre_pno[2] & 0x0000003F) << 12;
		CRD_DAT.PRE.pno = temp;											// ���ԏꇂ���
		CRD_DAT.PRE.GT_flg = 1;
	}else{
		wk = astoin( mag->prepid.pre_pno, 2 );
		wk += ( mag->prepid.pre_pno[2]&0x0f ) * 100;
		CRD_DAT.PRE.pno = (long)wk;										// ���ԏꇂ���
		CRD_DAT.PRE.GT_flg = 0;
	}
	CRD_DAT.PRE.mno = astoin( mag->prepid.pre_mno, 2 );					// �̔��@�����
	wk = (short)mag->prepid.pre_amo;
	if( wk < 0x40 ){												// "0"-"9" ?
		wk -= 0x31;
		wk += 1;
	}else if( wk < 0x5b ){											// "A"-"Z" ?
		wk -= 0x41;
		wk += 10;
	}else{															// "a"-"z"
		wk -= 0x61;
		wk += 36;
	}
	CRD_DAT.PRE.amo = wk * 1000L;									// �̔����z���
	CRD_DAT.PRE.ram = (long)astoinl( mag->prepid.pre_ram, 5 );		// �c�z���
	CRD_DAT.PRE.plm = PRC_GENDOGAKU_MIN;							// ���x�z���
	CRD_DAT.PRE.cno = (long)astoinl( mag->prepid.pre_cno, 5 );		// ���އ�
	if ( prm_get( COM_PRM,S_PRP,11,2,1 ) != 99 ){
		al_preck_Kigen ( (m_gtprepid*)mag, &clk_wok );		// clk_wok <-- �L��������
		CRD_DAT.PRE.kigen_year = clk_wok.year;
		CRD_DAT.PRE.kigen_mon  = clk_wok.mont;
		CRD_DAT.PRE.kigen_day  = clk_wok.date;
	}else{
		CRD_DAT.PRE.kigen_year = 0;
		CRD_DAT.PRE.kigen_mon  = 0;
		CRD_DAT.PRE.kigen_day  = 0;
	}
	ret = 0;
	for( ; ; ){
		if( 0L == (ulong)CRD_DAT.PRE.pno ){
			ret = 1;												// ���ԏꇂ�װ
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 72, 1, 6 ) == 1 )&&					// ��{�v���y�C�h�g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PRE.pno )){ 				// ��{���ԏꇂ?
			pkno_syu = KIHON_PKNO;									// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 1 ) == 1 )&&				// �g��1�v���y�C�h�g�p��
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PRE.pno )){			// �g��1���ԏꇂ?
			pkno_syu = KAKUCHOU_1;									// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 2 ) == 1 )&&				// �g��2�v���y�C�h�g�p��
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PRE.pno )){			// �g��2���ԏꇂ?
			pkno_syu = KAKUCHOU_2;									// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 3 ) == 1 )&&				// �g��3�v���y�C�h�g�p��
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PRE.pno )){			// �g��3���ԏꇂ?
			pkno_syu = KAKUCHOU_3;									// �g��3
		}
		else{
			ret = 1;														// ���ԏꇂ�װ
			break;
		}
		
		if( Ope_GT_Settei_Check( CRD_DAT.PRE.GT_flg, pkno_syu ) == 1 ){		// �ݒ�NG����
			ret = 33;
			break;
		}

		if( CPrmSS[S_PRP][1] != 1 ){								// ����߲�ގg�p�s�ݒ�
			ret = 26;												// �ݒ�װ
			break;
		}
		
		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.PRE.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.PRE.pno) || (CRD_DAT.PRE.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.PRE.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.PRE.pno) || (CRD_DAT.PRE.pno > APSF_PKNO_UPPER))) ){			//���ԏ�No�͈̓`�F�b�N
				ret = 1;												// ���ԏꇂ�װ
				break;
			}
		}

		if( CRD_DAT.PRE.ram == 0L ){
			ret = 7;												// �c�z���
			break;
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, 15, 0, 0, 0 ) ){			// 1���Z�̊�����ނ̌������ް?
				ret = 14;											// ���x�������ް
				break;
			}
		}

		if( RD_pos > 1 ){											// �\���װ?
			ret = 10;												// �\���װ
			break;
		}

		data[0] = astoin( &mag->prepid.pre_sta[0], 2 );				// �̔��N
		if( data[0] >= 80 ){
			data[0] += 1900;
		}else{
			data[0] += 2000;
		}
		data[1] = astoin( &mag->prepid.pre_sta[2], 2 );				// �̔���
		data[2] = astoin( &mag->prepid.pre_sta[4], 2 );				// �̔���
		if( chkdate( data[0], data[1], data[2] ) ){					// �̔�������NG?
			ret = 2;												// �ް��ُ�
			break;
		}
		if( dnrmlzm( data[0], data[1], data[2] ) > CLK_REC.ndat ){	// �̔��� > ���ݓ�
			ret = 13;												// ��ʋK��O
			break;
		}
		vl_now = V_PRI;												// ����߲�޶���
		break;
	}
	if ( (ret == 0) &&
		 (prm_get( COM_PRM,S_PRP,11,2,1 ) != 99) ){

		us1 = dnrmlzm( (short)CRD_DAT.PRE.kigen_year, (short)CRD_DAT.PRE.kigen_mon, (short)CRD_DAT.PRE.kigen_day );
		if ( us1 < CLK_REC.ndat ){
			ret = 3;
		}
	}
	if( ret == 0 )
	{
		(void)vl_prechg( (m_gtprepid*)mag );
		card_use[USE_PPC] += 1;										// ����߲�޶��ޖ���+1
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �񐔌�ذ�ށ��`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_kasck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = ��ʴװ                                      |*/
/*|                       3 = �����؂�                                     |*/
/*|                       7 = �c�z���                                      |*/
/*|                      10 = �\���װ                                      |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      14 = ���x�������ް                                |*/
/*|                      26 = �ݒ�װ                                      |*/
/*|						100 = �����؂�ԋ߁i3���ȓ��j					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_kasck( m_gtservic *mag )
{
	short	ret, wk;
	ushort	pkno_syu;

	CRD_DAT.SVS.pno = 												// ���ԏꇂ���
		(long)mag->servic.svc_pno[0] + (long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GT�t�H�[�}�b�g
		CRD_DAT.SVS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17�i�[
		CRD_DAT.SVS.GT_flg = 1;													//GT�t�H�[�}�b�g�t���O�Z�b�g
	}else{
		CRD_DAT.SVS.GT_flg = 0;													//GT�t�H�[�}�b�g�t���O�Z�b�g	
	}
	CRD_DAT.SVS.knd = (short)( mag->servic.svc_pno[1] & 0x0f );			// �����x�����
	CRD_DAT.SVS.cod =												// �P�ʋ��z���
		(short)mag->servic.svc_sno[1] + (((short)mag->servic.svc_sno[0])<<7);
	CRD_DAT.SVS.sts = (short)( mag->servic.svc_sts );						// �c���

	if( memcmp( mag->servic.svc_sta, ck_dat, 6 ) == 0 ){					// ������
		CRD_DAT.SVS.std = 0;										// �L���J�n�����
		CRD_DAT.SVS.end = 0xffff;									// �L���I�������
	}else{
		wk = (short)(mag->servic.svc_sta[0]);
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_sta[1], (short)mag->servic.svc_sta[2] ) ){	// �L���J�n������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.SVS.std = 											// �L���J�n�����
			dnrmlzm( wk,(short)mag->servic.svc_sta[1],(short)mag->servic.svc_sta[2] );

		wk = (short)mag->servic.svc_end[0];
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_end[1], (short)mag->servic.svc_end[2] ) ){	// �L���I��������NG?
			return( 2 );											// �ް��ُ�
		}
		CRD_DAT.SVS.end = 											// �L���I�������
			dnrmlzm( wk,(short)mag->servic.svc_end[1],(short)mag->servic.svc_end[2] );
	}

	/* TF-7700�̃`�F�b�N���� */
	/* 	�����ް�����														*/
	//		if( ! rangechk(1,11,vl_kas.rim ) ||			/* �����x���͈͊O �@*/
	//			! rangechk(1,11 ,vl_kas.nno) ||			/* �c��1-11�ȊO   */
	//			! rangechk(0,9990,vl_kas.tnk) ) 		/* �P�ʋ��z�͈͊O �@*/
	/* 	���ԏ�NO.����														*/
	/* 	�s��������	(�񐔌��ݒ�L��)										*/
	/* 	�}����������														*/
	/* 	�����؂�����														*/
	/* 	�����؂�ԋ�����													*/

	ret = 0;
	for( ; ; ){

		/* ���ԏ�ԍ����� */
		if( 0L               == (ulong)CRD_DAT.SVS.pno){
			ret = 1;												// ���ԏꇂ�װ
			break;
		}
		if(( prm_get( COM_PRM, S_SYS, 72, 1, 6 ) == 1 )&&			// ��{�񐔌����g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.SVS.pno )){ 		// ��{���ԏꇂ?
			pkno_syu = KIHON_PKNO;									// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 1 ) == 1 )&&		// �g��1�񐔌����g�p��
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.SVS.pno )){	// �g��1���ԏꇂ?
			pkno_syu = KAKUCHOU_1;									// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 2 ) == 1 )&&		// �g��2�񐔌����g�p��
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.SVS.pno )){	// �g��2���ԏꇂ?
			pkno_syu = KAKUCHOU_2;									// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 3 ) == 1 )&&		// �g��3�񐔌����g�p��
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.SVS.pno )){	// �g��3���ԏꇂ?
			pkno_syu = KAKUCHOU_3;									// �g��3
		}
		else{
			ret = 1;												// ���ԏꇂ�װ
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.SVS.GT_flg, pkno_syu ) == 1 ){	// �ݒ�NG����
			ret = 33;
			break;
		}

		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.SVS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.SVS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > APSF_PKNO_UPPER))) ){//���ԏ�No�͈̓`�F�b�N
				ret = 1;												// ���ԏꇂ�װ
				break;
			}
		}

		/* �s�������� (�񐔌��ݒ�L��) */
		if( CPrmSS[S_PRP][1] != 2 ){								// �񐔌��g�p�s�ݒ�
			ret = 26;												// �ݒ�װ
			break;
		}

		/* �}���������� */
		if( RD_pos > 1 ){											// �\���װ?
			ret = 10;												// �\���װ
			break;
		}

		/* �����؂����� */
			if( CRD_DAT.SVS.std > CLK_REC.ndat ){					// �����O
				ret = 6;
				break;
			}
			if( CRD_DAT.SVS.end < CLK_REC.ndat ){					// �����؂�
				ret = 3;
				break;
			}

		/* �c�� */
		if( CRD_DAT.SVS.sts == 0 ){									// 
			ret = 7;												// �c�z���
			break;
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, 13, CRD_DAT.SVS.knd, CRD_DAT.SVS.cod, CRD_DAT.SVS.sts ) ){	// 1���Z�̊�����ނ̌������ް?
				ret = 14;											// ���x�������ް
				break;
			}
		}

		/* �����ް����� */
		if( ! rangechk( 1, 11, CRD_DAT.SVS.knd ) ||					// �����x���͈͊O
			! rangechk( 1, 11, CRD_DAT.SVS.sts ) ||					// �c��1-11�ȊO
			! rangechk( 0, 9990, CRD_DAT.SVS.cod) ){ 				// �P�ʋ��z�͈͊O
			ret = 13;												// ��ʋK��O
		}

		/* �����v�Z�v�� ���� */
		if( !CRD_DAT.SVS.cod )
			vl_now = V_KAM;											// �񐔌��i�����j
		else
			vl_now = V_KAG;											// �񐔌��i�����j
		break;
	}
	if( ret == 0 )
	{
		ret = vl_kaschg( (m_gtservic*)mag );							// �����v�Z�p �񐔌��ް����
		card_use[USE_NUM] += 1;										// �񐔌�����+1
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ײ��ް��쐬                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpas( inpp, ck )                                    |*/
/*| PARAMETER    : m_apspas *inpp : Read Pass Card Data                    |*/
/*|                clk_rec  *ck   : Dispence Time                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpas( m_gtapspas *inpp, struct clk_rec *ck )
{
	m_apspas *outp = (m_apspas*)MDP_buf;

	memcpy( outp, &inpp->apspas.aps_idc, sizeof( m_apspas ) );
	outp->aps_sts = 1;												// �o�ɒ�
	outp->aps_wrt[0] = ck->mont;
	outp->aps_wrt[1] = ck->date;
	outp->aps_wrt[2] = ck->hour;
	outp->aps_wrt[3] = ck->minu;
	MDP_siz = sizeof( m_apspas );
	MDP_mag = sizeof( m_apspas );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[0], (ushort)MDP_siz, 0 );				// Odd Parity Set
		MDP_buf[127] = 0;		//GT�t�H�[�}�b�g�쐬�t���O
	}else{
		outp->aps_pno[0] |= (uchar)((inpp->magformat.ex_pkno & 0x01) << 7);
		outp->aps_pno[1] |= (uchar)((inpp->magformat.ex_pkno & 0x02) << 6);
		outp->aps_pcd[0] |= (uchar)((inpp->magformat.ex_pkno & 0x04) << 5);
		outp->aps_pcd[1] |= (uchar)((inpp->magformat.ex_pkno & 0x08) << 4);
		outp->aps_sta[0] |= (uchar)((inpp->magformat.ex_pkno & 0x10) << 3);
		outp->aps_sta[1] |= (uchar)((inpp->magformat.ex_pkno & 0x20) << 2);
		outp->aps_sta[2] |= (uchar)((inpp->magformat.ex_pkno & 0x40) << 1);
		outp->aps_end[0] |= (uchar)(inpp->magformat.ex_pkno & 0x80);
		md_pari2( &(uchar)MDP_buf[0], (ushort)1, 1 );					// ID�R�[�hEven Parity Set
		MDP_buf[127] = 1;		//GT�t�H�[�}�b�g�쐬�t���O
	}
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| �����ײ��ް��쐬(������X�V�p)                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpas_renewal( inpp, ck )                            |*/
/*| PARAMETER    : m_apspas *inpp : Read Pass Card Data                    |*/
/*|                clk_rec  *ck   : Dispence Time                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpas_renewal( m_apspas *inpp, struct clk_rec *ck )
{
uchar	r_sts;

	m_apspas *outp = (m_apspas*)MDP_buf;

	memcpy( outp, inpp, sizeof( m_apspas ) );

	outp->aps_end[0] = (uchar)(PayData.teiki.e_year % 100);			// �L�������i�I���F�N�j
	outp->aps_end[1] = PayData.teiki.e_mon;							// �L�������i�I���F���j
	outp->aps_end[2] = PayData.teiki.e_day;							// �L�������i�I���F���j

//	outp->aps_sts = 0;												// ����

	if( rangechk( 0, 3, outp->aps_sts ) ){							// ������ǎ掞�̽ð����0�`3�͈͓̔�
		if( !prm_get( COM_PRM,S_KOU,6,2,5 ) ){						// �S�ď���ð���ɏ�������
			outp->aps_sts = 0;										// ����
		}
		// ����ȊO�͓ǎ掞�̒�����ð���̂܂�
	}else{															// ������ǎ掞�̽ð����0�`3�ȊO
		r_sts = (uchar)prm_get( COM_PRM,S_KOU,5,2,1 );
		if( rangechk( 0, 3, r_sts ) ){								// �ǎ�ð���ݒ肪0�`3�F��O�ð���Ȃ�
			r_sts = 0;												// �����ð���ݒ�ł̏����Ȃ��Ƃ���
		}
		if( r_sts && ( r_sts == outp->aps_sts ) ){					// ��O����ŗ�O�ð����������ǎ掞
			outp->aps_sts = (uchar)prm_get( COM_PRM,S_KOU,6,2,1 );	// ��O�ð���̎��̏����ð���ɕύX����
		}else{
			outp->aps_sts = 0;										// ����
		}
	}

	outp->aps_wrt[0] = ck->mont;
	outp->aps_wrt[1] = ck->date;
	outp->aps_wrt[2] = ck->hour;
	outp->aps_wrt[3] = ck->minu;
	MDP_siz = sizeof( m_apspas );
	md_pari( &(uchar)MDP_buf[0], (ushort)MDP_siz, 0 );				// Odd Parity Set
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ����߲��ײ��ް��쐬                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpre( inpp, pre )                                   |*/
/*| PARAMETER    : m_prepid *inpp : ����ذ���ް�                           |*/
/*|                pre_rec  *pre  : ����߲�޶���ײē��e                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpre( m_gtprepid *inpp, pre_rec *pre )
{
	short	i;
	long	set;
	unsigned long	setzan, zenzan;
	m_prepid	*tic = ( m_prepid*)&MDP_buf[31];

	memset( MDP_buf, 0x20, 30 );
	zenzan = (long)astoinl( inpp->prepid.pre_ram, 5 );						// �O��c�z���
	for( i=7; i>0; i-- ){
		setzan = (unsigned long)( CPrmSS[S_PRP][2-1+i] );
		if( pre->ram < setzan ){									// �c���z < �ݒ���z
			if( zenzan >= setzan ){									// �O�c�z >= �ݒ���z
				MDP_buf[i*2+5] = 0x2a;								// �c�zϰ�'*'���
			}
		}
	}
	if(( pre->mno < 50 )&&( CPrmSS[S_PRP][9] )){					// ���� & ����󎚂���?
		pre->mno += 50;
		for( i=7; i>0; i-- ){
			if(( set = CPrmSS[S_PRP][2-1+i] ) == 0 ){				// �ݒ��?
				continue;											// No..continue
			}
			if( set <= pre->amo ){
				MDP_buf[i*2+7] = 0x2a;								// ����ϰ�'*'���
				break;
			}
		}
	}
	if( pre->ram == 0 ){
		MDP_buf[5] = 0x2a;											// �c�z0�~
	}

	MDP_buf[30] = 0x09;												// ��؂�ϰ�
	memcpy( tic, &inpp->prepid.pre_idc, sizeof( m_prepid ) );
	intoas( tic->pre_mno, (unsigned short)pre->mno, 2 );			// �̔��@�����
	intoasl( tic->pre_ram, (unsigned long)pre->ram, 5 );			// �c�z���
	MDP_siz = 31 + sizeof( m_prepid );
	MDP_mag = sizeof( m_prepid );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[31], sizeof( m_prepid ), 1 );			// Even Parity Set
		MDP_buf[127] = 0;		//GT�t�H�[�}�b�g�쐬�t���O
	}else{
		md_pari2( &(uchar)MDP_buf[31], sizeof( m_prepid ), 1 );			// Even Parity Set
		MDP_buf[127] = 1;		//GT�t�H�[�}�b�g�쐬�t���O
	}
	RD_PrcWriteFlag = 1;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �񐔌�ײ��ް��쐬                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkkas( inpp, pre )                                   |*/
/*| PARAMETER    : m_prepid *inpp : ����ذ���ް�                           |*/
/*|                pre_rec  *pre  : �񐔌�ײē��e    		               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-10-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkkas( m_gtkaisuutik *inpp, svs_rec *pre, struct clk_rec *ck )
{
	short	i, j;
	uchar	BeforeDosuu, AfterDosuu;
	m_kaisuutik	*tic = ( m_kaisuutik*)&MDP_buf[31];

	memset( MDP_buf, 0x20, 30 );

	/* ���ʈ󎚍쐬 */
		BeforeDosuu = (uchar)CRD_DAT.SVS.sts;
		AfterDosuu  = KaisuuAfterDosuu;

		j = (short)(BeforeDosuu - AfterDosuu);						// ���񗘗p��
		if ( j >= 2 ){												// 2��ȏ�
			for ( i = 0 ; i < (j-1) ; i ++ ){
				BeforeDosuu --;		  								// ���p���޸����
				MDP_buf[BeforeDosuu*2+5] = 0x2a;					// '��' �ް�
			}
		}
		MDP_buf[AfterDosuu*2+5] = 0x2a;								// '��' �ް�

	/* ���C�ް���� */
	MDP_buf[30] = 0x09;												// ��؂�ϰ�
	memcpy( tic, &inpp->kaisuutik.kaitik_idc, sizeof( m_kaisuutik ) );

	tic->kaitik_kai = (uchar)KaisuuAfterDosuu;						// �c�񐔾��

	tic->kaitik_wrt[0] = ck->mont;									// �����N�������
	tic->kaitik_wrt[1] = ck->date;
	tic->kaitik_wrt[2] = ck->hour;
	tic->kaitik_wrt[3] = ck->minu;

	MDP_siz = 31 + sizeof( m_kaisuutik );
	MDP_mag = sizeof( m_kaisuutik );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[31], sizeof( m_kaisuutik ), 0 );		// Odd Parity Set
		MDP_buf[127] = 0;		//GT�t�H�[�}�b�g�쐬�t���O
	}else{
		tic->kaitik_pno[0] |= (uchar)((inpp->magformat.ex_pkno & 0x01) << 7);
		tic->kaitik_pno[1] |= (uchar)((inpp->magformat.ex_pkno & 0x02) << 6);
		tic->kaitik_tan[0] |= (uchar)((inpp->magformat.ex_pkno & 0x04) << 5);
		tic->kaitik_tan[1] |= (uchar)((inpp->magformat.ex_pkno & 0x08) << 4);
		tic->kaitik_sta[0] |= (uchar)((inpp->magformat.ex_pkno & 0x10) << 3);
		tic->kaitik_sta[1] |= (uchar)((inpp->magformat.ex_pkno & 0x20) << 2);
		tic->kaitik_sta[2] |= (uchar)((inpp->magformat.ex_pkno & 0x40) << 1);
		tic->kaitik_end[0] |= (uchar)(inpp->magformat.ex_pkno & 0x80);
		md_pari2( &(uchar)MDP_buf[31], (ushort)1, 1 );					// ID�R�[�hEven Parity Set
		MDP_buf[127] = 1;		//GT�t�H�[�}�b�g�쐬�t���O
	}
	return;
}


// ��ID�ް�ð���
const	uchar	tik_id_tbl[TIK_SYU_MAX+1][ID_SYU_MAX] =
	{
//		ID1		ID2		ID3		ID4		ID5		ID6
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	// ����O�O�F�_�~�[
		0x21,	0x45,	0x50,	0x00,	0x00,	0x06,	// ����O�P�F���Ԍ��i�`�q�|�����j
		0x22,	0x46,	0x54,	0x00,	0x00,	0x20,	// ����O�Q�F���Ԍ��i�`�q-�T�����~���j
		0x23,	0x47,	0x56,	0x00,	0x00,	0x2E,	// ����O�R�F���Ԍ��i�`�q-�o�񒆎~���j
		0x24,	0x48,	0x57,	0x00,	0x00,	0x30,	// ����O�S�F���Ԍ��i���Z�O�j
		0x25,	0x49,	0x58,	0x00,	0x00,	0x39,	// ����O�T�F���Ԍ��i���Z��j
		0x26,	0x4A,	0x59,	0x00,	0x00,	0x3C,	// ����O�U�F���Ԍ��i���~���j
		0x27,	0x4B,	0x5A,	0x00,	0x00,	0x3D,	// ����O�V�F���Ԍ��i�|�����j
		0x28,	0x4C,	0x5B,	0x00,	0x00,	0x3E,	// ����O�W�F���Ԍ��i�Đ��Z���~���j
		0x29,	0x4D,	0x5C,	0x00,	0x00,	0x3F,	// ����O�X�F���Ԍ��i���Z�ς݌��j
		0x2A,	0x4E,	0x5D,	0x00,	0x00,	0x00,	// ����P�O�F�Ĕ��s��
		0x2B,	0x4F,	0x5F,	0x00,	0x00,	0x43,	// ����P�P�F������
		0x1A,	0x64,	0x6A,	0x70,	0x76,	0x44,	// ����P�Q�F�`�o�r�����
		0x0E,	0x1B,	0x1C,	0x1D,	0x1E,	0x1F,	// ����P�R�F�v���y�C�h�J�[�h
		0x2C,	0x65,	0x6B,	0x71,	0x77,	0x62,	// ����P�S�F�񐔌�
		0x2D,	0x66,	0x6C,	0x72,	0x78,	0x63,	// ����P�T�F�T�[�r�X���i�|�����E�������j
		0x41,	0x69,	0x6F,	0x75,	0x7B,	0x7E,	// ����P�U�F�W���J�[�h

	};

// ���했�g�pID�ݒ���ڽð��قP�i���했�̐ݒ��ް����ڽ�j
const	uchar	id_para_tbl1[TIK_SYU_MAX+1][4] =
	{
//		��{	�g���P	�g���Q	�g���R
		0,		0,		0,		0,					// ����O�O�F�_�~�[
		10,		0,		0,		0,					// ����O�P�F���Ԍ��i�`�q�|�����j
		10,		0,		0,		0,					// ����O�Q�F���Ԍ��i�`�q-�T�����~���j
		10,		0,		0,		0,					// ����O�R�F���Ԍ��i�`�q-�o�񒆎~���j
		10,		0,		0,		0,					// ����O�S�F���Ԍ��i���Z�O�j
		10,		0,		0,		0,					// ����O�T�F���Ԍ��i���Z��j
		10,		0,		0,		0,					// ����O�U�F���Ԍ��i���~���j
		10,		0,		0,		0,					// ����O�V�F���Ԍ��i�|�����j
		10,		0,		0,		0,					// ����O�W�F���Ԍ��i�Đ��Z���~���j
		10,		0,		0,		0,					// ����O�X�F���Ԍ��i���Z�ς݌��j
		10,		0,		0,		0,					// ����P�O�F�Ĕ��s��
		10,		0,		0,		0,					// ����P�P�F������
		11,		12,		13,		14,					// ����P�Q�F�`�o�r�����
		11,		12,		13,		14,					// ����P�R�F�v���y�C�h�J�[�h
		11,		12,		13,		14,					// ����P�S�F�񐔌�
		11,		12,		13,		14,					// ����P�T�F�T�[�r�X���i�|�����E�������j
		10,		0,		0,		0,					// ����P�U�F�W���J�[�h
	};

// ���했�g�pID�ݒ���ڽð��قQ�i���했�̐ݒ��ް��Ǐo���ʒu�j
const	char	id_para_tbl2[TIK_SYU_MAX+1] =
	{
		0,			// ����O�O�F�_�~�[
		1,			// ����O�P�F���Ԍ��i�`�q�|�����j
		1,			// ����O�Q�F���Ԍ��i�`�q-�T�����~���j
		1,			// ����O�R�F���Ԍ��i�`�q-�o�񒆎~���j
		1,			// ����O�S�F���Ԍ��i���Z�O�j
		1,			// ����O�T�F���Ԍ��i���Z��j
		1,			// ����O�U�F���Ԍ��i���~���j
		1,			// ����O�V�F���Ԍ��i�|�����j
		1,			// ����O�W�F���Ԍ��i�Đ��Z���~���j
		1,			// ����O�X�F���Ԍ��i���Z�ς݌��j
		1,			// ����P�O�F�Ĕ��s��
		1,			// ����P�P�F������
		1,			// ����P�Q�F�`�o�r�����
		3,			// ����P�R�F�v���y�C�h�J�[�h
		4,			// ����P�S�F�񐔌�
		2,			// ����P�T�F�T�[�r�X���i�|�����E�������j
		1,			// ����P�U�F�W���J�[�h
	};

/*[]---------------------------------------------------------------------------[]*/
/*|		���ް�������ID�ϊ�����													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Read_Tik_Chk( *tik_data, *pk_syu )						|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	���ް��߲��						|*/
/*|																				|*/
/*|						uchar	*pk_syu		=	���ԏ����ް��i�[�߲��			|*/
/*|													1=��{						|*/
/*|													2=�g��1						|*/
/*|													3=�g��2						|*/
/*|													4=�g��3						|*/
/*|																				|*/
/*|						���V����ID���g�p����ݒ���������ʂ��n�j�̏ꍇ�̂ݗL��	|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret = ��������									|*/
/*|										OK �F���ް��L��							|*/
/*|										NG �F���ް�����							|*/
/*|										 2 : �V����ID�g�p���Ȃ� (OK=0/NG=0xFF)	|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	Read_Tik_Chk( uchar *tik_data, uchar *pk_syu, uchar tik_data_length )
{
	uchar	ret = OK;			// �߂�l�i�������ʁj
	uchar	tik_id;				// ���h�c
	uchar	tik_syu;			// ����
	uchar	id_syu;				// ID��
	uchar	cmp_end;			// ID��r����
	ushort	pk_no;				// ���ԏ�No.
	uchar	pk_syu_wk;			// ���ԏ��ʁiܰ��j
	uchar	ng_pkno	= OFF;		// �m�f���ԏ�m���D����׸�
	uchar	idchg	= OFF;		// ID1�ϊ��׸�
	uchar	id_syu_settei;		// �g�p��ID

	if( prm_get( COM_PRM,S_PAY,10,1,4 ) != 0 ){						// �V�J�[�h�h�c�g�p����H

		// �V�J�[�h�h�c���g�p����ꍇ

		// ���ް���ID���猔�����������
		tik_id = tik_data[0];										// ��ID�擾
		cmp_end = OFF;
		for( tik_syu = 1 ; tik_syu <= TIK_SYU_MAX ; tik_syu++ ){	// ���했��ID��rٰ��

			for( id_syu = 0 ; id_syu < ID_SYU_MAX ; id_syu++ ){		// ID��(�P�`�T)��ID��rٰ��

				if( tik_id == tik_id_tbl[tik_syu][id_syu] ){		// ID��v�H
					cmp_end = ON;									// YES
					break;
				}
			}
			if( cmp_end == ON ){									// ID��v�H
				break;												// YES ->
			}
		}
		// �g��ID�ȊO�̃J�[�h�ł���΂͂���
		if( cmp_end == ON ){
			if( tik_data_length > 60 ){								// 60byte��蒷���f�[�^�͊g��ID�Ή��J�[�h�ɂ͖���
																	// �Ή��J�[�h�̍Œ���29byte�����A���C���[�_�[��
																	// �N���W�b�g�J�[�h����Ɠ��������Ŕ�r����
				cmp_end = OFF;
				if( tik_data[10] == 0x53 ){							// �f�[�^ID��53H
					if( (MAGred[MAG_GT_APS_TYPE] == 0) &&			// APS�t�H�[�}�b�g��PAK
						((tik_data[11] == 0x50) && (tik_data[12] == 0x41) && (tik_data[13] == 0x4b)) ){
						cmp_end = ON;								// �A�}�m�W���J�[�h
					}
					if( (MAGred[MAG_GT_APS_TYPE] == 1) &&			// GT�t�H�[�}�b�g��PGT
						((tik_data[11] == 0x50) && (tik_data[12] == 0x47) && (tik_data[13] == 0x54)) ){
						cmp_end = ON;								// �A�}�m�W���J�[�h
					}
				}
			}
		}
		if( cmp_end == ON ){

			// ���h�c���Ή��͈͓��̏ꍇ

			pk_no = PkNo_get( tik_data, tik_syu  );					// ���ް����璓�ԏ�No.�擾

			// ���ԏ��ʌ���
			if( CPrmSS[S_SYS][1] == (ulong)pk_no ){					// ��{�H
				pk_syu_wk = 1;
			}
			else if( CPrmSS[S_SYS][2] == (ulong)pk_no ){			// �g���P�H
				pk_syu_wk = 2;
			}
			else if( CPrmSS[S_SYS][3] == (ulong)pk_no ){			// �g���Q�H
				pk_syu_wk = 3;
			}
			else if( CPrmSS[S_SYS][4] == (ulong)pk_no ){			// �g���R�H
				pk_syu_wk = 4;
			}
			else{													// ���ԏ�No.�s��v
				pk_syu_wk = 0;
			}

			// ���했�Ɏg�p����ID�������
			switch( pk_syu_wk ){									// ���ԏ��ʁH

				case	1:											// ���ԏ��ʁ���{

					id_syu_settei = (uchar)prm_get(					// �g�p��ID��ݒ����Ұ��擾
													COM_PRM,
													S_PAY,
													(short)id_para_tbl1[tik_syu][pk_syu_wk-1],
													1,
													id_para_tbl2[tik_syu]
												);
					break;

				case	2:											// ���ԏ��ʁ��g���P
				case	3:											// ���ԏ��ʁ��g���Q
				case	4:											// ���ԏ��ʁ��g���R

					if(
						tik_syu == 1	||							// ����ʁ����Ԍ��i�`�q�|�����j
						tik_syu == 2	||							// ����ʁ����Ԍ��i�`�q-�T�����~���j
						tik_syu == 3	||							// ����ʁ����Ԍ��i�`�q-�o�񒆎~���j
						tik_syu == 4	||							// ����ʁ����Ԍ��i���Z�O�j
						tik_syu == 5	||							// ����ʁ����Ԍ��i���Z��j
						tik_syu == 6	||							// ����ʁ����Ԍ��i���~���j
						tik_syu == 7	||							// ����ʁ����Ԍ��i�|�����j
						tik_syu == 8	||							// ����ʁ����Ԍ��i�Đ��Z���~���j
						tik_syu == 9	||							// ����ʁ����Ԍ��i���Z�ς݌��j
						tik_syu == 10	||							// ����ʁ��Ĕ��s��
						tik_syu == 11	||							// ����ʁ�������
						tik_syu == 16								// ����ʁ��W���J�[�h
					){
						// ����ʂ����Ԍ��܂��͌W���J�[�h�̏ꍇ�i�g�����ԏ�No.�͂m�f�j
						ret		= NG;
						ng_pkno	= ON;
						idchg	= ON;
					}
					else{
						// ����ʂ����Ԍ��܂��͌W���J�[�h�ȊO�̏ꍇ
						id_syu_settei = (uchar)prm_get(				// �g�p��ID��ݒ����Ұ��擾
														COM_PRM,
														S_PAY,
														(short)id_para_tbl1[tik_syu][pk_syu_wk-1],
														1,
														id_para_tbl2[tik_syu]
													);
					}
					break;

				default:											// ���ԏ��ʁ����ԏ�m���D�s��v
					ret		= NG;
					ng_pkno	= ON;
					idchg	= ON;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}

			if( ret == OK ){

				// ���ԏ�m���D���n�j�̏ꍇ

				switch( id_syu_settei ){							// �g�p��ID��ݒ�H

					case	1:										// ID1
					case	2:										// ID2
					case	3:										// ID3
					case	4:										// ID4
					case	5:										// ID5

						if( tik_id != tik_id_tbl[tik_syu][id_syu_settei-1] ){
							// �g�p��ID���ID�ƕs��v
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	6:										// ID1��ID2

						if( ( tik_id != tik_id_tbl[tik_syu][0] ) && ( tik_id != tik_id_tbl[tik_syu][1] ) ){
							// �g�p��ID���ID�ƕs��v
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	7:										// ID1��ID3

						if( ( tik_id != tik_id_tbl[tik_syu][0] ) && ( tik_id != tik_id_tbl[tik_syu][2] ) ){
							// �g�p��ID���ID�ƕs��v
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	8:										// ID6
						if( tik_id != tik_id_tbl[tik_syu][5] ){
							// �g�p��ID���ID�ƕs��v
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;
					case	0:										// �ǂ܂Ȃ�
					default:										// ���̑��i�ݒ�G���[�j
						ret		= NG;
						ng_pkno	= ON;
						idchg	= ON;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				}

				if( ret == OK ){

					// �g�p��ID���ID�ƌ��ް���ID����v

					tik_data[0]	= tik_id_tbl[tik_syu][0];			// ���ް���ID��ID1�i�]���^�j��ID�ɕύX
					*pk_syu		= pk_syu_wk;						// ���ԏ��ʁi�o�����Ұ��j���

					switch( tik_syu ){								// ���했�̍ŏI�ǎ挔ID���ۑ�

						case	1:									// ���Ԍ��i�`�q�|�����j
						case	2:									// ���Ԍ��i�`�q-�T�����~���j
						case	3:									// ���Ԍ��i�`�q-�o�񒆎~���j
						case	4:									// ���Ԍ��i���Z�O�j
						case	5:									// ���Ԍ��i���Z��j
						case	6:									// ���Ԍ��i���~���j
						case	7:									// ���Ԍ��i�|�����j
						case	8:									// ���Ԍ��i�Đ��Z���~���j
						case	9:									// ���Ԍ��i���Z�ς݌��j
						case	10:									// �Ĕ��s��
						case	11:									// ������

							ReadIdSyu.pk_tik = (uchar)(id_syu+1);
							break;

						case	12:									// �`�o�r�����

							ReadIdSyu.teiki = (uchar)(id_syu+1);
							break;

						case	13:									// �v���y�C�h�J�[�h

							ReadIdSyu.pripay = (uchar)(id_syu+1);
							break;

						case	14:									// �񐔌�

							ReadIdSyu.kaisuu = (uchar)(id_syu+1);
							break;

						case	15:									// �T�[�r�X���i�|�����E�������j

							ReadIdSyu.svs_tik = (uchar)(id_syu+1);
							break;

						case	16:									// �W���J�[�h

							ReadIdSyu.kakari = (uchar)(id_syu+1);
							break;
					}
				}
			}
		}
		else{
			// ���h�c���Ή��͈͊O�i��ID�ް�ð��قɑ��݂��Ȃ��j
			ret = NG;
		}
	}
	else{
		ret = 2;
	}

	if( ret == NG ){

		// ���ް��������ȏꍇ

		if( ng_pkno == ON ){
			NgPkNo_set( tik_data, tik_syu );						// �K��O�̒��ԏ�No.������
		}
		if( idchg == ON ){
			tik_data[0]	= tik_id_tbl[tik_syu][0];					// ���ް���ID��ID1�i�]���^�j��ID�ɕύX
		}
	}

	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		���ԏ�No.�擾����														|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PkNo_get( *tik_data, tik_syu )							|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	���ް��߲��						|*/
/*|																				|*/
/*|						uchar	tik_syu		=	�����							|*/
/*|																				|*/
/*|	RETURN VALUE	:	ushort	pk_no = ���ԏ�No.								|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
ushort	PkNo_get( uchar *tik_data, uchar tik_syu )
{
	ushort	pk_no = 0;							// ���ԏ�No.

	switch( tik_syu ){							// ����H

//		case	1:								// ���Ԍ��i�`�q�|�����j
//		case	2:								// ���Ԍ��i�`�q-�T�����~���j
//		case	3:								// ���Ԍ��i�`�q-�o�񒆎~���j
//		case	4:								// ���Ԍ��i���Z�O�j
//		case	5:								// ���Ԍ��i���Z��j
//		case	6:								// ���Ԍ��i���~���j
//		case	7:								// ���Ԍ��i�|�����j
//		case	8:								// ���Ԍ��i�Đ��Z���~���j
//		case	9:								// ���Ԍ��i���Z�ς݌��j
//		case	10:								// �Ĕ��s��
//		case	11:								// ������
//
//			pk_no  = (ushort)tik_data[1];
//			pk_no += (ushort)pno_d1[tik_data[2]>>5];
//			pk_no += (((ushort)tik_data[8])<<3 )&0x0200;
//			break;

		case	12:								// �`�o�r�����
		case	14:								// �񐔌�
		case	15:								// �T�[�r�X���i�|�����E�������j

			pk_no  = (ushort)tik_data[1];
			pk_no += (ushort)pno_dt[tik_data[2]>>4];
			break;

		case	13:								// �v���y�C�h�J�[�h

			pk_no  = astoin( &tik_data[1], 2 );
			pk_no += ( tik_data[3]&0x0f ) * 100;
			break;

		case	16:								// �W���J�[�h

			pk_no = (	( (tik_data[27] & 0x0f) * 1000 ) +
						( (tik_data[28] & 0x0f) * 100 )  +
						( (tik_data[29] & 0x0f) * 10 )   +
						( (tik_data[30] & 0x0f) * 1 )
					);
			break;

	}
	return( pk_no );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		�K��O�̒��ԏ�No.�����ݏ���												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	NgPkNo_set( *tik_data, tik_syu  )						|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	���ް��߲��						|*/
/*|																				|*/
/*|						uchar	tik_syu		=	�����							|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
void	NgPkNo_set( uchar *tik_data, uchar tik_syu )
{

	switch( tik_syu ){							// ����H

		case	1:								// ���Ԍ��i�`�q�|�����j
		case	2:								// ���Ԍ��i�`�q-�T�����~���j
		case	3:								// ���Ԍ��i�`�q-�o�񒆎~���j
		case	4:								// ���Ԍ��i���Z�O�j
		case	5:								// ���Ԍ��i���Z��j
		case	6:								// ���Ԍ��i���~���j
		case	7:								// ���Ԍ��i�|�����j
		case	8:								// ���Ԍ��i�Đ��Z���~���j
		case	9:								// ���Ԍ��i���Z�ς݌��j
		case	10:								// �Ĕ��s��
		case	11:								// ������

			// ���ԏ�No.1023���
			tik_data[1] = (uchar)(tik_data[1] | 0x7f);
			tik_data[2] = (uchar)(tik_data[2] | 0x60);
			tik_data[8] = (uchar)(tik_data[8] | 0x40);
			break;

		case	12:								// �`�o�r�����
		case	14:								// �񐔌�
		case	15:								// �T�[�r�X���i�|�����E�������j

			// ���ԏ�No.1023���
			tik_data[1] = (uchar)(tik_data[1] | 0x7f);
			tik_data[2] = (uchar)(tik_data[2] | 0x70);
			break;

		case	13:								// �v���y�C�h�J�[�h

			// ���ԏ�No.1023���
			tik_data[1] = 0x32;
			tik_data[2] = 0x33;
			tik_data[3] = 0x0A;
			break;

		case	16:								// �W���J�[�h

			// ���ԏ�No.1023���
			tik_data[27] = 0x31;
			tik_data[28] = 0x30;
			tik_data[29] = 0x32;
			tik_data[30] = 0x33;
			break;

	}
}
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
/* MODULE NAME : ck_jis_credit												*/
/* �J�[�h���N���W�b�g�J�[�h���ۂ��`�F�b�N����								*/
/* Parameter:																*/
/*		MagDataSize  : ���[�h�f�[�^��										*/
/*		card_data(i) : ���[�h�������f�[�^									*/
/* Return:																	*/
/*		0 : ������															*/
/*		1 : JIS1/JIS2 �J�[�h												*/
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
uchar	ck_jis_credit( uchar MagDataSize, char *card_data )
{
	uchar	cWok;
	int		i;
	int		b_cnt = 0;								//�@�r�b�g�J�E���^

	// ID �� �r�b�g�� => cnt
	cWok = (uchar)card_data[0];
	for( i=0; i < 8; i++ ){
		if(cWok & 0x01) b_cnt++;
		cWok >>= 1;
	}

	// JIS1 ����
	if( ((uchar)card_data[0] == 0xff) &&			// ISO�ڼޯĎ��(0xff)
	    	((uchar)card_data[1] == 0xbb) )			// �J�n����(0xbb)
	{
		MAGred[MAG_ID_CODE] = 0x7F;					// �����R�[�h�ɕϊ�
		return 1;
	}

	// JIS2 ����
	if( ( b_cnt % 2 ) ||							// �p���e�B�[�͊�i�A�}�m�J�[�h�j
		( 69 > MagDataSize ) ||						// ���C�ް����� 69byte���� (JIS2�łȂ�)
													// FB�͊J�n�����̎��iIDϰ��j�`�I�������܂œǂݏo���̂�
													// JIS2�J�[�h��ǂނ� Length=70byte�ƂȂ�
		( (0x41 == card_data[0]) && (0x53 == card_data[10]) ) )	// ��ɌW������
																// ����ID = 41h ���� �ް�ID = 53h�ű�ɌW�����ނƔ��f����
	{
		return 0;
	}

	return 1;										// JIS2 ��������Ȃ�
}

/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
/* MODULE NAME : chk_for_inquiry											*/
/* ������⍇���`�F�b�N														*/
/* Parameter:																*/
/*		type : 																*/
/*			1 : �⍇��														*/
/*			2 : ����߽NG�̎��̂ݖ⍇��										*/
/* Return:																	*/
/*		0 : �⍇�����Ȃ�													*/
/*		1 : �⍇������														*/
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
uchar chk_for_inquiry(uchar type)
{
	if( _is_ntnet_normal()			&&						// NT-NET�ڑ�����H
		(OPECTL.ChkPassSyu == 0)	&&						// �⍇�����łȂ�
		(MifStat != MIF_WROTE_FAIL) &&						// ��������NG��̍ă^�b�`�łȂ�
		(uchar)prm_get( COM_PRM,S_NTN,26,1,4 ) == type) 	// ������⍇������?
	{
		return 1;
	}
	
	return 0;
}
/*[]-----------------------------------------------------------------------[]*/
/*| ����L�������`�F�b�N����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| NT-7700�̃`�F�b�N���W�b�N�ɍ��킹��B									|*/
/*|	�@ ذ�޵�ذ�ݒ�̏ꍇ�A���t�؊������� 0:00 �Ƃ�����������B				|*/
/*|	�A ذ��&ײĐݒ�̏ꍇ�A��P�����̌n�J�n����(31-0004)��������t�؊�����	|*/
/*|    �Ƃ�����������B														|*/
/*|	��UT��ذ��&ײČŒ�ݒ�̂��߁A��ɇA�ƂȂ�B							|*/
/*|																			|*/
/*| MODULE NAME  : Ope_PasKigenCheck()                                      |*/
/*| PARAMETER    : StartKigen_ndat = ��������̗L�������J�n��(ɰ�ײ�ޒl)	|*/
/*|				   EndKigen_ndat   = ��������̗L�������I����(ɰ�ײ�ޒl)	|*/
/*|				   TeikiSyu        = ������(1�`15)						|*/
/*|				   Target_ndat     = �����Ώ۔N����(ɰ�ײ�ޒl)				|*/
/*|				   Target_nmin     = �����Ώێ����@(ɰ�ײ�ޒl)				|*/
/*| RETURN VALUE : 0=�����J�n�����O�C1=�L���������C2=�����I��������     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : okuda			                                        |*/
/*| Date         : 2007/02/14                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
uchar	Ope_PasKigenCheck( ushort StartKigen_ndat, ushort EndKigen_ndat, short TeikiSyu, 
						   ushort Target_ndat, ushort Target_nmin )
{
	ushort	Kirikae_nmin;						// ���t�؊�����(ɰ�ײ�ޒl)
	uchar	ret=1;

	/** ���t�؊�����get **/

		Kirikae_nmin = (ushort)prm_tim( COM_PRM, S_RAT, (short)4 );	// ���t�؊������͗����̌n�J�n����(ɰ�ײ�ޒl)

	/** �L�����Ԃ̒l�����H���Ĕ�r���s�� **/

	if( Target_nmin < Kirikae_nmin ){			// �܂����t�؊������ɂȂ��Ă��Ȃ��i�Ώێ��ԁ��؊������j
		++StartKigen_ndat;						// �J�n���ƏI�������{�P�����Ĕ͈̓`�F�b�N���s��
		++EndKigen_ndat;
	}

	/** �͈����� **/

	if( Target_ndat < StartKigen_ndat )			// �����O�i�Ώۓ����J�n���j
		ret = 0;
	else if( EndKigen_ndat < Target_ndat )		// ������i�I�������Ώۓ��j
		ret = 2;

	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*| GT̫�ϯĐݒ���������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_GT_Settei_Check()                                    |*/
/*| PARAMETER    : gt_flg = �ǂݎ��������GTorAPS̫�ϯ�						|*/
/*|				   pno_syu= ��{,�g��1�`3�̔ԍ�								|*/
/*| RETURN VALUE : 0=�ݒ�OK,1=�ݒ�NG									    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : akiba			                                        |*/
/*| Date         : 2008/06/13                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
static short Ope_GT_Settei_Check( uchar gt_flg, ushort pno_syu )
{
short	ret;

	ret = 0;
	if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){									// APS/GT�ݒ�
		if( gt_flg == 1 ){														// ذ���ް���GT̫�ϯ�
			if( !prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){				// ���p���ԏꇂ��APS
				ret = 1;
			}
		}else{																	// ذ���ް���APS̫�ϯ�
			if( prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){					// ���p���ԏꇂ��GT
				ret = 1;
			}
		}
	}else{																		// APS�f�[�^
		if( gt_flg == 0 ){														// ذ���ް���APS̫�ϯ�
			if( prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){					// ���p���ԏꇂ��GT
				ret = 1;
			}
		}
	}
	if( GT_Settei_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z�p�����ذ��(�C�����Ŏg�p���������}���Ȃ��Ŏg�p�Ƃ���)     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_syusei( void )                                 |*/
/*| PARAMETER    : m_servic *mag : ���C�ް��i�[�擪���ڽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                      25 = �Ԏ�װ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_pasck_syusei( void )
{
	short	ret;
	short	w_syasyu;
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu

	vl_passet( vl_frs.lockno );
	w_syasyu = 0;

	ret = 0;
	if( prm_get(COM_PRM, S_TYP, 98, 1, 4) == 3 ){
		for( ; ; ){
			if( ryo_buf.syubet < 6 ){
				// ���Z�Ώۂ̗������A�`F(0�`5)
				data_adr = 10*(tsn_tki.kind-1)+9;					// �g�p�\������ʂ��ް����ڽ�擾
				data_pos = (char)(6-ryo_buf.syubet);				// �g�p�\������ʂ��ް��ʒu�擾
			}else{
				// ���Z�Ώۂ̗������G�`L(6�`11)
				data_adr = 10*(tsn_tki.kind-1)+10;					// �g�p�\������ʂ��ް����ڽ�擾
				data_pos = (char)(12-ryo_buf.syubet);				// �g�p�\������ʂ��ް��ʒu�擾
			}
			if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){	// �g�p�s�ݒ�H
				ret = 25;											// ���̎Ԏ�̌�
				break;
			}
			break;
		}
	}
	if( ret == 0 ){													// ����OK?
		if( w_syasyu ){
			vl_now = V_SYU;											// ��ʐ؊�
			syashu = (char)w_syasyu;								// �Ԏ�
		}else{														// �Ԏ�؊��łȂ�
			vl_now = V_TSC;											// �����(���Ԍ����p�L��)
		}
		card_use[USE_PAS] += 1;										// ���������+1
		PayData.teiki.syu = (uchar)tsn_tki.kind;					// ��������
		NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);
		PayData.teiki.status = (uchar)tsn_tki.status;				// ������ð��(�ǎ掞)
		PayData.teiki.id = tsn_tki.code;							// �����id
		NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);
		PayData.teiki.pkno_syu = (uchar)tsn_tki.pkno;				// ��������ԏ�m���D���
		NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);
		PayData.teiki.update_mon = 0;								// �X�V����
		PayData.teiki.s_year = tsn_tki.data[0];						// �L������(�J�n�F�N)
		PayData.teiki.s_mon = tsn_tki.data[1];						// �L������(�J�n�F��)
		PayData.teiki.s_day = tsn_tki.data[2];						// �L������(�J�n�F��)
		PayData.teiki.e_year = tsn_tki.data[3];						// �L������(�I���F�N)
		PayData.teiki.e_mon = tsn_tki.data[4];						// �L������(�I���F��)
		PayData.teiki.e_day = tsn_tki.data[5];						// �L������(�I���F��)
		PayData.teiki.update_rslt1 = 0;								// ����X�V���Z���̍X�V����			(OK�F�X�V�����^NG�F�X�V���s)
		PayData.teiki.update_rslt2 = 0;								// ����X�V���Z���̃��x�����s����	(OK�F���픭�s�^NG�F���s�s��)
		PassPkNoBackup = 0;											// ��������ԏ�ԍ��ޯ�����(n�������p)
		PassIdBackup = (ushort)tsn_tki.code;						// �����id�ޯ�����(n�������p)
		PassIdBackupTim = (ulong)( CPrmSS[S_TIK][9] * 60 * 2 );		// n�������p��ϰ�N��(500ms Timer)

		if( SKIP_APASS_CHK ){										// ��������߽OFF�ݒ�(��������������Ȃ�)�H
			OPECTL.Apass_off_seisan = 1;							// ��������߽OFF�ݒ��ԂŒ���g�p
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| JIS2����֐�														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_JIS2(void)			                               |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : result :  OK = JIS2                                     |*/
/*|                          NG = JIS2�ȊO                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-11-11                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	chk_JIS2( void )
{
	uchar i;
	uchar count = 0;
	uchar p_law = RDT_REC.rdat[3];
	uchar result = NG;
	
	for( i=0; i < 8; i++ ){										// �p���e�B���Z�o
		if(p_law & 0x01) count++;
		p_law >>= 1;
	}
	if( !((count % 2 == 0) && ((RDT_REC.idc2 - 3) >= 69)) ){	// JIS2�`���ł͂Ȃ�
		return result;											// NG�Ƃ���B
	}
	return (result = OK);
}

