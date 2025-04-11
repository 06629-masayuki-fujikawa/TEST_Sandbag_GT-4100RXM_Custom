/*[]----------------------------------------------------------------------[]*/
/*| հ�ް����ݽ����                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :                                                          |*/
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
#include	"irq1.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"flp_def.h"
#include	"LKmain.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"rkn_fun.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"mdl_def.h"
#include	"LCM.h"
#include	"cre_ctrl.h"

#define	GET_YEAR(d)		((d)/10000)
#define	GET_MONTH(d)	(((d)%10000)/100)
#define	GET_DAY(d)		((d)%100)
#define	GET_HM_WKNO(n,d)	(n&1)?(d%10):((d/1000)%10)
#define	GET_SPY_WKNO(d)		((d%100)/10)
#define	GET_SPY_YOBI(d)		(d%10)

static	const	ushort	LOCKPRN_MENU[][2] = {
	/* x, len */
		4, 16,
		4, 16,
		4, 12
	};
#define	dsp_menu(y, m, s, n, r)	grachr((ushort)(n+y), m[n][0], m[n][1], (ushort)r, COLOR_BLACK, LCD_BLINK_OFF, &s[n][(m[n][0])])
#define	dsp_prnmenu(y, no, rv)		dsp_menu(y, LOCKPRN_MENU, UMSTR6, no, rv)

/*--------------------------------------------------------------------------*/
/*   static function prototype                                              */
/*--------------------------------------------------------------------------*/
/* ���� */
static void	area_posi_dsp(short top, short no, char area, ulong posi, ushort rev);
/*  5.���b�N���u�J�� */
static char	lockctrl_set(char area, short kind);
static void	lockctrl_set_sts_dsp(short top, short no);
static void	lockctrl_set_all_dsp(short top, short max, short rev_no, short *idx);
/*  6.�Ԏ���� */
static char	locksts_set(char area, short kind);
static char	locksts_set_flpdata(short index);
static void	locksts_set_flpdata_dsp(char pos, short data, ushort rev);
static char locksts_set_flpdata_check(char pos, short data);
static void	locksts_set_all_dsp(short top, short max, short rev_no, short *idx);
/*  7.�◯�ԏ�� */
static char	staysts_search(short days);
short staysts_check(flp_com *p, short days);
static void	staysts_all_dsp(short top, short max, short rev_no, short *idx);
/*  8.���ԑ䐔 */
/*  9.���ԃR���g���[�� */
/* 10.�̎��؍Ĕ��s */
static void	receipt_all_dsp(ushort top, ushort max, ushort rev_no, ushort TotalCount);
/* 11.��t���Ĕ��s */
static char	lockuke_set(char area, short kind);
static void	lockuke_set_sts_dsp(short top, short no, flp_com *pflp);
static void	lockuke_set_all_dsp(short top, short max, short rev_no, short *idx);
/* 13.�c�x�Ɛؑ� */
/* 14.�V���b�^�[�J�� */
/* 15.���f�[�^�m�F */
/* 16.����L���^���� */
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//static char	passinv_set(char block, char mode);
//static char	passinv_allset( char mode, char block );
//static char	passinv_print(char block, char mode);
//static void	passinv_print_dsp(char pos, ushort rev);
//static void	passinv_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind);
///* 17.������Ɂ^�o�� */
//
//static char	passent_set(char block, char mode);
//
//ushort	UsMnt_PassEnter_BeforeValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID );
//ushort	UsMnt_PassEnter_AfterValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID );
//
//static char	passent_allset( char mode, char block );
//static char	passent_print(char block, char mode);
//static void	passent_print_dsp(char pos, ushort rev);
//static void	passent_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind);
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
/* 18.������`�F�b�N */
/* 19.�T�[�r�X�^�C�� */
/* 20.���b�N���u�J�^�C�}�[ */
/* 21.���ʓ��^���ʊ��� */
static void splday_check(void);
static char	splday_day(void);
static char	splday_period(void);
static char	splday_monday(void);
static void	splday_dsp(char pos, ushort rev);
static void	splday_day_dsp(char top, char no, char pos, long data, ushort rev);
static void	splday_period_dsp(char no, char pos, long data, ushort rev);
static void	splday_monday_dsp(char top, char no, char pos, long data, ushort rev);
static char	splday_week(void);
static void	splday_week_dsp(char top, char no, char pos, long data, ushort rev);
/* 22.�W���p�X���[�h */
/* 23.�o�b�N���C�g�_�����@ */
static void	backlgt_dsp(long data, ushort rev);
static void	backlgt_time_dsp(char chng, long time, ushort rev);
/* 25.�L�[���ʒ��� */
static void	keyvol_dsp(char pos);
/* 26.������ */
static void	ticval_kind_dsp(long kind, ushort rev);
static void	ticval_data_dsp(long kind, long data, ushort rev);
static void	ticval_date_dsp(long d1, long d2, long d3, long d4, char rev_pos);
static uchar ticval_date_chk(long date );
/* 27.�c�ƊJ�n���� */
static void	bsytime_dsp(char pos, long time, ushort rev);
/* 28.�����ݒ� */
unsigned short	UsMnt_mnylim(void);
static void	mnylim_dsp(char type, char no, char kind, char rev_pos, char top_pos);
unsigned short	UsMnt_mnytic(void);
static void	mnytic_data_dsp(char kind, long role, long data, ushort rev);
static void	mnytic_chng_dsp(char kind, long chng, ushort rev);
unsigned short	UsMnt_mnyshp(void);
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// static void	mnyshp_data_dsp(char top, char no, long role, long data, ushort rev);
// static void	mnyshp_chng_dsp(char top, char no, long chng, ushort rev);
static void	mnyshp_data_dsp(short top, short no, long role, long data, ushort rev);
static void	mnyshp_chng_dsp(short top, short no, long chng, ushort rev);
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
unsigned short	UsMnt_mnytax(void);
// MH810105(S) MH364301 �C���{�C�X�Ή�
//�C���{�C�X�p�ɍ�蒼���Ă���̂ŋ��v���g�͍폜
//static void	mnytax_dsp(long tax, ushort rev);
// MH810105(E) MH364301 �C���{�C�X�Ή�
unsigned short	UsMnt_mnykid(void);
static void	mnykid_role_dsp(char top, char kind, long data, ushort rev);
static void	mnykid_time_dsp(char top, char kind, long data, ushort rev);
unsigned short	UsMnt_mnycha(void);
static void	mnycha_time_dsp(char mode, long time, ushort rev);
static void	mnycha_money_dsp(char mode, long money, ushort rev);
unsigned short	UsMnt_mnycha_dec(void);
static void	mnycha_time_dsp_dec(long time, ushort rev);
static void	mnycha_money_dsp_dec(long money, ushort rev);
static void	mnycha_coefficient_dsp_dec(long coefficient, ushort rev);
unsigned short	UsMnt_mnytweek(void);
static void	mnytweek_dsp(char no);
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//static ushort	UsMnt_CreConect( void );
//static ushort	UsMnt_CreUnSend( void );
//static ushort	UsMnt_CreSaleNG( void );
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
static void	ftotal_dsp(char pos, ushort rev);
static ushort	ftotal_print(char kind);
static ushort	UsMnt_VoiceGuideTimeAllDay(char *volume, ushort ptn);
static char 	Calc_tmset_sc(ushort tNum, struct GUIDE_TIME * guide);
static ushort 	UsMnt_VoiceGuideTimeExtra(char *volume);
static char		get_volume(char);

/* ��������Z���~�f�[�^ */
static t_TKI_CYUSI teiki_cyusi;	// ��������~�f�[�^�ҏW�p���[�N
static void pstop_num_dsp(short numerator, short denominator);
static void pstop_data_dsp(struct TKI_CYUSI *data, long type);
static short pstop_del_dsp(short index, short mode, struct TKI_CYUSI* work_data);
static void pstop_show_data(short index, t_TKI_CYUSI *cyusi, struct TKI_CYUSI *work_data, char fk_page);
static void	time_dsp(ushort low, ushort col, ushort hour, ushort min, ushort rev);
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//static void	time_dsp2(ushort low, ushort col, ushort hour, ushort min, ushort rev);
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
static void	date_dsp(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev);
static short pstop_calcel_dsp(void);

#if	UPDATE_A_PASS
static	unsigned short	UsMnt_LabelPaperSet(void);
static	unsigned short	UsMnt_LabelPaySet(void);
static	unsigned short	UsMnt_LabelTest(void);
#endif	// UPDATE_A_PASS

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//static void		pextim_show_data(short index, PASS_EXTBL *work_data, char fk_page);
//static void		pextim_num_dsp(ushort numerator, ushort denominator);
//static void		pextim_data_dsp(PASS_EXTBL *data);
//static short	pextim_del_dsp(short index, short mode, PASS_EXTBL *work_data);
//static short	pextim_calcel_dsp(void);
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/* ���b�N���u���C���f�b�N�X�i�[�p���[�N�̈� */
static short	lock_idx[LOCK_MAX];

/* ���b�N���u���(�J�E����L���p) */
static char		lock_sts[LOCK_MAX];

/* ��������Z���~�f�[�^ */
static t_TKI_CYUSI teiki_cyusi;	// ��������~�f�[�^�ҏW�p���[�N
static	PARK_NUM_CTRL	ParkWkDt;							// ���ԑ䐔�Ǘ��ް�01�`03

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//static void cre_unsend_data_dsp(struct DATA_BK *data);	/* �N���W�b�g�����M����˗��f�[�^ �f�[�^�\�� */
//static void cre_saleng_show_data(short index);
//static void cre_saleng_num_dsp(unsigned char numerator, char denominator);
//static void cre_saleng_data_dsp(short index);
//static void	date_dsp3(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev);
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
extern	const	ushort	POS_DATA1_0[6][4];
extern	ushort	LogPtrGet( ushort cnt, ushort wp, ushort max, uchar req );
extern	void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
extern	void	LogCntDsp2( ushort LogCnt1, ushort LogCnt2, ushort type);
extern	void	LogDateDsp( ushort *Date, uchar pos );
extern	uchar	LogDateChk( uchar pos, short data );
extern	uchar	LogDataClr( ushort LogSyu );
extern	uchar	LogDatePosUp1( uchar *pos, uchar req );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//static	ushort	UsMnt_Edy_PayArmPrint( void );		// �A���[��������
//static	ushort	UsMnt_Edy_CentCon( void );			// �Z���^�[�ʐM�J�n
//static	ushort	UsMnt_Edy_ShimePrint( void );		// �d�������ߋL�^���
//#if (4 == AUTO_PAYMENT_PROGRAM)						// �����p��EM�ֺ���ޑ��M�idebug�����j
//static	ushort	UsMnt_Edy_Test( void );				// 
//#endif
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

#define USM_MAKE_MENU 10												/* �쐬�p���j���[�e�[�u���ő匏�� */
static	unsigned char	UsMnt_DiditalCasheMENU_STR[USM_MAKE_MENU][31];	/* �쐬�p���j���[�e�[�u��(����) */
static	unsigned short	UsMnt_DiditalCasheMENU_CTL[USM_MAKE_MENU][4];	/* �쐬�p���j���[�e�[�u��(����) */
/* ���j���[�e�[�u���쐬�֐� */
static	unsigned char	UsMnt_DiditalCasheMenuMake31(const CeMenuPara *prm_tbl, const unsigned char str_tbl[][31], const unsigned short ctr_tbl[][4], const unsigned char tbl_cnt);
unsigned short UsMnt_DiditalCasheSyuukei(ushort);
unsigned short UsMnt_DiditalCasheUseLog(ushort);

extern	uchar	SysMnt_Work[];	/* 32KB */
extern	Receipt_data	PayLog_Work;	// �ʐ��Z���O��o���p�G���A

static ushort UsMnt_NgLog_GetLogCnt(ushort Sdate, ushort Edate);
ushort	UsMnt_IoLog(ushort no);
static char	lock_io_set(char area, short kind);
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//static void	lockio_set_all_dsp(short top, short max, short rev_no, short *idx);
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern	void	LogDateGet2( date_time_rec *Old, date_time_rec *New, ushort *Date );
extern	void	LogDateDsp3( ushort *Date, uchar pos );
extern	uchar	LogDateChk2( uchar pos, short data );
extern const	ushort	POS_DATA31[10][4];
extern	uchar	LogDatePosUp3( uchar *pos, uchar req );
static 	t_ValidWriteData at_invwork;
static	char	AteVaild_ConfirmDelete(ushort attendNo);
const	uchar*	car_title_table[3][4] = {
		{ UMSTR1[26], UMSTR1[28], UMSTR1[39],UMSTR1[41],},	// ���ԁi�����j
		{ UMSTR1[26], UMSTR1[28], UMSTR1[39],UMSTR1[41],},	// ���ԁiIF�j
		{ UMSTR1[27], UMSTR1[29], UMSTR1[40],UMSTR1[42],},	// ����
	};

static void		Carfail_all_dsp(short, short, short, short *);
static ushort	UsMnt_CarFailSet( ushort );
static void		LockInfoDataGet_for_Failer( void );
static char		LockInfoDataSet_for_Failer( char ret_value );
static ushort	LockInfoFailer_ResetMessage( void );
static uchar	LockInfoDataSet_sub( ushort );
#define MAX_VOICE_GUIDE	3
short	GET_TIME_IDX(char n, char b, char m);
short	GET_MONEY_IDX(char n, char k, char b, char m);
char 	mnycha_sub( char set, short seg, char no, char kind, char band, long data );
short 	GET_LIM_IDX(char n, char k);
char 	GET_LIM_TYP( char kind, char* max_set, ushort* param_pos);
void 	mnylim_type_disp( char type , char top_pos);
#if (5 == AUTO_PAYMENT_PROGRAM)
unsigned short UsMnt_mc10print(void);
#endif
ushort	UsColorLCDLumine(void);
static void	Lumine_dsp(long , ushort );
static void	Lumine_time_dsp(char , long , ushort );
static ushort	UsColorLCDLumine_change(char);
#define		STAYSTS_AL_DISP_NO_REVERSE		(short)-1

extern	uchar	LCM_GetFlapMakerParam(uchar type, t_LockMaker* pMakerParam);

static unsigned short	UsMnt_ParkingCertificateSet(void);
void	UsMnt_datecheck( uchar );
static	void	tickvalid_check( void );

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
extern	const	ushort	POS_DATA1[6][4];
// �g���@�\/���σ��[�_����
extern	const	unsigned short	USM_EXTEC_TBL[][4];
static unsigned short UsMnt_ECReaderMnu( void );
static unsigned short UsMnt_EcReaderAlarmLog( void );
// �����ē�����/���[�_�ւ̃R�}���h���M�֐�
static char 	get_volume_EC(char num);
static char	 	VoiceGuide_VolumeChange( void );
static ushort 	ECVoiceGuide_VolumeTest( uchar vol );
static ushort 	ECVoiceGuide_SelectTarget( char *vol_actuary, char *vol_reader, ushort ptn, ushort swit );
static ushort	UsMnt_ECVoiceGuideTimeAllDay(char *volume, ushort ptn, uchar target);
static ushort	UsMnt_ECVoiceGuideTimeExtra(char *vol_actuary, char *vol_reader);
static ushort	ECVoiceGuideTimeExtra_Resultdisplay(uchar result);
#define ADJUS_MAX_VOLUME	15
#define ADJUS_MIN_VOLUME	0
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	const	unsigned short	USM_EXTEJ_TBL[][4];
static unsigned short UsMnt_EJournalMnu( void );
static ushort	UsMnt_EJournalSDInfo( void );
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
static int	get_next_curpos(ushort msg, int pos);
static int	Date_Check( short data, int pos );
static void	mnytax_dsp(ushort val, int pos, ushort rev);
static void	mnytax_dspinit_dsp(ushort* val);
static void	mnytax_editinit_dsp(ushort* val);
static void	before_tax_dsp(ushort tax, ushort rev);
static void	after_tax_dsp(ushort tax, ushort rev);
static void	ChangeBaseDate_dsp(ushort y, ushort m, ushort d);
static void	ChangeBaseDate_edit(uchar kind, ushort val, ushort rev);
// MH810105(E) MH364301 �C���{�C�X�Ή�

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���Ұ��ϊ��i�ėp�j																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_get(par1,par2,par3,par4,par5,par6)													   |*/
/*| PARAMETER	: par1	: ���Ұ���� 0=����/1=�ŗL															   |*/
/*|				: par2	: ���Ұ������No.																	   |*/
/*| 			: par3	: ���ڽ																				   |*/
/*| 			: par4	: ����(�����擾���邩)																   |*/
/*| 			: par5	: �ʒu(�����ڂ���擾���邩 �ʒu��654321�Ƃ���)										   |*/
/*| 			: par6	: �K�p����l(10�i��6���܂�)															   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: Y.Ise(AMANO)																				   |*/
/*| Date		: 2009-09-10																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long prm_set(char kin, short ses, short adr, char len, char pos, long dat)		/*								*/
{																				/*								*/
	long	prm_wk1, prm_wk2;													/* ���Ұ��ϊ�ܰ�				*/
	long	result	= -1;														/* �߂�l�p�ϐ�					*/
	long*	WPrmSS;																/* �l�𔽉f������p�����[�^		*/
	const	long arr[] = {0L, 1L, 10L, 100L, 1000L, 10000L, 100000L, 1000000L};	/* ���Ǘ��p�e�[�u��				*/
																				/*								*/
	if((pos > 0) && (len > 0) && ((len + pos) <= 7))							/* �w�茅��������				*/
	{																			/*								*/
		if( kin == PEC_PRM )													/* �ŗL���Ұ�					*/
		{																		/*								*/
			WPrmSS = &PPrmSS[ses][adr];											/* �ŗL���Ұ�					*/
		}																		/*								*/
		else																	/* ��L�ȊO(�������Ұ�)			*/
		{																		/*								*/
			WPrmSS = &CPrmSS[ses][adr];											/* �������Ұ�					*/
		}																		/*								*/
		prm_wk2 = (*WPrmSS) % arr[pos];											/* �ڕW����艺�̌����ꎞ�ޔ�	*/
		prm_wk1 = (*WPrmSS) / arr[pos + len];									/* �ڕW�̌����ŉ��ʌ��ֈړ�		*/
		prm_wk1 = (prm_wk1 * arr[len + 1]) + dat;								/* �ڕW���̒l�𔽉f				*/
		prm_wk1 = (prm_wk1 * arr[pos]) + prm_wk2;								/* �ڕW���ȉ��̒l�����ɖ߂�		*/
		*WPrmSS = prm_wk1;														/* �K�p�����l���p�����[�^�֔��f	*/
		result = *WPrmSS;														/* ���f��̒l��߂�l�p�ϐ���	*/
	}																			/*								*/
	return( result );															/* �ݒ蔽�f��̒l��Ԃ�			*/
}																				/*								*/

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���� ���I�����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_AreaSelect( char type )                           |*/
/*| PARAMETER    : char type : MNT_FLCTL:���b�N���u�J��                    |*/
/*|                          : MNT_FLSTS:�Ԏ����                          |*/
/*|                          : MNT_UKERP:��t���Ĕ��s                      |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_AreaSelect(short kind)
{
	ushort	msg;
	short	count;
	char	area[27];		/* A-Z(1-26) */
	char	max;
	char	top;
	int		no;
	short	i, j;
	uchar	ch;
	uchar	str[2];
	char	ret;
	char	dsp;
	char	(*func_set)(char area, short kind);		/* �ݒ��ʊ֐��|�C���^ */
	const uchar	*title;

	/* �ݒ��ʊ֐��̌��� */
	switch (kind) {
	case MNT_FLCTL:
		/* ���b�N���u�J�� */
		func_set = lockctrl_set;

		switch( gCurSrtPara ){
			case	MNT_FLAPUD:		// �t���b�v�㏸���~�i�ʁj
				title = UMSTR1[24];
				break;
			case	MNT_FLAPUD_ALL:	// �t���b�v�㏸���~�i�S�āj
				title = UMSTR1[25];
				break;
			case	MNT_BIKLCK:		// ���փ��b�N�J�i�ʁj
				title = UMSTR1[30];
				break;
			case	MNT_BIKLCK_ALL:	// ���փ��b�N�J�i�S�āj
				title = UMSTR1[31];
				break;
			default:				// ���̑��i�ʏ킠�肦�Ȃ����O�ׁ̈j
				title = UMSTR1[24];
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}

		break;
	case MNT_FLSTS:
		/* �Ԏ���� */
		func_set = locksts_set;

		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][0];
									/* "���Ԏ����i���ԁj���@�@�@�@�@" */
									/* "���Ԏ����i���ցj���@�@�@�@�@" */
		break;
	case MNT_UKERP:
		/* ��t���Ĕ��s */
		func_set = lockuke_set;
		title = UMSTR1[7];			/* "����t���Ĕ��s���@�@�@�@�@�@�@" */
		break;
	case MNT_IOLOG:
		/* ���o�� */
		func_set = lock_io_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][3];
									/* "�����o�ɏ��v�����g(����)���@" */
									/* "�����o�ɏ��v�����g(����)���@" */
		break;
	case MNT_CARFAIL:
		/* �Ԏ��̏� */
		func_set = locksts_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][2];
		break;
	default:	/* case BKRS_FLSTS: */
		/* �Ԏ���񒲐� */
		func_set = locksts_set;
		title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][1];
									/* "���Ԏ���񒲐�(����)���@�@�@�@" */
									/* "���Ԏ���񒲐�(����)���@�@�@�@" */
		break;
	}

	/* ���b�N��񂩂������������ */
	memset(area, 0, sizeof(area));
	count = 0;
	for (i = 0, j = 0; i < LOCK_MAX; i++) {
		WACDOG;										// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			area[LockInfo[i].area] = 1;
			count++;
		}
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);

	if( kind != MNT_CARFAIL ){
	/* ���b�N���u���ڑ��� */
		if (count == 0) {
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[21] );			/* "�@�ڑ�����ۯ����u�͂���܂��� " */
			Fun_Dsp( FUNMSG[8] );						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
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
					default:
						break;
				}
			}
		}
	}

	/* ���b�N���u�ڑ����[���g�p�Ȃ��A���͂`�̂� */
	if(CPrmSS[S_TYP][81] < 2){
		ret = func_set(CPrmSS[S_TYP][81]? 1 : 0, kind);
		if( kind == MNT_CARFAIL ){
			ret = LockInfoDataSet_for_Failer( ret );
		}
		if (ret == 1) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (ret == 2) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		return MOD_EXT;
	}

	/* ���b�N���u�ڑ����[���g�p���� */
	for (i = 1, j = 0; i < 27; i++) {
		if (area[i] == 1) {
			area[j++] = (char)i;
		}
	}
	max = (char)(j-1);

	top = no = 0;
	dsp = 1;
	for( ; ; ){
		if (dsp) {
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );
			for( i = 0; i < 5; i++ ){
				if (top+i > max) {
					break;
				}
				ch = (uchar)( 'A' + area[top+i] - 1);
				as1chg(&ch, str, 1);
				grachr ((ushort)(i+2), 4, 2, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, str);			//alphabet
				grachr ((ushort)(i+2), 6, 4, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[0]);	//���
			}
			Fun_Dsp( FUNMSG[25] );							/* // [25]	"�@���@�@���@�@�@�@ �Ǐo  �I�� " */
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return( MOD_CHG );
		case KEY_TEN_F5:
			BUZPI();
			return( MOD_EXT );
		case KEY_TEN_F1:				/* F1:�� */
		case KEY_TEN_F2:				/* F2:�� */
			BUZPI();
			if (msg == KEY_TEN_F1) {
				no--;
				if (no < 0) {
					no = max;
					top = (char)(max/5*5);
				}
				else if (no < top) {
					top -= 5;
				}
			}
			else {
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			/* �f�[�^�\�� */
			for( i = 0; i < 5; i++ ){
				if (top+i > max) {
					for ( ; i < 5; i++)
						grachr((ushort)(i+2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
					break;
				}
				ch = (uchar)( 'A' + area[top+i] - 1);
				as1chg(&ch, str, 1);
				grachr ((ushort)(i+2), 4, 2, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, str);			//alphabet
				grachr ((ushort)(i+2), 6, 4, (top+i==no) ? 1 : 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[0]);	//���
			}
			break;
		case KEY_TEN_F4:
			BUZPI();
			ret = func_set(area[no], kind);
			if( kind == MNT_CARFAIL ){
				ret = LockInfoDataSet_for_Failer( ret );
			}
			if (ret == 1) {
				return MOD_CHG;
			}
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʁ|�Ԏ��ԍ��\��                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : area_posi_dsp( top, no, posi, index, rev )              |*/
/*| PARAMETER    : short  top   : �擪�ԍ�                                 |*/
/*|              : short  no    : �\���ԍ�                                 |*/
/*|              : char   area  : ���                                     |*/
/*|              : ulong  posi  : �Ԏ��ԍ�                                 |*/
/*|              : ushort rev   : 0:���] 1:���]                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	area_posi_dsp(short top, short no, char area, ulong posi, ushort rev)
{
	uchar		ch;
	uchar		str[2];

	/* ��悠��̏ꍇ */
	if (CPrmSS[S_TYP][81] > 0 && area > 0) {
		ch = (uchar)('A' + area - 1);
		as1chg(&ch, str, 1);
		grachr((ushort)(2+no-top), 3, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, str);					/* ��� */
		opedsp((ushort)(2+no-top), 5, (ushort)posi, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
	}
	/* ���Ȃ�(����A�̂�)�̏ꍇ */
	else {
		if( posi == 9999L ){										/* ������X�V���Z? */
			grachr((ushort)(2+no-top), 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[43]);		/* "�X�V���Z"�\�� */
		}else{
			opedsp((ushort)(2+no-top), 4, (ushort)posi, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���b�N���u�J�|�ݒ���                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set( area )                                    |*/
/*| PARAMETER    : char area  : ���                                       |*/
/*|              : short kind : �@�\���(���g�p)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lockctrl_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	short	max;
	short	i, j;
	short	flp_no;
	ushort	cmd;
	ulong	ulwork;
	short   start, end;	
	uchar	allActionWatchFlag = 0;							// �S���u����Ď����t���O
	uchar	actionFlag;									// ���쒆����t���O
	uchar	updown;										// �㏸���~�t���O
	ushort	index;										// LockInfo�C���f�b�N�X
	ushort	indexCount;									// LockInfo�C���f�b�N�X�J�E���g��
	ushort	allCmdTimeout;								// �S����R�}���h�^�C���A�E�g����
	t_LockMaker flapMaker;								// ���u���

	ushort	lno;
	uchar	m_mode;			// �ׯ��/ۯ����u�蓮Ӱ�ޏ��
	cmd = 0;

	/* ���b�N���u���(LockInfo[])����area���ł���f�[�^�𒊏o���� */
	
	switch( gCurSrtPara ){
		case	MNT_FLAPUD:		// �t���b�v�㏸���~�i�ʁj
		case	MNT_FLAPUD_ALL:	// �t���b�v�㏸���~�i�S�āj
			ulwork = GetCarInfoParam();
			switch( ulwork&0x06 ){
				case	0x04:
					start = INT_CAR_START_INDEX;
					end   = start + INT_CAR_LOCK_MAX;
					break;
				case	0x06:
					start = CAR_START_INDEX;
					end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
					break;
				case	0x02:
				default:
					start = CAR_START_INDEX;
					end   = start + CAR_LOCK_MAX;
					break;
			}
			break;

		case	MNT_BIKLCK:		// ���փ��b�N�J�i�ʁj
		case	MNT_BIKLCK_ALL:	// ���փ��b�N�J�i�S�āj
			start = BIKE_START_INDEX;
			end   = start + BIKE_LOCK_MAX;
			break;

		default:				// ���̑��i�ʏ킠�肦�Ȃ����O�ׁ̈j
			start = CAR_START_INDEX;
			end   = start + CAR_LOCK_MAX;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}

	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			if (area == 0) {
				lock_idx[j] = i;
				lock_sts[j] = FLAPDT.flp_data[i].nstat.bits.b01 ? 1 : 0;
				j++;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j] = i;
					lock_sts[j] = FLAPDT.flp_data[i].nstat.bits.b01 ? 1 : 0;
					j++;
				}
			}
		}
	}
	max = j-1;
	indexCount = (ushort)j;

	/* �Y���f�[�^�Ȃ� */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "�@�@�@�f�[�^������܂���@�@�@" */
		Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)				
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:�I�� */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	switch( gCurSrtPara ){
		case	MNT_FLAPUD:		// �t���b�v�㏸���~�i�ʁj
			top = no = 0;
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				// "�@�y�Ԏ��ԍ��z�@�@�@�y��ԁz�@"
			lockctrl_set_all_dsp(top, max, no, lock_idx);									// �f�[�^�ꊇ�\��
			Fun_Dsp( FUNMSG2[24] );															// "  �{  �|�^�� ���~  �㏸  �I�� "
			break;

		case	MNT_FLAPUD_ALL:	// �t���b�v�㏸���~�i�S�āj
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );				// "�@�@�S���u�̓�����s���܂��@�@"
			Fun_Dsp( FUNMSG2[28] );															// "             ���~  �㏸  �I�� "
			break;

		case	MNT_BIKLCK:		// ���փ��b�N�J�i�ʁj
			top = no = 0;
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				// "�@�y�Ԏ��ԍ��z�@�@�@�y��ԁz�@"
			lockctrl_set_all_dsp(top, max, no, lock_idx);									// �f�[�^�ꊇ�\��
			Fun_Dsp( FUNMSG[74] );															// "�@�{�@�|�^�ǁ@�J�@�@�@ �I�� "
			break;

		case	MNT_BIKLCK_ALL:	// ���փ��b�N�J�i�S�āj
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );				// "�@�@�S���u�̓�����s���܂��@�@"
			Fun_Dsp( FUNMSG2[29] );															// "              �J    ��   �I�� "
			break;
	}

	input = -1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
		case KEY_MODECHG:
			Lagcan(OPETCBNO, 14);								// 1s�̊Ď��^�C�}���~�߂�
			Lagcan(OPETCBNO, 19);								// �S���u����Ď��^�C�}���~�߂�
			BUZPI();
			return 1;
		case KEY_TEN_F5:
			Lagcan(OPETCBNO, 14);								// 1s�̊Ď��^�C�}���~�߂�
			Lagcan(OPETCBNO, 19);								// �S���u����Ď��^�C�}���~�߂�
			BUZPI();
			return 0;
		case KEY_TEN_F1:				/* F1:�� */
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// �t���b�v�㏸���~�i�S�āj�@���́@���փ��b�N�J�i�S�āj�̏ꍇ�hF1�h������
				break;
			}
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* �f�[�^�ꊇ�\�� */
			lockctrl_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:				/* F2:���^�� */
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// �t���b�v�㏸���~�i�S�āj�@���́@���փ��b�N�J�i�S�āj�̏ꍇ�hF2�h������
				break;
			}
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
						break;
					}
				}
				if (i > max) {
					/* ���͂����Ԏ��ԍ���������Ȃ��ꍇ */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* �f�[�^�ꊇ�\�� */
			lockctrl_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F3:
		case KEY_TEN_F4:
			m_mode = OFF;
			switch( gCurSrtPara ){
				case	MNT_FLAPUD:		// �t���b�v�㏸���~�i�ʁj
				case	MNT_BIKLCK:		// ���փ��b�N�J�i�ʁj
					m_mode = m_mode_chk_psl( (ushort)(lock_idx[no]+1) );	// �ڑ�����Ă��������No.�̎蓮Ӱ�ޏ������
					break;

				case	MNT_FLAPUD_ALL:	// �t���b�v�㏸���~�i�S�āj
					ulwork = GetCarInfoParam();
					switch( ulwork&0x06 ){
						case	0x04:
							m_mode = m_mode_chk_all( LK_KIND_INT_FLAP );	// �����ׯ�ߑ��u�S����ق̎蓮Ӱ�ޏ������
							break;
						case	0x02:
						default:
							m_mode = m_mode_chk_all( LK_KIND_FLAP );		// �ׯ�ߑ��u�S����ق̎蓮Ӱ�ޏ������
							break;
					}
					break;

				case	MNT_BIKLCK_ALL:	// ���փ��b�N�J�i�S�āj
					m_mode = m_mode_chk_all( LK_KIND_LOCK );				// ۯ����u�S����ق̎蓮Ӱ�ޏ������
					break;
			}
			if( m_mode == ON ){
				// �蓮Ӱ�ޒ��̏ꍇ�A����s�Ƃ���
				BUZPIPI();
				break;
			}
			if(allActionWatchFlag) {
				// �S���u����Ď����͑���s�Ƃ���
				BUZPIPI();
				break;
			}

			BUZPI();

			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// �t���b�v�㏸���~�i�S�āj�@���́@���փ��b�N�J�i�S�āj�̏ꍇ
				if( gCurSrtPara == MNT_FLAPUD_ALL ){
					// �t���b�v�㏸���~�i�S�āj
					ulwork = GetCarInfoParam();
					if (msg == KEY_TEN_F3) {
						// �t���b�v���~�i�S�āj
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[33] );			// 
						lno = OPLOG_A_FLAP_DOWN;					// ���엚���F�S�ׯ�߉��~�i����ݽ�j
						switch( ulwork&0x06 ){
							case	0x04:
								cmd = INT_FLAP_A_DOWN_SND_MNT;		// �S�����ׯ�߉��~�v��(����ݽ)
								break;
							case	0x02:
							default:
								cmd = FLAP_A_DOWN_SND_MNT;			// �S�ׯ�߉��~�v��(����ݽ)
								break;
						}
					}
					else {
						// �t���b�v�㏸�i�S�āj
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[34] );			// 
						lno = OPLOG_A_FLAP_UP;						// ���엚���F�S�ׯ�ߏ㏸�i����ݽ�j
						switch( ulwork&0x06 ){
							case	0x04:
								cmd = INT_FLAP_A_UP_SND_MNT;		// �S�����ׯ�ߏ㏸�v��(����ݽ)
								break;
							
							case	0x02:
							default:
								cmd = FLAP_A_UP_SND_MNT;			// �S�ׯ�ߏ㏸�v��(����ݽ)
								break;
						}
					}
				}
				else{
					// ���փ��b�N�J�i�S�āj
					if (msg == KEY_TEN_F3) {
						// ���փ��b�N�J�i�S�āj
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[35] );			// 
						cmd = LOCK_A_OPN_SND_MNT;					// �Sۯ����u�J�v��(����ݽ)
						lno = OPLOG_A_LOCK_OPEN;					// ���엚���F�S����ۯ��J�i����ݽ�j
					}
					else {
						// ���փ��b�N�i�S�āj
						grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[36] );			// 
						cmd = LOCK_A_CLS_SND_MNT;					// �Sۯ����u�v��(����ݽ)
						lno = OPLOG_A_LOCK_CLOSE;					// ���엚���F�S����ۯ��i����ݽ�j
					}
				}

				// �t���b�v�^�X�N�փR�}���h���M�O�Ƀ^�C���A�E�g�Ď��^�C�}���J�n����
				allCmdTimeout = 0;
				if( gCurSrtPara == MNT_FLAPUD_ALL ){								// �t���b�v
					for(index = 0; index < indexCount; ++index) {
						if((FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0){	// �̏Ⴕ�Ă��Ȃ�
							LCM_GetFlapMakerParam(LockInfo[lock_idx[index]].lok_syu, &flapMaker);	// �t���b�v���u���擾
							if(cmd == INT_FLAP_A_DOWN_SND_MNT || cmd == FLAP_A_DOWN_SND_MNT) {	// �S���~
								allCmdTimeout += flapMaker.open_tm;					// ���~����
							}
							else {													// �S�㏸
								allCmdTimeout += flapMaker.clse_tm;					// �㏸����
							}
						}
					}
				}
				else {																// ���b�N���u
					for(index = 0; index < indexCount; ++index) {
						if((FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0){			// �̏Ⴕ�Ă��Ȃ�
							if(cmd == LOCK_A_OPN_SND_MNT) {							// �S�J
								allCmdTimeout += LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].open_tm / 10;	// ���~����(100ms�P��)
								if(LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].open_tm % 10) {
									++allCmdTimeout;
								}
							}
							else {													// �S��
								allCmdTimeout += LockMaker[LockInfo[index].lok_syu - 1].clse_tm / 10;	// �㏸����(100ms�P��)
								if(LockMaker[LockInfo[lock_idx[index]].lok_syu - 1].clse_tm % 10) {
									++allCmdTimeout;
								}
							}
						}
					}
				}
				Lagtim(OPETCBNO, 19, allCmdTimeout * 50);							// �R�}���h�^�C���A�E�g�Ď��^�C�}
				allActionWatchFlag = 1;
				queset( FLPTCBNO, cmd, 0, NULL );					// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				Ope_DisableDoorKnobChime();
				wopelg( lno, 0, 0 );								// ���엚��o�^
				for( i=0; i<LOCK_MAX; i++ )
				{
					WACDOG;											// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					Kakari_Numu[i] = OPECTL.Kakari_Num;
				}

				Lagtim(OPETCBNO, 6, 100);							// ����ް����ϰ�N���i�Q�b�j
				/* �Q�b�ҋ@���� */
				for ( ; ; ) {
					msg = StoF(GetMessage(), 1 );
					if (msg == TIMEOUT6) {
						// ����ް����ϰ��ѱ��
						break;
					}
					if (msg == KEY_MODECHG) {
						// ���[�h�`�F���W
						BUZPI();
						Lagcan(OPETCBNO, 6);
						return 1;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if (msg == LCD_DISCONNECT) {
						Lagcan(OPETCBNO, 6);
						return 2;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				Lagtim(OPETCBNO, 14, 50);											// ��ԊĎ��^�C�}(1s)
				break;
			}
			if (msg == KEY_TEN_F3) {
				cmd = FLAP_DOWN_SND;
				lock_sts[no] = 0;
				ulwork = (ulong)(( LockInfo[lock_idx[no]].area * 10000L ) + LockInfo[lock_idx[no]].posi );	// �����Get
				if(FALSE == LCM_CanFlapCommand(lock_idx[no], 0)) {			// ���~�R�}���h���s�s��?
					// ���݂̏�Ԃ����~�ς݂̏ꍇ�́A���~�R�}���h���󂯕t���Ȃ�
					BUZPIPI();
					input = -1;
					break;
				}
				
				if( lock_idx[no] < INT_FLAP_END_NO ){
					// ����Ώہ��ׯ��
					wopelg( OPLOG_FLAP_DOWN, 0, ulwork );	// ���엚��o�^�i�ׯ�߉��~�j
					Kakari_Numu[lock_idx[no]] = OPECTL.Kakari_Num;		//�W��No�ۑ�
				}
				else{
					// ����Ώہ�ۯ����u
					wopelg( OPLOG_LOCK_OPEN, 0, ulwork );	// ���엚��o�^�iۯ����u�J�j
					Kakari_Numu[lock_idx[no]] = OPECTL.Kakari_Num;		//�W��No�ۑ�
				}
			}
			else {
				cmd = FLAP_UP_SND;
				lock_sts[no] = 1;
				ulwork = (ulong)(( LockInfo[lock_idx[no]].area * 10000L ) + LockInfo[lock_idx[no]].posi );	// �����Get
				if(FALSE == LCM_CanFlapCommand(lock_idx[no], 1)) {			// �㏸�R�}���h���s�s��?
					// ���݂̏�Ԃ��㏸�ς݂̏ꍇ�́A�㏸�R�}���h���󂯕t���Ȃ�
					// ���샍�O�쐬�A��ʕ\���ύX�O�ɏ����𒆎~����
					BUZPIPI();
					input = -1;
					break;
				}
				if( lock_idx[no] < INT_FLAP_END_NO ){
					// ����Ώہ��ׯ��
					wopelg( OPLOG_FLAP_UP, 0, ulwork );		// ���엚��o�^�i�ׯ�ߏ㏸�j
				}
				else{
					// ����Ώہ�ۯ����u
					wopelg( OPLOG_LOCK_CLOSE, 0, ulwork );	// ���엚��o�^�iۯ����u�j
				}
			}
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[14]);		/* "���쒆" */
			/* ���b�N���u����v�� */
			flp_no = lock_idx[no] + 1;
			queset(FLPTCBNO, cmd, sizeof(flp_no), &flp_no);
			Lagtim(OPETCBNO, 6, 100);		/* 2sec timer start */
			/* �Q�b�ҋ@���� */
			for ( ; ; ) {
				msg = StoF(GetMessage(), 1 );
				/* �^�C���A�E�g(2�b)���o */
				if (msg == TIMEOUT6) {
					break;
				}
				/* ���[�h�`�F���W */
				if (msg == KEY_MODECHG) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if ( msg == LCD_DISCONNECT ) {	
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			Flapdt_sub_clear((ushort)(flp_no - 1));			// ���~�ڍ״ر�ر
			lockctrl_set_sts_dsp(top, no);
			input = -1;
			break;
		case KEY_TEN:
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// �t���b�v�㏸���~�i�S�āj�@���́@���փ��b�N�J�i�S�āj�̏ꍇ�hTEN�h������
				break;
			}
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			if( (gCurSrtPara == MNT_FLAPUD_ALL) || (gCurSrtPara == MNT_BIKLCK_ALL) ){
				// �t���b�v�㏸���~�i�S�āj�@���́@���փ��b�N�J�i�S�āj�̏ꍇ�h����h������
				break;
			}
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		case TIMEOUT14:											// 1s���̏�ԊĎ�
			Lagcan(OPETCBNO, 14);
			actionFlag = 0;
			if(gCurSrtPara == MNT_FLAPUD_ALL) {					// �t���b�v
				if(cmd == INT_FLAP_A_DOWN_SND_MNT || cmd == FLAP_A_DOWN_SND_MNT){
					updown = 0;									// ���~
				}
				else {
					updown = 1;									// �㏸
				}
				for (index = 0; index < indexCount; ++index) {
					if (LockInfo[lock_idx[index]].lok_syu != 0 && LockInfo[lock_idx[index]].ryo_syu != 0 &&	// ���u����
						(FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0) {				// �̏Ⴕ�Ă��Ȃ�
						if(updown != FLAPDT.flp_data[lock_idx[index]].nstat.bits.b01) {
							actionFlag = 1;
							break;
						}
					}
				}
			}
			else {												// ���b�N���u
				if(cmd == OPLOG_A_LOCK_OPEN){
					updown = 0;									// ���~
				}
				else {
					updown = 1;									// �㏸
				}
				for (index = 0; index < indexCount; ++index) {
					if (LockInfo[lock_idx[index]].lok_syu != 0 && LockInfo[lock_idx[index]].ryo_syu != 0 &&	// ���u����
						(FLAPDT.flp_data[lock_idx[index]].car_fail & 0x01) == 0) {				// �̏Ⴕ�Ă��Ȃ�
						if(updown != FLAPDT.flp_data[lock_idx[index]].nstat.bits.b01) {
							actionFlag = 1;
							break;
						}
					}
				}
			}
			
			if(actionFlag == 0) {								// �S���u���슮��
				Lagcan(OPETCBNO, 19);
				allActionWatchFlag = 0;
				BUZPI();
				grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );		// "�@�@�S���u�̓�����s���܂��@�@"
			}
			else {												// ���쒆
				Lagtim(OPETCBNO, 14, 50);
			}
			break;
		case TIMEOUT19:											// �S���u����Ď��^�C�}
			Lagcan(OPETCBNO, 14);								// 1s�̊Ď��^�C�}���~�߂�
			allActionWatchFlag = 0;
			BUZPI();
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[32] );		// "�@�@�S���u�̓�����s���܂��@�@"
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���b�N���u�J�|��ԕ\��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set_sts_dsp( top, no, *pflp )                  |*/
/*| PARAMETER    : short   top   : �擪�ԍ�                                |*/
/*|              : short   no    : �\���ԍ�                                |*/
/*|              : flp_com *pflp : ���b�N���                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockctrl_set_sts_dsp(short top, short no)
{
	grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */

	if (gCurSrtPara == MNT_FLAPUD) {
		// �t���b�v
		if (lock_sts[no]) {
			grachr((ushort)(2+no-top), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[34]);		/* "�㏸" */
			grachr((ushort)(2+no-top), 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);		/* "�@" */
		} else {
			grachr((ushort)(2+no-top), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[35]);		/* "���~" */
			grachr((ushort)(2+no-top), 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);		/* "�@" */
		}
	}
	else {
		// ���փ��b�N
		if (lock_sts[no]) {
			grachr((ushort)(2+no-top), 23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[15]);		/* "��" */
		} else {
			grachr((ushort)(2+no-top), 23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[16]);		/* "�J" */
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���b�N���u�J�|�S�f�[�^�\��(5���܂�)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockctrl_set_all_dsp( top, max, rev_no, *idx )          |*/
/*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
/*|              : short max    : �ő�ԍ�                                 |*/
/*|              : short rev_no : ���]�ԍ�                                 |*/
/*|              : short *idx   : ���b�N���u���̔z��ԍ��e�[�u��         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockctrl_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		lockctrl_set_sts_dsp(top, (short)(top+i));
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ����|�ݒ���                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set( area )                                     |*/
/*| PARAMETER    : char area  : ���                                       |*/
/*|              : short kind : �@�\���                                   |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	locksts_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	char	dsp;
	uchar			priend;
	T_FrmPrnStop	FrmPrnStop;
	short	max;
	short	i, j;
	T_FrmPkjouNow	pri_data;
	const uchar	*title;
	const uchar	*fun;
	short   start, end;	
	short	no2;
	short	max2;
	char	dsp2;
	char	loop;
	void	(*disp_set)(short, short, short, short *);		/* �ݒ��ʊ֐��|�C���^ */
	const uchar	*disp;	

	if( kind == MNT_CARFAIL ){
		LockInfoDataGet_for_Failer();
	}

	/* ���b�N���u���(LockInfo[])����area���ł���f�[�^�𒊏o���� */

	if (gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ) {
		i = GetCarInfoParam();
		switch( i&0x06 ){
			case	0x04:
				start = INT_CAR_START_INDEX;
				end   = start + INT_CAR_LOCK_MAX;
				break;
			case	0x06:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
				break;
			case	0x02:
			default:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX;
				break;
		}
	}
	else {
		start = BIKE_START_INDEX;
		end   = start + BIKE_LOCK_MAX;
	}
	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			if (area == 0) {
				lock_idx[j++] = i;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j++] = i;
				}
			}
		}
	}
	max = j-1;

	/* �Y���f�[�^�Ȃ� */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "�@�@�@�f�[�^������܂���@�@�@" */
		Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:�I�� */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	switch( kind ){
		case	MNT_FLSTS:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][0];
									/* "���Ԏ����i���Ԃh�e�j���@�@�@" */
									/* "���Ԏ����i���ԁj���@�@�@�@�@" */
									/* "���Ԏ����i���ցj���@�@�@�@�@" */
			fun = FUNMSG[73];		/* "  �{  �|�^�� �@�@ �����  �I�� " */
			disp_set = locksts_set_all_dsp;
			disp = UMSTR3[46];
			break;
		case	BKRS_FLSTS:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][1];
									/* "���Ԏ���񒲐��i���Ԃh�e�j���@" */
									/* "���Ԏ���񒲐�(����)���@�@�@�@" */
									/* "���Ԏ���񒲐�(����)���@�@�@�@" */
			fun = FUNMSG2[19];		/* "  �{  �|�^�� �ύX �����  �I�� " */
			disp_set = locksts_set_all_dsp;
			disp = UMSTR3[46];
			break;
		case	MNT_CARFAIL:
			title = car_title_table[(gCurSrtPara - MNT_INT_CAR)][2];
									/* "���Ԏ��̏�i���Ԃh�e�j���@�@�@" */
									/* "���Ԏ��̏�(����)���@�@�@�@�@�@" */
									/* "���Ԏ��̏�(����)���@�@�@�@�@�@" */
			fun = FUNMSG[97];		/* "  �{  �|�^�� �@�@ �Ǐo  �I�� " */		
			disp_set = Carfail_all_dsp;
			disp = CARFAIL[0];
			break;
	}
	dsp = 1;
	top = no = 0;
	input = -1;
	for ( ; ; ) {
		if (dsp) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);						/* "���w�w�w�w���@�@�@�@�@�@�@�@�@" */
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, disp );					/* "�@�y�Ԏ��ԍ��z�@�@�@�yXXXX�z�@" */
			disp_set(top, max, no, lock_idx);				/* �f�[�^�ꊇ�\�� */
			Fun_Dsp(fun);									/* "  �{  �|�^�� �w�w �����  �I�� " */
			dsp = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			if( (kind == BKRS_FLSTS)                &&
				(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if(msg == KEY_MODECHG){
//				return 1;
//			}
			if(msg == KEY_MODECHG){
				return 1;			// MOD_CHG
			}else{
				return 2;			// MOD_CUT
			}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_TEN_F5:
			BUZPI();
			if( (kind == BKRS_FLSTS)                &&
				(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return 0;
		case KEY_TEN_F1:			/* F1:�� */
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* �f�[�^�ꊇ�\�� */
			disp_set(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:			/* F2:���^�� */
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input){
						break;
					}
				}
				if (i > max) {
					/* ���͂����Ԏ��ԍ���������Ȃ��ꍇ */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* �f�[�^�ꊇ�\�� */
			disp_set(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F3:		/* F3:�ύX(�Ԏ���񒲐���ʂ̂�) */
			if (kind == BKRS_FLSTS) {
				/* ���Ԓ� */
				BUZPI();
				/* ���Ɏ����ύX��ʕ\�� */
				if (locksts_set_flpdata(lock_idx[no]) == 1) {	/* Ӱ����ݼ�? */
					if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
						(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
						NT_pcars_timer = 0;
					}
					return 1;
				}
// MH810100(S)
				if (locksts_set_flpdata(lock_idx[no]) == 2) {	/* LCD�ؒf�ʒm */
					if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
						(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
						NT_pcars_timer = 0;
					}
					return 2;
				}
// MH810100(E)
				
				dsp = 1;	/* �\�������ɖ߂� */
			}
			break;
		case KEY_TEN_F4:		/* F4:����� */
			BUZPI();
			if( kind == MNT_CARFAIL ){
				/* �̏��ԕύX��ʕ\�� */
				if (UsMnt_CarFailSet((ushort)lock_idx[no]) == MOD_CHG) {	/* Ӱ����ݼ�? */
					return 1;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if (UsMnt_CarFailSet((ushort)lock_idx[no]) == MOD_CUT) {	/*�ؒf�ʒm*/
					return 2;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				dsp = 1;									/* �\�������ɖ߂� */
				break;
			}
			// �v�����g���ڑI����ʂ֑J��
			dsp2 = 1;
			loop = 1;
			no2 = 0;
			max2 = 2;
			while (loop) {
				if (dsp2) {
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title);
					for (i = 0; i <= max2; i++) {
						if (i == no2) {
							// �I������Ă��鍀�ڂ̂ݔ��]�\��
							j = 1;
						} else {
							j = 0;
						}
						dsp_prnmenu(2, i, j);
					}
					Fun_Dsp(FUNMSG[68]);		/* "�@���@�@���@�@�@�@ ���s  �I�� " */
					dsp2 = 0;
				}
				msg = StoF( GetMessage(), 1 );
				switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					loop = 0;
					return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:
					BUZPI();
					loop = 0;
					return 1;
					break;
				case KEY_TEN_F5:				/* F5:�I�� */
					BUZPI();
					loop = 0;	//���݂̉�ʏI��
					dsp = 1;	//�O��ʂ�\��
					break;
				case KEY_TEN_F1:				/* F1:�� */
					BUZPI();
					dsp_prnmenu(2, no2, 0);
					no2--;
					if (no2 < 0) {
						no2 = max2;
					}
					dsp_prnmenu(2, no2, 1);
					break;
				case KEY_TEN_F2:				/* F2:�� */
					BUZPI();
					dsp_prnmenu(2, no2, 0);
					no2++;
					if (no2 > max2) {
						no2 = 0;
					}
					dsp_prnmenu(2, no2, 1);
					break;
				case KEY_TEN_F4:				/* F4:���s */
					if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					RecvBackUp.ReceiveFlg = 0;	// �v������M(�����׸ނ�0�ɂȂ�Ȃ���NTNETۯ������ް��̎�M���o���Ȃ��̂ł������ݸނ�0�ɏo����悤�ɂ��Ă���)
					/* �v�����g���s */
					pri_data.prn_menu = (uchar)no2;		// ���ƭ�(0:���Ԃ̂�/1:��Ԃ̂�/2:�S��)
					pri_data.prn_kind = R_PRI;
					pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
					queset(PRNTCBNO, PREQ_PKJOU_NOW, sizeof(T_FrmPkjouNow), &pri_data);
					Ope_DisableDoorKnobChime();

					/* �v�����g�I����҂����킹�� */
					Fun_Dsp(FUNMSG[82]);			// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
					for ( priend = 0 ; priend == 0 ; ) {
						msg = StoF( GetMessage(), 1 );
						if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
							msg &= (~INNJI_ENDMASK);
						}
						switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case KEY_MODECHG:		// Ӱ����ݼ�
							BUZPI();
							if( (kind == BKRS_FLSTS)                &&
								(prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
								(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
								NT_pcars_timer = 0;
							}
// MH810100(S)
//							return 1;
							if( msg == KEY_MODECHG ){
								return 1;		// MOD_CHG
							}else{
								return 2;		// MOD_CUT
							}
// MH810100(E)
						case PREQ_PKJOU_NOW:	// �󎚏I��
							priend = 1;
							break;
						case KEY_TEN_F3:		// F3���i���~�j
							BUZPI();
							FrmPrnStop.prn_kind = R_PRI;
							queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
							priend = 1;
							break;
						default:
							break;
						}
					}
					loop = 0;	// �v�����g���ڑI����ʂ̃��[�v�𔲂���
					input = -1;
					dsp = 1;
					break;
				default:
					break;
				}
			}
			break;
		case KEY_TEN:
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ����|�S�f�[�^�\��(5���܂�)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
/*|              : short max    : �ő�ԍ�                                 |*/
/*|              : short rev_no : ���]�ԍ�                                 |*/
/*|              : short *idx   : ���b�N���u���̔z��ԍ��e�[�u��         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	locksts_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);

		switch( FLAPDT.flp_data[idx[top+i]].mode ){					// ���u���(Ӱ��)�H

			case FLAP_CTRL_MODE1:		// ���~�ς�(�ԗ��Ȃ�)
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[0]);			// "  ���  " 
				break;

			case FLAP_CTRL_MODE2:		// ���~�ς�(�ԗ����褏㏸��ϰ���Ē�)
			case FLAP_CTRL_MODE3:		// �㏸���쒆
			case FLAP_CTRL_MODE4:		// �㏸�ς�(���Ԓ�)
				if( SvsTimChk( (ushort)(idx[top+i]+1) ) == OK ){	// ���޽��ђ�����
					grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[1]);		// "���޽���" 
				}
				else{
					grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[2]);		// "  ����  " 
				}
				break;

			case FLAP_CTRL_MODE5:		// ���~���쒆
			case FLAP_CTRL_MODE6:		// ���~�ς�(�ԗ�����׸���ϰ���Ē�)
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[3]);			// "�o�ɑ҂�" 
				break;

			default:	// ���̑�
				grachr((ushort)(2+i), 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_4[0]);			// "  ���  " 
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ����|�S�f�[�^�\��(5���܂�)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Carfail_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
/*|              : short max    : �ő�ԍ�                                 |*/
/*|              : short rev_no : ���]�ԍ�                                 |*/
/*|              : short *idx   : ���b�N���u���̔z��ԍ��e�[�u��         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	Carfail_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);

		switch( FLAPDT.flp_data[idx[top+i]].car_fail & 0x01 ){		// ���u���(Ӱ��)�H

			case 0:		// �̏�Ȃ�
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// "�Ȃ�" 
				break;

			case 1:		// �̏ᒆ
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		// "����" 
				break;

			default:	// ���̑�
				grachr((ushort)(2+i), 22, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);			// "�Ȃ�" 
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ���񒲐��|���Ɏ����ݒ���                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata( index )                            |*/
/*| PARAMETER    : short index : ���b�N���u���̃C���f�b�N�X              |*/
/*| RETURN VALUE : 0:MOD_EXT 1:MOD_CHG                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	locksts_set_flpdata(short index)
{
	ushort	msg;
	short	in_d[11];
	short	input;
	char	pos;
	int		next;
	short	y, m, d;
	flp_com	*pflp;

	/* ���엚��o�^(�Ԏ���񒲐�) */
	wopelg(OPLOG_SHASHITUDTSET, 0, 0);

	pflp = &FLAPDT.flp_data[index];

	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[65]);			/* "���Ɏ����F�@�@�F�@�@�@�@�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[66]);			/* "�@�@�@�@�@�@�@�N�@�@���@�@���@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[67]);			/* "��ԁ@�F�@�@�@�@�@�@�@�@�@�@�@" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[68]);			/* "���ԁ@�F�@�@�@�@���b�N���u�F�@" */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[69]);			/* "��t���F�@�@�@�@���~���F�@�@" */
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[70]);			/* "�Ïؔԍ��F�@�@�@�@�@�@�@�@�@�@" */
	Fun_Dsp(FUNMSG2[12]);						/* "�@���@�@���@ �ύX�@�����@�I�� " */

	// ���͉�ʂ̕\���㖳���Ȃ̂ŕ\�����Ȃ�
	if( index < INT_CAR_START_INDEX ){
		grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"IF");			/* "��ԁ@�F�@�@�@�@�@�@�@�@�@�@IF" */	
	}

	opedsp(0, 22, (ushort)LockInfo[index].posi, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	/* �ԍ� */

	in_d[0] = (short)pflp->hour;
	in_d[1] = (short)pflp->minu;
	in_d[2] = (short)pflp->year;
	in_d[3] = (short)pflp->mont;
	in_d[4] = (short)pflp->date;
	in_d[5] = (short)pflp->mode;
	in_d[6] = (short)pflp->nstat.bits.b00;
	in_d[7] = (short)pflp->nstat.bits.b01;
	in_d[8] = (short)pflp->uketuke;
	in_d[9] = (short)pflp->bk_syu;
	in_d[10] = (short)pflp->passwd;
	locksts_set_flpdata_dsp(0, in_d[0], 1);
	locksts_set_flpdata_dsp(1, in_d[1], 0);
	locksts_set_flpdata_dsp(2, in_d[2], 0);
	locksts_set_flpdata_dsp(3, in_d[3], 0);
	locksts_set_flpdata_dsp(4, in_d[4], 0);
	locksts_set_flpdata_dsp(5, in_d[5], 0);
	locksts_set_flpdata_dsp(6, in_d[6], 0);
	locksts_set_flpdata_dsp(7, in_d[7], 0);
	locksts_set_flpdata_dsp(8, in_d[8], 0);
	locksts_set_flpdata_dsp(9, in_d[9], 0);
	locksts_set_flpdata_dsp(10, in_d[10], 0);

	pos = 0;
	input = -1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {	/* Ӱ����ݼ� */
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {	/* Ӱ����ݼ� */	
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		if (msg == KEY_TEN_F5) {	/* �I�� */
			BUZPI();
			return 0;
		}
		/* ���Ɏ����ݒ� */
		if (pos <= 4) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	/* �� */
			case KEY_TEN_F2:	/* �� */
				next = pos;
				/* ���وʒu�����̈ʒu�ɾ�� */
				if (msg == KEY_TEN_F1) {
					if (--next < 0) {
						for (next = 10; next > 7; next--) {
							if (in_d[next]) {	/* "�Ȃ�"�̓X�L�b�v */
								break;
							}
						}
					}
				}
				else {
					if (++next == 5) {
						if (in_d[5] == 1) {	/* "�T�[�r�X�^�C����"�̓X�L�b�v */
							next = 6;
						}
					}
				}
				if (next > 4) {
					/* "��""��"�ɂ����Ɏ����𔲂����ꍇ */
					input = in_d[next];
					in_d[0] = (short)pflp->hour;
					in_d[1] = (short)pflp->minu;
					in_d[2] = (short)pflp->year;
					in_d[3] = (short)pflp->mont;
					in_d[4] = (short)pflp->date;
					locksts_set_flpdata_dsp(0, in_d[0], 0);
					locksts_set_flpdata_dsp(1, in_d[1], 0);
					locksts_set_flpdata_dsp(2, in_d[2], 0);
					locksts_set_flpdata_dsp(3, in_d[3], 0);
					locksts_set_flpdata_dsp(4, in_d[4], 0);
				}
				else {
					if (input != -1) {
						/* ���͒l�͈����� */
						if (locksts_set_flpdata_check(pos, input) != 1) {
							BUZPIPI();
							locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* �ĕ\�� */
							input = -1;
							break;
						}
						in_d[pos] = input;
						input = -1;
					}
					
				}
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);		/* ���]�\���ɖ߂� */
				locksts_set_flpdata_dsp(next, in_d[next], 1);	/* ���]�\�� */
				pos = next;
				break;
			case KEY_TEN_F4:	/* ���� */
				if(input != -1) {
					/* ���͒l�͈����� */
					if (locksts_set_flpdata_check(pos, input) != 1) {
						BUZPIPI();
						locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* �ĕ\�� */
						input = -1;
						break;
					}
				}
				else {
					input = in_d[pos];
				}
				y = (pos==2) ? input : in_d[2];
				m = (pos==3) ? input : in_d[3];
				d = (pos==4) ? input : in_d[4];
				if( !y || !m || !d || chkdate( y, m, d ) != 0 ){ // ���݂�����t������
					BUZPIPI();
					locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* �ĕ\�� */
					input = -1;
					break;
				}
				BUZPI();
				in_d[pos] = input;
				/* �f�[�^�X�V */
				pflp->hour = (uchar)in_d[0];
				pflp->minu = (uchar)in_d[1];
				pflp->year = (ushort)in_d[2];
				pflp->mont = (uchar)in_d[3];
				pflp->date = (uchar)in_d[4];
				/* �\�� */
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);	/* ���]�\�� */
				if (in_d[5] == 1) {	/* "�T�[�r�X�^�C����"�̓X�L�b�v */
					pos = 6;
				} else {
					pos = 5;
				}
				input = (short)pflp->mode;
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* ���]�\�� */
				break;
			case KEY_TEN:		/* �e���L�[ */
				BUZPI();
				if(input == -1) {
					input = (short)(msg - KEY_TEN0);
				}
				else {
					if( pos == 2 ) {
						/* �N���͂ł����4���̐��l�ɂ��� */
						input = (short)(((input * 10) + (msg - KEY_TEN0)) % 10000);
					}
					else{
						/* �N�ȊO�ł����2���̐��l�ɂ��� */
						input = (short)(((input * 10) + (msg - KEY_TEN0)) % 100);
					}
				}
				locksts_set_flpdata_dsp(pos, input, 1);		/* ���͒l��\������i���]�\���j*/
				break;
			case KEY_TEN_CL:	/* ��� */
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* �ĕ\�� */
				input = -1;
				break;
			default:
				break;
			}
		}
		/* ���̑� */
		else {
			switch (msg) {
			case KEY_TEN_F4:	/* ���� */
				in_d[pos] = input;
				if (pos == 5) {
					pflp->mode = (ushort)in_d[pos];
				}
				else if (pos == 6) {
					pflp->nstat.bits.b00 = (ushort)in_d[pos];
				}
				else if (pos == 7) {
					pflp->nstat.bits.b01 = (ushort)in_d[pos];
					pflp->nstat.bits.b02 = pflp->nstat.bits.b01;
					pflp->nstat.bits.b03 = 0;
					pflp->nstat.bits.b04 = 0;
					pflp->nstat.bits.b05 = 0;
					pflp->nstat.bits.b06 = 0;
					pflp->nstat.bits.b07 = 0;
					pflp->nstat.bits.b08 = 0;
					pflp->nstat.bits.b09 = 0;
				}
				else if (pos == 8) {
					pflp->uketuke = (ushort)in_d[pos];
					if (pflp->uketuke == 0) {
						pflp->u_year = 0;
						pflp->u_mont = 0;
						pflp->u_date = 0;
						pflp->u_hour = 0;
						pflp->u_minu = 0;
					}
				}
				else if (pos == 9) {
					pflp->bk_syu = (ushort)in_d[pos];
					Flapdt_sub_clear((ushort)index);			// ���~�ڍ״ر�ر
				}
				else {
					pflp->passwd = (ushort)in_d[pos];
				}
			case KEY_TEN_F1:	/* �� */
			case KEY_TEN_F2:	/* �� */
				BUZPI();
				locksts_set_flpdata_dsp(pos, in_d[pos], 0);	/* ���]�\�� */
				if (msg == KEY_TEN_F1) {
					pos--;
					if (pos > 7) {
						for ( ; pos > 7; pos--) {
							if (in_d[pos]) {
								break;
							}
						}
					}
					else if (pos == 5) {
						if (in_d[5] == 1) {	/* "�T�[�r�X�^�C����"�̓X�L�b�v */
							pos = 4;
						}
					}
				}
				else {
					pos++;
					if (pos > 7) {
						for ( ; pos <= 10; pos++) {
							if (in_d[pos]) {
								break;
							}
						}
					}
					if (pos > 10) {
						pos = 0;
					}
				}
				if (pos <= 4) {			/* ���Ɏ����ݒ� */
					input = -1;
				}
				else {
					input = in_d[pos];	/* ���̑� */
				}
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* ���]�\�� */
				break;
			case KEY_TEN_F3:	/* �ύX */
				BUZPI();
				if (pos == 5) {
					if (input < 3){
						input = 3;
					} else if (input < 5) {
						input = 5;
					} else {
						input = 0;
					}
				}
				else if (pos == 6 || pos == 7) {
					if (input) {
						input = 0;
					} else {
						input = 1;
					}
				}
				else {		/* if (pos == 8 || pos == 9 || pos == 10) */
					if (input) {
						input = 0;
					} else {
						input = in_d[pos];
					}
				}
				locksts_set_flpdata_dsp(pos, input, 1);	/* ���]�\�� */
				break;
			case KEY_TEN_CL:	/* ��� */
				BUZPI();
				input = in_d[pos];
				locksts_set_flpdata_dsp(pos, in_d[pos], 1);	/* �ĕ\�� */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ���񒲐��|���ڕ\��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata_dsp( pos, data, rev )               |*/
/*| PARAMETER    : char   pos  : ���ڔԍ�                                  |*/
/*|              : short  data : �f�[�^                                    |*/
/*|              : ushort rev  : ���]/���]                                 |*/
/*| RETURN VALUE : nothing                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	locksts_set_flpdata_dsp(char pos, short data, ushort rev)
{
	switch (pos) {
	case 0:		/* �� */
		opedsp(1, 10, (ushort)data, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 1:		/* �� */
		opedsp(1, 16, (ushort)data, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 2:		/* �N */
		opedsp(2, 6,  (ushort)data, 4, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 3:		/* �� */
		opedsp(2, 16, (ushort)data, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 4:		/* �� */
		opedsp(2, 22, (ushort)data, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		break;
	case 5:		/* ��� */
		switch (data) {
		case 0:
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[22]);		/* "�ҋ@" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@�@�@�@" */
			break;
		case 1:
			grachr(3, 8, 14, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[0]);		/* "�T�[�r�X�^�C��" */
			grachr(3, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "��" */
			break;
		case 2:
		case 3:
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);		/* "����" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@�@�@�@" */
			break;
		default:	/* case 4,5 */
			grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);		/* "�o��" */
			grachr(3, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@�@�@�@" */
			break;
		}
		break;
	case 6:		/* ���� */
		if (data) {
			grachr(4, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[24]);		/* "�ԗL" */
		}
		else {
			grachr(4, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[23]);		/* "�Ԗ�" */
		}
		break;
	case 7:		/* ���b�N���u */
		if (data) {
			grachr(4, 28, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[15]);		/* "��" */
		}
		else {
			grachr(4, 28, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[16]);		/* "�J" */
		}
		break;
	case 8:		/* ��t�� */
		if (data) {
			grachr(5, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "����" */
		}
		else {
			grachr(5, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "�Ȃ�" */
		}
		break;
	case 9:		/* ���~��� */
		if (data) {
			grachr(5, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "����" */
		}
		else {
			grachr(5, 26, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "�Ȃ�" */
		}
		break;
	default:	/* �Ïؔԍ� (case 10:) */
		if (data) {
			grachr(6, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[25]);		/* "����" */
		}
		else {
			grachr(6, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);		/* "�Ȃ�" */
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ���񒲐��|���ڔ���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : locksts_set_flpdata_check( pos, data )                  |*/
/*| PARAMETER    : char   pos  : ���ڔԍ�                                  |*/
/*|              : short  data : �f�[�^                                    |*/
/*| RETURN VALUE : 0:NG 1:OK                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char locksts_set_flpdata_check(char pos, short data)
{
	switch( pos ){
	case 0:		//hour
		if( data >= 24 ){
			return 0;
		}
		break;
	case 1:		//minit
		if( data >= 60 ){
			return 0;
		}
		break;
	case 2:		//year
		if(( data < 1990 ) || ( data > 2050 )){
			return 0;
		}
		break;
	case 3:		//month
		if(( data > 12 ) || ( data < 1 )){
			return 0;
		}
		break;
	case 4:		//date
		if(( data > 31 ) || ( data < 1 )){
			return 0;
		}
		break;
	default:
		break;
	}
	return 1;
}


/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�◯�ԏ��                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_StaySts( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura CPrmSS[3][72]�̎Q�ƕ��@���C��        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_StaySts(void)
{
	ushort	msg;
	char	dsp;
	short	days;
	char	ret;
	short	old;
	long	bk_prk;
	
	DP_CP[0] = DP_CP[1] = 0;
	gCurSrtPara = MNT_CAR;
	old = 1;
	days = -1;

	bk_prk = Is_CarMenuMake(CAR_2_MENU);
	do {
		msg = 0;
		dsp = 1;
		
		if (bk_prk) {								// �Ԏ��̐ݒ肪�P��ވȏ゠�邩�H
			if( Ext_Menu_Max > 1 ){					// �������j���[�\���̏ꍇ
			// ����/���֑I�����
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[41]);		/* "���◯�ԏ�񁄁@�@�@�@�@�@�@�@" */
				gCurSrtPara = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
				days = -1;							// �Ԏ��I���ƭ��ɖ߂�ꍇ�ɂ͓��͒l��ر
			}
			else{									// �P���j���[�\���̏ꍇ
				gCurSrtPara = (short)bk_prk;
				bk_prk = 0;
			}
		}else{
			BUZPIPI();								// �Ԏ��ݒ肪�����̂ŃG���[�ł͂���
			gCurSrtPara = MOD_EXT;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if ((gCurSrtPara == (short)MOD_EXT) || (gCurSrtPara == (short)MOD_CHG)){
		if ((gCurSrtPara == (short)MOD_EXT) || (gCurSrtPara == (short)MOD_CHG) || (gCurSrtPara == (short)MOD_CUT)){	
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			msg = gCurSrtPara;
			break;
		}

		while (msg != MOD_EXT) {
		
			if (dsp) {
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[((gCurSrtPara != MNT_BIK) ? 68 : 69)]);
														/* [68]: "���◯�ԏ��i���ԁj���@�@�@�@" */
														/* [69]: "���◯�ԏ��i���ցj���@�@�@�@" */

				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[42]);		/* "�@��������͂��Ă��������B�@�@" */
				grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[20]);			/* "�◯����" */
				grachr(4, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "�F" */
				opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* ���� */
				grachr(4, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[5]);			/* "��" */
				Fun_Dsp(FUNMSG2[15]);					/* "�@�@�@�@�@�@ �����@�@�@�@�I�� " */
				dsp = 0;
			}

			msg = StoF( GetMessage(), 1 );
			switch(KEY_TEN0to9(msg)){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
			case KEY_TEN_F5:				/* F5:�I�� */
				BUZPI();
				msg = MOD_EXT;
				break;
			case KEY_TEN_F3:				/* F3:���� */
				if (days == -1) {
					days = old;
				}
				if (days == 0) {
					BUZPIPI();
					opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* ���� */
					days = -1;
					break;
				}
				BUZPI();
				ret = staysts_search(days);
				if (ret == 1) {
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if (ret == 2) {
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				old = days;
				days = -1;
				dsp = 1;
				break;
			case KEY_TEN:
				BUZPI();
				if (days == -1) {
					days = 0;
				}
				days = (days*10 + msg-KEY_TEN0) % 100;
				opedsp(4, 16, (ushort)days, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* ���� */
				break;
			case KEY_TEN_CL:
				BUZPI();
				opedsp(4, 16, (ushort)old, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* ���� */
				days = -1;
				break;
			default:
				break;
			}
		}
	} while (bk_prk);

	return msg;
}

/*[]----------------------------------------------------------------------[]*/
/*|  �◯�ԏ��|�������ʉ��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_search( days )                                  |*/
/*| PARAMETER    : short days : �◯����                                   |*/
/*| RETURN VALUE : 1 : mode change                                         |*/
/*|              : 0 : F5 key                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/20 ART:ogura ���Ԓ��̎Ԏ��̂ݕ\������悤�ɏC��   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	staysts_search(short days)
{
	ushort	msg;
	short	i, j;
	short	max;
	short	top;
	short	tmp;
	T_FrmTeiRyuuJou	pri_data;
	short   index;
	short   start, end;
	
	/* �Y������ԗ����������� */
	if( gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ){
		tmp = GetCarInfoParam();
		switch( tmp & 0x06 ){
			case 0x04:
				start = INT_CAR_START_INDEX;
				end   = start + INT_CAR_LOCK_MAX;
				break;
			case 0x06:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
				break;
			case 0x02:
			default:
				start = CAR_START_INDEX;
				end   = start + CAR_LOCK_MAX;
				break;
		}
		index = 68;
	}else {
		start = BIKE_START_INDEX;
		end   = start + BIKE_LOCK_MAX;
		index = 69;
	}

	for (i = start, j = 0; i < end; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0 && FLAPDT.flp_data[i].nstat.bits.b00) {
			/* �w������o�߂��Ă��邩�ǂ������� */
			if (staysts_check(&FLAPDT.flp_data[i], days) <= 0) {
				lock_idx[j++] = i;
			}
		}
	}
	max = j-1;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[index]);			/* [68]: "���◯�ԏ��i���ԁj���@�@�@�@" */
												/* [69]: "���◯�ԏ��i���ցj���@�@�@�@" */

	/* �Y���f�[�^�Ȃ� */
	if (max == -1) {
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);		/* "�@�@�@�f�[�^������܂���@�@�@" */
		Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:�I�� */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	/* �Y���ԗ����� */
	if (CPrmSS[S_TYP][81] >= 2) {
		/* lock_idx[]����揇�Ƀ\�[�g���� */
		for (i = 0; i < max; i++) {
			for (j = max; j > i; j--) {
				if (LockInfo[lock_idx[j-1]].area > LockInfo[lock_idx[j]].area) {
					tmp = lock_idx[j];
					lock_idx[j] = lock_idx[j-1];
					lock_idx[j - 1]= tmp;
				}
			}
		}
	}

	top = 0;
	grachr(0, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[14]);			/* "�v" */
	opedsp(0, 22, (ushort)(max+1), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	/* ���v�䐔 */
	grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);			/* "��" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[43]);			/* "�@�y�Ԏ��ԍ��z�@�y���ɓ����z�@" */
	staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);	/* �f�[�^�ꊇ�\�� */
	Fun_Dsp(FUNMSG2[17]);					/* "�@���@�@���@       ����� �I�� " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return 1;
		case KEY_TEN_F5:				/* F5:�I�� */
			BUZPI();
			return 0;
		case KEY_TEN_F4:				/* F4:����� */
			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
				BUZPIPI();
				break;
			}
			BUZPI();
			/* �v�����g���s */
			pri_data.prn_kind = R_PRI;							// �������ʁFڼ��
			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// �@�BNo.
			pri_data.Kakari_no = 0;
			pri_data.Day = days;								// �◯����
			pri_data.Cnt = (short)(max+1);						// �◯�Ԑ�
			pri_data.Data = &lock_idx[0];						// �◯�ԏ���ް��߲���
			queset(PRNTCBNO, PREQ_TEIRYUU_JOU, sizeof(T_FrmTeiRyuuJou), &pri_data);
			Ope_DisableDoorKnobChime();							// �󎚒��̃h�A�m�u�J�`���C���}�~
			/* �v�����g�I����҂����킹�� */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
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
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if ( msg == LCD_DISCONNECT) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			break;
		case KEY_TEN_F1:				/* F1:�� */
			BUZPI();
			top -= 5;
			if(top < 0) {
				top = (short)(max/5*5);
			}
			/* �f�[�^�ꊇ�\�� */
			staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);
			break;
		case KEY_TEN_F2:				/* F2:�� */
			BUZPI();
			top += 5;
			if(top > max) {
				top = 0;
			}
			/* �f�[�^�ꊇ�\�� */
			staysts_all_dsp(top, max, STAYSTS_AL_DISP_NO_REVERSE, lock_idx);
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �◯�ԏ��|�◯�����`�F�b�N                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_check( *p, days )                               |*/
/*| PARAMETER    : flp_com *p   : �擪�ԍ�                                 |*/
/*|              : short   days : �\���ԍ�                                 |*/
/*| RETURN VALUE : ��:�w��������� ��or�O:�w������ȏ�                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura CPrmSS[3][72]�̎Q�ƕ��@���C��        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short staysts_check(flp_com *p, short days)
{
	short	year, mon, day, hour, min;
	short	ret;

	year = p->year;		/* ���ɔN */
	mon = p->mont;		/* ���Ɍ� */
	day = p->date;		/* ���ɓ� */
	hour = p->hour;		/* ���Ɏ� */
	min = p->minu;		/* ���ɕ� */

	/* ���ɓ�������days����̓������擾���� */
	day += days;
	for ( ; ; ) {
		if (day <= medget(year, mon)) {
			break;
		}
		day -= medget(year, mon);
		if (++mon > 12) {
			mon = 1;
			year++;
		}
	}

	ret = year - CLK_REC.year;
	if (ret == 0) {
		ret = mon - CLK_REC.mont;
		if (ret == 0) {
			ret = day - CLK_REC.date;
			/* ��������t�ŎZ�o����ꍇ(CPrmSS[S_TYP][72]==1)�͂����܂� */
			if (prm_get( COM_PRM,S_TYP,72,1,1 ) == 0 && ret == 0) {
				ret = hour - CLK_REC.hour;
				if (ret == 0) {
					ret = min - CLK_REC.minu;
				}
			}
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  �◯�ԏ��[�S�f�[�^�\��(5���܂�)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : staysts_all_dsp( top, max, rev_no, *idx )               |*/
/*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
/*|              : short max    : �ő�ԍ�                                 |*/
/*|              : short rev_no : ���]�ԍ�                                 |*/
/*|              : short *idx   : ���Z�����̔z��ԍ��e�[�u��               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	staysts_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		/* �Ԏ��ԍ��\�� */
		if(rev_no == STAYSTS_AL_DISP_NO_REVERSE) {
			area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, 0);
		}
		else {
			area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		}
		/* ���Z�����\�� */
		opedsp3((ushort)(2+i), 17, (ushort)FLAPDT.flp_data[idx[top+i]].mont, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);									/* "/" */
		opedsp3((ushort)(2+i), 20, (ushort)FLAPDT.flp_data[idx[top+i]].date, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		opedsp3((ushort)(2+i), 23, (ushort)FLAPDT.flp_data[idx[top+i]].hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);									/* ":" */
		opedsp3((ushort)(2+i), 26, (ushort)FLAPDT.flp_data[idx[top+i]].minu, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���ԑ䐔                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkCnt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_ParkCnt(void)
{
	ushort	msg;
	long	*pPRM[3];
	char	changing;
	int		pos;		/* 0:���� 1:���ԉ��� 2:���� */
	long	count;
	char	dsp;
	char	loop;
	char	comm;
	char	redkey = 0;
	char	syskey = 0;
	char	wrikey = 0;

	ushort	disp_max, disp_no;

	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 0 ) {		// ���ԏ󋵕\���@�\�Ȃ�
		BUZPIPI();
		return MOD_EXT;
	}
	
	comm = 0;
	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 1 ) {		// ���ԏ� �e�@�ŊǗ�
		comm = 1;

_RELOAD:	// �u�ēǁv�L�[������jump���x��

// �e�@�ƒʐM���Ȃ��ݒ�Ȃ�N�����Ȃ�
		if (!_is_ntnet_normal()) {
			BUZPIPI();
			return MOD_EXT;
		}
		// �䐔�₢���킹���{
		NTNET_Snd_Data104(NTNET_MANDATA_PARKNUMCTL);	// NT-NET�Ǘ��ް��v���쐬(���ԑ䐔�Ǘ�)
		// ��ʐؑւ�
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "�����ԑ䐔���@�@�@�@�@�@�@�@�@"
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[96]);					// "�@���ԑ䐔�₢���킹���ł��@�@"
		dsp = 0;
		grachr(4, 0, 30, (ushort)dsp, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	// "�@�@���΂炭���҂��������@�@�@"
		Fun_Dsp(FUNMSG[0]);						// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
		Lagtim(OPETCBNO, 6, (ushort)(prm_get(COM_PRM, S_NTN, 31, 2, 5)*50+1));	// ��ϰ6(XXs)�N��(NT-NET��M�Ď�)
		Lagtim(OPETCBNO, 7, 25);				// �����_�ŗp�^�C�}�[�Z�b�g(500msec) */
		loop = 1;
		while (loop) {
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				// ���[�h�`�F���W
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;
			case TIMEOUT6:
				// �ʐM�^�C���A�E�g
				BUZPIPI();
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "�����ԑ䐔���@�@�@�@�@�@�@�@�@"
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[97]);		// "���ԑ䐔�̎擾�Ɏ��s���܂����@"
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[98]);		// "�e�@�Ƃ̒ʐM���m�F���ĉ������@"
				Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				while (1) {
					msg = StoF( GetMessage(), 1 );
					switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case LCD_DISCONNECT:
						return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case KEY_MODECHG:
						// ���[�h�`�F���W
						BUZPI();
						return MOD_CHG;
					case KEY_TEN_F5:	/* �I��(F5) */
						BUZPI();
						return MOD_EXT;
					default:
						break;
					}
				}
				break;
			case TIMEOUT7:
				// �\�����]
				if (dsp) {
					dsp = 0;
				} else {
					dsp = 1;
				}
				grachr(4, 0, 30, (ushort)dsp, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);		// "�@�@���΂炭���҂��������@�@�@"
				Lagtim(OPETCBNO, 7, 25);				// �����_�ŗp�^�C�}�[�Z�b�g(500msec) */
				break;
			case IBK_NTNET_DAT_REC:		// NTNET�f�[�^��M
				// ��M�f�[�^�̒��ԑ䐔�ݒ��v���m�F
				if (NTNET_is234StateValid() != 1) {
					// �s��v�Ȃ�G���[�\��
					BUZPIPI();
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);		// "�����ԑ䐔���@�@�@�@�@�@�@�@�@"
					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[99]);		// "�e�@�Ɩ{�̂Ƃ̐ݒ肪�s��v�ł�"
					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[100]);		// "�ݒ���m�F���ĉ������@�@�@�@�@"
					Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
					while (1) {
						msg = StoF( GetMessage(), 1 );
						switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case LCD_DISCONNECT:
							return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
						case KEY_MODECHG:
							// ���[�h�`�F���W
							BUZPI();
							return MOD_CHG;
						case KEY_TEN_F5:	/* �I��(F5) */
							BUZPI();
							return MOD_EXT;
						default:
							break;
						}
					}
					break;
				} else {
					// ��v�Ȃ璓�ԑ䐔�\����(�����̏���)
					loop = 0;	// �䐔�\�������ֈڍs
				}
				break;
			default:
				break;
			}
		}
	}

	/* �ؑ։�ʐ��̐ݒ� */
	switch (prm_get(COM_PRM,S_SYS,39,1,1)) {
		case 3:  disp_max = 2; disp_no = 1; break;
		case 4:  disp_max = 3; disp_no = 1; break;
		case 0:
		default: disp_max = 0; disp_no = 0; break;	/* �ؑւȂ� */
	}
	
	pPRM[0] = &PPrmSS[S_P02][(4*disp_no)+3];
	pPRM[1] = &PPrmSS[S_P02][(4*disp_no)+4];
	pPRM[2] = &PPrmSS[S_P02][(4*disp_no)+2];

	ParkWkDt.CurNum		= *pPRM[2];				// ���ݒ��ԑ䐔
	ParkWkDt.NoFullNum	= *pPRM[1];				// ���ԉ����䐔
	ParkWkDt.FullNum	= *pPRM[0];				// ���ԑ䐔

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[34]);				/* "�����ԑ䐔���@�@�@�@�@�@�@�@�@" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[16]);					/* "�@�@���ԑ䐔�@�@�@�@�@�@�@�@�@" */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@" */
	opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@" */
	grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "�@�@�@�@�@�@�@�@�@�@�@�@�@��@" */
	grachr(3, 4, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[4]);					/* "�@�@���ԉ����䐔�@�@�@�@�@�@�@" */
	grachr(3, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@" */
	opedsp(3, 18, (ushort)*pPRM[1], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@" */
	grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "�@�@�@�@�@�@�@�@�@�@�@�@�@��@" */
	grachr(4, 4, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[3]);					/* "�@�@���ݒ��ԑ䐔�@�@�@�@�@�@�@" */
	grachr(4, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@" */
	opedsp(4, 18, (ushort)*pPRM[2], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@" */
	grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "�@�@�@�@�@�@�@�@�@�@�@�@�@��@" */
	grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[15]);					/* "�@�@��ԑ䐔�@�@�@�@�@�@�@�@�@" */
	grachr(5, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�@�@�@�@�@�@�@�@�F�@�@�@�@�@�@" */
	count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
	opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@" */
	grachr(5, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[13]);				/* "�@�@�@�@�@�@�@�@�@�@�@�@�@��@" */

	if (comm != 1) {
		// �{�̂ŃJ�E���g�̏ꍇ
		if (disp_max > 0) {
			grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "�m���ԂP�n" */
			Fun_Dsp(FUNMSG2[25]);					/* " �ؑ� �@�@�@ �ύX�@�@�@�@�I�� " */
		} else  {
			Fun_Dsp(FUNMSG2[0]);					/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
		}
	} else {
		// �ʐM�ŃJ�E���g�̏ꍇ
		if (disp_max > 0) {
			grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "�m���ԂP�n" */
			Fun_Dsp(FUNMSG2[35]);					/* " �ؑ� �ēǁ@ �ύX�@�@�@�@�I�� " */
		} else  {
			Fun_Dsp(FUNMSG2[33]);					/* "�@�@�@�ēǁ@ �ύX�@�@�@�@�I�� " */
		}
		redkey = 1;
	}

	changing = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT) {
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
					(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
					NT_pcars_timer = 0;
				}
				return MOD_EXT;
			case KEY_TEN_F3:	/* �ύX(F3) */
				BUZPI();
				changing = 1;
				if (comm == 1) {
					*pPRM[2] = ParkWkDt.CurNum;			// ���ݒ��ԑ䐔
					*pPRM[1] = ParkWkDt.NoFullNum;		// ���ԉ����䐔
					*pPRM[0] = ParkWkDt.FullNum;		// ���ԑ䐔
				}
				opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���ԑ䐔 ���] */
				if (comm != 1) {
					if (prm_get(COM_PRM,S_NTN,38,1,3) == 2) {
						Fun_Dsp(FUNMSG2[37]);		/* "�@���@�@���@ ���с@�����@�I�� " */
						syskey = 1;
						wrikey = 1;
					} else {
						Fun_Dsp(FUNMSG[20]);		/* "�@���@�@���@�@�@�@ ����  �I�� " */
						syskey = 0;
						wrikey = 1;
					}
				} else {
					Fun_Dsp(FUNMSG2[36]);		/* "�@���@�@���@ ���с@�@�@�@�I�� " */
					syskey = 1;
					wrikey = 0;
				}
				count = -1;
				pos = 0;
				break;
			case KEY_TEN_F1:	/* �ؑ�(F1) */
				if (disp_max > 0) {
					BUZPI();
					
					/* ��ʔԍ��ݒ� */
					disp_no = (disp_no == disp_max) ? 1 : (disp_no + 1);
					
					pPRM[0] = &PPrmSS[S_P02][(4*disp_no)+3];
					pPRM[1] = &PPrmSS[S_P02][(4*disp_no)+4];
					pPRM[2] = &PPrmSS[S_P02][(4*disp_no)+2];
					
					ParkWkDt.CurNum	   = *pPRM[2];			// ���ݒ��ԑ䐔
					ParkWkDt.NoFullNum = *pPRM[1];			// ���ԉ����䐔
					ParkWkDt.FullNum   = *pPRM[0];			// ���ԑ䐔

					/* ���l������ */
					grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[(disp_no-1)]);					/* "�m���Ԃw�n" */
					opedsp(2, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ���ԑ䐔     */
					opedsp(3, 18, (ushort)*pPRM[1], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ���ԉ����䐔 */
					opedsp(4, 18, (ushort)*pPRM[2], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ���ݒ��ԑ䐔 */
					count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
					opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* ��ԑ䐔     */
				}
				break;
			case KEY_TEN_F2:	/* �ē�(F2) */
				if (redkey) {
					goto _RELOAD;	// NT-NET�ʐM�����֖߂�
				}
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F3:	/* ����(F3) */
				if (syskey == 1) {
					// �����ێ��f�[�^������
					if (count != -1) {
						*pPRM[pos] = count;
						if (pos != 1) {
							count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
							opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
						}
					}
					// ����f�[�^���M(���ݑ䐔�Ǘ��f�[�^)
					ParkWkDt.CurNum    = *pPRM[2];			// ���ݒ��ԑ䐔
					ParkWkDt.NoFullNum = *pPRM[1];			// ���ԉ����䐔
					ParkWkDt.FullNum   = *pPRM[0];			// ���ԑ䐔
					changing = 0;
				} else {
					break;
				}
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				if (comm != 1) {
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �䐔 ���] */
				}else{
					opedsp((ushort)(2), 18, (ushort)ParkWkDt.FullNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �䐔 ���] */
					opedsp((ushort)(3), 18, (ushort)ParkWkDt.NoFullNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �䐔 ���] */
					opedsp((ushort)(4), 18, (ushort)ParkWkDt.CurNum, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �䐔 ���] */
					count = (ParkWkDt.FullNum > ParkWkDt.CurNum) ? (ParkWkDt.FullNum-ParkWkDt.CurNum) : 0;
					opedsp((ushort)(5), 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �䐔 ���] */
					*pPRM[2] = ParkWkDt.CurNum;			// ���ݒ��ԑ䐔
					*pPRM[1] = ParkWkDt.NoFullNum;		// ���ԉ����䐔
					*pPRM[0] = ParkWkDt.FullNum;		// ���ԑ䐔
				}
				if (comm != 1) {
					// �{�̂ŃJ�E���g�̏ꍇ
					if (disp_max > 0) {
						Fun_Dsp(FUNMSG2[25]);			/* " �ؑ� �@�@�@ �ύX�@�@�@�@�I�� " */
					} else {
						Fun_Dsp(FUNMSG2[0]);			/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
					}
				} else {
					// �ʐM�ŃJ�E���g�̏ꍇ
					if (disp_max > 0) {
						Fun_Dsp(FUNMSG2[35]);			/* " �ؑ� �ēǁ@ �ύX�@�@�@�@�I�� " */
					} else {
						Fun_Dsp(FUNMSG2[33]);			/* "�@�@�@�ēǁ@ �ύX�@�@�@�@�I�� " */
					}
				}
				changing = 0;
				break;
			case KEY_TEN_F4:	/* ����(F4) */
				if (!wrikey) {
					break;
				} else {
				if (count != -1) {
					*pPRM[pos] = count;
					OpelogNo = OPLOG_CHUSHADAISU;		// ���엚��o�^
					if (pos != 1) {
						count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
						opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* ��ԑ䐔 */
					}
				}
				}
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				if (comm != 1) {
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �䐔 ���] */
				}else{
					if (count != -1) {
						*pPRM[pos] = count;
					}
					opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �䐔 ���] */
					if (pos != 1) {
						count = (*pPRM[0] > *pPRM[2]) ? (*pPRM[0]-*pPRM[2]) : 0;
						opedsp(5, 18, (ushort)count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ��ԑ䐔 */
					}
				}
				if (msg == KEY_TEN_F1) {
					if (--pos < 0) {
						pos = 2;
					}
				}
				else {
					if (++pos > 2) {
						pos = 0;
					}
				}
				opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �䐔 ���] */
				count = -1;
				break;
			case KEY_TEN:		/* ����(�e���L�[) */
				BUZPI();
				if (count == -1) {
					count = 0;
				}
				count = (count*10 + msg-KEY_TEN0) % 10000;
				opedsp((ushort)(2+pos), 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �䐔 ���] */
				break;
			case KEY_TEN_CL:	/* ���(�e���L�[) */
				BUZPI();
				opedsp((ushort)(2+pos), 18, (ushort)*pPRM[pos], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �䐔 ���] */
				count = -1;
				break;
			default:
				break;
			}
		}
	}	
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���ԃR���g���[��                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_FullCtrl( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_FullCtrl(void)
{
	ushort	msg;
	long	*pCTRL;
	long	val_tbl[] = {1, 0, 2};
	const uchar	*str_tbl[] = {DAT4_2[18], DAT4_2[13], DAT4_2[17]};
	int		pos;		/* 0:�������� 1:���� 2:������� */

	ushort	disp_max, disp_no;

	if ( prm_get(COM_PRM,S_SYS,39,1,2) == 0 ) {		// ���ԏ󋵕\���@�\�Ȃ�
		BUZPIPI();
		return MOD_EXT;
	}
	
	/* �ؑ։�ʐ��̐ݒ� */
	switch (prm_get(COM_PRM,S_SYS,39,1,1)) {
		case 3:  disp_max = 2; disp_no = 1; break;
		case 4:  disp_max = 3; disp_no = 1; break;
		case 0:
		default: disp_max = 0; disp_no = 0; break;	/* �ؑւȂ� */
	}
	
	pCTRL = &PPrmSS[S_P02][(4*disp_no)+1];
	for (pos = 0; pos < 2; pos++) {
		if (*pCTRL == val_tbl[pos]) {
			break;
		}
	}
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[35] );					/* "�����ԃR���g���[�����@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10] );					/* "�@���݂̏�ԁ@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@" */
	grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );				/* "�@�@�@�@�@�@�@�w�w�w�w�@�@�@�@" */
	grachr( 4, 12,  8, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[0] );	/* "�@�@�@�@�@�@�@�������ԁ@�@�@�@" */
	grachr( 5, 12,  8, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[1] );	/* "�@�@�@�@�@�@�@�@�����@�@�@�@�@" */
	grachr( 6, 12,  8, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[2] );	/* "�@�@�@�@�@�@�@������ԁ@�@�@�@" */

	if (disp_max > 0) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[0]);				/* "�m���ԂP�n" */
		Fun_Dsp( FUNMSG2[27] );							/* "�@���@�@���@ �ؑ�  ����  �I�� " */
	}
	else {
		Fun_Dsp( FUNMSG[20] );							/* "�@���@�@���@�@�@�@ ����  �I�� " */
	}

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_CHG );
		case KEY_TEN_F5:					/* F5:Exit */
			BUZPI();
			if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
				(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
				NT_pcars_timer = 0;
			}
			return( MOD_EXT );
		case KEY_TEN_F3:					/* F3:Switch */
			if (disp_max > 0) {
				BUZPI();

				/* ��ʔԍ��ݒ� */
				disp_no = (disp_no == disp_max) ? 1 : (disp_no + 1);
				grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR4[(disp_no-1)]);				/* "�m���Ԃw�n" */
				pCTRL = &PPrmSS[S_P02][(4*disp_no)+1];

				grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
				for (pos = 0; pos < 2; pos++) {
					if (*pCTRL == val_tbl[pos]) {
						break;
					}
				}
				grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
				grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			}
			break;
		case KEY_TEN_F1:					/* F1: */
		case KEY_TEN_F2:					/* F2: */
			BUZPI();
			grachr((ushort)(4+pos), 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			if (msg == KEY_TEN_F1) {
				if (--pos < 0) {
					pos = 2;
				}
			}
			else {
				if (++pos > 2) {
					pos = 0;
				}
			}
			grachr((ushort)(4+pos), 12,  8, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos]);
			break;
		case KEY_TEN_F4:					/* F4:���� */
			BUZPI();
			*pCTRL = val_tbl[pos];
			OpelogNo = OPLOG_MANSYACONTROL;			// ���엚��o�^
			grachr( 2, 14,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[pos] );
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�̎��؍Ĕ��s                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Receipt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]��ܰ��ر�Ƃ��Ďg�p����B                       [MH544401] |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Receipt(void)
{
#define	RERECEIPT_MAX_COUNT		50
	ushort			msg;
	ushort			max;				/* �ő�ԍ�(����-1) */
	ushort			top;				/* �擪�ԍ� */
	ushort			no;					/* �ԍ�(0�`max) */
	T_FrmReceipt	rec_data;			/* �̎��؈󎚗v����ү�����ް� */
	ushort			LogTotalCount;
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	ushort			jnl_pri_wait_Timer;	// �W���[�i���󎚊����҂��^�C�}�[
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	Receipt_data	*pWkRec = (Receipt_data*)&SysMnt_Work[0];
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

	LogTotalCount = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP );					// �����Z����LOG����get
	if( RERECEIPT_MAX_COUNT < LogTotalCount ){
		max = RERECEIPT_MAX_COUNT;
	}else{
		max = LogTotalCount;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[45]);			/* "���̎��؍Ĕ��s���@�@�@�@�@�@�@" */

	/* �����Ȃ� */
	if (max == 0) {
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[48]);		/* "�@�@ ���Z����������܂��� �@�@" */
		Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
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
				return( MOD_EXT );
			default:
				break;
			}
		}
	}

	/* �������� */
	top = no = 0;
// MH810100(S) Y.Yamauchi 20191227 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[47]);			/* "�@�y�Ԏ��ԍ��z�@�y���Z�����z�@" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[185]);			/* 	"�@�y�Ԕԁz�@�y���Z�����z�@", */
// MH810100(E) Y.Yamauchi 20191227 �Ԕԃ`�P�b�g���X(�����e�i���X)
	receipt_all_dsp(top, max, no, LogTotalCount);		/* �f�[�^�ꊇ�\�� */

	if( ( prm_get(COM_PRM,S_PAY,25,2,1) != 0 )&&( CPrmSS[S_KOU][1] ) ){		// ����X�V����&�������������
		Fun_Dsp(FUNMSG2[23]);						/* "�@���@�@���@���x�� ���s  �I�� " */
	}else{
		Fun_Dsp(FUNMSG2[16]);						/* "�@���@�@���@�@�@�@ ���s  �I�� " */
	}

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
			return( MOD_EXT );
		case KEY_TEN_F4:				/* F4:���s */
// MH810105(S) MH364301 �C���{�C�X�Ή�
//			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
			if(Ope_isPrinterReady() == 0 ||			// ���V�[�g�v�����^���󎚕s�\�ȏ��
				(IS_INVOICE &&
// GG129000(S) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5689�F#6819�d�q�W���[�i���g�p�s�������Ƀ����e�i���X�ŗ̎��؍Ĕ��s����ƃG���[�����Đ�����Ȃ��j
//				 Ope_isJPrinterReady() == 0)){		// �C���{�C�X�ݒ肠��̓W���[�i����Ԃ��`�F�b�N
// GG129000(S) M.Fujikawa 2023/10/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7167
//				(Ope_isJPrinterReady() && (paperchk2() == -1)))) {
				(Ope_isJPrinterReady() == 0 || (paperchk2() == -1)))) {
// GG129000(E) M.Fujikawa 2023/10/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7167
// GG129000(E) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5689�F#6819�d�q�W���[�i���g�p�s�������Ƀ����e�i���X�ŗ̎��؍Ĕ��s����ƃG���[�����Đ�����Ȃ��j
// MH810105(E) MH364301 �C���{�C�X�Ή�
				BUZPIPI();
				break;
			}
			BUZPI();
			/* �̎��؍Ĕ��s */
// GG129002(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
			memset(&rec_data, 0, sizeof(rec_data));
// GG129002(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
			rec_data.prn_kind = R_PRI;									/* �������ʁFڼ�� */
// MH810105(S) MH364301 �C���{�C�X�Ή�
			if (IS_INVOICE) {
				rec_data.prn_kind = J_PRI;									// �W���[�i���󎚂���
			}
// MH810105(E) MH364301 �C���{�C�X�Ή�
			(void)Ope_SaleLog_1DataGet( (ushort)(LogTotalCount - no - 1), PAY_LOG_CMP, LogTotalCount, pWkRec );
			rec_data.prn_data = pWkRec;										/* �̎��؈��ް����߲����� */
			rec_data.kakari_no = OPECTL.Kakari_Num;							// �W��No.
			rec_data.reprint = ON;											// �Ĕ��s�׸޾�āi�Ĕ��s�j
			memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );	// �Ĕ��s�����i���ݓ����j���

			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			Ope_DisableDoorKnobChime();
			OpelogNo = OPLOG_RYOSHUSAIHAKKO;		// ���엚��o�^
			/* �v�����g�I����҂����킹�� */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
			for ( ; ; ) {
				msg = StoF( GetMessage(), 1 );
				/* �v�����g�I�� */
				if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
// MH810105(S) MH364301 �C���{�C�X�Ή�
//					Lagcan(OPETCBNO, 6);
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u��Ԃł���΃����e�i���X�I��
					if (CP_MODECHG) {
// MH810105(S) MH364301 �C���{�C�X�Ή�
						Lagcan(OPETCBNO, 6);
						Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 �C���{�C�X�Ή�
						return MOD_CHG;
					}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
// MH810105(S) MH364301 �C���{�C�X�Ή�
//					break;
					if (IS_INVOICE) {
						// �C���{�C�X�ݒ肠��
						if (OPECTL.Pri_Kind == J_PRI) {
							if (OPECTL.Pri_Result == PRI_NML_END) {
								// �Z���̎��؁i�������Z�Ȃǁj�󎚂ň󎚒��Ƀt�^���J�����ꍇ�A
								// �󎚊������󎚎��s�Ƃ������Œʒm�����ꍇ������
								// ���̂��߁A�󎚊�����M���Wait�^�C�}�𓮍삳����
								// �^�C�}���쒆�Ɉ󎚎��s����M�����ꍇ�͈󎚎��s�Ƃ��Ĉ���
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//								Lagtim(OPETCBNO, 7, 150);	// 3sec timer start
								jnl_pri_wait_Timer = GyouCnt_All_j + GyouCnt_j;	// ���M�ς݈��ް��S�s���擾
								Lagtim(OPETCBNO, 7, (jnl_pri_wait_Timer * 5));	// 1�s/100ms(20ms*5)
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
							}
							else {
								Lagcan(OPETCBNO, 6);
								break;
							}
						}
						else {
							Lagcan(OPETCBNO, 6);
							break;
						}
					}
					else {
						// �C���{�C�X�ݒ�Ȃ�
						Lagcan(OPETCBNO, 6);
						break;
					}
// MH810105(E) MH364301 �C���{�C�X�Ή�
				}
				/* �^�C���A�E�g(10�b)���o */
				if (msg == TIMEOUT6) {
					BUZPIPI();
					break;
				}
// MH810105(S) MH364301 �C���{�C�X�Ή�
				/* �^�C���A�E�g(3�b)���o */
				if(msg == TIMEOUT7){
					// ���V�[�g�󎚉\
					if (Ope_isPrinterReady() &&
						paperchk() != -1 &&
						Ope_isJPrinterReady() &&
						paperchk2() != -1) {
						rec_data.prn_kind = R_PRI;	// �������ʁFڼ��
						queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
						/* �v�����g�I����҂����킹�� */
						Lagtim(OPETCBNO, 6, 500);	// 10sec timer start
					}
					// ���V�[�g�󎚕s��
					else{
						Lagcan(OPETCBNO, 6);
						break;
					}
				}
// MH810105(E) MH364301 �C���{�C�X�Ή�
				/* ���[�h�`�F���W */
				if (msg == KEY_MODECHG) {
					BUZPI();
					Lagcan(OPETCBNO, 6);
// MH810105(S) MH364301 �C���{�C�X�Ή�
					Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 �C���{�C�X�Ή�
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if ( msg == LCD_DISCONNECT ) {	
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
// MH810105(S) MH364301 �C���{�C�X�Ή�
			// for���𔲂��鎞�O�̂���lagcan7�����Ă���
			Lagcan(OPETCBNO, 7);
// MH810105(E) MH364301 �C���{�C�X�Ή�
			break;
		case KEY_TEN_F3:				/* F3:���x�� */
			break;
		case KEY_TEN_F1:				/* F1:�� */
			BUZPI();
			if (no > 0) { 
				no--;
			}else{
				no = max - 1;
				top = (max-1)/5*5;
			}
			if (no < top) {
				top -= 5;
			}
			/* �f�[�^�ꊇ�\�� */
			receipt_all_dsp(top, max, no, LogTotalCount);
			break;
		case KEY_TEN_F2:				/* F2:�� */
			BUZPI();
			if (no < (max-1)) {
				no++;
			}else{
				top = no = 0;
			}
			if (no > top+4) {
				top = no;
			}
			/* �f�[�^�ꊇ�\�� */
			receipt_all_dsp(top, max, no, LogTotalCount);
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �̎��؍Ĕ��s�[�S�f�[�^�\��(5���܂�)                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : receipt_all_dsp( top, max, rev_no, TotalCount )         |*/
/*| PARAMETER    : ushort top        : �擪�ԍ�                            |*/
/*|              : ushort max        : �ő�ԍ�                            |*/
/*|              : ushort rev_no     : ���]�ԍ�                            |*/
/*|              : ushort TotalCount : FLASH���܂߂��S����      [MH544401] |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| SysMnt_Work[]��ܰ��ر�Ƃ��Ďg�p����B                       [MH544401] |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	receipt_all_dsp(ushort top, ushort max, ushort rev_no, ushort TotalCount)
{
	char	i;
	Receipt_data	*p = (Receipt_data*)&SysMnt_Work[0];
	t_OpeLog_FindNextLog	NextLogInfo;
// MH810100(S) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#3976 ���u���Z���ɗ̎��؍Ĕ��s�̕\����������Â炢)
	ushort mod = 0;
// MH810100(E) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#3976 ���u���Z���ɗ̎��؍Ĕ��s�̕\����������Â炢)

	for (i = 0; i < 5; i++) {
		if (top+i >= max) {
			break;
		}
		if(i == 0) {
			(void)Ope_SaleLog_First1DataGet( (ushort)(TotalCount - top - i - 1), PAY_LOG_CMP, TotalCount, p, &NextLogInfo);
		}
		else {
			(void)Ope_SaleLog_Next1DataGet( (ushort)(TotalCount - top - i - 1), (uchar)PAY_LOG_CMP, p, &NextLogInfo);
		}
		/* �Ԏ��ԍ��\�� */
// MH810100(S) Y.Yamauchi 20191227 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		area_posi_dsp((short)top, (short)(top+i), (char)(p->WPlace/10000), (ulong)(p->WPlace%10000), (rev_no==top+i)?1:0);
// MH810100(S) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#3976 ���u���Z���ɗ̎��؍Ĕ��s�̕\����������Â炢)
//		if( rev_no == (top+i) ){
//			grachr((ushort)(2+i), 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber));		/*�Ԕ�*/
//		}else {
//			grachr((ushort)(2+i), 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber));		/*�Ԕ�*/
//		}
		if( rev_no == (top+i) ){
			mod = 1;
		}else{
			mod = 0;
		}
		if( p->PayMode !=4 ){
// MH810100(S) 2020/07/28 �̎��؍Ĕ��s�ŎԔԂ��Ȃ��Ƃ킩��Â炢
//			grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber) );	// �Ԕ�
			if( p->CarNumber[0] != NULL){
				grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)(p->CarNumber) );	// �Ԕ�
			}else{
				grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, RECEIPT_DAT1[1] );	// �Ԕ�
			}
// MH810100(E) 2020/07/28 �̎��؍Ĕ��s�ŎԔԂ��Ȃ��Ƃ킩��Â炢
		}else{
			grachr( (ushort)(2+i), 4, 4, mod, COLOR_BLACK, LCD_BLINK_OFF, RECEIPT_DAT1[0] );				// "���u"
		}
// MH810100(E) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#3976 ���u���Z���ɗ̎��؍Ĕ��s�̕\����������Â炢)
// MH810100(E) Y.Yamauchi 20191227 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		/* ���Z�����\�� */
// MH810100(S) 2020/09/04 #4864 �����e�i���X�̗̎��؍Ĕ��s��ʂŖ����Z�o�ɐ��Z�́y���Z�����z���o�Ɏ����ŕ\������Ă���
//		opedsp3((ushort)(2+i), 17, (ushort)p->TOutTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
//		grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);						/* "/" */
//		opedsp3((ushort)(2+i), 20, (ushort)p->TOutTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
//		opedsp3((ushort)(2+i), 23, (ushort)p->TOutTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
//		grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);						/* ":" */
//		opedsp3((ushort)(2+i), 26, (ushort)p->TOutTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//		if(p->shubetsu == 0){
		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
		if(p->shubetsu == 0 || p->shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
			opedsp3((ushort)(2+i), 17, (ushort)p->TOutTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
			grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);						/* "/" */
			opedsp3((ushort)(2+i), 20, (ushort)p->TOutTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
			opedsp3((ushort)(2+i), 23, (ushort)p->TOutTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
			grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);						/* ":" */
			opedsp3((ushort)(2+i), 26, (ushort)p->TOutTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
		}else{
			opedsp3((ushort)(2+i), 17, (ushort)p->TUnpaidPayTime.Mon, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
			grachr((ushort)(2+i), 19, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							/* "/" */
			opedsp3((ushort)(2+i), 20, (ushort)p->TUnpaidPayTime.Day, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
			opedsp3((ushort)(2+i), 23, (ushort)p->TUnpaidPayTime.Hour, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
			grachr((ushort)(2+i), 25, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);							/* ":" */
			opedsp3((ushort)(2+i), 26, (ushort)p->TUnpaidPayTime.Min, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
		}
// MH810100(E) 2020/09/04 #4864 �����e�i���X�̗̎��؍Ĕ��s��ʂŖ����Z�o�ɐ��Z�́y���Z�����z���o�Ɏ����ŕ\������Ă���
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  �ʐ��ZLOG�����擾													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_SaleLog_TotalCountGet( void )						|*/
/*| PARAMETER    : Kind  = ���O�����J�E���g�w��(bit�Ή�)					|*/
/*|							all"0":�S�ʐ��Z���O������get					|*/
/*|							bit0=1:���Z����(�s���E�����o�ɂ�����)�̌ʐ��Z	|*/
/*|							       ���O������get							|*/
/*|							bit1=1:���Z���~�E�Đ��Z���~�̌ʐ��Z���O������	|*/
/*|							       get										|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   ���Z����(�s���E�����o�ɂ�����)�E���Z���~�E��	|*/
/*|							   ���Z���~�̌ʐ��Z���O������get����			|*/
/*| RETURN VALUE : cnt_tgt = Kind�Ŏw�肳�ꂽ�ʐ��Z���O�̓o�^����			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-02-28												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
ushort	Ope_SaleLog_TotalCountGet( uchar Kind )
{
	ushort	i;
	ushort	cnt_ttl;
	ushort	cnt_tgt = 0;
	uchar	cnt_flg = FALSE;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// �ʐ��Z���O�o�^������get(Flash+SRAM)

	if( Kind ){														// ���O�����擾�w�肠��?
		for( i = 0; i < cnt_ttl; i++ ){
			WACDOG;													// ���O�����̍ۂ̓E�H�b�`�h�b�N���Z�b�g���s
			memset( &PayLog_Work, 0, sizeof( Receipt_data ) );		// ���[�N�G���A�N���A
			ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, i, &PayLog_Work );// �ʐ��Z���O1����o��
			if( ulRet ){											// ��o��OK
				if(( PayLog_Work.PayClass == 2 ) ||					// �����敪�����Z���~?
				   ( PayLog_Work.PayClass == 3 )){					// �����敪���Đ��Z���~?
					if( Kind & 0x02 ){								// ���Z���~�^�Đ��Z���~�̎w�肠��?
						cnt_flg = TRUE;								// ���O�����Ɋ܂߂�
					}
				}else if(( PayLog_Work.OutKind !=  3 ) &&			// ���Z�o�Ɂ��s���o��?
						 ( PayLog_Work.OutKind !=  1 ) &&			// ���Z�o�Ɂ������o��?
						 ( PayLog_Work.OutKind != 11 ) &&			// ���Z�o�Ɂ������o��(���u)?
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//						 ( PayLog_Work.OutKind != 97 )){			// �t���b�v�㏸�A���b�N�^�C�}���o��
						 ( PayLog_Work.OutKind != 97 ) &&			// �t���b�v�㏸�A���b�N�^�C�}���o��
						 ( PayLog_Work.Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// �����ςł͂Ȃ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
						if( Kind & 0x01 ){							// ���Z�����̎w�肠��?
							cnt_flg = TRUE;							// ���O�����Ɋ܂߂�
					}
				}
				if( cnt_flg == TRUE ){								// ���O�����Ɋ܂߂�?
					cnt_tgt++;										// ���O�����J�E���g�A�b�v
				}
				cnt_flg = FALSE;
			}else{													// ��o��NG
				cnt_tgt = 0;
				break;
			}
		}
	}else{															// �S�ʐ��Z���O����get
		cnt_tgt = cnt_ttl;
	}

	return cnt_tgt;
}

/*[]-----------------------------------------------------------------------[]*/
/*|  �ʐ��ZLOG 1����o��													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	�Ō��ް���0�Ƃ����o�^�ԍ������ɁA�w���ް���1�����ǂݏo���B				|*/
/*|	Call���ɂ��ް���SRAM or FlashROM �ǂ���ɂ��邩���ӎ������Ȃ��֐��B		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = �ړI�ް��̔ԍ��i0�` �j							|*/
/*|						   0=�ŌÁA�ő吔�l=�ŐV							|*/
/*|						  �i��F�o�^����=1000�����̍ŐV�ް���Get����ꍇ999	|*/
/*|							��n�����Ɓj									|*/
/*|				   Kind  = ���o���w��(bit�Ή�)							|*/
/*|							all"0":�S�ʐ��Z���O����o���ΏۂƂ���			|*/
/*|							bit0=1:���Z����(�s���E�����o�ɂ�����)�̌ʐ��Z	|*/
/*|							       ���O����o���ΏۂƂ���					|*/
/*|							bit1=1:���Z���~�E�Đ��Z���~�̌ʐ��Z���O����o	|*/
/*|							       ���ΏۂƂ���								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   ���Z����(�s���E�����o�ɂ�����)�E���Z���~�E��	|*/
/*|							   ���Z���~�̌ʐ��Z���O����o���ΏۂƂ���		|*/
/*|				   Index_Kind  = �w�肳�ꂽ�ʐ��Z���O�̑�����				|*/
/*|								 (Ope_SaleLog_TotalCountGet()�Ŏ擾)		|*/
/*|				   pSetBuf = Read�ް���Ă���ر�ւ��߲��					|*/
/*| RETURN VALUE : 1=Read�����A0=���s										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-03-05												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf )
{
	ushort	cnt_ttl;
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// �ʐ��Z���O�o�^������get(Flash+SRAM)
	index_tgt = cnt_ttl - 1;
	index_knd = Index_Kind - 1;

	while( !flg_end ){
		WACDOG;														// ���O�����̍ۂ̓E�H�b�`�h�b�N���Z�b�g���s
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// �ʐ��Z���O���o��OK
			if( !Kind ){											// �S�ʐ��Z���O��o���Ώ�?
				flg_tgt = TRUE;										// �w�肳�ꂽ�ʐ��Z���O��o��OK
			}else if(( pSetBuf->PayClass == 2 ) ||					// �����敪�����Z���~?
					 ( pSetBuf->PayClass == 3 )){					// �����敪���Đ��Z���~?
				if( Kind & 0x02 ){									// ���Z���~�^�Đ��Z���~�̎w�肠��
					flg_tgt = TRUE;									// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// ���Z�o�Ɂ��s���o��?
					 ( pSetBuf->OutKind !=  1 ) &&					// ���Z�o�Ɂ������o��?
					 ( pSetBuf->OutKind != 11 ) &&					// ���Z�o�Ɂ������o��(���u)?
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//					 ( pSetBuf->OutKind != 97 )){					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->OutKind != 97 ) &&					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// �����ςł͂Ȃ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
					if( Kind & 0x01 ){								// ���Z�����̎w�肠��?
						flg_tgt = TRUE;								// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}
			if( flg_tgt == TRUE ){									// �w�肳�ꂽ���O?
				if( index_knd == Index ){							// �w�肳�ꂽ�ԍ�?
					flg_end = ON;									// �����I��(Read����)
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// �w�肳�ꂽ�ʐ��Z���O�����������p��?
						index_knd--;								// ���̎w�肳�ꂽ�ʐ��Z���O����
					}else{
						flg_end = ON;								// �����I��(Read���s)
					}
				}
			}
			if( flg_end == OFF ){									// �����I��?
				if( index_tgt ){									// �ʐ��Z���O�����������p��?
					index_tgt--;									// ���̌ʐ��Z���O����
				}else{
					flg_end = ON;									// �����I��(Read���s)
				}
			}
		}else{
			flg_end = ON;											// �ʐ��Z���O���o��NG
		}
	}

	return flg_tgt;
}
/*[]-----------------------------------------------------------------------[]*/
/*|  �ʐ��ZLOG 1����o��													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	�Ō��ް���0�Ƃ����o�^�ԍ������ɁA�w���ް���1�����ǂݏo���B				|*/
/*|	Call���ɂ��ް���SRAM or FlashROM �ǂ���ɂ��邩���ӎ������Ȃ��֐��B		|*/
/*| �A����Read����ꍇ�Ɏg�p����֐��B										|*/
/*| �o�͂��ꂽpNextLog�����̂܂�Ope_SaleLog_Next1DataGet()�Ɏg�p����		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = �ړI�ް��̔ԍ��i0�` �j							|*/
/*|						   0=�ŌÁA�ő吔�l=�ŐV							|*/
/*|						  �i��F�o�^����=1000�����̍ŐV�ް���Get����ꍇ999	|*/
/*|							��n�����Ɓj									|*/
/*|				   Kind  = ���o���w��(bit�Ή�)							|*/
/*|							all"0":�S�ʐ��Z���O����o���ΏۂƂ���			|*/
/*|							bit0=1:���Z����(�s���E�����o�ɂ�����)�̌ʐ��Z	|*/
/*|							       ���O����o���ΏۂƂ���					|*/
/*|							bit1=1:���Z���~�E�Đ��Z���~�̌ʐ��Z���O����o	|*/
/*|							       ���ΏۂƂ���								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   ���Z����(�s���E�����o�ɂ�����)�E���Z���~�E��	|*/
/*|							   ���Z���~�̌ʐ��Z���O����o���ΏۂƂ���		|*/
/*|				   Index_Kind  = �w�肳�ꂽ�ʐ��Z���O�̑�����				|*/
/*|								 (Ope_SaleLog_TotalCountGet()�Ŏ擾)		|*/
/*|				   pSetBuf = Read�ް���Ă���ر�ւ��߲��					|*/
/*|				   pNextLog = ���Ɍ������郍�O���i�[�̈�					|*/
/*|				   				Ope_SaleLog_Next1DataGet()�Ŏg�p����		|*/
/*| RETURN VALUE : 1=Read�����A0=���s										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : m.nagashima												|*/
/*| Date		 : 2012-03-05												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_First1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog )
{
	ushort	cnt_ttl;
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	cnt_ttl = Ope_Log_TotalCountGet( eLOG_PAYMENT );				// �ʐ��Z���O�o�^������get(Flash+SRAM)
	index_tgt = cnt_ttl - 1;
	index_knd = Index_Kind - 1;

	while( !flg_end ){
		WACDOG;														// ���O�����̍ۂ̓E�H�b�`�h�b�N���Z�b�g���s
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// �ʐ��Z���O���o��OK
			if( !Kind ){											// �S�ʐ��Z���O��o���Ώ�?
				flg_tgt = TRUE;										// �w�肳�ꂽ�ʐ��Z���O��o��OK
			}else if(( pSetBuf->PayClass == 2 ) ||					// �����敪�����Z���~?
					 ( pSetBuf->PayClass == 3 )){					// �����敪���Đ��Z���~?
				if( Kind & 0x02 ){									// ���Z���~�^�Đ��Z���~�̎w�肠��
					flg_tgt = TRUE;									// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// ���Z�o�Ɂ��s���o��?
					 ( pSetBuf->OutKind !=  1 ) &&					// ���Z�o�Ɂ������o��?
					 ( pSetBuf->OutKind != 11 ) &&					// ���Z�o�Ɂ������o��(���u)?
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//					 ( pSetBuf->OutKind != 97 )){					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->OutKind != 97 ) &&					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// �����ςł͂Ȃ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
					if( Kind & 0x01 ){								// ���Z�����̎w�肠��?
						flg_tgt = TRUE;								// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}
			if( flg_tgt == TRUE ){									// �w�肳�ꂽ���O?
				if( index_knd == Index ){							// �w�肳�ꂽ�ԍ�?
					flg_end = ON;									// �����I��(Read����)
					if(pNextLog) {
						pNextLog->nextIndex = index_tgt;			// ���������C���f�b�N�X���i�[����
						pNextLog->nextCount = index_knd;			// ���̌����Ώی������i�[����
					}
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// �w�肳�ꂽ�ʐ��Z���O�����������p��?
						index_knd--;								// ���̎w�肳�ꂽ�ʐ��Z���O����
					}else{
						flg_end = ON;								// �����I��(Read���s)
					}
				}
			}
			if( flg_end == OFF ){									// �����I��?
				if( index_tgt ){									// �ʐ��Z���O�����������p��?
					index_tgt--;									// ���̌ʐ��Z���O����
				}else{
					flg_end = ON;									// �����I��(Read���s)
				}
			}
		}else{
			flg_end = ON;											// �ʐ��Z���O���o��NG
		}
	}

	return flg_tgt;
}
/*[]-----------------------------------------------------------------------[]*/
/*|  �ʐ��ZLOG 1����o��													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	�Ō��ް���0�Ƃ����o�^�ԍ������ɁA�w���ް���1�����ǂݏo���B				|*/
/*|	Call���ɂ��ް���SRAM or FlashROM �ǂ���ɂ��邩���ӎ������Ȃ��֐��B		|*/
/*| �A����Read����ꍇ�Ɏg�p����֐��B										|*/
/*|	Ope_SaleLog_First1DataGet()�̌�ɃR�[�����邱��							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : Index = �ړI�ް��̔ԍ��i0�` �j							|*/
/*|						   0=�ŌÁA�ő吔�l=�ŐV							|*/
/*|						  �i��F�o�^����=1000�����̍ŐV�ް���Get����ꍇ999	|*/
/*|							��n�����Ɓj									|*/
/*|				   Kind  = ���o���w��(bit�Ή�)							|*/
/*|							all"0":�S�ʐ��Z���O����o���ΏۂƂ���			|*/
/*|							bit0=1:���Z����(�s���E�����o�ɂ�����)�̌ʐ��Z	|*/
/*|							       ���O����o���ΏۂƂ���					|*/
/*|							bit1=1:���Z���~�E�Đ��Z���~�̌ʐ��Z���O����o	|*/
/*|							       ���ΏۂƂ���								|*/
/*|							ex)Kind=3(0000011:PAY_LOG_CMP_STP)				|*/
/*|							   ���Z����(�s���E�����o�ɂ�����)�E���Z���~�E��	|*/
/*|							   ���Z���~�̌ʐ��Z���O����o���ΏۂƂ���		|*/
/*|				   pSetBuf = Read�ް���Ă���ر�ւ��߲��					|*/
/*|				   pNextLog = ���Ɍ������郍�O���i�[�̈�					|*/
/*| 							���̂܂܎���Ope_SaleLog_Next1DataGet()�Ɏg�p|*/
/*|								����										|*/
/*| RETURN VALUE : 1=Read�����A0=���s										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author		 : S.Takahashi												|*/
/*| Date		 : 2012-05-11												|*/
/*| UpDate		 :															|*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.----[]*/
uchar	Ope_SaleLog_Next1DataGet( ushort Index, uchar Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog )
{
	ushort	index_tgt;
	ushort	index_knd;
	uchar	flg_tgt = FALSE;
	uchar	flg_end = OFF;
	uchar	ulRet;

	index_tgt = pNextLog->nextIndex - 1;							// �����J�n�C���f�b�N�X
	index_knd = pNextLog->nextCount - 1;							// �����Ώۂ̃��O����

	while( !flg_end ){
		WACDOG;														// ���O�����̍ۂ̓E�H�b�`�h�b�N���Z�b�g���s
		ulRet = Ope_Log_1DataGet( eLOG_PAYMENT, index_tgt, (char*)pSetBuf );
		if( ulRet ){												// �ʐ��Z���O���o��OK
			if( !Kind ){											// �S�ʐ��Z���O��o���Ώ�?
				flg_tgt = TRUE;										// �w�肳�ꂽ�ʐ��Z���O��o��OK
			}else if(( pSetBuf->PayClass == 2 ) ||					// �����敪�����Z���~?
					 ( pSetBuf->PayClass == 3 )){					// �����敪���Đ��Z���~?
				if( Kind & 0x02 ){									// ���Z���~�^�Đ��Z���~�̎w�肠��
					flg_tgt = TRUE;									// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}else if(( pSetBuf->OutKind !=  3 ) &&					// ���Z�o�Ɂ��s���o��?
					 ( pSetBuf->OutKind !=  1 ) &&					// ���Z�o�Ɂ������o��?
					 ( pSetBuf->OutKind != 11 ) &&					// ���Z�o�Ɂ������o��(���u)?
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//					 ( pSetBuf->OutKind != 97 )){					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->OutKind != 97 ) &&					// �t���b�v�㏸�A���b�N�^�C�}���o��
					 ( pSetBuf->Electron_data.Ec.E_Status.BIT.deemed_receive != 1 )){	// �����ςł͂Ȃ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
					if( Kind & 0x01 ){								// ���Z�����̎w�肠��?
						flg_tgt = TRUE;								// �w�肳�ꂽ�ʐ��Z���O��o��OK
				}
			}
			if( flg_tgt == TRUE ){									// �w�肳�ꂽ���O?
				if( index_knd == Index ){							// �w�肳�ꂽ�ԍ�?
					flg_end = ON;									// �����I��(Read����)
					if(pNextLog) {
						pNextLog->nextIndex = index_tgt;
						pNextLog->nextCount = index_knd;
					}
				}else{
					flg_tgt = FALSE;
					if( index_knd ){								// �w�肳�ꂽ�ʐ��Z���O�����������p��?
						index_knd--;								// ���̎w�肳�ꂽ�ʐ��Z���O����
					}else{
						flg_end = ON;								// �����I��(Read���s)
					}
				}
			}
			if( flg_end == OFF ){									// �����I��?
				if( index_tgt ){									// �ʐ��Z���O�����������p��?
					index_tgt--;									// ���̌ʐ��Z���O����
				}else{
					flg_end = ON;									// �����I��(Read���s)
				}
			}
		}else{
			flg_end = ON;											// �ʐ��Z���O���o��NG
		}
	}

	return flg_tgt;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��t���Ĕ��s�|�ݒ���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set( area )                                     |*/
/*| PARAMETER    : char area  : ���                                       |*/
/*|              : short kind : �@�\���(���g�p)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lockuke_set(char area, short kind)
{
	ushort	msg;
	short	top;
	short	no;
	long	input;
	short	max;
	short	i, j;
	uchar	type;

	/* ���b�N���u���(LockInfo[])����area���ł���f�[�^�𒊏o���� */
	for (i = 0, j = 0; i < LOCK_MAX; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0 && 
			(FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4)) {
			if (area == 0) {
				lock_idx[j++] = i;
			}
			else {
				if (LockInfo[i].area == area) {
					lock_idx[j++] = i;
				}
			}
		}
	}
	max = j-1;

	/* �Y���f�[�^�Ȃ� */
	if (max == -1) {
		for (i = 0; i < 6; i++) {
			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
		}
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);						/* "�@�@�@�f�[�^������܂���@�@�@" */
		Fun_Dsp(FUNMSG[8]);																		/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
		for( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch(msg){					/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return 1;
			case KEY_TEN_F5:				/* F5:�I�� */
				BUZPI();
				return 0;
			default:
				break;
			}
		}
	}

	top = no = 0;
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[46] );				/* "�@�y�Ԏ��ԍ��z�@�@�@�y��ԁz�@" */
	lockuke_set_all_dsp(top, max, no, lock_idx);									/* �f�[�^�ꊇ�\�� */
	Fun_Dsp( FUNMSG[72] );															/* "  �{  �|�^��       ���s  �I�� " */

	input = -1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
			return 2;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
			return 1;
		case KEY_TEN_F5:
			BUZPI();
			LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
			return 0;
		case KEY_TEN_F1:				/* F1:�� */
			BUZPI();
			no--;
			if (no < 0) {
				no = max;
				top = (short)(max/5*5);
			}
			else if (no < top) {
				top -= 5;
			}
			/* �f�[�^�ꊇ�\�� */
			lockuke_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F2:				/* F2:���^�� */
			if (input == -1) {
				BUZPI();
				no++;
				if (no > max) {
					top = no = 0;
				}
				else if (no > top+4) {
					top = no;
				}
			}
			else {
				for (i = 0; i <= max; i++) {
					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
						break;
					}
				}
				if (i > max) {
					/* ���͂����Ԏ��ԍ���������Ȃ��ꍇ */
					BUZPIPI();
					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
					input = -1;
					break;
				}
				BUZPI();
				no = i;
				top = (short)(no/5*5);
			}
			/* �f�[�^�ꊇ�\�� */
			lockuke_set_all_dsp(top, max, no, lock_idx);
			input = -1;
			break;
		case KEY_TEN_F4:

			if((( FLAPDT.flp_data[lock_idx[no]].uketuke == 0 )&&				/* ���s�ς݃t���OOFF? */
				( prm_get( COM_PRM,S_TYP,62,1,1 ) == 1 )) ||					/* �����s�̔��s�͂Ȃ� */
				( prm_get(COM_PRM, S_TYP, 62, 1, 3) == 0 )){ 					/* ���s������O�� */
				// �����s�̎Ԏ��̓����e�i���X�ł͔��s�����Ȃ��B
				BUZPIPI();
				break;
			}

			if( PriRctCheck() != OK ) {											// ڼ�Ĉ󎚏����^�s������
				BUZPIPI();
				break;
			}

			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			/* ��t�����s���� */
			type = 1;
			if( FLAPDT.flp_data[lock_idx[no]].uketuke == 0 ){					/* �����s */
				type = 2;
			}
			else																/* 1���ȏ㔭�s�ς� */
			{
				FLAPDT.flp_data[lock_idx[no]].issue_cnt--;						/* ���s�����f�N�������g(�󎚑O�ɃC���N�������g�����) */
			}

			uke_isu((ulong)(LockInfo[lock_idx[no]].area * 10000L + LockInfo[lock_idx[no]].posi), (ushort)(lock_idx[no] + 1), type );	/* ��t�����s�v���i�Ĕ��s�j */
			Ope_DisableDoorKnobChime();
			OpelogNo = OPLOG_UKETUKESAIHAKKO;		// ���엚��o�^
			/* �I����҂����킹�� */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
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
					LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
					return 1;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if ( msg == LCD_DISCONNECT ) {
					Lagcan(OPETCBNO, 6);
					LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
					return 2;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				
			}
			lockuke_set_sts_dsp(top, no, &FLAPDT.flp_data[lock_idx[no]]);
			input = -1;
			break;
		case KEY_TEN:
			BUZPI();
			if (input == -1) {
				input = 0;
			}
			input = (input*10 + msg-KEY_TEN0) % 10000;
			area_posi_dsp(top, no, area, (ulong)input, 1);
			break;
		case KEY_TEN_CL:
			BUZPI();
			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���o�Ɉ󎚉��                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lock_io_set( area )                                     |*/
/*| PARAMETER    : char area  : ���                                       |*/
/*|              : short kind : �@�\���(���g�p)                           |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	lock_io_set(char area, short kind)
{
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	short	top;
//	short	no;
//	short	max;
//	short	i, j;
//	short	start, end;
//	ushort	msg;
//	long	input;
//	char	clr_flg = 0;
//	/* ���b�N���u���(LockInfo[])����area���ł���f�[�^�𒊏o���� */
//	if (gCurSrtPara == MNT_CAR || gCurSrtPara == MNT_INT_CAR ) {
//		no = (char)GetCarInfoParam();
//		switch( no&0x06 ){
//			case	0x04:
//				start = INT_CAR_START_INDEX;
//				end   = start + INT_CAR_LOCK_MAX;
//				break;
//			case	0x06:
//				start = CAR_START_INDEX;
//				end   = start + CAR_LOCK_MAX + INT_CAR_LOCK_MAX;
//				break;
//			case	0x02:
//			default:
//				start = CAR_START_INDEX;
//				end   = start + CAR_LOCK_MAX;
//				break;
//		}
//	}
//	else {
//		start = BIKE_START_INDEX;
//		end   = start + BIKE_LOCK_MAX;
//	}
//	
//	for (i = start, j = 0; i < end; i++) {
//		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
//		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
//			if (area == 0) {
//				lock_idx[j++] = i;
//			}
//			else {
//				if (LockInfo[i].area == area) {
//					lock_idx[j++] = i;
//				}
//			}
//		}
//	}
//	max = j-1;
//
//	/* �Y���f�[�^�Ȃ� */
//	if (max == -1) {
//		for (i = 0; i < 6; i++) {
//			grachr((ushort)(1+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
//		}
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[49]);						/* "�@�@�@�f�[�^������܂���@�@�@" */
//		Fun_Dsp(FUNMSG[8]);																		/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//		for( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){					/* FunctionKey Enter */
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			case KEY_MODECHG:
//				BUZPI();
//				return 1;
//			case KEY_TEN_F5:				/* F5:�I�� */
//				BUZPI();
//				return 0;
//			default:
//				break;
//			}
//		}
//	}
//
//	top = no = 0;
//	input = -1;
//	Fun_Dsp( FUNMSG[105] );															/* "�@�{�@�|�^�ǃN���A �Ǐo  �I�� " */
//	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "�@�y�Ԏ��ԍ��z�@�@�@�@�@�@�@�@" */
//	lockio_set_all_dsp(top, max, no, lock_idx);										/* �f�[�^�ꊇ�\�� */
//
//	for( ; ; ){
//		msg = StoF( GetMessage(), 1 );
//		switch( KEY_TEN0to9( msg ) ){
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		case KEY_MODECHG:
//			BUZPI();
//			LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
//			return 1;
//		case KEY_TEN_F5:
//			BUZPI();
//			LedReq(CN_TRAYLED, LED_OFF);	/* ��ݎ�o�����޲��LED OFF */
//			return 0;
//		case KEY_TEN_F1:				/* F1:�� */
//			BUZPI();
//			no--;
//			if (no < 0) {
//				no = max;
//				top = (short)(max/5*5);
//			}
//			else if (no < top) {
//				top -= 5;
//			}
//			/* �f�[�^�ꊇ�\�� */
//			lockio_set_all_dsp(top, max, no, lock_idx);
//			input = -1;
//			break;
//		case KEY_TEN_F2:				/* F2:�� */
//			if (input == -1) {
//				BUZPI();
//				no++;
//				if (no > max) {
//					top = no = 0;
//				}
//				else if (no > top+4) {
//					top = no;
//				}
//			}
//			else {
//				for (i = 0; i <= max; i++) {
//					if (LockInfo[lock_idx[i]].posi == (ulong)input) {
//						break;
//					}
//				}
//				if (i > max) {
//					/* ���͂����Ԏ��ԍ���������Ȃ��ꍇ */
//					BUZPIPI();
//					area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
//					input = -1;
//					break;
//				}
//				BUZPI();
//				no = i;
//				top = (short)(no/5*5);
//			}
//			/* �f�[�^�ꊇ�\�� */
//			lockio_set_all_dsp(top, max, no, lock_idx);
//			input = -1;
//			break;
//		case KEY_TEN_F4:
//			BUZPI();
//			msg = UsMnt_IoLog((ushort)LockInfo[lock_idx[no]].posi);
//			dispmlclr(1,7);																	/* ��ʕ\�������� */
//			Fun_Dsp( FUNMSG[105] );															/* "�@�{�@�|�^�ǃN���A �Ǐo  �I�� " */
//			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "�@�y�Ԏ��ԍ��z�@�@�@�@�@�@�@�@" */
//			lockio_set_all_dsp(top, max, no, lock_idx);										/* �f�[�^�ꊇ�\�� */
//			break;
//		case KEY_TEN:
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 10000;
//			area_posi_dsp(top, no, area, (ulong)input, 1);
//			break;
//		case KEY_TEN_CL:
//			BUZPI();
//			area_posi_dsp(top, no, area, LockInfo[lock_idx[no]].posi, 1);
//			input = -1;
//			break;
//		case KEY_TEN_F3:
//			BUZPI();
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[61]);				// "�@�S�Ẵ��O���N���A���܂����H", // [61]
//			Fun_Dsp(FUNMSG[19]);							// "�@�@�@�@�@�@ �͂� �������@�@�@"
//
//			for( clr_flg = 0;  clr_flg == 0; ){
//				msg = StoF( GetMessage(), 1 );			// �C�x���g�҂�
//
//				switch( msg ){							// �C�x���g�H
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					case KEY_MODECHG:						// �ݒ�L�[�ؑ�
//						clr_flg = 1;
//						break;
//
//					case KEY_TEN_F3:						// �e�R�i�͂��j�L�[����
//						LogDataClr(LOG_IOLOG);
//						// no break
//					case KEY_TEN_F4:						// �e�S�i�������j�L�[����
//						BUZPI();
//						clr_flg = 1;
//						break;
//					default:
//						break;
//				}
//				if(clr_flg == 1){
//					dispmlclr(1,7);																	/* ��ʕ\�������� */
//					Fun_Dsp( FUNMSG[105] );															/* "�@�{�@�|�^�ǁ@�@�@ �Ǐo  �I�� " */
//					grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[129] );			/* "�@�y�Ԏ��ԍ��z�@�@�@�@�@�@�@�@" */
//					lockio_set_all_dsp(top, max, no, lock_idx);										/* �f�[�^�ꊇ�\�� */
//				}
//			}
//			break;
//		default:
//			break;
//		}
//
//		/* ���[�h�`�F���W */
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
////		if (msg == KEY_MODECHG) {
//		if (msg == KEY_MODECHG || msg == LCD_DISCONNECT) {	
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			return 1;
//		}
//	}
	return 0;
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
}

// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
///*[]----------------------------------------------------------------------[]*/
///*|  ���o�ɏ���ʕ\���|�S�f�[�^�\��(5���܂�)                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : lockio_set_all_dsp( top, max, rev_no, *idx )           |*/
///*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
///*|              : short max    : �ő�ԍ�                                 |*/
///*|              : short rev_no : ���]�ԍ�                                 |*/
///*|              : short *idx   : ���b�N���u���̔z��ԍ��e�[�u��         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// static void	lockio_set_all_dsp(short top, short max, short rev_no, short *idx)
// {
// 	char	i;
//
// 	for (i = 0; i < 5; i++) {
// 		if (top+i > max) {
// 			break;
// 		}
// 		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
// 	}
// 	for ( ; i < 5; i++) {
// 		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
// 	}
// }
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)

/*[]----------------------------------------------------------------------[]*/
/*|  ��t���Ĕ��s�|��ԕ\��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set_sts_dsp( top, no, bit )                     |*/
/*| PARAMETER    : short top   : �擪�ԍ�                                  |*/
/*|              : short no    : �\���ԍ�                                  |*/
/*|              : BITS  bit   : ���b�N���                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockuke_set_sts_dsp(short top, short no, flp_com *pflp)
{
	if (pflp->uketuke) {
		grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_1[1]);		/* "���s��" */
	} else {
		grachr((ushort)(2+no-top), 21, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_1[0]);		/* "�����s" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��t���Ĕ��s�|�S�f�[�^�\��(5���܂�)                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockuke_set_all_dsp( top, max, rev_no, *idx )           |*/
/*| PARAMETER    : short top    : �擪�ԍ�                                 |*/
/*|              : short max    : �ő�ԍ�                                 |*/
/*|              : short rev_no : ���]�ԍ�                                 |*/
/*|              : short *idx   : ���b�N���u���̔z��ԍ��e�[�u��         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	lockuke_set_all_dsp(short top, short max, short rev_no, short *idx)
{
	char	i;

	for (i = 0; i < 5; i++) {
		if (top+i > max) {
			break;
		}
		area_posi_dsp(top, (short)(top+i), (char)LockInfo[idx[top+i]].area, LockInfo[idx[top+i]].posi, (rev_no==top+i)?1:0);
		lockuke_set_sts_dsp(top, (short)(top+i), &FLAPDT.flp_data[idx[top+i]]);
	}
	for ( ; i < 5; i++) {
		grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
	}
}

#define PARK_MAX	4

typedef struct {
	char no;		// ���ԏ�ԍ��C���f�b�N�X
	char mno;		// �}�X�^�[���Ԕԍ��C���f�b�N�X
} s_ParkNo;

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//unsigned short	UsMnt_PassInv(void)
//{
//	s_ParkNo	index[PARK_MAX];
//	ushort		msg;
//	char		ret;
//	char		max_block;
//	char		i, j;
//	const uchar	*str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	char		dsp;
//	int			pos;
//	
//	/* �L�����ԏ�ԍ����擾 */
//	max_block = 0;
//	memset(index, 0, (PARK_MAX * 2));
//	for (i = 0; i < PARK_MAX; i++) {
//		pos = (i == 0) ? 6 : i;
//		if (prm_get( COM_PRM,S_SYS,70,1,pos ) && (CPrmSS[S_SYS][i+1] != 0) ) {
//			for (j = 0; j < PARK_MAX; j++) {
//				if (CPrmSS[S_SYS][i+1] == CPrmSS[S_SYS][65+j]) {
//					index[max_block].no  = i;
//					index[max_block].mno = j;
//					max_block++;
//					break;
//				}
//			}
//		}
//	}
//
//	if (max_block == 0) {
//		dispclr();
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);	/* "������L���^�������@�@�@�@�@�@" */
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	/* "�@�@�@�f�[�^�͂���܂���@�@�@" */
//		Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//		
//		for ( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:�I�� */
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//	else {
//		pos = 0;
//		dsp = 1;
//		for( ; ; ) {
//			if (dsp) {
//				/* ������ʕ\�� */
//				dispclr();
//				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);	/* "������L���^�������@�@�@�@�@�@" */
//				for (i = 0; i < max_block; i++) {
//					grachr((ushort)(2+i), 4, 6, ((pos==i)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[i].no]);
//				}
//				if(max_block >= 2){
//					Fun_Dsp( FUNMSG2[13] );				/* "�@���@�@���@�����  �Ǐo  �I�� " */				
//				}
//				else{
//					Fun_Dsp( FUNMSG2[49] );				/* "�@�@�@�@�@�@�����  �Ǐo  �I�� " */				
//				}
//				dsp = 0;
//			}
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:�I�� */
//				BUZPI();
//				return MOD_EXT;
//			case KEY_TEN_F1:			/* F1:�� */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (--pos < 0) {
//						pos = max_block - 1;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F2:			/* F2:�� */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (++pos >= max_block) {
//						pos = 0;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F3:			/* F3:����� */
//				BUZPI();
//				/* �v�����g��ʂ� */
//				ret = passinv_print(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			case KEY_TEN_F4:			/* F4:�Ǐo */
//				BUZPI();
//				/* �ݒ��ʂ� */
//				ret = passinv_set(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ����L���^�����[�ݒ���                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_set( block )                                    |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//#define	PASINV(b, o)	(pas_tbl[((b)+(o)-1)].BIT.INV)
//static char	passinv_set(char block, char mode)
//{
//	ushort	msg;
//	long	no;
//	long	max;
//	long	top;
//	char	ret;
//	long	begin;
//	long	end;
//	char	i;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	const uchar	*str_tbl[] = {DAT2_6[11], DAT2_6[12]};
//	long	input;
//	char	dsp;
//	PAS_TBL pass_data;
//
//	if (block == 0) {
//		begin = 1;
//		end = (CPrmSS[S_SYS][61] == 0) ? 12000 : CPrmSS[S_SYS][61]-1;
//	}
//	else if (block == 1) {
//		begin = CPrmSS[S_SYS][61];
//		end = (CPrmSS[S_SYS][62] == 0) ? 12000 : CPrmSS[S_SYS][62]-1;
//	}
//	else if (block == 2) {
//		begin = CPrmSS[S_SYS][62];
//		end = (CPrmSS[S_SYS][63] == 0) ? 12000 : CPrmSS[S_SYS][63]-1;
//	}
//	else {	/* if (block == 3) */
//		begin = CPrmSS[S_SYS][63];
//		end = 12000;
//	}
//	max = end-begin;
//
//	no = top = 0;
//	input = -1;
//	dsp = 1;
//	for( ; ; ) {
//		if (dsp) {
//			/* ������ʕ\�� */
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);			/* "������L���^�������@�@�@�@�@�@" */
//			grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);	/* "�@�@�@�@�@�@�@�@�@�@�@�@�w�w�w" */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//				grachr((ushort)(1+i), 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);						/* "�|" */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);	/* "�L��"or"����" */
//			}
//			Fun_Dsp(FUNMSG[17]);					/* "  �{  �|�^�� �ύX �S�ύX �I�� " */
//			dsp = 0;
//			pass_data = pas_tbl[((begin)+(no)-1)];
//		}
//
//		msg = StoF( GetMessage(), 1 );
//		switch (KEY_TEN0to9(msg)) {
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:�I�� */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:�{ */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			no--;
//			if (no < 0) {
//				if (max > 5) {
//					top = max-5;
//				} else {
//					top = 0;
//				}
//				no = max;
//			}
//			else if (no < top) {
//				top--;
//			}
//			/* �\�� */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);	/* "�L��"or"����" */
//			}
//			pass_data = pas_tbl[((begin)+(no)-1)];
//			input = -1;
//			break;
//		case KEY_TEN_F2:		/* F2:�|�^�� */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			if (input == -1) {
//				BUZPI();
//				no++;
//				if (no > max) {
//					no = top = 0;
//				} else if (no > top+5) {
//					top++;
//				}
//			}
//			else {
//				if (input == 0 || input > max+1) {
//					BUZPIPI();
//				}
//				else {
//					BUZPI();
//					top = no = input-1;
//					if (top + 5 > max) {
//						if (max > 5) {
//							top = max-5;
//						} else {
//							top = 0;
//						}
//					}
//				}
//			}
//			/* �\�� */
//			for (i = 0; i < 6; i++) {
//				if (top+i > max) {
//					break;
//				}
//				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 5, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin,top+i)]);		/* "�L��"or"����" */
//			}
//			pass_data = pas_tbl[((begin)+(no)-1)];
//			input = -1;
//			break;
//		case KEY_TEN_F3:		/* F3:�ύX */
//			BUZPI();
//			PASINV(begin, no) ^= 1;
//			OpelogNo = OPLOG_TEIKIYUKOMUKO;		// ���엚��o�^
//			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);				/* �ԍ� */
//			grachr((ushort)(1+no-top), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[PASINV(begin, no)]);	/* "�L��"or"����" */
//			input = -1;
//			break;
//		case KEY_TEN_F4:		/* F4:�S�ύX */
//			passinv_check(pass_data, pas_tbl[((begin)+(no)-1)], (ushort)(no+1), (ushort)block);
//			BUZPI();
//			ret = passinv_allset( mode, block );
//			if (ret == 1) {
//				return 1;
//			}
//			dsp = 1;
//			input = -1;
//			break;
//		case KEY_TEN:			/* ����(�e���L�[) */
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 100000L;
//			opedpl((ushort)(1+no-top), 2, (ulong)input, 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//			break;
//		case KEY_TEN_CL:		/* ���(�e���L�[) */
//			BUZPI();
//			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//			input = -1;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ����L���^�����[�S�ݒ���                                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_allset( block )                                 |*/
///*| PARAMETER    : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*|                char block : block number                               |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passinv_allset( char mode, char block )
//{
//	ushort	msg;
//	char	pos;	/* 0:�L�� 1:���� */
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37]);		/* "������L���^�������@�@�@�@�@�@" */
//	grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);/* "�@�@�@�@�@�@�@�@�@�@�@�@�w�w�w" */
//	grachr(2, 12, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);		/* "�@�@�@�@�@�@�L���@�@�@�@�@�@�@" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[38]);		/* "�@�S�_��No���@�@�ɂ��܂����H�@" */
//	grachr(4, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);		/* "�@�@�@�@�@�@�����@�@�@�@�@�@�@" */
//	Fun_Dsp(FUNMSG[18]);													/* "�@���@�@���@ �͂� �������@�@�@" */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F1:		/* F1:�� */
//		case KEY_TEN_F2:		/* F2:�� */
//			BUZPI();
//			pos ^= 1;
//			grachr(2, 12, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);	/* "�@�@�@�@�@�@�L���@�@�@�@�@�@�@" */
//			grachr(4, 12, 4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);	/* "�@�@�@�@�@�@�����@�@�@�@�@�@�@" */
//			break;
//		case KEY_TEN_F3:		/* F3:�͂� */
//			BUZPI();
//			/* �S�ݒ�ύX */
//			if( pos == 0 ){
//				wopelg( OPLOG_TEIKIZENYUKO, 0, 0 );		// ���엚��o�^
//				// �S�L��
//				FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0, 0xFFFFFFFF, 0, 0 );
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//									, 0
//									, (ulong)(CPrmSS[S_SYS][65 + block])
//									, 0
//									, NTNET_PASSUPDATE_ALL_INITIAL
//									, 0
//									, NULL );
//			}else{
//				wopelg( OPLOG_TEIKIZENMUKO, 0, 0 );		// ���엚��o�^
//				// �S����
//				FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0xFFFF, 0xFFFFFFFF, 1, 0 );
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//									, 0
//									, (ulong)(CPrmSS[S_SYS][65 + block])
//									, 0
//									, NTNET_PASSUPDATE_ALL_INVALID
//									, 0
//									, NULL );
//			}
//			return 0;
//		case KEY_TEN_F4:		/* F4:������ */
//			BUZPI();
//			return 0;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ����L���^�����[�v�����g���                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_print( block )                                  |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passinv_print(char block, char mode)
//{
//	ushort			msg;
//	int				pos;		/* 0:�L���̂� 1:�����̂� 2:�S�� */
//	T_FrmTeikiData1	pri_data;
//	uchar			priend;
//	char			ret;
//	T_FrmPrnStop	FrmPrnStop;
//
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[37] );		/* "������L���^�������@�@�@�@�@�@" */
//	passinv_print_dsp(0, 1);												/* "�@�@�L���̂݃v�����g�@�@�@�@�@" */
//	passinv_print_dsp(1, 0);												/* "�@�@�����̂݃v�����g�@�@�@�@�@" */
//	passinv_print_dsp(2, 0);												/* "�@�@�S�ăv�����g�@�@�@�@�@�@�@" */
//	Fun_Dsp(FUNMSG[68]);													/* "�@���@�@���@�@�@�@ ���s  �I�� " */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:�I�� */
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:�� */
//		case KEY_TEN_F2:		/* F2:�� */
//			BUZPI();
//			passinv_print_dsp(pos, 0);
//			if (msg == KEY_TEN_F1) {
//				if (--pos < 0) {
//					pos = 2;
//				}
//			}
//			else {
//				if (++pos > 2) {
//					pos = 0;
//				}
//			}
//			passinv_print_dsp(pos, 1);
//			break;
//		case KEY_TEN_F4:		/* F4:���s */
//			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//				BUZPIPI();
//				break;
//			}
//			BUZPI();
//			/* �v�����g���s */
//			pri_data.prn_kind = R_PRI;
//			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			pri_data.Kakari_no = OPECTL.Kakari_Num;
//			pri_data.Pkno_syu = (uchar)mode;
//			if (pos == 0) {			/* �L���̂� */
//				pri_data.Req_syu = 1;
//			} else if (pos == 1) {	/* �����̂� */
//				pri_data.Req_syu = 2;
//			} else {				/* �S�� */
//				pri_data.Req_syu = 0;
//			}
//			queset(PRNTCBNO, PREQ_TEIKI_DATA1, sizeof(T_FrmTeikiData1), &pri_data);
//			Ope_DisableDoorKnobChime();
//
//			/* �v�����g�I����҂����킹�� */
//
//			Fun_Dsp(FUNMSG[82]);				// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
//
//			for ( priend = 0 ; priend == 0 ; ) {
//
//				msg = StoF( GetMessage(), 1 );
//
//				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
//					msg &= (~INNJI_ENDMASK);
//				}
//				switch( msg ){
//
//					case	KEY_MODECHG:		// Ӱ����ݼ�
//						BUZPI();
//						ret = 1;
//						priend = 1;
//						break;
//
//					case	PREQ_TEIKI_DATA1:	// �󎚏I��
//						ret = 0;
//						priend = 1;
//						break;
//
//					case	KEY_TEN_F3:			// F3���i���~�j
//
//						BUZPI();
//						FrmPrnStop.prn_kind = R_PRI;
//						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
//						ret = 0;
//						priend = 1;
//						break;
//				}
//			}
//			return(ret);
//
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ����L���^�����[�v�����g������\��                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_print( block )                                  |*/
///*| PARAMETER    : char pos   : 0:�L�� 1:���� 2:�S��                       |*/
///*|              : ushort rev : 0:���] 1:���]                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passinv_print_dsp(char pos, ushort rev)
//{
//	if (pos == 0) {
//		grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[11]);		/* "�@�@�L���@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(2, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "�@�@�@�@�̂݁@�@�@�@�@�@�@�@�@" */
//		grachr(2, 12, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�@�@�v�����g�@�@�@�@�@" */
//	}
//	else if (pos == 1) {
//		grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[12]);		/* "�@�@�����@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(3, 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "�@�@�@�@�̂݁@�@�@�@�@�@�@�@�@" */
//		grachr(3, 12, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�@�@�v�����g�@�@�@�@�@" */
//	}
//	else {
//		grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);		/* "�@�@�S�ā@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(4, 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�v�����g�@�@�@�@�@�@�@" */
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ����L���^�����@�f�[�^�`�F�b�N                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passinv_check( block )                                  |*/
///*| PARAMETER    : PAS_TBL before �O��l                                   |*/
///*| PARAMETER    : PAS_TBL after  ����l                                   |*/
///*| PARAMETER    : ushort index   �f�[�^�C���f�b�N�X                       |*/
///*| PARAMETER    : ushort parking_kind   ����敪                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void passinv_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind)
//{
//	if (before.BIT.INV != after.BIT.INV) {
//		if (after.BIT.INV) {
//			// ����
//			WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, 0xFFFF, 0xFFFFFFFF, 1 );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//							, (ulong)index
//							, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//							, after.BIT.PKN
//							, NTNET_PASSUPDATE_INVALID
//							, 0
//							, NULL );
//		} else {
//			// �L��
//			WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, 0, 0xFFFFFFFF, 0 );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//							, (ulong)index
//							, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//							, after.BIT.PKN
//							, NTNET_PASSUPDATE_INITIAL
//							, 0
//							, NULL );
//		}
//	}
//}
//
//unsigned short	UsMnt_PassEnter(void)
//{
//	s_ParkNo	index[PARK_MAX];
//	ushort		msg;
//	char		ret;
//	char		max_block;
//	char		i, j;
//	const uchar	*str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	char		dsp;
//	int			pos;
//	
//	/* �L�����ԏ�ԍ����擾 */
//	max_block = 0;
//	memset(index, 0, (PARK_MAX * 2));
//	for (i = 0; i < PARK_MAX; i++) {
//		pos = (char)((i == 0) ? 6 : i);
//		if (prm_get( COM_PRM,S_SYS,70,1,pos ) && (CPrmSS[S_SYS][i+1] != 0) ) {
//			for (j = 0; j < PARK_MAX; j++) {
//				if (CPrmSS[S_SYS][i+1] == CPrmSS[S_SYS][65+j]) {
//					index[max_block].no  = i;
//					index[max_block].mno = j;
//					max_block++;
//					break;
//				}
//			}
//		}
//	}
//
//	if (max_block == 0) {
//		dispclr();
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);	/* "��������Ɂ^�o�Ɂ��@�@�@�@�@�@" */
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	/* "�@�@�@�f�[�^�͂���܂���@�@�@" */
//		Fun_Dsp(FUNMSG[8]);					/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//		
//		for ( ; ; ) {
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:�I�� */
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//	else {
//		pos = 0;
//		dsp = 1;
//		for( ; ; ) {
//			if (dsp) {
//				/* ������ʕ\�� */
//				dispclr();
//				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);	/* "��������Ɂ^�o�Ɂ��@�@�@�@�@�@" */
//				for (i = 0; i < max_block; i++) {
//					grachr((ushort)(2+i), 4, 6, ((pos==i)?1:0), COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[i].no]);
//				}
//				if(max_block >= 2){
//					Fun_Dsp( FUNMSG2[13] );				/* "�@���@�@���@�����  �Ǐo  �I�� " */				
//				}
//				else{
//					Fun_Dsp( FUNMSG2[49] );				/* "�@�@�@�@�@�@�����  �Ǐo  �I�� " */				
//				}
//				dsp = 0;
//			}
//			msg = StoF( GetMessage(), 1 );
//			switch(msg){
//			case KEY_MODECHG:
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:			/* F5:�I�� */
//				BUZPI();
//				return MOD_EXT;
//			case KEY_TEN_F1:			/* F1:�� */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (--pos < 0) {
//						pos = (char)(max_block - 1);
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F2:			/* F2:�� */
//				if(max_block >= 2){
//					BUZPI();
//					grachr((ushort)(2+pos), 4, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//					if (++pos >= max_block) {
//						pos = 0;
//					}
//					grachr((ushort)(2+pos), 4, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[index[pos].no]);
//				}
//				break;
//			case KEY_TEN_F3:			/* F3:����� */
//				BUZPI();
//				/* �v�����g��ʂ� */
//				ret = passent_print(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			case KEY_TEN_F4:			/* F4:�Ǐo */
//				BUZPI();
//				/* �ݒ��ʂ� */
//				ret = passent_set(index[pos].mno, index[pos].no);
//				if (ret == 1) {
//					return MOD_CHG;
//				}
//				dsp = 1;
//				break;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ[�ݒ���                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_set( block )                                    |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//#define	PASSTS(b, o)	(pas_tbl[((b)+(o)-1)].BIT.STS)
//static char	passent_set(char block, char mode)
//{
//#define	DSP_COUNT_MAX	6			// �ő�\������
//
//	ushort	msg;
//	long	max;
//	char	ret;
//	long	begin;
//	long	end;
//	char	i;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//	const uchar	*str_tbl[] = {DAT2_6[13], DAT2_6[15], DAT2_6[14]};
//	long	input;
//	char	dsp;					// 1=�ĕ\���iID�͕ς�炷�X�e�[�^�X�\���̂ݍX�V�j���͍ŏ��̕\��
//									// 2=����ʕ\��
//									// 3=�O��ʕ\��
//									// 4=�擪ID����\��
//									// 5=����ID���Ō�ɕ\��
//	PAS_TBL pass_data;
//	char	DspCount;				// �\�������i0�`6�j
//	ushort	DspID[DSP_COUNT_MAX];	// �\��ID�i1�`12000�j
//	ushort	DspTop;					// �擪�ɕ\��������ID�i1�`12000�F�������f�O�j
//	char	CslPos;					// �J�[�\���\���s(0�`DSP_COUNT_MAX-1)
//	ushort	WkID, WkPrevID;			// work
//	WkID = WkPrevID = 0;
//
//	if (block == 0) {
//		begin = 1;
//		end = (CPrmSS[S_SYS][61] == 0) ? 12000 : CPrmSS[S_SYS][61]-1;
//	}
//	else if (block == 1) {
//		begin = CPrmSS[S_SYS][61];
//		end = (CPrmSS[S_SYS][62] == 0) ? 12000 : CPrmSS[S_SYS][62]-1;
//	}
//	else if (block == 2) {
//		begin = CPrmSS[S_SYS][62];
//		end = (CPrmSS[S_SYS][63] == 0) ? 12000 : CPrmSS[S_SYS][63]-1;
//	}
//	else {	/* if (block == 3) */
//		begin = CPrmSS[S_SYS][63];
//		end = 12000;
//	}
//	max = end-begin;
//
//	input = -1;
//	dsp = 4;			// �擪����\��
//
//	for( ; ; ) {
//		if (dsp) {
//			/* ������ʕ\�� */
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);			/* "��������Ɂ^�o�Ɂ��@�@�@�@�@�@" */
//			grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);	/* "�@�@�@�@�@�@�@�@�@�@�@�@�w�w�w" */
//
//			// �\���v���i���ŁC�O�ŁC�ĕ\���j�ɏ]�����ɐ擪�s�ɕ\������ID(DspTop)�����߂�
//			// ���łɃJ�[�\���ʒu(CslPos)���Z�b�g����
//
//passent_set_10:
//
//			if( 1 == dsp ){								// �ĕ\��
//				;										// �ĕ\���̏ꍇ�́@DspTop�l�̂܂�
//				;										// �J�[�\���ʒu���ς��Ȃ�
//			}
//			else if( 2 == dsp ){						// ����ʕ\���i�P�f�[�^�����ɂ��炵���\���j
//				if( DSP_COUNT_MAX == DspCount ){		// ���ݍő匏����\�����Ă��鎞�̂ݎ��y�[�W��ID�������s��
//					WkID = UsMnt_PassEnter_AfterValidIdSerach( (ushort)begin, (ushort)end, (ushort)(DspID[DSP_COUNT_MAX-1] + 1) );
//														// +�����ɗL��ID��T��
//					if( 0 == WkID ){					// �L����ID����
//						dsp = 4;						// �擪����\������
//						goto passent_set_10;
//					}
//					else{
//						DspTop = DspID[1];				// 2�s�ڂɕ\�����Ă���ID�����̐擪�Ƃ���
//						;								// �J�[�\���͖����s�̂܂�
//					}
//				}
//				else{									// �ő匏���\���Ŗ����ꍇ��
//					dsp = 4;							// �擪����\������
//					goto passent_set_10;
//				}
//			}
//			else if( 3 == dsp ){						// �O��ʕ\��
//				WkID = UsMnt_PassEnter_BeforeValidIdSerach( (ushort)begin, (ushort)end, (ushort)(DspID[0] - 1) );
//														// ���ݐ擪�ɕ\�����Ă���ID��1�O�̗L��ID�����߂�
//				if( 0 == WkID ){						// �O�͖����i���ɗL���Ȑ擪ID�j
//					dsp = 5;							// �������Ō�ɕ\��
//					goto passent_set_10;
//				}
//				else{									// 1�O�̗L��ID����
//					DspTop = WkID;						// ���擪�\��ID�Z�b�g
//				}
//				CslPos = 0;								// �J�[�\���͐擪�s
//			}
//			else if( 4 == dsp ){						// �擪����\���B�i�ŏ��ƍŏIID���玟�L�[�������j
//				DspTop = (ushort)begin;
//				CslPos = 0;								// �J�[�\���͐擪�s
//			}
//			else if( 5 == dsp ){						// �������Ō�ɕ\���B�i�擪ID����O�L�[�������j
//				WkID = (ushort)end;
//				for( i=0; i<DSP_COUNT_MAX; ++i ){		// ����ID����\��MAX�������̗L��ID��T��
//					WkID = UsMnt_PassEnter_BeforeValidIdSerach( (ushort)begin, (ushort)end, WkID );
//					if( 0 == WkID ){					// �O�ɗL����ID����
//						break;
//					}
//					WkPrevID = WkID;					// ���̂Ƃ���L����ID�i��ԂŁj
//					--WkID;
//				}
//				DspTop = WkPrevID;
//				CslPos = (char)(i - 1);					// �J�[�\���͖����s
//			}
//			dsp = 0;
//
//			// �\���擪ID����\������ID�i�ő� DSP_COUNT_MAX���j�� DspID[] �ɃZ�b�g����
//			WkID = DspTop;
//			for( i=0; i<DSP_COUNT_MAX; ++i ){			// �\��MAX������
//				WkID = UsMnt_PassEnter_AfterValidIdSerach( (ushort)begin, (ushort)end, WkID );
//														// +�����ɗL��ID��T��
//				if( 0 == WkID ){						// �L����ID����
//					break;
//				}
//				else{									// �L��ID����
//					DspID[i] = WkID;					// �\���e�[�u���ɃZ�b�g
//					++WkID;								// �������J�nID�Z�b�g
//				}
//			}
//			DspCount = i;								// �\�������Z�b�g
//
//			// �\��
//			for (i = 0; i < DspCount; i++) {
//				opedpl((ushort)(1+i), 2, (ulong)(DspID[i]-begin+1), 5, 1, (i==CslPos)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//				grachr((ushort)(1+i), 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);										/* "�|" */
//				grachr((ushort)(1+i), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[ pas_tbl[ DspID[i]-1 ].BIT.STS ]);		/* "����"or"�o��"or"����" */
//			}
//
//			Fun_Dsp(FUNMSG[17]);						/* "  �{  �|�^�� �ύX �S�ύX �I�� " */
//			dsp = 0;
//			if( 0 == DspCount ){						// 1�����\�����Ă��Ȃ�
//				pass_data.BYTE = 0;
//			}
//			else{
//				pass_data = pas_tbl[ DspID[CslPos]-1 ];
//			}
//		}
//
//		msg = StoF( GetMessage(), 1 );
//		switch (KEY_TEN0to9(msg)) {
//// MH810100(S)
//		case LCD_DISCONNECT:
//			return 2;
//// MH810100(E)
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:�I�� */
//			if( 0 != DspCount ){						// �\���f�[�^����
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:�{ */
//			if( 0 != DspCount ){						// �\���f�[�^����
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//			BUZPI();
//			if( 0 == CslPos ){							// �ŏ�s�ɃJ�[�\������
//				dsp = 3;								// �O�ŕ\���w��
//														// �����ւ̈ړ��������ɔC����
//			}
//			else{
//				// �ԍ��\���̃J�[�\���݈̂ړ�
//				opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ��J�[�\������ */
//				--CslPos;
//				opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ��J�[�\���_�� */
//
//				input = -1;								// Jump�o�^����
//				pass_data = pas_tbl[ DspID[CslPos]-1 ];	// �ύX�O�X�e�[�^�X�ۑ�
//			}
//// MH322914 (s) kasiyama 2016/07/07 �ŉ��i�Ő��l����->�u+�v�L�[��u-/�ǁv�Ő��l���c���Ă��邽�߼ެ��߂��Ă��܂�(���ʉ��PNo.993)(MH341106)
//			input = -1;
//// MH322914 (e) kasiyama 2016/07/07 �ŉ��i�Ő��l����->�u+�v�L�[��u-/�ǁv�Ő��l���c���Ă��邽�߼ެ��߂��Ă��܂�(���ʉ��PNo.993)(MH341106)
//			break;
//		case KEY_TEN_F2:		/* F2:�|�^�� */
//			if( 0 != DspCount ){						// �\���f�[�^����
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//
//			if (input == -1) {
//				BUZPI();
//				if( (DspCount-1) <= CslPos ){				// �ŉ��s�ɃJ�[�\������
//						dsp = 2;							// ���y�[�W�\���w��
//															// �擪�y�[�W����̕\���������ɔC����
//				}
//				else{
//					// �ԍ��\���̃J�[�\���݈̂ړ�
//					opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF );	/* �ԍ��J�[�\������ */
//					++CslPos;
//					opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* �ԍ��J�[�\���_�� */
//
//					input = -1;								// Jump�o�^����
//					pass_data = pas_tbl[ DspID[CslPos]-1 ];	// �ύX�O�X�e�[�^�X�ۑ�
//				}
//			}
//			else {
//				if (input == 0 || input > max+1) {
//					BUZPIPI();
//				}
//				else if( pas_tbl[ begin+input-2 ].BIT.INV ){	// �������
//					BUZPIPI();
//				}
//				else {
//					BUZPI();
//					DspTop = (ushort)(begin+input-1);		// �\���擪ID�X�V
//					dsp = 1;								// �ĕ\���w��
//					CslPos = 0;								// �J�[�\���͐擪
//				}
//			}
//			input = -1;
//			break;
//		case KEY_TEN_F3:		/* F3:�ύX */
//			if( 0 == DspCount ){						// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//			BUZPI();
//			if( pas_tbl[ DspID[CslPos]-1 ].BIT.STS >= 2 ){
//				pas_tbl[ DspID[CslPos]-1 ].BIT.STS = 0;
//			}
//			else{
//				++(pas_tbl[ DspID[CslPos]-1 ].BIT.STS);
//			}
//			grachr((ushort)(1+CslPos), 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, str_tbl[ pas_tbl[ DspID[CslPos]-1 ].BIT.STS ]);	/* "����"or"�o��"or"����" */
//
//			OpelogNo = OPLOG_TEIKIINOUT;		// ���엚��o�^
//			input = -1;
//			break;
//		case KEY_TEN_F4:		/* F4:�S�ύX */
//			if( 0 != DspCount ){						// �\���f�[�^����
//				passent_check( pass_data, pas_tbl[ DspID[CslPos]-1], (ushort)(DspID[CslPos]-begin+1), (ushort)block );
//			}
//			else{										// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//			BUZPI();
//			ret = passent_allset( mode, block );
//			if (ret == 1) {
//				return 1;
//			}
//			dsp = 1;
//			input = -1;
//			break;
//		case KEY_TEN:			/* ����(�e���L�[) */
//			if( 0 == DspCount ){						// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//			BUZPI();
//			if (input == -1) {
//				input = 0;
//			}
//			input = (input*10 + msg-KEY_TEN0) % 100000L;
//			opedpl((ushort)(1+CslPos), 2, (ulong)input, 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//			break;
//		case KEY_TEN_CL:		/* ���(�e���L�[) */
//			if( 0 == DspCount ){						// �\���f�[�^�Ȃ�
//				break;									// �L�[�N���b�N�����炳�Ȃ�
//			}
//			BUZPI();
//			opedpl((ushort)(1+CslPos), 2, (ulong)(DspID[CslPos]-begin+1), 5, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
//			input = -1;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ|���L�����ID�擾                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassEnter_BeforeValidIdSerach()                   |*/
///*| PARAMETER    : TopID    = ���ԏ���̐擪ID (1�`12000)                  |*/
///*|				   BottomID = ���ԏ���̍ŏIID (1�`12000)                  |*/
///*|				   TargetID = �ID (1�`12000)							   |*/
///*|							  �ID���O�����Ŏ��ɗL����ID��T���B	   |*/
///*|							�@�i�ID�������Ώۂł��j					   |*/
///*| RETURN VALUE : 0 = �f�[�^����                                          |*/
///*|              : 1�`12000 = ���̗L��ID                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
//ushort	UsMnt_PassEnter_BeforeValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID )
//{
//	ushort ret=0;
//
//	if( (TargetID < TopID) || (BottomID < TargetID) ){	// ���ԏ������͈̔͊O
//		return	ret;									// �f�[�^����
//	}
//
//	for( ; TopID <= TargetID; --TargetID ){				// ���ԏ���̎g�p���ID��(1�`12000)
//		if( pas_tbl[ TargetID-1 ].BIT.INV ){			// �������
//			continue;
//		}
//		else{											// �L�����
//			ret = TargetID;
//			break;
//		}
//	}
//	return	ret;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ|���L�����ID�擾                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassEnter_AfterValidIdSerach()                    |*/
///*| PARAMETER    : TopID    = ���ԏ���̐擪ID (1�`12000)                  |*/
///*|				   BottomID = ���ԏ���̍ŏIID (1�`12000)                  |*/
///*|				   TargetID = �ID (0�`12000)							   |*/
///*|							  �ID��������Ŏ��ɗL����ID��T��		   |*/
///*|							�@�i�ID�������Ώۂł��j					   |*/
///*| RETURN VALUE : 0 = �f�[�^����                                          |*/
///*|              : 1�`12000 = ���̗L��ID                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
//ushort	UsMnt_PassEnter_AfterValidIdSerach( ushort TopID, ushort BottomID, ushort TargetID )
//{
//	ushort ret=0;
//
//	if( (TargetID < TopID) || (BottomID < TargetID) ){	// ���ԏ������͈̔͊O
//		return	ret;									// �f�[�^����
//	}
//
//	for( ; TargetID <= BottomID; ++TargetID ){			// ���ԏ���̎g�p���ID��(1�`12000)
//		if( pas_tbl[ TargetID-1 ].BIT.INV ){			// �������
//			continue;
//		}
//		else{											// �L�����
//			ret = TargetID;
//			break;
//		}
//	}
//	return	ret;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ[�S�ݒ���                                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_allset( mode, block )                           |*/
///*| PARAMETER    : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*|                char block : block number                               |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passent_allset( char mode, char block )
//{
//	ushort	msg;
//	const uchar	*mode_str_tbl[] = {DAT3_3[8], DAT3_3[9], DAT3_3[10], DAT3_3[11]};
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36]);		/* "��������Ɂ^�o�Ɂ��@�@�@�@�@�@" */
//	grachr(0, 24, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, mode_str_tbl[mode]);/* "�@�@�@�@�@�@�@�@�@�@�@�@�w�w�w" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[39]);		/* "�S�_��No��������Ԃɂ��܂����H" */
//	Fun_Dsp(FUNMSG[19]);													/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
//
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F3:		/* F3:�͂� */
//			BUZPI();
//			/* �S�ݒ�ύX */
//			FillPassTbl( (ulong)(CPrmSS[S_SYS][65 + block]), 0, 0xFFFFFFFF, 0, PASS_DATA_FILL_VALID );
//			NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//								, 0
//								, (ulong)(CPrmSS[S_SYS][65 + block])
//								, 0
//								, NTNET_PASSUPDATE_ALL_INITIAL2
//								, 0
//								, NULL );
//			wopelg( OPLOG_TEIKIZENSHOKI, 0, 0 );		// ���엚��o�^
//			return 0;
//		case KEY_TEN_F4:		/* F4:������ */
//			BUZPI();
//			return 0;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ[�v�����g���                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_print( block )                                  |*/
///*| PARAMETER    : char block : block number                               |*/
///*|              : char mode  : 0:��{ 1:�g��1 2:�g��2 3:�g��3             |*/
///*| RETURN VALUE : 0 : exit                                                |*/
///*|              : 1 : mode change                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static char	passent_print(char block, char mode)
//{
//	ushort			msg;
//	int				pos;		/* 0:���ɂ̂� 1:�o�ɂ̂� 2:�S�� */
//	T_FrmTeikiData2	pri_data;
//	uchar			priend;
//	char			ret;
//	T_FrmPrnStop	FrmPrnStop;
//
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[36] );		/* "��������Ɂ^�o�Ɂ��@�@�@�@�@�@" */
//	passent_print_dsp(0, 1);						/* "�@�@���ɒ��̂݃v�����g�@�@�@�@" */
//	passent_print_dsp(1, 0);				/* "�@�@�o�ɒ��̂݃v�����g�@�@�@�@" */
//	passent_print_dsp(2, 0);				/* "�@�@�S�ăv�����g�@�@�@�@�@�@�@" */
//	Fun_Dsp(FUNMSG[68]);					/* "�@���@�@���@�@�@�@ ���s  �I�� " */
//
//	pos = 0;
//	for( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch(msg){
//		case KEY_MODECHG:
//			BUZPI();
//			return 1;
//		case KEY_TEN_F5:		/* F5:�I�� */
//			BUZPI();
//			return 0;
//		case KEY_TEN_F1:		/* F1:�� */
//		case KEY_TEN_F2:		/* F2:�� */
//			BUZPI();
//			passent_print_dsp(pos, 0);
//			if (msg == KEY_TEN_F1) {
//				if (--pos < 0)
//					pos = 2;
//			}
//			else {
//				if (++pos > 2)
//					pos = 0;
//			}
//			passent_print_dsp(pos, 1);
//			break;
//		case KEY_TEN_F4:		/* F4:���s */
//			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//				BUZPIPI();
//				break;
//			}
//			BUZPI();
//			/* �v�����g���s */
//			pri_data.prn_kind = R_PRI;
//			pri_data.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			pri_data.Kakari_no = OPECTL.Kakari_Num;
//			pri_data.Pkno_syu = (uchar)mode;
//			if (pos == 0)		/* ���ɂ̂� */
//				pri_data.Req_syu = 1;
//			else if (pos == 1)	/* �o�ɂ̂� */
//				pri_data.Req_syu = 2;
//			else				/* �S�� */
//				pri_data.Req_syu = 0;
//			queset(PRNTCBNO, PREQ_TEIKI_DATA2, sizeof(T_FrmTeikiData2), &pri_data);
//			Ope_DisableDoorKnobChime();
//
//			/* �v�����g�I����҂����킹�� */
//			Fun_Dsp(FUNMSG[82]);				// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
//
//			for ( priend = 0 ; priend == 0 ; ) {
//
//				msg = StoF( GetMessage(), 1 );
//
//				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
//					msg &= (~INNJI_ENDMASK);
//				}
//				switch( msg ){
//
//					case	KEY_MODECHG:		// Ӱ����ݼ�
//						BUZPI();
//						ret = 1;
//						priend = 1;
//						break;
//
//					case	PREQ_TEIKI_DATA2:	// �󎚏I��
//						ret = 0;
//						priend = 1;
//						break;
//
//					case	KEY_TEN_F3:			// F3���i���~�j
//
//						BUZPI();
//						FrmPrnStop.prn_kind = R_PRI;
//						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
//						ret = 0;
//						priend = 1;
//						break;
//				}
//			}
//			return(ret);
//
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ[�v�����g������\��                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_print( block )                                  |*/
///*| PARAMETER    : char pos   : 0:���� 1:�o�� 2:�S��                       |*/
///*|              : ushort rev : 0:���] 1:���]                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passent_print_dsp(char pos, ushort rev)
//{
//	if (pos == 0) {
//		grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);		/* "�@�@���Ɂ@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(2, 8, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "�@�@�@�@���@�@�@�@�@�@�@�@�@�@" */
//		grachr(2, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "�@�@�@�@�@�̂݁@�@�@�@�@�@�@�@" */
//		grachr(2, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�@�@�@�v�����g�@�@�@�@" */
//	}
//	else if (pos == 1) {
//		grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);		/* "�@�@�o�Ɂ@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(3, 8, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "�@�@�@�@���@�@�@�@�@�@�@�@�@�@" */
//		grachr(3, 10, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[16]);		/* "�@�@�@�@�@�̂݁@�@�@�@�@�@�@�@" */
//		grachr(3, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�@�@�@�v�����g�@�@�@�@" */
//	}
//	else {
//		grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);		/* "�@�@�S�ā@�@�@�@�@�@�@�@�@�@�@" */
//		grachr(4, 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[19]);		/* "�@�@�@�@�v�����g�@�@�@�@�@�@�@" */
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������Ɂ^�o�Ɂ@�f�[�^�`�F�b�N                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : passent_check( block )                                  |*/
///*| PARAMETER    : PAS_TBL before �O��l                                   |*/
///*| PARAMETER    : PAS_TBL after  ����l                                   |*/
///*| PARAMETER    : ushort index   �f�[�^�C���f�b�N�X                       |*/
///*| PARAMETER    : ushort parking_kind   ����敪                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	passent_check(PAS_TBL before, PAS_TBL after, ushort index, ushort parking_kind)
//{
//	if (before.BIT.STS != after.BIT.STS) {
//		WritePassTbl( (ulong)CPrmSS[S_SYS][65 +  parking_kind], index, (ushort)after.BIT.STS, (ulong)CPrmSS[S_SYS][1], 0 );
//		NTNET_Snd_Data116( NTNET_PASSUPDATE_STATUS
//						, (ulong)index
//						, (ulong)(CPrmSS[S_SYS][65 +  parking_kind])
//						, after.BIT.PKN
//						, after.BIT.STS
//						, 0
//						, NULL );
//	}
//}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F������`�F�b�N                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassCheck( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassCheck(void)
{
	ushort	msg;
	char	pos;

	pos = (char)(PPrmSS[S_P01][3] & 1);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[40] );					/* "��������`�F�b�N���@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[12] );					/* "�@���݂̐ݒ�@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );					/* "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@" */
	if (pos == 0)
		grachr( 2, 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );
	else
		grachr( 2, 16,  6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );
	grachr( 4, 15,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );	/* "�@�@�@�@�@�@�@ ���� �@�@�@�@�@" */
	grachr( 5, 14,  6, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );		/* "�@�@�@�@�@�@�@���Ȃ��@�@�@�@�@" */
	if(prm_get(0, S_NTN, 38, 1, 2) == 2){
		Fun_Dsp( FUNMSG[87] );								/* "�@���@�@���@ ����  ����  �I�� " */
	}else{
		Fun_Dsp( FUNMSG[20] );								/* "�@���@�@���@�@�@�@ ����  �I�� " */
	}

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
				pos ^= 1;
				grachr( 4, 15,  4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );	/* "�@�@�@�@�@�@�@ ���� �@�@�@�@�@" */
				grachr( 5, 14,  6, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );		/* "�@�@�@�@�@�@�@���Ȃ��@�@�@�@�@" */
				break;
			case KEY_TEN_F3:					/* F3:���� */
				if(prm_get(0, S_NTN, 38, 1, 2) != 2){
					break;
				}
			case KEY_TEN_F4:					/* F4:���� */
				BUZPI();
				PPrmSS[S_P01][3] = (long)pos;
				if((prm_get(0, S_NTN, 38, 1, 2) == 1) || (msg == KEY_TEN_F3)){
					NTNET_Snd_Data100();			//����f�[�^���M(����߽)
				}
				if( pos == 0 ){
					wopelg( OPLOG_TEIKICHKSURU, 0, 0 );		// ���엚��o�^
				}else{
					wopelg( OPLOG_TEIKICHKSHINAI, 0, 0 );		// ���엚��o�^
				}
				if (pos == 0) {
					grachr( 2, 16,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[10] );
					grachr( 2, 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]  );
				}
				else
					grachr( 2, 16,  6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_2[0] );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���ʓ��^���ʊ���                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_SplDay( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_SplDay(void)
{
	ushort			msg;
	int				pos;
	T_FrmSplDay		SplDay;
	char			ret;
	char			dsp;

	pos = 0;
	dsp = 1;
	for( ; ; ) {
		if (dsp) {
			/* ������ʕ\�� */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[0]);		/* "�����ʓ����@�@�@�@�@�@�@�@�@�@" */
			splday_dsp(0, ((pos==0)?1:0));		/* "�@���ʓ��@�@�@�@�@�@�@�@�@�@�@" */
			splday_dsp(1, ((pos==1)?1:0));		/* "�@���ʊ��ԁ@�@�@�@�@�@�@�@�@�@" */
			splday_dsp(2, ((pos==2)?1:0));		/* "�@�n�b�s�[�}���f�[�@�@�@�@�@�@" */
												/* OR                               */
												/* "�@���ʗj���@�@�@�@�@�@�@�@�@�@" */
			Fun_Dsp(FUNMSG2[13]);				/* "�@���@�@���@�����  �Ǐo  �I�� " */
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:		/* ��(F1) */
		case KEY_TEN_F2:		/* ��(F2) */
			BUZPI();
			splday_dsp(pos, 0);		/* �O��I������-���] */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0){
					pos = 2;
				}
			}
			else {
				if (++pos > 2){
					pos = 0;
				}
			}
			splday_dsp(pos, 1);		/* ����I������-���] */
			break;
		case KEY_TEN_F3:		/* �����(F3) */
			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
				BUZPIPI();
				break;
			}
			BUZPI();
			SplDay.Kikai_no = (uchar)CPrmSS[S_PAY][2];							// �@�B��
			SplDay.Kakari_no = OPECTL.Kakari_Num;								// �W��No.
			SplDay.prn_kind = R_PRI;											// ��������
			queset(PRNTCBNO, PREQ_SPLDAY, sizeof(T_FrmSplDay), &SplDay);		// ���ʓ��󎚗v��
			Ope_DisableDoorKnobChime();

			/* �v�����g�I����҂����킹�� */
			Lagtim(OPETCBNO, 6, 500);		/* 10sec timer start */
			for ( ; ; ) {
				msg = StoF( GetMessage(), 1 );
				/* �v�����g�I�� */
				if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
					Lagcan(OPETCBNO, 6);
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u��Ԃł���΃����e�i���X�I��
					if (CP_MODECHG) {
						return MOD_CHG;
					}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
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
				if ( msg == LCD_DISCONNECT ) {
					Lagcan(OPETCBNO, 6);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			break;
		case KEY_TEN_F4:		/* �Ǐo(F4) */
			BUZPI();
			OpelogNo = 0;
			splday_check();

			switch(pos){
				case 0:
					ret = splday_day();			/* "�@���ʓ��@�@�@�@�@�@�@�@�@�@�@" */
					break;
				case 1:
					ret = splday_period();		/* "�@���ʊ��ԁ@�@�@�@�@�@�@�@�@�@" */
					break;				
				default:
					if((prm_get( 0,S_TOK,(short)(41),1,1 )) == 1 ){
						ret = splday_week();		// ���ʗj��
					}
					else{
						ret = splday_monday();		// �n�b�s�[�}���f�[
					}
					break;
			}

			splday_check();

			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
				OpelogNo = 0;
				SetChange = 1;			// FLASH���ގw��
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	//FTP�t���O�Z�b�g
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}

			if (ret == 1){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (ret == 2){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

//--------------------------------------------------------------------------
// ���͓��t�̃`�F�b�N���s���A���݂̂̓��͂Ȃǂ͍폜����B
//--------------------------------------------------------------------------
static void splday_check( void )
{
	int	i;

	ulong	lData;
	ulong	sData1, sData2;
	ulong	sData3;

	// ���ʓ��`�F�b�N
	for( i = 0; i < SP_DAY_MAX; i++ ){
		lData = CPrmSS[S_TOK][9+i];
		sData1 = lData % 100;
		sData2 = (lData % 10000) / 100;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][9+i] = 0;
		}
	}

	// ���ʊ��ԃ`�F�b�N
	for( i = 0; i < SP_RANGE_MAX*2; i++ ){
		lData = CPrmSS[S_TOK][1+i] % 10000;
		sData1 = lData % 100;
		sData2 = (lData % 10000) / 100;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][1+i] = 0;
		}
	}
	// �J�n���E�I�������΂Őݒ肳��Ă��邩�`�F�b�N
	for( i = 0; i < SP_RANGE_MAX; i++ ){
		if (CPrmSS[S_TOK][i*2+1] && !CPrmSS[S_TOK][i*2+2]) {
			CPrmSS[S_TOK][i*2+1] = 0;
		}
		if (!CPrmSS[S_TOK][i*2+1] && CPrmSS[S_TOK][i*2+2]) {
			CPrmSS[S_TOK][i*2+2] = 0;
		}
	}

	for( i = 0; i < (SP_HAPPY_MAX/2); i++ ){
		lData = CPrmSS[S_TOK][42+i] / 1000;
		sData1 = lData / 10;
		sData2 = lData % 10;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][42+i] %= 1000;
		}

		lData = CPrmSS[S_TOK][42+i] % 1000;
		sData1 = lData / 10;
		sData2 = lData % 10;
		if( !sData1 || !sData2 ){
			CPrmSS[S_TOK][42+i] = CPrmSS[S_TOK][42+i] / 1000 * 1000;
		}
	}

	// ���ʗj���`�F�b�N
	for( i = 0; i < SP_WEEK_MAX; i++ ){
		lData = CPrmSS[S_TOK][56+i];	// 04-0056�`
		sData1 = ((lData % 10000 ) /100);	// �B�C�F��
		sData2 = ((lData % 100) / 10);		// �@�D�F�T
											// �@�E�F�j��
		sData3 = lData % 10;				// �@�E�F�j��

		if (!sData1 || !sData2 || !sData3) {
			CPrmSS[S_TOK][56+i] = 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��^���ʊ��ԕ\��                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_dsp( pos, rev )                                  |*/
/*| PARAMETER    : char pos   : 0:���ʓ� 1:���ʊ��� 2:�n�b�s�[�}���f�[     |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_dsp(char pos, ushort rev)
{
	switch(pos){
		case 0:
			grachr(2, 4, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);		/* "�@���ʓ��@�@�@�@�@�@�@�@�@�@�@" */
			break;
		case 1:
			grachr(3, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[1]);		/* "�@���ʊ��ԁ@�@�@�@�@�@�@�@�@�@" */
			break;
		default:
			if((prm_get( 0,S_TOK,(short)(41),1,1 )) == 1 ){
				grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[3]);		/* "�@���ʗj���@�@�@�@�@�@�@�@�@�@" */
			}else{
				grachr(4, 4, 16, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[2]);		/* "�@�n�b�s�[�}���f�[�@�@�@�@�@�@" */
			}
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��^���ʊ��ԁ[���ʓ��ݒ�                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_day( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_day(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-30 */
	long	*pSPD;
	char	dsp;
	char	mode;	/* 0:�ʏ� 1:��� */
	char	pos;	/* 0:�� 1:�� 2:�ؑ� */
	long	input;

	pSPD = &CPrmSS[S_TOK][9];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* ��ʕ\�� */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[0]);								/* "�����ʓ����@�@�@�@�@�@�@�@�@�@" */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[10]);			/* "�@���ʓ��@�@�F�@�@���@�@���@�@" */
				opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
				splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);									/* �� */
				splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);									/* �� */
				splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);									/* �U�� */
			}
			if (mode == 0) {
				grachr(1, 26, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[17]);	/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�U��" */
				splday_day_dsp(top, no, 0, pSPD[no], 1);							/* �� ���] */
				Fun_Dsp( FUNMSG[39] );												/* "�@�{�@�@�|�@ ���  ����  �I�� " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);		/* "�@������ԍ���I�����ĉ������@" */
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);			/* "���ʓ�" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� ���] */
				Fun_Dsp( FUNMSG[41] );												/* "�@�{�@�@�|�@ ��� �S��� �I�� " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	
		/* �ʏ��� */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* �{(F1) */
				/* �U�� */
				if (pos == 2) {
					BUZPI();
					input = 1;
					splday_day_dsp(top, no, pos, input*100000L, 1);
				}
				/* ���E�� */
				else {
					if (pos == 1) {
						Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
					}
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// �������m��Ŏ��̍s�ɑJ�ځF���ݍs�̃f�[�^���N���A
					}
					BUZPI();
					no--;
					if (no < 0) {
						top = 26;
						no = 30;
					}
					else if (no < top)
						top--;
					input = -1;
					/* �\�� */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* �U�� */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* �� ���] */
				}
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				/* �U�� */
				if (pos == 2) {
					BUZPI();
					input = 0;
					splday_day_dsp(top, no, pos, input*100000L, 1);
				}
				/* ���E�� */
				else {
					if (pos == 1) {
						pos = 0;
						Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
					}
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// �������m��őO�̍s�ɑJ�ځF���ݍs�̃f�[�^���N���A
					}
					BUZPI();
					no++;
					if (no > 30)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* �\�� */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* �U�� */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* �� ���] */
				}
				break;
			case KEY_TEN_F3:		/* ���(F3) */
				BUZPI();
				if (pos != 0) {		// BACK���F�O�̍��ڂɖ߂�
					if (pos == 1) {
						Fun_Dsp( FUNMSG[39] );				// "�@�{�@�@�|�@ ���  ����  �I�� "
					}
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos--;
					input = -1;
					splday_day_dsp(top, no, pos, pSPD[no], 1);
				}
				else {				// ������F���Ӱ�ނɑJ��
					if (!GET_DAY(pSPD[no])) {
						pSPD[no] = 0;			// �������m��Ŏ��̍s�ɑJ�ځF���ݍs�̃f�[�^���N���A
					}
					pos = 0;
					mode = 1;
					dsp = 1;
					OpelogNo = OPLOG_TOKUBETUBI;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				}
				break;
			case KEY_TEN_F4:		/* ����(F4) */
				if (pos == 0) {
					if (input == -1)
						input = GET_MONTH(pSPD[no]);
					if (input < 1 || input > 12) {
						BUZPIPI();
						input = -1;
						splday_day_dsp(top, no, pos, pSPD[no], 1);
						break;
					}
					BUZPI();
					// �����̐������`�F�b�N
					if (GET_DAY(pSPD[no]) > medget(2004, (short)input))
						pSPD[no] = pSPD[no]/100000L*100000L + input*100;
					else
						pSPD[no] = pSPD[no]/100000L*100000L + input*100 + pSPD[no]%100;
					OpelogNo = OPLOG_TOKUBETUBI;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos = 1;
					input = -1;
					splday_day_dsp(top, no, pos, pSPD[no], 1);
					Fun_Dsp( FUNMSG[30] );				/* "  �{    �|    ��   ����  �I�� " */
				}
				else if (pos == 1) {
					if (input == -1)
						input = GET_DAY(pSPD[no]);
					if (input < 1 || input > medget(2004, (short)GET_MONTH(pSPD[no]))) {	/* 2004�͉[�N */
						BUZPIPI();
						input = -1;
						splday_day_dsp(top, no, pos, pSPD[no], 1);
						break;
					}
					BUZPI();
					pSPD[no] = pSPD[no]/100*100 + input;
					OpelogNo = OPLOG_TOKUBETUBI;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_day_dsp(top, no, pos, pSPD[no], 0);
					pos = 2;
					input = (short)(pSPD[no]/100000L);
					splday_day_dsp(top, no, pos, pSPD[no], 1);
				}
				else {	/* if (pos == 2) */
					BUZPI();
					pSPD[no] = input*100000L + pSPD[no]%100000L;
					OpelogNo = OPLOG_TOKUBETUBI;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					no++;
					if (no > 30)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* �\�� */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
						splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);		/* �� */
						splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);		/* �U�� */
					}
					splday_day_dsp(top, no, 0, pSPD[no], 1);		/* �� ���] */
					Fun_Dsp( FUNMSG[39] );				/* "�@�{�@�@�|�@ ���  ����  �I�� " */
				}
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				BUZPI();
				if (pos == 2)
					break;
				if (input == -1)
					input = 0;
				input = (input*10 + msg-KEY_TEN0) % 100;
				if (pos == 0)
					splday_day_dsp(top, no, pos, input*100, 1);
				else	/* if (pos == 1) */
					splday_day_dsp(top, no, pos, input, 1);
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				if (pos == 2)
					input = (short)(pSPD[no]/100000L);
				else
					input = -1;
				splday_day_dsp(top, no, pos, pSPD[no], 1);
				break;
			default:
				break;
			}
		}
		/* ������ */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���(F3) */
				pSPD[no] = 0;
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* �{(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = 26;
					no = 30;
				}
				else if (no < top)
					top--;
				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "���ʓ�" */
					opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
					splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);										/* �� */
					splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);										/* �� */
					splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);										/* �U�� */
				}
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "���ʓ�" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� ���] */
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				BUZPI();
				no++;
				if (no > 30)
					top = no = 0;
				else if (no > top+4)
					top++;
				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "���ʓ�" */
					opedsp((ushort)(2+i), 8, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
					splday_day_dsp(top, (char)(top+i), 0, pSPD[top+i], 0);										/* �� */
					splday_day_dsp(top, (char)(top+i), 1, pSPD[top+i], 0);										/* �� */
					splday_day_dsp(top, (char)(top+i), 2, pSPD[top+i], 0);										/* �U�� */
				}
				grachr((ushort)(2+no-top), 2, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "���ʓ�" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� ���] */
				break;
			case KEY_TEN_F4:		/* �S���(F4) */
				BUZPI();
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[7]);		/* "�@�S�Ă̓��ʓ���������܂����H" */
				grachr((ushort)(2+no-top), 2, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_0[0]);					/* "���ʓ�" */
				opedsp((ushort)(2+no-top), 8, (ushort)(no+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
				Fun_Dsp( FUNMSG[19] );												/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
				for ( ; ; ) {
					msg = StoF( GetMessage(), 1 );
					if (msg == KEY_MODECHG) {
						BUZPI();
						return 1;
					}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X)
					if (msg == LCD_DISCONNECT) {
						return 2;
					}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	
					if (msg == KEY_TEN_F3) {		/* �͂�(F3) */
						BUZPI();
						for (i = 0; i < SP_DAY_MAX; i++) {
							pSPD[i] = 0;
						}
						break;
					}
					else if (msg == KEY_TEN_F4) {	/* ������(F4) */
						BUZPI();
						break;
					}
				}
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��\��                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_day_dsp( top, no, pos, data, rev )               |*/
/*| PARAMETER    : char top   : �擪�ԍ�                                   |*/
/*|              : char no    : �ԍ�                                       |*/
/*|              : char pos   : �ʒu(0:�� 1:�� 2:�U��)                     |*/
/*|              : long data  : �f�[�^                                     |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_day_dsp(char top, char no, char pos, long data, ushort rev)
{
	if (pos == 0) {
		if (GET_MONTH(data))
			opedsp((ushort)(2+no-top), 14, (ushort)GET_MONTH(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );	/* �� */
		else
			grachr((ushort)(2+no-top), 14, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);						/* "�@�@" */
	}
	else if (pos == 1) {
		if (GET_DAY(data))
			opedsp((ushort)(2+no-top), 20, (ushort)GET_DAY(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
		else
			grachr((ushort)(2+no-top), 20, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "�@�@" */
	}
	else {		/* if (pos == 2) */
		if (data/100000L)	/* �V�t�g���� */
			grachr( (ushort)(2+no-top), 27,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[0] );				/* �� */
		else		/* �V�t�g���Ȃ� */
			grachr( (ushort)(2+no-top), 27,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[2] );				/* �~ */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��^���ʊ��ԁ[���ʊ��Ԑݒ�                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_period( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_period(void)
{
	ushort	msg;
	char	i;
	long	input;
	int		no;
	char	pos;
	char	mode;
	long	*pSPP;

	pSPP = &CPrmSS[S_TOK][1];

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[1] );		/* "�����ʊ��ԁ��@�@�@�@�@�@�@�@�@" */
	for (i = 0; i < 3; i++) {
		grachr((ushort)(1+i*2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[11]);			/* "�@���ԁ@�J�n���F�@�@���@�@���@" */
		opedsp((ushort)(1+i*2), 6, (ushort)(i+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
		splday_period_dsp((char)(i*2), 0, pSPP[i*2], 0);		/* �� */
		splday_period_dsp((char)(i*2), 1, pSPP[i*2], 0);		/* �� */

		grachr((ushort)(2+i*2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[12]);			/* "�@�@�@�@�I�����F�@�@���@�@���@" */
		splday_period_dsp((char)(i*2+1), 0, pSPP[i*2+1], 0);	/* �� */
		splday_period_dsp((char)(i*2+1), 1, pSPP[i*2+1], 0);	/* �� */
	}
	splday_period_dsp(0, 0, pSPP[0], 1);						/* �� ���] */
	Fun_Dsp( FUNMSG[39] );										/* "�@�{�@�@�|�@ ���  ����  �I�� " */

	pos = 0;
	no = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		/* �ʏ��� */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* ��(F1) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
				}
				if (!GET_DAY(pSPP[no])) {
					pSPP[no] = 0;			// ���ɗL���f�[�^���Ȃ��ꍇ���ݍs�̃f�[�^���N���A
					splday_period_dsp(no, 0, 0, 0);
				}
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 0);
				if (--no < 0)
					no = 5;
				pos = 0;
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			case KEY_TEN_F2:		/* ��(F2) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "					
				}
				if (!GET_DAY(pSPP[no])) {
					pSPP[no] = 0;		// ���ɗL���f�[�^���Ȃ��ꍇ���ݍs�̃f�[�^���N���A
					splday_period_dsp(no, 0, 0, 0);
				}
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 0);
				if (++no > 5)
					no = 0;
				pos = 0;
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			case KEY_TEN_F3:		/* ���(F3) */
				BUZPI();
				if (pos == 0) {		// ������F���Ӱ�ނɑJ��
					if (!GET_DAY(pSPP[no])) {
						pSPP[no] = 0;			// ���ɗL���f�[�^���Ȃ��ꍇ���ݍs�̃f�[�^���N���A
					}
					splday_period_dsp(no, pos, pSPP[no], 0);
					mode = 1;
					no = 0;
					grachr((ushort)(1+no*2), 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "����" */
					opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
					Fun_Dsp(FUNMSG2[14]);											/* "�@�{�@�@�|�@ ���        �I�� " */
					OpelogNo = OPLOG_TOKUBETUKIKAN;									// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				}
				else {				// BACK���F���ɖ߂�
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos--;
					input = -1;
					splday_period_dsp(no, pos, pSPP[no], 1);
					Fun_Dsp( FUNMSG[39] );				// "�@�{�@�@�|�@ ���  ����  �I�� "
				}
				break;
			case KEY_TEN_F4:		/* ����(F4) */
				if (pos == 0) {
					if (input == -1)
						input = GET_MONTH(pSPP[no]);
					if (input < 1 || input > 12) {
						BUZPIPI();
						splday_period_dsp(no, pos, pSPP[no], 1);
						input = -1;
						break;
					}
					BUZPI();
					// �����̐������`�F�b�N
					if (GET_DAY(pSPP[no]) > medget(2004, (short)input))
						pSPP[no] = input*100;
					else
						pSPP[no] = input*100 + pSPP[no]%100;
					OpelogNo = OPLOG_TOKUBETUKIKAN;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos = 1;
					splday_period_dsp(no, pos, pSPP[no], 1);
					input = -1;
					Fun_Dsp( FUNMSG[30] );				/* "  �{    �|    ��   ����  �I�� " */
				}
				else {	/* if (pos == 1) */
					if (input == -1)
						input = GET_DAY(pSPP[no]);
					if (input < 1 || input > medget(2004, (short)GET_MONTH(pSPP[no]))) {	/* 2004�͉[�N */
						BUZPIPI();
						splday_period_dsp(no, pos, pSPP[no], 1);
						input = -1;
						break;
					}
					BUZPI();
					pSPP[no] = pSPP[no]/100*100 + input;
					OpelogNo = OPLOG_TOKUBETUKIKAN;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_period_dsp(no, pos, pSPP[no], 0);
					pos = 0;
					if (++no > 5)
						no = 0;
					splday_period_dsp(no, pos, pSPP[no], 1);
					input = -1;
					Fun_Dsp( FUNMSG[39] );				/* "�@�{�@�@�|�@ ���  ����  �I�� " */
				}
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				BUZPI();
				if (input == -1)
					input = 0;
				input = (input*10 + msg-KEY_TEN0) % 100;
				if (pos == 0)
					splday_period_dsp(no, pos, (long)(input*100), 1);
				else
					splday_period_dsp(no, pos, (long)input, 1);
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				splday_period_dsp(no, pos, pSPP[no], 1);
				input = -1;
				break;
			default:
				break;
			}
		}
		/* ������ */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���(F3) */
				pSPP[no*2] = 0;
				pSPP[no*2+1] = 0;
				splday_period_dsp((char)(no*2), 0, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2), 1, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2), 2, pSPP[no*2], 0);
				splday_period_dsp((char)(no*2+1), 0, pSPP[no*2+1], 0);
				splday_period_dsp((char)(no*2+1), 1, pSPP[no*2+1], 0);
				splday_period_dsp((char)(no*2+1), 2, pSPP[no*2+1], 0);
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				grachr((ushort)(1+no*2), 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "����" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
				no = 0;
				pos = 0;
				mode = 0;
				input = -1;
				splday_period_dsp(no, pos, pSPP[no], 1);
				Fun_Dsp( FUNMSG[39] );		/* "�@�{�@�@�|�@ ���  ����  �I�� " */
				break;
			case KEY_TEN_F1:		/* ��(F1) */
			case KEY_TEN_F2:		/* ��(F2) */
				BUZPI();
				grachr((ushort)(1+no*2), 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "����" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
				if (msg == KEY_TEN_F1) {
					if (--no < 0)
						no = 2;
				}
				else {
					if (++no > 2)
						no = 0;
				}
				grachr((ushort)(1+no*2), 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[1]);				/* "����" */
				opedsp((ushort)(1+no*2), 6, (ushort)(no+1), 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* ���� */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʊ��ԕ\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_period_dsp( no, pos, data, rev )                 |*/
/*| PARAMETER    : char no    : �ԍ�                                       |*/
/*|              : char pos   : �ʒu(0:�� 1:��)                            |*/
/*|              : long data  : �f�[�^                                     |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_period_dsp(char no, char pos, long data, ushort rev)
{
	if (pos == 0) {
		if (GET_MONTH(data))
			opedsp((ushort)(1+no), 16, (ushort)GET_MONTH(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		else
			grachr((ushort)(1+no), 16, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "�@�@" */
	}
	else {	/* pos == 1 */
		if (GET_DAY(data))
			opedsp((ushort)(1+no), 22, (ushort)GET_DAY(data), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		else
			grachr((ushort)(1+no), 22, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);					/* "�@�@" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��^���ʊ��ԁ[�n�b�s�[�}���f�[�ݒ�                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_monday( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_monday(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-11 */
	long	*pSPM;
	char	dsp;
	char	mode;	/* 0:�ʏ� 1:��� */
	char	pos;	/* 0:�� 1:���j */
	long	input;

	pSPM = &CPrmSS[S_TOK][42];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* ��ʕ\�� */
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[2] );			/* "���n�b�s�[�}���f�[���@�@�@�@�@" */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
				opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
				splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* �� */
				splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* �T */
			}
			if (mode == 0) {
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* �� ���] */
				Fun_Dsp( FUNMSG[39] );				/* "�@�{�@�@�|�@ ���  ����  �I�� " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);		/* "�@������ԍ���I�����ĉ������@" */
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� ���] */
				Fun_Dsp(FUNMSG2[14]);				/* "�@�{�@�@�|�@ ���        �I�� " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		/* �ʏ��� */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* �{(F1) */
				if (pos == 1) {
					Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
				}
				// �T�����ݒ�̏ꍇ���ݍs�̃f�[�^���N���A
				input = GET_HM_WKNO(no, pSPM[no/2]);
				if (!input) {
					pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
				}
				BUZPI();
				no--;
				if (no < 0) {
					top = SP_HAPPY_MAX-5;
					no = SP_HAPPY_MAX-1;
				}
				else if (no < top)
					top--;
				pos = 0;
				input = -1;
				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* �� */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* �T */
				}
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* �� ���] */
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				if (pos == 1) {
					pos = 0;
					Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
				}
				// �T�����ݒ�̏ꍇ���ݍs�̃f�[�^���N���A
				input = GET_HM_WKNO(no, pSPM[no/2]);
				if (!input) {
					pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
				}
				BUZPI();
				no++;
				if (no > SP_HAPPY_MAX-1)
					top = no = 0;
				else if (no > top+4)
					top++;
				input = -1;
				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* �� */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* �T */
				}
				splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* �� ���] */
				break;
			case KEY_TEN_F3:		/* ���(F3) */
				BUZPI();
				if (pos == 0) {		// ������F���Ӱ�ނɑJ��
					// �T�����ݒ�̏ꍇ���ݍs�̃f�[�^���N���A
					input = GET_HM_WKNO(no, pSPM[no/2]);
					if (!input) {
						pSPM[no/2] = (ulong)((no & 1) ? (pSPM[no/2]/1000)*1000 : pSPM[no/2]%1000);
					}
					mode = 1;
					dsp = 1;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				}
				else {				// BACK���F�O�̍��ڂɖ߂�
					splday_monday_dsp(top, no, pos, pSPM[no/2], 0);
					pos--;
					input = -1;
					splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
					Fun_Dsp( FUNMSG[39] );		// "�@�{�@�@�|�@ ���  ����  �I�� "
				}
				break;
			case KEY_TEN_F4:		/* ����(F4) */
				if (pos == 0) {
					if (input == -1)
						input = (no & 1) ? pSPM[no/2]%1000/10 : pSPM[no/2]/10000;
					if (input < 1 || input > 12) {
						BUZPIPI();
						input = -1;
						splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
						break;
					}
					BUZPI();
					if (no & 1)
						pSPM[no/2] = pSPM[no/2]/1000*1000 + input*10 + pSPM[no/2]%10;
					else
						pSPM[no/2] = input*10000 + pSPM[no/2]%10000;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_monday_dsp(top, no, pos, pSPM[no/2], 0);
					pos = 1;
					input = -1;
					splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
					Fun_Dsp( FUNMSG[30] );				/* "  �{    �|    ��   ����  �I�� " */
				}
				else {	/* if (pos == 1) */
					if (input == -1)
						input = (no & 1) ? pSPM[no/2]%10 : pSPM[no/2]/1000%10;
					if (input < 1 || input > 5) {
						BUZPIPI();
						input = -1;
						splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
						break;
					}
					BUZPI();
					if (no & 1)
						pSPM[no/2] = pSPM[no/2]/10*10 + input;
					else
						pSPM[no/2] = pSPM[no/2]/10000*10000 + input*1000 + pSPM[no/2]%1000;
					no++;
					if (no > SP_HAPPY_MAX-1)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					OpelogNo = OPLOG_TOKUBETUHAPPY;		// ���엚��o�^
					f_ParaUpdate.BIT.splday = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					/* �\�� */
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
						splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);		/* �� */
						splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);		/* �T */
					}
					splday_monday_dsp(top, no, 0, pSPM[no/2], 1);		/* �� ���] */
					Fun_Dsp( FUNMSG[39] );				/* "�@�{�@�@�|�@ ���  ����  �I�� " */
				}
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				BUZPI();
				if (input == -1)
					input = 0;
				if (pos == 0) {
					input = (input*10 + msg-KEY_TEN0) % 100;
					if (no & 1)
						splday_monday_dsp(top, no, pos, input*10, 1);
					else
						splday_monday_dsp(top, no, pos, input*10000, 1);
				}
				else {	/* if (pos == 1) */
					input = msg-KEY_TEN0;
					if (no & 1)
						splday_monday_dsp(top, no, pos, input, 1);
					else
						splday_monday_dsp(top, no, pos, input*1000, 1);
				}
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				splday_monday_dsp(top, no, pos, pSPM[no/2], 1);
				input = -1;
				break;
			default:
				break;
			}
		}
		/* ������ */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���(F3) */
				if (no & 1)
					pSPM[no/2] = pSPM[no/2]/1000*1000;
				else
					pSPM[no/2] = pSPM[no/2]%1000;
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* �{(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = SP_HAPPY_MAX-5;
					no = SP_HAPPY_MAX-1;
				}
				else if (no < top)
					top--;

				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* �� */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* �T */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� ���] */
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				BUZPI();
				no++;
				if (no > SP_HAPPY_MAX-1)
					top = no = 0;
				else if (no > top+4)
					top++;

				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[13]);			/* "�@�@�@�F�@�@���@��@���j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
					splday_monday_dsp(top, (char)(top+i), 0, pSPM[(top+i)/2], 0);							/* �� */
					splday_monday_dsp(top, (char)(top+i), 1, pSPM[(top+i)/2], 0);							/* �T */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� ���] */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �n�b�s�[�}���f�[�\��                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_monday_dsp( top, no, pos, data, rev )            |*/
/*| PARAMETER    : char top   : �擪�ԍ�                                   |*/
/*|              : char no    : �ԍ�                                       |*/
/*|              : char pos   : �ʒu(0:�� 1:�T)                            |*/
/*|              : long data  : �f�[�^                                     |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	splday_monday_dsp(char top, char no, char pos, long data, ushort rev)
{
	ushort	tmp;

	if (pos == 0) {
		tmp = (no & 1) ? (ushort)(data%1000/10) : (ushort)(data/10000);
		if (tmp)
			opedsp((ushort)(2+no-top), 8, tmp, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �� */
		else
			grachr((ushort)(2+no-top), 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);		/* "�@�@" */
	}
	else {	/* if (pos == 1) */
		tmp = (no & 1) ? (ushort)(data%10) : (ushort)(data/1000%10);
		if (tmp)
			opedsp((ushort)(2+no-top), 18, tmp, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �T */
		else
			grachr((ushort)(2+no-top), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);		/* "�@" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʓ��^���ʊ���--���ʗj���w��@�@�@�@                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_week( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : 0:exit 1:mode change                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	splday_week(void)
{
	ushort	msg;
	char	i;
	char	top;
	int		no;		/* 0-11 */
	long	*pSPM;
	char	dsp;
	char	mode;	/* 0:�ʏ� 1:��� */
	char	pos;	/* 0:�� 1:�T�@2:�j�� */
	long	input,weekcnt=0;

	pSPM = &CPrmSS[S_TOK][56];
	no = top = 0;
	dsp = 1;
	pos = 0;
	input = -1;
	mode = 0;
	for ( ; ; ) {
		if (dsp) {
			/* ��ʕ\�� */
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[18] );									/* "�����ʗj���� " */
			for (i = 0; i < 5; i++) {
				grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "�@�@�@�F�@�@���@��@�j���@�@" */
				opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
				splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* �� */
				splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* �T */
				splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* �j�� */
			}
			if (mode == 0) {
				splday_week_dsp(top, no, 0, pSPM[no], 1);														/* �� ���] */
				Fun_Dsp( FUNMSG[39] );																			/* "�@�{�@�@�|�@ ���  ����  �I�� " */
			}
			else {
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[6]);									/* "�@������ԍ���I�����ĉ������@" */
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� ���] */
				Fun_Dsp(FUNMSG2[14]);																			/* "�@�{�@�@�|�@ ���        �I�� " */
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG) {
			BUZPI();
			return 1;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( msg == LCD_DISCONNECT ) {
			return 2;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		/* �ʏ��� */
		if (mode == 0) {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				return 0;
			case KEY_TEN_F1:		/* �{(F1) */
				if (pos == 2) {
					BUZPI();
					input = 1;
					weekcnt--;
					if(weekcnt < 1)
						weekcnt = 7;
					splday_week_dsp(top, no, pos, weekcnt, 1);
				}else{
				BUZPI();
				no--;
				if (no < 0) {
					top = 7;
					no = 11;
				}
				else if (no < top)
					top--;
				input = -1;
				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "�@�@�@�F�@�@���@��@�j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* �� */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* �T */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* �j�� */
				}
				splday_week_dsp(top, no, pos, pSPM[no], 1);							/* �� ���] */
				}
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				if (pos == 1) {
					BUZPIPI();
					break;
				}else if(pos == 2){
					BUZPI();
					input = 0;
					weekcnt++;
					if(weekcnt > 7)
						weekcnt = 1;
					splday_week_dsp(top, no, pos, weekcnt, 1);

				}else{
					BUZPI();
					no++;
					if (no > 11)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;
					input = -1;
					/* �\�� */
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "�@�@�@�F�@�@���@��@�j���@�@" */
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* �� */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* �T */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* �j�� */
					}
					splday_week_dsp(top, no, 0, pSPM[no], 1);							/* �� ���] */
				}
				break;
			case KEY_TEN_F3:		/* ���(F3) */
				BUZPI();
				mode = 1;
				dsp = 1;
				OpelogNo = OPLOG_TOKUBETUWEEKLY;		// ���엚��o�^
				f_ParaUpdate.BIT.splday = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				break;
			case KEY_TEN_F4:		/* ����(F4) */
				if (pos == 0) {												/* �����͏�ԁH */
					if (input == -1)										/* �l�̕ύX�Ȃ���Ԃŏ������݉��� */
						input = ((pSPM[no] % 10000 ) /100);					/* �\������Ă���l���Z�b�g */
					if (input < 1 || input > 12) {							/* ���͒l�ݒ�͈̓`�F�b�N */
						BUZPIPI();
						input = -1;
						splday_week_dsp(top, no, pos, pSPM[no], 1);			
						break;
					}
					BUZPI();
					pSPM[no] = (input*100) + (pSPM[no] % 100);				/* ���͂��ꂽ�l���o�b�t�@�ɐݒ� */
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* ���엚��o�^ */
					f_ParaUpdate.BIT.splday = 1;							// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					splday_week_dsp(top, no, pos, pSPM[no], 0);
					pos = 1;												/* ���̓��͂� */
					input = -1;												/* ���͏�ԃt���O�������l�ɖ߂� */
					splday_week_dsp(top, no, pos, pSPM[no], 1);
				}
				else if(pos == 1){											/* �T���͏�ԁH */
					if (input == -1)										/* �l�̕ύX�Ȃ���Ԃŏ������݉��� */
						input = ((pSPM[no] % 100) / 10);					/* �\������Ă���l���Z�b�g */
					if (input < 1 || input > 5) {							/* ���͒l�ݒ�͈̓`�F�b�N */
						BUZPIPI();
						input = -1;
						splday_week_dsp(top, no, pos, pSPM[no], 1);
						break;
					}
					BUZPI();
					pSPM[no] = (pSPM[no]/100)*100 + input*10 + (pSPM[no]%10);	/* ���͂��ꂽ�l���o�b�t�@�ɐݒ� */
					if (no > 11)			
						top = no = 0;									
					else if (no > top+4)
						top++;
					pos = 2;												/* ���̓��͂� */
					input = -1;												/* ���͏�ԃt���O�������l�ɖ߂� */
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* ���엚��o�^ */
					f_ParaUpdate.BIT.splday = 1;							// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

					/* �\�� */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);		/* �� */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);		/* �T */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);		/* �j�� */
					}
					splday_week_dsp(top, no, 2, pSPM[no], 1);							/* �j�� ���] */
					weekcnt = (ushort)((pSPM[no]%10) ? (ushort)(pSPM[no]%10):1);

				}
				else {	/* if (pos == 2) */
					if (input == -1){										/* �l�̕ύX�Ȃ���Ԃŏ������݉��� */
						weekcnt = (pSPM[no]%10);							/* �\������Ă���l���Z�b�g */
					}
					if (weekcnt < 1 || weekcnt > 7 ) {						/* ���͒l�ݒ�͈̓`�F�b�N */
						if(weekcnt != 0){
							BUZPIPI();	
							input = -1;										/* ���͏�ԃt���O�������l�ɖ߂� */
							weekcnt = 1;									/* �j���J�E���g�������l�i���j�ɐݒ� */
							splday_week_dsp(top, no, pos, pSPM[no], 1);
							break;
						}else{
							weekcnt = 1;									/* �j���J�E���g�������l�i���j�ɐݒ� */
						}
					}

					BUZPI();
					OpelogNo = OPLOG_TOKUBETUWEEKLY;						/* ���엚��o�^ */
					f_ParaUpdate.BIT.splday = 1;							// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					pSPM[no] = (pSPM[no]/100)*100 + ((pSPM[no]%100)/10)*10 + weekcnt;	/* ���͂��ꂽ�l���o�b�t�@�ɐݒ� */
					no++;													/* ���͍s���C���N�������g */
					if (no > 11)
						top = no = 0;
					else if (no > top+4)
						top++;
					pos = 0;												/* ���̓��͂����ɐݒ� */
					input = -1;												/* ���͏�ԃt���O�������l�ɖ߂� */
					/* �\�� */
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0,
											COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
						splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);		/* �� */
						splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);		/* �T */
						splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);		/* �j�� */
					}
					splday_week_dsp(top, no, pos, pSPM[no], 1);							/* �� ���] */
				}
				break;
			case KEY_TEN:			/* ����(�e���L�[) */
				BUZPI();
				if (input == -1)
					input = 0;												/* ���͏�ԃt���O�̒l���X�V */
				if (pos == 0) {												/* �����͏�ԁH */
					input = (input*10 + msg-KEY_TEN0) % 100;				/* ���͒l���擾�i�Q���j */
					splday_week_dsp(top, no, pos, input*100, 1);
				}
				else if(pos == 1){											/* �T���͏�ԁH */
					input = msg-KEY_TEN0;									/* ���͒l�̎擾 */
					splday_week_dsp(top, no, pos, input*10, 1);
				}else{	/* if (pos == 2) */									/* �j�����͏�ԁH */
					input = msg-KEY_TEN0;									/* ���͒l�̎擾 */
					if(input > 7 || input == 0){							/* ���͒l�͈̔̓`�F�b�N */
						BUZPIPI();
						break;
					}
					splday_week_dsp(top, no, pos, input, 1);
					weekcnt = input;										/* �{-�L�[�Ƃ̐�������ۂ��ߗj���J�E���g����͒l�ōX�V */
				}
				break;
			case KEY_TEN_CL:		/* ���(�e���L�[) */
				BUZPI();
				splday_week_dsp(top, no, pos, pSPM[no], 1);
				input = -1;													/* ���͏�ԃt���O�������l�ɖ߂� */
				break;
			default:
				break;
			}
		}
		/* ������ */
		else {
			switch (msg) {
			case KEY_TEN_F3:		/* ���(F3) */
				pSPM[no] = 0;
			case KEY_TEN_F5:		/* �I��(F5) */
				BUZPI();
				mode = 0;
				pos = 0;
				dsp = 1;
				input = -1;
				break;
			case KEY_TEN_F1:		/* �{(F1) */
				BUZPI();
				no--;
				if (no < 0) {
					top = 7;
					no = 11;
				}
				else if (no < top)
					top--;

				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "�@�@�@�F�@�@���@��@�j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* �� */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* �T */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* �j�� */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �ԍ� ���] */
				break;
			case KEY_TEN_F2:		/* �|(F2) */
				BUZPI();
				no++;
				if (no > 11)
					top = no = 0;
				else if (no > top+4)
					top++;

				/* �\�� */
				for (i = 0; i < 5; i++) {
					grachr((ushort)(2+i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR2[19]);					/* "�@�@�@�F�@�@���@��@�j���@�@" */
					opedsp((ushort)(2+i), 2, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �ԍ� */
					splday_week_dsp(top, (char)(top+i), 0, pSPM[(top+i)], 0);										/* �� */
					splday_week_dsp(top, (char)(top+i), 1, pSPM[(top+i)], 0);										/* �T */
					splday_week_dsp(top, (char)(top+i), 2, pSPM[(top+i)], 0);										/* �j�� */
				}
				opedsp((ushort)(2+no-top), 2, (ushort)(no+1), 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �ԍ� ���] */
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʗj���\���@�@�@�@                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : splday_week_dsp( top, no, pos, data, rev )   �@         |*/
/*| PARAMETER    : char top   : �擪�ԍ�                                   |*/
/*|              : char no    : �ԍ�                                       |*/
/*|              : char pos   : �ʒu(0:�� 1:�T 2:�j��)                     |*/
/*|              : long data  : �f�[�^                                     |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static void	splday_week_dsp(char top, char no, char pos, long data, ushort rev)
{
	ushort	tmp=0;

	if (pos == 0) {
		tmp = (ushort)((data % 10000) / 100);							/* ������茎�Z�o */
		if (tmp)
			opedsp((ushort)(2+no-top), 8, tmp, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* �� */
		else
			grachr((ushort)(2+no-top), 8, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* �����\���p */
	}
	else if(pos == 1){
		tmp = (ushort)((data % 100) / 10);								/* �������T�Z�o */
		if (tmp)
			opedsp((ushort)(2+no-top), 18, tmp, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* �T */
		else
			grachr((ushort)(2+no-top), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* �����\���p */
	}else{
		tmp = (ushort)(data % 10);										/* �������j���Z�o */	
		if(!tmp && rev == 1){											/* �f�[�^���O�����]�\��������ꍇ(�����\���p) */
			grachr((ushort)(2+no-top), 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, WEEKLYFFNT[1]);		/* "��"�\�� */
		}else{			
			grachr( (ushort)(2+no-top), 20,  2, rev, COLOR_BLACK, LCD_BLINK_OFF, WEEKLYFFNT[tmp] );	/* "���`�y"�\�� */
		}
	}
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
			if ((OPECTL.PriEndMsg[no].BPrinStat & 0x08) == 0) {
			// �w�b�h���x�ُ�ȊO�͎��؂�ƌ��Ȃ�
				ret = PRI_NO_PAPER;
			}
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�v�����g���ʃ`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind : �Ώۃv�����^
///	@param[in]		disp : ���ʕ\������<br>
//							1:���l�\���i���~�����O�j<br>
//							2:���l�\��<br>
//							3:�����\���i���~�����O�j<br>
//							4:�����\��
///	@param[in]		line : ���ʕ\���s
///	@return			PRI_NML_END�^PRI_CSL_END�^PRI_ERR_END
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CheckPrintResult(uchar pri_kind, int disp, int line)
{
	static	const	char	msgtbl[][4] = {
		{  0,  9, 10,  0},		{ 0,  9, 10, 11},
		{  0, 12, 13,  0},		{ 0, 12, 13, 14}
	};
	int		ret = PRI_ERR_END;
	int		no;

	switch(pri_kind) {
	default:
		ret = check_print_result(0);
		break;
	case	RJ_PRI:
		ret = check_print_result(0);
		if (ret != PRI_NML_END)
			break;
	// not break
	case	J_PRI:
		ret = check_print_result(1);
		break;
	}
	if (disp) {
		switch(ret) {
		default:
			no = 0;
			break;
		case	PRI_ERR_END:
			no = 1;
			break;
		case	PRI_NO_PAPER:
			no = 2;
			break;
		case	PRI_CSL_END:
			no = 3;
			break;
		}
		no = msgtbl[disp-1][no];
		if (no != 0) {
			grachr( (ushort)line,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[no] );
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�F�v�����g
//[]----------------------------------------------------------------------[]
///	@return			
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/27
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
unsigned short	UsMnt_mnyprint(void)
{
	ushort msg;
	int		inji_end;
	int		can_req;
	T_FrmChargeSetup	FrmChargeSetup;
	T_FrmPrnStop		FrmPrnStop;

	dispclr();

	FrmChargeSetup.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// �@�B��
	FrmChargeSetup.Oiban = CountRead_Individual(SET_PRI_COUNT);						// �ǔԓǏo��
	memcpy( &FrmChargeSetup.NowTime, &CLK_REC, sizeof( date_time_rec ) );			// ���ݎ���
	FrmChargeSetup.prn_kind = R_PRI;
	queset( PRNTCBNO, PREQ_CHARGESETUP, sizeof(T_FrmChargeSetup), &FrmChargeSetup );// �����ݒ��
	Ope_DisableDoorKnobChime();
	wopelg( OPLOG_CHARGE_PRINT, 0, 0 );
	inji_end = 0;
	can_req = 0;
	Cal_Parameter_Flg = 0;					// �ݒ�l�ُ�t���O�N���A

	grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0]);		// "�������ݒ聄�@�@�@�@�@�@�@�@�@"
	grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "�@�@�@ �v�����g���ł� �@�@�@�@"
	Fun_Dsp(FUNMSG[82]);					// "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
	// �ǂ��ԃJ�E���g�A�b�v
	CountUp_Individual(SET_PRI_COUNT);

	while (1) {
		msg = StoF(GetMessage(), 1);
		if (msg == (INNJI_ENDMASK|PREQ_CHARGESETUP)) {
			inji_end = 1;
			msg &= (~INNJI_ENDMASK);
		}
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
			// �h�A�m�u���ǂ����̃`�F�b�N�����{
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			BUZPI();
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			break;
		case KEY_TEN_F3:
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// �󎚒��~�v��
					can_req = 1;
					FrmPrnStop.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					wopelg( OPLOG_CAN_SET_PRI, 0, 0);			// �v�����g���~���샂�j�^�o�^
				}
			}
			else {
				BUZPI();
				return MOD_EXT;
			}
			break;
		case PREQ_CHARGESETUP:
			if (inji_end == 1) {
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�T�[�r�X�^�C��                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_SrvTime( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define INDEX_MAX	12		//�Ԏ�A�`L�̌�
unsigned short	UsMnt_SrvTime(void)
{
	ushort	msg;
	long	*pSTM;
	int		pos;		/* 0:�T�[�r�X�^�C�� 1:�O���[�X�^�C�� 2:���O�^�C�� */
	char	changing;
	long	min;
	long	max_tbl[] = {720, 120, 120};

	ushort	sel;		//�T�[�r�X�^�C���؊��I��
	ushort	index;		//�Ԏ�ւ̃C���f�b�N�X

	sel = (ushort)prm_get( COM_PRM,S_STM,1,1,1 );
	if (sel == 0) {
		index = 0;
	} else {
		index = 1;
	}

	pSTM = &CPrmSS[S_STM][2];

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[23]);			/* "���T�[�r�X�^�C�����@�@�@�@�@�@" */
	if (index) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR5[0]);			/* "�m�`�Ԏ�n" */
	}
	grachr(2, 4, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[0]);					/* "�T�[�r�X�^�C��" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�F" */
	opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);				/* "��" */
	grachr(3, 4, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_1[1]);					/* "�O���[�X�^�C��" */
	grachr(3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�F" */
	opedsp(3, 20, (ushort)pSTM[index*3+1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);/* �� */
	grachr(3, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);				/* "��" */
	grachr(4, 4, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[8]);					/* "���O�^�C��" */
	grachr(4, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�F" */
	opedsp(4, 20, (ushort)pSTM[index*3+2], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);/* �� */
	grachr(4, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);			/* "��" */
	if (index) {
		Fun_Dsp(FUNMSG2[9]);						/* "�@���@�@���@ �ύX�@�@�@�@�I�� " */
	} else {
		Fun_Dsp(FUNMSG2[0]);						/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
	}

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* �ύX(F3) */
				BUZPI();
				opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
				Fun_Dsp(FUNMSG[20]);		/* "�@���@�@���@�@�@�@ ����  �I�� " */
				min = -1;
				pos = 0;
				changing = 1;
				break;
			case KEY_TEN_F1:	/* �� */
			case KEY_TEN_F2:	/* �� */
				if (index == 0) {	//�Ԏ�ؑւȂ�
					break;
				}
				//�Ԏ�C���f�b�N�X�̎擾
				if (msg == KEY_TEN_F1) {
					if (index == 1) {
						index = INDEX_MAX;
					} else {
						index--;
					}
				} else {
					if (index == INDEX_MAX) {
						index = 1;
					} else {
						index++;
					}
				}
				BUZPI();
				//��ʕ\���ύX
				grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR5[index-1]);				/* "�mA�`L�Ԏ�n" */
				opedsp(2, 20, (ushort)pSTM[index*3], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �T�[�r�X�^�C�� �� */
				opedsp(3, 20, (ushort)pSTM[index*3+1], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �O���[�X�^�C�� �� */
				opedsp(4, 20, (ushort)pSTM[index*3+2], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* ���O�^�C�� �� */
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				if (index) {
					Fun_Dsp(FUNMSG2[9]);			/* "�@���@�@���@ �ύX�@�@�@�@�I�� " */
				} else {
					Fun_Dsp(FUNMSG2[0]);			/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
				}
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �� ���] */
				changing = 0;
				break;
			case KEY_TEN_F4:	/* ����(F4) */
				if (min != -1) {
					if (min > max_tbl[pos]) {
						BUZPIPI();
						opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
						min = -1;
						break;
					}
					pSTM[index*3+pos] = min;
					OpelogNo = OPLOG_SERVICETIME;		// ���엚��o�^
					f_ParaUpdate.BIT.other = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
					mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTP�t���O�Z�b�g
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				}
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �� ���] */
				if (msg == KEY_TEN_F1) {
					if (--pos < 0)
						pos = 2;
				}
				else {
					if (++pos > 2)
						pos = 0;
				}
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �� ���] */
				min = -1;
				break;
			case KEY_TEN:
				BUZPI();
				if (min == -1)
					min = 0;
				min = (min*10 + msg-KEY_TEN0) % 1000;
				opedsp((ushort)(2+pos), 20, (ushort)min, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);							/* �� ���] */
				break;
			case KEY_TEN_CL:
				BUZPI();
				opedsp((ushort)(2+pos), 20, (ushort)pSTM[index*3+pos], 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �� ���] */
				min = -1;
				break;
			default:
				break;
			}
			
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���b�N���u�^�C�}�[                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LockTimer( void )                                 |*/
/*| PARAMETER    : type 0�F�t���b�v 1�F���b�N                              |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	GET_MIN(t)		(((t)%100000L)/100)
#define	GET_SEC(t)		((t)%100)
enum{
	IDLE = 0,
	KIND,
	MIN,
	SEC
};
unsigned short	UsMnt_LockTimer( uchar type )
{
	ushort	msg;
	long	*ptime;
	char	set;
	long	data;
	long	tmp;

	char	syu = 1;
	uchar	t_type;

	if( !type ){
		if( !Get_Pram_Syubet(FLAP_UP_TIMER) ){
			ptime = &CPrmSS[S_TYP][118];	// �t���b�v�㏸�^�C�}�[
			t_type = 0;
		}else{
			ptime = &CPrmSS[S_LTM][31];		// �t���b�v�㏸�^�C�}�[(�`��)
			t_type = 1;
		}
	}else{
		if( !Get_Pram_Syubet(ROCK_CLOSE_TIMER) ){
			ptime = &CPrmSS[S_TYP][69];		// ���b�N�^�C�}�[
			t_type = 0;
		}else{
			ptime = &CPrmSS[S_LTM][11];		// ���b�N�^�C�}�[(�`��)
			t_type = 1;
		}
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[24]);					/* "�����b�N���u�^�C�}�[���@�@�@" */
		grachr(3,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[(!type?27:28)]);		/* "�t���b�v�@" or "���փ��b�N" */
		grachr(3, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "�F" */
		if( t_type ){
			grachr(3, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);				/* ��� */
		}
		opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "��" */
		opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
		grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "�b" */
	Fun_Dsp(FUNMSG2[0]);						/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */

	set = IDLE;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
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
		if (set == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* �ύX(F3) */
				BUZPI();
				if( t_type ){
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* ��� */
					Fun_Dsp(FUNMSG[20]);				/* "�@���@�@���@�@�@�@ ����  �I�� " */
					set = KIND;
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					Fun_Dsp(FUNMSG2[1]);				/* "�@�@�@�@�@�@ �@�@�@�����@�I�� " */
					set = MIN;
				}
				data = -1;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:
				BUZPI();
				if( t_type ){
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* ��� */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
				}
				Fun_Dsp(FUNMSG2[0]);				/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
				set = IDLE;
				break;
			case KEY_TEN_F4:
				if (set == KIND) {
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);								/* ��� ���] */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					set = MIN;
					data = -1;
					Fun_Dsp(FUNMSG2[1]);											/* "�@�@�@�@�@�@ �@�@�@�����@�I�� " */
				}else if (set == MIN) {
					if (data != -1) {
						tmp = *ptime/100000L*100000L + data*100 + *ptime%100;
						if ((tmp%100000L) > 72000) {
							BUZPIPI();
							opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
							data = -1;
							break;
						}
						*ptime = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					data = -1;
					set = SEC;
				}
				else {
					if (data != -1) {
						tmp = *ptime/100*100 + data;
						if ((tmp%100000L) > 72000 || data > 59) {
							BUZPIPI();
							opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
							data = -1;
							break;
						}
						*ptime = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
					if( t_type ){
						grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);							/* ��� ���]*/
						Fun_Dsp(FUNMSG[20]);				/* "�@���@�@���@�@�@�@ ����  �I�� " */
					}else{
						opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �� ���] */
					}
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �b ���] */
					data = -1;
					set = (t_type ? KIND:MIN);
				}
				BUZPI();
				break;
			case KEY_TEN:
				if (data == -1)
					data = 0;
				if (set == MIN) {
					data = (data*10 + msg-KEY_TEN0) % 1000;
					opedsp(3, 16, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
				}
				else if(set == SEC){
					data = (data*10 + msg-KEY_TEN0) % 100;
					opedsp(3, 24, (ushort)data, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}else{	// set == KIND
					break;
				}
				BUZPI();
				break;
			case KEY_TEN_CL:
				if (set == MIN){
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
				}else if(set == SEC){
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}else{// set == KIND
					break;
				}
				BUZPI();
				data = -1;
				break;
			case KEY_TEN_F1:
				if( set == KIND ){
					BUZPI();
					syu--;
					ptime--;
					if( syu < 1 ){
						syu = 12;
						ptime += 12;
					}
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* �`�� */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}
				break;
			case KEY_TEN_F2:
				if( set == KIND ){
					BUZPI();
					syu++;
					ptime++;
					if( syu > 12 ){
						syu = 1;
						ptime -= 12;
					}
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu]);					/* �`�� */
					opedsp(3, 16, (ushort)GET_MIN(*ptime), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp(3, 24, (ushort)GET_SEC(*ptime), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�W���p�X���[�h                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassWord( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/20 ART:ogura �\���C��                             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassWord(void)
{
	const uchar	*role_tbl[] = {DAT3_3[1], DAT3_3[3], DAT3_3[4], DAT3_3[7]};
	ushort		msg;
	short		role;		// ����(0-3)
	short		lv;			// ���x��(0-5)
	long		pswd;
	char		no;			/* �W���ԍ�(0-9) */
	char		set;		/* 0:�����ݒ� 1:�p�X���[�h�ݒ� */
	char		i;
	char		top;

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[25]);				/* "���W���p�X���[�h���@�@�@�@�@�@" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[26]);				/* "           ����  ����  �߽ܰ��" */

	for (i=0; i < 5; i++) {
		pswd = (ushort)prm_get(COM_PRM, S_PSW, (short)(i*2+1), 4, 1);		// �W����1�`5(�p�X���[�h)
		role = (short)prm_get(COM_PRM, S_PSW, (short)(i*2+2), 1, 1);		// �W����1�`5(����)
		lv = (short)prm_get(COM_PRM, S_PSW, (short)(i*2+2), 1, 2);			// �W����1�`5(���x��)
		grachr((ushort)(2+i), 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, &DAT3_3[3][1]);				/* "�W��" */
		opedsp((ushort)(2+i), 4, (ushort)(i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �W���ԍ� */
		grachr((ushort)(2+i), 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					/* "�F" */
		if (i == 0) {
			grachr((ushort)(2+i), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� ���] */
		} else {
			grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� */
		}
		opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* ���x�� */
		if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
			grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);			/* "��������" */
		} else {
			opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �p�X���[�h */
		}
	}
	Fun_Dsp( FUNMSG2[12] );							/* "�@���@�@���@ �ύX�@�����@�I�� " */

	no = 0;
	set = 0;
	pswd = prm_get(COM_PRM, S_PSW, 1, 4, 1);				// �W�����P �p�X���[�h
	role = (short)prm_get(COM_PRM, S_PSW, 2, 1, 1);			// �W�����P ����(27-0002�E)
	lv = (short)prm_get(COM_PRM, S_PSW, 2, 1, 2);			// �W�����P ���x��(27-0002�D)
	top = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == KEY_TEN_F5) {	/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		}
		if (set == 0) {
		/* �����ݒ蒆 */
			switch (msg) {
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				if (msg == KEY_TEN_F1) {
					if (no == 0) {
						top = 5;
						no = 9;
					}
					else if (no == 5) {
						top = 0;
						no = 4;
					}
					else
						no--;
				}
				else {
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;
				}

				for (i=0; i < 5; i++) {
					pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// �W����1�`5(�p�X���[�h)
					role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// �W����1�`5(����)
					lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// �W����1�`5(���x��)
					opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �W���ԍ� */
					grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� */
					opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* ���x�� */
					if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
						grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "��������" */
					} else {
						opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �p�X���[�h */
					}
				}
				pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
				role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
				lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
				grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� ���] */
				break;
			case KEY_TEN_F3:	/* �ύX(F3) */
				/* �Z�p������̏ꍇ */
				if (OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4) {
					if (++role > 3)
						role = 0;
				}
				/* �Z�p������ȊO */
				else {
					/* "�Z�p��"�̏ꍇ�͕ύX�s�� */
					if (role == 3) {
						BUZPIPI();
						break;
					}
					if (++role > 2)
						role = 0;
				}
				BUZPI();
				grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� ���] */
				break;
			case KEY_TEN_F4:	/* ����(F4) */
				if (OPECTL.Mnt_lev < 3 && role == 3) {
					BUZPIPI();
					break;
				}
				BUZPI();
				prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 1, (long)role);
				if ((role == 0) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {	// ���̌W����
					if (role == 0) {	/* �����Ȃ�-���x���ƃp�X���[�h���O�ɂ��Ď��̌W���� */
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 2, 0);
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+1), 4, 1, 0);
					}
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;

					for (i=0; i < 5; i++) {
						pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// �W����1�`5(�p�X���[�h)
						role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// �W����1�`5(����)
						lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// �W����1�`5(���x��)
						opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �W���ԍ� */
						grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� */
						opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* ���x�� */
						if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
							grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "��������" */
						} else {
							opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �p�X���[�h */
						}
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� ���] */
				}
				else {
					Fun_Dsp(FUNMSG2[1]);					/* "�@�@�@�@�@�@ �@�@�@�����@�I�� " */
					grachr((ushort)(2+no-top), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);			/* ���� ���] */
					if ((OPECTL.Mnt_lev >= 3) || (OPECTL.PasswordLevel >= lv)) {	// ���x���ݒ��
						opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* ���x�� ���] */
						lv = -1;
						set = 1;
					} else {														// �p�X���[�h�ݒ��
						opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �p�X���[�h ���] */
						pswd = -1;
						set = 2;
					}
				}
				OpelogNo = OPLOG_KAKARIINPASS;		// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP�X�V�t���O�Z�b�g
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				break;
			default:
				break;
			}
		}
		else {
		/* ���x���܂��̓p�X���[�h�ݒ蒆 */
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F4:	/* ����(F4) */
				if (set == 1) {				// ���x���ݒ蒆
					if ((lv > 5) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
						BUZPIPI();
						lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
						opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���x�� ���] */
						break;
					}
					BUZPI();
					if (lv != -1) {
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+2), 1, 2, (long)lv);
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* ���x�� ���] */
					opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �p�X���[�h ���] */
					pswd = -1;
					set = 2;
				} else {					// �p�X���[�h�ݒ蒆
					BUZPI();
					if (pswd != -1) {
						prm_set(COM_PRM, S_PSW, (ushort)(no*2+1), 4, 1, pswd);
					}
					if (no == 4)
						top = no = 5;
					else if (no == 9)
						top = no = 0;
					else
						no++;

					for (i=0; i < 5; i++) {
						pswd = prm_get(COM_PRM, S_PSW, (short)((top+i)*2+1), 4, 1);							// �W����1�`5(�p�X���[�h)
						role = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 1);					// �W����1�`5(����)
						lv = (short)prm_get(COM_PRM, S_PSW, (short)((top+i)*2+2), 1, 2);					// �W����1�`5(���x��)
						opedsp((ushort)(2+i), 4, (ushort)(top+i+1), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �W���ԍ� */
						grachr((ushort)(2+i), 10, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� */
						opedsp((ushort)(2+i), 18, (ushort)lv, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* ���x�� */
						if ((OPECTL.Mnt_lev < 3 && role == 3) || ((OPECTL.Mnt_lev < 3) && (lv > OPECTL.PasswordLevel))) {
							grachr((ushort)(2+i), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		/* "��������" */
						} else {
							opedsp((ushort)(2+i), 22, pswd, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �p�X���[�h */
						}
					}
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);									// �W����1�`5(�p�X���[�h)
					role = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 1);
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					grachr((ushort)(2+no-top), 10, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, role_tbl[role]);		/* ���� ���] */
					Fun_Dsp(FUNMSG2[12]);							/* "�@���@�@���@ �ύX�@�����@�I�� " */
					set = 0;
				}
				OpelogNo = OPLOG_KAKARIINPASS;		// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				break;
			case KEY_TEN:		/* ����(�e���L�[) */
				BUZPI();
				if (set == 1) {				// ���x���ݒ蒆
					if (lv == -1) {
						lv = 0;
					}
					lv = (lv*10 + msg-KEY_TEN0) % 10;
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���x�� ���] */
				} else {					// �p�X���[�h�ݒ蒆
					if (pswd == -1) {
						pswd = 0;
					}
					pswd = (pswd*10 + msg-KEY_TEN0) % 10000;
					opedsp((ushort)(2+no-top), 22, (ushort)pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �p�X���[�h ���] */
				}
				break;
			case KEY_TEN_CL:	/* ���(�e���L�[) */
				BUZPI();
				if (set == 1) {				// ���x���ݒ蒆
					lv = (short)prm_get(COM_PRM, S_PSW, (short)(no*2+2), 1, 2);
					opedsp((ushort)(2+no-top), 18, (ushort)lv, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���x�� ���] */
					lv = -1;
				} else {					// �p�X���[�h�ݒ蒆
					pswd = prm_get(COM_PRM, S_PSW, (short)(no*2+1), 4, 1);
					opedsp((ushort)(2+no-top), 22, pswd, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �p�X���[�h ���] */
					pswd = -1;
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�o�b�N���C�g�_�����@                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_BackLignt( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_BackLignt(void)
{
	ushort	msg;
	long	blgt;
	char	changing;
	long	data;

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[32]);		/* "���o�b�N���C�g�_�����@���@�@�@" */
	grachr(2, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);			/* "���݂̏�� */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "�F" */

	blgt = CPrmSS[S_PAY][29] % 10;
	if (blgt < 1)
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "�펞�_��" */
	else if (blgt < 2)
		grachr(2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
	else if (blgt < 3)
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "�O���M��" */
	else
		grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */
	grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "�펞�_��" */
	grachr(4, 14, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
	grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "�O���M��" */
	grachr(5, 14, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */
	backlgt_time_dsp(1, CPrmSS[S_PAY][30], 0);
	grachr(6, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[8]);			/* "�`" */
	backlgt_time_dsp(2, CPrmSS[S_PAY][31], 0);
	backlgt_dsp(blgt, 1);
	Fun_Dsp( FUNMSG[13] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ��(F1) */
		case KEY_TEN_F2:	/* ��(F2) */
			if (changing)
				break;
			BUZPI();
			backlgt_dsp(blgt, 0);
			if (msg == KEY_TEN_F1) {
				blgt -= 1;
				if (blgt < 0)
					blgt += 4;
			}
			else {
				blgt += 1;
				if (blgt >= 4)
					blgt -= 4;
			}
			backlgt_dsp(blgt, 1);
			break;
		case KEY_TEN_F4:	/* ����(F4) */
			if (changing == 0) {
				BUZPI();
				CPrmSS[S_PAY][29] = CPrmSS[S_PAY][29]/10*10 + blgt;
				grachr(2, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);			/* "�@�@" */
				if (blgt < 1)
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "�펞�_��" */
				else if (blgt < 2)
					grachr(2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
				else if (blgt < 3)
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);			/* "�O���M��" */
				else {
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */
					backlgt_dsp(blgt, 0);
					backlgt_time_dsp(1, CPrmSS[S_PAY][30], 1);
					Fun_Dsp(FUNMSG2[1]);		/* "�@�@�@�@�@�@�@�@�@ ����  �I�� " */
					changing = 1;
					data = -1;
				}
				OpelogNo = OPLOG_BACKLIGHT;		// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
 				mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			}
			else {
				if (data != -1) {
					if (data%100 > 59 || data > 2359) {
						BUZPIPI();
						backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
						data = -1;
						break;
					}
					CPrmSS[S_PAY][29+changing] = data;
				}
				BUZPI();
				backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 0);
				if (changing == 1) {
					changing = 2;
					backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
					data = -1;
				}
				else {
					backlgt_dsp(blgt, 1);
					Fun_Dsp( FUNMSG[13] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */
					changing = 0;
				}
			}
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			if (data == -1)
				data = 0;
			data = (data*10 + msg-KEY_TEN0) % 10000;
			backlgt_time_dsp(changing, data, 1);
			break;
		case KEY_TEN_CL:	/* ���(�e���L�[) */
			BUZPI();
			if (changing == 0) {
				backlgt_dsp(blgt, 0);
				blgt = CPrmSS[S_PAY][29] % 10;
				backlgt_dsp(blgt, 1);
			}
			else {
				backlgt_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
				data = -1;
			}
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �o�b�N���C�g�_�����@�\��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( data, rev )                                 |*/
/*| PARAMETER    : long   data : �_�����                                  |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	backlgt_dsp(long data, ushort rev)
{
	data = data%10;
	if (data < 1) {
		grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);		/* "�펞�_��" */
	}
	else if (data < 2) {
		grachr(4, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
	}
	else if (data < 3) {
		grachr(5, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[9]);		/* "�O���M��" */
	}
	else {
		grachr(5, 14, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �o�b�N���C�g�_�����@�\��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : backlgt_time_dsp( chng, time, rev )                     |*/
/*| PARAMETER    : char   chng : 1:�J�n���� 2:�I������                     |*/
/*|              : long   time : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	backlgt_time_dsp(char chng, long time, ushort rev)
{
	chng -= 1;
	opedsp(6, (ushort)(4+(chng*12)), (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(6, (ushort)(8+(chng*12)), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
	opedsp(6, (ushort)(10+(chng*12)), (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
}


// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
//	@brief			�A�i�E���X�̉�����LCD�ɑ��M���邽�߂̒l�ɕϊ�
//[]----------------------------------------------------------------------[]
//	@param[in]		uchar volume:RXM�̉��ʁA	uchar kind:�A�i�E���X == 0,�L�[�� == 1
//	@return			�Ȃ�
//	@author			Y.Yamauchi
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2020/02/28<br> 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar Anounce_volume ( uchar volume )
{
	// LCD�ւ̉��ʑ��M�e�[�u��
	uchar	announce_vol[ADJUS_MAX_VOLUME+1] = {0,7,14,21,28,35,42,50,56,63,69,75,81,87,93,100}; // LCD�p�A�i�E���X����
	
	return announce_vol[volume];		// �A�i�E���X
}

//[]----------------------------------------------------------------------[]
//	@brief			�L�[�̉�����LCD�ɑ��M���邽�߂̒l�ɕϊ�
//[]----------------------------------------------------------------------[]
//	@param[in]		uchar volume:RXM�̉���
//					kind: 0 = �|�W�V�����i�L�[���ʐݒ肩��j
//						  1 = RXM�̃p�����[�^����
//	@return			�Ȃ�
//	@author			Y.Yamauchi
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2020/02/28<br> 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar Key_volume ( uchar volume, uchar kind)
{
	// LCD�ւ̉��ʑ��M�e�[�u��
	uchar	kye_vol[4] = {0, 1, 2, 3};	// LCD�p�L�[���� 0:���� 1:�� 2:�� 3:��

	if( kind == 1 ){
		switch( volume ){
			case 0:
				return 0;		// ����
			case 1:
				return 3;		// ��
			case 2:
				return 2;		// ��
			case 3:
				return 1;		// ��
			default:
				break;
		}
	}
	return ( kye_vol[volume] );
}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�u�U�[���ʒ���                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_KeyVolume( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_KeyVolume(void)
{
	ushort	msg;
	long	vol;
	int		pos;	/* 0:���� 1:�� 3:�� 4:�� */
	long	vol_tbl[5] = {0, 30, 20, 10};	// RXM�p�����[�^�p 0:���� 30:�� 20:�� 10:��
// MH810100(S) 2020/02/21 Y.Yamauchi �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	volume;	// ����
// MH810100(E) 2020/02/21 Y.Yamauchi �Ԕԃ`�P�b�g���X(�����e�i���X)

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[27]);		/* "���u�U�[���ʒ������@�@�@�@�@�@" */
	grachr(2, 4, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);			/* "���݂̏�� */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			/* "�F" */

	vol = CPrmSS[S_PAY][29] % 100;
	if (vol < 10) {
		grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);		/* "����" */
		pos = 0;
	}
	else if (vol < 20) {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);		/* "��" */
		pos = 3;
	}
	else if (vol < 30) {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);		/* "��" */
		pos = 2;
	}
	else {
		grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);		/* "��" */
		pos = 1;
	}
	keyvol_dsp(pos);						/* "�����@���@���@��" */
	Fun_Dsp( FUNMSG[13] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I��(F5) */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			volume = Key_volume( ((uchar)prm_get(COM_PRM, S_PAY, 29, 1, 2)), 1);	// RXM�̃L�[����
			PKTcmd_beep_volume( volume );		// �u�U�[�v���i�ݒ�j
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ��(F1) */
		case KEY_TEN_F2:	/* ��(F2) */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 3;
			}
			else {
				if (++pos > 3)
					pos = 0;
			}
			keyvol_dsp(pos);			/* "�����@���@���@��" */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			vol = CPrmSS[S_PAY][29];
//			CPrmSS[S_PAY][29] = vol_tbl[pos] + (vol%10);
//			BUZPI();
//			CPrmSS[S_PAY][29] = vol;
			volume = Key_volume( pos, 0);		// �L�[����
			PKTcmd_beep_volume( volume );		// �u�U�[�v���i�ݒ�j
			BUZPI();
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case KEY_TEN_F4:	/* ����(F4) */
			CPrmSS[S_PAY][29] = vol_tbl[pos] + (CPrmSS[S_PAY][29]%10);	// �p�����[�^�ɔ��f
			BUZPI();
			grachr(2, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6]);				/* "�@" */
			if (pos == 0)
				grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);			/* "����" */
			else if (pos == 3)
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);			/* "��" */
			else if (pos == 2)
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);			/* "��" */
			else
				grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);			/* "��" */
			OpelogNo = OPLOG_KEYVOLUME;		// ���엚��o�^
			f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP�X�V�t���O�Z�b�g
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			break;
		case KEY_TEN_CL:	/* ���(�e���L�[) */
			BUZPI();
			vol = CPrmSS[S_PAY][29] % 100;
			if (vol < 10)
				pos = 0;
			else if (vol < 20)
				pos = 3;
			else if (vol < 30)
				pos = 2;
			else
				pos = 1;
			keyvol_dsp(pos);			/* "�����@���@���@��" */
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ʕ\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( pos )                                       |*/
/*| PARAMETER    : char pos : ���]�ʒu                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	keyvol_dsp(char pos)
{
	grachr(4,  8, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[2]);			/* "����" */
	grachr(4, 14, 2, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[5]);			/* "��" */
	grachr(4, 18, 2, ((pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[6]);			/* "��" */
	grachr(4, 22, 2, ((pos==3)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[7]);			/* "��" */
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F������                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_TickValid( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_TickValid(void)
{
	TIC_PRM	*wktic;
	TIC_PRM	*ptic;
	TIC_PRM	tic;
	ushort	msg;
	char	changing;
	char	no;				/* �����ԍ�(1-3) */
	char	pos;
	ushort	rev;
	long	*p;
	char	kind;
	long	data;
	long	tmp;

	wktic = &tick_valid_data.tic_prm;
	ptic = (TIC_PRM *)&CPrmSS[S_DIS][8];

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "�����������@�@�@�@�@�@�@�@�@" */
	grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "����" ���] */
	opedsp(2, 8, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* "�P"   ���] */
	grachr(3, 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "����" */
	opedsp(3, 8, 2, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* "�Q" */
	grachr(4, 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);			/* "����" */
	opedsp(4, 8, 3, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* "�R" */
	Fun_Dsp( FUNMSG[25] );					/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */

	changing = 0;
	no = 1;
	tick_valid_data.no = no;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		if (changing == 0) {
		/* ---�����ԍ��I��--- */
			switch (msg) {
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				grachr((ushort)(1+no), 4, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
				opedsp((ushort)(1+no), 8, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
				if (msg == KEY_TEN_F1) {
					if (--no < 1)
						no = 3;
				}
				else {
					if (++no > 3)
						no = 1;
				}
				tick_valid_data.no = no;
				grachr((ushort)(1+no), 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" ���] */
				opedsp((ushort)(1+no), 8, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ�   ���] */
				break;
			case KEY_TEN_F4:	/* �Ǐo(F4) */
				BUZPI();
				tic = ptic[no-1];
				memcpy( wktic, &tic, sizeof(TIC_PRM) );
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);				/* "�����������@�@�@�@�@�@�@�@�@�@" */
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[29]);				/* "�@�@�@�@��ʁ@�@�@�@�@���e�@�@" */
				grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);					/* "����" */
				opedsp(2, 4, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �ԍ� */
				ticval_kind_dsp(tic.kind, 1);					/* ��� ���] */
				ticval_data_dsp(tic.kind, tic.data, 0);			/* ���e */
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "�@�ύX�@�@�@�@�@�N�@�@���@�@��" */
				grachr(3, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[9]);					/* "�O" */
				grachr(3, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);					/* "�J�n" */
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "�@�ύX�@�@�@�@�@�N�@�@���@�@��" */
				grachr(4, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[9]);					/* "�O" */
				grachr(4, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);					/* "�I��" */
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "�@�ύX�@�@�@�@�@�N�@�@���@�@��" */
				grachr(5, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[10]);					/* "��" */
				grachr(5, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);					/* "�J�n" */
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[30]);				/* "�@�ύX�@�@�@�@�@�N�@�@���@�@��" */
				grachr(6, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[10]);					/* "��" */
				grachr(6, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);					/* "�I��" */
				p = tic.date;
				ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* ���t */
				Fun_Dsp( FUNMSG2[10] );					/* "�@�@�@�@�@�@ �ύX�@�����@�I�� " */
				kind = (char)tic.kind;
				pos = 0;
				changing = 1;
				kind = (char)tic.kind;
				break;
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
		else {
		/* ---�e���ڐݒ蒆--- */
			if (msg == KEY_TEN_F5) {	/* �I��(F5) */
				if( NG == ticval_date_chk( tic.date[0] ) ||		// �ύX�O�J�n���t����
					NG == ticval_date_chk( tic.date[1] ) ||		// �ύX�O�I�����t����
					NG == ticval_date_chk( tic.date[2] ) ||		// �ύX��J�n���t����
					NG == ticval_date_chk( tic.date[3] ) ){		// �ύX��I�����t����

					// ���t�ް����s���ȏꍇ
					BUZPIPI();
					continue;
				}
				BUZPI();
				ptic[no-1] = tic;	/* �p�����[�^�X�V */
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "�����������@�@�@�@�@�@�@�@�@" */
				rev = (no == 1) ? 1 : 0;
				grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
				opedsp(2, 8, 1, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�P" */
				rev = (no == 2) ? 1 : 0;
				grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
				opedsp(3, 8, 2, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�Q" */
				rev = (no == 3) ? 1 : 0;
				grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
				opedsp(4, 8, 3, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�R" */
				Fun_Dsp( FUNMSG[25] );					/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
				changing = 0;
				if( OpelogNo ){
					wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
					OpelogNo = 0;
					SetChange = 1;			// FLASH���ގw��
					UserMnt_SysParaUpdateCheck( OpelogNo );
					SetChange = 0;
				}
			}
			if (pos == 0) {
			/* ---��ʐݒ蒆--- */
				switch (msg) {
				case KEY_TEN_F3:	/* �ύX */
					BUZPI();
					if (++kind > 4)
						kind = 0;
					if (kind == 3)
						kind = 4;	// ���������ڃ}�X�N
					ticval_kind_dsp((long)kind, 1);					/* ��� ���] */
					break;
				case KEY_TEN_F4:	/* ���� */
					BUZPI();
					OpelogNo = OPLOG_KENKIGEN;			// ���엚��o�^
					f_ParaUpdate.BIT.tickvalid = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					if (kind == 0) {			/* �Ȃ� */
						/* �����ԍ��I���� */
						tic.kind = kind;
						tic.data = tic.date[0] = tic.date[1] = tic.date[2] = tic.date[3] = 0;
						ptic[no-1] = tic;			/* �p�����[�^�X�V */
						dispclr();
						grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[28]);		/* "�����������@�@�@�@�@�@�@�@�@" */
						rev = (no == 1) ? 1 : 0;
						grachr(2, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
						opedsp(2, 8, 1, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�P" */
						rev = (no == 2) ? 1 : 0;
						grachr(3, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
						opedsp(3, 8, 2, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�Q" */
						rev = (no == 3) ? 1 : 0;
						grachr(4, 4, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[7]);		/* "����" */
						opedsp(4, 8, 3, 1, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);				/* "�R" */
						Fun_Dsp( FUNMSG[25] );						/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
						changing = 0;
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						if( OpelogNo ){
							wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
							OpelogNo = 0;
							SetChange = 1;			// FLASH���ގw��
							UserMnt_SysParaUpdateCheck( OpelogNo );
							SetChange = 0;
						}
					}
					else if (kind == 4) {	/* �S�� */
						/* ���t�ݒ�� */
						pos = 2;
						tic.kind = kind;
						ticval_kind_dsp((long)kind, 0);						/* ��� */
						ticval_data_dsp(tic.kind, tic.data, 0);			/* ���e */
						p = tic.date;
						ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
						Fun_Dsp(FUNMSG2[2]);			/* "�@���@�@���@�@�@�@ ����  �I�� " */
						data = -1;
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
					}
					else {
						/* ���e�ݒ�� */
						pos = 1;
						if (tic.kind != kind) {
							tic.kind = kind;
							tic.data = 0;
							memcpy( wktic, &tic, sizeof(TIC_PRM) );
						}
						ticval_kind_dsp((long)kind, 0);						/* ��� */
						ticval_data_dsp(tic.kind, tic.data, 1);			/* ���e ���] */
						if (kind == 1) {
							Fun_Dsp(FUNMSG2[4]);		/* "�@���@�@���@ �ύX�@����  �I�� " */
							data = tic.data;
						}
						else {
							Fun_Dsp(FUNMSG2[11]);	/* "�@���@�@���@ �S�ā@����  �I�� " */
							data = -1;
						}
					}
					break;
				case KEY_TEN_CL:
					BUZPI();
					kind = (char)tic.kind;
					ticval_kind_dsp((long)kind, 1);					/* ��� ���] */
					break;
				default:
					break;
				}
			}
			else if (pos == 1) {
			/* ---���e�ݒ蒆--- */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* �� */
					BUZPI();
					ticval_kind_dsp(tic.kind, 1);					/* ��� ���] */
					ticval_data_dsp(tic.kind, tic.data, 0);			/* ���e */
					Fun_Dsp( FUNMSG2[10] );			/* "�@�@�@�@�@�@ �ύX�@�����@�I�� " */
					pos = 0;
					break;
				case KEY_TEN_F4:	/* ���� */
					if (data != -1) {
						if (tic.kind == 2) {
							if (data > 100) {
								BUZPIPI();
								ticval_data_dsp(tic.kind, tic.data, 1);			/* ���e ���] */
								data = -1;
								break;
							}
						}
						tic.data = data;
						OpelogNo = OPLOG_KENKIGEN;			// ���엚��o�^
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						f_ParaUpdate.BIT.tickvalid = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* �� */
					BUZPI();
					pos = 2;
					ticval_data_dsp(tic.kind, tic.data, 0);			/* ���e */
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
					Fun_Dsp(FUNMSG2[2]);			/* "�@���@�@���@�@�@�@ ����  �I�� " */
					data = -1;
					break;
				case KEY_TEN_F3:	/* �ύXor�S�� */
					BUZPI();
					if (tic.kind == 1) {
						if (++data > 15)
							data = 0;
					}
					else {
						data = 0;
					}
					ticval_data_dsp(tic.kind, data, 1);			/* ���e ���] */
					break;
				case KEY_TEN:
					if (tic.kind != 1) {
						BUZPI();
						if (data == -1)
							data = 0;
						if (tic.kind == 2) {
							data = (data*10 + msg-KEY_TEN0) % 1000;
							opedsp(2, 20, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���e ���] */
						}
						else {
							data = (data*10 + msg-KEY_TEN0) % 10000;
							opedsp(2, 20, (ushort)data, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ���e ���] */
						}
					}
					break;
				case KEY_TEN_CL:
					BUZPI();
					ticval_data_dsp(tic.kind, tic.data, 1);			/* ���e ���] */
					if (tic.kind == 1) {
						data = tic.data;
					}
					else {
						data = -1;
					}
					break;
				default:
					break;
				}
			}
			else {
			/* ---���t�ݒ蒆--- */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* �� */
					if( (pos%3) == 2 ){
						// ���وʒu���N���͂̏ꍇ�i���͒��̓��t����ʂ̓��͍��ڂֈړ�����ꍇ�j
						if( NG == ticval_date_chk( tic.date[(pos-2)/3] ) ){	// ���t�ް�����
							BUZPIPI();										// �m�f�����وړ����Ȃ�
							break;
						}
					}
					BUZPI();
					if (--pos < 2) {
						if (tic.kind == 4) {
							/* ��ʐݒ�� */
							pos = 0;
							ticval_kind_dsp(tic.kind, 1);					/* ��� ���] */
							p = tic.date;
							ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* ���t */
							Fun_Dsp( FUNMSG2[10] );					/* "�@�@�@�@�@�@ �ύX�@�����@�I�� " */
							kind = (char)tic.kind;
						}
						else {
							/* ���e�ݒ�� */
							p = tic.date;
							ticval_date_dsp(p[0], p[1], p[2], p[3], (char)-1);	/* ���t */
							ticval_data_dsp(tic.kind, tic.data, 1);			/* ���e ���] */
							if (tic.kind == 1) {
								Fun_Dsp(FUNMSG2[4]);		/* "�@���@�@���@ �ύX�@����  �I�� " */
								data = tic.data;
							}
							else {
								Fun_Dsp(FUNMSG2[11]);		/* "�@���@�@���@ �S�ā@����  �I�� " */
								data = -1;
							}
						}
					}
					else {
						p = tic.date;
						ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
						data = -1;
					}
					break;
				case KEY_TEN_F4:	/* ���� */
					if (data != -1) {
						if ((pos%3) == 2) {
							tic.date[(pos-2)/3] = data*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + GET_DAY(tic.date[(pos-2)/3]);
						}
						else if ((pos%3) == 0) {
							if ( (data > 12) && (data != 99) ) {
								BUZPIPI();
								p = tic.date;
								ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
								data = -1;
								break;
							}
							tic.date[(pos-2)/3] = GET_YEAR(tic.date[(pos-2)/3])*10000 + data*100 + GET_DAY(tic.date[(pos-2)/3]);
						}
						else {
							if ( (data > 31) && (data != 99) ) {
								BUZPIPI();
								p = tic.date;
								ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
								data = -1;
								break;
							}
							tic.date[(pos-2)/3] = GET_YEAR(tic.date[(pos-2)/3])*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + data;
						}
						OpelogNo = OPLOG_KENKIGEN;			// ���엚��o�^
						memcpy( wktic, &tic, sizeof(TIC_PRM) );
						f_ParaUpdate.BIT.tickvalid = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* �� */
					if( (pos%3) == 1 ){
						// ���وʒu�������͂̏ꍇ�i���͒��̓��t����ʂ̓��͍��ڂֈړ�����ꍇ�j
						if( NG == ticval_date_chk( tic.date[(pos-2)/3] ) ){	// ���t�ް�����
							BUZPIPI();										// �m�f�����وړ����Ȃ�
							break;
						}
					}
					BUZPI();
					if (++pos > 13)
						pos = 2;
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
					data = -1;
					break;
				case KEY_TEN:
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 100;
					if ((pos%3) == 2)
						tmp = data*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + GET_DAY(tic.date[(pos-2)/3]);
					else if ((pos%3) == 0)
						tmp = GET_YEAR(tic.date[(pos-2)/3])*10000 + data*100 + GET_DAY(tic.date[(pos-2)/3]);
					else
						tmp = GET_YEAR(tic.date[(pos-2)/3])*10000 + GET_MONTH(tic.date[(pos-2)/3])*100 + data;
					p = tic.date;
					if (pos < 5)
						ticval_date_dsp(tmp, p[1], p[2], p[3], pos);	/* ���t */
					else if (pos < 8)
						ticval_date_dsp(p[0], tmp, p[2], p[3], pos);	/* ���t */
					else if (pos < 11)
						ticval_date_dsp(p[0], p[1], tmp, p[3], pos);	/* ���t */
					else
						ticval_date_dsp(p[0], p[1], p[2], tmp, pos);	/* ���t */
					break;
				case KEY_TEN_CL:
					BUZPI();
					p = tic.date;
					ticval_date_dsp(p[0], p[1], p[2], p[3], pos);	/* ���t */
					data = -1;
					break;
				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��ʕ\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_kind_dsp( kind, rev )                            |*/
/*| PARAMETER    : long   kind : ���                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_kind_dsp(long kind, ushort rev)
{
	const uchar	*p;
	ushort	len;

	grachr(2, 8, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);
	switch (kind) {
	case 0:
		p = DAT2_6[9];
		len = 4;
		break;
	case 1:
		p = DAT5_3[4];
		len = 10;
		break;
	case 2:
		p = DAT3_3[5];
		len = 6;
		break;
	case 3:
		p = DAT3_3[6];
		len = 6;
		break;
	default:
		p = DAT2_6[8];
		len = 4;
		break;
	}
	grachr(2, 8, len, rev, COLOR_BLACK, LCD_BLINK_OFF, p);
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���e�\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_data_dsp( kind, data, rev )                      |*/
/*| PARAMETER    : long   kind : ���                                      |*/
/*|              : long   data : ���e                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_data_dsp(long kind, long data, ushort rev)
{
	grachr(2, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);
	if (kind != 0 && kind != 4) {
		if (data == 0)
			grachr(2, 22, 4, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[8]);	/* "�S��" */
		else {
			if (kind == 1)
				grachr(2, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[data-1]);
			else if (kind == 2)
				opedsp(2, 20, (ushort)data, 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
			else
				opedsp(2, 20, (ushort)data, 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���t�\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_date_dsp( d1, d2, d3, d4, rev_pos)               |*/
/*| PARAMETER    : long d1      : �ύX�O�J�n����                           |*/
/*|              : long d2      : �ύX�O�I������                           |*/
/*|              : long d3      : �ύX��J�n����                           |*/
/*|              : long d4      : �ύX��I������                           |*/
/*|              : char rev_pos : ���]�ʒu                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ticval_date_dsp(long d1, long d2, long d3, long d4, char rev_pos)
{

	opedsp(3, 12, (ushort)GET_YEAR(d1), 2, 1, ((rev_pos==2)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(3, 18, (ushort)GET_MONTH(d1), 2, 0, ((rev_pos==3)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(3, 24, (ushort)GET_DAY(d1), 2, 0, ((rev_pos==4)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(4, 12, (ushort)GET_YEAR(d2), 2, 1, ((rev_pos==5)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(4, 18, (ushort)GET_MONTH(d2), 2, 0, ((rev_pos==6)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(4, 24, (ushort)GET_DAY(d2), 2, 0, ((rev_pos==7)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(5, 12, (ushort)GET_YEAR(d3), 2, 1, ((rev_pos==8)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(5, 18, (ushort)GET_MONTH(d3), 2, 0, ((rev_pos==9)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(5, 24, (ushort)GET_DAY(d3), 2, 0, ((rev_pos==10)?1:0), COLOR_BLACK, LCD_BLINK_OFF);

	opedsp(6, 12, (ushort)GET_YEAR(d4), 2, 1, ((rev_pos==11)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(6, 18, (ushort)GET_MONTH(d4), 2, 0, ((rev_pos==12)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
	opedsp(6, 24, (ushort)GET_DAY(d4), 2, 0, ((rev_pos==13)?1:0), COLOR_BLACK, LCD_BLINK_OFF);
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���t�`�F�b�N                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ticval_date_chk( date )                                 |*/
/*| PARAMETER    : long date     : �N����(yymmdd)                          |*/
/*|                                                                        |*/
/*| RETURN VALUE : OK/NG                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static uchar ticval_date_chk( long date )
{
	short	year;			// �N
	short	mon;			// ��
	short	day;			// ��

	if( (date == 0) || (date == 999999) ){
		return( OK );						// �N�����S�ĂO�̏ꍇ�n�j
	}

	year	= (short)GET_YEAR(date);		// �N�ް��擾
	mon		= (short)GET_MONTH(date);		// ���ް��擾
	day		= (short)GET_DAY(date);			// ���ް��擾

	if( mon == 0 || day == 0 ){
		return( NG );						// ���܂��͓����O�̏ꍇ�m�f

	}
	if( year >= 80 ){						// �N�ް��ϊ�(����Q��������S��)
		year += 1900;
	}else{
		year += 2000;
	}

	if( 0 != chkdate( year, mon, day ) ){	// ���݂����������
		return( NG );
	}
	return( OK );
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�c�ƊJ�n����                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_BusyTime( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_BusyTime(void)
{
	ushort	msg;
	char	changing;	/* 1:�����ݒ蒆 */
	char	pos;		/* 0:�J�n���� 1:�I������ */
	long	input;
	long	*ptim;

	ptim = &CPrmSS[S_SYS][40];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[31]);		/* "���c�ƊJ�n�������@�@�@�@�@�@�@" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);			/* "�J�n���� */
	bsytime_dsp(0, ptim[0], 0);				/* ���� ���] */
	grachr(3, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);			/* "�I������" */
	bsytime_dsp(1, ptim[1], 0);				/* ���� ���] */
	Fun_Dsp( FUNMSG2[0] );					/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I��(F5) */
			BUZPI();
			if (changing == 0)
				return MOD_EXT;
			bsytime_dsp(pos, ptim[pos], 0);		/* ���� ���] */
			Fun_Dsp( FUNMSG2[0] );				/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
			changing = 0;
			break;
		case KEY_TEN_F3:	/* �ύX(F3) */
			if (changing == 1)
				break;
			BUZPI();
			bsytime_dsp(0, ptim[0], 1);		/* ���� ���] */
			Fun_Dsp( FUNMSG[20] );			/* "�@���@�@���@�@�@�@ ����  �I�� " */
			input = -1;
			changing = 1;
			pos = 0;
			break;
		case KEY_TEN_F4:	/* ����(F4) */
			if (changing == 0)
				break;
			if (input != -1) {
				if (input%100 > 59 || input > 2359) {
					BUZPIPI();
					bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
					input = -1;
					break;
				}
				ptim[pos] = input;
				OpelogNo = OPLOG_EIGYOKAISHI;			// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTP�X�V�t���O
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			}
		case KEY_TEN_F1:	/* ��(F1) */
		case KEY_TEN_F2:	/* ��(F2) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 0);		/* ���� ���] */
			pos ^= 1;
			bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
			input = -1;
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			if (input == -1)
				input = 0;
			input = (input*10 + msg-KEY_TEN0) % 10000;
			bsytime_dsp(pos, input, 1);			/* ���� ���] */
			break;
		case KEY_TEN_CL:	/* ���(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �c�Ǝ��ԕ\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : keyvol_dsp( pos )                                       |*/
/*| PARAMETER    : char   pos  : �\���ʒu                                  |*/
/*|              : long   time : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	bsytime_dsp(char pos, long time, ushort rev)
{
	opedsp((ushort)(2+pos), 14, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr((ushort)(2+pos), 18, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
	opedsp((ushort)(2+pos), 20, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�����ݒ�                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Mnysetmenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Mnysetmenu(void)
{
	unsigned short	usUmnyEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0] );		/* [00]	"�������ݒ聄�@�@�@�@�@�@�@�@�@" */

		usUmnyEvent = Menu_Slt( MNYSETMENU, USM_MNS_TBL, (char)USM_MNS_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){

			/* �ő嗿�� */
			case MNT_MNYLIM:
				usUmnyEvent = UsMnt_mnylim();
				break;
			/* �T�[�r�X������ */
			case MNT_MNYTIC:
				usUmnyEvent = UsMnt_mnytic();
				break;
			/* �X���� */
			case MNT_MNYSHP:
				usUmnyEvent = UsMnt_mnyshp();
				break;
			/* ����ł̐ŗ� */
			case MNT_MNYTAX:
				usUmnyEvent = UsMnt_mnytax();
				break;
			/* �`�`�k����� */
			case MNT_MNYKID:
				usUmnyEvent = UsMnt_mnykid();
				break;
			/* �P�ʎ��ԁ^���� */
			case MNT_MNYCHA:
				if (CPrmSS[S_CAL][1] != 1) {
					usUmnyEvent = UsMnt_mnycha();
				} else {
					usUmnyEvent = UsMnt_mnycha_dec();
				}
				break;
			/* ��������j�� */
			case MNT_MNYTWK:
				usUmnyEvent = UsMnt_mnytweek();
				break;
			/* �v�����g */
			case MNT_PRISET:
				if (Ope_isPrinterReady() == 0) {	// ���V�[�g�o�͕s��
					BUZPIPI();
					break;
				}
#if (5 == AUTO_PAYMENT_PROGRAM)
				if(TENKEY_CL == 1){
					usUmnyEvent = UsMnt_mc10print();
					break;
				}
#endif
				usUmnyEvent = UsMnt_mnyprint();
				break;

			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return MOD_EXT;
			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
			SetChange = 1;			// FLASH���ގw��
			UserMnt_SysParaUpdateCheck( OpelogNo );
			SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
			mnt_SetFtpFlag( FTP_REQ_NORMAL );		// FTP�X�V�t���O
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}

		if( usUmnyEvent == MOD_CHG || usUmnyEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}
		if (usUmnyEvent == MOD_CHG){
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (usUmnyEvent == MOD_CUT) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�ő嗿���񐔐ݒ�A�h���X�擾                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_mnylim_kaisuu_adr(char, short*, char*, char*)       |*/
/*| PARAMETER    : char : �ő�^�C�v�̐ݒ�l                               |*/
/*|                short* : ���ʃp�����[�^�̃A�h���X                       |*/
/*|                char* : �ݒ�l�̌���                                    |*/
/*|                char* : �ݒ�l�̍ŉ��ʌ��ʒu                            |*/
/*| RETURN VALUE : void                                                    |*/
/*| �⑫         : 28-0002�̐ݒ�l���擾���A���ꂼ��̃A�h���X��Ԃ�       |*/
/*| 							  10�F�V�ő厞�ԁE�񐔂̃A�h���X           |*/
/*| 						��L�ȊO�F�]���ő厞�ԁE�񐔂̃A�h���X         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void get_mnylim_kaisuu_adr(char type, char kind, short* adr, char* len, char* pos)
{
	if(!RYO_CAL_EXTEND){
		switch(type)		/* 28-0002����擾�����l */
		{
		default:			/* �W�� */
		case SP_MX_N_HOUR:	/* �����Ԃ���ő�(��) */
			*adr = 14;		/* �A�h���X */
			*len = 6;		/* ���� */
			*pos = 1;		/* �ŉ��ʌ��ʒu */
			break;
		case SP_MX_N_MH_NEW:/* �����Ԃ���ő�(�V) */
			*adr = 38;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 1;		/* �ŉ��ʌ��ʒu */
			break;
		}
	}
	else{
		switch(type)		/* 56-0020����擾�����l */
		{
		default:			/* �W�� */
		case SP_MX_N_HOUR:		/* �����Ԃ���ő�(��) */
		case SP_MX_N_MH_NEW:	/* �����Ԃ���ő�(�V) */
			*adr = 32+kind;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 1;		/* �ŉ��ʌ��ʒu */
			break;
		}
	}
		
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�ő嗿�����Ԑݒ�A�h���X�擾                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_mnylim_zikann_adr(char, short*, char*, char*)       |*/
/*| PARAMETER    : char : �ő�^�C�v�̐ݒ�l                               |*/
/*|                short* : ���ʃp�����[�^�̃A�h���X                       |*/
/*|                char* : �ݒ�l�̌���                                    |*/
/*|                char* : �ݒ�l�̍ŉ��ʌ��ʒu                            |*/
/*| RETURN VALUE : void                                                    |*/
/*| �⑫         : 28-0002�̐ݒ�l���擾���A���ꂼ��̃A�h���X��Ԃ�       |*/
/*| 							  10�F�V�ő厞�ԁE�񐔂̃A�h���X           |*/
/*| 						��L�ȊO�F�]���ő厞�ԁE�񐔂̃A�h���X         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void get_mnylim_zikann_adr(char type, char kind, short* adr, char* len, char* pos)
{
	if(!RYO_CAL_EXTEND){
		switch(type)		/* 28-0002����擾�����l */
		{
		default:			/* �W�� */
		case SP_MX_N_HOUR:
			*adr = 13;		/* �A�h���X */
			*len = 6;		/* ���� */
			*pos = 1;		/* �ŉ��ʌ��ʒu */
			break;
		case SP_MX_N_MH_NEW:
			*adr = 38;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 3;		/* �ŉ��ʌ��ʒu */
			break;
		case SP_MX_N_MHOUR_W:
			*adr = 41;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 3;		/* �ŉ��ʌ��ʒu */
			break;
		}
	}
	else{
		switch(type)		/* 56-0020����擾�����l */
		{
		default:				/* �W�� */
		case SP_MX_N_HOUR:		/* �����Ԃ���ő�(��) */
		case SP_MX_N_MH_NEW:	/* �����Ԃ���ő�(�V) */
			*adr = 32+kind;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 3;		/* �ŉ��ʌ��ʒu */
			break;
		case SP_MX_N_MHOUR_W:
			*adr = 20+kind;		/* �A�h���X */
			*len = 2;		/* ���� */
			*pos = 3;		/* �ŉ��ʌ��ʒu */
			break;
		}
	}
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*|  ��ʖ��̍ő嗿���^�C�v�擾                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_LIM_TYP( )  									       |*/
/*| PARAMETER    : kind    :���										   |*/
/*|              : max_set :�ŉ��s�̈ʒu                                   |*/
/*|              : param_pos: �p�����[�^�ʒu							   |*/
/*| RETURN VALUE : type    :�������                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char GET_LIM_TYP( char kind, char* max_set, ushort* param_pos ){
	
	char type;
	if(RYO_CAL_EXTEND){					// �����v�Z�g���L��
		type = (char)prm_get(COM_PRM, S_CLX, (short)(2+(kind/3)), 2, (char)(6-((kind%3+1)*2)+1) );		// ��ʖ��̍ő嗿���^�C�v�擾
	}
	else {
		type = (char)CPrmSS[S_CAL][2];						// �ő嗿���^�C�v�擾
	}

	switch( type ){
		case 2:		// �����w������ő�
		case 3:		// �Q�펞���w������ő�
		case 5:		// ���ԑэő�
		case 10:	// �����Ԃ���ő�
		case 20:	// �v���
			break;

		default:
			type = 0;	// ��L�ȊO�͍ő�Ȃ��œ��삷��B
			break;
	}

	switch(type){
	case SP_MX_NON:					// �����ő嗿������
	default:
		*max_set = 2;
		*param_pos = 1;
		break;
	case SP_MX_INTIME:				// ���Ɏ����ő�
	case SP_MX_TIME:				// �����w��ő�
		*max_set = 3;
		*param_pos = 1;
		break;
	case SP_MX_WTIME:				// �Q��ނ̎����w��ő�
	case SP_MX_N_HOUR:				// �����Ԃ���ő�(���ݒ�)
	case SP_MX_N_MH_NEW:			// �����Ԃ���ő�
		*max_set = 4;
		*param_pos = 1;
		break;
	case SP_MX_N_MHOUR_W:			// �Q��ނ̂����Ԃ���ő�
		*max_set = 5;
		*param_pos = 1;
		break;
	case SP_MX_BAND:				// ���ԑэő�
		*max_set = 8;
		*param_pos = 15;
		break;
	}
	return type;
}
/*[]----------------------------------------------------------------------[]*/
/*|  �ő嗿���^�C�v���̉�ʕ\��                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnylim_type_disp( )  							       |*/
/*| PARAMETER    : type    :�ő嗿���^�C�v								   |*/
/*| PARAMETER    : top_pos :�����т̍ŏ�\���ԍ�						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void mnylim_type_disp( char type , char top_pos )
{

	if (type == SP_MX_INTIME) {				// ���Ɏ����ő�(6/12/24)
		if( CPrmSS[S_CAL][13] == 6 ){
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[67] );	/* "  �U���ԍő�F�@�@�@�@�@�@�O�~" */
		}else if( CPrmSS[S_CAL][13] == 12 ){
			grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[50] );	/* "�P�Q���ԍő�F�@�@�@�@�@�@�O�~" */
		}else{
			grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[9] );	/* "�Q�S���ԍő�F�@�@�@�@�@�@�O�~" */
		}
	}
	else if (type == SP_MX_TIME) {			// �����w������ő�
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[8] );		/* "�����ő嗿���F�@�@�@�@�@�@�O�~" */
	}
	else if (type == SP_MX_WTIME){			// �Q��ނ̎����w������ő�
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[10] );	/* "�����ő嗿���P�F�@�@�@�@�@�O�~" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[11] );	/* "�����ő嗿���Q�F�@�@�@�@�@�O�~" */
	}
	else if (type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW){				// �����Ԃ���ő�
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[150] );	/* "�@�@���ԁ@�@�F�@�@�@�@�@�@�O�~" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[153] );	/* "�J��Ԃ��񐔁F�@�@�@�@�@�@�@��" */
	}
	else if (type == SP_MX_BAND){			// ���ԑэő�
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[140 + top_pos ] );	/* "�@��P�����сF�@�@�@�@	�@�O�~" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[141 + top_pos ] );	/* "�@��Q�����сF�@�@	�@�@�@�O�~" */
		grachr(5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[142 + top_pos ] );	/* "�@��R�����сF	�@�@�@�@�@�O�~" */
		grachr(6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[143 + top_pos ] );	/* "�@��S�����сF	�@�@�@�@�@�O�~" */
	}
	else if(type == SP_MX_N_MHOUR_W){		// �Q��ނ̂����Ԃ���ő�
		grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[151] );	/* "�@�@����(�P)�F�@�@�@�@�@�@�O�~" */
		grachr(4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[152] );	/* "�@�@����(�Q)�F�@�@�@�@�@�@�O�~" */
		grachr(5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[153] );	/* "�J��Ԃ��񐔁F�@�@�@�@�@�@�@��" */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  �p�����[�^��̐�΍ő嗿����INDEX���擾����                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_LIM_IDX(char n, char k)                             |*/
/*| PARAMETER    : n  �����̌n                                             |*/
/*|              : k  ���                                                 |*/
/*| RETURN VALUE : index  ��΍ő嗿����INDEX                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short GET_LIM_IDX(char n, char k){
	short	index;
	short	wk;
	
	wk = (n%3 == 0) ? 3 : n%3;
	/* (�̌n*300 + 61(�擪�A�h���X))  +  (���*20) */
	index = (RYO_TAIKEI_SETCNT*((wk)-1)+61)+(k*RYO_SYUBET_SETCNT);
	
	return index;
	
}
/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�ő嗿��                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnylim( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*| �⑫         : �ő嗿���^�C�v  0�F�����ő嗿���Ȃ�                     |*/
/*| 							   1�F���o�Ɏ��ԍő�(6/12/24)              |*/
/*| 							   2�F�����w������ő�		               |*/
/*| 							   3�F2��ނ̎����w������ő�              |*/
/*| 							   4�F�����Ԃ���ő�		               |*/
/*| 							   5�F���ԑэő�			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
///* �p�����[�^��̐�΍ő嗿����INDEX���擾����}�N�� */
unsigned short	UsMnt_mnylim(void)
{
	ushort	msg;
	char	set;		/* 0:�̌n�ݒ蒆 1:��ʐݒ蒆 2-6:���z�ݒ蒆 7:���ԉ�ݒ蒆*/
	char	max_set;
	char	no;			/* �̌n(1-3) */
	int		kind;		/* ���(A-L) */
	int		type;		/* �����ő嗿���^�C�v */
	long	money;
	char	calc;		/* �����v�Z����(0:����� 1:������) */
	ushort	line_pos;				/* �\���ʒu */
	ushort	param_pos;				/* ���ʃp�����[�^�ʒu */
	long	time_count;				/* ���ԁE�� */
	short	adr_k, adr_z;			/* ���ڽNo.						*/
	char	pos_k, pos_z;			/* ���ʒu						*/
	long	*pRAT;					/* ���ʃp�����[�^�ւ̃|�C���^ */
	char	seg;					/* �Z�N�V����No.() */
	short	lim_sec;				/* �Z�N�V����No.(�v���)*/
	short	adr_z2;					/* ���ڽNo.(�v���)*/
	char	pos_z2;
	char	len_z, len_z2, len_k;
	short	para_idx;
	char	top_m_pos = 0;			/* �����т̍ŏ�\���ԍ�(���ԑэő�̂ݎg�p) 0:��1������ */

	/* �����͑�P�����̌n */
	pRAT = CPrmSS[S_RAT];
	seg = S_RAT;
	no = 1;
	kind = 0;
	type = GET_LIM_TYP(kind, &max_set, &param_pos);			// �ő嗿���^�C�v��max_set�擾
	calc = (char)CPrmSS[S_CAL][1];
	if (calc) {
		/* �����т̎���"�Q��̓����ő�"�͔F�߂Ă��Ȃ�("�����w������ő�"�Ƃ���) */
		if (type < 0 || type  > 2)
			type = 2;
	}
	if(!RYO_CAL_EXTEND){				// �����v�Z�g������H
		lim_sec = S_CAL;									// �ő嗿���̎��ԁE�񐔂̐ݒ�ʒu
	}
	else {
		lim_sec = S_CLX;
	}
	if(type == SP_MX_N_MHOUR_W){
		get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);			// �J��Ԃ��񐔂̐ݒ�擾
		get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);			// �ő�P�̐ݒ�擾
		get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);		// �ő�Q�̐ݒ�擾
	}
	else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){
		get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);			// �J��Ԃ��񐔂̐ݒ�擾
		get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);		// �ő厞�Ԃ̐ݒ�擾
	}
	time_count = -1;							// ����/��(���͒l)

	dispclr();

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[1] );			/* "���ő嗿�����@�@�@�@�@�@�@�@�@" */
	if (calc == 0) {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[14]);		/* "�i����сj" */
	}
	else {
		grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[15]);		/* "�i�����сj" */
	}

	grachr( 1, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );		/* "��@�����̌n" ���] */
	opedsp( 1, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* �̌n(1-3)      ���] */
	grachr( 1, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0] );		/* "�|" */
	grachr( 1, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L) */
	grachr( 1, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );		/* "��" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[7] );		/* "��΍ő嗿���F�@�@�@�@�@�@�O�~" */
	mnylim_type_disp( type ,top_m_pos);		/* �ő嗿���^�C�v���̕\�� */
	line_pos = 1;							/* ��ʕ\���J�n�ʒu */

	mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
	Fun_Dsp( FUNMSG[25] );						/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */

	set = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( msg ) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F3:					/* ���ԉ�(F3) */
			/* ���߂�4���A�ő嗿���ҏW�����A���ԉ��ҏW���̏ꍇ */						
			if(((type == SP_MX_N_MHOUR_W) && (set == 2 || set == 3 || set == 4 || set == 5)) ||		// �����E�񐔐ݒ蒆
				(type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW )&& (set == 2 || set == 3 || set == 4)){
				if(type == SP_MX_N_MHOUR_W){							// �Q��ނ̂����Ԃ���ő�
					Fun_Dsp( FUNMSG2[45] );				/* "�@���@�@���@�ő�Q ����  �I�� " */
				}
				else {
					Fun_Dsp( FUNMSG2[2] );				/* "�@���@�@���@       ����  �I�� " */
				}
				time_count = -1;			/* ���ԉ�ݒ�̈�������� */						
				set = 7;					/* ��Ԃ����ԉ�ҏWӰ�ނɐݒ� */
				mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
				BUZPI();
				break;
			}
		default:
			/* �����̌n �ݒ� */
			if (set == 0) {
				switch ( msg ) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					no--;
					switch(no){
						case 0:
							no = 9;			// ��P����X�����̌n
							seg = S_CLB;		// �Z�N�V����59
							break;
						case 3:				// ��S����R�����̌n
							seg = S_RAT;		// �Z�N�V����31
							break;
						case 6:				// ��V����U�����̌n
							seg = S_CLA;		// �Z�N�V����58
							break;
						default:
							break;
					}
					pRAT = CPrmSS[seg];		// �ݒ�ʒu�X�V
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	/* �̌n(1-3) ���] */
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
					break;
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					no++;
					switch(no){
					case 4:				// ��R����S�����̌n
						seg = S_CLA;		// �Z�N�V����58
						break;
					case 7:				// ��U����V�����̌n
						seg = S_CLB;		// �Z�N�V����59
						break;
					case 10:			
						no = 1;			// ��X����P�����̌n
						seg = S_RAT;		// �Z�N�V����31
						break;
					default:
						break;
					}
					pRAT = CPrmSS[seg];	// �ݒ�ʒu�X�V
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �̌n(1-3) ���] */
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr( line_pos, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );	/* "��@�����̌n" ���] */
					opedsp( line_pos, 2, (ushort)no, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �̌n(1-3)      ���] */
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
					grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "��"           ���] */
					Fun_Dsp( FUNMSG[28] );						/* "  ��    ��    ��   �Ǐo  �I�� " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* ���(A-L) �ݒ� */
			else if (set == 1) {
				switch ( msg ) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					if (--kind < 0)
						kind = 11;
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
					if(RYO_CAL_EXTEND){								// �����v�Z�g������
						type = GET_LIM_TYP( kind ,&max_set, &param_pos);				// ��ʖ��̍ő嗿���^�C�v�擾
						if(type == SP_MX_N_MHOUR_W){													// �Q��ނ̂����Ԃ���ő�
							get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);		// �ő�P�̐ݒ�擾
							get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);		// �񐔂̐ݒ�擾
							get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);	// �ő�Q�̐ݒ�擾
						}
						else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){						// �����Ԃ���ő�
							get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);					// �񐔂̐ݒ�擾
							get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);				// �ő厞�Ԃ̐ݒ�擾
						}
					}
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
					break;
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (++kind > 11)
						kind = 0;
					grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
					if(RYO_CAL_EXTEND){								// �����v�Z�g������
						type = GET_LIM_TYP( kind ,&max_set, &param_pos);				// ��ʖ��̍ő嗿���^�C�v�擾
						if(type == SP_MX_N_MHOUR_W){													// �Q��ނ̂����Ԃ���ő�
							get_mnylim_kaisuu_adr(SP_MX_N_MH_NEW, kind, &adr_k, &len_k, &pos_k);		// �ő厞�ԂP�̐ݒ�擾
							get_mnylim_zikann_adr(SP_MX_N_MH_NEW, kind, &adr_z, &len_z, &pos_z);		// �񐔂̐ݒ�擾
							get_mnylim_zikann_adr(SP_MX_N_MHOUR_W, kind, &adr_z2, &len_z2, &pos_z2);	// �ő厞�ԂQ�̐ݒ�擾
						}
						else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){						// �����Ԃ���ő�
							get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);					// �񐔂̐ݒ�擾
							get_mnylim_zikann_adr(type, kind, &adr_z2, &len_z2, &pos_z2);				// �ő厞�Ԃ̐ݒ�擾
						}
					}
					mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr( line_pos, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0] );	/* "��@�����̌n" ���] */
					opedsp( line_pos, 2, (ushort)no, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �̌n(1-3)      ���] */
					grachr( line_pos, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
					grachr( line_pos, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "��"           ���] */
					Fun_Dsp( FUNMSG[25] );						/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr( line_pos, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
					grachr( line_pos, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "��"           ���] */
					if( type == SP_MX_BAND ){		// ���ԑя��
						Fun_Dsp( FUNMSG[20] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */
					} else if(type != SP_MX_N_HOUR && type != SP_MX_N_MH_NEW && type != SP_MX_N_MHOUR_W){	/* ���߂��S�E�P�O�E�Q�O�ȊO�������ꍇ */
						Fun_Dsp( FUNMSG2[2] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */
					}else{
						Fun_Dsp( FUNMSG2[40] );					/* "�@���@�@���@���ԁ@ ����  �I�� " */
					}
					mnylim_dsp(type, no, kind, 0 ,top_m_pos);				/* ���z�\�� */
					money = -1;
					set = 2;
					break;
				default:
					break;
				}
			}
			else if(set == 7){									/* ���ԉ�ҏW���[�h */
				if( type == SP_MX_BAND ){	// ���ԑя��
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F1:	/* ��(F1) */
						--set;
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
						money = -1;
						break;
					case KEY_TEN_F4:	/* ����(F4) */
						para_idx = 0;
						if (money != -1) {
							para_idx = GET_LIM_IDX(no, kind)+(param_pos+4);	/* ���ԑэő��T������ */
							pRAT[para_idx] = money*10;
							OpelogNo = OPLOG_SAIDAIRYOKIN;			// ���엚��o�^
							f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
							mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
						}
					case KEY_TEN_F2:	/* ��(F2) */
						++set;
						if( 1 == top_m_pos && 8 == set ){	// �ŏ�\������2�����т�set��8�ɂȂ����Ƃ��A�ŏ�\���͑�3�����тɂ���
							top_m_pos++;
						}
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
						money = -1;
						break;
					case KEY_TEN:		/* ����(�e���L�[) */
						BUZPI();
						if (money == -1)
							money = 0;

						money = (money*10 + (msg-KEY_TEN0)) % 100000L;
						opedpl( (ushort)((line_pos*2-2 - top_m_pos ) + set), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���� ���] */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2 - top_m_pos ) + set), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
						break;
					case KEY_TEN_CL:	/* ���(�e���L�[) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* ���z�\�� */
						money = -1;
						time_count = -1;
						break;
					default:
						break;
					}
				} else {
					switch ( KEY_TEN0to9(msg) ) {

					case KEY_TEN_F4:								/* ����(F4) */
						OpelogNo = OPLOG_SAIDAIRYOKIN;				// ���엚��o�^
					case KEY_TEN_F1:
					case KEY_TEN_F2:
					case KEY_TEN_F3:
						if(msg == KEY_TEN_F3 && type != SP_MX_N_MHOUR_W){
							break;
						}
						if( time_count != -1){				/* �ݷ��ł̓��͂�����ꍇ */	
							if( time_count == 0 || time_count > 24 ||					/* ���͒l���O�A�Q�S�ȏ�̏ꍇ */
								(type == SP_MX_N_MHOUR_W && msg == KEY_TEN_F4 && time_count >= prm_get(COM_PRM, lim_sec, adr_z, len_z, pos_z)) ){
								/* �v����ł͍ő�P���ő�Q�ƂȂ�悤�ɐݒ肷�� */
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
								time_count = -1;										/* ���ԉ���͗̈�������� */
								break;
							}
							if( msg == KEY_TEN_F4 ){		
								prm_set(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2, time_count);
								f_ParaUpdate.BIT.other = 1;								// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
								mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
							}
						}else{																					// ���͂��Ȃ��ꍇ
							if( msg != KEY_TEN_F3 && prm_get(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2) == 0){	// �ő�Q�̐ݒ肪�O�̎��̓G���[(�e�R�̂݃J�[�\���ړ���)
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
								time_count = -1;										/* ���ԉ���͗̈�������� */
								BUZPIPI();					
								break;
							}
						}	
						BUZPI();
						if(msg == KEY_TEN_F3 && type == SP_MX_N_MHOUR_W){				/* �v��� */
							set = 8;										/* �ő�Q�̐ݒ�� */
							Fun_Dsp( FUNMSG2[44] );							// "�@���@�@���@����P ����  �I�� "
						}
						else{
							set = 3;										/* �����ݒ�� */
							Fun_Dsp( FUNMSG2[40] );							// "�@���@�@���@ ����  ����  �I�� "
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
						time_count = -1;										/* ���ԉ���͗̈�������� */
						break;
					case KEY_TEN:		/* ����(�e���L�[) */
						BUZPI();
						if (time_count == -1){				/* �ݷ��ł̓��͂��Ȃ��ꍇ */
							time_count = 0;					/* ���͗̈�̏����� */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* �ݷ����͒l�̎擾 */
						if( time_count == 0 ){				/* ���͒l��0�̏ꍇ */
							grachr( (ushort)(line_pos*2+1), 0, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
						opedpl( (ushort)(line_pos*2+1), 0, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���͒l ���] */
						break;
					case KEY_TEN_CL:	/* ���(�e���L�[) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* ���z�\�� */
						time_count = -1;
						money = -1;
						break;
					default:
						break;
					}
				}
			}
			else if(set == 8){
				if( type == SP_MX_BAND ){	// ���ԑя��
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F1:	/* ��(F1) */
						--set;
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
						money = -1;
						break;
					case KEY_TEN_F4:	/* ����(F4) */
						para_idx = 0;
						if (money != -1) {
							BUZPI();
							para_idx = GET_LIM_IDX(no, kind)+(param_pos-13);	/* ���ԑэő��T������ */
							pRAT[para_idx] = money*10;
							OpelogNo = OPLOG_SAIDAIRYOKIN;			// ���엚��o�^
							f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
							mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
						}
						break;
					case KEY_TEN_F2:	/* ��(F2) */
						BUZPIPI();	// ��6�����т܂ł����J�ڂ����Ȃ�
						break;
					case KEY_TEN:		/* ����(�e���L�[) */
						BUZPI();
						if (money == -1)
							money = 0;

						money = (money*10 + (msg-KEY_TEN0)) % 100000L;
						opedpl( (ushort)((line_pos*2-2) + set - top_m_pos ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���� ���] */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set - top_m_pos ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
						break;
					case KEY_TEN_CL:	/* ���(�e���L�[) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* ���z�\�� */
						money = -1;
						time_count = -1;
						break;
					default:
						break;
					}
				} else {
					switch ( KEY_TEN0to9(msg) ) {
					case KEY_TEN_F4:								/* ����(F4) */
						OpelogNo = OPLOG_SAIDAIRYOKIN;				// ���엚��o�^
					case KEY_TEN_F1:								/* ��(F1) */
					case KEY_TEN_F2:								/* ��(F2) */
					case KEY_TEN_F3:								/* ��(F3) */
						if(time_count != -1){
							if( time_count == 0 || time_count > 24 ||		 				/* ���͒l��0 */
								(msg == KEY_TEN_F4 && time_count <= prm_get(COM_PRM, lim_sec, adr_z2, len_z2, pos_z2)) ){
								/* �v����ł͍ő�P���ő�Q�ƂȂ�悤�ɐݒ肷�� */
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
								time_count = -1;										/* ���ԉ���͗̈�������� */
								break;
							}
							if( msg == KEY_TEN_F4 ){		
								prm_set(COM_PRM, lim_sec, adr_z, len_z, pos_z, time_count);
								f_ParaUpdate.BIT.other = 1;								// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
								mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
							}
						}else{	/* ���Ԃ̏�����Ԃ�0�����Ԑݒ蒆�̏ꍇ */
							if( msg != KEY_TEN_F3  && (prm_get(COM_PRM, lim_sec, adr_z, len_z, pos_z) == 0)){
								BUZPIPI();					
								mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
								time_count = -1;										/* ���ԉ���͗̈�������� */
								break;
							}	
						}
						BUZPI();
						if(msg == KEY_TEN_F3){							
							set = 7;										// �ő�P�̐ݒ��
							Fun_Dsp( FUNMSG2[45] );							// "�@���@�@���@����Q ����  �I�� "
						}
						else{
							set = 4;										// �����ݒ��
							Fun_Dsp( FUNMSG2[40] );							// "�@���@�@���@ ����  ����  �I�� "
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ��ʕ\���ύX */
						time_count = -1;										/* ���ԉ���͗̈�������� */
						money = -1;												/* ���ԉ񗿋����͗̈�������� */
						break;
					case KEY_TEN:		/* ����(�e���L�[) */
						BUZPI();
						if (time_count == -1){				/* �ݷ��ł̓��͂��Ȃ��ꍇ */
							time_count = 0;					/* ���͗̈�̏����� */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* �ݷ����͒l�̎擾 */
						if( time_count == 0 ){				/* ���͒l��0�̏ꍇ */
							grachr( (ushort)(line_pos*2+2), 0, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
						opedpl( (ushort)(line_pos*2+2), 0, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���͒l ���] */
						break;
					case KEY_TEN_CL:	/* ���(�e���L�[) */
						BUZPI();
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* ���z�\�� */
						time_count = -1;
						money = -1;
						break;
					default:
						break;
					}
				}
			}
			/* �ő嗿�� �ݒ� */
			else {
				switch ( KEY_TEN0to9(msg) ) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					if (--set < 2) {
						/* ���(A-L) �ݒ�� */
						grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
						grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "��"           ���] */
						mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
						Fun_Dsp( FUNMSG[28] );						/* "  ��    ��    ��   �Ǐo  �I�� " */
					}
					else {
						if( type == SP_MX_BAND ){	// ���ԑя��
							// �ŏ�\������2�����т�set��3�ɂȂ����Ƃ��A�ŏ�\���͑�1�����тɂ���
							// �ŏ�\������3�����т�set��4�ɂȂ����Ƃ��A�ŏ�\���͑�2�����тɂ���
							if( ( 1 == top_m_pos && 3 == set ) || ( 2 == top_m_pos && 4 == set ) ){
								top_m_pos--;
							}
						}
						mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
						money = -1;
					}
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					para_idx = 0;
					if(type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW || type == SP_MX_N_MHOUR_W){			// �����Ԃ���ݒ�
						switch(set){
						case 2:										/* ��΍ő嗿�� */
							if(money != -1){						// ���͂��Ȃ��ꍇ
								para_idx = GET_LIM_IDX(no, kind);
							}
							break;
						case 3:
							if(money != -1){					// ���͂�����ꍇ
								if(type == SP_MX_N_MHOUR_W && money >= pRAT[GET_LIM_IDX(no, kind)+param_pos]/10){
									/* �v����ł͍ő�P���ő�Q�ƂȂ�悤�ɐݒ肷�� */
									BUZPIPI();
									mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
									money = -1;
									para_idx = -1;
									break;
								}
								else if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR){			// �����Ԃ���ő�̍ő嗿��
									para_idx = GET_LIM_IDX(no, kind)+param_pos;
								}
								else{
									para_idx = GET_LIM_IDX(no, kind)+param_pos+1;
								}
							}
							break;
						case 4:	
							if(type == SP_MX_N_MH_NEW || type == SP_MX_N_HOUR ){				// �����Ԃ���ő�̂Ƃ��͉񐔂̐ݒ�
								if( time_count != -1 ){		// �񐔂̓��͂�����ꍇ
									prm_set(COM_PRM, lim_sec, adr_k, len_k, pos_k, time_count);
								}
								break;
							}
							else{						// �v����ł͍ő�Q�ő嗿���̐ݒ�
								if(money != -1){		// ���͂�����ꍇ
									if(money <= pRAT[GET_LIM_IDX(no, kind)+param_pos+1]/10){
										/* �v����ł͍ő�P���ő�Q�ƂȂ�悤�ɐݒ肷�� */
										BUZPIPI();
										mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
										money = -1;
										para_idx = -1;
										break;
									}
									else{
										para_idx = GET_LIM_IDX(no, kind)+(param_pos);
									}
								}
							}
							break;
						case 5:			// �v����A�񐔂̐ݒ�
							if( time_count != -1 ){		// �񐔂̓��͂�����ꍇ
								prm_set(COM_PRM, lim_sec, adr_k, len_k, pos_k, time_count);
							}
							break;
						}
						if(para_idx == -1){			// ���͒l�G���[
							break;
						}
						else if(para_idx != 0){				// ���z�̓��͂�����ꍇ
							pRAT[para_idx] = money*10;
						}
						OpelogNo = OPLOG_SAIDAIRYOKIN;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
						time_count = -1;										/* ���ԉ���͗̈�������� */
					}
					else
					if (money != -1) {

						switch(set){
							case 2:							/* ��΍ő嗿�� */
								para_idx = GET_LIM_IDX(no, kind);
								break;
							case 3:							/* �����E24���ԍő嗿���E�����ő嗿���Q�E���ԑэő��P������ */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos);
								break;
							case 4:							/* �����E24���ԍő嗿���E�����Ԃ���ő嗿���E���ԑэő��Q������ */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+1);
								break;
							case 5:							/* ���ԑэő��R�����сE�����Ԃ���ő嗿���Q */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+2);
								break;
							case 6:							/* ���ԑэő��S������ */
								para_idx = GET_LIM_IDX(no, kind)+(param_pos+3);
								break;
						}
						pRAT[para_idx] = money*10;
						OpelogNo = OPLOG_SAIDAIRYOKIN;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					if( type == SP_MX_BAND ){	// ���ԑя��
						if (++set > max_set) {
							BUZPIPI();	// ��6�����т܂ł����J�ڂ����Ȃ�
						} else {
							if( 0 == top_m_pos && 7 == set ){	// �ŏ�\������1�����т�set��7�ɂȂ����Ƃ��A�ŏ�\���͑�2�����тɂ���
								top_m_pos++;
							}
							BUZPI();
							mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
							money = -1;
						}
					} else {					// ���ԑя���ȊO
						BUZPI();
						if (++set > max_set) {
							/* ���(A-L) �ݒ�� */
							set = 1;
							grachr( line_pos, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind] );	/* ���(A-L)      ���] */
							grachr( line_pos, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1] );	/* "��"           ���] */
							mnylim_dsp(type, no, kind, (char)-1 ,top_m_pos);		/* ���z�\�� */
							Fun_Dsp( FUNMSG[28] );						/* "  ��    ��    ��   �Ǐo  �I�� " */
						}
						else {
							if(type != SP_MX_N_HOUR && type != SP_MX_N_MH_NEW && type != SP_MX_N_MHOUR_W){
								Fun_Dsp( FUNMSG2[2] );					/* "  ��    ��         �Ǐo  �I�� " */
							}else{
								Fun_Dsp( FUNMSG2[40] );					/* "  ��    ��  ����   �Ǐo  �I�� " */
							}
							mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);	/* ���z�\�� */
							money = -1;
						}
					}
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if( ((type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW) && set == 4) ||			// ���ԉ�/�񐔐ݒ�
						(type == SP_MX_N_MHOUR_W && set == 5)){
						if (time_count == -1){				/* �ݷ��ł̓��͂��Ȃ��ꍇ */
							time_count = 0;					/* ���͗̈�̏����� */
						}
						time_count = (time_count*10 + (msg-KEY_TEN0)) % 100;	/* �ݷ����͒l�̎擾 */
						if( time_count == 0 ){				/* ���͒l��0�̏ꍇ */
							grachr( (ushort)((line_pos*2-2) + set), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
						opedpl( (ushort)((line_pos*2-2) + set), 24, (ulong)time_count, 2, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���͒l ���] */
						break;
					}

					if (money == -1)
						money = 0;

					money = (money*10 + (msg-KEY_TEN0)) % 100000L;
					if( type == SP_MX_BAND ){	// ���ԑя��
						opedpl( (ushort)((line_pos*2-2) + set - top_m_pos ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���� ���] */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set - top_m_pos ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
					} else {
						opedpl( (ushort)((line_pos*2-2) + set ), 16, (ulong)money, 5, 0, 1, COLOR_BLACK,  LCD_BLINK_OFF );	/* ���� ���] */
						if( money == 0 ){
							grachr( (ushort)((line_pos*2-2) + set ), 24, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@" */
						}
					}
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnylim_dsp(type, no, kind, (char)(set-2) ,top_m_pos);			/* ���z�\�� */
					money = -1;
					time_count = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  �ő嗿���\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnylim_dsp( type, no, kind, mod1, mod2, mod3 )          |*/
/*| PARAMETER    : char type    : �����ő嗿���^�C�v                       |*/
/*|              : char no      : �̌n(1-3)                                |*/
/*|              : char kind    : ���(A-L)                                |*/
/*|              : char rev_pos : ���]�ʒu                                 |*/
/*|              : char top_pos : �����т̍ŏ�\���ԍ�                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnylim_dsp(char type, char no, char kind, char rev_pos, char top_pos )
{
	long	money;
	ulong	time_count;
	ushort	rev1, rev2, rev3, rev4;
	ushort	line_pos;
	ushort	param_pos;
	short	adr_k, adr_z;			/* ���ڽNo.						*/
	char	pos_k, pos_z;			/* ���ʒu						*/
	short	seg;
	long	*pRAT;
	ushort	rev6, rev7;
	char	len_z,len_k;
	uchar	i = 0;		// for���p
	struct {
		short	no;
		ushort	rev;
	} revise[] = {
		{   0, (rev_pos == 1) ? 1 : 0},	// ��1������
		{   1, (rev_pos == 2) ? 1 : 0},	// ��2������
		{   2, (rev_pos == 3) ? 1 : 0},	// ��3������
		{   3, (rev_pos == 4) ? 1 : 0},	// ��4������
		{   4, (rev_pos == 5) ? 1 : 0},	// ��5������
		{ -13, (rev_pos == 6) ? 1 : 0}	// ��6������ ���ʃp�����[�^����6�̂ݕʂ̏ꏊ�ɐݒ肳��Ă��邽�߁A-13���Ă���
	};

	rev1 = (rev_pos == 0) ? 1 : 0;		// ��΍ő嗿��
	rev2 = (rev_pos == 1) ? 1 : 0;		// 
	rev3 = (rev_pos == 2) ? 1 : 0;		// 
	rev4 = (rev_pos == 3) ? 1 : 0;		// 
	rev6 = (rev_pos == 5) ? 1 : 0;		// 
	rev7 = (rev_pos == 6) ? 1 : 0;		// 
	/* �����̌n�ɂ���ĎQ�Ƃ���p�����[�^�̃Z�N�V�������w�肷�� */
	seg = GET_PARSECT_NO(no);
	pRAT = CPrmSS[seg];
	

	if(type != SP_MX_BAND){			// ���ԑэő�ȊO
		if( type == SP_MX_N_HOUR || type == SP_MX_N_MH_NEW || type == SP_MX_N_MHOUR_W){					// �����ԍő�
			if(!RYO_CAL_EXTEND){		// �����v�Z�g��
				seg = S_CAL;								// �Z�N�V����28
			}
			else {
				seg = S_CLX;								// �Z�N�V����56
			}
		}
		param_pos = 1;
	}else{
		param_pos = 15;
	}
	line_pos = 2;									// �J�n�s

	dispmlclr(3, 6);
	mnylim_type_disp( type  ,top_pos);
	money = pRAT[GET_LIM_IDX(no, kind)] / 10;
	opedpl( line_pos, 16, (ulong)money, 5, 0, rev1, COLOR_BLACK,  LCD_BLINK_OFF );		/* ��΍ő嗿�� */
	if (money == 0)
		grachr( line_pos, 24, 2, rev1, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
	if (type != 0) {
		if (type == 4 || type == 10) {
			get_mnylim_kaisuu_adr(type, kind, &adr_k, &len_k, &pos_k);
			get_mnylim_zikann_adr(type, kind, &adr_z, &len_z, &pos_z);
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos]/10;			/* �������Ұ��擾 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* �����E�Q�S���ԍő嗿���E�������ԁ����񗿋� */
			if (money == 0){
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* �������Ұ��擾 */
			opedpl( line_pos, 0, time_count, 2, 0, rev6, COLOR_BLACK,  LCD_BLINK_OFF );				/* ���ԉ�/���� */
			if (time_count == 0){
				/* �O�𔽓]�\��(�ԐF�\��)���邽�߁A�󔒂̔��]�͕s�v */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}
			time_count = prm_get(COM_PRM, seg, adr_k, len_k, pos_k);	/* �������Ұ��擾 */
			opedpl( ++line_pos, 24, time_count, 2, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* ���ԉ�/�� */
			if (time_count == 0){
				grachr( line_pos, 24, 4, rev3, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[36]);				/* "�@" */
			}
		}else if(type == SP_MX_N_MHOUR_W){																// �Q��ނ̂����Ԃ���ő�
			get_mnylim_zikann_adr(type, kind, &adr_z, &len_z, &pos_z);
			
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos+1]/10;			/* �������Ұ��擾 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* �����E�Q�S���ԍő嗿���E�������ԁ����񗿋� */
			if (money == 0){
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}
			
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* �������Ұ��擾 */
			opedpl( line_pos, 0, time_count, 2, 0, rev6, COLOR_BLACK,  LCD_BLINK_OFF );				/* �ő厞�ԂP */
			if (time_count == 0){
				/* �O�𔽓]�\��(�ԐF�\��)���邽�߁A�󔒂̔��]�͕s�v */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}
			
			get_mnylim_kaisuu_adr(10, kind, &adr_k, &len_k, &pos_k);
			get_mnylim_zikann_adr(10, kind, &adr_z, &len_z, &pos_z);
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos]/10;			/* �������Ұ��擾 */
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* �����E�Q�S���ԍő嗿���E�������ԁ����񗿋� */
			if (money == 0){
				grachr( line_pos, 24, 2, rev3, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}
		
			time_count = prm_get(COM_PRM, seg, adr_z, len_z, pos_z);	/* �������Ұ��擾 */
			opedpl( line_pos, 0, time_count, 2, 0, rev7, COLOR_BLACK,  LCD_BLINK_OFF );				/* �ő厞�ԂQ */
			if (time_count == 0){
				/* �O�𔽓]�\��(�ԐF�\��)���邽�߁A�󔒂̔��]�͕s�v */
				grachr( line_pos, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
			}

			time_count = prm_get(COM_PRM, seg, adr_k, len_k, pos_k);	/* �������Ұ��擾 */
			opedpl( ++line_pos, 24, time_count, 2, 0, rev4, COLOR_BLACK,  LCD_BLINK_OFF );			/* �J��Ԃ��� */
			if (time_count == 0){
				grachr( line_pos, 24, 4, rev4, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[36]);				/* "�@" */
			}
		} else if ( SP_MX_BAND == type ){			// ���ԑя���i�����я���j
			for( i = 0; i < 4 ; i++ ){
				money = pRAT[GET_LIM_IDX(no, kind)+param_pos + revise[ top_pos + i ].no] / 10;
				opedpl( ++line_pos, 16, (ulong)money, 5, 0, revise[ top_pos + i ].rev, COLOR_BLACK,  LCD_BLINK_OFF );	/* ��1�����̑� */
				if (money == 0)
					grachr( line_pos, 24, 2, revise[ top_pos + i ].rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
			}
		}else{
			money = pRAT[GET_LIM_IDX(no, kind)+param_pos] / 10;
			opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev2, COLOR_BLACK,  LCD_BLINK_OFF );		/* �����E�Q�S���ԍő嗿���E�������ԁ����񗿋� */
			if (money == 0)
				grachr( line_pos, 24, 2, rev2, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
			if (type == 3 || type == 5) {
				money = pRAT[GET_LIM_IDX(no, kind)+(param_pos+1)] / 10;
				opedpl( ++line_pos, 16, (ulong)money, 5, 0, rev3, COLOR_BLACK,  LCD_BLINK_OFF );			/* �����ő嗿���Q */
				if (money == 0)
					grachr( line_pos, 24, 2, rev3, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�T�[�r�X������                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnytic( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnytic(void)
{
	typedef struct {
		long	role;		/* ���� */
		long	data;		/* �f�[�^ */
		long	chng;		/* �ؑ� */
	} SRV_PRM;
	SRV_PRM	*psrv;
	ushort	msg;
	char	set;			/* �O:�����ݒ蒆�^���وʒu�����޽����� */
							/* �P:�����ݒ蒆�^���وʒu�����޽������ */
							/* �Q:�����ݒ蒆�^���وʒu���������e 	*/
							/* �R:�ؑ֐ݒ蒆�^���وʒu�����޽����� */
							/* �S:�ؑ֐ݒ蒆�^���وʒu���Ԏ�ؑ� 	*/

	int		kind;			/* ��� 0�`14=A�`O */
	long	role;
	long	data;
	long	chng;
	uchar	i;
	uchar	page;
	ushort	line;
	uchar	svs_syu;

	psrv = (SRV_PRM *)&CPrmSS[S_SER][1];

	dispclr();

	set = 0;
	kind = 0;

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[2]);									/* "���T�[�r�X���������@�@�@�@�@�@" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[12]);								/* " [��ʁ[����] �@�@ [�������e] " */

	for( i=0 ; i<5 ; i++){											/* ���޽A�`E�\���iA�픽�]�\���j		*/

		line	= 2+i;												/* �\���s��� 						*/

		if( i==0 ){
			grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);						/* ��� ���] 						*/
			grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "��" ���] 						*/
		}
		else{
			grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);						/* ��� ���] 						*/
			grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "��" ���] 						*/
		}
		grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "�[" 							*/
		grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[i].role]);				/* ���� 							*/
		mnytic_data_dsp( (char)i, psrv[i].role, psrv[i].data, 0 );	/* �������e 						*/
	}

	Fun_Dsp( FUNMSG2[3] );											/* "�@���@�@���@�@���@��ؑ� �I�� " */

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:											/* ���[�h�ؑ� 						*/

			BUZPI();
			return MOD_CHG;

		case KEY_TEN_F5:											/* �I��(F5) 						*/

			BUZPI();
			return MOD_EXT;

		default:													/* ���̑� 							*/
			/* ����-��ʐݒ� */
			if (set == 0) {

				/* �O:�����ݒ蒆�^���وʒu�����޽����� */

				switch (msg) {

				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */

					BUZPI();
					page = (uchar)(kind/5);												/* ���ݕ\�������߰�ގ擾�iA�`E=0,F�`J=1,K�`O=2�j*/
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 14;
					}
					else {
						if (++kind > 14)
							kind = 0;
					}
					if( page != kind/5 ){
						/* �߰�ޕύX���� */
						for( i=0 ; i<5 ; i++){											/* ���޽���T��\�� 					*/
							line	= 2+i;												/* �\���s��� 						*/
							svs_syu	= (uchar)((kind/5)*5+i);							/* ���޽����� 					*/

							if( i==(uchar)(kind%5) ){
								grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ���وʒu�F��� ���] 				*/
								grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* ���وʒu�F"��" ���] 				*/
							}
							else{
								grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ��� ���] 						*/
								grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "��" ���] 						*/
							}
							grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "�[" 							*/
							grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);			/* ���� 							*/
							mnytic_data_dsp( (char)svs_syu, psrv[svs_syu].role, psrv[svs_syu].data, 0);	/* �������e 		*/
						}
					}
					else{														
						/* �߰�ޕύX�Ȃ� */
						line	= (kind+2-((kind/5)*5));
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ���وʒu�F��� ���] 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ���وʒu�F"��" ���] 				*/

						if (msg == KEY_TEN_F1) {
							line	= ((kind+1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind+1);
						}
						else {
							line	= ((kind-1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind-1);
						}
						grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);						/* �O���وʒu�F��� ���] 			*/
						grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* �O���وʒu�F"��" ���] 			*/
					}
					break;

				case KEY_TEN_F3:	/* ��(F3) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ���وʒu�F��� ���] 				*/
					grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ���وʒu�F"��" ���] 				*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);					/* ����      ���] 					*/
					Fun_Dsp( FUNMSG2[4] );																				/* "�@���@�@���@ �ύX�@����  �I�� " */
					role = psrv[kind].role;
					set = 1;
					break;

				case KEY_TEN_F4:	/* ��ؑ�(F4) */

					BUZPI();
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[15]);									/* " [��ʁ[�Ԏ�ؑ�] �@�@�@�@�@�@" */

					svs_syu = (uchar)((kind / 5) * 5);
					for( i=0 ; i<5 ; i++ ){
						mnytic_chng_dsp( (char)(svs_syu+i), psrv[svs_syu+i].chng, 0 );	/* �ؑ� 							*/
					}
					Fun_Dsp( FUNMSG2[5] );												/* "�@���@�@���@�@���@ ����  �I�� " */
					set = 3;
					break;

				default:
					break;
				}
			}
			/* ����-�����ݒ� */
			else if (set == 1) {

				/* �P:�����ݒ蒆�^���وʒu�����޽������ */

				switch (msg) {

				case KEY_TEN_F1:	/* ��(F1) */

					BUZPI();

					for( i=0 ; i<5 ; i++){												/* ���޽���T��\�� 					*/

						line	= 2+i;													/* �\���s��� 						*/
						svs_syu	= (uchar)((kind/5)*5+i);								/* ���޽����� 					*/

						if( i==(uchar)(kind%5) ){
							grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);					/* ���وʒu�F��� ���] 				*/
							grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* ���وʒu�F"��" ���] 				*/
						}
						else{
							grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);					/* ��� ���] 						*/
							grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* "��" ���] 						*/
						}
						grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);								/* "�[" 							*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);				/* ���� 							*/
					}
					Fun_Dsp( FUNMSG2[3] );																				/* "�@���@�@���@�@���@��ؑ� �I�� " */
					set = 0;
					break;

				case KEY_TEN_F4:	/* ����(F4) */
					/* �������ύX���ꂽ����e���O�ɂ��� */
					if (psrv[kind].role != role)
						psrv[kind].data = 0;
					psrv[kind].role = role;
					OpelogNo = OPLOG_SERVICEYAKUWARI;									/* ���엚��o�^ 					*/
					f_ParaUpdate.BIT.other = 1;											// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

				case KEY_TEN_F2:	/* ��(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));									/* �\���s��� 						*/

					/* ���Ԋ����E���������̏ꍇ�F�������e�ݒ�� */
					if (psrv[kind].role == 1 || psrv[kind].role == 2) {
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);				/* ����      ���] 					*/
						mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );									/* ���e      ���] 					*/
						Fun_Dsp( FUNMSG2[2] );																			/* "�@���@�@���@ �@�@�@����  �I�� " */
						data = -1;
						set = 2;
					}
					/* ���ݒ�E�S�z�����̏ꍇ�F��ʐݒ�� */
					else {
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ���وʒu�F��� ���] 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ���وʒu�F"��" ���] 				*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);				/* ����      ���] 					*/
						mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );	/* ���e      ���] 					*/
						Fun_Dsp( FUNMSG2[3] );											/* "�@���@�@���@�@���@��ؑ� �I�� " */
						set = 0;
					}
					break;

				case KEY_TEN_F3:	/* �ύX(F3) */

					BUZPI();
					if (++role > 4)
						role = 0;
					if (role == 3)	/* "������"���X�L�b�v���� */
						role = 4;
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);								/* ����      ���] 					*/
					break;

				case KEY_TEN_CL:	/* ���(�e���L�[) */

					BUZPI();
					role = psrv[kind].role;
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);								/* ����      ���] 					*/
					break;

				default:
					break;
				}
			}
			/* ����-���e�ݒ� */
			else if (set == 2) {

				/* �Q:�����ݒ蒆�^���وʒu���������e 	*/

				switch (KEY_TEN0to9(msg)) {

				case KEY_TEN_F1:	/* ��(F1) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 8, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[kind].role]);					/* ����      ���] 					*/
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );										/* ���e      ���] 					*/
					Fun_Dsp( FUNMSG2[4] );																				/* "�@���@�@���@ �ύX�@����  �I�� " */
					role = psrv[kind].role;
					set = 1;
					break;

				case KEY_TEN_F4:	/* ����(F4) */

					if (data != -1) {
						/* ���Ԋ��� */
						if (psrv[kind].role == 1) {
							if ((data%100) > 59) {
								BUZPIPI();
								mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );	/* ���e  ���] 				*/
								data = -1;
								break;
							}
						}
						psrv[kind].data = data;
						OpelogNo = OPLOG_SERVICEYAKUWARI;								/* ���엚��o�^ 					*/
						f_ParaUpdate.BIT.other = 1;										// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}

				case KEY_TEN_F2:	/* ��(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ���وʒu�F��� ���] 				*/
					grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ���وʒu�F"��" ���] 				*/
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 0 );										/* ���e      ���] 					*/
					Fun_Dsp( FUNMSG2[3] );																				/* "�@���@�@���@�@���@��ؑ� �I�� " */
					set = 0;
					break;

				case KEY_TEN:		/* ����(�e���L�[) */

					BUZPI();
					/* ���Ԋ����F�ő�l9:59(959) */
					if (psrv[kind].role == 1) {
						if (data == -1)
							data = 0;
						data = (data*10 + msg-KEY_TEN0) % 10000;
					}
					/* ���������F�ő�l9990�~ */
					else {
						if (data == -1)
							data = 0;
						data = (data*10 + (msg-KEY_TEN0)*10) % 10000;
					}
					mnytic_data_dsp( kind, psrv[kind].role, data, 1 );					/* ���e  ���] 						*/
					break;

				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnytic_data_dsp( kind, psrv[kind].role, psrv[kind].data, 1 );		/* ���e  ���] 						*/
					data = -1;
					break;

				default:
					break;
				}
			}
			/* �Ԏ�-��ʐݒ� */
			else if (set == 3) {

				/* �R:�ؑ֐ݒ蒆�^���وʒu�����޽����� */

				switch (msg) {

				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */

					BUZPI();
					page = (uchar)(kind/5);												/* ���ݕ\�������߰�ގ擾�iA�`E=0,F�`J=1,K�`O=2�j*/
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 14;
					}
					else {
						if (++kind > 14)
							kind = 0;
					}
					if( page != kind/5 ){
						/* �߰�ޕύX���� */
						for( i=0 ; i<5 ; i++){											/* ���޽���T��\�� 					*/

							line	= 2+i;												/* �\���s��� 						*/
							svs_syu	= (uchar)((kind/5)*5+i);							/* ���޽����� 					*/

							if( i==(uchar)(kind%5) ){
								grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ���وʒu�F��� ���] 				*/
								grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* ���وʒu�F"��" ���] 				*/
							}
							else{
								grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);				/* ��� ���] 						*/
								grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);						/* "��" ���] 						*/
							}
							grachr( line, 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "�[" 							*/
							mnytic_chng_dsp( (char)svs_syu, psrv[svs_syu].chng, 0 );									/* �ؑ� 							*/
						}
					}
					else{														
						/* �߰�ޕύX�Ȃ� */
						line	= (kind+2-((kind/5)*5));
						grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ���وʒu�F��� ���] 				*/
						grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* ���وʒu�F"��" ���] 				*/

						if (msg == KEY_TEN_F1) {
							line	= ((kind+1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind+1);
						}
						else {
							line	= ((kind-1)+2-((kind/5)*5));
							svs_syu	= (uchar)(kind-1);
						}
						grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[svs_syu]);						/* �O���وʒu�F��� ���] 			*/
						grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* �O���وʒu�F"��" ���] 			*/
					}
					break;

				case KEY_TEN_F3:	/* ��(F3) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));																	/* �\���s��� 						*/
					grachr( line, 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ���(A-C) ���] 					*/
					grachr( line, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* "��"      ���] 					*/
					mnytic_chng_dsp( kind, psrv[kind].chng, 1 );														/* �ؑ� ���] 						*/
					Fun_Dsp( FUNMSG2[4] );																				/* "�@���@�@���@ �ύX�@����  �I�� " */
					chng = psrv[kind].chng;
					set = 4;
					break;

				case KEY_TEN_F4:	/* ����(F4) */

					BUZPI();
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[12] );									/* " [��ʁ[����] �@�@ [�������e] " */

					for( i=0 ; i<5 ; i++){												/* ���޽���T��\�� 					*/

						line	= 2+i;													/* �\���s��� 						*/
						svs_syu	= (uchar)((kind/5)*5+i);								/* ���޽����� 					*/
						grachr( line, 8, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[psrv[svs_syu].role]);				/* ���� */
						mnytic_data_dsp( (char)svs_syu, psrv[svs_syu].role, psrv[svs_syu].data, 0);	/* ���e 				*/
					}
					Fun_Dsp( FUNMSG2[3] );												/* "�@���@�@���@�@���@��ؑ� �I�� " */
					set = 0;
					break;

				default:
					break;
				}
			}
			/* �Ԏ�-�ؑ֐ݒ� */
			else {	/* if (set == 4) */

				/* �S:�ؑ֐ݒ蒆�^���وʒu���Ԏ�ؑ� 	*/

				switch (msg) {

				case KEY_TEN_F4:	/* ����(F4) */

					psrv[kind].chng = chng;
					OpelogNo = OPLOG_SERVICEYAKUWARI;									/* ���엚��o�^ 					*/
					f_ParaUpdate.BIT.other = 1;											// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */

					BUZPI();
					line	= (kind+2-((kind/5)*5));									/* �\���s��� 						*/
					grachr( line, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ���وʒu�F��� ���] 				*/
					grachr( line, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* ���وʒu�F"��" ���] 				*/
					mnytic_chng_dsp( kind, psrv[kind].chng, 0 );						/* �ؑ� ���] 						*/
					Fun_Dsp( FUNMSG2[5] );												/* "�@���@�@���@�@���@ ����  �I�� " */
					set = 3;
					break;

				case KEY_TEN_F3:	/* �ύX(F3) */

					BUZPI();
					if (++chng > 12) {
						chng = 0;
					}
					mnytic_chng_dsp( kind, chng, 1 );									/* �ؑ� ���] 						*/
					break;

				case KEY_TEN_CL:	/* ���(�e���L�[) */

					BUZPI();
					chng = psrv[kind].chng;
					mnytic_chng_dsp( kind, chng, 1 );									/* �ؑ� ���]						*/
					break;

				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �������e�\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytic_data_dsp( kind, role, data, rev )                |*/
/*| PARAMETER    : char   kind : ���(0:A 1:B 2:C)                         |*/
/*|              : long   role : ����(0-4)                                 |*/
/*|              : long   data : ���e                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytic_data_dsp(char kind, long role, long data, ushort rev)
{
	ushort	line;	/* �\���s */

	line	= (kind+2-((kind/5)*5));						/* �\���s��� 	*/
	grachr( line, 16, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);					/* "�@" 		*/
	/* ���� */
	if (role == 1) {
		grachr( line, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "�[" 		*/
		// ���Ԋ�����99:59�܂łƂ���
		opedsp( line, 18, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� 			*/
		grachr( line, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" 		*/
		opedsp( line, 24, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� 			*/
	}
	/* ���� */
	else if (role == 2) {
		grachr( line, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "�[" 		*/
		opedsp( line, 18, (ushort)(data/10), 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* ���� 		*/
		if (data/10 == 0) {
			grachr( line, 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" 		*/
		}
		opedsp( line, 24, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* "�O" 		*/
		grachr( line, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);					/* "�~" 		*/
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��ʐؑ֕\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytic_chng_dsp( kind, chng, rev )                      |*/
/*| PARAMETER    : char   kind : ���(0:A 1:B 2:C)                         |*/
/*|              : long   chng : �ؑ�                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytic_chng_dsp(char kind, long chng, ushort rev)
{
	ushort	line;	/* �\���s */

	line	= (kind+2-((kind/5)*5));					/* �\���s��� 	*/
	grachr( line, 8, 22, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" 		*/
	if (chng == 0) {
		grachr( line, 10, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[1]);			/* "�@�Ȃ�" 	*/
	}
	else {
		grachr( line, 10, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[0]);			/* "�@�Ԏ�" 	*/
		grachr( line, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[chng-1]);		/* A-L 			*/
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�X����                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnyshp( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnyshp(void)
{
	typedef struct {
		long	role;		/* ���� */
		long	data;		/* �f�[�^ */
		long	chng;		/* �ؑ� */
	} SHP_PRM;
	SHP_PRM	*pshp;
	ushort	msg;
	char	set;			/* 0:�X�ݒ蒆 1:�����ݒ蒆 2:���e�ݒ蒆 3:�ؑ֐ݒ蒆 */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
//	char	no;				/* �X�ԍ� 1-100 */
	short	no;				/* �X�ԍ� 1-999 */
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
	long	role;
	long	data;
	long	chng;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 	char	top;
	short	top;			/* �擪�X�ԍ� 1-995 */
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
	char	i;
	short	input_no;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
	short	no_max;			/* �X�ԍ��ő� */
	short	top_max;		/* �擪�X�ԍ��ő� */
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)

	pshp = (SHP_PRM *)&CPrmSS[S_STO][1];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[3]);				/* "���X�������@�@�@�@�@�@�@�@�@�@" */
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[17]);			/* "�@�@�@�@�@�y�����z�y�������e�z" */
	for (i = 0; i < 5; i++) {
		grachr((ushort)(2+i), 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[4]);						/* "�X" */
		opedsp((ushort)(2+i), 2, (ushort)(i+1), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� */
		grachr((ushort)(2+i), 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);						/* "�[" */
		grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[i].role]);			/* ���� */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 		mnyshp_data_dsp(1, (char)(i+1), pshp[i].role, pshp[i].data, 0);								/* ���e */
		mnyshp_data_dsp(1, (i+1), pshp[i].role, pshp[i].data, 0);									/* ���e */
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
	}
	opedsp(2, 2, 1, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);											/* "�O�O�P" ���] */
	Fun_Dsp( FUNMSG2[6] );						/* "  �{  �|�^��  ��  ��ؑ� �I�� " */

	set = 0;
	no = top = 1;
	input_no = -1;

// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
	if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {	// ���X�܊�������
		no_max  = T_MISE_NO_CNT;
		top_max = T_MISE_NO_CNT - 4;
	} else {										// ���X�܊����Ȃ�
		no_max  = MISE_NO_CNT;
		top_max = MISE_NO_CNT - 4;
	}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* ����-�X�ԍ��ݒ� */
			if (set == 0) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					no--;
					if (no < 1) {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						no = 100;
// 						top = 96;
						no = no_max;
						top = top_max;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					else if (no < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);	/* ���� */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* ���e */
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* ����(���g�p) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* ���e(��)*/
						} else {							// �X����
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* ���� */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* ���e */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� ���] */
					input_no = -1;
					break;
				case KEY_TEN_F2:	/* ���^��(F2) */
					if (input_no == -1) {
						no++;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						if (no > 100) {
						if ( no > no_max ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
							no = top = 1;
						}
						else if (no > top+4) {
							top++;
						}
					}
					else {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						if (input_no < 1 || input_no > 100) {
						if ( (input_no < 1) || (input_no > no_max) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
							BUZPIPI();
							opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �X�ԍ� ���] */
							input_no = -1;
							break;
						}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						no = top = (char)input_no;
// 						if (top > 96) {
// 							top = 96;
						no = top = input_no;
						if ( top > top_max ) {
							top = top_max;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
						}
					}
					BUZPI();
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);	/* ���� */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* ���e */
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* ����(���g�p) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* ���e(��)*/
						} else {							// �X����
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* ���� */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* ���e */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� ���] */
					input_no = -1;
					break;
				case KEY_TEN_F3:	/* ��(F3) */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					if ( no > MISE_NO_CNT ) {
						BUZPIPI();
						break;
					}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* �X�ԍ� ���] */
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);		/* ���� ���] */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);									/* ���e */
					Fun_Dsp( FUNMSG2[4] );																			/* "�@���@�@���@ �ύX�@����  �I�� " */
					role = pshp[no-1].role;
					set = 1;
					break;
				case KEY_TEN_F4:	/* ��ؑ�(F4) */
					BUZPI();
					grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[18]);			/* "�@�@�@�@�@�y�Ԏ�ؑցz�@�@�@�@" */
					for (i = 0; i < 5; i++) {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);				/* �ؑ� */
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* �ؑ� */
						} else {							// �X����
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* �ؑ� */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					Fun_Dsp( FUNMSG2[7] );														/* "  �{  �|�^��  ��   ����  �I�� " */
					input_no = -1;
					set = 3;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (input_no == -1)
						input_no = 0;
					input_no = (input_no*10 + (msg-KEY_TEN0)) % 1000;
					opedsp((ushort)(2+(no-top)), 2, (ushort)input_no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �X�ԍ� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					input_no = -1;
					break;
				default:
					break;
				}
			}
			/* ����-�����ݒ� */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* �X�ԍ� ���] */
					grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);		/* ����   ���] */
					Fun_Dsp( FUNMSG2[6] );															/* "  �{  �|�^��  ��  ��ؑ� �I�� " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					/* �������ύX���ꂽ����e���O�ɂ��� */
					if (pshp[no-1].role != role)
						pshp[no-1].data = 0;
					pshp[no-1].role = role;
					OpelogNo = OPLOG_MISEKUWARI;			// ���엚��o�^
					f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					/* ���Ԋ����E���������̏ꍇ�F�������e�ݒ�� */
					if (pshp[no-1].role == 1 || pshp[no-1].role == 2) {
						grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	/* ���� ���] */
						mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);									/* ���e ���] */
						Fun_Dsp( FUNMSG2[2] );													/* "�@���@�@���@ �@�@�@����  �I�� " */
						data = -1;
						set = 2;
					}
					/* ���ݒ�E�S�z�����̏ꍇ�F��ʐݒ�� */
					else {
						opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� ���] */
						grachr((ushort)(2+no-top), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	 /* ����   ���] */
						mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);	/* ���e   ���] */
						Fun_Dsp( FUNMSG2[6] );														/* "  �{  �|�^��  ��  ��ؑ� �I�� " */
						input_no = -1;
						set = 0;
					}
					break;
				case KEY_TEN_F3:	/* �ύX(F3) */
					BUZPI();
					if (++role > 4)
						role = 0;
					if (role == 3)	/* "������"���X�L�b�v���� */
						role = 4;
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);	/* ���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					role = pshp[no-1].role;
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[role]);	/* ���� ���] */
					break;
				default:
					break;
				}
			}
			/* ����-�������e�ݒ� */
			else if (set == 2) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					grachr((ushort)(2+no-top), 10, 8, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[no-1].role]);	/* ���� ���] */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);	/* ���e ���] */
					Fun_Dsp( FUNMSG2[4] );						/* "�@���@�@���@ �ύX�@����  �I�� " */
					role = pshp[no-1].role;
					set = 1;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						/* ���Ԋ��� */
						if (pshp[no-1].role == 1) {
							if ((data%100) > 59) {
								BUZPIPI();
								mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);	/* ���e ���] */
								data = -1;
								break;
							}
						}
						pshp[no-1].data = data;
						OpelogNo = OPLOG_MISEKUWARI;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� ���] */
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 0);									/* ���e   ���] */
					Fun_Dsp( FUNMSG2[6] );														/* "  �{  �|�^��  ��  ��ؑ� �I�� " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					/* ���Ԋ����F�ő�l9:59(959) */
					if (pshp[no-1].role == 1) {
						if (data == -1)
							data = 0;
						data = (data*10 + msg-KEY_TEN0) % 10000;
					}
					/* ���������F�ő�l9990�~ */
					else {
						if (data == -1)
							data = 0;
						data = (data*10 + (msg-KEY_TEN0)*10) % 10000;
					}
					mnyshp_data_dsp(top, no, pshp[no-1].role, data, 1);				/* ���e ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnyshp_data_dsp(top, no, pshp[no-1].role, pshp[no-1].data, 1);	/* ���e ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* ��ؑ�-�X�ԍ��ݒ� */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* �{(F1) */
					BUZPI();
					no--;
					if (no < 1) {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						no = 100;
// 						top = 96;
						no = no_max;
						top = top_max;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					else if (no < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);								/* �ؑ� */
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* �ؑ� */
						} else {							// �X����
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* �ؑ� */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					input_no = -1;
					break;
				case KEY_TEN_F2:	/* �|�^��(F2) */
					if (input_no == -1) {
						no++;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						if (no > 100) {
						if ( no > no_max ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
							no = top = 1;
						}
						else if (no > top+4) {
							top++;
						}
					}
					else {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						if (input_no < 1 || input_no > 100) {
						if ( (input_no < 1) || (input_no > no_max) ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
							BUZPIPI();
							opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �X�ԍ� ���] */
							input_no = -1;
							break;
						}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						no = top = (char)input_no;
// 						if (top > 96) {
// 							top = 96;
						no = top = input_no;
						if ( top > top_max ) {
							top = top_max;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
						}
					}
					BUZPI();
					for (i = 0; i < 5; i++) {
						opedsp((ushort)(2+i), 2, (ushort)(top+i), 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						mnyshp_chng_dsp(top, (char)(top+i), pshp[top+i-1].chng, 0);								/* �ؑ� */
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							mnyshp_chng_dsp(top, (top+i), prm_get(COM_PRM, S_TAT, ((top + i) - 69), 2, 1), 0);	/* �ؑ� */
						} else {							// �X����
							mnyshp_chng_dsp(top, (top+i), pshp[top+i-1].chng, 0);								/* �ؑ� */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					input_no = -1;
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �X�ԍ� ���] */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 					mnyshp_chng_dsp(top, no, pshp[no-1].chng, 1);			/* �ؑ� ���] */
// 					Fun_Dsp( FUNMSG2[4] );						/* "�@���@�@���@ �ύX�@����  �I�� " */
// 					chng = pshp[no-1].chng;
					if ( no > MISE_NO_CNT ) {	// ���X�܊���
						chng = prm_get(COM_PRM, S_TAT, (no - 69), 2, 1);
					} else {					// �X����
						chng = pshp[no-1].chng;
					}
					mnyshp_chng_dsp(top, no, chng, 1);			/* �ؑ� ���] */
					Fun_Dsp( FUNMSG2[4] );						/* "�@���@�@���@ �ύX�@����  �I�� " */
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					set = 4;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					BUZPI();
					grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[17]);			/* "�@�@�@�@�@�y�����z�y�������e�z" */
					for (i = 0; i < 5; i++) {
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// 						grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* ���� */
// 						mnyshp_data_dsp(top, (char)(top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);
						if ( (top + i) > MISE_NO_CNT ) {	// ���X�܊���
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);					/* ����(���g�p) */
							grachr((ushort)(2+i), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* ���e(��)*/
						} else {							// �X����
							grachr((ushort)(2+i), 10, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[pshp[top+i-1].role]);/* ���� */
							mnyshp_data_dsp(top, (top+i), pshp[top+i-1].role, pshp[top+i-1].data, 0);				/* ���e */
						}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					Fun_Dsp( FUNMSG2[6] );						/* "  �{  �|�^��  ��  ��ؑ� �I�� " */
					input_no = -1;
					set = 0;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (input_no == -1)
						input_no = 0;
					input_no = (input_no*10 + (msg-KEY_TEN0)) % 1000;
					opedsp((ushort)(2+(no-top)), 2, (ushort)input_no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �X�ԍ� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					opedsp((ushort)(2+(no-top)), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);			/* �X�ԍ� ���] */
					input_no = -1;
					break;
				default:
					break;
				}
			}
			/* ��ؑ�-��ʐؑ֐ݒ� */
			else {	/* if (set == 4) */
				switch (msg) {
				case KEY_TEN_F4:	/* ����(F4) */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					if ( no > MISE_NO_CNT ) {	// ���X�܊���
						prm_set(COM_PRM, S_TAT, (no - 69), 2, 1, chng);
					} else {					// �X����
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
						pshp[no-1].chng = chng;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					OpelogNo = OPLOG_MISEKUWARI;			// ���엚��o�^
					f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					opedsp((ushort)(2+no-top), 2, (ushort)no, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �X�ԍ� ���] */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					if ( no > MISE_NO_CNT ) {	// ���X�܊���
						mnyshp_chng_dsp(top, no, prm_get(COM_PRM, S_TAT, (no - 69), 2, 1), 0);	/* �ؑ�   ���] */
					} else {					// �X����
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
						mnyshp_chng_dsp(top, no, pshp[no-1].chng, 0);							/* �ؑ�   ���] */
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					Fun_Dsp( FUNMSG2[7] );						/* "  �{  �|�^��  ��   ����  �I�� " */
					input_no = -1;
					set = 3;
					break;
				case KEY_TEN_F3:	/* �ύX(F3) */
					BUZPI();
					if (++chng > 12) {
						chng = 0;
					}
					mnyshp_chng_dsp(top, no, chng, 1);		/* �ؑ� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					if ( no > MISE_NO_CNT ) {	// ���X�܊���
						chng = prm_get(COM_PRM, S_TAT, (no - 69), 2, 1);
					} else {					// �X����
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
						chng = pshp[no-1].chng;
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
					mnyshp_chng_dsp(top, no, chng, 1);		/* �ؑ� ���] */
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �������e�\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnyshp_data_dsp( top, no, role, data, rev )             |*/
/*| PARAMETER    : short  top  : �擪�X�ԍ�                                |*/
/*|              : short  no   : �X�ԍ�                                    |*/
/*|              : long   role : ����                                      |*/
/*|              : long   data : ���e                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// static void	mnyshp_data_dsp(char top, char no, long role, long data, ushort rev)
static void	mnyshp_data_dsp(short top, short no, long role, long data, ushort rev)
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
{
	grachr((ushort)(2+no-top), 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);					/* "�@" */
	/* ���� */
	if (role == 1) {
		grachr((ushort)(2+no-top), 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�[" */
		opedsp((ushort)(2+no-top), 20, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
		grachr((ushort)(2+no-top), 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
		opedsp((ushort)(2+no-top), 26, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	}
	/* ���� */
	else if (role == 2) {
		grachr((ushort)(2+no-top), 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�[" */
		opedsp((ushort)(2+no-top), 20, (ushort)(data/10), 3, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* ���� */
		if (data/10 == 0) {
			grachr((ushort)(2+no-top), 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
		}
		opedsp((ushort)(2+no-top), 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);						/* "�O" */
		grachr((ushort)(2+no-top), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);				/* "�~" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ�ؑ֕\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnyshp_chng_dsp( top, no, chng, rev )                   |*/
/*| PARAMETER    : short  top  : �擪�X�ԍ�                                |*/
/*|              : short  no   : �X�ԍ�                                    |*/
/*|              : long   chng : �ؑ�                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
// static void	mnyshp_chng_dsp(char top, char no, long chng, ushort rev)
static void	mnyshp_chng_dsp(short top, short no, long chng, ushort rev)
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�X������ʂ̕ύX)
{
	grachr((ushort)(2+no-top), 10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
	if (chng == 0) {
		grachr((ushort)(2+no-top), 12, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[1]);			/* "�@�Ȃ�" */
	}
	else {
		grachr((ushort)(2+no-top), 12, 6, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[0]);			/* "�@�Ԏ�" */
		grachr((ushort)(2+no-top), 12, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[chng-1]);		/* A-L */
	}
}

// MH810105(S) MH364301 �C���{�C�X�Ή�
///*[]----------------------------------------------------------------------[]*/
///*|  �����ݒ�F����ł̐ŗ�                                                |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_mnytax( void )                                    |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//unsigned short	UsMnt_mnytax(void)
//{
//	ushort	msg;
//	long	tax;		/* �ŗ� */
//	char	changing;	/* 1:�ŗ��ݒ蒆 */
//	long	*ptax;
//
//	ptax = &CPrmSS[S_CAL][19];
//
//	dispclr();
//
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );			/* "������ł̐ŗ����@�@�@�@�@�@�@" */
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[13] );			/* "�@���݂̐ŗ��F�@�@�@�@�@�@�@�@" */
//	if (*ptax == 0) {
//		grachr( 2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );		/* "����łȂ��@" */
//	}
//	else {
//		mnytax_dsp(*ptax, 0);						/* �ŗ��\�� */
//	}
//	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[14] );			/* "�@�@���͔͈͂͂O�`�X�X�D�X�X�@" */
//	Fun_Dsp( FUNMSG2[0] );														/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
//
//	changing = 0;
//	for ( ; ; ) {
//		msg = StoF( GetMessage(), 1 );
//		switch ( KEY_TEN0to9(msg) ) {
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		case LCD_DISCONNECT:
//			return MOD_CUT;
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		case KEY_MODECHG:
//			BUZPI();
//			return MOD_CHG;
//		case KEY_TEN_F5:		/* �I��(F5) */
//			BUZPI();
//			return MOD_EXT;
//		case KEY_TEN_F3:		/* �ύX(F3) */
//			if (changing == 0) {
//				BUZPI();
//				mnytax_dsp(*ptax, 1);		/* �ŗ��\�� ���] */
//				Fun_Dsp( FUNMSG2[1] );		/* "�@�@�@�@�@�@�@�@�@ ����  �I�� " */
//				changing = 1;
//				tax = -1;
//			}
//			break;
//		case KEY_TEN_F4:		/* ����(F4) */
//			if (changing == 1) {
//				BUZPI();
//				if (tax != -1) {
//					*ptax = tax;	/* �p�����[�^�X�V */
//				}
//				/* �\�� */
//				if (*ptax == 0) {
//					grachr( 2, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	/* "����łȂ��@" */
//				}
//				else {
//					mnytax_dsp(*ptax, 0);				/* �ŗ��\�� */
//				}
//				Fun_Dsp( FUNMSG2[0] );			/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
//				changing = 0;
//				OpelogNo = OPLOG_SHOHIZEI;			// ���엚��o�^
//				f_ParaUpdate.BIT.other = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
//			}
//			break;
//		case KEY_TEN:			/* ����(�e���L�[) */
//			if (changing == 1) {
//				BUZPI();
//				if (tax == -1)
//					tax = 0;
//				tax = (tax*10 + (msg-KEY_TEN0)) % 10000;
//				mnytax_dsp(tax, 1);				/* �ŗ��\�� ���] */
//			}
//			break;
//		case KEY_TEN_CL:		/* ���(�e���L�[) */
//			if (changing == 1) {
//				BUZPI();
//				mnytax_dsp(*ptax, 1);				/* �ŗ��\�� ���] */
//				tax = -1;
//			}
//			break;
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �ŗ��\��                                                              |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : mnytax_dsp( tax, rev )                                  |*/
///*| PARAMETER    : long  tax  : �ŗ�                                       |*/
///*|              : ushort rev : 0:���] 1:���]                              |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	mnytax_dsp(long tax, ushort rev)
//{
//	opedsp( 2, 16, (ushort)(tax/100), 2, 0, rev , COLOR_BLACK, LCD_BLINK_OFF);	/* �������� */
//	grachr( 2, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[3] );				/* "�D" */
//	opedsp( 2, 22, (ushort)(tax%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	/* �������� */
//	grachr( 2, 26, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );				/* "��" */
//}
// �C���{�C�X�p�ɍ�蒼��
//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�F����ł̐ŗ�
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///					        MOD_EXT : F5 key
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
unsigned short	UsMnt_mnytax(void)
{
	ushort	msg;
	int		changing;			// 1:�ҏW�� 
	long	*ptax;				// 18-0077 �K�p�ŗ�
	long	*pdate;				// 18-0078 �ŗ��ύX���
	short	in_val=-1;			// ���͒l(2��)�A-1�͖����͏��
	int		cur_pos=0;			// �J�[�\���ʒu: �ύX�O�ŗ�    =0
								//               �ύX��ŗ�    =1
								//               �ύX���(�N)=2
								//               �ύX���(��)=3
								//               �ύX���(��)=4
	ushort	cur_val[5];			// �J�[�\���ʒu�̓��͒l
	ushort	cur_val_tmp;		// F4�������ɕύX�������t���G���[�̏ꍇ�ɔ����Č��̓��͒l��ޔ�����G���A

	ptax  = &CPrmSS[S_PRN][77];	// 18-0077 �K�p�ŗ� �擾
	pdate = &CPrmSS[S_PRN][78];	// 18-0078 �ŗ��ύX��� �擾

	dispclr();

	// �����l���擾
	cur_val[0] = (ushort)prm_get(COM_PRM, S_PRN, 77, 2, 3);	// �ύX�O�ŗ�(18-0077�B�C)�����o��
	cur_val[1] = (ushort)prm_get(COM_PRM, S_PRN, 77, 2, 1);	// �ύX��ŗ�(18-0077�D�E)�����o��
	cur_val[2] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 5);	// �N(18-0078�@�A)�����o��
	cur_val[3] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 3);	// ��(18-0078�B�C)�����o��
	cur_val[4] = (ushort)prm_get(COM_PRM, S_PRN, 78, 2, 1);	// ��(18-0078�D�E)�����o��

	// ����ł̐ŗ� �����\��
	mnytax_dspinit_dsp(cur_val);	// �ύX�O�ŗ��A�ύX��ŗ��A�ύX���
	Fun_Dsp( FUNMSG2[0] );			// "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� "

	changing = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( KEY_TEN0to9(msg) ) {
// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	// �I��(F5)
			BUZPI();
			if (changing == 1) {
				mnytax_dspinit_dsp(cur_val);								// �ύX�O�ŗ��A�ύX��ŗ��A�ύX���
				Fun_Dsp( FUNMSG2[0] );										// "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� "
				changing = 0;												// 
			} else {
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F1:	// �J�[�\���ړ�
		case KEY_TEN_F2:
			if (changing == 0) {
				break;
			}
			if((cur_pos >= 2) && (cur_pos <= 4)){							// �ύX�������ړ�����ꍇ
				if( in_val != -1 ){											// ���l����͂����ꍇ�̓`�F�b�N����
					if( Date_Check( in_val, cur_pos ) != 0 ) {				// �ύX����̓��͒l�͈̓`�F�b�N
						BUZPIPI();
						in_val = -1;
						break;
					}
				}
			}
			BUZPI();
			if( in_val != -1 ){
				cur_val[cur_pos] = in_val;												// ���͂����ꍇ�͓��͒l���Z�b�g
			}
			mnytax_dsp(cur_val[cur_pos], cur_pos, 0);									// ���͒l��\�� ���]
			cur_pos = get_next_curpos(msg, cur_pos);									// ���̃J�[�\���ʒu���擾
			in_val = -1;
			break;
		case KEY_TEN_F3:	// �ύX(F3)
			if (changing == 0) {
				BUZPI();
				cur_pos = 0;															// �J�[�\���ʒu�������ʒu�ɃZ�b�g����
				mnytax_editinit_dsp(cur_val);											// �ύX�O�ŗ��A�ύX��ŗ��A�ύX���
				Fun_Dsp( FUNMSG[20] );													// "�@���@�@���@�@�@�@ ����  �I�� "
				changing = 1;															// �ҏW��
				in_val = -1;															// �����͏��
			}
			break;
		case KEY_TEN_F4:	// ����(F4)
			if (changing == 1) {
				// ���ꂩ�̃J�[�\���œ��͍ς݂̏�Ԃŏ����̏ꍇ�͂��̒l�𔽉f���Ă���ݒ菈�����s��
				if( in_val != -1 ){
					cur_val_tmp = cur_val[cur_pos]; 									// �m�肵�Ă�����͒l��ޔ�����
					cur_val[cur_pos] = in_val;											// ���݂̓��͒l�𔽉f����
				}

				if( date_exist_check((short)(2000 + cur_val[2]), (short)cur_val[3], (short)cur_val[4]) != 0 ){	// ���݂�����t���`�F�b�N
					BUZPIPI();
					if( in_val != -1 ){													// �J�[�\��
						cur_val[cur_pos] = cur_val_tmp;									// ���ɒl�ɖ߂�
					}
					in_val = -1;
				} else {
					BUZPI();
					mnytax_dspinit_dsp(cur_val);												// �ύX�O�ŗ��A�ύX��ŗ��A�ύX���
					Fun_Dsp( FUNMSG2[0] );														// "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� "
					*ptax  = (cur_val[0] * 100) + cur_val[1];									// �ŗ��p�����[�^�X�V
					*pdate = (((long)cur_val[2]) * 10000) + (cur_val[3] * 100) + cur_val[4];	// �ύX����p�����[�^�X�V
					changing = 0;																// �ҏW����
					OpelogNo = OPLOG_SHOHIZEI;													// ���엚��o�^
					f_ParaUpdate.BIT.other = 1;													// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
				}
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			}
			break;
		case KEY_TEN:	// ����(�e���L�[)
			if (changing == 1) {
				BUZPI();
				if (in_val == -1){
					in_val = 0;
				}
				in_val = (short)( in_val % 10 ) * 10 + (short)(msg - KEY_TEN0);			// 2�����͂𐔎��ɕϊ�
			}
			break;
		case KEY_TEN_CL:	// ���(�e���L�[)
			if (changing == 1) {
				BUZPI();
				in_val = -1;
			}
			break;
		default:
			break;
		}

		if( (changing == 1) && ((msg == KEY_TEN_F1) || (msg == KEY_TEN_F2) || (msg == KEY_TEN_F3) || (msg == KEY_TEN_F4) || (KEY_TEN0to9(msg) == KEY_TEN) || (msg == KEY_TEN_CL) ) ){
			// �J�[�\�����͒l�\��
			if( in_val == -1 ){
				mnytax_dsp(cur_val[cur_pos], cur_pos, 1);	// ���͑O�̐ݒ�l��\�� ���]
			} else {
				mnytax_dsp((ushort)in_val, cur_pos, 1);		// ���͒l��\�� ���]
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���t���݃`�F�b�N�֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		y  : �N
///	@param[in]		m  : ��
///	@param[in]		d  : ��
///	@return			0  : ���݂�����t   -1 : ���݂��Ȃ����t
///	@author			
///	@note			None
///	@attention		chkdate(yyyy,mm,dd)���ƌ��A���̓��͒l��0�̏ꍇ�ł�
///					�ŏ��l��ݒ肵�Ă��܂��̂ŁA�{�֐��ł͓��͒l0���G���[
///					�ɂ���B���������ׂ�0�͋��e����B
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short	date_exist_check(short y, short m, short d)
{
	if ( y == 2000 && m == 0 && d == 0 ) {		// 00�N00��00���͋��e
		return ( 0 );
	}
	if( m == 0 ){
		return (-1);							// ����0�̏ꍇ�͑��݂��Ȃ����Ƃ��ĕԂ�
	}
	if( d == 0 ){
		return (-1);							// ����0�̏ꍇ�͑��݂��Ȃ����Ƃ��ĕԂ�
	}
	return chkdate( y, m, d);					// ���݂�����t���ǂ����`�F�b�N
}
//[]----------------------------------------------------------------------[]
///	@brief			���J�[�\���ʒu�Z�o
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		:F1/F2�L�[���b�Z�[�W
///	@param[in]		pos		:���݂̃J�[�\���ʒu
///	@return			���̃J�[�\���ʒu
///	@author			
///	@note			None
///	@attention		�J�[�\���ʒu: �ύX�O�ŗ�    =1
///					              �ύX��ŗ�    =2
///					              �ύX���(�N)=3
///					              �ύX���(��)=4
///					              �ύX���(��)=5
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static int	get_next_curpos(ushort msg, int pos)
{
	int next_pos;

	next_pos = pos;				// ���݂̃J�[�\���ʒu���Z�b�g

	if (msg == KEY_TEN_F1) {
		switch(pos){
		case 1:
		case 2:
		case 3:
		case 4:
			next_pos--;			// �J�[�\���ʒu���ЂƂO�Ɉړ�����
			break;
		case 0:
			next_pos = 4;		// �J�[�\���ʒu��ύX���(��)�Ɉړ�����
			break;
		default:
			break;
		}
	} else {	// F2
		switch(pos){
		case 0:
		case 1:
		case 2:
		case 3:
			next_pos++;			// �J�[�\���ʒu���ЂƂ�Ɉړ�����
			break;
		case 4:
			next_pos = 0;		// �J�[�\���ʒu��ύX�O�ŗ��Ɉړ�����
			break;
		default:
			break;
		}
	}

	return next_pos;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ύX����f�[�^�̃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: ���͒l
///	@param[in]		pos		: �N(2) or ��(3) or ��(4)
///	@return			ret		: 0 = OK -1 = NG
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static int Date_Check( short data, int pos )
{
	switch( pos ){
		case 2:		//year
			if( data > 79 ){
				return( -1 );
			}
			break;
		case 3:		//month
			if( data > 12 ){		// 0�͋��e����
				return( -1 );
			}
			break;
		case 4:		//date
			if( data > 31 ){		// 0�͋��e����
				return( -1 );
			}
			break;
		default:
			return( -1 );
	}
	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief			����ł̐ŗ��\��
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : �\������l
///	@param[in]		pos  : �J�[�\���ʒu
///	@param[in]		rev  : 0:���] 1:���]
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	mnytax_dsp(ushort val, int pos, ushort rev)
{
	switch(pos){
	case 0:
		before_tax_dsp(val, rev);					// �ύX�O�ŗ��\��
		break;
	case 1:
		after_tax_dsp(val, rev);					// �ύX��ŗ��\��
		break;
	case 2:
		ChangeBaseDate_edit(0, val, rev);			// �ύX���(�N)�\��
		break;
	case 3:
		ChangeBaseDate_edit(1, val, rev);			// �ύX���(��)�\��
		break;
	case 4:
		ChangeBaseDate_edit(2, val, rev);			// �ύX���(��)�\��
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			����ł̐ŗ��\��(�����\��)
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : �\������l�z��̃|�C���^
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	mnytax_dspinit_dsp(ushort* val)
{
	// �Œ蕶����ݒ�
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );		// "������ł̐ŗ����@�@�@�@�@�@�@"
// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188] );	// "�@�ύX�O�ŗ��F�@�@�@�@�@�@�@�@"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "�@�ύX��ŗ��F�@�@�@�@�@�@�@�@"
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "�ύX����@�@�@�N�@�@���@�@��"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "�@�ύX�O�ŗ��F�@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "�@�ύX��ŗ��F�@�@�@�@�@�@�@�@"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[191] );	// "�ύX����@�@�@�N�@�@���@�@��"
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100

	if(val[0] == 0) {
		grachr( 2, 14, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	// "����łȂ�" 
	} else {
		before_tax_dsp(val[0], 0);						// �ύX�O�ŗ��\�� 
	}
	if(val[1] == 0) {
		grachr( 3, 14, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[1] );	// "����łȂ�"
	} else {
		after_tax_dsp(val[1], 0);						// �ύX��ŗ��\��
	}
	if((val[2] == 0) && (val[3] == 0) && (val[4] == 0)) {
		// �N�����S��0�̏ꍇ��"�|�|"�\��
		grachr( 5, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "�|�|"
		grachr( 5, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "�|�|"
		grachr( 5, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3] );	// "�|�|"
		return;
	}
	ChangeBaseDate_dsp(val[2], val[3], val[4]);	// �ύX���(�N����)�\��
}

//[]----------------------------------------------------------------------[]
///	@brief			����ł̐ŗ��\��(�ύX�{�^�������㏉���\��)
//[]----------------------------------------------------------------------[]
///	@param[in]		val  : �\������l�z��̃|�C���^
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Co8pyright(C) 2022 AMANO Corp.---[]
static void	mnytax_editinit_dsp(ushort* val)
{
	// �Œ蕶����ݒ�
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[4] );		// "����ł̐ŗ��@�@�@�@�@�@�@�@�@"
// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188] );	// "�@�ύX�O�ŗ��F�@�@�@�@�@�@�@�@"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "�@�ύX��ŗ��F�@�@�@�@�@�@�@�@"
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "�ύX����@�@�@�N�@�@���@�@��"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[189] );	// "�@�ύX�O�ŗ��F�@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[190] );	// "�@�ύX��ŗ��F�@�@�@�@�@�@�@�@"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[191] );	// "�ύX����@�@�@�N�@�@���@�@��"
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100

	before_tax_dsp(val[0], 0);					// �ύX�O�ŗ��\�� 
	after_tax_dsp(val[1], 0);					// �ύX��ŗ��\��
	ChangeBaseDate_dsp(val[2], val[3], val[4]);	// �ύX���(�N����)�\��
}

//[]----------------------------------------------------------------------[]
///	@brief			�ύX�O�ŗ��\��
//[]----------------------------------------------------------------------[]
///	@param[in]		tax  : �ŗ�
///	@param[in]		rev  : 0:���] 1:���]
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	before_tax_dsp(ushort tax, ushort rev)
{
	opedsp( 2, 14, tax, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );		// �ŗ�
	grachr( 2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );		// "��"
	grachr( 2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"    " );	// "����łȂ�"��"�Ȃ�"���\������Ă���ꍇ�ɂ͂����ŏ���
}

//[]----------------------------------------------------------------------[]
///	@brief			�ύX��ŗ��\��
//[]----------------------------------------------------------------------[]
///	@param[in]		tax  : �ŗ�
///	@param[in]		rev  : 0:���] 1:���]
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	after_tax_dsp(ushort tax, ushort rev)
{
	opedsp( 3, 14, tax, 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF );		// �ŗ�
	grachr( 3, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[2] );		// "��"
	grachr( 3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"    " );	// "����łȂ�"��"�Ȃ�"���\������Ă���ꍇ�ɂ͂����ŏ���
}

//[]----------------------------------------------------------------------[]
///	@brief			�ύX����\��(�����\��)
//[]----------------------------------------------------------------------[]
///	@param[in]		y  : �N
///	@param[in]		m  : ��
///	@param[in]		d  : ��
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	ChangeBaseDate_dsp(ushort y, ushort m, ushort d)
{
	opedsp( 5, 12, y, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �N
	opedsp( 5, 18, m, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ��
	opedsp( 5, 24, d, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ��
}

//[]----------------------------------------------------------------------[]
///	@brief			�ύX����\��(�ҏW)
//[]----------------------------------------------------------------------[]
///	@param[in]		kind : �ύX�Ώ�(0:�N�A1:���A2:��)
///	@param[in]		val  : �ݒ�l
///	@param[in]		rev  : 0:���] 1:���]
///	@return			None
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	ChangeBaseDate_edit(uchar kind, ushort val, ushort rev)
{
	switch( kind ) {
	case 0:
		opedsp( 5, 12, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// �N
		break;
	case 1:
		opedsp( 5, 18, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// ��
		break;
	case 2:
		opedsp( 5, 24, val, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF );	// ��
		break;
	default:
		break;
	}
}
// MH810105(E) MH364301 �C���{�C�X�Ή�

/*[]----------------------------------------------------------------------[]*/
/*|  �����ݒ�F�`�`�k�����                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnykid( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura ���ԓ��͂��R���܂łɕύX             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* �p�����[�^���A-L�Ԏ�̖���/��ʎ��Ԋ�����INDEX���擾����}�N�� */
#define	GET_KID_IDX(k)			(1+6*(k))
unsigned short	UsMnt_mnykid(void)
{
	ushort	msg;
	char	set;		/* 0:��ʐݒ蒆 1:�����ݒ蒆 2:�������Ԑݒ蒆 */
	int		kind;		/* ���(0-11(=A-L)) */
	char	top;
	long	data;
	char	i;
	long	*pSHA;

	pSHA = CPrmSS[S_SHA];

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[5] );			/* "���`�`�k��������@�@�@�@�@�@�@" */
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[19]);			/* "�y��ʁ|���@���z �y�������ԁz " */
	for (i = 0; i < 5; i++) {
		grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[i]);			/* ��� */
		grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" */
		grachr((ushort)(2+i), 6, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);			/* "�[" */
		mnykid_role_dsp(0, i, pSHA[GET_KID_IDX(i)], 0);		/* ���� */
		mnykid_time_dsp(0, i, pSHA[GET_KID_IDX(i)], 0);		/* ���� */
	}
	grachr(2, 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* "�`" ���] */
	grachr(2, 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "��" ���] */
	Fun_Dsp( FUNMSG2[8] );														/* "�@���@�@���@�@���@ �@�@  �I�� " */

	set = 0;
	kind = top = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch ( KEY_TEN0to9(msg) ) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* ��ʐݒ蒆 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					kind--;
					if (kind < 0) {
						kind = 11;
						top = 7;
					}
					else if (kind < top) {
						top--;
					}
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[top+i]);						/* ��� */
						grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);							/* "��" */
						mnykid_role_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);	/* ���� */
						mnykid_time_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);	/* ���� */
					}
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
					break;
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					kind++;
					if (kind > 11) {
						kind = top = 0;
					}
					else if (kind > top+4) {
						top++;
					}
					for (i = 0; i < 5; i++) {
						grachr((ushort)(2+i), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[top+i]);			/* ��� */
						grachr((ushort)(2+i), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "��" */
						mnykid_role_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);						/* ���� */
						mnykid_time_dsp(top, (char)(top+i), pSHA[GET_KID_IDX(top+i)], 0);						/* ���� */
					}
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);										/* ���� ���] */
// MH810100(S) 2020/09/11 #4834 ���󖢑Ή��̎�ʊ��������ʃp�����[�^�ɂĐݒ�̗��p���\�ƂȂ��Ă���
//					Fun_Dsp( FUNMSG2[4] );												 /* "�@���@�@���@ �ύX�@����  �I�� " */
					Fun_Dsp( FUNMSG2[65] );												 /* "�@���@�@ �@ �ύX�@����  �I�� " */
// MH810100(E) 2020/09/11 #4834 ���󖢑Ή��̎�ʊ��������ʃp�����[�^�ɂĐݒ�̗��p���\�ƂȂ��Ă���
					data = pSHA[GET_KID_IDX(kind)];
					set = 1;
					break;
				default:
					break;
				}
			}
			/* �����ݒ蒆 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* ���� ���] */
					Fun_Dsp( FUNMSG2[8] );														 /* "�@���@�@���@�@���@ �@�@  �I�� " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
// GG121802(S) R.Endo 2023/09/08 #7129 �����e�i���X�̗����ݒ�ŁuA�`L������v�y�[�W�̖����ύX����F2�L�[���L���ɂȂ��Ă���
					BUZPI();
// GG121802(E) R.Endo 2023/09/08 #7129 �����e�i���X�̗����ݒ�ŁuA�`L������v�y�[�W�̖����ύX����F2�L�[���L���ɂȂ��Ă���
					pSHA[GET_KID_IDX(kind)] = data;
					OpelogNo = OPLOG_ALYAKUWARI;			// ���엚��o�^
					f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
// MH810100(S) 2020/09/11 #4834 ���󖢑Ή��̎�ʊ��������ʃp�����[�^�ɂĐݒ�̗��p���\�ƂȂ��Ă���
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* ���� ���] */
					Fun_Dsp( FUNMSG2[8] );														 /* "�@���@�@���@�@���@ �@�@  �I�� " */
					set = 0;
					break;
// MH810100(E) 2020/09/11 #4834 ���󖢑Ή��̎�ʊ��������ʃp�����[�^�ɂĐݒ�̗��p���\�ƂȂ��Ă���
				case KEY_TEN_F2:	/* ��(F2) */
// GG121802(S) R.Endo 2023/09/08 #7129 �����e�i���X�̗����ݒ�ŁuA�`L������v�y�[�W�̖����ύX����F2�L�[���L���ɂȂ��Ă���
// 					BUZPI();
// 					/* �Ԏ����=�g�p���� */
// 					if (pSHA[GET_KID_IDX(kind)]/10000) {
// 						/* �������Ԑݒ�� */
// 						mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);		/* ���� ���] */
// 						mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);		/* ���� ���] */
// 						Fun_Dsp( FUNMSG2[2] );						/* "�@���@�@���@�@�@�@ ����  �I�� " */
// 						data = -1;
// 						set = 2;
// 					}
// 					/* �Ԏ����=���g�p */
// 					else {
// 						/* ��ʐݒ�� */
// 						grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
// 						grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
// 						mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);										/* ���� ���] */
// 						mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);										/* ���� ���] */
// 						Fun_Dsp( FUNMSG2[8] );												/* "�@���@�@���@�@���@ �@�@  �I�� " */
// 						set = 0;
// 					}
// GG121802(E) R.Endo 2023/09/08 #7129 �����e�i���X�̗����ݒ�ŁuA�`L������v�y�[�W�̖����ύX����F2�L�[���L���ɂȂ��Ă���
					break;
				case KEY_TEN_F3:	/* �ύX(F3) */
					BUZPI();
					if (data/10000)
						data %= 10000;
					else
						data += 10000;
					mnykid_role_dsp(top, kind, data, 1);		/* ���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* ���� ���] */
					data = pSHA[GET_KID_IDX(kind)];
					break;
				default:
					break;
				}
			}
			/* �������Ԑݒ蒆 */
			else {	/*if (set == 2) */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					mnykid_role_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* ���� ���] */
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);	/* ���� ���] */
					Fun_Dsp( FUNMSG2[4] );						/* "�@���@�@���@ �ύX�@����  �I�� " */
					data = pSHA[GET_KID_IDX(kind)];
					set = 1;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						if (data%100 > 59) {
							BUZPIPI();
							mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* ���� ���] */
							data = -1;
							break;
						}
						pSHA[GET_KID_IDX(kind)] = 10000+data;
						OpelogNo = OPLOG_ALYAKUWARI;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					grachr((ushort)(2+kind-top), 2, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr((ushort)(2+kind-top), 4, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 0);												/* ���� ���] */
					Fun_Dsp( FUNMSG2[8] );														 /* "�@���@�@���@�@���@ �@�@  �I�� " */
					set = 0;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnykid_time_dsp(top, kind, 10000+data, 1);	/* ���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnykid_time_dsp(top, kind, pSHA[GET_KID_IDX(kind)], 1);	/* ���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ�����\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   top  : �擪���                                  |*/
/*|              : char   kind : ���                                      |*/
/*|              : long   data : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnykid_role_dsp(char top, char kind, long data, ushort rev)
{
	/* �Ԏ����=�g�p���� */
	if (data/10000) {
		grachr((ushort)(2+kind-top), 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[5]);		/* "�������" */
	}
	/* �Ԏ����=���g�p */
	else {
		grachr((ushort)(2+kind-top), 8, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[0]);		/* "���g�p�@" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �������ԕ\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   top  : �擪���                                  |*/
/*|              : char   kind : ���                                      |*/
/*|              : long   data : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : 05/07/21 ART:ogura ���ԕ\�����R���܂łɕύX             |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnykid_time_dsp(char top, char kind, long data, ushort rev)
{
	/* �Ԏ����=�g�p���� */
	if (data/10000) {
		data %= 10000;
		grachr((ushort)(2+kind-top), 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);					/* "�[" */
		opedsp((ushort)(2+kind-top), 18, (ushort)(data/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);									/* �� */
		grachr((ushort)(2+kind-top), 22, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
		opedsp((ushort)(2+kind-top), 24, (ushort)(data%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);									/* �� */
	}
	/* �Ԏ����=���g�p */
	else {
		grachr((ushort)(2+kind-top), 16, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);				/* "�@" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ђP�ʎ��Ԃ̐ݒ�Index���擾����                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_TIME_IDX                                            |*/
/*| PARAMETER    : n(�����̌n),  b(������),  m(��{/�ǉ�)                  |*/
/*| RETURN VALUE : index                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	GET_TIME_IDX(char n, char b, char m){
	
	short	index;
	short	wk;
	wk = (n%3 == 0) ? 3 : n%3;
	index = (RYO_TAIKEI_SETCNT*(wk-1)+6)+(4*((b)-1))+m;
	
	return index;
}
	
/*[]----------------------------------------------------------------------[]*/
/*| ��{/�ǉ������̐ݒ�Index���擾����                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_MONEY_IDX                                           |*/
/*| PARAMETER    :  n(�����̌n),  k(�������),b(������),  m(��{/�ǉ�)     |*/
/*| RETURN VALUE : index                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	GET_MONEY_IDX(char n, char k, char b, char m){
	
	short	index;
	short	wk;
	wk = (n%3 == 0) ? 3 : n%3;
	index = (RYO_TAIKEI_SETCNT*(wk-1)+64)+((RYO_SYUBET_SETCNT*k)+(2*((b)-1)))+m;
	
	return index;
}
/*[]----------------------------------------------------------------------[]*/
/*| �����ݒ�F�P�ʎ��ԁ^����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnycha( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_mnycha(void)
{
	ushort	msg;
	char	set;	/* 0:�����̌n�ݒ蒆 1:��ʐݒ蒆 2:�����ѐݒ蒆 3:��{-���� 4:��{-���� 5:�ǉ�-���� 6:�ǉ�-���� */
	char	no;		/* �����̌n(1-3) */
	int		kind;	/* ���(0-11) */
	char	band;	/* ������(1-6) */
	long	data;
	long	*pRAT;

	pRAT = CPrmSS[S_RAT];
	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[6]);				/* "���P�ʎ��ԁ^�������@�@�@�@�@�@" */
	grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[14]);			/* "�i����сj" */
	grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "��@�����̌n" ���] */
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* "�P" ���] */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�[" */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* ��� */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "��" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�[" */
	grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);			/* "��@������" */
	opedsp(2, 22, 1, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* "�P" */

	grachr(4, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);							/* "��{" */
	mnycha_time_dsp(0, pRAT[GET_TIME_IDX(1,1,0)], 0);		/* ���� */
	grachr(4, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "�[" */
	mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(1,0,1,0)], 0);	/* ���� */
	opedsp(4, 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "�O" */
	grachr(4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "�~" */

	grachr(5, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[1]);							/* "�ǉ�" */
	mnycha_time_dsp(1, pRAT[GET_TIME_IDX(1,1,1)], 0);		/* ���� */
	grachr(5, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);							/* "�[" */
	mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(1,0,1,1)], 0);	/* ���� */
	opedsp(5, 26, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "�O" */
	grachr(5, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "�~" */

	Fun_Dsp( FUNMSG[25] );					/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */

	set = 0;
	no = 1;
	kind = 0;
	band = 1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* �����̌n�ݒ蒆 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if(--no < 1){
							no = 9;			
						}
					}
					else {
						if(++no > 9){
							no = 1;
						}
					}
					/* �ؑւ��������̌n�̐ݒ�ʒu���w�� */
					pRAT = CPrmSS[GET_PARSECT_NO(no)];			// �ݒ�ʒu�X�V
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �̌n           ���] */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);				/* ��{-���� */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);		/* ��{-���� */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);				/* �ǉ�-���� */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);		/* �ǉ�-���� */
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);									/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* �̌n           ���] */
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);								/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);									/* "��" ���] */
					Fun_Dsp( FUNMSG[28] );																			/* "  ��    ��    ��   �Ǐo  �I�� " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* ��ʐݒ蒆 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 11;
					}
					else {
						if (++kind > 11)
							kind = 0;
					}
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);	/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);		/* "��" ���] */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);				/* ��{-���� */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);		/* ��{-���� */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);				/* �ǉ�-���� */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);		/* �ǉ�-���� */
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �̌n           ���] */
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);	/* ��� ���] */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);		/* "��" ���] */
					Fun_Dsp( FUNMSG[25] );						 /* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "��@������" ���] */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� �@�@�@�@�@���] */
					set = 2;
					break;
				default:
					break;
				}
			}
			/* �����ѐݒ蒆 */
			else if (set == 2) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--band < 1)
							band = 6;
					}
					else {
						if (++band > 6)
							band = 1;
					}
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);			/* "��@������" ���] */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �� �@�@�@�@�@���] */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);						/* ��{-���� */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);				/* ��{-���� */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);						/* �ǉ�-���� */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);				/* �ǉ�-���� */
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);		/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);			/* "��" ���] */
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "��@������" ���] */
					opedsp(2, 22, (ushort)band, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� �@�@�@�@�@���] */
					set = 1;
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "��@������" ���] */
					opedsp(2, 22, (ushort)band, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* ��           ���] */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);					/* ��{-����    ���] */
					Fun_Dsp( FUNMSG2[2] );													/* "�@���@�@���@�@�@�@ ����  �I�� " */
					data = -1;
					set = 3;
					break;
				default:
					break;
				}
			}
			/* ��{-���� */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[0]);		/* "��@������" ���] */
					opedsp(2, 22, (ushort)band, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* ��           ���] */
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);					/* ��{-����    ���] */
					Fun_Dsp( FUNMSG[28] );									 /* "  ��    ��    ��   �Ǐo  �I�� " */
					set = 2;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						if ( (data/100 >23) || (data%100 > 59)) {
							BUZPIPI();
							mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);	/* ��{-���� ���] */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX(no,band,0)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 0);			/* ��{-���� ���] */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* ��{-���� ���] */
					data = -1;
					set = 4;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp(0, data, 1);		/* ��{-���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* ��{-���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* ��{-���� */
			else if (set == 4) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* ��{-���� ���] */
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);	/* ��{-���� ���] */
					data = -1;
					set = 3;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX(no,kind,band,0)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 0);	/* ��{-���� ���] */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* �ǉ�-���� ���] */
					data = -1;
					set = 5;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp(0, data, 1);	/* ��{-���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* ��{-���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* �ǉ�-���� */
			else if (set == 5) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					mnycha_money_dsp(0, pRAT[GET_MONEY_IDX(no,kind,band,0)], 1);	/* ��{-���� ���] */
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);			/* �ǉ�-���� ���] */
					data = -1;
					set = 4;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						if ( (data/100 >23) || (data%100 > 59)) {
							BUZPIPI();
							mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);	/* �ǉ�-���� ���] */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX(no,band,1)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 0);			/* �ǉ�-���� ���] */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 1);	/* �ǉ�-���� ���] */
					data = -1;
					set = 6;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp(1, data, 1);		/* �ǉ�-���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* �ǉ�-���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* �ǉ�-���� */
			else {	/* if (set == 6) */
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					mnycha_time_dsp(1, pRAT[GET_TIME_IDX(no,band,1)], 1);			/* �ǉ�-���� ���] */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);	/* �ǉ�-���� ���] */
					data = -1;
					set = 5;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX(no,kind,band,1)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_time_dsp(0, pRAT[GET_TIME_IDX(no,band,0)], 1);			/* ��{-���� ���] */
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 0);	/* �ǉ�-���� ���] */
					data = -1;
					set = 3;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp(1, data, 1);	/* �ǉ�-���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_money_dsp(1, pRAT[GET_MONEY_IDX(no,kind,band,1)], 1);	/* �ǉ�-���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �P�ʎ��ԕ\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( top, kind, data, rev)                  |*/
/*| PARAMETER    : char   mode : 0:��{ 1:�ǉ�                             |*/
/*|              : long   time : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_time_dsp(char mode, long time, ushort rev)
{
	ushort	line;

	line = (mode == 0) ? 4 : 5;
	opedsp(line, 6, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
	grachr(line, 10, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
	opedsp(line, 12, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
}

/*[]----------------------------------------------------------------------[]*/
/*|  �P�ʗ����\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnykid_role_dsp( mode, money, rev )                     |*/
/*| PARAMETER    : char   mode  : 0:��{ 1:�ǉ�                            |*/
/*|              : long   money : ����                                     |*/
/*|              : ushort rev   : 0:���] 1:���]                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_money_dsp(char mode, long money, ushort rev)
{
	ushort	line;

	line = (mode == 0) ? 4 : 5;
	opedsp(line, 18, (ushort)(money/10), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* ���� */
	if (money/10 == 0)
		grachr(line, 24, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ݒ�F�P�ʎ��ԁ^����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnycha( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* �p�����[�^��̒P�ʎ��Ԃ�INDEX���擾����}�N�� */
#define	GET_TIME_IDX_DEC(n, p)			(300*((n)-1)+11+2*((p)-1))
/* �p�����[�^��̒P�ʗ�����INDEX���擾����}�N�� */
#define	GET_MONEY_IDX_DEC(n, k, p)		(300*((n)-1)+63+20*(k)+(p))
/* �p�����[�^��̒P�ʌW����INDEX���擾����}�N�� */
#define	GET_COEFFICIENT_IDX_DEC(n, p)	(300*((n)-1)+11+2*((p)-1)+1)
unsigned short	UsMnt_mnycha_dec(void)
{
	ushort	msg;
	char	set;	/* 0:�����̌n�ݒ蒆 1:��ʐݒ蒆 2:�����p�^�[���ݒ蒆 3:�P�ʎ��� 4:�P�ʗ��� 5:�P�ʌW�� */
	char	no;		/* �����̌n(1-3) */
	int		kind;	/* ���(0-11) */
	int		pattern;/* �p�^�[��(0-12) �O�F�[���*/
	long	data;
	long	*pRAT;

	pRAT = CPrmSS[S_RAT];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[6]);				/* "���P�ʎ��ԁ^�������@�@�@�@�@�@" */
	grachr(0, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[15]);			/* "�i�����сj" */
	grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "��@�����̌n" ���] */
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);					/* "�P" ���] */
	grachr(2, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�|" */
	grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[0]);				/* ��� */
	grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "��" */
	grachr(2, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[0]);				/* "�|" */
	grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[1]);			/* "��P����� " */

	grachr(4, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[27]);				/* "�P�ʎ���" */
	mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(1,1)], 0);						/* ���� */

	grachr(5, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[28]);				/* "�P�ʗ���" */
	mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(1,0,1)], 0);					/* ���� */
	opedsp(5, 22, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "�O" */
	grachr(5, 24, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);				/* "�~" */

	grachr(6, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[29]);				/* "�P�ʌW��" */
	mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(1,1)], 0);			/* �W�� */

	Fun_Dsp( FUNMSG[25] );														/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */

	set = 0;
	no = 1;
	kind = 0;
	pattern = 1;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		default:
			/* �����̌n�ݒ蒆 */
			if (set == 0) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--no < 1)
							no = 3;
					}
					else {
						if (++no > 3)
							no = 1;
					}
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);					/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �̌n           ���] */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* ���� */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);	/* �W�� */
					}
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);				/* ���� */
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);				/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			/* �̌n           ���] */
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);			/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);				/* "��" ���] */
					Fun_Dsp( FUNMSG[28] );														/* "  ��    ��    ��   �Ǐo  �I�� " */
					set = 1;
					break;
				default:
					break;
				}
			}
			/* ��ʐݒ蒆 */
			else if (set == 1) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--kind < 0)
							kind = 11;
					}
					else {
						if (++kind > 11)
							kind = 0;
					}
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* ���� */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);	/* �W�� */
					}
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);				/* ���� */
					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr(2, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);					/* "��@�����̌n" ���] */
					opedsp(2, 2, (ushort)no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �̌n           ���] */
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					Fun_Dsp( FUNMSG[25] );															/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
					set = 0;
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);				/* ��� ���] */
					grachr(2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);					/* "��" ���] */
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);			/* "��w����� " ���] */
					set = 2;
					break;
				default:
					break;
				}
			}
			/* �����p�^�[���ݒ蒆 */
			else if (set == 2) {
				switch (msg) {
				case KEY_TEN_F1:	/* ��(F1) */
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					if (msg == KEY_TEN_F1) {
						if (--pattern < 0)
							pattern = 12;
					}
					else {
						if (++pattern > 12)
							pattern = 0;
					}
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "��w����� " ���] */
					if (pattern != 0) {
						grachr(4, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[27]);							/* "�P�ʎ���" */
						grachr(5, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[28]);							/* "�P�ʗ���" */
						opedsp(5, 22, 0, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);								/* "�O" */
						grachr(5, 24, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[3]);							/* "�~" */
						grachr(6, 2, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[29]);							/* "�P�ʌW��" */
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);								/* ���� */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);						/* ���� */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);				/* �W�� */
					} else {
						grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);						/* "�@" */
						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);						/* "�@" */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);						/* ���� */
					}

					break;
				case KEY_TEN_F3:	/* ��(F3) */
					BUZPI();
					grachr(2, 14, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_4[kind]);							/* ��� ���] */
					grachr(2, 16, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[1]);								/* "��" ���] */
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "��@�@����� " ���] */
					set = 1;
					break;
				case KEY_TEN_F4:	/* �Ǐo(F4) */
					BUZPI();
					grachr(2, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);						/* "��@�@����� " ���] */
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* ����    ���] */
						Fun_Dsp( FUNMSG[20] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */
						set = 3;
					} else {
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* ���� ���] */
						Fun_Dsp( FUNMSG[22] );					/* "�@���@�@�@�@�@�@�@ ����  �I�� " */
						set = 4;
					}
					data = -1;
					break;
				default:
					break;
				}
			}
			/* ���� */
			else if (set == 3) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);		/* "��@�@����� " ���] */
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);					/* ����         ���] */
					Fun_Dsp( FUNMSG[28] );														/* "  ��    ��    ��   �Ǐo  �I�� " */
					set = 2;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						if (data%100 > 59) {
							// �����T�X�ȏゾ�ƌ��̕\���ɖ߂��B
							BUZPIPI();
							mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);	/* ���� ���] */
							data = -1;
							break;
						}
						pRAT[GET_TIME_IDX_DEC(no,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 0);			/* ���� ���] */
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* ���� ���] */
					data = -1;
					set = 4;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + msg-KEY_TEN0) % 10000;
					mnycha_time_dsp_dec(data, 1);		/* ���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* ���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* ���� */
			else if (set == 4) {
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					if (pattern != 0) {
						mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* ���� ���] */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);	/* ���� ���] */
						set = 3;
					} else {
						grachr(2, 20, 10, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT5_4[pattern]);	/* "��@�@����� " ���] */
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);		/* ���� ���] */
						Fun_Dsp( FUNMSG[28] );													/* "  ��    ��    ��   �Ǐo  �I�� " */
						set = 2;
					}
					data = -1;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					if( pattern != 0 || KEY_TEN0to9(msg) != KEY_TEN_F2)
						BUZPI();
					if (pattern != 0) {
						mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 0);	/* ���� ���] */
						mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 1);			/* �W�� ���] */
						set = 5;
					}
					data = -1;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)*10) % 100000L;
					mnycha_money_dsp_dec(data, 1);	/* ���� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* ���� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			/* �W�� */
			else {	/*if (set == 5) {*/
				switch (KEY_TEN0to9(msg)) {
				case KEY_TEN_F1:	/* ��(F1) */
					BUZPI();
					mnycha_money_dsp_dec(pRAT[GET_MONEY_IDX_DEC(no,kind,pattern)], 1);	/* ���� ���] */
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);			/* �W�� ���] */
					data = -1;
					set = 4;
					break;
				case KEY_TEN_F4:	/* ����(F4) */
					if (data != -1) {
						pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)] = data;
						OpelogNo = OPLOG_TANIJIKANRYO;			// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
				case KEY_TEN_F2:	/* ��(F2) */
					BUZPI();
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 0);			/* �W�� ���] */
					mnycha_time_dsp_dec(pRAT[GET_TIME_IDX_DEC(no,pattern)], 1);			/* ����    ���] */
					data = -1;
					set = 3;
					break;
				case KEY_TEN:		/* ����(�e���L�[) */
					BUZPI();
					if (data == -1)
						data = 0;
					data = (data*10 + (msg-KEY_TEN0)) % 10000;
					mnycha_coefficient_dsp_dec(data, 1);	/* �W�� ���] */
					break;
				case KEY_TEN_CL:	/* ���(�e���L�[) */
					BUZPI();
					mnycha_coefficient_dsp_dec(pRAT[GET_COEFFICIENT_IDX_DEC(no,pattern)], 1);			/* �W�� ���] */
					data = -1;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �P�ʎ��ԕ\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_time_dsp_dec(time, rev)                          |*/
/*| PARAMETER    : long   time : ����                                      |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_time_dsp_dec(long time, ushort rev)
{
	opedsp(4, 16, (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(4, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
	opedsp(4, 22, (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
}

/*[]----------------------------------------------------------------------[]*/
/*|  �P�ʗ����\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_money_dsp_dec(money, rev)                        |*/
/*| PARAMETER    : long   money : ����                                     |*/
/*|              : ushort rev   : 0:���] 1:���]                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_money_dsp_dec(long money, ushort rev)
{
	opedsp(5, 14, (ushort)(money/10), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* ���� */
	if (money/10 == 0)
		grachr(5, 20, 2, rev, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
}

/*[]----------------------------------------------------------------------[]*/
/*|  �P�ʌW���\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnycha_coefficient_dsp_dec(coefficient, rev)            |*/
/*| PARAMETER    : long   coefficient : �W��                                     |*/
/*|              : ushort rev   : 0:���] 1:���]                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnycha_coefficient_dsp_dec(long coefficient, ushort rev)
{
	opedsp(6, 18, (ushort)(coefficient), 4, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �W�� */
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ݒ�F��������j��                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mnytweek( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
/* �e�j�����L���Ȃ�O��Ԃ��}�N�� */
#define	IS_SUN(d)	((((d)%1000000L) < 100000L) ? 0 : 1)
#define	IS_MON(d)	((((d)%100000L) < 10000L) ? 0 : 1)
#define	IS_TUE(d)	((((d)%10000) < 1000) ? 0 : 1)
#define	IS_WED(d)	((((d)%1000) < 100) ? 0 : 1)
#define	IS_THU(d)	((((d)%100) < 10) ? 0 : 1)
#define	IS_FRI(d)	((((d)%10) < 1) ? 0 : 1)
#define	IS_SAT(d)	((((d)%1000000L) < 100000L) ? 0 : 1)
#define	IS_SP0(d)	((((d)%100000L) < 10000L) ? 0 : 1)
#define	IS_SP1(d)	((((d)%10000) < 1000) ? 0 : 1)
#define	IS_SP2(d)	((((d)%1000) < 100) ? 0 : 1)
#define	IS_SP3(d)	((((d)%100) < 10) ? 0 : 1)
unsigned short	UsMnt_mnytweek(void)
{
	ushort	msg;
	char	changing;
	char	pos;		/* 0:�j���ݒ� 1:���Ԑݒ� */
	char	no;			/* ����ԍ�(1-15) */
	long	*pdata;
	long	tbl[] = {100000,10000,1000,100,10,1};

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "����������j�����@�@�@�@�@�@�@" */
	grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "���" ���] */
	opedsp(2, 8, 1, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* "�O�P" ���] */
	grachr(2, 12, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[2]);		/* "�̖����j��" */
	mnytweek_dsp(1);														/* �����j�� */
	Fun_Dsp(FUNMSG2[9]);													/* "�@���@�@���@ �ύX�@�@�@�@�I�� " */

	changing = 0;
	no = 1;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {	
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (changing == 0) {
			/* ����ԍ�-�I�� */
			switch (msg) {
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				if (msg == KEY_TEN_F1) {
					if (--no < 1)
						no = 15;
				}
				else {
					if (++no > 15)
						no = 1;
				}
				opedsp(2, 8, (ushort)no, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� ���] */
				mnytweek_dsp(no);						/* �����j�� */
				break;
			case KEY_TEN_F3:	/* �ύX(F3) */
				BUZPI();
				pdata = &CPrmSS[S_PAS][6+(no-1)*10];
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "����������j�����@�@�@�@�@�@�@" */
				grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "���" */
				opedsp(2, 4, (ushort)no, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� */
				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[22]);		/* "�@�@�@�@�P�Q�R�S�T�U�V�@�@�W�@" */
				grachr(4, 2, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[5]);			/* "�j��" ���] */
				grachr(4,  8, 2, IS_MON(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "��" */
				grachr(4, 10, 2, IS_TUE(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "��" */
				grachr(4, 12, 2, IS_WED(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "��" */
				grachr(4, 14, 2, IS_THU(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "��" */
				grachr(4, 16, 2, IS_FRI(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "��" */
				grachr(4, 18, 2, IS_SAT(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "�y" */
				grachr(4, 20, 2, IS_SUN(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "��" */
				grachr(4, 24, 6, IS_SP0(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "���ʓ�" */
				grachr(5, 2, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);					/* "����" ���] */
				grachr(5,  8, 4, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
				opedsp(5, 12, 1, 1, 0, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�P" */
				grachr(5, 16, 4, IS_SP2(pdata[1]),   COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);/* "����" */
				opedsp(5, 20, 2, 1, 0, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�Q" */
				grachr(5, 24, 4, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
				opedsp(5, 28, 3, 1, 0, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�R" */
				Fun_Dsp(FUNMSG[6]);			/* "�@���@�@���@ �@�@�@�@�@�@�I�� " */
				changing = 1;
				pos = 0;
				OpelogNo = OPLOG_TEIKIMUKOYOUBI;		// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				break;
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
		else {
			/* �����j��-�ݒ蒆 */
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
				BUZPI();
				pos ^= 1;
				grachr(4, 2, 4, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[5]);			/* "�j��" */
				grachr(5, 2, 4, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);			/* "����" */
				break;
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				dispclr();
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[20]);		/* "����������j�����@�@�@�@�@�@�@" */
				grachr(2, 4, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[4]);			/* "���" ���] */
				opedsp(2, 8, (ushort)no, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �ԍ� ���] */
				grachr(2, 12, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[2]);		/* "�̖����j��" */
				mnytweek_dsp(no);														/* �����j�� */
				Fun_Dsp(FUNMSG2[9]);													/* "�@���@�@���@ �ύX�@�@�@�@�I�� " */
				changing = 0;
				break;
			case KEY_TEN:		/* ����(�e���L�[) */
				if (pos == 0) {
					switch (msg) {
					case KEY_TEN1:
						BUZPI();
						if (IS_MON(pdata[0]))
							pdata[0] -= tbl[1];
						else
							pdata[0] += tbl[1];
						grachr(4,  8, 2, IS_MON(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "��" */
						break;
					case KEY_TEN2:
						BUZPI();
						if (IS_TUE(pdata[0]))
							pdata[0] -= tbl[2];
						else
							pdata[0] += tbl[2];
						grachr(4, 10, 2, IS_TUE(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "��" */
						break;
					case KEY_TEN3:
						BUZPI();
						if (IS_WED(pdata[0]))
							pdata[0] -= tbl[3];
						else
							pdata[0] += tbl[3];
						grachr(4, 12, 2, IS_WED(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "��" */
						break;
					case KEY_TEN4:
						BUZPI();
						if (IS_THU(pdata[0]))
							pdata[0] -= tbl[4];
						else
							pdata[0] += tbl[4];
						grachr(4, 14, 2, IS_THU(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "��" */
						break;
					case KEY_TEN5:
						BUZPI();
						if (IS_FRI(pdata[0]))
							pdata[0] -= tbl[5];
						else
							pdata[0] += tbl[5];
						grachr(4, 16, 2, IS_FRI(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "��" */
						break;
					case KEY_TEN6:
						BUZPI();
						if (IS_SAT(pdata[1]))
							pdata[1] -= tbl[0];
						else
							pdata[1] += tbl[0];
						grachr(4, 18, 2, IS_SAT(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "�y" */
						break;
					case KEY_TEN7:
						BUZPI();
						if (IS_SUN(pdata[0]))
							pdata[0] -= tbl[0];
						else
							pdata[0] += tbl[0];
						grachr(4, 20, 2, IS_SUN(pdata[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "��" */
						break;
						break;
					case KEY_TEN8:
						BUZPI();
						if (IS_SP0(pdata[1]))
							pdata[1] -= tbl[1];
						else
							pdata[1] += tbl[1];
						grachr(4, 24, 6, IS_SP0(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "���ʓ�" */
						break;
					default:
						break;
					}
				}
				else {
					switch (msg) {
					case KEY_TEN1:
						BUZPI();
						if (IS_SP1(pdata[1]))
							pdata[1] -= tbl[2];
						else
							pdata[1] += tbl[2];
						grachr(5,  8, 4, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
						opedsp(5, 12, 1, 1, 0, IS_SP1(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�P" */
						break;
					case KEY_TEN2:
						BUZPI();
						if (IS_SP2(pdata[1]))
							pdata[1] -= tbl[3];
						else
							pdata[1] += tbl[3];
						grachr(5, 16, 4, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
						opedsp(5, 20, 2, 1, 0, IS_SP2(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�Q" */
						break;
					case KEY_TEN3:
						BUZPI();
						if (IS_SP3(pdata[1]))
							pdata[1] -= tbl[4];
						else
							pdata[1] += tbl[4];
						grachr(5, 24, 4, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
						opedsp(5, 28, 3, 1, 0, IS_SP3(pdata[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�R" */
						break;
					default:
						break;
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
/*|  �����j���\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mnytweek_dsp( no )                                      |*/
/*| PARAMETER    : char no : ����ԍ�                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	mnytweek_dsp(char no)
{
	long	*p;

	p = &CPrmSS[S_PAS][6+(no-1)*10];
	if (p[0] == 0 && p[1] == 0) {
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[21]);		/* "�@�@�Ȃ��i�S�j���L���j�@�@�@�@" */
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
	}
	else {
		grachr(4,  4, 2, IS_MON(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[1]);	/* "��" */
		grachr(4,  6, 2, IS_TUE(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[2]);	/* "��" */
		grachr(4,  8, 2, IS_WED(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[3]);	/* "��" */
		grachr(4, 10, 2, IS_THU(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[4]);	/* "��" */
		grachr(4, 12, 2, IS_FRI(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[5]);	/* "��" */
		grachr(4, 14, 2, IS_SAT(p[1]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[6]);	/* "�y" */
		grachr(4, 16, 2, IS_SUN(p[0]), COLOR_BLACK, LCD_BLINK_OFF, WEKFFNT[0]);	/* "��" */
		grachr(4, 18, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);			/* "�@" */
		grachr(4, 20, 6, IS_SP0(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[2]);	/* "���ʓ�" */
		grachr(5,  4, 4, IS_SP1(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
		opedsp(5,  8, 1, 1, 0, IS_SP1(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�P" */
		grachr(5, 12, 4, IS_SP2(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
		opedsp(5, 16, 2, 1, 0, IS_SP2(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�Q" */
		grachr(5, 20, 4, IS_SP3(p[1]), COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[6]);	/* "����" */
		opedsp(5, 24, 3, 1, 0, IS_SP3(p[1]), COLOR_BLACK, LCD_BLINK_OFF);		/* "�R" */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�g���@�\                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Extendmenu( void )                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_Extendmenu(void)
{
	unsigned short	usUextEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[51]);		/* "���g���@�\���@�@�@�@�@�@�@�@�@" */

		usUextEvent = Menu_Slt((void*)USM_ExtendMENU , (void*)USM_ExtendMENU_TBL , (char)Ext_Menu_Max, 1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usUextEvent) {

		/* �����䐔�W�v */
		case MNT_FTOTL:
			usUextEvent = UsMnt_FTotal();
			break;

		/* �N���W�b�g���� */
		case MNT_CREDIT:
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			usUextEvent = UsMnt_CreditMnu();
			BUZPIPI();
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		case MNT_EDY:
//			usUextEvent = UsMnt_EdyMnu();
//			break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		/* �r������������ */
		case MNT_SCA:
			usUextEvent = UsMnt_SuicaMnu();
			break;
		case MNT_HOJIN:					/* �@�l�J�[�h */
			BUZPIPI();
			break;
		case MNT_DLOCK:					// �d�����b�N����
			usUextEvent = Cardress_DoorOpen();
			break;
		/* �l�s�W�v */
		case MNT_MTOTL:
			usUextEvent = UsMnt_Total(MNT_MTOTL);
			break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�����ǉ�)
		/* ���σ��[�_���� */
		case MNT_ECR:
			usUextEvent =  UsMnt_ECReaderMnu();
			break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�����ǉ�)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		/* �d�q�W���[�i�� */
		case MNT_EJ:
			usUextEvent =  UsMnt_EJournalMnu();
			break;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return MOD_EXT;
		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
			SetChange = 1;			// FLASH���ގw��
		}
		if (usUextEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (usUextEvent == MOD_CUT) {	
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�g���@�\ - �����䐔�W�v                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ftotal( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_FTotal(void)
{
	ushort	msg;
	uchar	dsp;
	int		pos;
	ushort	ret;

	if( (prm_get(COM_PRM, S_TOT,  2, 1, 1) == 0) ||		// �����䐔�W�v�Ȃ�
		(prm_get(COM_PRM, S_NTN, 26, 1, 2) == 0) ||
		_is_ntnet_remote()	||						// �Z���^�[�ڑ��H
		(OPECTL.Mnt_lev < 2) )
	{
		BUZPIPI();
		return MOD_EXT;	
	}

	pos = 0;
	dsp = 1;
	for ( ; ; ) {
		if (dsp) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[52]);		/* "�������䐔�W�v�i�g���j���@�@�@" */
			ftotal_dsp(0, (pos==0)?1:0);											/* "���v�v�����g" */
			ftotal_dsp(1, (pos==1)?1:0);											/* "���v�v�����g" */
			ftotal_dsp(2, (pos==2)?1:0);											/* "�O�񍇌v�v�����g" */
			Fun_Dsp(FUNMSG[68]);													/* "�@���@�@���@�@�@�@ ���s  �I�� " */
			dsp = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:	/* Ӱ����ݼ� */
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I�� */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* �� */
		case KEY_TEN_F2:	/* �� */
			BUZPI();
			ftotal_dsp(pos, 0);			/* �O�񍀖�-���]�\�� */
			if (msg == KEY_TEN_F1) {
				if (--pos < 0)
					pos = 2;
			}
			else {
				if (++pos > 2)
					pos = 0;
			}
			ftotal_dsp(pos, 1);			/* ���񍀖�-���]�\�� */
			break;
		case KEY_TEN_F4:	/* ���s */
			BUZPI();
			ret = ftotal_print(pos);	/* �v�����g���s */
			if (ret == MOD_CHG)
				return MOD_CHG;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (ret == LCD_DISCONNECT)
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			dsp = 1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  �����䐔���v�\��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ftotal_dsp(pos, rev)                                    |*/
/*| PARAMETER    : char   pos : �\������(0:���v 1:���v 2:�O�񍇌v)         |*/
/*|              : ushort rev : 0:���] 1:���]                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	ftotal_dsp(char pos, ushort rev)
{
	const uchar	*str;
	ushort	len;

	switch (pos) {
	case 0:
		str = &TGTSTR[2][2];	/* "���v�v�����g" */
		len = 12;
		break;
	case 1:
		str = &TGTSTR[3][2];	/* "���v�v�����g" */
		len = 12;
		break;
	default:	/* case 2: */
		str = &TGTSTR[4][2];	/* "�O�񍇌v�v�����g" */
		len = 16;
		break;
	}
	grachr((ushort)(2+pos), 2, len, rev, COLOR_BLACK, LCD_BLINK_OFF, str);
}

/*[]----------------------------------------------------------------------[]*/
/*|  �����䐔���v �v�����g���s                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ftotal_print(kind)                                      |*/
/*| PARAMETER    : char  kind : �I�����(0:���v 1:���v 2:�O�񍇌v)         |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static ushort	ftotal_print(char kind)
{
	ushort			msg;
	uchar			mode;
	T_FrmSyuukei	FrmSyuukei;
	T_FrmPrnStop	FrmPrnStop;
	ushort			type;
	ushort			log;
	char			inji_end;
	ushort			rev;
	ushort			result;
	ushort			tmout;
	ulong			req;
	char			value;
	uchar			mnt_sw = 0;		// �󎚒��Ƀ����e�i���XOFF�ɂȂ������ǂ���

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[52]);		/* "�������䐔�W�v�i�g���j���@�@�@" */
	if (kind == 0) {
		grachr(1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[8]);		/* "�����䐔���v */
		req = NTNET_DATAREQ2_MSYOUKEI;
		value = 0;
	}
	else if (kind == 1) {
		grachr(1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[9]);		/* "�����䐔���v */
		req = NTNET_DATAREQ2_MGOUKEI;
		value = 1;
	}
	else {
		grachr(1, 2, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT8_1[3]);		/* "�O�񕡐��䐔���v */
		req = NTNET_DATAREQ2_MGOUKEI;
		value = 2;
	}
	rev = 0;
	grachr(2, 2, 20, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[53]);	/* "�@���΂炭���҂��������@�@�@�@" */
	Fun_Dsp(FUNMSG[82]);													/* "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@" */

	/* NTNET�f�[�^���M(�f�[�^�v��2) */
	NTNET_Snd_Data109(req, value);
	tmout = (ushort)(prm_get(COM_PRM, S_NTN, 30, 2, 1) * 1000 / 20);	/* 20msec�P�� */
	Lagtim(OPETCBNO, 6, tmout);			/* ��M�^�C���A�E�g���ԃZ�b�g */
	Lagtim(OPETCBNO, 7, 25);			/* �����_�ŗp�^�C�}�[�Z�b�g(500msec) */
	mode = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		/* ��M�����҂� */
		if (mode == 0) {
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	/* Ӱ����ݼ� */
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;
			case KEY_TEN_F3:	/* ���~ */
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_EXT;
			case TIMEOUT7:		/* �����_�� */
				rev ^= 1;
				grachr(2, 2, 20, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[53]);	/* "�@���΂炭���҂��������@�@�@�@" */
				Lagtim(OPETCBNO, 7, 25);												/* �����_�ŗp�^�C�}�[���X�^�[�g(500msec) */
				break;
			case IBK_NTNET_DAT_REC:		/* NTNET�f�[�^��M */
				switch (NTNET_isTotalEndReceived(&result)) {
				/* �f�[�^�v��2����NG ��M */
				case -1:
					Lagcan(OPETCBNO, 6);
					Lagcan(OPETCBNO, 7);
					BUZPIPI();
					if( 3 == result ){														// ���s����3�i�������v/���v�ް��Ȃ��j
						grachr(2, 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR3[65]);	// "�@�W�v�f�[�^������܂���@�@�@"
					}
					else{																	// ���̎��s�i�قƂ�ǂ̏ꍇ �װ����3�ƂȂ�j
						grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[66]);	// "�@���s������M�i�R�[�h�w�w�j�@"
						opedsp(2, 22, result, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// ����(�ԍ�)�\��
					}
					Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
					mode = 2;
					break;
				/* �W�v�I���ʒm ��M */
				case 1:
					Lagcan(OPETCBNO, 6);
					Lagcan(OPETCBNO, 7);
					/* �v�����g���s */
					if (kind == 0) {
						type = PREQ_F_TSYOUKEI;
						log = OPLOG_F_SHOKEI;
					}
					else if (kind == 1) {
						type = PREQ_F_TGOUKEI;
						log = OPLOG_F_GOKEI;
					}
					else {
						type = PREQ_F_TGOUKEI_Z;
						log = OPLOG_F_ZENGOKEI;
					}
					FrmSyuukei.prn_kind = R_PRI;
					sky.fsyuk.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// �@�B��
					sky.fsyuk.Kakari_no = OPECTL.Kakari_Num;							// �W��No.
					FrmSyuukei.prn_data = (void*)&sky.fsyuk;
					memcpy( &FrmSyuukei.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// �󎚎���	�F���ݎ���
// MH810105(S) MH364301 ������W�v�ł݂Ȃ����Ϗ����󎚂��Ă��܂�
					FrmSyuukei.print_flag = 0;
// MH810105(E) MH364301 ������W�v�ł݂Ȃ����Ϗ����󎚂��Ă��܂�
					queset(PRNTCBNO, type, sizeof(T_FrmSyuukei), &FrmSyuukei);
					wopelg(log, 0, 0);		/* ���엚��o�^ */

					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[54]);		/* "�@�v�����g���J�n���܂��@�@�@�@" */
					inji_end = 0;
					mode = 1;
					break;
				default:
					break;
				}
				break;
			case TIMEOUT6:		/* ��M�^�C���A�E�g */
				Lagcan(OPETCBNO, 7);
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[55]);		/* "�@ ����������܂��� �@�@�@�@�@" */
				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				mode = 2;
				break;
			default:
				break;
			}
		}
		/* �v�����g�� */
		else if (mode == 1) {
			if ((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
				inji_end = 1;						// �󎚏I��
				msg &= (~INNJI_ENDMASK);
			}
			switch (msg){
			case KEY_TEN_F3:		/* ���~ */
				BUZPI();
				if (inji_end == 0) {
					FrmPrnStop.prn_kind = R_PRI;			// �����Ώ��������ʾ��
					queset(PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop);	// �󎚒��~�v��
				}
				if (mnt_sw == 1) {
					return MOD_CHG;
				}
				return MOD_EXT;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				if (inji_end) {
					BUZPI();
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:		/* Ӱ����ݼ� */
				if (inji_end) {
					BUZPI();
					return MOD_CHG;
				}
				mnt_sw = 1;			// �����eOFF�ێ�
				break;
			case PREQ_F_TSYOUKEI:  	// �������v
			case PREQ_F_TGOUKEI: 	// �������v
			case PREQ_F_TGOUKEI_Z:	// �O�񕡐����v
				if (inji_end) {
					if (mnt_sw == 1) {
						return MOD_CHG;
					}
					switch (OPECTL.Pri_Result) {	// �󎚌��ʁH
					case PRI_NML_END:	/* ����I�� */
					case PRI_CSL_END:	/* ��ݾٗv���ɂ��I�� */
						return MOD_EXT;
					case PRI_ERR_END:	/* �ُ�I�� */
						grachr(2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[7]);		// "�@�v�����^�[�ُ�ł��B�@�@�@�@"
						Fun_Dsp(FUNMSG[8]);														// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
						mode = 2;
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
		/* �����Ȃ� or �v�����^�ُ� */
		else {	/* if (mode == 2) */
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	/* Ӱ����ݼ� */
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:	/* �I�� */
				BUZPI();
				return MOD_EXT;
			default:
				break;
			}
		}
	}

}


/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F��������Z���~�f�[�^                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PassStop( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PassStop(void)
{
	ushort	msg;
	char	fk_page;	// �t�@���N�V�����L�[�\���y�[�W
	char	repaint;	// ��ʍĕ`��t���O
	short	show_index;	// �\�����̃C���f�b�N�X
	short ret;
	struct TKI_CYUSI work_data;
	short data_count;
	f_NTNET_RCV_TEIKITHUSI = 0;
	
	dispclr();									// ��ʃN���A

	fk_page = 0;
	repaint = 1;
	show_index = 0;
	data_count = 0;

	
	// ��ʃ^�C�g����\��
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "����������~�f�[�^���@�@�@�@�@"

	Fun_Dsp(FUNMSG2[20]);													// "�@���@�@���@ ����� ����  �I�� "

	for ( ; ; ) {
	
		if (repaint) {
			pstop_show_data(show_index, &tki_cyusi, &work_data, fk_page);
			data_count = tki_cyusi.count;
			repaint = 0;
		}
		msg = StoF(GetMessage(), 1);
		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ���������ꂽ��
			if ( f_NTNET_RCV_TEIKITHUSI == 1 ) {
				f_NTNET_RCV_TEIKITHUSI = 0;
				BUZPIPI();
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( 1 == pstop_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
//														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
//					return MOD_CHG;
//				}
//				else
//					return MOD_EXT;
				if( 1 == pstop_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
					return MOD_CHG;
				}
				else if(3 == pstop_calcel_dsp()){
					return MOD_CUT;
				} else {
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
		}

		switch (msg) {
		case KEY_TEN_F1:			/* F1:�� */
			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��

				BUZPI();
				if (fk_page == 0) {			// �t�@���N�V�����L�[�P�y�[�W��
					show_index--;
					if (show_index < 0) {
						show_index = tki_cyusi.count -1 ;
					}
				} else {					// �t�@���N�V�����L�[�Q�y�[�W��
					// �폜
					ret = pstop_del_dsp(show_index, 0, &work_data);
					if (ret == 1) {
						return( MOD_CHG );
					}else if(ret == 2){
						return( MOD_EXT);
					} else if (ret == -1) {
						
						if (show_index >= tki_cyusi.count) {
							show_index--;
						}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}else if(ret == 3){
						return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}
					if (fk_page) {
						Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
					} else {
						Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
					}
				}
				repaint = 1;
			}
			break;
		case KEY_TEN_F2:			/* F2:�� */
			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
				if (fk_page == 0) {			// �t�@���N�V�����L�[�P�y�[�W��
					BUZPI();
					show_index++;
					if (show_index > tki_cyusi.count - 1) {
						show_index = 0;
					}
				} else {					// �t�@���N�V�����L�[�Q�y�[�W��
					// �S�폜
					BUZPI();
					ret = pstop_del_dsp(0, 1, &work_data);
					if (ret == 0) {
						if (fk_page) {
							Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
						} else {
							Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
						}
					} else if (ret == 1) {
						return( MOD_CHG );
					}
					else if (ret == 2) {
						return( MOD_EXT );
					}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
					else if (ret == 3) {
						return( MOD_CUT );
					}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				repaint = 1;
			}
			break;
		case KEY_TEN_F3:			/* F3:�v�����g */
			break;
		case KEY_TEN_F4:			/* F4:���� */
			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
				BUZPI();
				fk_page ^= 1;
				if (fk_page) {
					Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
				} else {
					Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
				}
			}
			break;
		case KEY_TEN_F5:			/* F5:�I�� */
			BUZPI();
			return( MOD_EXT );
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:	// ���[�h�`�F���W
			BUZPI();
			return( MOD_CHG );
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^ �\��                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_show_data                                         |*/
/*| PARAMETER    :                                         |*/
/*| PARAMETER    :                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_show_data(short index, t_TKI_CYUSI *cyusi, struct TKI_CYUSI *work_data, char fk_page)
{

	if (cyusi->count != 0) {		// �f�[�^�L��
		pstop_num_dsp((short)(index + 1), cyusi->count);

		TKI_Get(work_data, index);

		pstop_data_dsp(work_data, CPrmSS[S_TIK][5]);

		if (fk_page) {
			Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
		} else {
			Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
		}

	} else {					// �f�[�^����
		pstop_num_dsp(0, 0);
		displclr(1);
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);			// "�@�@�@�f�[�^�͂���܂���@�@�@"
		displclr(3);
		displclr(4);
		displclr(5);
		displclr(6);
		Fun_Dsp(FUNMSG[8]);						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^ �����\��                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_num_dsp( short numerator, short denominator )     |*/
/*| PARAMETER    : numerator   ���q                                        |*/
/*| PARAMETER    : denominator ����                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_num_dsp(short numerator, short denominator)
{
	// �g��\��
	grachr(0, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[31]);		// [  /  ]

	// ���q��\��
	opedsp3(0, 24, (unsigned short)numerator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

	// �����\��
	opedsp3(0, 27, (unsigned short)denominator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

}

/*[]----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^ �f�[�^�\��                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_data_dsp                                          |*/
/*| PARAMETER    : index �\���Ώ�                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void pstop_data_dsp(struct TKI_CYUSI *data, long type)
{
	// ���ʍ��ڂ�\��
	grachr(1, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[29]);			// �h�c
	grachr(1, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			// �F
	opedpl(1, 6, (unsigned long)data->no, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);

	grachr(1, 23, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[19]);		// [�@�@]
	if( data->pk == CPrmSS[S_SYS][1] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);		// ��{
	}
	else if( data->pk == CPrmSS[S_SYS][2] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[26]);	// �g�P
	}
	else if( data->pk == CPrmSS[S_SYS][3] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[27]);	// �g�Q
	}
	else if( data->pk == CPrmSS[S_SYS][4] ){
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[28]);	// �g�R
	}

	grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[14]);			// ����
	grachr(2, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);			// �F
	date_dsp(2, 6, (ushort)data->year, (ushort)data->mon, (ushort)data->day, 0);	// ���t
	time_dsp(2, 16, (unsigned short)data->hour, (unsigned short)data->min, 0);		// ����

	grachr(3, 0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[32]);		// ������
	grachr(3, 8, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		// �F
	opedsp(3, 10, (unsigned short)data->tksy, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);

	grachr(3, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[32]);	// ���
	grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		// �F
	grachr(3, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[data->syubetu]);

}

/*[]----------------------------------------------------------------------[]*/
/*|  ���ԕ\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : time_dsp                                                |*/
/*| PARAMETER    : ushort low  : �s                                        |*/
/*| PARAMETER    : ushort col  : ��                                        |*/
/*| PARAMETER    : ushort hour : ��                                        |*/
/*| PARAMETER    : ushort min  : ��                                        |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	time_dsp(ushort low, ushort col, ushort hour, ushort min, ushort rev)
{
	opedsp3(low, col, hour, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �� */
	grachr(low, (ushort)(col+2), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);	/* ":" */
	opedsp3(low, (ushort)(col+3), min , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �� */
}

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
///*[]----------------------------------------------------------------------[]*/
///*|  ���ԕ\��  �S�p                                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : time_dsp2                                               |*/
///*| PARAMETER    : ushort low  : �s                                        |*/
///*| PARAMETER    : ushort col  : ��                                        |*/
///*| PARAMETER    : ushort hour : ��                                        |*/
///*| PARAMETER    : ushort min  : ��                                        |*/
///*|              : ushort rev  : 0:���] 1:���]                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	time_dsp2(ushort low, ushort col, ushort hour, ushort min, ushort rev)
//{
//	opedsp(low, col, hour, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �� */
//	grachr(low, (ushort)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		/* ":" */
//	opedsp(low, (ushort)(col+6), min , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �� */
//}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

/*[]----------------------------------------------------------------------[]*/
/*|  ���t�\��                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : date_dsp                                                |*/
/*| PARAMETER    : ushort low  : �s                                        |*/
/*| PARAMETER    : ushort col  : ��                                        |*/
/*| PARAMETER    : ushort hour : ��                                        |*/
/*| PARAMETER    : ushort min  : ��                                        |*/
/*|              : ushort rev  : 0:���] 1:���]                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static void	date_dsp(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev)
{
	opedsp3(low, col, year, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �N */
	grachr(low, (ushort)(col+2), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);	/* "/" */
	opedsp3(low, (ushort)(col+3), month, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
	grachr(low, (ushort)(col+5), 1, rev, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);	/* "/" */
	opedsp3(low, (ushort)(col+6), day , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �� */
}

/*[]----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^ �폜��ʕ\��                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_del_dsp                                           |*/
/*| PARAMETER    : index �폜�Ώ�                                          |*/
/*| RETURN VALUE : short 0:normal                                          |*/
/*| RETURN VALUE : short 1:mode change                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short pstop_del_dsp(short index, short mode, struct TKI_CYUSI* work_data)
{

	ushort	msg;
	short del_index;		// �폜�Ώۂ̔z���̃C���f�b�N�X

	displclr(1);
	if (mode) {
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "����������~�f�[�^���@�@�@�@�@"
		grachr(2, 0, 30, 0,COLOR_BLACK, LCD_BLINK_OFF,  UMSTR3[61]);		// "�@�@ �S�f�[�^���폜���܂� �@�@"
	} else {
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);		// "�@�@�@�f�[�^���폜���܂��@�@�@"
	}
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);			// "�@�@�@ ��낵���ł����H �@�@�@"
	displclr(4);
	displclr(5);
	displclr(6);
	Fun_Dsp(FUNMSG[19]);						// "�@�@�@�@�@�@ �͂� �������@�@�@"

	// �폜�Ώۂ̃C���f�b�N�X�����߂�B
	del_index = tki_cyusi.wtp - tki_cyusi.count + index;
	if (del_index < 0) {
		del_index = TKI_CYUSI_MAX + del_index;
	}

	for ( ; ; ) {
	
		msg = StoF(GetMessage(), 1);
		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ���������ꂽ��
			if ( f_NTNET_RCV_TEIKITHUSI == 1 ) {
				f_NTNET_RCV_TEIKITHUSI = 0;
				BUZPIPI();
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( 1 == pstop_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
//														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
//					return 1;
//				}
//				return 2;
				if( 1 == pstop_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
					return MOD_CHG;
				}
				else if(3 == pstop_calcel_dsp()){
					return MOD_CUT;
				} else {
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
		}

		switch (msg) {
		case KEY_TEN_F3:			/* F3:�͂� */
			BUZPI();
			if (mode) {
				// �S�폜
				TKI_DeleteAll();
				wopelg(OPLOG_TEIKICHUSI_ALLDEL, 0, 0);
				NTNET_Snd_Data219(2, NULL);
			} else {
				// �P���폜
				if (memcmp(&tki_cyusi.dt[del_index], work_data, sizeof(struct TKI_CYUSI)) == 0) {
					// �폜�f�[�^��v
					TKI_Delete(del_index);
					wopelg(OPLOG_TEIKICHUSI_DEL, 0, 0);
					NTNET_Snd_Data219(1, work_data);
				}
			}
			displclr(2);
			displclr(3);
			return( -1 );
		case KEY_TEN_F4:			/* F4:������ */
			BUZPI();
			displclr(2);
			displclr(3);
			return( 0 );
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( 3 );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:	// ���[�h�`�F���W
			BUZPI();
			return( 1 );
		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^�F�������~���                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pstop_calcel_dsp( void )                                 |*/
/*| PARAMETER    : void                                                     |*/
/*| RETURN VALUE : short 0:normal                                           |*/
/*| RETURN VALUE : short 1:mode change                                      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
static short pstop_calcel_dsp(void)
{
	ushort	msg;

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[56]);		// "����������~�f�[�^���@�@�@�@�@"
	displclr(1);
	displclr(2);
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[149]);		// "�@�@�f�[�^���X�V����܂����@�@"

	displclr(4);
	displclr(5);
	displclr(6);
	Fun_Dsp(FUNMSG[8]);						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);

		switch (msg) {
		case KEY_TEN_F5:	/* F5:�I�� */
			BUZPI();
			displclr(2);
			displclr(3);
			return( 0 );
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( 3 );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:	// ���[�h�`�F���W
			BUZPI();
			return( 1 );
		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^  �f�[�^�擾                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_Get                                                  |*/
/*| PARAMETER    : data	= �擾�f�[�^�i�[�̈�						<OUT>   |*/
/*|				   ofs	= �擾�������f�[�^�̃e�[�u���擪����̃I�t�Z�b�g    |*/
/*| RETURN VALUE : �f�[�^����̏ꍇ	��	�R�s�[���f�[�^�A�h���X              |*/
/*|				   �f�[�^�Ȃ��̏ꍇ	��	NULL                                |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
const struct TKI_CYUSI *TKI_Get(struct TKI_CYUSI *data, short ofs)
{
	short data_index;

	if (tki_cyusi.count > 0) {
		if (tki_cyusi.count > ofs) {
			data_index = tki_cyusi.wtp - tki_cyusi.count + ofs;

			if (data_index < 0) {
				data_index = TKI_CYUSI_MAX + data_index;
			}
			if (data != NULL) {
				memcpy(data, &tki_cyusi.dt[data_index], sizeof(struct TKI_CYUSI));
			}
			return &tki_cyusi.dt[data_index];
		}
	}

	return NULL;
}

/*[]-----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^  �f�[�^�폜                                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_Delete                                               |*/
/*| PARAMETER    : ofs	= �폜�������f�[�^�̃e�[�u���擪����̃I�t�Z�b�g    |*/
/*| RETURN VALUE : none                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| REMARK       : �폜��̐擪�f�[�^���z���[0]�ɂ���悤�f�[�^���\��������|*/
/*|				   ��d�ۏ؂ɑΉ�                                           |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
void	TKI_Delete(short ofs)
{

	short old_index;		// �폜�O�̍ŌÃf�[�^�̃C���f�b�N�X
	short i;

	memset(&teiki_cyusi, 0, sizeof(t_TKI_CYUSI));

	teiki_cyusi.count = tki_cyusi.count - 1;
	teiki_cyusi.wtp = teiki_cyusi.count;
	
	// �ŌÃf�[�^�C���f�b�N�X���擾
	old_index = tki_cyusi.wtp - tki_cyusi.count;
	if (old_index < 0) {
		old_index = TKI_CYUSI_MAX + old_index;
	}

	for (i = 0; i < teiki_cyusi.count; ) {
		if (old_index != ofs) {	// �폜�Ώۂƈ�v�����Ƃ��̓R�s�[���Ȃ�
			memcpy(&teiki_cyusi.dt[i], &tki_cyusi.dt[old_index], sizeof(struct TKI_CYUSI));
			i++;
		}
		old_index++;
		if (old_index >= TKI_CYUSI_MAX) {	// �z��̍ő�T�C�Y�𒴂��Ă����ꍇ�擪�ɖ߂��B
			old_index = 0;
		}
	}
	
	nmisave(&tki_cyusi, &teiki_cyusi, sizeof(t_TKI_CYUSI));

}

/*[]-----------------------------------------------------------------------[]*/
/*|  ��������Z���~�f�[�^  �f�[�^�S�폜                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TKI_DeleteAll( void )                                    |*/
/*| PARAMETER    : void                                                     |*/
/*| RETURN VALUE : none                                                     |*/
//*[]----------------------------------------------------------------------[]*/
/*| REMARK       : ��d�ۏ؂ɑΉ�                                           |*/
//*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                          |*/
/*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
void	TKI_DeleteAll(void)
{
	memset(&teiki_cyusi, 0, sizeof(t_TKI_CYUSI));
	nmisave(&tki_cyusi, &teiki_cyusi, sizeof(t_TKI_CYUSI));

}

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F������o�Ɏ����f�[�^                            |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_PassExTime( void )                                |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//unsigned short	UsMnt_PassExTime(void)
//{
//	ushort				msg;
//	char				fk_page;	// �t�@���N�V�����L�[�\���y�[�W
//	char				repaint;	// ��ʍĕ`��t���O
//	short				show_index;	// �\�����̃C���f�b�N�X
//	short				ret;
//	ushort 				data_count;
//	PASS_EXTBL			work_data;
//
//	f_NTNET_RCV_TEIKIEXTIM = 0;
//
//	dispclr();									// ��ʃN���A
//
//	fk_page = 0;
//	repaint = 1;
//	show_index = 0;
//	data_count = 0;
//
//	// ��ʃ^�C�g����\��
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);			// "������o�Ƀf�[�^���@�@�@�@�@�@"
//	Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
//
//	for ( ; ; ) {
//		if (repaint) {
//			pextim_show_data(show_index, &work_data, fk_page);
//			data_count = pas_extimtbl.Count;
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ���������ꂽ��
//			if ( f_NTNET_RCV_TEIKIEXTIM == 1 ) {
//				f_NTNET_RCV_TEIKIEXTIM = 0;
//				BUZPIPI();
//				if( 1 == pextim_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
//														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
//					return MOD_CHG;
//				}
//				else
//					return MOD_EXT;
//			}
//		}
//
//		switch (msg) {
//		case KEY_TEN_F1:			/* F1:�� */
//			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
//				BUZPI();
//				if (fk_page == 0) {			// �t�@���N�V�����L�[�P�y�[�W��
//					show_index--;
//					if (show_index < 0) {
//						show_index = pas_extimtbl.Count -1 ;
//					}
//				} else {					// �t�@���N�V�����L�[�Q�y�[�W��
//					// �폜
//					ret = pextim_del_dsp(show_index, 0, &work_data);
//					if (ret == 1) {
//						return( MOD_CHG );
//					}else if(ret == 2){
//						return( MOD_EXT);
//					} else if (ret == -1) {
//						
//						if (show_index >= pas_extimtbl.Count) {
//							show_index--;
//						}
//					}
//					if (fk_page) {
//						Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
//					} else {
//						Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
//					}
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F2:			/* F2:�� */
//			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
//				if (fk_page == 0) {			// �t�@���N�V�����L�[�P�y�[�W��
//					BUZPI();
//					show_index++;
//					if (show_index > pas_extimtbl.Count - 1) {
//						show_index = 0;
//					}
//				} else {					// �t�@���N�V�����L�[�Q�y�[�W��
//					// �S�폜
//					BUZPI();
//					ret = pextim_del_dsp(0, 1, &work_data);
//					if (ret == 0) {
//						if (fk_page) {
//							Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
//						} else {
//							Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
//						}
//					} else if (ret == 1) {
//						return( MOD_CHG );
//					}
//					else if (ret == 2) {
//						return( MOD_EXT );
//					}
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F3:			/* F3:�v�����g */
//			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
//				if (fk_page == 0) {			// �t�@���N�V�����L�[�P�y�[�W��
//				}
//			}
//			break;
//		case KEY_TEN_F4:			/* F4:���� */
//			if (data_count != 0) {	// �f�[�^�����O�ȏ�̎��L��
//				BUZPI();
//				fk_page ^= 1;
//				if (fk_page) {
//					Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
//				} else {
//					Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
//				}
//			}
//			break;
//		case KEY_TEN_F5:			/* F5:�I�� */
//			BUZPI();
//			return( MOD_EXT );
//		case KEY_MODECHG:	// ���[�h�`�F���W
//			BUZPI();
//			return( MOD_CHG );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������o�Ɏ����f�[�^ �\��                                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_show_data                                        |*/
///*| PARAMETER    :                                         |*/
///*|              :                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_show_data(short index, PASS_EXTBL *work_data, char fk_page)
//{
//
//	if (pas_extimtbl.Count != 0) {		// �f�[�^�L��
//		pextim_num_dsp((ushort)(index + 1), pas_extimtbl.Count);
//		/* �\���f�[�^�L�� */
//		memcpy(work_data, &pas_extimtbl.PassExTbl[index], sizeof(PASS_EXTBL));
//		pextim_data_dsp(work_data);
//
//		if (fk_page) {
//			Fun_Dsp(FUNMSG2[21]);						// " �폜 �S�폜�@�@�@ ����  �I�� "
//		} else {
//			Fun_Dsp(FUNMSG2[20]);						// "�@���@�@���@ ����� ����  �I�� "
//		}
//	} else {					// �f�[�^����
//		pextim_num_dsp(0, 0);
//		displclr(1);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);			// "�@�@�@�f�[�^�͂���܂���@�@�@"
//		displclr(3);
//		displclr(4);
//		displclr(5);
//		displclr(6);
//		Fun_Dsp(FUNMSG[8]);						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������o�Ɏ����f�[�^ �����\��                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_num_dsp( ushort numerator, ushort denominator )  |*/
///*| PARAMETER    : numerator   ���q                                        |*/
///*| PARAMETER    : denominator ����                                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_num_dsp(ushort numerator, ushort denominator)
//{
//	// �g��\��
//	grachr(0, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[11]);		// [    /    ]
//	// ���q��\��
//	opedsp3(0, 19, numerator, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	// �����\��
//	opedsp3(0, 24, denominator, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������o�Ɏ����f�[�^ �f�[�^�\��                                       |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_data_dsp                                         |*/
///*| PARAMETER    : index �\���Ώ�                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void pextim_data_dsp(PASS_EXTBL *data)
//{
//	// ���ʍ��ڂ�\��
//	grachr(1, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[29]);								// �h�c
//	grachr(1, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// �F
//	opedpl(1, 6, (unsigned long)data->PassId, 5, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);			// ���ID
//
//	grachr(1, 23, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_3[19]);							// [�@�@]
//	if (data->ParkNo == CPrmSS[S_SYS][1]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[0]);							// ��{
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][2]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[26]);						// �g�P
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][3]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[27]);						// �g�Q
//	}
//	else if (data->ParkNo == CPrmSS[S_SYS][4]) {
//		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[28]);						// �g�R
//	}
//
//	grachr(2, 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[15]);								// �o��
//	grachr(2, 4, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// �F
//
//	opedsp3(2, 6, (ushort)(data->PassTime.Year%100), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �N
//	grachr(2, 8, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							// "/"
//	opedsp3(2, 9, (ushort)(data->PassTime.Mon), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// ��
//	grachr(2, 11, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[0]);							// "/"
//	opedsp3(2, 12, (ushort)(data->PassTime.Day), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// ��
//	opedsp3(2, 16, (ushort)(data->PassTime.Hour), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// ��
//	grachr(2, 18, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, HALFDAT1_0[1]);							// ":"
//	opedsp3(2, 19, (ushort)(data->PassTime.Min), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// ��
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ������o�Ɏ����f�[�^ �폜��ʕ\��                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_del_dsp                                          |*/
///*| PARAMETER    : index �폜�Ώ�                                          |*/
///*| RETURN VALUE : short 0:normal                                          |*/
///*| RETURN VALUE : short 1:mode change                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static short pextim_del_dsp(short index, short mode, PASS_EXTBL *work_data)
//{
//	ushort	msg;
//
//	displclr(1);
//	if (mode) {
//		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);		// "������o�Ƀf�[�^���@�@�@�@�@"
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[61]);		// "�@�@ �S�f�[�^���폜���܂� �@�@"
//	} else {
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);		// "�@�@�@�f�[�^���폜���܂��@�@�@"
//	}
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);			// "�@�@�@ ��낵���ł����H �@�@�@"
//	Fun_Dsp(FUNMSG[19]);									// "�@�@�@�@�@�@ �͂� �������@�@�@"
//
//	for ( ; ; ) {
//	
//		msg = StoF(GetMessage(), 1);
//		if( (KEY_TEN0 <= msg) && (msg <= KEY_TEN_CL) ){	// ���������ꂽ��
//			if ( f_NTNET_RCV_TEIKIEXTIM == 1 ) {
//				f_NTNET_RCV_TEIKIEXTIM = 0;
//				BUZPIPI();
//				if( 1 == pextim_calcel_dsp() ){			// �ް����ʐM�ōX�V����Ă��邩������
//														// �ύX����Ă�����u���[�j���O��ʁv�ɑJ�ڂ���
//					return 1;
//				}
//				return 2;
//			}
//		}
//		switch (msg) {
//		case KEY_TEN_F3:			/* F3:�͂� */
//			BUZPI();
//			if (mode) {
//				// �S�폜
//				memset(&pas_extimtbl, 0, sizeof(PASS_EXTIMTBL));
//				wopelg(OPLOG_PEXTIM_ALLDEL, 0, 0);		// ���엚��o�^
//				NTNET_Snd_Data116( NTNET_PASSUPDATE_OUTTIME
//								, 0
//								, 0
//								, 0
//								, 0
//								, 0
//								, NULL );
//			}
//			else {
//				// �P���폜
//				if (memcmp(&pas_extimtbl.PassExTbl[index], work_data, sizeof(PASS_EXTBL)) == 0) {
//					// �폜�f�[�^��v
//					PassExitTimeTblDelete((ulong)work_data->ParkNo, work_data->PassId);
//					wopelg(OPLOG_PEXTIM_DEL, 0, 0);		// ���엚��o�^
//					NTNET_Snd_Data116( NTNET_PASSUPDATE_OUTTIME
//									, (ulong)work_data->PassId
//									, work_data->ParkNo
//									, 0
//									, 0
//									, 0
//									, NULL );
//				}
//			}
//			displclr(2);
//			displclr(3);
//			return( -1 );
//		case KEY_TEN_F4:	/* F4:������ */
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:	// ���[�h�`�F���W
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  ������o�Ɏ����f�[�^�F�������~���                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : pextim_calcel_dsp( void )                                 |*/
///*| PARAMETER    : void                                                     |*/
///*| RETURN VALUE : short 0:normal                                           |*/
///*| RETURN VALUE : short 1:mode change                                      |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2005 AMANO Corp.---[]*/
//static short pextim_calcel_dsp(void)
//{
//	ushort	msg;
//
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[63]);		// "������o�Ƀf�[�^���@�@�@�@�@�@"
//	displclr(1);
//	displclr(2);
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[149]);		// "�@�@�f�[�^���X�V����܂����@�@"
//	Fun_Dsp(FUNMSG[8]);						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//
//	for ( ; ; ) {
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F5:	/* F5:�I�� */
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:	// ���[�h�`�F���W
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���x���v�����^�ݒ�                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelSet( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_LabelSet(void)
{
	unsigned short	usUextEvent;
	char	wk[2];

	/* ������X�V�@�\�Ȃ��̏ꍇ�͉�ʂ𔲂��� */

	if( ( prm_get(COM_PRM,S_PAY,25,2,1) == 0 )||( !CPrmSS[S_KOU][1] ) ){	// ����X�V�Ȃ�&����������Ȃ�
		BUZPIPI();
		return MOD_EXT;
	}

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[64]);		// "�����x���v�����^�ݒ聄�@�@�@�@"

		usUextEvent = Menu_Slt( LABELSETMENU, USM_LBSET_TBL, LBSET_MENU_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usUextEvent) {

		case MNT_LBPAPER:		// �p���c�ʐݒ�
			usUextEvent = UsMnt_LabelPaperSet();
			break;

		case MNT_LBPAY:			// ���x�����s�s���̐ݒ�
			usUextEvent = UsMnt_LabelPaySet();
			break;

		case MNT_LBTESTP:		// ���x���e�X�g��
			usUextEvent = UsMnt_LabelTest();
			break;

		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			return MOD_EXT;
		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
			SetChange = 1;			// FLASH���ގw��
		}
		if (usUextEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = -1;
			OPECTL.PasswordLevel = -1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (usUextEvent == MOD_CUT ) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = -1;
			OPECTL.PasswordLevel = -1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���x���v�����^�p���c�ʐݒ�                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelPaperSet( void )                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelPaperSet(void)
{
	ushort	msg;
	long	*pPRM[2];
	long	count;
	ushort	nearend;
	char	pos;		// 0:���ݖ��� 1:�p���؂ꖇ��
	char	changing;

	dispclr();

	pPRM[0] = &PPrmSS[S_P01][10];								// ���ٌ��ݖ����ݒ�
	pPRM[1] = &CPrmSS[S_KOU][25];								// ��̫�Ė����AƱ���ޖ����ݒ�
	nearend = (ushort)prm_get( COM_PRM,S_KOU,25,2,1 );			// Ʊ���ޖ���

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[0]);							// "���p���c�ʐݒ聄�@�@�@�@�@�@�@"

	grachr(2, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);								// "���݂̏��
	grachr(2, 15, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);								// "�F"
	if( LprnErrChk( 0 ) ){										// ��ײ݈ȊO�̴װ����?
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[26]);						// 
	}else if( ERR_CHK[mod_lprn][ERR_LPRN_OFFLINE] ){			// ��ײݏ��?
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[25]);						// 
	}else{														// ����
		grachr(2, 18, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[24]);						// 
	}
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[2]);							// "�@���݂̖����@ �F �@�@�@�@���@"
	opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[3]);							// "�@�p���؂ꖇ�� �F �@�@�@�@���@"
	opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							// "�@�@�@�@�@�@�@�@�@�@�@�w�w�@�@"

	if( OPECTL.Mnt_lev >= 2 )
		Fun_Dsp( FUNMSG2[0] );									// "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� "
	else
		Fun_Dsp( FUNMSG[8] );									// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	changing = 0;
	for ( ; ; ) {
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
		
		if (changing == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* �ύX(F3) */
				if( OPECTL.Mnt_lev < 2 ) break;
				BUZPI();
				changing = 1;
				opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
				Fun_Dsp( FUNMSG2[22] );															// "�@���@�@���@ �����@�����@�I�� "
				count = -1;
				pos = 0;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				if( pos == 0 )
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
				else
					opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// "�@�@�@�@�@�@�@�@�@�@�@�w�w�@�@"�p���؂ꖇ�����]

				Fun_Dsp(FUNMSG2[0]);							// "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� "
				changing = 0;
				break;
			case KEY_TEN_F4:	/* ����(F4) */
				if( count != -1 ){
					if( pos == 0 ){
						*pPRM[0] = count;
					}else{
						*pPRM[1] /= 100;
						*pPRM[1] *= 100;						// ��2��(�p���؂ꖇ��)�폜
						*pPRM[1] += count;						// ��2��(�p���؂ꖇ��)�X�V
						nearend = (ushort)prm_get( COM_PRM,S_KOU,25,2,1 );	// Ʊ���ޖ���
					}
					OpelogNo = OPLOG_LPRPAPERSET;				// ���엚��o�^
					SetChange = 1;								// FLASH���ގw��
				}
				// no break
			case KEY_TEN_F1:	/* ��(F1) */
			case KEY_TEN_F2:	/* ��(F2) */
			case KEY_TEN_CL:	/* ���(F4) */
				BUZPI();
				if( msg == KEY_TEN_F1 || msg == KEY_TEN_F2 || msg == KEY_TEN_F4 ){
					if( pos == 0 ){
						pos = 1;
						Fun_Dsp( FUNMSG[20] );					// "�@���@�@���@�@�@�@ ����  �I�� "
					}else{
						pos = 0;
						Fun_Dsp( FUNMSG2[22] );					// "�@���@�@���@ �����@�����@�I�� "
					}
				}
				if( pos == 0 ){
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
					opedsp(5, 22, nearend, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// "�@�@�@�@�@�@�@�@�@�@�@�w�w�@�@"�p���؂ꖇ�����]
				}else{
					opedsp(4, 18, (ushort)*pPRM[0], 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
					opedsp(5, 22, nearend, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);			// "�@�@�@�@�@�@�@�@�@�@�@�w�w�@�@"�p���؂ꖇ�����]
				}
				count = -1;
				break;
			case KEY_TEN:		/* ����(�e���L�[) */
				BUZPI();
				if (count == -1)
					count = 0;
				count = (count*10 + msg-KEY_TEN0) % 10000;
				if( pos == 0 ){
					opedsp(4, 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
				}else{
					opedsp(5, 22, (ushort)count, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "�@�@�@�@�@�@�@�@�@�@�@�w�w�@�@"�p���؂ꖇ�����]
				}
				break;
			case KEY_TEN_F3:	/* ����(F3) */
				if( pos == 0 ){
					BUZPI();
					count = prm_get( COM_PRM,S_KOU,25,4,3 );	// ��̫�ėp������
					opedsp(4, 18, (ushort)count, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		// "�@�@�@�@�@�@�@�@�@�w�w�w�w�@�@"���ݖ������]
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���x�����s�s���̐ݒ�                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelPaySet( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelPaySet(void)
{
	ushort	msg;
	char	pos;

	pos = (char)(CPrmSS[S_KOU][26] % 10L);

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[1] );							// "�����x�����s�s���̐ݒ聄�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[12] );								// "�@���݂̐ݒ�@�@�@�@�@�@�@�@�@"
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]  );								// "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@"
	grachr( 2, 14, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2+pos] );							// "�X�V���Z����@"or"�X�V���Z���Ȃ�"

	grachr( 4,  8, 12, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2] );					// "�X�V���Z����@"
	grachr( 5,  8, 14, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[3] );					// "�X�V���Z���Ȃ�"
	Fun_Dsp( FUNMSG[20] );											// "�@���@�@���@�@�@�@ ����  �I�� "

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
				pos ^= 1;
				grachr( 4,  8, 12, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2] );		// "�X�V���Z����@"
				grachr( 5,  8, 14, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[3] );		// "�X�V���Z���Ȃ�"
				break;
			case KEY_TEN_F4:					/* F4:���� */
				BUZPI();
				CPrmSS[S_KOU][26] /= 10;
				CPrmSS[S_KOU][26] *= 10;							// ��1��(�󎚐��Z���̐��Z)�폜
				CPrmSS[S_KOU][26] += (long)pos;						// ��1��(�󎚐��Z���̐��Z)�X�V

				if( pos == 0 ){
					OpelogNo = OPLOG_LPRKOSINARI;					// ���x�����s�s���̍X�V���葀��o�^
				}else{
					OpelogNo = OPLOG_LPRKOSINNASI;					// ���x�����s�s���̍X�V�Ȃ�����o�^
				}
				grachr( 2, 14, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_0[2+pos] );				// "�X�V���Z����@"or"�X�V���Z���Ȃ�"
				SetChange = 1;										// FLASH���ގw��
				break;
			default:
				break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���x���e�X�g��                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LabelTest( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	unsigned short	UsMnt_LabelTest(void)
{
	ushort	msg;

	if( LprnErrChk( 1 ) ){
		BUZPIPI();													// ���ي֘A�װ����ýĈ󎚂����Ȃ�
		return( MOD_EXT );
	}

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LBSETSTR[4] );							// "���e�X�g�󎚁��@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );								// "�@�󎚒��ł��@�@�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[8] );											// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	memset( &LabelPayData, 0, sizeof( Receipt_data ) );				// ���و󎚗pܰ��ޯ̧�ر

	LabelPayData.TOutTime.Year = CLK_REC.year;						// ���s�N
	LabelPayData.TOutTime.Mon  = CLK_REC.mont;						// ���s��
	LabelPayData.TOutTime.Day  = CLK_REC.date;						// ���s��

	OpLpr_snd( &LabelPayData, 1 );									// ���ُo��(ýĈ��)

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
				// no break
			case IBK_LPR_B1_REC_OK:				/* �����OK */
			case IBK_LPR_B1_REC_NG:				/* �����NG */
				return( MOD_EXT );									// ����������ڽ��ݽ�ɂĉ�ʂ�߂�
			default:
				break;
		}
	}
}
#endif	// UPDATE_A_PASS

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\ - �N���W�b�g�����ƭ�                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreditMnu( void )                                 |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//unsigned short	UsMnt_CreditMnu(void)
//{
//	unsigned short	usUcreEvent;
//	char	wk[2];
//
//	DP_CP[0] = DP_CP[1] = 0;
//
//	for( ; ; ) {
//
//		dispclr();								// ��ʃN���A
//		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[74] );		// "���N���W�b�g�������@�@�@�@�@�@�@�@�@"
//
//		usUcreEvent = Menu_Slt( EXTENDMENU_CRE, USM_EXTCRE_TBL, (char)USM_EXTCRE_MAX, (char)1 );
//		wk[0] = DP_CP[0];
//		wk[1] = DP_CP[1];
//
//		switch( usUcreEvent ){
//			case MNT_CREUSE:
//				usUcreEvent = UsMnt_CreUseLog();					// �N���W�b�g���p����
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CRECONECT:
//				usUcreEvent = UsMnt_CreConect();					// �N���W�b�g�ڑ��m�F
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CREUNSEND:
//				usUcreEvent = UsMnt_CreUnSend();					// �N���W�b�g�����M����˗��f�[�^
//				if (usUcreEvent == MOD_CHG){
//					return(MOD_CHG);
//				}
//				break;
//
//			case MNT_CRESALENG:
//				if (OPECTL.Mnt_lev < 2){							// �W���ȉ��͑���s��
//					BUZPIPI();
//				}else{
//					usUcreEvent = UsMnt_CreSaleNG();				// �N���W�b�g���㋑�ۃf�[�^
//					if (usUcreEvent == MOD_CHG){
//						return(MOD_CHG);
//					}
//				}
//				break;
//
//			case MOD_EXT:		// �I���i�e�T�j
//				return(MOD_EXT);
//
//			case MOD_CHG:		// ���[�h�`�F���W
//				return(MOD_CHG);
//
//			default:
//				break;
//		}
//		DP_CP[0] = wk[0];
//		DP_CP[1] = wk[1];
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g�ڑ��m�F                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreConect(void)                                   |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static ushort	UsMnt_CreConect( void )
//{
//	ushort			msg;
//	uchar			mode;
//	ushort			rev;				//���]�\���p
//	short			ret;
//	DATA_KIND_137_02	*Recv137_02;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[75]);			/* "���N���W�b�g�ڑ��m�F���@�@�@�@" */
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[77]);			/* "�@�@�@�ڑ��̊m�F���ł��@�@�@�@" */
//	rev = 0;
//	grachr(5, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//
//	ret = creOnlineTestCheck();				//�ڑ��m�F
//	wopelg( OPLOG_CRE_CON_CHECK, 0, 0 );	//���샍�O�F�ڑ��m�F
//
//	switch( ret ){
//	case  0:		//�����t
//		Lagtim(OPETCBNO, 7, 25);		//�����_�ŗp�^�C�}�[�Z�b�g(500msec)
//		mode = 0;
//		break;
//	case  1:		//���u�ݒ�Ȃ��ׁ̈A�o�b�t�@���o�^�i�����ɂ͗��Ȃ��͂��j
//	case -1:		//�ڼޯĐݒ�Ȃ�
//	default:
//		BUZPIPI();
//		displclr(3);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[80]);		/* "�@�@�@�ʐM�^�C���A�E�g���@�@�@" */
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[81]);		/* "�@�@�@�@�������܂����B�@�@�@�@" */
//		Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//		mode = 2;
//		break;
//	case -2:		//�ʐM������
//		BUZPIPI();
//		displclr(3);
//		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "�@�@���݃N���W�b�g�������B�@�@" */
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "�@�r�W�[�̂��ߎ��s�ł��܂���B" */
//		Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//		mode = 2;
//		break;
//	}
//
//	for ( ; ; ) {
//		msg = StoF(GetMessage(), 1);
//		// ��M�����҂�
//		if( mode == 0 ){
//			switch (msg) {
//			case KEY_MODECHG:			// Ӱ����ݼ�
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_CHG;
//			case TIMEOUT7:				// �����_��
//				rev ^= 1;
//				grachr(5, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//				if( rev ){
//					Lagtim(OPETCBNO, 7, 50);	// �����_�ŗp�^�C�}�[���X�^�[�g(���]=1sec)
//				}
//				else{
//					Lagtim(OPETCBNO, 7, 25);	// �����_�ŗp�^�C�}�[���X�^�[�g(�ʏ�=500msec)
//				}
//				break;
//			case CRE_EVT_02_OK:			// ����
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				displclr(3);
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[78]);			/* "�@�@�@�@�@����I���@�@�@�@�@�@" */
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[82]);			/* "�@����ɐڑ�����Ă��邱�Ƃ��@" */
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[83]);			/* "�@�@�@�m�F�ł��܂����B�@�@�@�@" */
//				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//				mode = 2;
//				break;
//			case CRE_EVT_02_NG:			// �T�[�o����G���[��M
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				displclr(3);
//				displclr(5);
//				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);			/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[165]);			/* "�@�@ �G���[����M���܂��� �@�@" */
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[166]);			/* "�i�R�[�h�F�w�w�w�w�|�w�w�w�w�j" */
//				Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//				opedsp(5, 10, Recv137_02->Result1, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				opedsp(5, 20, Recv137_02->Result2, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//				mode = 2;
//				break;
//			case TIMEOUT9:				// ��M�^�C���A�E�g
//				if( cre_ctl.Status == CRE_STS_IDLE ){	// �����҂���ѱ��
//					//���g���C�s�\�ȏꍇ
//					Lagcan(OPETCBNO, 7);
//					BUZPIPI();
//					displclr(3);
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[80]);		/* "�@�@�@�ʐM�^�C���A�E�g���@�@�@" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[81]);		/* "�@�@�@�@�������܂����B�@�@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;
//				}
//				break;
//			default:
//				break;
//			}
//		}
//		// ���ʕ\����
//		else {	// if (mode == 2)
//			switch (msg) {
//			case KEY_MODECHG:	// Ӱ����ݼ�
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:	// �I��
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g�����M����˗��f�[�^         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreUnSend(void)                                   |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	UsMnt_CreUnSend( void )
//{
//	ushort	msg;
//	uchar	mode;
//	ushort	rev = 0;	//���]�\���p
//	short	ret;
//	char	repaint;	// ��ʍĕ`��t���O
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//	uchar	key_flg;	// �e�L�[�F 0=�����A1=�L��
//
//	repaint = 1;
//	mode = 0;
//	if( cre_uriage.UmuFlag == ON ){						// �����M�f�[�^����
//		key_flg = 1;
//	}else{
//		key_flg = 0;
//	}
//
//	for ( ; ; ) {
//
//		if (repaint) {
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[160]);	/* "�������M����f�[�^���@�@�@�@�@" */
//			if( cre_uriage.UmuFlag == ON ){						// �����M�f�[�^����
//
//				Fun_Dsp(FUNMSG2[60]);							/* "�@�@�@�@�@�@ ����� ���M  �I�� " */
//				if( mode == 0 ){
//					cre_unsend_data_dsp( &cre_uriage.back );	//�f�[�^�\��
//				}
//			}
//			else {												// �����M�f�[�^�Ȃ�
//				Fun_Dsp(FUNMSG[8]);								// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "�@�@�@�f�[�^�͂���܂���@�@�@"
//			}
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		/* ���M�O�`��M���� */
//		if( mode < 2 ){
//			switch (msg) {
//			case KEY_MODECHG:			// Ӱ����ݼ�
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_CHG;
//
//			case KEY_TEN_F1:			/* F1:�폜 */
//				break;
//
//			case KEY_TEN_F3:			// �v�����g
//				if( mode == 1 ){
//					break;
//				}
//				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//					BUZPIPI();
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//					if( key_flg ){
//						/*  */
//						BUZPIPI();
//						repaint = 1;
//						key_flg = 0;
//					}
//					break;
//				}
//				BUZPI();
//
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// �󎚎���	�F���ݎ���
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// ����˗��ް�
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// �󎚗v��
//
//				wopelg( OPLOG_CRE_CAN_PRN,0,0 );			// ���엚��o�^
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//
//				repaint = 1;
//				break;
//
//			case KEY_TEN_F4:			// ���M
//				if( key_flg == 0 ){
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//					BUZPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "�@�@�@�@�@ ����I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "�@�@�@ ���M�������܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;		// ���M�ς�
//					key_flg = 0;	// ���񂩂�̓L�[����
//					break;
//				}
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				rev = 0;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//
//				cre_ctl.SalesKind = CRE_KIND_MANUAL;		// ���M�v���Ɏ蓮��ݒ�
//				if(cre_ctl.Initial_Connect_Done == 0) {		// ���J��?
//					cre_ctl.OpenKind = CRE_KIND_AUTO;		// �J�ǔ����v���Ɏ�������ݒ�
//					ret = creCtrl( CRE_EVT_SEND_OPEN );		// �J�ǃR�}���h���M
//				}
//				else {
//					ret = creSendData_SALES();				//����˗��f�[�^���M
//				}
//
//				if( ret == 0 ){								//�����t�Ȃ�
//					Lagtim(OPETCBNO, 7, 25);				//�����_�ŗp�^�C�}�[�Z�b�g(500msec)
//					wopelg( OPLOG_CRE_UNSEND_SEND, 0, 0 );	//�����M����đ�
//					mode = 1;	// ���M��
//					Fun_Dsp(FUNMSG[0]);								// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@ "
//				}else if( ret == 1 ){						//��t�s�i�����ɂ͗��Ȃ��j
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}else if( ret == -2 ){						//�ʐM������
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "�@�@���݃N���W�b�g�������B�@�@" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "�@�r�W�[�̂��ߎ��s�ł��܂���B" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}
//				break;
//
//			case KEY_TEN_F5:			// �I��
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_EXT;
//
//			case CRE_EVT_02_NG:			//�J�Ǐ����G���[
//				// no break
//			case CRE_EVT_06_OK:			//����
//				// no break
//			case CRE_EVT_06_NG:			//�T�[�o����G���[��M
//				if( mode != 1 ){
//					break;				// ���M���łȂ��ꍇ�͖�������
//				}
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				dispmlclr( 2, 6 );
//				if(msg == CRE_EVT_02_NG || 
//					(msg == CRE_EVT_06_NG && ((DATA_KIND_137_06 *)cre_ctl.RcvData)->Result1 == 99)) {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//				}
//				else {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "�@�@�@�@�@ ����I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "�@�@�@ ���M�������܂��� �@�@�@" */
//				}
//				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//				mode = 2;	// ���M�ς�
//				break;
//
//			case TIMEOUT7:				// �����_��
//				rev ^= 1;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//				if( rev ){
//					Lagtim(OPETCBNO, 7, 50);	// �����_�ŗp�^�C�}�[���X�^�[�g(���]=1sec)
//				}
//				else{
//					Lagtim(OPETCBNO, 7, 25);	// �����_�ŗp�^�C�}�[���X�^�[�g(�ʏ�=500msec)
//				}
//				break;
//			case TIMEOUT9:				// ��M�^�C���A�E�g
//				if( mode != 1 ){
//					break;				// ���M���łȂ��ꍇ�͖�������
//				}
//				if( cre_ctl.Status == CRE_STS_IDLE ){		// �����҂���ѱ��
//					Lagcan(OPETCBNO, 7);
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}
//				break;
//			default:
//				break;
//			}
//		}
//		// ���ʕ\����
//		else {	// mode >= 2
//			switch (msg) {
//			case KEY_MODECHG:		// Ӱ����ݼ�
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:		// �I��
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g�����M����˗��f�[�^ �f�[�^�\��                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_unsend_data_dsp                                     |*/
///*| PARAMETER    : index �\���Ώ�                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_unsend_data_dsp(struct DATA_BK *data)
//{
//	date_time_rec	*time;
//
//	time = &data->time;
//	displclr(2);
//
//	//���Z��
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "���Z���@�@�F�@�@�@�@�@�@�@�@�@"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//���Z����
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "���Z�����@�F�@�@�@�@�@�@�@�@�@"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//���Z���z
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "���Z���z�@�F�@�@�@�@�@�@�~�@�@"
//
//	if( data->ryo > 999999L ){							// 999999��������"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "�@�@�@�@�@�@������������"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//�����
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "������@�@�@�@�@�@�@�@�@�@�@�@"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890�@�@�@�@�@"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g���㋑�ۃf�[�^               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreSalesNG( void )                                 |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	UsMnt_CreSaleNG( void )
//{
//	ushort	msg;
//	char	repaint;	// ��ʍĕ`��t���O
//	short	show_index;	// �\�����̃C���f�b�N�X
//	char	data_count;	// �X�V��̃f�[�^����
//	T_FrmSaleNGPriReq	saleng_pri;
//
//	dispclr();									// ��ʃN���A
//
//	repaint = 1;
//	show_index = 0;
//	data_count = 0;
//
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[161]);	/* "�����㋑�ۃf�[�^���@" */
//
//	for ( ; ; ) {
//	
//		if (repaint) {
//			cre_saleng_show_data( show_index );	//�f�[�^�\��
//			data_count = cre_saleng.ken;
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F1:			/* F1:�� */
//			if (data_count > 1) {	// �f�[�^����2�ȏ�̎��L��
//				BUZPI();
//				if (--show_index < 0) {
//					show_index = cre_saleng.ken - 1;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F2:			/* F2:�� */
//			if (data_count > 1) {	// �f�[�^����2�ȏ�̎��L��
//				BUZPI();
//				if (++show_index > cre_saleng.ken - 1) {
//					show_index = 0;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F3:			/* F3:�v�����g */
//			if (data_count != 0) {	// �f�[�^����1�ȏ�̎��L��
//				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				saleng_pri.prn_kind = R_PRI;
//				saleng_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &saleng_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );						// �󎚎���	�F���ݎ���
//				saleng_pri.ng_data = &cre_saleng;														// �󎚎���	�F���㋑���ް�
//				queset( PRNTCBNO, PREQ_CREDIT_SALENG, (char)sizeof(T_FrmSaleNGPriReq), &saleng_pri );	// �󎚗v��
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//			}
//			break;
//		case KEY_TEN_F4:			/* F4:�폜 */
//			break;
//
//		case KEY_TEN_F5:			/* F5:�I�� */
//			BUZPI();
//			return( MOD_EXT );
//		case KEY_MODECHG:	// ���[�h�`�F���W
//			BUZPI();
//			return( MOD_CHG );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g���㋑��-�t�@���N�V�����\��  |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_CreSaleNG_fnc(char ken)                           |*/
///*| PARAMETER    : char ken : ����                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void	UsMnt_CreSaleNG_fnc( char ken )
//{
//	if( ken > 1 ){	// 2���ȏ�
//		Fun_Dsp(FUNMSG2[61]);						// "�@���@�@���@ ����ā@�@�@ �I�� "
//	} else {		// 2������
//		Fun_Dsp(FUNMSG2[62]);						// "�@�@�@�@�@�@ ����ā@�@�@ �I�� "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �\��                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_show_data                                    |*/
///*| PARAMETER    : index     : �\������e�[�u���ԍ�                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_show_data( short index )
//{
//	if (cre_saleng.ken != 0) {		// �f�[�^�L��
//		cre_saleng_num_dsp( (uchar)(index + 1), cre_saleng.ken );	// [ / ]
//		cre_saleng_data_dsp( index );
//		UsMnt_CreSaleNG_fnc( cre_saleng.ken );
//	} else {						// �f�[�^����
//		cre_saleng_num_dsp( 0, 0 );
//		displclr(1);
//		displclr(2);
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "�@�@�@�f�[�^�͂���܂���@�@�@"
//		displclr(4);
//		displclr(5);
//		displclr(6);
//		Fun_Dsp(FUNMSG[8]);					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �����\��                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_num_dsp( uchar numerator, char denominator ) |*/
///*| PARAMETER    : numerator   ���q                                        |*/
///*| PARAMETER    : denominator ����                                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_saleng_num_dsp( uchar numerator, char denominator )
//{
//	// �g��\��
//	grachr(0, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[31]);	// [  /  ]
//
//	// ���q��\��
//	opedsp3(0, 24, (unsigned short)numerator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//
//	// �����\��
//	opedsp3(0, 27, (unsigned short)denominator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �f�[�^�\��                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_data_dsp                                     |*/
///*| PARAMETER    : index �\���Ώ�                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_data_dsp( short index )
//{
//	struct DATA_BK	*data;
//	date_time_rec	*time;
//
//	data = &cre_saleng.back[ index ];
//	time = &data->time;
//	displclr(2);
//
//	//���Z��
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "���Z���@�@�F�@�@�@�@�@�@�@�@�@"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//���Z����
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "���Z�����@�F�@�@�@�@�@�@�@�@�@"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//���Z���z
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "���Z���z�@�F�@�@�@�@�@�@�~�@�@"
//
//	if( data->ryo > 999999L ){							// 999999��������"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "�@�@�@�@�@�@������������"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//�����
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "������@�@�@�@�@�@�@�@�@�@�@�@"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890�@�@�@�@�@"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���t�\��  �S�p (�����N�^�������^������)                               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : date_dsp3                                               |*/
///*| PARAMETER    : ushort low  : �s                                        |*/
///*| PARAMETER    : ushort col  : ��                                        |*/
///*| PARAMETER    : ushort hour : ��                                        |*/
///*| PARAMETER    : ushort min  : ��                                        |*/
///*|              : ushort rev  : 0:���] 1:���]                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void	date_dsp3(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev)
//{
//
//	opedsp(low, col  , year, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �N */
//	grachr(low, (unsigned short)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "�^" */
//	opedsp(low, (unsigned short)(col+6)  , month, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
//	grachr(low, (unsigned short)(col+10), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "�^" */
//	opedsp(low, (unsigned short)(col+12), day , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ���[�U�[�����e�i���X�F���p���׃��O                					   |*
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : UsMnt_CreUseLog										   |*
// *| PARAMETER    : none													   |*
// *| RETURN VALUE : MOD_CHG : mode change								   |*
// *|              : MOD_EXT : F5 key										   |*
// *[]----------------------------------------------------------------------[]*
// *| REMARK       : 														   |*
// *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//ushort	UsMnt_CreUseLog(void)
//{
//	date_time_rec	NewTime;
//	date_time_rec	OldTime;
//	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
//	ushort			pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
//	T_FrmLogPriReq3	FrmLogPriReq3;		// �󎚗v��ү����ܰ�
//	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
//	ushort			Date_Now[6];		// ���t�ް��i[0]�F�J�n�N�A[1]�F�J�n���A[2]�F�J�n���A[3]�F�I���N�A[4]�F�I�����A[5]�F�I�����j
//	uchar			pos		= 0;		// �����ް����͈ʒu�i�O�F�J�n�N�A�P�F�J�n���A�Q�F�J�n���A�R�F�I���N�A�S�F�I�����A�T�F�I�����j
//	short			inp		= -1;		// �����ް�
//	uchar			All_Req	= OFF;		// �u�S�āv�w�蒆�׸�
//	uchar			Date_Chk;			// ���t�w���ް������׸�
//	ushort			Sdate;				// �J�n���t
//	ushort			Edate;				// �I�����t
//	uchar			end_flg;			// �N���A�����ʏI���׸�
//	ushort			Date_Old[6];		// ���t�ް��i�J�n�N[0],��[1],��[2],��[3],��[4]�A�I��[5],��[6],��[7],��[8],��[9]�j
//	ushort			LogCnt_total;		// �����O����
//	ushort			LogCnt;				// �w����ԓ��Ώۃ��O����
//
//	Ope2_Log_NewestOldestDateGet( eLOG_CREUSE, &NewTime, &OldTime, &LogCnt_total );
//
//	/* ������ʕ\�� */
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[88] );	// "���N���W�b�g���p���ׁ��@�@�@�@"
//	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );	// �ŌÁ^�ŐV���O���t�\��
//	Date_Now[0] = (ushort)OldTime.Year;								// �J�n�i�N�j���ŌÃ��O�i�N�j
//	Date_Now[1] = (ushort)OldTime.Mon;								// �J�n�i���j���ŌÃ��O�i���j
//	Date_Now[2] = (ushort)OldTime.Day;								// �J�n�i���j���ŌÃ��O�i���j
//	
//	Date_Now[3] = (ushort)NewTime.Year;								// �J�n�i�N�j���ŐV���O�i�N�j
//	Date_Now[4] = (ushort)NewTime.Mon;								// �J�n�i���j���ŐV���O�i���j
//	Date_Now[5] = (ushort)NewTime.Day;								// �J�n�i���j���ŐV���O�i���j
//	memcpy(Date_Old, Date_Now, sizeof(Date_Old));					// �ύX�O���t�ް��Z�[�u
//	LogDateDsp4( &Date_Now[0] );									// �ŌÁ^�ŐV���O���t�f�[�^�\��
//	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
//	LogDateDsp5( &Date_Now[0], pos );								// ������t�f�[�^�\��
//	
//	// �w����ԓ����O�����擾
//	Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);	// �J�n���t�ް��擾
//	Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);	// �I�����t�ް��擾
//	LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//	LogCntDsp( LogCnt );											// ���O�����\��
//	
//	Fun_Dsp( FUNMSG[83] );											// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
//
//	/* ��ʑ���󂯕t�� */
//	for( ; ; ){
//		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�
//
//		/*  �󎚗v���O�i�󎚗v���O��ʁj*/
//		if( pri_cmd == 0 ){
//			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H
//
//				/* �ݒ�L�[�ؑ� */
//				case KEY_MODECHG:
//					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
//					break;
//
//				/* �O�`�X */
//				case KEY_TEN:
//					if( All_Req == OFF ){					// �u�S�āv�w�蒆�łȂ��ꍇ�i�w�蒆�́A��������j
//						BUZPI();
//						if( inp == -1 ){
//							inp = (short)(RcvMsg - KEY_TEN0);
//						}else{
//							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
//						}
//						opedsp(								// �����ް��\��
//								POS_DATA1_0[pos][0],		// �\���s
//								POS_DATA1_0[pos][1],		// �\�����
//								(ushort)inp,				// �\���ް�
//								POS_DATA1_0[pos][2],		// �ް�����
//								POS_DATA1_0[pos][3],		// �O�T�v���X�L��
//								1,							// ���]�\���F����
//								COLOR_BLACK,				// �����F�F��
//								LCD_BLINK_OFF				// �u�����N�F���Ȃ�
//						);
//					}
//					break;
//
//				/* ����L�[ */
//				case KEY_TEN_CL:
//					BUZPI();
//					displclr( 1 );							// 1�s�ڕ\���N���A
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
//						opedsp(								// ���͑O���ް���\��������
//								POS_DATA1_0[pos][0],		// �\���s
//								POS_DATA1_0[pos][1],		// �\�����
//								Date_Now[pos],				// �\���ް�
//								POS_DATA1_0[pos][2],		// �ް�����
//								POS_DATA1_0[pos][3],		// �O�T�v���X�L��
//								1,							// ���]�\���F����
//								COLOR_BLACK,				// �����F�F��
//								LCD_BLINK_OFF				// �u�����N�F���Ȃ�
//						);
//						inp = -1;							// ���͏�ԏ�����
//					}
//					break;
//
//				/* �e�P�i���j�e�Q�i���j�L�[���� */
//				case KEY_TEN_F1:
//				case KEY_TEN_F2:
//					displclr( 1 );							// 1�s�ڕ\���N���A
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ�́i�@���@�@���@�L�[�F���t���́j
//						if( inp == -1 ){		// ���͂Ȃ��̏ꍇ
//							BUZPI();
//							if( RcvMsg == KEY_TEN_F1 ){
//								LogDatePosUp( &pos, 0 );			// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
//							}else{
//								LogDatePosUp( &pos, 1 );			// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
//							}
//						}
//						else{				//	���͂���̏ꍇ
//							if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j
//								BUZPI();
//								if( pos == 0 ){							// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
//									Date_Now[pos] =
//									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//								}else{
//									Date_Now[pos] =
//									Date_Now[pos+3] = (ushort)inp;
//								}
//								if( RcvMsg == KEY_TEN_F1 ){
//									LogDatePosUp( &pos, 0 );			// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
//								}else{
//									LogDatePosUp( &pos, 1 );			// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
//								}
//							}
//							else{								//	�����ް��m�f
//								BUZPIPI();
//							}
//						}
//						LogDateDsp5( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
//						inp = -1;								// ���͏�ԏ�����
//					}
//					else if( RcvMsg == KEY_TEN_F2 ){		// �u�S�āv�w�蒆�ꍇ�́i�e�Q�F�u�N���A�v�L�[�j
//						BUZPI();
//						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "�@�@���O���N���A���܂����H�@�@"
//						Fun_Dsp(FUNMSG[19]);							// "�@�@�@�@�@�@ �͂� �������@�@�@"
//
//						for( end_flg = OFF ; end_flg == OFF ; ){
//							RcvMsg = StoF( GetMessage(), 1 );			// �C�x���g�҂�
//
//							switch( RcvMsg ){							// �C�x���g�H
//								case KEY_MODECHG:						// �ݒ�L�[�ؑ�
//									RcvMsg = MOD_CHG;					// �O��ʂɖ߂�
//									end_flg = ON;
//									break;
//
//								case KEY_TEN_F3:						// �e�R�i�͂��j�L�[����
//									BUZPI();
//									wopelg( OPLOG_CRE_USE_DEL,0,0 );		// ���엚��o�^
//									if( LogDataClr( eLOG_CREUSE ) == OK ){	// ���O�f�[�^�N���A
//										LogCnt_total = 0;				// ���O�����i�������Ұ��j�N���A
//										LogCntDsp( LogCnt_total );		// ���O�����\���i�O���j
//										Date_Now[0] = Date_Now[3] = (ushort)CLK_REC.year;	// �J�n�A�I���i�N�j�����ݓ����i�N�j
//										Date_Now[1] = Date_Now[4] = (ushort)CLK_REC.mont;	// �J�n�A�I���i���j�����ݓ����i���j
//										Date_Now[2] = Date_Now[5] = (ushort)CLK_REC.date;	// �J�n�A�I���i���j�����ݓ����i���j
//									}
//									end_flg = ON;
//									break;
//
//								case KEY_TEN_F4:						// �e�S�i�������j�L�[����
//									BUZPI();
//									end_flg = ON;
//									break;
//							}
//							if( end_flg != OFF ){
//								displclr( 6 );										// "�@�@���O���N���A���܂����H�@�@"�\���s�N���A
//								LogDateDsp4( &Date_Now[0] );						// �ŌÁ^�ŐV���O���t�f�[�^�\��
//								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
//								LogDateDsp5( &Date_Now[0], pos );					// ������t�f�[�^�\��
//								Fun_Dsp( FUNMSG[83] );								// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
//								All_Req = OFF;										// �u�S�āv�w�蒆�׸�ؾ��
//							}
//						}
//					}
//					break;
//
//				/* �e�R�i�S�āj�L�[���� */
//				case KEY_TEN_F3:
//					displclr( 1 );							// 1�s�ڕ\���N���A
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j
//						if( LogCnt_total != 0 ){		//	���O�f�[�^������ꍇ
//							BUZPI();
//							Date_Now[0] = (ushort)OldTime.Year;	// �J�n�i�N�j���ŌÃ��O�i�N�j
//							Date_Now[1] = (ushort)OldTime.Mon;	// �J�n�i���j���ŌÃ��O�i���j
//							Date_Now[2] = (ushort)OldTime.Day;	// �J�n�i���j���ŌÃ��O�i���j
//							
//							Date_Now[3] = (ushort)NewTime.Year;	// �J�n�i�N�j���ŐV���O�i�N�j
//							Date_Now[4] = (ushort)NewTime.Mon;	// �J�n�i���j���ŐV���O�i���j
//							Date_Now[5] = (ushort)NewTime.Day;	// �J�n�i���j���ŐV���O�i���j
//							pos = 0;								// ���وʒu���J�n�i�N�j
//							inp = -1;								// ���͏�ԏ�����
//							displclr( 5 );						// ������t�\���N���A
//							// �w����ԓ��Ώۃ��O�������擾���\������
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
//							Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// �I�����t�ް��擾
//							LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//							LogCntDsp( LogCnt );					// ���O�����\��
//							Fun_Dsp( FUNMSG[85] );					// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
//							All_Req = ON;							// �u�S�āv�w�蒆�׸޾��
//						}
//						else{					//	���O�f�[�^���Ȃ��ꍇ
//							BUZPIPI();
//						}
//					}
//					break;
//
//				/* �e�S�i���s�j�L�[���� */
//				case KEY_TEN_F4:
//					displclr( 1 );							// 1�s�ڕ\���N���A
//					Date_Chk = OK;
//					if( inp != -1 ){	//	���͂���̏ꍇ
//						if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�̏ꍇ
//							if( pos == 0 ){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
//								Date_Now[pos] =
//								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//							}else{
//								Date_Now[pos] =
//								Date_Now[pos+3] = (ushort)inp;
//							}
//						}else{								//	���̓f�[�^�m�f�̏ꍇ
//							Date_Chk = NG;
//						}
//					}
//					// �w����ԓ��Ώۃ��O�������擾���\������
//					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
//					Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// �I�����t�ް��擾
//					LogCnt = UsMnt_CreUseLog_GetLogCnt( Sdate, Edate );
//					if( LogCnt == 0 ){
//						BUZPIPI();
//						grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// �f�[�^�Ȃ��\��
//						inp = -1;									// ���͏�ԏ�����
//						break;
//					}
//					if( Date_Chk == OK ){
//						if( All_Req != ON ){	// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��
//							//	�N���������݂�����t���`�F�b�N����
//							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// �J�n���t����
//								Date_Chk = NG;
//							}
//							if( chkdate( (short)Date_Now[3], (short)Date_Now[4], (short)Date_Now[5] ) != 0 ){	// �I�����t����
//								Date_Chk = NG;
//							}
//						}
//					}
//					if( Date_Chk == OK ){
//						//	�J�n���t�����I�����t���`�F�b�N����
//						Sdate = dnrmlzm(							// �J�n���t�ް��擾
//											(short)Date_Now[0],
//											(short)Date_Now[1],
//											(short)Date_Now[2]
//										);
//						Edate = dnrmlzm(							// �I�����t�ް��擾
//											(short)Date_Now[3],
//											(short)Date_Now[4],
//											(short)Date_Now[5]
//										);
//						if( Sdate > Edate ){						// �J�n���t�^�I�����t����
//							Date_Chk = NG;
//						}
//					}
//					if( Date_Chk == OK ){	// �J�n���t���I�����t�����n�j�̏ꍇ
//						if(Ope_isPrinterReady() == 0){					// ���V�[�g�v�����^���󎚕s�\�ȏ��
//							BUZPIPI();
//							break;
//						}
//						BUZPI();
//						/*------	�󎚗v��ү���ޑ��M	-----*/
//						memset( &FrmLogPriReq3,0,sizeof(FrmLogPriReq3) );		// �󎚗v��ү����ܰ��O�ر
//						memcpy( &FrmLogPriReq3.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// �󎚎���	�F���ݎ���
//						FrmLogPriReq3.prn_kind	= R_PRI;						// �Ώ�������Fڼ��
//						FrmLogPriReq3.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
//						FrmLogPriReq3.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.
//
//						if( All_Req == ON ){
//							FrmLogPriReq3.BType	= 0;							// �������@	�F�S��
//						}else{
//							FrmLogPriReq3.BType	= 1;							// �������@	�F���t
//							FrmLogPriReq3.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq3.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq3.TSttTime.Day	= (uchar)Date_Now[2];
//						}
//						FrmLogPriReq3.LogCount = LogCnt_total;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
//						queset( PRNTCBNO, PREQ_CREDIT_USE, sizeof(T_FrmLogPriReq3), &FrmLogPriReq3 );
//						Ope_DisableDoorKnobChime();
//						pri_cmd = PREQ_CREDIT_USE;				// ���M����޾���
//						if( All_Req != ON ){					// �u�S�āv�w��Ȃ�
//							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
//							LogDateDsp5( &Date_Now[0], 0xff );	// ������t�f�[�^�\��
//						}
//						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// ���s���u�����N�\��
//						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
//
//					}else{	// �J�n���t���I�����t�����m�f�̏ꍇ
//						BUZPIPI();
//						LogDateDsp5( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
//						inp = -1;								// ���͏�ԏ�����
//					}
//					break;
//
//				/* �e�T�i�I���j�L�[���� */
//				case KEY_TEN_F5:							// 
//					BUZPI();
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
//						RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
//					}else{					//	�u�S�āv�w�蒆�̏ꍇ
//						pos = 0;							// ���وʒu���J�n�i�N�j
//						inp = -1;							// ���͏�ԏ�����
//						opedsp(								// ���ق�\��������
//								POS_DATA1_0[pos][0],		// �\���s
//								POS_DATA1_0[pos][1],		// �\�����
//								Date_Now[pos],				// �\���ް�
//								POS_DATA1_0[pos][2],		// �ް�����
//								POS_DATA1_0[pos][3],		// �O�T�v���X�L��
//								1,							// ���]�\���F����
//								COLOR_BLACK,				// �����F�F��
//								LCD_BLINK_OFF				// �u�����N�F���Ȃ�
//						);
//						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
//						LogDateDsp5( &Date_Now[0], pos );	// ������t�f�[�^�\��
//						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
//						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
//					}
//					break;
//				default:
//					break;
//			}
//		}
//		/* �󎚗v����i�󎚏I���҂���ʁj*/
//		else{
//			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
//				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
//			}
//
//			switch( RcvMsg ){								// �C�x���g�H
//
//				case KEY_MODECHG:	// �ݒ�L�[�ؑ�
//					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
//					break;
//
//				case KEY_TEN_F3:	// �e�R�i���~�j�L�[����
//					BUZPI();
//					/*------	�󎚒��~ү���ޑ��M	-----*/
//					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
//					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
//					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
//					break;
//
//				default:
//					break;
//			}
//		}
//
//		if( (RcvMsg == MOD_EXT) || (RcvMsg == MOD_CHG) ){
//			break;											// �O��ʂɖ߂�
//		}
//	}
//	return( RcvMsg );
//}
//
//ushort	UsMnt_CreUseLog_GetLogCnt(ushort Sdate, ushort Edate)
//{
//	ushort			log_date;			// ���O�i�[���t
//	ushort			LOG_Date[3];		// ���O�i�[���t�i[0]�F�N�A[1]�F���A[2]�F���j
//	ushort			log_cnt;			// �w����ԓ����O����
//	ushort			cnt;
//	meisai_log 		wk_log_dat;
//	ushort			LogCnt_total;
//	
//	log_cnt = 0;
//	LogCnt_total = Ope_Log_TotalCountGet( eLOG_CREUSE );
//	
//	for(cnt=0; cnt < LogCnt_total; cnt++) {
//		if( 0 == Ope_Log_1DataGet(eLOG_CREUSE, cnt, &wk_log_dat) ){
//			log_cnt = 0;
//			break;
//		}
//		LOG_Date[0] = wk_log_dat.PayTime.Year;
//		LOG_Date[1] = wk_log_dat.PayTime.Mon;
//		LOG_Date[2] = wk_log_dat.PayTime.Day;
//		log_date = dnrmlzm( (short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]);
//		if( (log_date >= Sdate) && (log_date <= Edate) ){
//			log_cnt++;
//		}
//	}
//	return(log_cnt);
//}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\ - �d���������ƭ�					   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : UsMnt_EdyMnu( void )                                    |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-11-06 Suzuki                                       |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//unsigned short	UsMnt_EdyMnu(void)
// {
// 	unsigned short	usUcreEvent;
// 	char	wk[2];
// 	unsigned char	MenuValue = 0;					// ���j���[���ڂ̕\����
// 	CeMenuPara make_table[] = 						// ���j���[�\�������e�[�u��
// 	{
// 		{0,			0,			0,		0},			// �A���[��������
// 		{0,			0,			0,		0},			// �Z���^�[�ʐM�J�n
// 		{0,			0,			0,		0},			// ���ߋL�^���
// 		{S_SCA,		59,			5,		1},			// �����W�v�v�����g
// 		{S_SCA,		59,			6,		1},			// ���p���׃v�����g
// #if (4 == AUTO_PAYMENT_PROGRAM)										// �����p��EM�ֺ���ޑ��M�idebug�����j
// 		{0,			0,			0,		0},			// �d�����M�e�X�g
// #endif
// 	};
//
// 	DP_CP[0] = DP_CP[1] = 0;
//
// 	/* �ǂ̈󎚋@�\���L���ł��邩�����ʃp�����[�^����擾���A�\���ۂ̔��f������ */
// 	MenuValue = UsMnt_DiditalCasheMenuMake31(make_table, EXTENDMENU_EDY, USM_EXTEDY_TBL, (sizeof(make_table) / sizeof(CeMenuPara)));
//
// 	for( ; ; ) {
//
// 		dispclr();								// ��ʃN���A
// 		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[101] );	// "���d�����������@�@�@�@�@�@�@�@"
//
// 		usUcreEvent = Menu_Slt( (void*)UsMnt_DiditalCasheMENU_STR, (void*)UsMnt_DiditalCasheMENU_CTL, (char)MenuValue, (char)1 );
// 		wk[0] = DP_CP[0];
// 		wk[1] = DP_CP[1];
//
// 		switch( usUcreEvent ){
// 			case MNT_EDY_ARM:										// �A���[��������
// 				usUcreEvent = UsMnt_Edy_PayArmPrint();
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_CENT:
// 				usUcreEvent = UsMnt_Edy_CentCon( );					// �Z���^�[�ʐM�J�n
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_SHIME:										// �d�������ߋL�^���
// 				usUcreEvent = UsMnt_Edy_ShimePrint();
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_MEISAI:									// �d�������p����
// 				usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_EDY_MEISAI);
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 			case MNT_EDY_SHUUKEI:									// �d�����W�v
// 				usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_EDY_SHUUKEI);
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
//
// 		#if (4 == AUTO_PAYMENT_PROGRAM)								// �����p��EM�ֺ���ޑ��M�idebug�����j
// 			case MNT_EDY_TEST:
// 				usUcreEvent = UsMnt_Edy_Test( );					// �Z���^�[�ʐM�J�n
// 				if (usUcreEvent == MOD_CHG){
// 					return(MOD_CHG);
// 				}
// 				break;
// 		#endif
//
// 			case MOD_EXT:		// �I���i�e�T�j
// 				return(MOD_EXT);
//
// 			case MOD_CHG:		// ���[�h�`�F���W
// 				return(MOD_CHG);
//
// 			default:
// 				break;
// 		}
// 		DP_CP[0] = wk[0];
// 		DP_CP[1] = wk[1];
// 	}
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ���[�U�[�����e�i���X�F�g���@�\ - �d���������i�P�D�A���[��������j   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_PayArmPrint( void )                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_PayArmPrint( void )
// {
// 	ushort	usSysEvent;
// 	ushort	NewOldDate[6];				// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
// 	ushort	LogCount;					// LOG�o�^����
// 	date_time_rec	NewestDateTime, OldestDateTime;
//
//  	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
// 	Ope2_Log_NewestOldestDateGet( eLOG_EDYARM, &NewestDateTime, &OldestDateTime, &LogCount );
//
// 	// �ŌÁ��ŐV���t�ް��擾
// 	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
//
// 	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
// 	usSysEvent = SysMnt_Log_CntDsp2	(
// 										LOG_EDYARM,			// ���O���
// 										LogCount,			// ���O����
// 										UMSTR3[102],		// �^�C�g���\���f�[�^�|�C���^
// 										PREQ_EDY_ARMPAY_LOG,// �󎚗v��ү���޺����
// 										&NewOldDate[0]		// �ŌÁ��ŐV�ް����t�ް��߲��
// 									);
// 	return usSysEvent;
//
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ���[�U�[�����e�i���X�F�g���@�\ - �d���������i�Q�D�Z���^�[�ʐM�J�n�j   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_CentCon( void )                               |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*| Date         : 2007-02-26                                              |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_CentCon( void )
// {
// 	R_CENTER_TRAFFIC_RESULT_DATA	*p_rcv_data;		// �Z���^�[�ʐM���{���ʃf�[�^�߲��
// 	R_CENTER_TRAFFIC_CHANGE_DATA	*p_rcv_center;		// �����ʐM�󋵕ω��ʒm�f�[�^�߲��
// 	ushort	msg;
// 	char	wk_stat;	// ���{���(0:�������/1:�J�n�w��/2:�ʐM�J�n/3:�ʐM�I��/9:�ُ퉞��)
// 	uchar	EndKey_stat = 0;							// F5�L�[�L��(1)�^����(0)
// 	ushort	rev;
// 	uchar	LimitTime;									// �����ʐM�I��Limit����
//
// 	/* Edy���p�\��Ԃ����`�F�b�N���� */
// 	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 &&		// Edy���p�\�ݒ肩�H
// 		Edy_Rec.edy_EM_err.BIT.Comfail == 0 &&			// EM�ʐM�G���[���������Ă��Ȃ����H
// 		Edy_Rec.edy_status.BIT.INITIALIZE ) {			// EdyӼޭ�ق̏��������������Ă��邩�H
// 		dispclr();										// ������ʕ\��
// 		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[103]);				/* "���Z���^�[�ʐM��          �@�@" */
// 		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[104]);				/* "�@�d�����Z���^�[�Ɛڑ����܂��@" */
// 		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[105]);				/* "�ʐM���͑��̑���͂ł��܂���" */
// 		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[106]);				/* "�@���s���Ă���낵���ł����H�@" */
// 		Fun_Dsp(FUNMSG[19]);							/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
// 		EndKey_stat = 0;								// F5�L�[�L��(1)�^����(0)
// 	} else {											// Edy���p�s���͖{��ʂɑJ�ڂ����Ȃ��i�x�����j
// 		BUZPIPI();
// 		return MOD_EXT;										
// 	}
//
// 	wk_stat = 0;											// ���{���(0:�������)
// 	LimitTime = (uchar)prm_get(COM_PRM, S_SCA, 61, 2, 3);
//
// 	CneterComLimitTime = 0;
// 	Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;			// �����ʐM�׸ޏ�����
//
// 	for ( ; ; ) {
//
// 		msg = StoF( GetMessage(), 1 );
//
// 		if( Edy_Rec.edy_status.BIT.CENTER_COM_START && CneterComLimitTime > LimitTime+5 ){
// 			if(wk_stat >= 1) {										// �J�n�w���ȍ~�̂ݎ�t
// 				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* �\���s�i3�j���N���A */
// 				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "�@�@�d�����Z���^�[�ʐM�I���@�@" */
// 				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "�@�@�@�@�y�ُ�I���z�@�@�@�@�@" */
// 				Fun_Dsp(FUNMSG[8]);									/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// 				EndKey_stat = 1;									// F5�L�[�L��(1)�^����(0)
// 				wk_stat = 9;										// �ُ퉞��
// 				Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// �����ʐM�I��
// 				CneterComLimitTime = 0;
// 				wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
// 			}
// 		}		
// 		switch( msg ){		/* FunctionKey Enter */
//
// 			case KEY_MODECHG:
// 				if(wk_stat == 1 || wk_stat == 2 ) break;	/* ������ʕ\����/�ʐM�����ȊO�͖��� */
// 				return MOD_CHG;
//
// 			case KEY_TEN_F1:								/* "IPaddr" */
// 				break;
//
// 			case KEY_TEN_F3:								/* "�͂�" */
// 				if(wk_stat != 0) break;						/* ������ʕ\�����ȊO�͖��� */
// 				BUZPI();
// 				auto_cnt_ndat = Nrm_YMDHM((date_time_rec *)&CLK_REC);	// �ŏI�Z���^�[�ʐM�J�n�����X�V
// 				Edy_SndData13(0);							// �Z���^�[�ʐM�J�n�w���i���߂Ȃ��j	���M
// 				wk_stat = 1;								// �Z���^�[�ʐM�J�n�w����
// 				Edy_Rec.edy_status.BIT.CENTER_COM_START = 1;// �����ʐM�J�n
// 															// �u�Z���^�[�ʐM���v��ʕ\��
// 				grachr(2, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);			/* "�@�@�d�����Z���^�[�ƒʐM���@�@" �i�_�ŕ\���j*/
// 				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[9]);			/* " �ʐM���͑��̑��삪�ł��܂��� " */
// 				Fun_Dsp(FUNMSG[0]);								/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
// 				EndKey_stat = 0;								// F5�L�[�L��(1)�^����(0)
// 				rev = 1;
// 				Lagtim(OPETCBNO, 7, 50);						// �_�ŗpTimer�X�^�[�g(1sec)
// 				break;
//
// 			case KEY_TEN_F4:										/* "������" */
// 				if(wk_stat != 0) break;								// ������ʕ\�����ȊO�͖���
// 				BUZPI();
// 				return MOD_EXT;
//
// 			case KEY_TEN_F5:										/* "�I��" */
// 				if(EndKey_stat == 0) break;							// F5�L�[����(0)���͖���
// 				BUZPI();
// 				Lagcan(OPETCBNO, 7);								/* �_�ŕ\���p�^�C�}�[�J�� */
// 				return MOD_EXT;
//
// 			case IBK_EDY_RCV:										/* EDY ��M����� ��M */
// 				switch( Edy_Rec.rcv_kind ){							// ��M�ް����
//
// 					case	R_EMSET_STATUS_READ:					// EM�ݒ�/�ð���ǂݏo���w���̉���
// 						memcpy(&Edy_SetStatus_ReadData, Edy_Rec.rcv_data, sizeof(Edy_SetStatus_ReadData));	// EM�ݒ�/�ð���ް��ۑ�
// 						if( (Edy_SetStatus_ReadData.Line_kind == 2) ) {	// �����ʁFLAN
// 							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[108]);
// 							opedsp3(6, 12, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[0], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 16, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[1], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 20, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[2], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 							opedsp3(6, 24, (ushort)Edy_SetStatus_ReadData.EM_IPaddress[3], 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
// 						}
// 						break;			
//
// 					case	R_FIRST_STATUS:										// ������Ԓʒm ��M
// 						if(wk_stat >= 1) {										// �J�n�w���ȍ~�̂ݎ�t
// 							grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* �\���s�i3�j���N���A */
// 							grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "�@�@�d�����Z���^�[�ʐM�I���@�@" */
// 							grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "�@�@�@�@�y�ُ�I���z�@�@�@�@�@" */
// 							Fun_Dsp(FUNMSG[8]);									/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// 							EndKey_stat = 1;									// F5�L�[�L��(1)�^����(0)
// 							wk_stat = 9;										// �ُ퉞��
// 							wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
// 							Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// �����ʐM�I��
// 							CneterComLimitTime = 0;
// 						}
// 						break;
//
// 					case	R_CENTER_TRAFFIC_RESULT:				// �����ʐM�J�n���� ��M
// 						p_rcv_data = (R_CENTER_TRAFFIC_RESULT_DATA *)&Edy_Rec.rcv_data;	// ��M�ް��߲��get
// 						if(p_rcv_data->Rcv_Status[0] == 0x00) {		// ���퉞����
// 							switch(p_rcv_data->Status) {
//
// 								case 0x01:							// �J�n�w����t ����
// 									if(wk_stat == 1) {								// �J�n�w�����̂ݎ�t
// 										wk_stat = 2;								// �J�n��t��
// 										wmonlg( OPMON_EDY_M_CEN_START, 0, 0 );		// ���LOG�o�^�i�Z���^�[�ʐM�J�n�j
// 									}
// 									break;
//
// 								case 0x02:							// �ʐM����
// 									if(wk_stat == 2) {								// �J�n��t�ώ��̂ݎ�t
// 										grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);		/* �\���s�i3�j���N���A */
// 										grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);	/* "�@�@�d�����Z���^�[�ʐM�I���@�@" */
// 										grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[12]);	/* "�@�@�@�@�y����I���z�@�@�@�@�@" */
// 										Fun_Dsp(FUNMSG[8]);							/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// 										EndKey_stat = 1;							// F5�L�[�L��(1)�^����(0)
// 										wk_stat = 3;								// �ʐM�I��
// 										wmonlg( OPMON_EDY_CEN_STOP, 0, 1 );			// ���LOG�o�^�i�Z���^�[�ʐM�I��:OK�j
// 										Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;// �����ʐM�I��
// 										CneterComLimitTime = 0;
// 									}
// 									break;
//
// 								default:
// 									break;
// 							}
// 						} else {									// �ُ퉞����
// 							if(wk_stat >= 1) {										// �J�n�w���ȍ~�̂ݎ�t
// 								grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);				/* �\���s�i3�j���N���A */
// 								grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[11]);			/* "�@�@�d�����Z���^�[�ʐM�I���@�@" */
// 								grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[13]);			/* "�@�@�@�@�y�ُ�I���z�@�@�@�@�@" */
// 								Fun_Dsp(FUNMSG[8]);									/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// 								EndKey_stat = 1;									// F5�L�[�L��(1)�^����(0)
// 								wk_stat = 9;										// �ُ퉞��
// 								wmonlg( OPMON_EDY_CEN_STOP, 0, 0 );					// ���LOG�o�^�i�Z���^�[�ʐM�I��:NG�j
// 								Edy_Rec.edy_status.BIT.CENTER_COM_START = 0;		// �����ʐM�I��
// 								CneterComLimitTime = 0;
// 							}
// 						}
// 						break;
//
// 					case	R_CENTER_TRAFFIC_CHANGE:				// �����ʐM�󋵕ω��ʒm
// 						p_rcv_center = (R_CENTER_TRAFFIC_CHANGE_DATA *)&Edy_Rec.rcv_data;	// ��M�ް��߲��get
// 						if(p_rcv_center->Rcv_Status[0] == 0x00) {
// 							switch(p_rcv_center->Status_code) {
// 								case 0x01:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[19]);		/* "�@�@�@�y�f�[�^�O�������z�@�@�@"*/
// 									break;
// 								case 0x02:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[10]);		/* "�@�@�@�y�R�l�N�V�������z�@�@�@"*/
// 									break;
// 								case 0x03:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[20]);		/* "�@�@�@�y�[���h�c���o���z�@�@�@"*/
// 									break;
// 								case 0x04:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[21]);		/* "�@�@�@�y�[���F�ؑ��o���z�@�@�@"*/
// 									break;
// 								case 0x05:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[22]);		/* "�@�@�@�y�Z�b�V�����J�n�z�@�@�@"*/
// 									break;
// 								case 0x06:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[23]);		/* "�@�@�@�y�f�[�^�W�M���z  �@�@�@"*/
// 									break;
// 								case 0x07:
// 								case 0x10:
// 								case 0x11:
// 								case 0x12:
// 								case 0x13:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[24]);		/* "�@�@�@�y�f�[�^�z�M���z  �@�@�@"*/
// 									break;
// 								case 0x08:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[25]);		/* "�@�@�@�y�Z�b�V�����I���z�@�@�@"*/
// 									break;
// 								case 0x09:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[26]);		/* "�@�@�@�y�N���[�Y�������z�@�@�@"*/
// 									break;
// 								case 0x0A:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[27]);		/* "�@�@�@�y�f�[�^�㏈�����z�@�@�@"*/
// 									break;
// 								case 0x0B:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[28]);		/* "�@�@�@�y�c�g�b�o�������z�@�@�@"*/
// 									break;
// 								default:
// 									grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[0]);			/* "�@�@�@�@              �@�@�@�@"*/
// 									break;
// 							}
// 						}
// 						break;
//
// 					default:
// 						break;
// 					}
// 					break;
//
// 			case TIMEOUT7:											/* �����_�� */
// 				if(wk_stat == 2) {									// �ʐM���͓_�ŕ\�������{����
// 					rev ^= 1;
// 					grachr(2, 0, 30, rev, COLOR_BLACK, LCD_BLINK_OFF, EDY_FSTCON_STR[8]);		/* "�@�@�d�����Z���^�[�ƒʐM���@�@" */
// 					Lagtim(OPETCBNO, 7, 50);						// �����_�ŗp�^�C�}�[���X�^�[�g
// 				}
// 				break;
//
// 			default:
// 				break;
// 		}
// 	}
// }
//
// /*[]----------------------------------------------------------------------[]*/
// /*|  ���[�U�[�����e�i���X�F�g���@�\ - �d���������i�R�D�d�������ߋL�^���j |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_ShimePrint( void )	                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_ShimePrint( void )
// {
// 	ushort	usSysEvent;
// 	ushort	NewOldDate[6];				// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
// 	ushort	LogCount;					// LOG�o�^����
// 	date_time_rec	NewestDateTime, OldestDateTime;
//
//  	// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
// 	Ope2_Log_NewestOldestDateGet( eLOG_EDYSHIME, &NewestDateTime, &OldestDateTime, &LogCount );
//
// 	// �ŌÁ��ŐV���t�ް��擾
// 	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
//
// 	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
// 	usSysEvent = SysMnt_Log_CntDsp2	(
// 										LOG_EDYSHIME,		// ���O���
// 										LogCount,			// ���O����
// 										UMSTR3[109],		// �^�C�g���\���f�[�^�|�C���^
// 										PREQ_EDY_SHIME_LOG,	// �󎚗v��ү���޺����
// 										&NewOldDate[0]		// �ŌÁ��ŐV�ް����t�ް��߲��
// 									);
// 	return usSysEvent;
//
// }
// #endif
//
// #if (4 == AUTO_PAYMENT_PROGRAM)						// �����p��EM�ֺ���ޑ��M�idebug�����j
// /*[]----------------------------------------------------------------------[]*/
// /*|  ���[�U�[�����e�i���X�F�g���@�\ - �d���������i�S�D�d�����M�e�X�g  �j   |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| MODULE NAME  : UsMnt_Edy_Test( void )   	                           |*/
// /*| PARAMETER    : void                                                    |*/
// /*| RETURN VALUE : unsigned short                                          |*/
// /*[]----------------------------------------------------------------------[]*/
// /*| Update       :                                                         |*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// ushort	UsMnt_Edy_Test( void )
// {
// 	ushort	msg;
// 	short	input=0;
// 	ulong	input_l;
// 	ushort	input_work=0;
// 	uchar	wk_stat=0;
// 	uchar	led = 0;
// 	union{
// 		ushort	us;
// 		uchar	uc[2];
// 	}u;
// 	const unsigned char	EDY_TEST_STR[][31] = {
// 		"�@�@�@�@              �@�@�@�@", //[00]
// 		"���d�����M�e�X�g���@�@�@�@�@�@", //[01]
// 		"�@���M�d���ԍ��F�@�@�@�@�@�@�@", //[02]
// 		"�@�@�@�@�@���M���I�@�@�@�@�@�@", //[02]
// 		"�@�@�@�@�@��M�����@�@�@�@�@�@", //[02]
// 	};
// 	/* Edy���p�\��Ԃ����`�F�b�N���� */
//
// 	dispclr();								// ��ʃN���A
// 	if( EDY_USE_ERR ) {						// EdyӼޭ�ق̴װ�����������Ă��Ȃ������������������Ă��邩�H
// 		BUZPIPI();
// 		return MOD_EXT;										
// 	}
//
// 	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[1]);	/* "���d�����M�e�X�g���@�@�@�@�@�@" */
// 	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[2]);	/* �@���M�d���ԍ��F�@�@�@�@�@�@�@ */
// 	opedsp(2, 16, 0, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
// 	Fun_Dsp(FUNMSG2[1]);
//
// 	for ( ; ; ) {
//
// 		msg = StoF( GetMessage(), 1 );
//
//
// 		switch( KEY_TEN0to9(msg) ){			
//
// 			case KEY_MODECHG:
// 				if(wk_stat != 0) break;		/* �d�����M���͖��� */
// 				Edy_StopAndLedOff();
// 				return MOD_CHG;
//
// 			case KEY_TEN:					/* ����(�e���L�[) */
// 				BUZPI();
// 				if (input == -1){			/* ���M�ς݂̏ꍇ */
// 					input = 0;				/* ����(�e���L�[)�N���A */
// 					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[0]);	/* "�@�@�@�@              �@�@�@�@" */
//
// 				}
// 				input_l = input;
// 				input = (ushort)((input_l*10 + msg-KEY_TEN0) % 10000);
// 				opedsp(2, 16, (ushort)(input), 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �d���ԍ��\�� */
// 				break;
//
// 			case KEY_TEN_F4:								/* "�����݁i�d�����M�j" */
// 				if( input == -1 )							/* �d�����M�ς� */
// 					input = input_work;						/* �����ē����d���𑗐M����ꍇ�A�ێ����Ă����l��� */
// 				else
// 					input_work = input;						/* ���񑗐M����d����ێ� */
//
// 				wk_stat = 1;								/* ���M���׸�ON */
// 				grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[3]);		/* "�@�@�@�@�@���M���I�@�@�@�@�@�@" */
// 				BUZPI();
// 				u.uc[0] = binbcd( (uchar)(input/100) );
// 				u.uc[1] = binbcd( (uchar)(input%100) );
// 				switch( u.us ){								/* ���͒l�ɂ��d���𑗐M���� */
// 					case	S_CARD_DETECTION_START:
// 						Edy_SndData01();
// 						break;
// 					case	S_SUBTRACTION:
// 						Edy_SndData02( (ulong)CPrmSS[1][5] );
// 						break;
// 					case	S_CARD_DETECTION_STOP:
// 						Edy_SndData04();
// 						break;
// 					case	S_LAST_PAY_READ:
// 						Edy_SndData05();
// 						break;
// 					case	S_CARD_STATUS_READ:
// 						Edy_SndData06();
// 						break;
// 					case	S_TAMPER_STATUS_READ:
// 						Edy_SndData07();
// 						break;
// 					case	S_EMSET_STATUS_READ:
// 						Edy_SndData10();
// 						break;
// 					case	S_EM_STATUS_READ:
// 						Edy_SndData11();
// 						break;
// 					case	S_CENTER_TRAFFIC_START:
// 						Edy_SndData13(0);
// 						break;
// 					case	S_FIRST_STATUS:
// 						Edy_SndData14();
// 						break;
// 					case	S_LED_CTRL:
// 						switch( led ){
// 							case	0:
// 								Edy_SndData15(1,0,0);
// 								led++;
// 								break;
// 							case	1:
// 								Edy_SndData15(0,1,0);
// 								led++;
// 								break;
// 							case	2:
// 								Edy_SndData15(0,0,1);
// 								led++;
// 								break;
// 							case	3:
// 								Edy_SndData15(1,1,0);
// 								led++;
// 								break;
// 							case	4:
// 								Edy_SndData15(0,1,1);
// 								led++;
// 								break;
// 							case	5:
// 								Edy_SndData15(1,0,1);
// 								led++;
// 								break;
// 							case	6:
// 								Edy_SndData15(1,1,1);
// 								led++;
// 								break;
// 							case	7:
// 								Edy_SndData15(2,0,0);
// 								led++;
// 								break;
// 							case	8:
// 								Edy_SndData15(0,2,0);
// 								led++;
// 								break;
// 							case	9:
// 								Edy_SndData15(0,0,2);
// 								led++;
// 								break;
// 							case	10:
// 								Edy_SndData15(2,2,2);
// 								led++;
// 								break;
// 							case	11:
// 								Edy_SndData15(0,0,0);
// 								led = 0;
// 								break;
// 						}
// 						break;
// 					case	S_DATE_SYN:
// 						Edy_SndData16();
// 						break;
// 					case	S_EM_CLOSE:
// 						if( prm_get(COM_PRM,S_SCA,56,1,3) == 1){
// 							Edy_Rec.edy_status.BIT.CLOSE_MODE_END = 0;
// 							Edy_Rec.edy_status.BIT.CLOSE_MODE = 1;
// 							Edy_SndData18();
// 						}
// 						break;
// 				}
// 				break;
// 			case KEY_TEN_F5:													/* "�I��" */
// 				BUZPI();
// 				Edy_StopAndLedOff();											/* Edy��ҋ@��Ԃɖ߂� */
// 				return MOD_EXT;
//
// 			case KEY_TEN_CL:													/* ���(�e���L�[) */
// 				opedsp(2, 16, 0, 4, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �d���ԍ��\�� */
// 				input = 0;
// 				break;
//
// 			case IBK_EDY_RCV:										/* EDY ��M����� ��M */
// 				switch( Edy_Rec.rcv_kind ){
// 					case	R_CENTER_TRAFFIC_RESULT:
// 						if( ((R_CENTER_TRAFFIC_RESULT_DATA*)(Edy_Rec.rcv_data))->Status == 2 )
// 							break;
// 					case	R_CENTER_TRAFFIC_CHANGE:
// 					case	R_ERR_CHANGE:
// 					continue;
// 				}
// 				grachr(4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, EDY_TEST_STR[4]);				/* "�@�@�@�@�@��M�����@�@�@�@�@�@" */
//  				input = -1;											
// 				wk_stat = 0;										/* ���M���׸�OFF */	
// 			default:	
// 				break;
// 		}
// 	}
// }
// #endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)


/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���� �n���I�����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PreAreaSelect( char type )                        |*/
/*| PARAMETER    : char type : MNT_FLCTL:���b�N���u�J��                    |*/
/*|                          : MNT_FLSTS:�Ԏ����                          |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PreAreaSelect(short kind)
{
	unsigned short ret;

	DP_CP[0] = DP_CP[1] = 0;

	ret = 0;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	while (ret != MOD_CHG) {
	while (ret != MOD_CHG && ret != MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		dispclr();

		switch (kind) {
		case MNT_FLCTL:
			/* ���b�N���u�J�� */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[9]);		/* "�����b�N���u�J���@�@�@�@�@�@" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_FLSTS:
			/* �Ԏ���� */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[2]);		/* "���Ԏ���񁄁@�@�@�@�@�@�@�@�@" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_CARFAIL:
			/* �Ԏ��̏� */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[38]);		/* "���Ԏ��̏၄�@�@�@�@�@�@�@�@�@" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		case MNT_IOLOG:
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[127]);		/* "�����o�ɏ��v�����g���@�@�@�@�@�@�@�@�@" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		default:	// BKRS_FLSTS
			/* �Ԏ���񒲐� */
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[64]);		/* "���Ԏ���񒲐����@�@�@�@�@�@�@" */
			ret = Menu_Slt( (void*)USM_ExtendMENU, (void*)USM_ExtendMENU_TBL, (char)Ext_Menu_Max, (char)1 );
			break;
		}
		gCurSrtPara = ret;
		
		if (ret == MOD_EXT) {
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		else if (ret != MOD_CHG) {
		else if (ret != MOD_CHG || ret != MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			switch( gCurSrtPara ){
				case	MNT_CAR:		// ����
				case	MNT_FLAPUD:		// �t���b�v�㏸���~�i�ʁj
				case	MNT_FLAPUD_ALL:	// �t���b�v�㏸���~�i�S�āj
				case	MNT_INT_CAR:	// ����(����)
					ret = (ushort)GetCarInfoParam();
					switch( ret&0x06 ){
						case	0x04:
							DispAdjuster = INT_CAR_START_INDEX;
							break;
						case	0x02:
						case	0x06:
						default:
							DispAdjuster = CAR_START_INDEX;
							break;
					}
					break;

				case	MNT_BIK:		// ����
				case	MNT_BIKLCK:		// ���փ��b�N�J�i�ʁj
				case	MNT_BIKLCK_ALL:	// ���փ��b�N�J�i�S�āj
					DispAdjuster = BIKE_START_INDEX;
					break;

				default:
					DispAdjuster = BIKE_START_INDEX;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}
			ret = UsMnt_AreaSelect(kind);
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ïؔԍ������o��                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_PwdKyo( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_PwdKyo(void)
{
	ushort	msg;
	ushort	data;
	ushort	start_pos;
	
	data = (ushort)prm_get(COM_PRM,S_PAY,41,1,1);
	if (data == 0) {
		// �Ïؔԍ������o�ɂȂ�
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();
	start_pos = (data == 1) ? 1 : (ushort)(CLK_REC.mont % 10);
														/* �P�F�J�n���P�Œ�^�Q�F���݂̌����ꌅ     */
	if (start_pos == 0) start_pos = 10;					/* �O�̏ꍇ�P�O���ڂ���Q��                 */

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[70]);					/* "���Ïؔԍ������o�Ɂ��@�@�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[71]);					/* "���݂̗����\�@�J�n���ʒu�́@�@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[72]);					/* "�@�@�@�@�@�@�@�@�@���ځ@�ł��B" */
	if (start_pos < 10)
		opedsp(3, 16, start_pos, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �J�n��(1��)                      */
	else
		opedsp(3, 14, start_pos, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* �J�n��(2��)                      */

	Fun_Dsp(FUNMSG[8]);																	/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (msg == KEY_MODECHG) {
		if (msg == KEY_MODECHG || msg == MOD_CUT) {	
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			BUZPI();
			return MOD_CHG;
		}
		else if (msg == KEY_TEN_F5) {
			BUZPI();
			return MOD_EXT;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���ʐؑ֎���                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_VolSwTime( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	UsMnt_VolSwTime(void)
{
	ushort	msg;
	char	changing;	/* 1:�����ݒ蒆 */
	char	pos;		/* 0:�J�n���� 1:�I������ */
	long	input;
	long	*ptim;

	ptim = &CPrmSS[S_SYS][42];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);			/* "�����ʐؑ֎������@�@�@�@�@�@�@" */
	grachr(2, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[6]);				/* "�J�n����                        */
	bsytime_dsp(0, ptim[0], 0);					/* ���� ���]                        */
	grachr(3, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[7]);				/* "�I������"                       */
	bsytime_dsp(1, ptim[1], 0);					/* ���� ���]                        */
	Fun_Dsp( FUNMSG2[0] );						/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */

	changing = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I��(F5) */
			BUZPI();
			if (changing == 0)
				return MOD_EXT;
			bsytime_dsp(pos, ptim[pos], 0);		/* ���� ���] */
			Fun_Dsp( FUNMSG2[0] );				/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
			changing = 0;
			break;
		case KEY_TEN_F3:	/* �ύX(F3) */
			if (changing == 1)
				break;
			BUZPI();
			bsytime_dsp(0, ptim[0], 1);		/* ���� ���] */
			Fun_Dsp( FUNMSG[20] );			/* "�@���@�@���@�@�@�@ ����  �I�� " */
			input = -1;
			changing = 1;
			pos = 0;
			break;
		case KEY_TEN_F4:	/* ����(F4) */
			if (changing == 0)
				break;
			if (input != -1) {
				if (input%100 > 59 || input > 2359) {
					BUZPIPI();
					bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
					input = -1;
					break;
				}
				ptim[pos] = input;
				OpelogNo = OPLOG_VLSWTIME;			// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;			// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			}
		case KEY_TEN_F1:	/* ��(F1) */
		case KEY_TEN_F2:	/* ��(F2) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 0);		/* ���� ���] */
			pos ^= 1;
			bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
			input = -1;
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			if (input == -1)
				input = 0;
			input = (input*10 + msg-KEY_TEN0) % 10000;
			bsytime_dsp(pos, input, 1);			/* ���� ���] */
			break;
		case KEY_TEN_CL:	/* ���(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			bsytime_dsp(pos, ptim[pos], 1);		/* ���� ���] */
			input = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F��Ԋm�F                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_StatusView( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const unsigned char	Det_Sts[][7] = {		// �uDET��ԁv�\���ް�
	"�n�m�@",	// [00]
	"�n�e�e",	// [01]
};

const unsigned char	Ari_Nasi[][5] = {		// �u�G���[�v�u�A���[���v�L���\���ް�
	"����",		// [00]
	"�Ȃ�",		// [01]
};

const unsigned char	Eigyou_Mode1[][5] = {	// �u�c�x�Ɓv�\���ް�1
	"�c��",	// [00]
	"�x��",	// [01]
};

const unsigned char	Eigyou_Mode2[][13] = {	// �u�c�x�Ɓv�\���ް�2
	"�i�����j�@�@",	// [00]
	"�i�����j�@�@",	// [01]
	"�i�ʐM�j�@�@",	// [02]
	"�i�M���j�@�@",	// [03]
	"�i�g���u���j",	// [04]
	"�i���ɖ��t�j",	// [05]
	"�i�ސ؂�j�@",	// [06]
	"�i�������j�@",	// [07]
	"�i�蓮�j�@�@",	// [08]
// MH810105(S) MH364301 �C���{�C�X�Ή�
	"�i���؂�j�@",	// [09]
// MH810105(E) MH364301 �C���{�C�X�Ή�
};

const unsigned char	Mankusha_Sts1[][5] = {	// �u����ԁv�\���ް�1
	"����",	// [00]
	"���",	// [01]
};

const unsigned char	Mankusha_Sts2[][9] = {	// �u����ԁv�\���ް�2
	"�i�����j",	// [00]
	"�i�����j",	// [01]
	"�i�ʐM�j",	// [02]
};

unsigned short	UsMnt_StatusView(void)
{
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	long i;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
	ushort	msg;
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	char factor;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	dispclr();												// ��ʃN���A

	// �P�s�ځF"����Ԋm�F���@�@�@�����F�����@"
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	grachr( 0, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"����Ԋm�F��");			// "����Ԋm�F��"	
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:290)�Ή�
//	grachr( 0, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"��Ԋm�F�@");			// "�@��Ԋm�F��"
	grachr( 0, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"��Ԋm�F�@");			// "�@��Ԋm�F��"
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:290)�Ή�
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)	

	// �Q�s�ځF"�@�G���[�@�@�F�w�w�@�@�@�@�@"
	grachr( 1, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"�G���[�@�@�F");
	if( Err_onf == 0 ){																		// �G���[�����L���\��
		grachr( 1, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[1] );					// �Ȃ��\��
	}
	else{
		grachr( 1, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[0] );					// ����\��
	}
	// �R�s�ځF"�@�A���[���@�@�F�w�w�@�@�@�@�@"
	grachr( 2, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"�A���[���@�F");
	if( Alm_onf == 0 ){																		// �A���[�������L���\��
		grachr( 2, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[1] );					// �Ȃ��\��
	}
	else{
		grachr( 2, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Ari_Nasi[0] );					// ����\��
	}

	// �S�s�ځF"�@�c�x�Ɓ@�@�@�F�w�w�w�w�@�@�@"
	grachr( 3, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"�c�x�Ɓ@�@�F");
	// �c��or�x��
	if (opncls() == 1) {
		// �c�ƒ�
		grachr(3, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode1[0]);	// �c��
		// �c�Ɨ��R
		if (OPECTL.opncls_eigyo == 1) {
			// �O���M�����͂̉c�ƐM������
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[3]);	// �i�M���j
		} else if (PPrmSS[S_P01][2] == 1) {
			// հ�ް����ݽ�ɂ�鋭���c��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[1]);	// �i�����j
		} else if (OPEN_stat == OPEN_NTNET) {
			// NT-NET�����ް��ɂ��c�Ǝw��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[2]);	// �i�ʐM�j
		} else {
			// �c�Ǝ��Ԓ�
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[0]);	// �i�����j
		}
	} else {
		// �x�ƒ�
		grachr(3, 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode1[1]);	// �x��
		// �x�Ɨ��R
		switch (CLOSE_stat) {	// opncls�֐���ق��邱�Ƃ�CLOSE_stat���X�V�����
		case 1:			// հ�ް����ݽ�ɂ�鋭���x��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[1]);	// �i�����j
			break;
		case 2:			// �c�Ǝ��ԊO
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[0]);	// �i�����j
			break;
		case 3:			// �������ɖ��t
		case 4:			// ��݋��ɖ��t
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[5]);	// �i���ɖ��t�j
			break;
		case 6:			// �ޑK�؂�
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[6]);	// �i�ސ؂�j
			break;
		case 7:			// �����
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[4]);	// �i�g���u���j
			break;
		case 8:			// NT-NET ���M�ޯ̧FULL
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[7]);	// �i�������j
			break;
		case 9:			// �ׯ��/ۯ����u�蓮Ӱ��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[8]);	// �i�蓮�j
			break;
		case 10:		// �O���M�����͂̋x�ƐM��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[3]);	// �i�M���j
			break;
		case 11:		// NT-NET�����ް��ɂ��x�Ǝw��
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[2]);	// �i�ʐM�j
			break;
// MH810105(S) MH364301 �C���{�C�X�Ή�
		case 15:		// ���޲��ݒ莞��������װ
			grachr(3, 18, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, Eigyou_Mode2[9]);	// �i���؂�j
			break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
		default:		// ���̑�
			break;
		}
	}

// MH810100(S) K.Onodera 2019/10/22 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	// �T�`�V�s�ځF"�@����ԁ@�@�F�w�w�w�w�@�@�@"
//	//			   "�@����ԂP�@�F�w�w�w�w�@�@�@"	����ԂP�`�R�܂�
//		// ����1�A2�A3
//		grachr( 4, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"����ԂP�@�F");
//		grachr( 5, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"����ԂQ�@�F");
//		grachr( 6, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"����ԂR�@�F");
//
//	// �ް��\��
//	for (i = 0; i < 3; i++) {
//		// ��Ԏ擾
//		factor = getFullFactor((uchar)i);
//		// ����/��ԕ\��
//		if (factor & 0x10) {
//			grachr( (ushort)(4+i), 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts1[0]);	// ����
//		} else {
//			grachr( (ushort)(4+i), 14, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts1[1]);	// ���
//		}
//		// ���R�\��
//		if (factor & 0x01) {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[1]);	// �i�����j
//		} else if (factor & 0x02) {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[2]);	// �i�ʐM�j
//		} else {
//			grachr( (ushort)(4+i), 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, Mankusha_Sts2[0]);	// �i�����j
//		}
//	}
// MH810100(E) K.Onodera 2019/10/22 �Ԕԃ`�P�b�g���X(�����e�i���X)

	// �t�@���N�V�����L�[�\��
	Fun_Dsp( FUNMSG[8] );				// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		
		else if (msg == KEY_TEN_F5) {
			BUZPI();
			return MOD_EXT;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���b�N���u�^�C�}�[�g��                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_LockTimerEx( void )                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	KIND_CHK( a,b )	(a&(b==2?0x02:0x01))
unsigned short	UsMnt_LockTimerEx(void)
{
	ushort	msg;
	long	*ptime[2];
	char	set;
	long	data;
	long	tmp;

	unsigned short	line = 0;	//1�F�t���b�v�I�𒆁A2�F���փ��b�N�I��
	char	syu[2];
	uchar	t_type = 0;

	memset( syu, 0, sizeof( syu ));
	if( !Get_Pram_Syubet(FLAP_UP_TIMER) ){
		ptime[0] = &CPrmSS[S_TYP][118];		// �t���b�v�㏸�^�C�}�[
	}else{
		ptime[0] = &CPrmSS[S_LTM][31];		// �t���b�v�㏸�^�C�}�[(�`��)
		t_type = 0x01;
		syu[0] = 1;
	}

	if( !Get_Pram_Syubet(ROCK_CLOSE_TIMER) ){
		ptime[1] = &CPrmSS[S_TYP][69];		// ���b�N�^�C�}�[
	}else{
		ptime[1] = &CPrmSS[S_LTM][11];		// ���b�N�^�C�}�[(�`��)
		t_type |= 0x02;
		syu[1] = 1;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[24]);					/* "�����b�N���u�^�C�}�[���@�@�@" */
	grachr(3,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[27]);					/* "�t���b�v�@" or "���փ��b�N" */
	grachr(3, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "�F" */
	if( t_type & 0x01 ){
		grachr(3, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);			/* ��� */
	}
	opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "��" */
	opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
	grachr(3, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "�b" */

	grachr(4,  0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[28]);					/* "�t���b�v�@" or "���փ��b�N" */
	grachr(4, 10, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);						/* "�F" */
	if( t_type & 0x02 ){
		grachr(4, 12, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);				/* ��� */
	}
	opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[11]);					/* "��" */
	opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
	grachr(4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[12]);					/* "�b" */

	Fun_Dsp(FUNMSG2[0]);						/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */

	set = 0;
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (msg == KEY_MODECHG) {
			BUZPI();
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (msg == LCD_DISCONNECT) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (set == 0) {
			switch (msg) {
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F3:	/* �ύX(F3) */
				BUZPI();
				if( t_type & 0x01 ){
					grachr(3, 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);					/* ��� ���]*/
					Fun_Dsp(FUNMSG[20]);											/* "�@���@�@���@�@�@�@ ����  �I�� " */
					set = KIND;
				}else{
					opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);		/* �� ���] */
					Fun_Dsp(FUNMSG[20]);											/* "�@���@�@���@�@�@�@ ����  �I�� " */
					set = MIN;
				}
				line = 1;
				data = -1;
				break;
			default:
				break;
			}
		}
		else {
			switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F1:	// �� F1
			case KEY_TEN_F2:	// �� F2
				if( set == KIND ){
					BUZPI();
					if( KEY_TEN0to9(msg) == KEY_TEN_F1 ){
						syu[line-1]--;
						ptime[line-1]--;
						if( syu[line-1] < 1 ){
							syu[line-1] = 12;
							ptime[line-1] += 12;
						}
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);				/* �`�� */
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
						opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					}else{
						syu[line-1]++;
						ptime[line-1]++;
						if( syu[line-1] > 12 ){
							syu[line-1] = 1;
							ptime[line-1] -= 12;
						}
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);				/* �`�� */
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
						opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					}
				}else{
				// �C���f���g�����킹��ׂɃ^�u��ǉ�
					BUZPI();
					if( t_type & 0x01 ){
						grachr(3, 12, 2, (line==1?0:1), COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);				/* ��� */
						opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* ��i �� ���] */
					}else{
						opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, (line==1?0:1), COLOR_BLACK, LCD_BLINK_OFF);	/* ��i �� ���] */
					}
					opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ��i �b ���] */
					
					if( t_type & 0x02 ){
						grachr(4, 12, 2, (line==2?0:1), COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);				/* ��� */
						opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);				/* ���i �� ���] */
					}else{
						opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, (line==2?0:1), COLOR_BLACK, LCD_BLINK_OFF);	/* ���i �� ���] */
					}
					opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);					/* ���i �b ���] */
					line = (line == 1?2:1);
					data = -1;		//��������Ă��Ȃ����͒l�̓N���A
					set = (KIND_CHK(t_type,line) ? KIND:MIN);
				}
				break;
			case KEY_TEN_F5:
				BUZPI();
				if( t_type & 0x01 ){
					grachr(3, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[0]]);	/* ��� */
				}
				if( t_type & 0x02 ){
					grachr(4, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[1]]);	/* ��� */
				}
				opedsp(3, 16, (ushort)GET_MIN(*ptime[0]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
				opedsp(3, 24, (ushort)GET_SEC(*ptime[0]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */
				opedsp(4, 16, (ushort)GET_MIN(*ptime[1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
				opedsp(4, 24, (ushort)GET_SEC(*ptime[1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b */

				Fun_Dsp(FUNMSG2[0]);				/* "�@�@�@�@�@�@ �ύX�@�@�@�@�I�� " */
				set = IDLE;
				break;
			case KEY_TEN_F4:
				if( set == KIND ){
					grachr((ushort)(line+2), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);		/* ��� ���] */
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					set = MIN;
					data = -1;
				}else if( set == MIN ){
					if (data != -1) {
						tmp = *ptime[line-1]/100000L*100000L + data*100 + *ptime[line-1]%100;
						if ((tmp%100000L) > 72000) {
							BUZPIPI();
							opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
							data = -1;
							break;
						}
						*ptime[line-1] = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					data = -1;
					set = SEC;
				}
				else {
					if (data != -1) {
						tmp = *ptime[line-1]/100*100 + data;
						if ((tmp%100000L) > 72000 || data > 59) {
							BUZPIPI();
							opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
							data = -1;
							break;
						}
						*ptime[line-1] = tmp;
						OpelogNo = OPLOG_ROCKTIMER;		// ���엚��o�^
						f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
						mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
					}
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
					line = (line == 1?2:1);
					if( KIND_CHK(t_type,line) ){
						grachr((ushort)(line+2), 12, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT2_7[syu[line-1]]);							/* ��� ���]*/
					}else{
						opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);				/* �� ���] */
					}
					data = -1;
					set = (KIND_CHK(t_type,line) ? KIND:MIN);
				}
				BUZPI();
				break;
			case KEY_TEN:
				if (data == -1)
					data = 0;
				if (set == MIN) {
					data = (data*10 + msg-KEY_TEN0) % 1000;
					opedsp((ushort)(line+2), 16, (ushort)data, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
				}
				else if(set == SEC){
					data = (data*10 + msg-KEY_TEN0) % 100;
					opedsp((ushort)(line+2), 24, (ushort)data, 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}else{	// set == KIND
					break;
				}
				BUZPI();
				break;
			case KEY_TEN_CL:
				if (set == MIN){
					opedsp((ushort)(line+2), 16, (ushort)GET_MIN(*ptime[line-1]), 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �� ���] */
				}else if(set == SEC){
					opedsp((ushort)(line+2), 24, (ushort)GET_SEC(*ptime[line-1]), 2, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);	/* �b ���] */
				}else{// set == KIND
					break;
				}
				BUZPI();
				data = -1;
				break;
			default:
				break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F���p���׃��O�󎚉�ʂւ̒��p����
//[]----------------------------------------------------------------------[]
///	@param[in]		MsgId			MNT_EDY_MEISAI : �d�������p����
///									MNT_SCA_MEISAI : �r�����������p����
///									MNT_EC_MEISAI  : ���σ��[�_���p����
///	@return			usSysEvent		MOD_CHG : mode change<br>
/// 								MOD_EXT : F5 key
///	@author			Y.Ise
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_DiditalCasheUseLog(ushort MsgId)
{
	ushort	usSysEvent;
	ushort	PrintNum;										// ����v�����̃��b�Z�[�W�ԍ�
	ushort	LogKind;										// �g�p���郍�O���
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	uchar	titleNum = 110;
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���

	switch(MsgId)											// �J�[�h��ʖ��ɕ\���^�C�g���ƃ��b�Z�[�W�ԍ�������
	{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	case MNT_EDY_MEISAI:									// �d����
//		LogKind = LOG_EDYMEISAI;							// ���O���
//		PrintNum = PREQ_EDY_USE_LOG;						// ����v�����̃��b�Z�[�W�ԍ�
//		break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case MNT_SCA_MEISAI:									// �r��������
	default:												// �J�[�h��ʂȂ�(�ُ�)
		LogKind = LOG_SCAMEISAI;							// ���O���
		PrintNum = PREQ_SCA_USE_LOG;						// ����v�����̃��b�Z�[�W�ԍ�
		break;

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�p�̖��׈󎚐ݒ�)
	case MNT_ECR_MEISAI:									// ���σ��[�_�����E����
		LogKind = LOG_ECMEISAI;								// ���O���
		PrintNum = PREQ_EC_USE_LOG;							// ����v�����̃��b�Z�[�W�ԍ�
		break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�p�̖��׈󎚐ݒ�)
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	case MNT_ECR_MINASHI:									// �݂Ȃ����Ϗ����E����
		LogKind = LOG_ECMINASHI;							// ���O���
		PrintNum = PREQ_EC_MINASHI_LOG;						// ����v�����̃��b�Z�[�W�ԍ�
		titleNum = 186;
		break;
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	}

	dispclr();
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[110]);						// ���O���v�����g�^�C�g���\��
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[titleNum]);					// ���O���v�����g�^�C�g���\��
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	grachr(2, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[113]);				/* "�@�@�@ ���O�̌������ł� �@�@�@" */
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[156]);				/* "�ő��R�O�b�����鎖������܂�" */
	grachr(5, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_ON, UMSTR3[114]);				/* "�@�@ ���΂炭���҂������� �@�@" */

	//	���O�����\����ʏ����S�i�Ώۊ��Ԏw�肠��^�������Ō�������������j
	usSysEvent = SysMnt_Log_CntDsp3	(
										LogKind,			// ���O���
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//										UMSTR3[110],		// �^�C�g���\���f�[�^�|�C���^
										UMSTR3[titleNum],	// �^�C�g���\���f�[�^�|�C���^
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
										PrintNum			// �󎚗v��ү���޺����
									);
	return usSysEvent;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F�����W�v���O�󎚉�ʂւ̒��p�֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		MsgId			MNT_EDY_MEISAI : �d�������p����
///									MNT_SCA_MEISAI : �r�����������p����
///									MNT_EC_SHUUKEI : ���σ��[�_�����W�v
///	@return			usSysEvent		MOD_CHG : mode change<br>
/// 								MOD_EXT : F5 key
///	@author			Y.Ise
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_DiditalCasheSyuukei(ushort MsgId)
{
extern	ushort	DiditalCashe_Log_CntDsp( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate );
	ushort	usSysEvent;
	ushort	NewOldDate[6];									// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
	ushort	LogCount;										// LOG�o�^����
	ushort	PrintNum;										// ����v�����̃��b�Z�[�W�ԍ�
	ushort	LogKind;										// �g�p���郍�O���
	date_time_rec	NewestDateTime, OldestDateTime;

	switch(MsgId)											// �J�[�h��ʖ��ɕ\���^�C�g���ƃ��b�Z�[�W�ԍ�������
	{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	case MNT_EDY_SHUUKEI:									// �d����
//		LogKind = LOG_EDYSYUUKEI;							// ���O���
//		PrintNum = PREQ_EDY_SYU_LOG;						// ����v�����̃��b�Z�[�W�ԍ�
//		break;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case MNT_SCA_SHUUKEI:									// �r��������
	default:												// �J�[�h��ʂȂ�(�ُ�)
		LogKind = LOG_SCASYUUKEI;							// ���O���
		PrintNum = PREQ_SCA_SYU_LOG;						// ����v�����̃��b�Z�[�W�ԍ�
		break;

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�p�̏W�v�󎚐ݒ�)
	case MNT_ECR_SHUUKEI:									// ���σ��[�_�����E�W�v
		LogKind = LOG_ECSYUUKEI;							// ���O���
		PrintNum = PREQ_EC_SYU_LOG;							// ����v�����̃��b�Z�[�W�ԍ�
		break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�p�̏W�v�󎚐ݒ�)

	}

	DiditalCashe_NewestOldestDateGet( LogKind, &NewestDateTime, &OldestDateTime, &LogCount );
//	// �ŌÁ��ŐV���t�ް��擾(���҂̔N�������������đ�O�����Ɋi�[)
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
	usSysEvent = DiditalCashe_Log_CntDsp(
										LogKind,			// ���O���
										LogCount,			// ���O����
										UMSTR3[111],		// �^�C�g���\���f�[�^�|�C���^
										PrintNum,			// �󎚗v��ү���޺����
										&NewOldDate[0]		// �ŌÁ��ŐV�ް����t�ް��߲��
									);
	return usSysEvent;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F���ʃp�����[�^���烁�j���[���쐬(31byte�e�[�u���p)
//[]----------------------------------------------------------------------[]
///	@param[in]		const CeMenuPara		*prm_tbl		: ���j���[�̕\�������e�[�u��
///					const unsigned char		 str_tbl[][31]	: ���j���[�̕�����e�[�u��(31�o�C�g)
///					const unsigned short	 ctr_tbl[][4]	: ���j���[�̐�����e�[�u��
///					const unsigned char		 tbl_cnt		: �������j���[�̑���
///	@return			unsigned char			ret				: �쐬�������j���[���ڐ�<br>
///	@author			Y.Ise
///	@note			���ʃp�����[�^��������ƈ�v�������ڂ݂̂Ń��j���[���쐬����B
///					�������ɕ\�����鍀�ڂ͑�������Section,Address,Assign�����o��0�ɂ���B
///	@attention		���j���[�̍쐬�ő吔��10��(USM_MAKE_MENU�f�t�@�C���ˑ�)�܂ŁB
///					�{�֐����g�p����ɂ�����A���j���[�̕�����͑S�p15����(31byte)�Œ�ł��邱�ƁB
///					���j���[�̕�����́A���ʌ��擪����2byte���폜���A�����ɔԍ���t������B
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static unsigned char UsMnt_DiditalCasheMenuMake31(
	const CeMenuPara		*prm_tbl,											/* ���j���[�̕\�������e�[�u�� */
	const unsigned char		 str_tbl[][31],										/* ���j���[�̕�����e�[�u�� */
	const unsigned short	 ctr_tbl[][4],										/* ���j���[�̐�����e�[�u�� */
	const unsigned char		 tbl_cnt											/* �������j���[�̑��� */
){
	unsigned char cnt;															/* ���[�v�p�ϐ� */
	unsigned char ret;															/* �쐬���ڐ� */

	memset(UsMnt_DiditalCasheMENU_STR, 0, sizeof(UsMnt_DiditalCasheMENU_STR));	// ���j���[���ڍ쐬�e�[�u��������
	memset(UsMnt_DiditalCasheMENU_CTL, 0, sizeof(UsMnt_DiditalCasheMENU_CTL));	// ���j���[����쐬�e�[�u��������

	for(cnt = 0, ret = 0; (cnt < tbl_cnt) && (cnt < USM_MAKE_MENU); cnt++)		// ���j���[���ڐ���������
	{
		//���j���[�̍��ڕ���������
		strcpy((char*)&UsMnt_DiditalCasheMENU_STR[cnt][0], (char*)&MENU_NUMBER[ret][0]);
		strcat((char*)&UsMnt_DiditalCasheMENU_STR[cnt][2], (char*)&str_tbl[cnt][2]);
		/* �������ݒ肳��Ă��Ȃ�(�������\��) */
		if((!prm_tbl[cnt].Section) && (!prm_tbl[cnt].Address) && (!prm_tbl[cnt].Assign))
		{
			//���j���[�̐���e�[�u�������
			memcpy(UsMnt_DiditalCasheMENU_CTL[ret], ctr_tbl[cnt], sizeof(UsMnt_DiditalCasheMENU_CTL[0]));
			ret++;																// ���j���[���ڐ��X�V
		}
		/* �w��̋��ʃp�����[�^�A�h���X����r�l�Ɠ��� */
		else if(prm_get(COM_PRM, prm_tbl[cnt].Section, prm_tbl[cnt].Address, 1, prm_tbl[cnt].Assign) == prm_tbl[cnt].Value)
		{
			//���j���[�̐���e�[�u�������
			memcpy(UsMnt_DiditalCasheMENU_CTL[ret], ctr_tbl[cnt], sizeof(UsMnt_DiditalCasheMENU_CTL[0]));
			ret++;																// ���j���[���ڐ��X�V
		}
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F�g���@�\ - Suica�����ƭ�
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			Suzuki
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2006-11-06
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
unsigned short	UsMnt_SuicaMnu(void)
{
	unsigned char	MenuValue = 0;					// ���j���[���ڂ̕\����
	unsigned short	usUcreEvent;
	char	wk[2];
	CeMenuPara make_table[] =						// ���j���[�\�������e�[�u��
	{
		{S_SCA,		7,			5,		1},			// �����W�v�v�����g
		{S_SCA,		7,			6,		1},			// ���p���׃v�����g
	};

	DP_CP[0] = DP_CP[1] = 0;
	/* �ǂ̈󎚋@�\���L���ł��邩�����ʃp�����[�^����擾���A�\���ۂ̔��f������ */
	MenuValue = UsMnt_DiditalCasheMenuMake31(make_table, EXTENDMENU_SCA, USM_EXTSCA_TBL, USM_EXTSCA_MAX);

	for( ; ; ) {

		dispclr();									// ��ʃN���A
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[112]);			// "���d�q�}�l�[�������@�@�@�@�@�@"
		usUcreEvent = Menu_Slt( (void*)UsMnt_DiditalCasheMENU_STR, (void*)UsMnt_DiditalCasheMENU_CTL, (char)MenuValue, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUcreEvent ){
			case MNT_SCA_MEISAI:					// �r�����������p����
				usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_SCA_MEISAI);
				if (usUcreEvent == MOD_CHG){
					return(MOD_CHG);
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if (usUcreEvent == MOD_CUT ){	
					return(MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			case MNT_SCA_SHUUKEI:					// �r���������W�v
				usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_SCA_SHUUKEI);
				if (usUcreEvent == MOD_CHG){
					return(MOD_CHG);
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if (usUcreEvent == MOD_CUT ){
					return(MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			case MOD_EXT:		// �I���i�e�T�j
				return(MOD_EXT);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_CUT:
				return(MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_CHG:		// ���[�h�`�F���W
				return(MOD_CHG);

			default:
				break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���[�U�[�����e�i���X�F�s�������O                					   |*
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : UsMnt_NgLog											   |*
 *| PARAMETER    : none													   |*
 *| RETURN VALUE : MOD_CHG : mode change								   |*
 *|              : MOD_EXT : F5 key										   |*
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 														   |*
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	UsMnt_NgLog(void)
{
	date_time_rec	NewTime;
	date_time_rec	OldTime;
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
	uchar			end_flg;			// �N���A�����ʏI���׸�
	ushort			Date_Old[6];		// ���t�ް��i�J�n�N[0],��[1],��[2],��[3],��[4]�A�I��[5],��[6],��[7],��[8],��[9]�j
	ushort LogSyu;
	ushort LogCnt_total;				// �����O����
	ushort LogCnt;						// �w����ԓ��Ώۃ��O����
	ushort PreqCmd;

	LogSyu = LOG_NGLOG;

	Ope2_Log_NewestOldestDateGet(eLOG_NGLOG, &NewTime, &OldTime, &LogCnt_total);
	PreqCmd = PREQ_NG_LOG;


	/* ������ʕ\�� */
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[126]);		// "���g�p�J�[�h���v�����g��    "
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[0] );		// �ŌÁ^�ŐV���O���t�\��
	Date_Now[0] = (ushort)OldTime.Year;	// �J�n�i�N�j���ŌÃ��O�i�N�j
	Date_Now[1] = (ushort)OldTime.Mon;	// �J�n�i���j���ŌÃ��O�i���j
	Date_Now[2] = (ushort)OldTime.Day;	// �J�n�i���j���ŌÃ��O�i���j
	
	Date_Now[3] = (ushort)NewTime.Year;	// �J�n�i�N�j���ŐV���O�i�N�j
	Date_Now[4] = (ushort)NewTime.Mon;	// �J�n�i���j���ŐV���O�i���j
	Date_Now[5] = (ushort)NewTime.Day;	// �J�n�i���j���ŐV���O�i���j
	memcpy(Date_Old, Date_Now, sizeof(Date_Old));		// �ύX�O���t�ް��Z�[�u
	LogDateDsp4( &Date_Now[0] );										// �ŌÁ^�ŐV���O���t�f�[�^�\��
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );		// ������t�\��
	LogDateDsp5( &Date_Now[0], pos );									// ������t�f�[�^�\��
	
	// �w����ԓ����O�����擾
	Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
	Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// �I�����t�ް��擾
	LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
	LogCntDsp( LogCnt );							// ���O�����\��
	
	Fun_Dsp( FUNMSG[83] );					// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "

	/* ��ʑ���󂯕t�� */
	for( ; ; ){
		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�

		/*  �󎚗v���O�i�󎚗v���O��ʁj*/
		if( pri_cmd == 0 ){
			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H

				/* �ݒ�L�[�ؑ� */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				/* �O�`�X */
				case KEY_TEN:
					if( All_Req == OFF ){					// �u�S�āv�w�蒆�łȂ��ꍇ�i�w�蒆�́A��������j
						BUZPI();
						if( inp == -1 ){
							inp = (short)(RcvMsg - KEY_TEN0);
						}else{
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

				/* ����L�[ */
				case KEY_TEN_CL:
					BUZPI();
					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
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
					break;

				/* �e�P�i���j�e�Q�i���j�L�[���� */
				case KEY_TEN_F1:
				case KEY_TEN_F2:
					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ�́i�@���@�@���@�L�[�F���t���́j
						if( inp == -1 ){		// ���͂Ȃ��̏ꍇ
							BUZPI();
							if( RcvMsg == KEY_TEN_F1 ){
								LogDatePosUp( &pos, 0 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
							}else{
								LogDatePosUp( &pos, 1 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
							}
						}
						else{				//	���͂���̏ꍇ
							if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j
								BUZPI();
								if( pos == 0 ){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
								}else{
									Date_Now[pos] =
									Date_Now[pos+3] = (ushort)inp;
								}
								if( RcvMsg == KEY_TEN_F1 ){
									LogDatePosUp( &pos, 0 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
								}else{
									LogDatePosUp( &pos, 1 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
								}
							}
							else{								//	�����ް��m�f
								BUZPIPI();
							}
						}
						// �w����ԓ��Ώۃ��O�������擾���\������
						LogDateDsp5( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
						inp = -1;								// ���͏�ԏ�����
					}
					else if( RcvMsg == KEY_TEN_F2 ){		// �u�S�āv�w�蒆�ꍇ�́i�e�Q�F�u�N���A�v�L�[�j
						BUZPI();
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[60]);				// "�@�@���O���N���A���܂����H�@�@"
						Fun_Dsp(FUNMSG[19]);															// "�@�@�@�@�@�@ �͂� �������@�@�@"

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
										LogCnt_total = 0;				// ���O�����i�������Ұ��j�N���A
										LogCntDsp( LogCnt_total );		// ���O�����\��
										Date_Now[0] = Date_Now[3] = (ushort)CLK_REC.year;	// �J�n�A�I���i�N�j�����ݓ����i�N�j
										Date_Now[1] = Date_Now[4] = (ushort)CLK_REC.mont;	// �J�n�A�I���i���j�����ݓ����i���j
										Date_Now[2] = Date_Now[5] = (ushort)CLK_REC.date;	// �J�n�A�I���i���j�����ݓ����i���j
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
								LogDateDsp4( &Date_Now[0] );						// �ŌÁ^�ŐV���O���t�f�[�^�\��
								grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
								LogDateDsp5( &Date_Now[0], pos );					// ������t�f�[�^�\��
								Fun_Dsp( FUNMSG[83] );								// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
								All_Req = OFF;										// �u�S�āv�w�蒆�׸�ؾ��
							}
						}
					}
					break;

				/* �e�R�i�S�āj�L�[���� */
				case KEY_TEN_F3:
					displclr( 1 );							// 1�s�ڕ\���N���A
					if( All_Req == OFF ){	// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j
						if( LogCnt_total != 0 ){		//	���O�f�[�^������ꍇ
							BUZPI();
							Date_Now[0] = (ushort)OldTime.Year;	// �J�n�i�N�j���ŌÃ��O�i�N�j
							Date_Now[1] = (ushort)OldTime.Mon;	// �J�n�i���j���ŌÃ��O�i���j
							Date_Now[2] = (ushort)OldTime.Day;	// �J�n�i���j���ŌÃ��O�i���j
							
							Date_Now[3] = (ushort)NewTime.Year;	// �J�n�i�N�j���ŐV���O�i�N�j
							Date_Now[4] = (ushort)NewTime.Mon;	// �J�n�i���j���ŐV���O�i���j
							Date_Now[5] = (ushort)NewTime.Day;	// �J�n�i���j���ŐV���O�i���j
							pos = 0;								// ���وʒu���J�n�i�N�j
							inp = -1;								// ���͏�ԏ�����
							displclr( 5 );							// ������t�\���N���A
							// �w����ԓ��Ώۃ��O�������擾���\������
							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
							Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// �I�����t�ް��擾
							LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
							LogCntDsp( LogCnt );					// ���O�����\��
							Fun_Dsp( FUNMSG[85] );					// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
							All_Req = ON;							// �u�S�āv�w�蒆�׸޾��
						}
						else{					//	���O�f�[�^���Ȃ��ꍇ
							BUZPIPI();
						}
					}
					break;

				/* �e�S�i���s�j�L�[���� */
				case KEY_TEN_F4:
					displclr( 1 );							// 1�s�ڕ\���N���A
					Date_Chk = OK;
					if( inp != -1 ){	//	���͂���̏ꍇ
						if( OK == LogDateChk( pos, inp ) ){	// �����ް��n�j�̏ꍇ
							if( pos == 0 ){					// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
							}else{
								Date_Now[pos] =
								Date_Now[pos+3] = (ushort)inp;
							}
						}else{								//	���̓f�[�^�m�f�̏ꍇ
							Date_Chk = NG;
						}
					}
					// �w����ԓ��Ώۃ��O�������擾���\������
					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
					Edate = dnrmlzm((short)Date_Now[3],	(short)Date_Now[4],	(short)Date_Now[5]);// �I�����t�ް��擾
					LogCnt = UsMnt_NgLog_GetLogCnt(Sdate, Edate);
					if( LogCnt == 0 ){
						BUZPIPI();
						grachr( 1, 0, 30, 0, COLOR_RED, LCD_BLINK_ON, LOGSTR3[4] );	// �f�[�^�Ȃ��\��
						inp = -1;							// ���͏�ԏ�����
						break;
					}
					if( Date_Chk == OK ){
						if( All_Req != ON ){	// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��
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
					if( Date_Chk == OK ){	// �J�n���t���I�����t�����n�j�̏ꍇ
						BUZPI();
						/*------	�󎚗v��ү���ޑ��M	-----*/
						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// �󎚗v��ү����ܰ��O�ر
						FrmLogPriReq2.prn_kind	= R_PRI;						// �Ώ�������Fڼ��
						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.

						if( All_Req == ON ){
							FrmLogPriReq2.BType	= 0;							// �������@	�F�S��
						}else{
							FrmLogPriReq2.BType	= 1;							// �������@	�F���t
							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
						}
						FrmLogPriReq2.LogCount = LogCnt_total;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
						queset( PRNTCBNO, PreqCmd, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
						Ope_DisableDoorKnobChime();
						pri_cmd = PreqCmd;					// ���M����޾���
						if( All_Req != ON ){					// �u�S�āv�w��Ȃ�
							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
							LogDateDsp5( &Date_Now[0], 0xff );	// ������t�f�[�^�\��
						}
						grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, LOGSTR3[3] );		// ���s���u�����N�\��
						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

					}else{	// �J�n���t���I�����t�����m�f�̏ꍇ
						BUZPIPI();
						LogDateDsp5( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
						inp = -1;								// ���͏�ԏ�����
					}
					break;

				/* �e�T�i�I���j�L�[���� */
				case KEY_TEN_F5:							// 
					BUZPI();
					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
						RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
					}else{					//	�u�S�āv�w�蒆�̏ꍇ
						pos = 0;							// ���وʒu���J�n�i�N�j
						inp = -1;							// ���͏�ԏ�����
						opedsp	(							// ���ق�\��������
									POS_DATA1_0[pos][0],	// �\���s
									POS_DATA1_0[pos][1],	// �\�����
									Date_Now[pos],			// �\���ް�
									POS_DATA1_0[pos][2],	// �ް�����
									POS_DATA1_0[pos][3],	// �O�T�v���X�L��
									1,						// ���]�\���F����
									COLOR_BLACK, 
									LCD_BLINK_OFF
								);
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR3[2] );	// ������t�\��
						LogDateDsp5( &Date_Now[0], pos );	// ������t�f�[�^�\��
						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
					}
					break;
				default:
					break;
			}
		}
		/* �󎚗v����i�󎚏I���҂���ʁj*/
		else{
			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
			}

			switch( RcvMsg ){								// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					RcvMsg = MOD_CUT;						// �O��ʂɖ߂�
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:	// �ݒ�L�[�ؑ�
					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
					break;

				case KEY_TEN_F3:	// �e�R�i���~�j�L�[����
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

ushort	UsMnt_NgLog_GetLogCnt(ushort Sdate, ushort Edate)
{
	ushort			log_date;			// ���O�i�[���t
	ushort			LOG_Date[3];		// ���O�i�[���t�i[0]�F�N�A[1]�F���A[2]�F���j
	ushort			log_cnt;			// �w����ԓ����O����
	ushort			cnt;
	ushort			LogCnt_total;

	log_cnt = 0;
	LogCnt_total = Ope_Log_TotalCountGet(eLOG_NGLOG);
	
	for(cnt=0; cnt < LogCnt_total; cnt++) {
		if( 0 == Ope_Log_1DataGet(eLOG_NGLOG, cnt, &nglog_data)){		// 
			log_cnt = 0;
			break;
		}
		LOG_Date[0] = nglog_data.NowTime.Year;
		LOG_Date[1] = nglog_data.NowTime.Mon;
		LOG_Date[2] = nglog_data.NowTime.Day;
		log_date = dnrmlzm(	(short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]) ;
		if( (log_date >= Sdate) && (log_date <= Edate) ) {
			log_cnt++;
		}
	}
	return(log_cnt);
}
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//static ulong nmlS;															/* �����J�n�����ϊ��p */
//static ulong nmlE;															/* �����I�������ϊ��p */
//ushort UsMnt_SearchIOLog(ushort offset, ushort no, ulong sdate, ulong edate, IoLog_Data *p)
//{
//	uchar BType;															/* �������@(0:������/1:�w�����/2:�w�����) */
//	ushort i, j;															/* ���O�e�[�u�������p���[�v�ϐ� */
//	ulong date1;															/* �Q�ƒ��̃C�x���g���������ϊ��p */
//	ulong date2;															/* �Q�ƒ��ЂƂ�̃C�x���g�����ϊ��p */
//	if(sdate && edate)														/* �����J�n�E�I���������Z�b�g����Ă��� */
//	{
//		BType = 2;															/* �������@���w����� */
//	}
//	else if(sdate)															/* �����J�n�����̂݃Z�b�g����Ă��� */
//	{
//		BType = 1;															/* �������@���w����� */
//	}
//	else																	/* ��L�ȊO */
//	{
//		BType = 0;															/* �������@�𖳏��� */
//	}
//	nmlS = sdate;															/* �����J�n���������ꎞ�̈�� */
//	nmlE = edate;															/* �����I�����������ꎞ�̈�� */
//
//	memset(p, 0, sizeof(IoLog_Data));
//	for (i = offset; 0 != Ope_Log_1DataGet(eLOG_IOLOG, i, p); i++) {			// �����̌����J�n�ʒu���烍�O�̈斖���܂�
//		/* �Q�ƒ����O���w�肳�ꂽ�Ԏ��ԍ��ƈ�v �܂��� �Ԏ��ԍ��w�薳�� ���� ���O�L�^���������Ă��� */
//		if ((p->room == no) || (no == 0xFFFF)) {
//			/* �Q�ƒ����O�̃C�x���g���Ō�܂Ō��� */
//			for (j = 0; (j < IO_EVE_CNT) && (p->iolog_data[j].Event_type != 0); j++) {
//				date1 = enc_nmlz_mdhm(								// �Q�ƒ��C�x���g����������ϊ�
//					p->iolog_data[j].NowTime.Year,					// �N
//					p->iolog_data[j].NowTime.Mon,					// ��
//					p->iolog_data[j].NowTime.Day,					// ��
//					p->iolog_data[j].NowTime.Hour,					// ��
//					p->iolog_data[j].NowTime.Min					// ��
//					);
//				switch (BType) {									// �������@
//				case 0:												// �������@:������
//					return (ushort)(i + 1);							// ���݂̃��O�Q�ƈʒu(+1)��Ԃ�
//				case 1:												// �������@:�w�����
//					if(date1 == nmlS) {								// �w������ƃC�x���g������������v
//						return (ushort)(i + 1);						// ���݂̃��O�Q�ƈʒu(+1)��Ԃ�
//					} else if ((j< (IO_EVE_CNT-1)) && (p->iolog_data[j+1].Event_type != 0)) {	// ���̋L�^���ɃC�x���g������
//						date2 = enc_nmlz_mdhm(						// ���̃C�x���g����������ϊ�
//							p->iolog_data[j+1].NowTime.Year,		// �N
//							p->iolog_data[j+1].NowTime.Mon,			// ��
//							p->iolog_data[j+1].NowTime.Day,			// ��
//							p->iolog_data[j+1].NowTime.Hour,		// ��
//							p->iolog_data[j+1].NowTime.Min			// ��
//							);
//						if ((date1 <= nmlS) && (nmlS <= date2)) {	// ���݂��w��ȍ~ ���� �����w��ȑO
//							return (ushort)(i + 1);					// ���݂̃��O�Q�ƈʒu(+1)��Ԃ�
//						}
//					}
//					break;
//				case 2:												// �������@:�w�����
//					if((date1 >= nmlS) && (date1 <= nmlE)) {		// �������J�n�ȍ~ ���� �������I���ȑO
//						return (ushort)(i + 1);						// ���݂̃��O�Q�ƈʒu(+1)��Ԃ�
//					} else if ((j< (IO_EVE_CNT-1)) && (p->iolog_data[j+1].Event_type != 0)) {	// ���̋L�^���ɃC�x���g������
//						date2 = enc_nmlz_mdhm(						// ���̃C�x���g����������ϊ�
//							p->iolog_data[j+1].NowTime.Year,		// �N
//							p->iolog_data[j+1].NowTime.Mon,			// ��
//							p->iolog_data[j+1].NowTime.Day,			// ��
//							p->iolog_data[j+1].NowTime.Hour,		// ��
//							p->iolog_data[j+1].NowTime.Min			// ��
//							);
//						if ((date1 <= nmlS) && (nmlS <= date2)) {	// ���݂��w��ȍ~ ���� �����w��ȑO
//							return (ushort)(i + 1);					// ���݂̃��O�Q�ƈʒu(+1)��Ԃ�
//						}
//					}
//					break;
//				}
//			}
//		}
//		memset(p, 0, sizeof(IoLog_Data));
//	}
//	memset(p, 0, sizeof(IoLog_Data));
//	return 0;
//}
//
//ushort	UsMnt_IoLog_GetLogCnt(ushort no, ushort* date)
//{
//	ushort offset;																					/* ��������v�������O�̈ʒu */
//	ushort log_cnt;																					/* �����Ɉ�v�������O�̌��� */
//	ulong Sdate, Edate;																				/* �����J�n�E�I�������ϊ��p */
//
//	Sdate = enc_nmlz_mdhm(date[0], (uchar)date[1], (uchar)date[2], (uchar)date[3], (uchar)date[4]);	/* �����J�n�������ϊ� */
//	Edate = enc_nmlz_mdhm(date[5], (uchar)date[6], (uchar)date[7], (uchar)date[8], (uchar)date[9]);	/* �����I���������ϊ� */
//	for(offset = 0, log_cnt = 0; 0 != (offset = UsMnt_SearchIOLog(offset, no, Sdate, Edate, &Io_log_wk)); log_cnt++);
//	return log_cnt;																					/* ������v���O������Ԃ� */
//}
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
/*[]----------------------------------------------------------------------[]*
 *| ���[�U�[�����e�i���X�F���o�Ƀ��O                					   |*
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : UsMnt_IoLog											   |*
 *| PARAMETER    : none													   |*
 *| RETURN VALUE : MOD_CHG : mode change								   |*
 *|              : MOD_EXT : F5 key										   |*
 *[]----------------------------------------------------------------------[]*
 *| REMARK       : 														   |*
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	UsMnt_IoLog(ushort no)
{
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	ushort			RcvMsg;				// ��Mү���ފi�[ܰ�
//	ushort			pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
//	T_FrmLogPriReq2	FrmLogPriReq2;		// �󎚗v��ү����ܰ�
//	T_FrmPrnStop	FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
//	ushort			Date_Now[10];		// �m�����t�ް��i�J�n�N[0],��[1],��[2],��[3],��[4]�A�I��[5],��[6],��[7],��[8],��[9]�j
//	ushort			Date_Old[10];		// �m��O���t�ް��i�J�n�N[0],��[1],��[2],��[3],��[4]�A�I��[5],��[6],��[7],��[8],��[9]�j
//	ushort			NewOldDate[10];		// �ŌÁ��ŐV���t�ް��i�J�n�N[0],��[1],��[2],��[3],��[4]�A�I��[5],��[6],��[7],��[8],��[9]�j
//	uchar			flg;				// �J�[�\���s�ړ��L(1)��(0)
//	uchar			pos		= 0;		// �����ް����͈ʒu�i�J�n�N(0),��(1),��(2)��(3)��(4)�A�I���N(5),��(6),��(7)��(8)��(9)�j
//	short			inp		= -1;		// �����ް�
//	uchar			All_Req	= OFF;		// �u�S�āv�w�蒆�׸�
//	uchar			Date_Chk;			// ���t�w���ް������׸�
//	ushort			Sdate, Edate;		// �J�n���t�A�I�����t�F����(YYYY+MM+DD)
//	ushort			Stime, Etime;		// �J�n�����A�I�������F����(hh+mm)
//	union ioLogNumberInfo num;			// ���O�Ǘ��ԍ��擾�p�\����
//
//	ushort LogCnt_total;				// �����O����
//	ushort LogCnt;						// �w����ԓ��Ώۃ��O����
//	extern	uchar	LogDateChk_Range_Minute( uchar pos, short data );
//	memset(&Io_log_wk, 0, sizeof(Io_log_wk));
//	/* ������ʕ\�� */
//	dispmlclr(1,7);																	// ��ʃN���A
//	Fun_Dsp(FUNMSG[77]);															// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
//	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, SMSTR1[24]);			// "�@�@���΂炭���҂����������@�@"
//	LogCnt = LogPtrGet2(no, &num.n);												// �ŌÁE�ŐV���O�f�[�^�̃|�C���^�[�擾
//	displclr(4);
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[128]);					// "�Ԏ��ԍ��F�@�@�@�@�@�@�@�@�@�@"
//	opedsp6(1, 10, no, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);							// �Ԏ��ԍ��\��
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[6]);					// "�J�n���t�x�x�x�x�N�l�l���c�c��"
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[8]);					// "�J�n���t�@�@�@�@�@�@�O�O�F�O�O"
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[7]);					// "�I�����t�x�x�x�x�N�l�l���c�c��"
//	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LOGSTR2[8]);					// "�I�����t�@�@�@�@�@�@�O�O�F�O�O"
//	if(num.n) {
//		Ope_Log_1DataGet(eLOG_IOLOG, (ushort)num.log.sdat, &Io_log_wk);
//		Date_Now[0]	= Io_log_wk.iolog_data[0].NowTime.Year;				// �J�n�i�N�j���ŌÃ��O�i�N�j
//		Date_Now[1]	= Io_log_wk.iolog_data[0].NowTime.Mon;				// �J�n�i���j���ŌÃ��O�i���j
//		Date_Now[2]	= Io_log_wk.iolog_data[0].NowTime.Day;				// �J�n�i���j���ŌÃ��O�i���j
//		Date_Now[3]	= Io_log_wk.iolog_data[0].NowTime.Hour;				// �J�n�i���j���ŌÃ��O�i���j
//		Date_Now[4]	= Io_log_wk.iolog_data[0].NowTime.Min;				// �J�n�i���j���ŌÃ��O�i���j
//		
//		Ope_Log_1DataGet(eLOG_IOLOG, (ushort)num.log.edat, &Io_log_wk);
//		Date_Now[5]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Year;	// �I���i�N�j���ŐV���O�i�N�j
//		Date_Now[6]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Mon;		// �I���i���j���ŐV���O�i���j
//		Date_Now[7]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Day;		// �I���i���j���ŐV���O�i���j
//		Date_Now[8]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Hour;	// �I���i���j���ŐV���O�i���j
//		Date_Now[9]	= Io_log_wk.iolog_data[num.log.evnt].NowTime.Min;		// �I���i���j���ŐV���O�i���j
//	} else {
//		Date_Now[0] = (ushort)CLK_REC.year;				// �J�n�i�N�j�����ݓ����i�N�j
//		Date_Now[1] = (ushort)CLK_REC.mont;				// �J�n�i���j�����ݓ����i���j
//		Date_Now[2] = (ushort)CLK_REC.date;				// �J�n�i���j�����ݓ����i���j
//		Date_Now[3] = (ushort)CLK_REC.hour;				// �J�n�i���j�����ݓ����i���j
//		Date_Now[4] = (ushort)CLK_REC.minu;				// �J�n�i���j�����ݓ����i���j
//		Date_Now[5] = (ushort)CLK_REC.year;				// �I���i�N�j�����ݓ����i�N�j
//		Date_Now[6] = (ushort)CLK_REC.mont;				// �I���i���j�����ݓ����i���j
//		Date_Now[7] = (ushort)CLK_REC.date;				// �I���i���j�����ݓ����i���j
//		Date_Now[8] = (ushort)CLK_REC.hour;				// �I���i���j�����ݓ����i���j
//		Date_Now[9] = (ushort)CLK_REC.minu;				// �I���i���j�����ݓ����i���j
//	}
//	memcpy(Date_Old, Date_Now, sizeof(Date_Old));		// �ύX�O���t�ް��Z�[�u
//	memcpy(NewOldDate, Date_Now, sizeof(NewOldDate));	// �w�S�āx�{�^���p�ŐV�E�ŌÓ��t�ް�
//	LogDateDsp3( &Date_Now[0], pos );					// �J�n���t�ް����I�����t�ް��@�\��
//
//	LogCnt_total = LogCnt;								// ���O����������
//	// ���O�����\���i�Ώی����^�������j
//	LogCntDsp2( LogCnt, LogCnt_total, 3 );											// ���O�����\���i�O���j
//	Fun_Dsp( FUNMSG[83] );															// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
//
//	/* ��ʑ���󂯕t�� */
//	for( ; ; ){
//		RcvMsg = StoF( GetMessage(), 1 );					// �C�x���g�҂�
//
//		/*  �󎚗v���O�i�󎚗v���O��ʁj*/
//		if( pri_cmd == 0 ){
//			switch( KEY_TEN0to9( RcvMsg ) ){				// �C�x���g�H
//
//				/* �ݒ�L�[�ؑ� */
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				case KEY_MODECHG:
//					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
//					break;
//
//				/* �O�`�X */
//				case KEY_TEN:
//					if( All_Req == OFF ){					// �u�S�āv�w�蒆�łȂ��ꍇ�i�w�蒆�́A��������j
//						BUZPI();
//						if( inp == -1 ){
//							inp = (short)(RcvMsg - KEY_TEN0);
//						}else{
//							inp = (short)( inp % 10 ) * 10 + (short)(RcvMsg - KEY_TEN0);
//						}
//						opedsp	(							// �����ް��\��
//									POS_DATA31[pos][0],		// �\���s
//									POS_DATA31[pos][1],		// �\�����
//									(ushort)inp,			// �\���ް�
//									POS_DATA31[pos][2],		// �ް�����
//									POS_DATA31[pos][3],		// �O�T�v���X�L��
//									1,						// ���]�\���F����
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//					}
//					break;
//
//				/* ����L�[ */
//				case KEY_TEN_CL:
//					BUZPI();
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
//						opedsp	(							// ���͑O���ް���\��������
//									POS_DATA31[pos][0],		// �\���s
//									POS_DATA31[pos][1],		// �\�����
//									Date_Now[pos],			// �\���ް�
//									POS_DATA31[pos][2],		// �ް�����
//									POS_DATA31[pos][3],		// �O�T�v���X�L��
//									1,						// ���]�\���F����
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//						inp = -1;							// ���͏�ԏ�����
//					}
//					break;
//
//				/* �e�P�i���j�e�Q�i���j�L�[���� */
//				case KEY_TEN_F1:
//				case KEY_TEN_F2:
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ�́i�@���@�@���@�L�[�F���t���́j
//						if( inp == -1 ){		// ���͂Ȃ��̏ꍇ
//							BUZPI();
//
//							if( RcvMsg == KEY_TEN_F1 ){
//								flg = LogDatePosUp3( &pos, 0 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
//							}else{
//								flg = LogDatePosUp3( &pos, 1 );		// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
//							}
//						}
//						else{				//	���͂���̏ꍇ
//							if( OK == LogDateChk_Range_Minute( pos, inp ) ){	// �����ް��n�j
//								BUZPI();
//								if( pos == 0 || pos == 5 ){		// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
//									Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//								}else{
//									Date_Now[pos] = (ushort)inp;
//								}
//								if( RcvMsg == KEY_TEN_F1 ){
//									flg = LogDatePosUp3( &pos, 0 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��|�P�j
//								}else{
//									flg = LogDatePosUp3( &pos, 1 );	// ���͈ʒu�ް��X�V�i�ʒu�ԍ��{�P�j
//								}
//							}
//							else{								//	�����ް��m�f
//								BUZPIPI();
//							}
//						}
//						// �w����ԓ��Ώۃ��O�������擾���\������
//						if( (flg == 1) && (memcmp(Date_Old, Date_Now, sizeof(Date_Old)) != 0) ) {	// �s�ړ��ŕω����������ꍇ�̂�
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
//							Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// �I�����t�ް��擾
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// �J�n�����ް��擾
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// �I�������ް��擾
//							LogCnt = 0;
//							if( Sdate < Edate ){
//								Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
//								LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//								Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							}else{
//								if( Sdate == Edate ){
//									if( Stime <= Etime ){
//										Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
//										LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//										Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//									}
//								}
//							}
//							LogCntDsp2( LogCnt, LogCnt_total, 4 );									// ���O�����ĕ\��
//							memcpy(Date_Old, Date_Now, sizeof(Date_Old));							// �ύX�O���t�ް��Z�[�u
//						}
//
//						LogDateDsp3( &Date_Now[0], pos );		// ���وʒu�ړ��i���͂m�f�̏ꍇ�A�ړ����Ȃ��j
//						inp = -1;								// ���͏�ԏ�����
//					}
//					break;
//
//				/* �e�R�i�S�āj�L�[���� */
//				case KEY_TEN_F3:
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�H�i�w�蒆�́A��������j
//						if( LogCnt_total != 0 ){		//	���O�f�[�^������ꍇ
//							BUZPI();
//							memcpy( &Date_Now[0], &NewOldDate[0], sizeof(Date_Now) );	// �ŌÁ��ŐV�ް����t���߰
//							pos = 0;								// ���وʒu���J�n�i�N�j
//							inp = -1;								// ���͏�ԏ�����
//							LogDateDsp3( &Date_Now[0], 0xff );		// �J�n���t�ް����I�����t�ް��@�\���i���ٕ\���Ȃ��j
//							// �w����ԓ��Ώۃ��O�������擾���\������
//							Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
//							Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// �I�����t�ް��擾
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// �J�n�����ް��擾
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// �I�������ް��擾
//							Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
//							LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							LogCntDsp2( LogCnt, LogCnt_total, 4 );			// ���O�����ĕ\��
//							Fun_Dsp( FUNMSG[81] );					// ̧ݸ��ݷ��\���F"�@�@�@�N���A�@�@�@ ���s  �I�� "
//							All_Req = ON;							// �u�S�āv�w�蒆�׸޾��
//						}
//						else{					//	���O�f�[�^���Ȃ��ꍇ
//							BUZPIPI();
//						}
//					}
//					break;
//
//				/* �e�S�i���s�j�L�[���� */
//				case KEY_TEN_F4:
//					Date_Chk = OK;
//					if( inp != -1 ){	//	���͂���̏ꍇ
//						if( OK == LogDateChk_Range_Minute( pos, inp ) ){	// �w����Ԃn�j�i�J�n<=�I���N���������j�̏ꍇ
//							if( pos == 0 || pos == 5 ){		// �����ް����ށi�N�ް��̏ꍇ�A���Q���̂ݏ���������j
//								Date_Now[pos] = (ushort)(Date_Now[pos] - (Date_Now[pos]%100) + (ushort)inp);
//							}else{
//								Date_Now[pos] = (ushort)inp;
//							}
//						}else{								//	���̓f�[�^�m�f�̏ꍇ
//							Date_Chk = NG;
//						}
//					}
//					// �w����ԓ��Ώۃ��O�������擾���\������
//					Sdate = dnrmlzm((short)Date_Now[0],	(short)Date_Now[1],	(short)Date_Now[2]);// �J�n���t�ް��擾
//					Edate = dnrmlzm((short)Date_Now[5],	(short)Date_Now[6],	(short)Date_Now[7]);// �I�����t�ް��擾
//					Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];								// �J�n�����ް��擾
//					Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];								// �I�������ް��擾
//					LogCnt = 0;
//					if( Sdate < Edate ){
//						Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
//						LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					}else{
//						if( Sdate == Edate ){
//							if( Stime <= Etime ){
//								Ope_KeyRepeatEnable(0);			// �L�[���s�[�g����
//								LogCnt = UsMnt_IoLog_GetLogCnt(no, Date_Now);
//								Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//							}
//						}
//					}
//					LogCntDsp2( LogCnt, LogCnt_total, 4 );			// ���O�����ĕ\��
//					if( Date_Chk == OK ){
//						if( All_Req != ON ){	// �u�S�āv�w��łȂ��ꍇ�A�N�����������s��
//							//	�N���������݂�����t���`�F�b�N����
//							if( chkdate( (short)Date_Now[0], (short)Date_Now[1], (short)Date_Now[2] ) != 0 ){	// �J�n���t����
//								Date_Chk = NG;
//							}
//							if( chkdate( (short)Date_Now[5], (short)Date_Now[6], (short)Date_Now[7] ) != 0 ){	// �I�����t����
//								Date_Chk = NG;
//							}
//						}
//					}
//					if( Date_Chk == OK ){
//						//	�J�n���t�����I�����t���`�F�b�N����
//						Sdate = dnrmlzm(							// �J�n���t�ް��擾
//											(short)Date_Now[0],
//											(short)Date_Now[1],
//											(short)Date_Now[2]
//										);
//						Edate = dnrmlzm(							// �I�����t�ް��擾
//											(short)Date_Now[5],
//											(short)Date_Now[6],
//											(short)Date_Now[7]
//										);
//						if( Sdate > Edate ){
//							Date_Chk = NG;
//						}
//
//						/*-------------------------------------------------*/
//						// 	�J�n���t���I�����t�̎��́A�J�n���������I���������`�F�b�N����
//						if( Sdate == Edate ){
//							Stime = (ushort)(Date_Now[3]*60) + Date_Now[4];		// �J�n�����ް��擾
//							Etime = (ushort)(Date_Now[8]*60) + Date_Now[9];		// �I�������ް��擾
//							if( Stime > Etime ){
//								Date_Chk = NG;
//							}
//						}
//						/*--------------------------------------------------*/
//
//					}
//					if( Date_Chk == OK ){	// �J�n���t�������I�����t���������n�j�̏ꍇ
//						BUZPI();
//						/*------	�󎚗v��ү���ޑ��M	-----*/
//						memset( &FrmLogPriReq2,0,sizeof(FrmLogPriReq2) );		// �󎚗v��ү����ܰ��O�ر
//						FrmLogPriReq2.prn_kind	= R_PRI;						// �Ώ�������F�ެ���
//						FrmLogPriReq2.Kikai_no	= (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
//						FrmLogPriReq2.Kakari_no	= OPECTL.Kakari_Num;			// �W��No.
//						FrmLogPriReq2.Room_no	= no;							// �Ԏ��ԍ�
//
//						if( All_Req == ON ){
//							FrmLogPriReq2.BType	= 0;							// �������@	�F�S��
//						}else if( (Sdate == Edate) && (Stime == Etime) ){
//							FrmLogPriReq2.BType	= 1;							// �������@	�F���t
//							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
//							FrmLogPriReq2.TSttTime.Hour	= (uchar)Date_Now[3];
//							FrmLogPriReq2.TSttTime.Min	= (uchar)Date_Now[4];
//						}else{
//							FrmLogPriReq2.BType	= 2;							// �������@	�F����
//							FrmLogPriReq2.TSttTime.Year	= Date_Now[0];
//							FrmLogPriReq2.TSttTime.Mon	= (uchar)Date_Now[1];
//							FrmLogPriReq2.TSttTime.Day	= (uchar)Date_Now[2];
//							FrmLogPriReq2.TSttTime.Hour	= (uchar)Date_Now[3];
//							FrmLogPriReq2.TSttTime.Min	= (uchar)Date_Now[4];
//							FrmLogPriReq2.TEndTime.Year	= Date_Now[5];
//							FrmLogPriReq2.TEndTime.Mon	= (uchar)Date_Now[6];
//							FrmLogPriReq2.TEndTime.Day	= (uchar)Date_Now[7];
//							FrmLogPriReq2.TEndTime.Hour	= (uchar)Date_Now[8];
//							FrmLogPriReq2.TEndTime.Min	= (uchar)Date_Now[9];
//						}
//						FrmLogPriReq2.LogCount = LogCnt_total;	// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
//						queset( PRNTCBNO, PREQ_IO_LOG, sizeof(T_FrmLogPriReq2), &FrmLogPriReq2 );
//						pri_cmd = PREQ_IO_LOG;					// ���M����޾���
//						LogDateDsp3( &Date_Now[0], 0xff );	// �J�n���t�ް����I�����t�ް��@�\���i���ٕ\���������j
//						Fun_Dsp( FUNMSG[82] );				// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
//
//					}else{	// �J�n���t���I�����t�����m�f�̏ꍇ
//						BUZPIPI();
//						LogDateDsp3( &Date_Now[0], pos );		// �J�n���t�ް����I�����t�ް��@�\��
//						inp = -1;								// ���͏�ԏ�����
//					}
//					break;
//
//				/* �e�T�i�I���j�L�[���� */
//				case KEY_TEN_F5:							// 
//					BUZPI();
//
//					if( All_Req == OFF ){	// �u�S�āv�w�蒆�łȂ��ꍇ
//						RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
//					}else{					//	�u�S�āv�w�蒆�̏ꍇ
//						pos = 0;							// ���وʒu���J�n�i�N�j
//						inp = -1;							// ���͏�ԏ�����
//						opedsp	(							// ���ق�\��������
//									POS_DATA31[pos][0],		// �\���s
//									POS_DATA31[pos][1],		// �\�����
//									Date_Now[pos],			// �\���ް�
//									POS_DATA31[pos][2],		// �ް�����
//									POS_DATA31[pos][3],		// �O�T�v���X�L��
//									1,						// ���]�\���F����
//									COLOR_BLACK, 
//									LCD_BLINK_OFF
//								);
//						Fun_Dsp( FUNMSG[83] );				// ̧ݸ��ݷ��\���F"�@���@�@���@ �S��  ���s  �I�� "
//						All_Req = OFF;						// �u�S�āv�w�蒆�׸�ؾ��
//					}
//					break;
//				default:
//					break;
//			}
//		}
//		/* �󎚗v����i�󎚏I���҂���ʁj*/
//		else{
//			if( RcvMsg == ( pri_cmd | INNJI_ENDMASK ) ){	// �󎚏I��ү���ގ�M�H
//				RcvMsg = MOD_EXT;							// YES�F�O��ʂɖ߂�
//			}
//
//			switch( RcvMsg ){								// �C�x���g�H
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				case LCD_DISCONNECT:
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				case KEY_MODECHG:	// �ݒ�L�[�ؑ�
//					RcvMsg = MOD_CHG;						// �O��ʂɖ߂�
//					break;
//
//				case KEY_TEN_F3:	// �e�R�i���~�j�L�[����
//					BUZPI();
//					/*------	�󎚒��~ү���ޑ��M	-----*/
//					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
//					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
//					RcvMsg = MOD_EXT;						// �O��ʂɖ߂�
//					break;
//
//				default:
//					break;
//			}
//		}
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
////		if(RcvMsg == MOD_CHG){								// ���[�h�`�F���W
//		if(RcvMsg == MOD_CHG || RcvMsg == LCD_DISCONNECT){								// ���[�h�`�F���W
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			RcvMsg = KEY_MODECHG;							// ���[�h�`�F���W�̃f�[�^�`����ύX
//			break;											// �O��ʂɖ߂�
//		}else if(RcvMsg == MOD_EXT){						// �I���{�^��
//			break;											// �O��ʂɖ߂�
//		}
//	}
//	return( RcvMsg );
	return( MOD_EXT );
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
}
/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�������v�����g                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_Logprintmenu( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	UsMnt_Logprintmenu(void)
{
	unsigned short	usPlogEvent;
	char	wk[2];
	ushort	NewOldDate[6];		// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
	ushort	LogCount;			// LOG�o�^����
	date_time_rec	NewestDateTime, OldestDateTime;

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[125] );		/* [125] "���������v�����g���@�@�@�@�@�@�@�@�@" */


		usPlogEvent = Menu_Slt( LOGPRNMENU, USM_LOG_TBL, (char)USM_LOG_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usPlogEvent ){

			/* �s���� */
			case MNT_NGLOG:
				if (OPECTL.Mnt_lev < 2 )		//�Ǘ��҃��x����
					BUZPIPI();
				else
					usPlogEvent = UsMnt_NgLog();
				break;

			/* ���o�� */
			case MNT_IOLOG:
				if(( gCurSrtPara = Is_CarMenuMake(CAR_2_MENU)) != 0 ){
					if( Ext_Menu_Max > 1){
						usPlogEvent = UsMnt_PreAreaSelect(MNT_IOLOG);
					}
					else {
						usPlogEvent = UsMnt_AreaSelect(MNT_IOLOG);
					}
				}else{
					BUZPIPI();
					continue;
				}
				break;

			/* �s���E�����o�� */
			case FUSKYO_LOG:
				// LOG�ް��̍ŌÁ��ŐV���t�Ɠo�^�����𓾂�
				Ope2_Log_NewestOldestDateGet( eLOG_ABNORMAL, &NewestDateTime, &OldestDateTime, &LogCount );
				// �ŌÁ��ŐV���t�ް��擾
				LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );
				//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
				usPlogEvent = SysMnt_Log_CntDsp2(LOG_ABNORMAL, LogCount, LOGSTR1[7], PREQ_FUSKYO_LOG, &NewOldDate[0]);
				break;

			case MOD_EXT:
				OPECTL.Mnt_mod = 1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				return MOD_EXT;
			default:
				break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
			SetChange = 1;			// FLASH���ގw��
		}
		if (usPlogEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (usPlogEvent == MOD_CUT ) {
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�W���L���f�[�^                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_AttendantValidData( void )                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	UsMnt_AttendantValidData(void)
{
	ushort	msg;
	T_FrmLogPriReq1	aid_pri;
	T_FrmPrnStop	FrmPrnStop;
	t_ValidWriteData *work = &at_invwork;		// �W���L���f�[�^���[�N�e�[�u��
	long	no,max,top,input;
	ushort	attendNo;							// �W��No.
	char	i,priend;
	char	dsp;

	dispclr();									// ��ʃN���A

//	max = MNT_ATTEND_REGSTER_COUNT-1;			// ��ʂœo�^�\�Ȍ���
	max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// ��ʂœo�^�\�Ȍ���
	if(max == MNT_ATTEND_REGSTER_COUNT) {
		--max;
	}

	no = top = 0;
	input = -1;
	dsp = 1;

	// ��ʃ^�C�g����\��
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);			// "���W���L���f�[�^���@�@�@�@�@"
	grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[8]);				//����������

	Fun_Dsp(FUNMSG2[50]);						// "�@���@�@���@ ����� ����  �I�� "

	memcpy(work, &Attend_Invalid_table, sizeof(t_ValidWriteData));
	
	for ( ; ; ) {
	
		opedpl(0, 20, (ulong)Attend_Invalid_table.count, 4, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
		if (dsp) {
			for (i = 0; i < 6; i++) {
				if (top+i > max)
					break;
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "�|" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);				/* �ԍ� */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "�@���@�@���@ ����� ����  �I�� "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "�@���@�@���@ ����� �폜  �I�� "
					}
				}
			}
			dsp = 0;
		}
		msg = StoF(GetMessage(), 1);

		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return (MOD_CHG);
		case KEY_TEN_F5:		/* F5:�I�� */
			BUZPI();
			return 0;
		case KEY_TEN_F2:		/* F2:�{ */
			BUZPI();
			no++;
			if (no > max) {
				no = top = 0;
			}
			else if (no > top+5) {
				top++;
			}
			for (i = 0; i < 6; i++) {
				if (top+i > max) {
					break;
				}
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "�|" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "�@���@�@���@ ����� ����  �I�� "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "�@���@�@���@ ����� �폜  �I�� "
					}
				}
			}
			input = -1;
			break;
		case KEY_TEN_F1:		/* F1:�|�^�� */
			BUZPI();
			no--;
			if (no < 0) {
				if (max > 5) {
					top = max-5;
				}
				else {
					top = 0;
				}
				no = max;
			}
			else if (no < top) {
				top--;
			}

			for (i = 0; i < 6; i++) {
				if (top+i > max) {
					break;
				}
				opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
				grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "�|" */
				opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
				if(no==top+i) {
					if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
						Fun_Dsp(FUNMSG2[50]);						// "�@���@�@���@ ����� ����  �I�� "
					}
					else {
						Fun_Dsp(FUNMSG2[30]);						// "�@���@�@���@ ����� �폜  �I�� "
					}
				}
			}
			input = -1;
			break;
		case KEY_TEN_F3:		/* ����� */
			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
				BUZPIPI();
				break;
			}
			BUZPI();
			aid_pri.prn_kind = R_PRI;
			aid_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
			aid_pri.Kakari_no = OPECTL.Kakari_Num;
			queset(PRNTCBNO, PREQ_ATEND_INV_DATA, sizeof(T_FrmLogPriReq1), &aid_pri);		// �󎚗v��
			Ope_DisableDoorKnobChime();

			/* �v�����g�I����҂����킹�� */
			Fun_Dsp(FUNMSG[82]);				// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"

			priend = 0;
			for ( ; ; ) {
				
				msg = StoF( GetMessage(), 1 );

				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
					msg &= (~INNJI_ENDMASK);
				}
				switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case	LCD_DISCONNECT:
						return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case	KEY_MODECHG:			// Ӱ����ݼ�
						BUZPI();
						return MOD_CHG;
	
					case	PREQ_ATEND_INV_DATA:	// �󎚏I��
						priend = 1;
						break;

					case	KEY_TEN_F3:				// F3���i���~�j
						BUZPI();
						FrmPrnStop.prn_kind = R_PRI;
						queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );	// �󎚒��~�v��
						priend = 1;
						break;
					case 	TIMEOUT6:
						BUZPIPI();
						priend = 1;
						break;
				}
				if(priend){
					Fun_Dsp(FUNMSG2[50]);			// "�@���@�@���@ ����� ����  �I�� "
					break;
				}
			}

			break;
		case KEY_TEN_F4:		// ����
			attendNo = Attend_Invalid_table.aino[dispPos(top,no-top)];
			if(attendNo == 0) {					// �W��No.���o�^�s
				if(input > 0 && input < 99) {
					if((short)-1 == AteVaild_Check((ushort)input)) {	// ���͌W��No.�`�F�b�N
						BUZPI();
						AteVaild_Update((ushort)input, 1);				// �W��No.��o�^
						max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// ��ʂ̍ő匏���X�V
						if(max == MNT_ATTEND_REGSTER_COUNT) {
							--max;
						}

						if(no < max) {
							no++;
						}
						if (no > max) {
							no = top = 0;
						}
						else if (no > top+5) {
							top++;
						}
						for (i = 0; i < 6; i++) {
							if (top+i > max) {
								break;
							}
							opedpl((ushort)(1+i), 2, (ulong)(top+i+1), 4, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
							grachr((ushort)(1+i), 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3]);				/* "�|" */
							opedpl((ushort)(1+i), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,i)], 2, 1, (no==top+i)?1:0, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
							if(no==top+i) {
								if(Attend_Invalid_table.aino[dispPos(top,i)] == 0) {
									Fun_Dsp(FUNMSG2[50]);						// "�@���@�@���@ ����� ����  �I�� "
								}
								else {
									Fun_Dsp(FUNMSG2[30]);						// "�@���@�@���@ ����� �폜  �I�� "
								}
							}
						}
					}
					else {
						// �W��No.�o�^�ς݂̓G���[
						BUZPIPI();
						opedpl((ushort)(1+no-top), 16, (ulong)attendNo, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �W��No. */
					}
				}
				else {							// �W��No.�͈͊O
					BUZPIPI();
					opedpl((ushort)(1+no-top), 16, (ulong)attendNo, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �W��No. */
				}
			}
			else {								// �W��No.�o�^�s
				BUZPI();
				if(1 != AteVaild_ConfirmDelete(attendNo)) {							// �W��No.�폜�m�F���
					// ��ʍĕ`��
					dispclr();														// ��ʃN���A
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);	// "���W���L���f�[�^���@�@�@�@�@"
					grachr(0, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[8]);		//����������
					max = MNT_MIN(MNT_ATTEND_REGSTER_COUNT, Attend_Invalid_table.count);// ��ʂ̍ő匏���X�V
					if(max == MNT_ATTEND_REGSTER_COUNT) {
						--max;
					}
					dsp = 1;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				else {
//					return (MOD_CHG);
//				}
				if(1 == AteVaild_ConfirmDelete(attendNo)){
					return (MOD_CHG);
				}else if(2 == AteVaild_ConfirmDelete(attendNo)){
					return (MOD_CUT);
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			input = -1;
			break;
		case KEY_TEN:			/* ����(�e���L�[) */
			if(0 == Attend_Invalid_table.aino[dispPos(top,no-top)]) {				// �W��No.���ݒ�s
				BUZPI();
				if (input == -1) {
					input = 0;
				}
				input = (input*10 + msg-KEY_TEN0) % 100L;
				opedpl((ushort)(1+no-top), 16, (ulong)input, 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �W��No. */
			}
			else {				// �W��No.�ݒ�s�͓��͕s��
				BUZPIPI();
				input = -1;
			}
			break;
		case KEY_TEN_CL:		/* ���(�e���L�[) */
			BUZPI();
			opedpl((ushort)(1+no-top), 2, (ulong)(no+1), 4, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �ԍ� */
			opedpl((ushort)(1+no-top), 16, (ulong)Attend_Invalid_table.aino[dispPos(top,no-top)], 2, 1, 1, COLOR_BLACK,  LCD_BLINK_OFF);	/* �W��No. */
			input = -1;
			break;
		default:
			break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*
 *| �W���L���f�[�^�F�o�^�E�폜											  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_Update										  |
 *| PARAMETER    : �W���ԍ��Fid	�o�^or�폜�Fstatus(0:�폜 1�F�o�^)		  |
 *| RETURN VALUE : ����F1	�ُ�F-1									  |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *| REMARK       : �폜��̐擪�f�[�^���z���[0]�ɂ���悤�f�[�^���\��������
 *|				   ��d�ۏ؂ɑΉ�										  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	AteVaild_Update(ushort id, ushort status)
{
	short idx;
	t_ValidWriteData *work = &at_invwork;			// �W���L���f�[�^���[�N�e�[�u��
	ushort	tmparea;
	memset(work,0,sizeof(t_ValidWriteData));

	if( id > 10000 || !id )
		return (char)-1;
		
	if( Attend_Invalid_table.count == MNT_ATTEND_REGSTER_COUNT && status == 1 ){	// �o�^����ɒB���Ă���ꍇ�̓o�^����
		wopelg(OPLOG_SET_ATE_INV_MAX, 0, 0);			// ���샂�j�^�o�^
		return (char)-1;								// �o�^�����͂��Ȃ�
	}

	idx = AteVaild_Check(id);
	
	if(status){										// �ް��̃A�b�v�f�[�g
		memcpy(work, &Attend_Invalid_table, sizeof(t_ValidWriteData));

		if (idx >= 0) {
			// �Y���f�[�^���� �� ���̃f�[�^���㏑��
				work->aino[idx] = id;
		}else{

			// �Y���f�[�^�Ȃ� �� �Ō���ɒǉ�
            work->aino[work->wtp] = id;

			if (work->count < ADN_VAILD_MAX) {
				work->count++;
			}
			if (++work->wtp >= ADN_VAILD_MAX) {
				work->wtp = 0;
			}
		}
	}else{														// �ް��̍폜
		if (idx >= 0) {											// �폜�ް����������ꍇ
            memcpy(work, &Attend_Invalid_table, 4); 			// �o�^���āE�����݈ʒu�̺�߰
			if(work->count == ADN_VAILD_MAX && work->wtp > 0){	// �ް����ݸ��ޯ̧��ɂȂ��Ă���ꍇ
				if(work->wtp < _AteGetDataPos(idx)){			// �폜�ް��̈ʒu�������ʒu�������ɂ������ꍇ
					/*[]----------------------------------------------------------------------[]
					 *|			   �擪(����)�ʒu   	  �폜�f�[�^
					 *|				  ��			          ��
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]        [999] 
					 *|		����������������������������������������������      ��-����
					 *|		���P�b�Q���R�b�S���T���U���V���W���X��10��11���E�E�E��999��
					 *|		����������������������������������������������      ��-����
					 *|			  				�b�@�擪�ʒu����폜�ް��܂ł��߰
					 *|			  				�b�A�폜�ް��̎����ް�����z��̍ŏI�ް��܂ł��߰
					 *|			  				�b�B�z��̐擪����擪�ʒu(�����ʒu)�܂ł��߰����B
					 *|			  				�����擪�ް����z��̐擪�ɗ���悤�ɍč\���������B
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]       [997][998][999] 
					 *|		����������������������������������������������      ����������������
					 *|		���R�b�S���T�b�U���V���W��10��11��12��13��14���E�E�E���P���Q���󔒄�
					 *|		����������������������������������������������      ����������������
					 *[]----------------------------------------------------------------------[]*/
					tmparea = (ushort)((work->count-1)-(_AteGetDataPos(idx)));
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[work->wtp], (sizeof(Attend_Invalid_table.aino[0])*(_AteGetDataPos(idx)-Attend_Invalid_table.wtp)));
	            	memcpy(&work->aino[(_AteGetDataPos(idx)-work->wtp)], &Attend_Invalid_table.aino[_AteGetDataPos(idx+1)], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
            		memcpy(&work->aino[tmparea+idx], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0])*work->wtp));
	  			}else{											// �폜�ް��̈ʒu�������ʒu�����O�������͓����������ꍇ
					/*[]----------------------------------------------------------------------[]
					 *|			  �폜�f�[�^		  �擪(����)�ʒu
					 *|		    	  �� 			          ��
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]        [999] 
					 *|		����������������������������������������������      ��-����
					 *|		���P�b�Q���R�b�S���T���U���V���W���X��10��11���E�E�E��999��
					 *|		����������������������������������������������      ��-����
					 *|			  				�b�@�擪�ʒu����z��̍ŏI�ް��܂ł��߰
					 *|			  				�b�A�z��̐擪����폜�ް��܂ł��߰
					 *|			  				�b�B�폜�ް��̎����ް�����擪�ʒu(�����ʒu)�܂ł��߰����B
					 *|			  				�b���폜�f�[�^���ŏI�ް��̏ꍇ�͇B�͎��{���Ȃ�
					 *|			  				�����擪�ް����z��̐擪�ɗ���悤�ɍč\���������B
					 *|      [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10]       [997][998][999] 
					 *|		����������������������������������������������      ����������������
					 *|		���X�b10��11�b12��13��14��15��16��17��18��19���E�E�E���V���W���󔒄�
					 *|		����������������������������������������������      ����������������
					 *[]----------------------------------------------------------------------[]*/
					if(!idx){							// �폜�ʒu���擪(����)�ʒu�������ꍇ
						work->wtp++;					// ��߰�J�n�ʒu�����炷
					}else{								
						if(idx != ADN_VAILD_MAX-1)		// �폜�ʒu���ŏI�ް��ʒu�ł͖����ꍇ
							memcpy(&work->aino[(ushort)(work->count-work->wtp)+_AteGetDataPos(idx)], &Attend_Invalid_table.aino[_AteGetDataPos(idx)+1], (sizeof(Attend_Invalid_table.aino[0])*(Attend_Invalid_table.wtp-(_AteGetDataPos(idx)+1))));
					}
					tmparea = (ushort)(work->count-work->wtp);
	            	memcpy(&work->aino[0], &Attend_Invalid_table.aino[work->wtp], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
	            	memcpy(&work->aino[tmparea], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0])*_AteGetDataPos(idx)));

				}
			}else{
				/*	�擪�ް����폜����ꍇ */
				if(!idx){
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[1], (sizeof(Attend_Invalid_table.aino[0]) * (ADN_VAILD_MAX-1)));
				}else{
					/*	�r���̂��폜����ꍇ */
					tmparea = (ushort)_AteGetDataPos(idx);
		            memcpy(&work->aino[0], &Attend_Invalid_table.aino[0], (sizeof(Attend_Invalid_table.aino[0]) * tmparea));
					tmparea = (ushort)((Attend_Invalid_table.count-1)-(_AteGetDataPos(idx)));
		            memcpy(&work->aino[_AteGetDataPos(idx)], &Attend_Invalid_table.aino[_AteGetDataPos(idx+1)], (sizeof(Attend_Invalid_table.aino[0])*tmparea));
				}
			}	
			work->count--;
			work->wtp = work->count;
		}else{
			return (char)-1;
		}
	}
	// ���[�N�G���A������̈�֏�������
	nmisave(&Attend_Invalid_table, work, sizeof(t_ValidWriteData));
	wopelg(OPLOG_SET_ATE_INV, 0, 0);
	return 1;

}
/*[]----------------------------------------------------------------------[]*
 *| �W���L���f�[�^�F�Q��												  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_Check     									  |
 *| PARAMETER    : �W���ԍ��Fid										      |
 *| RETURN VALUE : �Y���f�[�^�L�F�v�f�ԍ�	�Y���f�[�^���F-1              |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-03-14				    						  |
 *| REMARK       : 														  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	AteVaild_Check( ushort id )
{
	short idx,ofs,ret=-1;

	for (idx = 0; idx < Attend_Invalid_table.count; idx++) {
		ofs = _AteIdx2Ofs(idx);
		if (Attend_Invalid_table.aino[ofs] == id) {
			ret = idx;
			break;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *| �L���W��ID�e�[�u���f�[�^�X�V										  |
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : AteVaild_table_update								  |
 *| PARAMETER    : �W���ԍ��Fid										      |
 *| RETURN VALUE : �Y���f�[�^�L�F�v�f�ԍ�	�Y���f�[�^���F-1              |
 *[]----------------------------------------------------------------------[]*
 *| Author		 : T.Namioka			    							  |
 *| Date		 : 2006-06-22				    						  |
 *| REMARK       : 														  |
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	AteVaild_table_update( ushort *data, ushort size)
{
	t_ValidWriteData *work = &at_invwork;			// �����W���f�[�^���[�N�e�[�u��
	short ofs;

	memset(work,0,sizeof(t_ValidWriteData));
	memcpy(&work->aino, data, (size_t)size);

	for (ofs = 0; ofs < ADN_VAILD_MAX; ofs++) {
		if (!work->aino[ofs]) {
			memset( &work->aino[ofs],0,(sizeof(work->aino[ofs])*(ADN_VAILD_MAX-ofs)) );
			break;
		}
	}
	work->count = ofs;
	if( ofs != ADN_VAILD_MAX )
		work->wtp = ofs;
	else
		work->wtp = 0;
	nmisave(&Attend_Invalid_table, work, sizeof(t_ValidWriteData));
	wopelg(OPLOG_SET_ATE_INV, 0, 0);

}
/*[]----------------------------------------------------------------------[]*/
/*|  �L���W���f�[�^�폜��ʁ@�@                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AteVaild_ConfirmDelete(  )                              |*/
/*| PARAMETER    : ushort attendNo : �폜����W��No.                       |*/
/*| RETURN VALUE : 0 : exit                                                |*/
/*|              : 1 : mode change                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	AteVaild_ConfirmDelete(ushort attendNo)
{
	ushort	msg;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[124]);		// "���W���L���f�[�^���@�@�@�@�@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[155]);		// "�@�W�����@�@���폜���܂����H�@"
	Fun_Dsp(FUNMSG[19]);												// "�@�@�@�@�@�@ �͂� �������@�@�@"

	opedpl(3, 8, (ulong)attendNo, 2, 1, 0, COLOR_BLACK,  LCD_BLINK_OFF);	// �W��No.

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return 1;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return 1;
		case KEY_TEN_F3:		/* F3:�͂� */
			BUZPI();
			AteVaild_Update(attendNo, 0);								// �W��No.���폜
			return 0;
		case KEY_TEN_F4:		/* F4:������ */
			BUZPI();
			return 0;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ԏ��̏�						                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_CarFailSet( ushort Index )  	                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ushort	UsMnt_CarFailSet( ushort Index )
{
	ushort	msg;
	uchar	pos;
	uchar	fail = (uchar)(FLAPDT.flp_data[Index].car_fail & 0x01);
	uchar	disp = 0;
	ulong	posi = LockInfo[Index].posi;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, car_title_table[gCurSrtPara - MNT_INT_CAR][2]);	/* "���Ԏ��̏�iXX�j���@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[1] );										/* "�@�Ԏ��ԍ��F�@�@�@�@�@�@�@�@�@" */
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[2]  );									/* "�@���ݐݒ�F�@�@�@�@�@�@�@�@�@" */
	opedsp( 2,  12, (ushort)posi, (ushort)(posi/1000?4:((posi/100)?3:(posi/10)?2:1)), 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* �ԍ� */
	grachr( 3, 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, (fail ? CARFAIL[4]:CARFAIL[3]));					/* �̏��� */
	
	grachr( 4, 12,  8, ((fail==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[3] );						/* "�@�@�@�@�@�@�̏�Ȃ��@�@�@�@�@" */
	grachr( 5, 12,  8, ((fail==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[4] );						/* "�@�@�@�@�@�@�̏Ⴀ��@�@�@�@�@" */
	Fun_Dsp( FUNMSG[98] );																					/* "�@���@�@���@ �@�@  �ݒ�  �I�� " */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
				if( disp )
					break;
				BUZPI();
				fail ^= 1;
				grachr( 4, 12,  8, ((fail==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[3] );	/* "�@�@�@�@�@�@�̏�Ȃ��@�@�@�@�@" */
				grachr( 5, 12,  8, ((fail==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[4] );	/* "�@�@�@�@�@�@�̏Ⴀ��@�@�@�@�@" */
				break;
			case KEY_TEN_F3:					/* F3: */
				break;
			case KEY_TEN_F4:					/* F4:�ݒ� */
				if( disp )
					break;
				BUZPI();
				if( (FLAPDT.flp_data[Index].car_fail & 0x01) == fail )
					return MOD_EXT;
				
				grachr( 3, 12,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, (fail ? CARFAIL[4]:CARFAIL[3]));			/* �̏��� */
				
				if( FLAPDT.flp_data[Index].car_fail & 0x80 ){					// ��x��Ԃ�ω������������ɖ߂���
					FLAPDT.flp_data[Index].car_fail = fail;						// ����ύXBIT�𗎂Ƃ�
				}else{															// �ȑO�̏�Ԃ���ύX
					FLAPDT.flp_data[Index].car_fail = (uchar)(fail|0x80);		// ����ύXBIT���Z�b�g
				}
				if( fail == 0 ){
					wopelg( OPLOG_CCRFAIL_ON, 0, posi );		// ���엚��o�^
				}else{
					wopelg( OPLOG_CCRFAIL_OFF, 0, posi );		// ���엚��o�^
				}
				pos = (uchar)(prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[Index].ryo_syu-1)*6)),1,1 ) & 0x03);
				grachr(4, 0,  30, 0, COLOR_RED, LCD_BLINK_OFF, CARFAIL[5]);					/* "�@����  �ݒ肵�܂���  �����@�@" */
				if( prm_get(COM_PRM, S_SYS, 39, 1, 2) ){
					switch( prm_get(COM_PRM, S_SYS, 39, 1, 1) ){
						case	0:
						default:
							grachr( 5, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[6] );
							break;
						case	3:
						case	4:
							grachr( 5, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[pos+6] );
							break;
						
					}
					grachr( 6, 0,  30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[10]  );
				}else{
					dispmlclr(5,6);
				}
				Fun_Dsp( FUNMSG[8] );								/* "�@�@�@�@�@�@�@�@�@�@�@  �I�� " */
				disp = 1;	
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ԏ��̏᎞�̎Ԏ����擾		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataGet_for_Failer( void )  	                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : �Ԏ��̏��ƭ����ɂ͖{�֐����Ăяo���āA�ر����LockInfo�� |*/
/*| 		     : �Đݒ肵�āA�\�����s����悤�ɂ���B					   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	LockInfoDataGet_for_Failer( void )
{
	ushort	i;
	uchar	ret=0;
	
	for( i=0; i<LOCK_MAX; i++ ){
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( FLAPDT.flp_data[i].car_fail & 0x01 ){
			LockInfo[i] = bk_LockInfo[i];
			ret = 1;
		}
	}
	
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);								// RAM���SUM�X�V
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);							// FLASH������
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ԏ��̏᎞�̎Ԏ����擾		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataSet_for_Failer( ret_value ) 	               |*/
/*| PARAMETER    : ret_value 0:MOD_EXT 1:MOD_CHG                           |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : �Ԏ��̏��ƭ����𔲂���ꍇ�ɂ͖{�֐����Ăяo������	   |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char	LockInfoDataSet_for_Failer( char ret_value )
{

	ushort	i;
	uchar	ret=0;
	uchar	result = 0;
	char	ret2 = ret_value;
	
	for( i=0; i<LOCK_MAX; i++ ){
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if(( result = LockInfoDataSet_sub(i)) != 0 ){
			ret |= result;							// LockInfo�̍X�V����
		}

	}
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);			// RAM���SUM�X�V
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);		// FLASH������
		if(( ret & 0x02 ) == 0x02 ){
			if( LockInfoFailer_ResetMessage() == MOD_EXT ){	// ��ʏI�����I���L�[
				if( ret_value == 1 ){				// �q��ʂ���̖߂肪�����e�L�[
					ret2 = 1;						// �����e�L�[�Ƃ��Ė߂�
				}else{
					ret2 = 0;						// �I���L�[�Ƃ��Ė߂�
				}
			}else{
				ret2 = 1;							// �����e�L�[�Ƃ��Ė߂�
			}
		}
	}
	return ret2;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ԏ��̏᎞�̍ċN�����b�Z�[�W�\���֐�            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoFailer_ResetMessage( void )  	               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2010/01/07                                              |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	LockInfoFailer_ResetMessage( void )
{
	ushort	msg;

	dispmlclr(1,6);
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[11] );		/* "�̏����������ɂ͐��Z�@�́@�@" */
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[12]  );		/* "�ċN�����K�v�ł��B�@�@�@�@�@�@" */
	
	Fun_Dsp( FUNMSG2[43] );						/* "�@�@�@�@�@�@ �m�F �@�@�@�@�@�@" */

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch(msg){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F3:					/* F3:Exit */
				BUZPI();
				return MOD_EXT;
			default:
				break;
		}
	}

}

/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F�Ԏ��̏᎞�̎Ԏ����ݒ�		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataSet_sub( void )  		                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : �Ԏ��̏��ƭ����𔲂���ꍇ�ɂ͖{�֐����Ăяo������	   |*/
/*| 		     : 														   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	LockInfoDataSet_sub( ushort index )
{
	uchar	ret=0;
	
	if( FLAPDT.flp_data[index].car_fail & 0x01 ){			// �̏Ⴀ��
		bk_LockInfo[index] = LockInfo[index];
		memset( &LockInfo[index], 0, sizeof( LockInfo[0]) );
		if(index >= INT_CAR_START_INDEX && index < TOTAL_CAR_LOCK_MAX) {
			// �t���b�v�̏ꍇ�͘A�Ԃ�1�ɐݒ肵�Ă���
			LockInfo[index].lok_no = 1;
		}
		FLAPDT.flp_data[index].car_fail = 0x01;				// �ăZ�b�g
		ret = 1;
	}else{													// �̏�Ȃ�
		if( FLAPDT.flp_data[index].car_fail & 0x80 ){		// �̏Ⴀ�聨�Ȃ��ɕύX����
			LockInfo[index] = bk_LockInfo[index];
			FLAPDT.flp_data[index].car_fail = 0;			// ��ʂ̃r�b�g�𗎂Ƃ�
			ret = 2;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*|  ���d���F�Ԏ��̏᎞�̎Ԏ����ݒ�					                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockInfoDataCheck( void )  	     			           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka			    	                               |*/
/*| Date         : 2009/06/09                                              |*/
/*| REMARK       : �Ԏ��̏��ƭ����ɒ╡�d�����ꍇ�ɂ́A�N�����ɃG���A��	   |*/
/*| 		     : �X�V��Z�߂Ď��{����									   |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	LockInfoDataCheck( void )
{

	ushort	i;
	uchar	ret=0;

	for( i=0;i<LOCK_MAX; i++ ){
		WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( FLAPDT.flp_data[i].car_fail & 0x80 ){				// �Ԏ��̏�ݒ莞�ɒ�d
			if( FLAPDT.flp_data[i].car_fail & 0x01 ){			// �̏�Ȃ��˂���
				bk_LockInfo[i] = LockInfo[i];					// �Ԏ��ݒ���o�b�N�A�b�v
				memset( &LockInfo[i], 0, sizeof( LockInfo[0]) );// �Ԏ��ݒ���N���A
				if(i >= INT_CAR_START_INDEX && i < TOTAL_CAR_LOCK_MAX) {
					// �t���b�v�̏ꍇ�͘A�Ԃ�1�ɐݒ肵�Ă���
					LockInfo[i].lok_no = 1;
				}
				memset( &FLAPDT.flp_data[i], 0, sizeof( FLAPDT.flp_data[0]) );	// �Ԏ������N���A
				FLAPDT.flp_data[i].car_fail = 0x01;				// �ăZ�b�g
			}else{												// �̏Ⴀ��˂Ȃ�
				LockInfo[i] = bk_LockInfo[i];					// �o�b�N�A�b�v�f�[�^�����X�g�A
				FLAPDT.flp_data[i].car_fail = 0;				// ��ʂ̃r�b�g�𗎂Ƃ�
			}
			ret = 1;
		}
	}
	
	if( ret ){
		DataSumUpdate(OPE_DTNUM_LOCKINFO);								// RAM���SUM�X�V
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);							// FLASH������
	}
}

// �����ē����Ԑؑ֋@�\
//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F�����ē�����
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change
///							MOD_EXT : F5 key
///	@author			
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
struct GUIDE_TIME {
	short sTime[2];		// �J�n����
	short eTime[2];		// �I������
	char  volume;		// ����
};

ushort	UsMnt_VoiceGuideTime(void)
{
	ushort	pos;
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_flag = 1;
	uchar	disp_item_flag = 1;
	char volume;
	char nSet;

// MH810100(S) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�

	// ���ʐݒ萔���擾
	nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (MAX_VOICE_GUIDE < nSet) {
		// �ő�ݒ萔��3
		nSet = MAX_VOICE_GUIDE;
	}
	// ���ʎ擾
	volume = get_volume(nSet);
	
	// �ݒ萔��1�ȉ��Ȃ�I���Œ�A2�ȏ�Ȃ玞�ԑѕʂɃJ�[�\�������킹��
	pos = (nSet != 0);

	do {
		// �������ē����ԁ���ʕ\��
		if ( 1 == disp_flag){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "�������ē����ԁ��@�@�@�@�@�@�@"
			grachr(1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[130]);		// "���݂̐ݒ�F
			if (nSet == 0) {
				grachr(1, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);	// �I���Œ艹��
			} else {
				grachr(1, 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);	// ���ԑѕʉ��ʐؑ�
			}
			grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[131]);		// ���݂̉��ʁF
			opedsp(2, 12, (ushort)volume, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF); // ����
			if (volume > 0) {
				Fun_Dsp( FUNMSG[99] );	//"�@���@�@���@�e�X�g ����  �I�� "
			} else {
				Fun_Dsp( FUNMSG[25] );	//"�@���@�@���@�@�@�@ �Ǐo  �I�� "
			}
			disp_flag = 0;
		}
		if ( 1 == disp_item_flag){
			grachr(4, 6, 12, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);		// �I���Œ艹��
			grachr(5, 6, 16, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);		// ���ԑѕʉ��ʐؑ�
			disp_item_flag = 0;
		}
		
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
			Lagcan(OPETCBNO, 6);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
			return MOD_CHG;
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
			ret = MOD_EXT;
			break;
		case KEY_TEN_F4:							// ����(F4)
			BUZPI();
			if (pos == 0) {
				// �I���Œ艹��
				ret = UsMnt_VoiceGuideTimeAllDay(&volume, 0);
				if( ret == 1 ){
					CPrmSS[S_SYS][53] = 0;			// 01-0053
					CPrmSS[S_SYS][54] = 0;			// 01-0054
					CPrmSS[S_SYS][55] = 0;			// 01-0055
					CPrmSS[S_SYS][56] = 0;			// 01-0056
					ret = 0;
					nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053�E
				}
				Ope_EnableDoorKnobChime();
			} else {
				// ���ԑѕʉ��ʐؑ�
				ret = UsMnt_VoiceGuideTimeExtra(&volume);
				// �ݒ萔���m�F
				nSet = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053�E
				volume = get_volume(nSet);
			}
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
				OpelogNo = 0;
				SetChange = 1;					// FLASH���ގw��
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP�X�V�t���O
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F3:							// (F3)
			// �����𗬂�
			if (volume != 0) {		// 0�Ȃ牽�����Ȃ�
				Ope_DisableDoorKnobChime();
				// �����I���C�x���g�������̂ŁA�A���[���Ď��^�C�}�Ԃ̓A���[����}�~����
				Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
//				VoiceGuideTest(volume);	// �u�����p���肪�Ƃ��������܂����B�܂��́`�v
				lcd_volume	= Anounce_volume((uchar) volume );
				PKTcmd_audio_volume( lcd_volume );
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
			}
			break;
		case KEY_TEN_F1:							// (F1)
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// �I���E���ԑт̃J�[�\���ړ�
			pos ^= 1;
			disp_item_flag = 1;
			break;
		default:
			break;
		}
	} while (0 == ret);
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F�����ē����ԁi�I���Œ艹�ʐݒ�j
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume	���ʒl�i�[�̈�ւ̃|�C���^
//	@param[in]		ptn		0: �I���Œ艹�ʐݒ�
//							1: �p�^�[���P�i���ԑѕʉ��ʐؑցj
//							2: �p�^�[���Q�i���ԑѕʉ��ʐؑցj
//							3: �p�^�[���R�i���ԑѕʉ��ʐؑցj
///	@return			ret		MOD_CHG : mode change
///							0		: ����I��
///							1		: ����I��(�������݂���)
///	@author			Yamada
///	@note
///	-	�I���Œ艹�ʐݒ�̏ꍇ�͉��ʒl���m��i�����L�[�����j�������_�Ńp�^�[���Q,�R(�ݒ�A�h���X01-0051�̉��S��)���O�N���A����B
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_VoiceGuideTimeAllDay(char *volume, ushort ptn)
{
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_item_flag = 1;
	ushort	vol;
	long	*pvol;
	uchar	each_volume[ MAX_VOICE_GUIDE ];	/*	�p�^�[���P�`�R�̊e����
											 *	each_volume[0]:�p�^�[���P�i���ԑѕʉ��ʐؑցj�^�I���Œ艹��
											 *	each_volume[1]:�p�^�[���Q�i���ԑѕʉ��ʐؑցj
											 *	each_volume[2]:�p�^�[���R�i���ԑѕʉ��ʐؑցj
											 */
	char	f_write = 0;
	ushort	column;							// �`��ʒu
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�	
	uchar	lcd_volume;						// ����
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
	

	pvol = &CPrmSS[S_SYS][51];			// 01-0051
	// ���ݐݒ肳��Ă��鉹�ʂ��L��
	memset( each_volume, 0, sizeof(each_volume) );
	each_volume[ 0 ] = (uchar)( ( *pvol / 10000 ) % 100 );	// �p�^�[���P�̉��ʂ����o��
	each_volume[ 1 ] = (uchar)( ( *pvol / 100 ) % 100 );	// �p�^�[���Q�̉��ʂ����o��
	each_volume[ 2 ] = (uchar)( *pvol % 100 );				// �p�^�[���R�̉��ʂ����o��

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[136]);	// �����L�[�ŉ��ʂ�ݒ肵�ĉ�����
	grachr(4, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[137]);	// ���ʐݒ�F�@�@�@�@�@�@�@�@�@�@
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[138]);	// 1�@�@�@�@�@�@�@�@�@�@�@�@�@ 16

	vol = (*volume);
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	if (vol > 15) {
//		// ���ʍő�l��15
//		vol = 15;
//	}
	if (vol > ADJUS_MAX_VOLUME) {
		// ���ʍő�l��15
		vol = ADJUS_MAX_VOLUME;
	}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

	do {
		// �������ē����ԁ���ʕ\��
		if ( 1 == disp_item_flag ){
			opedsp(4, 10, vol, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// ����
			if ( vol <= 15 ) {
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor( COLOR_DARKORANGE );
// MH810100(E) S.Takahashi 2019/12/25
				column = (ushort)(vol*2);
				if(column) {
					grachr( 6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);		// �v���O���X�o�[�\��
				}
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
// MH810100(E) S.Takahashi 2019/12/25
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
			}
			if ( vol > 0) {
				Fun_Dsp( FUNMSG[100] );
			} else {
				Fun_Dsp( FUNMSG[13] );
			}
			disp_item_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm(AVM_STOP);
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if (msg == KEY_MODECHG) {
//				return MOD_CHG;
//			} else {
//				return ret;
//			}
			if (msg == KEY_MODECHG) {
				return MOD_CHG;
			} else if(msg == LCD_DISCONNECT) {
				return MOD_CUT;
			} else {
				return ret;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_TEN_F4:							// ����(F4)
			BUZPI();
			// ���ʏ�����
			f_write = 1;
			*volume = (char)vol;
			switch ( ptn ) {
			case 0:
				*pvol = (long)( (long)*volume * 10000L );
				break;
			case 1:
				*pvol = (long)(( (long)*volume * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)each_volume[ 2 ]);
				break;
			case 2:
				*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)*volume * 100 ) + (long)each_volume[ 2 ]);
				break;
			case 3:
				*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)*volume);
				break;
			default:
				*pvol = 0;	// ptn���͈͊O�̏ꍇ�͂O�ɂ��Ă���
				break;
			}

			// ���엚��o�^�EFLASH�ɃZ�[�u
			OpelogNo = OPLOG_VLSWTIME;
			f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

			if (f_write == 1 && ptn == 0) {
				CPrmSS[S_SYS][53] = 0;			// 01-0053
				CPrmSS[S_SYS][54] = 0;			// 01-0054
				CPrmSS[S_SYS][55] = 0;			// 01-0055
				CPrmSS[S_SYS][56] = 0;			// 01-0056
			}
			ret = 1;
			break;
		case KEY_TEN_F3:	/* (F3) */
			// �����𗬂�
			if (vol != 0) {		// 0�Ȃ牽�����Ȃ�
				Ope_DisableDoorKnobChime();
				// �����I���C�x���g�������̂ŁA�A���[���Ď��^�C�}�Ԃ̓A���[����}�~����
				Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
//				VoiceGuideTest((char)vol);	// �u�����p���肪�Ƃ��������܂����B�܂��́`�v
				lcd_volume = Anounce_volume( (uchar)vol );
				PKTcmd_audio_volume( lcd_volume );		// �A�i�E���X���ʂ�LCD�ɑ��M
// MH810100(E) Y.Yamauchi 2020/02/25 #3918 ���ʂ��ύX����Ȃ�
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// �v���O���X�o�[���ֈړ�
			if (vol > 0) {
				vol--;
				disp_item_flag = 1;
			}
			break;
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// �v���O���X�o�[�E�ֈړ�
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//			if (vol < 15) {
			if (vol < ADJUS_MAX_VOLUME) {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				vol++;
				disp_item_flag = 1;
			}
			break;
		default:
			break;
		}
	} while (1);

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ԑѕʂ̌v�Z
//[]----------------------------------------------------------------------[]
///	@return			ret		1	: ����I��
///							0	: ���͎����s��
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Calc_tmset_sc(ushort tNum, struct GUIDE_TIME * guide)
{
	ushort time[3];
	ushort wtime;
	ushort num;
	char i;
	char ret = 1;

	for ( i = 0; i < tNum ; ++i ){
		time[i] = guide[i].sTime[0]*60 + guide[i].sTime[1];
		if ( guide[i].sTime[1] >= 60 || time[i] >= 24*60 ){
			return 0;
		}
	}
	num = tNum;
	for ( i = 0; i < (num-1) ; ++i ){
		// 2���ȏ�̊J�n���Ԃ̍����Ȃ��ꍇ�𖵏��Ƃ݂Ȃ�
		if(time[i] + 1 >= time[i+1]){
			num = i+1;
			ret = 0;
			break;
		}
	}
	for ( i = 0; i < tNum; ++i ){
		if(i < num){
			if( i == num - 1 ){
				wtime = time[0];
			}else{
				wtime = time[i+1];
			}
			if ( 0 == wtime ){
				wtime = 24*60-1;
			} else {
				--wtime;
			}
			guide[i].eTime[0] = wtime/60;
			guide[i].eTime[1] = wtime%60;
		}
		else{
			guide[i].eTime[0] = -1;
			guide[i].eTime[1] = -1;
		}
	}
	return ret;
}


//[]----------------------------------------------------------------------[]
///	@brief			���[�U�[�����e�i���X�F�����ē����ԁi���ԑѕʉ��ʐؑ֐ݒ�j
//[]----------------------------------------------------------------------[]
///	@return			ret		1	: �I���Œ��ʂɈڍs�i���g�p,�R�����g�Q�Ɓj
///							0	: ����I��
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/12/26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_VoiceGuideTimeExtra(char *volume)
{
	ushort	msg;
	uchar	disp_flag = 1;
	ushort	pos_l = 0;
	ushort	pos_c = 0;	// 0:��,1:��
	struct	GUIDE_TIME guideTime[3];
	ushort	tNum;
	ushort	num;
	ushort	calc_flag = 0;
	ushort	i;
	char	flag;
	ushort	tmInput;
	long	*ptim;
	const char prm_pos[3] = {5, 3, 1};
	ulong	tmpvol;
	char	cl_flag = 1;
	ushort	ret;
	ptim = CPrmSS[S_SYS];
	
	// �ݒ萔�擾
	tNum = (short)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (tNum == 0) {
		tNum = 1;
	} else if (tNum > MAX_VOICE_GUIDE) {
		tNum = MAX_VOICE_GUIDE;
	}
	num = tNum;

	// ������
	memset(guideTime, 0, sizeof(struct GUIDE_TIME) * 3);

	for (i = 0; i < num; i++) {
		// �J�n���� 01-0054,01-0055,01-0056
		guideTime[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// ��
		guideTime[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// ��
		// ���� 01-0051�@�A,�B�C,�D�E
		guideTime[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);	// No  �J�n����   �I������	 ����

	tmInput = guideTime[0].sTime[0] * 100 + guideTime[0].sTime[1];
	while (1) {
		if ( 1 == disp_flag ){
			calc_flag = Calc_tmset_sc(num, guideTime);
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);														// No  �J�n����   �I������	 ����
			for( i = 0; i < MAX_VOICE_GUIDE ; ++i ){
				if( i < num ){
					if( guideTime[i].eTime[1] == -1 ){
						grachr((ushort)(2 + i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);								// �@ �|�|�F�|�| �|�|�F�|�|�@�|�|
					}
					flag = (i == pos_l) && (0 == pos_c);
					opedsp((ushort)(i + 2), 3,	(ushort)guideTime[i].sTime[0], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // �@ �����F�|�| �|�|�F�|�|�@�|�|
					grachr((ushort)(i + 2), 7,	2, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); // �R�����𔽓]
					opedsp((ushort)(i + 2), 9,	(ushort)guideTime[i].sTime[1], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // �@ �|�|�F���� �|�|�F�|�|�@�|�|
					if( guideTime[i].eTime[1] != -1 ){
						opedsp((ushort)(i + 2), 14, (ushort)guideTime[i].eTime[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// �@ �|�|�F�|�| �����F�|�|�@�|�|
						grachr((ushort)(i + 2), 18,	2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 			// �R�����𐳓]�\��
						opedsp((ushort)(i + 2), 20, (ushort)guideTime[i].eTime[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// �@ �|�|�F�|�| �|�|�F�����@�|�|
					}
					opedsp((ushort)(i + 2), 26, (ushort)guideTime[i].volume,   2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF); 			// �@ �|�|�F�|�| �|�|�F�|�|�@����
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//�����|�|�F�|�| �|�|�F�|�|�@�|�|
				}else{
					grachr((ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);									// �@ �|�|�F�|�| �|�|�F�|�|�@�|�|
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//�����|�|�F�|�| �|�|�F�|�|�@�|�|
				}
			}

			
			if( tNum > 1 && (pos_l + 1) == tNum ){
				Fun_Dsp( FUNMSG[101] );					// "�@���ʁ@�@���@�@ �폜 �@�����@�@�I���@"
			}else{
				Fun_Dsp( FUNMSG[102] );					// "�@���ʁ@�@���@�@�@�@�@�@�����@�@�I���@"
			}
			disp_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
			return 0;
		case KEY_TEN_F4:							// ����(F4)
			if ( 1 == calc_flag ){
				// ���Ԃ�������
				ptim[54 + pos_l] = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
				if( tNum < pos_l + 1 ){
					tNum = pos_l + 1;
				}
				ptim[53] = tNum;

				// ���ʂ̌p�����̒l�̂܂܁u�����v�����ꍇ�ɂ́A���̉��ʂŊm�肳����
				switch (pos_l) {
				case 0:
					tmpvol = ptim[51] % 10000;
					tmpvol += (ulong)(guideTime[pos_l].volume) * 10000;
					break;
				case 1:
					tmpvol  = ptim[51] / 10000;
					tmpvol *= 10000;
					tmpvol += ptim[51] % 100;
					tmpvol += (ulong)(guideTime[pos_l].volume) * 100;
					break;
				case 2:
					tmpvol  = ptim[51] / 100;
					tmpvol *= 100;
					tmpvol += guideTime[pos_l].volume;
					break;
				default:
					tmpvol = 0;
					break;
				}
				ptim[51] = tmpvol;
				// �m�肵���Ƃ��ɂ͈�O�̉��ʂ��p�����ĕ\��
				if ( pos_l < MAX_VOICE_GUIDE ){
					if( !ptim[54 + pos_l + 1] )								// ���̊J�n���������ݒ�̏ꍇ
					guideTime[pos_l+1].volume = guideTime[pos_l].volume;
				}
				// ���엚��o�^�EFLASH�ɃZ�[�u
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;									// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				goto UsMnt_VoiceGuide_CUR_DWN;	// �J�[�\���ړ�
			}else{
				// �������s���ł���΁APIPI���̌�A�\����߂�
				BUZPIPI();
				// �J�n���� 01-0054,01-0055,01-0056
				guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
				guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
				tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
				disp_flag = 1;
				break;
			}
		case KEY_TEN_F2:							// (F2)
			// �ŐV�̉��ʐݒ����ʂɔ��f������
			guideTime[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			if ( pos_l+1 < MAX_VOICE_GUIDE ){
				if( !ptim[54 + pos_l + 1] )									// ���̊J�n���������ݒ�̏ꍇ
					guideTime[pos_l+1].volume = guideTime[pos_l].volume;	// ��O�̐ݒ���p��
			}
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
UsMnt_VoiceGuide_CUR_DWN:
			BUZPI();
			
			++pos_l;
			if(pos_l < MAX_VOICE_GUIDE) {
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
			}
			disp_flag = 1;
			pos_c = 0;
			if ( pos_l > tNum || pos_l >= 3 ){
				pos_l = 0;
				if(num > 1 && tNum < 3 ){
					num--;
				}
			} else {
				if ( tNum <= pos_l ){
					num++;
				}
			}
			tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
			cl_flag = 1;
			break;		
		case KEY_TEN_F3:	// (F3)
			// �ݒ�폜(�J�n�����A���ʂ��N���A�A�ݒ萔��-1)
			if( tNum > 1 && (pos_l + 1) == tNum ){
				tNum--;
				ptim[53] = tNum;
				ptim[54 + pos_l] = 0;
				switch (pos_l) {
				case 0:
					tmpvol = ptim[51] % 10000;
					break;
				case 1:
					/*
					 *	@note	�p�^�[���Q���폜�������Ƀp�^�[���R�̉��ʒl���c���Ă����Ȃ��悤�ɂ���B�i�O�N���A����j
					 */
					tmpvol = ( ptim[51] / 10000 ) * 10000;
					break;
				case 2:
					tmpvol  = ptim[51] / 100;
					tmpvol *= 100;
					break;
				default:
					tmpvol = 0;
					break;
				}
				ptim[51] = tmpvol;
				// ���엚��o�^�EFLASH�ɃZ�[�u
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				goto UsMnt_VoiceGuide_CUR_DWN;
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// ���ʐݒ��ʂ�
			ret = UsMnt_VoiceGuideTimeAllDay(&guideTime[pos_l].volume, (ushort)(pos_l + 1));
			*volume = guideTime[pos_l].volume;
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
				OpelogNo = 0;
				SetChange = 1;					// FLASH���ގw��
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
			}
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if( ret == MOD_CUT){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			for (i = 1; i < 7; i++) {
				displclr(i);
			}
			disp_flag = 1;
			break;
		case KEY_TEN:								// ����(�e���L�[)
			// ���Ԑݒ�
			BUZPI();
			if( cl_flag ){
				tmInput = 0;
				cl_flag = 0;
			}
			tmInput = (tmInput % 1000) * 10 + msg - KEY_TEN0;
			guideTime[pos_l].sTime[0] = tmInput / 100;
			guideTime[pos_l].sTime[1] = tmInput % 100;
			disp_flag = 1;
			break;
		case KEY_TEN_CL:							// ���(�e���L�[)
			BUZPI();
			// �J�n���� 01-0054,01-0055,01-0056
			guideTime[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
			tmInput = guideTime[pos_l].sTime[0] * 100 + guideTime[pos_l].sTime[1];
			// ���� 01-0051�@�A,�B�C,�D�E
			guideTime[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			disp_flag = 1;
			cl_flag = 1;
			break;
		default:
			break;
		}
	}
	
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ԑѕʂ̉��ʎ擾
//[]----------------------------------------------------------------------[]
///	@return			vol		:	����
///	@param[in]		num		:	�ݒ萔
///	@author			Yamada
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_volume(char num)
{
	char ptn, i, vol, pos;
	short nowtime, settime1, settime2;
	
	nowtime = (short)CLK_REC.hour * 100 + (short)CLK_REC.minu;	// ���ݎ���
	// �J�n���Ԃ̃p�^�[�����擾
	if (num <= 1) {
		ptn = 1;				// �ݒ萔��1�ȉ��̓p�^�[��1
	} else {
		for (i = 0; i < num - 1; i++) {
			// ���ݎ������ݒ�p�^�[��i��i+1�̊Ԃɂ��邩���肷��
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054�`
			if (settime1 <= nowtime) {
				settime2 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 55), 4, 1);
				if (settime1+1 < settime2){
					if (nowtime < settime2) {
						ptn = (char)(i + 1);
						break;
					}
				}else{
					ptn = (char)(i + 1);
					break;
				}
			}
		}
		if (i == num - 1) {
			ptn = num;
		}
	}
	// ���ʎ擾
	switch (ptn) {
	case 1:	pos = 5;	break;
	case 2:	pos = 3;	break;
	case 3:	pos = 1;	break;
	default:			return 0;
	}
	vol = (char)prm_get(COM_PRM, S_SYS, 51, 2, pos);
	
	return vol;
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����̐ݒ���擾���� 				       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_Search_Ifno	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2009-07-24                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	GetCarInfoParam( void )
{
	uchar	param[2]={0};
	uchar	ret=0;

	param[0] = (uchar)prm_get(COM_PRM,S_TYP,100,1,1);
	param[1] = (uchar)prm_get(COM_PRM,S_TYP,100,1,2);
	
	if( param[0] )				// ���֕��݂���H
		ret = 0x01;

	if(param[1]){
		ret |= 0x04;			// ���ԁi�����j����H
	}
	return ret;
}
#if (5 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| �����v�Z�G���A���̃f�[�^�󎚏��� 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_mc10print  	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : sugata                                                 |*/
/*| Date         : 2010-08-25                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
unsigned short UsMnt_mc10print(void)
{
	ushort msg,dat;
	int		inji_end;
	int		can_req;
	T_FrmChargeSetup	FrmChargeSetup;
	T_FrmPrnStop		FrmPrnStop;

	dispclr();

	grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[0]);		// "�������ݒ聄�@�@�@�@�@�@�@�@�@"
	grachr(2,  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[0]);		// ""
	opedsp(2, 2, 1, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);
	Fun_Dsp( FUNMSG[96] );
	inji_end = 1;
	dat = 1;
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		if (msg == (INNJI_ENDMASK|PREQ_CHARGESETUP)) {
			inji_end = 1;
			msg &= (~INNJI_ENDMASK);
		}
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:		/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F3:
			if (inji_end == 0) {
				BUZPI();
				if (can_req == 0) {
				// �󎚒��~�v��
					can_req = 1;
					FrmPrnStop.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
				}
			}
			else{
				if(dat == 0){
					BUZPIPI();
					break;
				}
				FrmChargeSetup.Kikai_no = (uchar)CPrmSS[S_PAY][2];						// �@�B��
				FrmChargeSetup.NowTime.Mon = 0xff;							// mc10�󎚃t���O�Ƃ��Ďg�p
				FrmChargeSetup.NowTime.Year = dat;							// �����̌n�w��
				FrmChargeSetup.prn_kind = R_PRI;
				queset( PRNTCBNO, PREQ_CHARGESETUP, sizeof(T_FrmChargeSetup), &FrmChargeSetup );// �����ݒ��
				inji_end = 0;
				can_req = 0;
				grachr(3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "�@�@�@ �v�����g���ł� �@�@�@�@"
				Fun_Dsp(FUNMSG[82]);													// "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
			}
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			if (inji_end != 0) {
				BUZPI();
				dat = msg-KEY_TEN0;
				opedsp(2, 2, dat, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);
			}
			break;
		case PREQ_CHARGESETUP:
			if (inji_end == 1) {
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);;
				Fun_Dsp( FUNMSG[96] );
			}
			break;
		default:
			break;
		}
		
	}

}
#endif
//[]----------------------------------------------------------------------[]
///	@brief	���[�U�[�����e�i���X�F�R���g���X�g����
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	UsColorLCDLumine(void)
{

	ushort	msg;
	uchar	disp_flag;
	long	blgt;
	char	changing;
	long	data;
	ushort	tim1, tim2;
	ushort	set;
// MH810100(S) S.Takahashi 2020/02/10 #3848 [�I��]��2�񉟉����Ȃ��ƋP�x������ʂɑJ�ڂ��Ȃ�
	ushort ret = 0;
// MH810100(E) S.Takahashi 2020/02/10 #3848 [�I��]��2�񉟉����Ȃ��ƋP�x������ʂɑJ�ڂ��Ȃ�
	char	off_flag;

	disp_flag = 1;
	changing = 0;
	for ( ; ; ) {
		if (1 == disp_flag){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0] );		// "���P�x�������@�@�@�@�@�@�@�@�@"
			grachr(1, 2, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_3[1]);					// "���݂̏��"
			grachr(1, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);					// "�F" 
			grachr(2, 2, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[10]);		// "���݂̋P�x�F"

			blgt = CPrmSS[S_PAY][29] % 10;
			if (blgt == 0) {
				grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);				// "�펞�_��"
				off_flag = 0;															// ON
			} else if (blgt == 1) {
				grachr(1, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);			// "�e���L�[����"
				off_flag = 0;															// ON
			} else {
				grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);			// "�鎞�ԑ�"
				off_flag = 0;															// OFF
				set = (ushort)CPrmSS[S_PAY][30];
				tim1 = ( set / 100 ) * 60 + ( set % 100 );
				set = (ushort)CPrmSS[S_PAY][31];
				tim2 = ( set / 100 ) * 60 + ( set % 100 );
				if( tim1 == tim2 ){														// ��Ԏ��ԑтȂ�
					off_flag = 0;														// 
				}else if( tim1 < tim2 ){
					if(( tim1 <= CLK_REC.nmin )&&( CLK_REC.nmin <= tim2 )){				// ��Ԏ��ԑ�
						off_flag = 1;													// 
					}
				}else{
					if(( CLK_REC.nmin >= tim1 )||( tim2 >= CLK_REC.nmin )){				// ��Ԏ��ԑ�
						off_flag = 1;													// 
					}
				}
			}
			opedsp(2, 16, (ushort)prm_get(COM_PRM, S_PAY, (short)(27+off_flag), 2, 1), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// �P�x�l(02-0027�D�E/02-0028�D�E)
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27+off_flag, 2, 1));	// �_�����̋P�x
			grachr(4, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);					// "�펞�_��" */
			grachr(4, 14, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);				// "�e���L�[����"
			grachr(5, 4, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);					// "�鎞�ԑ�"
			Lumine_time_dsp(1, CPrmSS[S_PAY][30], 0);
			grachr(6, 14, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_3[8]);				// "�`"
			Lumine_time_dsp(2, CPrmSS[S_PAY][31], 0);
			Lumine_dsp(blgt, 1);
			Fun_Dsp( FUNMSG2[48] );														// "�@���@�@���@ �����@����  �I�� "
			disp_flag = 0;
		}
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:	/* �I��(F5) */
			LcdBackLightCtrl( ON );							// back light ON
			BUZPI();
			return MOD_EXT;
		case KEY_TEN_F1:	/* ��(F1) */
		case KEY_TEN_F2:	/* ��(F2) */
			if (changing)
				break;
			BUZPI();
			Lumine_dsp(blgt, 0);
			if (msg == KEY_TEN_F1) {
				if (blgt == 3) {
					blgt -= 2;
				} else {
					blgt -= 1;
				}
				if (blgt < 0)
					blgt += 4;
			}
			else {
				if (blgt == 1) {
					blgt += 2;
				} else {
					blgt += 1;
				}
				if (blgt >= 4)
					blgt -= 4;
			}
			Lumine_dsp(blgt, 1);
			break;
		case KEY_TEN_F3:	// ����(F3)
			if (changing){
				break;
			}
			BUZPI();
			// �P�x�l�ύX
// MH810100(S) S.Takahashi 2020/02/10 #3848 [�I��]��2�񉟉����Ȃ��ƋP�x������ʂɑJ�ڂ��Ȃ�
//			if (UsColorLCDLumine_change(off_flag) == MOD_CHG ) {
//				return MOD_CHG;
//			}
//// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if ( UsColorLCDLumine_change(off_flag) == LCD_DISCONNECT){
//				return MOD_CUT;
//			}
//// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			ret = UsColorLCDLumine_change(off_flag);
			if (ret == MOD_CHG || ret == MOD_CUT) {
				return ret;
			}
// MH810100(E) S.Takahashi 2020/02/10 #3848 [�I��]��2�񉟉����Ȃ��ƋP�x������ʂɑJ�ڂ��Ȃ�
			LcdBackLightCtrl( ON );							// back light ON
			disp_flag = 1;
			break;
		case KEY_TEN_F4:	/* ����(F4) */
			if (changing == 0) {
				BUZPI();
				CPrmSS[S_PAY][29] = CPrmSS[S_PAY][29]/10*10 + blgt;
				grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_5[2]);				/* "�@�@" */
				if (blgt == 0)
					grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);			/* "�펞�_��" */
				else if (blgt == 1)
					grachr(1, 16, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
				else {
					grachr(1, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */
					Lumine_dsp(blgt, 0);
					Lumine_time_dsp(1, CPrmSS[S_PAY][30], 1);
					Fun_Dsp(FUNMSG2[1]);		/* "�@�@�@�@�@�@�@�@�@ ����  �I�� " */
					changing = 1;
					data = -1;
				}
				OpelogNo = OPLOG_BACKLIGHT;		// ���엚��o�^
				f_ParaUpdate.BIT.other = 1;		// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL );	// FTP�X�V�t���O�Z�b�g
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
			}
			else {
				if (data != -1) {
					if (data%100 > 59 || data > 2359) {
						BUZPIPI();
						Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
						data = -1;
						break;
					}
					CPrmSS[S_PAY][29+changing] = data;
				}
				BUZPI();
				Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 0);
				if (changing == 1) {
					changing = 2;
					Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
					data = -1;
				}
				else {
					Lumine_dsp(blgt, 1);
					Fun_Dsp( FUNMSG2[48] );												// "�@���@�@���@ �����@����  �I�� "
					changing = 0;
					disp_flag = 1;
				}
			}
			break;
		case KEY_TEN:		/* ����(�e���L�[) */
			if (changing == 0)
				break;
			BUZPI();
			if (data == -1)
				data = 0;
			data = (data*10 + msg-KEY_TEN0) % 10000;
			Lumine_time_dsp(changing, data, 1);
			break;
		case KEY_TEN_CL:	/* ���(�e���L�[) */
			BUZPI();
			if (changing == 0) {
				Lumine_dsp(blgt, 0);
				blgt = CPrmSS[S_PAY][29] % 10;
				Lumine_dsp(blgt, 1);
			}
			else {
				Lumine_time_dsp(changing, CPrmSS[S_PAY][29+changing], 1);
				data = -1;
			}
			break;
		default:
			break;
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief	���[�U�[�����e�i���X�F�R���g���X�g����
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static	ushort	UsColorLCDLumine_change(char off_flag)
{
	ushort	msg;
	int		lumine_change = 0;
	uchar	side;								// 0:�_��(���Z����鎞�ԑ�)�C1:����(�ҋ@��������ԑ�)
	uchar	mode;
	unsigned short	lumine_num[2];				// [0]:�_�����̋P�x�C[1]:�������̋P�x
	int		write_flg = 0;
	int		mode_change = 0;

	side = off_flag;
 	mode = (uchar)prm_get(COM_PRM, S_PAY, 29, 1, 1);			// 02-0029�E
	lumine_num[0] = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027�D�E
	lumine_num[1] = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028�D�E

	BUZPI();
	dispclr();
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[7]);		// "�����L�[�ŋP�x��ݒ肵�ĉ�����"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[8]);		// "�P�x�ݒ�F�@�@�@�@�@�@�@�@�@�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[9]);		// "0 �@�@�@�@�@�@�@�@�@�@�@�@�@15"
	if (mode > 0) {
		Fun_Dsp(FUNMSG2[48]);						// "�@���@�@���@ �ؑ�  ����  �I�� "
	} else {
		Fun_Dsp(FUNMSG[13]);						// "�@���@�@���@�@�@�@ ����  �I�� "
	}
	lumine_change = 1;


	while (1) {
		msg = StoF(GetMessage(), 1);			// ү���ގ�M

		switch (msg) {
		case KEY_TEN_F5:						// �I��
			BUZPI();
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// ���엚��o�^
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// �_�����̋P�x
			return MOD_EXT;
			break;

		case KEY_TEN_F1:						// ��
			BUZPI();
			// �v���O���X�o�[���ֈړ�
			if (lumine_num[side] > 0) {
				lumine_num[side]--;
				lumine_change = 1;
			}
			break;

		case KEY_TEN_F2:						// ��
			BUZPI();
			// �v���O���X�o�[�E�ֈړ�
			if (lumine_num[side] < LCD_LUMINE_MAX) {
				lumine_num[side]++;
				lumine_change = 1;
			}
			break;

		case KEY_TEN_F3:						// �ؑ�
			if (mode > 0) {
				BUZPI();
				if (side) {
					side = 0;
				} else {
					side = 1;
				}
				lumine_change = 1;
			}
			break;
		case KEY_TEN_F4:						// ����
			BUZPI();
			CPrmSS[S_PAY][27 + side] = lumine_num[side];
			write_flg = 1;
			break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			mode_change = 2;	// �u���샂�[�h�ؑփC�x���g��M�ς݁v�Ƃ���
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// ���엚��o�^
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// �_�����̋P�x
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			mode_change = 1;	// �u���샂�[�h�ؑփC�x���g��M�ς݁v�Ƃ���
			if (write_flg) {
				OpelogNo = OPLOG_CONTRAST;		// ���엚��o�^
			}
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));	// �_�����̋P�x
			break;
		default:
			break;
		}

		if (lumine_change) {
			lcd_contrast((unsigned char)lumine_num[side]);
			backlight = lumine_num[side];
			switch (mode) {
			case 1:		// �e���L�[���͂ɂ��_��
			case 3:		// ��Ԏ��ԑт̂ݏ펞�_��(���Ԃ̓e���L�[���͂ɂ��_��)
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[mode+side]);	// "���P�x�������@�@�@�@�i�w�w�w�j"
				break;
			default:	// �펞�_��
				grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0]);			// "���P�x�������@�@�@�@�@�@�@�@�@"
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}
			opedsp(4, 10, (ushort)lumine_num[side], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);		// �P�x
			if (lumine_num[side] < LCD_LUMINE_MAX) {
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor(COLOR_DARKORANGE);			// �J���[LCD�ւ̔w�i�F�\��
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				grachr( 6 , 0, (ushort)((lumine_num[side]*2)), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);	// �v���O���X�o�[�\��
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor(COLOR_WHITE);					// �J���[LCD�ւ̔w�i�F�\��
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				grachr(6, (ushort)((lumine_num[side]*2)+1), 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
			}else{
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor(COLOR_DARKORANGE);			// �J���[LCD�ւ̔w�i�F�\��
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				grachr( 6 , 0, 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);								// �v���O���X�o�[�\��
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor(COLOR_WHITE);					// �J���[LCD�ւ̔w�i�F�\��
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				lumine_num[side] = LCD_LUMINE_MAX;
			}
			lumine_change = 0;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if ( mode_change != 0 ) {
		if ( mode_change == 1 ) {
			if (write_flg) {
				wopelg(OPLOG_CONTRAST,0,0);			// ���엚��o�^
			}
			return MOD_CHG;
		}
		if ( mode_change == 2 ) {
			if (write_flg) {
				wopelg(OPLOG_CONTRAST,0,0);			// ���엚��o�^
			}
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}
}

//[]----------------------------------------------------------------------[]
///	@brief	���[�U�[�����e�i���X�F�R���g���X�g�������@�\�� 
//[]----------------------------------------------------------------------[]
///	@param[in]		data : �_�����
///	@param[in]		rev  : 0:���] 1:���]   
///	@return			void   
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	Lumine_dsp(long data, ushort rev)
{
	data = data%10;
	if (data == 0) {
		grachr(4, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[8]);		/* "�펞�_��" */
	}
	else if (data == 1) {
		grachr(4, 14, 12, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT6_1[2]);		/* "�e���L�[����" */
	}
	else {
		grachr(5, 4, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[10]);		/* "�鎞�ԑ�" */

	}
}
//[]----------------------------------------------------------------------[]
///	@brief	���[�U�[�����e�i���X�F�R���g���X�g�������@�\��2
//[]----------------------------------------------------------------------[]
///	@param[in]		chng : 1:�J�n���� 2:�I������
///	@param[in]		time : ����
///	@param[in]		rev  : 0:���] 1:���]      
///	@return			void   
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/15
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	Lumine_time_dsp(char chng, long time, ushort rev)
{
	chng -= 1;
	opedsp(6, (ushort)(4+(chng*12)), (ushort)(time/100), 2, 0, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
	grachr(6, (ushort)(8+(chng*12)), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);				/* "�F" */
	opedsp(6, (ushort)(10+(chng*12)), (ushort)(time%100), 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ԏؖ����ݒ�					 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkingCertificateSet( void )                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static unsigned short	UsMnt_ParkingCertificateSet(void)
{
	char	val;		// 03-0062�B
	int		pos;		// same as num
	char	max = 1;
	short	in_now = -1;
	ushort	len;
	ushort	num;
	long	param;
	ushort	msg;
	static	const	uchar	setting[] = {2, 1, 0};

	num = (ushort)prm_get(COM_PRM, S_TYP, 62, 1, 3);		// 03-0062�C
	pos = (num == 0)? 0 : 1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[43]);		// "�����ԏؖ������s�ݒ聄"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[10]);		/* "�@���݂̏�ԁ@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		/* "�@�@�@�@�@�@�F�@�@�@�@�@�@�@�@" */
	len = (ushort)strlen(P_CERTI_SEL[0]);
	grachr( 4, 14, len, ((pos==0)?1:0), COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[0] );
	len = (ushort)strlen(P_CERTI_SEL[1]);
	grachr( 5, 14, len, ((pos==1)?1:0), COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[1] );

	len = (ushort)strlen(P_CERTI_SEL[pos]);
	grachr( 2, 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos] );

	if (pos != 0) {
	// ���s�\����(���݂̏�Ԃ�"����"�ݒ�̂ݕ\��)
		grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[44] );	/* "  ���s�\�����F  ���i�P�`�X�j" */
		opedsp( 6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
		max = 2;
	}

	Fun_Dsp( FUNMSG[20] );					/* "�@���@�@���@�@�@�@ ����  �I�� " */

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (KEY_TEN0to9(msg)) {
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
			if (pos == 2) {
			// ���s�\����
				opedsp(6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
				in_now = -1;
			} else {
				grachr((ushort)(4+pos), 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos]);
			}
			if (msg == KEY_TEN_F1) {
				if (--pos < 0) {
					pos = max;
				}
			}
			else {
				if (++pos > max) {
					pos = 0;
				}
			}
			if (pos == 2) {
			// ���s�\����
				opedsp(6, 16, num, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			} else {
				len = (ushort)strlen(P_CERTI_SEL[pos]);
				grachr((ushort)(4+pos), 14, len, 1, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos]);
			}
			break;
		case KEY_TEN_F4:					/* F4:���� */
			BUZPI();
			if (pos == 2) {
			// ���s�\����
				if (in_now != -1) {
				// ���͒l����
					param = (prm_get(COM_PRM, S_TYP, 62, 1, 4) * 1000);		// 03-0062�B
					param += (in_now * 100);								// ���͒l
					param += (prm_get(COM_PRM, S_TYP, 62, 1, 2) * 10);		// 03-0062�D
					param += prm_get(COM_PRM, S_TYP, 62, 1, 1);				// 03-0062�E
					CPrmSS[S_TYP][62] = param;
					num = (ushort)in_now;
					in_now = -1;
				}
			} else {
				if (pos == 1) {
				// "����"�����s�\�����\��(7�s��)
					if (num != 0) {
					// "����"��"����"�Ȃ�ύX�Ȃ�(���O�o�^�͍s��)
						OpelogNo = OPLOG_SHOUMEISETTEI;
						break;
					}
					num = 2;
					grachr( 2, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[6] );						/* "  " */
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[44] );						/* "  ���s�\�����F  ���i�P�`�X�j" */
					opedsp( 6, 16, num, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
					val = setting[prm_get(COM_PRM, S_TYP, 62, 1, 4)];	// 03-0062�B�̐ݒ�ɂ����
					CPrmSS[S_ANA][6] = 195153;	// 25-0006�u���ԏؖ������K�v�ȕ��́A���ԏؖ����L�[�������ĉ������B���Z�������́A���Z�L�[�������ĉ������B�v�u�����͎���L�[�������ĉ������B�v
					CPrmSS[S_ANA][7] = 000000;	// 25-0007�u(�Ȃ�)�v
					CPrmSS[S_ANA][8] = 000000;	// 25-0008�u(�Ȃ�)�v
					max = 2;
				} else {
				// "���Ȃ�"�����s�\�����\���폜(7�s��)
					if (num == 0) {
					// "���Ȃ�"��"���Ȃ�"�Ȃ�ύX�Ȃ�(���O�o�^�͍s��)
						OpelogNo = OPLOG_SHOUMEISETTEI;
						break;
					}
					num = 0;
					displclr(6);
					val = pos;
					CPrmSS[S_ANA][6] = 141153;	// 25-0006�u�Ԏ��ԍ����m�F���ėǂ���ΐ��Z�L�[�������ĉ������B�v�u�����͎���L�[�������ĉ������B�v
					CPrmSS[S_ANA][7] = 000000;	// 25-0007�u(�Ȃ�)�v
					CPrmSS[S_ANA][8] = 000000;	// 25-0008�u(�Ȃ�)�v
					max = 1;
				}
				grachr( 2, 14, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)P_CERTI_SEL[pos] );
				param = (prm_get(COM_PRM, S_TYP, 62, 1, 4) * 1000);		// 03-0062�B
				param += (num * 100);									// 03-0062�C
				param += (prm_get(COM_PRM, S_TYP, 62, 1, 2) * 10);		// 03-0062�D
				param += val;											// 03-0062�E
				CPrmSS[S_TYP][62] = param;
			}
			OpelogNo = OPLOG_SHOUMEISETTEI;
			SetChange = 1;
			break;
		case KEY_TEN:						/* �e���L�[: */
			if (pos < 2) {
				break;
			}
			if (msg != KEY_TEN0) {
			// KEY0�͎󂯕t���Ȃ�
				BUZPI();
				in_now = (short)(msg - KEY_TEN0);
				// ���͒l��\������i���]�\���j
				opedsp(6, 16, (ushort)in_now, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			}
			break;
		case KEY_TEN_CL:					/* C: */
			if (pos < 2) {
				break;
			}
			BUZPI();
			opedsp(6, 16, num, 1, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
			in_now = -1;
			break;
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ԏؖ������j���[				 		       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : UsMnt_ParkingCertificate( void )                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned short	UsMnt_ParkingCertificate(void)
{
	unsigned short	usUmnyEvent;
	char	wk[2];
	char	cnt;

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR1[37]);		//	"�����ԏؖ�����"

		cnt = (prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 )? 1 : 2;
		usUmnyEvent = Menu_Slt( PARKING_CERTIFICATE_MENU, USM_CERTI_TBL, cnt, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUmnyEvent ){
		case MNT_CERTI_SET:
			usUmnyEvent = UsMnt_ParkingCertificateSet();
			break;

		case MNT_CERTI_ISSU:
			usUmnyEvent = UsMnt_AreaSelect(MNT_UKERP);
			break;

		case MOD_EXT:
			OPECTL.Mnt_mod = 1;
			return MOD_EXT;

		default:
			break;
		}

		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			OpelogNo = 0;
		}

		if (usUmnyEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CHG;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���͓��t�̃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[]		event	: 0:���ʓ��^���ʊ��� 1:������
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			matsushima
///	@date			Create	: 2014/11/07<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	UsMnt_datecheck( uchar event )
{

	switch( event ){
	case 0:
		splday_check();							// ���ʓ��^���ʊ��Ԃ̓��͓��t�̃`�F�b�N
		break;

	case 1:
		tickvalid_check();						// �������̓��͓��t�̃`�F�b�N
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------------------
// ���͓��t�̃`�F�b�N���s���ANG�̏ꍇ�͍폜����B
//--------------------------------------------------------------------------
static void tickvalid_check( void )
{

	TIC_PRM	*ptic;
	TIC_PRM	tic;
	char	no;				/* �����ԍ�(1-3) */

	ptic = (TIC_PRM *)&CPrmSS[S_DIS][8];
	memcpy( &tic, &tick_valid_data.tic_prm, sizeof(TIC_PRM) );
	no = tick_valid_data.no;

	if(( no < 1)||( no > 3 )){
		return;				// �����ԍ� �͈͊O
	}

	// �������̓��t�`�F�b�N
	if( NG == ticval_date_chk( tic.date[0] ) ){		// �ύX�O�J�n���t����
		tic.date[0] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[1] ) ){		// �ύX�O�I�����t����
		tic.date[1] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[2] ) ){		// �ύX��J�n���t����
		tic.date[2] = 0;
	}
	if(	NG == ticval_date_chk( tic.date[3] ) ){		// �ύX��I�����t����
		tic.date[3] = 0;
	}

	// �p�����[�^�X�V
	ptic[no-1] = tic;

}

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
/*[]----------------------------------------------------------------------[]*/
/*| �@�@�@�@�@�@���[�_�����e�i���X�f�[�^���M�i���ʃe�X�g�j			       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ECVoiceGuide_VolumeTest				                   |*/
/*| PARAMETER    : vol		: ����					                       |*/
/*| RETURN VALUE : ret  	: 1 (���s�����ǂ���Ƃ�)                       |*/
/*| 					  	: MOD_CHG			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-06-13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static ushort ECVoiceGuide_VolumeTest( uchar vol ){

	ushort		msg;
	ushort		ret = 1;
	uchar		Receiveflag = 0;	// ���ʎ�M�t���O�@0 = ��M���A�@1 = ��MOK
	uchar		funflag = 0;		// �t�@���N�V�����L�[�\���t���O
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar		lcd_flag = 0;		// �ؒf���m�p�̃t���O
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
	// ���σ��[�_������OK�H(���σ��[�_�N���V�[�P���X�N���ςłȂ���΃R�}���h�͑��M���Ȃ�)
	if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
		return ret;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[179]);	// " �Đ����ł��B�@�@�@�@�@�@�@�@ "

	// ������
	memset(&MntTbl, 0x0, sizeof(EC_MNT_TBL));

	MntTbl.cmd = 1;						// ���ʃe�X�g�Z�b�g
	MntTbl.vol = (uchar)(vol);			// ���ʃZ�b�g
	if(MntTbl.vol > EC_MAX_VOLUME) {
		MntTbl.vol = EC_MAX_VOLUME;
	} 

	// ���[�_�����e�i���X�f�[�^���M�i�����e�X�g�j
	Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
	// �^�C���A�E�g�ݒ�
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	for( ; ; ) {
		// ���ʎ�M�҂����͕\�����Ȃ�
		if(Receiveflag && funflag){
			Fun_Dsp( FUNMSG[8] );					//"                    �@ �I��  "
			funflag = 0;
		}

		msg = StoF(GetMessage(), 1);
		
		switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:							// �I��(F5)
				// ���[�_����ԓ��܂��́A�^�C���A�E�g����܂ŏI�������Ȃ�
				if(Receiveflag){
					BUZPI();
					Lagcan(OPETCBNO, 6);
					return ret;
				}
				break;
			case EC_MNT_UPDATE:
				if(!Receiveflag){
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
					Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
					Receiveflag = 1;
					funflag = 1;
					// �����e�X�g�łȂ����A�������Ȃ��B
					if( RecvMntTbl.cmd != 1){
						BUZPIPI();
						grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[178]);	// "  �����Đ��Ɏ��s���܂�������  "
						break;
					}
					grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[177]);	// "  �����Đ��ɐ������܂�������  "
				}
				break;
			case EC_CMDWAIT_TIMEOUT:
				if(!Receiveflag){
					Receiveflag = 1;
					funflag = 1;
					BUZPIPI();
					grachr(4, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[178]);	// "  �����Đ��Ɏ��s���܂�������  "
				}
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				lcd_flag = 1;
				break;
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
		// �ؒf�҂����A��M�ς�
		if( lcd_flag && Receiveflag ){
			Lagcan( OPETCBNO, 6 );
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �@�@�@�@�@�@			���ʕύX�f�[�^���M					           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : VoiceGuide_VolumeChange				                   |*/
/*| PARAMETER    : void			     				                       |*/
/*| RETURN VALUE : ret	: 1 = ���s                                         |*/
/*|						: 0 = �����@�i����A���ғ��������j                 |*/
/*|						: 2 = ���������@�R�}���h���M���Ȃ�                 |*/
/*|						: 9 = �ؒf�ʒm                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-03-1                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static char VoiceGuide_VolumeChange( void ){

	ushort		msg;
	char		ret = 0;
	uchar		loop = 0;

	// ���σ��[�_������OK�H(���σ��[�_�N���V�[�P���X�N���ςłȂ���΃R�}���h�͑��M���Ȃ�)
	if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
		BUZPI();
		ret = 2;
		return ret;
	}

// MH810103 GG119202(S) ���ʕύX���̕\���C��
	// ���ʕύX����8�s�ڂ��N���A����
	Fun_Dsp( FUNMSG[0] );	// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
// MH810103 GG119202(E) ���ʕύX���̕\���C��

	// ���ʕύX�f�[�^�Z�b�g
	SetEcVolume();

	// ���ʕύX�f�[�^���M
	Ec_Pri_Data_Snd( S_VOLUME_CHG_DATA, 0 );
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */
	loop = 1;

	// �ԓ����^�C���A�E�g������܂ő҂�
	while(loop){
		msg = StoF(GetMessage(), 1);

		switch (KEY_TEN0to9(msg)) {
			case EC_VOLUME_UPDATE:
				// 0�F�ύXOK�A1�F�ύXNG
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if(!RecvVolumeRes){
				if(!ECCTL.RecvVolumeRes){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					BUZPI();
					ret = 0;
				} else {
					BUZPIPI();
					ret = 1;
				}
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
				loop = 0;
				break;
			case EC_CMDWAIT_TIMEOUT:
				BUZPIPI();
				ret = 1;
				loop = 0;
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = 9;
				loop = 0;
				break;
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
			default:
				break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			�����ē����ԃ��j���[
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change
//							MOD_EXT : F5 key
//	@author			Inaba
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]

struct GUIDE_TIME_EC {
	struct GUIDE_TIME gt[3];
	char  volume_reader[3];		// ���[�_����
};

ushort	UsMnt_ECVoiceGuide(void)
{
	ushort	pos;
	ushort	msg;
	char 	ptn;							// ���ʐݒ�p�^�[����
	ushort	ret = 0;
	char	str_vol[15];					// ���ʕ\���ϐ� ���Z�@/���[�_
	uchar	disp_flag = 1;
	uchar	disp_item_flag = 1;
	char 	volume_Actuary;					// ���Z�@�̉���
	char 	volume_Reader;					// ���[�_�̉���

// MH810103 GG119202(S) �I���Œ艹�ʐݒ��ԂŎ��ԑщ��ʐؑ։�ʂɑJ�ڂ��ύX�Ȃ��Ŗ߂�ƏI���Œ艹�ʂ��I������Ă���(#5094)
	// ���ʐݒ萔���擾 (���ʃp�^�[���ݒ�p�����[�^�̈ʒu�͕ω��Ȃ�)
	ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (MAX_VOICE_GUIDE < ptn) {
		// �ő�ݒ萔��3
		ptn = MAX_VOICE_GUIDE;
	}
	// �ݒ萔��1�ȉ��Ȃ�I���Œ�A2�ȏ�Ȃ玞�ԑѕʂɃJ�[�\�������킹��
	pos = (ptn != 0);
// MH810103 GG119202(E) �I���Œ艹�ʐݒ��ԂŎ��ԑщ��ʐؑ։�ʂɑJ�ڂ��ύX�Ȃ��Ŗ߂�ƏI���Œ艹�ʂ��I������Ă���(#5094)
	for( ; ; ) {

		// �������ē����ԁ���ʕ\��
		if ( 1 == disp_flag){
			// ���ʐݒ萔���擾 (���ʃp�^�[���ݒ�p�����[�^�̈ʒu�͕ω��Ȃ�)
			ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
			if (MAX_VOICE_GUIDE < ptn) {
				// �ő�ݒ萔��3
				ptn = MAX_VOICE_GUIDE;
			}
			// ���Z�@�̉��ʎ擾
			volume_Actuary = get_volume(ptn);
			// ���[�_�̉��ʎ擾
			volume_Reader = get_volume_EC(ptn);
			// �ݒ萔��1�ȉ��Ȃ�I���Œ�A2�ȏ�Ȃ玞�ԑѕʂɃJ�[�\�������킹��
// MH810103 GG119202(S) �I���Œ艹�ʐݒ��ԂŎ��ԑщ��ʐؑ։�ʂɑJ�ڂ��ύX�Ȃ��Ŗ߂�ƏI���Œ艹�ʂ��I������Ă���(#5094)
//			// �ݒ萔��1�ȉ��Ȃ�I���Œ�A2�ȏ�Ȃ玞�ԑѕʂɃJ�[�\�������킹��
//			pos = (ptn != 0);
// MH810103 GG119202(E) �I���Œ艹�ʐݒ��ԂŎ��ԑщ��ʐؑ։�ʂɑJ�ڂ��ύX�Ȃ��Ŗ߂�ƏI���Œ艹�ʂ��I������Ă���(#5094)

			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "�������ē����ԁ��@�@�@�@�@�@�@"
			grachr(1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[130]);		// "���݂̐ݒ�F
			if (ptn == 0) {
				grachr(1, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);	// �I���Œ艹��
			} else {
				grachr(1, 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);	// ���ԑѕʉ��ʐؑ�
			}
			memset(str_vol, 0, sizeof(str_vol));
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[170]);		// ���݂̉��ʁF���Z�@�^���[�_
			sprintf(str_vol, "�@%02d�@�^�@%02d�@", (int)volume_Actuary, (int)volume_Reader);
			grachr(3, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_vol);

			Fun_Dsp( FUNMSG[99] );	//"�@���@�@���@�e�X�g ����  �I�� "
			disp_flag = 0;
		}
		if ( 1 == disp_item_flag){
			grachr(5, 6, 12, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[132]);		// �I���Œ艹��
			grachr(6, 6, 16, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[133]);		// ���ԑѕʉ��ʐؑ�
			disp_item_flag = 0;
		}
		
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
			Lagcan(OPETCBNO, 6);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
			return MOD_CHG;
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm( AVM_STOP );
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
			return MOD_EXT;
		case KEY_TEN_F4:							// �Ǐo(F4)
			BUZPI();
			if (pos == 0) {
				// �I���Œ艹��
				// �����ύX�@��I�����
				ret = ECVoiceGuide_SelectTarget(&volume_Actuary, &volume_Reader, 0, 0 );
			} else {
				// ���ԑѕʉ��ʐؑ�
				ret = UsMnt_ECVoiceGuideTimeExtra(&volume_Actuary, &volume_Reader);
				// �ݒ萔���m�F
				ptn = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053�E
				volume_Actuary = get_volume(ptn);
				volume_Reader = get_volume_EC(ptn);
			}
			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
				OpelogNo = 0;
				SetChange = 1;					// FLASH���ގw��
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
				mnt_SetFtpFlag( FTP_REQ_NORMAL ); // FTP�X�V�t���O
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			// �����e�i���XOFF
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			if( ret == MOD_CHG ){
//				return MOD_CHG;
//			}
			if( ret == MOD_CHG || ret == MOD_CUT ){
				return ret;
			}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F3:							// (F3) �e�X�g
			BUZPI();
			Ope_DisableDoorKnobChime();
			// �����e�X�g�@��I����ʂ�\��
			ret = ECVoiceGuide_SelectTarget(&volume_Actuary, &volume_Reader, ptn, 1 );
			// �����e�i���XOFF
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
			disp_flag = 1;
			disp_item_flag = 1;
			break;
		case KEY_TEN_F1:							// (F1)
		case KEY_TEN_F2:							// (F2)
			BUZPI();
			// �I���E���ԑт̃J�[�\���ړ�
			pos ^= 1;
			disp_item_flag = 1;
			break;
		default:
			break;
		}
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			���ԑѕ�,���[�_�̉��ʎ擾
//[]----------------------------------------------------------------------[]
//	@return			vol		:	����
//	@param[in]		num		:	�ݒ萔
//	@author			Inaba
//	@note			get_volume(char num)�Ɠ��l
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_volume_EC(char num)
{
	char ptn, i, vol, pos;
	short nowtime, settime1, settime2;
	
	nowtime = (short)CLK_REC.hour * 100 + (short)CLK_REC.minu;	// ���ݎ���
	// �J�n���Ԃ̃p�^�[�����擾
	if (num <= 1) {
		ptn = 1;				// �ݒ萔��1�ȉ��̓p�^�[��1
	} else {
		for (i = 0; i < num - 1; i++) {
			// ���ݎ������ݒ�p�^�[��i��i+1�̊Ԃɂ��邩���肷��
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054�`
			if (settime1 <= nowtime) {
				settime2 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 55), 4, 1);
				if (settime1+1 < settime2){
					if (nowtime < settime2) {
						ptn = (char)(i + 1);
						break;
					}
				}else{
					ptn = (char)(i + 1);
					break;
				}
			}
		}
		if (i == num - 1) {
			ptn = num;
		}
	}
	// ���ʎ擾
	switch (ptn) {
	case 1:	pos = 5;	break;
	case 2:	pos = 3;	break;
	case 3:	pos = 1;	break;
	default:			return 0;
	}
	vol = (char)prm_get(COM_PRM, S_ECR, 27, 2, pos);
	
	return vol;
}

//[]----------------------------------------------------------------------[]
//	@brief			���[�U�[�����e�i���X�F�ΏۑI��
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume_Actuary	���Z�@���ʒl�i�[�̈�ւ̃|�C���^
//					volume_Reader	���[�_���ʊi�[�̈�ւ̃|�C���^
//					nset			���ʐݒ�p�^�[����
//					swit			0: �����ύX
//									1: �����e�X�g
//	@return			ret		MOD_CHG : mode change
//							0		: ����I��
//							1		: ����I��(�������݂���)
//	@author			Inaba
//	@note
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort ECVoiceGuide_SelectTarget(char *vol_actuary, char *vol_reader, ushort ptn, ushort swit )
{

	char 	volume_Actuary;					// ���Z�@�̉���
	char 	volume_Reader;					// ���[�_�̉���
	uchar	pos = 0;						// �J�[�\���ʒu
	ushort	msg;
	ushort	disp_item_flag = 1;
	ushort	disp_all = 1;
	ushort	ret = 0;
// MH810100(S) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
	// ���Z�@�̉��ʎ擾
	volume_Actuary = *vol_actuary;
	// ���[�_�̉��ʎ擾
	volume_Reader = *vol_reader;

	for( ; ; ) {

		if(disp_all){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);		// "�������ē����ԁ��@�@�@�@�@�@�@"
			if(!swit){
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[171]);	// "���ʕύX�̑Ώۂ�I�����ĉ������B"
				Fun_Dsp( FUNMSG[25] );	//"�@���@�@���@�@�@�@ �Ǐo  �I�� "
			}
			else{
				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[172]);	// "�����e�X�g�����s���܂��B"
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[173]);	// "�Ώۂ�I�����ĉ������B"
				Fun_Dsp( FUNMSG[120] );	//"�@���@�@���@�@�e�X�g�@�@   �I�� "
			}
			disp_all = 0;
		}

		if ( 1 == disp_item_flag){
			grachr(4, 12, 6, (0==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[174]);		// ���Z�@
			grachr(5, 10, 10, (1==pos), COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[175]);		// ���σ��[�_
			disp_item_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
				Lagcan(OPETCBNO, 6);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
// MH810100(S) Y.Yamauchi 20191220 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				ope_anm( AVM_STOP );
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191220 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Lagcan(OPETCBNO, 6);
				return MOD_CHG;
			case KEY_TEN_F5:							// �I��(F5)
				BUZPI();
// MH810100(S) Y.Yamauchi 20191220 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				ope_anm( AVM_STOP );
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191220 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Lagcan(OPETCBNO, 6);
				return ret;
			case KEY_TEN_F4:							// �Ǐo(F4)
				// 	���ʃe�X�g�̏ꍇ�A�������Ȃ�
				if(swit == 1){
					continue;
				}
				BUZPI();
				if (pos == 0) {
					//  ���Z�@�I�� �I���Œ艹��
					ret = UsMnt_ECVoiceGuideTimeAllDay(&volume_Actuary, ptn, 0);
				} else {
					// ���[�_�I�� �I���Œ艹��
					ret = UsMnt_ECVoiceGuideTimeAllDay(&volume_Reader, ptn , 1);
				}
				Ope_EnableDoorKnobChime();
				// �����e�i���XOFF
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( ret == MOD_CHG ){
//					return MOD_CHG;
//				}
				if( ret == MOD_CHG || ret == MOD_CUT ){
					return ret;
				}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
				disp_item_flag = 1;
				disp_all = 1;
				break;
			case KEY_TEN_F3:							// (F3)
				// 	�����ύX�ꍇ�A�������Ȃ��B
				if(swit == 0){
					continue;
				}
				if(pos == 0){
					// ���Z�@�I���@�����e�X�g
					if (volume_Actuary == 0) {	// 0�Ȃ�s�B�s�B
						BUZPIPI();
						continue;
					}
					Ope_DisableDoorKnobChime();
					// �����I���C�x���g�������̂ŁA�A���[���Ď��^�C�}�Ԃ̓A���[����}�~����
					Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);

// MH810100(S) Y.Yamauchi 2020/02/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					VoiceGuideTest((char)volume_Actuary);	 // �u�����p���肪�Ƃ��������܂����B�܂��́`�v
					lcd_volume = Anounce_volume( (uchar)volume_Actuary );
					PKTcmd_audio_volume( lcd_volume );		// �A�i�E���X���ʂ�LCD�ɑ��M
// MH810100(E) Y.Yamauchi 2020/02/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
					
				} else {
					// ���[�_�I���@�����e�X�g
					BUZPI();
					ret = ECVoiceGuide_VolumeTest((char)volume_Reader);
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( ret == MOD_CUT ){
						return MOD_CUT;
					}
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
					// �����e�i���XOFF(����߂Ă�����)
					if( !OPE_SIG_DOORNOBU_Is_OPEN ){
						return MOD_CHG;
					}
					disp_all = 1;
				}
				disp_item_flag = 1;
				break;
			case KEY_TEN_F1:							// (F1)
			case KEY_TEN_F2:							// (F2)
				BUZPI();
				// ���Z�@�E���[�_�@�J�[�\���ړ�
				pos ^= 1;
				disp_item_flag = 1;
				break;
			default:
				break;
		}
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			�I���Œ艹�ʐݒ�iEC���[�_�ڑ����j
//[]----------------------------------------------------------------------[]
//	@param[in,out]	volume	���ʒl�i�[�̈�ւ̃|�C���^
//	@param[in]		ptn		0: �I���Œ艹�ʐݒ�
//							1: �p�^�[���P�i���ԑѕʉ��ʐؑցj
//							2: �p�^�[���Q�i���ԑѕʉ��ʐؑցj
//							3: �p�^�[���R�i���ԑѕʉ��ʐؑցj
// 					target  0: ���Z�@
//							1: ���[�_
//	@return			ret		MOD_CHG : mode change
//							0		: ����I��
//							1		: ����I��(�������݂���)
//	@author			Inaba
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
ushort	UsMnt_ECVoiceGuideTimeAllDay(char *volume, ushort ptn, uchar target)
{
	ushort	msg;
	ushort	ret = 0;
	uchar	disp_item_flag = 1;
	uchar	disp_all = 1;
	uchar	result = 2;  // 0 = OK, 1 = NG, 2 = �����\�����Ȃ�(����)
	uchar	max_vol;
	uchar	min_vol;
	ushort	vol;
	long	*pvol;							// ���Z�@����
	uchar	each_volume[ MAX_VOICE_GUIDE ];	/*	�p�^�[���P�`�R�̊e����
											 *	each_volume[0]:�p�^�[���P�i���ԑѕʉ��ʐؑցj�^�I���Œ艹��
											 *	each_volume[1]:�p�^�[���Q�i���ԑѕʉ��ʐؑցj
											 *	each_volume[2]:�p�^�[���R�i���ԑѕʉ��ʐؑցj
											 */
	char	f_write = 0;
	ushort	column;							// �`��ʒu
// MH810100(S) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	lcd_volume;
// MH810100(E) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X�i�����e�i���X
	
	if (MAX_VOICE_GUIDE < ptn) {
		// �ő�ݒ萔��3
		ptn = MAX_VOICE_GUIDE;
	}
	// ���Z�@�̉��ʎ擾
// MH810103 GG119202(S) ���ݎ��ԑт̉��ʂ��\������Ȃ�
	vol = (ushort)*volume;
// MH810103 GG119202(E) ���ݎ��ԑт̉��ʂ��\������Ȃ�
	if(target == 0){
// MH810103 GG119202(S) ���ݎ��ԑт̉��ʂ��\������Ȃ�
//		vol = get_volume(ptn);
// MH810103 GG119202(E) ���ݎ��ԑт̉��ʂ��\������Ȃ�
		pvol = &CPrmSS[S_SYS][51];			// 01-0051
		max_vol = (uchar)ADJUS_MAX_VOLUME;
		min_vol = (uchar)ADJUS_MIN_VOLUME;
	} else {
// MH810103 GG119202(S) ���ݎ��ԑт̉��ʂ��\������Ȃ�
//		vol = get_volume_EC(ptn);
// MH810103 GG119202(E) ���ݎ��ԑт̉��ʂ��\������Ȃ�
		pvol = &CPrmSS[S_ECR][27];			// 50-0027
		max_vol = (uchar)EC_MAX_VOLUME;
		min_vol = (uchar)EC_MIN_VOLUME;
	}

	// ���ݐݒ肳��Ă��鉹�ʂ��L��
	memset( each_volume, 0, sizeof(each_volume) );
	each_volume[ 0 ] = (uchar)( ( *pvol / 10000 ) % 100 );	// �p�^�[���P�̉��ʂ����o��
	each_volume[ 1 ] = (uchar)( ( *pvol / 100 ) % 100 );	// �p�^�[���Q�̉��ʂ����o��
	each_volume[ 2 ] = (uchar)( *pvol % 100 );				// �p�^�[���R�̉��ʂ����o��

	if (vol > max_vol) {
		// �ő�l���傫���l�̏ꍇ�A�ő�l�ɕύX����
		vol = max_vol;
	}
	if (vol < min_vol) {
		// �ŏ��l��菬�����l�̏ꍇ�A�ŏ��l�ɕύX����
		vol = min_vol;
	}

	for( ; ; ) {

		if(disp_all){
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
			if( target == 0 ){
				// ���Z�@�I���H
				grachr(1, 0, 6, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[174]);	// "���Z�@"
// MH810103 GG119202(S) �\�����������ӂ�C��
//				grachr(1, 6, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "�@�@�@�̉��ʂ�ύX���܂�"
				grachr(1, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "�@�@�@�̉��ʂ�ύX���܂�"
// MH810103 GG119202(E) �\�����������ӂ�C��
			}
			else {
				// ���σ��[�_�I���H
				grachr(1, 0, 10, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[175]);	// "���σ��[�_"
// MH810103 GG119202(S) �\�����������ӂ�C��
//				grachr(1, 10, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "�@�@�@    �̉��ʂ�ύX���܂�"
				grachr(1, 10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[176]);// "�@�@�@    �̉��ʂ�ύX���܂�"
// MH810103 GG119202(E) �\�����������ӂ�C��
			}
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[136]);	// �����L�[�ŉ��ʂ�ݒ肵�ĉ�����
			grachr(4, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[137]);	// ���ʐݒ�F�@�@�@�@�@�@�@�@�@�@
			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[138]);	// 1�@�@�@�@�@�@�@�@�@�@�@�@�@ 16
			disp_all = 0;
		}

		// �������ē����ԁ���ʕ\��
		if ( 1 == disp_item_flag ){
			if(result == 2){
			grachr(3, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, UMSTR3[154]);
		}	
			opedsp(4, 10, vol, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);// ����
			if ( vol <= max_vol ) {
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_DARKORANGE);
				PKTcmd_text_1_backgroundcolor( COLOR_DARKORANGE );
// MH810100(E) S.Takahashi 2019/12/25
				column = (ushort)(vol*2);
				if(column) {
					grachr( 6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);		// �v���O���X�o�[�\��
				}
// MH810100(S) S.Takahashi 2019/12/25
//				dsp_background_color(COLOR_WHITE);
				PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
// MH810100(E) S.Takahashi 2019/12/25
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
			}
			if ( vol > 0) {
				Fun_Dsp( FUNMSG[100] );
			} else {
				Fun_Dsp( FUNMSG[13] );
			}
			disp_item_flag = 0;
		}

		if(result != 2){
			// result = 0   �����ɐ������܂����B
			// result = 1   �����Ɏ��s���܂����B
			grachr(3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[(182 + result)]);
			result = 2;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
			case KEY_TEN_F5:							// �I��(F5)
				BUZPI();
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				ope_anm(AVM_STOP);
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
				Lagcan(OPETCBNO, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if (msg == KEY_MODECHG) {
//					return MOD_CHG;
//				} else {
//					return ret;
//				}
				if (msg == KEY_MODECHG) {
					return MOD_CHG;
				} else if (msg == LCD_DISCONNECT) {
					return MOD_CUT;
				}else{
					return ret;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			
			case KEY_TEN_F4:							// ����(F4)
				if(target == 0){
					BUZPI();
				}
				// ���ʏ�����
				f_write = 1;
				*volume = (char)vol;
				switch ( ptn ) {
					case 0:
						*pvol = (long)( (long)*volume * 10000L );
						break;
					case 1:
						*pvol = (long)(( (long)*volume * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)each_volume[ 2 ]);
						break;
					case 2:
						*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)*volume * 100 ) + (long)each_volume[ 2 ]);
						break;
					case 3:
						*pvol = (long)(( (long)each_volume[ 0 ] * 10000L ) + ( (long)each_volume[ 1 ] * 100 ) + (long)*volume);
						break;
					default:
						*pvol = 0;	// ptn���͈͊O�̏ꍇ�͂O�ɂ��Ă���
						break;
				}
				// ���엚��o�^�EFLASH�ɃZ�[�u
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				if (f_write == 1 && ptn == 0) {
					CPrmSS[S_SYS][53] = 0;			// 01-0053
					CPrmSS[S_SYS][54] = 0;			// 01-0054
					CPrmSS[S_SYS][55] = 0;			// 01-0055
					CPrmSS[S_SYS][56] = 0;			// 01-0056
				}
				if(target == 1){	// ���[�_
					result = VoiceGuide_VolumeChange( );
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( result == 9 ){
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
					// �����e�i���XOFF(����߂Ă�����)
					if( !OPE_SIG_DOORNOBU_Is_OPEN ){
						return MOD_CHG;
					}
					disp_all = 1;
					disp_item_flag = 1;
				}
				ret = 1;
// MH810100(S) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
 				mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;
			case KEY_TEN_F3:	/* (F3) */
				// �����𗬂�
				if (vol != 0) {		// 0�Ȃ牽�����Ȃ�
					if(target == 0){	// ���Z�@
						Ope_DisableDoorKnobChime();
						// �����I���C�x���g�������̂ŁA�A���[���Ď��^�C�}�Ԃ̓A���[����}�~����
						Lagtim(OPETCBNO, 6, OPE_DOOR_KNOB_ALARM_START_TIMER);
// MH810100(S) Y.Yamauchi 2020/02/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						VoiceGuideTest((char)vol);	// �u�����p���肪�Ƃ��������܂����B�܂��́`�v
						lcd_volume = Anounce_volume( (uchar)vol );
						PKTcmd_audio_volume( lcd_volume );		// �A�i�E���X���ʂ�LCD�ɑ��M
// MH810100(E) Y.Yamauchi 2020/02/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
					} else {	//�@���[�_
						Ope_DisableDoorKnobChime();
						BUZPI();
						ret = ECVoiceGuide_VolumeTest((char)vol);
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
						if( ret == MOD_CUT ){
							return MOD_CUT;
						}
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
						// �����e�i���XOFF(����߂Ă�����)
						if( !OPE_SIG_DOORNOBU_Is_OPEN ){
							return MOD_CHG;
						}
						disp_item_flag = 1;
						disp_all = 1;
					}
				}
				break;
			case KEY_TEN_F1:							// (F1)
				BUZPI();
				// �v���O���X�o�[���ֈړ�
				if (vol > min_vol) {
					vol--;
					disp_item_flag = 1;
				}
				break;
			case KEY_TEN_F2:							// (F2)
				BUZPI();
				// �v���O���X�o�[�E�ֈړ�
				if (vol < max_vol) {
					vol++;
					disp_item_flag = 1;
				}
				break;
			default:
				break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			���ԑѕʉ��ʐؑ֐ݒ�E�������ʉ�ʁiEC���[�_�j
//[]----------------------------------------------------------------------[]
//	@param[in,out]	result	0	: ����
//							1	: ���s
//							2	: �\�����Ȃ�
//	@return			ret		0	: �J�ڌ��ɑJ�ڂ���B
// 							9	: �ؒf�ʒm
//	@author			Inaba
//	@note			None
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/06/13<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort ECVoiceGuideTimeExtra_Resultdisplay(uchar result)
{
	ushort	msg;
	ushort	ret = 1;

	if( result == 2 ){
		ret = 0;
		return ret;
	}
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
	else if( result == 9 ){
		return MOD_CUT;
	}
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, UMSTR3[182 + result]);
	// result == 0  " ���������ɐ������܂����B���� "
	// result == 1  " ���������Ɏ��s���܂����B���� "
		
	Fun_Dsp( FUNMSG[8] );	//"                    �@ �I��  "

	while(ret) {
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/20 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			ope_anm(AVM_STOP);
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 2019/12/20 �Ԕԃ`�P�b�g���X(�����e�i���X)
			Lagcan(OPETCBNO, 6);
			ret = 0;
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case LCD_DISCONNECT:						// �ؒf�ʒm
			PKTcmd_audio_end( 0, 0 );				// �A�i�E���X�I���v��
			Lagcan(OPETCBNO, 6);
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
		default:
			break;
		}
	}
	displclr(2);
	return ret;
}

//[]----------------------------------------------------------------------[]
//	@brief			���ԑѕʉ��ʐؑ֐ݒ�iEC���[�_�j
//[]----------------------------------------------------------------------[]
//	@param[in,out]	vol_actuary	���Z�@���ʒl�i�[�̈�ւ̃|�C���^
//					vol_reader	���[�_���ʒl�i�[�̈�ւ̃|�C���^
//	@return			ret		1	: �I���Œ��ʂɈڍs�i���g�p,�R�����g�Q�Ɓj
//							0	: ����I��
//	@author			Inaba
//	@note			None
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/25<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static ushort	UsMnt_ECVoiceGuideTimeExtra(char *vol_actuary, char *vol_reader)
{
	ushort	msg;
	uchar	disp_flag = 1;
	uchar	value_ref = 1;
	ushort	pos_l = 0;
	ushort	pos_c = 0;	// 0:��,1:��
	struct	GUIDE_TIME_EC guideTime;
	char	str_vol[6];
	ushort	tNum;
	ushort	num;
	ulong	tmpvol_actuary;
	ulong	tmpvol_reader;
	ushort	calc_flag = 0;
	char	cl_flag = 1;
	ushort	i;
	ushort	ret = 0;
	char	flag;
	ushort	tmInput;
	long	*ptim, *ptim_reader;
	const char prm_pos[3] = {5, 3, 1};
	ptim = CPrmSS[S_SYS];				// ���Z�@���� ���ʐݒ�p�^�[��
	ptim_reader = CPrmSS[S_ECR];		// ���[�_����

	// �ݒ萔�擾
	tNum = (short)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if (tNum == 0) {
		tNum = 1;
	} else if (tNum > MAX_VOICE_GUIDE) {
		tNum = MAX_VOICE_GUIDE;
	}
	num = tNum;

	// ������
	memset(&guideTime, 0, sizeof(struct GUIDE_TIME_EC));
	
	for (i = 0; i < num; i++) {
		// �J�n���� 01-0054,01-0055,01-0056
		guideTime.gt[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// ��
		guideTime.gt[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// ��
		// ���� 01-0051�@�A,�B�C,�D�E
		guideTime.gt[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
		// ���� 50-0027�@�A,�B�C,�D�E
		guideTime.volume_reader[i] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[i]);
		value_ref = 0;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[73]);	// "�������ē����ԁ��@�@�@�@�@�@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);	// No  �J�n����   �I������	 ����

	tmInput = guideTime.gt[0].sTime[0] * 100 + guideTime.gt[0].sTime[1];
	while (1) {
		if(value_ref){
			for (i = 0; i < num; i++) {
			// �J�n���� 01-0054,01-0055,01-0056
			guideTime.gt[i].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 3);	// ��
			guideTime.gt[i].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 2, 1);	// ��
			// ���� 01-0051�@�A,�B�C,�D�E
			guideTime.gt[i].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[i]);
			// ���� 50-0027�@�A,�B�C,�D�E
			guideTime.volume_reader[i] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[i]);
			value_ref = 0;
			}
		}

		if ( 1 == disp_flag ){
			calc_flag = Calc_tmset_sc(num, guideTime.gt);
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[134]);														// No  �J�n����   �I������	 ����
			for( i = 0; i < MAX_VOICE_GUIDE ; ++i ){
				if( i < num ){
					if( guideTime.gt[i].eTime[1] == -1 ){
						grachr((ushort)(2 + i), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);								// �@ �|�|�F�|�| �|�|�F�|�|�@�|�|
					}
					flag = (i == pos_l) && (0 == pos_c);
					opedsp((ushort)(i + 2), 3,	(ushort)guideTime.gt[i].sTime[0], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // �@ �����F�|�| �|�|�F�|�|�@�|�|
					grachr((ushort)(i + 2), 7,	2, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 	// �R�����𔽓]
					opedsp((ushort)(i + 2), 9,	(ushort)guideTime.gt[i].sTime[1], 2, 1, (ushort)flag, COLOR_BLACK, LCD_BLINK_OFF); // �@ �|�|�F���� �|�|�F�|�|�@�|�|
					if( guideTime.gt[i].eTime[1] != -1 ){
						opedsp((ushort)(i + 2), 14, (ushort)guideTime.gt[i].eTime[0], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// �@ �|�|�F�|�| �����F�|�|�@�|�|
						grachr((ushort)(i + 2), 18,	2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]); 			// �R�����𐳓]�\��
						opedsp((ushort)(i + 2), 20, (ushort)guideTime.gt[i].eTime[1], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);		// �@ �|�|�F�|�| �|�|�F�����@�|�|
					}
					memset(str_vol, 0, sizeof(str_vol));
					sprintf(str_vol, "%02d/%02d", (int)guideTime.gt[i].volume, (int)guideTime.volume_reader[i]);
					grachr((ushort)(i + 2), 25, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_vol);				// �@ �|�|�F�|�| �|�|�F�|�|�@����
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//�����|�|�F�|�| �|�|�F�|�|�@�|�|
				}else{
					grachr((ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[135]);									// �@ �|�|�F�|�| �|�|�F�|�|�@�|�|
					opedsp((ushort)(i + 2), 0,	(ushort)(i + 1),  1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);							//�����|�|�F�|�| �|�|�F�|�|�@�|�|
				}
			}

			if( tNum > 1 && (pos_l + 1) == tNum ){
				Fun_Dsp( FUNMSG[101] );					// "�@���ʁ@�@���@�@ �폜 �@�����@�@�I���@"
			}else{
				Fun_Dsp( FUNMSG[102] );					// "�@���ʁ@�@���@�@�@�@�@�@�����@�@�I���@"
			}
			disp_flag = 0;
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
		case KEY_TEN_F5:							// �I��(F5)
			BUZPI();
			return ret;
		case KEY_TEN_F4:							// ����(F4)
			if ( 1 == calc_flag ){
				// ���Ԃ�������
				ptim[54 + pos_l] = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				if( tNum < pos_l + 1 ){
					tNum = pos_l + 1;
				}
				ptim[53] = tNum;

				// ���ʂ̌p�����̒l�̂܂܁u�����v�����ꍇ�ɂ́A���̉��ʂŊm�肳����
				switch (pos_l) {
					case 0:
						// ���Z�@���ʕۑ�
						tmpvol_actuary = ptim[51] % 10000;
						tmpvol_actuary += (ulong)(guideTime.gt[pos_l].volume) * 10000;
						// ���[�_���ʕۑ�
						tmpvol_reader = ptim_reader[27] % 10000;
						tmpvol_reader += (ulong)(guideTime.volume_reader[pos_l]) * 10000;
						break;
					case 1:
						// ���Z�@���ʕۑ�
						tmpvol_actuary  = ptim[51] / 10000;
						tmpvol_actuary *= 10000;
						tmpvol_actuary += ptim[51] % 100;
						tmpvol_actuary += (ulong)(guideTime.gt[pos_l].volume) * 100;
						// ���[�_���ʕۑ�
						tmpvol_reader  = ptim_reader[27] / 10000;
						tmpvol_reader *= 10000;
						tmpvol_reader += ptim_reader[27] % 100;
						tmpvol_reader += (ulong)(guideTime.volume_reader[pos_l]) * 100;
						break;
					case 2:
						// ���Z�@���ʕۑ�
						tmpvol_actuary  = ptim[51] / 100;
						tmpvol_actuary *= 100;
						tmpvol_actuary += guideTime.gt[pos_l].volume;
						// ���[�_���ʕۑ�
						tmpvol_reader  = ptim_reader[27] / 100;
						tmpvol_reader *= 100;
						tmpvol_reader += guideTime.volume_reader[pos_l];
						break;
					default:
						tmpvol_actuary = 0;
						tmpvol_reader = 0;
						break;
				}
				ptim[51] = tmpvol_actuary;
				ptim_reader[27] = tmpvol_reader;
				// �m�肵���Ƃ��ɂ͈�O�̉��ʂ��p�����ĕ\��
				if ( pos_l < MAX_VOICE_GUIDE ){
					if( !ptim[54 + pos_l + 1] )	{						// ���̊J�n���������ݒ�̏ꍇ
						guideTime.gt[pos_l+1].volume = guideTime.gt[pos_l].volume;
						guideTime.volume_reader[pos_l+1] = guideTime.volume_reader[pos_l];
					}
				}
				// ���엚��o�^�EFLASH�ɃZ�[�u
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;									// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				

				ret = VoiceGuide_VolumeChange( );
				ret = ECVoiceGuideTimeExtra_Resultdisplay(ret);
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if( ret == MOD_CUT ){
					return ret;
				}
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
				// �����e�i���XOFF(����߂Ă�����)
				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
					return MOD_CHG;
				}

				// �J�[�\���ړ�
				++pos_l;
				if(pos_l < MAX_VOICE_GUIDE) {
					guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
					guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
				}
				disp_flag = 1;
				pos_c = 0;
				if ( pos_l > tNum || pos_l >= 3 ){
					pos_l = 0;
					if(num > 1 && tNum < 3 ){
						num--;
					}
				} else {
					if ( tNum <= pos_l ){
						num++;
					}
				}
				tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				cl_flag = 1;
				// ���ʕύX�f�[�^���M
			}
			break;
		case KEY_TEN_F2:							// (F2)
			// �ŐV�̉��ʐݒ����ʂɔ��f������
			guideTime.gt[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]); // ���Z�@�̉���
			guideTime.volume_reader[pos_l] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[pos_l]); // ���[�_�̉���
			if ( pos_l+1 < MAX_VOICE_GUIDE ){
				if( !ptim[54 + pos_l + 1] )	{								// ���̊J�n���������ݒ�̏ꍇ
					guideTime.gt[pos_l+1].volume = guideTime.gt[pos_l].volume;	// ��O�̐��Z�@���ʂ��p��
					guideTime.volume_reader[pos_l+1] = guideTime.volume_reader[pos_l];	// ��O�̃��[�_���ʂ��p��
				}
			}
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
			BUZPI();
			
			++pos_l;
			if(pos_l < MAX_VOICE_GUIDE) {
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
			}
			disp_flag = 1;
			pos_c = 0;
			if ( pos_l > tNum || pos_l >= 3 ){
				pos_l = 0;
				if(num > 1 && tNum < 3 ){
					num--;
				}
			} else {
				if ( tNum <= pos_l ){
					num++;
				}
			}
			tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
			cl_flag = 1;
			break;		
		case KEY_TEN_F3:	// (F3)
			// �ݒ�폜(�J�n�����A���ʂ��N���A�A�ݒ萔��-1)
			if( tNum > 1 && (pos_l + 1) == tNum ){
				tNum--;
				ptim[53] = tNum;
				ptim[54 + pos_l] = 0;
				switch (pos_l) {
				case 0:
					tmpvol_actuary = ptim[51] % 10000;
					tmpvol_reader = ptim_reader[27] % 10000;
					break;
				case 1:
					/*
					 *	@note	�p�^�[���Q���폜�������Ƀp�^�[���R�̉��ʒl���c���Ă����Ȃ��悤�ɂ���B�i�O�N���A����j
					 */
					tmpvol_actuary = ( ptim[51] / 10000 ) * 10000;
					tmpvol_reader= ( ptim_reader[27] / 10000 ) * 10000;
					break;
				case 2:
					tmpvol_actuary  = ptim[51] / 100;
					tmpvol_actuary *= 100;
					tmpvol_reader  = ptim_reader[27] / 100;
					tmpvol_reader *= 100;
					break;
				default:
					tmpvol_actuary = 0;
					tmpvol_reader = 0;
					break;
				}
				ptim[51] = tmpvol_actuary;
				ptim[27] = tmpvol_reader;
				// ���엚��o�^�EFLASH�ɃZ�[�u
				OpelogNo = OPLOG_VLSWTIME;
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
				
				ret = VoiceGuide_VolumeChange(  );
// MH810100(S) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if( ret == 9 ){
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2020/03/11 �Ԕԃ`�P�b�g���X(�����e�i���X)
				// �����e�i���XOFF(����߂Ă�����)
				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
					return MOD_CHG;
				}

				// �J�[�\���ړ�
				++pos_l;
				if(pos_l < MAX_VOICE_GUIDE) {
					guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
					guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
				}
				disp_flag = 1;
				pos_c = 0;
				if ( pos_l > tNum || pos_l >= 3 ){
					pos_l = 0;
					if(num > 1 && tNum < 3 ){
						num--;
					}
				} else {
					if ( tNum <= pos_l ){
						num++;
					}
				}
				tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
				cl_flag = 1;
			}
			break;
		case KEY_TEN_F1:							// (F1)
			BUZPI();
			// ���ʐݒ��ʂ�
			ret = ECVoiceGuide_SelectTarget( &guideTime.gt[pos_l].volume, &guideTime.volume_reader[pos_l], (ushort)(pos_l + 1), 0 );
			if(ret == 1){
				value_ref = 1;
			}
			*vol_actuary = guideTime.gt[pos_l].volume;
			*vol_reader = guideTime.volume_reader[pos_l];

			if( OpelogNo ){
				wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
				OpelogNo = 0;
				SetChange = 1;					// FLASH���ގw��
				UserMnt_SysParaUpdateCheck( OpelogNo );
				SetChange = 0;
			}
			if( ret == MOD_CHG ){
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if(ret == MOD_CUT ){
				return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			for (i = 1; i < 7; i++) {
				displclr(i);
			}
			disp_flag = 1;
			break;
		case KEY_TEN:								// ����(�e���L�[)
			// ���Ԑݒ�
			BUZPI();
			if( cl_flag ){
				tmInput = 0;
				cl_flag = 0;
			}
			tmInput = (tmInput % 1000) * 10 + msg - KEY_TEN0;
			guideTime.gt[pos_l].sTime[0] = tmInput / 100;
			guideTime.gt[pos_l].sTime[1] = tmInput % 100;
			disp_flag = 1;
			break;
		case KEY_TEN_CL:							// ���(�e���L�[)
			BUZPI();
			// �J�n���� 01-0054,01-0055,01-0056
			guideTime.gt[pos_l].sTime[0] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 3);	// ��
			guideTime.gt[pos_l].sTime[1] = (short)prm_get(COM_PRM, S_SYS, (short)(pos_l + 54), 2, 1);	// ��
			tmInput = guideTime.gt[pos_l].sTime[0] * 100 + guideTime.gt[pos_l].sTime[1];
			// ���� 01-0051�@�A,�B�C,�D�E
			guideTime.gt[pos_l].volume = (char)prm_get(COM_PRM, S_SYS, 51, 2, prm_pos[pos_l]);
			// ���� 50-0027�@�A,�B�C,�D�E
			guideTime.volume_reader[pos_l] = (char)prm_get(COM_PRM, S_ECR, 27, 2, prm_pos[pos_l]);
			disp_flag = 1;
			cl_flag = 1;
			break;
		default:
			break;
		}
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
//	@brief			���[�U�[�����e�i���X�F�g���@�\ - ���σ��[�_�������j���[
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change<br>
//							MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyMnu()�ڐA
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019-01-29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	UsMnt_ECReaderMnu(void)
{
	ushort	usUcreEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();									// ��ʃN���A
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[167]);			// "�����σ��[�_�������@�@�@�@�@�@"
		usUcreEvent = Menu_Slt( (void*)EXTENDMENU_EC, USM_EXTEC_TBL, (char)USM_EXTEC_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usUcreEvent ){

		case MNT_ECR_SHUUKEI:					// ���σ��[�_���� - �����W�v�v�����g
			usUcreEvent = UsMnt_DiditalCasheSyuukei(MNT_ECR_SHUUKEI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;

		case MNT_ECR_MEISAI:					// ���σ��[�_���� - ���p���׃v�����g
			usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_ECR_MEISAI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;

		case MNT_ECR_ALARM_LOG:					// ���σ��[�_���� - ������������L�^
			usUcreEvent = UsMnt_EcReaderAlarmLog();
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
		case MNT_ECR_MINASHI:					// ���σ��[�_���� - �݂Ȃ����σv�����g
			usUcreEvent = UsMnt_DiditalCasheUseLog(MNT_ECR_MINASHI);
			if (usUcreEvent == MOD_CHG){
				return(MOD_CHG);
			}
			else if(usUcreEvent == MOD_CUT){
				return(MOD_CUT);
			}
			break;
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���

		case MOD_EXT:		// �I���i�e�T�j
			return(MOD_EXT);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case MOD_CUT:
			return(MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case MOD_CHG:		// ���[�h�`�F���W
			return(MOD_CHG);

		default:
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ���σ��[�_�����W�v���擾  		                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetEcShuukeiCnt                                        |*/
/*| PARAMETER    : s_date : �m�[�}���C�Y�����J�n���t                       |*/
/*|              : e_date : �m�[�}���C�Y�����I�����t                       |*/
/*| RETURN VALUE : ushort   log_cnt �����W�v��                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-01-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
ushort	GetEcShuukeiCnt(ushort s_date, ushort e_date)
{
	ushort			log_date;			// ���O�i�[���t
	ushort			log_cnt;			// �w����ԓ����O����
	ushort			cnt;
	ushort			total;
	ushort			pos;				// �������n�߂�z��̈ʒu
	date_time_rec	*date_work;
	ushort			LOG_Date[3];		// ���O�i�[���t�i[0]�F�N�A[1]�F���A[2]�F���j


	total = Syuukei_sp.ec_inf.cnt - 1;
	log_cnt = 0;
	for (cnt = 1; cnt <= total; cnt++) {
		/* ���̎Q�ƈʒu�ֈړ� */
		pos = (Syuukei_sp.ec_inf.ptr + SYUUKEI_DAY_EC - cnt) % SYUUKEI_DAY_EC;
	
		/* Target�ް��̓o�^������ɰ�ײ�ޒl��get */
		date_work = &Syuukei_sp.ec_inf.bun[pos].SyuTime;
		LOG_Date[0] = date_work->Year;
		LOG_Date[1] = date_work->Mon;
		LOG_Date[2] = date_work->Day;
		log_date = dnrmlzm((short)LOG_Date[0], (short)LOG_Date[1], (short)LOG_Date[2]) ;
		
		// �͈͂ɊY�����邩�`�F�b�N
		if ((log_date >= s_date) && (log_date <= e_date)) {
			log_cnt++;
		}
	}
	return log_cnt;
}

//[]----------------------------------------------------------------------[]
//	@brief			���[�U�[�����e�i���X�F���σ��[�_���� ������������L�^
//[]----------------------------------------------------------------------[]
//	@param[in]		None
//	@return			usSysEvent		MOD_CHG : mode change<br>
// 									MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyAlarmLog�ڐA
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static	ushort UsMnt_EcReaderAlarmLog(void)
{
	ushort				msg;
	int					mode;				// 0:���O�Ȃ��A1:���O�����\�����A2:�S�f�[�^�폜�m�F��
											// 3:1���ڃv�����g���A4:�v�����g���~�v���A5:�v�����g���~�v����
	int					disp;
	ushort				log_cnt;			// ���O����
	ushort				pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
	uchar				pri;				// �Ώۃv�����^(0/R_PRI/J_PRI/RJ_PRI)
	T_FrmEcAlarmLog		FrmPrnAlarmLog;		// ������������L�^�󎚗v�����b�Z�[�W
	T_FrmPrnStop		FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
	int					cancel_key_flag;	// �v�����g���~�L�[�����t���O


	// ���O�����擾
	mode = 0;
	log_cnt = EcAlarmLog_GetCount();
	if (log_cnt != 0) {
		mode = 1;
	}

	// �^�C�g���\��
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[168]);		/* "��������������L�^���@�@�@�@�@" */
	disp = 1;

	for ( ; ; ) {
		// ��ʍX�V
		if (disp) {
			disp = 0;
			displclr(2);
			displclr(3);
			switch (mode) {
			case 0:		// ���O�Ȃ�
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);		// "�@�@�@�f�[�^�͂���܂���@�@�@"
				Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				break;
			case 1:		// ���O��������				
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[169]);		// "�@�@�@���̃f�[�^������܂��@�@"
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//				opedsp( 3, 4, log_cnt, 1, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �Ώۃ��O�����\���i�O�T�v���X�j
				opedsp( 3, 2, log_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �Ώۃ��O�����\���i�O�T�v���X�j
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
				Fun_Dsp( FUNMSG2[63] );					// " �ر�@�@�@�@ ����� �@�@�@�I�� "
				break;
			case 2:		// �S�폜�m�F
				displclr(6);
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[61]);		// "�@�@ �S�f�[�^���폜���܂� �@�@"
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);		// "�@�@�@ ��낵���ł����H �@�@�@"
				Fun_Dsp( FUNMSG[19] );					// "�@�@�@�@�@�@ �͂� �������@�@�@"
				break;
			case 3:		// �v�����g��
				displclr(6);
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[8]);		// "�@�@�@ �v�����g���ł� �@�@�@�@"
				Fun_Dsp( FUNMSG[82] );					// "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
				break;
			case 5:		// �v�����g���~�v����
				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, TGTSTR[14]);		// 	"�@ �@ �󎚂𒆎~���܂��� �@ �@"
				Fun_Dsp( FUNMSG[0] );					//  "�@�@�@�@�@�@�@�@�@ �@�@  �@�@ "
				break;
			default:
				break;
			}
		}
		
		// ���b�Z�[�W����
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
			// �h�A�m�u���ǂ����̃`�F�b�N�����{
// MH810103(s) �d�q�}�l�[�Ή� #5499 �������������ʂŃh�A�m�u���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
//			if (mode == 3 && CP_MODECHG) {
			if (mode != 3 && CP_MODECHG) {
// MH810103(e) �d�q�}�l�[�Ή� #5499 �������������ʂŃh�A�m�u���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			// BUZPI();
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			}
			break;
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)

		case KEY_TEN_F5:	/* �I��(F5) */
			if (mode >= 2) {
			// �f�[�^�Ȃ�or�f�[�^�����\�����ȊO�������Ȃ�
				break;
			}

			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F4:	/* ������(F4) */
			if (mode != 2) {
			// �S�폜�m�F���ȊO�A�������Ȃ�
				break;
			}

			BUZPI();
			// ���O�N���A���Ȃ�
			mode = 1;
			disp = 1;			// ��ʍX�V
			break;

		case KEY_TEN_F3:	/* �v�����g(F3)�A���~(F3)�A�͂�(F3) */
			if (mode == 1) {
			// ���O�����\����
				pri = R_PRI;
				if (check_printer(&pri) != 0) {
				// ���V�[�g�v�����^���؂� or �G���[������
					BUZPIPI();
					break;
				}
				BUZPI();
				cancel_key_flag = 0;
				/*------	������������L�^�󎚗v��ү���ޑ��M	-----*/
// MH810105 GG119202(S) T���v�A���󎚑Ή�
				memset(&FrmPrnAlarmLog, 0, sizeof(FrmPrnAlarmLog));
// MH810105 GG119202(E) T���v�A���󎚑Ή�
				FrmPrnAlarmLog.prn_kind = R_PRI;						// ��������
				FrmPrnAlarmLog.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// �@�B��
				pri_cmd = PREQ_EC_ALARM_LOG;
				queset( PRNTCBNO, pri_cmd, sizeof(T_FrmEcAlarmLog), &FrmPrnAlarmLog );
				mode = 3;
				disp = 1;
			}
			else if (mode == 2) {
			// �S�폜�m�F��
				BUZPI();
				// ���O�N���A
				EcAlarmLog_Clear();
				log_cnt = 0;
				mode = 0;
				disp = 1;			// ��ʍX�V
			}
			else if (mode == 3) {
			// 1���ڃv�����g��
				if (!cancel_key_flag) {
					BUZPI();
					cancel_key_flag = 1;
				}
			}
			else if (mode == 4) {
			// �v�����g���~��
				/*------	�󎚒��~ү���ޑ��M	-----*/
				FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
				queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
				BUZPI();
				mode = 5;
				disp = 1;
			}
			break;

		case KEY_TEN_F1:	/* �N���A(F1) */
			if (mode != 1) {
			// ���O�����\�����ȊO�A�������Ȃ�
				break;
			}

			BUZPI();
			mode = 2;
			disp = 1;			// ��ʍX�V
			break;

		case OPE_EC_ALARM_LOG_PRINT_1_END:	/* ��������������O1���󎚊��� */
			if (mode == 3) {
				if (cancel_key_flag) {
					/*------	�󎚒��~ү���ޑ��M	-----*/
					FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
					mode = 5;
					disp = 1;
				}
				else {
					mode = 4;
				}
			}
			break;

		default:
			// �󎚃��b�Z�[�W����
			if (pri_cmd != 0) {
				if (msg == ( pri_cmd | INNJI_ENDMASK )) { 			// �󎚏I��ү���ގ�M�H
					mode = 1;
					disp = 1;
				}
			}
			break;
		}
	} // end of for

	// �㏈��
	// �Ȃ�
}

//[]----------------------------------------------------------------------[]
//	@brief			�v�����^�G���[�`�F�b�N
//[]----------------------------------------------------------------------[]
//	@param[in/out]	*pri_kind : �Ώۃv�����^(0/R_PRI/J_PRI/RJ_PRI)
//	@return			ret		0x00 = �G���[�Ȃ�
//	@return					0x01 = �G���[����i�W���[�i���g�p�ݒ�Ȃ����܂ށj
//	@return					0x02 = ���؂�i�A���[���j����
//	@author			Inaba
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29<br> 
//                          :   GM494202_494302��Ope_isPrinterErrotExist()��update
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	check_printer(uchar *pri_kind)
{
	uchar	ret;	// ���V�[�g
	uchar	ret2;	// �W���[�i��
	uchar	back;	// ���̈󎚐�

	back = *pri_kind;

	ret = 0;
	ret2 = 0;
	switch (*pri_kind) {
	case	RJ_PRI:
	case	R_PRI:
	// ���V�[�g�v�����^�`�F�b�N
		if( (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) ||		// Printer Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_HEADHEET)) ||		// Head Heet Up Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_CUTTER)) ) {			// Cutter       Error
			ret |= 0x01;
		}

		if( ALM_CHK[ALMMDL_SUB][ALARM_RPAPEREND] != 0 ){
 		// ���؂�
			ret |= 0x02;
		}

		if (ret != 0) {
		// ���؂� or �G���[������
			*pri_kind &= (~R_PRI);		// �󎚐�F���V�[�g����
		}
		if (back == R_PRI) {
		// �󎚐恁���V�[�g�Ȃ炱���Ń`�F�b�N�I��
			break;
		}
	// no break;	// RJ_PRI
	case	J_PRI:
	// �W���[�i���v�����^�`�F�b�N
		if ((IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_PRINTCOM)) ||		// Printer Error
		    (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)) ) {		// Head Heet Up Error
			ret2 |= 0x01;
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL) ||			// �ʐM�s��
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||	// SD�J�[�h�g�p�s��
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||		// �����ݎ��s
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iE2204�̖��̕ύX�j
//			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WAKEUP_ERR)) {		// ���ڑ�
			IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED)) {		// ���ڑ�
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iE2204�̖��̕ύX�j
			ret2 |= 0x01;
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

		if (ALM_CHK[ALMMDL_SUB][ALARM_JPAPEREND] != 0) {
 		// ���؂�
			ret2 |= 0x02;
		}

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
			// SD�J�[�h�G���h
			ret2 |= 0x02;
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

		if (ret2 != 0) {
		// �ݒ�Ȃ� or ���؂� or �G���[������
			*pri_kind &= (~J_PRI);		// �󎚐�F�W���[�i������
		}
		break;
	default :
		break;
	}

	return (uchar)(ret | ret2);
}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//[]----------------------------------------------------------------------[]
//	@brief			���[�U�[�����e�i���X�F�g���@�\ - �d�q�W���[�i�����j���[
//[]----------------------------------------------------------------------[]
//	@return			ret		MOD_CHG : mode change<br>
//							MOD_EXT : F5 key
//	@author			
//	@note			
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2021-01-08
//					Update	:	
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static ushort	UsMnt_EJournalMnu(void)
{
	ushort	usEJournalEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; ) {

		dispclr();									// ��ʃN���A
		grachr(0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[187]);			// "���d�q�W���[�i�����@�@�@�@�@�@"
		usEJournalEvent = Menu_Slt( (void*)EXTENDMENU_EJ, USM_EXTEJ_TBL, (char)USM_EXTEJ_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usEJournalEvent ){
		case LCD_DISCONNECT:
			return MOD_CUT;

		case MNT_EJ_SD_INF:					// �r�c�J�[�h���
			usEJournalEvent = UsMnt_EJournalSDInfo();
			if (usEJournalEvent == MOD_CHG){
				return(MOD_CHG);
			}
			break;

		case MOD_EXT:		// �I���i�e�T�j
			return(MOD_EXT);

		case MOD_CHG:		// ���[�h�`�F���W
			return(MOD_CHG);

		default:
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			SD�J�[�h���i�d�q�W���[�i���j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=���[�h�ؑ�, MOD_EXT=�I��
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static ushort	UsMnt_EJournalSDInfo(void)
{
	ushort	ret = 0;	// �߂�l
	short	msg = -1;	// ��Mү����
	ulong	upper, lower, work;

	if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
		IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR)) {
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[188]);		// "���r�c�J�[�h��񁄁@�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[24]);		// "�@�S�̃T�C�Y�F�@�@�@�@�@�l�a�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[25]);		// "�@�󂫃T�C�Y�F�@�@�@�@�@�l�a�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[26]);		// "�@�g�p���ԁ@�F�@�@�@�@�@���ԁ@"

	Fun_Dsp( FUNMSG[8] );												// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_INFO);					// SD�J�[�h���v��

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
		switch( msg){							// ��Mү���ށH
		case LCD_DISCONNECT:
			return MOD_CUT;
		case KEY_MODECHG:						// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SD_INFO:						// SD�J�[�h���ʒm
			memcpy(&upper, &eja_work_buff[0], 4);
			memcpy(&lower, &eja_work_buff[4], 4);
			work = upper * (1 << 12);			// ��ʌ���MB�P�ʂɕϊ�
			work += (lower / (1024*1024));		// ���ʌ���MB�P�ʂɕϊ�
			opedpl3(2, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �S�̃T�C�Y

			memcpy(&upper, &eja_work_buff[8], 4);
			memcpy(&lower, &eja_work_buff[12], 4);
			work = upper * (1 << 12);			// ��ʌ���MB�P�ʂɕϊ�
			work += (lower / (1024*1024));		// ���ʌ���MB�P�ʂɕϊ�
			opedpl3(3, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �󂫃T�C�Y

			memcpy(&work, &eja_work_buff[16], 4);
			opedpl3(4, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �g�p����
			break;
		default:
			break;
		}
	}
	return ret;
}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

