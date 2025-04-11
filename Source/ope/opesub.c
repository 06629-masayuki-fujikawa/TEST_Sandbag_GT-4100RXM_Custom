/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް�֘A����                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"lcd_def.h"
#include	"flp_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"common.h"
#include	"ntnet.h"
#include	"cnm_def.h"
#include	"suica_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ope_ifm.h"
#include	"ntnet_def.h"
#include	"remote_dl.h"
#include	"mnt_def.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
// MH810100(S) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
#include	"pkt_def.h"
// MH810100(E) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)

extern ulong FLT_FindFirstLog(short id, t_FltLogHandle *handle, char *buf);
extern ulong FLT_FindNextLog_OnlyDate(short id, t_FltLogHandle *handle, char *buf);
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//extern	void	Pcard_shut_close( void );
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern void lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern void lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//// MH810100(S) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//void DiscountSortCheck();
BOOL DiscountSortCheck();
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
extern BOOL IsDupSyubetuwari(void);
// MH810100(E) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// MH810100(S) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// BOOL IsWaribikiLimitOver(void);
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)

// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
extern uchar r_zero_call;
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j

// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
short GetTypeSwitch(ushort syu, ushort card_no);
short CheckDiscount(ushort syu, ushort card_no, ushort info);
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)

static	short	opncls_TrbClsChk( void );
char	save_op_faz;

uchar	Chu_Syu_Status;				// ���~�W�v���{�ð�� 0�F�����{ 1�F���{�ς�
char	PrcKigenStr[32];
// MH321800(S) G.So IC�N���W�b�g�Ή�
t_CyclicDisp	Ope2CyclicDisp;
// MH321800(E) G.So IC�N���W�b�g�Ή�
#define	_CHKDATE(s)		chkdate2((short)(s.year + 2000), (short)s.mon, (short)s.day)
short chkdate2(short yyyy, short mm, short dd);
static date_time_rec	*get_log_date_time_rec( ushort LogSyu, uchar *record );
Receipt_data	PayInfoData_SK;							// ���Z���f�[�^�p�����E�s���o�Ƀf�[�^
uchar	PayInfo_Class;									// ���Z���f�[�^�p�����敪
// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
ulong	g_ulZumiWari;								// �g�p�ς݊����z�𕪊�����悤
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
ulong	g_ulZumiWariOrg;							// �g�p�ς݊����z�𕪊�����܂�
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j


/*[]----------------------------------------------------------------------[]*/
/*| ���C���ޏ���                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_card( msg, mod )                                     |*/
/*| PARAMETER    : msg : ү����                                            |*/
/*|                paymod : 0=�ʏ� 1=�C��                                  |*/
/*| RETURN VALUE : ret : ���Z����Ӱ��                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_card( ushort msg, ushort paymod )
{
	extern	uchar ryodsp;
	short	ret, rd_snd, anm;
	short	dno;
	short	ErrNum=0;
	char	f_Card = 0;												// 0:�s���W������ 1:�W�����ގ� -1:�W������NG
	uchar	PrcKiFlg = 0;
	uchar	CardKind;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	uchar	credit_check = CRE_STATUS_OK;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

	ret = 0;
	rd_snd = 2;														// ��o�����ړ�
	anm = 0;
	dno = 0;

	if( MifStat == MIF_WRITING )	// Mifare���^�b�`(�ǎ抮��)���Ă����玥�C���ނ͓ǂ܂Ȃ�
		goto MifareWriting;		

	CardKind = (uchar)((OPECTL.LastCardInfo&0x8000) ? 0xFF:MAGred[MAG_ID_CODE]);	// JIS�J�[�h����
	switch( CardKind ){	
		case 0x1a:													// APS���
			if( SyuseiEndKeyChk() == 2 ){
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}
// �s��C��(S) K.Onodera 2016/10/13 #1587 ���z�w��̉��u���Z���ɑS�������}������ƃt���[�Y����
			// ���u���Z(���z�w��)
			if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE ){
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}
// �s��C��(E) K.Onodera 2016/10/13 #1587 ���z�w��̉��u���Z���ɑS�������}������ƃt���[�Y����

			switch( OPECTL.CR_ERR_DSP = (char)al_pasck( (m_gtapspas *)MAGred ) ){
				case 0:												// OK
					if( SKIP_APASS_CHK ){							// ��������߽OFF�ݒ�(��������������Ȃ�)�H
						OPECTL.Apass_off_seisan = 1;				// ��������߽OFF�ݒ��ԂŒ���g�p
					}
					al_pasck_set();							// PayData�ɒ��������������
					if(OPECTL.PassNearEnd == 1){
						ope_anm( AVM_KIGEN_NEAR_END );
					}
					ryo_cal( 1, OPECTL.Pr_LokNo );					// �����v�Z
					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// ������
					}

					if( paymod ){									// �C���H
						ac_flg.cycl_fg = 70;						// ����
					}else{
						ac_flg.cycl_fg = 10;						// ����
					}

					ret = in_mony( msg, paymod );
					al_mkpas( (m_gtapspas *)MAGred, &CLK_REC );
					if( CPrmSS[S_TIK][10] == 0 ){					// ��������̧����Ȃ��ݒ�?
						rd_snd = 16;								// ������ް�ײ�(���̧�����)
					}else{
						rd_snd = 9;									// ������ް�ײ�(���̧��L��)
					}
					read_sht_opn();										/* ���Cذ�ް���������� */
					if( ryo_buf.zankin ){							// ���Z���闿��������ꍇ
					if( !SyuseiEndKeyChk() ){
						cn_stat( 3, 2 );									/* ������ */
					}
					}

					if (ope_imf_GetStatus() == 2) {
						if (is_paid_remote(&PayData) < 0) {
						// �U�֐��Z�����{�i������p�w���j�Ȃ炱���Ōv��
							ope_ifm_FurikaeCalc( 0 );
							ret = in_mony(IFMPAY_FURIKAE, 0);
							// ���łɐ��Z���������Ă���΂O�~�U�ւȂ̂�ret�͕ω����Ȃ��i�㏑��ok�j
						}
					}
// MH322914(S) K.Onodera 2016/09/14 AI-V�Ή��F�U�֐��Z
					if( PiP_GetFurikaeSts() ){
// �s��C��(S) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
//						if( is_paid_remote(&PayData) < 0 ){
// �s��C��(E) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
							ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
							vl_now = V_FRK;						// �U�֐��Z
							ryo_cal( 3, OPECTL.Pr_LokNo );		// �T�[�r�X���Ƃ��Čv�Z
							ret = in_mony(OPE_REQ_FURIKAE_GO, 0);
// �s��C��(S) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
//						}
// �s��C��(E) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
					}
// MH322914(E) K.Onodera 2016/09/14 AI-V�Ή��F�U�֐��Z
					if ( OPECTL.Pay_mod == 2 ) {					// �C�����Z��
						if (is_paid_syusei(&PayData) < 0) {
							ope_ifm_FurikaeCalc( 1 );
							ret = in_mony(IFMPAY_FURIKAE, 0);
							// ���łɐ��Z���������Ă���΂O�~�U�ւȂ̂�ret�͕ω����Ȃ��i�㏑��ok�j
						}
					}
// �s��C��(S) K.Onodera 2016/11/24 #1581 �U�֐��Z�̐��Z���@�ɁA2=��������Z���Z�b�g����Ă��܂�
//					if(OpeNtnetAddedInfo.PayMethod != 11){
					if(OpeNtnetAddedInfo.PayMethod != 11 && OpeNtnetAddedInfo.PayMethod != 12 ){
// �s��C��(E) K.Onodera 2016/11/24 #1581 �U�֐��Z�̐��Z���@�ɁA2=��������Z���Z�b�g����Ă��܂�
						OpeNtnetAddedInfo.PayMethod = 2;	// ���Z���@=2:��������Z
					}
					break;

				case 5:												// �������
					dno = 3;										// "     ���̃J�[�h�͖����ł�     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// �����؂�
				case 6:												// �����O
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				case 99:											// HOST�֖⍇��
					read_sht_cls();										/* ���Cذ�ް������۰�� */
					cn_stat( 2, 2 );									/* �����s�� */
					rd_snd = 0;
					NTNET_Snd_Data142( OPECTL.ChkPassPkno, OPECTL.ChkPassID );	// ������⍇���ް�(�ް����142)�쐬
					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);	// "                              "
					blink_reg(7, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5]);	// "     ���΂炭���҂�������     "
					Lagtim( OPETCBNO, 6, (ushort)(prm_get( COM_PRM,S_NTN,31,2,1 )*50+1) );	// ��ϰ6(XXs)�N��(������⍇���Ď�)
					rd_snd = 0;										// ذ�ް���䖳��(���ނ�ذ�ް���ɕۗ�����)
					LagCan500ms(LAG500_MIF_LED_ONOFF);
					if( MIFARE_CARD_DoesUse ){						// Mifare���L���ȏꍇ
						op_MifareStop_with_LED();					// Mifare����
					}
					ret = 0;
					break;

				case 4:
				case 29:											// �⍇���^�C���A�E�gNG��
					if( OPECTL.CR_ERR_DSP == 4 )
						dno = 19;										// "      ���o�ɕs��v�ł�        "
					else
						dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

		case 0x2d:													// ���޽��
			if( SyuseiEndKeyChk() ){
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_svsck( (m_gtservic *)MAGred ) ){
				case 0:												// OK
					ryo_cal( 3, OPECTL.Pr_LokNo );					// �����v�Z

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// ������
					}

					if( paymod ){									// �C���H
						ac_flg.cycl_fg = 70;						// ����
					}else{
						ac_flg.cycl_fg = 10;						// ����
					}

					ret = in_mony( msg, paymod );

					if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){			// �����z < ���ԗ��� ?
						if(( CPrmSS[S_DIS][3] == 1 )&&				// ���޽����荞��
						   ( CPrmSS[S_DIS][2] == 0 )){				// �p��ϰ��󎚂��Ȃ�
							/*** �A���}�������� ***/
							cr_service_holding = 1;					// ���޽���A������ �ۗ���
							rd_snd = 3;								// ذ�޺����(���Ԍ��ۗ���)
						}else{
							/*** �A���}�������Ȃ� ***/
							rd_snd = 10;							// ���ތ���r�o
						}
					}else{
						rd_snd = 11;								// ���ޑO������Č�A����r�o (���Ԍ��}�����\�Ƃ���ׂ�TF-4300N��Ă��ڐA)
					}
					break;

				case 3:												// �����؂�
				case 6:												// �����O
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				case 14:											// ���x�������ް
					dno = 8;										// "    ���x�����𒴂��Ă��܂�    "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

		case 0x0e:													// ����߲�޶���
			if( SyuseiEndKeyChk() ){
				dno = 1;											// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}
			if( 3 == prm_get( COM_PRM,S_PAY,21,1,3) || 
				4 == prm_get( COM_PRM,S_PAY,21,1,3)) {
				// �v�����^�Ȃ��̏ꍇ�̓v���y�C�h�J�[�h�͎g�p�s��
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_preck( (m_gtprepid *)MAGred ) ){
				case 0:												// OK
					Ope_Last_Use_Card = 1;							// �v���y���p
					ryo_cal( 2, OPECTL.Pr_LokNo );					// �����v�Z

					OPECTL.LastUsePCardBal = (ulong)CRD_DAT.PRE.ram;// ����߲�ގc�z�ۑ�

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// ������
					}

					if( paymod ){									// �C���H
						ac_flg.cycl_fg = 70;						// ����
					}else{
						ac_flg.cycl_fg = 10;						// ����
					}

					ret = in_mony( msg, paymod );
					al_mkpre( (m_gtprepid *)MAGred, &CRD_DAT.PRE );
// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//					if( (CPrmSS[S_PRP][10] == 1) ||					// ���̧�����ݒ�
//						((CPrmSS[S_PRP][10] == 2) && (CRD_DAT.PRE.ram)) ){	// �c�x����0�łȂ�������̧�����
					if( ( prm_get( COM_PRM,S_PRP,10,1,1) == 1) ||						// ���̧�����ݒ�
						((prm_get( COM_PRM,S_PRP,10,1,1) == 2) && (CRD_DAT.PRE.ram)) ){	// �c�x����0�łȂ�������̧�����
// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
						rd_snd = 12;								// �ް�ײ�(���̧��L��)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//						if(1 == prm_get( COM_PRM,S_PRP,10,1,2)){
//							Pcard_shut_close();
//						}
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
					}
					else{
						rd_snd = 17;								// �ް�ײ�(���̧�����)
					}
					break;

				case 7:												// �c�z���
					dno = 4;										// "       �c�z������܂���       "
					anm = AVM_CARD_ERR1;
					break;

				case 10:											// �\���װ
					dno = 5;										// "      �}���������Ⴂ�܂�      "
					anm = AVM_CARD_ERR1;
					break;

				case 14:											// ���x�������ް
					dno = 8;										// "    ���x�����𒴂��Ă��܂�    "
					anm = AVM_CARD_ERR1;
					break;

				case 3:												// �L�������؂� 
					alm_chk( ALMMDL_SUB2, ALARM_OUTSIDE_PERIOD_PRC, 2 );	/* �װ�۸ޓo�^ �����E����	*/
					PrcKiFlg = 1;
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			GetPrcKigenStr ( PrcKigenStr );
			break;

		case 0x2c:													// �񐔌�
			if( SyuseiEndKeyChk() ){
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}
			if( 3 == prm_get( COM_PRM,S_PAY,21,1,3) || 
				4 == prm_get( COM_PRM,S_PAY,21,1,3)) {
				// �v�����^�Ȃ��̏ꍇ�͉񐔌��g�p�s��
				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
				anm = AVM_CARD_ERR1;
				break;
			}

			switch( OPECTL.CR_ERR_DSP = (char)al_kasck( (m_gtservic *)MAGred ) ){
				case 100:											// OK (�����؂�ԋ�)
					dno = 10;										// "���̶��ނ͂������������؂�ł�"
					OPECTL.PassNearEnd = 1;
					ope_anm( AVM_KIGEN_NEAR_END );
				case 0:												// OK

					Ope_Last_Use_Card = 2;							// �񐔌����p
					ryo_cal( 2, OPECTL.Pr_LokNo );					// �����v�Z

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// ������
					}

					ac_flg.cycl_fg = 10;							// ����

					ret = in_mony( msg, paymod );

					al_mkkas( (m_gtkaisuutik *)MAGred, &CRD_DAT.SVS, &CLK_REC );	// ���ް��쐬
// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//					if( (CPrmSS[S_PRP][10] == 1) ||					// ���̧�����ݒ�
//						((CPrmSS[S_PRP][10] == 2) && (KaisuuAfterDosuu)) ){	// �c�x����0�łȂ�������̧�����
					if( ( prm_get( COM_PRM,S_PRP,10,1,1) == 1) ||							// ���̧�����ݒ�
						((prm_get( COM_PRM,S_PRP,10,1,1) == 2) && (KaisuuAfterDosuu)) ){	// �c�x����0�łȂ�������̧�����
// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
						rd_snd = 20;								// �ް�ײ�(���̧��L��)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//						if(1 == prm_get( COM_PRM,S_PRP,10,1,2)){
//							Pcard_shut_close();
//						}
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
					}
					else{
						rd_snd = 19;								// �ް�ײ�(���̧�����)
					}
					break;

				case 3:												// �����؂�
				case 6:												// �����O
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				case 7:												// �c�z���
					dno = 4;										// "       �c�z������܂���       "
					anm = AVM_CARD_ERR1;
					break;

				case 10:											// �\���װ
					dno = 5;										// "      �}���������Ⴂ�܂�      "
					anm = AVM_CARD_ERR1;
					break;

				case 14:											// ���x�������ް
					dno = 8;										// "    ���x�����𒴂��Ă��܂�    "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;
	    default:
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//			if( SyuseiEndKeyChk() ){
//				dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
//				anm = AVM_CARD_ERR1;
//				break;
//			}
//			// JIS1 �ƔF���������A���̑��́i�A�}�m�J�[�h�ȊO�A�v���J�ȊO�j�ꍇ�B�C�����Z�� credit �s��
//			if( RD_Credit_kep != 0 && ryo_buf.zankin != 0L && paymod != 1)
//			{
//// �N���W�b�g�J�[�h�̂Ƃ��̂݁A���د��\���p��ϰSTOP
//				Lagcan( OPETCBNO, 10 );									// ���p�\�}�̻��د��\���p��ϰSTOP
//				if( Suica_Rec.Data.BIT.CTRL_MIRYO || 
//					Edy_Rec.edy_status.BIT.CTRL_MIRYO )					// ���ϖ������������Ă���ꍇ�͸ڼޯĂ̎g�p��s�Ƃ���
//				{
//					opr_snd( 2 );										//�O���r�o
//					return 0;
//				}
//
//				// �N���W�b�g���Z�\��Ԃ��`�F�b�N
//				credit_check = creStatusCheck();
//				if( credit_check != 0 ){								// �G���[����̏ꍇ
//					if( credit_check & CRE_STATUS_DISABLED ){			// �N���W�b�g�ݒ�Ȃ�
//						ErrNum = 2;		// "    ���̃J�[�h�͎g���܂���    "
//					}
//					if( credit_check & CRE_STATUS_NOTINIT ){			// ����ڑ�������
//						;	// �^�M�⍇���\
//					}
//					if( credit_check & CRE_STATUS_STOP ){				// �N���W�b�g��~��
//						ErrNum = 2;		// "    ���̃J�[�h�͎g���܂���    "
//					}
//					if( credit_check & CRE_STATUS_UNSEND ){				// ����˗������M
//						;	// �^�M�⍇���\
//					}
//					if( credit_check & CRE_STATUS_SENDING ){			// �ʐM��
//						;	// �^�M�⍇���\
//					}
//					if( credit_check & CRE_STATUS_PPP_DISCONNECT ){		// PPP���ڑ�
//						ErrNum = 2;		// "    ���̃J�[�h�͎g���܂���    "
//					}
//				}
//				if( ErrNum == 0 ){
//					// ���Z���x�z�@�`�F�b�N
//					if( creLimitCheck( ryo_buf.zankin ) != 0 )
//					{														// "�N���W�b�g�J�[�h�ł�"
//						ErrNum = 4;											// "���Z���x�z�������Ă��܂�"
//					}
//					else if( e_incnt > 0 || PayData.Electron_data.Suica.pay_ryo || PayData.Electron_data.Edy.pay_ryo )
//					{														// �@�d�q�}�l�[�g�p�ώ��̓N���W�b�g�̎g�p��s�Ƃ���
//						ErrNum = 2;											// "    ���̃J�[�h�͎g���܂���    "
//					}
//					else if( OpeNtnetAddedInfo.PayMethod == 11 &&			// �U�֐��Z�v����M�ς�
//							 vl_frs.antipassoff_req &&						// ����g�p����
//							 OPECTL.Ope_mod != 2 &&							// �C�����Z�łȂ�
//							 !PayData.teiki.syu ){							// ��������g�p�̏ꍇ
//						ErrNum = 2;											// "    ���̃J�[�h�͎g���܂���    "
//					}
//					else
//					{
//						OPECTL.InquiryFlg = 1;		// �O���Ɖ�t���OON
//						if( MIFARE_CARD_DoesUse ){									// Mifare���L���ȏꍇ
//							op_MifareStop_with_LED();								// Mifare����
//						}
//						LagCan500ms(LAG500_MIF_LED_ONOFF);
//
//						creSeisanInfo.amount = ryo_buf.zankin;				// CRE�ւ̐��Z�v���p
//						ryo_buf.credit.pay_ryo = ryo_buf.zankin;			// �v�Z�p
//
//						memset( creSeisanInfo.jis_1, 0x20, sizeof(creSeisanInfo.jis_1) );
//						memset( creSeisanInfo.jis_2, 0x20, sizeof(creSeisanInfo.jis_2) );
//
//						if( MAGred[MAG_ID_CODE] == CREDIT_CARDID_JIS1 )
//						{
//							memcpy( creSeisanInfo.jis_1, &MAGred[MAG_ID_CODE+2], sizeof(creSeisanInfo.jis_1) );
//						}else{
//							memcpy( creSeisanInfo.jis_2, &MAGred[MAG_ID_CODE], sizeof(creSeisanInfo.jis_2) );
//						}
//
//						if( credit_check & CRE_STATUS_SENDING ){			// �ʐM���̏ꍇ
//							;	// ������M����܂ŉ������Ȃ�
//						}
//						else if( credit_check & CRE_STATUS_NOTINIT ){		// ����ڑ��������̏ꍇ�́A�J�Ǐ������s��
//							cre_ctl.OpenKind = CRE_KIND_AUTO;				// �J�ǔ����v���Ɏ�������ݒ�
//							// �J�ǃR�}���h(01)���M
//							creCtrl( CRE_EVT_SEND_OPEN );
//						}
//						else if( credit_check & CRE_STATUS_UNSEND ){		// ����˗������M�̏ꍇ�́A����˗����M�������s��
//							cre_ctl.SalesKind = CRE_KIND_AUTO;				// ����˗������v���Ɏ�������ݒ�
//							// ����˗��f�[�^(05)���M
//							creCtrl( CRE_EVT_SEND_SALES );
//						}else{
//							// �^�M�⍇���f�[�^(03)���M, ���ꂪ���튮����� ����˗��f�[�^(05)���M���s���B
//							if(	creCtrl( CRE_EVT_SEND_CONFIRM ) != 0 )
//							{
//								creSeisanInfo.amount = 0;
//								ryo_buf.credit.pay_ryo = 0;
//								ErrNum = 2;										// "    ���̃J�[�h�͎g���܂���    "
//
//								xPause( 20 );		// 200ms wait���Ă���NG���[�����M�iMifare���������ł��Ȃ����߁j
//								queset( OPETCBNO, CRE_EVT_04_NG1, 0, 0 );	// OpeMain�֒ʒm (Mifare,Suica,Edy�𕜊�)
//							}
//						}
//					}
//				}
//				if(ErrNum)
//				{
//					opr_snd( 2 );		//�O���r�o
//					// ���b�Z�[�W�\�� �G���[
//					creMessageAnaOnOff( 1, (short)ErrNum );
//					OPECTL.InquiryFlg = 0;		// �O���Ɖ�t���OOFF
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
//
//				}else{
//					read_sht_cls();									// �����I�ɼ���������
//					cn_stat( 2, 2 );								// �����s��
//
//					/* �Ɖ�\������O�ɒʏ�\�����邱�ƂŁA�₢���킹������ɃG���[�\������������Ȃ��悤�ɂ��� */
//					if( ryodsp ){
//						Lcd_Receipt_disp();
//					}else{
//						dsp_intime( 7, OPECTL.Pr_LokNo );			// ���Ɏ����\��
//					}
//					// ���b�Z�[�W�\���@�Ɖ
//					creMessageAnaOnOff( 1, 1 );
//					if( OPECTL.op_faz == 0 ){
//						OPECTL.op_faz = 1;							// ������
//					}
//				}
//				goto AL_CARD_END;
//			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���C���[�_�[�ɃN���W�b�g�J�[�h�}�������Ƃ��̈ē������E�\��
			if (isEC_USE()) {
				OPECTL.CR_ERR_DSP = 8;								// "   ���� �g�p�s���ł� ����   "
				if (CardKind == 0xff && isEcBrandNoReady(BRANDNO_CREDIT)) {
				// JIS�J�[�h�Ȃ特����ς���
					anm = AVM_CARD_ERR8;							// �ųݽ �u���σ��[�_�[�Ő��Z���Ă��������v
					dno = 56;										// "���σ��[�_�[�Ő��Z���Ă�������"
				}
				else {
					anm = AVM_CARD_ERR4;							// �ųݽ �u���̃J�[�h�͂��戵���ł��܂���v
					dno = 41;										// "���̃J�[�h�͂��戵���ł��܂���"
				}
				if( rd_tik == 0 ){									// �������ۯ��ɒ��Ԍ�����?
					rd_snd = 13;									// ����o����(ذ�ް�E����������߂�)
				}
				break;
			}
// MH810103 GG119202(E) ���C���[�_�[�ɃN���W�b�g�J�[�h�}�������Ƃ��̈ē������E�\��
MifareWriting:
			OPECTL.CR_ERR_DSP = 8;									// "   ���� �g�p�s���ł� ����   "
			anm = AVM_CARD_ERR1;
			dno = 1;												// "    ���̃J�[�h�͎g���܂���    "
			if( rd_tik == 0 ){										// �������ۯ��ɒ��Ԍ�����?
				rd_snd = 13;										// ����o����(ذ�ް�E����������߂�)
			}
			break;
	}

	if(MAGred[MAG_ID_CODE] == 0x1a && ( OPECTL.CR_ERR_DSP != 0 && OPECTL.CR_ERR_DSP != 99 )){						// ���C��������G���[����
		if( MIFARE_CARD_DoesUse && OPECTL.ChkPassSyu == 0 ){ 								// ���C������g�p����ݒ肩�±���߽�ʐM�������łȂ��ꍇ
			op_MifareStart();											// Mifare��L����
		}
	}

	if( rd_snd ){
		opr_snd( rd_snd );
	}

	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}

	if( dno ){
		if(PrcKiFlg != 0){
			Lcd_WmsgDisp_ON2(  1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );	// ���̃J�[�h�͊����؂�ł�
			Lcd_WmsgDisp_ON ( 1, COLOR_RED, LCD_BLINK_OFF, (unsigned char*)PrcKigenStr );	// �L�������w�w�w�w�N�w�w���w�w��
		}else{
			grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );	// �װ�\���͔��]�����ɂĕ\��
		}
// MH810105(S) MH364301 �C���{�C�X�Ή�
		OPECTL.f_CrErrDisp = 1;							// �G���[�\����
// MH810105(E) MH364301 �C���{�C�X�Ή�
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//AL_CARD_END:
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	if( OPECTL.CR_ERR_DSP || ErrNum || f_Card == 1 ){
		Op_StopModuleWait_sub( 2 );						// �d�q�}�̍Ċ������v��
	}
	return( ret );
}

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
////[]----------------------------------------------------------------------[]
/////	@brief			�s�J�[�h��t��
////[]----------------------------------------------------------------------[]
/////	@return			none
////[]----------------------------------------------------------------------[]
/////	@author			MATSUSHITA
/////	@date			Create	:	2020/06/22<br>
/////					Update
////[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//void	AcceptTCard(void)
//{
//	short   i;
//	long	Err;
//	char    prev_Tcard_Retry_Cnt = Tcard_Retry_Cnt;
//	
//
//	ec_MessageAnaOnOff(0, 0);
//	T_Rate_now.disable = 0;
//	if ((OPECTL.op_faz > 1)  									// ���Z�@�̏����t�F�[�Y�����Z���i�������^�������j�łȂ�
//	|| (Suica_Rec.Data.BIT.CTRL_MIRYO != 0 )){					// �d�q�}�l�[�̏����������������ł���B
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;				// ���̑��̃G���[
//	/*	ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);			�G���[�\�����Ȃ� */
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// �u�����h�I�����ʉ���NG
//		// �u�����h�I���f�[�^�͑��M���Ȃ�
//		return;
//	}
//	if (T_Card_Use != 0) { 										// ����T�J�[�h��t�ςł���B	
//		BrandResRsltTbl.no = EC_BRAND_REASON_211;				// ���̑��̃G���[
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();										// �u�����h�I���f�[�^
//		return;
//	}
//
//	// T�J�[�h�ԍ��̎擾
//	memset(T_membar_num, 0x20, sizeof(T_membar_num));
//	for(i = 0; i < sizeof(T_membar_num); i++){
//		if (MAGred_HOUJIN[MAG_ID_CODE + i] == ' ') {							// �I�[�X�y�[�X
//			break;
//		}
//		if(chk_num(MAGred_HOUJIN[MAG_ID_CODE + i])){							//  '0'�`'9'�͈͓̔� ASCII
//			T_membar_num[i] = (uchar)(MAGred_HOUJIN[MAG_ID_CODE + i] & 0x0f);	// ���l�ɕϊ�
//		}
//		else {																	// �t�H�[�}�b�g�G���[
//			// ���σ��[�_�Ń`�F�b�N����̂ł����ɂ͗��Ȃ��������O�̂���
//			i = -1;	/* �J�[�h�f�[�^�s�� */
//			break;
//		}
//	}
//	if (i < 0) {			// ����ԍ��̑S���������i�P�`�X�j
//		err_chk( ERRMDL_CARD, ERR_TCARD_CHK_NG, 2, 0, 0 );
//		BrandResRsltTbl.no = EC_BRAND_REASON_203;			// ���f�[�^���܂܂��J�[�h��ǂݎ�肵���B
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);			// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();									// �u�����h�I���f�[�^
//		return;
//	}
//	
//	Err = chk_Tcard_use();
//	if (Err != 0) {											// �s�J�[�h�g�p�ۂ̐ݒ肪�L��
//		err_chk2( ERRMDL_CARD, ERR_CARD_T, 2, 2, 0, &Err );
//		T_Card_Use = 0;										// �s�J�[�h�g�p�׸ނ����낷
//		if(Err == 13){										// �񓯊��f�[�^�����^�C���A�E�g����
//			Card_Retry_DispAna(0, &Tcard_Retry_Cnt);
//			if (prev_Tcard_Retry_Cnt < 3) {					// 3��܂ł�203��Ԃ��B�ȍ~��208��Ԃ��B
//				// ���Z�@�\����Card_Retry_DispAna()�ōs�����߁AEC_BrandResultDisp�͌Ă΂Ȃ�
//				BrandResRsltTbl.no = EC_BRAND_REASON_203;	// ���f�[�^���܂܂��J�[�h��ǂݎ�肵���B
//			} else {
//				// ���Z�@�\����Card_Retry_DispAna()�ōs�����߁AEC_BrandResultDisp�͌Ă΂Ȃ�
//				BrandResRsltTbl.no = EC_BRAND_REASON_208;	// �L���ȃJ�[�h�ł��邪�g�p�ł��Ȃ��B
//			}
//		}
//		else{
//			BrandResRsltTbl.no = EC_BRAND_REASON_208;		// �L���ȃJ�[�h�ł��邪�g�p�ł��Ȃ��B
//		}
//		ec_BrandResultDisp(BRANDNO_TCARD, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);			// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();									// �u�����h�I���f�[�^
//		return;
//	}
//	
//	
//	// T�J�[�h��t		
//	Ope_Last_Use_Card = 6;	
//	ope_anm( AVM_TCARD_OK );							// �s�J�[�h��t�A�i�E���X
//	T_Card_Use = 0xff;
//	in_mony( EC_BRAND_RESULT_TCARD, 0 );
//	Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);			// �u�����h�I�����ʉ���OK
//	Ope_EcPayStart();									// �u�����h�I���f�[�^
//}

////[]----------------------------------------------------------------------[]
/////	@brief			�@�l�J�[�h�₢���킹
////[]----------------------------------------------------------------------[]
/////	@return			0: OK �@�l�J�[�h�₢���킹���s
/////					1: NG
////[]----------------------------------------------------------------------[]
/////	@author			MATSUSHITA
/////	@date			Create	:	2020/07/01<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//int	InquireHoujin(void)
//{
//	extern	void	chg_ShiftIN_OUT(char *data, short size);
//	extern	void	NTNETDOPA_Snd_Data148(uchar type);
//
//	m_Tcard *Hcard_dat;
//	short	ErrNum;
//	short   i;
//
//	ec_MessageAnaOnOff(0, 0);
//	if ((OPECTL.op_faz > 1)  								// ���Z�@�̏����t�F�[�Y�����Z���i�������^�������j�łȂ�
//	|| (Suica_Rec.Data.BIT.CTRL_MIRYO != 0 )){				// �d�q�}�l�[�̏����������������ł���B
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;				// ���̑��̃G���[
//	/*	ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);			�G���[�\�����Ȃ� */
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);				// �u�����h�I�����ʉ���NG
//		// �u�����h�I���f�[�^�͑��M���Ȃ�
//		return 1;
//	}
//	
//	// al_card()���Ɠ����̃`�F�b�N�����{����
//	// �d�q�}�l�[���ς͓������[�_�[�ōs�����߂��肦�Ȃ����ȗ�
//	if( OpeNtnetAddedInfo.PayMethod == 11 &&						// �U�֐��Z�v����M�ς�
//		 vl_frs.antipassoff_req &&									// ����g�p����
//		 OPECTL.Ope_mod != 2 &&										// �C�����Z�łȂ�
//		 !PayData.teiki.syu ){										// ��������g�p�̏ꍇ
//		BrandResRsltTbl.no = EC_BRAND_REASON_299;					// ���̑��̃G���[
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();											// �u�����h�I���f�[�^
//		return 1;							
//	}
//	
//	Hcard_dat = (m_Tcard*)&MAGred_HOUJIN[MAG_ID_CODE];
//	chg_ShiftIN_OUT((char*)Hcard_dat, (short)sizeof( m_Tcard ));
//	memcpy(&H_card_buf, Hcard_dat, sizeof( m_Tcard ));
//	for(i = 0; i < 16; i++){
//		if ( (H_card_buf.cre_mem_no[i] == ' ') 							// �I�[�X�y�[�X
//		  || (chk_num(H_card_buf.cre_mem_no[i]))) {						// '0'�`'9'
//			/* �����Ȃ� */
//		} else {
//			i = -1;			/* �G���[ */	
//			break;
//		}
//	}
//	if ((i < 0) 													// ����ԍ��̑S��������'0'�`'9'or ' '�łȂ�
//	||  (!chk_num_range( (char*)Hcard_dat->credit_limit, 4 ))) {	// �L������������'0'�`'9'�łȂ�
//		BrandResRsltTbl.no = EC_BRAND_REASON_203;						// ���f�[�^���܂܂��J�[�h��ǂݎ�肵���B
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);						// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();												// �u�����h�I���f�[�^
//		return 1;
//	}
//	
//	// ����z�𒴂��Ă���B�܂��͗��p�ݒ�łȂ��B		
//	ErrNum = chk_Hojin_use();
//	if (ErrNum != 0) {				
//		if (ErrNum == 4) {											// ���ϋ��z���@�l�J�[�h���Z���x�z�𒴂��Ă���B
//			BrandResRsltTbl.no = EC_BRAND_REASON_209;				// ���̑��̃G���[
//		} else {
//			BrandResRsltTbl.no = EC_BRAND_REASON_207;				// ���s�@�̃��b�Z�[�W�A�A�i�E���X�ɍ��킹�āA���R�R�[�h��207�ŕԂ��Ă��܂��B
//		}
//		ec_BrandResultDisp(BRANDNO_HOUJIN, BrandResRsltTbl.no);
//		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// �u�����h�I�����ʉ���NG
//		Ope_EcPayStart();
//		return 1;													// �u�����h�I���f�[�^
//	}
//		
//	H_card_buf.Ptr2_Chk = 0;						// ��g��J�[�h�`�F�b�N�L���͌Œ�
//	H_Card_Use = 1;
//	OPECTL.InquiryFlg = 1;							// �O���Ɖ�t���OON
//	
//	ryo_buf.credit.pay_ryo = ryo_buf.zankin;		// �v�Z�p
//	memset(ryo_buf.credit.card_no, 0, sizeof(ryo_buf.credit.card_no));
//	memcpy(ryo_buf.credit.card_no, H_card_buf.cre_mem_no, 16);
//	NTNETDOPA_Snd_Data148( 3 );						// �₢���킹�������҂��^�C�}�[�J�n
//	
//	// ���b�Z�[�W�\���@�Ɖ
//	ec_MessageAnaOnOff ( 2, 1 );
//	if( OPECTL.op_faz == 0 ){
//		OPECTL.op_faz = 1;							// ������
//	}
//	return 0;
//}

//[]----------------------------------------------------------------------[]
///	@brief			������t��
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EnableNyukin(void) 
{
	read_sht_opn();									// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
	cn_stat( 3, 2 );								// ���� & COIN�ē�����
	if( MIFARE_CARD_DoesUse ){						// Mifare���L���ȏꍇ
		op_MifareStart();							// Mifare�𗘗p�����i�ĊJ�j
	}
	if( OPECTL.credit_in_mony ){					// ��������ŸڼޯĂƂ̂���Ⴂ�����̏ꍇ
		in_mony ( COIN_EVT, 0 );					// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
		OPECTL.credit_in_mony = 0;					// ����Ⴂ�׸޸ر
	}
// MH810104 GG119202(S) �㌈�ρE���قȂ��u�����h�̃J�[�h�ǂݍ��ݎ��s���ɗ��p�\�}�̻��د����ĊJ���Ȃ�
	Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��(�T�C�N���b�N�\���͂�����)
// MH810104 GG119202(E) �㌈�ρE���قȂ��u�����h�̃J�[�h�ǂݍ��ݎ��s���ɗ��p�\�}�̻��د����ĊJ���Ȃ�
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�I�����ʖ��I������
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void BrandResultUnselected(void)
{
	// �I�����ʃG���[�̕\��
	ec_BrandResultDisp(BRANDNO_UNKNOWN, BrandResRsltTbl.no);
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�I�����ʗ��R�R�[�h�擾
//[]----------------------------------------------------------------------[]
/// @param			msg    			GetMessage()�̖߂�l
///	@return			���R�R�[�h
///                 �Y���R�[�h�Ȃ��̏ꍇ��EC_BRAND_REASON_299��Ԃ��B
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort GetBrandReasonCode(ushort msg)
{
	ushort code;

	switch(msg) {
//	case HOJIN_NEGA_RETRY:
//		if (Hojin_Retry_Flag) {
//			code = EC_BRAND_REASON_203;		// �J�[�h�̖₢���킹���ʂ���M�ł��Ȃ��B
//		} else {
//			code = EC_BRAND_REASON_208;		// �L���ȃJ�[�h�ł��邪�g�p�ł��Ȃ��B
//		}
//		break;
//	case HOJIN_NEGA_NG:
//		code = EC_BRAND_REASON_207;			// �l�K�J�[�h��ǂݎ�肵���B
//		break;
	default:
		code = EC_BRAND_REASON_299;			// ���̑�
		break;
	}
	return code;
}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| ���C���ޏ���(������X�V�p)                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_card_renewal( msg, mod )                             |*/
/*| PARAMETER    : msg : ү����                                            |*/
/*|                paymod : 0=�ʏ� 1=�C��                                  |*/
/*| RETURN VALUE : ret : 0=NG 1=OK                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_card_renewal( ushort msg, ushort paymod )
{
	short	ret, rd_snd, anm;
	short	dno;

	ret = 0;
	rd_snd = 2;														// ��o�����ړ�
	anm = 0;
	dno = 0;

	switch( MAGred[0] ){
		case 0x1a:													// APS���

			switch( OPECTL.CR_ERR_DSP = (char)al_pasck_renewal( (m_gtapspas *)MAGred ) ){
				case 0:												// OK
					rd_snd = 0;
					ret = 1;
					break;

				case 5:												// �������
					dno = 3;										// "     ���̃J�[�h�͖����ł�     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// �����؂�
				case 6:												// �����O
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				case 50:											// �X�V���ԊO
					dno = 11;										// " �X�V�\�Ȋ��Ԃł͂���܂��� "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;

	    default:
			OPECTL.CR_ERR_DSP = 8;									// "   ���� �g�p�s���ł� ����   "
			anm = AVM_CARD_ERR1;
			dno = 1;												// "    ���̃J�[�h�͎g���܂���    "
			if( rd_tik == 0 ){										// �������ۯ��ɒ��Ԍ�����?
				rd_snd = 13;										// ����o����(ذ�ް�E����������߂�)
			}
			break;
	}

	if( rd_snd ){
		opr_snd( rd_snd );
	}

	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}

	if( dno ){
		grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );						// �װ�\���͔��]�����ɂĕ\��
	}
	return( ret );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ��[������������                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hojuu_card( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*|                                                                        |*/
/*| RETURN VALUE : ret : OK/ NG                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-08-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	hojuu_card( void )
{
	short		ret = OK;	// �߂�l
	short		anm = 0;	// �ē�ү���ޔԍ�
	m_kakari	*crd_data;	// ���C�����ް��߲��
	long		pk_no;		// ���ԏ�m���D

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	// �W���J�[�h�`�F�b�N
	if(	( crd_data->kkr_idc != 0x41 )
			||
		( crd_data->kkr_did != 0x53 )
			||
		( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&
			( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
			||
		( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&
			( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){

		// �W���J�[�h�ȊO
		ret = NG;
		anm = AVM_CARD_ERR1;
	}
	else{
		// �W���J�[�h
		for( ; ; ){
			if( crd_data->kkr_type != 0x4b ){							// �J�[�h�^�C�v����[�J�[�h�H
				// ��[�J�[�h�łȂ��ꍇ
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}

			if( MAGred[MAG_GT_APS_TYPE] == 1 ){//GT�t�H�[�}�b�g
				pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +		// ���ԏ�m���D�擾
							( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
							( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}else{//APS�t�H�[�}�b�g
				pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// ���ԏ�m���D�擾
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}

			if( CPrmSS[S_SYS][1] != pk_no ){							// ���ԏ�m���D����{���ԏ�m���D�H
				// ���ԏ�m���D����v���Ȃ��ꍇ
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			//GT�t�H�[�}�b�g�`�F�b�N
			if( ( prm_get( COM_PRM,S_SYS,12,1,6 ) == 0 ) && (MAGred[MAG_GT_APS_TYPE] == 1) ||
				( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1 ) && (MAGred[MAG_GT_APS_TYPE] == 0) ){
				//GT�t�H�[�}�b�g���Ή��̏ꍇ
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			if( (crd_data->kkr_role < 0x30)||(crd_data->kkr_role > 0x34) ){
				// �������͈͊O�̏ꍇ
				ret = NG;
				anm = AVM_CARD_ERR1;
				break;
			}
			break;														// ��[�J�[�h�`�F�b�N�n�j
		}
	}
	if( DOWNLOADING() ){
		ret = NG;
		anm = AVM_CARD_ERR1;
	}
	if( anm ){
		ope_anm(  anm );								// Card Error Announce
	}
	if( ret == OK ){
		ret = (short)(crd_data->kkr_role - 0x30);
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| IC���ޏ���                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_iccard( msg, mod )                                   |*/
/*| PARAMETER    : msg : ү����                                            |*/
/*|                paymod : 0=�ʏ� 1=�C��                                  |*/
/*| RETURN VALUE : ret : ���Z����Ӱ��                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_iccard( ushort msg, ushort paymod )
{
	short	ret;
	short	anm, dno;

	ret = 0;
	anm = 0;
	dno = 0;

	switch( msg ){
		case IBK_MIF_A2_OK_EVT:										// Mifare�ް��Ǐo������

			switch( OPECTL.CR_ERR_DSP = (char)MifareDataChk() ){
				case 0:												// OK
					if (paymod == 0) {								// �ʏ퐸�Z�̏ꍇ
						MifareDataWrt();							// Mifare�����ݏ���
						break;
					}
					if( SKIP_APASS_CHK ){							// ��������߽OFF�ݒ�(��������������Ȃ�)�H
						OPECTL.Apass_off_seisan = 1;				// ��������߽OFF�ݒ��ԂŒ���g�p
					}
					al_pasck_set();							// PayData�ɒ��������������

					ryo_cal( 1, OPECTL.Pr_LokNo );					// �����v�Z
					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;							// ������
					}
					ret = in_mony( msg, paymod );
					MifareDataWrt();								// Mifare�����ݏ���

					if( ryo_buf.zankin ){							// ���Z���闿��������ꍇ
					if( !SyuseiEndKeyChk() ){
						read_sht_opn();										/* ���Cذ�ް���������� */
						cn_stat( 3, 2 );									/* ������ */
					}
					}
					break;

				case 5:												// �������
					dno = 3;										// "     ���̃J�[�h�͖����ł�     "
					anm = AVM_CARD_ERR3;
					break;

				case 3:												// �����؂�
				case 6:												// �����O
					dno = 2;										// "   ���̃J�[�h�͊����؂�ł�   "
					anm = AVM_KIGEN_END;
					break;

				case 99:											// HOST�֖⍇��
					read_sht_cls();										/* ���Cذ�ް������۰�� */
					cn_stat( 2, 2 );									/* �����s�� */
					NTNET_Snd_Data142( OPECTL.ChkPassPkno, OPECTL.ChkPassID );	// ������⍇���ް�(�ް����142)�쐬
					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
					blink_reg(7, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5]);		// "     ���΂炭���҂�������     "
					Lagtim( OPETCBNO, 6, (ushort)(prm_get( COM_PRM,S_NTN,31,2,1 )*50+1) );	// ��ϰ6(XXs)�N��(������⍇���Ď�)
					ret = 0;
					break;
				case 29:											// �⍇���^�C���A�E�gNG��
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
				case 4:
					dno = 19;										// "       ���o�ɕs��v�ł�       "
					anm = AVM_CARD_ERR1;
					break;

				default:
					dno = 1;										// "    ���̃J�[�h�͎g���܂���    "
					anm = AVM_CARD_ERR1;
					break;
			}
			break;
		case IBK_MIF_A4_OK_EVT:										// Mifare�ް�������OK
			if (paymod == 0) {										// �ʏ퐸�Z�̏ꍇ
				if( SKIP_APASS_CHK ){								// ��������߽OFF�ݒ�(��������������Ȃ�)�H
					OPECTL.Apass_off_seisan = 1;					// ��������߽OFF�ݒ��ԂŒ���g�p
				}
				al_pasck_set();								// PayData�ɒ��������������
				ryo_cal( 1, OPECTL.Pr_LokNo );						// �����v�Z

				if( OPECTL.op_faz == 0 ){
					OPECTL.op_faz = 1;								// ������
				}
				ac_flg.cycl_fg = 10;								// ����

				ret = in_mony( IBK_MIF_A2_OK_EVT, paymod );
				if( ryo_buf.zankin ){							// ���Z���闿��������ꍇ
				if( !SyuseiEndKeyChk() ){
					read_sht_opn();										/* ���Cذ�ް���������� */
					cn_stat( 3, 2 );									/* ������ */
				}
				}
				if (ope_imf_GetStatus() == 2) {
					if (is_paid_remote(&PayData) < 0) {
					// �U�֐��Z�����{�i������p�w���j�Ȃ炱���Ōv��
						ope_ifm_FurikaeCalc( 0 );
						ret = in_mony(IFMPAY_FURIKAE, 0);
						// ���łɐ��Z���������Ă���΂O�~�U�ւȂ̂�ret�͕ω����Ȃ��i�㏑��ok�j
					}
				}
			}
			break;
	}

	if( OPECTL.CR_ERR_DSP != 0 && OPECTL.CR_ERR_DSP != 99 ){											// ���C��������G���[���� 
		Op_StopModuleWait_sub( 2 );						// �d�q�}�̍Ċ������v��
	}

	if( anm ){
		ope_anm( anm );								// Card Error Announce
	}

	if( dno ){
		grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[dno] );						// �װ�\���͔��]�����ɂĕ\��
		if (OPECTL.Pay_mod == 0) {
			OPECTL.MIF_CR_ERR = 1;		// Mifare���ް��װ
		}
// MH810105(S) MH364301 �C���{�C�X�Ή�
		OPECTL.f_CrErrDisp = 1;			// �G���[�\����
// MH810105(E) MH364301 �C���{�C�X�Ή�
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 1���Z����                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set( wflg, typ )                                |*/
/*| PARAMETER    : wflg : 0 = �ʏ�, 1 = ��d                               |*/
/*|              : typ  : 0 = ���Z����, 1 = ���Z���~                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PayData_set( uchar wflg, uchar typ )
{
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	ushort	num;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort	i;
	uchar	isSet = 0;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
	struct clk_rec *pTime;
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
// MH810105(S) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
	date_time_rec	pay_time;
// MH810105(E) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�

	if( OPECTL.f_KanSyuu_Cmp )										// �����W�v���{�ς݁i�{�֐���Call���ꂽ�Ƃ������Ɓj
		return;

	PayData.WFlag = wflg;											// �󎚎�ʂP�i0:�ʏ� 1:���d�j
	PayData.chuusi = typ;											// �󎚎�ʂQ�i0:�̎��� 1:���Z���~ 2:�a��؁j
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if( PayData.teiki.update_mon ){									// �X�V���Z?
//		PayData.WPlace = 9999;										// �X�V���͎Ԏ��ԍ�9999�Ƃ���B
//	}else if( ryo_buf.pkiti == 0xffff ){							// ������Z?
//		PayData.WPlace = LOCKMULTI.LockNo;							// �ڋq�p���Ԉʒu�ԍ�
//// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//	}else if( !OPECTL.Pr_LokNo ){									// ������Z�H
//		PayData.WPlace = OPECTL.Op_LokNo;
//// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//	}else{
//		num = ryo_buf.pkiti - 1;
//		PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// �ڋq�p���Ԉʒu�ԍ�
//	}
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// �@�B��
	PayData.Seisan_kind = 0;										// ���Z���(�����Œ�)
	if( typ == 0 ){													// ���Z����
		CountGet( PAYMENT_COUNT, &PayData.Oiban );					// �ǔ�
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else{															// ���Z���~
		CountGet( CANCEL_COUNT, &PayData.Oiban );					// �ǔ�
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
// MH810100(S) K.Onodera  2020/01/16 �Ԕԃ`�P�b�g���X(���Z���~���A�����͂Ȃ��������Ƃɂ���)
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// �������N���A
		ryo_buf.waribik	   = 0L;									// �����z�N���A
		ryo_buf.fee_amount = 0L;									// �������� �����z�N���A
		ryo_buf.tim_amount = 0L;									// ���Ԋ��� ���Ԑ��N���A
		ryo_buf.kazei = ryo_buf.tyu_ryo;							// �ېőΏۊz �����z�l��
		NTNET_Data152_DiscDataClear();								// ���Z���f�[�^�p�����ڍ׃G���A�N���A
// MH810100(E) K.Onodera  2020/01/16 �Ԕԃ`�P�b�g���X(���Z���~���A�����͂Ȃ��������Ƃɂ���)
	}
	PayData.TInTime.Year = car_in_f.year;							// ���� �N
	PayData.TInTime.Mon  = car_in_f.mon;							//      ��
	PayData.TInTime.Day  = car_in_f.day;							//      ��
	PayData.TInTime.Hour = car_in_f.hour;							//      ��
	PayData.TInTime.Min  = car_in_f.min;							//      ��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	PayData.TInTime_Sec  = car_in_f.sec;							//      �b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	PayData.TOutTime.Year = car_ot_f.year;							// �o�� �N
	PayData.TOutTime.Mon  = car_ot_f.mon;							//      ��
	PayData.TOutTime.Day  = car_ot_f.day;							//      ��
	PayData.TOutTime.Hour = car_ot_f.hour;							//      ��
	PayData.TOutTime.Min  = car_ot_f.min;							//      ��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	PayData.TOutTime_Sec  = car_ot_f.sec;							//      �b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
// GG124100(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// 	if( OpeNtnetAddedInfo.PayClass == 1 ){	// �Đ��Z
	if ( (OpeNtnetAddedInfo.PayClass == 1) &&						// �Đ��Z
		 (OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME) ) {	// LCD������ɏ���M
// GG124100(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
		PayData.BeforeTPayTime.Year = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;	// ���Z �N
		PayData.BeforeTPayTime.Mon  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;	//      ��
		PayData.BeforeTPayTime.Day  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;	//      ��
		PayData.BeforeTPayTime.Hour = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;	//      ��
		PayData.BeforeTPayTime.Min  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;	//      ��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		PayData.BeforeTPayTime_Sec  = RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;	//      �b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	}
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
// MH810105(S) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//	PayData.WTaxRate = Disp_Tax_Value( &PayData.TOutTime );			// �K�p�ŗ�
//// MH810105(E) MH364301 �C���{�C�X�Ή�
	pTime = GetPayStartTime();
	// ���Z�J�n�̓��t��date_time_rec�ɕϊ�
	pay_time.Year = pTime->year;
	pay_time.Mon  = pTime->mont;
	pay_time.Day  = pTime->date;
	pay_time.Hour  = pTime->hour;
	pay_time.Min  = pTime->minu;
	PayData.WTaxRate = Disp_Tax_Value( &pay_time );			// �K�p�ŗ�
// MH810105(E) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
	if( ryo_buf.ryo_flg < 2 ){										// ���Ԍ����Z����
		PayData.WPrice = ryo_buf.tyu_ryo;							// ���ԗ���
	}else{
		PayData.WPrice = ryo_buf.tei_ryo;							// �������
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	/*** �����ނ͗����v�Z���ŉ��Z�ς� ***/
	PayData.syu = (char)(ryo_buf.syubet + 1);						// ���Ԏ��
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)
// GG124100(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// 	if( typ == 1 ){													// ���Z���~
	if ( (typ == 1) &&												// ���Z���~
		 (OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME) ) {	// LCD������ɏ���M
// GG124100(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
		PayData.WPrice = RTPay_Data.crd_info.dtZaishaInfo.ParkingFee;	// ���ԗ��������񗿋��v�Z���̗����ɖ߂�
		NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
		PayData.syu = RTPay_Data.crd_info.dtZaishaInfo.shFeeType;		// ������ʂ���M�l�ɖ߂�(RTPay_Data�̍ݎԏ��_������ʂ͎Q�Ƃׁ݂̂̈A��M�l�̂܂�)
	}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.BeforeTwari = ryo_buf.dis_tim;							// �O�񎞊Ԋ����z
	PayData.BeforeRwari = ryo_buf.dis_fee;							// �O�񗿋������z
	PayData.BeforePwari = ryo_buf.dis_per;							// �O�񁓊������z
	PayData.Wtax = ryo_buf.tax;										// �����
	PayData.WInPrice = ryo_buf.nyukin;								// �����z
	NTNET_Data152Save((void *)(&PayData.WInPrice), NTNET_152_INPRICE);
	PayData.WChgPrice = ryo_buf.turisen;							// �ޑK
	NTNET_Data152Save((void *)(&PayData.WChgPrice), NTNET_152_CHGPRICE);
// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
//	PayData.WTotalPrice = ryo_buf.nyukin - ryo_buf.turisen;			// �����̎����z
	PayData.WTotalPrice = ryo_buf.nyukin - (ryo_buf.turisen + ryo_buf.kabarai);
// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
	NTNET_Data152Save((void *)(&PayData.WTotalPrice), NTNET_152_CASHPRICE);
	PayData.WFusoku = ryo_buf.fusoku;								// ���ߕs�����z
	NTNET_Data152Save((void *)(&PayData.WFusoku), NTNET_152_HARAIMODOSHIFUSOKU);
	memcpy ( &PayData.credit, &ryo_buf.credit, sizeof(credit_use));
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)�����v�Z
	PayData.zenkai = ryo_buf.zenkai;								// �O��̎��z
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)�����v�Z
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	PayData.WBillAmount = Billingcalculation(&PayData);				// �����z
	if (typ == 0 || typ == 1) {
		PayData.WTaxPrice = TaxAmountcalculation(&PayData , typ);	// �ېőΏۊz
		if (PayData.WTaxPrice > PayData.WBillAmount) {
			// ���������Ȃ��̏ꍇ�A�����z���ېőΏۊz�����邱�Ƃ�����̂ŁA
			// �����z�ɍ��킹��
			PayData.WTaxPrice = PayData.WBillAmount;
		}
	}
	else {
		PayData.WTaxPrice = 0;
	}
	memcpy ( &PayData.RegistNum, &RegistNum, sizeof(RegistNum));
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j

	if( OPECTL.Apass_off_seisan ){									// ��������߽OFF���Z����
		PayData.teiki.Apass_off_seisan = 1;
	}
	PayData.PayCalMax = ntnet_nmax_flg;								// �ő嗿���z������t���O�Z�b�g
	PayData.in_coin[0]	= (uchar)ryo_buf.in_coin[0];	// ��������(10�~)
	PayData.in_coin[1]	= (uchar)ryo_buf.in_coin[1];	// ��������(50�~)
	PayData.in_coin[2]	= (uchar)ryo_buf.in_coin[2];	// ��������(100�~)
	PayData.in_coin[3]	= (uchar)ryo_buf.in_coin[3];	// ��������(500�~)
	PayData.in_coin[4]	= (uchar)ryo_buf.in_coin[4];	// ��������(1000�~)
	PayData.out_coin[0] = (uchar)ryo_buf.out_coin[0] + ryo_buf.out_coin[4];	// ���o����(10�~) + ���o����10�~(�\�~)
	PayData.out_coin[1] = (uchar)ryo_buf.out_coin[1] + ryo_buf.out_coin[5];	// ���o����(50�~) + ���o����50�~(�\�~)
	PayData.out_coin[2] = (uchar)ryo_buf.out_coin[2] + ryo_buf.out_coin[6];	// ���o����(100�~)+ ���o����100�~(�\�~)
	PayData.out_coin[3] = (uchar)ryo_buf.out_coin[3];	// ���o����(500�~)
	NTNET_Data152Save((void *)(&PayData.in_coin[0]), NTNET_152_INCOIN);
	NTNET_Data152Save((void *)(&PayData.out_coin[0]), NTNET_152_OUTCOIN);
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	for( i=0; (i < DETAIL_SYU_MAX) && (0 != PayData.DetailData[i].ParkingNo); i++ ){
		if( PayData.DetailData[i].DiscSyu == NTNET_FUTURE ||
			PayData.DetailData[i].DiscSyu == NTNET_AFTER_PAY ){
			isSet = 1;	// ���ɌĂяo���ꂽ
		}
	}
	// �Z�b�g�H
	if( i != DETAIL_SYU_MAX && isSet == 0 ){
		memset( &PayData.DetailData[i], 0, sizeof(PayData.DetailData[i]) );
		if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
			// �������Z���A�x���\��z�Z�b�g�L��H
			if( g_PipCtrl.stRemoteFee.Type == 0 && g_PipCtrl.stRemoteFee.FutureFee ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// ���ԏꇂ
				PayData.DetailData[i].DiscSyu = NTNET_FUTURE;									// ������ʁF������Z�\��z
				PayData.DetailData[i].uDetail.Future.Reserve1 = 1;								// 1�Œ�
				PayData.DetailData[i].uDetail.Future.Total = g_PipCtrl.stRemoteFee.FutureFee;	// �x���\��z
			}
			// ������Z�H
			else if( g_PipCtrl.stRemoteFee.Type == 1 ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// ���ԏꇂ
				PayData.DetailData[i].DiscSyu = NTNET_AFTER_PAY;								// ������ʁF������Z�z
				PayData.DetailData[i].uDetail.AfterPay.Reserve1 = 1;							// 1�Œ�
				PayData.DetailData[i].uDetail.AfterPay.Total = g_PipCtrl.stRemoteFee.Price;		// �x���z
			}
		}
		else if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
			// �x���\��z�Z�b�g�L��H
			if( g_PipCtrl.stRemoteTime.FutureFee ){
				PayData.DetailData[i].ParkingNo = CPrmSS[S_SYS][1];								// ���ԏꇂ
				PayData.DetailData[i].DiscSyu = NTNET_FUTURE;									// ������ʁF������Z�\��z
				PayData.DetailData[i].uDetail.Future.Reserve1 = 1;								// 1�Œ�
				PayData.DetailData[i].uDetail.Future.Total = g_PipCtrl.stRemoteTime.FutureFee;	// �x���\��z
			}
		}
		// PayData�Z�b�g�L��H
		if( PayData.DetailData[i].ParkingNo ){
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DPARKINGNO, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DSYU, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DNO, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DCOUNT, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DISCOUNT, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DINFO1, i);
			NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[i], NTNET_152_DINFO2, i);
		}
	}
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
// GG124100(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
	if ( OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME ) {			// LCD������ɏ���M
// GG124100(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
		PayData.shubetsu = RTPay_Data.shubetsu;							// ���O/�������

		// ���Z�J�n�������擾
		pTime = GetPayStartTime();

		PayData.TUnpaidPayTime.Year = pTime->year;						// ���Z �N
		PayData.TUnpaidPayTime.Mon  = pTime->mont;						//      ��
		PayData.TUnpaidPayTime.Day  = pTime->date;						//      ��
		PayData.TUnpaidPayTime.Hour = pTime->hour;						//      ��
		PayData.TUnpaidPayTime.Min  = pTime->minu;						//      ��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		PayData.TUnpaidPayTime_Sec  = pTime->seco;						//      �b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
// GG124100(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
	}
// GG124100(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j
	// ���Z���Ƒ��M�f�[�^�쐬�������ł͂Ȃ��̂ŁA���M�f�[�^�쐬���Ɏ����W�v�Ȃǔ������邩������Ȃ�
	// ���Z���O�ɐ��Z���ɑO��T���v���������āA���M�f�[�^�쐬����sky�̑O��T���v�������Q�Ƃ��Ȃ��悤�ɂ���
	memcpy(&PayData.Before_Ts_Time, &sky.tsyuk.OldTime, sizeof(sky.tsyuk.OldTime));
	PayData.Before_Ts_Time.Sec = 0;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j
}

// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
//[]----------------------------------------------------------------------[]
///	@brief			QR�̎��ؕ\��
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			R.Endo
///	@date			Create	:	2024/12/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	QrReciptDisp( void )
{
	ushort	dsp_tm;	
	char	qrcode[QRCODE_RECEIPT_SIZE + 4];

	// RT�̎��؃f�[�^�o�^
	RTReceipt_LogRegist_AddOiban();

	// QR�̎��؃f�[�^�쐬
	MakeQRCodeReceipt(qrcode, sizeof(qrcode));

	// QR�̎��؃|�b�v�A�b�v�\��
	lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR�̎���

	// QR�R�[�h���s�ē��\���^�C�}�[�J�n
	dsp_tm = (ushort)prm_get(COM_PRM, S_LCD, 96, 2, 1);
	Lagtim(OPETCBNO, TIMERNO_QR_RECEIPT_DISP, (dsp_tm * 50));

	// ���Z�����ē��^�C�}�[����
	if ( LagChkExe(OPETCBNO, 1) ) {
		if ( r_zero_call ) {
			dsp_tm = (ushort)prm_get(COM_PRM, S_TYP, 129, 4, 1);
		} else {
			dsp_tm = (ushort)prm_get(COM_PRM, S_TYP, 128, 4, 1);
		}
		Lagtim(OPETCBNO, 1, (dsp_tm * 50));
	}
}
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j

/*[]----------------------------------------------------------------------[]*/
/*| �̎������s����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ryo_isu( no )                                           |*/
/*| PARAMETER    : no : 0 = �̎���(�ޑK�s������)���s                       |*/
/*|                   : 1 = �̎���(�ޑK�s���L��)���s                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���o�Ɏ����ɌŒ�lcar_in_f,car_ot_f���g�p����                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ryo_isu( char no )
{
	MsgBuf	*msb;
	T_FrmReceipt	ReceiptPreqData;								// ���̎��؈󎚗v����ү�����ް�
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// // GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// // GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j

	if( OPECTL.RECI_SW != (char)-1 ){
		PayData_set( 0, 0 );										// 1���Z����
	}

// MH810105(S) MH364301 �C���{�C�X�Ή�
//	if( no == 1 || ReceiptChk() == 0 ){								// ڼ�Ĕ��s��?
// GG129000(S) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//	if(( no == 1 || ReceiptChk() == 0)
	if(( no == 1 || QRIssueChk(1) != 0 || ReceiptChk() == 0)		// QR���Ԍ����s����H ���V�[�g���s�ݒ肠��H
// GG129000(E) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 	&& (Ope_isPrinterReady() == 1) ){								// ڼ�Ĕ��s��?
	&& (IS_ERECEIPT || (Ope_isPrinterReady() == 1)) ){				// ڼ�Ĕ��s��?
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// MH810105(E) MH364301 �C���{�C�X�Ή�

// MH810105(S) MH364301 �C���{�C�X�Ή�
		if (IS_INVOICE) {											// ���޲�?
			if( OPECTL.RECI_SW == (char)-1 ){						// �������ł̗̎��ؔ��s
				if (OPECTL.f_DelayRyoIsu != 2) {
					// �C���{�C�X�ݒ肠��ŃW���[�i���󎚊������Ă��Ȃ��ꍇ��
					// �������s���Ȃ�
					return;
				}
				else if (OPECTL.f_ReIsuType != 0) {
					// �̎��؍Ĕ��s�v����t���͉������s���Ȃ�
					return;
				}
			}
			else{													// �����ł͂Ȃ�
				if (Ope_isJPrinterReady() == 0) {
					// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł�
					// �����Ȃ������ꍇ�͗̎��؂��󎚂��Ȃ�
					return;
				}
				else if (OPECTL.f_DelayRyoIsu != 2) {
					// �W���[�i���󎚊�����Ƀ��V�[�g�󎚂��s��
					OPECTL.f_DelayRyoIsu = 1;						// �W���[�i���󎚊����҂�
					return;
				}
			}
		}
// MH810105(E) MH364301 �C���{�C�X�Ή�

// GG129004(S) R.Endo 2024/12/11 #7560 �d�q�̎��؂𔭍s����ݒ�ŗ̎��؃{�^������������QR�R�[�h��\��������ƁA�̎��؎��o������LED���_������
// 		LedReq( CN_TRAYLED, LED_ON );								// ��ݎ�o�����޲��LED ON
// GG129004(E) R.Endo 2024/12/11 #7560 �d�q�̎��؂𔭍s����ݒ�ŗ̎��؃{�^������������QR�R�[�h��\��������ƁA�̎��؎��o������LED���_������

		ryo_buf.ryos_fg = 1;										// �̎������s���

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		if ( prm_get(COM_PRM, S_RTP, 58, 1, 1) == 1 ) {	// 17-0058 �̎��؈󎚐� �d�q�̎���
			if ( OPECTL.f_KanSyuu_Cmp ) {	// �����W�v���{
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// 				// RT�̎��؃f�[�^�o�^
// 				RTReceipt_LogRegist_AddOiban();
// 
// 				// QR�̎��؃f�[�^�쐬
// 				MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// 
// 				// QR�̎��؃|�b�v�A�b�v�\��
// 				lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR�̎���
				// QR�̎��ؕ\��
				QrReciptDisp();
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
			} else {
				// �����W�v��RT�̎��؃f�[�^�̍쐬���K�{�̂��ߎ��{�O�ł���Ύ��{��ɗ\��
				OPECTL.f_eReceiptReserve = 1;
			}
		} else {
// GG129004(S) R.Endo 2024/12/11 #7560 �d�q�̎��؂𔭍s����ݒ�ŗ̎��؃{�^������������QR�R�[�h��\��������ƁA�̎��؎��o������LED���_������
			LedReq(CN_TRAYLED, LED_ON);								// ��ݎ�o�����޲��LED ON
// GG129004(E) R.Endo 2024/12/11 #7560 �d�q�̎��؂𔭍s����ݒ�ŗ̎��؃{�^������������QR�R�[�h��\��������ƁA�̎��؎��o������LED���_������
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

		if( ( msb = GetBuf() ) == NULL ){
			if( OPECTL.RECI_SW != (char)-1 ){
				kan_syuu();											// ���Z�����W�v
			}
			return;
		}
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		if( ryo_stock == 1 ){										// �X�g�b�N��
			// �X�g�b�N����̏ꍇ���V�[�g�f�[�^�͂��łɑ��M���Ă��邽��
			// �󎚊J�n�̂ݍs��
			MsgSndFrmPrn(PREQ_STOCK_PRINT, R_PRI, 0);				// �f�[�^�������ݏI���i�󎚊J�n�j
			ryo_stock = 0xff;										// �X�g�b�N�󎚊���
		}
		else if( ryo_stock == 0 ){									// �X�g�b�N�Ȃ�
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129002(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
		memset(&ReceiptPreqData, 0, sizeof(ReceiptPreqData));
// GG129002(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
		msb->msg.command = PREQ_RYOUSYUU;							// ���M����ށF�̎��؈󎚗v��
		ReceiptPreqData.prn_kind = R_PRI;							// �������ʁFڼ��
// MH810105(S) MH364301 �C���{�C�X�Ή��i�W���[�i���󎚃^�C�~���O�ύX�j
		// UT-4000�ɍ��킹�ă��V�[�g�󎚂̂ݗv������
		// �W���[�i���󎚂�Log_regist�ŗv������
//		if( OPECTL.RECI_SW != (char)-1 && PrnJnlCheck() == ON ){
//			//	�ެ���������ڑ�����̏ꍇ
//			switch(CPrmSS[S_RTP][1]){	// �ެ��و󎚏����H
//				case	0:
//				case	1:
//				case	2:
//					// �ެ��ق͂O�~�����L�^
//					ReceiptPreqData.prn_kind = RJ_PRI;
//					break;
//				case	3:
//				case	4:
//				case	5:
//					// �ެ��ق͒��ԗ����O�~���ȊO�L�^
//					if( PayData.WPrice != 0 ){
//						//	���ԗ������O
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//
//				case	6:
//				case	7:
//				case	8:
//					// �ެ��ق͌����̎��O�~���ȊO�L�^
//					if( PayData.WTotalPrice != 0 ){
//						//	�����̎��z���O
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//				case	9:
//				case   10:
//				case   11:
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
////					if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
//					if( PayData.WTotalPrice != 0 ||
//						SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
//						EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//						//	�����̎��z���O OR �d�q�}�̂ɂ�鐸�Z���s��ꂽ�ꍇ
//						ReceiptPreqData.prn_kind = RJ_PRI;
//					}
//					break;
//				default:
//					ReceiptPreqData.prn_kind = R_PRI;
//			}
//		}
// MH810105(E) MH364301 �C���{�C�X�Ή��i�W���[�i���󎚃^�C�~���O�ύX�j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//		Pri_Work[0].Receipt_data = PayData;
//		ReceiptPreqData.prn_data = &Pri_Work[0].Receipt_data;	// �ް��߲�����
		memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
		ReceiptPreqData.prn_data = &Cancel_pri_work;			// �ް��߲�����
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		ReceiptPreqData.reprint = OFF;								// �Ĕ��s�׸�ؾ�āi�ʏ�j

		memcpy( &msb->msg.data, &ReceiptPreqData, sizeof(T_FrmReceipt) );	// ���M�ް����

		PutMsg( PRNTCBNO, msb );									// ���������ֈ󎚗v��ү���ޑ��M

// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
		ryo_inji = 1;												// �̎��؈󎚒�
// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

		sky.tsyuk.Ryosyuu_pri_cnt += 1L;							// �̎������s����+1
		if(OPECTL.Pr_LokNo != 0xffff){
			syusei[OPECTL.Pr_LokNo-1].infofg |= SSS_RYOUSYU;
		}
	}

	if( OPECTL.RECI_SW != (char)-1 ){
		kan_syuu();													// ���Z�����W�v
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �̎��ؔ��s�^�s���菈��                                             |*/
/*|																		   |*/
/*|	PayData�Z�b�g�O�� ryo_buf �����Ĕ���B								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReceiptChk2( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0=�̎��؈󎚉�, 1=�̎��؈󎚕s��                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2007/04/03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	ReceiptChk( void )
{
	uchar	ret = 1;												// ڼ�Ĉ󎚕s��

// MH810105(S) MH364301 �C���{�C�X�Ή�
//	switch( CPrmSS[S_RTP][1] ){										// ڼ�Ĉ󎚏���
	long rec_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );		// 17-0001�̒l��ϐ��ɕۑ�

	rec_pri_jadge = invoice_prt_param_check( rec_pri_jadge );		// �p�����[�^�̕ϊ����s���B

	switch( rec_pri_jadge ){										// ڼ�Ĉ󎚏���
// MH810105(E) MH364301 �C���{�C�X�Ή�
		case 0:
		case 3:
		case 6:
		case 9:
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
		case 12:
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
			// ڼ�Ă�0�~�����L�^
			ret = 0;												// ڼ�Ĉ󎚉�
			break;

		case 1:
		case 4:
		case 7:
		case 10:
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
		case 13:
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
			if( ((ryo_buf.ryo_flg < 2) && (ryo_buf.tyu_ryo != 0)) ||
				((ryo_buf.ryo_flg >= 2) && (ryo_buf.tei_ryo != 0)) ){
				// ���ԗ�����0
				ret = 0;											// ڼ�Ĉ󎚉�
			}
			break;

		case 2:
		case 5:
		case 8:
		case 11:
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
		case 14:
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
			// ڼ�Ă͌����̎�0�~���ȊO�L�^
			//if( PayData.WTotalPrice != 0 ){
			if( (ryo_buf.nyukin - ryo_buf.turisen) != 0){
				// �����̎��z��0
				ret = 0;											// ڼ�Ĉ󎚉�
			}
			break;

		default:
			ret = 1;												// ڼ�Ĉ󎚕s��
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~����                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chu_isu( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	chu_isu( void )
{
	if( Chu_Syu_Status )
		return;

	if(OPECTL.op_faz == 3){											// ����{�^�����������ꂽ�ꍇ
		PayData_set( 0, 1 );										// 1���Z����
		azukari_isu(1);												// ���Z���~���̗a��ؔ��s
		cyu_syuu();													// ���Z���~�W�v
		Chu_Syu_Status = 1;
	}else{
		PayData_set( 0, 0 );										// 1���Z����
		azukari_isu(0);												// ���Z���̗a��ؔ��s
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�a��ؔ��s����
//[]----------------------------------------------------------------------[]
///	@param[in]		cancel 	: 0:���Z, 1:���Z���~
///	@return			None
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/08/25
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	azukari_isu(uchar cancel)
{
	T_FrmAzukari	AzukariPreqData;									// ���a��؈󎚗v����ү�����ް�
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//	ePOP_DISP_KIND	DispCode;	// �|�b�v�A�b�v�\�����
//	uchar	DispStatus;			// �\���X�e�[�^�X
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

	if( ryo_buf.fusoku != 0 ){
		if(CPrmSS[S_SYS][44] != 2){									//�a��ؐݒ肪�Q�̏ꍇ���̓W���[�i�������ɏo�͂���
			LedReq( CN_TRAYLED, LED_ON );							// ��ݎ�o�����޲��LED ON

			memset(&AzukariData, 0, sizeof(AzukariData));
			AzukariData.WFlag = 0;									// �󎚎�ʂP�i0:�ʏ� 1:���d�j
			AzukariData.chuusi = cancel;							// ���Z���~���ǂ���
			if( !OPECTL.multi_lk )									// ������Z�⍇�����ȊO?
				AzukariData.Lok_No = OPECTL.Pr_LokNo;				// ���b�N���uNo
			else
				AzukariData.Lok_No = (ushort)(LOCKMULTI.LockNo%10000);	// ���b�N���uNo

			AzukariData.WPlace = OPECTL.Op_LokNo;					// �ڋq�p���Ԉʒu�ԍ�
			AzukariData.Kikai_no = (uchar)CPrmSS[S_PAY][2];			// �@�B��
			
// MH810100(S) Y.Yamauchi 2020/02/17 �Ԕԃ`�P�b�g���X�i�a��؈󎚏C���j
			AzukariData.CarSearchFlg = PayData.CarSearchFlg;		// �Ԕ�/�����������
			memcpy( AzukariData.CarSearchData, PayData.CarSearchData, sizeof(AzukariData.CarSearchData) );		// �Ԕ�(����)
			memcpy( AzukariData.CarNumber, PayData.CarNumber, sizeof(AzukariData.CarNumber) );		// �Ԕ�(���Z)
			memcpy( AzukariData.CarDataID, PayData.CarDataID, sizeof(AzukariData.CarDataID) );		// �Ԕԃf�[�^ID
// MH810100(E) Y.Yamauchi 2020/02/17 �Ԕԃ`�P�b�g���X�i�a��؈󎚏C���j
			CountGet( DEPOSIT_COUNT, &AzukariData.Oiban );			// �ǔ�
			AzukariData.Seisan_kind = 0;							// ���Z���(�����Œ�)
			AzukariData.Oiban2 = PayData.Oiban;						// ���Z�^���~�ǔ�
			AzukariData.TTime.Year = PayData.TOutTime.Year;			// ���Z �N
			AzukariData.TTime.Mon  = PayData.TOutTime.Mon;			//      ��
			AzukariData.TTime.Day  = PayData.TOutTime.Day;			//      ��
			AzukariData.TTime.Hour = PayData.TOutTime.Hour;			//      ��
			AzukariData.TTime.Min  = PayData.TOutTime.Min;			//      ��
			AzukariData.WTotalPrice = ryo_buf.dsp_ryo;				// �����z		�����v���i���ԗ����|���������j

			AzukariData.WInPrice = ryo_buf.nyukin;					// �����z		�����a��
// �d�l�ύX(S) K.Onodera 2016/11/07 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//			AzukariData.WChgPrice = ryo_buf.turisen - ryo_buf.fusoku;	// ���ߋ��z	�����ߊz���i�ޑK���z�|���o�s���z�j
			AzukariData.WChgPrice = (ryo_buf.turisen+ryo_buf.kabarai) - ryo_buf.fusoku;	// ���ߋ��z	�����ߊz���i�ޑK���z+�ߕ����|���o�s���z�j
// �d�l�ύX(E) K.Onodera 2016/11/07 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
			AzukariData.Syusei = 0;									// �C�����Z����
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//			if(OPECTL.Pay_mod == 2){	//�C�����Z���͒ޑK���z�ł͂Ȃ��ďC�����ߊz�Ƃ���
//				AzukariData.Syusei = 1;								// �C�����Z����
//				if(( PayData.MMTwari )&&(!(PayData.Zengakufg&0x02))){	// �C���������z�̕��߂���
//					AzukariData.Syusei = 2;							// �C�������ߗL��
//					AzukariData.WChgPrice = PayData.MMTwari			// �C�����ߋ��z(���߂��ׂ����z-���o���Ȃ��������z)
//												- ryo_buf.fusoku;
//				}
//			}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
			AzukariData.WFusoku = ryo_buf.fusoku;					// ���ߕs�����z	�����o�s���z

			AzukariPreqData.prn_kind = R_PRI;						// �������ʁFڼ��
			AzukariPreqData.prn_data = (Azukari_data *)&AzukariData;// �a��؈��ް����߲�����

			queset( PRNTCBNO, PREQ_AZUKARI, sizeof( T_FrmAzukari ), &AzukariPreqData );	// �a��ؔ��s

			sky.tsyuk.Azukari_pri_cnt += 1L;						// �a��ؔ��s��+1
// MH810100(S) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//			// �߯�߱��ߕ\���v��(���Z���ɔ�������\���v��) & �Ή����鉹���ē��𗬂�
//			//	code;						status															����				ү����
//			//	1:�a��ؔ��s(���Z������)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)	0,2=35�^1,3��33		0,1=22/2,3=24
//			//	2:�a��ؔ��s(�����߂���)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)	0,2=35�^1,3��33		26
//			// cancel	0 = ���Z���̗a��ؔ��s		1 = ���Z���~���̗a��ؔ��s
//			// ��OPECTL.RECI_SW		// 1 = �̎������ݎg�p	0 = �̎������ݖ��g�p
//			// ��OPECTL.Fin_mod		// 0 = ���z		1 = �ނ薳��		2 = �ނ�L��	3 = ү��װ(�ނ�L��)
//			// 1:�a��ؔ��s(���Z������)	2:�a��ؔ��s(�����߂���)
//			// ���Z���̗a��ؔ��s
//			if (cancel == 0) {
//				DispCode = POP_AZUKARI_PAY;
//			}
//			// ���Z���~���̗a��ؔ��s
//			else {
//				DispCode = POP_AZUKARI_RET;
//			}
//			// 0 = ���z, 1 = �ނ薳��, 2 = �ނ�L��, 3 = ү��װ(�ނ�L��)
//			switch (OPECTL.Fin_mod) {
//				case 0:
//				case 1:
//				default:
//					// 1 = �̎������ݎg�p	0 = �̎������ݖ��g�p
//					if (OPECTL.RECI_SW == 0) {
//						DispStatus = 0;		// �ޑK�Ȃ�
//					}
//					else {
//						DispStatus = 2;		// �ޑK�Ȃ�(�̎���)
//					}
//					break;
//				case 2:
//				case 3:
//					if (OPECTL.RECI_SW == 0) {
//						DispStatus = 1;		// �ޑK����
//					}
//					else {
//						DispStatus = 3;		// �ޑK����(�̎���)
//					}
//					break;
//			}
//			// �|�b�v�A�b�v�\���v��(�a��ؔ��s)
//			lcdbm_notice_dsp( DispCode, DispStatus );
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
			azukari_popup(cancel);
// MH810100(E) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ��t�����s����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : uke_isu( op_no, pr_no, re_pri )                         |*/
/*| PARAMETER    : op_no : �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999)  |*/
/*|                pr_no : ���������p���Ԉʒu�ԍ�(1�`324)                  |*/
/*|                re_pri: ���s���(0:�ʏ�^1�F�Ĕ��s�^2�F�����e���񔭍s)  |*/
/*| RETURN VALUE : ret   : 0=���sOK  1=���s�ς�NG                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-09-06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	uke_isu( unsigned long op_no, unsigned short pr_no, unsigned char re_pri )
{
	T_FrmUketuke	UketukePreqData;								// ����t���󎚗v����ү�����ް�

	unsigned char		remote;
	remote = (unsigned char)(re_pri & 0xf0);
	re_pri &= 0x0f;
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/04 ��t���͏�������Ɋ֌W�Ȃ����s���A�Ĕ��s�Ƃ���
//	if( (FLAPDT.flp_data[pr_no-1].issue_cnt < (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 3)) || re_pri == 1 ){	// ���x�����ȉ� or �Ĕ��s
	if( (FLAPDT.flp_data[pr_no-1].issue_cnt < (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 3)) || re_pri == 1 || remote == 0x20 ){	// ���x�����ȉ� or �Ĕ��s or Parkipro�v��
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/04 ��t���͏�������Ɋ֌W�Ȃ����s���A�Ĕ��s�Ƃ���
		FLAPDT.flp_data[pr_no-1].issue_cnt++;						// ���ԏؖ������s�񐔍X�V
		FLAPDT.flp_data[pr_no-1].uketuke = 1;						// ���s�ςݾ��
		LedReq( CN_TRAYLED, LED_ON );								// ��ݎ�o�����޲��LED ON

		memset( &UketukeData, 0, sizeof( Uketuke_data ) );

		UketukeData.Lok_No = pr_no;
		UketukeData.WPlace = op_no;									// ���Ԉʒu��
		UketukeData.Kikai_no = (uchar)CPrmSS[S_PAY][2];				// �@�B��
		UketukeData.PrnCnt = FLAPDT.flp_data[pr_no-1].issue_cnt;	// ���ԏؖ������s��
		if( re_pri == 1 ){											// �Ĕ��s?
			UketukeData.Oiban = UketukeNoBackup[pr_no-1];			// �Ĕ��s�ǔ�
			memcpy( &UketukePreqData.PriTime, &CLK_REC, sizeof(date_time_rec) );	// �Ĕ��s�����i���ݓ����j���
			UketukeData.ISTime.Year = FLAPDT.flp_data[pr_no-1].u_year;	// ���s �N
			UketukeData.ISTime.Mon  = FLAPDT.flp_data[pr_no-1].u_mont;	//      ��
			UketukeData.ISTime.Day  = FLAPDT.flp_data[pr_no-1].u_date;	//      ��
			UketukeData.ISTime.Hour = FLAPDT.flp_data[pr_no-1].u_hour;	//      ��
			UketukeData.ISTime.Min  = FLAPDT.flp_data[pr_no-1].u_minu;	//      ��
		}else{														// �ʏ픭�s�y�у����e�i���X���񔭍s
			UketukeData.Oiban = CountRead_Individual( INFO_COUNT );		// �ǔ�
			UketukeNoBackup[pr_no-1] = UketukeData.Oiban;			// ���s�ǔ��ޯ�����
			memcpy( &UketukeData.ISTime, &CLK_REC, sizeof(date_time_rec) );	// ���s �N��������
			FLAPDT.flp_data[pr_no-1].u_year = UketukeData.ISTime.Year;	// ���s �N�ۑ�
			FLAPDT.flp_data[pr_no-1].u_mont = UketukeData.ISTime.Mon;	//      ���ۑ�
			FLAPDT.flp_data[pr_no-1].u_date = UketukeData.ISTime.Day;	//      ���ۑ�
			FLAPDT.flp_data[pr_no-1].u_hour = UketukeData.ISTime.Hour;	//      ���ۑ�
			FLAPDT.flp_data[pr_no-1].u_minu = UketukeData.ISTime.Min;	//      ���ۑ�
		}
		if( re_pri != 0 ){												// �ʏ�ȊO�͌W��No.����
			if (remote == 0)
				UketukePreqData.kakari_no = OPECTL.Kakari_Num;			// �W��No.
			else
				UketukePreqData.kakari_no = 99;							// �W��No.
		}
		UketukeData.TTime.Year = FLAPDT.flp_data[pr_no-1].year;		// ���� �N
		UketukeData.TTime.Mon  = FLAPDT.flp_data[pr_no-1].mont;		//      ��
		UketukeData.TTime.Day  = FLAPDT.flp_data[pr_no-1].date;		//      ��
		UketukeData.TTime.Hour = FLAPDT.flp_data[pr_no-1].hour;		//      ��
		UketukeData.TTime.Min  = FLAPDT.flp_data[pr_no-1].minu;		//      ��
		if(( (prm_get( COM_PRM,S_TYP,63,1,1 ) == 2) ||
			 (prm_get( COM_PRM,S_TYP,63,1,1 ) == 4) ) &&
			 (remote == 0) )
		{
			UketukeData.Pword = FLAPDT.flp_data[pr_no-1].passwd;	// �߽ܰ��
		}

		if (re_pri == 1) {											// �Ĕ��s
			// �Ĕ��s�Ȃ�ڼ�Ă̂�
			UketukePreqData.prn_kind = R_PRI;						// �������ʁFڼ��
		} else {
			// �Ĕ��s�łȂ���ΐݒ�Ɉˑ�
			if (prm_get(COM_PRM, S_PRN, 17, 1, 4) == 1) {
				UketukePreqData.prn_kind = RJ_PRI;					// �������ʁFڼ��&�ެ���
			} else {
				UketukePreqData.prn_kind = R_PRI;					// �������ʁFڼ��
			}
		}
		UketukePreqData.prn_data = (Uketuke_data *)&UketukeData;	// ��t�����ް����߲�����
		UketukePreqData.reprint  = re_pri;							// ���s��ʾ��
		queset( PRNTCBNO, PREQ_UKETUKE, sizeof( T_FrmUketuke ), &UketukePreqData );	// ��t�����s

		if( re_pri != 1 ){											// �Ĕ��s�ȊO?
			CountUp_Individual( INFO_COUNT );						// ��t�����s�ǔ�+1

			sky.tsyuk.Uketuke_pri_Tcnt += 1L;						// ��t�����s��+1
			loktl.tloktl.loktldat[pr_no-1].Uketuke_pri_cnt += 1;	// ���Ԉʒu�ʎ�t�����s��+1

		}
		LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// �d�q���ϒ��~�̎��ؔ��s���̎��o����LED������ϰ
		return( 0 );												// ���sOK
	}else{
		return( 1 );												// ���s�ς�NG
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ��t���A�߽ܰ�ޓo�^�`�F�b�N����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : uke_isu( pr_no )                                        |*/
/*| PARAMETER    : pr_no : ���������p���Ԉʒu�ԍ�(1�`324)                  |*/
/*|                                                                        |*/
/*|                                                                        |*/
/*| RETURN VALUE : ret   : 0=���sNG  1=���sOK                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-09-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	pri_time_chk( unsigned short pr_no, uchar type )
{
ushort	ltim;

	struct	CAR_TIM	car_in_time;					//���Ɏ��Ԋi�[
	struct	CAR_TIM	pri_chk_time;					//���݁i���s�j���Ԋi�[
	short	pritim;									//���Ɂ`���s���Ԋi�[
	
	pri_chk_time.year	 = CLK_REC.year;			// ���� �N�ۑ�
	pri_chk_time.mon	 = CLK_REC.mont;			//      ���ۑ�
	pri_chk_time.day	 = CLK_REC.date;			//      ���ۑ�
	pri_chk_time.hour	 = CLK_REC.hour;			//      ���ۑ�
	pri_chk_time.min	 = CLK_REC.minu;			//      ���ۑ�

	car_in_time.year	= FLAPDT.flp_data[pr_no-1].year;		// ���� �N
	car_in_time.mon		= FLAPDT.flp_data[pr_no-1].mont;		//      ��
	car_in_time.day		= FLAPDT.flp_data[pr_no-1].date;		//      ��
	car_in_time.hour	= FLAPDT.flp_data[pr_no-1].hour;		//      ��
	car_in_time.min		= FLAPDT.flp_data[pr_no-1].minu;		//      ��

	pritim = (short)ec71( &car_in_time,&pri_chk_time );			//���Ɂ`���s�Ԃ̎��Ԏ擾

	if( type == 0 ){				// ��t�����s
		ltim = (ushort)prm_get( COM_PRM,S_TYP,99,3,4 );
	}else{							// �Ïؔԍ��o�^
		ltim = (ushort)prm_get( COM_PRM,S_TYP,99,3,1 );
	}
	if( (ltim != 0) && ( pritim > ltim ) ){				//���s�\�`�F�b�N
		return( 0 );												// ���sNG
	}else{
		return( 1 );												// ���sOK
	}

}

/*[]----------------------------------------------------------------------[]*/
/*| ����ݽ�p�߽ܰ�ޖ��̖����擾����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mtpass_get( uchar inp, uchar *pass )                    |*/
/*| PARAMETER    : inp   : �߽ܰ�ޓ��͌���                                 |*/
/*|              : *pass : �߽ܰ���ް��߲��                                |*/
/*| RETURN VALUE : ret : ����  1 = �W������                                |*/
/*|                            2 = �Ǘ��ґ���                              |*/
/*|                            3 = �Z�p������(��ɋZ�p����p)               |*/
/*|                           -1 = �߽ܰ�ޕs��v                           |*/
/*| �����l��                                                               |*/
/*|    �������Ұ��ŗ^����ꂽ�߽ܰ�ނ��ݒ�ð��قɑ��݂����ꍇ�A            |*/
/*|    OPECTL.Kakari_Num���߽ܰ�ނɑΉ�����W��No.��Ă���                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const unsigned char		SYS_PAS_ENG[8] = { 'C', 4, 3, 9, 1, 5, 3, 1 };
const unsigned char		SYS_PAS_DEV[9] = { 'C', 3, 1, 1, 3, 1, 5, 5, 7 };

char	mtpass_get( uchar inp, uchar *pass )
{
	short	s_wk;
	char	i;
	char	yakuwari = (char)-1;
	char	kakari_no;
	uchar	c_key = OFF;
	char	passlevel = 0;

	if( memcmp( SYS_PAS_DEV, pass, 9 ) == 0 ){
		// �����߽ܰ�ށ��b�R�P�P�R�P�T�T�V

		yakuwari	= 4;												// �J���ґ�������
		kakari_no	= 99;												// �W��No.���
		passlevel	= (char)-1;											// ���b�N�������Ȃ�
	}
	else if( memcmp( SYS_PAS_ENG, &pass[1], 8 ) == 0 ){					// �߽ܰ��(C4391531)��v?

		// �����߽ܰ�ށ��b�S�R�X�P�T�T�V

		yakuwari	= 3;												// �Z�p����������
		kakari_no	= 99;												// �W��No.���
		passlevel	= (char)-1;											// ���b�N�������Ȃ�
	}
	else{
		// �����߽ܰ�ށ��b�S�R�X�P�T�R�P

		if( inp == 0 ){
			// �߽ܰ�ޓ��͂Ȃ�
			for( i = 0; i < 10; i++ ){									// �߽ܰ�ސݒ�ð��ٌ���
				if(	(CPrmSS[S_PSW][(i*2)+1] == 0L) 
						&&
					(CPrmSS[S_PSW][(i*2)+2] != 0) ){					// �߽ܰ�ނȂ��i0000�j���@�������O�H
					
					// �߽ܰ�ނȂ��i0000�j���@�������O
					if( (yakuwari < (char)CPrmSS[S_PSW][(i*2)+2]) || yakuwari == 0xff ){
						// �O�����߽ܰ�ނȂ�(0000)�̖�����荂�������̏ꍇ

						yakuwari	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 1);	// ��������
						passlevel	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 2);	// ��������
						kakari_no	= (char)(i+1);						// �W��No.����
					}
				}
			}
		}
		else{

			// �߽ܰ�ޓ��͂���

			s_wk = 0;
			for( i = 5; i < 9; i++ ){
				if( pass[i] == 'C' ){
					// �P�`�S���ڂɁu�b�v�L�[�����͂���Ă���ꍇ
					c_key = ON;											// �u�b�v�����͂���
				}
				s_wk *= 10;
				s_wk += (short)pass[i];
			}

			if( c_key == OFF ){											// �P�`�S���ڂɁu�b�v�������͂���Ă��Ȃ��ꍇ�̂��߽ܰ�ޔ�r���s��

				for( i = 0; i < 10; i++ ){								// �߽ܰ�ސݒ�ð��ٌ���
					if(	(CPrmSS[S_PSW][(i*2)+1] == 0L)
							||
						(CPrmSS[S_PSW][(i*2)+2] == 0)){
						// �߽ܰ�ސݒ�Ȃ�(0000)�@�܂��́@�������O�@�̏ꍇ
						continue;
					}
					if( s_wk == (short)CPrmSS[S_PSW][(i*2)+1] ){		// �߽ܰ�ވ�v?
						// �߽ܰ�ވ�v
						yakuwari	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 1);	// ����ݽ�̖���(����)�擾
						passlevel	= (char)prm_get( COM_PRM, S_PSW, (i*2)+2, 1, 2);	// ��������
						kakari_no	= (char)(i+1);						// �߽ܰ�ނɑΉ������W��No.���
						break;
					}
				}
			}
		}
	}
	if( yakuwari != (char)-1 ){
		// �߽ܰ�ނ��ݒ�ð��قɑ��݂����ꍇ
		OPECTL.Kakari_Num = kakari_no;									// �W��No.���
		OPECTL.PasswordLevel = passlevel;
	}
	return( yakuwari );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������o�Ɏ���ð��ٓǏo��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblRead()                                   |*/
/*| PARAMETER    : pas : ���ԏꇂ                                          |*/
/*|                pid : ID = 1 �` 12000                                   |*/
/*|                date_time_rec : ð��قɓo�^����Ă���o�Ɏ���[out]      |*/
/*| RETURN VALUE : ret :  0 = ð��ٓo�^����, 1 = ð��ٓo�^�L��             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	PassExitTimeTblRead( ulong pas, ushort pid, date_time_rec *exittime )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort	i;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	char	ret = 0;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	for( i=0; i<pas_extimtbl.Count; i++ ){
//		if( pas == pas_extimtbl.PassExTbl[i].ParkNo && pid == pas_extimtbl.PassExTbl[i].PassId ){
//			// ���ԏꇂ�ƒ����ID������
//			memcpy( exittime, &pas_extimtbl.PassExTbl[i].PassTime, sizeof( date_time_rec ) );
//			ret = 1;
//			break;
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������o�Ɏ���ð��ُ�����                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblWrite()                                  |*/
/*| PARAMETER    : pas : ���ԏꇂ                                          |*/
/*|                pid : ID = 1 �` 12000                                   |*/
/*|                date_time_rec : ð��قɓo�^����o�Ɏ���                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PassExitTimeTblWrite( ulong pas, ushort pid, date_time_rec *exittime )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	/* ������o�Ɏ����e�[�u���ɊY���f�[�^�����݂��Ă���΍폜 */
//	PassExitTimeTblDelete(pas, pid);
//	
//	/* �����f�[�^ALL0�Ȃ�폜�̂ݎ��s */
//	if (exittime->Year != 0 || exittime->Mon != 0 || exittime->Day != 0 ||
//		exittime->Hour != 0 || exittime->Min != 0) {
//		if (pas_extimtbl.Count >= PASS_EXTIMTBL_MAX) {
//			/* �e�[�u���ő匏���o�^�ς݂̏ꍇ�́A�ŌẪf�[�^(index=0)���폜���� */
//			PassExitTimeTblDelete((ulong)pas_extimtbl.PassExTbl[0].ParkNo, pas_extimtbl.PassExTbl[0].PassId);
//		}
//		/* �Ō���Ƀf�[�^��o�^���� */
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].PassTime = *exittime;
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].ParkNo = pas;
//		pas_extimtbl.PassExTbl[pas_extimtbl.Count].PassId = pid;
//		pas_extimtbl.Count++;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������o�Ɏ���ð��ٍ폜                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PassExitTimeTblDelete()                                 |*/
/*| PARAMETER    : pas : ���ԏꇂ                                          |*/
/*|                pid : ID = 1 �` 12000                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PassExitTimeTblDelete( ulong pas, ushort pid )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort	i, hit;
//
//	hit = 0;
//
//	for( i=0; i<pas_extimtbl.Count; i++ ){
//		if( pas == pas_extimtbl.PassExTbl[i].ParkNo && pid == pas_extimtbl.PassExTbl[i].PassId ){
//			// ���ԏꇂ�ƒ����ID������
//			hit = 1;
//			break;
//		}
//	}
//
//	if( hit ){
//		memmove( &pas_extimtbl.PassExTbl[i],
//				 &pas_extimtbl.PassExTbl[i+1],
//				 sizeof( PASS_EXTBL )*(PASS_EXTIMTBL_MAX-i) );		// �폜(���)
//		memset( &pas_extimtbl.PassExTbl[PASS_EXTIMTBL_MAX-1], 0, sizeof( PASS_EXTBL ) );
//		pas_extimtbl.Count--;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}
typedef struct {
	unsigned long	mnt_st;
	unsigned long	mnt_ed;
	unsigned long	nt_st;
	unsigned long	now;
	unsigned long	hikaku;
	unsigned long	jikan_wk[3];	// ���[�J���C�Y���ԁi��ʂQbyte=day ���ʂQbyte=fun�j
	char			eigyo_wk[3];	// ���ꂼ��̖߂�l�i�[
	char			cstat_wk[3];	// �x�Ƃ̗v���i�[
} nrmlyz;
nrmlyz NRMLYZ;
//[]-----------------------------------------------------------------------[]
// �m�[�}���C�Y���ƃm�[�}���C�Y���ŁA�m�[�}���C�Y�������쐬����				|
//																			|
//	norm_day(in) : �m�[�}���C�Y��											|
//  norm_min(in) : �m�[�}���C�Y��											|
//  Return:																	|
//			���16�r�b�g���� ����16�r�b�g����								|
//																			|
//  T.Okamoto	2006/08/31	Create											|
//[]-----------------------------------------------------------------------[]
//  �召��r�̂��߂̕ϊ��ł���A���m�ȍ����͂���ł͌v�Z�ł��Ȃ�			|
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]
ulong time_nrmlz ( ushort norm_day, ushort norm_min )
{
	ulong		retval;
	retval = (ulong)norm_day;
	retval <<= 16;
	retval += (ulong)(norm_min);
	return retval;
}
/*[]----------------------------------------------------------------------[]*/
/*| �c�Ɓ^�x������                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opncls()                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 1 = �c��(Open) / 2 = �x��(Close)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	opncls( void )
{
	int		cnt;			// �J�E���^
	int		i,target;
	short	sts;
// MH322914 (s) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)
	int		bufferfull = 0;
// MH322914 (e) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)

	sts = 0;

// MH322914 (s) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)
	/** �o�b�t�@FULL���� **/
	if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&				// NT-NET ���Z�ް����M�ޯ̧FULL���x��? ����
	   (_is_ntnet_remote())){ 									// ���uNT-NET�̐ݒ�L��
		if(IsErrorOccuerd(prm_get(COM_PRM, S_CEN, 51, 1, 3) ? ERRMDL_LANIBK : ERRMDL_FOMAIBK, ERR_RAU_PAYMENT_BUFERFULL)) {
			// �o�b�t�@�t�����������Ă���΋x��
			bufferfull = 1;
		}
		else {
			// �o�b�t�@�t��������ԂȂ甭�����Ă���Ȃ甭����ԂɕύX����
			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);
		}
	}
// MH322914 (e) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
//	else if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&			// NT-NET ���Z�ް����M�ޯ̧FULL���x��? ����
	if(( prm_get( COM_PRM,S_SSS,41,1,1 ) == 2 )&&				// NT-NET ���Z�ް����M�ޯ̧FULL���x��? ����
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
	   (_is_ntnet_normal())){ 									// NT-NET�̐ݒ�L��
		if(IsErrorOccuerd(ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL)) {
			// �o�b�t�@�t�����������Ă���΋x��
			bufferfull = 1;
		}
		else {
			// �o�b�t�@�t��������ԂŃo�b�t�@�t����ԂȂ甭����ԂɕύX����
			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_NTNET);
		}
	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

	/** ����� **/
	if( opncls_TrbClsChk() ){									// �x�ƂƂ�������ٗv���L��
		sts = 2;
		CLOSE_stat = 7;
	}
// MH322914 (s) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)
//	/** �ޯ̧FULL **/
//	if(( prm_get( COM_PRM,S_NTN,41,1,1 ) == 2 )&&				// NT-NET ���Z�ް����M�ޯ̧FULL���x��? ����
//	   (_is_ntnet_remote())){ 									// ���uNT-NET�̐ݒ�L��
//		if(IsErrorOccuerd(prm_get(COM_PRM, S_CEN, 51, 1, 3) ? ERRMDL_LANIBK : ERRMDL_FOMAIBK, ERR_RAU_PAYMENT_BUFERFULL)) {
//			// �o�b�t�@�t�����������Ă���΋x��
//			sts = 2;
//			CLOSE_stat = 8;										// NT-NET ���M�ޯ̧FULL
//		}
//		else {
//			// �o�b�t�@�t��������ԂȂ甭�����Ă���Ȃ甭����ԂɕύX����
//			Log_CheckBufferFull(TRUE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);
//		}
//	}
	else if( bufferfull == 1 ) {
		sts = 2;
		CLOSE_stat = 8;										// NT-NET ���M�ޯ̧FULL
	}
// MH322914 (e) kasiyama 2016/07/11 �g���u���M���ŋx�ƒ��ɋ����c�Ƃł���̂��C��[���ʃo�ONo.1249](MH341106)
	/** �����x�ƐM�� **/
	else if( OPECTL.opncls_kyugyo == 1 ){						// �����x�Ƃ̐ړ_���͗L��
		sts = 2;
		CLOSE_stat = 10;										// �����c�x��(�M��)
	}
	/** �����c�ƐM�� **/
	else if( OPECTL.opncls_eigyo == 1 ){						// �����c�Ƃ̐ړ_���͗L��
		CLOSE_stat = 10;										// �����c�x��(�M��)
	}
	/** հ�ް����ݽ **/
	else if( PPrmSS[S_P01][2] != 0 ){							// հ�ް����ݽ�ɂ����鋭���c�x�Ɛ؊��L��
		/*** �����c�x�� ***/
		sts = (short)PPrmSS[S_P01][2];
		CLOSE_stat = 1;											// �����c�x��
	}
	/** ��݋��ɖ��t **/
	// ���������Z���[�h���ͺ�ݖ��t�ł��x�ƂƂ��Ȃ�
	else if( (cinful() == -1) ){								// ��݋��ɖ��t�Ŕ�����Ӱ�ނł��Ȃ�
		sts = 2;
		CLOSE_stat = 4;											// ��݋��ɖ��t
	}
	/** �������ɖ��t **/
	else if( (prm_get( COM_PRM,S_SYS,38,1,1 ) == 1) &&			// �������ɖ��t���x�Ƃɂ���ݒ��
			 (notful() == -1) ){								// �������ɖ��t
			sts = 2;
			CLOSE_stat = 3;										// �������ɖ��t
	}
	/** �ޑK�؂� **/
	else if( (prm_get( COM_PRM,S_SYS,38,1,2 ) == 1) &&			// �ޑK�؂ꎞ�x�Ƃɂ���ݒ��
			 (Ex_portFlag[EXPORT_CHGEND] == 1)){ 				// �ޑK�؂�
			sts = 2;
			CLOSE_stat = 6;										// �x�Ɨ��R���ޑK�؂�
	}
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//	/** ���V�[�g�^�W���[�i�����؂ꎞ�ɋx�� **/
//	else if( ( IS_INVOICE &&									// �C���{�C�X�K�p��
//			  prm_get( COM_PRM, S_SYS, 16, 1, 1 ) == 1) &&		// ���V�[�g�^�W���[�i�����؂ꎞ�ɋx�Ƃɂ���ݒ��
//			 ((Ope_isPrinterReady() == 0) || 					// ���V�[�g�o�͕s�܂���
//			  (Ope_isJPrinterReady() == 0))) {					// �W���[�i��(�d�q�W���[�i��)�o�͕s��
//		sts = 2;
//		CLOSE_stat = 15;										// �x�Ɨ��R�����V�[�g�^�W���[�i�����؂�
//	}
//// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
	else{


	/*** �ʐM�ɂ��c�x�� or �c�x�Ǝ��Ԏw�� ***/

		memset(&NRMLYZ,0,sizeof(NRMLYZ));
		//�ݒ莞�Ԃ����S�m�[�}���C�Y
		NRMLYZ.mnt_st = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
								   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][40]/100), (short)(CPrmSS[S_SYS][40]%100) ) );
		NRMLYZ.mnt_ed = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
								   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][41]/100), (short)(CPrmSS[S_SYS][41]%100) ) );
		//NTNET�w�����Ԃ����S�m�[�}���C�Y
		NRMLYZ.nt_st = time_nrmlz( FLAGPT.receive_rec.flg_ocd, FLAGPT.receive_rec.flg_oct );
		//���ݎ��Ԃ����S�m�[�}���C�Y
		NRMLYZ.now = time_nrmlz ( CLK_REC.ndat, CLK_REC.nmin );
		//�ݒ莞�ԑ召�ϊ�
		if( NRMLYZ.mnt_st > NRMLYZ.mnt_ed ){ //EX>now 3:00 ,Start 8:00 , End 4:00
			if( NRMLYZ.mnt_ed > NRMLYZ.now ){
				//�J�n��O���ɂ���
				NRMLYZ.mnt_st = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
										 (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][40]/100), (short)(CPrmSS[S_SYS][40]%100) ) );
			}
		}else if( NRMLYZ.mnt_st < NRMLYZ.mnt_ed ){ //EX> now 1:00 , Start 2:00 , End 10:00 
			if( NRMLYZ.mnt_st > NRMLYZ.now ){
				//�I����O���ɂ���
				NRMLYZ.mnt_ed = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
									     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_SYS][41]/100), (short)(CPrmSS[S_SYS][41]%100) ) );
			}
		}else if( NRMLYZ.mnt_st == NRMLYZ.mnt_ed ){
			NRMLYZ.mnt_st = 0L;
			NRMLYZ.mnt_ed = 0L;
		}

		// ���ꂼ�ꌻ�݂��ߋ��̎w��� eigyo_wk ���̑��@�Ɋi�[���A�L���Ȑ��� cnt ����.
		cnt = 0;
		if( (NRMLYZ.mnt_st != 0L) && (NRMLYZ.mnt_st <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.mnt_st;
			NRMLYZ.eigyo_wk[cnt] = 1;									//�c��		(�߂�l)
			NRMLYZ.cstat_wk[cnt] = 0;									//�c�ƒ�	(CLOSE_stat)
			cnt ++;
		}
		if( (NRMLYZ.mnt_ed != 0L) && (NRMLYZ.mnt_ed <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.mnt_ed;
			NRMLYZ.eigyo_wk[cnt] = 2;									//�x��		(�߂�l)
			NRMLYZ.cstat_wk[cnt] = 2;									//�c�Ǝ��ԊO(CLOSE_stat)
			cnt ++;
		}
		if( (NRMLYZ.nt_st != 0L) && (NRMLYZ.nt_st <= NRMLYZ.now) ){
			NRMLYZ.jikan_wk[cnt] = NRMLYZ.nt_st;
			NRMLYZ.eigyo_wk[cnt] = (char)FLAGPT.receive_rec.flg_ock ;	//NTNET�̎w��(�߂�l)
			// NT-NET�ɂ��w���������c�x��
			NRMLYZ.cstat_wk[cnt] = 11;
			cnt ++;
		}

		// ��ԑ傫������ ( == ��Ԍ��݂���߂�)�@�w��͂Ȃɂ�
		if( cnt != 0 ){
			for(i=0; i<cnt; i++){
				if( NRMLYZ.hikaku <= NRMLYZ.jikan_wk[i] ){
					NRMLYZ.hikaku = NRMLYZ.jikan_wk[i];
					target = i;
				}
			}
			sts = (short)NRMLYZ.eigyo_wk[target];
			CLOSE_stat = NRMLYZ.cstat_wk[target];
		}

		// �c�Ƃ̏ꍇ�A�ʐM�ɂ�鋭���c�Ƃ��c�Ǝ��ԓ����̔����׸ނ�Ă���
		if (sts != 2) {
			if (CLOSE_stat == 11) {
				// NT-NET�ɂ�鋭���w��
				OPEN_stat = OPEN_NTNET;
			} else {
				// �c�Ǝ��ԓ�
				OPEN_stat = OPEN_TIMESET;
			}
		}
	}
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(�c�x�Ǝ��Ԏw�肵���Ƃ��ɋx�ƂƂȂ�Ȃ��o�O�C���ڐA)
//	if (isEC_USE() != 0) {						// ���σ��[�_�ڑ��ݒ肠��
//	// ���x�ƂƂ͂��Ȃ�
//		if ((prm_get(COM_PRM, S_ECR, 1, 6, 1) == 0)
//		&&	(prm_get(COM_PRM, S_ECR, 2, 1, 6) == 0)
//		&&	(prm_get(COM_PRM, S_ECR, 2, 2, 1) == 0)) {
//		// �L���u�����h�ݒ�Ȃ�
//			err_chk((char)jvma_setup.mdl, ERR_EC_SETTEI_BRAND_NG, 1, 0, 0);			// E3264:�װ۸ޓo�^�i�o�^�j
//			Suica_Rec.suica_err_event.BIT.BRANDSETUP_NG = 1;						// �ݒ����׸ނ�o�^
//		}
//	}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(�c�x�Ǝ��Ԏw�肵���Ƃ��ɋx�ƂƂȂ�Ȃ��o�O�C���ڐA)
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�

	if( sts != 2 ){
		sts = 1;
		CLOSE_stat = 0;
	}
	/****************************************/
	/* CLOSE_stat = 0:�c�ƒ�				*/
	/*              1:�����c�x��			*/
	/*              2:�c�Ǝ��ԊO			*/
	/*              3:�������ɖ��t			*/
	/*              4:��݋��ɖ��t			*/
	/*              5:����ذ�ް�E��			*/
	/*              6:�ޑK�؂�				*/
	/*              7:���ү��ʐM�s�Ǒ�		*/
	/*              8:NT-NET ���M�ޯ̧FULL	*/
	/*              9:�ׯ��/ۯ����u�蓮Ӱ��	*/
	/*             10:�M���ŋx�Ǝw��		*/
	/*             11:�ʐM�ŋx�Ǝw��		*/
// MH810105(S) MH364301 �C���{�C�X�Ή�
	/*             15:ڼ��/�ެ��َ��؂�		*/
// MH810105(E) MH364301 �C���{�C�X�Ή�
	/****************************************/

	return( sts );
}

/*[]----------------------------------------------------------------------[]*/
/*| ����ِM���v���i�װ/�װсj�ł̉c�Ɓ^�x������                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�װ/�װѺ��ޕ\�ɂāA�x�ƂƂ���װ/�װт̔����L����Ԃ��B			   |*/
/*|	�������ŋx�ƂƂ��鍀�ڂ̔����󋵂� Err_cls, Alm_cls�ɾ�Ă���Ă���A   |*/
/*|	���̑��̏����t���ڂ͂����őS����������B							   |*/
/*|																		   |*/
/*|	�x�Ɨv���͗D�揇�ʂ����蕡�G�Ȕ���ƂȂ�B							   |*/
/*| �{�֐��ł͏�L opncls() �֐��̈ꕔ�𔲂��o�������̂ł���A������D�� |*/
/*|	�x�̍��������i���ɖ��t�j�ƒႢ�����i�ޑK�؂�j�������ΏۊO�Ƃ���B	   |*/
/*|	�܂�NT-NET�ʐM�ޯ̧Full������ٗv���ł͂Ȃ��̂ŁA������ΏۊO�Ƃ���B   |*/
/*|																		   |*/
/*|	��������t�x�ƂƂȂ�װ/�װт� ��L�Ɏ������ΏۊO���ڂƂȂ�B		   |*/
/*| �{�֐��͏��������t�x�Ɨv���𑝂₵���ꍇ�̂��߂ɗp�ӂ���B			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opncls_TrbClsChk()                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 2 = �x�Ɨv���L��A0=�Ȃ�                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-12-09                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	short	opncls_TrbClsChk( void )
{
	/* �������x�ƴװ/�װє����� */
	if( Err_cls || Alm_cls ){
		return	(short)2;
	}

	return	(short)0;
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| ����������֘A�̴װ�̗L����m��                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LprnErrChk()                                            |*/
/*| PARAMETER    : char offline = 0 : ��ײݴװ���������Ȃ�                 |*/
/*|                             = 1 : ��ײݴװ����������                   |*/
/*| RETURN VALUE : ret : 0=�װ�Ȃ�  1=�װ����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-03-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	LprnErrChk( char offline )
{
	if( ERR_CHK[mod_lprn][ERR_LPRN_COMFAIL]		||					// �ʐM�^�C���A�E�g
		ERR_CHK[mod_lprn][ERR_LPRN_CUTTER]		||					// �J�b�^�[�G���[
		ERR_CHK[mod_lprn][ERR_LPRN_HEADOPEN]	||					// �w�b�h�I�[�v��
		ERR_CHK[mod_lprn][ERR_LPRN_HEAD]		||					// �w�b�h�G���[
		ERR_CHK[mod_lprn][ERR_LPRN_SENSOR]		||					// �Z���T�G���[
		ERR_CHK[mod_lprn][ERR_LPRN_BUFF_FULL]	||					// �o�b�t�@�I�[�o�[
		ERR_CHK[mod_lprn][ERR_LPRN_MEDIA]		||					// ���f�B�A�G���[
		ERR_CHK[mod_lprn][ERR_LPRN_OTHER]		||					// ���̑��̃G���[
		ALM_CHK[ALMMDL_SUB][ALARM_LBPAPEREND]	||					// ���x�����؂�
		ALM_CHK[ALMMDL_SUB][ALARM_LBRIBBONEND] )					// ���x�����{���؂�
	{
		return( 1 );
	}
	if( offline ){
		if( ERR_CHK[mod_lprn][ERR_LPRN_OFFLINE] ){					// �ʐM�I�t���C��
			return( 1 );
		}
	}
	return( 0 );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ���ٔ��s����-1                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LabelCountDown( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-04-20                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	LabelCountDown( void )
{
	if( PPrmSS[S_P01][10] > 0L ){
		PPrmSS[S_P01][10]--;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�����M���o��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayEndSig()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	PayEndSig( void )
{
	uchar	set;

	if( ryo_buf.ryo_flg >= 2 ){										// ����g�p?
		set = (uchar)prm_get( COM_PRM,S_PAS,(short)(4+10*(PayData.teiki.syu-1)),1,2 );	// ��������̐M���o�͐ݒ�
	}else{
		set = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+(ryo_buf.syubet*6)),1,2 );		// ��ʖ��̐M���o�͐ݒ�
	}

	if(( set == 0 )||( set == 2 )){
		OutSignalCtrl( SIG_OUTCNT1, SIG_ONOFF );					// ���Z�����M��1�o��
	}
	if(( set == 1 )||( set == 2 )){
		OutSignalCtrl( SIG_OUTCNT2, SIG_ONOFF );					// ���Z�����M��2�o��
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ǔԶ���+1 �ʒǔԗp                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CountUp_Individual( kind )                              |*/
/*| PARAMETER    : kind = �ǔԂ̎��                                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : yanagawa                                                |*/
/*| Date         : 2012/10/25                                              |*/
/*| UpDate       : �֐�����ύX��������                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	CountUp_Individual( uchar kind )
{
	PPrmSS[S_P03][kind]++;
	if( PPrmSS[S_P03][kind] > 999999L ){
		PPrmSS[S_P03][kind] = 1L;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ǔԶ��ēǏo�� �ʒǔԗp                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CountRead_Individual( kind )                            |*/
/*| PARAMETER    : kind = �ǔԂ̎��                                       |*/
/*| RETURN VALUE : ret  = �e��ǔ�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : yanagawa                                                |*/
/*| Date         : 2012/10/25                                              |*/
/*| UpDate       : �֐�����ύX��������                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	CountRead_Individual( uchar kind )
{
	return(ulong)( PPrmSS[S_P03][kind] );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�f�[�^�擾�֐�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| RETURN VALUE : PAS_TBL*                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
PAS_TBL *GetPassData( ulong ParkingNo, ushort PassId)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	long index;
//	PAS_TBL *pcdpt;
//
//	if( PassId > 0 ){
//		PassId--;
//	}else{
//		return(NULL);
//	}
//
//	if( (ulong)CPrmSS[S_SYS][65] == ParkingNo ){
//		if( CPrmSS[S_SYS][61] )		index = CPrmSS[S_SYS][61];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - 1 ) ){							// ð��ه@�͈͓̔�?
//			pcdpt = &pas_tbl[PassId];									// ð��ه@
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][66] == ParkingNo ){
//		if( CPrmSS[S_SYS][62] )		index = CPrmSS[S_SYS][62];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][61] ) ){			// ð��هA�͈͓̔�?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][61] - 1 ];			// ð��هA
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][67] == ParkingNo ){
//		if( CPrmSS[S_SYS][63] )		index = CPrmSS[S_SYS][63];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][62] ) ){			// ð��هB�͈͓̔�?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][62] - 1 ];			// ð��هB
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][68] == ParkingNo ){
//		if( PassId < (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){		// ð��هC�͈͓̔�?
//			pcdpt = &pas_tbl[ PassId + CPrmSS[S_SYS][63] - 1 ];			// ð��هC
//		}else{
//			return(NULL);
//		}
//	}else{
//		return(NULL);
//	}
//	
//	return pcdpt;
	return(NULL);
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�������݊֐�                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : WritePassTbl                                            |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| PARAMETER    : ushort Status �X�e�[�^�X�@0�`15,0xffff=�����܂Ȃ�       |*/
/*| PARAMETER    : ulong UseParking ���p�����ԏꇂ�@���ԏꇂ1�`999 1001�`260000,0xffffffff=�����܂Ȃ�|*/
/*| PARAMETER    : ushort Valid �L�������@0=�L��,1=����,0xffff=�����܂Ȃ�  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void WritePassTbl( ulong ParkingNo, ushort PassId, ushort Status, ulong UseParking, ushort Valid )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	PAS_TBL *pass_data;
//	uchar use_parking;
//	
//	pass_data = GetPassData(ParkingNo, PassId);
//	
//	if (pass_data == NULL) {
//		return;
//	}
//
//	if (UseParking != 0xFFFFFFFF) {
//		use_parking = NTNET_GetParkingKind((ulong)UseParking, PKOFS_SEARCH_MASTER);
//		if (use_parking == 0xFF) {
//			return;
//		}
//	}
//
//	if (Status != 0xFFFF) {
//		pass_data->BIT.STS = Status;
//	}
//	
//	if (UseParking != 0xFFFFFFFF) {
//		pass_data->BIT.PKN = use_parking;
//	}
//	
//	if (Valid != 0xFFFF) {
//		pass_data->BIT.INV = Valid;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�f�[�^�擾�֐�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort *Start                                           |*/
/*| PARAMETER    : ushort *End                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Start = 0�`11999�CEnd=1�`12000                           MH544401�ǋL  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void GetPassArea( ulong ParkingNo, ushort *Start, ushort *End)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	*Start = 12000;
//	*End = 0;
//
//	if (ParkingNo == CPrmSS[S_SYS][65]) {
//		*Start	= 0;
//		*End	= (ushort)CPrmSS[S_SYS][61];
//	} else if (ParkingNo == CPrmSS[S_SYS][66]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][61] - 1);
//		*End	= (ushort)CPrmSS[S_SYS][62];
//	} else if (ParkingNo == CPrmSS[S_SYS][67]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][62] - 1);
//		*End	= (ushort)CPrmSS[S_SYS][63];
//	} else if (ParkingNo == CPrmSS[S_SYS][68]) {
//		*Start	= (ushort)(CPrmSS[S_SYS][63] - 1);
//		*End	= 12000;
//	}
//
//	if (*End == 0) {
//		*End = 12000;
//	}
//	if (*End != 12000) {
//		*End -= 1;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�������݊֐��i�P�̈�j                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FillPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort Status �X�e�[�^�X�@0�`15,0xffff=�����܂Ȃ�       |*/
/*| PARAMETER    : ulong UseParking ���p�����ԏꇂ�@���ԏꇂ1�`999 1001�`260000,0xffffffff=�����܂Ȃ�|*/
/*| PARAMETER    : ushort Valid �L�������@0=�L��,1=����,0xffff=�����܂Ȃ�  |*/
/*| PARAMETER    : ushort FillFlag 0:�S�f�[�^�X�V�@�@1:�L���f�[�^�̂ݍX�V  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FillPassTbl( ulong ParkingNo, ushort Status, ulong UseParking, ushort Valid, ushort FillType )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort i;
//	ushort Start;
//	ushort End;
//	uchar use_parking;
//
//	GetPassArea(ParkingNo, &Start, &End);
//	
//	use_parking = NTNET_GetParkingKind((ulong)UseParking, PKOFS_SEARCH_MASTER);
//
//	if (Start >= End) {
//		return;
//	}
//
//	if (FillType == 1) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0) {
//				pas_tbl[i].BIT.INV = Valid;
//				pas_tbl[i].BIT.STS = Status;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else if (FillType == 2) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0 && pas_tbl[i].BIT.STS == 1) {
//				pas_tbl[i].BIT.STS = 0;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else if (FillType == 3) {
//		for (i = Start; i < End; i++) {
//			if (pas_tbl[i].BIT.INV == 0 && pas_tbl[i].BIT.STS == 2) {
//				pas_tbl[i].BIT.STS = 0;
//				if (UseParking != 0xFFFFFFFF) {
//					pas_tbl[i].BIT.PKN = use_parking;
//				}
//			}
//		}
//	} else {
//		for (i = Start; i < End; i++) {
//			pas_tbl[i].BIT.INV = Valid;
//			if (Status != 0xFFFF) {
//				pas_tbl[i].BIT.STS = Status;
//			}
//			if (UseParking != 0xFFFFFFFF) {
//				pas_tbl[i].BIT.PKN = use_parking;
//			}
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}


/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�������݊֐�                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReadPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| PARAMETER    : ushort *PassData ������f�[�^                           |*/
/*| RETURN VALUE : short 0:OK -1:NG                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short ReadPassTbl( ulong ParkingNo, ushort PassId, ushort *PassData )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	PAS_TBL *pass_data;
//	
//	pass_data = GetPassData(ParkingNo, PassId);
//	
//	if (pass_data == NULL) {
//		return -1;
//	}
//
//	PassData[0] = (ushort)pass_data->BIT.INV;	// 0:�L��, 1:����
//	PassData[1] = (ushort)pass_data->BIT.STS;	// 0:�������, 1:�o�ɒ�, 2,���ɒ�
//	PassData[2] = (ushort)pass_data->BIT.PKN;	// ���p�����ԏ� 0=��{, 1=�g��1, 2=�g��2, 3=�g��3
//
//	return 0;
	PassData[0] = 0;
	PassData[1] = 0;
	PassData[2] = 0;

	return 0;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�V�X�e�[�^�X�f�[�^�擾�֐�                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassRenewalData                                      |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| RETURN VALUE : PAS_RENEWAL*                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
PAS_RENEWAL *GetPassRenewalData( ulong ParkingNo, ushort PassId, char *pposi)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	long index;
//	PAS_RENEWAL *pcdpt;
//
//	if( PassId > 0 ){
//		PassId--;
//	}else{
//		return(NULL);
//	}
//
//	*pposi = (char)-1;
//	if( (ulong)CPrmSS[S_SYS][65] == ParkingNo ){
//		if( CPrmSS[S_SYS][61] )		index = CPrmSS[S_SYS][61];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - 1 ) ){							// ð��ه@�͈͓̔�?
//			pcdpt = &pas_renewal[PassId/4];									// ð��ه@
//			*pposi = (char)(PassId%4);											// ð��ه@
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][66] == ParkingNo ){
//		if( CPrmSS[S_SYS][62] )		index = CPrmSS[S_SYS][62];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][61] ) ){			// ð��هA�͈͓̔�?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][61] - 1)/4 ];		// ð��هA
//			*pposi = (char)((PassId + CPrmSS[S_SYS][61] - 1)%4);			// ð��هA
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][67] == ParkingNo ){
//		if( CPrmSS[S_SYS][63] )		index = CPrmSS[S_SYS][63];
//		else						index = PAS_MAX + 1;
//
//		if( PassId < (ushort)( index - CPrmSS[S_SYS][62] ) ){			// ð��هB�͈͓̔�?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][62] - 1)/4 ];		// ð��هB
//			*pposi = (char)((PassId + CPrmSS[S_SYS][62] - 1)%4);			// ð��هB
//		}else{
//			return(NULL);
//		}
//	}
//	else if( (ulong)CPrmSS[S_SYS][68] == ParkingNo ){
//		if( PassId < (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){		// ð��هC�͈͓̔�?
//			pcdpt = &pas_renewal[ (PassId + CPrmSS[S_SYS][63] - 1)/4 ];		// ð��هC
//			*pposi = (char)((PassId + CPrmSS[S_SYS][63] - 1)%4);			// ð��هC
//		}else{
//			return(NULL);
//		}
//	}else{
//		return(NULL);
//	}
//	
//	return pcdpt;
	return(NULL);
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}
/*[]----------------------------------------------------------------------[]*/
/*| ������X�V�X�e�[�^�X�擾�֐�                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ReadPassRenewalTbl                                      |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| PARAMETER    : ushort *PassData ������f�[�^                           |*/
/*| RETURN VALUE : short 0:OK -1:NG                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short ReadPassRenewalTbl( ulong ParkingNo, ushort PassId, ushort *PassData )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	char  pass_posi;
//	PAS_RENEWAL *pass_data;
//
//	pass_data = GetPassRenewalData(ParkingNo, PassId, &pass_posi );
//
//	if (pass_data == NULL) {
//		return -1;
//	}
//
//	switch( pass_posi ){
//
//		case 0:
//			PassData[0] = (ushort)pass_data->BIT.Bt01;
//			break;
//		case 1:
//			PassData[0] = (ushort)pass_data->BIT.Bt23;
//			break;
//		case 2:
//			PassData[0] = (ushort)pass_data->BIT.Bt45;
//			break;
//		case 3:
//			PassData[0] = (ushort)pass_data->BIT.Bt67;
//			break;
//	}
//	return 0;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�������݊֐�                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : WritePassRenewalTbl                                     |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort PassId �����ID�@1�`12000                        |*/
/*| PARAMETER    : ushort Status �X�e�[�^�X�@0�`15,0xffff=�����܂Ȃ�       |*/
/*| PARAMETER    : ushort UseParking ���p�����ԏꇂ�@���ԏꇂ1�`999,0xffff=�����܂Ȃ�|*/
/*| PARAMETER    : ushort Valid �L�������@0=�L��,1=����,0xffff=�����܂Ȃ�  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void WritePassRenewalTbl( ulong ParkingNo, ushort PassId, ushort Valid )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	char  pass_posi;
//	PAS_RENEWAL *pass_data;
//	
//	pass_data = GetPassRenewalData(ParkingNo, PassId, &pass_posi);
//	
//	if (pass_data == NULL) {
//		return;
//	}
//
//	switch( pass_posi ){
//
//		case 0:
//			pass_data->BIT.Bt01 = Valid;
//			break;
//		case 1:
//			pass_data->BIT.Bt23 = Valid;
//			break;
//		case 2:
//			pass_data->BIT.Bt45 = Valid;
//			break;
//		case 3:
//			pass_data->BIT.Bt67 = Valid;
//			break;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�f�[�^�擾�֐�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPassData                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort *Start                                           |*/
/*| PARAMETER    : ushort *End                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Start = 0�`11999�CEnd=1�`12000                           MH544401�ǋL  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short GetPassRenewalArea( ulong ParkingNo, ushort *Start, ushort *End, ushort *Sposi, ushort *Eposi )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	*Start = 12000;
//	*End = 0;
//
//	if (ParkingNo == CPrmSS[S_SYS][65]) {
//
//		*Start	= 0;
//		*Sposi	= 0;
//		*Eposi	= 0;
//		if( CPrmSS[S_SYS][61] > 1 ){
//			*End	= (ushort)((CPrmSS[S_SYS][61] - 2)/4);
//			*Eposi	= (ushort)((CPrmSS[S_SYS][61] - 2)%4);
//		}
//	} else if (ParkingNo == CPrmSS[S_SYS][66]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][61] - 1)/4);
//		*End	= (ushort)((CPrmSS[S_SYS][62] - 2)/4);
//		*Sposi	= (ushort)((CPrmSS[S_SYS][61] - 1)%4);
//		*Eposi	= (ushort)((CPrmSS[S_SYS][62] - 2)%4);
//
//	} else if (ParkingNo == CPrmSS[S_SYS][67]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][62] - 1)/4);
//		*End	= (ushort)((CPrmSS[S_SYS][63] - 2)/4);
//		*Sposi	= (ushort)((CPrmSS[S_SYS][62] - 1)%4);
//		*Eposi	= (ushort)((CPrmSS[S_SYS][63] - 2)%4);
//
//	} else if (ParkingNo == CPrmSS[S_SYS][68]) {
//
//		*Start	= (ushort)((CPrmSS[S_SYS][63] - 1)/4);
//		*End	= (12000 - 1)/4;
//		*Sposi	= (ushort)((CPrmSS[S_SYS][63] - 1)%4);
//		*Eposi	= (ushort)((12000 - 1)%4);
//
//	}
//
//	if( *Start == 12000 ){
//		return -1;
//	}
//
//	if (*End == 0) {
//		*End = (12000 - 1)/4;
//		*Eposi	= (ushort)((12000 - 1)%4);
//	}
//	return 0;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�e�[�^�X�������݊֐��i�P�̈�j                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FillPassTbl                                             |*/
/*| PARAMETER    : ushort ParkingNo ���ԏꇂ�@1�`999                       |*/
/*| PARAMETER    : ushort Status �L�������@0=�L��,1=����,0xffff=�����܂Ȃ� |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FillPassRenewalTbl( ulong ParkingNo, uchar Status )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort i;
//	short  ret;
//	ushort Start,End;
//	ushort Sposi,Eposi;
//	ushort Valid;
//	ushort s_adrs,e_adrs;
//
//
//	PAS_RENEWAL *pcdpt;
//
//	ret = GetPassRenewalArea(ParkingNo, &Start, &End, &Sposi, &Eposi );
//
//	if( ret == -1 ){
//		return;
//	}
//
//	Valid = 0;
//	if( Status != 0 ){
//		Valid = 1;
//	}
//
//	if( Start == End ){
//		pcdpt = &pas_renewal[Start];
//		for( i=Sposi; i < Eposi; i++ ){ 
//			switch( i ){
//			case 0:
//				pcdpt->BIT.Bt01 = Valid;
//				break;
//			case 1:
//				pcdpt->BIT.Bt23 = Valid;
//				break;
//			case 2:
//				pcdpt->BIT.Bt45 = Valid;
//				break;
//			case 3:
//				pcdpt->BIT.Bt67 = Valid;
//				break;
//			}
//		}
//	}else if( Start < End ){
//		pcdpt = &pas_renewal[Start];
//		if( Sposi == 0 ){
//			s_adrs = Start;
//		}else{
//			s_adrs = Start + 1;
//			for( i=Sposi; i < 4; i++ ){ 
//
//				switch( i ){
//
//				case 0:
//					pcdpt->BIT.Bt01 = Valid;
//					break;
//				case 1:
//					pcdpt->BIT.Bt23 = Valid;
//					break;
//				case 2:
//					pcdpt->BIT.Bt45 = Valid;
//					break;
//				case 3:
//					pcdpt->BIT.Bt67 = Valid;
//					break;
//				}
//			}
//		}
//		if( Eposi == 3 ){
//			e_adrs = End;
//		}else{
//			e_adrs = End - 1;
//			pcdpt = &pas_renewal[End];
//			for( i=0; i < Eposi; i++ ){ 
//
//				switch( i ){
//
//				case 0:
//					pcdpt->BIT.Bt01 = Valid;
//					break;
//				case 1:
//					pcdpt->BIT.Bt23 = Valid;
//					break;
//				case 2:
//					pcdpt->BIT.Bt45 = Valid;
//					break;
//				case 3:
//					pcdpt->BIT.Bt67 = Valid;
//					break;
//				}
//			}
//		}
//		if( s_adrs <= e_adrs ){
//			pcdpt = &pas_renewal[s_adrs];
//			if(Status == 1){
//				Status = 0x55;
//			}
//			memset( pcdpt, (int)Status, sizeof(PAS_RENEWAL)*(e_adrs - s_adrs + 1));
//		}
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG���
///	@param[out]	*NewestDate	: �ŐV�N��������
///	@param[out]	*OldestDate	: �ŌÔN��������
///	@param[out]	*RegCount	: �o�^����
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Ope2_Log_NewestOldestDateGet( ushort LogSyu,
								  date_time_rec *NewestDate, date_time_rec *OldestDate,
								  ushort *RegCount )
{
	ulong			Nrm_Newest, Nrm_Oldest;
	date_time_rec	*date;
	extern uchar	SysMnt_Work[];			// ������� workarea (32KB)

	*RegCount = Ope_Log_TotalCountGet(LogSyu);
	if( *RegCount == 0 ){		// �f�[�^�Ȃ����͌��ݎ����Ƃ���
		Nrm_Oldest = Nrm_YMDHM((date_time_rec*)&CLK_REC);
		Nrm_Newest = Nrm_Oldest;
	}else{
		Ope_Log_1DataGet(LogSyu, 0, SysMnt_Work);
		date = get_log_date_time_rec(LogSyu, SysMnt_Work);
		Nrm_Oldest = Nrm_YMDHM(date);
		Ope_Log_1DataGet(LogSyu, *RegCount-1, SysMnt_Work);
		Nrm_Newest = Nrm_YMDHM(date);
	}

	// �ް��`���ϊ�(ɰ�ײ�ށ� date_time_rec)
	UnNrm_YMDHM(OldestDate, Nrm_Oldest);
	UnNrm_YMDHM(NewestDate, Nrm_Newest);
}
/* Suica�����W�v�͋������ɂčs���̂ŕ��������� */
//[]----------------------------------------------------------------------[]
///	@brief		LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG���
///	@param[out]	*NewestDate	: �ŐV�N��������
///	@param[out]	*OldestDate	: �ŌÔN��������
///	@param[out]	*RegCount	: �o�^����
///	@return		void
//[]----------------------------------------------------------------------[]
///	�ŌÁ��ŐV���t�͓o�^����Ă���LOG�S�̂�����ċ��߂�B
///	�o�^�ް����Ȃ��ꍇ�͌��ݎ�����Ă���B	
//[]----------------------------------------------------------------------[]
/// Author       : Okuda				
/// Date         : 2006/05/18			
/// UpDate       :						
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	DiditalCashe_NewestOldestDateGet( ushort LogSyu,
									  date_time_rec	*NewestDate,
									  date_time_rec	*OldestDate,
									  ushort *RegCount )
{
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n�񌟍����������������
	extern	uchar	SysMnt_Work[];			// ������� workarea (32KB)
	void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 	// ���ٰ�� prottype define
											   ulong *OldestDateTime, 
											   ulong TargetDateTime );
	ushort	Count=0, i;
	ulong	Nrm_Newest, Nrm_Oldest, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	usPtr;							// �������n�߂�z��̈ʒu

	Nrm_Newest = Nrm_Oldest = 0L;

	/* LOG��ʖ����� */
	switch( LogSyu ){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case LOG_EDYSYUUKEI:							// �d�����W�v
//		Count = Syuukei_sp.edy_inf.cnt - 1;			// �W�v�����X�V
//		/* �W�v�����������J��Ԃ� */
//		for(i = 1; i <= Count; i++)
//		{
//			/* ���̎Q�ƈʒu�ֈړ� */
//			usPtr = (Syuukei_sp.edy_inf.ptr + SYUUKEI_DAY - i) % SYUUKEI_DAY;
//			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
//			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.edy_inf.bun[usPtr].SyuTime );
//			/* �ŐV&�Ō��ް��X�V */
//			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
//		}
//		break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case LOG_SCASYUUKEI:							// �r���������W�v
		Count = Syuukei_sp.sca_inf.cnt - 1;			// �W�v�����X�V
		/* �W�v�����������J��Ԃ� */
		for(i = 1; i <= Count; i++)
		{
			/* ���̎Q�ƈʒu�ֈړ� */
			usPtr = (Syuukei_sp.sca_inf.ptr + SYUUKEI_DAY - i) % SYUUKEI_DAY;
			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.sca_inf.bun[usPtr].SyuTime );
			/* �ŐV&�Ō��ް��X�V */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
	case LOG_ECSYUUKEI:								// ���σ��[�_�W�v
		Count = Syuukei_sp.ec_inf.cnt - 1;			// �W�v�����X�V
		for(i = 1; i <= Count; i++)
		{
			/* ���̎Q�ƈʒu�ֈړ� */
			usPtr = (Syuukei_sp.ec_inf.ptr + SYUUKEI_DAY_EC - i) % SYUUKEI_DAY_EC;
			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&Syuukei_sp.ec_inf.bun[usPtr].SyuTime );
			/* �ŐV&�Ō��ް��X�V */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
	}

	/* �f�[�^�Ȃ����͍ŐV���ŌÓ������������ݎ����Ƃ��� */
	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );	// ���ݎ��Ԃ�save����
	if( Nrm_Newest == 0L )
		Nrm_Newest = Nrm_YMDHM( &wk_CLK_REC );
	if( Nrm_Oldest == 0L )
		Nrm_Oldest = Nrm_YMDHM( &wk_CLK_REC );

	/* �Ō�&�ŐV���� �ް��`���ϊ�(ɰ�ײ�ށ� date_time_rec) */
	UnNrm_YMDHM( NewestDate, Nrm_Newest );
	UnNrm_YMDHM( OldestDate, Nrm_Oldest );
	*RegCount = Count;
}

/*[]-----------------------------------------------------------------------[]*/
/*| �N���������m�[�}���C�Y�l�𓾂�											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	date_time_rec�^�ް����� t_NrmYMDHM�^ɰ�ײ�ޒl�ɕϊ�����B				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *wk_CLK_REC = �Ώێ����ް��ւ��߲��(date_time_rec�^)		|*/
/*| RETURN VALUE : ɰ�ײ�ޒl (b31-16=�N����, b15-0=����)					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned long	Nrm_YMDHM( date_time_rec *wk_CLK_REC )
{
	t_NrmYMDHM	Nrm;

	Nrm.us[0] = dnrmlzm( (short)wk_CLK_REC->Year, (short)wk_CLK_REC->Mon, (short)wk_CLK_REC->Day );
	Nrm.us[1] = (ushort)tnrmlz( (short)0, (short)0, (short)wk_CLK_REC->Hour, (short)wk_CLK_REC->Min );

	return	Nrm.ul;
}

/*[]-----------------------------------------------------------------------[]*/
/*| �N���������f�[�^ �m�[�}���C�Y�l �� CLK_REC�^�ɕϊ�						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	t_NrmYMDHM�^ɰ�ײ�ޒl���� date_time_rec�^�ɕϊ�����B					|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *wk_CLK_REC = �Ώێ����ް��ւ��߲��(date_time_rec�^)		|*/
/*|                Nrm_YMDHM   = ɰ�ײ�ޒl (b31-16=�N����, b15-0=����)		|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	UnNrm_YMDHM( date_time_rec *wk_CLK_REC, ulong Nrm_YMDHM )
{
	t_NrmYMDHM	Nrm;
	short		wks[3];


	Nrm.ul = Nrm_YMDHM;

	idnrmlzm( Nrm.us[0], &wks[0], &wks[1], &wks[2] );		// �N������ɰ�ײ��
	wk_CLK_REC->Year = (ushort)wks[0];
	wk_CLK_REC->Mon  = (uchar)wks[1];
	wk_CLK_REC->Day  = (uchar)wks[2];

	itnrmlz ( (short)Nrm.us[1], 0, 0, &wks[0], &wks[1] );	// ������ɰ�ײ��
	wk_CLK_REC->Hour = (uchar)wks[0];
	wk_CLK_REC->Min  = (uchar)wks[1];
}

/*[]-----------------------------------------------------------------------[]*/
/*| Target�����ް�����ŌÁ��ŐV���t���X�V����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : *NewestDateTime = �ŐV�����ް��ւ��߲��					|*/
/*|				   *OldestDateTime = �ŌÎ����ް��ւ��߲��					|*/
/*|				    TargetDateTime = Target����								|*/
/*|				  ���ް��`���� b31-16=�N����, b15-0=���� ��ɰ�ײ�ޒl������B|*/
/*| RETURN VALUE : void														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Target�������ŌÂ��Â��A���͍ŐV���V�����ꍇ�ʹر���X�V����B		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/18												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 
										   ulong *OldestDateTime, 
										   ulong TargetDateTime )
{
	if( *NewestDateTime == 0L )					// �O�f�[�^�Ȃ�
		*NewestDateTime = TargetDateTime;		// �ŐV�����X�V

	if( *OldestDateTime == 0L )					// �O�f�[�^�Ȃ�
		*OldestDateTime = TargetDateTime;

	if( *NewestDateTime < TargetDateTime )		// �ŐV���V����
		*NewestDateTime = TargetDateTime;		// �ŐV�����X�V
	else if( *OldestDateTime > TargetDateTime )	// �ŌÂ��Â�
		*OldestDateTime = TargetDateTime;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ړILOG�ް����� �w��N�������͈͓����ް����������邩�����߂�֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ���O��� 	"LOG_PAYMENT"		= �ʐ��Z<br>
///											"LOG_PAYSTOP"		= ���Z���~<br>
///											"LOG_TTOTAL"		= T���v<br>
///											"LOG_MONEYMANAGE"	= ���K�Ǘ�<br>
///											"LOG_COINBOX"		= ��݋��ɍ��v<br>
///											"LOG_NOTEBOX"		= �������ɍ��v
///											"LOG_EDYMEISAI"		= �d�������p����<br>
///											"LOG_SCAMEISAI"		= �r�����������p����<br>
///	@param[in]		NewDate	: ���t�͈́i�V���t���j
///	@param[in]		OldDate	: ���t�͈́i�����t���j
///	@param[in]		NewTime	: ���t�͈́i�V���ԁj
///	@param[in]		OldTime	: ���t�͈́i�����ԁj
///	@return			ret		: �w����t�͈͓���LOG����
///	@author			Imai
///	@note			�ŌÁ��ŐV���t�͓o�^����Ă���LOG�S�̂�����ċ��߂�B<br>
///	                �o�^�ް����Ȃ��ꍇ�͌��ݎ�����Ă���B
///					���LOpe2_Log_CountGet_inDate()�𗬗p�B<br>
///					Ope2_Log_CountGet_inDate()���Q�ƁB<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/07/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_CountGet_inDateTime( ushort LogSyu, ushort *NewDate, ushort *OldDate,
									ushort NewTime, ushort OldTime )
{
	extern	uchar	SysMnt_Work[];			// ������� workarea (32KB)
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n�񌟍����������������

	ushort	Count=0, i, inData_Count=0;
	ulong	Nrm_New, Nrm_Old, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	LogCount_inSRAM;				// SRAM�� LOG�o�^����
	ushort	LogCount_inFROM;				// FlashROM�� LOG�o�^����
	ulong	ulRet;							// LOG�����擾work
	ulong	ulWork;							// LOG�����擾work
	t_FltLogHandle handle;
	Receipt_data tmpReceipt;				// ���f�����p���Z���

	/* �����J�n���� ɰ�ײ�ޒlget */
	wk_CLK_REC.Year = OldDate[0];
	wk_CLK_REC.Mon  = (uchar)OldDate[1];
	wk_CLK_REC.Day  = (uchar)OldDate[2];
	wk_CLK_REC.Hour = (uchar)OldTime;
	wk_CLK_REC.Min  = 0;
	Nrm_Old = Nrm_YMDHM( &wk_CLK_REC );

	/* �����I������ ɰ�ײ�ޒlget */
	wk_CLK_REC.Year = NewDate[0];
	wk_CLK_REC.Mon  = (uchar)NewDate[1];
	wk_CLK_REC.Day  = (uchar)NewDate[2];
	wk_CLK_REC.Hour = (uchar)(NewTime-1);
	wk_CLK_REC.Min  = 59;
	Nrm_New = Nrm_YMDHM( &wk_CLK_REC );
	LogCount_inSRAM = 0;
	Nrm_Target = 0;

	/* LOG��ʖ����� */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// �ʐ��Z
		/** LOG����get **/

		/* FlashROM�� LOG�o�^����get */
		ulRet = FLT_GetSaleLogRecordNum( (long*) &ulWork );	// FlashROM��LOG�o�^����get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
Ope2_Log_ReadNG_210:
			ulWork = 0L;							// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖����̂Ń��g���C�͂��Ȃ�
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM�� LOG�o�^����set

		/* SRAM�� LOG�o�^����get */

		/* LOG�o�^�����iTotal�jget */
		Count = LogCount_inSRAM + LogCount_inFROM;	// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/

		/* FlashROM���ް����� */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstSaleLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextSaleLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:�o�^��ƒ�)
				goto Ope2_Log_ReadNG_210;			// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
			}										// ������������Flash���ɂ�LOG�����Ƃ��ē��삷��

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletask��1�T��ɕ��A����

		/* SRAM���ް����� */
		for( i=0; i<LogCount_inSRAM; ++i ){
			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

	case	LOG_TTOTAL:					// T���v
		/** LOG����get **/

		/* FlashROM�� LOG�o�^����get */
		ulRet = FLT_GetTotalLogRecordNum( (long*) &ulWork );	// FlashROM��LOG�o�^����get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
Ope2_Log_ReadNG_220:
			ulWork = 0L;							// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖����̂Ń��g���C�͂��Ȃ�
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM�� LOG�o�^����set

		/* SRAM�� LOG�o�^����get */

		/* LOG�o�^�����iTotal�jget */
		Count = LogCount_inSRAM + LogCount_inFROM;	// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/

		/* FlashROM���ް����� */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstTotalLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextTotalLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:�o�^��ƒ�)
				goto Ope2_Log_ReadNG_220;			// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
			}										// ������������Flash���ɂ�LOG�����Ƃ��ē��삷��

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletask��1�T��ɕ��A����

		/* SRAM���ް����� */
		for( i=0; i<LogCount_inSRAM; ++i ){
			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

	case	LOG_PAYSTOP:				// ���Z���~
		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set

		for( i=0; i<Count; ++i ){

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

	case	LOG_MONEYMANAGE:			// ���K�Ǘ�
		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set

		for( i=0; i<Count; ++i ){

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

	case	LOG_COINBOX:				// ��݋��ɍ��v
		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set

		for( i=0; i<Count; ++i ){

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

	case	LOG_NOTEBOX:				// �������ɍ��v
		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set

		for( i=0; i<Count; ++i ){

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
				++inData_Count;
		}
		break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case	LOG_EDYARM:				// �A���[��������
//		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set
//
//		for( i=0; i<Count; ++i ){
//
//			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
//
//			/* �͈͓������ް������� */
//			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
//				++inData_Count;
//		}
//		break;
//
//	case	LOG_EDYSHIME:				// �d�������ߋL�^
//		Count = Ope2_Log_CountGet( LogSyu );		// �o�^����set
//
//		for( i=0; i<Count; ++i ){
//
//			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
//
//			/* �͈͓������ް������� */
//			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
//				++inData_Count;
//		}
//		break;
//
//#ifdef	FUNCTION_MASK_EDY 
//	case	LOG_EDYMEISAI:							// �d�������p���Z��
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case	LOG_SCAMEISAI:							// �r�����������p���Z��
		/* FlashROM�ւ̐��Z�������O�擾(�擾�����Ō�������������) */
		if(!(FLT_GetSaleLogRecordNum((long*)&ulWork) & 0xFF000000))
		{
			Count = (ushort)ulWork;
			for(i = 0; i < Count; i++)					/* FlashROM����f�[�^��ǂݏo�����[�v */
			{
				// �ʐ��Z�f�[�^�P���Ǐo��
				if( 0 == i )
					ulRet = FLT_FindFirstEpayKnd( &handle, (char*)&tmpReceipt, 1, 0);
				else
					ulRet = FLT_FindNextEpayKnd( &handle, (char*)&tmpReceipt, 1);

				xPause( 0 );						// taskchg�ɕ��A����

				if( 0 != (ulRet & 0xff000000) ){		// �Ǐo�����s(Y)
					break;								// FlashROM�����������I��
				}
				/* �擾������񂪈�������̏����ƈ�v */
				if(is_match_receipt(&tmpReceipt, Nrm_Old, Nrm_New, LogSyu))
				{
					/* �q�b�g�����X�V */
					inData_Count++;
				}
			}
			FLT_CloseLogHandle( &handle );				// Handle & Lock release
		}
		break;
	}

	return	inData_Count;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ړILOG�ް����� �w��N�������͈͓����ް����������邩�����߂�֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu		: ���O��� 	"LOG_PAYMENT"		= �ʐ��Z<br>
///											"LOG_TTOTAL"		= T���v<br>
///											"LOG_EDYMEISAI"		= Edy���p����<br>
///											"LOG_SCAMEISAI"		= Suica���p����<br>
///	@param[in]		NewDate		: ���t�͈́i�V���t���j
///	@param[in]		OldDate		: ���t�͈́i�����t���j
///	@param[in]		NewTime		: ���t�͈́i�V���ԁj
///	@param[in]		OldTime		: ���t�͈́i�����ԁj
///	@param[out]		FirstIndex	: �ŏ��ɏ�����v����FlashROM�̔ԍ�
///	@param[out]		LastIndex	: �Ō�ɏ�����v����FlashROM�̔ԍ�
///	@return			ret			: �w����t�͈͓���LOG����
///	@author			Ise
///	@note			Ope2_Log_CountGet_inDateTime�֐������̂܂ܗ��p
///					�{�֐��͏�L�֐�+FlashROM�̌����ōŏ��E�Ō�ɏ�����v����
///					�ԍ����U(ushort *FirstIndex)�A��V(ushort *LastIndex)����
///					�Ɋi�[���ăR�[�����֕Ԃ�<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/011/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_CountGet_inDateTime_AttachOffset( ushort LogSyu, ushort *NewDate, ushort *OldDate,
									ushort NewTime, ushort OldTime, ushort *FirstIndex, ushort *LastIndex )
{
	extern	uchar	SysMnt_Work[];			// ������� workarea (32KB)
	#define	TASK_SWITCH_INTERVAL_COUNT	100	// n�񌟍����������������

	ushort	Count=0, i, inData_Count=0;
	ulong	Nrm_New, Nrm_Old, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ushort	LogCount_inSRAM;				// SRAM�� LOG�o�^����
	ushort	LogCount_inFROM;				// FlashROM�� LOG�o�^����
	ulong	ulRet;							// LOG�����擾work
	ulong	ulWork;							// LOG�����擾work
	t_FltLogHandle handle;
	Receipt_data tmpReceipt;				// ���f�����p���Z���

	/* �����J�n���� ɰ�ײ�ޒlget */
	wk_CLK_REC.Year = OldDate[0];
	wk_CLK_REC.Mon  = (uchar)OldDate[1];
	wk_CLK_REC.Day  = (uchar)OldDate[2];
	wk_CLK_REC.Hour = (uchar)OldTime;
	wk_CLK_REC.Min  = 0;
	Nrm_Old = Nrm_YMDHM( &wk_CLK_REC );

	/* �����I������ ɰ�ײ�ޒlget */
	wk_CLK_REC.Year = NewDate[0];
	wk_CLK_REC.Mon  = (uchar)NewDate[1];
	wk_CLK_REC.Day  = (uchar)NewDate[2];
	wk_CLK_REC.Hour = (uchar)(NewTime-1);
	wk_CLK_REC.Min  = 59;
	Nrm_New = Nrm_YMDHM( &wk_CLK_REC );

	*FirstIndex = 0xFFFF;					// ��v����Flash�ԍ��i�[�p�ϐ�������
	*LastIndex = 0xFFFF;					// ��v����Flash�ԍ��i�[�p�ϐ�������
	LogCount_inSRAM = 0;

	/* LOG��ʖ����� */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// �ʐ��Z
		/** LOG����get **/

		/* FlashROM�� LOG�o�^����get */
		ulRet = FLT_GetSaleLogRecordNum( (long*) &ulWork );	// FlashROM��LOG�o�^����get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
Ope2_Log_ReadNG_210:
			ulWork = 0L;							// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖����̂Ń��g���C�͂��Ȃ�
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM�� LOG�o�^����set

		/* SRAM�� LOG�o�^����get */

		/* LOG�o�^�����iTotal�jget */
		Count = LogCount_inSRAM + LogCount_inFROM;	// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/

		/* FlashROM���ް����� */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstSaleLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextSaleLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:�o�^��ƒ�)
				goto Ope2_Log_ReadNG_210;			// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
			}										// ������������Flash���ɂ�LOG�����Ƃ��ē��삷��

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
			{
				if(!inData_Count && FirstIndex)		// �����J�n��ŏ��ɏ�����v
				{
					*FirstIndex = i;				// ��v����Flash�̔ԍ����i�[
				}
				if(LastIndex)						// �Q��ڈȍ~������v����
				{
					*LastIndex = i;					// ��v����Flash�̔ԍ����i�[
				}
				++inData_Count;
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletask��1�T��ɕ��A����

		/* SRAM���ް����� */
		break;

	case	LOG_TTOTAL:					// T���v
		/** LOG����get **/

		/* FlashROM�� LOG�o�^����get */
		ulRet = FLT_GetTotalLogRecordNum( (long*) &ulWork );	// FlashROM��LOG�o�^����get
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
Ope2_Log_ReadNG_220:
			ulWork = 0L;							// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖����̂Ń��g���C�͂��Ȃ�
		}
		LogCount_inFROM = (ushort)ulWork;			// FlashROM�� LOG�o�^����set

		/* SRAM�� LOG�o�^����get */

		/* LOG�o�^�����iTotal�jget */
		Count = LogCount_inSRAM + LogCount_inFROM;	// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/

		/* FlashROM���ް����� */
		for( i=0; i<LogCount_inFROM; ++i ){
			if( 0 == i )
				ulRet = FLT_FindFirstTotalLog( &handle, (char*)&SysMnt_Work[0] );
			else
				ulRet = FLT_FindNextTotalLog_OnlyDate ( &handle, (char*)&SysMnt_Work[0] );
			if( 0 != (ulRet & 0xff000000) ){		// Read error (BUSY:�o�^��ƒ�)
				goto Ope2_Log_ReadNG_220;			// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
			}										// ������������Flash���ɂ�LOG�����Ƃ��ē��삷��

			if( 0 == (i+1) % TASK_SWITCH_INTERVAL_COUNT )
				xPause( 0 );						// idletask��1�T��ɕ��A����

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* �͈͓������ް������� */
			if( (Nrm_Old <= Nrm_Target) && (Nrm_Target <= Nrm_New) )	// �w�莞�Ԕ͈͓�
			{
				if(!inData_Count && FirstIndex)		// �����J�n��ŏ��ɏ�����v
				{
					*FirstIndex = i;				// ��v����Flash�̔ԍ����i�[
				}
				if(LastIndex)						// �Q��ڈȍ~������v����
				{
					*LastIndex = i;					// ��v����Flash�̔ԍ����i�[
				}
				++inData_Count;
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		xPause( 0 );								// idletask��1�T��ɕ��A����
		break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case	LOG_EDYMEISAI:							// �d�������p���Z��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case	LOG_SCAMEISAI:							// �r�����������p���Z��
		/* FlashROM�ւ̐��Z�������O�擾(�擾�����Ō�������������) */
		if(!(FLT_GetSaleLogRecordNum((long*)&ulWork) & 0xFF000000))
		{
			Count = (ushort)ulWork;
			for(i = 0; i < Count; i++)					/* FlashROM����f�[�^��ǂݏo�����[�v */
			{
				// �ʐ��Z�f�[�^�P���Ǐo��
				if( 0 == i )
					ulRet = FLT_FindFirstEpayKnd( &handle, (char*)&tmpReceipt, 1, 0);
				else
					ulRet = FLT_FindNextEpayKnd( &handle, (char*)&tmpReceipt, 1);

				xPause( 0 );						// taskchg�ɕ��A����

				if( 0 != (ulRet & 0xff000000) ){		// �Ǐo�����s(Y)
					break;								// FlashROM�����������I��
				}
				/* �擾������񂪈�������̏����ƈ�v */
				if(is_match_receipt(&tmpReceipt, Nrm_Old, Nrm_New, LogSyu))
				{
					if(!inData_Count)					// �����J�n��ŏ��ɏ�����v
					{
						*FirstIndex = i;				// �Ō�Ɉ�v����Flash�̔ԍ����i�[
					}
					*LastIndex = i;						// �Ō�Ɉ�v����Flash�̔ԍ����i�[
					/* �q�b�g�����X�V */
					inData_Count++;
				}
			}
			FLT_CloseLogHandle( &handle );				// Handle & Lock release
		}
		break;
	}
	return	inData_Count;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ړILOG��ʖ����ް����������邩�����߂�֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu		: ���O��� 	"LOG_PAYMENT"		= �ʐ��Z<br>
///											"LOG_TTOTAL"		= T���v<br>
///											"LOG_GTTOTAL"		= GT���v<br>
///											"LOG_EDYMEISAI"		= Edy���p����<br>
///											"LOG_SCAMEISAI"		= Suica���p����<br>
///	@param[out]		NewestDate	: �ŐV����������Ĵر�ւ��߲��
///	@param[out]		OldestDate	: �ŌÓ���������Ĵر�ւ��߲��
///	@param[out]		FirstIndex	: �ŏ��ɏ�����v����FlashROM�̔ԍ�
///	@param[out]		LastIndex	: �Ō�ɏ�����v����FlashROM�̔ԍ�
///	@return			ret			: �w����t�͈͓���LOG����
///	@author			Ise
///	@note			Ope2_Log_CountGet_inDateTime�֐������̂܂ܗ��p
///					�{�֐��͏�L�֐�+FlashROM�̌����ōŏ��E�Ō�ɏ�����v����
///					�ԍ����S(ushort *FirstIndex)�A��T(ushort *LastIndex)����
///					�Ɋi�[���ăR�[�����֕Ԃ�<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/011/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ope2_Log_NewestOldestDateGet_AttachOffset(	ushort			LogSyu,
													date_time_rec	*NewestDate,
													date_time_rec	*OldestDate,
													ushort			*FirstIndex,
													ushort			*LastIndex )
{
	extern	uchar	SysMnt_Work[];			// ������� workarea (32KB)
	void	Ope2_Log_NewestOldestDateGet_sub1( ulong *NewestDateTime, 	// ���ٰ�� prottype define
											   ulong *OldestDateTime, 
											   ulong TargetDateTime );
	ushort	Count=0, i;
	ulong	Nrm_Newest, Nrm_Oldest, Nrm_Target;
	date_time_rec	wk_CLK_REC;
	ulong	ulRet;							// LOG�����擾work
	t_FltLogHandle handle;
	Receipt_data *tmpReceipt;				// ���f�����p���Z���
	ushort	Count_wk=0;
	i = 0;

	Nrm_Newest = Nrm_Oldest = 0L;
	*FirstIndex = 0xFFFF;
	*LastIndex = 0xFFFF;

	/* LOG��ʖ����� */
	switch( LogSyu ){
	case	LOG_PAYMENT:				// �ʐ��Z
		/** LOG����get **/

		/* LOG�o�^�����iTotal�jget */
		Count = Ope2_Log_CountGet( LOG_PAYMENT );		// �o�^����set

		tmpReceipt = (Receipt_data*)&SysMnt_Work[0];

		/** �ŐV&�Ō��ް��̓������擾 **/
		ulRet = FLT_FindFirstLog(eLOG_PAYMENT, &handle, (char*)&SysMnt_Work[0] );
		if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
			Count = 0;								// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
													// ������������LOG�����Ƃ��ē��삷��
		} else {
			if((tmpReceipt->OutKind == 1) || (tmpReceipt->OutKind == 11) || (tmpReceipt->OutKind == 3)) {
				i = 0;// �����o�ɁA�s���o�ɂ̃f�[�^�̏ꍇ�����̑ΏۂƂȂ�Ȃ����߃X�L�b�v(�J�E���g�ΏۊO)
			}else{
				i = 1;
				/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
				Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

				/* �ŐV&�Ō��ް��X�V */
				Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
			}

			while(  i < Count ){
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_PAYMENT, &handle, (char*)&SysMnt_Work[0] );

				if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
					Count = 0;								// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
					break;									// ������������LOG�����Ƃ��ē��삷��
				}

				if((tmpReceipt->OutKind == 1) || (tmpReceipt->OutKind == 11) || (tmpReceipt->OutKind == 3)) {
					continue;// �����o�ɁA�s���o�ɂ̃f�[�^�̏ꍇ�����̑ΏۂƂȂ�Ȃ����߃X�L�b�v(�J�E���g�ΏۊO)
				} else {
					i++;
				}
				/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
				Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );

				/* �ŐV&�Ō��ް��X�V */
				Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
			}
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// �Ō�Ɉ�v�����ԍ����i�[
		*LastIndex = i;								// �Ō�Ɉ�v�����ԍ����i�[
		break;

	case	LOG_TTOTAL:					// T���v

		/** LOG����get **/

		/* LOG�o�^�����iTotal�jget */
		Count = Ope2_Log_CountGet( LOG_TTOTAL );		// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/
		for( i=0; i < Count; ++i ){
			if( 0 == i ){
				ulRet = FLT_FindFirstLog(eLOG_TTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}else{
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_TTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}

			if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
				Count = 0;								// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
				break;									// ������������LOG�����Ƃ��ē��삷��
			}

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* �ŐV&�Ō��ް��X�V */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// �Ō�Ɉ�v�����ԍ����i�[
		*LastIndex = i;								// �Ō�Ɉ�v�����ԍ����i�[
		break;

	case	LOG_GTTOTAL:				// GT���v

		/** LOG����get **/

		/* LOG�o�^�����iTotal�jget */
		Count = Ope2_Log_CountGet( LOG_GTTOTAL );		// �o�^����set

		/** �ŐV&�Ō��ް��̓������擾 **/
		for( i=0; i < Count; ++i ){
			if( 0 == i ){
				ulRet = FLT_FindFirstLog(eLOG_GTTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}else{
				ulRet = FLT_FindNextLog_OnlyDate(eLOG_GTTOTAL, &handle, (char*)&SysMnt_Work[0] );
			}

			if( 0 != (ulRet & 0xff000000) ){			// Read error (BUSY:�o�^��ƒ�)
				Count = 0;								// ���̎��ɓo�^��ƒ��ł��邱�Ƃ͖�����
				break;									// ������������LOG�����Ƃ��ē��삷��
			}

			/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
			Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Syu_log*)&SysMnt_Work[0])->syukei_log.NowTime) );

			/* �ŐV&�Ō��ް��X�V */
			Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
		}
		FLT_CloseLogHandle( &handle );				// Handle & Lock release

		*FirstIndex = 0;							// �Ō�Ɉ�v�����ԍ����i�[
		*LastIndex = i;								// �Ō�Ɉ�v�����ԍ����i�[
		break;

	case	LOG_SCAMEISAI:								// �r�����������p����
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	case	LOG_ECMEISAI:								// ���σ��[�_���p����
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	case	LOG_ECMINASHI:								// �݂Ȃ�����(���σ��[�_)���Z��
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			Count_wk = Ope_Log_TotalCountGet(eLOG_PAYMENT);
			for(i = 0; i < Count_wk; i++)					/* FlashROM����f�[�^��ǂݏo�����[�v */
			{
				// �ʐ��Z�f�[�^�P���Ǐo��
				Ope_Log_1DataGet(eLOG_PAYMENT, i, SysMnt_Work);

//				/* �擾������񂪈�������̏����ƈ�v */
//				if(is_match_receipt(&tmpReceipt, 0, 0, LogSyu))
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				if(is_match_receipt((Receipt_data*)&SysMnt_Work, 0, 0, LOG_SCAMEISAI))
				if(is_match_receipt((Receipt_data*)&SysMnt_Work, 0, 0, LogSyu))
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
				{
					/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
					Nrm_Target = Nrm_YMDHM( (date_time_rec*)&(((Receipt_data*)&SysMnt_Work[0])->TOutTime) );
					/* �ŐV&�Ō��ް��X�V */
					Ope2_Log_NewestOldestDateGet_sub1( &Nrm_Newest, &Nrm_Oldest, Nrm_Target );
					if(!Count)							// �����J�n��ŏ��ɏ�����v
					{
						*FirstIndex = i;				// �Ō�Ɉ�v����Flash�̔ԍ����i�[
					}
					*LastIndex = i;						// �Ō�Ɉ�v����Flash�̔ԍ����i�[
					/* �q�b�g�����X�V */
					Count++;
				}
			}
		break;

	}

	/* �f�[�^�Ȃ����͍ŐV���ŌÓ������������ݎ����Ƃ��� */
	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );	// ���ݎ��Ԃ�save����
	if( Nrm_Newest == 0L )
		Nrm_Newest = Nrm_YMDHM( &wk_CLK_REC );
	if( Nrm_Oldest == 0L )
		Nrm_Oldest = Nrm_YMDHM( &wk_CLK_REC );

	/* �Ō�&�ŐV���� �ް��`���ϊ�(ɰ�ײ�ށ� date_time_rec) */
	UnNrm_YMDHM( NewestDate, Nrm_Newest );
	UnNrm_YMDHM( OldestDate, Nrm_Oldest );

	return Count;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG����w��N�����ް����������邩�����߂�
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu	: LOG���
///	@param[in]	*Date	: �w��N����
///	@param[out]	*id		: �擪���R�[�hID
///	@return		ushort	: ����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope2_Log_CountGet_inDate( ushort LogSyu, ushort *Date, ushort *id )
{
	ushort			end, count;
	date_time_rec	dtrec;
	extern	uchar	SysMnt_Work[];			// ������� workarea (32KB)

	count = Ope_Log_TotalCountGet(LogSyu);
	if( count==0 ){					// ���R�[�h�Ȃ�
		return 0;
	}

	/* �����J�n���� ɰ�ײ�ޒlget */
	dtrec.Year = Date[0];
	dtrec.Mon  = (uchar)Date[1];
	dtrec.Day  = (uchar)Date[2];
	dtrec.Hour = 0;
	dtrec.Min  = 0;
	*id = count-1;
	if( !Ope2_Log_Get_inDate(LogSyu, Nrm_YMDHM(&dtrec), id, SysMnt_Work) ){	// �Y���Ȃ�
		return 0;
	}

	dtrec.Day++;
	end = count-1;
	if( !Ope2_Log_Get_inDate(LogSyu, Nrm_YMDHM(&dtrec), &end, SysMnt_Work) ){	// �Y���Ȃ�
		end++;
	}

	return (ushort)(end-*id);
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG����w��N�����̍ŏ��̃��R�[�h�𓾂�
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG���
///	@param[in]	date		: �w��N����(ɰ�ײ�ޒl)
///	@param[in/out]	*id		: ���R�[�hID
///	@param[out]	*pBuf		: ���R�[�h�f�[�^
///	@return		BOOL		: TRUE=����, FALSE=�Ȃ�
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL Ope2_Log_Get_inDate( ushort LogSyu, ulong Nrm_Date, ushort *id, void *pBuf )
{
	BOOL			result=FALSE;
	ushort			min=0, work=*id;
	ulong			Nrm_Target;
	date_time_rec	*drec;

	drec = get_log_date_time_rec(LogSyu, pBuf);
	Ope_Log_1DataGet(LogSyu, 0, pBuf);
	drec->Hour = 0;
	drec->Min  = 0;
	Nrm_Target = Nrm_YMDHM(drec);
	if( Nrm_Target==Nrm_Date ){			// �q�b�g
		*id = 0;
		return TRUE;
	}

	if( *id == 1 ){
		Ope_Log_1DataGet(LogSyu, 1, pBuf);
		drec->Hour = 0;
		drec->Min  = 0;
		Nrm_Target = Nrm_YMDHM(drec);
		if( Nrm_Target == Nrm_Date ){	// �q�b�g
			result = TRUE;
		}
	}else{
	while( min+1<*id ){
		Ope_Log_1DataGet(LogSyu, work, pBuf);
		drec->Hour = 0;
		drec->Min  = 0;
		Nrm_Target = Nrm_YMDHM(drec);
		if( Nrm_Target==Nrm_Date ){		// �q�b�g
			*id = work;
			result = TRUE;
		}
		else if( Nrm_Target>Nrm_Date ){	// �Â�
			*id = work;
		}
		else{
			min = work;
		}
		work = min + (*id-min)/2;
	}
	}

	return result;
}

/*[]-----------------------------------------------------------------------[]*/
/*| LOG�o�^����get															|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : LogSyu =	���O��� 	"LOG_PAYMENT"		= �ʐ��Z		|*/
/*|										"LOG_PAYSTOP"		= ���Z���~		|*/
/*|										"LOG_TTOTAL"		= T���v			|*/
/*|										"LOG_GTTOTAL"		= GT���v		|*/
/*|										"LOG_MONEYMANAGE"	= ���K�Ǘ�		|*/
/*|										"LOG_COINBOX"		= ��݋��ɍ��v	|*/
/*|										"LOG_NOTEBOX"		= �������ɍ��v	|*/
/*|										"LOG_ABNORMAL"		= �s���o��		|*/
/*|										"LOG_EDYMEISAI"		= �d�������p����|*/
/*|										"LOG_SCAMEISAI"		= �r�����������p����|*/
/*|										"LOG_EDYSYUUKEI"	= �d�����W�v	|*/
/*|										"LOG_SCASYUUKEI"	= �r���������W�v|*/
/*| RETURN VALUE : LOG����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	�ʐ��ZLOG�C�W�vLOG��FlashROM+SRAM��İ�ٌ�����߂��B					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2006/05/19												|*/
/*| UpDate       :															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	Ope2_Log_CountGet( ushort LogSyu )
{
	ushort	LogCount=0;

	switch( LogSyu ){
	case	LOG_PAYMENT:				// �ʐ��Z
		// FlashROM + SRAM LOG�o�^�����iTotal�jget
		LogCount = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP_STP );	// �o�^����set(�s���E�����o�ɂ�����)
		break;

	case	LOG_TTOTAL:					// T���v
		// FlashROM + SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_TTOTAL );	// �o�^����set
		break;

	case	LOG_GTTOTAL:				// GT���v
		// FlashROM + SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_GTTOTAL );	// �o�^����set
		break;

	case	LOG_MONEYMANAGE:			// ���K�Ǘ�
		// SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_MNYMNG_SRAM );	// �o�^����set
		break;

	case	LOG_COINBOX:				// ��݋��ɍ��v
		// SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_COINBOX );	// �o�^����set
		break;

	case	LOG_NOTEBOX:				// �������ɍ��v
		// SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_NOTEBOX );	// �o�^����set
		break;

	case	LOG_ABNORMAL:				// �s���o��
		// FlashROM + SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_ABNORMAL );	// �o�^����set
		break;

	case	LOG_MNYMNG_SRAM:			// ���K�Ǘ����O�o�^(SRAM)
		// SRAM LOG�o�^����get
		LogCount = Ope_Log_TotalCountGet( eLOG_MNYMNG_SRAM );	// �o�^����set
		break;
	}

	return	LogCount;
}

/*[]-----------------------------------------------------------------------[]*/
/*| ��ݎ�o�����޲��LED��������											    |*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : �Ȃ�                                                     |*/
/*| RETURN VALUE : �Ȃ�                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                  |*/
/*| Date         : 2006-02-10                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_ReciptLedOff( void )
{
	LedReq( CN_TRAYLED, LED_OFF );				// ��ݎ�o�����޲��LED����
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			ret		1=���V�[�g�o�͉�
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/03/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		Ope_isPrinterReady(void)
{
	if(ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0) 
		return 0;
// MH810105(S) MH364301 �C���{�C�X�Ή��i�v�����^�I�[�v�����󎚏o�͔���ɒǉ�����j
	if(ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] != 0){
		return 0;
	}
// MH810105(E) MH364301 �C���{�C�X�Ή��i�v�����^�I�[�v�����󎚏o�͔���ɒǉ�����j
	if (OPECTL.Mnt_mod == 0) {
	// �ʏ펞
		if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM))
		// ڼ��������ʐM�s�ǔ�����
			return 0;
	}
	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			�W���[�i���v�����^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			ret		1=�o�͉�
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/08/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		Ope_isJPrinterReady(void)
{
	if (PrnJnlCheck() == OFF)
		return -1;
	if(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0)
		return 0;
// MH810105(S) MH364301 �C���{�C�X�Ή��i�v�����^�I�[�v�����󎚏o�͔���ɒǉ�����j
	if(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] != 0){
		return 0;
	}
// MH810105(E) MH364301 �C���{�C�X�Ή��i�v�����^�I�[�v�����󎚏o�͔���ɒǉ�����j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	if (ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
		return 0;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	if (OPECTL.Mnt_mod == 0) {
	// �ʏ펞
		if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM))
		// �ެ���������ʐM�s�ǔ�����
			return 0;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)
		) {
			return 0;
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	return 1;
}
/*[]-----------------------------------------------------------------------[]*/
/*| LCD�V�s�ڂ�ܰ�ݸޕ\��				   		                            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : time = ү���ޕ\������									|*/
/*|						    x500ms�̒l�i��F10=5sec�j						|*/
/*|							   0=���Ԏw��Ȃ��i�\���͖߂��Ȃ��j�B			|*/
/*|				   pMessage = �\�����b�Z�[�W�P�s���i30�����ւ��߲���j		|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-11-14                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope2_WarningDisp( short time, const uchar *pMessage )
{
	Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, pMessage );	// ү���ޔ��]�\��
	if( time ){									// �\�����Ԏw�肠��
		LagTim500ms( LAG500_ERROR_DISP_DELAY, (short)(time + 1), Ope2_WarningDispEnd );
												// �w�莞�Ԍ��ү���ޏ���
	}
}

void	Ope2_WarningDispEnd( void )
{
	if( Miryo_Exec_check() )					// ���ݖ������̏ꍇ��
		return;									// ��ʏ��������Ȃ�
	LagCan500ms( LAG500_ERROR_DISP_DELAY );
	Lcd_WmsgDisp_OFF();
	Lcd_WmsgDisp_OFF2();
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	ec_MessagePtnNum = 0;
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	ECCTL.ec_MessagePtnNum = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
}

// MH321800(S) G.So IC�N���W�b�g�Ή�
/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6��7�s�ڂɃ��[�j���O���T�C�N���b�N�\���J�n 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : interval		= ���b�Z�[�W�\���Ԋu						|*/
/*|						    		x500ms�̒l (��F10=5sec)				|*/
/*|						   			0���w�肳�ꂽ�ꍇ�T�C�N���b�N���Ȃ�		|*/
/*|				   err_num		= �\������G���[���b�Z�[�W(ERR_CHR[])�̔ԍ�	|*/
/*|				  					[0]:1�y�[�W6�s�ځA[1]:1�y�[�W7�s��		|*/
/*|				  					[2]:2�y�[�W6�s�ځA[3]:2�y�[�W7�s��		|*/
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
/*|				   mode			= 0=�ʏ�A1=���]							|*/
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//void	Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[] )
void	Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[], uchar mode )
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
{
	// ��ʕ\��
	if( DspSts == LCD_WMSG_ON || DspSts2 == LCD_WMSG_ON ) {
	// �G���[���b�Z�[�W�\����
		LagCan500ms(LAG500_ERROR_DISP_DELAY);		// �^�C�}�[�L�����Z��
	}

	if (interval > 0) {
		// �T�C�N���b�N�\���f�[�^�ێ�
		Ope2CyclicDisp.interval = (interval / 500) + 1;
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
		Ope2CyclicDisp.mode = mode;
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
		Ope2CyclicDisp.err_page[0][0] = err_num[0];			// 1�y�[�W��(6�s��)
		Ope2CyclicDisp.err_page[0][1] = err_num[1];			// 1�y�[�W��(7�s��)
		Ope2CyclicDisp.err_page[1][0] = err_num[2];			// 2�y�[�W��(6�s��)
		Ope2CyclicDisp.err_page[1][1] = err_num[3];			// 2�y�[�W��(7�s��)
		Ope2CyclicDisp.page = 1;							// 1�y�[�W�ڂ�\��

		// ����\��
		Ope2_ErrChrCyclicDisp();
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6��7�s�ڂɃ��[�j���O���T�C�N���b�N�\����~ 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	Ope2_ErrChrCyclicDispStop( void )
{
	if (Ope2CyclicDisp.page != 0) {
		LagCan500ms(LAG500_ERROR_DISP_DELAY);		// �^�C�}�[�L�����Z��
		Ope2CyclicDisp.page = 0;
		Lcd_WmsgDisp_OFF2();
		Lcd_WmsgDisp_OFF();
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| LCD 6��7�s�ڂɃ��[�j���O���T�C�N���b�N�\��	 	                        |*/
/*[]-----------------------------------------------------------------------[]*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                     |*/
/*| Date         : 2019-01-29                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	Ope2_ErrChrCyclicDisp( void )
{
	uchar	page;
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
	uchar	mode;
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�

	if (Ope2CyclicDisp.page == 0) {
		return;
	}

	// ��ʐ؂�ւ�
	page = Ope2CyclicDisp.page;
	page--;
	Lcd_WmsgDisp_OFF2();
	Lcd_WmsgDisp_OFF();
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//	Lcd_WmsgDisp_ON2( 0, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][0]] );	// ���b�Z�[�W�\��(6�s��)
//	Lcd_WmsgDisp_ON ( 0, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][1]] );	// ���b�Z�[�W�\��(7�s��)
	mode = Ope2CyclicDisp.mode;
	Lcd_WmsgDisp_ON2( (ushort)mode, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][0]] );	// ���b�Z�[�W�\��(6�s��)
	Lcd_WmsgDisp_ON ( (ushort)mode, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[Ope2CyclicDisp.err_page[page][1]] );	// ���b�Z�[�W�\��(7�s��)
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
	Ope2CyclicDisp.page = (page == 0) ? 2 : 1;

	// �w�莞�Ԍ�Ƀ��b�Z�[�W��؊�����
	LagTim500ms( LAG500_ERROR_DISP_DELAY, Ope2CyclicDisp.interval, Ope2_ErrChrCyclicDisp );
}
// MH321800(E) G.So IC�N���W�b�g�Ή�

short chkdate2(short yyyy, short mm, short dd)
{
	/* data  check */
	if((yyyy < 1980) || (yyyy > 2079)){
		return(-1);		/* YEAR  ERROR */
	}
	if((mm < 1) || (mm > 12)){
		return(-1);		/* MONTH ERROR */
	}
	if((dd < 1) || (dd > medget(yyyy,mm))){
		return(-1);		/* DAY   ERROR */
	}

	return(0);	/* NON ERROR */
}

/*[]-----------------------------------------------------------------------[]*/
/*| ���Z�f�[�^�����p�t�����Z���ҏW����		                            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetAddPayData( void )                                    |*/
/*| PARAM		 :	void													|*/
/*|	RETURN 	     :	void													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                              |*/
/*| Date         : 2012-02-13                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void SetAddPayData( void )
{
	int		i;
	int j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	date_time_rec	wk_CLK_REC;
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
// MH810100(S) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
	date_time_rec	wk_CLK_REC;
// MH810100(E) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	// NT-NET��{�f�[�^�Z�b�g
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ����ID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 22;							// ���O�ް�
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){							// ���Z���@=5:������X�V
			PayData.DataBasic.DataKind = 22;							// ���O�ް�
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 56;							// ���O�ް�
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){							// ���Z���@=5:������X�V
			PayData.DataBasic.DataKind = 56;							// ���O�ް�
		}
	}
	PayData.DataBasic.DataKeep = 0;									// �ް��ێ��׸�
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// ���ԏꇂ
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// �@����
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// �@�B��
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// �����N
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// ������
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// ������
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// ������
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// ������
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// �����b

	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// �Z���^�[�ǔ�
	}
	PayData.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�
	PayData.PayMethod = (uchar)OpeNtnetAddedInfo.PayMethod;			// ���Z���@�Z�b�g
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;			// �����敪�Z�b�g
	PayInfo_Class = PayData.PayClass;								// ���Z���f�[�^�p�����敪��Ҕ�
	if( PayData.chuusi == 1 ){										// ���Z���~
		PayData.PayClass += 2;
		// ���Z���~���̎������Z�b�g
		NTNetTime_152.Year = NTNetTime_152_wk.Year;					// �N
		NTNetTime_152.Mon  = NTNetTime_152_wk.Mon;					// ��
		NTNetTime_152.Day  = NTNetTime_152_wk.Day;					// ��
		NTNetTime_152.Hour = NTNetTime_152_wk.Hour;					// ��
		NTNetTime_152.Min  = NTNetTime_152_wk.Min;					// ��
		NTNetTime_152.Sec  = NTNetTime_152_wk.Sec;					// �b
	}
	if( PayData.PayMethod == 5 ){									// �X�V���Z��
		if( PayData.PayClass == 0 || PayData.PayClass == 1 ){
			PayData.PayClass = 10;									// �����敪�̔�����
		}
		else if( PayData.PayClass == 2 || PayData.PayClass == 3 ){
			PayData.PayClass = 11;									// �����敪�̔����
		}
	}

	// ntnet_decision_credit()���Őݒ���Q�Ƃ��Ă���̂ŁA�ȉ��̏������s��
	if( ( PayData.PayClass == 0 || PayData.PayClass == 1 ) ){
		if (ntnet_decision_credit(&PayData.credit)){				// �����z��PayData�������Ă���̂œ��ꂷ��
			if(PayData.credit.cre_type == CREDIT_HOJIN){
				PayData.PayClass += 14;								// 14:�@�l�J�[�h���Z / 15:�@�l�J�[�h�Đ��Z
			}else{
				PayData.PayClass += 4;								// 4:�ڼޯĐ��Z / 5:�ڼޯčĐ��Z
			}
		}
	}
	PayInfo_Class = PayData.PayClass;								// ���Z���f�[�^�p�����敪��Ҕ�
// �s��C��(S) K.Onodera 2016/10/05 #1507 �N���W�b�g���Z��̐��Z���f�[�^�v���̏����N���������Z�����ɂȂ�Ȃ�[���ʃo�ONo.1179]
//	if( PayData.PayClass == 0 ){
	if( PayData.PayClass == 0 || PayData.PayClass == 4 ){
// �s��C��(E) K.Onodera 2016/10/05 #1507 �N���W�b�g���Z��̐��Z���f�[�^�v���̏����N���������Z�����ɂȂ�Ȃ�[���ʃo�ONo.1179]
		// ���Z�J�n���̎������Z�b�g
		NTNetTime_152.Year = NTNetTime_152_wk.Year;					// �N
		NTNetTime_152.Mon  = NTNetTime_152_wk.Mon;					// ��
		NTNetTime_152.Day  = NTNetTime_152_wk.Day;					// ��
		NTNetTime_152.Hour = NTNetTime_152_wk.Hour;					// ��
		NTNetTime_152.Min  = NTNetTime_152_wk.Min;					// ��
		NTNetTime_152.Sec  = NTNetTime_152_wk.Sec;					// �b
	}

	for( i = 0; i < WTIK_USEMAX; i++ ){
		if (PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) break;
		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE) break;
	}
	if( i != WTIK_USEMAX ){
		// ���z/�U�֐��Z����
		PayData.PayMode = 4;										// ���ZӰ��(���u���Z)
	}else{
		PayData.PayMode = 0;										// ���ZӰ��(�������Z)
	}
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	for( i = 0; i < DETAIL_SYU_MAX; i++ ){
		if (PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2) break;
	}
	if( i != DETAIL_SYU_MAX ){
		PayData.PayMode = 4;									// ���ZӰ��(���u���Z)
	}else{
		if( !PayData.PayMode ){
			PayData.PayMode = OpeNtnetAddedInfo.PayMode;
		}
	}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);

	for (i = 0; i < WTIK_USEMAX; i++) {
		if (PayData.DiscountData[i].DiscSyu == NTNET_SYUSEI_1) break;
	}
	if(i < WTIK_USEMAX) {
		if( PayData.PayClass == 2 || PayData.PayClass == 3 ){		// ���Z���~�A�Đ��Z���~
			if(i < (WTIK_USEMAX-3)){// �I�[�o�[�A�N�Z�X�K�[�h
				memmove( &PayData.DiscountData[i], &PayData.DiscountData[i+3], sizeof( DISCOUNT_DATA )*(WTIK_USEMAX-(i+3)) );	// �������
				memset( &PayData.DiscountData[WTIK_USEMAX-3], 0, sizeof( DISCOUNT_DATA )*3);	// ���Z�ް��������쐬�ر�ر
			}
		}else{
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//			if( PayData.Zengakufg&0x02 ){							// ����������
//				if(i < (WTIK_USEMAX-2)){// �I�[�o�[�A�N�Z�X�K�[�h
//					PayData.DiscountData[i+2].Discount = PayData.MMTwari;	// �������z�Z�b�g
//				}
//			}else{
//				PayData.DiscountData[i].Discount = PayData.MMTwari;	// ���ߊz�Z�b�g
//			}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
		}
	}
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	// �Ō�Ƀ`�F�b�N����
	if( Ope_SyubetuWariCheck(&PayData.DiscountData[0]) == TRUE){
		// �N���A�����̂ł��炷
		// �O�ɂP�㏑���ňړ�
		memmove(&PayData.DiscountData[0],&PayData.DiscountData[1],sizeof(DISCOUNT_DATA)*(WTIK_USEMAX-1));
		// �Ō�����N���A���Ă���
		memset(&PayData.DiscountData[WTIK_USEMAX-1],0,sizeof(DISCOUNT_DATA));
	}
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j

	for(i=0; i<WTIK_USEMAX; i++){
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
	}

	if( ryo_buf.ryos_fg == 1 ){										// �̎��ؔ��s?
		PayData.ReceiptIssue = 1;									// �̎��؂���
		NTNET_Data152Save((void *)(&PayData.ReceiptIssue), NTNET_152_RECEIPTISSUE);
	}

	if( ryo_buf.ryo_flg >= 2 || PayData.PayMethod == 5 ){			// ��������Z����or������X�V��
		PayData.teiki.ParkingNo =
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	// ��������ԏꇂ
		NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);
		PayData.PassCheck = 1;										// ����߽�������Ȃ�
		if( prm_get( COM_PRM,S_PAS,(short)(2+10*(PayData.teiki.syu-1)),1,1 ) ){
			PayData.PassCheck = 0;									// ����߽��������
			if( SKIP_APASS_CHK ){									// ��������߽OFF
				PayData.PassCheck = 2;								// ����߽����(����OFF)
			}
		}
		if(( CPrmSS[S_NTN][27] == 0 )&&								// �o���ް�?
		   ( PayData.PayClass != 2 && PayData.PayClass != 3 )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(4+10*(PayData.teiki.syu-1)),1,1 ) )){	// �������ʖ����Ă���ݒ�?
			PayData.CountSet = 2;									// �ݎԶ���-1����
		}else{
			PayData.CountSet = 1;									// �ݎԶ��Ă��Ȃ�
		}
		if( PayData.PayMethod == 5 ){
			PayData.CountSet = 1;									// �ݎԶ��Ă��Ȃ�
		}
	}else{
		PayData.PassCheck = 1;										// ����߽����
		if(( CPrmSS[S_NTN][27] == 0 )&&								// �o���ް�?
		   ( PayData.PayClass != 2 && PayData.PayClass != 3 )&&
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		   ( PayData.PayMethod != 13 ) &&							// ������Z�łȂ��H
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		   ( prm_get( COM_PRM,S_SHA,(short)(2+((PayData.syu-1)*6)),1,1 ) )){	// ��ʖ����Ă���ݒ�?
			PayData.CountSet = 2;									// �ݎԶ���-1����
		}else{
			PayData.CountSet = 1;									// �ݎԶ��Ă��Ȃ�
		}
	}
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);
	if( PayData.OutKind == 98 ){
		PayData.CountSet = 1;										// �ݎԶ��Ă��Ȃ�
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	PayData.credit.CreditDate[0] = OpeNtnetAddedInfo.CreditDate[0];	// �ڼޯĶ��ޗL������(�N)
//	PayData.credit.CreditDate[1] = OpeNtnetAddedInfo.CreditDate[1];	// �ڼޯĶ��ޗL������(��)
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	// ������ް���0�Œ�
//	PayData.PascarCnt = 0;					// ����ԗ��J�E���g
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	if (PayData.ID != 56) {					// 22/56
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// ������ʖ��Ɍ��ݍݎԶ���
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
					curnum++;
					curnum_1++;
				}
				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
					curnum++;
					curnum_2++;
				}
				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
					curnum++;
					curnum_3++;
				}
			}
		}
	}
	// ����ԗ��J�E���g 0�Œ�
	if (_is_ntnet_remote()) {												// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum;									// ���ݑ䐔
//				switch (PPrmSS[S_P02][1]) {									// ���������Ӱ��
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
//			case 4:															// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum_1;									// ���ݑ䐔�P
//				switch (PPrmSS[S_P02][5]) {									// ��������ԃ��[�h1
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// ���ԑ䐔�Q
//				PayData.CarCnt2 = curnum_2;									// ���ݑ䐔�Q
//				switch (PPrmSS[S_P02][9]) {									// ��������ԃ��[�h2
//				case	1:													// ��������
//					PayData.Full[1] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[1] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];// ���ԑ䐔�R
//					PayData.CarCnt3 = curnum_3;								// ���ݑ䐔�R
//					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h3
//					case	1:												// ��������
//						PayData.Full[2] = 11;								// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:												// �������
//						PayData.Full[2] = 10;								// �u������ԁv��ԃZ�b�g
//						break;
//					default:												// ����
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//			// �䐔�Ǘ��ǔ� ------------------------
//			memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
//			// ���ꎞ���H
//			if( GetVehicleCountDate() == Nrm_YMDHM( &wk_CLK_REC ) ){
//				// ���ꎞ���̏ꍇ�͒ǂ��Ԃ��{�P
//				AddVehicleCountSeqNo();
//			}else{
//				// �قȂ鎞���̏ꍇ�͒ǂ��Ԃ��O�Ƃ���
//				SetVehicleCountDate( Nrm_YMDHM( &wk_CLK_REC ) );
//				ClrVehicleCountSeqNo();
//			}
//			PayData.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// �N
//			PayData.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// ��
//			PayData.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// ��
//			PayData.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// ��
//			PayData.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// ��
//			PayData.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// �ǔ�
//			PayData.CarCntInfo.Reserve1		= 0;						// �\��
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//		}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
// MH810100(S) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
			// �䐔�Ǘ��ǔ� ------------------------
			memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
			// ���ꎞ���H
			if( GetVehicleCountDate() == Nrm_YMDHM( &wk_CLK_REC ) ){
				// ���ꎞ���̏ꍇ�͒ǂ��Ԃ��{�P
				AddVehicleCountSeqNo();
			}else{
				// �قȂ鎞���̏ꍇ�͒ǂ��Ԃ��O�Ƃ���
				SetVehicleCountDate( Nrm_YMDHM( &wk_CLK_REC ) );
				ClrVehicleCountSeqNo();
			}
			PayData.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// �N
			PayData.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// ��
			PayData.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// ��
			PayData.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// ��
			PayData.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// ��
			PayData.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// �ǔ�
			PayData.CarCntInfo.Reserve1		= 0;						// �\��
// MH810100(E) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �s���E�����o�ɗp1���Z����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set_SK( void )                                  |*/
/*| PARAMETER    : pr_lokno  : ���������p���Ԉʒu�ԍ�(1�`324)              |*/
/*|              : paymethod : ���Z����                                    |*/
/*|              : payclass  : �����敪                                    |*/
/*|              : outkind   : 0=�ʏ퐸�Z, 1=�����o��                      |*/
/*|              :             2=���Z���Ȃ��o��, 3=�s���o��                |*/
/*|              : fustype   : 0=���Z���Ȃ��o��, 1=�i�Y���Ȃ��j            |*/
/*|                            2=�C���̕s��(�����L), 3=�C���̕s��(������)  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	PayData_set_SK( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind, uchar fustype )
{
	ushort	num;
	int i, j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	num = pr_lokno - 1;

	PayData.WFlag = 0;												// �󎚎�ʂP�i0:�ʏ� 1:���d�j
	PayData.chuusi = 0;												// �󎚎�ʂQ�i0:�̎��� 1:���Z���~ 2:�a��؁j
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// �@�B��
	PayData.Seisan_kind = 0;										// ���Z���(�����Œ�)

	if(outkind == 1 || outkind == 11 ){
		CountGet( KIYOUSEI_COUNT, &PayData.Oiban );					// �����o�ɒǔ�
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else if(outkind == 3){
		CountGet( FUSEI_COUNT, &PayData.Oiban );					// �s���o�ɒǔ�
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}else{
		PayData.Oiban.i = 0L;										// ���Z�ǂ���
		PayData.Oiban.w = 0L;										// ���Z�ǂ���
		NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);
	}
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// �Z���^�[�ǔ�
	}
	PayData.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�

	PayData.PayMethod = (uchar)paymethod;							// ���Z���@
	if( (fustype == 2)||(fustype == 3) ){							// �C�����Z�ł̕s��
		PayData.PayMethod = 10;										// ���Z���@(�C�����敪10)
	}
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)payclass;								// �����敪
	PayInfo_Class = PayData.PayClass;								// ���Z���f�[�^�p�����敪��Ҕ�
	PayData.PayMode = 0;											// ���ZӰ��
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
//																	// �ڋq�p���Ԉʒu�ԍ�
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	PayData.TOutTime.Year = car_ot_f.year;							// �o��	�N
	PayData.TOutTime.Mon  = car_ot_f.mon;							//		��
	PayData.TOutTime.Day  = car_ot_f.day;							//		��
	PayData.TOutTime.Hour = car_ot_f.hour;							//		��
	PayData.TOutTime.Min  = car_ot_f.min;							//		��

	if(outkind == 1 || outkind == 11 ){
		PayData.KakariNo = Kakari_Numu[num];						// �W����
	}else{
		PayData.KakariNo = 0;										// �W����
	}
	NTNET_Data152Save((void *)(&PayData.KakariNo), NTNET_152_KAKARINO);
	PayData.OutKind = (uchar)outkind;								// ���Z�o��
	NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
	PayData.TInTime.Year = car_in_f.year;							// ����	�N
	PayData.TInTime.Mon  = car_in_f.mon;							//		��
	PayData.TInTime.Day  = car_in_f.day;							//		��
	PayData.TInTime.Hour = car_in_f.hour;							//		��
	PayData.TInTime.Min  = car_in_f.min;							//		��

	PayData.syu = ryo_buf.tik_syu;									// �������
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.WPrice = ryo_buf.dsp_ryo;								// ���ԗ���
	PayData.Wtax = ryo_buf.tax;										// �����
	if( fustype == 3 ){												// �C�����Z�̗������������ł̕s���ް�
		PayData.WPrice = 0;											// ���ԗ���
		PayData.Wtax = 0;											// ����Ŋz
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.PassCheck = 1;											// ����߽����
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);

	if(( CPrmSS[S_NTN][27] == 0 )&&									// �o���ް�?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// ��ʖ����Ă���ݒ�?
		PayData.CountSet = 2;										// �ݎԶ���-1����
		if( fustype == 2 ){											// �C�����Z�̍ݎԶ��ĂȂ��̕s���ް�
			PayData.CountSet = 1;									// �ݎԶ��Ă��Ȃ�
		}
	}else{
		PayData.CountSet = 1;										// �ݎԶ��Ă��Ȃ�
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
	PayData.PayCalMax = ntnet_nmax_flg;								// �ő嗿���z������

	// NT-NET��{�f�[�^�Z�b�g
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ����ID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 22;							// ���O�ް�
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 56;							// ���O�ް�
		}
	}
	PayData.DataBasic.DataKeep = 0;									// �ް��ێ��׸�
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// ���ԏꇂ
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// �@����
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// �@�B��
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// �����N
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// ������
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// ������
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// ������
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// ������
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// �����b

// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	// ������ް���0�Œ�
//	PayData.PascarCnt = 0;					// ����ԗ��J�E���g
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	if (PayData.ID != 56) {					// 22/56
		// ���Z���f�[�^�p�����E�s���o�Ƀf�[�^��Ҕ�
		memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// ������ʖ��Ɍ��ݍݎԶ���
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
					curnum++;
					curnum_1++;
				}
				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
					curnum++;
					curnum_2++;
				}
				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
					curnum++;
					curnum_3++;
				}
			}
		}
	}
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	// ����ԗ��J�E���g 0�Œ�
//	if (_is_ntnet_remote()) {												// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum;									// ���ݑ䐔
//				switch (PPrmSS[S_P02][1]) {									// ���������Ӱ��
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
//			case 4:															// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum_1;									// ���ݑ䐔�P
//				switch (PPrmSS[S_P02][5]) {									// ��������ԃ��[�h1
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// ���ԑ䐔�Q
//				PayData.CarCnt2 = curnum_2;									// ���ݑ䐔�Q
//				switch (PPrmSS[S_P02][9]) {									// ��������ԃ��[�h2
//				case	1:													// ��������
//					PayData.Full[1] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[1] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];			// ���ԑ䐔�R
//					PayData.CarCnt3 = curnum_3;								// ���ݑ䐔�R
//					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h3
//					case	1:												// ��������
//						PayData.Full[2] = 11;								// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:												// �������
//						PayData.Full[2] = 10;								// �u������ԁv��ԃZ�b�g
//						break;
//					default:												// ����
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//		}
//	}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	// ���Z���f�[�^�p�����E�s���o�Ƀf�[�^��Ҕ�
	memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
}
// �\���̂̃����o�[�̃I�t�Z�b�g�����߂�}�N��
#define OFS_MEMBER(x, y)	( (ulong)&((x*)wp)-> ## y - (ulong)wp )

//[]----------------------------------------------------------------------[]
///	@brief		LOG���R�[�h������t�f�[�^�𓾂�
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: LOG���
///	@return		ushort		: �I�t�Z�b�g�l
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/03/12<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static date_time_rec *get_log_date_time_rec( ushort LogSyu, uchar *record )
{
	ushort	ofs;
	void	*wp;
	wp		= 0;

	switch( LogSyu ){
	case eLOG_PAYMENT:
		ofs = OFS_MEMBER(Receipt_data, TOutTime);
		break;
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	case eLOG_ENTER:
//		ofs = OFS_MEMBER(enter_log, InTime);
//		break;
	case eLOG_RTPAY:
		ofs = OFS_MEMBER(RTPay_log_date, dtTimeYtoSec);
		break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	case eLOG_RTRECEIPT:
		ofs = OFS_MEMBER(RTReceipt_log_date, dtTimeYtoSec);
		break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	case eLOG_TTOTAL:
		ofs = OFS_MEMBER(SYUKEI, NowTime);
		break;
	case eLOG_ERROR:
		ofs = OFS_MEMBER(Err_log, Date_Time);
		break;
	case eLOG_ALARM:
		ofs = OFS_MEMBER(Arm_log, Date_Time);
		break;
	case eLOG_OPERATE:
		ofs = OFS_MEMBER(Ope_log, Date_Time);
		break;
	case eLOG_MONITOR:
		ofs = OFS_MEMBER(Mon_log, Date_Time);
		break;
	case eLOG_ABNORMAL:
		ofs = OFS_MEMBER(flp_log, Date_Time);
		break;
	case eLOG_MONEYMANAGE:
		ofs = OFS_MEMBER(TURI_KAN, NowTime);
		break;
	case eLOG_PARKING:
		ofs = OFS_MEMBER(ParkCar_log, Time);
		break;
	case eLOG_NGLOG:
		ofs = OFS_MEMBER(NGLOG_DATA, NowTime);
		break;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	case eLOG_IOLOG:
//		ofs = OFS_MEMBER(IOLOG_DATA, NowTime);
	case eLOG_DC_QR:
		ofs = OFS_MEMBER(DC_QR_log_date, dtTimeYtoSec);
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
		break;
	case eLOG_CREUSE:
	case eLOG_HOJIN_USE:
		ofs = OFS_MEMBER(meisai_log, PayTime);
		break;
	case eLOG_REMOTE_SET:
		ofs = OFS_MEMBER(t_Change_data, rcv_Time);
		break;
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//	case eLOG_LONGPARK:
//		ofs = OFS_MEMBER(LongPark_log, Time);
	case eLOG_LONGPARK_PWEB:
		ofs = OFS_MEMBER(LongPark_log_Pweb, ProcDate);
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
		break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	case eLOG_RISMEVENT:
//		ofs = OFS_MEMBER(RismEvent_log, Time);
	case eLOG_DC_LANE:
		ofs = OFS_MEMBER(DC_LANE_log_date, dtTimeYtoSec);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		break;
	case eLOG_GTTOTAL:
		ofs = OFS_MEMBER(SYUKEI, NowTime);
		break;
	case eLOG_COINBOX:
		ofs = OFS_MEMBER(COIN_SYU, NowTime);
		break;
	case eLOG_NOTEBOX:
		ofs = OFS_MEMBER(NOTE_SYU, NowTime);
		break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case eLOG_EDYARM:
//		ofs = OFS_MEMBER(edy_arm_log, TOutTime);
//		break;
//	case eLOG_EDYSHIME:
//		ofs = OFS_MEMBER(edy_shime_log, ShimeTime);
//		break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case eLOG_POWERON:
		ofs = OFS_MEMBER(Pon_log, Pdw_Date);	// ��d����
		break;
	case eLOG_MNYMNG_SRAM:
		ofs = OFS_MEMBER(TURI_KAN, NowTime);
		break;
	default:	// parameter error
		ofs = 0;
		break;
	}
	return (date_time_rec*)(record+ofs);
}
/*[]----------------------------------------------------------------------[]*/
/*| �t���b�v�㏸�A���b�N����ϓ��o�ɗp1���Z����                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PayData_set_LO( void )                                  |*/
/*| PARAMETER    : pr_lokno  : ���������p���Ԉʒu�ԍ�(1�`324)              |*/
/*|              : paymethod : ���Z����                                    |*/
/*|              : payclass  : �����敪                                    |*/
/*|              : outkind   : 0=�ʏ퐸�Z, 1=�����o��                      |*/
/*|              :             2=���Z���Ȃ��o��, 3=�s���o��                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : A.iiizumi                                               |*/
/*| Date         : 2012-06-14                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	PayData_set_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind)
{
	ushort	num;
	int i, j;
	ushort curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	num = pr_lokno - 1;

	PayData.WFlag = 0;												// �󎚎�ʂP�i0:�ʏ� 1:���d�j
	PayData.chuusi = 0;												// �󎚎�ʂQ�i0:�̎��� 1:���Z���~ 2:�a��؁j
	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// �@�B��
	PayData.Seisan_kind = 0;										// ���Z���(�����Œ�)
	CountGet( PAYMENT_COUNT, &PayData.Oiban );						// ���Z�ǂ���
	NTNET_Data152Save((void *)&PayData.Oiban, NTNET_152_OIBAN);

	PayData.PayMethod = (uchar)paymethod;							// ���Z���@
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);

	PayData.PayClass = (uchar)payclass;								// �����敪
	PayData.PayMode = 0;											// ���ZӰ��
	NTNET_Data152Save((void *)&PayData.PayMode, NTNET_152_PAYMODE);
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
//																	// �ڋq�p���Ԉʒu�ԍ�
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	PayData.TOutTime.Year = car_ot_f.year;							// �o��	�N
	PayData.TOutTime.Mon  = car_ot_f.mon;							//		��
	PayData.TOutTime.Day  = car_ot_f.day;							//		��
	PayData.TOutTime.Hour = car_ot_f.hour;							//		��
	PayData.TOutTime.Min  = car_ot_f.min;							//		��

	PayData.KakariNo = 0;											// �W����
	NTNET_Data152Save((void *)(&PayData.KakariNo), NTNET_152_KAKARINO);
	PayData.OutKind = (uchar)outkind;								// ���Z�o��
	NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
	PayData.TInTime.Year = car_in_f.year;							// ����	�N
	PayData.TInTime.Mon  = car_in_f.mon;							//		��
	PayData.TInTime.Day  = car_in_f.day;							//		��
	PayData.TInTime.Hour = car_in_f.hour;							//		��
	PayData.TInTime.Min  = car_in_f.min;							//		��

	PayData.Wtax = 0;												// �����
	PayData.syu = (ushort)LockInfo[num].ryo_syu;					// �������
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);
	PayData.WPrice = 0;												// ���ԗ���
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.PassCheck = 1;											// ����߽����
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);

	if(( CPrmSS[S_NTN][27] == 0 )&&									// �o���ް�?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// ��ʖ����Ă���ݒ�?
		PayData.CountSet = 2;										// �ݎԶ���-1����
	}else{
		PayData.CountSet = 1;										// �ݎԶ��Ă��Ȃ�
	}
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);

	// NT-NET��{�f�[�^�Z�b�g
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ����ID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 22;							// ���O�ް�
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;							// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 56;							// ���O�ް�
		}
	}
	PayData.DataBasic.DataKeep = 0;									// �ް��ێ��׸�
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// ���ԏꇂ
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// �@����
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// �@�B��
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// �����N
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// ������
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// ������
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// ������
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// ������
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// �����b

	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		PayData.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_PAY);	// �Z���^�[�ǔ�
	}
	PayData.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�
	
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	// ������ް���0�Œ�
//	PayData.PascarCnt = 0;					// ����ԗ��J�E���g
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	if (PayData.ID != 56) {					// 22/56
		// �t���b�v�㏸�A���b�N����ϓ��o�Ƀf�[�^��Ҕ�
		PayInfo_Class = 102;
		memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
		return;
	}
	for (i = 0; i < LOCK_MAX; i++) {
		// ������ʖ��Ɍ��ݍݎԶ���
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
					curnum++;
					curnum_1++;
				}
				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
					curnum++;
					curnum_2++;
				}
				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
					curnum++;
					curnum_3++;
				}
			}
		}
	}
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	// ����ԗ��J�E���g 0�Œ�
//	if (_is_ntnet_remote()) {												// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][3];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum;									// ���ݑ䐔
//				switch (PPrmSS[S_P02][1]) {									// ���������Ӱ��
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum >= PPrmSS[S_P02][3]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
//			case 4:															// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
//				PayData.FullNo1 = (ushort)PPrmSS[S_P02][7];					// ���ԑ䐔
//				PayData.CarCnt1 = curnum_1;									// ���ݑ䐔�P
//				switch (PPrmSS[S_P02][5]) {									// ��������ԃ��[�h1
//				case	1:													// ��������
//					PayData.Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[0] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						PayData.Full[0] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				PayData.FullNo2 = (ushort)PPrmSS[S_P02][11];				// ���ԑ䐔�Q
//				PayData.CarCnt2 = curnum_2;									// ���ݑ䐔�Q
//				switch (PPrmSS[S_P02][9]) {									// ��������ԃ��[�h2
//				case	1:													// ��������
//					PayData.Full[1] = 11;									// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					PayData.Full[1] = 10;									// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						PayData.Full[1] = 1;								// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				if (i == 4) {
//					PayData.FullNo3 = (ushort)PPrmSS[S_P02][15];			// ���ԑ䐔�R
//					PayData.CarCnt3 = curnum_3;								// ���ݑ䐔�R
//					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h3
//					case	1:												// ��������
//						PayData.Full[2] = 11;								// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:												// �������
//						PayData.Full[2] = 10;								// �u������ԁv��ԃZ�b�g
//						break;
//					default:												// ����
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							PayData.Full[2] = 1;							// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//		}
//	}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	// �t���b�v�㏸�A���b�N����ϓ��o�Ƀf�[�^��Ҕ�
	PayInfo_Class = 102;
	memcpy(&PayInfoData_SK, &PayData, sizeof(PayData));
}

//[]----------------------------------------------------------------------[]
///	@brief			�W��������������
//[]----------------------------------------------------------------------[]
///	@return			ret : 1:OK -1:���޴װ 0:�s���������� 
///	@author			okuda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/12/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
short	Kakariin_card( void )
{
	short		ret = 1;	// �߂�l
	short		anm = 0;	// �ē�ү���ޔԍ�
	m_kakari	*crd_data;	// ���C�����ް��߲��
	long		pk_no;		// ���ԏ�m���D
	char		card_read= OK;	// �J�[�h�L���t���O
	uchar		id_syu_settei;	// �g�p��ID
	uchar		data[2];		// LOG�o�^���Ɏg�p����W���ԍ�

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	if (prm_get(COM_PRM, S_PAY, 10, 1, 4) != 0) {// �V�J�[�h�h�c�g�p����H
	//�g�p����
		id_syu_settei = (uchar)prm_get(COM_PRM,S_PAY,10,1,1);	// �g�p���h�c��ݒ�擾
		switch( id_syu_settei ){
		case 1:
		case 2:
		case 3:
			id_syu_settei = (uchar)(id_syu_settei - 1);
			break;
		case 8:
			id_syu_settei = 5;
			break;
		default:// �ǂ܂Ȃ�,���̑�(�ݒ�G���[)
			if(MAGred[MAG_GT_APS_TYPE] == 1){				// GT�t�H�[�}�b�g
				id_syu_settei = 0;
			}else{
				ret = -1;
				anm = AVM_CARD_ERR1;
				card_read = NG;
			}
			break;
		}
		// �W���J�[�h�`�F�b�N
		if(card_read == OK){
// NOTE:Read_Tik_Chk()�Łu����ʁv�͋����I��0x41�ɏ�����������B����NG�̏ꍇ�ARead_Tik_Chk()�Œ��ԏ�No��NG�̂��̂ɋ����I
// �ɏ��������邽�߂��̊֐��ł̔���͌���ʂ�ID1�ł��邱�Ƃ𔻒肷��Ηǂ�
			if( crd_data->kkr_idc == 0x41 ){// �����:�W���J�[�h
				if( ( crd_data->kkr_did != 0x53 )// �f�[�^�h�c=53H
						||
					( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&	// APS�t�H�[�}�b�g��PAK
					( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
						||
					( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&	// GT�t�H�[�}�b�g��PGT
					( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){
					// �W���J�[�h�ȊO
					ret = -1;
					anm = AVM_CARD_ERR1;
					card_read = NG;
				}
					///////�����̃p�X�ɗ������̂݌W���J�[�h�̒��g�𔻒肷��
			} else {
				// �W���J�[�h�ȊO
				ret = -1;
				anm = AVM_CARD_ERR1;
				card_read = NG;
			}
		}
	} else {
	//�g�p���Ȃ�
		// �W���J�[�h�`�F�b�N
		if(	( crd_data->kkr_idc != 0x41 )
				||
			( crd_data->kkr_did != 0x53 )
				||
			( (MAGred[MAG_GT_APS_TYPE] == 0 ) &&
				( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x41 || crd_data->kkr_park[2] != 0x4b ) ) 
				||
			( (MAGred[MAG_GT_APS_TYPE] == 1 ) &&
				( crd_data->kkr_park[0] != 0x50 ||  crd_data->kkr_park[1] != 0x47 || crd_data->kkr_park[2] != 0x54 ) ) ){
			// �W���J�[�h�ȊO
			ret = -1;
			anm = 44;
			card_read = NG;
		}
	///////�����̃p�X�ɗ������̂݌W���J�[�h�̒��g�𔻒肷��
	}
	
	if(card_read == OK){
		// �W���J�[�h
		for( ; ; ){
			if( crd_data->kkr_type != 0x20 ){							// �W���J�[�h�ł͂Ȃ�
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			if( MAGred[MAG_GT_APS_TYPE] == 1 ){//GT�t�H�[�}�b�g

				pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +		// ���ԏ�m���D�擾
							( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
							( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);

			}else{//APS�t�H�[�}�b�g
				pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// ���ԏ�m���D�擾
							( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
							( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
							( (crd_data->kkr_pno[3] & 0x0f) * 1L )
						);
			}

			if( CPrmSS[S_SYS][1] != pk_no ){							// ���ԏ�m���D����{���ԏ�m���D�H
				// ���ԏ�m���D����v���Ȃ��ꍇ
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			//GT�t�H�[�}�b�g�`�F�b�N
			if( ( prm_get( COM_PRM,S_SYS,12,1,6 ) == 0 ) && (MAGred[MAG_GT_APS_TYPE] == 1) ||
				( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1 ) && (MAGred[MAG_GT_APS_TYPE] == 0) ){
				//GT�t�H�[�}�b�g���Ή��̏ꍇ
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}

			/* �W���ԍ� 01�`99���� */
			pk_no = (	( (crd_data->kkr_kno[0] & 0x0f) * 1000 ) +		// �W���m���D�擾
						( (crd_data->kkr_kno[1] & 0x0f) * 100 )  +
						( (crd_data->kkr_kno[2] & 0x0f) * 10 )   +
						( (crd_data->kkr_kno[3] & 0x0f) * 1 )
					);

			if( (pk_no < 1) || (99 < pk_no) ){
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}

			/* �������� 1�`4���� */
			if( (crd_data->kkr_role < '1') || ('4' < crd_data->kkr_role) ){
				ret = -1;
				anm = AVM_CARD_ERR1;
				break;
			}
			memcpy(data,&crd_data->kkr_kno[2],sizeof(data));			// ����No�̎擾	
			if(pk_no != 99 && -1 == AteVaild_Check((ushort)pk_no)) {	// �L���W���f�[�^�o�^�Ȃ��H(No.99�̓`�F�b�N�ΏۊO)
				NgLog_write( NG_CARD_AMANO, data, sizeof(data) );		// �s������LOG�o�^���s��
				ret = 0;
				anm = AVM_CARD_ERR3;									// ���̃J�[�h�͖����ł��B
				break;
			}
			
			// �����܂�break����Ȃ��̂ŁA����J�[�h
			NgLog_write( OK_CARD_AMANO, data, sizeof(data) );			// ����J�[�hLOG�o�^���s��
			break;														// �J�[�h�`�F�b�N�n�j
		}

	}
	if( anm != 0 ){
		// �ē�ү���ޑ��o����̏ꍇ
		ope_anm(anm);
	}
	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			�W���J�[�h���̖����R�[�h�ƌW������HEX�l�ŕԂ�
//[]----------------------------------------------------------------------[]
///	@param[in]		pMntLevel	����ݽ���ْl(1-4)��Ĵر�ւ��߲��
///	@param[in]		pPassLevel	�߽ܰ������(0-5)��Ĵر�ւ��߲��
///	@param[in]		pKakariNum	�W���ԍ�(0-254)��Ĵر�ւ��߲��
///	@return			ret			1=���W�F�N�g���{�A0=���{���ĂȂ�
///	@author			Okuda
///	@note			MAG_red[] �ɌW���J�[�h��񂪂���ꍇ�̂ݗL���B<br>
///					�����e�i���X���x����1�`4�̒l��Ԃ��i0�̏ꍇ1�Ƃ���j
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/12/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	Ope_KakariCardInfoGet( uchar *pMntLevel, uchar* pPassLevel, uchar *pKakariNum )
{
	m_kakari	*crd_data;							// ���C�����ް��߲��
	ushort		wkus;

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];

	wkus = (	( (crd_data->kkr_kno[0] & 0x0f) * 1000 ) +		// �W���m���D�擾
				( (crd_data->kkr_kno[1] & 0x0f) * 100 )  +
				( (crd_data->kkr_kno[2] & 0x0f) * 10 )   +
				( (crd_data->kkr_kno[3] & 0x0f) * 1 )
			);
	*pKakariNum = (uchar)wkus;

	wkus = (ushort)(crd_data->kkr_role - '0');		// �����R�[�h Hex get
	if( (wkus < 1) || (4 < wkus) ){
		wkus = 1;
	}
	*pMntLevel = (uchar)wkus;
	
	wkus = (ushort)(crd_data->kkr_lev - '0');		// ���x��
	if( wkus > 5 ) {
		wkus = 0;
	}
	*pPassLevel = (uchar)wkus;
}
/*[]-----------------------------------------------------------------------[]*/
/*| ���Z���~���̐��Z���f�[�^�����p�t�����Z���ҏW����		            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetNoPayData( void )                                     |*/
/*| PARAM		 :	void													|*/
/*|	RETURN 	     :	void													|*/
/*[]-------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void SetNoPayData( void )
{
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	ushort num;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	PayData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		PayData.ID = 56;
	}

	// NT-NET��{�f�[�^�Z�b�g
	PayData.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];// ����ID
	if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 23;						// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 22;						// ���O�ް�
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){						// ���Z���@=5:������X�V
			PayData.DataBasic.DataKind = 22;						// ���O�ް�
		}
	} else {
		if( CPrmSS[S_NTN][27] == 0 ){
			PayData.DataBasic.DataKind = 57;						// �o���ް�
		}else{
			PayData.DataBasic.DataKind = 56;						// ���O�ް�
		}
		if( OpeNtnetAddedInfo.PayMethod == 5 ){						// ���Z���@=5:������X�V
			PayData.DataBasic.DataKind = 56;						// ���O�ް�
		}
	}
	PayData.DataBasic.DataKeep = 0;									// �ް��ێ��׸�
	PayData.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];			// ���ԏꇂ
	PayData.DataBasic.ModelCode = NTNET_MODEL_CODE;					// �@����
	PayData.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];			// �@�B��
	PayData.DataBasic.Year = (uchar)( CLK_REC.year % 100 );			// �����N
	PayData.DataBasic.Mon = (uchar)CLK_REC.mont;					// ������
	PayData.DataBasic.Day = (uchar)CLK_REC.date;					// ������
	PayData.DataBasic.Hour = (uchar)CLK_REC.hour;					// ������
	PayData.DataBasic.Min = (uchar)CLK_REC.minu;					// ������
	PayData.DataBasic.Sec = (uchar)CLK_REC.seco;					// �����b

	// ���Z���~���̎������Z�b�g
	NTNetTime_152.Year = CLK_REC.year;								// �N
	NTNetTime_152.Mon  = CLK_REC.mont;								// ��
	NTNetTime_152.Day  = CLK_REC.date;								// ��
	NTNetTime_152.Hour = CLK_REC.hour;								// ��
	NTNetTime_152.Min  = CLK_REC.minu;								// ��
	NTNetTime_152.Sec  = (ushort)CLK_REC.seco;						// �b

	PayData.PayMethod = (uchar)OpeNtnetAddedInfo.PayMethod;			// ���Z���@�Z�b�g
	NTNET_Data152Save((void *)(&PayData.PayMethod), NTNET_152_PAYMETHOD);
	PayData.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;			// �����敪�Z�b�g
	PayInfo_Class = PayData.PayClass;								// ���Z���f�[�^�p�����敪��Ҕ�
	if( PayData.PayMethod == 5 ){									// �X�V���Z��
		if( PayData.PayClass == 0 || PayData.PayClass == 1 ){
			PayData.PayClass = 10;									// �����敪�̔�����
		}
		else if( PayData.PayClass == 2 || PayData.PayClass == 3 ){
			PayData.PayClass = 11;									// �����敪�̔����
		}
	}

// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	num = ryo_buf.pkiti - 1;
//	PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// �ڋq�p���Ԉʒu�ԍ�
//	NTNET_Data152Save((void *)(&PayData.WPlace), NTNET_152_WPLACE);
	PayData.WPlace = 0L;
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	if( ryo_buf.ryo_flg < 2 ){										// ���Ԍ����Z����
		PayData.WPrice = ryo_buf.tyu_ryo;							// ���ԗ���
	}else{
		PayData.WPrice = ryo_buf.tei_ryo;							// �������
	}
	NTNET_Data152Save((void *)(&PayData.WPrice), NTNET_152_PRICE);
	PayData.syu = (char)(ryo_buf.syubet + 1);						// ���Ԏ��
	NTNET_Data152Save((void *)(&PayData.syu), NTNET_152_SYUBET);

	// ntnet_decision_credit()���Őݒ���Q�Ƃ��Ă���̂ŁA�ȉ��̏������s��
	if( ( PayData.PayClass == 0 || PayData.PayClass == 1 ) ){
		if (ntnet_decision_credit(&PayData.credit)){				// �����z��PayData�������Ă���̂œ��ꂷ��
			if(PayData.credit.cre_type == CREDIT_HOJIN){
				PayData.PayClass += 14;								// 14:�@�l�J�[�h���Z / 15:�@�l�J�[�h�Đ��Z
			}else{
				PayData.PayClass += 4;								// 4:�ڼޯĐ��Z / 5:�ڼޯčĐ��Z
			}
		}
	}
	PayInfo_Class = 2;												// �����敪:���Z���~
	PayData.CountSet = 1;											// �ݎԶ��Ă��Ȃ�
	NTNET_Data152Save((void *)(&PayData.CountSet), NTNET_152_COUNTSET);
	PayData.PassCheck = 1;											// ����߽�������Ȃ�
	NTNET_Data152Save((void *)(&PayData.PassCheck), NTNET_152_ANTIPASSCHECK);
}
/*[]-----------------------------------------------------------------------[]*/
/*| �w�����̗\�~������𒲂ׂ�								            |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : subtube_use_check( int )                                 |*/
/*| PARAM		 :	int		mny_type�i���� 0=10�~, 1=50�~, 2=100�~�j		|*/
/*|	RETURN 	     :	int		ret		�i0=���g�p, 1=�\�~�P, 2=�\�~�Q�j		|*/
/*[]-------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
int	subtube_use_check( int mny_type )
{
	int		i, ret = 0;
	uchar	bit, idx, tube[] = { 0, 0 };

	bit = 0x01;
	idx = 0;
	for( i=0; i<3; i++ ){
		if( CN_SUB_SET[i] ){
			tube[idx++] |= bit;
			if( idx > 1 ){
				break;
			}
		}
		bit <<= 1;
	}
	switch( mny_type ){
	case 0:		// 10�~
		if( tube[0] & 0x01 ){
			ret = 1;
		}
		if( tube[1] & 0x01 ){
			ret = 2;
		}
		break;
	case 1:		// 50�~
		if( tube[0] & 0x02 ){
			ret = 1;
		}
		if( tube[1] & 0x02 ){
			ret = 2;
		}
		break;
	case 2:		// 100�~
		if( tube[0] & 0x04 ){
			ret = 1;
		}
		if( tube[1] & 0x04 ){
			ret = 2;
		}
		break;
	}

	return ret;
}

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�������Z�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		f_Button	1=��������<br>
///								0=������
///	@return			ret			0=�������Z�����i�ݒ肪�����j<br>
///								1=OK�i�����v�Z�ް��܂ō쐬�ς݁j
///	@author			Okuda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/10/21<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	Ope_FunCheck( char f_Button )
{
	(void)vl_funchg( f_Button, (m_gtticstp*)MAGred );			// �����v�Z�ް��ݒ�

	return	(uchar)1;
}
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
//[]----------------------------------------------------------------------[]
///	@brief		���u���Z���Ɏ����w�萸�Z �f�[�^�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void Ope_Set_tyudata( void )
{
	vl_tikchg();
}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//[]----------------------------------------------------------------------[]
///	@brief		�J�[�h��񂩂�A�����v�Z�p�f�[�^�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_Set_tyudata_Card( void )
{
	vl_lcd_tikchg();
}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//[]----------------------------------------------------------------------[]
///	@brief		�������ԉ���
//[]----------------------------------------------------------------------[]
///	@param[in]	LockNo      : ���������p���Ԉʒu�ԍ�(1�`324) 
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)���Ή�
///	@param[in]	knd: 1=���Z 2=�����^�s�� 
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)���Ή�
///	@return		none
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/05/16<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//void	LongTermParkingRel( ulong LockNo )
void	LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
{
	ulong	ulwork;
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){	// �������Ԍ��o����
//		if((LockNo == 0)||(LockNo > 324)){
//			return;// �C���f�b�N�X�j��h�~
//		}else{
//			if(LongParkingFlag[LockNo-1] != 0){	// �������ԏ�ԁF�������ԏ�Ԃ���
//				ulwork = (ulong)(( LockInfo[LockNo-1].area * 10000L ) + LockInfo[LockNo-1].posi );	// �����擾
//				alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031���� �������Ԍ��o
//				LongParkingFlag[LockNo-1] = 0;	// �������ԏ�ԂȂ�
//			}
//		}
//	}
	uchar	prm_wk;
	ushort	wHour;
	ushort	wHour2;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�pSRAM�팸�j
//	if((LockNo == 0)||(LockNo > 324)){
	if((LockNo == 0)||(LockNo > LOCK_MAX)){
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�pSRAM�팸�j
		return;// �C���f�b�N�X�j��h�~
	}
	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);		// �������Ԍ��o����
	ulwork = (ulong)(( LockInfo[LockNo-1].area * 10000L ) + LockInfo[LockNo-1].posi );	// �����擾

	if(prm_wk == 0){	// �������Ԍ��o(�A���[���f�[�^�ɂ��ʒm)
		if(LongParkingFlag[LockNo-1] != 0){	// �������ԏ�ԁF�������ԏ�Ԃ���
			alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031���� �������Ԍ��o
			LongParkingFlag[LockNo-1] = 0;	// �������ԏ�ԂȂ�
		}
	}
	else if(prm_wk == 2){	// �������Ԍ��o(�A���[���f�[�^�ɂ��ʒm+�������ԃf�[�^�ɂ��ʒm)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// �������Ԍ��o����1
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// �ݒ肪0�̏ꍇ�͋����I��48���ԂƂ���
		}
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// �������Ԍ��o����2
		// �������Ԍ��o����1�̉���(�A���[���f�[�^�ɂ��ʒm)
		if(LongParkingFlag[LockNo-1] != 0){	// �������ԏ�ԁF�������ԏ�Ԃ���
			alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);// A0031���� �������Ԍ��o
			LongParkingFlag[LockNo-1] = 0;	// �������ԏ�ԂȂ�
		}

		// ���Z�^�o�ɂɂ�蒷�����Ԃ���������Ƃ��͒������ԃf�[�^�̉����̑��M�͒�������1�܂��͒�������2�̂ǂ��炩��ŗǂ�
		// ParkingWeb�͎Ԏ��P�ʂŒ������Ԃ𔻒f���邽��
		
		if(wHour2 == 0){
			// �������Ԍ��o����1�̉���(�������ԃf�[�^�ɂ��ʒm)
			if(flp->flp_state.BIT.b00 != 0){	// ��������1��ԁF�������ԏ�Ԃ���
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour, LONGPARK_LOG_RESET, knd);// ��������1�F���� �������ԃf�[�^���O����
				Log_regist( LOG_LONGPARK );// �������ԃf�[�^���O�o�^
				flp->flp_state.BIT.b00 = 0;	// ��������1��ԂȂ�
				flp->flp_state.BIT.b01 = 0;	// ��������2��ԂȂ�
			}
		}else{
			// �������Ԍ��o����2�̉���(�������ԃf�[�^�ɂ��ʒm)
			if(flp->flp_state.BIT.b01 != 0){		// ��������2��ԁF�������ԏ�Ԃ���
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour2, LONGPARK_LOG_RESET, knd);// ��������2�F���� �������ԃf�[�^���O����
				Log_regist( LOG_LONGPARK );// �������ԃf�[�^���O�o�^
				flp->flp_state.BIT.b00 = 0;	// ��������1��ԂȂ�
				flp->flp_state.BIT.b01 = 0;	// ��������2��ԂȂ�
			}else if(flp->flp_state.BIT.b00 != 0){	// ��������1��ԁF�������ԏ�Ԃ���
				Make_Log_LongParking_Pweb( (ushort)LockNo, wHour, LONGPARK_LOG_RESET, knd);// ��������1�F���� �������ԃf�[�^���O����
				Log_regist( LOG_LONGPARK );// �������ԃf�[�^���O�o�^
				flp->flp_state.BIT.b00 = 0;	// ��������1��ԂȂ�
				flp->flp_state.BIT.b01 = 0;	// ��������2��ԂȂ�
			}
		}
	}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
}
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�

// MH810100(S) 2020/09/02 #4803 [08-0001�F�S�T�[�r�X���g�p���x����]�Őݒ肳��Ă��閇���𒴂����ۂɕ\�������POPUP���A�\������Ȃ�
//// MH810100(S) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//BOOL IsBackupAreaFound(int nCheckIndex )
//{
//	BOOL bRet = FALSE;
//	int i = 0;
//	
//	for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
//		if( memcmp(&DiscountBackUpArea[i],&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[nCheckIndex],sizeof(stDiscount2_t)) == 0){
//			bRet = TRUE;
//			break;
//		}
//	}
//	return bRet;
//	
////	ulong	DiscParkNo;				// ���ԏꇂ(0�`999999)
////	ushort 	DiscSyu;				// ���(0�`9999)
////	ushort 	DiscCardNo;				// ���ދ敪(0�`65000)
////	uchar 	DiscNo;					// �敪(0�`9)
////	uchar 	DiscCount;				// ����(0�`99)
////	ulong 	Discount;				// ���z/����(0�`999999)
////	ulong	UsedDisc;				// �����������p���̎g�p�ς݊����i���z/���ԁj(0�`999999)
////	ushort 	DiscInfo;				// �������(0�`65000)
////	ushort 	DiscCorrType;			// �Ή����ގ��(0�`65000)
////	uchar 	DiscStatus;				// �ð��(0�`9)
////	uchar 	DiscFlg;				// ������(0�`9)
//
//}
//// MH810100(E) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
// MH810100(E) 2020/09/02 #4803 [08-0001�F�S�T�[�r�X���g�p���x����]�Őݒ肳��Ă��閇���𒴂����ۂɕ\�������POPUP���A�\������Ȃ�

// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
//[]----------------------------------------------------------------------[]
/// @brief		���x�����I�[�o�[���m�F���邩
//[]----------------------------------------------------------------------[]
///	@param[in]	status		: �������X�e�[�^�X
/// @return 	0: �m�F���Ȃ�<br>
/// 			1: �m�F����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/08
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static short check_for_limit_over(uchar status)
{
	switch ( status ) {
	case 0:	// ������
	case 1:	// ���񊄈�
		return 1;
	case 2:	// ������(���x�����I�[�o�[�̔���͑ΏۊO)
	default:
		return 0;
	}
}
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//[]----------------------------------------------------------------------[]
///	@brief			�I�����C����������
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			1�F���Z�����敪	<br>
///					2�F���������	<br>
///					3�F����			<br>
///					4�F�J�[�h�敪	<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short DiscountForOnline( void )
{
	short	wk = 0;
	short	i, iCount, mai;
	short	cardknd;
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	short	kind = 0;
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(S) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
	short	code = 0;
// MH810100(E) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu
	stDiscount2_t* pDisc = NULL;
	const char USE_CNT_prm_pos[4] = { 0, 5, 3, 1 };

	pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage];

	// m_stDisc�֊����f�[�^�R�s�[
	m_stDisc.DiscParkNo		= pDisc->DiscParkNo;		// ���� ���ԏꇂ(0�`999999)
	m_stDisc.DiscSyu		= pDisc->DiscSyu;			// ���� ���(0�`9999)
	m_stDisc.DiscCardNo		= pDisc->DiscCardNo;		// ���� ���ދ敪(0�`65000)
	m_stDisc.DiscNo			= pDisc->DiscNo;			// ���� �敪(0�`9)
	m_stDisc.DiscCount		= pDisc->DiscCount;			// ���� ����(0�`99)
	m_stDisc.DiscInfo		= pDisc->DiscInfo;			// ���� �������(0�`65000)
	m_stDisc.DiscCorrType	= pDisc->DiscCorrType;		// ���� �Ή����ގ��(0�`65000)
	m_stDisc.DiscStatus		= pDisc->DiscStatus;		// ���� �ð��(0�`9)
	m_stDisc.DiscFlg		= pDisc->DiscFlg;			// ���� ������(0�`9)
	m_stDisc.Discount		= 0;						// �����z(��������) = �����v�Z���ɐώZ�����̂ŃN���A

// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
	g_ulZumiWari = pDisc->UsedDisc;								// �g�p�ς݊����z�𕪊�����悤
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
	g_ulZumiWariOrg = g_ulZumiWari;								// �g�p�ς݊����z�𕪊�����悤
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j

	// ���Z�����敪���u���Z�v�u�Đ��Z�v�ȊO
	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType != 1) &&
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType != 2) ){
		// �����X�e�[�^�X�������ς� or ���񊄈�
		if( m_stDisc.DiscStatus == 2 || m_stDisc.DiscStatus == 1 ){
			return 1;
		}
	}
	// ��Ώ�
	if( m_stDisc.DiscStatus == 6 ){
// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//		return 1;
		return 2;
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
	}

	// ������� == ��ʊ���(����=��������=150)
	if (m_stDisc.DiscSyu == NTNET_SYUBET_TIME) {
		// �W���ʂ�et40/et47�Ōv�Z��������ŵ�ײ��ް��ɂྯĂ���̂ł����ł͉������Ȃ�
		return 2;
	}

	/************************************/
	/* ������ʂɂ�鏈��				*/
 	/*		��S_STO		�X����			*/
	/*		��S_SER		���޽���ݒ�		*/
	/************************************/
	switch (m_stDisc.DiscSyu) {
		case NTNET_SVS_M:			// ���޽��(���z=1)
		case NTNET_SVS_T:			// ���޽��(����=101)
			cardknd = 11;
			break;
		case NTNET_KAK_M:			// �X����(���z=2)
		case NTNET_KAK_T:			// �X����(����=102)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
		case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
			cardknd = 12;
			break;
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
		case NTNET_TKAK_M:			// ���X�܊����i���z�j
		case NTNET_TKAK_T:			// ���X�܊����i���ԁj
			if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {	// ���X�܊�������
				cardknd = 12;
			} else {										// ���X�܊����Ȃ�
				return 2;
			}
			break;
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
		default:
			return 2;
	}

	// 08-0001	�S���޽���g�p���x����	�D�E:�S���޽���g�p���x���� 00=���g�p/1�`99(��)
	mai = (short)prm_get(COM_PRM, S_DIS, 1, 2, 1);

// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
	if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
		// �g�p���޽������ >= �S���޽���g�p���x����
		if (card_use[USE_SVC] >= mai) {
			// ���x��������
			return 3;
		}
// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
	}
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����

	//11=���޽��, 12=�|����, 13=�񐔌��C14=������

// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
	// �`�F�b�N
	if ( CheckDiscount(m_stDisc.DiscSyu, m_stDisc.DiscCardNo, m_stDisc.DiscInfo) ) {
		return 4;
	}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)

	// �T�[�r�X���H
	if( cardknd == 11 ){
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
// 		// �T�[�r�X����͈̓`�F�b�N
// 		if( !rangechk( 1, SVS_MAX, m_stDisc.DiscCardNo ) ){
// 			return 4;
// 		}
// 
// 		// �؊���̎Ԏ�ݒ�Ȃ��A�������A�����ς݂͎Ԏ�؊����󂯕t���Ȃ�
// 		if( CPrmSS[S_SER][3+3*(m_stDisc.DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(m_stDisc.DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 				return 4;										// ��ʐؑւȂ�
// 			}
// // MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// //// MH810100(S) 2020/08/06 #4579�y���񐸎Z(������������)�Ŏg�p������ʐؑւ��A�Đ��Z���ɓK�p����Ȃ�
// ////			if(( ryo_buf.nyukin )||( ryo_buf.waribik )){		// �����ς�? or �����ς�?
// //			if( ryo_buf.nyukin ){		// �����ς�? (�����͂ق��Ń`�F�b�N�ςׂ݂̈����ł̓`�F�b�N���Ȃ�)
// //// MH810100(E) 2020/08/06 #4579�y���񐸎Z(������������)�Ŏg�p������ʐؑւ��A�Đ��Z���ɓK�p����Ȃ�
// //				return 4;										// ���Z�����װ
// //			}
// // MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// 		}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
		if( 1 == (short)prm_get( COM_PRM, S_DIS, 1, 1, 6 ) ){				// ��ʎg�p���x�����ݒ�H
			if( m_stDisc.DiscCardNo <= 3 ){
				data_adr = 106;												// �g�p���x�����ʂ��ް����ڽ�擾
				data_pos = (char)USE_CNT_prm_pos[m_stDisc.DiscCardNo];		// �g�p���x�������ް��ʒu�擾
			}else if( m_stDisc.DiscCardNo <= 6 ){
				data_adr = 107;												// �g�p���x�������ް����ڽ�擾
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-3)];	// �g�p���x�������ް��ʒu�擾
			}else if( m_stDisc.DiscCardNo <= 9 ){
				data_adr = 108;												// �g�p���x�������ް����ڽ�擾
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-6)];	// �g�p���x�������ް��ʒu�擾
			}else if( m_stDisc.DiscCardNo <= 12 ){
				data_adr = 109;												// �g�p���x�������ް����ڽ�擾
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-9)];	// �g�p���x�������ް��ʒu�擾
			}else if( m_stDisc.DiscCardNo <= 15 ){
				data_adr = 110;												// �g�p���x�������ް����ڽ�擾
				data_pos = (char)USE_CNT_prm_pos[(m_stDisc.DiscCardNo-12)];	// �g�p���x�������ް��ʒu�擾
			}
			// �g�p���x�����ݒ�I�[�o�[
			wk = prm_get( COM_PRM, S_SER, data_adr, 2, data_pos );
// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
			if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
				if( card_use2[m_stDisc.DiscCardNo-1] >= wk ){
					return 3;
				}
// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
			}
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
		}
	}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
// 	// �X�����H
// 	else{
// 		// �X���͈̓`�F�b�N
// 		if( !rangechk( 1, MISE_NO_CNT, m_stDisc.DiscCardNo ) ){
// 			return 4;
// 		}
// 
// 		// �؊���̎Ԏ�ݒ�Ȃ��A�������A�����ς݂͎Ԏ�؊����󂯕t���Ȃ�
// 		if( CPrmSS[S_STO][3+3*(m_stDisc.DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(m_stDisc.DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 				return 4;										// ��ʐؑւȂ�
// 			}
// // MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// //// MH810100(S) 2020/08/06 #4579�y���񐸎Z(������������)�Ŏg�p������ʐؑւ��A�Đ��Z���ɓK�p����Ȃ�
// ////			if(( ryo_buf.nyukin )||( ryo_buf.waribik )){		// �����ς�? or �����ς�?
// //			if( ryo_buf.nyukin ){		// �����ς�? (�����͂ق��Ń`�F�b�N�ςׂ݂̈����ł̓`�F�b�N���Ȃ�)
// //// MH810100(E) 2020/08/06 #4579�y���񐸎Z(������������)�Ŏg�p������ʐؑւ��A�Đ��Z���ɓK�p����Ȃ�
// //				return 4;										// ���Z�����װ
// //			}
// // MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// 		}
// 	}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)

	if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
		// 1���Z�̊�����ނ̌����I�[�o�[�H
// MH810100(S) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
//		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, m_stDisc.DiscCardNo, (short)m_stDisc.DiscInfo, 0 ) ){
		// �T�[�r�X��
		if( cardknd == 11 ){
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			kind = (short)m_stDisc.DiscCardNo;	// ����Z�b�g
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			code = (short)m_stDisc.DiscInfo;	// �|�����R�[�h�Z�b�g
		}
		// �|����
		else{
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			kind = (short)m_stDisc.DiscInfo;	// �������/������ʂ��Z�b�g
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			code = (short)m_stDisc.DiscCardNo;	// �X�����Z�b�g
		}
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, m_stDisc.DiscCardNo, code, 0 ) ){
		if( CardSyuCntChk( m_stDisc.DiscNo, cardknd, kind, code, 0 ) ){
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera  2020/04/08 #4128 �Ԕԃ`�P�b�g���X(10��ȏ�̓ǎ�ɓX�������K�p����Ȃ�)
			return 3;		// ���x�����I�[�o�[
		}
	}

	// ���� <- �����ް�_����
	iCount = m_stDisc.DiscCount;

// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
	if ( check_for_limit_over(m_stDisc.DiscStatus) ) {
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
		// �T�[�r�X���g�p���������x���z���Ă���
		if ((iCount + card_use[USE_SVC]) >= mai) {
			// ���� <- (�g�p���x���� - �g�p���޽������)
			iCount = mai - card_use[USE_SVC];
		}

		// �T�[�r�X���H(�T�[�r�X���̏ꍇ�A��ʖ��̖������`�F�b�N)
		if(( cardknd == 11 ) && prm_get( COM_PRM, S_DIS, 1, 1, 6 )){
			// �T�[�r�X���했�̎g�p���x�����𒴂��Ă�H
			if( (iCount + card_use2[m_stDisc.DiscCardNo-1]) >= wk ){
				// ���� <- (�g�p���x���� - �g�p���޽������)
				iCount = wk - card_use2[m_stDisc.DiscCardNo-1];
			}
		}
// MH810102(S) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����
	}
// MH810102(E) R.Endo 2021/04/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5477 �y�C�I���a��{���w�E�����z�����σT�[�r�X���͌��x�����I�[�o�[�ł��K�p����

	// �T�[�r�X���f�[�^�N���A
	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );

// MH810103(S) R.Endo 2021/06/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5699 �g�����ԏꇂ��QR������(�X����)����{���ԏꈵ���ɂȂ�
// 	// �����敪 == 1, 2, 3
// 	if (1 <= m_stDisc.DiscNo && m_stDisc.DiscNo <= 3){
// 		// ���޽���ް�_�g��1�`3���(APS�Ȃ̂Ŋg��2�`3����M���邱�Ƃ͂Ȃ�)
// 		vl_svs.pkno = m_stDisc.DiscNo;
// 	}
// 	// �����敪 == 0
// 		else{
// 		// ���޽���ް�_�͈͊O(��Q�����܂�)�͊�{���
// 		vl_svs.pkno = 0;
// 	}
	// ���ԏꇂ�`�F�b�N
	if( 0L == m_stDisc.DiscParkNo ){
		return 4;	// ���ԏꇂ�G���[
	}

	// ���ԏꇂ���
	if( (prm_get(COM_PRM, S_SYS, 71, 1, 6) == 1) &&				// ��{�T�[�r�X���g�p��
		(CPrmSS[S_SYS][1] == m_stDisc.DiscParkNo) ) {			// ��{���ԏꇂ�ƈ�v
		vl_svs.pkno = KIHON_PKNO;	// ��{
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 1) == 1) &&		// �g��1�T�[�r�X���g�p��
		       (CPrmSS[S_SYS][2] == m_stDisc.DiscParkNo) ) {	// �g��1���ԏꇂ�ƈ�v
		vl_svs.pkno = KAKUCHOU_1;	// �g��1
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 2) == 1) &&		// �g��2�T�[�r�X���g�p��
		       (CPrmSS[S_SYS][3] == m_stDisc.DiscParkNo) ) {	// �g��2���ԏꇂ�ƈ�v
		vl_svs.pkno = KAKUCHOU_2;	// �g��2
	} else if( (prm_get(COM_PRM, S_SYS, 71, 1, 3) == 1) &&		// �g��3�T�[�r�X���g�p��
		       (CPrmSS[S_SYS][4] == m_stDisc.DiscParkNo) ) {	// �g��3���ԏꇂ�ƈ�v
		vl_svs.pkno = KAKUCHOU_3;	// �g��3
	} else{
		return 4;	// ���ԏꇂ�G���[
	}
// MH810103(E) R.Endo 2021/06/08 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5699 �g�����ԏꇂ��QR������(�X����)����{���ԏꈵ���ɂȂ�

	// �T�[�r�X���H
	if( m_stDisc.DiscSyu == NTNET_SVS_M ||
		m_stDisc.DiscSyu == NTNET_SVS_T ){

		// ���޽���ް�_���޽���� <- �������ދ敪
		vl_svs.sno  = (char)m_stDisc.DiscCardNo;

		// ���޽���ް�_�XNO.( 0�`100) <- �������
		vl_svs.mno  = (short)m_stDisc.DiscInfo;

		if( m_stDisc.DiscStatus == 2 ){	// �����ς�
			vl_svs.mno |= 0x8000;
		}
	}
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
	// ���X�܊����H
	else if ( (m_stDisc.DiscSyu == NTNET_TKAK_M) ||
		      (m_stDisc.DiscSyu == NTNET_TKAK_T) ) {
		// �T�[�r�X���f�[�^ �T�[�r�X���� <- �������(�������)
		// ���X�܊����ł͊������(1�`15)���i�[����B
		// �T�[�r�X�����26�A���X�܊����̊�����ʂ�15�A�X�����̊�����ނ�101���ő�ׁ̈A
		// 29�܂ł̓T�[�r�X���A30�`49�͑��X�܊����A50�ȍ~�͓X�����Ƃ��ċ�ʂ���B
		vl_svs.sno = 0;
		if ( (m_stDisc.DiscInfo > 0) && (m_stDisc.DiscInfo < 225) ) {
			vl_svs.sno = (char)(30 + m_stDisc.DiscInfo);
		}

		// �T�[�r�X���f�[�^ �X�� <- �J�[�h�敪(�X��)
		vl_svs.mno = (short)m_stDisc.DiscCardNo;

		if ( m_stDisc.DiscStatus == 2 ) {	// �����ς�
			vl_svs.mno |= 0x8000;
		}
	}
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
	// �X�����H
	else{

		// ���޽���ް�_���޽���� <- 0
		vl_svs.sno  = 0;
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
		// vl_svs.sno�́A�X��������0�ŁA�T�[�r�X���̏ꍇ�A�J�[�h�敪�́A1�`26�ɂȂ�B
		// ������ނ́A1�`101�Ȃ̂ŁA�{50���āAvl_svs.sno�Ɋi�[����B
		if(m_stDisc.DiscInfo > 0 && m_stDisc.DiscInfo < 205){
			vl_svs.sno = 50 + m_stDisc.DiscInfo;
		}
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j

		// ���޽���ް�_�XNO.( 0�`100) <- �������ދ敪
		vl_svs.mno  = (short)m_stDisc.DiscCardNo;
		if( m_stDisc.DiscStatus == 2 ){	// �����ς�
			vl_svs.mno |= 0x8000;
		}
	}

	// �|���茔�������ɖ��Ή��Ȃ̂Ŗ�����ٰ�߂���
	for (i = 0; i < iCount; i++) {

		// �g�p���޽��, �|����, ���������� + 1
		card_use[USE_SVC] += 1;
		if( m_stDisc.DiscStatus < 2 ){
			card_use[USE_N_SVC] += 1;
// MH810100(S) K.Onodera  2020/04/09 #4130 �Ԕԃ`�P�b�g���X(�T�[�r�X�����했���x�������K�p����Ȃ�)
// MH810102(S) R.Endo 2021/04/05 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5472 �y�C�I���a��{���w�E�����z�Đ��Z���̌��했���x�����̔��肪���Ғʂ�łȂ�
//			// �T�[�r�X���H
//			if( cardknd == 11 ){
//				card_use2[m_stDisc.DiscCardNo-1] += 1;
//			}
// MH810102(E) R.Endo 2021/04/05 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5472 �y�C�I���a��{���w�E�����z�Đ��Z���̌��했���x�����̔��肪���Ғʂ�łȂ�
// MH810100(E) K.Onodera  2020/04/09 #4130 �Ԕԃ`�P�b�g���X(�T�[�r�X�����했���x�������K�p����Ȃ�)
		}
// MH810102(S) R.Endo 2021/04/05 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5472 �y�C�I���a��{���w�E�����z�Đ��Z���̌��했���x�����̔��肪���Ғʂ�łȂ�
		// �T�[�r�X���H
		if ( cardknd == 11 ) {
			switch ( m_stDisc.DiscStatus ) {
			case 0:	// ������
			case 1:	// ���񊄈�
			case 2:	// ������(�ȑO�̐��Z�Ŏg�p���ꂽ�T�[�r�X�����J�E���g����)
				card_use2[m_stDisc.DiscCardNo-1] += 1;
				break;
			default:
				break;
			}
		}
// MH810102(E) R.Endo 2021/04/05 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5472 �y�C�I���a��{���w�E�����z�Đ��Z���̌��했���x�����̔��肪���Ғʂ�łȂ�

		// ����VL�����ް� <- ���޽��
		vl_now = V_SAK;

		// -------------------------- //
		// �T�[�r�X���Ƃ��ČĂяo��
		// -------------------------- //
		ryo_cal( 3, OPECTL.Pr_LokNo );		// Pr_LokNo = ���������p���Ԉʒu�ԍ�(1�`324))

		if( OPECTL.op_faz == 0 ){
			OPECTL.op_faz = 1;			// ������
		}

		ac_flg.cycl_fg = 10;			// ����

		in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );
	}

	// �����ް�_���� <- ����
	m_stDisc.DiscCount = (uchar)iCount;

	// �������H
	if( m_stDisc.DiscStatus == 0 ){
		m_stDisc.DiscStatus = 1;	// ���񊄈���(���Z�����Ŋ����ςƂ���)
	}

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�I�����C����������
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			1�F���Z�����敪	<br>
///					2�F���������	<br>
///					3�F����			<br>
///					4�F�J�[�h�敪	<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	OnlineDiscount( void )
{
	int i = 0, j = 0;
// MH810100(S) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//	int check_index = -1;
// MH810100(E) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
	short ret = 0;
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
	BOOL bFound = FALSE;
	unsigned long	waribikTmp = 0;
	unsigned long	waribikTmp2 = 0;
	unsigned long	zenkaiwariTmp = 0;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	unsigned short wk_dat;
	unsigned short wk_dat2;
	DISCOUNT_DATA	disTemp;
	stDiscount_t	dis2Temp;
	int iTemp = 0;
//	BOOL	bRepay = FALSE;		// �Đ��Z
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j

	// �L�������� �����d�����p����	0=�����Ȃ��1=�����d���Ȃ��2=�����d������3=�ް��s���99=���̑��ُ�
	// 0=�����Ȃ��1=�����d���Ȃ��̎��̂݊����������s��
	if( (lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup != 0) &&
		(lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup != 1) ){
// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//		return (1);
		return (101);
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
	}
// MH810100(S) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
	if(IsDupSyubetuwari() == TRUE){
		// ���ԃG���[
// MH810100(S) 2020/08/31 #4771 �y���؉ێw�E�����z����������̎�ʐؑւŕ\�������G���[���b�Z�[�W���s�K��(No60)
//		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(4 - 1) );	// �|�b�v�A�b�v�\���v��
		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(5 - 1) );	// �|�b�v�A�b�v�\���v��
// MH810100(E) 2020/08/31 #4771 �y���؉ێw�E�����z����������̎�ʐؑւŕ\�������G���[���b�Z�[�W���s�K��(No60)
		return 4;
	}
// MH810100(E) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// MH810100(S) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	if(IsWaribikiLimitOver() == TRUE){
// 		// ���ԃG���[
// 		lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(6 - 1) );	// �|�b�v�A�b�v�\���v��
// 		return 4;
// 	}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)

// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	wk_dat = (ushort)(1+6*(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = ��ʎ��Ԋ����l		*/
	if( wk_dat2){
		// ��ʎ��Ԋ�������������
		memset( &dis2Temp, 0, sizeof(stDiscount_t) );
		// �N���A�O�ɕێ����Ă���
		memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
		// RT_Pay�p���ێ�
		dis2Temp.DiscParkNo = disTemp.ParkingNo;
		dis2Temp.DiscSyu = disTemp.DiscSyu;
		dis2Temp.DiscCardNo = disTemp.DiscNo;
		dis2Temp.DiscCount = disTemp.DiscCount;
		dis2Temp.Discount = disTemp.Discount;
		dis2Temp.DiscStatus = 1;		// ����
	}
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#if 1	// ��������F�N���E�h�����v�Z��discount_init(cal_cloud/cal_cloud.c)�ɑ����B
		// ���C������ۂ�discount_init�����m�F���邱�ƁB
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	// RT_Pay�APayData�̊���������U�N���A
	memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// ���Z�f�[�^�������쐬�G���A�N���A
	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );
	NTNET_Data152_DiscDataClear();
	NTNET_Data152_DetailDataClear();

	// ���Ԍ��A����ȊO�̎g�p�������N���A
	for( i=0; i<USE_MAX; i++ ){
		if( i != USE_TIK && i != USE_PAS ){
			card_use[i] = 0;
		}
	}
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	i = 0;	// ���pQR�o�b�N�A�b�v�C���f�b�N�X�Ƃ��ė��p���邽�߂ɃN���A
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	memset( card_use2, 0, sizeof( card_use2 ) );
	CardUseSyu = 0;
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#endif	// �����܂ŁF�N���E�h�����v�Z��discount_init(cal_cloud/cal_cloud.c)�ɑ����B
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	// ���Ɏ�����߂�
	memcpy( &car_in, &car_in_f, sizeof( struct CAR_TIM ) );
	// �߂�����ɁA��ʎ��Ԃ����Z����
	IncCatintime(&car_in);	

	// ryo_cal�Ŏg�p���銄�������N���A
	ClearRyoCalDiscInfo();

	// ��ʐ؂�ւ��̃`�F�b�N�y�ѕ��ёւ�
	bFound = DiscountSortCheck();
	m_nDiscPage = 0;
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	if( wk_dat2){
		// ��ʎ��Ԋ�������������擪�ɓ���Ă����i���Ԍ������͎��{�ςׁ݂̈jcal�ōēx�Z�b�g����邪�A�����Ȃ��Ō����̏ꍇ�ڂ�Ȃ��Ȃ�̂ŁA�����ŃZ�b�g�B
		if(disTemp.ParkingNo != 0){
			memcpy( &PayData.DiscountData[0], &disTemp, sizeof( DISCOUNT_DATA ) );
		}
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &dis2Temp, sizeof( stDiscount_t ) );

	}
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	if( bFound){
		// ��ʐ֑ؑO���o���Ă���
		waribikTmp2 = ryo_buf.waribik;
		if(waribikTmp2 >= ryo_buf.zenkai){
			// waribik��zenkai��������Ă�������Ă���
			waribikTmp2 = waribikTmp2 - ryo_buf.zenkai;
		}
		// �O�񊄂������ł��ڂ��Ă���
		zenkaiwariTmp = ryo_buf.zenkaiWari;
		// ��ʐؑւ��݂������̂ŁA�����ł܂����{���Ă���

		// -------------------- //
		// ����������
		// -------------------- //
		ret = DiscountForOnline();
		// ����NG�H
		if( ret ){
			lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// �|�b�v�A�b�v�\���v��
			return 4;
		}
// MH810100(S) 2020/09/04  ��ʎ��Ԋ���
//		// QR���p���̊����f�[�^��r�p�Ƀo�b�N�A�b�v
//		memcpy( &DiscountBackUpArea[0], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
//		// ���A���^�C�����Z�f�[�^�p�f�[�^�֐��Z���ʂ���������
//		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], &m_stDisc, sizeof(m_stDisc) );
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 		// QR���p���̊����f�[�^��r�p�Ƀo�b�N�A�b�v
// 		memcpy( &DiscountBackUpArea[i++], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
		// ���A���^�C�����Z�f�[�^�p�f�[�^�֐��Z���ʂ���������
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &m_stDisc, sizeof(m_stDisc) );
		m_nDiscPage++;		
// MH810100(E) 2020/09/04  ��ʎ��Ԋ���

		// ��ʐؑւɊ��������Ă�����Atmp�ɂ����
		waribikTmp = ryo_buf.waribik;
		ryo_buf.waribik = waribikTmp2;
		// �߂�
		ryo_buf.zenkaiWari = zenkaiwariTmp;
// MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
		g_checkKirikae = 1;								// ��ʐؑփ`�F�b�N�ς݃t���O
// MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// MH810100(S) 2020/09/08 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
		if( wk_dat2){
			wk_dat = (ushort)(1+6*(ryo_buf.syubet));
			wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = ��ʎ��Ԋ����l		*/
			// ��ʊ������肩���ʊ�������
			if( wk_dat2){
				// ��ʐؑւœ���ւ���Ă��邩������Ȃ��̂ŁA�㏑������
				for(iTemp = 0 ; iTemp < ONL_DISC_MAX; iTemp++ ){
					if(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu == NTNET_SYUBET|| 
					RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu == NTNET_SYUBET_TIME){
						memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscParkNo = disTemp.ParkingNo;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscSyu = disTemp.DiscSyu;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCardNo = disTemp.DiscNo;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCount = disTemp.DiscCount;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].Discount = disTemp.Discount;
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscTime = 0;
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscNo = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscInfo = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscCorrType = 0;
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscStatus = 1;		// ����
						RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[iTemp].DiscFlg = 0;
						break;
					}
				}
			}else{
				// ��ʊ������肩���ʊ����Ȃ���
				// �P�܂��ɂ��炷
				memmove(&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0],&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[1],sizeof(stDiscount_t)*(ONL_MAX_DISC_NUM-1 ));
				// ��Ԍ����N���A
				memset(&RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[ONL_MAX_DISC_NUM-1],0,sizeof(stDiscount_t));
				j--;

			}
		}else{
			wk_dat = (ushort)(1+6*(ryo_buf.syubet));
			wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = ��ʎ��Ԋ����l		*/
			// ��ʊ����Ȃ������ʊ�������
			if( wk_dat2){
				memset( &dis2Temp, 0, sizeof(stDiscount_t) );
				memcpy(&disTemp,&PayData.DiscountData[0],sizeof( DISCOUNT_DATA ));
				// RT_Pay�p���ێ�
				dis2Temp.DiscParkNo = disTemp.ParkingNo;
				dis2Temp.DiscSyu = disTemp.DiscSyu;
				dis2Temp.DiscCardNo = disTemp.DiscNo;
				dis2Temp.DiscCount = disTemp.DiscCount;
				dis2Temp.Discount = disTemp.Discount;
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
				dis2Temp.DiscTime = 0;
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
				dis2Temp.DiscStatus = 1;		// ����
				// ���ɂP���炷
				memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[1], &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], sizeof( stDiscount_t ) );
				// �����ɓ����
				memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[0], &dis2Temp, sizeof( stDiscount_t ) );
				j++;
			}
		}
// MH810100(E) 2020/09/08 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	}
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

	// �����͓��ɏ���M���ɖ���ŏ������蒼��
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
////	ryo_buf.zankin += ryo_buf.waribik;
////	ryo_buf.dsp_ryo += ryo_buf.waribik;
//	ryo_buf.zankin += ryo_buf.waribik - ryo_buf.zenkai;
//	ryo_buf.dsp_ryo += ryo_buf.waribik - ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
//	ryo_buf.zankin = ryo_buf.zankin - ryo_buf.zenkaiWari;
//	ryo_buf.dsp_ryo = ryo_buf.dsp_ryo - ryo_buf.zenkaiWari;
//// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
// MH810101(S) R.Endo 2021/01/29 �y�A���]���w�E�����z�L�������؂����X�V��̍Đ��Z�Ŏc�z���s���l�ƂȂ�(No.02-0001)
//	ryo_buf.zankin = ryo_buf.zankin + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
//	ryo_buf.dsp_ryo = ryo_buf.dsp_ryo + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	if ( (ryo_buf.zankin + ryo_buf.waribik) > (ryo_buf.zenkai + ryo_buf.zenkaiWari) ) {
		ryo_buf.zankin = ryo_buf.zankin + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	} else {
		ryo_buf.zankin = 0;
	}
	if ( (ryo_buf.dsp_ryo + ryo_buf.waribik) > (ryo_buf.zenkai + ryo_buf.zenkaiWari) ) {
		ryo_buf.dsp_ryo = ryo_buf.dsp_ryo + ryo_buf.waribik - ryo_buf.zenkai - ryo_buf.zenkaiWari;
	} else {
		ryo_buf.dsp_ryo = 0;
	}
// MH810101(E) R.Endo 2021/01/29 �y�A���]���w�E�����z�L�������؂����X�V��̍Đ��Z�Ŏc�z���s���l�ƂȂ�(No.02-0001)
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

	// �����N���A
	ryo_buf.dis_tim = ryo_buf.dis_fee = ryo_buf.dis_per = 0L;
	ryo_buf.waribik = ryo_buf.fee_amount = ryo_buf.tim_amount = 0L;
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//	ryo_buf.waribik = ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
//	ryo_buf.waribik = ryo_buf.waribik + ryo_buf.zenkaiWari;
//// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
	ryo_buf.waribik = ryo_buf.zenkai + ryo_buf.zenkaiWari + waribikTmp;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

	ryo_buf_n.require = 0L;
	ryo_buf_n.dis     = 0L;

// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//	// RT_Pay�APayData�̊���������U�N���A
//	memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// ���Z�f�[�^�������쐬�G���A�N���A
//// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:151)�Ή�
////	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount2_t)*ONL_MAX_DISC_NUM );
//	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );
//// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:151)�Ή�
//	NTNET_Data152_DiscDataClear();
//	NTNET_Data152_DetailDataClear();
//// MH810100(S) 2020/07/29 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//	// ��ʐ؂�ւ��̃`�F�b�N�y�ѕ��ёւ�
//	DiscountSortCheck();
//// MH810100(E) 2020/07/29 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//
//	// ���Ԍ��A����ȊO�̎g�p�������N���A
//	for( i=0; i<USE_MAX; i++ ){
//		if( i != USE_TIK && i != USE_PAS ){
//			card_use[i] = 0;
//		}
//	}
//	memset( card_use2, 0, sizeof( card_use2 ) );
//	CardUseSyu = 0;
//
//	// ���Ɏ�����߂�
//	memcpy( &car_in, &car_in_f, sizeof( struct CAR_TIM ) );
// MH810100(S) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�
//	// �߂�����ɁA��ʎ��Ԃ����Z����
//	IncCatintime(&car_in);	
// MH810100(E) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

// MH810100(S) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//	for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
//		if( !DiscountBackUpArea[i].DiscParkNo && lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscParkNo ){
//			// �C���f�b�N�X�� i �Ƃ���
//			check_index = i;
//			break;
//		}
//	}
// MH810100(E) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)


// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//	// ryo_cal�Ŏg�p���銄�������N���A
//	ClearRyoCalDiscInfo();
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
//// MH810100(S) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)
//	i = 0;	// ���pQR�o�b�N�A�b�v�C���f�b�N�X�Ƃ��ė��p���邽�߂ɃN���A
// MH810100(E) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	// �ő劄���f�[�^�������[�v
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//	for( m_nDiscPage=0; m_nDiscPage < ONL_DISC_MAX; m_nDiscPage++ ){
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
//	if( bFound){
//		i = 1;
//		j = 1;
//		m_nDiscPage = 1;
//	}
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
	for( ; m_nDiscPage < ONL_DISC_MAX; m_nDiscPage++ ){
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
		if(( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscParkNo == 0L ) &&
			( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscSyu == 0 )){
			break;
		}
		taskchg(IDLETSKNO);
// MH810100(S) 2020/06/11 �w�肵�Ȃ���ʂ������珈�����Ȃ�
		switch (lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage].DiscSyu) {
			case NTNET_SVS_M:			// ���޽��(���z=1)
			case NTNET_SVS_T:			// ���޽��(����=101)
			case NTNET_KAK_M:			// �X����(���z=2)
			case NTNET_KAK_T:			// �X����(����=102)
			case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
			case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
			case NTNET_TKAK_M:			// ���X�܊����i���z�j
			case NTNET_TKAK_T:			// ���X�܊����i���ԁj
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
				break;
			default:
				continue;
				break;
		}
// MH810100(E) 2020/06/11 �w�肵�Ȃ���ʂ������珈�����Ȃ�
		// -------------------- //
		// ����������
		// -------------------- //
		ret = DiscountForOnline();
		// ����NG�H
		if( ret ){
			// �O��QR�Ȃ�����A�ǉ��ƂȂ銄���H
// MH810100(S) 2020/09/02 #4803 [08-0001�F�S�T�[�r�X���g�p���x����]�Őݒ肳��Ă��閇���𒴂����ۂɕ\�������POPUP���A�\������Ȃ�
//// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
//// MH810100(S) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
////			if( m_nDiscPage == check_index ){
//			if(IsBackupAreaFound(m_nDiscPage ) == FALSE){
//// MH810100(S) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
//				lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// �|�b�v�A�b�v�\���v��
//// MH810100(E) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
//				// ���ʂ�NG���������AQR�o�^�f�[�^�̓o�^�����Ȃ�
//				break;
//			}else{
//				continue;
//			}
			lcdbm_notice_dsp( POP_UNABLE_DISCOUNT, (uchar)(ret - 1) );	// �|�b�v�A�b�v�\���v��
			break;
// MH810100(E) 2020/09/02 #4803 [08-0001�F�S�T�[�r�X���g�p���x����]�Őݒ肳��Ă��閇���𒴂����ۂɕ\�������POPUP���A�\������Ȃ�
		}
		// �O�񂩂�̍�����NG�łȂ���ΐ���Ƃ��Ă���
		ret = 0;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// // MH810100(S) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)
// 		// QR���p���̊����f�[�^��r�p�Ƀo�b�N�A�b�v
// 		memcpy( &DiscountBackUpArea[i++], &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage], sizeof(stDiscount2_t) );
// // MH810100(E) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

		// ���A���^�C�����Z�f�[�^�p�f�[�^�֐��Z���ʂ���������
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j++], &m_stDisc, sizeof(m_stDisc) );
	}

	return (ret);
}

// MH810100(S) K.Onodera  2020/02/12 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//[]----------------------------------------------------------------------[]
///	@brief			�����ϊz��o������
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			������
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
// ulong	Ope_GetLastDisc( void )
ulong	Ope_GetLastDisc( long ldiscount )
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
{
	stDiscount2_t* pDisc;
	ulong last_disc;

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH810103(s) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
//	if (f_sousai != 0) {
//		return 0;
//	}
//// MH810103(e) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

	pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[m_nDiscPage];

// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
//// MH810100(S) 2020/06/05 #4199 �y�A���]���w�E�����zNT-NET���Z�f�[�^�ł̊����̍����i�[
////// MH810100(S) 2020/05/27 �Ԕԃ`�P�b�g���X(#4196)
//////	if( pDisc->DiscSyu >= NTNET_SVS_T ){	// ���Ԋ����H
//////		last_disc = pDisc->DiscountT;	// ���� ���Ԃ����o��(0�`999999�~)
//////	}else{
//////		last_disc = pDisc->DiscountM;	// ���� ���z�����o��(0�`999999�~)
//////	}
////	last_disc = pDisc->Discount;				// ���z/����(0�`999999)
////// MH810100(E) 2020/05/27 �Ԕԃ`�P�b�g���X(#4196)
//	last_disc = pDisc->UsedDisc;				// ���z/����(0�`999999)
//// MH810100(E) 2020/06/05 #4199 �y�A���]���w�E�����zNT-NET���Z�f�[�^�ł̊����̍����i�[
	if( pDisc->DiscCount == 1){
		last_disc = pDisc->UsedDisc;				// ���z/����(0�`999999)
	}else{
		// �������̏ꍇ�́A������
		if( g_ulZumiWari > ldiscount){
			last_disc = ldiscount;
			g_ulZumiWari -= ldiscount;
		}else{
			last_disc = g_ulZumiWari;
			g_ulZumiWari = 0;
		}

	}
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�

	return last_disc;
}
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
ulong	Ope_GetLastDiscOrg()
{	
	return g_ulZumiWariOrg;							// �g�p�ς݊����z�𕪊�����܂�
}
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j

// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
BOOL Ope_SyubetuWariCheck(DISCOUNT_DATA* pDisc)
{
	BOOL bRet = FALSE;
	BOOL bRepay = FALSE;
	unsigned short wk_dat;
	unsigned short wk_dat2;
	int i;

// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
		// �N���E�h�����v�Z�ł̓`�F�b�N���Ȃ�
		return FALSE;
	}
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

	wk_dat = (ushort)(1+6*(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = ��ʎ��Ԋ����l		*/

	// ���Z�����敪���u���Z�v�u�Đ��Z�v�ȊO
	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=���Z
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ){		// 2=�Đ��Z
		// ���Z������������H
		if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
			(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ){
			bRepay = TRUE;
		}
	}
	if(wk_dat2 && bRepay){	
		for( i=0; i<ONL_MAX_DISC_NUM; i++ ){
			if( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscSyu == NTNET_SYUBET_TIME ||
				lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].DiscSyu == NTNET_SYUBET  ){
				if( pDisc->Discount > lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].UsedDisc){
					// 1��ڂɒ��r���[�Ȑ��g�����ꍇ
					pDisc->DiscCount = 0;		// 0��
					pDisc->Discount = pDisc->Discount - lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i].UsedDisc;
					bRet = FALSE;
				}else{
					// 2��ڂ͍ڂ��Ȃ�
					memset(pDisc,0,sizeof( DISCOUNT_DATA ));
					bRet = TRUE;
				}
				break;
			}

		}
	}
	return bRet;
}
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j

// MH810100(E) K.Onodera  2020/02/12 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//[]----------------------------------------------------------------------[]
///	@brief			���A���^�C�����Z�f�[�^�̃Z���^�[�ǔԂ��擾
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			�Z���^�[�ǔ�
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ulong	lcdbm_RTPay_GetCenterSeqNo( REAL_SEQNO_TYPE type )
{
	if( type < REAL_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		return REAL_SeqNo[type];
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			���A���^�C�����Z�f�[�^�̃Z���^�[�ǔԂ��X�V
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_RTPay_UpdateCenterSeqNo( REAL_SEQNO_TYPE type )
{
	if( type < REAL_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		++REAL_SeqNo[type];
		if(REAL_SeqNo[type] == 0) {
			REAL_SeqNo[type] = 1;				// �Z���^�[�ǔԂ�1�`0xffffffff�܂�
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����Z�f�[�^�̃Z���^�[�ǔԂ��X�V����Log�ɏ�������
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// call�� = LOG_PAYMENT, LOG_PAYSTOP��Log_regist�����ݸނ�call�����
void RTPay_LogRegist_AddOiban( void )
{
	REAL_SEQNO_TYPE type = REAL_SEQNO_PAY;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
	if( RTPay_Data.shubetsu == 1 ){ //������Z
		type =REAL_SEQNO_PAY_AFTER;					// ���Z�f�[�^(������Z)
	}
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�

	// ���A���^�C�����Z�f�[�^�̃Z���^�[�ǔԎ擾
	RTPay_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
	RTPay_Data.CenterOibanFusei = 0;
// MH810100(S) 2020/07/27 #4557�y���؉ێw�E�����z ���A���^�C�����Z�f�[�^�̏����������s��(�`�~�F39)
	// �����N���������b
	RTPay_Data.crd_info.dtTimeYtoSec.shYear = CLK_REC.year;
	RTPay_Data.crd_info.dtTimeYtoSec.byMonth = CLK_REC.mont;
	RTPay_Data.crd_info.dtTimeYtoSec.byDay = CLK_REC.date;
	RTPay_Data.crd_info.dtTimeYtoSec.byHours = CLK_REC.hour;
	RTPay_Data.crd_info.dtTimeYtoSec.byMinute = CLK_REC.minu;
	RTPay_Data.crd_info.dtTimeYtoSec.bySecond = CLK_REC.seco;
// MH810100(E) 2020/07/27 #4557�y���؉ێw�E�����z ���A���^�C�����Z�f�[�^�̏����������s��(�`�~�F39)

	// �Z���^�[�ǔ� == 0�H
	if( RTPay_Data.CenterOiban == 0 ){
		// �ǔԃJ�E���g�A�b�v���čĎ擾
		lcdbm_RTPay_UpdateCenterSeqNo(type);
		RTPay_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
		RTPay_Data.CenterOibanFusei = 1;
	}

	// �Z���^�[�ǔԂ��X�V����
	lcdbm_RTPay_UpdateCenterSeqNo(type);

	// ���Z�ް�/���Z���~����log�����ݏ���
	Log_regist( LOG_RTPAY );
}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//[]----------------------------------------------------------------------[]
///	@brief		RT�̎��؃f�[�^�̃Z���^�[�ǔԂ��X�V����Log�ɏ�������
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void RTReceipt_LogRegist_AddOiban( void )
{
	REAL_SEQNO_TYPE type = REAL_SEQNO_RECEIPT;

	// RT�̎��؃f�[�^�̃Z���^�[�ǔԎ擾
	RTReceipt_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
	RTReceipt_Data.CenterOibanFusei = 0;

	// �����N���������b
	RTReceipt_Data.receipt_info.dtTimeYtoSec.shYear = CLK_REC.year;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byMonth = CLK_REC.mont;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byDay = CLK_REC.date;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byHours = CLK_REC.hour;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.byMinute = CLK_REC.minu;
	RTReceipt_Data.receipt_info.dtTimeYtoSec.bySecond = CLK_REC.seco;

	// �Z���^�[�ǔ� == 0�H
	if ( RTReceipt_Data.CenterOiban == 0 ) {
		// �ǔԃJ�E���g�A�b�v���čĎ擾
		lcdbm_RTPay_UpdateCenterSeqNo(type);
		RTReceipt_Data.CenterOiban = lcdbm_RTPay_GetCenterSeqNo(type);
		RTReceipt_Data.CenterOibanFusei = 1;
	}

	// �Z���^�[�ǔԂ��X�V����
	lcdbm_RTPay_UpdateCenterSeqNo(type);

	// �̎��؃f�[�^��log�����ݏ���
	Log_regist(LOG_RTRECEIPT);
}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

//[]----------------------------------------------------------------------[]
///	@brief			�Z���^�[�ǔԂ��擾����iDC-NET�ʐM�j
//[]----------------------------------------------------------------------[]
///	@param			type	: �ǔԎ��
//[]----------------------------------------------------------------------[]
///	@return			��ʂɑΉ�����Z���^�[�ǔ�<br>
///					0		: �ǔԎ�ʕs��
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ulong DC_GetCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		return DC_SeqNo_wk[type];
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�Z���^�[�ǔԂ��X�V����iDC-NET�ʐM�j
//[]----------------------------------------------------------------------[]
///	@param			type	: �ǔԎ��
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_UpdateCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		++DC_SeqNo_wk[type];
		if(DC_SeqNo_wk[type] == 0) {
			DC_SeqNo_wk[type] = 1;				// �Z���^�[�ǔԂ�1�`0xffffffff�܂�
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�Z���^�[�ǔԂ�Push����iDC-NET�ʐM�j
//[]----------------------------------------------------------------------[]
///	@param			type	: �ǔԎ��
//[]----------------------------------------------------------------------[]
///	@return			��ʂɑΉ�����Z���^�[�ǔ�<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_PushCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		DC_SeqNo_wk[type] =  DC_SeqNo[type];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�Z���^�[�ǔԂ�Pop����iDC-NET�ʐM�j
//[]----------------------------------------------------------------------[]
///	@param			type	: �ǔԎ��
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_PopCenterSeqNo( DC_SEQNO_TYPE type )
{
	if( type < DC_SEQNO_TYPE_COUNT ){			// �ǔԎ�ʂ��͈͓���
		DC_SeqNo[type] =  DC_SeqNo_wk[type];
	}
}

// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			���������`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		pno		: ���ԏꇂ
///	@param[in/out]	*p_kind	: ������ʂւ̃|�C���^
///	@param[in]		mno		: �X��
///	@param[out]		*p_info	: �����K�p��(�������1)�ւ̃|�C���^
//[]----------------------------------------------------------------------[]
///	@return			ret		: �������
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
///	@mnote			�Y���X���̊��������������ł���ꍇ�ANTNET_SHOP_DISC_AMT
///					(��������(����))/NTNET_SHOP_DISC_TIME(��������(����))��
///					��߂��A���������łȂ��ꍇ�͓��͊�����ʂ����̂܂ܕԂ�
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void CheckShoppingDisc( ulong pno, ushort *p_kind, ushort mno, ushort *p_info )
{
	stDiscount2_t	*pDisc;
	ushort			i;
	ushort			wk = 0;

	for( i = 0; i < ONL_MAX_DISC_NUM; i++) {
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];

		if(( pDisc->DiscSyu == NTNET_SHOP_DISC_AMT ) ||	// ������� = ��������(����)�H
		   ( pDisc->DiscSyu == NTNET_SHOP_DISC_TIME )){	// ������� = ��������(����)�H
			if(( pDisc->DiscParkNo == pno ) &&	// ���ԏꇂ�ƓX������v�H
			   ( pDisc->DiscCardNo == mno )){
				wk = (ushort)CPrmSS[S_STO][1+3*(mno-1)];
				if( wk == 1 ){
					*p_kind = NTNET_SHOP_DISC_TIME;
				}else{
					*p_kind = NTNET_SHOP_DISC_AMT;
				}
				*p_info = pDisc->DiscInfo;	// �����K�p���Z�b�g
				return;
			}
		}
	}

	return;
}
// MH810100(E) K.Onodera  2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)

// MH810100(S) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
void	azukari_popup(uchar cancel)
{
	ePOP_DISP_KIND	DispCode;	// �|�b�v�A�b�v�\�����
	uchar	DispStatus;			// �\���X�e�[�^�X
	// �߯�߱��ߕ\���v��(���Z���ɔ�������\���v��) & �Ή����鉹���ē��𗬂�
	// cancel	0 = ���Z���̗a��ؔ��s		1 = ���Z���~���̗a��ؔ��s
	// ��OPECTL.RECI_SW		// 1 = �̎������ݎg�p	0 = �̎������ݖ��g�p
	// ��OPECTL.Fin_mod		// 0 = ���z		1 = �ނ薳��		2 = �ނ�L��	3 = ү��װ(�ނ�L��)
	// 1:�a��ؔ��s(���Z������)	2:�a��ؔ��s(�����߂���)
	// ���Z���̗a��ؔ��s
	if (cancel == 0) {
		DispCode = POP_AZUKARI_PAY;
	}
	// ���Z���~���̗a��ؔ��s
	else {
		DispCode = POP_AZUKARI_RET;
	}
	// 0 = ���z, 1 = �ނ薳��, 2 = �ނ�L��, 3 = ү��װ(�ނ�L��)
	switch (OPECTL.Fin_mod) {
		case 0:
		case 1:
		default:
			// 1 = �̎������ݎg�p	0 = �̎������ݖ��g�p
			if (OPECTL.RECI_SW == 0) {
				DispStatus = 0;		// �ޑK�Ȃ�
			}
			else {
				DispStatus = 2;		// �ޑK�Ȃ�(�̎���)
			}
			break;
		case 2:
		case 3:
			if (OPECTL.RECI_SW == 0) {
				DispStatus = 1;		// �ޑK����
			}
			else {
				DispStatus = 3;		// �ޑK����(�̎���)
			}
			break;
	}
	// �|�b�v�A�b�v�\���v��(�a��ؔ��s)
	lcdbm_notice_dsp( DispCode, DispStatus );
}
// MH810100(E) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)

// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
//// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//// ��ʊ������݂��āA��������擪�Ɏ����Ă���i2�ȏ゠�邱�Ƃ͑z�肵�Ă��Ȃ��j
//void DiscountSortCheck()
BOOL DiscountSortCheck()
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
{
	int i;
	int nFoundCnt = ONL_DISC_MAX + 1;
	stDiscount2_t* pDisc = NULL;
	stDiscount2_t tmpDisc;
	BOOL bRepay = FALSE;
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
	BOOL bRet = FALSE;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))

	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=���Z
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ||		// 2=�Đ��Z
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4) ){		// 4=�Đ��Z���~
		bRepay = TRUE;
	}

	memset(&tmpDisc,0,sizeof(stDiscount2_t));

	// ��ʐ؂�ւ�������i�ݒ肪���������ꍇ�͏����𔲂���i���ƂŃG���[�ƂȂ�onlinediscount������Łj
	for( i =0; i < ONL_DISC_MAX; i++ ){
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(��ʐ؊��\�[�g�̕ύX)
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ���޽��(���z=1)
// 			case NTNET_SVS_T:			// ���޽��(����=101)
// 				// �T�[�r�X����͈̓`�F�b�N
// 				if( !rangechk( 1, SVS_MAX, pDisc->DiscCardNo ) ){
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// //					return;
// 					return FALSE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 				}
// 
// 				// �؊���̎Ԏ�ݒ�
// 				if( CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// //						return;										// ��ʐؑւȂ�
// 						return FALSE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 					}
// 					if(bRepay == FALSE){
// 						// ������(�㏑��)
// 						nFoundCnt = i;
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 						bRet = TRUE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 					}else{
// 						// �Đ��Z���́A�g�p�ς݂̏ꍇ�݈̂�ԏ�ɂ����Ă���
// 						if( pDisc->DiscStatus == 2){
// 							// ������(�㏑��)
// 							nFoundCnt = i;
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 							bRet = TRUE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 						}
// 					}
// 				}
// 				break;
// 			case NTNET_KAK_M:			// �X����(���z=2)
// 			case NTNET_KAK_T:			// �X����(����=102)
// 			case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
// 			case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
// 				// �X���͈̓`�F�b�N
// 				if( !rangechk( 1, MISE_NO_CNT, pDisc->DiscCardNo ) ){
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// //					return;
// 					return FALSE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 				}
// 
// 				// �؊���̎Ԏ�ݒ�
// 				if( CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// // MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// //						return;										// ��ʐؑւȂ�
// 						return FALSE;
// // MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
// 					}
		// �`�F�b�N
		if ( CheckDiscount(pDisc->DiscSyu, pDisc->DiscCardNo, pDisc->DiscInfo) ) {
			return FALSE;
		}

		// ��ʐ؊�����
		if ( GetTypeSwitch(pDisc->DiscSyu, pDisc->DiscCardNo) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(��ʐ؊��\�[�g�̕ύX)
			if ( bRepay == FALSE ) {
				// ������(�㏑��)
				nFoundCnt = i;
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
				bRet = TRUE;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
			} else {
				// �Đ��Z���́A�g�p�ς݂̏ꍇ�݈̂�ԏ�ɂ����Ă���
				if ( pDisc->DiscStatus == 2 ) {
					// ������(�㏑��)
					nFoundCnt = i;
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
					bRet = TRUE;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
				}
			}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(��ʐ؊��\�[�g�̕ύX)
// 				}
// 				break;
// 			default:
// 				continue;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(��ʐ؊��\�[�g�̕ύX)
		}
		if( nFoundCnt < (ONL_DISC_MAX + 1)){
			// ��������
			break;
		}
	}
	// 0�łȂ��āA�������ꍇ�i0�̏ꍇ�͓���ւ���K�v���Ȃ����߁j
	if( nFoundCnt != 0 && nFoundCnt < (ONL_DISC_MAX + 1)){
		// tmp�Ɉړ�
		memcpy(&tmpDisc,&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[nFoundCnt],sizeof(stDiscount2_t));
		for( i = nFoundCnt; i > 0; i--){
			// 1�����ɂ��炷
			memcpy( &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i],&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i-1],sizeof(stDiscount2_t));
		}
		// tmp��擪�Ɏ����Ă���
		memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[0],&tmpDisc,sizeof(stDiscount2_t));

	}
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
	return bRet;
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4745 �y���؉ێw�E�����z��ʐؑ֌�̍Đ��Z�ŏ��񐸎Z���Ɏg�p�����������K�p����Ă��Ȃ�(No58))
}

// �����̎�ʊ��������邩�ǂ����`�F�b�N�i�Đ��Z���́A�����X�e�[�^�X0�i����������������NG�j�j
BOOL IsDupSyubetuwari(void)
{
	BOOL bRet = TRUE;
	int i;
	int nFoundCnt = 0;
	stDiscount2_t* pDisc = NULL;
	BOOL bRepay = FALSE;
	BOOL bRepayNotDiscount = FALSE;

	if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=���Z
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ||		// 2=�Đ��Z
		(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4) ){		// 4=�Đ��Z���~
		bRepay = TRUE;
	}

	// ��ʐ؂�ւ�������
	for( i =0; i < ONL_DISC_MAX; i++ ){
		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ʐ؊��`�F�b�N�̕ύX)
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ���޽��(���z=1)
// 			case NTNET_SVS_T:			// ���޽��(����=101)
// 				// �T�[�r�X����͈̓`�F�b�N
// 				if( !rangechk( 1, SVS_MAX, pDisc->DiscCardNo ) ){
// 					continue;
// 				}
// 
// 				// �؊���̎Ԏ�ݒ�
// 				if( CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 						continue;										// ��ʐؑւȂ�
// 					}
// 					// ������
// 					nFoundCnt++;
// 					if(bRepay){
// 						// �Đ��Z���ŁA���������ǂ���
// 						if(pDisc->DiscStatus == 0){
// 							bRepayNotDiscount = TRUE;
// 						}
// 					}
// 				}
// 				break;
// 			case NTNET_KAK_M:			// �X����(���z=2)
// 			case NTNET_KAK_T:			// �X����(����=102)
// 			case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
// 			case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
// 				// �X���͈̓`�F�b�N
// 				if( !rangechk( 1, MISE_NO_CNT, pDisc->DiscCardNo ) ){
// 					continue;
// 				}
// 
// 				// �؊���̎Ԏ�ݒ�
// 				if( CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)] ){			// ��ʐؑ֗L��?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(pDisc->DiscCardNo-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 						continue;										// ��ʐؑւȂ�
// 					}
		// �`�F�b�N
		if ( CheckDiscount(pDisc->DiscSyu, pDisc->DiscCardNo, pDisc->DiscInfo) ) {
			continue;
		}

		// ��ʐ؊�����
		if ( GetTypeSwitch(pDisc->DiscSyu, pDisc->DiscCardNo) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ʐ؊��`�F�b�N�̕ύX)
			// ������
			nFoundCnt++;
			if ( bRepay ) {
				// �Đ��Z���ŁA���������ǂ���
				if ( pDisc->DiscStatus == 0 ) {
					bRepayNotDiscount = TRUE;
				}
			}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ʐ؊��`�F�b�N�̕ύX)
// 				}
// 				break;
// 			default:
// 				continue;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������ʐ؊��`�F�b�N�̕ύX)
		}
	}
	if(nFoundCnt > 1){
		// 2�ȏ㔭��
		bRet = TRUE;
	}else{
		// 1�ȉ�
// MH810100(S) 2020/08/19 �Ԕԃ`�P�b�g���X(#4744 �y���؉ێw�E�����zQR������(1h)������������̎�ʐؑւŖ����ɂȂ�(No56))
//		bRet = FALSE;
		// 1�������ς݂�
// MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
//		if( nFoundCnt && ryo_buf.nyukin){
		// �����O�Ɏ�ʐؑփ`�F�b�N�ς�g_checkKirikae��1
		if( nFoundCnt && ryo_buf.nyukin && g_checkKirikae == 0){
// MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
			// ���łɓ����ς�
			bRet = TRUE;
		}else{
			bRet = FALSE;
		}
// MH810100(E) 2020/08/19 �Ԕԃ`�P�b�g���X(#4744 �y���؉ێw�E�����zQR������(1h)������������̎�ʐؑւŖ����ɂȂ�(No56))
	}
	// �Đ��Z�Ŗ������̎�ʐؑւ𔭌�
	if(bRepayNotDiscount){
		bRet  = TRUE;
	}
	return bRet;
}

// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// // ������񂩂�ݎԏ���Copy����
// void CopyDiscountCalcInfoToZaisha(void)
// {
// 	uchar 	loop_cnt;
// 	stDiscount2_t* pDisc2;
// 	stDiscount_t*  pDisc3;
// 	/************************************************************************************/
// 	/* �ݎԏ��_����01�`25�͊����̐��Z�����I����(OnlineDiscount = ��ײ݊�������)��		*/
// 	/* m_stDisc�����߰����̂ł��̊������Z����ް����g�p����							*/
// 	/************************************************************************************/
// 	taskchg(IDLETSKNO);
// 	for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
// 		pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
// 		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
// 
// 		pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// ���� ���ԏꇂ(0�`999999)
// 		pDisc3->DiscSyu			= pDisc2->DiscSyu;				// ���� ���(0�`9999)
// 		pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// ���� ���ދ敪(0�`65000)
// 		pDisc3->DiscNo			= pDisc2->DiscNo;				// ���� �敪(0�`9)
// 		pDisc3->DiscCount		= pDisc2->DiscCount;			// ���� ����(0�`99)
// 		pDisc3->DiscInfo		= pDisc2->DiscInfo;				// ���� �������(0�`65000)
// 		pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// ���� �Ή����ގ��(0�`65000)
// 		pDisc3->DiscStatus		= pDisc2->DiscStatus;			// ���� �ð��(0�`9)
// 		pDisc3->DiscFlg			= pDisc2->DiscFlg;				// ���� ������(0�`9)
// 
// 		pDisc3->Discount		= pDisc2->UsedDisc;				// ���z/����(0�`999999)
// 	}
// }
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// MH810100(S) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// BOOL IsWaribikiLimitOver(void)
// {
// 	BOOL bRet = FALSE;
// 	int i;
// 	int nFoundCnt = 0;
// 	stDiscount2_t* pDisc = NULL;
// 
// 	// �L���Ȋ��������J�E���g����
// 	for( i =0; i < ONL_DISC_MAX; i++ ){
// 		pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
// 		switch (pDisc->DiscSyu) {
// 			case NTNET_SVS_M:			// ���޽��(���z=1)
// 			case NTNET_SVS_T:			// ���޽��(����=101)
// 			case NTNET_KAK_M:			// �X����(���z=2)
// 			case NTNET_KAK_T:			// �X����(����=102)
// 			case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
// 			case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
// // MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������������`�F�b�N�̕ύX)
// 			case NTNET_TKAK_M:			// ���X��(���z=3)
// 			case NTNET_TKAK_T:			// ���X��(����=103)
// // MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(������������`�F�b�N�̕ύX)
// 				nFoundCnt++;
// 				break;
// 			default:
// 				continue;
// 		}
// 	}
// 	if( nFoundCnt > WTIK_USEMAX){
// 		bRet = TRUE;
// 	}
// 	return bRet;
// }
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) 2020/09/04 #4584 �y���؉ێw�E�����z�@ ���ꊄ�����퐔�̐������u�P�v�̏ꍇ�iNo33 �ĕ]��NG�j(��������Ή�)
// MH810103(s) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
void RTPay_pre_sousai(void)
{
	// ryo_cal�����s����O�ɏ����Z�b�g����
	m_stDisc.DiscParkNo = CPrmSS[S_SYS][1];			// ��{���ԏ�ԍ����
	m_stDisc.DiscCount = 1;							// 1��
	m_stDisc.Discount = 0;							// 0�N���A
	m_stDisc.DiscNo = 0;							// 0:��{
	m_stDisc.DiscInfo = 0;
	m_stDisc.DiscCorrType = 0;
	m_stDisc.DiscStatus = 1;						// ���񊄈�
	m_stDisc.DiscFlg = 0;							// 

	// vl_sousai�֐��ɂ�vl_svs�ɃZ�b�g����Ă���̂ł������Q��
	if(vl_svs.mno != 0){
		// �X��
		m_stDisc.DiscSyu = NTNET_KAK_M;
		m_stDisc.DiscCardNo = vl_svs.mno;
	}else{
		// �T�[�r�X��
		m_stDisc.DiscSyu = NTNET_SVS_M;
		m_stDisc.DiscCardNo = vl_svs.sno;

	}

}

void RTPay_set_sousai(void)
{
	short cnt = 0;

	// ���E���������{���ꂽ�̂ŁA���A���^�C�����Z�f�[�^�p�f�[�^�֐��Z���ʂ���������
	for( cnt = 0; cnt < ONL_DISC_MAX; cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt].DiscParkNo == 0L ) &&
			( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt].DiscSyu == 0 )){
			// ��G���A����
			break;
		}
	}
	if( cnt < ONL_DISC_MAX){
		// ���A���^�C�����Z�f�[�^�p�f�[�^�֐��Z���ʂ���������
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt], &m_stDisc, sizeof(m_stDisc) );
	}
}
// MH810103(e) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�

// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
//[]----------------------------------------------------------------------[]
///	@brief			��ʐ؊��擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		syu		: ���
///	@param[in]		card_no	: �J�[�h�敪
///	@return			��ʐ؊�<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2021/08/30<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
short GetTypeSwitch(ushort syu, ushort card_no)
{
	short typeswitch = 0;

	switch ( syu ) {
	case NTNET_SVS_M:			// ���޽��(���z=1)
	case NTNET_SVS_T:			// ���޽��(����=101)
		// �T�[�r�X���ݒ肩��擾
		typeswitch = (short)prm_get(COM_PRM, S_SER, (card_no * 3), 2, 1);
		break;
	case NTNET_KAK_M:			// �X����(���z=2)
	case NTNET_KAK_T:			// �X����(����=102)
	case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
	case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
		// �X�����ݒ肩��擾
		typeswitch = (short)prm_get(COM_PRM, S_STO, (card_no * 3), 2, 1);
		break;
	case NTNET_TKAK_M:			// ���X�܊���(���z=3)
	case NTNET_TKAK_T:			// ���X�܊���(����=103)
		if ( card_no > MISE_NO_CNT ) {
			// ���X�܊����ݒ肩��擾
			typeswitch = (short)prm_get(COM_PRM, S_TAT, (card_no - 69), 2, 1);
		} else {
			// �X�����ݒ肩��擾
			typeswitch = (short)prm_get(COM_PRM, S_STO, (card_no * 3), 2, 1);
		}
		break;
	default:
		break;
	}

	return typeswitch;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		syu		: ���
///	@param[in]		card_no	: �J�[�h�敪
///	@param[in]		info	: �������
///	@return			0  : OK<br>
///					13 : ��ʋK��O<br>
///					27 : ������ʴװ<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2021/08/30<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
short CheckDiscount(ushort syu, ushort card_no, ushort info)
{
	short typeswitch = 0;

	switch ( syu ) {
	case NTNET_SVS_M:			// ���޽��(���z=1)
	case NTNET_SVS_T:			// ���޽��(����=101)
		// �T�[�r�X����͈̓`�F�b�N
		if ( !rangechk(1, SVS_MAX, card_no) ) {
			return 13;
		}
		break;
	case NTNET_KAK_M:			// �X����(���z=2)
	case NTNET_KAK_T:			// �X����(����=102)
	case NTNET_SHOP_DISC_AMT:	// ��������(���z=6)
	case NTNET_SHOP_DISC_TIME:	// ��������(����=106)
		// �X���͈̓`�F�b�N
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
// 		if ( !rangechk(1, 100, card_no) ) {
		if ( !rangechk(1, MISE_NO_CNT, card_no) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
			return 13;
		}
		break;
	case NTNET_TKAK_M:			// ���X�܊���(���z=3)
	case NTNET_TKAK_T:			// ���X�܊���(����=103)
		// �X���͈̓`�F�b�N
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
// 		if ( !rangechk(1, 255, card_no) ) {
		if ( !rangechk(1, T_MISE_NO_CNT, card_no) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
			return 13;
		}
		// ��ʔ͈̓`�F�b�N
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
// 		if ( !rangechk(1, 7, info) ) {
		if ( !rangechk(1, T_MISE_SYU_CNT, info) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
			return 13;
		}
		break;
	default:
		break;
	}

	// ��ʐ؊���̎Ԏ�ݒ�`�F�b�N
	typeswitch = GetTypeSwitch(syu, card_no);
	if ( typeswitch ) {
		if ( prm_get(COM_PRM, S_SHA, ((typeswitch * 6) - 5), 2, 5) == 0 ) {
			return 27;
		}
	}

	return 0;
}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�I�����C�����������̕ύX)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[�̈󎚏���
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/11
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcFailureContactPri( void )
{
	T_FrmReceipt	FailureContactData;
	ushort	num;

	memset(&FailureContactData, 0, sizeof(T_FrmReceipt));

	FailureContactData.prn_kind = R_PRI;		/* �������ʁFڼ�� */

	memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );

	// �Ԏ��ԍ����܂��Z�b�g����Ă��Ȃ����߁APayData_set�Ɠ���������
	// �Ԏ��ԍ��Z�b�g���s��
	if( Cancel_pri_work.teiki.update_mon ){							// �X�V���Z?
		Cancel_pri_work.WPlace = 9999;								// �X�V���͎Ԏ��ԍ�9999�Ƃ���B
	}else if( ryo_buf.pkiti == 0xffff ){							// ������Z?
		Cancel_pri_work.WPlace = LOCKMULTI.LockNo;					// �ڋq�p���Ԉʒu�ԍ�
	}else if( ryo_buf.pkiti == 0 ){									// ������Z�H
		// ���u���Z�i������Z�j�͒��Ԉʒu�ԍ����O�̂��߁A��M�����Ԏ������Z�b�g����
		Cancel_pri_work.WPlace = OPECTL.Op_LokNo;
	}else{
		num = ryo_buf.pkiti - 1;
		Cancel_pri_work.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// �ڋq�p���Ԉʒu�ԍ�
	}
	FailureContactData.prn_data = &Cancel_pri_work;		// �̎��؈��ް����߲�����

	// pritask�֒ʒm
	queset(PRNTCBNO, PREQ_RECV_FAILURECONTACT, sizeof(T_FrmReceipt), &FailureContactData);
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
//// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��؃v�����g�ݒ�j
//const uchar pri_prm_tbl[2][2][2][12] = {
const uchar pri_prm_tbl[2][2][2][15] = {
	// �C���{�C�X�Ȃ��A�d�q�}�l�[���p�Ȃ��A�W���[�i���v�����^
//	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	// �C���{�C�X�Ȃ��A�d�q�}�l�[���p�Ȃ��A�d�q�W���[�i��
//	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	// �C���{�C�X�Ȃ��A�d�q�}�l�[���p����A�W���[�i���v�����^
//	0, 1, 1, 3, 4, 4, 3, 4, 4, 9,10, 4,
	0, 1, 1, 3, 4, 4, 9,10,10, 9,10,10,12,13,13,
	// �C���{�C�X�Ȃ��A�d�q�}�l�[���p����A�d�q�W���[�i��
//	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	// �C���{�C�X����A�d�q�}�l�[���p�Ȃ��A�W���[�i���v�����^
//	0, 1, 2, 0, 4, 5, 0, 4, 8, 0, 4,11,
	0, 1, 2, 3, 4, 5,12,13,14,12,13,14,12,13,14,
	// �C���{�C�X����A�d�q�}�l�[���p�Ȃ��A�d�q�W���[�i��
//	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
	// �C���{�C�X����A�d�q�}�l�[���p����A�W���[�i���v�����^
//	0, 1, 1, 0, 4, 4, 0, 4, 4, 0, 4, 4,
	0, 1, 1, 3, 4, 4,12,13,13,12,13,13,12,13,13,
	// �C���{�C�X����A�d�q�}�l�[���p����A�d�q�W���[�i��
//	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1,
};
//// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��؃v�����g�ݒ�j
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�C���{�C�X����17-0001�̃p�����[�^�ϊ�
//[]----------------------------------------------------------------------[]
///	@return			jnl_pri_jadge�@�ϊ���̒l
///	@author			Hayakawa
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/05
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long invoice_prt_param_check( long param_jadge )
{
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��؃v�����g�ݒ�j
//	if ( IS_INVOICE ) {							// �C���{�C�X�K�p��
//		if (isEMoneyReader()){					// �d�q�}�l�[�^�p����
//			if (!isEJA_USE()){					// �W���[�i���v�����^�̏ꍇ
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 2:
//					param_jadge = 1;
//					break;
//				case 5:
//				case 7:
//				case 8:
//				case 10:
//				case 11:
//					param_jadge = 4;
//					break;
//				default:						// ����ȊO��param_jadge�̒l�͂��̂܂�
//					break;
//				}
//			}
//			else {								// �d�q�W���[�i���̏ꍇ
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 2:
//				case 4:
//				case 5:
//				case 7:
//				case 8:
//				case 10:
//				case 11:
//					param_jadge = 1;
//					break;
//				default:						// ����ȊO��param_jadge�̒l�͂��̂܂�
//					break;
//				}
//			}
//		}
//		else {									// �d�q�}�l�[�^�p�Ȃ�
//			if (!isEJA_USE()){					// �W���[�i���v�����^�̏ꍇ
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 7:
//				case 10:
//					param_jadge = 4;
//					break;
//				default:						// ����ȊO��param_jadge�̒l�͂��̂܂�
//					break;
//				}
//			}
//			else {								// �d�q�W���[�i���̏ꍇ
//				switch(param_jadge){
//				case 3:
//				case 6:
//				case 9:
//					param_jadge = 0;
//					break;
//				case 4:
//				case 7:
//				case 10:
//					param_jadge = 1;
//					break;
//				case 5:
//				case 8:
//				case 11:
//					param_jadge = 2;
//					break;
//				default:						// ����ȊO��param_jadge�̒l�͂��̂܂�
//					break;
//				}
//			}
//		}
//	}
//	return param_jadge;						// �C���{�C�X��K�p���Ȃ��ꍇ�͈��������̂܂ܕԂ�
	uchar	ec_brandchk;
	uchar	inv, emn, eja;

	ec_brandchk = 0;
	if (isEC_USE()){
		if (isEcEmoneyEnabled(0, 0) ||
			isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ||
			isEcBrandNoEnabledForRecvTbl(BRANDNO_QR)){
			ec_brandchk = 1;
		}
	}

	// �C���{�C�X
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//	inv = IS_INVOICE;
	inv = IS_INVOICE_PRM;
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
	// �d�q�}�l�[���p
	emn = 0;
	if (isSX10_USE() ||	ec_brandchk) {
		emn = 1;
	}
	// �d�q�W���[�i���ڑ�
	eja = isEJA_USE();

// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
//	if (param_jadge > 11) {
	if (param_jadge > 14) {
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
		if (!inv && !emn && !eja) {
			// �ǂݑւ��Ȃ�
			return param_jadge;
		}
		else {
			// 0�Ƃ��Ĉ���
			return 0;
		}
	}

	// �ݒ�ǂݑւ�
	return (long)pri_prm_tbl[inv][emn][eja][param_jadge];

// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��؃v�����g�ݒ�j
}
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//[]----------------------------------------------------------------------[]
///	@brief		���~�̎��ؔ��s�Ώ۔��菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=�ΏۊO, 1=�Ώ�
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/06/01<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
uchar	cancelReceipt_chk( void )
{
	uchar i;
	uchar ret = 0;
	wari_tiket	wari_dt;

	// �v���y�C�h�g�p/�񐔌��g�p
	if( card_use[USE_PPC] || card_use[USE_NUM] ){
		// �ېőΏۂɂ���
		if( prm_get( COM_PRM, S_PRP, 1, 1, 2 ) == 0 ){
			ret = 1;
			return ret;
		}
	}

	// �g�p���������f�[�^����ېőΏۊ����f�[�^������
	for( i = 0; i < WTIK_USEMAX; i++ )
	{

		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );
		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){
			break;
		}

		// �ېőΏېݒ肪�L���ł���A����������0�~�łȂ��ꍇ�̂ݒ��~�̎��ؔ��s�ΏۂƂ���
		if( cancelReceipt_Waridata_chk( &wari_dt ) &&
			wari_dt.ryokin != 0 ){
			ret = 1;
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		�����f�[�^���~�̎��ؔ��s�Ώ۔��菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=�ΏۊO, 1=�Ώ�
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/06/03<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
uchar	cancelReceipt_Waridata_chk( wari_tiket *wk_waridata )
{
	uchar	ret = 0;
	ushort	svs_syu = 0;
	short	section = 0;

	if( wk_waridata->tik_syu == 0 || wk_waridata->tik_syu == INVALID ){
		return ret;
	}

	svs_syu = wk_waridata->syubetu - 1;

	switch( wk_waridata->tik_syu )
	{
		case SERVICE:
			section = S_SER;
			break;

		case KAKEURI:
			// �XNo.101�ȏ�i���X�܊����j�͉ېőΏۂƂ��Ȃ�
			if( wk_waridata->syubetu >= 101 ){
				return ret;
			}
			section = S_STO;
			break;

		default:
			return ret;
	}

	if( prm_get( COM_PRM, section, (short)(1 + (svs_syu * 3)), 1, 2 ) == 1 ){
		ret = 1;
	}

	return ret;
}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
//[]----------------------------------------------------------------------[]
///	@brief		�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ulong	*p_dat 14-0005,0006�@��������Z�ݒ�
///	@param[in]		uchar	taxableAdd_set�@14-0007�D
///							0:������^���|���ɉ��Z���Ȃ�
///							1:������^���|���ւ̉��Z�ݒ�(14-0005,0006)�ɏ]��
///							2:������ɉ��Z����
///							3:���|���ɉ��Z����
///							4:�������Z����
///						 0xff:�������Ȃ�
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2022/08/05<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void	setting_taxableAdd( ulong *p_dat, uchar taxableAdd_set )
{

	if( p_dat == NULL ){
		return;
	}

	if( taxableAdd_set == 0xff ){									// �ېőΏۂł͂Ȃ�
		return;														// �������Ȃ�
	}
	else{															// �ېőΏۂł���
		switch (taxableAdd_set)										// 14-0007�D�ɏ]��
		{
		case 0:	// ������^���|���ɉ��Z���Ȃ�
			(*p_dat) = 0L;
			break;
		default:
		case 1:	// ������^���|���ւ̉��Z�ݒ�(14-0005,0006)�ɏ]��
			// �������Ȃ�
			break;
		case 2:	// ������ɉ��Z����
			(*p_dat) = 1L;
			break;
		case 3:	// ���|���ɉ��Z����
			(*p_dat) = 2L;
			break;
		case 4:	// �������Z����
			(*p_dat) = 3L;
			break;
		}
	}
}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			�̎��؈󎚃f�[�^�X�g�b�N
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2023/09/28
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	pri_ryo_stock( uchar kind )
{
	T_FrmReceipt	ReceiptPreqData_Stock;

	memset(&ReceiptPreqData_Stock, 0, sizeof(T_FrmReceipt));

	// ���߈󎚂��s�����ߐ�Ƀf�[�^�𑗐M���Ă���
	// �V�v�����^�̂ݗL��
	if( isNewModel_R() ){							// �V�v�����^

// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 		if (Ope_isPrinterReady() == 0 ||
		if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
			(IS_INVOICE && Ope_isJPrinterReady() == 0)) {
			// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł������Ȃ������ꍇ
			// �󎚕s�̏ꍇ�̓X�g�b�N���Ȃ�
			return;
		}

		// �X�g�b�N
		if( ryo_stock == 0 ){						// �X�g�b�N�Ȃ�

			ReceiptPreqData_Stock.prn_kind = kind;	// ���
			ReceiptPreqData_Stock.prn_data = &PayData;

			// �X�g�b�N�Ώۃf�[�^�t���O�Ƃ��Ď؂��i�����ł����g�p���Ȃ����ߖ��Ȃ��j
			// PREQ_RYOUSYUU�����ŃX�g�b�N�Ώۃf�[�^���𔻒肷�邽�߂Ɏg�p����
			ReceiptPreqData_Stock.dummy = 1;
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData_Stock);

			ryo_stock = 1;							// �X�g�b�N����
		}
	}
}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
//[]----------------------------------------------------------------------[]
///	@brief		�����z�v�Z����
//[]----------------------------------------------------------------------[]
///	@return		�����z�i���ԗ���-�ېőΏۊO�����z�j
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2024/2/19<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
ulong Billingcalculation(Receipt_data *data)
{
	uchar			i;
	ulong			taxNotDis_total = 0;		// �ېőΏۊO�����z
	ulong			billing = 0;				// �����z
	wari_tiket		wari_dt;

	// �ېőΏۊz�A�����z���v�Z����
	// �O�񊄈��z
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// �������ް�����
		disc_wari_conv_prev( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// �����ް��Ȃ�
			continue;
		}
		if( wari_dt.ryokin ){					// ������������
			switch ( wari_dt.tik_syu ) {
			case C_SERVICE:			// ���޽��
			case C_KAKEURI:			// �|����
			case C_SHOPPING:		// ��������
				taxNotDis_total += wari_dt.ryokin;	// �ېőΏۊO
				break;
			default:				// ���̑�
				// �����ȊO
				break;
			}
		}
	}
 	// ���񊄈��z�i�܂Ƃ܂�ꍇ������ׁA�ŏ�����Č����j
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// �������ް�����
		disc_wari_conv_new( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// �����ް��Ȃ�
			continue;
		}
		if( wari_dt.ryokin ){					// ������������
			switch ( wari_dt.tik_syu ) {
			case SERVICE:			// ���޽��
			case KAKEURI:			// �|����
				if ( !cancelReceipt_Waridata_chk(&wari_dt) ) {
					taxNotDis_total += wari_dt.ryokin;	// �ېőΏۊO
				}
				break;
			case KAISUU:			// �񐔌�
			case PREPAID:			// �v���y�C�h�J�[�h
				if ( prm_get(COM_PRM, S_PRP, 1, 1, 2) == 1 ) {
					taxNotDis_total += wari_dt.ryokin;	// �ېőΏۊO
				}
				break;
			case SHOPPING:			// ��������
			case C_SERVICE:			// ���޽��
			case C_KAKEURI:			// �|����
			case C_KAISUU:			// ���Z���~�񐔌�
			case C_SHOPPING:		// ��������
				taxNotDis_total += wari_dt.ryokin;	// �ېőΏۊO
				break;
			default:				// ���̑�
				// �����ȊO
				break;
			}
		}
	}
	// �O�񎞊ԁi�����j������O������߲�ޗ��p���z�v�Z
	taxNotDis_total += (data->ppc_chusi_ryo +
						data->zenkai);

	// �����z
	if( data->WPrice > taxNotDis_total ){ 			// �����z��蒓�ԗ����̕����傫���ꍇ
		billing = data->WPrice - taxNotDis_total;	// �����z�i���ԗ���-�ېőΏۊO�����z�j
	}
	else{
		billing = 0;
	}

	return billing;
}
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�

// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
//[]----------------------------------------------------------------------[]
///	@brief		�ېőΏۊ����z�v�Z
//[]----------------------------------------------------------------------[]
///	@param[in]	data	: PayData�̃|�C���^
///	@param[in]	typ		: 0=���Z���� 1=���Z���~
///	@return		�ېőΏۊ����z
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2024/5/20<br>
///				Update	:
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
ulong	TaxAmountcalculation(Receipt_data *data , uchar typ)
{
	uchar			i;
	ulong			tax_amount = 0;			// �̎��z�i�ېőΏۊz�j
	wari_tiket		wari_dt;

	// TODO:�ېőΏۊ����Ή����ɏC������
	// �ېőΏۊz���v�Z����
	for( i = 0; i < WTIK_USEMAX ; i++ ){		// �������ް�����
		disc_wari_conv_new( &data->DiscountData[i], &wari_dt );
 		if( wari_dt.tik_syu == 0 || wari_dt.tik_syu == INVALID ){	// �����ް��Ȃ�
			continue;
		}
		if( wari_dt.ryokin ){					// ������������
			switch ( wari_dt.tik_syu ) {
			case SERVICE:			// ���޽��
			case KAKEURI:			// �|����
				if ( cancelReceipt_Waridata_chk(&wari_dt) ) {
					tax_amount += wari_dt.ryokin;	// �ېőΏ�
				}
				break;
			case KAISUU:			// �񐔌�
			case PREPAID:			// �v���y�C�h�J�[�h
				if ( prm_get(COM_PRM, S_PRP, 1, 1, 2) == 0 ) {
					tax_amount += wari_dt.ryokin;	// �ېőΏ�
				}
				break;
			default:				// ���̑�
				// �����ȊO
				break;
			}
		}
	}

	if (typ == 0) {
		// �ېőΏۊz�ɓd�q�Ȱ�A�ڼޯāA�����̎��z�����Z
		tax_amount += (data->Electron_data.Ec.pay_ryo +
						data->credit.pay_ryo +
						(data->WInPrice - data->WChgPrice));
	}

	return tax_amount;
}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
