/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽ����                                                         |*/
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
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"AppServ.h"
#include	"sysmnt_def.h"
#include	"fla_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"ntnet.h"
#include	"flp_def.h"
#include	"tbl_rkn.h"
#include	"rkn_fun.h"
#include	"ntnet_def.h"
#include	<stdlib.h>
#include	"remote_dl.h"
#include	"IFM.h"
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)

/* Function define */
/* SysMntMain()��Call���� */
unsigned short	SysMnt_Movcnt( void );
unsigned short	SysMnt_LkMovcnt( void );
unsigned short	SysMnt_LkMcntClr( short kind, ushort s_no, char ctype );
unsigned short	SysMnt_CardIssue( void );
unsigned short	SysMnt_MvcntMenu( void );
// MH810100(S) Y.Yamauchi 20191009 �Ԕԃ`�P�b�g���X(�����e�i���X)
unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 20191009 �Ԕԃ`�P�b�g���X(�����e�i���X)

ulong	BitShift_Left(ulong ,int );
void	LogDataClr_CheckBufferFullRelease(ushort LogSyu );

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* retry count, when bcc error occur */
#define	_SYSMNT_RETRY				2

#define	_SYSMNT_LOCK_ALL			(0xFFFFFFFF)

/* parameter of SysMntDisplayBackupRestoreTitle() */
enum {
	_SYSMNT_BKRS_START,
	_SYSMNT_BKRS_EXEC,
	_SYSMNT_BKRS_END,
};

/* parameter of sysmnt_GetMessage() */
enum {
	_SYSMNT_WAIT_MSG,
	_SYSMNT_NOT_WAIT_MSG
};

/* �����v�Z�e�X�g ��ʃ��[�h */
enum {
	RT_MODE0 = 0,
	RT_MODE1,
	RT_MODE_MAX
};

/* �����v�Z�e�X�g �T�u��ʃ��[�h */
enum {
	RT_SUB_MODE0 = 0,
	RT_SUB_MODE1,
	RT_SUB_MODE2,
	RT_SUB_MODE3,
	RT_SUB_MODE_MAX
};

/* �����v�Z�e�X�g ��{��� �\������ */
enum {
	RT_I_YEAR = 0,
	RT_I_MONT,
	RT_I_DAY,
	RT_I_HOUR,
	RT_I_MIN,
	RT_O_YEAR,
	RT_O_MONT,
	RT_O_DAY,
	RT_O_HOUR,
	RT_O_MIN,
// MH810100(S) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	RT_FLP_NO,
	RT_KIND,
// MH810100(E) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
	RT_PASS,
	RT_BASE_MAX
};

/* �����v�Z�e�X�g �v�Z���ʉ�� �\������ */
enum {
	RT_RYOUKIN = 0,
	RT_U_CARD,
	RT_RESULT_MAX
};

/* �����v�Z�e�X�g ����������͉�� �\������ */
enum {
	RT_PASS_KIND = 0,
	RT_S_YEAR,
	RT_S_MONT,
	RT_S_DAY,
	RT_E_YEAR,
	RT_E_MONT,
	RT_E_DAY,
	RT_PASSINFO_MAX
};

/* �����v�Z�e�X�g �|������(�|�����p)���͉�� �\������ */
enum {
	RT_KAKE_NO = 0,
	RT_KAKEINFO_MAX
};

/* �����v�Z�e�X�g �T�[�r�X����ʓ��͉�� �\������ */
enum {
	RT_SERV_KIND = 0,
	RT_SERVKIND_MAX
};

/* �����v�Z�e�X�g �g�p�J�[�h */
enum {
	RT_CRD_TICK = 1,
	RT_CRD_PASS,
	RT_CRD_SERV,
	RT_CRD_KAKE
};

#define DISP_CHENGE(low, col, siz, zer, mod, cnt)	\
	do \
	{ \
		ulong tempdata = disp_Position((ushort)(cnt),&col,&siz);\
		opedpl3(low, col, tempdata, siz, zer, mod, COLOR_BLACK,  LCD_BLINK_OFF);	\
	} while (0)

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* parameter of SysMntUploadLog() */
typedef struct {
	ulong		record_size;								/* size of 1 record */
	const uchar	*filename;									/* filename to upload data of */
	const char	*ram_record;								/* top address of log records on ram */
	ulong		ram_record_num;								/* number of records on ram */
	ulong		(*get_record_num)(long *);					/* api pointer to get number of records on flash */
	ulong		(*get_first_log)(t_FltLogHandle*, char*);	/* api pointer to get oldest record data on flash */
	ulong		(*get_next_log)(t_FltLogHandle*, char*);	/* api pointer to get record data on flash */
}t_UploadLogParam;

struct T_FrmPcRyoukin{										/*								*/
	unsigned char			Flap_no;						/* �ׯ��No.						*/
	unsigned char			Kaisu;							/* �J��Ԃ���					*/
	date_time_rec			TInTime;						/*���ɓ���(���t�\���̎Q��)		*/
	date_time_rec			TOutTime;						/*�o�ɓ���(���t�\���̎Q��)		*/
	date_time_rec			InAdd;							/* ���ɉ��Z�� 					*/
	date_time_rec			OutAdd;							/* �o�ɉ��Z��					*/
	unsigned short			Ticket[10];						/* ����							*/
	unsigned char			PkTicket;						/* '0':���Z�O���Ԍ�				*/
	unsigned char			yobi1;							/* �ׯ��No.						*/
	unsigned char			yobi2;							/* �ׯ��No.						*/
															/*								*/
}; 															/*								*/

// �����v�Z�e�X�g�\�����ڐ���e�[�u���\��
typedef struct {
	ushort	line;		// �s
	ushort	lpos;		// ��
	ushort	keta;		// ��
	uchar	width;		// ���p(0)�^�S�p(1)
	uchar	min;		// �ŏ��l
	ushort	max1;		// �ő�l1
	uchar	max2;		// �ő�l2
} t_RT_DispItem;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_sysmnt_NextBlkNo(no)	((uchar)(((no)+1 > 99) ? 1 : (no)+1))

static	ushort	SysMnt_BackupRestore(void);
static	ushort	SysMnt_BackupRestoreData(ushort kind);
static	ushort	SysMnt_FileTransfer(void);
static	void	sysmnt_StartBkRsUpDwn(t_SysMnt_ErrInfo *errinfo);
#define			_sysmnt_StartMemoryInit(errinfo)	sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartBackupRestore(errinfo)	sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartUpload(errinfo)		sysmnt_StartBkRsUpDwn(errinfo)
#define			_sysmnt_StartDownload(errinfo)		sysmnt_StartBkRsUpDwn(errinfo)
static	void	sysmnt_InitErr(t_SysMnt_ErrInfo *errinfo);
static	void	sysmnt_OnFlashErr(t_SysMnt_ErrInfo *errinfo, ulong fltcode);

static	void	SysMntDisplayBackupRestoreTitle(ushort kind, int phase);
static	void	SysMntDisplayResult(t_SysMnt_ErrInfo *errinfo);
static	short	sysmnt_WaitF5(void);
static	short	sysmnt_GetMessage(int option);
static	BOOL	sysmnt_RamLock(ulong lock);
static	void	sysmnt_RamUnlock(ulong unlock);
static uchar	SysMnt_Log_Printer(ushort LogSyu);
uchar	LogDatePosUp2( uchar *pos, uchar req );
void	LogDateGet2( date_time_rec *Old, date_time_rec *New, ushort *Date );
void	LogDateDsp2( ushort *Date, uchar pos );
uchar	LogDateChk2( uchar pos, short data );
uchar	LogDateChk_Range_Minute( uchar pos, short data );
static	void	RT_init_disp_item( void );
static	void	RT_base_disp( long *p_in );
// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
static	void	RT_attention_disp( void );
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
static	void	RT_change_disp(char mode, char sub_mode, char pos, long data, ushort rev);
static	uchar	RT_check_disp_data( long data );
static	uchar	RT_check_ymd( long year, long month, long day );
static	void	RT_get_disp_data( long *p_in );
static	void	RT_move_cursor( ushort msg, long *p_in );
static	uchar	RT_check_KEY_TEN_F3( long *p_in );
static	short	RT_ryo_cal( void );
static	uchar	RT_check_KEY_TEN( long *p_in, ushort msg );
static	uchar	RT_check_disp_change( long *p_in, ushort msg );
static	void	RT_recover_disp(void);
static	void	RT_init_cal_data( void );
static	void	RT_make_mag_data( char *p_data );
static	void	RT_log_regist( void );
static	void	RT_make_pass( m_gtapspas *p_magP );
static	void	RT_make_service( m_gtservic *p_magS );
static	short	RT_day_max( void );
static	short	RT_set_tim( ushort, struct clk_rec * );
static	void	RT_disp_cal_result( void );
static	void	RT_bkp_rst_data( char kind );
static	void	RT_clear_error( void );
static	void	RT_get_valid_no( char *p_data );
static	void	RT_ryo_cal2( char, ushort );
static	void	RT_edit_log_data( T_FrmReceipt *p_pri_data );
static	void	RT_vl_carchg( ushort );

unsigned char bin_to_asc( unsigned char c );

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/* timer for receive */
ushort	SysMnt_RcvTimer;

/* work */
extern	uchar	SysMnt_Work[SysMnt_Work_Size];	/* 64KB */
	char	SysMnt_Work2[4][36];
	char	SysMnt_Work3[6][20];
	char	SysMnt_UP_DW_Text[T_FOOTER_GYO_MAX][36];

	T_SYSMNT_PKT	SysMnt_SndBuf;
	T_SYSMNT_PKT	SysMnt_RcvBuf;

/* filename to upload & download */
	const uchar	SYSMNT_LOGO_FILENAME[]			= {"Logo.bmp"};
	const uchar	SYSMNT_HEADER_FILENAME[]		= {"Header.txt"};
	const uchar	SYSMNT_FOOTER_FILENAME[]		= {"Footer.txt"};
	const uchar	SYSMNT_PARAM_FILENAME[]			= {"Parameter.csv"};
	const uchar	SYSMNT_LOCKPARAM_FILENAME[]		= {"LockInfo.csv"};
	const uchar	SYSMNT_SALELOG_FILENAME[]		= {"SaleLog_%02d%02d%02d%02d%02d%02d.bin"};
	const uchar	SYSMNT_TOTALLOG_FILENAME[]		= {"TotalLog_%02d%02d%02d%02d%02d%02d.bin"};
	const uchar	SYSMNT_ACCEPTFTR_FILENAME[]		= {"uketuke.txt"};
	const uchar	SYSMNT_SYOMEI_FILENAME[]		= {"shomeiran.txt"};
	const uchar	SYSMNT_KAMEI_FILENAME[]			= {"kameiten.txt"};
	const uchar	SYSMNT_EDYAT_FILENAME[]			= {"EdyAtCmd.txt"};
	const uchar	SYSMNT_USERDEF_FILENAME[]		= {"userdefine.ini"};
	const uchar	SYSMNT_ERR_FILENAME[]			= {"ErrLog.csv"};
	const uchar	SYSMNT_ARM_FILENAME[]			= {"ArmLog.csv"};
	const uchar	SYSMNT_NONI_FILENAME[]			= {"MonitorLog.csv"};
	const uchar	SYSMNT_OPE_MONI_FILENAME[]		= {"OpeMonitorLog.csv"};
	const uchar	SYSMNT_CAR_INFO_FILENAME[]		= {"CarStatusInfo.csv"};
	const uchar	SYSMNT_TCARDFTR_FILENAME[]		= {"TCardFooter.txt"};
	const uchar	SYSMNT_AZUFTR_FILENAME[]		= {"AzukariFooter.txt"};
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	const uchar	SYSMNT_CREKBRFTR_FILENAME[]		= {"CreKabaraiFooter.txt"};
	const uchar	SYSMNT_EPAYKBRFTR_FILENAME[]	= {"EpayKabaraiFooter.txt"};
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	const uchar	SYSMNT_FUTUREFTR_FILENAME[]		= {"GojitsuFooter.txt"};
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	const uchar	SYSMNT_EMGFOOTER_FILENAME[]		= {"emgfooter.txt"};
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	const uchar	SYSMNT_REMOTE_DL_RESULT_FILENAME[] = {"RemoteDLResult.txt"};
	const uchar	SYSMNT_REMOTE_DL_PARAM[] 		= {"PARAMETER.TXT"};
	const uchar	SYSMNT_REMOTE_DL_CONNECT[] 		= {"CONNECT.TXT"};
// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
	const uchar	SYSMNT_UP_PARAM_LCD[] 			= {"PARAMETER.DAT"};
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)

/* flag to check mode change occured */
static	BOOL	SysMntModeChange;
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
static	BOOL	SysMntLcdDisconnect;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
t_RT_Ctrl	rt_ctrl;								// �����v�Z�e�X�g����f�[�^
													// mode=0:��{��ʕҏW��
													// 		1:�����v�Z���ʕ\����
													// sub_mode=0:mode=0/1 ��{��ʕҏW��
													// 			1:mode=0/1 ��������ҏW��
													// 			2:mode=0/1 �|������ҏW��
													// 			3:mode=  1 �T�[�r�X����ʕҏW��
t_RT_Ctrl	rt_ctrl_bk1;							// �����v�Z�e�X�g����f�[�^�o�b�N�A�b�v�̈�(�e��ʁ̎q��ʑJ�ڗp)
t_RT_Ctrl	rt_ctrl_bk2;							// �����v�Z�e�X�g����f�[�^�o�b�N�A�b�v�̈�(������ʁ̌v�Z���ʉ�ʑJ�ڗp)
long	base_data[RT_BASE_MAX];						// ��{��ʊ�{�f�[�^
long	rslt_data[RT_RESULT_MAX];					// �v�Z���ʉ�ʃf�[�^
long	pass_data[RT_PASSINFO_MAX];					// ����������͉�ʃf�[�^
long	kake_data[RT_KAKEINFO_MAX];					// �|������(�|�����p)���͉�ʃf�[�^
long	serv_data[RT_SERVKIND_MAX];					// �T�[�r�X����ʉ�ʃf�[�^
long	base_data_bk[RT_BASE_MAX];					// ��{��ʊ�{�f�[�^�o�b�N�A�b�v
long	pass_data_bk[RT_PASSINFO_MAX];				// ����������͉�ʃf�[�^
long	kake_data_bk[RT_KAKEINFO_MAX];				// �|������(�|�����p)���͉�ʃf�[�^
long	serv_data_bk[RT_SERVKIND_MAX];				// �T�[�r�X����ʉ�ʃf�[�^

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

ushort	SysMnt_Log( void );
ushort	SysMnt_Log_CntDsp1( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd );
ushort	SysMnt_LogClr( void );
uchar	LogDatePosUp( uchar *pos, uchar req );
uchar	LogDateChk( uchar pos, short data );
void	LogCntDsp( ushort LogCnt );
void	LogDateDsp( ushort *Date, uchar pos );
void	LogDateDsp4( ushort *Date );
void	LogDateDsp5( ushort *Date, uchar pos );
ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req );
void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
uchar	LogDataClr( ushort LogSyu );

static ushort	card_iss_pass(void);
static void		cardiss_pass_prm_dsp(char pos, long prm, ushort rev);
static char		cardiss_pass_prm_chk( long *prm );
static uchar	FRdr_DipSW_GetStatus( void );
static ushort	card_iss_kakari(void);
static void		cardiss_kakari_prm_dsp(char pos, char prm, ushort rev);
static char		cardiss_kakari_prm_chk(char pos, char prm);

static ulong	disp_Position( ushort, ushort*, ushort*);

/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽҲݏ���                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMntMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void SysMntMain( void )
{
	unsigned short	usSysEvent = MNT_NONE;
	char	wk[2];
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar ucReq = 0;	// 0=OK/1=NG/2=OK(�ċN���s�v)
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//	ushort	mode = 0;
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)

	DP_CP[0] = DP_CP[1] = 0;

	memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));	// �����v�Z�e�X�g����f�[�^������

	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[0] );			// [00]	"���V�X�e�������e�i���X���@�@�@"

		if(( rt_ctrl.param.set != ON ) &&
		   ( rt_ctrl.param.no == 0 )){
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// 			usSysEvent = Menu_Slt( SMMENU, SYS_TBL, (char)SYS_MENU_MAX, (char)1 );
			if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
				usSysEvent = Menu_Slt(SMMENU_CC, SYS_TBL_CC, (char)SYS_MENU_MAX_CC, (char)1);
			} else {					// �ʏ헿���v�Z���[�h
				usSysEvent = Menu_Slt(SMMENU, SYS_TBL, (char)SYS_MENU_MAX, (char)1);
			}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usSysEvent ){

		/* �p�����[�^�[�ݒ� */
			case MNT_PRSET:
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				usSysEvent = ParSetMain();				
				usSysEvent = BPara_Set();
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
		/* ����`�F�b�N */
			case MNT_FNCHK:
				usSysEvent = FncChkMain();
				break;
		/* ����J�E���g */
			case MNT_FNCNT:
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				usSysEvent = SysMnt_MvcntMenu();				
				usSysEvent = SysMnt_Movcnt();
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
		/* ���O�t�@�C���v�����g */
			case MNT_LOGFL:
				usSysEvent = SysMnt_Log();
				break;
		/* �o�b�N�A�b�v�^���X�g�A */
			case MNT_BAKRS:
				usSysEvent = SysMnt_BackupRestore();
				break;
//���g�p�ł��B(S)
		/* �J�[�h���s */
			case MNT_CDISS:
				if(0 == prm_get(COM_PRM, S_PAY, 21, 1, 3)) {	// ���C���[�_�[�ڑ��Ȃ�(02-0021�C=0)
					BUZPIPI();
					break;
				}
				if((prm_get( COM_PRM,S_SYS,12,1,6 ) == 1) &&
					((GTF_PKNO_LOWER > CPrmSS[S_SYS][1]) ||
					(CPrmSS[S_SYS][1] > GTF_PKNO_UPPER)) ){
					BUZPIPI();								//���ԏ�No�ݒ�ԈႢ
					break;
				}else if((prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) &&
					(( APSF_PKNO_LOWER > CPrmSS[S_SYS][1] ) ||
					(CPrmSS[S_SYS][1] > APSF_PKNO_UPPER)) ){
					BUZPIPI();								//���ԏ�No�ݒ�ԈႢ
					break;
				}
				usSysEvent = SysMnt_CardIssue();
				break;
//���g�p�ł��B(E)
		/* �t�@�C���]�� */
			case MNT_FLTRF:
				usSysEvent = SysMnt_FileTransfer();
				break;
		/* �V�X�e���ؑ� */
			case MNT_SYSSW:
				usSysEvent = SysMnt_SystemSwitch();
				break;
		/* �����v�Z�e�X�g */
			case MNT_RYOTS:
				ryo_test_flag = 1;
				usSysEvent = SysMnt_RyoTest();
				ryo_test_flag = 0;
				break;

			//Return Status of User Menu
// MH810100(S) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return;
				break;
// MH810100(E) Y.Yamauchi 20191213 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return;
				break;

			default:
				break;
		}
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of User Operation
		if( usSysEvent == MOD_CHG ){
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			return;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
//			mode = op_wait_mnt_close();	// �����e�i���X�I������
//			if( mode == MOD_CHG ){
			{
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
				if( mnt_GetFtpFlag() != FTP_REQ_NONE ){
					ucReq = 1;	// ���s
					if( 0 == lcdbm_setting_upload_FTP() ){		// LCDӼޭ�قɑ΂������Ұ�����۰�ނ��s��
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
// MH810100(S) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
			}
			else {
				// �����v�Z�e�X�g����̖߂�Ń��j���[��\���ł��Ȃ��̂ŁA
				// �����v�Z�f�[�^�����N���A����
				memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));	// �����v�Z�e�X�g����f�[�^������
			}
// MH810100(E) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4777 �����e�i���X�Ńu�U�[���ʕύX��Ƀh�A���琸�Z�{�^���������Ă����Z�J�n�ł��Ȃ�)
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if( usSysEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;							// ��������Ԃ�
			OPECTL.init_sts = 0;							// ��������������ԂƂ���
			OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
// MH810100(S) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(#3887 �ʐM�������̍l���R��C��)
			OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
// MH810100(E) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(#3887 �ʐM�������̍l���R��C��)
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

/*[]----------------------------------------------------------------------[]*/
/*| ����J�E���g���j���[                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_MvcntMenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort ����Ӱ��(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//���g�p�ł��B�iS�j
unsigned short	SysMnt_MvcntMenu( void )
{
	unsigned short	usCntEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{
		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[1] );			// "������J�E���g���@�@�@�@�@�@�@"

		Is_CarMenuMake(MV_CNT_MENU);

		usCntEvent = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usCntEvent ){

		/* 1.����J�E���g */
			case FCNT_FUNC:
				wopelg( OPLOG_DOUSACOUNT, 0, 0 );	// ���엚��o�^
				usCntEvent = SysMnt_Movcnt();
				break;

		/* 2.�t���b�v���u����J�E���g */
			case FCNT_FLAP:
				gCurSrtPara = FCNT_FLAP;
				wopelg( OPLOG_FLAPDCNT, 0, 0 );		// �t���b�v���u����J�E���g���엚��o�^
				MovCntClr = OFF;					// ���춳�ĸر���s��Ԃ��������i����āj
				usCntEvent = SysMnt_LkMovcnt();
				if( MovCntClr == ON ){					// ���춳�ĸر���s����H
					wopelg( OPLOG_FLAPDCNTCLR, 0, 0 );	// ���엚��o�^�i�ׯ�ߑ��u���춳�ĸر�j
				}
				break;

		/* 3.���b�N���u����J�E���g */
			case FCNT_ROCK:
				gCurSrtPara = FCNT_ROCK;
				wopelg( OPLOG_LOCKDCNT, 0, 0 );		// ���b�N���u����J�E���g���엚��o�^
				MovCntClr = OFF;					// ���춳�ĸر���s��Ԃ��������i����āj
				usCntEvent = SysMnt_LkMovcnt();
				if( MovCntClr == ON ){					// ���춳�ĸر���s����H
					wopelg( OPLOG_LOCKDCNTCLR, 0, 0 );	// ���엚��o�^�iۯ����u���춳�ĸر�j
				}
				break;

			//Return Status of User Menu
			case MOD_EXT:
				return MOD_EXT;

			default:
				break;
		}
		//Return Status of User Operation
		if( usCntEvent == MOD_CHG ){
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if( usCntEvent == MOD_CUT ){
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];	
	}
}
//���g�p�ł��B�iE�j
/*[]----------------------------------------------------------------------[]*/
/*| ���춳������                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_Movcnt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort ����Ӱ��(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura �d�l�C���ɑΉ�                       |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SysMnt_Movcnt( void )
{
	ushort	msg;
	int		top;
	int		pos;
	char	i;
	char	mod;		/* 0:�v�����g��� 1:�N���A��� 2:�S�N���A��� */
	char	pgchg;
	T_FrmDousaCnt	DousaCntData;
	ushort	culmn = 17;
	ushort	datasize = 9;
// MH810100(S) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
	int		inji_end = 0;
// MH810100(E) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)


	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[1] );		// [01]	"������J�E���g���@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[59] );					// [59]	"�@���@�@���@�N���A ����� �I�� "

	top = 0;
	mod = 0;
	pgchg = 1;
	for( ; ; ){

		if (pgchg) {
			for (i = 0; i < 6 ; i++) {
				if (top+i < MOV_CNT_MAX){
					grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CNTSTR[top+i] );				// �e���ڕ\��
					DISP_CHENGE((ushort)(1+i),culmn,datasize,0,0,top+i);			// ���ĕ\��
				}
				else {
					displclr((ushort)(i+1));									// ���g�p�̍s�͸ر
				}
			}
			if (mod == 1) {
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);		// ���ĕ\��(���])
			}
			pgchg = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT) {
			BUZPI();
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
	/*--- �v�����g��� ---*/
		if (mod == 0) {
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F5:	// F5:Exit
				BUZPI();
				return( MOD_EXT );
			case KEY_TEN_F1:	// F1:"��"
			case KEY_TEN_F2:	// F2:"��"
				BUZPI();
				if (msg == KEY_TEN_F1) {
					top -= 6;
					if (top < 0)
						top = (char)((MOV_CNT_MAX-1)/6*6);
				}
				else {
					top += 6;
					if (top >= MOV_CNT_MAX)
						top = 0;
				}
				pgchg = 1;
				break;
			case KEY_TEN_F3:	// F3:"�ر"
				BUZPI();
				mod = 1;
				pos = top;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ���ĕ\��(���])
				Fun_Dsp( FUNMSG[62] );		// [62]	"�@���@�@���@�N���A �S��  �I�� "
				wopelg( OPLOG_DOUSACOUNTCLR, 0, 0 );	// ���엚��o�^
				break;
			case KEY_TEN_F4:	// F4:"�����"
				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
					BUZPIPI();
					break;
				}
				BUZPI();
				/* �v�����g���s */
				DousaCntData.prn_kind = R_PRI;
				DousaCntData.Kikai_no = (uchar)CPrmSS[S_PAY][2];
				queset( PRNTCBNO, PREQ_DOUSACNT, sizeof( T_FrmDousaCnt ), &DousaCntData );	//Move Count Print
				Ope_DisableDoorKnobChime();

				/* �v�����g�I����҂����킹�� */
				Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					/* �v�����g�I�� */
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
// MH810100(S) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						inji_end = 1;
// MH810100(E) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						break;
					}
					/* �^�C���A�E�g(10�b)���o */
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* ���[�h�`�F���W */
					if (msg == KEY_MODECHG) {
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
						// �h�A�m�u���ǂ����̃`�F�b�N�����{
						if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
						}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if ( msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					
				}
				break;
			default:
				break;
			}
		}
	/*--- �N���A��� ---*/
		else if (mod == 1) {
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F5:	// F5:Exit
				BUZPI();
				mod = 0;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,0,pos);			// ���ĕ\��(���])
				Fun_Dsp( FUNMSG[59] );					// [59]	"�@���@�@���@�N���A ����� �I�� "
				break;
			case KEY_TEN_F3:	// F3:"�ر"
				Mov_cnt_dat[pos] = 0;
// MH810100(S) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
				if (pos == LCD_LIGTH_CNT) {
					lcdbm_notice_ope(LCDBM_OPCD_CLEAR_COUNT, 0);
				}
// MH810100(E) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
				/* break */
			case KEY_TEN_F1:	// F1:"��"
			case KEY_TEN_F2:	// F2:"��"
				BUZPI();
				if (msg == KEY_TEN_F1) {
					pos--;
					if (pos < 0) {
						pos = (char)(MOV_CNT_MAX-1);
						top = (char)(pos/6*6);
						pgchg = 1;
					}
					else if (pos < top) {
						top -= 6;
						pgchg = 1;
					}
					else {
						DISP_CHENGE((ushort)(1+pos+1-top),culmn,datasize,0,0,pos+1);		// ���ĕ\��(���]) �O�̍s
						DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ���ĕ\��(���]) ���̍s
					}
				}
				else {
					pos++;
					if (pos >= MOV_CNT_MAX) {
						pos = 0;
						top = 0;
						pgchg = 1;
					}
					else if (pos >= top+6) {
						top += 6;
						pgchg = 1;
					}
					else {
						DISP_CHENGE((ushort)(1+pos-1-top),culmn,datasize,0,0,pos-1);		// ���ĕ\��(���]) �O�̍s
						DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,1,pos);			// ���ĕ\��(���]) ���̍s
					}
				}
				break;
			case KEY_TEN_F4:	// F4:"�S�ر"
				BUZPI();
				mod = 2;
				DISP_CHENGE((ushort)(1+pos-top),culmn,datasize,0,0,pos);			// ���ĕ\��(���])
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR2[9]);		/* "�@�@�@�S�ăN���A���܂����H�@�@" */
				Fun_Dsp( FUNMSG[19] );				/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
				break;
			default:
				break;
			}
		}
	/*--- �S�N���A��� ---*/
		else {	/* if (mod == 2) */
			switch( msg ){	// FunctionKey Enter
			case KEY_TEN_F3:	// F3:"�͂�"
				/* �S�f�[�^�N���A */
				memset(Mov_cnt_dat, 0, sizeof(Mov_cnt_dat));
// MH810100(S) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
				lcdbm_notice_ope(LCDBM_OPCD_CLEAR_COUNT, 0);
// MH810100(E) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
			case KEY_TEN_F4:	// F4:"������"
				BUZPI();
				Fun_Dsp( FUNMSG[62] );		// [62]	"�@���@�@���@�N���A �S��  �I�� "
				pgchg = 1;
				mod = 1;
				break;
			default:
				break;
			}
		}

// MH810100(S) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
		if (inji_end == 1 && CP_MODECHG) {
			// �󎚏I����A�h�A�m�u��Ԃł���΃����e�i���X�I��
			return MOD_CHG;
		}
// MH810100(E) S.Nishimoto 2020/09/02 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		����J�E���g���\���ʒu���򏈗�										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	disp_Position( cnt,  culmn, datasize)				|*/
/*|																			|*/
/*|	PARAMETER		:	ushort cnt			�\�����鍀�ڗv�f��				|*/
/*|						ushort *culmn		�\���ʒu						|*/
/*|						ushort *datasize	�\���T�C�Y						|*/
/*|																			|*/
/*|	RETURN VALUE	:	ulong	 			�\�����铮��J�E���g			|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	fsiabc T.Namioka											|*/
/*|	Date	:	2006-02-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
static ulong	disp_Position( ushort cnt, ushort *culmn, ushort *datasize)
{
	ulong tmpdata=0;
// MH810100(S) Y.Yamauchi 2019/10/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	if(cnt == LCD_LIGTH_CNT){
//		tmpdata = Mov_cnt_dat[cnt]/60;	//���P�ʂ����ԒP�ʂɕϊ������f�[�^�̐ݒ�
//		*culmn = 16;					//�J�E���g���̕\���J�n�ʒu�̐ݒ�
//		*datasize = 8;					//�f�[�^�T�C�Y�̐ݒ�
//	//�\�����ڂ�LCD�ޯ�ײē_���ȊO�̏ꍇ
//	}else{
//		tmpdata = Mov_cnt_dat[cnt];		//�f�[�^�̐ݒ�
//		*culmn = 17;					//�J�E���g���̕\���J�n�ʒu�̐ݒ�
//		*datasize = 9;					//�f�[�^�T�C�Y�̐ݒ�
//	}
//	return tmpdata;
//		
//}
	//�\�����ڂ�LCD�܂���QRذ�ް���쎞�Ԃ������ꍇ
	if(cnt == QR_READER_CNT){
		tmpdata = Mov_cnt_dat[cnt]/60;	//���P�ʂ����ԒP�ʂɕϊ������f�[�^�̐ݒ�
		*culmn = 16;					//�J�E���g���̕\���J�n�ʒu�̐ݒ�
		*datasize = 8;					//�f�[�^�T�C�Y�̐ݒ�
	}else if(cnt == LCD_LIGTH_CNT){
		tmpdata = Mov_cnt_dat[cnt];	
		*culmn = 16;					//�J�E���g���̕\���J�n�ʒu�̐ݒ�
		*datasize = 8;					//�f�[�^�T�C�Y�̐ݒ�
	}
	//�\�����ڂ�LCD,QR�ȊO�̏ꍇ
	else{
		tmpdata = Mov_cnt_dat[cnt];		//�f�[�^�̐ݒ�
		*culmn = 17;					//�J�E���g���̕\���J�n�ʒu�̐ݒ�
		*datasize = 9;					//�f�[�^�T�C�Y�̐ݒ�
	}
// MH810100(E) Y.Yamauchi 2019/10/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
	
	return tmpdata;
}


unsigned short	SysMnt_LkMovcnt( void )
{
	short	msg = -1;
	ushort	page[2];
	short	inpt = -1;
	char	timeout = 1;
	T_FrmLockDcnt	pri_data;
	char	fk_page = 0;			// ̧ݸ��ݷ��\����ʁi0�F���춳�ĕ\����ʁ^1:�ر�����ʁj
	ushort	s_no_wk;				// ���uNo.ܰ�
	ushort	s_no_max;				// �ő呕�uNo.
	ulong	cnt1,cnt2,cnt3;			// ���춳����i�[ܰ�
	uchar	re_dsp = 0;				// �S��ʍĕ\���׸�
	ushort	ret;					// �ر�m�F��ʖ߂�ð��
	char	type;
	ushort	tno;					// �[��No.
	ushort	lock_no;				// �[�����ʔ�

	dispclr();

	if( gCurSrtPara == FCNT_FLAP ){
		// �t���b�v���u����J�E���g
		memset( MntFlapDoCount, 0xff, sizeof( MntFlapDoCount ) );	// �ׯ�ߑ��u���춳���ޯ̧0xff�ر
		type = (char)GetCarInfoParam();
		switch( type&0x06 ){
			case	0x04:
				s_no_max = INT_CAR_LOCK_MAX;						// �ׯ�ߑ��u�ő呕�uNo.���
				type = _MTYPE_INT_FLAP;
				break;
			case	0x06:
				s_no_max = CAR_LOCK_MAX+INT_CAR_LOCK_MAX;			// �ׯ�ߑ��u�ő呕�uNo.���
				type = (uchar)(_MTYPE_FLAP + _MTYPE_INT_FLAP);
				break;
			case	0x02:
			default:
				s_no_max = CAR_LOCK_MAX;							// �ׯ�ߑ��u�ő呕�uNo.���
				type = _MTYPE_FLAP;
				break;
		}
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[75] );							// [75]	"���t���b�v���u����J�E���g���@"
		for(s_no_wk = 0; s_no_wk < s_no_max; ++s_no_wk) {
			lock_no = 1;
			
			if(type == _MTYPE_INT_FLAP) {
				tno = LockInfo[s_no_wk + INT_CAR_START_INDEX].if_oya;
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// �����
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// �蓮�����
				MntFlapDoCount[s_no_wk + INT_CAR_START_INDEX][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// �̏��
			}
			else {
				tno = LockInfo[s_no_wk].if_oya;
				MntFlapDoCount[s_no_wk][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// �����
				MntFlapDoCount[s_no_wk][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// �蓮�����
				MntFlapDoCount[s_no_wk][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// �̏��
			}
		}
	}
	else{
		// ���b�N���u����J�E���g
		s_no_max = BIKE_LOCK_MAX;									// ۯ����u�ő呕�uNo.���
		memset( MntLockDoCount, 0xff, sizeof( MntLockDoCount ) );	// ۯ����u���춳���ޯ̧0xff�ر
		type = _MTYPE_LOCK;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[2] );							// [02]	"�����b�N���u����J�E���g���@�@"
		for(s_no_wk = 0; s_no_wk < s_no_max; ++s_no_wk) {
			tno = LockInfo[s_no_wk + BIKE_START_INDEX].if_oya;
			lock_no = LockInfo[s_no_wk + BIKE_START_INDEX].lok_no;

			MntLockDoCount[s_no_wk][0] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulAction;	// �����
			MntLockDoCount[s_no_wk][1] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulManual;	// �蓮�����
			MntLockDoCount[s_no_wk][2] = IFM_LockAction.sSlave[tno - 1].sLock[lock_no - 1].ulTrouble;	// �̏��
		}
	}
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"�@�@ ���΂炭���҂������� �@�@"
	page[0] = page[1] = 1;

	if( gCurSrtPara == FCNT_FLAP ){
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"�@�t���b�v���u�@�@�@�@�@�@�@�@"
	}
	else{
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"�@���b�N���u�@�@�@�@�@�@�@�@�@"
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );	// [04]	"�@����񐔁@�@�@�@�@�@�@�@��@"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );	// [05]	"�@�蓮����񐔁@�@�@�@�@�@��@"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );	// [06]	"�@�̏�񐔁@�@�@�@�@�@�@�@��@"
	Fun_Dsp( FUNMSG[88] );				// [88]	"  �{  �|�^�� ����� ����  �I�� "
	page[1] = page[0] + 1;				// �����I�ɉ�ʍX�V�����邽�߂�page[1]��page[0]�Ƃ���
	timeout = 0;
	goto LInit;

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg )){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );		// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );		// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"�I��"
				if( timeout )	break;
				BUZPI();
				Lagcan( OPETCBNO, 6 );		// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1
				if( timeout )	break;

				if( fk_page == 0 ){
					// ���ĕ\����ʁF"�{"
					BUZPI();
					inpt = -1;
					page[0] = (( page[0] >= s_no_max ) ? 1 : page[0] + 1 );
				}
				else{
					// �ر�����ʁF"�ر"
					if( gCurSrtPara == FCNT_FLAP ){
						// �ׯ�ߑ��u
						if( type == _MTYPE_INT_FLAP )
							s_no_wk = page[0]+INT_CAR_START_INDEX;	// ���uNo.��āi51�`100�j
						else
						s_no_wk = page[0];					// ���uNo.��āi1�`50�j
					}
					else{
						// ۯ����u
						s_no_wk = page[0]+BIKE_START_INDEX;	// ���uNo.��āi101�`150�j
					}
					if( LockInfo[s_no_wk-1].lok_syu ){
						// �ڑ�����
						BUZPI();
						ret = SysMnt_LkMcntClr( gCurSrtPara, s_no_wk, type );	// �ر�m�F��ʏ���
					if( ret == MOD_CHG ){
						return( MOD_CHG );
						}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( ret == MOD_CUT ){
						return( MOD_CUT );
						}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						page[1] = page[0] + 1;		// �����I�ɉ�ʍX�V�����邽�߂�page[1]��page[0]�Ƃ���
						re_dsp = 1;					// �S��ʍĕ\��
					}
					else{
						// �ڑ��Ȃ�
						BUZPIPI();
					}
				}
				break;

			case KEY_TEN_F2:	// F2
				if( timeout )	break;
				BUZPI();

				if( fk_page == 0 ){
					// ���ĕ\����ʁF"-/��"
					if( inpt == -1 ){
						// ���uNo.�����͂̏ꍇ�F"-"
						page[0] = (( page[0] <= 1 ) ? s_no_max : page[0] - 1 );
					}
					else{
						// ���uNo.���͍ς݂̏ꍇ�F"�Ǐo"
						if(( 1 <= inpt )&&( inpt <= s_no_max )){
							BUZPI();
							page[0] = inpt;
							inpt = -1;
						}else if( inpt == -1 ){
							BUZPI();
						}else{
							BUZPIPI();
							opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
							inpt = -1;
						}
					}
				}
				else{
					// �ر�����ʁF"�S�ر"
					ret = SysMnt_LkMcntClr( gCurSrtPara, 0, type );	// �ر�m�F��ʏ���
					if( ret == MOD_CHG ){
						return( MOD_CHG );
					}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( ret == MOD_CUT ){
						return( MOD_CUT );
					}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
					page[1] = page[0] + 1;		// �����I�ɉ�ʍX�V�����邽�߂�page[1]��page[0]�Ƃ���
					re_dsp = 1;					// �S��ʍĕ\��
				}
				break;

			case KEY_TEN_F3:	// F3:"�����"
				if( timeout || fk_page ){		// ���춳�ēǏo���@�܂��́@�ر�����ʂł�����ķ����͕s��
					BUZPIPI();
					break;
				}
				if(Ope_isPrinterReady() == 0){	// ���V�[�g�v�����^���󎚕s�\�ȏ��
					BUZPIPI();
					break;
				}
				BUZPI();

				/* ������󎚗v�� */
				pri_data.prn_kind = R_PRI;							// �������ʁFڼ��
				pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// �@�BNo.
				pri_data.Req_syu = type;							// 1:ۯ� 2:IF�ׯ�� 3:�����ׯ�� 5:�ׯ�ߗ���																	
				queset(PRNTCBNO, PREQ_LK_DOUSACNT, sizeof(T_FrmLockDcnt), &pri_data);
				Ope_DisableDoorKnobChime();

				/* �v�����g�I����҂����킹�� */
				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					/* �v�����g�I�� */
					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
						Lagcan(OPETCBNO, 6);
						break;
					}
					/* �^�C���A�E�g(10�b)���o */
					if (msg == TIMEOUT6) {
						BUZPIPI();
						break;
					}
					/* ���[�h�`�F���W */
					if (msg == KEY_MODECHG) {
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return MOD_CHG;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if (msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				break;

			case KEY_TEN_CL:	/* ��� */
				if( timeout || fk_page )	break;	// ���춳�ēǏo���@�܂��́@�ر�����ʂł͎�������͕s��
				BUZPI();
				if(inpt != -1)	inpt = -1;
				opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;

			case KEY_TEN_F4:	// F4:"����"�i̧ݸ��ݷ��ؑցj
				if( timeout )	break;
				BUZPI();
				fk_page ^= 1;
				if (fk_page) {
					opedsp( 2, 22, page[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// ���uNo.���]�\��OFF
					Fun_Dsp(FUNMSG[89]);														// " �ر  �S�ر        ����  �I�� "
				} else {
					opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// ���uNo.���]�\��ON
					Fun_Dsp(FUNMSG[88]);														// "  �{  �|�^�� ����� ����  �I�� "
				}
				break;
				
			case TIMEOUT6:		// ��ϰ6��ѱ��(����ݽ����p)
				if( gCurSrtPara == FCNT_FLAP ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"�@�t���b�v���u�@�@�@�@�@�@�@�@"
				}
				else{
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"�@���b�N���u�@�@�@�@�@�@�@�@�@"
				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );	// [04]	"�@����񐔁@�@�@�@�@�@�@�@��@"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );	// [05]	"�@�蓮����񐔁@�@�@�@�@�@��@"
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );	// [06]	"�@�̏�񐔁@�@�@�@�@�@�@�@��@"
				Fun_Dsp( FUNMSG[88] );				// [88]	"  �{  �|�^�� ����� ����  �I�� "
				page[1] = page[0] + 1;				// �����I�ɉ�ʍX�V�����邽�߂�page[1]��page[0]�Ƃ���
				timeout = 0;
				break;

			case KEY_TEN:
				if( timeout || fk_page )	break;	// ���춳�ēǏo���@�܂��́@�ر�����ʂł��ݷ����͕s��
				BUZPI();
				inpt = ( inpt == -1 ) ? (short)(msg - KEY_TEN0):
					( inpt % 100 ) * 10 + (short)(msg - KEY_TEN0);
				opedsp( 2, 22, (ushort)inpt, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;

			default:
				break;
		}
LInit:
		/*** Page Change ***/
		if( page[0] != page[1] ){

			if( re_dsp ){
				// �S��ʍĕ\���i�ر��ʂ���̕��A�j
				if( gCurSrtPara == FCNT_FLAP ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[79] );	// [79]	"�@�t���b�v���u�@�@�@�@�@�@�@�@"
				}
				else{
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[3] );	// [03]	"�@���b�N���u�@�@�@�@�@�@�@�@�@"
				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[4] );		// [04]	"�@����񐔁@�@�@�@�@�@�@�@��@"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[5] );		// [05]	"�@�蓮����񐔁@�@�@�@�@�@��@"
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[6] );		// [06]	"�@�̏�񐔁@�@�@�@�@�@�@�@��@"
				if (fk_page) {
					Fun_Dsp(FUNMSG[89]);												// " �ر  �S�ر        ����  �I�� "
					opedsp( 2, 22, page[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ���b�N���u�m���D�\���i�ʏ�\���j
				} else {
					Fun_Dsp(FUNMSG[88]);												// "  �{  �|�^�� ����� ����  �I�� "
					opedsp( 2, 22, page[0], 3, 1, 1 , COLOR_BLACK, LCD_BLINK_OFF);	// ���b�N���u�m���D�\���i���]�\���j
				}
				re_dsp = 0;																// �S��ʍĕ\���׸�ؾ��
			}
			else{
				opedsp( 2, 22, page[0], 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ���b�N���u�m���D�\���i���]�\���j
			}
			if( gCurSrtPara == FCNT_FLAP ){
				// �ׯ�ߑ��u
				if( type == _MTYPE_INT_FLAP )
					s_no_wk = page[0]+INT_CAR_START_INDEX;	// ���uNo.��āi51�`100�j
				else
				s_no_wk = page[0];						// ���uNo.��āi1�`50�j
			}
			else{
				// ۯ����u
				s_no_wk = page[0]+BIKE_START_INDEX;		// ���uNo.��āi101�`150�j
			}

			if( LockInfo[s_no_wk-1].lok_syu ){
				/*** ۯ����u�ڑ��L�� ***/
				s_no_wk = page[0]-1;						// ���춳���ް��̔z��No.���
				if( gCurSrtPara == FCNT_FLAP ){
					if( type == _MTYPE_INT_FLAP )
						s_no_wk = s_no_wk+INT_CAR_START_INDEX;	// Index��āi50�`99�j
					// �ׯ�ߑ��u
					cnt1 = MntFlapDoCount[s_no_wk][0];		// �����
					cnt2 = MntFlapDoCount[s_no_wk][1];		// �蓮�����
					cnt3 = MntFlapDoCount[s_no_wk][2];		// �̏��
				}
				else{
					// ۯ����u
					cnt1 = MntLockDoCount[s_no_wk][0];		// �����
					cnt2 = MntLockDoCount[s_no_wk][1];		// �蓮�����
					cnt3 = MntLockDoCount[s_no_wk][2];		// �̏��
				}

				if( cnt1 == 0xffffffff ){
					/*** ���춳���ް�����M ***/
					grachr( 1, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][10] );	// [07]	"�@�@�@�@�@�@�@�@�@�@�@����M�@"
					grachr( 3, 20, 6, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"�@�@�@�@�@�@�@�@�@�@******�@�@"
					grachr( 4, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"�@�@�@�@�@�@�@�@�@�@�@****�@�@"
					grachr( 5, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );	// [07]	"�@�@�@�@�@�@�@�@�@�@�@****�@�@"
				}else{
					/*** ���춳���ް���M�ς� ***/
					displclr( 1 );							// Line Clear
					opedpl3( 3, 20, cnt1, 6, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// ����񐔕\��
					opedpl3( 4, 22, cnt2, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �蓮����񐔕\��
					opedpl3( 5, 22, cnt3, 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �̏�񐔕\��
				}
			}else{
				/*** ۯ����u�ڑ����� ***/
				grachr( 1, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][0] );		// [07]	"�@�@�@�@�@�@�@�@�@�@�@���ڑ��@"
				grachr( 3, 20, 6, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"�@�@�@�@�@�@�@�@�@�@******�@�@"
				grachr( 4, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"�@�@�@�@�@�@�@�@�@�@�@****�@�@"
				grachr( 5, 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF,  &SMSTR1[7][24] );		// [07]	"�@�@�@�@�@�@�@�@�@�@�@****�@�@"
			}
			page[1] = page[0];
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ��/ۯ����u���춳�ĸر�m�F��ʏ���                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_LkMcntClr( kind, s_no  )                         |*/
/*|                                                                        |*/
/*| PARAMETER    : short kind�F���춳�Ď��                               |*/
/*|                  FCNT_FLAP=�ׯ�ߓ��춳��                               |*/
/*|                  FCNT_ROCK=ۯ����u���춳��                             |*/
/*|                                                                        |*/
/*|                ushort s_no�F���uNo.                                    |*/
/*|                  0    =�S���춳�ĸر                                   |*/
/*|                  0�ȊO=�ʓ��춳�ĸر                                 |*/
/*|                                                                        |*/
/*| RETURN VALUE : ushort ����Ӱ��(MOD_CHG/MOD_EXT)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	SysMnt_LkMcntClr( short kind, ushort s_no, char ctype  )
{
	ushort	ret;
	ushort	line;
	short	msg;
	ushort	s_no_wk;
	char	type;
	short	search_max;

	for( line = 1 ; line < 6 ; line++ ){
		displclr( line );					// ���춳�ĕ\���ر
	}
	if( s_no != 0 ){
		// �ʓ��춳�ĸر
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[76] );	// "�@���u�@�@�@�̓���J�E���g���@"

		if( kind == FCNT_ROCK ){
			// ۯ����u�̏ꍇ�A�\�����鑕�uNo.��ϊ��i101�`150 �� 1�`50�j
			opedsp( 2, 6, (ushort)(s_no-BIKE_START_INDEX), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// ���uNo.�\��
			s_no_wk = s_no-BIKE_START_INDEX-1;											// s_no_wk�����춳���ް��z��No.(0�`49)
		}
		else{
			if( ctype == _MTYPE_INT_FLAP )
				opedsp( 2, 6, (ushort)(s_no-INT_CAR_START_INDEX), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ���uNo.�\��
			else
			opedsp( 2, 6, s_no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ���uNo.�\��
			s_no_wk = s_no-1;												// s_no_wk�����춳���ް��z��No.(0�`49)
		}
	}
	else{
		// �S���춳�ĸر
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[77] );	// "�@�S���u�̓���J�E���g���@�@�@"
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[78] );		// "�@�N���A���܂����H�@�@�@�@�@�@"

	Fun_Dsp( FUNMSG[19] );					// "�@�@�@�@�@�@ �͂� �������@�@�@"

	for( ret = 0 ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// �L�[���͑҂�

		switch( msg ){						// �L�[���͂ɂ��e�����֕���
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ���췰�ؑ�
				ret = MOD_CHG;
				break;

			case KEY_TEN_F3:				// F3:"�͂�"
				BUZPI();
				ret = MOD_EXT;
				MovCntClr = ON;				// ���춳�ĸر���s��ԂɗL��āi����۸ޓo�^�p�j

				if( kind == FCNT_FLAP ){
					// ���u��ʁ��t���b�v
					if( s_no != 0 ){
						// �ʓ��춳�ĸر
						MntFlapDoCount[s_no_wk][0] = 0;							// ����񐔸ر
						MntFlapDoCount[s_no_wk][1] = 0;							// �蓮����񐔸ر
						MntFlapDoCount[s_no_wk][2] = 0;							// �̏�񐔸ر
						queset( FLPTCBNO, LK_SND_CNT, sizeof(s_no), &s_no );	// ���춳����ر�i�w��j�v�����M
					}
					else{
						// �S���춳�ĸر
						if( ctype & _MTYPE_INT_FLAP ){
							search_max = INT_FLAP_END_NO;
							if( ctype == _MTYPE_INT_FLAP )
								s_no_wk = INT_CAR_START_INDEX;
							else
								s_no_wk = 0;
						}else{
							search_max = CAR_LOCK_MAX;
							s_no_wk = 0;
						}
						for( ; s_no_wk<search_max ; s_no_wk++ ){	// �ׯ�ߑ��u����ٰ��
							if( MntFlapDoCount[s_no_wk][0] != 0xffffffff ){
								// �ڑ�����Ă����ׯ�ߑ��u�̓��춳���ޯ̧��0�ر����
								MntFlapDoCount[s_no_wk][0] = 0;					// ����񐔸ر
								MntFlapDoCount[s_no_wk][1] = 0;					// �蓮����񐔸ر
								MntFlapDoCount[s_no_wk][2] = 0;					// �̏�񐔸ر
							}
						}
						type = (char)GetCarInfoParam();
						switch( type&0x06 ){
							case	0x04:
								type = _MTYPE_INT_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ����ݽ���v�����M
								break;
							
							case	0x06:
								type = _MTYPE_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ����ݽ���v�����M
								type = _MTYPE_INT_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ����ݽ���v�����M
								break;
							case	0x02:
							default:
								type = _MTYPE_FLAP;
								queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );			// ����ݽ���v�����M
								break;
						}
					}
				}
				else if( kind == FCNT_ROCK ){
					// ���u��ʁ����b�N���u
					if( s_no != 0 ){
						// �ʓ��춳�ĸر
						MntLockDoCount[s_no_wk][0] = 0;							// ����񐔸ر
						MntLockDoCount[s_no_wk][1] = 0;							// �蓮����񐔸ر
						MntLockDoCount[s_no_wk][2] = 0;							// �̏�񐔸ر
						queset( FLPTCBNO, LK_SND_CNT, sizeof(s_no), &s_no );	// ���춳����ر�i�w��j�v�����M
					}
					else{
						// �S���춳�ĸر
						for( s_no_wk=0 ; s_no_wk<BIKE_LOCK_MAX ; s_no_wk++ ){	// ۯ����u����ٰ��
							WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
							if( MntLockDoCount[s_no_wk][0] != 0xffffffff ){
								// �ڑ�����Ă���ۯ����u�̓��춳���ޯ̧��0�ر����
								MntLockDoCount[s_no_wk][0] = 0;					// ����񐔸ر
								MntLockDoCount[s_no_wk][1] = 0;					// �蓮����񐔸ر
								MntLockDoCount[s_no_wk][2] = 0;					// �̏�񐔸ر
							}
						}
						type = _MTYPE_LOCK;
						queset( FLPTCBNO, LK_SND_A_CNT, 1, &type );		// ���춳����ر�i�S�āj�v�����M
					}
				}
				break;

			case KEY_TEN_F4:				// F4:"������"
				BUZPI();
				ret = MOD_EXT;
				break;
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�e��۸ޏ��i�����j�󎚑��쏈��										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 ����Ӱ��(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMnt_Log( void )
{
	ushort	usSysEvent;
	char	wk[2];
	ushort	NewOldDate[6];		// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
	ushort	LogCount;			// LOG�o�^����
	date_time_rec	NewestDateTime, OldestDateTime;


	DP_CP[0] = DP_CP[1] = 0;


	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[48] );			// [48]	"�����O�t�@�C���v�����g���@�@�@"

// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// 		usSysEvent = Menu_Slt( LOGMENU, LOG_FILE_TBL, (char)LOG_MENU_MAX, (char)1 );
		if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
			usSysEvent = Menu_Slt(LOGMENU_CC, LOG_FILE_TBL_CC, (char)LOG_MENU_MAX_CC, (char)1);
		} else {					// �ʏ헿���v�Z���[�h
			usSysEvent = Menu_Slt(LOGMENU, LOG_FILE_TBL, (char)LOG_MENU_MAX, (char)1);
		}
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usSysEvent ){

			case	ERR_LOG:	// �P�F�G���[���
				Ope2_Log_NewestOldestDateGet( eLOG_ERROR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_ERROR,
													LogCount,
													LOGSTR1[0],
													PREQ_ERR_LOG,
													&NewOldDate[0]
												);
				break;

			case	ARM_LOG:	// �Q�F�A���[�����
				Ope2_Log_NewestOldestDateGet( eLOG_ALARM, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_ALARM,
													LogCount,
													LOGSTR1[1],
													PREQ_ARM_LOG,
													&NewOldDate[0]
												);
				break;

			case	TGOU_LOG:	// �R�F�s���v���
				Ope2_Log_NewestOldestDateGet( eLOG_TTOTAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_TTOTAL,
													LogCount,
													LOGSTR1[2],
													PREQ_TGOUKEI_J,
													&NewOldDate[0]
												);
				break;

			case	GTGOU_LOG:	// �S�F�f�s���v���
				Ope2_Log_NewestOldestDateGet( eLOG_GTTOTAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_GTTOTAL,
													LogCount,
													LOGSTR1[15],
													PREQ_GTGOUKEI_J,
													&NewOldDate[0]
												);
				break;

			case	COIN_LOG:	// �S�F�R�C�����ɏ��

			 	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_COINBOX, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_COINBOX,
													LogCount,
													LOGSTR1[3],
													PREQ_COKINKO_J,
													&NewOldDate[0]
												);
				break;

			case	NOTE_LOG:	// �T�F�������ɏ��

			 	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_NOTEBOX, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_NOTEBOX,
													LogCount,
													LOGSTR1[4],
													PREQ_SIKINKO_J,
													&NewOldDate[0]
												);
				break;

			case	TURI_LOG:	// �U�F�ޑK�Ǘ����
			 	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_MNYMNG_SRAM, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_MNYMNG_SRAM,
													LogCount,
													LOGSTR1[5],
													PREQ_TURIKAN_LOG,
													&NewOldDate[0]
												);
				break;

			case	SEISAN_LOG:	// �V�F�ʐ��Z���

			 	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_PAYMENT, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_PAYMENT,
													LogCount,
													LOGSTR1[6],
													PREQ_KOBETUSEISAN,
													&NewOldDate[0]
												);
				break;
//���g�p�ł��iS�j
			case	FUSKYO_LOG:	// �W�F�s���E�����o�ɏ��

			 	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_ABNORMAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );


				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_ABNORMAL,
													LogCount,
													LOGSTR1[7],
													PREQ_FUSKYO_LOG,
													&NewOldDate[0]
												);
				break;
//���g�p�ł��iE�j
			case	OPE_LOG:	// �P�O�F������
				Ope2_Log_NewestOldestDateGet( eLOG_OPERATE, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_OPERATE,
													LogCount,
													LOGSTR1[9],
													PREQ_OPE_LOG,
													&NewOldDate[0]
												);
				break;

			case	TEIFUK_LOG:	// �P�P�F�╜�d���
				Ope2_Log_NewestOldestDateGet( eLOG_POWERON, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_POWERON,
													LogCount,
													LOGSTR1[10],
													PREQ_TEIFUK_LOG,
													&NewOldDate[0]
												);
				break;

			case	MONI_LOG:	// �P�Q�F���j�^���
				Ope2_Log_NewestOldestDateGet( eLOG_MONITOR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_MONITOR,
													LogCount,
													LOGSTR1[11],
													PREQ_MON_LOG,
													&NewOldDate[0]
												);
				break;

			case	ADDPRM_LOG:	// �P�R�F�ݒ�X�V���
				usSysEvent = SysMnt_Log_CntDsp1	(	//	���O�����\����ʏ����P�i�Ώۊ��Ԏw��Ȃ��j
													ADDPRM_LOG,
													(ushort)SetDiffLogBuff.Header.UseInfoCnt,
													LOGSTR1[12],
													SETDIFFLOG_PRINT
												);
				break;
			case	RTPAY_LOG:	// �P�S�F�����e�X�g���
				usSysEvent = SysMnt_Log_CntDsp1	(	//	���O�����\����ʏ����P�i�Ώۊ��Ԏw��Ȃ��j
													LOG_RTPAYMENT,
													(ushort)RT_PAY_LOG_DAT.RT_pay_count,
													LOGSTR1[14],
													PREQ_RT_PAYMENT_LOG
												);
				break;
			case	RMON_LOG:	// �P�T�F���u�Ď����
				Ope2_Log_NewestOldestDateGet( eLOG_REMOTE_MONITOR, &NewestDateTime, &OldestDateTime, &LogCount );
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				usSysEvent = SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
													LOG_REMOTE_MONITOR,
													LogCount,
													LOGSTR1[16],
													PREQ_RMON_LOG,
													&NewOldDate[0]
												);
				break;
			case	CLEAR_LOG:	// �P�U�F�S���O�t�@�C���N���A

				usSysEvent = SysMnt_LogClr();		// ���O�t�@�C���N���A����
				break;

			//Return Status of User Menu
			case MOD_EXT:
				//OPECTL.Mnt_mod = 1;

				return(usSysEvent);
				break;

			default:
				break;
		}
		//Return Status of User Operation
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usSysEvent == MOD_CHG ){
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return(usSysEvent);
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���O�����\����ʏ����P�i�󎚑Ώۊ��Ԏw��Ȃ��j						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp1( LogSyu,LogCnt,*Title,PreqCmd )	|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	:	���O���						|*/
/*|																			|*/
/*|						ushort	LogCnt	:	���O����						|*/
/*|																			|*/
/*|						ushort	*Title	:	�^�C�g���\���f�[�^�|�C���^		|*/
/*|																			|*/
/*|						ushort	PreqCmd	:	�󎚗v��ү���޺����				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 ����Ӱ��(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMnt_Log_CntDsp1( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd )
{
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	ushort			pri_cmd = 0;		// �󎚗v������ފi�[ܰ�
	T_FrmLogPriReq1	FrmLogPriReq1;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
	uchar			end_flg;			// �N���A�����ʏI���׸�


	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );			// ���O���v�����g�^�C�g���\��
	LogCntDsp( LogCnt );													// ���O�����\��
	Fun_Dsp( FUNMSG[85] );													// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		if( pri_cmd == 0 ){

			// �󎚗v���O�i�󎚗v���O��ʁj
			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�

					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				case KEY_TEN_F2:							// �e�Q�i�N���A�j�L�[����

					BUZPI();
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);		// "�@�@���O���N���A���܂����H�@�@"
					Fun_Dsp(FUNMSG[19]);													// "�@�@�@�@�@�@ �͂� �������@�@�@"

					for( end_flg = OFF ; end_flg == OFF ; ){
						RcvMsg = StoF( GetMessage(), 1 );			// �C�x���g�҂�

						switch( RcvMsg ){							// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							case LCD_DISCONNECT:
								RcvMsg = MOD_CUT;					// �O��ʂɖ߂�
								end_flg = ON;

								break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							case KEY_MODECHG:						// �ݒ�L�[�ؑ�

								RcvMsg = MOD_CHG;					// �O��ʂɖ߂�
								end_flg = ON;

								break;

							case KEY_TEN_F3:						// �e�R�i�͂��j�L�[����

								BUZPI();
								if( LogDataClr( LogSyu ) == OK ){	// ���O�f�[�^�N���A
									LogCnt = 0;						// ���O�����i�������Ұ��j�N���A
									LogCntDsp( LogCnt );			// ���O�����\���i�O���j
								}
								end_flg = ON;

								break;

							case KEY_TEN_F4:						// �e�S�i�������j�L�[����

								BUZPI();
								end_flg = ON;

								break;
						}
						if( end_flg != OFF ){
							displclr( 6 );							// "�@�@���O���N���A���܂����H�@�@"�\���s�N���A
							Fun_Dsp( FUNMSG[85] );					// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
						}
					}

					break;

				case KEY_TEN_F4:							// �e�S�i���s�j�L�[����
					if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					/*------	�󎚗v��ү���ޑ��M	-----*/
					FrmLogPriReq1.prn_kind = R_PRI;											// �Ώ�������Fڼ��
					FrmLogPriReq1.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// �@�B��	�F�ݒ��ް�
					FrmLogPriReq1.Kakari_no = OPECTL.Kakari_Num;							// �W��No.
					memcpy( &FrmLogPriReq1.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// �󎚎���	�F���ݎ���
					queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq1), &FrmLogPriReq1 );

					pri_cmd = PreqCmd;						// ���M����޾���

					Fun_Dsp( FUNMSG[82] );					// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

					break;

				case KEY_TEN_F5:							// �e�T�i�I���j�L�[����

					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
			}

			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();

					/*------	�󎚒��~ү���ޑ��M	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// �O��ʂɖ߂�
		}
	}
	return( RcvMsg );
}

//[]----------------------------------------------------------------------[]
///	@brief		Log��ʂ�eLog��ʂɕϊ�����
//[]----------------------------------------------------------------------[]
///	@param[in]	LogSyu		: Log���
///	@return		eLogSyu		: eLog���
//[]----------------------------------------------------------------------[]
///	@author		m.nagashima
///	@date		Create	: 2012/03/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Conv_to_eLogSyu( ushort LogSyu )
{
	ushort	eLogSyu = 0xffff;

	switch( LogSyu ){
		case LOG_PAYMENT:
			eLogSyu = eLOG_PAYMENT;			// ���Z
			break;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//		case LOG_ENTER:
//			eLogSyu = eLOG_ENTER;			// ����
//			break;
		case LOG_RTPAY:
			eLogSyu = eLOG_RTPAY;			// RT���Z�f�[�^
			break;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case LOG_RTRECEIPT:
			eLogSyu = eLOG_RTRECEIPT;		// RT�̎��؃f�[�^
			break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case LOG_TTOTAL:
			eLogSyu = eLOG_TTOTAL;			// �W�v
			break;
		case LOG_ERROR:
			eLogSyu = eLOG_ERROR;			// �G���[
			break;
		case LOG_ALARM:
			eLogSyu = eLOG_ALARM;			// �A���[��
			break;
		case LOG_OPERATE:
			eLogSyu = eLOG_OPERATE;			// ����
			break;
		case LOG_MONITOR:
			eLogSyu = eLOG_MONITOR;			// ���j�^
			break;
		case LOG_ABNORMAL:
			eLogSyu = eLOG_ABNORMAL;		// �s�������o��
			break;
		case LOG_MONEYMANAGE:
			eLogSyu = eLOG_MONEYMANAGE;		// �ޑK�Ǘ�
			break;
		case LOG_PARKING:
			eLogSyu = eLOG_PARKING;			// ���ԑ䐔�f�[�^
			break;
		case LOG_NGLOG:
			eLogSyu = eLOG_NGLOG;			// �s�����
			break;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//		case LOG_IOLOG:
//			eLogSyu = eLOG_IOLOG;			// ���o�ɗ���
		case LOG_DC_QR:
			eLogSyu = eLOG_DC_QR;			// QR�m��E����f�[�^
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
			break;
		case LOG_CREUSE:
			eLogSyu = eLOG_CREUSE;			// �N���W�b�g���p
			break;
		case LOG_HOJIN_USE:
			eLogSyu = eLOG_HOJIN_USE;		// �@�l�J�[�h���p
			break;
		case LOG_REMOTE_SET:
			eLogSyu = eLOG_REMOTE_SET;		// ���u�����ݒ�
			break;
		case LOG_LONGPARK:
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//			eLogSyu = eLOG_LONGPARK;		// ��������
			eLogSyu = eLOG_LONGPARK_PWEB;	// ��������(ParkingWeb�p)
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
			break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		case LOG_RISMEVENT:
//			eLogSyu = eLOG_RISMEVENT;		// RISM�C�x���g
		case LOG_DC_LANE:
			eLogSyu = eLOG_DC_LANE;			// ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			break;
		case LOG_GTTOTAL:
			eLogSyu = eLOG_GTTOTAL;			// GT�W�v
			break;
		case LOG_COINBOX:
			eLogSyu = eLOG_COINBOX;			// �R�C�����ɏW�v(ram�̂�)
			break;
		case LOG_NOTEBOX:
			eLogSyu = eLOG_NOTEBOX;			// �������ɏW�v(ram�̂�)
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case LOG_EDYARM:
//			eLogSyu = eLOG_EDYARM;			// Edy�A���[��(ram�̂�)
//			break;
//		case LOG_EDYSHIME:
//			eLogSyu = eLOG_EDYSHIME;		// Edy����(ram�̂�)
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case LOG_POWERON:
			eLogSyu = eLOG_POWERON;			// ���d(ram�̂�)
			break;
		case LOG_MNYMNG_SRAM:
			eLogSyu = eLOG_MNYMNG_SRAM;		// ���d(ram�̂�)
			break;
		case LOG_REMOTE_MONITOR:
			eLogSyu = eLOG_REMOTE_MONITOR;	// ���u�Ď�
			break;
		default :
			break;
	}

	return eLogSyu;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���O�����\����ʏ����Q�i�󎚑Ώۊ��Ԏw�肠��j						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp2( LogCnt, *Title, PreqCmd )		|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu		:	���O���					|*/
/*|																			|*/
/*|						ushort	LogCnt		:	���O����					|*/
/*|																			|*/
/*|						ushort	*Title		:	�^�C�g���\���f�[�^�|�C���^	|*/
/*|																			|*/
/*|						ushort	PreqCmd		:	�󎚗v��ү���޺����			|*/
/*|																			|*/
/*|						ushort	*NewOldDate	:	�ŌÁ��ŐV�ް����t�ް��߲��	|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 ����Ӱ��(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// �\�����ږ��̕\�����@�ް�ð��قP�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA1[6][4] = {
	{ 4, 12, 2 ,1 },	// �J�n�i�N�j
	{ 4, 18, 2 ,0 },	// �J�n�i���j
	{ 4, 24, 2, 0 },	// �J�n�i���j
	{ 5, 12, 2, 1 },	// �I���i�N�j
	{ 5, 18, 2, 0 }, 	// �I���i���j
	{ 5, 24, 2, 0 }		// �I���i���j
};
const	ushort	POS_DATA1_0[3][4] = {
	{ 5, 12, 2, 1 },	// ����i�N�j
	{ 5, 18, 2, 0 },	// ����i���j
	{ 5, 24, 2, 0 },	// ����i���j
};

ushort	SysMnt_Log_CntDsp2( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate )
{
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	ushort			pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
	T_FrmLogPriReq2	FrmLogPriReq2;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
	ushort			Date_Now[6];		// ���t�ް��i[0]�F�J�n�N�A[1]�F�J�n���A[2]�F�J�n���A[3]�F�I���N�A[4]�F�I�����A[5]�F�I�����j
	uchar			pos		= 0;		// �����ް����͈ʒu�i�O�F�J�n�N�A�P�F�J�n���A�Q�F�J�n���A�R�F�I���N�A�S�F�I�����A�T�F�I�����j
	short			inp		= -1;		// �����ް�
	uchar			All_Req	= OFF;		// �u�S�āv�w�蒆�׸�
	uchar			Date_Chk;			// ���t�w���ް������׸�
	uchar			end_flg;			// �N���A�����ʏI���׸�
	ushort			wks;
	ushort			Date_Now_Prev[6];	// �O��܂ł̔N����
	ushort			eLogSyu;			// ���O���
	ushort			index;				// ���O�����ʒu

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );							// ���O���v�����g�^�C�g���\��
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// �ŌÁ^�ŐV���O���t�\��
	LogCntDsp( LogCnt );									// ���O�����\��

	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );
	memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

	LogDateDsp4( &Date_Now[0] );							// �ŌÁ^�ŐV���O���t�f�[�^�\��
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
	LogDateDsp5( &Date_Now[0], pos );						// ������t�f�[�^�\��

	Fun_Dsp( FUNMSG[83] );									// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "

	eLogSyu = Conv_to_eLogSyu( LogSyu );					// Log��ʁ�eLog��ʕϊ�
	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		if( pri_cmd == 0 ){

			// �󎚗v���O�i�󎚗v���O��ʁj

			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�

					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				case KEY_TEN:								// �O�`�X

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// �����ް��\��
									POS_DATA1_0[pos][0],	// �\���s
									POS_DATA1_0[pos][1],	// �\�����
									(ushort)inp,			// �\���ް�
									POS_DATA1_0[pos][2],	// �ް�����
									POS_DATA1_0[pos][3],	// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// ����L�[

					BUZPI();

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						opedsp	(							// ���͑O���ް���\��������
									POS_DATA1_0[pos][0],	// �\���s
									POS_DATA1_0[pos][1],	// �\�����
									Date_Now[pos],			// �\���ް�
									POS_DATA1_0[pos][2],	// �ް�����
									POS_DATA1_0[pos][3],	// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// ���͏�ԏ�����
					}
					else{
						//	�u�S�āv�w�蒆�̏ꍇ

						pos = 0;							// ���وʒu���J�n�i�N�j
						inp = -1;							// ���͏�ԏ�����
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
						LogDateDsp5( &Date_Now[0], pos );						// ������t�f�[�^�\��
						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
					}

					break;

				case KEY_TEN_F1:							// �e�P�i���j�L�[����
				case KEY_TEN_F2:							// �e�Q�i���j�L�[����	�����t���͒��̏ꍇ
															// �e�Q�i�N���A�j����	���u�S�āv�w�蒆�̏ꍇ

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						if( inp == -1 ){						// ���͂���H

							//	���͂Ȃ��̏ꍇ
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
							}
							else{
								LogDatePosUp( &pos, 1 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
							}
						}
						else{
							//	���͂���̏ꍇ
							if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�H

								//	�����ް��n�j�̏ꍇ
								BUZPI();

								if( pos == 0 ){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
								}
								else{
									LogDatePosUp( &pos, 1 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
								}
							}
							else{
								//	�����ް��m�f�̏ꍇ
								BUZPIPI();
							}
						}
						/* �w��N�����͈͓����ް����ĕ\�� */
						LogDateDsp5( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
						inp = -1;								// ���͏�ԏ�����
					}
					else if( RcvMsg == KEY_TEN_F2 ){

						// �u�S�āv�w�蒆�̂e�Q�i�N���A�j�L�[

						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "�@�@���O���N���A���܂����H�@�@"
						Fun_Dsp(FUNMSG[19]);							// "�@�@�@�@�@�@ �͂� �������@�@�@"

						for( end_flg = OFF ; end_flg == OFF ; ){
							RcvMsg = StoF( GetMessage(), 1 );			// �C�x���g�҂�

							switch( RcvMsg ){							// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
								case LCD_DISCONNECT:					// �ؒf�ʒm
									RcvMsg = MOD_CUT;					// �O��ʂɖ߂�
									end_flg = ON;

									break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
								case KEY_MODECHG:						// �ݒ�L�[�ؑ�

									RcvMsg = MOD_CHG;					// �O��ʂɖ߂�
									end_flg = ON;

									break;

								case KEY_TEN_F3:						// �e�R�i�͂��j�L�[����

									BUZPI();
									if( LogDataClr( LogSyu ) == OK ){	// ���O�f�[�^�N���A
										LogCnt = 0;						// ���O�����i�������Ұ��j�N���A
										/* �f�[�^�Ȃ����͈͓̔��t�͍����ɂ��� */
										NewOldDate[0] = NewOldDate[3] = (ushort)CLK_REC.year;		// �J�n�A�I���i�N�j�����ݓ����i�N�j
										NewOldDate[1] = NewOldDate[4] = (ushort)CLK_REC.mont;		// �J�n�A�I���i���j�����ݓ����i���j
										NewOldDate[2] = NewOldDate[5] = (ushort)CLK_REC.date;		// �J�n�A�I���i���j�����ݓ����i���j
										LogCntDsp( LogCnt );			// ���O�����\���i�O���j
										switch(LogSyu)					// �N���A���O�o�^����
										{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//										case LOG_EDYSYUUKEI:			// �d�����W�v�N���A
//											wopelg(OPLOG_EDYSYU_CLR,0,0);
//											break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
										case LOG_SCASYUUKEI:			// �r���������W�v�N���A
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
										case LOG_ECSYUUKEI:			// ���σ��[�_�W�v�N���A
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
											wopelg(OPLOG_SCASYU_CLR,0,0);
											break;
										default:						// ��L�ȊO(�o�^�Ώۂł͂Ȃ�)
											break;
										}
									}
									end_flg = ON;

									break;

								case KEY_TEN_F4:						// �e�S�i�������j�L�[����

									BUZPI();
									end_flg = ON;

									break;
							}
							if( end_flg != OFF ){
								displclr( 6 );										// "�@�@���O���N���A���܂����H�@�@"�\���s�N���A
								memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );	// ���t�\���͈͂�Max�Ƃ���
								LogCntDsp( LogCnt );								// ���O�����\��
								LogDateDsp4( &Date_Now[0] );						// �ŌÁ^�ŐV���O���t�f�[�^�\��
								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
								LogDateDsp5( &Date_Now[0], pos );					// ������t�f�[�^�\��
								Fun_Dsp( FUNMSG[83] );								// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
								All_Req = OFF;										// �u�S�āv�w�蒆�׸�ؾ��
							}
						}
					}

					break;

				case KEY_TEN_F3:							// �e�R�i�S�āj�L�[����

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						if( LogCnt != 0 ){

							//	���O�f�[�^������ꍇ
							BUZPI();
							memcpy	(							// �ŌÁ��ŐV�ް����t���߰
										&Date_Now[0],
										NewOldDate,
										12
									);
							pos = 0;							// ���وʒu���J�n�i�N�j
							inp = -1;							// ���͏�ԏ�����
							displclr( 5 );						// ������t�\���N���A
							LogCntDsp( LogCnt );				// ���O�����\��
							Fun_Dsp( FUNMSG[85] );				// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
							All_Req = ON;						// �u�S�āv�w�蒆�׸޾��
						}
						else{
							//	���O�f�[�^���Ȃ��ꍇ
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// �e�S�i���s�j�L�[����

					displclr( 1 );							// 1�s�ڕ\���N���A
					Date_Chk = OK;

					if( inp != -1 ){						// ���͂���H

						//	���͂���̏ꍇ
						if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�H

							//	���̓f�[�^�n�j�̏ꍇ
							if( pos == 0 ){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}
						else{
							//	���̓f�[�^�m�f�̏ꍇ
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��

							//	�N���������݂�����t���`�F�b�N����

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// �J�n���t����
								Date_Chk = NG;
							}
						}
					}

					if( Date_Chk == OK ){

						// �J�n���t���I�����t�����n�j�̏ꍇ
						wks = Ope2_Log_CountGet_inDate( eLogSyu, &Date_Now[0], &index );
						if( wks == 0 ){
							BUZPIPI();
							grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// �f�[�^�Ȃ��\��
							inp = -1;											// ���͏�ԏ�����
							break;
						}
						if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
							BUZPIPI();
							break;
						}
						BUZPI();

						/*------	�󎚗v��ү���ޑ��M	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// �󎚗v��ү����ܰ��O�ر
						FrmLogPriReq2.prn_kind	= SysMnt_Log_Printer(LogSyu);	// ��ޖ��ɑΏ�������𔻒f
						if(!FrmLogPriReq2.prn_kind)								// �擾�����v�����^��ʂ�0
						{
							RcvMsg = MOD_EXT;									// �O��ʂɖ߂�
							break;
						}
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// �������@	�F�S��
						}
						else{
							FrmLogPriReq2.BType	= 1;							// �������@	�F���t
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						FrmLogPriReq2.LogCount = LogCnt;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)

						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
						Ope_DisableDoorKnobChime();

						pri_cmd = PreqCmd;					// ���M����޾���
						if( All_Req != ON ){					// �u�S�āv�w��Ȃ�
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
							LogDateDsp5( &Date_Now[0], 0xff );	// ������t�f�[�^�\��
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// ���s���u�����N�\��
						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( LogSyu == LOG_EDYARM )
//							wopelg( OPLOG_EDY_ARM_PRN,0,0 );	// ���엚��o�^�iEdy����װя������āj
//						else if( LogSyu == LOG_EDYSHIME )
//							wopelg( OPLOG_EDY_SHIME_PRN,0,0 );	// ���엚��o�^�iEdy���ߋL�^�������āj
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

					}
					else{
						// �J�n���t���I�����t�����m�f�̏ꍇ
						BUZPIPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
						LogDateDsp5( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
						Fun_Dsp( FUNMSG[83] );					// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;							// �u�S�āv�w�蒆�׸�ؾ��
						inp = -1;								// ���͏�ԏ�����
					}

					break;

				case KEY_TEN_F5:							// �e�T�i�I���j�L�[����

					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
			}

			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:						// �ؒf�ʒm
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)s
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();

					/*------	�󎚒��~ү���ޑ��M	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
			break;											// �O��ʂɖ߂�
		}
	}
	return( RcvMsg );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�����\����ʏ����iSuica�����W�v�󎚗p�j							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_Log_CntDsp2( LogCnt, *Title, PreqCmd )		|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu		:	���O���					|*/
/*|																			|*/
/*|						ushort	LogCnt		:	���O����					|*/
/*|																			|*/
/*|						ushort	*Title		:	�^�C�g���\���f�[�^�|�C���^	|*/
/*|																			|*/
/*|						ushort	PreqCmd		:	�󎚗v��ү���޺����			|*/
/*|																			|*/
/*|						ushort	*NewOldDate	:	�ŌÁ��ŐV�ް����t�ް��߲��	|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 ����Ӱ��(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	DiditalCashe_Log_CntDsp( ushort LogSyu, ushort LogCnt, uchar *title, ushort PreqCmd, ushort *NewOldDate )
{
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	ushort			pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
	T_FrmLogPriReq2	FrmLogPriReq2;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
	ushort			Date_Now[6];		// ���t�ް��i[0]�F�J�n�N�A[1]�F�J�n���A[2]�F�J�n���A[3]�F�I���N�A[4]�F�I�����A[5]�F�I�����j
	uchar			pos		= 0;		// �����ް����͈ʒu�i�O�F�J�n�N�A�P�F�J�n���A�Q�F�J�n���A�R�F�I���N�A�S�F�I�����A�T�F�I�����j
	short			inp		= -1;		// �����ް�
	uchar			All_Req	= OFF;		// �u�S�āv�w�蒆�׸�
	uchar			Date_Chk;			// ���t�w���ް������׸�
	ushort			Sdate;				// �J�n���t
	ushort			Edate;				// �I�����t
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
	ushort			LogCntNow;			// ���݂̃��O��
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
	uchar			end_flg;			// �N���A�����ʏI���׸�
	ushort			Date_Now_Prev[6];	// �O��܂ł̔N����
	uchar			f_line_chg = 0;		// �J�[�\���ړ����̍s�ړ�����


	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,title );							// ���O���v�����g�^�C�g���\��
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "�@�@�ۗL���O�����F�@�@�@�@���@"
	opedsp( 2, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �����ް��\���i�S���A�O�T�v���X�j
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,LOGSTR2[1] );						// �J�n���t�\��
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,LOGSTR2[2] );						// �I�����t�\��

	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );
	memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
	LogCntNow = LogCnt;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j

	LogDateDsp( &Date_Now[0], pos );						// �J�n���t�ް����I�����t�ް��@�\��

	Fun_Dsp( FUNMSG[83] );									// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		if( pri_cmd == 0 ){

			// �󎚗v���O�i�󎚗v���O��ʁj

			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:						// �ؒf�ʒm
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�

					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				case KEY_TEN:								// �O�`�X

					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// �����ް��\��
									POS_DATA1[pos][0],		// �\���s
									POS_DATA1[pos][1],		// �\�����
									(ushort)inp,			// �\���ް�
									POS_DATA1[pos][2],		// �ް�����
									POS_DATA1[pos][3],		// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// ����L�[

					BUZPI();

					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						opedsp	(							// ���͑O���ް���\��������
									POS_DATA1[pos][0],		// �\���s
									POS_DATA1[pos][1],		// �\�����
									Date_Now[pos],			// �\���ް�
									POS_DATA1[pos][2],		// �ް�����
									POS_DATA1[pos][3],		// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// ���͏�ԏ�����
					}
					else{
						//	�u�S�āv�w�蒆�̏ꍇ

						pos = 0;							// ���وʒu���J�n�i�N�j
						inp = -1;							// ���͏�ԏ�����
						opedsp	(							// ���ق�\��������
									POS_DATA1[pos][0],		// �\���s
									POS_DATA1[pos][1],		// �\�����
									Date_Now[pos],			// �\���ް�
									POS_DATA1[pos][2],		// �ް�����
									POS_DATA1[pos][3],		// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
					}

					break;

				case KEY_TEN_F1:							// �e�P�i���j�L�[����
				case KEY_TEN_F2:							// �e�Q�i���j�L�[����	�����t���͒��̏ꍇ
															// �e�Q�i�N���A�j����	���u�S�āv�w�蒆�̏ꍇ

					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						if( inp == -1 ){						// ���͂���H

							//	���͂Ȃ��̏ꍇ
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								if( pos != 0 ){
									pos = (uchar)((pos) - 1);
								}
								else{
									pos = 5;
								}
								if((pos == 2) || (pos == 5))
								{
									f_line_chg = 1;				/* �s�ړ����� */
								}
							}
							else{
								pos = (uchar)((pos) + 1);
								if( pos >= 6 ){
									pos = 0;
								}
								if((pos == 0) || (pos == 3))
								{
									f_line_chg = 1;				/* �s�ړ��Ȃ� */
								}
							}
						}
						else{
							//	���͂���̏ꍇ
							if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�H

								//	�����ް��n�j�̏ꍇ
								BUZPI();

								if( pos == 0 || pos == 3 ){		// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
									Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] = (ushort)inp;
								}

								if( RcvMsg == KEY_TEN_F1 ){
									if( pos != 0 ){
										pos = (uchar)((pos) - 1);
									}
									else{
										pos = 5;
									}
									if((pos == 2) || (pos == 5))
									{
										f_line_chg = 1;				/* �s�ړ����� */
									}
								}
								else{
									pos = (uchar)((pos) + 1);
									if( pos >= 6 ){
										pos = 0;
									}
									if((pos == 0) || (pos == 3))
									{
										f_line_chg = 1;				/* �s�ړ��Ȃ� */
									}
								}
							}
							else{
								//	�����ް��m�f�̏ꍇ
								BUZPIPI();
							}
						}
						/* �w��N�����͈͓����ް����ĕ\�� */
						if( (f_line_chg) &&						// �s�ړ������@����
							(0 != memcmp( Date_Now_Prev, Date_Now, sizeof(Date_Now) ) ) )
						{										// ���܂łƔN�����l���قȂ�ꍇ
//							wks = Ope2_Log_CountGet_inDate( LogSyu, &Date_Now[3], &Date_Now[0] );
//							LogCntDsp( wks );		// ���O�����\��
							memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );

// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
							switch (LogSyu) {
							case LOG_ECSYUUKEI:
								Sdate = dnrmlzm(							// �J�n���t�ް��擾
													(short)Date_Now[0],
													(short)Date_Now[1],
													(short)Date_Now[2]
												);

								Edate = dnrmlzm(							// �I�����t�ް��擾
													(short)Date_Now[3],
													(short)Date_Now[4],
													(short)Date_Now[5]
												);
											
								LogCntNow = GetEcShuukeiCnt(Sdate, Edate);
								grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "�@�@�ۗL���O�����F�@�@�@�@���@"
								opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �����ް��\���i�S���A�O�T�v���X�j
								break;
							default:
								break;
							}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
						}
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
						f_line_chg = 0;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
						LogDateDsp( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
						inp = -1;								// ���͏�ԏ�����
					}
					else if( RcvMsg == KEY_TEN_F2 ){

						// �u�S�āv�w�蒆�̂e�Q�i�N���A�j�L�[

						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK,LCD_BLINK_OFF, SMSTR1[60]);				// "�@�@���O���N���A���܂����H�@�@"
						Fun_Dsp(FUNMSG[19]);							// "�@�@�@�@�@�@ �͂� �������@�@�@"

						for( end_flg = OFF ; end_flg == OFF ; ){
							RcvMsg = StoF( GetMessage(), 1 );			// �C�x���g�҂�

							switch( RcvMsg ){							// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
								case LCD_DISCONNECT:					// �ؒf�ʒm
									RcvMsg = MOD_CUT;					// �O��ʂɖ߂�
									end_flg = ON;

									break;								
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
								case KEY_MODECHG:						// �ݒ�L�[�ؑ�

									RcvMsg = MOD_CHG;					// �O��ʂɖ߂�
									end_flg = ON;

									break;

								case KEY_TEN_F3:						// �e�R�i�͂��j�L�[����

									BUZPI();
									if( LogDataClr( LogSyu ) == OK ){	// ���O�f�[�^�N���A
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
//										LogCnt = 0;						// ���O�����i�������Ұ��j�N���A
										LogCnt = LogCntNow = 0;			// ���O�����i�������Ұ��j�N���A
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
										/* �f�[�^�Ȃ����͈͓̔��t�͍����ɂ��� */
										NewOldDate[0] = NewOldDate[3] = (ushort)CLK_REC.year;		// �J�n�A�I���i�N�j�����ݓ����i�N�j
										NewOldDate[1] = NewOldDate[4] = (ushort)CLK_REC.mont;		// �J�n�A�I���i���j�����ݓ����i���j
										NewOldDate[2] = NewOldDate[5] = (ushort)CLK_REC.date;		// �J�n�A�I���i���j�����ݓ����i���j
										grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "�@�@�ۗL���O�����F�@�@�@�@���@"
										opedsp( 2, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �����ް��\���i�S���A�O�T�v���X�j
										switch(LogSyu)					// �N���A���O�o�^����
										{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//										case LOG_EDYSYUUKEI:			// �d�����W�v�N���A
//											wopelg(OPLOG_EDYSYU_CLR,0,0);
//											break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
										case LOG_SCASYUUKEI:			// �r���������W�v�N���A
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
										case LOG_ECSYUUKEI:				// ���σ��[�_�W�v�N���A
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
											wopelg(OPLOG_SCASYU_CLR,0,0);
											break;
										default:						// ��L�ȊO(�o�^�Ώۂł͂Ȃ�)
											break;
										}
									}
									end_flg = ON;

									break;

								case KEY_TEN_F4:						// �e�S�i�������j�L�[����

									BUZPI();
									end_flg = ON;

									break;
							}
							if( end_flg != OFF ){
								displclr( 6 );										// "�@�@���O���N���A���܂����H�@�@"�\���s�N���A
								memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );	// ���t�\���͈͂�Max�Ƃ���
//								LogCntDsp( LogCnt );								// ���O�����\��
								LogDateDsp( &Date_Now[0], pos );					// �J�n���t�ް����I�����t�ް��@�\��
								Fun_Dsp( FUNMSG[83] );								// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
								All_Req = OFF;										// �u�S�āv�w�蒆�׸�ؾ��
							}
						}
					}

					break;

				case KEY_TEN_F3:							// �e�R�i�S�āj�L�[����

					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						if( LogCnt != 0 ){

							//	���O�f�[�^������ꍇ
							BUZPI();
							memcpy	(							// �ŌÁ��ŐV�ް����t���߰
										&Date_Now[0],
										NewOldDate,
										12
									);
							pos = 0;							// ���وʒu���J�n�i�N�j
							inp = -1;							// ���͏�ԏ�����
							LogDateDsp( &Date_Now[0], 0xff );	// �J�n���t�ް����I�����t�ް��@�\���i���ٕ\���Ȃ��j
//							LogCntDsp( LogCnt );				// ���O�����\��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
							switch (LogSyu) {
							case LOG_ECSYUUKEI:
								LogCntNow = LogCnt;
								grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "�@�@�ۗL���O�����F�@�@�@�@���@"
								opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �����ް��\���i�S���A�O�T�v���X�j
								break;
							default:
								break;
							}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
							Fun_Dsp( FUNMSG[85] );				// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
							All_Req = ON;						// �u�S�āv�w�蒆�׸޾��
						}
						else{
							//	���O�f�[�^���Ȃ��ꍇ
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// �e�S�i���s�j�L�[����

					Date_Chk = OK;

					if( inp != -1 ){						// ���͂���H

						//	���͂���̏ꍇ
						if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�H

							//	���̓f�[�^�n�j�̏ꍇ
							if( pos == 0 || pos == 3 ){		// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
								Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] = (ushort)inp;
							}
						}
						else{
							//	���̓f�[�^�m�f�̏ꍇ
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��

							//	�N���������݂�����t���`�F�b�N����

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// �J�n���t����
								Date_Chk = NG;
							}
							if( chkdate( (short)Date_Now[3], (short)Date_Now[4], (short)Date_Now[5] ) != 0 ){	// �I�����t����
								Date_Chk = NG;
							}
						}
					}
					if( Date_Chk == OK ){

						//	�J�n���t�����I�����t���`�F�b�N����

						Sdate = dnrmlzm(							// �J�n���t�ް��擾
											(short)Date_Now[0],
											(short)Date_Now[1],
											(short)Date_Now[2]
										);

						Edate = dnrmlzm(							// �I�����t�ް��擾
											(short)Date_Now[3],
											(short)Date_Now[4],
											(short)Date_Now[5]
										);

						if( Sdate > Edate ){						// �J�n���t�^�I�����t����
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						// �J�n���t���I�����t�����n�j�̏ꍇ

						// ���͂���Ό����ĕ\��
						if( inp != -1 ){						// ���͂���H
							if( 0 != memcmp( Date_Now_Prev, Date_Now, sizeof(Date_Now) ) )
							{									// ���܂łƔN�����l���قȂ�ꍇ
//								wks = Ope2_Log_CountGet_inDate( LogSyu, &Date_Now[3], &Date_Now[0] );
//								LogCntDsp( wks );		// ���O�����\��
								memcpy( Date_Now_Prev, Date_Now, sizeof(Date_Now) );
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
								switch (LogSyu) {
								case LOG_ECSYUUKEI:
									LogCntNow = GetEcShuukeiCnt(Sdate, Edate);
									grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );			// "�@�@�ۗL���O�����F�@�@�@�@���@"
									opedsp( 2, 18, LogCntNow, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �����ް��\���i�S���A�O�T�v���X�j
									break;
								default:
									break;
								}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�}���`�d�q�}�l�[���[�_�Ή��ڐA�j
							}
						}
						if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
							BUZPIPI();
							break;
						}
						BUZPI();

						/*------	�󎚗v��ү���ޑ��M	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// �󎚗v��ү����ܰ��O�ر
						FrmLogPriReq2.prn_kind	= SysMnt_Log_Printer(LogSyu);	// ��ޖ��ɑΏ�������𔻒f
						if(!FrmLogPriReq2.prn_kind)								// �擾�����v�����^��ʂ�0
						{
							RcvMsg = MOD_EXT;									// �O��ʂɖ߂�
							break;
						}
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// �������@	�F�S��
						}
						else if( Sdate == Edate ){
							FrmLogPriReq2.BType	= 1;							// �������@	�F���t
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						else{
							FrmLogPriReq2.BType	= 2;							// �������@	�F����
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
							FrmLogPriReq2.TEndTime.Year	= Date_Now[3];
							FrmLogPriReq2.TEndTime.Mon	= (uchar)Date_Now[4];
							FrmLogPriReq2.TEndTime.Day	= (uchar)Date_Now[5];
						}
						FrmLogPriReq2.LogCount = LogCnt;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)

						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );

						pri_cmd = PreqCmd;					// ���M����޾���
						LogDateDsp( &Date_Now[0], 0xff );	// �J�n���t�ް����I�����t�ް��@�\���i���ٕ\���������j
						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

					}
					else{
						// �J�n���t���I�����t�����m�f�̏ꍇ
						BUZPIPI();
						LogDateDsp( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
						inp = -1;								// ���͏�ԏ�����
					}

					break;

				case KEY_TEN_F5:							// �e�T�i�I���j�L�[����

					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
			}

			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:						// �ؒf�ʒm
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();

					/*------	�󎚒��~ү���ޑ��M	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// �O��ʂɖ߂�
		}
	}
	return( RcvMsg );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�t�@�C���N���A�i�S�āj���쏈��									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMnt_LogClr( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	 ����Ӱ��(MOD_CHG/MOD_EXT)					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//#define		CLR_LOG_CNT		13		// ���O�t�@�C����ʐ�
#define		CLR_LOG_CNT		14		// ���O�t�@�C����ʐ�
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)

const	ushort	LogSyuTbl[CLR_LOG_CNT] = {
	LOG_ERROR,		// �G���[���
	LOG_ALARM,		// �A���[�����
	LOG_OPERATE,	// ������
	LOG_POWERON,	// �╜�d���
	LOG_TTOTAL,		// �s���v���
	LOG_COINBOX,	// �R�C�����ɏ��
	LOG_NOTEBOX,	// �������ɏ��
	LOG_MNYMNG_SRAM,// �ޑK�Ǘ����(SRAM)
	LOG_PAYMENT,	// �ʐ��Z���
	LOG_ABNORMAL,	// �s���E�����o�ɏ��
	LOG_MONITOR,	// ���j�^���
	ADDPRM_LOG,		// �ݒ�X�V����
	LOG_RTPAYMENT,	// �����e�X�g���
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
	LOG_LONGPARK,	// �������ԃf�[�^(ParkingWeb�p)
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
};

ushort	SysMnt_LogClr( void )
{
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	uchar			i;					// ۸�̧�ٸرٰ�ߏ��������
	uchar			Clr_Ope = OFF;		// �ر�����׸�

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[59] );						// "�����O�t�@�C���N���A���@�@�@�@"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[61]);						// "�@�S�Ẵ��O���N���A���܂����H"
	Fun_Dsp(FUNMSG[19]);									// "�@�@�@�@�@�@ �͂� �������@�@�@"

	for( ; ; ){
		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		switch( RcvMsg ){									// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:						// �ؒf�ʒm
				RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	// �ݒ�L�[�ؑ�

				RcvMsg = MOD_CHG;							// �O��ʂɖ߂�
				break;

			case KEY_TEN_F3:	// �e�R�i�͂��j�L�[����

				if( Clr_Ope == OFF ){						// �N���A����O�H

					BUZPI();

					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[62]);		// "�@�S�Ẵ��O���N���A���ł��B�@"
					Fun_Dsp(FUNMSG[0]);						// ̧ݸ��ݷ��\���N���A
					for(i = 0;i < eLOG_MAX; i++){
						FLT_LogErase2(i);
						Log_CheckBufferFull(FALSE, i, eLOG_TARGET_REMOTE);				// �o�b�t�@�t�������`�F�b�N
					}
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//					memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );// ���o�ɗ����̃��O�������ŃN���A����K�v������
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
					for( i = 0 ; i < CLR_LOG_CNT ; i++ ){	// ���O�t�@�C���S�N���A
						LogDataClr( LogSyuTbl[i] );
					}
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[63]);		// "�@�S�Ẵ��O���N���A���܂����B"
					Fun_Dsp(FUNMSG[8]);						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

					Clr_Ope = ON;							// �N���A����ς݃Z�b�g
				}
				break;

			case KEY_TEN_F4:	// �e�S�i�������j�L�[����

				if( Clr_Ope == OFF ){						// �N���A����O�H
					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
				}
				break;

			case KEY_TEN_F5:	// �e�T�i�I���j�L�[����

				if( Clr_Ope == ON ){						// �N���A�����H
					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
				}
				break;

			default:
				break;
		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			break;											// �O��ʂɖ߂�
		}
	}
	return( RcvMsg );
}


/*[]-----------------------------------------------------------------------[]*/
/*|		���O���t�ʒu�ԍ��X�V����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDatePosUp( uchar *pos, uchar req )				|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*pos= �ʒu�ԍ��ް��߲��						|*/
/*|																			|*/
/*|						uchar	req	= �����v��								|*/
/*|										�O�F�ʒu�ԍ��|�P�i���ړ��j			|*/
/*|										�P�F�ʒu�ԍ��{�P�i�E�ړ��j			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	LogDatePosUp( uchar *pos, uchar req )
{
	uchar ret = 0;					/* �s�ړ�����(0:�Ȃ�/1:����) */
	if( req == 0 ){

		// �ʒu�ԍ��|�P�i���ړ��j
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = 2;
		}
	}
	else{
		// �ʒu�ԍ��{�P�i�E�ړ��j
		*pos = (uchar)((*pos) + 1);
		if( *pos >= 3 ){
			*pos = 0;
		}
	}
	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O���t�f�[�^�`�F�b�N����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateChk( pos, data )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pos	= ���t�ʒu�ԍ��ް�						|*/
/*|																			|*/
/*|						short	data= ���t�ް�								|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret	= ��������(OK/NG)						|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// ���ږ����ް��͈������ް�ð��فi�ŏ��l�A�ő�l�j
const	short	LOG_DATE_DATA[6][2] = {
	{ 0, 99 },	// �J�n�i�N�j
	{ 1, 12 },	// �J�n�i���j
	{ 1, 31 },	// �J�n�i���j
	{ 0, 99 },	// �I���i�N�j
	{ 1, 12 },	// �I���i���j
	{ 1, 31 },	// �I���i���j
};

uchar	LogDateChk( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA[pos][0] && data <= LOG_DATE_DATA[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�����\������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp( LogCnt )									|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogCnt	:	���O����						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogCntDsp( ushort LogCnt )
{
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[1] );		// "�@�@�ۗL���O�����F�@�@�@�@���@"

	opedsp( 3, 18, LogCnt, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �����ް��\���i�S���A�O�T�v���X�j
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���O�����\�������Q													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp2( LogCnt )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogCnt1	:	�Ώۃ��O����					|*/
/*|					:	ushort	LogCnt2	:	�����O����						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	LogCntDsp2( ushort LogCnt1, ushort LogCnt2, ushort type)
{
	ushort	w_len1, w_len2;
	ushort	w_pos1, w_pos2, w_pos3, w_pos4;

	w_pos1 = 0;
	// �Ώۃ��O�����̌���
	if( LogCnt1 >= 1000 ) {
		w_len1 = 4;
	} else if (LogCnt1 >= 100 ) {
		w_len1 = 3;
	} else if (LogCnt1 >= 10 ) {
		w_len1 = 2;
	} else {
		w_len1 = 1;
	}

	// �����O�����̌���
	if( LogCnt2 >= 1000 ) {
		w_len2 = 4;
	} else if (LogCnt2 >= 100 ) {
		w_len2 = 3;
	} else if (LogCnt2 >= 10 ) {
		w_len2 = 2;
	} else {
		w_len2 = 1;
	}

	// �\���ʒu������
	if( type == 3 ) {			// �N���W�b�g���p���׃��O
		w_pos1 = 10;			// �Ώۃ��O����
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[95] );
	}
	else if( type == 4) {	//���o�Ƀ��O
		w_pos1 = 10;			// �Ώۃ��O����
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[95] );
	}
	w_pos2 = w_pos1 + w_len1*2;	// �u�^�v
	w_pos3 = w_pos2 + 2;		// �����O����
	w_pos4 = w_pos3 + w_len2*2; // �u���v

	// �\��
	opedsp( 2, w_pos1, LogCnt1, w_len1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �Ώۃ��O�����\���i�O�T�v���X�j
	grachr( 2, w_pos2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"�^" );
	opedsp( 2, w_pos3, LogCnt2, w_len2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �����O�����\���i�O�T�v���X�j
	grachr( 2, w_pos4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"��" );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O���t�ʒu�ԍ��X�V�����Q�i�s�������v�����g�p�j					|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDatePosUp1( uchar *pos, uchar req )				|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*pos= �ʒu�ԍ��ް��߲��	(0-9)				|*/
/*|																			|*/
/*|						uchar	req	= �����v��								|*/
/*|										�O�F�ʒu�ԍ��|�P�i���ړ��j			|*/
/*|										�P�F�ʒu�ԍ��{�P�i�E�ړ��j			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	�s�ړ��i0:�Ȃ��^1:����j					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
uchar	LogDatePosUp1( uchar *pos, uchar req )
{
	uchar	flg = 0;
 
	if( req == 0 ){		// �ʒu�ԍ��|�P�i���ړ��j
		if( (*pos == 0)||(*pos == 3) ) {
			flg = 1;
		}
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = 5;
		}
	}
	else{				// �ʒu�ԍ��{�P�i�E�ړ��j		
		if( (*pos == 2)||(*pos == 5) ) {
			flg = 1;
		}
		*pos = (uchar)((*pos) + 1);
		if( *pos >= 6 ){
			*pos = 0;
		}
	}
	return(flg);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�󎚊J�n�^�I�����t�ް��\������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	���t�ް��߲���					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	���]�\���ʒu					|*/
/*|											�O�`�T�F�w��ʒu���]�\��		|*/
/*|											���̑��F���]�\���Ȃ�			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
// �\�����ږ��̕\�����@�ް�ð��قQ�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA2[6][4] = {
	{ 4,  8, 4, 1 },	// �J�n�i�N�j
	{ 4, 18, 2, 0 },	// �J�n�i���j
	{ 4, 24, 2, 0 },	// �J�n�i���j
	{ 5,  8, 4, 1 },	// �I���i�N�j
	{ 5, 18, 2, 0 }, 	// �I���i���j
	{ 5, 24, 2, 0 }		// �I���i���j
};

void	LogDateDsp( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// ���t�ް��\���ʒu
	ushort	rev_data;		// ���]�\���ް�


	for( pos = 0 ; pos < 6 ; pos++){		// ���t�ް��\���F�J�n�i�N�j�`�I���i���j

		opedsp	(
					POS_DATA2[pos][0],		// �\���s
					POS_DATA2[pos][1],		// �\�����
					*Date,					// �\���ް�
					POS_DATA2[pos][2],		// �ް�����
					POS_DATA2[pos][3],		// �O�T�v���X�L��
					0,						// ���]�\���F�Ȃ�
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;			// ���]�\���ް�����
		}

		Date++;								// ���t�ް��߲���X�V
	}
	if( Rev_Pos <= 5 ){						// ���]�\���w�肠��H

		opedsp	(							// �w��ʒu�𔽓]�\��������
					POS_DATA1[Rev_Pos][0],	// �\���s
					POS_DATA1[Rev_Pos][1],	// �\�����
					rev_data,				// �\���ް�
					POS_DATA1[Rev_Pos][2],	// �ް�����
					POS_DATA1[Rev_Pos][3],	// �O�T�v���X�L��
					1,						// ���]�\���F����
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�ŌÁ^�ŐV���O���t�f�[�^�\������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp4( *Date )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	���t�ް��߲���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	m.nagashima													|*/
/*|	Date	:	2012-03-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
// �\�����ږ��̕\�����@�ް�ð��قS�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA4[6][4] = {
	{ 2,  0, 4, 1 },	// �ŌÃ��O�i�N�j
	{ 2,  6, 2, 0 },	// �ŌÃ��O�i���j
	{ 2, 10, 2, 0 },	// �ŌÃ��O�i���j
	{ 2, 16, 4, 1 },	// �ŐV���O�i�N�j
	{ 2, 22, 2, 0 }, 	// �ŐV���O�i���j
	{ 2, 26, 2, 0 }		// �ŐV���O�i���j
};

void	LogDateDsp4( ushort *Date )
{
	uchar	pos;			// ���t�f�[�^�\���ʒu

	for( pos = 0 ; pos < 6 ; pos++){		// ���t�ް��\���F�J�n�i�N�j�`�I���i���j

		opedsp3	(
					POS_DATA4[pos][0],		// �\���s
					POS_DATA4[pos][1],		// �\�����
					*Date,					// �\���ް�
					POS_DATA4[pos][2],		// �ް�����
					POS_DATA4[pos][3],		// �O�T�v���X�L��
					0,						// ���]�\���F�Ȃ�
					COLOR_BLACK,
					LCD_BLINK_OFF
				);


		Date++;								// ���t�ް��߲���X�V
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������t�f�[�^�\������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp5( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	���t�ް��߲���					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	���]�\���ʒu					|*/
/*|											�O�`�R�F�w��ʒu���]�\��		|*/
/*|											���̑��F���]�\���Ȃ�			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	m.nagashima													|*/
/*|	Date	:	2012-03-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
// �\�����ږ��̕\�����@�ް�ð��قT�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA5_0[3][4] = {
	{ 5,  8, 4, 1 },	// ����i�N�j
	{ 5, 18, 2, 0 },	// ����i���j
	{ 5, 24, 2, 0 },	// ����i���j
};
const	ushort	POS_DATA5_1[3][4] = {
	{ 5, 12, 2, 1 },	// ����i�N�j
	{ 5, 18, 2, 0 },	// ����i���j
	{ 5, 24, 2, 0 },	// ����i���j
};

void	LogDateDsp5( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// ���t�ް��\���ʒu
	ushort	rev_data;		// ���]�\���ް�


	for( pos = 0 ; pos < 3 ; pos++){			// ������t�f�[�^�\���F����i�N�j�`����i���j

		opedsp	(
					POS_DATA5_0[pos][0],		// �\���s
					POS_DATA5_0[pos][1],		// �\�����
					*Date,						// �\���ް�
					POS_DATA5_0[pos][2],		// �ް�����
					POS_DATA5_0[pos][3],		// �O�T�v���X�L��
					0,							// ���]�\���F�Ȃ�
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;				// ���]�\���ް�����
		}

		Date++;									// ���t�ް��߲���X�V
	}
	if( Rev_Pos < 3 ){							// ���]�\���w�肠��H

		opedsp	(								// �w��ʒu�𔽓]�\��������
					POS_DATA5_1[Rev_Pos][0],	// �\���s
					POS_DATA5_1[Rev_Pos][1],	// �\�����
					rev_data,					// �\���ް�
					POS_DATA5_1[Rev_Pos][2],	// �ް�����
					POS_DATA5_1[Rev_Pos][3],	// �O�T�v���X�L��
					1,							// ���]�\���F����
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�|�C���^�擾����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet( cnt, wp, max, req )						|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt	= �Ώ��ޯ̧�̌��݊i�[����				|*/
/*|																			|*/
/*|						ushort	wp	= �Ώ��ޯ̧��ײ��߲��					|*/
/*|																			|*/
/*|						ushort	max	= �Ώ��ޯ̧�̍ő�i�[����				|*/
/*|																			|*/
/*|						uchar	req	= �����v��								|*/
/*|										�O�F�Ō��ް��߲���擾				|*/
/*|										�P�F�ŐV�ް��߲���擾				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ۸��ް��߲���i�z��ԍ��j				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req )
{
	ushort	ret = 0;

	if( cnt != 0 ){

		if( req == 0 ){

			// �Ō��ް�����
			if( cnt == max ){
				ret = wp;
			}
			else{
				ret = 0;
			}
		}
		else{
			// �ŐV�ް�����
			if( wp != 0 ){
				ret = wp-1;
			}
			else{
				ret = max-1;
			}
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�|�C���^�擾�����R�i�s�������v�����g�p�j				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet3( cnt, wp, max, req )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req	= �����v��								|*/
/*|										�O�F�Ō��ް��߲���擾				|*/
/*|										�P�F�ŐV�ް��߲���擾				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ۸��ް��߲���i�z��ԍ��j				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
ushort	LogPtrGet3( uchar req )
{
	ushort	posi = 0;
	return(posi);
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�|�C���^�擾�����Q�i���o�ɏ��v�����g�p�j				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet2( no )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	no		= �Ԏ��ԍ�							|*/
/*|					:	ulong	*inf	= ۸��ް��߲���i�z��ԍ��j			|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ����									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-04-20													|*/
/*|	Update	:   2013-01-09 m.onouchi 										|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort LogPtrGet2(ushort no, ulong *inf)
{
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	uchar i;																	// �������Q�Ɛ�p���[�v�ϐ�
//	ushort pt = 0;																// ��������v�������O�̈ʒu
//	ushort log_cnt = 0;															// �����Ɉ�v�������O�̌���
//	ulong Wdate;																// �Q�ƒ��������
//	ulong Sdate = 0, Edate = 0;													// �ŐV�E�ŌÓ������(��)
//
//	((union ioLogNumberInfo *)(inf))->n = 0;
//	pt = UsMnt_SearchIOLog(0, no, 0, 0, &Io_log_wk);							// �ŏ��̃��O����
//	if (pt) {																	// �����Ɉ�v�������O����
//		/* ���ۊǗ��e�[�u���̏I�[���擾 */
//		for(i = 0; ((i+1) < IO_EVE_CNT) && (Io_log_wk.iolog_data[i+1].Event_type); i++);	// ���ی��������܂ŃV�[�N
//		Sdate = enc_nmlz_mdhm(													// �Q�ƒ��C�x���g����������ϊ�
//			Io_log_wk.iolog_data[0].NowTime.Year,								// �N
//			Io_log_wk.iolog_data[0].NowTime.Mon,								// ��
//			Io_log_wk.iolog_data[0].NowTime.Day,								// ��
//			Io_log_wk.iolog_data[0].NowTime.Hour,								// ��
//			Io_log_wk.iolog_data[0].NowTime.Min									// ��
//		);
//		Edate = enc_nmlz_mdhm(													// �Q�ƒ��C�x���g����������ϊ�
//			Io_log_wk.iolog_data[i].NowTime.Year,								// �N
//			Io_log_wk.iolog_data[i].NowTime.Mon,								// ��
//			Io_log_wk.iolog_data[i].NowTime.Day,								// ��
//			Io_log_wk.iolog_data[i].NowTime.Hour,								// ��
//			Io_log_wk.iolog_data[i].NowTime.Min									// ��
//		);
//		((union ioLogNumberInfo *)(inf))->log.sdat = (ulong)(pt - 1);			// �ŌÃ��O�̈���X�V
//		((union ioLogNumberInfo *)(inf))->log.edat = (ulong)(pt - 1);			// �ŐV���O�̈���X�V
//		((union ioLogNumberInfo *)(inf))->log.evnt = (ulong)i;					// �ŐV���۔����i�[�ʒu�X�V
//		log_cnt++;																// ������v�����X�V
//		while (0 != (pt = UsMnt_SearchIOLog(pt, no, 0, 0, &Io_log_wk))) {		// ���̏�����v���O������
//			for(i = 0; ((i+1) < IO_EVE_CNT) && (Io_log_wk.iolog_data[i+1].Event_type); i++);	// ���ی��������܂ŃV�[�N
//			Wdate = enc_nmlz_mdhm(												// �Q�ƒ��C�x���g����������ϊ�
//				Io_log_wk.iolog_data[0].NowTime.Year,							// �N
//				Io_log_wk.iolog_data[0].NowTime.Mon,							// ��
//				Io_log_wk.iolog_data[0].NowTime.Day,							// ��
//				Io_log_wk.iolog_data[0].NowTime.Hour,							// ��
//				Io_log_wk.iolog_data[0].NowTime.Min								// ��
//			);
//			if (Edate < Wdate) {												// �ێ����Ă��鎞�Ԃ���
//				Edate = Wdate;													// �ŐV���ԍX�V
//				((union ioLogNumberInfo *)(inf))->log.edat = (ulong)(pt - 1);	// �ŐV���O�̈���X�V
//				((union ioLogNumberInfo *)(inf))->log.evnt = (ulong)i;			// �ŐV���۔����i�[�ʒu�X�V
//			}
//			Wdate = enc_nmlz_mdhm(												// �Q�ƒ��C�x���g����������ϊ�
//				Io_log_wk.iolog_data[i].NowTime.Year,							// �N
//				Io_log_wk.iolog_data[i].NowTime.Mon,							// ��
//				Io_log_wk.iolog_data[i].NowTime.Day,							// ��
//				Io_log_wk.iolog_data[i].NowTime.Hour,							// ��
//				Io_log_wk.iolog_data[i].NowTime.Min								// ��
//			);
//			if (Sdate > Wdate) {												// �ێ����Ă��鎞�Ԃ��O
//				Sdate = Wdate;													// �ŌÎ��ԍX�V
//				((union ioLogNumberInfo *)(inf))->log.sdat = (ulong)(pt - 1);	// �ŌÃ��O�̈���X�V
//			}
//			log_cnt++;															// ������v�����X�V
//		}
//	}
//	return log_cnt;															// �����Ɉ�v�������O�̌�����Ԃ�
	return 0;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ŌÁ��ŐV���O�f�[�^���t�f�[�^�擾����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateGet( *Old, *New, *Date )						|*/
/*|																			|*/
/*|	PARAMETER		:	date_time_rec	*Old	= �Ō�۸��ް����t�ް��߲��	|*/
/*|																			|*/
/*|						date_time_rec	*New	= �ŐV۸��ް����t�ް��߲��	|*/
/*|																			|*/
/*|						ushort			*Date	= ���t�ް��i�[�߲��			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date )
{
	// �Ō�۸��ް����t�ް�
	*Date = Old->Year;
	Date++;
	*Date = Old->Mon;
	Date++;
	*Date = Old->Day;
	Date++;
	// �ŐV۸��ް����t�ް�
	*Date = New->Year;
	Date++;
	*Date = New->Mon;
	Date++;
	*Date = New->Day;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�N���A����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDataClr( LogSyu )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	= ���O�f�[�^���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret = OK/NG									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	LogDataClr( ushort LogSyu )
{
	uchar	ret = OK;	// �߂�l�i�������ʁj

	switch( LogSyu ){				// ���O�f�[�^��ʁH

		case	LOG_ERROR:			// �G���[���
			FLT_LogErase2( eLOG_ERROR );
			break;

		case	LOG_ALARM:			// �A���[�����
			FLT_LogErase2( eLOG_ALARM );
			break;

		case	LOG_OPERATE:		// ������
			FLT_LogErase2( eLOG_OPERATE );
			break;

		case	LOG_POWERON:		// �╜�d���
			FLT_LogErase2( eLOG_POWERON );
			break;

		case	LOG_TTOTAL:			// �s���v���
			FLT_LogErase2( eLOG_TTOTAL );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//			FLT_LogErase2( eLOG_LCKTTL );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			break;

		case	LOG_GTTOTAL:		// �f�s���v���
			FLT_LogErase2( eLOG_GTTOTAL );
			break;

		case	LOG_COINBOX:		// �R�C�����ɏ��
			FLT_LogErase2( eLOG_COINBOX );
			break;

		case	LOG_NOTEBOX:		// �������ɏ��
			FLT_LogErase2( eLOG_NOTEBOX );
			break;

		case	LOG_MONEYMANAGE:	// �ޑK�Ǘ����
			FLT_LogErase2( eLOG_MONEYMANAGE );
			break;

		case	LOG_PAYMENT:		// �ʐ��Z���
			FLT_LogErase2( eLOG_PAYMENT );
			break;

		case	LOG_ABNORMAL:		// �s���E�����o�ɏ��
			FLT_LogErase2( eLOG_ABNORMAL );
			break;

		case	LOG_MONITOR:		// ���j�^���
			FLT_LogErase2( eLOG_MONITOR );
			break;

		case	LOG_CREUSE:			// �N���W�b�g���p���׃��O
			FLT_LogErase2( eLOG_CREUSE );
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case	LOG_EDYARM:			// Edy�A���[��������O
//			FLT_LogErase2( eLOG_EDYARM );
//			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {	// Edy�ݒ肪���邩
//				wopelg( OPLOG_EDY_ARM_CLR,0,0 );	// ���엚��o�^�iEdy����װя��ر�j
//			}
//			break;
//
//		case	LOG_EDYSHIME:			// Edy���ߋL�^���O
//			FLT_LogErase2( eLOG_EDYSHIME );
//			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ) {	// Edy�ݒ肪���邩
//				wopelg( OPLOG_EDY_SHIME_CLR,0,0 );	// ���엚��o�^�iEdy���ߋL�^���ر�j
//			}
//			break;
//#ifdef	FUNCTION_MASK_EDY 
//		case LOG_EDYSYUUKEI:								// �d�����W�v����
//			Syuukei_sp.edy_inf.cnt = 0;						// �W�v����������
//			ck_syuukei_ptr_zai(LOG_EDYSYUUKEI);				// �W�v��񏉊�������
//			memset( &bk_syu_dat, 0, sizeof( bk_syu_dat ));
//			break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case LOG_SCASYUUKEI:								// �r���������W�v����
			Syuukei_sp.sca_inf.cnt = 0;						// �W�v����������
			ck_syuukei_ptr_zai(LOG_SCASYUUKEI);				// �W�v��񏉊�������
			memset( &bk_syu_dat, 0, sizeof( bk_syu_dat ));
			break;

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
		case LOG_ECSYUUKEI:									// ���σ��[�_�W�v����
			Syuukei_sp.ec_inf.cnt = 0;						// �W�v����������
			ck_syuukei_ptr_zai(LOG_ECSYUUKEI);				// �W�v��񏉊�������
			memset( &bk_syu_dat_ec, 0, sizeof( bk_syu_dat_ec ));
			break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
		case ADDPRM_LOG:									// �ݒ�X�V���
			SetDiffLogReset(1);								// �ݒ�ύX�������O�N���A
			break;

		case LOG_HOJIN_USE:
			FLT_LogErase2( eLOG_HOJIN_USE );
			break;

		case	LOG_NGLOG:						// �s�����O
			FLT_LogErase2( eLOG_NGLOG );
			break;

		case	LOG_REMOTE_SET:					// ���u�����ݒ胍�O
			FLT_LogErase2( eLOG_REMOTE_SET );
			break;

// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//		case	LOG_ENTER:						// ���Ƀ��O
//			FLT_LogErase2( eLOG_ENTER );
//			break;
		case	LOG_RTPAY:						// RT���Z�f�[�^
			FLT_LogErase2( eLOG_RTPAY );
			break;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
		
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case	LOG_RTRECEIPT:					// RT�̎��؃f�[�^
			FLT_LogErase2( eLOG_RTRECEIPT );
			break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

		case	LOG_PARKING:					// ���ԑ䐔�f�[�^
			FLT_LogErase2( eLOG_PARKING );
			break;
		
		case	LOG_LONGPARK:					// �������ԃf�[�^
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//			FLT_LogErase2( eLOG_LONGPARK );
			FLT_LogErase2( eLOG_LONGPARK_PWEB );
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
			break;
		
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		case	LOG_RISMEVENT:					// RISM�C�x���g�f�[�^
//			FLT_LogErase2( eLOG_RISMEVENT );
		case LOG_DC_LANE:						// ���[�����j�^�f�[�^
			FLT_LogErase2( eLOG_DC_LANE );
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			break;

// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//		case LOG_IOLOG:
//			memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );
//			FLT_LogErase2( eLOG_IOLOG );
		case LOG_DC_QR:
			FLT_LogErase2( eLOG_DC_QR );
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
			break;

		case	LOG_MNYMNG_SRAM:				// �ޑK�Ǘ����(SRAM)
			FLT_LogErase2( eLOG_MNYMNG_SRAM );
			FLT_LogErase2( eLOG_MONEYMANAGE );
			break;

		case	LOG_RTPAYMENT:					// �����e�X�g���(SRAM)
			RT_PAY_LOG_DAT.RT_pay_count = 0;	// �W�v����������
			RT_PAY_LOG_DAT.RT_pay_wtp = 0;		// ���C�g�|�C���^�N���A
			memset( &RT_PAY_LOG_DAT, 0, sizeof( RT_PAY_LOG_DAT ));	// �����e�X�g�������O�N���A
			break;
		case	LOG_REMOTE_MONITOR:				// ���u�Ď��f�[�^
			FLT_LogErase2( eLOG_REMOTE_MONITOR );
			break;
		default:								// ���̑��i���O�f�[�^��ʃG���[�j
			ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	if(ret == OK) {
		LogDataClr_CheckBufferFullRelease(LogSyu);
	}

	return(ret);
}

/*[]----------------------------------------------------------------------[]*
 *| backup / restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_BackupRestore
 *| PARAMETER    : none
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	SysMnt_BackupRestore(void)
{
	ushort	usSysEvent;
	char	wk[2];
	
	/* initialize cursor position */
	DP_CP[0] = DP_CP[1] = 0;
	
	while (1) {
		/* clear screen */
		dispclr();
		/* display title */
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[39]);			/* [39]	"���o�b�N�A�b�v�^���X�g�A��" */
		/* display function */
		Fun_Dsp(FUNMSG[25]);						/* [25]	"�@���@�@���@�@�@�@ �Ǐo  �I�� " */
		/* display items */
		usSysEvent = Menu_Slt(BKRSMN, BKRS_TBL, (char)BKRS_MAX, (char)1);
		
		/* save current cursor position */
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		
		/* dispatch event */
		switch (usSysEvent) {
			case BKRS_BK:		/* no break */
			case BKRS_RS:
				usSysEvent = SysMnt_BackupRestoreData(usSysEvent);
				break;
//���g�p�ł��iS�j				
			case BKRS_FLSTS:
				if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
					if( Ext_Menu_Max > 1){
						usSysEvent = UsMnt_PreAreaSelect(BKRS_FLSTS);
					}
					else {
						DispAdjuster = Car_Start_Index[gCurSrtPara-MNT_INT_CAR];
						usSysEvent = UsMnt_AreaSelect(BKRS_FLSTS);
					}
				}else{
					BUZPIPI();
				}
				break;
//���g�p�ł��iE�j				
			case MOD_EXT:		/* end function selected */
				return MOD_EXT;					/* exit screen! */
			default:
				break;
		}
		
		if( usSysEvent == MOD_CHG || usSysEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}

		if (usSysEvent == MOD_CHG) {
		/* mode change occured */
			return MOD_CHG;						/* exit screen! */
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( usSysEvent == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		/* restore cursor position */
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*
 *| backup / restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_BackupRestoreData
 *| PARAMETER    : kind - BKRS_BK or BKRS_RS
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	SysMnt_BackupRestoreData(ushort kind)
{
// MH810100(S) Y.Yamauchi 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	short	msg;
	short msg, ret;
// MH810100(E) Y.Yamauchi 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
	t_SysMnt_ErrInfo	errinfo;
	
	/* display foolproof */
	dispclr();
	SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_START);
	Fun_Dsp(FUNMSG[19]);								/* [19]	"�@�@�@�@�@�@ �͂� �������@�@�@" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);					/* [21]	"�@�@�@ ��낵���ł����H �@�@�@" */
	while (1) {
		/* wait key input */
		msg = sysmnt_GetMessage(_SYSMNT_WAIT_MSG);
		/* dispath event */
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;				/* exit screen! */
		case KEY_TEN_F3:	/* �͂� */
			BUZPI();
			Ope_DisableDoorKnobChime();
			/* display executing */
			dispclr();
			SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_EXEC);
			Fun_Dsp(FUNMSG[77]);						/* [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
			grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);	/* [24]	"�@�@���΂炭���҂����������@�@" */
			/* initialization */
			_sysmnt_StartBackupRestore(&errinfo);
			/* execute download */
			switch (kind) {
			case BKRS_BK:
				xPause( 20 );		// 200ms wait�i�޻ްOFF����܂ő҂j
				wopelg( OPLOG_ALLBACKUP, 0, 0 );			// ���엚��o�^
				sysmnt_Backup(&errinfo);
				break;
			case BKRS_RS:
				xPause( 20 ); 		// 200ms wait�i�޻ްOFF����܂ő҂j
				sysmnt_Restore(&errinfo);
				wopelg( OPLOG_ALLRESTORE, 0, 0 );			// ���엚��o�^
				break;
			}
			if(!Ope_IsEnableDoorKnobChime()) {
				Ope_EnableDoorKnobChime();
			}
			/* display result */
			dispclr();
			SysMntDisplayBackupRestoreTitle(kind, _SYSMNT_BKRS_END);
			Fun_Dsp(FUNMSG[8]);							/* [08]	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
			SysMntDisplayResult(&errinfo);
			/* wait for user response */
// MH810100(S) Y.Yamauchi 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if (sysmnt_WaitF5() == KEY_MODECHG) {
//				return MOD_CHG;			/* exit screen! */
//			}
			ret = sysmnt_WaitF5();
			if( ret == LCD_DISCONNECT ){
				return MOD_CUT;
			}else if( ret == KEY_MODECHG ){
				return MOD_CHG;
			}
// MH810100(E) Y.Yamauchi 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return MOD_EXT;				/* exit screen! */
		case KEY_TEN_F4:	/* ������ */
			BUZPI();
			return MOD_EXT;				/* exit screen! */
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| backup
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_Backup
 *| PARAMETER    : errinfo - error information				<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_Backup(t_SysMnt_ErrInfo *errinfo)
{
	ulong ret;
	ulong verup = 0;
	uchar i;
	uchar f_Exec = FLT_EXCLUSIVE;			/* �^�X�N�ؑւ������s */

	// �o�b�N�A�b�v���̃��O�o�[�W�������i�[
	bk_log_ver = LOG_VERSION;

	//���X�g�A���ɃT�C�Y���������`�F�b�N���邽�߂Ɋi�[
	memset(&BR_LOG_SIZE,0,sizeof(BR_LOG_SIZE));
	for( i = 0; i < eLOG_MAX; i++ ){
		BR_LOG_SIZE[i] = LogDatMax[i][0];
	}

	/* lock RAM data */
	if (!sysmnt_RamLock(_SYSMNT_LOCK_ALL)) {
		errinfo->errmsg = _SYSMNT_ERR_DATA_LOCKED;
		remotedl_monitor_info_set(41);
		return;
	}
	
	/* write on flash */
	_di();
	ret = FLT_Backup(verup,f_Exec);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		ret = FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );		// ���O�T�C�Y��񏑂�����
	}
	_ei();
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		sysmnt_OnFlashErr(errinfo, ret);
		goto _end;
	}
	/* wait flash written */

_end:

	/* unlock RAM data */
	sysmnt_RamUnlock(_SYSMNT_LOCK_ALL);
}

/*[]----------------------------------------------------------------------[]*
 *| restore
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_Restore
 *| PARAMETER    : errinfo - error information				<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_Restore(t_SysMnt_ErrInfo *errinfo)
{
	ulong ret;
	uchar i;
	uchar f_Exec = FLT_EXCLUSIVE;			/* �^�X�N�ؑւ������s */
	uchar f_LogSzChg = 0;
	memset(&BR_LOG_SIZE,0,sizeof(BR_LOG_SIZE));
	if (!sysmnt_RamLock(_SYSMNT_LOCK_ALL)) {
		errinfo->errmsg = _SYSMNT_ERR_DATA_LOCKED;
		return;
	}
	
	/* write on flash */
	_di();
	AppServ_PParam_Copy = ON;				// FLASH �� RAM �����Ұ���߰�L������߰����
	ret = FLT_Restore( f_Exec );
	AppServ_PParam_Copy = OFF;				// FLASH �� RAM �����Ұ���߰�L������߰���Ȃ�
	_ei();
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		sysmnt_OnFlashErr(errinfo, ret);
		goto _end;
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);		/* update parameter sum on ram */
	DataSumUpdate(OPE_DTNUM_LOCKINFO);		/* update parameter sum on ram */
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
	mnt_SetFtpFlag( FTP_REQ_WITH_POWEROFF );		// FTP�X�V�t���O�Z�b�g�i�v�d�f�j
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)

_end:
	/* unlock RAM data */
	sysmnt_RamUnlock(_SYSMNT_LOCK_ALL);
	ret = FLT_ReadBRLOGSZ( &BR_LOG_SIZE[0] );
	for(i = 0;i < eLOG_MAX; i++){
		if( BR_LOG_SIZE[i] != LogDatMax[i][0] ){
			FLT_LogErase2(i);
			f_LogSzChg = 1;
		}
	}
	if(f_LogSzChg != 0){
		for( i = 0; i < eLOG_MAX ; i++ ){
			BR_LOG_SIZE[i] = LogDatMax[i][0];							// �V���O�T�C�Yset
		}
		FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );			// �V���O�T�C�Y��񏑂�����
	}
}

/*[]----------------------------------------------------------------------[]*
 *| display title of screen
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMntDisplayBackupRestoreTitle
 *| PARAMETER    : kind - kind of screen (BKRS_XX)
 *|				   phase - phase of execution
 *|					(_SYSMNT_BKRS_START/_SYSMNT_BKRS_EXEC/_SYSMNT_BKRS_END)
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SysMntDisplayBackupRestoreTitle(ushort kind, int phase)
{
	ushort w1, w2;
	
	switch (kind) {
	case BKRS_BK:
		w1 = 46;
		switch (phase) {
		case _SYSMNT_BKRS_START:
			w2 = 40;
			break;
		case _SYSMNT_BKRS_EXEC:
			w2 = 41;
			break;
		case _SYSMNT_BKRS_END:
			w2 = 42;
			break;
		default:
			return;
		}
		break;
	case BKRS_RS:
		w1 = 47;
		switch (phase) {
		case _SYSMNT_BKRS_START:
			w2 = 43;
			break;
		case _SYSMNT_BKRS_EXEC:
			w2 = 44;
			break;
		case _SYSMNT_BKRS_END:
			w2 = 45;
			break;
		default:
			return;
		}
		break;
	default:
		return;
	}
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[w1]);
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[w2]);
}

/*[]----------------------------------------------------------------------[]*
 *| display result of execution
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMntDisplayResult
 *| PARAMETER    : errinfo - result of execution
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SysMntDisplayResult(t_SysMnt_ErrInfo *errinfo)
{
	if (errinfo->errmsg == _SYSMNT_ERR_NONE) {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* [27]	"�@�@�@�@�@ ����I�� �@�@�@�@�@" */
		BUZPI();
	}
	else {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* [28]	"�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[errinfo->errmsg]);	/* error detail */
		if (errinfo->errmsg == _SYSMNT_ERR_WRITE || errinfo->errmsg == _SYSMNT_ERR_ERASE) {
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[35]);	/* [35]	"�A�h���X�F�@�@�@�g�@�@�@�@�@�@" */
			opedpl2(6, 10, errinfo->address, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		/* 		"�@�@�@�@�@XXXXXX�@�@�@�@�@�@�@" */
		}
		else if (errinfo->errmsg == _SYSMNT_ERR_VERIFY) {
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[37]);	/* [37]	"�ēx���s���ĉ������B�@�@�@�@�@" */
		}
		BUZPIPI();
	}
}

/*[]----------------------------------------------------------------------[]*
 *| wait for F5 pushed
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_WaitF5
 *| PARAMETER    : none
 *| RETURN VALUE : event occured(KEY_TEN_F5/KEY_MODECHG)
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	sysmnt_WaitF5(void)
{
	short msg;
	
	/* read event stocked */
	while (1) {
		msg = sysmnt_GetMessage(_SYSMNT_NOT_WAIT_MSG);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (msg == KEY_MODECHG) {
		if (msg == KEY_MODECHG ||  msg == LCD_DISCONNECT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (!SysMntModeChange) {
				SysMntModeChange = TRUE;
			}
			else {
				SysMntModeChange = FALSE;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (!SysMntLcdDisconnect) {
				SysMntLcdDisconnect = TRUE;
			}
			else {
				SysMntLcdDisconnect = FALSE;
			}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}
		else if (msg == TIMEOUT6) {
			break;
		}
	}
	
	if (SysMntModeChange ) {
		msg =  KEY_MODECHG;
// MH810100(S)
	} else if(SysMntLcdDisconnect){
		msg =  LCD_DISCONNECT;
// MH810100(E)
	}
	else {
		/* wait F5 input */
		while (1) {
			msg = sysmnt_GetMessage(_SYSMNT_WAIT_MSG);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if (msg == KEY_MODECHG || msg == KEY_TEN_F5) {
			if (msg == KEY_MODECHG || msg == KEY_TEN_F5 || msg == LCD_DISCONNECT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
			}
		}
	}
	
	BUZPI();
	
	return msg;
}



/*[]----------------------------------------------------------------------[]*
 *| initialize backup/restore/download/upload operations
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_StartBkRsUpDwn
 *| PARAMETER    : errinfo - error information to initialize	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_StartBkRsUpDwn(t_SysMnt_ErrInfo *errinfo)
{
	sysmnt_InitErr(errinfo);
	SysMntModeChange = FALSE;
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
	SysMntLcdDisconnect = FALSE;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
}

/*[]----------------------------------------------------------------------[]*
 *| initialize error information
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_InitErr
 *| PARAMETER    : errinfo - error information to initialize	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_InitErr(t_SysMnt_ErrInfo *errinfo)
{
	errinfo->errmsg = _SYSMNT_ERR_NONE;
}

/*[]----------------------------------------------------------------------[]*
 *| regist error result to call flash api, to error information
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_OnFlashErr
 *| PARAMETER    : errinfo - error information to regist to	<OUT>
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_OnFlashErr(t_SysMnt_ErrInfo *errinfo, ulong fltcode)
{
	switch (_FLT_RtnKind(fltcode)) {
	case FLT_ERASE_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_ERASE;
		errinfo->address	= _FLT_RtnDetail(fltcode);
		remotedl_monitor_info_set(42);
		break;
	case FLT_WRITE_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_WRITE;
		errinfo->address	= _FLT_RtnDetail(fltcode);
		remotedl_monitor_info_set(43);
		break;
	case FLT_VERIFY_ERR:
		errinfo->errmsg		= _SYSMNT_ERR_VERIFY;
		remotedl_monitor_info_set(44);
		break;
	case FLT_PARAM_ERR:		/* no break */
		/* this path should not be passed. */
	case FLT_NODATA:
		errinfo->errmsg		= _SYSMNT_ERR_NO_DATA;
		remotedl_monitor_info_set(45);
		break;
	case FLT_BUSY:			/* no break */
	case FLT_NOT_LOCKED:
		errinfo->errmsg		= _SYSMNT_ERR_DATA_LOCKED;
		remotedl_monitor_info_set(46);
		break;
	case FLT_INVALID_SIZE:
		errinfo->errmsg		= _SYSMNT_ERR_INVALID_DATA;
		remotedl_monitor_info_set(47);
		break;
	default:	/* case FLT_NORMAL */
		errinfo->errmsg	= _SYSMNT_ERR_NONE;
		break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| get message with using timeout
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_GetMessage
 *| PARAMETER    : option - _SYSMNT_WAIT_MSG/_SYSMNT_NOT_WAIT_MSG
 *| RETURN VALUE : message
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	sysmnt_GetMessage(int option)
{
	short msg;
	
	if (option == _SYSMNT_NOT_WAIT_MSG) {
	/* start timer to set timeout */
		Lagtim(OPETCBNO, 6, 1);
	}
	/* wait message */
	msg = StoF(GetMessage(), 1);
	if (option == _SYSMNT_NOT_WAIT_MSG) {
	/* stop timer */
		Lagcan(OPETCBNO, 6);
	}
	
	return msg;
}

/*[]----------------------------------------------------------------------[]*
 *| lock ram area
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_RamLock
 *| PARAMETER    : lock - ram area to lock (_SYSMNT_LOCK_XXX)
 *| RETURN VALUE : TRUE as success to lock
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	sysmnt_RamLock(ulong lock)
{
	int i;
	
	/* get semaphore */
	for (i = 0; i < 32; i++) {
		if (lock & BitShift_Left(1L,i)) {
			if (!Log_SemGet((uchar)i)) {
				break;		/* can't get semaphore */
			}
		}
	}
	if (i < 32) {
	/* failed to get */
		/* release semaphore, already have get */
		while (--i >= 0) {
			if (lock & BitShift_Left(1L,i)) {
				Log_SemFre((uchar)i);
			}
		}
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*
 *| unlock ram area
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : sysmnt_RamUnlock
 *| PARAMETER    : unlock - ram area to unlock (_SYSMNT_LOCK_XXX)
 *| RETURN VALUE : none
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sysmnt_RamUnlock(ulong unlock)
{
	int i;
	
	for (i = 0; i < 32; i++) {
		if (unlock & BitShift_Left(1L,i)) {
			Log_SemFre((uchar)i);
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �r�b�g���V�t�g�i�S�o�C�g�܂őΉ��j
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : BitShift_Left
 *| PARAMETER    : 	ulong 	data	�F���f�[�^
 *| 				int		bit_cnt	�F���V�t�g�������r�b�g��
 *| RETURN VALUE : 	ulong output	�F����
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	BitShift_Left(ulong data,int bit_cnt)
{
	int i;
	ulong output;

	output = data;
	for(i=0;i<bit_cnt;i++){
		output = output << 1;
	}
	return(output);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���S�󎚃f�[�^�v�����^�o�^����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMntLogoDataRegDsp( )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar *title = ��ʃ^�C�g��							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	SysMntLogoDataRegDsp( const uchar *title )
{
	T_FrmLogoRegist	FrmLogoRegist;		// ���S�󎚃f�[�^�o�^�v��ү���ލ쐬�ر
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	uchar			wait_cnt= 0;		// ���S�f�[�^�o�^�����҂���
	uchar			end_sts	= 0;		// �o�^�����I�����

	// ���S�f�[�^�o�^����ʕ\��
	dispclr();												// ��ʃN���A
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);								// �޳�۰�����ٕ\��
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[54]);						// [54]	"�@�v�����^�Ƀf�[�^�o�^���ł��@"
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);				// [24]	"�@�@���΂炭���҂����������@�@"
	Fun_Dsp(FUNMSG[77]);									// [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"

	// ���S�f�[�^�o�^�v�����b�Z�[�W�쐬
	Logo_Reg_sts_rct = OFF;									// ۺޓo�^�������ؾ�āiڼ�āj
	Logo_Reg_sts_jnl = OFF;									// ۺޓo�^�������ؾ�āi�ެ��فj


	FrmLogoRegist.prn_kind = R_PRI;							// �������ʁFڼ��
	Logo_Reg_sts_jnl = ON;									// ۺޓo�^������ԁi�ެ��فj�F�o�^�I�����

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ���S�󎚃f�[�^�o�^�v��

	Lagtim( OPETCBNO, 6, 1*50 );							// ۺ��ް��o�^�����҂���ϰ�i�P�b�j�N��

	// ���S�f�[�^�o�^�I���҂�
	for( ; end_sts == 0 ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		switch( RcvMsg ){									// �C�x���g�H

			case TIMEOUT6:									// ۺ��ް��o�^�����҂���ϰ�i�Q�b�j��ѱ��

				if( (Logo_Reg_sts_rct == ON) && (Logo_Reg_sts_jnl == ON) ){
					// �o�^������ԁF�I��
					end_sts = 1;							// �o�^�I���҂���ʏI���i����I���j
				}
				else{
					// �o�^������ԁF���I��

					if( wait_cnt < 20 ){					// �ő�Q�O�b�܂œo�^�I���҂��Ƃ���

						if( wait_cnt < 15 ){
							grachr(6, (ushort)(wait_cnt*2), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[38]);	// �����҂��u���v�\��
						}
						wait_cnt++;							// �o�^�����҂��񐔁{�P
						Lagtim( OPETCBNO, 6, 1*50 );		// ۺ��ް��o�^�����҂���ϰ�i�P�b�j�N��
					}
					else{
						end_sts = 2;						// �o�^�I���҂���ʏI���i�ُ�I���j
					}
				}
				break;
		}
	}

	dispclr();												// ��ʃN���A
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);		// �޳�۰�����ٕ\��

	if( end_sts == 1 ){										// �o�^�������ʁH
		// ����I��
		BUZPI();
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[55]);					// [55]	"�@�_�E�����[�h�ƃv�����^�ւ́@"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[56]);					// [56]	"�@�f�[�^�o�^���I�����܂����B�@"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);					// [27]	"�@�@�@�@�@ ����I�� �@�@�@�@�@"
	}
	else{
		// �ُ�I��
		BUZPIPI();
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[57]);					// [57]	"�@�v�����^�ւ̃f�[�^�o�^�Ɂ@�@"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[58]);					// [58]	"�@���s���܂����B�@�@�@�@�@�@�@"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);					// [28]	"�@�@�@�@�@ �ُ�I�� �@�@�@�@�@"
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���S�󎚃f�[�^�v�����^�o�^����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SysMntLogoDataReg( )								|*/
/*|																			|*/
/*|	PARAMETER		:	*end_sts = �o�^�����I�����							|*/
/*|																			|*/
/*|	RETURN VALUE	:	MOD_EXT												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	SysMntLogoDataReg(void)
{
	T_FrmLogoRegist	FrmLogoRegist;		// ���S�󎚃f�[�^�o�^�v��ү���ލ쐬�ر
	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
	uchar			wait_cnt= 0;		// ���S�f�[�^�o�^�����҂���
	uchar			end_sts = 0;

	// ���S�f�[�^�o�^�v�����b�Z�[�W�쐬
	Logo_Reg_sts_rct = OFF;									// ۺޓo�^�������ؾ�āiڼ�āj
	Logo_Reg_sts_jnl = OFF;									// ۺޓo�^�������ؾ�āi�ެ��فj

	FrmLogoRegist.prn_kind = R_PRI;							// �������ʁFڼ��
	Logo_Reg_sts_jnl = ON;									// ۺޓo�^������ԁi�ެ��فj�F�o�^�I�����

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ���S�󎚃f�[�^�o�^�v��

	Lagtim( OPETCBNO, 6, 1*50 );							// ۺ��ް��o�^�����҂���ϰ�i�P�b�j�N��

	// ���S�f�[�^�o�^�I���҂�
	for( ; end_sts == 0 ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		switch( RcvMsg ){									// �C�x���g�H

			case TIMEOUT6:									// ۺ��ް��o�^�����҂���ϰ�i�Q�b�j��ѱ��

				if( (Logo_Reg_sts_rct == ON) && (Logo_Reg_sts_jnl == ON) ){
					// �o�^������ԁF�I��
					end_sts = 1;							// �o�^�I���҂���ʏI���i����I���j
				}
				else{
					// �o�^������ԁF���I��

					if( wait_cnt < 20 ){					// �ő�Q�O�b�܂œo�^�I���҂��Ƃ���
						wait_cnt++;							// �o�^�����҂��񐔁{�P
						Lagtim( OPETCBNO, 6, 1*50 );		// ۺ��ް��o�^�����҂���ϰ�i�P�b�j�N��
					}
					else{
						// �o�^�I���҂���ʏI���i�ُ�I���j
						end_sts = 2;						// �o�^�I���҂���ʏI���i�ُ�I���j
					}
				}
				break;
		}
	}

	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| �V�X�e�������e�i���X�F�J�[�h���s                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_CardIssue( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	SysMnt_CardIssue( void )
{
	ushort	usSysEvent;
	char	wk[2];
	ushort	ret = 0;
	ushort	msg;

	DP_CP[0] = DP_CP[1] = 0;

	dispclr();
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR2[24]);	/* "              ��              " */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[25]);			/* " �v���X�`�b�N�J�[�h(��ێ�)�� " */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[26]);			/* " �������݂���Ƃ��͕K��       " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[27]);			/* " ���C���[�_�[��SW6(�ێ��͐ݒ�)" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[28]);			/* " ��OFF�ɂ��Ă�������          " */
	Fun_Dsp( FUNMSG2[43] );													/* "�@�@�@�@�@�@ �m�F �@�@�@�@�@�@" */
	for ( ret = 0 ; ret == 0 ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F3:		/* �m�F(F3) */
			BUZPI();
			ret = MOD_EXT;
			break;
		default:
			break;
		}
	}
	ret = 0;

	for ( ret = 0 ; ret == 0 ; ){

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0] );		/* "���J�[�h���s���@�@�@�@�@�@�@�@" */

		usSysEvent = Menu_Slt( CARDMENU, CARD_ISU_TBL, (char)CARD_ISU_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usSysEvent) {
		/* 1.��� */
		case CARD_PASS:
			usSysEvent = card_iss_pass();
			break;
		/* 2.�W���J�[�h */
		case CARD_KKRI:
			usSysEvent = card_iss_kakari();
			break;
		/* 3.Mifare��� */
		case MOD_EXT:
			ret = MOD_EXT;
			break;
		default:
			break;
		}
		if (usSysEvent == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( usSysEvent == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}

	dispclr();
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR2[24]);	/* "              ��              " */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[27]);			/* " ���C���[�_�[��SW6(�ێ��͐ݒ�)" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[29] );			/* " ��ύX�����ꍇ��             " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[30] );			/* " �K�����ɖ߂��Ă�������       " */
	Fun_Dsp( FUNMSG2[43] );													/* "�@�@�@�@�@�@ �m�F �@�@�@�@�@�@" */
	ret = 0;
	for ( ret = 0 ; ret == 0 ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT) {	
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F3:		/* �m�F(F3) */
			BUZPI();
			return MOD_EXT;
			break;
		default:
			break;
		}
	}
	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| �J�[�h���s�F���                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : card_iss_pass( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
enum {
	CI_KIND = 0,
	CI_NO,
	CI_B_YEAR,
	CI_B_MONT,
	CI_B_DAY,
	CI_E_YEAR,
	CI_E_MONT,
	CI_E_DAY,
	CI_MONT,
	CI_DAY,
	CI_HOUR,
	CI_MIN,
	CI_STS,
	_CI_MAX_
};
static ushort	card_iss_pass(void)
{
	ushort		msg;
	long		prm[_CI_MAX_];		/* �ݒ�p�����[�^ */
	int			pos;
	char		mode;
	long		input;
	m_apspas	*outp;
	char		err_flag;
	uchar		tbl[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	uchar		str[4];
	uchar		ch[2];
	uchar		move_cmd = OFF;			// ���ގ捞�v������ޑ��M�׸�
	long		prm_wk;					// ���Ұ��ۑ�ܰ�
	uchar		IdSyuPara;				// �g�pID��ݒ����Ұ�
	uchar		sts_bit = 0;			// �f�B�b�v�X�C�b�`�`�F�b�N�r�b�g
	uchar		dipSW_getSts;			// DipSW �ݒ�擾���

	dipSW_getSts = FRdr_DipSW_GetStatus();		/* ���C���[�_�[��DipSW��Ԃ��擾 */

	OPE_red = 6;
	opr_snd( 90 );						// ذ�޺���ނ𖳌��Ƃ���ׁAýĺ���ނ𑗐M

	read_sht_opn();						// ������u�J�v

	prm[CI_KIND] = 1;
	prm[CI_NO] = 1;
	prm[CI_B_YEAR] = prm[CI_E_YEAR] = (long)(CLK_REC.year%100);
	prm[CI_B_MONT] = prm[CI_E_MONT] = prm[CI_MONT] = (long)CLK_REC.mont;
	prm[CI_B_DAY] = prm[CI_E_DAY] = prm[CI_DAY] = (long)CLK_REC.date;
	prm[CI_HOUR] = CLK_REC.hour;
	prm[CI_MIN] = CLK_REC.minu;
	prm[CI_STS] = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0]);			/* "���J�[�h���s���@�@�@�@�@�@�@�@" */
	grachr(0, 16, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "�@�@�@�@�@�@�@�@����@�@�@�@�@" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[1]);			/* "��ʁ@�@�@�@ ���No.�@�@�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[2]);			/* "�L���J�n�@�@�@�N�@�@���@�@���@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[3]);			/* "�L���I���@�@�@�N�@�@���@�@���@" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[4]);			/* "�����@�@�@�@���@�@���@�@�F�@�@" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[5]);			/* "�X�e�[�^�X�@�@�@�@�@�@�@�@�@�@" */
	/* �p�����[�^�\�� */
	cardiss_pass_prm_dsp(0, prm[0], 1);
	for (pos = 1; pos < _CI_MAX_; pos++) {
		cardiss_pass_prm_dsp(pos, prm[pos], 0);
	}
	Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */

	mode = 0;
	pos = 0;
	input = -1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if ( msg == KEY_MODECHG ) {
			BUZPI();
			rd_shutter();							// ���Cذ�ް���������
			if( move_cmd == ON ){
				opr_snd( 90 );						// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
			}
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ذ�޺����(���Ԍ��ۗ���)
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT ) {
			BUZPI();
			rd_shutter();							// ���Cذ�ް���������
			if( move_cmd == ON ){
				opr_snd( 90 );						// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
			}
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ذ�޺����(���Ԍ��ۗ���)
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		/* �p�����[�^�ݒ蒆 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				rd_shutter();						// ���Cذ�ް���������
				if( move_cmd == ON ){
					opr_snd( 90 );					// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				}
				OPE_red = 2;
				if(( RD_mod != 10 )&&( RD_mod != 11 )){
					opr_snd( 3 );					// ذ�޺����(���Ԍ��ۗ���)
				}
				return MOD_EXT;
			case KEY_TEN_F1:		/* ��(F1) */
			case KEY_TEN_F2:		/* ��(F2) */
				if (input != -1) {
					/* �L���͈̓`�F�b�N */
					prm_wk = prm[pos];				// ���͑O�����Ұ�����
					prm[pos] = input;				// ���͂��ꂽ���Ұ����
					if (cardiss_pass_prm_chk( prm )) {
						// ���Ұ������m�f
						prm[pos] = prm_wk;			// ���͑O�����Ұ��ɖ߂�
						BUZPIPI();
						cardiss_pass_prm_dsp(pos, prm[pos], 1);
						input = -1;
						break;
					}
				}
				BUZPI();
				/* �\��-���] */
				cardiss_pass_prm_dsp(pos, prm[pos], 0);
				if (pos == CI_STS) {
					Fun_Dsp( FUNMSG2[16] );			/* "�@���@�@���@�@�@�@ ���s  �I�� " */
				}
				/* �J�[�\���ړ� */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = (char)(_CI_MAX_-1);
				}
				else {
					if (++pos >= _CI_MAX_)
						pos = 0;
				}
				/* �\��-���] */
				cardiss_pass_prm_dsp(pos, prm[pos], 1);
				if (pos == CI_STS) {
					Fun_Dsp( FUNMSG2[18] );			/* "�@���@�@���@ �ؑց@���s  �I�� " */
					input = prm[pos];
				}
				else {
					input = -1;
				}
				break;
			case KEY_TEN_F3:		/* �ؑ�(F3) */
				if (pos == CI_STS) {
					BUZPI();
					if (++input > 3)
						input = 0;
					/* �\��-���] */
					cardiss_pass_prm_dsp(pos, input, 1);
				}
				break;
			case KEY_TEN_F4:		/* ���s(F4) */
				if (input != -1) {
					/* �L���͈̓`�F�b�N */
					prm_wk = prm[pos];				// ���͑O�����Ұ�����
					prm[pos] = input;				// ���͂��ꂽ���Ұ����
					if (cardiss_pass_prm_chk( prm )) {
						// ���Ұ������m�f
						prm[pos] = prm_wk;			// ���͑O�����Ұ��ɖ߂�
						BUZPIPI();
						cardiss_pass_prm_dsp(pos, prm[pos], 1);
						input = -1;
						break;
					}
				}
				BUZPI();
				/* �J�[�h��荞�ݗv�� */
				opr_snd(200);
				move_cmd = ON;						// ���ގ捞�v�����M�׸޾��
				/* �J�[�h���s��ʂ֑J�� */
				cardiss_pass_prm_dsp(pos, prm[pos], 0);
				grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
				Fun_Dsp( FUNMSG[82] );				/* "�@�@�@�@�@�@ ���~             " */
				mode = 1;
				err_flag = 0;
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				if (pos == CI_STS)
					break;
				BUZPI();
				if (input == -1)
					input = 0;
				if (pos == CI_NO)
					input = (input*10 + msg-KEY_TEN0) % 100000;
				else
					input = (input*10 + msg-KEY_TEN0) % 100;
				cardiss_pass_prm_dsp(pos, input, 1);
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				cardiss_pass_prm_dsp(pos, prm[pos], 1);
				if (pos == CI_STS) {
					input = prm[pos];
				}
				else {
					input = -1;
				}
				break;
			default:
				break;
			}
		}
		else if ( mode == 3 ) {
			/************************************************/
			/*		���~�{�^��������̃J�[�h�ʒu����		*/
			/************************************************/
			switch ( msg ) {
				case ARC_CR_E_EVT:						/* ���[�_�[���슮���C�x���g */
					Lagcan( OPETCBNO, 6 );
					mode = 0;
					if(dipSW_getSts == 1){				/* DipSW�ݒ��Ԏ擾�ς� */
						sts_bit = RDS_REC.state[1];		// ���C���[�_�[(DipSwitch SW2 �̒l�擾
						sts_bit &= 0x0F;				// ����4bit�̂ݗL��
						switch(sts_bit){
							case 0x03:					// �C�O3:ISO_Track2�N���W�b�g�Ή�(�㉺2����,�t�ʎ�t)
							case 0x04:					// ����:JIS�U�N���W�b�g�Ή�(�t�ʎ�t)
							case 0x05:					// ACI�����N���W�b�g�Ή�(�㉺2����,�t�ʎ�t,HR200 127�X�ܑΉ�)
								if(RED_REC.posi[0] & 0x07){		// ���C���[�_�[�����i�Z���T�[1,2,3�j�ɃJ�[�h���ʒu���Ă���ꍇ(�t�ʎ�t)�́A�ԋp�R�}���h�𔭍s����
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* ��ԗv����ϰ(2s)�N�� */
									mode = 4;						// �J�[�h�ԋp�R�}���h�̏I���҂���Ԃ�
								}
								break;
							default:
								if ( RED_REC.posi[0] & 0x70){	// ���C���[�_�[�����i�Z���T�[7,6,5�j�ɃJ�[�h���ʒu���Ă���ꍇ(���ʎ�t��)�́A�ԋp�R�}���h�𔭍s����
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* ��ԗv����ϰ(2s)�N�� */
									mode = 4;						// �J�[�h�ԋp�R�}���h�̏I���҂���Ԃ�
								}
								break;
						}
					}
					break;
				case TIMEOUT6:					/* �����҂���ϰ��ѱ�� */
					mode = 0;
					break;
				default:
					break;
			}
		}
		else if ( mode == 4 ) {
			/****************************************/
			/*		�J�[�h�ԋp�R�}���h�I���҂�		*/
			/****************************************/
			switch ( msg ) {
			case ARC_CR_E_EVT:				/* ���[�_�[���슮���C�x���g */
				Lagcan( OPETCBNO, 6 );
				mode = 0;					// �p�����[�^�ݒ��Ԃ�
				break;
			case TIMEOUT6:					/* �����҂���ϰ��ѱ�� */
				mode = 0;					// �p�����[�^�ݒ��Ԃ�
				break;
			default:
				break;
			}
		}
		/* �J�[�h���s�� */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���~(F3) */
				BUZPI();
				if (err_flag) {
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
					err_flag = 0;
					/* �J�[�h��荞�ݗv�� */
					opr_snd(200);
				}
				else {
					pos = 0;
					cardiss_pass_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
					Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */
					input = -1;
					mode = 3;
					opr_snd( 90 );						// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
					Lagtim( OPETCBNO, 6, 2*50 );		/* ��ԗv����ϰ(2s)�N�� */
				}
				break;

			case ARC_CR_EOT_RCMD:	/* �J�[�h������� */

				if( err_flag ){
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
					err_flag = 0;
					opr_snd(200);						/* ���ގ捞�v��						*/
				}
				break;

			case ARC_CR_E_EVT:		/* ���[�_�[���슮���C�x���g */
				/* �J�[�h��荞�ݏI�� */
				if (mode == 1) {
					/* ����f�[�^�쐬 */
					outp = (m_apspas *)MDP_buf;

					if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// �V�J�[�h�h�c�g�p����H
						(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APS�t�H�[�}�b�g�Ŏg�p����H

						// �V�J�[�h�h�c���g�p����ꍇ
						IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,11,1,1 );				// ��{���ԏ�No.��ID�i������j�ݒ����Ұ��擾
						switch( IdSyuPara ){

							case	1:													// ID1
								outp->aps_idc = 0x1A;
								break;
							case	2:													// ID2
								outp->aps_idc = 0x64;
								break;
							case	3:													// ID3
								outp->aps_idc = 0x6A;
								break;
							case	4:													// ID4
								outp->aps_idc = 0x70;
								break;
							case	5:													// ID5
								outp->aps_idc = 0x76;
								break;
							case	6:													// ID1��ID2
								outp->aps_idc = 0x1A;
								break;
							case	7:													// ID1��ID3
								outp->aps_idc = 0x1A;
								break;
							case	8:													// ID6
								outp->aps_idc = 0x44;
								break;
							case	0:													// �ǂ܂Ȃ�
							default:													// ���̑��i�ݒ�l�װ�j
								outp->aps_idc = 0x1A;
								break;
						}
					}
					else{
						// �V�J�[�h�h�c���g�p���Ȃ��ꍇ
						outp->aps_idc = 0x1A;											/* �J�[�h���(0x1A) */
					}
					outp->aps_pno[0] = (uchar)(CPrmSS[S_SYS][1] & 0x7FL);				/* ���ԏ�ԍ� ����7bit(bit6-bit0) */
					outp->aps_pno[1] = (uchar)(((CPrmSS[S_SYS][1] & 0x0080L) >> 1) |	/* bit6:���ԏ�ԍ�bit7 */
											   ((CPrmSS[S_SYS][1] & 0x0100L) >> 3) |	/* bit5:���ԏ�ԍ�bit8 */
											   ((CPrmSS[S_SYS][1] & 0x0200L) >> 5) |	/* bit4:���ԏ�ԍ�bit9 */
											   prm[CI_KIND]);							/* bit3-bit0:�������� */
					outp->aps_pcd[0] = (uchar)(prm[CI_NO] >> 7);						/* ������ԍ� ���7bit */
					outp->aps_pcd[1] = (uchar)(prm[CI_NO] & 0x7FL);						/*            ����7bit */
					outp->aps_sta[0] = (uchar)prm[CI_B_YEAR];							/* �L���J�n�N */
					outp->aps_sta[1] = (uchar)prm[CI_B_MONT];							/*         �� */
					outp->aps_sta[2] = (uchar)prm[CI_B_DAY];							/*         �� */
					outp->aps_end[0] = (uchar)prm[CI_E_YEAR];							/* �L���J�n�N */
					outp->aps_end[1] = (uchar)prm[CI_E_MONT];							/*         �� */
					outp->aps_end[2] = (uchar)prm[CI_E_DAY];							/*         �� */
					outp->aps_sts = (uchar)prm[CI_STS];									/* �X�e�[�^�X */
					outp->aps_wrt[0] = (uchar)prm[CI_MONT];								/* ������ */
					outp->aps_wrt[1] = (uchar)prm[CI_DAY];								/* �@�@�� */
					outp->aps_wrt[2] = (uchar)prm[CI_HOUR];								/* �@�@�� */
					outp->aps_wrt[3] = (uchar)prm[CI_MIN];								/* �@�@�� */
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g						
						outp->aps_pno[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x0400L) >> 3);/* bit7:���ԏ�ԍ�bit10 */
						outp->aps_pno[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x0800L) >> 4);/* bit7:���ԏ�ԍ�bit11 */
						outp->aps_pcd[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x1000L) >> 5);/* bit7:���ԏ�ԍ�bit12 */
						outp->aps_pcd[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x2000L) >> 6);/* bit7:���ԏ�ԍ�bit13 */
						outp->aps_sta[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x4000L) >> 7);/* bit7:���ԏ�ԍ�bit14 */
						outp->aps_sta[1] |= (uchar)((CPrmSS[S_SYS][1] & 0x8000L) >> 8);/* bit7:���ԏ�ԍ�bit15 */
						outp->aps_sta[2] |= (uchar)((CPrmSS[S_SYS][1] & 0x10000L) >> 9);/* bit7:���ԏ�ԍ�bit16 */
						outp->aps_end[0] |= (uchar)((CPrmSS[S_SYS][1] & 0x20000L) >> 10);/* bit7:���ԏ�ԍ�bit17 */
						MDP_buf[127] = 1;		//GT�t�H�[�}�b�g�쐬�t���O
					}else{
						MDP_buf[127] = 0;		//GT�t�H�[�}�b�g�쐬�t���O
					}
					MDP_mag = sizeof( m_apspas );
					MDP_siz = sizeof(m_apspas);
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g						
						md_pari2((uchar *)MDP_buf, (ushort)1, 1);								/* �p���e�B�쐬 */
					}else{
						md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 0);								/* �p���e�B�쐬 */
					}
					/* ����f�[�^�������� */
					opr_snd(201);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[8]);		/* "�@�@�@�@���������ł����@�@�@�@" */
					mode = 2;
				}
				/* �J�[�h�������ݏI�� */
				else {
					/* �����m�f */
					if (RED_REC.ercd) {
						ch[0] = tbl[RED_REC.ercd >> 4 & 0x0F];
						ch[1] = tbl[RED_REC.ercd & 0x0F];
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[7]);		/* "�@�@�@�@�����m�f�i�d�@�@�j�@�@" */
						as1chg(ch, str, 2);
						grachr(6, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str);						/* �G���[�R�[�h */
						err_flag = 1;
					}
					/* �����n�j */
					else {
						/* ���No.���P���₷ */
						if (++prm[CI_NO] > 12000)
							prm[CI_NO] = 1;
						cardiss_pass_prm_dsp(CI_NO, prm[CI_NO], 0);
						grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
						/* �J�[�h��荞�ݗv�� */
						opr_snd(200);
					}
					mode = 1;
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ����|�p�����[�^�\��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_pass_prm_dsp( pos, prm, rev )                   |*/
/*| PARAMETER    : char   pos : �\�����ڔԍ�                               |*/
/*|              : long   prm : �f�[�^���e                                 |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	cardiss_pass_prm_dsp(char pos, long prm, ushort rev)
{
	ushort	line;
	ushort	lpos;
	ushort	keta;
	const uchar	*pstr;

	if (pos == CI_STS) {
		if (prm == 0)
			pstr = DAT2_6[18];	/* "���s" */
		else if (prm == 1)
			pstr = DAT2_6[15];	/* "����" */
		else if (prm == 2)
			pstr = DAT2_6[14];	/* "�o��" */
		else
			pstr = DAT2_6[19];	/* "���Z" */
		grachr(5, 12, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, pstr);
	}
	else {
		keta = 2;
		switch (pos) {
		case CI_KIND:
			line = 1;
			lpos = 4;
			break;
		case CI_NO:
			keta = 5;
			line = 1;
			lpos = 20;
			break;
		case CI_B_YEAR:
			line = 2;
			lpos = 10;
			break;
		case CI_B_MONT:
			line = 2;
			lpos = 16;
			break;
		case CI_B_DAY:
			line = 2;
			lpos = 22;
			break;
		case CI_E_YEAR:
			line = 3;
			lpos = 10;
			break;
		case CI_E_MONT:
			line = 3;
			lpos = 16;
			break;
		case CI_E_DAY:
			line = 3;
			lpos = 22;
			break;
		case CI_MONT:
			line = 4;
			lpos = 8;
			break;
		case CI_DAY:
			line = 4;
			lpos = 14;
			break;
		case CI_HOUR:
			line = 4;
			lpos = 20;
			break;
		default:	/* case CI_MIN: */
			line = 4;
			lpos = 26;
			break;
		}
		opedpl(line, lpos, (ulong)prm, keta, 1, rev, COLOR_BLACK,  LCD_BLINK_OFF);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ����|�p�����[�^�`�F�b�N                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_pass_prm_chk( *prm )                            |*/
/*| PARAMETER    : long   *prm : ���Ұ��߲��                               |*/
/*|                                                                        |*/
/*| RETURN VALUE : 0:OK                                                    |*/
/*|              : 1:NG                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	cardiss_pass_prm_chk( long *prm )
{
	short	para_data[_CI_MAX_];		// �ݒ����Ұ�ܰ�
	uchar	i;							// ٰ�߶����

	// ���Ұ��ް��擾
	for( i=0 ; i<_CI_MAX_ ; i++ ){
		para_data[i] = (short)*prm;
		prm++;
	}
	// �L���J�n�N�ް��ϊ��i 80�`99=1980�`1999 / 00�`79=2000�`2079 �j
	if( para_data[CI_B_YEAR] < 80 ){
		para_data[CI_B_YEAR] += 2000;
	}
	else{
		para_data[CI_B_YEAR] += 1900;
	}
	// �L���I���N�ް��ϊ��i 80�`99=1980�`1999 / 00�`79=2000�`2079 �j
	if( para_data[CI_E_YEAR] < 80 ){
		para_data[CI_E_YEAR] += 2000;
	}
	else{
		para_data[CI_E_YEAR] += 1900;
	}
	// �������
	if( (para_data[CI_KIND] < 1) || (para_data[CI_KIND] > 15) ){
		return(1);
	}
	// ���No.����
	if( (para_data[CI_NO] < 1) || (para_data[CI_NO] > 12000) ){
		return(1);
	}
	// �L���J�n���t����
	if( ( (para_data[CI_B_MONT] == 0) || (para_data[CI_B_DAY] == 0) )
			||
		( chkdate( para_data[CI_B_YEAR], para_data[CI_B_MONT], para_data[CI_B_DAY] ) != 0 ) ){
		return(1);
	}
	// �L���I�����t����
	if( ( (para_data[CI_E_MONT] == 0) || (para_data[CI_E_DAY] == 0) )
			||
		( chkdate( para_data[CI_E_YEAR], para_data[CI_E_MONT], para_data[CI_E_DAY] ) != 0 ) ){
		return(1);
	}
	// �������t����
	if( ( (para_data[CI_MONT] == 0) || (para_data[CI_DAY] == 0) )
			||
		( chkdate( (short)CLK_REC.year, para_data[CI_MONT], para_data[CI_DAY] ) != 0 ) ){
		return(1);
	}
	// ������������
	if( (para_data[CI_HOUR] > 23) || (para_data[CI_MIN] > 59) ){
		return(1);
	}

	// �����I���i���Ұ�����j
	return(0);
}

/*[]----------------------------------------------------------------------[]*/
/*| �J�[�h���s�F�W���J�[�h                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : card_iss_kakari( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
enum {
	CIK_KNO = 0,
	CIK_TYPE,
	CIK_ROLE,
	CIK_LEVEL,
	_CIK_MAX_
};
static ushort	card_iss_kakari(void)
{
	ushort		msg;
	char		prm[_CIK_MAX_];		/* �ݒ�p�����[�^ */
	int			pos;
	char		mode;
	char		input;
	m_kakari	*outp;
	char		err_flag;
	uchar		str[4];
	uchar		ch[2];
	uchar		type_tbl[] = {0x20, 0x49, 0x4b, 0x53, 0x4a};	/* ' ', 'I', 'K', 'S', 'J' */
	uchar		tbl[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	uchar		move_cmd = OFF;			// ���ގ捞�v������ޑ��M�׸�
	uchar		IdSyuPara;				// �g�pID��ݒ����Ұ�
	uchar		sts_bit = 0;			// �f�B�b�v�X�C�b�`�`�F�b�N�r�b�g
	uchar		dipSW_getSts;			// DipSW �ݒ�擾���

	dipSW_getSts = FRdr_DipSW_GetStatus();		/* ���C���[�_�[��DipSW��Ԃ��擾 */
	
	OPE_red = 6;
	opr_snd( 90 );						// ذ�޺���ނ𖳌��Ƃ���ׁAýĺ���ނ𑗐M

	read_sht_opn();						// ������u�J�v

	prm[CIK_KNO] = 1;
	prm[CIK_TYPE] = 2;
	prm[CIK_ROLE] = 1;
	prm[CIK_LEVEL] = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[0]);			/* "���J�[�h���s���@�@�@�@�@�@�@�@" */
	grachr(0, 16, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[10]);		/* "�@�@�@�@�@�@�@�@�W���J�[�h�@�@" */
	grachr(1, 0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[22]);			/* "�W��No.�@�@�@�@�@�@ �@�@�@�@�@" */
	grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[6]);			/* "�J�[�h�^�C�v�@�@�@�@�@�@�@�@�@" */
	grachr(3, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[20]);			/* "�����@�@�@�@�@�@�@�@�@�@�@�@�@" */
	grachr(4, 0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[15]);			/* "���x���@�@�@�@�@�@�@�@�@�@�@�@" */
	/* �p�����[�^�\�� */
	cardiss_kakari_prm_dsp(0, prm[0], 1);
	for (pos = 1; pos < _CIK_MAX_; pos++) {
		cardiss_kakari_prm_dsp(pos, prm[pos], 0);
	}
	Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */

	mode = 0;
	pos = 0;
	input = (char)-1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			rd_shutter();							// ���Cذ�ް���������
			if( move_cmd == ON ){
				opr_snd( 90 );						// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
			}	
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ذ�޺����(���Ԍ��ۗ���)
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg ==  LCD_DISCONNECT ) {
			BUZPI();
			rd_shutter();							// ���Cذ�ް���������
			if( move_cmd == ON ){
				opr_snd( 90 );						// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
			}	
			OPE_red = 2;
			if(( RD_mod != 10 )&&( RD_mod != 11 )){
				opr_snd( 3 );						// ذ�޺����(���Ԍ��ۗ���)
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		/* �p�����[�^�ݒ蒆 */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				rd_shutter();						// ���Cذ�ް���������
				if( move_cmd == ON ){
					opr_snd( 90 );					// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				}
				OPE_red = 2;
				if(( RD_mod != 10 )&&( RD_mod != 11 )){
					opr_snd( 3 );					// ذ�޺����(���Ԍ��ۗ���)
				}
				return MOD_EXT;
			case KEY_TEN_F1:		/* ��(F1) */
			case KEY_TEN_F2:		/* ��(F2) */
				if (input != (char)-1) {
					/* �L���͈̓`�F�b�N */
					if (cardiss_kakari_prm_chk(pos, input)) {
						BUZPIPI();
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						input = (char)-1;
						break;
					}
					/* �p�����[�^�X�V */
					prm[pos] = input;
				}
				BUZPI();
				/* �\��-���] */
				cardiss_kakari_prm_dsp(pos, prm[pos], 0);
				if (pos == CIK_TYPE) {
					Fun_Dsp( FUNMSG2[16] );			/* "�@���@�@���@�@�@�@ ���s  �I�� " */
				}
				/* �J�[�\���ړ� */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = (char)(_CIK_MAX_-1);
				}
				else {
					if (++pos >= _CIK_MAX_)
						pos = 0;
				}
				/* �\��-���] */
				cardiss_kakari_prm_dsp(pos, prm[pos], 1);
				if (pos == CIK_TYPE) {
					Fun_Dsp( FUNMSG2[18] );			/* "�@���@�@���@ �ؑց@���s  �I�� " */
					input = prm[pos];
				}
				else {
					input = (char)-1;
				}
				break;
			case KEY_TEN_F3:		/* �ؑ�(F3) */
				if (pos == CIK_TYPE) {
					BUZPI();
					if (input == 2) {	// 2:��[�J�[�h
						input = 0;		// 0:�W���J�[�h
					}
					else{
						input = 2;		// 2:��[�J�[�h
					}
					/* �\��-���] */
					cardiss_kakari_prm_dsp(pos, input, 1);
				}
				break;
			case KEY_TEN_F4:		/* ���s(F4) */
				if (input != (char)-1) {
					/* �L���͈̓`�F�b�N */
					if (cardiss_kakari_prm_chk(pos, input)) {
						BUZPIPI();
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						input = (char)-1;
						break;
					}
					/* �p�����[�^�X�V */
					prm[pos] = input;
				}
				BUZPI();
				/* �J�[�h��荞�ݗv�� */
				opr_snd(200);
				move_cmd = ON;						// ���ގ捞�v�����M�׸޾��
				/* �J�[�h���s��ʂ֑J�� */
				cardiss_kakari_prm_dsp(pos, prm[pos], 0);
				grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
				Fun_Dsp( FUNMSG[82] );				/* "�@�@�@�@�@�@ ���~             " */
				mode = 1;
				err_flag = 0;
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				if (pos == CIK_TYPE)
					break;
				BUZPI();
				if (input == (char)-1)
					input = 0;
				if (pos == CIK_KNO)
					input = (char)((input*10 + msg-KEY_TEN0) % 100);
				else
					input = (char)(msg - KEY_TEN0);
				cardiss_kakari_prm_dsp(pos, input, 1);
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				cardiss_kakari_prm_dsp(pos, prm[pos], 1);
				if (pos == CIK_TYPE) {
					input = prm[pos];
				}
				else {
					input = (char)-1;
				}
				break;
			default:
				break;
			}
		}
		else if ( mode == 3 ) {
			/************************************************/
			/*		���~�{�^��������̃J�[�h�ʒu����		*/
			/************************************************/
			switch ( msg ) {
				case ARC_CR_E_EVT:						/* ���[�_�[���슮���C�x���g */
					Lagcan( OPETCBNO, 6 );
					mode = 0;
					if(dipSW_getSts == 1){				/* DipSW�ݒ��Ԏ擾�ς� */
						sts_bit = RDS_REC.state[1];		// ���C���[�_�[(DipSwitch SW2 �̒l�擾
						sts_bit &= 0x0F;				// ����4bit�̂ݗL��
						switch(sts_bit){
							case 0x03:					// �C�O3:ISO_Track2�N���W�b�g�Ή�(�㉺2����,�t�ʎ�t)
							case 0x04:					// ����:JIS�U�N���W�b�g�Ή�(�t�ʎ�t)
							case 0x05:					// ACI�����N���W�b�g�Ή�(�㉺2����,�t�ʎ�t,HR200 127�X�ܑΉ�)
								if(RED_REC.posi[0] & 0x07){		// ���C���[�_�[�����i�Z���T�[1,2,3�j�ɃJ�[�h���ʒu���Ă���ꍇ(�t�ʎ�t)�́A�ԋp�R�}���h�𔭍s����
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* ��ԗv����ϰ(2s)�N�� */
									mode = 4;						// �J�[�h�ԋp�R�}���h�̏I���҂���Ԃ�
								}
								break;
							default:
								if ( RED_REC.posi[0] & 0x70){	// ���C���[�_�[�����i�Z���T�[7,6,5�j�ɃJ�[�h���ʒu���Ă���ꍇ(���ʎ�t��)�́A�ԋp�R�}���h�𔭍s����
									opr_snd(2);
									Lagtim( OPETCBNO, 6, 2*50 );	/* ��ԗv����ϰ(2s)�N�� */
									mode = 4;						// �J�[�h�ԋp�R�}���h�̏I���҂���Ԃ�
								}
								break;
						}
					}
					break;
				case TIMEOUT6:					/* �����҂���ϰ��ѱ�� */
					mode = 0;
					break;
				default:
					break;
			}
		}
		else if ( mode == 4 ) {
			/****************************************/
			/*		�J�[�h�ԋp�R�}���h�I���҂�		*/
			/****************************************/
			switch ( msg ) {
			case ARC_CR_E_EVT:				/* ���[�_�[���슮���C�x���g */
				Lagcan( OPETCBNO, 6 );
				mode = 0;					// �p�����[�^�ݒ��Ԃ�
				break;
			case TIMEOUT6:					/* �����҂���ϰ��ѱ�� */
				mode = 0;					// �p�����[�^�ݒ��Ԃ�
				break;
			default:
				break;
			}
		}
		/* �J�[�h���s�� */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���~(F3) */
				BUZPI();
				if (err_flag) {
					grachr(6, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[6]);		/* "�@�@�J�[�h��}�����ĉ������@�@" */
					err_flag = 0;
					/* �J�[�h��荞�ݗv�� */
					opr_snd(200);
				}
				else {
					pos = 0;
					cardiss_kakari_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);	/* �s�N���A */
					Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */
					input = (char)-1;
					mode = 3;
					opr_snd( 90 );							// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
					Lagtim( OPETCBNO, 6, 2*50 );		/* ��ԗv����ϰ(2s)�N�� */
				}
				break;

			case ARC_CR_EOT_RCMD:	/* �J�[�h������� */

				if( err_flag ){
					pos = 0;
					cardiss_kakari_prm_dsp(pos, prm[pos], 1);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
					Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */
					input = (char)-1;
					mode = 0;
					opr_snd( 90 );							// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				}
				break;

			case ARC_CR_E_EVT:		/* ���[�_�[���슮���C�x���g */
				/* �J�[�h��荞�ݏI�� */
				if (mode == 1) {
					/* �W���J�[�h�f�[�^�쐬 */
					outp = (m_kakari *)MDP_buf;

					if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// �V�J�[�h�h�c�g�p����H
						(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APS�t�H�[�}�b�g�Ŏg�p����H

						// �V�J�[�h�h�c���g�p����ꍇ
						IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,10,1,1 );				// ��{���ԏ�No.��ID�i�W�����ށj�ݒ����Ұ��擾
						switch( IdSyuPara ){

							case	1:													// ID1
								outp->kkr_idc = 0x41;
								break;
							case	2:													// ID2
								outp->kkr_idc = 0x69;
								break;
							case	3:													// ID3
								outp->kkr_idc = 0x6F;
								break;
							case	4:													// ID4
								outp->kkr_idc = 0x75;
								break;
							case	5:													// ID5
								outp->kkr_idc = 0x7B;
								break;
							case	6:													// ID1��ID2
								outp->kkr_idc = 0x41;
								break;
							case	7:													// ID1��ID3
								outp->kkr_idc = 0x41;
								break;
							case	8:													// ID6
								outp->kkr_idc = 0x7E;
								break;
							case	0:													// �ǂ܂Ȃ�
							default:													// ���̑��i�ݒ�l�װ�j
								outp->kkr_idc = 0x41;
								break;
						}
					}
					else{
						// �V�J�[�h�h�c���g�p���Ȃ��ꍇ
						outp->kkr_idc = 0x41;
					}
					outp->kkr_year[0] = (uchar)(CLK_REC.year%100/10 + 0x30);
					outp->kkr_year[1] = (uchar)(CLK_REC.year%10 + 0x30);
					outp->kkr_mon[0] = (uchar)(CLK_REC.mont/10 + 0x30);
					outp->kkr_mon[1] = (uchar)(CLK_REC.mont%10 + 0x30);
					outp->kkr_day[0] = (uchar)(CLK_REC.date/10 + 0x30);
					outp->kkr_day[1] = (uchar)(CLK_REC.date%10 + 0x30);
					outp->kkr_did = 0x53;
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g
						outp->kkr_park[0] = 0x50;	/* 'P' */
						outp->kkr_park[1] = 0x47;	/* 'G' */
						outp->kkr_park[2] = 0x54;	/* 'T' */
					}else{//APS�t�H�[�}�b�g
						outp->kkr_park[0] = 0x50;	/* 'P' */
						outp->kkr_park[1] = 0x41;	/* 'A' */
						outp->kkr_park[2] = 0x4B;	/* 'K' */
					}
					outp->kkr_role = (uchar)(prm[CIK_ROLE] + 0x30);
					outp->kkr_lev = (uchar)(prm[CIK_LEVEL] + 0x30);
					outp->kkr_type = type_tbl[prm[CIK_TYPE]];
					outp->kkr_kno[0] = 0x30;
					outp->kkr_kno[1] = 0x30;
					outp->kkr_kno[2] = (uchar)(prm[CIK_KNO]/10 + 0x30);
					outp->kkr_kno[3] = (uchar)(prm[CIK_KNO]%10 + 0x30);
					memset(&outp->kkr_jdg, 0x30, sizeof(outp->kkr_jdg));
					memset(outp->kkr_rsv1, 0x20, sizeof(outp->kkr_rsv1));
					memset(outp->kkr_rsv2, 0x20, sizeof(outp->kkr_rsv2));
					memset(outp->kkr_rsv3, 0x20, sizeof(outp->kkr_rsv3));
					if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g
						outp->kkr_rsv2[4] = (uchar)(CPrmSS[S_SYS][1]/100000 + 0x30);
						outp->kkr_rsv2[5] = (uchar)(CPrmSS[S_SYS][1]%100000/10000 + 0x30);
						outp->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]%10000/1000 + 0x30);
						outp->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
						outp->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
						outp->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
						MDP_buf[127] = 1;
					}else{//APS�t�H�[�}�b�g
						outp->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]/1000 + 0x30);
						outp->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
						outp->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
						outp->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
						MDP_buf[127] = 0;
					}
					MDP_mag = sizeof( m_kakari );
					MDP_siz = sizeof(m_kakari);
					md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 1);								/* �p���e�B�쐬(�������è) */
					/* �W���J�[�h�������� */
					opr_snd(202);
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[8]);		/* "�@�@�@�@���������ł����@�@�@�@" */
					mode = 2;
				}
				/* �J�[�h�������ݏI�� */
				else {
					/* �����m�f */
					if (RED_REC.ercd) {
						ch[0] = tbl[RED_REC.ercd >> 4 & 0x0F];
						ch[1] = tbl[RED_REC.ercd & 0x0F];
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR2[7]);		/* "�@�@�@�@�����m�f�i�d�@�@�j�@�@" */
						as1chg(ch, str, 2);
						grachr(6, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str);			/* �G���[�R�[�h */
						err_flag = 1;
						mode = 1;
					}
					/* �����n�j */
					else {
						pos = 0;
						cardiss_kakari_prm_dsp(pos, prm[pos], 1);
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
						Fun_Dsp( FUNMSG2[16] );					/* "�@���@�@���@�@�@�@ ���s  �I�� " */
						input = (char)-1;
						mode = 0;
						opr_snd( 90 );							// ���ގ捞�v���𖳌��Ƃ���ׁAýĺ���ނ𑗐M
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �W���J�[�h�|�p�����[�^�\��                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_kakari_prm_dsp( pos, prm, rev )                 |*/
/*| PARAMETER    : char   pos : �\�����ڔԍ�                               |*/
/*|              : char   prm : �f�[�^���e                                 |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	cardiss_kakari_prm_dsp(char pos, char prm, ushort rev)
{
	switch (pos) {
	case CIK_KNO:
		opedsp(1, 7, (ushort)prm, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );		/* �W��No. */
		break;
	case CIK_TYPE:
		if (prm == 0) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[10]);		/* "�W���J�[�h" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@" */
		}
		else if (prm == 1) {
			grachr(2, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[5]);		/* "�C���x���g��" */
			grachr(2, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@" */
		}
		else if (prm == 2) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[11]);		/* "��[�J�[�h" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@" */
		}
		else if (prm == 3) {
			grachr(2, 14, 10, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[12]);		/* "���v�J�[�h" */
			grachr(2, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@" */
		}
		else {
			grachr(2, 14, 16, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_1[1]);		/* "�����I�����J�[�h" */
		}
		break;
	case CIK_ROLE:
		opedsp(3, 8, (ushort)prm, 1, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
		break;
	default:	/* case CIK_LEVEL: */
		opedsp(4, 8, (ushort)prm, 1, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* ���x�� */
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �W���J�[�h�|�p�����[�^�`�F�b�N                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cardiss_kakari_prm_chk( pos, prm )                      |*/
/*| PARAMETER    : char   pos : �\�����ڔԍ�                               |*/
/*|              : char   prm : �f�[�^���e                                 |*/
/*| RETURN VALUE : 0:OK                                                    |*/
/*|              : 1:NG                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	cardiss_kakari_prm_chk(char pos, char prm)
{
	switch (pos) {
	case CIK_KNO:
		if (prm < 1){
			return 1;
		}
		break;
	case CIK_TYPE:
		if (prm > 4){
			return 1;
		}
		break;
	case CIK_ROLE:
		if (prm < 1 || prm > 3){
			return 1;
		}
		break;
	case CIK_LEVEL:
		if (prm > 4){
			return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���C���[�_�[DipSW�ݒ�擾����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdr_DipSW_GetStatus()   			                   |*/
/*| PARAMETER    : void 					            	               |*/
/*| RETURN VALUE : uchar	dipSW_getSts                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.takeuchi                                              |*/
/*| Date         : 2012/10/19                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	FRdr_DipSW_GetStatus( void )
{
	
	short		msg;
	uchar dipSWSts;

	dipSWSts = 0;

	if( opr_snd( 95 ) == 0 ){ 				/* ��ԗv�� */
		Lagtim( OPETCBNO, 6, 2*50 );		/* ��ԗv����ϰ(2s)�N�� */
	}
	else{
		return dipSWSts;
	}
	
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );		/* ү���ގ�M */

		switch( msg){						/* ��Mү���ށH */
			case ARC_CR_E_EVT:				/* �I������ގ�M */
				Lagcan( OPETCBNO, 6 );
				if(RDS_REC.result == 0){
					dipSWSts = 1;			/* DipSW�ݒ��Ԏ擾���� */
				}
			case TIMEOUT6:					/* �����҂���ϰ��ѱ�� */
				return dipSWSts;
			default:
				break;
		}
	}
	return dipSWSts;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�|�C���^�擾�����i�N���W�b�g���p���׃v�����g�p�j			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogPtrGet_M( req )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req	= �����v��								|*/
/*|										�O�F�Ō��ް��߲���擾				|*/
/*|										�P�F�ŐV�ް��߲���擾				|*/
/*|					:	uchar	type = ���׃^�C�v							|*/
/*|							CREDIT_CARD/CREDIT_iD/...�i�ȉ��ǉ��j			|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= ۸��ް��߲���i�z��ԍ��j				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Yanase(COSMO)												|*/
/*|	Date	:	2006-07-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
ushort	LogPtrGet_M( uchar req, uchar type )
{
	ushort	posi = 0;
	return(posi);
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�����\����ʏ����R�i�󎚑Ώۊ��Ԏw�肠��j
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ���O���
///	@param[in]		LogCnt	: ���O����
///	@param[in]		title	: �^�C�g���\���f�[�^�|�C���^
///	@param[in]		PreqCmd	: �󎚗v��ү���޺����
///	@param[in]		NewOldDate	: �ŌÁ��ŐV�ް����t�ް��߲��
///	@return			����Ӱ��(MOD_CHG/MOD_EXT)
///	@note			SysMnt_Log_CntDsp2()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
#define		_POS_MAX	6		// �J�[�\���ړ�MAX�l
#define		_POS_SYEAR	0		// �J�n"�N"�ʒu
#define		_POS_EYEAR	3		// �I��"�N"�ʒu
// �\�����ږ��̕\�����@�ް�ð��قP�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA_TIME1[_POS_MAX][4] = {
	{ 4,  6, 2 ,1 },	// �J�n�i�N�j
	{ 4, 12, 2 ,0 },	// �J�n�i���j
	{ 4, 18, 2, 0 },	// �J�n�i���j
	{ 5,  6, 2, 1 },	// �I���i�N�j
	{ 5, 12, 2, 0 }, 	// �I���i���j
	{ 5, 18, 2, 0 },	// �I���i���j
};

//[]----------------------------------------------------------------------[]
///	@brief			���O�����\����ʏ����S�i�󎚑Ώۊ��Ԏw�肠��j
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ���O���
///	@param[in]		title	: �^�C�g���\���f�[�^�|�C���^
///	@param[in]		PreqCmd	: �󎚗v��ү���޺����
///	@return			����Ӱ��(MOD_CHG/MOD_EXT)
///	@note			SysMnt_Log_CntDsp2_Time()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Ise
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	SysMnt_Log_CntDsp3(ushort LogSyu, const uchar *title, ushort PreqCmd)
{
	uchar			pos		= 0;			// �����ް����͈ʒu�i�O�F�J�n�N�A�P�F�J�n���A�Q�F�J�n���A�R�F�J�n���A
											//					 �S�F�I���N�A�T�F�I�����A�U�F�I�����A�V�F�I�����j
	uchar			All_Req	= OFF;			// �u�S�āv�w�蒆�׸�
	uchar			Date_Chk;				// ���t�w���ް������׸�
	short			inp		= -1;			// �����ް�
	ushort			RcvMsg;					// ��Mү���ފi�[ܰ�
	ushort			pri_cmd	= 0;			// �󎚗v������ފi�[ܰ�
	ushort			NewOldDate[_POS_MAX];	// ���t�ް��i[0]�F�J�n�N�A[1]�F�J�n���A[2]�F�J�n���A[3]�F�J�n���A
	ushort			Date_Now[_POS_MAX];		// 			 [4]�F�I���N�A[5]�F�I�����A[6]�F�I�����A[7]�F�I�����j
	ushort			Sdate;					// �J�n���t
	ushort			Edate;					// �I�����t
	ushort			wks, wks2;
	ushort			LogMax, LogCnt;		// Log�ő匏��
	ushort			FstIdx = 0;			// FlashROM�����ōŏ��Ɉ�v�����ԍ�
	ushort			LstIdx = 0;			// FlashROM�����ōŌ�Ɉ�v�����ԍ�
	date_time_rec	NewestDateTime;		// �����Ɉ�v�������̍ł��V�������Z����
	date_time_rec	OldestDateTime;		// �����Ɉ�v�������̍ł��Â����Z����
	T_FrmLogPriReq4	FrmLogPriReq4;		// �󎚗v��ү����ܰ�(�W���p)
	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
	ushort			FstIdxAll;			// FlashROM�����ōŏ��Ɉ�v�����ԍ�(�S�Ėt�p)
	ushort			LstIdxAll;			// FlashROM�����ōŌ�Ɉ�v�����ԍ�(�S�Ėt�p)
	ushort			i;					// ���[�v�ϐ�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	ulong			start_time;
	ulong			past_time;
	
	start_time = LifeTimGet();
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

	
	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
	LogMax = Ope2_Log_NewestOldestDateGet_AttachOffset( LogSyu, &NewestDateTime, &OldestDateTime, &FstIdx, &LstIdx);
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	// ���O��������ʂ��Œ�1�b�ԕ\��
	switch (LogSyu) {
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	case LOG_ECMINASHI:
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	case LOG_ECMEISAI:
		past_time = LifePastTimGet( start_time );
		if (past_time < 100UL) {
			// �t�@���N�V�����L�[���\�������O�̂��ߓ��e�Ȃ��ŕ\�����Ă���
			Fun_Dsp( FUNMSG[0] );											// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
			Lagtim(OPETCBNO, 6, (ushort)((1000UL - (past_time*10)) / 20));	// ����ް����ϰ�N��
			for( ;; ){
				RcvMsg = StoF(GetMessage(), 1 );
				if (RcvMsg == TIMEOUT6) {
					// ����ް����ϰ��ѱ��
					break;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if (RcvMsg == KEY_MODECHG) {
				if (RcvMsg == KEY_MODECHG || RcvMsg == LCD_DISCONNECT) {	// ���[�h�`�F���W�������͐ؒf�ʒm
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return RcvMsg;
				}
			}
		}
		break;
	default:
		break;
	}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	
	// �ŌÁ��ŐV���t�ް��擾(���҂̔N�������������đ�O�����Ɋi�[)
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
	FstIdxAll = FstIdx;					// �S�Ėt�p�ϐ��ɍŏ���v�ԍ��𔽉f
	LstIdxAll = LstIdx;					// �S�Ėt�p�ϐ��ɍŌ��v�ԍ��𔽉f


	LogCnt = LogMax;										// Log�����K�p
	memcpy( Date_Now, NewOldDate, sizeof(Date_Now) );

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );							// ���O���v�����g�^�C�g���\��
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// �ŌÁ^�ŐV���O���t�\��
	LogCntDsp( LogCnt );									// ���O�����\��

	LogDateDsp4( &Date_Now[0] );							// �ŌÁ^�ŐV���O���t�f�[�^�\��
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
	LogDateDsp5( &Date_Now[0], pos );						// ������t�f�[�^�\��

	Fun_Dsp( FUNMSG[83] );									// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "

	for( ; ; ){

		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		if( pri_cmd == 0 ){

			// �󎚗v���O�i�󎚗v���O��ʁj

			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�

					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				case KEY_TEN:								// �O�`�X

					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						BUZPI();

						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}
						else{
							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
						}
						opedsp	(							// �����ް��\��
									POS_DATA5_1[pos][0],	// �\���s
									POS_DATA5_1[pos][1],	// �\�����
									(ushort)inp,			// �\���ް�
									POS_DATA5_1[pos][2],	// �ް�����
									POS_DATA5_1[pos][3],	// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
					}

					break;

				case KEY_TEN_CL:							// ����L�[

					BUZPI();

					if( All_Req == OFF ){					// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						opedsp	(							// ���͑O���ް���\��������
									POS_DATA5_1[pos][0],	// �\���s
									POS_DATA5_1[pos][1],	// �\�����
									Date_Now[pos],			// �\���ް�
									POS_DATA5_1[pos][2],	// �ް�����
									POS_DATA5_1[pos][3],	// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK,
									LCD_BLINK_OFF
								);
						inp = -1;							// ���͏�ԏ�����
					}
					else{
						//	�u�S�āv�w�蒆�̏ꍇ

						pos = 0;							// ���وʒu���J�n�i�N�j
						inp = -1;							// ���͏�ԏ�����
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
						LogDateDsp5( &Date_Now[0], pos );						// ������t�f�[�^�\��
						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
					}

					break;

				case KEY_TEN_F1:							// �e�P�i���j�L�[����
				case KEY_TEN_F2:							// �e�Q�i���j�L�[����	�����t���͒��̏ꍇ
															// �e�Q�i�N���A�j����	���u�S�āv�w�蒆�̏ꍇ

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H

						//	�u�S�āv�w�蒆�łȂ��ꍇ

						if( inp == -1 ){						// ���͂���H

							//	���͂Ȃ��̏ꍇ
							BUZPI();

							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
							}
							else{
								LogDatePosUp( &pos, 1 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
							}
						}
						else{
							//	���͂���̏ꍇ
							if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�H

								//	�����ް��n�j�̏ꍇ
								BUZPI();

								if( pos == 0 || pos == 3){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}
								else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
								}
								else{
									LogDatePosUp( &pos, 1 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
								}
							}
							else{
								//	�����ް��m�f�̏ꍇ
								BUZPIPI();
							}
						}
						/* �w��N�����͈͓����ް����ĕ\�� */
						LogDateDsp5( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
						inp = -1;								// ���͏�ԏ�����
					}

					break;

				case KEY_TEN_F3:							// �e�R�i�S�āj�L�[����

					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){						// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j

						if( LogCnt != 0 ){

							//	���O�f�[�^������ꍇ
							BUZPI();
							memcpy	(							// �ŌÁ��ŐV�ް����t���߰
										&Date_Now[0],
										NewOldDate,
										12
									);
							FstIdx = FstIdxAll;					// �ŏ���v�ԍ��𔽉f
							LstIdx = LstIdxAll;					// �Ō��v�ԍ��𔽉f
							pos = 0;							// ���وʒu���J�n�i�N�j
							inp = -1;							// ���͏�ԏ�����
							displclr( 5 );						// ������t�\���N���A
							LogCntDsp( LogCnt );				// ���O�����\��
							Fun_Dsp( FUNMSG[81] );				// "�@�@�@�@�@�@�@�@�@ ���s  �I�� "
							All_Req = ON;						// �u�S�āv�w�蒆�׸޾��
						}
						else{
							//	���O�f�[�^���Ȃ��ꍇ
							BUZPIPI();
						}
					}

					break;

				case KEY_TEN_F4:							// �e�S�i���s�j�L�[����

					displclr( 1 );							// 1�s�ڕ\���N���A
					Date_Chk = OK;

					if( inp != -1 ){						// ���͂���H

						//	���͂���̏ꍇ
						if( OK == LogDateChk2( pos, inp ) ){// �����ް��n�j�H

							//	���̓f�[�^�n�j�̏ꍇ
							if( pos == 0 || pos == 3){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}
							else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}
						else{
							//	���̓f�[�^�m�f�̏ꍇ
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						if( All_Req != ON ){						// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��

							//	�N���������݂�����t���`�F�b�N����

							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// �J�n���t����
								Date_Chk = NG;
							}
						}
					}
					if( ( All_Req == ON ) && ( Date_Chk == OK ) ){

						//	�J�n���t�����I�����t���`�F�b�N����

						Sdate = dnrmlzm(							// �J�n���t�ް��擾
											(short)Date_Now[_POS_SYEAR],
											(short)Date_Now[_POS_SYEAR+1],
											(short)Date_Now[_POS_SYEAR+2]
										);

						Edate = dnrmlzm(							// �I�����t�ް��擾
											(short)Date_Now[_POS_EYEAR],
											(short)Date_Now[_POS_EYEAR+1],
											(short)Date_Now[_POS_EYEAR+2]
										);

						if( Sdate > Edate ){	// �J�n���t�^�I�����t����
							Date_Chk = NG;
						}
					}
					if( Date_Chk == OK ){

						Sdate = dnrmlzm(							// �J�n���t�ް��擾
											(short)Date_Now[_POS_SYEAR],
											(short)Date_Now[_POS_SYEAR+1],
											(short)Date_Now[_POS_SYEAR+2]
										);
						wks = 0;
//						index = Ope_Log_TotalCountGet(eLOG_PAYMENT);
						if( All_Req == OFF ){
							wks2 = Ope2_Log_CountGet_inDate( eLOG_PAYMENT, &Date_Now[0], &FstIdx );
							for(i = 0; i < wks2; i++)					/* FlashROM����f�[�^��ǂݏo�����[�v */
							{
								// �ʐ��Z�f�[�^�P���Ǐo��
								Ope_Log_1DataGet(eLOG_PAYMENT, (ushort)(FstIdx + i), SysMnt_Work);	

// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�o�O�C���ڐA�j
//								if(is_match_receipt((Receipt_data*)&SysMnt_Work, Sdate, 0, LOG_SCAMEISAI))
								if(is_match_receipt((Receipt_data*)&SysMnt_Work, Sdate, 0, LogSyu))
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�o�O�C���ڐA�j
								{
									wks++;
								}
							}
							if( wks == 0 ){
								BUZPIPI();
								grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// �f�[�^�Ȃ��\��
								inp = -1;											// ���͏�ԏ�����
								break;
							}
						}
//						LogCntDsp( wks );		// ���O�����\��

						if (Ope_isPrinterReady() == 0) {		// ���V�[�g�o�͕s��
							BUZPIPI();
							break;
						}
						// �J�n���t���I�����t�����n�j�̏ꍇ
						BUZPI();
						/*------	�󎚗v��ү���ޑ��M	-----*/
						memset( &FrmLogPriReq4,0,sizeof(FrmLogPriReq4) );		// �󎚗v��ү����ܰ��O�ر
						FrmLogPriReq4.prn_kind	= R_PRI;						// �Ώ�������Fڼ��
						FrmLogPriReq4.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
						FrmLogPriReq4.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.
						if( All_Req == ON ){
							FrmLogPriReq4.BType	= 0;							// �������@	�F�S��
							FrmLogPriReq4.LogCount = LogMax;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
							FrmLogPriReq4.Ffst_no		= 0;
						}
						else {
							FrmLogPriReq4.BType	= 1;							// �������@	�F���t
							FrmLogPriReq4.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq4.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq4.TSttTime.Day	= (uchar)Date_Now[2];
							FrmLogPriReq4.LogCount = wks;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
							FrmLogPriReq4.Ffst_no		= FstIdx;
						}
						FrmLogPriReq4.Flst_no		= LstIdx;
						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq4), &FrmLogPriReq4 );
						Ope_DisableDoorKnobChime();
						pri_cmd = PreqCmd;					// ���M����޾���
						if( All_Req != ON ){					// �u�S�āv�w��Ȃ�
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
							LogDateDsp5( &Date_Now[_POS_SYEAR], 0xff );					// ������t�f�[�^�\��
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// ���s���u�����N�\��
						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

					}
					else{
						// �J�n���t���I�����t�����m�f�̏ꍇ
						BUZPIPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
						LogDateDsp5( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
						Fun_Dsp( FUNMSG[83] );					// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;							// �u�S�āv�w�蒆�׸�ؾ��
						inp = -1;								// ���͏�ԏ�����
					}

					break;

				case KEY_TEN_F5:							// �e�T�i�I���j�L�[����

					BUZPI();
					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
					RcvMsg = MOD_EXT;						// YES�F�O��ʂɖ߂�
			}

			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();
					/*------	�󎚒��~ү���ޑ��M	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );

					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�

					break;

				default:
					break;
			}

		}
		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
			break;											// �O��ʂɖ߂�
		}
	}
	return( RcvMsg );
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�����\����ʂ̈󎚏����őΏۃv�����^�𔻒f
//[]----------------------------------------------------------------------[]
///	@param[in]		LogSyu	: ���O���
///	@return			�v�����^���(J_PRI/R_PRI/RJ_PRI/0)
///	@note			SysMnt_Log_CntDspxx()�֐����ʗp<br>
///					�e��ʂŔ��f�ΏۂƂȂ�l���ُ�ȏꍇ��0��Ԃ��B<br>
//[]----------------------------------------------------------------------[]
///	@author			Ise
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static uchar	SysMnt_Log_Printer(ushort LogSyu)
{
	uchar prn_kind;											// ��������

	switch(LogSyu)											// ��ʖ��Ƀv�����^��ʂ𔻒f
	{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case LOG_EDYSYUUKEI:									// �d�����W�v
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case LOG_SCASYUUKEI:									// �r���������W�v
		switch(prm_get(COM_PRM, S_SCA, 100, 1, 1))			// 35-0100�E:�����W�v�󎚏o�͐�
		{
		case 3:												// �󎚐悪���V�[�g�ƃW���[�i��
			if((Ope_isJPrinterReady() == 1) && (Ope_isPrinterReady() == 1))
			{												// ���V�[�g�E�W���[�i�����󎚉\�ȏ��
				prn_kind	= RJ_PRI;						// �Ώ�������Fڼ��&�ެ���
				break;
			}
			if(Ope_isJPrinterReady() == 1)					// �W���[�i���v�����^���󎚉\�ȏ��
			{
				prn_kind	= J_PRI;						// �Ώ�������F�ެ���
				break;
			}												// ��L�ȊO���͉���case���Ń��V�[�g�̉ۂ𔻒f
		case 2:												// �󎚐悪���V�[�g
			if(Ope_isPrinterReady() == 1)					// ���V�[�g�v�����^���󎚉\�ȏ��
			{
				prn_kind	= R_PRI;						// �Ώ�������Fڼ��
			}
			else											// �󎚕s��(���؂�)
			{
				prn_kind	= 0;							// �Ώ�������F�Ȃ�
			}
			break;
		case 1:												// �󎚐悪�W���[�i��
			if(Ope_isJPrinterReady() == 1)					// �W���[�i���v�����^���󎚉\�ȏ��
			{
				prn_kind	= J_PRI;						// �Ώ�������F�ެ���
			}
			else
			{
				prn_kind	= 0;							// �Ώ�������F�Ȃ�
			}
			break;
		case 0:												// �󎚐�ݒ�Ȃ�
		default:											// �K�i�O�̐ݒ�(�ُ�)
			prn_kind	= 0;								// �Ώ�������F�Ȃ�
			break;
		}
		break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	case LOG_ECSYUUKEI:										// ���σ��[�_�W�v
		switch(prm_get(COM_PRM, S_ECR, 4, 1, 1))			// 50-004�E:�����W�v�󎚏o�͐�
		{
		case 3:												// �󎚐悪���V�[�g�ƃW���[�i��
			if((Ope_isJPrinterReady() == 1) && (Ope_isPrinterReady() == 1))
			{												// ���V�[�g�E�W���[�i�����󎚉\�ȏ��
				prn_kind	= RJ_PRI;						// �Ώ�������Fڼ��&�ެ���
				break;
			}
			if(Ope_isJPrinterReady() == 1)					// �W���[�i���v�����^���󎚉\�ȏ��
			{
				prn_kind	= J_PRI;						// �Ώ�������F�ެ���
				break;
			}												// ��L�ȊO���͉���case���Ń��V�[�g�̉ۂ𔻒f
		case 2:												// �󎚐悪���V�[�g
			if(Ope_isPrinterReady() == 1)					// ���V�[�g�v�����^���󎚉\�ȏ��
			{
				prn_kind	= R_PRI;						// �Ώ�������Fڼ��
			}
			else											// �󎚕s��(���؂�)
			{
				prn_kind	= 0;							// �Ώ�������F�Ȃ�
			}
			break;
		case 1:												// �󎚐悪�W���[�i��
			if(Ope_isJPrinterReady() == 1)					// �W���[�i���v�����^���󎚉\�ȏ��
			{
				prn_kind	= J_PRI;						// �Ώ�������F�ެ���
			}
			else
			{
				prn_kind	= 0;							// �Ώ�������F�Ȃ�
			}
			break;
		case 0:												// �󎚐�ݒ�Ȃ�
		default:											// �K�i�O�̐ݒ�(�ُ�)
			prn_kind	= 0;								// �Ώ�������F�Ȃ�
			break;
		}
		break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	default:
		if(Ope_isPrinterReady() == 1)						// ���V�[�g�v�����^���󎚉\�ȏ��
		{
			prn_kind	= R_PRI;							// �Ώ�������Fڼ��
		}
		else												// �󎚕s��(���؂�)
		{
			prn_kind	= 0;								// �Ώ�������F�Ȃ�
		}
		break;
	}

	return prn_kind;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O���t�ʒu�ԍ��X�V����("����"�ǉ�)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	pos		: �ʒu�ԍ��ް��߲��
///	@param[in]		req		: �����v��
///								�O�F�ʒu�ԍ��|�P�i���ړ��j
///								�P�F�ʒu�ԍ��{�P�i�E�ړ��j
///	@return			uchar	: �s�ړ�����(0:�Ȃ�/1:����)
///	@note			LogDatePosUp()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			ISE
///	@date			Create	: 20098/02/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	LogDatePosUp2( uchar *pos, uchar req )
{
	uchar ret = 0;					/* �s�ړ�����(0:�Ȃ�/1:����) */

	if( req == 0 ){

		// �ʒu�ԍ��|�P�i���ړ��j
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = _POS_MAX-1;
		}
		if((*pos == 3) || (*pos == (_POS_MAX-1)))
		{
			ret = 1;				/* �s�ړ����� */
		}
	}
	else{
		// �ʒu�ԍ��{�P�i�E�ړ��j
		*pos = (uchar)((*pos) + 1);
		if( *pos >= _POS_MAX ){
			*pos = 0;
		}
		if((*pos == 0) || (*pos == (_POS_MAX/2)))
		{
			ret = 1;				/* �s�ړ��Ȃ� */
		}
	}
	return ret;
}
#define		_POS_MAX_IO	10		// �J�[�\���ړ�MAX�l
//[]----------------------------------------------------------------------[]
///	@brief			���O���t�ʒu�ԍ��X�V����("��"�ǉ�)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	pos		: �ʒu�ԍ��ް��߲��
///	@param[in]		req		: �����v��
///								�O�F�ʒu�ԍ��|�P�i���ړ��j
///								�P�F�ʒu�ԍ��{�P�i�E�ړ��j
///	@return			uchar	: �s�ړ�����(0:�Ȃ�/1:����)
///	@note			LogDatePosUp2()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			ISE
///	@date			Create	: 2009/06/12<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	LogDatePosUp3( uchar *pos, uchar req )
{
	uchar ret = 0;					/* �s�ړ�����(0:�Ȃ�/1:����) */

	if( req == 0 ){

		// �ʒu�ԍ��|�P�i���ړ��j
		if( *pos != 0 ){
			*pos = (uchar)((*pos) - 1);
		}
		else{
			*pos = _POS_MAX_IO-1;
		}
		if((*pos == ((_POS_MAX_IO/2)-1)) || (*pos == (_POS_MAX_IO-1)))
		{
			ret = 1;				/* �s�ړ����� */
		}
	}
	else{
		// �ʒu�ԍ��{�P�i�E�ړ��j
		*pos = (uchar)((*pos) + 1);
		if( *pos >= _POS_MAX_IO ){
			*pos = 0;
		}
		if((*pos == 0) || (*pos == (_POS_MAX_IO/2)))
		{
			ret = 1;				/* �s�ړ��Ȃ� */
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ŌÁ��ŐV���O�f�[�^���t�f�[�^�擾����("����"�ǉ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		Old		: �Ō�۸��ް����t�ް��߲��
///	@param[in]		New		: �ŐV۸��ް����t�ް��߲��
///	@param[out]		Date	: ���t�ް��i�[�߲��
///	@return			void
///	@note			LogDateGet()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	LogDateGet2(date_time_rec *Old, date_time_rec *New, ushort *Date)
{
	// �Ō�۸��ް����t�ް�
	*Date = Old->Year;
	Date++;
	*Date = Old->Mon;
	Date++;
	*Date = Old->Day;
	Date++;
	*Date = Old->Hour;
	Date++;
	// �ŐV۸��ް����t�ް�
	*Date = New->Year;
	Date++;
	*Date = New->Mon;
	Date++;
	*Date = New->Day;
	Date++;
	*Date = New->Hour;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�󎚊J�n�^�I�����t�ް��\������
//[]----------------------------------------------------------------------[]
///	@param[in]		Date	: ���t�ް��߲���
///	@param[in]		Rev_Pos	: ���]�\���ʒu<br>
///								�O�`�T�F�w��ʒu���]�\��<br>
///								���̑��F���]�\���Ȃ�
///	@return			void
///	@note			LogDateDsp()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// �\�����ږ��̕\�����@�ް�ð��قQ�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA_TIME2[_POS_MAX][4] = {
	{ 4,  6, 2, 1 },	// �J�n�i�N�j
	{ 4, 12, 2, 0 },	// �J�n�i���j
	{ 4, 18, 2, 0 },	// �J�n�i���j
	{ 5,  6, 2, 1 },	// �I���i�N�j
	{ 5, 12, 2, 0 }, 	// �I���i���j
	{ 5, 18, 2, 0 },	// �I���i���j
};

void	LogDateDsp2( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// ���t�ް��\���ʒu
	ushort	rev_data;		// ���]�\���ް�


	for( pos = 0 ; pos < _POS_MAX ; pos++){		// ���t�ް��\���F�J�n�i�N�j�`�I���i���j
		opedsp	(
					POS_DATA_TIME2[pos][0],		// �\���s
					POS_DATA_TIME2[pos][1],		// �\�����
					*Date,						// �\���ް�
					POS_DATA_TIME2[pos][2],		// �ް�����
					POS_DATA_TIME2[pos][3],		// �O�T�v���X�L��
					0,							// ���]�\���F�Ȃ�
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;				// ���]�\���ް�����
		}

		Date++;									// ���t�ް��߲���X�V
	}
	if( Rev_Pos <= _POS_MAX-1 ){				// ���]�\���w�肠��H

		opedsp	(								// �w��ʒu�𔽓]�\��������
					POS_DATA_TIME1[Rev_Pos][0],	// �\���s
					POS_DATA_TIME1[Rev_Pos][1],	// �\�����
					rev_data,					// �\���ް�
					POS_DATA_TIME1[Rev_Pos][2],	// �ް�����
					POS_DATA_TIME1[Rev_Pos][3],	// �O�T�v���X�L��
					1,							// ���]�\���F����
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���O�󎚊J�n�^�I�����t�ް��\�������R�i���o�ɏ��v�����g�p�j		|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDateDsp3( *Date, pos )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	*Date	:	���t�ް��߲���					|*/
/*|																			|*/
/*|						uchar	Rev_Pos	:	���]�\���ʒu					|*/
/*|											�O�`�T�F�w��ʒu���]�\��		|*/
/*|											���̑��F���]�\���Ȃ�			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
// �\�����ږ��̕\�����@�ް�ð��قR�i�\���s�A�\����сA�\�������A�O�T�v���X�L���j
const	ushort	POS_DATA31[10][4] = {
	{ 3, 12, 2, 1 },	// �J�n�i�N�j
	{ 3, 18, 2, 0 },	// �J�n�i���j
	{ 3, 24, 2, 0 },	// �J�n�i���j
	{ 4, 20, 2, 0 },	// �J�n�i���j
	{ 4, 26, 2, 1 },	// �J�n�i���j
	{ 5, 12, 2, 1 },	// �I���i�N�j
	{ 5, 18, 2, 0 }, 	// �I���i���j
	{ 5, 24, 2, 0 },	// �I���i���j
	{ 6, 20, 2, 0 },	// �I���i���j
	{ 6, 26, 2, 1 }		// �I���i���j
};
const	ushort	POS_DATA32[10][4] = {
	{ 3,  8, 4, 1 },	// �J�n�i�N�j
	{ 3, 18, 2, 0 },	// �J�n�i���j
	{ 3, 24, 2, 0 },	// �J�n�i���j
	{ 4, 20, 2, 0 },	// �J�n�i���j
	{ 4, 26, 2, 1 },	// �J�n�i���j
	{ 5,  8, 4, 1 },	// �I���i�N�j
	{ 5, 18, 2, 0 }, 	// �I���i���j
	{ 5, 24, 2, 0 },	// �I���i���j
	{ 6, 20, 2, 0 },	// �I���i���j
	{ 6, 26, 2, 1 }		// �I���i���j
};

void	LogDateDsp3( ushort *Date, uchar Rev_Pos )
{
	uchar	pos;			// ���t�ް��\���ʒu
	ushort	rev_data;		// ���]�\���ް�


	for( pos = 0 ; pos < 10 ; pos++){		// ���t�ް��\���F�J�n�i�N�j�`�I���i���j

		opedsp	(
					POS_DATA32[pos][0],		// �\���s
					POS_DATA32[pos][1],		// �\�����
					*Date,					// �\���ް�
					POS_DATA32[pos][2],		// �ް�����
					POS_DATA32[pos][3],		// �O�T�v���X�L��
					0,						// ���]�\���F�Ȃ�
					COLOR_BLACK,
					LCD_BLINK_OFF
				);

		if( pos == Rev_Pos ){
			rev_data = *Date%100;			// ���]�\���ް�����
		}

		Date++;								// ���t�ް��߲���X�V
	}
	if( Rev_Pos <= 9 ){						// ���]�\���w�肠��H

		opedsp	(							// �w��ʒu�𔽓]�\��������
					POS_DATA31[Rev_Pos][0],	// �\���s
					POS_DATA31[Rev_Pos][1],	// �\�����
					rev_data,				// �\���ް�
					POS_DATA31[Rev_Pos][2],	// �ް�����
					POS_DATA31[Rev_Pos][3],	// �O�T�v���X�L��
					1,						// ���]�\���F����
					COLOR_BLACK,
					LCD_BLINK_OFF
				);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���O���t�f�[�^�`�F�b�N����("����"�ǉ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: ���t�ʒu�ԍ��ް�
///	@param[in]		data	: ���t�ް�
///	@return			ret		: ��������(OK/NG)
///	@note			LogDateChk()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/07/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
// ���ږ����ް��͈������ް�ð��فi�ŏ��l�A�ő�l�j
const	short	LOG_DATE_DATA2[_POS_MAX][2] = {
	{ 0, 99 },	// �J�n�i�N�j
	{ 1, 12 },	// �J�n�i���j
	{ 1, 31 },	// �J�n�i���j
	{ 0, 99 },	// �I���i�N�j
	{ 1, 12 },	// �I���i���j
	{ 1, 31 },	// �I���i���j
};
uchar	LogDateChk2( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA2[pos][0] && data <= LOG_DATE_DATA2[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			���O���t�f�[�^�`�F�b�N����("��"�ǉ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: ���t�ʒu�ԍ��ް�
///	@param[in]		data	: ���t�ް�
///	@return			ret		: ��������(OK/NG)
///	@note			LogDateChk()���Q��<br>
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2009/10/29<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
// ���ږ����ް��͈������ް�ð��فi�ŏ��l�A�ő�l�j
const	short	LOG_DATE_DATA3[_POS_MAX_IO][2] = {
	{ 0, 99 },	// �J�n�i�N�j
	{ 1, 12 },	// �J�n�i���j
	{ 1, 31 },	// �J�n�i���j
	{ 0, 23 },	// �J�n�i���j
	{ 0, 59 },	// �J�n�i���j
	{ 0, 99 },	// �I���i�N�j
	{ 1, 12 },	// �I���i���j
	{ 1, 31 },	// �I���i���j
	{ 0, 23 },	// �I���i���j
	{ 0, 59 },	// �I���i���j
};
uchar	LogDateChk_Range_Minute( uchar pos, short data )
{
	uchar	ret;

	if( data >= LOG_DATE_DATA3[pos][0] && data <= LOG_DATE_DATA3[pos][1] ){
		ret = OK;
	}
	else{
		ret = NG;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*
 *| transfer file (serial interface & FTP connection)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : SysMnt_FileTransfer
 *| PARAMETER    : none
 *| RETURN VALUE : event type
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort	SysMnt_FileTransfer(void)
{
	ushort	usSysEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for ( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[4] );		/* "���ڑ������I�����@�@�@�@�@�@�@" */
		Fun_Dsp(FUNMSG[25]);					/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
		usSysEvent = Menu_Slt(FLTRFMENU, FILE_TRF_TBL, (char)FL_TRF_MAX, (char)1);

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usSysEvent) {
		/* 2.FTP�ڑ�(�N���C�A���g) */
		case FILE_FTP_CLI:
			BUZPIPI();
			break;
		/* 3.FTP�ڑ�(�T�[�o�[) */
		case FILE_FTP_SEV:
			Ope_DisableDoorKnobChime();
			usSysEvent = SysMnt_FTPServer();
			break;
		case MOD_EXT:
			return MOD_EXT;
		default:
			break;
		}
		if (usSysEvent == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (usSysEvent == MOD_CUT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

// �\�����ږ���F3�L�[���͖���/�L������e�[�u��
static const uchar RT_KEY_TEN_F3_valid_tbl[RT_MODE_MAX][RT_SUB_MODE_MAX][18] = {
		// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17		<=�\������No.
// MH810100(S) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)			
//		{{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 4, 4, 4, 4, 4},	// mode=0,	sub_mode=0
		{{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 4, 4, 4, 4, 4},	// mode=0,	sub_mode=0
// MH810100(E) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)	
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},	// 			sub_mode=3
		{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// mode=1,	sub_mode=0
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}	// 			sub_mode=3
	};																// 0 = ����
																	// 1 = �L��	��ʐ؊�(�����ʃp�����[�^�ݒ���)
																	// 2 = �L��	�\���؊�(�������)
																	// 3 = �L��	��ʐ؊�(������������͉��)
																	// 4 = �L��	��ʐ؊�(���|��������͉��)
																	// 5 = �L��	�\���؊�(�T�[�r�X�����)

// �\�����ږ��̃e���L�[���͖���/�L������e�[�u��
static const uchar RT_KEY_TEN_valid_tbl[RT_MODE_MAX][RT_SUB_MODE_MAX][18] = {
		// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17		<=�\������No.
// MH810100(S) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//		{{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 0, 3, 1, 3, 1, 3, 1},	// mode=0,	sub_mode=0
		{{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 3, 1, 3, 1, 3, 1},	// mode=0,	sub_mode=0
// MH810100(E) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)			
		 { 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 3, 1, 3, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},	// 			sub_mode=3
		{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// mode=1,	sub_mode=0
		 { 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=1
		 { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 			sub_mode=2
		 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}	// 			sub_mode=3
	};																// 0 = ����
																	// 1 = �L��	1���\������
																	// 2 = �L��	2���\������
																	// 3 = �L��	3���\������
//[]----------------------------------------------------------------------[]
///	@brief			�V�X�e�������e�i���X�F�����v�Z�e�X�g
//[]----------------------------------------------------------------------[]
///	@return			MOD_CHG	: mode change<br>
///					MOD_EXT	: F5 key
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/30<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
unsigned short	SysMnt_RyoTest( void )
{
	ushort	msg;
	long	input;				// ���̓f�[�^
	short	result;
	uchar	ret;
	T_FrmReceipt	rec_data;	// �̎��؈󎚗v����ү�����ް�/

// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
//	f_sousai = 0;
//// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
	memset(&rt_ctrl_bk1, 0, sizeof(t_RT_Ctrl));
	memset(&rt_ctrl_bk2, 0, sizeof(t_RT_Ctrl));
	if( rt_ctrl.param.set == ON ){							// ���ʃp�����[�^�ݒ肩��̖߂�
		rt_ctrl.param.set = OFF;
	}else{													// �ʏ�N��or���ʃp�����[�^�ݒ肩��̋N��
		RT_init_disp_item();								// �\�����ڂ̏�����
		if( rt_ctrl.param.no == 0 ){						// ���ʃp�����[�^�ݒ肩��̋N���ł͂Ȃ�
			rt_ctrl.nrml_strt = ON;							// �ʏ�N���t���OON
		}
	}

// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
// 	RT_base_disp(&input);									// ��{��ʕ\��
	if ( OPECTL.RT_show_attention == 1 ) {
		// ���Ӊ�ʕ\��
		RT_attention_disp();
	} else {
		// ��{��ʕ\��
		RT_base_disp(&input);
	}
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		if( msg == KEY_MODECHG ){
			BUZPI();
			memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));			// �����v�Z�e�X�g����f�[�^������
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if( msg == LCD_DISCONNECT ){
			BUZPI();
			memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));			// �����v�Z�e�X�g����f�[�^������
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
		if ( OPECTL.RT_show_attention == 1 ) {
			switch( KEY_TEN0to9(msg) ) {
			case KEY_TEN_F1:		// �߂�
				BUZPI();

				if ( rt_ctrl.nrml_strt == ON ) {			// �ʏ�I��
					// �����v�Z�e�X�g����f�[�^������
					memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
					return MOD_EXT;
				} else {									// ���ʃp�����[�^�ݒ�ւ̖߂�
					// �����v�Z�e�X�g����f�[�^������(param.no������)
					memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
					return MNT_PRSET;
				}

				break;
			case KEY_TEN_F5:		// ����
				BUZPI();

				// ��{��ʕ\��
				RT_base_disp(&input);

				// ���̃��O�C���܂Œ��Ӊ�ʂ�\�����Ȃ�
				OPECTL.RT_show_attention = 0;

				break;
			default:
				break;
			}

			continue;
		}
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�

		// �����v�Z�O��ʕ\����
		if( rt_ctrl.mode == 0 ){
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// �I��(F5)
				BUZPI();
				if( rt_ctrl.sub_mode == 0 ){				// �e��ʁF�����v�Z�e�X�g�I��
					if( rt_ctrl.nrml_strt == ON ){			// �ʏ�I��
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
															// �����v�Z�e�X�g����f�[�^������
						return MOD_EXT;
					}else{									// ���ʃp�����[�^�ݒ�ւ̖߂�
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
															// �����v�Z�e�X�g����f�[�^������(param.no������)
						return MNT_PRSET;
					}
				}else{										// �q��ʁF��{��ʂɖ߂�
					memcpy(&rt_ctrl, &rt_ctrl_bk1, sizeof(t_RT_Ctrl));
					RT_base_disp(&input);					// ��{��ʕ\��
				}
				break;
			case KEY_TEN_F1:		// ��(F1)
			case KEY_TEN_F2:		// ��(F2)
				if (input != -1) {
					// �L���͈̓`�F�b�N
					if( RT_check_disp_data(input) ) {
						// �`�F�b�N�m�f(�\���l�߂��͊֐��̒��ōs��)
						BUZPIPI();
						input = -1;
						break;
					}
				}
				// �J�[�\���ړ��O�̕\�����ڂ𐳓]
				RT_get_disp_data(&input);					// �\���l�擾
				// �\�����ڂ̍X�V
				RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 0);
				// �J�[�\���ړ���̕\�����ڂ𔽓]���V�s�ڂ̕\��
				RT_move_cursor(msg, &input);
				break;
			case KEY_TEN_F3:		// �؊��E�p�����[�^(F3)
// MH810100(S) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( (rt_ctrl.param.set != ON) && (rt_ctrl.param.no == 0) ){
// MH810100(E) Y.Yamauchi 2019121 �Ԕԃ`�P�b�g���X(�����e�i���X)
					ret =  RT_check_KEY_TEN_F3(&input);
					if( ret == 1 ){
						return MNT_PRSET ;
					}
// MH810100(S) Y.Yamauchi 2019/12/14 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				}
// MH810100(E) Y.Yamauchi 2019/12/14 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
			case KEY_TEN_F4:		// �����E���s(F4)
				if (input != -1){
					// �L���͈̓`�F�b�N
					if( RT_check_disp_data(input) ) {
						// �`�F�b�N�m�f(�\���l�߂��͊֐��̒��ōs��)
						BUZPIPI();
						input = -1;
						break;
					}
				}
				RT_bkp_rst_data(0);							// �f�[�^�o�b�N�A�b�v
				result = RT_ryo_cal();
				if( !result ){								// �����v�ZOK
					BUZPI();
					rt_ctrl.mode = 1;						// �����v�Z���ʉ��
					rt_ctrl.sub_mode = 0;					// �e��ʁF�����v�Z�e�X�g���ʕ\����
					rt_ctrl.pos = 0;
					input = -1;
				}else{
					BUZPIPI();
				}
				break;
			case KEY_TEN:			// ����(�e���L�[)
				if( input == -1 ){							// ���̍��ڂł̍ŏ��̓���
					input = 0;
				}
				if( RT_check_KEY_TEN(&input, msg) ){		// �e���L�[�`�F�b�N�����͒l�X�V
					break;
				}
				BUZPI();
				// �\�����ڂ̍X�V
				RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
				break;
			case KEY_TEN_CL:		// ���(�e���L�[)
				if( RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos] ){
					BUZPI();
					RT_get_disp_data(&input);				// �\���l�擾
					RT_change_disp(0, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
					if(( rt_ctrl.sub_mode == 0 ) &&
// MH810100(S) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						( rt_ctrl.pos == RT_FLP_NO )){		// �t���b�v��
					   ( rt_ctrl.pos == RT_KIND )){			// �������
// MH810100(E) Y.Yamauchi 2019/12/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
							input = base_data[rt_ctrl.pos];	// �\���l��ݒ�
					}else{
						input = -1;
					}
				}
				break;
			default:
				break;
			}
		// �����v�Z���ʉ�ʕ\����
		}else if( rt_ctrl.mode == 1 ){
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// �I��(F5)
				BUZPI();
				if( rt_ctrl.sub_mode == 0 ){				// �e��ʁF�����v�Z�e�X�g���ʕ\����
					if( rt_ctrl.param.no == 0 ){			// �ʏ�I��
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));
															// �����v�Z�e�X�g����f�[�^������
					}else{									// ���ʃp�����[�^�ݒ�ւ̖߂�
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));
															// �����v�Z�e�X�g����f�[�^������(param.no������)
					}
					RT_bkp_rst_data(1);						// �f�[�^���X�g�A
					RT_base_disp(&input);					// ��{��ʕ\��
				}else{										// �q��ʁF�e��ʂɖ߂�
					rt_ctrl.sub_mode = 0;					// �e��ʁF�����v�Z�e�X�g���ʕ\����
					rt_ctrl.pos = 0;
					RT_recover_disp();
					input = -1;
				}
				break;
			case KEY_TEN_F1:		// ������E��(F1)
			case KEY_TEN_F2:		// �T�[�r�X���E��(F2)
				if( rt_ctrl.sub_mode == 0 ){				// �e��ʁF�����v�Z�e�X�g���ʕ\����
					if( ryo_buf.dsp_ryo ){					// �\��������0�~
						RT_check_disp_change(&input, msg);
					}
				}else if( rt_ctrl.sub_mode == 1 ){			// ����������͉��
					if (input != -1) {
						// �L���͈̓`�F�b�N
						if( RT_check_disp_data(input) ) {
							// �`�F�b�N�m�f(�\���l�߂��͊֐��̒��ōs��)
							BUZPIPI();
							input = -1;
							break;
						}
					}
					// �J�[�\���ړ��O�̕\�����ڂ𐳓]
					RT_get_disp_data(&input);				// �\���l�擾
					// �\�����ڂ̍X�V
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 0);
					// �J�[�\���ړ���̕\�����ڂ𔽓]���V�s�ڂ̕\��
					RT_move_cursor(msg, &input);
				}
				break;
			case KEY_TEN_F3:		// �|�����E�؊�(F3)
				if( rt_ctrl.sub_mode == 0 ){				// �e��ʁF�����v�Z�e�X�g���ʕ\����
					if( ryo_buf.dsp_ryo ){					// �\��������0�~
						RT_check_disp_change(&input, msg);
					}
				}else{										// �q���
					RT_check_KEY_TEN_F3(&input);
				}
				break;
			case KEY_TEN_F4:		// ����
				if( rt_ctrl.sub_mode != 0 ){				// �q���
					if (input != -1) {
						// �L���͈̓`�F�b�N
						if( RT_check_disp_data(input) ){
							// �`�F�b�N�m�f(�\���l�߂��͊֐��̒��ōs��)
							BUZPIPI();
							input = -1;
							break;
						}
					}
					result = RT_ryo_cal();
					if( !result ){							// �����v�ZOK
						BUZPI();
						rt_ctrl.mode = 1;					// �����v�Z���ʉ��
						rt_ctrl.sub_mode = 0;				// �e��ʁF�����v�Z�e�X�g���ʕ\����
						rt_ctrl.pos = 0;
						input = -1;
					}else{
						BUZPIPI();
					}
				}else{
					RT_edit_log_data(&rec_data);			// ���O�o�^�p�f�[�^�ҏW
					RT_log_regist();
					BUZPI();
					// �o�^��͊�{��ʂ�
					if( rt_ctrl.param.no == 0 ){			// �ʏ�I��
						memset(&rt_ctrl, 0, sizeof(t_RT_Ctrl));// �����v�Z�e�X�g����f�[�^������
					}else{									// ���ʃp�����[�^�ݒ�ւ̖߂�
						memset(&rt_ctrl, 0, (sizeof(t_RT_Ctrl) - sizeof(rt_ctrl.param.no)));// �����v�Z�e�X�g����f�[�^������(param.no������)
					}
					RT_bkp_rst_data(1);						// �f�[�^���X�g�A
					RT_base_disp(&input);					// ��{��ʕ\��
				}
				break;
			case KEY_TEN:			// ����(�e���L�[)
				if( rt_ctrl.sub_mode != 0 ){				// �q���
					if( input == -1 ){						// ���̍��ڂł̍ŏ��̓���
						input = 0;
					}
					if( RT_check_KEY_TEN(&input, msg) ){	// �e���L�[�`�F�b�N�����͒l�X�V
						break;
					}
					BUZPI();
					// �\�����ڂ̍X�V
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
				}
				break;
			case KEY_TEN_CL:		// ���(�e���L�[)
				if( RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos] ){
					BUZPI();
					RT_get_disp_data(&input);				// �\���l�擾
					RT_change_disp(1, rt_ctrl.sub_mode, rt_ctrl.pos, input, 1);
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					if( rt_ctrl.sub_mode == 3 ){			// �T�[�r�X����ʓ��͉��
//						input = serv_data[RT_SERV_KIND];	// �\���l��ݒ�
//					}else{
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
						input = -1;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					}
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				break;
			default:
				break;
			}
		// �G���[��ʕ\����
		}else{
			switch( KEY_TEN0to9(msg) ){
			case KEY_TEN_F5:		// �I��(F5)
				if( rt_ctrl_bk1.mode == 1 ){				// �����v�Z���ʉ��(�q���)�ŃG���[����
					BUZPI();
					rt_ctrl.mode = 1;						// �����v�Z���ʉ��
					rt_ctrl.sub_mode = 0;					// �e��ʁF�����v�Z�e�X�g���ʕ\����
					rt_ctrl.pos = 0;
					OPECTL.CR_ERR_DSP = 0;
					RT_recover_disp();						// ��ʕ��A
					input = -1;
				}
				break;
			case KEY_TEN_F4:		// �G���[�N���A(F4)
				BUZPI();
				if( rt_ctrl_bk1.mode == 0 ){				// ��{��ʂŃG���[����
					memcpy(&rt_ctrl, &rt_ctrl_bk1, sizeof(t_RT_Ctrl));
															// rt_ctrl���X�g�A
					RT_base_disp(&input);					// ��{��ʕ\��
				}else{										// �����v�Z���ʉ��(�q���)�ŃG���[����
					rt_ctrl.mode = 1;						// �����v�Z���ʂ̎q���(sub_mode�͈����p��)
					RT_clear_error();						// �G���[�N���A
					OPECTL.CR_ERR_DSP = 0;
					if( rt_ctrl.sub_mode != 3 ){			// �T�[�r�X����ʓ��͉�ʈȊO
						input = -1;
					}
				}
				break;
			default:
				break;
			}
		}
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �\�����ڂ̏�����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_init_disp_item( void )
{
	// ��{���(mode=0,sub_mode=0) �\�����ڏ�����
	base_data[RT_I_YEAR]    = base_data[RT_O_YEAR]   = (long)(CLK_REC.year%100);
															// ���ɔN=�o�ɔN=���ݔN
	base_data[RT_I_MONT]    = base_data[RT_O_MONT]   = (long)CLK_REC.mont;
															// ���Ɍ�=�o�Ɍ�=���݌�
	base_data[RT_I_DAY]     = base_data[RT_O_DAY]    = (long)CLK_REC.date;
															// ���ɓ�=�o�ɓ�=���ݓ�
	base_data[RT_I_HOUR]    = base_data[RT_I_MIN]    = 0;	// ���Ɏ���=0
	base_data[RT_O_HOUR]    = CLK_REC.hour;					// �o�Ɏ�=���ݎ�
	base_data[RT_O_MIN]     = CLK_REC.minu;					// �o�ɕ�=���ݕ�
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	base_data[RT_FLP_NO]    = 1;							// �t���b�v��=1
	base_data[RT_KIND]      = 0;							// �������=A��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)

	// �v�Z���ʉ��(mode=1,sub_mode=0) �\�����ڏ�����
	rslt_data[RT_RYOUKIN]   = 0; 							// �v�Z���ʉ�ʃf�[�^
	rslt_data[RT_U_CARD]    = 0; 							// �v�Z���ʉ�ʃf�[�^

	// ����������͉��(mode=0/1,sub_mode=1) �\�����ڏ�����
	base_data[RT_PASS] = 0;									// ������=0
	pass_data[RT_S_YEAR]    = pass_data[RT_E_YEAR]   = (long)(CLK_REC.year%100);
															// �L���J�n�N=�L���I���N=���ݔN
	pass_data[RT_S_MONT]    = pass_data[RT_E_MONT]   = (long)CLK_REC.mont;
															// �L���J�n��=�L���I����=���݌�
	pass_data[RT_S_DAY]     = pass_data[RT_E_DAY]    = (long)CLK_REC.date;
															// �L���J�n��=�L���I����=���ݓ�
	// �|������(�|�����p)���͉��(mode=1,sub_mode=2) �\�����ڏ�����
	kake_data[RT_KAKE_NO]   = 0;							// �XNo.=0

	// �T�[�r�X����ʓ��͉��(mode=1,sub_mode=3) �\�����ڏ�����
	serv_data[RT_SERV_KIND] = 0;							// �T�[�r�X�����=A��

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ��{��ʕ\��
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: �\���f�[�^�|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_base_disp( long *p_in )
{
	uchar	base_max;												// ��{��ʕ\�����ڍő吔
	char	disp_pos;												// �\���ʒu
	long	disp_data;												// �\���f�[�^
	ushort	rev;													// ���]

	// clear screen
	dispclr();
	// ��{��ʐ���
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);		// "�������v�Z�e�X�g���@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[2]);		// "�e���ڂ���͂��ĉ������@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[3]);		// "���Ɏ����F  �N  ��  ��  �F  �@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[4]);		// "�o�Ɏ����F  �N  ��  ��  �F  �@"
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[5]);		// "������ʁF�@��@�����ʁF�@�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[13]);	// "������ʁF�@��@�@�@�@�@�@�@�@"
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	base_max = RT_BASE_MAX - 1;										// ���ڐ�
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[7]);		// "�@���s�F�����v�Z�����s���܂��@"

	for (disp_pos = 0; disp_pos < base_max; disp_pos++){
		disp_data = base_data[disp_pos];
		if( disp_pos == rt_ctrl.pos ){								// �J�[�\���ʒu�f�[�^
			rev = 1;												// ���]
		}else{
			rev = 0;												// ���]
		}
		RT_change_disp(0, 0, disp_pos, disp_data, rev);				// �\�����ڂ̍X�V
	}
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	 if( rt_ctrl.param.no != 0){										// �V�X�e���p�����[�^����̗v��
//	 	Fun_Dsp( FUNMSG2[46] );										// "�@���@�@���@�@�@�@ ���s  �I�� "
	if( rt_ctrl.pos == RT_KIND ){
		Fun_Dsp( FUNMSG2[64] );										// "�@���@�@���@ �؊�  ���s  �I�� "
		*p_in = base_data[RT_KIND];
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}else{
// GG124100(S) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
// 		Fun_Dsp( FUNMSG2[52] );										// "�@���@�@���@���Ұ� ���s  �I�� "
		if ( rt_ctrl.nrml_strt == ON ) {							// �ʏ�N��
			Fun_Dsp( FUNMSG2[52] );									// "�@���@�@���@���Ұ� ���s  �I�� "
		} else {													// ���ʃp�����[�^�[�ݒ肩��̋N��
			Fun_Dsp( FUNMSG2[46] );									// "�@���@�@���@�@�@�@ ���s  �I�� "
		}
// GG124100(E) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
		*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)		
	}
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	return;
}

// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ���Ӊ�ʕ\��
//[]----------------------------------------------------------------------[]
///	@param[in/out]	void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			R.Endo
///	@date			Create	: 2021/09/29
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void	RT_attention_disp( void )
{
	// clear screen
	dispclr();

	// ���Ӊ�ʐ���
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);		// "�������v�Z�e�X�g���@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[21]);	// "�{�e�X�g�̐��������F�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[22]);	// "�E�Ԏ�؊����Ƃ��Ȃ������́@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[23]);	// "�@�ŏ��ɓK�p���Ă��������B�@�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[24]);	// "�E���X�܊����͔�Ή��ł��B�@�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[25]);	// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[26]);	// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
	Fun_Dsp(FUNMSG[114]);											// " �߂�                    ���� "

	return;
}
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�

// ��{��ʍ���(mode=0,sub_mode=0)
static const t_RT_DispItem disp_item_base[] = {
	{ 2, 10, 2, 0, 0,  99,   0},		// ���ɔN
	{ 2, 14, 2, 0, 1,  12,   0},		// ���Ɍ�
	{ 2, 18, 2, 0, 1,  31,   0},		// ���ɓ�
	{ 2, 22, 2, 0, 0,  23,   0},		// ���Ɏ�
	{ 2, 26, 2, 0, 0,  59,   0},		// ���ɕ�
	{ 3, 10, 2, 0, 0,  99,   0},		// �o�ɔN
	{ 3, 14, 2, 0, 1,  12,   0},		// �o�Ɍ�
	{ 3, 18, 2, 0, 1,  31,   0},		// �o�ɓ�
	{ 3, 22, 2, 0, 0,  23,   0},		// �o�Ɏ�
	{ 3, 26, 2, 0, 0,  59,   0},		// �o�ɕ�
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	{ 4, 10, 4, 0, 1,9900,   0},		// �Ԏ���
	{ 4, 10, 1, 1, 0,  11,   0},		// �������
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	{ 4, 26, 2, 1, 0,  15,   0},		// ������(0:�Ȃ�/01�`15)
	{ 5,  4, 3, 0, 0, 100, 255},		// �X1-No.		(�ő�l�F1�X��=100/���X��=255)
};

// ����������͉�ʍ���(mode=0/1,sub_mode=1)
static const t_RT_DispItem disp_item_pass[] = {
	{ 2, 10, 2, 1, 0,  15,   0},		// ������(0:�Ȃ�/01�`15)
	{ 3, 10, 2, 0, 0,  99,   0},		// �L���J�n�N
	{ 3, 14, 2, 0, 1,  12,   0},		// �L���J�n��
	{ 3, 18, 2, 0, 1,  31,   0},		// �L���J�n��
	{ 4, 10, 2, 0, 0,  99,   0},		// �L���I���N
	{ 4, 14, 2, 0, 1,  12,   0},		// �L���I����
	{ 4, 18, 2, 0, 1,  31,   0},		// �L���I����
};

// �|������(�|�����p)���͉�ʍ���(mode=1,sub_mode=2)
static const t_RT_DispItem disp_item_kake[] = {
	{ 2,  5, 3, 0, 0, 100,  0},			// �X1-No.
};

// �T�[�r�X����ʓ��͉�ʍ���(mode=1,sub_mode=3)
static const t_RT_DispItem disp_item_serv[] = {
	{ 2,  6, 1, 1, 0,  14,   0},		// �T�[�r�X�����(0�`14:A�`O��)
};

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �\�����ڂ̍X�V
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: ��ʕҏW���[�h
///	@param[in]		sub_mode: �q��ʕҏW���[�h
///	@param[in]		pos		: �\�����ڔԍ�
///	@param[in]		prm		: �f�[�^���e
///	@param[in]		rev		: 0:���] 1:���] 
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/03/31<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_change_disp(char mode, char sub_mode, char pos, long data, ushort rev)
{
	ushort	line;
	ushort	lpos;
	ushort	keta;
	uchar	width;
	uchar	disp_flg = 1;

	if( !mode ){		// mode=0 ��{���
		switch( sub_mode ){
		case  0:		// ��{��ʕҏW��
			line  = disp_item_base[pos].line;
			lpos  = disp_item_base[pos].lpos;
			keta  = disp_item_base[pos].keta;
			width = disp_item_base[pos].width;
			break;
		default:
			disp_flg = 0;
			break;
		}
	}else{				// mode=1 �v�Z���ʉ��
		switch( sub_mode ){
		case  1:		// ����������͉�ʕҏW��
			line  = disp_item_pass[pos].line;
			lpos  = disp_item_pass[pos].lpos;
			keta  = disp_item_pass[pos].keta;
			width = disp_item_pass[pos].width;
			break;
		case  2:		// �|������(�|�����p)���͉�ʕҏW��
			line  = disp_item_kake[pos].line;
			lpos  = disp_item_kake[pos].lpos;
			keta  = disp_item_kake[pos].keta;
			width = disp_item_kake[pos].width;
			break;
		case  3:		// �T�[�r�X����ʓ��͉�ʕҏW��
			line  = disp_item_serv[pos].line;
			lpos  = disp_item_serv[pos].lpos;
			keta  = disp_item_serv[pos].keta;
			width = disp_item_serv[pos].width;
			break;
		default:
			disp_flg = 0;
			break;
		}
	}

	if( disp_flg ){
		if( width ){	// �S�p
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if(( mode == 1 ) && ( sub_mode == 3 ) && ( pos == RT_SERV_KIND )){
			if(( mode == 0 ) && ( sub_mode == 0 ) && ( pos == RT_KIND )){
				grachr(4, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data]);		// �������="�`"�`"�k"�\��
			}else if(( mode == 1 ) && ( sub_mode == 3 ) && ( pos == RT_SERV_KIND )){
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
				grachr(2, 6, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data]);			// �T�[�r�X�����="�`"�`"�b(�n)"�\��
			}else if(( mode == 0 ) && ( sub_mode == 0 ) && ( pos == RT_PASS ) && ( data == 0 )){
				grachr(4, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// ������="�Ȃ�"�\��
			}else if(( sub_mode == 1 ) && ( pos == RT_PASS_KIND ) && ( data == 0 )){
				grachr(2, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// ������="�Ȃ�"�\��
			}else{
				opedpl(line, lpos, (ulong)data, keta, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
			}
		}else{			// ���p
			opedpl3(line, lpos, (ulong)data, keta, 1, rev, COLOR_BLACK,  LCD_BLINK_OFF);
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �\���f�[�^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: ���Ұ��߲��
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2005/04/12<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
static uchar	RT_check_disp_data( long data )
{
	ushort	max_wk;
	long	data_wk = 0;
	uchar	ret = 1;
	// ���͒l���`�F�b�N���Ăn�j�Ȃ�f�[�^�i�[
	// �m�f�Ȃ�\����߂��āi���]�j���^�[��

	if( rt_ctrl.mode == 0 ){		// mode=0 ��{���
		switch( rt_ctrl.sub_mode ){
		case  0:		// ��{��ʕҏW��
			data_wk = base_data[rt_ctrl.pos];
			if(( rt_ctrl.pos == RT_I_DAY ) ||				// ���ɓ�
			  ( rt_ctrl.pos == RT_O_DAY )){					// �o�ɓ�
				max_wk = (uchar)RT_day_max();				// �Y�����̍ŏI���Z�o
			}else{
				max_wk = disp_item_base[rt_ctrl.pos].max1;
			}
			if(( disp_item_base[rt_ctrl.pos].min <= data ) &&
			   ( data <= max_wk )){							// �͈͓�
				base_data[rt_ctrl.pos] = data;				// ���͒l�i�[
				if( rt_ctrl.pos <= RT_I_DAY ){
					ret = RT_check_ymd(base_data[RT_I_YEAR], base_data[RT_I_MONT], base_data[RT_I_DAY]);
				}else if(( RT_O_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_O_DAY)){
					ret = RT_check_ymd(base_data[RT_O_YEAR], base_data[RT_O_MONT], base_data[RT_O_DAY]);
				}else{
					ret = 0;
				}
				if( ret ){
					base_data[rt_ctrl.pos] = data_wk;
				}
			}
			break;
		default:
			break;
		}
	}else{						// mode=1 �v�Z���ʉ��
		switch( rt_ctrl.sub_mode ){
		case  1:		// ����������͉�ʕҏW��
			data_wk = pass_data[rt_ctrl.pos];
			if(( rt_ctrl.pos == RT_S_DAY ) ||				// �L���J�n��
			   ( rt_ctrl.pos == RT_E_DAY )){ 				// �L���I����
				max_wk = (uchar)RT_day_max();				// �Y�����̍ŏI���Z�o
			}else{
				max_wk = disp_item_pass[rt_ctrl.pos].max1;
			}
			if(( disp_item_pass[rt_ctrl.pos].min <= data ) &&
			   ( data <= max_wk )){							// �͈͓�
				pass_data[rt_ctrl.pos] = data;				// ���͒l�i�[
				if(( RT_S_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_S_DAY)){
					ret = RT_check_ymd(pass_data[RT_S_YEAR], pass_data[RT_S_MONT], pass_data[RT_S_DAY]);
				}else if(( RT_E_YEAR <= rt_ctrl.pos ) && ( rt_ctrl.pos <= RT_E_DAY)){
					ret = RT_check_ymd(pass_data[RT_E_YEAR], pass_data[RT_E_MONT], pass_data[RT_E_DAY]);
				}else{
					ret = 0;
				}
				if( ret ){
					pass_data[rt_ctrl.pos] = data_wk;
				}
			}
			break;
		case  2:		// �|������(�|�����p)���͉�ʕҏW��
			data_wk = kake_data[rt_ctrl.pos];
			if(( disp_item_kake[rt_ctrl.pos].min <= data ) &&
			   ( data <= disp_item_kake[rt_ctrl.pos].max1 )){// �͈͓�
				kake_data[rt_ctrl.pos] = data;				// ���͒l�i�[
				ret = 0;
			}
			break;
		case  3:		// �T�[�r�X����ʓ��͉�ʕҏW��
			data_wk = serv_data[rt_ctrl.pos];
			if(( disp_item_serv[rt_ctrl.pos].min <= data ) &&
			   ( data <= disp_item_serv[rt_ctrl.pos].max1 )){// �͈͓�
				serv_data[rt_ctrl.pos] = data;				// ���͒l�i�[
				ret = 0;
			}
			break;
		default:
			break;
		}
	}

	if( ret ){		// �`�F�b�NNG
		RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, data_wk, 1);
	}														// ���͑O�̃f�[�^�𔽓]�\��

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �N�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		year	: �N
///	@param[in]		month	: ��
///	@param[in]		day		: ��
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_ymd( long year, long month, long day )
{
	uchar ret = 0;

	if( year < 80 ){
		year += 2000;
	}
	else{
		year += 1900;
	}

	if( DATE_CHK_ERR((short)year, (short)month, (short)day) ){
		ret = 1;
	}

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �\���f�[�^�擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: �\���f�[�^�|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_get_disp_data( long *p_in )
{

	if( rt_ctrl.mode == 0 ){								// mode=0 ��{���
		switch( rt_ctrl.sub_mode ){
		case  0:											// ��{��ʕҏW��
			*p_in = base_data[rt_ctrl.pos];
			break;
		default:
			break;
		}
	}else{													// mode=1 �v�Z���ʉ��
		switch( rt_ctrl.sub_mode ){
		case  1:											// ����������͉�ʕҏW��
			*p_in = pass_data[rt_ctrl.pos];
			break;
		case  2:											// �|������(�|�����p)���͉�ʕҏW��
			*p_in = kake_data[rt_ctrl.pos];
			break;
		case  3:											// �T�[�r�X����ʓ��͉�ʕҏW��
			*p_in = serv_data[rt_ctrl.pos];
			break;
		default:
			break;
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �J�[�\���ړ�
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: ���b�Z�[�W
///	@param[in/out]	p_in	: �\���f�[�^�|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/04<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_move_cursor( ushort msg, long *p_in )
{
	long	data;

	if( msg == KEY_TEN_F1 ){								// �O���ڂɈړ�
		if( rt_ctrl.mode == 0 ){							// mode=0 ��{���
			switch( rt_ctrl.sub_mode ){
			case  0:										// ��{��ʕҏW��
				BUZPI();
				--rt_ctrl.pos;
				if( rt_ctrl.pos < 0 ){
					rt_ctrl.pos = (char)( RT_BASE_MAX - 2);	// ������ʈʒu�Ƃ���
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				 }
//				 if( rt_ctrl.param.no != 0 ){
//				 	Fun_Dsp( FUNMSG2[46] );					// "�@���@�@���@�@�@�@ ���s  �I�� "
					Fun_Dsp( FUNMSG2[64] );					// "�@���@�@���@ �؊�  ���s  �I�� "
					*p_in = base_data[rt_ctrl.pos];			// �\���l��Ԃ�
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)					
				}else{
// GG124100(S) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
// 					Fun_Dsp( FUNMSG2[52] );					// "�@���@�@���@���Ұ� ���s  �I�� "
					if ( rt_ctrl.nrml_strt == ON ) {		// �ʏ�N��
						Fun_Dsp( FUNMSG2[52] );				// "�@���@�@���@���Ұ� ���s  �I�� "
					} else {								// ���ʃp�����[�^�[�ݒ肩��̋N��
						Fun_Dsp( FUNMSG2[46] );				// "�@���@�@���@�@�@�@ ���s  �I�� "
					}
// GG124100(E) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
				}
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				*p_in = -1;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810100(S) Y.Yamauchi #3909 �����v�Z�e�X�g?���������ۂ̏o�ɁE���Ɏ������Ԉ���ĕ\�������
				*p_in = -1;
// MH810100(E) Y.Yamauchi #3909 �����v�Z�e�X�g?���������ۂ̏o�ɁE���Ɏ������Ԉ���ĕ\�������
				break;
			default:
				break;
			}
		}else{												// mode=1 �v�Z���ʉ��
			switch( rt_ctrl.sub_mode ){
			case  1:										// ����������͉�ʕҏW��
				BUZPI();
				--rt_ctrl.pos;
				if( rt_ctrl.pos < 0 ){
					rt_ctrl.pos = (char)( RT_PASSINFO_MAX - 1);	// �L���I�����ʒu�Ƃ���
				}
				Fun_Dsp( FUNMSG2[2] );						// "�@���@�@���@�@�@�@ ����  �I�� "
				*p_in = -1;
				break;
			case  2:										// �|������(�|�����p)���͉�ʕҏW��
			case  3:										// �T�[�r�X����ʓ��͉�ʕҏW��
				// ���ڈړ����Ȃ��̂Œʂ�Ȃ�
			default:
				break;
			}
		}
	}else{													// �㍀�ڂɈړ�
		if( rt_ctrl.mode == 0 ){							// mode=0 ��{���
			switch( rt_ctrl.sub_mode ){
			case  0:										// ��{��ʕҏW��
				BUZPI();
				++rt_ctrl.pos;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( rt_ctrl.pos >= (RT_BASE_MAX - 1) ){		// ���X�܊����Ȃ����̈ʒu�ő�l�ȏ�
				if( rt_ctrl.pos >= RT_BASE_MAX - 1 ){			// �ʒu�ő�l�ȏ�
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
					rt_ctrl.pos = 0;						// �擪�ʒu�Ƃ���
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				 }
//				 if( rt_ctrl.param.no != 0){					// �V�X�e���p�����[�^����̗v��
//				 	Fun_Dsp( FUNMSG2[46] );					// "�@���@�@���@�@�@�@ ���s  �I�� "
//				 }else{
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)					
// GG124100(S) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
// 					Fun_Dsp( FUNMSG2[52] );					// "�@���@�@���@���Ұ� ���s  �I�� "
					if ( rt_ctrl.nrml_strt == ON ) {		// �ʏ�N��
						Fun_Dsp( FUNMSG2[52] );				// "�@���@�@���@���Ұ� ���s  �I�� "
					} else {								// ���ʃp�����[�^�[�ݒ肩��̋N��
						Fun_Dsp( FUNMSG2[46] );				// "�@���@�@���@�@�@�@ ���s  �I�� "
					}
// GG124100(E) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
				}


// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				*p_in = -1;
				if( rt_ctrl.pos == RT_KIND ){				// �������
					Fun_Dsp( FUNMSG2[64] );					// "�@���@�@���@ �؊�  ���s  �I�� "
					*p_in = base_data[rt_ctrl.pos];			// �\���l��Ԃ�
				}else{
					*p_in = -1;
				}
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
			default:
				break;
			}
		}else{												// mode=1 �v�Z���ʉ��
			switch( rt_ctrl.sub_mode ){
			case  1:										// ����������͉�ʕҏW��
				BUZPI();
				++rt_ctrl.pos;
				if( rt_ctrl.pos >= RT_PASSINFO_MAX ){		// ����������͈ʒu�ő�l�ȏ�
					rt_ctrl.pos = 0;						// �擪�ʒu�Ƃ���
				}
				Fun_Dsp( FUNMSG2[2] );						// "�@���@�@���@�@�@�@ ����  �I�� "
				*p_in = -1;
				break;
			case  2:										// �|������(�|�����p)���͉�ʕҏW��
			case  3:										// �T�[�r�X����ʓ��͉�ʕҏW��
				// ���ڈړ����Ȃ��̂Œʂ�Ȃ�
			default:
				break;
			}
		}
	}
	RT_get_disp_data(&data);								// �\���l�̎擾
	RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, data, 1);
															// ���݃J�[�\���ʒu���]

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g F3�L�[�`�F�b�N�����͒l�X�V
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: ���͒l�̃|�C���^
///	@return			kind	: 0 = ����	1 = �ݒ�	2�`5 = �؊�
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_KEY_TEN_F3( long *p_in )
{
	uchar	kind;

	kind = RT_KEY_TEN_F3_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos];
	switch( kind ){
	case  1:		// ���ʃp�����[�^�ݒ�ւ̑J��
// GG124100(S) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
		if ( rt_ctrl.nrml_strt == ON ) {					// �ʏ�N��
// GG124100(E) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
			BUZPI();
			rt_ctrl.param.set = ON;							// ���ʃp�����[�^�ݒ�v��ON
// GG124100(S) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
		} else {											// ���ʃp�����[�^�[�ݒ肩��̋N��
			kind = 0;
		}
// GG124100(E) R.Endo 2022/02/17 �Ԕԃ`�P�b�g���X3.0 #6138 �p�����[�^�ݒ��ʂ��痿���v�Z�e�X�g��ʑJ�ڎ��Ɂu�p�����[�^�v�{�^�����\������Ă��܂�
		break;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	case  2:		// �\���؊�(�������)
		BUZPI();
		if( ++(*p_in) > 11 ){								// input > L��
			*p_in = 0;										// A��ɖ߂�
		}
		RT_change_disp(rt_ctrl.mode, rt_ctrl.sub_mode, rt_ctrl.pos, *p_in, 1);
															// ���]�\��
		break;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	case  5:		// �\���؊�(�T�[�r�X�����)
		BUZPI();
		if( rt_ctrl.sub_mode != 3 ){
			memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));// rt_ctrl�o�b�N�A�b�v
			rt_ctrl.sub_mode = 3;							// �T�[�r�X����ʓ��͉��
			rt_ctrl.pos = 0;								// �T�[�r�X����ʈʒu
		}
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);					// "�������v�Z�e�X�g���@�@�@�@�@�@"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[14]);				// "�T�[�r�X����ʂ���͂��ĉ�����"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[15]);				// "��ʁF�@��@�@�@�@�@�@�@�@�@�@"
		if( ++(*p_in) > 14 )								// input > O��
		{
			*p_in = 0;										// A��ɖ߂�
		}
		grachr(2, 6, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[*p_in]);				// �T�[�r�X�����="�`"�`"�b(�n)"�\��
		Fun_Dsp( FUNMSG2[54] );								// "�@�@�@�@�@�@ �؊�  ����  �I�� "
	default:
		break;
	}

	return(kind);
}
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
///	@brief			���Ԍ� �����ݒ�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		m_gtticstp *mag	: ���Ԍ��f�[�^
///					uchar check_type: 0 
///					uchar f_CheckOnly: 0 <br>
///@return			ret		: 13 = ��ʋK��O�װ 0 = ����
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/12/01<br>
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]	
//static short RT_al_tikck2( m_gtticstp *mag, uchar check_type, uchar f_CheckOnly )
static short RT_al_tikck2( m_gtticstp *mag )
{
	short	ret = 0;
	/* ���Ԍ� �����ݒ�`�F�b�N */
	if( prm_get( COM_PRM,S_SHA,(short)(1+6*( base_data[RT_KIND] )),2,5 ) == 0L ){	// �Ԏ�ݒ�Ȃ��i�g�p����ݒ�łȂ��j
		ret = 13;											// ��ʋK��O�װ
	}
	return( ret );
}
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �����v�Z���s
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_ryo_cal( void )
{
	short	ret = 0;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	ushort	posi;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
	
	memset( MAGred, '\0', sizeof(MAGred) );
	// ���C�f�[�^����
	RT_make_mag_data(MAGred);
	if( rt_ctrl.mode == 0 ){		// ��{��ʂ���̗����v�Z���s
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrl�o�b�N�A�b�v
		RT_init_cal_data();									// �e�헿���v�Z�p�f�[�^������
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( LKopeGetLockNum( (uchar)0, (ushort)base_data[RT_FLP_NO], &posi ) ){
//			OPECTL.Op_LokNo = (ulong)( base_data[RT_FLP_NO] );		// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
//			OPECTL.Pr_LokNo = posi;									// ���������p���Ԉʒu�ԍ�(1�`324)
//			ret = (short)FlpSetChk(posi-1);
//		}
//
//		if( ret == 0 ){										// �`�F�b�NNG
//			grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, CARDERRSTR[17] );			// " ���� �ݒ肳��Ă��܂��� ���� "
//			Fun_Dsp( FUNMSG2[57] );							// "�@�@�@�@�@�@�@�@�@�װ�ر�@�@�@"
//			rt_ctrl.mode = 2;								// �G���[�\�����
//			rt_ctrl.sub_mode = 0;							// �e���
//			rt_ctrl.pos =0;
//			return(1);
//	}
//
//		ret = RT_set_tim( OPECTL.Pr_LokNo, &CLK_REC );
		ret = RT_al_tikck2( (m_gtticstp*)MAGred );		// ���Ԍ� �����ݒ�`�F�b�N
		if( ret != 0 ){				// �`�F�b�NNG
			// �G���[�\��
			CardErrDsp( ret );							//  �G���[�\��
			Fun_Dsp( FUNMSG2[57] );						// "�@�@�@�@�@�@�@�@�@�װ�ر�@�@�@"
			rt_ctrl.mode = 2;							// �G���[�\�����
			rt_ctrl.sub_mode = 0;						// �e���
			rt_ctrl.pos =0;
			return(1);
		}
		OPECTL.CalStartTrigger = (uchar)1;				// �����v�Z�J�n�v��set
		card_use[USE_TIK] += 1;

		ret = RT_set_tim((ushort)OPECTL.CalStartTrigger, &CLK_REC);
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if( ret ){
			// �G���[�\��
			grachr(1, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, SMSTR4[20]);				// �G���[("�@ ���o�Ɏ�������G���[�ł� �@")
			Fun_Dsp( FUNMSG2[57] );							// "�@�@�@�@�@�@�@�@�@�װ�ر�@�@�@"
			rt_ctrl.mode = 2;								// �G���[�\�����
			rt_ctrl.sub_mode = 0;							// �e���
			rt_ctrl.pos =0;
			return(1);
		}

		RT_ryo_cal2(0, OPECTL.Pr_LokNo);						// �����v�Z���s
		RT_disp_cal_result();								// �����v�Z���ʕ\��
	}else{						// �v�Z���ʉ�ʂ���̗����v�Z���s
		switch( rt_ctrl.sub_mode ){
		case  1:											// ����������͉��
			ret = al_pasck( (m_gtapspas *)MAGred );
			if( ret != 0 ){									// �`�F�b�NNG
				OPECTL.CR_ERR_DSP = ret;
				// �G���[�\��
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				CardErrDsp();
				CardErrDsp( ret );
// MH810100(E Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Fun_Dsp( FUNMSG2[55] );						// "�@�@�@�@�@�@�@�@�@�װ�ر �I�� "
				rt_ctrl.mode = 2;							// �G���[�\�����
				return(1);
			}
			memcpy( &RT_PayData.teiki, &InTeiki_PayData_Tmp, sizeof(teiki_use) );// ��������Z�b�g
			PayData.teiki.id = InTeiki_PayData_Tmp.id;							 // �𗝃`�F�b�N�Ŏg�p���邽��ID�Z�b�g
			card_use[USE_PAS] += 1;
			rt_ctrl.last_card.kind = RT_CRD_PASS;			// �ŏI�g�p�J�[�h���=�����
			rt_ctrl.last_card.info = (short)pass_data[RT_PASS_KIND];
															// �ŏI�g�p�J�[�h���=��������
			RT_ryo_cal2(1, OPECTL.Pr_LokNo);				// �����v�Z���s
			RT_disp_cal_result();							// �����v�Z���ʕ\��
			break;
		case  2:											// �|��������͉��
		case  3:											// �T�[�r�X����ʓ��͉��
			ret = al_svsck( (m_gtservic *)MAGred );
			if( ret != 0 ){									// �`�F�b�NNG
				// �G���[�\��
				OPECTL.CR_ERR_DSP = ret;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//				CardErrDsp();				
				CardErrDsp( ret );
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)					
				Fun_Dsp( FUNMSG2[55] );						// "�@�@�@�@�@�@�@�@�@�װ�ر �I�� "
				rt_ctrl.mode = 2;							// �G���[�\�����
				return(1);
			}
			if( rt_ctrl.sub_mode == 2 ){					// �|��������͉��
				rt_ctrl.last_card.kind = RT_CRD_KAKE;		// �ŏI�g�p�J�[�h���=�|����
				rt_ctrl.last_card.info = (short)kake_data[RT_KAKE_NO];
															// �ŏI�g�p�J�[�h���=�XNo.
			}else{
				rt_ctrl.last_card.kind = RT_CRD_SERV;		// �ŏI�g�p�J�[�h���=�T�[�r�X��
				rt_ctrl.last_card.info = (short)serv_data[RT_SERV_KIND];
															// �ŏI�g�p�J�[�h���=�T�[�r�X�����
			}
			RT_ryo_cal2(3, OPECTL.Pr_LokNo);				// �����v�Z���s
			RT_disp_cal_result();							// �����v�Z���ʕ\��
			memcpy( PayData.DiscountData, RT_PayData.DiscountData, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* �𗝃`�F�b�N�̂��߁A���Z�ް��������쐬�Z�b�g*/
			break;
		default:
			break;
		}
	}

	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �e���L�[�`�F�b�N�����͒l�X�V
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: ���͒l�̃|�C���^
///	@param[in]		msg		: �L�[���e
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/01<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_KEY_TEN( long *p_in, ushort msg )
{
	uchar	keta;

	keta = RT_KEY_TEN_valid_tbl[rt_ctrl.mode][rt_ctrl.sub_mode][rt_ctrl.pos];
	if( !keta ) {	// �e���L�[����
		return(1);
	}

	switch( keta ){
	case  1:		// 1��
		*p_in = (long)(msg - KEY_TEN0);
		break;
	case  2:		// 2��
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 100);
		break;
	case  3:		// 3��
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 1000);
		break;
	case  4:		// 4��
		*p_in = (long)(((*p_in * 10) + msg - KEY_TEN0) % 10000);
		break;
	default:
		return(1);
		break;
	}

	return(0);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �\���؊��`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_in	: ���͒l�̃|�C���^
///	@param[in]		msg		: �L�[���e
///	@return			0 = OK	1 = NG
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/13<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static uchar	RT_check_disp_change( long *p_in, ushort msg )
{
	char	disp_pos;										// �\���ʒu

	switch( msg ){
	case KEY_TEN_F1:		// ��ʐ؊�(������������͉��)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrl�o�b�N�A�b�v
		// rt_ctrl.mode�͈��p��
		rt_ctrl.sub_mode = 1;										// ����������͉��
		rt_ctrl.pos = 0;											// �������ʈʒu
		pass_data[RT_PASS_KIND] = base_data[RT_PASS];				// �����ʒl�R�s�[
		dispclr();													// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);	// "�������v�Z�e�X�g���@�@�@�@�@�@"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[8]);	// "�����������͂��ĉ������@�@"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[9]);	// "�����ʁF�@�@�@�@�@�@�@�@�@�@"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[10]);	// "�L���J�n�F  �N  ��  ���@�@�@�@"
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[11]);	// "�L���I���F  �N  ��  ���@�@�@�@"
		RT_change_disp(1, 1, 0, pass_data[0], 1);			// �������ʂ𔽓]�\��
		for (disp_pos = 1; disp_pos < RT_PASSINFO_MAX; disp_pos++) {
			RT_change_disp(1, 1, disp_pos, pass_data[disp_pos], 0);	// �e���ڕ\��
		}
		*p_in = -1;
		Fun_Dsp( FUNMSG2[2] );								// "�@���@�@���@�@�@�@ ����  �I�� "
		break;
	case KEY_TEN_F2:		// �\���؊�(�T�[�r�X�����)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrl�o�b�N�A�b�v
		rt_ctrl.sub_mode = 3;								// �T�[�r�X����ʓ��͉��
		rt_ctrl.pos = 0;									// �T�[�r�X����ʈʒu
		*p_in = serv_data[RT_SERV_KIND];					// �T�[�r�X����ʎ擾
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);					// "�������v�Z�e�X�g���@�@�@�@�@�@"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[14]);					// "�T�[�r�X����ʂ���͂��ĉ�����"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[15]);					// "��ʁF�@��@�@�@�@�@�@�@�@�@�@"
		if( *p_in > 14 )									// input > O��
		{
			*p_in = 0;										// A��ɖ߂�
		}
		grachr(2, 6, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[*p_in]);					// �T�[�r�X�����="�`"�`"�n"�\��
		Fun_Dsp( FUNMSG2[54] );								// "�@�@�@�@�@�@ �؊�  ����  �I�� "
		break;
	case KEY_TEN_F3:		// ��ʐ؊�(���|��������͉��)
		BUZPI();
		memcpy(&rt_ctrl_bk1, &rt_ctrl, sizeof(t_RT_Ctrl));	// rt_ctrl�o�b�N�A�b�v
		// rt_ctrl.mode�͈��p��
		rt_ctrl.sub_mode = 2;								// �|��������͉��
		rt_ctrl.pos = 0;									// �X1No.��ʈʒu
		dispclr();											// clear screen
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);						// "�������v�Z�e�X�g���@�@�@�@�@�@"
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[12]);					// "�|���������͂��ĉ������@�@"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[6]);						// "�X1�F   �@�@�@�@�@�@�@�@�@�@�@"
		RT_change_disp(1, 2, 0, kake_data[0], 1);			// �X1No.�𔽓]�\��
		Fun_Dsp( FUNMSG2[1] );								// "�@�@�@�@�@�@�@�@�@ ����  �I�� "
		*p_in = -1;
		break;
	default:
		break;
	}

	return(0);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ��ʕ��A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/13<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_recover_disp(void)
{
	uchar	base_max;										// ��{��ʕ\�����ڍő吔
	char	disp_pos;										// �\���ʒu
	long	disp_data;										// �\���f�[�^

	dispclr();												// clear screen
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[1]);							// "�������v�Z�e�X�g���@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR4[16]);				// "�v�Z���ʁF�@�@�@�@�@�@�@�@�@�~"
	opedpl(1, 16, (ulong)ryo_buf.dsp_ryo,6, 0, 0, COLOR_FIREBRICK, LCD_BLINK_OFF);		// �v�Z���ʕ\��
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[3]);							// "���Ɏ����F  �N  ��  ��  �F  �@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[4]);							// "�o�Ɏ����F  �N  ��  ��  �F  �@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[13]);						// 	"������ʁF�@��@�@�@�@�@�@�@�@", 
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//	base_max = RT_BASE_MAX;								// ���ڐ�
	base_max = RT_BASE_MAX - 1 ;						// ���ڐ�
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)		

	for (disp_pos = 0; disp_pos < base_max; disp_pos++) {
		disp_data = base_data[disp_pos];
		RT_change_disp(0, 0, disp_pos, disp_data, 0);
	}
	switch( rt_ctrl.last_card.kind ){
	case RT_CRD_PASS:										// �����
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[17]);					// "�g�p����F������@��ʁ@�@�@�@"
		opedpl(6, 22, (ulong)rt_ctrl.last_card.info,2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case RT_CRD_SERV:										// �T�[�r�X��
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR4[18]);					// "�g�p����F�T�[�r�X���@�@��@�@"
		grachr(6, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF,DAT1_4[rt_ctrl.last_card.info]);
		break;
	case RT_CRD_KAKE:										// �|����
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[19]);						// "�g�p����F�|�����X�@No�@�@�@�@"
		opedpl(6, 22, (ulong)rt_ctrl.last_card.info,3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	default			:
		break;
	}
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)			
//	Fun_Dsp( FUNMSG2[53] );									// "����� �T�� �|�����@�@�@ �I�� "
	if( !rt_ctrl.pos == RT_KIND ){							// �V�X�e���p�����[�^����̗v��
		Fun_Dsp( FUNMSG2[64] );								// "�@���@�@���@ �؊�  ���s  �I�� "
	}else{ 
		Fun_Dsp( FUNMSG2[53] );								// "����� �T�� �|�����@�@�@ �I�� "
	}
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)			
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �e�헿���v�Z�p�f�[�^������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/20<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_init_cal_data( void )
{
	// �����v�Z�p�f�[�^�������ŏ���������
	CRD_DAT.PAS.cod = 0;									// ������޸ر
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)			
	OPECTL.CalStartTrigger = 0;								// �����v�Z�J�n�v��set
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)				
	CardUseSyu = 0;											// 1���Z�̊�����ނ̌����ر
	PassIdBackup = 0;										// �����id�ޯ�����(n�������p)�ر
	PassIdBackupTim = 0;									// ���n����������
	PassPkNoBackup = 0;										// ��������ԏ�ԍ��ޯ�����(n�������p)�ر

	/** �ر�ر **/
	cm27();													// �׸޸ر
	OPECTL.ChkPassPkno = 0L;								// ������⍇�������ԏꇂ�ر
	OPECTL.ChkPassID = 0L;									// ������⍇���������ID�ر
	memset( card_use , 0 , sizeof( card_use ) ) ;			// 1���Z���ގg�p���
	memset( &CRD_DAT.PAS, 0, sizeof(pas_rcc) );
	memset( &CRD_DAT.SVS, 0, sizeof(svs_rec) );
	memset( &CRD_DAT.PRE, 0, sizeof(pre_rec) );
	memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ���C�f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: ���C�f�[�^�̃|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_mag_data( char *p_data )
{
	m_gtapspas	*p_magP;		// �����
	m_gtservic	*p_magS;		// �T�[�r�X��

	if( rt_ctrl.mode == 0 ){								// ��{���
		if( !base_data[RT_PASS] ){							// ���Ԍ�
		}else{												// �����
			p_magP = (m_gtapspas *)p_data;
			RT_make_pass(p_magP);
		}
	}else{													// �v�Z���ʉ��
		if( rt_ctrl.sub_mode == 1 ){						// �����
			p_magP = (m_gtapspas *)p_data;
			RT_make_pass(p_magP);
		}else{												// �|�����E�T�[�r�X��
			p_magS = (m_gtservic *)p_data;
			RT_make_service(p_magS);
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ��������C�f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_mag	: ���C�f�[�^�̃|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_pass( m_gtapspas *p_magP )
{
	p_magP->apspas.aps_idc = 0x1a;							// �����
	RT_get_valid_no((char *)p_magP);						// �L�������ID�擾
	p_magP->apspas.aps_pno[1] |= (uchar)(pass_data[RT_PASS_KIND] & 0x0f);
															// ��������
	p_magP->apspas.aps_sta[0] = (uchar)(pass_data[RT_S_YEAR] & 0x7f);
	p_magP->apspas.aps_sta[1] = (uchar)(pass_data[RT_S_MONT] & 0x7f);
	p_magP->apspas.aps_sta[2] = (uchar)(pass_data[RT_S_DAY]  & 0x7f);
															// �L���J�n�N����
	p_magP->apspas.aps_end[0] = (uchar)(pass_data[RT_E_YEAR] & 0x7f);
	p_magP->apspas.aps_end[1] = (uchar)(pass_data[RT_E_MONT] & 0x7f);
	p_magP->apspas.aps_end[2] = (uchar)(pass_data[RT_E_DAY]  & 0x7f);
															// �L���I���N����
	p_magP->apspas.aps_sts    = 0x02;						// �X�e�[�^�X=���ɌŒ�
	p_magP->apspas.aps_wrt[0] = (uchar)(base_data[RT_I_MONT] & 0x7f);
	p_magP->apspas.aps_wrt[1] = (uchar)(base_data[RT_I_DAY]  & 0x7f);
	p_magP->apspas.aps_wrt[2] = (uchar)(base_data[RT_I_HOUR] & 0x7f);
	p_magP->apspas.aps_wrt[3] = (uchar)(base_data[RT_I_MIN]  & 0x7f);
															// ������������=���Ɍ�������

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �T�[�r�X�����C�f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_mag	: ���C�f�[�^�̃|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_make_service( m_gtservic *p_magS )
{
	long	pno;				// ���ԏ�No.

	p_magS->servic.svc_idc = 0x2d;							// �T�[�r�X��
	if( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 ){			// ��{�T�[�r�X���g�p��
		pno = CPrmSS[S_SYS][1];								// ��{���ԏ�No.�擾
	}else if( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 ){	// �g��1�T�[�r�X���g�p��
		pno = CPrmSS[S_SYS][2];								// �g��1���ԏ�No.�擾
	}else if( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 ){	// �g��2�T�[�r�X���g�p��
		pno = CPrmSS[S_SYS][3];								// �g��2���ԏ�No.�擾
	}else{													// ���̑��͊g��3�T�[�r�X���g�p�Ƃ���
		pno = CPrmSS[S_SYS][4];								// �g��3���ԏ�No.�擾
	}
	p_magS->servic.svc_pno[0] = (uchar)(pno & 0x7f);		// P0�`P6
	if( 0 != (pno & 0x0080) ){
		p_magS->servic.svc_pno[1] |= 0x40;					// P7
	}
	if( 0 != (pno & 0x0100) ){
		p_magS->servic.svc_pno[1] |= 0x20;					// P8
	}
	if( 0 != (pno & 0x0200) ){
		p_magS->servic.svc_pno[1] |= 0x10;					// P9
	}
	if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){
		p_magS->magformat.type = 1;							// �����=GT�t�H�[�}�b�g
		p_magS->magformat.ex_pkno = (uchar)((pno & 0x3fc00) >> 10);
															// P10-P17
	}
	if( rt_ctrl.sub_mode == 2 ){							// �|����
															// �T�[�r�X�����=0�Œ�
		p_magS->servic.svc_sno[0]  = (uchar)((kake_data[RT_KAKE_NO] >> 7) & 0x7f);
		p_magS->servic.svc_sno[1]  = (uchar)(kake_data[RT_KAKE_NO] & 0x7f);
															// �XNo.
	}else{													// �T�[�r�X��
		p_magS->servic.svc_pno[1] |= (uchar)((serv_data[RT_SERV_KIND] + 1) & 0x0f);
															// �T�[�r�X�����
		p_magS->servic.svc_sno[1]  = 0x01;					// �XNo.(�|����R�[�h)=1�Œ�
	}
	p_magS->servic.svc_sta[0] = 0;							// �L���J�n�N=0�Œ�
	p_magS->servic.svc_sta[1] = 0;							// �L���J�n��=0�Œ�
	p_magS->servic.svc_sta[2] = 0;							// �L���J�n��=0�Œ�
	p_magS->servic.svc_end[0] = 99;							// �L���I���N=99�Œ�
	p_magS->servic.svc_end[1] = 99;							// �L���I����=99�Œ�
	p_magS->servic.svc_end[2] = 99;							// �L���I����=99�Œ�
															// �������L���Ƃ���

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �Y�����̍ŏI���Z�o
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			short	: �ő��
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_day_max( void )
{
	long	year;
	long	mont;
	short	day;

	if( rt_ctrl.sub_mode == 0 ){							// �e���
		if( rt_ctrl.pos == RT_I_DAY ){
			year = base_data[RT_I_YEAR];
			mont = base_data[RT_I_MONT];
		}else{
			year = base_data[RT_O_YEAR];
			mont = base_data[RT_O_MONT];
		}
	}else{													// �q���
		if( rt_ctrl.pos == RT_S_DAY ){
			year = pass_data[RT_S_YEAR];
			mont = pass_data[RT_S_MONT];
		}else{
			year = pass_data[RT_E_YEAR];
			mont = pass_data[RT_E_MONT];
		}
	}
	if( year < 80 ){
		year += 2000;
	}else{
		year += 1900;
	}
	day = medget((short)year, (short)mont);

	return(day);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ���o�Ɏ������
//[]----------------------------------------------------------------------[]
///	@param[in]		num      : ���Z�J�n�ضް�i���Ɏ��� �擾�����f�p�j<br>
///                 			    1:���Ԍ��}���i���Z�O�j<br>
///                 			    2:���Ԍ��}���i�|�����j<br>
///                 			    3:���Ԍ��}���i���~���j<br>
///                 			    4:�������Z<br>
///                 			    5:������}��<br>
///	@return			ret      : ���o�Ɏ�������(1980�`2079�N�ȓ���?)0:OK 1:NG
///	@author			
///	@attention		�O���[�o���ϐ�car_in, car_ot, car_in_f, car_ot_f�ɓ��o�Ɏ������Z�b�g����
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	:	2011/04/11<br>
///					Update	:	2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static short	RT_set_tim( ushort num, struct clk_rec *clk_para )
{
	ushort	in_tim;
	ushort	out_tim;
	ushort	in_year;
	ushort	out_year;

	// set_tim()���K�v�����𔲏o���ĕύX
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// ���Ԏ����ر
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// �o�Ԏ����ر
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// ���Ԏ���Fix�ر
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// �o�Ԏ���Fix�ر
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// �Čv�Z�p���Ԏ����ر

	if( base_data[RT_I_YEAR] < 80 ){
		in_year = (ushort)(base_data[RT_I_YEAR] + 2000);
	}else{
		in_year = (ushort)(base_data[RT_I_YEAR] + 1900);
	}
	if( base_data[RT_O_YEAR] < 80 ){
		out_year = (ushort)(base_data[RT_O_YEAR] + 2000);
	}else{
		out_year = (ushort)(base_data[RT_O_YEAR] + 1900);
	}

	car_in.year = in_year;											// ���� �N
	car_in.mon  = base_data[RT_I_MONT];								//      ��
	car_in.day  = base_data[RT_I_DAY];								//      ��
	car_in.hour = base_data[RT_I_HOUR];								//      ��
	car_in.min  = base_data[RT_I_MIN];								//      ��

	car_ot.year = out_year;											// �o�� �N
	car_ot.mon  = (uchar)base_data[RT_O_MONT];						//      ��
	car_ot.day  = (uchar)base_data[RT_O_DAY];						//      ��
	car_ot.hour = (uchar)base_data[RT_O_HOUR];						//      ��
	car_ot.min  = (uchar)base_data[RT_O_MIN];						//      ��

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// ���Ԏ���Fix
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// �o�Ԏ���Fix

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// ���Ɏ����K��O?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// �o�Ɏ����K��O?
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (���ɓ�+1�N)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// �o�ɓ�normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		return( 1 );
	}

	in_tim = dnrmlzm((short)( car_in.year ),						/* (���ɓ�)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){
		return( 1 );
	}else if( in_tim == out_tim ){										// ���ɔN����=�o�ɔN����
		in_tim  = tnrmlz (0, 0, (short)car_in.hour, (short)car_in.min);
		out_tim = tnrmlz (0, 0, (short)car_ot.hour, (short)car_ot.min);
		if( in_tim > out_tim ){										/*								*/
			return( 1 );
		}
	}																/*								*/
	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ���o�Ɏ������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/12<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_disp_cal_result( void )
{
	uchar	base_max;										// ��{��ʕ\�����ڍő吔
	char	disp_pos;										// �\���ʒu
	long	disp_data;										// �\���f�[�^

	// �����v�Z���ʂ̕\��
	dispclr();												// clear screen
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[1]);					// "�������v�Z�e�X�g���@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF,SMSTR4[16]);			// "�v�Z���ʁF�@�@�@�@�@�@�@�@�@�~"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[3]);					// "���Ɏ����F  �N  ��  ��  �F  �@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[4]);					// "�o�Ɏ����F  �N  ��  ��  �F  �@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[13]);					// "������ʁF�@��@�@�@�@�@�@�@�@"
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)			
//	base_max = RT_BASE_MAX;									// ���ڐ�
	base_max = RT_BASE_MAX - 1 ;									// ���ڐ�
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X�i�����e�i���X)	
	switch( rt_ctrl.sub_mode ){
	case  1:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[17]);				// "�g�p����F������@��ʁ@�@�@�@"
		opedpl(6, 22, (ulong)pass_data[RT_PASS_KIND],2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		base_data[RT_PASS] = pass_data[RT_PASS_KIND];
		break;
	case  2:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[19]);				// "�g�p����F�|�����@�X�@No�@�@�@"
		opedpl(6, 22, (ulong)kake_data[RT_KAKE_NO],3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case  3:
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[18]);				// "�g�p����F�T�[�r�X���@�@��@�@"
		grachr(6, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF,DAT1_4[serv_data[RT_SERV_KIND]]);
		break;
	default:
		break;
	}
	opedpl(1, 16, (ulong)ryo_buf.dsp_ryo,6, 0, 0, COLOR_FIREBRICK, LCD_BLINK_OFF);	// �v�Z����(����)�\��
	for (disp_pos = 0; disp_pos < base_max; disp_pos++) {
		disp_data = base_data[disp_pos];
		RT_change_disp(0, 0, disp_pos, disp_data, 0);
	}
	if( !ryo_buf.dsp_ryo ){									// �\������=0�~
		Fun_Dsp( FUNMSG2[56] );								// "�@�@�@�@�@�@�@�@�@ �o�^  �I�� "
	}else{
		Fun_Dsp( FUNMSG2[53] );								// "����� �T�� �|�����@�@�@ �I�� "
	}
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �f�[�^�o�b�N�A�b�v�^���X�g�A
//[]----------------------------------------------------------------------[]
///	@param[in]		char	kind = 0:�o�b�N�A�b�v�^1:���X�g�A
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/19<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_bkp_rst_data( char kind )
{
	if( !kind ){
		memcpy(&base_data_bk[0], &base_data[0], sizeof(long)*RT_BASE_MAX);
		memcpy(&pass_data_bk[0], &pass_data[0], sizeof(long)*RT_PASSINFO_MAX);
		memcpy(&kake_data_bk[0], &kake_data[0], sizeof(long)*RT_KAKEINFO_MAX);
		memcpy(&serv_data_bk[0], &serv_data[0], sizeof(long)*RT_SERVKIND_MAX);
		memcpy(&rt_ctrl_bk2, &rt_ctrl, sizeof(t_RT_Ctrl));
															// �f�[�^�o�b�N�A�b�v
	}else{
		memcpy(&base_data[0], &base_data_bk[0], sizeof(long)*RT_BASE_MAX);
		memcpy(&pass_data[0], &pass_data_bk[0], sizeof(long)*RT_PASSINFO_MAX);
		memcpy(&kake_data[0], &kake_data_bk[0], sizeof(long)*RT_KAKEINFO_MAX);
		memcpy(&serv_data[0], &serv_data_bk[0], sizeof(long)*RT_SERVKIND_MAX);
		memcpy(&rt_ctrl, &rt_ctrl_bk2, sizeof(t_RT_Ctrl));
															// �f�[�^���X�g�A
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �G���[�\���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/22<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_clear_error( void )
{

	if( rt_ctrl.mode == 0 ){
		grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[2]);						// "�e���ڂ���͂��ĉ������@�@�@�@"
		Fun_Dsp( FUNMSG2[2] );								// "�@���@�@���@�@�@�@ ����  �I�� "
	}else{
		switch( rt_ctrl.sub_mode ){
		case  1:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[8]);					// "�����������͂��ĉ������@�@"
			Fun_Dsp( FUNMSG2[2] );							// "�@���@�@���@�@�@�@ ����  �I�� "
			break;
		case  2:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[12]);				// "�|���������͂��ĉ������@�@"
			Fun_Dsp( FUNMSG2[1] );							// "�@�@�@�@�@�@�@�@�@ ����  �I�� "
			break;
		case  3:
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR4[14]);				// "�T�[�r�X����ʂ���͂��ĉ�����"
			Fun_Dsp( FUNMSG2[54] );							// "�@�@�@�@�@�@ �؊�  ����  �I�� "
			break;
		default:
			break;
		}
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g ���O�p�f�[�^�ҏW
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_edit_log_data( T_FrmReceipt *p_pri_data )
{
	ushort	num;
	
	// �ȉ��A�̎��؍Ĕ��s����(UsMnt_Receipt())���Q�l�ɍ쐬
	RT_PayData.TInTime.Year = car_in_f.year;						// ���� �N
	RT_PayData.TInTime.Mon  = car_in_f.mon;							//      ��
	RT_PayData.TInTime.Day  = car_in_f.day;							//      ��
	RT_PayData.TInTime.Hour = car_in_f.hour;						//      ��
	RT_PayData.TInTime.Min  = car_in_f.min;							//      ��
	RT_PayData.TOutTime.Year = car_ot_f.year;						// �o�� �N
	RT_PayData.TOutTime.Mon  = car_ot_f.mon;						//      ��
	RT_PayData.TOutTime.Day  = car_ot_f.day;						//      ��
	RT_PayData.TOutTime.Hour = car_ot_f.hour;						//      ��
	RT_PayData.TOutTime.Min  = car_ot_f.min;						//      ��
	if( ryo_buf.ryo_flg < 2 ){										// ���Ԍ����Z����
		RT_PayData.WPrice = ryo_buf.tyu_ryo;						// ���ԗ����Z�b�g
	}else{
		RT_PayData.WPrice = ryo_buf.tei_ryo;						// ��������Z�b�g
	}

	RT_PayData.Kikai_no    = (uchar)CPrmSS[S_PAY][2];		// �@�BNo.�Z�b�g
	RT_PayData.Seisan_kind = 0xFF;							// ���Z����=�����v�Z�e�X�g�Z�b�g

	RT_PayData.Oiban.w = 999999L;							// �ǂ���=999999�Ƃ���
	RT_PayData.Oiban.i = 999999L;							// �ǂ���=999999�Ƃ���

	RT_PayData.syu = (char)(ryo_buf.syubet + 1);			// ���Ԏ��

	num = ryo_buf.pkiti - 1;
	RT_PayData.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// �ڋq�p���Ԉʒu�ԍ�

	p_pri_data->prn_data = (Receipt_data *)&RT_PayData;		// �̎��؈��ް����߲�����
	p_pri_data->prn_kind = R_PRI;							// �������ʁFڼ��
	p_pri_data->reprint = OFF;								// �Ĕ��s�׸�ؾ�āi�ʏ�j


	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �L�����Ԍ�No.�^�����ID�擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	p_data	: ���C�f�[�^�̃|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	: 2011/04/28<br>
///					Update	: 2012/10/15
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
static void	RT_get_valid_no( char *p_data )
{
	m_gtapspas	*p_magP;		// �����
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	long	i;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	long	pno = 0;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	long	pass_id = 0;
	long	pass_id = 1;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	if( p_data[MAG_ID_CODE] == 0x1a ){						// �����
		p_magP   = (m_gtapspas *)p_data;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		for( i = 1; i < PAS_MAX+1; i++ ){
//			if( !pas_tbl[i-1].BIT.INV ){					// �L��
//				if(( !CPrmSS[S_SYS][61] ) ||				// ����e�[�u�������@�Ȃ�
//				   (( CPrmSS[S_SYS][61] ) && ( i < CPrmSS[S_SYS][61] ))){
//															// ����e�[�u�������@��菬
//					pno = CPrmSS[S_SYS][1];					// ��{���ԏ�No.�Ƃ���
//					pass_id = i;							// �����ID�͂��̂܂�
//				}else{
//					if(( !CPrmSS[S_SYS][62] ) ||			// ����e�[�u�������A�Ȃ�
//					   (( CPrmSS[S_SYS][62] ) && ( i < CPrmSS[S_SYS][62] ))){
//															// ����e�[�u�������A��菬
//						pno = CPrmSS[S_SYS][2];				// �g���P���ԏ�No.�Ƃ���
//						pass_id = i - CPrmSS[S_SYS][61] + 1;// �����ID=i-����e�[�u�������@+1
//					}else{
//						if(( !CPrmSS[S_SYS][63] ) ||		// ����e�[�u�������B�Ȃ�
//						   (( CPrmSS[S_SYS][63] ) && ( i < CPrmSS[S_SYS][63] ))){
//															// ����e�[�u�������B��菬
//							pno = CPrmSS[S_SYS][3];			// �g���Q���ԏ�No.�Ƃ���
//							pass_id = i - CPrmSS[S_SYS][62] + 1;
//															// �����ID=i-����e�[�u�������A+1
//						}else{
//							pno = CPrmSS[S_SYS][4];			// �g���R���ԏ�No.�Ƃ���
//							pass_id = i - CPrmSS[S_SYS][63] + 1;
//															// �����ID=i-����e�[�u�������B+1
//						}
//					}
//				}
//				break;
//			}
//		}
//		if( !pno ){
//			// �L�������������Ȃ������ꍇ�A��{���ԏ�̒����ID=1�Ƃ���
//			pno = CPrmSS[S_SYS][1];							// ��{���ԏ�No.�Ƃ���
//			pass_id = 1;									// �����ID=1�Ƃ���
//		}
		pno = CPrmSS[S_SYS][1];							// ��{���ԏ�No.�Ƃ���
		pass_id = 1;									// �����ID=1�Ƃ���
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
		p_magP->apspas.aps_pno[0] = (uchar)(pno & 0x7f);	// ���ԏ�No.P0�`P6�ݒ�
		if( 0 != (pno & 0x0080) ){
			p_magP->apspas.aps_pno[1] |= 0x40;				// ���ԏ�No.P7�ݒ�
		}
		if( 0 != (pno & 0x0100) ){
			p_magP->apspas.aps_pno[1] |= 0x20;				// ���ԏ�No.P8�ݒ�
		}
		if( 0 != (pno & 0x0200) ){
			p_magP->apspas.aps_pno[1] |= 0x10;				// ���ԏ�No.P9�ݒ�
		}
		if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){
			p_magP->magformat.type = 1;						// �����=GT�t�H�[�}�b�g
			p_magP->magformat.ex_pkno = (uchar)((pno & 0x3fc00) >> 10);
															// P10-P17
		}
		p_magP->apspas.aps_pcd[0] |= (uchar)((pass_id >> 7) & 0x7f);
		p_magP->apspas.aps_pcd[1] |= (uchar)(pass_id & 0x7f);// �����ID�ݒ�
	}

	return;
}
//[]-----------------------------------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g �����v�Z�������C��
//[]-----------------------------------------------------------------------------------------------[]
///	PARAMETER	: r_knd	; �����v�Z���															   |*/
///						;   0:���Ԍ�															   |*/
///						;   1:�����															   |*/
///						;   2:����߲�޶���														   |*/
///						;   3:���޽��															   |*/
///						;   4:���~��(���g�p)													   |*/
///						;   6:�|�����Ԍ�(���g�p)												   |*/
///						; 100:�C�����Z															   |*/
///				  num	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
///	 RETURN VALUE: void	;																		   |*/
//[]----------------------------------------------------------------------------------------------[]
///	@author			Y.shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---------------------------[]
static	void	RT_ryo_cal2( char r_knd, ushort num )							/*								*/
{																	/*								*/
	ulong	sot_tax = 0L;											/*								*/
	ulong	dat = 0L;												/*								*/
	struct	REQ_RKN		*rhs_p;										/*								*/
	short	calcreqnum	;											/* ���������v�Z�v����			*/
	char	cnt;													/*								*/
	char	i;														/*								*/
	uchar	wrcnt = 0;												/*								*/
	ushort	wk_dat;													/*								*/
	ushort	wk_dat2;												/*								*/
	ulong	wk_dat3;												/*								*/
	ulong	wk_pay_ryo;				//�Đ��Z�p�v���y�C�h���z�i�[�G���A

	struct	CAR_TIM	Adjustment_Beginning;							/*								*/
																	/*								*/
	memcpy( &Adjustment_Beginning, &car_ot, sizeof( Adjustment_Beginning ) );	/*					*/
																	/*								*/
	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
																	/*								*/
	if( r_knd == 0 ){																/*								*/
		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* ���݌v�Z��i�[�ر�ر			*/
		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* �����ޯ̧�ر�				*/
		ryo_buf.ryo_flg = 0;										/* �ʏ헿�ྯ�					*/
		ryo_buf.pkiti = num;										/* ���Ԉʒu�ԍ����				*/
		memset( &RT_PayData, 0, sizeof( Receipt_data ) );			/* ���Z���,�̎��؈��ް�		*/
		memset( &PayData, 0, sizeof( Receipt_data ) );				/* ���Z���,�̎��؈��ް�		*/
// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
		memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	/* ���Z�f�[�^�ێ�				*/
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
		memset( &PPC_Data_Detail, 0, sizeof( PPC_Data_Detail ));	/* ����ߏڍ׈󎚴ر�ر			*/	
		ntnet_nmax_flg = 0;											/*�@�ő嗿���׸޸ر	*/
		memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );	/* NT-NET���Z�ް��t�����ر	*/
		memset( RT_PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* ���Z�ް��������쐬�ر�ر	*/
		memset( card_use, 0, sizeof( card_use ) );					/* 1���Z���ޖ��g�p����ð���		*/
		memset( card_use2, 0, sizeof( card_use2 ) );				/* 1���Z���޽�����g�p����ð���	*/
		CardUseSyu = 0;												/* 1���Z�̊�����ނ̌����ر		*/
		discount_tyushi = 0;										/*�@�����z�@�@�@�@�@�@�@�@�@�@�@*/
		memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));		/*	�ڍג��~�ر�ر				*/
		Flap_Sub_Num = 0;											/*	�ڍג��~�ر					*/
		RT_vl_carchg( num );										/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ryo_buf_n.require = ryo_buf.zankin;								/*								*/
	ryo_buf_n.dis     = ryo_buf.waribik;							/*								*/
	ryo_buf_n.nyu     = ryo_buf.nyukin;								/*								*/
																	/*								*/
	tol_dis = ryo_buf.waribik;										/*								*/
																	/*								*/
	ec09();															/* VL�������Ұ����				*/
	if( CPrmSS[S_STM][1] == 0 ){									/* �S���ʻ��޽���,��ڰ����		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ���޽���get					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ��ڰ����get					*/
	}else{															/* ��ʖ����޽���				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(rysyasu-1))];		/* ��ʖ����޽���get			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(rysyasu-1))];		/* ��ʖ���ڰ����get			*/
	}																/*								*/

	SvsTime_Syu[num-1] = rysyasu;								/* ���Z���̗�����ʾ�āi׸���я����p�j*/

	et02();															/* �����v�Z						*/
	for( calcreqnum =  0 ;											/* �����v�Z�v����				*/
		  calcreqnum < (short)req_crd_cnt	;						/* ex.�e�����ɗ����v�Z			*/
		  calcreqnum ++					)							/* ���������قȂ�				*/
	{																/*								*/
		memcpy( &req_rkn, req_crd + calcreqnum,						/* �����v�Z�v������				*/
				sizeof(req_rkn)				) ;						/* ex.2 ���ڈȍ~���v����		*/
		et02()	;													/* �����v�Z						*/
	}																/*								*/
																	/*								*/
	if( tki_flg != OFF )											/* ���ԑђ���㎞�Ԋ��� 		*/
	{																/* �������Ȃ�					*/
		if( tki_ken ){												/*								*/
			ryo_buf.dis_fee = 0L;									/* �O�񗿋������z�ر			*/
			ryo_buf.dis_tim = 0L;									/* �O�񎞊Ԋ����z�ر			*/
			ryo_buf.fee_amount = 0L;								/* ���������z�ر				*/
			ryo_buf.tim_amount = 0L;								/* �������Ԑ��ر				*/
			wk_pay_ryo = PayData_Sub.pay_ryo;
			memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
			PayData_Sub.pay_ryo = wk_pay_ryo;
			wrcnt_sub = 0;
			discount_tyushi = 0;									/*�@�����z�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
		}															/*								*/
		for( i = 0; i < (short)tki_ken; i++ )						/* �ۑ����ėL�闿���v�Z 		*/
		{															/* ��ٰ��						*/
			memset( &req_rkn.param,0x00,21);						/* �����v��ð��ٸر				*/
			memcpy( (char *)&req_rkn,								/* �ۑ����Ă������v�Z�v 		*/
					(char *)&req_tki[i],10 );						/* �����Ăїv��					*/
			if((req_rkn.param == RY_FRE_K)							/* �g��1�񐔌������v��			*/
			 ||(req_rkn.param == RY_FRE)							/* �񐔌������v��				*/
			 ||(req_rkn.param == RY_FRE_K2)							/* �g��2�񐔌������v��			*/
			 ||(req_rkn.param == RY_FRE_K3)							/* �g��3�񐔌������v��			*/
			 ||(req_rkn.param == RY_PCO_K2)							/* �g��2����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO_K3)							/* �g��3����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO_K)							/* �g��1����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO))							/* ����߲�޶���					*/
			{														/*								*/
			   req_rkn.param = 0xff;								/*								*/
			}														/*								*/
			et02();													/* �����v�Z����					*/
		}															/*								*/
		tki_ken = 0;												/* �����v�Z�ۑ������ر			*/
		re_req_flg = OFF;											/* ���ԑђ���㎞�Ԋ��� 		*/
	}																/* �׸�ؾ��						*/
																	/*								*/
	ryo_buf.zankin  = ryo_buf_n.require ;							/* ���ԗ��ྯ�					*/
	ryo_buf.tax     = ryo_buf_n.tax;								/* �ŋྯ�						*/
	ryo_buf.waribik = ryo_buf_n.dis;								/* �����z						*/
	if( OPECTL.Ope_mod != 22 )										/* ����X�V���łȂ��Ȃ�			*/
	ryo_buf.syubet  = (char)(rysyasu - 1);							/* ������ʾ��(��ʐ؊���)		*/
																	/*								*/
	if( r_knd == 1 )												/* ���������?					*/
	{																/*								*/
		ryo_buf.ryo_flg += 2;										/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* ���Ԍ����Z����				*/
	{																/*								*/
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X�i�����e�i���X)
//		ryo_buf.tik_syu = rysyasu;									/* �������						*/
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;							/* �������					*/
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)		
	}																/*								*/
	else															/*								*/
	{															/*								*/
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		ryo_buf.pas_syu = rysyasu;									/* ������̗������				*/
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
		ryo_buf.tei_ryo = ryo_buf_n.ryo;							/* ��������ྯ�				*/
	}																/*								*/
																	/*								*/
	/*====================================================*/		/*								*/
	// ����������O�̊e�����������폜����							/*								*/
	/*====================================================*/		/*								*/
	for( cnt = 0; cnt < rhspnt; cnt++ )								/* �����v�Z���ް����ɂȂ�܂�	*/
	{																/*								*/
		rhs_p = &req_rhs[cnt];										/*								*/
		if(( rhs_p->param == RY_PKC		||							/* �����v�Z�v��������������̏ꍇ	*/
			 rhs_p->param == RY_PKC_K	||							/*								*/
			 rhs_p->param == RY_PKC_K2	||							/*								*/
			 rhs_p->param == RY_PKC_K3	||				/*								*/
			 ((rhs_p->param == RY_KCH) && (( rhs_p->data[1] >> 16 )== 2))) &&	// ������Ԏ�ؑ�
			 cnt != 0 )
		{															/*								*/
			for( i = (char)(cnt-1); i > 0; i-- )					/*								*/
			{														/*								*/
				rhs_p = &req_rhs[i];								/*								*/
				if( rhs_p->param == RY_TWR || 						/* ������������					*/
					rhs_p->param == RY_RWR || 						/* ���Ԋ�������					*/
					rhs_p->param == RY_SKC || 						/* ���޽������					*/
					rhs_p->param == RY_SKC_K ||						/* �g�����޽������				*/
					rhs_p->param == RY_SKC_K2 ||					/* �g��2���޽������				*/
					rhs_p->param == RY_SKC_K3 ||					/* �g��3���޽������				*/
					rhs_p->param == RY_KAK ||						/* �|��������					*/
					rhs_p->param == RY_KAK_K ||						/* �g��1�|����					*/
					rhs_p->param == RY_KAK_K2 ||					/* �g��2�|����					*/
					rhs_p->param == RY_KAK_K3 ||					/* �g��3�|����					*/
					rhs_p->param == RY_SSS ||						/* �C��							*/
					rhs_p->param == RY_CSK ||						/* ���Z���~���޽��				*/
					rhs_p->param == RY_SNC			)				/* �X��������					*/
				{													/*								*/
					rhs_p->param = 0xff;							/* �v���ް�FF�ر				*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	/*==============================================================================================*/
	/* �������Ұ����̎���(�ʐ��Z���܂�)�A���Z�ް��̊���������S�Ă����ō쐬����				*/
	/*==============================================================================================*/
	if( r_knd != 0 )												/* ���Ԍ��ȊO?					*/
	{																/*								*/
		memset( RT_PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* ���Z�ް��������쐬�ر�ر	*/
																	/*								*/
		for( cnt = 0; cnt < rhspnt; cnt++ )							/*								*/
		{															/*								*/
			rhs_p = &req_rhs[cnt];									/*								*/
			switch( rhs_p->param )									/* �����v�Z�v�����ނɂ�蕪��	*/
			{														/*								*/
			  case RY_SKC:											/* ��{���޽��					*/
			  case RY_SKC_K:										/* �g��1���޽��					*/
			  case RY_SKC_K2:										/* �g��2���޽��					*/
			  case RY_SKC_K3:										/* �g��3���޽��					*/
				wk_dat = (ushort)((rhs_p->data[0] &					/* ��޾��						*/
									0xffff0000) >> 16);				/*								*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* ������0�Ȃ�1���				*/
				}													/*								*/
																	/*								*/
				if(( wk_dat >= 1 )&&( wk_dat <= SVS_MAX )){			/* ���޽��A�`O					*/
					// ���Z�ް��p									/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//					wk_dat2 = (ushort)CPrmSS[S_SER][1+3*(wk_dat-1)];/*								*/
					wk_dat2 = (ushort)prm_get( COM_PRM, S_SER, (short)(1+3*(wk_dat-1)), 1, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					if( wk_dat2 == 1 ){								/*								*/
						wk_dat2 = NTNET_SVS_T;						/* �������=���޽��(����)		*/
					}else{											/*								*/
						wk_dat2 = NTNET_SVS_M;						/* �������=���޽��(����)		*/
					}												/*								*/
																	/*								*/
					if( rhs_p->param == RY_SKC ){					/* ��{���ԏꇂ					*/
						wk_dat3 = CPrmSS[S_SYS][1];					/* ���ԏꇂ���					*/
					}else if( rhs_p->param == RY_SKC_K ){			/* �g��1���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][2];					/* ���ԏꇂ���					*/
					}else if( rhs_p->param == RY_SKC_K2 ){			/* �g��2���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][3];					/* ���ԏꇂ���					*/
					}else{											/* �g��3���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][4];					/* ���ԏꇂ���					*/
					}												/*								*/
																	/*								*/
					for( i=0; i < WTIK_USEMAX; i++ ){				/*								*/
						if(( RT_PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* ���ԏꇂ��������?*/
						   ( RT_PayData.DiscountData[i].DiscSyu == wk_dat2 )&&		/* ������ʂ�������?*/
						   ( RT_PayData.DiscountData[i].DiscNo == wk_dat )&&		/* �����敪��������?*/
						   ( RT_PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x0000ffff) ))	/* �������1(�|�����)��������? */
						{											/*								*/
							RT_PayData.DiscountData[i].DiscCount +=	/* �g�p����						*/
										(ushort)rhs_p->data[1];		/*								*/
							RT_PayData.DiscountData[i].Discount += rhs_p->data[2];	/* �����z			*/
							if( wk_dat2 == NTNET_SVS_T ){			/* ���Ԋ���?					*/
								RT_PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*			*/
									prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));	/* �������2(�������Ԑ��F��) */
							}										/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){	/* �����Y������					*/
						RT_PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* ���ԏꇂ					*/
						RT_PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* �������					*/
						RT_PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* �����敪(���޽��A�`C)	*/
						RT_PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
										(ushort)rhs_p->data[1];		/*								*/
						RT_PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* �����z			*/
						RT_PayData.DiscountData[wrcnt].DiscInfo1 =		/*								*/
									rhs_p->data[0] & 0x0000ffff;	/* �������1(�|�����)			*/
						if( wk_dat2 == NTNET_SVS_T ){				/* ���Ԋ���?					*/
							RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*			*/
								prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));/* �������2(�������Ԑ�) */
						}else{										/*								*/
							RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;	/* �������2(���g�p) */
						}											/*								*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 						RT_PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p			*/
// 						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z) */
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
						wrcnt++;									/*								*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_KAK:											/* ��{�|����(�X������)			*/
			  case RY_KAK_K:										/* �g��1�|����(�X������)		*/
			  case RY_KAK_K2:										/* �g��2�|����(�X������)		*/
			  case RY_KAK_K3:										/* �g��3�|����(�X������)		*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* ������0�Ȃ�1���				*/
				}													/*								*/
				// ���Z�ް��p										/*								*/
				wk_dat = (ushort)rhs_p->data[0];					/* �X��							*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				wk_dat2 = (ushort)CPrmSS[S_STO][1+3*(wk_dat-1)];	/*								*/
				wk_dat2 = (ushort)prm_get( COM_PRM, S_STO, (short)(1+3*(wk_dat-1)), 1, 1 );   /*	*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
				if( wk_dat2 == 1 ){									/*								*/
					wk_dat2 = NTNET_KAK_T;							/* �������=�X����(����)		*/
				}else{												/*								*/
					wk_dat2 = NTNET_KAK_M;							/* �������=�X����(����)		*/
				}													/*								*/
																	/*								*/
				if( rhs_p->param == RY_KAK ){						/* ��{���ԏꇂ					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_KAK_K ){				/* �g��1���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_KAK_K2 ){				/* �g��2���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* ���ԏꇂ���					*/
				}else{												/* �g��3���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* ���ԏꇂ���					*/
				}													/*								*/
																	/*								*/
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/
					if(( RT_PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* ���ԏꇂ��������?	*/
					   ( RT_PayData.DiscountData[i].DiscSyu == wk_dat2 )&&	/* ������ʂ�������?		*/
					   ( RT_PayData.DiscountData[i].DiscNo == wk_dat ))	/* �����敪��������?		*/
					{												/*								*/
						RT_PayData.DiscountData[i].DiscCount +=		/* �g�p����						*/
									(ushort)rhs_p->data[1];			/*								*/
						RT_PayData.DiscountData[i].Discount += rhs_p->data[2];	/* �����z				*/
						if( wk_dat2 == NTNET_KAK_T ){				/* ���Ԋ���?					*/
							RT_PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*				*/
								prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));	/* �������2(�������Ԑ��F��) */
						}											/*								*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* �����Y������					*/
					RT_PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* ���ԏꇂ						*/
					RT_PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* �������						*/
					RT_PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* �����敪(�X��)				*/
					RT_PayData.DiscountData[wrcnt].DiscCount =			/* �g�p����						*/
										(ushort)rhs_p->data[1];		/*								*/
					RT_PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* �����z				*/
					RT_PayData.DiscountData[wrcnt].DiscInfo1 = 0L;		/* �������1(���g�p)			*/
					if( wk_dat2 == NTNET_KAK_T ){					/* ���Ԋ���?					*/
						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*				*/
							prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));/* �������2(�������Ԑ�)*/
					}else{											/*								*/
						RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;/* �������2(���g�p) */
					}												/*								*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					RT_PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p				*/
// 					RT_PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z)*/
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		CardUseSyu = wrcnt;											/* 1���Z�̊�����ނ̌����ر		*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg <= 1 )										/* ����g�p�������Z�H			*/
	{																/*								*/
		if(	ryo_buf.tyu_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tyu_ryo -	ryo_buf.waribik;	/* �ېőΏۊz�����ԗ��� �| �����z�v	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	} else {														/* ����g�p						*/
		if(	ryo_buf.tei_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tei_ryo -	ryo_buf.waribik;	/* �ېőΏۊz����������ԗ��� �| �����z�v	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( CPrmSS[S_CAL][19] )											/* �ŗ��ݒ肠��					*/
	{																/*								*/
		if( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )					/* �O��?						*/
		{															/*								*/
			sot_tax = ryo_buf.tax;									/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	switch( ryo_buf.ryo_flg )										/*								*/
	{																/*								*/
		case 0:														/* �ʏ�(����g�p����)���Z		*/
			if(( ryo_buf.tyu_ryo + sot_tax )						/* ���ԗ����{�O�� �� �����z�{�����z�v?	*/
					< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay ))	/*								*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay )	/*								*/
						- ( ryo_buf.tyu_ryo + sot_tax );			/*								*/
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* ������������߂�				*/
				}													/*								*/
			}														/*								*/

			if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )	/* �\��������(���ԗ����{�O��)	*/
								- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	�|�����z�v	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* �\���������O					*/
				if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		case 2:														/* �ʏ�(����g�p)���Z			*/
																	/* �����z = ���ԗ���			*/
			if(( ryo_buf.tei_ryo + sot_tax )						/* ������ԗ����{�O�� �� �����z�{�����z�v�H*/
			< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*					*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)	/*				*/
					- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* ������������߂�				*/
				}													/*								*/
			}														/*								*/
			if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*	*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* �\��������(������ԗ����{�O��)	*/
								- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		�|�����z�v	*/
			} else {												/*								*/
			ryo_buf.dsp_ryo = 0l;									/* �\���������O					*/
				if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
	}																/*								*/
	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
	memcpy( &car_ot, &Adjustment_Beginning, sizeof( car_ot ) );		/*								*/
	return;															/*								*/
}																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���Ԍ��ް�����(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: RT_vl_carchg( no )															   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: Y.shiraishi																	   |*/
/*| Date		: 2012-11-08																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RT_vl_carchg( ushort no )
{
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	 if( no >= 1 && no <= LOCK_MAX ){							// ���Ԉʒu�ԍ�����(1�`LOCK_MAX)
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)	

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );		// ���Ԍ��ް�(vl_tik)������(0�ر)
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		 vl_tik.syu = LockInfo[no-1].ryo_syu;					// ��ʾ��(���Ԉʒu�ݒ薈)
		vl_tik.syu = base_data[RT_KIND]+1;
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
		cr_dat_n = 0x24;										// ��ID���޾��(���Z�O��)
		vl_now = V_CHM;											// �Ǎ��݌���ʾ��(���Z�O��)

		vl_tik.chk = 0;											// ���Z�O�����

		vl_tik.cno = KIHON_PKNO;								// ���ԏꇂ���(��{���ԏꇂ)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// �����@No.
		vl_tik.hno = 0;											// ����No.
		vl_tik.mno = 0;											// �X�����

		vl_tik.tki = car_in_f.mon;								// ���Ԍ����
		vl_tik.hii = car_in_f.day;								// ���ԓ����
		vl_tik.jii = car_in_f.hour;								// ���Ԏ����
		vl_tik.fun = car_in_f.min;					 			// ���ԕ����

		syashu       = vl_tik.syu	;							// �Ԏ�

		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// ���ԔN
		car_in.mon   = vl_tik.tki;								//     ��
		car_in.day   = vl_tik.hii;								//     ��
		car_in.hour  = vl_tik.jii;								//     ��
		car_in.min   = vl_tik.fun;								//     ��
		car_in.week  = (char)youbiget( car_in.year ,			//     �j��
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// �o�ɔN
		car_ot.mon  = hzuk.m;									//     ��
		car_ot.day  = hzuk.d;									//     ��
		car_ot.week = hzuk.w;									//     �j��
		car_ot.hour = jikn.t;									//     ��
		car_ot.min  = jikn.m;									//     ��

		Flap_Sub_Flg = 0;
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	 }
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
}

//[]----------------------------------------------------------------------[]
///	@brief			�����v�Z�e�X�g log�o�^
//[]----------------------------------------------------------------------[]
/*| MODULE NAME  : Log_regist( Lno )                                       |*/
/*| PARAMETER    : ۸އ�                                                   |*/
/*| RETURN VALUE : void                                                    |*/
//[]----------------------------------------------------------------------[]
///	@author			Y.shiraishi
///	@date			Create	: 2012/10/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void RT_log_regist( void )
{
	memcpy( &RT_PAY_LOG_DAT.RT_pay_log_dat[RT_PAY_LOG_DAT.RT_pay_wtp], &RT_PayData, sizeof( Receipt_data ) );
	if( RT_PAY_LOG_DAT.RT_pay_count < RT_PAY_LOG_CNT ){
		RT_PAY_LOG_DAT.RT_pay_count++;	/* RAM�o�^������+1 */
	}

	RT_PAY_LOG_DAT.RT_pay_wtp++;			/* ײ��߲�� +1 */
	if( RT_PAY_LOG_DAT.RT_pay_wtp >= RT_PAY_LOG_CNT ){
		RT_PAY_LOG_DAT.RT_pay_wtp = 0;
	}

	return;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		���O�f�[�^�N���A����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogDataClr( LogSyu )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	LogSyu	= ���O�f�[�^���					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret = OK/NG									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	LogDataClr_CheckBufferFullRelease(ushort LogSyu )
{
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
#if 0		// UT4000�̏������g�p����
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	switch( LogSyu ){				// ���O�f�[�^��ʁH
		case	LOG_ERROR:			// �G���[���
			Log_CheckBufferFull(FALSE, eLOG_ERROR, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_ALARM:			// �A���[�����
			Log_CheckBufferFull(FALSE, eLOG_ALARM, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_OPERATE:		// ������
			Log_CheckBufferFull(FALSE, eLOG_OPERATE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_POWERON:		// �╜�d���
			Log_CheckBufferFull(FALSE, eLOG_POWERON, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_TTOTAL:			// �s���v���
			Log_CheckBufferFull(FALSE, eLOG_TTOTAL, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//			Log_CheckBufferFull(FALSE, eLOG_LCKTTL, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			break;

		case	LOG_GTTOTAL:		// �f�s���v���
			Log_CheckBufferFull(FALSE, eLOG_GTTOTAL, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_COINBOX:		// �R�C�����ɏ��
			Log_CheckBufferFull(FALSE, eLOG_COINBOX, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_NOTEBOX:		// �������ɏ��
			Log_CheckBufferFull(FALSE, eLOG_NOTEBOX, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_MONEYMANAGE:	// �ޑK�Ǘ����
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_PAYMENT:		// �ʐ��Z���
			Log_CheckBufferFull(FALSE, eLOG_PAYMENT, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_ABNORMAL:		// �s���E�����o�ɏ��
			Log_CheckBufferFull(FALSE, eLOG_ABNORMAL, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_MONITOR:		// ���j�^���
			Log_CheckBufferFull(FALSE, eLOG_MONITOR, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_CREUSE:			// �N���W�b�g���p���׃��O
			Log_CheckBufferFull(FALSE, eLOG_CREUSE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case	LOG_EDYARM:			// Edy�A���[��������O
//			Log_CheckBufferFull(FALSE, eLOG_EDYARM, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
//			break;
//
//		case	LOG_EDYSHIME:			// Edy���ߋL�^���O
//			Log_CheckBufferFull(FALSE, eLOG_EDYSHIME, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		case LOG_HOJIN_USE:
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_NGLOG:						// �s�����O
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_REMOTE_SET:					// ���u�����ݒ胍�O
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_SET, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//		case	LOG_ENTER:						// ���Ƀ��O
//			Log_CheckBufferFull(FALSE, eLOG_ENTER, eLOG_TARGET_REMOTE);		// �o�b�t�@�t�������`�F�b�N
//			break;
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
		
		case	LOG_PARKING:					// ���ԑ䐔�f�[�^
			Log_CheckBufferFull(FALSE, eLOG_PARKING, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;
		
		case	LOG_LONGPARK:					// �������ԃf�[�^
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//			Log_CheckBufferFull(FALSE, eLOG_LONGPARK, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			Log_CheckBufferFull(FALSE, eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
			break;
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//		case LOG_IOLOG:
//			Log_CheckBufferFull(FALSE, eLOG_IOLOG, eLOG_TARGET_REMOTE);		// �o�b�t�@�t�������`�F�b�N
//			break;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)

		case	LOG_MNYMNG_SRAM:				// �ޑK�Ǘ����(SRAM)
			Log_CheckBufferFull(FALSE, eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);		// �o�b�t�@�t�������`�F�b�N
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);		// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_REMOTE_MONITOR:
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;

		default:								// ���̑��i���O�f�[�^��ʃG���[�j
			break;
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
#endif
	short	target;					// �`�F�b�N�Ώ�
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
	short	targetList[NTNET_TARGET_MAX];
	int		targetCount = 0;		// �Ώۃ^�[�Q�b�g��
	int		index = 0;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
//	if(_is_ntnet_normal()) {		// NT-NET
//		target = eLOG_TARGET_NTNET;
//	}
//	else if(_is_ntnet_remote()) {	// ���uNT-NET
//		target = eLOG_TARGET_REMOTE;
//	}
//	else {
//		return;
//	}
	memset(targetList, 0, sizeof(targetList));
	if(_is_ntnet_normal()) {		// NT-NET
		targetList[targetCount] = eLOG_TARGET_NTNET;
		++targetCount;
	}

	if(_is_ntnet_remote()) {		// ���uNT-NET
		targetList[targetCount] = eLOG_TARGET_REMOTE;
		++targetCount;
	}

	if(targetCount <= 0) {
		// �[���ԁA���u���ݒ肪�Ȃ��Ȃ牽�����Ȃ�
		return;
	}

	for(index = 0; index < targetCount; ++index) {
		target = targetList[index];
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
	switch( LogSyu ){				// ���O�f�[�^��ʁH
		case	LOG_ERROR:			// �G���[���
			Log_CheckBufferFull(FALSE, eLOG_ERROR, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_ALARM:			// �A���[�����
			Log_CheckBufferFull(FALSE, eLOG_ALARM, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_OPERATE:		// ������
			Log_CheckBufferFull(FALSE, eLOG_OPERATE, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_POWERON:		// �╜�d���
			Log_CheckBufferFull(FALSE, eLOG_POWERON, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_TTOTAL:			// �s���v���
			Log_CheckBufferFull(FALSE, eLOG_TTOTAL, target);				// �o�b�t�@�t�������`�F�b�N
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
//			Log_CheckBufferFull(FALSE, eLOG_LCKTTL, target);				// �o�b�t�@�t�������`�F�b�N
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
			break;

		case	LOG_GTTOTAL:		// �f�s���v���
			Log_CheckBufferFull(FALSE, eLOG_GTTOTAL, target);				// �o�b�t�@�t�������`�F�b�N
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
//// MH364300 GG119A17(S) // MH341110(S) A.Iiizumi 2017/12/20 �O��T/GT���v�ŎԎ��ʒu���W�v�̈󎚂�����Ȃ��s��C��(GT�p�Ԏ��ʒu���W�v���O�ǉ�) (���ʉ��P��1392)
//			Log_CheckBufferFull(FALSE, eLOG_GT_LCKTTL, target);				// �o�b�t�@�t�������`�F�b�N
//// MH364300 GG119A17(E) // MH341110(E) A.Iiizumi 2017/12/20 �O��T/GT���v�ŎԎ��ʒu���W�v�̈󎚂�����Ȃ��s��C��(GT�p�Ԏ��ʒu���W�v���O�ǉ�) (���ʉ��P��1392)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
			break;

		case	LOG_COINBOX:		// �R�C�����ɏ��
			Log_CheckBufferFull(FALSE, eLOG_COINBOX, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_NOTEBOX:		// �������ɏ��
			Log_CheckBufferFull(FALSE, eLOG_NOTEBOX, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_MONEYMANAGE:	// �ޑK�Ǘ����
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, target);			// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_PAYMENT:		// �ʐ��Z���
			Log_CheckBufferFull(FALSE, eLOG_PAYMENT, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_ABNORMAL:		// �s���E�����o�ɏ��
			Log_CheckBufferFull(FALSE, eLOG_ABNORMAL, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_MONITOR:		// ���j�^���
			Log_CheckBufferFull(FALSE, eLOG_MONITOR, target);				// �o�b�t�@�t�������`�F�b�N
			break;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�̏����ɖ߂��j
//// MH321801(S) ��������L�^�����𑝂₷
////		case	LOG_CREUSE:			// �N���W�b�g���p���׃��O
////			Log_CheckBufferFull(FALSE, eLOG_CREUSE, target);				// �o�b�t�@�t�������`�F�b�N
////			break;
//// MH321801(E) ��������L�^�����𑝂₷
		case	LOG_CREUSE:			// �N���W�b�g���p���׃��O
			Log_CheckBufferFull(FALSE, eLOG_CREUSE, eLOG_TARGET_REMOTE);	// �o�b�t�@�t�������`�F�b�N
			break;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�̏����ɖ߂��j

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case	LOG_EDYARM:			// Edy�A���[��������O
//			Log_CheckBufferFull(FALSE, eLOG_EDYARM, target);				// �o�b�t�@�t�������`�F�b�N
//			break;
//
//		case	LOG_EDYSHIME:			// Edy���ߋL�^���O
//			Log_CheckBufferFull(FALSE, eLOG_EDYSHIME, target);				// �o�b�t�@�t�������`�F�b�N
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		case LOG_HOJIN_USE:
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_NGLOG:						// �s�����O
			Log_CheckBufferFull(FALSE, eLOG_HOJIN_USE, target);				// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_REMOTE_SET:					// ���u�����ݒ胍�O
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_SET, target);				// �o�b�t�@�t�������`�F�b�N
			break;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
//		case	LOG_ENTER:						// ���Ƀ��O
//			Log_CheckBufferFull(FALSE, eLOG_ENTER, target);					// �o�b�t�@�t�������`�F�b�N
//			break;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
		
		case	LOG_PARKING:					// ���ԑ䐔�f�[�^
			Log_CheckBufferFull(FALSE, eLOG_PARKING, target);				// �o�b�t�@�t�������`�F�b�N
			break;
		
		case	LOG_LONGPARK:					// �������ԃf�[�^
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//			Log_CheckBufferFull(FALSE, eLOG_LONGPARK, target);				// �o�b�t�@�t�������`�F�b�N
			Log_CheckBufferFull(FALSE, eLOG_LONGPARK_PWEB, target);			// �o�b�t�@�t�������`�F�b�N
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
			break;
		
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
//		case LOG_IOLOG:
//			Log_CheckBufferFull(FALSE, eLOG_IOLOG, target);					// �o�b�t�@�t�������`�F�b�N
//			break;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j

		case	LOG_MNYMNG_SRAM:				// �ޑK�Ǘ����(SRAM)
			Log_CheckBufferFull(FALSE, eLOG_MNYMNG_SRAM, target);					// �o�b�t�@�t�������`�F�b�N
			Log_CheckBufferFull(FALSE, eLOG_MONEYMANAGE, target);					// �o�b�t�@�t�������`�F�b�N
			break;

		case	LOG_REMOTE_MONITOR:
			Log_CheckBufferFull(FALSE, eLOG_REMOTE_MONITOR, target);	// �o�b�t�@�t�������`�F�b�N
			break;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j
//// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//		case	LOG_LEAVE:						// �o�Ƀ��O
//			Log_CheckBufferFull(FALSE, eLOG_LEAVE, target);					// �o�b�t�@�t�������`�F�b�N
//			break;
//// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100�s�v�����j

		default:								// ���̑��i���O�f�[�^��ʃG���[�j
			break;
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
}
