/*[]----------------------------------------------------------------------[]*/
/*| operation task control                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<machine.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"irq1.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"mnt_def.h"
#include	"AppServ.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"LKmain.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"tbl_rkn.h"
#include	"ifm_ctrl.h"
#include	"ope_ifm.h"
#include	"ntnetauto.h"

#include	"remote_dl.h"
#include	"can_api.h"
#include	"can_def.h"
#include	"rtc_readwrite.h"
#include	"I2c_driver.h"
#include	"bluetooth.h"
#include	"updateope.h"
#include	"strdef.h"
#include	"pip_def.h"
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"ntcom.h"
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"FlashSerial.h"
#include	"ftpctrl.h"
#include	"ifm_ctrl.h"
#include	"oiban.h"
#include	"rau.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

static void start_dsp( void );

static short DateTimeCheck( void );
static void	start_up_LD1( void );

extern	unsigned short	UsMnt_mnyctl( void );
extern	unsigned short	UsMnt_mnyctl2( void );
extern	void 	Ether_DTinit(void);
// MH810100(S) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�N���ʒm��M��OPE�^�X�N�����������܂ő҂�)
extern void PktStartUpStsChg( uchar faze );		// �N��������ԍX�V
// MH810100(E) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�N���ʒm��M��OPE�^�X�N�����������܂ő҂�)
// MH810100(S) K.Onodera  2020/01/14 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
extern void PktResetReqFlgSet( uchar rst_req );	// ���Z�b�g�v���t���O�Z�b�g
// MH810100(E) K.Onodera  2020/01/14 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:5;				// Bit 3-7 = �\��
		unsigned char	START_STATUS:1;		// Bit 2 = �N�������ޱ�̊J���
		unsigned char	TIMER_END:1;		// Bit 1 = �}�X�N�^�C�}�[�^�C���A�E�g
		unsigned char	MASK:1;				// Bit 0 = �U���Z���T�[�}�X�N��
	} BIT;
} t_mask_VIB_SNS;

t_mask_VIB_SNS	mask_VIB_SNS;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//static	char	auto_cntcom_phase_check( uchar* );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

void	ck_syuukei_ptr_zai( ushort LogKind );
extern	uchar	LogDataClr( ushort LogSyu );
#define	_RTC_INT_ENB()	{\
	P_INTC.ISCR.BIT.IRQ2SC = 2;				/* IRQ2 �����オ��edge */		\
	P_INTC.IER.BIT.IRQ2E = 1;				/* IRQ2 �����݋��� */			\
	wait2us( 5L );							/* wait 10us */					\
	if( P_INTC.ISR.BIT.IRQ2F ){				\
		P_INTC.ISR.BIT.IRQ2F = 0; 			/* IRQ2 Inservice flg clear */	\
	}										\
}
ushort	DoorLockTimer;
ulong	attend_no;
static uchar	StartupDoorArmDisable = 0;

static uchar Printer_Status_Get( uchar Type, uchar *NEW, uchar *OLD );
static void I2C_Err_Regist( unsigned short msg, unsigned char ErrData );
static	t_OpeKeyRepeat	OpeKeyRepeat;			// �����lOFF

void	Ope_KeyRepeat();
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//static	void	SoundInsProgram( void );
static	void		lcdbm_LcdIpPort_DefaultSetting( void );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
// MH810100(S) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
//static	void	SoundPlayProgram( void );
// MH810100(E) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
uchar	DisableDoorKnobChime = 0;		// �h�A�m�u�߂��Y��`���C���}�~�t���O
static uchar chk_fan_timectrl( void );
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
uchar Is_in_lagtim( void );		// ���Z���� + ׸���ю��� >= ���ݎ��Ԃ�����
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
static	void	kinko_syuukei(void);
unsigned char	Pri_Open_Status_R;			// �v�����^�J�o�[���
unsigned char	Pri_Open_Status_J;			// �v�����^�J�o�[���
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
//static	void	log_clear_all(void);
static	void	log_clear_all(uchar factor);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// �A�C�h���T�u�^�X�N�ֈړ�
//// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//static void LongTermParkingCheck( void );
//static short LongTermParking_stay_hour_check(flp_com *p, short hours);
//void	LongTermParkingCheck_Resend( void );
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
extern uchar	KSG_RauBkupAPNName[32];// APN�̐ݒ�l�̔�r�p�o�b�N�A�b�v FOMA���W���[���ɐݒ�ς݂̃f�[�^����r���邽�߂̂���
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
int		ModuleStartFlag;
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ������Ҳ�                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opetask( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void opetask( void )
{
	unsigned char	f_ErrAlm=0;										// b0:1=E0057(���v�ǂݍ��ݎ��s)����
	char	LOCKCLR = 0;											// ۯ�������Ұ���̫��
																	// b1:1=E0058(�@����ǂݍ��ݎ��s)����
	char	f_CPrmDefaultSet=0;										// 1=�������Ұ� default��ċN��
	uchar i;
	uchar log_ver_temp = 0;
	ushort interval;
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	uchar chg_info_ver_temp = 0;
// GG120600(e) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
// MH810105(s) 2022/11/07 K.A (���V�[�g�v�����^�ʐM���ł��Ȃ��΍�)
	ushort	delay;
// MH810105(e) 2022/11/07 K.A (���V�[�g�v�����^�ʐM���ł��Ȃ��΍�)

	uchar	remote_timer_set;
	uchar	f_parm_datchk;
	uchar	f_log_init = 0;// 1=���O�̂݃C�j�V�������{,2=�X�[�p�[�C�j�V�������{(���샂�j�^�o�^����p)
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
	ModuleStartFlag = 1;
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
	dog_init();														// Watch dog timer start

	dog_init();

	/*** ү���޸ر, �O�����͏����l�ǂݎ��, �O���o�͏����� ***/
	Ptout_Init();													// �o���߰ď�����
	ExIOSignalwt(EXPORT_JP_RES, 1);									// �W���[�i���v�����^���Z�b�g
	ExIOSignalwt(EXPORT_URES, 1);									// �W���[�i���v�����^���Z�b�g	

	ExIOSignalwt(EXPORT_M_LD0, 1 );									// �ʏ�FH�i�f�W�^���A���v�Q�C���F0�j�ݒ�

	Mcb_Init();														// Message buffer initial
	Tcb_Init();
	f_port_scan_OK = 0;												// ���̓|�[�g�̊��荞�݃X�L�����֎~

	wait2us(50000L);												// wait 100ms
	CP_CAN_RES = 0;													// CAN Reset ����
	CP_FB_RES = 1;													// FB-7000 Reset ����
	wait2us(50000L);												// wait 100ms
	dog_init();
	CAN1_init();													// �@����ʐM�|�[�g������
	/*** LED, LCD OFF ***/
	LedInit();														// LED initial(All OFF)
	dispinit();														// LCD initial
// MH810105(s) 2022/11/07 K.A (���V�[�g�v�����^�ʐM���ł��Ȃ��΍�)
	delay = 30;														// �R�b
	while(delay != 0) {
		wait2us(50000L);											// wait 100ms
		dog_init();
		delay--;
	}
// MH810105(e) 2022/11/07 K.A (���V�[�g�v�����^�ʐM���ł��Ȃ��΍�)

	/*** �����\�� ***/
	start_dsp();

	/*** ��ظر ***/
	prm_init( COM_PRM );											// �������Ұ��̊i�[���ڽ���擾
	prm_init( PEC_PRM );											// �����Ұ��̊i�[���ڽ���擾

	StartingFunctionKey = 0;
	I2C1_init();												
	Rspi_from_Init();
	Flash2Init();
// �N������ɉ����f�[�^�̗L���𔻒肵�ALED�_�łɂĒm�点�������߂ɉ����f�[�^�^�p�ʍX�V�����͐�ɍs�Ȃ�
	if( read_rotsw() == 4 ){										// ----- �X�[�p�[�C�j�V�����C�Y(RSW=4)? -----
		// �^�p�ʂ̃C�j�V�����C�Y�����͐�ɂ���Ă���
		update_sw_area_init();										// �^�p�ʏ�񏉊�������
	}
	get_from_swdata();												// FROM�̉^�p�ʏ����擾��RAM�ɓW�J����
	wave_data_swupdate();											// �����f�[�^�^�p�ʍX�V����
	f_wave_datchk = wave_data_swchk();								// �����f�[�^�^�p�ʃ`�F�b�N����
	start_up_LD1();													// �N�����̃v���O�����A�����L����LED�_�ŏ���

	log_ver_temp = FLT_read_log_version();
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	chg_info_ver_temp = FLT_read_chg_info_version();
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
//	if( (LOG_VERSION != log_ver_temp) ||// ���O�o�[�W�����s��v
//	    (memcmp( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM )) != 0) ){// �p�X���[�hNG
//		// FROM�Ɋi�[����Ă��郍�O�o�[�W������SRAM�Ɋi�[����
//		bk_log_ver = log_ver_temp;
//		// �v���O�����̃��O�o�[�W������FROM�Ɋi�[����
//		FLT_write_log_version(LOG_VERSION);
//		// ���O�p�X���[�h�Z�b�g
//		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );
//
//		// �o�b�N�A�b�v�f�[�^�̍\���̂̂݃T�C�Y�ύX�����ꍇ�A
//		// case���Ƀ��O�o�[�W������ǉ����A���O���������s��Ȃ��悤�ɂ��邱��
//		switch (bk_log_ver) {
////		case XX:
////			break;
//		default:
//			log_clear_all();
//			f_log_init = 1;											// ���O�C�j�V�������{
//			break;
//		}
	if( memcmp( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM )) != 0 ){// �p�X���[�hNG
		// ���O�̊Ǘ��G���A�����Ă���̂Ŗ������ɏ���������
		// FROM�Ɋi�[����Ă��郍�O�o�[�W������SRAM�Ɋi�[����
		bk_log_ver = log_ver_temp;
		// �v���O�����̃��O�o�[�W������FROM�Ɋi�[����
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//		FLT_write_log_version(LOG_VERSION);
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
		// ���O�p�X���[�h�Z�b�g
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );
		log_clear_all(1);	// �p�X���[�hNG�ɂ��N���A
		f_log_init = 1;											// ���O�C�j�V�������{
	}else if( LOG_VERSION != log_ver_temp ){// ���O�o�[�W�����s��v
		// FROM�Ɋi�[����Ă��郍�O�o�[�W������SRAM�Ɋi�[����
		bk_log_ver = log_ver_temp;
		// �v���O�����̃��O�o�[�W������FROM�Ɋi�[����
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//		FLT_write_log_version(LOG_VERSION);
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
		// ���O�p�X���[�h�Z�b�g
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );

		// �o�b�N�A�b�v�f�[�^�̍\���̂̂݃T�C�Y�ύX�����ꍇ�A
		// case���Ƀ��O�o�[�W������ǉ����A���O���������s��Ȃ��悤�ɂ��邱��
		switch (bk_log_ver) {
		default:
			log_clear_all(0);	// ���O�o�[�W�����s��v�ɂ��N���A
			f_log_init = 1;											// ���O�C�j�V�������{
			break;
		}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
// MH810103(s) �o�[�W�����A�b�v��P��ڂ̕��d���ɂ������X�g�A�ł��Ȃ��s��C��
//	}else if( bk_log_ver < 5 && log_ver_temp < 5){
//		// ���O�o�[�W������5�����ł́Abk_log_ver��0�̂͂������AFROM�o�[�W������LOG_VERSION����v���Ă���̂ɁA5�����̏ꍇ�̓Z�b�g�������B
//		// ���O�o�[�W������5�ȏ�́Abk_log_ver�ɐ������l�������Ă���͂��ł���B
//		bk_log_ver = log_ver_temp;
// MH810103(e) �o�[�W�����A�b�v��P��ڂ̕��d���ɂ������X�g�A�ł��Ȃ��s��C��
	} 
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	if( CHG_INFO_VERSION != chg_info_ver_temp ){// chg_info�o�[�W�����s��v)
		switch (chg_info_ver_temp) {
		case 0xFF:// GG120600���O��0xFF
			remotedl_chg_info_log0_to1();
			break;
		default:
			break;
		}
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
	}
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	TENKEY_F1 = 0;
	TENKEY_F2 = 0;
	TENKEY_F3 = 0;
	TENKEY_F4 = 0;
	TENKEY_F5 = 0;
	switch (read_rotsw()) {
	case 1:		// �������[�C�j�V����
		TENKEY_F1 = 1;
		TENKEY_F3 = 1;
		break;
	case 2:		// �p�����[�^�f�t�H���g�Z�b�g
		TENKEY_F2 = 1;
		TENKEY_F4 = 1;
		break;
	case 3:		// �������[�C�j�V�����{�p�����[�^�f�t�H���g�Z�b�g
		TENKEY_F3 = 1;
		TENKEY_F5 = 1;
		break;
// MH810100(S) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
//	case 7:
//		OPECTL.Mnt_mod = 5;
//		break;
//	case 8:		// ۯ����u���ؽı�v��(F1&F2&F3 ON)?   AppServ_FukudenProc()�ŕK�v
//		TENKEY_F1 = 1;
//		TENKEY_F2 = 1;
//		TENKEY_F3 = 1;
//		break;
// MH810100(E) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
	default:
		break;
	}
	if( remotedl_update_chk() || 									// ----- �v���O�����X�V��H -----
		(( TENKEY_F1 == 1 )&&( TENKEY_F3 == 1 )) ){					// ----- RAM�ر�v���P(F1&F3 ON)? -----
		Log_Write_Pon();// ���O�Ǘ����d����(�S��)
		FLT_WriteLog_Pon();// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
		StartingFunctionKey = 2;
		RAMCLR = memclr( 1 );										// SRAM�������i�������ر�j
		prm_clr( PEC_PRM, 1, 0 );									// �����Ұ���̫��
		LOCKCLR = 1;												// ۯ�������Ұ���̫��
	}
	else if( ( TENKEY_F2 == 1 )&&( TENKEY_F4 == 1 ) ){				// ----- RAM�ر�v���Q(F2&F4 ON)? -----
		Log_Write_Pon();// ���O�Ǘ����d����(�S��)
		FLT_WriteLog_Pon();// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
		StartingFunctionKey = 3;
		NtNet_FukudenParam.DataKind = 0;
		prm_clr( COM_PRM, 1, 0 );									// �������Ұ���̫��
		RAMCLR = memclr( 0 );										// SRAM�������i�����t���ر�j
		lockinfo_clr(1);											// �Ԏ����Ұ���̫��
		LOCKCLR = 1;												// ۯ�������Ұ���̫��
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ���Ұ����̫�ľ��       "�\��
		f_CPrmDefaultSet = 1;										// 1=�������Ұ� default���
		f_ParaUpdate.BYTE = 0;										// ��̫�ľ�Ă���̂ŁA�t���O���N���A
	}
	else if( ( TENKEY_F3 == 1 )&&( TENKEY_F5 == 1 ) ){				// ----- RAM�ر�v���R(F3&F5 ON)? -----
		Log_Write_Pon();// ���O�Ǘ����d����(�S��)
		FLT_WriteLog_Pon();// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
		StartingFunctionKey = 1;
		NtNet_FukudenParam.DataKind = 0;
		log_init();
		prm_clr( COM_PRM, 1, 0 );									// �������Ұ���̫��
		prm_clr( PEC_PRM, 1, 0 );									// �����Ұ���̫��
		RAMCLR = memclr( 1 );										// SRAM�������i�������ر�j
		lockinfo_clr(1);											// �Ԏ����Ұ���̫��
		LOCKCLR = 1;												// ۯ�������Ұ���̫��
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ���Ұ����̫�ľ��       "�\��
		f_CPrmDefaultSet = 1;										// 1=�������Ұ� default���
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// �f�t�H���g�Z�b�g���O�o�^����
		f_ParaUpdate.BYTE = 0;										// ��̫�ľ�Ă���̂ŁA�t���O���N���A
	}
	else if( read_rotsw() == 4 ){				// ----- �X�[�p�[�C�j�V�����C�Y(RSW=4)? -----
		Flash2ChipErase();											// FROM�S�̈����
		for( i = 0; i < eLOG_MAX; i++ ){
			memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
		}
		Log_clear_log_bakupflag();// LOG�f�[�^�o�^���d�����p�̃t���O�A�ޔ��f�[�^�N���A����
		for( i = 0; i < eLOG_MAX; i++ ){
			BR_LOG_SIZE[i] = LogDatMax[i][0];
		}
		FLT_req_tbl_clear();// ���O�T�C�Y��񏑂����݂̑O�ɗv�����N���A����K�v������
		FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ���O�T�C�Y��񏑂�����
		// FLT_WriteBRLOGSZ()��_flt_EnableTaskChg()���R�[������A�^�X�N�`�F���W�̔�����}�~����
		_flt_DisableTaskChg();
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//		FLT_write_log_version(LOG_VERSION);							// �v���O�����̃��O�o�[�W������FROM�Ɋi�[
		FLT_write_log_version(LOG_VERSION,CHG_INFO_VERSION);
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
		f_log_init = 2;												// �X�[�p�[�C�j�V�������{(���샂�j�^�o�^����p)
		memcpy( LOG_PASS_RAM, LOG_PASS_ROM, sizeof( LOG_PASS_RAM ) );	// ���O�p�X���[�h�Z�b�g
		StartingFunctionKey = 1;
		NtNet_FukudenParam.DataKind = 0;

		log_init();
		prm_clr( COM_PRM, 1, 0 );									// �������Ұ���̫��
		prm_clr( PEC_PRM, 1, 0 );									// �����Ұ���̫��
		RAMCLR = memclr( 1 );										// SRAM�������i�������ر�j
		lockinfo_clr(1);											// �Ԏ����Ұ���̫��
		LOCKCLR = 1;												// ۯ�������Ұ���̫��
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[46] );	// "       ���Ұ����̫�ľ��       "�\��
		f_CPrmDefaultSet = 1;										// 1=�������Ұ� default���
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// �f�t�H���g�Z�b�g���O�o�^����
		f_ParaUpdate.BYTE = 0;										// ��̫�ľ�Ă���̂ŁA�t���O���N���A
	}
	else if( read_rotsw() == 5 ){
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
// rotsw=5��LCD�ʐM�ݒ���ި̫�Đݒ�Ɋ֘A���鏈����ǉ�
//		SoundInsProgram();

		// ���O�Ǘ����d����(�S��)
		Log_Write_Pon();

		// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
		FLT_WriteLog_Pon();

		// SRAM�������i�����t���ر�j
		RAMCLR = memclr( 0 );

		// LCD�ʐM�ݒ���ި̫��(RomTable = cprm_rec)�Ή�
		lcdbm_LcdIpPort_DefaultSetting();

// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X (#4014 RSW=5���ݒ�X�V���󎚂ɋL�^����Ȃ�)
		SetSetDiff( SETDIFFLOG_SYU_IP_DEF );	// IP�f�t�H���g�Z�b�g���O�o�^����
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X (#4014 RSW=5���ݒ�X�V���󎚂ɋL�^����Ȃ�)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	}
// MH810100(S) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
//	else if( read_rotsw() == 6 ){
//		SoundPlayProgram();
//	}
// MH810100(E) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
	else if( read_rotsw() == 9 ){
		Log_Write_Pon();// ���O�Ǘ����d����(�S��)
		FLT_WriteLog_Pon();// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
		for(i = 0 ;i < FAN_EXE_BUF ;i++){							// FAN�쓮���O�̏�����
			fan_exe_time.fan_exe[i].year = 0;
			fan_exe_time.fan_exe[i].mont = 0;
			fan_exe_time.fan_exe[i].date = 0;
			fan_exe_time.fan_exe[i].hour = 0;
			fan_exe_time.fan_exe[i].minu = 0;
			fan_exe_time.fan_exe[i].f_exe = 0;
			fan_exe_time.index = 0;
		}
	}
	else{															// ----- �ʏ헧�グ�� -----
		Log_Write_Pon();// ���O�Ǘ����d����(�S��)
		FLT_WriteLog_Pon();// ���O�Ǘ����d����(FROM�̏������ݕ����̂�)
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		NTNET_UpdateParam(&NtNet_FukudenParam);
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		RAMCLR = memclr( 0 );										// SRAM�������i�����t���ر�j
	}
	clr_req = 0;
	if (FLAGPT.memflg != _MEMFLG_VAL || RAMCLR) {
		FLAGPT.memflg = _MEMFLG_VAL;
		clr_req = _CLR_ALLMEM;
	}
	parm_data_swupdate();											// ���ʃp�����[�^�^�p�ʍX�V����
	f_parm_datchk = parm_data_swchk();								// ���ʃp�����[�^�^�p�ʃ`�F�b�N����

	if( RAMCLR ){
		grachr( 6, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[47] );	// "          ��ذ�Ƽ��           "�\��
	}

	dog_init();
	Ether_DTinit();
	CLK_REC_msec = 0;		// ���v�����̑O�ɃN���A
	_ei();															// enable interrupts

	AppServ_FukudenProc( RAMCLR, f_CPrmDefaultSet );				// FlashRom����n ���d����
	IFM_Init((int)RAMCLR);
// Note:���O�̎擾���Ƀ^�X�N�؂芷�����s����ƁA���O���o�^�����\��������
// 		�^�X�N�؂�ւ���������Flash�ɃA�N�Z�X���邽�߁Aflatask��_flt_DisableTaskChg()���g�p
	_flt_DisableTaskChg();					// �^�X�N�؂�ւ� �֎~
	IFM_RcdBufClrAll();						// �f�[�^�e�[�u���̏�����
	IFM_Accumulate_Payment_Init();			// ���Z�f�[�^�����쐬
	IFM_Accumulate_Error_Init();			// �G���[�f�[�^�����쐬
	_flt_EnableTaskChg();					// �^�X�N�؂�ւ� ����

	NTNET_Init((uchar)RAMCLR);										// NT-NET�ر�Ƽ��
	ntautoInit((uchar)RAMCLR);
	ope_imf_Init();
// MH322914(S) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
	PiP_FurikaeInit();
// MH322914(E) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
	if (prm_get(COM_PRM, S_SYS, 80, 1, 6) != 0) {
		OPECTL.Seisan_Chk_mod = ON;									// ���Z����Ӱ��ON
	}
	else{
		OPECTL.Seisan_Chk_mod = OFF;								// ���Z����Ӱ��OFF
	}

	if (NTNetID100ResetFlag == 1) {
		NTNET_Snd_Data101_2();
		wopelg(OPLOG_SET_CTRL_RESET, 0, 0);							// ���엚��o�^
	}
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
	if(RAMCLR != 0){// �������C�j�V�������s
		LongTermParkingCheck_r10_defset();// �������Ԍ��o���Ԃ̐ݒ���L��
	}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
	if (OPECTL.Seisan_Chk_mod == ON) {			// ���Z����Ӱ��ON
		// SRAM�������i�������ر�j
		log_init();
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
// note : GT-4100�̓Z���^�[�ʐM���K�v�Ȃ̂ŁA�������N���A�͍s��Ȃ�
//		memclr(1);
//
//		// �Ԏ����Ұ��l�ߒ���
//		memset(LockInfo, 0, sizeof(LockInfo));
//
//		for (i = 0; i < INT_CAR_LOCK_MAX; i++) {
//			LockInfo[lockinfo_recAP1[i].adr] = lockinfo_recAP1[i].dat;
//			LockInfo[lockinfo_recAP2[i].adr] = lockinfo_recAP2[i].dat;
//		}
//		
//		// �ʐM�ݒ薳����
//		CPrmSS[S_TYP][41]  = 0;		// ۯ����u�pI/F�Րe�@�ڑ��䐔
//		CPrmSS[S_TYP][101] = 0;		// �ׯ�ߗpI/F������ِ�
//		CPrmSS[S_TYP][41]  = 0;			// ۯ����u�pI/F�Րe�@�ڑ��䐔�i0=���ڐ���j
//		CPrmSS[S_TYP][42]  = 200000;	// ۯ����u�pI/F�Րe�@�ڑ��䐔�i20��j
//		CPrmSS[S_TYP][100]  = 10;		// 
//		CPrmSS[S_TYP][47]  = 1001;		// 
//		CPrmSS[S_TYP][118]  = 3;		// 
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z
		CPrmSS[S_SYS][80]  = 100000;		// 
		// �e����p�f�[�^������
		memset(&AutoPayment, 0, sizeof(t_AutoPayment));
	}
#endif

	if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){			// FLASH��RAM�ւ̃p�����[�^�f�[�^�]��
		grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[61] );	// "      �޳�۰�����Ұ�����      "�\��
		SetSetDiff(SETDIFFLOG_SYU_DEFLOAD);							// �f�t�H���g�Z�b�g���O�o�^����
	}
	dsp_background_color(COLOR_WHITE);
	SetDiffLogReset(2);												// ���ʃp�����[�^�~���[�����O

	if(	LOCKCLR == 1 ){												// ۯ�������Ұ���̫��
		lockmaker_clr(1);											// ۯ�������Ұ���̫��(�������Ұ��m���s��)
	}
	NTNET_ClrSetup();

	remote_timer_set = remotedl_restore();

	/*** �╜�d۸ޓo�^���� ***/
	Log_regist( LOG_POWERON );										// �╜�d۸ޓo�^
	if(f_log_init == 1){											// ���O�C�j�V�������{
		wopelg( OPLOG_DATA_INITIAL, 0, 0 );							// ���엚��o�^ �f�[�^�C�j�V�����N��
	}else if(f_log_init == 2){										// �X�[�p�[�C�j�V�������{
		wopelg( OPLOG_ALL_INITIAL, 0, 0 );							// ���엚��o�^ �S�C�j�V�����C�Y�N��
	}

// MH810100(S) K.Onodera  2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
	PktResetReqFlgSet( SystemResetFlg );	// LCD���Z�b�g�ʒm�v���v�ۃZ�b�g
	SystemResetFlg = 0;						// �V�X�e�����Z�b�g���s�t���OOFF
// MH810100(E) K.Onodera  2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
	FLAGPT.nmi_flg = 0;												// NMI�׸޸ر
	
	if( FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC ){		// �N�����̐ݒ�X�V�����͎��v�m���ɓo�^����
		SetSetDiff(SETDIFFLOG_SYU_INVALID);					// �ݒ�X�V�������O�o�^	
	}
	
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_INSTRUCTION ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_INSTRUCTION, 2, 0, 0 );	// ����`���ߗ�O���荞�ݔ���
	}
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_SUPERVISOR_INST ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_SUPERVISOR_INST, 2, 0, 0 );	// �������ߗ�O����
	}
	if( FLAGPT.event_CtrlBitData.BIT.ILLEGAL_FLOATINGPOINT ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_ILLEGAL_FLOATINGPOINT, 2, 0, 0 );	// ���������_��O����
	}
	if( FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_UNKOWNVECT, 2, 0, 0 );			// ���o�^�޸���������
	}
	
	FLAGPT.event_CtrlBitData.BYTE = 0;								// �׸ނ̸ر
	inc_dct( MOV_PDWN_CNT, 1 );										// ���d����񐔶���
	/*** ���d���� ***/

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
//	// ���̏����͏d������,����ĂԕK�v�������̂œd��ON���Ɛݒ�ύX���̂�Call���鎖�Ƃ���B
//	mc10();															// �����v�Z�p�ݒ辯�
	cal_cloud_init();												// �N���E�h�����v�Z ����������
	if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
		// ����ĂԕK�v�̖����d�������Ȃ̂œd��ON���Ɛݒ�ύX���̂�Call����
		mc10();
	}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	if( RAMCLR != 0 ){
		Prn_LogoHead_Init();
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		LogDataClr(LOG_EDYSYUUKEI);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		LogDataClr(LOG_SCASYUUKEI);
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		LogDataClr(LOG_ECSYUUKEI);
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		memset( &Edy_Recv_Buf, 0, sizeof( Edy_Recv_Buf ));
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	}
	else{
		ope_Furikae_fukuden();
		turikan_fuk();												// �ޑK�Ǘ����d����
		fukuden();													// �W�v���A���d����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		Edy_Fukuden();
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		Mifare_WrtNgDataUpdate_Exec();								// Mifare�����ݎ��s���ޏ�񕜓d����
		LockInfoDataCheck();
	}

	Ope_InSigSts_Initial();											// ���͐M�����(OPE�p)������
	
	TimeAdjustInit();												// ���������␳��������

	/*** ��ϰ���� ***/
	Lag10msInit();
	Lag20msInit();
	Lag500msInit();
	Btcom_Init();
	TPU1_init();													// TPU1 Initial
	TPU2_init();													// TPU2 Initial
	TPU3_init();													// TPU3 Initial
	CMTU0_CMT0_init();												// JVMA SYN�M���p1shot�^�C�}������
	Sci2Init();														// JVMA�V���A���|�[�g������
	Sci3Init();														// ���C���[�_�[�ʐM�|�[�g������
	Sci4Init();														// Flap����ʐM�|�[�g������
	Sci7Init();														// serverFOMA_Rism�ʐM�|�[�g������
	Sci9Init();														// NT-NET�ʐM�|�[�g������
	Sci10Init();													// ��ڐGIC�ʐM�|�[�g������
	Sci11Init();													// �N���W�b�gFOMA_Cappi�ʐM�|�[�g������
// MH810103 GG119202(S) DT10��SPI�h���C�o�Ή�
#ifdef	DT10_SPI
	Rspi_DT10_Init();
#endif	// DT10_SPI
// MH810103 GG119202(E) DT10��SPI�h���C�o�Ή�
	WACDOG;															// Watch dog timer reset

	if( OPECTL.Mnt_mod != 5 ){
	RAU_Credit_Enabale = (uchar)prm_get( COM_PRM,S_PAY,24,1,2 );
	if(RAU_Credit_Enabale != 2 && RAU_Credit_Enabale != 3){
		RAU_Credit_Enabale = 0;
	}
	/*** Ӽޭ�ً@��ڑ��҂� ***/
	door_int_read();												// �h�A��Ԃ����[�h����
	if( OPE_SIG_DOOR_Is_OPEN ){
		OPECTL.f_DoorSts = 1;										// �ޱ�J��ԂƂ���
	}
	mojule_wait();													// �ڑ��@��N���҂�( ARCNET�@��܂� )	
	key_read();														// key scan data read
	prn_int();														// �����������
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	Prn_GetModel_R();												// ���V�[�g�v�����^���f���擾
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

	CountFree(ALL_COUNT);		// ���d�����ΏۊO�i�󎚓r���j�̒ʂ��ǂ��Ԃ�ԋp


	/*** ���װ�o�^�͂���ȍ~�ōs���i mojule_wait()���őS�ر�o�^���Ă��邽�� �j***/

	PortAlm_Init();													// �x����Reset�i�����W�J�n�j

	/*** �������Ұ� �ѴװLOG�o�^ ***/								// mojule_wait() �őS�װ�ر�o�^����̂� ���̌�ɓo�^����
	if( 0 != (PowonSts_Param & 0x0002) ){							// SRAM�㋤�����Ұ� �Ѵװ
		err_chk( ERRMDL_MAIN, ERR_MAIN_SETTEIRAM, 2, 0, 0 );		// RAM�� �ݒ�ر �шُ� �װ�o�^
	}
	if( 0 != (PowonSts_Param & 0x0020) ){							// FlashROM�㋤�����Ұ� �Ѵװ
		err_chk( ERRMDL_MAIN, ERR_MAIN_SETTEIFLASH, 2, 0, 0 );		// FlashROM�� �ݒ�ر �шُ� �װ�o�^
	}
	if( 0 != (f_ErrAlm & 0x01) ){									// b0:1=E0057(���v�ǂݍ��ݎ��s)����
		err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKREADFAIL, 1, 0, 0 );	// ���v�ǂݍ��ݎ��s
	}
	if( 0 != (f_ErrAlm & 0x02) ){									// b1:1=E0058(�@����ǂݍ��ݎ��s)����
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_MACHINEREADFAIL, 2, 0 );	// �װ۸ޓo�^(�@����ǂݍ��ݎ��s)
	}

	if( RTC_Err.BIT.RTC_VLF ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);				/* VLF(���U��H�d���ቺ	*/
	}
	if( RTC_Err.BIT.RTC_VDET ){
		err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);			/* VDET(���x�⏞��H�d���ቺ) */
	}
	RTC_Err.BYTE = 0;
	if(f_parm_datchk == PARM_AREA_ERR) {
		err_chk( ERRMDL_MAIN, ERR_MAIN_BOOTINFO_PARM, 1, 0, 0 );	// ���ʃp�����[�^�^�p�ʈُ�(�����̂݁A�`�F�b�N�̎��_�ŏC���͂��Ă���)
	}
	if((f_wave_datchk == WAVE_AREA_ERR_SETAREA1) || (f_wave_datchk == WAVE_AREA_ERR_SETAREA2)) {
		err_chk( ERRMDL_MAIN, ERR_MAIN_BOOTINFO_WAVE, 1, 0, 0 );	// �����f�[�^�^�p�ʈُ�(�����̂݁A�`�F�b�N�̎��_�ŏC���͂��Ă���)
	}

// MH810100(S) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	AVM_Sodiac_Init();
//
//	red_int();														// ذ�ް������
// MH810100(E) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	}

// MH810103 GG119202(S) �N���V�[�P���X�s��C��
	ModuleStartFlag = 0;
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// GM760201(S) �ݒ�A�b�v���[�h���̓d��OFF/ON��AMANO��ʂ���i�܂Ȃ��^�C�~���O�����邽�߁A�C��
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
// 	remotedl_info_clr();
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
// GM760201(E) �ݒ�A�b�v���[�h���̓d��OFF/ON��AMANO��ʂ���i�܂Ȃ��^�C�~���O�����邽�߁A�C��
	/*** ���ڰ���Ӱ�ފm�� ***/
	if(OPECTL.Mnt_mod != 5){
		if(stricmp((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2) != 0){
			memset(&Chk_info.Chk_str1, 0, 30);		// �Œ蕶����1�E2���N���A����
		}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
// PowerOn���ɕK��op_init00()���N�����邽�߂̏����ɕύX
//		if( CP_MODECHG == 0 ){											// �ݒ�Ӱ�޽���(�J������)ON?
//			OPECTL.Mnt_mod = 1;											// ����ݽӰ�ޑI��������
//			OPECTL.Mnt_lev = (char)-1;									// ����ݽ�����߽ܰ�ޖ���
//			OPECTL.Ope_mod = 0;											// ���ڰ���Ӱ�޸ر
//			OPECTL.Pay_mod = 0;											// ���Z�̎��(�ʏ퐸�Z)
//			OPECTL.PasswordLevel = (char)-1;
//		}else{
//			OPECTL.Mnt_mod = 0;											// ���ڰ���Ӱ�ޏ�����
//			OPECTL.Mnt_lev = (char)-1;									// ����ݽ�����߽ܰ�ޖ���
//			OPECTL.Ope_mod = 0;											// ���ڰ���Ӱ�޸ر
//			OPECTL.Pay_mod = 0;											// ���Z�̎��(�ʏ퐸�Z)
//			OPECTL.PasswordLevel = (char)-1;
//		}
//		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//		OPECTL.Kakari_Num = 0;											// ����ݽ�W���ԍ�������

		// PowerOn���ɕK��op_init00()���N�����邽�߂̏���
		OPECTL.init_sts = 0;										// ��������
		OPECTL.Mnt_mod = 0;											// ���ڰ���Ӱ�ޏ�����
		OPECTL.Mnt_lev = (char)-1;									// ����ݽ�����߽ܰ�ޖ���
		OPECTL.Ope_mod = 255;										// ���ڰ���Ӱ�ޕύX
		OPECTL.Pay_mod = 0;											// ���Z�̎��(�ʏ퐸�Z)
		OPECTL.PasswordLevel = (char)-1;
		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
		OPECTL.Kakari_Num = 0;											// ����ݽ�W���ԍ�������
		OPECTL.sended_opcls = (char)-1;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	}
	// �C�j�V������̓d�������グ���ɏ����ݒ���s�킹�邽�߁B
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	ck_syuukei_ptr_zai(LOG_SCASYUUKEI);
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	ck_syuukei_ptr_zai(LOG_ECSYUUKEI);
// MH321800(E) hosoda IC�N���W�b�g�Ή�
	if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// �N�������ޱ�̊J��Ԃ�����
		mask_VIB_SNS.BIT.START_STATUS = 1;							// �N�������ޱ�̏ꍇ���׸�ON
	}

	if(OPE_SIG_DOOR_and_NOBU_are_CLOSE && OPE_IS_EBABLE_MAG_LOCK){	// �O�ʔ��������ޱ��ނ���Ԃ��d�����b�N���s���ݒ�
		OPE_SIG_OUT_DOORNOBU_LOCK(1);								// �ޱ��ނ�ۯ����s��
		wmonlg( OPMON_LOCK_CLOSE, 0, 0 );							// ����o�^
		wopelg2( OPLOG_DOORNOBU_LOCK, 0L, 0L );						// ���엚��o�^(�ޱ���ۯ�)
		attend_no = 0;					// �h�A���b�N�W�������N���A
	}

	if( GT_Settei_flg == 1 && PPrmSS[S_P01][1] != 0 ){				// ���Cذ�ް���߈ُ�̏ꍇ�ż�����J���ݒ莞
		read_sht_cls();												// ������۰��
	}

	if(SFV_SNDSTS.BIT.note != 0){
		err_chk( ERRMDL_NOTE, ERR_NOTE_SAFE, 2, 0, 0 );
		IoLog_write(IOLOG_EVNT_ERR_NOTESAFE, (ushort)(( LockInfo[ryo_buf.pkiti - 1].area * 10000L ) + LockInfo[ryo_buf.pkiti - 1].posi ), 0, 0);
		SFV_SNDSTS.BIT.note = 0;
	}
	if(SFV_SNDSTS.BIT.coin != 0){
		err_chk( ERRMDL_COIM, ERR_COIN_SAFE, 2, 0, 0 );
		IoLog_write(IOLOG_EVNT_ERR_COINSAFE, (ushort)(( LockInfo[ryo_buf.pkiti - 1].area * 10000L ) + LockInfo[ryo_buf.pkiti - 1].posi ), 0, 0);
		SFV_SNDSTS.BIT.coin = 0;
	}
	if( remote_timer_set == 1 ){									// ���ʏ��̃A�b�v���[�h���K�v�ȏꍇ
		// �ڑ����g���C�Ԋu(�b)
		interval = prm_get(COM_PRM, S_RDL, 5, 3, 3);
		if (!interval) {
			interval = 1;
		}
		LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
	}
	else {
		remotedl_pow_flg_set(FALSE);
	}

	creCheckRejectSaleData();										// ���ۃf�[�^������΃G���[�o��

	LcdBackLightCtrl( ON );											// back light ON ��start_dsp()�C���ɂ��ǉ�
	// �N�����ɃZ���^�[�p�[�����f�[�^�𑗐M����
	NTNET_Snd_Data65(REMOTE_PC_TERMINAL_NO);
// MH810100(S) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�N���ʒm��M��OPE�^�X�N�����������܂ő҂�)
	PktStartUpStsChg( 1 );
// MH810100(E) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�N���ʒm��M��OPE�^�X�N�����������܂ő҂�)
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// �A�C�h���T�u�^�X�N�Ɉړ�
//// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//	LongTermParkingCheck();// �������ԃ`�F�b�N
//// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

	/*** ���ڰ������ ***/
	for( ; ; ) {
		switch( OPECTL.Mnt_mod ){									// ���ڰ���Ӱ��
			case 0:													// �ʏ�Ӱ��
				OpeMain();
				NTNET_ClrSetup();
				break;
			case 1:													// ����ݽӰ�ޑI��
				MntMain();
				break;
			case 2:													// �W��,�Ǘ�������ݽӰ��
				UserMntMain();
				break;
			case 3: 												// �Z�p������ݽӰ��
				SysMntMain();
				break;
			case 4: 												// ��[��������ݽӰ��
				dsp_background_color(COLOR_WHITE);
				if( CPrmSS[S_KAN][1] ){
					UsMnt_mnyctl();									// �ޑK��[�i���K�Ǘ�����̏ꍇ�j
				}else{
					UsMnt_mnyctl2();								// �ޑK��[�i���K�Ǘ��Ȃ��̏ꍇ�j
				}
				OPECTL.Mnt_mod = 0;									// ���ڰ���Ӱ�� �� �ʏ�
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				break;
			case 5:													// �H�ꌟ�����[�h
				ChekModeMain();
				break;
			default:
				OPECTL.Mnt_mod = 0;									// ���ڰ���Ӱ�ޏ�����
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;							// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				OPECTL.Ope_mod = 0;									// ���ڰ���Ӱ�޸ر
				OPECTL.Pay_mod = 0;									// ���Z�̎��(�ʏ퐸�Z)
				break;
		}
		rd_shutter();												// ���Cذ�ް���������
		LedReq( CN_TRAYLED, LED_OFF );								// ��ݎ�o�����޲��LED OFF
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �U���Z���T�[�^�C���A�E�g�����҂����� �R�[���o�b�N�֐�                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : vib_sns_timeout( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void			                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : y.katoh                                                 |*/
/*| Date         : 2007-09-25                                              |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void vib_sns_timeout( void )
{
	if( OPE_SIG_DOOR_Is_OPEN == 0 && FNT_VIB_SNS != 0 ){	// �h�A���U���Z���T�[ON�̏ꍇ
		alm_chk( ALMMDL_MAIN, ALARM_VIBSNS, 1 );	// �A���[���o�^
	}												// ����ȊO�̏ꍇ�̓A���[���o�^���Ȃ�
	
	mask_VIB_SNS.BIT.MASK = 0;						// �U���Z���T�[�}�X�N�t���OOFF
	mask_VIB_SNS.BIT.TIMER_END = 1;					// �U���Z���T�[�}�X�N�^�C�}�[�^�C���A�E�g
}

/*[]----------------------------------------------------------------------[]*/
/*| ү���ޑ҂�����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMessage( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ү���ނ̺���ޕ�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

unsigned short GetMessage( void )
{
	MsgBuf		*msb;
	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;
	ushort PrintKind;
	T_FrmIrekaeTest FrmIrekaeTest;
	T_FrmEnd *FrmEnd;
	short	VIB_SNS_timer;
	static char	on_off;

	for( ; ; ) {
		taskchg( IDLETSKNO );										// Change task to idletask
		if( (msb = GetMsg( OPETCBNO )) == NULL ){
			continue;
		}
		kinko_syuukei();

		sc.comd = msb->msg.command;
		OPECTL.Comd_knd = sc.cc[0];									// Command kind
		OPECTL.Comd_cod = sc.cc[1];									// Command code

		switch( OPECTL.Comd_knd ){

			case MSGGETHIGH(KEY_TEN):								// KEY event

				OPECTL.on_off = msb->msg.data[0];
				switch( sc.comd ){

					case KEY_MODECHG:								// Mode change key
						BUZPI();
						break;

					case KEY_DOOR1:									// Door1 key event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							if( OPECTL.on_off == 1 ){				// OFF(Door Open)
								wopelg( OPLOG_DOOROPEN, 0, 0 );		// ���엚��o�^
								wmonlg( OPMON_DOOROPEN, 0, 0 );		// ����o�^
								mask_VIB_SNS.BIT.MASK = 0;			// �h�A�J�Ń}�X�N�t���O������
								mask_VIB_SNS.BIT.TIMER_END = 0;		// �h�A�J�Ń}�X�N�^�C�}�[�^�C���A�E�g���m������
								LagCan500ms(LAG500_VIB_SNS_TIMER);	//  �^�C�}�[�L�����Z��
								if(OPE_SIG_DOORNOBU_Is_OPEN){				// �ޱ��ނ��J(��)���
									StartupDoorArmDisable = 1;
								}
								else {
									if( StartupDoorArmDisable == 1 ){				// �N�����̃h�A�x���}�~
										on_off = !(OPECTL.on_off & 1);
										queset(OPETCBNO, KEY_DOORARM, 1, &on_off);	// �h�A�x��
									}
								}
								if(DoorLockTimer){								// �ޱ�J��� ��ۯ�������ϰ��čς�
									DoorLockTimer = 0;							// �ޱ�J��� ��ۯ�������ϰ�׸޸ر
									Lagcan( OPETCBNO, 16 );					// �ޱ�J��� ��ۯ�������ϰؾ��
								}
								Lagcan(OPETCBNO, 5);				// �h�A�m�u�߂��Y��h�~�^�C�}�[��~
								opa_chime(OFF);						// �`���C����~
								if(OPE_SIG_DOORNOBU_Is_OPEN){				// �ޱ��ނ��J��Ԃɂ����ꍇ
									OPECTL.f_DoorSts = 1;
								}
							}else{
								StartupDoorArmDisable = 1;
								if ( mask_VIB_SNS.BIT.MASK == 0 )			// �h�A�J�ɂ��^�C�}�[�ċN���ی�
								{
									VIB_SNS_timer = (short)prm_get(COM_PRM, S_PAY, 32, 2, 1);		// �U���Z���T�[�}�X�N���Ԏ擾
									
									// �^�C���A�E�g�l���O�Ŗ����ꍇ�́A�}�X�N�^�C�}�[�X�^�[�g
									if ( VIB_SNS_timer != 0 )
									{
										mask_VIB_SNS.BIT.TIMER_END = 0;		// �}�X�N�^�C�}�[�^�C���A�E�g���m������
										mask_VIB_SNS.BIT.MASK = 1;			// �U���Z���T�[�}�X�N��
										LagTim500ms( LAG500_VIB_SNS_TIMER, (short)((VIB_SNS_timer * 2) + 1), vib_sns_timeout );
									}
								}								
								wopelg( OPLOG_DOORCLOSE, 0, 0 );	// ���엚��o�^
								wmonlg( OPMON_DOORCLOSE, 0, 0 );	// ����o�^
								if( !OPE_SIG_DOORNOBU_Is_OPEN ){			// �ޱ��ނ��c(��)�̏�Ԃ̏ꍇ
									if(prm_get(COM_PRM, S_PAY, 17, 3, 4)){	// �ޱ����ޕ� ���ۯ��҂����Ԑݒ肠��
										Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// �ޱ����ޕ� ���ۯ��҂�����ؾ��
									}
								}
								else{
									Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// �ޱ����ޕ� ���ۯ��҂�����ؾ��
								}
								Ope_StartDoorKnobTimer();
								Lagcan( OPETCBNO, 33 );
							}
						}
						OPECTL.coin_syukei = 0;						// ��݋��ɏW�v���o�͂Ƃ���
						OPECTL.note_syukei = 0;						// �������ɏW�v���o�͂Ƃ���
						mask_VIB_SNS.BIT.START_STATUS = 0;				// �ޱ�J�ŋN������Ԃ��׸�ؾ��
						break;

					case KEY_COINSF:								// Coin safe event
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// �`���C����~
						}
						if( OPECTL.on_off == 1 ){					// ON
							if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
								if( OPECTL.coin_syukei == 0 ){		// ��݋��ɏW�v���o��?
									coin_kinko_evt = 1;				// �R�C�����Ɉ�������
								}
								wopelg( OPLOG_COINKINKO, 0, 0 );	// ���엚��۸ޓo�^
								wmonlg( OPMON_COINKINKO_UNSET, 0, 0 );	// ����o�^
							}
						}
						else{										// OFF (��݋��ɒ�)
							if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
								wmonlg( OPMON_COINKINKO_SET, 0, 0 );	// ����o�^
							}
						}
						sc.comd = 0;
						break;

					case KEY_DOORARM:								// Door alarm event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							#if (DEBUG_MODE_01 == DEBUG_MODE)		// Ͻ��ݒ�i�]���p�j
								OPECTL.on_off = 1;					// ���OFF�i�x�񖳂��j
							#endif
							if( OPECTL.on_off == 0 ){				// High
								Lagtim( OPETCBNO, 33, 1*50 );		// 1s�^�C�}�Z�b�g(�x��u�U�[����f�B���C�^�C�}(�h�A�x��))
							}else{									// Low
								alm_chk( ALMMDL_MAIN, ALARM_DOORALARM, 0 );
								Lagcan( OPETCBNO, 33);
							}
						}
						sc.comd = 0;
						break;

					case KEY_VIBSNS:								// �U���ݻ event
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							VIB_SNS_timer = (short)prm_get(COM_PRM, S_PAY, 32, 2, 1);		// �U���Z���T�[�}�X�N���Ԏ擾
							if( OPECTL.on_off == 1 && 
								(( mask_VIB_SNS.BIT.MASK == 0 && !VIB_SNS_timer ) || 				// �U���Z���T�[ON���}�X�N���Ԃ��O�̏ꍇ��
								   mask_VIB_SNS.BIT.TIMER_END == 1 || mask_VIB_SNS.BIT.START_STATUS == 1 )){	// �}�X�N�^�C�}�[���^�C���A�E�g�ς݂��N�������ޱ��Ԃł̐U�����m�͑����A���[���o�^
								Lagtim( OPETCBNO, 34, 1*50 );		// 1s�^�C�}�Z�b�g(�x��u�U�[����f�B���C�^�C�}(�U���Z���T�[))
							}
							else if( OPECTL.on_off == 0 ){ 
								Lagcan( OPETCBNO, 34 );
							}
						}
						sc.comd = 0;
						break;

					case KEY_NTFALL:								// ����ذ�ް�E�����m����
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							#if (DEBUG_MODE_01 == DEBUG_MODE)		// Ͻ��ݒ�i�]���p�j
								OPECTL.on_off = 1;					// ���OFF�i�x�񖳂��j
							#endif
							if( OPECTL.on_off == 0 ){				// High
								Lagtim( OPETCBNO, 35, 1*50 );		// 1s�^�C�}�Z�b�g(�x��u�U�[����f�B���C�^�C�}(�����E��))
							}else{									// Low
								Lagcan( OPETCBNO, 35 );
							}
						}
						sc.comd = 0;
						break;

					case KEY_CNDRSW:								// ���ү�������������
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// �`���C����~
						}
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							if( OPECTL.on_off == 0 ){				// High
								wopelg( OPLOG_COINMECHSWCLOSE, 0, 0 );	// ���엚��o�^
								wmonlg( OPMON_COINMECHSWCLOSE, 0, 0 );	// ����o�^
							}else{
								wopelg( OPLOG_COINMECHSWOPEN, 0, 0 );	// ���엚��o�^
								wmonlg( OPMON_COINMECHSWOPEN, 0, 0 );	// ����o�^
							}
						}
						break;

					case KEY_NOTESF:								// �������ɶ�ް����
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							Ope_StartDoorKnobTimer();
							opa_chime(OFF);							// �`���C����~
						}
						if( OPECTL.Ope_Mnt_flg != 6 && OPECTL.Mnt_mod != 5){  		// SW����������?
							if( OPECTL.on_off == 0 ){				// High
								wopelg( OPLOG_NOTERDSWCLOSE, 0, 0 );	// ���엚��o�^
								wmonlg( OPMON_NOTERDSWCLOSE, 0, 0 );	// ����o�^
							}else{
								wopelg( OPLOG_NOTERDSWOPEN, 0, 0 );		// ���엚��o�^
								wmonlg( OPMON_NOTERDSWOPEN, 0, 0 );		// ����o�^
							}
						}
						break;
					case KEY_DOORNOBU:
						if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// SW����������?
							if(OPE_SIG_DOORNOBU_Is_OPEN){				// �ޱ��ނ��J��Ԃɂ����ꍇ
								Ope_StartDoorKnobTimer();
								on_off = !(OPECTL.on_off & 1);
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//// MH810100(S) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
//								// �󎚏������̓h�A�m�u�J�̃C�x���g���ʒm���Ȃ�
//								if (OPECTL.Mnt_mod == 0 ||
//										(rct_proc_data.Printing == 0 && jnl_proc_data.Printing == 0)) {
//// MH810100(E) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
								// �󎚏�����/�p�����[�^�A�b�v���[�h���̓h�A�m�u�J�̃C�x���g���ʒm���Ȃ�
								if (OPECTL.Mnt_mod == 0 ||
										(rct_proc_data.Printing == 0 && jnl_proc_data.Printing == 0 &&
										OPECTL.lcd_prm_update == 0)) {
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
// MH810100(S) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
								}
// MH810100(E) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
								queset(OPETCBNO, KEY_DOORARM, 1, &on_off);	// �h�A�x�����
								Ope_CenterDoorTimer( 0 );				// �^�C�}�[��~
							}else{
								Lagcan( OPETCBNO, 16 );					// �ޱ�J��� ��ۯ���ϰؾ��
								Lagcan( OPETCBNO, 17 );					// �ޱ����ޕ� ���ۯ��҂���ϰؾ��
								if( !OPE_SIG_DOOR_Is_OPEN ){					// �ޱ��
									if( prm_get(COM_PRM, S_PAY, 17, 3, 4) ){	// �ޱ����ޕ� ���ۯ��҂����Ԑݒ肠��
										Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50) );	// �ޱ����ޕ� ���ۯ�������ϰ����
									}
								}
								Lagcan(OPETCBNO, 5);					// �h�A�m�u�߂��Y��h�~�^�C�}�[��~
								opa_chime(OFF);							// �`���C����~
								on_off = !(OPECTL.on_off & 1);
								if( OPECTL.Mnt_mod != 0 ){				// �����e�i���X��
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//									if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){	// �󎚏������ł͂Ȃ��ꍇ�̂�
									if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) && 	// �󎚏������ł͂Ȃ��ꍇ��
											 OPECTL.lcd_prm_update == 0){									// �p�E�����[�^�A�b�v���[�h���łȂ��ꍇ
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
										queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
									}
								}
								else{
									queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
								}
								if( OPECTL.f_DoorSts && (!OPE_SIG_DOOR_Is_OPEN) ){
									// �h�A�m�u�ʏ�A�h�A��ԂŃ^�C�}�[�X�^�[�g������B�h�A�J�ł̓X�^�[�g�����Ȃ��B
									Ope_CenterDoorTimer( 1 );			// �^�C�}�[�N��
								}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
								// �h�A����RSW��0�ȊO�H
								if( read_rotsw() != 0 ){
									lcdbm_notice_alm( lcdbm_alarm_check() );	// �x���ʒm
								}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
							}
						}
// MH810100(S) Yamauchi 2020/02/18 �Ԕԃ`�P�b�g���X_�h�A��Ԃ̒ʒm�͖������ōs��
						// �h�A���
						lcdbm_notice_ope( LCDBM_OPCD_DOOR_STATUS, (ushort)on_off );
// MH810100(E) Yamauchi 2020/02/18 �Ԕԃ`�P�b�g���X
						break;						
					case KEY_FANST:									// FAN�쓮�J�n���x���o
						if( 1 == chk_fan_timectrl() ){				// FAN������~����
							if(OPECTL.on_off == 1) {				// FAN�쓮�J�n���x���B
								fan_exe_state = 1;					// �ʏ퐧�䎞�̏�Ԃ��o�b�N�A�b�v
							}else{
								fan_exe_state = 0;					// �ʏ퐧�䎞�̏�Ԃ��o�b�N�A�b�v
							}
						}else{
							if(OPECTL.on_off == 1) {				// FAN�쓮�J�n���x���B
								CP_FAN_CTRL_SW = 1;					// FAN�쓮
								fan_exe_state = 1;					// �ʏ퐧�䎞�̏�Ԃ��o�b�N�A�b�v
								// FAN���쓮�������Ԃ��L�^
								fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
								fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
								fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
								fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
								fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
								fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 1;//ON
								fan_exe_time.index++;
								if(fan_exe_time.index >= FAN_EXE_BUF) {
									fan_exe_time.index = 0;
								}
							} else {// OPECTL.on_off == 0
								CP_FAN_CTRL_SW = 0;					// FAN��~
								fan_exe_state = 0;					// �ʏ퐧�䎞�̏�Ԃ��o�b�N�A�b�v
								// FAN���~�������Ԃ��L�^
								fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
								fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
								fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
								fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
								fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
								fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 2;//OFF
								fan_exe_time.index++;
								if(fan_exe_time.index >= FAN_EXE_BUF) {
									fan_exe_time.index = 0;
								}
							}
						}
						break;
					case KEY_TEN_F1:
					case KEY_TEN_F2:
						if( OPECTL.on_off == 1) {					// ON
							if(OpeKeyRepeat.enable == 1) {			// �L�[���s�[�g�L��
								if(OpeKeyRepeat.key == sc.comd) {	// �L�[ON2��ڈȍ~(���s�[�g)
									LagTim20ms(LAG20_KEY_REPEAT, 11, Ope_KeyRepeat);	// 200ms
								}
								else {								// �L�[ON����
									OpeKeyRepeat.key = sc.comd;		// ���s�[�g�ΏۃL�[�R�}���h�ޔ�
									LagTim20ms(LAG20_KEY_REPEAT, 26, Ope_KeyRepeat);	// 500ms
								}
							}
						}
						else {										// OFF
							OpeKeyRepeat.key = 0;
							LagCan20ms(LAG20_KEY_REPEAT);			// ���s�[�g�^�C�}�폜
							sc.comd = 0;							// OFF����ү���ޖ����Ƃ���
						}
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							if( Ope_IsEnableDoorKnobChime() ){
								// �h�A�m�u�߂��Y��`���C���L���̏ꍇ�̂݃^�C�}���Đݒ肷��
								Ope_StartDoorKnobTimer();
							}
							opa_chime(OFF);								// �`���C����~
						}
						break;
					case KEY_TEN_F5:
						if( OPECTL.Mnt_mod != 0 && OPECTL.Mnt_mod != 5 ){					// �����e�i���X��
							if( !OPE_SIG_DOORNOBU_Is_OPEN ){		// �ޱ��ޕ��
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
							}
						}
					default:
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							if( Ope_IsEnableDoorKnobChime() && sc.comd != KEY_MANDET ){	// �h�A�m�u�߂��Y��`���C���L��
								Ope_StartDoorKnobTimer();
							}
							if( sc.comd != KEY_MANDET ) {			// �l�̌��m�ȊO�̓A���[����~����
								opa_chime(OFF);						// �`���C����~
							}
						}
						if( sc.comd == KEY_RXIIN )
						{	// RYB���͐M��
							if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// ���o�͐M���������łȂ�
								InSignalCtrl(INSIG_OPOPEN);			//�����c�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
								InSignalCtrl(INSIG_OPCLOSE);		//�����x�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
								if( OPECTL.on_off == 0 ){			// OFF?(Y)
									sc.comd = 0;					// OFF����ү���ޖ����Ƃ���
								}
							}
						}else{
							if( OPECTL.on_off == 0 ){				// OFF?(Y)
								sc.comd = 0;						// OFF����ү���ޖ����Ƃ���
							}
						}
						break;
				}
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
			case MSGGETHIGH(LCD_BASE):				// lcdbm_rsp
				switch( sc.comd ) {
					// �G���[�ʒm
					case LCD_ERROR_NOTICE:
						memcpy( &LcdRecv.lcdbm_rsp_error, &msb->msg.data[0], sizeof( lcdbm_rsp_error_t ));
						// ��M�G���[�̓o�^����
						lcdbm_ErrorReceiveProc();
						break;

					// ���Ϗ���ް�
					case LCD_ICC_SETTLEMENT_STS:
						memcpy( &lcdbm_ICC_Settlement_Status, &msb->msg.data[0], sizeof( unsigned char ));
						break;

					// ���ό��ʏ��
					case LCD_ICC_SETTLEMENT_RSLT:
						memcpy( &lcdbm_ICC_Settlement_Result, &msb->msg.data[0], sizeof( unsigned char ));
						break;

					// ���A���^�C���ʐM�a�ʌ���(�����e�i���X)
					case LCD_MNT_REALTIME_RESULT:
						memcpy( &LcdRecv.lcdbm_rsp_rt_con_rslt, &msb->msg.data[0], sizeof( lcdbm_rsp_rt_con_rslt_t ));
						break;

					// DC-NET�ʐM�a�ʌ���(�����e�i���X)
					case LCD_MNT_DCNET_RESULT:
						memcpy( &LcdRecv.lcdbm_rsp_dc_con_rslt, &msb->msg.data[0], sizeof( lcdbm_rsp_dc_con_rslt_t ));
						break;

					// ����ʒm
					case LCD_OPERATION_NOTICE:
						memcpy( &LcdRecv.lcdbm_rsp_notice_ope, &msb->msg.data[0], sizeof( lcdbm_rsp_notice_ope_t ));
						if (LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_RESET_NOT) {
							on_off = 1;				// ؾ�Ēʒm
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү���ޑ��M����
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCDBM_OPCD_RESET_NOT��M����ope�����͖�����
						}
// MH810100(S) 2020/07/29 #4560 ��ʂ��t���[�Y���Ă��܂��ꍇ������
						// �N���ʒm(�t�F�[�Y�����ꂽ�̂ł�������ؒf)
						if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_STA_NOT && LcdRecv.lcdbm_rsp_notice_ope.status != 0){
							on_off = 1;				// ؾ�Ēʒm
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү���ޑ��M����
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCDBM_OPCD_RESET_NOT��M����ope�����͖�����

						}
// MH810100(E) 2020/07/29 #4560 ��ʂ��t���[�Y���Ă��܂��ꍇ������
						if (LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PRM_UPL_REQ) {
 							if(0 == lcdbm_setting_upload_FTP()){						// LCDӼޭ�قɑ΂������Ұ�����۰�ނ��s��
								// ����
								lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, 0 );	// �ݒ�A�b�v���[�h�ʒm���M�i OK(�ċN���v) �j
							}else{
								// ���s
								lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, 1 );	// �ݒ�A�b�v���[�h�ʒm���M�i 1=NG )
							}
						}
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
						if(LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PAY_STP) {
							OPECTL.f_req_paystop = (uchar)LcdRecv.lcdbm_rsp_notice_ope.status;
						}
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
						break;

					// �ؒf���m(PKTtask��LCD�Ƃ̒ʐM���s�ʎ��ɔ��s�����)
					case LCD_COMMUNICATION_CUTTING:
						if( OPECTL.Ope_mod != 255 ){
							on_off = 2;				// �ؒf���m
							// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү���ޑ��M����
							queset(OPETCBNO, LCD_DISCONNECT, 1, &on_off);
							// LCD_COMMUNICATION_CUTTING��M����ope�����͖�����
						}
						break;

					// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү���ޑ��M����
					case LCD_DISCONNECT:
						memcpy( &LcdRecv.lcdbm_lcd_disconnect, &msb->msg.data[0], sizeof( lcdbm_lcd_disconnect_t ));
						break;
				}
				break;		// return == sc.comd
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

			case MSGGETHIGH(CLOCK_CHG):								// Clock change event
				ClkrecUpdate( &msb->msg.data[0] );
				TimeAdjustMain(sc.comd);							// �����␳

				AutoDL_UpdateTimeCheck();

				if( 1 == f_NTNET_RCV_MNTTIMCHG ){					// ����ݽ���ݎ����ύX
					NTNET_Snd_Data229();							// NT-NET���v�ް��쐬
					f_NTNET_RCV_MNTTIMCHG = 0;
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
				if (OPECTL.EJAClkSetReq != 0) {
					MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);			// �����ݒ�
					OPECTL.EJAClkSetReq = 0;
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_TimeSyncSend_Chk();
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				//LCD�ޯ�ײē��춳�čX�V
//				if(SD_LCD && backlight != 0){
//					inc_dct(LCD_LIGTH_CNT,1);
//				}
				// QR���[�_�[���쎞��
				if( QR_READER_USE ){
					inc_dct(QR_READER_CNT,1);
				}
				SUBCPU_MONIT = 0;									// ���CPU�Ď��׸޸ر
				err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 0, 0, 0 ); 		// �װ۸ޓo�^
				if( DateTimeCheck() == 0 ){
					// �����͈�����OK
					err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKFAIL, 0, 0, 0 ); 	// �װ۸ޓo�^
				}else{
					err_chk( ERRMDL_MAIN, ERR_MAIN_CLOCKFAIL, 1, 0, 0 ); 	// �װ۸ޓo�^
				}
				auto_syuukei();										// �����W�v����

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//				if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {		// Edy���p�\��Ԃ��H
//					auto_centercomm( 3 );
//				}
//				if(	Edy_Rec.edy_status.BIT.CENTER_COM_START ){
//					CneterComLimitTime++;
//					Edy_CentComm_Recv( 1 );
//				}
//				Edy_SndData18();									// �Ǻ���ޑ��M
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

				creOneMinutesCheckProc();
				ntautoTimeAdjust();

				// ���ʃp�����[�^�ŗL���ɂȂ��Ă�����̏W�v�����X�V����(02-0024�B�C)
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				if(prm_get(COM_PRM, S_PAY, 24, 1, 3) != 0)				/* �CSuica�L�� */
				if ( isSX10_USE() )
// MH321800(E) hosoda IC�N���W�b�g�Ή�
				{
					ck_syuukei_ptr_zai(LOG_SCASYUUKEI);
				}
// MH321800(S) hosoda IC�N���W�b�g�Ή�
				else if ( isEC_USE() ) {
				// ���σ��[�_
					ck_syuukei_ptr_zai(LOG_ECSYUUKEI);
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				else if(prm_get(COM_PRM, S_PAY, 24, 1, 4) != 0)			/* �BEdy�L�� */
//				{
//					ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);
//				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// �A�C�h���T�u�^�X�N�Ɉړ�
//// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//					LongTermParkingCheck();// �������ԃ`�F�b�N
//// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322917(S) A.Iiizumi 2018/12/05 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				// �^�p���Ƀ��A���^�C�����p�̓��t�ؑ֊�����̐ݒ�ύX���s��ꂽ�Ƃ��̐ݒ�X�V����
				date_uriage_prmcng_judge();
// MH322917(E) A.Iiizumi 2018/12/05 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				if( 1 == chk_fan_timectrl() ){// FAN������~����
					if(f_fan_timectrl == 0){// �O��͋�����~�łȂ�
						// FAN���~�������Ԃ��L�^
						fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
						fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
						fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
						fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
						fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
						fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 2;//OFF
						fan_exe_time.index++;
						if(fan_exe_time.index >= FAN_EXE_BUF) {
							fan_exe_time.index = 0;
						}
					}
					f_fan_timectrl = 1;// FAN������~���t���O�Z�b�g
					CP_FAN_CTRL_SW =0;// FAN�����~
				}else{
					if(f_fan_timectrl != 0){
						f_fan_timectrl = 0;// FAN������~���t���O�N���A
						CP_FAN_CTRL_SW = fan_exe_state;// �ʏ퐧�䎞�̏�Ԃɖ߂�
						if(fan_exe_state == 1){// �ʏ퐧�䎞��ON�ł���
							// FAN���쓮�������Ԃ��L�^
							fan_exe_time.fan_exe[fan_exe_time.index].year = CLK_REC.year;
							fan_exe_time.fan_exe[fan_exe_time.index].mont = CLK_REC.mont;
							fan_exe_time.fan_exe[fan_exe_time.index].date = CLK_REC.date;
							fan_exe_time.fan_exe[fan_exe_time.index].hour = CLK_REC.hour;
							fan_exe_time.fan_exe[fan_exe_time.index].minu = CLK_REC.minu;
							fan_exe_time.fan_exe[fan_exe_time.index].f_exe = 1;//ON
							fan_exe_time.index++;
							if(fan_exe_time.index >= FAN_EXE_BUF) {
								fan_exe_time.index = 0;
							}
						}
					}
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
				EJA_TimeAdjust();
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				break;
			case MSGGETHIGH(TIMEOUT):								// Timme out
				if(OPECTL.Comd_cod == 0x03){
					if(CardStackStatus){
						StackCardEject(4);
					}
				}
				else
				if(OPECTL.Comd_cod == 0x05){	// �h�A�m�u�߂��Y��h�~�^�C�}�[(5sec)�^�C���A�E�g
					if ((OPE_SIG_DOOR_Is_OPEN == 0) && (OPE_SIG_DOORNOBU_Is_OPEN)) {	// �h�A���h�A�m�u�J
						if( Ope_IsEnableDoorKnobChime() ){							// �h�A�m�u�߂��Y��`���C���L��
							opa_chime(ON);
							Ope_StopDoorKnobTimer();
						}
					}
				}
				else if(OPECTL.Comd_cod == 0x06) {			// �����e�i���X�����i�󎚓��j�^�C���A�E�g
					if(!Ope_IsEnableDoorKnobChime()){
						// �h�A�m�u�J�`���C����������Ԃň󎚓����^�C���A�E�g�����ꍇ�͗L���ɂ���
						Ope_EnableDoorKnobChime();			// �h�A�m�u�J�Ď��^�C�}�J�n
					}
				}
				else if(OPECTL.Comd_cod == 29) {			// �h�A�m�u�߂��Y��h�~�A���[���x�����o�͎��ԃ^�C���A�E�g
					opa_chime(OFF);
				}
				else
				if(OPECTL.Comd_cod == 0x10){
					if( DoorLockTimer && OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5 ){					// �ޱ�J��� ��ۯ�������ϰ��ON������`�F�b�N�̓��o�͐M���`�F�b�N���ȊO
						Lagcan( OPETCBNO, 16 );										// �ޱ�J��� ��ۯ�������ϰؾ��
						DoorLockTimer=0;												// �ޱ�J��� ��ۯ�������ϰ�ر
						if(!prm_get(COM_PRM, S_PAY, 17, 3, 4) && !OPE_SIG_DOORNOBU_Is_OPEN){	// �ޱ����ޕ� ���ۯ��҂����Ԃ��O�����ޱ��ނ��c(��)�̏�Ԃ̏ꍇ
						}else{														// �ޱ�J��� ��ۯ�������ϰؾ��
							Lagtim( OPETCBNO, 17, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 4)*50));	// �ޱ����ޕ� ���ۯ��҂���ϰ����
						}
					}
				}else if(OPECTL.Comd_cod == 0x11){
					if( !OPE_SIG_DOOR_Is_OPEN && OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5 	// �O�ʔ�������`�F�b�N�̓��o�͐M���`�F�b�N���ȊO
						&& OPE_IS_EBABLE_MAG_LOCK){													// ���d�����b�N���s��
						OPE_SIG_OUT_DOORNOBU_LOCK(1);								// �ޱ���ۯ�
						wmonlg( OPMON_LOCK_CLOSE, 0, 0 );							// ����o�^
						wopelg2( OPLOG_DOORNOBU_LOCK, 0L, 0L );						// ���엚��o�^(�ޱ���ۯ�)
						DoorLockTimer = 0;												// �ޱ�J��� ��ۯ�������ϰ�ر
						Lagcan( OPETCBNO, 17 );										// ��ϰ11ؾ��
						attend_no = 0;					// �h�A���b�N�W�������N���A
					}
				}
				else if( OPECTL.Comd_cod == 0x09 ){	// Time out No.9
					creCtrl( CRE_EVT_TIME_UP );  	//��ѱ�ď���
				}
				else if(OPECTL.Comd_cod == 0x16){
					AVM_Sodiac_Play_Wait_Tim(0);	// 0ch�����Đ��v�����M
				}
				else if(OPECTL.Comd_cod == 0x17){
					AVM_Sodiac_Play_Wait_Tim(1);	// 1ch�����Đ��v�����M
				}
				else if(OPECTL.Comd_cod == 0x18){
					AVM_Sodiac_TimeOut();			// �����I���ʒm�Ȃ���ϰ��ѱ��
				}
				else if(OPECTL.Comd_cod == 30){
					Ope_CenterDoorResend();			// �Z���^�[�ɃG���[�E�A���[���𑗂鏈��
				}
				else if(OPECTL.Comd_cod == 33){
					alm_chk( ALMMDL_MAIN, ALARM_DOORALARM, 1 );			// �h�A�x�񌟒m
				}
				else if(OPECTL.Comd_cod == 34){
					alm_chk( ALMMDL_MAIN, ALARM_VIBSNS, 1 );			// �U���Z���T�[���m
				}
				else if(OPECTL.Comd_cod == 35){
					alm_chk( ALMMDL_MAIN, ALARM_NOTERE_REMOVE, 1 );		// �����E�����m
				}
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
				else if(OPECTL.Comd_cod == TIMERNO_EC_BRAND_NEGO_WAIT) {
					if (isEC_USE()) {
						if ((ECCTL.phase == EC_PHASE_BRAND) &&
							(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3)) {
							// �t�F�[�Y�͂��̂܂܂őҋ@��Ԃɖ߂����^�C�~���O�ōĊJ����
						}
						else {
							// �t�F�[�Y�����������ɖ߂��ăX�^���o�C�R�}���h�����蒼��
							ECCTL.phase = EC_PHASE_INIT;				// ��������
							ECCTL.step = 0;
						}
					}
				}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
				TimeAdjustMain(sc.comd);
				break;

			case MSGGETHIGH(COIN_EVT):								// Coin Mech event
				if (OPECTL.Mnt_mod) {						// �����e�i���X��
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);							// �`���C����~
				}
				OPECTL.CN_QSIG = OPECTL.Comd_cod;
				switch( OPECTL.CN_QSIG ){
					case MSGGETLOW(COIN_ES_TIM):					// ���ۑ҂���ѱ��?
						safecl( 7 );								// ���ɖ����Z�o
						sc.comd = COIN_EVT;
						break;
					case MSGGETLOW(COIN_CASSET):					// ���Ē�?
						wopelg( OPLOG_TURICASETTE, 0, 0 );
						break;
					case MSGGETLOW(COIN_INVENTRY):					// ү����݂ɂ�������؏I��
						wopelg( OPLOG_INVBUTTON, 0, 0 );
						break;
					case MSGGETLOW(COIN_RJ_EVT):					// ���Z���~
						sc.comd = COIN_RJ_EVT;
						break;
					default:
						sc.comd = COIN_EVT;
						break;
				}
				break;

			case MSGGETHIGH(NOTE_EVT):								// Note Reader event
				if (OPECTL.Mnt_mod) {								// �����e�i���X��
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);									// �`���C����~
				}
				OPECTL.NT_QSIG = OPECTL.Comd_cod;
				if( sc.comd == NOTE_SO_EVT ){						// Note safe event
					if( msb->msg.data[0] == 1 ){					// �����J
						inc_dct( MOV_NOTE_BOX, 1 );					// �������ɔ��� ���춳��up
						if( OPECTL.Ope_Mnt_flg != 4 && OPECTL.Mnt_mod != 5){  			// ����������?
							if( OPECTL.note_syukei == 0 ){			// �������ɏW�v���o��?
								note_kinko_evt = 1;					// �������Ɉ�������
							}
						}
						wopelg( OPLOG_NOTEKINKO, 0, 0 );			// ���엚��۸ޓo�^
						wmonlg( OPMON_NOTEKINKO_UNSET, 0, 0 );		// ����o�^
						sc.comd = 0;
					}
					else{											// ������
						wmonlg( OPMON_NOTEKINKO_SET, 0, 0 );		// ����o�^
						sc.comd = 0;
					}
				}else{
					sc.comd = NOTE_EVT;
				}
				break;

			case MSGGETHIGH(PRNFINISH_EVT):							// Print End event
				if (OPECTL.Mnt_mod) {								// �����e�i���X��
					if(!Ope_IsEnableDoorKnobChime()) {
						// �h�A�m�u�J�`���C����������Ԃň󎚂��I�������ꍇ�͗L���ɂ���
						Ope_EnableDoorKnobChime();
					}
				}
				FrmEnd = (T_FrmEnd*)msb->msg.data;
				OPECTL.Pri_Result	= FrmEnd->BMode;				// �󎚌��ʂ�ܰ��֊i�[
				OPECTL.Pri_Kind		= FrmEnd->BPrikind;				// �������ʂ�ܰ��֊i�[
				OPECTL.PriEndMsg[(OPECTL.Pri_Kind == R_PRI)? 0 : 1] = *FrmEnd;

				PrintKind = sc.comd & (~INNJI_ENDMASK);				// �󎚎�ʎ擾
				if (PrintKind == PREQ_COKINKO_G ||					// �R�C�����ɍ��v
					PrintKind == PREQ_SIKINKO_G) {					// �약���ɍ��v
					// ���ɏW�v��ُ͈�ł��W�v�����Ƃ���
					if( OPECTL.Pri_Kind == R_PRI ){					// �������ʁH
						// ڼ�Ĉ󎚏I��
						rct_goukei_pri = OFF;
					}
					else{
						// �ެ��و󎚏I��
						jnl_goukei_pri = OFF;
					}
					if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){

						// ���v�L�^�̈󎚂��S�ďI�������ꍇ

						switch( PrintKind ){
							case PREQ_COKINKO_G:					// �R�C�����ɏW�v����
								// NOTE: ڼ��������A�ެ���������ǂ��炩���ُ펞�́A����Ɉ󎚂ł������̲���Ă����Ԃ�Ȃ��d�g�݂ɂȂ��Ă��邪�A
								//       ڼ��������A�ެ���������Ƃ��Ɉُ펞�́A���ꂼ��̈󎚈ُ�I���̲���Ă��Ԃ�d�g�݂ɂȂ��Ă���B
								if(OPECTL.coin_syukei != (char)-1){ // ��݋��ɏW�v���W�v�̏ꍇ�Ɏ��{����B
									OPECTL.coin_syukei = (char)-1;	// ��݋��ɏW�v�o�͍ς݂Ƃ���
									kinko_clr( 0 );					// �R�C�����ɏW�v�ر�X�V����
								}
								break;
							case PREQ_SIKINKO_G:					// �������ɏW�v����
								// NOTE: ڼ��������A�ެ���������ǂ��炩���ُ펞�́A����Ɉ󎚂ł������̲���Ă����Ԃ�Ȃ��d�g�݂ɂȂ��Ă��邪�A
								//       ڼ��������A�ެ���������Ƃ��Ɉُ펞�́A���ꂼ��̈󎚈ُ�I���̲���Ă��Ԃ�d�g�݂ɂȂ��Ă���B
								if(OPECTL.note_syukei != (char)-1){     // �������ɏW�v���W�v�̏ꍇ�Ɏ��{����B
									OPECTL.note_syukei = (char)-1;		// �������ɏW�v�o�͍ς݂Ƃ���
									kinko_clr( 1 );						// �������ɏW�v�ر�X�V����
								}
								break;
							default:
								break;
						}
					}
				}
				else if( PrnGoukeiChk( PrintKind ) == YES ){		// T���v, GT���v�L�^�̈󎚏I���H
					// ���v�L�^�̈󎚏I��
					if( FrmEnd->BMode == PRI_NML_END ){				// ����I��?
						// ����I��
						if( OPECTL.Pri_Kind == R_PRI ){				// �������ʁH
							// ڼ�Ĉ󎚏I��
							rct_goukei_pri = OFF;
						}
						else{
							// �ެ��و󎚏I��
							jnl_goukei_pri = OFF;
						}
						if( rct_goukei_pri == OFF && jnl_goukei_pri == OFF ){

							// ���v�L�^�̈󎚂��S�ďI�������ꍇ�i��������͐ݒ�ɏ]���j

							switch( PrintKind ){
								case PREQ_TGOUKEI:					// �s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									if( Check_syuukei_clr( 0 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									ac_flg.syusyu = 11;				// 11:�s���v�󎚊���
									Make_Log_TGOUKEI();				// �Ԏ����W�v���܂߂�T���v۸ލ쐬
									if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){	// �s���v���M����
										NTNET_Snd_TGOUKEI();
									}
									syuukei_clr( 0 );				// �s�W�v�ر�X�V����
									if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// �o�b�`���M�ݒ�L
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
								case PREQ_GTGOUKEI:					// �f�s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									if( Check_syuukei_clr( 1 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									syuukei_clr( 1 );				// �f�s�W�v�ر�X�V����
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
								case PREQ_MTGOUKEI:					// �l�s���v�󎚊���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									if( Check_syuukei_clr( 2 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									ac_flg.syusyu = 102;			// 102:�l�s���v�󎚊���
									syuukei_clr( 2 );				// �l�s�W�v�ر�X�V����
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
									break;
								default:
									break;
							}
						}
					}
					else {
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;
					// �W�v�ł��Ȃ���Βǔԕԋp
						switch( PrintKind ){
						case PREQ_TGOUKEI:
							CountFree(T_TOTAL_COUNT);
							break;
						case PREQ_GTGOUKEI:
							CountFree(GT_TOTAL_COUNT);
							break;
						default:
							break;
						}
					}
				}
				else if( PrintKind == PREQ_AT_SYUUKEI ){			// �����W�v�̈󎚏I���H
					// �����W�v�̈󎚏I��
					if( OPECTL.Pri_Kind == R_PRI ){					// �������ʁH
						// ڼ�Ĉ󎚏I��
						rct_atsyuk_pri = OFF;
					}
					else{
						// �ެ��و󎚏I��
						jnl_atsyuk_pri = OFF;
					}
					if( rct_atsyuk_pri == OFF && jnl_atsyuk_pri == OFF ){
						// �����W�v�̈󎚂��S�ďI�������ꍇ�i��������͐ݒ�ɏ]���j	�������W�v�̏ꍇ�A�ُ�I���ł��W�v�����Ƃ���
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						if( Check_syuukei_clr( 0 )){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						ac_flg.syusyu = 11;							// 11:�s���v�󎚊���
						Make_Log_TGOUKEI();							// �Ԏ����W�v���܂߂�T���v۸ލ쐬
						if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){	// �s���v���M����
							NTNET_Snd_TGOUKEI();
						}
						syuukei_clr( 0 );							// �s�W�v�ر�X�V����
						if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// �o�b�`���M�ݒ�L
			 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
							}
							else {
								ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
							}
						}
						auto_syu_prn = 3;							// �W�v��� �� �����W�v����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//						// Edy�ݒ肪���邩								�������ݸނ�T���v�ォ
//						if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && prm_get(COM_PRM, S_SCA, 57, 1, 6) == 0 ) {	
//							auto_centercomm( 2 );						// �����W�v�i�s���v����āj��̎����Z���^�[�ʐM�i���߁j�������{
//						}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
					}
				}
				else if( PrintKind == PREQ_TURIKAN ){					// �ޑK�Ǘ��󎚊���

					uchar terget_pri = PrnGoukeiPri(PREQ_TURIKAN);	// �󎚐�擾
					uchar clr = 0;

					switch( terget_pri ){
						case 0: // �Ȃ�
						case R_PRI:
							if( OPECTL.Pri_Kind == R_PRI ){
								clr = 1;
							}
							break;
						case J_PRI:
							if( OPECTL.Pri_Kind == J_PRI ){
								clr = 1;
							}
							break;
						case RJ_PRI:
							if( OPECTL.Pri_Kind == R_PRI ){
								turikan_pri_status |= 0x01;
							}
							if( OPECTL.Pri_Kind == J_PRI ){
								turikan_pri_status |= 0x02;
							}
							if( turikan_pri_status == 0x03 ){
								clr = 1;
							}
							break;
					}
					if( clr ){
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						if( Check_turikan_clr()){
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						turikan_clr();
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
						}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
					}
				}
// MH321800(S) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				else if (PrintKind == PREQ_EDY_SYU_LOG
//						|| PrintKind == PREQ_SCA_SYU_LOG
				else if (PrintKind == PREQ_SCA_SYU_LOG
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						|| PrintKind == PREQ_EC_SYU_LOG) {
					// �����Ȃ�
					// �W���[�i���󎚒��Ɏ��؂ꂪ���������ꍇ�A���̒���̏����ɂ�胁�b�Z�[�W�����ɏ����������Ă��܂�
					// �����e�i���X��ʂ܂ŃC�x���g�����B���Ȃ��B
					// ���̂��߁A�����W�v�ł͂��̃p�X��ʂ����b�Z�[�W�̏��������������Ȃ�
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
				else if(OPECTL.Mnt_mod != 0){
					if((OPECTL.Pri_Result == PRI_ERR_END) && (OPECTL.Pri_Kind == J_PRI)){
						if( OPECTL.Mnt_mod != 0 && OPECTL.Mnt_mod != 5 ){				// �����e�i���X��
							if( !OPE_SIG_DOORNOBU_Is_OPEN ){							// �ޱ��ޕ��
								queset(OPETCBNO, KEY_MODECHG, 1, &on_off);
							}
						}
						if (OPECTL.Mnt_mod) {						// �����e�i���X��
							if( Ope_IsEnableDoorKnobChime() && sc.comd != KEY_MANDET ){	// �h�A�m�u�߂��Y��`���C���L��
								Ope_StartDoorKnobTimer();
							}
							if( sc.comd != KEY_MANDET ) {			// �l�̌��m�ȊO�̓A���[����~����
								opa_chime(OFF);						// �`���C����~
							}
						}
						if( sc.comd == KEY_RXIIN )
						{	// RYB���͐M��
							if( OPECTL.Ope_Mnt_flg != 8 && OPECTL.Mnt_mod != 5){  		// ���o�͐M���������łȂ�
								InSignalCtrl(INSIG_OPOPEN);			//�����c�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
								InSignalCtrl(INSIG_OPCLOSE);		//�����x�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
								if( OPECTL.on_off == 0 ){			// OFF?(Y)
									sc.comd = 0;					// OFF����ү���ޖ����Ƃ���
								}
							}
						}else{
							if( OPECTL.on_off == 0 ){				// OFF?(Y)
								sc.comd = 0;						// OFF����ү���ޖ����Ƃ���
							}
						}
					}
				}
				break;

			case MSGGETHIGH(PRINTER_EVT):							// ������ð����M
				if (OPECTL.Mnt_mod) {								// �����e�i���X��
					Ope_StartDoorKnobTimer();
					opa_chime(OFF);									// �`���C����~
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//				if( sc.comd == PREQ_JUSHIN_ERR1 ){					// ������ð����M
				if( sc.comd == PREQ_JUSHIN_ERR1 ||					// ������ð����M
					sc.comd == PREQ_JUSHIN_INFO ){
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					unsigned char	NEW,OLD;
					unsigned char	max;
					
					do{
						max = Printer_Status_Get( msb->msg.data[0], &NEW, &OLD );
						if( max == 0xff ) {
							// �����Ώۂ̃v�����^�C�x���g�Ȃ��̏ꍇ�͏����I��
							break;
						}
						
						
						if( msb->msg.data[0] == R_PRI ){
							// ڼ��������ð����M
							
							err_pr_chk( NEW, OLD ); // ڼ��������װ����
							if( ( ( OLD & 0x02 ) != 0 )&&
								( ( NEW & 0x02 ) == 0 ) ){  /* �W����? */

								if( OPE_SIG_DOOR_Is_OPEN ){					// �ޱ�J?
									FrmIrekaeTest.prn_kind = R_PRI;
									queset( PRNTCBNO, PREQ_IREKAE_TEST, sizeof(T_FrmIrekaeTest), &FrmIrekaeTest );
																					// �p������ւ����e�X�g��
								}
							}
						}
						else{
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
							if (sc.comd == PREQ_JUSHIN_INFO) {
								err_eja_chk(NEW, OLD);	// �G���[�`�F�b�N
// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5944 SD�J�[�h���p�s���̐��Z�@�̌x����ʂ��Ⴄ
								lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810104 GG119201(E) �d�q�W���[�i���Ή� #5944 SD�J�[�h���p�s���̐��Z�@�̌x����ʂ��Ⴄ
							}
							else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
							// �ެ���������ð����M
							err_pr_chk2( NEW, OLD ); // �ެ���������װ����
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
							}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
						}
					}while( max );
				}
				break;

			case MSGGETHIGH(ARC_CR_EVT):							// Reader
				opr_ctl( sc.comd );									// message���͏���
				break;

			case MSGGETHIGH(IBK_EVT):								// IBK event
				if(( sc.comd >= IBK_MIF_EVT_ERR )&&
				   ( sc.comd <= IBK_MIF_AF_NG_EVT )){
					/* Mifare event */
					memcpy( &MIF_ENDSTS.sts1, &msb->msg.data[0], sizeof( t_MIF_ENDSTS ));	// �I���ð�����
					OpMif_ctl( sc.comd );							// message���͏���
				}
				else if(( sc.comd >= IBK_NTNET_ERR_REC )&&
					    ( sc.comd <= IBK_NTNET_BUFSTATE_CHG )){
					/* Nt-net event */
					NTNET_GetRevData( sc.comd, &msb->msg.data[0] );	// message����(��M�ް����)����
				}
				else if( sc.comd == IBK_NTNET_CHKPASS ){
					/* ������⍇�������ް���M */
					memcpy( &PassChk, &msb->msg.data[0], 22 );		// ������⍇�����ʕۑ�
				}
				else if(( sc.comd >= IBK_LPR_SRLT_REC )&&
					    ( sc.comd <= IBK_LPR_B1_REC )){
					/* Label printer event */
					sc.comd = OpLpr_ctl(sc.comd, msb->msg.data);	// message���͏���
				}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//				else if((sc.comd & 0xfff0) == (IBK_EDY_RCV & 0xfff0)) {
//					Edy_Ope_Recv_Ctrl( sc.comd, msb->msg.data );
//				}
//
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				break;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			case MSGGETHIGH(CRE_EVT):								// credit event
//				if( sc.comd == CRE_EVT_RECV ){
//					creCtrl( CRE_EVT_RECV_ANSWER );					// �N���W�b�g�����f�[�^��M
//				}
//				else if ( sc.comd == CRE_EVT_CONN_PPP) {			// PPP�ڑ�����
//					if( cre_ctl.Initial_Connect_Done == 0 ) {		// ����ڑ�(�J��)������
//						if( CREDIT_ENABLED() ){
//							cre_ctl.OpenKind = CRE_KIND_STARTUP;	// �J�ǔ����v���ɋN������ݒ�
//							creCtrl( CRE_EVT_SEND_OPEN );			// �J�ǃR�}���h(01)���M.
//						}
//					}
//				}
//				break;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)

			case MSGGETHIGH(SUICA_EVT):								// ��ʌnIC�J�[�h�C�x���g
				if( SUICA_EVT == sc.comd ){							// SX-10���烁�b�Z�[�W��M
					Ope_Suica_Event( sc.comd, OPECTL.Ope_mod );
				}
				else if( SUICA_INITIAL_END == sc.comd ){
					if( Suica_Rec.Data.BIT.FUKUDEN_SET ){			/* ���Z�f�[�^���M�v������ 		*/
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//-						Suica_Data_Snd( S_PAY_DATA,&CLK_REC);		/* SX-10���ߏ������s			*/
//-						Suica_Rec.Data.BIT.FUKUDEN_SET = 0;			/* ���Z�f�[�^���M�v���N���A		*/
						if ( isSX10_USE() ) {
							Suica_Data_Snd( S_PAY_DATA,&CLK_REC);	/* SX-10���ߏ������s			*/
							Suica_Rec.Data.BIT.FUKUDEN_SET = 0;		/* ���Z�f�[�^���M�v���N���A		*/
						}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
					}
					// �C�j�V�������������_�ŁA�ҋ@�Ŏ�t�s���(��t�����M���Ă��Ȃ�)�̏ꍇ�́A��t�𑗐M����
					if( (( !Suica_Rec.Data.BIT.CTRL_MIRYO && !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ) &&
						 ( OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 ))){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						if (isEC_USE()) {
							EcSendCtrlEnableData();
						}
						else {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
						Suica_Ctrl( S_CNTL_DATA, 0x01 );						// Suica���p���ɂ���
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
					}
				}
				else if( SUICA_PAY_DATA_ERR == sc.comd ){
					PayDataErrDisp();
				}
				break;
			case MSGGETHIGH(REMOTE_DL_EVT):							// ���u�_�E�����[�h�C�x���g
				remote_evt_recv( sc.comd, msb->msg.data );
				break;
			case MSGGETHIGH(I2C_EVT):								// I2C�C�x���g
				switch( sc.comd ){
					case	I2C_RP_ERR_EVT:
					case	I2C_JP_ERR_EVT:
						I2C_Err_Regist( sc.comd, msb->msg.data[0]);
						break;
				}
				break;

			case MSGGETHIGH(SODIAC_EVT):							// SODIAC�C�x���g
				switch( sc.comd ){
					case	SODIAC_NEXT_REQ:
						AVM_Sodiac_Play_WaitReq( msb->msg.data[0], msb->msg.data[1]);
						break;
				}
				break;
			case MSGGETHIGH(SNTPMSG_EVT):
				TimeAdjustMain(sc.comd);
				break;
			case MSGGETHIGH(OPE_EVT):
				switch (sc.comd) {
				case OPE_REQ_CALC_FEE:
					// �Ԏ������`�F�b�N
					if (CheckReqCalcData()) {
						sc.comd = 0;
					}
					break;
				default:
					break;
				}
				break;
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
			case MSGGETHIGH(EC_EVT):
				switch (sc.comd) {
				case EC_EVT_COMFAIL:				// �ʐM�s�ǔ���
				case EC_EVT_NOT_AVAILABLE_ERR:		// ���σ��[�_�؂藣���G���[����
					if (1 <= OPECTL.Ope_mod && OPECTL.Ope_mod <= 3) {
						// �ҋ@��ԈȊO��JVMA���Z�b�g���Ȃ�
						break;
					}
					// JVMA���Z�b�g��v�����ăR�C�����b�N�^�X�N�̏�����������҂�
					Ec_check_PendingJvmaReset();
					break;
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
				case EC_EVT_DEEMED_SETTLE_FUKUDEN:
					// �݂Ȃ����ψ����i���d�j�̏���
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//					EcRegistSettlementAbort();
					// ���Ϗ������s(�x��)
// MH810103 MHUT40XX(S) E3247�󎚎��̗��R�R�[�h�ԈႢ���C��
//					EcRegistDeemedSettlementError(1);
					EcRegistDeemedSettlementError(2);
// MH810103 MHUT40XX(E) E3247�󎚎��̗��R�R�[�h�ԈႢ���C��
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
					sc.comd = 0;
					break;
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
				default:
					break;
				}
				break;
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
			default:
				break;
		}
		break;
	}

	FreeBuf( msb );
	return( sc.comd );
}

//[]----------------------------------------------------------------------[]
///	@brief			���ɏW�v
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	void	kinko_syuukei(void)
{
	if (coin_kinko_evt == 0 && note_kinko_evt == 0 ){
		return;		// �ۗ��C�x���g�Ȃ�
	}
	if (OPECTL.Mnt_mod == 0 &&
		(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3)){
		return;		// ���Z���͂��Ȃ�
	}
	if (turi_dat.turi_in != 0){
		return;		// �ޑK�Ǘ���
	}
	if (CountCheck() == 0){
		return;		// ���̍��v�󎚒�
	}
	if ( coin_kinko_evt == 1 ){
		coin_kinko_evt = 0;
		if(OPECTL.coin_syukei == 0){		// ��݋��ɏW�v���o��?
			kinko_syu( 0, 1 );				// ��݋��ɏW�v&��
			OPECTL.coin_syukei = 1;			// ��݋��ɏW�v�o�͒��Ƃ���
			return;
		}
	}
	if ( note_kinko_evt == 1 ){
		note_kinko_evt = 0;
		if( OPECTL.note_syukei == 0){		// �������ɏW�v���o��?
			kinko_syu( 1, 1 );				// �������ɏW�v&��
			OPECTL.note_syukei = 1;			// �������ɏW�v�o�͒��Ƃ���
			return;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Ӽޭ�ًN���҂�                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mojule_wait                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mojule_wait( void )
{
	unsigned char	CNMStartFlg;
	unsigned long	CNMStartTime;
	ulong req;
	long param;
// MH810103 GG119202(S) ���σ��[�_�N�������҂����Ԑݒ�폜
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	unsigned long	ec_WakeUp_Timer = 0;
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH321800 GG119202(E) ���σ��[�_�N�������҂����Ԑݒ�폜
// MH321800(S) E3210�o�^�^�C�~���O�ύX
	short	timer;
// MH321800(E) E3210�o�^�^�C�~���O�ύX

	read_sht_opn();													// ذ�ް������J,LED�_��

	LedReq( CN_TRAYLED, LED_ONOFF );								// ��ݎ�o�����޲��LED�_��

	Ope_ErrAlmClear();												// NT-NET HOST�װ/�װщ����ް����M
	Suica_fukuden_que_check();										// ���d���̎�M�o�b�t�@�`�F�b�N


	/*--------------------------------------------------------------*/
	/*	�����ؾ�ĉ��� ������̳��Ă� �ő�5�b�K�v					*/
	/*	��������A�͐��펞3�b,��Ķ������ʏ�ʒu�ɂȂ��ꍇ5�b�K�v		*/
	/*	ү���ؾ�ĉ�������܂ų���2�b�K�v							*/
	/*	ARCNET�͋N���܂łɍő�ARC_INI_TOUT�b�K�v(�ʐM�ُ펞��)		*/
	/*--------------------------------------------------------------*/
	ExIOSignalwt(EXPORT_JP_RES, 0);									// �W���[�i���v�����^���Z�b�g

//	timeout = 0;
	CNMStartFlg = 1;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
	if( (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 6) != 0 ){		// ���σ��[�_�ڑ�����H
		CP_CN_SYN = 0;
		// �^�C�}���Ԃ�� 10ms * (X * 100) = Xsec Wait
// MH810103 GG119202(S) ���σ��[�_�N�������҂����Ԑݒ�폜
//		ec_WakeUp_Timer = (ulong)prm_get(COM_PRM, S_ECR, 20, 3, 1);
//// MH321800(S) E3210�o�^�^�C�~���O�ύX
////		if(ec_WakeUp_Timer == 0) {
////			ec_WakeUp_Timer = 300;
////		} else if(ec_WakeUp_Timer < 10){
////			ec_WakeUp_Timer = 10;
////		}
//		if (ec_WakeUp_Timer < 10) {
//			// �ŏ��l��10�b�Ƃ���
//			ec_WakeUp_Timer = 10;
//		}
//// MH321800(E) E3210�o�^�^�C�~���O�ύX
//		ec_WakeUp_Timer *= 100;
// MH810103 GG119202(E) ���σ��[�_�N�������҂����Ԑݒ�폜
	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	CNMStartTime = LifeTimGet();
	for( ; ; ){

		taskchg( IDLETSKNO );										// task change

		Ope_MsgBoxClear();

// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
		if( (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 6) == 0 ){		// ���σ��[�_�ڑ��Ȃ��H
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
		if( CNMStartFlg ){
			if( 200 <= LifePastTimGet( CNMStartTime ) ){			// 10ms * 200 = 2sec Wait
				CNMTSK_START = 1;									// ���ү�����N��
				CP_CN_SYN = 1;										// ���ү�ؾ�ĉ���
				CNMStartFlg = 0;
				// �N���܂ł̑҂����Ԃ̉��P�A�R�C�����b�N�X�^���o�COK�ő҂��I���Ƃ���
				break;
			}
		}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
		}
		else {														// ���σ��[�_�ڑ�����H
			if( CNMStartFlg == 1 ){
				// SYN�M����1�b��L���x���ɂ���
				if( 100 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 100 = 1sec Wait
					CP_CN_SYN = 1;									// ���ү�ؾ�ĉ���
					CNMStartFlg = 2;
					CNMStartTime = LifeTimGet();
				}
			}
			else if( CNMStartFlg == 2 ){
				// SYN�M����H���x���ɂ��Ă���4�b��ɃR�C�����b�N�^�X�N���N������
				// CN_reset()�ɍ��킹��4�b��Ƃ���
				if( 400 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 400 = 4sec Wait
					CNMTSK_START = 1;								// ���ү�����N��
					CNMStartFlg = 0;
					CNMStartTime = LifeTimGet();
// MH321800(S) E3210�o�^�^�C�~���O�ύX
// MH810103 GG119202(S) E3210�o�^�����C��
//					// JVMA�ʐM�J�n����3��(�Œ�)�o�߂��Ă����σ��[�_�̏������V�[�P���X���������Ȃ��ꍇ�A
//					// E3210��o�^����
					// JVMA�ʐM�J�n���珉�����V�[�P���X�����҂����Ԍo�߂��Ă�
					// ���σ��[�_�̏������V�[�P���X���������Ȃ��ꍇ�AE3210��o�^����
// MH810103 GG119202(E) E3210�o�^�����C��
					timer = OPE_EC_WAIT_BOOT_TIME * 2;
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
					if (EcReaderVerUpFlg != 0) {
						timer = OPE_EC_WAIT_BOOT_TIME_VERUP * 2;
					}
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
					LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH321800(E) E3210�o�^�^�C�~���O�ύX
// MH810103 GG119202(S) �N������������
//// GG119202(S) ���σ��[�_�N�������҂����Ԑݒ�폜
//					// ���σ��[�_�̋N��������҂����ɑҋ@��ʂ֑J�ڂ���
//					break;
//// GG119202(E) ���σ��[�_�N�������҂����Ԑݒ�폜
					CNMStartFlg = 3;
					CNMStartTime = LifeTimGet();
// MH810103 GG119202(E) �N������������
				}
			}
// MH810103 GG119202(S) ���σ��[�_�N�������҂����Ԑݒ�폜
//			else {
//				// ���σ��[�_�̋N����҂�
//				if( Suica_Rec.Data.BIT.INITIALIZE == 1 ){				// ���σ��[�_�����������H
//					break;
//				}
//				if( Suica_Rec.Data.BIT.INITIALIZE != 1 && 
//					ec_WakeUp_Timer <= LifePastTimGet( CNMStartTime ) ){
//// MH321800(S) E3210�o�^�^�C�~���O�ύX
////					// E3210����
////					err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 1, 0, 0 );
//// MH321800(E) E3210�o�^�^�C�~���O�ύX
//					break;
//				}
//			}
// MH810103 GG119202(E) ���σ��[�_�N�������҂����Ԑݒ�폜
// MH810103 GG119202(S) �N������������
			else if( CNMStartFlg == 3 ){
				if( 50 <= LifePastTimGet( CNMStartTime ) ){		// 10ms * 50 = 0.5sec Wait
				// �R�C�����b�N�^�X�N�N���ɂ��ڑ�����郊�[�_�[�����肳��邽�߁A�҂����킹��B
					break;
				}
			}
// MH810103 GG119202(E) �N������������
		}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
	}

	/*** Ӽޭ�ُ����� ***/
	/*--------------------------------------------------------------*/
	/*	���Cذ�ް�̏������͐��펞��ذ�ް�Ƽ�ٓ��슮����M�ɂĊ����B	*/
	/*	�ُ펞����ѱ�Ă�15�b�ɂ��Ă��邽�ߍő�15�b�K�v�B			*/
	/*--------------------------------------------------------------*/

	req = 0;

	if (StartingFunctionKey == 1) {
		// F3F5�������ꂽ�Ƃ��́A�������Ɏ��v�f�[�^�Ɛ���f�[�^��v������B
		req |= NTNET_MANDATA_CLOCK;									// ���v�ް��v��
		req |= NTNET_MANDATA_CTRL;									// �����ް��v��
	} else {
		param = prm_get(0, S_NTN, 43, 1, 1);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ���v�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_CLOCK;
		}
		param = prm_get(0, S_NTN, 43, 1, 2);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ����f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_CTRL;
		}
		param = prm_get(0, S_NTN, 43, 1, 3);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ���ʐݒ�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_COMMON;
		}
		param = prm_get(0, S_NTN, 43, 1, 4);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ������X�e�[�^�X�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSSTS;
		}
		param = prm_get(0, S_NTN, 44, 1, 4);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ������X�V�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSCHANGE;
		}
		param = prm_get(0, S_NTN, 43, 1, 6);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ������o�Ɏ����f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSEXIT;
		}
		param = prm_get(0, S_NTN, 44, 1, 1);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// �Ԏ��p�����[�^�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_LOCKINFO;
		}
		param = prm_get(0, S_NTN, 44, 1, 2);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ���b�N���u�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_LOCKMARKER;
		}
		param = prm_get(0, S_NTN, 43, 1, 5);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ��������~�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSSTOP;
		}
		param = prm_get(0, S_NTN, 44, 1, 6);
		if ((param == 1) || (param == 2 && StartingFunctionKey == 2)) {
			// ���ʓ��ݒ�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_SPECIALDAY;
		}
	}
	NTNET_Snd_Data104(req);											// NT-NET�Ǘ��ް��v���쐬

	NTNET_Snd_Data190();
	rd_shutter();													// ���Cذ�ް���������

	LedReq( CN_TRAYLED, LED_OFF );									// ��ݎ�o�����޲��LED OFF

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �񏉊����ر�ر                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : memclr                                                  |*/
/*| PARAMETER    : flg = 1:�������Ÿر                                     |*/
/*| RETURN VALUE : 0:saved memory  1:memory all clear                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	memclr( char flg )
{
	char	ret;
	ushort	i;

	if (memcmp(PASSPT.pas_word, PASSDF.pas_word, sizeof(PASSPT.pas_word)) != 0) {
		f_ParaUpdate.BYTE = 0;										// �������j���̂��߁A�X�V�t���O���N���A
	}

	if( flg != 0 )													// �������ر?
	{
		ret = 1;
		memset( &FLAGPT, 0, sizeof( flg_rec ) );
		memset( &FLAPDT, 0, sizeof( flp_rec ) );
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			FLAPDT.flp_data[i].timer = -1;							// �ׯ�ߏ㏸(ۯ����u��)��ϰ�A׸������ϰ�ر
			FLPCTL.Flp_mv_tm[i] = -1;								// �ׯ�ߔ���Ď���ϰ�ر
			FLPCTL.Flp_uperr_tm[i] = -1;							// �㏸ۯ�(�ُ�)�װ������ϰ�ر
			FLPCTL.Flp_dwerr_tm[i] = -1;							// ���~ۯ�(�J�ُ�)�װ������ϰ�ر
			FLAPDT.flp_data[i].in_chk_cnt = -1;						// ���ɔ�����ϰ�ر
		}
		memset( FLAPDT_SUB, 0, sizeof( flp_com_sub )*10 );			// �ׯ�߻�޴ر�ر
		memset( Mov_cnt_dat, 0, sizeof( Mov_cnt_dat ) );
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		memset( pas_tbl, 0, sizeof( PAS_TBL )*PAS_MAX );			// �����ð��ٸر
//		memset( pas_renewal, 0, sizeof( PAS_TBL )*(PAS_MAX/4) );	// ������X�Vð��ٸر
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
		memset( syusei, 0, sizeof( struct SYUSEI )*LOCK_MAX );
		memset( &tki_cyusi, 0, sizeof( t_TKI_CYUSI ) );				// ��������~�ޯ̧�ر
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		memset( &pas_extimtbl, 0, sizeof( PASS_EXTIMTBL ) );		// ������o�Ɏ���ð��ٸر
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
		memset( NTNetDataCont, 0, sizeof( NTNetDataCont ) );		// �e�ް��ǂ���
		memset( UketukeNoBackup, 0, sizeof( UketukeNoBackup ) );	// ��t�����s�ǔ��ޯ����߸ر
		memset( &CarCount, 0, sizeof( CAR_COUNT ) );				// ���o�ɑ䐔���Ĵر�ر
		memset( &CarCount_W, 0, sizeof( CAR_COUNT ) );				// ���o�ɑ䐔����ܰ��ر�ر
		syu_init();
		cnm_mem( 0 );
		memset( &ac_flg, 0, sizeof(struct AC_FLG) );				// 0:�ҋ@���
		memset( &Attend_Invalid_table, 0, sizeof( Attend_Invalid_table ) );				// �W������ð���
// MH810100(S) Y.Yamauchi  2019/12/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
		memset( &ticketdata, 0, sizeof( ticketdata ) );				// �W������ð���
// MH810100(E) Y.Yamauchi  2019/12/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		LaneStsSeqNo = 0;											// ��Ԉ�A�ԍ��N���A
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

		LKcom_PassBreak();

		memcpy( PASSPT.pas_word, PASSDF.pas_word, sizeof( PASSPT.pas_word ) );	// �߽ܰ��[AMANO]���
		auto_syu_prn	= 0;	// �����W�v���		������
		auto_syu_ndat	= 0;	// �����W�v�J�n���t	������
		coin_kinko_evt = 0;		// �R�C�����ɃC�x���g
		note_kinko_evt = 0;		// �������ɃC�x���g
		Mifare_WrtNgDataUpdate( 1, (t_MIF_CardData*)0L );			// Mifare�����ݎ��s���ޏ��ر
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//		NTBUF_AllClr();												//NT-NET�o�b�t�@���̃f�[�^��S�č폜
		NTBUF_AllClr_startup();										//NT-NET�o�b�t�@���̃f�[�^��S�č폜
		NTCom_ClearData(0);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		auto_cnt_prn	= 0;	// �����Z���^�[�ʐM���		������
//		auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);		// �ŏI�Z���^�[�ʐM���{�����Ɍ��ݎ��Ծ��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//		memset( &cre_saleng, 0, sizeof( cre_saleng ) );				// �N���W�b�g���㋑�ۃf�[�^
//		memset( &cre_uriage, 0, sizeof( cre_uriage ) );		// �N���W�b�g������W�`�F�b�N�G���A	
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//		memset( &ClkLastSend, 0, sizeof( ClkLastSend ) );	// �J�[�h���v���ŏI���M����
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
		creCtrlInit( flg );

// MH321800(S) hosoda IC�N���W�b�g�Ή�
		Product_Select_Data = 0;							// ���M�����I�����i�f�[�^(���ώ��̔�r�p)
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		Edy_OpeDealNo = 0;					// Main�Ǘ��pEdy����ʔ�
//		Edy_ShimeDataClear();								// EM�����ް��ꎞ�ۑ��ر�ر
//#endif
//		memset( &PayData_save, 0, sizeof(PayData_save) );	// ��d�OPayData�ۑ���ر
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		memset( &ETC_cache, 0xFF, sizeof(ETC_cache));
		memset( &bk_LockInfo, 0, sizeof(bk_LockInfo));
		memset( &CAL_WORK, 0, sizeof( t_cal_bak_rec ));							/* ���u�p�ް��i�[�ر	  byte	*/

		ntnet_nmax_flg = 0;							/*�@�ő嗿���׸޸ر	*/

		memset( LongTermParkingPrevTime, 0, sizeof( LongTermParkingPrevTime ));
// GM760201(S) �ݒ�A�b�v���[�h���̓d��OFF/ON��AMANO��ʂ���i�܂Ȃ��^�C�~���O�����邽�߁A�C��
//		remotedl_info_clr();
// GM760201(E) �ݒ�A�b�v���[�h���̓d��OFF/ON��AMANO��ʂ���i�܂Ȃ��^�C�~���O�����邽�߁A�C��
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		remotedl_info_clr();
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		memset(&Chk_info, 0, sizeof(Chk_info));
		for(i=0;i<RT_PAY_LOG_CNT;i++){
			memset(&RT_PAY_LOG_DAT.RT_pay_log_dat[i], 0 ,sizeof(Receipt_data));
		}
		RT_PAY_LOG_DAT.RT_pay_count = 0;
		RT_PAY_LOG_DAT.RT_pay_wtp = 0;
		memset( Encryption_Key[0], 0, sizeof(Encryption_Key[0]) * ENCKEY_NUM );
		// �Z���^�[�ǔԏ�����
		memset(Rau_SeqNo, 0, sizeof(Rau_SeqNo));
		for(i = 0; i < RAU_SEQNO_TYPE_COUNT; ++i) {
			Rau_SeqNo[i] = 1;
		}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		// DC-NET�ʐM�p�Z���^�[�ǔԏ�����
		memset( DC_SeqNo, 0, sizeof(DC_SeqNo) );
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
//		for( i = 0; i < DC_SEQNO_TYPE_COUNT; ++i ){
//			DC_SeqNo[i] = 1;
//		}
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
		memcpy( &DC_SeqNo_wk, &DC_SeqNo, sizeof( DC_SeqNo_wk ) );
		// ���A���^�C���ʐM�p�Z���^�[�ǔԏ�����
		memset( REAL_SeqNo, 0, sizeof(REAL_SeqNo) );
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
//		for( i = 0; i < REAL_SEQNO_TYPE_COUNT; ++i ){
//			REAL_SeqNo[i] = 1;
//		}
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//���t�ؑ֊�̏��v(���A���^�C���f�[�^�p)
		Date_Syoukei.Date_Time.Hour = 0xFF;
		Date_Syoukei.Date_Time.Min  = 0xFF;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�

// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));	// Edy��ʒ[��ID
		EcAlarmLog_Clear();
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
		memset(RecvBrandTbl, 0, sizeof(RecvBrandTbl));
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		memset(&eja_prn_buff, 0, sizeof(eja_prn_buff));
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
		EcReaderVerUpFlg = 0;
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		prn_job_id = 0;
		memset(&prn_dat_que, 0, sizeof(prn_dat_que));
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		memset(&pri_rcv_buff_r, 0, sizeof(PRN_RCV_DATA_BUFF_R));
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	}else{
		ret = 0;
		if( memcmp( PASSPT.pas_word, PASSDF.pas_word, sizeof( PASSPT.pas_word )) == 0 ){
			// �߽ܰ������OK
			nmitrap();
		}
		cnm_mem( 1 );
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			FLPCTL.Flp_mv_tm[i] = -1;								// �ׯ�ߔ���Ď���ϰ�ر
		}
		creCtrlInit( flg );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		memcpy( &PayData_save, &PayData, sizeof(PayData_save) );	// ��d�OPayData�ۑ�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		if(	2 == auto_syu_prn ){									// �u2:�W�v�󎚒��v
			auto_syu_prn = 0;										// 0:�󂫁i�W�v�����҂��j
		}
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		Remote_Cal_Data_Restor();
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/04/02 #4092 �Ԕԃ`�P�b�g���X(���s���Ă��o�b�N�A�b�v�������e�ɖ߂�Ȃ�)
//		for( i = 0; i < DC_SEQNO_TYPE_COUNT; ++i ){
//			if( DC_SeqNo[i] != DC_SeqNo_wk[i] ){
//				DC_SeqNo[i] = DC_SeqNo_wk[i];
//			}
//		}
// MH810100(E) K.Onodera  2020/04/02 #4092 �Ԕԃ`�P�b�g���X(���s���Ă��o�b�N�A�b�v�������e�ɖ߂�Ȃ�)
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
	}
	for( i = 0; i < BIKE_START_INDEX; i++ ){									/* �t���b�v */
		if(LockInfo[i].lok_syu == 0){
			if( FLAPDT.flp_data[i].nstat.bits.b04 == 1 )						/* �㏸ۯ���(��ۯ���)			*/
			{																	/*								*/
				FLAPDT.flp_data[i].nstat.bits.b04 = 0;							/* �㏸ۯ�����(��ۯ�����)		*/
			}																	/*								*/
			if( FLAPDT.flp_data[i].nstat.bits.b05 == 1 )						/* ���~ۯ���(�Jۯ���)			*/
			{																	/*								*/
				FLAPDT.flp_data[i].nstat.bits.b05 = 0;							/* ���~ۯ�����(�Jۯ�����)		*/
			}
			FLAPDT.flp_data[i].nstat.bits.b12 = 0;		// �㏸�ُ��
			FLAPDT.flp_data[i].nstat.bits.b13 = 0;		// ���~�ُ��
		}
	}
	for( i = BIKE_START_INDEX; i < LOCK_MAX; i++ ){								/* ���b�N���u */
		WACDOG;																	// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if(LockInfo[i].lok_syu == 0){
		if( FLAPDT.flp_data[i].nstat.bits.b04 == 1 )							/* �㏸ۯ���(��ۯ���)			*/
		{																		/*								*/
			FLAPDT.flp_data[i].nstat.bits.b04 = 0;								/* �㏸ۯ�����(��ۯ�����)		*/
		}																		/*								*/
		if( FLAPDT.flp_data[i].nstat.bits.b05 == 1 )							/* ���~ۯ���(�Jۯ���)			*/
		{																		/*								*/
			FLAPDT.flp_data[i].nstat.bits.b05 = 0;								/* ���~ۯ�����(�Jۯ�����)		*/
		}
		FLAPDT.flp_data[i].nstat.bits.b12 = 0;		// �㏸�ُ��
		FLAPDT.flp_data[i].nstat.bits.b13 = 0;		// ���~�ُ��
		}
	}

	SetTaskID( 0xFF );				// TaskID�̏�����

	memset( &CN_RDAT.r_dat09[0], 0x01, 4 ); /* �ނ�؂��ԉ���̈� */
	flp_DownLock_Initial();														/* ���~ۯ���ϰ�ر������		*/
	SetDiffLogReset((uchar)flg);												// �ݒ�ύX������񏉊�������
	memset( &TempFus, 0, sizeof( TempFus ));
	CountInit((uchar)flg);			// �ǂ��ԏ�����
	turi_kan_f_defset_wait = TURIKAN_DEFAULT_NOWAIT;	//�f�t�H���g�Z�b�g�҂��t���O�N���A
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
	memset( LongParkingFlag, 0, sizeof( LongParkingFlag ) );//�������ԏ�ԊǗ��t���O�N���A
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
	memset( KSG_RauBkupAPNName, 0xFF, sizeof( KSG_RauBkupAPNName ) );// APN�̐ݒ�l�̔�r�p�o�b�N�A�b�v �N���A�i�ݒ�A�h���X�̃f�[�^��0�̉\��������̂�FF�ŏ���������j
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ����莞��ʕ\��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : start_dsp                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : ̫�ĕύX�ɔ����C�� 2005-08-31 Hashimoto                 |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	start_dsp( void )
{
	unsigned char	buf[16];

	blightLevel = 0xFF;
	blightMode = 0xFF;

	// �N�����̋P�x�l�͕K���f�t�H���g�l���Z�b�g����悤�ɓ��ꂷ��B
	// �N����ʌ�̎��̉�ʂֈڍs����^�C�~���O��
	// �{���̐ݒ�l���Z�b�g�������B
	// (02-0027���ُ�l�������ꍇ�̒l�̏C���͍s��Ȃ�)
	backlight = LCD_LUMINE_DEF;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	lcd_contrast( backlight );										// �o�b�N���C�g�P�x�ݒ�
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j

	lcd_backlight( ON );											// back light ON
	dsp_background_color(COLOR_BLACK);
	as1chg( VERSNO.ver_part, buf, 8 );								// ����ް�ޮ�(����)��\���������ނ֕ϊ�����
	grachr( 1, 7, 16, 0, COLOR_WHITE, LCD_BLINK_OFF, (uchar*)buf );	// ����ް�ޮݕ\��

	grachr( 5, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[7] );	// "     ���΂炭���҂�������     "
}

/*[]----------------------------------------------------------------------[]*/
/*| ���v�ް�����(CLK_REC������)                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : ClkrecUpdate( void )                                     |*/
/*| PARAMETER   : void  :                                                  |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimo                                                |*/
/*| Date        : 2005-07-12                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	DateTimeCheck( void )
{
	if( (CLK_REC.year < 1990)||(CLK_REC.year > 2050) ){
		return( -1 );
	}
	if( (CLK_REC.mont < 1)||(CLK_REC.mont > 12) ){
		return( -1 );
	}
	if( (CLK_REC.date < 1)||(CLK_REC.date > 31) ){
		return( -1 );
	}
	if( (CLK_REC.hour > 23) ){
		return( -1 );
	}
	if( (CLK_REC.minu > 59) ){
		return( -1 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| �����W�v�����@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : auto_syuukei                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  K.Motohashi                                            |*/
/*| Date         :  2005-09-01                                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	auto_syuukei( void )
{
	uchar			syu_time_h10;	// �����W�v�����i���F�P�O�ʁj
	uchar			syu_time_h01;	// �����W�v�����i���F�P�ʁj
	uchar			syu_time_m10;	// �����W�v�����i���F�P�O�ʁj
	uchar			syu_time_m01;	// �����W�v�����i���F�P�ʁj
	ushort			syu_time;		// �����W�v�����i�ϊ��ް��j
	T_FrmSyuukei	FrmSyuukei;		// �W�v�󎚗v��ү���ލ쐬�ر

	if(( auto_syu_prn == 3 ) &&	( auto_syu_ndat != CLK_REC.ndat )){

		// �����W�v�������������t���ւ�����ꍇ
		auto_syu_prn = 0;														// �W�v��� �� �󂫁i�W�v�����҂��j
	}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( auto_cnt_prn == 2 )
//		return;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	if( ( prm_get( COM_PRM, S_TOT, 8, 1, 1 ) != 0 ) && ( auto_syu_prn == 0 ) ){

		// �����W�v������@���@�W�v�����҂��@�̏ꍇ

		syu_time_h10 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 4 );				// �����W�v�����i���F�P�O�ʁj�ݒ��ް��擾
		syu_time_h01 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 3 );				// �����W�v�����i���F�P�ʁj�@�ݒ��ް��擾
		syu_time_m10 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 2 );				// �����W�v�����i���F�P�O�ʁj�ݒ��ް��擾
		syu_time_m01 = (uchar)prm_get( COM_PRM, S_TOT, 9, 1, 1 );				// �����W�v�����i���F�P�ʁj�@�ݒ��ް��擾

		syu_time =	(syu_time_h10*600) +										// �����W�v�����ް������ݎ����Ƃ̔�r�p�ɕϊ�
					(syu_time_h01*60)  +
					(syu_time_m10*10)  +
					syu_time_m01;

		if( syu_time <= CLK_REC.nmin ){											// ������v�������͂��łɎ��s�������o�߂��Ă����ꍇ
			// �����W�v�����ɂȂ����ꍇ
			auto_syu_prn = 1;													// �W�v��� �� �W�v�J�n�v��
			auto_syu_ndat = CLK_REC.ndat;										// �W�v���t���
		}
	}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( ( auto_syu_prn == 1 ) && ( OPECTL.Mnt_mod == 0 ) && ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ) && 
//		(auto_cnt_prn != 2) ){	// ���������ʐM���{���łȂ�
	if( ( auto_syu_prn == 1 ) && ( OPECTL.Mnt_mod == 0 ) && ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ) ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		if (AppServ_IsLogFlashWriting(eLOG_TTOTAL) != 0)
			return;																// �����ݒ��Ȃ玩��T�W�v�����{���Ȃ�

		if (CountCheck() == 0){
			return;				// ���ɏW�v���Ȃ�҂�
		}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		auto_cnt_prn = 0;		// ���������ʐM���s��Ȃ��i�W�v���{��ɍs�����߁j
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		// �W�v�J�n�v���@���@����ݽ���쒆�^���Z���쒆�łȂ��ꍇ�i�����W�v���s���j

		if( prm_get( COM_PRM, S_TOT, 8, 1, 2 ) == 1 ){							// ���ɏW�v����^���Ȃ��H

			// �����W�v�ŋ��ɏW�v������ꍇ
			kinko_syu( 0, 0 );													// ��݋��ɏW�v�i�󎚂Ȃ��j
			kinko_syu( 1, 0 );													// �������ɏW�v�i�󎚂Ȃ��j
		}
		FrmSyuukei.prn_data = (void*)syuukei_prn(PREQ_AT_SYUUKEI, &sky.tsyuk);
		memcpy( &sky.tsyuk.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// ���ݎ���
		FrmSyuukei.prn_kind = R_PRI;											// ��������
// MH810105 GG119202(S) T���v�A���󎚑Ή�
		FrmSyuukei.print_flag = 0;
		if (isEC_USE()) {
			// ���σ��[�_�ڑ������T���v�󎚂̏ꍇ�͐ݒ���Q�Ƃ��ĘA���󎚂��s��
			ec_linked_total_print(PREQ_AT_SYUUKEI, &FrmSyuukei);
		}
		else {
// MH810105 GG119202(E) T���v�A���󎚑Ή�
		queset( PRNTCBNO, PREQ_AT_SYUUKEI, sizeof(T_FrmSyuukei), &FrmSyuukei );	// �����W�v�󎚗v��
// MH810105 GG119202(S) T���v�A���󎚑Ή�
		}
// MH810105 GG119202(E) T���v�A���󎚑Ή�
		wopelg( OPLOG_T_GOKEI, 0, 0 );											// ���엚��o�^�iT���v�j

		auto_syu_prn = 2;														// �W�v��� �� �W�v�󎚒�

	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �װ/�װщ����o�^                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  okuda                                                  |*/
/*| Date         :  2005-12-13                                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_ErrAlmClear( void )
{
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	/* �e�@�ָر�d�����M */
//	NTNET_Snd_Data120_CL();											// NT-NET HOST�װ�����ް����M
//	NTNET_Snd_Data121_CL();											// NT-NET HOST�װщ����ް����M
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

	/* ���M�������Ƃ�LOG�Ɏc�� */

	/* �װLOG�o�^ */
	memcpy( &Err_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
	Err_work.Errsyu = 255;											// �װ���
	Err_work.Errcod = 255;											// �װ����
// MH322915(S) K.Onodera 2017/05/22 �S�����G���[�X�e�[�^�X
//	Err_work.Errdtc = 2;											// �װ����/����
	Err_work.Errdtc = 0;											// �װ����
// MH322915(E) K.Onodera 2017/05/22 �S�����G���[�X�e�[�^�X
	Err_work.Errlev = 5;											// �װ����
	// ��������Ă��ޱ�ɂ��đ���
	Err_work.ErrDoor = 0;											// �ޱ��ԂƂ���
	Err_work.Errinf = 0;											// �װ���L��
	Log_regist( LOG_ERROR );										// �װ۸ޓo�^

	/* �װ�LOG�o�^ */
	memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
	Arm_work.Armsyu = 255;											// �װю��
	Arm_work.Armcod = 255;											// �װѺ���
// MH322915(S) K.Onodera 2017/05/22 �S�����G���[�X�e�[�^�X
//	Arm_work.Armdtc = 2;											// �װє���/����
	Arm_work.Armdtc = 0;											// �װщ���
// MH322915(E) K.Onodera 2017/05/22 �S�����G���[�X�e�[�^�X
	Arm_work.Armlev = 5;											// �װ�����
	// ��������Ă��ޱ�ɂ��đ���
	Arm_work.ArmDoor = 0;											// �ޱ��ԂƂ���
	Arm_work.Arminf = 0;											// �װя��L��
	Log_regist( LOG_ALARM );										// �װ�۸ޓo�^

// GM849100(S) ���S�����R�[���Z���^�[�Ή��@�A���[���f�[�^���G���[�X�U�����ő��M
	// AFFFF��alm_chk()�œo�^���Ă��Ȃ��̂ŁA������E9699(�S�A���[��������)�o�^����
	err_chk2(ERRMDL_ALARM, (char)99, 2, 0, 0, NULL);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��@�A���[���f�[�^���G���[�X�U�����ő��M

	IFM_Snd_ErrorClear();											// IFM�֑S�G���[�����ʒm
}

/*[]-----------------------------------------------------------------------[]*/
/*| Opetask��M���[�����N���A�i�p���j����                                   |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	RYB�M�����͏�񂾂��͍ŐV�̏�Ԃ𔽉f����B								|*/
/*|	�i����PowerON���Opetask�N���O�̂�Call�����j							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-09-27                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_MsgBoxClear( void )
{
	MsgBuf	*msb;
	ushort	wks;

	if( (msb = GetMsg( OPETCBNO )) != NULL ){					// ү���ނ���?

		wks = msb->msg.command;									// Mail-ID get
		if ((wks == IBK_NTNET_DAT_REC ) ||						// NT-NETү���ގ�M
				  (wks == IFM_RECV ) ||							// IFMү���ގ�M
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				  (wks == IBK_EDY_RCV ) ||						// Edyү���ގ�M
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				  (wks == SUICA_INITIAL_END ) ||				// Suica���������������
				  (wks == SUICA_EVT ) ||						// Suica���������������
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
				  (wks == EC_EVT_DEEMED_SETTLE_FUKUDEN) ||		// �݂Ȃ����ψ����i���d�j
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
				  (wks == IBK_NTNET_FREE_REC ) ||				// NTNET FREE�߹���ް���M
				  (wks == IBK_NTNET_BUFSTATE_CHG ) ||			// NTNET �ޯ̧��ԕω�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				  (wks == CRE_EVT_CONN_PPP ) ||					// �N���W�b�gPPP�ڑ��C�x���g
//				  (wks == IBK_EDY_ERR_REC )) {					// Edy�װү���ގ�M
				  (wks == CRE_EVT_CONN_PPP )) {					// �N���W�b�gPPP�ڑ��C�x���g
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			PutMsg( OPETCBNO, msb );							// ��Mү���ލēo�^
			return;												// ү���މ�����Ȃ�
		}
		FreeBuf( msb ); 										// ү���މ��(�ǂݎ̂Ă�)
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| RYB���͏�񏉊���                                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PowerON�� �-���͏����l�i���ݒl�j��ǂ�ł������ݽ���n�߂�O��			|*/
/*|	key_dat[] ��������쐬����B											|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : none														|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005-09-27                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Ope_InSigSts_Initial( void )
{

	//�ėp�����߰Ă������͗\�������߰Ă�ON�̏ꍇ
	if( RXM1_IN == 1 || RXI1_IN	== 1)
	{
		InSignalCtrl(INSIG_OPOPEN);		//�����c�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
		InSignalCtrl(INSIG_OPCLOSE);	//�����x�Ƃ����ʃp�����[�^�̔ėp�����߰Ă������͗\�������߰Ăɓo�^����Ă���ꍇ�̓��C���ɃC�x���g��ʒm����B
	}
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*| �d���������Z���^�[�ʐM�@�\							                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : auto_centercomm                                         |*/
///*| PARAMETER    : void                                                    |*/
///*| PARAMETER    : uchar execkind  1�F�蓮T�W�v 2�F����T�W�v 3:���������ʐM|*/
///*| RETURN VALUE : 1=���������ʐM�����s�����A0=���ĂȂ�                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :  suzuki		                                           |*/
///*| Date         :  2007-02-26                                             |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//uchar	auto_centercomm( uchar execkind )
//{
//	uchar			w_time_h10;				// �����ʐM����	�i���F�P�O�ʁj
//	uchar			w_time_h01;				// 				�i���F�P�ʁj
//	uchar			w_time_m10;				// 				�i���F�P�O�ʁj
//	uchar			w_time_m01;				// 				�i���F�P�ʁj
//	ushort			w_time;					// 				�i�ϊ��ް��j
//	uchar			w_exec_flg;				// work�i���{�̗v:1/��:0�j
//	uchar 			w_shime_kind=0;			// 0�F�����ʐM�̂� 1�F�����ʐM�����ߏ���
//
//	if( auto_cnt_prn == 2 )
//		return	0;
//
//	// �����Z���^�[�ʐM�u����v�ݒ�ŏꍇ�̓Z���^�[�ʐM�J�n�̗v�ۂ𔻕ʂ���
//		if( (auto_syu_prn == 1) || (auto_syu_prn == 2) ){						// �����W�v���{�҂�or���{��
//			auto_cnt_prn = 0;		// ���������ʐM���s��Ȃ��i�W�v���{��ɍs�����߁j
//			return	0;
//		}
//
//		if( Edy_Rec.edy_status.BIT.INITIALIZE == 0 )							// Edy�̏����ݒ肪�������Ă��Ȃ��ꍇ�͒ʐM���Ȃ�
//			return 0; 										// �N�����̾����ʐM�́A���̂P�����i���ɔ��肷��
//
//		w_exec_flg = autocnt_execchk( execkind );								// �����Z���^�[�ʐM���{�������� (�v:1/��:0�j
//		if( w_exec_flg == 2 && execkind != 3 )									// �w�莞�Ԑݒ肩�P�����i���̒ʐM�v���łȂ��ꍇ
//			return 0;
//
//		if( auto_cnt_prn == 0 ){												// ���s�\��̖�����
//																				// �����ʐM�Z���^�[�J�n���� �ݒ��ް��擾
//			w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 4 );				// �i���F�P�O�ʁj
//			w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 3 );				// �i���F�P�ʁj
//			w_time_m10 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 2 );				// �i���F�P�O�ʁj
//			w_time_m01 = (uchar)prm_get( COM_PRM, S_SCA, 58, 1, 1 );				// �i���F�P�ʁj
//			w_time =	(w_time_h10*600) +											// �����W�v�����ް������ݎ����Ƃ̔�r�p�ɕϊ�
//						(w_time_h01*60)  +
//						(w_time_m10*10)  +
//						w_time_m01;
//
//			if( execkind != 3 || ((w_time == CLK_REC.nmin) ||						// �蓮�E����T���v��̗v�����w�莞�Ԃ�
//				( ( prm_get( COM_PRM, S_SCA, 58, 1, 5 ) == 0 ) && (fg_autocnt_fstchk == 0) && (w_time < CLK_REC.nmin)) )) {			// �����オ�莞�Ɏw�莞�Ԃ��o�߂��Ă����ꍇ
//
//				fg_autocnt_fstchk = 1;												// ���グ����Edy�����Z���^�[�ʐM�v�ۊm�F�t���O�i�m�F�ρj
//
//				if(w_exec_flg == 1 || ( execkind == 3 && w_exec_flg == 2 )) {		// �Z���^�[�ʐM���{���������H
//					auto_cnt_prn = 1;												// �����Z���^�[�ʐM���s�҂�
//				}
//			}
//		}
//
//	// �����Z���^�[�ʐM�J�n�v���@���@����ݽ���쒆�^���Z���쒆�łȂ��ꍇ�i�����Z���^�[�ʐM���s���j
//		if( auto_cnt_prn == 1 ){ 													// �����ʐM���{�v������
//			if( auto_cntcom_phase_check( &execkind ) ){
//				switch( execkind ){
//					case 1:																// �蓮T�W�v��̾����ʐM����
//						wmonlg( OPMON_EDY_MT_CEN_START, 0, 0 );							// ���LOG�o�^�i�Z���^�[�ʐM�J�n�j
//						w_shime_kind = 1;												// �����ʐM�����ߏ������s
//						break;
//					case 2:																// ����T�W�v��̾����ʐM����
//						wmonlg( OPMON_EDY_AT_CEN_START, 0, 0 );							// ���LOG�o�^�i�Z���^�[�ʐM�J�n�j
//						w_shime_kind = 1;												// �����ʐM�����ߏ������s
//						break;
//					case 3:																// �w�莞�Ԃł̾����ʐM
//						wmonlg( OPMON_EDY_A_CEN_START, 0, 0 );							// ���LOG�o�^�i�Z���^�[�ʐM�J�n�j
//						execkind = 2;
//						if( w_exec_flg == 2 ){											// �w�莞�ԂŒ��ߎ��s�����H
//							w_shime_kind = 1;											// �����ʐM�����ߏ������s
//						}else{
//							w_shime_kind = 0;											// �����ʐM�̂�
//						}
//						break;
//				}
//				Edy_CentComm_sub(execkind, w_shime_kind );								// �Z���^�[�ʐM�������{�i���ߏ�������ʕ\���Ȃ��w��j
//				if( edy_cnt_ctrl.BIT.exec_status ){										// �ʐM�J�n
//					auto_cnt_prn = 2;													// �ʐM���t���O���Z�b�g
//				}else{																	// �ʐM�����{�i�G���[�������j
//					auto_cnt_prn = 0;													// �ʐM��ԃt���O���N���A
//				}
//				return	1;
//			}
//		}
//	return	0;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| �d���������Z���^�[���{�v�۔��菈��					                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : autocnt_execchk	                                       |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : void                                                    |*/
///*| RETURN VALUE : uchar w_exec_flg						  				   |*/
///*|					  0�F�ʐM���Ȃ�						  				   |*/
///*|                   1�F�ʐM���s(���߂����s���邩�ǂ����͌Ăь��Ŕ��f)	   |*/
///*|					  2�F�����ʐM�����ߎ��s(�w�莞�Ԑݒ莞��p)			   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :  suzuki		                                           |*/
///*| Date         :  2007-02-26                                             |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//uchar	autocnt_execchk( uchar type )
//{
//	uchar			w_time_h10;				// �����ʐM����	�i���F�P�O�ʁj
//	uchar			w_time_h01;				// 				�i���F�P�ʁj
//	uchar			w_time_m10;				// 				�i���F�P�O�ʁj
//	uchar			w_time_m01;				// 				�i���F�P�ʁj
//	ushort			wk_s1, wk_s2;			// work�i�����M������O�����j
//	t_NrmYMDHM		wl_Time1, wl_Time2;		// work�i�ώZ�����j
//	uchar			w_exec_flg = 0;			// work�i���{�̗v:1/��:0�j
//
//	if( prm_get( COM_PRM, S_SCA, 57, 1, 6 ) == 1 ){						// Edy���ߎ��s���ݸނ��w�莞�Ԃ̏ꍇ
//		w_exec_flg = 2;													// �����Z���^�[�ʐM���{ (�v:1�j
//	}else{																// Edy���ߎ��s���ݸނ�T���v���̏ꍇ
//		if( type != 3 ){													// �����ʐM���s��ʂ��������s�łȂ��ꍇ
//			w_exec_flg = 1;													// �����ʐM&���߂����s
//			return w_exec_flg;
//		}
//		// ���{�������u�����MLOG�����I�[�o�[�v�̏ꍇ
//		if( prm_get( COM_PRM, S_SCA, 57, 1, 5 ) == 1 ){
//			if(Edy_Rec.edy_status.BIT.INITIALIZE == 1) {					// EdyӼޭ�ق̏��������������Ă��邩�H
//				wk_s1 = (ushort)bcdbin2(OpeEmStatusInfo.NotSnd_Log_Num);	// ���ݖ����M������O����
//				w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 4 );	// ���E�����M������O�����ݒ�l�i1000�ʁj
//				w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 3 );	// �i100�ʁj
//				w_time_m10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 2 );	// �i10�ʁj
//				w_time_m01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 1 );	// �i1�ʁj
//				wk_s2 =	(w_time_h10*1000) +	(w_time_h01*100) + (w_time_m10*10) + w_time_m01;
//				if(wk_s1 >= wk_s2) {
//					w_exec_flg = 1;											// �����Z���^�[�ʐM���{ (�v:1�j
//				}
//			}
//		}
//
//		// ���{�������u�O��ʐM����̌o�ߎ��ԃI�[�o�[�v�̏ꍇ
//		else if( prm_get( COM_PRM, S_SCA, 57, 1, 5 ) == 2 ){
//			wl_Time1.ul = Nrm_YMDHM((date_time_rec *)&CLK_REC);				// wl_Time1 = ���ݎ���
//			w_time_h10 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 2 );		// �o�ߊ����ݒ�����l�i10�ʁj
//			w_time_h01 = (uchar)prm_get( COM_PRM, S_SCA, 57, 1, 1 );		// 					 �i 1�ʁj
//			if(	auto_cnt_ndat > wl_Time1.ul){ 								// ���Z�@�����X�V���ɂ�閵����
//				w_exec_flg = 1;												// �����Z���^�[�ʐM���{ (�v:1�j
//			}
//			else{
//				wl_Time2.ul = auto_cnt_ndat;								// �O��Z���^�[�ʐM����get
//				wl_Time2.us[0] += ((w_time_h10*10) + w_time_h01);			// wl_Time2 = ����ʐM�J�n�\�莞��
//
//				if( wl_Time1.ul >= wl_Time2.ul ){							// �\�莞�Ԃ��߂��Ă���
//					w_exec_flg = 1;											// �����Z���^�[�ʐM���{ (�v:1�j
//				}
//			}
//		}
//	}
//	return(w_exec_flg);
//}
//#endif
////[]----------------------------------------------------------------------[]
/////	@brief			Edy�Z���^�[�ʐM���{�\�`�F�b�N
/////	@param[in]		execkind	1�F�蓮T�W�v<br>
/////								2�F����T�W�v<br>
/////								3�F���������ʐM
/////	@return			0�F���s�s��<br>
/////					1�F���s��<br>
/////	@author			Namioka
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2008/12/03<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//static char auto_cntcom_phase_check(uchar *execkind )
//{
//	char ret = 0;
//	if( *execkind == 1 || 															// �蓮T���v��̒ʐM�v������
//	  ( ( *execkind == 2 || *execkind == 3 ) && 									// ����T���v��̒ʐM�v�����͎w�莞�Ԃł̒ʐM�v����
//	  	(OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100)))){	// �ҋ@�܂��͋x�ƒ��̏ꍇ
//	  	ret = 1;																	// ���s��
//	  	if( *execkind != 1 && edy_cnt_ctrl.BIT.wait_kind ){											// �O����s�t�F�[�Y�O�ŕ����i�҂����Ă���ꍇ
//	 		*execkind = edy_cnt_ctrl.BIT.wait_kind;									// ���s��ʂ̍X�V
//	 	}else{
//	 		edy_cnt_ctrl.BIT.wait_kind = 0;											// ���s��ʂ̃N���A
//		}	 		
//	}else{																			// ���s�t�F�[�Y�O�Ȃ̂ŁA���񕪕��i���ɍă`�F�b�N
//		if( !edy_cnt_ctrl.BIT.wait_kind ){											// ���łɎ��s�\�񂪂���ꍇ�́A�㏑�����Ȃ�
//			edy_cnt_ctrl.BIT.wait_kind = *execkind;									// ���s�t�F�[�Y�̕ۑ�
//		}
//	}
//	 return ret;
//
//}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//[]----------------------------------------------------------------------[]
///	@brief			�����W�v����
///	@param[in]		LogKind		LOG_SCASYUUKEI(22):Suica���p�W�v
///								LOG_EDYSYUUKEI(21):Edy���p�W�v
///	@return			void
///	@author			Ise
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void ck_syuukei_ptr_zai( ushort LogKind )
{
	char	chk = 0;				/* ���񗧂��グ�������f�p�t���O */
	uchar	hour;					/* ���ʃp�����[�^�̓����W�v�ݒ莞���擾�p(��) */
	uchar	minu;					/* ���ʃp�����[�^�̓����W�v�ݒ莞���擾�p(��) */
	uchar	ptr_cnt[2];				/* �╜�d���̃f�[�^�ۏ�ϐ�(0:���C�g�|�C���^/1:�W�v����) */
	ulong	old_date;				/* �O��ʒm������� */
	ulong	now_date;				/* ����ʒm������� */
	ulong	sy1_date;				/* �O��ʒm���̏W�v������񂻂̂P */
	ulong	sy2_date;				/* �O��ʒm���̏W�v������񂻂̂Q */
	ulong	sy3_date;				/* ����ʒm���̏W�v������� */
	syuukei_info	*p_syu_info;	/* ��������W�v���\���̂̃A�h���X */
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	syuukei_info2	*p_syu_info2 = 0;	/* ��������W�v���\���̂̃A�h���X */
// MH321800(E) hosoda IC�N���W�b�g�Ή�
	date_time_rec	wrk_date;		/* ���̏W�v���ԓ���p�\���� */

	// �W�v������Ă���N�����ƌ��N�������Ⴄ�ꍇ�͎��������t�ؑւɂȂ��Ă��Ȃ��Ă�
	// �����W�v�̊i�[�|�C���^��ύX����B

	/* �L���ȏW�v�Ώۋ@�\�𔻒f���Ĉȉ��̏����ɔ��f���� */
	if(LogKind == LOG_SCASYUUKEI)							/* Suica */
	{
		hour = (uchar)prm_get(COM_PRM, S_SCA, 7, 2, 3);		/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
		minu = (uchar)prm_get(COM_PRM, S_SCA, 7, 2, 1);		/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
		p_syu_info = &Syuukei_sp.sca_inf;					/* �W�v����\���̃A�h���X���擾 */
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	else if(LogKind == LOG_EDYSYUUKEI)						/* Edy */
//	{
//		hour = (uchar)prm_get(COM_PRM, S_SCA, 59, 2, 3);	/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
//		minu = (uchar)prm_get(COM_PRM, S_SCA, 59, 2, 1);	/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
//		p_syu_info = &Syuukei_sp.edy_inf;					/* �W�v����\���̃A�h���X���擾 */
//	}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	else if(LogKind == LOG_ECSYUUKEI)						/* ���σ��[�_ */
	{
		hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);		/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
		minu = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);		/* ���ʃp�����[�^����W�v���Ԑݒ���w���x�擾 */
		p_syu_info = 0;										/* �W�v����\���̃A�h���X�Ȃ� */
		p_syu_info2 = &Syuukei_sp.ec_inf;					/* �W�v����\���̃A�h���X���擾 */
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
	else													/* ��L�ȊO(�ǂ̏W�v�Ώۋ@�\��������) */
	{
		p_syu_info = 0;										/* �W�v����\���̃A�h���X�Ȃ� */
	}

	if(p_syu_info)														/* ��L���f�Ő���ɃA�h���X���擾 */
	{
		// ����̒ʒm�����𔽉f
		memcpy(&p_syu_info->NowTime, &CLK_REC, sizeof(date_time_rec));
		now_date = Nrm_YMDHM(&p_syu_info->NowTime);						/* ����ʒm���������m�[�}���C�Y�Ŏ擾 */
		old_date = Nrm_YMDHM(&p_syu_info->OldTime);						/* �O��ʒm���������m�[�}���C�Y�Ŏ擾 */
		/* �W�v�̓��t�ɌÂ�(�O��ʒm�������)�����擾 */
		memcpy(&wrk_date, &p_syu_info->OldTime, sizeof(date_time_rec));
		wrk_date.Hour = hour;											/* �W�v����K�p */
		wrk_date.Min = minu;											/* �W�v����K�p */
		sy1_date = Nrm_YMDHM(&wrk_date);								/* �O��ʒm���������m�[�}���C�Y�Ŏ擾 */
		sy2_date = sy1_date + 0x10000;									/* �擾�������������P���i�߂� */
		sy3_date = sy1_date - 0x10000;									/* �擾�������������P���߂� */

		//����H�i�d�������グ�� or �f�[�^�N���A���j
		if(p_syu_info->cnt == 0)
		{
			ptr_cnt[0] = 0;												// �W�v������������
			ptr_cnt[1] = 1;												// �i�[�|�C���^��������
			chk = 1;													// �N��or�N���A�t���O�L��
			/* �N���A�v���̃��W���[���������ɂȂ��Ă��� */
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( ((LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//				((LogKind == LOG_EDYSYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			if( (LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			{
				// �X�V���������Ǘ��\���̂֔��f
				nmisave(&p_syu_info->ptr, ptr_cnt, 2);
				// ���W�v�i�����j�G���A�̃N���A����
				memset( &p_syu_info->now, 0, sizeof(syu_dat) );
				memset( &p_syu_info->bun[p_syu_info->ptr], 0, sizeof(syu_dat) );
				// ����̒ʒm������ێ�
				memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
				// �����I��
				return;
			}
		}
		// ����ȊO�H�i�����ω��ʒm�j
		else
		{
			ptr_cnt[0] = p_syu_info->ptr;								// ���݂̏W�v�������擾
			ptr_cnt[1] = p_syu_info->cnt;								// ���ݏW�v�̊i�[�|�C���^���擾
			chk = 0;													// �N��or�N���A�t���O����
			/* �w��̃��W���[�����L���ɂȂ��Ă��� */
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( ((LogKind == LOG_SCASYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//				((LogKind == LOG_EDYSYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			if( (LogKind == LOG_SCASYUUKEI) && (prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			{
				/* ���݂̓�����񂪃N���A����Ă���(���W���[���L���ŏ��̎����ʒm) */
				if((!p_syu_info->now.SyuTime.Mon) && (!p_syu_info->now.SyuTime.Day))
				{
					/* �O��ʒm���ꂽ���Ԃ��W�v�J�n���ԂƂ��ēK�p */
					memcpy( &p_syu_info->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
					// ����̒ʒm������ێ�
					memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
					// �����I��
					return;
				}
			}
			/* �w��̃��W���[���������ɂȂ��Ă��� */
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			else if(((LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3))) ||
//					((LogKind == LOG_EDYSYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 4))))
			else if( (LogKind == LOG_SCASYUUKEI) && (!prm_get(COM_PRM, S_PAY, 24, 1, 3)) )
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			{
				// ����̒ʒm������ێ�
				memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
				// �����I��
				return;
			}
		}
		/* �O��̍���ʒm�ԂɏW�v���Ԃ��܂����ł���@�܂��́@�d���N��or�N���A�t���O�L�� */
		if( ((now_date >= sy1_date) && (sy1_date > old_date)) ||		// ����ʒm�����@���@�O��ʒm���̏W�v�����@���@�O��ʒm����
			((now_date >= sy2_date) && (sy2_date > old_date)) ||		// ����ʒm�����@���@����ʒm���̏W�v�����@���@�O��ʒm����
			((now_date <= sy1_date) && (sy1_date < old_date)) ||		// ����ʒm�����@���@�O��ʒm���̏W�v�����@���@�O��ʒm����
			((now_date <= sy3_date) && (sy3_date < old_date)) ||		// ����ʒm�����@���@����ʒm���̏W�v�����{�P���@���@�O��ʒm����
			(chk))														// ����N��or�N���A�t���O�L��
		{
			//����i�d�������グ�� or �f�[�^�N���A���j�ȊO�œ��t�X�V����H
			if( chk == 0 ) {
				// �������܂ł̏W�v����ۑ�
				memcpy( &p_syu_info->bun[ptr_cnt[0]], &p_syu_info->now, sizeof(syu_dat) );
				//�|�C���^�̍X�V
				if( ptr_cnt[0] < (SYUUKEI_DAY-1) ) {					// SYUUKEI_DAY = 101
					ptr_cnt[0]++;
				} else {
					ptr_cnt[0] = 0;										// �e�[�u���I�[��0(�ŏ�)�ɖ߂�
				}
				//�J�E���g�̍X�V
				if( ptr_cnt[1] < SYUUKEI_DAY ) {						// SYUUKEI_DAY = 101
					ptr_cnt[1]++;
				}
			}
			// �X�V���������Ǘ��\���̂֔��f
			nmisave(&p_syu_info->ptr, ptr_cnt, 2);
			//���W�v�i�����j�G���A�̃N���A����
			memset( &p_syu_info->now, 0, sizeof(syu_dat) );
			memset( &p_syu_info->bun[p_syu_info->ptr], 0, sizeof(syu_dat) );
			// ���ɏW�v����N�������Z�b�g
			memcpy( &p_syu_info->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
		}
		// ����̒ʒm������ێ�
		memcpy( &p_syu_info->OldTime, &p_syu_info->NowTime, sizeof(date_time_rec) );
	}
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	if(p_syu_info2)														/* ��L���f�Ő���ɃA�h���X���擾 */
	{
		// ����̒ʒm�����𔽉f
		memcpy(&p_syu_info2->NowTime, &CLK_REC, sizeof(date_time_rec));
		now_date = Nrm_YMDHM(&p_syu_info2->NowTime);					/* ����ʒm���������m�[�}���C�Y�Ŏ擾 */
		old_date = Nrm_YMDHM(&p_syu_info2->OldTime);					/* �O��ʒm���������m�[�}���C�Y�Ŏ擾 */
		/* �W�v�̓��t�ɌÂ�(�O��ʒm�������)�����擾 */
		memcpy(&wrk_date, &p_syu_info2->OldTime, sizeof(date_time_rec));
		wrk_date.Hour = hour;											/* �W�v����K�p */
		wrk_date.Min = minu;											/* �W�v����K�p */
		sy1_date = Nrm_YMDHM(&wrk_date);								/* �O��ʒm���������m�[�}���C�Y�Ŏ擾 */
		sy2_date = sy1_date + 0x10000;									/* �擾�������������P���i�߂� */
		sy3_date = sy1_date - 0x10000;									/* �擾�������������P���߂� */

		//����H�i�d�������グ�� or �f�[�^�N���A���j
		if(p_syu_info2->cnt == 0)
		{
			ptr_cnt[0] = 0;												// �i�[�|�C���^��������
			ptr_cnt[1] = 1;												// �W�v������������
			chk = 1;													// �N��or�N���A�t���O�L��
			/* �N���A�v���̃��W���[���������ɂȂ��Ă��� */
			if((LogKind == LOG_ECSYUUKEI) && (!isEC_USE()))
			{
				// �X�V���������Ǘ��\���̂֔��f
				nmisave(&p_syu_info2->ptr, ptr_cnt, 2);
				// ���W�v�i�����j�G���A�̃N���A����
				memset( &p_syu_info2->next, 0, sizeof(syu_dat2) );
				memset( &p_syu_info2->now, 0, sizeof(syu_dat2) );
				memset( &p_syu_info2->bun[p_syu_info2->ptr], 0, sizeof(syu_dat2) );
				// ����̒ʒm������ێ�
				memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
				// �����I��
				return;
			}
		}
		// ����ȊO�H�i�����ω��ʒm�j
		else
		{
			ptr_cnt[0] = p_syu_info2->ptr;								// ���ݏW�v�̊i�[�|�C���^���擾
			ptr_cnt[1] = p_syu_info2->cnt;								// ���݂̏W�v�������擾
			chk = 0;													// �N��or�N���A�t���O����
			/* �w��̃��W���[�����L���ɂȂ��Ă��� */
			if((LogKind == LOG_ECSYUUKEI) && (isEC_USE()))
			{
				/* ���݂̓�����񂪃N���A����Ă���(���W���[���L���ŏ��̎����ʒm) */
				if((!p_syu_info2->now.SyuTime.Mon) && (!p_syu_info2->now.SyuTime.Day))
				{
					/* �O��ʒm���ꂽ���Ԃ��W�v�J�n���ԂƂ��ēK�p */
					memcpy( &p_syu_info2->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
					// ����̒ʒm������ێ�
					memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
					// �����I��
					return;
				}
			}
			/* �w��̃��W���[���������ɂȂ��Ă��� */
			else if((LogKind == LOG_ECSYUUKEI) && (!isEC_USE()))
			{
				// ����̒ʒm������ێ�
				memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
				// �����I��
				return;
			}
		}
		/* �O��̍���ʒm�ԂɏW�v���Ԃ��܂����ł���@�܂��́@�d���N��or�N���A�t���O�L�� */
		if( ((now_date >= sy1_date) && (sy1_date > old_date)) ||		// ����ʒm�����@���@�O��ʒm���̏W�v�����@���@�O��ʒm����
			((now_date >= sy2_date) && (sy2_date > old_date)) ||		// ����ʒm�����@���@����ʒm���̏W�v�����@���@�O��ʒm����
			((now_date <= sy1_date) && (sy1_date < old_date)) ||		// ����ʒm�����@���@�O��ʒm���̏W�v�����@���@�O��ʒm����
			((now_date <= sy3_date) && (sy3_date < old_date)) ||		// ����ʒm�����@���@����ʒm���̏W�v�����{�P���@���@�O��ʒm����
			(chk))														// ����N��or�N���A�t���O�L��
		{
			//����i�d�������グ�� or �f�[�^�N���A���j�ȊO�œ��t�X�V����H
			if( chk == 0 ) {
				// �������܂ł̏W�v����ۑ�
				memcpy( &p_syu_info2->bun[ptr_cnt[0]], &p_syu_info2->now, sizeof(syu_dat2) );
				//�|�C���^�̍X�V
				if( ptr_cnt[0] < (SYUUKEI_DAY_EC-1) ) {
					ptr_cnt[0]++;
				} else {
					ptr_cnt[0] = 0;										// �e�[�u���I�[��0(�ŏ�)�ɖ߂�
				}
				//�J�E���g�̍X�V
				if( ptr_cnt[1] < SYUUKEI_DAY_EC ) {
					ptr_cnt[1]++;
				}
			}
			// �X�V���������Ǘ��\���̂֔��f
			nmisave(&p_syu_info2->ptr, ptr_cnt, 2);
			//���W�v�i�����j�G���A�̃N���A����
			memcpy( &p_syu_info2->now, &p_syu_info2->next, sizeof(syu_dat2) );		// �������Ƃ��ďW�v���ꂽ�f�[�^�𓖓��Ɉړ�
			memset( &p_syu_info2->next, 0, sizeof(syu_dat2) );						// �������Ƃ��ďW�v���ꂽ�G���A���N���A
			memset( &p_syu_info2->bun[p_syu_info2->ptr], 0, sizeof(syu_dat2) );
			// ���ɏW�v����N�������Z�b�g
			memcpy( &p_syu_info2->now.SyuTime, &CLK_REC, sizeof(date_time_rec) );
		}
		// ����̒ʒm������ێ�
		memcpy( &p_syu_info2->OldTime, &p_syu_info2->NowTime, sizeof(date_time_rec) );
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�

	return;
}

/*[]----------------------------------------------------------------------[]*
 *| MAF�ʐM�p���ʃp�����[�^�X�V											  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : MAFParamUpdate										  |
 *| PARAMETER    : �����āFseg	���ڽ�Fadd�@�ް��Fdata          		  |
  *| RETURN VALUE : ����F1	�ُ�F-1									  |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	MAFParamUpdate(ushort seg, ushort add, long data)
{
	char	ret=(char)-1;

	/* ��M�ް��m�F */
	if ((seg > 0 && seg < C_PRM_SESCNT_MAX) && (add > 0 && add <= CPrmCnt[seg])){	/* ������(�����) && �J�n���ڽ */
		/* �ް��X�V */
		CPrmSS[seg][add] = (long)data;

		DataSumUpdate(OPE_DTNUM_COMPARA);			/* update parameter sum on ram */
												// �����Ұ��ͺ�߰���Ȃ��iAppServ_PParam_Copy = OFF�j
		FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		/* FlashROM update */
		// �p�X���[�h�f�[�^�ł�mc10�͕K�v�Ȃ����A�{�΍��O�̉^�p����@���l�����A�X�V�����͎c��
		f_NTNET_RCV_MC10_EXEC = 1;				/* mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ���set */
		SetSetDiff(SETDIFFLOG_SYU_REMSET);		// �p�X���[�h�ύX���o�^����
		ret = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �N������LD1�_�ŏ���                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : start_up_LD1                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2010-03-10                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void	start_up_LD1( void )
{
	ExIOSignalwt(EXPORT_M_LD1, 1 );
	if(f_wave_datchk != WAVE_AREA_NODATA) {// �����f�[�^�����݂���ꍇ
		ExIOSignalwt(EXPORT_M_LD2, 1 );
	}
	wait2us( 200000L ); 						// wait 400ms
	ExIOSignalwt(EXPORT_M_LD1, 0 );
	ExIOSignalwt(EXPORT_M_LD2, 0 );
	wait2us( 150000L ); 						// wait 300ms
	if(f_wave_datchk != WAVE_AREA_NODATA) {// �����f�[�^�����݂���ꍇ
		ExIOSignalwt(EXPORT_M_LD2, 1 );
	}
	ExIOSignalwt(EXPORT_M_LD1, 1 );
	wait2us( 200000L ); 						// wait 400ms
	ExIOSignalwt(EXPORT_M_LD1, 0 );
	ExIOSignalwt(EXPORT_M_LD2, 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| �v�����^�̃X�e�[�^�X�擾����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Printer_Status_Get	                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static uchar Printer_Status_Get( uchar Type, uchar *NEW, uchar *OLD )
{
	PRN_DATA_BUFF	*p;							// ������ް��ޯ̧�߲���
	
	if( Type == R_PRI ){
		p = &rct_prn_buff;
	}else{
		p = &jnl_prn_buff;
	}

	if( p->PrnStateRcvCnt > 1 ){
		*NEW = p->PrnState[1];
		*OLD = p->PrnState[2];
		p->PrnStateRcvCnt = 1;				// �c��̃`�F�b�N�񐔂��P��Ƃ���
	}
	else if(p->PrnStateRcvCnt == 0) {		// �v�����^�C�x���g��M�Ȃ�
		return 0xff;
	}else{
		*NEW = p->PrnState[0];
		*OLD = p->PrnState[1];
		p->PrnStateRcvCnt--;				// ��M�ςݕ������Z
	}
	return p->PrnStateRcvCnt;

}

/*[]----------------------------------------------------------------------[]*/
/*| �v�����^�G���[�o�^����		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : I2C_Err_Regist                                          |*/
/*| PARAMETER    : msg �F�C�x���g���b�Z�[�W ErrData�F�G���[�f�[�^          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka						                           |*/
/*| Date         : 2012-03-6	                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void I2C_Err_Regist( unsigned short msg, unsigned char ErrData )
{
	
	short	err_code;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	char	err_mdl;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	
	if( msg == I2C_RP_ERR_EVT ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		err_mdl = ERRMDL_PRINTER;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		err_code = ERR_PRNT_PARITY_ERR_R;
	}else{
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (isEJA_USE()) {
			err_mdl = ERRMDL_EJA;
			err_code = ERR_EJA_PARITY_ERR;
		}
		else {
			err_mdl = ERRMDL_PRINTER;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		err_code = ERR_PRNT_PARITY_ERR_J;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//	if( ErrData & BIT_PARITY_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, err_code, 2, 0, 0 );
//	}
//	if( ErrData & BIT_FRAMING_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, (err_code+1), 2, 0, 0 );
//	}
//	if( ErrData & BIT_OVERRUN_ERR_LSR ){
//		err_chk( ERRMDL_PRINTER, (err_code+2), 2, 0, 0 );
//	}
	if( ErrData & BIT_PARITY_ERR_LSR ){
		err_chk( err_mdl, err_code, 2, 0, 0 );
	}
	if( ErrData & BIT_FRAMING_ERR_LSR ){
		err_chk( err_mdl, (err_code+1), 2, 0, 0 );
	}
	if( ErrData & BIT_OVERRUN_ERR_LSR ){
		err_chk( err_mdl, (err_code+2), 2, 0, 0 );
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
}
/*[]----------------------------------------------------------------------[]*/
/*|	�L�[�������̃��s�[�g�^�C�}�֐�										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi                                             |*/
/*| Date         : 2012/05/14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void Ope_KeyRepeat()
{
	uchar on_off = 1;
	
	if(OpeKeyRepeat.key != 0) {
		queset(OPETCBNO, OpeKeyRepeat.key, 1, &on_off);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �L�[���s�[�g�����L����������	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mnt_mode	�����e�i���X���[�h                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi                                             |*/
/*| Date         : 2012/05/14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void Ope_KeyRepeatEnable(char mnt_mode)
{
	if(mnt_mode != 0) {
// MH810100(S) S.Takahashi 2020/02/13 #3841 �u���v�u���v�𓯎��Ƀ^�b�`�A�����ɗ����ƃJ�[�\���ړ������܂܂ƂȂ�
//		OpeKeyRepeat.enable = 1;				// �L�[���s�[�g�L��
// MH810100(E) S.Takahashi 2020/02/13 #3841 �u���v�u���v�𓯎��Ƀ^�b�`�A�����ɗ����ƃJ�[�\���ړ������܂܂ƂȂ�
	}
	else {
		LagCan20ms(LAG20_KEY_REPEAT);			// ���s�[�g�^�C�}�폜
		OpeKeyRepeat.enable = 0;				// �L�[���s�[�g����
		OpeKeyRepeat.key = 0;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �����C���X�g�[���v���O����		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2012/06/05                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
//static	void	SoundInsProgram( void )
void	SoundInsProgram( void )
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
{
	ushort	i;
	uchar	BcdTime[11];
	ushort	chk_sum = 0;
	ushort	cmp_sum = 0;
	ulong	read_Addr = 0;
	ulong	sect_cnt;
	ulong	data_cnt = 0;
	ulong	buf_cnt;
	ulong	data_len= 0;
	
	f_SoundIns_OK = 0;
	// �O�ׁ̈A�������[�N���A�͎��{����
	WACDOG;														// Watch dog timer reset

	for( i = 0; i < eLOG_MAX; i++ ){
		memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
	}
	log_init();
	prm_clr( COM_PRM, 1, 0 );									// �������Ұ���̫��
	prm_clr( PEC_PRM, 1, 0 );									// �����Ұ���̫��
	RAMCLR = memclr( 1 );										// SRAM�������i�������ر�j
	Ether_DTinit();
	_ei();															// enable interrupts
	Flash2Init();
	Flash1Init();
	/* �v���󂯕t���������� */
	FLT_req_tbl_clear();
	// INIT����������Ă����Ȃ��ƁA�G���[�o�^�����s�����ꍇ�ɕs��l�ɂ�郁�����A�N�Z�X����������̂ŁA
	// ���͎g�p���Ȃ����ꉞ����Ă���
	NTNET_Init((uchar)RAMCLR);										// NT-NET�ر�Ƽ��
	FLAGPT.nmi_flg = 0;												// NMI�׸޸ر

	// �^�C�}�[�֘A�̏��������{
	Lag10msInit();
	Lag20msInit();
	Lag500msInit();
	TPU1_init();													// TPU1 Initial
	TPU2_init();													// TPU2 Initial
	TPU3_init();													// TPU3 Initial
	WACDOG;															// Watch dog timer reset
	
	// �������[�W�����p�[PIN���O���ċN������炵���̂ŁA���v�͋[���I��2012/01/01 0:00���Z�b�g����
	BcdTime[0] = 0x20;
	BcdTime[1] = 0x12;
	BcdTime[2] = 0x01;
	BcdTime[3] = 0x01;
	BcdTime[4] = 0x00;
	BcdTime[5] = 0x00;
	BcdTime[6] = 0x00;
	BcdTime[7] = 0x00;
	BcdTime[8] = 0x00;
	BcdTime[9] = 0x00;
	BcdTime[10] = 0x00;
	
	ClkrecUpdate(BcdTime);
	BootInfo.sw.wave = OPERATE_AREA2;							// �����f�[�^�̃C���X�g�[���́A�ʂP�ɂ���ׁA�ꎞ�I�ɉ^�p�ʂ�ʂQ�Ƃ���
	SysMnt_FTPServer();
	_flt_DisableTaskChg();
	FLT_read_wave_sum_version(0,swdata_write_buf);// �����f�[�^��1�̏������[�h
	// �����O�X�̎擾
	data_len = (ulong)(swdata_write_buf[LENGTH_OFFSET]<<24);
	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+1]<<16);
	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+2]<<8);
	data_len += (ulong)swdata_write_buf[LENGTH_OFFSET+3];
	// �`�F�b�N�T���̎擾
	cmp_sum = (ushort)(swdata_write_buf[SUM_OFFSET]<<8);
	cmp_sum += (ushort)swdata_write_buf[SUM_OFFSET+1];
	// FROM�����1�̉����f�[�^��ǂݏo���T�����Z�o����
	read_Addr = FLT_SOUND0_SECTOR;
	for(sect_cnt = 0; sect_cnt < FLT_SOUND0_SECTORMAX; sect_cnt++,read_Addr += FLT_SOUND0_SECT_SIZE) {
		FlashReadData_direct(read_Addr, &FLASH_WRITE_BUFFER[0], FLT_SOUND0_SECT_SIZE);// 1�Z�N�^�������[�h
		for (buf_cnt = 0; buf_cnt < FLT_SOUND0_SECT_SIZE; buf_cnt++) {
			chk_sum += FLASH_WRITE_BUFFER[buf_cnt];
			data_cnt++;
			if(data_len <= data_cnt){
				goto smchk_loop_end;
			}
		}
	}
smchk_loop_end:
	if(cmp_sum != chk_sum){// �_�E�����[�h�t�@�C���̃`�F�b�N�T���Ƃ�����x��r���A����Ă����ꍇ�͎��s
		f_SoundIns_OK = 0;
	}
	BootInfo.sw.wave = OPERATE_AREA1;							// �����f�[�^�̃C���X�g�[����́A�ʂP���^�p�ʂɂ���
	FLT_write_wave_swdata(OPERATE_AREA1);						// FROM�ɂ��^�p�ʂ���������
	// �ʐM���I���΁Aؾ�Ă����܂ŉ������Ȃ�
	while(1){
		taskchg( IDLETSKNO );
		ExIOSignalwt(EXPORT_M_LD2, 1 );// LD2�_���ŏI��(������)
		if(f_SoundIns_OK == 0){// ���s
			wait2us( 150000L ); // wait 300ms
			ExIOSignalwt(EXPORT_M_LD2, 0 );// LD2�_��
			wait2us( 150000L ); // wait 300ms
		}
	}

}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//[]----------------------------------------------------------------------[]
///	@brief		LCD�Ƃ̒ʐM�ݒ�f�t�H���g
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static	void	lcdbm_LcdIpPort_DefaultSetting( void )
{
	int		i;				// index

	for( i=0; 0xff != cprm_rec[i].ses; i++) {
		// �W���ʐM���W���[���ݒ�
		if( cprm_rec[i].ses == 41 ){
			// �ʐM�K�i�C�z�X�gIP�A�h���X�C�T�u�l�b�g�}�X�N�C�f�t�H���g�Q�[�g�E�F�C
			if( cprm_rec[i].adr >= 1 && cprm_rec[i].adr <= 7 ){
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr] = cprm_rec[i].dat;
			}
		}
		// LCD�ʐM�ݒ�
		else if( cprm_rec[i].ses == 48 ){
			// LCD�p�l��IP�A�h���X�@�`�C�C�|�[�g�ԍ�
			if( cprm_rec[i].adr >= 1 && cprm_rec[i].adr <= 7 ){
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr] = cprm_rec[i].dat;
			}
		}
	}
	// update parameter sum on ram
	DataSumUpdate( OPE_DTNUM_COMPARA );
}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
///*[]----------------------------------------------------------------------[]*/
///*| �����e�X�g�Đ��v���O����		                                       |*/
///*[]----------------------------------------------------------------------[]*/
///*| PARAMETER    : void							                           |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : T.Namioka	                                           |*/
///*| Date         : 2012/06/05                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
//static	void	SoundPlayProgram( void )
//{
//	ushort	msg = 0;
//	ushort	i;
//	for( i = 0; i < eLOG_MAX; i++ ){
//		memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
//	}
//	log_init();
//	prm_clr( COM_PRM, 1, 0 );									// �������Ұ���̫��
//	prm_clr( PEC_PRM, 1, 0 );									// �����Ұ���̫��
//	RAMCLR = memclr( 1 );										// SRAM�������i�������ر�j
//	FLT_req_tbl_clear();										// �v���󂯕t����������
//// ��P�̂Œʏ�f�t�H���g�Ŗ炷�ɂ͑傫������̂� ���ʂ��u3�v�Ƃ���
//	CPrmSS[S_SYS][51] = 30000L;// ch1����
//	CPrmSS[S_SYS][60] = 3L;// �u�U�[����
//	AVM_Sodiac_Init();
//	while( 1 ){
//		if( AVM_Sodiac_Ctrl[0].play_cmp ){
//			if( msg > ANN_MSG_MAX){
//				msg = 0;
//			}
//			avm_test_no[0] = msg++;
//			avm_test_ch = 0;
//			avm_test_cnt = 1;
//			ope_anm( AVM_AN_TEST );
//		}
//		taskchg( IDLETSKNO );
//	}
//}
// MH810100(E) K.Onodera  2020/02/21 #3895 �Ԕԃ`�P�b�g���X(�\���RSW�ŋN�����A������ʕ\��NG�ƂȂ�s��C��)
/*[]----------------------------------------------------------------------[]*/
/*| �h�A�m�u�߂��Y��^�C�}�J�n                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_StartDoorKnobTimer(void)
{
	short	doorKnobAlarmTimer;

	doorKnobAlarmTimer = OPE_DOOR_KNOB_ALARM_START_TIMER;
	if(doorKnobAlarmTimer) {
		Lagtim(OPETCBNO, 5, doorKnobAlarmTimer);	// �h�A�m�u�߂��Y��h�~�^�C�}�[�X�V
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �h�A�m�u�߂��Y��x�����o�͎��Ԑݒ�                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_StopDoorKnobTimer(void)
{
	short	doorKnobAlarmStopTimer;

	doorKnobAlarmStopTimer = OPE_DOOR_KNOB_ALARM_STOP_TIMER;
	if(doorKnobAlarmStopTimer) {
		Lagtim(OPETCBNO, 29, doorKnobAlarmStopTimer);	// �h�A�m�u�߂��Y��x�����o�͎��ԃ^�C�}�[
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �h�A�m�u�߂��Y��`���C���L��                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_EnableDoorKnobChime(void)
{
	DisableDoorKnobChime = 0;				// �h�A�m�u�߂��Y��`���C���}�~�t���OOFF(�`���C���L��)
	Ope_StartDoorKnobTimer();				// �h�A�m�u�߂��Y��^�C�}�J�n
}
/*[]----------------------------------------------------------------------[]*/
/*| �h�A�m�u�߂��Y��`���C������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	Ope_DisableDoorKnobChime(void)
{
	DisableDoorKnobChime = 1;				// �h�A�m�u�߂��Y��`���C���}�~�t���OON(�`���C������)
}
/*[]----------------------------------------------------------------------[]*/
/*| �h�A�m�u�߂��Y��`���C����Ԏ擾                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : 1 : �L��                                                |*/
/*|              : 0 : ����                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi	                                           |*/
/*| Date         : 2012/06/22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	Ope_IsEnableDoorKnobChime(void)
{
	if(DisableDoorKnobChime == 0) {
		return 1;
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �����ɂ��FAN���䔻�菈��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_fan_timectrl                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 1 : ������~                                            |*/
/*|              : 0 : �ʏ퐧��                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : A.iiizumi                                               |*/
/*| Date         : 2012-07-04                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static uchar chk_fan_timectrl( void )
{
	ulong	st_time;
	ulong	ed_time;
	ulong	now_time;
	uchar	ret = 0;

	//�ݒ莞�Ԃ����S�m�[�}���C�Y
	// 02-0063�B�C�D�E FAN������~�J�n����
	st_time = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
							   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][63]/100), (short)(CPrmSS[S_PAY][63]%100) ) );
	// 02-0064�B�C�D�E FAN������~�I������
	ed_time = time_nrmlz ( (ushort)dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ),
							   (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );

	//���ݎ��Ԃ����S�m�[�}���C�Y
	now_time = time_nrmlz ( CLK_REC.ndat, CLK_REC.nmin );

	//�ݒ莞�ԑ召�ϊ�
	if( st_time > ed_time ){ //EX>Start 20:00 , End 6:00
		if( ed_time > now_time ){//now 3:00
			//�J�n��O���ɂ���	02-0063�B�C�D�E FAN������~�J�n����
			st_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
									 (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][63]/100), (short)(CPrmSS[S_PAY][63]%100) ) );
		}else if(now_time > st_time){//now 20:00
			//�I�������̓��ɂ��� 02-0064�B�C�D�E FAN������~�I������
			ed_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) + 1),
								     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );
		}
	}else if( st_time < ed_time ){ //EX> now 1:00 , Start 2:00 , End 10:00 
		if( st_time > now_time ){
			//�I����O���ɂ��� 02-0064�B�C�D�E FAN������~�I������
			ed_time = time_nrmlz ( (ushort)(dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date ) - 1),
								     (ushort)tnrmlz ( (short)0, (short)0, (short)(CPrmSS[S_PAY][64]/100), (short)(CPrmSS[S_PAY][64]%100) ) );
		}
	}else if( st_time == ed_time ){
		return(ret); //�J�n�ƏI������v�����ꍇ�͐��䂵�Ȃ�
	}

	if(st_time <= now_time){// FAN���� ������~�J�n
		ret = 1;//������~
	}else{
		ret = 0;//�ʏ퐧��(������~����)
	}
	if(ed_time <= now_time){
		ret = 0;//�ʏ퐧��(������~����)
	}
	return ret;
}
// MH810100(S) 2020/06/16 #4231 �y�A���]���w�E�����z���O�^�C���I�[�o��ɏo�ɂ��Ă��邪�A�����Z�o�ɂ̍Đ��Z�������ɂȂ��Ă��܂�(No.02-0027)
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))_for_LCD_IN_CAR_INFO_�����v�Z
//// if (���Z���� + ׸���ю��� >= ���ݎ���) ---> op_mod230(���Z�ς݈ē�)
//// return		1:	op_mod230(���Z�ς݈ē�)�ֈڍs
//uchar Is_in_lagtim( void )
//{
//	ushort	prm_minutes = 0;
//	uchar	syu;
//	ulong	st_time;
//	ulong	now_time;
//	uchar	ret = 0;	// 0 = ׸���ѵ���(op_mod230(���Z�ς݈ē�)�ւ̈ڍs�Ȃ�)
//	ushort	Year;
//	uchar	Month, Day, Hours, Minute, Second;
//	ulong	wkul;
//	struct clk_rec PaymentTime;
//	struct clk_rec NowTime;
//
//	Year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;		// �ݎԏ��_���Z_�N	2000�`2099(��������)
//	Month	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;		// �ݎԏ��_���Z_��	1�`12(��������)
//	Day		= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;		// �ݎԏ��_���Z_��	1�`31(��������)
//	Hours	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;		// �ݎԏ��_���Z_��	0�`23(��������)
//	Minute	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;		// �ݎԏ��_���Z_��	0�`59(��������)
//	Second	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;		// �ݎԏ��_���Z_�b	0�`59(��������)
//
//	//���Z���� + ׸���ю��Ԃ����Sɰ�ײ��
//	PaymentTime.year	= Year;		// �ݎԏ��_���Z_�N	2000�`2099(��������)
//	PaymentTime.mont	= Month;	// �ݎԏ��_���Z_��	1�`12(��������)
//	PaymentTime.date	= Day;		// �ݎԏ��_���Z_��	1�`31(��������)
//	PaymentTime.hour	= Hours;	// �ݎԏ��_���Z_��	0�`23(��������)
//	PaymentTime.minu	= Minute;	// �ݎԏ��_���Z_��	0�`59(��������)
//	PaymentTime.seco	= Second;	// �ݎԏ��_���Z_�b	0�`59(��������)
//	PaymentTime.week	= 0;		// Day   (0:SUN-6:SAT)
//	PaymentTime.ndat	= 0;		// Normalize Date
//	PaymentTime.nmin	= 0;		// Normalize Minute
//	// �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget
//	st_time = c_Normalize_sec(&PaymentTime);
//
//	NowTime.year	= CLK_REC.year;
//	NowTime.mont	= CLK_REC.mont;
//	NowTime.date	= CLK_REC.date;
//	NowTime.hour	= CLK_REC.hour;
//	NowTime.minu	= CLK_REC.minu;
//	NowTime.seco	= CLK_REC.seco;
//	NowTime.week	= 0;		// Day   (0:SUN-6:SAT)
//	NowTime.ndat	= 0;		// Normalize Date
//	NowTime.nmin	= 0;		// Normalize Minute
//
//	// �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget
//	now_time = c_Normalize_sec(&NowTime);
//
//	// 29-0001	���޽��ѐ؊�	�E:���޽��ѐ؊�    0=�Ȃ�(�S�Ԏ틤��)/1=����(�Ԏ했�ɐݒ�)
//	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// �S�Ԏ틤��?
//		// 29-0004	�S�Ԏ틤��	׸����	�C�D�E:����	001�`120(��)/000=�ݒ�s�� ���Z����o�ɂ܂ł̗�������������(000010)
//		prm_minutes = (short)CPrmSS[S_STM][4];
//
//	// ��ʖ����޽���
//	}else{
//		// �Ԏ���擾
//		syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
//		if(( 1 <= syu )&&( syu <= 12 )){
//			// �Ԏ했׸����		29-0007, 29-0010,  29-0013... A�Ԏ� - L�Ԏ�
//			prm_minutes = (short)CPrmSS[S_STM][7 + (3 * (syu - 1))];
//		}
//	}
//
//	wkul = (prm_minutes * 60);		// �P�ʂ�b��
//	st_time += wkul;
//
//	// if (���Z���� + ׸���ю��� >= ���ݎ���) ---> op_mod230(���Z�ς݈ē�)
//	if (st_time >= now_time) {
//		ret = 1;				// 1 = op_mod230(���Z�ς݈ē�)�ֈڍs����
//	}
//	return ret;
//}
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))_for_LCD_IN_CAR_INFO_�����v�Z

// ���Z����  �ˁ�   �o�Ɏ���				�̏ꍇ�@�F���Z�����{���O�^�C�� >= �o�Ɏ���	---> op_mod230(���Z�ς݈ē�)
// �o�Ɏ���  �ˁ�   ���Z����				�̏ꍇ�A�F�����Z�o�ɂ̐��Z�ς݂̂͂�		---> op_mod230(���Z�ς݈ē�)
// ���Z����  �ˁ�   ���ݎ���				�̏ꍇ�B�F���Z�����{���O�^�C�� >= ���ݎ���	---> op_mod230(���Z�ς݈ē�)
uchar Is_in_lagtim( void )
{
	ushort	prm_minutes = 0;
	uchar	syu;
	ulong	Payst_time;
	ulong	Exitst_time;
	ulong	now_time;
	uchar	ret = 0;	// 0 = ׸���ѵ���(op_mod230(���Z�ς݈ē�)�ւ̈ڍs�Ȃ�)
	ulong	wkul;
	struct clk_rec PaymentTime;
	struct clk_rec ExitTime;
	struct clk_rec NowTime;

	// �o�Ɏ���
	ExitTime.year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.shYear;		// �ݎԏ��_�o��_�N	2000�`2099(��������)
	ExitTime.mont	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byMonth;		// �ݎԏ��_�o��_��	1�`12(��������)
	ExitTime.date	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byDay;		// �ݎԏ��_�o��_��	1�`31(��������)
	ExitTime.hour	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byHours;		// �ݎԏ��_�o��_��	0�`23(��������)
	ExitTime.minu	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.byMinute;	// �ݎԏ��_�o��_��	0�`59(��������)
	ExitTime.seco	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec.bySecond;	// �ݎԏ��_�o��_�b	0�`59(��������)
	ExitTime.week	= 0;		// Day   (0:SUN-6:SAT)
	ExitTime.ndat	= 0;		// Normalize Date
	ExitTime.nmin	= 0;		// Normalize Minute
	// �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget
	Exitst_time = c_Normalize_sec(&ExitTime);
	
	// ���Z����
	PaymentTime.year	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear;		// �ݎԏ��_���Z_�N	2000�`2099(��������)
	PaymentTime.mont	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth;	// �ݎԏ��_���Z_��	1�`12(��������)
	PaymentTime.date		= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay;	// �ݎԏ��_���Z_��	1�`31(��������)
	PaymentTime.hour	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours;	// �ݎԏ��_���Z_��	0�`23(��������)
	PaymentTime.minu	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute;	// �ݎԏ��_���Z_��	0�`59(��������)
	PaymentTime.seco	= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;	// �ݎԏ��_���Z_�b	0�`59(��������)
	PaymentTime.week	= 0;		// Day   (0:SUN-6:SAT)
	PaymentTime.ndat	= 0;		// Normalize Date
	PaymentTime.nmin	= 0;		// Normalize Minute
	// �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget
	Payst_time = c_Normalize_sec(&PaymentTime);

	NowTime.year	= CLK_REC.year;
	NowTime.mont	= CLK_REC.mont;
	NowTime.date	= CLK_REC.date;
	NowTime.hour	= CLK_REC.hour;
	NowTime.minu	= CLK_REC.minu;
	NowTime.seco	= CLK_REC.seco;
	NowTime.week	= 0;		// Day   (0:SUN-6:SAT)
	NowTime.ndat	= 0;		// Normalize Date
	NowTime.nmin	= 0;		// Normalize Minute
	// �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget
	now_time = c_Normalize_sec(&NowTime);

	// 29-0001	���޽��ѐ؊�	�E:���޽��ѐ؊�    0=�Ȃ�(�S�Ԏ틤��)/1=����(�Ԏ했�ɐݒ�)
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// �S�Ԏ틤��?
		// 29-0004	�S�Ԏ틤��	׸����	�C�D�E:����	001�`120(��)/000=�ݒ�s�� ���Z����o�ɂ܂ł̗�������������(000010)
		prm_minutes = (short)CPrmSS[S_STM][4];

	// ��ʖ����޽���
	}else{
		// �Ԏ���擾
		syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
		if(( 1 <= syu )&&( syu <= 12 )){
			// �Ԏ했׸����		29-0007, 29-0010,  29-0013... A�Ԏ� - L�Ԏ�
			prm_minutes = (short)CPrmSS[S_STM][7 + (3 * (syu - 1))];
		}
	}

	wkul = (prm_minutes * 60);		// �P�ʂ�b��

// ���Z����  �ˁ�   �o�Ɏ���				�̏ꍇ�@�F���Z�����{���O�^�C�� >= �o�Ɏ���	---> op_mod230(���Z�ς݈ē�)
// �o�Ɏ���  �ˁ�   ���Z����				�̏ꍇ�A�F�����Z�o�ɂ̐��Z�ς݂̂͂�		---> op_mod230(���Z�ς݈ē�)
// ���Z����  �ˁ�   ���ݎ���				�̏ꍇ�B�F���Z�����{���O�^�C�� >= ���ݎ���	---> op_mod230(���Z�ς݈ē�)
	
	// �o�Ɏ���������ꍇ
	if(ExitTime.year && ExitTime.mont && ExitTime.date){
		if( Exitst_time >= Payst_time){
			// �@�F���Z�����{���O�^�C�� >= �o�Ɏ���
			if( (Payst_time + wkul ) >= Exitst_time){
				ret = 1;				// 1 = op_mod230(���Z�ς݈ē�)�ֈڍs����
			}
		}else{
			// �A�F�����Z�o�ɂ̐��Z�ς݂̂͂�
			ret = 1;				// 1 = op_mod230(���Z�ς݈ē�)�ֈڍs����
		}
	}else{
		// �B�F���Z�����{���O�^�C�� >= ���ݎ���
		if( (Payst_time + wkul) >= now_time){
			ret = 1;				// 1 = op_mod230(���Z�ς݈ē�)�ֈڍs����
		}
	}
	return ret;
}

// MH810100(E) 2020/06/16 #4231 �y�A���]���w�E�����z���O�^�C���I�[�o��ɏo�ɂ��Ă��邪�A�����Z�o�ɂ̍Đ��Z�������ɂȂ��Ă��܂�(No.02-0027)
/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ɑ���G���[�E�A���[���̑��M�^�C�}�[���N��                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_CenterDoorTimer                                     |*/
/*| PARAMETER    : start_stop: 0=stop, 1=start                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : t.hashimoto                                             |*/
/*| Date         : 2013-02-22                                              |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void Ope_CenterDoorTimer( char start_stop )
{
	ushort	usT;

	if( !OPE_Is_DOOR_ERRARM ){				// �h�A�J���̃G���[�E�A���[�����䂠��H
		// �V�X�e�������e�ŋ@�\�Ȃ��ɕύX���ꂽ�ꍇ�̑Ώ�
		OPECTL.f_DoorSts = 0;
		return;
	}
	if( prm_get(COM_PRM, S_PAY, 24, 1, 1) == 2 ){				// ���uNT-NET
		if( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1 ){			// �ڑ���͒��ԏ�Z���^�[
			if( start_stop != 0 ){
				usT = (ushort)(prm_get(COM_PRM, S_CEN, 96, 2, 1));
				if( usT == 0 ){
					usT = 10;
				}
				Lagtim( OPETCBNO, 30,  usT * 50 );
			}else{
				Lagcan( OPETCBNO, 30 );							// ��ϰؾ��
			}
		}
	}
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
//static void log_clear_all(void)
static void log_clear_all(uchar factor)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
{
	uchar i;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
	uchar is_need_delete = FLT_check_syukei_log_delete();
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)

	// FROM�Ɋi�[���Ă��郍�O�o�[�W�����ƃv���O�����R�[�h�̃��O�o�[�W�������قȂ�ꍇ
	// �\�t�g�o�[�W�����A�b�v�Ń��O�̍\�����ύX�ƂȂ������߃��O������������
	// �܂��A�p�X���[�h�̈悪�j�󂳂�Ă����ꍇ�A���O�����������s���A�N�Z�X���Ȃ��悤�ɕی삷��
	// (�o�b�N�A�b�v�o�b�e���[�̃W�����p��OPEN�̏�ԂŋN���������A�����f�[�^�̗̈��j�󂷂�P�[�X�����邽��)
	for(i = 0;i < eLOG_MAX; i++){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
		if( 0 == factor &&								// �N���A�v�������O�o�[�W�����s��v�A����
			0 == is_need_delete &&						// �W�v���O�̊Ǘ���񂪕ύX����Ă��Ȃ�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//			(eLOG_TTOTAL == i || eLOG_LCKTTL == i) ) {	// �W�v���O�ƎԎ����W�v���O�͍폜���Ȃ�
			(eLOG_TTOTAL == i) ) {						// �W�v���O�͍폜���Ȃ�
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810104(S) 2021/10/27 �Ԕԃ`�P�b�g���X ���O�o�[�W�������s��v�̏ꍇ�A�W�v���O���N���A����Ă��܂��s��Ή�
			BR_LOG_SIZE[i] = LogDatMax[i][0];			// �����ŃT�C�Y��ێ����Ă����Ȃ���0�ŏ������܂�邽�߁AFLT_init�ō폜����Ă��܂�
// MH810104(E) 2021/10/27 �Ԕԃ`�P�b�g���X ���O�o�[�W�������s��v�̏ꍇ�A�W�v���O���N���A����Ă��܂��s��Ή�
			continue;
		}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
		FLT_LogErase2(i);
		BR_LOG_SIZE[i] = LogDatMax[i][0];
	}
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );// ���o�ɗ����̃��O�������ŃN���A����K�v������
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
	log_init();
	Log_clear_log_bakupflag();									// LOG�f�[�^�o�^���d�����p�̃t���O�A�ޔ��f�[�^�N���A����
	FLT_req_tbl_clear();										// ���O�T�C�Y��񏑂����݂̑O�ɗv�����N���A����K�v������
	FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ���O�T�C�Y��񏑂�����
	// FLT_WriteBRLOGSZ()��_flt_EnableTaskChg()���R�[������A�^�X�N�`�F���W�̔�����}�~����
	_flt_DisableTaskChg();
}
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// �A�C�h���T�u�^�X�N�ֈړ�
//-// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//-//[]----------------------------------------------------------------------[]
//-///	@brief		�������ԃ`�F�b�N
//-//[]----------------------------------------------------------------------[]
//-///	@return		
//-///	@author		A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/05/16<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//void	LongTermParkingCheck( void )
//{
//	ushort	wHour;
//	ushort	i;
//	ulong	ulwork;
//
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){					// �������Ԍ��o����
///	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
//	if(prm_wk == 0){	// �������Ԍ��o(�A���[���f�[�^�ɂ��ʒm)
//		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// ��������
//		if(wHour == 0){
//			wHour = 48;												// �ݒ肪0�̏ꍇ�͋����I��48���ԂƂ���
//		}
//		for( i = 0; i < LOCK_MAX; i++ ){
//			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
//			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// �Ԏ��L��
//				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//�u�o�ɏ������v�܂��́u��ԁv�ȊO�ŗL��
//					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����擾
//					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// �w�莞�Ԉȏ���
//						if(LongParkingFlag[i] == 0){					// �������ԏ�ԁF�������ԏ�ԂȂ�
//							LongParkingFlag[i] = 1;						// �������ԏ�Ԃ���
//							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 1, 2, 1, &ulwork);	// A0031�o�^ �������Ԍ��o
//						}
//					}else{// �w�莞�Ԗ���
//						if(LongParkingFlag[i] != 0){					// �������ԏ�ԁF�������ԏ�Ԃ���
//							// �����ύX�ɂ�蒷�����ԏ�ԁu����v���u�Ȃ��v�ɕω�����ꍇ�͉�������
//							LongParkingFlag[i] = 0;						// �������ԏ�ԂȂ�
//							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);	// A0031���� �������Ԍ��o
//						}
//					}
//				}
//			}
//		}
//	}
//
//}
//-//[]----------------------------------------------------------------------[]
//-///	@brief		�������Ԏ��ԃ`�F�b�N
//-//[]----------------------------------------------------------------------[]
//-///	@param[in]  flp_com *p   : �擪�ԍ�
//-///	@param[in]  short   hours: ���Ԏ��� 
//-///	@return     ret          : 0:�w�莞�Ԗ��� 1:�w�莞�ԓ��B
//-///	@author     A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/05/16<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
//short LongTermParking_stay_hour_check(flp_com *p, short hours)
//{
//	short	year, mon, day, hour, min;
//	short	add_day, add_hour;
//
//	year = p->year;		/* ���ɔN */
//	mon = p->mont;		/* ���Ɍ� */
//	day = p->date;		/* ���ɓ� */
//	hour = p->hour;		/* ���Ɏ� */
//	min = p->minu;		/* ���ɕ� */
//
//	// ���Ԏ���(��)����A���Ɋ��Z����
//	add_day = hours / 24;
//	add_hour = hours % 24;
//
//	// ���ɓ�������add_hour���Ԍ�̓������擾����
//	hour += add_hour;
//	if(hour >= 24){
//		day++;//1�����Z
//		hour = hour % 24;
//		for ( ; ; ) {
//			if (day <= medget(year, mon)) {// �����ȉ�
//				break;
//			}
//			// �Y�����̓����������Č������Z
//			day -= medget(year, mon);
//			if (++mon > 12) {
//				mon = 1;
//				year++;
//			}
//		}
//	}
//	// ���ɓ�������add_day����̓������擾����
//	day += add_day;
//	for ( ; ; ) {
//		if (day <= medget(year, mon)) {// �����ȉ�
//			break;
//		}
//		// �Y�����̓����������Č������Z
//		day -= medget(year, mon);
//		if (++mon > 12) {
//			mon = 1;
//			year++;
//		}
//	}
//
//	if(CLK_REC.year > year){
//		return 1;	//���ݔN���������ԔN���߂���(�w�莞�ԓ��B)
//	}
//	if(CLK_REC.year < year){
//		return 0;	//���ݔN���������ԔN���ߋ�(�w�莞�Ԗ���)
//	}
//	// �ȉ� �N�͓���
//	if(CLK_REC.mont > mon){
//		return 1;	//���݌����������Ԍ����߂���(�w�莞�ԓ��B)
//	}
//	if(CLK_REC.mont < mon){
//		return 0;	//���݌����������Ԍ����ߋ�(�w�莞�Ԗ���)
//	}
//	// �ȉ� ���͓���
//	if(CLK_REC.date > day){
//		return 1;	//���ݓ����������ԓ����߂���(�w�莞�ԓ��B)
//	}
//	if(CLK_REC.date < day){
//		return 0;	//���ݓ����������ԓ����ߋ�(�w�莞�Ԗ���)
//	}
//	// �ȉ� ���͓���
//	if(CLK_REC.hour > hour){
//		return 1;	//���ݎ����������Ԏ����߂���(�w�莞�ԓ��B)
//	}
//	if(CLK_REC.hour < hour){
//		return 0;	//���ݎ����������Ԏ����ߋ�(�w�莞�Ԗ���)
//	}
//	// �ȉ� ���͓���
//	if(CLK_REC.minu > min){
//		return 1;	//���ݕ����������ԕ����߂���(�w�莞�ԓ��B)
//	}
//	if(CLK_REC.minu < min){
//		return 0;	//���ݕ����������ԕ����ߋ�(�w�莞�Ԗ���)
//	}
//	return 1;// �N����������v(�w�莞�ԓ��B)
//}
//-//[]----------------------------------------------------------------------[]
//-///	@brief		�������ԃ`�F�b�N(�h�A���ɃZ���^�[�ɑ΂��đ��M���鏈��)
//-//[]----------------------------------------------------------------------[]
//-///	@return		
//-///	@author		A.Iiizumi
//-//[]----------------------------------------------------------------------[]
//-///	@date		Create	:	2018/07/26<br>
//-///				Update	:	
//-//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//void	LongTermParkingCheck_Resend( void )
//{
//	ushort	wHour;
//	ushort	i;
//	ulong	ulwork;
//
//	if ( prm_get(COM_PRM, S_TYP, 135, 1, 5) == 0 ){					// �������Ԍ��o����
//	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
//	if((prm_wk == 0) || (prm_wk == 2)){
//		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// ��������
//		if(wHour == 0){
//			wHour = 48;												// �ݒ肪0�̏ꍇ�͋����I��48���ԂƂ���
//		}
//
//		if( isDefToErrAlmTbl(1, ALMMDL_MAIN, ALARM_LONG_PARKING) == FALSE ){
//			// �A���[���f�[�^�̑��M���x���̐ݒ�i34-0037�B�j�ő��M���郌�x���łȂ��ꍇ�̓`�F�b�N���Ȃ�
//			return;
//		}
//		// �h�A���ɒ������ԃ`�F�b�N���s�����o���ł���΃Z���^�[�ɑ΂��čđ����邽�߂Ƀ��O�ɓo�^����
//		for( i = 0; i < LOCK_MAX; i++ ){
//			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
//			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// �Ԏ��L��
//				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//�u�o�ɏ������v�܂��́u��ԁv�ȊO�ŗL��
//					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����擾
//					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// �w�莞�Ԉȏ���
//						// A0031�o�^ �������Ԍ��o
//						memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
//						Arm_work.Armsyu = ALMMDL_MAIN;										// �װю��
//						Arm_work.Armcod = ALARM_LONG_PARKING;								// �װѺ���
//						Arm_work.Armdtc = 1;												// �װє���/����
//											
//						Arm_work.Armlev = (uchar)getAlmLevel( ALMMDL_MAIN, ALARM_LONG_PARKING );// �װ�����
//						Arm_work.ArmDoor = ERR_LOG_RESEND_F;								// �đ���ԂƂ���
//						Arm_work.Arminf = 2;												// �t���ް�(bin)����
//						Arm_work.ArmBinDat = ulwork;										// bin �A���[�������
//
//						Log_regist( LOG_ALARM );											// �װ�۸ޓo�^
//					}
//				}
//			}
//		}
//	}
//
//}
//-// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//[]----------------------------------------------------------------------[]
///	@brief			���v���킹�i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EJA_TimeAdjust(void)
{
	ushort	adj_time;

	if (!isEJA_USE()) {
		return;
	}

	adj_time = (ushort)prm_tim(COM_PRM, S_REC, 1);
	if (adj_time == CLK_REC.nmin) {
		// �����ݒ�v��
		MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
	}
}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�


