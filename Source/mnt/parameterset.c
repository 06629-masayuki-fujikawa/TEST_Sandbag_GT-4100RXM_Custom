/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽ����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      : ��۰ٕ����ύX 2005-06-15 T.Hashimoto                     |*/
/*| UpDate      : font_change 2005-09-01 T.Hashimoto                       |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"LKmain.H"
#include	"PRM_TBL.H"
#include	"AppServ.h"
#include	"sysmnt_def.h"
#include	"fla_def.h"
#include	"ntnet.h"
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

/* Function Define */
// MH810100(S) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
unsigned short	CPara_Set( void );
unsigned short	RPara_Set( void );

unsigned short	SetDefault(ushort mode, char * p_f_DataChange);

void	pra_dp_1( long pno, short mod, short direction );
void	car_pra_dp( long pno, short mod, short direction );
void	rck_pra_dp( long pno, short mod, short direction );
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// void	pra_dp_2( long pno, short mod );
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
short	adrno( long address );
short	sesno( long address );
long	nextadd( long address, int mode );
char	isvalidadd( long add );
short	csesno( long address );
short	cadrno( long address );
long	cnextadd( long add, int mode );
char	cisvalidadd( long add );
unsigned long	cprmread( short carno, short caradd );
void	cprmwrit( short carno, short caradd, long data );
short	rsesno( long address );
short	radrno( long address );
long	rnextadd( long add, int mode );
char	risvalidadd( long add );
unsigned long	rprmread( short carno, short caradd );
void	rprmwrit( short carno, short caradd, long data );
long	prm_set(char kin, short ses, short adr, char len, char pos, long dat);

static short	gCurLoc;		// ���وʒu�̒i��
static long		gPreAdr;		// �O��̱��ڽ�l

short DispAdjuster;	// �Ԏ��p�����[�^�\���p�����l
const	ushort	Car_Start_Index[3] = {INT_CAR_START_INDEX, CAR_START_INDEX, BIKE_START_INDEX};

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// long	nomask_ses_next31( long add );
// long	nomask_ses_prev31( long add );
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
long	nomask_line_top( long pno, short curloc );
char	get_disp_line( long add );
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// char	is_nomask( long add );
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
unsigned short	PutGuideMessage( void );
char	IsRebootRequest( long add );

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
long get_nomask_ses_add_common( long current_add, int mode, PRM_MASK* p_mask, short mask_max );
long get_nomask_ses_add( long current_add, int mode );
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

//���g�p�ł��B�iS�j
/*[]----------------------------------------------------------------------[]*/
/*| ���Ұ��ݒ��ƭ����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ParSetMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short ParSetMain( void )
{
	unsigned short	usParaEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[0] );			// [00]	"���p�����[�^�[�ݒ聄�@�@�@�@�@"

		if(( rt_ctrl.param.set == ON ) ||			// �����v�Z�e�X�g����̋N��
		   ( rt_ctrl.param.no != 0 )){				// �����v�Z�e�X�g����̖߂�
			usParaEvent = BASC_PARA;
		}else{
			usParaEvent = Menu_Slt( PRMENU, PARA_SET_TBL, (char)PARA_SET_MAX, (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usParaEvent ){

			case BASC_PARA:
				usParaEvent = BPara_Set();			// ��{���Ұ�
				break;
			case CELL_PARA:
				usParaEvent = CPara_Set();			// �Ԏ����Ұ�
				break;
			case ROCK_PARA:
				usParaEvent = RPara_Set();			// ۯ�������Ұ�
				break;

			//Return Status of Prameter Menu
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usParaEvent;
				break;

			default:
				break;
		}
		//Return Status of Prameter Operation
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usParaEvent == MOD_CHG ){
		if( usParaEvent == MOD_CHG || usParaEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usParaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
		if(( usParaEvent == MOD_EXT ) &&
		   (( rt_ctrl.param.set == ON ) || ( rt_ctrl.param.no != 0 ))){
			return MNT_RYOTS;
		}
	}
}
//���g�p�ł��B�iS�j

/*[]----------------------------------------------------------------------[]*/
/*| ��{���Ұ��ݒ�                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BPara_Set( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short BPara_Set( void )
{
	unsigned short	usBParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmSetteiData FrmSetteiData;
	char	f_DataChange;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	char	edy_DataChange;
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	char	param_chk = 0;
	ushort	sntp_prm_after;		// SNTP���������ύX���j�^���O�o�^�p(�ݒ�34-0122�̇B�C�D�E��ێ�)

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	// �I�[�g�ϐ������������Ă��Ȃ����߁A���j���[�̂ǂ��ɂ�����Ȃ��ꍇ�͕s��l�ɂȂ��Ă��܂��̂Ő擪�ŃN���A
	f_DataChange = 0;

	for( ; ; )
	{
		dispclr();
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[1] );			// [01]	"����{�p�����[�^�[�ݒ聄�@�@�@"
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[0] );			// [0]	"���p�����[�^�[�ݒ聄�@�@�@"
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

		if(( rt_ctrl.param.set == ON ) ||			// �����v�Z�e�X�g����̋N��
		   ( rt_ctrl.param.no != 0 )){				// �����v�Z�e�X�g����̖߂�
			usBParaEvent = CMON_PARA;
		}else{
			usBParaEvent = Menu_Slt( BPRMENU, BPARA_SET_TBL, (char)BPARA_SET_MAX, (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usBParaEvent ){

			case CMON_PARA:
				sntp_prm_before = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);		// SNTP���������̒l���擾
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				usBParaEvent = SysParWrite( &f_DataChange, &edy_DataChange );
				usBParaEvent = SysParWrite( &f_DataChange );
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				if( 1 == f_ParaUpdate.BIT.bpara ){					// changed parameter data
					DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
					wopelg( OPLOG_KYOTUPARAWT, 0, 0 );					// ���엚��o�^
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
					mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );	// FTP�X�V�t���O�Z�b�g�i�v�d�f�j
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( edy_DataChange == 1 )						// Edy�֘A�ݒ�ύX�H
//						wopelg( OPLOG_EDY_PRM_CHG,0,0 );			// ���엚��o�^(Edy�֘A�ݒ�ύX)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					sntp_prm_after = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);
					if (sntp_prm_before != sntp_prm_after) {
					// SNTP���������ݒ�l�ɕύX���������ꍇ�AM8036�o�^
						wopelg( OPLOG_SET_SNTP_SYNCHROTIME, (ulong)sntp_prm_before, (ulong)sntp_prm_after);
					}
					SetSetDiff(SETDIFFLOG_SYU_SYSMNT);				// �ݒ�X�V�������O�o�^

					// ���̏����͏d������,����ĂԕK�v�������̂œd��ON���Ɛݒ�ύX���̂�Call���鎖�Ƃ���B 
					mc10();		// �����v�Z�p�ݒ辯�
//					LCD_renewal = (uchar)CPrmSS[S_PAY][27];			// ���׽Ē����̂��ߍĕ\���v��
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
					PrnGetRegistNum();
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
					param_chk = 1;
					f_ParaUpdate.BYTE = 0;
				}
				break;

			case INDV_PARA:
				break;

			case PRNT_PARA:
				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
					BUZPIPI();
					break;
				}
				FrmSetteiData.prn_kind = R_PRI;
				FrmSetteiData.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_SETTEIDATA, sizeof(T_FrmSetteiData), &FrmSetteiData );
				Ope_DisableDoorKnobChime();
				/* �v�����g�I����҂����킹�� */
				Fun_Dsp(FUNMSG[82]);				// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

				for ( priend = 0 ; priend == 0 ; ) {

					usBParaEvent = StoF( GetMessage(), 1 );

					if( (usBParaEvent&0xff00) == (INNJI_ENDMASK|0x0600) ){
						usBParaEvent &= (~INNJI_ENDMASK);
					}
					switch( usBParaEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case LCD_DISCONNECT:
							priend = 1;
							break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case	KEY_MODECHG:		// Ӱ����ݼ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
							// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
							// �h�A�m�u���ǂ����̃`�F�b�N�����{
							if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
							priend = 1;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
							}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
							break;

						case	PREQ_SETTEIDATA:	// �󎚏I��
							priend = 1;
							break;

						case	KEY_TEN_F3:			// F3���i���~�j

							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
							priend = 1;
							break;
					}
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( usBParaEvent == KEY_MODECHG ){
				if( usBParaEvent == KEY_MODECHG || usBParaEvent == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( param_chk ){
						usBParaEvent = parameter_upload_chk();
						param_chk = 0;
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					}
//						return MOD_CHG;
//					}
					}else if ( usBParaEvent == KEY_MODECHG ){
						return MOD_CHG;
					}else{
						return MOD_CUT;
					}
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Fun_Dsp(FUNMSG[25]);				// F���\���@"�@���@�@���@�@�@�@ �Ǐo  �I�� "

				break;

			case DEFA_PARA:
				f_DataChange = 0;
				usBParaEvent = SetDefault(DEFA_PARA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
					DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update

					// ���̏����͏d������,����ĂԕK�v�������̂œd��ON���Ɛݒ�ύX���̂�Call���鎖�Ƃ���B 
					mc10();		// �����v�Z�p�ݒ辯�
//					LCD_renewal = (uchar)CPrmSS[S_PAY][27];			// ���׽Ē����̂��ߍĕ\���v��
					SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);				// �ݒ�X�V�������O�o�^
					SetDiffLogReset(2);								// ���ʃp�����[�^�~���[�����O
					// �Ԏ��p�����[�^�̒[���ݒ�����ʃp�����[�^�ɔ��f
					param_chk = 1;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
					mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );				// FTP�X�V�t���O�Z�b�g�i�v�d�f�j
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
					PrnGetRegistNum();
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
				}
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				if( param_chk ){
					usBParaEvent = parameter_upload_chk();
				}
				return usBParaEvent;
				break;
	
// MH810100(S) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_CHG:
				if( param_chk ){
					usBParaEvent = parameter_upload_chk();
				}
				return usBParaEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usBParaEvent == MOD_CHG ){
//			if( param_chk ){
		if( usBParaEvent == MOD_CHG ||  usBParaEvent == MOD_CUT ){
			if( param_chk &&  usBParaEvent == MOD_CHG ){
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				usBParaEvent = parameter_upload_chk();
			}
			return usBParaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
		if(( usBParaEvent == MOD_EXT ) &&
		   (( rt_ctrl.param.set == ON ) || ( rt_ctrl.param.no != 0 ))){
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			return MOD_EXT;
		   	return MNT_RYOTS;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �������Ұ��ݒ�                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = SysParWrite();                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : short    ret;    0x00fe : F5 Key ( End Key )            |*/
/*|              :                  0x00ff : Maintenance Key               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

#define Lin				(gCurLoc+2)
#define	TRIM6DGT(d)		(d % 1000000L)
#define	BPAR_TOP		(long)10001

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//unsigned short	SysParWrite( char * p_f_DataChange, char * p_edy_DataChange )
unsigned short	SysParWrite( char * p_f_DataChange )
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[14];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position
	char			i;
	unsigned short	f_Reboot;

	*p_f_DataChange = 0;
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	*p_edy_DataChange = 0;
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;
	f_Reboot = 0;
	if( rt_ctrl.param.no != 0 ){		// �����v�Z�e�X�g����̖߂�
		no = rt_ctrl.param.no;
		rt_ctrl.param.no = 0;
	}else{								// �ʏ�N��
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 		if(OPECTL.Mnt_lev < 4){
// 		no = nomask_all_next( BPAR_TOP - 1 );
// 		}
// 		else{
// 			no = BPAR_TOP;
// 		}
		no = nomask_all_next(BPAR_TOP - 1);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[3] );	// [03]	"�����ʃp�����[�^�[�ݒ聄�@�@�@"
	if( rt_ctrl.param.set == ON ){										// �����v�Z�e�X�g����̋N��
		Fun_Dsp( FUNMSG[29] );											// [29]	"  �{  �|�^��  ��   ���  �I�� "
	}else{																// �ʏ�N���܂��͗����v�Z�e�X�g����̖߂�
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 		Fun_Dsp( FUNMSG[116] );											// [116]"  ��  ���^��  ��  �e�X�g �I�� "
		if ( CLOUD_CALC_MODE ) {										// �N���E�h�����v�Z���[�h
			Fun_Dsp(FUNMSG[29]);										// [29]	"  ��  ���^��  ��         �I�� "
		} else {														// �ʏ헿���v�Z���[�h
			Fun_Dsp(FUNMSG[116]);										// [116]"  ��  ���^��  ��  �e�X�g �I�� "
		}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	}
																		// [30]	"  ��    ��    ��   ����  �I��  "
																		//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	pra_dp_1( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){

// MH810100(S) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				if( rt_ctrl.param.set == ON ){		// �����v�Z�e�X�g����̋N��
					rt_ctrl.param.no = 0;			// �p�����[�^�A�h���X�N���A
				}
				if( f_Reboot ){
					ret = PutGuideMessage();
					return( ret );
				}
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					address = -1;
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 				if(OPECTL.Mnt_lev < 4){
// 					no = nomask_prev( no );
// 				}
// 				else{
// 					no = nextadd( no, -1 );
// 				}
					no = nomask_prev(no);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
					pra_dp_1( no, 0, 1 );
				}else{
					data = -1;
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 				if(OPECTL.Mnt_lev < 4){
// 					no = nomask_prev( no );
// 				}
// 				else{
// 					no = nextadd( no, -1 );			//F1 address"-"
// 				}
					no = nomask_prev(no);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
					pra_dp_1( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 						if(OPECTL.Mnt_lev < 4){
// 							no = nomask_next( no );
// 						}
// 						else{
// 							no = nextadd( no, 1 );
// 						}
						no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
						pra_dp_1( no, 0, 0 );

					}else {						//F2 address set"XXXXXX"
						if( isvalidadd( address ) ){
							BUZPI();
							no = address;
							gCurLoc = 0;
							pra_dp_1( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}else{							//F2 address"+"
					BUZPI();
					data = -1;
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 					if(OPECTL.Mnt_lev < 4){
// 						no = nomask_next( no );
// 					}
// 					else{
// 						no = nextadd( no, 1 );
// 					}
					no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
					pra_dp_1( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					pra_dp_1( no, 1, 2 );
				}else {
					r_md = 0;
					if( rt_ctrl.param.set == ON ){	// �����v�Z�e�X�g����̋N��
						Fun_Dsp( FUNMSG[29] );		// [29]	"  ��  ���^��  ��         �I�� "
					}else{							// �ʏ�N���܂��͗����v�Z�e�X�g����̖߂�
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 						Fun_Dsp( FUNMSG[116] );		// [112]]"  �{  �|�^��  ��  �e�X�g �I�� "
						if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
							Fun_Dsp(FUNMSG[29]);	// [29]	"  ��  ���^��  ��         �I�� "
						} else {					// �ʏ헿���v�Z���[�h
							Fun_Dsp(FUNMSG[116]);	// [116]"  ��  ���^��  ��  �e�X�g �I�� "
						}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
					}
					pra_dp_1( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:						// ����
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						for( i = 0; i < 6; i++ ){
							ddd[i] -= '0';
							if( ddd[i] > 9 ){
								ddd[i] -= 0x07;
							}
						}
						CPrmSS[sesno(no)][adrno(no)] = data;
						nmisave( &(CPrmSS[sesno(no)][adrno(no)]), &data, 4 );
						f_ParaUpdate.BIT.bpara = 1;
						*p_f_DataChange = 1;
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( sesno(no) == S_SCA && ( adrno(no) >= 51 && adrno(no) <= 100 ))
//							*p_edy_DataChange = 1;
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
						if( f_Reboot == 0 ){
							if( IsRebootRequest( no ) ){
								f_Reboot = 1;
							}
						}
					}
					data = -1;
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 					if(OPECTL.Mnt_lev < 4){
// 						no = nomask_next( no );
// 					}
// 					else{
// 						no = nextadd( no, 1 );		//F1 address"+"
// 					}
					no = nomask_next(no);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
					pra_dp_1( no, 1, 0 );
				}else{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 					if( rt_ctrl.param.set != ON ){	// �����v�Z�e�X�g����̋N���łȂ�
					if ( (rt_ctrl.param.set != ON) &&	// �����v�Z�e�X�g����̋N���łȂ�
						 !CLOUD_CALC_MODE ) {			// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
						BUZPI();
						rt_ctrl.param.no = no;		// �p�����[�^�A�h���X��ێ�
						return( MOD_EXT );
					}
				}
				break;

			case KEY_TEN_CL:						// Clear"C"
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
					cnvdec2( (char *)ddd, sesno(no) );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;		/* "�|" */
					wk[5] = 0x7c;
					//address
					cnvdec4( (char *)ddd, (long)adrno(no) );
					as1chg( ddd, &wk[6], 4 );
					grachr ( (unsigned short)Lin,  2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ) {
						data = -1;
					}
					intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(no)][adrno(no)]), 6 );
					as1chg( ddd, wk, 6 );
					grachr ( (unsigned short)Lin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;


			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 100000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					cnvdec2( (char *)ddd, sesno(address) );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;		/* "�|" */
					wk[5] = 0x7c;
					//address
					cnvdec4( (char *)ddd, (long)adrno(address) );
					as1chg( ddd, &wk[6], 4 );
					grachr ( (unsigned short)Lin,  2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( (unsigned char *)ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)Lin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*|  System Parameter Read                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pra_dp_1( pno, mod );                                   |*/
/*| PARAMETER    : short    pno;    Parameter No. ( 1 - 1499 )             |*/
/*|              :          mod;    Cursor Disp Position                   |*/
/*|                                   0 : Parameter No.                    |*/
/*|                                   1 : Parameter Data                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

void pra_dp_1( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[26];
	unsigned char	ddd[6];
	long			add;

	// incadd<=�ŏ�i�ɕ\��������ڽ�����߂�B
	incadd = pno;
	if( sesno(gPreAdr) != sesno(pno) ){
		if( direction == 1 )
		{	// -����
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 			if(OPECTL.Mnt_lev < 4){
// 				add = nomask_line_top( pno, 4 );
// 				if( add ){
// 					incadd = add;
// 				}
// 			}
// 			else{
// 				incadd = pno - 4;
// 			}
			add = nomask_line_top(pno, 4);
			if ( add ) {
				incadd = add;
			}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
		}
	}else{
		if( direction == 0 )
		{	// +����
			if( gCurLoc >= 4 ){
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, 4 );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - 4;
// 				}
				add = nomask_line_top(pno, 4);
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
			}else{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, (short)(gCurLoc + 1) );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - gCurLoc - 1;
// 				}
				add = nomask_line_top(pno, (short)(gCurLoc + 1));
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
			}
		}else if( direction == 1 )
		{	// -����
			if( gCurLoc != 0 ){
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 				if(OPECTL.Mnt_lev < 4){
// 					add = nomask_line_top( pno, (short)(gCurLoc - 1) );
// 					if( add ){
// 						incadd = add;
// 					}
// 				}
// 				else{
// 					incadd = pno - gCurLoc + 1;
// 				}
				add = nomask_line_top(pno, (short)(gCurLoc - 1));
				if ( add ) {
					incadd = add;
				}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
			}
		}
		else
		{	// ���E�ړ�
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 			if(OPECTL.Mnt_lev < 4){
// 				add = nomask_line_top( pno, gCurLoc );
// 				if( add ){
// 					incadd = add;
// 				}
// 			}
// 			else{
// 				incadd = pno - gCurLoc;
// 			}
			add = nomask_line_top(pno, gCurLoc);
			if ( add ) {
				incadd = add;
			}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
		}
	}
	lincnt = get_disp_line( incadd );

	for( i = 0; i < 5 ; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 2));
			continue;
		}

		//section no
		cnvdec2( (char *)ddd, sesno(incadd) );
		as1chg( ddd, work, 2 );
		//"-"
		work[4] = 0x81;		/* '�|' */
		work[5] = 0x7c;
		//address
		cnvdec4( (char *)ddd, (long)adrno(incadd) );
		as1chg( ddd, &work[6], 4 );
		//data
		intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(incadd)][adrno(incadd)]), 6 );
		as1chg( ddd, &work[14], 6 );

		if( pno == incadd ){

			gCurLoc = i;  // ���ݶ��وʒu����
			if( mod == 0 ){
				grachr( (unsigned short)(i+2), 2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		//sec & add
				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );	//data
			} else {
				grachr( (unsigned short)(i+2), 2, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+2), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
			}
		} else {
				grachr( (unsigned short)(i+2), 2, 14, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
		}

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 		if(OPECTL.Mnt_lev < 4){
// 			incadd = nomask_ses_next( incadd );
// 		}
// 		else{
// 			incadd = nextadd(incadd, 1);
// 		}
		incadd = nomask_ses_next(incadd);
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	}
	gPreAdr = pno;	// �O����ڽ����
	return;
}

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// /*[]----------------------------------------------------------------------[]*/
// /*|  System Parameter Read                                                 |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : pra_dp_2( pno, mod );                                   |*/
// /*| PARAMETER    : short    pno;    Parameter No. ( 1 - 1499 )             |*/
// /*|              :          mod;    Cursor Disp Position                   |*/
// /*|                                   0 : Parameter No.                    |*/
// /*|                                   1 : Parameter Data                   |*/
// /*| RETURN VALUE : void                                                    |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
// 
// void pra_dp_2( long pno, short mod )
// {
// 	short			i;
// 	long			incadd;
// 	unsigned char	work[26];
// 	unsigned char	ddd[6];
// 	long			BPAR_BTM;
// 	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
// 	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];
// 
// 	if(OPECTL.Mnt_lev < 4){
// 		incadd = nomask_all_prev( pno );
// 		if( incadd == 0 ){
// 			// �Y���f�[�^�Ȃ��̏ꍇ�A�Ō������T������
// 			incadd = nomask_all_prev( BPAR_BTM + 1 );
// 		}
// 		incadd = nomask_all_prev( incadd );
// 		if( incadd == 0 ){
// 			// �Y���f�[�^�Ȃ��̏ꍇ�A�Ō������T������
// 			incadd = nomask_all_prev( BPAR_BTM + 1 );
// 		}
// 	}
// 	else{
// 		incadd = nextadd( pno, -1 );
// 		incadd = nextadd( incadd, -1 );
// 	}
// 
// 	for( i = 0; i < 5; i++ ){
// 
// 		//section no
// 		cnvdec2( (char *)ddd, sesno(incadd) );
// 		as1chg( ddd, work, 2 );
// 		//"-"
// 		work[4] = 0x81;		/* "�|" */
// 		work[5] = 0x7c;
// 		//address
// 		cnvdec4( (char *)ddd, (long)adrno(incadd) );
// 		as1chg( ddd, &work[6], 4 );
// 		//data
// 		intoasl( (unsigned char *)ddd, (unsigned long)TRIM6DGT(CPrmSS[sesno(incadd)][adrno(incadd)]), 6 );
// 		as1chg( ddd, &work[14], 6 );
// 
// 		if( i == 2 ){	//fix to 3rd line
// 
// 			if( mod == 0 ){
// 				grachr( (unsigned short)(i+2), 2, 14, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		//sec & add
// 				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );	//data
// 			} else {
// 				grachr( (unsigned short)(i+2), 2, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
// 				grachr( (unsigned short)(i+2), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
// 			}
// 		} else {
// 				grachr( (unsigned short)(i+2), 2, 14, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
// 				grachr( (unsigned short)(i+2), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[14] );
// 		}
// 
// 		if(OPECTL.Mnt_lev < 4){
// 			incadd = nomask_ses_prev( incadd );
// 		}
// 		else{
// 			incadd = nextadd(incadd, 1);
// 		}
// 	}
// 	return;
// }
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

/* ���A�h���X�Z�N�V������Ԃ� */
short	sesno( long address )
{

	return (short)(address/10000L);
}
/* ���A�h���X��Ԃ� */
short	adrno( long address )
{
	return (short)(address%10000L);
}
/* ���O��̃A�h���X(6digit)��Ԃ� */
long	nextadd( long add, int mode )
{

	long nextadd;
	long			BPAR_BTM;
	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];
	nextadd = 0;

	if( 1 == mode ){

		if( add >= BPAR_BTM ){

			nextadd = BPAR_TOP;

		}else if( CPrmCnt[sesno(add)] < adrno(add) + 1 ){
			// ����݂̍Ō�ł���Ύ�����݂̍ŏ��ɂ���
			nextadd = 1 + ((sesno(add) + 1) * 10000L);

		}else{

			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if( add <= BPAR_TOP ){

			nextadd = BPAR_BTM;

		}else if( 0 >= (adrno(add)-1) ){
			// ����݂̍ŏ��ł���ΑO����݂̍ŏ��ɂ���
			nextadd = CPrmCnt[sesno(add) - 1] + (sesno(add) - 1) * 10000L;

		}else{

			nextadd = add - 1;
		}
	}else{

	}
	return nextadd;
}
/* ���e���L�[���͒l�͗L���A�h���X�H */
char	isvalidadd( long add )
{
	char ret = 1;

	if( ( C_PRM_SESCNT_MAX - 1 < sesno(add) )||
		( sesno(add) == 0 ) ||
		( CPrmCnt[sesno(add)] < adrno(add) ) ||
		( adrno(add) == 0 ) )
	{
		ret = 0;
	}

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	if(OPECTL.Mnt_lev < 4){
// 		if( ret ){
// 			// ��}�X�N�A�h���X���H
// 			ret = is_nomask( add );
// 		}
// 	}
	// �}�X�N�A�h���X�ł���Ζ���
	if ( is_nomask(add) == 0 ) {
		ret = 0;
	}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����Ұ���ݒ菈��                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CPara_Set()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : �ݒ�Ӱ�޽���OFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-04-14                                              |*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CPara_Set( void )
{
	unsigned short	usCParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmLockPara	FrmLockPara;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
	char	f_DataChange;
	long	bk_prk;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[2] );							// "���Ԏ��p�����[�^�[�ݒ聄�@�@�@"

		usCParaEvent = Menu_Slt( CPRMENU, CPARA_SET_TBL, (char)CPARA_SET_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usCParaEvent ){
			case CARP_PARA:
				if(( bk_prk = Is_CarMenuMake( CAR_3_MENU )) == 0 ){
					BUZPIPI();
					continue;
				}
				do {
					if( Ext_Menu_Max > 1 ){
						dispclr();
						grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[71]);		/* "���Ԏ��p�����[�^�[���@�@�@�@�@" */
						gCurSrtPara = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
					}else{
						gCurSrtPara = (short)bk_prk;
						bk_prk = 0;
					}
					switch ((ushort)gCurSrtPara) {
					case MOD_EXT:
						break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case MOD_CHG:
						return (ushort)gCurSrtPara;
						break;
					default:
						// �\���p�����l�ݒ�
						DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
					

						f_DataChange = 0;
						usCParaEvent = CarParWrite( &f_DataChange );

						if( 1 == f_ParaUpdate.BIT.cpara ) {					// changed parameter data
							wopelg( OPLOG_SHASHITUPARAWT, 0, 0 );			// ���엚��o�^
							DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM���SUM�X�V
							FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH������
							if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
								NTNET_Snd_Data225(0);						// NTNET�փf�[�^���M
							}
							f_ParaUpdate.BYTE = 0;
						}

						if (usCParaEvent == MOD_EXT) break;
						if (usCParaEvent == MOD_CHG) return MOD_CHG;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						if (usCParaEvent == LCD_DISCONNECT ) return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					}
				} while (bk_prk && (gCurSrtPara != (short)MOD_EXT));
				break;
			case CARP_PRNT:
				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
					BUZPIPI();
					break;
				}
				Cal_Parameter_Flg = 0;
				FrmLockPara.prn_kind = R_PRI;
				FrmLockPara.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_LOCK_PARA, sizeof(T_FrmLockPara), &FrmLockPara );
				Ope_DisableDoorKnobChime();
				/* �v�����g�I����҂����킹�� */
				Fun_Dsp(FUNMSG[82]);				// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

				for ( priend = 0 ; priend == 0 ; ) {

					usCParaEvent = StoF( GetMessage(), 1 );

					if( (usCParaEvent&0xff00) == (INNJI_ENDMASK|0x0600) ){
						usCParaEvent &= (~INNJI_ENDMASK);
					}
					switch( usCParaEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case LCD_DISCONNECT:	
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case	KEY_MODECHG:		// Ӱ����ݼ�
							priend = 1;
							break;

						case	PREQ_LOCK_PARA:		// �󎚏I��
							priend = 1;
							break;

						case	KEY_TEN_F3:			// F3���i���~�j

							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
							priend = 1;
							break;
					}
				}
				if( usCParaEvent == KEY_MODECHG ){
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if( usCParaEvent == LCD_DISCONNECT ){
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Fun_Dsp(FUNMSG[25]);				// F���\���@"�@���@�@���@�@�@�@ �Ǐo  �I�� "

				break;

			case CARP_DEFA:
				f_DataChange = 0;
				usCParaEvent = SetDefault(CARP_DEFA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
					DataSumUpdate(OPE_DTNUM_LOCKINFO);				/* RAM���SUM�X�V */
					FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);	/* FLASH������ */
					// �Ԏ��p�����[�^�̒[���ݒ�����ʃp�����[�^�ɔ��f
					if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
						NTNET_Snd_Data225(0);						// NTNET�փf�[�^���M
					}
				}
				break;
			case MOD_EXT:											// F5 Key
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( usCParaEvent );
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)	
			case MOD_CHG:											// �ݒ�Ӱ�޽���OFF
				return( usCParaEvent );
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usCParaEvent == KEY_MODECHG ){
		if( usCParaEvent == KEY_MODECHG ||usCParaEvent == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return( usCParaEvent );
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����Ұ���ݒ菑��������                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CarParWrite()                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : �ݒ�Ӱ�޽���OFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define CLin			(gCurLoc+1)
#define	CarAddMax		6
#define	IntCarAddMax	6

#define	CCPAR_TOP		(((CAR_START_INDEX+1)*100L)+1)							// �Ԏ����Ұ���J�n���ڽ(�Ԏ�1-���ڽ01)
#define	CCPAR_BTM		(((CAR_START_INDEX+CAR_LOCK_MAX)*100L)+CarAddMax)		// �Ԏ����Ұ���I�����ڽ(�Ԏ�50-���ڽ6)
#define	CICPAR_TOP		(((INT_CAR_START_INDEX+1)*100L)+1)						// �Ԏ����Ұ���J�n���ڽ(�Ԏ�51-���ڽ01)
#define	CICPAR_BTM		(((INT_CAR_START_INDEX+INT_CAR_LOCK_MAX)*100L)+IntCarAddMax)// �Ԏ����Ұ���I�����ڽ(�Ԏ�100-���ڽ6)
#define	CBPAR_TOP		(((BIKE_START_INDEX+1)*100L)+1)							// �Ԏ����Ұ���J�n���ڽ(�Ԏ�101-���ڽ01)
#define	CBPAR_BTM		(((BIKE_START_INDEX+BIKE_LOCK_MAX)*100L)+CarAddMax)		// �Ԏ����Ұ���I�����ڽ(�Ԏ�150-���ڽ6)

unsigned short	CarParWrite( char * p_f_DataChange )
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[12];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position

	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;

	dispclr();	
	if (gCurSrtPara == MNT_CAR) {
	// ����
		no = CCPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[8] );		// [06]	"���Ԏ��p�����[�^�[�i���Ԃh�e�j��"
	}
	else if (gCurSrtPara == MNT_INT_CAR) {
	// ���ԁi�����j
		no = CICPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[6] );		// [06]	"���Ԏ��p�����[�^�[�i���ԁj���@"
	}
	else if (gCurSrtPara == MNT_BIK) {
	// ����
		no = CBPAR_TOP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[7] );		// [07]	"���Ԏ��p�����[�^�[�i���ցj���@"
	}
	
	Fun_Dsp( FUNMSG[29] );					// [29]	"  �{  �|�^��  ��   ���  �I�� "
											// [30]	"       ����   ��   ���  �I�� "
											//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	car_pra_dp( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					address = -1;
					no = cnextadd( no, -1 );
					car_pra_dp( no, 0, 1 );
				}else{
					data = -1;
					no = cnextadd( no, -1 );		//F1 address"-"
					car_pra_dp( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
						no = cnextadd( no, 1 );
						car_pra_dp( no, 0, 0 );

					}else {						//F2 address set"XXXXXX"
						if( cisvalidadd( address ) ){
							BUZPI();
							no = address;
							no += (DispAdjuster * 100);
							gCurLoc = 0;
							car_pra_dp( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}
				else{							//F2 address"+"
					BUZPI();
					data = -1;
					no = cnextadd( no, 1 );
					car_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					car_pra_dp( no, 1, 2 );
				}else {
					r_md = 0;
					Fun_Dsp( FUNMSG[29] );
					car_pra_dp( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						cprmwrit( csesno(no), cadrno(no), data );
						f_ParaUpdate.BIT.cpara = 1;
						*p_f_DataChange = 1;
					}
					data = -1;
					no = cnextadd( no, 1 );		//F1 address"+"
					car_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_CL:						// Clear
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
// MH322914 (s) kasiyama 2016/07/20 �Ԏ��p�����[�^�A�h���X�\��NG�΍�(���ʉ��PNo.1255)(GM567102)
//					intoas( ddd, (ushort)csesno(no), 3 );
					intoas( ddd, (ushort)(csesno(no) - DispAdjuster), 3 );
// MH322914 (e) kasiyama 2016/07/20 �Ԏ��p�����[�^�A�h���X�\��NG�΍�(���ʉ��PNo.1255)(GM567102)
					as1chg( ddd, wk, 3 );
					//"-"
					wk[6] = 0x81;	/* "�|" */
					wk[7] = 0x7c;
					//address
					intoas( ddd, (ushort)cadrno(no), 2 );
					as1chg( ddd, &wk[8], 2 );
					grachr ( (unsigned short)CLin,  2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ){
						data = -1;
					}
					intoasl( ddd, cprmread( csesno(no), cadrno(no) ), 6 );
					as1chg( ddd, wk, 6 );
					grachr( (unsigned short)CLin, 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;

			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 10000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					intoas( ddd, (ushort)csesno(address), 3 );
					as1chg( ddd, wk, 3 );
					//"-"
					wk[6] = 0x81;	/* "�|" */
					wk[7] = 0x7c;
					//address
					intoas( ddd, (ushort)cadrno(address), 2 );
					as1chg( ddd, &wk[8], 2 );
					grachr ( (unsigned short)CLin,  2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)CLin,  18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����Ұ���ݒ�ǎ揈��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : car_pra_dp()                                            |*/
/*| PARAMETER    : pno : Parameter No. ( 1-01 �` 540-13 )                  |*/
/*|                mod : Cursor Disp Position  0 = Parameter No.           |*/
/*|                                            1 = Parameter Data          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	car_pra_dp( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[24];
	unsigned char	ddd[6];

	// incadd<=�ŏ�i�ɕ\��������ڽ�����߂�B
	incadd = pno;
	if( csesno(gPreAdr) != csesno(pno) ){
		if( direction == 1 )
		{	// -����
			if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
				incadd = pno - 5;
			}
			else {
			incadd = pno - 5;
			}
		}
	}else{
		if( direction == 0 )
		{	// +����
			if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
				if( gCurLoc >= 5 ){
					incadd = pno - 5;
				}else{
					incadd = pno - gCurLoc - 1;
				}
			}
			else {
			if( gCurLoc >= 5 ){
				incadd = pno - 5;
			}else{
				incadd = pno - gCurLoc - 1;
			}
			}
		}else if( direction == 1 )
		{	// -����
			if( gCurLoc != 0 ){
				incadd = pno - gCurLoc + 1;
			}
		}
		else
		{	// ���E�ړ�
			incadd = pno - gCurLoc;
		}
	}

	if( incadd >= CICPAR_TOP && incadd <= CICPAR_BTM ) {
		lincnt = ( IntCarAddMax < cadrno( incadd + 5 ) ) ?
					 ( IntCarAddMax - cadrno(incadd) + 1 ) : 6;
	}
	else {
	lincnt = ( CarAddMax < cadrno( incadd + 5 ) ) ?
				 ( CarAddMax - cadrno(incadd) + 1 ) : 6;
	}

	for( i = 0; i < 6; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 1));
			continue;
		}

		//car no
		intoas( ddd, (ushort)(csesno(incadd) - DispAdjuster), 3 );
		as1chg( ddd, work, 3 );
		//"-"
		work[6] = 0x81;	/* "�|" */
		work[7] = 0x7c;
		//address
		intoas( ddd, (ushort)cadrno(incadd), 2 );
		as1chg( ddd, &work[8], 2 );
		//data
		intoasl( ddd, cprmread( csesno(incadd), cadrno(incadd) ), 6 );
		as1chg( ddd, &work[12], 6 );

		if( pno == incadd ){

			gCurLoc = i; // ���ݶ��وʒu����

			if( mod == 0 ){
				grachr( (unsigned short)(i+1), 2, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		// car no
				grachr( (unsigned short)(i+1), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[12] );	// data
			} else {
				grachr( (unsigned short)(i+1), 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 18, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[12] );
			}
		} else {
			grachr( (unsigned short)(i+1), 2, 12, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
			grachr( (unsigned short)(i+1), 18, 12, 0,  COLOR_BLACK, LCD_BLINK_OFF, &work[12] );
		}

		incadd = cnextadd(incadd, 1);
	}
	gPreAdr = pno;	// �O����ڽ����
	return;
}

/* �Ԏ�����Ԃ� */
short	csesno( long address )
{
	return (short)(address/100L);
}

/* �A�h���X��Ԃ� */
short	cadrno( long address )
{
	return (short)(address%100L);
}

/* ���O��̃A�h���X(6digi)��Ԃ� */
long	cnextadd( long add, int mode )
{

	long nextadd;
	long add_top, add_btm;
	long add_max;
	
	add_max = 0;
	add_top = 0;
	nextadd = 0;
	add_btm = 0;
	if (gCurSrtPara == MNT_CAR) {
		add_top = CCPAR_TOP;
		add_btm = CCPAR_BTM;
		add_max = CarAddMax;
	}
	else if (gCurSrtPara == MNT_INT_CAR){
		add_top = CICPAR_TOP;
		add_btm = CICPAR_BTM;
		add_max = IntCarAddMax;
	}
	else if (gCurSrtPara == MNT_BIK){
		add_top = CBPAR_TOP;
		add_btm = CBPAR_BTM;
		add_max = CarAddMax;
	}
	
	if( 1 == mode ){

		if ( add >= add_btm ) {
			nextadd = add_top;
		}else if( add_max < cadrno(add) + 1 ){
			nextadd = 1 + ((csesno(add) + 1) * 100L);
		}else{
			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if ( add <= add_top ) {
			nextadd = add_btm;
		}else if( 0 >= cadrno(add) - 1 ){
			nextadd = add_max + (csesno(add) - 1) * 100L;
		}else{
			nextadd = add - 1;
		}
	}
	return nextadd;
}
/* ���e���L�[���͒l�͗L���A�h���X�H */
char	cisvalidadd( long add )
{
	char ret = 1;
	long min, max;
	short c_no, c_ad;
	long add_max;
	
	add_max = 0;
	max = 0;
	min = 0;
	if (gCurSrtPara == MNT_CAR) {
		min = CAR_START_INDEX + 1;
		max = CAR_START_INDEX + CAR_LOCK_MAX;
		add_max = CarAddMax;
	}
	else if (gCurSrtPara == MNT_INT_CAR){
		min = INT_CAR_START_INDEX + 1;
		max = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;
		add_max = IntCarAddMax;
	}
	else if (gCurSrtPara == MNT_BIK){
		min = BIKE_START_INDEX + 1;
		max = BIKE_START_INDEX + BIKE_LOCK_MAX;
		add_max = CarAddMax;
	}

	c_no = csesno(add) + DispAdjuster;
	c_ad = cadrno(add);

	if( ( min > c_no ) || ( max < c_no ) || ( c_no == 0 ) ||
		( add_max < c_ad ) || ( c_ad == 0 ) )
	{
		ret = 0;
	}
	return( ret );
}

/* �Ԏ��ݒ�ǎ� */
unsigned long	cprmread( short carno, short caradd )
{
	ulong	ret = 0;

	switch( caradd ){
		case 1:
			ret = (ulong)LockInfo[carno-1].lok_syu;
			break;
		case 2:
			ret = (ulong)LockInfo[carno-1].ryo_syu;
			break;
		case 3:
			ret = (ulong)LockInfo[carno-1].area;
			break;
		case 4:
			ret = (ulong)LockInfo[carno-1].posi;
			break;
		case 5:
			ret = (ulong)LockInfo[carno-1].if_oya;
			break;
		case 6:
			if (gCurSrtPara == MNT_BIK) {	// ���ւ̏ꍇ�͂��̂܂�
				ret = (ulong)LockInfo[carno-1].lok_no;
			} else {						// ���Ԃ̏ꍇ�A�ϊ����K�v
				ret = (ulong)LockInfo[carno-1].lok_no + PRM_LOKNO_MIN;	// �\���p��+100������
			}
			break;
		default:
			break;
	}

	return ret;
}

/* �Ԏ��ݒ菑���� */
void	cprmwrit( short carno, short caradd, long data )
{
	switch( caradd ){
		case 1:
			LockInfo[carno-1].lok_syu = (uchar)data;
			break;
		case 2:
			LockInfo[carno-1].ryo_syu = (uchar)data;
			break;
		case 3:
			LockInfo[carno-1].area = (uchar)data;
			break;
		case 4:
			LockInfo[carno-1].posi = (ulong)data;
			break;
		case 5:
			LockInfo[carno-1].if_oya = (uchar)data;
			break;
		case 6:
			if (gCurSrtPara == MNT_BIK) {	// ���ւ̏ꍇ�͂��̂܂�
				LockInfo[carno-1].lok_no = (uchar)data;
			} else {						// ���Ԃ̏ꍇ�A�ϊ����K�v
				if( data < PRM_LOKNO_MIN || data > PRM_LOKNO_MAX ){	// �͈̓`�F�b�N( 100�`315 �L�� )
					LockInfo[carno-1].lok_no = 0;								// �͈͊O�̏ꍇ�A0(�ڑ������)��ۑ�
				} else {
					LockInfo[carno-1].lok_no = (uchar)(data - PRM_LOKNO_MIN);	// �͈͓��̏ꍇ�A100�������ĕۑ�����(�ۑ��̈悪1byte�̂���)
				}
			}
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ۯ����u���Ұ���ݒ菈��                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RPara_Set()                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : �ݒ�Ӱ�޽���OFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	RPara_Set( void )
{
	unsigned short	usRParaEvent;
	char	wk[2];
	char	org[2];
	T_FrmLockSettei	FrmLockSettei;
	char	f_DataChange;

	if (!(GetCarInfoParam() & 0x01)) {	// ���֏ꕹ�݂Ȃ��̏ꍇ
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[5] );							// "��ۯ�������Ұ���ݒ聄�@�@�@�@ "

		usRParaEvent = Menu_Slt( RPRMENU, RPARA_SET_TBL, (char)RPARA_SET_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usRParaEvent ){
			case RCKP_PARA:
				f_DataChange = 0;
				usRParaEvent = RckParWrite( &f_DataChange );
				if( 1 == f_DataChange ){							// changed parameter data
					wopelg( OPLOG_ROCKPARAWT, 0, 0 );			// ���엚��o�^
				}
				break;
			case RCKP_PRNT:
				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
					BUZPIPI();
					break;
				}
				FrmLockSettei.prn_kind = R_PRI;
				FrmLockSettei.Kikai_no = (ushort)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_LOCK_SETTEI, sizeof(T_FrmLockSettei), &FrmLockSettei );
				Ope_DisableDoorKnobChime();
				/* �v�����g�I����҂����킹�� */
				Lagtim(OPETCBNO, 6,1000);		/* 20sec timer start */
				for ( ; ; ) {
					usRParaEvent = StoF( GetMessage(), 1 );
					/* �v�����g�I�� */
					if((usRParaEvent&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					/* �^�C���A�E�g(20�b)���o */
					if (usRParaEvent == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* ���[�h�`�F���W */
					if (usRParaEvent == KEY_MODECHG) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if ( usRParaEvent == LCD_DISCONNECT ) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				break;
			case RCKP_DEFA:
				f_DataChange = 0;
				usRParaEvent = SetDefault(RCKP_DEFA, &f_DataChange);
				if( 1 == f_DataChange ){							// changed parameter data
				}
				break;
			case MOD_EXT:											// F5 Key
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return( usRParaEvent );
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)						
			case MOD_CHG:											// �ݒ�Ӱ�޽���OFF
				return( usRParaEvent );
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usRParaEvent == MOD_CHG ){
		if( usRParaEvent == MOD_CHG || usRParaEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return( usRParaEvent );
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ۯ�������Ұ���ݒ菑��������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RckParWrite()                                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0xfffe : F5 Key ( End Key )                       |*/
/*|              :       0xffff : �ݒ�Ӱ�޽���OFF                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	ROCK_MAX	6
#define	RLin		(gCurLoc+1)
#define	RckAddMax	6
#define	RPAR_TOP	101L						// ۯ�������Ұ���J�n���ڽ(01-���ڽ01)
#define	RPAR_BTM	(ROCK_MAX*100)+RckAddMax	// ۯ�������Ұ���I�����ڽ(06-���ڽ06)

unsigned short	RckParWrite( char * p_f_DataChange )
{
	unsigned short	msg;
	unsigned char	ddd[6];		//work buffer
	unsigned char	wk[12];		//Display address
	unsigned short	ret;
	long			no;			//Current address pointer
	long			address;	//Input address
	long			data;		//Input data
	char			r_md;		//Cursol position

	address = -1;
	data = -1;
	r_md = 0;
	ret = 0;
	no = RPAR_TOP;

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[5] );		// [05]	"��ۯ�������Ұ���ݒ聄�@�@�@�@ "
	Fun_Dsp( FUNMSG[29] );												// [29]	"  �{  �|�^��  ��   ���  �I�� "
																		// [30]	"       ����   ��   ���  �I�� "
																		//		   F1    F2    F3    F4    F5 
	gCurLoc = 0;
	gPreAdr = 0;
	rck_pra_dp( no, 0, 2 );

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:
				BUZPI();
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:						//F1 address"-"
				BUZPI();
				if( r_md == 0 ){
					BUZPI();
					address = -1;
					no = rnextadd( no, -1 );
					rck_pra_dp( no, 0, 1 );
				}else{							//F1 Write
					data = -1;
					no = rnextadd( no, -1 );		//F1 address"-"
					rck_pra_dp( no, 1, 1 );
				}
				break;

			case KEY_TEN_F2:
				if( r_md == 0 ){				//F2 address"+"
					if( address == -1 ){
						BUZPI();
						no = rnextadd( no, 1 );
						rck_pra_dp( no, 0, 0 );
					}else {						//F2 address set"XXXXXX"
						if( risvalidadd( address ) ){
							BUZPI();
							no = address;
							gCurLoc = 0;
							rck_pra_dp( no, 0, 2 );
						}else {
							BUZPIPI();
						}
						address = -1;
					}
				}else{							//F2 address"+"
					BUZPI();
					data = -1;
					no = rnextadd( no, 1 );
					rck_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_F3:						//F3 Cursol shift"<-->"
				BUZPI();
				data = -1;
				address = -1;
				if( r_md == 0 ){
					r_md = 1;
					Fun_Dsp( FUNMSG[30] );
					rck_pra_dp( no, 1, 2 );
				}else {
					r_md = 0;
					Fun_Dsp( FUNMSG[29] );
					rck_pra_dp( no, 0, 2 );
				}
				break;

			case KEY_TEN_F4:						//F4
				if( r_md != 0 ){
					BUZPI();
					if( data != -1 ){
						rprmwrit( rsesno(no), radrno(no), data );
						*p_f_DataChange = 1;
					}
					data = -1;
					no = rnextadd( no, 1 );		//F1 address"+"
					rck_pra_dp( no, 1, 0 );
				}
				break;

			case KEY_TEN_CL:						// Clear
				BUZPI();
				if( r_md == 0 ){
					if( address != -1 ){
						address = -1;
					}
					//section no
					intoas( ddd, (ushort)rsesno(no), 2 );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;	/* "�|" */
					wk[5] = 0x7c;
					//address
					intoas( ddd, (ushort)radrno(no), 2 );
					as1chg( ddd, &wk[6], 2 );
					grachr ( (unsigned short)RLin,  2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					if( data != -1 ){
						data = -1;
					}
					intoasl( ddd, rprmread( rsesno(no), radrno(no) ), 6 );
					as1chg( ddd, wk, 6 );
					grachr( (unsigned short)RLin, 16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;

			case KEY_TEN:
				BUZPI();
				if( r_md == 0 ){
					address = ( address == -1 ) ? (short)(msg - KEY_TEN0):
						( address % 1000 ) * 10 + (short)(msg - KEY_TEN0);//@
					//section no
					intoas( ddd, (ushort)rsesno(address), 2 );
					as1chg( ddd, wk, 2 );
					//"-"
					wk[4] = 0x81;	/* "�|" */
					wk[5] = 0x7c;
					//address
					intoas( ddd, (ushort)radrno(address), 2 );
					as1chg( ddd, &wk[6], 2 );
					grachr ( (unsigned short)RLin,  2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}else {
					data = ( data == -1 ) ? (short)(msg - KEY_TEN0) :
											( data % 100000L ) * 10 + (short)(msg - KEY_TEN0);
					intoasl( ddd, (unsigned long)data, 6 );
					as1chg ( ddd, wk, 6 );
					grachr ( (unsigned short)RLin,  16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, wk );
				}
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����Ұ���ݒ�ǎ揈��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rck_pra_dp()                                            |*/
/*| PARAMETER    : pno : Parameter No. ( 1-01 �` 540-13 )                  |*/
/*|                mod : Cursor Disp Position  0 = Parameter No.           |*/
/*|                                            1 = Parameter Data          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Tanaka                                                  |*/
/*| Date         : 2005-04-27                                              |*/
/*| Update       : �s��۰ٕ����ɂ���                                       |*/
/*|              : 2005-06-15 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	rck_pra_dp( long pno, short mod, short direction )
{
	short			i;
	long			incadd;
	unsigned short	lincnt;
	unsigned char	work[22];
	unsigned char	ddd[6];

	// incadd<=�ŏ�i�ɕ\��������ڽ�����߂�B
	incadd = pno;
	if( rsesno(gPreAdr) != rsesno(pno) ){
		if( direction == 1 )
		{	// -����
			incadd = pno - 5;
		}
	}else{
		if( direction == 0 )
		{	// +����
			if( gCurLoc >= 5 ){
				incadd = pno - 5;
			}else{
				incadd = pno - gCurLoc - 1;
			}
		}else if( direction == 1 )
		{	// -����
			if( gCurLoc != 0 ){
				incadd = pno - gCurLoc + 1;
			}
		}
		else
		{	// ���E�ړ�
			incadd = pno - gCurLoc;
		}
	}
	lincnt = ( RckAddMax < radrno( incadd + 5 ) ) ?
				 ( RckAddMax - radrno(incadd) + 1 ) : 6;

	for( i = 0; i < 6; i++ ){

		if( i >= lincnt ){
			displclr((unsigned short)(i + 1));
			continue;
		}

		//car no
		intoas( ddd, (ushort)rsesno(incadd), 2 );
		as1chg( ddd, work, 2 );
		//"-"
		work[4] = 0x81;	/* "�|" */
		work[5] = 0x7c;
		//address
		intoas( ddd, (ushort)radrno(incadd), 2 );
		as1chg( ddd, &work[6], 2 );
		//data
		intoasl( ddd, rprmread( rsesno(incadd), radrno(incadd) ), 6 );
		as1chg( ddd, &work[10], 6 );

		if( pno == incadd ){

			gCurLoc = i; // ���ݶ��وʒu����

			if( mod == 0 ){
				grachr( (unsigned short)(i+1), 2, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, work );		// car no
				grachr( (unsigned short)(i+1), 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );	// data
			} else {
				grachr( (unsigned short)(i+1), 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 16, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );
			}
		} else {
				grachr( (unsigned short)(i+1), 2, 10, 0,  COLOR_BLACK, LCD_BLINK_OFF, work );
				grachr( (unsigned short)(i+1), 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, &work[10] );
		}

		incadd = rnextadd(incadd, 1);
	}
	gPreAdr = pno;	// �O����ڽ����
	return;
}

/* �Ԏ�����Ԃ� */
short	rsesno( long address )
{
	return (short)(address/100L);
}

/* �A�h���X��Ԃ� */
short	radrno( long address )
{
	return (short)(address%100L);
}

/* ���O��̃A�h���X(6digi)��Ԃ� */
long	rnextadd( long add, int mode )
{

	long nextadd = 0;

	if( 1 == mode ){

		if( add >= RPAR_BTM ){

			nextadd = RPAR_TOP;

		}else if( RckAddMax < radrno(add) + 1 ){

			nextadd = 1 + ((rsesno(add) + 1) * 100L);

		}else{

			nextadd = add + 1;
		}

	}else if( -1 == mode ){

		if( add <= RPAR_TOP ){

			nextadd = RPAR_BTM;

		}else if( 0 >= radrno(add) - 1 ){

			nextadd = RckAddMax + (rsesno(add) - 1) * 100L;

		}else{

			nextadd = add - 1;
		}

	}
	return nextadd;
}
/* ���e���L�[���͒l�͗L���A�h���X�H */
char	risvalidadd( long add )
{
	char ret = 1;

	if( ( ROCK_MAX < rsesno(add) ) ||
		( rsesno(add) == 0 ) ||
		( RckAddMax < radrno(add) ) ||
		( radrno(add) == 0 ) )
	{
		ret = 0;
	}
	return( ret );
}

/* �Ԏ��ݒ�ǎ� */
unsigned long	rprmread( short rckno, short rckadd )
{
	ulong	ret = 0;

	switch( rckadd ){
		case 1:
			ret = (ulong)LockMaker[rckno-1].in_tm;
			break;
		case 2:
			ret = (ulong)LockMaker[rckno-1].ot_tm;
			break;
		case 3:
			ret = (ulong)LockMaker[rckno-1].r_cnt;
			break;
		case 4:
			ret = (ulong)LockMaker[rckno-1].r_tim;
			break;
		case 5:
			ret = (ulong)LockMaker[rckno-1].open_tm;
			break;
		case 6:
			ret = (ulong)LockMaker[rckno-1].clse_tm;
			break;
	}

	return ret;
}

/* �Ԏ��ݒ菑���� */
void	rprmwrit( short rckno, short rckadd, long data )
{
	switch( rckadd ){
		case 1:
			LockMaker[rckno-1].in_tm = (uchar)data;
			break;
		case 2:
			LockMaker[rckno-1].ot_tm = (uchar)data;
			break;
		case 3:
			LockMaker[rckno-1].r_cnt = (uchar)data;
			break;
		case 4:
			LockMaker[rckno-1].r_tim = (ushort)data;
			break;
		case 5:
			LockMaker[rckno-1].open_tm = (uchar)data;
			break;
		case 6:
			LockMaker[rckno-1].clse_tm = (uchar)data;
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �p�����[�^�[������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetDefault( type )                                      |*/
/*| PARAMETER    : ushort mode : DEFA_PARA ��{�p�����[�^�[                |*/
/*|              :             : CARP_DEFA �Ԏ��p�����[�^�[                |*/
/*|              :             : RCKP_DEFA ���b�N��ʃp�����[�^�[          |*/
/*|              : char * p_f_DataChange : default��Ă����s�����׸�        |*/
/*|              :                         1=���s����, 0=���ĂȂ�          |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : �ݒ�Ӱ�޽���OFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SetDefault( ushort mode, char * p_f_DataChange )
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[14]);		/* "���f�t�H���g�Z�b�g���@�@�@�@�@" */
	if (mode == DEFA_PARA) {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[10]);	/* "�@���ʃp�����[�^���@�@�@�@�@�@" */
	}
	else if (mode == CARP_DEFA) {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[11]);	/* "�@�Ԏ��p�����[�^�[���@�@�@�@�@" */
	}
	else {	/* if (type == RCKP_DEFA) */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[12]);	/* "�@���b�N��ʃp�����[�^�[���@�@" */
	}
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[13]);		/* "�@�@�f�t�H���g�ɂ��܂����H�@�@" */
	Fun_Dsp(FUNMSG[19]);													/* "�@�@�@�@�@�@ �͂� �������@�@�@" */

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F3:	/* "�͂�" */
			/* �f�t�H���g�l���Z�b�g���� */
			if (mode == DEFA_PARA) {
				prm_clr(0, 1, 0);
			}
			else if (mode == CARP_DEFA) {
				lockinfo_clr(1);
			}
			else {	/* if (type == RCKP_DEFA) */
				lockmaker_clr(1);
			}
			*p_f_DataChange = 1;
		case KEY_TEN_F4:	/* "������" */
			BUZPI();
			return MOD_EXT;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���̔�}�X�N�f�[�^�̃A�h���X��Ԃ�                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_next                                             |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = ���̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_next( long add )
{
	long	ret = 0, tmp = 0;

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	tmp = nomask_ses_next( add );
// 	if( tmp == 0 ){
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	// �Y���f�[�^�Ȃ��̏ꍇ�A���̃Z�N�V������T��
	tmp = nomask_all_next( add );
	if( tmp == 0 ){
		// �Y���f�[�^�Ȃ��̏ꍇ�A�擪����T������
		ret = nomask_all_next( BPAR_TOP - 1 );
	}else{
		ret = tmp;
	}
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	}else{
// 		ret = tmp;
// 	}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �O�̔�}�X�N�f�[�^�̃A�h���X��Ԃ�                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_prev                                             |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = ���̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_prev( long add )
{
	long	ret = 0, tmp = 0;
	long	BPAR_BTM;
	BPAR_BTM = (C_PRM_SESCNT_MAX-1)*10000L;
	BPAR_BTM += CPrmCnt[C_PRM_SESCNT_MAX-1];

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	tmp = nomask_ses_prev( add );
// 	if( tmp == 0 ){
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	// �Y���f�[�^�Ȃ��̏ꍇ�A�O�̃Z�N�V������T��
	tmp = nomask_all_prev( add );
	if( tmp == 0 ){
		// �Y���f�[�^�Ȃ��̏ꍇ�A�Ō������T������
		ret = nomask_all_prev( BPAR_BTM + 1 );
	}else{
		ret = tmp;
	}
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	}else{
// 		ret = tmp;
// 	}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
	return ret;
}
/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�S�̂��玟�̔�}�X�N�f�[�^�̃A�h���X��Ԃ�                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_all_next                                         |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = ���̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_all_next( long add )
{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	long	ret = 0, ret31 = 0;
// 	short	i;
// 
// 	add = nextadd( add, 1 );
// 
// 	// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 	if( sesno(add) == S_RAT ){
// 		ret31 = nomask_ses_next31(add);
// 		if( ret31 ){
// 			// �Z�N�V�����R�P���Ŕ�}�X�N�A�h���X����������
// 			return ret31;
// 		}
// 		// ������Ȃ������ꍇ�͎��̃Z�N�V�����̐擪����ʏ�Ή��ɖ߂��Čp������
// 		add = 1 + (sesno(add)+1) * 10000L;
// 	}
// 
// 	for( i=0; i<prm_mask_max; i++ ){
// 		if( sesno(param_mask[i].from) < sesno(add) ){
// 			// �����Ώۂ��Z�N�V�����ԍ����������ꍇ�̓X�L�b�v����
// 			continue;
// 		}
// 		if( add < param_mask[i].from ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add <= param_mask[i].to ){
// 			// �}�X�N�ΏۂȂ̂ŁAto �̎��̃A�h���X��
// 			add = nextadd( param_mask[i].to, 1 );
// 			if( sesno(param_mask[i].from) > sesno(add) ){
// 				// ��������ꍇ�͊Y���f�[�^�Ȃ��Ƃ���
// 				break;
// 			}
// 
// 			// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 			if( sesno(add) == S_RAT ){
// 				ret31 = nomask_ses_next31(add);
// 				if( ret31 ){
// 					// �Z�N�V�����R�P���Ŕ�}�X�N�A�h���X����������
// 					return ret31;
// 				}
// 				// ������Ȃ������ꍇ�͎��̃Z�N�V�����̐擪����ʏ�Ή��ɖ߂��Čp������
// 				add = 1 + (sesno(add)+1) * 10000L;
// 			}
// 
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		if( i == prm_mask_max-1 ){
// 			// �Ō�̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	ret = 0;
	long	next_add = add;
	short	i;

	// �Z�N�V�������̎��̔�}�X�N�A�h���X�擾�����݂�
	next_add = nomask_ses_next(next_add);

	if ( next_add != 0 ) {	// �擾����
		ret = next_add;	// ��}�X�N�A�h���X
	} else {				// �擾���s
		for ( i = sesno(add) + 1; i < C_PRM_SESCNT_MAX; i++ ) {
			// �Z�N�V�������̍ŏ��̃A�h���X-1��ݒ�
			next_add = (i * 10000L);

			// ���Z�N�V�����̔�}�X�N�A�h���X�擾�����݂�
			next_add = nomask_ses_next(next_add);

			// �擾�����������}�X�N�A�h���X��Ԃ�
			if ( next_add != 0 ) {
				ret = next_add;	// ��}�X�N�A�h���X
				break;
			}
		}
	}

	return ret;
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�S�̂���O�̔�}�X�N�f�[�^�̃A�h���X��Ԃ�                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_all_prev                                         |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = �O�̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_all_prev( long add )
{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	long	ret = 0, ret31 = 0;
// 	short	i;
// 
// 	add = nextadd( add, -1 );
// 
// 	// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 	if( sesno(add) == S_RAT ){
// 		ret31 = nomask_ses_prev31(add);
// 		if( ret31 ){
// 			// �Z�N�V�����R�P���Ŕ�}�X�N�A�h���X����������
// 			return ret31;
// 		}
// 		// ������Ȃ������ꍇ�͑O�̃Z�N�V�����̍Ō������ʏ�Ή��ɖ߂��Čp������
// 		add = CPrmCnt[(sesno(add)-1)] + (sesno(add)-1) * 10000L;
// 	}
// 
// 	for( i=prm_mask_max-1; i>=0; i-- ){
// 		if( sesno(param_mask[i].from) > sesno(add) ){
// 			// �����Ώۂ��Z�N�V�����ԍ����傫���ꍇ�̓X�L�b�v����
// 			continue;
// 		}
// 		if( add > param_mask[i].to ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add >= param_mask[i].from ){
// 			// �}�X�N�ΏۂȂ̂ŁAfrom �̑O�̃A�h���X��
// 			add = nextadd( param_mask[i].from, -1 );
// 			if( sesno(param_mask[i].from) < sesno(add) ){
// 				// ��������ꍇ�͊Y���f�[�^�Ȃ��Ƃ���
// 				break;
// 			}
// 
// 			// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 			if( sesno(add) == S_RAT ){
// 				ret31 = nomask_ses_prev31(add);
// 				if( ret31 ){
// 					// �Z�N�V�����R�P���Ŕ�}�X�N�A�h���X����������
// 					return ret31;
// 				}
// 				// ������Ȃ������ꍇ�͑O�̃Z�N�V�����̍Ō������ʏ�Ή��ɖ߂��Čp������
// 				add = CPrmCnt[(sesno(add)-1)] + (sesno(add)-1) * 10000L;
// 			}
// 
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		if( i == 0 ){
// 			// �擪�̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	ret = 0;
	long	prev_add = add;
	short	i;

	// �Z�N�V�������̑O�̔�}�X�N�A�h���X�擾�����݂�
	prev_add = nomask_ses_prev(prev_add);

	if ( prev_add != 0 ) {	// �擾����
		ret = prev_add;	// ��}�X�N�A�h���X
	} else {				// �擾���s
		for ( i = sesno(add) - 1; i > 0; i-- ) {
			// �Z�N�V�������̍Ō�̃A�h���X+1��ݒ�
			prev_add = (i * 10000L) + CPrmCnt[i] + 1;

			// �O�Z�N�V�����̔�}�X�N�A�h���X�擾�����݂�
			prev_add = nomask_ses_prev(prev_add);

			// �擾�����������}�X�N�A�h���X��Ԃ�
			if ( prev_add != 0 ) {
				ret = prev_add;	// ��}�X�N�A�h���X
				break;
			}
		}
	}

	return ret;
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z�N�V�������Ŏ��̔�}�X�N�A�h���X��Ԃ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_ses_next                                         |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = ���̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_ses_next( long add )
{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	long	ret = 0, keep = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 
// 	add = nextadd( add, 1 );
// 	if( sesno(add) != sno ){
// 		// �Z�N�V�����ԍ�����v���Ȃ��ꍇ�͊Y���f�[�^�Ȃ�
// 		return ret;
// 	}
// 
// 	// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 	if( sno == S_RAT ){
// 		ret = nomask_ses_next31(add);
// 		return ret;
// 	}
// 
// 	// �Y���Z�N�V�����Ƀ}�X�N���ꌏ���Ȃ��ꍇ�ɔ����ĕۑ�����
// 	keep = add;
// 
// 	for( i=0; i<prm_mask_max; i++ ){
// 		if( sesno(param_mask[i].from) < sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����������ꍇ�̓X�L�b�v����
// 			continue;
// 		}
// 		if( sesno(param_mask[i].from) > sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����傫���ꍇ�i�Z�N�V�����̐ؑւ�j
// 			if( keep ){
// 				// �ێ��A�h���X������Ζ߂�l�Ƃ��ĕԂ��B�Ȃ���ΊY���f�[�^�Ȃ�
// 				ret = add;
// 			}
// 			break;
// 		}
// 		// �ȉ��A�}�X�N����
// 		keep = 0;
// 		if( add < param_mask[i].from ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add <= param_mask[i].to ){
// 			// �}�X�N�ΏۂȂ̂ŁAto �̎��̃A�h���X��
// 			add = nextadd( param_mask[i].to, 1 );
// 			if( sesno(add) != sno ){
// 				// �Z�N�V�������ς�����ׁA�Y���f�[�^�Ȃ�
// 				break;
// 			}
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		// �Z�N�V�����̐ؑւ���l�����Ĉ�U�ۑ�
// 		keep = add;
// 		if( i == prm_mask_max-1 ){
// 			// �Ō�̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	next_add = 0;

	// ���̃A�h���X���擾
	next_add = nextadd(add, 1);

	// �Z�N�V�������؂�ւ����
	if ( sesno(next_add) != sesno(add) ) {
		return 0;	// �Y���f�[�^�Ȃ�
	}

	// ���̔�}�X�N�A�h���X���擾
	next_add = get_nomask_ses_add(next_add, 1);

	return next_add;
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z�N�V�������őO�̔�}�X�N�A�h���X��Ԃ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_ses_prev                                         |*/
/*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = �O�̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_ses_prev( long add )
{
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	long	ret = 0, keep = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 
// 	add = nextadd( add, -1 );
// 	if( sesno(add) != sno ){
// 		// �Z�N�V�����ԍ�����v���Ȃ��ꍇ�͊Y���f�[�^�Ȃ�
// 		return ret;
// 	}
// 
// 	// �Z�N�V�����R�P�̏ꍇ�͕ʑΉ�
// 	if( sno == S_RAT ){
// 		ret = nomask_ses_prev31(add);
// 		return ret;
// 	}
// 
// 	// �Y���Z�N�V�����Ƀ}�X�N���ꌏ���Ȃ��ꍇ�ɔ����ĕۑ�����
// 	keep = add;
// 
// 	for( i=prm_mask_max-1; i>=0; i-- ){
// 		if( sesno(param_mask[i].from) > sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����傫���ꍇ�̓X�L�b�v����
// 			continue;
// 		}
// 		if( sesno(param_mask[i].from) < sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����������ꍇ�i�Z�N�V�����̐ؑւ�j
// 			if( keep ){
// 				// �ێ��f�[�^������Ζ߂�l�Ƃ��ĕԂ��B�Ȃ���ΊY���f�[�^�Ȃ�
// 				ret = add;
// 			}
// 			break;
// 		}
// 		// �ȉ��A�}�X�N����
// 		keep = 0;
// 		if( add > param_mask[i].to ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add >= param_mask[i].from ){
// 			// �}�X�N�ΏۂȂ̂ŁAfrom �̑O�̃A�h���X��
// 			add = nextadd( param_mask[i].from, -1 );
// 			if( sesno(add) != sno ){
// 				// �Z�N�V�������ς�����ׁA�Y���f�[�^�Ȃ�
// 				break;
// 			}
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		// �Z�N�V�����̐ؑւ���l�����Ĉ�U�ۑ�
// 		keep = add;
// 		if( i == 0 ){
// 			// �ŏ��̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
	long	prev_add = 0;

	// �O�̃A�h���X���擾
	prev_add = nextadd(add, -1);

	// �Z�N�V�������؂�ւ����
	if ( sesno(prev_add) != sesno(add) ) {
		return 0;	// �Y���f�[�^�Ȃ�
	}

	// ���̔�}�X�N�A�h���X���擾
	prev_add = get_nomask_ses_add(prev_add, -1);

	return prev_add;
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
}

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// /*[]----------------------------------------------------------------------[]*/
// /*| �Z�N�V�����R�P���Ŏ��̔�}�X�N�A�h���X��Ԃ�                           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : nomask_ses_next31                                       |*/
// /*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
// /*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
// /*|              :          : 0 �ȊO = ���̃A�h���X                        |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// long	nomask_ses_next31( long add )
// {
// 	long	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// �Z�N�V�����R�P�łȂ��ꍇ�͊Y���f�[�^�Ȃ���Ԃ�
// 	if( sno != S_RAT ){
// 		return ret;
// 	}
// 
// 	// �Z�N�V�����R�P�i�����ݒ�j�̑Ή�
// 	if( CPrmSS[S_CAL][1] == 0 ){
// 		// �����
// 		mask_max = prm_mask31_0_max;
// 		pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 	}else if( CPrmSS[S_CAL][1] == 1 ){
// 		// ������
// 		mask_max = prm_mask31_1_max;
// 		pmask = (PRM_MASK *)&param_mask31_1[0];
// 	}
// 
// 	for( i=0; i<mask_max; i++ ){
// 		if( add < pmask[i].from ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add <= pmask[i].to ){
// 			// �}�X�N�ΏۂȂ̂ŁAto �̎��̃A�h���X��
// 			add = nextadd( pmask[i].to, 1 );
// 			if( sesno(add) != sno ){
// 				// �Z�N�V�������ς�����ׁA�Y���f�[�^�Ȃ�
// 				break;
// 			}
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		if( i == mask_max-1 ){
// 			// �Ō�̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
// }
// 
// /*[]----------------------------------------------------------------------[]*/
// /*| �Z�N�V�����R�P���őO�̔�}�X�N�A�h���X��Ԃ�                           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : nomask_ses_prev31                                       |*/
// /*| PARAMETER    : long add : ���݂̃A�h���X�i�U���j                       |*/
// /*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
// /*|              :          : 0 �ȊO = �O�̃A�h���X                        |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// long	nomask_ses_prev31( long add )
// {
// 	long	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// �Z�N�V�����R�P�łȂ��ꍇ�͊Y���f�[�^�Ȃ���Ԃ�
// 	if( sno != S_RAT ){
// 		return 0;
// 	}
// 
// 	// �Z�N�V�����R�P�i�����ݒ�j�̑Ή�
// 	if( CPrmSS[S_CAL][1] == 0 ){
// 		// �����
// 		mask_max = prm_mask31_0_max;
// 		pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 	}else if( CPrmSS[S_CAL][1] == 1 ){
// 		// ������
// 		mask_max = prm_mask31_1_max;
// 		pmask = (PRM_MASK *)&param_mask31_1[0];
// 	}
// 
// 	for( i=mask_max-1; i>=0; i-- ){
// 		if( add > pmask[i].to ){
// 			// ��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 		if( add >= pmask[i].from ){
// 			// �}�X�N�ΏۂȂ̂ŁAfrom �̑O�̃A�h���X��
// 			add = nextadd( pmask[i].from, -1 );
// 			if( sesno(add) != sno ){
// 				// �Z�N�V�������ς�����ׁA�Y���f�[�^�Ȃ�
// 				break;
// 			}
// 		}
// 		// �}�X�N�͈͂𒴂���
// 		if( i == 0 ){
// 			// �ŏ��̗v�f�Ȃ�A��}�X�N�A�h���X�Ȃ̂Ŗ߂�l�Ƃ��ĕԂ�
// 			ret = add;
// 			break;
// 		}
// 	}
// 	return ret;
// }
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
//[]----------------------------------------------------------------------[]
///	@brief			�Z�N�V�������̔�}�X�N�A�h���X���擾(�}�X�N�f�[�^�����ʏ���)
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: ���݂̃A�h���X
///	@param[in]		mode		: ���[�h<br>
///								    1=���̔�}�X�N�A�h���X��Ԃ�<br>
///								    0=���݂̃A�h���X����}�X�N�A�h���X���m�F<br>
///								   -1=�O�̔�}�X�N�A�h���X��Ԃ�<br>
///	@param[in]		p_mask		: �}�X�N�f�[�^�e�[�u��
///	@param[in]		mask_max	: �}�X�N�f�[�^����
///	@return			ret			: 0 = �Y���Ȃ��A�}�X�N�A�h���X<br>
///								  0�ȊO = ��}�X�N�A�h���X<br>
///	@note			���݂̃A�h���X����`�F�b�N����
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long get_nomask_ses_add_common( long current_add, int mode, PRM_MASK* p_mask, short mask_max )
{
	long	ret = 0;
	long	next_add = current_add;
	short	current_ses = sesno(current_add);
	short	i;

	// �}�X�N�f�[�^�̃`�F�b�N
	if ( mode >= 0 ) {			// ���A�m�F
		for ( i = 0; i < mask_max; i++ ) {
			// �����Ώۂ��Z�N�V�����ԍ���������
			if ( sesno(p_mask[i].from) < sesno(next_add) ) {
				continue;	// �X�L�b�v
			}

			// �����Ώۂ��Z�N�V�����ԍ����傫��(����Z�N�V����)
			if ( sesno(p_mask[i].from) > sesno(next_add) ) {
				ret = next_add;	// ��}�X�N�A�h���X
				break;
			}

			// �A�h���X����}�X�N�A�h���X
			if ( next_add < p_mask[i].from ) {
				ret = next_add;	// ��}�X�N�A�h���X
				break;
			}

			// �A�h���X���}�X�N�A�h���X
			if ( next_add <= p_mask[i].to ) {
				// ���݂̃A�h���X�̊m�F�Ȃ炱���ŏI��
				if ( mode == 0 ) {
					return 0;	// �}�X�N�A�h���X
				}

				// �}�X�N�A�h���X�Ȃ̂Ń}�X�N�I���A�h���X�̎���ݒ�
				next_add = nextadd(p_mask[i].to, 1);

				// �Z�N�V�������؂�ւ����
				if ( sesno(next_add) != current_ses ) {
					return 0;	// �Y���f�[�^�Ȃ�
				}
			}
		}

		// �Ō�̗v�f
		if ( i == mask_max ) {
			ret = next_add;	// ��}�X�N�A�h���X
		}
	} else {					// �O
		for ( i = mask_max - 1; i >= 0; i-- ) {
			// �����Ώۂ��Z�N�V�����ԍ����傫��
			if ( sesno(p_mask[i].from) > sesno(next_add) ) {
				continue;	// �X�L�b�v
			}

			// �����Ώۂ��Z�N�V�����ԍ���������
			if ( sesno(p_mask[i].from) < sesno(next_add) ) {
				ret = next_add;	// ��}�X�N�A�h���X
				break;
			}

			// �A�h���X����}�X�N�A�h���X
			if ( next_add > p_mask[i].to ) {
				ret = next_add;	// ��}�X�N�A�h���X
				break;
			}

			// �A�h���X���}�X�N�A�h���X
			if ( next_add >= p_mask[i].from ) {
				// �}�X�N�A�h���X�Ȃ̂Ń}�X�N�I���A�h���X�̑O��ݒ�
				next_add = nextadd(p_mask[i].from, -1);

				// �Z�N�V�������؂�ւ����
				if ( sesno(next_add) != current_ses ) {
					return 0;	// �Y���f�[�^�Ȃ�
				}
			}
		}

		// �ŏ��̗v�f
		if ( i < 0 ) {
			ret = next_add;	// ��}�X�N�A�h���X
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�Z�N�V�������̔�}�X�N�A�h���X���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: ���݂̃A�h���X
///	@param[in]		mode		: ���[�h<br>
///								    1=���̔�}�X�N�A�h���X��Ԃ�<br>
///								    0=���݂̃A�h���X����}�X�N�A�h���X���m�F<br>
///								   -1=�O�̔�}�X�N�A�h���X��Ԃ�<br>
///	@return			ret			: 0 = �Y���Ȃ��A�}�X�N�A�h���X<br>
///								  0�ȊO = ��}�X�N�A�h���X<br>
///	@note			���݂̃A�h���X����`�F�b�N����
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long get_nomask_ses_add( long current_add, int mode )
{
	long	next_add = current_add;
	long	temp_add;
	int		check_flg = 0;
	short	current_ses = sesno(current_add);

	if ( OPECTL.Mnt_lev < 4 ) {	// �Z�p�Ҍ����ȉ�
		check_flg = 1;	// �`�F�b�N����
	}

	while ( check_flg ) {
		// �`�F�b�N�J�n���̃A�h���X��ێ�
		temp_add = next_add;

		// �Z�p�Ҍ����ł͐ݒ�֎~�Ƃ���A�h���X���}�X�N
		if ( current_ses == S_RAT ) {	// �Z�N�V����31(�����ݒ�)
			if ( CPrmSS[S_CAL][1] == 1 ) {	// ������
				next_add = get_nomask_ses_add_common(next_add, mode,
					(PRM_MASK*)&param_mask31_1[0], prm_mask31_1_max);
			} else {						// �����
				next_add = get_nomask_ses_add_common(next_add, mode,
					(PRM_MASK*)&param_mask31_0[0], prm_mask31_0_max);
			}
		} else {						// �Z�N�V����31(�����ݒ�)�ȊO
			next_add = get_nomask_ses_add_common(next_add, mode, 
				(PRM_MASK*)&param_mask[0], prm_mask_max);
		}

		// �N���E�h�����v�Z�ł͐ݒ�֎~�Ƃ���A�h���X���}�X�N
		if ( ( next_add != 0 ) && CLOUD_CALC_MODE ) {	// ��}�X�N�A�h���X���N���E�h�����v�Z���[�h
			next_add = get_nomask_ses_add_common(next_add, mode, 
				(PRM_MASK*)&param_mask_cc[0], prm_mask_max_cc);
		}

		if ( (next_add == 0) ||			// �Y���Ȃ��A�}�X�N�A�h���X
			 (next_add == temp_add) ) {	// �S�Ẵ`�F�b�N�Ŕ�}�X�N�A�h���X
			check_flg = 0;	// �`�F�b�N�I��
		}
	}

	return next_add;
}

//[]----------------------------------------------------------------------[]
///	@brief			��}�X�N�A�h���X���m�F
//[]----------------------------------------------------------------------[]
///	@param[in]		current_add	: ���݂̃A�h���X
///	@return			ret			: 0 = �Y���Ȃ��A�}�X�N�A�h���X<br>
///								  0�ȊO = ��}�X�N�A�h���X<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long is_nomask( long current_add )
{
	return get_nomask_ses_add(current_add, 0);
}
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

/*[]----------------------------------------------------------------------[]*/
/*| �J�[�\���ʒu�ɉ�������ʍŏ�i�̔�}�X�N�A�h���X��Ԃ�                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nomask_line_top                                         |*/
/*| PARAMETER    : long pno : ���݂̃A�h���X�i�U���j                       |*/
/*|              : short curloc : �J�[�\���ʒu                             |*/
/*| RETURN VALUE : long ret : 0 = �Y���f�[�^�Ȃ�                           |*/
/*|              :          : 0 �ȊO = �O�̃A�h���X                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long	nomask_line_top( long pno, short curloc )
{
	long	ret = 0, add = 0;
	short	i;

	add = pno;
	for( i=0; i<curloc; i++ ){
		// �J�[�\���ʒu����擪�s�܂ők���Ĕ�}�X�N�A�h���X���擾����
		add = nomask_ses_prev(add);
		if( add == 0 ){
			break;
		}
		ret = add;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ʕ\������s����Ԃ�                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_disp_line                                           |*/
/*| PARAMETER    : long add : �擪�s�̃A�h���X�i�U���j                     |*/
/*| RETURN VALUE : char line_cnt : �\���s��                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	get_disp_line( long add )
{
	char	line_cnt = 1;	// �Œ�P�s�͕\������
	short	i;

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	if(OPECTL.Mnt_lev < 4){
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
		for( i=0; i<4; i++ ){
			// �Z�N�V�������̎��̔�}�X�N�A�h���X���擾����
			add = nomask_ses_next(add);
			if( add == 0 ){
				// ���̊Y���f�[�^�Ȃ�
				break;
			}
			line_cnt++;
		}
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// 	}
// 	else{
// 		line_cnt = ( CPrmCnt[sesno(add)] < adrno(add + 4) ) ?
// 					 (CPrmCnt[sesno(add)] - adrno(add) + 1) : 5;
// 	}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)

	return line_cnt;
}

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
// /*[]----------------------------------------------------------------------[]*/
// /*| �w��A�h���X����}�X�N�A�h���X�����肷��                               |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : is_nomask                                               |*/
// /*| PARAMETER    : long add : �ݒ�A�h���X�i�U���j                         |*/
// /*| RETURN VALUE : char ret : 0 = �}�X�N����                               |*/
// /*|                         : 1 = �}�X�N���Ȃ� or �Y���f�[�^�Ȃ�           |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// char	is_nomask( long add )
// {
// 	char	ret = 0;
// 	short	sno = sesno(add);
// 	short	i;
// 	short	mask_max = 0;
// 	PRM_MASK	*pmask = 0;
// 
// 	// �Z�N�V�����R�P�H
// 	if( sno == S_RAT ){
// 		// �Z�N�V�����R�P�i�����ݒ�j�̑Ή�
// 		if( CPrmSS[S_CAL][1] == 0 ){
// 			// �����
// 			mask_max = prm_mask31_0_max;
// 			pmask = (PRM_MASK *)&param_mask31_0[0];
// 
// 		}else if( CPrmSS[S_CAL][1] == 1 ){
// 			// ������
// 			mask_max = prm_mask31_1_max;
// 			pmask = (PRM_MASK *)&param_mask31_1[0];
// 		}
// 	}else{
// 		mask_max = prm_mask_max;
// 		pmask = (PRM_MASK *)&param_mask[0];
// 	}
// 
// 	for( i=0; i<mask_max; i++ ){
// 		if( sesno(pmask[i].from) < sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����������ꍇ�̓X�L�b�v����
// 			continue;
// 		}
// 		if( sesno(pmask[i].from) > sno ){
// 			// �����Ώۂ��Z�N�V�����ԍ����傫���Ȃ����ꍇ�͔�}�X�N�A�h���X
// 			ret = 1;
// 			break;
// 		}
// 		if( add < pmask[i].from ){
// 			// ��}�X�N�A�h���X�Ȃ̂� OK ��Ԃ�
// 			ret = 1;
// 			break;
// 		}
// 		if( add <= pmask[i].to ){
// 			// �}�X�N�ΏۂȂ̂� NG ��Ԃ�
// 			break;
// 		}
// 	}
// // MH810104(S) R.Endo 2021/09/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6012 �y�ʕ����w�E�z60-0201�ȍ~�iQR/�o�[�R�[�h�t�H�[�}�b�g�p�^�[���ݒ�j�̃A�h���X�̃}�X�N���O���Ă��i��\�����\���j�W�����v�Ǐo�ł��Ȃ� 
// 	if ( i == mask_max ) {
// 		// �Y���f�[�^�Ȃ��Ȃ��}�X�N�A�h���X�Ȃ̂� OK ��Ԃ�
// 		ret = 1;
// 	}
// // MH810104(E) R.Endo 2021/09/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6012 �y�ʕ����w�E�z60-0201�ȍ~�iQR/�o�[�R�[�h�t�H�[�}�b�g�p�^�[���ݒ�j�̃A�h���X�̃}�X�N���O���Ă��i��\�����\���j�W�����v�Ǐo�ł��Ȃ� 
// 
// 	return ret;
// }
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
/*[]----------------------------------------------------------------------[]*/
/*| �ċN���ē�����ʕ\������                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PutGuideMessage                                         |*/
/*| PARAMETER    : long add : �ݒ�A�h���X�i�U���j                         |*/
/*	RETURN VALUE :	MOD_EXT	: F5 Key ( End Key )                           |*/
/*					MOD_CHG	: �ݒ�Ӱ�޽���OFF                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned short	PutGuideMessage( void )
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PMSTR1[3]);			/* "�����ʃp�����[�^�[�ݒ聄�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[21]);		/* "�@ ���Z�@�̍ċN�����K�v�ł� �@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[22]);		/* "�@�@ �d����OFF/ON���s���A �@�@" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[23]);		/* "�@���Z�@���ċN�����Ă��������@" */
	Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:												/* " �I�� " */
			BUZPI();
			return MOD_EXT;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �X�V�A�h���X���ċN���v�����ڂ����肷��                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : IsRebootRequest                                         |*/
/*| PARAMETER    : long add : �ݒ�A�h���X�i�U���j                         |*/
/*| RETURN VALUE : char ret : 0 = �ċN���s�v, 1 = �ċN���K�v               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	IsRebootRequest( long add )
{
	char	ret = 0;

	return ret;
}
