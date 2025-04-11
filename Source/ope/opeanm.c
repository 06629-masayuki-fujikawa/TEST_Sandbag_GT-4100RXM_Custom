/*[]----------------------------------------------------------------------[]*/
/*| �ųݽϼ݊֘A����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include 	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"flp_def.h"
#include	"cnm_def.h"

char	IsSoundOnTime(void);
char	avm_alarm_flg;
char err_chk_note(void);
char	an_vol_flg;

short	AN_buf = -1;
//
// �O�ɏo���������R�Fcct.c �� "���̃J�[�h�͎g���܂���" �� an_msag ( )�ōs���Ă��邪�A
// �N���W�b�g���Z���� "���΂炭���҂�������" ���o���O�� AVM_STOP ���o���Ă��邱�ƂŁA
// �G���[�����������ꍇ�i�܂�"���΂炭..."�̑O������)�A"���΂炭..." ���������Ă��܂����̑Ή�.
// cct.c �ł� an_msag (  ) ���ɂ���� -1 �ɂ���B
//
uchar	ope_chk_paid_cash( void );

void	ope_anm( short prm )
{
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// ope_anm()���̏�����S�폜
//	short	cnt, ctl, wait;
//	char	ch = 0;
//	short	msg_cnt = 0;
//	short	an_msgno[10] = {0};					// ���b�Z�[�W
//		
//
//	wait = 0;
//	if( SODIAC_ERR_NONE == Avm_Sodiac_Err_flg ){
//		switch(prm){
//// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////		case AVM_Edy_OK:			/* Edy OK �� */
////		case AVM_Edy_NG:			/* Edy NG �� */
//// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case AVM_IC_OK:			/* IC�J�[�h���m�� */
//		case AVM_IC_NG1:			/* NG�� */
//		case AVM_IC_NG2:			/* NG�� */
//			/* �ݒ�Ɏ������ɒ��ډ���炷����(���ʉ�:ch1) */
//			an_msgno[0] = prm;
//			an_msag( an_msgno,  1, 1, 0, 1 );
//			break;
//		case AVM_RYOUKIN:						// �����ǂݏグ
//		case AVM_SHASHITU:						// �Ԏ��ǂݏグ
//			if( 0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 ) && 0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		
//				if(wait == 0){			// �������̃A�i�E���X�𒆒f���Ė炷
//					an_stop(0);		// ��~
//				}
//			}
//			if( prm == AVM_RYOUKIN ){
//				if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 )){		// �����ǂݏグ�ݒ�Ȃ�
//					return;
//				}
//				if(1 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// �Ԏ��ǂݏグ�ݒ肠��
//					wait = 1;										// ��I����҂�
//				}
//			}
//			else{
//				if( 0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// �Ԏ��ǂݏグ�ݒ�Ȃ�
//					return;
//				}
//			}
//			msg_cnt = an_msag_edit_rxm(prm);
//			if(msg_cnt != 0){ // ���b�Z�[�W���������ꂽ���̂ݑ��M����
//				// �J��Ԃ���:1��,�C���^�[�o����,�`�����l��1
//				if(wait == 0){										// �������̃A�i�E���X�𒆒f���Ė炷
//					an_stop(0);									// ��~
//				}
//				an_msag( (short *)an_msgbuf,  msg_cnt, 1, 0, 0 );
//			}
//			break;
//		case AVM_TEST:							// �o�[�W�����v��
//			an_test( 4 );
//			break;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�u�U�[��
		switch(prm){
		case AVM_BOO:
			if(OPECTL.Mnt_mod == 5){
				return;
			}
			if(avm_alarm_flg == 0 && CPrmSS[S_SYS][60] != 0){	// �x��ON
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
//				ExIOSignalwt(EXPORT_M_LD0, 0 );						// �x�񔭖C���̂݁FL�i�f�W�^���A���v�Q�C���F1�ȏ�j
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
				avm_alarm_flg = 1;
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
//				an_boo2(AVM_BOO);
				PKTcmd_alarm_start(BEEP_KIND_ALARM, BEEP_TYPE_START);
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
			}
			else if(avm_alarm_flg){						// �x��OFF
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
//				ExIOSignalwt(EXPORT_M_LD0, 1 );						// �ʏ�FH�i�f�W�^���A���v�Q�C���F0�j�ݒ�
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
				avm_alarm_flg = 0;
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
//				an_stop(1);			// ch1 Stop
				PKTcmd_alarm_start(BEEP_KIND_ALARM, BEEP_TYPE_STOP);
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
			}
			break;
		default:												// ���̑��A�i�E���X(�ݒ�Q��)
			break;
		}			
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�u�U�[��
//		case AVM_STOP:								// ������~
//			an_stop(0);			// ch0 Stop
//			if(!avm_alarm_flg){						// �x�񔭖C���͎~�߂Ȃ�
//				an_stop(1);			// ch1 Stop
//			}
//			break;
//		case AVM_AN_TEST:							// �A�i�E���X�e�X�g(�����e�i���X)
//			an_stop(0);
//			an_msag( (short *)avm_test_no, avm_test_cnt, 1, 1, avm_test_ch );
//			break;
//
//		default:											// ���̑��A�i�E���X(�ݒ�Q��)
//			if(prm == AVM_BGM){													// �`�����l���w��
//				ch = 1;
//			}
//			else{
//				ch = 0;
//			}
//			wait = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),1,3 );			// ���f
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
//			if (prm == AVM_SELECT_EMONEY) {
//				if (isEcEmoneyEnabled(1, 0) &&
//					check_enable_multisettle() == 1) {
//					// �o�^No.50�͓d�q�}�l�[�V���O���ݒ�̏ꍇ�A
//					// �����J�n����=2�Ƃ���
//					wait = 2;
//				}
//			}
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
//			if(wait == 0){										// �������̃A�i�E���X�𒆒f���Ė炷
//				if(ch == 1){
//					an_stop(ch);				// ��~
//				}
//				else{
//					if(OPECTL.PassNearEnd == 0){
//						an_stop(ch);			// ��~
//					}
//					else{
//						if(prm != AVM_KIGEN_NEAR_END){
//							OPECTL.PassNearEnd = 0;
//						}
//						else{
//							an_stop(ch);		// ��~
//						}
//					}
//				}
//			}
//			msg_cnt = msg_set(an_msgno, prm);						// ���b�Z�[�W���擾
//
//			cnt = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),2,4 );			// ������
//			if( cnt != 0 ){
//				if( cnt == 99 ){
//					cnt = 0x0f;
//				}
//				ctl = (short)prm_get( COM_PRM,S_ANA,(1+((prm-2)*4)),2,1 );			// �������
//
//				an_msag( an_msgno,  msg_cnt, cnt, ctl, ch );
//			}
//			break;
//		}
//	}
//	return;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
}
/*[]----------------------------------------------------------------------[]*/
/*| ���b�Z�[�W���i�[����                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : msg_set(short *an_msgno, short prm )                    |*/
/*| PARAMETER    : prm : �ݒ���ڽ��25-0002�`25-0030�̱��ڽ��2�`30         |*/
/*|              : an_msgno : ���b�Z�[�W���̊i�[��                         |*/
/*| RETURN VALUE : msg_cnt  : ���b�Z�[�W��                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short	msg_set(short *an_msgno, short prm )
{
	short	msg_cnt, p1, p2;
	char	i,j;
	short	rag_time;
	
	msg_cnt = 0;

	for(i=1; i<4; i++){
		p1 = (short)( CPrmSS[S_ANA][1+((prm-2)*4)+i] / 1000L );			// ү���އ�1
		p2 = (short)( CPrmSS[S_ANA][1+((prm-2)*4)+i] % 1000L );			// ү���އ�2
		if(p1 != 0 ){
			an_msgno[msg_cnt] = p1;
			msg_cnt++;
		}
		if(p2 != 0 ){
			an_msgno[msg_cnt] = p2;
			msg_cnt++;
		}
	}
	
	/* �g�p�s�}�̂̃��b�Z�[�W�͍폜���� */
	if( prm == AVM_TURIARI || prm == AVM_TURINASI || prm == AVM_RYOUSYUU){
		for(i = 0;i < msg_cnt; i++){
// MH810105(S) MH364301 �C���{�C�X�Ή�
//			if(an_msgno[i] == 147 && ( Ope_isPrinterReady() == 0 || OPECTL.RECI_SW == 1)){				// ���؂��Ԃŗ̎��؃��b�Z�[�W
			if( an_msgno[i] == 147 &&
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 				((Ope_isPrinterReady() == 0 || (IS_INVOICE && Ope_isJPrinterReady() == 0)) ||
				(((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) || (IS_INVOICE && Ope_isJPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
				 OPECTL.RECI_SW == 1) ){										// ���؂��Ԃŗ̎��؃��b�Z�[�W
// MH810105(E) MH364301 �C���{�C�X�Ή�
				for(j = i; j < msg_cnt; j++){
					memcpy(&an_msgno[j], &an_msgno[j+1], 2);
				}
				msg_cnt-=1;
				i-=1;
			}
		}
		if( prm != AVM_RYOUSYUU && err_chk_note() != 0 ){						// �������[�_�[�g�p�s��
			an_msgno[msg_cnt] = 180;											// �������p�s���b�Z�[�W
			msg_cnt++;
		}
	}
	if( AVM_FLAP_BAN == an_msgno[msg_cnt - 1] || AVM_LOCK_BAN == an_msgno[msg_cnt - 1] ){	// �g�t���b�v(���b�N)�������������Ƃ��m�F��A�h
		if( 0 == prm_get( COM_PRM,S_STM,1,1,1 )){	// �T�[�r�X�^�C���؊�    0=�Ȃ�(�S�Ԏ틤��)�^1=����(�Ԏ했�ɐݒ�)
			rag_time = prm_get( COM_PRM,S_STM,4,3,1 );									// �S�Ԏ틤�ʂ̃��O�^�C���擾
		} else {
			rag_time = prm_get( COM_PRM,S_STM,(short)(7+((SvsTime_Syu[ OPECTL.Pr_LokNo - 1 ] - 1)*3)),3,1 );	// �e�Ԏ틤�ʂ̃��O�^�C���擾
		}
		if( 10 > msg_cnt 												// ���b�Z�[�W�̈悪�󂢂Ă��Ȃ��ꍇ�͎��{���Ȃ�
			&& (( 3 == OPECTL.Ope_mod ) || ( 220 == OPECTL.Ope_mod ))){	// ���Z���� or ���O�^�C�������̏ꍇ
			if( 0 < rag_time && 16 > rag_time ){	// ���O�^�C����1�`15�̏ꍇ�́g�����ȓ��ɏo�ɂ��Ă��������h
				an_msgno[msg_cnt] = AVM_1MINUTE + rag_time - 1;	// �g�����ȓ��Ɂh
				msg_cnt++;
			}
		}
		if( 10 > msg_cnt ){	// ���b�Z�[�W�̈悪�󂢂Ă��Ȃ��ꍇ�͎��{���Ȃ�
			an_msgno[msg_cnt] = AVM_SYUKKO;						// �g�o�ɂ��ĉ������h
			msg_cnt++;
		}
	}
			
	return msg_cnt;
	
}
/*[]----------------------------------------------------------------------[]*/
/*| �������[�_�[�̎g�p�۔��f                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : err_chk_note(void)                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret 0�F�g�p�� 1�F�g�p�s��                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char err_chk_note(void)
{
	char ret,i;
	ret = 0;
	for(i=0;i<10;i++){
		if(ERR_CHK[mod_note][i]){
			ret = 1;
			break;
		}
	}
	if( ALM_CHK[1][6] ){	// �������ɖ��t�A���[���������H
		ret = 1;			// �g�p�s�Ƃ���B
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�h�A�m�u�߂��Y��h�~�`���C��
//[]----------------------------------------------------------------------[]
///	@param[in]		onoff	: 0:OFF, 1:ON
///	@return			void
///	@author			m.onouchi
///	@note			�h�A�m�u�߂��Y��h�~�`���C���̊J�n�C��~�𐧌䂷��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/13<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void opa_chime(char onoff)
{
	static char chime_flg = 0;

	if (onoff) {	// ON
		if (!chime_flg) {
			if (!avm_alarm_flg) {				// �h�A�x��Ȃ�
// MH810100(S) S.Takahashi 2020/02/21 #3915 �h�A�m�u�߂��Y��h�~�A���[������Ȃ�
//				an_boo2(AVM_IC_NG2);			// �b��I�Ƀ��b�Z�[�W�ԍ�105���g�p����B
				PKTcmd_alarm_start(BEEP_KIND_CHIME, BEEP_TYPE_START);
// MH810100(E) S.Takahashi 2020/02/21 #3915 �h�A�m�u�߂��Y��h�~�A���[������Ȃ�
			}
		}
	} else {		// OFF
		if (chime_flg) {
			if (!avm_alarm_flg) {				// �h�A�x��Ȃ�
// MH810100(S) S.Takahashi 2020/02/21 #3915 �h�A�m�u�߂��Y��h�~�A���[������Ȃ�
//				an_stop(1);						// �`�����l���P(���ʉ�)������~
				PKTcmd_alarm_start(BEEP_KIND_CHIME, BEEP_TYPE_STOP);
// MH810100(E) S.Takahashi 2020/02/21 #3915 �h�A�m�u�߂��Y��h�~�A���[������Ȃ�
			}
		}
	}
	chime_flg = onoff;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���݂̓����z�Ő��Z�����������`�F�b�N����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ope_chk_paid_cash(void)                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret 0�F���Z�������������� 1�F�����s��                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-05-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	ope_chk_paid_cash( void )
{
	if( ryo_buf.dsp_ryo <= cn_GetTempCredit()) {
		return 0;		// ���Z����
	}
	else {
		return 1;		// �����s��
	}
}

