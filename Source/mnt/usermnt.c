/*[]----------------------------------------------------------------------[]*/
/*| հ�ް����ݽ����                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"irq1.h"
#include	"flp_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"tbl_rkn.h"
#include	"cnm_def.h"
#include	"mdl_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mif.h"
#include	"sysmnt_def.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include 	"ksg_def.h"
#include	"raudef.h"
#include	"ntnetauto.h"
#include	"oiban.h"
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#include	"remote_dl.h"
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)

/* Function Define */
unsigned short	UsMnt_Clock( void );
unsigned short	UsMnt_Clock2( void );
unsigned short	UsMnt_shtctl( void );
unsigned short	UsMnt_OCsw( void );
unsigned short	UsMnt_mnyctl( void );
unsigned short	UsMnt_mnyctl2( void );
unsigned short	UsMnt_mnychg( void );
unsigned short	UsMnt_invtry( void );
unsigned short	UsMnt_mnybox( void );

unsigned short	UsMnt_Total( short );
unsigned short	UsMnt_Mnctlmenu( void );
unsigned short	UsMnt_ErrAlm( void );
unsigned short	UsMnt_TicketCkk(void);
// MH810100(S) K.Onodera 2019/12/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
unsigned short	UsMnt_QR_DataCkk(void);
// MH810100(E) K.Onodera 2019/12/04 �Ԕԃ`�P�b�g���X(�����e�i���X)

static int mnyctl_nuchk( uchar, short, uchar, uchar );
static void mnyctl_dsp( ushort, ushort, ushort, ushort );
static uchar mnyctl_cur( uchar, ushort, ushort );
static void clk_sub( short * );
static void HasseiErrSearch( short );
static void ServiceDsp( void );
static void PassDsp( void );
static void PrepaidDsp( short );
static void KaiDsp( void );
static uchar AmanoCard_Dsp( void );
static int	CheckPrinter(uchar pri_kind);
static int	check_print_result(int no);
static void	syukei_all_dsp(uchar topNo, uchar rev_no, uchar elog );
// MH810105 GG119202(S) T���v�A���󎚑Ή�
//extern	uchar	SysMnt_Work[];	/* 32KB */
// MH810105 GG119202(E) T���v�A���󎚑Ή�


/*[]----------------------------------------------------------------------[]*/
/*| ���ʃp�����[�^�X�V�`�F�b�N���T���X�V����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UserMnt_SysParaUpdateCheck( ushort )                    |*/
/*| PARAMETER    : wkOpelogNo : OpelogNo �Ɠ���l                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���[�U�[�����e�i���X��ʏI�����_��Call���鏈���ŁA���엚���f�[�^       |*/
/*| �iOpelogNo�j���狤�ʃp�����[�^(CPrm[])���ύX����Ă��邩�ۂ����`�F�b�N |*/
/*| ���A�ύX����Ă���ꍇ�͋��ʃp�����[�^�̃T�����X�V���鏈���B           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar UserMnt_SysParaUpdateCheck( ushort	wkOpelogNo )
{	
	uchar	ret = 0;										// 0:�p�����[�^�X�V�Ȃ�
															// 1:�p�����[�^�X�V����
	int	doSumCheck = 0;

	switch( wkOpelogNo ){
	case	OPLOG_SHOKIMAISUHENKO:							// �ޑK�Ǘ��|���������ύX
	case	OPLOG_TOKUBETUBI:								// ���ʓ�
	case	OPLOG_TOKUBETUKIKAN:							// ���ʊ���
	case	OPLOG_TOKUBETUHAPPY:							// �n�b�s�[�}���f�[
	case	OPLOG_TOKUBETUYOBI:								// ���ʗj��
	case	OPLOG_SERVICETIME:								// �T�[�r�X�^�C��
	case	OPLOG_ROCKTIMER:								// ���b�N���u�^�C�}�[
	case	OPLOG_KAKARIINPASS:								// �W���p�X���[�h
	case	OPLOG_BACKLIGHT:								// �o�b�N���C�g�_��
	case	OPLOG_KEYVOLUME:								// �u�U�[���ʒ���
	case	OPLOG_KENKIGEN:									// ������
	case	OPLOG_EIGYOKAISHI:								// �c�ƊJ�n����
	case	OPLOG_SAIDAIRYOKIN:								// �ő嗿��
	case	OPLOG_SERVICEYAKUWARI:							// �T�[�r�X������
	case	OPLOG_MISEKUWARI:								// �X����
	case	OPLOG_SHOHIZEI:									// ����ł̐ŗ�
	case	OPLOG_ALYAKUWARI:								// �`�`�k�����
	case	OPLOG_TANIJIKANRYO:								// �P�ʎ��ԗ���
	case	OPLOG_TEIKIMUKOYOUBI:							// ��������j��
	case	OPLOG_CONTRAST:									// �R���g���X�g����
	case	OPLOG_VLSWTIME:									// ���ʐؑ֎���
	case	OPLOG_TOKUBETUWEEKLY:							// ���ʗj��

		doSumCheck = -1;
	}

	if( doSumCheck | SetChange ){
			// �`�F�b�N�T���m�f
			// 1.���ʃp�����[�^�̃T�����Čv�Z
			DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
			// 2.�t���b�V���ɃZ�[�u
			(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
			// 3.�����v�Z�p�ݒ���X�V
			mc10();	//���̏����͏d������,����ĂԕK�v�������̂œd��ON���Ɛݒ�ύX���̂�Call���鎖�Ƃ���B
			// 4.�ݒ�X�V�������O�o�^
			SetSetDiff(SETDIFFLOG_SYU_USRMNT);
			f_ParaUpdate.BYTE = 0;							// ���d����RAM��p�����[�^�f�[�^��SUM�X�V���Ȃ�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
		ret = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| հ�ް����ݽҲݏ���                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UserMntMain( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void UserMntMain( void )
{
	unsigned short	usUserEvent;
	char	wk[2];
	unsigned short	parm;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar ucReq = 0;	// 0=OK/1=NG/2=OK(�ċN���s�v)
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//	ushort	mode = 0;
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	wtype = 0;			// ��Ɨp�̈ꎞ�̈�
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[0] );			// [00]	"�����[�U�[�����e�i���X���@�@�@"

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// 		usUserEvent = Is_MenuStrMake( 1 );
		if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
			usUserEvent = Is_MenuStrMake(3);
		} else {					// �ʏ헿���v�Z���[�h
			usUserEvent = Is_MenuStrMake(1);
		}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		if( OPECTL.Mnt_lev < 2 ){
			// �W�������ł͖����Ƃ���
			switch( usUserEvent ){
				case MNT_CLOCK:		/* ���v���킹 */
				case MNT_MNCNT:		/* �ޑK�Ǘ� */
				case MNT_INVLD:		/* ����L���^����(���g�p) */
				case MNT_ENTRD:		/* ������Ɂ^�o��(���g�p) */
				case MNT_SERVS:		/* �T�[�r�X�^�C�� */
				case MNT_FLTIM:		/* ���b�N���u�J�^�C�}�[(���g�p) */
				case MNT_SPCAL:		/* ���ʓ��^���ʊ��� */
				case MNT_PWMOD:		/* �W���p�X���[�h */
				case MNT_BKLIT:		/* �o�b�N���C�g�_�����@ */
				case MNT_TKEXP:		/* ������(���g�p) */
				case MNT_OPNHR:		/* �c�ƊJ�n���� */
				case MNT_PSTOP:		/* ��������Z���~�f�[�^(���g�p) */
				case MNT_PWDKY:		/* �Ïؔԍ������o��(���g�p) */
				case MNT_VLSW:		/* ���ʐؑ֎��� */
				case MNT_MNYSET:	/* �����ݒ� */
					BUZPIPI();
					continue;
			}
		}

		OpelogNo = 0;
		SetChange = 0;
		OpelogNo2 = 0;

		switch( usUserEvent ){
	/* �s�W�v */
		case MNT_TTOTL:
			usUserEvent = UsMnt_Total(MNT_TTOTL);
			break;

	/* ���b�N���u�J��(���g�p) */
		case MNT_FLCTL:
			if( Is_CarMenuMake(LOCK_CTRL_MENU) != 0 ){
				usUserEvent = UsMnt_PreAreaSelect(MNT_FLCTL);
			}else{
				BUZPIPI();
				continue;
			}
			break;
	/* �Ԏ����(���g�p) */
		case MNT_FLSTS:
			if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
				if( Ext_Menu_Max > 1){
					usUserEvent = UsMnt_PreAreaSelect(MNT_FLSTS);
				}
				else {
					DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
					usUserEvent = UsMnt_AreaSelect(MNT_FLSTS);
				}
			}else{
				BUZPIPI();
				continue;
			}
			break;

	/* �̎��؍Ĕ��s */
		case MNT_REPRT:
			usUserEvent = UsMnt_Receipt();
			break;

	/* ��t���Ĕ��s(���g�p) */
		case MNT_UKERP:
			usUserEvent = UsMnt_ParkingCertificate();
			break;

	/* �G���[�E�A���[���m�F */
		case MNT_ERARM:
			usUserEvent = UsMnt_ErrAlm();		 	// �G���[�E�A���[���m�F
			break;

	/* �◯�ԏ��(���g�p) */
		case MNT_STAYD:
			usUserEvent = UsMnt_StaySts();
			break;

	/* �������v�����g */
		case MNT_LOGPRN:
			usUserEvent = UsMnt_Logprintmenu();
			break;

	/* �Ԏ��̏�(���g�p) */
		case MNT_CARFAIL:
			if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
				if( Ext_Menu_Max > 1){
					usUserEvent = UsMnt_PreAreaSelect(MNT_CARFAIL);
				}
				else {
					usUserEvent = UsMnt_AreaSelect(MNT_CARFAIL);
				}
			}else{
				BUZPIPI();
				continue;
			}
			break;

	/* ���v���킹 */
		case MNT_CLOCK:
			usUserEvent = UsMnt_Clock();
			break;
	/* ���ʐؑ֎��� */
		case MNT_VLSW:
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//			usUserEvent = UsMnt_VoiceGuideTime();
			//���σ��[�_�ڑ��ݒ肠��H
			if( isEC_USE() ){
				usUserEvent = UsMnt_ECVoiceGuide(); 
			}else{
				usUserEvent = UsMnt_VoiceGuideTime();
			}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

			break;

	/* ���ԑ䐔(���g�p) */
		case MNT_FLCNT:
			usUserEvent = UsMnt_ParkCnt();
			break;
	/* ���ԃR���g���[��(���g�p) */
		case MNT_FLCTRL:
			usUserEvent = UsMnt_FullCtrl();
			break;

	/* �ޑK�Ǘ� */
		case MNT_MNCNT:
			usUserEvent = UsMnt_Mnctlmenu();
			break;
	/* �f�s�W�v */
		case MNT_GTOTL:
			usUserEvent = UsMnt_Total(MNT_GTOTL);
			break;
	/* �c�x�Ɛؑ� */
		case MNT_OPCLS:
			usUserEvent = UsMnt_OCsw();
			break;
	/* �V���b�^�[�J��(���g�p) */
		case MNT_SHTER:
			usUserEvent = UsMnt_shtctl();
			break;
	/* ���f�[�^�m�F(���g�p) */
		case MNT_TKTDT:
			usUserEvent = UsMnt_TicketCkk();
			break;

// MH810100(S) Y.Yamauchi 2019/11/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
	/* QR�f�[�^�m�F */
		case MNT_QRCHECK:
			usUserEvent = UsMnt_QR_DataCkk();
			break;
// MH810100(E) Y.Yamauchi 2019/11/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

	/* �����ݒ� */
		case MNT_MNYSET:
			usUserEvent = UsMnt_Mnysetmenu();
			break;
	/* �T�[�r�X�^�C�� */
		case MNT_SERVS:
			usUserEvent = UsMnt_SrvTime();
			break;

	/* ���b�N���u�J�^�C�}�[(���g�p) */
		case MNT_FLTIM:
			if(( parm = (ushort)GetCarInfoParam()) == 0 ){
				BUZPIPI();
				continue;
			}
			if ((parm & 0x01) && ( parm & 0x06 )) {		//���ցE���ԕ���
				usUserEvent = UsMnt_LockTimerEx();
			} else {
				if( parm & 0x01 ){
					wtype = 1;
				}else{
					wtype = 0;
				}
				usUserEvent = UsMnt_LockTimer(wtype);
			}
			break;

	/* ���ʓ��^���ʊ��� */
		case MNT_SPCAL:
			usUserEvent = UsMnt_SplDay();
			break;
	/* �c�ƊJ�n���� */
		case MNT_OPNHR:
			usUserEvent = UsMnt_BusyTime();
			break;

	/* ������(���g�p) */
		case MNT_TKEXP:
			usUserEvent = UsMnt_TickValid();
			break;
	/* ����L���^����(���g�p) */
		case MNT_INVLD:
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			usUserEvent = UsMnt_PassInv();
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			break;
	/* ������Ɂ^�o��(���g�p) */
		case MNT_ENTRD:
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			usUserEvent = UsMnt_PassEnter();
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			break;
	/* ������`�F�b�N(���g�p) */
		case MNT_PASCK:
			usUserEvent = UsMnt_PassCheck();
			break;
	/* ��������Z���~�f�[�^(���g�p) */
		case MNT_PSTOP:
			usUserEvent = UsMnt_PassStop();
			break;

	/* �W���p�X���[�h */
		case MNT_PWMOD:
			usUserEvent = UsMnt_PassWord();
			break;
	/* ��Ԋm�F */
		case MNT_STSVIEW:
			usUserEvent = UsMnt_StatusView();
			break;
	/* �P�x���� */
		case MNT_CNTRS:
			usUserEvent = UsColorLCDLumine();
			break;
	/* �u�U�[���ʒ��� */
		case MNT_KEYVL:
			usUserEvent = UsMnt_KeyVolume();
			break;

	/* �W���L���f�[�^(���g�p) */
		case MNT_ATTENDDATA:
			if(OPECTL.Mnt_lev >= 2 && prm_get(COM_PRM, S_PAY, 21, 1, 3)) {	// �Ǘ��҈ȏォ���C���[�_����
				usUserEvent = UsMnt_AttendantValidData();
			}
			else {
				BUZPIPI();
				continue;
			}
			break;
	/* �Ïؔԍ������o��(���g�p) */
		case MNT_PWDKY:
			usUserEvent = UsMnt_PwdKyo();
			break;

	/* �g���@�\ */
		case MNT_EXTEND:
			if( !Is_ExtendMenuMake() ){				// �g���@�\�����i�L���ȋ@�\�Ȃ��j
				BUZPIPI();
				continue;
			}
			usUserEvent = UsMnt_Extendmenu();
			break;
		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return;
		default:
			break;
		}

		if(( OpelogNo )||( OpelogNo2 )||( SetChange )){
			if( OpelogNo ) wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			if( OpelogNo2 ) wopelg( OpelogNo2, 0, 0 );		// ���엚��o�^
			OpelogNo2 = 0;
			if( UserMnt_SysParaUpdateCheck( OpelogNo ) ){
				usUserEvent = parameter_upload_chk();
			}
		}

		if( usUserEvent == MOD_CHG || usUserEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of User Operation
		if( usUserEvent == MOD_CHG ){
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			OPECTL.PasswordLevel = (char)-1;
//			return;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//			mode = op_wait_mnt_close();	// �����e�i���X�I������
//			if( mode == MOD_CHG ){
			{
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
				if( mnt_GetFtpFlag() != FTP_REQ_NONE ){				// FTP�t���O���m�F
					ucReq = 1;	// ���s
					if( 0 == lcdbm_setting_upload_FTP() ){			// FTP�J�n
						// ����
						switch( mnt_GetFtpFlag() ){
							// 1:FTP�v������
							case FTP_REQ_NORMAL:
								ucReq = 2;	// OK(�ċN���s�v)
								break;
							// 2:FTP�v������(�d�f�v)
							case FTP_REQ_WITH_POWEROFF:
								ucReq = 0;	// OK(�ċN���v)
								break;
							default:
								break;
						}
					}
					lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)ucReq );	// �ݒ�A�b�v���[�h�v�����M
					mnt_SetFtpFlag( FTP_REQ_NONE );								// FTP�X�V�t���O�i�X�V�I���j���Z�b�g
				}
// GG124100(S) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
				// ���LCD�ւ̃p�����[�^�A�b�v���[�h���s���Ă��牓�u�Ď��f�[�^�𑗐M����
				// (LCD�ւ̃p�����[�^�A�b�v���[�h���ɉ��u�����e�i���X�v�����͂��̂�h�~���邽��)
				if ( mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE ) {
					// parkingWeb�ڑ�����
					if ( _is_ntnet_remote() ) {
						// �[���Őݒ�p�����[�^�ύX
						rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
						// �[�����Őݒ肪�X�V���ꂽ���߁A�\�񂪓����Ă�����L�����Z�����s��
						remotedl_cancel_setting();
					}
					mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
				}
// GG124100(E) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//				return;
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
			}
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
			op_wait_mnt_close();	// �����e�i���X�I������
			return;
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
			}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if( usUserEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;							// ��������Ԃ�
			OPECTL.init_sts = 0;							// ��������������ԂƂ���
			OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
			OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
			OPECTL.PasswordLevel = (char)-1;
			return;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���v�ݒ�																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_Clock( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*| 			: 2005-07-19 ART:ogura modify													   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

static const unsigned short	CLCK_PNT[5][4] = {
	{ 2, 12 ,1, 2 }, { 2, 18 ,1, 2 }, { 3, 2, 1, 4 }, { 3, 12, 0, 2 }, { 3, 18, 0, 2 } 
};

unsigned short	UsMnt_Clock( void )
{

	ushort			msg = 0;
	short			ndat;
	unsigned short	ret;
	int				ret2;
	short			in_now = -1;
	short			in_d[5];
	short			pos = 0;
	struct	clk_rec	clk_data;
	short			y, m, d;
	ushort			mode = 0;

	TimeAdjustCtrl(1);			// ���������␳��~

	in_d[0] = (short)CLK_REC.hour;
	in_d[1] = (short)CLK_REC.minu;
	in_d[2] = (short)CLK_REC.year;
	in_d[3] = (short) CLK_REC.mont;
	in_d[4] = (short) CLK_REC.date;

	dispclr();

/* Display */
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "�����v���킹���@�@�@�@�@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[0] );			// "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[1] );			// "�@�@�@�@�@�N�@�@���@�@���i�@�j"
	if( TimeAdjustCtrl(0) == 0 ){	// �����s�̏ꍇ
		Fun_Dsp( FUNMSG[45] );												// "�@���@�@���@�Z�b�g       �I�� "
	} else {						// �����\�̏ꍇ
		Fun_Dsp( FUNMSG2[58] );												// "�@���@�@���@�Z�b�g ����  �I�� "
	}

	clk_sub( in_d );
	opedsp( 2, 12, (unsigned short)CLK_REC.hour, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );

	ret2 = 0;

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );
		if( ret2 ){
			if( msg == TIMEOUT6 ){				// ��ѱ�ĂŔ�����
				ret = MOD_EXT;
				continue;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if( (msg == KEY_TEN_F5)||(msg == KEY_MODECHG) ){
			if( (msg == KEY_TEN_F5)||(msg == KEY_MODECHG) ||(msg == LCD_DISCONNECT)){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				;
			}else{
				continue;
			}
		}

		switch( KEY_TEN0to9( msg ) )
		{
			case KEY_TEN_F5:
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
				if (mode != 0) {
					break;
				}
				// ���v�Z�b�g�O�̂݉����\
				if(( in_now != -1 ) && ( clk_test( in_now, (char)pos ) != 0 )){ // ���͒l�͈�����
					BUZPIPI();
					in_now = -1;
					break;
				}
				if( in_now != -1 ) {
					in_d[pos] = in_now;
					in_now = -1;
					if (pos >= 2) {
						// �N�E���E���̈ʒu�ł���Ηj�����X�V����
						ndat = dnrmlzm( (short)in_d[2], (short)in_d[3], (short)in_d[4] );
						grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((ndat + 6) % 7)] ); // �j���\��
					}
				}
				BUZPI();
				// ���]�\���𐳓]�\���ɖ߂�
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_d[pos], CLCK_PNT[pos][3], CLCK_PNT[pos][2], 0, COLOR_BLACK, LCD_BLINK_OFF );
				// ���وʒu�����̈ʒu�ɾ��
				if( msg == KEY_TEN_F2 ){
					pos = ( 4 < pos + 1 ) ? 0 : pos + 1;
				}else{
					pos = ( 0 > pos - 1 ) ? 4 : pos - 1;
				}
				break;
			case KEY_TEN:
				if (mode != 0) {
					break;
				}
				// ���v�Z�b�g�O�̂݉����\
				BUZPI();
				if( in_now == -1 )
				{	// in_now <= ���͒l
					in_now = (short)(msg - KEY_TEN0);
				}else{
					if( pos == 2 )
					{	// �N���͂ł����4���̐��l�ɂ���
						in_now = (short)( in_now % 1000 ) * 10 + (short)(msg - KEY_TEN0);
					}else{
						// �N�ȊO�ł����2���̐��l�ɂ���
						in_now = (short)( in_now % 10 ) * 10 + (short)(msg - KEY_TEN0);
					}
				}
				// ���͒l��\������i���]�\���j
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_now, CLCK_PNT[pos][3], CLCK_PNT[pos][2], 1,
																							COLOR_BLACK, LCD_BLINK_OFF );	/* reverse */
				break;
			case KEY_TEN_CL:
				if (mode != 0) {
					break;
				}
				// ���v�Z�b�g�O�̂݉����\
				BUZPI();
				in_now = -1;
				break;
			case KEY_TEN_F4:
				if (mode != 0) {
					break;
				}
				if( TimeAdjustCtrl(0) == 1 ){	// SNTP�@�\�L��
					// ���v�Z�b�g�O�̂݉����\
					BUZPI();
					ret = (ushort)UsMnt_Clock2();				// �����␳�@�\��
				}
				break;
			case KEY_TEN_F3:
				if (mode != 0) {
					break;
				}
				// ���v�Z�b�g�O�̂݉����\
				if(( in_now != -1 ) && ( clk_test( in_now, (char)pos ) != 0 )){ // ���͒l�͈�����
					BUZPIPI();
					in_now = -1;
					break;
				}
				if (in_now == -1){
					in_now = in_d[pos];
				}
				y = (pos==2) ? in_now : in_d[2];
				m = (pos==3) ? in_now : in_d[3];
				d = (pos==4) ? in_now : in_d[4];
				if( chkdate( y, m, d ) != 0 ){ // ���݂�����t������
					BUZPIPI();
					in_now = -1;
					break;
				}
				in_d[pos] = in_now;
				BUZPI();
				clk_data.year = in_d[2];
				clk_data.mont = (char)in_d[3];
				clk_data.date = (char)in_d[4];
				clk_data.hour = (char)in_d[0];
				clk_data.minu = (char)in_d[1];
				clk_data.seco = 0;
				clk_data.ndat = dnrmlzm( (short)clk_data.year, (short)clk_data.mont, (short)clk_data.date );
				clk_data.nmin = tnrmlz ( (short)0, (short)0, (short)clk_data.hour, (short)clk_data.minu );
				clk_data.week = (unsigned char)((clk_data.ndat + 6) % 7);

				if(prm_get(0, S_NTN, 38, 1, 1) == 1){	//���ѕύX���M�ݒ�(���v)���P�̎�
					f_NTNET_RCV_MNTTIMCHG = 1;		// ����ݽ���ݎ����ύX
				}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_Rec.edy_status.BIT.MNTTIMCHG = 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				Ope_clk_set( &clk_data, OPLOG_TOKEISET2 );			// ���v�X�V
				in_d[0] = (short)clk_data.hour;
				in_d[1] = (short)clk_data.minu;
				in_d[2] = (short)clk_data.year;
				in_d[3] = (short)clk_data.mont;
				in_d[4] = (short)clk_data.date;
				clk_sub( in_d );
				grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((clk_data.ndat + 6) % 7)] ); // �j���\��

				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[19] );	// "�@�@ << �Z�b�g���܂��� >> �@�@"
				Lagtim( OPETCBNO, 6, 50*3 );
				ret2 = -1;
				// �I���L�[�ȊO�������s�\�ɂ���
				mode = 1;
				Fun_Dsp( FUNMSG[8] );											// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				break;
		}
		if (mode == 0) {
			// ���وʒu�̐��l�𔽓]�\��������
			if(( msg == KEY_TEN_F1 ) || ( msg == KEY_TEN_F2 ) || ( msg == KEY_TEN_F3 ) || ( msg == KEY_TEN_CL )){
				opedsp( CLCK_PNT[pos][0], CLCK_PNT[pos][1], (unsigned short)in_d[pos], CLCK_PNT[pos][3], CLCK_PNT[pos][2], 1,
																							COLOR_BLACK, LCD_BLINK_OFF );
			}
		}
	}
	Lagcan( OPETCBNO, 6 );
	TimeAdjustCtrl(2);			// ���������␳��~����
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| NTP�T�[�o�[�Ɏ����v�����o���A�V�X�e�����v��␳����											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_Clock2( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: MATSUSHITA																	   |*/
/*| Date		: 2013-02-13																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
unsigned short	UsMnt_Clock2( void )
{
	ushort	msg;
	struct	clk_rec		clk_data;
	date_time_rec2		nettime;
	short	in_d[6];
	ushort	ret, mode;
	ushort	net_msec;
	ulong	margin[3];		// [0]=day part, [1]=msec part, [2]=sign
	int		ans;


	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "�����v���킹���@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[157] );		// "   �Z���^�[�ɐڑ����Ă��܂�   "
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[114] );		// "�@�@ ���΂炭���҂������� �@�@"
	Fun_Dsp( FUNMSG[0] );

	ReqServerTime(SNTPMSG_RECV_TIME);		// �T�[�o�[�Ɏ����v�����o��
	mode = 0;								// SNTP����̉����҂���Ԃɂ���
// sntp����
	for( ret = 0; ret == 0; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// �␳���s�t���O�𗎂Ƃ�
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// �␳���s�t���O�𗎂Ƃ�
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:
		// �I���L�[
			if (mode != 0) {
				BUZPI();
				ret = MOD_EXT;
			}
			break;
		case SNTPMSG_RECV_TIME:
			if (mode == 0) {
				Lagcan(OPETCBNO, _TIMERNO_SNTP);

			// �T�[�o�[�����擾
				ans = sntpGetTime(&nettime, &net_msec);
				if (ans < 0) {
				// �T�[�o�[���Ԃ̎擾�Ɏ��s�����ꍇ�̓^�C���A�E�g�I���Ɠ��������ɂ���
					Lagtim(OPETCBNO, _TIMERNO_SNTP, 1);
					break;
				}

				if (CLK_REC.nmin == (23*60+59) && CLK_REC.seco >= 50) {
				// �V�X�e�����v��23:59:50�`59�̊Ԃł͎����␳���s�킸�A�^�C���A�E�g�I���Ɠ��������ɂ���
					RegistReviseLog(SNTP_GUARD_TIME);		// ���O�o�^
					Lagtim(OPETCBNO, _TIMERNO_SNTP, 1);
					break;
				}

				err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 0, 0, 0, 0);	// E0083����
				wmonlg(OPMON_TIME_AUTO_REVISE, 0, 0);					// ���j�^���O�o�^

				CheckTimeMargin(&nettime, net_msec, margin);
				if (ans == 1) {
				// �����ɒx������
					RegistReviseLog(SNTP_DELAYED_RETRY);	// ���O�o�^
					ReqServerTime(SNTPMSG_RECV_TIME);
					break;
				}
				net_msec = AddTimeMargin(margin, in_d);

			// �蓮���v�␳�ł͌덷�ɂ�����炸�����␳���s��
				BUZPI();
				if (net_msec) {
					// 1ms�P�ʂ̃^�C�}���Z�b�g
					Lagtim_1ms( OPETCBNO, 1, net_msec );
					break;
				}
			}
			// no break
		case TIMEOUT1_1MS:
			if (mode == 0) {
				RegistReviseLog(SNTP_NORMAL_REVISE);

				clk_data.year = in_d[2];
				clk_data.mont = (char)in_d[3];
				clk_data.date = (char)in_d[4];
				clk_data.hour = (char)in_d[0];
				clk_data.minu = (char)in_d[1];
				clk_data.seco = (char)in_d[5];
				clk_data.ndat = dnrmlzm( (short)clk_data.year, (short)clk_data.mont, (short)clk_data.date );
				clk_data.nmin = tnrmlz ( (short)0, (short)0, (short)clk_data.hour, (short)clk_data.minu );
				clk_data.week = (unsigned char)((clk_data.ndat + 6) % 7);
				if(prm_get(0, S_NTN, 38, 1, 1) == 1){		// ���ѕύX���M�ݒ�(���v)���P�̎�
					f_NTNET_RCV_MNTTIMCHG = 1;				// ����ݽ���ݎ����ύX
				}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_Rec.edy_status.BIT.MNTTIMCHG = 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

				mode = 1;									// SNTP������M��̏�Ԃɂ���
				Lagtim( OPETCBNO, 6, 50*3 );				// �����Z�b�g��3�b�o�߂ŏ�ʃ��j���[�ɖ߂�^�C�}�[
				Ope_clk_set( &clk_data, OPLOG_SET_TIME2_SNTP );		// ���v�X�V

				dispclr();
				grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "�����v���킹���@�@�@�@�@�@�@�@"
				grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[0] );			// "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@"
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BASDAT[1] );			// "�@�@�@�@�@�N�@�@���@�@���i�@�j"

				in_d[0] = (short)clk_data.hour;
				in_d[1] = (short)clk_data.minu;
				in_d[2] = (short)clk_data.year;
				in_d[3] = (short)clk_data.mont;
				in_d[4] = (short)clk_data.date;
				in_d[5] = (short)clk_data.seco;
				clk_sub( in_d );						// �V�X�e�����v�̎�����\��

// MH810100(S) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X�i#3960 SNTP�ł̎��v�Z�b�g��A�������f����Ȃ��j
//				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[19] );		// "�@�@ << �Z�b�g���܂��� >> �@�@"
				grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR1[45] );		// "�@<<���b�o�ߌ�Ɋ������܂��@>>"
// MH810100(E) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X�i#3960 SNTP�ł̎��v�Z�b�g��A�������f����Ȃ��j
				Fun_Dsp( FUNMSG[8] );													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			}
			break;
		case TIMEOUT_SNTP:
		// SNTP�^�C���A�E�g
			if (mode == 0) {
				RegistReviseLog(SNTP_TIMEOUT);			// ���O�o�^
				RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// ���s �␳���s�t���O�𗎂Ƃ�

			// �蓮�̏ꍇ�͍ėv�����Ȃ�
				BUZPIPI();
				err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 1, 0, 0, 0);		// E0083����
				dispclr();
				grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[1] );			// "�����v���킹���@�@�@�@�@�@�@�@"
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[158] );		// "   �Z���^�[�ɐڑ��ł��܂���   "
				grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[159] );		// "   �����̎擾�Ɏ��s���܂���   "
				Fun_Dsp( FUNMSG[8] );													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				mode = 2;
			}
			break;
		case TIMEOUT6:
		// �����Z�b�g��3�b�o�߂������̃^�C�}�[
			ret = MOD_EXT;
			break;
		default:
			break;
		}
	}
	Lagcan(OPETCBNO, _TIMERNO_SNTP);
	Lagcan_1ms(OPETCBNO, 1);
	if (mode == 1) {
		TimeAdjustCtrl(3);					// ��~���������Z�b�g�v��
	}
	else {
		TimeAdjustCtrl(2);					// ��~�����v��
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���v�̍X�V����                                                        |*/
/*[]----------------------------------------------------------------------[]*/
void	Ope_clk_set( struct clk_rec *p_clk_data, ushort OpeMonCode )
{
	struct	clk_rec wk_CLK_REC_old;			// ���v�X�V�O�̎���
	ushort	wk_CLK_REC_msec_old;			//					�~���b
	ulong	wkul,wkul2;
	unsigned char clkstr[11];
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;

	c_Now_CLK_REC_ms_Read( &wk_CLK_REC_old, &wk_CLK_REC_msec_old );		// ���v�X�V�O�̎���get

	timset( p_clk_data );									// ���v�X�V

	// ���v�X�V�����|��OPE�^�X�N�֒ʒm
	// ���vCPU����ʒm���ꂽ�̂Ɠ���I/F�Œʒm����
	clkstr[0] = binbcd( (unsigned char)( p_clk_data->year / 100 ));
	clkstr[1] = binbcd( (unsigned char)( p_clk_data->year % 100 ));
	clkstr[2] = binbcd( p_clk_data->mont );
	clkstr[3] = binbcd( p_clk_data->date );
	clkstr[4] = binbcd( p_clk_data->hour );
	clkstr[5] = binbcd( p_clk_data->minu );
	clkstr[6] = binbcd( p_clk_data->seco );

	u_LifeTime.ul = LifeTim2msGet();
	clkstr[7] = u_LifeTime.uc[0];
	clkstr[8] = u_LifeTime.uc[1];
	clkstr[9] = u_LifeTime.uc[2];
	clkstr[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// ���������f�[�^���M�v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	queset( OPETCBNO, CLOCK_CHG, 11, clkstr ); // Send message to opration task
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	if (isEJA_USE()) {
		// �d�q�W���[�i���֎����ݒ�f�[�^�𑗐M����
		OPECTL.EJAClkSetReq = 1;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

	// ���v�X�VLOG�o�^
	wkul = c_Normalize_sec( &wk_CLK_REC_old );				// �ύX�O����
	wkul2 = c_Normalize_sec( p_clk_data );					// �ύX�㎞��
	wopelg( OpeMonCode, wkul, wkul2 );						// ����LOG�o�^(FT-4800)
// MH810100(S) Y.Yamauchi 2020/02/26 �Ԕԃ`�P�b�g���X�i���v���킹�j
	if( OpeMonCode == OPLOG_SET_TIME2_SNTP ){	// SNTP �������v�X�V�@�\�ɂ�鎞�v�Z�b�g�A���v���킹�i�Z���^�[�j
		PKTcmd_clock( 1 );				// LCD�Ɏ��v���M�i�Z���^�[�j
	} else {							// ���v���킹�i�蓮�j�ANTNET�i���v�f�[�^�F119�j
		PKTcmd_clock( 0 );				// LCD�Ɏ��v���M(�蓮)
	}
// MH810100(E) Y.Yamauchi 2020/02/26 �Ԕԃ`�P�b�g���X�i���v���킹�j
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���v�f�[�^�̃`�F�b�N                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| RETURN VALUE : short	ret;    0 : OK  -1 : NG                        |*/
/*[]----------------------------------------------------------------------[]*/
char clk_test( short data, char pos )
{
	switch( pos ){
		case 0:		//hour
			if( data >= 24 ) {
				return( (char)-1 );
			}
			break;
		case 1:		//minit
			if( data >= 60 ) {
				return( (char)-1 );
			}
			break;
		case 2:		//year
			if(( data < 1990 ) || ( data > 2050 )){
				return( (char)-1 );
			}
			break;
		case 3:		//month
			if(( data > 12 ) || ( data < 1 )){
				return( (char)-1 );
			}
			break;
		case 4:		//date
			if(( data > 31 ) || ( data < 1 )){
				return( (char)-1 );
			}
			break;
		case 5:		//week
			break;
		default:
			return( (char)-1 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���v�f�[�^�̕\��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
static void clk_sub( short * dat )
{
	ushort ndat;

	opedsp( 2, 12, (unsigned short)dat[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 18, (unsigned short)dat[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	opedsp( 3, 2, (unsigned short) dat[2], 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	//not 0sup
	opedsp( 3, 12, (unsigned short) dat[3], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 18, (unsigned short) dat[4], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );

	ndat = dnrmlzm( (short)dat[2], (short)dat[3], (short)dat[4] );
	grachr( 3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[(unsigned char)((ndat + 6) % 7)] );

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| ������J��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_shtctl( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	SHTMOD_CNT	2

unsigned short	UsMnt_shtctl( void )
{
	ushort	msg;
	long	*pSHT;
	const uchar	*str_tbl[] = {DAT2_2[1], DAT2_2[0]};
	char	pos;

	pSHT = &PPrmSS[S_P01][1];
	pos = (*pSHT == 0) ? 0 : 1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[33] );					/* "���V���b�^�[�J�����@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "�@���݂̏�ԁ@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@" */
	grachr( 2, 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "�@�@�@�@�@�@�@�w�w�@�@�@�@�@�@" */
	grachr( 4, 14,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "�@�@�@�@�@�@�@�����@�@�@�@�@�@" */
	grachr( 5, 14,  4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "�@�@�@�@�@�@�@�J���@�@�@�@�@�@" */
	Fun_Dsp( FUNMSG[20] );								/* "�@���@�@���@�@�@�@ ����  �I�� " */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				return( MOD_EXT );
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
				BUZPI();
				grachr((ushort)(4+pos), 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				pos ^= 1;
				grachr((ushort)(4+pos), 14,  4, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				break;
			case KEY_TEN_F4:					/* F4:���� */
				BUZPI();
				*pSHT = (long)pos;
				OpelogNo = OPLOG_SHUTTERKAIHO;		// ���엚��o�^
				grachr( 2, 14,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �c�x�Ɛ؊�                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_OCsw( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	OCMOD_CNT	3

unsigned short	UsMnt_OCsw( void )
{
	ushort	msg;
	long	*pOPCL;
	long	val_tbl[] = {1, 0, 2};
	const uchar	*str_tbl[] = {DAT4_2[12], DAT4_2[13], DAT4_2[14]};
	int		pos;

	pOPCL = &PPrmSS[S_P01][2];
	for (pos = 0; pos < 2; pos++) {
		if (*pOPCL == val_tbl[pos])
			break;
	}
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[11] );					/* "���c�x�Ɛؑց��@�@�@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "�@���݂̏�ԁ@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@" */
	grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "�@�@�@�@�@�@�@�w�w�w�w�@�@�@�@" */
	grachr( 4, 12,  8, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "�@�@�@�@�@�@�@�����c�Ɓ@�@�@�@" */
	grachr( 5, 12,  8, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "�@�@�@�@�@�@�@�@�����@�@�@�@�@" */
	grachr( 6, 12,  8, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[2] );	/* "�@�@�@�@�@�@�@�����x�Ɓ@�@�@�@" */
	Fun_Dsp( FUNMSG[20] );								/* "�@���@�@���@�@�@�@ ����  �I�� " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:					/* F5:Exit */
			BUZPI();
			return( MOD_EXT );
		case KEY_TEN_F1:					/* F1: */
		case KEY_TEN_F2:					/* F2: */
			BUZPI();
			grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 2;
			}
			else {
				if (++pos > 2)
					pos = 0;
			}
			grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			break;
		case KEY_TEN_F4:					/* F4:���� */
			BUZPI();
			*pOPCL = val_tbl[pos];
			OpelogNo = OPLOG_EIKYUGYOKIRIKAE;		// ���엚��o�^
			grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
// MH810100(S) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Total                                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Total( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : hashimo                                                 |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	DispMax_Syu	6
#define	UsMnt_Total_Wait_To	50*3 //3S
unsigned short	UsMnt_Total( short syu_kind )
{
static	const	ushort	menu_reverse[3] = { 12, 12, 16 };
static	const	ushort	pri_cmd[][3] = {
		{PREQ_TSYOUKEI, PREQ_TGOUKEI, PREQ_TGOUKEI_Z}		// T
	,	{PREQ_GTSYOUKEI, PREQ_GTGOUKEI, PREQ_GTGOUKEI_Z}	// GT
	,	{PREQ_MTSYOUKEI, PREQ_MTGOUKEI, PREQ_MTGOUKEI_Z}	// MT
};
static	const	ushort	ope_log[][3] = {
		{OPLOG_T_SHOKEI, OPLOG_T_GOKEI, OPLOG_T_ZENGOKEI}
	,	{OPLOG_GT_SHOKEI, OPLOG_GT_GOKEI, OPLOG_GT_ZENGOKEI}
	,	{OPLOG_MT_SHOKEI, OPLOG_MT_GOKEI, OPLOG_MT_ZENGOKEI}
};
	short			msg;
	unsigned char	pos[2], menu_su;
	uchar			pri_kind;		// �󎚐���������

	T_FrmSyuukei	FrmSyuukei;
	T_FrmPrnStop	FrmPrnStop;
	int				inji_end;
	int				can_req;
	uchar			fg_Ttotal;		//�s�W�v�́u���v�v�����g�v�I������t���O
	struct SKY		*p_sky;
	SYUKEI			*p_syukei;
	date_time_rec	*p_sky_date;	// �W�v�����ް��߲���
	ushort			pri_req;
	ushort			oplg_no;
	uchar			mnt_sw = 0;		// �󎚒��Ƀ����e�i���XOFF�ɂȂ������ǂ���
	uchar			R_END;			// �󎚐���I���t���O
	uchar			J_END;
	uchar			LogCount;		// ���O�o�^����
	uchar			maxPage;		// �ő�y�[�W��
	uchar			nowPage;		// ���ݕ\���y�[�W
	uchar			nowselect;		// ���݂̃J�[�\���ʒu�l
	uchar			syu_elog;		// T/GT�W�v��eLog�l��ێ�

// initialize ---------------
	pos[0] = 0;
	pos[1] = 0;

TOTALTOP:
// menu loop ---------------
	Lagcan( OPETCBNO, 6 );
	dispclr();
	pri_kind = 0;
	R_END = 0;
	J_END = 0;
	fg_Ttotal = 0;			//�s�W�v�́u���v�v�����g�v�I������t���O

	switch(syu_kind) {
	default:
//	case	MNT_TTOTL:
		menu_su = 0;
		break;
	case	MNT_GTOTL:
		menu_su = 1;
		break;
	case	MNT_MTOTL:
		menu_su = 19;
		break;
	}
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[menu_su] );	// �^�C�g���s
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[2] );			// [02]	"�@���v�v�����g�@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[3] );			// [03]	"�@���v�v�����g�@�@�@�@�@�@�@�@"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[4] );			// [04]	"�@�O�񍇌v�v�����g�@�@�@�@�@�@"
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[5] );			// [05]	"�@���s�F�v�����g���J�n���܂��@"
	Fun_Dsp( FUNMSG[68] );													// [XX]	"�@���@�@���@�@�@�@ ���s  �I�� "

	grachr( (unsigned short)(pos[1]+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[1]+2]);
	grachr( (unsigned short)(pos[0]+2), 2, menu_reverse[pos[0]], 1, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[0]+2]+2);

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return( MOD_CHG );

			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return( MOD_EXT );

			case KEY_TEN_F1:						/* F1: */
				BUZPI();
				pos[0] = (unsigned char)(( 0 == pos[0] ) ? ( 2 ) : ( pos[0] - 1 ));
				break;

			case KEY_TEN_F2:						/* F2: */
				BUZPI();
				pos[0] = (unsigned char)((2 == pos[0] ) ? 0 : ( pos[0] + 1 ));
				break;

			case TIMEOUT6:							// �^�C���A�E�g
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[21] );// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
				break;

			case KEY_TEN_F4:						/* F5: ���s */
				if( OPECTL.Mnt_lev < 2 ){								// �W���Ń��O�C�����H
					if(	(syu_kind == MNT_TTOTL && pos[0] == 1) ||		//  T���v�����{����H
						(syu_kind == MNT_GTOTL && pos[0] == 1) ||		// GT���v�����{����H
						(syu_kind == MNT_MTOTL && pos[0] == 1)){		// MT���v�����{����H
						BUZPIPI();										// �G���[��
						break;											// ���s�����Ȃ��̂Ŕ�����
					}
				}
			// decide source data
				if (CheckPrinter(pri_kind) == 0 ||
					auto_syu_prn == 2) {
				// �v�����^�ُ�
				// �����W�v��
					BUZPIPI();
					break;
				}
				p_sky = (pos[0] == 2)? &skybk : & sky;
				switch(syu_kind) {
				default:
			//	case	MNT_TTOTL:
					p_syukei = &p_sky->tsyuk;
					pri_req = pri_cmd[0][pos[0]];
					oplg_no = ope_log[0][pos[0]];
					syu_elog = eLOG_TTOTAL;
					break;
				case	MNT_GTOTL:
					p_syukei = &p_sky->gsyuk;
					pri_req = pri_cmd[1][pos[0]];
					oplg_no = ope_log[1][pos[0]];                           //���샍�O�o�^�o0=T�W�v�A1=GT�W�v�A�Q=MT�W�v�p
					syu_elog = eLOG_GTTOTAL;
					break;
				case	MNT_MTOTL:
					p_syukei = &p_sky->msyuk;
					pri_req = pri_cmd[2][pos[0]];
					oplg_no = ope_log[2][pos[0]];
					break;
				}
			// check condtion
				if (pos[0] == 1) {
				// ���v�H
					if (CountCheck() == 0) {
					// ���ɏW�v��
						grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " �������F���΂炭���҂������� "
						Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
						BUZPIPI();
						break;
					}
					if (pri_req == PREQ_TGOUKEI) {
					// �t���b�V���������ݒ���T�W�v�s�Ƃ���(�޸�No.157)
						if (AppServ_IsLogFlashWriting(eLOG_TTOTAL) != 0) {	// Flash�ւ̏����ݒ��H
							grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " �������F���΂炭���҂������� "
							Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
							BUZPIPI();					// �������ݒ��Ȃ̂ŃG���[��
							break;						// ������
						}
					}
					else if (pri_req == PREQ_GTGOUKEI) {
					// �t���b�V���������ݒ���GT�W�v�s�Ƃ���
						if (AppServ_IsLogFlashWriting(eLOG_GTTOTAL) != 0) {	// Flash�ւ̏����ݒ��H
							grachr( 5, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, TGTSTR[22] );// " �������F���΂炭���҂������� "
							Lagtim( OPETCBNO, 6, UsMnt_Total_Wait_To );
							BUZPIPI();					// �������ݒ��Ȃ̂ŃG���[��
							break;						// ������
						}
					}
					pri_kind = PrnGoukeiPri(pri_req);
				}
				else {
					if (pos[0] == 2) {
						if( MNT_MTOTL != syu_kind ){	// MT�W�v�̏ꍇ�͏]������
							// �O��W�v�H
							if( 0 == Ope_Log_TotalCountGet(syu_elog)){	// ���O�o�^�����擾
								// �O��W�v�H
								p_sky_date = (date_time_rec*)&p_syukei->NowTime;
								if( (p_sky_date->Year == 0) &&
								    (p_sky_date->Mon  == 0) &&
								    (p_sky_date->Day  == 0) &&
								    (p_sky_date->Hour == 0) &&
									(p_sky_date->Min  == 0) ){
									// �O��W�v�ް����Ȃ��ꍇ
									BUZPIPI();
									break;
								}
							} else {
								BUZPI();
								goto SELECTDATE;	// T/GT�W�v�̏ꍇ�A���t�I����ʂ�
							}
						} else {
							// �O��W�v�H
							p_sky_date = (date_time_rec*)&p_syukei->NowTime;
							if( (p_sky_date->Year == 0) &&
							    (p_sky_date->Mon  == 0) &&
							    (p_sky_date->Day  == 0) &&
							    (p_sky_date->Hour == 0) &&
								(p_sky_date->Min  == 0) ){
								// �O��W�v�ް����Ȃ��ꍇ
								BUZPIPI();
								break;
							}
						}
					}
					pri_kind = R_PRI;
				}
				BUZPI();
				wopelg( oplg_no , 0, 0 );
				Lagcan( OPETCBNO, 6 );
				goto TOTALPRN;		// Go Print!!

			default:
				break;
		}

		if(pos[0] != pos[1])
		{
			grachr( (unsigned short)(pos[1]+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[1]+2]);
			grachr( (unsigned short)(pos[0]+2), 2, menu_reverse[pos[0]], 1, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[pos[0]+2]+2);
			pos[1] = pos[0];
		}

	}

SELECTDATE:
// request to print ----------
	Lagcan( OPETCBNO, 6 );

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[20] );	// ���O�񍇌v�v�����g��
	Fun_Dsp(FUNMSG2[51]);								// "�@���@�@���@�@���@ ���s  �I�� "

	LogCount = Ope_Log_TotalCountGet(syu_elog);		// ���O�o�^�����擾
			
	if( 0 != LogCount % DispMax_Syu ){
		maxPage = (LogCount / DispMax_Syu ) + 1;		// �ő�y�[�W�����Z�b�g
	} else {
		maxPage = (LogCount / DispMax_Syu );			// �ő�y�[�W�����Z�b�g
	}
	nowPage = 1;
	nowselect = 1;
	syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// �f�[�^�ꊇ�\��


	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:				/* F5:�I�� */
			BUZPI();
			goto TOTALTOP;
			break;
			return( MOD_EXT );
		case KEY_TEN_F4:				/* F4:���s */
// MH810105 GG119202(S) T���v�A���󎚑Ή�
//			p_syukei = (SYUKEI*)SysMnt_Work;
			memset(g_TempUse_Buffer, 0, sizeof(g_TempUse_Buffer));
			p_syukei = (SYUKEI*)g_TempUse_Buffer;
// MH810105 GG119202(E) T���v�A���󎚑Ή�
			if( 0 == Ope_Log_1DataGet( syu_elog, (ushort)LogCount - (nowPage - 1)*DispMax_Syu - nowselect , p_syukei ) ){	// �ŌÂ���n�Ԗڂ̃��O���擾
				BUZPIPI();	// �������擾�ł��Ȃ���Δ�����
				break;
			}
			pri_kind = R_PRI;
			BUZPI();
			goto TOTALPRN;		// Go Print!!
			break;
		case KEY_TEN_F3:				/* F3:���x�� */
			BUZPI();
			if( nowPage == maxPage ){
				nowPage = 1;
				nowselect = 1;
			} else {
				nowPage++;
				nowselect = 1;
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// �f�[�^�ꊇ�\��
			break;
		case KEY_TEN_F1:				/* F1:�� */
			BUZPI();
			if (nowselect > 1) {
				nowselect--;			// �I���ʒu��1����
			}else{
				if( nowPage == 1 ){
					nowPage = maxPage;
					nowselect = LogCount - ( maxPage - 1)*DispMax_Syu;
				} else {
					nowPage--;
					nowselect = DispMax_Syu;
				}
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// �f�[�^�ꊇ�\��
			break;
		case KEY_TEN_F2:				/* F2:�� */
			BUZPI();
			if( nowPage == maxPage ){
				if ( nowselect < LogCount - ( maxPage - 1)*DispMax_Syu) {
					nowselect++;
				}else{
					nowPage = 1;
					nowselect = 1;
				}				
			} else {
				if ( nowselect < DispMax_Syu) {
					nowselect++;
				}else{
					nowPage++;
					nowselect = 1;
				}
			}
			syukei_all_dsp( LogCount - (nowPage - 1)*DispMax_Syu -1 , nowselect, syu_elog);	// �f�[�^�ꊇ�\��
			break;
		default:
			break;
		}
	}

TOTALPRN:
// request to print ----------
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[menu_su] );	// �^�C�g���s
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8] );			// [08]	"�@�@�@ �v�����g���ł� �@�@�@�@"
	Fun_Dsp( FUNMSG[82] );													// [51]	"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

	if (pos[0] != 2) {
	// ���v�E���v
		FrmSyuukei.prn_data = (void*)syuukei_prn((int)pri_req, p_syukei);
	}
	else {
		FrmSyuukei.prn_data = (void*)p_syukei;
	}
	if (pos[0] != 1) {													// ���v�ȊO
		// �O��̍��v�Œ��~�����ꍇ�A�ȉ��̃t���O��ON�܂܂ƂȂ�̂�OFF�ɂ���
		rct_goukei_pri = OFF;											// ���v�L�^�󎚏�ԁiڼ�ėp�jؾ��
		jnl_goukei_pri = OFF;											// ���v�L�^�󎚏�ԁi�ެ��ٗp�jؾ��
	}
	FrmSyuukei.prn_kind = pri_kind;
	memcpy( &FrmSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// ���ݎ������
// MH810105 GG119202(S) T���v�A���󎚑Ή�
	FrmSyuukei.print_flag = 0;
	if (isEC_USE() &&
		(pri_req == PREQ_TGOUKEI ||
		 pri_req == PREQ_TGOUKEI_Z)) {
		// ���σ��[�_�ڑ������T���v�󎚂̏ꍇ�͐ݒ���Q�Ƃ��ĘA���󎚂��s��
		ec_linked_total_print(pri_req, &FrmSyuukei);
	}
	else {
// MH810105 GG119202(E) T���v�A���󎚑Ή�
	queset( PRNTCBNO, pri_req, sizeof(T_FrmSyuukei), &FrmSyuukei );
// MH810105 GG119202(S) T���v�A���󎚑Ή�
	}
// MH810105 GG119202(E) T���v�A���󎚑Ή�

	for (inji_end = 0, can_req = 0; inji_end < 10; ){

		//----------------------------------------
		// �󎚏I���҂�
		//----------------------------------------

		msg = StoF( GetMessage(), 1 );

		if( msg == (INNJI_ENDMASK|pri_req) )
		{
			if( rct_goukei_pri != ON){												// ���V�[�g�v�����^���삵�Ă��Ȃ�
				if( OPECTL.Pri_Kind == 1 &&											// ���V�[�g�󎚊���
					PRI_NML_END == check_print_result(0) &&							// ����I��
					PrnGoukeiChk( (ushort)(msg & (~INNJI_ENDMASK))) == YES){		// ���v��
					R_END = 1;
				}
			}
			if( jnl_goukei_pri != ON){												// �W���[�i���v�����^���삵�Ă��Ȃ�
				if( OPECTL.Pri_Kind == 2 &&											// �W���[�i���󎚊���
					PRI_NML_END == check_print_result(1) &&							// ����I��
					PrnGoukeiChk( (ushort)(msg & (~INNJI_ENDMASK))) == YES){		// ���v��
					J_END = 1;
				}
			}
			if ( pos[0] == 1 ) {				// T/GT/MT���v�̈󎚒��H
			// ���v�����^�i�W�v�j���I�����Ă���H
				if (rct_goukei_pri != ON && jnl_goukei_pri != ON) {
					inji_end = 1;				// �󎚏I��
				}
			}
			else {
			// ���̏ꍇ�̓��V�[�g�v�����^�̂�
				inji_end = 1;					// �󎚏I��
			}

			msg &= (~INNJI_ENDMASK);
		}

		switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
			// �h�A�m�u���ǂ����̃`�F�b�N�����{
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			mnt_sw = 1;			// �����eOFF�ێ�
			if (inji_end == 0 || inji_end == 4 ){
				break;			// �󎚎��s���͖���
			}
			inji_end = 10;		// ���[�vend
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			break;

		case KEY_TEN_F4:
			if(inji_end != 3){
				break;
			}
			inji_end = 1;
			// no break
		case KEY_TEN_F3:						/* F3:Stop */
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// �󎚒��~�v��
					can_req = 1;
					if (pri_kind != 0) {
						FrmPrnStop.prn_kind = pri_kind;
						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					}
				}
				if (pri_req == PREQ_TGOUKEI) {
					CountFree(T_TOTAL_COUNT);			// 12-40
					wopelg( OPLOG_CAN_T_SUM , 0, 0 );
				} else if (pri_req == PREQ_GTGOUKEI) {
					CountFree(GT_TOTAL_COUNT);
					wopelg( OPLOG_CAN_GT_SUM , 0 ,0 );	// 12-41
				} else if (pri_req == PREQ_MTGOUKEI) {
					wopelg( OPLOG_CAN_MT_SUM , 0 ,0 );	// 12-42
				}
				
				break;
			}
			else if (inji_end == 1) {
				BUZPI();
				inji_end = 10;

				if (pri_req == PREQ_TGOUKEI) {	// �p�����ł������������ꍇ�A�ǔԂ�ԋp
					// T���v
					CountFree(T_TOTAL_COUNT);
					wopelg( OPLOG_CAN_T_SUM , 0 ,0 );	// 12-40
				} else if (pri_req == PREQ_GTGOUKEI) {
					// GT���v
					CountFree(GT_TOTAL_COUNT);
					wopelg( OPLOG_CAN_GT_SUM , 0 ,0 );	// 12-41
				} else if( pri_req == PREQ_MTGOUKEI) {
					// MT���v
					wopelg( OPLOG_CAN_MT_SUM , 0 ,0 );	// 12-42
				}
			}
			else if (inji_end == 3) {								// �󎚂Ȃ��ŏW�v���͂�
				BUZPI();
				/*	���V�[�g�E�W���[�i�������󎚐ݒ�ŕЕ����؂ꂾ�����ꍇ	*/
				/*	�����Ŏg����v�����^�Ɉ󎚗v�����o��					*/
				if( (pri_kind == 3) && 						// ���V�[�g�E�W���[�i�������Ɉ�
					(R_END != 1 && J_END != 1)){			// �����Ƃ��󎚊������Ă��Ȃ�
					if(CheckPrinter(RJ_PRI)){				// ���v�����^�g�p��
						pri_kind = RJ_PRI;
						goto TOTALPRN;
					}
					else if(CheckPrinter(R_PRI)){			// ���V�[�g�v�����^�g�p��
						pri_kind = R_PRI;
						goto TOTALPRN;
					}
					else if( CheckPrinter(J_PRI) ){			// �W���[�i���v�����^�g�p��
						pri_kind = J_PRI;
						goto TOTALPRN;
					}
				}
				/*	�󎚂��Ȃ������ꍇ�A�W�v�d�����M�Ƹر���s�� 		*/
				/*	���̎��_�ň󎚊����t�F�[�Y�Ƃ���					*/
				switch( syu_kind ){
					case MNT_TTOTL:									// �s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						if( Check_syuukei_clr( 0 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						ac_flg.syusyu = 11;
						Make_Log_TGOUKEI();				// �Ԏ����W�v���܂߂�T���v۸ލ쐬
						CountGet( T_TOTAL_COUNT, &p_syukei->Oiban );		// T���v�ǔ�
						syuukei_clr( 0 );							// �s�W�v�ر�X�V����
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// �o�b�`���M�ݒ�L
							if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//						if (prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1) {
//							// �蓮�u�s���v�v�����g�v��̃Z���^�[�ʐM�����i���߁j
//							auto_centercomm( 1 );
//							if( edy_cnt_ctrl.BIT.exec_status ){
//								inji_end = 4;						// Edy�Z���^�[�ʐM���{��
//								continue;
//							}
//						}						
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						break;
					case MNT_GTOTL:									// �f�s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						if( Check_syuukei_clr( 1 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						ac_flg.syusyu = 22;
						CountGet( GT_TOTAL_COUNT, &p_syukei->Oiban );		// GT���v�ǔ�
						syuukei_clr( 1 );							// �f�s�W�v�ر�X�V����
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// �o�b�`���M�ݒ�L(T���v�Ɠ����j
							if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						break;
					case MNT_MTOTL:									// �l�s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						if( Check_syuukei_clr( 2 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						ac_flg.syusyu = 102;
						syuukei_clr( 2 );							// �l�s�W�v�ر�X�V����
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						break;
				}
				inji_end = 10;
			}
			break;
			case KEY_TEN_F5:						/* F5:Exit */
				if( (inji_end != 0) && (inji_end != 3) && (inji_end != 4) ){
					BUZPI();
					goto TOTALTOP;
				}
			break;

		case PREQ_TSYOUKEI:  	// �s���v
		case PREQ_GTSYOUKEI: 	// �f�s���v
		case PREQ_TGOUKEI:  	// �s���v
		case PREQ_GTGOUKEI: 	// �f�s���v
		case PREQ_TGOUKEI_Z:	// �O��s
		case PREQ_GTGOUKEI_Z: 	// �O��f�s
		case PREQ_MTSYOUKEI: 	// �l�s���v
		case PREQ_MTGOUKEI: 	// �l�s���v
		case PREQ_MTGOUKEI_Z: 	// �O��l�s
			if( inji_end == 1 || (inji_end == 0 && OPECTL.Pri_Result == PRI_ERR_END))
			{
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
//				if (mnt_sw == 1) {
//					goto TOTALTOP;
//				}
				// �h�A�m�u��Ԃ̏ꍇ�̓����e�i���X�I��
				if (mnt_sw == 1 || CP_MODECHG) {
					return MOD_CHG;
				}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				switch( OPECTL.Pri_Result ){	// �󎚌��ʁH
					case PRI_NML_END:
					case PRI_CSL_END:
						if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){
							inji_end = 10;
						// Edy���[�U�����e�i�s�W�v�u���v�v�����g�v�I����̒��ߏ������{�j
							if((syu_kind == MNT_TTOTL) && (pos[0] == 1) && (fg_Ttotal == 0)) {
							//�s���v�v�����g������I����P��̂ݎ��{����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//								if (prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1) {
//								// �蓮�u�s���v�v�����g�v��̃Z���^�[�ʐM�����i���߁j
//									auto_centercomm( 1 );
//									if( edy_cnt_ctrl.BIT.exec_status ){
//										inji_end = 4;						// Edy�Z���^�[�ʐM���{��
//									}
//								}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
								fg_Ttotal = 1;		// ���ߎ��s�ς�
							}
						}
						break;
					case PRI_ERR_END:
// MH810100(S) Y.Yamauchi 2020/02/17 #3856 ���؂��G���[�������Ɂu�v�����g���ł��v�ƕ\�������
						displclr(3);
// MH810100(E) Y.Yamauchi 2020/02/17 #3856 ���؂��G���[�������Ɂu�v�����g���ł��v�ƕ\�������
						if ((ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0) ||	// ڼ�Ď��؂�
							(ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0)){	// �ެ��َ��؂�
							grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[10] );		//"�@�󎚂ł��܂���i���؂�j�@�@"
						}
						else{
							grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[9] );		//"�@�󎚂ł��܂���i�G���[�j�@�@"
						}
						if((msg == PREQ_TGOUKEI) || (msg == PREQ_GTGOUKEI) || (msg == PREQ_MTGOUKEI) ){	// ���v��
							grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[15] );		//"�@�󎚂Ȃ��ŏW�v���s���܂����H"
							grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[17] );		//"�������̎��͖��������������"
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[18] );		//"�ēx�W�v�������s���ĉ������B�@"
							Fun_Dsp( FUNMSG[19] );												//"�@�@�@�@�@�@ �͂� �������@�@�@"
							inji_end = 3;
						}
						else{																	// ���v�󎚈ȊO
							Fun_Dsp( FUNMSG[8] );												// [08]	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
						}
						
					default:
						break;
				}
			}
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case IBK_EDY_RCV:
//			if( inji_end == 4 && !edy_cnt_ctrl.BIT.exec_status ){
//				inji_end = 10;
//			}
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		default:
			break;
		}
	}

	if (mnt_sw == 1) {
	// �󎚒��Ƀ����e�i���XOFF���ꂽ(�󎚏I���őҋ@��ʂ�)
		return(MOD_CHG);
	}
	goto TOTALTOP;		// ���j���[�֖߂�
}

/*[]-----------------------------------------------------------------------[]*/
/*|  �W�v�󎚗���\��(5���܂�)												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : receipt_all_dsp( topNo, rev_no, elog )					|*/
/*| PARAMETER    : uchar topNo		: �擪LogNo.							|*/
/*|              : uchar rev_no		: ���]�ԍ�								|*/
/*|              : uchar elog		: T/GT�W�v�̏��						|*/
/*| RETURN VALUE : void														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]��ܰ��ر�Ƃ��Ďg�p����B									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	syukei_all_dsp(uchar topNo, uchar rev_no, uchar elog )
{
// MH810105 GG119202(S) T���v�A���󎚑Ή�
//	SYUKEI	*syukei = (SYUKEI*)SysMnt_Work;
	SYUKEI	*syukei = (SYUKEI*)g_TempUse_Buffer;
// MH810105 GG119202(E) T���v�A���󎚑Ή�
	uchar	i;
	rev_no = rev_no - 1;	// �␳
	
	for (i = 0; i < DispMax_Syu; i++,topNo--) {
		if( 1 == Ope_Log_1DataGet( elog, (ushort)topNo, syukei ) ){	// �ŌÂ���n�Ԗڂ̃��O���擾
			grachr( 1+i,  1, 18, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF, &LOGSTR2[1][12] );	// "�@�@�N�@�@���@�@��"
			grachr( 1+i, 23,  2, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		// "�F"
			opedsp( 1+i,  1, (ushort)syukei->NowTime.Year, 2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// �N
			opedsp( 1+i,  7, (ushort)syukei->NowTime.Mon,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// ��
			opedsp( 1+i, 13, (ushort)syukei->NowTime.Day,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// ��
			opedsp( 1+i, 19, (ushort)syukei->NowTime.Hour, 2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// ��
			opedsp( 1+i, 25, (ushort)syukei->NowTime.Min,  2, 1, (rev_no == i), COLOR_BLACK, LCD_BLINK_OFF);	// ��
			if( topNo == 0 ){
				i++;
				break;
			}
		} else {
			break;
		}
	}
	for ( ; i < DispMax_Syu; i++) {
		grachr(1+i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Money Control                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Mnctl( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#define	COINCHK_CNT	4	//�\�~�A�ۗ��͕ʉ��

unsigned short	UsMnt_Mnctlmenu( void ){

	unsigned short	usUmnyEvent;
	char	wk[2];
	char	org[2];
	T_FrmTuriKan	FrmTuriKan;
	ushort			msg;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[0] );		// [00]	"���ޑK�Ǘ����@�@�@�@�@�@�@�@�@" */

		if( CPrmSS[S_KAN][1] == 0 ){
			// ���K�Ǘ�Ӱ�ސݒ聁���Ȃ�
			usUmnyEvent = Menu_Slt( MNYMENU, USM_MNC_TBL1, (char)USM_MNC_MAX1, (char)1 );
		}
		else{
			// ���K�Ǘ�Ӱ�ސݒ聁����
			usUmnyEvent = Menu_Slt( MNYMENU, USM_MNC_TBL2, (char)USM_MNC_MAX2, (char)1 );
		}
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){

			case MNT_MNCNT:
				if( CPrmSS[S_KAN][1] ){
					usUmnyEvent = UsMnt_mnyctl();
				}else{
					usUmnyEvent = UsMnt_mnyctl2();
				}
				break;
			case MNT_IVTRY:
				usUmnyEvent = UsMnt_invtry();
				break;
			case MNT_MNYBOX:
				usUmnyEvent = UsMnt_mnybox();
				break;
			case MNT_MNYCHG:
				if( CPrmSS[S_KAN][1] ){
					usUmnyEvent = UsMnt_mnychg();
				}else{
					BUZPIPI();
				}
				break;
			case MNT_MNYPRI:	// �ޑK�Ǘ����v�v�����g
				turikan_gen();
				FrmTuriKan.prn_kind = R_PRI;
				FrmTuriKan.prn_data = &turi_kan;
				turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];							// �@�B��
				turi_kan.Kakari_no = OPECTL.Kakari_Num;									// �W���ԍ�set
				memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );			// ���ݎ���
				turikan_subtube_set();
				queset( PRNTCBNO, PREQ_TURIKAN_S, sizeof(T_FrmTuriKan), &FrmTuriKan  );	// �ޑK�Ǘ��i���v�j�󎚗v��
				OpelogNo = OPLOG_TURIKAN_SHOKEI;										// ���엚��o�^

				// �v�����g�I����҂����킹��
				Lagtim( OPETCBNO, 6, 5*50 );	// �󎚗v���ް����ϰ(�T�b)�N��
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					// �v�����g�I��
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					// �^�C���A�E�g���o
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					// ���[�h�`�F���W
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					if (msg == KEY_MODECHG) {
					if (msg == KEY_MODECHG || msg == LCD_DISCONNECT) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
						// �h�A�m�u���ǂ����̃`�F�b�N�����{
						if (CP_MODECHG && msg == KEY_MODECHG) {
							break;
						}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						BUZPI();
						Lagcan(OPETCBNO, 6);

						OPECTL.Mnt_mod = 0;
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						OPECTL.Mnt_lev = (char)-1;
						OPECTL.PasswordLevel = (char)-1;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						return(MOD_CHG);
						if( msg == KEY_MODECHG ){
							return(MOD_CHG);
						} else {
							return(MOD_CUT);
						}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}
					// �e�T�i�I���j�L�[
					if (msg == KEY_TEN_F5) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						DP_CP[0] = org[0];
						DP_CP[1] = org[1];
						return( MOD_EXT );
					}
				}
				break;
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( MOD_EXT );
				break;

			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
			UserMnt_SysParaUpdateCheck( OpelogNo );
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usUmnyEvent == MOD_CHG ){
		if( usUmnyEvent == MOD_CHG ||usUmnyEvent == MOD_CUT ){	
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			return usUmnyEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�ޑK��[���O�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			1=��[��
//[]----------------------------------------------------------------------[]
///	@author			
///	@date			Create	: 
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		UsMnt_mnyctl_chk( void )
{
	if ((CN_RDAT.r_dat0c[1] & 0x40) != 0						// �R�C�����b�N�J�Z�b�g�E
		|| turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT 
		|| ((int)err_cnm_chk() < 0)
	) {
		return 0;
	}
	return 1;
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| �ޑK��[
//|   �\�~���Ƃ���SUB�`���[�u�A�⏕�`���[�u���ݒ�\�B
//|   SUB�`���[�u�́A10�~�A100�~�A���ݒ�\�B
//|   �⏕�`���[�u�́A10�~�A50�~�A100�~�A���ݒ�\�B
//|   SUB�ƕ⏕�𓯂�����ɐݒ肷�邱�Ƃ��\�B
//|   �\�~���ݒ�Ȃ����\�B
//|     ���ݖ���
//|     turi_dat.coin_dsp[0-3]  �z��
//|     turi_dat.coin_dsp[4]    10�~�\�~�i����Œ�A�ݒ肠��̏ꍇ�̂ݒl������j
//|     turi_dat.coin_dsp[5]    50�~�\�~�i����Œ�A�ݒ肠��̏ꍇ�̂ݒl������j
//|     turi_dat.coin_dsp[6]    100�~�\�~�i����Œ�A�ݒ肠��̏ꍇ�̂ݒl������j
//|     �ݒ薇��
//|     initial_cnt[0-3]  �z��
//|     initial_cnt[4]   10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
//|     initial_cnt[5]   10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnyctl( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyctl()	//�ޑK��[
{
	short	msg = -1;
	int		i, j;
	int		subtube = 0;
	ushort	initial_cnt[6];
	ushort	sub_dsp[2];
	ushort	ret, ret1, ret2;
	uchar	adr[] = { 27, 30, 33 }, tube[] = { 0, 0 };
	uchar	bit, idx;

	int		mod = 0;					// 0:�ޑK��[�A1:�ނ荇�킹

	if (! UsMnt_mnyctl_chk() ) {		// ���s�s�H
		BUZPIPI();
		return(MOD_EXT);
	}

	turikan_proc(MNY_CTL_BEFORE);

	bit = 0x01;
	idx = 0;
	sub_dsp[0] = 0;
	sub_dsp[1] = 0;
	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// �e����ɑ΂���\�~�̊����󋵂��擾����
		switch( ret ){
		case 1:	// �\�~�P
			tube[idx++] |= bit;
			break;
		case 2:	// �\�~�Q
			tube[idx++] |= bit;
			break;
		default:
			break;
		}
		if( idx > 1 ){
			break;
		}
		bit <<= 1;
	}
	idx = 0;
	for( i=0; i<2; i++ ){		// �\�~�P�E�Q�̌��ݖ����Ɋi�[
		if( tube[i] & 0x01 ){			// \10
			sub_dsp[idx++] = turi_dat.coin_dsp[4];
		} else if( tube[i] & 0x02 ){	// \50
			sub_dsp[idx++] = turi_dat.coin_dsp[5];
		} else if( tube[i] & 0x04 ){	// \100
			sub_dsp[idx++] = turi_dat.coin_dsp[6];
		}
		if( idx > 1 ){
			break;
		}
	}
	if( tube[0] && tube[1] ){	// ����̊ȑf���p
		subtube = 3;
	} else {
		if( tube[0] ){
			subtube = 1;
		} else if( tube[1] ){
			subtube = 2;
		}
	}
	initial_cnt[0] = (short)CPrmSS[S_KAN][3];
	initial_cnt[1] = (short)CPrmSS[S_KAN][9];
	initial_cnt[2] = (short)CPrmSS[S_KAN][15];
	initial_cnt[3] = (short)CPrmSS[S_KAN][21];
	initial_cnt[4] = 0;
	idx = 4;
	for( i=0; i<2; i++ ){		// �\�~�P�E�Q�̏����ݒ薇�����擾
		if( tube[i] & 0x01 ){			// \10
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[0], 3, 1 );
		} else if( tube[i] & 0x02 ){	// \50
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[1], 3, 1 );
		} else if( tube[i] & 0x04 ){	// \100
			initial_cnt[idx++] = (ushort)prm_get( COM_PRM, S_KAN, adr[2], 3, 1 );
		}
		if( idx > 5 ){
			break;
		}
	}

	ret = ret2 = 0;
	for( ; ret == 0 ; ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "���ޑK��[���@�@�@�@�@�@�@�@�@"

		if( subtube != 3 ){
			grachr( 1, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	/* // "�@�@�@�@�@�@���ݖ����@��������" */
		}else{
			grachr( 0, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	/* // "�@�@�@�@�@�@���ݖ����@��������" */
		}
		if(( CPrmSS[S_KAN][1] == 2 )&&( OPECTL.Mnt_lev >= 2 )){
			Fun_Dsp( FUNMSG[71] );					// "�@�@�@�@�@�@�@�@�@�ލ��� �I�� " */
		}else{
			Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
		}

		// �z����\��
		j = 1;
		if( subtube != 3 ) j = 2;
		for( i = 0; i < 4 ; i++ ){
			grachr( (unsigned short)(i + j),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"����"
			grachr( (unsigned short)(i + j), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"�F"
			opedsp( (unsigned short)(i + j), 12, (ushort)turi_dat.coin_dsp[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( (unsigned short)(i + j), 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"��"
			opedsp( (unsigned short)(i + j), 22, initial_cnt[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( (unsigned short)(i + j), 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"��"
		}

		// �\�~����\��
		if( subtube == 3 ){
			// �\�~�P�Ɨ\�~�Q�𗼕��g�p
			for( i=0; i<2; i++ ){
				switch( tube[i] ){
				case 0x01:	// \10
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"����"
					break;
				case 0x02:	// \50
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"����"
					break;
				case 0x04:	// \100
					grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"����"
					break;
				}
			}
			grachr( 5, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
			opedsp( 5, 12, sub_dsp[0], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 5, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
			opedsp( 5, 22, initial_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 5, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"

			grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
			opedsp( 6, 12, sub_dsp[1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
			opedsp( 6, 22, initial_cnt[5], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
		} else if( subtube ){
			// �Е��̂ݎg�p
			if( tube[0] == 0x01 || tube[1] == 0x01 ){	// \10
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"����"
			}
			if( tube[0] == 0x02 || tube[1] == 0x02 ){	// \50
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"����"
			}
			if( tube[0] == 0x04 || tube[1] == 0x04 ){	// \100
				grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"����"
			}
			grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
			opedsp( 6, 12, sub_dsp[0], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
			opedsp( 6, 22, initial_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
		}

		ret1 = 0;
		for( ; (ret1 == 0)&&(ret == 0) ; ){

			msg = StoF( GetMessage(), 1 );

			switch( KEY_TEN0to9( msg ) ){							/* FunctionKey Enter */

				case TIMEOUT6:
					ret = ret2;
					break;

				case COIN_EVT:
					switch( mod ) {
						case 2: // �I���҂�
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_EN_EVT)) )
							{	// ���o��
								ret = ret2;
								break;
							}
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_IH_EVT)) )
							{
								// ��������i�ۗL�����ω��j
								turikan_proc( MNY_INCOIN );
							}
							break;
						case 1: // �ނ荇�킹��
							if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_EN_EVT)) )
							{	// ���o��
								dispclr();
								grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "���ޑK��[���@�@�@�@�@�@�@�@�@"
								grachr( 3,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, MNCSTR[8] );	// "�@�@���������ލ��킹�������@�@"
								Fun_Dsp( FUNMSG[77] );													// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
								turikan_proc(MNY_CTL_AUTOSTART);
								break;
							}
							if( ( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_OT_EVT)) ) ||
							  ( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_ER_EVT)) ) )
							{	// ���o����
								turikan_proc(MNY_CTL_AUTOCOMPLETE);
								ret1 = 1;
								mod = 0;
								grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );	// "�@�@���������ލ��킹�������@�@" ������
							}
							break;
						case 0: // ��[��
							turikan_proc( MNY_INCOIN );

							j = 1;
							if( subtube != 3 ) j = 2;
							for( i = 0; i < 4; i++ ){
												/* �ۗL�����̑�������\���ر�ɉ��Z���� */
								if( turi_dat.coin_dsp[i] != turi_dat.dsp_ini[i] )
								{
									opedsp( (ushort)(i + j), 12, (ushort)turi_dat.coin_dsp[i], 3, 0, 0,
																		COLOR_BLACK, LCD_BLINK_OFF );
								}
							}
							break;
						default:
							break;
					}
					break;
				case KEY_TEN_F4:						// �ނ荇�킹(������)
					if( CPrmSS[S_KAN][1] != 2 ) break;
					if( OPECTL.Mnt_lev < 2 ) break;
					if( mod != 0 ) break;
					if( cn_errst[0] ){					/* �R�C�����b�N�̏ᒆ?(Y)			*/
						BUZPIPI();
						break;
					}
					for( i = 0; i < 4; i++ )
					{
						if( turi_dat.coin_dsp[i] > initial_cnt[i] )
						{
							mod = 1;
							break;
						}
					}
					if( mod == 1 )
					{
						BUZPI();
						turikan_proc(MNY_CTL_AUTO);
					}else{
						BUZPIPI();
					}
					break;

				case ARC_CR_R_EVT:						// ����IN
					// �����e�i���X�ł̶��ޑ}��(OPE_red == 2)�̏ꍇ�́Aopered�Ŗ߂����߁A�����ł͂��ǂ��Ȃ�				//
					// FT�EUT�ł͐��Z���Ƀ����e��ʂɈڍs���邱�Ƃ��Ȃ��̂ŁA�����e��ʒ���OPE_red��2�ȊO�ɂȂ邱�Ƃ�	//
					// �Ȃ����O�̂��߁AOPE_red��2�ȊO�̏ꍇ�͖߂��悤�ɂ���												//
					if( OPE_red != 2 )
					opr_snd( 13 );						// ���r�o
					if( NG == hojuu_card() ){			// ��[��������
						// ��[���ނłȂ��ꍇ
						break;
					}									// ��[��������ݽӰ�ޏI��
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:
				case KEY_TEN_F5:						// F5:Exit
					if( mod != 0 ) break;
					BUZPI();

					cn_stat( 2, 0 );				// �����s��
					Lagtim( OPETCBNO, 6, 250 ); 	// 5sec
					mod = 2;						// �ޑK��[���I��
												// ���o�\��ԁiCREM OFF�j�ɂȂ�܂ő҂�
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					ret2 = MOD_CHG;
//					if( msg == KEY_TEN_F5 )
//					{
//						ret2 = MOD_EXT;
//					}	
					if( msg == KEY_TEN_F5 )
					{
						ret2 = MOD_EXT;
					}else if(  msg == LCD_DISCONNECT ){
						ret2 = MOD_CUT;
					}else{
						ret2 = MOD_CHG;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					break;

				default:
					break;
			}
		}
	}
	Lagcan( OPETCBNO, 6 );
	turikan_proc( MNY_CTL_AFTER );
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| �ޑK��[�i���K�Ǘ��Ȃ��̏ꍇ�j
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnyctl2( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyctl2()	//�ޑK��[
{
	short	msg = -1;
	int		i, mod;
	ushort	ret, ret2;
	const uchar	*ARINASHI;

	if (! UsMnt_mnyctl_chk() ) {		// ���s�s�H
		BUZPIPI();
		return(MOD_EXT);
	}
	turikan_proc(MNY_CTL_BEFORE);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[1] );			// "���ޑK��[���@�@�@�@�@�@�@�@�@"
	grachr( 1, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[9]+14 );		// "�@�@�@�@�@�@�@�ޑK��ԁ@�@�@�@"
	Fun_Dsp( FUNMSG[8] );													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	// �z����\��
	for( i = 0; i < 4 ; i++ ){
		grachr( (unsigned short)(i + 2),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"����"
		grachr( (unsigned short)(i + 2), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"�F"
		if( CN_RDAT.r_dat09[i] != 0 ){
			ARINASHI = DAT6_0[9];
		}else{
			ARINASHI = DAT6_0[10];
		}
		grachr( (unsigned short)(i + 2), 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, ARINASHI );		// ����^�Ȃ�
	}

	mod = 0;
	ret = ret2 = 0;
	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){							/* FunctionKey Enter */

			case TIMEOUT6:
				ret = ret2;
				break;

			case COIN_EVT:
				switch( mod ) {

					case 2: // �I���҂�
						if( OPECTL.CN_QSIG == (uchar)(COIN_EN_EVT & 0x00ff ) )
						{	// ���o��
							ret = ret2;
						}
						break;

					case 0: // ��[��

						turikan_proc( MNY_INCOIN );

						for( i = 0; i < 4; i++ ){
											/* �ۗL�����̑�������\���ر�ɉ��Z���� */
							if( CN_RDAT.r_dat09[i] != 0 ){
								ARINASHI = DAT6_0[9];
							}else{
								ARINASHI = DAT6_0[10];
							}
							grachr( (unsigned short)(i + 2), 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, ARINASHI );		// ����^�Ȃ�
						}
						break;

					default:
						break;
				}
				break;

			case ARC_CR_R_EVT:					// ����IN
				// �����e�i���X�ł̶��ޑ}��(OPE_red == 2)�̏ꍇ�́Aopered�Ŗ߂����߁A�����ł͂��ǂ��Ȃ�				//
				// FT�EUT�ł͐��Z���Ƀ����e��ʂɈڍs���邱�Ƃ��Ȃ��̂ŁA�����e��ʒ���OPE_red��2�ȊO�ɂȂ邱�Ƃ�	//
				// �Ȃ����O�̂��߁AOPE_red��2�ȊO�̏ꍇ�͖߂��悤�ɂ���												//
				if( OPE_red != 2 )
					opr_snd( 13 );				// ���r�o
				if( NG == hojuu_card() ){		// ��[��������
					// ��[���ނłȂ��ꍇ
					break;
				}								// ��[��������ݽӰ�ޏI��
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
			case KEY_MODECHG:
			case KEY_TEN_F5:					// F5:Exit
				if( mod != 0 ) break;
				BUZPI();

				cn_stat( 2, 0 );				// �����s��
				Lagtim( OPETCBNO, 6, 250 ); 	// 5sec
				mod = 2;						// �ޑK��[���I��
											// ���o�\��ԁiCREM OFF�j�ɂȂ�܂ő҂�
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				ret2 = MOD_CHG;
//				if( msg == KEY_TEN_F5 )
//				{
//					ret2 = MOD_EXT;
//				}
//				
//				break;
				if( msg == KEY_TEN_F5 )
				{
					ret2 = MOD_EXT;
				}else if( msg == LCD_DISCONNECT ){
					ret2 = MOD_CUT;
				}else{
					ret2 = MOD_CHG;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 6 );
	turikan_proc( MNY_CTL_AFTER );
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
//| �ޑK�����ύX
//|   �\�~���Ƃ���SUB�`���[�u�A�⏕�`���[�u���ݒ�\�B
//|   SUB�`���[�u�́A10�~�A100�~�A���ݒ�\�B
//|   �⏕�`���[�u�́A10�~�A50�~�A100�~�A���ݒ�\�B
//|   SUB�ƕ⏕�𓯂�����ɐݒ肷�邱�Ƃ��\�B
//|   �\�~���ݒ�Ȃ����\�B
//|     ���ݖ���
//|     current_cnt[0-3]  �z��
//|     current_cnt[4]    10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
//|     current_cnt[5]    10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
//|     �ݒ薇��
//|     current_cnt[6-9]  �z��
//|     current_cnt[10]   10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
//|     current_cnt[11]   10�~�܂���50�~�܂���100�~�\�~�i����ρA�ݒ肠��̏ꍇ�̂ݒl������j
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_mnychg( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																		   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnychg()
{
	short	msg = -1;
	int		i, j;
	int		genmai_change = 0;
	int		subtube = 0;
	short	inpt = -1;
	uchar	pos[2];
	ushort	current_cnt[12], usWork, ret;
	int		Mnt_lev = OPECTL.Mnt_lev;
	uchar	adr[] = { 27, 30, 33 }, tube[] = { 0, 0 };
	uchar	bit, idx, wkc;


	if (turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT) {		// �ޑK�Ǘ����v�󎚒��H
		BUZPIPI();
		return MOD_EXT;
	}
	pos[0] = pos[1] = 0;

	turikan_proc( MNY_CHG_BEFORE );

	for( i = 0; i < 4 ; i++ ){
		current_cnt[i] = turi_dat.coin_dsp[i];
	}
	bit = 0x01;
	idx = 0;
	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// �e����ɑ΂���\�~�̊����󋵂��擾����
		switch( ret ){
		case 1:	// �\�~�P
			tube[idx++] |= bit;
			break;
		case 2:	// �\�~�Q
			tube[idx++] |= bit;
			break;
		default:
			break;
		}
		if( idx > 1 ){
			break;
		}
		bit <<= 1;
	}
	idx = 4;
	for( i=0; i<2; i++ ){		// �\�~�P�E�Q�̌��ݖ������擾
		if( tube[i] & 0x01 ){			// \10
			current_cnt[idx++] = turi_dat.coin_dsp[4];
		} else if( tube[i] & 0x02 ){	// \50
			current_cnt[idx++] = turi_dat.coin_dsp[5];
		} else if( tube[i] & 0x04 ){	// \100
			current_cnt[idx++] = turi_dat.coin_dsp[6];
		}
		if( idx > 5 ){
			break;
		}
	}
	if( tube[0] && tube[1] ){	// ����̊ȑf���p
		subtube = 3;
	} else {
		if( tube[0] ){
			subtube = 1;
		} else if( tube[1] ){
			subtube = 2;
		}
	}

	current_cnt[6] = (short)CPrmSS[S_KAN][3];
	current_cnt[7] = (short)CPrmSS[S_KAN][9];
	current_cnt[8] = (short)CPrmSS[S_KAN][15];
	current_cnt[9] = (short)CPrmSS[S_KAN][21];
	idx = 10;
	for( i=0; i<2; i++ ){		// �\�~�P�E�Q�̏����ݒ薇�����擾
		if( tube[i] & 0x01 ){			// \10
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[0], 3, 1 );
		} else if( tube[i] & 0x02 ){	// \50
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[1], 3, 1 );
		} else if( tube[i] & 0x04 ){	// \100
			current_cnt[idx++] = (short)prm_get( COM_PRM, S_KAN, adr[2], 3, 1 );
		}
		if( idx > 11 ){
			break;
		}
	}

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[7] );			// "���ޑK�������@�@�@�@�@�@�@   " */
	if( subtube != 3 ){
		grachr( 1, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	// "�@�@�@�@�@�@���ݖ����@��������" */
	}else{
		grachr( 0, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[6]+12 );	// "�@�@�@�@�@�@���ݖ����@��������" */
	}

	usWork = 0;
	if( Mnt_lev > 1 ){
		Fun_Dsp( FUNMSG[75] );					// "�@���@�@���@�@���@ ����  �I�� " */
		usWork = 1; // �ŏ��������]������
	}else{
		Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
	}

	// �z����\��
	j = 1;
	if( subtube != 3 ) j = 2;
	for( i = 0; i < 4 ; i++ ){
		grachr( (unsigned short)(j + i),  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i]+2 );	//"����"
		grachr( (unsigned short)(j + i), 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"�F"
		opedsp( (unsigned short)(j + i), 12, current_cnt[i], 3, 0, usWork, COLOR_BLACK, LCD_BLINK_OFF );
		usWork = 0;
		grachr( (unsigned short)(j + i), 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"��"
		opedsp( (unsigned short)(j + i), 22, current_cnt[6+i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( (unsigned short)(j + i), 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"��"
	}

	// �\�~����\��
	if( subtube == 3 ){
		// �\�~�P�Ɨ\�~�Q�𗼕��g�p
		for( i=0; i<2; i++ ){
			switch( tube[i] ){
			case 0x01:	// \10
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"����"
				break;
			case 0x02:	// \50
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );//"����"
				break;
			case 0x04:	// \100
				grachr( i+5,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"����"
				break;
			}
		}
		grachr( 5, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
		opedsp( 5, 12, current_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 5, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
		opedsp( 5, 22, current_cnt[10], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 5, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"

		grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
		opedsp( 6, 12, current_cnt[5], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
		opedsp( 6, 22, current_cnt[11], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
	} else if( subtube ){
		// �Е��̂ݎg�p
		if( tube[0] == 0x01 || tube[1] == 0x01 ){	// \10
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[7]+2 );	//"����"
		}
		if( tube[0] == 0x02 || tube[1] == 0x02 ){	// \50
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[13]+2 );	//"����"
		}
		if( tube[0] == 0x04 || tube[1] == 0x04 ){	// \100
			grachr( 6,  0, 10,  0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[8]+2 );	//"����"
		}
		grachr( 6, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		//"�F"
		opedsp( 6, 12, current_cnt[4], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
		opedsp( 6, 22, current_cnt[10], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		grachr( 6, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		//"��"
	}

	ret = 0;
	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						/* �I�� */
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN_F4:						/* ���� */
				if( Mnt_lev < 2 ) break;

				if( inpt != -1 ){
					if( mnyctl_nuchk( pos[0], inpt, tube[0], tube[1] ) != 0 ){
						BUZPIPI();
						inpt = -1;
						break;
					}
				}

				BUZPI();
				pos[1] = pos[0];
				if( inpt != -1 )
				{
					current_cnt[pos[0]] = inpt;

					if( pos[0] > 5 )
					{
						if( pos[0] < 10 )
						{	// ������������
							usWork = (ushort)(pos[0]-6);
							CPrmSS[S_KAN][3+usWork*6] = inpt;
						}else{
							if( pos[0] == 10 ){
								wkc = tube[0];
							}else{
								wkc = tube[1];
							}
							switch( wkc ){
							case 0x01:	// \10
								CPrmSS[S_KAN][27] = inpt;
								break;
							case 0x02:	// \50
								CPrmSS[S_KAN][30] = inpt;
								break;
							case 0x04:	// \100
								CPrmSS[S_KAN][33] = inpt;
								break;
							}
						}
						OpelogNo = OPLOG_SHOKIMAISUHENKO;		// ���엚��o�^
						SetChange = 1;
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}else{
						genmai_change = -1;
						switch( pos[0] )
						{
							case 0: turikan_proc(MNY_CHG_10YEN); break;
							case 1: turikan_proc(MNY_CHG_50YEN); break;
							case 2: turikan_proc(MNY_CHG_100YEN); break;
							case 3: turikan_proc(MNY_CHG_500YEN); break;
							case 4:
							case 5:
								switch( tube[(pos[0]-4)] ){
								case 0x01:	// \10
									turikan_proc(MNY_CHG_10SUBYEN);
									break;
								case 0x02:	// \50
									turikan_proc(MNY_CHG_50SUBYEN);
									break;
								case 0x04:	// \100
									turikan_proc(MNY_CHG_100SUBYEN);
									break;
								}
								break;
						}
					}
				}
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F2, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F3:						/* �� or �� */
				if( Mnt_lev < 2 ) break;

				BUZPI();
				pos[1] = pos[0];

				if( pos[0] < 6 )
					Fun_Dsp( FUNMSG[76] );						/* // "�@���@�@���@�@���@ ����  �I�� " */
				else
					Fun_Dsp( FUNMSG[75] );						/* // "�@���@�@���@�@���@ ����  �I�� " */

				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F3,(ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_CL:						//Clear
				if( Mnt_lev < 2 ) break;
				BUZPI();
				inpt = -1;
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F1:						/* �� */
				if( Mnt_lev < 2 ) break;
				BUZPI();
				pos[1] = pos[0];
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F1, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN_F2:						/* �� */
				if( Mnt_lev < 2 ) break;
				BUZPI();
				pos[1] = pos[0];
				pos[0] = mnyctl_cur( pos[0], KEY_TEN_F2, (ushort)subtube );
				inpt = -1;
				mnyctl_dsp((ushort)(pos[1]), current_cnt[pos[1]], 0, (ushort)subtube );
				mnyctl_dsp((ushort)(pos[0]), current_cnt[pos[0]], 1, (ushort)subtube );
				break;

			case KEY_TEN:
				if( Mnt_lev < 2 ) break;
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 100 ) * 10 + (short)(msg - KEY_TEN0);
				if( inpt != -1 ){						//���͂���
					mnyctl_dsp((ushort)(pos[0]), (ushort)inpt, 1, (ushort)subtube );
				}
				break;

			default:
				break;
		}
	}
	if( genmai_change != 0 )
	{	// ���ݖ�����ύX����
		for( i = 0; i < 4; i++ ){
			turi_dat.coin_dsp[i] = current_cnt[i];
		}
		for( i=0; i<2; i++ ){
			if( i == 0 ){
				usWork = current_cnt[4];
			} else {
				usWork = current_cnt[5];
			}
			switch( tube[i] ){
			case 0x01:	// \10
				turi_dat.coin_dsp[4] = usWork;
				break;
			case 0x02:	// \50
				turi_dat.coin_dsp[5] = usWork;
				break;
			case 0x04:	// \100
				turi_dat.coin_dsp[6] = usWork;
				break;
			}
		}
	}
	turikan_proc( MNY_CHG_AFTER );	// �ޑK�Ǘ������

	Lagtim( OPETCBNO, 6, 150 );	// 3sec
	for( ; ; )
	{
		msg = GetMessage();
		if(( msg == COIN_EVT)||(msg == TIMEOUT6) )
		{
			break;
		}
	}
	Lagcan( OPETCBNO, 6 );
	return( ret );
}

// ���͒l����
// return = 0:OK -1:NG
// ---------------------------------------------------------------------------------
// NOTE: FT4000�����̖������菈����FT4800�̏�������̗��p�����A
//       FT4000�Ŏg�p����R�C�����b�N�͗L����t��������FT4800�̂��Ƃ͈̂قȂ�ׁA
//       GT7700�ł̏������Q�l�ɏC�����s�����B
// ---------------------------------------------------------------------------------
static int mnyctl_nuchk( uchar loc, short inpt, uchar sub1, uchar sub2 )
{
	int nResult = 0; // OK
	short nInpChk;
	short nChknu10sub;
	short nChknu100sub;
	short nChknu50sub;
	uchar wkc;
	nInpChk = 0;

	nChknu10sub = 0;
	nChknu100sub = 0;
	nChknu50sub = 0;
	wkc = 0;
	switch( loc ){
	case 4:
	case 5:
		if( loc == 4 ){
			wkc = sub1;
		} else {
			wkc = sub2;
		}
		switch( wkc ){
		case 0x01:		// \10
			nChknu10sub = TUB_CURMAX10SUB + TUB_CURMAX10YO;
			break;
		case 0x02:		// \50
			nChknu50sub = TUB_CURMAX50YO;
			break;
		case 0x04:		// \100
			nChknu100sub = TUB_CURMAX100SUB + TUB_CURMAX100YO;
			break;
		}
		break;
	case 10:
	case 11:
		if( loc == 10 ){
			wkc = sub1;
		} else {
			wkc = sub2;
		}
		switch( wkc ){
		case 0x01:		// \10
			nChknu10sub = TUB_MAX10SUB + TUB_MAX10YO;
			break;
		case 0x02:		// \50
			nChknu50sub = TUB_MAX50YO;
			break;
		case 0x04:		// \100
			nChknu100sub = TUB_MAX100SUB + TUB_MAX100YO;
			break;
		}
		break;
	}

	switch( loc ){
		case 0:
			nInpChk = TUB_CURMAX10;
			break;
		case 1:
			nInpChk = TUB_CURMAX50;
			break;
		case 2:
			nInpChk = TUB_CURMAX100;
			break;
		case 3:
			nInpChk = TUB_CURMAX500;
			break;
		case 6:
			nInpChk = TUB_MAX10;
			break;
		case 7:
			nInpChk = TUB_MAX50;
			break;
		case 8:
			nInpChk = TUB_MAX100;
			break;
		case 9:
			nInpChk = TUB_MAX500;
			break;
		case 4:		// ���ݖ����F�\�~�P
		case 10:	// ���������F�\�~�P
		case 5:		// ���ݖ����F�\�~�Q
		case 11:	// ���������F�\�~�Q
			if( loc == 4 || loc == 10 ){
				wkc = sub1;
			} else {	// loc == 5 || loc == 11
				wkc = sub2;
			}
			switch( wkc ){
			case 0x01:		// \10
				nInpChk = nChknu10sub;
				break;
			case 0x02:		// \50
				nInpChk = nChknu50sub;
				break;
			case 0x04:		// \100
				nInpChk = nChknu100sub;
				break;
			}
			break;
	}

	if( inpt > nInpChk ){
		nResult = -1;
	}
	return( nResult );
}

// �����\�����s��
static const ushort dsp_loc[][2] = {
	{ 1, 12 },	// 10�~
	{ 2, 12 },	// 50�~
	{ 3, 12 },	// 100�~
	{ 4, 12 },	// 500�~
	{ 5, 12 },	// SUB 10�~ or 100�~
	{ 6, 12 },	// SUB 100�~

	{ 1, 22 },	// 10�~
	{ 2, 22 },	// 50�~
	{ 3, 22 },	// 100�~
	{ 4, 22 },	// 500�~
	{ 5, 22 },	// SUB 10�~ or 100�~
	{ 6, 22 } };// SUB 100�~

static void mnyctl_dsp( ushort loc, ushort dat, ushort rev, ushort sub )
{
	if( sub == 3 ){
		opedsp( dsp_loc[loc][0], dsp_loc[loc][1], dat, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );
	}else{
		opedsp( (ushort)(dsp_loc[loc][0]+1), dsp_loc[loc][1], dat, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );
	}
}

// ���وړ�����
static uchar mnyctl_cur( uchar pos, ushort inkey, ushort subtube )
{
	uchar max1, max2;

	if( subtube == 3 )
	{	// 10�~�A100�~�Ƃ��ɂ���
		max1 = 5;
		max2 = 11;
	}else if( subtube ){
		max1 = 4;
		max2 = 10;
	}else{
		max1 = 3;
		max2 = 9;
	}
	switch( inkey )
	{
		case KEY_TEN_F1: //��
			if( pos <= max1 )
			{
				if( pos == 0 ){
					pos = max1;
				}else{
					pos--;
				}
			}else{
				if( pos == 6 ){
					pos = max2;
				}else{
					pos--;
				}
			}
			break;
		case KEY_TEN_F2: // ��
			if( pos <= max1 )
			{
				if( pos == max1 ){
					pos = 0;
				}else{
					pos++;
				}
			}else{
				if( pos >= max2 ){
					pos = 6;
				}else{
					pos++;
				}
			}
			break;
		case KEY_TEN_F3: //���@��
			if( pos <= max1 ){
				pos = 6;
			}else{
				pos = 0;
			}
			break;
	}
	return pos;
}

unsigned short	UsMnt_invtry()	//�C���x���g��
{

	short			msg = -1;
	short			inpt = -1;
	unsigned short	cindsp, inv_excute, ret;
	int				dsp;
	int				mod;
	char			mec_wait = 0;

	if (turi_kan_f_defset_wait == TURIKAN_DEFAULT_WAIT) {		// �ޑK�Ǘ����v�󎚒��H
		BUZPIPI();
		return MOD_EXT;
	}
	turikan_proc( MNY_COIN_INVSTART );
	inv_excute = 0;
	cindsp = 0;
	mod = 0;
INVTOP:

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[2] );			/* // [01]	"���C���x���g�����@�@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[3] );			/* // [01]	"�@���o���z�F�@�@�@�@�@�@�@�@�@" */
	grachr( 2, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3] );			//"�~"
	Fun_Dsp( FUNMSG[70] );						/* // [70]	"�@�@�@�@�@�@�@�@�@ ���o  �I�� " */


	dsp = 1;
	ret = 0;
	for( ; ret == 0; ){

		if( dsp ){
			cindsp = (unsigned short)( inpt == -1 ? 0 : inpt );
			opedsp( 2, 12, cindsp, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// input
			opedsp( 2, 16, 0, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// 0
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg )){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:						/* Exit */
				if( 1 == mod )
					break;
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN:
				if( 1 == mod )
					break;
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 10 ) * 10 + (short)(msg - KEY_TEN0);
				dsp = 1;
				break;

			case KEY_TEN_CL:						// Clear
				if( 1 == mod )
					break;
				BUZPI();
				inpt = -1;
				dsp = 1;
				break;

			case KEY_TEN_F3:						/* �͂� */
				if( mod == 1 ){
					mod = 2;
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] );		// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
					Fun_Dsp( FUNMSG[70] );													// "�@�@�@�@�@�@�@�@�@ ���o  �I�� "
				}else{
					break;
				}
			case COIN_EVT:					// �R�C�����b�N�C�x���g
				if( msg == COIN_EVT ){
					if( OPECTL.CN_QSIG == 0x35 ){   	// �C���x���g���J�n���ɂ͕ۗL�����̑��M������̂ŁA�ۗL�������M�����������ɒǉ�
						mec_wait = 0x02;
						break;
					}else if( OPECTL.CN_QSIG == 0x07 ){	// ���ү�����u���o����ԁv��M
						Lagcan( OPETCBNO, 6 );
						mec_wait |= 0x02;
						if( !(mec_wait & 0x01) ){
							break;
						}
					}else{
						break;
					}
				}
			case KEY_TEN_F4:						/* Start(������) */
				if( mod == 1 )						// �ޱ�Jү���ޒ�
				{
					BUZPI();
					mod = 0;
					goto INVTOP;
				}
				if(( inpt == -1 )||( inpt == 0 ))
				{
					BUZPIPI();
					break;
				}
				BUZPI();
				if( mec_wait <= 1 ){
					mec_wait = 1;
					Lagtim( OPETCBNO, 6, 50*5 );	// ���o�\��ԑJ�ڑ҂��Ď� 5s
					break;
				}

				mod = 0;
				inpt = -1;
				inv_excute = 123;
				opedsp( 2, 12, cindsp, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// input
				//���o�J�n
				safecl( 0 ); 						/* ���ɓ����O���� */
				if( refund( (long)( cindsp * 10 ) ) != 0 ){
					//err
					BUZPIPI();
					inv_excute = 0;
					dsp = 1;
					break;
				}else{
					//ok
					;
				}
				Fun_Dsp( FUNMSG[8] );			/* // [08]	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				Lagtim( OPETCBNO, 6, 50*60*3 );	// ���o�Ď� 3min

				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case LCD_DISCONNECT:
							ret = MOD_CUT;
							break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case KEY_MODECHG:
							BUZPI();
							ret = MOD_CHG;
							break;
						case TIMEOUT6:
							Lagcan( OPETCBNO, 6 );
							cn_stat( 2, 0 );				// CREM OFF
							mec_wait = 0;
							goto INVTOP;
							break;
						case COIN_EVT:
							if( ( OPECTL.CN_QSIG == (uchar)(COIN_OT_EVT & 0x00ff ) ) ||
								( OPECTL.CN_QSIG == (uchar)(COIN_ER_EVT & 0x00ff ) ) )
							{	// ���o����
								Lagcan( OPETCBNO, 6 );
								cn_stat( 2, 0 );			// CREM OFF
								mec_wait = 0;
								goto INVTOP;
							}
							break;
						default:
							break;
					}
				}
				break;
			case TIMEOUT6:
				Lagcan( OPETCBNO, 6 );
				BUZPIPI();
				mec_wait = 0;
				goto INVTOP;
				break;

			default:
				break;
		}
	}
	if( inv_excute )
	{
		xPause( 40L ); 	// 400ms wait
						// ���Ɍv�Z���N�����Ă��܂��ׁA���o�ް������S�Ɏ�M����܂ő҂B
		turikan_proc( MNY_COIN_INVCOMPLETE );
	}
	turi_dat.turi_in = 0;
	return( ret );
}

unsigned short	UsMnt_mnybox()	//���Ɋm�F
{
	short			msg = -1;
	unsigned char	i;
	unsigned short	cincnt[COINCHK_CNT+1];
	unsigned short	ret = 0;
	unsigned char	pritimer = OFF;
	T_FrmCoSyuukei FrmCoSyuukei;
	T_FrmSiSyuukei FrmSiSyuukei;
	date_time_rec	*p_sky_date;	// �W�v�����ް��߲���
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
	int				inji_end = 0;
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)

	Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
	memset( cincnt, 0, COINCHK_CNT );
	for( i = 0; i < 4; i++ )
	{
		cincnt[i] = (unsigned short)(SFV_DAT.safe_dt[i]);
	}
	cincnt[4] = (unsigned short)(SFV_DAT.nt_safe_dt);

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[4] );				// [01]	"�����Ɋm�F���@�@�@�@�@�@�@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNCSTR[5] );				// [05]	"�@�@�@�@�@�@���ݖ����@�@�@�@�@"
	Fun_Dsp( FUNMSG[84] );														// [84]	"�����v�R���v���O�v�R�O�v �I�� "

	for( i = 0; i < (COINCHK_CNT+1) ; i++ ){
		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );	//"����"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	//"�F"
		opedsp( (unsigned short)(i + 2), 14, cincnt[i], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	//���ɖ���
		grachr( (unsigned short)(i + 2), 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	//"��"
	}


	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
		if ((msg & INNJI_ENDMASK) != 0) {
			msg &= ~INNJI_ENDMASK;
			inji_end = 1; // �󎚏I��
		}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)

		switch( KEY_TEN0to9( msg )){				// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
				// �h�A�m�u���ǂ����̃`�F�b�N�����{
				if (CP_MODECHG && pritimer == ON) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				BUZPI();
				ret = MOD_CHG;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				break;

			case KEY_TEN_F2:						// �e�Q�i�R���v�j�L�[
				if( pritimer == OFF ){
					if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					kinko_syu( 2, 1 );				// ��݋��ɏ��v
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_COIN_SHOKEI, 0, 0 );	// ���엚��o�^

					Lagtim( OPETCBNO, 6, 3*50 );	// �󎚗v���ް����ϰ(�R�b)�N���i�A�����Ĉ󎚗v���������Ȃ��悤�ɶް�ނ���j
					pritimer = ON;					// ��ϰ�׸޾��
				}
				break;

			case KEY_TEN_F1:						// �e�P�i�����v�j�L�[
				if( pritimer == OFF ){
					if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					kinko_syu( 3, 1 );				// �������ɏ��v
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_NOTE_SHOKEI, 0, 0 );	// ���엚��o�^

					Lagtim( OPETCBNO, 6, 3*50 );	// �󎚗v���ް����ϰ(�R�b)�N���i�A�����Ĉ󎚗v���������Ȃ��悤�ɶް�ނ���j
					pritimer = ON;					// ��ϰ�׸޾��
				}
				break;
			case KEY_TEN_F3:						// �e�R�i���O�v�j�L�[
				if( pritimer == OFF ){
					p_sky_date = &nobk_syu.NowTime;		// �O�񎆕����ɍ��v�F����W�v�����ް��߲���
					if( (p_sky_date->Year == 0) &&
					    (p_sky_date->Mon  == 0) &&
					    (p_sky_date->Day  == 0) &&
					    (p_sky_date->Hour == 0) &&
					    (p_sky_date->Min  == 0) ){

						// �O��W�v�ް����Ȃ��ꍇ
						BUZPIPI();
						break;
					}
					if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					memcpy( &FrmSiSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// ���ݎ������
					FrmSiSyuukei.prn_kind = R_PRI;
					FrmSiSyuukei.prn_data = &nobk_syu;
					queset( PRNTCBNO, PREQ_SIKINKO_Z, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// �������ɍ��v��
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_NOTE_ZENGOUKEI, 0, 0 );
					Lagtim( OPETCBNO, 6, 3*50 );	// �󎚗v���ް����ϰ(�R�b)�N���i�A�����Ĉ󎚗v���������Ȃ��悤�ɶް�ނ���j
					pritimer = ON;					// ��ϰ�׸޾��
				}
				break;
			case KEY_TEN_F4:						// �e�S�i�R�O�v�j�L�[
				if( pritimer == OFF ){
					p_sky_date = &cobk_syu.NowTime;		// �O��݋��ɍ��v�F����W�v�����ް��߲���
					if( (p_sky_date->Year == 0) &&
					    (p_sky_date->Mon  == 0) &&
					    (p_sky_date->Day  == 0) &&
					    (p_sky_date->Hour == 0) &&
					    (p_sky_date->Min  == 0) ){

						// �O��W�v�ް����Ȃ��ꍇ
						BUZPIPI();
						break;
					}
					if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					memcpy( &FrmCoSyuukei.PriTime, &CLK_REC, sizeof(date_time_rec) );		// ���ݎ������
					FrmCoSyuukei.prn_kind = R_PRI;
					FrmCoSyuukei.prn_data = &cobk_syu;
					queset( PRNTCBNO, PREQ_COKINKO_Z, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ��݋��ɍ��v��
					Ope_DisableDoorKnobChime();
					wopelg( OPLOG_COIN_ZENGOUKEI, 0, 0 );
					Lagtim( OPETCBNO, 6, 3*50 );	// �󎚗v���ް����ϰ(�R�b)�N���i�A�����Ĉ󎚗v���������Ȃ��悤�ɶް�ނ���j
					pritimer = ON;					// ��ϰ�׸޾��
				}
				break;

			case KEY_TEN_F5:						// �e�T�i�I���j�L�[
				BUZPI();
				ret = MOD_EXT;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;

			case TIMEOUT6:							// �󎚗v���ް����ϰ��ѱ��
				pritimer = OFF;						// ��ϰ�׸�ؾ��
				break;

			default:
				break;
		}
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
		// �󎚏I����A�h�A�m�u��Ԃł���΃����e�i���X�I��
		if (inji_end == 1 && CP_MODECHG) {
			ret = MOD_CHG;
			break;
		}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
	}
	if( pritimer == ON ){							// �󎚗v���ް����ϰ�N�����H
		Lagcan( OPETCBNO, 6 );						// ��ϰ����
	}
	return( ret );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �G���[�E�A���[���m�F																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: UsMnt_ErrAlm( void )															   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: ushort																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto																	   |*/
/*| Date		: 2005-06-25																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// ALM_CHK[]�ɑΉ����镶�����ð��ٔԍ����`����
static const unsigned short	AlmTableNo[ALM_MOD_MAX][ALM_NUM_MAX] = {
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 
{	// Main 00
	0, 2, 0, 0, 0,22,23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
{	// Sub 01
	0,97, 4, 0, 0, 0,13,15, 0, 0, 0, 0, 0, 0, 0, 5, 6, 7, 8, 0, 
	0, 9,10,11,12, 0, 0, 0,96, 3, 0,16,14, 0, 0, 0, 0, 0, 0, 0, 
// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5949 �G���[�R�[�h�Ɛ��Z�@�̃G���[�\������v���Ȃ�
//// MH810104 GG119201(S) �d�q�W���[�i���Ή�
////	0, 0, 0, 0, 0, 0,113,114,115,116, 0, 0, 0,75, 0, 0, 0, 0, 0, 0, 
//	0, 0, 0, 0, 0,326,113,114,115,116, 0, 0, 0,75, 0,324,325, 0, 0, 0, 
//// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	0, 0, 0, 0, 0,325,113,114,115,116, 0, 0, 0,75, 0,323,324, 0, 0, 0, 
// MH810104 GG119201(E) �d�q�W���[�i���Ή� #5949 �G���[�R�[�h�Ɛ��Z�@�̃G���[�\������v���Ȃ�
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
// MH810105 GG119202(S) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 298, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 327, 0, 0, 0, 0, 0, 0, 0, 0, 298, 0, 0, 0, 0, 0, 0, 
// MH810105 GG119202(E) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
},
{	// 02
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
{	// 05 �F�؋@�֘A
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
},
};
// ERR_CHK[]�ɑΉ����镶�����ð��ٔԍ����`����
static const unsigned short	ErrTableNo[ERR_MOD_MAX][ERR_NUM_MAX] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 
{	// Main 00
	 0,17,0,0, 0,18,19,24,25, 0,20,21, 0,180,181, 0,26,72,73, 0,
	 0, 0, 0, 0, 0,68,69,70,71, 0, 0,74, 0, 0, 0,131, 0, 0, 0, 0,
	 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,95, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 51, 0, 0, 0, 65, 66, 0, 0, 0, 0, 0, 0, 0,200,201,202,203,
},
{	// Reader 01
	 0,29, 182, 0, 0, 0, 0, 0, 0, 0,30,31, 0,32,33,34,35,36,37,38,
	39, 0, 0, 0,40,41, 0, 0, 0, 0, 0, 0,28, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Printer 02
	 0,42, 0,43,45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	46, 0,47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Coin 03
	 0,48, 0, 0, 0,49,50,52, 0, 0, 0, 0,53,54,55,56, 0, 0,57,58,
	59,60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Note 04
	 0,61, 0, 0, 0,62,63,64,67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// IF-ROCK 05
	 0,76,77,78,79,80,81,82,83,84,85,86,87, 0, 0, 0, 0, 0, 0, 0,
	// 05-20�͎q�@�ʐM�ُ�ł��邪�A�����ł͕\���ԍ��;�Ă��Ȃ��B�װ�̐��Ƃ͕ʂɐ����邽�߁B
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Mifare 06
	 0,98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// NT-NET Child 07
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 227, 0, 0, 0, 0, 228, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// NT-NET IBK 08
	 0,99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// ���������	64
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,104,  0,  0,  0,  0,  0,  0,  0,  0,105,106,107,108,109,110,111,  0,  0,  0,
	112,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},
{	// �ׯ�� 10
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,117,118,119,120,121,122,123,124,125,126,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,129,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
},
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//{	// CCT Credit 54
//	 0,144,145,146,147,148,149, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
//	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
//},
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
{	// LAN  Mod=72
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,165,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00�`19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 20�`39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,253,	/* 40�`59 */
	242,  0,  0,243,244,245,246,247,248,249,250,251,  0,  0,  0,  0,  0,  0,  0,  0,	/* 60�`79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80�`99 */
},
{	// Dopa  Mod=73
	0, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
{	// Suica	69
	 0,150, 151, 0,152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0,239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//{	// Edy	62
//	//0  1   2   3   4  5  6  7  8  9  10  11  12 13 14 15 16 17 18 19
//	 0,153,154,155,156, 0, 0, 0, 0, 0,157,  0,158, 0, 0, 0, 0, 0, 0, 0,
//   159,160,  0,161,  0, 0, 0, 0, 0, 0,162,163,164, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//	 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0,
//},
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
{	// remote 55
	 0,167, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},
{	// Dopa  Mod=77
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,184,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00�`19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,254,255,266,  0,  0,  0,	/* 20�`39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,253,	/* 40�`59 */
	242,  0,  0,243,244,245,246,247,248,249,250,251,  0,  0,185,186,187,  0,  0,  0,	/* 60�`79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80�`99 */
},
{	// NT-NET-DOPA MAF	65
	 0, 0, 225,226, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0,204, 0,205, 0,206, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,226, /* ���ށF60�` 79 */
	 207,208, 0, 0, 0, 0, 0, 0, 0, 0,224, 0, 0, 0,209,210,211,0, 0, 0, /* ���ށF80�` 99 */
},
{	// NT-NET-DOPA 	67
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},
{	// Cappi Credit 74
	 0,188, 0, 0,189,190, 0,191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},
{	// iDC 75
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},
{	// CRR 15
	  0,212,213,214,215,216,217,218,219,220,221,222,223,  0,  0,  0,  0,  0,  0,  0,/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,/* ���ށF80�` 99 */
},
{	// CARD 76
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},
{	// �ȈՃt���b�v�^���b�N���u Mod=16
	 0,76,77,78,79,80,81,82,83,84,85,86,87,131,132,133,134,135,136,137, /* ���ށF 0�` 19 */
   138,139,140,141,142,143,212,213,214,215,216,217,  0,  0,  0,  0,  0,  0,  0,  0, /* ���ށF20�` 39 */
	 0,  0,  0,218,219,220,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 234, 0, 0, 0, 0, 					/* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 					/* ���ށF80�` 99 */
},
{	// CAN 86
	 0,240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},
{	// SODIAC 25
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF 0�` 19 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF20�` 39 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF40�` 59 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF60�` 79 */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ���ށF80�` 99 */
},	
{	// I2C_Error
	//	ERR_I2C_BFULL_SEND_RP	10	I2Cڼ�đ��M�ޯ̧��		235
	//	ERR_I2C_BFULL_RECV_RP	11	I2Cڼ�Ď�M�ޯ̧��		236
	//	ERR_I2C_BFULL_SEND_JP	12	I2C�ެ��ّ��M�ޯ̧��	237
	//	ERR_I2C_BFULL_RECV_JP	13	I2C�ެ��َ�M�ޯ̧��	238
//	1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 235, 236, 237, 238, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�G���[�\�������Ή�)
{	// ���σ��[�_	32
//  0     1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19 
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
//	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ���ށF 0�` 19 */
// MH810103 GG119202(S) ���σ��[�_����̍ċN���v�����L�^����
//	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274, 288, 300,   0,   0,   0,   0,   0,   0,   0, /* ���ށF 0�` 19 */
	 0, 267, 268, 269, 270, 271, 272,   0,   0, 273, 274, 288, 315, 319,   0,   0,   0,   0,   0,   0, /* ���ށF 0�` 19 */
// MH810103 GG119202(E) ���σ��[�_����̍ċN���v�����L�^����
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
//   275,   0, 276, 277, 278, 279, 280, 281,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ���ށF20�` 39 */
   275,   0, 276, 277, 278, 279, 280, 281,   0,   0,   0,   0,   0,   0, 318,   0,   0,   0,   0,   0, /* ���ށF20�` 39 */
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ���ށF40�` 59 */
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//     0,   0,   0,   0, 299,   0,   0,   0,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ���ށF40�` 59 */
// MH810103 GG119202(S) ���ϒ�~�G���[�̓o�^
//     0,   0,   0,   0, 299,   0,   0, 301,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ���ށF40�` 59 */
// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
//     0,   0,   0,   0, 314,   0, 317, 316,   0,   0,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ���ށF40�` 59 */
     0,   0,   0,   0, 314,   0, 317, 316,   0,   332,   0,   0, 282,   0,   0,   0,   0,   0,   0, 283, /* ���ށF40�` 59 */
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
// MH810103 GG119202(E) ���ϒ�~�G���[�̓o�^
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//   284, 285, 286, 287, 288, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297,   0,   0,   0, /* ���ށF60�` 79 */
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��^PiTaPa�Ή�
//   284, 285, 286, 287,   0, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297,   0,   0,   0, /* ���ށF60�` 79 */
   284, 285, 286, 287,   0, 289,   0, 290,   0,   0, 291, 292, 293, 294, 295, 296, 297, 328, 329, 330, /* ���ށF60�` 79 */
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��^PiTaPa�Ή�
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ���ށF80�` 99 */
},
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�G���[�\�������Ή�)

// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
{	// �Ԕԃ`�P�b�g���XLCD�ʐM	26
	  0,299,300,301,302,303,304,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,305,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF80�` 99 */
},

// �o�[�R�[�h	30
{
	  0,  0,  0,  0,  0,306,  0,  0,  0,  0,307,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF80�` 99 */
},

// DC-NET�ʐM	28
{
	  0,311,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF80�` 99 */
},

// ���A���^�C���ʐM	37
{
	  0,308,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,309,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,310,  0,	/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF80�` 99 */
},

// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
// SD�J�[�h	31
{
	//0                                      10
// GG129000(S) R.Endo 2023/06/21 �Ԕԃ`�P�b�g���X4.1 #7063 �^�p����SD�J�[�h�̃t�@�C���V�X�e�����j������(FCR P230154)
// 	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,312,313,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,312,313,  0,333,  0,  0,  0,  0,  0,	/* ���ށF 0�` 19 */
// GG129000(E) R.Endo 2023/06/21 �Ԕԃ`�P�b�g���X4.1 #7063 �^�p����SD�J�[�h�̃t�@�C���V�X�e�����j������(FCR P230154)
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF20�` 39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF40�` 59 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF60�` 79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* ���ށF80�` 99 */
},
// MH810100(E) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
{	// �d�q�W���[�i��	22
//	  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19
// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5949 �G���[�R�[�h�Ɛ��Z�@�̃G���[�\������v���Ȃ�
	  0, 320, 321, 322, 326,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	//  0�`19
// MH810104 GG119201(E) �d�q�W���[�i���Ή� #5949 �G���[�R�[�h�Ɛ��Z�@�̃G���[�\������v���Ȃ�
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 20�`39
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 40�`59
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 60�`79
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 80�`99
},
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

{	// �T�uCPU�@�\	80
	 0,88,89, 0, 229,230,231,232,233, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
},

{	// �N���W�b�g( �G���[77�Ƌ��ʂ̕�������g���Ă��� )
	//0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19
	  0,256,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 00�`19 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,257,258,  0,  0,  0,  0,	/* 20�`39 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 40�`59 */
	263,264,265,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,259,260,261,262,  0,  0,	/* 60�`79 */
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* 80�`99 */
},

// �ȉ� 3���̃G���[�R�[�h ���ꏈ���p
{	// LAN2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
{	// Dopa2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
},
};

static	const	char	error_pri[] = {
	mod_main
	,	mod_SubCPU
	,	mod_read,	mod_prnt,	mod_coin,	mod_note
	,	mod_ifflap,	mod_ifrock, mod_crrflap
	,	mod_ntnet,	mod_ntibk
	,	mod_lprn,	mod_mifr
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	,	mod_cct,	mod_Suica,	mod_Edy
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
	,	mod_Suica
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	,	mod_remote
	,	mod_fomaibk
	,	mod_lanibk,	mod_lanibk2
	,	mod_dopaibk,	mod_dopaibk2
	,	mod_ntmf,	mod_ntdp
	,	mod_cappi
	,	mod_idc
	,	mod_card
	,	mod_flapcrb
	,	mod_can
	,	mod_sodiac
	,	mod_I2c	
	,	mod_Credit
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	,	mod_ec
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	,	mod_tklslcd
	,	mod_barcode
	,	mod_dc_net
	,	mod_realtime
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
	,	mod_sd
// MH810100(E) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	,	mod_eja
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	,	mod_cc
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
};
unsigned short UsMnt_ErrAlm( void )
{
	int i, j;
	short TotalPageNo;
	short CurrentPageNo;
	short msg = -1;
	ushort ret;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[23] );			/* "���G���[�E�A���[���m�F���@�@�@" */

	TotalPageNo = 0;
	// �������װт̐��𐔂���
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		for( j = 1; j < ALM_NUM_MAX; j++ ){
			if( ALM_CHK[i][j] && AlmTableNo[i][j] ){
				 TotalPageNo++;
			}
		}
	}

	// �������װ�̐��𐔂���
	for( i = 0; i < ERR_MOD_MAX; i++ ){
		for( j = 1; j < ERR_NUM_MAX; j++ ){
			if( ERR_CHK[i][j] && ErrTableNo[i][j] ){
				TotalPageNo++;
			}
		}
	}

	// ���b�N���u�ƃt���b�v�̊J�ُ�i���b�N�j�̐��𐔂���i�ȈՃt���b�v�^���b�N�j
	if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) ){
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 0, _MTYPE_LOCK ) );
		}
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 1, _MTYPE_LOCK ) );
		}
	}
	if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) ){
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 0, _MTYPE_INT_FLAP ) );
		}
		if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
			TotalPageNo += ( LKopeLockErrCheck( 1, _MTYPE_INT_FLAP ) );
		}
	}

	// ����ʐ����
	if( TotalPageNo ){
		TotalPageNo--;
		TotalPageNo = (TotalPageNo / 6) + 1;
	}else{
		TotalPageNo = -1;	// �Ȃɂ��������Ă��Ȃ�
	}

	if( TotalPageNo == -1 ){
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[0] ); // "  �������̃G���[�E�A���[����  "
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[1] ); // "        ����܂���            "
	}

	if( TotalPageNo > 1 ){
		Fun_Dsp( FUNMSG[6] );				// "�@���@�@���@�@�@�@�@�@�@ �I�� "
	}else{
		Fun_Dsp( FUNMSG[8] );				// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	}

	CurrentPageNo = 1;
	if( TotalPageNo != -1 ){
		HasseiErrSearch( CurrentPageNo );		// �װ��\������
	}

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg )
		{
			case KEY_TEN_F5:
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
				if( TotalPageNo == -1 ) break;
				if( TotalPageNo == 1 ) break;		// 1�߰�ނ����Ȃ��̂Ŗ���
				BUZPI();
				if( CurrentPageNo == 1 ){
					CurrentPageNo = TotalPageNo;
				}else{
					CurrentPageNo--;
				}
				HasseiErrSearch( CurrentPageNo );	// �װ��\������
				break;
			case KEY_TEN_F2:
				if( TotalPageNo == -1 ) break;
				if( TotalPageNo == 1 ) break;		// 1�߰�ނ����Ȃ��̂Ŗ���
				BUZPI();
				if( CurrentPageNo == TotalPageNo ){
					CurrentPageNo = 1;
				}else{
					CurrentPageNo++;
				}
				HasseiErrSearch( CurrentPageNo );	// �װ��\������
				break;
			case KEY_TEN_F3:
			case KEY_TEN_F4:
			case KEY_TEN_CL:
				break;
		}
	}
	return( ret );
}

//----------------------------------------------------------------------------
// 1)�������̱װтƴװ�������B
// 2)�߰�ޔԍ��ɂ��\�����鱲�т�z��Dspitem�־�Ă���
// 3)�z��Dspitem��\������
//----------------------------------------------------------------------------
static void HasseiErrSearch( short page )
{
	static uchar	dsp[30];
	int		i, j;
	int		k;
	int		TotalSearch = 0;
	int 	StartItemNu;
	int 	ItemOfPage = 0;		// 1��ʂɕ\�����鱲�т̐�

	struct {
		ushort	DspItem[6];
		ushort	SlaveIfErr[6];		// �q�@IF�մװ�p
		uchar	LockArea[6];		// ���
		ushort	LockPosi[6];		// �Ԏ��ԍ�
	} dpsb;

	uchar	buf[6];

	uchar	tno = 0;

	memset( &dpsb, 0, sizeof(dpsb) );

	// ��ʂɕ\������ŏ��̱��єԍ����߰�ޔԍ����羯Ă���
	if( page ) page -= 1;
	StartItemNu = page * 6;

	// �܂��װт��绰�����
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		for( j = 1; j < ALM_NUM_MAX; j++ ){
			if( ALM_CHK[i][j] && AlmTableNo[i][j] )
			{
				TotalSearch++;
				if( TotalSearch > StartItemNu ){
					dpsb.DspItem[ItemOfPage] = AlmTableNo[i][j];
					ItemOfPage++;
					if( ItemOfPage >= 6 ){
						// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
						j = ALM_NUM_MAX;
						i = ALM_MOD_MAX;
					}
				}
			}
		}
	}
	if( ItemOfPage < 6 ){
		// ���ɴװ�������
		for( k = 0; k < ERR_MOD_MAX; k++ ){
			i = error_pri[k];
			for( j = 1; j < ERR_NUM_MAX; j++ ){
				if( ERR_CHK[i][j] && ErrTableNo[i][j] )
				{
					TotalSearch++;
					if( TotalSearch > StartItemNu ){
						dpsb.DspItem[ItemOfPage] = ErrTableNo[i][j];
						ItemOfPage++;
						if( ItemOfPage >= 6 ){
							// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
							j = ERR_NUM_MAX;
							k = ERR_MOD_MAX;
						}
					}
				}
			}
		}
	}

	if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) ){
		if( ItemOfPage < 6 ){
			// �����ׯ�ߑ��u�̏㏸�ُ�i�ׯ�߁j�̐��𐔂���
			if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKCLOSEFAIL] ){
				for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){
					WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b04 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 93;
								// �Ԏ��ԍ���āi�\���p�j
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
									break;
								}
							}
						}
					}
				}
			}
		}

		if( ItemOfPage < 6 ){
			// �����ׯ�ߑ��u�̉��~�ُ�i�ׯ�߁j�̐��𐔂���
			if( ERR_CHK[mod_flapcrb][ERR_FLAPLOCK_LOCKOPENFAIL] ){
				for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){
					WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b05 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 94;
								// �Ԏ��ԍ���āi�\���p�j
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) ){
		if( ItemOfPage < 6 ){
			// �����ׯ�ߑ��u�̏㏸�ُ�i�ׯ�߁j�̐��𐔂���
			if( ERR_CHK[mod_flapcrb][ERR_FLAPCLOSEFAIL] ){
				for( i = INT_CAR_START_INDEX; i < BIKE_START_INDEX; i++ ){
					WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b04 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 127;
								// �Ԏ��ԍ���āi�\���p�j
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
									break;
								}
							}
						}
					}
				}
			}
		}

		if( ItemOfPage < 6 ){
			// �����ׯ�ߑ��u�̉��~�ُ�i�ׯ�߁j�̐��𐔂���
			if( ERR_CHK[mod_flapcrb][ERR_FLAPOPENFAIL] ){
				for( i = INT_CAR_START_INDEX; i < BIKE_START_INDEX; i++ ){
					WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					if( LockInfo[i].lok_syu ){
						if( FLAPDT.flp_data[i].nstat.bits.b05 ){
							TotalSearch++;
							if( TotalSearch > StartItemNu ){
								dpsb.DspItem[ItemOfPage] = 128;
								// �Ԏ��ԍ���āi�\���p�j
								dpsb.LockArea[ItemOfPage] = (uchar)LockInfo[i].area;
								dpsb.LockPosi[ItemOfPage] = (ushort)LockInfo[i].posi;
								ItemOfPage++;
								if( ItemOfPage >= 6 ){
									// �\�����鱲�ѐ�����ʈ�t�Ȃ̂Ż���I��
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	for( i = 0; i < 6; i++ ){
		if( dpsb.DspItem[i] ){
			if(( dpsb.DspItem[i] == 93 )||( dpsb.DspItem[i] == 94 )){
				// ���b�N���u�J�ُ�
// MH322914 (s) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
				dsp[18] = '('; // 0x0028;
				if( dpsb.LockArea[i] ){
					dsp[19] = (uchar)( 0x40 + dpsb.LockArea[i] );
					dsp[20] = '-'; // 0x002d;
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[21] = (uchar)(buf[0]);
					dsp[22] = (uchar)(buf[1]);
					dsp[23] = (uchar)(buf[2]);
					dsp[24] = (uchar)(buf[3]);
					dsp[25] = ')'; // 0x0029
				}else{
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[19] = (uchar)(buf[0]);
					dsp[20] = (uchar)(buf[1]);
					dsp[21] = (uchar)(buf[2]);
					dsp[22] = (uchar)(buf[3]);
					dsp[23] = ')';
				}
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // �װ�A�װѕ\��
			}else if(( dpsb.DspItem[i] == 127 )||( dpsb.DspItem[i] == 128 )){
				// �ׯ�ߑ��u�J�ُ�
// MH322914 (s) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
				dsp[20] = '('; // 0x0028;
				if( dpsb.LockArea[i] ){
					dsp[21] = (uchar)( 0x40 + dpsb.LockArea[i] );
					dsp[22] = '-'; // 0x002d;
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[23] = (uchar)(buf[0]);
					dsp[24] = (uchar)(buf[1]);
					dsp[25] = (uchar)(buf[2]);
					dsp[26] = (uchar)(buf[3]);
					dsp[27] = ')'; // 0x0029
				}else{
					intoas( buf, dpsb.LockPosi[i], 4 );
					dsp[21] = (uchar)(buf[0]);
					dsp[22] = (uchar)(buf[1]);
					dsp[23] = (uchar)(buf[2]);
					dsp[24] = (uchar)(buf[3]);
					dsp[25] = ')';
				}
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // �װ�A�װѕ\��
			}else if( dpsb.DspItem[i] == 129 ){
				// IBC�O���ް�������
// MH322914 (s) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
//				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(ushort)*30 );
				memcpy( dsp, ALARM_STR[dpsb.DspItem[i]], sizeof(uchar)*30 );
// MH322914 (e) kasiyama 2016/07/07 �ÓI�̈�̃������j��C��[���ʃo�ONo.1244](MH341106)
				dsp[24] = '('; // 0x0028;
				intoas( buf, (ushort)tno, 2 );
				dsp[25] = (uchar)(buf[0]);
				dsp[26] = (uchar)(buf[1]);
				dsp[27] = ')';
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dsp ); // �װ�A�װѕ\��
			}else if( (150 <= dpsb.DspItem[i]) && (dpsb.DspItem[i] <= 152) ){			// Suica���[�_�[�G���[
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[SuicaErrCharChange(dpsb.DspItem[i])] );	// "�d�q�}�l�[���[�_�[..."�\��
			}else{
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				if (dpsb.DspItem[i] == 330) {		// E32-79
					if (RecvSubBrandTbl.tbl_sts.BIT.MORE_LESS == 2){
						dpsb.DspItem[i] = 331;		// less
					}
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ALARM_STR[dpsb.DspItem[i]] ); // �װ�A�װѕ\��
			}
		}else{
			grachr( (ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNMSG[77] ); // �s�ر
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���f�[�^�m�F                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_TicketCkk( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_TicketCkk(void)
{
	ushort	msg;
	int	dsp, loop;
	short	page, pagemax;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[44]);	/* "�����f�[�^�m�F���@�@�@�@�@�@�@" */

// MH810100(S) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	CardErrDsp();		
	CardErrDsp( OPECTL.CR_ERR_DSP );	// �G���[�\��
// MH810100(E) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(�����e�i���X)
	for( ; ; ) {

		page = 0;
		if(( MAGred[MAG_ID_CODE] != (char)-1 ) && 		// MAGred�̒l��-1(0xFF)�̏ꍇ�͓ǎ�s�J�[�h�Ȃ̂ł���ȊO�ŁA
		   ( OPECTL.LastCardInfo & 0x8000) ){			// �ŏ��Bit�̃`�F�b�N��JIS�\�����𔻒�
			grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[6] );			// "JIS�J�[�h�@�@�@" 
		}else{
			switch( OPECTL.LastCardInfo ){
				case 0x2d:
					break;
				case 0x1a:
					break;
				case 0x0e:
					PrepaidDsp(0);				// ����߲�ޏ��쐬
					page = 1;
					pagemax = 2;
					break;
				case 0x2c:						// �񐔌�
					break;
				case 0x41:						// ���A�}�m�W���^��[�J�[�h��
					break;
				default:
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDERRSTR[11] );			// "�@�@�@���f�[�^�͂���܂���@�@"
					break;
			}
		}

		dsp = 1;
		for( loop=1; loop; ) {

			if( dsp ){
				dsp = 0;
				switch( OPECTL.LastCardInfo ){
					case 0x2d:
						ServiceDsp();
						break;
					case 0x1a:
						PassDsp();
						break;
					case 0x0e:
						PrepaidDsp(page);
						break;
					case 0x2c:							// �񐔌�
						KaiDsp();
						break;
					case 0x41:											// ���A�}�m�W���^��[�J�[�h��
						if(AmanoCard_Dsp())								// ��ʕ\�����쐬
						{
							grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDERRSTR[11] );		// "�@�@�@���f�[�^�͂���܂���@�@"
						}
						break;
				}
				if( page == 1 ){
					Fun_Dsp(FUNMSG[11]);				/* "�@�@�@�@���@�@�@�@�@�@�@ �I�� " */
				}else if( page == 2 ){
					Fun_Dsp(FUNMSG[12]);				/* "�@���@�@�@�@�@�@�@�@�@�@ �I�� " */
				}else{
					Fun_Dsp(FUNMSG[8]);					/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				}
			}

			msg = StoF( GetMessage(), 1 );

			switch( msg ){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:
					BUZPI();
					return( MOD_CHG );
				case KEY_TEN_F5:				/* F5:Exit */
					BUZPI();
					return( MOD_EXT );

				case KEY_TEN_F1:
					if( page != 0 ){
						if( page != 1 ){
							page--;
							BUZPI();
							dsp = 1;
						}
					}
					break;
				case KEY_TEN_F2:
					if( page != 0 ){
						if( page < pagemax ){
							BUZPI();
							page++;
							dsp = 1;
						}
					}
					break;

				case ARC_CR_R_EVT:				// Card inserted
					displclr(1);
					displclr(2);
					displclr(3);
					displclr(4);
					displclr(5);
					displclr(6);
					loop = 0;
					break;
				default:
					break;
			}
		}
	}
}


// �T�[�r�X���\��
static void ServiceDsp( void )
{
	uchar	buf[2];
	m_servic *mag = (m_servic*)&MAGred[MAG_ID_CODE];
	ulong	temp;
	static const char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };
	ushort	wk1, wk2;

	cr_dsp.service.Kind = (ushort)( mag->svc_pno[1] & 0x0f );
	cr_dsp.service.ShopNo = (short)mag->svc_sno[1] + (((short)mag->svc_sno[0])<<7);
	memcpy( cr_dsp.service.StartDate, mag->svc_sta, 6 );
	cr_dsp.service.Status = (ushort)( mag->svc_sts );

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->svc_pno[0];
		wk2 = mag->svc_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17�i�[
		wk2 = 0;
	}else{
		wk1 = mag->svc_pno[0];
		wk2 = mag->svc_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// ���ԏ�No.���
		wk2 = 0;
	}

	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"

	if( cr_dsp.service.Kind != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[0] );			// "�T�[�r�X���@�@�@"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[0] );			// "  ���      ���ރR�[�h        "
		buf[0] = (uchar)(0x82);	// '�`'
		buf[1] = (uchar)(0x60 + cr_dsp.service.Kind-1);
	    grachr( 3, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &buf[0] );							 // ���
		opedpl( 3, 22, (ulong)cr_dsp.service.ShopNo, 4, 1, 0 , COLOR_BLACK,  LCD_BLINK_OFF); // ���ރR�[�h
		wk2 = 1;
	}
	else if( cr_dsp.service.Status != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[2] );			// "�������@�@�@"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[3] );			// "  ���        �X��         "
		opedpl( 3, 6, (ulong)cr_dsp.service.Status, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // ���
		opedpl( 3,18, (ulong)cr_dsp.service.ShopNo, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // �X��
		wk2 = 2;
	}
	else if( cr_dsp.service.ShopNo != 0 ){
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[1] );			// "�|�����@�@�@"
		grachr( 3, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[3]+14 );		// "  �X��         "
		opedpl( 3, 6, (ulong)cr_dsp.service.ShopNo, 3, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF ); // �X��
		wk2 = 2;
	}
	if( memcmp( cr_dsp.service.StartDate, ck_dat, 6 ) != 0 ){
		// ��������
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );			// "  �L���J�n      �N    ��    ��"
		grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );			// "  �L���I��      �N    ��    ��"
		opedpl( 4, 12, (ulong)cr_dsp.service.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// �L���J�n�i�N�j�\��
		opedpl( 4, 18, (ulong)cr_dsp.service.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// �@�@�@�@�i���j�\��
		opedpl( 4, 24, (ulong)cr_dsp.service.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// �@�@�@�@�i���j�\��
		opedpl( 5, 12, (ulong)cr_dsp.service.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �L���I���i�N�j�\��
		opedpl( 5, 18, (ulong)cr_dsp.service.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �@�@�@�@�i���j�\��
		opedpl( 5, 24, (ulong)cr_dsp.service.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �@�@�@�@�i���j�\��
	}
	if( wk2 != 0 ){
		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.service.ParkNo = wk1;
				grachr( 2, ((wk2==2)?6:10), 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j	�\��
				break;
			}
		}
	}
}

static void PrepaidDsp( short page )
{
	char c;
	m_prepid *mag = (m_prepid*)&MAGred[MAG_ID_CODE];
	ushort	wk1;
	ulong	temp;
	struct	clk_rec		clk_wok;

	if( page == 0 ){
		cr_dsp.prepaid.Mno = (ushort)astoin( mag->pre_mno, 2 );
		cr_dsp.prepaid.IssueDate[0] = (uchar)astoin( mag->pre_sta, 2 );
		cr_dsp.prepaid.IssueDate[1] = (uchar)astoin( mag->pre_sta+2, 2 );
		cr_dsp.prepaid.IssueDate[2] = (uchar)astoin( mag->pre_sta+4, 2 );
		c = mag->pre_amo;
		if(( c > 0x30)&&( c < 0x3a )){
			cr_dsp.prepaid.SaleAmount = (ulong)(c-0x30) * 1000L;
		}else if(( c > 0x40)&&( c < 0x5b )){
			cr_dsp.prepaid.SaleAmount = ((ulong)(c-0x40) * 1000L) + 9000L;
		}else{
			cr_dsp.prepaid.SaleAmount = ((ulong)(c-0x60) * 1000L) + 35000L;
		}
		cr_dsp.prepaid.RemainAmount = astoinl( mag->pre_ram, 5 );
		cr_dsp.prepaid.LimitAmount = (ushort)astoinl( mag->pre_plm, 3 );
		cr_dsp.prepaid.CardNo = astoinl( mag->pre_cno, 5 );
	}else if( page == 1 ){
		if(MAGred[MAG_GT_APS_TYPE] == 1){
			temp  =  (long)mag->pre_pno[0] & 0x0000003F;
			temp |= ((long)mag->pre_pno[1] & 0x0000003F) << 6;
			temp |= ((long)mag->pre_pno[2] & 0x0000003F) << 12;
		}else{
			temp = astoin( mag->pre_pno, 2 );
			temp += ( mag->pre_pno[2]&0x0f ) * 100;
		}
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[3] );			// "�v���y�C�h�J�[�h"
		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.prepaid.ParkNo = wk1;
				grachr( 2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j	�\��
				break;
			}
		}
		grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[4] );			// "  �@�B��      ����            "
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[5] );			// "  �̔����z              �~    "
		grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[6] );			// "  �J�[�h�c�z            �~    "
		grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[7] );			// "  ���s      �N    ��    ��    "
		opedpl( 3, 8, (ulong)cr_dsp.prepaid.Mno, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 3, 18, (ulong)cr_dsp.prepaid.CardNo, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 4, 14, (ulong)cr_dsp.prepaid.SaleAmount, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 5, 14, (ulong)cr_dsp.prepaid.RemainAmount, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 8, (ulong)cr_dsp.prepaid.IssueDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 14, (ulong)cr_dsp.prepaid.IssueDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
		opedpl( 6, 20, (ulong)cr_dsp.prepaid.IssueDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );
	}else{
		grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[3] );			// "�v���y�C�h�J�[�h"
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[25] );		// "  ���x�z                   �~ ",
		opedpl( 3, 18, (ulong)cr_dsp.prepaid.LimitAmount, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
		displclr(4);
		if ( prm_get( COM_PRM,S_PRP,11,2,1 ) == 99 ){
			grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[27] );	// " (�L������           �����Ȃ�)",
		}else{
			grachr ( 5, 0, 30, 0, COLOR_BLACK,LCD_BLINK_OFF, CARDSTR2[26] );	// " (�L������     �N    ��    ��)",
			al_preck_Kigen ( (m_gtprepid*)MAGred, &clk_wok );
			opedpl ( 5, 10, (ulong)(clk_wok.year % 100), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
			opedpl ( 5, 16, (ulong)(clk_wok.mont),       2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
			opedpl ( 5, 22, (ulong)(clk_wok.date),       2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
		}
		displclr(6);
	}
}

static void PassDsp( void )
{
	ushort	wk1, wk2;

	ulong	temp;
	m_apspas *mag = (m_apspas*)&MAGred[MAG_ID_CODE];

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->aps_pno[0];
		wk2 = mag->aps_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17�i�[
		wk2 = 0;
	}else{
		wk1 = mag->aps_pno[0];
		wk2 = mag->aps_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// ���ԏ�No.���
		wk2 = 0;
	}
	cr_dsp.pass.Kind = mag->aps_pno[1] & 0x0f;
	wk2 = mag->aps_pcd[0];
	wk1 = mag->aps_pcd[1];
	cr_dsp.pass.Code = wk1 | (wk2<<7);
	cr_dsp.pass.StartDate[0] = mag->aps_sta[0];
	cr_dsp.pass.StartDate[1] = mag->aps_sta[1];
	cr_dsp.pass.StartDate[2] = mag->aps_sta[2];
	cr_dsp.pass.EndDate[0] = mag->aps_end[0];
	cr_dsp.pass.EndDate[1] = mag->aps_end[1];
	cr_dsp.pass.EndDate[2] = mag->aps_end[2];
	cr_dsp.pass.Status = mag->aps_sts;
	cr_dsp.pass.WriteDate[0] = mag->aps_wrt[0];
	cr_dsp.pass.WriteDate[1] = mag->aps_wrt[1];
	cr_dsp.pass.WriteDate[2] = mag->aps_wrt[2];
	cr_dsp.pass.WriteDate[3] = mag->aps_wrt[3];

	grachr( 2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[4] );								// "�����          "
	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "������"

		// �����ð��ٕ���������ꍇ

		for( wk1=0 ; wk1<=3 ; wk1++ ){
			if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
				cr_dsp.prepaid.ParkNo = wk1;
				grachr( 2, 6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j	�\��
				break;
			}
		}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[9] );								// "  ���        ����            "
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );								// "  �L���J�n      �N    ��    ��"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );								// "  �L���I��      �N    ��    ��"
	grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[10] );							// "            ��    ��    �F    "

	opedpl( 3, 6, (ulong)cr_dsp.pass.Kind, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );				// ���
	opedpl( 3, 18, (ulong)cr_dsp.pass.Code, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );				// ����
	opedpl( 4, 12, (ulong)cr_dsp.pass.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �L���J�n�i�N�j
	opedpl( 4, 18, (ulong)cr_dsp.pass.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j
	opedpl( 4, 24, (ulong)cr_dsp.pass.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j
	opedpl( 5, 12, (ulong)cr_dsp.pass.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �L���I���i�N�j
	opedpl( 5, 18, (ulong)cr_dsp.pass.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j
	opedpl( 5, 24, (ulong)cr_dsp.pass.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j

	switch( cr_dsp.pass.Status ){
		case 0:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[0] );						// "����"
			break;
		case 1:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[2] );						// "�o��"
			break;
		case 2:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[1] );						// "����"
			break;
		case 3:
			grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[3] );						// "���Z"
			break;
		default:
			if( cr_dsp.pass.Status == (uchar)prm_get( COM_PRM,S_KOU,5,2,1 ) ){
				grachr( 6, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR[4] );					// "�X�V"
			}
			break;
	}
	opedpl( 6, 8, (ulong)cr_dsp.pass.WriteDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// ���������i���j�\��
	opedpl( 6, 14, (ulong)cr_dsp.pass.WriteDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j�\��
	opedpl( 6, 20, (ulong)cr_dsp.pass.WriteDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j�\��
	opedpl( 6, 26, (ulong)cr_dsp.pass.WriteDate[3], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �@�@�@�@�i���j�\��
}

static void KaiDsp( void )
{
	ushort	wk1, wk2;
	ushort	line;
	ulong	temp;
	char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };

	m_kaisuutik *mag = (m_kaisuutik *)&MAGred[MAG_ID_CODE];

	if(MAGred[MAG_GT_APS_TYPE] == 1){
		temp = (long)mag->kaitik_pno[0];
		wk2 = mag->kaitik_pno[1] & 0xf0;
		temp |= (long)((wk2 & 0x0040)<<1);
		temp |= (long)((wk2 & 0x0020)<<3);
		temp |= (long)((wk2 & 0x0010)<<5);
		temp |= ((long)MAGred[MAG_EX_GT_PKNO] & 0x000000ff)<< 10L;	//P10-P17�i�[
		wk2 = 0;
	}else{
		wk1 = mag->kaitik_pno[0];
		wk2 = mag->kaitik_pno[1] & 0xf0;
		wk1 |= ((wk2 & 0x0040)<<1);
		wk1 |= ((wk2 & 0x0020)<<3);
		wk1 |= ((wk2 & 0x0010)<<5);
		temp = wk1;					// ���ԏ�No.���
		wk2 = 0;
	}

	cr_dsp.kaiticket.LimDosu		= mag->kaitik_pno[1] & 0x0f;		// �����x�����

	wk2 = mag->kaitik_tan[0];
	wk1 = mag->kaitik_tan[1];
	cr_dsp.kaiticket.TanRyo 		= wk1 | (wk2<<7);					// �P�ʗ��ྯ�

	cr_dsp.kaiticket.StartDate[0]	= mag->kaitik_sta[0];				// �L���J�n�i�N�j���
	cr_dsp.kaiticket.StartDate[1]	= mag->kaitik_sta[1];				// �@�@�@�@�i���j���
	cr_dsp.kaiticket.StartDate[2]	= mag->kaitik_sta[2];				// �@�@�@�@�i���j���

	cr_dsp.kaiticket.EndDate[0]		= mag->kaitik_end[0];				// �L���I���i�N�j���
	cr_dsp.kaiticket.EndDate[1]		= mag->kaitik_end[1];				// �@�@�@�@�i���j���
	cr_dsp.kaiticket.EndDate[2]		= mag->kaitik_end[2];				// �@�@�@�@�i���j���

	cr_dsp.kaiticket.Kaisu			= mag->kaitik_kai;					// �񐔁i�c��j���

	cr_dsp.kaiticket.WriteDate[0]	= mag->kaitik_wrt[0];				// ���������i���j���
	cr_dsp.kaiticket.WriteDate[1]	= mag->kaitik_wrt[1];				// �@�@�@�@�i���j���
	cr_dsp.kaiticket.WriteDate[2]	= mag->kaitik_wrt[2];				// �@�@�@�@�i���j���
	cr_dsp.kaiticket.WriteDate[3]	= mag->kaitik_wrt[3];				// �@�@�@�@�i���j���

	line = 2;															// �\���J�n�s
	grachr( line, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR1[5] );								// "�񐔌�          "
	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[MAGred[MAG_GT_APS_TYPE]] );			// "<*>"
	for( wk1=0 ; wk1<=3 ; wk1++ ){
		if( (long)temp == CPrmSS[S_SYS][1+wk1] ){
			cr_dsp.kaiticket.ParkNo = wk1;
			grachr( 2, 6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j	�\��
			break;
		}
	}
	line++;

	if( cr_dsp.kaiticket.TanRyo != 0 ){
		// �������񐔌�
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[11] );							// "  �c�^�����x��      ��^    ��"
		opedpl( line, 16, (ulong)cr_dsp.kaiticket.Kaisu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �񐔁i�c��j	�\��
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.LimDosu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �����x��		�\��
		line++;
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[13] );							// "  �P�ʋ��z          �~        "
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.TanRyo, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �P�ʋ��z	�\��
		line++;
	}
	else{
		// �������񐔌�
		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[12] );							// "  �c�x��        ��            "
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.Kaisu, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �񐔁i�c��j	�\��
		line++;
	}

	if( memcmp( cr_dsp.kaiticket.StartDate, ck_dat, 6 ) != 0 ){

		// �L����������

		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[1] );							// "  �L���J�n      �N    ��    ��"
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.StartDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �N	�\��
		opedpl( line, 18, (ulong)cr_dsp.kaiticket.StartDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// ��	�\��
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.StartDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// ��	�\��
		line++;

		grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[2] );							// "  �L���I��      �N    ��    ��"
		opedpl( line, 12, (ulong)cr_dsp.kaiticket.EndDate[0], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �N	�\��
		opedpl( line, 18, (ulong)cr_dsp.kaiticket.EndDate[1], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// ��	�\��
		opedpl( line, 24, (ulong)cr_dsp.kaiticket.EndDate[2], 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// ��	�\��
	}
}
// MH810100(S) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(�����e�i���X)		
//void CardErrDsp( void )	
void CardErrDsp( short err )															// �����v�Z�e�X�g��err���g���ăG���[�\���𕪊򂳂��Ă��܂��B
{
//	switch( OPECTL.CR_ERR_DSP )
	switch( err )
// MH810100(E) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(�����e�i���X)			
	{
		case 0:
			break;
		case 1:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[0] );				// " ���� ���ԏꇂ���Ⴂ�܂� ���� "
			break;
		case 2:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[12] );			// "   ���� �f�[�^�ُ�ł� ����   "
			break;
		case 3:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[2] );				// " �����@�@�����؂�ł��@�@���� "
			break;
		case 4:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[4] );				// " ����������o�ɃG���[�ł����� "
			break;
		case 5:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[3] );				// " �����@�����o�^����ł��@���� "
			break;
		case 6:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[1] );				// "     ���� �����O�ł� ����     "
			break;
		case 7:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[6] );				// " ����  �c�z������܂���  ���� "
			break;
		case 8:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[13] );			// "   ���� �g�p�s���ł� ����   "
			break;
		case 9:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[7] );				// "    ���� ��d�g�p�ł� ����    "
			break;
		case 10:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[8] );				// " ���� �}���������Ⴂ�܂� ���� "
			break;
		case 13:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[9] );				// "  �����@��ʂ��K��O�ł��@����"
			break;
		case 14:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[10] );			// "  �������x�����𒴂��Ă܂�����"
			break;
		case 20:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[14] );			// "   ���� �ǎ�ł��܂��� ����   "
			break;
		case 25:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[16] );			// "  ���� ���̎Ԏ�̌��ł� ����  "
			break;
		case 26:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[17] );			// " ���� �ݒ肳��Ă��܂��� ���� "
			break;
		case 27:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[18] );			// " ���� ������ʂ��Ⴂ�܂� ���� "
			break;
		case 28:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[19] );			// " ���� ���x�������O���ł� ���� "
			break;
		case 29:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[20] );			// "   ���� �⍇���ُ�ł� ����   "
			break;
		case 30:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[21] );			// " ���� ���Z���Ԃ��Ⴂ�܂� ���� "
			break;
		case 40:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[0] );			// �Z���G���[
			break;
		case 41:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[1] );			// �����G���[
			break;
		case 42:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[2] );			// �X�^�[�g�r�b�g�����G���[
			break;
		case 43:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[3] );			// �f�[�^�G���[
			break;
		case 44:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[4] );			// �p���e�B�G���[
			break;
		case 45:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[5] );			// �ǎ��b�q�b�G���[
			break;
		case 46:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[22] );			// "   ���� �ǎ�ł��܂��� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[6] );			// ���̑��̃G���[
			break;
		case 47:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[23] );			// "   ���� �f�[�^�ُ�ł� ����   "
			grachr( 1, 21, 4, 0, COLOR_RED, LCD_BLINK_OFF, MAGREADERRMSG[7] );			// CRC�G���[
			break;
		case 50:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[15] );			// "   ���� �X�V�����O�ł� ����   "
			break;
		case 33:
			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[24] );			// " �����J�[�h�ݒ肪�Ⴂ�܂����� "
			break;
		default:
			break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �g���@�\ ���j���[�\�z                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_ExtendMenuMake( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : �ƭ��\���̍��ڐ��A0=�ƭ�Open����K�v�Ȃ�                |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���[�U�����e�i���X���g���@�\���j���[��I�����ꂽ�ꍇ�A�q���j���[��     |*/
/*|	�\�z����															   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar Is_ExtendMenuMake( void )
{
	uchar	mno;		//���j���[������ԍ�
	uchar	ret;		//���j���[���ڐ�

	ret =0;
	mno =0;

	memset(&USM_ExtendMENU[0][0], 0, sizeof(USM_ExtendMENU));			//�\�z��i���j���[������j���N���A
	memset(&USM_ExtendMENU_TBL[0][0], 0, sizeof(USM_ExtendMENU_TBL));	//�\�z��i����e�[�u���j���N���A

// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	// �����䐔�W�v
//	if( (prm_get(COM_PRM, S_NTN, 26, 1, 2) == 1) &&			// �����W�v�⍇��������
//		(prm_get(COM_PRM, S_TOT,  2, 1, 1) == 1) &&			// �����䐔�W�v������
//		( (2 <= OPECTL.Mnt_lev) && (OPECTL.Mnt_lev <= 4) ) )// �������Ǘ��҈ȏ�
//	{
//		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//		ret += 1;
//	}
//
//	// �N���W�b�g����
//	mno += 1;
//	if( (prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) ||	// �ڼޯĻ��ް
//		(prm_get(COM_PRM, S_PAY, 24, 1, 2) == 3) )
//	{
//		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//		ret += 1;
//	}
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	// �d��������
//	mno += 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	// �r������������
//	mno += 1;
//
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i����ύX�j
////	if( prm_get (COM_PRM, S_PAY, 24, 1, 3) !=0 )
//	if( isSX10_USE() )
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i����ύX�j
//	{
//		if(prm_get(COM_PRM, S_SCA, 7, 2, 5) != 0)
//		{
//			Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
//			ret += 1;
//		}
//	}
//
//	// �@�l�J�[�h
//	mno += 1;
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

	// �l�s�W�v
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	mno += 1;
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	if( (prm_get ( COM_PRM, S_TOT, 1, 1, 1 ) != 0) &&	// MT�W�v����
		( (2 <= OPECTL.Mnt_lev) ))						// �������Ǘ��҈ȏ�
	{
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}

	// �d�����b�N����
	mno += 1;
	if(CPrmSS[S_PAY][17]) {							// �d�����b�N����̐ݒ�
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (�g���@�\�Ɍ��σ��[�_�����ǉ�)
	// ���σ��[�_����
	mno += 1;
	if( isEC_USE() ){		// ���σ��[�_�ڑ�
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (�g���@�\�Ɍ��σ��[�_�����ǉ�)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	// �d�q�W���[�i��
	mno += 1;
	if( isEJA_USE() ){		// �d�q�W���[�i���ڑ�����
		Is_ExtendMenuMakeTable(ret,mno,EXTEND_MENU);
		ret += 1;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	Ext_Menu_Max = ret;
	return	ret;
}

typedef struct{
	const uchar	(*num)[3];
	const uchar	(*str)[29];
	const ushort	(*Ctrl_table)[4];
}t_make_table_info;
static	const	t_make_table_info	make_table_info[] = {
	{MENU_NUMBER,	EXTENDMENU_BASE,	USM_EXT_TBL},	// �g���@�\
	{MENU_NUMBER,	KIND_3_MENU,		USM_KIND3_TBL},	// �Ԏ����j���[�i�R���j���[�p�j
	{MENU_NUMBER,	KIND_2_MENU,		USM_KIND_TBL},	// �Ԏ����j���[�i�Q���j���[�p�j
	{MENU_NUMBER,	FCNTMENUCOUNT,		FUNC_CNT_TBL2},	// ����J�E���g���j���[
	{MENU_NUMBER,	LOCKMENU,			USM_LOCK_TBL},	// ۯ����u�J���j���[
};

/*[]----------------------------------------------------------------------[]*/
/*| �g���@�\ ���j���[�e�[�u���\�z                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_ExtendMenuMakeTable( void )                          |*/
/*| PARAMETER    : ret=�\�z��̔z��ԍ�                                    |*/
/*|              : mno=�\�z���̔z��ԍ�                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	���[�U�����e�i���X���g���@�\���j���[��I�����ꂽ�ꍇ�A�q���j���[��     |*/
/*|	�\�z����															   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Is_ExtendMenuMakeTable(uchar ret, uchar mno, uchar type )
{
	const t_make_table_info *p = &make_table_info[type];
	
	//���j���[�̍��ڕ���������
	memcpy(&USM_ExtendMENU[ret][0], p->num[ret], sizeof(p->num[0]) );			//�ԍ�������
	memcpy(&USM_ExtendMENU[ret][2], p->str[mno], sizeof(p->str[0]) );			//�����������

	//���j���[�̐���e�[�u�������
	memcpy(&USM_ExtendMENU_TBL[ret][0], p->Ctrl_table[mno], sizeof(p->Ctrl_table[0]));
	USM_ExtendMENU_TBL[ret][2] = (ushort)ret;

}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��I�� ���j���[�\�z				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_CarMenuMake( void )     	     	 		           |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : ������|�C���^                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			                                       |*/
/*| Date         : 2009-06-09                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short Is_CarMenuMake( uchar type )
{
	uchar	mno=0;		//���j���[������ԍ�
	uchar	ret=0;		//���j���[���ڐ�
	short	menu=0;		//���j���[���ڐ�

	uchar	param;
	
	param = GetCarInfoParam();
	
	memset(&USM_ExtendMENU[0][0], 0, sizeof(USM_ExtendMENU));			//�\�z��i���j���[������j���N���A
	memset(&USM_ExtendMENU_TBL[0][0], 0, sizeof(USM_ExtendMENU_TBL));	//�\�z��i����e�[�u���j���N���A

	switch( type ){
		case	CAR_3_MENU:								// �Ԏ����j���[�i�R��ʗp�j
			// ����
			if( param & 0x04 )
			{
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				menu = MNT_INT_CAR;
			}

			// ���ԁiIF-3100/3150�j
			mno++;
			if( param & 0x02 )
			{
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				menu = MNT_CAR;
			}
			break;
		case	MV_CNT_MENU:							// ����J�E���g���j���[
			Is_ExtendMenuMakeTable(ret,mno,type);
			ret++;
			mno++;
			// no break;
		case	CAR_2_MENU:								// �Ԏ����j���[�i�Q��ʗp�j
			if( param & 0x06 ){
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;

				if(( param & 0x06 ) == 0x04 )
					menu = MNT_INT_CAR;
				else
					menu = MNT_CAR;
			}
			break;
		case	LOCK_CTRL_MENU:							// ���b�N�J���j���[
			if( param & 0x06 ){
				Is_ExtendMenuMakeTable(ret,mno,type);	// �t���b�v�㏸���~�i�ʁj
				ret++;
				mno++;

				Is_ExtendMenuMakeTable(ret,mno,type);	// �t���b�v�㏸���~�i�S�āj
				ret++;

				if(( param & 0x06 ) == 0x04 )
					menu = MNT_INT_CAR;
				else
					menu = MNT_CAR;
			}
	}
	// ����
	mno++;
	if( param & 0x01 )
	{
		switch( type ){
			case	MV_CNT_MENU:
				if( ret == 0 ){
					mno++; 
				}
				break;
			case	LOCK_CTRL_MENU:
				if( ret == 0 ){
					mno++; 
				}
				Is_ExtendMenuMakeTable(ret,mno,type);
				ret++;
				mno++;
				break;
		}
		Is_ExtendMenuMakeTable(ret,mno,type);
		ret++;
		menu = MNT_BIK;
	}

	Ext_Menu_Max = ret;
	return menu;
}

typedef struct{
	const uchar	(*str)[27];
	const ushort	(*Ctrl_table)[4];
	const ushort	max;
}t_make_menu;
static	const	t_make_menu	make_menu[] = {
	{FCMENU,	FUN_CHK_TBL,	FUN_CHK_MAX},
	{UMMENU,	USER_TBL,		USER_MENU_MAX},
// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�ڑ����A���σ��[�_�`�F�b�N�ǉ�)
// MH810100(S)
//	{FCMENU2,	FUN_CHK_TBL2,	FUN_CHK_MAX},
	{FCMENU2,	FUN_CHK_TBL2,	FUN_CHK_MAX2},
// MH810100(E)
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�ڑ����A���σ��[�_�`�F�b�N�ǉ�)
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
	{UMMENU_CC,	USER_TBL_CC,	USER_MENU_MAX_CC},
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
};

/*[]----------------------------------------------------------------------[]*/
/*| ���j���[�e�[�u���\�z 					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Is_MenuStrMake( void )	     	      		           |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : ������|�C���^                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			                                       |*/
/*| Date         : 2009-06-09                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ushort Is_MenuStrMake( uchar type )
{
	const t_make_menu	*p = &make_menu[type];
	// �Ƃ肠�����A�T�O���j���[���m�ہB����`�F�b�N�Ƌ��L�Ȃ̂ŁA�ǂ��炩�̃T�C�Y��
	// �T�O�𒴂�����v�ύX�B
	static uchar	MENU_STR[50][31];
	char	i;
	ushort	event;
	
	memset( MENU_STR[0], 0, sizeof( MENU_STR ));
	
	//���j���[�̍��ڕ���������
	for( i=0;i<p->max;i++ ){
		memcpy(&MENU_STR[i][0], MNT_MENU_NUMBER[i], sizeof(MNT_MENU_NUMBER[0]) );	//�ԍ�������
		memcpy(&MENU_STR[i][4], p->str[i], sizeof(p->str[0]) );				//�����������
	}
	
	event = Menu_Slt( (void*)MENU_STR, p->Ctrl_table, (char)p->max, (char)1 );

	return	event;
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*|  �Z���^�[�ʐM�������{���[�`��										   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Edy_CentComm_sub( uchar disp_type )	                   |*/
///*| PARAMETER    : uchar disp_type�i��ʕ\��1:����^0:�Ȃ��j               |*/
///*| 			 : uchar comm_type�i���ߏ���1:����^0:�Ȃ��j               |*/
///*| RETURN VALUE : void						                               |*/
///*| Date         : 2007-02-26                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//void	Edy_CentComm_sub( uchar disp_type, uchar comm_type )
//{
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 &&			// Edy���p�\��Ԃ��H
//		Edy_Rec.edy_EM_err.BIT.Comfail == 0 &&				// EM�ʐM�G���[���������Ă��Ȃ����H
//		Edy_Rec.edy_status.BIT.INITIALIZE ) {				// EdyӼޭ�ق̏��������������Ă��邩�H
//
//		auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);	// �ŏI�Z���^�[�ʐM�J�n�����X�V
//		Edy_SndData13(comm_type);							// �Z���^�[�ʐM�J�n�w���i���ߏ����̗L���j���M
//		if(disp_type ==1) {
//			dispclr();										// ������ʁu�Z���^�[�ʐM���v�\��
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[0]);					/* "���s�W�v���@�@�@�@�@�@�@�@�@�@" */
//			grachr(2, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);			/* "�@�@�d�����Z���^�[�ƒʐM���@�@" �i�_�ŕ\���j*/
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[9]);			/* " �ʐM���͑��̑��삪�ł��܂��� " */
//			Fun_Dsp(FUNMSG[0]);																/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
//			edy_cnt_ctrl.BIT.blink_status = 1;				// �u�����N�X�e�[�^�X�𔽓]�ɐݒ�
//			LagTim500ms( LAG500_EDY_LED_RESET_TIMER,		// �_�ŗpTimer�X�^�[�g(1sec)
//			 3, Edy_CentComm_Blink );						// EdyLED�p�̃^�C�}�[���g�p����
//		}
//		else if(disp_type == 2) {							// �����Z���^�[�ʐM��
//			if( OPECTL.Ope_mod == 0 ){							// �ҋ@��ʒ���1-3�s�ڂ̕\��������
//				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "�@�@					�@�@�@" */
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "�@�@					�@�@�@" */
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "�@�@					�@�@�@" */
//			}
//			grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);				/* "�@�@�d�����Z���^�[�ƒʐM���@�@" */
//			grachr(5, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, EDY_DATECHK_STR[13]);	/* "     ���΂炭���҂�������     " */
//		}
//		edy_cnt_ctrl.BIT.comm_type = comm_type;
//		edy_cnt_ctrl.BIT.disp_type = disp_type;	
//		edy_cnt_ctrl.BIT.exec_status = 1;	
//		Edy_Rec.edy_status.BIT.CENTER_COM_START = 1;		// �����ʐM�J�n		
//	} else {
//		wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
//	}
//	
//}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)


/*[]----------------------------------------------------------------------[]*/
/*| �A�}�m�W���J�[�h�\��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AmanoCard_Dsp( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static uchar AmanoCard_Dsp( void )
{
	uchar		gt_ap;
	m_kakari	*crd_data;											// ���C�����ް��߲��
	ulong		atend_no;
	ushort		wk;
	ulong		pk_no;

	crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];
	gt_ap = MAGred[MAG_GT_APS_TYPE];

	if (MAGred[MAG_GT_APS_TYPE] == 0 ){								//GT���Ή����[�_�[�ł��V�W���J�[�h�͓ǂ߂�.
		if ( crd_data->kkr_park[0] == 'P' && crd_data->kkr_park[1] == 'G' && crd_data->kkr_park[2] == 'T' ){ // PGT //
			gt_ap = 1;												//GT//
		}
	}

	/* �J�[�h��� */
	if( crd_data->kkr_type == 0x20 ) {
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[17] );						// "�W���J�[�h                    "
	} else if( crd_data->kkr_type == 0x4b ) {
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[18] );						// "�ޑK��[�J�[�h                "
	} else {
		return(1);													// �ȊO
	}

	if( gt_ap == 1 ){	//GT�t�H�[�}�b�g
		pk_no = (	( (crd_data->kkr_rsv2[4] & 0x0f) * 100000L ) +	// ���ԏ�m���D�擾
					( (crd_data->kkr_rsv2[5] & 0x0f) * 10000L )  +
					( (crd_data->kkr_pno[0] & 0x0f) * 1000L )  +
					( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
					( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
					( (crd_data->kkr_pno[3] & 0x0f) * 1L )
				);
	}else{				//APS�t�H�[�}�b�g
		pk_no = (	( (crd_data->kkr_pno[0] & 0x0f) * 1000L ) +		// ���ԏ�m���D�擾
					( (crd_data->kkr_pno[1] & 0x0f) * 100L )  +
					( (crd_data->kkr_pno[2] & 0x0f) * 10L )   +
					( (crd_data->kkr_pno[3] & 0x0f) * 1L )
				);
	}

	for( wk=0 ; wk<=3 ; wk++ ){
		if( (long)pk_no == CPrmSS[S_SYS][1+wk] ){
			grachr( 2, ((crd_data->kkr_type == 0x4b)?14:10), 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j	�\��
			break;
		}
	}

	grachr( 2, 27, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, GT_APS_MARK[gt_ap] );						// "<*>"

	/* �W��NO. */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[15] );		// "  �W��No. �F                  "
	atend_no = (( (crd_data->kkr_kno[0] & 0x0f) * 1000)  +						//1000�̂��擾
				( (crd_data->kkr_kno[1] & 0x0f) * 100 )	 +						//100�̌����擾
				( (crd_data->kkr_kno[2] & 0x0f) * 10 )	 +						//10�̌����擾
				( (crd_data->kkr_kno[3] & 0x0f) * 1 ));							//1�̌����擾
	opedpl( 3, 12, atend_no, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �W��NO �\��

	/* ���� */
	switch( crd_data->kkr_role ) {
		case '0':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[20] );					// "  �����@  �F������ʑ���      "
			break;
		case '1':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[21] );					// "  �����@  �F�W������          "
			break;
		case '2':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[22] );					// "  �����@  �F�Ǘ��ґ���        "
			break;
		case '3':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[23] );					// "  �����@  �F�Z�p������        "
			break;
		case '4':
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[24] );					// "  �����@  �F�J���ґ���        "
			break;
		default:
			break;
	}

	/* ���s�� */
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARDSTR2[16] );	// "  ���s��  �F    �N    ��    ��"
	atend_no = (( (crd_data->kkr_year[0] & 0x0f) * 10) +					//10�̌����擾
				( (crd_data->kkr_year[1] & 0x0f) * 1 ));					//1�̌����擾
	opedpl( 5, 12, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// ���s�i�N�j�\��
	atend_no = (( (crd_data->kkr_mon[0] & 0x0f) * 10)  +					//10�̌����擾
				( (crd_data->kkr_mon[1] & 0x0f) * 1 ));						//1�̌����擾
	opedpl( 5, 18, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �@�@�i���j�\��
	atend_no = (( (crd_data->kkr_day[0] & 0x0f) * 10)  +					//10�̌����擾
				( (crd_data->kkr_day[1] & 0x0f) * 1 ));						//1�̌����擾
	opedpl( 5, 24, atend_no, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	// �@�@�i���j�\��

	return(0);
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////[]----------------------------------------------------------------------[]
/////	@brief			�Z���^�[�ʐM��M����
////[]----------------------------------------------------------------------[]
/////	@param[in]		type 0:IBK(EM)����̎�M 1:�����i����̌Ăяo��
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 2008/12/08<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void	Edy_CentComm_Recv( uchar type )
//{
//	R_CENTER_TRAFFIC_RESULT_DATA	*p_rcv_data;		// �Z���^�[�ʐM���{���ʃf�[�^�߲��
//	R_CENTER_TRAFFIC_CHANGE_DATA	*p_rcv_center;		// �����ʐM�󋵕ω��ʒm�f�[�^�߲��
//	uchar	LimitTime;									// �����ʐM�I��Limit����
//	int		wk_OKosNG=-1;								// �����ʐM�I��(-1:������0:OK/1:NG)
//
//	if( !edy_cnt_ctrl.BIT.exec_status )
//		return;
//	
//	if( type ){											// �����i����̌Ăяo��
//		LimitTime = (uchar)prm_get(COM_PRM, S_SCA, 61, 2, 3);
//
//		if( Edy_Rec.edy_status.BIT.CENTER_COM_START && CneterComLimitTime > LimitTime+5 ){
//			if(edy_cnt_ctrl.BIT.exec_status >= 1) {		// �J�n�w���ȍ~�̂ݎ�t
//				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
//				wk_OKosNG = 1;							// �����ʐM�I��(0:OK/1:NG)
//				goto Edy_CentComm_sub_End;
//			}
//		}
//		return;											// �����i����̃R�[���̏ꍇ�́A�d����͏����͂��Ȃ�
//	}
//	switch( Edy_Rec.rcv_kind ){							// ��M�ް����
//
//		case	R_FIRST_STATUS:							// ������Ԓʒm ��M
//			if(edy_cnt_ctrl.BIT.exec_status >= 1) {			// �J�n�w���ȍ~�̂ݎ�t
//				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
//				wk_OKosNG = 1;							// �����ʐM�I��(0:OK/1:NG)
//			}
//			break;
//
//		case	R_CENTER_TRAFFIC_RESULT:				// �����ʐM�J�n���� ��M
//			p_rcv_data = (R_CENTER_TRAFFIC_RESULT_DATA *)&Edy_Rec.rcv_data;	// ��M�ް��߲��get
//			if(p_rcv_data->Rcv_Status[0] == 0x00) {		// ���퉞����
//				switch(p_rcv_data->Status) {
//
//					case 0x01:							// �J�n�w����t ����
//						if(edy_cnt_ctrl.BIT.exec_status == 1) {					// �J�n�w�����̂ݎ�t
//							edy_cnt_ctrl.BIT.exec_status = 2;					// �J�n��t��
//						}
//						break;
//
//					case 0x02:							// �ʐM����
//						if(edy_cnt_ctrl.BIT.exec_status >= 1) {					// �J�n�w���ȍ~�̂ݎ�t
//							edy_cnt_ctrl.BIT.exec_status = 3;					// �ʐM�I��
//							wmonlg( OPMON_EDY_CEN_STOP, 0, 1 );	// ���LOG�o�^�i�Z���^�[�ʐM�I��:OK�j
//							wk_OKosNG = 0;						// �����ʐM�I��(0:OK/1:NG)
//						}
//						break;
//
//					default:
//						break;
//				}
//			} else {									// �ُ퉞����
//				if(edy_cnt_ctrl.BIT.exec_status >= 1) {							// �J�n�w���ȍ~�̂ݎ�t
//					wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );		// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
//					wk_OKosNG = 1;							// �����ʐM�I��(0:OK/1:NG)
//				}
//			}
//			break;
//
//		case	R_CENTER_TRAFFIC_CHANGE:				// �����ʐM�󋵕ω��ʒm
//			if(edy_cnt_ctrl.BIT.disp_type !=1) break;
//
//			p_rcv_center = (R_CENTER_TRAFFIC_CHANGE_DATA *)&Edy_Rec.rcv_data;	// ��M�ް��߲��get
//			if(p_rcv_center->Rcv_Status[0] == 0x00) {
//				switch(p_rcv_center->Status_code) {
//					case 0x01:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[19]);		/* "�@�@�@�y�f�[�^�O�������z�@�@�@"*/
//						break;
//					case 0x02:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[10]);		/* "�@�@�@�y�R�l�N�V�������z�@�@�@"*/
//						break;
//					case 0x03:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[20]);		/* "�@�@�@�y�[���h�c���o���z�@�@�@"*/
//						break;
//					case 0x04:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[21]);		/* "�@�@�@�y�[���F�ؑ��o���z�@�@�@"*/
//						break;
//					case 0x05:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[22]);		/* "�@�@�@�y�Z�b�V�����J�n�z�@�@�@"*/
//						break;
//					case 0x06:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[23]);		/* "�@�@�@�y�f�[�^�W�M���z  �@�@�@"*/
//						break;
//					case 0x07:
//					case 0x10:
//					case 0x11:
//					case 0x12:
//					case 0x13:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[24]);		/* "�@�@�@�y�f�[�^�z�M���z  �@�@�@"*/
//						break;
//					case 0x08:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[25]);		/* "�@�@�@�y�Z�b�V�����I���z�@�@�@"*/
//						break;
//					case 0x09:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[26]);		/* "�@�@�@�y�N���[�Y�������z�@�@�@"*/
//						break;
//					case 0x0A:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[27]);		/* "�@�@�@�y�f�[�^�㏈�����z�@�@�@"*/
//						break;
//					case 0x0B:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[28]);		/* "�@�@�@�y�c�g�b�o�������z�@�@�@"*/
//						break;
//					default:
//						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "�@�@�@�@              �@�@�@�@"*/
//						break;
//				}
//			}
//			break;
//
//		default:
//			break;
//	}
//	
//Edy_CentComm_sub_End:
//	if( wk_OKosNG != -1 ){
//		if( edy_cnt_ctrl.BIT.disp_type == 1 ){
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	// �_�ŕ\���p�^�C�}�[�J��
//				dispclr();										// ��ʕ\������
//		}
//
//		if(edy_cnt_ctrl.BIT.disp_type == 2) {
//			if( OPECTL.Ope_mod == 0 ){						// �ҋ@��ʒ��Ŏ����Z���^�[�ʐM���s�����ꍇ
//				if( Edy_Rec.edy_status.BIT.ZAN_SW ){		// �c�z�Ɖ�̏ꍇ
//					Edy_StopAndLedOff();					// Edy�̒�~��LED_OFF
//					Edy_Rec.edy_status.BIT.ZAN_SW = 0;		// �c���Ɖ����݉����׸޾��
//				}
//				dispclr();									// ��ʕ\������ 
//				OpeLcd( 1 );								// �ҋ@��ʍĕ\��
//				Op_Event_enable();
//
//			}else if( OPECTL.Ope_mod == 100 ){
//				dispclr();									// ��ʕ\������ 
//				OpeLcd( 17 );								// �x�ƒ���ʍĕ\��
//				Op_Event_enable();
//			}else{											// ����ȊO�i�x�ƒ��j
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);
//			}
//		}
//
//		if(edy_cnt_ctrl.BIT.comm_type == 1) {				// �u�d�������߁v����ŃZ���^�[�ʐM�����{����
//			if(edy_cnt_ctrl.BIT.disp_type == 2){ 			// �u�����v�s���v��
//				Edy_Shime_Pri(1, (uchar)wk_OKosNG);			// ���O�ۑ���ڼ�Ĉ󎚏���
//			} else {										// �u�蓮�v�s���v��
//				Edy_Shime_Pri(0, (uchar)wk_OKosNG);			// ���O�ۑ���ڼ�Ĉ󎚏���
//			}
//		}
//
//		Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// �����ʐM�I������
//		CneterComLimitTime = 0;
//		edy_cnt_ctrl.SHORT = 0;								// �Z���^�[�ʐM�t���O�N���A
//		
//	}
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			�Z���^�[�ʐM���̃u�����N�\���ؑ�
////[]----------------------------------------------------------------------[]
/////	@param[in]		None
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 2008/12/08<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void	Edy_CentComm_Blink( void )
//{
//	if( edy_cnt_ctrl.BIT.disp_type ){
//		if(edy_cnt_ctrl.BIT.exec_status == 2) {				// �ʐM���͓_�ŕ\�������{����
//			edy_cnt_ctrl.BIT.blink_status ^= 1;
//			grachr(2, 0, 30, (ushort)edy_cnt_ctrl.BIT.blink_status, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);		/* "�@�@�d�����Z���^�[�ƒʐM���@�@" */
//			LagTim500ms( LAG500_EDY_LED_RESET_TIMER,		// �_�ŗpTimer�X�^�[�g(1sec)
//			 3, Edy_CentComm_Blink );						// EdyLED�p�̃^�C�}�[���g�p����
//		}
//	}
//}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind : �Ώۃv�����^
///	@return			1 = OK
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CheckPrinter(uchar pri_kind)
{
	switch(pri_kind) {
	case	R_PRI:
		if ((Ope_isPrinterReady() && Pri_Open_Status_R == 0)
			&& !IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {	// ���V�[�g�v�����^�ʐM�s��
			return 1;
		}
		break;
	case	RJ_PRI:
		if ((! Ope_isPrinterReady() || Pri_Open_Status_R != 0)
			|| IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {	// ���V�[�g�v�����^�ʐM�s��
			break;
		}
	// not break
	case	J_PRI:
		if ((Ope_isJPrinterReady() && Pri_Open_Status_J == 0)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)				// �d�q�W���[�i���ʐM�s��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iE2204�������Ƀ��V�[�g��T���v���󎚂���Ȃ��j
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE)
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)
			&& !IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR)
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iE2204�������Ƀ��V�[�g��T���v���󎚂���Ȃ��j
			&& !IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM)) {	// �W���[�i���v�����^�ʐM�s��
			return 1;
		}
		break;
	default:
//	case	0:
		return 1;		// �󎚎w��Ȃ��͏��OK
		break;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����g���ʃ`�F�b�N�i�T�u�j
//[]----------------------------------------------------------------------[]
///	@param[in]		no : �Ώۃv�����^�i0/1�j
///	@return			PRI_NML_END�^PRI_CSL_END�^PRI_ERR_END
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	int	check_print_result(int no)
{
	int		ret;
	ret = OPECTL.PriEndMsg[no].BMode;
	if (ret == PRI_ERR_END) {
		if (OPECTL.PriEndMsg[no].BStat == PRI_ERR_STAT) {
//			if ((OPECTL.PriEndMsg[no].BPrinStat & 0x08) == 0) {
//			// �w�b�h���x�ُ�ȊO�͎��؂�ƌ��Ȃ�
			if (OPECTL.PriEndMsg[no].BPrinStat & 0x0e) {
			// �j�A�G���h�ȊO�͎��؂�Ƃ���
				ret = PRI_NO_PAPER;
			}
		}
	}
	return ret;
}

// MH810100(S) Y.Yamauchi 2019/12/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
///	@brief			�����̌����𑪂�
//[]----------------------------------------------------------------------[]
///	@return			�����̌���
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/14<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort get_digit( ulong data )
{
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//	ushort length = 1;
//
//	while( data != 0){
//		data = data / 10;
//		++length;
//	}
	ushort length = 0;

	do {
		data = data / 10;
		++length;
	} while ( data != 0 );
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P

	return length;
}
//[]----------------------------------------------------------------------[]
///	@brief			QR�f�[�^�X�V����
//[]----------------------------------------------------------------------[]
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/14<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void push_ticket( tMediaDetail *MediaDetail, ushort err )
{
	// �ŐV�f�[�^��O��f�[�^�ɂ���
	ticketdata.backdata[4] = ticketdata.backdata[3];			// �ߋ�����5�Ԗڂ̃f�[�^���ߋ�����4�Ԗڂ̃f�[�^�ɃR�s�[
	ticketdata.backdata[3] = ticketdata.backdata[2];			// �ߋ�����4�Ԗڂ̃f�[�^���ߋ�����3�Ԗڂ̃f�[�^�ɃR�s�[
	ticketdata.backdata[2] = ticketdata.backdata[1];			// �ߋ�����3�Ԗڂ̃f�[�^���ߋ�����2�Ԗڂ̃f�[�^�ɃR�s�[
	ticketdata.backdata[1] = ticketdata.backdata[0];			// �ŐV�f�[�^��O��f�[�^�ɃR�s�[

	// �ŐV�f�[�^�ɍ��ǂ�QR�f�[�^���Z�b�g
	memcpy( &ticketdata.backdata[0].QR_Data, (uchar*)&MediaDetail->Barcode.QR_data, sizeof(ticketdata.backdata[0].QR_Data) );

	ticketdata.backdata[0].id = MediaDetail->Barcode.id;		// �f�[�^ID���Z�b�g(10000=QR���㌔ / 10001=QR������)
	ticketdata.backdata[0].err = err;							// �G���[���Z�b�g
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ticketdata.backdata[0].type = MediaDetail->Barcode.qr_type;	// QR�t�H�[�}�b�g�^�C�v���Z�b�g(1=�W�� / 2=�ʋ���)
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j

	if( ticketdata.cnt < 5 ){
		ticketdata.cnt++;	// �f�[�^�����X�V
	}
}
// MH810100(S) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
//[]----------------------------------------------------------------------[]
///	@brief			QR �G���[�\��
//[]----------------------------------------------------------------------[]
///	@param[in]		result		: QR�ǎ挋�ʉ���
///	@return			ret         :�G���[�̕\��
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// static uchar	Lcd_QR_ErrDisp( ushort result )
uchar Lcd_QR_ErrDisp( ushort result )
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
{
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:533)�Ή�
//	uchar ret;
	uchar ret = RESULT_NO_ERROR;
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:533)�Ή�

	switch ( result )
	{
	case 0x04:
// MH810100(S) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
		ret = RESULT_BAR_FORMAT_ERR;	// �t�H�[�}�b�g�G���[
// MH810100(E) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
		break;
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	case 0x01:
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	case 0x05:
		ret = RESULT_BAR_ID_ERR;		// �ΏۊO
		break;
	default:
		break;
	}
	return ret;
}

// MH810100(E) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
//[]----------------------------------------------------------------------[]
///	@brief			QR ID�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: �����ڂ̃f�[�^��
///	@return			ID���
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static ushort	pop_ticketID( int no )
{
	return ticketdata.backdata[no].id;
}

//[]----------------------------------------------------------------------[]
///	@brief			QR�f�[�^�G���[�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: �����ڂ̃f�[�^��
///	@return			�G���[���
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort	pop_ticketErr( int no )
{
	return ticketdata.backdata[no].err;
}
//[]----------------------------------------------------------------------[]
///	@brief			QR�f�[�^�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: �����ڂ̃f�[�^��
///	@return			QR�f�[�^���
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void*	pop_ticketData( int no )
{
	return &ticketdata.backdata[no].QR_Data;
}

// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			QR�t�H�[�}�b�g�^�C�v�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		no		: �����ڂ̃f�[�^��
///	@return			QR�t�H�[�}�b�g�^�C�v
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2023/01/10<br>
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
static ushort	pop_ticketType( int no )
{
	return ticketdata.backdata[no].type;
}
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
///[]----------------------------------------------------------------------[]
/// @brief        QR�G���[�f�[�^�\��  
//[]----------------------------------------------------------------------[]
/// @param[in]	 :  int page : �����ڂ̃f�[�^
///					int err  : �G���[���e
//[]----------------------------------------------------------------------[]
/// @return      : void
//[]----------------------------------------------------------------------[]
/// @date    	 Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// static void  UsMnt_QR_ErrDisp( int err )
void UsMnt_QR_ErrDisp( int err )
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
{
	switch ( err )
	{
	case RESULT_BAR_USED:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[1] );	// " �@�����g�p�σo�[�R�[�h�����@ "
		break;

	case RESULT_BAR_READ_MAX:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[2] );	// "�@�@�@ ��������������� �@�@�@"
		break;

	case RESULT_BAR_EXPIRED:
		grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[3] );	//"�@�@�����L�������͈͊O�����@�@"
		break;

// ����QR�̃t�H�[�}�b�g���������Ȃ����߁A���g��\�������Ȃ�

	case RESULT_BAR_FORMAT_ERR:
// MH810100(S) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//		grachr( 3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[4] );	// " �@�@�@�t�H�[�}�b�g�s���@�@�@ "
		grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[4]);	// " �@�����t�H�[�}�b�g�s�������@ "
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[19]);			// "�t�H�[�}�b�g���������Ȃ����߁@"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[21]);			// "���e��\���ł��܂���@�@�@�@�@"
		dispmlclr(4, 1);
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[22]);			// "�ǂݎ�����p�q�R�[�h���ݒ�Ɂ@"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);				// "��肪�������m�F���ĉ������@�@"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// MH810100(E) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
		break;

	case RESULT_BAR_ID_ERR:
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//		grachr( 3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[5] );	// " �@�@�@�ΏۊO�o�[�R�[�h�@�@�@ "
		grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[5]);	// " �@�����ΏۊO�o�[�R�[�h�����@ "
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[20]);			// "�ΏۊO�̂p�q�R�[�h��ǂ񂾂���"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[21]);			// "���e��\���ł��܂���@�@�@�@�@"
		dispmlclr(4, 1);
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[22]);			// "�ǂݎ�����p�q�R�[�h���ݒ�Ɂ@"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);				// "��肪�������m�F���ĉ������@�@"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
		break;
	
	default:
		break;
	}

}
///[]----------------------------------------------------------------------[]
/// @brief          QR�f�[�^�\��
//[]----------------------------------------------------------------------[]
///	@param[in]	slide_page		: ���ړ�
///	@param[in]	updown_page		: �㉺�ړ�
///	@param[in]	slide_pagemax	: ���ړ��ő�y�[�W��
///	@param[in]	updown_pagemax	: �㉺�ړ��ő�y�[�W��
//[]----------------------------------------------------------------------[]
/// @return      : MOD_CHG : mode change
///              : MOD_EXT : F5 key<br>
//[]----------------------------------------------------------------------[]
/// @date    	 Create	: 2020/01/12<br>
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// static void UsMnt_QR_DataDisp( ushort slide_page, ushort updown_page , ushort slide_pagemax)
static void UsMnt_QR_DataDisp( ushort slide_page, ushort updown_page , ushort slide_pagemax, ushort updown_pagemax)
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
{
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//	uchar	i;							// �\���p�̃J�E���^
//	ushort	max = 0;
//	uchar	top = (updown_page * 5);	// �P�y�[�W�̍ő�\����
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
	QR_AmountInfo*	 pAmntQR = NULL;	// QR�f�[�^��� QR���㌔
	QR_DiscountInfo* pDisQR = NULL;		// QR�f�[�^��� QR������
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	QR_TicketInfo*	 pTicQR = NULL;		// QR�f�[�^��� QR���Ԍ�
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//	uchar	temp[10];					// �o�C�i������Ascii�ɕϊ��p
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
	uchar	err;						// �G���[�\���p
	uchar	size;
	uchar	len;
	uchar	pos;
	ushort	_slide_page = slide_page + 1;
// MH810100(S) 2020/09/17 #4869�yQR�f�[�^�m�F��ʂŁA���ԏ�i���o�[���\������Ă���
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//	ushort  wk1 = 0;
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// MH810100(E) 2020/09/17 #4869�yQR�f�[�^�m�F��ʂŁA���ԏ�i���o�[���\������Ă���

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	dispmlclr( 1, 6 );
//	// �������H
//	if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page)) ){
//		pDisQR = (QR_DiscountInfo*)pop_ticketData( slide_page );
//	}
//	// ���㌔�H
//	else{
//		pAmntQR = (QR_AmountInfo*)pop_ticketData( slide_page );
//	}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

	// �G���[�擾
	err = pop_ticketErr( slide_page );
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	if ( err != RESULT_NO_ERROR || err != RESULT_QR_INQUIRYING ){
//		UsMnt_QR_ErrDisp( err );
//	}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_TITLE[0] );	// 	"�p�q�f�[�^�m�F�@�@�@�@�@�@�@�@"	
	size = get_digit(( ulong ) slide_pagemax );
	len = get_digit(( ulong ) _slide_page );
	pos = 30 - (size*2);
	opedsp( 0, pos, slide_pagemax, size, 0, 0, COLOR_BLACK, 0);
	grachr( 0,(pos -2), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "�^" );
	opedsp( 0, ((pos -2) -(len*2)), _slide_page, len, 0, 0, COLOR_BLACK, 0);
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	dispmlclr(1, 6);
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//	Fun_Dsp(FUNMSG[124]);													// 	"  ��    ��    ��    ��   �I�� ",
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�

	if( err != RESULT_BAR_ID_ERR && err!= RESULT_BAR_FORMAT_ERR ){				// �ΏۊO,�t�H�[�}�b�g�s����ID�ȊO�̏ꍇ
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//		for( i = 0; i < 5 ; i++ ){
//			// ������
//			if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page))){
//// MH810100(S) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
////				max = QR_DISCOUNT_DATA_MAX - 1;
//				max = QR_DISCOUNT_DATA_MAX;
//// MH810100(E) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
//				if( max > top + i ){
//					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[top+i] );	// 3�s�ڂ���e���ڕ\��
//				}
//			}
//			// ���㌔
//			else{
//				max = QR_AMOUNT_HEAD + 1;
//				max += (pAmntQR->DataCount * (BAR_DATA_OFFSET+1));
//				if( max > top + i ){
//					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[top+i] );		// 3�s�ڂ���e���ڕ\��
//				}
//			}
//		}
//
//		switch( pop_ticketID(slide_page) ){
//			// QR���㌔
//			case BAR_ID_AMOUNT:
//				switch( updown_page ){
//					case 0:
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Year,4);	// ���s�N
//						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Mon,2);	// ���s��
//						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Day,2);	// ���s��
//						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Hour,2);	// ���s��
//						grachr(  3, 18, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Min,2);	// ���s��
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pAmntQR->IssueDate.Sec,2);	// ���s�b
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						opedpl3( 4, (30-(get_digit( pAmntQR->FacilityCode ))), pAmntQR->FacilityCode, get_digit( pAmntQR->FacilityCode ), 0, 0,COLOR_BLACK,0 );	// �{�݃R�[�h
//						opedpl3( 5, (30- get_digit( pAmntQR->ShopCode )), pAmntQR->ShopCode, get_digit( pAmntQR->ShopCode ) , 0, 0,COLOR_BLACK,0 ); // �X�܃R�[�h
//						opedpl3( 6, (30-(get_digit( pAmntQR->PosNo ))), pAmntQR->PosNo, get_digit( pAmntQR->PosNo ), 0, 0,COLOR_BLACK,0 );	// �X�ܓ��[���ԍ�
//						break;
//					case 1:
//						opedpl3( 2, (30- get_digit( pAmntQR->IssueNo )), pAmntQR->IssueNo, get_digit( pAmntQR->IssueNo ) , 0, 0,COLOR_BLACK,0 );	// ���V�[�g���s�ǔ�
//						if ( (ulong)pAmntQR->DataCount == 1 ){
//							opedpl3( 3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0,COLOR_BLACK,0 );	// �o�[�R�[�h�^�C�v�P
//							opedpl3( 4, (30-(get_digit( pAmntQR->BarData[0].data ))), (ulong)pAmntQR->BarData[0].data, get_digit( pAmntQR->BarData[0].data ), 0, 0,COLOR_BLACK,0 );	// �f�[�^�P
//						} else {
//							opedpl3( 3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0,COLOR_BLACK,0 );	// �o�[�R�[�h�^�C�v�P
//							opedpl3( 4, (30-( get_digit( pAmntQR->BarData[0].data ))), (ulong)pAmntQR->BarData[0].data, get_digit( pAmntQR->BarData[0].data ), 0, 0,COLOR_BLACK,0 );	// �f�[�^�P
//							opedpl3( 5, 28, (ulong)pAmntQR->BarData[1].type, 2, 0, 0,COLOR_BLACK,0 );	// �o�[�R�[�h�^�C�v�Q
//							opedpl3( 6, (30-(get_digit( pAmntQR->BarData[1].data))), (ulong)pAmntQR->BarData[1].data, get_digit( pAmntQR->BarData[1].data), 0, 0,COLOR_BLACK,0 );	// �f�[�^�Q
//						}
//						break;
//					case 2:
//						if( (ulong)pAmntQR->DataCount == 3 ){
//							opedpl3( 2, 28, (ulong)pAmntQR->BarData[2].type, 2, 0, 0,COLOR_BLACK,0 );	// �o�[�R�[�h�^�C�v�R
//							opedpl3( 3, (30-(get_digit( pAmntQR->BarData[2].data ))), (ulong)pAmntQR->BarData[2].data, get_digit( pAmntQR->BarData[2].data ), 0, 0,COLOR_BLACK,0 );	// �f�[�^�R
//						}
//						break;
//					default:
//						break;
//				}
//				break;
//
//			// QR������
//			case BAR_ID_DISCOUNT:
//				switch( updown_page ){
//					case 0:
//						// TODO :�x�^�����ł͂Ȃ����@��T��
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Year,4);		// ���s�N
//						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Mon,2);		// ���s��
//						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Day,2);		// ���s��
//						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(S) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
////						opedpl3( 3, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// ���ԏ�ԍ�
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// �������
////						opedpl3( 5, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // �����敪
////						opedpl3( 3, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// ���ԏ�ԍ�
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// �������
////						opedpl3( 5, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // �����敪
////						opedpl3( 6, (30-(get_digit( pDisQR->ShopNp ))), pDisQR->ShopNp, get_digit( pDisQR->ShopNp ), 0, 0,COLOR_BLACK,0 );	// �X�ԍ�
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Hour,2);	// ���s��
//						grachr(  3, 18, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Min,2);	// ���s��
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->IssueDate.Sec,2);	// ���s�b
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(S) 2020/09/17 #4869�yQR�f�[�^�m�F��ʂŁA���ԏ�i���o�[���\������Ă���
////						opedpl3( 4, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->ParkingNo, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// ���ԏ�ԍ�
//						for( wk1=0 ; wk1<=3 ; wk1++ ){
//							if( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+wk1] ){
//								break;
//							}
//						}
//						grachr( 4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[wk1] );	// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�^  �j	�\��
//// MH810100(E) 2020/09/17 #4869�yQR�f�[�^�m�F��ʂŁA���ԏ�i���o�[���\������Ă���
//						opedpl3( 5, (30-(get_digit( pDisQR->ParkingNo ))), pDisQR->DiscKind, get_digit( pDisQR->ParkingNo ) , 0, 0,COLOR_BLACK,0 );	// �������
//						opedpl3( 6, (30-(get_digit( pDisQR->DiscClass ))), pDisQR->DiscClass, get_digit( pDisQR->DiscClass ), 0, 0,COLOR_BLACK,0 ); // �����敪
//// MH810100(E) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
//						break;
//					case 1:
//// MH810100(S) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Year,4);		// �L���J�n�N
////						grachr(  2, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Mon,2);		// �L���J�n��
////						grachr(  2, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Day,2);		// �L���J�n��
////						grachr(  2, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Year,4);		// �L���I���N
////						grachr(  3, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Mon,2);		// �L���I����
////						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
////						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Day,2);		// �L���I����
////						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						opedpl3( 2, (30-(get_digit( pDisQR->ShopNp ))), pDisQR->ShopNp, get_digit( pDisQR->ShopNp ), 0, 0,COLOR_BLACK,0 );	// �X�ԍ�
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Year,4);		// �L���J�n�N
//						grachr(  3, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Mon,2);		// �L���J�n��
//						grachr(  3, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->StartDate.Day,2);		// �L���J�n��
//						grachr(  3, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Year,4);		// �L���I���N
//						grachr(  4, 16, 4, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Mon,2);		// �L���I����
//						grachr(  4, 22, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//						intoas((uchar *)temp,(ushort)pDisQR->EndDate.Day,2);		// �L���I����
//						grachr(  4, 26, 2, 0,COLOR_BLACK,LCD_BLINK_OFF,(const uchar*)temp );
//// MH810100(E) S.Takahashi 2020/02/14 �������ɔ��s�����b��ǉ�
//// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//						opedpl3( 5, (30-(get_digit( pDisQR->FacilityCode ))), pDisQR->FacilityCode, get_digit( pDisQR->FacilityCode ), 0, 0,COLOR_BLACK,0 );	// �{�݃R�[�h
//						opedpl3( 6, (30- get_digit( pDisQR->ShopCode )), pDisQR->ShopCode, get_digit( pDisQR->ShopCode ) , 0, 0,COLOR_BLACK,0 );				// �X�܃R�[�h
//// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//						break;
//// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//					case 2:
//						opedpl3( 2, (30-(get_digit( pDisQR->PosNo ))), pDisQR->PosNo, get_digit( pDisQR->PosNo ), 0, 0,COLOR_BLACK,0 );			// �X�ܓ��[���ԍ�
//						opedpl3( 3, (30- get_digit( pDisQR->IssueNo )), pDisQR->IssueNo, get_digit( pDisQR->IssueNo ) , 0, 0,COLOR_BLACK,0 );	// ���V�[�g���s�ǔ�
//						break;
//// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//					default:
//						break;
//				}
//				break;
//
//			default:
//				break;
//		}
		switch( pop_ticketID(slide_page) ) {
		case BAR_ID_DISCOUNT:	// ������
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
			pDisQR = (QR_DiscountInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[1]);	// "�F������"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//			UsMnt_QR_DispDisQR(pDisQR, updown_page);
			UsMnt_QR_DispDisQR(pDisQR, updown_page, pop_ticketType(slide_page));
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
			break;
		case BAR_ID_AMOUNT:		// ���㌔
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
			pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[0]);	// "�F���㌔"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//			UsMnt_QR_DispAmntQR(pAmntQR, updown_page);
			UsMnt_QR_DispAmntQR(pAmntQR, updown_page, pop_ticketType(slide_page));
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
			break;
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
		case BAR_ID_TICKET:		// ���Ԍ�
			pTicQR = (QR_TicketInfo*)pop_ticketData(slide_page);
			grachr(0, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[2]);	// "�F���Ԍ�"
			UsMnt_QR_DispTicQR(pTicQR, updown_page);
			break;
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
		default:
			break;
		}
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
	}

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	UsMnt_QR_ErrDisp(err);
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
	if ( (pDisQR != NULL) || (pAmntQR != NULL) ) {	// �������A���㌔
		if ( updown_page == 0 ) {
			Fun_Dsp(FUNMSG[126]);											// "        ��    ��    ��   �I�� "
		} else if ( updown_page >= updown_pagemax ) {
			Fun_Dsp(FUNMSG[127]);											// "  ��          ��    ��   �I�� "
		} else {
			Fun_Dsp(FUNMSG[124]);											// "  ��    ��    ��    ��   �I�� "
		}
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	} else if ( pTicQR != NULL ) {	// ���Ԍ�
		// �\�����e��1�y�[�W�̂�
		Fun_Dsp(FUNMSG[128]);												// "              ��    ��   �I�� "
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	} else {										// �G���[
		Fun_Dsp(FUNMSG[128]);												// "              ��    ��   �I�� "
	}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
}
///[]----------------------------------------------------------------------[]
/// @brief	�㉺�ړ��̃y�[�W�̍ő吔
///[]----------------------------------------------------------------------[]
///	@param[in]	slide_page		: ���ړ�
/// @return 	updown_page_max	: �㉺�ړ��̃y�[�W�̍ő吔
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// ushort	UsMnt_QR_CardChk( QR_AmountInfo*  pAmntQR, ushort slide_page)
ushort UsMnt_QR_CardChk(ushort slide_page)
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
{
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//	ushort updown_pagemax;
//
//	if( BAR_ID_DISCOUNT == (pop_ticketID(slide_page))){
//// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
////		updown_pagemax = 1;
//		updown_pagemax = 2;		// �y�[�W���g��(2->3)
//// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//	} else {
//		pAmntQR = (QR_AmountInfo*)pop_ticketData( slide_page );
//		if( pAmntQR->DataCount == 3 ){
//			updown_pagemax = 2;
//		} else {
//			updown_pagemax = 1;
//		}
//	}
//	return updown_pagemax;
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//	pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
	QR_AmountInfo *pAmntQR = (QR_AmountInfo*)pop_ticketData(slide_page);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
	return UsMnt_QR_GetUpdownPageMax(pop_ticketID(slide_page), pAmntQR);
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
}

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//[]----------------------------------------------------------------------[]
/// @brief		�����\��
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: �\���s
///	@param[in]	col			: �\����
///	@param[in]	dat			: �f�[�^
///	@param[in]	siz			: �T�C�Y
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispNum(ushort low, ushort col, ulong dat, ushort siz)
{
	opedpl3(low, col, dat, siz, 1, 0, COLOR_BLACK, 0);
}

//[]----------------------------------------------------------------------[]
/// @brief		�����\��(�E��)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: �\���s
///	@param[in]	dat			: �f�[�^
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispNumRight(ushort low, ulong dat)
{
	ushort siz = get_digit(dat);
	opedpl3(low, (30-siz), dat, siz, 0, 0, COLOR_BLACK, 0);
}

//[]----------------------------------------------------------------------[]
/// @brief		QR�������̕\��(�������)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: �\���s
///	@param[in]	pDisQR		: QR�f�[�^(QR������)
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispDisQR_DiscKind(ushort low, QR_DiscountInfo *pDisQR)
{
	switch ( pDisQR->DiscKind ) {
	case 1:		// �T�[�r�X��(���z)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[0]);
		break;
	case 2:		// �X����(���z)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[1]);
		break;
	case 3:		// ���X�܊���(���z)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[2]);
		break;
	case 4:		// ������(���z)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[3]);
		break;
	case 50:	// ��ʊ���
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[4]);
		break;
	case 101:	// �T�[�r�X��(����)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[5]);
		break;
	case 102:	// �X����(����)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[6]);
		break;
	case 103:	// ���X�܊���(����)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[7]);
		break;
	case 104:	// ������(����)
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[8]);
		break;
	case 150:	// �g�p�֎~
		grachr(low, 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, DISC_KIND_STR[9]);
		break;
	default:	// ���̑�
		// ���l�����̂܂ܕ\��
		UsMnt_QR_DispNumRight(low, pDisQR->DiscKind);
		break;
	}
}

//[]----------------------------------------------------------------------[]
/// @brief		QR�������̕\��(�����敪)
//[]----------------------------------------------------------------------[]
///	@param[in]	low			: �\���s
///	@param[in]	pDisQR		: QR�f�[�^(QR������)
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/04/01
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void UsMnt_QR_DispDisQR_DiscClass(ushort low, QR_DiscountInfo *pDisQR)
{
	int is_disp_num = 0;

	switch ( pDisQR->DiscKind ) {
	case 1:		// �T�[�r�X��(���z)
	case 50:	// ��ʊ���
	case 101:	// �T�[�r�X��(����)
		if ( pDisQR->DiscClass < _countof(DAT2_7) ) {
			grachr(low, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[pDisQR->DiscClass]);
		} else {
			is_disp_num = 1;	// ���l�����̂܂ܕ\��
		}
		break;
	case 2:		// �X����(���z)
	case 102:	// �X����(����)
		switch ( pDisQR->DiscClass ) {
		case 0:		// �X����
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[0]);
			break;
		case 1:		// �g�p�֎~
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[1]);
			break;
		case 2:		// �e�i���g����
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[2]);
			break;
		case 3:		// ���L�Ҋ���
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[3]);
			break;
		case 4:		// ���������i�ꗥ�j
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[4]);
			break;
		case 5:		// �Ԏ튄��
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[5]);
			break;
		case 6:		// �����ʒu����
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[6]);
			break;
		case 7:		// �F�؋@����
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[7]);
			break;
		case 100:	// �V�X�e����Q������
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[8]);
			break;
		case 101:	// �o�ɉ�������
			grachr(low, 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, DISCOUNT_TYPE_STR[9]);
			break;
		default:	// ���̑�
			is_disp_num = 1;	// ���l�����̂܂ܕ\��
			break;
		}
		break;
	case 3:		// ���X�܊���(���z)
	case 4:		// ������(���z)
	case 103:	// ���X�܊���(����)
	case 104:	// ������(����)
	case 150:	// �g�p�֎~
	default:	// ���̑�
		is_disp_num = 1;	// ���l�����̂܂ܕ\��
		break;
	}
	
	if ( is_disp_num ) {
		// ���l�����̂܂ܕ\��
		UsMnt_QR_DispNumRight(low, pDisQR->DiscClass);
	}
}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//[]----------------------------------------------------------------------[]
/// @brief		QR�������̕\��
//[]----------------------------------------------------------------------[]
///	@param[in]	pDisQR		: QR�f�[�^(QR������)
///	@param[in]	updown_page	: �㉺�ړ��̃y�[�W
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page)
void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page, ushort qrtype)
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
{
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	ushort i;
//	ushort top;
//	ushort max;
//	uchar temp[10];
//
//	top = updown_page * 5;
//	max = QR_DISCOUNT_DATA_MAX;
//
//	// 3�s�ڂ���e���ڕ\��
//	for( i = 0; i < 5 ; i++ ) {
//		if ( max > top + i ) {
//			grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,
//				QR_DISCOUNT_DATA_CHECK_STR[top+i]);
//		}
//	}
//
//	switch( updown_page ) {
//	case 0:
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Year, 4);		// ���s�N
//		grachr(2, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Mon, 2);			// ���s��
//		grachr(2, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Day, 2);			// ���s��
//		grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Hour, 2);		// ���s��
//		grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Min, 2);			// ���s��
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->IssueDate.Sec, 2);			// ���s�b
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		for ( i = 0; i <= 3; i++ ) {
//			if ( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
//				break;
//			}
//		}
//		grachr(4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// ���ԏ�i��{�^�g�P�^�g�Q�^�g�R�^�@�@�j
//		opedpl3(5, (30-(get_digit(pDisQR->DiscKind))), pDisQR->DiscKind,
//			get_digit(pDisQR->DiscKind), 0, 0, COLOR_BLACK, 0);			// �������
//		opedpl3(6, (30-(get_digit(pDisQR->DiscClass))), pDisQR->DiscClass,
//			get_digit(pDisQR->DiscClass), 0, 0, COLOR_BLACK, 0);		// �����敪
//		break;
//	case 1:
//		opedpl3(2, (30-(get_digit(pDisQR->ShopNp))), pDisQR->ShopNp,
//			get_digit(pDisQR->ShopNp), 0, 0, COLOR_BLACK, 0);			// �X�ԍ�
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Year, 4);		// �L���J�n�N
//		grachr(3, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Mon, 2);			// �L���J�n��
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->StartDate.Day, 2);			// �L���J�n��
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Year, 4);			// �L���I���N
//		grachr(4, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Mon, 2);			// �L���I����
//		grachr(4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pDisQR->EndDate.Day, 2);			// �L���I����
//		grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		opedpl3(5, (30-(get_digit(pDisQR->FacilityCode))), pDisQR->FacilityCode,
//			get_digit(pDisQR->FacilityCode), 0, 0, COLOR_BLACK, 0);		// �{�݃R�[�h
//		opedpl3(6, (30-get_digit(pDisQR->ShopCode)), pDisQR->ShopCode,
//			get_digit(pDisQR->ShopCode), 0, 0, COLOR_BLACK, 0);			// �X�܃R�[�h
//		break;
//	case 2:
//		opedpl3(2, (30-(get_digit(pDisQR->PosNo))), pDisQR->PosNo,
//			get_digit(pDisQR->PosNo), 0, 0, COLOR_BLACK, 0);			// �X�ܓ��[���ԍ�
//		opedpl3(3, (30-get_digit(pDisQR->IssueNo)), pDisQR->IssueNo,
//			get_digit(pDisQR->IssueNo), 0, 0, COLOR_BLACK, 0);			// ���V�[�g���s�ǔ�
//		break;
//	default:
//		break;
//	}
	ushort i;
	ushort j;
	ushort size = _countof(QR_DISCOUNT_DATA_CHECK_STR);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 7) && (j < size); i++, j++ ) {	// 1�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNum(2, 11, pDisQR->IssueDate.Year, 4);		// ���s�N
		UsMnt_QR_DispNum(2, 16, pDisQR->IssueDate.Mon, 2);		// ���s��
		UsMnt_QR_DispNum(2, 19, pDisQR->IssueDate.Day, 2);		// ���s��
		UsMnt_QR_DispNum(2, 22, pDisQR->IssueDate.Hour, 2);		// ���s��
		UsMnt_QR_DispNum(2, 25, pDisQR->IssueDate.Min, 2);		// ���s��
		UsMnt_QR_DispNum(2, 28, pDisQR->IssueDate.Sec, 2);		// ���s�b
		UsMnt_QR_DispNumRight(3, pDisQR->FacilityCode);			// �{�݃R�[�h
		UsMnt_QR_DispNumRight(4, pDisQR->ShopCode);				// �X�܃R�[�h
		UsMnt_QR_DispNumRight(5, pDisQR->PosNo);				// �X�ܓ��[���ԍ�
		UsMnt_QR_DispNumRight(6, pDisQR->IssueNo);				// ���V�[�g���s�ǔ�
		break;
	case 1:
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		if( qrtype != QR_FORMAT_CUSTOM ) {
		// �W���t�H�[�}�b�g
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		for( i = 2, j = 5; (i < 7) && (j < size); i++, j++ ) {	// 6�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		for ( i = 0; i <= 3; i++ ) {
			if ( (long)pDisQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
				break;
			}
		}
		grachr(2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// ���ԏ�i��{�^�g�P�^�g�Q�^�g�R�^�@�@�j
		UsMnt_QR_DispDisQR_DiscKind(3, pDisQR);							// �������
		UsMnt_QR_DispDisQR_DiscClass(4, pDisQR);						// �����敪
		UsMnt_QR_DispNumRight(5, pDisQR->ShopNp);						// �X�ԍ�
		UsMnt_QR_DispNum(6, 9, pDisQR->StartDate.Year, 4);				// �L���J�n�N
		UsMnt_QR_DispNum(6, 14, pDisQR->StartDate.Mon, 2);				// �L���J�n��
		UsMnt_QR_DispNum(6, 17, pDisQR->StartDate.Day, 2);				// �L���J�n��
		UsMnt_QR_DispNum(6, 20, pDisQR->EndDate.Year, 4);				// �L���I���N
		UsMnt_QR_DispNum(6, 25, pDisQR->EndDate.Mon, 2);				// �L���I����
		UsMnt_QR_DispNum(6, 28, pDisQR->EndDate.Day, 2);				// �L���I����
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		} else {
		// �ʋ��ʃt�H�[�}�b�g
		for( i = 2, j = 6; (i < 5) && (j < size); i++, j++ ) {	// 6�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DISCOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispDisQR_DiscKind(2, pDisQR);							// �������
		UsMnt_QR_DispDisQR_DiscClass(3, pDisQR);						// �����敪
		UsMnt_QR_DispNumRight(4, pDisQR->ShopNp);						// �X�ԍ�
		}
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		break;
	default:
		break;
	}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
}

//[]----------------------------------------------------------------------[]
/// @brief		QR���㌔�̕\��
//[]----------------------------------------------------------------------[]
///	@param[in]	pAmntQR		: QR�f�[�^(QR���㌔)
///	@param[in]	updown_page	: �㉺�ړ��̃y�[�W
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page)
void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page, ushort qrtype)
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
{
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	ushort i;
//	ushort top;
//	ushort max;
//	uchar temp[10];
//
//	top = updown_page * 5;
//	max = QR_AMOUNT_HEAD + 1;
//	max += (pAmntQR->DataCount * (BAR_DATA_OFFSET + 1));
//
//	// 3�s�ڂ���e���ڕ\��
//	for( i = 0; i < 5 ; i++ ) {
//		if ( max > top + i ) {
//			grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[top+i]);
//		}
//	}
//
//	switch( updown_page ) {
//	case 0:
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Year, 4);		// ���s�N
//		grachr(2, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Mon, 2);		// ���s��
//		grachr(2, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Day, 2);		// ���s��
//		grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Hour, 2);		// ���s��
//		grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Min, 2);		// ���s��
//		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		intoas((uchar*)temp, (ushort)pAmntQR->IssueDate.Sec, 2);		// ���s�b
//		grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)temp);
//		opedpl3(4, (30-(get_digit(pAmntQR->FacilityCode))), pAmntQR->FacilityCode,
//			get_digit(pAmntQR->FacilityCode), 0, 0, COLOR_BLACK, 0);	// �{�݃R�[�h
//		opedpl3(5, (30-get_digit(pAmntQR->ShopCode)), pAmntQR->ShopCode,
//			get_digit(pAmntQR->ShopCode), 0, 0, COLOR_BLACK, 0);		// �X�܃R�[�h
//		opedpl3(6, (30-(get_digit(pAmntQR->PosNo))), pAmntQR->PosNo,
//			get_digit(pAmntQR->PosNo), 0, 0, COLOR_BLACK, 0);			// �X�ܓ��[���ԍ�
//		break;
//	case 1:
//		opedpl3(2, (30-get_digit(pAmntQR->IssueNo)), pAmntQR->IssueNo,
//			get_digit(pAmntQR->IssueNo), 0, 0, COLOR_BLACK, 0);			// ���V�[�g���s�ǔ�
//		if ( (ulong)pAmntQR->DataCount == 1 ) {
//			opedpl3(3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0, COLOR_BLACK, 0);	// �o�[�R�[�h�^�C�v�P
//			opedpl3(4, (30-(get_digit(pAmntQR->BarData[0].data))), (ulong)pAmntQR->BarData[0].data,
//				get_digit(pAmntQR->BarData[0].data), 0, 0, COLOR_BLACK, 0);				// �f�[�^�P
//		} else {
//			opedpl3(3, 28, (ulong)pAmntQR->BarData[0].type, 2, 0, 0, COLOR_BLACK, 0);	// �o�[�R�[�h�^�C�v�P
//			opedpl3(4, (30-(get_digit(pAmntQR->BarData[0].data))), (ulong)pAmntQR->BarData[0].data,
//				get_digit(pAmntQR->BarData[0].data), 0, 0, COLOR_BLACK, 0);				// �f�[�^�P
//			opedpl3(5, 28, (ulong)pAmntQR->BarData[1].type, 2, 0, 0, COLOR_BLACK, 0);	// �o�[�R�[�h�^�C�v�Q
//			opedpl3(6, (30-(get_digit(pAmntQR->BarData[1].data))), (ulong)pAmntQR->BarData[1].data,
//				get_digit(pAmntQR->BarData[1].data), 0, 0, COLOR_BLACK, 0);				// �f�[�^�Q
//		}
//		break;
//	case 2:
//		if ( (ulong)pAmntQR->DataCount == 3 ) {
//			opedpl3(2, 28, (ulong)pAmntQR->BarData[2].type, 2, 0, 0, COLOR_BLACK, 0);	// �o�[�R�[�h�^�C�v�R
//			opedpl3(3, (30-(get_digit(pAmntQR->BarData[2].data))), (ulong)pAmntQR->BarData[2].data,
//				get_digit(pAmntQR->BarData[2].data), 0, 0, COLOR_BLACK, 0);				// �f�[�^�R
//		}
//		break;
//	default:
//		break;
//	}
	ushort i;
	ushort j;
	ushort size = _countof(QR_AMOUNT_DATA_CHECK_STR) - (BAR_DATA_OFFSET * 3) + (BAR_DATA_OFFSET * pAmntQR->DataCount);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 7) && (j < size); i++, j++ ) {	// 1�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNum(2, 11, pAmntQR->IssueDate.Year, 4);	// ���s�N
		UsMnt_QR_DispNum(2, 16, pAmntQR->IssueDate.Mon, 2);		// ���s��
		UsMnt_QR_DispNum(2, 19, pAmntQR->IssueDate.Day, 2);		// ���s��
		UsMnt_QR_DispNum(2, 22, pAmntQR->IssueDate.Hour, 2);	// ���s��
		UsMnt_QR_DispNum(2, 25, pAmntQR->IssueDate.Min, 2);		// ���s��
		UsMnt_QR_DispNum(2, 28, pAmntQR->IssueDate.Sec, 2);		// ���s�b
		UsMnt_QR_DispNumRight(3, pAmntQR->FacilityCode);		// �{�݃R�[�h
		UsMnt_QR_DispNumRight(4, pAmntQR->ShopCode);			// �X�܃R�[�h
		UsMnt_QR_DispNumRight(5, pAmntQR->PosNo);				// �X�ܓ��[���ԍ�
		UsMnt_QR_DispNumRight(6, pAmntQR->IssueNo);				// ���V�[�g���s�ǔ�
		break;
	case 1:
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		if( qrtype != QR_FORMAT_CUSTOM ) {
		// �W���t�H�[�}�b�g
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		for( i = 2, j = 5; (i < 7) && (j < size); i++, j++ ) {	// 6�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->DataCount);			// �f�[�^��
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[0].type);		// �o�[�R�[�h�^�C�v�P
		UsMnt_QR_DispNumRight(4, pAmntQR->BarData[0].data);		// �f�[�^�P
		if ( pAmntQR->DataCount >= 2 ) {
			UsMnt_QR_DispNumRight(5, pAmntQR->BarData[1].type);	// �o�[�R�[�h�^�C�v�Q
			UsMnt_QR_DispNumRight(6, pAmntQR->BarData[1].data);	// �f�[�^�Q
		}
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		} else {
		// �ʋ��ʃt�H�[�}�b�g
		for( i = 2, j = 6; (i < 4) && (j < size); i++, j++ ) {	// 6�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->BarData[0].type);		// �o�[�R�[�h�^�C�v�P
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[0].data);		// �f�[�^�P
		}
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
		break;
	case 2:
		for( i = 2, j = 10; (i < 7) && (j < size); i++, j++ ) {	// 11�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_AMOUNT_DATA_CHECK_STR[j]);
		}
		UsMnt_QR_DispNumRight(2, pAmntQR->BarData[2].type);	// �o�[�R�[�h�^�C�v�R
		UsMnt_QR_DispNumRight(3, pAmntQR->BarData[2].data);	// �f�[�^�R
		break;
	default:
		break;
	}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
}

// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
//[]----------------------------------------------------------------------[]
/// @brief		QR���Ԍ��̕\��
//[]----------------------------------------------------------------------[]
///	@param[in]	pDisQR		: QR�f�[�^(QR���Ԍ�)
///	@param[in]	updown_page	: �㉺�ړ��̃y�[�W
/// @return 	����
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2023/01/05
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void UsMnt_QR_DispTicQR(QR_TicketInfo *pTicQR, ushort updown_page)
{
	ushort i;
	ushort j;
	ushort size = _countof(QR_TICKET_DATA_CHECK_STR);

	switch( updown_page ) {
	case 0:
		for( i = 2, j = 0; (i < 6) && (j < size); i++, j++ ) {	// 1�s�ڂ���
			grachr(i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TICKET_DATA_CHECK_STR[j]);
		}
		for ( i = 0; i <= 3; i++ ) {
			if ( (long)pTicQR->ParkingNo == CPrmSS[S_SYS][1+i] ) {
				break;
			}
		}
		UsMnt_QR_DispNum(2, 11, pTicQR->IssueDate.Year, 4);				// ���s�N
		UsMnt_QR_DispNum(2, 16, pTicQR->IssueDate.Mon, 2);				// ���s��
		UsMnt_QR_DispNum(2, 19, pTicQR->IssueDate.Day, 2);				// ���s��
		UsMnt_QR_DispNum(2, 22, pTicQR->IssueDate.Hour, 2);				// ���s��
		UsMnt_QR_DispNum(2, 25, pTicQR->IssueDate.Min, 2);				// ���s��
		UsMnt_QR_DispNum(2, 28, pTicQR->IssueDate.Sec, 2);				// ���s�b
		grachr(3, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, PASSSTR2[i]);	// ���ԏ�i��{�^�g�P�^�g�Q�^�g�R�^�@�@�j
		UsMnt_QR_DispNumRight(4, pTicQR->EntryMachineNo);				// �����@�ԍ�
		UsMnt_QR_DispNumRight(5, pTicQR->ParkingTicketNo);				// ���Ԍ��ԍ�
		break;
	default:
		break;
	}
}
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j

//[]----------------------------------------------------------------------[]
/// @brief		�㉺�ړ��̃y�[�W�̍ő吔
//[]----------------------------------------------------------------------[]
///	@param[in]	id				: QR�R�[�hID
///	@param[in]	pAmntQR			: QR�f�[�^(QR���㌔)
/// @return 	updown_page_max	: �㉺�ړ��̃y�[�W�̍ő吔
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2021/03/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ushort UsMnt_QR_GetUpdownPageMax(ushort id, QR_AmountInfo *pAmntQR)
{
	ushort updown_page_max;

	if( id == BAR_ID_DISCOUNT ) {	// ������
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//		updown_page_max = 2;
		updown_page_max = 1;
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	} else if( id == BAR_ID_TICKET ) {	// ���Ԍ�
		updown_page_max = 0;
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	} else {						// ���㌔
		if( pAmntQR->DataCount == 3 ){
			updown_page_max = 2;
		} else {
			updown_page_max = 1;
		}
	}

	return updown_page_max;
}
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P

///[]----------------------------------------------------------------------[]
/// @brief		�o�[�R�[�h�\�`�F�b�N
///[]----------------------------------------------------------------------[]
/// @param		: tMediaDetail *MediaDetail : QR�f�[�^���
//[]-----------------------------------------------------------------------[]
///	@return		: RESULT_NO_ERROR : �g�p�\�ȃo�[�R�[�h
///				 RESULT_BAR_FORMAT_ERR�F�t�H�[�}�b�g�G���[<br>
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static OPE_RESULT UsMnt_UseBarcodeChk( tMediaDetail *MediaDetail )
{
	// ��Ή��o�[�R�[�h
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
//	if( MediaDetail->Barcode.id != BAR_ID_AMOUNT && MediaDetail->Barcode.id != BAR_ID_DISCOUNT ){
	if( MediaDetail->Barcode.id != BAR_ID_AMOUNT && 
		MediaDetail->Barcode.id != BAR_ID_DISCOUNT &&
		MediaDetail->Barcode.id != BAR_ID_TICKET ){
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
		return RESULT_BAR_ID_ERR;	// �ΏۊO
	}

	return RESULT_NO_ERROR;
}
//[]----------------------------------------------------------------------[]
///	@brief			�o�[�R�[�h���
//[]----------------------------------------------------------------------[]
/// @param		: tMediaDetail *MediaDetail : QR�f�[�^���
//[]----------------------------------------------------------------------[]
///	@return			TRUE : ��͐���
///[]----------------------------------------------------------------------[]
///  @date       : 2020.1.13 
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL UsMnt_ParseBarcodeData( tMediaDetail *MediaDetail )
{
	ushort	row_size = 0;
	lcdbm_rsp_QR_rd_rslt_t *pQR = &LcdRecv.lcdbm_rsp_QR_rd_rslt;

	// LCD�ŉ�͍ς݂̂���, ��̓f�[�^���R�s�[
	MediaDetail->Barcode.id = pQR->id;					// QR�R�[�hID
	MediaDetail->Barcode.rev = pQR->rev;					// QR�R�[�h�t�H�[�}�b�gRev.
	MediaDetail->Barcode.enc_type = pQR->enc_type;		// QR�R�[�h�G���R�[�h�^�C�v
	MediaDetail->Barcode.info_size = pQR->info_size;		// QR�f�[�^���(�p�[�X�f�[�^)�T�C�Y
	memcpy( &MediaDetail->Barcode.QR_data, &pQR->QR_data, sizeof(MediaDetail->Barcode.QR_data) );	// QR�p�[�X�f�[�^
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	MediaDetail->Barcode.qr_type = pQR->qr_type;		// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j

	// ���f�[�^�R�s�[
	row_size = pQR->data_size;
	if( row_size > BAR_DATASIZE ){
		row_size = BAR_DATASIZE;
	}
	memcpy( MediaDetail->RowData, pQR->data, row_size );
	return TRUE;
}

///[]----------------------------------------------------------------------[]
/// @brief ���[�U�[�����e�i���X�FQR�f�[�^�m�F                                    
///[]----------------------------------------------------------------------[]
/// @param 	     : void
///[]----------------------------------------------------------------------[]
/// @return      : MOD_CHG : mode change
///                MOD_EXT : F5 key 
///                MOD_CUT : LCD_DISCONNECT
///[]----------------------------------------------------------------------[]
/// @date        : 2020.1.13                                               
///[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort	UsMnt_QR_DataCkk( void )
{
	ushort msg;
	ushort updown_page = 0;	// �㉺�ړ�
	ushort	Ret_updown_pagemax;	// �㉺�ړ��ő�y�[�W��
	ushort slide_page = 0;	// ���ړ�
	ushort slide_pagemax = ticketdata.cnt; // ���ړ��̍ő�y�[�W��
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//	QR_AmountInfo*	 pAmntQR;			// QR�f�[�^��� QR���㌔
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
	ushort result;
	int mode = 0;							// 0 = �ʏ�A1 = �f�[�^�Ȃ��A

	dispclr();

	// �^�C�g���\��
	if( slide_pagemax < 1){
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_TITLE[0] );	// 	"�p�q�f�[�^�m�F�@�@�@�@�@�@�@�@",
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_DATA_CHECK_ERR[0] );	// 	" �@�@�f�[�^�͂���܂���@�@ ",
		Fun_Dsp(FUNMSG[8]);															// 	"                         �I�� ",
		mode = 1;																	// F�T�ȊO���삳���Ȃ��B
	}else {
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//		UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
		result = pop_ticketErr(slide_page);
		Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
		UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
		mode = 0;
	}

	// �ǎ�J�n�v��
// MH810100(S) Y.Yamauchi 2020/03/16 �Ԕԃ`�P�b�g���X (#4037 QR�f�[�^�m�F��ʂɑJ�ڌ�A�������Ă��܂�)
//	if( FALSE == PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) == FALSE ){
//		// ���s��
//		BUZPIPI();
//	 }
	if( FALSE == PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) ){
		// ���s��
		BUZPIPI();
	}
// MH810100(E) Y.Yamauchi 2020/03/16 �Ԕԃ`�P�b�g���X (#4037 QR�f�[�^�m�F��ʂɑJ�ڌ�A�������Ă��܂�)

	while( 1 ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){

			// ���샂�[�h�ؑ�
			case LCD_DISCONNECT:
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
// MH810100(S) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				return MOD_CUT;
				break;

			case KEY_MODECHG:
				BUZPI();
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
// MH810100(S) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				return MOD_CHG;
				break;

			// �u���v
			case KEY_TEN_F1:
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//				if(mode == 0){
//					BUZPI();
//					if( slide_pagemax >= 1){
//						Ret_updown_pagemax = UsMnt_QR_CardChk( pAmntQR, slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
//						if( updown_page == 0 ){
//							updown_page = Ret_updown_pagemax;
//						}else{
//							updown_page--;
//						}
//
//						UsMnt_QR_DataDisp( slide_page, updown_page , slide_pagemax);
//					}
//				}
// GG124100(S) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6577 ���[�U�[�����e�i���X��QR�f�[�^�m�F�ŁA�g�p�σo�[�R�[�h�̏�񂪈ꕔ�����\������Ȃ� [���ʉ��P���� No1529]
// 				if ( (mode == 0) && (result == RESULT_NO_ERROR) && (updown_page > 0) ) {
				if ( (mode == 0) &&							// �f�[�^����
					 (result != RESULT_BAR_FORMAT_ERR) &&	// �t�H�[�}�b�g�G���[
					 (result != RESULT_BAR_ID_ERR) &&		// �ΏۊO
					 (updown_page > 0) ) {					// ��Ɉړ��ł��Ȃ�
// GG124100(E) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6577 ���[�U�[�����e�i���X��QR�f�[�^�m�F�ŁA�g�p�σo�[�R�[�h�̏�񂪈ꕔ�����\������Ȃ� [���ʉ��P���� No1529]
					BUZPI();
					updown_page--;
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
				}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
				break;

			// �u���v
			case KEY_TEN_F2:
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//				if(mode == 0){
//					BUZPI();
//					if( slide_pagemax >= 1){
//						Ret_updown_pagemax = UsMnt_QR_CardChk( pAmntQR, slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
//						updown_page++;
//						if( updown_page > Ret_updown_pagemax ){
//							updown_page = 0;
//						}
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
//					}
//				}
// GG124100(S) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6577 ���[�U�[�����e�i���X��QR�f�[�^�m�F�ŁA�g�p�σo�[�R�[�h�̏�񂪈ꕔ�����\������Ȃ� [���ʉ��P���� No1529]
// 				if ( (mode == 0) && (result == RESULT_NO_ERROR) && (updown_page < Ret_updown_pagemax) ) {
				if ( (mode == 0) &&							// �f�[�^����
					 (result != RESULT_BAR_FORMAT_ERR) &&	// �t�H�[�}�b�g�G���[
					 (result != RESULT_BAR_ID_ERR) &&		// �ΏۊO
					 (updown_page < Ret_updown_pagemax) ) {	// ���Ɉړ��ł��Ȃ�
// GG124100(E) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6577 ���[�U�[�����e�i���X��QR�f�[�^�m�F�ŁA�g�p�σo�[�R�[�h�̏�񂪈ꕔ�����\������Ȃ� [���ʉ��P���� No1529]
					BUZPI();
					updown_page++;
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
				}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
				break;

			// �u���v
			case KEY_TEN_F3:
				if(mode == 0){
					BUZPI();
					if( slide_pagemax > 1){
						if( slide_page == 0 ){
							slide_page = slide_pagemax - 1;
						}else{
							slide_page--;
						}
						updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
						result = pop_ticketErr(slide_page);
						Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
						UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
					}
				}
				break;

			// �u���v
			case KEY_TEN_F4:
				if(mode == 0){
					BUZPI();
					if( slide_pagemax > 1){
						slide_page++;
						if( slide_page >= slide_pagemax ){
							slide_page = 0;
						}
						updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//						UsMnt_QR_DataDisp( slide_page, updown_page, slide_pagemax );
						result = pop_ticketErr(slide_page);
						Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
						UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
					}
				}
				break;

			// �u�I���v
			case KEY_TEN_F5:
// MH810100(S) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				Lagcan(OPETCBNO, 6);
// MH810100(E) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
				BUZPI();
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
				return MOD_EXT;

			// �ǎ挋��
			case LCD_MNT_QR_READ_RESULT:
				// QR�f�[�^�̃T�C�Y���O�̂��͖̂���
// MH810100(S) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
//				if( LcdRecv.lcdbm_rsp_QR_rd_rslt.result != 0x09 ) {
				if ( LcdRecv.lcdbm_rsp_QR_rd_rslt.result != 0x01 ) {	// �ُ� �ȊO
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// MH810100(E) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
					if( LcdRecv.lcdbm_rsp_QR_rd_rslt.result == 0x00 ){
						// ����
						if( UsMnt_ParseBarcodeData( &MediaDetail ) ){			// �o�[�R�[�h���	
							result = UsMnt_UseBarcodeChk( &MediaDetail );		// �o�[�R�[�h�\�`�F�b�N
// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//							if( result == RESULT_NO_ERROR ){
//								updown_page = 0;								// ����f�[�^�̏ꍇ�A�y�[�W�ʒu��������
//							}
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
						}
					}
// MH810100(S) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
					else {	
						// �t�H�[�}�b�g�s��,�ΏۊO
						result = Lcd_QR_ErrDisp( LcdRecv.lcdbm_rsp_QR_rd_rslt.result );
					}
// MH810100(E) Y.Yamauchi 2020/03/25 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
					push_ticket( &MediaDetail, result);			// QR�f�[�^�X�V����
					slide_pagemax = ticketdata.cnt;				// �y�[�W�����X�V
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//					BUZPI();
//					UsMnt_QR_DataDisp( 0, 0, slide_pagemax );
					if ( result == RESULT_NO_ERROR ) {
						BUZPI();
					} else {
						BUZPIPI();
					}
					slide_page = 0;
					updown_page = 0;
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax);
					Ret_updown_pagemax = UsMnt_QR_CardChk(slide_page);	// �㉺�ړ��̃y�[�W�̍ő吔
					UsMnt_QR_DataDisp(slide_page, updown_page, slide_pagemax, Ret_updown_pagemax);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
					mode = 0;
// MH810100(S) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
//					PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STA );
					Lagtim(OPETCBNO, 6, 100);		/* 2sec timer start */
// MH810100(E) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
				} else {												// �ُ�
					// �ُ�̏ꍇ��QR���[�_�[�Ƃ̒ʐM�s�ǂ�z�肵�ăf�B���C��10�b�ɑ��₷�B
					// �ǂݎ�������e�̃G���[�ł͂Ȃ�����QR�f�[�^�X�V������\�������͍s��Ȃ��B
					Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// MH810100(S) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
				}
// MH810100(E) Y.Yamauchi 2020/03/30 �Ԕԃ`�P�b�g���X(#4068�őΏۊOQR��ǂݍ���ł���ʂɑJ�ڂ��Ȃ�)
				break;
// MH810100(S) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
			case TIMEOUT6:
				PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STA );
				break;
// MH810100(E) 2020/07/17 #4546�y�A���]���w�E�����z�y�A���]���w�E�����zQR�R�[�h�m�F�i�����e�i���X�j�̓ǂݎ��Ԋu���Z��
			default:
				break;
		}
	}
}
// MH810100(E) Yamauchi 2019/12/04 �Ԕԃ`�P�b�g���X(�����e�i���X)